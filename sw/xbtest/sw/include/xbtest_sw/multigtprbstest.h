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

#ifndef _MULTIGTPRBSTEST_H
#define _MULTIGTPRBSTEST_H

#include <list>

#include "testinterface.h"
#include "devicemgt.h"
#include "gtmacpackage.h"

namespace xbtest
{

class MultiGTPrbsTest : public TestInterface
{

public:
    MultiGTPrbsTest(
        Logging * log, XbtestSwConfig * xbtest_sw_config, Multi_GT_t multi_gt_type,
        ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
        const Gt_Parameters_t & test_parameters, const uint & cu_idx, const uint & gt_index
    );
    ~MultiGTPrbsTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

    std::vector<uint> GetAvailableLane();

private:

    // duration limited due to BER counter size of 54b
    #define MAX_DURATION_48H  (uint)(172800)
    #define MIN_BER_THRESHOLD (double)(0.0)
    #define NOM_BER_THRESHOLD (double)(1e-9)
    #define MAX_BER_THRESHOLD (double)(1.0)

    const std::string MULTI_GT_TEST_PASS     = "PASS";
    const std::string MULTI_GT_TEST_FAIL     = "FAIL";
    const std::string MULTI_GT_TEST_DISABLE  = "DISABLE";

    const std::string REP = "%";

    const uint RST_WATCHDOG_INIT     = 5;

    const uint  CTRL_REG_ADDR               = 0x40;
    const uint  GT_LANE_0_CFG_REG_ADDR      = 0x41;
    const uint  GT_LANE_1_CFG_REG_ADDR      = 0x42;
    const uint  GT_LANE_2_CFG_REG_ADDR      = 0x43;
    const uint  GT_LANE_3_CFG_REG_ADDR      = 0x44;
    const std::array<uint,4> GT_LANES_CFG_REG_ADDR = {GT_LANE_0_CFG_REG_ADDR, GT_LANE_1_CFG_REG_ADDR, GT_LANE_2_CFG_REG_ADDR, GT_LANE_3_CFG_REG_ADDR};

    const uint LANES_CFG_LOOPBACK_OFFSET        = 29;
    const uint LANES_CFG_RX_USE_LPM_OFFSET      = 26;
    const uint LANES_CFG_RX_POLARITY_OFFSET     = 25;
    const uint LANES_CFG_TX_POLARITY_OFFSET     = 24;
    const uint LANES_CFG_TX_POST_EMPH_OFFSET    = 18;
    const uint LANES_CFG_TX_MAIN_CURSOR_OFFSET  = 11;
    const uint LANES_CFG_TX_PRE_EMPH_OFFSET     = 5;
    const uint LANES_CFG_TX_DIFFCTRL_OFFSET     = 0;

    const uint LANES_CFG_LOOPBACK_MASK        = 0x7;
    const uint LANES_CFG_RX_USE_LPM_MASK      = 0x1;
    const uint LANES_CFG_RX_POLARITY_MASK     = 0x1;
    const uint LANES_CFG_TX_POLARITY_MASK     = 0x1;
    const uint LANES_CFG_TX_POST_EMPH_MASK    = 0x3F;
    const uint LANES_CFG_TX_MAIN_CURSOR_MASK  = 0x7F;
    const uint LANES_CFG_TX_PRE_EMPH_MASK     = 0x3F;
    const uint LANES_CFG_TX_DIFFCTRL_MASK     = 0x1F;

    const uint  RESET_GT_ALL                = (0x1 << 0);
    const uint  RESET_GT_TX_PLL_DATAPATH    = (0x1 << 2);
    const uint  RESET_GT_RX_PLL_DATAPATH    = (0x1 << 3);
    const uint  TX_ENABLE                   = (0x1 << 4);
    const uint  RX_ENABLE                   = (0x1 << 5);
    const uint  RESET_GT_TX_DATAPATH        = (0x1 << 6);
    const uint  RESET_GT_RX_DATAPATH        = (0x1 << 7);
    const uint  CLEAR_STATUS_REGISTERS      = (0x1 << 8);
    const uint  LATCH_STATUS_REGISTERS      = (0x1 << 9);
    const uint  DISABLE_REF_PRBS            = (0x1 << 28);

    const uint GTWIZ_RESET_LIVE         = (0x1 << 10);
    const uint GTWIZ_RESET_FALLING      = (0x1 << 11);
    const uint GTWIZ_RESET_RISING       = (0x1 << 12);

    const uint TX_ERR_INJ_LN_0   = (0x1 << 24);
    const uint TX_ERR_INJ_LN_1   = (0x1 << 25);
    const uint TX_ERR_INJ_LN_2   = (0x1 << 26);
    const uint TX_ERR_INJ_LN_3   = (0x1 << 27);
    const std::array<uint,4> TX_ERR_INJ = {TX_ERR_INJ_LN_0, TX_ERR_INJ_LN_1, TX_ERR_INJ_LN_2, TX_ERR_INJ_LN_3};

    const uint TX_WORD_CNT_LN_0_REG_ADDR     = 0x48;
    const uint TX_WORD_CNT_LN_0_1_REG_ADDR   = 0x49;
    const uint TX_WORD_CNT_LN_1_REG_ADDR     = 0x4A;
    const uint TX_WORD_CNT_LN_2_REG_ADDR     = 0x4B;
    const uint TX_WORD_CNT_LN_2_3_REG_ADDR   = 0x4C;
    const uint TX_WORD_CNT_LN_3_REG_ADDR     = 0x4D;
    const uint STOPWATCH_REG_ADDR            = 0x4E;

    const uint RX_WORD_CNT_LSB_LN_0_REG_ADDR                        = 0x50;
    const uint RX_WORD_CNT_MSB_ERR_BIT_CNT_LSB_LN_0_REG_ADDR        = 0x51;
    const uint RX_ERR_BIT_CNT_MSB_LN_0_REG_ADDR                     = 0x52;
    const uint RX_WORD_CNT_LSB_LN_1_REG_ADDR                        = 0x53;
    const uint RX_WORD_CNT_MSB_ERR_BIT_CNT_LSB_LN_1_REG_ADDR        = 0x54;
    const uint RX_ERR_BIT_CNT_MSB_LN_1_REG_ADDR                     = 0x55;
    const uint RX_WORD_CNT_LSB_LN_2_REG_ADDR                        = 0x56;
    const uint RX_WORD_CNT_MSB_ERR_BIT_CNT_LSB_LN_2_REG_ADDR        = 0x57;
    const uint RX_ERR_BIT_CNT_MSB_LN_2_REG_ADDR                     = 0x58;
    const uint RX_WORD_CNT_LSB_LN_3_REG_ADDR                        = 0x59;
    const uint RX_WORD_CNT_MSB_ERR_BIT_CNT_LSB_LN_3_REG_ADDR        = 0x5A;
    const uint RX_ERR_BIT_CNT_MSB_LN_3_REG_ADDR                     = 0x5B;
    const uint HW_SEC_CNT_REG_ADDR                                  = 0x5C;
    const uint STAT_REG_ADDR                                        = 0x5D;

    // index after register re-ordering
    const uint RX_WORD_CNT_LN_0        = 0;
    const uint RX_ERR_BIT_CNT_LN_0     = 1;
    const uint RX_WORD_CNT_LN_1        = 2;
    const uint RX_ERR_BIT_CNT_LN_1     = 3;
    const uint RX_WORD_CNT_LN_2        = 4;
    const uint RX_ERR_BIT_CNT_LN_2     = 5;
    const uint RX_WORD_CNT_LN_3        = 6;
    const uint RX_ERR_BIT_CNT_LN_3     = 7;
    const uint TIMESTAMP               = 8;

    const uint SEC_CNT_MASK       = (0x7FFFFFFF);
    const uint TOGGLE_1_SEC       = (0x1 << 31);

    const uint GT_POWER_GOOD_LIVE       = (0x1 << 0);
    const uint GT_POWER_GOOD_FALLING    = (0x1 << 1);
    const uint GT_POWER_GOOD_RISING     = (0x1 << 2);
    const uint PLL_LOCK_LIVE            = (0x1 << 4);
    const uint PLL_LOCK_FALLING         = (0x1 << 5);
    const uint PLL_LOCK_RISING          = (0x1 << 6);

    const uint TX_RST_DONE_LIVE         = (0x1 << 8);
    const uint TX_RST_DONE_FALLING      = (0x1 << 9);
    const uint TX_RST_DONE_RISING       = (0x1 << 10);
    const uint RX_RST_DONE_LIVE         = (0x1 << 12);
    const uint RX_RST_DONE_FALLING      = (0x1 << 13);
    const uint RX_RST_DONE_RISING       = (0x1 << 14);

    const uint RX_PRBS_ERR_MASK         = (0xF << 20);
    const uint RX_PRBS_ERR_LN_0         = (0x1 << 20);
    const uint RX_PRBS_ERR_LN_1         = (0x1 << 21);
    const uint RX_PRBS_ERR_LN_2         = (0x1 << 22);
    const uint RX_PRBS_ERR_LN_3         = (0x1 << 23);
    const std::array<uint,4> RX_PRBS_ERR = {RX_PRBS_ERR_LN_0, RX_PRBS_ERR_LN_1, RX_PRBS_ERR_LN_2, RX_PRBS_ERR_LN_3};

    const uint RX_SEED_ZERO_MASK        = (0xF << 24);
    const uint RX_SEED_ZERO_LN_0        = (0x1 << 24);
    const uint RX_SEED_ZERO_LN_1        = (0x1 << 25);
    const uint RX_SEED_ZERO_LN_2        = (0x1 << 26);
    const uint RX_SEED_ZERO_LN_3        = (0x1 << 27);
    const std::array<uint,4> RX_SEED_ZERO = {RX_SEED_ZERO_LN_0, RX_SEED_ZERO_LN_1, RX_SEED_ZERO_LN_2, RX_SEED_ZERO_LN_3};

    const uint TX_SEED_ZERO_MASK        = (0xF << 28);
    const uint TX_SEED_ZERO_LN_0        = (0x1 << 28);
    const uint TX_SEED_ZERO_LN_1        = (0x1 << 29);
    const uint TX_SEED_ZERO_LN_2        = (0x1 << 30);
    const uint TX_SEED_ZERO_LN_3        = (0x1 << 31);
    const std::array<uint,4> TX_SEED_ZERO = {TX_SEED_ZERO_LN_0, TX_SEED_ZERO_LN_1, TX_SEED_ZERO_LN_2, TX_SEED_ZERO_LN_3};

    const uint RESET_STAT_REG_LN_0_ADDR = 0x62;
    const uint RESET_STAT_REG_LN_1_ADDR = 0x63;
    const uint RESET_STAT_REG_LN_2_ADDR = 0x64;
    const uint RESET_STAT_REG_LN_3_ADDR = 0x65;
    const std::array<uint,4> RESET_STAT_REG_ADDR = {RESET_STAT_REG_LN_0_ADDR, RESET_STAT_REG_LN_1_ADDR, RESET_STAT_REG_LN_2_ADDR, RESET_STAT_REG_LN_3_ADDR};

    const uint AP_CLK_RESET_STAT_REG_LN_0_ADDR = 0x66;
    const uint AP_CLK_RESET_STAT_REG_LN_1_ADDR = 0x67;
    const uint AP_CLK_RESET_STAT_REG_LN_2_ADDR = 0x68;
    const uint AP_CLK_RESET_STAT_REG_LN_3_ADDR = 0x69;
    const std::array<uint,4> AP_CLK_RESET_STAT_REG_ADDR = {AP_CLK_RESET_STAT_REG_LN_0_ADDR, AP_CLK_RESET_STAT_REG_LN_1_ADDR, AP_CLK_RESET_STAT_REG_LN_2_ADDR, AP_CLK_RESET_STAT_REG_LN_3_ADDR};

    const uint LANE_GT_RESET_ALL             = (0x1 << 0);
    const uint LANE_GT_RESET_TX_PLL_DATAPATH = (0x1 << 1);
    const uint LANE_GT_RESET_RX_PLL_DATAPATH = (0x1 << 2);
    const uint LANE_GT_RESET_TX_DATAPATH     = (0x1 << 3);
    const uint LANE_GT_RESET_RX_DATAPATH     = (0x1 << 4);

    const uint GTWIZ_TX_RST_DONE_LIVE        = (0x1 << 0);
    const uint GTWIZ_TX_RST_DONE_FALLING     = (0x1 << 1);
    const uint GTWIZ_TX_RST_DONE_RISING      = (0x1 << 2);
    const uint GTWIZ_CH_TX_RST_DONE_LIVE     = (0x1 << 3);
    const uint GTWIZ_CH_TX_RST_DONE_FALLING  = (0x1 << 4);
    const uint GTWIZ_CH_TX_RST_DONE_RISING   = (0x1 << 5);
    const uint GTWIZ_CH_TX_SYNC_DONE_LIVE    = (0x1 << 6);
    const uint GTWIZ_CH_TX_SYNC_DONE_FALLING = (0x1 << 7);
    const uint GTWIZ_CH_TX_SYNC_DONE_RISING  = (0x1 << 8);
    const uint GTWIZ_RX_RST_DONE_LIVE        = (0x1 << 9);
    const uint GTWIZ_RX_RST_DONE_FALLING     = (0x1 << 10);
    const uint GTWIZ_RX_RST_DONE_RISING      = (0x1 << 11);
    const uint GTWIZ_CH_RX_RST_DONE_LIVE     = (0x1 << 12);
    const uint GTWIZ_CH_RX_RST_DONE_FALLING  = (0x1 << 13);
    const uint GTWIZ_CH_RX_RST_DONE_RISING   = (0x1 << 14);
    const uint GTWIZ_CH_RX_SYNC_DONE_LIVE    = (0x1 << 15);
    const uint GTWIZ_CH_RX_SYNC_DONE_FALLING = (0x1 << 16);
    const uint GTWIZ_CH_RX_SYNC_DONE_RISING  = (0x1 << 17);
    const uint GTWIZ_TX_BUFFERBYPASS_LIVE    = (0x1 << 18);
    const uint GTWIZ_TX_BUFFERBYPASS_FALLING = (0x1 << 19);
    const uint GTWIZ_TX_BUFFERBYPASS_RISING  = (0x1 << 20);
    const uint GTWIZ_RX_BUFFERBYPASS_LIVE    = (0x1 << 21);
    const uint GTWIZ_RX_BUFFERBYPASS_FALLING = (0x1 << 22);
    const uint GTWIZ_RX_BUFFERBYPASS_RISING  = (0x1 << 23);
    const uint GTWIZ_LN_POWER_GOOD_LIVE      = (0x1 << 24);
    const uint GTWIZ_LN_POWER_GOOD_FALLING   = (0x1 << 25);
    const uint GTWIZ_LN_POWER_GOOD_RISING    = (0x1 << 26);

    const bool PLL_LOCK_EXPECTED = false;
    const bool PLL_UNSTABLE      = true;

    const uint GTF_DATA_SIZE_BITS = 32;
    const double RATE_10GBE = 10.3125;
    const double RATE_10GBE_PRECISION = 0.5; // 0.5%, 10GbE tolerance is +/- 200ppm, which is 0.02%
    const double RATE_10GBE_LOW_THRESHOLD = RATE_10GBE * (1 - (RATE_10GBE_PRECISION / 100) );
    const double RATE_10GBE_HIGH_THRESHOLD = RATE_10GBE * (1 + (RATE_10GBE_PRECISION / 100) );

    const uint GTM_DATA_SIZE_BITS = 128;
    const double RATE_56GBE = 56.42;
    const double RATE_56GBE_PRECISION = 0.5; // 0.5%, 56GbE tolerance is +/- 200ppm, which is 0.02%
    const double RATE_56GBE_LOW_THRESHOLD = RATE_56GBE * (1 - (RATE_56GBE_PRECISION / 100) );
    const double RATE_56GBE_HIGH_THRESHOLD = RATE_56GBE * (1 + (RATE_56GBE_PRECISION / 100) );

    const uint GTYP_DATA_SIZE_BITS = 66;
    const double RATE_32GBS = 32.00;
    const double RATE_32GBS_PRECISION = 0.5; // 0.5%,
    const double RATE_32GBS_LOW_THRESHOLD = RATE_32GBS * (1 - (RATE_32GBS_PRECISION / 100) );
    const double RATE_32GBS_HIGH_THRESHOLD = RATE_32GBS * (1 + (RATE_32GBS_PRECISION / 100) );

    using LaneParam_t = struct LaneParam_t {
        std::string gt_settings;
        std::string gt_lpbk;
        bool        disable_ref_prbs;
        double      ber_threshold;
        uint32_t    gt_tx_diffctrl;
        uint32_t    gt_tx_main_cursor;
        uint32_t    gt_tx_pre_emph;
        uint32_t    gt_tx_post_emph;
        bool        gt_rx_use_lpm;
        std::string tx_polarity;
        std::string rx_polarity;
        bool        enable;
    };
    const LaneParam_t RSTC_LANEPARAM = {
        .gt_settings            = "",
        .gt_lpbk                = GT_LPBK_DISABLE,
        .disable_ref_prbs       = false,
        .ber_threshold          = NOM_BER_THRESHOLD,
        .gt_tx_diffctrl         = 0,
        .gt_tx_main_cursor      = DEFAULT_GT_TX_MAIN_CURSOR,
        .gt_tx_pre_emph         = 0,
        .gt_tx_post_emph        = 0,
        .gt_rx_use_lpm          = false,
        .tx_polarity            = TX_POLARITY_NORMAL,
        .rx_polarity            = RX_POLARITY_NORMAL,
        .enable                 = true,
    };

    using MultiGTPrbsTestPRBSTestcaseCfg_t = struct MultiGTPRBSTestcaseCfg_t {
        LaneParam_t                 global_param;
        std::map<uint, LaneParam_t> lane_param;
    };

    using Cmd_t = enum Cmd_t {
        RUN,
        CLEAR_STATUS,
        CHECK_STATUS,
        CONFIG_GT,
        TX_RX_RST,
        TX_RST,
        RX_RST,
        TX_DATAPATH_RST,
        RX_DATAPATH_RST,
        INSERT_ERROR
    };

    using MultiGTPrbsTestItConfig_t = struct MultiGTPrbsTestItConfig_t {
        uint                        duration;
        Cmd_t                       cmd;
        std::string                 cmd_str;
        std::vector<std::string>    test_strvect;
        std::vector<std::string>    test_strvect_dbg;
        uint                        insert_error_lane_idx;
    };

    using RateSelection_t = struct RateSelection_t {
        uint    data_size;
        double  data_rate;
        double  precision;
        double  low_threshold;
        double  high_threshold;
    };

    using Rx_Status_t = struct Rx_Status_t {
        std::array<uint64_t,4>  word_cnt;
        std::array<uint64_t,4>  bit_error_cnt;
        bool                    toggle_1_sec;
        uint32_t                timestamp_1_sec;
    };
    const Rx_Status_t RESET_CHAN_STATUS = {
        .word_cnt           = {0,0,0,0},
        .bit_error_cnt      = {0,0,0,0},
        .toggle_1_sec       = false,
        .timestamp_1_sec    = 0
    };

    using Rx_Lane_Status_t = struct Rx_Lane_Status_t {
        uint32_t            sample_cnt;
        double              bit_cnt;
        double              bit_error_cnt;
        double              acc_bit_cnt;
        double              acc_bit_error_cnt;
        bool                result;
        std::string         result_str;
        double              rate;
        double              ber;
    };
    const Rx_Lane_Status_t RESET_RX_LANE_STATUS = {
        .sample_cnt         = 0,
        .bit_cnt            = 0.0,
        .bit_error_cnt      = 0.0,
        .acc_bit_cnt        = 0.0,
        .acc_bit_error_cnt  = 0.0,
        .result             = RET_SUCCESS,
        .result_str         = MULTI_GT_TEST_PASS,
        .rate               = 0.0,
        .ber                = 0.0
    };

    std::array<Rx_Lane_Status_t,4> m_rx_lane_status = {RESET_RX_LANE_STATUS, RESET_RX_LANE_STATUS, RESET_RX_LANE_STATUS, RESET_RX_LANE_STATUS};

    using Tx_Status_t = struct Tx_Status_t {
        std::array<uint64_t,4>  word_cnt;
        uint32_t                stopwatch;
    };
    const Tx_Status_t RESET_TX_STATUS = {
        .word_cnt       = {0,0,0,0},
        .stopwatch      = 0
    };

    DeviceMgt *                                             m_devicemgt = nullptr;
    Gt_Parameters_t                                         m_test_parameters;
    uint                                                    m_cu_idx;
    uint                                                    m_gt_index;
    Multi_GT_t                                              m_multi_gt_type;
    std::string                                             m_multi_gt_name;
    std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>     m_test_sequence;
    MultiGTPRBSTestcaseCfg_t                                m_TC_Cfg;
    std::list<MultiGTPrbsTestItConfig_t>                    m_test_it_list;

    std::string                                             m_outputfile_name;
    bool                                                    m_use_outputfile = false;
    std::ofstream                                           m_outputfile;
    std::ofstream                                           m_RT_outputfile;
    std::string                                             m_RT_outputfile_name;
    std::string                                             m_RT_outputfile_head;

    uint                                            m_gt_num_lane;
    std::vector<uint>                               m_available_lane;
    std::vector<uint>                               m_enable_lane;
    RateSelection_t                                 m_rate;
    Rx_Status_t                                     m_rx_status;
    Tx_Status_t                                     m_tx_status;
    bool                                            m_tog_1sec = false;
    bool                                            m_tog_1sec_last = true;
    uint32_t                                        m_hw_sec_cnt = 0;
    uint32_t                                        m_hw_sec_cnt_expected = 0;
    uint64_t                                        m_sw_sec_cnt;
    uint64_t                                        m_sw_sec_cnt_start;

    uint64_t                                        m_buffer_size;
    uint                                            m_toggle_error_cnt = 0;

    bool    WriteMultiGtPrbsCu              ( const uint & address, const uint & value);
    bool    ReadMultiGtPrbsCu               ( const uint & address, uint & read_data );
    bool    ReadMultiGtPrbsStatus           ( uint32_t * read_buffer, const uint64_t & size );
    bool    ResetGT  ();
    bool    ResetGTTx();
    bool    ResetGTTxDatapath();
    bool    ResetGTTxDatapathLane(uint lane_idx);
    bool    ResetGTRx();
    bool    ResetGTRxDatapath();
    bool    ResetGTRxDatapathLane(uint lane_idx);
    bool    CheckGTPowerGood( std::map<uint, bool> & lane_failure );
    bool    CheckAllResetAndSyncDoneHigh();
    bool    CheckResetRequest();
    bool    CheckResetOver(std::map<uint, bool> & lane_failure);
    bool    CheckGTFTxResetRequestLane(uint lane_idx);
    bool    CheckTxResetRequest();
    bool    CheckRxResetRequestLane(uint lane_idx);
    bool    CheckRxResetRequest();
    bool    CheckTxResetOver( std::map<uint, bool> & lane_failure);
    bool    CheckTxResetStable( std::map<uint, bool> & lane_failure);
    bool    CheckRxResetOver( std::map<uint, bool> & lane_failure);
    bool    CheckRxResetStable( std::map<uint, bool> & lane_failure);
    bool    CheckPllLock( std::map<uint, bool> & lane_failure, const bool & expect );
    void    SendGTCfg();
    bool    SetAllLaneFailure( std::map<uint, bool> & lane_failure );
    int     RunTest() override;
    bool    StartCU() override;
    bool    EnableWatchdogClkThrotDetection() override;
    bool    StopCU() override;
    bool    CheckWatchdogAndClkThrottlingAlarms() override;
    bool    GetTestSequence();
    bool    ParseTestSequenceSettings   ( std::list<MultiGTPrbsTestItConfig_t> & test_list );
    void    PrintLaneParam              ( const LaneParam_t & lane_param );

    void    ClearStatus();
    void    LatchStatus();
    double  ComputeRate(uint64_t word_count, uint duration);
    void    ComputeVariationRates(uint64_t word_count, uint32_t duration, std::map<uint, double> & rates);
    bool    SelectRate(std::map<uint, double> & rates, uint & duration_sel, double & rate_sel);
    bool    CheckLaneStatus( std::map<uint, bool> & lane_failure);
    void    CheckTimestamp1Sec();
    void    WaitHw1SecToggle();
    void    GetHw1SecToggle(bool & toggle_1_sec, uint32_t & timestamp_1_sec);
    void    ReadTxStatus();
    void    ReadRxStatus();
    void    ComputeRxStatus();
    void    EnableTXPrbs();
    void    DisableTXPrbs();
    void    EnableRXPrbsCheck();
    void    DisableRXPrbsCheck();
    void    InsertError(uint lane_idx);

    void    WriteToMeasurementFile();

};

} // namespace

#endif /* _MULTIGTPRBSTEST_H */
