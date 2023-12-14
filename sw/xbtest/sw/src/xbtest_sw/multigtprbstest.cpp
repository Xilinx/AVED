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
#include <iomanip>

#include "multigtprbstest.h"

namespace xbtest
{

MultiGTPrbsTest::MultiGTPrbsTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, Multi_GT_t multi_gt_type,
    ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
    const Gt_Parameters_t & test_parameters, const uint & cu_idx, const uint & gt_index
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    this->m_gt_index = gt_index;
    this->m_multi_gt_type = multi_gt_type;

    switch (this->m_multi_gt_type) {
        case GTM: {
            this->m_log_header          = LOG_HEADER_GTMPRBS;
            this->m_queue_testcase      = GTM_PRBS_MEMBER.name;
            this->m_queue_thread        = "GTM_PRBS[" + std::to_string(gt_index) + "]";
            this->m_multi_gt_name       = "GTM";
            this->m_rate.data_size      = GTM_DATA_SIZE_BITS;
            this->m_rate.data_rate      = RATE_56GBE;
            this->m_rate.precision      = RATE_56GBE_PRECISION;
            this->m_rate.low_threshold  = RATE_56GBE_LOW_THRESHOLD;
            this->m_rate.high_threshold = RATE_56GBE_HIGH_THRESHOLD;
            break;
        }
        case GTYP: {
            this->m_log_header          = LOG_HEADER_GTYPPRBS;
            this->m_queue_testcase      = GTYP_PRBS_MEMBER.name;
            this->m_queue_thread        = "GTYP_PRBS[" + std::to_string(gt_index) + "]";
            this->m_multi_gt_name       = "GTYP";
            this->m_rate.data_size      = GTYP_DATA_SIZE_BITS;
            this->m_rate.data_rate      = RATE_32GBS;
            this->m_rate.precision      = RATE_32GBS_PRECISION;
            this->m_rate.low_threshold  = RATE_32GBS_LOW_THRESHOLD;
            this->m_rate.high_threshold = RATE_32GBS_HIGH_THRESHOLD;
            break;
        }
        default: {
            this->m_log_header          = LOG_HEADER_GTFPRBS;
            this->m_queue_testcase      = GTF_PRBS_MEMBER.name;
            this->m_queue_thread        = "GTF_PRBS[" + std::to_string(gt_index) + "]";
            this->m_multi_gt_name       = "GTF";
            this->m_rate.data_size      = GTF_DATA_SIZE_BITS;
            this->m_rate.data_rate      = RATE_10GBE;
            this->m_rate.precision      = RATE_10GBE_PRECISION;
            this->m_rate.low_threshold  = RATE_10GBE_LOW_THRESHOLD;
            this->m_rate.high_threshold = RATE_10GBE_HIGH_THRESHOLD;
            break;
        }
    }

    this->m_log_header.replace(this->m_log_header.find(REP), REP.length(), std::to_string(gt_index));

    // trim last space to align delimiter "::" in log
    if (gt_index >= 10)
    {
        this->m_log_header.erase(this->m_log_header.size()-1);
    }

    this->m_devicemgt         = device_mgt;
    this->m_test_parameters   = test_parameters;
    this->m_cu_idx            = cu_idx;
}

MultiGTPrbsTest::~MultiGTPrbsTest() = default;

bool MultiGTPrbsTest::PreSetup()
{
    auto global_settings_failure = RET_SUCCESS;
    this->m_state = TestState::TS_PRE_SETUP;

        LogMessage(MSG_CMN_004);

    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }

    switch (this->m_multi_gt_type) {
        case GTM: {
            this->m_gt_num_lane       = this->m_device->GetGTMNumLanes( this->m_cu_idx , this->m_gt_index);
            break;
        }
        case GTYP: {
            this->m_gt_num_lane       = this->m_device->GetGTYPNumLanes( this->m_cu_idx , this->m_gt_index);
            break;
        }
        default: {
            this->m_gt_num_lane       = this->m_device->GetGTFNumLanes( this->m_cu_idx , this->m_gt_index);
            break;
        }
    }

    if (this->m_gt_num_lane == 1)
    {
        this->m_available_lane = {0};
    }
    else
    {
        this->m_available_lane = {0,1,2,3};
    }

    this->m_available_lane = {0,1,2,3};

    for (const auto & lane_idx : this->m_available_lane)
    {
        this->m_TC_Cfg.lane_param[lane_idx] = RSTC_LANEPARAM;
    }

    global_settings_failure |= GetTestSequence();

    this->m_buffer_size = MULTI_GT_BUFFER_SIZE;

    std::vector<std::string> first_line;
    first_line.emplace_back("Global time (s)");
    first_line.emplace_back("Rx[0] Test result");
    first_line.emplace_back("Rx[0] Link Speed (Gbps)");
    first_line.emplace_back("Rx[0] Bit Cnt");
    first_line.emplace_back("Rx[0] Bit Error Cnt");
    first_line.emplace_back("Rx[0] Acc Bit Cnt");
    first_line.emplace_back("Rx[0] Acc Bit Error Cnt");
    first_line.emplace_back("Rx[0] BER");

    first_line.emplace_back("Rx[1] Test result");
    first_line.emplace_back("Rx[1] Link Speed (Gbps)");
    first_line.emplace_back("Rx[1] Bit Cnt");
    first_line.emplace_back("Rx[1] Bit Error Cnt");
    first_line.emplace_back("Rx[1] Acc Bit Cnt");
    first_line.emplace_back("Rx[1] Acc Bit Error Cnt");
    first_line.emplace_back("Rx[1] BER");

    first_line.emplace_back("Rx[2] Test result");
    first_line.emplace_back("Rx[2] Link Speed (Gbps)");
    first_line.emplace_back("Rx[2] Bit Cnt");
    first_line.emplace_back("Rx[2] Bit Error Cnt");
    first_line.emplace_back("Rx[2] Acc Bit Cnt");
    first_line.emplace_back("Rx[2] Acc Bit Error Cnt");
    first_line.emplace_back("Rx[2] BER");

    first_line.emplace_back("Rx[3] Test result");
    first_line.emplace_back("Rx[3] Link Speed (Gbps)");
    first_line.emplace_back("Rx[3] Bit Cnt");
    first_line.emplace_back("Rx[3] Bit Error Cnt");
    first_line.emplace_back("Rx[3] Acc Bit Cnt");
    first_line.emplace_back("Rx[3] Acc Bit Error Cnt");
    first_line.emplace_back("Rx[3] BER");


    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {

        switch (this->m_multi_gt_type) {
            case GTM: {
                this->m_outputfile_name = "gtmprbs";
                break;
            }
            case GTYP: {
                this->m_outputfile_name = "gtypprbs";
                break;
            }
            default: {
                this->m_outputfile_name = "gtfprbs";
                break;
            }
        }
        this->m_use_outputfile  = true;

        global_settings_failure |= OpenOutputFile(this->m_outputfile_name + "_gt_" + std::to_string(this->m_gt_index) + ".csv", this->m_outputfile);

        this->m_RT_outputfile_name = this->m_outputfile_name + "_gt_" + std::to_string(this->m_gt_index) + ".csv";
        global_settings_failure |= OpenRTOutputFile(this->m_RT_outputfile_name, this->m_RT_outputfile);
        if (global_settings_failure == RET_SUCCESS)
        {
            this->m_outputfile << StrVectToStr(first_line, ",") << "\n";
            this->m_outputfile.flush();

            this->m_RT_outputfile_head = StrVectToStr(first_line, ",");
            this->m_RT_outputfile.close();
        }
    }

    // lane enable
    bool lane_enable = false;
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamBool(LANE_ENABLE_MEMBER[lane_idx], this->m_test_parameters.global_config, this->m_TC_Cfg.lane_param[lane_idx].enable, true);
        if (this->m_TC_Cfg.lane_param[lane_idx].enable)
        {
            this->m_enable_lane.push_back(lane_idx);
            lane_enable = true;
        }
    }
    // check if at least 1 lane has been enabled
    if (!lane_enable)
    {
        LogMessage(MSG_ETH_085);
        global_settings_failure |= RET_FAILURE;
        this->m_TC_Cfg.global_param.enable = false;
    }

    // Get lane configurations from JSON. For each parameter:
    //      1) Get this->m_TC_Cfg.global_param, the configuration overwrite for all lanes of all GTs.
    //          Note this->m_TC_Cfg.global_param is then set as default/nominal configuration to the lane configuration overwrite.
    //      2) Get this->m_TC_Cfg.lane_param[lane_idx], lane_idx in this->m_available_lane, the configuration overwrite for lane "lane_idx"

    // default configuration of GT; all lanes configured identically
    global_settings_failure |= GetJsonParamStr(GT_SETTINGS_MEMBER, this->m_test_parameters.global_config, SUPPORTED_GT_SETTINGS, this->m_TC_Cfg.global_param.gt_settings, GT_SETTINGS_MODULE);
    for (const auto & lane_idx : this->m_available_lane)
    {
        this->m_TC_Cfg.lane_param[lane_idx].gt_settings = this->m_TC_Cfg.global_param.gt_settings;
    }

    std::string tx_polarity;
    uint32_t    tx_main_cursor;
    uint32_t    tx_diffctrl;
    uint32_t    tx_pre_emphasis;
    uint32_t    tx_post_emphasis;
    std::string rx_polarity;
    bool        rx_equaliser_lpm;

    auto gt_settings = this->m_xbtest_sw_config->GetGTSettings(this->m_gt_index);

    if (this->m_TC_Cfg.global_param.gt_settings == GT_SETTINGS_CABLE)
    {
        tx_main_cursor   = gt_settings.transceivers.cable.tx_main_cursor;
        tx_polarity      = gt_settings.transceivers.cable.tx_polarity;
        tx_diffctrl      = gt_settings.transceivers.cable.tx_differential_swing_control;
        tx_pre_emphasis  = gt_settings.transceivers.cable.tx_pre_emphasis;
        tx_post_emphasis = gt_settings.transceivers.cable.tx_post_emphasis;
        rx_polarity      = gt_settings.transceivers.cable.rx_polarity;
        rx_equaliser_lpm = RX_EQUALISER_DFM_EN;

        if (StrMatchNoCase(gt_settings.transceivers.cable.rx_equalizer, RX_EQUALISER_LPM))
        {
            rx_equaliser_lpm = RX_EQUALISER_LPM_EN;
        }
    }
    else
    {
        tx_main_cursor   = gt_settings.transceivers.module.tx_main_cursor;
        tx_polarity      = gt_settings.transceivers.module.tx_polarity;
        tx_diffctrl      = gt_settings.transceivers.module.tx_differential_swing_control;
        tx_pre_emphasis  = gt_settings.transceivers.module.tx_pre_emphasis;
        tx_post_emphasis = gt_settings.transceivers.module.tx_post_emphasis;
        rx_polarity      = gt_settings.transceivers.module.rx_polarity;
        rx_equaliser_lpm = RX_EQUALISER_DFM_EN;

        if (StrMatchNoCase(gt_settings.transceivers.module.rx_equalizer, RX_EQUALISER_LPM))
        {
            rx_equaliser_lpm = RX_EQUALISER_LPM_EN;
        }
    }

    // overwrite each GT settings globally then allow a per lane overwrite
    global_settings_failure |= GetJsonParamNum<double>(GT_BER_THRESHOLD_MEMBER, this->m_test_parameters.global_config, MIN_BER_THRESHOLD, NOM_BER_THRESHOLD, MAX_BER_THRESHOLD, this->m_TC_Cfg.global_param.ber_threshold);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamNum<double>(GT_BER_THRESHOLD_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_BER_THRESHOLD, this->m_TC_Cfg.global_param.ber_threshold, MAX_BER_THRESHOLD, this->m_TC_Cfg.lane_param[lane_idx].ber_threshold);
    }

    // gt_loopback
    global_settings_failure |= GetJsonParamStr(GT_LOOPBACK_MEMBER, this->m_test_parameters.global_config, SUPPORTED_GT_LPBK_TYPE, this->m_TC_Cfg.global_param.gt_lpbk,  GT_LPBK_DISABLE);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamStr(GT_LOOPBACK_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, SUPPORTED_GT_LPBK_TYPE, this->m_TC_Cfg.lane_param[lane_idx].gt_lpbk, this->m_TC_Cfg.global_param.gt_lpbk);
    }

    // disable_ref_prbs
    global_settings_failure |= GetJsonParamBool(GT_DISABLE_REF_PRBS_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.global_param.disable_ref_prbs,false);

    // overwrite gt_tx_diffctrl
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_DIFFCTRL, tx_diffctrl, MAX_GT_TX_DIFFCTRL, this->m_TC_Cfg.global_param.gt_tx_diffctrl);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_DIFFCTRL, this->m_TC_Cfg.global_param.gt_tx_diffctrl, MAX_GT_TX_DIFFCTRL, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl);
    }
    // overwrite gt_tx_main_cursor
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_MAIN_CURSOR_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_MAIN_CURSOR, tx_main_cursor, MAX_GT_TX_MAIN_CURSOR, this->m_TC_Cfg.global_param.gt_tx_main_cursor);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_MAIN_CURSOR_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_MAIN_CURSOR, this->m_TC_Cfg.global_param.gt_tx_main_cursor, MAX_GT_TX_MAIN_CURSOR, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_main_cursor);
    }

    // overwrite gt_tx_pre_emph
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_PRE_EMPH_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_PRE_EMPH, tx_pre_emphasis, MAX_GT_TX_PRE_EMPH, this->m_TC_Cfg.global_param.gt_tx_pre_emph);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_PRE_EMPH_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_PRE_EMPH, this->m_TC_Cfg.global_param.gt_tx_pre_emph, MAX_GT_TX_PRE_EMPH, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph);
    }
    // overwrite gt_tx_post_emph
    global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_POST_EMPH_MEMBER, this->m_test_parameters.global_config, MIN_GT_TX_POST_EMPH, tx_post_emphasis, MAX_GT_TX_POST_EMPH, this->m_TC_Cfg.global_param.gt_tx_post_emph);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamNum<uint32_t>(GT_TX_POST_EMPH_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, MIN_GT_TX_POST_EMPH, this->m_TC_Cfg.global_param.gt_tx_post_emph, MAX_GT_TX_POST_EMPH, this->m_TC_Cfg.lane_param[lane_idx].gt_tx_post_emph);
    }
    // overwrite gt_rx_use_lpm
    global_settings_failure |= GetJsonParamBool(GT_RX_USE_LPM_MEMBER, this->m_test_parameters.global_config, this->m_TC_Cfg.global_param.gt_rx_use_lpm,  rx_equaliser_lpm);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamBool(GT_RX_USE_LPM_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, this->m_TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm, this->m_TC_Cfg.global_param.gt_rx_use_lpm);
    }
    // overwrite tx_polarity
    global_settings_failure |= GetJsonParamStr(GT_TX_POLARITY_MEMBER, this->m_test_parameters.global_config, SUPPORTED_TX_POLARITY_TYPE, this->m_TC_Cfg.global_param.tx_polarity,  tx_polarity);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamStr(GT_TX_POLARITY_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, SUPPORTED_TX_POLARITY_TYPE, this->m_TC_Cfg.lane_param[lane_idx].tx_polarity, this->m_TC_Cfg.global_param.tx_polarity);
    }
    // overwrite rx_polarity
    global_settings_failure |= GetJsonParamStr(GT_RX_POLARITY_MEMBER, this->m_test_parameters.global_config, SUPPORTED_RX_POLARITY_TYPE, this->m_TC_Cfg.global_param.rx_polarity,  rx_polarity);
    for (const auto & lane_idx : this->m_available_lane)
    {
        global_settings_failure |= GetJsonParamStr(GT_RX_POLARITY_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters.lane_config[lane_idx].param, SUPPORTED_RX_POLARITY_TYPE, this->m_TC_Cfg.lane_param[lane_idx].rx_polarity, this->m_TC_Cfg.global_param.rx_polarity);
    }

    LogMessage(MSG_CMN_021, {"Test configuration:"});
    LogMessage(MSG_CMN_021, {"- Global configuration parameters"});
    LogMessage(MSG_CMN_021, {"\t- " + GT_DISABLE_REF_PRBS_MEMBER.name       + " : " +      BoolToStr(this->m_TC_Cfg.global_param.disable_ref_prbs)});
    PrintLaneParam(this->m_TC_Cfg.global_param);

    // Display here all lane parameters including hidden in DEBUG
    for (const auto & lane_idx : this->m_available_lane)
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

void MultiGTPrbsTest::Run()
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

void MultiGTPrbsTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void MultiGTPrbsTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

bool MultiGTPrbsTest::WriteMultiGtPrbsCu( const uint & address, const uint & value)
{
    switch (this->m_multi_gt_type) {
        case GTM: {
            return this->m_device->WriteGtmPrbsCu(this->m_cu_idx, this->m_gt_index, address, value);
            break;
        }
        case GTYP: {
            return this->m_device->WriteGtypPrbsCu(this->m_cu_idx, this->m_gt_index, address, value);
            break;
        }
        default: {
            return this->m_device->WriteGtfPrbsCu(this->m_cu_idx, this->m_gt_index, address, value);
            break;
        }
    }
}

bool MultiGTPrbsTest::ReadMultiGtPrbsCu( const uint & address, uint & read_data )
{
    switch (this->m_multi_gt_type) {
        case GTM: {
            return this->m_device->ReadGtmPrbsCu(this->m_cu_idx, this->m_gt_index, address, read_data);
            break;
        }
        case GTYP: {
            return this->m_device->ReadGtypPrbsCu(this->m_cu_idx, this->m_gt_index, address, read_data);
            break;
        }
        default: {
            return this->m_device->ReadGtfPrbsCu(this->m_cu_idx, this->m_gt_index, address, read_data);
            break;
        }
    }
}

bool MultiGTPrbsTest::ReadMultiGtPrbsStatus(uint32_t * read_buffer, const uint64_t & size )
{
    switch (this->m_multi_gt_type) {
        case GTM: {
            return this->m_device->ReadGtmPrbsCuStatus(this->m_cu_idx, this->m_gt_index, read_buffer, size);
            break;
        }
        case GTYP: {
            return this->m_device->ReadGtypPrbsCuStatus(this->m_cu_idx, this->m_gt_index, read_buffer, size);
            break;
        }
        default: {
            return this->m_device->ReadGtfPrbsCuStatus(this->m_cu_idx, this->m_gt_index, read_buffer, size);
            break;
        }
    }
}

bool MultiGTPrbsTest::ResetGT()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT All"});
    this->m_device->LockGTRstMtx(this->m_gt_index);
    CheckAllResetAndSyncDoneHigh();
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | RESET_GT_ALL);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}

bool MultiGTPrbsTest::ResetGTTx()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT all Tx PLL & datapath"});
    this->m_device->LockGTRstMtx(this->m_gt_index);
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | RESET_GT_TX_PLL_DATAPATH);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}
bool MultiGTPrbsTest::ResetGTTxDatapath()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT all Tx datapath"});
    this->m_device->LockGTRstMtx(this->m_gt_index);
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | RESET_GT_TX_DATAPATH);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}

bool MultiGTPrbsTest::ResetGTTxDatapathLane(uint lane_idx)
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT Tx datapath lane[" + std::to_string(lane_idx) + "]" });
    this->m_device->LockGTRstMtx(this->m_gt_index);
    ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | LANE_GT_RESET_TX_DATAPATH);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}

bool MultiGTPrbsTest::ResetGTRx()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT all Rx PLL & datapath"});
    this->m_device->LockGTRstMtx(this->m_gt_index);
    ReadMultiGtPrbsCu(CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | RESET_GT_RX_PLL_DATAPATH);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}
bool MultiGTPrbsTest::ResetGTRxDatapath()
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT all Rx datapath"});
    this->m_device->LockGTRstMtx(this->m_gt_index);
    ReadMultiGtPrbsCu(CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | RESET_GT_RX_DATAPATH);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}

bool MultiGTPrbsTest::ResetGTRxDatapathLane(uint lane_idx)
{
    auto ret = RET_SUCCESS;
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Reset GT Rx datapath lane[" + std::to_string(lane_idx) + "]" });
    this->m_device->LockGTRstMtx(this->m_gt_index);
    ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | LANE_GT_RESET_RX_DATAPATH);
    WaitSecTick(1);
    this->m_device->UnLockGTRstMtx(this->m_gt_index);

    return ret;
}

bool MultiGTPrbsTest::CheckGTPowerGood( std::map<uint, bool> & lane_failure )
{
    uint read_data;
    //check power good: live is high and no rising or falling edges
    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
    auto power_check_pass = RET_SUCCESS;

    if ((read_data & GT_POWER_GOOD_LIVE) != GT_POWER_GOOD_LIVE)
    {
        LogMessage(MSG_ETH_064);
        power_check_pass = RET_FAILURE;
    }

    if ((read_data & GT_POWER_GOOD_RISING) == GT_POWER_GOOD_RISING)
    {
        LogMessage(MSG_ETH_072, {"rising"});
        power_check_pass = RET_FAILURE;
    }

    if ((read_data & GT_POWER_GOOD_FALLING) == GT_POWER_GOOD_FALLING)
    {
        LogMessage(MSG_ETH_072, {"falling"});
        power_check_pass = RET_FAILURE;
    }

    if (power_check_pass == RET_FAILURE)
    {
        if (this->m_multi_gt_type == GTF)
        {
            // check every single lane to see which one is failing
            for (const auto & lane_idx : this->m_enable_lane)
            {
                ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
                if ((read_data & GTWIZ_LN_POWER_GOOD_LIVE) != GTWIZ_LN_POWER_GOOD_LIVE)
                {
                    LogMessage(MSG_ETH_082, {std::to_string(lane_idx)});
                    power_check_pass = RET_FAILURE;
                    lane_failure[lane_idx] = RET_FAILURE;
                }

                if ((read_data & GTWIZ_LN_POWER_GOOD_RISING) == GTWIZ_LN_POWER_GOOD_RISING)
                {
                    LogMessage(MSG_ETH_083, {std::to_string(lane_idx), "rising"});
                    power_check_pass = RET_FAILURE;
                    lane_failure[lane_idx] = RET_FAILURE;
                }

                if ((read_data & GTWIZ_LN_POWER_GOOD_FALLING) == GTWIZ_LN_POWER_GOOD_FALLING)
                {
                    LogMessage(MSG_ETH_083, {std::to_string(lane_idx), "falling"});
                    power_check_pass = RET_FAILURE;
                    lane_failure[lane_idx] = RET_FAILURE;
                }
            }
        }
        else
        {
            SetAllLaneFailure(lane_failure);
        }
    }
    if (power_check_pass == RET_SUCCESS)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"OK: Power good stay high"});
    }
    return RET_SUCCESS;
}

bool MultiGTPrbsTest::CheckAllResetAndSyncDoneHigh()
{
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
    // Expect live status high
    if ((read_data & TX_RST_DONE_LIVE) != TX_RST_DONE_LIVE)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx GT reset done is not high"});
        ret = RET_FAILURE;
    }

    if ((read_data & RX_RST_DONE_LIVE) != RX_RST_DONE_LIVE)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx GT reset done is not high"});
        ret = RET_FAILURE;
    }

    if (ret == RET_FAILURE)
    {
        // check which lane did fail
        if ( this->m_multi_gt_type == GTF )
        {
            for (const auto & lane_idx : this->m_enable_lane)
            {
                ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
                // Expect live status high
                if ((read_data & GTWIZ_TX_RST_DONE_LIVE) != GTWIZ_TX_RST_DONE_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Wizard reset done is not high"});
                    ret = RET_FAILURE;
                }
                if ((read_data & GTWIZ_TX_BUFFERBYPASS_LIVE) != GTWIZ_TX_BUFFERBYPASS_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Wizard bufferbypass reset done is not high"});
                    ret = RET_FAILURE;
                }
                if ((read_data & GTWIZ_CH_TX_RST_DONE_LIVE) != GTWIZ_CH_TX_RST_DONE_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Channel reset done is not high"});
                    ret = RET_FAILURE;
                }
                if ((read_data & GTWIZ_CH_TX_SYNC_DONE_LIVE) != GTWIZ_CH_TX_SYNC_DONE_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Channel sync done is not high"});
                    ret = RET_FAILURE;
                }

                if ((read_data & GTWIZ_RX_RST_DONE_LIVE) != GTWIZ_RX_RST_DONE_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Wizard reset done is not high"});
                    ret = RET_FAILURE;
                }
                if ((read_data & GTWIZ_RX_BUFFERBYPASS_LIVE) != GTWIZ_RX_BUFFERBYPASS_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Wizard bufferbypass reset done is not high"});
                    ret = RET_FAILURE;
                }
                if ((read_data & GTWIZ_CH_RX_RST_DONE_LIVE) != GTWIZ_CH_RX_RST_DONE_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Channel reset done is not high"});
                    ret = RET_FAILURE;
                }
                if ((read_data & GTWIZ_CH_RX_SYNC_DONE_LIVE) != GTWIZ_CH_RX_SYNC_DONE_LIVE)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Channel sync done is not high"});
                    ret = RET_FAILURE;
                }
            }
        }
    }
    if (ret == RET_SUCCESS)
    {
        if ( this->m_multi_gt_type == GTF )
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"OK: all channels Tx/Rx wizard & bufferbypass & channel reset + sync done are high"});
        }
        else
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"OK: Tx/Rx reset done are high"});
        }
    }

    return ret;
}


bool MultiGTPrbsTest::CheckResetRequest()
{
    auto ret = RET_SUCCESS;
    // Expect falling status high: a falling edge has been detected
    // this doens't occur all the time.  The power up/ reset FSM could sometime not come into lock all the time
    //  so the reset will not be high and falling edge can't be detected ...
    //      so it's why it's a debug message and not fully reported error
    CheckTxResetRequest();
    CheckRxResetRequest();

    return ret;
}

bool MultiGTPrbsTest::CheckResetOver(std::map<uint, bool> & lane_failure)
{
    // check that both TX and RX are over otherwise request a new full reset
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    auto rst_watchdog = RST_WATCHDOG_INIT;
    auto rst_check_fail = true;

    std::map<uint, bool>  reset_over;
    while (rst_watchdog > 0)
    {
        rst_check_fail = false;

        ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
        if ( ((read_data & TX_RST_DONE_LIVE) != TX_RST_DONE_LIVE) || ((read_data & RX_RST_DONE_LIVE) != RX_RST_DONE_LIVE) )
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx or Rx GT reset not done"});
            rst_check_fail = true;
            if ( this->m_multi_gt_type != GTF )
            {
                for (const auto & lane_idx : this->m_enable_lane)
                {
                    reset_over[lane_idx] = RET_FAILURE;
                }
            }
        }

        // for GTF, check which lane did fail and other status as debug (bufferbypass ...)
        if ( this->m_multi_gt_type == GTF )
        {
            for (const auto & lane_idx : this->m_enable_lane)
            {
                reset_over[lane_idx] = RET_SUCCESS;
                // use all live status to check that reset is over, they must all be high
                ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
                if ((read_data & GTWIZ_TX_RST_DONE_LIVE) != GTWIZ_TX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Tx GTF Wizard reset"});
                }
                if ((read_data & GTWIZ_TX_BUFFERBYPASS_LIVE) != GTWIZ_TX_BUFFERBYPASS_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Tx GTF Wizard bufferbypass reset"});
                }
                if ((read_data & GTWIZ_CH_TX_RST_DONE_LIVE) != GTWIZ_CH_TX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Tx GTF Channel reset"});
                }
                if ((read_data & GTWIZ_CH_TX_SYNC_DONE_LIVE) != GTWIZ_CH_TX_SYNC_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Tx GTF Channel sync"});
                }
                // RX
                if ((read_data & GTWIZ_RX_RST_DONE_LIVE) != GTWIZ_RX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Rx GTF Wizard reset"});
                }
                if ((read_data & GTWIZ_RX_BUFFERBYPASS_LIVE) != GTWIZ_RX_BUFFERBYPASS_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Rx GTF Wizard bufferbypass reset"});
                }
                if ((read_data & GTWIZ_CH_RX_RST_DONE_LIVE) != GTWIZ_CH_RX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Rx GTF Channel reset"});
                }
                if ((read_data & GTWIZ_CH_RX_SYNC_DONE_LIVE) != GTWIZ_CH_RX_SYNC_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_ETH_080, {std::to_string(lane_idx), "Rx GTF Channel sync"});
                }
            }
        }

        // all lanes reports their various done high, quit the watchdog loop
        if (!rst_check_fail)
        {
            break;
        }

        rst_watchdog--;

        if ((rst_watchdog > 0) && (!(this->m_abort)))
        {
            LogMessage(MSG_ETH_079, {"Tx or RX"});
            // request a full reset of the GT if TX or RX is failing
            ClearStatus();
            ResetGT();
            WaitSecTick(2);
            CheckResetRequest();
        }
    }
    if (rst_check_fail)
    {
        // save which lane did fail
        for (const auto & lane_idx : this->m_enable_lane)
        {
            if (reset_over[lane_idx] == RET_FAILURE)
            {
                lane_failure[lane_idx] = RET_FAILURE;
            }
        }
        LogMessage(MSG_ETH_069, {"Tx or Rx", std::to_string(RST_WATCHDOG_INIT)});
        ret = RET_FAILURE;
        Abort();
    }
    else
    {
        LogMessage(MSG_ETH_078, {"Tx and Rx", std::to_string(RST_WATCHDOG_INIT - rst_watchdog + 1)});
    }

    return ret;

}

bool MultiGTPrbsTest::CheckGTFTxResetRequestLane(uint lane_idx)
{
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    // Check that a falling edge have been detected
    ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
    //LogMessage(MSG_DEBUG_TESTCASE, {"\t- Lane[" + std::to_string(lane_idx) + "] read data: 0x" + NumToStrHex<uint>(read_data)});
    if ((read_data & GTWIZ_TX_RST_DONE_FALLING) != GTWIZ_TX_RST_DONE_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Wizard reset done is not going low"});
    }
    if ((read_data & GTWIZ_TX_BUFFERBYPASS_FALLING) != GTWIZ_TX_BUFFERBYPASS_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Wizard bufferbypass reset done is not going low"});
    }
    if ((read_data & GTWIZ_CH_TX_RST_DONE_FALLING) != GTWIZ_CH_TX_RST_DONE_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Channel reset done is not going low"});
    }
    if ((read_data & GTWIZ_CH_TX_SYNC_DONE_FALLING) != GTWIZ_CH_TX_SYNC_DONE_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx Lane[" + std::to_string(lane_idx) + "] GTF Channel sync done is not going low"});
    }

    if (ret == RET_SUCCESS)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"OK: Tx Lane[" + std::to_string(lane_idx) + "]: wizard, bufferbypass & channel reset + sync done going low"});
    }

    return ret;
}

bool MultiGTPrbsTest::CheckTxResetRequest()
{
    // Expect falling status high: a falling edge has been detected
    // this doens't occur all the time.  The power up/ reset FSM could sometime not come into lock all the time
    //  so the reset will not be high and falling edge can't be detected ...
    //      so it's why it's a debug message and not fully reported error
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
    if ((read_data & TX_RST_DONE_FALLING) != TX_RST_DONE_FALLING)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx GT reset done is going low"});
    }

    // for GTF, also check other sub status: bufferbypass etc
    if ( this->m_multi_gt_type == GTF )
    {
        for (const auto & lane_idx : this->m_enable_lane)
        {
            CheckGTFTxResetRequestLane(lane_idx);
        }
    }

    return ret;
}

bool MultiGTPrbsTest::CheckTxResetOver( std::map<uint, bool> & lane_failure)
{
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    auto rst_watchdog = RST_WATCHDOG_INIT;
    auto rst_check_fail = true;

    std::map<uint, bool>  reset_over;
    while (rst_watchdog > 0)
    {
        rst_check_fail = false;
        ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
        if ( (read_data & TX_RST_DONE_LIVE) != TX_RST_DONE_LIVE)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"KO: Tx GT reset not done"});
            rst_check_fail = true;
            if ( this->m_multi_gt_type != GTF )
            {
                for (const auto & lane_idx : this->m_enable_lane)
                {
                    reset_over[lane_idx] = RET_FAILURE;
                }
            }
        }

        // for GTF, check which lane did fail and other status as debug (bufferbypass ...)
        if ( this->m_multi_gt_type == GTF )
        {
            for (const auto & lane_idx : this->m_enable_lane)
            {
                reset_over[lane_idx] = RET_SUCCESS;
                // use all live status to check that reset is over, they must all be high
                ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
                if ((read_data & GTWIZ_TX_RST_DONE_LIVE) != GTWIZ_TX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Tx GTF Wizard reset done not high"});
                }
                if ((read_data & GTWIZ_TX_BUFFERBYPASS_LIVE) != GTWIZ_TX_BUFFERBYPASS_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Tx GTF Wizard bufferbypass reset done not high"});
                }
                if ((read_data & GTWIZ_CH_TX_RST_DONE_LIVE) != GTWIZ_CH_TX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Tx GTF Channel reset done not high"});
                }
                if ((read_data & GTWIZ_CH_TX_SYNC_DONE_LIVE) != GTWIZ_CH_TX_SYNC_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Tx GTF Channel sync done not high"});
                }
            }
        }
        // all lanes reports reset high, quit the watchdog loop
        if (!rst_check_fail)
        {
            break;
        }

        rst_watchdog--;

        if ((rst_watchdog > 0) && (!(this->m_abort)))
        {
            LogMessage(MSG_ETH_079, {"Tx"});
            // request a full reset of the GT if TX is failing
            ClearStatus();
            ResetGT();
            WaitSecTick(2);
            CheckTxResetRequest();
        }
    }
    if (rst_check_fail)
    {
        // save which lane did fail
        for (const auto & lane_idx : this->m_enable_lane)
        {
            if (reset_over[lane_idx] == RET_FAILURE)
            {
                lane_failure[lane_idx] = RET_FAILURE;
            }
        }
        LogMessage(MSG_ETH_069, {"Tx", std::to_string(RST_WATCHDOG_INIT)});
        ret = RET_FAILURE;
        Abort();
    }
    else
    {
        LogMessage(MSG_ETH_078, {"Tx", std::to_string(RST_WATCHDOG_INIT - rst_watchdog + 1)});
    }

    return ret;
}

bool MultiGTPrbsTest::CheckTxResetStable( std::map<uint, bool> & lane_failure)
{
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
    if ( (read_data & TX_RST_DONE_LIVE) != TX_RST_DONE_LIVE)
    {
        LogMessage(MSG_ETH_084, {"Tx", "GT reset done", "low"});
        ret = RET_FAILURE;
        // there is no lane specific indication for GTM/GTYP
        if ( this->m_multi_gt_type != GTF )
        {
            SetAllLaneFailure(lane_failure);
        }
    }

    // Expect not falling
    if ((read_data & TX_RST_DONE_FALLING) == TX_RST_DONE_FALLING)
    {
        LogMessage(MSG_ETH_084, {"Tx", "GT reset done", "falling"});
        ret = RET_FAILURE;
        // there is no lane specific indication for GTM/GTYP
        if ( this->m_multi_gt_type != GTF )
        {
            SetAllLaneFailure(lane_failure);
        }
    }
    // Expect not rising
    if ((read_data & TX_RST_DONE_RISING) == TX_RST_DONE_RISING)
    {
        LogMessage(MSG_ETH_084, {"Tx", "GT reset done", "rising"});
        ret = RET_FAILURE;
        // there is no lane specific indication for GTM/GTYP
        if ( this->m_multi_gt_type != GTF )
        {
            SetAllLaneFailure(lane_failure);
        }
    }

    // for GTF, check which lane did fail and other status as debug (bufferbypass ...)
    if ( this->m_multi_gt_type == GTF )
    {
        for (const auto & lane_idx : this->m_enable_lane)
        {
            ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
            // Expect live status high
            if ((read_data & GTWIZ_TX_RST_DONE_LIVE) != GTWIZ_TX_RST_DONE_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Wizard reset done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_TX_RST_DONE_FALLING) == GTWIZ_TX_RST_DONE_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Wizard reset done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_TX_RST_DONE_RISING) == GTWIZ_TX_RST_DONE_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Wizard reset done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }

            // Expect live status high
            if ((read_data & GTWIZ_TX_BUFFERBYPASS_LIVE) != GTWIZ_TX_BUFFERBYPASS_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Wizard bufferbypass reset done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_TX_BUFFERBYPASS_FALLING) == GTWIZ_TX_BUFFERBYPASS_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Wizard bufferbypass reset done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_TX_BUFFERBYPASS_RISING) == GTWIZ_TX_BUFFERBYPASS_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Wizard bufferbypass reset done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }

            // Expect live status high
            if ((read_data & GTWIZ_CH_TX_RST_DONE_LIVE) != GTWIZ_CH_TX_RST_DONE_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Channel reset done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_CH_TX_RST_DONE_FALLING) == GTWIZ_CH_TX_RST_DONE_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Channel reset done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_CH_TX_RST_DONE_RISING) == GTWIZ_CH_TX_RST_DONE_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Channel reset done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }

            // Expect live status high
            if ((read_data & GTWIZ_CH_TX_SYNC_DONE_LIVE) != GTWIZ_CH_TX_SYNC_DONE_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Channel sync done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_CH_TX_SYNC_DONE_FALLING) == GTWIZ_CH_TX_SYNC_DONE_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Channel sync done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_CH_TX_SYNC_DONE_RISING) == GTWIZ_CH_TX_SYNC_DONE_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Tx", "GTF Channel sync done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
        }
    }
    if (ret == RET_SUCCESS)
    {
        LogMessage(MSG_ETH_081, {"Tx"});
    }
    return ret;
}

bool MultiGTPrbsTest::CheckRxResetRequestLane( uint lane_idx)
{
    // use all falling edge to check that reset is triggered
    auto ret = RET_SUCCESS;
    uint read_data = 0;

    ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
    //LogMessage(MSG_DEBUG_TESTCASE, {"\t- Lane[" + std::to_string(lane_idx) + "] read data: 0x" + NumToStrHex<uint>(read_data)});
    if ((read_data & GTWIZ_RX_RST_DONE_FALLING) != GTWIZ_RX_RST_DONE_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Wizard reset done is not going low"});
    }
    if ((read_data & GTWIZ_RX_BUFFERBYPASS_FALLING) != GTWIZ_RX_BUFFERBYPASS_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Wizard bufferbypass reset done is not going low"});
    }
    if ((read_data & GTWIZ_CH_RX_RST_DONE_FALLING) != GTWIZ_CH_RX_RST_DONE_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Channel reset done is not going low"});
    }
    if ((read_data & GTWIZ_CH_RX_SYNC_DONE_FALLING) != GTWIZ_CH_RX_SYNC_DONE_FALLING)
    {
        ret = RET_FAILURE;
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx Lane[" + std::to_string(lane_idx) + "] GTF Channel sync done is not going low"});
    }

    if (ret == RET_SUCCESS )
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"OK: Rx Lane[" + std::to_string(lane_idx) + "]: wizard, bufferbypass & channel reset + sync done going low"});
    }

    return ret;
}

bool MultiGTPrbsTest::CheckRxResetRequest()
{
    // Expect falling status high: a falling edge has been detected
    // this doens't occur all the time.  The power up/ reset FSM could sometime not come into lock all the time
    //  so the reset will not be high and falling edge can't be detected ...
    //      so it's why it's a debug message and not fully reported error
    auto ret = RET_SUCCESS;
    uint read_data = 0;
    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);

    if ((read_data & RX_RST_DONE_FALLING) != RX_RST_DONE_FALLING)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx GT reset done is not going low"});
    }
    // for GTF, also check other sub status: bufferbypass etc
    if ( this->m_multi_gt_type == GTF )
    {
        for (const auto & lane_idx : this->m_enable_lane)
        {
            CheckRxResetRequestLane(lane_idx);
        }
    }
    return ret;
}

bool MultiGTPrbsTest::CheckRxResetOver( std::map<uint, bool> & lane_failure)
{
    auto ret = RET_SUCCESS;
    uint read_data = 0;
    auto rst_watchdog = RST_WATCHDOG_INIT;

    auto rst_check_fail = false;
    std::map<uint, bool>  reset_over;
    while (rst_watchdog > 0)
    {
        rst_check_fail = false;
        ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
        if ( (read_data & RX_RST_DONE_LIVE) != RX_RST_DONE_LIVE)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"KO: Rx GT reset not done"});
            rst_check_fail = true;
            if ( this->m_multi_gt_type != GTF )
            {
                for (const auto & lane_idx : this->m_enable_lane)
                {
                    reset_over[lane_idx] = RET_FAILURE;
                }
            }
        }

        // for GTF, check which lane did fail and other status as debug (bufferbypass ...)
        if ( this->m_multi_gt_type == GTF )
        {
            for (const auto & lane_idx : this->m_enable_lane)
            {
                reset_over[lane_idx] = RET_SUCCESS;
                // use all live status to check that reset is over, they must all be high
                ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
                if ((read_data & GTWIZ_RX_RST_DONE_LIVE) != GTWIZ_RX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Rx GTF Wizard reset done not high"});
                }
                if ((read_data & GTWIZ_RX_BUFFERBYPASS_LIVE) != GTWIZ_RX_BUFFERBYPASS_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Rx GTF Wizard bufferbypass reset done not high"});
                }
                if ((read_data & GTWIZ_CH_RX_RST_DONE_LIVE) != GTWIZ_CH_RX_RST_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Rx GTF Channel reset done not high"});
                }
                if ((read_data & GTWIZ_CH_RX_SYNC_DONE_LIVE) != GTWIZ_CH_RX_SYNC_DONE_LIVE)
                {
                    rst_check_fail = true;
                    reset_over[lane_idx] = RET_FAILURE;
                    LogMessage(MSG_DEBUG_TESTCASE, {"KO: Lane[" + std::to_string(lane_idx) + "]: Rx GTF Channel sync done not high"});
                }
            }
        }
        // all lanes reports reset high, quit the watchdog loop
        if (!rst_check_fail)
        {
            break;
        }
        rst_watchdog--;

        if ((rst_watchdog > 0) && (!(this->m_abort)))
        {

            LogMessage(MSG_ETH_079, {"Rx"});

            // request datapath reset for the failing lanes
            for (const auto & lane_idx : this->m_enable_lane)
            {
                if (reset_over[lane_idx] == RET_FAILURE)
                {
                    ResetGTRxDatapathLane(lane_idx);
                }
            }
            WaitSecTick(2);

            // check if these reset went OK
            for (const auto & lane_idx : this->m_enable_lane)
            {
                if (reset_over[lane_idx] == RET_FAILURE)
                {
                    CheckRxResetRequestLane(lane_idx);
                }
            }
        }
    }

    if (rst_check_fail)
    {
        // save which lane did fail
        for (const auto & lane_idx : this->m_enable_lane)
        {
            if (reset_over[lane_idx] == RET_FAILURE)
            {
                lane_failure[lane_idx] = RET_FAILURE;
            }
        }
        LogMessage(MSG_ETH_069, {"Rx", std::to_string(RST_WATCHDOG_INIT)});
        ret = RET_FAILURE;
        Abort();
    }
    else
    {
        LogMessage(MSG_ETH_078, {"Rx",std::to_string(RST_WATCHDOG_INIT - rst_watchdog + 1)});
    }

    return ret;

}
bool MultiGTPrbsTest::CheckRxResetStable( std::map<uint, bool> & lane_failure)
{
    auto ret = RET_SUCCESS;
    uint read_data = 0;
    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
    if ( (read_data & RX_RST_DONE_LIVE) != RX_RST_DONE_LIVE)
    {
        LogMessage(MSG_ETH_084, {"Rx", "GT reset done", "low"});
        ret = RET_FAILURE;
        // there is no lane specific indication for GTM/GTYP
        if ( this->m_multi_gt_type != GTF )
        {
            SetAllLaneFailure(lane_failure);
        }
    }
    // Expect not falling
    if ((read_data & RX_RST_DONE_FALLING) == RX_RST_DONE_FALLING)
    {
        LogMessage(MSG_ETH_084, {"Rx", "GT reset done", "falling"});
        ret = RET_FAILURE;
        // there is no lane specific indication for GTM/GTYP
        if ( this->m_multi_gt_type != GTF )
        {
            SetAllLaneFailure(lane_failure);
        }
    }
    // Expect not rising
    if ((read_data & RX_RST_DONE_RISING) == RX_RST_DONE_RISING)
    {
        LogMessage(MSG_ETH_084, {"Rx", "GT reset done", "rising"});
        ret = RET_FAILURE;
        // there is no lane specific indication for GTM/GTYP
        if ( this->m_multi_gt_type != GTF )
        {
            SetAllLaneFailure(lane_failure);
        }
    }

    // for GTF, check which lane did fail and other status as debug (bufferbypass ...)
    if ( this->m_multi_gt_type == GTF )
    {
        for (const auto & lane_idx : this->m_enable_lane)
        {
            ReadMultiGtPrbsCu(RESET_STAT_REG_ADDR[lane_idx], read_data);
            // Expect live status high
            if ((read_data & GTWIZ_RX_RST_DONE_LIVE) != GTWIZ_RX_RST_DONE_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Wizard reset done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_RX_RST_DONE_FALLING) == GTWIZ_RX_RST_DONE_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Wizard reset done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_RX_RST_DONE_RISING) == GTWIZ_RX_RST_DONE_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Wizard reset done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }

            // Expect live status high
            if ((read_data & GTWIZ_RX_BUFFERBYPASS_LIVE) != GTWIZ_RX_BUFFERBYPASS_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Wizard bufferbypass reset done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_RX_BUFFERBYPASS_FALLING) == GTWIZ_RX_BUFFERBYPASS_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Wizard bufferbypass reset done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_RX_BUFFERBYPASS_RISING) == GTWIZ_RX_BUFFERBYPASS_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Wizard bufferbypass reset done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect live status high
            if ((read_data & GTWIZ_CH_RX_RST_DONE_LIVE) != GTWIZ_CH_RX_RST_DONE_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Channel reset done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_CH_RX_RST_DONE_FALLING) == GTWIZ_CH_RX_RST_DONE_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Channel reset done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_CH_RX_RST_DONE_RISING) == GTWIZ_CH_RX_RST_DONE_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Channel reset done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }

            // Expect live status high
            if ((read_data & GTWIZ_CH_RX_SYNC_DONE_LIVE) != GTWIZ_CH_RX_SYNC_DONE_LIVE)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Channel sync done", "low"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not falling
            if ((read_data & GTWIZ_CH_RX_SYNC_DONE_FALLING) == GTWIZ_CH_RX_SYNC_DONE_FALLING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Channel sync done", "falling"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
            // Expect not rising
            if ((read_data & GTWIZ_CH_RX_SYNC_DONE_RISING) == GTWIZ_CH_RX_SYNC_DONE_RISING)
            {
                LogMessage(MSG_ETH_068, {std::to_string(lane_idx), "Rx", "GTF Channel sync done", "rising"});
                lane_failure[lane_idx] = RET_FAILURE;
                ret = RET_FAILURE;
            }
        }
    }

    if (ret == RET_SUCCESS)
    {
        LogMessage(MSG_ETH_081, {"Rx"});
    }

    return ret;
}

bool MultiGTPrbsTest::CheckPllLock( std::map<uint, bool> & lane_failure, const bool & expect )
{
    auto ret = RET_SUCCESS;
    uint read_data;

    ReadMultiGtPrbsCu(STAT_REG_ADDR, read_data);
    // Expect PLL lock status to be high
    if ((read_data & PLL_LOCK_LIVE) != PLL_LOCK_LIVE)
    {
        LogMessage(MSG_ETH_067, {"low"});
        ret = RET_FAILURE;
        Abort();
    }

    // if PLL_UNSTABLE, no need to check edge detector, just check that lock is there
    if (expect == PLL_LOCK_EXPECTED)
    {
       // Expect PLL lock status not falling
        if ((read_data & PLL_LOCK_FALLING) == PLL_LOCK_FALLING)
        {
            LogMessage(MSG_ETH_067, {"falling"});
            ret = RET_FAILURE;
        }

        // Expect PLL lock status not rising
        if ((read_data & PLL_LOCK_RISING) == PLL_LOCK_RISING)
        {
            LogMessage(MSG_ETH_067, {"rising"});
            ret = RET_FAILURE;
        }
    }

    if (ret == RET_FAILURE)
    {
        return SetAllLaneFailure(lane_failure);
    }

    LogMessage(MSG_DEBUG_TESTCASE, {"OK: PLL lock stays high"});
    return RET_SUCCESS;
}


void MultiGTPrbsTest::EnableTXPrbs()
{
    LogMessage(MSG_DEBUG_TESTCASE, {"Enable TX PRBS"});
    auto data = TX_ENABLE;
    if (this->m_TC_Cfg.global_param.disable_ref_prbs)
    {
        data = data | DISABLE_REF_PRBS;
    }
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, data);
}

void MultiGTPrbsTest::DisableTXPrbs()
{
    LogMessage(MSG_DEBUG_TESTCASE, {"Disable TX PRBS"});
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, 0x0);
}

void MultiGTPrbsTest::EnableRXPrbsCheck()
{
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Enable RX PRBS Check"});
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | RX_ENABLE);
}

void MultiGTPrbsTest::DisableRXPrbsCheck()
{
    uint read_data;

    LogMessage(MSG_DEBUG_TESTCASE, {"Disable RX PRBS Check"});
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data & ~RX_ENABLE);
}

void MultiGTPrbsTest::SendGTCfg()
{
    LogMessage(MSG_DEBUG_TESTCASE, {"Send GT config"});
    uint write_data;
    for (const auto & lane_idx : this->m_enable_lane)
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

        write_data = 0; // init
        write_data |= (  lane_lpbk                                                                            & LANES_CFG_LOOPBACK_MASK      ) << LANES_CFG_LOOPBACK_OFFSET;
        write_data |= ( (this->m_TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm                       ? 0x1 : 0x0) & LANES_CFG_RX_USE_LPM_MASK    ) << LANES_CFG_RX_USE_LPM_OFFSET;
        write_data |= ( (this->m_TC_Cfg.lane_param[lane_idx].rx_polarity == RX_POLARITY_INVERTED ? 0x1 : 0x0) & LANES_CFG_RX_POLARITY_MASK   ) << LANES_CFG_RX_POLARITY_OFFSET;
        write_data |= ( (this->m_TC_Cfg.lane_param[lane_idx].tx_polarity == TX_POLARITY_INVERTED ? 0x1 : 0x0) & LANES_CFG_TX_POLARITY_MASK   ) << LANES_CFG_TX_POLARITY_OFFSET;
        write_data |= (  this->m_TC_Cfg.lane_param[lane_idx].gt_tx_post_emph                                  & LANES_CFG_TX_POST_EMPH_MASK  ) << LANES_CFG_TX_POST_EMPH_OFFSET;
        write_data |= (  this->m_TC_Cfg.lane_param[lane_idx].gt_tx_main_cursor                                & LANES_CFG_TX_MAIN_CURSOR_MASK) << LANES_CFG_TX_MAIN_CURSOR_OFFSET;
        write_data |= (  this->m_TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph                                   & LANES_CFG_TX_PRE_EMPH_MASK   ) << LANES_CFG_TX_PRE_EMPH_OFFSET;
        write_data |= (  this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl                                   & LANES_CFG_TX_DIFFCTRL_MASK   ) << LANES_CFG_TX_DIFFCTRL_OFFSET;

        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Lane[" + std::to_string(lane_idx) + "] write data: 0x" + NumToStrHex<uint>(write_data)});
        WriteMultiGtPrbsCu(GT_LANES_CFG_REG_ADDR[lane_idx], write_data);
    }
    /*
    uint read_data;
    for (const auto & lane_idx : this->m_enable_lane)
    {
        ReadMultiGtPrbsCu(GT_LANES_CFG_REG_ADDR[lane_idx], read_data);
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Lane[" + std::to_string(lane_idx) + "] read data: 0x" + NumToStrHex<uint>(read_data)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- LOOPBACK      : 0x" + std::to_string((read_data >> LANES_CFG_LOOPBACK_OFFSET      ) & LANES_CFG_LOOPBACK_MASK      )});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- RX_USE_LPM    : 0x" + std::to_string((read_data >> LANES_CFG_RX_USE_LPM_OFFSET    ) & LANES_CFG_RX_USE_LPM_MASK    )});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- RX_POLARITY   : 0x" + std::to_string((read_data >> LANES_CFG_RX_POLARITY_OFFSET   ) & LANES_CFG_RX_POLARITY_MASK   )});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- TX_POLARITY   : 0x" + std::to_string((read_data >> LANES_CFG_TX_POLARITY_OFFSET   ) & LANES_CFG_TX_POLARITY_MASK   )});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- TX_POST_EMPH  : 0x" + std::to_string((read_data >> LANES_CFG_TX_POST_EMPH_OFFSET  ) & LANES_CFG_TX_POST_EMPH_MASK  )});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- TX_MAIN_CURSOR: 0x" + std::to_string((read_data >> LANES_CFG_TX_MAIN_CURSOR_OFFSET) & LANES_CFG_TX_MAIN_CURSOR_MASK)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- TX_PRE_EMPH   : 0x" + std::to_string((read_data >> LANES_CFG_TX_PRE_EMPH_OFFSET   ) & LANES_CFG_TX_PRE_EMPH_MASK   )});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t\t- TX_DIFFCTRL   : 0x" + std::to_string((read_data >> LANES_CFG_TX_DIFFCTRL_OFFSET   ) & LANES_CFG_TX_DIFFCTRL_MASK   )});
    }
    */
}

void MultiGTPrbsTest::ClearStatus()
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Clear status"});
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | CLEAR_STATUS_REGISTERS);
    for (const auto & lane_idx : this->m_available_lane)
    {
        this->m_rx_lane_status[lane_idx] = RESET_RX_LANE_STATUS;
        if (!this->m_TC_Cfg.lane_param[lane_idx].enable)
        {
            this->m_rx_lane_status[lane_idx].result_str = MULTI_GT_TEST_DISABLE;
        }
    }

}

void MultiGTPrbsTest::LatchStatus()
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Latch status"});
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | LATCH_STATUS_REGISTERS);
}

void MultiGTPrbsTest::InsertError(uint lane_idx)
{
    uint read_data;
    LogMessage(MSG_DEBUG_TESTCASE, {"Insert Error"});
    ReadMultiGtPrbsCu (CTRL_REG_ADDR, read_data);
    WriteMultiGtPrbsCu(CTRL_REG_ADDR, read_data | TX_ERR_INJ[lane_idx]);
}


double MultiGTPrbsTest::ComputeRate(uint64_t word_count, uint duration)
{
    double rate = 0.0;
    if (duration > 0)
    {
        rate =  ( (double)(word_count) * (double)(this->m_rate.data_size) ) / ( (double)(duration) * (double)(1000000000) );
    }
    return rate;
}

void MultiGTPrbsTest::ComputeVariationRates(uint64_t word_count, uint32_t duration, std::map<uint, double> & rates)
{
    // compute rate around duration received,
    // it's protection against CDC race in the HW between toggle 1sec and latch status
    if (duration >= 1)
    {
        rates[duration - 1] =  ComputeRate(word_count, duration - 1);
        rates[duration]     =  ComputeRate(word_count, duration);
        rates[duration + 1] =  ComputeRate(word_count, duration + 1);
        /*
        LogMessage(MSG_DEBUG_TESTCASE, {"Rate variations computed (duration/rate): " +
                                        std::to_string(duration - 1) + "/" + Float_to_String<double>(rates[duration - 1],4) + "; " +
                                        std::to_string(duration)     + "/" + Float_to_String<double>(rates[duration],4)     + "; " +
                                        std::to_string(duration + 1) + "/" + Float_to_String<double>(rates[duration + 1],4) + "; "
                                    });
        */

    }
    else
    {
        rates[0] =  0.0;
    }

}

bool MultiGTPrbsTest::SelectRate(std::map<uint, double> & rates, uint32_t & duration_sel, double & rate_sel)
{
    auto ret = RET_FAILURE;
    // map: first element is duration
    //      second element is rate

    // pick any of the rate which is within the range
    for (const auto & rate : rates)
    {
        if ((rate.second > this->m_rate.low_threshold) && (rate.second < this->m_rate.high_threshold))
        {
            duration_sel = rate.first;
            rate_sel = rate.second;
            ret = RET_SUCCESS;
            //LogMessage(MSG_DEBUG_TESTCASE, {"Rate selected: " + std::to_string(duration_sel) + ", " + Float_to_String<double>(rate_sel,4)});
            return ret;
        }
    }
    return ret;
}

void MultiGTPrbsTest::ComputeRxStatus()
{

    // current result for a single lane
    bool lane_error;

    // And Parse each of lane results
    for (const auto & lane_idx : this->m_enable_lane)
    {
        lane_error = RET_SUCCESS;
        this->m_rx_lane_status[lane_idx].sample_cnt++;
        this->m_rx_lane_status[lane_idx].bit_cnt = (double)this->m_rx_status.word_cnt[lane_idx] * (double)this->m_rate.data_size;
        this->m_rx_lane_status[lane_idx].bit_error_cnt = (double)this->m_rx_status.bit_error_cnt[lane_idx];
        this->m_rx_lane_status[lane_idx].acc_bit_cnt += (double)this->m_rx_lane_status[lane_idx].bit_cnt;
        this->m_rx_lane_status[lane_idx].acc_bit_error_cnt += (double)this->m_rx_lane_status[lane_idx].bit_error_cnt;
        //compute rate over all samples received so far
        this->m_rx_lane_status[lane_idx].rate = this->m_rx_lane_status[lane_idx].acc_bit_cnt / ((double) this->m_rx_lane_status[lane_idx].sample_cnt * (double)(1000000000));

        if (this->m_rx_lane_status[lane_idx].acc_bit_cnt == 0)
        {
            LogMessage(MSG_ETH_061, {std::to_string(lane_idx), "received"});
            lane_error = RET_FAILURE;
        }
        else
        {
            this->m_rx_lane_status[lane_idx].ber = this->m_rx_lane_status[lane_idx].acc_bit_error_cnt / this->m_rx_lane_status[lane_idx].acc_bit_cnt;
        }

        if (this->m_rx_lane_status[lane_idx].bit_cnt == 0)
        {
            lane_error = RET_FAILURE;
            LogMessage(MSG_ETH_061, {std::to_string(lane_idx), "received"});
        }

        if ((this->m_rx_lane_status[lane_idx].rate < this->m_rate.low_threshold) || (this->m_rx_lane_status[lane_idx].rate > this->m_rate.high_threshold))
        {
            lane_error = RET_FAILURE;
            LogMessage(MSG_ETH_071, {std::to_string(lane_idx), "Rx", Float_to_String<double>(this->m_rate.data_rate,2),
                        Float_to_String<double>(this->m_rate.precision,1), Float_to_String<double>(this->m_rx_lane_status[lane_idx].rate,4) });
        }

        if (this->m_rx_lane_status[lane_idx].ber > this->m_TC_Cfg.lane_param[lane_idx].ber_threshold)
        {
            lane_error = RET_FAILURE;
            LogMessage(MSG_ETH_086, {std::to_string(lane_idx), Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].acc_bit_error_cnt,3),
                                        Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].ber, 3),
                                        Float_to_Scientific_String<double>(this->m_TC_Cfg.lane_param[lane_idx].ber_threshold, 1)});
        }

        // accumulate the result, since last clear
        if (lane_error == RET_FAILURE)
        {
            this->m_rx_lane_status[lane_idx].result = lane_error;
            this->m_rx_lane_status[lane_idx].result_str = MULTI_GT_TEST_FAIL;
        }
    }
}

bool MultiGTPrbsTest::CheckLaneStatus(std::map<uint, bool> & lane_failure)
{
    // global result of all lanes
    auto ret = RET_SUCCESS;
    // current result for a single lane
    bool lane_error;

    LogMessage(MSG_ETH_002);
    uint global_status;
    ReadMultiGtPrbsCu(STAT_REG_ADDR, global_status);
    std::map<uint, double>  variation_rates;
    std::map<uint, double>  selected_rate;
    double data_rate = 0.0;
    auto rate_selection = RET_FAILURE;
    double tx_bit_cnt;

    ReadTxStatus();
    auto hw_sec_cnt = this->m_tx_status.stopwatch;
    hw_sec_cnt--; // first second is not accumulating anything

    LogMessage(MSG_DEBUG_TESTCASE, {"\t- Measurement duration (since last clear): " + std::to_string(hw_sec_cnt) + " seconds" });

    // And Parse each of lane results
    for (const auto & lane_idx : this->m_available_lane)
    {
        if (!this->m_TC_Cfg.lane_param[lane_idx].enable)
        {
            lane_failure[lane_idx] = RET_SUCCESS;
        }
        else
        {
            lane_error = RET_SUCCESS;

            // RX results, everyting has been computed and checked along, just display the pass if there is any

            if (this->m_rx_lane_status[lane_idx].acc_bit_cnt == 0)
            {
                LogMessage(MSG_ETH_061, {std::to_string(lane_idx), "received"});
                lane_error = RET_FAILURE;
            }
            else
            {
                LogMessage(MSG_ETH_089, {std::to_string(lane_idx), Float_to_String<double>(this->m_rx_lane_status[lane_idx].acc_bit_cnt, 0), "received",
                            std::to_string(this->m_rx_lane_status[lane_idx].sample_cnt) });

                lane_error |= this->m_rx_lane_status[lane_idx].result;
                if ((this->m_rx_lane_status[lane_idx].rate > this->m_rate.low_threshold) && (this->m_rx_lane_status[lane_idx].rate < this->m_rate.high_threshold))
                {
                    LogMessage(MSG_ETH_070, {std::to_string(lane_idx), "Rx", Float_to_String<double>(this->m_rate.data_rate,2),
                                Float_to_String<double>(this->m_rate.precision,1), Float_to_String<double>(this->m_rx_lane_status[lane_idx].rate,4) });
                }
                if (this->m_rx_lane_status[lane_idx].ber < this->m_TC_Cfg.lane_param[lane_idx].ber_threshold)
                {
                    LogMessage(MSG_ETH_087, {std::to_string(lane_idx), Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].acc_bit_error_cnt,3),
                                            Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].ber, 3),
                                            Float_to_Scientific_String<double>(this->m_TC_Cfg.lane_param[lane_idx].ber_threshold, 1)});
                }
            }

            // TX results are only check now
            if (this->m_tx_status.word_cnt[lane_idx] == 0)
            {
                LogMessage(MSG_ETH_061, {std::to_string(lane_idx), "transmitted"});
                lane_error = RET_FAILURE;
            }
            else
            {
                tx_bit_cnt = (double) this->m_tx_status.word_cnt[lane_idx] * (double)this->m_rate.data_size;
                LogMessage(MSG_ETH_089, {std::to_string(lane_idx), Float_to_String<double>(tx_bit_cnt, 0), "transmitted", std::to_string(hw_sec_cnt) });

                ComputeVariationRates(this->m_tx_status.word_cnt[lane_idx], hw_sec_cnt, variation_rates);
                rate_selection = SelectRate(variation_rates, hw_sec_cnt, data_rate);

                if (rate_selection == RET_SUCCESS)
                {
                    LogMessage(MSG_ETH_070, {std::to_string(lane_idx), "Tx", Float_to_String<double>(this->m_rate.data_rate,2),
                            Float_to_String<double>(this->m_rate.precision,1), Float_to_String<double>(data_rate,4)});
                }
                else
                {
                    // could not find a valid rate, print what has been received
                    data_rate = ComputeRate(this->m_tx_status.word_cnt[lane_idx], hw_sec_cnt);
                    LogMessage(MSG_ETH_071, {std::to_string(lane_idx), "Tx", Float_to_String<double>(this->m_rate.data_rate,2),
                            Float_to_String<double>(this->m_rate.precision,1), Float_to_String<double>(data_rate,4) });
                    lane_error = RET_FAILURE;
                }
            }

            if ( (global_status & RX_SEED_ZERO[lane_idx]) == RX_SEED_ZERO[lane_idx] )
            {
                lane_error = RET_FAILURE;
                LogMessage(MSG_ETH_058, { std::to_string(lane_idx), "Rx"});
            }
            else
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Rx seed good for lane[" + std::to_string(lane_idx) + "]"});
            }

            if ( (global_status & TX_SEED_ZERO[lane_idx]) == TX_SEED_ZERO[lane_idx] )
            {
                lane_error = RET_FAILURE;
                LogMessage(MSG_ETH_058, { std::to_string(lane_idx), "Tx"});
            }
            else
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"TX seed good for lane[" + std::to_string(lane_idx) + "]"});
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
            // save the current lane result into the array of results
            ret |= lane_error;
        }
    }
    return ret;
}

bool MultiGTPrbsTest::SetAllLaneFailure( std::map<uint, bool> & lane_failure )
{
    for (const auto & lane_idx : this->m_enable_lane)
    {
        lane_failure[lane_idx] = RET_FAILURE;
    }
    return RET_FAILURE;
}

void MultiGTPrbsTest::ReadRxStatus()
{
    uint32_t rd_buffer[this->m_buffer_size/(uint64_t)(sizeof(uint32_t))];
    ReadMultiGtPrbsStatus(&rd_buffer[0], this->m_buffer_size);

    int j = 0;
    for (int i = 0; i <= 3; i++)
    {
        this->m_rx_status.word_cnt[i]       = ((uint64_t)(rd_buffer[j+1] & 0xFFFF ) << 32) + (uint64_t)(rd_buffer[j]);
        this->m_rx_status.bit_error_cnt[i]  = ((uint64_t)(rd_buffer[j+2]) << 16) + (uint64_t)((rd_buffer[j+1] & 0xFFFF0000) >> 16);
        j = j + 3;
    }
    this->m_rx_status.toggle_1_sec      = (rd_buffer[j] & 0x80000000) >> 31;
    this->m_rx_status.timestamp_1_sec   = rd_buffer[j] & 0x7FFFFFFF;

    //for (int i = 0; i < 13; i++)
    //{
    //    LogMessage(MSG_DEBUG_TESTCASE, {"rd_buffer[" +  std::to_string(i) + "]: 0x" + NumToStrHex<uint>(rd_buffer[i])});
    //}
    //for (int i = 0; i <= 3; i++)
    //{
    //    LogMessage(MSG_DEBUG_TESTCASE, {"word_cnt[" +  std::to_string(i) + "]: 0x" + NumToStrHex<uint64_t>(this->m_rx_status.word_cnt[i])});
    //    LogMessage(MSG_DEBUG_TESTCASE, {"bit_error_cnt[" +  std::to_string(i) + "]: 0x" + NumToStrHex<uint64_t>(this->m_rx_status.bit_error_cnt[i])});
    //}
}

void MultiGTPrbsTest::ReadTxStatus()
{
    //TX status are simple read register as they are not part of the multiread
    uint32_t rddata[7];
    ReadMultiGtPrbsCu(TX_WORD_CNT_LN_0_REG_ADDR  , rddata[0]);
    ReadMultiGtPrbsCu(TX_WORD_CNT_LN_0_1_REG_ADDR, rddata[1]);
    ReadMultiGtPrbsCu(TX_WORD_CNT_LN_1_REG_ADDR  , rddata[2]);
    ReadMultiGtPrbsCu(TX_WORD_CNT_LN_2_REG_ADDR  , rddata[3]);
    ReadMultiGtPrbsCu(TX_WORD_CNT_LN_2_3_REG_ADDR, rddata[4]);
    ReadMultiGtPrbsCu(TX_WORD_CNT_LN_3_REG_ADDR  , rddata[5]);
    ReadMultiGtPrbsCu(STOPWATCH_REG_ADDR         , rddata[6]);

    int j = 0;
    for (int i = 0; i < 2; i++)
    {
        this->m_tx_status.word_cnt[2*i]       = ((uint64_t)(rddata[j+1] & 0xFFFF) << 32) + (uint64_t)(rddata[j]);
        this->m_tx_status.word_cnt[2*i+1]     = ((uint64_t)(rddata[j+2]) << 16) + (uint64_t)((rddata[j+1] & 0xFFFF0000) >> 16);
        j = j + 3;
    }
    this->m_tx_status.stopwatch     = rddata[j];
}

void MultiGTPrbsTest::GetHw1SecToggle(bool & toggle_1_sec, uint32_t & timestamp_1_sec)
{
    uint32_t read_data;
    ReadMultiGtPrbsCu(HW_SEC_CNT_REG_ADDR, read_data);
    toggle_1_sec = (read_data & 0x80000000) >> 31;
    timestamp_1_sec = read_data & 0x7FFFFFFF;

    //LogMessage(MSG_DEBUG_TESTCASE, {"HW Sec toggle: " + std::to_string(toggle_1_sec) + ", HW sec Counter: " + std::to_string(timestamp_1_sec) });
}

void MultiGTPrbsTest::WaitHw1SecToggle()
{
    uint watchdog = 5; // Ensure 1 sec is detected in 4 * 333 ms = 1.33 sec
    while ((watchdog > 0) && (!(this->m_abort)))
    {
        GetHw1SecToggle(this->m_tog_1sec, this->m_hw_sec_cnt);
        if (this->m_tog_1sec != this->m_tog_1sec_last)
        {
            this->m_tog_1sec_last = this->m_tog_1sec;
            break;
        }
        this->m_timer->WaitFor333msTick();
        watchdog--;
    }

    if (watchdog == 0)
    {
        LogMessage(MSG_DEBUG_EXCEPTION, {"1 sec HW toggle not detected"});
        this->m_toggle_error_cnt++;
        if (this->m_toggle_error_cnt >= 5)
        {
            LogMessage(MSG_MEM_019, {std::to_string(this->m_toggle_error_cnt)});
        }
    }
    else
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"1 sec toggle detected after " + std::to_string(5-watchdog) + " wait of 333ms"});
        this->m_toggle_error_cnt = 0;
    }

    CheckTimestamp1Sec();

}

void MultiGTPrbsTest::CheckTimestamp1Sec()
{
    // Under normal operations, the HW 1sec timestamp shall increment by one between two consecutive measurements (read by host every 1 sec)
    if (this->m_hw_sec_cnt > 1)
    {
        if (this->m_hw_sec_cnt != this->m_hw_sec_cnt_expected)
        {
            LogMessage(MSG_ETH_088, {std::to_string(this->m_hw_sec_cnt), std::to_string(this->m_hw_sec_cnt_expected)});
        }
    }
    this->m_hw_sec_cnt_expected = this->m_hw_sec_cnt + 1;
}


int MultiGTPrbsTest::RunTest()
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
        std::map<uint, std::vector<double>>  lane_rx_cnt;

        LogMessage(MSG_CMN_032, {std::to_string(test_it_cnt), StrVectToTest(test_it.test_strvect)}); // start test
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Extrapolated test sequence: " + StrVectToTest(test_it.test_strvect_dbg)});

        GetHw1SecToggle(this->m_tog_1sec, this->m_hw_sec_cnt);
        this->m_tog_1sec_last     = this->m_tog_1sec;
        this->m_sw_sec_cnt_start  = this->m_timer->GetSecCnt();

        if (test_it.cmd == CONFIG_GT)
        {
            ClearStatus();
            SendGTCfg();
            EnableTXPrbs();
        }
        else if (test_it.cmd == TX_RX_RST)
        {

            test_it_failure |= CheckGTPowerGood(lane_failure);
            ClearStatus();
            ResetGT(); // global reset
            WaitSecTick(2);
            test_it_failure |= CheckPllLock(lane_failure, PLL_UNSTABLE);
            test_it_failure |= CheckResetRequest();
            test_it_failure |= CheckResetOver(lane_failure);
            ClearStatus();
        }
        else if ((test_it.cmd == TX_RST)  || (test_it.cmd == TX_DATAPATH_RST) )
        {
            test_it_failure |= CheckGTPowerGood(lane_failure);
            ClearStatus();
            if (test_it.cmd == TX_DATAPATH_RST)
            {
                test_it_failure |= ResetGTTxDatapath();
            }
            else
            {
                test_it_failure |= ResetGTTx();
            }
            WaitSecTick(2);
            test_it_failure |= CheckTxResetRequest();

            ClearStatus();
        }
        else if ((test_it.cmd == RX_RST) || (test_it.cmd == RX_DATAPATH_RST) )
        {
            test_it_failure |= CheckGTPowerGood(lane_failure);
            ClearStatus();
            if (test_it.cmd == RX_DATAPATH_RST)
            {
                test_it_failure |= ResetGTRxDatapath();
            }
            else
            {
                test_it_failure |= ResetGTRx();
            }
            WaitSecTick(1);
            test_it_failure |= CheckRxResetRequest();

            ClearStatus();
        }
        else if (test_it.cmd == INSERT_ERROR )
        {
            InsertError(test_it.insert_error_lane_idx);
        }

        auto duration_divider = test_it.duration / 10;
        if (duration_divider == 0)
        {
            duration_divider = 1;
        }

        this->m_sw_sec_cnt  = this->m_timer->GetSecCnt();
        uint64_t remaining_time;

        if (test_it.cmd != RUN )
        {
            // do not rely on the HW sec count as there are some mutex
            // Loop until done or abort
            for (uint i = test_it.duration; (i >= 1) && (!(this->m_abort)); i--)
            {
                if (((i % duration_divider == 0) || (i == test_it.duration)) && (!(this->m_abort)))
                {
                    LogMessage(MSG_CMN_048, {std::to_string(i), ""});
                }
                WaitSecTick(1);

                this->m_testcase_queue_value.remaining_time = std::to_string(i);
                PushTestcaseQueue();
            }
        }
        else
        {
            // initialize the expected second counter
            GetHw1SecToggle(this->m_tog_1sec, this->m_hw_sec_cnt);
            this->m_hw_sec_cnt_expected = this->m_hw_sec_cnt + 1;
            do
            {
                WaitHw1SecToggle();
                if (test_it.cmd == RUN )
                {
                    ReadRxStatus();
                    ComputeRxStatus();
                    WriteToMeasurementFile();
                }

                // Compute remaining time and check we need to display it at next iteration
                auto tmp_sw_sec_cnt = this->m_timer->GetSecCnt();
                if (tmp_sw_sec_cnt != this->m_sw_sec_cnt)
                {
                    this->m_sw_sec_cnt = tmp_sw_sec_cnt;
                    remaining_time = test_it.duration - (this->m_sw_sec_cnt - this->m_sw_sec_cnt_start);

                    if (((remaining_time % duration_divider == 0) || (remaining_time == test_it.duration)) && (!(this->m_abort)))
                    {
                        LogMessage(MSG_CMN_048, {std::to_string(remaining_time), ""});
                    }
                    this->m_testcase_queue_value.remaining_time = std::to_string(remaining_time);
                    PushTestcaseQueue();
                }
            } while ((this->m_sw_sec_cnt - this->m_sw_sec_cnt_start < test_it.duration) && !(this->m_abort));
        }
        LogMessage(MSG_CMN_049); // test duration reached


        if (test_it.cmd == CONFIG_GT)
        {
            EnableRXPrbsCheck();
        }
        else if ((test_it.cmd == TX_RX_RST) || (test_it.cmd == TX_RST) || (test_it.cmd == RX_RST) || (test_it.cmd == TX_DATAPATH_RST) || (test_it.cmd == RX_DATAPATH_RST) )
        {
            test_it_failure |= CheckTxResetStable(lane_failure);
            test_it_failure |= CheckRxResetStable(lane_failure);
            test_it_failure |= CheckPllLock(lane_failure, PLL_LOCK_EXPECTED);
        }
        else if (test_it.cmd == CLEAR_STATUS)
        {
            ClearStatus();

        }
        else if (test_it.cmd == CHECK_STATUS)
        {
            LatchStatus();
            // Pause so results are not interleaved.
            //      std::this_thread::sleep_for(std::chrono::milliseconds(100*this->m_gt_index));
            // Check resets and PLL lock status stable during run
            test_it_failure |= CheckGTPowerGood(lane_failure);
            test_it_failure |= CheckPllLock(lane_failure, PLL_LOCK_EXPECTED);
            test_it_failure |= CheckTxResetStable(lane_failure);
            test_it_failure |= CheckRxResetStable(lane_failure);
            test_it_failure |= CheckLaneStatus(lane_failure);
        }

        for (const auto & lane_idx : this->m_available_lane)
        {
            if (this->m_TC_Cfg.lane_param[lane_idx].enable)
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
            else
            {
                this->m_test_it_lane_results[lane_idx][test_it_cnt-1] = TITR_DISABLED;
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
                for (const auto & lane_idx : this->m_enable_lane)
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

bool MultiGTPrbsTest::StartCU()
{
    return RET_SUCCESS;
}

bool MultiGTPrbsTest::EnableWatchdogClkThrotDetection()
{
    return RET_SUCCESS;
}

bool MultiGTPrbsTest::StopCU()
{
    return RET_SUCCESS;
}

bool MultiGTPrbsTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_SUCCESS;
}

bool MultiGTPrbsTest::GetTestSequence()
{
    for (const auto & lane_idx : this->m_available_lane)
    {
        this->m_test_it_lane_results[lane_idx] = {};
    }
    auto it = FindJsonParam(this->m_test_parameters.global_config, TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters.global_config.end())
    {
        this->m_test_sequence = TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>>(it->second);
        for (const auto & lane_idx : this->m_available_lane)
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

bool MultiGTPrbsTest::ParseTestSequenceSettings( std::list<MultiGTPrbsTestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    int  test_cnt = 0;

    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_sequence_dbg;
    std::vector<bool>        lane_selection;
    for (const auto & test_seq_param : this->m_test_sequence)
    {
        if (this->m_abort)
        {
            break;
        }

        auto parse_it_failure = RET_SUCCESS;
        MultiGTPrbsTestItConfig_t test_it_cfg;

        test_cnt++;

        test_it_cfg.test_strvect = {}; // Displays what user entered
        test_it_cfg.test_strvect_dbg = {}; // Displays with extrapolated parameters

        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_seq_param.duration.value, MIN_DURATION, MAX_DURATION_48H);
            test_it_cfg.duration = test_seq_param.duration.value;
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure = CheckStringInSet(MODE_TEST_SEQ_MEMBER.name, test_seq_param.mode.value, SUPPORTED_MULTI_GT_PRBS_SEQUENCE_MODE);
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
            else if (StrMatchNoCase(test_seq_param.mode.value, TX_RST_STR))
            {
                test_it_cfg.cmd = TX_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, RX_RST_STR))
            {
                test_it_cfg.cmd = RX_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, TX_DATAPATH_RST_STR))
            {
                test_it_cfg.cmd = TX_DATAPATH_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, RX_DATAPATH_RST_STR))
            {
                test_it_cfg.cmd = RX_DATAPATH_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CHECK_STATUS_STR))
            {
                test_it_cfg.cmd = CHECK_STATUS;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CLEAR_STATUS_STR))
            {
                test_it_cfg.cmd = CLEAR_STATUS;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_MULTI_GT_STR))
            {
                test_it_cfg.cmd = CONFIG_GT;
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

void MultiGTPrbsTest::PrintLaneParam ( const LaneParam_t & lane_param )
{
    LogMessage(MSG_CMN_021, {"\t- Enable : "                                        +      BoolToStr(lane_param.enable)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_BER_THRESHOLD_MEMBER.name          + " : " + Float_to_Scientific_String<double>(lane_param.ber_threshold,3)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_SETTINGS_MEMBER.name               + " : " +                lane_param.gt_settings});
    LogMessage(MSG_CMN_021, {"\t- " + GT_LOOPBACK_MEMBER.name               + " : " +                lane_param.gt_lpbk});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_DIFFCTRL_MEMBER.name            + " : " + std::to_string(lane_param.gt_tx_diffctrl)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_MAIN_CURSOR_MEMBER.name         + " : " + std::to_string(lane_param.gt_tx_main_cursor)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_PRE_EMPH_MEMBER.name            + " : " + std::to_string(lane_param.gt_tx_pre_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POST_EMPH_MEMBER.name           + " : " + std::to_string(lane_param.gt_tx_post_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_RX_USE_LPM_MEMBER.name             + " : " +      BoolToStr(lane_param.gt_rx_use_lpm)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POLARITY_MEMBER.name            + " : " +                lane_param.tx_polarity});
    LogMessage(MSG_CMN_021, {"\t- " + GT_RX_POLARITY_MEMBER.name            + " : " +                lane_param.rx_polarity});
}

void MultiGTPrbsTest::WriteToMeasurementFile()
{
    std::vector<std::string> str_vect;
    str_vect.emplace_back(std::to_string(this->m_timer->GetSecCnt()));

    for (const auto & lane_idx : this->m_available_lane)
    {
        str_vect.emplace_back( this->m_rx_lane_status[lane_idx].result_str);
        str_vect.emplace_back( std::to_string(this->m_rx_lane_status[lane_idx].rate));
        str_vect.emplace_back( std::to_string((uint64_t)(this->m_rx_lane_status[lane_idx].bit_cnt)));
        str_vect.emplace_back( std::to_string((uint64_t)(this->m_rx_lane_status[lane_idx].bit_error_cnt)));
        str_vect.emplace_back( Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].acc_bit_cnt, 2));
        str_vect.emplace_back( Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].acc_bit_error_cnt, 2));
        str_vect.emplace_back( Float_to_Scientific_String<double>(this->m_rx_lane_status[lane_idx].ber, 3));
    }

    if (this->m_use_outputfile && !(this->m_abort) )
    {
        this->m_outputfile << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile.flush();

        (void)OpenRTOutputFile(this->m_RT_outputfile_name, this->m_RT_outputfile); // Don't check if open worked..
        this->m_RT_outputfile << this->m_RT_outputfile_head + "\n" + StrVectToStr(str_vect, ",");
        this->m_RT_outputfile.flush();
        this->m_RT_outputfile.close();
    }

}

std::vector<uint> MultiGTPrbsTest::GetAvailableLane()
{
    return this->m_available_lane;
}

} // namespace
