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

#ifndef _XBTESTSWPACKAGE_H
#define _XBTESTSWPACKAGE_H

#include "xbtestpackage.h"

namespace xbtest
{

#define XILINX_CL_PLATFORM_NAME std::string("Xilinx")

#define DMA_SW_VERSION_MAJOR                    (uint)(2)
#define DMA_SW_VERSION_MINOR                    (uint)(2)
#define P2P_CARD_SW_VERSION_MAJOR               (uint)(1)
#define P2P_CARD_SW_VERSION_MINOR               (uint)(0)
#define P2P_NVME_SW_VERSION_MAJOR               (uint)(1)
#define P2P_NVME_SW_VERSION_MINOR               (uint)(0)
#define PWR_SW_VERSION_MAJOR                    (uint)(3)
#define PWR_SW_VERSION_MINOR                    (uint)(0)
#define MEM_SW_VERSION_MAJOR                    (uint)(3)
#define MEM_SW_VERSION_MINOR                    (uint)(0)
#define GT_MAC_SW_VERSION_MAJOR                 (uint)(3)
#define GT_MAC_SW_VERSION_MINOR                 (uint)(0)
#define GT_LPBK_SW_VERSION_MAJOR                (uint)(1)
#define GT_LPBK_SW_VERSION_MINOR                (uint)(0)
#define GT_PRBS_SW_VERSION_MAJOR                (uint)(1)
#define GT_PRBS_SW_VERSION_MINOR                (uint)(0)
#define GTF_PRBS_SW_VERSION_MAJOR               (uint)(1)
#define GTF_PRBS_SW_VERSION_MINOR               (uint)(0)
#define GTM_PRBS_SW_VERSION_MAJOR               (uint)(1)
#define GTM_PRBS_SW_VERSION_MINOR               (uint)(0)
#define GTYP_PRBS_SW_VERSION_MAJOR              (uint)(1)
#define GTYP_PRBS_SW_VERSION_MINOR              (uint)(0)
#define VERIFY_SW_VERSION_MAJOR                 (uint)(1)
#define VERIFY_SW_VERSION_MINOR                 (uint)(0)

// value keep for detection of old hw/sw mismatch
// BEGIN ... do not remove
#define MIN_PWR_HW_VERSION_MAJOR                (uint)(1)
#define MIN_PWR_HW_VERSION_MINOR                (uint)(5)
#define MIN_MEM_HW_VERSION_MAJOR                (uint)(1)
#define MIN_MEM_HW_VERSION_MINOR                (uint)(4)
#define MIN_GT_MAC_HW_VERSION_MAJOR             (uint)(1)
#define MIN_GT_MAC_HW_VERSION_MINOR             (uint)(5)
#define MIN_GT_LPBK_HW_VERSION_MAJOR            (uint)(1)
#define MIN_GT_LPBK_HW_VERSION_MINOR            (uint)(0)
#define MIN_GT_PRBS_HW_VERSION_MAJOR            (uint)(1)
#define MIN_GT_PRBS_HW_VERSION_MINOR            (uint)(0)
#define MIN_GTF_PRBS_HW_VERSION_MAJOR           (uint)(1)
#define MIN_GTF_PRBS_HW_VERSION_MINOR           (uint)(0)
#define MIN_GTM_PRBS_HW_VERSION_MAJOR           (uint)(1)
#define MIN_GTM_PRBS_HW_VERSION_MINOR           (uint)(0)
#define MIN_GTYP_PRBS_HW_VERSION_MAJOR          (uint)(1)
#define MIN_GTYP_PRBS_HW_VERSION_MINOR          (uint)(0)
#define MIN_VERIFY_HW_VERSION_MAJOR             (uint)(1)
#define MIN_VERIFY_HW_VERSION_MINOR             (uint)(0)
// END ... do not remove

// component ID present in the Build info
#define BI_PWR_HW_COMPONENT_ID                  (uint)(0)
#define BI_MEM_HW_COMPONENT_ID                  (uint)(1)
#define BI_GT_MAC_HW_COMPONENT_ID               (uint)(4)
#define BI_GT_LPBK_HW_COMPONENT_ID              (uint)(3)
#define BI_VERIFY_HW_COMPONENT_ID               (uint)(5)
#define BI_GT_PRBS_HW_COMPONENT_ID              (uint)(6)
#define BI_GTF_PRBS_HW_COMPONENT_ID             (uint)(7)
#define BI_GTM_PRBS_HW_COMPONENT_ID             (uint)(8)
#define BI_GTYP_PRBS_HW_COMPONENT_ID            (uint)(9)

#define BI_MEM_TYPE_SINGLE_CHANNEL              (uint)(1)
#define BI_MEM_TYPE_MULTI_CHANNEL               (uint)(2)

#define BI_MAJOR_MINOR_VERSION_ADDR             (uint)(0x0000)
#define BI_BUILD_VERSION_ADDR                   (uint)(0x0001)
#define BI_COMPONENT_ID_ADDR                    (uint)(0x0002)
#define BI_RESERVED                             (uint)(0x0003)
#define BI_INFO_1_2_ADDR                        (uint)(0x0004)
#define BI_INFO_3_4_ADDR                        (uint)(0x0005)
#define BI_INFO_5_6_ADDR                        (uint)(0x0006)
#define BI_INFO_7_8_ADDR                        (uint)(0x0007)

#define CMN_SCRATCH_PAD_ADDR                    (uint)(0x0010)
#define CMN_RESET_DETECTION_ADDR                (uint)(0x0011)
#define CMN_CTRL_STATUS_ADDR                    (uint)(0x0012)
#define CMN_WATCHDOG_THROTTLE_ADDR              (uint)(0x0013)
#define CMN_STATUS_LIVE_ADDR                    (uint)(0x0014)
#define CMN_STATUS_LATCH_ADDR                   (uint)(0x0015)

#define CMN_STATUS_START                        (uint)((0x1 << 0))
#define CMN_STATUS_ALREADY_START                (uint)((0x1 << 4))

#define CMN_WATCHDOG_EN                         (uint)((0x1 << 4))
#define CMN_WATCHDOG_16SEC                      (uint)((0x0 << 5))
#define CMN_WATCHDOG_32SEC                      (uint)((0x1 << 5))
#define CMN_WATCHDOG_64SEC                      (uint)((0x2 << 5))
#define CMN_WATCHDOG_128SEC                     (uint)((0x3 << 5))
#define CMN_WATCHDOG_ALARM                      (uint)((0x1 << 8))
#define CMN_WATCHDOG_RST                        (uint)((0x1 << 12))

#define CMN_THROTTLE_ALARM_RST                  (uint)((0x1 << 16))
#define CMN_THROTTLE_APCLK_AVAIL                (uint)((0x1 << 17))
#define CMN_THROTTLE_APCLK_DETECT               (uint)((0x1 << 18))
#define CMN_THROTTLE_APCLK2_AVAIL               (uint)((0x1 << 19))
#define CMN_THROTTLE_APCLK2_DETECT              (uint)((0x1 << 20))
#define CMN_THROTTLE_MON_ENABLE                 (uint)((0x1 << 21))

// division factor of watchdog duration
#define WATCHDOG_RESET_RATE                     (uint)(2)
// default reset rate in second
#define WATCHDOG_RESET                          (uint)(8)

#define PWR_THROTTLE_MODE_INTERNAL_MACRO_STR    std::string("INTERNAL_MACRO")
#define PWR_THROTTLE_MODE_INTERNAL_CLK_STR      std::string("INTERNAL_CLK")
#define PWR_THROTTLE_MODE_EXTERNAL_MACRO_STR    std::string("EXTERNAL_MACRO")
#define PWR_THROTTLE_MODE_EXTERNAL_CLK_STR      std::string("EXTERNAL_CLK")

#define PWR_THROTTLE_MODE_INTERNAL_MACRO        (uint)(0)
#define PWR_THROTTLE_MODE_INTERNAL_CLK          (uint)(1)
#define PWR_THROTTLE_MODE_EXTERNAL_MACRO        (uint)(2)
#define PWR_THROTTLE_MODE_EXTERNAL_CLK          (uint)(3)

#define CU_WRAPPER_IDX                          (int)(-1)
#define GT_LANE_INDEXES                         std::vector<uint>({0, 1, 2, 3})
#define MAX_GT_SUPPORTED                        (uint)(32)

#define GT_RATE_SWITCH_10GBE_25GBE              (uint)(0)
#define GT_RATE_FIX_10GBE                       (uint)(1)
#define GT_RATE_FIX_25GBE                       (uint)(2)
#define GT_RATE_FIX_56GBE                       (uint)(3)
#define GT_RATE_FIX_32GBS_AURORA                (uint)(4)


#define GT_IP_GTY_WIZARD                       (uint)(0)
#define GT_IP_SUBSYSTEM_GT                     (uint)(1)
#define GT_IP_GTF_WIZARD                       (uint)(2)


// Define structure containing parsed JSON parameters
//TestcaseParamBase is a base structure that will enable to build a std::map with different parameter types
class TestcaseParamBase
{
public:
    virtual ~TestcaseParamBase() = 0;
};
inline TestcaseParamBase::~TestcaseParamBase() = default;
// TestcaseParam will be use to cast TestcaseParamBase to the correct type of JSON parameter
template<class T>
class TestcaseParam : public TestcaseParamBase
{
public:
    explicit TestcaseParam( const T & data ) : data(data) {}
    TestcaseParam() = default;
    T data;
};

template<typename T> inline T TestcaseParamCast ( TestcaseParamBase * teacase_param )
{
    return dynamic_cast<TestcaseParam<T>&>(*teacase_param).data;
}

using Json_Parameter_t  = std::pair<std::string, TestcaseParamBase*>;
using Json_Parameters_t = std::map<std::string, TestcaseParamBase*>;

using Tasks_Type_Parameters_t = struct Tasks_Type_Parameters_t {
    bool                exists;
    Json_Parameters_t   param;
};

using Sensor_Limit_t = struct Sensor_Limit_t {
    double  min;
    double  max;
    bool    min_exists;
    bool    max_exists;
    bool    value_out_range;
};
#define RST_SENSOR_LIMIT Sensor_Limit_t({ \
    .min                = 0.0, \
    .max                = 0.0, \
    .min_exists         = false, \
    .max_exists         = false, \
    .value_out_range    = false \
})

using Device_Mgmt_Sensor_t = struct Device_Mgmt_Sensor_t {
    std::string     id;
    std::string     type;
    Sensor_Limit_t  warning_threshold;
    Sensor_Limit_t  error_threshold;
    Sensor_Limit_t  abort_threshold;
};
#define RST_DEVICE_MGMT_SENSOR Device_Mgmt_Sensor_t({ \
    .id                 = "", \
    .type               = "", \
    .warning_threshold  = RST_SENSOR_LIMIT, \
    .error_threshold    = RST_SENSOR_LIMIT, \
    .abort_threshold    = RST_SENSOR_LIMIT \
}) \

using Device_Mgmt_Type_Parameters_t = struct Device_Mgmt_Type_Parameters_t {
    bool                                exists;
    Json_Parameters_t                   param;
    std::vector<Device_Mgmt_Sensor_t>   sensor;
};

using Tasks_Parameters_t = struct Tasks_Parameters_t {
    Tasks_Type_Parameters_t         timer;
    Device_Mgmt_Type_Parameters_t   device_mgmt;
    Tasks_Type_Parameters_t         debug;
    Tasks_Type_Parameters_t         gtmacmgmt;
};

using Mmio_Parameters_t = struct Mmio_Parameters_t {
    bool                exists;
    Json_Parameters_t   global_config;
};

using Dma_Type_Parameters_t = struct Dma_Type_Parameters_t {
    bool                exists;
    Json_Parameters_t   param;
};

using Dma_Parameters_t = struct Dma_Parameters_t {
    bool                                        exists;
    Json_Parameters_t                           global_config;
    std::map<uint, Dma_Type_Parameters_t>       memory_config;
};

using P2P_Card_Type_Parameters_t = struct P2P_Card_Type_Parameters_t {
    bool                exists;
    Json_Parameters_t   param;
};

using P2P_Card_Parameters_t = struct P2P_Card_Parameters_t {
    bool                                        exists;
    Json_Parameters_t                           global_config;
    std::map<uint, P2P_Card_Type_Parameters_t>  memory_config;
};

using P2P_Nvme_Type_Parameters_t = struct P2P_Nvme_Type_Parameters_t {
    bool                exists;
    Json_Parameters_t   param;
};

using P2P_Nvme_Parameters_t = struct P2P_Nvme_Parameters_t {
    bool                                        exists;
    Json_Parameters_t                           global_config;
    std::map<uint, P2P_Nvme_Type_Parameters_t>  memory_config;
};

using Memory_Type_Parameters_t = struct Memory_Type_Parameters_t {
    bool                                        exists;
    Json_Parameters_t                           global_config;
    std::map<uint,        Json_Parameters_t>    memory_chan_config;
    std::map<std::string, Json_Parameters_t>    memory_tag_config;
    Json_Parameters_t                           global_config_pattern;
    std::map<uint,        Json_Parameters_t>    memory_chan_config_pattern;
    std::map<std::string, Json_Parameters_t>    memory_tag_config_pattern;
};

using Memory_Parameters_t = struct Memory_Parameters_t {
    bool                                        exists;
    std::map<uint, Memory_Type_Parameters_t>    memory_config;
};

using Power_Parameters_t = struct Power_Parameters_t {
    bool                    exists;
    Json_Parameters_t       global_config;
};

using Gt_Lane_Parameters_t = struct Gt_Lane_Parameters_t {
    bool                exists;
    Json_Parameters_t   param;
};

using Gt_Parameters_t = struct Gt_Parameters_t {
    bool                                    exists;
    Json_Parameters_t                       global_config;
    std::map<uint, Gt_Lane_Parameters_t>    lane_config;
};

using Testcases_Parameters_t = struct Testcases_Parameters_t {
    Mmio_Parameters_t                   mmio;
    Dma_Parameters_t                    dma;
    P2P_Card_Parameters_t               p2p_card;
    P2P_Nvme_Parameters_t               p2p_nvme;
    Memory_Parameters_t                 memory;
    Power_Parameters_t                  power;
    std::map<uint, Gt_Parameters_t>     gt_mac;
    std::map<uint, Gt_Parameters_t>     gt_lpbk;
    std::map<uint, Gt_Parameters_t>     gt_prbs;
    std::map<uint, Gt_Parameters_t>     gtf_prbs;
    std::map<uint, Gt_Parameters_t>     gtm_prbs;
    std::map<uint, Gt_Parameters_t>     gtyp_prbs;
};

#define TEST_SEQ_PARAM_ARRAY_SIZE (uint)(2)

using Test_Seq_Param_Uint_t = struct Test_Seq_Param_Uint_t {
    uint value;
    bool exists;
};
#define RST_TEST_SEQ_PARAM_UINT Test_Seq_Param_Uint_t({ \
    .value  = 0, \
    .exists = false \
}) \

using Test_Seq_Param_Uint64_t = struct Test_Seq_Param_Uint64_t {
    uint64_t    value;
    bool        exists;
};
#define RST_TEST_SEQ_PARAM_UINT64 Test_Seq_Param_Uint64_t({ \
    .value  = 0, \
    .exists = false \
}) \

using Test_Seq_Param_Uint8_t = struct Test_Seq_Param_Uint8_t {
    uint8_t     value;
    bool        exists;
};
#define RST_TEST_SEQ_PARAM_UINT8 Test_Seq_Param_Uint8_t({ \
    .value  = 0, \
    .exists = false \
}) \

using Test_Seq_Param_Str_t = struct Test_Seq_Param_Str_t {
    std::string value;
    bool        exists;
};
#define RST_TEST_SEQ_PARAM_STR Test_Seq_Param_Str_t({ \
    .value  = "", \
    .exists = false \
}) \

// need to update inputparser.cpp too when adding/removing parameter in test sequence
#define DURATION_TEST_SEQ_MEMBER            Json_Val_Def_t({REQUIRED_TRUE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "duration",     "test duration (s)"})
#define BAR_TEST_SEQ_MEMBER                 Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "bar",          "PCIe Base Address Register (BAR) index"})
#define OFFSET_TEST_SEQ_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "offset",       "address offset in PCIe BAR"})
#define BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "buffer_size",  "buffer size (Bytes)"})


// MMIO
#define MMIO_TEST_SEQ_DEF            Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    BAR_TEST_SEQ_MEMBER, \
    OFFSET_TEST_SEQ_MEMBER, \
    BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER \
}) \

using Mmio_Test_Sequence_Parameters_t = struct Mmio_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Uint8_t  bar;
    Test_Seq_Param_Uint64_t offset;
    Test_Seq_Param_Uint64_t buffer_size;
    uint                    num_param; // number of parameters provided by user
};
#define RST_MMIO_TEST_SEQUENCE_PARAMETERS Mmio_Test_Sequence_Parameters_t({ \
    .duration    = RST_TEST_SEQ_PARAM_UINT, \
    .bar         = RST_TEST_SEQ_PARAM_UINT8, \
    .offset      = RST_TEST_SEQ_PARAM_UINT64, \
    .buffer_size = RST_TEST_SEQ_PARAM_UINT64, \
    .num_param   = 0 \
})

// DMA
#define TARGET_TEST_SEQ_MEMBER      Json_Val_Def_t({REQUIRED_TRUE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,   "target",       "target (memory type or memory tag)"})
#define BUFFER_SIZE_TEST_SEQ_MEMBER Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "buffer_size",  "buffer size (MB)"})

#define DMA_TEST_SEQ_DEF            Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    TARGET_TEST_SEQ_MEMBER, \
    BUFFER_SIZE_TEST_SEQ_MEMBER \
}) \


using DMA_Test_Sequence_Parameters_t = struct DMA_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    target; // Can be either a memory type or a memory tag
    Test_Seq_Param_Uint64_t buffer_size;
    uint                    num_param; // number of parameters provided by user
};
#define RST_DMA_TEST_SEQUENCE_PARAMETERS DMA_Test_Sequence_Parameters_t({ \
    .duration    = RST_TEST_SEQ_PARAM_UINT, \
    .target      = RST_TEST_SEQ_PARAM_STR, \
    .buffer_size = RST_TEST_SEQ_PARAM_UINT64, \
    .num_param   = 0 \
})

// P2P_CARD
#define SOURCE_TEST_SEQ_MEMBER      Json_Val_Def_t({REQUIRED_TRUE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING, "source", "source (memory type or memory tag)"})
#define TARGET_TEST_SEQ_MEMBER_OPT  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING, "target", "target (memory type or memory tag)"})
#define P2P_CARD_TEST_SEQ_DEF Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    SOURCE_TEST_SEQ_MEMBER, \
    TARGET_TEST_SEQ_MEMBER_OPT, \
    BUFFER_SIZE_TEST_SEQ_MEMBER \
}) \

using P2P_Card_Test_Sequence_Parameters_t = struct P2P_Card_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    source; // Can be either a memory type or a memory tag
    Test_Seq_Param_Str_t    target; // Can be either a memory type or a memory tag
    Test_Seq_Param_Uint64_t buffer_size;
    uint                    num_param; // number of parameters provided by user
};
#define RST_P2P_CARD_TEST_SEQUENCE_PARAMETERS P2P_Card_Test_Sequence_Parameters_t({ \
    .duration       = RST_TEST_SEQ_PARAM_UINT, \
    .source         = RST_TEST_SEQ_PARAM_STR, \
    .target         = RST_TEST_SEQ_PARAM_STR, \
    .buffer_size    = RST_TEST_SEQ_PARAM_UINT64, \
    .num_param      = 0 \
})

// P2P_NVME
#define SOURCE_TEST_SEQ_MEMBER_OPT  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING, "source", "source (memory type or memory tag)"})
#define P2P_NVME_TEST_SEQ_DEF Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    SOURCE_TEST_SEQ_MEMBER_OPT, \
    TARGET_TEST_SEQ_MEMBER_OPT, \
    BUFFER_SIZE_TEST_SEQ_MEMBER \
}) \

using P2P_Nvme_Test_Sequence_Parameters_t = struct P2P_Nvme_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    source; // Can be either a memory type or a memory tag
    Test_Seq_Param_Str_t    target; // Can be either a memory type or a memory tag
    Test_Seq_Param_Uint64_t buffer_size;
    uint                    num_param; // number of parameters provided by user
};
#define RST_P2P_NVME_TEST_SEQUENCE_PARAMETERS P2P_Nvme_Test_Sequence_Parameters_t({ \
    .duration       = RST_TEST_SEQ_PARAM_UINT, \
    .source         = RST_TEST_SEQ_PARAM_STR, \
    .target         = RST_TEST_SEQ_PARAM_STR, \
    .buffer_size    = RST_TEST_SEQ_PARAM_UINT64, \
    .num_param      = 0 \
})

// MEMORY
#define MODE_TEST_SEQ_MEMBER            Json_Val_Def_t({REQUIRED_TRUE,   HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,  "mode",             "mode of the xbtest HW IP"})
#define WR_RATE_TEST_SEQ_MEMBER         Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "wr_rate",          "write rate (%)"})
#define WR_BANDWIDTH_TEST_SEQ_MEMBER    Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "wr_bandwidth",     "write bandwidth requested (MBps)"})
#define WR_START_ADDR_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "wr_start_addr",    "write start address (MB)"})
#define WR_BURST_SIZE_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "wr_burst_size",    "write burst size (Bytes)"})
#define WR_BLOCK_SIZE_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "wr_block_size",    "write block size (MB)"})
#define WR_OUTSTANDING_TEST_SEQ_MEMBER  Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "wr_outstanding",   "maximum number of outstanding writes"})
#define RD_RATE_TEST_SEQ_MEMBER         Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "rd_rate",          "read rate (%)"})
#define RD_BANDWIDTH_TEST_SEQ_MEMBER    Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "rd_bandwidth",     "read bandwidth requested (%)"})
#define RD_START_ADDR_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "rd_start_addr",    "read start address (MB)"})
#define RD_BURST_SIZE_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "rd_burst_size",    "read burst size (Bytes)"})
#define RD_BLOCK_SIZE_TEST_SEQ_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "rd_block_size",    "read block size (MB)"})
#define RD_OUTSTANDING_TEST_SEQ_MEMBER  Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "rd_outstanding",   "maximum number of outstanding reads"})
#define MEMORY_TEST_SEQ_DEF             Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    MODE_TEST_SEQ_MEMBER, \
    WR_RATE_TEST_SEQ_MEMBER, \
    WR_BANDWIDTH_TEST_SEQ_MEMBER, \
    WR_START_ADDR_TEST_SEQ_MEMBER, \
    WR_BURST_SIZE_TEST_SEQ_MEMBER, \
    WR_BLOCK_SIZE_TEST_SEQ_MEMBER, \
    WR_OUTSTANDING_TEST_SEQ_MEMBER, \
    RD_RATE_TEST_SEQ_MEMBER, \
    RD_BANDWIDTH_TEST_SEQ_MEMBER, \
    RD_START_ADDR_TEST_SEQ_MEMBER, \
    RD_BURST_SIZE_TEST_SEQ_MEMBER, \
    RD_BLOCK_SIZE_TEST_SEQ_MEMBER, \
    RD_OUTSTANDING_TEST_SEQ_MEMBER \
}) \

using Memory_Test_Sequence_Wr_Rd_Parameters_t = struct Memory_Test_Sequence_Wr_Rd_Parameters_t {
    Test_Seq_Param_Uint_t   rate;
    Test_Seq_Param_Uint_t   bandwidth;
    Test_Seq_Param_Uint_t   start_addr;
    Test_Seq_Param_Uint_t   burst_size;
    Test_Seq_Param_Uint_t   block_size;
    Test_Seq_Param_Uint_t   outstanding;
};
#define RST_MEMORY_TEST_SEQUENCE_WR_RD_PARAMETERS Memory_Test_Sequence_Wr_Rd_Parameters_t({ \
    .rate        = RST_TEST_SEQ_PARAM_UINT, \
    .bandwidth   = RST_TEST_SEQ_PARAM_UINT, \
    .start_addr  = RST_TEST_SEQ_PARAM_UINT, \
    .burst_size  = RST_TEST_SEQ_PARAM_UINT, \
    .block_size  = RST_TEST_SEQ_PARAM_UINT, \
    .outstanding = RST_TEST_SEQ_PARAM_UINT \
}) \

using Memory_Test_Sequence_Parameters_t = struct Memory_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t                   duration;
    Test_Seq_Param_Str_t                    mode;
    Memory_Test_Sequence_Wr_Rd_Parameters_t write;
    Memory_Test_Sequence_Wr_Rd_Parameters_t read;
    uint                                    num_param; // number of parameters provided by user
};
#define RST_MEMORY_TEST_SEQUENCE_PARAMETERS Memory_Test_Sequence_Parameters_t({ \
    .duration   = RST_TEST_SEQ_PARAM_UINT, \
    .mode       = RST_TEST_SEQ_PARAM_STR, \
    .write      = RST_MEMORY_TEST_SEQUENCE_WR_RD_PARAMETERS, \
    .read       = RST_MEMORY_TEST_SEQUENCE_WR_RD_PARAMETERS, \
    .num_param  = 0 \
}) \

// POWER
#define TOGGLE_RATE_TEST_SEQ_MEMBER Json_Val_Def_t({REQUIRED_TRUE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT, "toggle_rate", "toggle rate (%)"})
#define POWER_TEST_SEQ_DEF          Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    TOGGLE_RATE_TEST_SEQ_MEMBER \
}) \

using Power_Test_Sequence_Parameters_t = struct Power_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Uint_t   toggle_rate;
    uint                    num_param; // number of parameters provided by user
};
#define RST_POWER_TEST_SEQUENCE_PARAMETERS Power_Test_Sequence_Parameters_t({ \
    .duration       = RST_TEST_SEQ_PARAM_UINT, \
    .toggle_rate    = RST_TEST_SEQ_PARAM_UINT, \
    .num_param      = 0 \
}) \

// GT_MAC
#define GT_MAC_TEST_SEQ_DEF Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    MODE_TEST_SEQ_MEMBER \
}) \

using GTMAC_Test_Sequence_Parameters_t = struct GTMAC_Test_Sequence_Parameters_t{
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    mode;
    uint                    num_param; // number of parameters provided by user
};
#define RST_GTMAC_TEST_SEQUENCE_PARAMETERS GTMAC_Test_Sequence_Parameters_t({ \
    .duration   = RST_TEST_SEQ_PARAM_UINT, \
    .mode       = RST_TEST_SEQ_PARAM_STR, \
    .num_param  = 0 \
}) \
// GT_LPBK
#define GT_LPBK_TEST_SEQ_DEF Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    MODE_TEST_SEQ_MEMBER \
}) \

using GTLpbk_Test_Sequence_Parameters_t = struct GTLpbk_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    mode;
    uint                    num_param; // number of parameters provided by user
};
#define RST_GTLPBK_TEST_SEQUENCE_PARAMETERS GTLpbk_Test_Sequence_Parameters_t({ \
    .duration   = RST_TEST_SEQ_PARAM_UINT, \
    .mode       = RST_TEST_SEQ_PARAM_STR, \
    .num_param  = 0 \
}) \

// GT_PRBS
#define GT_PRBS_TEST_SEQ_DEF Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    MODE_TEST_SEQ_MEMBER \
}) \

using GTPrbs_Test_Sequence_Parameters_t = struct GTPrbs_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    mode;
    uint                    num_param; // number of parameters provided by user
};
#define RST_GTPRBS_TEST_SEQUENCE_PARAMETERS GTPrbs_Test_Sequence_Parameters_t({ \
    .duration   = RST_TEST_SEQ_PARAM_UINT, \
    .mode       = RST_TEST_SEQ_PARAM_STR, \
    .num_param  = 0 \
}) \

// GTF_PRBS
// GTM_PRBS
// GTYP_PRBS
#define MULTI_GT_PRBS_TEST_SEQ_DEF Json_Params_Def_t({ \
    DURATION_TEST_SEQ_MEMBER, \
    MODE_TEST_SEQ_MEMBER \
}) \

using MultiGTPrbs_Test_Sequence_Parameters_t = struct MutliGTPrbs_Test_Sequence_Parameters_t {
    Test_Seq_Param_Uint_t   duration;
    Test_Seq_Param_Str_t    mode;
    uint                    num_param; // number of parameters provided by user
};
#define RST_MULTI_GTPRBS_TEST_SEQUENCE_PARAMETERS MultiGTPrbs_Test_Sequence_Parameters_t({ \
    .duration   = RST_TEST_SEQ_PARAM_UINT, \
    .mode       = RST_TEST_SEQ_PARAM_STR, \
    .num_param  = 0 \
}) \


#define PS_KERNEL_NAME_POWER std::string("krnl_powertest_ps")

// AIE types
#define AIE_TYPE_AIE             std::string("AIE_CORE")
#define AIE_TYPE_AIE_ML          std::string("AIE_ML_CORE")
#define SUPPORTED_AIE_TYPE       std::set<std::string>({ \
    AIE_TYPE_AIE, \
    AIE_TYPE_AIE_ML \
}) \
// AIE control modes
#define AIE_CONTROL_PS            std::string("ps")
#define AIE_CONTROL_PL            std::string("pl")
#define SUPPORTED_AIE_CONTROL     std::set<std::string>({ \
    AIE_CONTROL_PS, \
    AIE_CONTROL_PL \
}) \
// AIE status modes
#define AIE_STATUS_NONE         std::string("none")
#define AIE_STATUS_PL            std::string("pl")
#define SUPPORTED_AIE_STATUS     std::set<std::string>({ \
    AIE_STATUS_NONE, \
    AIE_STATUS_PL \
}) \

// Memory test modes
#define ALTERNATE_WR_RD             std::string("alternate_wr_rd")
#define ONLY_WR                     std::string("only_wr")
#define ONLY_RD                     std::string("only_rd")
#define SIMULTANEOUS_WR_RD          std::string("simultaneous_wr_rd")
#define SUPPORTED_MEMORY_TC_MODE    std::set<std::string>({ \
    ALTERNATE_WR_RD, \
    ONLY_WR, \
    ONLY_RD, \
    SIMULTANEOUS_WR_RD \
}) \

// GT_MAC test modes
#define CHECK_STATUS_STR          std::string("check_status")
#define CLEAR_STATUS_STR          std::string("clear_status")
#define RUN_STR                   std::string("run")
#define TX_RST_STR                std::string("tx_rst")
#define TX_DATAPATH_RST_STR       std::string("tx_datapath_rst")
#define RX_RST_STR                std::string("rx_rst")
#define RX_DATAPATH_RST_STR       std::string("rx_datapath_rst")
#define TX_RX_RST_STR             std::string("tx_rx_rst")
#define CONF_10GBE_NO_FEC_STR     std::string("conf_10gbe_no_fec")
#define CONF_10GBE_C74_FEC_STR    std::string("conf_10gbe_c74_fec")
#define CONF_25GBE_NO_FEC_STR     std::string("conf_25gbe_no_fec")
#define CONF_25GBE_C74_FEC_STR    std::string("conf_25gbe_c74_fec")
#define CONF_25GBE_RS_FEC_STR     std::string("conf_25gbe_rs_fec")
#define SUPPORTED_GT_MAC_SEQUENCE_MODE std::set<std::string>({ \
    CHECK_STATUS_STR, \
    CLEAR_STATUS_STR, \
    RUN_STR, \
    TX_RST_STR, \
    RX_RST_STR, \
    TX_RX_RST_STR, \
    CONF_10GBE_NO_FEC_STR, \
    CONF_10GBE_C74_FEC_STR, \
    CONF_25GBE_NO_FEC_STR, \
    CONF_25GBE_C74_FEC_STR, \
    CONF_25GBE_RS_FEC_STR \
}) \

// GT_LPBK test modes
#define SUPPORTED_GT_LPBK_SEQUENCE_MODE std::set<std::string>({ \
    RUN_STR, \
    CONF_25GBE_NO_FEC_STR, \
    TX_RX_RST_STR \
}) \

#define CONF_25GBE_STR          std::string("conf_25gbe")
#define CONF_MULTI_GT_STR       std::string("conf_gt")
#define INSERT_ERROR_LN_0_STR   std::string("insert_error_lane_0")
#define INSERT_ERROR_LN_1_STR   std::string("insert_error_lane_1")
#define INSERT_ERROR_LN_2_STR   std::string("insert_error_lane_2")
#define INSERT_ERROR_LN_3_STR   std::string("insert_error_lane_3")

// GT_PRBS test modes
#define SUPPORTED_GT_PRBS_SEQUENCE_MODE std::set<std::string>({ \
    RUN_STR, \
    CONF_25GBE_STR, \
    TX_RX_RST_STR, \
    CHECK_STATUS_STR, \
    CLEAR_STATUS_STR, \
    INSERT_ERROR_LN_0_STR, \
    INSERT_ERROR_LN_1_STR, \
    INSERT_ERROR_LN_2_STR, \
    INSERT_ERROR_LN_3_STR \
}) \


// MULTI_GT_PRBS test modes
// GTF_PRBS test modes
// GTM_PRBS test modes
// GTYP_PRBS test modes
#define SUPPORTED_MULTI_GT_PRBS_SEQUENCE_MODE std::set<std::string>({ \
    RUN_STR, \
    CONF_MULTI_GT_STR, \
    TX_RX_RST_STR, \
    TX_RST_STR, \
    RX_RST_STR, \
    TX_DATAPATH_RST_STR, \
    RX_DATAPATH_RST_STR, \
    CHECK_STATUS_STR, \
    CLEAR_STATUS_STR, \
    INSERT_ERROR_LN_0_STR, \
    INSERT_ERROR_LN_1_STR, \
    INSERT_ERROR_LN_2_STR, \
    INSERT_ERROR_LN_3_STR \
}) \

// Test sequence parameters names
#define DURATION                    std::string("duration")
#define MEM_TAG                     std::string("memory tag")
#define BAR                         std::string("bar")
#define OFFSET                      std::string("offset")
#define BUFFER_SIZE                 std::string("buffer size")
#define BUFFER_CNT                  std::string("number of buffers")
#define TOT_SIZE                    std::string("total size")
// Memory
#define TEST_MODE                   std::string("test mode")
// Memory write
#define WR_MEM_RATE                 std::string("write rate")
#define WR_MEM_BANDWIDTH            std::string("write bandwidth requested")
#define WR_START_ADDR               std::string("write start address offset")
#define WR_END_ADDR                 std::string("write end address offset")
#define WR_BURST_SIZE               std::string("write burst size")
#define WR_BLOCK_SIZE               std::string("write block size")
// Memory read
#define RD_MEM_RATE                 std::string("read rate")
#define RD_MEM_BANDWIDTH            std::string("read bandwidth requested")
#define RD_START_ADDR               std::string("read start address offset")
#define RD_END_ADDR                 std::string("read end address offset")
#define RD_BURST_SIZE               std::string("read burst size")
#define RD_BLOCK_SIZE               std::string("read block size")
// Memory write
#define WR_DATA_SIZE                std::string("AXI write data size")
#define WR_BURST_XFER               std::string("number of write transfers per burst")
#define WR_NUM_BURST                std::string("total number of write bursts per block")
#define WR_TOTAL_XFER               std::string("total number of write transfers per block")
#define WR_OUTSTANDING              std::string("maximum number of outstanding writes")
#define WR_MAX_BW                   std::string("maximum write BW")
#define WR_MIN_BW                   std::string("minimum write BW")
#define WR_AVG_BW                   std::string("average write BW")
#define WR_LIV_BW                   std::string("live write BW")
#define WR_LIV_BURST_INST           std::string("live number of write bursts per second")
#define WR_LIV_BURST_TIME_MIN       std::string("live minimum write burst time")
#define WR_LIV_BURST_TIME_AVG       std::string("live average write burst time")
#define WR_LIV_BURST_TIME_MAX       std::string("live maximum write burst time")
#define WR_BURST_INST               std::string("average number of write bursts per second")
#define WR_BURST_TIME_MIN           std::string("minimum write burst time")
#define WR_BURST_TIME_AVG           std::string("average write burst time")
#define WR_BURST_TIME_MAX           std::string("maximum write burst time")
#define WR_LIV_BURST_LATENCY_MIN    std::string("live minimum write burst latency")
#define WR_LIV_BURST_LATENCY_AVG    std::string("live average write burst latency")
#define WR_LIV_BURST_LATENCY_MAX    std::string("live maximum write burst latency")
#define WR_BURST_LATENCY_MIN        std::string("minimum write burst latency")
#define WR_BURST_LATENCY_AVG        std::string("average write burst latency")
#define WR_BURST_LATENCY_MAX        std::string("maximum write burst latency")
// Memory read
#define RD_DATA_SIZE                std::string("AXI read data size")
#define RD_BURST_XFER               std::string("number of read transfers per burst")
#define RD_NUM_BURST                std::string("total number of read bursts per block")
#define RD_TOTAL_XFER               std::string("total number of read transfers per block")
#define RD_OUTSTANDING              std::string("maximum number of outstanding reads")
#define RD_MAX_BW                   std::string("maximum read BW")
#define RD_MIN_BW                   std::string("minimum read BW")
#define RD_AVG_BW                   std::string("average read BW")
#define RD_LIV_BW                   std::string("live read BW")
#define RD_LIV_BURST_INST           std::string("live number of read bursts per second")
#define RD_LIV_BURST_TIME_MIN       std::string("live minimum read burst time")
#define RD_LIV_BURST_TIME_AVG       std::string("live average read burst time")
#define RD_LIV_BURST_TIME_MAX       std::string("live maximum read burst time")
#define RD_BURST_INST               std::string("average number of read bursts per second")
#define RD_BURST_TIME_MIN           std::string("minimum read burst time")
#define RD_BURST_TIME_AVG           std::string("average read burst time")
#define RD_BURST_TIME_MAX           std::string("maximum read burst time")
#define RD_LIV_BURST_LATENCY_MIN    std::string("live minimum read burst latency")
#define RD_LIV_BURST_LATENCY_AVG    std::string("live average read burst latency")
#define RD_LIV_BURST_LATENCY_MAX    std::string("live maximum read burst latency")
#define RD_BURST_LATENCY_MIN        std::string("minimum read burst latency")
#define RD_BURST_LATENCY_AVG        std::string("average read burst latency")
#define RD_BURST_LATENCY_MAX        std::string("maximum read burst latency")
// total write+read
#define WR_RD_AVG_BW                std::string("average total write+read BW")
#define WR_RD_LIV_BW                std::string("live total write+read BW")
#define WR_RD_LIV_XFER_CNT          std::string("live total write+read number of transfers per second")
#define WR_RD_LIV_XFER_ACC          std::string("accumulated total write+read number of transfers per second")

#define DATA_INTEG                  std::string("data integrity")
#define DATA_INTEG_LIV              std::string("live data integrity")
#define DATA_INTEG_NA               std::string("N/A")
#define DATA_INTEG_NC               std::string("Not Checked")
#define DATA_INTEG_KO               std::string("KO")
#define DATA_INTEG_OK               std::string("OK")
#define DATA_INTEGS                 std::vector<std::string>({ \
    DATA_INTEG_NA, \
    DATA_INTEG_NC, \
    DATA_INTEG_KO, \
    DATA_INTEG_OK \
}) \

#define BW_RESULT_NA                std::string("N/A")
#define BW_RESULT_PASS              std::string("PASS")
#define BW_RESULT_FAILURE           std::string("FAILURE")
#define BW_RESULTS                  std::vector<std::string>({ \
    BW_RESULT_NA, \
    BW_RESULT_PASS, \
    BW_RESULT_FAILURE \
}) \


#define MAX_UINT8_VAL               (uint8_t)(0xFF)
#define MAX_UINT_VAL                (uint)(0xFFFFFFFF)
#define MAX_UINT64_VAL              (uint64_t)(0xFFFFFFFFFFFFFFFF)

#define MAX_NUM_PARSER_ERROR        (uint)(20)
#define MIN_DURATION                (uint)(1)
#define MAX_DURATION                (uint)(MAX_UINT_VAL)

// Command line parameters names
#define TIMESTAMP_MODE              std::string("timestamp")

// JSON members
// Level 0 nodes
#define TASKS_MEMBER        Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "Tasks",       "tasks definitions"})
#define TESTCASES_MEMBER    Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "Testcases",   "testcases definitions"})
#define BASE_PARAMETERS_DEF Json_Params_Def_t({ \
    TASKS_MEMBER, \
    TESTCASES_MEMBER \
}) \

// tasks nodes
// "tasks" level 1 nodes

// there is no definition with this task
#define GTMACMGMT_MEMBER               Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE,  JSON_NODE_VALUE, TYPE_ID_STRING, "gt_mac_mgmt",    DESCRIPTION_NOT_SET})

#define DEVICE_MGMT_MEMBER      Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "device_mgmt",  "device_mgmt task parameters definition"})
#define DEBUG_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_TRUE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "debug",        "debug task parameters definition"})
#define TASKS_PARAMETERS_DEF    Json_Params_Def_t({ \
    DEVICE_MGMT_MEMBER, \
    DEBUG_MEMBER \
}) \

// "tasks"."device_mgt" parameters
#define USE_OUTPUT_FILE_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,   "use_output_file",     "Store in a file all measurements of the testcase or task"})
#define WATCHDOG_DURATION_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,   "watchdog_duration",   "Specify the watchdog duration"})
#define DUMP_DURATION_ALARM_MEMBER          Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,   "dump_duration_alarm", "Specify the maximum sensor reading duration for which a critical warning is displayed"})
#define SENSOR_MEMBER                       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_ARRAY, TYPE_ID_OBJECT, "sensor",              "List of sensor definitions. Override existing sensor definition or define new sensor"})
#define TASKS_DEVICE_MGMT_PARAMETERS_DEF    Json_Params_Def_t({ \
    USE_OUTPUT_FILE_MEMBER, \
    WATCHDOG_DURATION_MEMBER, \
    DUMP_DURATION_ALARM_MEMBER, \
    SENSOR_MEMBER \
}) \

#define SENSOR_TYPE_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_STRING,         "type",              "Sensor type. Supported values are: " + StrSetToStr(QuoteStrSet(SUPPORTED_SENSOR_TYPES), ", ")})
#define SENSOR_ID_MEMBER                Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_STRING,         "id",                "Sensor ID. For \"" + XRT_INFO_DEVICE_ELECTRICAL + "\" power_rails sources, if both current and voltage are present then the thresholds are specified in W and compared against the source power. If only current (or voltage) is present, then the thresholds are in A (or V) and compared against current (or voltage)"})
#define SENSOR_WARNING_THRESHOLD_MEMBER Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "warning_threshold", "Warning limits: if measurement is out of range, a critical warning is displayed (once), no test case is stopped and global result is pass"})
#define SENSOR_ERROR_THRESHOLD_MEMBER   Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "error_threshold",   "Error limits  : if measurement is out of range, an error is displayed (once), no test case is stopped and global result is failure"})
#define SENSOR_ABORT_THRESHOLD_MEMBER   Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "abort_threshold",   "Abort limits  : if measurement is out of range, a failure is displayed (once), all test cases are stopped and global result is failure"})
#define DEVICE_MGMT_SENSOR_DEF          Json_Params_Def_t({ \
    SENSOR_TYPE_MEMBER, \
    SENSOR_ID_MEMBER, \
    SENSOR_WARNING_THRESHOLD_MEMBER, \
    SENSOR_ERROR_THRESHOLD_MEMBER, \
    SENSOR_ABORT_THRESHOLD_MEMBER \
}) \

#define SENSOR_THRESHOLD_MIN_MEMBER Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_DOUBLE, "min", "Minimum threshold"})
#define SENSOR_THRESHOLD_MAX_MEMBER Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_DOUBLE, "max", "Maximum threshold"})
#define DEVICE_MGMT_THRESHOLD_DEF   Json_Params_Def_t({ \
    SENSOR_THRESHOLD_MIN_MEMBER, \
    SENSOR_THRESHOLD_MAX_MEMBER \
}) \


// "tasks"."debug" parameters
#define GET_MSG_DEF_MEMBER          Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_TRUE,  JSON_NODE_VALUE, TYPE_ID_STRING, "get_msg_def",    DESCRIPTION_NOT_SET})
#define TASKS_DEBUG_PARAMETERS_DEF  Json_Params_Def_t({ \
    GET_MSG_DEF_MEMBER \
}) \

// Common test JSON members
#define GLOBAL_CONFIG_MEMBER           Json_Val_Def_t({REQUIRED_TRUE,  HIDDEN_FALSE, JSON_NODE_OBJECT,               TYPE_ID_NOT_APPLICABLE,         "global_config",            "global configuration parameters definition"})
#define TEST_SEQUENCE_MEMBER           Json_Val_Def_t({REQUIRED_TRUE,  HIDDEN_FALSE, JSON_NODE_ARRAY,                TYPE_ID_TEST_SEQUENCE,          "test_sequence",            "describe the sequence of tests to perform describing each test in an array"})
#define OPTIONAL_TEST_SEQUENCE_MEMBER  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, TEST_SEQUENCE_MEMBER.node_type, TEST_SEQUENCE_MEMBER.typeId,    TEST_SEQUENCE_MEMBER.name,  TEST_SEQUENCE_MEMBER.description})
// DMA + Memory test JSON members
#define CHECK_BW_MEMBER                Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,     "check_bw",                 "enable bandwidth checking"})
#define CHECK_DATA_INTEGRITY_MEMBER    Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,     "check_data_integrity",     "enable data integrity checking"})
#define TOTAL_SIZE_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "total_size",               "total amount of data (MB) per bandwidth measurement"})
#define TOTAL_SIZE_BYTES_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT64_T, "total_size",               "total amount of data (Bytes) per bandwidth measurement"})
#define LO_THRESH_WR_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "lo_thresh_wr",             "overwrite low  threshold of the write bandwidth (MB/s)"})
#define HI_THRESH_WR_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "hi_thresh_wr",             "overwrite high threshold of the write bandwidth (MB/s)"})
#define LO_THRESH_RD_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "lo_thresh_rd",             "overwrite low  threshold of the read  bandwidth (MB/s)"})
#define HI_THRESH_RD_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "hi_thresh_rd",             "overwrite high threshold of the read  bandwidth (MB/s)"})
#define LO_THRESH_WR_KBPS_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "lo_thresh_wr",             "overwrite low  threshold of the write bandwidth (kB/s)"})
#define HI_THRESH_WR_KBPS_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "hi_thresh_wr",             "overwrite high threshold of the write bandwidth (kB/s)"})
#define LO_THRESH_RD_KBPS_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "lo_thresh_rd",             "overwrite low  threshold of the read  bandwidth (kB/s)"})
#define HI_THRESH_RD_KBPS_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "hi_thresh_rd",             "overwrite high threshold of the read  bandwidth (kB/s)"})
#define STOP_ON_ERROR_MEMBER           Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,     "stop_on_error",            "stop the test iteration when first error occurs"})

// testcases nodes
// This testcase has no JSON parameters
#define VERIFY_STR                  std::string("verify")
#define MMIO_MEMBER                 Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "mmio",       "MMIO testcase parameters definition"})
#define DMA_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "dma",        "DMA testcase parameters definition"})
#define P2P_CARD_MEMBER             Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "p2p_card",   "P2P_CARD testcase parameters definition"})
#define P2P_NVME_MEMBER             Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "p2p_nvme",   "P2P_NVME testcase parameters definition"})
#define POWER_MEMBER                Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "power",      "Power testcase parameters definition"})
#define MEMORY_MEMBER               Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "memory",     "Memory testcase parameters definition"})
#define GT_MAC_MEMBER               Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "gt_mac",     "GT_MAC testcase parameters definition"})
#define GT_LPBK_MEMBER              Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "gt_lpbk",    "GT_LPBK testcase parameters definition"})
#define GT_PRBS_MEMBER              Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "gt_prbs",    "GT_PRBS testcase parameters definition"})
#define GTF_PRBS_MEMBER             Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "gtf_prbs",   "GTF_PRBS testcase parameters definition"})
#define GTM_PRBS_MEMBER             Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "gtm_prbs",   "GTM_PRBS testcase parameters definition"})
#define GTYP_PRBS_MEMBER            Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "gtyp_prbs",  "GTYP_PRBS testcase parameters definition"})
#define TESTCASES_PARAMETERS_DEF    Json_Params_Def_t({ \
    MMIO_MEMBER, \
    DMA_MEMBER, \
    P2P_CARD_MEMBER, \
    P2P_NVME_MEMBER, \
    POWER_MEMBER, \
    MEMORY_MEMBER, \
    GT_MAC_MEMBER, \
    GT_LPBK_MEMBER, \
    GT_PRBS_MEMBER, \
    GTF_PRBS_MEMBER, \
    GTM_PRBS_MEMBER, \
    GTYP_PRBS_MEMBER \
}) \

// "testcases"."mmio" parameters
#define MMIO_SUB_PARAMETERS_DEF      Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER \
}) \

#define MMIO_GLOBAL_CONFIG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    TOTAL_SIZE_BYTES_MEMBER, \
    LO_THRESH_WR_KBPS_MEMBER, \
    HI_THRESH_WR_KBPS_MEMBER, \
    LO_THRESH_RD_KBPS_MEMBER, \
    HI_THRESH_RD_KBPS_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \


// "testcases"."dma" parameters
#define MEMORY_TYPE_CONFIG_MEMBER   Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "memory_type_config", "DMA testcase memory type specific configuration parameters definition"})
#define DMA_SUB_PARAMETERS_DEF      Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
    MEMORY_TYPE_CONFIG_MEMBER \
}) \

#define DMA_GLOBAL_CONFIG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \

#define DMA_MEMORY_TYPE_PARAMETERS_DEF  Json_Params_Def_t({ \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    TOTAL_SIZE_MEMBER, \
    LO_THRESH_WR_MEMBER, \
    HI_THRESH_WR_MEMBER, \
    LO_THRESH_RD_MEMBER, \
    HI_THRESH_RD_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \

// "testcases"."p2p_card" parameters
#define P2P_CARD_SUB_PARAMETERS_DEF Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
    MEMORY_TYPE_CONFIG_MEMBER \
}) \

#define P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \

#define P2P_CARD_MEMORY_TYPE_PARAMETERS_DEF Json_Params_Def_t({ \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    TOTAL_SIZE_MEMBER, \
    LO_THRESH_WR_MEMBER, \
    HI_THRESH_WR_MEMBER, \
    LO_THRESH_RD_MEMBER, \
    HI_THRESH_RD_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \

// "testcases"."p2p_nvme" parameters
#define P2P_NVME_SUB_PARAMETERS_DEF Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
    MEMORY_TYPE_CONFIG_MEMBER \
}) \

#define P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \

#define P2P_NVME_MEMORY_TYPE_PARAMETERS_DEF Json_Params_Def_t({ \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    TOTAL_SIZE_MEMBER, \
    LO_THRESH_WR_MEMBER, \
    HI_THRESH_WR_MEMBER, \
    LO_THRESH_RD_MEMBER, \
    HI_THRESH_RD_MEMBER, \
    STOP_ON_ERROR_MEMBER \
}) \


// "testcases"."memory" parameters
#define MEMORY_TAG_CONFIG_MEMBER     Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "memory_tag_config",     "memory tag specific parameters definition in memory testcase configuration"})
#define MEMORY_CHANNEL_CONFIG_MEMBER Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "memory_channel_config", "memory channel specific parameters definition in memory testcase configuration"})
#define MEMORY_SC_SUB_PARAMETERS_DEF Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
    MEMORY_TAG_CONFIG_MEMBER \
}) \

#define MEMORY_MC_SUB_PARAMETERS_DEF Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
    MEMORY_CHANNEL_CONFIG_MEMBER \
}) \

#define DISABLE_MEMORY_MEMBER               Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_BOOL,           "disable_memory",         "disable memory test"})
#define DISABLE_PREWRITE_MEMBER             Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_BOOL,           "disable_prewrite",       "disable pre-writing of the targeted memory before a test, including read operations, starts"})
#define PATTERN_MEMBER                      Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "pattern",                "Define the data pattern transfered to the memory"})
#define SINGLE_AXI_THREAD_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_BOOL,           "single_axi_thread",      "use only one AXI thread per channel of the xbtest HW IPs. Option applicable if supported by xbtest HW IP (refer to UG)"})
#define LO_THRESH_ALT_WR_BW_MEMBER          Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_alt_wr_bw",    "overwrite low  threshold of the write bandwidth in \"" + ALTERNATE_WR_RD + "\" mode (MB/s)"})
#define HI_THRESH_ALT_WR_BW_MEMBER          Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_alt_wr_bw",    "overwrite high threshold of the write bandwidth in \"" + ALTERNATE_WR_RD + "\" mode (MB/s)"})
#define LO_THRESH_ALT_RD_BW_MEMBER          Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_alt_rd_bw",    "overwrite low  threshold of the read  bandwidth in \"" + ALTERNATE_WR_RD + "\" mode (MB/s)"})
#define HI_THRESH_ALT_RD_BW_MEMBER          Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_alt_rd_bw",    "overwrite high threshold of the read  bandwidth in \"" + ALTERNATE_WR_RD + "\" mode (MB/s)"})
#define LO_THRESH_ONLY_WR_BW_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_only_wr_bw",   "overwrite low  threshold of the write bandwidth in \"" + ONLY_WR + "\" mode (MB/s)"})
#define HI_THRESH_ONLY_WR_BW_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_only_wr_bw",   "overwrite high threshold of the write bandwidth in \"" + ONLY_WR + "\" mode (MB/s)"})
#define LO_THRESH_ONLY_RD_BW_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_only_rd_bw",   "overwrite low  threshold of the read  bandwidth in \"" + ONLY_RD + "\" mode (MB/s)"})
#define HI_THRESH_ONLY_RD_BW_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_only_rd_bw",   "overwrite high threshold of the read  bandwidth in \"" + ONLY_RD + "\" mode (MB/s)"})
#define LO_THRESH_SIMUL_WR_BW_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_simul_wr_bw",  "overwrite low  threshold of the write bandwidth in \"" + SIMULTANEOUS_WR_RD + "\" mode (MB/s)"})
#define HI_THRESH_SIMUL_WR_BW_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_simul_wr_bw",  "overwrite high threshold of the write bandwidth in \"" + SIMULTANEOUS_WR_RD + "\" mode (MB/s)"})
#define LO_THRESH_SIMUL_RD_BW_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_simul_rd_bw",  "overwrite low  threshold of the read  bandwidth in \"" + SIMULTANEOUS_WR_RD + "\" mode (MB/s)"})
#define HI_THRESH_SIMUL_RD_BW_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_simul_rd_bw",  "overwrite high threshold of the read  bandwidth in \"" + SIMULTANEOUS_WR_RD + "\" mode (MB/s)"})
#define CHECK_LATENCY_MEMBER                Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_BOOL,           "check_latency",          "enable latency checking"})
#define LO_THRESH_ALT_WR_LAT_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_alt_wr_lat",   "overwrite low  threshold of the write latency in \"" + ALTERNATE_WR_RD + "\" mode (ns)"})
#define HI_THRESH_ALT_WR_LAT_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_alt_wr_lat",   "overwrite high threshold of the write latency in \"" + ALTERNATE_WR_RD + "\" mode (ns)"})
#define LO_THRESH_ALT_RD_LAT_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_alt_rd_lat",   "overwrite low  threshold of the read  latency in \"" + ALTERNATE_WR_RD + "\" mode (ns)"})
#define HI_THRESH_ALT_RD_LAT_MEMBER         Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_alt_rd_lat",   "overwrite high threshold of the read  latency in \"" + ALTERNATE_WR_RD + "\" mode (ns)"})
#define LO_THRESH_ONLY_WR_LAT_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_only_wr_lat",  "overwrite low  threshold of the write latency in \"" + ONLY_WR + "\" mode (ns)"})
#define HI_THRESH_ONLY_WR_LAT_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_only_wr_lat",  "overwrite high threshold of the write latency in \"" + ONLY_WR + "\" mode (ns)"})
#define LO_THRESH_ONLY_RD_LAT_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_only_rd_lat",  "overwrite low  threshold of the read  latency in \"" + ONLY_RD + "\" mode (ns)"})
#define HI_THRESH_ONLY_RD_LAT_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_only_rd_lat",  "overwrite high threshold of the read  latency in \"" + ONLY_RD + "\" mode (ns)"})
#define LO_THRESH_SIMUL_WR_LAT_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_simul_wr_lat", "overwrite low  threshold of the write latency in \"" + SIMULTANEOUS_WR_RD + "\" mode (ns)"})
#define HI_THRESH_SIMUL_WR_LAT_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_simul_wr_lat", "overwrite high threshold of the write latency in \"" + SIMULTANEOUS_WR_RD + "\" mode (ns)"})
#define LO_THRESH_SIMUL_RD_LAT_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "lo_thresh_simul_rd_lat", "overwrite low  threshold of the read  latency in \"" + SIMULTANEOUS_WR_RD + "\" mode (ns)"})
#define HI_THRESH_SIMUL_RD_LAT_MEMBER       Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE,  TYPE_ID_UINT,           "hi_thresh_simul_rd_lat", "overwrite high threshold of the read  latency in \"" + SIMULTANEOUS_WR_RD + "\" mode (ns)"})
#define MEMORY_GLOBAL_CONFIG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    DISABLE_MEMORY_MEMBER, \
    DISABLE_PREWRITE_MEMBER, \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    LO_THRESH_ALT_WR_BW_MEMBER, \
    HI_THRESH_ALT_WR_BW_MEMBER, \
    LO_THRESH_ALT_RD_BW_MEMBER, \
    HI_THRESH_ALT_RD_BW_MEMBER, \
    LO_THRESH_ONLY_WR_BW_MEMBER, \
    HI_THRESH_ONLY_WR_BW_MEMBER, \
    LO_THRESH_ONLY_RD_BW_MEMBER, \
    HI_THRESH_ONLY_RD_BW_MEMBER, \
    LO_THRESH_SIMUL_WR_BW_MEMBER, \
    HI_THRESH_SIMUL_WR_BW_MEMBER, \
    LO_THRESH_SIMUL_RD_BW_MEMBER, \
    HI_THRESH_SIMUL_RD_BW_MEMBER, \
    CHECK_LATENCY_MEMBER, \
    LO_THRESH_ALT_WR_LAT_MEMBER, \
    HI_THRESH_ALT_WR_LAT_MEMBER, \
    LO_THRESH_ALT_RD_LAT_MEMBER, \
    HI_THRESH_ALT_RD_LAT_MEMBER, \
    LO_THRESH_ONLY_WR_LAT_MEMBER, \
    HI_THRESH_ONLY_WR_LAT_MEMBER, \
    LO_THRESH_ONLY_RD_LAT_MEMBER, \
    HI_THRESH_ONLY_RD_LAT_MEMBER, \
    LO_THRESH_SIMUL_WR_LAT_MEMBER, \
    HI_THRESH_SIMUL_WR_LAT_MEMBER, \
    LO_THRESH_SIMUL_RD_LAT_MEMBER, \
    HI_THRESH_SIMUL_RD_LAT_MEMBER \
}) \

#define MEMORY_TAG_CHAN_PARAMETERS_DEF Json_Params_Def_t({ \
    DISABLE_MEMORY_MEMBER, \
    OPTIONAL_TEST_SEQUENCE_MEMBER, \
    DISABLE_PREWRITE_MEMBER, \
    CHECK_BW_MEMBER, \
    CHECK_DATA_INTEGRITY_MEMBER, \
    LO_THRESH_ALT_WR_BW_MEMBER, \
    HI_THRESH_ALT_WR_BW_MEMBER, \
    LO_THRESH_ALT_RD_BW_MEMBER, \
    HI_THRESH_ALT_RD_BW_MEMBER, \
    LO_THRESH_ONLY_WR_BW_MEMBER, \
    HI_THRESH_ONLY_WR_BW_MEMBER, \
    LO_THRESH_ONLY_RD_BW_MEMBER, \
    HI_THRESH_ONLY_RD_BW_MEMBER, \
    LO_THRESH_SIMUL_WR_BW_MEMBER, \
    HI_THRESH_SIMUL_WR_BW_MEMBER, \
    LO_THRESH_SIMUL_RD_BW_MEMBER, \
    HI_THRESH_SIMUL_RD_BW_MEMBER, \
    CHECK_LATENCY_MEMBER, \
    LO_THRESH_ALT_WR_LAT_MEMBER, \
    HI_THRESH_ALT_WR_LAT_MEMBER, \
    LO_THRESH_ALT_RD_LAT_MEMBER, \
    HI_THRESH_ALT_RD_LAT_MEMBER, \
    LO_THRESH_ONLY_WR_LAT_MEMBER, \
    HI_THRESH_ONLY_WR_LAT_MEMBER, \
    LO_THRESH_ONLY_RD_LAT_MEMBER, \
    HI_THRESH_ONLY_RD_LAT_MEMBER, \
    LO_THRESH_SIMUL_WR_LAT_MEMBER, \
    HI_THRESH_SIMUL_WR_LAT_MEMBER, \
    LO_THRESH_SIMUL_RD_LAT_MEMBER, \
    HI_THRESH_SIMUL_RD_LAT_MEMBER \
}) \

#define MIN_HW_VERSION_PATTERN  double(6.1)
#define PATTERN_CTRL_PRBS       std::string("prbs")
#define PATTERN_CTRL_FIXED      std::string("fixed")
#define PATTERN_CTRL_ALTERNATE  std::string("alternate")

#define SUPPORTED_PATTERN_CTRL   std::set<std::string>({ \
    PATTERN_CTRL_PRBS, \
    PATTERN_CTRL_FIXED, \
    PATTERN_CTRL_ALTERNATE \
}) \

#define PATTERN_LIST_MEMBER     Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_ARRAY, TYPE_ID_STRING, "patterns",  "define list of 1 or 2 patterns. A pattern must be a string 32-bit hexadecimal value starting with 0x with maximum 8 hexadecimal characters"})
#define PATTERN_CTRL_MEMBER     Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING, "control",   "select pattern: Supported values: \"" + PATTERN_CTRL_PRBS + "\" (default), \"" + PATTERN_CTRL_ALTERNATE + "\" (alternate between first two patterns of \"" + PATTERN_LIST_MEMBER.name + "\" list) or \"" + PATTERN_CTRL_FIXED + "\" (constant data set with first element of \"" + PATTERN_LIST_MEMBER.name + "\" list)"})
#define MEMORY_PATTERN_DEF Json_Params_Def_t({ \
    PATTERN_LIST_MEMBER, \
    PATTERN_CTRL_MEMBER \
}) \

// "testcases"."power" parameters
#define POWER_SUB_PARAMETERS_DEF    Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
}) \

#define DISABLE_REG_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL, "disable_reg",  "disable the register slices of the Power xbtest HW IP"})
#define DISABLE_DSP_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL, "disable_dsp",  "disable the DSP of the Power xbtest HW IP"})
#define DISABLE_BRAM_MEMBER                 Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL, "disable_bram", "disable the BRAM of the Power xbtest HW IP"})
#define DISABLE_URAM_MEMBER                 Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL, "disable_uram", "disable the URAMs of the Power xbtest HW IP"})
#define DISABLE_AIE_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL, "disable_aie",  "disable the AIEs of the Power xbtest HW IP"})
#define DISABLE_TOG_RAMP_MEMBER             Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL, "disable_toggle_ramp", "disable the toggle ramp protection Power xbtest HW IP"})
// disable_aie is added to the POWER_GLOBAL_CONFIG_PARAMETERS_DEF only if the feature is present in HW design (USE_AIE in USER_METADATA)
#define POWER_GLOBAL_CONFIG_PARAMETERS_DEF  Json_Params_Def_t( { \
    TEST_SEQUENCE_MEMBER, \
    DISABLE_REG_MEMBER, \
    DISABLE_DSP_MEMBER, \
    DISABLE_BRAM_MEMBER, \
    DISABLE_URAM_MEMBER, \
    DISABLE_TOG_RAMP_MEMBER \
}) \

#define POWER_SLR_CONFIG_PARAMETERS_DEF     Json_Params_Def_t({ \
}) \

// "testcases"."gt_mac" parameters
#define LANE_CONFIG_MEMBER          Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "lane_config",   "gt_mac testcase lane specific parameters definition"})
#define GT_INDEX_PARAMETERS_DEF     Json_Params_Def_t({ \
    GLOBAL_CONFIG_MEMBER, \
    LANE_CONFIG_MEMBER \
}) \

// Global config only
#define GT_SETTINGS_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,   "gt_settings",          "select GT default settings"})
// Global & Lane config
#define UTILISATION_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "utilisation",          "define the transmit utilization (0-100%)"})
#define TRAFFIC_TYPE_MEMBER                 Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,   "traffic_type",         "define the content of the payload area of the packets"})
#define PACKET_CFG_MEMBER                   Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,   "packet_cfg",           "define the packet length"})
#define MATCH_TX_RX_MEMBER                  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,     "match_tx_rx",          "enable RX and TX packet count match"})
#define MAC_TO_MAC_CONNECTION_MEMBER        Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "mac_to_mac_connection", "define GT port to port connection"})
#define GT_TX_DIFFCTRL_MEMBER               Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "gt_tx_diffctrl",       "overwrite the GTY Transceiver TXDIFFCTRL input"})
#define GT_TX_MAIN_CURSOR_MEMBER            Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "gt_tx_main_cursor",    "overwrite the GTY Transceiver TXMAINCURSOR input"})
#define GT_TX_PRE_EMPH_MEMBER               Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "gt_tx_pre_emph",       "overwrite the GTY Transceiver TXPRECURSOR input"})
#define GT_TX_POST_EMPH_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,     "gt_tx_post_emph",      "overwrite the GTY Transceiver TXPOSTCURSOR input"})
#define GT_RX_USE_LPM_MEMBER                Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,     "gt_rx_use_lpm",        "overwrite the GTY Transceiver RXLPMEN input"})
#define GT_TX_POLARITY_MEMBER               Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,   "gt_tx_polarity",       "invert the polarity of outgoing data, TXp/TXn swap"})
#define GT_MAC_GLOBAL_CFG_PARAMETERS_DEF    Json_Params_Def_t({ \
    OPTIONAL_TEST_SEQUENCE_MEMBER, \
    GT_SETTINGS_MEMBER, \
    UTILISATION_MEMBER, \
    TRAFFIC_TYPE_MEMBER, \
    PACKET_CFG_MEMBER, \
    MATCH_TX_RX_MEMBER, \
    MAC_TO_MAC_CONNECTION_MEMBER, \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER \
}) \

// Lane config only
#define DISABLE_LANE_MEMBER             Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,    "disable_lane", "disable packet generator"})
#define TX_MAPPING_MEMBER               Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "tx_mapping",   "specify the TX lane number which will be checked against RX status"})
#define DEST_ADDR_MEMBER                Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,  "dest_addr",    "specify the destination MAC address"})
#define SOURCE_ADDR_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,  "source_addr",  "specify the source MAC address"})
#define GT_MAC_LANE_CFG_PARAMETERS_DEF  Json_Params_Def_t({ \
    UTILISATION_MEMBER, \
    TRAFFIC_TYPE_MEMBER, \
    PACKET_CFG_MEMBER, \
    MATCH_TX_RX_MEMBER, \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER, \
    DISABLE_LANE_MEMBER, \
    TX_MAPPING_MEMBER, \
    SOURCE_ADDR_MEMBER, \
    DEST_ADDR_MEMBER \
}) \

// "testcases"."gt_lpbk" parameters
#define GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    GT_SETTINGS_MEMBER, \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER \
}) \

#define GT_LPBK_LANE_CFG_PARAMETERS_DEF Json_Params_Def_t({ \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER \
}) \

#define GT_LOOPBACK_MEMBER              Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,  "gt_loopback",          "define gt loopback"})
#define GT_DISABLE_REF_PRBS_MEMBER      Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,    "disable_ref_prbs",     "disable reference prbs"})
#define GT_PRBS_ERROR_THRESHOLD_MEMBER  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_UINT,    "prbs_error_threshold", "threshold in percent under which prbs errors are ignored"})

#define GT_PRBS_LANE_CFG_PARAMETERS_DEF Json_Params_Def_t({ \
    DISABLE_LANE_MEMBER, \
    GT_PRBS_ERROR_THRESHOLD_MEMBER, \
    GT_LOOPBACK_MEMBER, \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER, \
    GT_RX_POLARITY_MEMBER \
}) \

// "testcases"."gt_prbs" parameters
#define DEFAULT_MEMBER Json_Val_Def_t({REQUIRED_FALSE,  HIDDEN_FALSE,  JSON_NODE_OBJECT, TYPE_ID_NOT_APPLICABLE, "default", "gt_prbs testcase parameters definition to apply to all available GTs"})

#define GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    GT_DISABLE_REF_PRBS_MEMBER, \
    GT_PRBS_ERROR_THRESHOLD_MEMBER, \
    GT_SETTINGS_MEMBER, \
    GT_LOOPBACK_MEMBER, \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER \
}) \

// "testcases"."gtf_prbs" parameters
// "testcases"."gtm_prbs" parameters
// "testcases"."gtyp_prbs" parameters
#define GT_RX_POLARITY_MEMBER   Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_STRING,  "gt_rx_polarity",       "invert the polarity of incoming data, RXp/RXn swap"})
#define LANE_0_ENABLE_MEMBER    Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,  "lane_0_enable",  "enable lane 0"})
#define LANE_1_ENABLE_MEMBER    Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,  "lane_1_enable",  "enable lane 1"})
#define LANE_2_ENABLE_MEMBER    Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,  "lane_2_enable",  "enable lane 2"})
#define LANE_3_ENABLE_MEMBER    Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_BOOL,  "lane_3_enable",  "enable lane 3"})
#define LANE_ENABLE_MEMBER Json_Params_Def_t({ LANE_0_ENABLE_MEMBER, LANE_1_ENABLE_MEMBER, LANE_2_ENABLE_MEMBER, LANE_3_ENABLE_MEMBER })
#define GT_BER_THRESHOLD_MEMBER  Json_Val_Def_t({REQUIRED_FALSE, HIDDEN_FALSE, JSON_NODE_VALUE, TYPE_ID_DOUBLE,    "ber_threshold", "Bit error rate threshold under which errors are ignored"})


#define MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF Json_Params_Def_t({ \
    TEST_SEQUENCE_MEMBER, \
    GT_DISABLE_REF_PRBS_MEMBER, \
    GT_BER_THRESHOLD_MEMBER, \
    GT_SETTINGS_MEMBER, \
    GT_LOOPBACK_MEMBER, \
    GT_TX_DIFFCTRL_MEMBER, \
    GT_TX_MAIN_CURSOR_MEMBER, \
    GT_TX_PRE_EMPH_MEMBER, \
    GT_TX_POST_EMPH_MEMBER, \
    GT_RX_USE_LPM_MEMBER, \
    GT_TX_POLARITY_MEMBER, \
    GT_RX_POLARITY_MEMBER, \
    LANE_0_ENABLE_MEMBER, \
    LANE_1_ENABLE_MEMBER, \
    LANE_2_ENABLE_MEMBER, \
    LANE_3_ENABLE_MEMBER \
}) \

using Multi_GT_t = enum Multi_GT_t {
    GTF = 0,
    GTM,
    GTYP
};

using ThreadType = enum ThreadType {
    THREAD_TYPE_TESTCASE    = 0,
    THREAD_TYPE_TASK        = 1
};

using TestCaseThreadResult = enum TestCaseThreadResult {
    TC_FAIL,
    TC_PASS,
    TC_ABORTED
};

inline Json_Parameters_t::iterator FindJsonParam( Json_Parameters_t & json_parameters, const Json_Val_Def_t & json_val_def )
{
    // Check if str present in json_parameters using lower case and return iterator
    for (auto param_it = json_parameters.begin(); param_it != json_parameters.end(); ++param_it)
    {
        if (StrMatchNoCase(param_it->first, json_val_def.name))
        {
            return param_it;
        }
    }
    return json_parameters.end();
}

inline void EraseJsonParam( Json_Parameters_t & json_parameters, const Json_Val_Def_t & json_val_def )
{
    // Check if str present in json_parameters using lower case and return iterator
    for (auto param_it = json_parameters.begin(); param_it != json_parameters.end(); ++param_it)
    {
        if (StrMatchNoCase(param_it->first, json_val_def.name))
        {
            json_parameters.erase(param_it->first);
            break;
        }
    }
}

template<typename T> inline void InsertJsonParam( Json_Parameters_t & json_parameters, const Json_Val_Def_t & json_val_def, const T & value )
{
    json_parameters.insert( Json_Parameter_t(json_val_def.name, new TestcaseParam<T>(value)) );
}

// User metadata
#define SINGLE_CHANNEL  std::string("single_channel")
#define MULTI_CHANNEL   std::string("multi_channel")
#define MEMORY_CU_TYPES std::set<std::string>({SINGLE_CHANNEL, MULTI_CHANNEL})
#define HOST            std::string("host")
#define BOARD           std::string("board")
#define MEMORY_TARGETS  std::set<std::string>({HOST, BOARD})

// Test iteration result
using TestIterationResult = enum TestIterationResult {
    TITR_NOT_TESTED,
    TITR_DISABLED,
    TITR_PASSED,
    TITR_FAILED,
    TITR_ABORTED
};

// Line HEADER
// (% is replace by an index at runtime for test with multiple threads)
#define LOG_HEADER_XBT_SW_CFG     std::string("XBT_SW_CFG  ")
#define LOG_HEADER_PFM_DEF        std::string("CARD_DEF    ")
#define LOG_HEADER_INPUTPARSER    std::string("INPUT_PARSER")
#define LOG_HEADER_XCLBIN_PARSER  std::string("META_PARSER ")
#define LOG_HEADER_DEVICE         std::string("DEVICE      ")
#define LOG_HEADER_VERIFY         std::string("VERIFY      ")
#define LOG_HEADER_MMIO           std::string("MMIO        ")
#define LOG_HEADER_DMA            std::string("DMA         ")
#define LOG_HEADER_P2P_CARD       std::string("P2P_CARD    ")
#define LOG_HEADER_P2P_NVME       std::string("P2P_NVME    ")
#define LOG_HEADER_POWER          std::string("POWER       ")
//                                  ex   SC=> "MEMORY       :: <type> :: <BD tag> :: ";
//                                  ex   MC=> "MEMORY       :: <type> :: ";
//                                  ex   SC=> "MEMORY       :: DDR   :: bank_1 :: ";
//                                  ex   MC=> "MEMORY       :: HBM   :: ";
#define LOG_HEADER_MEMORY         std::string("MEMORY      ")
//                                  ex        "GT_MAC[0]   "
#define LOG_HEADER_GTMAC          std::string("GT_MAC[%]   ")
//                                  ex        "GT_LPBK[0]  "
#define LOG_HEADER_GTLPBK         std::string("GT_LPBK[%]  ")
#define LOG_HEADER_GTPRBS         std::string("GT_PRBS[%]  ")
#define LOG_HEADER_GTFPRBS        std::string("GTF_PRBS[%] ")
#define LOG_HEADER_GTMPRBS        std::string("GTM_PRBS[%] ")
#define LOG_HEADER_GTYPPRBS       std::string("GTYP_PRBS[%]")
#define LOG_HEADER_DEVICE_MGT     std::string("DEVICE_MGMT ")
#define LOG_HEADER_GTMACMGMT      std::string("GTMAC_MGMT  ")

// Sensor sources
using Power_Rail_Source_t = struct Power_Rail_Source_t {
    std::string         type;
    std::string         display_name_power;
    std::string         display_name_current_value;
    std::string         display_name_current_status;
    std::string         display_name_voltage_value;
    std::string         display_name_voltage_status;
    std::string         unit_power;
    std::string         unit_current;
    std::string         unit_voltage;
    double              power;
    bool                power_is_present;
    Sensor_Limit_t      warning_threshold;
    Sensor_Limit_t      error_threshold;
    Sensor_Limit_t      abort_threshold;
    bool                node_absence_msg;
    bool                found;
    bool                card_power;
    Power_Rail_Info_t   info;
};
#define RST_POWER_POWER_RAIL_SOURCE Power_Rail_Source_t({ \
    .type                           = XRT_INFO_DEVICE_ELECTRICAL, \
    .display_name_power             = "", \
    .display_name_current_value     = "", \
    .display_name_current_status    = "", \
    .display_name_voltage_value     = "", \
    .display_name_voltage_status    = "", \
    .unit_power                     = "W", \
    .unit_current                   = "A", \
    .unit_voltage                   = "V", \
    .power                          = 0.0, \
    .power_is_present               = false, \
    .warning_threshold              = RST_SENSOR_LIMIT, \
    .error_threshold                = RST_SENSOR_LIMIT, \
    .abort_threshold                = RST_SENSOR_LIMIT, \
    .node_absence_msg               = true, \
    .found                          = false, \
    .card_power                     = false, \
    .info                           = RST_POWER_RAIL_INFO \
}) \

using Power_Consumption_Source_t = struct Power_Consumption_Source_t {
    std::string                 type;
    std::string                 display_name_value;
    std::string                 display_name_status;
    std::string                 unit;
    Sensor_Limit_t              warning_threshold;
    Sensor_Limit_t              error_threshold;
    Sensor_Limit_t              abort_threshold;
    bool                        node_absence_msg;
    bool                        found;
    bool                        card_power;
    Power_Consumption_Info_t    info;
};
#define RST_POWER_CONSUMPTION_SOURCE Power_Consumption_Source_t({ \
    .type                   = XRT_INFO_DEVICE_ELECTRICAL, \
    .display_name_value     = "", \
    .display_name_status    = "", \
    .unit                   = "W", \
    .warning_threshold      = RST_SENSOR_LIMIT, \
    .error_threshold        = RST_SENSOR_LIMIT, \
    .abort_threshold        = RST_SENSOR_LIMIT, \
    .node_absence_msg       = true, \
    .found                  = false, \
    .card_power             = false, \
    .info                   = RST_POWER_CONSUMPTION_INFO \
}) \

using Thermal_Source_t = struct Thermal_Source_t {
    std::string         type;
    std::string         display_name_value;
    std::string         display_name_status;
    std::string         unit;
    Sensor_Limit_t      warning_threshold;
    Sensor_Limit_t      error_threshold;
    Sensor_Limit_t      abort_threshold;
    bool                node_absence_msg;
    bool                found;
    bool                card_temperature;
    Thermal_El_Info_t   info;
};
#define RST_THERMAL_SOURCE Thermal_Source_t({ \
    .type                   = XRT_INFO_DEVICE_THERMAL, \
    .display_name_value     = "", \
    .display_name_status    = "", \
    .unit                   = "deg C", \
    .warning_threshold      = RST_SENSOR_LIMIT, \
    .error_threshold        = RST_SENSOR_LIMIT, \
    .abort_threshold        = RST_SENSOR_LIMIT, \
    .node_absence_msg       = true, \
    .found                  = false, \
    .card_temperature       = false, \
    .info                   = RST_THERMAL_EL_INFO \
}) \

using Fan_Source_t = struct Fan_Source_t {
    std::string     type;
    std::string     display_name_value;
    std::string     display_name_status;
    std::string     unit;
    Sensor_Limit_t  warning_threshold;
    Sensor_Limit_t  error_threshold;
    Sensor_Limit_t  abort_threshold;
    bool            node_absence_msg;
    bool            found;
    Fan_Info_t      info;
};
#define RST_FAN_SOURCE Fan_Source_t({ \
    .type                   = XRT_INFO_DEVICE_MECHANICAL, \
    .display_name_value     = "", \
    .display_name_status    = "", \
    .unit                   = "rpm", \
    .warning_threshold      = RST_SENSOR_LIMIT, \
    .error_threshold        = RST_SENSOR_LIMIT, \
    .abort_threshold        = RST_SENSOR_LIMIT, \
    .node_absence_msg       = true, \
    .found                  = false, \
    .info                   = RST_FAN_INFO \
}) \

using Sensor_t = struct Sensor_t {
    bool                                    dump_valid;
    uint64_t                                dump_cnt;
    uint64_t                                dump_time;
    double                                  card_temperature;
    double                                  card_power;
    bool                                    card_temperature_found;
    bool                                    card_power_found;
    std::vector<Power_Rail_Source_t>        power_rail_sources;
    std::vector<Power_Consumption_Source_t> power_consumption_sources;
    std::vector<Thermal_Source_t>           thermal_sources;
    std::vector<Fan_Source_t>               fan_sources;
};
#define RST_SENSOR Sensor_t({ \
    .dump_valid                 = false, \
    .dump_cnt                   = 0, \
    .dump_time                  = 0, \
    .card_temperature           = 0.0, \
    .card_power                 = 0.0, \
    .card_temperature_found     = false, \
    .card_power_found           = false, \
    .power_rail_sources         = {}, \
    .power_consumption_sources  = {}, \
    .thermal_sources            = {}, \
    .fan_sources                = {} \
}) \


// xbtest_pfm_def structure
using Xbtest_Pfm_Def_Runtime_t = struct Xbtest_Pfm_Def_Runtime_t {
    uint download_time;
};

using Xbtest_Pfm_Def_Sensor_Source_t = struct Xbtest_Pfm_Def_Sensor_Source_t {
    std::string  id;
    std::string  type;
    bool         card_power;
    bool         card_temperature;
};

using Mem_Thresh_t = struct Mem_Thresh_t {
    uint average;
    uint high;
    uint low;
};

using Mem_Thresh_WrRd_t = struct Mem_Thresh_WrRd_t {
    Mem_Thresh_t write;
    Mem_Thresh_t read;
};

using Mem_Thresh_Wr_t = struct Mem_Thresh_Wr_t {
    Mem_Thresh_t write;
};

using Mem_Thresh_Rd_t = struct Mem_Thresh_Rd_t {
    Mem_Thresh_t read;
};

using Mem_Thresh_CU_t = struct Mem_Thresh_CU_t {
    Mem_Thresh_WrRd_t   alt_wr_rd;
    Mem_Thresh_Wr_t     only_wr;
    Mem_Thresh_Rd_t     only_rd;
    Mem_Thresh_WrRd_t   simul_wr_rd;
};

using Mem_Rate_t = struct Mem_Rate_t {
    uint nominal;
};

using Mem_Rate_WrRd_t = struct Mem_Rate_WrRd_t {
    Mem_Rate_t write;
    Mem_Rate_t read;
};

using Mem_Rate_Wr_t = struct Mem_Rate_Wr_t {
    Mem_Rate_t write;
};

using Mem_Rate_Rd_t = struct Mem_Rate_Rd_t {
    Mem_Rate_t read;
};

using Mem_Rate_CU_t = struct Mem_Rate_CU_t {
    Mem_Rate_WrRd_t   alt_wr_rd;
    Mem_Rate_Wr_t     only_wr;
    Mem_Rate_Rd_t     only_rd;
    Mem_Rate_WrRd_t   simul_wr_rd;
};

using Mem_Outstanding_t = struct Mem_Outstanding_t {
    uint nominal;
};

using Mem_Outstanding_WrRd_t = struct Mem_Outstanding_WrRd_t {
    Mem_Outstanding_t write;
    Mem_Outstanding_t read;
};

using Mem_Outstanding_Wr_t = struct Mem_Outstanding_Wr_t {
    Mem_Outstanding_t write;
};

using Mem_Outstanding_Rd_t = struct Mem_Outstanding_Rd_t {
    Mem_Outstanding_t read;
};

using Mem_Outstanding_CU_t = struct Mem_Outstanding_CU_t {
    Mem_Outstanding_WrRd_t   alt_wr_rd;
    Mem_Outstanding_Wr_t     only_wr;
    Mem_Outstanding_Rd_t     only_rd;
    Mem_Outstanding_WrRd_t   simul_wr_rd;
};

using Mem_Burst_Size_t = struct Mem_Burst_Size_t {
    uint nominal;
    bool exists;
};

using Mem_Burst_Size_WrRd_t = struct Mem_Burst_Size_WrRd_t {
    Mem_Burst_Size_t write;
    Mem_Burst_Size_t read;
};

using Mem_Burst_Size_Wr_t = struct Mem_Burst_Size_Wr_t {
    Mem_Burst_Size_t write;
};

using Mem_Burst_Size_Rd_t = struct Mem_Burst_Size_Rd_t {
    Mem_Burst_Size_t read;
};

using Mem_Burst_Size_CU_t = struct Mem_Burst_Size_CU_t {
    Mem_Burst_Size_WrRd_t   alt_wr_rd;
    Mem_Burst_Size_Wr_t     only_wr;
    Mem_Burst_Size_Rd_t     only_rd;
    Mem_Burst_Size_WrRd_t   simul_wr_rd;
};

using MMIO_Config_t = struct MMIO_Config_t {
    uint64_t    total_size;
    uint64_t    buffer_size;
    uint8_t     bar;
    uint64_t    offset;

    bool total_size_exists;
    bool buffer_size_exists;
    bool bar_exists;
    bool offset_exists;
};
#define RST_MMIO_CONFIG MMIO_Config_t({ \
    .total_size         = 0, \
    .buffer_size        = 0, \
    .bar                = 0, \
    .offset             = 0, \
    .total_size_exists  = false, \
    .buffer_size_exists = false, \
    .bar_exists         = false, \
    .offset_exists      = false \
}) \

using Dma_Config_t = struct Dma_Config_t {
    uint total_size;
    uint buffer_size;

    bool total_size_exists;
    bool buffer_size_exists;
};

using P2P_Card_Config_t = struct P2P_Card_Config_t {
    uint total_size;
    uint buffer_size;

    bool total_size_exists;
    bool buffer_size_exists;
};

using P2P_Nvme_Config_t = struct P2P_Nvme_Config_t {
    uint total_size;
    uint buffer_size;

    bool total_size_exists;
    bool buffer_size_exists;
};

using Xbtest_Pfm_Def_Memory_t = struct Xbtest_Pfm_Def_Memory_t {
    std::string             name;
    Mem_Thresh_WrRd_t       dma_bw;
    Dma_Config_t            dma_config;
    Mem_Thresh_WrRd_t       p2p_card_bw;
    P2P_Card_Config_t       p2p_card_config;
    Mem_Thresh_WrRd_t       p2p_nvme_bw;
    P2P_Nvme_Config_t       p2p_nvme_config;
    Mem_Thresh_CU_t         cu_bw;
    Mem_Thresh_CU_t         cu_latency;
    Mem_Rate_CU_t           cu_rate;
    Mem_Outstanding_CU_t    cu_outstanding;
    Mem_Burst_Size_CU_t     cu_burst_size;

    bool dma_bw_exists;
    bool dma_config_exists;
    bool p2p_card_bw_exists;
    bool p2p_card_config_exists;
    bool p2p_nvme_bw_exists;
    bool p2p_nvme_config_exists;
};

using Xbtest_Pfm_Def_Mmio_t = struct Xbtest_Pfm_Def_Mmio_t {
    Mem_Thresh_WrRd_t       bw;
    MMIO_Config_t           config;

    bool bw_exists;
    bool config_exists;
};

using GT_Trans_Settings_t = struct GT_Trans_Settings_t {
    std::string     tx_polarity;
    uint            tx_main_cursor;
    uint            tx_differential_swing_control;
    uint            tx_pre_emphasis;
    uint            tx_post_emphasis;
    std::string     rx_equalizer;
    std::string     rx_polarity;
};

using GT_Multi_Settings_t = struct GT_Multi_Settings_t {
    GT_Trans_Settings_t  cable;
    GT_Trans_Settings_t  module;
};

using Xbtest_Pfm_Def_GT_Settings_t = struct Xbtest_Pfm_Def_GT_Settings_t {
    std::string         name;
    GT_Multi_Settings_t transceivers;
};

using Xbtest_Pfm_Def_GT_t = struct Xbtest_Pfm_Def_GT_t {
    std::vector<Xbtest_Pfm_Def_GT_Settings_t> settings;
    uint                                      num_gt;           // Number of GT defined in platfrom definition JSON file
};

using Xbtest_Pfm_Def_t = struct Xbtest_Pfm_Def_t {
    Xbtest_Pfm_Def_Runtime_t                    runtime;
    std::vector<Xbtest_Pfm_Def_Sensor_Source_t> sensor;
    std::vector<Xbtest_Pfm_Def_Memory_t>        memory;
    Xbtest_Pfm_Def_Mmio_t                       mmio;
    Xbtest_Pfm_Def_GT_t                         gt;
};

// xbtest metadata structures

using PCIe_Bar_Ep_t = struct PCIe_BAR_t {
    uint        pcie_base_address_register;
    uint        pcie_physical_function;
    uint64_t    offset;
    uint64_t    range;
    std::string name;
};

using Xclbin_Partition_Metadata_t = struct Xclbin_Partition_Metadata_t {
    std::vector<PCIe_Bar_Ep_t>  pcie_bars;
    std::vector<PCIe_Bar_Ep_t>  pcie_eps;
};

// IP_LAYOUT structures: used to get xbtest HW IP connectivity
using Xclbin_Ip_Data_t = struct Xclbin_Ip_Data_t {
    std::string m_name;
    uint        m_ip_layout_index;
};

using Xclbin_Ip_Layout_t = struct Xclbin_Ip_Layout_t {
    std::vector<Xclbin_Ip_Data_t>  m_ip_data;
};

// MEM_TOPOLOGY structures
using Xclbin_Memory_Data_t = struct Xclbin_Memory_Data_t {
    uint8_t     m_used;
    uint64_t    size_bytes;
    std::string m_tag;
    std::string m_ip_name;
    uint64_t    m_base_address;
    uint        mem_data_idx;
};

// CONNECTIVITY structures
using Xclbin_Connection_t = struct Xclbin_Connection_t {
    int32_t arg_index;
    int32_t m_ip_layout_index;
    int32_t mem_data_index;
};

using Xclbin_Connectivity_t = struct Xclbin_Connectivity_t {
    std::vector<Xclbin_Connection_t>  m_connection;
};

// CLOCK_FREQ_TOPOLOGY structures
using Xclbin_Clock_Freq_t = struct Xclbin_Clock_Freq_t {
    bool    found;
    int16_t freq_Mhz;
};

using Xclbin_Clock_Freq_Topology_t = struct Xclbin_Clock_Freq_Topology_t {
    std::vector<Xclbin_Clock_Freq_t>  m_clock_freq;
};

// USER_METADATA structures
using Xclbin_UM_BI_Version_t = struct Xclbin_UM_BI_Version_t {
    uint major;
    uint minor;
};

using Xclbin_UM_BI_Xbtest_t = struct Xclbin_UM_BI_Xbtest_t {
    Xclbin_UM_BI_Version_t  version;
    uint                    build;
    std::string             date;
    bool                    internal_release;
};

using Xclbin_UM_BI_Board_t = struct Xclbin_UM_BI_Board_t {
    std::string name;
    std::string interface_uuid;
};

using Xclbin_UM_Build_Info_t = struct Xclbin_UM_Build_Info_t {
    Xclbin_UM_BI_Xbtest_t xbtest;
    Xclbin_UM_BI_Board_t  board;
};

using Xclbin_UM_Def_AIE_t = struct Xclbin_UM_Def_AIE_t {
    bool        exists;
    uint        freq;
    std::string type;
    std::string control;
    std::string status;
};

using Xclbin_UM_Def_Memory_t = struct Xclbin_UM_Def_Memory_t {
    std::string name;
    std::string target;
    std::string type;
    uint        axi_data_size;
    uint        num_axi_thread;
    uint        idx;
};

using Xclbin_UM_Def_GT_t = struct Xclbin_UM_Def_GT_t {
    uint        gt_index;
    std::string gt_group_select;
    std::string gt_serial_port;
    std::string gt_diff_clocks;
};

using Xclbin_UM_Def_CU_Type_Config_t = struct Xclbin_UM_Def_CU_Type_Config_t {
    // Verify xbtest HW IP
    bool        dna_read;
    // Power xbtest HW IP
    uint        SLR;
    std::string throttle_mode;
    uint        use_aie;
    // Memory xbtest HW IP
    std::string memory_type;
    // GT_MAC/GT_LPBK/GT_PRBS xbtest HW IP
    uint        gt_index;
    // GTF_PRBS xbtest HW IP
    std::vector<bool> gtf_indexes;
    // GTM_PRBS xbtest HW IP
    std::vector<bool> gtm_indexes;
    // GTYP_PRBS xbtest HW IP
    std::vector<bool> gtyp_indexes;
};

using Xclbin_UM_Def_Compute_Units_t = struct Xclbin_UM_Def_Compute_Units_t {
    std::string                    name;
    uint                           SLR;
    uint                           mode;
    std::vector<std::string>       connectivity; // sptags used in v++
    Xclbin_UM_Def_CU_Type_Config_t cu_type_configuration;
};

using Xclbin_UM_Definition_t = struct Xclbin_UM_Definition_t {
    Xclbin_UM_Def_AIE_t                        aie;
    std::vector<uint>                          clocks;
    std::vector<Xclbin_UM_Def_Memory_t>        memory;
    std::vector<Xclbin_UM_Def_GT_t>            gt;
    std::vector<Xclbin_UM_Def_Compute_Units_t> compute_units;
};

using Xclbin_User_Metadata_t = struct Xclbin_User_Metadata_t {
    Xclbin_UM_Build_Info_t build_info;
    Xclbin_UM_Definition_t definition;
};

using Xclbin_Metadata_t = struct Xclbin_Metadata_t {
    Xclbin_Partition_Metadata_t         partition_metadata;
    Xclbin_Ip_Layout_t                  ip_layout;
    std::vector<Xclbin_Memory_Data_t>   memory_topology;
    Xclbin_Connectivity_t               connectivity;
    Xclbin_Clock_Freq_Topology_t        clock_freq_topology;
    Xclbin_User_Metadata_t              user_metadata;
};

// Actual xbtest SW configuration structures
// This is to merge info from xbtest metadata, xbtest_pfm_def and XRT API device info
// This contains only the actual resources available in current HW design
using Xbtest_GT_t = struct Xbtest_GT_t {
    // USER_METADATA
    uint                gt_index;
    std::string         gt_group_select;
    std::string         gt_serial_port;
    std::string         gt_diff_clocks;
    // xbtest_pfm_def
    std::string         name;
    GT_Multi_Settings_t transceivers;
};

using Xbtest_Memory_t = struct Xbtest_Memory_t {
    // USER_METADATA
    std::string                 name;
    std::string                 target;
    std::string                 type;
    uint                        axi_data_size;
    uint                        num_axi_thread;
    uint                        idx;
    // MEM_TOPOLOGY
    std::vector<std::string>    available_tag;

    // xbtest_pfm_def
    Mem_Thresh_WrRd_t           dma_bw;
    bool                        dma_bw_exists;
    Dma_Config_t                dma_config;
    bool                        dma_config_exists;
    Mem_Thresh_WrRd_t           p2p_card_bw;
    bool                        p2p_card_bw_exists;
    P2P_Card_Config_t           p2p_card_config;
    bool                        p2p_card_config_exists;
    Mem_Thresh_WrRd_t           p2p_nvme_bw;
    bool                        p2p_nvme_bw_exists;
    P2P_Nvme_Config_t           p2p_nvme_config;
    bool                        p2p_nvme_config_exists;

    Mem_Thresh_CU_t             cu_bw;
    Mem_Thresh_CU_t             cu_latency;
    Mem_Rate_CU_t               cu_rate;
    Mem_Outstanding_CU_t        cu_outstanding;
    Mem_Burst_Size_CU_t         cu_burst_size;

    // device info memory
    uint                        size_mb; // Size of each memory bank for SC. Total size of all memory banks for MC
    uint                        cu_size_mb; // Size of each memory bank for SC. Total size of all memory banks for MC
    // From xbtest HW IP
    std::vector<std::string>    vpp_sptags;
    uint                        num_channels;
    uint                        num_cu;
};

using Xbtest_CU_Type_Config_AIE_t = struct Xbtest_CU_Type_Config_AIE_t {
    bool        exists;
    uint        freq;
    std::string type;
    std::string control;
    std::string status;
};

using Xbtest_CU_Type_Config_t = struct Xbtest_CU_Type_Config_t {
    // Verify xbtest HW IP
    bool                        dna_read;
    // Power xbtest HW IP
    uint                        SLR;
    std::string                 throttle_mode;
    uint                        use_aie;
    Xbtest_CU_Type_Config_AIE_t aie;
    // Memory xbtest HW IP
    Xbtest_Memory_t *           memory; // pointer to memory definition
    // GT_MAC/LPBK xbtest HW IP
    Xbtest_GT_t *               gt; // pointer to GT definition
    std::vector<Xbtest_GT_t *>  gts; // pointer to GT definition
};

using Xbtest_Connection_t = struct Xbtest_Connection_t {
    // USER_METADATA
    std::string                 vpp_sptag;                  // sptag used when building HW design with Vitis
    uint                        vpp_sptag_occurrence;       // Index of sptag occurrence, multiple xbtest HW IP port can be connected to same memory
    // MEM_TOPOLOGY
    std::vector<uint>           mem_data_indexes;           // Memory topology indexes connected to xbtest HW IP port
    std::vector<std::string>    mem_tags;                   // Memory topology tags connected to xbtest HW IP port
    // device info MEM_TOPOLOGY
    std::vector<uint64_t>       dev_info_base_addresses;    // Base address of each memory tags connected to xbtest HW IP port
    std::vector<uint64_t>       dev_info_sizes_bytes;       // Size of each memory tags connected to xbtest HW IP port
    uint64_t                    dev_info_total_sizes_bytes; // Total size of all memory tags connected to xbtest HW IP port
    // Based on other connections, for memory test
    std::vector<uint64_t>       actual_sizes_bytes;         // Size of each memory tags connected to xbtest HW IP port in memory test case
    uint64_t                    actual_total_size_bytes;    // Total size of all memory tags connected to xbtest HW IP port in memory test case
    uint64_t                    actual_base_address;        // Base address in memory of xbtest HW IP port in memory test case
};

using Xbtest_Compute_Units_t = struct Xbtest_Compute_Units_t {
    // IP_LAYOUT
    int32_t                             m_ip_layout_index;
    // USER_METADATA
    std::string                         name;
    uint                                SLR;
    uint                                mode;
    Xbtest_CU_Type_Config_t             cu_type_configuration;
    // USER_METADATA / CONNECTIVITY / MEM_TOPOLOGY merged
    std::vector<Xbtest_Connection_t>    connectivity; // sptags used in v++
    // Other flags computed by host application
    bool                                skipped;   // applicable for Memory xbtest HW IP with target = host. Used to skip tests when the host memory is not enabled
    bool                                companion; // This kind of xbtest HW IP is not controllable by the host application as it's not connected to PLRAM (e.g Power xbtest HW IP controlled by another Power xbtest HW IP)
};

using Xbtest_Command_Line_Sw_t = struct Xbtest_Command_Line_Sw_t {
    std::vector<std::string>    data;
    // Arg with no opt
    bool                        help;
    bool                        version;
    bool                        disable_log;
    bool                        console_detailed_format;
    bool                        force;
    // Arg with opt
    Xbtest_CL_option_Str_t      test_json;
    Xbtest_CL_option_Str_t      xbtest_pfm_def;
    Xbtest_CL_option_Str_t      p2p_xbtest_pfm_def;
    Xbtest_CL_option_Str_t      xclbin;
    Xbtest_CL_option_Str_t      p2p_xclbin;
    Xbtest_CL_option_Str_t      device_bdf;
    Xbtest_CL_option_Str_t      p2p_target_bdf;
    Xbtest_CL_option_Str_t      p2p_nvme_path;
    Xbtest_CL_option_Str_t      log_dir;
    Xbtest_CL_option_Str_t      timestamp_mode;
    Xbtest_CL_option_Int_t      verbosity;
    Xbtest_CL_option_Str_t      msg_id;
    Xbtest_CL_option_Str_t      guide;
    Xbtest_CL_option_Str_t      design_pdi;
};

using Xbtest_Logging_t = struct Xbtest_Logging_t {
    std::string log_dir;
    bool        log_dir_created;
};

using Xbtest_Configuration_t = struct Xbtest_Configuration_t {
    Xbtest_Command_Line_Sw_t            command_line;
    std::string                         bdf;
    std::string                         xbtest_pfm_def;
    std::string                         xclbin;
    std::string                         design_pdi;
    Xbtest_Logging_t                    logging;
    Xbtest_System_t                     system;
    std::vector<Xbtest_GT_t>            gt;
    std::vector<Xbtest_Memory_t>        memory;
    std::vector<Xbtest_Compute_Units_t> compute_units;
};

// Vitis Analyser header
#define VA_CSV_HEADER_MEMORY std::string("global time,test,memory tag,test mode,buffer size,measurement id,data integrity,write bw,average write bw,read bw,average read bw,write latency,average write latency,read latency,average read latency")

template<typename T> inline bool StrHexToNum( const std::string & hex_str_in, T & value, const int hex_size = -1 )
{
    std::string hex_str = hex_str_in;

    std::transform(hex_str.begin(), hex_str.end(), hex_str.begin(), tolower);

    std::size_t pos = hex_str.find("0x");
    if (pos != 0)
    {
        return RET_FAILURE;
    }
    std::string hex_str_2 = hex_str.substr(pos+2);

    if (hex_size > 0)
    {
        if (hex_str_2.size() > (uint)hex_size)
        {
            return RET_FAILURE;
        }
    }

    std::stringstream ss;
    ss << std::hex << hex_str_2;
    ss >> value;

    // Check convertion
    std::string hex_str_chk = NumToStrHex(value, hex_str_2.size());
    std::transform(hex_str_chk.begin(), hex_str_chk.end(), hex_str_chk.begin(), tolower);

    if (hex_str_chk != hex_str_2)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

template <typename T> inline bool AllocateHostBuffer( const int & page_size, const std::size_t & num, T ** ptr )
{
    void* ptr_tmp = nullptr;
    if (posix_memalign(&ptr_tmp, page_size, num * sizeof(T)))
    {
        return RET_FAILURE;
    }
    if (ptr_tmp == nullptr)
    {
        return RET_FAILURE;
    }
    *ptr = reinterpret_cast<T*>(ptr_tmp);
    return RET_SUCCESS;
}
template <typename T> inline void DeallocateHostBuffer( T * p )
{
    if (p != nullptr)
    {
        free(p);
    }
}

template<typename T> inline bool IsPowerOf2 ( const T & val )
{
    return (val > 0) && ((val & (val - 1)) == 0);
}

#define CU_AP_CTRL_START   (uint32_t)(0x001)
#define CU_AP_CTRL_DONE    (uint32_t)(0x002)
#define CU_AP_CTRL_IDLE    (uint32_t)(0x004)
#define CU_AP_CTRL_ADDR    (uint64_t)(0x000)

#define SCALAR00_ADDR      (uint64_t)(0x010)
#define SCALAR01_ADDR      (uint64_t)(0x018)
#define SCALAR02_ADDR      (uint64_t)(0x020)
#define SCALAR03_ADDR      (uint64_t)(0x028)

#define AXI00_PTR0_ADDR_0    (uint64_t)(0x030)
#define AXI00_PTR0_ADDR_1    (uint64_t)(0x034)
#define AXI01_PTR0_ADDR_0    (uint64_t)(0x038)
#define AXI01_PTR0_ADDR_1    (uint64_t)(0x03c)
#define AXI02_PTR0_ADDR_0    (uint64_t)(0x040)
#define AXI02_PTR0_ADDR_1    (uint64_t)(0x044)
#define AXI03_PTR0_ADDR_0    (uint64_t)(0x048)
#define AXI03_PTR0_ADDR_1    (uint64_t)(0x04c)
#define AXI04_PTR0_ADDR_0    (uint64_t)(0x050)
#define AXI04_PTR0_ADDR_1    (uint64_t)(0x054)
#define AXI05_PTR0_ADDR_0    (uint64_t)(0x058)
#define AXI05_PTR0_ADDR_1    (uint64_t)(0x05c)
#define AXI06_PTR0_ADDR_0    (uint64_t)(0x060)
#define AXI06_PTR0_ADDR_1    (uint64_t)(0x064)
#define AXI07_PTR0_ADDR_0    (uint64_t)(0x068)
#define AXI07_PTR0_ADDR_1    (uint64_t)(0x06c)
#define AXI08_PTR0_ADDR_0    (uint64_t)(0x070)
#define AXI08_PTR0_ADDR_1    (uint64_t)(0x074)
#define AXI09_PTR0_ADDR_0    (uint64_t)(0x078)
#define AXI09_PTR0_ADDR_1    (uint64_t)(0x07c)
#define AXI10_PTR0_ADDR_0    (uint64_t)(0x080)
#define AXI10_PTR0_ADDR_1    (uint64_t)(0x084)
#define AXI11_PTR0_ADDR_0    (uint64_t)(0x088)
#define AXI11_PTR0_ADDR_1    (uint64_t)(0x08c)
#define AXI12_PTR0_ADDR_0    (uint64_t)(0x090)
#define AXI12_PTR0_ADDR_1    (uint64_t)(0x094)
#define AXI13_PTR0_ADDR_0    (uint64_t)(0x098)
#define AXI13_PTR0_ADDR_1    (uint64_t)(0x09c)
#define AXI14_PTR0_ADDR_0    (uint64_t)(0x0a0)
#define AXI14_PTR0_ADDR_1    (uint64_t)(0x0a4)
#define AXI15_PTR0_ADDR_0    (uint64_t)(0x0a8)
#define AXI15_PTR0_ADDR_1    (uint64_t)(0x0ac)
#define AXI16_PTR0_ADDR_0    (uint64_t)(0x0b0)
#define AXI16_PTR0_ADDR_1    (uint64_t)(0x0b4)
#define AXI17_PTR0_ADDR_0    (uint64_t)(0x0b8)
#define AXI17_PTR0_ADDR_1    (uint64_t)(0x0bc)
#define AXI18_PTR0_ADDR_0    (uint64_t)(0x0c0)
#define AXI18_PTR0_ADDR_1    (uint64_t)(0x0c4)
#define AXI19_PTR0_ADDR_0    (uint64_t)(0x0c8)
#define AXI19_PTR0_ADDR_1    (uint64_t)(0x0cc)
#define AXI20_PTR0_ADDR_0    (uint64_t)(0x0d0)
#define AXI20_PTR0_ADDR_1    (uint64_t)(0x0d4)
#define AXI21_PTR0_ADDR_0    (uint64_t)(0x0d8)
#define AXI21_PTR0_ADDR_1    (uint64_t)(0x0dc)
#define AXI22_PTR0_ADDR_0    (uint64_t)(0x0e0)
#define AXI22_PTR0_ADDR_1    (uint64_t)(0x0e4)
#define AXI23_PTR0_ADDR_0    (uint64_t)(0x0e8)
#define AXI23_PTR0_ADDR_1    (uint64_t)(0x0ec)
#define AXI24_PTR0_ADDR_0    (uint64_t)(0x0f0)
#define AXI24_PTR0_ADDR_1    (uint64_t)(0x0f4)
#define AXI25_PTR0_ADDR_0    (uint64_t)(0x0f8)
#define AXI25_PTR0_ADDR_1    (uint64_t)(0x0fc)
#define AXI26_PTR0_ADDR_0    (uint64_t)(0x100)
#define AXI26_PTR0_ADDR_1    (uint64_t)(0x104)
#define AXI27_PTR0_ADDR_0    (uint64_t)(0x108)
#define AXI27_PTR0_ADDR_1    (uint64_t)(0x10c)
#define AXI28_PTR0_ADDR_0    (uint64_t)(0x110)
#define AXI28_PTR0_ADDR_1    (uint64_t)(0x114)
#define AXI29_PTR0_ADDR_0    (uint64_t)(0x118)
#define AXI29_PTR0_ADDR_1    (uint64_t)(0x11c)
#define AXI30_PTR0_ADDR_0    (uint64_t)(0x120)
#define AXI30_PTR0_ADDR_1    (uint64_t)(0x124)
#define AXI31_PTR0_ADDR_0    (uint64_t)(0x128)
#define AXI31_PTR0_ADDR_1    (uint64_t)(0x12c)
#define AXI32_PTR0_ADDR_0    (uint64_t)(0x130)
#define AXI32_PTR0_ADDR_1    (uint64_t)(0x134)
#define AXI33_PTR0_ADDR_0    (uint64_t)(0x138)
#define AXI33_PTR0_ADDR_1    (uint64_t)(0x13c)
#define AXI34_PTR0_ADDR_0    (uint64_t)(0x140)
#define AXI34_PTR0_ADDR_1    (uint64_t)(0x144)
#define AXI35_PTR0_ADDR_0    (uint64_t)(0x148)
#define AXI35_PTR0_ADDR_1    (uint64_t)(0x14c)
#define AXI36_PTR0_ADDR_0    (uint64_t)(0x150)
#define AXI36_PTR0_ADDR_1    (uint64_t)(0x154)
#define AXI37_PTR0_ADDR_0    (uint64_t)(0x158)
#define AXI37_PTR0_ADDR_1    (uint64_t)(0x15c)
#define AXI38_PTR0_ADDR_0    (uint64_t)(0x160)
#define AXI38_PTR0_ADDR_1    (uint64_t)(0x164)
#define AXI39_PTR0_ADDR_0    (uint64_t)(0x168)
#define AXI39_PTR0_ADDR_1    (uint64_t)(0x16c)
#define AXI40_PTR0_ADDR_0    (uint64_t)(0x170)
#define AXI40_PTR0_ADDR_1    (uint64_t)(0x174)
#define AXI41_PTR0_ADDR_0    (uint64_t)(0x178)
#define AXI41_PTR0_ADDR_1    (uint64_t)(0x17c)
#define AXI42_PTR0_ADDR_0    (uint64_t)(0x180)
#define AXI42_PTR0_ADDR_1    (uint64_t)(0x184)
#define AXI43_PTR0_ADDR_0    (uint64_t)(0x188)
#define AXI43_PTR0_ADDR_1    (uint64_t)(0x18c)
#define AXI44_PTR0_ADDR_0    (uint64_t)(0x190)
#define AXI44_PTR0_ADDR_1    (uint64_t)(0x194)
#define AXI45_PTR0_ADDR_0    (uint64_t)(0x198)
#define AXI45_PTR0_ADDR_1    (uint64_t)(0x19c)
#define AXI46_PTR0_ADDR_0    (uint64_t)(0x1a0)
#define AXI46_PTR0_ADDR_1    (uint64_t)(0x1a4)
#define AXI47_PTR0_ADDR_0    (uint64_t)(0x1a8)
#define AXI47_PTR0_ADDR_1    (uint64_t)(0x1ac)
#define AXI48_PTR0_ADDR_0    (uint64_t)(0x1b0)
#define AXI48_PTR0_ADDR_1    (uint64_t)(0x1b4)
#define AXI49_PTR0_ADDR_0    (uint64_t)(0x1b8)
#define AXI49_PTR0_ADDR_1    (uint64_t)(0x1bc)
#define AXI50_PTR0_ADDR_0    (uint64_t)(0x1c0)
#define AXI50_PTR0_ADDR_1    (uint64_t)(0x1c4)
#define AXI51_PTR0_ADDR_0    (uint64_t)(0x1c8)
#define AXI51_PTR0_ADDR_1    (uint64_t)(0x1cc)
#define AXI52_PTR0_ADDR_0    (uint64_t)(0x1d0)
#define AXI52_PTR0_ADDR_1    (uint64_t)(0x1d4)
#define AXI53_PTR0_ADDR_0    (uint64_t)(0x1d8)
#define AXI53_PTR0_ADDR_1    (uint64_t)(0x1dc)
#define AXI54_PTR0_ADDR_0    (uint64_t)(0x1e0)
#define AXI54_PTR0_ADDR_1    (uint64_t)(0x1e4)
#define AXI55_PTR0_ADDR_0    (uint64_t)(0x1e8)
#define AXI55_PTR0_ADDR_1    (uint64_t)(0x1ec)
#define AXI56_PTR0_ADDR_0    (uint64_t)(0x1f0)
#define AXI56_PTR0_ADDR_1    (uint64_t)(0x1f4)
#define AXI57_PTR0_ADDR_0    (uint64_t)(0x1f8)
#define AXI57_PTR0_ADDR_1    (uint64_t)(0x1fc)
#define AXI58_PTR0_ADDR_0    (uint64_t)(0x200)
#define AXI58_PTR0_ADDR_1    (uint64_t)(0x204)
#define AXI59_PTR0_ADDR_0    (uint64_t)(0x208)
#define AXI59_PTR0_ADDR_1    (uint64_t)(0x20c)
#define AXI60_PTR0_ADDR_0    (uint64_t)(0x210)
#define AXI60_PTR0_ADDR_1    (uint64_t)(0x214)
#define AXI61_PTR0_ADDR_0    (uint64_t)(0x218)
#define AXI61_PTR0_ADDR_1    (uint64_t)(0x21c)
#define AXI62_PTR0_ADDR_0    (uint64_t)(0x220)
#define AXI62_PTR0_ADDR_1    (uint64_t)(0x224)
#define AXI63_PTR0_ADDR_0    (uint64_t)(0x228)
#define AXI63_PTR0_ADDR_1    (uint64_t)(0x22c)
#define AXI64_PTR0_ADDR_0    (uint64_t)(0x230)
#define AXI64_PTR0_ADDR_1    (uint64_t)(0x234)

#define SCALAR_ARG_ADDR    std::vector<uint64_t>({ \
    SCALAR00_ADDR, \
    SCALAR01_ADDR, \
    SCALAR02_ADDR, \
    SCALAR03_ADDR \
})

#define AXI_PTR0_ARG_ADDR    std::vector<std::vector<uint64_t>>({ \
    {AXI00_PTR0_ADDR_0,AXI00_PTR0_ADDR_1}, \
    {AXI01_PTR0_ADDR_0,AXI01_PTR0_ADDR_1}, \
    {AXI02_PTR0_ADDR_0,AXI02_PTR0_ADDR_1}, \
    {AXI03_PTR0_ADDR_0,AXI03_PTR0_ADDR_1}, \
    {AXI04_PTR0_ADDR_0,AXI04_PTR0_ADDR_1}, \
    {AXI05_PTR0_ADDR_0,AXI05_PTR0_ADDR_1}, \
    {AXI06_PTR0_ADDR_0,AXI06_PTR0_ADDR_1}, \
    {AXI07_PTR0_ADDR_0,AXI07_PTR0_ADDR_1}, \
    {AXI08_PTR0_ADDR_0,AXI08_PTR0_ADDR_1}, \
    {AXI09_PTR0_ADDR_0,AXI09_PTR0_ADDR_1}, \
    {AXI10_PTR0_ADDR_0,AXI10_PTR0_ADDR_1}, \
    {AXI11_PTR0_ADDR_0,AXI11_PTR0_ADDR_1}, \
    {AXI12_PTR0_ADDR_0,AXI12_PTR0_ADDR_1}, \
    {AXI13_PTR0_ADDR_0,AXI13_PTR0_ADDR_1}, \
    {AXI14_PTR0_ADDR_0,AXI14_PTR0_ADDR_1}, \
    {AXI15_PTR0_ADDR_0,AXI15_PTR0_ADDR_1}, \
    {AXI16_PTR0_ADDR_0,AXI16_PTR0_ADDR_1}, \
    {AXI17_PTR0_ADDR_0,AXI17_PTR0_ADDR_1}, \
    {AXI18_PTR0_ADDR_0,AXI18_PTR0_ADDR_1}, \
    {AXI19_PTR0_ADDR_0,AXI19_PTR0_ADDR_1}, \
    {AXI20_PTR0_ADDR_0,AXI20_PTR0_ADDR_1}, \
    {AXI21_PTR0_ADDR_0,AXI21_PTR0_ADDR_1}, \
    {AXI22_PTR0_ADDR_0,AXI22_PTR0_ADDR_1}, \
    {AXI23_PTR0_ADDR_0,AXI23_PTR0_ADDR_1}, \
    {AXI24_PTR0_ADDR_0,AXI24_PTR0_ADDR_1}, \
    {AXI25_PTR0_ADDR_0,AXI25_PTR0_ADDR_1}, \
    {AXI26_PTR0_ADDR_0,AXI26_PTR0_ADDR_1}, \
    {AXI27_PTR0_ADDR_0,AXI27_PTR0_ADDR_1}, \
    {AXI28_PTR0_ADDR_0,AXI28_PTR0_ADDR_1}, \
    {AXI29_PTR0_ADDR_0,AXI29_PTR0_ADDR_1}, \
    {AXI30_PTR0_ADDR_0,AXI30_PTR0_ADDR_1}, \
    {AXI31_PTR0_ADDR_0,AXI31_PTR0_ADDR_1}, \
    {AXI32_PTR0_ADDR_0,AXI32_PTR0_ADDR_1}, \
    {AXI33_PTR0_ADDR_0,AXI33_PTR0_ADDR_1}, \
    {AXI34_PTR0_ADDR_0,AXI34_PTR0_ADDR_1}, \
    {AXI35_PTR0_ADDR_0,AXI35_PTR0_ADDR_1}, \
    {AXI36_PTR0_ADDR_0,AXI36_PTR0_ADDR_1}, \
    {AXI37_PTR0_ADDR_0,AXI37_PTR0_ADDR_1}, \
    {AXI38_PTR0_ADDR_0,AXI38_PTR0_ADDR_1}, \
    {AXI39_PTR0_ADDR_0,AXI39_PTR0_ADDR_1}, \
    {AXI40_PTR0_ADDR_0,AXI40_PTR0_ADDR_1}, \
    {AXI41_PTR0_ADDR_0,AXI41_PTR0_ADDR_1}, \
    {AXI42_PTR0_ADDR_0,AXI42_PTR0_ADDR_1}, \
    {AXI43_PTR0_ADDR_0,AXI43_PTR0_ADDR_1}, \
    {AXI44_PTR0_ADDR_0,AXI44_PTR0_ADDR_1}, \
    {AXI45_PTR0_ADDR_0,AXI45_PTR0_ADDR_1}, \
    {AXI46_PTR0_ADDR_0,AXI46_PTR0_ADDR_1}, \
    {AXI47_PTR0_ADDR_0,AXI47_PTR0_ADDR_1}, \
    {AXI48_PTR0_ADDR_0,AXI48_PTR0_ADDR_1}, \
    {AXI49_PTR0_ADDR_0,AXI49_PTR0_ADDR_1}, \
    {AXI50_PTR0_ADDR_0,AXI50_PTR0_ADDR_1}, \
    {AXI51_PTR0_ADDR_0,AXI51_PTR0_ADDR_1}, \
    {AXI52_PTR0_ADDR_0,AXI52_PTR0_ADDR_1}, \
    {AXI53_PTR0_ADDR_0,AXI53_PTR0_ADDR_1}, \
    {AXI54_PTR0_ADDR_0,AXI54_PTR0_ADDR_1}, \
    {AXI55_PTR0_ADDR_0,AXI55_PTR0_ADDR_1}, \
    {AXI56_PTR0_ADDR_0,AXI56_PTR0_ADDR_1}, \
    {AXI57_PTR0_ADDR_0,AXI57_PTR0_ADDR_1}, \
    {AXI58_PTR0_ADDR_0,AXI58_PTR0_ADDR_1}, \
    {AXI59_PTR0_ADDR_0,AXI59_PTR0_ADDR_1}, \
    {AXI60_PTR0_ADDR_0,AXI60_PTR0_ADDR_1}, \
    {AXI61_PTR0_ADDR_0,AXI61_PTR0_ADDR_1}, \
    {AXI62_PTR0_ADDR_0,AXI62_PTR0_ADDR_1}, \
    {AXI63_PTR0_ADDR_0,AXI63_PTR0_ADDR_1}, \
    {AXI64_PTR0_ADDR_0,AXI64_PTR0_ADDR_1} \
})

} // namespace

#endif /* _XBTESTSWPACKAGE_H */
