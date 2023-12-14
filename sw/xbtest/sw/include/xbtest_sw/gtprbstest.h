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

#ifndef _GTPRBSTEST_H
#define _GTPRBSTEST_H

#include <list>

#include "testinterface.h"
#include "devicemgt.h"
#include "gtmacpackage.h"

namespace xbtest
{

class GTPrbsTest : public TestInterface
{

public:
    GTPrbsTest(
        Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
        const Gt_Parameters_t & test_parameters, const uint & cu_idx
    );
    ~GTPrbsTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

private:
    const std::string REP = "%";

    const uint  CTRL_REG_ADDR               = 0x20;
    const uint  GT_LANE_0_CFG_REG_ADDR      = 0x21;
    const uint  GT_LANE_1_CFG_REG_ADDR      = 0x22;
    const uint  GT_LANE_2_CFG_REG_ADDR      = 0x23;
    const uint  GT_LANE_3_CFG_REG_ADDR      = 0x24;
    const std::array<uint,4>  GT_LANES_CFG_REG_ADDR = {GT_LANE_0_CFG_REG_ADDR, GT_LANE_1_CFG_REG_ADDR, GT_LANE_2_CFG_REG_ADDR, GT_LANE_3_CFG_REG_ADDR};

    const uint  RESET_GT            = (0x1 << 0);
    const uint  GT_POWER_GOOD       = (0x1 << 1);
    const uint  GT_TX_RST_DONE      = (0x1 << 2);
    const uint  GT_RX_RST_DONE      = (0x1 << 3);
    const uint  TX_ENABLE           = (0x1 << 4);
    const uint  RX_ENABLE           = (0x1 << 5);
    const uint  PLL_LOCK            = (0x1 << 6);
    const uint  CLEAR_RX_PRBS_ERR   = (0x1 << 8);
    const uint  CLEAR_RX_SEED_ZERO  = (0x1 << 9);
    const uint  CLEAR_TX_SEED_ZERO  = (0x1 << 10);
    const uint  DISABLE_REF_PRBS    = (0x1 << 28);

    const uint RX_PRBS_ERR_MASK  = (0xF << 12);
    const uint RX_PRBS_ERR_LN_0  = (0x1 << 12);
    const uint RX_PRBS_ERR_LN_1  = (0x1 << 13);
    const uint RX_PRBS_ERR_LN_2  = (0x1 << 14);
    const uint RX_PRBS_ERR_LN_3  = (0x1 << 15);
    const std::array<uint,4> RX_PRBS_ERR = {RX_PRBS_ERR_LN_0, RX_PRBS_ERR_LN_1, RX_PRBS_ERR_LN_2, RX_PRBS_ERR_LN_3};

    const uint RX_SEED_ZERO_MASK = (0xF << 16);
    const uint RX_SEED_ZERO_LN_0 = (0x1 << 16);
    const uint RX_SEED_ZERO_LN_1 = (0x1 << 17);
    const uint RX_SEED_ZERO_LN_2 = (0x1 << 18);
    const uint RX_SEED_ZERO_LN_3 = (0x1 << 19);
    const std::array<uint,4> RX_SEED_ZERO = {RX_SEED_ZERO_LN_0, RX_SEED_ZERO_LN_1, RX_SEED_ZERO_LN_2, RX_SEED_ZERO_LN_3};

    const uint TX_SEED_ZERO_MASK = (0xF << 20);
    const uint TX_SEED_ZERO_LN_0 = (0x1 << 21);
    const uint TX_SEED_ZERO_LN_1 = (0x1 << 22);
    const uint TX_SEED_ZERO_LN_2 = (0x1 << 23);
    const uint TX_SEED_ZERO_LN_3 = (0x1 << 24);
    const std::array<uint,4> TX_SEED_ZERO = {TX_SEED_ZERO_LN_0, TX_SEED_ZERO_LN_1, TX_SEED_ZERO_LN_2, TX_SEED_ZERO_LN_3};

    const uint TX_ERR_INJ_LN_0   = (0x1 << 24);
    const uint TX_ERR_INJ_LN_1   = (0x1 << 25);
    const uint TX_ERR_INJ_LN_2   = (0x1 << 26);
    const uint TX_ERR_INJ_LN_3   = (0x1 << 27);

    const uint TX_WORD_CNT_LSB_LN_0_REG_ADDR     = 0x28;
    const uint TX_WORD_CNT_MSB_LN_0_REG_ADDR     = 0x29;
    const uint TX_WORD_CNT_LSB_LN_1_REG_ADDR     = 0x2A;
    const uint TX_WORD_CNT_MSB_LN_1_REG_ADDR     = 0x2B;
    const uint TX_WORD_CNT_LSB_LN_2_REG_ADDR     = 0x2C;
    const uint TX_WORD_CNT_MSB_LN_2_REG_ADDR     = 0x2D;
    const uint TX_WORD_CNT_LSB_LN_3_REG_ADDR     = 0x2E;
    const uint TX_WORD_CNT_MSB_LN_3_REG_ADDR     = 0x2F;

    const std::array<uint,4> TX_WORD_CNT_LSB_REG_ADDR = {TX_WORD_CNT_LSB_LN_0_REG_ADDR, TX_WORD_CNT_LSB_LN_1_REG_ADDR, TX_WORD_CNT_LSB_LN_2_REG_ADDR, TX_WORD_CNT_LSB_LN_3_REG_ADDR};
    const std::array<uint,4> TX_WORD_CNT_MSB_REG_ADDR = {TX_WORD_CNT_MSB_LN_0_REG_ADDR, TX_WORD_CNT_MSB_LN_1_REG_ADDR, TX_WORD_CNT_MSB_LN_2_REG_ADDR, TX_WORD_CNT_MSB_LN_3_REG_ADDR};

    const uint RX_WORD_CNT_LSB_LN_0_REG_ADDR     = 0x30;
    const uint RX_WORD_CNT_MSB_LN_0_REG_ADDR     = 0x31;
    const uint RX_ERR_WORD_CNT_LSB_LN_0_REG_ADDR = 0x32;
    const uint RX_ERR_WORD_CNT_MSB_LN_0_REG_ADDR = 0x33;

    const uint RX_WORD_CNT_LSB_LN_1_REG_ADDR     = 0x34;
    const uint RX_WORD_CNT_MSB_LN_1_REG_ADDR     = 0x35;
    const uint RX_ERR_WORD_CNT_LSB_LN_1_REG_ADDR = 0x36;
    const uint RX_ERR_WORD_CNT_MSB_LN_1_REG_ADDR = 0x37;

    const uint RX_WORD_CNT_LSB_LN_2_REG_ADDR     = 0x38;
    const uint RX_WORD_CNT_MSB_LN_2_REG_ADDR     = 0x39;
    const uint RX_ERR_WORD_CNT_LSB_LN_2_REG_ADDR = 0x3A;
    const uint RX_ERR_WORD_CNT_MSB_LN_2_REG_ADDR = 0x3B;

    const uint RX_WORD_CNT_LSB_LN_3_REG_ADDR     = 0x3C;
    const uint RX_WORD_CNT_MSB_LN_3_REG_ADDR     = 0x3D;
    const uint RX_ERR_WORD_CNT_LSB_LN_3_REG_ADDR = 0x3E;
    const uint RX_ERR_WORD_CNT_MSB_LN_3_REG_ADDR = 0x3F;

    const std::array<uint,4> RX_WORD_CNT_LSB_REG_ADDR     = {RX_WORD_CNT_LSB_LN_0_REG_ADDR, RX_WORD_CNT_LSB_LN_1_REG_ADDR, RX_WORD_CNT_LSB_LN_2_REG_ADDR, RX_WORD_CNT_LSB_LN_3_REG_ADDR};
    const std::array<uint,4> RX_WORD_CNT_MSB_REG_ADDR     = {RX_WORD_CNT_MSB_LN_0_REG_ADDR, RX_WORD_CNT_MSB_LN_1_REG_ADDR, RX_WORD_CNT_MSB_LN_2_REG_ADDR, RX_WORD_CNT_MSB_LN_3_REG_ADDR};
    const std::array<uint,4> RX_ERR_WORD_CNT_LSB_REG_ADDR = {RX_ERR_WORD_CNT_LSB_LN_0_REG_ADDR, RX_ERR_WORD_CNT_LSB_LN_1_REG_ADDR, RX_ERR_WORD_CNT_LSB_LN_2_REG_ADDR, RX_ERR_WORD_CNT_LSB_LN_3_REG_ADDR};
    const std::array<uint,4> RX_ERR_WORD_CNT_MSB_REG_ADDR = {RX_ERR_WORD_CNT_MSB_LN_0_REG_ADDR, RX_ERR_WORD_CNT_MSB_LN_1_REG_ADDR, RX_ERR_WORD_CNT_MSB_LN_2_REG_ADDR, RX_ERR_WORD_CNT_MSB_LN_3_REG_ADDR};

    using LaneParam_t = struct LaneParam_t {
        std::string gt_settings;
        std::string gt_lpbk;
        bool        disable_ref_prbs;
        uint32_t    prbs_error_threshold;
        uint32_t    gt_tx_diffctrl;
        uint32_t    gt_tx_main_cursor;
        uint32_t    gt_tx_pre_emph;
        uint32_t    gt_tx_post_emph;
        bool        gt_rx_use_lpm;
        std::string tx_polarity;
    };
    const LaneParam_t RSTC_LANEPARAM = {
        .gt_settings            = "",
        .gt_lpbk                = GT_LPBK_DISABLE,
        .disable_ref_prbs       = false,
        .prbs_error_threshold   = NOM_PRBS_ERROR_THRESHOLD,
        .gt_tx_diffctrl         = 0,
        .gt_tx_main_cursor      = DEFAULT_GT_TX_MAIN_CURSOR,
        .gt_tx_pre_emph         = 0,
        .gt_tx_post_emph        = 0,
        .gt_rx_use_lpm          = false,
        .tx_polarity            = TX_POLARITY_NORMAL
    };

    using GTPRBSTestcaseCfg_t = struct GTPRBSTestcaseCfg_t {
        LaneParam_t                 global_param;
        std::map<uint, LaneParam_t> lane_param;
    };

    using Cmd_t = enum Cmd_t {
        RUN,
        CLEAR_STATUS,
        CHECK_STATUS,
        CONFIG_25GBE,
        TX_RX_RST,
        INSERT_ERROR
    };

    using GTPrbsTestItConfig_t = struct GTPrbsTestItConfig_t {
        uint                        duration;
        Cmd_t                       cmd;
        std::string                 cmd_str;
        std::vector<std::string>    test_strvect;
        std::vector<std::string>    test_strvect_dbg;
        uint                        insert_error_lane_idx;
    };

    DeviceMgt *                                     m_devicemgt = nullptr;
    Gt_Parameters_t                                 m_test_parameters;
    uint                                            m_cu_idx;
    uint                                            m_gt_index;
    std::vector<GTPrbs_Test_Sequence_Parameters_t>  m_test_sequence;
    GTPRBSTestcaseCfg_t                             m_TC_Cfg;
    std::list<GTPrbsTestItConfig_t>                 m_test_it_list;

    bool    WriteGtPrbsCu               ( const uint & address, const uint & value );
    bool    ReadGtPrbsCu                ( const uint & address, uint & read_data );
    bool    ResetGT();
    bool    CheckGTPowerGood();
    bool    CheckResetDone();
    bool    CheckPllLock();
    void    SendGTCfg();
    int     RunTest() override;
    bool    StartCU() override;
    bool    EnableWatchdogClkThrotDetection() override;
    bool    StopCU() override;
    bool    CheckWatchdogAndClkThrottlingAlarms() override;
    bool    GetTestSequence();
    bool    ParseTestSequenceSettings   ( std::list<GTPrbsTestItConfig_t> & test_list );
    void    PrintLaneParam              ( const LaneParam_t & lane_param );

    void    ClearLaneStatus();
    bool    CheckLaneStatus( std::map<uint, bool> & lane_failure, uint run_duration );
    bool    CheckLanesError( std::map<uint, uint64_t> & lanes_error);
    void    EnableTXPrbs();
    void    DisableTXPrbs();
    void    EnableRXPrbsCheck();
    void    DisableRXPrbsCheck();
    void    InsertError(uint lane_idx);

};

} // namespace

#endif /* _GTPRBSTEST_H */
