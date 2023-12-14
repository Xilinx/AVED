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

#include "gtlpbktest.h"

namespace xbtest
{
    
GTLpbkTest::GTLpbkTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
    const Gt_Parameters_t & test_parameters, const uint & cu_idx
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_gt_index = device->GetGtlpbkCuGTIndex(cu_idx);

    this->m_log_header            = LOG_HEADER_GTLPBK;
    this->m_log_header.replace(LOG_HEADER_GTLPBK.find(REP), REP.length(), std::to_string(device->GetGtlpbkCuGTIndex(cu_idx)));

    this->m_devicemgt         = device_mgt;
    this->m_test_parameters   = test_parameters;
    this->m_cu_idx            = cu_idx;

    this->m_queue_testcase  = GT_LPBK_MEMBER.name;
    this->m_queue_thread    = "GT_LPBK[" + std::to_string(device->GetGtlpbkCuGTIndex(cu_idx)) + "]";
}

GTLpbkTest::~GTLpbkTest() = default;

bool GTLpbkTest::PreSetup()
{
    auto global_settings_failure = RET_SUCCESS;
    this->m_state = TestState::TS_PRE_SETUP;
    
    LogMessage(MSG_CMN_004);

    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }
    
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        this->m_TC_Cfg.lane_param[lane_idx] = RSTC_LANEPARAM;
    }

    global_settings_failure |= GetTestSequence();

    // Get lane configurations from JSON. For each parameter:
    //      1) Get this->m_TC_Cfg.global_param, the configuration overwrite for all lines.
    //          Note this->m_TC_Cfg.global_param is then set as default/nominal configuration to the lane configuration overwrite.
    //      2) Get this->m_TC_Cfg.lane_param[lane_idx], lane_idx in GT_LANE_INDEXES, the configuration overwrite for lane "lane_idx"

    // default configuration of GT; all lanes configured identically
    global_settings_failure |= GetJsonParamStr(GT_SETTINGS_MEMBER, this->m_test_parameters.global_config, SUPPORTED_GT_SETTINGS, this->m_TC_Cfg.global_param.gt_settings, GT_SETTINGS_MODULE);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        this->m_TC_Cfg.lane_param[lane_idx].gt_settings = this->m_TC_Cfg.global_param.gt_settings;
    }

    std::string tx_polarity;
    uint32_t    tx_diffctrl;
    uint32_t    tx_pre_emphasis;
    uint32_t    tx_post_emphasis;
    bool        rx_equaliser_lpm;

    auto gt_settings = this->m_xbtest_sw_config->GetGTSettings(this->m_gt_index);

    if (this->m_TC_Cfg.global_param.gt_settings == GT_SETTINGS_CABLE)
    {
        tx_polarity      = gt_settings.transceivers.cable.tx_polarity;
        tx_diffctrl      = gt_settings.transceivers.cable.tx_differential_swing_control;
        tx_pre_emphasis  = gt_settings.transceivers.cable.tx_pre_emphasis;
        tx_post_emphasis = gt_settings.transceivers.cable.tx_post_emphasis;
        rx_equaliser_lpm = RX_EQUALISER_DFM_EN;

        if (StrMatchNoCase(gt_settings.transceivers.cable.rx_equalizer, RX_EQUALISER_LPM))
        {
            rx_equaliser_lpm = RX_EQUALISER_LPM_EN;
        }
    }
    else
    {
        tx_polarity      = gt_settings.transceivers.module.tx_polarity;
        tx_diffctrl      = gt_settings.transceivers.module.tx_differential_swing_control;
        tx_pre_emphasis  = gt_settings.transceivers.module.tx_pre_emphasis;
        tx_post_emphasis = gt_settings.transceivers.module.tx_post_emphasis;
        rx_equaliser_lpm = RX_EQUALISER_DFM_EN;

        if (StrMatchNoCase(gt_settings.transceivers.module.rx_equalizer, RX_EQUALISER_LPM))
        {
            rx_equaliser_lpm = RX_EQUALISER_LPM_EN;
        }
    }

    // overwrite each GT settings globally then allow a per lane overwrite
    // overwrite gt_tx_diffctrl
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_DIFFCTRL, tx_diffctrl, MAX_GT_TX_DIFFCTRL, this->m_TC_Cfg.global_param.gt_tx_diffctrl);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_DIFFCTRL, this->m_TC_Cfg.global_param.gt_tx_diffctrl, MAX_GT_TX_DIFFCTRL, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl);
    }
    // overwrite gt_tx_pre_emph
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_PRE_EMPH_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_PRE_EMPH, tx_pre_emphasis, MAX_GT_TX_PRE_EMPH, this->m_TC_Cfg.global_param.gt_tx_pre_emph);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_PRE_EMPH_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_PRE_EMPH, this->m_TC_Cfg.global_param.gt_tx_pre_emph, MAX_GT_TX_PRE_EMPH, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph);
    }
    // overwrite gt_tx_post_emph
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_POST_EMPH_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_POST_EMPH, tx_post_emphasis, MAX_GT_TX_POST_EMPH, this->m_TC_Cfg.global_param.gt_tx_post_emph);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_POST_EMPH_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_POST_EMPH, this->m_TC_Cfg.global_param.gt_tx_post_emph, MAX_GT_TX_POST_EMPH, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_post_emph);
    }
    // overwrite gt_rx_use_lpm
    global_settings_failure |= GetJsonParamBool(GT_RX_USE_LPM_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.global_param.gt_rx_use_lpm,  rx_equaliser_lpm);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamBool(GT_RX_USE_LPM_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, this->m_TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm, this->m_TC_Cfg.global_param.gt_rx_use_lpm);
    }
    // overwrite tx_polarity
    global_settings_failure |= GetJsonParamStr(GT_TX_POLARITY_MEMBER, this->m_test_parameters.global_config, SUPPORTED_TX_POLARITY_TYPE, this->m_TC_Cfg.global_param.tx_polarity,  tx_polarity);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamStr(GT_TX_POLARITY_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, SUPPORTED_TX_POLARITY_TYPE, this->m_TC_Cfg.lane_param[lane_idx].tx_polarity, this->m_TC_Cfg.global_param.tx_polarity);
    }

    LogMessage(MSG_CMN_021, {"Test configuration:"});
    LogMessage(MSG_CMN_021, {"- Global configuration parameters"});
    PrintLaneParam(this->m_TC_Cfg.global_param);

    // Display here all lane parameters including hidden in DEBUG
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        LogMessage(MSG_CMN_021, {"- Lane " + std::to_string(lane_idx) + " configuration parameters"});
        PrintLaneParam(this->m_TC_Cfg.lane_param[lane_idx]);
    }

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

void GTLpbkTest::Run()
{
    this->m_state     = TestState::TS_RUNNING;
    this->m_result    = TestResult::TR_FAILED;

    auto test_state = TEST_STATE_FAILURE;

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

void GTLpbkTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void GTLpbkTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

bool GTLpbkTest::WriteGtlpbkCu( const uint & address, const uint & value)
{
    return this->m_device->WriteGtlpbkCu(this->m_cu_idx, address, value);
}

bool GTLpbkTest::ReadGtlpbkCu( const uint & address, uint & read_data )
{
    return this->m_device->ReadGtlpbkCu(this->m_cu_idx, address, read_data);
}

void GTLpbkTest::ResetGT()
{
    // simple write to the register pulses the reset
    WriteGtlpbkCu(RESET_REG_ADDR, 0x0);
}

void GTLpbkTest::SendGTCfg()
{
    uint write_data;
    uint read_data;
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        write_data =   (  (this->m_TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm ? 0x1 : 0x0)                        << 24 )
                     | ( ((this->m_TC_Cfg.lane_param[lane_idx].tx_polarity == TX_POLARITY_INVERTED) ? 0x1 : 0x0) << 22 )
                     | (  (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_post_emph & 0x001F)                         << 17 )
                     | (  (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph  & 0x001F)                         << 5 )
                     | (  (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl  & 0x001F)                         << 0 );
        WriteGtlpbkCu(GT_LANES_CFG_REG_ADDR[lane_idx], write_data);
    }
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        ReadGtlpbkCu(GT_LANES_CFG_REG_ADDR[lane_idx], read_data);
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- read data: 0x" + NumToStrHex<uint>(read_data)});
    }
}

int GTLpbkTest::RunTest()
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
        std::map<uint, bool> lane_failure;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            lane_failure[lane_idx] = RET_SUCCESS;
        }

        LogMessage(MSG_CMN_032, {std::to_string(test_it_cnt), StrVectToTest(test_it.test_strvect)}); // start test
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Extrapolated test sequence: " + StrVectToTest(test_it.test_strvect_dbg)});

        if (test_it.cmd == CONFIG_25GBE)
        {
            SendGTCfg();
            ResetGT();
        }
        else if (test_it.cmd == TX_RX_RST)
        {
            ResetGT();
        }
        else if (test_it.cmd == RUN )
        {
            //do nothing
        }

        auto duration_divider = test_it.duration/10;
        if (duration_divider == 0)
        {
            duration_divider = 1;
        }

        // Loop until done or abort
        for (uint i = test_it.duration; (i >= 1) && (!(this->m_abort)); i--)
        {
            if (((i % duration_divider == 0) || (i == test_it.duration)) && (!(this->m_abort)))
            {
                LogMessage(MSG_CMN_048, {std::to_string(i), ""});
            }
            // GetFreqencyCounters();
            WaitSecTick(1);

            this->m_testcase_queue_value.remaining_time = std::to_string(i);
            PushTestcaseQueue();
        }
        LogMessage(MSG_CMN_049); // test duration reached

        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if (lane_failure[lane_idx] == RET_SUCCESS)
            {
                this->m_test_it_lane_results[lane_idx][test_it_cnt-1] = TITR_PASSED;
            }
            else
            {
                this->m_test_it_lane_results[lane_idx][test_it_cnt-1] = TITR_FAILED;
            }
        }

        if (test_it_failure == RET_SUCCESS)
        {
            LogMessage(MSG_ETH_012);
        }
        else
        {
            LogMessage(MSG_ETH_013);
        }

        if (this->m_abort)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            // Set all remaining test result to aborted
            for (uint i = test_it_cnt - 1; i < this->m_test_sequence.size(); i++)
            {
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    this->m_test_it_lane_results[lane_idx][i] = TITR_ABORTED;
                }
                this->m_testcase_queue_value.completed++;
                this->m_testcase_queue_value.pending--;
                this->m_testcase_queue_value.failed++;
            }
        }
        else if (test_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            test_failure = RET_FAILURE;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.failed++;
        }
        else
        {
            LogMessage(MSG_CMN_033_PASS, {std::to_string(test_it_cnt)});
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

bool GTLpbkTest::StartCU()
{
    return RET_SUCCESS;
}

bool GTLpbkTest::EnableWatchdogClkThrotDetection()
{
    return RET_SUCCESS;
}

bool GTLpbkTest::StopCU()
{
    return RET_SUCCESS;
}

bool GTLpbkTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_SUCCESS;
}

bool GTLpbkTest::GetTestSequence()
{
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        this->m_test_it_lane_results[lane_idx] = {};
    }
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_test_sequence = TestcaseParamCast<std::vector<GTLpbk_Test_Sequence_Parameters_t>>(it->second);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            for (uint ii = 0; ii < this->m_test_sequence.size(); ii++)
            {
                this->m_test_it_lane_results[lane_idx].emplace_back(TITR_NOT_TESTED);
            }
        }
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool GTLpbkTest::ParseTestSequenceSettings( std::list<GTLpbkTestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    int  test_cnt = 0;

    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_sequence_dbg;

    for (const auto & test_seq_param : this->m_test_sequence)
    {
        if (this->m_abort)
        {
            break;
        }

        auto parse_it_failure = RET_SUCCESS;
        GTLpbkTestItConfig_t test_it_cfg;

        test_cnt++;

        test_it_cfg.test_strvect = {}; // Displays what user entered
        test_it_cfg.test_strvect_dbg = {}; // Displays with extrapolated parameters

        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_seq_param.duration.value, MIN_DURATION, MAX_DURATION);
            test_it_cfg.duration = test_seq_param.duration.value;
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure = CheckStringInSet(MODE_TEST_SEQ_MEMBER.name, test_seq_param.mode.value, SUPPORTED_GT_LPBK_SEQUENCE_MODE);
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            // mode already checked in input parser
            test_it_cfg.cmd_str = test_seq_param.mode.value;

            if (StrMatchNoCase(test_seq_param.mode.value, RUN_STR))
            {
                test_it_cfg.cmd = RUN;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, TX_RX_RST_STR))
            {
                test_it_cfg.cmd = TX_RX_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_10GBE_NO_FEC_STR))
            {
                test_it_cfg.cmd = CONFIG_10GBE;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_25GBE_NO_FEC_STR))
            {
                test_it_cfg.cmd = CONFIG_25GBE;
            }
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");

        parse_failure |= parse_it_failure;

        if (parse_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_025,{std::to_string(test_cnt)});

            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + " contains erroneous parameters");
            parse_error_cnt ++;
            if (parse_error_cnt > MAX_NUM_PARSER_ERROR)
            {
                break;
            }
        }
        else
        {
            // the test is valid; add it to the test_list
            test_list.emplace_back(test_it_cfg);
            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + ". " + std::to_string(test_seq_param.num_param) + " parameters provided");
        }

        test_seq_strvect.emplace_back(StrVectToTest(test_it_cfg.test_strvect));
    }

    // display the entire test sequence
    LogMessage(MSG_DEBUG_TESTCASE, {"Extrapolated test sequence:"});
    for (const auto & test : test_sequence_dbg)
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

void GTLpbkTest::PrintLaneParam ( const LaneParam_t & lane_param )
{
    LogMessage(MSG_CMN_021, {"\t- " + GT_SETTINGS_MEMBER.name       + " : " +                lane_param.gt_settings});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_DIFFCTRL_MEMBER.name    + " : " + std::to_string(lane_param.gt_tx_diffctrl)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_PRE_EMPH_MEMBER.name    + " : " + std::to_string(lane_param.gt_tx_pre_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POST_EMPH_MEMBER.name   + " : " + std::to_string(lane_param.gt_tx_post_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_RX_USE_LPM_MEMBER.name     + " : " +      BoolToStr(lane_param.gt_rx_use_lpm)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POLARITY_MEMBER.name    + " : " +                lane_param.tx_polarity});
}

} // namespace
