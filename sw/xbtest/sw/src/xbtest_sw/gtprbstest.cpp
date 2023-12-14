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

#include "gtprbstest.h"

namespace xbtest
{

GTPrbsTest::GTPrbsTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
    const Gt_Parameters_t & test_parameters, const uint & cu_idx
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_gt_index = device->GetGtPrbsCuGTIndex(cu_idx);

    this->m_log_header            = LOG_HEADER_GTPRBS;
    this->m_log_header.replace(LOG_HEADER_GTPRBS.find(REP), REP.length(), std::to_string(device->GetGtPrbsCuGTIndex(cu_idx)));

    this->m_devicemgt         = device_mgt;
    this->m_test_parameters   = test_parameters;
    this->m_cu_idx            = cu_idx;

    this->m_queue_testcase  = GT_PRBS_MEMBER.name;
    this->m_queue_thread    = "GT_PRBS[" + std::to_string(device->GetGtPrbsCuGTIndex(cu_idx)) + "]";
}

GTPrbsTest::~GTPrbsTest() = default;

bool GTPrbsTest::PreSetup()
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
    uint32_t    tx_main_cursor;
    uint32_t    tx_diffctrl;
    uint32_t    tx_pre_emphasis;
    uint32_t    tx_post_emphasis;
    bool        rx_equaliser_lpm;

    auto gt_settings = this->m_xbtest_sw_config->GetGTSettings(this->m_gt_index);

    if (this->m_TC_Cfg.global_param.gt_settings == GT_SETTINGS_CABLE)
    {
        tx_polarity      = gt_settings.transceivers.cable.tx_polarity;
        tx_main_cursor   = gt_settings.transceivers.cable.tx_main_cursor;
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
        tx_main_cursor   = gt_settings.transceivers.module.tx_main_cursor;
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
    // overwrite prbs_error_threshold
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_PRBS_ERROR_THRESHOLD_MEMBER, this->m_test_parameters.global_config, MIN_PRBS_ERROR_THRESHOLD, NOM_PRBS_ERROR_THRESHOLD, MAX_PRBS_ERROR_THRESHOLD, this->m_TC_Cfg.global_param.prbs_error_threshold);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_PRBS_ERROR_THRESHOLD_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_PRBS_ERROR_THRESHOLD, this->m_TC_Cfg.global_param.prbs_error_threshold, MAX_PRBS_ERROR_THRESHOLD, this->m_TC_Cfg.lane_param[lane_idx].prbs_error_threshold);
    }
    // gt_loopback
    global_settings_failure |= GetJsonParamStr(GT_LOOPBACK_MEMBER, this->m_test_parameters.global_config, SUPPORTED_GT_LPBK_TYPE, this->m_TC_Cfg.global_param.gt_lpbk,  GT_LPBK_DISABLE);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamStr(GT_LOOPBACK_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, SUPPORTED_GT_LPBK_TYPE, this->m_TC_Cfg.lane_param[lane_idx].gt_lpbk, this->m_TC_Cfg.global_param.gt_lpbk);
    }

    // disable_ref_prbs
    global_settings_failure |= GetJsonParamBool(GT_DISABLE_REF_PRBS_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.global_param.disable_ref_prbs,false);

    // overwrite gt_tx_diffctrl
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_DIFFCTRL, tx_diffctrl, MAX_GT_TX_DIFFCTRL, this->m_TC_Cfg.global_param.gt_tx_diffctrl);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_DIFFCTRL, this->m_TC_Cfg.global_param.gt_tx_diffctrl, MAX_GT_TX_DIFFCTRL, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl);
    }
    // overwrite gt_tx_main_cursor
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_MAIN_CURSOR_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_MAIN_CURSOR, tx_main_cursor, MAX_GT_TX_MAIN_CURSOR, this->m_TC_Cfg.global_param.gt_tx_main_cursor);
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_MAIN_CURSOR_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_MAIN_CURSOR, this->m_TC_Cfg.global_param.gt_tx_main_cursor, MAX_GT_TX_MAIN_CURSOR, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_main_cursor);
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
    LogMessage(MSG_CMN_021, {"\t- " + GT_DISABLE_REF_PRBS_MEMBER.name       + " : " +      BoolToStr(this->m_TC_Cfg.global_param.disable_ref_prbs)});
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

void GTPrbsTest::Run()
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

void GTPrbsTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void GTPrbsTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

bool GTPrbsTest::WriteGtPrbsCu( const uint & address, const uint & value )
{
    return this->m_device->WriteGtPrbsCu(this->m_cu_idx, address, value);
}

bool GTPrbsTest::ReadGtPrbsCu( const uint & address, uint & read_data )
{
    return this->m_device->ReadGtPrbsCu(this->m_cu_idx, address, read_data);
}

bool GTPrbsTest::ResetGT()
{
    auto ret = CheckGTPowerGood();
    uint read_data;

    //check power good, if not set, reset is not allowed according to UG578
    if (ret == RET_SUCCESS)
    {
        this->m_device->LockGTRstMtx(this->m_gt_index);
        ReadGtPrbsCu (CTRL_REG_ADDR, read_data);
        WriteGtPrbsCu(CTRL_REG_ADDR, read_data | RESET_GT);
        WaitSecTick(1);
        this->m_device->UnLockGTRstMtx(this->m_gt_index);
    }
    return ret;
}

bool GTPrbsTest::CheckGTPowerGood()
{
    auto ret = RET_SUCCESS;
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    //check power good
    if ((read_data & GT_POWER_GOOD) != GT_POWER_GOOD)
    {
        LogMessage(MSG_ETH_064);
        ret = RET_FAILURE;
    }
    return ret;
}

bool GTPrbsTest::CheckResetDone()
{
    auto ret = RET_SUCCESS;
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    //check both reset done
    if ((read_data & GT_TX_RST_DONE) != GT_TX_RST_DONE)
    {
        LogMessage(MSG_ETH_065, {"Tx"});
        ret = RET_FAILURE;
    }

    if ((read_data & GT_RX_RST_DONE) != GT_RX_RST_DONE)
    {
        LogMessage(MSG_ETH_065, {"Rx"});
        ret = RET_FAILURE;
    }

    return ret;
}

bool GTPrbsTest::CheckPllLock()
{
    auto ret = RET_SUCCESS;
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    //check both reset done
    if ((read_data & PLL_LOCK) != PLL_LOCK)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"ERROR: PLL not locked"});
        ret = RET_FAILURE;
    }
    return ret;
}

void GTPrbsTest::EnableTXPrbs()
{
    auto data = TX_ENABLE;
    if (this->m_TC_Cfg.global_param.disable_ref_prbs)
    {
        data = data | DISABLE_REF_PRBS;
    }
    WriteGtPrbsCu(CTRL_REG_ADDR, data);
}

void GTPrbsTest::DisableTXPrbs()
{
    WriteGtPrbsCu(CTRL_REG_ADDR, 0x0);
}

void GTPrbsTest::EnableRXPrbsCheck()
{
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    WriteGtPrbsCu(CTRL_REG_ADDR, read_data | RX_ENABLE);
}

void GTPrbsTest::DisableRXPrbsCheck()
{
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    WriteGtPrbsCu(CTRL_REG_ADDR, read_data & ~RX_ENABLE);
}

void GTPrbsTest::SendGTCfg()
{
    uint write_data;
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {

        auto lane_lpbk = GT_LPBK_DISABLE_CFG;
        if (this->m_TC_Cfg.lane_param[lane_idx].gt_lpbk == GT_LPBK_NEAR_END_PCS)
        {
            lane_lpbk = GT_LPBK_NEAR_END_PCS_CFG;
        }
        else if (this->m_TC_Cfg.lane_param[lane_idx].gt_lpbk == GT_LPBK_NEAR_END_PMA)
        {
            lane_lpbk = GT_LPBK_NEAR_END_PMA_CFG;
        }
        else if (this->m_TC_Cfg.lane_param[lane_idx].gt_lpbk == GT_LPBK_FAR_END_PCS)
        {
            lane_lpbk = GT_LPBK_FAR_END_PCS_CFG;
        }
        else if (this->m_TC_Cfg.lane_param[lane_idx].gt_lpbk == GT_LPBK_FAR_END_PMA)
        {
            lane_lpbk = GT_LPBK_FAR_END_PMA_CFG;
        }

        write_data = (   (lane_lpbk & 0x7) << 28)
                     | ( (this->m_TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm ? 0x1 : 0x0) << 24 )
                     | ( ((this->m_TC_Cfg.lane_param[lane_idx].tx_polarity == TX_POLARITY_INVERTED) ? 0x1 : 0x0) << 22 )
                     | ( (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_post_emph   & 0x001F) << 17 )
                     | ( (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_main_cursor & 0x007F) << 10 )
                     | ( (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph    & 0x001F) << 5  )
                     | ( (this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl    & 0x001F) << 0  );
        WriteGtPrbsCu(GT_LANES_CFG_REG_ADDR[lane_idx], write_data);
    }
    /*
    uint read_data;
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        ReadGtPrbsCu(GT_LANES_CFG_REG_ADDR[lane_idx], read_data);
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- read data: 0x" + NumToStrHex<uint>(read_data)});
    }
    */
}

void GTPrbsTest::ClearLaneStatus()
{
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    WriteGtPrbsCu(CTRL_REG_ADDR, read_data | CLEAR_RX_PRBS_ERR | CLEAR_RX_SEED_ZERO | CLEAR_TX_SEED_ZERO);
}

void GTPrbsTest::InsertError(uint lane_idx)
{
    uint read_data;
    ReadGtPrbsCu(CTRL_REG_ADDR, read_data);
    WriteGtPrbsCu(CTRL_REG_ADDR, read_data | (TX_ERR_INJ_LN_0 << lane_idx));
}

bool GTPrbsTest::CheckLaneStatus(std::map<uint, bool> & lane_failure, uint run_duration )
{
    // global result of all lanes
    auto ret = RET_SUCCESS;
    // current result for a single lane
    bool lane_error;
    bool prbs_error;

    LogMessage(MSG_ETH_002);
    uint global_status;
    ReadGtPrbsCu(CTRL_REG_ADDR, global_status);

    // And Parse each of lane results
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        lane_error = RET_SUCCESS;
        prbs_error = RET_SUCCESS;

        // Fail if we detect PRBS error or null seeds
        if ( (global_status & RX_PRBS_ERR[lane_idx]) == RX_PRBS_ERR[lane_idx] )
        {
            prbs_error = RET_FAILURE;
        }

        uint tx_word_cnt_lsb;
        uint tx_word_cnt_msb;
        uint rx_word_cnt_lsb;
        uint rx_word_cnt_msb;

        ReadGtPrbsCu(TX_WORD_CNT_LSB_REG_ADDR[lane_idx], tx_word_cnt_lsb);
        ReadGtPrbsCu(TX_WORD_CNT_MSB_REG_ADDR[lane_idx], tx_word_cnt_msb);
        ReadGtPrbsCu(RX_WORD_CNT_LSB_REG_ADDR[lane_idx], rx_word_cnt_lsb);
        ReadGtPrbsCu(RX_WORD_CNT_MSB_REG_ADDR[lane_idx], rx_word_cnt_msb);

        auto word_cnt_msb       = (uint64_t)(rx_word_cnt_msb);
        auto rx_word_cnt        = (uint64_t)(word_cnt_msb << 32) + (uint64_t)(rx_word_cnt_lsb);

        word_cnt_msb       = (uint64_t)(tx_word_cnt_msb);
        auto tx_word_cnt        = (uint64_t)(word_cnt_msb << 32) + (uint64_t)(tx_word_cnt_lsb);

        if (rx_word_cnt == 0)
        {
            LogMessage(MSG_ETH_061, {std::to_string(lane_idx),"received"});
            ret = RET_FAILURE;
            lane_error = RET_FAILURE;
        }
        else
        {
            // auto data_rate =  ( (double)(rx_word_cnt) * (double)(16) ) / ( (double)(run_duration) * (double)(1000000000) );
            auto data_rate =  ( (double)(rx_word_cnt) * (double)(64) ) / ( (double)(run_duration) * (double)(1000000000) );
            LogMessage(MSG_ETH_062, {std::to_string(lane_idx),"received", std::to_string(rx_word_cnt)});
            LogMessage(MSG_DEBUG_TESTCASE, {"Lane[" + std::to_string(lane_idx) + "]: RX data rate of " + std::to_string(data_rate) + " over " + std::to_string(run_duration) + " sec"});

            if (prbs_error == RET_FAILURE)
            {
                uint rx_err_word_cnt_lsb;
                uint rx_err_word_cnt_msb;

                ReadGtPrbsCu(RX_ERR_WORD_CNT_LSB_REG_ADDR[lane_idx], rx_err_word_cnt_lsb);
                ReadGtPrbsCu(RX_ERR_WORD_CNT_MSB_REG_ADDR[lane_idx], rx_err_word_cnt_msb);

                auto err_word_cnt_msb       = (uint64_t)(rx_err_word_cnt_msb);
                auto rx_err_word_cnt        = (uint64_t)(err_word_cnt_msb << 32) + (uint64_t)(rx_err_word_cnt_lsb);

                auto error_rate = (double)(rx_err_word_cnt) * (double)(100) / (double)(rx_word_cnt);

                std::string error_rate_str;
                if (error_rate > 100)
                {
                    error_rate_str = "100";
                }
                else if (error_rate < 1)
                {
                    error_rate_str = "< 1";
                }
                else
                {
                    error_rate_str = std::to_string(uint(error_rate));
                }

                if (error_rate > (double)(this->m_TC_Cfg.lane_param[lane_idx].prbs_error_threshold))
                {
                    ret = RET_FAILURE;
                    lane_error = RET_FAILURE;
                    LogMessage(MSG_ETH_057, {std::to_string(lane_idx)});
                    LogMessage(MSG_ETH_063, {std::to_string(lane_idx), std::to_string(rx_err_word_cnt), error_rate_str, std::to_string(this->m_TC_Cfg.lane_param[lane_idx].prbs_error_threshold)});
                }
                else
                {
                    LogMessage(MSG_ETH_073, {std::to_string(lane_idx)});
                    LogMessage(MSG_ETH_074, {std::to_string(lane_idx), std::to_string(rx_err_word_cnt), error_rate_str, std::to_string(this->m_TC_Cfg.lane_param[lane_idx].prbs_error_threshold)});
                }
            }
            else
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"PRBS good for lane["+std::to_string(lane_idx)+"]"});
            }
        }
        if (tx_word_cnt == 0)
        {
            LogMessage(MSG_ETH_061, {std::to_string(lane_idx),"transmitted"});
            ret = RET_FAILURE;
            lane_error = RET_FAILURE;
        }
        else
        {
            // auto data_rate =  ( (double)(tx_word_cnt) * (double)(16) ) / ( (double)(run_duration) * (double)(1000000000) );
            auto data_rate =  ( (double)(tx_word_cnt) * (double)(64) ) / ( (double)(run_duration) * (double)(1000000000) );
            LogMessage(MSG_ETH_062, {std::to_string(lane_idx),"transmitted", std::to_string(tx_word_cnt)});
            LogMessage(MSG_DEBUG_TESTCASE, {"Lane[" + std::to_string(lane_idx) + "]: TX data rate of " + std::to_string(data_rate) + " over " + std::to_string(run_duration) + " sec"});
        }

        if ( (global_status & RX_SEED_ZERO[lane_idx]) == RX_SEED_ZERO[lane_idx] )
        {
            LogMessage(MSG_ETH_058, { std::to_string(lane_idx), "Rx"});
            ret = RET_FAILURE;
            lane_error = RET_FAILURE;
        }
        if ( (global_status & TX_SEED_ZERO[lane_idx]) == TX_SEED_ZERO[lane_idx] )
        {
            LogMessage(MSG_ETH_058, { std::to_string(lane_idx),"Tx"});
            ret = RET_FAILURE;
            lane_error = RET_FAILURE;
        }

        if (lane_error == RET_SUCCESS)
        {
            LogMessage(MSG_ETH_059, {std::to_string(lane_idx)});
        }
        else
        {
            LogMessage(MSG_ETH_060, {std::to_string(lane_idx)});
        }
        // save the current lane result into the array of results
        lane_failure[lane_idx] |= lane_error;
    }
    return ret;
}


bool GTPrbsTest::CheckLanesError(std::map<uint, uint64_t> & lanes_error)
{
    auto ret = RET_SUCCESS;
    // Parse each of lane results
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        uint rx_err_word_cnt_lsb;
        uint rx_err_word_cnt_msb;

        ReadGtPrbsCu(RX_ERR_WORD_CNT_LSB_REG_ADDR[lane_idx], rx_err_word_cnt_lsb);
        ReadGtPrbsCu(RX_ERR_WORD_CNT_MSB_REG_ADDR[lane_idx], rx_err_word_cnt_msb);

        auto err_word_cnt_msb       = (uint64_t)(rx_err_word_cnt_msb);
        auto rx_err_word_cnt        = (uint64_t)(err_word_cnt_msb << 32) + (uint64_t)(rx_err_word_cnt_lsb);

        lanes_error[lane_idx] = rx_err_word_cnt;
        if (rx_err_word_cnt != 0)
        {
            ret = RET_FAILURE;
        }
    }
    return ret;

}

int GTPrbsTest::RunTest()
{
    uint test_it_cnt     = 1;
    auto test_failure    = RET_SUCCESS;
    auto test_it_failure = RET_SUCCESS;
    uint run_duration    = 0;

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
            test_it_failure |= CheckGTPowerGood();
            SendGTCfg();
            EnableTXPrbs();
        }
        else if (test_it.cmd == TX_RX_RST)
        {
            test_it_failure |= ResetGT();
        }
        else if (test_it.cmd == INSERT_ERROR )
        {
            InsertError(test_it.insert_error_lane_idx);
        }
        else if (test_it.cmd == RUN )
        {
            // nothing to do
        }

        auto duration_divider = test_it.duration / 10;
        run_duration = run_duration + test_it.duration;

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


        if (test_it.cmd == CONFIG_25GBE)
        {
            test_it_failure |= CheckResetDone();
            EnableRXPrbsCheck();
        }
        else if (test_it.cmd == TX_RX_RST)
        {
            test_it_failure |= CheckResetDone();
        }
        else if (test_it.cmd == CLEAR_STATUS)
        {
            ClearLaneStatus();
            run_duration = 0;
        }
        else if (test_it.cmd == CHECK_STATUS)
        {
            // test_it_failure |= CheckPllLock();
            test_it_failure |= CheckLaneStatus(lane_failure, run_duration);
        }

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

//    DisableTXPrbs();
    DisableRXPrbsCheck();

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

bool GTPrbsTest::StartCU()
{
    return RET_SUCCESS;
}

bool GTPrbsTest::EnableWatchdogClkThrotDetection()
{
    return RET_SUCCESS;
}

bool GTPrbsTest::StopCU()
{
    return RET_SUCCESS;
}

bool GTPrbsTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_SUCCESS;
}

bool GTPrbsTest::GetTestSequence()
{
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        this->m_test_it_lane_results[lane_idx] = {};
    }
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_test_sequence = TestcaseParamCast<std::vector<GTPrbs_Test_Sequence_Parameters_t>>(it->second);
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

bool GTPrbsTest::ParseTestSequenceSettings( std::list<GTPrbsTestItConfig_t> & test_list )
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
        GTPrbsTestItConfig_t test_it_cfg;

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
            parse_it_failure = CheckStringInSet(MODE_TEST_SEQ_MEMBER.name, test_seq_param.mode.value, SUPPORTED_GT_PRBS_SEQUENCE_MODE);
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
            else if (StrMatchNoCase(test_seq_param.mode.value, CHECK_STATUS_STR))
            {
                test_it_cfg.cmd = CHECK_STATUS;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CLEAR_STATUS_STR))
            {
                test_it_cfg.cmd = CLEAR_STATUS;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_25GBE_STR))
            {
                test_it_cfg.cmd = CONFIG_25GBE;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, INSERT_ERROR_LN_0_STR))
            {
                test_it_cfg.cmd = INSERT_ERROR;
                test_it_cfg.insert_error_lane_idx = 0;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, INSERT_ERROR_LN_1_STR))
            {
                test_it_cfg.cmd = INSERT_ERROR;
                test_it_cfg.insert_error_lane_idx = 1;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, INSERT_ERROR_LN_2_STR))
            {
                test_it_cfg.cmd = INSERT_ERROR;
                test_it_cfg.insert_error_lane_idx = 2;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, INSERT_ERROR_LN_3_STR))
            {
                test_it_cfg.cmd = INSERT_ERROR;
                test_it_cfg.insert_error_lane_idx = 3;
            }
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");

        parse_failure |= parse_it_failure;

        if (parse_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_025,{std::to_string(test_cnt)});

            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + " contains erroneous parameters");
            parse_error_cnt++;
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

void GTPrbsTest::PrintLaneParam ( const LaneParam_t & lane_param )
{
    LogMessage(MSG_CMN_021, {"\t- " + GT_PRBS_ERROR_THRESHOLD_MEMBER.name   + " : " + std::to_string(lane_param.prbs_error_threshold)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_SETTINGS_MEMBER.name               + " : " +                lane_param.gt_settings});
    LogMessage(MSG_CMN_021, {"\t- " + GT_LOOPBACK_MEMBER.name               + " : " +                lane_param.gt_lpbk});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_DIFFCTRL_MEMBER.name            + " : " + std::to_string(lane_param.gt_tx_diffctrl)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_MAIN_CURSOR_MEMBER.name         + " : " + std::to_string(lane_param.gt_tx_main_cursor)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_PRE_EMPH_MEMBER.name            + " : " + std::to_string(lane_param.gt_tx_pre_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POST_EMPH_MEMBER.name           + " : " + std::to_string(lane_param.gt_tx_post_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_RX_USE_LPM_MEMBER.name             + " : " +      BoolToStr(lane_param.gt_rx_use_lpm)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POLARITY_MEMBER.name            + " : " +                lane_param.tx_polarity});
}

} // namespace
