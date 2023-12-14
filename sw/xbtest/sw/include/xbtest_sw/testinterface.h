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

#ifndef _TESTINTERFACE_H
#define _TESTINTERFACE_H

#ifdef USE_NO_DRIVER
#include <deviceinterface_nodrv.h>
#endif
#ifdef USE_AMI
#include <deviceinterface_ami.h>
#endif
#ifdef USE_XRT
#include "deviceinterface_ocl.h"
#endif

#include "timer.h"
#include "consolemgmtsw.h"

namespace xbtest
{

class TestInterface
{

public:
    using TestResult = enum TestResult {
        TR_NOT_SET,
        TR_PASSED,
        TR_FAILED,
        TR_ABORTED
    };

    using TestState = enum TestState {
        TS_NOT_SET,
        TS_PRE_SETUP,
        TS_RUNNING,
        TS_POST_TEARDOWN,
        TS_COMPLETE
    };

    Message_t                                           MSG_CMN_033_ERROR           = MSG_CMN_033; // Error by default in message definition
    Message_t                                           MSG_CMN_033_PASS            = MSG_CMN_033;
    Message_t                                           MSG_CMN_039_ERROR           = MSG_CMN_039; // Error by default in message definition
    Message_t                                           MSG_CMN_039_WARN            = MSG_CMN_039;
    TestResult                                          m_result                    = TR_NOT_SET;
    TestState                                           m_state                     = TS_NOT_SET;
    std::vector<TestIterationResult>                    m_test_it_results;
    std::map<uint, std::vector<TestIterationResult>>    m_test_it_lane_results;
    Logging *                                           m_log = nullptr;
    std::string                                         m_log_header;
    std::string                                         m_log_header_2;
    std::string                                         m_log_header_3;
    XbtestSwConfig *                                    m_xbtest_sw_config          = nullptr;
    std::atomic<bool>                                   m_abort;
    ConsoleMgmtSw *                                     m_console_mgmt              = nullptr;
    Timer *                                             m_timer                     = nullptr;
    DeviceInterface *                                   m_device                    = nullptr;
    std::string                                         m_queue_testcase;
    std::string                                         m_queue_thread;
    Testcase_Queue_Value_t                              m_testcase_queue_value      = RESET_TESTCASE_QUEUE_VALUE;
    uint                                                m_errors                    = 0;
    uint                                                m_warnings                  = 0;
    uint                                                m_watchdog_reset_rate       = WATCHDOG_RESET;
    uint                                                m_watchdog_reset_time       = 0;
    bool                                                m_watchdog_alarm            = false;
    bool                                                m_apclk_throttling_alarm    = false;
    bool                                                m_apclk2_throttling_alarm   = false;

    TestInterface( Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device );
    ~TestInterface();

    virtual bool PreSetup() = 0;
    virtual void Run() = 0;
    virtual void PostTeardown() = 0;
    virtual void Abort() = 0;
    virtual int  RunTest() = 0;
    virtual bool StartCU() = 0;
    virtual bool EnableWatchdogClkThrotDetection() = 0;
    virtual bool StopCU() = 0;
    virtual bool CheckWatchdogAndClkThrottlingAlarms() = 0;

    std::string                         ResultToString             ( const TestResult & result );
    TestResult                          GetResult();
    TestState                           GetTestState();
    std::vector<TestIterationResult>    GetItResult();
    std::vector<TestIterationResult>    GetItLaneResult            ( const uint & idx );
    bool                                CheckStringInSet           ( const std::string & param_name, const std::string & value, const std::set<std::string>    & test_set );
    bool                                CheckStringInVect          ( const std::string & param_name, const std::string & value, const std::vector<std::string> & test_vect );
    bool                                OpenOutputFile             ( const std::string & filename, std::ofstream & ofs );
    bool                                OpenRTOutputFile           ( const std::string & filename, std::ofstream & ofs );
    void                                LogMessage                 ( const Message_t & message, const std::vector<std::string> & arg_list );
    void                                LogMessage                 ( const Message_t & message );
    void                                LogMessageHeader2          ( const std::string & log_header_2, const Message_t & message, const std::vector<std::string> & arg_list );
    void                                LogMessageHeader2          ( const std::string & log_header_2, const Message_t & message );
    void                                IncMessageCount            ( const LogLevel & Level );
    bool                                CheckJsonParamNotDefined   ( const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_param, const std::string & justification);
    bool                                GetJsonParamArrayStr       ( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, std::vector<std::string> & param, const std::vector<std::string> & param_default );
    bool                                GetJsonParamAnyStr         ( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt,   Json_Parameters_t & json_param, std::string & param, const std::string & param_default );
    bool                                GetJsonParamAnyStr         ( const Json_Val_Def_t & json_val_def,                                   Json_Parameters_t & json_param, std::string & param, const std::string & param_default );
    bool                                GetJsonParamStr            ( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt,   Json_Parameters_t & json_param, const std::set<std::string> & supported_set, std::string & param, const std::string & param_default );
    bool                                GetJsonParamStr            ( const Json_Val_Def_t & json_val_def,                                   Json_Parameters_t & json_param, const std::set<std::string> & supported_set, std::string & param, const std::string & param_default );
    bool                                GetJsonParamBool           ( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt,   Json_Parameters_t & json_param, bool & param, const bool & param_default );
    bool                                GetJsonParamBool           ( const Json_Val_Def_t & json_val_def,                                   Json_Parameters_t & json_param, bool & param, const bool & param_default );
    void                                IsCheckDisabled            ( const bool & check_exists, const bool & default_check, bool & check, bool & check_disabled );
    void                                PushTestcaseQueue();
    void                                ResetWatchdog();
    void                                WaitSecTick                ( const uint & quantity );
    uint                                GetWatchdogConfigData      ( const uint & watchdog_duration);

    template<typename T> bool ConvertStringToNum( const std::string & param_name, const std::string & str_in, T & value )
    {
        if (ConvString2Num<T>(str_in, value) == RET_FAILURE)
        {
            LogMessage(MSG_CMN_016, {str_in, param_name});
            return RET_FAILURE;
        }
        return RET_SUCCESS;
    }

    template<typename T> bool CheckParam( const std::string & name, const T & request, const T & min, const T & max )
    {
        // check the value is within range
        if (request < min)
        {
            LogMessage(MSG_CMN_014, {std::to_string(request), name, std::to_string(min), std::to_string(max)});
            return RET_FAILURE;
        }
        if (request > max)
        {
            LogMessage(MSG_CMN_015, {std::to_string(request), name, std::to_string(min), std::to_string(max)});
            return RET_FAILURE;
        }
        return RET_SUCCESS;
    }

    template<typename T> bool CheckThresholdLoVsHi( const Json_Val_Def_t & json_val_def_min, const T & param_min, const Json_Val_Def_t & json_val_def_max, const T & param_max )
    {
        if (param_min > param_max)
        {
            LogMessage(MSG_CMN_017, {json_val_def_min.name, std::to_string(param_min), json_val_def_max.name, std::to_string(param_max)});
            return RET_FAILURE;
        }
        return RET_SUCCESS;
    }

    template<typename T> bool GetJsonParamNum( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, const T & param_min, const T & param_nom, const T & param_max, T & param  )
    {
        auto it = FindJsonParam(json_param, json_val_def);
        if (it != json_param.end())
        {
            // overwrite default value
            param = TestcaseParamCast<T>(it->second);
            if (CheckParam<T> (json_val_def.name + name_cmplt, param, param_min, param_max))
            {
                return RET_FAILURE;
            }
            LogMessage(MSG_CMN_013, {json_val_def.name + name_cmplt, std::to_string(param)});
        }
        else
        {
            // use default value
            param = param_nom;
            // check if the nominal value is in range
            if (CheckParam<T> (json_val_def.name + name_cmplt, param, param_min, param_max))
            {
                LogMessage(MSG_CMN_036, {std::to_string(param), json_val_def.name + name_cmplt, std::to_string(param_min), std::to_string(param_max)});
                return RET_FAILURE;
            }
            if (json_val_def.hidden == HIDDEN_FALSE)
            {
                LogMessage(MSG_CMN_012, {json_val_def.name + name_cmplt, std::to_string(param)});
            }
        }
        return RET_SUCCESS;
    }

    template<typename T> bool GetJsonParamNum( const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_param, const T & param_min, const T & param_nom, const T & param_max, T & param )
    {
        return GetJsonParamNum<T>(json_val_def, "", json_param, param_min, param_nom, param_max, param);
    }

};

} // namespace

#endif /* _TESTINTERFACE_H */
