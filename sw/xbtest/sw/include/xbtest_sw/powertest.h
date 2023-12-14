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

#ifndef _POWERTEST_H
#define _POWERTEST_H

#include <list>

#include "testinterface.h"
#include "devicemgt.h"

namespace xbtest
{

class PowerTest : public TestInterface
{

public:
    PowerTest(
        Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
        const Power_Parameters_t & test_parameters
    );
    ~PowerTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

private:
    using PowerTestcaseCfg_t = struct PowerTestcaseCfg_t {
        bool disable_toggle_ramp;
        std::map<uint,bool> disable_reg;
        std::map<uint,bool> disable_dsp;
        std::map<uint,bool> disable_bram;
        std::map<uint,bool> disable_uram;
        std::map<uint,bool> disable_aie;
    };

    using TestItConfig_t = struct TestItConfig_t {
        uint                        duration;
        int                         toggle_rate;
        std::vector<std::string>    test_strvect;
        std::vector<std::string>    test_strvect_dbg;
    };

    using Meas_Temp_Pwr_t = struct Meas_Temp_Pwr_t {
        double      temp;
        double      power;
        bool        temp_found;
        bool        power_found;
        uint        temp_sample_cnt;
        uint        power_sample_cnt;
    };
    const Meas_Temp_Pwr_t RST_MEAS_TEMP_PWR = {
        0.0,    // double   temp;
        0.0,    // double   power;
        false,  // bool     temp_found;
        false,  // bool     power_found;
        0,      // uint     temp_sample_cnt;
        0       // uint     power_sample_cnt;
    };

    // enable/disable the various macro in order to dissipate power
    const uint   PWR_CTRL_REG_ADDR           = 0x20;
    const uint   PWR_STAT_REG_CYCLE_CNT      = 0x21;
    const uint   PWR_STAT_REG_M_RDY_CNT      = 0x22;
    const uint   PWR_STAT_REG_M_XFER_CNT     = 0x23;
    const uint   PWR_STAT_REG_S_XFER_CNT     = 0x24;
    const int    QTY_THROTTLE_STEP           = 512; // 9 bits
    const uint   PWR_TOG_PAT_MASK            = (2*QTY_THROTTLE_STEP)-1;
    const uint   PWR_CTRL_ENABLE_REG         = (0x1 << 16);
    const uint   PWR_CTRL_ENABLE_DSP         = (0x1 << 17);
    const uint   PWR_CTRL_ENABLE_BRAM        = (0x1 << 18);
    const uint   PWR_CTRL_ENABLE_URAM        = (0x1 << 19);
    const uint   PWR_CTRL_ENABLE_AIE         = (0x1 << 20);
    const uint   MIN_TOGGLE_RATE             = 0;
    const uint   MAX_TOGGLE_RATE             = 100;
    const uint   RST_NEW_SENSOR_WATCHDOG     = 10; // in sec
    const uint   PS_KERNEL_RESTART_SEC       = 10; // in sec
    const double        POWER_GRADIENT              = 10.0; // maximum toggle rate step per second supported

    DeviceMgt *                                     m_devicemgt = nullptr;
    Power_Parameters_t                              m_test_parameters;
    std::vector<Power_Test_Sequence_Parameters_t>   m_test_sequence;
    uint                                            m_num_cu;
    PowerTestcaseCfg_t                              m_TC_Cfg;
    std::string                                     m_outputfile_name;
    bool                                            m_use_outputfile = false;
    std::ofstream                                   m_outputfile;
    std::ofstream                                   m_RT_outputfile;
    std::string                                     m_RT_outputfile_name;
    std::string                                     m_RT_outputfile_head;
    std::list<TestItConfig_t>                       m_test_it_list;
    double                                          m_current_percent = 0.0;

    uint                                            m_powertest_ps_started = false;
#ifdef USE_XRT
    xrt::run *                                      m_powertest_ps_xrt_run = nullptr;
#endif

    int             Percent2Throttle            ( const double & percent );
    bool            SetPwrThrottle              ( const uint & test_it_cnt, const double & percent, const uint & duration, Sensor_t & sensor );
    void            CfgThrottle                 ( const int & throttle );
    void            StopPowerCu();
    void            StartPowerCu();
    Meas_Temp_Pwr_t MeasPwr                     ( const uint & num_sample, Sensor_t & sensor, const double & percent );
    void            WriteToMeasurementFile      ( const uint & test_it_cnt, const uint & time, const Sensor_t & sensor, const double & toggle_rate );
    bool            DisplayAIEStats();
    int             RunTest() override;
    bool            StartCU() override;
    bool            EnableWatchdogClkThrotDetection() override;
    bool            StopCU() override;
    bool            CheckWatchdogAndClkThrottlingAlarms() override;
    bool            GetTestSequence();
    bool            ParseTestSequenceSettings   ( std::list<TestItConfig_t> & test_list );

#ifdef USE_XRT
    bool            StartPowertestPs            ( const uint & toggle_rate, const uint & duration = 1 );
    bool            WaitPowertestPs();
    bool            GetPowertestPsState         ( ert_cmd_state & state );
    std::string     PsCUStateToStr              ( const ert_cmd_state & state );
#endif
};

} // namespace

#endif /* _POWERTEST_H */
