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

#include "testinterface.h"

namespace xbtest
{

TestInterface::TestInterface( Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device )
{
    this->m_state  =  TestState::TS_NOT_SET;
    this->m_result = TestResult::TR_PASSED;

    this->m_log               = log;
    this->m_abort             = false;
    this->m_xbtest_sw_config  = xbtest_sw_config;

    this->m_console_mgmt    = console_mgmt;
    this->m_timer           = timer;
    this->m_device          = device;

    MSG_CMN_033_PASS.log_level = LOG_PASS;
    MSG_CMN_039_WARN.log_level = LOG_WARN;
}

TestInterface::~TestInterface () = default;

std::string TestInterface::ResultToString( const TestResult & result )
{
    switch (result)
    {
        case TR_PASSED:     return "PASSED";    break;
        case TR_FAILED:     return "FAILED";    break;
        case TR_ABORTED:    return "ABORTED";   break;
        default:            return "NOTSET";    break;
    }
}

TestInterface::TestResult TestInterface::GetResult()
{
    return this->m_result;
}

TestInterface::TestState TestInterface::GetTestState()
{
    return this->m_state;
}

std::vector<TestIterationResult> TestInterface::GetItResult()
{
    return this->m_test_it_results;
}

std::vector<TestIterationResult> TestInterface::GetItLaneResult( const uint & idx )
{
    return this->m_test_it_lane_results[idx];
}

bool TestInterface::CheckStringInSet ( const std::string & param_name, const std::string & value, const std::set<std::string> & test_set )
{
    if (!FindStringInSet(value, test_set))
    {
        LogMessage(MSG_CMN_001, {value, param_name, StrSetToStr(QuoteStrSet(test_set), ", ")});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool TestInterface::CheckStringInVect ( const std::string & param_name, const std::string & value, const std::vector<std::string> & test_vect )
{
    if (!FindStringInVect(value, test_vect))
    {
        auto test_vect_q = QuoteStrVect(test_vect);
        LogMessage(MSG_CMN_001, {value, param_name, StrVectToStr(test_vect_q, ", ")});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool TestInterface::OpenOutputFile( const std::string & filename, std::ofstream & ofs )
{
    auto fullname = this->m_xbtest_sw_config->GetLogDir() + "/" + filename;

    LogMessage(MSG_CMN_020, {fullname});
    if (FileExists(fullname))
    {
        if (this->m_xbtest_sw_config->GetCommandLineForce())
        {
            LogMessage(MSG_CMN_019, {fullname}); // warning
        }
        else
        {
            LogMessage(MSG_CMN_053, {fullname});
            return RET_FAILURE;
        }
    }
    if (!OpenFile(fullname, ofs))
    {
        LogMessage(MSG_CMN_018, {fullname});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool TestInterface::OpenRTOutputFile( const std::string & filename, std::ofstream & ofs )
{
    auto fullname = this->m_xbtest_sw_config->GetLogDir() + "/RT/" + filename;

    if (!FileExists(fullname))
    {
        LogMessage(MSG_CMN_020, {fullname});
    }
    if (!OpenFile(fullname, ofs))
    {
        LogMessage(MSG_CMN_018, {fullname});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}


void TestInterface::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, this->m_log_header_2, this->m_log_header_3, message, arg_list);
    IncMessageCount(message.log_level);
}

void TestInterface::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, this->m_log_header_2, this->m_log_header_3, message);
    IncMessageCount(message.log_level);
}

void TestInterface::LogMessageHeader2 ( const std::string & log_header_2, const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, log_header_2, this->m_log_header_3, message, arg_list);
    IncMessageCount(message.log_level);
}

void TestInterface::LogMessageHeader2 ( const std::string & log_header_2, const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, log_header_2, this->m_log_header_3, message);
    IncMessageCount(message.log_level);
}

void TestInterface::IncMessageCount ( const LogLevel & Level )
{
    if ((Level == LOG_FAILURE) || (Level == LOG_ERROR))
    {
        this->m_errors++;
    }
    if ((Level == LOG_CRIT_WARN) || (Level == LOG_WARN))
    {
        this->m_warnings++;
    }
}

bool TestInterface::CheckJsonParamNotDefined( const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_param, const std::string & justification)
{
    auto ret = RET_SUCCESS;
    auto it = FindJsonParam(json_param, json_val_def);
    if (it != json_param.end())
    {
        LogMessage(MSG_CMN_050, {json_val_def.name, justification});
        ret = RET_FAILURE;
    }
    return ret;
}

bool TestInterface::GetJsonParamArrayStr( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, std::vector<std::string> & param, const std::vector<std::string> & param_default )
{
    auto it = FindJsonParam(json_param, json_val_def);
    if (it != json_param.end())
    {
        param = TestcaseParamCast<std::vector<std::string>>(it->second);
        LogMessage(MSG_CMN_013, {json_val_def.name + name_cmplt, StrVectToStr(param, ", ")});
    }
    else
    {
        // use default value
        param = param_default;
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_CMN_012, {json_val_def.name + name_cmplt, StrVectToStr(param, ", ")});
        }
    }
    return RET_SUCCESS;
}

bool TestInterface::GetJsonParamAnyStr( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, std::string & param, const std::string & param_default )
{
    auto it = FindJsonParam(json_param, json_val_def);
    if (it != json_param.end())
    {
        param = TestcaseParamCast<std::string>(it->second);
        LogMessage(MSG_CMN_013, {json_val_def.name + name_cmplt, param});
    }
    else
    {
        // use default value
        param = param_default;
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_CMN_012, {json_val_def.name + name_cmplt, param});
        }
    }
    return RET_SUCCESS;
}

bool TestInterface::GetJsonParamAnyStr( const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_param, std::string & param, const std::string & param_default )
{
    return GetJsonParamAnyStr(json_val_def, "", json_param, param, param_default);
}

bool TestInterface::GetJsonParamStr( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, const std::set<std::string> & supported_set, std::string & param, const std::string & param_default )
{
    GetJsonParamAnyStr(json_val_def, name_cmplt, json_param, param, param_default);
    return CheckStringInSet(json_val_def.name + name_cmplt, param, supported_set);
}

bool TestInterface::GetJsonParamStr( const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_param, const std::set<std::string> & supported_set, std::string & param, const std::string & param_default )
{
    return GetJsonParamStr(json_val_def, "", json_param, supported_set, param, param_default);
}

bool TestInterface::GetJsonParamBool( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, bool & param, const bool & param_default )
{
    auto it = FindJsonParam(json_param, json_val_def);
    if (it != json_param.end())
    {
        // overwrite default value
        param = TestcaseParamCast<bool>(it->second);
        // no need to check if the value is correct (true or false) as the boolean check as been done inside the input parser
        LogMessage(MSG_CMN_013, {json_val_def.name + name_cmplt, BoolToStr(param)});
    }
    else
    {
        // use default value
        param = param_default;
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_CMN_012, {json_val_def.name + name_cmplt, BoolToStr(param)});
        }
    }
    return RET_SUCCESS;
}

bool TestInterface::GetJsonParamBool( const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_param, bool & param, const bool & param_default )
{
    return GetJsonParamBool(json_val_def, "", json_param, param, param_default);
}

void TestInterface::IsCheckDisabled( const bool & check_exists, const bool & default_check, bool & check, bool & check_disabled )
{
    if (!check_exists && default_check)
    {
        check          = false;
        check_disabled = true;
    }
}

void TestInterface::PushTestcaseQueue()
{
    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        // update message count
        this->m_testcase_queue_value.errors   = this->m_errors;
        this->m_testcase_queue_value.warnings = this->m_warnings;
        this->m_console_mgmt->PushTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }
}

void TestInterface::ResetWatchdog()
{
    // this function needs to be called after every WaitFor1sTick() to check the last time the watchdog has been reset
    // create a new reset if the last reset was long time ago
    auto tmp_sw_sec_cnt = this->m_timer->GetSecCnt();
    if ((tmp_sw_sec_cnt - this->m_watchdog_reset_time) > this->m_watchdog_reset_rate)
    {
        // watchdog is reset by any read or write to the BI or the Common_Status
        // so simply check the status of watchdog and clock throttling detection mechanism
        CheckWatchdogAndClkThrottlingAlarms();
        this->m_watchdog_reset_time = tmp_sw_sec_cnt;
    }
}

void TestInterface::WaitSecTick( const uint & quantity )
{
    for (uint i = 0; (i < quantity) && (!(this->m_abort)); i++)
    {
        this->m_timer->WaitFor1sTick();
        ResetWatchdog();
    }
}
uint TestInterface::GetWatchdogConfigData( const uint & watchdog_duration )
{
    // enable clk throttling monitoring + clear any alarms
    auto reg_data = CMN_WATCHDOG_ALARM | CMN_THROTTLE_MON_ENABLE | CMN_THROTTLE_ALARM_RST;
    // enable and configure watchdog if necessary
    switch (watchdog_duration)  {
        case 0  :
                    // do not enable watchdog
                    break;
        case 32 :
                    reg_data |= CMN_WATCHDOG_EN | CMN_WATCHDOG_32SEC;
                    break;
        case 64 :
                    reg_data |= CMN_WATCHDOG_EN | CMN_WATCHDOG_64SEC;
                    break;
        case 128:
                    reg_data |= CMN_WATCHDOG_EN | CMN_WATCHDOG_128SEC;
                    break;
        default : // 16
                    reg_data |= CMN_WATCHDOG_EN | CMN_WATCHDOG_16SEC;
                    break;
    }
    return reg_data;
}

} // namespace
