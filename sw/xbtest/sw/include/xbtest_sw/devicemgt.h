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

#ifndef _DEVICEMGT_H
#define _DEVICEMGT_H

#include "testinterface.h"

#ifdef USE_NO_DRIVER
#include "deviceinfoparser_nodrv.h"
#endif
#ifdef USE_AMI
#include "deviceinfoparser_ami.h"
#endif
#ifdef USE_XRT
#include "deviceinfoparser_xrt.h"
#endif

namespace xbtest
{

class DeviceMgt : public TestInterface
{

public:
    DeviceMgt( Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, const Device_Mgmt_Type_Parameters_t & task_parameters );
    ~DeviceMgt();

    bool        PreSetup() override;
    void        Run() override;
    void        PostTeardown() override;
    void        Abort() override;
    Sensor_t    GetSensorInstant();
    uint        GetWatchdogDuration();

private:
    const uint       QTY_DUMP_DUR            = 5;
    const uint64_t   DUMP_DURATION_ALARM     = 5;
    const int        CHECK_MEAS_RET_ABORT    = -1;
    const int        CHECK_MEAS_RET_ERROR    = 1;
    const int        CHECK_MEAS_RET_PASS     = 0;
    const uint32_t   WATCHDOG_DURATION_MIN   = 0;
    const uint32_t   WATCHDOG_DURATION_NOM   = 16;
    const uint32_t   WATCHDOG_DURATION_MAX   = 128;

    const uint DUMP_DURATION_ALARM_MIN = 1;
    const uint DUMP_DURATION_ALARM_NOM = 10;
    const uint DUMP_DURATION_ALARM_MAX = MAX_UINT_VAL;

    Device_Mgmt_Type_Parameters_t   m_task_parameters;
    std::string                     m_bdf;
    Sensor_t                        m_sensor_instant        = RST_SENSOR;
    bool                            m_stop_req              = false; // request from main to stop
    bool                            m_stop                  = false; // task received stop req and first measurement is available used instead of m_abort
    DeviceInfoParser *              m_device_info_parser    = nullptr;
    bool                            m_use_outputfile        = false;
    std::ofstream                   m_outputfile;
    std::ofstream                   m_RT_outputfile;
    std::string                     m_RT_outputfile_name;
    std::string                     m_RT_outputfile_head;
    bool                            m_overall_task_failure  = RET_SUCCESS;
    uint32_t                        m_watchdog_duration     = WATCHDOG_DURATION_NOM;
    uint                            m_dump_duration_alarm;
    std::map<std::string, uint64_t> m_dump_durations;

    uint32_t invalid_sensor_cnt = 0; // Global counter of invalid sensor status. Increments if any of sensor monitored by xbtest is invalid
	const uint32_t INVALID_SENSOR_MAX_CNT  = 5; // Disable warning after 5 messages

    void CheckMeasurementStatus                ( const std::string & device_info_type, const bool & is_present, const bool & found,
        const std::string & display_name, const std::string & sensor_status );
    int     CheckMeasurementValue        ( const std::string & device_info_type, const bool & is_present, const bool & found,
        const std::string & display_name, const std::string & unit_str, const double & value,
        Sensor_Limit_t & warning_threshold, Sensor_Limit_t & error_threshold, const Sensor_Limit_t &  abort_threshold );
    int     CheckMeasurements();
    bool    GetSensors              ( Sensor_t & sensor ); // currently CMC reads (via the SC) sensor values every 100ms
    int     RunTask();
    void    RunPushSensorQueue();
    void    RunMeasFile();
    void    WriteToMeasurementFile  ( const uint64_t & time, const Sensor_t & sensor );
    bool    StartCU() override; // not used
    bool    EnableWatchdogClkThrotDetection() override; // not used
    bool    StopCU() override; // not used
    bool    CheckWatchdogAndClkThrottlingAlarms() override; // not used
    int     RunTest() override; // not used

};

} // namespace

#endif /* _DEVICEMGT_H */
