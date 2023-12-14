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

#include "commandlineparserbase.h"

namespace xbtest
{

CommandLineParserBase::CommandLineParserBase( Logging * log, std::atomic<bool> * abort, int argc, char ** argv )
{
    this->m_log     = log;
    this->m_abort   = abort;
    this->m_argc    = argc;
    this->m_argv    = argv;
}

CommandLineParserBase::~CommandLineParserBase() = default;

void CommandLineParserBase::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, message, arg_list);
}

void CommandLineParserBase::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, message);
}

bool CommandLineParserBase::ParseOptionArgumentStr( Xbtest_CL_option_Str_t & cl_option, const char & test_option, const std::string & optarg_str )
{
    return ParseOptionArgumentStr(cl_option, test_option, optarg_str, true);
}

bool CommandLineParserBase::ParseOptionArgumentStr( Xbtest_CL_option_Str_t & cl_option, const char & test_option, const std::string & optarg_str, const bool & unique )
{
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentStr: test_option = " + std::string(1, test_option)});
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentStr: optarg_str  = " + optarg_str});
    if (cl_option.exists && unique)
    {
        LogMessage(MSG_GEN_001, {std::string(1, test_option)});
        return RET_FAILURE;
    }

    cl_option.value  = optarg_str;
    cl_option.exists = true;
    LogMessage(MSG_GEN_013, {OptionCharToName(test_option), cl_option.value});
    return RET_SUCCESS;
}

bool CommandLineParserBase::ParseOptionArgumentStr( std::string & cl_option, const char & test_option, const std::string & optarg_str )
{
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentStr: test_option = " + std::string(1, test_option)});
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentStr: optarg_str  = " + optarg_str});

    cl_option  = optarg_str;

    LogMessage(MSG_GEN_013, {OptionCharToName(test_option), optarg_str});
    return RET_SUCCESS;
}

bool CommandLineParserBase::ParseOptionArgumentInt( Xbtest_CL_option_Int_t & cl_option, const char & test_option, const std::string & optarg_str)
{
    return ParseOptionArgumentInt(cl_option, test_option, optarg_str, true);
}

bool CommandLineParserBase::ParseOptionArgumentInt( Xbtest_CL_option_Int_t & cl_option, const char & test_option, const std::string & optarg_str, const bool & unique )
{
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentInt: test_option = " + std::string(1, test_option)});
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentInt: optarg_str  = " + optarg_str});
    if (cl_option.exists && unique)
    {
        LogMessage(MSG_GEN_001, {std::string(1, test_option)});
        return RET_FAILURE;
    }

    if (ConvString2Num<int>(optarg_str, cl_option.value) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    cl_option.exists = true;
    LogMessage(MSG_GEN_013, {OptionCharToName(test_option), std::to_string(cl_option.value)});
    return RET_SUCCESS;
}

bool CommandLineParserBase::ParseOptionArgumentInt( int & cl_option, const char & test_option, const std::string & optarg_str )
{
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentInt: test_option = " + std::string(1, test_option)});
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentInt: optarg_str  = " + optarg_str});
    if (ConvString2Num<int>(optarg_str, cl_option) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    LogMessage(MSG_GEN_013, {OptionCharToName(test_option), std::to_string(cl_option)});
    return RET_SUCCESS;
}

bool CommandLineParserBase::ParseOptionArgumentUint( Xbtest_CL_option_Uint_t & cl_option, const char & test_option, const std::string & optarg_str )
{
    return ParseOptionArgumentUint(cl_option, test_option, optarg_str, true);
}

bool CommandLineParserBase::ParseOptionArgumentUint( Xbtest_CL_option_Uint_t & cl_option, const char & test_option, const std::string & optarg_str, const bool & unique )
{
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentUint: test_option = " + std::string(1, test_option)});
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentUint: optarg_str  = " + optarg_str});
    if (cl_option.exists && unique)
    {
        LogMessage(MSG_GEN_001, {std::string(1, test_option)});
        return RET_FAILURE;
    }
    if (ConvString2Num<uint>(optarg_str, cl_option.value) == RET_FAILURE)
    {
        LogMessage(MSG_GEN_014, {optarg_str, OptionCharToName(test_option), "Expected unsigned integer"});
        return RET_FAILURE;
    }
    cl_option.exists = true;
    LogMessage(MSG_GEN_013, {OptionCharToName(test_option), std::to_string(cl_option.value)});
    return RET_SUCCESS;
}

bool CommandLineParserBase::ParseOptionArgumentUint( uint & cl_option, const char & test_option, const std::string & optarg_str )
{
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentUint: test_option = " + std::string(1, test_option)});
    LogMessage(MSG_DEBUG_PARSING, {"ParseOptionArgumentUint: optarg_str  = " + optarg_str});
    if (ConvString2Num<uint>(optarg_str, cl_option) == RET_FAILURE)
    {
        LogMessage(MSG_GEN_014, {optarg_str, OptionCharToName(test_option), "Expected unsigned integer"});
        return RET_FAILURE;
    }
    LogMessage(MSG_GEN_013, {OptionCharToName(test_option), std::to_string(cl_option)});
    return RET_SUCCESS;
}

bool CommandLineParserBase::CheckFileExists( const std::string & name, const std::string & file )
{
    if (!FileExists(file))
    {
        LogMessage(MSG_ITF_003, {name, file});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool CommandLineParserBase::SetVerbosity( const int & verbosity )
{
    // Check verbosity
    if (verbosity < MIN_VERBOSITY)
    {
        LogMessage(MSG_CMN_014, {std::to_string(verbosity), VERBOSITY, std::to_string(MIN_VERBOSITY), std::to_string(MAX_VERBOSITY)});
        return RET_FAILURE;
    }
    if (verbosity > MAX_VERBOSITY)
    {
        LogMessage(MSG_CMN_015, {std::to_string(verbosity), VERBOSITY, std::to_string(MIN_VERBOSITY), std::to_string(MAX_VERBOSITY)});
        return RET_FAILURE;
    }
    auto log_level = static_cast<LogLevel>(verbosity);
    this->m_log->SetVerbosity(log_level);
    return RET_SUCCESS;
}

void CommandLineParserBase::PrintCommandLineOptionNotSupported( const char & test_option )
{
    LogMessage(MSG_GEN_005, {std::string(1, test_option)});
}

void CommandLineParserBase::PrintMessage( const std::string & msg_id )
{
    Message_t message;
    if (this->m_log->GetMessage(msg_id, message))
    {
        if (message.state == "used")
        {
            // Print message not found
            LogMessage(MSG_GEN_033, {"The message \"" +  msg_id + "\" is:"});
            LogMessage(MSG_GEN_033, {"\t- Severity      : " +  LogLevelToSeverity(message.log_level)});
            LogMessage(MSG_GEN_033, {"\t- Content       : " +  message.msg});
            LogMessage(MSG_GEN_033, {"\t- Details       : " +  message.detail});
            if (!message.resolution.empty())
            {
                LogMessage(MSG_GEN_033, {"\t- Resolution    : " +  message.resolution});
            }
        }
        else
        {
            // Print message not found
            LogMessage(MSG_GEN_033, {"The message \"" +  msg_id + "\" is obsolete in this xbtest version"});
        }
    }
    else
    {
        // Print message not found
        LogMessage(MSG_GEN_033, {"The message \"" +  msg_id + "\" does not exist in this xbtest version"});
        LogMessage(MSG_GEN_033, {"Check the provided message ID, or check the selected xbtest version (see -d option)"});
    }
}

bool CommandLineParserBase::CheckTargetP2P( const std::string & source_bdf, const Xbtest_CL_option_Str_t & p2p_target_bdf, const Xbtest_CL_option_Str_t & p2p_nvme_path )
{
    // Check p2p target card and P2P NVMe path are not combined
    if ((p2p_target_bdf.exists) && (p2p_nvme_path.exists))
    {
        LogMessage(MSG_GEN_081);
        return RET_FAILURE;
    }
    if (p2p_target_bdf.exists)
    {
        // Check P2P target card BDF is valid
        if (CheckCardBDF(p2p_target_bdf.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // Check source card BDF and P2P target card BDF are different
        if (CompareBDF(source_bdf, p2p_target_bdf.value))
        {
            LogMessage(MSG_GEN_074, {source_bdf, p2p_target_bdf.value});
            return RET_FAILURE;
        }
    }
    if (p2p_nvme_path.exists)
    {
        // Check file exists and wr/rd permissions also for device node
        if (CheckFileExists(P2P_NVME_PATH, p2p_nvme_path.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (!IsFileWritable(p2p_nvme_path.value))
        {
            LogMessage(MSG_GEN_080, {"write", p2p_nvme_path.value});
            return RET_FAILURE;
        }
        if (!IsFileReadable(p2p_nvme_path.value))
        {
            LogMessage(MSG_GEN_080, {"read", p2p_nvme_path.value});
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

#ifdef USE_AMI
bool CommandLineParserBase::GetAMIBuildVersion()
{
    if (ami_get_api_version(&(this->m_ami_version)) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_182, {std::string(ami_get_last_error())});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}
#endif

} // namespace
