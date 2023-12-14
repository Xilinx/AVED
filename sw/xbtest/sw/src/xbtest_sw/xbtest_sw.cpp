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

#include "xbtest.h"
#include "xbtestpackage.h"
#include "logging.h"
#include "xbtestswconfig.h"
#include "inputparser.h"
#include "testcase.h"
#ifdef USE_NO_DRIVER
#include "deviceinterface_nodrv.h"
#include "deviceinfoparser_nodrv.h"
#endif
#ifdef USE_AMI
#include "deviceinterface_ami.h"
#include "deviceinfoparser_ami.h"
#endif
#ifdef USE_XRT
#include "deviceinterface_ocl.h"
#include "deviceinfoparser_xrt.h"
#endif
#include "verifytest.h"
#include "mmiotest.h"
#include "dmatest.h"
#include "p2pcardtest.h"
#include "p2pnvmetest.h"
#include "powertest.h"
#include "memorytest.h"
#include "devicemgt.h"
#include "gtmactest.h"
#include "gtlpbktest.h"
#include "gtprbstest.h"
#include "multigtprbstest.h"
#include "timer.h"
#include "consolemgmtsw.h"
#include "gtmacmgmt.h"

namespace xbtest
{

void LogMessageSw ( Logging & log, const Message_t & message, const std::vector<std::string> & arg_list )
{
    log.LogMessage(LOG_HEADER_GENERAL, message, arg_list);
}
void LogMessageSw ( Logging & log, const Message_t & message )
{
    log.LogMessage(LOG_HEADER_GENERAL, message);
}

void StopThreadsSw ( ConsoleMgmtSw & console_mgmt, Timer & timer, std::thread & signal_thread, std::thread & console_mgmt_thread, std::thread & timer_thread, std::atomic<bool> & stop_signal_thread )
{
    stop_signal_thread = true;
    signal_thread.join();

    console_mgmt.Stop();
    console_mgmt_thread.join();

    timer.Stop();
    timer_thread.join();
}

// signal handling

void HandleSignalsSw( Logging * log, Timer * timer, std::atomic<bool> * abort, std::atomic<bool> * stop_signal_thread )
{
    while (!(*stop_signal_thread))
    {
        if (get_sig_abort() && !(*abort))
        {
            LogMessageSw(*log, MSG_GEN_012);
            *abort = true;
        }
        if (get_sig_terminate())
        {
            LogMessageSw(*log, MSG_GEN_035);
            std::_Exit(EXIT_FAILURE);
        }
        timer->WaitFor1sTick();
    }
}

bool DisplayTestResult          ( Logging & log, const std::string & base_name, const TestCaseThreadResult & test_result );
bool PrintJsonAndThreadResult   ( Logging & log, XbtestSwConfig & xbtest_sw_config, const std::vector<TestCase*> & tc_list, InputParser & input_parser, Tasks_Parameters_t & tasks_param, Testcases_Parameters_t & testcases_param );
uint GetNumTestCasesNotCompleted( const std::vector<TestCase*> & tc_list, const ThreadType & thread_type );
bool CheckTestCasesCompleted    ( Logging & log, std::atomic<bool> & abort, const std::vector<TestCase*> & tc_list );
void SignalAbortTestCases       ( Logging & log, const std::vector<TestCase*> & tc_list, const ThreadType & thread_type );
void CheckTestCasesAborted      ( Logging & log, const std::vector<TestCase*> & tc_list, const ThreadType & thread_type );
bool WaitEndOfTestCases         ( Logging & log, Timer & timer, std::atomic<bool> & abort, const std::vector<TestCase*> & tc_list, const ThreadType & thread_type );
bool DumpMessageDefinition      ( Logging & log, XbtestSwConfig & xbtest_sw_config, Tasks_Type_Parameters_t & debug );
bool CheckP2PCanRun             ( Logging & log, XbtestSwConfig & xbtest_sw_config, XbtestSwConfig & xbtest_sw_config_p2p, const bool & p2p_card_exists, const bool & p2p_nvme_exists );

void DisplayHeaderSw( Logging & log, XbtestSwConfig & xbtest_sw_config, std::vector<std::string> & cmd_line )
{
    LogMessageSw(log, MSG_GEN_039, {LOG_LINE_DELIMITER});
    LogMessageSw(log, MSG_DEBUG_SETUP, {"Command line: " + StrVectToStr(cmd_line, " ")});
    LogMessageSw(log, MSG_GEN_039, {"xbtest version: " + XBTEST_VERSION_STR});
    LogMessageSw(log, MSG_GEN_016, {"\t - SW Build    : " + std::to_string(XBTEST_BUILD_VERSION) + " on " + XBTEST_BUILD_DATE});
    #ifdef INTERNAL_RELEASE
    LogMessageSw(log, MSG_GEN_039, {"\t - Release     : INTERNAL"});
    #endif
    LogMessageSw(log, MSG_GEN_016, {"\t - Process ID  : " + std::to_string(getpid())});
    LogMessageSw(log, MSG_GEN_016, {LOG_LINE_DELIMITER});
    LogMessageSw(log, MSG_GEN_016, {"System: "});
    LogMessageSw(log, MSG_DEBUG_SETUP, {"\t - User          : " + xbtest_sw_config.GetSystemUsername()});
    LogMessageSw(log, MSG_GEN_016, {"\t - Name          : " + xbtest_sw_config.GetSystemSysname()});
    LogMessageSw(log, MSG_DEBUG_SETUP, {"\t - Node          : " + xbtest_sw_config.GetSystemNodename()});
    LogMessageSw(log, MSG_GEN_016, {"\t - Release       : " + xbtest_sw_config.GetSystemRelease()});
    LogMessageSw(log, MSG_GEN_016, {"\t - Version       : " + xbtest_sw_config.GetSystemVersion()});
    LogMessageSw(log, MSG_GEN_016, {"\t - Machine       : " + xbtest_sw_config.GetSystemMachine()});
#ifdef USE_NO_DRIVER
    LogMessageSw(log, MSG_GEN_039, {"\t - Running without driver"});
#endif
    if (!(xbtest_sw_config.GetHostInfoVersion().empty()))
    {
#ifdef USE_AMI
        LogMessageSw(log, MSG_GEN_039, {"\t - Running with AMI driver " + xbtest_sw_config.GetHostInfoVersion()});
#endif
#ifdef USE_XRT
        LogMessageSw(log, MSG_GEN_039, {"\t - Running with XRT " + xbtest_sw_config.GetHostInfoVersion()});
#endif
    }
    LogMessageSw(log, MSG_GEN_016, {LOG_LINE_DELIMITER});
    LogMessageSw(log, MSG_GEN_039, {"Start of xbtest session at: " + xbtest_sw_config.GetSessionStartTime()});
    LogMessageSw(log, MSG_GEN_039, {LOG_LINE_DELIMITER});
}

void DisplayFooterSw( Logging & log, XbtestSwConfig & xbtest_sw_config, std::vector<std::string> & cmd_line )
{
    xbtest_sw_config.SetSessionTime(false);
    LogMessageSw(log, MSG_GEN_040, {LOG_LINE_SUMMARY});
    LogMessageSw(log, MSG_GEN_017, {"XBTEST: "});
    LogMessageSw(log, MSG_GEN_017, {"\t - Version     : " + XBTEST_VERSION_STR});
    LogMessageSw(log, MSG_GEN_017, {"\t - SW Build    : " + std::to_string(XBTEST_BUILD_VERSION) + " on " + XBTEST_BUILD_DATE});
#ifdef INTERNAL_RELEASE
    LogMessageSw(log, MSG_GEN_017, {"\t - Release     : INTERNAL"});
#endif
    LogMessageSw(log, MSG_GEN_017, {"\t - Process ID  : " + std::to_string(getpid())});
    LogMessageSw(log, MSG_GEN_017, {"\t - Command line: " + StrVectToStr(cmd_line, " ")});
    LogMessageSw(log, MSG_GEN_017, {LOG_LINE_DELIMITER});
    LogMessageSw(log, MSG_GEN_040, {"End of xbtest session at: " + xbtest_sw_config.GetSessionEndTime()});
    LogMessageSw(log, MSG_GEN_017, {LOG_LINE_DELIMITER});
}

bool CheckDriverAccessSw( Logging & log, std::atomic<bool> & abort )
{
    DeviceInfoParser device_info_parser_0(&log, "0", &abort); // Use first device found
    return device_info_parser_0.CheckDriverAccess();
}

// run_core() use argc/argv to allow building only "test sw" but might be simplified in future

int run_core ( int argc, char** argv )
{
    set_multitest_mode(false);
    return run_core_inner(argc, argv);
}

int run_core_inner ( int argc, char** argv )
{
    // Setup for signal handler
    std::atomic<bool> abort;
    abort = false;
    set_sig_abort(false);
    set_sig_terminate(false);

    // Logging
    Logging log(&abort, XBTEST_SW); // get instance of log

    // Timer
    Timer timer(&log, Timer::RES_333ms);

    // xbtest SW configuration
    XbtestSwConfig xbtest_sw_config    (&log, &abort, "main");
    XbtestSwConfig xbtest_sw_config_p2p(&log, &abort, "p2p");

    // Console Mgmt
    ConsoleMgmtSw console_mgmt(&log, &timer, &xbtest_sw_config);
    log.SetUseConsole(false);
    if (get_multitest_mode())
    {
        console_mgmt.DisableOutput();
    }

    // Threads
    std::atomic<bool>   stop_signal_thread;
    std::thread         signal_thread;
    std::thread         timer_thread;
    std::thread         console_mgmt_thread;

    std::vector<std::string> cmd_line;
    cmd_line.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        cmd_line.emplace_back(std::string(argv[i]));
    }

    try
    {
        // xbtest_failures levels:
        // - after each level propagate failure to next levels
        // - summary is printed depending on level
        enum FailureLevel {
            LEVEL_0             = 0,    // Failure before Test json parsing
            LEVEL_1             = 1,    // Failure after Test json parsing and before test run
            LEVEL_2             = 2,    // Failure after test run / Global failures
            MAX_FAILURE_LEVEL   = 3,    // To keep with max enum value
        };
        bool xbtest_failures[MAX_FAILURE_LEVEL] = {RET_SUCCESS, RET_SUCCESS, RET_SUCCESS};

        std::vector<TestCase*> tc_list;

        // Logging
        if (log.GetMessagesParserRet() == RET_FAILURE) // Check central message definition passed
        {
            return EXIT_FAILURE;
        }

        // Threads
        stop_signal_thread  = false;
        signal_thread       = std::thread(&HandleSignalsSw, &log, &timer, &abort, &stop_signal_thread);
        timer_thread        = std::thread(&Timer::RunTask, &timer);
        console_mgmt_thread = std::thread(&ConsoleMgmtSw::ConsoleTask, &console_mgmt);

        // Parse command line options and arguments
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            // Do not exit on command line failure
            auto command_line_ret = xbtest_sw_config.ParseCommandLine(argc, argv);
            if (command_line_ret == COMMAND_LINE_EXIT_SUCCESS)
            {
                LogMessageSw(log, MSG_GEN_007);
                StopThreadsSw(console_mgmt, timer, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread);
                return EXIT_SUCCESS;
            }
            if (command_line_ret == COMMAND_LINE_RET_FAILURE)
            {
                LogMessageSw(log, MSG_GEN_009);
                xbtest_failures[LEVEL_0] = RET_FAILURE;
            }
        }

        // Check driver
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            if (CheckDriverAccessSw(log, abort) == RET_FAILURE)
            {
                xbtest_failures[LEVEL_0] = RET_FAILURE;
            }
        }

        // Set an xbtest-sw config for the P2P target card
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_sw_config_p2p.SetCommandLine(xbtest_sw_config.GetCommandLine());
            }
        }

        // Parse xbutil dump without -d to get some system info
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_0] |= xbtest_sw_config.ParseDeviceInfoWithoutBdf();
        }

        // Display xbtest log header: xbtest version, command line, System and XRT info...
        DisplayHeaderSw(log, xbtest_sw_config, cmd_line);

        //Create device interface
        //  - will create different type depending on the compile flags
        //  - DeviceInterface represents a class derived from the base DeviceInterface_Base abstract class
        DeviceInterface device(&log, &xbtest_sw_config, &xbtest_sw_config_p2p, &abort, false);
        DeviceInterface device_p2p(&log, &xbtest_sw_config, &xbtest_sw_config_p2p, &abort, true);

        // Select device
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_0] = device.SelectDevice();
        }
        // Select p2p target device

        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_failures[LEVEL_0] = device_p2p.SelectDevice();
            }
        }

        // Generate the actual configuration after command line is parsed, before HW design is downloaded
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GenerateConfigPreXclbinLoad() == RET_FAILURE) // before HW design in loaded
            {
                LogMessageSw(log, MSG_ITF_101, {xbtest_sw_config.GetBdf()});
                xbtest_failures[LEVEL_0] = RET_FAILURE;
            }
        }
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                if (xbtest_sw_config_p2p.GenerateConfigPreXclbinLoad() == RET_FAILURE) // before HW design in loaded
                {
                    LogMessageSw(log, MSG_ITF_101, {xbtest_sw_config_p2p.GetBdf()});
                    xbtest_failures[LEVEL_0] = RET_FAILURE;
                }
            }
        }

        // Test JSON parser
        InputParser input_parser(&log, &xbtest_sw_config, &abort);

        // Display test JSON guide (based on "-g" option)
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            auto guide = xbtest_sw_config.GetCommandLineGuide();
            if (guide.exists)
            {
                auto guide_ret = input_parser.PrintGuide(guide.value);
                StopThreadsSw(console_mgmt, timer, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread);
                if (guide_ret == RET_SUCCESS)
                {
                    return EXIT_SUCCESS;
                }
                return EXIT_FAILURE;
            }
        }

        // Test JSON parameters can exists only if the xbtest HW IP exists in HW design using xbtest_sw_config (xbtest HW IPs are detected before HW design is loaded)
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_0] |= input_parser.Parse();
        }

        Tasks_Parameters_t tasks_param;
        Testcases_Parameters_t testcases_param;

        // Parameters get reset value at the start of input_parser.Parse()
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            tasks_param     = input_parser.GetTasksParameters();
            testcases_param = input_parser.GetTestcasesParameters();
        }

        // Check is new sensor are defined in test JSON file
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_0] |= xbtest_sw_config.SensorOverrideJson(tasks_param.device_mgmt);
        }

        // Display actual xbtest SW configuration, except parameter computed after HW design is loaded (actual memory sizes)
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            xbtest_sw_config.PrintActualConfig();

            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_sw_config_p2p.PrintActualConfig();
            }
        }

        // Perform some of the tasks (no thread), need to be done after test JSON parsing
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_0] |= DumpMessageDefinition(log, xbtest_sw_config, tasks_param.debug);
        }
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            // No test performed when the following parameters are defined
            if (FindJsonParam(tasks_param.debug.param, GET_MSG_DEF_MEMBER) != tasks_param.debug.param.end())
            {
                StopThreadsSw(console_mgmt, timer, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread);
                return EXIT_SUCCESS;
            }
        }

        // console_mgmt exits start-up phase when the command line are parsed
        if ((!abort) && (xbtest_failures[LEVEL_0] == RET_SUCCESS))
        {
            console_mgmt.SetUp();
            console_mgmt.Start();
        }

        // Propagate failure to next level
        for (uint i = LEVEL_1; i < MAX_FAILURE_LEVEL; i++)
        {
            xbtest_failures[i] |= xbtest_failures[LEVEL_0];
        }

        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_1] = CheckP2PCanRun(log, xbtest_sw_config, xbtest_sw_config_p2p, testcases_param.p2p_card.exists, testcases_param.p2p_nvme.exists);
        }

        // Setup device
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_1] = device.SetupDevice();
        }
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_failures[LEVEL_1] = device_p2p.SetupDevice();
            }
        }

        // Check download time, do no abort in case of error
        auto download_time_chk = RET_SUCCESS;
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            download_time_chk |= device.CheckXclbinDownloadTime();
        }
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                download_time_chk |= device_p2p.CheckXclbinDownloadTime();
            }
        }

        // Once HW design is downloaded, get other device info and complete the configuration metadata
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_1] = xbtest_sw_config.ParseDeviceInfoPostXclbinDownload();
        }
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_failures[LEVEL_1] = xbtest_sw_config_p2p.ParseDeviceInfoPostXclbinDownload();
            }
        }

        // Update the configuration metadata
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            xbtest_sw_config.PrintDeviceInfo();
            if (xbtest_sw_config.GenerateConfigPostXclbinLoad() == RET_FAILURE)
            {
                LogMessageSw(log, MSG_ITF_101, {xbtest_sw_config.GetBdf()});
                xbtest_failures[LEVEL_1] = RET_FAILURE;
            }
        }
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_sw_config_p2p.PrintDeviceInfo();
                if (xbtest_sw_config_p2p.GenerateConfigPostXclbinLoad() == RET_FAILURE)
                {
                    LogMessageSw(log, MSG_ITF_101, {xbtest_sw_config_p2p.GetBdf()});
                    xbtest_failures[LEVEL_1] = RET_FAILURE;
                }
            }
        }

        // Check some design info
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_1] = device.CheckDeviceInfo();
        }
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists)
            {
                xbtest_failures[LEVEL_1] = device_p2p.CheckDeviceInfo();
            }
        }

        // Check test can run
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (testcases_param.memory.exists)
            {
                // Run 1 thread of memory test for each Memory xbtest HW IP
                // The thread is run only if the memory type is present in the test JSON
                for (const auto & memory : xbtest_sw_config.GetMemoryDefinitions())
                {
                    if (testcases_param.memory.memory_config[memory.idx].exists && xbtest_sw_config.IsOneMemCuSkipped(memory.name))
                    {
                        LogMessageSw(log, MSG_GEN_038, {memory.name});
                        xbtest_failures[LEVEL_1] = RET_FAILURE;
                        break;
                    }
                }
            }
        }

        // MMIO testcase
        auto mmio_internal_abort = false;
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
        MmioTest  mmio_test(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, testcases_param.mmio);
        TestCase mmio_tc(&log, THREAD_TYPE_TESTCASE, MMIO_MEMBER.name, "", static_cast<TestInterface *>(&mmio_test));

        if ((testcases_param.mmio.exists) && (!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            mmio_tc.StartThread();
            tc_list.emplace_back(&mmio_tc);
            mmio_internal_abort = WaitEndOfTestCases(log, timer, abort, tc_list, THREAD_TYPE_TESTCASE);
        }
#endif
        if (mmio_internal_abort)
        {
            abort = false; // Overwrite abort generated by DMAtest for verify task
        }

        // DMA testcase
        auto dma_internal_abort = false;
#ifdef USE_XRT
        DMATest  dma_test(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, testcases_param.dma);
        TestCase dma_tc(&log, THREAD_TYPE_TESTCASE, DMA_MEMBER.name, "", static_cast<TestInterface *>(&dma_test));
        if ((testcases_param.dma.exists) && (!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            dma_tc.StartThread();
            tc_list.emplace_back(&dma_tc);
            dma_internal_abort = WaitEndOfTestCases(log, timer, abort, tc_list, THREAD_TYPE_TESTCASE);
        }
#endif
        if (dma_internal_abort)
        {
            abort = false; // Overwrite abort generated by DMAtest for verify task
        }

        // p2p_card testcase
        auto p2p_card_internal_abort = false;
#ifdef USE_XRT
        P2PCardTest p2p_card_test(&log, &xbtest_sw_config, &xbtest_sw_config_p2p, &console_mgmt, &timer, &device, testcases_param.p2p_card);
        TestCase    p2p_card_tc(&log, THREAD_TYPE_TESTCASE, P2P_CARD_MEMBER.name, "", static_cast<TestInterface *>(&p2p_card_test));
        if ((testcases_param.p2p_card.exists) && (!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            p2p_card_tc.StartThread();
            tc_list.emplace_back(&p2p_card_tc);
            p2p_card_internal_abort = WaitEndOfTestCases(log, timer, abort, tc_list, THREAD_TYPE_TESTCASE);
        }
#endif
        if (p2p_card_internal_abort)
        {
            abort = false; // Overwrite abort generated by P2PCardTest for verify task
        }

        // p2p_nvme testcase
        auto p2p_nvme_internal_abort = false;
#ifdef USE_XRT
        P2PNvmeTest p2p_nvme_test(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, testcases_param.p2p_nvme);
        TestCase    p2p_nvme_tc(&log, THREAD_TYPE_TESTCASE, P2P_NVME_MEMBER.name, "", static_cast<TestInterface *>(&p2p_nvme_test));
        if ((testcases_param.p2p_nvme.exists) && (!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            p2p_nvme_tc.StartThread();
            tc_list.emplace_back(&p2p_nvme_tc);
            p2p_nvme_internal_abort = WaitEndOfTestCases(log, timer, abort, tc_list, THREAD_TYPE_TESTCASE);
        }
#endif
        if (p2p_nvme_internal_abort)
        {
            abort = false; // Overwrite abort generated by P2PCardTest for verify task
        }

        // Create kernels, create buffers, set args, read build info
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            xbtest_failures[LEVEL_1] = device.SetupComputeUnits();
        }

        // device_mgt task, don't check if present in json, run anyway
        // Note: device_mgt not designed as singleton but other test might call this test directly, so this object must be created before the other
        DeviceMgt device_mgt(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, tasks_param.device_mgmt);
        TestCase  device_mgt_tc(&log, THREAD_TYPE_TASK, DEVICE_MGMT_MEMBER.name, "", static_cast<TestInterface *>(&device_mgt));

        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            device_mgt_tc.StartThread();
            tc_list.emplace_back(&device_mgt_tc);
        }

        // Verify testcase
        //  3 parts test concept:
        //    1) PreTestChecks: always executed (equivalent to PreSetup of other testcase)
        //    2) Test thread: as per other testcase but (so far) only doing resetwatchog (and empty PreSetup/PostTeardown)
        //    3) PostTestChecks: always executed (equivalent to PostTeardown of other testcase)
        VerifyTest verify_test(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, &device_mgt);
        TestCase   verify_tc(&log, THREAD_TYPE_TASK, VERIFY_STR, "", static_cast<TestInterface *>(&verify_test));

        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            //always executed

            xbtest_failures[LEVEL_1] |= verify_test.PreTestChecks();

            // Only launch thread if all pretest were successful
            if ( (!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS) )
            {
                verify_tc.StartThread();
                tc_list.emplace_back(&verify_tc);
            }
        }
        if (mmio_internal_abort || dma_internal_abort || p2p_card_internal_abort || p2p_nvme_internal_abort)
        {
            abort = true; // Revert overwrite abort generated by MmioTest or DMAtest or P2PCardTest or P2PNvmeTest for verify
        }

        // Power testcase
        PowerTest power_test(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, &device_mgt, testcases_param.power);
        TestCase power_tc(&log, THREAD_TYPE_TESTCASE, POWER_MEMBER.name, "", static_cast<TestInterface *>(&power_test));
        if ((testcases_param.power.exists) && (!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            power_tc.StartThread();
            tc_list.emplace_back(&power_tc);
        }

        // GT MAC mgmt
        GTMACMgmt gt_mac_mgmt(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, tasks_param.gtmacmgmt, testcases_param.gt_mac);
        TestCase  gtmacmgmt_tc(&log, THREAD_TYPE_TASK, GTMACMGMT_MEMBER.name, "", static_cast<TestInterface *>(&gt_mac_mgmt));
        auto gt_mac_mgmt_exists = false;
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            for (uint cu_idx = 0; cu_idx < device.GetNumGtmacCu(); cu_idx++)
            {
                auto gt_index = device.GetGtmacCuGTIndex(cu_idx);
                if (testcases_param.gt_mac[gt_index].exists) // Set up GT MAC mgmt if at least one GT testcase was provided
                {
                    gt_mac_mgmt_exists = true;
                    xbtest_failures[LEVEL_1] |= gt_mac_mgmt.GenConfig(); // Just generate the configuration for GT MAC but do not start the thread now
                    break;
                }
            }
        }
        // GT MAC testcase
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            // test is existing, so now launch the thread
            if (gt_mac_mgmt_exists)
            {
                gtmacmgmt_tc.StartThread();
                tc_list.emplace_back(&gtmacmgmt_tc);
            }

            for (uint cu_idx = 0; cu_idx < device.GetNumGtmacCu(); cu_idx++)
            {
                auto gt_index = device.GetGtmacCuGTIndex(cu_idx); // Note cu_idx is the index of the xbtest HW IP in the array != from gt_index
                if (testcases_param.gt_mac[gt_index].exists)
                {
                    auto * gt_mac_test = new GTMACTest(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, &gt_mac_mgmt, &device_mgt, testcases_param.gt_mac[gt_index], cu_idx);
                    auto * gt_mac_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, GT_MAC_MEMBER.name + " " + std::to_string(gt_index), "", static_cast<TestInterface *>(gt_mac_test));
                    gt_mac_tc->StartThread();
                    tc_list.emplace_back(gt_mac_tc);
                    std::this_thread::sleep_for(std::chrono::milliseconds(250)); // delay gt thread so with identical test_sequence over multiple xbtest HW IPs, results are not interleaved
                }
            }
        }
        // GT LPBK testcase
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            for (uint cu_idx = 0; cu_idx < device.GetNumGtlpbkCu(); cu_idx++)
            {
                auto gt_index = device.GetGtlpbkCuGTIndex(cu_idx); // Note cu_idx is the index of the xbtest HW IP in the array and is different from gt_index
                if (testcases_param.gt_lpbk[gt_index].exists)
                {
                    auto * gt_lpbk_test = new GTLpbkTest(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, &device_mgt, testcases_param.gt_lpbk[gt_index], cu_idx);
                    auto * gt_lpbk_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, GT_LPBK_MEMBER.name + " " + std::to_string(gt_index), "", static_cast<TestInterface *>(gt_lpbk_test));
                    gt_lpbk_tc->StartThread();
                    tc_list.emplace_back(gt_lpbk_tc);
                }
            }
        }
        // GT PRBS testcase
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            for (uint cu_idx = 0; cu_idx < device.GetNumGtPrbsCu(); cu_idx++)
            {
                auto gt_index = device.GetGtPrbsCuGTIndex(cu_idx); // Note cu_idx is the index of the xbtest HW IP in the array and is different from gt_index
                if (testcases_param.gt_prbs[gt_index].exists)
                {
                    auto * gt_prbs_test = new GTPrbsTest(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, &device_mgt, testcases_param.gt_prbs[gt_index], cu_idx);
                    auto * gt_prbs_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, GT_PRBS_MEMBER.name + " " + std::to_string(gt_index), "", static_cast<TestInterface *>(gt_prbs_test));
                    gt_prbs_tc->StartThread();
                    tc_list.emplace_back(gt_prbs_tc);
                }
            }
        }
        // GTF PRBS testcase
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            for (uint cu_idx = 0; cu_idx < device.GetNumGtfPrbsCu(); cu_idx++)
            {
                auto gtf_indexes = device.GetGtfPrbsCuGTFIndexes(cu_idx); // Note cu_idx is the index of the xbtest HW IP in the array and is different from gt_index
                for (const auto & gt_index : gtf_indexes)
                {
                    if (testcases_param.gtf_prbs[gt_index].exists)
                    {
                        auto * gtf_prbs_test = new MultiGTPrbsTest(&log, &xbtest_sw_config, GTF, &console_mgmt, &timer, &device, &device_mgt, testcases_param.gtf_prbs[gt_index], cu_idx, gt_index);
                        auto * gtf_prbs_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, GTF_PRBS_MEMBER.name + " " + std::to_string(gt_index), "", static_cast<TestInterface *>(gtf_prbs_test));
                        gtf_prbs_tc->StartThread();
                        tc_list.emplace_back(gtf_prbs_tc);
                    }
                }
            }
        }
        // GTM PRBS testcase
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            for (uint cu_idx = 0; cu_idx < device.GetNumGtmPrbsCu(); cu_idx++)
            {
                auto gtm_indexes = device.GetGtmPrbsCuGTMIndexes(cu_idx); // Note cu_idx is the index of the xbtest HW IP in the array and is different from gt_index
                for (const auto & gt_index : gtm_indexes)
                {
                    if (testcases_param.gtm_prbs[gt_index].exists)
                    {
                        auto * gtm_prbs_test = new MultiGTPrbsTest(&log, &xbtest_sw_config, GTM, &console_mgmt, &timer, &device, &device_mgt, testcases_param.gtm_prbs[gt_index], cu_idx, gt_index);
                        auto * gtm_prbs_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, GTM_PRBS_MEMBER.name + " " + std::to_string(gt_index), "", static_cast<TestInterface *>(gtm_prbs_test));
                        gtm_prbs_tc->StartThread();
                        tc_list.emplace_back(gtm_prbs_tc);
                    }
                }
            }
        }
        // GTYP PRBS testcase
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            for (uint cu_idx = 0; cu_idx < device.GetNumGtypPrbsCu(); cu_idx++)
            {
                auto gtyp_indexes = device.GetGtypPrbsCuGTYPIndexes(cu_idx); // Note cu_idx is the index of the xbtest HW IP in the array and is different from gt_index
                for (const auto & gt_index : gtyp_indexes)
                {
                    if (testcases_param.gtyp_prbs[gt_index].exists)
                    {
                        auto * gtyp_prbs_test = new MultiGTPrbsTest(&log, &xbtest_sw_config, GTYP, &console_mgmt, &timer, &device, &device_mgt, testcases_param.gtyp_prbs[gt_index], cu_idx, gt_index);
                        auto * gtyp_prbs_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, GTYP_PRBS_MEMBER.name + " " + std::to_string(gt_index), "", static_cast<TestInterface *>(gtyp_prbs_test));
                        gtyp_prbs_tc->StartThread();
                        tc_list.emplace_back(gtyp_prbs_tc);
                    }
                }
            }
        }
        // Memory Test
        if ((!abort) && (xbtest_failures[LEVEL_1] == RET_SUCCESS))
        {
            if (testcases_param.memory.exists)
            {
                // Run 1 thread of memory test for each Memory xbtest HW IP
                // The thread is run only if the memory type is present in the test JSON
                for (const auto & memory : xbtest_sw_config.GetMemoryDefinitions())
                {
                    if (testcases_param.memory.memory_config[memory.idx].exists)
                    {
                        for (uint cu_idx = 0; cu_idx < memory.num_cu; cu_idx++)
                        {
                            auto type_name = MEMORY_MEMBER.name + " " + memory.name;
                            std::string thread_name;
                            if (memory.type == SINGLE_CHANNEL)
                            {
                                auto  cu_name = device.GetMemoryCuName(memory.type, memory.name, cu_idx);
                                thread_name   = xbtest_sw_config.GetCUConnectionVppSptag(cu_name, 0);
                            }
                            auto * memory_test = new MemoryTest(&log, &xbtest_sw_config, &console_mgmt, &timer, &device, &device_mgt, testcases_param.memory.memory_config[memory.idx], testcases_param.power.exists, memory.idx, cu_idx);
                            auto * memory_tc   = new TestCase(&log, THREAD_TYPE_TESTCASE, type_name, thread_name, static_cast<TestInterface *>(memory_test));
                            memory_tc->StartThread();
                            tc_list.emplace_back(memory_tc);
                        }
                    }
                }
            }
        }

        // Propagate failure to next level
        for (uint i = LEVEL_2; i < MAX_FAILURE_LEVEL; i++)
        {
            xbtest_failures[i] |= xbtest_failures[LEVEL_1];
        }

        // Wait all test completion and manage interrupt. Then stop and wait for the end of all tasks
        timer.WaitFor1sTick();

        // Wait end of test ONLY
        WaitEndOfTestCases(log, timer, abort, tc_list, THREAD_TYPE_TESTCASE);

        // stop tasks as task doesn't use on testcase complete to stop
        SignalAbortTestCases(log, tc_list, THREAD_TYPE_TASK);

        // Wait end of tasks
        WaitEndOfTestCases(log, timer, abort, tc_list, THREAD_TYPE_TASK);

        xbtest_failures[LEVEL_2] |= verify_test.GetPostTestChecksResult();

        // Check if a task has failed
        timer.WaitFor1sTick();
        for (const auto & tc : tc_list)
        {
            if (tc->GetTestCaseType() == THREAD_TYPE_TASK)
            {
                if (tc->GetThreadResult() != TestCaseThreadResult::TC_PASS)
                {
                    xbtest_failures[LEVEL_2] = RET_FAILURE;
                }
            }
        }
        xbtest_failures[LEVEL_2] |= abort; // Make sure abort is reported even if already detected when TC state is ABORT
        xbtest_failures[LEVEL_2] |= download_time_chk; // Make sure download time error is propagated

        console_mgmt.SetAllTcEnded();

        // Display xbtest log footer: xbtest version, command line.
        DisplayFooterSw(log, xbtest_sw_config, cmd_line);

        // Display test configuration and results
        if (xbtest_failures[LEVEL_0] == RET_SUCCESS) // Don't print testcases/tasks configuration at LEVEL_0 failures because it is before Test JSON parsing
        {
            input_parser.SetLogHeader(LOG_HEADER_GENERAL); // Overwrite the line header of the class to print parameters
            if (xbtest_failures[LEVEL_1] == RET_SUCCESS) // Don't print Device and Test params at LEVEL_0 failures because it is before Test are run
            {
                xbtest_failures[LEVEL_2] |= PrintJsonAndThreadResult(
                    log,
                    xbtest_sw_config,
                    tc_list,
                    input_parser,
                    tasks_param,
                    testcases_param
                );
            }
        }

        // Display logging statistic and info
        std::string msg_count_str;
        xbtest_failures[LEVEL_2] |= log.GetReportMsgCount(msg_count_str);
        LogMessageSw(log, MSG_GEN_040, {msg_count_str});

        // Report first error
        auto first_error = log.GetFirstError();
        if (!first_error.id.empty())
        {
            LogMessageSw(log, MSG_GEN_040, {"First error reported in run:"});
            LogMessageSw(log, MSG_GEN_040, {"\t- Message   : " +  first_error.msg});
            LogMessageSw(log, MSG_GEN_040, {"\t- Details   : " +  first_error.detail});
            LogMessageSw(log, MSG_GEN_040, {"\t- Resolution: " +  first_error.resolution});
            LogMessageSw(log, MSG_GEN_040, {"For more information on other messages, try command line option '-m'"});
            xbtest_failures[LEVEL_2] |= RET_FAILURE; // Make sure global result is failure is one error/failure is present in log
        }

        // Propagate failure to next level
        for (uint i = LEVEL_2; i < MAX_FAILURE_LEVEL; i++)
        {
            xbtest_failures[i] |= xbtest_failures[LEVEL_2];
        }

        // Global result
        LogMessageSw(log, MSG_GEN_040, {LOG_LINE_DELIMITER});
        if (xbtest_failures[MAX_FAILURE_LEVEL-1] == RET_SUCCESS)
        {
            LogMessageSw(log, MSG_GEN_024);
        }
        else
        {
            LogMessageSw(log, MSG_GEN_023);
        }
        LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DONE});

        StopThreadsSw(console_mgmt, timer, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread);

        if (xbtest_failures[MAX_FAILURE_LEVEL-1] == RET_SUCCESS)
        {
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    }
    catch (const std::exception& ex) // catch all thrown exceptions, should not occur
    {
        LogMessageSw(log, MSG_GEN_025, {std::string(ex.what())});
    }

    StopThreadsSw(console_mgmt, timer, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread);
    return EXIT_FAILURE;
}

bool DisplayTestResult( Logging & log, const std::string & base_name, const TestCaseThreadResult & test_result )
{
    auto ret = RET_FAILURE;
    switch (test_result)
    {
        default:
            LogMessageSw(log, MSG_GEN_019, {base_name});
            break;
        case TestCaseThreadResult::TC_ABORTED:
            LogMessageSw(log, MSG_GEN_020, {base_name});
            break;
        case TestCaseThreadResult::TC_FAIL:
            LogMessageSw(log, MSG_GEN_021, {base_name});
            break;
        case TestCaseThreadResult::TC_PASS:
            LogMessageSw(log, MSG_GEN_022, {base_name});
            ret = RET_SUCCESS;
            break;
    }
    return ret;
}

bool PrintJsonAndThreadResult( Logging & log, XbtestSwConfig & xbtest_sw_config, const std::vector<TestCase*> & tc_list, InputParser & input_parser, Tasks_Parameters_t & tasks_param, Testcases_Parameters_t & testcases_param )
{
    auto ret = RET_SUCCESS;
    std::vector<std::string> node_title;

    //Print device_mgmt task
    auto base_name = DEVICE_MGMT_MEMBER.name;
    std::vector<std::string> base_title = {TASKS_MEMBER.name, base_name};
    if (tasks_param.device_mgmt.exists)
    {
        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(base_title, " "), TASKS_DEVICE_MGMT_PARAMETERS_DEF, tasks_param.device_mgmt.param, false);
        LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
    }
    // Print debug task
    base_name = DEBUG_MEMBER.name;
    base_title = {TASKS_MEMBER.name, base_name};
    if (tasks_param.debug.exists)
    {
        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(base_title, " "), TASKS_DEBUG_PARAMETERS_DEF, tasks_param.debug.param, false);
        LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
    }
    // Print MMIO testcase
    base_name = MMIO_MEMBER.name;
    if (testcases_param.mmio.exists)
    {
        for (const auto & tc : tc_list)
        {
            if (tc->GetTypeName() != base_name)
            {
                continue;
            }
            node_title = {TESTCASES_MEMBER.name, base_name};

            // Print global parameter after lane parameter because the test sequence belong to it
            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), MMIO_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.mmio.global_config, false);

            // Print now results for tests that runs in a single thread
            input_parser.PrintJsonTestSeqAndResult(true, base_name, MMIO_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.mmio.global_config, tc->GetItResult());
            ret |= DisplayTestResult(log, base_name, tc->GetThreadResult());
            LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
        }
    }
    // Print DMA testcase
    base_name = DMA_MEMBER.name;
    if (testcases_param.dma.exists)
    {
        for (const auto & tc : tc_list)
        {
            if (tc->GetTypeName() != base_name)
            {
                continue;
            }
            node_title = {TESTCASES_MEMBER.name, base_name};

            // Print parameters except test_sequence
            node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
            for (auto & cfg : testcases_param.dma.memory_config)
            {
                if (cfg.second.exists)
                {
                    node_title.emplace_back(xbtest_sw_config.GetMemorySettings(cfg.first).name);
                    input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), DMA_MEMORY_TYPE_PARAMETERS_DEF, cfg.second.param, false);
                    node_title.pop_back();
                }
            }
            node_title.pop_back();
            // Print global parameter after lane parameter because the test sequence belong to it
            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), DMA_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.dma.global_config, false);

            // Print now results for tests that runs in a single thread
            input_parser.PrintJsonTestSeqAndResult(true, base_name, DMA_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.dma.global_config, tc->GetItResult());
            ret |= DisplayTestResult(log, base_name, tc->GetThreadResult());
            LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
        }
    }
    // Print p2p_card testcase
    base_name = P2P_CARD_MEMBER.name;
    if (testcases_param.p2p_card.exists)
    {
        for (const auto & tc : tc_list)
        {
            if (tc->GetTypeName() != base_name)
            {
                continue;
            }
            node_title = {TESTCASES_MEMBER.name, base_name};

            // Print parameters except test_sequence
            node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
            for (auto & cfg : testcases_param.p2p_card.memory_config)
            {
                if (cfg.second.exists)
                {
                    node_title.emplace_back(xbtest_sw_config.GetMemorySettings(cfg.first).name);
                    input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), P2P_CARD_MEMORY_TYPE_PARAMETERS_DEF, cfg.second.param, false);
                    node_title.pop_back();
                }
            }
            node_title.pop_back();
            // Print global parameter after lane parameter because the test sequence belong to it
            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.p2p_card.global_config, false);

            // Print now results for tests that runs in a single thread
            input_parser.PrintJsonTestSeqAndResult(true, base_name, P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.p2p_card.global_config, tc->GetItResult());
            ret |= DisplayTestResult(log, base_name, tc->GetThreadResult());
            LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
        }
    }
    // Print p2p_nvme testcase
    base_name = P2P_NVME_MEMBER.name;
    if (testcases_param.p2p_nvme.exists)
    {
        for (const auto & tc : tc_list)
        {
            if (tc->GetTypeName() != base_name)
            {
                continue;
            }
            node_title = {TESTCASES_MEMBER.name, base_name};

            // Print parameters except test_sequence
            node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
            for (auto & cfg : testcases_param.p2p_nvme.memory_config)
            {
                if (cfg.second.exists)
                {
                    node_title.emplace_back(xbtest_sw_config.GetMemorySettings(cfg.first).name);
                    input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), P2P_NVME_MEMORY_TYPE_PARAMETERS_DEF, cfg.second.param, false);
                    node_title.pop_back();
                }
            }
            node_title.pop_back();
            // Print global parameter after lane parameter because the test sequence belong to it
            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.p2p_nvme.global_config, false);

            // Print now results for tests that runs in a single thread
            input_parser.PrintJsonTestSeqAndResult(true, base_name, P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF, testcases_param.p2p_nvme.global_config, tc->GetItResult());
            ret |= DisplayTestResult(log, base_name, tc->GetThreadResult());
            LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
        }
    }

    // Print memory testcase
    base_name = MEMORY_MEMBER.name;
    if (testcases_param.memory.exists)
    {
        for (const auto & memory : xbtest_sw_config.GetMemoryDefinitions())
        {
            auto first_test_found = false; // when multiple single channel are running, only display once the parameters
            if (testcases_param.memory.memory_config[memory.idx].exists)
            {
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                std::map<std::string, TestCaseThreadResult>   test_result_map;
                auto type_name = base_name + " " + memory.name;
                for (const auto & tc : tc_list)
                {
                    if (tc->GetTypeName() != type_name)
                    {
                        continue;
                    }
                    if (!first_test_found)
                    {
                        node_title = {TESTCASES_MEMBER.name, base_name, memory.name};
                        // Print parameter except test_sequence
                        // Even if test sequence is overwritten for a channel/tag, the test_it_results are only displayed in the global_config test_sequence
                        if (memory.type == SINGLE_CHANNEL)
                        {
                            node_title.emplace_back(MEMORY_TAG_CONFIG_MEMBER.name);
                            for (const auto & cfg : testcases_param.memory.memory_config[memory.idx].memory_tag_config)
                            {
                                auto tag        = cfg.first;
                                auto tag_config = cfg.second;
                                node_title.emplace_back(tag);
                                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), input_parser.GetMemoryTagChanParametersDef(memory.name), tag_config, true);

                                for (const auto & cfg_pattern : testcases_param.memory.memory_config[memory.idx].memory_tag_config_pattern)
                                {
                                    if (cfg_pattern.first == tag)
                                    {
                                        auto config = cfg_pattern.second;
                                        input_parser.PrintJsonParameters(true, base_name, PATTERN_MEMBER.name, MEMORY_PATTERN_DEF, config, true, 1);
                                    }
                                }

                                node_title.pop_back();
                            }
                        }
                        else
                        {
                            node_title.emplace_back(MEMORY_CHANNEL_CONFIG_MEMBER.name);
                            for (const auto & cfg : testcases_param.memory.memory_config[memory.idx].memory_chan_config)
                            {
                                auto ch_idx    = cfg.first;
                                auto ch_config = cfg.second;
                                node_title.emplace_back(std::to_string(ch_idx));
                                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), input_parser.GetMemoryTagChanParametersDef(memory.name), ch_config, true);


                                for (const auto & cfg_pattern : testcases_param.memory.memory_config[memory.idx].memory_chan_config_pattern)
                                {
                                    if (cfg_pattern.first == ch_idx)
                                    {
                                        auto config = cfg_pattern.second;
                                        input_parser.PrintJsonParameters(true, base_name, PATTERN_MEMBER.name, MEMORY_PATTERN_DEF, config, true, 1);
                                    }
                                }

                                node_title.pop_back();
                            }
                        }
                        node_title.pop_back();
                        // Print global parameter after lane parameter because the test sequence belong to it
                        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), input_parser.GetMemoryGlobalParametersDef(memory.name), testcases_param.memory.memory_config[memory.idx].global_config, false);
                        if (!testcases_param.memory.memory_config[memory.idx].global_config_pattern.empty())
                        {
                            input_parser.PrintJsonParameters(true, base_name, PATTERN_MEMBER.name, MEMORY_PATTERN_DEF, testcases_param.memory.memory_config[memory.idx].global_config_pattern, false, 1);
                        }
                    }
                    first_test_found = true;

                    auto test_it_results    = tc->GetItResult();
                    auto test_result        = tc->GetThreadResult();

                    if (memory.type == SINGLE_CHANNEL)
                    {
                        test_it_results_map.emplace(tc->GetThreadName(), test_it_results);
                        test_result_map.emplace(tc->GetThreadID(), test_result);
                    }
                    else
                    {
                        // Print now results for tests that runs in a single thread
                        input_parser.PrintJsonTestSeqAndResult(true, base_name, input_parser.GetMemoryGlobalParametersDef(memory.name), testcases_param.memory.memory_config[memory.idx].global_config, test_it_results);
                        ret |= DisplayTestResult(log, type_name, test_result);
                        LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
                    }
                }
                if ((memory.type == SINGLE_CHANNEL) && (first_test_found))
                {
                    // Print now results for tests that runs in multiple threads
                    input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, input_parser.GetMemoryGlobalParametersDef(memory.name), testcases_param.memory.memory_config[memory.idx].global_config, test_it_results_map);
                    for (const auto & test_result_thread : test_result_map)
                    {
                        ret |= DisplayTestResult(log, test_result_thread.first, test_result_thread.second);
                    }
                    LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
                }
            }
        }
    }

    // Print power testcase
    base_name = POWER_MEMBER.name;
    if (testcases_param.power.exists)
    {
        for (const auto & tc : tc_list)
        {
            if (tc->GetTypeName() != base_name)
            {
                continue;
            }
            node_title = {TESTCASES_MEMBER.name, base_name};

            // Print global parameter after lane parameter because the test sequence belong to it
            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), input_parser.GetPowerGlobalParametersDef(), testcases_param.power.global_config, false);

            // Print now results for tests that runs in a single thread
            input_parser.PrintJsonTestSeqAndResult(true, base_name, input_parser.GetPowerGlobalParametersDef(), testcases_param.power.global_config, tc->GetItResult());
            ret |= DisplayTestResult(log, base_name, tc->GetThreadResult());
            LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
        }
    }

    // Print GT MAC testcase
    base_name = GT_MAC_MEMBER.name;
    for (const auto & gt_index : xbtest_sw_config.GetAvailableGTMACs())
    {
        auto type_name = base_name + " " + std::to_string(gt_index);
        if (testcases_param.gt_mac[gt_index].exists)
        {
            for (const auto & tc : tc_list)
            {
                if ((tc->GetTypeName() != type_name))
                {
                    continue;
                }
                node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};
                // Print parameter except test_sequence
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    if (testcases_param.gt_mac[gt_index].lane_config[lane_idx].exists)
                    {
                        node_title.emplace_back(std::to_string(lane_idx));
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_MAC_LANE_CFG_PARAMETERS_DEF, testcases_param.gt_mac[gt_index].lane_config[lane_idx].param, false);
                        node_title.pop_back();
                    }
                }
                node_title.pop_back();
                // Print global parameter after lane parameter because the test sequence belong to it
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_MAC_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gt_mac[gt_index].global_config, false);

                // Print now results for tests that runs in a single thread, including test sequence result for each lane
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    test_it_results_map.emplace("lane[" + std::to_string(lane_idx) + "]", tc->GetItLaneResult(lane_idx));
                }

                // get the test sequence from the GT MAC xbtest HW IP as it has been overwritten (by GTMgmT) when GT MAC xbtest HW IP are paired
                auto gt_mac_test = reinterpret_cast<GTMACTest*>(tc->GetTestInterfaceBase());
                auto test_sequence = gt_mac_test->GetTestSequence();
                // insert it back into
                EraseJsonParam(testcases_param.gt_mac[gt_index].global_config, TEST_SEQUENCE_MEMBER);
                InsertJsonParam<std::vector<GTMAC_Test_Sequence_Parameters_t>>(testcases_param.gt_mac[gt_index].global_config, TEST_SEQUENCE_MEMBER, test_sequence);

                input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, GT_MAC_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gt_mac[gt_index].global_config, test_it_results_map);

                ret |= DisplayTestResult(log, type_name, tc->GetThreadResult());
                LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
            }
        }
    }

    // Print GT LPBK testcase
    base_name = GT_LPBK_MEMBER.name;
    for (const auto & gt_index : xbtest_sw_config.GetAvailableGTLpbks())
    {
        auto type_name = base_name + " " + std::to_string(gt_index);
        if (testcases_param.gt_lpbk[gt_index].exists)
        {
            for (const auto & tc : tc_list)
            {
                if ((tc->GetTypeName() != type_name))
                {
                    continue;
                }
                node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};
                // Print parameter except test_sequence
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    if (testcases_param.gt_lpbk[gt_index].lane_config[lane_idx].exists)
                    {
                        node_title.emplace_back(std::to_string(lane_idx));
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_LPBK_LANE_CFG_PARAMETERS_DEF, testcases_param.gt_lpbk[gt_index].lane_config[lane_idx].param, false);
                        node_title.pop_back();
                    }
                }
                node_title.pop_back();
                // Print global parameter after lane parameter because the test sequence belong to it
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gt_lpbk[gt_index].global_config, false);

                // Print now results for tests that runs in a single thread, including test sequence result for each lane
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    test_it_results_map.emplace("lane["+std::to_string(lane_idx)+"]", tc->GetItLaneResult(lane_idx));
                }
                input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gt_lpbk[gt_index].global_config, test_it_results_map);

                ret |= DisplayTestResult(log, type_name, tc->GetThreadResult());
                LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
            }
        }
    }
    // Print GT PRBS testcase
    base_name = GT_PRBS_MEMBER.name;
    for (const auto & gt_index : xbtest_sw_config.GetAvailableGTPrbss())
    {
        auto type_name = base_name + " " + std::to_string(gt_index);
        if (testcases_param.gt_prbs[gt_index].exists)
        {
            for (const auto & tc : tc_list)
            {
                if ((tc->GetTypeName() != type_name))
                {
                    continue;
                }
                node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};
                // Print parameter except test_sequence
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    if (testcases_param.gt_prbs[gt_index].lane_config[lane_idx].exists)
                    {
                        node_title.emplace_back(std::to_string(lane_idx));
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_PRBS_LANE_CFG_PARAMETERS_DEF, testcases_param.gt_prbs[gt_index].lane_config[lane_idx].param, false);
                        node_title.pop_back();
                    }
                }
                node_title.pop_back();
                // Print global parameter after lane parameter because the test sequence belong to it
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gt_prbs[gt_index].global_config, false);

                // Print now results for tests that runs in a single thread, including test sequence result for each lane
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    test_it_results_map.emplace("lane["+std::to_string(lane_idx)+"]", tc->GetItLaneResult(lane_idx));
                }
                input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gt_prbs[gt_index].global_config, test_it_results_map);

                ret |= DisplayTestResult(log, type_name, tc->GetThreadResult());
                LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
            }
        }
    }
    // Print GTF PRBS testcase
    base_name = GTF_PRBS_MEMBER.name;
    for (const auto & gt_index : xbtest_sw_config.GetAvailableGTFPrbss())
    {
        auto type_name = base_name + " " + std::to_string(gt_index);
        if (testcases_param.gtf_prbs[gt_index].exists)
        {
            for (const auto & tc : tc_list)
            {
                if ((tc->GetTypeName() != type_name))
                {
                    continue;
                }
                auto gtf_prbs_test = reinterpret_cast<MultiGTPrbsTest*>(tc->GetTestInterfaceBase());
                auto available_lane  = gtf_prbs_test->GetAvailableLane();

                node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};
                // Print parameter except test_sequence
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : available_lane)
                {
                    if (testcases_param.gtf_prbs[gt_index].lane_config[lane_idx].exists)
                    {
                        node_title.emplace_back(std::to_string(lane_idx));
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_PRBS_LANE_CFG_PARAMETERS_DEF, testcases_param.gtf_prbs[gt_index].lane_config[lane_idx].param, false);
                        node_title.pop_back();
                    }
                }
                node_title.pop_back();
                // Print global parameter after lane parameter because the test sequence belong to it
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gtf_prbs[gt_index].global_config, false);

                // Print now results for tests that runs in a single thread, including test sequence result for each lane
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                for (const auto & lane_idx : available_lane)
                {
                    test_it_results_map.emplace("lane["+std::to_string(lane_idx)+"]", tc->GetItLaneResult(lane_idx));
                }
                input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gtf_prbs[gt_index].global_config, test_it_results_map);

                ret |= DisplayTestResult(log, type_name, tc->GetThreadResult());
                LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
            }
        }
    }
    // Print GTM PRBS testcase
    base_name = GTM_PRBS_MEMBER.name;
    for (const auto & gt_index : xbtest_sw_config.GetAvailableGTMPrbss())
    {
        auto type_name = base_name + " " + std::to_string(gt_index);
        if (testcases_param.gtm_prbs[gt_index].exists)
        {
            for (const auto & tc : tc_list)
            {
                if ((tc->GetTypeName() != type_name))
                {
                    continue;
                }
                auto gtm_prbs_test = reinterpret_cast<MultiGTPrbsTest*>(tc->GetTestInterfaceBase());
                auto available_lane  = gtm_prbs_test->GetAvailableLane();

                node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};
                // Print parameter except test_sequence
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : available_lane)
                {
                    if (testcases_param.gtm_prbs[gt_index].lane_config[lane_idx].exists)
                    {
                        node_title.emplace_back(std::to_string(lane_idx));
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_PRBS_LANE_CFG_PARAMETERS_DEF, testcases_param.gtm_prbs[gt_index].lane_config[lane_idx].param, false);
                        node_title.pop_back();
                    }
                }
                node_title.pop_back();
                // Print global parameter after lane parameter because the test sequence belong to it
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gtm_prbs[gt_index].global_config, false);

                // Print now results for tests that runs in a single thread, including test sequence result for each lane
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                for (const auto & lane_idx : available_lane)
                {
                    test_it_results_map.emplace("lane["+std::to_string(lane_idx)+"]", tc->GetItLaneResult(lane_idx));
                }
                input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gtm_prbs[gt_index].global_config, test_it_results_map);

                ret |= DisplayTestResult(log, type_name, tc->GetThreadResult());
                LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
            }
        }
    }
    // Print GTYP PRBS testcase
    base_name = GTYP_PRBS_MEMBER.name;
    for (const auto & gt_index : xbtest_sw_config.GetAvailableGTYPPrbss())
    {
        auto type_name = base_name + " " + std::to_string(gt_index);
        if (testcases_param.gtyp_prbs[gt_index].exists)
        {
            for (const auto & tc : tc_list)
            {
                if ((tc->GetTypeName() != type_name))
                {
                    continue;
                }
                auto gtyp_prbs_test = reinterpret_cast<MultiGTPrbsTest*>(tc->GetTestInterfaceBase());
                auto available_lane   = gtyp_prbs_test->GetAvailableLane();

                node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};
                // Print parameter except test_sequence
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : available_lane)
                {
                    if (testcases_param.gtyp_prbs[gt_index].lane_config[lane_idx].exists)
                    {
                        node_title.emplace_back(std::to_string(lane_idx));
                        input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), GT_PRBS_LANE_CFG_PARAMETERS_DEF, testcases_param.gtyp_prbs[gt_index].lane_config[lane_idx].param, false);
                        node_title.pop_back();
                    }
                }
                node_title.pop_back();
                // Print global parameter after lane parameter because the test sequence belong to it
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                input_parser.PrintJsonParameters(true, base_name, StrVectToStr(node_title, " "), MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gtyp_prbs[gt_index].global_config, false);

                // Print now results for tests that runs in a single thread, including test sequence result for each lane
                std::map<std::string, std::vector<TestIterationResult>> test_it_results_map;
                for (const auto & lane_idx : available_lane)
                {
                    test_it_results_map.emplace("lane["+std::to_string(lane_idx)+"]", tc->GetItLaneResult(lane_idx));
                }
                input_parser.PrintJsonTestSeqAndResultThreads(true, base_name, MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, testcases_param.gtyp_prbs[gt_index].global_config, test_it_results_map);

                ret |= DisplayTestResult(log, type_name, tc->GetThreadResult());
                LogMessageSw(log, MSG_GEN_018, {LOG_LINE_DELIMITER});
            }
        }
    }

    return ret;
}

uint GetNumTestCasesNotCompleted( const std::vector<TestCase*> & tc_list, const ThreadType & thread_type )
{
    uint tc_count = 0;
    for (const auto & tc : tc_list)
    {
        if (tc->GetTestCaseType() == thread_type)
        {
            if (!tc->GetTestComplete())
            {
                tc_count++;
            }
        }
    }
    return tc_count;
}

bool CheckTestCasesCompleted( Logging & log, std::atomic<bool> & abort, const std::vector<TestCase*> & tc_list )
{
    auto internal_abort = false;
    for (const auto & tc : tc_list)
    {
        if (tc->GetTestComplete() || internal_abort)
        {
            continue; // no need to check test if already completed or if abort has been detected
        }
        if (tc->WaitThread(0))
        {
            if (tc->CheckTestAborted()) // thread has completed, check if it has aborted
            {
                LogMessageSw(log, MSG_DEBUG_TESTCASE, {tc->GetThreadID() + ": internally aborted"});
                abort           = true; // set flag to abort all other thread
                internal_abort  = true;
            }
            tc->SetTestComplete(); // complete test
        }
    }
    return internal_abort;
}

void SignalAbortTestCases( Logging & log, const std::vector<TestCase*> & tc_list, const ThreadType & thread_type )
{
    if (thread_type == THREAD_TYPE_TESTCASE)
    {
        LogMessageSw(log, MSG_DEBUG_TESTCASE, {"Propagate abort to all testcases"});
    }
    else
    {
        LogMessageSw(log, MSG_DEBUG_TESTCASE, {"Propagate stop to all tasks"}); // task doesn't abort but they are stopped
    }
    for (const auto & tc : tc_list)
    {
        if (tc->GetTestCaseType() == thread_type)
        {
            if (!tc->GetTestComplete())
            {
                tc->SignalAbortTest();
            }
        }
    }
}

void CheckTestCasesAborted( Logging & log, const std::vector<TestCase*> & tc_list, const ThreadType & thread_type )
{
    for (const auto & tc : tc_list)
    {
        if ((tc->GetTestCaseType() != thread_type) || (tc->GetTestComplete()))
        {
            continue; // no need to check test if already completed, and wait for abort to be detected
        }
        if (tc->WaitThread(0))
        {
            tc->SetTestComplete(); // complete test
            if (!tc->CheckTestAborted()) // thread has completed, check if it has aborted
            {
                LogMessageSw(log, MSG_DEBUG_TESTCASE, {tc->GetThreadID() + ": thread already ended before being aborted"});
            }
            if (GetNumTestCasesNotCompleted(tc_list, thread_type) == 0)
            {
                LogMessageSw(log, MSG_DEBUG_TESTCASE, {"All threads correctly aborted"});
            }
        }
    }
}

bool WaitEndOfTestCases( Logging & log, Timer & timer, std::atomic<bool> & abort, const std::vector<TestCase*> & tc_list, const ThreadType & thread_type )
{
    auto internal_abort   = false;
    auto abort_propagated = false;
    while (GetNumTestCasesNotCompleted(tc_list, thread_type) > 0)
    {
        timer.WaitFor1sTick();
        if (!abort) // Manage end of test without abort and thread with internal abort
        {
            internal_abort = CheckTestCasesCompleted(log, abort, tc_list); // Check for all testcase type and if abort
        }
        else
        {
            if (!abort_propagated)
            {
                // if abort detected, signal abort to all thread not completed
                SignalAbortTestCases(log, tc_list, thread_type);
                abort_propagated = true;
            }
            CheckTestCasesAborted(log, tc_list, thread_type); // Check thread aborted
        }
    }
    return internal_abort;
}

bool DumpMessageDefinition( Logging & log, XbtestSwConfig & xbtest_sw_config, Tasks_Type_Parameters_t & debug )
{
    auto it = FindJsonParam(debug.param, GET_MSG_DEF_MEMBER);
    if (it != debug.param.end())
    {
        return log.WriteMessagesJson(xbtest_sw_config.GetLogDir() + "/" + TestcaseParamCast<std::string>(it->second), xbtest_sw_config.GetCommandLineForce());
    }
    return RET_SUCCESS;
}

bool CheckP2PCanRun( Logging & log, XbtestSwConfig & xbtest_sw_config, XbtestSwConfig & xbtest_sw_config_p2p, const bool & p2p_card_exists, const bool & p2p_nvme_exists )
{
    if (xbtest_sw_config.GetCommandLineP2pTargetBdf().exists && !p2p_card_exists)
    {
        LogMessageSw(log, MSG_GEN_076, {"-T", P2P_TARGET_BDF, P2P_CARD_MEMBER.name});
        return RET_FAILURE;
    }
    if (xbtest_sw_config.GetCommandLineP2pNvmePath().exists && !p2p_nvme_exists)
    {
        LogMessageSw(log, MSG_GEN_076, {"-N", P2P_NVME_PATH, P2P_NVME_MEMBER.name});
        return RET_FAILURE;
    }
    if (p2p_card_exists && !(xbtest_sw_config.GetCommandLineP2pTargetBdf().exists))
    {
        LogMessageSw(log, MSG_GEN_077, {P2P_CARD_MEMBER.name, "-T", P2P_TARGET_BDF});
        return RET_FAILURE;
    }
    if (p2p_nvme_exists && !(xbtest_sw_config.GetCommandLineP2pNvmePath().exists))
    {
        LogMessageSw(log, MSG_GEN_077, {P2P_NVME_MEMBER.name, "-N", P2P_NVME_PATH});
        return RET_FAILURE;
    }
    // Check P2P supported on source card
    if (p2p_card_exists && !(xbtest_sw_config.GetIsP2PSupported()))
    {
        LogMessageSw(log, MSG_GEN_078, {P2P_CARD_MEMBER.name, "supported", "source"});
        return RET_FAILURE;
    }
    if (p2p_nvme_exists && !(xbtest_sw_config.GetIsP2PSupported()))
    {
        LogMessageSw(log, MSG_GEN_078, {P2P_NVME_MEMBER.name, "supported", "source"});
        return RET_FAILURE;
    }
    // Check P2P enabled on source card
    if (p2p_card_exists && !(xbtest_sw_config.IsP2PEnabled()))
    {
        LogMessageSw(log, MSG_GEN_078, {P2P_CARD_MEMBER.name, "enabled", "source"});
        return RET_FAILURE;
    }
    if (p2p_nvme_exists && !(xbtest_sw_config.IsP2PEnabled()))
    {
        LogMessageSw(log, MSG_GEN_078, {P2P_NVME_MEMBER.name, "enabled", "source"});
        return RET_FAILURE;
    }
    // Check P2P supported on target card
    if (p2p_card_exists && !(xbtest_sw_config_p2p.GetIsP2PSupported()))
    {
        LogMessageSw(log, MSG_GEN_078, {P2P_CARD_MEMBER.name, "supported", "target"});
        return RET_FAILURE;
    }
    // Check P2P enabled on target card card
    if (p2p_card_exists && !(xbtest_sw_config_p2p.IsP2PEnabled()))
    {
        LogMessageSw(log, MSG_GEN_078, {P2P_CARD_MEMBER.name, "enabled", "target"});
        return RET_FAILURE;
    }
    // Check if source card is not nodma for p2p_card
    if (p2p_card_exists && !(xbtest_sw_config.GetIsDmaSupported()))
    {
        LogMessageSw(log, MSG_GEN_079, {P2P_CARD_MEMBER.name});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

} // namespace

