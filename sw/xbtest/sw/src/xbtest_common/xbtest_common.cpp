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

#include <csignal>
#include <sys/wait.h>
#include <thread>
#include <iostream>
#include <future>

#include "xbtest.h"
#include "xbtestcommonpackage.h"
#include "logging.h"
#include "xjsonparser.h"
#include "xbtestcommonconfig.h"
#include "timer.h"
#include "consolemgmtcommon.h"

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

// Threads
using Xbtest_Thread_t = struct Xbtest_Thread_t {
    std::string             bdf;
    bool                    child_exists;
    pid_t                   child_pid;
    Card_Config_t           card_config;
    Card_Test_Queue_Value_t card_test_queue_value;
    std::vector<int>        tests_ret_codes;
    int                     thread_ret;
    std::thread             thread;
};

using Card_Status_Thread_t = struct Card_Status_Thread_t {
    std::string             bdf;
    Card_Config_t           card_config;
    Sensor_Queue_Value_t    sensor_queue_value;
    bool                    power_node_absence_msg;
    bool                    temperature_node_absence_msg;
    int                     thread_ret;
    std::thread             thread;
};

void LogMessageCommon ( Logging & log, const Message_t & message )
{
    log.LogMessage(LOG_HEADER_GENERAL, message);
}
void LogMessageCommon ( Logging & log, const Message_t & message, const std::vector<std::string> & arg_list )
{
    log.LogMessage(LOG_HEADER_GENERAL, message, arg_list);
}
void LogMessageCommonBdf ( Logging & log, const Message_t & message, const std::string & bdf )
{
    log.LogMessage(LOG_HEADER_GENERAL, bdf, "", message);
}
void LogMessageCommonBdf ( Logging & log, const Message_t & message, const std::string & bdf, const std::vector<std::string> & arg_list )
{
    log.LogMessage(LOG_HEADER_GENERAL, bdf, "", message, arg_list);
}

void StopThreadsCommon( Timer & timer, ConsoleMgmtCommon & console_mgmt, std::thread & signal_thread, std::thread & console_mgmt_thread, std::thread & timer_thread, std::atomic<bool> & stop_signal_thread )
{
    stop_signal_thread = true;
    signal_thread.join();

    console_mgmt.Stop();
    console_mgmt_thread.join();

    timer.Stop();
    timer_thread.join();
}

// signal handling

void HandleSignalsCommon( Logging * log, Timer * timer, std::atomic<bool> * abort, std::atomic<bool> * stop_signal_thread )
{
    while (!(*stop_signal_thread))
    {
        if (get_sig_abort() && !(*abort))
        {
            LogMessageCommon(*log, MSG_GEN_012);
            *abort = true;
        }
        if (get_sig_terminate())
        {
            set_sig_terminate(false); // Reset the flag so we'll detect if other SIGBUS arrives
        }
        timer->WaitFor1sTick();
    }
}

#define RET_CODE_NOT_SET  (int)(-1)
#define RET_CODE_PASS     (int)(0)
#define RET_CODE_FAIL     (int)(1)
#define RET_CODE_ABORT    (int)(2)

int RunXbtest ( Logging & log, ConsoleMgmtCommon & console_mgmt, XbtestCommonConfig & xbtest_common_config, Timer & timer, std::atomic<bool> * abort, std::vector<Xbtest_Thread_t> & xbtest_sw_threads, const uint & thread_idx, const uint & test_idx, const uint & num_test, const std::vector<std::string> & sys_cmd )
{
    const uint64_t DURATION_DIVIDER = 10;
    auto tmp         = "xbtest run " + std::to_string(test_idx) + "): ";
    auto sys_cmd_str = StrVectToStr(sys_cmd, " ");

    LogMessageCommonBdf(log, MSG_GEN_064, xbtest_sw_threads[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test), sys_cmd_str});

    // Don't fork if abort received
    // But after fork is done run inner xbtest even if abort, and parent will kill it once it started
    if (*abort)
    {
        return RET_CODE_ABORT;
    }

    // Use fork to run xbtest inner on selected card with new PID so correct process receives sigbus and handles it.

    pid_t pid = fork();

    // DO NOT use LogMessage in fork()'ed child as log and console mgmt uses mutex and we do not protect lock here

    if (pid < 0) // On failure, -1 is returned
    {
        LogMessageCommonBdf(log, MSG_ITF_113, xbtest_sw_threads[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test), "fork failed", sys_cmd_str});
        return RET_CODE_FAIL;
    }
    if (pid == 0) // pid=0 is returned in the child process
    {

        // The list of arguments argv must be terminated by a null pointer
        std::vector<char *> sys_cmd_char;
        sys_cmd_char.reserve(sys_cmd.size()+1);
        for (const auto & str : sys_cmd)
        {
            sys_cmd_char.emplace_back(const_cast<char*>(str.c_str()));
        }
        sys_cmd_char.emplace_back(nullptr);

        auto ret = run_core_inner(sys_cmd.size(), sys_cmd_char.data());
        std::_Exit(ret);
    }
    else // the pid of the child process is returned in the parent process
    {
        xbtest_sw_threads[thread_idx].child_pid = pid;
        xbtest_sw_threads[thread_idx].child_exists = true;
        LogMessageCommonBdf(log, MSG_DEBUG_TESTCASE, xbtest_sw_threads[thread_idx].bdf, {tmp + "(parent) - Child pid = " + std::to_string(pid)});

        pid_t wid;
        int status;
        uint64_t elapsed_time = 0;

        do {
            wid = waitpid(pid, &status, WNOHANG); // Run with non-blocking waitpid

            xbtest_sw_threads[thread_idx].card_test_queue_value.elapsed_time = std::to_string(elapsed_time);
            if (xbtest_common_config.GetMultitestMode())
            {
                console_mgmt.PushCardTestQueue(xbtest_sw_threads[thread_idx].card_config.dsa_name, xbtest_sw_threads[thread_idx].bdf, xbtest_sw_threads[thread_idx].card_test_queue_value);
            }
            if ((elapsed_time % DURATION_DIVIDER == 0) && (xbtest_common_config.GetMultitestMode()))
            {
                LogMessageCommonBdf(log, MSG_GEN_060, xbtest_sw_threads[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test), std::to_string(elapsed_time)});
            }

            if (wid == 0)
            {
                if (*abort)
                {
                    kill(xbtest_sw_threads[thread_idx].child_pid, SIGINT); // Propagate Interrupt to child process also here, just in case...
                }
                timer.WaitFor1sTick();
                elapsed_time++;
            }
        } while (wid == 0);

        xbtest_sw_threads[thread_idx].child_exists = false;

        if (WIFSIGNALED(status))
        {
            LogMessageCommonBdf(log, MSG_ITF_113, xbtest_sw_threads[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test), "process killed by signal (" + std::to_string(WTERMSIG(status)) + ")", sys_cmd_str});
            return RET_CODE_FAIL;
        }
        if (!WIFEXITED(status)) // the child didn't terminate normally
        {
            LogMessageCommonBdf(log, MSG_ITF_113, xbtest_sw_threads[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test), "abnormal termination", sys_cmd_str});
            return RET_CODE_FAIL;
        }
        // Return results
        if (*abort)
        {
            return RET_CODE_ABORT;
        }
        if (WEXITSTATUS(status) == 0)
        {
            return RET_CODE_PASS;
        }
        return RET_CODE_FAIL;
    }
    return RET_CODE_FAIL;
}

void RunXbtestThread( Logging * log, ConsoleMgmtCommon * console_mgmt, XbtestCommonConfig * xbtest_common_config, Timer * timer, std::atomic<bool> * abort, std::vector<Xbtest_Thread_t> * xbtest_sw_threads, const uint & thread_idx )
{
    LogMessageCommonBdf(*log, MSG_DEBUG_TESTCASE, (*xbtest_sw_threads)[thread_idx].bdf, {"Starting test thread (" + std::to_string(thread_idx) + ")"});
    auto num_test = (*xbtest_sw_threads)[thread_idx].card_config.tests.size();

    for (uint i=0; (i < num_test) && !(*abort); i++)
    {
        (*xbtest_sw_threads)[thread_idx].tests_ret_codes[i] = RET_CODE_NOT_SET;
        uint test_idx = i+1;

        (*xbtest_sw_threads)[thread_idx].card_test_queue_value.elapsed_time = std::to_string(0);
        (*xbtest_sw_threads)[thread_idx].card_test_queue_value.test         = (*xbtest_sw_threads)[thread_idx].card_config.tests[i].test_id_name;
        console_mgmt->PushCardTestQueue((*xbtest_sw_threads)[thread_idx].card_config.dsa_name, (*xbtest_sw_threads)[thread_idx].bdf, (*xbtest_sw_threads)[thread_idx].card_test_queue_value);

        auto ret_code = RunXbtest(*log, *console_mgmt, *xbtest_common_config, *timer, abort, *xbtest_sw_threads, thread_idx, test_idx, num_test, (*xbtest_sw_threads)[thread_idx].card_config.tests[i].sys_cmd);

        LogMessageCommonBdf(*log, MSG_DEBUG_TESTCASE, (*xbtest_sw_threads)[thread_idx].bdf, {"Test " + std::to_string(test_idx) + ": xbtest SW return code: " + std::to_string(ret_code)});

        if (*abort)
        {
            ret_code = RET_CODE_ABORT;
        }

        (*xbtest_sw_threads)[thread_idx].tests_ret_codes[i] = ret_code;

        if (ret_code == RET_CODE_FAIL)
        {
            (*xbtest_sw_threads)[thread_idx].thread_ret = RET_CODE_FAIL;
        }

        if (ret_code == RET_CODE_PASS)
        {
            (*xbtest_sw_threads)[thread_idx].card_test_queue_value.passed++;
        }
        else
        {
            (*xbtest_sw_threads)[thread_idx].card_test_queue_value.failed++;
        }
        (*xbtest_sw_threads)[thread_idx].card_test_queue_value.completed++;
        (*xbtest_sw_threads)[thread_idx].card_test_queue_value.pending--;
        (*xbtest_sw_threads)[thread_idx].card_test_queue_value.elapsed_time = NOT_APPLICABLE;
        (*xbtest_sw_threads)[thread_idx].card_test_queue_value.test         = NOT_APPLICABLE;

        if (xbtest_common_config->GetMultitestMode())
        {
            console_mgmt->PushCardTestQueue((*xbtest_sw_threads)[thread_idx].card_config.dsa_name, (*xbtest_sw_threads)[thread_idx].bdf, (*xbtest_sw_threads)[thread_idx].card_test_queue_value);
            if (ret_code == RET_CODE_ABORT)
            {
                LogMessageCommonBdf(*log, MSG_GEN_052, (*xbtest_sw_threads)[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test)});
            }
            else if (ret_code == RET_CODE_PASS)
            {
                LogMessageCommonBdf(*log, MSG_GEN_051, (*xbtest_sw_threads)[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test)});
            }
            else
            {
                LogMessageCommonBdf(*log, MSG_GEN_050, (*xbtest_sw_threads)[thread_idx].bdf, {std::to_string(test_idx), std::to_string(num_test)});
            }
        }
    }

    if (*abort)
    {
        (*xbtest_sw_threads)[thread_idx].thread_ret = RET_CODE_ABORT;

        // Set all remaining test result to aborted
        for (uint64_t remain_test = (*xbtest_sw_threads)[thread_idx].card_test_queue_value.completed; remain_test < num_test; remain_test++)
        {
            (*xbtest_sw_threads)[thread_idx].tests_ret_codes[remain_test] = RET_CODE_ABORT;
            (*xbtest_sw_threads)[thread_idx].card_test_queue_value.completed++;
            (*xbtest_sw_threads)[thread_idx].card_test_queue_value.failed++;
            (*xbtest_sw_threads)[thread_idx].card_test_queue_value.pending--;
        }
    }
    if (xbtest_common_config->GetMultitestMode())
    {
        console_mgmt->PushCardTestQueue((*xbtest_sw_threads)[thread_idx].card_config.dsa_name, (*xbtest_sw_threads)[thread_idx].bdf, (*xbtest_sw_threads)[thread_idx].card_test_queue_value);
        if ((*xbtest_sw_threads)[thread_idx].thread_ret == RET_CODE_ABORT)
        {
            LogMessageCommonBdf(*log, MSG_GEN_055, (*xbtest_sw_threads)[thread_idx].bdf); // card aborted
        }
        else if ((*xbtest_sw_threads)[thread_idx].thread_ret == RET_CODE_PASS)
        {
            LogMessageCommonBdf(*log, MSG_GEN_057, (*xbtest_sw_threads)[thread_idx].bdf); // card pass
        }
        else
        {
            LogMessageCommonBdf(*log, MSG_GEN_056, (*xbtest_sw_threads)[thread_idx].bdf); // card fail
        }

        while(!console_mgmt->CardTestQueueEmpty((*xbtest_sw_threads)[thread_idx].card_config.dsa_name, (*xbtest_sw_threads)[thread_idx].bdf))
        {
            LogMessageCommonBdf(*log, MSG_DEBUG_TESTCASE, (*xbtest_sw_threads)[thread_idx].bdf, {"Waiting queue empty"}); // card aborted
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void RunCardStatusThread( Logging * log, ConsoleMgmtCommon * console_mgmt, Timer * timer, std::atomic<bool> * abort, std::vector<Card_Status_Thread_t> * card_stat_threads, const uint & thread_idx, std::atomic<bool> * stop_card_stat_threads )
{
    LogMessageCommonBdf(*log, MSG_DEBUG_TESTCASE, (*card_stat_threads)[thread_idx].bdf, {"Starting status thread (" + std::to_string(thread_idx) + ")"});

    std::vector<std::string> node_title;
    std::string tmp_str;
    auto device_info_init_ret = RET_SUCCESS;

    DeviceInfoParser device_info_parser(log, (*card_stat_threads)[thread_idx].bdf, abort);

    while (!(*stop_card_stat_threads) && !(*abort))
    {
        auto device_info_ret = RET_SUCCESS;

        // Create new xrt::device every seconds and release it after get sensor as xbtest-common does not terminate on SIGBUS
        auto electrical_info    = RST_ELECTRICAL_INFO;
        auto thermal_info       = RST_THERMAL_INFO;

        if (device_info_init_ret == RET_SUCCESS)
        {
            device_info_ret = device_info_parser.GetSensorsInfo(electrical_info, thermal_info);
        }
        else
        {
            device_info_ret = RET_FAILURE;
        }
        // get sensor_queue_value
        (*card_stat_threads)[thread_idx].sensor_queue_value.dump_valid = false;
        if (device_info_ret == RET_SUCCESS)
        {
            (*card_stat_threads)[thread_idx].sensor_queue_value.dump_valid = true;
        }
        (*card_stat_threads)[thread_idx].sensor_queue_value.dump_cnt++;
        (*card_stat_threads)[thread_idx].sensor_queue_value.power_found = false;
        for (const auto & info : electrical_info.power_rails)
        {
            if (StrMatchNoCase((*card_stat_threads)[thread_idx].card_config.lib.card_power_id, info.id))
            {
                if (info.current.is_present && info.voltage.is_present)
                {
                    (*card_stat_threads)[thread_idx].sensor_queue_value.power         = info.voltage.volts * info.current.amps;
                    (*card_stat_threads)[thread_idx].sensor_queue_value.power_found   = true;
                }
            }
        }
        for (const auto & info : electrical_info.power_consumptions)
        {
            if (StrMatchNoCase((*card_stat_threads)[thread_idx].card_config.lib.card_power_id, info.id))
            {
                (*card_stat_threads)[thread_idx].sensor_queue_value.power         = info.power_consumption_watts;
                (*card_stat_threads)[thread_idx].sensor_queue_value.power_found   = true;
            }
        }
        (*card_stat_threads)[thread_idx].sensor_queue_value.temperature_found = false;
        for (const auto & info : thermal_info.thermals)
        {
            if (StrMatchNoCase((*card_stat_threads)[thread_idx].card_config.lib.card_temperature_id, info.location_id))
            {
                (*card_stat_threads)[thread_idx].sensor_queue_value.temperature       = info.temp_c;
                (*card_stat_threads)[thread_idx].sensor_queue_value.temperature_found = true;
            }
        }
        // display sensor absence
        if (!(*card_stat_threads)[thread_idx].sensor_queue_value.power_found && (*card_stat_threads)[thread_idx].power_node_absence_msg)
        {
            (*card_stat_threads)[thread_idx].power_node_absence_msg = false;
            LogMessageCommonBdf(*log,MSG_MGT_022, (*card_stat_threads)[thread_idx].bdf, {XRT_INFO_DEVICE_ELECTRICAL, (*card_stat_threads)[thread_idx].card_config.lib.card_power_id});
        }
        if (!(*card_stat_threads)[thread_idx].sensor_queue_value.temperature_found && (*card_stat_threads)[thread_idx].temperature_node_absence_msg)
        {
            (*card_stat_threads)[thread_idx].temperature_node_absence_msg = false;
            LogMessageCommonBdf(*log,MSG_MGT_022, (*card_stat_threads)[thread_idx].bdf, {XRT_INFO_DEVICE_THERMAL, (*card_stat_threads)[thread_idx].card_config.lib.card_temperature_id});
        }
        console_mgmt->PushSensorQueue((*card_stat_threads)[thread_idx].card_config.dsa_name, (*card_stat_threads)[thread_idx].bdf, (*card_stat_threads)[thread_idx].sensor_queue_value);
        // display sensor
        auto temperature_str = NOT_APPLICABLE;
        if ((*card_stat_threads)[thread_idx].sensor_queue_value.temperature_found)
        {
            temperature_str = Float_to_String<double>((*card_stat_threads)[thread_idx].sensor_queue_value.temperature, 0) + " C";
        }
        auto power_str = NOT_APPLICABLE;
        if ((*card_stat_threads)[thread_idx].sensor_queue_value.power_found)
        {
            power_str = Float_to_String<double>((*card_stat_threads)[thread_idx].sensor_queue_value.power, 1) + " W";
        }
        LogMessageCommonBdf(*log,MSG_GEN_061, (*card_stat_threads)[thread_idx].bdf, {power_str, temperature_str});

        timer->WaitFor1sTick(); // Got card status, wait before continuing
    }

    auto    api_total       = (*card_stat_threads)[thread_idx].sensor_queue_value.dump_cnt;
    auto    api_fail_total  = device_info_parser.GetApiFailTotal();
    double  percent         = 100.0 * (double)(api_fail_total) / (double)(api_total);

    if (percent > PERCENT_API_FAIL_ERROR_LIMIT)
    {
        LogMessageCommonBdf(*log,MSG_MGT_023, (*card_stat_threads)[thread_idx].bdf, {Float_to_String<double>(percent,3), Float_to_String<double>(PERCENT_API_FAIL_ERROR_LIMIT,0), std::to_string(api_fail_total), std::to_string(api_total) });
    }
    else if (percent > PERCENT_API_FAIL_WARNING_LIMIT)
    {
        LogMessageCommonBdf(*log,MSG_MGT_024, (*card_stat_threads)[thread_idx].bdf, {Float_to_String<double>(percent,3), Float_to_String<double>(PERCENT_API_FAIL_WARNING_LIMIT,0), std::to_string(api_fail_total), std::to_string(api_total) });
    }
}

void DisplayHeaderCommon( Logging & log, XbtestCommonConfig & xbtest_common_config, std::vector<std::string> & cmd_line )
{
    if (xbtest_common_config.GetMultitestMode())
    {
        LogMessageCommon(log, MSG_GEN_039, {LOG_LINE_DELIMITER});
        LogMessageCommon(log, MSG_GEN_039, {"Command line: " + StrVectToStr(cmd_line, " ")});
        LogMessageCommon(log, MSG_GEN_039, {"xbtest version: " + XBTEST_VERSION_STR});
        LogMessageCommon(log, MSG_GEN_016, {"\t - SW Build    : " + std::to_string(XBTEST_BUILD_VERSION) + " on " + XBTEST_BUILD_DATE});
#ifdef INTERNAL_RELEASE
        LogMessageCommon(log, MSG_GEN_039, {"\t - Release     : INTERNAL"});
#endif
        LogMessageCommon(log, MSG_GEN_016, {"\t - Process ID  : " + std::to_string(getpid())});
        LogMessageCommon(log, MSG_GEN_016, {LOG_LINE_DELIMITER});
        LogMessageCommon(log, MSG_GEN_016, {"System: "});
        LogMessageCommon(log, MSG_GEN_016, {"\t - User          : " + xbtest_common_config.GetSystemUsername()});
        LogMessageCommon(log, MSG_GEN_016, {"\t - Name          : " + xbtest_common_config.GetSystemSysname()});
        LogMessageCommon(log, MSG_GEN_016, {"\t - Node          : " + xbtest_common_config.GetSystemNodename()});
        LogMessageCommon(log, MSG_GEN_016, {"\t - Release       : " + xbtest_common_config.GetSystemRelease()});
        LogMessageCommon(log, MSG_GEN_016, {"\t - Version       : " + xbtest_common_config.GetSystemVersion()});
        LogMessageCommon(log, MSG_GEN_016, {"\t - Machine       : " + xbtest_common_config.GetSystemMachine()});
        LogMessageCommon(log, MSG_GEN_039, {LOG_LINE_DELIMITER});
        LogMessageCommon(log, MSG_GEN_039, {"Start of xbtest session at: " + xbtest_common_config.GetSessionStartTime()});
        LogMessageCommon(log, MSG_GEN_039, {LOG_LINE_DELIMITER});
    }
}

void DisplayXbtestFooter( Logging & log, XbtestCommonConfig & xbtest_common_config, std::vector<std::string> & cmd_line )
{
    xbtest_common_config.SetEndSessionTime();

    if (xbtest_common_config.GetMultitestMode())
    {
        LogMessageCommon(log, MSG_GEN_040, {LOG_LINE_COMMON});
        LogMessageCommon(log, MSG_GEN_017, {"xbtest: "});
        LogMessageCommon(log, MSG_GEN_017, {"\t - Version     : " + XBTEST_VERSION_STR});
        LogMessageCommon(log, MSG_GEN_017, {"\t - SW Build    : " + std::to_string(XBTEST_BUILD_VERSION) + " on " + XBTEST_BUILD_DATE});
        #ifdef INTERNAL_RELEASE
        LogMessageCommon(log, MSG_GEN_017, {"\t - Release     : INTERNAL"});
        #endif
        LogMessageCommon(log, MSG_GEN_017, {"\t - Process ID  : " + std::to_string(getpid())});
        LogMessageCommon(log, MSG_GEN_017, {"\t - Command line: " + StrVectToStr(cmd_line, " ")});
        LogMessageCommon(log, MSG_GEN_017, {LOG_LINE_DELIMITER});
        LogMessageCommon(log, MSG_GEN_040, {"End of xbtest session at: " + xbtest_common_config.GetSessionEndTime()});
        LogMessageCommon(log, MSG_GEN_017, {LOG_LINE_DELIMITER});
    }
}

bool CheckDriverAccessCommon( Logging & log, std::atomic<bool> & abort )
{
    DeviceInfoParser device_info_parser_0(&log, "0", &abort); // Use first device found
    return device_info_parser_0.CheckDriverAccess();
}

int run( int argc, char** argv )
{
    // Setup for signal handler
    std::atomic<bool> abort;
    abort = false;
    set_sig_abort(false);
    set_sig_terminate(false);

    // Logging
    Logging log(&abort, XBTEST_COMMON);

    // Timer
    Timer timer(&log, Timer::RES_1s);

    // xbtest common configuration
    XbtestCommonConfig xbtest_common_config(&log, &abort);

    // Console Mgmt
    ConsoleMgmtCommon console_mgmt(&log, &timer, &xbtest_common_config);
    // log.SetUseConsole(true);
    log.SetUseConsole(false);

    // Threads
    std::atomic<bool>   stop_signal_thread;
    std::thread         signal_thread;
    std::thread         timer_thread;
    std::thread         console_mgmt_thread;

    std::vector<Xbtest_Thread_t>        xbtest_sw_threads;
    std::atomic<bool>                   stop_card_stat_threads;
    std::vector<Card_Status_Thread_t>   card_stat_threads;

    std::vector<std::string> cmd_line;
    cmd_line.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        cmd_line.emplace_back(std::string(argv[i]));
    }

    try
    {
        auto xbtest_common_ret = RET_SUCCESS;

        // Logging
        if (log.GetMessagesParserRet() == RET_FAILURE)
        {
            return EXIT_FAILURE; // Check central message definition passed
        }

        // Threads
        stop_card_stat_threads  = false;
        stop_signal_thread      = false;
        signal_thread           = std::thread(&HandleSignalsCommon, &log, &timer, &abort, &stop_signal_thread);
        timer_thread            = std::thread(&Timer::RunTask, &timer);
        console_mgmt_thread     = std::thread(&ConsoleMgmtCommon::ConsoleTask, &console_mgmt);

        // Scan installation
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (xbtest_common_config.ScanHostSetup() == RET_FAILURE)
            {
                xbtest_common_ret = RET_FAILURE;
            }
        }

        // Parse command line options
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            uint command_line_ret = xbtest_common_config.ParseCommandLine(argc, argv);
            if (command_line_ret == COMMAND_LINE_EXIT_SUCCESS)
            {
                LogMessageCommon(log, MSG_GEN_007);
                StopThreadsCommon(timer, console_mgmt, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread); // Stop other Threads
                return EXIT_SUCCESS;
            }
            if (command_line_ret == COMMAND_LINE_RET_FAILURE)
            {
                LogMessageCommon(log, MSG_GEN_009);
                xbtest_common_ret = RET_FAILURE;
            }
        }

        // Check driver
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (CheckDriverAccessCommon(log, abort) == RET_FAILURE)
            {
                xbtest_common_ret = RET_FAILURE;
            }
        }

        // Check for valid xbtest HW designs
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (xbtest_common_config.CheckNumLibValid() == RET_FAILURE)
            {
                xbtest_common_ret = RET_FAILURE;
            }
        }

        // Parse the card configuration JSON file if provided
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (xbtest_common_config.ParseCardJson() == RET_FAILURE)
            {
                xbtest_common_ret = RET_FAILURE;
            }
        }

        // Generate the actual card configuration
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (xbtest_common_config.GenerateActualCardConfig() == RET_FAILURE)
            {
                xbtest_common_ret = RET_FAILURE;
            }
        }

        // Display xbtest log header and the configuration of the run in multi-test mode
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            DisplayHeaderCommon(log, xbtest_common_config, cmd_line);
        }

        std::map<std::string,Card_Config_t> card_config_map;
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (xbtest_common_config.GetMultitestMode())
            {
                xbtest_common_config.PrintConfig();
            }
            card_config_map = xbtest_common_config.GetCardConfigMap();
        }


        // Allocate all queues before starting the console_mgmt. It won't be used in single test mode
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (!abort)
            {
                for (const auto & cfg : card_config_map)
                {
                    console_mgmt.AllocateCardTestQueue(cfg.second.dsa_name, cfg.first, RESET_CARD_TEST_QUEUE_VALUE);
                    console_mgmt.AllocateSensorQueue  (cfg.second.dsa_name, cfg.first, RESET_SENSOR_QUEUE_VALUE);
                }
            }
        }

        // console_mgmt exits start-up phase when the command line are parsed
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (console_mgmt.SetUp() == RET_FAILURE)
            {
                xbtest_common_ret = RET_FAILURE;
            }
        }

        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            console_mgmt.Start();
        }

        // Set flag to indicate to xbtest SW that common is running in multitest mode
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            if (xbtest_common_config.GetMultitestMode())
            {
                set_multitest_mode(true);
            }
            else
            {
                set_multitest_mode(false);
            }
        }

        // Start the tests and card management
        auto testing = false;
        if ((!abort) && (xbtest_common_ret == RET_SUCCESS))
        {
            testing = true;
            // Start the tests
            xbtest_sw_threads.clear();
            for (const auto & cfg : card_config_map)
            {
                LogMessageCommonBdf(log, MSG_DEBUG_TESTCASE, cfg.first, {"Launch xbtest threads"}); // one xbtest thread per card
                Xbtest_Thread_t xbtest_thread;
                xbtest_thread.bdf                           = cfg.first;
                xbtest_thread.card_config                   = xbtest_common_config.GetCardConfig(cfg.first);
                xbtest_thread.card_test_queue_value         = RESET_CARD_TEST_QUEUE_VALUE;
                xbtest_thread.card_test_queue_value.pending = cfg.second.tests.size();
                console_mgmt.PushCardTestQueue(cfg.second.dsa_name, xbtest_thread.bdf, xbtest_thread.card_test_queue_value);
                xbtest_thread.tests_ret_codes.reserve(cfg.second.tests.size());
                xbtest_thread.thread_ret                    = RET_CODE_PASS;
                xbtest_sw_threads.emplace_back(std::move(xbtest_thread));
            }
            for (uint thread_idx = 0; thread_idx<xbtest_sw_threads.size(); thread_idx++)
            {
                xbtest_sw_threads[thread_idx].thread = std::thread(&RunXbtestThread, &log, &console_mgmt, &xbtest_common_config, &timer, &abort, &xbtest_sw_threads, thread_idx); // create thread after emplace_back xbtest_thread to xbtest_sw_threads
            }

            // Start the card management
            if (xbtest_common_config.GetMultitestMode())
            {
                LogMessageCommon(log, MSG_DEBUG_TESTCASE, {"Launch card status threads"}); // one ard status thread per card

                card_stat_threads.clear();
                for (const auto & cfg : card_config_map)
                {
                    Card_Status_Thread_t card_status_thread;
                    card_status_thread.bdf                          = cfg.first;
                    card_status_thread.card_config                  = xbtest_common_config.GetCardConfig(cfg.first);
                    card_status_thread.sensor_queue_value           = RESET_SENSOR_QUEUE_VALUE;
                    card_status_thread.power_node_absence_msg       = true;
                    card_status_thread.temperature_node_absence_msg = true;
                    card_status_thread.thread_ret                   = RET_CODE_PASS;
                    card_stat_threads.emplace_back(std::move(card_status_thread));
                }
                for (uint thread_idx = 0; thread_idx<card_stat_threads.size(); thread_idx++)
                {
                    card_stat_threads[thread_idx].thread = std::thread(&RunCardStatusThread, &log, &console_mgmt, &timer, &abort, &card_stat_threads, thread_idx, &stop_card_stat_threads); // create thread after emplace_back card_status_thread to card_stat_threads
                }
            }

            // Wait all xbtest threads finish
            for (auto & th : xbtest_sw_threads)
            {
                th.thread.join();
                if (th.thread_ret != RET_CODE_PASS)
                {
                    xbtest_common_ret = RET_FAILURE;
                }
            }

            // All card runs ended, stop card status threads
            if (xbtest_common_config.GetMultitestMode())
            {
                stop_card_stat_threads = true;

                // Wait card status threads finish for all cards
                for (auto & th : card_stat_threads)
                {
                    th.thread.join();
                    if (th.thread_ret != RET_CODE_PASS)
                    {
                        xbtest_common_ret = RET_FAILURE;
                    }
                }
                LogMessageCommon(log, MSG_GEN_062);
            }

            console_mgmt.SetAllTcEnded();
        }

        // Display xbtest log footer
        DisplayXbtestFooter(log, xbtest_common_config, cmd_line);

        // Display xbtest test results
        if (xbtest_common_config.GetMultitestMode())
        {
            if (testing && xbtest_common_config.GetMultitestMode())
            {
                // Wait all xbtest threads finish
                for (const auto & th : xbtest_sw_threads)
                {
                    std::vector<std::string> results;
                    results.reserve(th.card_config.tests.size());
                    for (uint i = 0; i < th.card_config.tests.size(); i++)
                    {
                        auto result = th.card_config.tests[i].test_id_name + ": ";
                        if (th.tests_ret_codes[i] == RET_CODE_ABORT)
                        {
                            result += "ABORTED";
                        }
                        else if (th.tests_ret_codes[i] == RET_CODE_FAIL)
                        {
                            result += "FAILED";
                        }
                        else if (th.tests_ret_codes[i] == RET_CODE_PASS)
                        {
                            result += "PASSED";
                        }
                        else
                        {
                            result += "NOT SET";
                        }
                        results.emplace_back(result);
                    }
                    if (th.thread_ret == RET_CODE_ABORT)
                    {
                        LogMessageCommonBdf(log, MSG_GEN_065, th.bdf, {StrVectToStr(results, " / ")});
                    }
                    else if (th.thread_ret == RET_CODE_PASS)
                    {
                        LogMessageCommonBdf(log, MSG_GEN_067, th.bdf, {StrVectToStr(results, " / ")});
                    }
                    else
                    {
                        LogMessageCommonBdf(log, MSG_GEN_066, th.bdf, {StrVectToStr(results, " / ")});
                    }
                }

                LogMessageCommon(log, MSG_GEN_040, {LOG_LINE_DELIMITER});
            }

            if (abort)
            {
                LogMessageCommon(log, MSG_GEN_046); // Common aborted
            }
            else if (xbtest_common_ret == RET_SUCCESS)
            {
                LogMessageCommon(log, MSG_GEN_048); // Common pass
            }
            else
            {
                LogMessageCommon(log, MSG_GEN_047); // Common fail
            }
        }

        StopThreadsCommon(timer, console_mgmt, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread); // Stop other Threads
        if (xbtest_common_ret == RET_SUCCESS)
        {
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    }
    catch (const std::exception& ex) // catch all thrown exceptions, should not occur
    {
        LogMessageCommon(log, MSG_GEN_025, {std::string(ex.what())});
    }

    StopThreadsCommon(timer, console_mgmt, signal_thread, console_mgmt_thread, timer_thread, stop_signal_thread); // Stop other Threads
    return EXIT_FAILURE;
}

} // namespace
