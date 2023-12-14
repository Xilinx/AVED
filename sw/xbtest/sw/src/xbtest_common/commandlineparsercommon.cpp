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

#include "commandlineparsercommon.h"

#ifdef USE_XRT

// xrt/include/version.h
#include "version.h"

#endif

namespace xbtest
{

CommandLineParserCommon::CommandLineParserCommon(
    Logging * log, std::atomic<bool> * abort, int argc, char ** argv, const std::vector<std::string> & amd_xbtest_libs,
    const std::map<std::string, HW_Lib_Config_t> & map_lib_config
) : CommandLineParserBase::CommandLineParserBase(log, abort, argc, argv)
{
    this->m_amd_xbtest_libs = amd_xbtest_libs;
    this->m_map_lib_config  = map_lib_config;

    this->m_command_line.data.clear();
    for (int i = 0; i < argc; ++i)
    {
        this->m_command_line.data.emplace_back(std::string(argv[i]));
    }
    this->m_command_line.all_cards.clear();
    this->m_command_line.singlecard_mode          = false;
    this->m_command_line.multicard_mode           = false;
    this->m_command_line.multitest_mode           = false;
    this->m_command_line.cardjson_mode            = false;
    this->m_command_line.help                     = false;
    this->m_command_line.version                  = false;
    this->m_command_line.card_json.exists         = false;
    this->m_command_line.p2p_target_bdf.exists    = false;
    this->m_command_line.p2p_nvme_path.exists     = false;
    this->m_command_line.disable_log              = false;
    this->m_command_line.log_dir.exists           = false;
    this->m_command_line.console_detailed_format  = false;
    this->m_command_line.force                    = false;
    ResetCommandLineTest();
}

CommandLineParserCommon::~CommandLineParserCommon() = default;

uint CommandLineParserCommon::ParseCommandLine()
{
    LogMessage(MSG_DEBUG_PARSING, {"Parsing command line: " + StrVectToStr(this->m_command_line.data, " ")});

    // Pre-process the command line to detect modes
    auto d_cnt = CountOpts(this->m_command_line.data, 'd');
    auto D_cnt = CountOpts(this->m_command_line.data, 'D');
    auto c_cnt = CountOpts(this->m_command_line.data, 'c');
    auto j_cnt = CountOpts(this->m_command_line.data, 'j');
    auto T_cnt = CountOpts(this->m_command_line.data, 'T');
    auto N_cnt = CountOpts(this->m_command_line.data, 'N');

    if (T_cnt > 0)
    {
        if (D_cnt > 0)
        {
            LogMessage(MSG_GEN_071, {"-T"});
            return COMMAND_LINE_RET_FAILURE;
        }
        if (d_cnt == 0)
        {
            LogMessage(MSG_GEN_072, {"-T"});
            return COMMAND_LINE_RET_FAILURE;
        }
        if (d_cnt > 1)
        {
            LogMessage(MSG_GEN_073, {"-T"});
            return COMMAND_LINE_RET_FAILURE;
        }
    }
    if (N_cnt > 0)
    {
        if (D_cnt > 0)
        {
            LogMessage(MSG_GEN_071, {"-N"});
            return COMMAND_LINE_RET_FAILURE;
        }
        if (d_cnt == 0)
        {
            LogMessage(MSG_GEN_072, {"-N"});
            return COMMAND_LINE_RET_FAILURE;
        }
        if (d_cnt > 1)
        {
            LogMessage(MSG_GEN_073, {"-N"});
            return COMMAND_LINE_RET_FAILURE;
        }
    }

    if (d_cnt > MAX_NUM_CARD)
    {
        LogMessage(MSG_GEN_054, {"cards", std::to_string(d_cnt), "-d", std::to_string(MAX_NUM_CARD)});
        return COMMAND_LINE_RET_FAILURE;
    }
    // command line option cannot be combined with -D
    if (D_cnt > 0)
    {
        if (d_cnt > 0)
        {
            LogMessage(MSG_GEN_044, {"d"});
            return COMMAND_LINE_RET_FAILURE;
        }
        if (c_cnt > 0)
        {
            LogMessage(MSG_GEN_044, {"c"});
            return COMMAND_LINE_RET_FAILURE;
        }
        if (j_cnt > 0)
        {
            LogMessage(MSG_GEN_044, {"j"});
            return COMMAND_LINE_RET_FAILURE;
        }
        this->m_command_line.cardjson_mode  = true;
        this->m_command_line.multitest_mode = true;
    }
    else
    {
        if (d_cnt == 1)
        {
            this->m_command_line.singlecard_mode = true;
            if (c_cnt + j_cnt > 1)
            {
                this->m_command_line.multitest_mode = true;
            }
        }
        else if (d_cnt > 1)
        {
            auto first_opt = this->m_command_line.data[1].substr(0,2);
            if (first_opt != "-d")
            {
                LogMessage(MSG_GEN_045);
                return COMMAND_LINE_RET_FAILURE;
            }
            auto last_opt = this->m_command_line.data[this->m_command_line.data.size()-2].substr(0,2);
            if (last_opt == "-d")
            {
                LogMessage(MSG_GEN_059);
                return COMMAND_LINE_RET_FAILURE;
            }
            this->m_command_line.multicard_mode = true;
            this->m_command_line.multitest_mode = true;
        }
    }

    // first character of optstring is :
    //   - so it returns ':' instead of '?' to indicate a missing option argument
    //   - getopt will not print error messages, error messages are handled by xbtests
    // other command line option should not be marked as unknown (GEN_005) here, they should be passed to the SW
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    // P2P option disabled for AMI
    std::string opt_string = ":hvLFfl:m:g:b:j:c:d:D:nee:EE:xx:XX:";
#endif
#ifdef USE_XRT
    std::string opt_string = ":hvLFfl:m:g:b:j:c:d:D:T:N:nee:EE:xx:XX:";
#endif
    int test_option;

    LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: opt_string = " + opt_string});

    auto last_optind     = optind;
    int  num_skipped_opt = 0;
    auto last_opt_is_bdf = false;
    auto test            = RESET_TEST_CONFIG;

    optind = 0; // reinitialization of getopt()
    while ((test_option = getopt(this->m_argc, this->m_argv, opt_string.c_str())) != -1)
    {
        auto opt_is_bdf = false;
        std::string optarg_str;
        std::string optopt_str;
        std::vector<std::string> curr_args_to_fwd_sw_level; // Use this in case block to avoid mixing opt as the uknown arg are put at the end

        if (optarg != nullptr) // nullptr for example when the option do not have an argument
        {
            optarg_str = std::string(optarg);
        }
        if (isprint(optopt) != 0)
        {
            char buff [256];
            sprintf(buff, "%c", optopt);
            optopt_str = std::string(buff);
        }
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: test_option= " + std::string(1, test_option)});
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: optarg_str = " + optarg_str});
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: optopt_str = " + optopt_str});

        switch (test_option)
        {
            case 'h':
            {
                if (this->m_command_line.multitest_mode)
                {
                    LogMessage(MSG_GEN_069, {"h"});
                    return COMMAND_LINE_RET_FAILURE;
                }
                this->m_command_line.help    = true;
                curr_args_to_fwd_sw_level.emplace_back("-h");
                break;
            }
            case 'v':
            {
                if (this->m_command_line.multitest_mode)
                {
                    LogMessage(MSG_GEN_069, {"v"});
                    return COMMAND_LINE_RET_FAILURE;
                }
                this->m_command_line.version = true;
                curr_args_to_fwd_sw_level.emplace_back("-v");
                break;
            }
            case 'm':
            {
                if (this->m_command_line.multitest_mode)
                {
                    LogMessage(MSG_GEN_069, {"m"});
                    return COMMAND_LINE_RET_FAILURE;
                }
                if (ParseOptionArgumentStr(this->m_command_line.msg_id, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                curr_args_to_fwd_sw_level.emplace_back("-m");
                curr_args_to_fwd_sw_level.emplace_back(this->m_command_line.msg_id.value);
                break;
            }
            case 'g':
            {
                if (this->m_command_line.multitest_mode)
                {
                    LogMessage(MSG_GEN_069, {"g"});
                    return COMMAND_LINE_RET_FAILURE;
                }
                if (ParseOptionArgumentStr(this->m_command_line.guide, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                curr_args_to_fwd_sw_level.emplace_back("-g");
                curr_args_to_fwd_sw_level.emplace_back(this->m_command_line.guide.value);
                break;
            }
            case 'D':
            {
                if (ParseOptionArgumentStr(this->m_command_line.card_json, test_option, optarg_str) == RET_FAILURE)
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
            case 'd':
            {
                // In multicard mode, command line starts with -d and different configuration are delimited with one or more -d
                if (this->m_command_line.multicard_mode && !(this->m_command_line.cards.empty()) && !last_opt_is_bdf)
                {
                    if (MapNewTests() == RET_FAILURE)
                    {
                        return COMMAND_LINE_RET_FAILURE;
                    }
                    ResetCommandLineTest();
                }

                std::string bdf;
                if (ParseOptionArgumentStr(bdf, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                // Detect if bdf provided is correct
                if (CheckCardBDF(bdf) == RET_FAILURE)
                {
                    LogMessage(MSG_GEN_014, {bdf, OptionCharToName('d'), "Incorrect format"});
                    return COMMAND_LINE_RET_FAILURE;
                }
                // Detect if same bdf provided more than once
                auto bdf_count = std::count(this->m_command_line.all_cards.begin(), this->m_command_line.all_cards.end(), bdf);
                if (bdf_count > 0)
                {
                    LogMessage(MSG_GEN_043, {bdf});
                    return COMMAND_LINE_RET_FAILURE;
                }
                this->m_command_line.all_cards.emplace_back(bdf);
                this->m_command_line.cards.emplace_back(bdf);

                opt_is_bdf = true; // We will know if we have separate card config
                break;
            }
            case 'c':
            {
                test.is_pre_canned = true;
                if (ParseOptionArgumentStr(test.pre_canned, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                this->m_command_line.tests.emplace_back(test);
                test = RESET_TEST_CONFIG; // reset for next test
                break;
            }
            case 'j':
            {
                test.is_test_json  = true;
                if (ParseOptionArgumentStr(test.test_json, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                this->m_command_line.tests.emplace_back(test);
                test = RESET_TEST_CONFIG; // reset for next test
                break;
            }
            case 'b':
            {
                if (ParseOptionArgumentInt(this->m_command_line.verbosity, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                // We set the common level verbosity every time we encounter it!
                if (SetVerbosity(this->m_command_line.verbosity.value) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                curr_args_to_fwd_sw_level.emplace_back("-b");
                curr_args_to_fwd_sw_level.emplace_back(std::to_string(this->m_command_line.verbosity.value));
                break;
            }
            case 'L':
            {
                this->m_command_line.disable_log = true;
                curr_args_to_fwd_sw_level.emplace_back("-L");
                break;
            }
            case 'l':
            {
                if (ParseOptionArgumentStr(this->m_command_line.log_dir, test_option, optarg_str) == RET_FAILURE)
                {
                    return COMMAND_LINE_RET_FAILURE;
                }
                // it will be passed to SW later for single test mode
                break;
            }
            case 'F':
            {
                this->m_command_line.console_detailed_format = true;
                curr_args_to_fwd_sw_level.emplace_back("-F");
                break;
            }
            case 'f':
            {
                this->m_command_line.force = true;
                curr_args_to_fwd_sw_level.emplace_back("-f");
                break;
            }
            case 'e' : // reserved for SW level
            case 'E' : // reserved for SW level
            case 'x' : // reserved for SW level
            case 'X' : // reserved for SW level
            case 'r' : // depreciated
            case 'n' : // depreciated
            {
                PrintCommandLineOptionNotSupported(test_option);
                return COMMAND_LINE_RET_FAILURE;
                break;
            }
            case ':': // ':' means an option requiring an argument was found without arguments. Invalid
            {
                LogMessage(MSG_GEN_004, {optopt_str});
                return COMMAND_LINE_RET_FAILURE;
                break;
            }
            default: // Default ('?') managed after
            {
                break;
            }
        }

        // Compute expect optind value of previous iteration
        int expected_last_optind = optind - 1;
        if (optarg != nullptr) // nullptr for example when the option do not have an argument
        {
            expected_last_optind--;
        }

        // A unknown non-option was present previously = argument of a previous option, need to forward it to SW
        if (last_optind != expected_last_optind)
        {
            for (int optind_tmp = last_optind; optind_tmp < expected_last_optind; optind_tmp++)
            {
                auto skipped_opt = std::string(this->m_argv[optind_tmp]);
                this->m_command_line.args.emplace_back(skipped_opt);
                LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: skipped_opt = " + skipped_opt});
                num_skipped_opt++;
            }
        }
        // An unknown option is present currently, need to forward it to SW
        if (test_option == '?')
        {
            this->m_command_line.args.emplace_back("-" + optopt_str);
        }
        last_optind     = optind;
        last_opt_is_bdf = opt_is_bdf;
        this->m_command_line.args.insert(this->m_command_line.args.end(), curr_args_to_fwd_sw_level.begin(), curr_args_to_fwd_sw_level.end());

        if (this->m_command_line.tests.size() > MAX_NUM_TEST)
        {
            LogMessage(MSG_GEN_054, {"tests", std::to_string(this->m_command_line.tests.size()), "-c or -j", std::to_string(MAX_NUM_TEST)});
            return COMMAND_LINE_RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: this->m_command_line.args = " + StrVectToStr(this->m_command_line.args, " ")});
        LogMessage(MSG_DEBUG_PARSING, {"---------------------------------------------------------------------------------------------------------------------------"});
    }

    // forward CL options other non-opt
    while (optind + num_skipped_opt < this->m_argc)
    {
        auto other_opt = std::string(this->m_argv[optind+num_skipped_opt]);
        this->m_command_line.args.emplace_back(other_opt);
        LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: other_opt = " + other_opt});
        optind++;
    }
    LogMessage(MSG_DEBUG_PARSING, {"ParseCommandLine: this->m_command_line.args with non-opt = " + StrVectToStr(this->m_command_line.args, " ")});

    // Add last configuration parsed
    if (!(this->m_command_line.cardjson_mode))
    {
        if (MapNewTests() == RET_FAILURE)
        {
            return COMMAND_LINE_RET_FAILURE;
        }
    }

    // Option at common level
    if (!(this->m_command_line.singlecard_mode) && !(this->m_command_line.multicard_mode) && !(this->m_command_line.cardjson_mode))
    {
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
        if ((this->m_command_line.help) || (this->m_command_line.version) || (this->m_command_line.msg_id.exists))
        {
            return COMMAND_LINE_EXIT_SUCCESS;
        }
        if (this->m_command_line.guide.exists)
        {
            LogMessage(MSG_GEN_034, {"g", "d"});
            return COMMAND_LINE_RET_FAILURE;
        }
        // No card BDF or card JSON provided
        LogMessage(MSG_GEN_008, {"d or -D"});
        return COMMAND_LINE_RET_FAILURE;
    }
    // Cannot combine -l and / -L
    if (this->m_command_line.log_dir.exists && this->m_command_line.disable_log)
    {
        LogMessage(MSG_GEN_036);
        return COMMAND_LINE_RET_FAILURE;
    }
    // Cards provided without test
    if (!(this->m_command_line.cards.empty()) && this->m_command_line.tests.empty())
    {
        LogMessage(MSG_GEN_008, {"c or -j"});
        return COMMAND_LINE_RET_FAILURE;
    }
    if (this->m_command_line.p2p_target_bdf.exists || this->m_command_line.p2p_nvme_path.exists)
    {
        if (CheckTargetP2P(this->m_command_line.cards.front(), this->m_command_line.p2p_target_bdf, this->m_command_line.p2p_nvme_path) == RET_FAILURE)
        {
            return COMMAND_LINE_RET_FAILURE;
        }
    }
    PrintConfig();
    return COMMAND_LINE_RET_SUCCESS;
}

Xbtest_Command_Line_Common_t CommandLineParserCommon::GetCommandLine()
{
    return this->m_command_line;
}

void CommandLineParserCommon::ResetCommandLineTest()
{
    this->m_command_line.msg_id.exists        = false;
    this->m_command_line.guide.exists         = false;
    this->m_command_line.verbosity.exists     = false;

    this->m_command_line.cards.clear();
    this->m_command_line.tests.clear();
    this->m_command_line.args.clear();
}

std::string CommandLineParserCommon::OptionCharToName( const char & option_char )
{
    switch (option_char)
    {
        case 'h': return HELP;
        case 'v': return VERSION;
        case 'm': return MSG_ID;
        case 'g': return GUIDE;
        case 'd': return DEVICE_BDF;
        case 'c': return PRE_CANNED;
        case 'j': return TEST_JSON;
        case 'D': return CARD_JSON_FILE;
        case 'T': return P2P_TARGET_BDF;
        case 'N': return P2P_NVME_PATH;
        case 'l': return LOG_DIR;
        case 'L': return DISABLE_LOG;
        case 'F': return CONSOLE_DETAILED_FORMAT;
        case 'f': return FORCE;
        case 'b': return VERBOSITY;
    }
    return UNKNOWN_OPTION;
}

bool CommandLineParserCommon::MapNewTests()
{
    // Adding empty test as only non-test arguments were provided
    if (this->m_command_line.tests.empty() && !(this->m_command_line.args.empty()))
    {
        this->m_command_line.tests.emplace_back(RESET_TEST_CONFIG);
    }
    // If some tests or args were provided, append them to the configuration.
    if (!(this->m_command_line.tests.empty()))
    {
        LogMessage(MSG_DEBUG_SETUP, {"Adding " + std::to_string(this->m_command_line.tests.size()) + " tests for cards: " + StrVectToStr(this->m_command_line.cards, ", ")});
        for (const auto & bdf : this->m_command_line.cards)
        {
            Card_Config_t card_config_map;
            card_config_map.args = this->m_command_line.args;
            for (const auto & test: this->m_command_line.tests)
            {
                Test_Config_t test2 = test;
                test2.args = card_config_map.args;
                card_config_map.tests.emplace_back(test2);
            }
            this->m_command_line.card_config_map.emplace(bdf, card_config_map);
        }
    }
    return RET_SUCCESS;
}

void CommandLineParserCommon::PrintConfig()
{
    for (const auto & cfg : this->m_command_line.card_config_map)
    {
        auto bdf         = cfg.first;
        auto card_config = cfg.second;

        LogMessage(MSG_DEBUG_SETUP, {"Card " + bdf + " configuration: "});
        for (uint i = 0; i < card_config.tests.size(); i++)
        {
            std::vector<std::string> tmp;
            if (card_config.tests[i].is_pre_canned)
            {
                tmp.emplace_back(PRE_CANNED + ": " + card_config.tests[i].pre_canned);
            }
            else if (card_config.tests[i].is_test_json)
            {
                tmp.emplace_back(TEST_JSON + ": " + card_config.tests[i].test_json);
            }
            if (!card_config.tests[i].args.empty())
            {
                tmp.emplace_back(ARGS + ": " + StrVectToStr(card_config.tests[i].args, " "));
            }
            LogMessage(MSG_DEBUG_SETUP, {"\t - Test " + std::to_string(i+1) + ") " + StrVectToStr(tmp, " / ")});
        }
    }
}

void CommandLineParserCommon::PrintUsage()
{
    LogMessage(MSG_GEN_002, {"Usage: " + this->m_command_line.data[0] + " [options]"});
    LogMessage(MSG_GEN_002, {"Command and option summary:"});
    LogMessage(MSG_GEN_002, {"\t -h                                 : Print this message"});
    LogMessage(MSG_GEN_002, {"\t -v                                 : Print version"});
    LogMessage(MSG_GEN_002, {"\t -m <message ID>                    : Display the message definition for the given <message ID>. E.g: use \"-m GEN_002\""});
    LogMessage(MSG_GEN_002, {"\t -d <card BDF> -g <testcase/task>   : Print basic test JSON options for the selected <testcase/task> for selected card at <card BDF>"});
    LogMessage(MSG_GEN_002, {"\t -d <card BDF> -h                   : Print other card specific options for selected card at <card BDF>"});
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"\t -d <card BDF>                      : Select card identified by its BDF <card BDF>"});
    LogMessage(MSG_GEN_002, {"\t -j <test JSON file>                : Select test JSON <test JSON file>"});
    LogMessage(MSG_GEN_002, {"\t -c <pre-canned test>               : Select pre-canned test <pre-canned test>"});
    LogMessage(MSG_GEN_002, {"\t -D <card JSON file>                : Provide a card configuration JSON file <card JSON file> instead of -d/-c and -j"});
#ifdef USE_XRT
    LogMessage(MSG_GEN_002, {"\t -T <card BDF>                      : Select P2P target card BDF <card BDF>. When specified, only one BDF must be provided using -d"});
    LogMessage(MSG_GEN_002, {"\t -N <path>                          : Select path <path> used in P2P_NVME test case. When specified, only one BDF must be provided using -d"});
#endif
    LogMessage(MSG_GEN_002, {"\t -l <log directory>                 : Define the name of a directory <log directory> in which all log files will be stored"});
    LogMessage(MSG_GEN_002, {"\t -L                                 : Disable logging directory generation"});
    LogMessage(MSG_GEN_002, {"\t -F                                 : Disable the dynamic display mode"});
    LogMessage(MSG_GEN_002, {"\t -f                                 : When possible, force an operation"});
    LogMessage(MSG_GEN_002, {"\t -b <verbosity>                     : Select verbosity level <verbosity> in [" + std::to_string(MIN_VERBOSITY) + "," + std::to_string(MAX_VERBOSITY) + "]"});
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"For complete documentation, refer to UG1361"});
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
    LogMessage(MSG_GEN_002, {"Command line options -c and -j can be mixed to define a sequence of tests."});
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"Example of commands:"});
    LogMessage(MSG_GEN_002, {"\t - Single card / single test       : xbtest -d <card BDF> -c <pre-canned test>"});
    LogMessage(MSG_GEN_002, {"\t - Single card / multiple tests    : xbtest -d <card BDF> -c <pre-canned test 1> -j <test JSON file 2> -c <pre-canned test 3>"});
    LogMessage(MSG_GEN_002, {"\t - Multiple cards / same tests     : xbtest -d <card 0 BDF> -d <card 1 BDF> -c <pre-canned test 1> -j <test JSON file 2>"});
    LogMessage(MSG_GEN_002, {"\t - Multiple cards / different tests: xbtest -d <card 0 BDF> -d <card 1 BDF> -c <pre-canned test 01.1> -c <pre-canned test 01.2> \\"});
    LogMessage(MSG_GEN_002, {"\t                                            -d <card 2 BDF>                 -c <pre-canned test 2.1>  -c <pre-canned test 2.2> "});
    LogMessage(MSG_GEN_002, {"\t - Card configuration JSON file    : xbtest -D <card JSON file>"});
#ifdef USE_XRT
    LogMessage(MSG_GEN_002, {"\t - P2P card / card                 : xbtest -d <source card BDF> -T <P2P target card BDF> -c <p2p_card pre-canned test>"});
    LogMessage(MSG_GEN_002, {"\t - P2P card / NVMe                 : xbtest -d <card BDF>        -N <P2P NVME path>       -c <p2p_nvme pre-canned test>"});
#endif
    LogMessage(MSG_GEN_002, {""});
    if (this->m_map_lib_config.empty())
    {
        LogMessage(MSG_GEN_002, {"No xbtest HW design is currently installed."});
    }
    else
    {
        auto is_one_invalid = false;
        LogMessage(MSG_GEN_002, {"The following xbtest HW designs are currently installed:"});
        for (const auto & cfg : this->m_map_lib_config)
        {
            if (!(cfg.second.valid))
            {
                is_one_invalid = true;
                continue;
            }
            LogMessage(MSG_GEN_002, {"-" + cfg.second.lib_name});
            LogMessage(MSG_GEN_002, {"\t - Available pre-canned tests (see option -c):"});
            // LogMessage(MSG_GEN_002, {"\t\t * -c " + pad(ALL, ' ', 15, false) + " run all pre-canned tests listed below one after the other"}); // Commented feature
            for (const auto & pre_canned_test : cfg.second.pre_canned_test_list)
            {
                LogMessage(MSG_GEN_002, {"\t\t * -c " + pad(pre_canned_test, ' ', 15, false) + " (" + TEST_JSON + ": " + cfg.second.lib_path + "/test/" + pre_canned_test + ".json)"});
            }
        }
        if (is_one_invalid)
        {
            PrintInvalidLib();
        }
    }
    LogMessage(MSG_GEN_002, {""});
    LogMessage(MSG_GEN_002, {"xbtest HW design installation directories: "});
    for (const auto & lib : this->m_amd_xbtest_libs)
    {
        LogMessage(MSG_GEN_002, {"\t - " + lib});
    }
}

void CommandLineParserCommon::PrintVersion()
{
    LogMessage(MSG_GEN_003, {this->m_command_line.data[0] + " host code version " + XBTEST_VERSION_STR});
    LogMessage(MSG_GEN_003, {"\t - SW Build: " + std::to_string(XBTEST_BUILD_VERSION) + " on " + XBTEST_BUILD_DATE});
    #ifdef INTERNAL_RELEASE
    LogMessage(MSG_GEN_003, {"\t - Release : INTERNAL"});
    #endif

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
    LogMessage(MSG_GEN_003, {"\t - Built using XRT:"});
    LogMessage(MSG_GEN_003, {"\t\t * Version  : " + std::string(xrt_build_version)});
    LogMessage(MSG_GEN_003, {"\t\t * Branch   : " + std::string(xrt_build_version_branch)});
    LogMessage(MSG_GEN_003, {"\t\t * Hash     : " + std::string(xrt_build_version_hash)});
    LogMessage(MSG_GEN_003, {"\t\t * Hash date: " + std::string(xrt_build_version_hash_date)});
    LogMessage(MSG_GEN_003, {"\t\t * Date     : " + std::string(xrt_build_version_date_rfc)});
#endif

    LogMessage(MSG_GEN_003, {""});
    if (this->m_map_lib_config.empty())
    {
        LogMessage(MSG_GEN_003, {"No xbtest HW design is currently installed."});
    }
    else
    {
        LogMessage(MSG_GEN_003, {"The following xbtest HW designs are currently installed:"});
        auto is_one_invalid = false;
        for (const auto & cfg : this->m_map_lib_config)
        {
            if (!cfg.second.valid)
            {
                is_one_invalid = true;
                continue;
            }
            LogMessage(MSG_GEN_003, {"\t - " + cfg.second.lib_name});
            LogMessage(MSG_GEN_003, {"\t\t * Version: " + std::to_string(cfg.second.build_info.xbtest.version.major) + "." + std::to_string(cfg.second.build_info.xbtest.version.minor)});
            LogMessage(MSG_GEN_003, {"\t\t * Date   : " + cfg.second.build_info.xbtest.date});
            LogMessage(MSG_GEN_003, {"\t\t * UUID   : " + cfg.first});
            if (cfg.second.build_info.xbtest.internal_release)
            {
                LogMessage(MSG_GEN_003, {"\t\t * Release       : INTERNAL"});
            }
        }
        if (is_one_invalid)
        {
            PrintInvalidLib();
        }
    }
    LogMessage(MSG_GEN_003, {""});
    LogMessage(MSG_GEN_003, {"xbtest HW design installation directories: "});
    for (const auto & lib : this->m_amd_xbtest_libs)
    {
        LogMessage(MSG_GEN_003, {"\t - " + lib});
    }
    LogMessage(MSG_GEN_003, {""});
    LogMessage(MSG_GEN_003, {"For other xbtest version information, try xbtest -d <device BDF> -v"});
}

void CommandLineParserCommon::PrintInvalidLib()
{
    LogMessage(MSG_GEN_042, {""});
    LogMessage(MSG_GEN_042, {"The following invalid xbtest HW designs were found in the system:"});
    for (const auto & cfg : this->m_map_lib_config)
    {
        if (!(cfg.second.valid))
        {
            LogMessage(MSG_GEN_042, {"\t - " + cfg.second.lib_name + " (" + cfg.second.lib_path + ")"});
        }
    }
}

uint CommandLineParserCommon::CountOpts( const std::vector<std::string> & args, const char & char_opt )
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

    uint cnt = 0;
    std::string opt_string = ":" + std::string(1, char_opt) + ":";
    int test_option;

    optind = 0; // reinitialization of getopt()
    while ((test_option = getopt(argc, argv, opt_string.c_str())) != -1)
    {
        if (test_option == char_opt)
        {
            cnt++;
        }
    }
    optind = 1; // reset optind
    return cnt;
}

} // namespace
