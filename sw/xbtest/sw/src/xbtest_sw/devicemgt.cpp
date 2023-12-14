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

#include <thread>

#include "devicemgt.h"

namespace xbtest
{

DeviceMgt::DeviceMgt(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, const Device_Mgmt_Type_Parameters_t & task_parameters
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_log_header      = LOG_HEADER_DEVICE_MGT;
    this->m_task_parameters = task_parameters;

    this->m_sensor_instant  = this->m_xbtest_sw_config->GetSensorUsed();
    this->m_bdf             = this->m_xbtest_sw_config->GetBdf();
}

DeviceMgt::~DeviceMgt() = default;

bool DeviceMgt::PreSetup()
{
    this->m_state = TestState::TS_PRE_SETUP;
    LogMessage(MSG_CMN_004);

    auto ret = RET_SUCCESS;
    // parse task parameters

    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {
        // measurement output file
        auto it = FindJsonParam(this->m_task_parameters.param, USE_OUTPUT_FILE_MEMBER);
        auto use_output_file_exists = false;
        if (it != this->m_task_parameters.param.end())
        {
            use_output_file_exists = true;
        }

        bool use_output_file_param;
        this->m_overall_task_failure |= GetJsonParamBool(USE_OUTPUT_FILE_MEMBER, this->m_task_parameters.param, use_output_file_param, false);

        if (this->m_overall_task_failure == RET_SUCCESS)
        {
            this->m_use_outputfile = use_output_file_param || (!(this->m_task_parameters.sensor.empty()) && !use_output_file_exists && !use_output_file_param);
        }
        if ((this->m_overall_task_failure == RET_SUCCESS) && (this->m_use_outputfile))
        {
            this->m_overall_task_failure |= OpenOutputFile("sensor.csv", this->m_outputfile); // Create file
        }
        if ((this->m_overall_task_failure == RET_SUCCESS) && (this->m_use_outputfile))
        {
            this->m_RT_outputfile_name = "sensor.csv";
            this->m_overall_task_failure |= OpenRTOutputFile(this->m_RT_outputfile_name, this->m_RT_outputfile); // Create file
        }

        if ((this->m_overall_task_failure == RET_SUCCESS) && (this->m_use_outputfile))
        {
            // Write first row
            std::vector<std::string> first_line;
            first_line.emplace_back("time (s)");
            first_line.emplace_back("measurement ID");
            first_line.emplace_back("measurement valid");
            first_line.emplace_back("sensor reading duration (s)");
            for (const auto & source : this->m_sensor_instant.fan_sources)
            {
                first_line.emplace_back(source.display_name_value);
                first_line.emplace_back(source.display_name_status);
            }
            for (const auto & source : this->m_sensor_instant.thermal_sources)
            {
                first_line.emplace_back(source.display_name_value);
                first_line.emplace_back(source.display_name_status);
            }
            for (const auto & source : this->m_sensor_instant.power_consumption_sources)
            {
                first_line.emplace_back(source.display_name_value);
                first_line.emplace_back(source.display_name_status);
            }
            for (const auto & source : this->m_sensor_instant.power_rail_sources)
            {
                first_line.emplace_back(source.display_name_current_value);
                first_line.emplace_back(source.display_name_current_status);
                first_line.emplace_back(source.display_name_voltage_value);
                first_line.emplace_back(source.display_name_voltage_status);
                first_line.emplace_back(source.display_name_power);
            }
            this->m_outputfile << StrVectToStr(first_line, ",") << "\n";
            this->m_outputfile.flush();

            this->m_RT_outputfile_head = StrVectToStr(first_line, ",");
            this->m_RT_outputfile.close();
        }
    }

    this->m_overall_task_failure |= GetJsonParamNum<uint32_t>(DUMP_DURATION_ALARM_MEMBER, "",  this->m_task_parameters.param, DUMP_DURATION_ALARM_MIN, DUMP_DURATION_ALARM_NOM, DUMP_DURATION_ALARM_MAX, this->m_dump_duration_alarm);
    std::string key;
    for (uint i = 0; i < QTY_DUMP_DUR; i++)
    {
        key = "["+std::to_string(i)+", "+std::to_string(i+1)+"] sec";
        this->m_dump_durations[key] = 0;
    }
    key = "> " + std::to_string(QTY_DUMP_DUR) +" sec";
    this->m_dump_durations[key] = 0;
    key = "> " + std::to_string(this->m_dump_duration_alarm) +" sec alarm";
    this->m_dump_durations[key] = 0;



    this->m_overall_task_failure |= GetJsonParamNum<uint32_t>(WATCHDOG_DURATION_MEMBER, "",  this->m_task_parameters.param, WATCHDOG_DURATION_MIN, WATCHDOG_DURATION_NOM, WATCHDOG_DURATION_MAX, this->m_watchdog_duration);
    if (this->m_overall_task_failure == RET_FAILURE)
    {
        ret = RET_FAILURE;
        this->m_use_outputfile = false;
    }
    switch (this->m_watchdog_duration)  {
        case 0  :
                    LogMessage(MSG_MGT_012); // disable watchdog
                    break;

        case 16 :   LogMessage(MSG_MGT_011, { std::to_string(this->m_watchdog_duration) }); // default
                    break;

        case 32 :
        case 64 :
        case 128:
                    LogMessage(MSG_MGT_013, { std::to_string(this->m_watchdog_duration) });
                    break;
        default : //any other value are not supported
                    LogMessage(MSG_MGT_014, { std::to_string(this->m_watchdog_duration), "0 (disabled), 16 (default), 32, 64 and 128" });
                    ret = RET_FAILURE;
                    break;
    }

    return ret;
}

void DeviceMgt::Run()
{
    this->m_state     = TestState::TS_RUNNING;
    this->m_result    = TestResult::TR_FAILED;

    int task_state = 1;

    if (!(this->m_stop))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});

        std::thread threadMeasFile;
        if (this->m_use_outputfile)
        {
            threadMeasFile = std::thread(&DeviceMgt::RunMeasFile, this);
        }
        std::thread threadSensorQueue;
        if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
        {
            threadSensorQueue = std::thread(&DeviceMgt::RunPushSensorQueue, this);
        }
        task_state = RunTask();

        if (this->m_use_outputfile)
        {
            threadMeasFile.join();
        }
        if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
        {
            threadSensorQueue.join();
        }
    }

    if (task_state < 0)
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (task_state > 0)
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

void DeviceMgt::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void DeviceMgt::Abort()
{
    if (!(this->m_stop_req))
    {
        LogMessage(MSG_CMN_007);
        this->m_stop_req = true;
    }
}

Sensor_t DeviceMgt::GetSensorInstant()
{
    return this->m_sensor_instant;
}

uint DeviceMgt::GetWatchdogDuration()
{
    return this->m_watchdog_duration;
}

void DeviceMgt::CheckMeasurementStatus( const std::string & device_info_type, const bool & is_present, const bool & found,
    const std::string & display_name, const std::string & sensor_status )
{
    if (found && is_present)
    {
        // LogMessage(MSG_DEBUG_TESTCASE, {device_info_type + " / " + display_name + " / sensor_status: "  + sensor_status});
        if ((sensor_status != STATUS_SENSOR_OK) && (sensor_status != STATUS_SENSOR_OK_CACHED) && (sensor_status != STATUS_SENSOR_NOT_APPLICABLE))
        {
            if (this->invalid_sensor_cnt < INVALID_SENSOR_MAX_CNT)
            {
                this->invalid_sensor_cnt++;
                LogMessage(MSG_ITF_175, {device_info_type, sensor_status, display_name} );

                if (this->invalid_sensor_cnt == INVALID_SENSOR_MAX_CNT)
                {
                    LogMessage(MSG_ITF_145, {LogLevelToSeverity(MSG_ITF_175.log_level), MSG_ITF_175.id});
                }
            }
        }
    }
}
int DeviceMgt::CheckMeasurementValue( const std::string & device_info_type, const bool & is_present, const bool & found,
    const std::string & display_name, const std::string & unit_str, const double & value,
    Sensor_Limit_t & warning_threshold, Sensor_Limit_t & error_threshold, const Sensor_Limit_t & abort_threshold )
{
    auto check_meas_ret = CHECK_MEAS_RET_PASS;
    if (found && is_present)
    {
        // when value is out of abort range for a source, failure is displayed and xbtest aborts
        if ((abort_threshold.min_exists) && (value < abort_threshold.min))
        {
            // report value lower than min abort_threshold
            LogMessage(MSG_MGT_019, {device_info_type, display_name, std::to_string(value) + " " + unit_str, "lower", std::to_string(abort_threshold.min) + " " + unit_str});
            return CHECK_MEAS_RET_ABORT;
        }
        if ((abort_threshold.max_exists) && (value > abort_threshold.max))
        {
            // report value greater than max abort_threshold
            LogMessage(MSG_MGT_019, {device_info_type, display_name, std::to_string(value) + " " + unit_str, "greater", std::to_string(abort_threshold.max) + " " + unit_str});
            return CHECK_MEAS_RET_ABORT;
        }
        // when value is out of error range for a source, error is displayed
        if (!error_threshold.value_out_range)
        {
            if ((error_threshold.min_exists) && (value < error_threshold.min))
            {
                // report value lower than min error_threshold
                LogMessage(MSG_MGT_016, {device_info_type, display_name, std::to_string(value) + " " + unit_str, "lower", std::to_string(error_threshold.min) + " " + unit_str});
                error_threshold.value_out_range = true;
                check_meas_ret = CHECK_MEAS_RET_ERROR;
            }
            if ((error_threshold.max_exists) && (value > error_threshold.max))
            {
                // report value greater than max error_threshold
                LogMessage(MSG_MGT_016, {device_info_type, display_name, std::to_string(value) + " " + unit_str, "greater", std::to_string(error_threshold.max) + " " + unit_str});
                error_threshold.value_out_range = true;
                check_meas_ret = CHECK_MEAS_RET_ERROR;
            }
        }
        // when value is out of warning range for a source, critical warning is displayed, and it is not checked for the rest of the test
        if (!warning_threshold.value_out_range)
        {
            if ((warning_threshold.min_exists) && (value < warning_threshold.min))
            {
                // report value lower than min warning_threshold
                LogMessage(MSG_MGT_005, {device_info_type, display_name, std::to_string(value) + " " + unit_str, "lower", std::to_string(warning_threshold.min) + " " + unit_str});
                warning_threshold.value_out_range = true;
            }
            if ((warning_threshold.max_exists) && (value > warning_threshold.max))
            {
                // report value greater than max warning_threshold
                LogMessage(MSG_MGT_005, {device_info_type, display_name, std::to_string(value) + " " + unit_str, "greater", std::to_string(warning_threshold.max) + " " + unit_str});
                warning_threshold.value_out_range = true;
            }
        }
    }
    return check_meas_ret;
}

int DeviceMgt::CheckMeasurements()
{
    for (auto & source : this->m_sensor_instant.power_rail_sources)
    {
            CheckMeasurementStatus(
                source.type,
                source.info.voltage.is_present,
                source.found,
                source.display_name_voltage_value,
                source.info.voltage.sensor_status
            );
            CheckMeasurementStatus(
                source.type,
                source.info.current.is_present,
                source.found,
                source.display_name_current_value,
                source.info.current.sensor_status
            );
    }
    for (auto & source : this->m_sensor_instant.power_consumption_sources)
    {
        CheckMeasurementStatus(
            source.type,
            source.info.is_present,
            source.found,
            source.display_name_value,
            source.info.sensor_status
        );
    }

    for (auto & source : this->m_sensor_instant.thermal_sources)
    {
        CheckMeasurementStatus(
            source.type,
            source.info.is_present,
            source.found,
            source.display_name_value,
            source.info.sensor_status
        );
    }
    for (auto & source : this->m_sensor_instant.fan_sources)
    {
        CheckMeasurementStatus(
            source.type,
            source.info.is_present,
            source.found,
            source.display_name_value,
            source.info.sensor_status
        );
    }

    auto check_meas_ret = CHECK_MEAS_RET_PASS;

    for (auto & source : this->m_sensor_instant.power_rail_sources)
    {
        // If power is present, thresholds are for power
        // If power is not present, it means that only voltage or only current is available and thresholds are for available quantity
        if (source.power_is_present)
        {
            check_meas_ret = CheckMeasurementValue(
                source.type,
                source.power_is_present,
                source.found,
                source.display_name_power,
                source.unit_power,
                source.power,
                source.warning_threshold,
                source.error_threshold,
                source.abort_threshold
            );
        }
        else if (source.info.voltage.is_present)
        {
            check_meas_ret = CheckMeasurementValue(
                source.type,
                source.info.voltage.is_present,
                source.found,
                source.display_name_voltage_value,
                source.unit_voltage,
                source.info.voltage.volts,
                source.warning_threshold,
                source.error_threshold,
                source.abort_threshold
            );
        }
        else if (source.info.current.is_present)
        {
            check_meas_ret = CheckMeasurementValue(
                source.type,
                source.info.current.is_present,
                source.found,
                source.display_name_current_value,
                source.unit_current,
                source.info.current.amps,
                source.warning_threshold,
                source.error_threshold,
                source.abort_threshold
            );
        }

        if (check_meas_ret == CHECK_MEAS_RET_ABORT)
        {
            return CHECK_MEAS_RET_ABORT;
        }
    }
    for (auto & source : this->m_sensor_instant.power_consumption_sources)
    {
        check_meas_ret = CheckMeasurementValue(
            source.type,
            source.info.is_present,
            source.found,
            source.display_name_value,
            source.unit,
            source.info.power_consumption_watts,
            source.warning_threshold,
            source.error_threshold,
            source.abort_threshold
        );
        if (check_meas_ret == CHECK_MEAS_RET_ABORT)
        {
            return CHECK_MEAS_RET_ABORT;
        }
    }

    for (auto & source : this->m_sensor_instant.thermal_sources)
    {
        check_meas_ret = CheckMeasurementValue(
            source.type,
            source.info.is_present,
            source.found,
            source.display_name_value,
            source.unit,
            source.info.temp_c,
            source.warning_threshold,
            source.error_threshold,
            source.abort_threshold
        );
        if (check_meas_ret == CHECK_MEAS_RET_ABORT)
        {
            return CHECK_MEAS_RET_ABORT;
        }
    }
    for (auto & source : this->m_sensor_instant.fan_sources)
    {
        check_meas_ret = CheckMeasurementValue(
            source.type,
            source.info.is_present,
            source.found,
            source.display_name_value,
            source.unit,
            source.info.speed_rpm,
            source.warning_threshold,
            source.error_threshold,
            source.abort_threshold
        );
        if (check_meas_ret == CHECK_MEAS_RET_ABORT)
        {
            return CHECK_MEAS_RET_ABORT;
        }
    }
    return check_meas_ret;
}

bool DeviceMgt::GetSensors( Sensor_t & sensor )
{
    auto start_dump         = this->m_timer->GetSecCnt();
    auto electrical_info    = RST_ELECTRICAL_INFO;
    auto thermal_info       = RST_THERMAL_INFO;
    auto mechanical_info    = RST_MECHANICAL_INFO;
    auto device_info_ret    = this->m_device_info_parser->GetSensorsInfo(electrical_info, thermal_info, mechanical_info);
    auto new_sensor         = sensor;
    new_sensor.card_power_found = false;
    new_sensor.card_temperature_found = false;

    new_sensor.dump_valid = false;
    if (device_info_ret == RET_SUCCESS)
    {
        new_sensor.dump_valid = true;
    }

    for (auto & source : new_sensor.power_rail_sources)
    {
        source.found                    = false;
        source.info.current.is_present  = false;
        source.info.voltage.is_present  = false;
        source.power_is_present         = false;
        for (const auto & info : electrical_info.power_rails)
        {
            if (StrMatchNoCase(source.info.id, info.id))
            {
                source.info = info;

                if (source.info.voltage.is_present && source.info.current.is_present)
                {
                    source.power            = source.info.voltage.volts * source.info.current.amps; // V * A to W
                    source.power_is_present = true;

                }
                if (source.card_power)
                {
                    new_sensor.card_power_found = true;
                    new_sensor.card_power = 0;
                    if (source.power_is_present)
                    {
                        new_sensor.card_power = source.power;
                    }
                }
                source.found = true;
            }
        }
        if (!source.found && source.node_absence_msg)
        {
            source.node_absence_msg = false;
            LogMessage(MSG_MGT_022, {source.type, source.info.id});
        }
    }
    for (auto & source : new_sensor.power_consumption_sources)
    {
        source.found = false;
        for (const auto & info : electrical_info.power_consumptions)
        {
            if (StrMatchNoCase(source.info.id, info.id))
            {
                source.info    = info;
                source.found   = true;
                if (source.card_power)
                {
                    new_sensor.card_power_found = true;
                    new_sensor.card_power = info.power_consumption_watts;
                }
            }
        }
        if (!source.found && source.node_absence_msg)
        {
            source.node_absence_msg = false;
            LogMessage(MSG_MGT_022, {source.type, source.info.id});
        }
    }
    for (auto & source : new_sensor.thermal_sources)
    {
        source.found            = false;
        source.info.is_present  = false;
        for (const auto & info : thermal_info.thermals)
        {
            if (StrMatchNoCase(source.info.location_id, info.location_id))
            {
                source.info  = info;
                source.found = true;
                if (source.card_temperature)
                {
                    new_sensor.card_temperature_found = true;
                    new_sensor.card_temperature = info.temp_c;
                }
            }
        }
        if (!source.found && source.node_absence_msg)
        {
            source.node_absence_msg = false;
            LogMessage(MSG_MGT_022, {source.type, source.info.location_id});
        }
    }
    for (auto & source : new_sensor.fan_sources)
    {
        source.found            = false;
        source.info.is_present  = false;
        for (const auto & info : mechanical_info.fans)
        {
            if (StrMatchNoCase(source.info.location_id, info.location_id))
            {
                source.info = info;
                source.found = true;
            }
        }
        if (!source.found && source.node_absence_msg)
        {
            source.node_absence_msg = false;
            LogMessage(MSG_MGT_022, {source.type, source.info.location_id});
        }
    }

#if defined(USE_AMI) || defined(USE_XRT)
    // Default
    if (!new_sensor.card_power_found)
    {
        for (const auto & info : electrical_info.power_consumptions)
        {
            if (StrMatchNoCase(DEFAULT_CARD_POWER_ID, info.id))
            {
                new_sensor.card_power       = info.power_consumption_watts;
                new_sensor.card_power_found = true;
            }
        }
    }
    if (!new_sensor.card_temperature_found)
    {
        for (const auto & info : thermal_info.thermals)
        {
            if (StrMatchNoCase(DEFAULT_CARD_TEMPERATURE_ID, info.location_id))
            {
                new_sensor.card_temperature         = info.temp_c;
                new_sensor.card_temperature_found   = true;
            }
        }
    }
#endif

    new_sensor.dump_cnt++;

    auto end_dump           = this->m_timer->GetSecCnt();
    new_sensor.dump_time    = end_dump-start_dump;

    sensor = new_sensor;
    return RET_SUCCESS;
}

int DeviceMgt::RunTask()
{
    auto task_ret       = TEST_STATE_PASS;
    auto dump_alarm_msg = true;
    // Create xrt::device only once as xbtest-sw terminates on sigbus
    this->m_device_info_parser = this->m_xbtest_sw_config->GetDeviceInfoParser();

    while (!(this->m_stop) && (this->m_overall_task_failure == RET_SUCCESS))
    {
        WaitSecTick(1);

        // Get sensor value using XRT C++ API and execution duration
        auto dump_ret = RET_FAILURE; // RET_FAILURE when GetSensors failed or not executed
        if (!(this->m_stop) && (this->m_overall_task_failure == RET_SUCCESS))
        {
            dump_ret = GetSensors(this->m_sensor_instant);
        }

        //LogMessage(MSG_DEBUG_TESTCASE, {"\t dump_time: "  + std::to_string(this->m_sensor_instant.dump_time)});
        this->m_overall_task_failure |= dump_ret;

        // Check execution duration
        if (dump_ret == RET_SUCCESS)
        {
            std::string key = "> " + std::to_string(this->m_dump_duration_alarm) + " sec alarm";
            if (this->m_sensor_instant.dump_time >= this->m_dump_duration_alarm)
            {
                this->m_dump_durations[key]++;
                if (dump_alarm_msg)
                {
                    dump_alarm_msg = false; // only display the message once
                    LogMessage(MSG_MGT_010, {std::to_string(this->m_dump_duration_alarm)});
                }
            }

            // if this->m_sensor_instant.dump_time = 0 it means that the the dump took less than 1 second
            // if this->m_sensor_instant.dump_time = 1 it means that dump duration is [1,2[ sec
            // if this->m_sensor_instant.dump_time = 2 it means that dump duration is [2,3[ sec
            // ...
            //accumulate the duration of GetSensors per interval of 1 second
            if (this->m_sensor_instant.dump_time >= QTY_DUMP_DUR)
            {
                key = "> " + std::to_string(QTY_DUMP_DUR) +" sec";
            }
            else
            {
                key = "["+std::to_string(this->m_sensor_instant.dump_time)+", "+std::to_string(this->m_sensor_instant.dump_time+1)+"] sec";
            }
            this->m_dump_durations[key]++;

            // Perform some check on the sensor values
            auto check_meas_ret = CheckMeasurements();
            if (check_meas_ret == CHECK_MEAS_RET_ABORT)
            {
                this->m_overall_task_failure =  RET_FAILURE; // Abort other threads
            }
            else if (check_meas_ret == CHECK_MEAS_RET_ERROR)
            {
                task_ret = TEST_STATE_FAILURE;
            }
        }

        // First dump executed, stop if requested
        if (this->m_stop_req)
        {
            this->m_stop = true;
        }
    }

    auto api_total      = this->m_sensor_instant.dump_cnt;
    auto api_fail_total = this->m_device_info_parser->GetApiFailTotal();
    auto percent        = (double)(100) * (double)(api_fail_total) / (double)(api_total);

    if (percent > PERCENT_API_FAIL_ERROR_LIMIT)
    {
        LogMessage(MSG_MGT_023, {Float_to_String<double>(percent,3), Float_to_String<double>(PERCENT_API_FAIL_ERROR_LIMIT,0), std::to_string(api_fail_total), std::to_string(api_total) });
    }
    else if (percent > PERCENT_API_FAIL_WARNING_LIMIT)
    {
        LogMessage(MSG_MGT_024, {Float_to_String<double>(percent,3), Float_to_String<double>(PERCENT_API_FAIL_WARNING_LIMIT,0), std::to_string(api_fail_total), std::to_string(api_total) });
    }

    // Sensor reading duration
    LogMessage(MSG_MGT_008);
    std::string key;
    for (uint i = 0; i < QTY_DUMP_DUR; i++)
    {
        key = "["+std::to_string(i)+", "+std::to_string(i+1)+"] sec";
        LogMessage(MSG_MGT_009, {"\t* "+key+": " + std::to_string(this->m_dump_durations[key])});
    }
    key = "> " + std::to_string(QTY_DUMP_DUR) +" sec";
    LogMessage(MSG_MGT_009, {"\t* "+key+": " + std::to_string(this->m_dump_durations[key])});
    key = "> " + std::to_string(this->m_dump_duration_alarm) +" sec alarm";
    LogMessage(MSG_MGT_009, {"\t* "+key+": " + std::to_string(this->m_dump_durations[key])});


    // As this test is run as background task, the abort signal is not considered as a failure
    // An internal task failure is considered as an abort, so that all other tests will be aborted as a consequence
    if ((task_ret == TEST_STATE_PASS) && (this->m_overall_task_failure == RET_SUCCESS) && (this->m_stop))
    {
        return TEST_STATE_PASS;
    }
    if (this->m_overall_task_failure == RET_FAILURE)
    {
        return TEST_STATE_ABORT;
    }
    return TEST_STATE_FAILURE;
}

void DeviceMgt::RunPushSensorQueue()
{
    Sensor_Queue_Value_t sensor_queue_value;
    auto queue_allocated = false;
    while (!(this->m_stop) && (this->m_overall_task_failure == RET_SUCCESS))
    {
        WaitSecTick(1);
        // wait until the first value is available
        if (this->m_sensor_instant.dump_cnt > 0)
        {
            sensor_queue_value.dump_valid           = this->m_sensor_instant.dump_valid;
            sensor_queue_value.dump_cnt             = this->m_sensor_instant.dump_cnt;
            sensor_queue_value.temperature          = this->m_sensor_instant.card_temperature;
            sensor_queue_value.power                = this->m_sensor_instant.card_power;
            sensor_queue_value.temperature_found    = this->m_sensor_instant.card_temperature_found;
            sensor_queue_value.power_found          = this->m_sensor_instant.card_power_found;
            // Send sensor values to console
            if (!queue_allocated) // this is to make sure the power start being displayed in the console only after the first measure is obtained
            {
                this->m_console_mgmt->AllocateSensorQueue(sensor_queue_value);
                queue_allocated = true;
            }
            else
            {
                this->m_console_mgmt->PushSensorQueue(sensor_queue_value);
            }
        }
    }
}

void DeviceMgt::RunMeasFile()
{
    uint64_t time = 0;
    while (!(this->m_stop) && (this->m_overall_task_failure == RET_SUCCESS))
    {
        WaitSecTick(1);
        // wait until the first value is available
        if (this->m_sensor_instant.dump_cnt > 0)
        {
            WriteToMeasurementFile(time, this->m_sensor_instant);
        }
        time++;
    }
}

void DeviceMgt::WriteToMeasurementFile( const uint64_t & time, const Sensor_t & sensor )
{
    std::vector<std::string> str_vect;
    str_vect.emplace_back(std::to_string(time));
    str_vect.emplace_back(std::to_string(sensor.dump_cnt));
    if (sensor.dump_valid)
    {
        str_vect.emplace_back("OK");
    }
    else
    {
        str_vect.emplace_back("KO");
    }
    str_vect.emplace_back(std::to_string(sensor.dump_time));
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
    this->m_outputfile << StrVectToStr(str_vect, ",") << "\n";
    this->m_outputfile.flush();

    (void)OpenRTOutputFile(this->m_RT_outputfile_name, this->m_RT_outputfile); // Don't check if open worked..
    this->m_RT_outputfile << this->m_RT_outputfile_head + "\n" + StrVectToStr(str_vect, ",");
    this->m_RT_outputfile.flush();
    this->m_RT_outputfile.close();
}

bool DeviceMgt::StartCU()
{
    return RET_FAILURE;
}
bool DeviceMgt::EnableWatchdogClkThrotDetection()
{
    return RET_FAILURE;
}
bool DeviceMgt::StopCU()
{
    return RET_FAILURE;
}
bool DeviceMgt::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_FAILURE;
}
int DeviceMgt::RunTest()
{
    return TEST_STATE_FAILURE;
}

} // namespace