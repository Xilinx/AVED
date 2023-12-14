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

#ifndef _GTMACPACKAGE_H
#define _GTMACPACKAGE_H

#include "xqueue.h"
#include "semaphore.h"

namespace xbtest
{

using MAC_Addr_t = struct MAC_Addr_t {
    uint64_t    addr;
    bool        valid;
};
#define RST_MAC_ADDR MAC_Addr_t({ \
    .addr   = 0, \
    .valid  = false \
}) \

using GT_MAC_Addr_t = std::map<uint, MAC_Addr_t>;

using gt_mapping_t = struct gt_mapping_t {
    bool            enable;
    int32_t         cfg_source;
    uint32_t        paired_gt;
};
#define RST_GT_MAPPING gt_mapping_t({ \
    .enable     = false, \
    .cfg_source = 0, \
    .paired_gt  = 0 \
}) \

using LaneParam_t = struct LaneParam_t {
    bool            disable_lane;
    bool            Src_MAC_Addr_valid;
    uint64_t        Src_MAC_Addr;
    bool            Dest_MAC_Addr_valid;
    uint64_t        Dest_MAC_Addr;
    std::string     lane_rate;
    uint32_t        utilisation;
    uint32_t        mac_to_mac_connection;
    std::string     fec_mode;
    std::string     traffic_type;
    uint32_t        packet_size;
    std::string     packet_cfg;
    gt_mapping_t    gt_mapping;
    uint32_t        tx_mapping;
    uint32_t        rx_mapping;
    bool            match_tx_rx;
    std::string     gt_settings;
    uint32_t        gt_tx_diffctrl;
    uint32_t        gt_tx_main_cursor;
    uint32_t        gt_tx_pre_emph;
    uint32_t        gt_tx_post_emph;
    bool            gt_rx_use_lpm;
    std::string     tx_polarity;
    std::string     dest_mac_addr;
    std::string     src_mac_addr;
};
#define RST_LANEPARAM LaneParam_t({ \
    .disable_lane           = false, \
    .Src_MAC_Addr_valid     = false, \
    .Src_MAC_Addr           = 0, \
    .Dest_MAC_Addr_valid    = false, \
    .Dest_MAC_Addr          = 0, \
    .lane_rate              = "", \
    .utilisation            = 0, \
    .mac_to_mac_connection  = 0, \
    .fec_mode               = "", \
    .traffic_type           = "", \
    .packet_size            = 0, \
    .packet_cfg             = "", \
    .gt_mapping             = RST_GT_MAPPING, \
    .tx_mapping             = 0, \
    .rx_mapping             = 0, \
    .match_tx_rx            = false, \
    .gt_settings            = "", \
    .gt_tx_diffctrl         = 0, \
    .gt_tx_main_cursor      = 0, \
    .gt_tx_pre_emph         = 0, \
    .gt_tx_post_emph        = 0, \
    .gt_rx_use_lpm          = false, \
    .tx_polarity            = "", \
    .dest_mac_addr          = "", \
    .src_mac_addr           = "" \
}) \

using GTMACTestcaseCfg_t = struct GTMACTestcaseCfg_t {
    bool                                            exist;
    LaneParam_t                                     global_param;
    std::map<uint, LaneParam_t>                     lane_param;
    std::vector<GTMAC_Test_Sequence_Parameters_t>   test_sequence;
};

using MACCmd_t = enum MACCmd_t {
    CHECK_STATUS,
    CLEAR_STATUS,
    CONFIG_SEND,
    CONFIG_10GBE,
    CONFIG_10GBE_C74_FEC,
    CONFIG_25GBE,
    CONFIG_25GBE_C74_FEC,
    CONFIG_25GBE_RS_FEC,
    CU_RUN,
    CU_STOP,
    TX_RST,
    RX_RST,
    TX_RX_RST
};

using TestItConfig_t = struct TestItConfig_t {
    int                         target_GT_MAC;
    uint                        duration;
    MACCmd_t                    mac_cmd;
    std::string                 mac_cmd_str;
    int                         Speed;
    std::vector<std::string>    test_strvect;
    std::vector<std::string>    test_strvect_dbg;
};

using Traffic_Type_t = enum Traffic_Type_t {
    TT_ZERO = 0,
    TT_CNT  = 1,
    TT_4CYC = 2,
    TT_FF   = 3
};

using MAC_Config_t = struct MAC_Config_t {
    uint64_t        Dest_Addr;
    uint64_t        Source_Addr;
    uint32_t        EtherType;
    Traffic_Type_t  Traffic_Type;
    uint32_t        Script_Base;
    uint32_t        Script_Length;
    float           Utilisation;
    uint32_t        MTU;
    uint32_t        GT_Tx_Main_Cursor;  // Range 0..127, default = 80 control = 'gt_tx_main_cursor' (DEFAULT_GT_TX_MAIN_CURSOR)
    uint32_t        GT_Tx_Diff;         // Range 0..31,  default = 0  control = 'gt_tx_diffctrl'
    uint32_t        GT_Tx_Pre;          // Range 0..31,  default = 0  control = 'gt_tx_pre_emph'
    uint32_t        GT_Tx_Post;         // Range 0..31,  default = 0  control = 'gt_tx_post_emph'

    bool            Active;
    bool            Set_LFI;
    bool            Set_RFI;
    bool            Set_Idle;
    bool            Lcl_Loopback;
    bool            Match_Tx_Rx;
    bool            GT_Rx_Eq_Sel;   // false = DFE, true = LPM, default = DFE   control = 'gt_rx_use_lpm'
    bool            tx_polarity;
};
#define RST_MAC_CONFIG MAC_Config_t({ \
    .Dest_Addr          = 0, \
    .Source_Addr        = 0, \
    .EtherType          = 0, \
    .Traffic_Type       = TT_ZERO, \
    .Script_Base        = 0, \
    .Script_Length      = 0, \
    .Utilisation        = 0.0, \
    .MTU                = 0, \
    .GT_Tx_Main_Cursor  = DEFAULT_GT_TX_MAIN_CURSOR, \
    .GT_Tx_Diff         = 0, \
    .GT_Tx_Pre          = 0, \
    .GT_Tx_Post         = 0, \
    .Active             = false, \
    .Set_LFI            = false, \
    .Set_RFI            = false, \
    .Set_Idle           = false, \
    .Lcl_Loopback       = false, \
    .Match_Tx_Rx        = false, \
    .GT_Rx_Eq_Sel       = false, \
    .tx_polarity        = false \
}) \

// 48bits MAC bytes counter will saturates after ~62h, set limit to 60h
#define MAX_DURATION_10GBE  (uint)(216000)
// 48bits MAC bytes counter will saturates after ~25h, set limit to 24h
#define MAX_DURATION_25GBE  (uint)(86400)

// JSON parameters
#define LANE_RATE_10GBE     std::string("10gbe")
#define LANE_RATE_25GBE     std::string("25gbe")
#define SUPPORTED_LANE_RATE std::set<std::string>({ \
    LANE_RATE_10GBE, \
    LANE_RATE_25GBE \
}) \

#define MIN_UTILISATION (uint32_t)(0)
#define NOM_UTILISATION (uint32_t)(50)
#define MAX_UTILISATION (uint32_t)(100)

#define FEC_MODE_NONE       std::string("none")
#define FEC_MODE_CLAUSE_74  std::string("clause_74")
#define FEC_MODE_RS_FEC     std::string("rs_fec")
#define SUPPORTED_FEC_MODE  std::set<std::string>({ \
    FEC_MODE_NONE, \
    FEC_MODE_CLAUSE_74, \
    FEC_MODE_RS_FEC \
}) \

#define TRAFFIC_TYPE_0X00       std::string("0x00")
#define TRAFFIC_TYPE_0XFF       std::string("0xff")
#define TRAFFIC_TYPE_COUNT      std::string("count")
#define TRAFFIC_TYPE_PATTERN    std::string("pattern")
#define SUPPORTED_TRAFFIC_TYPE std::set<std::string>({ \
    TRAFFIC_TYPE_0X00, \
    TRAFFIC_TYPE_0XFF, \
    TRAFFIC_TYPE_COUNT, \
    TRAFFIC_TYPE_PATTERN \
}) \

#define PACKET_CFG_SWEEP        std::string("sweep")
#define PACKET_CFG_NO_SWEEP     std::string("no_sweep")

#define NOM_PACKET_SIZE         (uint32_t)(1024)

#define SMALL_PACKET_SIZE_MIN   (uint32_t)(64)
#define SMALL_PACKET_SIZE_MAX   (uint32_t)(1535)
#define BIG_PACKET_SIZE_MIN     (uint32_t)(9000)
#define BIG_PACKET_SIZE_MAX     (uint32_t)(10000)

#define MIN_PRBS_ERROR_THRESHOLD (uint32_t)(0)
#define NOM_PRBS_ERROR_THRESHOLD (uint32_t)(5)
#define MAX_PRBS_ERROR_THRESHOLD (uint32_t)(100)

#define MIN_GT_TX_DIFFCTRL      (uint32_t)(0)
#define MAX_GT_TX_DIFFCTRL      (uint32_t)(31)

#define MIN_GT_TX_MAIN_CURSOR       (uint32_t)(0)
#define MAX_GT_TX_MAIN_CURSOR       (uint32_t)(127)

// 0x50 from XXV example design trans_debug.v file
#define DEFAULT_GT_TX_MAIN_CURSOR   (uint32_t)(0)

#define MIN_GT_TX_PRE_EMPH      (uint32_t)(0)
#define MAX_GT_TX_PRE_EMPH      (uint32_t)(31)

#define MIN_GT_TX_POST_EMPH     (uint32_t)(0)
#define MAX_GT_TX_POST_EMPH     (uint32_t)(31)

#define RX_EQUALISER_DFM_EN     false
#define RX_EQUALISER_LPM_EN     true

#define RX_EQUALISER_LPM        std::string("lpm")

#define RX_POLARITY_NORMAL          std::string("normal")
#define RX_POLARITY_INVERTED        std::string("inverted")
#define SUPPORTED_RX_POLARITY_TYPE  std::set<std::string>({ \
    RX_POLARITY_NORMAL, \
    RX_POLARITY_INVERTED \
}) \

#define TX_POLARITY_NORMAL          std::string("normal")
#define TX_POLARITY_INVERTED        std::string("inverted")
#define SUPPORTED_TX_POLARITY_TYPE  std::set<std::string>({ \
    TX_POLARITY_NORMAL, \
    TX_POLARITY_INVERTED \
}) \

#define GT_SETTINGS_MODULE      std::string("module")
#define GT_SETTINGS_CABLE       std::string("cable")
#define SUPPORTED_GT_SETTINGS   std::set<std::string>({ \
    GT_SETTINGS_MODULE, \
    GT_SETTINGS_CABLE \
}) \


#define GT_LPBK_DISABLE         std::string("disable")
#define GT_LPBK_NEAR_END_PCS    std::string("near end pcs")
#define GT_LPBK_NEAR_END_PMA    std::string("near end pma")
// far end can't be used as UG578 mentions for both far end loopback mode the gearbox can't be used
#define GT_LPBK_FAR_END_PCS     std::string("far end pcs")
#define GT_LPBK_FAR_END_PMA     std::string("far end pma")
// FAR END are not tested
#define SUPPORTED_GT_LPBK_TYPE  std::set<std::string>({ \
    GT_LPBK_DISABLE, \
    GT_LPBK_NEAR_END_PCS, \
    GT_LPBK_NEAR_END_PMA \
})

#define GT_LPBK_DISABLE_CFG      (uint)(0)
#define GT_LPBK_NEAR_END_PCS_CFG (uint)(1)
#define GT_LPBK_NEAR_END_PMA_CFG (uint)(2)
#define GT_LPBK_FAR_END_PCS_CFG  (uint)(4)
#define GT_LPBK_FAR_END_PMA_CFG  (uint)(6)


//MAC addresses beginning 02, 06, 0A or 0E are locally administered.
#define DEFAULT_TEST_MAC_ADDR   (uint64_t)(0x06bbccddee00)
#define ALVEO_MAC_ADDR_POOL_1   (uint64_t)(0x000a35000000)
#define ALVEO_MAC_ADDR_POOL_2   (uint64_t)(0x005d03000000)
#define BROADCAST_MAC_ADDR      (uint64_t)(0xffffffffffff)
#define INVALID_MAC_ADDR_POOL_1 (uint64_t)(0x010203040500)

#define NO_MAC_ADDR_OVERWRITE   std::string("no_mac_addr_overwrite")
#define TEST_MAC_ADDR           std::string("test_address")
#define ALVEO_RDM_MAC_ADDR      std::string("alveo_random_address")

#define SELF_TRAFFIC_SOURCE     (int32_t)(-1)
#define NO_VALID_MAC_ADDRESS    (int64_t)(0)

inline std::string GetHeader2( const uint & lane_idx )
{
    return "Lane[" + std::to_string(lane_idx) + "]";
}

#define GTMACMSG_HEADER_STRING      (uint)(0)
#define GTMACMSG_HEADER_TEST_IT_CNT (uint)(1)
#define GTMACMSG_HEADER_MAC_STATUS  (uint)(2)
#define GTMACMSG_CFG_DONE           std::string("cfg done")

using TrafficCounters_t = struct TrafficCounters_t {
    uint64_t    Good_Packet;
    uint64_t    Good_Byte;
    uint64_t    Total_Packet;
    uint64_t    Bad_Dest_Addr;
    uint64_t    Last_Bad_Dest_Addr;
    uint64_t    Bad_Src_Addr;
    uint64_t    Last_Bad_Src_Addr;
    uint64_t    Freq_Cnt;
};
#define RST_TRAFFICCOUNTERS TrafficCounters_t({ \
    .Good_Packet        = 0, \
    .Good_Byte          = 0, \
    .Total_Packet       = 0, \
    .Bad_Dest_Addr      = 0, \
    .Last_Bad_Dest_Addr = 0, \
    .Bad_Src_Addr       = 0, \
    .Last_Bad_Src_Addr  = 0, \
    .Freq_Cnt           = 0 \
}) \


using MACTrafficCnt_t = std::map<uint, TrafficCounters_t>;

using TestItCnt_t = struct TestItCnt_t {
    bool    gt_pair_en;
    int32_t GT_paired;
    uint    current_cnt;
    uint    received_cnt;
};
#define RST_TESTITCNT TestItCnt_t({ \
    .gt_pair_en     = false, \
    .GT_paired      = SELF_TRAFFIC_SOURCE, \
    .current_cnt    = 0, \
    .received_cnt   = 0 \
}) \

using MACStatusCnt_t = struct MACStatusCnt_t {
    int32_t             GT_paired;
    MACStatus_t         status;
    bool                new_status;
};

} // namespace

#endif /* _GTMACPACKAGE_H */
