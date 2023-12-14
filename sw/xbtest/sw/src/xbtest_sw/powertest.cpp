// Copyright (C) 2022 Xilinx, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cmath>

#include "powertest.h"

namespace xbtest
{

PowerTest::PowerTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
    const Power_Parameters_t & test_parameters
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_log_header        = LOG_HEADER_POWER;

    this->m_timer             = timer;
    this->m_devicemgt         = device_mgt;
    this->m_test_parameters   = test_parameters;

    this->m_num_cu = this->m_device->GetNumPowerCu();

    this->m_queue_testcase    = POWER_MEMBER.name;
    this->m_queue_thread      = POWER_MEMBER.name;
}

PowerTest::~PowerTest() = default;

bool PowerTest::PreSetup()
{
    auto global_settings_failure = RET_SUCCESS;
    this->m_state = TestState::TS_PRE_SETUP;

    LogMessage(MSG_CMN_004);

    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }

    for (uint cu_idx = 0; cu_idx < this->m_device->GetNumPowerCu(); cu_idx++)
    {
        auto cu_name = this->m_device->GetPwrCuName(cu_idx);
        // If AIE info is not defined in xcbin user metadata (backward compatibility), AIE is controlled by PL and SW does not display AIE status
        if ((this->m_xbtest_sw_config->GetPwrUseAIE(cu_name) == 1) && this->m_xbtest_sw_config->GetPwrAIEInfoExists(cu_name))
        {
            if (CheckStringInSet("AIE type", this->m_xbtest_sw_config->GetPwrAIEType(cu_name), SUPPORTED_AIE_TYPE))
            {
                global_settings_failure = RET_FAILURE;
            }
            if (CheckStringInSet("AIE control", this->m_xbtest_sw_config->GetPwrAIEControl(cu_name), SUPPORTED_AIE_CONTROL))
            {
                global_settings_failure = RET_FAILURE;
            }
            if (CheckStringInSet("AIE status", this->m_xbtest_sw_config->GetPwrAIEStatus(cu_name), SUPPORTED_AIE_STATUS))
            {
                global_settings_failure = RET_FAILURE;
            }
        }
    }

#ifdef USE_XRT
    // Setup xbtest HW IP that uses PS kernel to control AIE
    for (uint cu_idx = 0; cu_idx < this->m_device->GetNumPowerCu(); cu_idx++)
    {
        auto cu_name = this->m_device->GetPwrCuName(cu_idx);
        if (this->m_xbtest_sw_config->GetPwrUseAIE(cu_name) == 1)
        {
            if (StrMatchNoCase(this->m_xbtest_sw_config->GetPwrAIEControl(cu_name), AIE_CONTROL_PS) && !(this->m_TC_Cfg.disable_aie[cu_idx]))
            {
                this->m_powertest_ps_xrt_run = this->m_device->GetPowertestPsRun(PS_KERNEL_NAME_POWER);
                break;
            }
        }
    }
    if (this->m_powertest_ps_xrt_run != nullptr)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Using PS kernel to control AIE power"});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"PS kernel is not used to control AIE power"});
    }
#endif

    global_settings_failure |= GetTestSequence();
    global_settings_failure |= GetJsonParamBool(DISABLE_TOG_RAMP_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.disable_toggle_ramp, false);

    // macro enable
    bool disable_reg;
    bool disable_dsp;
    bool disable_bram;
    bool disable_uram;
    bool disable_aie;
    global_settings_failure |= GetJsonParamBool(DISABLE_REG_MEMBER,  this->m_test_parameters.global_config, disable_reg,  false);
    global_settings_failure |= GetJsonParamBool(DISABLE_DSP_MEMBER,  this->m_test_parameters.global_config, disable_dsp,  false);
    global_settings_failure |= GetJsonParamBool(DISABLE_BRAM_MEMBER, this->m_test_parameters.global_config, disable_bram, false);
    global_settings_failure |= GetJsonParamBool(DISABLE_URAM_MEMBER, this->m_test_parameters.global_config, disable_uram, false);
    global_settings_failure |= GetJsonParamBool(DISABLE_AIE_MEMBER,  this->m_test_parameters.global_config, disable_aie,  false);

    // Report sites disabled
    if (disable_reg)
    {
        LogMessage(MSG_PWR_001, {"FF"});
    }
    if (disable_dsp)
    {
        LogMessage(MSG_PWR_001, {"DSP"});
    }
    if (disable_bram)
    {
        LogMessage(MSG_PWR_001, {"BRAM"});
    }
    if (disable_uram)
    {
        LogMessage(MSG_PWR_001, {"URAM"});
    }
    if (disable_aie)
    {
        LogMessage(MSG_PWR_001, {"AIE"});
    }
    for (uint cu_idx = 0; cu_idx < this->m_num_cu; cu_idx++)
    {
        this->m_TC_Cfg.disable_reg[cu_idx]  = disable_reg;
        this->m_TC_Cfg.disable_dsp[cu_idx]  = disable_dsp;
        this->m_TC_Cfg.disable_bram[cu_idx] = disable_bram;
        this->m_TC_Cfg.disable_uram[cu_idx] = disable_uram;
        this->m_TC_Cfg.disable_aie[cu_idx]  = disable_aie;
    }

    // measurement output file
    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {
        auto sensor = this->m_xbtest_sw_config->GetSensorUsed();

        std::vector<std::string> first_line;
        first_line.emplace_back("Global time (s)");
        first_line.emplace_back("Test");
        first_line.emplace_back("Test time (s)");
        first_line.emplace_back("Toggle rate (%)");
        first_line.emplace_back("Measurement ID");
        first_line.emplace_back("Measurement valid");
        for (const auto & source : sensor.fan_sources)
        {
            first_line.emplace_back(source.display_name_value);
            first_line.emplace_back(source.display_name_status);
        }
        for (const auto & source : sensor.thermal_sources)
        {
            first_line.emplace_back(source.display_name_value);
            first_line.emplace_back(source.display_name_status);
        }
        for (const auto & source : sensor.power_consumption_sources)
        {
            first_line.emplace_back(source.display_name_value);
            first_line.emplace_back(source.display_name_status);
        }
        for (const auto & source : sensor.power_rail_sources)
        {
            first_line.emplace_back(source.display_name_current_value);
            first_line.emplace_back(source.display_name_current_status);
            first_line.emplace_back(source.display_name_voltage_value);
            first_line.emplace_back(source.display_name_voltage_status);
            first_line.emplace_back(source.display_name_power);
        }

        this->m_outputfile_name = "power";
        this->m_use_outputfile  = true;

        global_settings_failure |= OpenOutputFile(this->m_outputfile_name + ".csv", this->m_outputfile);

        this->m_RT_outputfile_name = this->m_outputfile_name + ".csv";
        global_settings_failure |= OpenRTOutputFile(this->m_RT_outputfile_name, this->m_RT_outputfile);

        if (global_settings_failure == RET_SUCCESS)
        {
            this->m_outputfile << StrVectToStr(first_line, ",") << "\n";
            this->m_outputfile.flush();

            this->m_RT_outputfile_head = StrVectToStr(first_line, ",");
            this->m_RT_outputfile.close();
        }
    }

    if (global_settings_failure == RET_SUCCESS)
    {
        global_settings_failure |= ParseTestSequenceSettings(this->m_test_it_list);
    }
    if (global_settings_failure == RET_FAILURE)
    {
        Abort();
    }
    return global_settings_failure;
}

void PowerTest::Run()
{
    this->m_state     = TestState::TS_RUNNING;
    this->m_result    = TestResult::TR_FAILED;

    auto test_state = TEST_STATE_FAILURE;

    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});
        test_state = RunTest();
    }

    if ((test_state < TEST_STATE_PASS) || (this->m_abort)) // any negatif state is an abort code
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (test_state > TEST_STATE_PASS) // any positive state is an error code
    {
        LogMessage(MSG_CMN_010, {GetCurrentTime()});
        this->m_result = TestResult::TR_FAILED;
    }
    else
    {
        LogMessage(MSG_CMN_011, {GetCurrentTime()});
        this->m_result = TestResult::TR_PASSED;
    }
}

void PowerTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void PowerTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

int PowerTest::Percent2Throttle( const double & percent )
{
    int temp = std::round(percent * (double)(QTY_THROTTLE_STEP-1) / (double)(100.0));
    // saturation
    if (percent < 0.0)
    {
        temp = 0;
    }
    else if (percent > 100.0)
    {
        temp = QTY_THROTTLE_STEP-1;
    }
    return temp;
}

bool PowerTest::SetPwrThrottle( const uint & test_it_cnt, const double & percent, const uint & duration, Sensor_t & sensor )
{
    auto ret = RET_SUCCESS;
    auto ramp_up    = false;
    uint tmp_time   = 0;
    if (!(this->m_TC_Cfg.disable_toggle_ramp))
    {
        if (percent > this->m_current_percent)
        {
            ramp_up = true;
        }
        // limit throttle ramp to xyz percent per second
        uint time_cnt = 0;
        while ( (abs (percent - this->m_current_percent) > POWER_GRADIENT) )
        {
            if (ramp_up)
            {
                this->m_current_percent += POWER_GRADIENT;
            }
            else
            {
                this->m_current_percent -= POWER_GRADIENT;
            }
            auto throttle = Percent2Throttle(this->m_current_percent);
            CfgThrottle(throttle);
#ifdef USE_XRT
            ret |= StartPowertestPs(this->m_current_percent);
#endif
            this->m_timer->WaitFor1sTick();

            sensor = this->m_devicemgt->GetSensorInstant();
            ResetWatchdog();

            WriteToMeasurementFile(test_it_cnt, tmp_time, sensor, this->m_current_percent);
            // reset watchdog regularly
            time_cnt++;

            auto card_power_str = NOT_APPLICABLE;
            if (sensor.card_power_found)
            {
                card_power_str = Float_to_String<double>(sensor.card_power, 1);
            }
            auto card_temperature_str = NOT_APPLICABLE;
            if (sensor.card_temperature_found)
            {
                card_temperature_str = Float_to_String<double>(sensor.card_temperature, 0);
            }
            LogMessage(MSG_PWR_060, {Float_to_String<double>(percent, 1), std::to_string(time_cnt), card_temperature_str, card_power_str, Float_to_String<double>(this->m_current_percent,1)});

            ret |= DisplayAIEStats();

            // in case of abort, interrupt on ramp up only. Always let run ramp down
            if (this->m_abort && ramp_up)
            {
                break;
            }
        }
    }
    // in case of abort,
    //      let finish the ramp down
    //      ignore any ramp up
    // when no abort, just finish the ramp
    if ( (this->m_abort && !ramp_up) || (!(this->m_abort)) )
    {
        auto throttle = Percent2Throttle(percent);
        CfgThrottle(throttle);

        // Cannot abort PS kernel, so restart it every 5s
        auto rem_div = duration % PS_KERNEL_RESTART_SEC;
        if (rem_div > 0)
        {
#ifdef USE_XRT
            ret |= StartPowertestPs(percent, rem_div);
#endif
        }
        this->m_current_percent = percent;

    }
    return ret;
}

void PowerTest::CfgThrottle( const int & throttle )
{
    uint read_data;
    for (uint cu_idx = 0; cu_idx < this->m_num_cu; cu_idx++)
    {
        this->m_device->ReadPowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);
        //LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP[" + std::to_string(cu_idx) + "]: cfg register before 0x" + NumToStrHex<unsigned int>(read_data)});
        // keep previous content and wipe out the throttle  then insert the new one
        read_data = (read_data & ~PWR_TOG_PAT_MASK) + (throttle & PWR_TOG_PAT_MASK);
        this->m_device->WritePowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);

        //this->m_device->ReadPowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);
        //LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP[" + std::to_string(cu_idx) + "]: cfg register after 0x" + NumToStrHex<unsigned int>(read_data)});
    }
}

void PowerTest::StopPowerCu()
{
    uint read_data;
    for (uint cu_idx = 0; cu_idx < this->m_num_cu; cu_idx++)
    {
        this->m_device->ReadPowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);
        // set null throttle, disable everything
        read_data = read_data & ~(PWR_CTRL_ENABLE_REG | PWR_CTRL_ENABLE_DSP | PWR_CTRL_ENABLE_BRAM | PWR_CTRL_ENABLE_URAM | PWR_CTRL_ENABLE_AIE | PWR_TOG_PAT_MASK);
        this->m_device->WritePowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);
    }
}

void PowerTest::StartPowerCu()
{
    uint read_data;
    for (uint cu_idx = 0; cu_idx < this->m_num_cu; cu_idx++)
    {
        this->m_device->ReadPowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);
        // Set null throttle, disable everything
        read_data &= ~(PWR_CTRL_ENABLE_REG | PWR_CTRL_ENABLE_DSP | PWR_CTRL_ENABLE_BRAM | PWR_CTRL_ENABLE_URAM | PWR_CTRL_ENABLE_AIE | PWR_TOG_PAT_MASK);

        // Enable each type of macro if authorized
        if (!(this->m_TC_Cfg.disable_reg[cu_idx]))
        {
            read_data |= PWR_CTRL_ENABLE_REG;
        }
        if (!(this->m_TC_Cfg.disable_dsp[cu_idx]))
        {
            read_data |= PWR_CTRL_ENABLE_DSP;
        }
        if (!(this->m_TC_Cfg.disable_bram[cu_idx]))
        {
            read_data |= PWR_CTRL_ENABLE_BRAM;
        }
        if (!(this->m_TC_Cfg.disable_uram[cu_idx]))
        {
            read_data |= PWR_CTRL_ENABLE_URAM;
        }
        if (!(this->m_TC_Cfg.disable_aie[cu_idx]))
        {
            read_data |= PWR_CTRL_ENABLE_AIE;
        }
        this->m_device->WritePowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);

        //this->m_device->ReadPowerCu(cu_idx, PWR_CTRL_REG_ADDR, read_data);
        //LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP[" + std::to_string(cu_idx) + "]: cfg register after 0x" + NumToStrHex<unsigned int>(read_data)});
    }
}

PowerTest::Meas_Temp_Pwr_t PowerTest::MeasPwr( const uint & num_sample, Sensor_t & sensor, const double & percent )
{
    // This function reports the average of num_sample different samples, waiting for 1 sec between each time it checks a new sample is available
    // Do not run with num_sample = 0
    auto    new_sensor_watchdog         = RST_NEW_SENSOR_WATCHDOG; // Init watchdog
    auto    new_power_watchdog          = RST_NEW_SENSOR_WATCHDOG; // Init watchdog
    auto    new_temperature_watchdog    = RST_NEW_SENSOR_WATCHDOG; // Init watchdog
    auto    result              = RST_MEAS_TEMP_PWR;
    double  pwr_sum             = 0.0;
    double  temp_sum            = 0.0;
    uint    time_cnt            = 0;
    uint    tmp_test_it_cnt     = 0;
    sensor          = this->m_devicemgt->GetSensorInstant();
    auto dump_cnt   = sensor.dump_cnt;  // don't care about dump_cnt, when result.power_sample_cnt = 0

    LogMessage(MSG_PWR_012, {std::to_string(num_sample)}); // Report number of readings

    while ((result.power_sample_cnt < num_sample) && (result.temp_sample_cnt < num_sample) && (!(this->m_abort)))
    {
        this->m_timer->WaitFor1sTick();
        sensor = this->m_devicemgt->GetSensorInstant();
        ResetWatchdog();
        time_cnt++;

        if (new_sensor_watchdog == 0)
        {
            LogMessage(MSG_PWR_059, {std::to_string(RST_NEW_SENSOR_WATCHDOG)});
            new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset watchdog when it reach 0
        }

        if (dump_cnt != sensor.dump_cnt) // Check new sensor is available from devicemgt
        {
            new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset watchdog when new sensor available
            dump_cnt            = sensor.dump_cnt;

            auto card_power_str = NOT_APPLICABLE;
            auto card_temperature_str = NOT_APPLICABLE;
            auto avg_card_power_str_str = NOT_APPLICABLE;
            auto avg_card_temperature_str = NOT_APPLICABLE;

            if (sensor.card_power_found) // Check new card power is available in sensor from devicemgt
            {
                new_power_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset card power watchdog when new card power available
                pwr_sum += sensor.card_power;
                card_power_str = Float_to_String<double>(sensor.card_power, 1) + " W";
                result.power_found = true;
                result.power_sample_cnt++;
                avg_card_power_str_str = Float_to_String<double>(pwr_sum / (double)(result.power_sample_cnt), 1) + " W";
            }
            else
            {
                new_power_watchdog--;
            }

            if (sensor.card_temperature_found) // Check new card temperature is available in sensor from devicemgt
            {
                new_temperature_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset card temperature watchdog when new card power available
                temp_sum += sensor.card_temperature;
                card_temperature_str = Float_to_String<double>(sensor.card_temperature, 0) + " C";
                result.temp_found = true;
                result.temp_sample_cnt++;
                avg_card_temperature_str = Float_to_String<double>(temp_sum / (double)(result.temp_sample_cnt), 0) + " C";
            }
            else
            {
                new_temperature_watchdog--;
            }

            std::string msg;
            msg += "MeasPwr: Power = "  + card_power_str;
            msg += " (average = "       + avg_card_power_str_str    + ", sample = " + std::to_string(result.power_sample_cnt);
            msg += "). Temperature = "  + card_temperature_str;
            msg += " (average = "       + avg_card_temperature_str  + ", sample = " + std::to_string(result.temp_sample_cnt);
            msg += "). Time = "         + std::to_string(time_cnt);

            LogMessage(MSG_DEBUG_TESTCASE, {msg});

            WriteToMeasurementFile(tmp_test_it_cnt, time_cnt, sensor, percent);
        }
        else
        {
            new_sensor_watchdog--;
        }

        if ((new_power_watchdog == 0) || (new_temperature_watchdog == 0))
        {
            LogMessage(MSG_PWR_061, {std::to_string(RST_NEW_SENSOR_WATCHDOG)});
            break;
        }
    }

    // Report status
    if (!(this->m_abort))
    {
        auto power_str = NOT_APPLICABLE;
        if (result.power_found)
        {
            result.power    = pwr_sum / (double)(result.power_sample_cnt);
            power_str       = Float_to_String<double>(result.power, 1);
        }

        auto temp_str = NOT_APPLICABLE;
        if (result.temp_found)
        {
            result.temp = std::round(temp_sum / (double)(result.temp_sample_cnt));
            temp_str    = Float_to_String<double>(result.temp, 0);
        }
        LogMessage(MSG_PWR_013, {"total power", power_str, Float_to_String<double>(percent,1), std::to_string(time_cnt), temp_str});
    }
    return result;
}

void PowerTest::WriteToMeasurementFile( const uint & test_it_cnt,const  uint & time, const Sensor_t & sensor, const double & toggle_rate )
{

    std::vector<std::string> str_vect;
    str_vect.emplace_back(std::to_string(this->m_timer->GetSecCnt()));
    str_vect.emplace_back(std::to_string(test_it_cnt));
    str_vect.emplace_back(std::to_string(time));
    str_vect.emplace_back(Float_to_String(toggle_rate,1));
    str_vect.emplace_back(std::to_string(sensor.dump_cnt));
    if (sensor.dump_valid)
    {
        str_vect.emplace_back("OK");
    }
    else
    {
        str_vect.emplace_back("KO");
    }
    for (const auto & source : sensor.fan_sources)
    {
        if (source.found && source.info.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.speed_rpm));
            str_vect.emplace_back(               source.info.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
    }
    for (const auto & source : sensor.thermal_sources)
    {
        if (source.found && source.info.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.temp_c));
            str_vect.emplace_back(               source.info.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
    }
    for (const auto & source : sensor.power_consumption_sources)
    {
        if (source.found)
        {
            str_vect.emplace_back(std::to_string(source.info.power_consumption_watts));
            str_vect.emplace_back(               source.info.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
    }
    for (const auto & source : sensor.power_rail_sources)
    {
        if (source.found && source.info.current.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.current.amps));
            str_vect.emplace_back(               source.info.current.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
        if (source.found && source.info.voltage.is_present)
        {
            str_vect.emplace_back(std::to_string(source.info.voltage.volts));
            str_vect.emplace_back(               source.info.voltage.sensor_status);
        }
        else
        {
            str_vect.emplace_back("0.0");
            str_vect.emplace_back("");
        }
        if (source.found && source.power_is_present)
        {
            str_vect.emplace_back(std::to_string(source.power));
        }
        else
        {
            str_vect.emplace_back("0.0");
        }
    }

    if (this->m_use_outputfile && !(this->m_abort) )
    {
        this->m_outputfile << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile.flush();

        (void)OpenRTOutputFile(this->m_RT_outputfile_name, this->m_RT_outputfile);  // Don't check if open worked..
        this->m_RT_outputfile << this->m_RT_outputfile_head + "\n" + StrVectToStr(str_vect, ",");
        this->m_RT_outputfile.flush();
        this->m_RT_outputfile.close();
    }
}

bool PowerTest::DisplayAIEStats()
{
    for (uint cu_idx = 0; cu_idx < this->m_device->GetNumPowerCu(); cu_idx++)
    {
        auto cu_name = this->m_device->GetPwrCuName(cu_idx);
        if (this->m_xbtest_sw_config->GetPwrUseAIE(cu_name) == 1)
        {
            auto pl_freq = (double)(this->m_xbtest_sw_config->GetClockFrequency(0));
            double pl_data_size_bytes;
            if (this->m_xbtest_sw_config->GetPwrAIEType(cu_name) == AIE_TYPE_AIE_ML)
            {
                pl_data_size_bytes = 4;
            }
            else
            {
                pl_data_size_bytes = 16;
            }

            // PL Stat for PL master (PL->AIE stream)
            std::vector<std::string> msg;
            if (StrMatchNoCase(this->m_xbtest_sw_config->GetPwrAIEControl(cu_name), AIE_CONTROL_PL))
            {
                uint cycle_cnt;
                uint m_rdy_cnt;
                uint m_xfer_cnt;

                this->m_device->ReadPowerCu(cu_idx, PWR_STAT_REG_CYCLE_CNT,  cycle_cnt);
                this->m_device->ReadPowerCu(cu_idx, PWR_STAT_REG_M_RDY_CNT,  m_rdy_cnt);
                this->m_device->ReadPowerCu(cu_idx, PWR_STAT_REG_M_XFER_CNT, m_xfer_cnt);

                auto m_trdy_rate  = (double)(m_rdy_cnt)  / (double)(cycle_cnt) * (double)(100.0);
                auto m_xfer_rate  = (double)(m_xfer_cnt) / (double)(cycle_cnt) * (double)(100.0);
                auto m_xfer_bw    = pl_data_size_bytes * (double)(pl_freq) * (double)(1000) * (double)(1000) * (double)(m_xfer_cnt) / (double)(cycle_cnt) / (double)(1024) / (double)(1024);

                msg.emplace_back("PL->AIE = " + Float_to_String<double>(m_xfer_bw, 1) + " MBps (ready = " + Float_to_String<double>(m_trdy_rate, 1) + "%, " + "xfer = " + Float_to_String<double>(m_xfer_rate, 1) + "%)");
            }
            // PL stat for PL slave (AIE->PL stream)
            if (StrMatchNoCase(this->m_xbtest_sw_config->GetPwrAIEStatus(cu_name), AIE_STATUS_PL))
            {
                uint cycle_cnt;
                uint s_xfer_cnt;

                this->m_device->ReadPowerCu(cu_idx, PWR_STAT_REG_CYCLE_CNT,  cycle_cnt);
                this->m_device->ReadPowerCu(cu_idx, PWR_STAT_REG_S_XFER_CNT, s_xfer_cnt);

                auto s_xfer_rate  = (double)(s_xfer_cnt) / (double)(cycle_cnt) * (double)(100.0);
                auto s_xfer_bw    = pl_data_size_bytes * (double)(pl_freq) * (double)(1000) * (double)(1000) * (double)(s_xfer_cnt) / (double)(cycle_cnt) / (double)(1024) / (double)(1024);

                msg.emplace_back("AIE->PL = " + Float_to_String<double>(s_xfer_bw, 1) + " MBps (xfer = " + Float_to_String<double>(s_xfer_rate, 1) + "%)");
            }
#ifdef USE_XRT
            // PS stat
            if (this->m_powertest_ps_xrt_run != nullptr)
            {
                ert_cmd_state state;
                if (GetPowertestPsState(state) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                msg.emplace_back("PS kernel state: "  + PsCUStateToStr(state));
            }
#endif
            if (!msg.empty())
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"xbtest HW IP[" + std::to_string(cu_idx) + "]: " + StrVectToStr(msg, " / ")});
            }
        }
    }
    return RET_SUCCESS;
}

int PowerTest::RunTest()
{
    uint test_it_cnt     = 1;
    auto test_failure    = RET_SUCCESS;
    auto test_it_failure = RET_SUCCESS;
    Sensor_t sensor;
    double percent = 0.0;

    if (!(this->m_abort))
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Number of test iterations: " + std::to_string(this->m_test_it_list.size())});
        this->m_testcase_queue_value.pending = this->m_test_it_list.size();
        PushTestcaseQueue();
    }

    // dump_cnt should be 0 at start up before the first measurement are received
    if (!(this->m_abort))
    {
        auto new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Init watchdog
        LogMessage(MSG_DEBUG_TESTCASE, {"Wait for first sensor measurement to be available"});
        do
        {
            if (new_sensor_watchdog == 0)
            {
                LogMessage(MSG_PWR_059, {std::to_string(RST_NEW_SENSOR_WATCHDOG)});
                new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset watchdog when it reach 0
            }

            sensor = this->m_devicemgt->GetSensorInstant();
            if (sensor.dump_cnt == 0)
            {
                this->m_timer->WaitFor1sTick();
                new_sensor_watchdog--;
            }
        } while ((sensor.dump_cnt == 0) && (!(this->m_abort)));
        LogMessage(MSG_DEBUG_TESTCASE, {"First measurement is available"});
    }

    LogMessage(MSG_PWR_025); //  "Start all Power xbtest HW IPs with a null toggle rate"
    StartPowerCu();
    this->m_current_percent = 0.0;
    if (!(this->m_abort))
    {
        WaitSecTick(1);
        auto base_temp_pwr          = MeasPwr(5, sensor, this->m_current_percent);
        auto base_card_power_str    = NOT_APPLICABLE;
        if (base_temp_pwr.power_found)
        {
            base_card_power_str = Float_to_String<double>(base_temp_pwr.power,1);
        }
        LogMessage(MSG_PWR_043, {base_card_power_str});
    }

    for (auto & test_it : this->m_test_it_list)
    {
        if (this->m_abort)
        {
            break;
        }

        this->m_testcase_queue_value.remaining_time = std::to_string(test_it.duration);
        this->m_testcase_queue_value.parameters = StrVectToTest(test_it.test_strvect);
        PushTestcaseQueue();

        test_it_failure = RET_SUCCESS;
        LogMessage(MSG_CMN_032, {std::to_string(test_it_cnt), StrVectToTest(test_it.test_strvect)}); // start test
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Extrapolated test sequence: " + StrVectToTest(test_it.test_strvect_dbg)});

        percent = (double)(test_it.toggle_rate);
        test_it_failure |= SetPwrThrottle(test_it_cnt, percent, test_it.duration, sensor);

        sensor = this->m_devicemgt->GetSensorInstant();
        auto dump_cnt = sensor.dump_cnt;
        auto new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Init watchdog

        // Loop until done or abort
        for (uint run_time = 0; (run_time < test_it.duration) && (test_it_failure == RET_SUCCESS) && (!(this->m_abort)); run_time++)
        {
            auto remain_time = test_it.duration - run_time;

#ifdef USE_XRT
            if (!(this->m_abort))
            {
                if ((remain_time > 0) && (remain_time % PS_KERNEL_RESTART_SEC == 0))
                {
                    test_it_failure |= StartPowertestPs(test_it.toggle_rate, PS_KERNEL_RESTART_SEC);
                }
            }
#endif
            this->m_timer->WaitFor1sTick();
            sensor = this->m_devicemgt->GetSensorInstant();
            ResetWatchdog();

            if (new_sensor_watchdog == 0)
            {
                LogMessage(MSG_PWR_059, {std::to_string(RST_NEW_SENSOR_WATCHDOG)});
                new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset watchdog when it reach 0
            }

            //LogMessage(MSG_DEBUG_TESTCASE, {"Sensors sample: " + std::to_string(sensor.dump_cnt)});
            if (sensor.dump_cnt != dump_cnt) // New sensor available
            {
                dump_cnt = sensor.dump_cnt;
                new_sensor_watchdog = RST_NEW_SENSOR_WATCHDOG; // Reset watchdog when new sensor available

                WriteToMeasurementFile(test_it_cnt, run_time, sensor, test_it.toggle_rate);
                // display status remaining time, temperature, power, toggle rate
                auto card_power_str = NOT_APPLICABLE;
                if (sensor.card_power_found)
                {
                    card_power_str = Float_to_String<double>(sensor.card_power, 1) + " W";
                }
                auto card_temperature_str = NOT_APPLICABLE;
                if (sensor.card_temperature_found)
                {
                    card_temperature_str = Float_to_String<double>(sensor.card_temperature, 0) + " C";
                }
                std::string msg;
                msg += "; Temperature: " + card_temperature_str;
                msg += "; Power: "       + card_power_str;
                msg += "; Toggle rate: " + Float_to_String<double>(percent,1) + " %";
                LogMessage(MSG_CMN_048, {std::to_string(remain_time), msg});
            }
            else
            {
                new_sensor_watchdog--;
                LogMessage(MSG_DEBUG_TESTCASE, {"\t" + std::to_string(remain_time) + " seconds remaining ; No new sensor reading received"});
            }

            test_it_failure |= DisplayAIEStats(); // We could display AIE stats less often?

            this->m_testcase_queue_value.remaining_time = std::to_string(remain_time);
            PushTestcaseQueue();
        }

#ifdef USE_XRT
        test_it_failure |= WaitPowertestPs();
#endif

        LogMessage(MSG_CMN_049); // test duration reached

        if (this->m_abort)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            // Set all remaining test result to aborted
            for (uint i = test_it_cnt - 1; i < this->m_test_sequence.size(); i++)
            {
                this->m_test_it_results[i] = TITR_ABORTED;
                this->m_testcase_queue_value.completed++;
                this->m_testcase_queue_value.pending--;
                this->m_testcase_queue_value.failed++;
            }
        }
        else if (test_it_failure == RET_FAILURE)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            this->m_test_it_results[test_it_cnt-1] = TITR_FAILED;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.failed++;
        }
        else
        {
            LogMessage(MSG_CMN_033_PASS, {std::to_string(test_it_cnt)});
            this->m_test_it_results[test_it_cnt-1] = TITR_PASSED;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.passed++;
        }
        if (this->m_testcase_queue_value.pending == 0)
        {
            this->m_testcase_queue_value.remaining_time    = NOT_APPLICABLE;
            this->m_testcase_queue_value.parameters        = NOT_APPLICABLE;
        }
        this->m_testcase_queue_value.errors   = this->m_errors;
        this->m_testcase_queue_value.warnings = this->m_warnings;
        PushTestcaseQueue();
        test_it_cnt++;
    }

    // Stop Power xbtest HW IP
    if (this->m_TC_Cfg.disable_toggle_ramp)
    {
        LogMessage(MSG_PWR_054, {"immediately"});
    }
    else
    {
        LogMessage(MSG_PWR_054, {"gradually"});
    }
    uint    tmp_test_it_cnt = 0;
    uint    tmp_time        = 0;
    double  tmp_percent     = 0.0;
    double  tmp_toggle_rate = 0.0;

    test_failure |= SetPwrThrottle(tmp_test_it_cnt, tmp_percent, 1, sensor);
    StopPowerCu();
#ifdef USE_XRT
    test_failure |= WaitPowertestPs();
#endif
    this->m_timer->WaitFor1sTick();
    sensor = this->m_devicemgt->GetSensorInstant();

    WriteToMeasurementFile(tmp_test_it_cnt, tmp_time, sensor, tmp_toggle_rate);

    // check for overall test failure
    if (this->m_abort)
    {
        return TEST_STATE_ABORT;
    }
    if (test_failure == RET_FAILURE)
    {
        return TEST_STATE_FAILURE;
    }
   return TEST_STATE_PASS;
}

bool PowerTest::StartCU()
{
    return RET_SUCCESS;
}

bool PowerTest::EnableWatchdogClkThrotDetection()
{
    return RET_SUCCESS;
}

bool PowerTest::StopCU()
{
    return RET_SUCCESS;
}

bool PowerTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_SUCCESS;
}

bool PowerTest::GetTestSequence()
{
    this->m_test_it_results.clear();
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_test_sequence = TestcaseParamCast<std::vector<Power_Test_Sequence_Parameters_t>>(it->second);
        for (uint ii = 0; ii < this->m_test_sequence.size(); ii++)
        {
            this->m_test_it_results.emplace_back(TITR_NOT_TESTED);
        }
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool PowerTest::ParseTestSequenceSettings( std::list<TestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    int  test_cnt = 0;

    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_sequence_dbg;

    for (const auto & test_seq_param : this->m_test_sequence)
    {
        if (this->m_abort)
        {
            break;
        }

        auto parse_it_failure = RET_SUCCESS;
        TestItConfig_t test_it_cfg;

        test_cnt++;

        test_it_cfg.test_strvect = {}; // Displays what user entered
        test_it_cfg.test_strvect_dbg = {}; // Displays with extrapolated parameters

        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_seq_param.duration.value, MIN_DURATION, MAX_DURATION);
            test_it_cfg.duration = test_seq_param.duration.value;
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckParam<uint>(TOGGLE_RATE_TEST_SEQ_MEMBER.name, test_seq_param.toggle_rate.value, MIN_TOGGLE_RATE, MAX_TOGGLE_RATE);
            test_it_cfg.toggle_rate = (int)(test_seq_param.toggle_rate.value);
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + TOGGLE_RATE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.toggle_rate.value));
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + TOGGLE_RATE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.toggle_rate.value));

        parse_failure |= parse_it_failure;

        if (parse_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_025,{std::to_string(test_cnt)});
            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + " contains erroneous parameters");
            parse_error_cnt ++;
            if (parse_error_cnt > MAX_NUM_PARSER_ERROR)
            {
                break;
            }
        }
        else
        {
            // the test is valid; add it to the test_list
            test_list.emplace_back(test_it_cfg);
            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + ". " + std::to_string(test_seq_param.num_param) + " parameters provided");
        }

        test_seq_strvect.emplace_back(StrVectToTest(test_it_cfg.test_strvect));
    }

    // display the entire test sequence
    LogMessage(MSG_DEBUG_TESTCASE, {"Extrapolated test sequence:"});
    for (const auto & test : test_sequence_dbg)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {test});
    }

    if (parse_failure == RET_SUCCESS)
    {
        LogMessage(MSG_CMN_047, {std::to_string(test_seq_strvect.size()), "[ " + StrVectToStr(test_seq_strvect, ", ") + " ]"});
        LogMessage(MSG_CMN_023);
    }
    else
    {
        LogMessage(MSG_CMN_024);
    }

    return parse_failure;
}

#ifdef USE_XRT
bool PowerTest::StartPowertestPs( const uint & toggle_rate, const uint & duration )
{
    if (this->m_powertest_ps_xrt_run == nullptr) // PS kernel not used
    {
        return RET_SUCCESS;
    }
    if (WaitPowertestPs() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    try
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Setting PS kernel argument 0 with toggle rate value: " + std::to_string(toggle_rate)});
        this->m_powertest_ps_xrt_run->set_arg(0, toggle_rate);
        LogMessage(MSG_DEBUG_TESTCASE, {"Setting PS kernel argument 1 with duration value: " + std::to_string(duration)});
        this->m_powertest_ps_xrt_run->set_arg(1, duration);

        LogMessage(MSG_DEBUG_TESTCASE, {"Starting PS kernel"});
        this->m_powertest_ps_xrt_run->start();
        this->m_powertest_ps_started = true;
    }
    catch (const std::exception& e)
    {
        LogMessage(MSG_ITF_155, {PS_KERNEL_NAME_POWER, e.what()});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool PowerTest::WaitPowertestPs()
{
    if (this->m_powertest_ps_xrt_run == nullptr) // PS kernel not used
    {
        return RET_SUCCESS;
    }
    if (!(this->m_powertest_ps_started))
    {
        return RET_SUCCESS;
    }
    try
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Waiting PS kernel to complete"});
        auto state = this->m_powertest_ps_xrt_run->wait(); // Blocking

        this->m_powertest_ps_started = false;

        /*
        // return_code() not defined in XRT 2.14

        auto return_code = this->m_powertest_ps_xrt_run->return_code();
        LogMessage(MSG_DEBUG_TESTCASE, {"PS kernel done (state = " + PsCUStateToStr(state) + ", return code = " + std::to_string(return_code) + ")"});

        if ((return_code != 0) || (state != ERT_CMD_STATE_COMPLETED))
        {
            LogMessage(MSG_ITF_156, {PS_KERNEL_NAME_POWER, std::to_string(state), std::to_string(return_code)});
            return RET_FAILURE;
        }
        */
        LogMessage(MSG_DEBUG_TESTCASE, {"PS kernel done (state = " + PsCUStateToStr(state) + ")"});

        if (state != ERT_CMD_STATE_COMPLETED)
        {
            LogMessage(MSG_ITF_156, {PS_KERNEL_NAME_POWER, std::to_string(state), "n/a"});
            return RET_FAILURE;
        }
    }
    catch (const std::exception& e)
    {
        LogMessage(MSG_ITF_157, {PS_KERNEL_NAME_POWER, e.what()});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool PowerTest::GetPowertestPsState( ert_cmd_state & state )
{
    try
    {
        state = this->m_powertest_ps_xrt_run->state();
    }
    catch (const std::exception& e)
    {
        LogMessage(MSG_ITF_157, {PS_KERNEL_NAME_POWER, e.what()});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

std::string PowerTest::PsCUStateToStr( const ert_cmd_state & state )
{
    switch (state)
    {
        case ERT_CMD_STATE_NEW:               return "NEW";         break;
        case ERT_CMD_STATE_QUEUED:            return "QUEUED";      break;
        case ERT_CMD_STATE_RUNNING:           return "RUNNING";     break;
        case ERT_CMD_STATE_COMPLETED:         return "COMPLETED";   break;
        case ERT_CMD_STATE_ERROR:             return "ERROR";       break;
        case ERT_CMD_STATE_ABORT:             return "ABORT";       break;
        case ERT_CMD_STATE_SUBMITTED:         return "SUBMITTED";   break;
        case ERT_CMD_STATE_TIMEOUT:           return "TIMEOUT";     break;
        case ERT_CMD_STATE_NORESPONSE:        return "NORESPONSE";  break;
        default:                              return "UNKNOWN";     break;
    }
}

#endif

} // namespace