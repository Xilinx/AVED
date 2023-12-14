
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

module gt_test_mac_top #(
    parameter integer C_MAJOR_VERSION               = 3,    // Major version
    parameter integer C_MINOR_VERSION               = 0,    // Minor version
    parameter integer C_BUILD_VERSION               = 0,    // Build version
    parameter integer C_CLOCK0_FREQ                 = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ                 = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer C_BLOCK_ID                    = 0,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_GT_INDEX                    = 0,
    parameter integer C_GT_NUM_GT                   = 1,   // 1 or 2
    parameter integer C_GT_NUM_LANE                 = 4,   // 1 -> 4,
    parameter integer C_GT_RATE                     = 0,   // 0: switchable 10/25GbE; 1: fixed 10GbE; 2: fixed 25GbE
    parameter integer C_GT_TYPE                     = 0,   // 0: GTY, ; 1: GTM
    parameter integer C_GT_MAC_IP_SEL               = 0,   // 0: xxv, ; 1: xbtest_sub_mac_gt
    parameter integer C_GT_MAC_ENABLE_RSFEC         = 0,
    parameter integer DEST_SYNC_FF                  = 4,
    parameter integer DISABLE_SIM_ASSERT            = 0
) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,

    input  wire         watchdog_alarm_in,

    input  wire                              QSFP_CK_P,
    input  wire                              QSFP_CK_N,
    input  wire [C_GT_NUM_LANE - 1 : 0]      QSFP_RX_N,
    input  wire [C_GT_NUM_LANE - 1 : 0]      QSFP_RX_P,
    output wire [C_GT_NUM_LANE - 1 : 0]      QSFP_TX_N,
    output wire [C_GT_NUM_LANE - 1 : 0]      QSFP_TX_P,

    output wire         plram_awvalid,
    input  wire         plram_awready,
    output wire [63:0]  plram_awaddr,
    output wire [7:0]   plram_awlen,
    output wire         plram_wvalid,
    input  wire         plram_wready,
    output wire [31:0]  plram_wdata,
    output wire [3:0]   plram_wstrb,
    output wire         plram_wlast,
    input  wire         plram_bvalid,
    output wire         plram_bready,
    output wire         plram_arvalid,
    input  wire         plram_arready,
    output wire [63:0]  plram_araddr,
    output wire [7:0]   plram_arlen,
    input  wire         plram_rvalid,
    output wire         plram_rready,
    input  wire [31:0]  plram_rdata,
    input  wire         plram_rlast,

    // SDx Control Signals
    input  wire [63:0]  axi00_ptr0,
    input  wire [31:0]  scalar00,
    input  wire [31:0]  scalar01,
    input  wire [31:0]  scalar02,
    input  wire [31:0]  scalar03,

    input  wire         start_pulse,
    output logic        done_pulse
);
    timeunit 1ps;
    timeprecision 1ps;

    localparam integer SIM_DIVIDER
    // synthesis translate_off
                                    = 500;
    localparam integer DUMMY_DIVIDER
    // synthesis translate_on
                                    = 1;

    logic         ap_rst_gate         ;
    wire          ap_rst_allow        ;

    wire  [0:3]   rx_core_clk         ;
    wire  [0:3]   rx_clk_out          ;
    wire  [0:3]   tx_clk_out          ;

    wire  [3:0]   rx_reset            ;
    wire  [3:0]   user_rx_reset       ;
    wire  [3:0]   mac_user_rx_reset   ;
    wire  [3:0]   stat_rx_status      ;
    wire  [3:0]   mac_stat_rx_status  ;
    wire  [3:0]   tx_reset            ;
    wire  [3:0]   user_tx_reset       ;
    wire  [3:0]   mac_user_tx_reset   ;

    wire [0:3]    rx_axis_tvalid      ;
    wire [63:0]   rx_axis_tdata       [0:3];
    wire [0:3]    rx_axis_tlast       ;
    wire [7:0]    rx_axis_tkeep       [0:3];
    wire [0:3]    rx_axis_tuser       ;

    wire [0:3]    tx_axis_tready      ;
    wire [0:3]    tx_axis_tvalid      ;
    wire [63:0]   tx_axis_tdata       [0:3];
    wire [0:3]    tx_axis_tlast       ;
    wire [7:0]    tx_axis_tkeep       [0:3];
    wire [0:3]    tx_axis_tuser       ;


    wire [0:3]    Config_Upd          ;
    wire [47:0]   Dest_Addr           [0:3];
    wire [47:0]   Source_Addr         [0:3];
    wire [15:0]   Ether_Type          [0:3];
    wire [1:0]    Payload_Type        [0:3];
    wire [0:3]    Gen_Pkt             ;
    wire [13:0]   Gen_Length          [0:3];
    wire [0:3]    Gen_Half_Full       ;
    wire [0:3]    Gen_Full            ;

    wire          s_axi_aclk;
    wire          s_axi_aresetn;

    wire [31:0]   s_axi_awaddr        [0:3];
    wire [0:3]    s_axi_awvalid       ;
    wire [0:3]    s_axi_awready       ;
    wire [31:0]   s_axi_wdata         [0:3];
    wire [3:0]    s_axi_wstrb         [0:3];
    wire [0:3]    s_axi_wvalid        ;
    wire [0:3]    s_axi_wready        ;
    wire [1:0]    s_axi_bresp         [0:3];
    wire [0:3]    s_axi_bvalid        ;
    wire [0:3]    s_axi_bready        ;
    wire [31:0]   s_axi_araddr        [0:3];
    wire [0:3]    s_axi_arvalid       ;
    wire [0:3]    s_axi_arready       ;
    wire [31:0]   s_axi_rdata         [0:3];
    wire [1:0]    s_axi_rresp         [0:3];
    wire [0:3]    s_axi_rvalid        ;
    wire [0:3]    s_axi_rready        ;

    wire          MAC_aresetn;
    wire [0:3]    MAC_local_fault     ;

    wire [0:3]    ctl_rx_rate         ;

    wire [55:0]   tx_preamblein       [0:3];
    wire [0:3]    ctl_tx_send_lfi     ;
    wire [0:3]    ctl_tx_send_rfi     ;
    wire [0:3]    ctl_tx_send_idle    ;

    wire [0:3]    gt_eyescanreset;
    wire [0:3]    gt_eyescantrigger;
    wire [15:0]   gt_pcsrsvdin        [0:3];
    wire [0:3]    gt_rxbufreset;
    wire [0:3]    gt_rxcdrhold;
    wire [0:3]    gt_rxcommadeten;
    wire [0:3]    gt_rxdfeagchold;
    wire [0:3]    gt_rxdfelpmreset;
    wire [0:3]    gt_rxlatclk;
    wire [0:3]    gt_rxlpmen;
    wire [0:3]    gt_rxpcsreset;
    wire [0:3]    gt_rxpmareset;
    wire [0:3]    gt_rxpolarity;
    wire [0:3]    gt_rxprbscntreset;
    wire [3:0]    gt_rxprbssel        [0:3];
    wire [2:0]    gt_rxrate           [0:3];
    wire [0:3]    gt_rxslide_in;
    wire [0:3]    gt_txinhibit;
    wire [0:3]    gt_txlatclk;
    wire [6:0]    gt_txmaincursor     [0:3];
    wire [0:3]    gt_txpcsreset;
    wire [0:3]    gt_txpmareset;
    wire [0:3]    gt_txpolarity;
    wire [0:3]    gt_tx_polarity;
    wire [4:0]    gt_txpostcursor     [0:3];
    wire [0:3]    gt_txprbsforceerr;
    wire [0:3]    gt_txelecidle;
    wire [3:0]    gt_txprbssel        [0:3];
    wire [4:0]    gt_txprecursor      [0:3];
    wire [4:0]    gt_txdiffctrl       [0:3];

    wire          gt_drpclk;
    wire          gt_drprst;
    wire [15:0]   gt_drpdo            [0:3];
    wire [0:0]    gt_drprdy           [0:3];
    wire [0:0]    gt_drpen            [0:3];
    wire [0:0]    gt_drpwe            [0:3];
    wire [9:0]    gt_drpaddr          [0:3];
    wire [15:0]   gt_drpdi            [0:3];
    wire [0:3]    gt_drp_done         ;

    wire [0:3]    axi_ctl_md_switch   ;
    wire [3:0]    rxdfecfokfcnum      [0:3];
    wire [0:3]    rxafecfoken         ;
    wire [1:0]    txpllclksel_in      [0:3];
    wire [1:0]    rxpllclksel_in      [0:3];
    wire [1:0]    txsysclksel_in      [0:3];
    wire [1:0]    rxsysclksel_in      [0:3];
    wire [2:0]    txoutclksel_in      [0:3];
    wire [2:0]    rxoutclksel_in      [0:3];

    wire [0:3]    mode_change         ;
    wire [0:3]    pm_tick             ;

    wire [11:0]   mac_rst_status      ;

    wire          Traffic_Cnt_Latch;

    wire [3:0]    Cnt_RAM_Rd_Addr     [0:3];
    wire [31:0]   Cnt_RAM_Out         [0:3];

    wire rxrecclkout_0;
    wire [55:0] rx_preambleout_0;
    wire stat_rx_block_lock_0;
    wire stat_rx_framing_err_valid_0;
    wire stat_rx_framing_err_0;
    wire stat_rx_hi_ber_0;
    wire stat_rx_valid_ctrl_code_0;
    wire stat_rx_bad_code_0;
    wire [1:0] stat_rx_total_packets_0;
    wire stat_rx_total_good_packets_0;
    wire [3:0] stat_rx_total_bytes_0;
    wire [13:0] stat_rx_total_good_bytes_0;
    wire stat_rx_packet_small_0;
    wire stat_rx_jabber_0;
    wire stat_rx_packet_large_0;
    wire stat_rx_oversize_0;
    wire stat_rx_undersize_0;
    wire stat_rx_toolong_0;
    wire stat_rx_fragment_0;
    wire stat_rx_packet_64_bytes_0;
    wire stat_rx_packet_65_127_bytes_0;
    wire stat_rx_packet_128_255_bytes_0;
    wire stat_rx_packet_256_511_bytes_0;
    wire stat_rx_packet_512_1023_bytes_0;
    wire stat_rx_packet_1024_1518_bytes_0;
    wire stat_rx_packet_1519_1522_bytes_0;
    wire stat_rx_packet_1523_1548_bytes_0;
    wire [1:0] stat_rx_bad_fcs_0;
    wire stat_rx_packet_bad_fcs_0;
    wire [1:0] stat_rx_stomped_fcs_0;
    wire stat_rx_packet_1549_2047_bytes_0;
    wire stat_rx_packet_2048_4095_bytes_0;
    wire stat_rx_packet_4096_8191_bytes_0;
    wire stat_rx_packet_8192_9215_bytes_0;
    wire stat_rx_unicast_0;
    wire stat_rx_multicast_0;
    wire stat_rx_broadcast_0;
    wire stat_rx_vlan_0;
    wire stat_rx_inrangeerr_0;
    wire stat_rx_bad_preamble_0;
    wire stat_rx_bad_sfd_0;
    wire stat_rx_got_signal_os_0;
    wire stat_rx_test_pattern_mismatch_0;
    wire stat_rx_truncated_0;
    wire stat_rx_local_fault_0;
    wire stat_rx_remote_fault_0;
    wire stat_rx_received_local_fault_0;
    wire  stat_rx_status_0;
    wire tx_unfout_0;
    wire stat_tx_total_packets_0;
    wire [3:0] stat_tx_total_bytes_0;
    wire stat_tx_total_good_packets_0;
    wire [13:0] stat_tx_total_good_bytes_0;
    wire stat_tx_packet_64_bytes_0;
    wire stat_tx_packet_65_127_bytes_0;
    wire stat_tx_packet_128_255_bytes_0;
    wire stat_tx_packet_256_511_bytes_0;
    wire stat_tx_packet_512_1023_bytes_0;
    wire stat_tx_packet_1024_1518_bytes_0;
    wire stat_tx_packet_1519_1522_bytes_0;
    wire stat_tx_packet_1523_1548_bytes_0;
    wire stat_tx_packet_small_0;
    wire stat_tx_packet_large_0;
    wire stat_tx_packet_1549_2047_bytes_0;
    wire stat_tx_packet_2048_4095_bytes_0;
    wire stat_tx_packet_4096_8191_bytes_0;
    wire stat_tx_packet_8192_9215_bytes_0;
    wire stat_tx_unicast_0;
    wire stat_tx_multicast_0;
    wire stat_tx_broadcast_0;
    wire stat_tx_vlan_0;
    wire stat_tx_bad_fcs_0;
    wire stat_tx_frame_error_0;
    wire stat_tx_local_fault_0;
    wire [31:0]  user_reg0_0;

    wire stat_rx_rsfec_hi_ser_0;
    wire stat_rx_rsfec_lane_alignment_status_0;
    wire stat_rx_rsfec_corrected_cw_inc_0;
    wire stat_rx_rsfec_uncorrected_cw_inc_0;
    wire [2:0] stat_rx_rsfec_err_count0_inc_0;
    wire stat_tx_rsfec_lane_alignment_status_0;
    wire gtpowergood_out_0;

    wire [1:0] gt_rxstartofseq_0; // Outputs
    wire [1:0] gt_txbufstatus_0;
    wire [16:0] gt_dmonitorout_0;
    wire gt_eyescandataerror_0;
    wire [2:0] gt_rxbufstatus_0;
    wire gt_rxprbserr_0;

    wire tx_clk_out_1;
    wire rx_core_clk_1;
    wire rx_clk_out_1;
    wire rxrecclkout_1;
    wire [55:0] rx_preambleout_1;
    wire stat_rx_block_lock_1;
    wire stat_rx_framing_err_valid_1;
    wire stat_rx_framing_err_1;
    wire stat_rx_hi_ber_1;
    wire stat_rx_valid_ctrl_code_1;
    wire stat_rx_bad_code_1;
    wire [1:0] stat_rx_total_packets_1;
    wire stat_rx_total_good_packets_1;
    wire [3:0] stat_rx_total_bytes_1;
    wire [13:0] stat_rx_total_good_bytes_1;
    wire stat_rx_packet_small_1;
    wire stat_rx_jabber_1;
    wire stat_rx_packet_large_1;
    wire stat_rx_oversize_1;
    wire stat_rx_undersize_1;
    wire stat_rx_toolong_1;
    wire stat_rx_fragment_1;
    wire stat_rx_packet_64_bytes_1;
    wire stat_rx_packet_65_127_bytes_1;
    wire stat_rx_packet_128_255_bytes_1;
    wire stat_rx_packet_256_511_bytes_1;
    wire stat_rx_packet_512_1023_bytes_1;
    wire stat_rx_packet_1024_1518_bytes_1;
    wire stat_rx_packet_1519_1522_bytes_1;
    wire stat_rx_packet_1523_1548_bytes_1;
    wire [1:0] stat_rx_bad_fcs_1;
    wire stat_rx_packet_bad_fcs_1;
    wire [1:0] stat_rx_stomped_fcs_1;
    wire stat_rx_packet_1549_2047_bytes_1;
    wire stat_rx_packet_2048_4095_bytes_1;
    wire stat_rx_packet_4096_8191_bytes_1;
    wire stat_rx_packet_8192_9215_bytes_1;
    wire stat_rx_unicast_1;
    wire stat_rx_multicast_1;
    wire stat_rx_broadcast_1;
    wire stat_rx_vlan_1;
    wire stat_rx_inrangeerr_1;
    wire stat_rx_bad_preamble_1;
    wire stat_rx_bad_sfd_1;
    wire stat_rx_got_signal_os_1;
    wire stat_rx_test_pattern_mismatch_1;
    wire stat_rx_truncated_1;
    wire stat_rx_local_fault_1;
    wire stat_rx_remote_fault_1;
    wire stat_rx_received_local_fault_1;
    wire  stat_rx_status_1;
    wire tx_unfout_1;
    wire stat_tx_total_packets_1;
    wire [3:0] stat_tx_total_bytes_1;
    wire stat_tx_total_good_packets_1;
    wire [13:0] stat_tx_total_good_bytes_1;
    wire stat_tx_packet_64_bytes_1;
    wire stat_tx_packet_65_127_bytes_1;
    wire stat_tx_packet_128_255_bytes_1;
    wire stat_tx_packet_256_511_bytes_1;
    wire stat_tx_packet_512_1023_bytes_1;
    wire stat_tx_packet_1024_1518_bytes_1;
    wire stat_tx_packet_1519_1522_bytes_1;
    wire stat_tx_packet_1523_1548_bytes_1;
    wire stat_tx_packet_small_1;
    wire stat_tx_packet_large_1;
    wire stat_tx_packet_1549_2047_bytes_1;
    wire stat_tx_packet_2048_4095_bytes_1;
    wire stat_tx_packet_4096_8191_bytes_1;
    wire stat_tx_packet_8192_9215_bytes_1;
    wire stat_tx_unicast_1;
    wire stat_tx_multicast_1;
    wire stat_tx_broadcast_1;
    wire stat_tx_vlan_1;
    wire stat_tx_bad_fcs_1;
    wire stat_tx_frame_error_1;
    wire stat_tx_local_fault_1;
    wire [31:0]  user_reg0_1;
    wire stat_rx_rsfec_hi_ser_1;
    wire stat_rx_rsfec_lane_alignment_status_1;
    wire stat_rx_rsfec_corrected_cw_inc_1;
    wire stat_rx_rsfec_uncorrected_cw_inc_1;
    wire [2:0] stat_rx_rsfec_err_count0_inc_1;
    wire stat_tx_rsfec_lane_alignment_status_1;
    wire gtpowergood_out_1;

    wire [1:0] gt_rxstartofseq_1; // Outputs
    wire [1:0] gt_txbufstatus_1;
    wire [16:0] gt_dmonitorout_1;
    wire gt_eyescandataerror_1;
    wire [2:0] gt_rxbufstatus_1;
    wire gt_rxprbserr_1;

    wire tx_clk_out_2;
    wire rx_core_clk_2;
    wire rx_clk_out_2;
    wire rxrecclkout_2;
    wire [55:0] rx_preambleout_2;
    wire stat_rx_block_lock_2;
    wire stat_rx_framing_err_valid_2;
    wire stat_rx_framing_err_2;
    wire stat_rx_hi_ber_2;
    wire stat_rx_valid_ctrl_code_2;
    wire stat_rx_bad_code_2;
    wire [1:0] stat_rx_total_packets_2;
    wire stat_rx_total_good_packets_2;
    wire [3:0] stat_rx_total_bytes_2;
    wire [13:0] stat_rx_total_good_bytes_2;
    wire stat_rx_packet_small_2;
    wire stat_rx_jabber_2;
    wire stat_rx_packet_large_2;
    wire stat_rx_oversize_2;
    wire stat_rx_undersize_2;
    wire stat_rx_toolong_2;
    wire stat_rx_fragment_2;
    wire stat_rx_packet_64_bytes_2;
    wire stat_rx_packet_65_127_bytes_2;
    wire stat_rx_packet_128_255_bytes_2;
    wire stat_rx_packet_256_511_bytes_2;
    wire stat_rx_packet_512_1023_bytes_2;
    wire stat_rx_packet_1024_1518_bytes_2;
    wire stat_rx_packet_1519_1522_bytes_2;
    wire stat_rx_packet_1523_1548_bytes_2;
    wire [1:0] stat_rx_bad_fcs_2;
    wire stat_rx_packet_bad_fcs_2;
    wire [1:0] stat_rx_stomped_fcs_2;
    wire stat_rx_packet_1549_2047_bytes_2;
    wire stat_rx_packet_2048_4095_bytes_2;
    wire stat_rx_packet_4096_8191_bytes_2;
    wire stat_rx_packet_8192_9215_bytes_2;
    wire stat_rx_unicast_2;
    wire stat_rx_multicast_2;
    wire stat_rx_broadcast_2;
    wire stat_rx_vlan_2;
    wire stat_rx_inrangeerr_2;
    wire stat_rx_bad_preamble_2;
    wire stat_rx_bad_sfd_2;
    wire stat_rx_got_signal_os_2;
    wire stat_rx_test_pattern_mismatch_2;
    wire stat_rx_truncated_2;
    wire stat_rx_local_fault_2;
    wire stat_rx_remote_fault_2;
    wire stat_rx_received_local_fault_2;
    wire  stat_rx_status_2;
    wire tx_unfout_2;
    wire stat_tx_total_packets_2;
    wire [3:0] stat_tx_total_bytes_2;
    wire stat_tx_total_good_packets_2;
    wire [13:0] stat_tx_total_good_bytes_2;
    wire stat_tx_packet_64_bytes_2;
    wire stat_tx_packet_65_127_bytes_2;
    wire stat_tx_packet_128_255_bytes_2;
    wire stat_tx_packet_256_511_bytes_2;
    wire stat_tx_packet_512_1023_bytes_2;
    wire stat_tx_packet_1024_1518_bytes_2;
    wire stat_tx_packet_1519_1522_bytes_2;
    wire stat_tx_packet_1523_1548_bytes_2;
    wire stat_tx_packet_small_2;
    wire stat_tx_packet_large_2;
    wire stat_tx_packet_1549_2047_bytes_2;
    wire stat_tx_packet_2048_4095_bytes_2;
    wire stat_tx_packet_4096_8191_bytes_2;
    wire stat_tx_packet_8192_9215_bytes_2;
    wire stat_tx_unicast_2;
    wire stat_tx_multicast_2;
    wire stat_tx_broadcast_2;
    wire stat_tx_vlan_2;
    wire stat_tx_bad_fcs_2;
    wire stat_tx_frame_error_2;
    wire stat_tx_local_fault_2;
    wire [31:0]  user_reg0_2;
    wire stat_rx_rsfec_hi_ser_2;
    wire stat_rx_rsfec_lane_alignment_status_2;
    wire stat_rx_rsfec_corrected_cw_inc_2;
    wire stat_rx_rsfec_uncorrected_cw_inc_2;
    wire [2:0] stat_rx_rsfec_err_count0_inc_2;
    wire stat_tx_rsfec_lane_alignment_status_2;
    wire gtpowergood_out_2;

    wire [1:0] gt_rxstartofseq_2; // Outputs
    wire [1:0] gt_txbufstatus_2;
    wire [16:0] gt_dmonitorout_2;
    wire gt_eyescandataerror_2;
    wire [2:0] gt_rxbufstatus_2;
    wire gt_rxprbserr_2;

    wire tx_clk_out_3;
    wire rx_core_clk_3;
    wire rx_clk_out_3;
    wire rxrecclkout_3;
    wire [55:0] rx_preambleout_3;
    wire stat_rx_block_lock_3;
    wire stat_rx_framing_err_valid_3;
    wire stat_rx_framing_err_3;
    wire stat_rx_hi_ber_3;
    wire stat_rx_valid_ctrl_code_3;
    wire stat_rx_bad_code_3;
    wire [1:0] stat_rx_total_packets_3;
    wire stat_rx_total_good_packets_3;
    wire [3:0] stat_rx_total_bytes_3;
    wire [13:0] stat_rx_total_good_bytes_3;
    wire stat_rx_packet_small_3;
    wire stat_rx_jabber_3;
    wire stat_rx_packet_large_3;
    wire stat_rx_oversize_3;
    wire stat_rx_undersize_3;
    wire stat_rx_toolong_3;
    wire stat_rx_fragment_3;
    wire stat_rx_packet_64_bytes_3;
    wire stat_rx_packet_65_127_bytes_3;
    wire stat_rx_packet_128_255_bytes_3;
    wire stat_rx_packet_256_511_bytes_3;
    wire stat_rx_packet_512_1023_bytes_3;
    wire stat_rx_packet_1024_1518_bytes_3;
    wire stat_rx_packet_1519_1522_bytes_3;
    wire stat_rx_packet_1523_1548_bytes_3;
    wire [1:0] stat_rx_bad_fcs_3;
    wire stat_rx_packet_bad_fcs_3;
    wire [1:0] stat_rx_stomped_fcs_3;
    wire stat_rx_packet_1549_2047_bytes_3;
    wire stat_rx_packet_2048_4095_bytes_3;
    wire stat_rx_packet_4096_8191_bytes_3;
    wire stat_rx_packet_8192_9215_bytes_3;
    wire stat_rx_unicast_3;
    wire stat_rx_multicast_3;
    wire stat_rx_broadcast_3;
    wire stat_rx_vlan_3;
    wire stat_rx_inrangeerr_3;
    wire stat_rx_bad_preamble_3;
    wire stat_rx_bad_sfd_3;
    wire stat_rx_got_signal_os_3;
    wire stat_rx_test_pattern_mismatch_3;
    wire stat_rx_truncated_3;
    wire stat_rx_local_fault_3;
    wire stat_rx_remote_fault_3;
    wire stat_rx_received_local_fault_3;
    wire  stat_rx_status_3;
    wire tx_unfout_3;
    wire stat_tx_total_packets_3;
    wire [3:0] stat_tx_total_bytes_3;
    wire stat_tx_total_good_packets_3;
    wire [13:0] stat_tx_total_good_bytes_3;
    wire stat_tx_packet_64_bytes_3;
    wire stat_tx_packet_65_127_bytes_3;
    wire stat_tx_packet_128_255_bytes_3;
    wire stat_tx_packet_256_511_bytes_3;
    wire stat_tx_packet_512_1023_bytes_3;
    wire stat_tx_packet_1024_1518_bytes_3;
    wire stat_tx_packet_1519_1522_bytes_3;
    wire stat_tx_packet_1523_1548_bytes_3;
    wire stat_tx_packet_small_3;
    wire stat_tx_packet_large_3;
    wire stat_tx_packet_1549_2047_bytes_3;
    wire stat_tx_packet_2048_4095_bytes_3;
    wire stat_tx_packet_4096_8191_bytes_3;
    wire stat_tx_packet_8192_9215_bytes_3;
    wire stat_tx_unicast_3;
    wire stat_tx_multicast_3;
    wire stat_tx_broadcast_3;
    wire stat_tx_vlan_3;
    wire stat_tx_bad_fcs_3;
    wire stat_tx_frame_error_3;
    wire stat_tx_local_fault_3;
    wire [31:0]  user_reg0_3;
    wire stat_rx_rsfec_hi_ser_3;
    wire stat_rx_rsfec_lane_alignment_status_3;
    wire stat_rx_rsfec_corrected_cw_inc_3;
    wire stat_rx_rsfec_uncorrected_cw_inc_3;
    wire [2:0] stat_rx_rsfec_err_count0_inc_3;
    wire stat_tx_rsfec_lane_alignment_status_3;
    wire gtpowergood_out_3;

    wire [1:0] gt_rxstartofseq_3; // Outputs
    wire [1:0] gt_txbufstatus_3;
    wire [16:0] gt_dmonitorout_3;
    wire gt_eyescandataerror_3;
    wire [2:0] gt_rxbufstatus_3;
    wire gt_rxprbserr_3;

    wire sys_reset;
    wire dclk;
    wire gt_refclk_out;
    wire qpllreset_in_0;

    wire toggle_1_sec_cont;

///////////////////////////////////////////////////////////////////////////////
//
//      Reset Gating
//
///////////////////////////////////////////////////////////////////////////////

always_ff @(posedge ap_clk) begin

    ap_rst_gate <= ap_rst & ap_rst_allow;

end

///////////////////////////////////////////////////////////////////////////////
//
//      Static Wiring
//
///////////////////////////////////////////////////////////////////////////////

    assign  s_axi_aclk      = ap_clk;
    assign  s_axi_aresetn   = ~ap_rst_gate;
    assign  sys_reset       = ap_rst_gate;

    //pg210-25g-ethernet: When FIFO is included, rx_core_clk can be driven by tx_clk_out, rx_clk_out, or another asynchronous clock at the same frequency
    assign  rx_core_clk     = tx_clk_out;

///////////////////////////////////////////////////////////////////////////////
//
//      Generate a 75 MHz DRP Clock to meet < 250 MHz requirement
//
///////////////////////////////////////////////////////////////////////////////

BUFGCE_DIV #(
        .BUFGCE_DIVIDE(4)
    ) DRP_CK (
        .I      (ap_clk     ),
        .CE     (1'b1       ),
        .CLR    (1'b0       ),
        .O      (gt_drpclk  )
    );

    assign  dclk    = gt_drpclk;

    assign  axi_aclk = gt_drpclk;
    wire    axi_aresetn;

xpm_cdc_async_rst #(
    .DEST_SYNC_FF       (DEST_SYNC_FF),
    .RST_ACTIVE_HIGH    (1)
) xpm_cdc_async_rst_inst (
    .src_arst   (ap_rst_gate),
    .dest_clk   (gt_drpclk  ),
    .dest_arst  (gt_drprst  )
);

xpm_cdc_async_rst #(
    .DEST_SYNC_FF       (DEST_SYNC_FF),
    .RST_ACTIVE_HIGH    (0)
) xpm_cdc_async_rstn_inst (
    .src_arst   (s_axi_aresetn),
    .dest_clk   (axi_aclk    ),
    .dest_arst  (axi_aresetn )
);


///////////////////////////////////////////////////////////////////////////////
//
//      Instantiate the XXV MAC Core
//
///////////////////////////////////////////////////////////////////////////////
assign mac_rst_status = {~mac_stat_rx_status, mac_user_rx_reset, mac_user_tx_reset};

for(genvar ii = 0; ii < 4; ii++) begin : cdc_resync

    xpm_cdc_async_rst #(
        .DEST_SYNC_FF       (DEST_SYNC_FF),
        .RST_ACTIVE_HIGH    (1)
    ) xpm_cdc_async_user_tx_rst (
        .src_arst   (user_tx_reset[ii]),
        .dest_clk   (ap_clk),
        .dest_arst  (mac_user_tx_reset[ii]  )
    );

    xpm_cdc_async_rst #(
        .DEST_SYNC_FF       (DEST_SYNC_FF),
        .RST_ACTIVE_HIGH    (1)
    ) xpm_cdc_async_user_rx_rst (
        .src_arst   (user_rx_reset[ii]),
        .dest_clk   (ap_clk),
        .dest_arst  (mac_user_rx_reset[ii]  )
    );

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF      ),
        .INIT_SYNC_FF   ( 0                 ),
        .SRC_INPUT_REG  ( 1                 ),
        .SIM_ASSERT_CHK ( 0                 )
    )
    xpm_cdc_stat_rx_stat (
        .src_clk  ( rx_clk_out[ii]          ),
        .src_in   ( stat_rx_status[ii]      ),
        .dest_out ( mac_stat_rx_status[ii]  ),
        .dest_clk ( ap_clk                  )
    );

end

mac_wrapper #(
    .C_GT_NUM_GT            (C_GT_NUM_GT        ),
    .C_GT_NUM_LANE          (C_GT_NUM_LANE      ),
    .C_GT_RATE              (C_GT_RATE          ),
    .C_GT_TYPE              (C_GT_TYPE          ),
    .C_GT_MAC_IP_SEL        (C_GT_MAC_IP_SEL    ),
    .C_GT_MAC_ENABLE_RSFEC  (C_GT_MAC_ENABLE_RSFEC)
    ) mac_wrapper (

    .qpllreset_in_0                 (ap_rst),
    // Transceiver Interface

    .gt_txp_out                     (QSFP_TX_P),
    .gt_txn_out                     (QSFP_TX_N),
    .gt_rxp_in                      (QSFP_RX_P),
    .gt_rxn_in                      (QSFP_RX_N),

    .gt_refclk_p                    (QSFP_CK_P),
    .gt_refclk_n                    (QSFP_CK_N),

    // Control Interfaces

    .tx_clk_out_0                   (tx_clk_out[0]      ),
    .rx_core_clk_0                  (rx_core_clk[0]     ),
    .rx_clk_out_0                   (rx_clk_out[0]      ),

    .rx_reset_0                     (rx_reset[0]        ),
    .user_rx_reset_0                (user_rx_reset[0]   ),
    .stat_rx_status_0               (stat_rx_status[0]  ),
    .gtwiz_reset_tx_datapath_0      (tx_reset[0]        ),
    .gtwiz_reset_rx_datapath_0      (rx_reset[0]        ),

    .tx_reset_0                     (tx_reset[0]        ),
    .user_tx_reset_0                (user_tx_reset[0]   ),

    .tx_clk_out_1                   (tx_clk_out[1]      ),
    .rx_core_clk_1                  (rx_core_clk[1]     ),
    .rx_clk_out_1                   (rx_clk_out[1]      ),

    .rx_reset_1                     (rx_reset[1]        ),
    .user_rx_reset_1                (user_rx_reset[1]   ),
    .stat_rx_status_1               (stat_rx_status[1]  ),
    .tx_reset_1                     (tx_reset[1]        ),
    .user_tx_reset_1                (user_tx_reset[1]   ),
    .gtwiz_reset_tx_datapath_1      (tx_reset[1]        ),
    .gtwiz_reset_rx_datapath_1      (rx_reset[1]        ),

    .tx_clk_out_2                   (tx_clk_out[2]      ),
    .rx_core_clk_2                  (rx_core_clk[2]     ),
    .rx_clk_out_2                   (rx_clk_out[2]      ),

    .rx_reset_2                     (rx_reset[2]        ),
    .user_rx_reset_2                (user_rx_reset[2]   ),
    .stat_rx_status_2               (stat_rx_status[2]  ),
    .tx_reset_2                     (tx_reset[2]        ),
    .user_tx_reset_2                (user_tx_reset[2]   ),
    .gtwiz_reset_tx_datapath_2      (tx_reset[2]        ),
    .gtwiz_reset_rx_datapath_2      (rx_reset[2]        ),

    .tx_clk_out_3                   (tx_clk_out[3]      ),
    .rx_core_clk_3                  (rx_core_clk[3]     ),
    .rx_clk_out_3                   (rx_clk_out[3]      ),

    .rx_reset_3                     (rx_reset[3]        ),
    .user_rx_reset_3                (user_rx_reset[3]   ),
    .stat_rx_status_3               (stat_rx_status[3]  ),
    .tx_reset_3                     (tx_reset[3]        ),
    .user_tx_reset_3                (user_tx_reset[3]   ),
    .gtwiz_reset_tx_datapath_3      (tx_reset[3]        ),
    .gtwiz_reset_rx_datapath_3      (rx_reset[3]        ),

    .ctl_rx_rate_10g_25gn_0         (ctl_rx_rate[0]     ),
    .ctl_rx_rate_10g_25gn_1         (ctl_rx_rate[1]     ),
    .ctl_rx_rate_10g_25gn_2         (ctl_rx_rate[2]     ),
    .ctl_rx_rate_10g_25gn_3         (ctl_rx_rate[3]     ),

    .rxdfecfokfcnum_in_0            (rxdfecfokfcnum[0]  ),
    .rxdfecfokfcnum_in_1            (rxdfecfokfcnum[1]  ),
    .rxdfecfokfcnum_in_2            (rxdfecfokfcnum[2]  ),
    .rxdfecfokfcnum_in_3            (rxdfecfokfcnum[3]  ),

    .tx_preamblein_0                (tx_preamblein[0]   ),
    .ctl_tx_send_lfi_0              (ctl_tx_send_lfi[0] ),
    .ctl_tx_send_rfi_0              (ctl_tx_send_rfi[0] ),
    .ctl_tx_send_idle_0             (ctl_tx_send_idle[0]),

    .tx_preamblein_1                (tx_preamblein[1]   ),
    .ctl_tx_send_lfi_1              (ctl_tx_send_lfi[1] ),
    .ctl_tx_send_rfi_1              (ctl_tx_send_rfi[1] ),
    .ctl_tx_send_idle_1             (ctl_tx_send_idle[1]),

    .tx_preamblein_2                (tx_preamblein[2]   ),
    .ctl_tx_send_lfi_2              (ctl_tx_send_lfi[2] ),
    .ctl_tx_send_rfi_2              (ctl_tx_send_rfi[2] ),
    .ctl_tx_send_idle_2             (ctl_tx_send_idle[2]),

    .tx_preamblein_3                (tx_preamblein[3]   ),
    .ctl_tx_send_lfi_3              (ctl_tx_send_lfi[3] ),
    .ctl_tx_send_rfi_3              (ctl_tx_send_rfi[3] ),
    .ctl_tx_send_idle_3             (ctl_tx_send_idle[3]),

    // Clock Selection

    .txpllclksel_in_0               (txpllclksel_in[0]  ),
    .rxpllclksel_in_0               (rxpllclksel_in[0]  ),
    .txsysclksel_in_0               (txsysclksel_in[0]  ),
    .rxsysclksel_in_0               (rxsysclksel_in[0]  ),
    .txoutclksel_in_0               (txoutclksel_in[0]  ),
    .rxoutclksel_in_0               (rxoutclksel_in[0]  ),

    .txpllclksel_in_1               (txpllclksel_in[1]  ),
    .rxpllclksel_in_1               (rxpllclksel_in[1]  ),
    .txsysclksel_in_1               (txsysclksel_in[1]  ),
    .rxsysclksel_in_1               (rxsysclksel_in[1]  ),
    .txoutclksel_in_1               (txoutclksel_in[1]  ),
    .rxoutclksel_in_1               (rxoutclksel_in[1]  ),

    .txpllclksel_in_2               (txpllclksel_in[2]  ),
    .rxpllclksel_in_2               (rxpllclksel_in[2]  ),
    .txsysclksel_in_2               (txsysclksel_in[2]  ),
    .rxsysclksel_in_2               (rxsysclksel_in[2]  ),
    .txoutclksel_in_2               (txoutclksel_in[2]  ),
    .rxoutclksel_in_2               (rxoutclksel_in[2]  ),

    .txpllclksel_in_3               (txpllclksel_in[3]  ),
    .rxpllclksel_in_3               (rxpllclksel_in[3]  ),
    .txsysclksel_in_3               (txsysclksel_in[3]  ),
    .rxsysclksel_in_3               (rxsysclksel_in[3]  ),
    .txoutclksel_in_3               (txoutclksel_in[3]  ),
    .rxoutclksel_in_3               (rxoutclksel_in[3]  ),

    // GT Configuration

    .gt_eyescanreset_0              (gt_eyescanreset[0]     ),
    .gt_eyescantrigger_0            (gt_eyescantrigger[0]   ),
    .gt_pcsrsvdin_0                 (gt_pcsrsvdin[0]        ),
    .gt_rxbufreset_0                (gt_rxbufreset[0]       ),
    .gt_rxcdrhold_0                 (gt_rxcdrhold[0]        ),
    .gt_rxcommadeten_0              (gt_rxcommadeten[0]     ),
    .gt_rxdfeagchold_0              (gt_rxdfeagchold[0]     ),
    .gt_rxdfelpmreset_0             (gt_rxdfelpmreset[0]    ),
    .gt_rxlatclk_0                  (gt_rxlatclk[0]         ),
    .gt_rxlpmen_0                   (gt_rxlpmen[0]          ),
    .gt_rxpcsreset_0                (gt_rxpcsreset[0]       ),
    .gt_rxpmareset_0                (gt_rxpmareset[0]       ),
    .gt_rxpolarity_0                (gt_rxpolarity[0]       ),
    .gt_rxprbscntreset_0            (gt_rxprbscntreset[0]   ),
    .gt_rxprbssel_0                 (gt_rxprbssel[0]        ),
    .gt_rxrate_0                    (gt_rxrate[0]           ),
    .gt_rxslide_in_0                (gt_rxslide_in[0]       ),
    .gt_txinhibit_0                 (gt_txinhibit[0]        ),
    .gt_txlatclk_0                  (gt_txlatclk[0]         ),
    .gt_txmaincursor_0              (gt_txmaincursor[0]     ),
    .gt_txpcsreset_0                (gt_txpcsreset[0]       ),
    .gt_txpmareset_0                (gt_txpmareset[0]       ),
    .gt_txpolarity_0                (gt_txpolarity[0]       ),
    .gt_txpostcursor_0              (gt_txpostcursor[0]     ),
    .gt_txprbsforceerr_0            (gt_txprbsforceerr[0]   ),
    .gt_txelecidle_0                (gt_txelecidle[0]       ),
    .gt_txprbssel_0                 (gt_txprbssel[0]        ),
    .gt_txprecursor_0               (gt_txprecursor[0]      ),
    .gt_txdiffctrl_0                (gt_txdiffctrl[0]       ),

    .gt_eyescanreset_1              (gt_eyescanreset[1]     ),
    .gt_eyescantrigger_1            (gt_eyescantrigger[1]   ),
    .gt_pcsrsvdin_1                 (gt_pcsrsvdin[1]        ),
    .gt_rxbufreset_1                (gt_rxbufreset[1]       ),
    .gt_rxcdrhold_1                 (gt_rxcdrhold[1]        ),
    .gt_rxcommadeten_1              (gt_rxcommadeten[1]     ),
    .gt_rxdfeagchold_1              (gt_rxdfeagchold[1]     ),
    .gt_rxdfelpmreset_1             (gt_rxdfelpmreset[1]    ),
    .gt_rxlatclk_1                  (gt_rxlatclk[1]         ),
    .gt_rxlpmen_1                   (gt_rxlpmen[1]          ),
    .gt_rxpcsreset_1                (gt_rxpcsreset[1]       ),
    .gt_rxpmareset_1                (gt_rxpmareset[1]       ),
    .gt_rxpolarity_1                (gt_rxpolarity[1]       ),
    .gt_rxprbscntreset_1            (gt_rxprbscntreset[1]   ),
    .gt_rxprbssel_1                 (gt_rxprbssel[1]        ),
    .gt_rxrate_1                    (gt_rxrate[1]           ),
    .gt_rxslide_in_1                (gt_rxslide_in[1]       ),
    .gt_txinhibit_1                 (gt_txinhibit[1]        ),
    .gt_txlatclk_1                  (gt_txlatclk[1]         ),
    .gt_txmaincursor_1              (gt_txmaincursor[1]     ),
    .gt_txpcsreset_1                (gt_txpcsreset[1]       ),
    .gt_txpmareset_1                (gt_txpmareset[1]       ),
    .gt_txpolarity_1                (gt_txpolarity[1]       ),
    .gt_txpostcursor_1              (gt_txpostcursor[1]     ),
    .gt_txprbsforceerr_1            (gt_txprbsforceerr[1]   ),
    .gt_txelecidle_1                (gt_txelecidle[1]       ),
    .gt_txprbssel_1                 (gt_txprbssel[1]        ),
    .gt_txprecursor_1               (gt_txprecursor[1]      ),
    .gt_txdiffctrl_1                (gt_txdiffctrl[1]       ),

    .gt_eyescanreset_2              (gt_eyescanreset[2]     ),
    .gt_eyescantrigger_2            (gt_eyescantrigger[2]   ),
    .gt_pcsrsvdin_2                 (gt_pcsrsvdin[2]        ),
    .gt_rxbufreset_2                (gt_rxbufreset[2]       ),
    .gt_rxcdrhold_2                 (gt_rxcdrhold[2]        ),
    .gt_rxcommadeten_2              (gt_rxcommadeten[2]     ),
    .gt_rxdfeagchold_2              (gt_rxdfeagchold[2]     ),
    .gt_rxdfelpmreset_2             (gt_rxdfelpmreset[2]    ),
    .gt_rxlatclk_2                  (gt_rxlatclk[2]         ),
    .gt_rxlpmen_2                   (gt_rxlpmen[2]          ),
    .gt_rxpcsreset_2                (gt_rxpcsreset[2]       ),
    .gt_rxpmareset_2                (gt_rxpmareset[2]       ),
    .gt_rxpolarity_2                (gt_rxpolarity[2]       ),
    .gt_rxprbscntreset_2            (gt_rxprbscntreset[2]   ),
    .gt_rxprbssel_2                 (gt_rxprbssel[2]        ),
    .gt_rxrate_2                    (gt_rxrate[2]           ),
    .gt_rxslide_in_2                (gt_rxslide_in[2]       ),
    .gt_txinhibit_2                 (gt_txinhibit[2]        ),
    .gt_txlatclk_2                  (gt_txlatclk[2]         ),
    .gt_txmaincursor_2              (gt_txmaincursor[2]     ),
    .gt_txpcsreset_2                (gt_txpcsreset[2]       ),
    .gt_txpmareset_2                (gt_txpmareset[2]       ),
    .gt_txpolarity_2                (gt_txpolarity[2]       ),
    .gt_txpostcursor_2              (gt_txpostcursor[2]     ),
    .gt_txprbsforceerr_2            (gt_txprbsforceerr[2]   ),
    .gt_txelecidle_2                (gt_txelecidle[2]       ),
    .gt_txprbssel_2                 (gt_txprbssel[2]        ),
    .gt_txprecursor_2               (gt_txprecursor[2]      ),
    .gt_txdiffctrl_2                (gt_txdiffctrl[2]       ),

    .gt_eyescanreset_3              (gt_eyescanreset[3]     ),
    .gt_eyescantrigger_3            (gt_eyescantrigger[3]   ),
    .gt_pcsrsvdin_3                 (gt_pcsrsvdin[3]        ),
    .gt_rxbufreset_3                (gt_rxbufreset[3]       ),
    .gt_rxcdrhold_3                 (gt_rxcdrhold[3]        ),
    .gt_rxcommadeten_3              (gt_rxcommadeten[3]     ),
    .gt_rxdfeagchold_3              (gt_rxdfeagchold[3]     ),
    .gt_rxdfelpmreset_3             (gt_rxdfelpmreset[3]    ),
    .gt_rxlatclk_3                  (gt_rxlatclk[3]         ),
    .gt_rxlpmen_3                   (gt_rxlpmen[3]          ),
    .gt_rxpcsreset_3                (gt_rxpcsreset[3]       ),
    .gt_rxpmareset_3                (gt_rxpmareset[3]       ),
    .gt_rxpolarity_3                (gt_rxpolarity[3]       ),
    .gt_rxprbscntreset_3            (gt_rxprbscntreset[3]   ),
    .gt_rxprbssel_3                 (gt_rxprbssel[3]        ),
    .gt_rxrate_3                    (gt_rxrate[3]           ),
    .gt_rxslide_in_3                (gt_rxslide_in[3]       ),
    .gt_txinhibit_3                 (gt_txinhibit[3]        ),
    .gt_txlatclk_3                  (gt_txlatclk[3]         ),
    .gt_txmaincursor_3              (gt_txmaincursor[3]     ),
    .gt_txpcsreset_3                (gt_txpcsreset[3]       ),
    .gt_txpmareset_3                (gt_txpmareset[3]       ),
    .gt_txpolarity_3                (gt_txpolarity[3]       ),
    .gt_txpostcursor_3              (gt_txpostcursor[3]     ),
    .gt_txprbsforceerr_3            (gt_txprbsforceerr[3]   ),
    .gt_txelecidle_3                (gt_txelecidle[3]       ),
    .gt_txprbssel_3                 (gt_txprbssel[3]        ),
    .gt_txprecursor_3               (gt_txprecursor[3]      ),
    .gt_txdiffctrl_3                (gt_txdiffctrl[3]       ),

    // DRP Interface

    .gt_drpclk_0                    (gt_drpclk          ),
    .gt_drprst_0                    (gt_drprst          ),
    .gt_drpdo_0                     (gt_drpdo[0]        ),
    .gt_drprdy_0                    (gt_drprdy[0]       ),
    .gt_drpen_0                     (gt_drpen[0]        ),
    .gt_drpwe_0                     (gt_drpwe[0]        ),
    .gt_drpaddr_0                   (gt_drpaddr[0]      ),
    .gt_drpdi_0                     (gt_drpdi[0]        ),
    .gt_drp_done_0                  (gt_drp_done[0]     ),

    .gt_drpclk_1                    (gt_drpclk          ),
    .gt_drprst_1                    (gt_drprst          ),
    .gt_drpdo_1                     (gt_drpdo[1]        ),
    .gt_drprdy_1                    (gt_drprdy[1]       ),
    .gt_drpen_1                     (gt_drpen[1]        ),
    .gt_drpwe_1                     (gt_drpwe[1]        ),
    .gt_drpaddr_1                   (gt_drpaddr[1]      ),
    .gt_drpdi_1                     (gt_drpdi[1]        ),
    .gt_drp_done_1                  (gt_drp_done[1]     ),

    .gt_drpclk_2                    (gt_drpclk          ),
    .gt_drprst_2                    (gt_drprst          ),
    .gt_drpdo_2                     (gt_drpdo[2]        ),
    .gt_drprdy_2                    (gt_drprdy[2]       ),
    .gt_drpen_2                     (gt_drpen[2]        ),
    .gt_drpwe_2                     (gt_drpwe[2]        ),
    .gt_drpaddr_2                   (gt_drpaddr[2]      ),
    .gt_drpdi_2                     (gt_drpdi[2]        ),
    .gt_drp_done_2                  (gt_drp_done[2]     ),

    .gt_drpclk_3                    (gt_drpclk          ),
    .gt_drprst_3                    (gt_drprst          ),
    .gt_drpdo_3                     (gt_drpdo[3]        ),
    .gt_drprdy_3                    (gt_drprdy[3]       ),
    .gt_drpen_3                     (gt_drpen[3]        ),
    .gt_drpwe_3                     (gt_drpwe[3]        ),
    .gt_drpaddr_3                   (gt_drpaddr[3]      ),
    .gt_drpdi_3                     (gt_drpdi[3]        ),
    .gt_drp_done_3                  (gt_drp_done[3]     ),

    .axi_ctl_core_mode_switch_0     (axi_ctl_md_switch[0]),
    .axi_ctl_core_mode_switch_1     (axi_ctl_md_switch[1]),
    .axi_ctl_core_mode_switch_2     (axi_ctl_md_switch[2]),
    .axi_ctl_core_mode_switch_3     (axi_ctl_md_switch[3]),

    .rxafecfoken_in_0               (rxafecfoken[0]     ),
    .rxafecfoken_in_1               (rxafecfoken[1]     ),
    .rxafecfoken_in_2               (rxafecfoken[2]     ),
    .rxafecfoken_in_3               (rxafecfoken[3]     ),

    // AXIS Streaming Interface

    .rx_axis_tvalid_0               (rx_axis_tvalid[0]  ),
    .rx_axis_tdata_0                (rx_axis_tdata[0]   ),
    .rx_axis_tlast_0                (rx_axis_tlast[0]   ),
    .rx_axis_tkeep_0                (rx_axis_tkeep[0]   ),
    .rx_axis_tuser_0                (rx_axis_tuser[0]   ),

    .tx_axis_tready_0               (tx_axis_tready[0]  ),
    .tx_axis_tvalid_0               (tx_axis_tvalid[0]  ),
    .tx_axis_tdata_0                (tx_axis_tdata[0]   ),
    .tx_axis_tlast_0                (tx_axis_tlast[0]   ),
    .tx_axis_tkeep_0                (tx_axis_tkeep[0]   ),
    .tx_axis_tuser_0                (tx_axis_tuser[0]   ),

    .rx_axis_tvalid_1               (rx_axis_tvalid[1]  ),
    .rx_axis_tdata_1                (rx_axis_tdata[1]   ),
    .rx_axis_tlast_1                (rx_axis_tlast[1]   ),
    .rx_axis_tkeep_1                (rx_axis_tkeep[1]   ),
    .rx_axis_tuser_1                (rx_axis_tuser[1]   ),

    .tx_axis_tready_1               (tx_axis_tready[1]  ),
    .tx_axis_tvalid_1               (tx_axis_tvalid[1]  ),
    .tx_axis_tdata_1                (tx_axis_tdata[1]   ),
    .tx_axis_tlast_1                (tx_axis_tlast[1]   ),
    .tx_axis_tkeep_1                (tx_axis_tkeep[1]   ),
    .tx_axis_tuser_1                (tx_axis_tuser[1]   ),

    .rx_axis_tvalid_2               (rx_axis_tvalid[2]  ),
    .rx_axis_tdata_2                (rx_axis_tdata[2]   ),
    .rx_axis_tlast_2                (rx_axis_tlast[2]   ),
    .rx_axis_tkeep_2                (rx_axis_tkeep[2]   ),
    .rx_axis_tuser_2                (rx_axis_tuser[2]   ),

    .tx_axis_tready_2               (tx_axis_tready[2]  ),
    .tx_axis_tvalid_2               (tx_axis_tvalid[2]  ),
    .tx_axis_tdata_2                (tx_axis_tdata[2]   ),
    .tx_axis_tlast_2                (tx_axis_tlast[2]   ),
    .tx_axis_tkeep_2                (tx_axis_tkeep[2]   ),
    .tx_axis_tuser_2                (tx_axis_tuser[2]   ),

    .rx_axis_tvalid_3               (rx_axis_tvalid[3]  ),
    .rx_axis_tdata_3                (rx_axis_tdata[3]   ),
    .rx_axis_tlast_3                (rx_axis_tlast[3]   ),
    .rx_axis_tkeep_3                (rx_axis_tkeep[3]   ),
    .rx_axis_tuser_3                (rx_axis_tuser[3]   ),

    .tx_axis_tready_3               (tx_axis_tready[3]  ),
    .tx_axis_tvalid_3               (tx_axis_tvalid[3]  ),
    .tx_axis_tdata_3                (tx_axis_tdata[3]   ),
    .tx_axis_tlast_3                (tx_axis_tlast[3]   ),
    .tx_axis_tkeep_3                (tx_axis_tkeep[3]   ),
    .tx_axis_tuser_3                (tx_axis_tuser[3]   ),

    // AXI Memory Mapped Configuration / Status Interface

    .s_axi_aclk_0                   (axi_aclk           ),
    .s_axi_aresetn_0                (axi_aresetn        ),
    .stat_rx_internal_local_fault_0 (MAC_local_fault[0] ),
    .pm_tick_0                      (pm_tick[0]         ),

    .s_axi_awaddr_0                 (s_axi_awaddr[0]    ),
    .s_axi_awvalid_0                (s_axi_awvalid[0]   ),
    .s_axi_awready_0                (s_axi_awready[0]   ),
    .s_axi_wdata_0                  (s_axi_wdata[0]     ),
    .s_axi_wstrb_0                  (s_axi_wstrb[0]     ),
    .s_axi_wvalid_0                 (s_axi_wvalid[0]    ),
    .s_axi_wready_0                 (s_axi_wready[0]    ),
    .s_axi_bresp_0                  (s_axi_bresp[0]     ),
    .s_axi_bvalid_0                 (s_axi_bvalid[0]    ),
    .s_axi_bready_0                 (s_axi_bready[0]    ),
    .s_axi_araddr_0                 (s_axi_araddr[0]    ),
    .s_axi_arvalid_0                (s_axi_arvalid[0]   ),
    .s_axi_arready_0                (s_axi_arready[0]   ),
    .s_axi_rdata_0                  (s_axi_rdata[0]     ),
    .s_axi_rresp_0                  (s_axi_rresp[0]     ),
    .s_axi_rvalid_0                 (s_axi_rvalid[0]    ),
    .s_axi_rready_0                 (s_axi_rready[0]    ),

    .s_axi_aclk_1                   (axi_aclk           ),
    .s_axi_aresetn_1                (axi_aresetn        ),
    .stat_rx_internal_local_fault_1 (MAC_local_fault[1] ),
    .pm_tick_1                      (pm_tick[1]         ),

    .s_axi_awaddr_1                 (s_axi_awaddr[1]    ),
    .s_axi_awvalid_1                (s_axi_awvalid[1]   ),
    .s_axi_awready_1                (s_axi_awready[1]   ),
    .s_axi_wdata_1                  (s_axi_wdata[1]     ),
    .s_axi_wstrb_1                  (s_axi_wstrb[1]     ),
    .s_axi_wvalid_1                 (s_axi_wvalid[1]    ),
    .s_axi_wready_1                 (s_axi_wready[1]    ),
    .s_axi_bresp_1                  (s_axi_bresp[1]     ),
    .s_axi_bvalid_1                 (s_axi_bvalid[1]    ),
    .s_axi_bready_1                 (s_axi_bready[1]    ),
    .s_axi_araddr_1                 (s_axi_araddr[1]    ),
    .s_axi_arvalid_1                (s_axi_arvalid[1]   ),
    .s_axi_arready_1                (s_axi_arready[1]   ),
    .s_axi_rdata_1                  (s_axi_rdata[1]     ),
    .s_axi_rresp_1                  (s_axi_rresp[1]     ),
    .s_axi_rvalid_1                 (s_axi_rvalid[1]    ),
    .s_axi_rready_1                 (s_axi_rready[1]    ),

    .s_axi_aclk_2                   (axi_aclk           ),
    .s_axi_aresetn_2                (axi_aresetn        ),
    .stat_rx_internal_local_fault_2 (MAC_local_fault[2] ),
    .pm_tick_2                      (pm_tick[2]         ),

    .s_axi_awaddr_2                 (s_axi_awaddr[2]    ),
    .s_axi_awvalid_2                (s_axi_awvalid[2]   ),
    .s_axi_awready_2                (s_axi_awready[2]   ),
    .s_axi_wdata_2                  (s_axi_wdata[2]     ),
    .s_axi_wstrb_2                  (s_axi_wstrb[2]     ),
    .s_axi_wvalid_2                 (s_axi_wvalid[2]    ),
    .s_axi_wready_2                 (s_axi_wready[2]    ),
    .s_axi_bresp_2                  (s_axi_bresp[2]     ),
    .s_axi_bvalid_2                 (s_axi_bvalid[2]    ),
    .s_axi_bready_2                 (s_axi_bready[2]    ),
    .s_axi_araddr_2                 (s_axi_araddr[2]    ),
    .s_axi_arvalid_2                (s_axi_arvalid[2]   ),
    .s_axi_arready_2                (s_axi_arready[2]   ),
    .s_axi_rdata_2                  (s_axi_rdata[2]     ),
    .s_axi_rresp_2                  (s_axi_rresp[2]     ),
    .s_axi_rvalid_2                 (s_axi_rvalid[2]    ),
    .s_axi_rready_2                 (s_axi_rready[2]    ),

    .s_axi_aclk_3                   (axi_aclk           ),
    .s_axi_aresetn_3                (axi_aresetn        ),
    .stat_rx_internal_local_fault_3 (MAC_local_fault[3] ),
    .pm_tick_3                      (pm_tick[3]         ),

    .s_axi_awaddr_3                 (s_axi_awaddr[3]    ),
    .s_axi_awvalid_3                (s_axi_awvalid[3]   ),
    .s_axi_awready_3                (s_axi_awready[3]   ),
    .s_axi_wdata_3                  (s_axi_wdata[3]     ),
    .s_axi_wstrb_3                  (s_axi_wstrb[3]     ),
    .s_axi_wvalid_3                 (s_axi_wvalid[3]    ),
    .s_axi_wready_3                 (s_axi_wready[3]    ),
    .s_axi_bresp_3                  (s_axi_bresp[3]     ),
    .s_axi_bvalid_3                 (s_axi_bvalid[3]    ),
    .s_axi_bready_3                 (s_axi_bready[3]    ),
    .s_axi_araddr_3                 (s_axi_araddr[3]    ),
    .s_axi_arvalid_3                (s_axi_arvalid[3]   ),
    .s_axi_arready_3                (s_axi_arready[3]   ),
    .s_axi_rdata_3                  (s_axi_rdata[3]     ),
    .s_axi_rresp_3                  (s_axi_rresp[3]     ),
    .s_axi_rvalid_3                 (s_axi_rvalid[3]    ),
    .s_axi_rready_3                 (s_axi_rready[3]    ),

    // Default map everything else

    .*
  );

///////////////////////////////////////////////////////////////////////////////
//
//      Instantiate the Streaming Traffic Generators
//
///////////////////////////////////////////////////////////////////////////////

timer #(
    .C_CLOCK_FREQ           ( C_CLOCK0_FREQ     ),
    .C_1_SEC_TIMER_EN       ( 1                 ),
    .C_CLK_2_EN             ( 0                 ),
    .C_SIM_DIVIDER          ( SIM_DIVIDER       )
) u_timer (
    .clk_1                  ( ap_clk_cont       ),
    .clk_2                  ( 1'b0              ),

    .toggle_1_sec_1         ( toggle_1_sec_cont ),
    .toggle_1_sec_2         (                   ),

    .timer_rst              ( 1'b0              ),
    .timer_end_1            (                   ),
    .timer_end_2            (                   )
);


for (genvar index=0; index < C_GT_NUM_LANE; index=index+1)
begin: lane

    wire mode_change_drp;
    wire axi_ctl_md_switch_drp;
    wire rx_reset_cdc;

    gt_traffic_gen #(
        .DEST_SYNC_FF   (DEST_SYNC_FF)
    ) tg (

        .clk_300        (ap_clk                 ),
        .rst_300        (ap_rst_gate            ),

        .clk_axis       (tx_clk_out[index]      ),

        .Config_Upd     (Config_Upd[index]      ),
        .Dest_Addr      (Dest_Addr[index]       ),
        .Source_Addr    (Source_Addr[index]     ),
        .Ether_Type     (Ether_Type[index]      ),
        .Payload_Type   (Payload_Type[index]    ),

        .Gen_Pkt        (Gen_Pkt[index]         ),
        .Gen_Length     (Gen_Length[index]      ),

        .Gen_Half_Full  (Gen_Half_Full[index]   ),
        .Gen_Full       (Gen_Full[index]        ),

        .tx_tready      (tx_axis_tready[index]  ),
        .tx_tvalid      (tx_axis_tvalid[index]  ),
        .tx_tdata       (tx_axis_tdata[index]   ),
        .tx_tlast       (tx_axis_tlast[index]   ),
        .tx_tkeep       (tx_axis_tkeep[index]   ),
        .tx_tuser       (tx_axis_tuser[index]   )
    );

    xpm_cdc_async_rst #(
        .DEST_SYNC_FF(DEST_SYNC_FF),
        .INIT_SYNC_FF(0),               // DECIMAL; 0=disable simulation init values, 1=enable simulation init values
        .RST_ACTIVE_HIGH(1)             // DECIMAL; 0=active low reset, 1=active high reset
    )
    cdc_rx_reset (
        .dest_arst(rx_reset_cdc),
        .dest_clk(rx_core_clk[index] ),
        .src_arst(rx_reset[index])
    );

    gt_traffic_cnt #(
        .DEST_SYNC_FF   (DEST_SYNC_FF)
    ) rx_tc (
        .clk_300                (ap_clk                     ),
        .rst_300                (ap_rst_gate                ),
        .toggle_1sec            (toggle_1_sec_cont          ),

        .clk_axis               (rx_core_clk[index]         ),
        .rst_axis               (rx_reset_cdc               ),

        .Config_Upd             (Config_Upd[index]          ),
        .Source_Addr            (Dest_Addr[index]           ),
        .Dest_Addr              (Source_Addr[index]         ),

        .Latch_and_Clear_Cnt    (Traffic_Cnt_Latch          ),

        .axi_tvalid             (rx_axis_tvalid[index]      ),
        .axi_tdata              (rx_axis_tdata[index]       ),
        .axi_tkeep              (rx_axis_tkeep[index]       ),
        .axi_tlast              (rx_axis_tlast[index]       ),
        .axi_tuser              (rx_axis_tuser[index]       ),

        .Cnt_RAM_Rd_Addr        (Cnt_RAM_Rd_Addr[index]),
        .Cnt_RAM_Out            (Cnt_RAM_Out[index])
    );

    xpm_cdc_pulse #(
        .DEST_SYNC_FF   (DEST_SYNC_FF),
        .RST_USED       (1),
        .REG_OUTPUT     (1)
    ) cdc_switch1 (
        .src_clk    (ap_clk             ),
        .src_rst    (ap_rst_gate        ),
        .src_pulse  (mode_change[index] ),

        .dest_clk   (gt_drpclk          ),
        .dest_rst   (gt_drprst          ),
        .dest_pulse (mode_change_drp    )
    );

    // ignore XXV_Ethernet update rate request CR-1065681 / CR-1065685
//    xpm_cdc_pulse #(
//        .DEST_SYNC_FF   (DEST_SYNC_FF),
//        .RST_USED       (1),
//        .REG_OUTPUT     (1)
//    ) cdc_switch2 (
//        .src_clk    (rx_clk_out[index]          ),
//        .src_rst    (user_rx_reset[index]       ),
//        .src_pulse  (axi_ctl_md_switch[index]   ),
//
//        .dest_clk   (gt_drpclk                  ),
//        .dest_rst   (gt_drprst                  ),
//        .dest_pulse (axi_ctl_md_switch_drp      )
//    );

    mac_core_trans_debug tr_dbg (
        .drp_clk                    (gt_drpclk                  ),
        .reset                      (gt_drprst                  ),

        .gt_drpdo                   (gt_drpdo[index]            ),
        .gt_drprdy                  (gt_drprdy[index]           ),
        .gt_drpen                   (gt_drpen[index]            ),
        .gt_drpwe                   (gt_drpwe[index]            ),
        .gt_drpaddr                 (gt_drpaddr[index]          ),
        .gt_drpdi                   (gt_drpdi[index]            ),

        //.axi_ctl_core_mode_switch   (axi_ctl_md_switch_drp      ),  // ignore XXV_Ethernet update rate request CR-1065681 / CR-1065685
        .axi_ctl_core_mode_switch   (1'b0      ),
        .mode_change                (mode_change_drp            ),
        .gt_drp_done                (gt_drp_done[index]         ),
        .ctl_rx_rate_10g_25gn       (ctl_rx_rate[index]         ),
        .txpllclksel                (txpllclksel_in[index]      ),
        .rxpllclksel                (rxpllclksel_in[index]      ),
        .txsysclksel                (txsysclksel_in[index]      ),
        .rxsysclksel                (rxsysclksel_in[index]      ),
        .rxdfecfokfcnum             (rxdfecfokfcnum[index]      ),
        .rxafecfoken                (rxafecfoken[index]         )
    );

    assign  txoutclksel_in[index]   = 3'b101;
    assign  rxoutclksel_in[index]   = 3'b101;

    assign  tx_preamblein[index]    = 56'h00000000000000;
    assign  ctl_tx_send_lfi[index]  = 1'b0;
    assign  ctl_tx_send_rfi[index]  = 1'b0;
    assign  ctl_tx_send_idle[index] = 1'b0;

    assign  s_axi_bready[index]     = 1'b1;

// coming from trans_debug.v
    assign gt_eyescanreset[index]   = 1'b0;
    assign gt_eyescantrigger[index] = 1'b0;
    assign gt_pcsrsvdin[index]      = 16'b0;

    assign gt_rxbufreset[index]     = 1'b0;
    assign gt_rxcdrhold[index]      = 1'b0;
    assign gt_rxcommadeten[index]   = 1'b0;
    assign gt_rxdfeagchold[index]   = 1'b0;
    assign gt_rxdfelpmreset[index]  = 1'b0;
    assign gt_rxlatclk[index]       = gt_drpclk;
    assign gt_rxpcsreset[index]     = 1'b0;
    assign gt_rxpmareset[index]     = 1'b0;
    assign gt_rxpolarity[index]     = 1'b0;
    assign gt_rxprbscntreset[index] = 1'b0;
    assign gt_rxprbssel[index]      = 4'b0;
    assign gt_rxrate[index]         = 3'b0;
    assign gt_rxslide_in[index]     = 1'b0;

    assign gt_txinhibit[index]      = 1'b0;
    assign gt_txlatclk[index]       = gt_drpclk;
    assign gt_txpcsreset[index]     = 1'b0;
    assign gt_txpmareset[index]     = 1'b0;
    assign gt_txprbsforceerr[index] = 1'b0;
    assign gt_txelecidle[index]     = 1'b0;
    assign gt_txprbssel[index]      = 4'b0;

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF      ),
        .INIT_SYNC_FF   ( 0                 ),
        .SRC_INPUT_REG  ( 0                 ),
        .SIM_ASSERT_CHK ( 0                 )
    )
    xpm_cdc_tx_pol (
        .src_clk  ( ap_clk                ),
        .src_in   ( gt_tx_polarity[index] ),
        .dest_out ( gt_txpolarity[index]  ),
        .dest_clk ( tx_clk_out[index]     )
    );

end


gt_mac_config #(
    .C_MAJOR_VERSION        (C_MAJOR_VERSION    ),
    .C_MINOR_VERSION        (C_MINOR_VERSION    ),
    .C_BUILD_VERSION        (C_BUILD_VERSION    ),
    .C_CLOCK0_FREQ          (C_CLOCK0_FREQ      ),
    .C_CLOCK1_FREQ          (C_CLOCK1_FREQ      ),
    .DEST_SYNC_FF           (DEST_SYNC_FF       ),
    .C_BLOCK_ID             (C_BLOCK_ID         ),
    .C_GT_INDEX             (C_GT_INDEX         ),
    .C_GT_NUM_GT            (C_GT_NUM_GT        ),
    .C_GT_NUM_LANE          (C_GT_NUM_LANE      ),
    .C_GT_RATE              (C_GT_RATE          ),
    .C_GT_TYPE              (C_GT_TYPE          ),
    .C_GT_MAC_ENABLE_RSFEC  (C_GT_MAC_ENABLE_RSFEC )
    ) mc (
    .clk_300                (ap_clk         ),
    .clk_300_cont           (ap_clk_cont    ),
    .rst_300_raw            (ap_rst         ),
    .rst_300_gate           (ap_rst_gate    ),

    .axi_clk                (axi_aclk       ),
    .axi_clk_rst_n          (axi_aresetn    ),

    .watchdog_alarm_in      (watchdog_alarm_in),

    .rst_allow              (ap_rst_allow   ),

    .Start                  (start_pulse    ),
    .Done                   (done_pulse     ),
    .AXI00_Ptr0             (axi00_ptr0     ),
    .Scalar00               (scalar00       ),
    .Scalar01               (scalar01       ),

    .PLRAM_awready          (plram_awready  ),
    .PLRAM_awvalid          (plram_awvalid  ),
    .PLRAM_awaddr           (plram_awaddr   ),
    .PLRAM_awlen            (plram_awlen    ),

    .PLRAM_wready           (plram_wready   ),
    .PLRAM_wvalid           (plram_wvalid   ),
    .PLRAM_wdata            (plram_wdata    ),
    .PLRAM_wstrb            (plram_wstrb    ),
    .PLRAM_wlast            (plram_wlast    ),

    .PLRAM_bvalid           (plram_bvalid   ),
    .PLRAM_bready           (plram_bready   ),

    .PLRAM_arready          (plram_arready  ),
    .PLRAM_arvalid          (plram_arvalid  ),
    .PLRAM_araddr           (plram_araddr   ),
    .PLRAM_arlen            (plram_arlen    ),

    .PLRAM_rready           (plram_rready   ),
    .PLRAM_rvalid           (plram_rvalid   ),
    .PLRAM_rlast            (plram_rlast    ),
    .PLRAM_rdata            (plram_rdata    ),

    .MAC_aresetn            (MAC_aresetn    ),
    .MAC_local_fault        (MAC_local_fault),

    .MAC_Sel_Mode           (ctl_rx_rate    ),
    .MAC_Mode_Change        (mode_change    ),
    .MAC_PM_Tick            (pm_tick        ),
    .MAC_rx_reset           (rx_reset       ),
    .MAC_tx_reset           (tx_reset       ),

    .MAC_awready            (s_axi_awready  ),
    .MAC_awvalid            (s_axi_awvalid  ),
    .MAC_awaddr_0           (s_axi_awaddr[0]),
    .MAC_awaddr_1           (s_axi_awaddr[1]),
    .MAC_awaddr_2           (s_axi_awaddr[2]),
    .MAC_awaddr_3           (s_axi_awaddr[3]),

    .MAC_wready             (s_axi_wready   ),
    .MAC_wvalid             (s_axi_wvalid   ),
    .MAC_wdata_0            (s_axi_wdata[0] ),
    .MAC_wdata_1            (s_axi_wdata[1] ),
    .MAC_wdata_2            (s_axi_wdata[2] ),
    .MAC_wdata_3            (s_axi_wdata[3] ),
    .MAC_wstrb_0            (s_axi_wstrb[0] ),
    .MAC_wstrb_1            (s_axi_wstrb[1] ),
    .MAC_wstrb_2            (s_axi_wstrb[2] ),
    .MAC_wstrb_3            (s_axi_wstrb[3] ),

    .MAC_arready            (s_axi_arready  ),
    .MAC_arvalid            (s_axi_arvalid  ),
    .MAC_araddr_0           (s_axi_araddr[0]),
    .MAC_araddr_1           (s_axi_araddr[1]),
    .MAC_araddr_2           (s_axi_araddr[2]),
    .MAC_araddr_3           (s_axi_araddr[3]),

    .MAC_rready             (s_axi_rready   ),
    .MAC_rvalid             (s_axi_rvalid   ),
    .MAC_rdata_0            (s_axi_rdata[0] ),
    .MAC_rdata_1            (s_axi_rdata[1] ),
    .MAC_rdata_2            (s_axi_rdata[2] ),
    .MAC_rdata_3            (s_axi_rdata[3] ),
    .MAC_rresp_0            (s_axi_rresp[0] ),
    .MAC_rresp_1            (s_axi_rresp[1] ),
    .MAC_rresp_2            (s_axi_rresp[2] ),
    .MAC_rresp_3            (s_axi_rresp[3] ),

    .Config_Upd             (Config_Upd     ),
    .Dest_MAC_Addr_0        (Dest_Addr[0]   ),
    .Dest_MAC_Addr_1        (Dest_Addr[1]   ),
    .Dest_MAC_Addr_2        (Dest_Addr[2]   ),
    .Dest_MAC_Addr_3        (Dest_Addr[3]   ),
    .Source_MAC_Addr_0      (Source_Addr[0] ),
    .Source_MAC_Addr_1      (Source_Addr[1] ),
    .Source_MAC_Addr_2      (Source_Addr[2] ),
    .Source_MAC_Addr_3      (Source_Addr[3] ),
    .MAC_Ethertype_0        (Ether_Type[0]  ),
    .MAC_Ethertype_1        (Ether_Type[1]  ),
    .MAC_Ethertype_2        (Ether_Type[2]  ),
    .MAC_Ethertype_3        (Ether_Type[3]  ),
    .MAC_Traffic_Type_0     (Payload_Type[0]),
    .MAC_Traffic_Type_1     (Payload_Type[1]),
    .MAC_Traffic_Type_2     (Payload_Type[2]),
    .MAC_Traffic_Type_3     (Payload_Type[3]),

    .GT_Rx_LPM_En           (gt_rxlpmen         ),
    .GT_TX_Polarity         (gt_tx_polarity     ),
    .GT_Tx_Main_Cursor_0    (gt_txmaincursor[0] ),
    .GT_Tx_Main_Cursor_1    (gt_txmaincursor[1] ),
    .GT_Tx_Main_Cursor_2    (gt_txmaincursor[2] ),
    .GT_Tx_Main_Cursor_3    (gt_txmaincursor[3] ),
    .GT_Tx_Post_Cursor_0    (gt_txpostcursor[0] ),
    .GT_Tx_Post_Cursor_1    (gt_txpostcursor[1] ),
    .GT_Tx_Post_Cursor_2    (gt_txpostcursor[2] ),
    .GT_Tx_Post_Cursor_3    (gt_txpostcursor[3] ),
    .GT_Tx_Pre_Cursor_0     (gt_txprecursor[0]  ),
    .GT_Tx_Pre_Cursor_1     (gt_txprecursor[1]  ),
    .GT_Tx_Pre_Cursor_2     (gt_txprecursor[2]  ),
    .GT_Tx_Pre_Cursor_3     (gt_txprecursor[3]  ),
    .GT_Tx_Diff_Ctrl_0      (gt_txdiffctrl[0]   ),
    .GT_Tx_Diff_Ctrl_1      (gt_txdiffctrl[1]   ),
    .GT_Tx_Diff_Ctrl_2      (gt_txdiffctrl[2]   ),
    .GT_Tx_Diff_Ctrl_3      (gt_txdiffctrl[3]   ),

    .Gen_Pkt                (Gen_Pkt        ),
    .Gen_Length_0           (Gen_Length[0]  ),
    .Gen_Length_1           (Gen_Length[1]  ),
    .Gen_Length_2           (Gen_Length[2]  ),
    .Gen_Length_3           (Gen_Length[3]  ),
    .Gen_Half_Full          (Gen_Half_Full  ),

    .Mac_Rst_Status         (mac_rst_status),

    .Traffic_Cnt_Latch      (Traffic_Cnt_Latch),

    .Cnt_RAM_Rd_Addr_0      (Cnt_RAM_Rd_Addr[0]),
    .Cnt_RAM_Rd_Addr_1      (Cnt_RAM_Rd_Addr[1]),
    .Cnt_RAM_Rd_Addr_2      (Cnt_RAM_Rd_Addr[2]),
    .Cnt_RAM_Rd_Addr_3      (Cnt_RAM_Rd_Addr[3]),

    .Cnt_RAM_Data_0         (Cnt_RAM_Out[0]),
    .Cnt_RAM_Data_1         (Cnt_RAM_Out[1]),
    .Cnt_RAM_Data_2         (Cnt_RAM_Out[2]),
    .Cnt_RAM_Data_3         (Cnt_RAM_Out[3])
);

endmodule : gt_test_mac_top
`default_nettype wire
