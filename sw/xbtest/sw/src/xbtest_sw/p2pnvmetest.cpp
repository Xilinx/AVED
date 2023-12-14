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

#include "p2pnvmetest.h"

#ifdef USE_XRT

#include <fcntl.h>

#include "clpackage.h"

namespace xbtest
{

P2PNvmeTest::P2PNvmeTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, const P2P_Nvme_Parameters_t & test_parameters
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_log_header              = LOG_HEADER_P2P_NVME;
    this->m_test_parameters         = test_parameters;

    this->m_filename                = this->m_xbtest_sw_config->GetCommandLineP2pNvmePath().value;

    this->m_queue_testcase          = P2P_NVME_MEMBER.name;
    this->m_queue_thread            = P2P_NVME_MEMBER.name;
}

P2PNvmeTest::~P2PNvmeTest () = default;

bool P2PNvmeTest::SetupMemoryConfig( bool check_bw_global, bool check_data_integrity_global, bool stop_on_error_global, bool check_bw_exists_global, bool check_data_integrity_exists_global )
{
    auto global_settings_failure = RET_SUCCESS;

    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        if (memory.target == BOARD)
        {
            auto name_cmplt = " for memory type " + memory.name;

            this->m_TC_Cfg.memory[memory.idx] = RST_P2PMEMTYPECFG;

            // Define test configurations, default xbtest_pfm_def.json, overwrite with test JSON
            this->m_TC_Cfg.memory[memory.idx].name = memory.name;
            this->m_TC_Cfg.memory[memory.idx].tag  = memory.available_tag;

            auto max_total_size = memory.size_mb;
            if (memory.type == MULTI_CHANNEL)
            {
                max_total_size /= memory.available_tag.size();
            }
            this->m_TC_Cfg.memory[memory.idx].nom_total_size = max_total_size;
            if (memory.p2p_nvme_config.total_size_exists)
            {
                this->m_TC_Cfg.memory[memory.idx].nom_total_size = memory.p2p_nvme_config.total_size;
                LogMessage(MSG_DEBUG_TESTCASE, {"Using nominal total size defined in card definition (" + std::to_string(this->m_TC_Cfg.memory[memory.idx].nom_total_size) + ") for memory " + memory.name});
            }
            else
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Using default nominal total size (" + std::to_string(this->m_TC_Cfg.memory[memory.idx].nom_total_size) + ") for memory " + memory.name});
            }

            global_settings_failure |= GetJsonParamNum<uint>(TOTAL_SIZE_MEMBER,   name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, MIN_TOTAL_SIZE,   this->m_TC_Cfg.memory[memory.idx].nom_total_size,    max_total_size,     this->m_TC_Cfg.memory[memory.idx].total_size);

            this->m_TC_Cfg.memory[memory.idx].check_bw_exists = check_bw_exists_global;
            auto it = FindJsonParam(this->m_test_parameters.memory_config[memory.idx].param, CHECK_BW_MEMBER);
            if (it != this->m_test_parameters.memory_config[memory.idx].param.end())
            {
                this->m_TC_Cfg.memory[memory.idx].check_bw_exists = true;
            }
            this->m_TC_Cfg.memory[memory.idx].check_data_integrity_exists = check_data_integrity_exists_global;
            it = FindJsonParam(this->m_test_parameters.memory_config[memory.idx].param, CHECK_DATA_INTEGRITY_MEMBER);
            if (it != this->m_test_parameters.memory_config[memory.idx].param.end())
            {
                this->m_TC_Cfg.memory[memory.idx].check_data_integrity_exists = true;
            }
            global_settings_failure |= GetJsonParamBool(CHECK_BW_MEMBER,                name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, this->m_TC_Cfg.memory[memory.idx].check_bw,                check_bw_global);
            global_settings_failure |= GetJsonParamBool(CHECK_DATA_INTEGRITY_MEMBER,    name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, this->m_TC_Cfg.memory[memory.idx].check_data_integrity,    check_data_integrity_global);
            global_settings_failure |= GetJsonParamBool(STOP_ON_ERROR_MEMBER,           name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, this->m_TC_Cfg.memory[memory.idx].stop_on_error,           stop_on_error_global);

            global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_WR_MEMBER, name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, MIN_LO_THRESH_WR, memory.p2p_nvme_bw.write.low,  MAX_LO_THRESH_WR,   this->m_TC_Cfg.memory[memory.idx].lo_thresh_wr);
            global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_WR_MEMBER, name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, MIN_HI_THRESH_WR, memory.p2p_nvme_bw.write.high, MAX_HI_THRESH_WR,   this->m_TC_Cfg.memory[memory.idx].hi_thresh_wr);
            global_settings_failure |= GetJsonParamNum<uint>(LO_THRESH_RD_MEMBER, name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, MIN_LO_THRESH_RD, memory.p2p_nvme_bw.read.low,   MAX_LO_THRESH_RD,   this->m_TC_Cfg.memory[memory.idx].lo_thresh_rd);
            global_settings_failure |= GetJsonParamNum<uint>(HI_THRESH_RD_MEMBER, name_cmplt, this->m_test_parameters.memory_config[memory.idx].param, MIN_HI_THRESH_RD, memory.p2p_nvme_bw.read.high,  MAX_HI_THRESH_RD,   this->m_TC_Cfg.memory[memory.idx].hi_thresh_rd);

            Json_Val_Def_t json_val_def;

            json_val_def = LO_THRESH_WR_MEMBER; json_val_def.name += name_cmplt;
            global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def, this->m_TC_Cfg.memory[memory.idx].lo_thresh_wr, HI_THRESH_WR_MEMBER, this->m_TC_Cfg.memory[memory.idx].hi_thresh_wr);

            json_val_def = LO_THRESH_RD_MEMBER; json_val_def.name += name_cmplt;
            global_settings_failure |= CheckThresholdLoVsHi<uint>(json_val_def, this->m_TC_Cfg.memory[memory.idx].lo_thresh_rd, HI_THRESH_RD_MEMBER, this->m_TC_Cfg.memory[memory.idx].hi_thresh_rd);

            this->m_TC_Cfg.memory[memory.idx].nom_buffer_size = NOM_BUFFER_SIZE;
            if (memory.p2p_nvme_config.buffer_size_exists)
            {
                this->m_TC_Cfg.memory[memory.idx].nom_buffer_size = memory.p2p_nvme_config.buffer_size;
                LogMessage(MSG_DEBUG_TESTCASE, {"Using nominal buffer size defined in card definition (" + std::to_string(this->m_TC_Cfg.memory[memory.idx].nom_buffer_size) + ") for memory " + memory.name});
            }
            else
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Using default nominal buffer size (" + std::to_string(this->m_TC_Cfg.memory[memory.idx].nom_buffer_size) + ") for memory " + memory.name});
            }

            this->m_TC_Cfg.memory[memory.idx].max_buffer_size = this->m_TC_Cfg.memory[memory.idx].total_size;
            if (this->m_TC_Cfg.memory[memory.idx].max_buffer_size > MAX_BUFFER_SIZE)
            {
                this->m_TC_Cfg.memory[memory.idx].max_buffer_size = MAX_BUFFER_SIZE;
                LogMessage(MSG_DEBUG_TESTCASE, {"Maximum buffer size saturated (" + std::to_string(MAX_BUFFER_SIZE)+ ") for memory " + memory.name});
            }

            if (this->m_TC_Cfg.memory[memory.idx].nom_buffer_size > this->m_TC_Cfg.memory[memory.idx].max_buffer_size)
            {
                this->m_TC_Cfg.memory[memory.idx].nom_buffer_size = this->m_TC_Cfg.memory[memory.idx].max_buffer_size;
                LogMessage(MSG_DEBUG_TESTCASE, {"Nominal buffer size saturated (" + std::to_string(this->m_TC_Cfg.memory[memory.idx].nom_buffer_size) + ") for memory " + memory.name});
            }
        }
    }
    return global_settings_failure;
}

bool P2PNvmeTest::PreSetup()
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
    LogMessage(MSG_P2P_001, {std::to_string(this->m_page_size)});

    // Get Global parameter
    bool check_bw_global;
    bool check_data_integrity_global;
    bool stop_on_error_global;
    bool check_bw_exists_global = false;
    bool check_data_integrity_exists_global = false;

    auto it = FindJsonParam(this->m_test_parameters.global_config, CHECK_BW_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        check_bw_exists_global = true;
    }
    it = FindJsonParam(this->m_test_parameters.global_config, CHECK_DATA_INTEGRITY_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        check_data_integrity_exists_global = true;
    }
    global_settings_failure |= GetJsonParamBool(CHECK_BW_MEMBER,                this->m_test_parameters.global_config, check_bw_global,             false);
    global_settings_failure |= GetJsonParamBool(CHECK_DATA_INTEGRITY_MEMBER,    this->m_test_parameters.global_config, check_data_integrity_global, true);
    global_settings_failure |= GetJsonParamBool(STOP_ON_ERROR_MEMBER,           this->m_test_parameters.global_config, stop_on_error_global,        false);

    // Get parameters defined in xbtest_pfm_def.json and overwrite parameters if present in JSON file
    // Get parameters for memory types on card
    global_settings_failure |= SetupMemoryConfig(check_bw_global, check_data_integrity_global, stop_on_error_global, check_bw_exists_global, check_data_integrity_exists_global);

    // Output files
    std::vector<std::string> first_line_detail;
    first_line_detail.emplace_back("Test");
    first_line_detail.emplace_back("Card mode");
    first_line_detail.emplace_back(MEM_TAG);
    first_line_detail.emplace_back(BUFFER_SIZE + " (MB)");
    first_line_detail.emplace_back("Cycle ID");
    first_line_detail.emplace_back("Data Integrity");
    first_line_detail.emplace_back(WR_LIV_BW + " (MBps)");
    first_line_detail.emplace_back(WR_MIN_BW + " (MBps)");
    first_line_detail.emplace_back(WR_AVG_BW + " (MBps)");
    first_line_detail.emplace_back(WR_MAX_BW + " (MBps)");
    first_line_detail.emplace_back(RD_LIV_BW + " (MBps)");
    first_line_detail.emplace_back(RD_MIN_BW + " (MBps)");
    first_line_detail.emplace_back(RD_AVG_BW + " (MBps)");
    first_line_detail.emplace_back(RD_MAX_BW + " (MBps)");

    std::vector<std::string> first_line_result;
    first_line_result.emplace_back("Test");
    first_line_result.emplace_back("Card mode");
    first_line_result.emplace_back(MEM_TAG);
    first_line_result.emplace_back(DURATION    + " (s)");
    first_line_result.emplace_back(BUFFER_SIZE + " (MB)");
    first_line_result.emplace_back(BUFFER_CNT);
    first_line_result.emplace_back(TOT_SIZE + " (MB)");
    first_line_result.emplace_back("Number of cycles");
    first_line_result.emplace_back("Data Integrity");
    first_line_result.emplace_back(WR_MIN_BW + " (MBps)");
    first_line_result.emplace_back(WR_AVG_BW + " (MBps)");
    first_line_result.emplace_back(WR_MAX_BW + " (MBps)");
    first_line_result.emplace_back(RD_MIN_BW + " (MBps)");
    first_line_result.emplace_back(RD_AVG_BW + " (MBps)");
    first_line_result.emplace_back(RD_MAX_BW + " (MBps)");

    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {
        this->m_outputfile_name = "p2p_nvme";
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
    LogMessage(MSG_CMN_021, {"\t- " + CHECK_BW_MEMBER.name              + " : " + BoolToStr(check_bw_global)});
    LogMessage(MSG_CMN_021, {"\t- " + CHECK_DATA_INTEGRITY_MEMBER.name  + " : " + BoolToStr(check_data_integrity_global)});
    LogMessage(MSG_CMN_021, {"\t- " + STOP_ON_ERROR_MEMBER.name         + " : " + BoolToStr(stop_on_error_global)});

    for (const auto & cfg : this->m_TC_Cfg.memory)
    {
        LogMessage(MSG_CMN_021, {"Test configuration for memory type " + cfg.second.name + ":"});
        LogMessage(MSG_CMN_021, {"\t- Applicable tags: "                    +   StrVectToStr(cfg.second.tag, ", ")});
        LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_WR_MEMBER.name          + ": " + std::to_string(cfg.second.lo_thresh_wr)});
        LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_WR_MEMBER.name          + ": " + std::to_string(cfg.second.hi_thresh_wr)});
        LogMessage(MSG_CMN_021, {"\t- " + LO_THRESH_RD_MEMBER.name          + ": " + std::to_string(cfg.second.lo_thresh_rd)});
        LogMessage(MSG_CMN_021, {"\t- " + HI_THRESH_RD_MEMBER.name          + ": " + std::to_string(cfg.second.hi_thresh_rd)});
        LogMessage(MSG_CMN_021, {"\t- " + CHECK_BW_MEMBER.name              + ": " +      BoolToStr(cfg.second.check_bw)});
        LogMessage(MSG_CMN_021, {"\t- " + CHECK_DATA_INTEGRITY_MEMBER.name  + ": " +      BoolToStr(cfg.second.check_data_integrity)});
        LogMessage(MSG_CMN_021, {"\t- " + STOP_ON_ERROR_MEMBER.name         + ": " +      BoolToStr(cfg.second.stop_on_error)});
    }

    if (global_settings_failure == RET_SUCCESS)
    {
        global_settings_failure |= ParseTestSequenceSettings(this->m_test_it_list);
    }

    // Context, command queue
    this->m_cl_cmd_queue  = this->m_device->GetCmdQueueInstance();
    this->m_cl_context    = this->m_device->GetContextInstance();

    if (global_settings_failure == RET_FAILURE)
    {
        Abort();
    }
    return global_settings_failure;
}

void P2PNvmeTest::Run()
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

void P2PNvmeTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void P2PNvmeTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

bool P2PNvmeTest::GetTestSequence()
{
    this->m_test_it_results.clear();
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_test_sequence = TestcaseParamCast<std::vector<P2P_Nvme_Test_Sequence_Parameters_t>>(it->second);
        for (uint ii = 0; ii < this->m_test_sequence.size(); ii++)
        {
            this->m_test_it_results.emplace_back(TITR_NOT_TESTED);
        }
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool P2PNvmeTest::GetConfigIndexFromType ( const std::string & mem_type, uint & mem_cfg_idx )
{
    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        if (memory.target == BOARD)
        {
            if (StrMatchNoCase(mem_type, this->m_TC_Cfg.memory[memory.idx].name))
            {
                mem_cfg_idx = memory.idx;
                return RET_SUCCESS;
            }
        }
    }
    LogMessage(MSG_P2P_024, {"type", mem_type});
    return RET_FAILURE;
}

bool P2PNvmeTest::GetConfigIndexFromTag ( const std::string & mem_tag, uint & mem_cfg_idx )
{
    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        if (memory.target == BOARD)
        {
            if (FindStringInVect(mem_tag, this->m_TC_Cfg.memory[memory.idx].tag))
            {
                mem_cfg_idx = memory.idx;
                return RET_SUCCESS;
            }
        }
    }
    LogMessage(MSG_P2P_024, {"tag", mem_tag});
    return RET_FAILURE;
}

bool P2PNvmeTest::SetOtherTestSequenceItSettings( const P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param, const std::list<TestItConfig_t> & test_list, TestItConfig_t & test_it_cfg )
{
    // duration
    test_it_cfg.duration = test_seq_param.duration.value;
    if (CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_it_cfg.duration, MIN_DURATION, MAX_DURATION) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // buffer_size
    if (test_seq_param.buffer_size.exists)
    {
        test_it_cfg.buffer_size = test_seq_param.buffer_size.value;
    }
    else
    {
        test_it_cfg.buffer_size = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].nom_buffer_size;
    }
    if (CheckParam<uint64_t>(BUFFER_SIZE_TEST_SEQ_MEMBER.name, test_it_cfg.buffer_size, MIN_BUFFER_SIZE, this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].max_buffer_size) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (!IsPowerOf2<uint64_t>(test_it_cfg.buffer_size))
    {
        LogMessage(MSG_P2P_023, {BUFFER_SIZE, std::to_string(test_it_cfg.buffer_size), "not a power of 2", std::to_string(test_list.size()+1)});
        return RET_FAILURE;
    }
    test_it_cfg.buff_size_bytes = test_it_cfg.buffer_size * (uint64_t)(1024) * (uint64_t)(1024);  // Bytes
    test_it_cfg.buff_size_int   = test_it_cfg.buff_size_bytes / (uint64_t)(sizeof(int));  // Bytes
    auto buff_cnt_d             = (double)(this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].total_size) / (double)(test_it_cfg.buffer_size);
    test_it_cfg.buffer_count    = (uint64_t)buff_cnt_d;
    if ((test_it_cfg.buffer_count == 0) || ((double)(test_it_cfg.buffer_count) != buff_cnt_d))
    {
        LogMessage(MSG_P2P_004, {std::to_string(buff_cnt_d)});
        return RET_FAILURE;
    }
    test_it_cfg.total_size_bytes = test_it_cfg.buffer_count * test_it_cfg.buff_size_bytes;
    if (this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].total_size % test_it_cfg.buffer_size != 0) // This ensure total_size is a multiple of buffer_size and total_size > buffer_size
    {
        LogMessage(MSG_P2P_002, {TOTAL_SIZE_MEMBER.name, std::to_string(this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].total_size), BUFFER_SIZE, std::to_string(test_it_cfg.buffer_size), std::to_string(test_list.size() + 1)});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void P2PNvmeTest::SetTestSequenceStr( const P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param, TestItConfig_t & test_it_cfg )
{
    test_it_cfg.test_strvect.clear();
    test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name    + "\": "   + std::to_string(  test_seq_param.duration.value));
    if (test_seq_param.source.exists)
    {
        test_it_cfg.test_strvect.emplace_back("\"" + SOURCE_TEST_SEQ_MEMBER_OPT.name  + "\": \"" +                  test_it_cfg.mem_tag + "\"");
    }
    if (test_seq_param.target.exists)
    {
        test_it_cfg.test_strvect.emplace_back("\"" + TARGET_TEST_SEQ_MEMBER_OPT.name  + "\": \"" +                  test_it_cfg.mem_tag + "\"");
    }
    if (test_seq_param.buffer_size.exists)
    {
        test_it_cfg.test_strvect.emplace_back("\"" + BUFFER_SIZE_TEST_SEQ_MEMBER.name + "\": "   + std::to_string(  test_seq_param.buffer_size.value));
    }
}

void P2PNvmeTest::SetTestSequenceStrDbg( TestItConfig_t & test_it_cfg )
{
    test_it_cfg.test_strvect_dbg.clear();
    test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.duration));
    if (!test_it_cfg.source_nvme)
    {
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + SOURCE_TEST_SEQ_MEMBER_OPT.name + "\": \"" + test_it_cfg.mem_tag + "\"");
    }
    else
    {
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + TARGET_TEST_SEQ_MEMBER_OPT.name + "\": \"" + test_it_cfg.mem_tag + "\"");
    }
    test_it_cfg.test_strvect_dbg.emplace_back("\"" + BUFFER_SIZE_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_it_cfg.buffer_size));
}

void P2PNvmeTest::AppendTestIt( const P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param, TestItConfig_t & test_it_cfg, std::list<TestItConfig_t> & test_list, std::vector<std::string> & test_seq_strvect, std::vector<std::string> & test_seq_strvect_dbg, bool & parse_it_failure )
{
    if (parse_it_failure == RET_SUCCESS)
    {
        parse_it_failure |= GetInMemTopology(test_it_cfg);
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
}

bool P2PNvmeTest::ParseTestSequenceSettings( std::list<TestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    uint parent_test_cnt = 0;

    auto glob_mem_type_used = false;
    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_seq_strvect_dbg;

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

        // Check test sequence defined by source or by target
        std::string src_tgt_member_name;
        test_it_cfg.source_nvme     = false;
        if (test_seq_param.source.exists)
        {
            test_it_cfg.mem_type_tag    = test_seq_param.source.value;
            src_tgt_member_name         = SOURCE_TEST_SEQ_MEMBER_OPT.name;
            // Check if source card is not nodma for p2p_nvme
            if (!(this->m_xbtest_sw_config->GetIsDmaSupported()))
            {
                LogMessage(MSG_GEN_079, {P2P_NVME_MEMBER.name});
                parse_it_failure = RET_FAILURE;
            }
            else
            {
                LogMessage(MSG_P2P_035, {SOURCE_TEST_SEQ_MEMBER_OPT.name, std::to_string(test_list.size()+1), TARGET_TEST_SEQ_MEMBER_OPT.name});
            }
        }
        else if (test_seq_param.target.exists)
        {
            LogMessage(MSG_P2P_035, {TARGET_TEST_SEQ_MEMBER_OPT.name, std::to_string(test_list.size()+1), SOURCE_TEST_SEQ_MEMBER_OPT.name});
            test_it_cfg.mem_type_tag    = test_seq_param.target.value;
            test_it_cfg.source_nvme     = true;
            src_tgt_member_name         = TARGET_TEST_SEQ_MEMBER_OPT.name;
        }
        else
        {
            LogMessage(MSG_P2P_034, {SOURCE_TEST_SEQ_MEMBER_OPT.name, TARGET_TEST_SEQ_MEMBER_OPT.name, std::to_string(test_list.size()+1)});
            parse_it_failure = RET_FAILURE;
        }

        // Check test sequence defined by memory type or by memory target
        if (parse_it_failure == RET_SUCCESS)
        {
            test_it_cfg.mem_type_used = false;
            if (FindStringInVect(test_it_cfg.mem_type_tag, this->m_xbtest_sw_config->GetAvailableBoardMemories()))
            {
                // User has specified a memory type in the test sequence, a test will be created for each tags of this type
                glob_mem_type_used          = true;
                test_it_cfg.mem_type_used   = true;
                test_it_cfg.mem_type        = test_it_cfg.mem_type_tag;
                parse_it_failure |= GetConfigIndexFromType(test_it_cfg.mem_type, test_it_cfg.mem_cfg_idx); // By design this should always ret success
            }
            else if (FindStringInVect(test_it_cfg.mem_type_tag, this->m_xbtest_sw_config->GetAvailableBoardTag()))
            {
                // User has specified a memory tag in the test sequence, only one test will be created for this tag
                test_it_cfg.mem_tag         = test_it_cfg.mem_type_tag;
                // Find the memory type of the memory tag
                parse_it_failure |= GetConfigIndexFromTag(test_it_cfg.mem_tag, test_it_cfg.mem_cfg_idx); // By design this should always ret success
                if (parse_it_failure == RET_SUCCESS)
                {
                    test_it_cfg.mem_type = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].name;
                }
            }
            else
            {
                test_it_cfg.mem_tag = test_it_cfg.mem_type_tag;
                LogMessage(MSG_P2P_024, {src_tgt_member_name, test_it_cfg.mem_type_tag});
                parse_it_failure = RET_FAILURE;
            }
        }

        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= SetOtherTestSequenceItSettings(test_seq_param, test_list, test_it_cfg);
        }

        if (test_it_cfg.mem_type_used)
        {
            LogMessage(MSG_P2P_019, {std::to_string(test_list.size()+1), "memory type (" + test_it_cfg.mem_type + ")", std::to_string(this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].tag.size()+1), "one per each memory tag of the memory type"});
            test_it_cfg.memory_data.mem_data_idx = 0; // Start looking at index 0
            for (const auto & tag : this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].tag)
            {
                test_it_cfg.mem_tag = tag;
                AppendTestIt(test_seq_param, test_it_cfg, test_list, test_seq_strvect, test_seq_strvect_dbg, parse_it_failure);
            }
        }
        else
        {
            AppendTestIt(test_seq_param, test_it_cfg, test_list, test_seq_strvect, test_seq_strvect_dbg, parse_it_failure);
        }

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

    if (glob_mem_type_used)
    {
        LogMessage(MSG_P2P_025, {"card memory type"});
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

bool P2PNvmeTest::CreateClBuffer( std::vector<cl::Buffer> & cl_buffers, const TestItConfig_t & test_it, bool use_p2p_buffer )
{
    auto cl_err = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
    {
        auto mem_flags = static_cast<cl_mem_flags>(CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX);
        std::string flags_str = "CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX | XCL_MEM_TOPOLOGY";

        cl_mem_ext_ptr_t ext;
        ext.param   = nullptr;
        ext.obj     = nullptr;
        ext.flags   = (unsigned)(test_it.memory_data.mem_data_idx) | XCL_MEM_TOPOLOGY;

        if (use_p2p_buffer)
        {
            ext.flags |= XCL_MEM_EXT_P2P_BUFFER;
            flags_str += " | XCL_MEM_EXT_P2P_BUFFER";
        }

        cl_buffers.emplace_back(cl::Buffer(*this->m_cl_context, mem_flags, test_it.buff_size_bytes, &ext, &cl_err));
        CheckClBufferConstructor(cl_err, "index " + std::to_string(buff_idx), flags_str, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
    }
    return RET_SUCCESS;
}

bool P2PNvmeTest::MapClBuffer( std::vector<cl::Buffer> & cl_buffers, std::vector<void *> & cl_buf_ptr, const TestItConfig_t & test_it )
{
    auto cl_err = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::vector<cl::Event> waitEvent;

    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
    {
        cl::Event Event;

        cl_buf_ptr[buff_idx] = this->m_cl_cmd_queue->enqueueMapBuffer(cl_buffers[buff_idx], CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, 0, test_it.buff_size_bytes, nullptr, &Event, &cl_err);
        CheckClEnqueueMapBuffer(cl_err, "index " + std::to_string(buff_idx), "CL_MAP_READ | CL_MAP_WRITE", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

        waitEvent.emplace_back(Event);
        cl_err = cl::WaitForEvents(waitEvent);
        CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

        cl_err = this->m_cl_cmd_queue->finish();
        CheckClCommandQueueFinish(cl_err, "map buffer index " + std::to_string(buff_idx), chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

        waitEvent.clear();
    }
    return RET_SUCCESS;
}

bool P2PNvmeTest::MigrateClBuffer( std::vector<cl::Buffer> & cl_buffers, bool wr_n_rd )
{
    auto cl_err = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    cl::Event Event;
    std::vector<cl::Event> waitEvent;
    cl_mem_migration_flags mem_migration_flags;
    std::string tmp = "Migrate data ";

    if (wr_n_rd)
    {
        tmp += "(Host->Card) ";
        mem_migration_flags = 0;
    }
    else
    {
        tmp += "(Host<-Card) ";
        mem_migration_flags = CL_MIGRATE_MEM_OBJECT_HOST;
    }

    std::vector<cl::Memory> cl_memories;
    cl_memories.reserve(cl_buffers.size());
    for (auto cl_buffer : cl_buffers)
    {
        cl_memories.emplace_back(cl_buffer);
    }

    cl_err  = this->m_cl_cmd_queue->enqueueMigrateMemObjects(cl_memories, mem_migration_flags, nullptr, &Event);
    CheckClEnqueueMigrateMemObjects (cl_err, tmp, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
    waitEvent.emplace_back(Event);

    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

    cl_err = this->m_cl_cmd_queue->finish();
    CheckClCommandQueueFinish(cl_err,  tmp, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

    waitEvent.clear(); // clear events

    return RET_SUCCESS;
}

bool P2PNvmeTest::UnmapClBuffer( std::vector<cl::Buffer> & cl_buffers, std::vector<void *> & cl_buf_ptr )
{
    auto cl_err = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    for (uint64_t buff_idx = 0; buff_idx < cl_buf_ptr.size(); buff_idx++)
    {
        if (cl_buf_ptr[buff_idx] != nullptr)
        {
            cl_err = this->m_cl_cmd_queue->enqueueUnmapMemObject(cl_buffers[buff_idx], cl_buf_ptr[buff_idx]);
            CheckClEnqueueUnmapBuffer(cl_err, "index " + std::to_string(buff_idx), chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

            cl_err = this->m_cl_cmd_queue->finish();
            CheckClCommandQueueFinish(cl_err, "unmap buffer index " + std::to_string(buff_idx), chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
        }
    }
    cl_buf_ptr.clear();
    return RET_SUCCESS;
}

void P2PNvmeTest::ReleaseClBuffer( std::vector<cl::Buffer> & cl_buffers )
{
    cl_buffers.clear();
}

void P2PNvmeTest::ReleaseHostbuffer( std::vector<int *> & host_buf )
{
    for (auto & buf : host_buf)
    {
        DeallocateHostBuffer<int>(buf);
    }
    host_buf.clear();
}

void P2PNvmeTest::StopOnError( const TestItConfig_t & test_it, const uint & test_it_cnt, const bool & test_it_failure, bool & stop_test_it, const std::string & msg )
{
    if (!stop_test_it && (test_it_failure == RET_FAILURE) && (this->m_TC_Cfg.memory[test_it.mem_cfg_idx].stop_on_error))
    {
        LogMessage(MSG_P2P_028, {std::to_string(test_it_cnt), msg});
        stop_test_it = true;
    }
}

int P2PNvmeTest::RunTest()
{
    uint test_it_cnt     = 1;
    auto test_failure    = RET_SUCCESS;
    auto test_it_failure = RET_SUCCESS;

    if (!(this->m_abort))
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Number of test iterations: " + std::to_string(this->m_test_it_list.size())});
        this->m_testcase_queue_value.pending = this->m_test_it_list.size();
        PushTestcaseQueue();
    }

    if (OpenNvmeFd() == RET_FAILURE)
    {
        return TEST_STATE_FAILURE;
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
        LogMessage(MSG_CMN_042, {"\t- Extra test parameters: " + BUFFER_SIZE + ": " + std::to_string(test_it.buffer_size) + " MB, " + TOTAL_SIZE_MEMBER.name + ": " + Float_to_String<double>(((double)test_it.total_size_bytes)/1024.0/1024.0,0) + " MB"});

        uint64_t t_0 = 0;
        uint64_t t_1 = 0;
        Rate_t rate_wr = RST_RATE;
        Rate_t rate_rd = RST_RATE;
        auto stop_test_it           = false;
        auto test_it_data_integ_str = DATA_INTEG_NA;    // default
        auto test_it_bw_result_str  = BW_RESULT_NA;     // default

        int *                   ref_data_buf = nullptr;
        std::vector<int *>      host_buf(test_it.buffer_count, nullptr);
        std::vector<cl::Buffer> cl_buffers;
        std::vector<void *>     cl_buf_ptr(test_it.buffer_count, nullptr);
        std::vector<int *>      cl_buf_data_ptr(test_it.buffer_count, nullptr);

        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_CNT  + ": "            + std::to_string(test_it.buffer_count)});
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_SIZE + " (MB): "       + std::to_string(test_it.buffer_size)});
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_SIZE + " (Bytes): "    + std::to_string(test_it.buff_size_bytes)});
            LogMessage(MSG_DEBUG_TESTCASE, {BUFFER_SIZE + " (int): "      + std::to_string(test_it.buff_size_int)});
            LogMessage(MSG_DEBUG_TESTCASE, {TOT_SIZE    + " (Bytes): "    + std::to_string(test_it.total_size_bytes)});
            LogMessage(MSG_DEBUG_TESTCASE, {TOT_SIZE    + " (MB): "       + std::to_string((double)(test_it.total_size_bytes) / (double)(1024) / (double)(1024))});
        }

        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Allocate reference data buffer"});
            test_it_failure |= AllocateHostBuffer<int>(this->m_page_size, test_it.buff_size_int, &ref_data_buf);
            if (!(this->m_abort) && (test_it_failure == RET_FAILURE))
            {
                LogMessage(MSG_P2P_006);
            }
        }

        auto ref_cnt = (uint)(GetTimestamp());
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
            test_it_failure |= AllocateHostBuffer<int>(this->m_page_size, test_it.buff_size_int, &(host_buf[buff_idx]));
            if (!(this->m_abort) && (test_it_failure == RET_FAILURE))
            {
                LogMessage(MSG_P2P_007, {std::to_string(buff_idx)});
            }
        }

        if (test_it.source_nvme)
        {
            if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Create OpenCL P2P buffers for memory topology index " + std::to_string(test_it.memory_data.mem_data_idx)});
                test_it_failure |= CreateClBuffer(cl_buffers, test_it, CREATE_P2P_BUFFER);
            }
        }
        else
        {
            if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Create OpenCL regular buffers for memory topology index " + std::to_string(test_it.memory_data.mem_data_idx)});
                test_it_failure |= CreateClBuffer(cl_buffers, test_it, CREATE_REGULAR_BUFFER);
            }
        }

        if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Map OpenCL P2P buffers to host access pointers"});
            test_it_failure |= MapClBuffer(cl_buffers, cl_buf_ptr, test_it);
        }
        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && (test_it_failure == RET_SUCCESS); buff_idx++)
        {
            cl_buf_data_ptr[buff_idx] = reinterpret_cast<int *>(cl_buf_ptr[buff_idx]);
        }

        if (test_it.source_nvme)
        {
            if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Write reference data to source NVMe"});
            }
            for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && (test_it_failure == RET_SUCCESS); buff_idx++)
            {
                test_it_failure |= WriteNvmeFd(ref_data_buf, test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
            }
        }
        else
        {
            if (!(this->m_abort) && (test_it_failure == RET_SUCCESS))
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Write reference data to source card"});
            }
            for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && (test_it_failure == RET_SUCCESS); buff_idx++)
            {
                memcpy(cl_buf_data_ptr[buff_idx], ref_data_buf, test_it.buff_size_bytes);
            }
            test_it_failure |= MigrateClBuffer(cl_buffers, MIGRATE_TO_CARD);
        }

        // Used to display elapsed duration
        auto duration_divider = (uint64_t)(test_it.duration) / (uint64_t)(10);
        if (duration_divider == 0)
        {
            duration_divider = 1;
        }

        auto duration_us = (uint64_t)(test_it.duration) * (uint64_t)(1000000);
        uint64_t elapsed_d = 0; // us

        test_it.t_start         = GetTimestamp();
        test_it.wr_rd_cycle_cnt = 0;
        test_it.elapsed         = 0;
        test_it.test_it_stat    = RST_TEST_IT_STAT;

        if (test_it_failure == RET_SUCCESS)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"Initialization done, starting P2P"});

            while ((test_it.elapsed < duration_us) && !(this->m_abort) && !stop_test_it)
            {
                auto data_integ_str = DATA_INTEG_NA; // default

                ////
                // Write
                ////

                // P2P write
                if (test_it.source_nvme)
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"P2P write transfer (NVMe->Card)"});
                        t_0 = GetTimestamp();
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            test_it_failure |= ReadNvmeFd(cl_buf_data_ptr[buff_idx], test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                        }
                        t_1 = GetTimestamp();

                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "P2P write transfer (NVMe->Card)");
                    }
                }
                else
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"P2P write transfer (Card->NVMe)"});
                        t_0 = GetTimestamp();
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            test_it_failure |= WriteNvmeFd(cl_buf_data_ptr[buff_idx], test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                        }
                        t_1 = GetTimestamp();

                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "P2P write transfer (Card->NVMe)");
                    }
                }
                if (!(this->m_abort) && !stop_test_it)
                {
                    if (ComputeRate(test_it, t_1, t_0, rate_wr) == RET_FAILURE)
                    {
                        LogMessage(MSG_P2P_008, {"write"});
                        test_it_failure = RET_FAILURE;
                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "P2P write bandwidth");
                    }
                }

                ////
                // Read
                ////

                // Reset data on source
                if (test_it.source_nvme)
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"Reset data on source NVMe"});
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            memset(host_buf[buff_idx], 0, test_it.buff_size_bytes);
                        }
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && !stop_test_it; buff_idx++)
                        {
                            test_it_failure |= WriteNvmeFd(host_buf[buff_idx], test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                            StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "reset data on source NVMe");
                        }
                    }
                }
                else
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"Reset data on source card"});
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            memset(cl_buf_data_ptr[buff_idx], 0, test_it.buff_size_bytes);
                        }
                        test_it_failure |= MigrateClBuffer(cl_buffers, MIGRATE_TO_CARD);
                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "reset data on source card");
                    }
                }

                // P2P Read
                if (test_it.source_nvme)
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"P2P read transfer (NVMe<-Card)"});
                        t_0 = GetTimestamp();
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            test_it_failure |= WriteNvmeFd(cl_buf_data_ptr[buff_idx], test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                        }
                        t_1 = GetTimestamp();

                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "P2P read transfer (NVMe<-Card)");
                    }
                }
                else
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"P2P read transfer (Card<-NVMe)"});
                        t_0 = GetTimestamp();
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            test_it_failure |= ReadNvmeFd(cl_buf_data_ptr[buff_idx], test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                        }
                        t_1 = GetTimestamp();

                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "P2P read transfer (Card<-NVMe)");
                    }
                }
                if (!(this->m_abort) && !stop_test_it)
                {
                    if (ComputeRate(test_it, t_1, t_0, rate_rd) == RET_FAILURE)
                    {
                        LogMessage(MSG_P2P_008, {"read"});
                        test_it_failure = RET_FAILURE;
                        StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "P2P read bandwidth");
                    }
                }

                // Read data on source
                if (test_it.source_nvme)
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"Read data on source NVMe"});
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            memset(host_buf[buff_idx], 0, test_it.buff_size_bytes);
                        }
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && !stop_test_it; buff_idx++)
                        {
                            test_it_failure |= ReadNvmeFd(host_buf[buff_idx], test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                            StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "Read data on source NVMe");
                        }
                    }
                }
                else
                {
                    if (!(this->m_abort) && !stop_test_it)
                    {
                        LogMessage(MSG_DEBUG_TESTCASE, {"Read data on source card"});
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                        {
                            memset(host_buf[buff_idx], 0, test_it.buff_size_bytes);
                        }
                        for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort) && !stop_test_it; buff_idx++)
                        {
                            memcpy(host_buf[buff_idx], cl_buf_data_ptr[buff_idx], test_it.buff_size_bytes);
                        }
                    }
                }

                // Check data integrity on source
                if (!(this->m_abort) && !stop_test_it)
                {
                    for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                    {
                        if (!this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
                        {
                            data_integ_str  = DATA_INTEG_NC;
                        }
                        else if (CheckDataIntegrity(host_buf[buff_idx], ref_data_buf, test_it.buff_size_bytes, test_it.buff_size_int) == RET_FAILURE)
                        {
                            LogMessage(MSG_P2P_009, {test_it.mem_tag, std::to_string(buff_idx)});
                            data_integ_str  = DATA_INTEG_KO;
                            test_it_failure = RET_FAILURE;
                            StopOnError(test_it, test_it_cnt, test_it_failure, stop_test_it, "data integrity on source");
                        }
                        else if (data_integ_str == DATA_INTEG_NA)
                        {
                            data_integ_str = DATA_INTEG_OK;
                        }
                    }
                }
                // Reload data on data integrity error
                if (!(this->m_abort) && !stop_test_it)
                {
                    if ((data_integ_str == DATA_INTEG_KO) && !(this->m_TC_Cfg.memory[test_it.mem_cfg_idx].stop_on_error))
                    {
                        LogMessage(MSG_P2P_027, {test_it.mem_tag, "[0.." + std::to_string(test_it.buffer_count-1) + "]"});
                        if (test_it.source_nvme)
                        {
                            for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                            {
                                test_it_failure |= WriteNvmeFd(ref_data_buf, test_it.buff_size_bytes, buff_idx*test_it.buff_size_bytes);
                            }
                        }
                        else
                        {
                            for (uint64_t buff_idx = 0; (buff_idx < test_it.buffer_count) && !(this->m_abort); buff_idx++)
                            {
                                memcpy(cl_buf_data_ptr[buff_idx], ref_data_buf, test_it.buff_size_bytes);
                            }
                            test_it_failure |= MigrateClBuffer(cl_buffers, MIGRATE_TO_CARD);
                        }
                    }
                }




                // Global test data integrity result
                if (data_integ_str == DATA_INTEG_NC)
                {
                    test_it_data_integ_str = DATA_INTEG_NC; // not checked
                }
                else if (data_integ_str == DATA_INTEG_NA)
                {
                    test_it_data_integ_str = DATA_INTEG_NA; // stop on error
                }
                else if (data_integ_str == DATA_INTEG_OK)
                {
                    if (test_it_data_integ_str == DATA_INTEG_NA) // n/a at start-up, move to OK after first check OK
                    {
                        test_it_data_integ_str = DATA_INTEG_OK;
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

                if ((test_it.elapsed - elapsed_d) >= (duration_divider * (uint64_t)(1000000)))
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
            LogMessage(MSG_P2P_026, {std::to_string(test_it.wr_rd_cycle_cnt)});

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
        LogMessage(MSG_DEBUG_TESTCASE, {"Unmap OpenCL buffers to host access pointers"});
        test_it_failure |= UnmapClBuffer(cl_buffers, cl_buf_ptr);

        LogMessage(MSG_DEBUG_TESTCASE, {"Release OpenCL buffers"});
        ReleaseClBuffer(cl_buffers);

        LogMessage(MSG_DEBUG_TESTCASE, {"Release host buffers"});
        DeallocateHostBuffer<int>(ref_data_buf);
        ReleaseHostbuffer(host_buf);

        // Results
        if ((test_it_failure == RET_SUCCESS) && !(this->m_abort))
        {
            if (this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_bw & this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
            {
                LogMessage(MSG_P2P_011, {test_it.mem_tag}); // BW/integ pass
            }
            else if (!this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_bw & this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
            {
                LogMessage(MSG_P2P_013, {test_it.mem_tag}); // integ pass
            }
            else if (this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_bw & !this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
            {
                LogMessage(MSG_P2P_037, {test_it.mem_tag}); // BW pass
            }
            else
            {
                LogMessage(MSG_P2P_039, {test_it.mem_tag}); // BW/integ not checked
            }
        }
        else
        {
            if (this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_bw & this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
            {
                LogMessage(MSG_P2P_012, {test_it.mem_tag}); // BW/integ fail
            }
            else if (!this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_bw & this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
            {
                LogMessage(MSG_P2P_014, {test_it.mem_tag}); // integ fail
            }
            else if (this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_bw & !this->m_TC_Cfg.memory[test_it.mem_cfg_idx].check_data_integrity)
            {
                LogMessage(MSG_P2P_038, {test_it.mem_tag}); // BW fail
            }
            else
            {
                LogMessage(MSG_P2P_040, {test_it.mem_tag}); // BW/integ not checked
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

    if (CloseNvmeFd() == RET_FAILURE)
    {
        return TEST_STATE_FAILURE;
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

bool P2PNvmeTest::OpenNvmeFd()
{
    LogMessage(MSG_DEBUG_TESTCASE, {"Open NVMe file"});
    this->m_nvmeFd = open64(this->m_filename.c_str(), O_RDWR | O_DIRECT | O_LARGEFILE);
    if (this->m_nvmeFd == -1)
    {
        LogMessage(MSG_P2P_031, {"open", this->m_filename, strerror(errno)});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool P2PNvmeTest::CloseNvmeFd()
{
    if (this->m_nvmeFd != -1)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Close NVMe file"});
        if (close(this->m_nvmeFd) == -1)
        {
            LogMessage(MSG_P2P_031, {"close", this->m_filename, strerror(errno)});
            return RET_FAILURE;
        }
        this->m_nvmeFd = -1;
    }
    return RET_SUCCESS;
}

// Under Linux 2.4, transfer sizes, and the alignment of the user buffer and the file offset must all be multiples of the logical block size of the file system.
// Under Linux 2.6, alignment to 512-byte boundaries suffices.
// Don't check the size as the minimum buffer size is 1MB which is multiple of 512

bool P2PNvmeTest::WriteNvmeFd( int * ptr, const uint64_t & size, const uint64_t & offset )
{
    auto ret = pwrite64(this->m_nvmeFd, (const void *)(ptr), size, offset);
    if ((uint64_t)(ret) == size)
    {
        return RET_SUCCESS;
    }
    if (ret > 0)
    {
        LogMessage(MSG_P2P_032, {"write", this->m_filename, "only " + std::to_string(ret) + " of " + std::to_string(size) + " Bytes were successfully transferred"});
    }
    else if (ret == 0)
    {
        LogMessage(MSG_P2P_032, {"write", this->m_filename, "no data was transferred"});
    }
    else
    {
        LogMessage(MSG_P2P_032, {"write", this->m_filename, strerror(errno)});
    }
    return RET_FAILURE;
}

bool P2PNvmeTest::ReadNvmeFd( int * ptr, const uint64_t & size, const uint64_t & offset )
{
    auto ret = pread64(this->m_nvmeFd, (void *)(ptr), size, offset);
    if ((uint64_t)(ret) == size)
    {
        return RET_SUCCESS;
    }
    if (ret > 0)
    {
        LogMessage(MSG_P2P_032, {"read", this->m_filename, "only " + std::to_string(ret) + " of " + std::to_string(size) + " Bytes were successfully transferred"});
    }
    else if (ret == 0)
    {
        LogMessage(MSG_P2P_032, {"read", this->m_filename, "no data was transferred"});
    }
    else
    {
        LogMessage(MSG_P2P_032, {"read", this->m_filename, strerror(errno)});
    }
    return RET_FAILURE;
}

bool P2PNvmeTest::GetInMemTopology( TestItConfig_t & test_it_cfg )
{
    return this->m_xbtest_sw_config->GetInMemTopology(test_it_cfg.mem_tag, test_it_cfg.memory_data, true);
}

bool P2PNvmeTest::CheckDataIntegrity( int * host_buf, int * ref_data_buf, const uint64_t & buff_size_bytes, const uint64_t & buff_size_int )
{
    if (memcmp(host_buf, ref_data_buf, buff_size_bytes) != 0)
    {
        LogMessage(MSG_P2P_015);
        // Report where the comparison first failed
        uint64_t data_err_cnt = 0;
        for (uint64_t i = 0; i < buff_size_int; i++)
        {
            if (host_buf[i] != ref_data_buf[i])
            {
                if (data_err_cnt == 0)
                {
                    LogMessage(MSG_P2P_021, {"First error at index : " + std::to_string(i)});
                    LogMessage(MSG_P2P_021, {"\t - Write data: " + std::to_string(ref_data_buf[i])});
                    LogMessage(MSG_P2P_021, {"\t - Read data:  " + std::to_string(host_buf[i])});
                }
                data_err_cnt++;
            }
        }
        LogMessage(MSG_DEBUG_TESTCASE, {"Quantity of incorrect values: " + std::to_string(data_err_cnt) + "/" + std::to_string(buff_size_int)});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool P2PNvmeTest::ComputeRate( const TestItConfig_t & test_it_cfg, const uint64_t & t_1, const uint64_t & t_0, Rate_t & rate )
{
    auto duration = t_1 - t_0; // us

    if (duration == 0)
    {
        return RET_FAILURE;
    }

    rate.inst = (double)(test_it_cfg.total_size_bytes) / (double)(1024) / (double)(1024); // MBytes
    rate.inst *= (double)(1000000);
    rate.inst /= (double)(duration); // MBytes/s

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
    rate.avg   = rate.acc / (test_it_cfg.wr_rd_cycle_cnt + 1);
    rate.valid = true;

    return RET_SUCCESS;
}

bool P2PNvmeTest::CheckRate( TestItConfig_t & test_it_cfg, const Rate_t & rate_wr, const Rate_t & rate_rd, const bool & report_pass_fail )
{
    auto ret = RET_SUCCESS;
    std::string result_str;
    // Select BW range depending on memory type
    auto lo_thresh_wr = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].lo_thresh_wr;
    auto hi_thresh_wr = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].hi_thresh_wr;
    auto lo_thresh_rd = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].lo_thresh_rd;
    auto hi_thresh_rd = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].hi_thresh_rd;

    auto check_bw_disabled  = false;
    auto check_bw_exists    = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].check_bw_exists;
    auto default_check_bw   = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].check_bw;
    auto check_bw           = default_check_bw;

    if (report_pass_fail)
    {
        if (test_it_cfg.duration < MIN_RESULTS_CHECK_DURATION)
        {
            LogMessage(MSG_CMN_051, {DURATION_TEST_SEQ_MEMBER.name + " (" + std::to_string(test_it_cfg.duration) + ") of test is lower than " + std::to_string(MIN_RESULTS_CHECK_DURATION), "bandwidth"});
            IsCheckDisabled(check_bw_exists, default_check_bw, check_bw, check_bw_disabled);
        }
        if (test_it_cfg.buffer_size != this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].nom_buffer_size)
        {
            LogMessage(MSG_CMN_051, {BUFFER_SIZE + " (" + std::to_string(test_it_cfg.buffer_size) + ") of test does not equal " + std::to_string(this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].nom_buffer_size), "bandwidth"});
            IsCheckDisabled(check_bw_exists, default_check_bw, check_bw, check_bw_disabled);
        }
        if (this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].total_size != this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].nom_total_size)
        {
            LogMessage(MSG_CMN_051, {TOTAL_SIZE_MEMBER.name + " (" + std::to_string(this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].total_size) + ") of test does not equal " + std::to_string(this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].nom_total_size), "bandwidth"});
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
    auto rate_wr_chk = rate_wr.inst;
    auto rate_rd_chk = rate_rd.inst;
    if (report_pass_fail)
    {
        rate_wr_chk = rate_wr.avg;
        rate_rd_chk = rate_rd.avg;
    }

    if (test_it_cfg.source_nvme)
    {
        result_str = "\tWRITE {NVMe->PCIe->Card} [MB/s]:";
    }
    else
    {
        result_str = "\tWRITE {Card->PCIe->NVMe} [MB/s]:";
    }
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
    if ( !(rate_wr.valid) || ((uint)(rate_wr_chk) < lo_thresh_wr) || ((uint)(rate_wr_chk) > hi_thresh_wr) )
    {
        result_str += " outside " + wr_bw_range;
        if (report_pass_fail)
        {
            LogMessage(MSG_P2P_016, {result_str});
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
            LogMessage(MSG_P2P_017, {result_str});
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {result_str});
            test_it_cfg.test_it_stat.wr_bw_pass_cnt++;
        }
    }

    if (test_it_cfg.source_nvme)
    {
        result_str = "\tREAD  {NVMe<-PCIe<-Card} [MB/s]:";
    }
    else
    {
        result_str = "\tREAD  {Card<-PCIe<-NVMe} [MB/s]:";
    }
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

    if ( !(rate_rd.valid) || ((uint)(rate_rd_chk) < lo_thresh_rd) || ((uint)(rate_rd_chk) > hi_thresh_rd) )
    {
        result_str += " outside " + rd_bw_range;
        if (report_pass_fail)
        {
            LogMessage(MSG_P2P_016, {result_str});
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
            LogMessage(MSG_P2P_017, {result_str});
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

void P2PNvmeTest::PrintResults( const bool & info_n_debug, const TestItConfig_t & test_it_cfg, const Rate_t & rate_wr, const Rate_t & rate_rd )
{
    auto lo_thresh_wr = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].lo_thresh_wr;
    auto hi_thresh_wr = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].hi_thresh_wr;
    auto lo_thresh_rd = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].lo_thresh_rd;
    auto hi_thresh_rd = this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].hi_thresh_rd;
    auto wr_bw_range  = "[" + std::to_string(lo_thresh_wr) + ", " + std::to_string(hi_thresh_wr) + "]";
    auto rd_bw_range  = "[" + std::to_string(lo_thresh_rd) + ", " + std::to_string(hi_thresh_rd) + "]";

    std::string result = "Bandwidth results (Min, Avg, Max) [MB/s]; ";

    if (test_it_cfg.source_nvme)
    {
        result += "WRITE {NVMe->PCIe->Card} (";
    }
    else
    {
        result += "WRITE {Card->PCIe->NVMe} (";
    }
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
    if (test_it_cfg.source_nvme)
    {
        result += "\tREAD {NVMe<-PCIe<-Card} (";
    }
    else
    {
        result += "\tREAD {Card<-PCIe<-NVMe} (";
    }
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
        LogMessage(MSG_P2P_018, {result});
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {result});
    }

    if (this->m_TC_Cfg.memory[test_it_cfg.mem_cfg_idx].check_bw)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.wr_bw_pass_cnt) + " WRITE  inside " + wr_bw_range});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.rd_bw_pass_cnt) + " READ   inside " + rd_bw_range});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.wr_bw_fail_cnt) + " WRITE outside " + wr_bw_range});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(test_it_cfg.test_it_stat.rd_bw_fail_cnt) + " READ  outside " + rd_bw_range});
    }
}

void P2PNvmeTest::WriteToMeasurementFileResult ( const uint & test_it_cnt, const TestItConfig_t & test_it_cfg, const std::string & data_integ_str, const Rate_t & rate_wr, const Rate_t & rate_rd )
{

    if (this->m_use_outputfile)
    {
        std::vector<std::string> str_vect;
        str_vect.emplace_back(std::to_string(  test_it_cnt                                        ));
        if (test_it_cfg.source_nvme)
        {
            str_vect.emplace_back(             "target"                                                );
        }
        else
        {
            str_vect.emplace_back(             "source"                                                );
        }
        str_vect.emplace_back(                 test_it_cfg.mem_tag                                 );
        str_vect.emplace_back(std::to_string(  test_it_cfg.duration                               ));
        str_vect.emplace_back(std::to_string(  test_it_cfg.buffer_size                            ));
        str_vect.emplace_back(std::to_string(  test_it_cfg.buffer_count                           ));
        str_vect.emplace_back(std::to_string(  test_it_cfg.buffer_count * test_it_cfg.buffer_size ));
        str_vect.emplace_back(std::to_string(  test_it_cfg.wr_rd_cycle_cnt                        ));
        str_vect.emplace_back(                 data_integ_str                                      );
        str_vect.emplace_back(std::to_string(  rate_wr.min                                        ));
        str_vect.emplace_back(std::to_string(  rate_wr.avg                                        ));
        str_vect.emplace_back(std::to_string(  rate_wr.max                                        ));
        str_vect.emplace_back(std::to_string(  rate_rd.min                                        ));
        str_vect.emplace_back(std::to_string(  rate_rd.avg                                        ));
        str_vect.emplace_back(std::to_string(  rate_rd.max                                        ));

        this->m_outputfile_result << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile_result.flush();
    }
}

void P2PNvmeTest::WriteToMeasurementFileDetail ( const uint & test_it_cnt, const TestItConfig_t & test_it_cfg, const std::string & data_integ_str, const Rate_t & rate_wr, const Rate_t & rate_rd)
{
    std::vector<std::string> str_vect;
    str_vect.emplace_back(std::to_string(  test_it_cnt                 ));
    if (test_it_cfg.source_nvme)
    {
        str_vect.emplace_back(             "target"                         );
    }
    else
    {
        str_vect.emplace_back(             "source"                         );
    }
    str_vect.emplace_back(                 test_it_cfg.mem_tag          );
    str_vect.emplace_back(std::to_string(  test_it_cfg.buffer_size     ));
    str_vect.emplace_back(std::to_string(  test_it_cfg.wr_rd_cycle_cnt ));
    str_vect.emplace_back(                 data_integ_str               );
    str_vect.emplace_back(std::to_string(  rate_wr.inst                ));
    str_vect.emplace_back(std::to_string(  rate_wr.min                 ));
    str_vect.emplace_back(std::to_string(  rate_wr.avg                 ));
    str_vect.emplace_back(std::to_string(  rate_wr.max                 ));
    str_vect.emplace_back(std::to_string(  rate_rd.inst                ));
    str_vect.emplace_back(std::to_string(  rate_rd.min                 ));
    str_vect.emplace_back(std::to_string(  rate_rd.avg                 ));
    str_vect.emplace_back(std::to_string(  rate_rd.max                 ));

    if (this->m_use_outputfile)
    {
        this->m_outputfile_detail << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile_detail.flush();

        (void)OpenRTOutputFile(this->m_RT_outputfile_detail_name, this->m_RT_outputfile_detail); // Don't check if open worked..
        this->m_RT_outputfile_detail << this->m_RT_outputfile_detail_head + "\n" + StrVectToStr(str_vect, ",");
        this->m_RT_outputfile_detail.flush();
        this->m_RT_outputfile_detail.close();
    }
}

bool P2PNvmeTest::StartCU()
{
    return RET_FAILURE;
}
bool P2PNvmeTest::EnableWatchdogClkThrotDetection()
{
    return RET_FAILURE;
}
bool P2PNvmeTest::StopCU()
{
    return RET_FAILURE;
}
bool P2PNvmeTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_FAILURE; // not a real xbtest HW IP, no watchdog
}

} // namespace

#endif