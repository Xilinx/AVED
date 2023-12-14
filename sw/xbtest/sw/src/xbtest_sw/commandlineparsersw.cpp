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

#ifdef USE_XRT
// xrt/include/version.h
#include "version.h"
#endif

#include "commandlineparsersw.h"

namespace xbtest
{

// Define static variable
std::mutex CommandLineParserSw::m_mutex_getopt;

CommandLineParserSw::CommandLineParserSw( Logging * log, std::atomic<bool> * abort, int argc, char ** argv ) : CommandLineParserBase::CommandLineParserBase(log, abort, argc, argv)
{
    this->m_command_line.data.clear();
    for (int i = 0; i < argc; ++i)
    {
        this->m_command_line.data.emplace_back(std::string(argv[i]));
    }
    // Opt with no arg
    this->m_command_line.help                       = false;
    this->m_command_line.version                    = false;
    this->m_command_line.disable_log                = false;
    this->m_command_line.console_detailed_format    = false;
    this->m_command_line.force                      = false;
    // Opt with arg
    this->m_command_line.test_json.exists           = false;
    this->m_command_line.xbtest_pfm_def.exists      = false;
    this->m_command_line.xclbin.exists              = false;
    this->m_command_line.device_bdf.exists          = false;
    this->m_command_line.p2p_target_bdf.exists      = false;
    this->m_command_line.p2p_nvme_path.exists       = false;
    this->m_command_line.p2p_xclbin.exists          = false;
    this->m_command_line.p2p_xbtest_pfm_def.exists  = false;
    this->m_command_line.log_dir.exists             = false;
    this->m_command_line.timestamp_mode.exists      = false;
    this->m_command_line.verbosity.exists           = false;
    this->m_command_line.msg_id.exists              = false;
    this->m_command_line.guide.exists               = false;
    this->m_command_line.design_pdi.exists          = false;
}

CommandLineParserSw::~CommandLineParserSw() = default;

uint CommandLineParserSw::ParseCommandLine()
{
    // Prevent multple instances of CommandLineParserSw using getopt() at same time
    std::lock_guard<std::mutex> guard(xbtest::CommandLineParserSw::m_mutex_getopt);

    LogMessage(MSG_DEBUG_PARSING, {"Parsing command line: " + StrVectToStr(this->m_command_line.data, " ")});

   // first character of optstring is :
   //   - so it returns ':' instead of '?' to indicate a missing option argument
   //   - getopt will not print error messages, error messages are handled by xbtest
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    // No xclbin for AMI
    // P2P option disabled for AMI
    std::string opt_string = ":hvLFfpm:g:j:e:l:d:t:b:i:s:";
#endif
#ifdef USE_XRT
    // No PDI for AMI
    std::string opt_string = ":hvLFfpm:g:j:e:E:l:d:T:N:x:X:t:b:s:";
#endif

    int test_option;

    LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: opt_string = " + opt_string});

    optind = 0; // reinitialization of getopt()
    while ((test_option = getopt(this->m_argc, this->m_argv, opt_string.c_str())) != -1)
    {
        std::string optarg_str;
        std::string buff_str;

        if (optarg != nullptr) // nullptr for example when the option do not have an argument
        {
            optarg_str = std::string(optarg);
        }
        if (isprint(optopt) != 0)
        {
            char buff [256];
            sprintf(buff, "%c", optopt);
            buff_str = std::string(buff);
        }
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: test_option= " + std::string(1, test_option)});
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: optarg_str = " + optarg_str});
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: buff_str   = " + buff_str});

        switch (test_option)
        {
            // Opt with no arg
            case 'h':
            {
                this->m_command_line.help = true;
                break;
            }
            case 'v':
            {
                this->m_command_line.version = true;
                break;
            }
            case 'L':
            {
                this->m_command_line.disable_log = true;
                break;
            }
            case 'F':
            {
                this->m_command_line.console_detailed_format = true;
                break;
            }
            case 'f':
            {
                this->m_command_line.force = true;
                break;
            }
            case 'i':
            {
                if (ParseOptionArgumentStr(this->m_command_line.design_pdi, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'j':
            {
                if (ParseOptionArgumentStr(this->m_command_line.test_json, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'e':
            {
                if (ParseOptionArgumentStr(this->m_command_line.xbtest_pfm_def, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'E':
            {
                if (ParseOptionArgumentStr(this->m_command_line.p2p_xbtest_pfm_def, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'x':
            {
                if (ParseOptionArgumentStr(this->m_command_line.xclbin, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'X':
            {
                if (ParseOptionArgumentStr(this->m_command_line.p2p_xclbin, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'd':
            {
                if (ParseOptionArgumentStr(this->m_command_line.device_bdf, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'T':
            {
                if (ParseOptionArgumentStr(this->m_command_line.p2p_target_bdf, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'N':
            {
                if (ParseOptionArgumentStr(this->m_command_line.p2p_nvme_path, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'l':
            {
                if (ParseOptionArgumentStr(this->m_command_line.log_dir, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 't':
            {
                if (ParseOptionArgumentStr(this->m_command_line.timestamp_mode, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'b':
            {
                if (ParseOptionArgumentInt(this->m_command_line.verbosity, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                if (SetVerbosity(this->m_command_line.verbosity.value) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'm':
            {
                if (ParseOptionArgumentStr(this->m_command_line.msg_id, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case 'g':
            {
                if (ParseOptionArgumentStr(this->m_command_line.guide, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                break;
            }
            case ':':
            {
                LogMessage(MSG_GEN_004, {buff_str});
                return COMMAND_LINE_RET_FAILURE;
                break;
            }
            default: /* '?' */
            {
                LogMessage(MSG_GEN_005, {buff_str});
                return COMMAND_LINE_RET_FAILURE;
                break;
            }
        }
    }

    LogMessage(MSG_DEBUG_PARSING, {"Check for unknown command line options"});
    if (optind < this->m_argc)
    {
        std::vector<std::string> non_argv;
        non_argv.clear();
        while (optind < this->m_argc)
        {
            non_argv.emplace_back(std::string(this->m_argv[optind++]));
        }
        LogMessage(MSG_GEN_006, {std::to_string(non_argv.size()), StrVectToStr(non_argv, ", ")});
        return COMMAND_LINE_RET_FAILURE;
    }

    if (this->m_command_line.log_dir.exists && this->m_command_line.disable_log)
    {
        LogMessage(MSG_GEN_036);
        return COMMAND_LINE_RET_FAILURE;
    }

    // Help and version
    if (this->m_command_line.help)
    {
        PrintUsage();
    }
    if (this->m_command_line.version)
    {
#ifdef USE_AMI
        if (GetAMIBuildVersion() == RET_FAILURE)
        {
            return COMMAND_LINE_RET_FAILURE;
        }
#endif
        PrintVersion();
    }
    if (this->m_command_line.msg_id.exists)
    {
        PrintMessage(this->m_command_line.msg_id.value);
    }
    if (this->m_command_line.disable_log)
    {
        this->m_command_line.log_dir.exists = false;
    }
    if ((this->m_command_line.help) || (this->m_command_line.version) || (this->m_command_line.msg_id.exists))
    {
        return COMMAND_LINE_EXIT_SUCCESS;
    }

    LogMessage(MSG_DEBUG_PARSING, {"Checking mandatory options"});
    // If "-g" provided, the "-j"/"-e"/"-x" options are not mandatory
    if (!(this->m_command_line.test_json.exists))
    {
        if (!(this->m_command_line.guide.exists))
        {
            LogMessage(MSG_GEN_008, {"j"});
            return COMMAND_LINE_RET_FAILURE;
        }
    }
    else
    {
        if (CheckFileExists(TEST_JSON, this->m_command_line.test_json.value) == RET_FAILURE)
        {
            return COMMAND_LINE_RET_FAILURE;
        }
    }

    // "-e" always mandatory
    if (!(this->m_command_line.xbtest_pfm_def.exists))
    {
        LogMessage(MSG_GEN_008, {"e"});
        return COMMAND_LINE_RET_FAILURE;
    }
    if (CheckFileExists(XBTEST_PFM_DEF, this->m_command_line.xbtest_pfm_def.value) == RET_FAILURE)
    {
        return COMMAND_LINE_RET_FAILURE;
    }

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    if (!(this->m_command_line.design_pdi.exists))
    {
        LogMessage(MSG_GEN_008, {"e"});
        return COMMAND_LINE_RET_FAILURE;
    }
    if (CheckFileExists(DESIGN_PDI, this->m_command_line.design_pdi.value) == RET_FAILURE)
    {
        return COMMAND_LINE_RET_FAILURE;
    }
#endif
#ifdef USE_XRT
    if (!(this->m_command_line.xclbin.exists))
    {
        LogMessage(MSG_GEN_008, {"x"});
        return COMMAND_LINE_RET_FAILURE;
    }
    if (CheckFileExists(XCLBIN, this->m_command_line.xclbin.value) == RET_FAILURE)
    {
        return COMMAND_LINE_RET_FAILURE;
    }
#endif

    // "-d" always mandatory
    if (!(this->m_command_line.device_bdf.exists))
    {
        LogMessage(MSG_GEN_008, {"d"});
        return COMMAND_LINE_RET_FAILURE;
    }
    // Detect if bdf provided is correct
    if (CheckCardBDF(this->m_command_line.device_bdf.value) == RET_FAILURE)
    {
        LogMessage(MSG_GEN_014, {this->m_command_line.device_bdf.value, OptionCharToName('d'), "Incorrect format"});
        return COMMAND_LINE_RET_FAILURE;
    }
    // Check timestamp_mode
    if (this->m_command_line.timestamp_mode.exists)
    {
        if (!FindStringInSet(this->m_command_line.timestamp_mode.value, SUPPORTED_TIMESTAMP_MODE))
        {
            LogMessage(MSG_GEN_014, {this->m_command_line.timestamp_mode.value, TIMESTAMP_MODE, "Supported values are " + StrSetToStr(SUPPORTED_TIMESTAMP_MODE, ", ")});
            return COMMAND_LINE_RET_FAILURE;
        }
        this->m_log->SetTimestampMode(this->m_command_line.timestamp_mode.value);
    }
    if (this->m_command_line.p2p_target_bdf.exists || this->m_command_line.p2p_nvme_path.exists)
    {
        if (CheckTargetP2P(this->m_command_line.device_bdf.value, this->m_command_line.p2p_target_bdf, this->m_command_line.p2p_nvme_path) == RET_FAILURE)
        {
            return COMMAND_LINE_RET_FAILURE;
        }
    }
    return COMMAND_LINE_RET_SUCCESS;
}

Xbtest_Command_Line_Sw_t CommandLineParserSw::GetCommandLine()
{
    return this->m_command_line;
}

std::string CommandLineParserSw::OptionCharToName( const char & option_char )
{
    switch (option_char)
    {
        case 'h': return HELP;
        case 'v': return VERSION;
        case 'm': return MSG_ID;
        case 'g': return GUIDE;
        case 'j': return TEST_JSON;
        case 'd': return DEVICE_BDF;
        case 'T': return P2P_TARGET_BDF;
        case 'N': return P2P_NVME_PATH;
        case 'l': return LOG_DIR;
        case 'L': return DISABLE_LOG;
        case 'f': return FORCE;
        case 'i': return DESIGN_PDI;
        // case 'I': return P2P_DESIGN_PDI;
        case 'F': return CONSOLE_DETAILED_FORMAT;
        case 'e': return XBTEST_PFM_DEF;
        case 'E': return P2P_XBTEST_PFM_DEF;
        case 'x': return XCLBIN;
        case 'X': return P2P_XCLBIN;
        case 't': return TIMESTAMP_MODE;
        case 'b': return VERBOSITY;
    }
    return UNKNOWN_OPTION;
}

void CommandLineParserSw::PrintUsage()
{
    LogMessage(MSG_GEN_002, {"Usage: " + this->m_command_line.data[0] + " [options]"});
    LogMessage(MSG_GEN_002, {"Command and option summary:"});
    LogMessage(MSG_GEN_002, {"\t -h                                 : Print this message"});
    LogMessage(MSG_GEN_002, {"\t -v                                 : Print version"});
    LogMessage(MSG_GEN_002, {"\t -m <message ID>                    : Display the message definition for the given <message ID>. E.g: use \"-m GEN_002\""});
    LogMessage(MSG_GEN_002, {"\t -g <testcase/task> -d <card BDF>   : Display basic test JSON options for the selected <testcase/task> for selected card at <card BDF>"});
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"\t -d <card BDF>                      : Select card identified by its BDF <card BDF>"});
    LogMessage(MSG_GEN_002, {"\t -j <test JSON file>                : Select test JSON <test JSON file>"});
    LogMessage(MSG_GEN_002, {"\t -l <log directory>                 : Define the name of a directory <log directory> in which all log files will be stored"});
    LogMessage(MSG_GEN_002, {"\t -L                                 : Disable logging directory generation"});
    LogMessage(MSG_GEN_002, {"\t -f                                 : When possible, force an operation"});
    LogMessage(MSG_GEN_002, {"\t -F                                 : Disable the dynamic display mode"});
    LogMessage(MSG_GEN_002, {"\t -e <card definition JSON file>     : Select card definition JSON file <card definition JSON file>"});
#ifdef USE_XRT
    LogMessage(MSG_GEN_002, {"\t -x <xclbin>                        : Select xclbin <xclbin>"});
    LogMessage(MSG_GEN_002, {"\t -E <card definition JSON file>     : Select card definition JSON file <card definition JSON file> of P2P target card"});
    LogMessage(MSG_GEN_002, {"\t -X <xclbin>                        : Select xclbin <xclbin> of P2P target card"});
    LogMessage(MSG_GEN_002, {"\t -T <card BDF>                      : Select P2P target card BDF <card BDF>. When specified, only one BDF must be provided using -d"});
    LogMessage(MSG_GEN_002, {"\t -N <path>                          : Select path <path> used in P2P_NVME test case. When specified, only one BDF must be provided using -d"});
#endif
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    LogMessage(MSG_GEN_002, {"\t -i <design_pdi>                    : Select design PDI <design_pdi>. Directory named 'metadata' containing required metadata JSON files is expected at same direcotry as PDI"});
#endif
    LogMessage(MSG_GEN_002, {"\t -t <timestamp>                     : Set timestamp mode <timestamp>. Supported values: " + StrSetToStr(SUPPORTED_TIMESTAMP_MODE, ", ")});
    LogMessage(MSG_GEN_002, {"\t -b <verbosity>                     : Select verbosity level <verbosity> in [" + std::to_string(MIN_VERBOSITY) + "," + std::to_string(MAX_VERBOSITY) + "]"});
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"For complete documentation, refer to the User Guide"});
    LogMessage(MSG_GEN_002, {""});
#ifdef USE_NO_DRIVER
    LogMessage(MSG_GEN_002, {"Try \"lspci\" to get BDF of installed cards (see option -d)"});
#endif
#ifdef USE_AMI
    LogMessage(MSG_GEN_002, {"Try \"ami_tool overview\" to get BDF of installed cards (see option -d)"});
#endif
#ifdef USE_XRT
    LogMessage(MSG_GEN_002, {"Try \"xbutil examine\" to get BDF of installed cards (see option -d)"});
#endif
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"The following testcase/task guides are available depending on the selected card (see option -g):"});

    for (const auto & task_def : TASKS_PARAMETERS_DEF)
    {
        auto name = task_def.name;
        std::transform(name.begin(), name.end(), name.begin(), tolower);
        if (task_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_GEN_002, {"\t\t * -g " + pad(name, ' ', GEN_002_PAD, PAD_ON_RIGHT) + " : " + task_def.description});
        }
    }
    for (const auto & tescase_def : TESTCASES_PARAMETERS_DEF)
    {
        auto name = tescase_def.name;
        std::transform(name.begin(), name.end(), name.begin(), tolower);
        if (tescase_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_GEN_002, {"\t\t * -g " + pad(tescase_def.name, ' ', GEN_002_PAD, PAD_ON_RIGHT) + " : " + tescase_def.description});
        }
    }
}

void CommandLineParserSw::PrintVersion()
{
    LogMessage(MSG_GEN_003, {this->m_command_line.data[0] + " host code version " + XBTEST_VERSION_STR});
    LogMessage(MSG_GEN_003, {"\t - SW build: " + std::to_string(XBTEST_BUILD_VERSION) + " on " + XBTEST_BUILD_DATE});
    #ifdef INTERNAL_RELEASE
    LogMessage(MSG_GEN_003, {"\t - Release : INTERNAL"});
    #endif
    LogMessage(MSG_GEN_003, {"\t - Testcases version:"});
    LogMessage(MSG_GEN_003, {"\t\t * DMA        : " + std::to_string(DMA_SW_VERSION_MAJOR)          + "." + std::to_string(DMA_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * P2P_CARD   : " + std::to_string(P2P_CARD_SW_VERSION_MAJOR)     + "." + std::to_string(P2P_CARD_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * P2P_NVME   : " + std::to_string(P2P_NVME_SW_VERSION_MAJOR)     + "." + std::to_string(P2P_NVME_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * POWER      : " + std::to_string(PWR_SW_VERSION_MAJOR)          + "." + std::to_string(PWR_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * MEMORY     : " + std::to_string(MEM_SW_VERSION_MAJOR)          + "." + std::to_string(MEM_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * GT_MAC     : " + std::to_string(GT_MAC_SW_VERSION_MAJOR)       + "." + std::to_string(GT_MAC_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * GT_LPBK    : " + std::to_string(GT_LPBK_SW_VERSION_MAJOR)      + "." + std::to_string(GT_LPBK_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * GT_PRBS    : " + std::to_string(GT_PRBS_SW_VERSION_MAJOR)      + "." + std::to_string(GT_PRBS_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * GTF_PRBS   : " + std::to_string(GTF_PRBS_SW_VERSION_MAJOR)     + "." + std::to_string(GTF_PRBS_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * GTM_PRBS   : " + std::to_string(GTM_PRBS_SW_VERSION_MAJOR)     + "." + std::to_string(GTM_PRBS_SW_VERSION_MINOR)});
    LogMessage(MSG_GEN_003, {"\t\t * GTYp_PRBS  : " + std::to_string(GTYP_PRBS_SW_VERSION_MAJOR)    + "." + std::to_string(GTYP_PRBS_SW_VERSION_MINOR)});

#ifdef USE_NO_DRIVER
    LogMessage(MSG_GEN_003, {" \t - Built without driver (XRT or AMI)"});
#endif
#ifdef USE_AMI
    LogMessage(MSG_GEN_003, {" \t - Built using AMI API:"});
    LogMessage(MSG_GEN_003, {" \t\t * Major software version        : " + std::to_string(this->m_ami_version.major)});
    LogMessage(MSG_GEN_003, {" \t\t * Minor software version        : " + std::to_string(this->m_ami_version.minor)});
    LogMessage(MSG_GEN_003, {" \t\t * Patch number (if applicable)  : " + std::to_string(this->m_ami_version.patch)});
    LogMessage(MSG_GEN_003, {" \t\t * Status                        : " + std::to_string(this->m_ami_version.dev_commits)});
#endif
#ifdef USE_XRT
    LogMessage(MSG_GEN_003, {" \t - Built using XRT:"});
    LogMessage(MSG_GEN_003, {" \t\t * Version  : " + std::string(xrt_build_version)});
    LogMessage(MSG_GEN_003, {" \t\t * Branch   : " + std::string(xrt_build_version_branch)});
    LogMessage(MSG_GEN_003, {" \t\t * Hash     : " + std::string(xrt_build_version_hash)});
    LogMessage(MSG_GEN_003, {" \t\t * Hash date: " + std::string(xrt_build_version_hash_date)});
    LogMessage(MSG_GEN_003, {" \t\t * Date     : " + std::string(xrt_build_version_date_rfc)});
#endif

}

} // namespace
