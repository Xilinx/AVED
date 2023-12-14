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
#include <sys/utsname.h>

#include "xbtestcommonconfig.h"

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

XbtestCommonConfig::XbtestCommonConfig( Logging * log, std::atomic<bool> * abort )
{
    this->m_log                 = log;
    this->m_abort               = abort;
    this->m_command_line_parsed = false;

    ParseSystemInfo();
    SetStartSessionTime();
}

XbtestCommonConfig::~XbtestCommonConfig() = default;

bool XbtestCommonConfig::ScanHostSetup()
{
    return ScanLibraries();
}

bool XbtestCommonConfig::CheckNumLibValid()
{
    if (this->m_num_lib_valid == 0)
    {
        LogMessage(MSG_ITF_108);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

std::map<std::string, Card_Config_t> XbtestCommonConfig::GetCardConfigMap()
{
    return this->m_xbtest_common_config.card_config_map;
}

Card_Config_t XbtestCommonConfig::GetCardConfig( const std::string & bdf )
{
    auto it = this->m_xbtest_common_config.card_config_map.find(bdf);
    return it->second;
}

void XbtestCommonConfig::ParseSystemInfo()
{
    this->m_xbtest_common_config.system.username  = "UNKNOWN";
    this->m_xbtest_common_config.system.sysname   = "UNKNOWN";
    this->m_xbtest_common_config.system.nodename  = "UNKNOWN";
    this->m_xbtest_common_config.system.release   = "UNKNOWN";
    this->m_xbtest_common_config.system.version   = "UNKNOWN";
    this->m_xbtest_common_config.system.machine   = "UNKNOWN";

    char username[LOGIN_NAME_MAX];
    if (getlogin_r(username, LOGIN_NAME_MAX) == 0)
    {
        this->m_xbtest_common_config.system.username = std::string(username);
    }
       utsname tmp_utsname;
    if (uname(&tmp_utsname) == 0)
    {
        this->m_xbtest_common_config.system.sysname   = std::string(tmp_utsname.sysname);
        this->m_xbtest_common_config.system.nodename  = std::string(tmp_utsname.nodename);
        this->m_xbtest_common_config.system.release   = std::string(tmp_utsname.release);
        this->m_xbtest_common_config.system.version   = std::string(tmp_utsname.version);
        this->m_xbtest_common_config.system.machine   = std::string(tmp_utsname.machine);
    }
}

void XbtestCommonConfig::SetStartSessionTime()
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    this->m_xbtest_common_config.system.start_time    = FormatTime(now, "%a %b %d %H:%M:%S %Y %Z"); // Sun Jul 26 17:26:07 2020 BST
    this->m_xbtest_common_config.system.start_time2   = FormatTime(now, "%Y-%m-%d_%H-%M-%S");       // 2020-07-26_17-26-07
    SetCommonLogDir("./xbtest_logs/" + this->m_xbtest_common_config.system.start_time2);                           // ./xbtest_logs/2020-07-26_17-26-07
}

void XbtestCommonConfig::SetEndSessionTime()
{
    this->m_xbtest_common_config.system.end_time = GetCurrentTime();
}

std::string XbtestCommonConfig::GetSessionStartTime()
{
    return this->m_xbtest_common_config.system.start_time;
}
std::string XbtestCommonConfig::GetSessionEndTime()
{
    return this->m_xbtest_common_config.system.end_time;
}
std::string XbtestCommonConfig::GetSystemUsername()
{
    return this->m_xbtest_common_config.system.username;
}
std::string XbtestCommonConfig::GetSystemSysname()
{
    return this->m_xbtest_common_config.system.sysname;
}
std::string XbtestCommonConfig::GetSystemNodename()
{
    return this->m_xbtest_common_config.system.nodename;
}
std::string XbtestCommonConfig::GetSystemRelease()
{
    return this->m_xbtest_common_config.system.release;
}
std::string XbtestCommonConfig::GetSystemVersion()
{
    return this->m_xbtest_common_config.system.version;
}
std::string XbtestCommonConfig::GetSystemMachine()
{
    return this->m_xbtest_common_config.system.machine;
}

uint XbtestCommonConfig::ParseCommandLine( int argc, char** argv )
{
    LogMessage(MSG_DEBUG_PARSING, {"Parsing command line"});

    CommandLineParserCommon command_line_parser(this->m_log, this->m_abort, argc, argv, this->m_amd_xbtest_libs, this->m_xbtest_common_config.map_lib_config);
    auto command_line_ret = command_line_parser.ParseCommandLine();
    if (command_line_ret != COMMAND_LINE_RET_SUCCESS)
    {
        return command_line_ret;
    }
    this->m_xbtest_common_config.command_line = command_line_parser.GetCommandLine();

    if (this->m_xbtest_common_config.command_line.log_dir.exists)
    {
        SetCommonLogDir(this->m_xbtest_common_config.command_line.log_dir.value);
    }
    else
    {
        SetCommonLogDir("./xbtest_logs/" + this->m_xbtest_common_config.system.start_time2);
    }
    this->m_command_line_parsed = true;
    return COMMAND_LINE_RET_SUCCESS;
}

bool XbtestCommonConfig::ParseCardJson()
{
    LogMessage(MSG_DEBUG_PARSING, {"Parsing card configuration JSON file"});

    this->m_xbtest_common_config.card_config_map = this->m_xbtest_common_config.command_line.card_config_map;
    if (this->m_xbtest_common_config.command_line.cardjson_mode)
    {
        LogMessage(MSG_GEN_016, {"Using card configuration JSON file: " + this->m_xbtest_common_config.command_line.card_json.value});
        CardJsonParser card_json_parser(this->m_log, this->m_xbtest_common_config.command_line.card_json.value, this->m_abort);
        if (card_json_parser.Parse() == RET_FAILURE)
        {
            LogMessage(MSG_ITF_111);
            return RET_FAILURE;
        }
        this->m_xbtest_common_config.card_config_map = card_json_parser.GetCardConfigMap();
    }
    return RET_SUCCESS;
}

void XbtestCommonConfig::FormatUuid( std::string & uuid )
{
    // Format UUID: lower case without dashes
    std::transform(uuid.begin(), uuid.end(), uuid.begin(), tolower);
    auto interface_uuid_split = split(uuid, '-');
    uuid = StrVectToStr(interface_uuid_split, "");
}

bool XbtestCommonConfig::GetActualLibConfig( const std::string & bdf, std::string & uuid, std::string & dsa_name, HW_Lib_Config_t & lib )
{
    LogMessageBdf(MSG_DEBUG_PARSING, bdf, {"Getting actual xbtest HW design configuration"});
    // Get dsa name and uuid
    auto device_info_parser = DeviceInfoParser(this->m_log, bdf, this->m_abort);

#ifdef USE_AMI
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (device_info_parser.GetLogicUuid(uuid) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            uuid = RST_LOGIC_UUID;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (uuid == RST_LOGIC_UUID)
    {
        LogMessageBdf(MSG_GEN_032, bdf, {"uuid"}); // Error board not supported
        return RET_FAILURE;
    }
#endif
#ifdef USE_XRT
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (device_info_parser.GetInterfaceUuid(uuid) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            uuid = RST_INTERFACE_UUID;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (uuid == RST_INTERFACE_UUID)
    {
        LogMessageBdf(MSG_GEN_032, bdf, {"uuid"}); // Error board not supported
        return RET_FAILURE;
    }
#endif
    LogMessageBdf(MSG_DEBUG_SETUP, bdf, {"found uuid: " + uuid});
    FormatUuid(uuid);

    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (device_info_parser.GetNameInfo(dsa_name) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            dsa_name = RST_NAME_INFO;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessageBdf(MSG_DEBUG_SETUP, bdf, {"found device name: " + dsa_name});

    // Get HW version for card found based on target identifier (UUID)
    auto lib_it = this->m_xbtest_common_config.map_lib_config.find(uuid);
    if (lib_it == this->m_xbtest_common_config.map_lib_config.end())
    {
        LogMessageBdf(MSG_GEN_029, bdf, {"UUID", uuid});
        return RET_FAILURE;
    }
    lib = lib_it->second;

    // Report if HW design in use is internal release
    if (lib.build_info.xbtest.internal_release)
    {
        LogMessageBdf(MSG_ITF_137, bdf);
    }

    return RET_SUCCESS;
}

bool XbtestCommonConfig::GenerateActualCardConfig()
{
    LogMessage(MSG_DEBUG_SETUP, {"Generating the actual card configuations"});

    // Check if multiple cards were selected
    this->m_xbtest_common_config.multicard_mode  = false;
    if (this->m_xbtest_common_config.card_config_map.size() > 1)
    {
        this->m_xbtest_common_config.multicard_mode = true;
        LogMessage(MSG_DEBUG_SETUP, {"Multiple cards were provided"});
    }
    // Check if multiple test will be run: one or more tests on multiple cards, or multiple tests on a single card
    this->m_xbtest_common_config.multitest_mode = false;
    if (this->m_xbtest_common_config.multicard_mode)
    {
        this->m_xbtest_common_config.multitest_mode = true;
    }
    else
    {
        for (const auto & cfg : this->m_xbtest_common_config.card_config_map)
        {
            auto card_config = cfg.second;
            if (card_config.tests.size() > 1)
            {
                this->m_xbtest_common_config.multitest_mode = true;
                LogMessage(MSG_DEBUG_SETUP, {"Multiple tests were provided on a single-card card"});
                break;
            }
        }
    }
    if (*(this->m_abort))
    {
        return RET_FAILURE;
    }

    // By default log directory created at common level if multiple test are run
    SetUseCommonLogDir(this->m_xbtest_common_config.multitest_mode & !(this->m_xbtest_common_config.command_line.disable_log));

    if (GetUseCommonLogDir())
    {
        LogMessage(MSG_GEN_016, {"Using common log directory: " + GetCommonLogDir()});
    }

    if (this->m_log->GenerateLogDir(GetCommonLogDir(), !(GetUseCommonLogDir()), GetCommandLineForce()) == RET_FAILURE)
    {
        return COMMAND_LINE_RET_FAILURE;
    }

    // Determine full configuration
    for (auto & cfg : this->m_xbtest_common_config.card_config_map)
    {
        auto bdf         = cfg.first;
        auto card_config = cfg.second;

        LogMessageBdf(MSG_GEN_016, bdf, {"Getting card configuration"});

        // Get xbtest HW design of selected card
        if (GetActualLibConfig(bdf, card_config.interface_uuid, card_config.dsa_name, card_config.lib) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        // Define xbtest-sw command line arguments when P2P target selected
        std::vector<std::string> p2p_args;

#ifdef USE_XRT
        if (this->m_xbtest_common_config.command_line.p2p_target_bdf.exists)
        {
            // Get xbtest HW design of P2P target card
            std::string     target_interface_uuid;
            std::string     target_dsa_name;
            HW_Lib_Config_t target_lib;
            if (GetActualLibConfig(this->m_xbtest_common_config.command_line.p2p_target_bdf.value, target_interface_uuid, target_dsa_name, target_lib) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            p2p_args.emplace_back("-T");
            p2p_args.emplace_back(this->m_xbtest_common_config.command_line.p2p_target_bdf.value);
            p2p_args.emplace_back("-X");
            p2p_args.emplace_back(target_lib.xclbin);
            p2p_args.emplace_back("-E");
            p2p_args.emplace_back(target_lib.xbtest_pfm_def);
        }
        if (this->m_xbtest_common_config.command_line.p2p_nvme_path.exists)
        {
            p2p_args.emplace_back("-N");
            p2p_args.emplace_back(this->m_xbtest_common_config.command_line.p2p_nvme_path.value);
        }
#endif
        // Check pre-canned test provided for each card exists and set the path of its test JSON file
        std::vector<Test_Config_t> tests2;
        for (const auto & test : card_config.tests)
        {
            auto test2 = test;
            uint test_idx = tests2.size() + 1;
            // Set test JSON file
            if (test.is_pre_canned)
            {
                if (!FindStringInVect(test.pre_canned, card_config.lib.pre_canned_test_list))
                {
                    LogMessageBdf(MSG_GEN_030, bdf, {test.pre_canned, StrVectToStr(card_config.lib.pre_canned_test_list, ", ")});
                    return RET_FAILURE;
                }
                test2.is_test_json  = true;
                test2.test_json     = card_config.lib.lib_path + "/xbtest/test/" + test.pre_canned + ".json";
            }
            else if (test.is_test_json)
            {
                // Check provided test json file exists
                if (CheckFileExists(TEST_JSON + " provided in command line or in " + CARD_JSON_FILE, test.test_json, true) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
            }

            // Set the test name used as ID
            test2.test_id_name = pad(std::to_string(test_idx), '0', 3, PAD_ON_LEFT); // 001
            if (test2.is_test_json)
            {
                auto path_split = SplitNoEmpty(test2.test_json, '/');
                auto file       = path_split.back();
                std::string name;
                std::string ext;
                GetNameExt(file, name, ext);
                test2.test_id_name += "_" + name; // 001_mytest
            }

            // Set test log directory
            test2.use_log_dir = false;
            if (GetUseCommonLogDir())
            {
                test2.use_log_dir   = true;
                test2.log_dir       = this->m_xbtest_common_config.common_log_dir + "/" + FormatName(bdf, "-") + "/" + test2.test_id_name; // ./xbtest_logs/2020-07-26_17-26-07/card_0000-86-00-1/<test_id_name>
            }
            else if (this->m_xbtest_common_config.command_line.log_dir.exists && !(this->m_xbtest_common_config.command_line.disable_log))
            {
                test2.use_log_dir = true;
                test2.log_dir     = this->m_xbtest_common_config.command_line.log_dir.value;
            }

            // Merge args from xbtest common command line and card JSON
            if (this->m_xbtest_common_config.command_line.cardjson_mode && !(this->m_xbtest_common_config.command_line.args.empty()))
            {
                if (InsertOptions(bdf, test_idx, test2.args, this->m_xbtest_common_config.command_line.args) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
            }

            // Set args from common to SW command line
            std::vector<std::string> test_args_from_common;
            test_args_from_common.emplace_back("-d");
            test_args_from_common.emplace_back(bdf);
            if (test2.is_test_json)
            {
                test_args_from_common.emplace_back("-j");
                test_args_from_common.emplace_back(test2.test_json);
            }
            if (test2.use_log_dir)
            {
                test_args_from_common.emplace_back("-l");
                test_args_from_common.emplace_back(test2.log_dir);
            }

#ifdef USE_AMI
            test_args_from_common.emplace_back("-i");
            test_args_from_common.emplace_back(card_config.lib.pdi);
#endif
#ifdef USE_XRT
            test_args_from_common.emplace_back("-x");
            test_args_from_common.emplace_back(card_config.lib.xclbin);
#endif

            test_args_from_common.emplace_back("-e");
            test_args_from_common.emplace_back(card_config.lib.xbtest_pfm_def);

            test2.sys_cmd.clear();

            if (InsertOptions(bdf, test_idx, test2.sys_cmd, test_args_from_common) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            if (!p2p_args.empty())
            {
                if (InsertOptions(bdf, test_idx, test2.sys_cmd, p2p_args) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
            }
            // Set user args to SW command line
            if (InsertOptions(bdf, test_idx, test2.sys_cmd, test2.args) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test2.sys_cmd.insert(test2.sys_cmd.begin(), "xbtest");
            tests2.emplace_back(test2);
        }
        card_config.tests = tests2;

        // Done getting card config
        this->m_xbtest_common_config.card_config_map.at(bdf) = card_config;
    }
    return RET_SUCCESS;
}

void XbtestCommonConfig::PrintConfig()
{
    LogMessage(MSG_GEN_049, {"Cards configuration:"});
    for (const auto & cfg : this->m_xbtest_common_config.card_config_map)
    {
        auto bdf         = cfg.first;
        auto card_config = cfg.second;

        LogMessageBdf(MSG_DEBUG_SETUP, bdf, {"Name: " + card_config.dsa_name});
        LogMessageBdf(MSG_DEBUG_SETUP, bdf, {"Interface UUID: " + card_config.interface_uuid});

        std::vector<std::string> test_id_names;
        test_id_names.reserve(test_id_names.size());
        for (const auto & test : card_config.tests)
        {
            test_id_names.emplace_back(test.test_id_name);
        }
        LogMessageBdf(MSG_GEN_049, bdf, {"Tests: " + StrVectToStr(test_id_names, ", ")});
        for (const auto & test : card_config.tests)
        {
            std::vector<std::string> tmp;
            if (test.is_pre_canned)
            {
                tmp.emplace_back(PRE_CANNED + ": " + test.pre_canned);
            }
            else if (test.is_test_json)
            {
                tmp.emplace_back(TEST_JSON + ": " + test.test_json);
            }
            if (!test.args.empty())
            {
                tmp.emplace_back(ARGS + ": " + StrVectToStr(test.args, " "));
            }
            if (test.use_log_dir)
            {
                tmp.emplace_back(LOG_DIR + ": " + test.log_dir);
            }
            LogMessageBdf(MSG_DEBUG_SETUP, bdf, {"\t * " + test.test_id_name + ") " + StrVectToStr(tmp, " / ")});
        }
    }
}

bool XbtestCommonConfig::GetMultitestMode()
{
    return this->m_xbtest_common_config.multitest_mode;
}

std::string XbtestCommonConfig::GetCommonLogDir()
{
    return this->m_xbtest_common_config.common_log_dir;
}

std::string XbtestCommonConfig::GetCommonRTLogDir()
{
    return GetCommonLogDir() + "/RT";
}

void XbtestCommonConfig::SetCommonLogDir( const std::string & common_log_dir )
{
    this->m_xbtest_common_config.common_log_dir = common_log_dir;
}

bool XbtestCommonConfig::GetUseCommonLogDir()
{
    return this->m_xbtest_common_config.use_common_log_dir;
}

void XbtestCommonConfig::SetUseCommonLogDir( const bool & use_common_log_dir )
{
    this->m_xbtest_common_config.use_common_log_dir = use_common_log_dir;
}

bool XbtestCommonConfig::GetCommandLineConsoleDetailedFormat()
{
    return this->m_xbtest_common_config.command_line.console_detailed_format;
}

bool XbtestCommonConfig::GetCommandLineForce()
{
    return this->m_xbtest_common_config.command_line.force;
}

void XbtestCommonConfig::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, message, arg_list);
}

void XbtestCommonConfig::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, message);
}

void XbtestCommonConfig::LogMessageBdf ( const Message_t & message, const std::string & bdf, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, bdf, "", message, arg_list);
}

void XbtestCommonConfig::LogMessageBdf ( const Message_t & message, const std::string & bdf )
{
    this->m_log->LogMessage(this->m_log_header, bdf, "", message);
}

void XbtestCommonConfig::PrintRequiredNotFound( const std::string & file_type, const std::vector<std::string> & node_title_in )
{
    LogMessage(MSG_ITF_044, {file_type, StrVectToStr(node_title_in, ".")});
}

void XbtestCommonConfig::PrintHWConfigRequiredNotFound( const std::string & lib_name, const std::string & file_type, const std::vector<std::string> & node_title_in )
{
    LogMessage(MSG_ITF_107, {lib_name, "Required parameter not found in " + file_type + ": " + StrVectToStr(node_title_in, ".")});
}

std::vector<std::string> XbtestCommonConfig::ListPreCannedTests( const std::string & lib_path )
{
    LogMessage(MSG_DEBUG_PARSING, {"List available pre-canned tests"});
    std::vector<std::string> pre_canned_test_list;
    auto list_base_dir  = lib_path + "/test/*";
    auto file_list      = ListDirectoryContent(list_base_dir);
    for (const auto & file : file_list)
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found pre-canned test file : " + file});
        std::string name;
        std::string ext;
        GetNameExt(file, name, ext);
        if (ext == "json")
        {
            pre_canned_test_list.emplace_back(name);
        }
    }
    return pre_canned_test_list;
}

bool XbtestCommonConfig::GetHWBuildInfo ( const std::string & lib_name, const std::string & config_filename, HW_Build_Info_t & hw_build_info )
{
    LogMessage(MSG_DEBUG_PARSING, {"Parsing xbtest HW design build information for " + lib_name});
    ConfigParser config_parser(this->m_log, config_filename, this->m_abort);
    config_parser.SetReportLevel(LOG_CRIT_WARN);
    if (config_parser.Parse() == RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"GetHWBuildInfo: Parse " + HW_CONFIG_JSON + " failed"});
        return RET_FAILURE;
    }

    std::vector<std::string> node_title = {BUILD_INFO, XBTEST, VERSION, MAJOR};
    if (config_parser.ExtractNodeValueInt<uint>(node_title, hw_build_info.xbtest.version.major) == RET_FAILURE)
    {
        PrintHWConfigRequiredNotFound(lib_name, HW_CONFIG_JSON, node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, VERSION, MINOR};
    if (config_parser.ExtractNodeValueInt<uint>(node_title, hw_build_info.xbtest.version.minor) == RET_FAILURE)
    {
        PrintHWConfigRequiredNotFound(lib_name, HW_CONFIG_JSON, node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, DATE};
    if (config_parser.ExtractNodeValueStr(node_title, hw_build_info.xbtest.date) == RET_FAILURE)
    {
        PrintHWConfigRequiredNotFound(lib_name, HW_CONFIG_JSON, node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, INTERNALRELEASE};
    hw_build_info.xbtest.internal_release = false;
    if (config_parser.NodeExists(node_title))
    {
        if (config_parser.ExtractNodeValueBool(node_title, hw_build_info.xbtest.internal_release) == RET_FAILURE)
        {
            PrintHWConfigRequiredNotFound(lib_name, HW_CONFIG_JSON, node_title);
            return RET_FAILURE;
        }
    }
    node_title = {BUILD_INFO, BOARD, NAME};
    if (config_parser.ExtractNodeValueStr(node_title, hw_build_info.board.name) == RET_FAILURE)
    {
        PrintHWConfigRequiredNotFound(lib_name, HW_CONFIG_JSON, node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, BOARD, INTERFACE_UUID};
    if (config_parser.ExtractNodeValueStr(node_title, hw_build_info.board.interface_uuid) == RET_FAILURE)
    {
        PrintHWConfigRequiredNotFound(lib_name, HW_CONFIG_JSON, node_title);
        return RET_FAILURE;
    }
    FormatUuid(hw_build_info.board.interface_uuid);

    LogMessage(MSG_DEBUG_SETUP, {"xbtest HW design build information for : " + lib_name});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Major Version           : " + std::to_string(hw_build_info.xbtest.version.major)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Minor version           : " + std::to_string(hw_build_info.xbtest.version.minor)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Build date              : " +                hw_build_info.xbtest.date});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Internal release        : " +      BoolToStr(hw_build_info.xbtest.internal_release)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Board name              : " +                hw_build_info.board.name});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Board interface UUID    : " +                hw_build_info.board.interface_uuid});
    return RET_SUCCESS;
}

bool XbtestCommonConfig::GetHWBuildInfo2 ( const std::string & lib_name, const std::string & config_filename, HW_Build_Info_t & hw_build_info )
{
    LogMessage(MSG_DEBUG_PARSING, {"Parsing xbtest HW design build information for " + lib_name});
    ConfigParser config_parser(this->m_log, config_filename, this->m_abort);
    config_parser.SetReportLevel(LOG_CRIT_WARN);
    if (config_parser.Parse() == RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"GetHWBuildInfo: Parse " + PARTITION_METADATA + " failed"});
        return RET_FAILURE;
    }

    std::vector<std::string> node_title = {PARTITION_METADATA, LOGIC_UUID};
    if (config_parser.ExtractNodeValueStr(node_title, hw_build_info.board.logic_uuid) == RET_FAILURE)
    {
        PrintHWConfigRequiredNotFound(lib_name, PARTITION_METADATA, node_title);
        return RET_FAILURE;
    }
    FormatUuid(hw_build_info.board.logic_uuid);

    LogMessage(MSG_DEBUG_SETUP, {"\t - Board logic UUID        : " + hw_build_info.board.logic_uuid});
    return RET_SUCCESS;
}

bool XbtestCommonConfig::GetPlatformDefinition ( const std::string & lib_name, const std::string & config_filename, HW_Lib_Config_t & hw_lib_config )
{
    LogMessage(MSG_DEBUG_PARSING, {"Get card definition for " + lib_name});
    ConfigParser config_parser(this->m_log, config_filename, m_abort);
    if (config_parser.Parse() == RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"Parse " + HW_CONFIG_JSON + " failed"});
        return RET_FAILURE;
    }

    std::vector<std::string> node_title = {DEVICE, SENSOR};
    LogMessage(MSG_DEBUG_PARSING, {"Get " + StrVectToStr(node_title, ".") + " parameters"});

    // Default
    hw_lib_config.card_power_id          = DEFAULT_CARD_POWER_ID;
    hw_lib_config.card_temperature_id    = DEFAULT_CARD_TEMPERATURE_ID;

    if (config_parser.NodeExists(node_title))
    {
        // Get sensor sources
        for (uint idx = 0; idx < MAX_SENSORS; idx++)
        {
            node_title = {DEVICE, SENSOR, std::to_string(idx)};
            if (!config_parser.NodeExists(node_title))
            {
                continue; // Finished get sensor sources
            }

            LogMessage(MSG_DEBUG_PARSING, {"\t - Get sensor sources[" + std::to_string(idx) + "]"});
            std::string id;
            std::string type;
            bool card_power;
            bool card_temperature;

            // ID
            node_title.push_back(ID);
            if (!config_parser.NodeExists(node_title))
            {
                continue;
            }
            if (config_parser.ExtractNodeValueStr(node_title, id) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            node_title.pop_back();

            // CARD_POWER
            node_title.push_back(CARD_POWER);
            if (config_parser.NodeExists(node_title))
            {
                if (config_parser.ExtractNodeValueBool(node_title, card_power) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                if (card_power)
                {
                    hw_lib_config.card_power_id   = id;
                }
            }
            node_title.pop_back();


            // CARD_TEMPERATURE
            node_title.push_back(CARD_TEMPERATURE);
            if (config_parser.NodeExists(node_title))
            {
                if (config_parser.ExtractNodeValueBool(node_title, card_temperature) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                if (card_temperature)
                {
                    hw_lib_config.card_temperature_id   = id;
                }
            }
            node_title.pop_back();
        }
    }
    return RET_SUCCESS;
}

bool XbtestCommonConfig::CheckFileExists( const std::string & source_file, const std::string & filename, const bool & log_error )
{
    if (!FileExists(filename))
    {
        if (log_error)
        {
            LogMessage(MSG_ITF_003, {source_file, filename});
        }
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool XbtestCommonConfig::ScanLibraries()
{
    LogMessage(MSG_GEN_016, {"Scanning installed xbtest HW designs..."});
    this->m_amd_xbtest_libs.clear();
    auto amd_xbtest_libs_env = getenv("AMD_XBTEST_LIBS");
    if (amd_xbtest_libs_env != nullptr)
    {
        LogMessage(MSG_DEBUG_PARSING, {"Using provided AMD_XBTEST_LIBS"});
        this->m_amd_xbtest_libs_str  = std::string(amd_xbtest_libs_env);
        this->m_amd_xbtest_libs      = SplitNoEmpty(this->m_amd_xbtest_libs_str, ':');
    }
#ifdef USE_AMI
    this->m_amd_xbtest_libs.emplace_back("/opt/amd/aved");
#endif
#ifdef USE_XRT
    this->m_amd_xbtest_libs.emplace_back("/opt/amd/xbtest/lib");
#endif

    for (const auto & lib : this->m_amd_xbtest_libs)
    {
        LogMessage(MSG_DEBUG_PARSING, {"\t - " + lib});
    }

    this->m_num_lib_valid = 0; // Check if at least one valid xbtest HW design is detected

    for (const auto & lib : this->m_amd_xbtest_libs)
    {
#ifdef USE_AMI
        auto list_base_dir = lib + "/*";

        this->m_xbtest_common_config.lib_name_list.clear();
        for (const auto & design_dir_name : ListDirectoryContent(list_base_dir)) // List the card directory names
        {
            this->m_xbtest_common_config.lib_name_list.emplace_back(design_dir_name);
        }
#endif
#ifdef USE_XRT
        auto list_base_dir = lib + "/*";
        this->m_xbtest_common_config.lib_name_list = ListDirectoryContent(list_base_dir); // List the xbtest HW design names
#endif

        for (const auto & lib_name : this->m_xbtest_common_config.lib_name_list)
        {
            HW_Lib_Config_t lib_config;
            lib_config.valid                = true; // Valid by default. Is added to std::map with lib_name as identifier if invalid. If valid, identifier is UUID
            lib_config.lib_name             = lib_name;
            std::replace(lib_config.lib_name.begin(), lib_config.lib_name.end(), '/', '-');
            lib_config.lib_path             = lib;
            lib_config.lib_path             += "/";
            lib_config.lib_path             += lib_name;

#ifdef USE_AMI
            lib_config.metadata_path                = lib_config.lib_path      + "/xbtest/metadata";
            lib_config.xbtest_pfm_def               = lib_config.metadata_path + "/xbtest_pfm_def.json";
            lib_config.pdi                          = lib_config.lib_path      + "/design.pdi";
            lib_config.config_filename              = lib_config.metadata_path + "/user_metadata.json"; // Get xbtest HW version
            lib_config.xbtest_metadata_filename     = lib_config.metadata_path + "/xbtest_metadata.json"; // Get logic UUID
            lib_config.pre_canned_test_list = ListPreCannedTests(lib_config.lib_path+"/xbtest");
#endif
#ifdef USE_XRT
            lib_config.xbtest_pfm_def               = lib_config.lib_path + "/xbtest_pfm_def.json";
            lib_config.xclbin                       = lib_config.lib_path + "/xclbin/xbtest_stress.xclbin";
            lib_config.config_filename              = lib_config.lib_path + "/config.json";
            lib_config.pre_canned_test_list = ListPreCannedTests(lib_config.lib_path);
#endif


            LogMessage(MSG_DEBUG_PARSING, {"Using:"});
            LogMessage(MSG_DEBUG_PARSING, {"\t - xbtest HW design name     : " + lib_config.lib_name});
            LogMessage(MSG_DEBUG_PARSING, {"\t - xbtest HW design path     : " + lib_config.lib_path});

#ifdef USE_AMI
            LogMessage(MSG_DEBUG_PARSING, {"\t - Metadata path             : " + lib_config.metadata_path});
            LogMessage(MSG_DEBUG_PARSING, {"\t - PDI                       : " + lib_config.pdi});
            LogMessage(MSG_DEBUG_PARSING, {"\t - xbtest metatadata         : " + lib_config.xbtest_metadata_filename});
#endif
#ifdef USE_XRT
            LogMessage(MSG_DEBUG_PARSING, {"\t - xclbin                    : " + lib_config.xclbin});
#endif

            LogMessage(MSG_DEBUG_PARSING, {"\t - xbtest_pfm_def            : " + lib_config.xbtest_pfm_def});
            LogMessage(MSG_DEBUG_PARSING, {"\t - Config file               : " + lib_config.config_filename});

            if (CheckFileExists(XBTEST_PFM_DEF, lib_config.xbtest_pfm_def, false) == RET_FAILURE)
            {
                LogMessage(MSG_ITF_107, {lib_config.lib_name, "Required xbtest HW design file not found: " + lib_config.xbtest_pfm_def});
                lib_config.valid = false;
            }

#ifdef USE_AMI
            if (CheckFileExists(DESIGN_PDI, lib_config.pdi, false) == RET_FAILURE)
            {
                LogMessage(MSG_ITF_107, {lib_config.lib_name, "Required xbtest HW design file not found: " + lib_config.pdi});
                lib_config.valid = false;
            }
#endif
#ifdef USE_XRT
            if (CheckFileExists(XCLBIN, lib_config.xclbin, false) == RET_FAILURE)
            {
                LogMessage(MSG_ITF_107, {lib_config.lib_name, "Required xbtest HW design file not found: " + lib_config.xclbin});
                lib_config.valid = false;
            }
#endif

            if (CheckFileExists(HW_CONFIG_JSON, lib_config.config_filename, false) == RET_FAILURE)
            {
                LogMessage(MSG_ITF_107, {lib_config.lib_name, "Required xbtest HW design file not found: " + lib_config.config_filename});
                lib_config.valid = false;
            }
            if (GetHWBuildInfo(lib_config.lib_name, lib_config.config_filename, lib_config.build_info) == RET_FAILURE)
            {
                lib_config.valid = false;
            }

#ifdef USE_AMI
            if (GetHWBuildInfo2(lib_config.lib_name, lib_config.xbtest_metadata_filename, lib_config.build_info) == RET_FAILURE)
            {
                lib_config.valid = false;
            }
#endif

            if (GetPlatformDefinition(lib_config.lib_name, lib_config.xbtest_pfm_def, lib_config) == RET_FAILURE)
            {
                lib_config.valid = false;
            }

#ifdef USE_XRT
            // Ignore installed HW designs when interface_uuid is not defined
            if (lib_config.build_info.board.interface_uuid == "NOT DEFINED")
            {
                LogMessage(MSG_ITF_107, {lib_config.lib_name, "Interface uuid not defined"});
                lib_config.valid = false;
            }
#endif

            auto identifier_value = "invalid_" + lib_config.lib_name; // default identify as invalid
            if (lib_config.valid)
            {
#ifdef USE_AMI
                identifier_value = lib_config.build_info.board.logic_uuid;
#endif
#ifdef USE_XRT
                identifier_value = lib_config.build_info.board.interface_uuid;
#endif
                this->m_num_lib_valid++;
            }
            this->m_xbtest_common_config.map_lib_config.emplace(identifier_value, lib_config);
        }
    }
    return RET_SUCCESS;
}

bool XbtestCommonConfig::InsertOptions( const std::string & bdf, const uint & test_idx, std::vector<std::string> & arg_dst, const std::vector<std::string> & arg_src )
{
    auto opts_dst = GetOpts(arg_dst);
    auto opts_src = GetOpts(arg_src);

    for (const auto & opt_dst : opts_dst)
    {
        if (FindInVect(opt_dst, opts_src))
        {
            LogMessageBdf(MSG_GEN_058, bdf, {opt_dst, std::to_string(test_idx)});
            return RET_FAILURE;
        }
    }
    arg_dst.insert(arg_dst.end(), arg_src.begin(), arg_src.end());
    return RET_SUCCESS;
}

std::vector<std::string> XbtestCommonConfig::GetOpts( const std::vector<std::string> & args )
{
    std::vector<std::string> args_tmp = {"dummy_cmd"};
    args_tmp.insert(args_tmp.end(), args.begin(), args.end());
    std::vector<char *> arg_char;
    arg_char.reserve(args_tmp.size());
    for (const auto & str : args_tmp)
    {
        arg_char.emplace_back(const_cast<char*>(str.c_str()));
    }
    arg_char.emplace_back(nullptr);
    char **argv = &arg_char[0];
    int    argc = args_tmp.size();

    optind = 0; // reset getopt
    std::string opt_string = ":";
    std::vector<std::string> opts;

    optind = 0; // reinitialization of getopt()
    while (getopt(argc, argv, opt_string.c_str()) != -1)
    {
        if (isprint(optopt) != 0)
        {
            char buff [256];
            sprintf(buff, "%c", optopt);
            auto str = std::string(buff);
            opts.emplace_back(str);
        }
    }
    optind = 1; // reset optind
    return opts;
}

void XbtestCommonConfig::GetNameExt( const std::string & file, std::string & name, std::string & ext )
{
    auto name_ext = split(file, '.');
    ext  = name_ext.back();
    if (name_ext.size() > 1)
    {
        name_ext.pop_back();
        name = StrVectToStr(name_ext, ".");
    }
    else
    {
        name = name_ext.back();
    }
}

} // namespace
