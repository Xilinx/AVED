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

#include "mmiotest.h"

#if defined(USE_AMI) || defined(USE_NO_DRIVER)

namespace xbtest
{

MmioTest::MmioTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, const Mmio_Parameters_t & test_parameters
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_log_header        = LOG_HEADER_MMIO;
    this->m_test_parameters   = test_parameters;

    this->m_queue_testcase    = MMIO_MEMBER.name;
    this->m_queue_thread      = MMIO_MEMBER.name;
}

MmioTest::~MmioTest () = default;

bool MmioTest::PreSetup()
{
    auto global_settings_failure = RET_SUCCESS;
    this->m_state = TestState::TS_PRE_SETUP;

    LogMessage(MSG_CMN_004);

    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }

    global_settings_failure |= GetTestSequence();

    this->m_page_size = getpagesize();
    LogMessage(MSG_MIO_001, {std::to_string(this->m_page_size)});

    // Get SLR0 configuration and status and status memory as its range will be used as default parameter for MMIO test
    std::string ep_name = "cfg_stat_mem_slr0_ctrl";
    uint64_t    ep_axi_addr; // not used
    if (this->m_device->GetEndPointConfig(ep_name, this->m_TC_Cfg.nom_buffer_size, this->m_TC_Cfg.nom_offset, this->m_TC_Cfg.nom_bar, ep_axi_addr) == RET_FAILURE)
    {
        global_settings_failure = RET_FAILURE;
    }
    this->m_TC_Cfg.nom_total_size = this->m_TC_Cfg.nom_buffer_size;

    // Get parameters defined in xbtest_pfm_def.json and overwrite parameters if present in JSON file
    auto mmio_def = this->m_xbtest_sw_config->GetMmioDefinitions();

    if (mmio_def.config.total_size_exists)
    {
        this->m_TC_Cfg.nom_total_size = mmio_def.config.total_size;
        LogMessage(MSG_DEBUG_TESTCASE, {"Using nominal " + TOT_SIZE + " defined in card definition: " + std::to_string(this->m_TC_Cfg.nom_total_size)});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Using default nominal " + TOT_SIZE + ": " + std::to_string(this->m_TC_Cfg.nom_total_size)});
    }
    if (mmio_def.config.buffer_size_exists)
    {
        this->m_TC_Cfg.nom_buffer_size = mmio_def.config.buffer_size;
        LogMessage(MSG_DEBUG_TESTCASE, {"Using nominal " + BUFFER_SIZE + " defined in card definition: " + std::to_string(this->m_TC_Cfg.nom_buffer_size)});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Using default nominal " + BUFFER_SIZE + ": " + std::to_string(this->m_TC_Cfg.nom_buffer_size)});
    }
    if (mmio_def.config.bar_exists)
    {
        this->m_TC_Cfg.nom_bar = mmio_def.config.bar;
        LogMessage(MSG_DEBUG_TESTCASE, {"Using nominal " + BAR + " defined in card definition: " + std::to_string(this->m_TC_Cfg.nom_bar)});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Using default nominal " + BAR + ": " + std::to_string(this->m_TC_Cfg.nom_bar)});
    }
    if (mmio_def.config.offset_exists)
    {
        this->m_TC_Cfg.nom_offset = mmio_def.config.offset;
        LogMessage(MSG_DEBUG_TESTCASE, {"Using nominal " + OFFSET + " defined in card definition: " + NumToStrHex(this->m_TC_Cfg.nom_offset, 16)});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Using default nominal " + OFFSET + ": 0x" + NumToStrHex(this->m_TC_Cfg.nom_offset, 16)});
    }

    // Get global parameter from test JSON

    this->m_TC_Cfg.check_bw_exists = false;
    auto it = FindJsonParam(this->m_test_parameters.global_config, CHECK_BW_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_TC_Cfg.check_bw_exists = true;
    }
    global_settings_failure |= GetJsonParamBool(CHECK_BW_MEMBER,                this->m_test_parameters.global_config, this->m_TC_Cfg.check_bw,              false);
    global_settings_failure |= GetJsonParamBool(CHECK_DATA_INTEGRITY_MEMBER,    this->m_test_parameters.global_config, this->m_TC_Cfg.check_data_integrity,  true);
    global_settings_failure |= GetJsonParamBool(STOP_ON_ERROR_MEMBER,           this->m_test_parameters.global_config, this->m_TC_Cfg.stop_on_error,         false);

    global_settings_failure |= GetJsonParamNum<uint64_t>(TOTAL_SIZE_BYTES_MEMBER, this->m_test_parameters.global_config, MIN_TOTAL_SIZE, this->m_TC_Cfg.nom_total_size, MAX_TOTAL_SIZE, this->m_TC_Cfg.total_size);

    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_WR_KBPS_MEMBER, this->m_test_parameters.global_config, MIN_LO_THRESH_WR, mmio_def.bw.write.low,  MAX_LO_THRESH_WR, this->m_TC_Cfg.lo_thresh_wr);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_WR_KBPS_MEMBER, this->m_test_parameters.global_config, MIN_HI_THRESH_WR, mmio_def.bw.write.high, MAX_HI_THRESH_WR, this->m_TC_Cfg.hi_thresh_wr);

    global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_RD_KBPS_MEMBER, this->m_test_parameters.global_config, MIN_LO_THRESH_RD, mmio_def.bw.read.low,   MAX_LO_THRESH_RD, this->m_TC_Cfg.lo_thresh_rd);
    global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_RD_KBPS_MEMBER, this->m_test_parameters.global_config, MIN_HI_THRESH_RD, mmio_def.bw.read.high,  MAX_HI_THRESH_RD, this->m_TC_Cfg.hi_thresh_rd);

    global_settings_failure |= CheckThresholdLoVsHi<uint>(LO_THRESH_WR_KBPS_MEMBER, this->m_TC_Cfg.lo_thresh_wr, HI_THRESH_WR_KBPS_MEMBER, this->m_TC_Cfg.hi_thresh_wr);
    global_settings_failure |= CheckThresholdLoVsHi<uint>(LO_THRESH_RD_KBPS_MEMBER, this->m_TC_Cfg.lo_thresh_rd, HI_THRESH_RD_KBPS_MEMBER, this->m_TC_Cfg.hi_thresh_rd);

    // Output files
    std::vector<std::string> first_line_detail;
    first_line_detail.emplace_back("Test");
    first_line_detail.emplace_back(BAR);
    first_line_detail.emplace_back(OFFSET);
    first_line_detail.emplace_back(BUFFER_SIZE + " (Bytes)");
    first_line_detail.emplace_back("Cycle ID");
    first_line_detail.emplace_back("Data Integrity");
    first_line_detail.emplace_back(WR_LIV_BW + " (kBps)");
    first_line_detail.emplace_back(WR_MIN_BW + " (kBps)");
    first_line_detail.emplace_back(WR_AVG_BW + " (kBps)");
    first_line_detail.emplace_back(WR_MAX_BW + " (kBps)");
    first_line_detail.emplace_back(RD_LIV_BW + " (kBps)");
    first_line_detail.emplace_back(RD_MIN_BW + " (kBps)");
    first_line_detail.emplace_back(RD_AVG_BW + " (kBps)");
    first_line_detail.emplace_back(RD_MAX_BW + " (kBps)");

    std::vector<std::string> first_line_result;
    first_line_result.emplace_back("Test");
    first_line_result.emplace_back(DURATION    + " (s)");
    first_line_result.emplace_back(BAR);
    first_line_result.emplace_back(OFFSET);
    first_line_result.emplace_back(BUFFER_SIZE + " (Bytes)");
    first_line_result.emplace_back(BUFFER_CNT);
    first_line_result.emplace_back(TOT_SIZE + " (Bytes)");
    first_line_result.emplace_back("Number of cycles");
    first_line_result.emplace_back("Data Integrity");
    first_line_result.emplace_back(WR_MIN_BW + " (kBps)");
    first_line_result.emplace_back(WR_AVG_BW + " (kBps)");
    first_line_result.emplace_back(WR_MAX_BW + " (kBps)");
    first_line_result.emplace_back(RD_MIN_BW + " (kBps)");
    first_line_result.emplace_back(RD_AVG_BW + " (kBps)");
    first_line_result.emplace_back(RD_MAX_BW + " (kBps)");

    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {
        this->m_outputfile_name = "mmio";
        this->m_use_outputfile  = true;
        global_settings_failure |= OpenOutputFile(this->m_outputfile_name + "_detail.csv", this->m_outputfile_detail);
        global_settings_failure |= OpenOutputFile(this->m_outputfile_name + "_result.csv", this->m_outputfile_result);

        this->m_RT_outputfile_detail_name = this->m_outputfile_name + "_detail.csv";
        global_settings_failure |= OpenRTOutputFile(this->m_RT_outputfile_detail_name, this->m_RT_outputfile_detail);

        if (global_settings_failure == RET_SUCCESS)
        {
            this->m_outputfile_detail << StrVectToStr(first_line_detail, ",") << "\n";
            this->m_outputfile_detail.flush();

            this->m_outputfile_result << StrVectToStr(first_line_result, ",") << "\n";
            this->m_outputfile_result.flush();

            this->m_RT_outputfile_detail_head = StrVectToStr(first_line_detail, ",");
            this->m_RT_outputfile_detail.close();
        }
    }

    LogMessage(MSG_CMN_021, {"Global configuration:"});
    LogMessage(MSG_CMN_021, {"\t- " + CHECK_BW_MEMBER.name              + ": " +      BoolToStr(this->m_TC_Cfg.check_bw)});
    LogMessage(MSG_CMN_021, {"\t- " + CHECK_DATA_INTEGRITY_MEMBER.name  + ": " +      BoolToStr(this->m_TC_Cfg.check_data_integrity)});
    LogMessage(MSG_CMN_021, {"\t- " + TOTAL_SIZE_BYTES_MEMBER.name      + ": " + std::to_string(this->m_TC_Cfg.total_size)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_WR_KBPS_MEMBER.name          + ": " + std::to_string(this->m_TC_Cfg.lo_thresh_wr)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_WR_KBPS_MEMBER.name          + ": " + std::to_string(this->m_TC_Cfg.hi_thresh_wr)});
    LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_RD_KBPS_MEMBER.name          + ": " + std::to_string(this->m_TC_Cfg.lo_thresh_rd)});
    LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_RD_KBPS_MEMBER.name          + ": " + std::to_string(this->m_TC_Cfg.hi_thresh_rd)});
    LogMessage(MSG_CMN_021, {"\t- " + STOP_ON_ERROR_MEMBER.name         + ": " +      BoolToStr(this->m_TC_Cfg.stop_on_error)});


    if (global_settings_failure == RET_SUCCESS)
    {
        global_settings_failure |= ParseTestSequenceSettings(this->m_test_it_list);
    }

    if (global_settings_failure == RET_FAILURE)
    {
        Abort();
    }
    return global_settings_failure;
}

void MmioTest::Run()
{
    this->m_state       = TestState::TS_RUNNING;
    this->m_result      = TestResult::TR_FAILED;
    auto test_state     = TEST_STATE_FAILURE;

    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});
        test_state = RunTest();
    }

    if ((test_state < TEST_STATE_PASS) || (this->m_abort)) // any negatif state is an abort code
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (test_state > TEST_STATE_PASS) // any positive state is an error code
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

void MmioTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void MmioTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

bool MmioTest::GetTestSequence()
{
    this->m_test_it_results.clear();
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_test_sequence = TestcaseParamCast<std::vector<Mmio_Test_Sequence_Parameters_t>>(it->second);
        for (uint ii = 0; ii < this->m_test_sequence.size(); ii++)
        {
            this->m_test_it_results.emplace_back(TITR_NOT_TESTED);
        }
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

void MmioTest::SetTestSequenceStr( const Mmio_Test_Sequence_Parameters_t & test_seq_param, TestItConfig_t & test_it_cfg )
{
    test_it_cfg.test_strvect.clear();
    test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name              + "\": "   + std::to_string(test_seq_param.duration.value));
    if (test_seq_param.bar.exists)
    {
        test_it_cfg.test_strvect.emplace_back("\"" + BAR_TEST_SEQ_MEMBER.name                   + "\": "   + std::to_string(test_seq_param.bar.value));
    }
    if (test_seq_param.offset.exists)
    {
        test_it_cfg.test_strvect.emplace_back("\"" + OFFSET_TEST_SEQ_MEMBER.name                + "\": 0x" +    NumToStrHex(test_seq_param.offset.value, 16));
    }
    if (test_seq_param.buffer_size.exists)
    {
        test_it_cfg.test_strvect.emplace_back("\"" + BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER.name     + "\": "   + std::to_string(test_seq_param.buffer_size.value));
    }
}

void MmioTest::SetTestSequenceStrDbg( TestItConfig_t & test_it_cfg )
{
    test_it_cfg.test_strvect_dbg.clear();
    test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name              + "\": "   + std::to_string(test_it_cfg.duration));
    test_it_cfg.test_strvect_dbg.emplace_back("\"" + BAR_TEST_SEQ_MEMBER.name                   + "\": "   + std::to_string(test_it_cfg.bar));
    test_it_cfg.test_strvect_dbg.emplace_back("\"" + OFFSET_TEST_SEQ_MEMBER.name                + "\": 0x" +    NumToStrHex(test_it_cfg.offset, 16));
    test_it_cfg.test_strvect_dbg.emplace_back("\"" + BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER.name     + "\": "   + std::to_string(test_it_cfg.buffer_size));
}

bool MmioTest::ParseTestSequenceSettings( std::list<TestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    uint parent_test_cnt = 0;

    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_seq_strvect_dbg;

    // Get PCIe BAR configuration and parse BDF
    auto pcie_bars = this->m_xbtest_sw_config->GetPcieBars();
    auto bdf = this->m_xbtest_sw_config->GetBdf();

    int dom;
    int b;
    int d;
    int f;

    if (CheckCardBDF(bdf, dom, b, d, f) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    for (const auto & test_seq_param : this->m_test_sequence)
    {
        if (this->m_abort)
        {
            break;
        }

        auto parse_it_failure = RET_SUCCESS;
        TestItConfig_t test_it_cfg;

        parent_test_cnt++;
        test_it_cfg.parent_test_idx = parent_test_cnt;

        // duration
        if (parse_it_failure == RET_SUCCESS)
        {
            test_it_cfg.duration = test_seq_param.duration.value;
            if (CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_it_cfg.duration, MIN_DURATION, MAX_DURATION) == RET_FAILURE)
            {
                parse_it_failure = RET_FAILURE;
            }
        }

        // bar
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_seq_param.bar.exists)
            {
                test_it_cfg.bar = test_seq_param.bar.value;
            }
            else
            {
                test_it_cfg.bar = this->m_TC_Cfg.nom_bar;
            }
            if (CheckParam<uint8_t>(BAR_TEST_SEQ_MEMBER.name, test_it_cfg.bar, MIN_BAR, MAX_BAR) == RET_FAILURE)
            {
                parse_it_failure = RET_FAILURE;
            }
        }

        // Get BAR configuration and update maximum possible values depending on BAR selected by user
        std::vector<uint8_t> valid_bars;
        auto found = false;
        PCIe_Bar_Ep_t bar_cfg;

        if (parse_it_failure == RET_SUCCESS)
        {
            // Check BAR provided exists
            for (const auto & bar: pcie_bars)
            {
                if (bar.pcie_physical_function == (uint)f) // Currently only PF 0 supported
                {
                    bar_cfg = bar;
                    valid_bars.emplace_back(bar.pcie_base_address_register);

                    if (bar.pcie_base_address_register == (uint)test_it_cfg.bar)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                LogMessage(MSG_MIO_005,{NumVectToStr<uint8_t>(valid_bars, ",")}); // Invalid settings provided
                parse_it_failure = RET_FAILURE;
            }
        }

        // total_size
        // Saturate total size to PCIe BAR range
        test_it_cfg.total_size = this->m_TC_Cfg.total_size;
        test_it_cfg.max_total_size = bar_cfg.range;

        if (test_it_cfg.total_size > test_it_cfg.max_total_size)
        {
            test_it_cfg.total_size = test_it_cfg.max_buffer_size;
            LogMessage(MSG_DEBUG_TESTCASE, {"Test " + std::to_string(test_list.size()+1) + " Total size saturated to " + std::to_string(test_it_cfg.total_size)});
        }

        // buffer_size
        test_it_cfg.nom_buffer_size = this->m_TC_Cfg.nom_buffer_size;
        test_it_cfg.max_buffer_size = test_it_cfg.total_size;
        if (test_it_cfg.nom_buffer_size > test_it_cfg.total_size)
        {
            test_it_cfg.nom_buffer_size = test_it_cfg.total_size;
            LogMessage(MSG_DEBUG_TESTCASE, {"Test " + std::to_string(test_list.size()+1) + " Nominal buffer size saturated to " + std::to_string(test_it_cfg.nom_buffer_size)});
        }

        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_seq_param.buffer_size.exists)
            {
                test_it_cfg.buffer_size = test_seq_param.buffer_size.value;
            }
            else
            {
                test_it_cfg.buffer_size = test_it_cfg.nom_buffer_size;
            }
            if (CheckParam<uint64_t>(BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER.name, test_it_cfg.buffer_size, MIN_BUFFER_SIZE, test_it_cfg.max_buffer_size) == RET_FAILURE)
            {
                parse_it_failure = RET_FAILURE;
            }
        }

        // offset

        // Maximum possible value of offset depend on buffer size provided
        test_it_cfg.nom_offset = this->m_TC_Cfg.nom_offset;
        test_it_cfg.max_offset = test_it_cfg.max_total_size - test_it_cfg.total_size;
        if (test_it_cfg.nom_offset > test_it_cfg.max_offset)
        {
            test_it_cfg.nom_offset = test_it_cfg.max_offset;
            LogMessage(MSG_DEBUG_TESTCASE, {"Test " + std::to_string(test_list.size()+1) + " Nominal offset saturated to " + std::to_string(test_it_cfg.nom_offset)});
        }

        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_seq_param.offset.exists)
            {
                test_it_cfg.offset = test_seq_param.offset.value;
            }
            else
            {
                test_it_cfg.offset = test_it_cfg.nom_offset;
            }
            if (CheckParam<uint64_t>(OFFSET_TEST_SEQ_MEMBER.name, test_it_cfg.offset, MIN_OFFSET, test_it_cfg.max_offset) == RET_FAILURE)
            {
                parse_it_failure = RET_FAILURE;
            }
        }

        if (parse_it_failure == RET_SUCCESS)
        {
            test_it_cfg.actual_offset = this->m_device->SetPeekPokeAddr(test_it_cfg.offset, test_it_cfg.bar);
        }

        // Compute, check number of buffers
        double buff_cnt_d;

        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.buffer_size % sizeof(uint32_t) != 0)
            {
                LogMessage(MSG_MIO_004, {BUFFER_SIZE, std::to_string(test_it_cfg.buffer_size), "data word size", std::to_string(sizeof(uint32_t))});
                parse_it_failure = RET_FAILURE;
            }
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.total_size % test_it_cfg.buffer_size != 0)
            {
                LogMessage(MSG_MIO_004, {TOT_SIZE, std::to_string(test_it_cfg.total_size), BUFFER_SIZE, std::to_string(test_it_cfg.buffer_size)});
                parse_it_failure = RET_FAILURE;
            }
        }
        if (parse_it_failure == RET_SUCCESS)
        {

            test_it_cfg.buff_size_int   = test_it_cfg.buffer_size / (uint64_t)(sizeof(uint32_t));  // Bytes
            buff_cnt_d                  = (double)(test_it_cfg.total_size) / (double)(test_it_cfg.buffer_size);
            test_it_cfg.buffer_count    = (uint64_t)buff_cnt_d;
            test_it_cfg.total_size = test_it_cfg.buffer_count * test_it_cfg.buffer_size;
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            if (test_it_cfg.total_size % test_it_cfg.buffer_size != 0) // This ensure total_size is a multiple of buffer_size and total_size > buffer_size
            {
                LogMessage(MSG_MIO_002, {TOTAL_SIZE_BYTES_MEMBER.name, std::to_string(test_it_cfg.total_size), BUFFER_SIZE, std::to_string(test_it_cfg.buffer_size), std::to_string(test_list.size()+1)});
                parse_it_failure = RET_FAILURE;
            }
        }

        SetTestSequenceStr(test_seq_param, test_it_cfg);
        SetTestSequenceStrDbg(test_it_cfg);
        test_seq_strvect.emplace_back(StrVectToTest(test_it_cfg.test_strvect));

        if (parse_it_failure == RET_SUCCESS)
        {
            test_seq_strvect_dbg.emplace_back("\t- Test " + std::to_string(test_list.size()+1) + ": " + "" + StrVectToTest(test_it_cfg.test_strvect_dbg) + ". " + std::to_string(test_seq_param.num_param) + " parameters provided");
        }
        else
        {
            LogMessage(MSG_CMN_025,{std::to_string(test_list.size()+1)}); // Invalid settings provided
            test_seq_strvect_dbg.emplace_back("\t- Test " + std::to_string(test_list.size()+1) + ": " + StrVectToTest(test_it_cfg.test_strvect) + " contains erroneous parameters");
        }
        test_list.emplace_back(test_it_cfg);

        if (parse_it_failure == RET_FAILURE)
        {
            parse_error_cnt++;
            if (parse_error_cnt > MAX_NUM_PARSER_ERROR)
            {
                break;
            }
        }
        parse_failure |= parse_it_failure;
    }

    // display the entire test sequence
    LogMessage(MSG_DEBUG_TESTCASE, {"Extrapolated test sequence:"});
    for (const auto & test : test_seq_strvect_dbg)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {test});
    }

    if (parse_failure == RET_SUCCESS)
    {
        LogMessage(MSG_CMN_047, {std::to_string(test_seq_strvect.size()), "[ " + StrVectToStr(test_seq_strvect, ", ") + " ]"});
        LogMessage(MSG_CMN_023);
    }
    else
    {
        LogMessage(MSG_CMN_024);
    }

    return parse_failure;
}

void MmioTest::StopOnError( const uint & test_it_cnt, const bool & test_it_failure, bool & stop_test_it, const std::string & msg )
{
    if (!stop_test_it && (test_it_failure == RET_FAILURE) && (this->m_TC_Cfg.stop_on_error))
    {
        LogMessage(MSG_MIO_028, {std::to_string(test_it_cnt), msg});
        stop_test_it = true;
    }
}

int MmioTest::RunTest()
{
    uint                    test_it_cnt     = 1;
    auto                    test_failure    = RET_SUCCESS;
    auto                    test_it_failure = RET_SUCCESS;
    auto                    mem_ret         = RET_SUCCESS;

    if (!(this->m_abort))
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Number of test iterations: " + std::to_string(this->m_test_it_list.size())});
        this->m_testcase_queue_value.pending = this->m_test_it_list.size();
        PushTestcaseQueue();
    }

    for (auto & test_it : this->m_test_it_list)
    {
        if (this->m_abort)
        {
            break;
        }

        this->m_testcase_queue_value.remaining_time = std::to_string(test_it.duration);
        this->m_testcase_queue_value.parameters = StrVectToTest(test_it.test_strvect);
        PushTestcaseQueue();

        test_it_failure = RET_SUCCESS;
        LogMessage(MSG_CMN_032, {std::to_string(test_it_cnt), StrVectToTest(test_it.test_strvect)}); // start test
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Extrapolated test sequence: " + StrVectToTest(test_it.test_strvect_dbg)});
        LogMessage(MSG_CMN_042, {"\t- Extra test parameters: " + BAR + ": " + std::to_string(test_it.bar) + ", " + OFFSET + ": 0x" + NumToStrHex(test_it.offset, 16) + ", " + BUFFER_SIZE + ": " + std::to_string(test_it.buffer_size) + " Bytes, " + TOTAL_SIZE_BYTES_MEMBER.name + ": " + std::to_string(test_it.total_size) + " Bytes"});

        uint64_t t_0 = 0;
        uint64_t t_1 = 0;
        Rate_t rate_wr = RST_RATE;
        Rate_t rate_rd = RST_RATE;
        auto stop_test_it           = false;
        auto test_it_data_integ_str = DATA_INTEG_NA;    // default
        auto test_it_bw_result_str  = BW_RESULT_NA;     // default

        uint32_t *                   ref_data_buf = nullptr;
        std::vector<uint32_t *>      host_buf(test_it.buffer_count, nullptr);

        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {BAR                     + ": "            + std::to_string(test_it.bar)});
            LogMessage(MSG_DEBUG_TESTCASE, {OFFSET                  + ": 0x"          +    NumToStrHex(test_it.offset, 16) + " = " + std::to_string(test_it.offset)});
            LogMessage(MSG_DEBUG_TESTCASE, {"Actual " + OFFSET      + ": 0x"          +    NumToStrHex(test_it.actual_offset, 16) + " = " + std::to_string(test_it.actual_offset)});
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_CNT              + ": "            + std::to_string(test_it.buffer_count)});
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_SIZE             + " (Bytes): "    + std::to_string(test_it.buffer_size)});
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_SIZE             + " (int): "      + std::to_string(test_it.buff_size_int)});
            LogMessage(MSG_DEBUG_TESTCASE, {TOT_SIZE                + " (Bytes): "    + std::to_string(test_it.total_size)});
        }

        auto ref_t   = GetTimestamp();
        auto ref_cnt = (uint)(ref_t);

        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Allocate reference data buffer"});
            test_it_failure |= AllocateHostBuffer<uint32_t>(this->m_page_size, test_it.buff_size_int, &ref_data_buf);
            if (!(this->m_abort) && (test_it_failure == RET_FAILURE))
            {
                LogMessage(MSG_MIO_006);
            }
        }
        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Initialize reference data buffers with counter, start value: " + std::to_string(ref_cnt)});
        }
        for (uint64_t ii = 0; (ii < test_it.buff_size_int) && !(this->m_abort) && (test_it_failure == RET_SUCCESS); ii++)
        {
            ref_data_buf[ii] = ref_cnt++; // 32-bits counter
        }
        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Allocate host buffers"});
        }
        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && (test_it_failure == RET_SUCCESS); buff_idx++)
        {
            test_it_failure |= AllocateHostBuffer<uint32_t>(this->m_page_size, test_it.buff_size_int, &(host_buf[buff_idx]));
            if (!(this->m_abort) && (test_it_failure == RET_FAILURE))
            {
                LogMessage(MSG_MIO_007, {std::to_string(buff_idx)});
            }
        }

        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Initialize host buffers with reference data"});
        }
        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && (test_it_failure == RET_SUCCESS); buff_idx++)
        {
            memmove(host_buf[buff_idx], ref_data_buf, test_it.buffer_size);
        }

        // Used to display elapsed duration
        auto duration_divider = (uint64_t)(test_it.duration) / (uint64_t)(10);
        if (duration_divider == 0)
        {
            duration_divider = 1;
        }

        auto        duration_us = (uint64_t)(test_it.duration) * (uint64_t)(1000000);
        uint64_t    elapsed_d   = 0; // us

        test_it.t_start         = GetTimestamp();
        test_it.wr_rd_cycle_cnt = 0;
        test_it.elapsed         = 0;
        test_it.test_it_stat    = RST_TEST_IT_STAT;

        if (test_it_failure == RET_SUCCESS)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Initialization done, starting MMIO transfers"});

            while ((test_it.elapsed < duration_us) && !(this->m_abort) && !stop_test_it)
            {
                auto data_integ_str = DATA_INTEG_NA; // default

                ////
                // Write
                ////
                // MMIO write
                if (!(this->m_abort) && !stop_test_it)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"MMIO write transfer (Host->Card)"});

                    t_0     = GetTimestamp();
                    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                    {
                        mem_ret = this->m_device->MemWrite("MMIO", test_it.bar, test_it.actual_offset + buff_idx*test_it.buffer_size, test_it.buffer_size / (uint64_t)(sizeof(uint32_t)), host_buf[buff_idx]);
                    }
                    t_1     = GetTimestamp();

                    if (mem_ret == RET_FAILURE)
                    {
                        LogMessage(MSG_MIO_003, {"write", "to"});
                        test_it_failure = RET_FAILURE;
                    }

                    StopOnError(test_it_cnt, test_it_failure, stop_test_it, "MMIO write transfer (Host->Card)");
                }
                if (!(this->m_abort) && !stop_test_it)
                {
                    if (ComputeRate(test_it, t_1, t_0, rate_wr) == RET_FAILURE)
                    {
                        LogMessage(MSG_MIO_008, {"write"});
                        test_it_failure = RET_FAILURE;
                        StopOnError(test_it_cnt, test_it_failure, stop_test_it, "MMIO write bandwidth");
                    }
                }


                ////
                // Read
                ////
                // Reset data in host buffers
                if (!(this->m_abort) && !stop_test_it)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"Reset data in host buffers"});
                    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                    {
                        memset(host_buf[buff_idx], 0, test_it.buffer_size);
                    }
                }
                // MMIO read
                if (!(this->m_abort) && !stop_test_it)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"MMIO read transfer (Host<-Card)"});

                    // CL_MIGRATE_MEM_OBJECT_HOST indicates that the specified set of memory objects are to be migrated to the host,
                    // regardless of the target command-queue.
                    t_0     = GetTimestamp();
                    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                    {
                        mem_ret = this->m_device->MemRead("MMIO", test_it.bar, test_it.actual_offset + buff_idx*test_it.buffer_size, test_it.buffer_size / (uint64_t)(sizeof(uint32_t)), host_buf[buff_idx]);
                    }
                    t_1     = GetTimestamp();

                    if (mem_ret == RET_FAILURE)
                    {
                        LogMessage(MSG_MIO_003, {"read", "from"});
                        test_it_failure = RET_FAILURE;
                    }

                    StopOnError(test_it_cnt, test_it_failure, stop_test_it, "MMIO read transfer (Host<-Card)");
                }

                if (!(this->m_abort) && !stop_test_it)
                {
                    if (ComputeRate(test_it, t_1, t_0, rate_rd) == RET_FAILURE)
                    {
                        LogMessage(MSG_MIO_008, {"read"});
                        test_it_failure = RET_FAILURE;
                        StopOnError(test_it_cnt, test_it_failure, stop_test_it, "MMIO read bandwidth");
                    }
                }

                // Check data integrity in host buffer
                if (!(this->m_abort) && !stop_test_it)
                {
                    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                    {
                        if (!this->m_TC_Cfg.check_data_integrity)
                        {
                            data_integ_str  = DATA_INTEG_NC;
                        }
                        else if (CheckDataIntegrity(host_buf[buff_idx], ref_data_buf, test_it.buffer_size, test_it.buff_size_int) == RET_FAILURE)
                        {
                            LogMessage(MSG_MIO_009, {std::to_string(buff_idx)});
                            data_integ_str  = DATA_INTEG_KO;
                            test_it_failure = RET_FAILURE;
                            StopOnError(test_it_cnt, test_it_failure, stop_test_it, "data integrity");

                            if (!(this->m_TC_Cfg.stop_on_error))
                            {
                                // Reload host buffer with reference data when data integrity failed
                                LogMessage(MSG_MIO_027, {std::to_string(buff_idx)});
                                memmove(host_buf[buff_idx], ref_data_buf, test_it.buffer_size);
                            }
                        }
                        else if (data_integ_str == DATA_INTEG_NA)
                        {
                            data_integ_str = DATA_INTEG_OK;
                        }
                    }
                }

                // Global test data integrity result
                if (data_integ_str == DATA_INTEG_NC)
                {
                    test_it_data_integ_str  = DATA_INTEG_NC; // not checked
                }
                else if (data_integ_str == DATA_INTEG_NA)
                {
                    test_it_data_integ_str  = DATA_INTEG_NA; // stop on error
                }
                else if (data_integ_str == DATA_INTEG_OK)
                {
                    if (test_it_data_integ_str == DATA_INTEG_NA) // n/a at start-up, move to OK after first check OK
                    {
                        test_it_data_integ_str  = DATA_INTEG_OK;
                    }
                }
                else
                {
                    test_it_data_integ_str  = DATA_INTEG_KO;
                }
                // Check current rate as DEBUG
                if (!(this->m_abort))
                {
                    CheckRate(test_it, rate_wr, rate_rd, false);
                }

                WriteToMeasurementFileDetail(test_it_cnt, test_it, data_integ_str, rate_wr, rate_rd);
                test_it.wr_rd_cycle_cnt++;

                if ((test_it.elapsed - elapsed_d) >= duration_divider * (uint64_t)(1000000))
                {
                    LogMessage(MSG_CMN_048, {std::to_string((uint64_t)(test_it.duration) - (test_it.elapsed / (uint64_t)(1000000))), ""});
                    if (test_it.wr_rd_cycle_cnt != 0)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {std::to_string(test_it.wr_rd_cycle_cnt) + " write-read-check cycles performed"});
                        PrintResults(false, test_it, rate_wr, rate_rd); // Print current result as debug
                    }
                    elapsed_d = test_it.elapsed;
                }

                test_it.elapsed = (GetTimestamp() - test_it.t_start);
                this->m_testcase_queue_value.remaining_time = std::to_string((uint64_t)(test_it.duration) - (test_it.elapsed / (uint64_t)(1000000)));
                PushTestcaseQueue();
            }
            LogMessage(MSG_CMN_049); // test duration reached
            LogMessage(MSG_MIO_026, {std::to_string(test_it.wr_rd_cycle_cnt)});

            if (!(this->m_abort))
            {
                PrintResults(true, test_it, rate_wr, rate_rd);
                test_it_bw_result_str = BW_RESULT_PASS;
                if (CheckRate(test_it, rate_wr, rate_rd, true) == RET_FAILURE)
                {
                    test_it_bw_result_str   = BW_RESULT_FAILURE;
                    test_it_failure         = RET_FAILURE;
                }
            }
        }
        WriteToMeasurementFileResult(test_it_cnt, test_it, test_it_data_integ_str, rate_wr, rate_rd);

        // Release
        LogMessage(MSG_DEBUG_TESTCASE, {"Release host buffers"});
        DeallocateHostBuffer<uint32_t>(ref_data_buf);
        for (auto & buf : host_buf)
        {
            DeallocateHostBuffer<uint32_t>(buf);
        }
        host_buf.clear();

        // Results
        if ((test_it_failure == RET_SUCCESS) && !(this->m_abort))
        {
            if (this->m_TC_Cfg.check_bw & this->m_TC_Cfg.check_data_integrity)
            {
                LogMessage(MSG_MIO_011); // BW and integ pass
            }
            else if (!this->m_TC_Cfg.check_bw & this->m_TC_Cfg.check_data_integrity)
            {
                LogMessage(MSG_MIO_013); // integ pass
            }
            else if (this->m_TC_Cfg.check_bw & !this->m_TC_Cfg.check_data_integrity)
            {
                LogMessage(MSG_MIO_029); // BW pass
            }
            else
            {
                LogMessage(MSG_MIO_031); // no BW/data integ checked
            }
        }
        else
        {
            if (this->m_TC_Cfg.check_bw & this->m_TC_Cfg.check_data_integrity)
            {
                LogMessage(MSG_MIO_012); // BW and or integ fail
            }
            else if (!this->m_TC_Cfg.check_bw & this->m_TC_Cfg.check_data_integrity)
            {
                LogMessage(MSG_MIO_014); // integ fail
            }
            else if (this->m_TC_Cfg.check_bw & !this->m_TC_Cfg.check_data_integrity)
            {
                LogMessage(MSG_MIO_030); // BW fail
            }
            else
            {
                LogMessage(MSG_MIO_032); // no BW/data integ checked
            }
        }

        LogMessage(MSG_DEBUG_TESTCASE, {"Total test duration: " + std::to_string((double)(test_it.elapsed) / (double)(1000) / (double)(1000)) + " seconds. Expected duration: " + std::to_string(test_it.duration) + " seconds"});
        LogMessage(MSG_DEBUG_TESTCASE, {"Total quantity of write-read-check cycles performed: " + std::to_string(test_it.wr_rd_cycle_cnt)});

        if (this->m_abort)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            // Set all remaining test results to aborted
            for (uint i = test_it.parent_test_idx - 1; i < this->m_test_sequence.size(); i++)
            {
                this->m_test_it_results[i] = TITR_ABORTED;
                this->m_testcase_queue_value.completed++;
                this->m_testcase_queue_value.pending--;
                this->m_testcase_queue_value.failed++;
            }
        }
        else if (test_it_failure == RET_FAILURE)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            this->m_test_it_results[test_it.parent_test_idx-1] = TITR_FAILED;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.failed++;
        }
        else
        {
            LogMessage(MSG_CMN_033_PASS, {std::to_string(test_it_cnt)});
            // Update only when parent test has not already failed.
            // This could be the case when memory name is provided (not tag)
            if (this->m_test_it_results[test_it.parent_test_idx-1] != TITR_FAILED)
            {
                this->m_test_it_results[test_it.parent_test_idx-1] = TITR_PASSED;
            }
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.passed++;
        }
        if (this->m_testcase_queue_value.pending == 0)
        {
            this->m_testcase_queue_value.remaining_time    = NOT_APPLICABLE;
            this->m_testcase_queue_value.parameters        = NOT_APPLICABLE;
        }
        PushTestcaseQueue();
        test_it_cnt++;
    }

    // check for overall test failure
    if (this->m_abort)
    {
        return TEST_STATE_ABORT;
    }
    if (test_failure == RET_FAILURE)
    {
        return TEST_STATE_FAILURE;
    }
    return TEST_STATE_PASS;
}

bool MmioTest::CheckDataIntegrity( uint32_t * host_buf, uint32_t * ref_data_buf, const uint64_t & buff_size_bytes, const uint64_t & buff_size_int )
{
    if (memcmp(host_buf, ref_data_buf, buff_size_bytes) != 0)
    {
        LogMessage(MSG_MIO_015);
        // Report where the comparison first failed
        uint64_t data_err_cnt = 0;
        for (uint64_t i = 0; i < buff_size_int; i++)
        {
            if (host_buf[i] != ref_data_buf[i])
            {
                if (data_err_cnt == 0)
                {
                    LogMessage(MSG_MIO_021, {"First error at index : " + std::to_string(i)});
                    LogMessage(MSG_MIO_021, {"\t - Write data: " + std::to_string(ref_data_buf[i])});
                    LogMessage(MSG_MIO_021, {"\t - Read data:  " + std::to_string(host_buf[i])});
                }
                data_err_cnt++;
            }
        }
        LogMessage(MSG_DEBUG_TESTCASE, {"Quantity of incorrect values: " + std::to_string(data_err_cnt) + "/" + std::to_string(buff_size_int)});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool MmioTest::ComputeRate( const TestItConfig_t & test_it_cfg, const uint64_t & t_1, const uint64_t & t_0, Rate_t & rate )
{
    auto duration = t_1 - t_0; // us

    if (duration == 0)
    {
        return RET_FAILURE;
    }

    rate.inst = (double)(test_it_cfg.total_size) / (double)(1024); // kBytes
    rate.inst *= (double)(1000000);
    rate.inst /= (double)(duration); // kBytes/s

    if (test_it_cfg.wr_rd_cycle_cnt == 0)
    {
        rate.min = rate.inst;
        rate.max = rate.inst;
        rate.acc = rate.inst;
        rate.avg = rate.inst;
    }
    else
    {
        if (rate.inst < rate.min)
        {
            rate.min = rate.inst;
        }
        if (rate.max  < rate.inst)
        {
            rate.max = rate.inst;
        }
        rate.acc += rate.inst;
    }
    rate.avg = rate.acc / (test_it_cfg.wr_rd_cycle_cnt + 1);
    rate.valid = true;

    return RET_SUCCESS;
}

bool MmioTest::CheckRate( TestItConfig_t & test_it_cfg, const Rate_t & rate_wr, const Rate_t & rate_rd, const bool & report_pass_fail )
{
    auto ret = RET_SUCCESS;
    std::string result_str;
    // Select BW range depending on memory type
    auto lo_thresh_wr = this->m_TC_Cfg.lo_thresh_wr;
    auto hi_thresh_wr = this->m_TC_Cfg.hi_thresh_wr;
    auto lo_thresh_rd = this->m_TC_Cfg.lo_thresh_rd;
    auto hi_thresh_rd = this->m_TC_Cfg.hi_thresh_rd;

    auto check_bw_disabled  = false;
    auto check_bw_exists    = this->m_TC_Cfg.check_bw_exists;
    auto default_check_bw   = this->m_TC_Cfg.check_bw;
    auto check_bw           = default_check_bw;

    if (report_pass_fail)
    {
        if (test_it_cfg.duration < MIN_RESULTS_CHECK_DURATION)
        {
            LogMessage(MSG_CMN_051, {DURATION_TEST_SEQ_MEMBER.name + " (" + std::to_string(test_it_cfg.duration) + ") of test is lower than " + std::to_string(MIN_RESULTS_CHECK_DURATION), "bandwidth"});
            IsCheckDisabled(check_bw_exists, default_check_bw, check_bw, check_bw_disabled);
        }
        if (test_it_cfg.buffer_size != this->m_TC_Cfg.nom_buffer_size)
        {
            LogMessage(MSG_CMN_051, {BUFFER_SIZE + " (" + std::to_string(test_it_cfg.buffer_size) + ") of test does not equal " + std::to_string(this->m_TC_Cfg.nom_buffer_size), "bandwidth"});
            IsCheckDisabled(check_bw_exists, default_check_bw, check_bw, check_bw_disabled);
        }
        if (test_it_cfg.total_size != this->m_TC_Cfg.nom_total_size)
        {
            LogMessage(MSG_CMN_051, {TOTAL_SIZE_BYTES_MEMBER.name + " (" + std::to_string(test_it_cfg.total_size) + ") of test does not equal " + std::to_string(this->m_TC_Cfg.nom_total_size), "bandwidth"});
            IsCheckDisabled(check_bw_exists, default_check_bw, check_bw, check_bw_disabled);
        }

        if (check_bw_disabled)
        {
            LogMessage(MSG_CMN_052, {"bandwidth"});
        }

        if (!check_bw)
        {
            return RET_SUCCESS;
        }
    }

    auto wr_bw_range = "[" + std::to_string(lo_thresh_wr) + ", " + std::to_string(hi_thresh_wr) + "]";
    auto rd_bw_range = "[" + std::to_string(lo_thresh_rd) + ", " + std::to_string(hi_thresh_rd) + "]";

    // If BW check is not enabled in test JSON, we check the instantaneous BW without reporting BW errors
    uint rate_wr_chk = rate_wr.inst;
    uint rate_rd_chk = rate_rd.inst;
    if (report_pass_fail)
    {
        rate_wr_chk = rate_wr.avg;
        rate_rd_chk = rate_rd.avg;
    }

    result_str = "\tWRITE {Host->PCIe->FPGA} [kB/s]:";
    if (report_pass_fail)
    {
        result_str += " average ";
    }
    else
    {
        result_str += " instantaneous ";
    }
    if (rate_wr.valid)
    {
        result_str += std::to_string(rate_wr_chk) + " ";
    }
    else
    {
        result_str += NOT_APPLICABLE + " ";
    }

    if ( (rate_wr_chk < lo_thresh_wr) || (rate_wr_chk > hi_thresh_wr) )
    {
        result_str += " outside " + wr_bw_range;
        if (report_pass_fail)
        {
            LogMessage(MSG_MIO_016, {result_str});
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {result_str});
            test_it_cfg.test_it_stat.wr_bw_fail_cnt++;
        }
        ret = RET_FAILURE;
    }
    else
    {
        result_str += " inside " + wr_bw_range;
        if (report_pass_fail)
        {
            LogMessage(MSG_MIO_017, {result_str});
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {result_str});
            test_it_cfg.test_it_stat.wr_bw_pass_cnt++;
        }
    }

    result_str = "\tREAD  {Host<-PCIe<-FPGA} [kB/s]:";
    if (report_pass_fail)
    {
        result_str += " average ";
    }
    else
    {
        result_str += " instantaneous ";
    }
    if (rate_rd.valid)
    {
        result_str += std::to_string(rate_rd_chk) + " ";
    }
    else
    {
        result_str += NOT_APPLICABLE + " ";
    }

    if ((rate_rd_chk < lo_thresh_rd) || (rate_rd_chk > hi_thresh_rd) )
    {
        result_str += " outside " + rd_bw_range;
        if (report_pass_fail)
        {
            LogMessage(MSG_MIO_016, {result_str});
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {result_str});
            test_it_cfg.test_it_stat.rd_bw_fail_cnt++;
        }
        ret = RET_FAILURE;
    }
    else
    {
        result_str += " inside " + rd_bw_range;
        if (report_pass_fail)
        {
            LogMessage(MSG_MIO_017, {result_str});
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {result_str});
            test_it_cfg.test_it_stat.rd_bw_pass_cnt++;
        }
    }

    if (!report_pass_fail)
    {
        return RET_SUCCESS;
    }
    return ret;
}

void MmioTest::PrintResults( const bool & info_n_debug, const TestItConfig_t & test_it_cfg, const Rate_t & rate_wr, const Rate_t & rate_rd )
{
    auto lo_thresh_wr = this->m_TC_Cfg.lo_thresh_wr;
    auto hi_thresh_wr = this->m_TC_Cfg.hi_thresh_wr;
    auto lo_thresh_rd = this->m_TC_Cfg.lo_thresh_rd;
    auto hi_thresh_rd = this->m_TC_Cfg.hi_thresh_rd;
    auto wr_bw_range  = "[" + std::to_string(lo_thresh_wr) + ", " + std::to_string(hi_thresh_wr) + "]";
    auto rd_bw_range  = "[" + std::to_string(lo_thresh_rd) + ", " + std::to_string(hi_thresh_rd) + "]";

    std::string result = "Bandwidth results (Min, Avg, Max) [kB/s]; WRITE {Host->PCIe->FPGA} (";
    if (rate_wr.valid)
    {
        result += Float_to_String_Ceil<double>(rate_wr.min, 2) + ", ";
        result += Float_to_String_Ceil<double>(rate_wr.avg, 2) + ", ";
        result += Float_to_String_Ceil<double>(rate_wr.max, 2) + "); ";
    }
    else
    {
        result += NOT_APPLICABLE + ", ";
        result += NOT_APPLICABLE + ", ";
        result += NOT_APPLICABLE + "); ";
    }
    result += "\tREAD {Host<-PCIe<-FPGA} (";
    if (rate_rd.valid)
    {
        result += Float_to_String_Ceil<double>(rate_rd.min, 2) + ", ";
        result += Float_to_String_Ceil<double>(rate_rd.avg, 2) + ", ";
        result += Float_to_String_Ceil<double>(rate_rd.max, 2) + ")";
    }
    else
    {
        result += NOT_APPLICABLE + ", ";
        result += NOT_APPLICABLE + ", ";
        result += NOT_APPLICABLE + ")";
    }

    if (info_n_debug)
    {
        LogMessage(MSG_MIO_018, {result});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {result});
    }

    if (this->m_TC_Cfg.check_bw)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.wr_bw_pass_cnt) + " WRITE  inside " + wr_bw_range});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.rd_bw_pass_cnt) + " READ   inside " + rd_bw_range});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.wr_bw_fail_cnt) + " WRITE outside " + wr_bw_range});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.rd_bw_fail_cnt) + " READ  outside " + rd_bw_range});
    }
}

void MmioTest::WriteToMeasurementFileResult ( const uint & test_it_cnt, const TestItConfig_t & test_it_cfg, const std::string & data_integ_str, const Rate_t & rate_wr, const Rate_t & rate_rd )
{

    if (this->m_use_outputfile)
    {
        std::vector<std::string> str_vect;
        str_vect.emplace_back(  std::to_string(  test_it_cnt                                        ));
        str_vect.emplace_back(  std::to_string(  test_it_cfg.duration                               ));
        str_vect.emplace_back(  std::to_string(  test_it_cfg.bar                                    ));
        str_vect.emplace_back("0x"+NumToStrHex(  test_it_cfg.offset, 16                             ));
        str_vect.emplace_back(  std::to_string(  test_it_cfg.buffer_size                            ));
        str_vect.emplace_back(  std::to_string(  test_it_cfg.buffer_count                           ));
        str_vect.emplace_back(  std::to_string(  test_it_cfg.buffer_count * test_it_cfg.buffer_size ));
        str_vect.emplace_back(  std::to_string(  test_it_cfg.wr_rd_cycle_cnt                        ));
        str_vect.emplace_back(                   data_integ_str                                      );
        str_vect.emplace_back(  std::to_string(  rate_wr.min                                        ));
        str_vect.emplace_back(  std::to_string(  rate_wr.avg                                        ));
        str_vect.emplace_back(  std::to_string(  rate_wr.max                                        ));
        str_vect.emplace_back(  std::to_string(  rate_rd.min                                        ));
        str_vect.emplace_back(  std::to_string(  rate_rd.avg                                        ));
        str_vect.emplace_back(  std::to_string(  rate_rd.max                                        ));

        this->m_outputfile_result << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile_result.flush();
    }
}

void MmioTest::WriteToMeasurementFileDetail ( const uint & test_it_cnt, const TestItConfig_t & test_it_cfg, const std::string & data_integ_str, const Rate_t & rate_wr, const Rate_t & rate_rd)
{
    std::vector<std::string> str_vect;
    str_vect.emplace_back(  std::to_string(  test_it_cnt                 ));
    str_vect.emplace_back(  std::to_string(  test_it_cfg.bar             ));
    str_vect.emplace_back("0x"+NumToStrHex(  test_it_cfg.offset, 16      ));
    str_vect.emplace_back(  std::to_string(  test_it_cfg.buffer_size     ));
    str_vect.emplace_back(  std::to_string(  test_it_cfg.wr_rd_cycle_cnt ));
    str_vect.emplace_back(                   data_integ_str               );
    str_vect.emplace_back(  std::to_string(  rate_wr.inst                ));
    str_vect.emplace_back(  std::to_string(  rate_wr.min                 ));
    str_vect.emplace_back(  std::to_string(  rate_wr.avg                 ));
    str_vect.emplace_back(  std::to_string(  rate_wr.max                 ));
    str_vect.emplace_back(  std::to_string(  rate_rd.inst                ));
    str_vect.emplace_back(  std::to_string(  rate_rd.min                 ));
    str_vect.emplace_back(  std::to_string(  rate_rd.avg                 ));
    str_vect.emplace_back(  std::to_string(  rate_rd.max                 ));

    if (this->m_use_outputfile)
    {
        this->m_outputfile_detail << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile_detail.flush();

        (void)OpenRTOutputFile(this->m_RT_outputfile_detail_name, this->m_RT_outputfile_detail);  // Don't check if open worked..
        this->m_RT_outputfile_detail << this->m_RT_outputfile_detail_head + "\n" + StrVectToStr(str_vect, ",");
        this->m_RT_outputfile_detail.flush();
        this->m_RT_outputfile_detail.close();
    }
}

bool MmioTest::StartCU()
{
    return RET_FAILURE;
}
bool MmioTest::EnableWatchdogClkThrotDetection()
{
    return RET_FAILURE;
}
bool MmioTest::StopCU()
{
    return RET_FAILURE;
}
bool MmioTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_FAILURE; // not a real xbtest HW IP, no watchdog
}

} // namespace

#endif