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

#include <cmath>

#include "verifytest.h"

namespace xbtest
{

VerifyTest::VerifyTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_devicemgt   = device_mgt;

    this->m_log_header  = LOG_HEADER_VERIFY;

    this->m_queue_testcase    = VERIFY_STR;
    this->m_queue_thread      = VERIFY_STR;
}

VerifyTest::~VerifyTest() = default;

bool VerifyTest::PreSetup()
{
    //nothing to do as PreTestChecks was called before the verify thread
    return RET_SUCCESS;
}

void VerifyTest::PostTeardown()
{
    // PostTestChecks is always called after the verify thread
    // Run PostTestChecks as soon as the thread ends so we check the watchdog status as soon as ResetWatchdog stops being called
    this->post_test_checks_result = PostTestChecks();
}

bool VerifyTest::GetPostTestChecksResult()
{
    return this->post_test_checks_result;
}

void VerifyTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_007);
        this->m_abort = true;
    }
}

bool VerifyTest::PreTestChecks()
{
    this->m_state = TestState::TS_PRE_SETUP;
    LogMessage(MSG_CMN_004);

    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }

    return RunPreTests();
}

bool VerifyTest::PostTestChecks()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
    auto test_failure = RET_SUCCESS;
    this->m_testcase_queue_value.remaining_time = NOT_APPLICABLE;
    this->m_testcase_queue_value.parameters = NOT_APPLICABLE;
    PushTestcaseQueue();

    if (RunPostTests() == RET_FAILURE)
    {
        test_failure |= RET_FAILURE;
        this->test_state = TEST_STATE_FAILURE;
        this->m_testcase_queue_value.failed++;
    }
    else
    {
        this->m_testcase_queue_value.passed++;
    }

    this->m_testcase_queue_value.pending--;
    this->m_testcase_queue_value.completed++;
    PushTestcaseQueue();

    return test_failure;
}

void VerifyTest::Run()
{
    this->m_state  = TestState::TS_RUNNING;
    this->m_result = TestResult::TR_FAILED;

    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});
        this->test_state = RunTest();
    }

    if ((this->test_state < 0))
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (this->test_state > 0)
    {
        LogMessage(MSG_CMN_010, {GetCurrentTime()});
        this->m_result = TestResult::TR_FAILED;
    }
    else
    {
        LogMessage(MSG_CMN_011, {GetCurrentTime()});
        this->m_result = TestResult::TR_PASSED;
    }

}

bool VerifyTest::WriteVerifyCu( const uint & address, const uint & value)
{
    return this->m_device->WriteVerifyCu(this->m_cu_idx, address, value);
}

bool VerifyTest::ReadVerifyCu( const uint & address, uint & read_data )
{
    return this->m_device->ReadVerifyCu(this->m_cu_idx, address, read_data);
}

bool VerifyTest::RunPreTests()
{
    auto test_failure               = RET_SUCCESS;
    uint overall_verify_pass_cnt    = 0;
    uint overall_verify_fail_cnt    = 0;

    auto num_known_cu = this->m_device->GetNumVerifyCu();
    num_known_cu     += this->m_device->GetNumPowerCu();
    num_known_cu     += this->m_device->GetNumGtmacCu();
    num_known_cu     += this->m_device->GetNumGtlpbkCu();
    num_known_cu     += this->m_device->GetNumGtPrbsCu();
    num_known_cu     += this->m_device->GetNumGtfPrbsCu();
    num_known_cu     += this->m_device->GetNumGtmPrbsCu();
    num_known_cu     += this->m_device->GetNumGtypPrbsCu();
    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        num_known_cu += this->m_device->GetNumMemoryCu(memory.type, memory.name);
    }

    // init display
    this->m_testcase_queue_value.remaining_time = NOT_APPLICABLE;
    this->m_testcase_queue_value.parameters = NOT_APPLICABLE;
    this->m_testcase_queue_value.pending = 3; // PreTestChecks, Verify Thread & PostTestChecks
    PushTestcaseQueue();

    if (num_known_cu == 0)
    {
        LogMessage(MSG_VER_006);
        test_failure |= RET_FAILURE;
    }
    else
    {
        // Check Verify xbtest HW IP build info
        auto test_it_failure = RET_SUCCESS;
        uint verify_pass_cnt = 0;
        uint verify_fail_cnt = 0;
        auto build_info = this->m_device->GetVerifyCuBI(this->m_cu_idx);

        // Check build info of xbtest HW IP
        test_it_failure |= CheckVerifyCu(build_info, verify_pass_cnt, verify_fail_cnt);

        LogMessage(MSG_VER_015, {std::to_string(build_info.build_version)});

        PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
        if (test_it_failure == RET_SUCCESS)
        {
            LogMessage(MSG_VER_007, {build_info.cu_name});
        }
        else
        {
            LogMessage(MSG_VER_008, {build_info.cu_name});
        }
        overall_verify_pass_cnt += verify_pass_cnt;
        overall_verify_fail_cnt += verify_fail_cnt;
        test_failure |= test_it_failure;


        // Check Power xbtest HW IPs build info
        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumPowerCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;
            auto build_info = this->m_device->GetPowerCuBI(cu_idx);

            // Check build info of xbtest HW IP
            test_it_failure |= CheckPowerCu(build_info, cu_idx, verify_pass_cnt, verify_fail_cnt);
            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }
        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumGtmacCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;
            auto build_info = this->m_device->GetGtmacCuBI(cu_idx);

            // Check build info of xbtest HW IPs
            test_it_failure |= CheckGtmacCu(build_info, cu_idx, verify_pass_cnt, verify_fail_cnt);
            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }
        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumGtlpbkCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;
            auto build_info = this->m_device->GetGtlpbkCuBI(cu_idx);

            // Check build info of xbtest HW IPs
            test_it_failure |= CheckGtlpbkCu(build_info, cu_idx, verify_pass_cnt, verify_fail_cnt);
            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }
        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumGtPrbsCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;
            auto build_info = this->m_device->GetGtPrbsCuBI(cu_idx);

            // Check build info of xbtest HW IPs
            test_it_failure |= CheckGtPrbsCu(build_info, cu_idx, verify_pass_cnt, verify_fail_cnt);
            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }
        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumGtfPrbsCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;

            // Check build info of xbtest HW IP top level
            auto build_info = this->m_device->GetGtfPrbsCuBI(cu_idx, CU_WRAPPER_IDX);
            LogMessage(MSG_DEBUG_TESTCASE, {"Check wrapper for for xbtest HW IP " + build_info.cu_name});
            test_it_failure |= CheckGtfPrbsCu(build_info, cu_idx, CU_WRAPPER_IDX, verify_pass_cnt, verify_fail_cnt);


            // Check build info of all cores of the xbtest HW IP
            auto gtf_indexes = this->m_device->GetGtfPrbsCuGTFIndexes(cu_idx);
            LogMessage(MSG_DEBUG_TESTCASE, {"Number of channels to be checked for xbtest HW IP " + build_info.cu_name + ": " + std::to_string(gtf_indexes.size())});
            for (uint gt_idx=0; gt_idx < gtf_indexes.size(); gt_idx++)
            {
                auto gt_index = gtf_indexes[gt_idx];
                LogMessage(MSG_DEBUG_TESTCASE, {"\t - Verifying build info for GT index " + std::to_string(gt_index)});
                auto ch_build_info = this->m_device->GetGtfPrbsCuBI(cu_idx, gt_idx);
                test_it_failure |= CheckGtfPrbsCu(ch_build_info, cu_idx, gt_index, verify_pass_cnt, verify_fail_cnt);
            }

            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }

        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumGtmPrbsCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;

            // Check build info of xbtest HW IP top level
            auto build_info = this->m_device->GetGtmPrbsCuBI(cu_idx, CU_WRAPPER_IDX);
            LogMessage(MSG_DEBUG_TESTCASE, {"Check wrapper for for xbtest HW IP " + build_info.cu_name});
            test_it_failure |= CheckGtmPrbsCu(build_info, cu_idx, CU_WRAPPER_IDX, verify_pass_cnt, verify_fail_cnt);


            // Check build info of all cores of the xbtest HW IP
            auto gtm_indexes = this->m_device->GetGtmPrbsCuGTMIndexes(cu_idx);
            LogMessage(MSG_DEBUG_TESTCASE, {"Number of channels to be checked for xbtest HW IP " + build_info.cu_name + ": " + std::to_string(gtm_indexes.size())});
            for (uint gt_idx=0; gt_idx < gtm_indexes.size(); gt_idx++)
            {
                auto gt_index = gtm_indexes[gt_idx];
                LogMessage(MSG_DEBUG_TESTCASE, {"\t - Verifying build info for GT index " + std::to_string(gt_index)});
                auto ch_build_info = this->m_device->GetGtmPrbsCuBI(cu_idx, gt_idx);
                test_it_failure |= CheckGtmPrbsCu(ch_build_info, cu_idx, gt_index, verify_pass_cnt, verify_fail_cnt);
            }

            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }
        for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumGtypPrbsCu()) && (!(this->m_abort)); cu_idx++)
        {
            auto test_it_failure = RET_SUCCESS;
            uint verify_pass_cnt = 0;
            uint verify_fail_cnt = 0;

            // Check build info of xbtest HW IP top level
            auto build_info = this->m_device->GetGtypPrbsCuBI(cu_idx, CU_WRAPPER_IDX);
            LogMessage(MSG_DEBUG_TESTCASE, {"Check wrapper for for xbtest HW IP " + build_info.cu_name});
            test_it_failure |= CheckGtypPrbsCu(build_info, cu_idx, CU_WRAPPER_IDX, verify_pass_cnt, verify_fail_cnt);


            // Check build info of all cores of the xbtest HW IP
            auto gtyp_indexes = this->m_device->GetGtypPrbsCuGTYPIndexes(cu_idx);
            LogMessage(MSG_DEBUG_TESTCASE, {"Number of channels to be checked for xbtest HW IP " + build_info.cu_name + ": " + std::to_string(gtyp_indexes.size())});
            for (uint gt_idx=0; gt_idx < gtyp_indexes.size(); gt_idx++)
            {
                auto gt_index = gtyp_indexes[gt_idx];
                LogMessage(MSG_DEBUG_TESTCASE, {"\t - Verifying build info for GT index " + std::to_string(gt_index)});
                auto ch_build_info = this->m_device->GetGtypPrbsCuBI(cu_idx, gt_idx);
                test_it_failure |= CheckGtypPrbsCu(ch_build_info, cu_idx, gt_index, verify_pass_cnt, verify_fail_cnt);
            }

            PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
            if (test_it_failure == RET_SUCCESS)
            {
                LogMessage(MSG_VER_007, {build_info.cu_name});
            }
            else
            {
                LogMessage(MSG_VER_008, {build_info.cu_name});
            }
            overall_verify_pass_cnt += verify_pass_cnt;
            overall_verify_fail_cnt += verify_fail_cnt;
            test_failure |= test_it_failure;
        }


        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            // Check build info
            for (uint cu_idx = 0; (cu_idx < this->m_device->GetNumMemoryCu(memory.type, memory.name)) && (!(this->m_abort)); cu_idx++)
            {
                auto test_it_failure = RET_SUCCESS;
                uint verify_pass_cnt = 0;
                uint verify_fail_cnt = 0;

                // Check build info of xbtest HW IP top level
                auto build_info = this->m_device->GetMemoryCuBI(memory.type, memory.name, cu_idx, CU_WRAPPER_IDX);
                test_it_failure |= CheckMemoryCu(build_info, memory.type, memory.name, cu_idx, verify_pass_cnt, verify_fail_cnt, CU_WRAPPER_IDX);

                // Check build info of all channels of Memory xbtest HW IPs. SC Memory xbtest HW IPs have 1 channel only
                LogMessage(MSG_DEBUG_TESTCASE, {"Number of channels to be checked for xbtest HW IP " + build_info.cu_name + ": " + std::to_string(memory.num_channels)});
                for (uint ch_idx = 0; ch_idx < memory.num_channels; ch_idx++)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"\t - Verifying build info for channel " + std::to_string(ch_idx)});
                    auto ch_build_info = this->m_device->GetMemoryCuBI(memory.type, memory.name, cu_idx, ch_idx);
                    test_it_failure |= CheckMemoryCu(ch_build_info, memory.type, memory.name, cu_idx, verify_pass_cnt, verify_fail_cnt, ch_idx);
                }
                PrintVerifyCuSummary(build_info.cu_name, verify_pass_cnt, verify_fail_cnt);
                if (test_it_failure == RET_SUCCESS)
                {
                    LogMessage(MSG_VER_007, {build_info.cu_name});
                }
                else
                {
                    LogMessage(MSG_VER_008, {build_info.cu_name});
                }
                overall_verify_pass_cnt += verify_pass_cnt;
                overall_verify_fail_cnt += verify_fail_cnt;
                test_failure |= test_it_failure;
            }
        }
        // Report Verify test skipped
        for (uint cu_idx = 0; cu_idx < this->m_device->GetNumSkippedCu(); cu_idx++)
        {
            LogMessage(MSG_VER_011, {this->m_device->GetSkippedCuName(cu_idx)});
        }
        // Report Verify test companion
        for (uint cu_idx = 0; cu_idx < this->m_device->GetNumCompanionCu(); cu_idx++)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Verify test not applicable for xbtest HW IP " + this->m_device->GetCompanionCuName(cu_idx)});
        }
        // Global verify task summary
        LogMessage(MSG_DEBUG_TESTCASE, {"Verify build info completed for all xbtest HW IPs"});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Total number of pass tests: " + std::to_string(overall_verify_pass_cnt)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Total number of fail tests: " + std::to_string(overall_verify_fail_cnt)});

        if (((overall_verify_pass_cnt + overall_verify_fail_cnt) == 0) && (!(this->m_abort)))
        {
            LogMessage(MSG_VER_009);
            test_failure = RET_FAILURE;
        }
    }

    if (test_failure == RET_SUCCESS)
    {
        if (!(this->m_abort))
        {
            if (StartCU() == RET_FAILURE)
            {
                Abort();
                test_failure |= RET_FAILURE;
            }
            else
            {
                test_failure |= EnableWatchdogClkThrotDetection();
                test_failure |= ReadDNA();
            }
        }
    }

    if (this->m_abort)
    {
        this->m_testcase_queue_value.failed++;
    }
    else if (test_failure == RET_FAILURE)
    {
        this->m_testcase_queue_value.failed++;
    }
    else
    {
        this->m_testcase_queue_value.passed++;
    }
    this->m_testcase_queue_value.pending--;
    this->m_testcase_queue_value.completed++;
    PushTestcaseQueue();

    this->pretest_done = true;
    return test_failure;
}


int VerifyTest::RunTest()
{
    //auto test_failure = RET_SUCCESS;
    // this->m_testcase_queue_value.remaining_time = NOT_APPLICABLE;
    // this->m_testcase_queue_value.parameters = NOT_APPLICABLE;
    // PushTestcaseQueue();

    // nothing done so far

    // Loop until abort
    while (!(this->m_abort))
    {
        this->m_timer->WaitFor1sTick();
        ResetWatchdog();
    }

    this->test_state  = TEST_STATE_PASS;

    this->m_testcase_queue_value.passed++;
    this->m_testcase_queue_value.pending--;
    this->m_testcase_queue_value.completed++;
    PushTestcaseQueue();

    return test_state;
}

bool VerifyTest::ReadDNA()
{
    auto ret = RET_SUCCESS;
    uint read_data;
    uint time_out = 0;
    if (this->m_device->GetVerifyCuDnaRead(this->m_cu_idx) == 1)
    {
        ReadVerifyCu(VERIFY_CTRL_REG_ADDR, read_data);
        if ((read_data & VERIFY_DNA_READ_AVAILABLE) == VERIFY_DNA_READ_AVAILABLE)
        {
            WriteVerifyCu(VERIFY_CTRL_REG_ADDR, VERIFY_DNA_READ_REQUEST);
            do
            {
                ReadVerifyCu(VERIFY_CTRL_REG_ADDR, read_data);
                // LogMessage(MSG_DEBUG_TESTCASE, {"DNA CTRL: 0x"+NumToStrHex<uint32_t>(read_data)});
                this->m_timer->WaitFor1sTick();
                time_out++;
            }
            while ( ((read_data & VERIFY_DNA_READ_DONE) != VERIFY_DNA_READ_DONE) && (time_out < DNA_READ_TIMEOUT) );

            if (time_out >= DNA_READ_TIMEOUT)
            {
                ret = RET_FAILURE;
                LogMessage(MSG_VER_013, {std::to_string(DNA_READ_TIMEOUT)});
            }
            else
            {
                uint DNA_0;
                uint DNA_1;
                uint DNA_2;
                ReadVerifyCu(VERIFY_DNA_0_REG_ADDR, DNA_0);
                ReadVerifyCu(VERIFY_DNA_1_REG_ADDR, DNA_1);
                ReadVerifyCu(VERIFY_DNA_2_REG_ADDR, DNA_2);
                LogMessage(MSG_VER_014, {NumToStrHex<uint32_t>(DNA_2,8),NumToStrHex<uint32_t>(DNA_1,8),NumToStrHex<uint32_t>(DNA_0,8)});
            }
        }
        else
        {
            LogMessage(MSG_VER_012);
        }
    }
    else
    {
        LogMessage(MSG_VER_012);
    }
    return ret;
}

bool VerifyTest::RunPostTests()
{
    auto test_failure = RET_SUCCESS;
    // if PreTestChecks wasn't executed, do not execute RunPostTests
    if (!this->pretest_done)
    {
        return RET_SUCCESS;
    }
    test_failure = CheckWatchdogAndClkThrottlingAlarms();

    if (!(this->m_apclk_throttling_alarm))
    {
        LogMessage(MSG_CMN_040, {"AP clock"});
    }
    if (!(this->m_apclk2_throttling_alarm))
    {
        LogMessage(MSG_CMN_040, {"AP 2 clock"});
    }

    test_failure |= StopCU();

    return test_failure;
}


bool VerifyTest::CheckVersion( const std::string & cu_name, const double & hw_version, const double & sw_version, const double & min_hw_version )
{
    if (hw_version >= min_hw_version)
    {
        if (std::trunc(hw_version) == std::trunc(sw_version))
        {
            LogMessage(MSG_VER_001, {cu_name, Float_to_String(hw_version,1), Float_to_String(sw_version,1)});
            return RET_SUCCESS;
        }
        LogMessage(MSG_VER_002, {cu_name, Float_to_String(hw_version,1), Float_to_String(sw_version,1)});
        return RET_FAILURE;
    }
    LogMessage(MSG_VER_003, {cu_name, Float_to_String(hw_version,1), Float_to_String(sw_version,1)});
    return RET_FAILURE;
}

bool VerifyTest::CheckVerifyCu( Build_Info & build_info, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;
    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version) + (double)(build_info.minor_version) / (double)(10.0);
    auto min_hw_version   = (double)(MIN_VERIFY_HW_VERSION_MAJOR) + (double)(MIN_VERIFY_HW_VERSION_MINOR) / (double)(10.0);
    auto sw_version       = (double)(VERIFY_SW_VERSION_MAJOR)     + (double)(VERIFY_SW_VERSION_MINOR)     / (double)(10.0);
    auto component_id     = BI_VERIFY_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (build_info.build_version != XBTEST_BUILD_VERSION)
    {
        LogMessage(MSG_VER_004, {build_info.cu_name, std::to_string(build_info.build_version), std::to_string(XBTEST_BUILD_VERSION)});
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);

        // Scratch pad test
        // Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        WriteVerifyCu(CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        ReadVerifyCu(CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckPowerCu( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;
    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version) + (double)(build_info.minor_version) / (double)(10.0);
    auto min_hw_version   = (double)(MIN_PWR_HW_VERSION_MAJOR) + (double)(MIN_PWR_HW_VERSION_MINOR) / (double)(10.0);
    auto sw_version       = (double)(PWR_SW_VERSION_MAJOR)     + (double)(PWR_SW_VERSION_MINOR)     / (double)(10.0);
    auto component_id     = BI_PWR_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);

        auto slr = this->m_device->GetPowerCuSlrIndex(cu_idx);
        ret |= VerifyBIValue<uint>(build_info, "SLR", build_info.slr, slr, verify_pass_cnt, verify_fail_cnt);

        auto expected_throttle_mode = PWR_THROTTLE_MODE_INTERNAL_MACRO;
        if (this->m_xbtest_sw_config->GetPwrCUThrottleMode(build_info.cu_name) == PWR_THROTTLE_MODE_INTERNAL_CLK_STR)
        {
            expected_throttle_mode = PWR_THROTTLE_MODE_INTERNAL_CLK;
        }
        ret |= VerifyBIValue<uint>(build_info, "Throttle mode", build_info.throttle_mode, expected_throttle_mode, verify_pass_cnt, verify_fail_cnt);

        auto expected_use_aie = this->m_xbtest_sw_config->GetPwrUseAIE(build_info.cu_name);
        ret |= VerifyBIValue<uint>(build_info, "AIE", build_info.use_aie, expected_use_aie, verify_pass_cnt, verify_fail_cnt);

        // Scratch pad test
        // Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WritePowerCu(cu_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadPowerCu(cu_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckGtmacCu( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;
    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version)    + (double)(build_info.minor_version)    / (double)(10.0);
    auto min_hw_version   = (double)(MIN_GT_MAC_HW_VERSION_MAJOR) + (double)(MIN_GT_MAC_HW_VERSION_MINOR) / (double)(10.0);
    auto sw_version       = (double)(GT_MAC_SW_VERSION_MAJOR)     + (double)(GT_MAC_SW_VERSION_MINOR)     / (double)(10.0);
    auto component_id     = BI_GT_MAC_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);

        auto gt_index = this->m_device->GetGtmacCuGTIndex(cu_idx);
        ret |= VerifyBIValue<uint>(build_info, "GT index", build_info.gt_index, gt_index, verify_pass_cnt, verify_fail_cnt);

        // Scratch pad test, Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteGtmacCu(cu_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadGtmacCu(cu_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckGtlpbkCu( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;

    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version)     + (double)(build_info.minor_version)      / (double)(10.0);
    auto min_hw_version   = (double)(MIN_GT_LPBK_HW_VERSION_MAJOR) + (double)(MIN_GT_LPBK_HW_VERSION_MINOR)  / (double)(10.0);
    auto sw_version       = (double)(GT_LPBK_SW_VERSION_MAJOR)     + (double)(GT_LPBK_SW_VERSION_MINOR)      / (double)(10.0);
    auto component_id     = BI_GT_LPBK_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);

        auto gt_index = this->m_device->GetGtlpbkCuGTIndex(cu_idx);
        ret |= VerifyBIValue<uint>(build_info, "GT index", build_info.gt_index, gt_index, verify_pass_cnt, verify_fail_cnt);

        // Scratch pad test, Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteGtlpbkCu(cu_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadGtlpbkCu(cu_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckGtPrbsCu( Build_Info & build_info, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;

    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version)     + (double)(build_info.minor_version)      / (double)(10.0);
    auto min_hw_version   = (double)(MIN_GT_PRBS_HW_VERSION_MAJOR) + (double)(MIN_GT_PRBS_HW_VERSION_MINOR)  / (double)(10.0);
    auto sw_version       = (double)(GT_PRBS_SW_VERSION_MAJOR)     + (double)(GT_PRBS_SW_VERSION_MINOR)      / (double)(10.0);
    auto component_id     = BI_GT_PRBS_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);

        auto gt_index = this->m_device->GetGtPrbsCuGTIndex(cu_idx);
        ret |= VerifyBIValue<uint>(build_info, "GT index", build_info.gt_index, gt_index, verify_pass_cnt, verify_fail_cnt);

        // Scratch pad test, Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteGtPrbsCu(cu_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadGtPrbsCu(cu_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckGtfPrbsCu( Build_Info & build_info, const uint & cu_idx, const int & gt_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;

    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version)      + (double)(build_info.minor_version)       / (double)(10.0);
    auto min_hw_version   = (double)(MIN_GTF_PRBS_HW_VERSION_MAJOR) + (double)(MIN_GTF_PRBS_HW_VERSION_MINOR)  / (double)(10.0);
    auto sw_version       = (double)(GTF_PRBS_SW_VERSION_MAJOR)     + (double)(GTF_PRBS_SW_VERSION_MINOR)      / (double)(10.0);
    auto component_id     = BI_GTF_PRBS_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);


        if (gt_idx == CU_WRAPPER_IDX) // Check this only for wrapper
        {
            ret |= VerifyBIValue<uint>(build_info, "Number max of GTs",   build_info.gt_num_gt,  MAX_GT_SUPPORTED, verify_pass_cnt, verify_fail_cnt);
        }
        else
        {
            ret |= VerifyBIValue<uint>(build_info, "GT index",  build_info.gt_index,           gt_idx, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<bool>(build_info, "GT Enable", build_info.gt_enable,            TRUE, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<uint>(build_info, "GT rate",   build_info.gt_rate,  GT_RATE_FIX_10GBE, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<uint>(build_info, "GT IP SEL", build_info.gt_ip_sel, GT_IP_GTF_WIZARD, verify_pass_cnt, verify_fail_cnt);
        }

        // ret |= VerifyBIValue<uint>(build_info, "Number of lanes", build_info.gt_num_lane, 4, verify_pass_cnt, verify_fail_cnt);


        // Scratch pad test, Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteGtfPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadGtfPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckGtmPrbsCu( Build_Info & build_info, const uint & cu_idx, const int & gt_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;

    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version)      + (double)(build_info.minor_version)       / (double)(10.0);
    auto min_hw_version   = (double)(MIN_GTM_PRBS_HW_VERSION_MAJOR) + (double)(MIN_GTM_PRBS_HW_VERSION_MINOR)  / (double)(10.0);
    auto sw_version       = (double)(GTM_PRBS_SW_VERSION_MAJOR)     + (double)(GTM_PRBS_SW_VERSION_MINOR)      / (double)(10.0);
    auto component_id     = BI_GTM_PRBS_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);


        if (gt_idx == CU_WRAPPER_IDX) // Check this only for wrapper
        {
            ret |= VerifyBIValue<uint>(build_info, "Number max of GTs",   build_info.gt_num_gt,  MAX_GT_SUPPORTED, verify_pass_cnt, verify_fail_cnt);
        }
        else
        {
            ret |= VerifyBIValue<uint>(build_info, "GT index",  build_info.gt_index,           gt_idx, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<bool>(build_info, "GT Enable", build_info.gt_enable,            TRUE, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<uint>(build_info, "GT rate",   build_info.gt_rate,  GT_RATE_FIX_56GBE, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<uint>(build_info, "GT IP SEL", build_info.gt_ip_sel, GT_IP_SUBSYSTEM_GT, verify_pass_cnt, verify_fail_cnt);
        }

        // ret |= VerifyBIValue<uint>(build_info, "Number of lanes", build_info.gt_num_lane, 4, verify_pass_cnt, verify_fail_cnt);


        // Scratch pad test, Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteGtmPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadGtmPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckGtypPrbsCu( Build_Info & build_info, const uint & cu_idx, const int & gt_idx, uint & verify_pass_cnt, uint & verify_fail_cnt)
{
    auto ret = RET_SUCCESS;

    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version)       + (double)(build_info.minor_version)        / (double)(10.0);
    auto min_hw_version   = (double)(MIN_GTYP_PRBS_HW_VERSION_MAJOR) + (double)(MIN_GTYP_PRBS_HW_VERSION_MINOR)  / (double)(10.0);
    auto sw_version       = (double)(GTYP_PRBS_SW_VERSION_MAJOR)     + (double)(GTYP_PRBS_SW_VERSION_MINOR)      / (double)(10.0);
    auto component_id     = BI_GTYP_PRBS_HW_COMPONENT_ID;

    if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
    {
        verify_pass_cnt++;
    }
    else
    {
        verify_fail_cnt++;
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);


        if (gt_idx == CU_WRAPPER_IDX) // Check this only for wrapper
        {
            ret |= VerifyBIValue<uint>(build_info, "Number max of GTs",   build_info.gt_num_gt,  MAX_GT_SUPPORTED, verify_pass_cnt, verify_fail_cnt);
        }
        else
        {
            ret |= VerifyBIValue<uint>(build_info, "GT index",  build_info.gt_index,           gt_idx, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<bool>(build_info, "GT Enable", build_info.gt_enable,            TRUE, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<uint>(build_info, "GT rate",   build_info.gt_rate,  GT_RATE_FIX_32GBS_AURORA, verify_pass_cnt, verify_fail_cnt);
            ret |= VerifyBIValue<uint>(build_info, "GT IP SEL", build_info.gt_ip_sel, GT_IP_SUBSYSTEM_GT, verify_pass_cnt, verify_fail_cnt);
        }

        // ret |= VerifyBIValue<uint>(build_info, "Number of lanes", build_info.gt_num_lane, 4, verify_pass_cnt, verify_fail_cnt);


        // Scratch pad test, Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteGtypPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadGtypPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

bool VerifyTest::CheckMemoryCu( Build_Info & build_info, const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, uint & verify_pass_cnt, uint & verify_fail_cnt, const int & ch_idx)
{
    auto ret = RET_SUCCESS;
    // from SW version 3.4 and onwards, only major HW & SW should match
    // make sure than xbtest HW IP is above its minimal version too
    auto hw_version       = (double)(build_info.major_version) + (double)(build_info.minor_version) / (double)(10.0);
    auto min_hw_version   = (double)(MIN_MEM_HW_VERSION_MAJOR) + (double)(MIN_MEM_HW_VERSION_MINOR) / (double)(10.0);
    auto sw_version       = (double)(MEM_SW_VERSION_MAJOR)     + (double)(MEM_SW_VERSION_MINOR)     / (double)(10.0);
    auto component_id     = BI_MEM_HW_COMPONENT_ID;

    if (ch_idx == CU_WRAPPER_IDX) // Only check for the wrapper, don't check for the sub-cores
    {
        if (CheckVersion(build_info.cu_name, hw_version, sw_version, min_hw_version) == RET_SUCCESS)
        {
            verify_pass_cnt++;
        }
        else
        {
            verify_fail_cnt++;
            ret = RET_FAILURE;
        }
    }

    if (build_info.build_version != XBTEST_BUILD_VERSION)
    {
        LogMessage(MSG_VER_004, {build_info.cu_name, std::to_string(build_info.build_version), std::to_string(XBTEST_BUILD_VERSION)});
    }

    if (ret == RET_SUCCESS)
    {
        ret |= VerifyBIValue<uint> (build_info, "Component ID",     build_info.component_id, component_id, verify_pass_cnt, verify_fail_cnt);

        auto mem_num_channels = this->m_xbtest_sw_config->GetMemCUNumChannels(build_info.cu_name);
        ret |= VerifyBIValue<uint>(build_info, "Number of cores", build_info.mem_num_channels, mem_num_channels, verify_pass_cnt, verify_fail_cnt);

        uint mem_type;
        if (memory_type == SINGLE_CHANNEL)
        {
            mem_type = BI_MEM_TYPE_SINGLE_CHANNEL;
        }
        else
        {
            mem_type = BI_MEM_TYPE_MULTI_CHANNEL;
        }
        ret |= VerifyBIValue<uint>(build_info, "Memory type", build_info.mem_type, mem_type, verify_pass_cnt, verify_fail_cnt);

        if (ch_idx != CU_WRAPPER_IDX) // Check this only for Memory xbtest HW IP channel
        {
            int mem_ch_idx = ch_idx + 1;
            ret |= VerifyBIValue<int>(build_info, "xbtest HW IP channel", build_info.mem_ch_idx, mem_ch_idx, verify_pass_cnt, verify_fail_cnt);
        }

        // Scratch pad test
        // Check default value of scratch pad
        if ((build_info.scratch_pad != (uint)(0xFFFF0000)) && (build_info.scratch_pad != (uint)(0x0000FFFF)))
        {
            LogMessage(MSG_VER_005, {build_info.cu_name, "Scratch pad read test", NumToStrHex<uint>(build_info.scratch_pad), "0xFFFF0000 or 0x0000FFFF"});
            ret = RET_FAILURE;
            verify_fail_cnt++;
        }
        else
        {
            verify_pass_cnt++;
        }

        uint scratch_pad_2 = (~build_info.scratch_pad) & 0xFFFFFFFF; // Compute next expected scratch pad

        this->m_device->WriteMemoryCu(memory_type, memory_name, cu_idx, ch_idx, CMN_SCRATCH_PAD_ADDR, ~(build_info.scratch_pad & 0x1)); // Write to scratch pad

        this->m_device->ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, CMN_SCRATCH_PAD_ADDR, build_info.scratch_pad); // Read new scratch pad
        ret |= VerifyBIValue<uint>(build_info, "Scratch pad write test", build_info.scratch_pad, scratch_pad_2, verify_pass_cnt, verify_fail_cnt);
    }
    return ret;
}

void VerifyTest::PrintVerifyCuSummary( const std::string & cu_name, const uint & verify_pass_cnt, const uint & verify_fail_cnt )
{
    LogMessage(MSG_DEBUG_TESTCASE, {"Verify build info completed for xbtest HW IP " + cu_name});
    LogMessage(MSG_DEBUG_TESTCASE, {"\t - Number of pass tests: " + std::to_string(verify_pass_cnt)});
    LogMessage(MSG_DEBUG_TESTCASE, {"\t - Number of fail tests: " + std::to_string(verify_fail_cnt)});
}

bool VerifyTest::StartCU()
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Start Verify xbtest HW IP"});

    // start the xbtest HW IP and check if the "already started" is received back
    // if it received back, check if the watchdog triggered before, meaning that previous test was abruptly stopped (kill process or terminal closed)
    //      if watchdog is enabled and alarm is present, we can safely
    //          1) clear watchdog
    //          2) start the test
    //      if no alarm or not enable, the previous run of xbtest was left in an unknown state so ask for user to reload/reset xbtest HW design
    // the watchdog doesn't clear the start bit
    // the watchdog is always disabled at the of the test

    WriteVerifyCu(CMN_CTRL_STATUS_ADDR, CMN_STATUS_START);

    ReadVerifyCu(CMN_CTRL_STATUS_ADDR, read_data);

    if ((read_data & CMN_STATUS_ALREADY_START) == CMN_STATUS_ALREADY_START)
    {
        ReadVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, read_data);

        // check if watchdog is already enable and error is detected
        if ( ((read_data & CMN_WATCHDOG_EN) == CMN_WATCHDOG_EN) && ((read_data & CMN_WATCHDOG_ALARM) == CMN_WATCHDOG_ALARM) )
        {
            LogMessage(MSG_CMN_027);
            // it's safe to restart the xbtest HW IP, but first clear the start bit and the watchdog
            // stop the xbtest HW IP
            WriteVerifyCu(CMN_CTRL_STATUS_ADDR, 0x0); // this also clears the already_start bit
            // stop watchdog and clear error
            WriteVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, CMN_WATCHDOG_ALARM);
            // start the test
            WriteVerifyCu(CMN_CTRL_STATUS_ADDR, CMN_STATUS_START);
        }
        else
        {
            LogMessage(MSG_CMN_028);
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool VerifyTest::EnableWatchdogClkThrotDetection()
{
    uint read_data;
    // check if there was an alarm present (for reporting only,no error generation)

    ReadVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, read_data);

    if ((read_data & CMN_WATCHDOG_ALARM) == CMN_WATCHDOG_ALARM)
    {
        LogMessage(MSG_CMN_029);
    }
    // check clock throttle detection module is present
    // It must be there. It runs on clk continuous and this clock is needed for measurements & watchdog
    // it's alternative way of checking that clk cont is connected
    if ((read_data & CMN_THROTTLE_APCLK_AVAIL) != CMN_THROTTLE_APCLK_AVAIL)
    {
        LogMessage(MSG_CMN_039_ERROR, {"AP clock"});
        return RET_FAILURE;
    }
    // configure watchdog
    WriteVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, GetWatchdogConfigData(this->m_devicemgt->GetWatchdogDuration()));
    this->m_watchdog_reset_rate = this->m_devicemgt->GetWatchdogDuration() / WATCHDOG_RESET_RATE;

    return RET_SUCCESS;
}

bool VerifyTest::StopCU()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    // disable watchdog & clock throttling mon and clear any alarm detected

    WriteVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, CMN_WATCHDOG_ALARM | CMN_THROTTLE_ALARM_RST);

    // stop the xbtest HW IP and check if the "already started" is present, meaning that another test has tried to start the xbtest HW IP
    ReadVerifyCu(CMN_CTRL_STATUS_ADDR, read_data);

    if ((read_data & CMN_STATUS_ALREADY_START) == CMN_STATUS_ALREADY_START)
    {
        LogMessage(MSG_CMN_030, {"Verify xbtest HW IP"});
        ret = RET_FAILURE;
    }
    // stop the xbtest HW IP
    WriteVerifyCu(CMN_CTRL_STATUS_ADDR, 0x0);
    return ret;
}

bool VerifyTest::CheckWatchdogAndClkThrottlingAlarms()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Check watchdog alarm & Clk throttle detection"});

    ReadVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, read_data);

    // check watchdog alarm
    if ( ((read_data & CMN_WATCHDOG_EN) == CMN_WATCHDOG_EN) && ((read_data & CMN_WATCHDOG_ALARM) == CMN_WATCHDOG_ALARM) )
    {
        if (!(this->m_watchdog_alarm))
        {
            LogMessage(MSG_CMN_031);
            this->m_watchdog_alarm = true;
            // clear error & reset watchdog
            WriteVerifyCu(CMN_WATCHDOG_THROTTLE_ADDR, read_data | CMN_WATCHDOG_ALARM | CMN_WATCHDOG_RST);
        }
        ret = RET_FAILURE;
    }
    // check clock throttle detection alarm
    // clock throttle is not considered as an error, it's a warning
    if ((read_data & CMN_THROTTLE_MON_ENABLE) == CMN_THROTTLE_MON_ENABLE)
    {
        if (!(this->m_apclk_throttling_alarm))
        {
            if ((read_data & CMN_THROTTLE_APCLK_DETECT) == CMN_THROTTLE_APCLK_DETECT)
            {
                LogMessage(MSG_CMN_041, {"AP clock"});
                this->m_apclk_throttling_alarm = true;
            }
        }
        if (!(this->m_apclk2_throttling_alarm))
        {
            if ((read_data & CMN_THROTTLE_APCLK2_AVAIL) == CMN_THROTTLE_APCLK2_AVAIL)
            {
                if ((read_data & CMN_THROTTLE_APCLK2_DETECT) == CMN_THROTTLE_APCLK2_DETECT)
                {
                    LogMessage(MSG_CMN_041, {"AP 2 clock"});
                    this->m_apclk2_throttling_alarm = true;
                }
            }
            else
            {
                LogMessage(MSG_CMN_039_WARN, {"AP 2 clock"});
                this->m_apclk2_throttling_alarm = true;
            }
        }
    }

    return ret;
}

} // namespace
