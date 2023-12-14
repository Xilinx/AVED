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

// default_nettype of none prevents implicit wire declaration.
`default_nettype none

`define KRNL_XBTEST_CORE_QSFP_PARAM(INDEX) \
    parameter integer C_QUAD_EN_``INDEX``       = 1, \
    parameter integer C_GT_IP_SEL_``INDEX``     = 2, \
    parameter integer C_GT_RATE_``INDEX``       = 1, \
    parameter integer C_GT_REFCLK_SEL_``INDEX`` = ``INDEX``,

`define MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(INDEX) \
    .C_QUAD_EN_``INDEX``        ( C_QUAD_EN_``INDEX``       ), \
    .C_GT_IP_SEL_``INDEX``      ( C_GT_IP_SEL_``INDEX``     ), \
    .C_GT_RATE_``INDEX``        ( C_GT_RATE_``INDEX``       ), \
    .C_GT_REFCLK_SEL_``INDEX``  ( C_GT_REFCLK_SEL_``INDEX`` ),


`define KRNL_XBTEST_CORE_QSFP_PORT(INDEX) \
    input  wire                             QSFP_CK_N_``INDEX``, \
    input  wire                             QSFP_CK_P_``INDEX``, \
    input  wire [3 : 0]                     QSFP_RX_N_``INDEX``, \
    input  wire [3 : 0]                     QSFP_RX_P_``INDEX``, \
    output wire [3 : 0]                     QSFP_TX_N_``INDEX``, \
    output wire [3 : 0]                     QSFP_TX_P_``INDEX``, \
    output wire [0 : 0]                     gt_recov_clk_n_``INDEX``, \
    output wire [0 : 0]                     gt_recov_clk_p_``INDEX``, \
    output wire                             gt_recov_clk_``INDEX``,

`define MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(INDEX) \
    .QSFP_CK_N_``INDEX``        ( QSFP_CK_N_``INDEX``       ), \
    .QSFP_CK_P_``INDEX``        ( QSFP_CK_P_``INDEX``       ), \
    .QSFP_RX_N_``INDEX``        ( QSFP_RX_N_``INDEX``       ), \
    .QSFP_RX_P_``INDEX``        ( QSFP_RX_P_``INDEX``       ), \
    .QSFP_TX_N_``INDEX``        ( QSFP_TX_N_``INDEX``       ), \
    .QSFP_TX_P_``INDEX``        ( QSFP_TX_P_``INDEX``       ), \
    .gt_recov_clk_n_``INDEX``   ( gt_recov_clk_n_``INDEX``  ), \
    .gt_recov_clk_p_``INDEX``   ( gt_recov_clk_p_``INDEX``  ), \
    .gt_recov_clk_``INDEX``     ( gt_recov_clk_``INDEX``    ),

`define KRNL_XBTEST_CORE_AXI_PARAM(INDEX)                       \
    parameter integer C_M``INDEX``_AXI_THREAD_ID_WIDTH  = 2,    \
    parameter integer C_M``INDEX``_AXI_ADDR_WIDTH       = 64,   \
    parameter integer C_M``INDEX``_AXI_DATA_WIDTH       = 512,

`define KRNL_XBTEST_CORE_AXI_PORT(INDEX)                                        \
    output wire [C_M``INDEX``_AXI_THREAD_ID_WIDTH-1:0]  m``INDEX``_axi_awid,    \
    output wire                                         m``INDEX``_axi_awvalid, \
    input  wire                                         m``INDEX``_axi_awready, \
    output wire [C_M``INDEX``_AXI_ADDR_WIDTH-1:0]       m``INDEX``_axi_awaddr,  \
    output wire [8-1:0]                                 m``INDEX``_axi_awlen,   \
    output wire                                         m``INDEX``_axi_wvalid,  \
    input  wire                                         m``INDEX``_axi_wready,  \
    output wire [C_M``INDEX``_AXI_DATA_WIDTH-1:0]       m``INDEX``_axi_wdata,   \
    output wire [C_M``INDEX``_AXI_DATA_WIDTH/8-1:0]     m``INDEX``_axi_wstrb,   \
    output wire                                         m``INDEX``_axi_wlast,   \
    input  wire [C_M``INDEX``_AXI_THREAD_ID_WIDTH-1:0]  m``INDEX``_axi_bid,     \
    input  wire                                         m``INDEX``_axi_bvalid,  \
    output wire                                         m``INDEX``_axi_bready,  \
                                                                                \
    output wire [C_M``INDEX``_AXI_THREAD_ID_WIDTH-1:0]  m``INDEX``_axi_arid,    \
    output wire                                         m``INDEX``_axi_arvalid, \
    input  wire                                         m``INDEX``_axi_arready, \
    output wire [C_M``INDEX``_AXI_ADDR_WIDTH-1:0]       m``INDEX``_axi_araddr,  \
    output wire [8-1:0]                                 m``INDEX``_axi_arlen,   \
    input  wire [C_M``INDEX``_AXI_THREAD_ID_WIDTH-1:0]  m``INDEX``_axi_rid,     \
    input  wire                                         m``INDEX``_axi_rvalid,  \
    output wire                                         m``INDEX``_axi_rready,  \
    input  wire [C_M``INDEX``_AXI_DATA_WIDTH-1:0]       m``INDEX``_axi_rdata,   \
    input  wire                                         m``INDEX``_axi_rlast,   \
    input  wire [C_M``INDEX``_AXI_ADDR_WIDTH-1:0]       axi``INDEX``_ptr0,

`define MEMTEST_WRAPPER_AXI_PARAM_MAP(INDEX)                                    \
    .C_M``INDEX``_AXI_THREAD_ID_WIDTH   ( C_M``INDEX``_AXI_THREAD_ID_WIDTH  ),  \
    .C_M``INDEX``_AXI_ADDR_WIDTH        ( C_M``INDEX``_AXI_ADDR_WIDTH       ),  \
    .C_M``INDEX``_AXI_DATA_WIDTH        ( C_M``INDEX``_AXI_DATA_WIDTH       ),

`define MEMTEST_WRAPPER_AXI_PORT_MAP(INDEX)                 \
    .m``INDEX``_axi_awid     ( m``INDEX``_axi_awid      ),  \
    .m``INDEX``_axi_awvalid  ( m``INDEX``_axi_awvalid   ),  \
    .m``INDEX``_axi_awready  ( m``INDEX``_axi_awready   ),  \
    .m``INDEX``_axi_awaddr   ( m``INDEX``_axi_awaddr    ),  \
    .m``INDEX``_axi_awlen    ( m``INDEX``_axi_awlen     ),  \
    .m``INDEX``_axi_wvalid   ( m``INDEX``_axi_wvalid    ),  \
    .m``INDEX``_axi_wready   ( m``INDEX``_axi_wready    ),  \
    .m``INDEX``_axi_wdata    ( m``INDEX``_axi_wdata     ),  \
    .m``INDEX``_axi_wstrb    ( m``INDEX``_axi_wstrb     ),  \
    .m``INDEX``_axi_wlast    ( m``INDEX``_axi_wlast     ),  \
    .m``INDEX``_axi_bid      ( m``INDEX``_axi_bid       ),  \
    .m``INDEX``_axi_bvalid   ( m``INDEX``_axi_bvalid    ),  \
    .m``INDEX``_axi_bready   ( m``INDEX``_axi_bready    ),  \
                                                            \
    .m``INDEX``_axi_arid     ( m``INDEX``_axi_arid      ),  \
    .m``INDEX``_axi_arvalid  ( m``INDEX``_axi_arvalid   ),  \
    .m``INDEX``_axi_arready  ( m``INDEX``_axi_arready   ),  \
    .m``INDEX``_axi_araddr   ( m``INDEX``_axi_araddr    ),  \
    .m``INDEX``_axi_arlen    ( m``INDEX``_axi_arlen     ),  \
    .m``INDEX``_axi_rid      ( m``INDEX``_axi_rid       ),  \
    .m``INDEX``_axi_rvalid   ( m``INDEX``_axi_rvalid    ),  \
    .m``INDEX``_axi_rready   ( m``INDEX``_axi_rready    ),  \
    .m``INDEX``_axi_rdata    ( m``INDEX``_axi_rdata     ),  \
    .m``INDEX``_axi_rlast    ( m``INDEX``_axi_rlast     ),  \
    .axi``INDEX``_ptr0       ( axi``INDEX``_ptr0        ),

module krnl_xbtest_core #(
    parameter         C_FAMILY                      = "no_family",
    parameter integer C_BUILD_VERSION               = 0,    // Build version
    parameter integer C_CLOCK0_FREQ                 = 300,  // Frequency for clock0 (ap_clk),
    parameter integer C_CLOCK1_FREQ                 = 500,  // Frequency for clock1 (ap_clk_2)

    parameter integer C_KRNL_MODE                   = 0,    // Validation kernel mode (POWER = 0, MEMORY = 1, GT_10 = 2, GT_25 = 3)
    parameter integer C_KRNL_SLR                    = 0,    // Kernel SLR (SLR0 = 0, SLR1 = 1, SLR2 = 2, SLR3 = 3)
    parameter integer C_GT_INDEX                    = 0,

    parameter integer C_MEM_KRNL_INST               = 0,    // Memory kernel instance
    parameter integer C_NUM_USED_M_AXI              = 1,    // Number of used M_AXI ports 1..32 (enables M01_AXI .. M32_AXI for memory kernel)
    parameter integer C_MEM_TYPE                    = 0,    // 1 single-channel 2 multi-channel
    parameter integer C_USE_AXI_ID                  = 0,    // 1 use axi id, 0 disable

    parameter integer C_THROTTLE_MODE               = 1,
    parameter integer C_USE_AIE                     = 0,    // 0: Disable AIE, 1: Enable AIE
    parameter integer C_AXIS_AIE_DATA_WIDTH         = 128,

    parameter integer C_GT_NUM_GT                   = 1,   //
    parameter integer C_GT_NUM_LANE                 = 4,   // 1 -> 4
    parameter integer C_GT_RATE                     = 0,   // 0: switchable 10/25GbE; 1: fixed 10GbE; 2: fixed 25GbE
    parameter integer C_GT_TYPE                     = 0,   // 0: GTY; 1: GTM
    parameter integer C_GT_MAC_IP_SEL               = 0,   // 0: xxv; 1: xbtest_sub_mac_gt
    parameter integer C_GT_MAC_ENABLE_RSFEC         = 0,

    parameter integer C_GT_IP_SEL                   = 0,  // 0: gt_wizard ; 1: use xbtest_subv_gt

    `KRNL_XBTEST_CORE_QSFP_PARAM(00)
    `KRNL_XBTEST_CORE_QSFP_PARAM(01)
    `KRNL_XBTEST_CORE_QSFP_PARAM(02)
    `KRNL_XBTEST_CORE_QSFP_PARAM(03)
    `KRNL_XBTEST_CORE_QSFP_PARAM(04)
    `KRNL_XBTEST_CORE_QSFP_PARAM(05)
    `KRNL_XBTEST_CORE_QSFP_PARAM(06)
    `KRNL_XBTEST_CORE_QSFP_PARAM(07)
    `KRNL_XBTEST_CORE_QSFP_PARAM(08)
    `KRNL_XBTEST_CORE_QSFP_PARAM(09)
    `KRNL_XBTEST_CORE_QSFP_PARAM(10)
    `KRNL_XBTEST_CORE_QSFP_PARAM(11)
    `KRNL_XBTEST_CORE_QSFP_PARAM(12)
    `KRNL_XBTEST_CORE_QSFP_PARAM(13)
    `KRNL_XBTEST_CORE_QSFP_PARAM(14)
    `KRNL_XBTEST_CORE_QSFP_PARAM(15)
    `KRNL_XBTEST_CORE_QSFP_PARAM(16)
    `KRNL_XBTEST_CORE_QSFP_PARAM(17)
    `KRNL_XBTEST_CORE_QSFP_PARAM(18)
    `KRNL_XBTEST_CORE_QSFP_PARAM(19)
    `KRNL_XBTEST_CORE_QSFP_PARAM(20)
    `KRNL_XBTEST_CORE_QSFP_PARAM(21)
    `KRNL_XBTEST_CORE_QSFP_PARAM(22)
    `KRNL_XBTEST_CORE_QSFP_PARAM(23)
    `KRNL_XBTEST_CORE_QSFP_PARAM(24)
    `KRNL_XBTEST_CORE_QSFP_PARAM(25)
    `KRNL_XBTEST_CORE_QSFP_PARAM(26)
    `KRNL_XBTEST_CORE_QSFP_PARAM(27)
    `KRNL_XBTEST_CORE_QSFP_PARAM(28)
    `KRNL_XBTEST_CORE_QSFP_PARAM(29)
    `KRNL_XBTEST_CORE_QSFP_PARAM(30)
    `KRNL_XBTEST_CORE_QSFP_PARAM(31)

    parameter integer C_DNA_READ                    = 0,

    parameter integer C_M00_AXI_ADDR_WIDTH  = 64,
    parameter integer C_M00_AXI_DATA_WIDTH  = 512,


    `KRNL_XBTEST_CORE_AXI_PARAM(01)
    `KRNL_XBTEST_CORE_AXI_PARAM(02)
    `KRNL_XBTEST_CORE_AXI_PARAM(03)
    `KRNL_XBTEST_CORE_AXI_PARAM(04)
    `KRNL_XBTEST_CORE_AXI_PARAM(05)
    `KRNL_XBTEST_CORE_AXI_PARAM(06)
    `KRNL_XBTEST_CORE_AXI_PARAM(07)
    `KRNL_XBTEST_CORE_AXI_PARAM(08)
    `KRNL_XBTEST_CORE_AXI_PARAM(09)
    `KRNL_XBTEST_CORE_AXI_PARAM(10)
    `KRNL_XBTEST_CORE_AXI_PARAM(11)
    `KRNL_XBTEST_CORE_AXI_PARAM(12)
    `KRNL_XBTEST_CORE_AXI_PARAM(13)
    `KRNL_XBTEST_CORE_AXI_PARAM(14)
    `KRNL_XBTEST_CORE_AXI_PARAM(15)
    `KRNL_XBTEST_CORE_AXI_PARAM(16)
    `KRNL_XBTEST_CORE_AXI_PARAM(17)
    `KRNL_XBTEST_CORE_AXI_PARAM(18)
    `KRNL_XBTEST_CORE_AXI_PARAM(19)
    `KRNL_XBTEST_CORE_AXI_PARAM(20)
    `KRNL_XBTEST_CORE_AXI_PARAM(21)
    `KRNL_XBTEST_CORE_AXI_PARAM(22)
    `KRNL_XBTEST_CORE_AXI_PARAM(23)
    `KRNL_XBTEST_CORE_AXI_PARAM(24)
    `KRNL_XBTEST_CORE_AXI_PARAM(25)
    `KRNL_XBTEST_CORE_AXI_PARAM(26)
    `KRNL_XBTEST_CORE_AXI_PARAM(27)
    `KRNL_XBTEST_CORE_AXI_PARAM(28)
    `KRNL_XBTEST_CORE_AXI_PARAM(29)
    `KRNL_XBTEST_CORE_AXI_PARAM(30)
    `KRNL_XBTEST_CORE_AXI_PARAM(31)
    `KRNL_XBTEST_CORE_AXI_PARAM(32)
    `KRNL_XBTEST_CORE_AXI_PARAM(33)
    `KRNL_XBTEST_CORE_AXI_PARAM(34)
    `KRNL_XBTEST_CORE_AXI_PARAM(35)
    `KRNL_XBTEST_CORE_AXI_PARAM(36)
    `KRNL_XBTEST_CORE_AXI_PARAM(37)
    `KRNL_XBTEST_CORE_AXI_PARAM(38)
    `KRNL_XBTEST_CORE_AXI_PARAM(39)
    `KRNL_XBTEST_CORE_AXI_PARAM(40)
    `KRNL_XBTEST_CORE_AXI_PARAM(41)
    `KRNL_XBTEST_CORE_AXI_PARAM(42)
    `KRNL_XBTEST_CORE_AXI_PARAM(43)
    `KRNL_XBTEST_CORE_AXI_PARAM(44)
    `KRNL_XBTEST_CORE_AXI_PARAM(45)
    `KRNL_XBTEST_CORE_AXI_PARAM(46)
    `KRNL_XBTEST_CORE_AXI_PARAM(47)
    `KRNL_XBTEST_CORE_AXI_PARAM(48)
    `KRNL_XBTEST_CORE_AXI_PARAM(49)
    `KRNL_XBTEST_CORE_AXI_PARAM(50)
    `KRNL_XBTEST_CORE_AXI_PARAM(51)
    `KRNL_XBTEST_CORE_AXI_PARAM(52)
    `KRNL_XBTEST_CORE_AXI_PARAM(53)
    `KRNL_XBTEST_CORE_AXI_PARAM(54)
    `KRNL_XBTEST_CORE_AXI_PARAM(55)
    `KRNL_XBTEST_CORE_AXI_PARAM(56)
    `KRNL_XBTEST_CORE_AXI_PARAM(57)
    `KRNL_XBTEST_CORE_AXI_PARAM(58)
    `KRNL_XBTEST_CORE_AXI_PARAM(59)
    `KRNL_XBTEST_CORE_AXI_PARAM(60)
    `KRNL_XBTEST_CORE_AXI_PARAM(61)
    `KRNL_XBTEST_CORE_AXI_PARAM(62)
    `KRNL_XBTEST_CORE_AXI_PARAM(63)
    `KRNL_XBTEST_CORE_AXI_PARAM(64)

    parameter integer DEST_SYNC_FF          = 4,
    parameter integer DISABLE_SIM_ASSERT    = 0
)
(
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         ap_clk_2,
    input  wire         ap_clk_2_cont,
    input  wire         ap_rst_2,

    input  wire         ap_clk_div2_in,
    output wire         ap_clk_div2_out,
    input  wire         ap_clk_div4_in,
    output wire         ap_clk_div4_out,

    input  wire         watchdog_alarm_in,
    output wire         watchdog_alarm_out,

    input  wire         pwr_clk_in,
    output wire         pwr_clk_out,
    input  wire         pwr_throttle_in,
    output wire         pwr_throttle_out,
    input  wire         pwr_FF_en_in,
    input  wire         pwr_DSP_en_in,
    input  wire         pwr_BRAM_en_in,
    input  wire         pwr_URAM_en_in,
    output wire         pwr_FF_en_out,
    output wire         pwr_DSP_en_out,
    output wire         pwr_BRAM_en_out,
    output wire         pwr_URAM_en_out,

    // Transceiver Interface
    input  wire         QSFP_CK_N,
    input  wire         QSFP_CK_P,
    input  wire [3 : 0] QSFP_RX_N,
    input  wire [3 : 0] QSFP_RX_P,
    output wire [3 : 0] QSFP_TX_N,
    output wire [3 : 0] QSFP_TX_P,

    `KRNL_XBTEST_CORE_QSFP_PORT(00)
    `KRNL_XBTEST_CORE_QSFP_PORT(01)
    `KRNL_XBTEST_CORE_QSFP_PORT(02)
    `KRNL_XBTEST_CORE_QSFP_PORT(03)
    `KRNL_XBTEST_CORE_QSFP_PORT(04)
    `KRNL_XBTEST_CORE_QSFP_PORT(05)
    `KRNL_XBTEST_CORE_QSFP_PORT(06)
    `KRNL_XBTEST_CORE_QSFP_PORT(07)
    `KRNL_XBTEST_CORE_QSFP_PORT(08)
    `KRNL_XBTEST_CORE_QSFP_PORT(09)
    `KRNL_XBTEST_CORE_QSFP_PORT(10)
    `KRNL_XBTEST_CORE_QSFP_PORT(11)
    `KRNL_XBTEST_CORE_QSFP_PORT(12)
    `KRNL_XBTEST_CORE_QSFP_PORT(13)
    `KRNL_XBTEST_CORE_QSFP_PORT(14)
    `KRNL_XBTEST_CORE_QSFP_PORT(15)
    `KRNL_XBTEST_CORE_QSFP_PORT(16)
    `KRNL_XBTEST_CORE_QSFP_PORT(17)
    `KRNL_XBTEST_CORE_QSFP_PORT(18)
    `KRNL_XBTEST_CORE_QSFP_PORT(19)
    `KRNL_XBTEST_CORE_QSFP_PORT(20)
    `KRNL_XBTEST_CORE_QSFP_PORT(21)
    `KRNL_XBTEST_CORE_QSFP_PORT(22)
    `KRNL_XBTEST_CORE_QSFP_PORT(23)
    `KRNL_XBTEST_CORE_QSFP_PORT(24)
    `KRNL_XBTEST_CORE_QSFP_PORT(25)
    `KRNL_XBTEST_CORE_QSFP_PORT(26)
    `KRNL_XBTEST_CORE_QSFP_PORT(27)
    `KRNL_XBTEST_CORE_QSFP_PORT(28)
    `KRNL_XBTEST_CORE_QSFP_PORT(29)
    `KRNL_XBTEST_CORE_QSFP_PORT(30)
    `KRNL_XBTEST_CORE_QSFP_PORT(31)

    // AXI4 master interfaces
    output wire                              m00_axi_awvalid,
    input  wire                              m00_axi_awready,
    output wire [C_M00_AXI_ADDR_WIDTH-1:0]   m00_axi_awaddr,
    output wire [8-1:0]                      m00_axi_awlen,
    output wire                              m00_axi_wvalid,
    input  wire                              m00_axi_wready,
    output wire [C_M00_AXI_DATA_WIDTH-1:0]   m00_axi_wdata,
    output wire [C_M00_AXI_DATA_WIDTH/8-1:0] m00_axi_wstrb,
    output wire                              m00_axi_wlast,
    input  wire                              m00_axi_bvalid,
    output wire                              m00_axi_bready,
    output wire                              m00_axi_arvalid,
    input  wire                              m00_axi_arready,
    output wire [C_M00_AXI_ADDR_WIDTH-1:0]   m00_axi_araddr,
    output wire [8-1:0]                      m00_axi_arlen,
    input  wire                              m00_axi_rvalid,
    output wire                              m00_axi_rready,
    input  wire [C_M00_AXI_DATA_WIDTH-1:0]   m00_axi_rdata,
    input  wire                              m00_axi_rlast,
    input  wire [C_M00_AXI_ADDR_WIDTH-1:0]   axi00_ptr0,

    `KRNL_XBTEST_CORE_AXI_PORT(01)
    `KRNL_XBTEST_CORE_AXI_PORT(02)
    `KRNL_XBTEST_CORE_AXI_PORT(03)
    `KRNL_XBTEST_CORE_AXI_PORT(04)
    `KRNL_XBTEST_CORE_AXI_PORT(05)
    `KRNL_XBTEST_CORE_AXI_PORT(06)
    `KRNL_XBTEST_CORE_AXI_PORT(07)
    `KRNL_XBTEST_CORE_AXI_PORT(08)
    `KRNL_XBTEST_CORE_AXI_PORT(09)
    `KRNL_XBTEST_CORE_AXI_PORT(10)
    `KRNL_XBTEST_CORE_AXI_PORT(11)
    `KRNL_XBTEST_CORE_AXI_PORT(12)
    `KRNL_XBTEST_CORE_AXI_PORT(13)
    `KRNL_XBTEST_CORE_AXI_PORT(14)
    `KRNL_XBTEST_CORE_AXI_PORT(15)
    `KRNL_XBTEST_CORE_AXI_PORT(16)
    `KRNL_XBTEST_CORE_AXI_PORT(17)
    `KRNL_XBTEST_CORE_AXI_PORT(18)
    `KRNL_XBTEST_CORE_AXI_PORT(19)
    `KRNL_XBTEST_CORE_AXI_PORT(20)
    `KRNL_XBTEST_CORE_AXI_PORT(21)
    `KRNL_XBTEST_CORE_AXI_PORT(22)
    `KRNL_XBTEST_CORE_AXI_PORT(23)
    `KRNL_XBTEST_CORE_AXI_PORT(24)
    `KRNL_XBTEST_CORE_AXI_PORT(25)
    `KRNL_XBTEST_CORE_AXI_PORT(26)
    `KRNL_XBTEST_CORE_AXI_PORT(27)
    `KRNL_XBTEST_CORE_AXI_PORT(28)
    `KRNL_XBTEST_CORE_AXI_PORT(29)
    `KRNL_XBTEST_CORE_AXI_PORT(30)
    `KRNL_XBTEST_CORE_AXI_PORT(31)
    `KRNL_XBTEST_CORE_AXI_PORT(32)
    `KRNL_XBTEST_CORE_AXI_PORT(33)
    `KRNL_XBTEST_CORE_AXI_PORT(34)
    `KRNL_XBTEST_CORE_AXI_PORT(35)
    `KRNL_XBTEST_CORE_AXI_PORT(36)
    `KRNL_XBTEST_CORE_AXI_PORT(37)
    `KRNL_XBTEST_CORE_AXI_PORT(38)
    `KRNL_XBTEST_CORE_AXI_PORT(39)
    `KRNL_XBTEST_CORE_AXI_PORT(40)
    `KRNL_XBTEST_CORE_AXI_PORT(41)
    `KRNL_XBTEST_CORE_AXI_PORT(42)
    `KRNL_XBTEST_CORE_AXI_PORT(43)
    `KRNL_XBTEST_CORE_AXI_PORT(44)
    `KRNL_XBTEST_CORE_AXI_PORT(45)
    `KRNL_XBTEST_CORE_AXI_PORT(46)
    `KRNL_XBTEST_CORE_AXI_PORT(47)
    `KRNL_XBTEST_CORE_AXI_PORT(48)
    `KRNL_XBTEST_CORE_AXI_PORT(49)
    `KRNL_XBTEST_CORE_AXI_PORT(50)
    `KRNL_XBTEST_CORE_AXI_PORT(51)
    `KRNL_XBTEST_CORE_AXI_PORT(52)
    `KRNL_XBTEST_CORE_AXI_PORT(53)
    `KRNL_XBTEST_CORE_AXI_PORT(54)
    `KRNL_XBTEST_CORE_AXI_PORT(55)
    `KRNL_XBTEST_CORE_AXI_PORT(56)
    `KRNL_XBTEST_CORE_AXI_PORT(57)
    `KRNL_XBTEST_CORE_AXI_PORT(58)
    `KRNL_XBTEST_CORE_AXI_PORT(59)
    `KRNL_XBTEST_CORE_AXI_PORT(60)
    `KRNL_XBTEST_CORE_AXI_PORT(61)
    `KRNL_XBTEST_CORE_AXI_PORT(62)
    `KRNL_XBTEST_CORE_AXI_PORT(63)
    `KRNL_XBTEST_CORE_AXI_PORT(64)

    output wire [C_AXIS_AIE_DATA_WIDTH-1:0]     m_axis_aie0_tdata,
    output wire                                 m_axis_aie0_tvalid,
    input  wire                                 m_axis_aie0_tready,
    output wire  [C_AXIS_AIE_DATA_WIDTH/8-1:0]  m_axis_aie0_tkeep,
    output wire                                 m_axis_aie0_tlast,

    input  wire [C_AXIS_AIE_DATA_WIDTH-1:0]     s_axis_aie0_tdata,
    input  wire                                 s_axis_aie0_tvalid,
    output wire                                 s_axis_aie0_tready,
    input  wire  [C_AXIS_AIE_DATA_WIDTH/8-1:0]  s_axis_aie0_tkeep,
    input  wire                                 s_axis_aie0_tlast,

    // SDx Control Signals
    input  wire                              ap_start,
    output wire                              ap_idle,
    output wire                              ap_done,
    input  wire [32-1:0]                     scalar00,
    input  wire [32-1:0]                     scalar01,
    input  wire [32-1:0]                     scalar02,
    input  wire [32-1:0]                     scalar03
);

    ///////////////////////////////////////////////////////////////////////////////
    // Local Parameters
    ///////////////////////////////////////////////////////////////////////////////
    // Large enough for interesting traffic.
    localparam integer  LP_DEFAULT_LENGTH_IN_BYTES  = 16384;
    localparam integer  LP_NUM_EXAMPLES             = 1;

    ///////////////////////////////////////////////////////////////////////////////
    // Wires and Variables
    ///////////////////////////////////////////////////////////////////////////////
    wire         ap_done_0;
    wire         ap_start_pulse;
    logic        ap_start_r                 = 1'b0;
    logic        ap_idle_r                  = 1'b1;
    logic [LP_NUM_EXAMPLES-1:0] ap_done_r   = {LP_NUM_EXAMPLES{1'b0}};

    ///////////////////////////////////////////////////////////////////////////////
    // Begin RTL
    ///////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////
    // create pulse when ap_start transitions to 1
    always_ff @(posedge ap_clk) begin
        ap_start_r <= ap_start;
    end

    assign ap_start_pulse = ap_start & ~ap_start_r;

    // ap_idle is asserted when done is asserted, it is de-asserted when ap_start_pulse
    // is asserted
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin
            ap_idle_r <= 1'b1;
        end else begin
            ap_idle_r <= ap_done ? 1'b1 : ap_start_pulse ? 1'b0 : ap_idle;
        end
    end

    assign ap_idle = ap_idle_r;

    // Done logic
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin
            ap_done_r <= '0;
        end else begin
            ap_done_r <= (ap_start_pulse | ap_done) ? '0 : ap_done_r | ap_done_0;
        end
    end

    assign ap_done = &ap_done_r;

    generate
        ///////////////////////////////////////////////////////////////////////////////
        // POWER MODE
        ///////////////////////////////////////////////////////////////////////////////
        if (C_KRNL_MODE == 0) begin
            powertest_top # (
                .C_FAMILY               ( C_FAMILY              ),
                .C_BUILD_VERSION        ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ          ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ          ( C_CLOCK1_FREQ         ),
                .C_BLOCK_ID             ( C_KRNL_MODE           ),
                .C_KRNL_SLR             ( C_KRNL_SLR            ),
                .C_THROTTLE_MODE        ( C_THROTTLE_MODE       ),
                .C_USE_AIE              ( C_USE_AIE             ),
                .C_AXIS_AIE_DATA_WIDTH  ( C_AXIS_AIE_DATA_WIDTH ),
                .C_PLRAM_ADDR_WIDTH     ( C_M00_AXI_ADDR_WIDTH  ),
                .C_PLRAM_DATA_WIDTH     ( C_M00_AXI_DATA_WIDTH  ),
                .DEST_SYNC_FF           ( DEST_SYNC_FF          ),
                .DISABLE_SIM_ASSERT     ( DISABLE_SIM_ASSERT    )
            ) inst_powertest_top (
                .ap_clk             ( ap_clk                ),
                .ap_clk_cont        ( ap_clk_cont           ),
                .ap_rst             ( ap_rst                ),
                .ap_clk_2           ( ap_clk_2              ),
                .ap_clk_2_cont      ( ap_clk_2_cont         ),
                .ap_rst_2           ( ap_rst_2              ),

                .watchdog_alarm_in  ( watchdog_alarm_in     ),

                .pwr_clk_in         ( pwr_clk_in            ),
                .pwr_clk_out        ( pwr_clk_out           ),
                .pwr_throttle_in    ( pwr_throttle_in       ),
                .pwr_throttle_out   ( pwr_throttle_out      ),

                .pwr_FF_en_in       ( pwr_FF_en_in      ),
                .pwr_DSP_en_in      ( pwr_DSP_en_in     ),
                .pwr_BRAM_en_in     ( pwr_BRAM_en_in    ),
                .pwr_URAM_en_in     ( pwr_URAM_en_in    ),
                .pwr_FF_en_out      ( pwr_FF_en_out     ),
                .pwr_DSP_en_out     ( pwr_DSP_en_out    ),
                .pwr_BRAM_en_out    ( pwr_BRAM_en_out   ),
                .pwr_URAM_en_out    ( pwr_URAM_en_out   ),

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .m_axis_aie0_tdata  ( m_axis_aie0_tdata     ),
                .m_axis_aie0_tvalid ( m_axis_aie0_tvalid    ),
                .m_axis_aie0_tready ( m_axis_aie0_tready    ),
                .m_axis_aie0_tkeep  ( m_axis_aie0_tkeep     ),
                .m_axis_aie0_tlast  ( m_axis_aie0_tlast     ),

                .s_axis_aie0_tdata  ( s_axis_aie0_tdata     ),
                .s_axis_aie0_tvalid ( s_axis_aie0_tvalid    ),
                .s_axis_aie0_tready ( s_axis_aie0_tready    ),
                .s_axis_aie0_tkeep  ( s_axis_aie0_tkeep     ),
                .s_axis_aie0_tlast  ( s_axis_aie0_tlast     ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),
                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end

        ///////////////////////////////////////////////////////////////////////////////
        // MEMORY MODE
        ///////////////////////////////////////////////////////////////////////////////

        if (C_KRNL_MODE == 1) begin
            memtest_wrapper #(
                .C_BUILD_VERSION            ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ              ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ              ( C_CLOCK1_FREQ         ),
                .C_BLOCK_ID                 ( C_KRNL_MODE           ),
                .C_MEM_KRNL_INST            ( C_MEM_KRNL_INST       ),
                .C_NUM_USED_M_AXI           ( C_NUM_USED_M_AXI      ),
                .C_MEM_TYPE                 ( C_MEM_TYPE            ),
                .C_USE_AXI_ID               ( C_USE_AXI_ID          ),
                .C_PLRAM_ADDR_WIDTH         ( C_M00_AXI_ADDR_WIDTH  ),
                .C_PLRAM_DATA_WIDTH         ( C_M00_AXI_DATA_WIDTH  ),

                `MEMTEST_WRAPPER_AXI_PARAM_MAP(01)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(02)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(03)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(04)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(05)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(06)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(07)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(08)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(09)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(10)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(11)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(12)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(13)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(14)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(15)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(16)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(17)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(18)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(19)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(20)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(21)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(22)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(23)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(24)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(25)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(26)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(27)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(28)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(29)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(30)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(31)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(32)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(33)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(34)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(35)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(36)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(37)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(38)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(39)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(40)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(41)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(42)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(43)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(44)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(45)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(46)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(47)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(48)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(49)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(50)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(51)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(52)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(53)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(54)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(55)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(56)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(57)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(58)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(59)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(60)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(61)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(62)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(63)
                `MEMTEST_WRAPPER_AXI_PARAM_MAP(64)

                .DEST_SYNC_FF               ( DEST_SYNC_FF          )
            ) u_memtest_wrapper_0 (
                .ap_clk             ( ap_clk        ),
                .ap_clk_cont        ( ap_clk_cont   ),
                .ap_rst             ( ap_rst        ),

                .watchdog_alarm_in  ( watchdog_alarm_in     ),

                `MEMTEST_WRAPPER_AXI_PORT_MAP(01)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(02)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(03)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(04)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(05)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(06)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(07)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(08)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(09)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(10)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(11)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(12)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(13)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(14)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(15)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(16)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(17)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(18)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(19)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(20)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(21)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(22)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(23)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(24)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(25)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(26)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(27)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(28)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(29)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(30)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(31)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(32)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(33)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(34)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(35)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(36)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(37)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(38)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(39)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(40)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(41)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(42)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(43)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(44)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(45)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(46)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(47)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(48)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(49)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(50)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(51)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(52)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(53)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(54)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(55)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(56)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(57)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(58)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(59)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(60)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(61)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(62)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(63)
                `MEMTEST_WRAPPER_AXI_PORT_MAP(64)

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),
                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end
        ///////////////////////////////////////////////////////////////////////////////
        // C_KRNL_MODE == 2: GT MODE (QSFP - 10GbE)
        //
        //  NOT SUPORTED
        //
        ///////////////////////////////////////////////////////////////////////////////
        if (C_KRNL_MODE == 3) begin
            gt_lpbk_25_top # (
                .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ         ),
                .C_BLOCK_ID         ( C_KRNL_MODE           ),
                .C_GT_INDEX         ( C_GT_INDEX            ),
                .C_PLRAM_ADDR_WIDTH ( C_M00_AXI_ADDR_WIDTH  ),
                .C_PLRAM_DATA_WIDTH ( C_M00_AXI_DATA_WIDTH  ),
                .DEST_SYNC_FF       ( DEST_SYNC_FF          ),
                .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT    )
            ) u_gt_lpbk_25_top (
                .ap_clk             ( ap_clk                ),
                .ap_clk_cont        ( ap_clk_cont           ),
                .ap_rst             ( ap_rst                ),

                // watchdog not used by this CU
                //.watchdog_alarm_in  ( watchdog_alarm_in     ),

                .QSFP_CK_N          ( QSFP_CK_N             ),
                .QSFP_CK_P          ( QSFP_CK_P             ),
                .QSFP_RX_N          ( QSFP_RX_N             ),
                .QSFP_RX_P          ( QSFP_RX_P             ),
                .QSFP_TX_N          ( QSFP_TX_N             ),
                .QSFP_TX_P          ( QSFP_TX_P             ),

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),
                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end
        ///////////////////////////////////////////////////////////////////////////////
        // GT MODE (QSFP - XXV Ethernet MAC)
        ///////////////////////////////////////////////////////////////////////////////
        if (C_KRNL_MODE == 4) begin
            gt_test_mac_top #(
                .C_BUILD_VERSION        ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ          ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ          ( C_CLOCK1_FREQ         ),
                .C_BLOCK_ID             ( C_KRNL_MODE           ),
                .C_GT_INDEX             ( C_GT_INDEX            ),
                .C_GT_NUM_GT            ( C_GT_NUM_GT           ),
                .C_GT_NUM_LANE          ( C_GT_NUM_LANE         ),
                .C_GT_RATE              ( C_GT_RATE             ),
                .C_GT_TYPE              ( C_GT_TYPE             ),
                .C_GT_MAC_IP_SEL        ( C_GT_MAC_IP_SEL       ),
                .C_GT_MAC_ENABLE_RSFEC  ( C_GT_MAC_ENABLE_RSFEC ),
                .DEST_SYNC_FF           ( DEST_SYNC_FF          )
            ) inst_gt_test_top (
                .ap_clk             ( ap_clk                ),
                .ap_clk_cont        ( ap_clk_cont           ),
                .ap_rst             ( ap_rst                ),

                .watchdog_alarm_in  ( watchdog_alarm_in     ),

                .QSFP_CK_N          ( QSFP_CK_N             ),
                .QSFP_CK_P          ( QSFP_CK_P             ),
                .QSFP_RX_N          ( QSFP_RX_N             ),
                .QSFP_RX_P          ( QSFP_RX_P             ),
                .QSFP_TX_N          ( QSFP_TX_N             ),
                .QSFP_TX_P          ( QSFP_TX_P             ),

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),

                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end

        if (C_KRNL_MODE == 5) begin
            verify_top # (
                .C_BUILD_VERSION        ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ          ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ          ( C_CLOCK1_FREQ         ),
                .C_BLOCK_ID             ( C_KRNL_MODE           ),
                .C_DNA_READ             ( C_DNA_READ            ),
                .DEST_SYNC_FF           ( DEST_SYNC_FF          ),
                .DISABLE_SIM_ASSERT     ( DISABLE_SIM_ASSERT    )
            ) inst_verify_top (
                .ap_clk             ( ap_clk                ),
                .ap_clk_cont        ( ap_clk_cont           ),
                .ap_rst             ( ap_rst                ),
                .ap_clk_2           ( ap_clk_2              ),
                .ap_clk_2_cont      ( ap_clk_2_cont         ),
                .ap_rst_2           ( ap_rst_2              ),

                .ap_clk_div2_out    ( ap_clk_div2_out       ),
                .ap_clk_div4_out    ( ap_clk_div4_out       ),

                .watchdog_alarm_out ( watchdog_alarm_out    ),

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),
                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end

        if (C_KRNL_MODE == 6) begin
            gt_prbs_25_top # (
                .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ         ),
                .C_BLOCK_ID         ( C_KRNL_MODE           ),
                .C_GT_INDEX         ( C_GT_INDEX            ),
                .C_PLRAM_ADDR_WIDTH ( C_M00_AXI_ADDR_WIDTH  ),
                .C_PLRAM_DATA_WIDTH ( C_M00_AXI_DATA_WIDTH  ),
                .DEST_SYNC_FF       ( DEST_SYNC_FF          ),
                .C_GT_IP_SEL        ( C_GT_IP_SEL           ),
                .C_GT_RATE          ( C_GT_RATE             ),
                .C_GT_NUM_LANE      ( C_GT_NUM_LANE         ),
                .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT    )
            ) u_gt_prbs_25_top (
                .ap_clk             ( ap_clk                ),
                .ap_clk_cont        ( ap_clk_cont           ),
                .ap_rst             ( ap_rst                ),

                .ap_clk_div2_in     ( ap_clk_div2_in        ),

                .watchdog_alarm_in  ( watchdog_alarm_in     ),

                .QSFP_CK_N          ( QSFP_CK_N             ),
                .QSFP_CK_P          ( QSFP_CK_P             ),
                .QSFP_RX_N          ( QSFP_RX_N             ),
                .QSFP_RX_P          ( QSFP_RX_P             ),
                .QSFP_TX_N          ( QSFP_TX_N             ),
                .QSFP_TX_P          ( QSFP_TX_P             ),

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),
                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end

        if ((C_KRNL_MODE == 7) || (C_KRNL_MODE == 8) || (C_KRNL_MODE == 9) ) begin

            multi_gt_prbs_wrapper # (
                .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
                .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ         ),
                .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ         ),
                .C_KRNL_MODE        ( C_KRNL_MODE           ),
                .C_PLRAM_ADDR_WIDTH ( C_M00_AXI_ADDR_WIDTH  ),
                .C_PLRAM_DATA_WIDTH ( C_M00_AXI_DATA_WIDTH  ),
                .DEST_SYNC_FF       ( DEST_SYNC_FF          ),

                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(00)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(01)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(02)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(03)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(04)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(05)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(06)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(07)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(08)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(09)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(10)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(11)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(12)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(13)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(14)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(15)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(16)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(17)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(18)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(19)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(20)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(21)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(22)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(23)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(24)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(25)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(26)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(27)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(28)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(29)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(30)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM_MAP(31)

                .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT    )
            ) u_multi_gt_prbs_wrapper (
                .ap_clk             ( ap_clk                ),
                .ap_clk_cont        ( ap_clk_cont           ),
                .ap_rst             ( ap_rst                ),
                .ap_clk_2           ( ap_clk_2              ),
                .ap_clk_2_cont      ( ap_clk_2_cont         ),
                .ap_rst_2           ( ap_rst_2              ),
                .ap_clk_div2_in     ( ap_clk_div2_in        ),

                .watchdog_alarm_in  ( watchdog_alarm_in     ),

                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(00)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(01)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(02)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(03)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(04)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(05)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(06)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(07)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(08)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(09)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(10)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(11)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(12)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(13)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(14)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(15)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(16)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(17)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(18)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(19)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(20)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(21)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(22)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(23)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(24)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(25)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(26)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(27)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(28)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(29)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(30)
                `MULTI_GT_PRBS_WRAPPER_QSFP_PORT_MAP(31)

                .plram_awvalid      ( m00_axi_awvalid       ),
                .plram_awready      ( m00_axi_awready       ),
                .plram_awaddr       ( m00_axi_awaddr        ),
                .plram_awlen        ( m00_axi_awlen         ),
                .plram_wvalid       ( m00_axi_wvalid        ),
                .plram_wready       ( m00_axi_wready        ),
                .plram_wdata        ( m00_axi_wdata         ),
                .plram_wstrb        ( m00_axi_wstrb         ),
                .plram_wlast        ( m00_axi_wlast         ),
                .plram_bvalid       ( m00_axi_bvalid        ),
                .plram_bready       ( m00_axi_bready        ),
                .plram_arvalid      ( m00_axi_arvalid       ),
                .plram_arready      ( m00_axi_arready       ),
                .plram_araddr       ( m00_axi_araddr        ),
                .plram_arlen        ( m00_axi_arlen         ),
                .plram_rvalid       ( m00_axi_rvalid        ),
                .plram_rready       ( m00_axi_rready        ),
                .plram_rdata        ( m00_axi_rdata         ),
                .plram_rlast        ( m00_axi_rlast         ),

                .axi00_ptr0         ( axi00_ptr0            ),
                .scalar00           ( scalar00              ),
                .scalar01           ( scalar01              ),
                .scalar02           ( scalar02              ),
                .scalar03           ( scalar03              ),
                .start_pulse        ( ap_start_pulse        ),
                .done_pulse         ( ap_done_0             )
            );
        end



    endgenerate

endmodule : krnl_xbtest_core
`default_nettype wire
