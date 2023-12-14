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

`timescale 1 ps / 1 ps


// HOW TO create this file
//
// open vivado with VCK5000 board, use the following command
//      create_bd_design "xbtest_sub_xxv_gt_0"
//      use bd.tcl and port.tcl for versal and VCK5000
//
//  save the project
//      right click on the bd in the Sources window => create HDL wrapper (select let vivado namge wrapper and auto-update)
//
//  add the second instance and remove the RS_FEC_stat connections

module xbtest_sub_xxv_gt_wrapper_versal_gty_4lanes #(
    parameter integer C_GT_MAC_ENABLE_RSFEC = 0
) (
    FEC_stat_0_0_stat_fec_inc_cant_correct_count,
    FEC_stat_0_0_stat_fec_inc_correct_count,
    FEC_stat_0_0_stat_fec_lock_error,
    FEC_stat_0_0_stat_fec_rx_lock,
    FEC_stat_0_1_stat_fec_inc_cant_correct_count,
    FEC_stat_0_1_stat_fec_inc_correct_count,
    FEC_stat_0_1_stat_fec_lock_error,
    FEC_stat_0_1_stat_fec_rx_lock,
    FEC_stat_0_2_stat_fec_inc_cant_correct_count,
    FEC_stat_0_2_stat_fec_inc_correct_count,
    FEC_stat_0_2_stat_fec_lock_error,
    FEC_stat_0_2_stat_fec_rx_lock,
    FEC_stat_0_3_stat_fec_inc_cant_correct_count,
    FEC_stat_0_3_stat_fec_inc_correct_count,
    FEC_stat_0_3_stat_fec_lock_error,
    FEC_stat_0_3_stat_fec_rx_lock,
    RS_FEC_stat_0_0_stat_rx_rsfec_corrected_cw_inc,
    RS_FEC_stat_0_0_stat_rx_rsfec_err_count0_inc,
    RS_FEC_stat_0_0_stat_rx_rsfec_hi_ser,
    RS_FEC_stat_0_0_stat_rx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_0_stat_rx_rsfec_uncorrected_cw_inc,
    RS_FEC_stat_0_0_stat_tx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_1_stat_rx_rsfec_corrected_cw_inc,
    RS_FEC_stat_0_1_stat_rx_rsfec_err_count0_inc,
    RS_FEC_stat_0_1_stat_rx_rsfec_hi_ser,
    RS_FEC_stat_0_1_stat_rx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_1_stat_rx_rsfec_uncorrected_cw_inc,
    RS_FEC_stat_0_1_stat_tx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_2_stat_rx_rsfec_corrected_cw_inc,
    RS_FEC_stat_0_2_stat_rx_rsfec_err_count0_inc,
    RS_FEC_stat_0_2_stat_rx_rsfec_hi_ser,
    RS_FEC_stat_0_2_stat_rx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_2_stat_rx_rsfec_uncorrected_cw_inc,
    RS_FEC_stat_0_2_stat_tx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_3_stat_rx_rsfec_corrected_cw_inc,
    RS_FEC_stat_0_3_stat_rx_rsfec_err_count0_inc,
    RS_FEC_stat_0_3_stat_rx_rsfec_hi_ser,
    RS_FEC_stat_0_3_stat_rx_rsfec_lane_alignment_status,
    RS_FEC_stat_0_3_stat_rx_rsfec_uncorrected_cw_inc,
    RS_FEC_stat_0_3_stat_tx_rsfec_lane_alignment_status,
    apb3clk_quad,
    axis_rx_0_0_tdata,
    axis_rx_0_0_tkeep,
    axis_rx_0_0_tlast,
    axis_rx_0_0_tuser,
    axis_rx_0_0_tvalid,
    axis_rx_0_1_tdata,
    axis_rx_0_1_tkeep,
    axis_rx_0_1_tlast,
    axis_rx_0_1_tuser,
    axis_rx_0_1_tvalid,
    axis_rx_0_2_tdata,
    axis_rx_0_2_tkeep,
    axis_rx_0_2_tlast,
    axis_rx_0_2_tuser,
    axis_rx_0_2_tvalid,
    axis_rx_0_3_tdata,
    axis_rx_0_3_tkeep,
    axis_rx_0_3_tlast,
    axis_rx_0_3_tuser,
    axis_rx_0_3_tvalid,
    axis_tx_0_0_tdata,
    axis_tx_0_0_tkeep,
    axis_tx_0_0_tlast,
    axis_tx_0_0_tready,
    axis_tx_0_0_tuser,
    axis_tx_0_0_tvalid,
    axis_tx_0_1_tdata,
    axis_tx_0_1_tkeep,
    axis_tx_0_1_tlast,
    axis_tx_0_1_tready,
    axis_tx_0_1_tuser,
    axis_tx_0_1_tvalid,
    axis_tx_0_2_tdata,
    axis_tx_0_2_tkeep,
    axis_tx_0_2_tlast,
    axis_tx_0_2_tready,
    axis_tx_0_2_tuser,
    axis_tx_0_2_tvalid,
    axis_tx_0_3_tdata,
    axis_tx_0_3_tkeep,
    axis_tx_0_3_tlast,
    axis_tx_0_3_tready,
    axis_tx_0_3_tuser,
    axis_tx_0_3_tvalid,
    ctl_tx_0_0_ctl_tx_send_idle,
    ctl_tx_0_0_ctl_tx_send_lfi,
    ctl_tx_0_0_ctl_tx_send_rfi,
    ctl_tx_0_1_ctl_tx_send_idle,
    ctl_tx_0_1_ctl_tx_send_lfi,
    ctl_tx_0_1_ctl_tx_send_rfi,
    ctl_tx_0_2_ctl_tx_send_idle,
    ctl_tx_0_2_ctl_tx_send_lfi,
    ctl_tx_0_2_ctl_tx_send_rfi,
    ctl_tx_0_3_ctl_tx_send_idle,
    ctl_tx_0_3_ctl_tx_send_lfi,
    ctl_tx_0_3_ctl_tx_send_rfi,
    gt_rxn_in_0,
    gt_rxp_in_0,
    gt_txn_out_0,
    gt_txp_out_0,
    gtwiz_reset_clk_freerun_in_0_0,
    gtwiz_reset_clk_freerun_in_0_1,
    gtwiz_reset_clk_freerun_in_0_2,
    gtwiz_reset_clk_freerun_in_0_3,
    pm_tick_0_0,
    pm_tick_0_1,
    pm_tick_0_2,
    pm_tick_0_3,
    gt_refclk_clk_n,
    gt_refclk_clk_p,
    rx_core_clk_0_0,
    rx_core_clk_0_1,
    rx_core_clk_0_2,
    rx_core_clk_0_3,
    rx_preambleout_0_0,
    rx_preambleout_0_1,
    rx_preambleout_0_2,
    rx_preambleout_0_3,
    rx_reset_0_0,
    rx_reset_0_1,
    rx_reset_0_2,
    rx_reset_0_3,
    rx_resetdone_out_0_0,
    rx_resetdone_out_0_1,
    rx_resetdone_out_0_2,
    rx_resetdone_out_0_3,
    rx_serdes_clk_0_0,
    rx_serdes_clk_0_1,
    rx_serdes_clk_0_2,
    rx_serdes_clk_0_3,
    rx_serdes_reset_0_0,
    rx_serdes_reset_0_1,
    rx_serdes_reset_0_2,
    rx_serdes_reset_0_3,
    rx_usrclk_0,
    rx_usrclk_1,
    rx_usrclk_2,
    rx_usrclk_3,
    s_axi_0_0_araddr,
    s_axi_0_0_arready,
    s_axi_0_0_arvalid,
    s_axi_0_0_awaddr,
    s_axi_0_0_awready,
    s_axi_0_0_awvalid,
    s_axi_0_0_bready,
    s_axi_0_0_bresp,
    s_axi_0_0_bvalid,
    s_axi_0_0_rdata,
    s_axi_0_0_rready,
    s_axi_0_0_rresp,
    s_axi_0_0_rvalid,
    s_axi_0_0_wdata,
    s_axi_0_0_wready,
    s_axi_0_0_wstrb,
    s_axi_0_0_wvalid,
    s_axi_0_1_araddr,
    s_axi_0_1_arready,
    s_axi_0_1_arvalid,
    s_axi_0_1_awaddr,
    s_axi_0_1_awready,
    s_axi_0_1_awvalid,
    s_axi_0_1_bready,
    s_axi_0_1_bresp,
    s_axi_0_1_bvalid,
    s_axi_0_1_rdata,
    s_axi_0_1_rready,
    s_axi_0_1_rresp,
    s_axi_0_1_rvalid,
    s_axi_0_1_wdata,
    s_axi_0_1_wready,
    s_axi_0_1_wstrb,
    s_axi_0_1_wvalid,
    s_axi_0_2_araddr,
    s_axi_0_2_arready,
    s_axi_0_2_arvalid,
    s_axi_0_2_awaddr,
    s_axi_0_2_awready,
    s_axi_0_2_awvalid,
    s_axi_0_2_bready,
    s_axi_0_2_bresp,
    s_axi_0_2_bvalid,
    s_axi_0_2_rdata,
    s_axi_0_2_rready,
    s_axi_0_2_rresp,
    s_axi_0_2_rvalid,
    s_axi_0_2_wdata,
    s_axi_0_2_wready,
    s_axi_0_2_wstrb,
    s_axi_0_2_wvalid,
    s_axi_0_3_araddr,
    s_axi_0_3_arready,
    s_axi_0_3_arvalid,
    s_axi_0_3_awaddr,
    s_axi_0_3_awready,
    s_axi_0_3_awvalid,
    s_axi_0_3_bready,
    s_axi_0_3_bresp,
    s_axi_0_3_bvalid,
    s_axi_0_3_rdata,
    s_axi_0_3_rready,
    s_axi_0_3_rresp,
    s_axi_0_3_rvalid,
    s_axi_0_3_wdata,
    s_axi_0_3_wready,
    s_axi_0_3_wstrb,
    s_axi_0_3_wvalid,
    s_axi_aclk_0_0,
    s_axi_aclk_0_1,
    s_axi_aclk_0_2,
    s_axi_aclk_0_3,
    s_axi_aresetn_0_0,
    s_axi_aresetn_0_1,
    s_axi_aresetn_0_2,
    s_axi_aresetn_0_3,
    stat_rx_0_0_stat_rx_bad_code,
    stat_rx_0_0_stat_rx_bad_fcs,
    stat_rx_0_0_stat_rx_bad_preamble,
    stat_rx_0_0_stat_rx_bad_sfd,
    stat_rx_0_0_stat_rx_block_lock,
    stat_rx_0_0_stat_rx_broadcast,
    stat_rx_0_0_stat_rx_fragment,
    stat_rx_0_0_stat_rx_framing_err,
    stat_rx_0_0_stat_rx_framing_err_valid,
    stat_rx_0_0_stat_rx_got_signal_os,
    stat_rx_0_0_stat_rx_hi_ber,
    stat_rx_0_0_stat_rx_inrangeerr,
    stat_rx_0_0_stat_rx_internal_local_fault,
    stat_rx_0_0_stat_rx_jabber,
    stat_rx_0_0_stat_rx_local_fault,
    stat_rx_0_0_stat_rx_multicast,
    stat_rx_0_0_stat_rx_oversize,
    stat_rx_0_0_stat_rx_packet_1024_1518_bytes,
    stat_rx_0_0_stat_rx_packet_128_255_bytes,
    stat_rx_0_0_stat_rx_packet_1519_1522_bytes,
    stat_rx_0_0_stat_rx_packet_1523_1548_bytes,
    stat_rx_0_0_stat_rx_packet_1549_2047_bytes,
    stat_rx_0_0_stat_rx_packet_2048_4095_bytes,
    stat_rx_0_0_stat_rx_packet_256_511_bytes,
    stat_rx_0_0_stat_rx_packet_4096_8191_bytes,
    stat_rx_0_0_stat_rx_packet_512_1023_bytes,
    stat_rx_0_0_stat_rx_packet_64_bytes,
    stat_rx_0_0_stat_rx_packet_65_127_bytes,
    stat_rx_0_0_stat_rx_packet_8192_9215_bytes,
    stat_rx_0_0_stat_rx_packet_bad_fcs,
    stat_rx_0_0_stat_rx_packet_large,
    stat_rx_0_0_stat_rx_packet_small,
    stat_rx_0_0_stat_rx_received_local_fault,
    stat_rx_0_0_stat_rx_remote_fault,
    stat_rx_0_0_stat_rx_stomped_fcs,
    stat_rx_0_0_stat_rx_test_pattern_mismatch,
    stat_rx_0_0_stat_rx_toolong,
    stat_rx_0_0_stat_rx_total_bytes,
    stat_rx_0_0_stat_rx_total_good_bytes,
    stat_rx_0_0_stat_rx_total_good_packets,
    stat_rx_0_0_stat_rx_total_packets,
    stat_rx_0_0_stat_rx_truncated,
    stat_rx_0_0_stat_rx_undersize,
    stat_rx_0_0_stat_rx_unicast,
    stat_rx_0_0_stat_rx_valid_ctrl_code,
    stat_rx_0_0_stat_rx_vlan,
    stat_rx_0_1_stat_rx_bad_code,
    stat_rx_0_1_stat_rx_bad_fcs,
    stat_rx_0_1_stat_rx_bad_preamble,
    stat_rx_0_1_stat_rx_bad_sfd,
    stat_rx_0_1_stat_rx_block_lock,
    stat_rx_0_1_stat_rx_broadcast,
    stat_rx_0_1_stat_rx_fragment,
    stat_rx_0_1_stat_rx_framing_err,
    stat_rx_0_1_stat_rx_framing_err_valid,
    stat_rx_0_1_stat_rx_got_signal_os,
    stat_rx_0_1_stat_rx_hi_ber,
    stat_rx_0_1_stat_rx_inrangeerr,
    stat_rx_0_1_stat_rx_internal_local_fault,
    stat_rx_0_1_stat_rx_jabber,
    stat_rx_0_1_stat_rx_local_fault,
    stat_rx_0_1_stat_rx_multicast,
    stat_rx_0_1_stat_rx_oversize,
    stat_rx_0_1_stat_rx_packet_1024_1518_bytes,
    stat_rx_0_1_stat_rx_packet_128_255_bytes,
    stat_rx_0_1_stat_rx_packet_1519_1522_bytes,
    stat_rx_0_1_stat_rx_packet_1523_1548_bytes,
    stat_rx_0_1_stat_rx_packet_1549_2047_bytes,
    stat_rx_0_1_stat_rx_packet_2048_4095_bytes,
    stat_rx_0_1_stat_rx_packet_256_511_bytes,
    stat_rx_0_1_stat_rx_packet_4096_8191_bytes,
    stat_rx_0_1_stat_rx_packet_512_1023_bytes,
    stat_rx_0_1_stat_rx_packet_64_bytes,
    stat_rx_0_1_stat_rx_packet_65_127_bytes,
    stat_rx_0_1_stat_rx_packet_8192_9215_bytes,
    stat_rx_0_1_stat_rx_packet_bad_fcs,
    stat_rx_0_1_stat_rx_packet_large,
    stat_rx_0_1_stat_rx_packet_small,
    stat_rx_0_1_stat_rx_received_local_fault,
    stat_rx_0_1_stat_rx_remote_fault,
    stat_rx_0_1_stat_rx_stomped_fcs,
    stat_rx_0_1_stat_rx_test_pattern_mismatch,
    stat_rx_0_1_stat_rx_toolong,
    stat_rx_0_1_stat_rx_total_bytes,
    stat_rx_0_1_stat_rx_total_good_bytes,
    stat_rx_0_1_stat_rx_total_good_packets,
    stat_rx_0_1_stat_rx_total_packets,
    stat_rx_0_1_stat_rx_truncated,
    stat_rx_0_1_stat_rx_undersize,
    stat_rx_0_1_stat_rx_unicast,
    stat_rx_0_1_stat_rx_valid_ctrl_code,
    stat_rx_0_1_stat_rx_vlan,
    stat_rx_0_2_stat_rx_bad_code,
    stat_rx_0_2_stat_rx_bad_fcs,
    stat_rx_0_2_stat_rx_bad_preamble,
    stat_rx_0_2_stat_rx_bad_sfd,
    stat_rx_0_2_stat_rx_block_lock,
    stat_rx_0_2_stat_rx_broadcast,
    stat_rx_0_2_stat_rx_fragment,
    stat_rx_0_2_stat_rx_framing_err,
    stat_rx_0_2_stat_rx_framing_err_valid,
    stat_rx_0_2_stat_rx_got_signal_os,
    stat_rx_0_2_stat_rx_hi_ber,
    stat_rx_0_2_stat_rx_inrangeerr,
    stat_rx_0_2_stat_rx_internal_local_fault,
    stat_rx_0_2_stat_rx_jabber,
    stat_rx_0_2_stat_rx_local_fault,
    stat_rx_0_2_stat_rx_multicast,
    stat_rx_0_2_stat_rx_oversize,
    stat_rx_0_2_stat_rx_packet_1024_1518_bytes,
    stat_rx_0_2_stat_rx_packet_128_255_bytes,
    stat_rx_0_2_stat_rx_packet_1519_1522_bytes,
    stat_rx_0_2_stat_rx_packet_1523_1548_bytes,
    stat_rx_0_2_stat_rx_packet_1549_2047_bytes,
    stat_rx_0_2_stat_rx_packet_2048_4095_bytes,
    stat_rx_0_2_stat_rx_packet_256_511_bytes,
    stat_rx_0_2_stat_rx_packet_4096_8191_bytes,
    stat_rx_0_2_stat_rx_packet_512_1023_bytes,
    stat_rx_0_2_stat_rx_packet_64_bytes,
    stat_rx_0_2_stat_rx_packet_65_127_bytes,
    stat_rx_0_2_stat_rx_packet_8192_9215_bytes,
    stat_rx_0_2_stat_rx_packet_bad_fcs,
    stat_rx_0_2_stat_rx_packet_large,
    stat_rx_0_2_stat_rx_packet_small,
    stat_rx_0_2_stat_rx_received_local_fault,
    stat_rx_0_2_stat_rx_remote_fault,
    stat_rx_0_2_stat_rx_stomped_fcs,
    stat_rx_0_2_stat_rx_test_pattern_mismatch,
    stat_rx_0_2_stat_rx_toolong,
    stat_rx_0_2_stat_rx_total_bytes,
    stat_rx_0_2_stat_rx_total_good_bytes,
    stat_rx_0_2_stat_rx_total_good_packets,
    stat_rx_0_2_stat_rx_total_packets,
    stat_rx_0_2_stat_rx_truncated,
    stat_rx_0_2_stat_rx_undersize,
    stat_rx_0_2_stat_rx_unicast,
    stat_rx_0_2_stat_rx_valid_ctrl_code,
    stat_rx_0_2_stat_rx_vlan,
    stat_rx_0_3_stat_rx_bad_code,
    stat_rx_0_3_stat_rx_bad_fcs,
    stat_rx_0_3_stat_rx_bad_preamble,
    stat_rx_0_3_stat_rx_bad_sfd,
    stat_rx_0_3_stat_rx_block_lock,
    stat_rx_0_3_stat_rx_broadcast,
    stat_rx_0_3_stat_rx_fragment,
    stat_rx_0_3_stat_rx_framing_err,
    stat_rx_0_3_stat_rx_framing_err_valid,
    stat_rx_0_3_stat_rx_got_signal_os,
    stat_rx_0_3_stat_rx_hi_ber,
    stat_rx_0_3_stat_rx_inrangeerr,
    stat_rx_0_3_stat_rx_internal_local_fault,
    stat_rx_0_3_stat_rx_jabber,
    stat_rx_0_3_stat_rx_local_fault,
    stat_rx_0_3_stat_rx_multicast,
    stat_rx_0_3_stat_rx_oversize,
    stat_rx_0_3_stat_rx_packet_1024_1518_bytes,
    stat_rx_0_3_stat_rx_packet_128_255_bytes,
    stat_rx_0_3_stat_rx_packet_1519_1522_bytes,
    stat_rx_0_3_stat_rx_packet_1523_1548_bytes,
    stat_rx_0_3_stat_rx_packet_1549_2047_bytes,
    stat_rx_0_3_stat_rx_packet_2048_4095_bytes,
    stat_rx_0_3_stat_rx_packet_256_511_bytes,
    stat_rx_0_3_stat_rx_packet_4096_8191_bytes,
    stat_rx_0_3_stat_rx_packet_512_1023_bytes,
    stat_rx_0_3_stat_rx_packet_64_bytes,
    stat_rx_0_3_stat_rx_packet_65_127_bytes,
    stat_rx_0_3_stat_rx_packet_8192_9215_bytes,
    stat_rx_0_3_stat_rx_packet_bad_fcs,
    stat_rx_0_3_stat_rx_packet_large,
    stat_rx_0_3_stat_rx_packet_small,
    stat_rx_0_3_stat_rx_received_local_fault,
    stat_rx_0_3_stat_rx_remote_fault,
    stat_rx_0_3_stat_rx_stomped_fcs,
    stat_rx_0_3_stat_rx_test_pattern_mismatch,
    stat_rx_0_3_stat_rx_toolong,
    stat_rx_0_3_stat_rx_total_bytes,
    stat_rx_0_3_stat_rx_total_good_bytes,
    stat_rx_0_3_stat_rx_total_good_packets,
    stat_rx_0_3_stat_rx_total_packets,
    stat_rx_0_3_stat_rx_truncated,
    stat_rx_0_3_stat_rx_undersize,
    stat_rx_0_3_stat_rx_unicast,
    stat_rx_0_3_stat_rx_valid_ctrl_code,
    stat_rx_0_3_stat_rx_vlan,
    stat_rx_status_0_0,
    stat_rx_status_0_1,
    stat_rx_status_0_2,
    stat_rx_status_0_3,
    stat_tx_0_0_stat_tx_bad_fcs,
    stat_tx_0_0_stat_tx_broadcast,
    stat_tx_0_0_stat_tx_frame_error,
    stat_tx_0_0_stat_tx_local_fault,
    stat_tx_0_0_stat_tx_multicast,
    stat_tx_0_0_stat_tx_packet_1024_1518_bytes,
    stat_tx_0_0_stat_tx_packet_128_255_bytes,
    stat_tx_0_0_stat_tx_packet_1519_1522_bytes,
    stat_tx_0_0_stat_tx_packet_1523_1548_bytes,
    stat_tx_0_0_stat_tx_packet_1549_2047_bytes,
    stat_tx_0_0_stat_tx_packet_2048_4095_bytes,
    stat_tx_0_0_stat_tx_packet_256_511_bytes,
    stat_tx_0_0_stat_tx_packet_4096_8191_bytes,
    stat_tx_0_0_stat_tx_packet_512_1023_bytes,
    stat_tx_0_0_stat_tx_packet_64_bytes,
    stat_tx_0_0_stat_tx_packet_65_127_bytes,
    stat_tx_0_0_stat_tx_packet_8192_9215_bytes,
    stat_tx_0_0_stat_tx_packet_large,
    stat_tx_0_0_stat_tx_packet_small,
    stat_tx_0_0_stat_tx_total_bytes,
    stat_tx_0_0_stat_tx_total_good_bytes,
    stat_tx_0_0_stat_tx_total_good_packets,
    stat_tx_0_0_stat_tx_total_packets,
    stat_tx_0_0_stat_tx_unicast,
    stat_tx_0_0_stat_tx_vlan,
    stat_tx_0_1_stat_tx_bad_fcs,
    stat_tx_0_1_stat_tx_broadcast,
    stat_tx_0_1_stat_tx_frame_error,
    stat_tx_0_1_stat_tx_local_fault,
    stat_tx_0_1_stat_tx_multicast,
    stat_tx_0_1_stat_tx_packet_1024_1518_bytes,
    stat_tx_0_1_stat_tx_packet_128_255_bytes,
    stat_tx_0_1_stat_tx_packet_1519_1522_bytes,
    stat_tx_0_1_stat_tx_packet_1523_1548_bytes,
    stat_tx_0_1_stat_tx_packet_1549_2047_bytes,
    stat_tx_0_1_stat_tx_packet_2048_4095_bytes,
    stat_tx_0_1_stat_tx_packet_256_511_bytes,
    stat_tx_0_1_stat_tx_packet_4096_8191_bytes,
    stat_tx_0_1_stat_tx_packet_512_1023_bytes,
    stat_tx_0_1_stat_tx_packet_64_bytes,
    stat_tx_0_1_stat_tx_packet_65_127_bytes,
    stat_tx_0_1_stat_tx_packet_8192_9215_bytes,
    stat_tx_0_1_stat_tx_packet_large,
    stat_tx_0_1_stat_tx_packet_small,
    stat_tx_0_1_stat_tx_total_bytes,
    stat_tx_0_1_stat_tx_total_good_bytes,
    stat_tx_0_1_stat_tx_total_good_packets,
    stat_tx_0_1_stat_tx_total_packets,
    stat_tx_0_1_stat_tx_unicast,
    stat_tx_0_1_stat_tx_vlan,
    stat_tx_0_2_stat_tx_bad_fcs,
    stat_tx_0_2_stat_tx_broadcast,
    stat_tx_0_2_stat_tx_frame_error,
    stat_tx_0_2_stat_tx_local_fault,
    stat_tx_0_2_stat_tx_multicast,
    stat_tx_0_2_stat_tx_packet_1024_1518_bytes,
    stat_tx_0_2_stat_tx_packet_128_255_bytes,
    stat_tx_0_2_stat_tx_packet_1519_1522_bytes,
    stat_tx_0_2_stat_tx_packet_1523_1548_bytes,
    stat_tx_0_2_stat_tx_packet_1549_2047_bytes,
    stat_tx_0_2_stat_tx_packet_2048_4095_bytes,
    stat_tx_0_2_stat_tx_packet_256_511_bytes,
    stat_tx_0_2_stat_tx_packet_4096_8191_bytes,
    stat_tx_0_2_stat_tx_packet_512_1023_bytes,
    stat_tx_0_2_stat_tx_packet_64_bytes,
    stat_tx_0_2_stat_tx_packet_65_127_bytes,
    stat_tx_0_2_stat_tx_packet_8192_9215_bytes,
    stat_tx_0_2_stat_tx_packet_large,
    stat_tx_0_2_stat_tx_packet_small,
    stat_tx_0_2_stat_tx_total_bytes,
    stat_tx_0_2_stat_tx_total_good_bytes,
    stat_tx_0_2_stat_tx_total_good_packets,
    stat_tx_0_2_stat_tx_total_packets,
    stat_tx_0_2_stat_tx_unicast,
    stat_tx_0_2_stat_tx_vlan,
    stat_tx_0_3_stat_tx_bad_fcs,
    stat_tx_0_3_stat_tx_broadcast,
    stat_tx_0_3_stat_tx_frame_error,
    stat_tx_0_3_stat_tx_local_fault,
    stat_tx_0_3_stat_tx_multicast,
    stat_tx_0_3_stat_tx_packet_1024_1518_bytes,
    stat_tx_0_3_stat_tx_packet_128_255_bytes,
    stat_tx_0_3_stat_tx_packet_1519_1522_bytes,
    stat_tx_0_3_stat_tx_packet_1523_1548_bytes,
    stat_tx_0_3_stat_tx_packet_1549_2047_bytes,
    stat_tx_0_3_stat_tx_packet_2048_4095_bytes,
    stat_tx_0_3_stat_tx_packet_256_511_bytes,
    stat_tx_0_3_stat_tx_packet_4096_8191_bytes,
    stat_tx_0_3_stat_tx_packet_512_1023_bytes,
    stat_tx_0_3_stat_tx_packet_64_bytes,
    stat_tx_0_3_stat_tx_packet_65_127_bytes,
    stat_tx_0_3_stat_tx_packet_8192_9215_bytes,
    stat_tx_0_3_stat_tx_packet_large,
    stat_tx_0_3_stat_tx_packet_small,
    stat_tx_0_3_stat_tx_total_bytes,
    stat_tx_0_3_stat_tx_total_good_bytes,
    stat_tx_0_3_stat_tx_total_good_packets,
    stat_tx_0_3_stat_tx_total_packets,
    stat_tx_0_3_stat_tx_unicast,
    stat_tx_0_3_stat_tx_vlan,
    tx_core_clk_0_0,
    tx_core_clk_0_1,
    tx_core_clk_0_2,
    tx_core_clk_0_3,
    tx_preamblein_0_0,
    tx_preamblein_0_1,
    tx_preamblein_0_2,
    tx_preamblein_0_3,
    tx_reset_0_0,
    tx_reset_0_1,
    tx_reset_0_2,
    tx_reset_0_3,
    tx_resetdone_out_0_0,
    tx_resetdone_out_0_1,
    tx_resetdone_out_0_2,
    tx_resetdone_out_0_3,
    tx_unfout_0_0,
    tx_unfout_0_1,
    tx_unfout_0_2,
    tx_unfout_0_3,
    tx_usrclk_0,
    tx_usrclk_1,
    tx_usrclk_2,
    tx_usrclk_3,
    user_reg0_0_0,
    user_reg0_0_1,
    user_reg0_0_2,
    user_reg0_0_3);
  output FEC_stat_0_0_stat_fec_inc_cant_correct_count;
  output FEC_stat_0_0_stat_fec_inc_correct_count;
  output FEC_stat_0_0_stat_fec_lock_error;
  output FEC_stat_0_0_stat_fec_rx_lock;
  output FEC_stat_0_1_stat_fec_inc_cant_correct_count;
  output FEC_stat_0_1_stat_fec_inc_correct_count;
  output FEC_stat_0_1_stat_fec_lock_error;
  output FEC_stat_0_1_stat_fec_rx_lock;
  output FEC_stat_0_2_stat_fec_inc_cant_correct_count;
  output FEC_stat_0_2_stat_fec_inc_correct_count;
  output FEC_stat_0_2_stat_fec_lock_error;
  output FEC_stat_0_2_stat_fec_rx_lock;
  output FEC_stat_0_3_stat_fec_inc_cant_correct_count;
  output FEC_stat_0_3_stat_fec_inc_correct_count;
  output FEC_stat_0_3_stat_fec_lock_error;
  output FEC_stat_0_3_stat_fec_rx_lock;
  output RS_FEC_stat_0_0_stat_rx_rsfec_corrected_cw_inc;
  output [2:0]RS_FEC_stat_0_0_stat_rx_rsfec_err_count0_inc;
  output RS_FEC_stat_0_0_stat_rx_rsfec_hi_ser;
  output RS_FEC_stat_0_0_stat_rx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_0_stat_rx_rsfec_uncorrected_cw_inc;
  output RS_FEC_stat_0_0_stat_tx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_1_stat_rx_rsfec_corrected_cw_inc;
  output [2:0]RS_FEC_stat_0_1_stat_rx_rsfec_err_count0_inc;
  output RS_FEC_stat_0_1_stat_rx_rsfec_hi_ser;
  output RS_FEC_stat_0_1_stat_rx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_1_stat_rx_rsfec_uncorrected_cw_inc;
  output RS_FEC_stat_0_1_stat_tx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_2_stat_rx_rsfec_corrected_cw_inc;
  output [2:0]RS_FEC_stat_0_2_stat_rx_rsfec_err_count0_inc;
  output RS_FEC_stat_0_2_stat_rx_rsfec_hi_ser;
  output RS_FEC_stat_0_2_stat_rx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_2_stat_rx_rsfec_uncorrected_cw_inc;
  output RS_FEC_stat_0_2_stat_tx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_3_stat_rx_rsfec_corrected_cw_inc;
  output [2:0]RS_FEC_stat_0_3_stat_rx_rsfec_err_count0_inc;
  output RS_FEC_stat_0_3_stat_rx_rsfec_hi_ser;
  output RS_FEC_stat_0_3_stat_rx_rsfec_lane_alignment_status;
  output RS_FEC_stat_0_3_stat_rx_rsfec_uncorrected_cw_inc;
  output RS_FEC_stat_0_3_stat_tx_rsfec_lane_alignment_status;
  input apb3clk_quad;
  output [63:0]axis_rx_0_0_tdata;
  output [7:0]axis_rx_0_0_tkeep;
  output axis_rx_0_0_tlast;
  output axis_rx_0_0_tuser;
  output axis_rx_0_0_tvalid;
  output [63:0]axis_rx_0_1_tdata;
  output [7:0]axis_rx_0_1_tkeep;
  output axis_rx_0_1_tlast;
  output axis_rx_0_1_tuser;
  output axis_rx_0_1_tvalid;
  output [63:0]axis_rx_0_2_tdata;
  output [7:0]axis_rx_0_2_tkeep;
  output axis_rx_0_2_tlast;
  output axis_rx_0_2_tuser;
  output axis_rx_0_2_tvalid;
  output [63:0]axis_rx_0_3_tdata;
  output [7:0]axis_rx_0_3_tkeep;
  output axis_rx_0_3_tlast;
  output axis_rx_0_3_tuser;
  output axis_rx_0_3_tvalid;
  input [63:0]axis_tx_0_0_tdata;
  input [7:0]axis_tx_0_0_tkeep;
  input axis_tx_0_0_tlast;
  output axis_tx_0_0_tready;
  input axis_tx_0_0_tuser;
  input axis_tx_0_0_tvalid;
  input [63:0]axis_tx_0_1_tdata;
  input [7:0]axis_tx_0_1_tkeep;
  input axis_tx_0_1_tlast;
  output axis_tx_0_1_tready;
  input axis_tx_0_1_tuser;
  input axis_tx_0_1_tvalid;
  input [63:0]axis_tx_0_2_tdata;
  input [7:0]axis_tx_0_2_tkeep;
  input axis_tx_0_2_tlast;
  output axis_tx_0_2_tready;
  input axis_tx_0_2_tuser;
  input axis_tx_0_2_tvalid;
  input [63:0]axis_tx_0_3_tdata;
  input [7:0]axis_tx_0_3_tkeep;
  input axis_tx_0_3_tlast;
  output axis_tx_0_3_tready;
  input axis_tx_0_3_tuser;
  input axis_tx_0_3_tvalid;
  input ctl_tx_0_0_ctl_tx_send_idle;
  input ctl_tx_0_0_ctl_tx_send_lfi;
  input ctl_tx_0_0_ctl_tx_send_rfi;
  input ctl_tx_0_1_ctl_tx_send_idle;
  input ctl_tx_0_1_ctl_tx_send_lfi;
  input ctl_tx_0_1_ctl_tx_send_rfi;
  input ctl_tx_0_2_ctl_tx_send_idle;
  input ctl_tx_0_2_ctl_tx_send_lfi;
  input ctl_tx_0_2_ctl_tx_send_rfi;
  input ctl_tx_0_3_ctl_tx_send_idle;
  input ctl_tx_0_3_ctl_tx_send_lfi;
  input ctl_tx_0_3_ctl_tx_send_rfi;
  input [3:0]gt_rxn_in_0;
  input [3:0]gt_rxp_in_0;
  output [3:0]gt_txn_out_0;
  output [3:0]gt_txp_out_0;
  input gtwiz_reset_clk_freerun_in_0_0;
  input gtwiz_reset_clk_freerun_in_0_1;
  input gtwiz_reset_clk_freerun_in_0_2;
  input gtwiz_reset_clk_freerun_in_0_3;
  input pm_tick_0_0;
  input pm_tick_0_1;
  input pm_tick_0_2;
  input pm_tick_0_3;
  input [0:0]gt_refclk_clk_n;
  input [0:0]gt_refclk_clk_p;
  input rx_core_clk_0_0;
  input rx_core_clk_0_1;
  input rx_core_clk_0_2;
  input rx_core_clk_0_3;
  output [55:0]rx_preambleout_0_0;
  output [55:0]rx_preambleout_0_1;
  output [55:0]rx_preambleout_0_2;
  output [55:0]rx_preambleout_0_3;
  input rx_reset_0_0;
  input rx_reset_0_1;
  input rx_reset_0_2;
  input rx_reset_0_3;
  output rx_resetdone_out_0_0;
  output rx_resetdone_out_0_1;
  output rx_resetdone_out_0_2;
  output rx_resetdone_out_0_3;
  input rx_serdes_clk_0_0;
  input rx_serdes_clk_0_1;
  input rx_serdes_clk_0_2;
  input rx_serdes_clk_0_3;
  input rx_serdes_reset_0_0;
  input rx_serdes_reset_0_1;
  input rx_serdes_reset_0_2;
  input rx_serdes_reset_0_3;
  output rx_usrclk_0;
  output rx_usrclk_1;
  output rx_usrclk_2;
  output rx_usrclk_3;
  input [31:0]s_axi_0_0_araddr;
  output s_axi_0_0_arready;
  input s_axi_0_0_arvalid;
  input [31:0]s_axi_0_0_awaddr;
  output s_axi_0_0_awready;
  input s_axi_0_0_awvalid;
  input s_axi_0_0_bready;
  output [1:0]s_axi_0_0_bresp;
  output s_axi_0_0_bvalid;
  output [31:0]s_axi_0_0_rdata;
  input s_axi_0_0_rready;
  output [1:0]s_axi_0_0_rresp;
  output s_axi_0_0_rvalid;
  input [31:0]s_axi_0_0_wdata;
  output s_axi_0_0_wready;
  input [3:0]s_axi_0_0_wstrb;
  input s_axi_0_0_wvalid;
  input [31:0]s_axi_0_1_araddr;
  output s_axi_0_1_arready;
  input s_axi_0_1_arvalid;
  input [31:0]s_axi_0_1_awaddr;
  output s_axi_0_1_awready;
  input s_axi_0_1_awvalid;
  input s_axi_0_1_bready;
  output [1:0]s_axi_0_1_bresp;
  output s_axi_0_1_bvalid;
  output [31:0]s_axi_0_1_rdata;
  input s_axi_0_1_rready;
  output [1:0]s_axi_0_1_rresp;
  output s_axi_0_1_rvalid;
  input [31:0]s_axi_0_1_wdata;
  output s_axi_0_1_wready;
  input [3:0]s_axi_0_1_wstrb;
  input s_axi_0_1_wvalid;
  input [31:0]s_axi_0_2_araddr;
  output s_axi_0_2_arready;
  input s_axi_0_2_arvalid;
  input [31:0]s_axi_0_2_awaddr;
  output s_axi_0_2_awready;
  input s_axi_0_2_awvalid;
  input s_axi_0_2_bready;
  output [1:0]s_axi_0_2_bresp;
  output s_axi_0_2_bvalid;
  output [31:0]s_axi_0_2_rdata;
  input s_axi_0_2_rready;
  output [1:0]s_axi_0_2_rresp;
  output s_axi_0_2_rvalid;
  input [31:0]s_axi_0_2_wdata;
  output s_axi_0_2_wready;
  input [3:0]s_axi_0_2_wstrb;
  input s_axi_0_2_wvalid;
  input [31:0]s_axi_0_3_araddr;
  output s_axi_0_3_arready;
  input s_axi_0_3_arvalid;
  input [31:0]s_axi_0_3_awaddr;
  output s_axi_0_3_awready;
  input s_axi_0_3_awvalid;
  input s_axi_0_3_bready;
  output [1:0]s_axi_0_3_bresp;
  output s_axi_0_3_bvalid;
  output [31:0]s_axi_0_3_rdata;
  input s_axi_0_3_rready;
  output [1:0]s_axi_0_3_rresp;
  output s_axi_0_3_rvalid;
  input [31:0]s_axi_0_3_wdata;
  output s_axi_0_3_wready;
  input [3:0]s_axi_0_3_wstrb;
  input s_axi_0_3_wvalid;
  input s_axi_aclk_0_0;
  input s_axi_aclk_0_1;
  input s_axi_aclk_0_2;
  input s_axi_aclk_0_3;
  input s_axi_aresetn_0_0;
  input s_axi_aresetn_0_1;
  input s_axi_aresetn_0_2;
  input s_axi_aresetn_0_3;
  output stat_rx_0_0_stat_rx_bad_code;
  output [1:0]stat_rx_0_0_stat_rx_bad_fcs;
  output stat_rx_0_0_stat_rx_bad_preamble;
  output stat_rx_0_0_stat_rx_bad_sfd;
  output stat_rx_0_0_stat_rx_block_lock;
  output stat_rx_0_0_stat_rx_broadcast;
  output stat_rx_0_0_stat_rx_fragment;
  output stat_rx_0_0_stat_rx_framing_err;
  output stat_rx_0_0_stat_rx_framing_err_valid;
  output stat_rx_0_0_stat_rx_got_signal_os;
  output stat_rx_0_0_stat_rx_hi_ber;
  output stat_rx_0_0_stat_rx_inrangeerr;
  output stat_rx_0_0_stat_rx_internal_local_fault;
  output stat_rx_0_0_stat_rx_jabber;
  output stat_rx_0_0_stat_rx_local_fault;
  output stat_rx_0_0_stat_rx_multicast;
  output stat_rx_0_0_stat_rx_oversize;
  output stat_rx_0_0_stat_rx_packet_1024_1518_bytes;
  output stat_rx_0_0_stat_rx_packet_128_255_bytes;
  output stat_rx_0_0_stat_rx_packet_1519_1522_bytes;
  output stat_rx_0_0_stat_rx_packet_1523_1548_bytes;
  output stat_rx_0_0_stat_rx_packet_1549_2047_bytes;
  output stat_rx_0_0_stat_rx_packet_2048_4095_bytes;
  output stat_rx_0_0_stat_rx_packet_256_511_bytes;
  output stat_rx_0_0_stat_rx_packet_4096_8191_bytes;
  output stat_rx_0_0_stat_rx_packet_512_1023_bytes;
  output stat_rx_0_0_stat_rx_packet_64_bytes;
  output stat_rx_0_0_stat_rx_packet_65_127_bytes;
  output stat_rx_0_0_stat_rx_packet_8192_9215_bytes;
  output stat_rx_0_0_stat_rx_packet_bad_fcs;
  output stat_rx_0_0_stat_rx_packet_large;
  output stat_rx_0_0_stat_rx_packet_small;
  output stat_rx_0_0_stat_rx_received_local_fault;
  output stat_rx_0_0_stat_rx_remote_fault;
  output [1:0]stat_rx_0_0_stat_rx_stomped_fcs;
  output stat_rx_0_0_stat_rx_test_pattern_mismatch;
  output stat_rx_0_0_stat_rx_toolong;
  output [3:0]stat_rx_0_0_stat_rx_total_bytes;
  output [13:0]stat_rx_0_0_stat_rx_total_good_bytes;
  output stat_rx_0_0_stat_rx_total_good_packets;
  output [1:0]stat_rx_0_0_stat_rx_total_packets;
  output stat_rx_0_0_stat_rx_truncated;
  output stat_rx_0_0_stat_rx_undersize;
  output stat_rx_0_0_stat_rx_unicast;
  output stat_rx_0_0_stat_rx_valid_ctrl_code;
  output stat_rx_0_0_stat_rx_vlan;
  output stat_rx_0_1_stat_rx_bad_code;
  output [1:0]stat_rx_0_1_stat_rx_bad_fcs;
  output stat_rx_0_1_stat_rx_bad_preamble;
  output stat_rx_0_1_stat_rx_bad_sfd;
  output stat_rx_0_1_stat_rx_block_lock;
  output stat_rx_0_1_stat_rx_broadcast;
  output stat_rx_0_1_stat_rx_fragment;
  output stat_rx_0_1_stat_rx_framing_err;
  output stat_rx_0_1_stat_rx_framing_err_valid;
  output stat_rx_0_1_stat_rx_got_signal_os;
  output stat_rx_0_1_stat_rx_hi_ber;
  output stat_rx_0_1_stat_rx_inrangeerr;
  output stat_rx_0_1_stat_rx_internal_local_fault;
  output stat_rx_0_1_stat_rx_jabber;
  output stat_rx_0_1_stat_rx_local_fault;
  output stat_rx_0_1_stat_rx_multicast;
  output stat_rx_0_1_stat_rx_oversize;
  output stat_rx_0_1_stat_rx_packet_1024_1518_bytes;
  output stat_rx_0_1_stat_rx_packet_128_255_bytes;
  output stat_rx_0_1_stat_rx_packet_1519_1522_bytes;
  output stat_rx_0_1_stat_rx_packet_1523_1548_bytes;
  output stat_rx_0_1_stat_rx_packet_1549_2047_bytes;
  output stat_rx_0_1_stat_rx_packet_2048_4095_bytes;
  output stat_rx_0_1_stat_rx_packet_256_511_bytes;
  output stat_rx_0_1_stat_rx_packet_4096_8191_bytes;
  output stat_rx_0_1_stat_rx_packet_512_1023_bytes;
  output stat_rx_0_1_stat_rx_packet_64_bytes;
  output stat_rx_0_1_stat_rx_packet_65_127_bytes;
  output stat_rx_0_1_stat_rx_packet_8192_9215_bytes;
  output stat_rx_0_1_stat_rx_packet_bad_fcs;
  output stat_rx_0_1_stat_rx_packet_large;
  output stat_rx_0_1_stat_rx_packet_small;
  output stat_rx_0_1_stat_rx_received_local_fault;
  output stat_rx_0_1_stat_rx_remote_fault;
  output [1:0]stat_rx_0_1_stat_rx_stomped_fcs;
  output stat_rx_0_1_stat_rx_test_pattern_mismatch;
  output stat_rx_0_1_stat_rx_toolong;
  output [3:0]stat_rx_0_1_stat_rx_total_bytes;
  output [13:0]stat_rx_0_1_stat_rx_total_good_bytes;
  output stat_rx_0_1_stat_rx_total_good_packets;
  output [1:0]stat_rx_0_1_stat_rx_total_packets;
  output stat_rx_0_1_stat_rx_truncated;
  output stat_rx_0_1_stat_rx_undersize;
  output stat_rx_0_1_stat_rx_unicast;
  output stat_rx_0_1_stat_rx_valid_ctrl_code;
  output stat_rx_0_1_stat_rx_vlan;
  output stat_rx_0_2_stat_rx_bad_code;
  output [1:0]stat_rx_0_2_stat_rx_bad_fcs;
  output stat_rx_0_2_stat_rx_bad_preamble;
  output stat_rx_0_2_stat_rx_bad_sfd;
  output stat_rx_0_2_stat_rx_block_lock;
  output stat_rx_0_2_stat_rx_broadcast;
  output stat_rx_0_2_stat_rx_fragment;
  output stat_rx_0_2_stat_rx_framing_err;
  output stat_rx_0_2_stat_rx_framing_err_valid;
  output stat_rx_0_2_stat_rx_got_signal_os;
  output stat_rx_0_2_stat_rx_hi_ber;
  output stat_rx_0_2_stat_rx_inrangeerr;
  output stat_rx_0_2_stat_rx_internal_local_fault;
  output stat_rx_0_2_stat_rx_jabber;
  output stat_rx_0_2_stat_rx_local_fault;
  output stat_rx_0_2_stat_rx_multicast;
  output stat_rx_0_2_stat_rx_oversize;
  output stat_rx_0_2_stat_rx_packet_1024_1518_bytes;
  output stat_rx_0_2_stat_rx_packet_128_255_bytes;
  output stat_rx_0_2_stat_rx_packet_1519_1522_bytes;
  output stat_rx_0_2_stat_rx_packet_1523_1548_bytes;
  output stat_rx_0_2_stat_rx_packet_1549_2047_bytes;
  output stat_rx_0_2_stat_rx_packet_2048_4095_bytes;
  output stat_rx_0_2_stat_rx_packet_256_511_bytes;
  output stat_rx_0_2_stat_rx_packet_4096_8191_bytes;
  output stat_rx_0_2_stat_rx_packet_512_1023_bytes;
  output stat_rx_0_2_stat_rx_packet_64_bytes;
  output stat_rx_0_2_stat_rx_packet_65_127_bytes;
  output stat_rx_0_2_stat_rx_packet_8192_9215_bytes;
  output stat_rx_0_2_stat_rx_packet_bad_fcs;
  output stat_rx_0_2_stat_rx_packet_large;
  output stat_rx_0_2_stat_rx_packet_small;
  output stat_rx_0_2_stat_rx_received_local_fault;
  output stat_rx_0_2_stat_rx_remote_fault;
  output [1:0]stat_rx_0_2_stat_rx_stomped_fcs;
  output stat_rx_0_2_stat_rx_test_pattern_mismatch;
  output stat_rx_0_2_stat_rx_toolong;
  output [3:0]stat_rx_0_2_stat_rx_total_bytes;
  output [13:0]stat_rx_0_2_stat_rx_total_good_bytes;
  output stat_rx_0_2_stat_rx_total_good_packets;
  output [1:0]stat_rx_0_2_stat_rx_total_packets;
  output stat_rx_0_2_stat_rx_truncated;
  output stat_rx_0_2_stat_rx_undersize;
  output stat_rx_0_2_stat_rx_unicast;
  output stat_rx_0_2_stat_rx_valid_ctrl_code;
  output stat_rx_0_2_stat_rx_vlan;
  output stat_rx_0_3_stat_rx_bad_code;
  output [1:0]stat_rx_0_3_stat_rx_bad_fcs;
  output stat_rx_0_3_stat_rx_bad_preamble;
  output stat_rx_0_3_stat_rx_bad_sfd;
  output stat_rx_0_3_stat_rx_block_lock;
  output stat_rx_0_3_stat_rx_broadcast;
  output stat_rx_0_3_stat_rx_fragment;
  output stat_rx_0_3_stat_rx_framing_err;
  output stat_rx_0_3_stat_rx_framing_err_valid;
  output stat_rx_0_3_stat_rx_got_signal_os;
  output stat_rx_0_3_stat_rx_hi_ber;
  output stat_rx_0_3_stat_rx_inrangeerr;
  output stat_rx_0_3_stat_rx_internal_local_fault;
  output stat_rx_0_3_stat_rx_jabber;
  output stat_rx_0_3_stat_rx_local_fault;
  output stat_rx_0_3_stat_rx_multicast;
  output stat_rx_0_3_stat_rx_oversize;
  output stat_rx_0_3_stat_rx_packet_1024_1518_bytes;
  output stat_rx_0_3_stat_rx_packet_128_255_bytes;
  output stat_rx_0_3_stat_rx_packet_1519_1522_bytes;
  output stat_rx_0_3_stat_rx_packet_1523_1548_bytes;
  output stat_rx_0_3_stat_rx_packet_1549_2047_bytes;
  output stat_rx_0_3_stat_rx_packet_2048_4095_bytes;
  output stat_rx_0_3_stat_rx_packet_256_511_bytes;
  output stat_rx_0_3_stat_rx_packet_4096_8191_bytes;
  output stat_rx_0_3_stat_rx_packet_512_1023_bytes;
  output stat_rx_0_3_stat_rx_packet_64_bytes;
  output stat_rx_0_3_stat_rx_packet_65_127_bytes;
  output stat_rx_0_3_stat_rx_packet_8192_9215_bytes;
  output stat_rx_0_3_stat_rx_packet_bad_fcs;
  output stat_rx_0_3_stat_rx_packet_large;
  output stat_rx_0_3_stat_rx_packet_small;
  output stat_rx_0_3_stat_rx_received_local_fault;
  output stat_rx_0_3_stat_rx_remote_fault;
  output [1:0]stat_rx_0_3_stat_rx_stomped_fcs;
  output stat_rx_0_3_stat_rx_test_pattern_mismatch;
  output stat_rx_0_3_stat_rx_toolong;
  output [3:0]stat_rx_0_3_stat_rx_total_bytes;
  output [13:0]stat_rx_0_3_stat_rx_total_good_bytes;
  output stat_rx_0_3_stat_rx_total_good_packets;
  output [1:0]stat_rx_0_3_stat_rx_total_packets;
  output stat_rx_0_3_stat_rx_truncated;
  output stat_rx_0_3_stat_rx_undersize;
  output stat_rx_0_3_stat_rx_unicast;
  output stat_rx_0_3_stat_rx_valid_ctrl_code;
  output stat_rx_0_3_stat_rx_vlan;
  output stat_rx_status_0_0;
  output stat_rx_status_0_1;
  output stat_rx_status_0_2;
  output stat_rx_status_0_3;
  output stat_tx_0_0_stat_tx_bad_fcs;
  output stat_tx_0_0_stat_tx_broadcast;
  output stat_tx_0_0_stat_tx_frame_error;
  output stat_tx_0_0_stat_tx_local_fault;
  output stat_tx_0_0_stat_tx_multicast;
  output stat_tx_0_0_stat_tx_packet_1024_1518_bytes;
  output stat_tx_0_0_stat_tx_packet_128_255_bytes;
  output stat_tx_0_0_stat_tx_packet_1519_1522_bytes;
  output stat_tx_0_0_stat_tx_packet_1523_1548_bytes;
  output stat_tx_0_0_stat_tx_packet_1549_2047_bytes;
  output stat_tx_0_0_stat_tx_packet_2048_4095_bytes;
  output stat_tx_0_0_stat_tx_packet_256_511_bytes;
  output stat_tx_0_0_stat_tx_packet_4096_8191_bytes;
  output stat_tx_0_0_stat_tx_packet_512_1023_bytes;
  output stat_tx_0_0_stat_tx_packet_64_bytes;
  output stat_tx_0_0_stat_tx_packet_65_127_bytes;
  output stat_tx_0_0_stat_tx_packet_8192_9215_bytes;
  output stat_tx_0_0_stat_tx_packet_large;
  output stat_tx_0_0_stat_tx_packet_small;
  output [3:0]stat_tx_0_0_stat_tx_total_bytes;
  output [13:0]stat_tx_0_0_stat_tx_total_good_bytes;
  output stat_tx_0_0_stat_tx_total_good_packets;
  output stat_tx_0_0_stat_tx_total_packets;
  output stat_tx_0_0_stat_tx_unicast;
  output stat_tx_0_0_stat_tx_vlan;
  output stat_tx_0_1_stat_tx_bad_fcs;
  output stat_tx_0_1_stat_tx_broadcast;
  output stat_tx_0_1_stat_tx_frame_error;
  output stat_tx_0_1_stat_tx_local_fault;
  output stat_tx_0_1_stat_tx_multicast;
  output stat_tx_0_1_stat_tx_packet_1024_1518_bytes;
  output stat_tx_0_1_stat_tx_packet_128_255_bytes;
  output stat_tx_0_1_stat_tx_packet_1519_1522_bytes;
  output stat_tx_0_1_stat_tx_packet_1523_1548_bytes;
  output stat_tx_0_1_stat_tx_packet_1549_2047_bytes;
  output stat_tx_0_1_stat_tx_packet_2048_4095_bytes;
  output stat_tx_0_1_stat_tx_packet_256_511_bytes;
  output stat_tx_0_1_stat_tx_packet_4096_8191_bytes;
  output stat_tx_0_1_stat_tx_packet_512_1023_bytes;
  output stat_tx_0_1_stat_tx_packet_64_bytes;
  output stat_tx_0_1_stat_tx_packet_65_127_bytes;
  output stat_tx_0_1_stat_tx_packet_8192_9215_bytes;
  output stat_tx_0_1_stat_tx_packet_large;
  output stat_tx_0_1_stat_tx_packet_small;
  output [3:0]stat_tx_0_1_stat_tx_total_bytes;
  output [13:0]stat_tx_0_1_stat_tx_total_good_bytes;
  output stat_tx_0_1_stat_tx_total_good_packets;
  output stat_tx_0_1_stat_tx_total_packets;
  output stat_tx_0_1_stat_tx_unicast;
  output stat_tx_0_1_stat_tx_vlan;
  output stat_tx_0_2_stat_tx_bad_fcs;
  output stat_tx_0_2_stat_tx_broadcast;
  output stat_tx_0_2_stat_tx_frame_error;
  output stat_tx_0_2_stat_tx_local_fault;
  output stat_tx_0_2_stat_tx_multicast;
  output stat_tx_0_2_stat_tx_packet_1024_1518_bytes;
  output stat_tx_0_2_stat_tx_packet_128_255_bytes;
  output stat_tx_0_2_stat_tx_packet_1519_1522_bytes;
  output stat_tx_0_2_stat_tx_packet_1523_1548_bytes;
  output stat_tx_0_2_stat_tx_packet_1549_2047_bytes;
  output stat_tx_0_2_stat_tx_packet_2048_4095_bytes;
  output stat_tx_0_2_stat_tx_packet_256_511_bytes;
  output stat_tx_0_2_stat_tx_packet_4096_8191_bytes;
  output stat_tx_0_2_stat_tx_packet_512_1023_bytes;
  output stat_tx_0_2_stat_tx_packet_64_bytes;
  output stat_tx_0_2_stat_tx_packet_65_127_bytes;
  output stat_tx_0_2_stat_tx_packet_8192_9215_bytes;
  output stat_tx_0_2_stat_tx_packet_large;
  output stat_tx_0_2_stat_tx_packet_small;
  output [3:0]stat_tx_0_2_stat_tx_total_bytes;
  output [13:0]stat_tx_0_2_stat_tx_total_good_bytes;
  output stat_tx_0_2_stat_tx_total_good_packets;
  output stat_tx_0_2_stat_tx_total_packets;
  output stat_tx_0_2_stat_tx_unicast;
  output stat_tx_0_2_stat_tx_vlan;
  output stat_tx_0_3_stat_tx_bad_fcs;
  output stat_tx_0_3_stat_tx_broadcast;
  output stat_tx_0_3_stat_tx_frame_error;
  output stat_tx_0_3_stat_tx_local_fault;
  output stat_tx_0_3_stat_tx_multicast;
  output stat_tx_0_3_stat_tx_packet_1024_1518_bytes;
  output stat_tx_0_3_stat_tx_packet_128_255_bytes;
  output stat_tx_0_3_stat_tx_packet_1519_1522_bytes;
  output stat_tx_0_3_stat_tx_packet_1523_1548_bytes;
  output stat_tx_0_3_stat_tx_packet_1549_2047_bytes;
  output stat_tx_0_3_stat_tx_packet_2048_4095_bytes;
  output stat_tx_0_3_stat_tx_packet_256_511_bytes;
  output stat_tx_0_3_stat_tx_packet_4096_8191_bytes;
  output stat_tx_0_3_stat_tx_packet_512_1023_bytes;
  output stat_tx_0_3_stat_tx_packet_64_bytes;
  output stat_tx_0_3_stat_tx_packet_65_127_bytes;
  output stat_tx_0_3_stat_tx_packet_8192_9215_bytes;
  output stat_tx_0_3_stat_tx_packet_large;
  output stat_tx_0_3_stat_tx_packet_small;
  output [3:0]stat_tx_0_3_stat_tx_total_bytes;
  output [13:0]stat_tx_0_3_stat_tx_total_good_bytes;
  output stat_tx_0_3_stat_tx_total_good_packets;
  output stat_tx_0_3_stat_tx_total_packets;
  output stat_tx_0_3_stat_tx_unicast;
  output stat_tx_0_3_stat_tx_vlan;
  input tx_core_clk_0_0;
  input tx_core_clk_0_1;
  input tx_core_clk_0_2;
  input tx_core_clk_0_3;
  input [55:0]tx_preamblein_0_0;
  input [55:0]tx_preamblein_0_1;
  input [55:0]tx_preamblein_0_2;
  input [55:0]tx_preamblein_0_3;
  input tx_reset_0_0;
  input tx_reset_0_1;
  input tx_reset_0_2;
  input tx_reset_0_3;
  output tx_resetdone_out_0_0;
  output tx_resetdone_out_0_1;
  output tx_resetdone_out_0_2;
  output tx_resetdone_out_0_3;
  output tx_unfout_0_0;
  output tx_unfout_0_1;
  output tx_unfout_0_2;
  output tx_unfout_0_3;
  output tx_usrclk_0;
  output tx_usrclk_1;
  output tx_usrclk_2;
  output tx_usrclk_3;
  output [31:0]user_reg0_0_0;
  output [31:0]user_reg0_0_1;
  output [31:0]user_reg0_0_2;
  output [31:0]user_reg0_0_3;

  wire FEC_stat_0_0_stat_fec_inc_cant_correct_count;
  wire FEC_stat_0_0_stat_fec_inc_correct_count;
  wire FEC_stat_0_0_stat_fec_lock_error;
  wire FEC_stat_0_0_stat_fec_rx_lock;
  wire FEC_stat_0_1_stat_fec_inc_cant_correct_count;
  wire FEC_stat_0_1_stat_fec_inc_correct_count;
  wire FEC_stat_0_1_stat_fec_lock_error;
  wire FEC_stat_0_1_stat_fec_rx_lock;
  wire FEC_stat_0_2_stat_fec_inc_cant_correct_count;
  wire FEC_stat_0_2_stat_fec_inc_correct_count;
  wire FEC_stat_0_2_stat_fec_lock_error;
  wire FEC_stat_0_2_stat_fec_rx_lock;
  wire FEC_stat_0_3_stat_fec_inc_cant_correct_count;
  wire FEC_stat_0_3_stat_fec_inc_correct_count;
  wire FEC_stat_0_3_stat_fec_lock_error;
  wire FEC_stat_0_3_stat_fec_rx_lock;
  wire RS_FEC_stat_0_0_stat_rx_rsfec_corrected_cw_inc;
  wire [2:0]RS_FEC_stat_0_0_stat_rx_rsfec_err_count0_inc;
  wire RS_FEC_stat_0_0_stat_rx_rsfec_hi_ser;
  wire RS_FEC_stat_0_0_stat_rx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_0_stat_rx_rsfec_uncorrected_cw_inc;
  wire RS_FEC_stat_0_0_stat_tx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_1_stat_rx_rsfec_corrected_cw_inc;
  wire [2:0]RS_FEC_stat_0_1_stat_rx_rsfec_err_count0_inc;
  wire RS_FEC_stat_0_1_stat_rx_rsfec_hi_ser;
  wire RS_FEC_stat_0_1_stat_rx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_1_stat_rx_rsfec_uncorrected_cw_inc;
  wire RS_FEC_stat_0_1_stat_tx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_2_stat_rx_rsfec_corrected_cw_inc;
  wire [2:0]RS_FEC_stat_0_2_stat_rx_rsfec_err_count0_inc;
  wire RS_FEC_stat_0_2_stat_rx_rsfec_hi_ser;
  wire RS_FEC_stat_0_2_stat_rx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_2_stat_rx_rsfec_uncorrected_cw_inc;
  wire RS_FEC_stat_0_2_stat_tx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_3_stat_rx_rsfec_corrected_cw_inc;
  wire [2:0]RS_FEC_stat_0_3_stat_rx_rsfec_err_count0_inc;
  wire RS_FEC_stat_0_3_stat_rx_rsfec_hi_ser;
  wire RS_FEC_stat_0_3_stat_rx_rsfec_lane_alignment_status;
  wire RS_FEC_stat_0_3_stat_rx_rsfec_uncorrected_cw_inc;
  wire RS_FEC_stat_0_3_stat_tx_rsfec_lane_alignment_status;
  wire apb3clk_quad;
  wire [63:0]axis_rx_0_0_tdata;
  wire [7:0]axis_rx_0_0_tkeep;
  wire axis_rx_0_0_tlast;
  wire axis_rx_0_0_tuser;
  wire axis_rx_0_0_tvalid;
  wire [63:0]axis_rx_0_1_tdata;
  wire [7:0]axis_rx_0_1_tkeep;
  wire axis_rx_0_1_tlast;
  wire axis_rx_0_1_tuser;
  wire axis_rx_0_1_tvalid;
  wire [63:0]axis_rx_0_2_tdata;
  wire [7:0]axis_rx_0_2_tkeep;
  wire axis_rx_0_2_tlast;
  wire axis_rx_0_2_tuser;
  wire axis_rx_0_2_tvalid;
  wire [63:0]axis_rx_0_3_tdata;
  wire [7:0]axis_rx_0_3_tkeep;
  wire axis_rx_0_3_tlast;
  wire axis_rx_0_3_tuser;
  wire axis_rx_0_3_tvalid;
  wire [63:0]axis_tx_0_0_tdata;
  wire [7:0]axis_tx_0_0_tkeep;
  wire axis_tx_0_0_tlast;
  wire axis_tx_0_0_tready;
  wire axis_tx_0_0_tuser;
  wire axis_tx_0_0_tvalid;
  wire [63:0]axis_tx_0_1_tdata;
  wire [7:0]axis_tx_0_1_tkeep;
  wire axis_tx_0_1_tlast;
  wire axis_tx_0_1_tready;
  wire axis_tx_0_1_tuser;
  wire axis_tx_0_1_tvalid;
  wire [63:0]axis_tx_0_2_tdata;
  wire [7:0]axis_tx_0_2_tkeep;
  wire axis_tx_0_2_tlast;
  wire axis_tx_0_2_tready;
  wire axis_tx_0_2_tuser;
  wire axis_tx_0_2_tvalid;
  wire [63:0]axis_tx_0_3_tdata;
  wire [7:0]axis_tx_0_3_tkeep;
  wire axis_tx_0_3_tlast;
  wire axis_tx_0_3_tready;
  wire axis_tx_0_3_tuser;
  wire axis_tx_0_3_tvalid;
  wire ctl_tx_0_0_ctl_tx_send_idle;
  wire ctl_tx_0_0_ctl_tx_send_lfi;
  wire ctl_tx_0_0_ctl_tx_send_rfi;
  wire ctl_tx_0_1_ctl_tx_send_idle;
  wire ctl_tx_0_1_ctl_tx_send_lfi;
  wire ctl_tx_0_1_ctl_tx_send_rfi;
  wire ctl_tx_0_2_ctl_tx_send_idle;
  wire ctl_tx_0_2_ctl_tx_send_lfi;
  wire ctl_tx_0_2_ctl_tx_send_rfi;
  wire ctl_tx_0_3_ctl_tx_send_idle;
  wire ctl_tx_0_3_ctl_tx_send_lfi;
  wire ctl_tx_0_3_ctl_tx_send_rfi;
  wire [3:0]gt_rxn_in_0;
  wire [3:0]gt_rxp_in_0;
  wire [3:0]gt_txn_out_0;
  wire [3:0]gt_txp_out_0;
  wire gtwiz_reset_clk_freerun_in_0_0;
  wire gtwiz_reset_clk_freerun_in_0_1;
  wire gtwiz_reset_clk_freerun_in_0_2;
  wire gtwiz_reset_clk_freerun_in_0_3;
  wire pm_tick_0_0;
  wire pm_tick_0_1;
  wire pm_tick_0_2;
  wire pm_tick_0_3;
  wire [0:0]gt_refclk_clk_n;
  wire [0:0]gt_refclk_clk_p;
  wire rx_core_clk_0_0;
  wire rx_core_clk_0_1;
  wire rx_core_clk_0_2;
  wire rx_core_clk_0_3;
  wire [55:0]rx_preambleout_0_0;
  wire [55:0]rx_preambleout_0_1;
  wire [55:0]rx_preambleout_0_2;
  wire [55:0]rx_preambleout_0_3;
  wire rx_reset_0_0;
  wire rx_reset_0_1;
  wire rx_reset_0_2;
  wire rx_reset_0_3;
  wire rx_resetdone_out_0_0;
  wire rx_resetdone_out_0_1;
  wire rx_resetdone_out_0_2;
  wire rx_resetdone_out_0_3;
  wire rx_serdes_clk_0_0;
  wire rx_serdes_clk_0_1;
  wire rx_serdes_clk_0_2;
  wire rx_serdes_clk_0_3;
  wire rx_serdes_reset_0_0;
  wire rx_serdes_reset_0_1;
  wire rx_serdes_reset_0_2;
  wire rx_serdes_reset_0_3;
  wire rx_usrclk_0;
  wire rx_usrclk_1;
  wire rx_usrclk_2;
  wire rx_usrclk_3;
  wire [31:0]s_axi_0_0_araddr;
  wire s_axi_0_0_arready;
  wire s_axi_0_0_arvalid;
  wire [31:0]s_axi_0_0_awaddr;
  wire s_axi_0_0_awready;
  wire s_axi_0_0_awvalid;
  wire s_axi_0_0_bready;
  wire [1:0]s_axi_0_0_bresp;
  wire s_axi_0_0_bvalid;
  wire [31:0]s_axi_0_0_rdata;
  wire s_axi_0_0_rready;
  wire [1:0]s_axi_0_0_rresp;
  wire s_axi_0_0_rvalid;
  wire [31:0]s_axi_0_0_wdata;
  wire s_axi_0_0_wready;
  wire [3:0]s_axi_0_0_wstrb;
  wire s_axi_0_0_wvalid;
  wire [31:0]s_axi_0_1_araddr;
  wire s_axi_0_1_arready;
  wire s_axi_0_1_arvalid;
  wire [31:0]s_axi_0_1_awaddr;
  wire s_axi_0_1_awready;
  wire s_axi_0_1_awvalid;
  wire s_axi_0_1_bready;
  wire [1:0]s_axi_0_1_bresp;
  wire s_axi_0_1_bvalid;
  wire [31:0]s_axi_0_1_rdata;
  wire s_axi_0_1_rready;
  wire [1:0]s_axi_0_1_rresp;
  wire s_axi_0_1_rvalid;
  wire [31:0]s_axi_0_1_wdata;
  wire s_axi_0_1_wready;
  wire [3:0]s_axi_0_1_wstrb;
  wire s_axi_0_1_wvalid;
  wire [31:0]s_axi_0_2_araddr;
  wire s_axi_0_2_arready;
  wire s_axi_0_2_arvalid;
  wire [31:0]s_axi_0_2_awaddr;
  wire s_axi_0_2_awready;
  wire s_axi_0_2_awvalid;
  wire s_axi_0_2_bready;
  wire [1:0]s_axi_0_2_bresp;
  wire s_axi_0_2_bvalid;
  wire [31:0]s_axi_0_2_rdata;
  wire s_axi_0_2_rready;
  wire [1:0]s_axi_0_2_rresp;
  wire s_axi_0_2_rvalid;
  wire [31:0]s_axi_0_2_wdata;
  wire s_axi_0_2_wready;
  wire [3:0]s_axi_0_2_wstrb;
  wire s_axi_0_2_wvalid;
  wire [31:0]s_axi_0_3_araddr;
  wire s_axi_0_3_arready;
  wire s_axi_0_3_arvalid;
  wire [31:0]s_axi_0_3_awaddr;
  wire s_axi_0_3_awready;
  wire s_axi_0_3_awvalid;
  wire s_axi_0_3_bready;
  wire [1:0]s_axi_0_3_bresp;
  wire s_axi_0_3_bvalid;
  wire [31:0]s_axi_0_3_rdata;
  wire s_axi_0_3_rready;
  wire [1:0]s_axi_0_3_rresp;
  wire s_axi_0_3_rvalid;
  wire [31:0]s_axi_0_3_wdata;
  wire s_axi_0_3_wready;
  wire [3:0]s_axi_0_3_wstrb;
  wire s_axi_0_3_wvalid;
  wire s_axi_aclk_0_0;
  wire s_axi_aclk_0_1;
  wire s_axi_aclk_0_2;
  wire s_axi_aclk_0_3;
  wire s_axi_aresetn_0_0;
  wire s_axi_aresetn_0_1;
  wire s_axi_aresetn_0_2;
  wire s_axi_aresetn_0_3;
  wire stat_rx_0_0_stat_rx_bad_code;
  wire [1:0]stat_rx_0_0_stat_rx_bad_fcs;
  wire stat_rx_0_0_stat_rx_bad_preamble;
  wire stat_rx_0_0_stat_rx_bad_sfd;
  wire stat_rx_0_0_stat_rx_block_lock;
  wire stat_rx_0_0_stat_rx_broadcast;
  wire stat_rx_0_0_stat_rx_fragment;
  wire stat_rx_0_0_stat_rx_framing_err;
  wire stat_rx_0_0_stat_rx_framing_err_valid;
  wire stat_rx_0_0_stat_rx_got_signal_os;
  wire stat_rx_0_0_stat_rx_hi_ber;
  wire stat_rx_0_0_stat_rx_inrangeerr;
  wire stat_rx_0_0_stat_rx_internal_local_fault;
  wire stat_rx_0_0_stat_rx_jabber;
  wire stat_rx_0_0_stat_rx_local_fault;
  wire stat_rx_0_0_stat_rx_multicast;
  wire stat_rx_0_0_stat_rx_oversize;
  wire stat_rx_0_0_stat_rx_packet_1024_1518_bytes;
  wire stat_rx_0_0_stat_rx_packet_128_255_bytes;
  wire stat_rx_0_0_stat_rx_packet_1519_1522_bytes;
  wire stat_rx_0_0_stat_rx_packet_1523_1548_bytes;
  wire stat_rx_0_0_stat_rx_packet_1549_2047_bytes;
  wire stat_rx_0_0_stat_rx_packet_2048_4095_bytes;
  wire stat_rx_0_0_stat_rx_packet_256_511_bytes;
  wire stat_rx_0_0_stat_rx_packet_4096_8191_bytes;
  wire stat_rx_0_0_stat_rx_packet_512_1023_bytes;
  wire stat_rx_0_0_stat_rx_packet_64_bytes;
  wire stat_rx_0_0_stat_rx_packet_65_127_bytes;
  wire stat_rx_0_0_stat_rx_packet_8192_9215_bytes;
  wire stat_rx_0_0_stat_rx_packet_bad_fcs;
  wire stat_rx_0_0_stat_rx_packet_large;
  wire stat_rx_0_0_stat_rx_packet_small;
  wire stat_rx_0_0_stat_rx_received_local_fault;
  wire stat_rx_0_0_stat_rx_remote_fault;
  wire [1:0]stat_rx_0_0_stat_rx_stomped_fcs;
  wire stat_rx_0_0_stat_rx_test_pattern_mismatch;
  wire stat_rx_0_0_stat_rx_toolong;
  wire [3:0]stat_rx_0_0_stat_rx_total_bytes;
  wire [13:0]stat_rx_0_0_stat_rx_total_good_bytes;
  wire stat_rx_0_0_stat_rx_total_good_packets;
  wire [1:0]stat_rx_0_0_stat_rx_total_packets;
  wire stat_rx_0_0_stat_rx_truncated;
  wire stat_rx_0_0_stat_rx_undersize;
  wire stat_rx_0_0_stat_rx_unicast;
  wire stat_rx_0_0_stat_rx_valid_ctrl_code;
  wire stat_rx_0_0_stat_rx_vlan;
  wire stat_rx_0_1_stat_rx_bad_code;
  wire [1:0]stat_rx_0_1_stat_rx_bad_fcs;
  wire stat_rx_0_1_stat_rx_bad_preamble;
  wire stat_rx_0_1_stat_rx_bad_sfd;
  wire stat_rx_0_1_stat_rx_block_lock;
  wire stat_rx_0_1_stat_rx_broadcast;
  wire stat_rx_0_1_stat_rx_fragment;
  wire stat_rx_0_1_stat_rx_framing_err;
  wire stat_rx_0_1_stat_rx_framing_err_valid;
  wire stat_rx_0_1_stat_rx_got_signal_os;
  wire stat_rx_0_1_stat_rx_hi_ber;
  wire stat_rx_0_1_stat_rx_inrangeerr;
  wire stat_rx_0_1_stat_rx_internal_local_fault;
  wire stat_rx_0_1_stat_rx_jabber;
  wire stat_rx_0_1_stat_rx_local_fault;
  wire stat_rx_0_1_stat_rx_multicast;
  wire stat_rx_0_1_stat_rx_oversize;
  wire stat_rx_0_1_stat_rx_packet_1024_1518_bytes;
  wire stat_rx_0_1_stat_rx_packet_128_255_bytes;
  wire stat_rx_0_1_stat_rx_packet_1519_1522_bytes;
  wire stat_rx_0_1_stat_rx_packet_1523_1548_bytes;
  wire stat_rx_0_1_stat_rx_packet_1549_2047_bytes;
  wire stat_rx_0_1_stat_rx_packet_2048_4095_bytes;
  wire stat_rx_0_1_stat_rx_packet_256_511_bytes;
  wire stat_rx_0_1_stat_rx_packet_4096_8191_bytes;
  wire stat_rx_0_1_stat_rx_packet_512_1023_bytes;
  wire stat_rx_0_1_stat_rx_packet_64_bytes;
  wire stat_rx_0_1_stat_rx_packet_65_127_bytes;
  wire stat_rx_0_1_stat_rx_packet_8192_9215_bytes;
  wire stat_rx_0_1_stat_rx_packet_bad_fcs;
  wire stat_rx_0_1_stat_rx_packet_large;
  wire stat_rx_0_1_stat_rx_packet_small;
  wire stat_rx_0_1_stat_rx_received_local_fault;
  wire stat_rx_0_1_stat_rx_remote_fault;
  wire [1:0]stat_rx_0_1_stat_rx_stomped_fcs;
  wire stat_rx_0_1_stat_rx_test_pattern_mismatch;
  wire stat_rx_0_1_stat_rx_toolong;
  wire [3:0]stat_rx_0_1_stat_rx_total_bytes;
  wire [13:0]stat_rx_0_1_stat_rx_total_good_bytes;
  wire stat_rx_0_1_stat_rx_total_good_packets;
  wire [1:0]stat_rx_0_1_stat_rx_total_packets;
  wire stat_rx_0_1_stat_rx_truncated;
  wire stat_rx_0_1_stat_rx_undersize;
  wire stat_rx_0_1_stat_rx_unicast;
  wire stat_rx_0_1_stat_rx_valid_ctrl_code;
  wire stat_rx_0_1_stat_rx_vlan;
  wire stat_rx_0_2_stat_rx_bad_code;
  wire [1:0]stat_rx_0_2_stat_rx_bad_fcs;
  wire stat_rx_0_2_stat_rx_bad_preamble;
  wire stat_rx_0_2_stat_rx_bad_sfd;
  wire stat_rx_0_2_stat_rx_block_lock;
  wire stat_rx_0_2_stat_rx_broadcast;
  wire stat_rx_0_2_stat_rx_fragment;
  wire stat_rx_0_2_stat_rx_framing_err;
  wire stat_rx_0_2_stat_rx_framing_err_valid;
  wire stat_rx_0_2_stat_rx_got_signal_os;
  wire stat_rx_0_2_stat_rx_hi_ber;
  wire stat_rx_0_2_stat_rx_inrangeerr;
  wire stat_rx_0_2_stat_rx_internal_local_fault;
  wire stat_rx_0_2_stat_rx_jabber;
  wire stat_rx_0_2_stat_rx_local_fault;
  wire stat_rx_0_2_stat_rx_multicast;
  wire stat_rx_0_2_stat_rx_oversize;
  wire stat_rx_0_2_stat_rx_packet_1024_1518_bytes;
  wire stat_rx_0_2_stat_rx_packet_128_255_bytes;
  wire stat_rx_0_2_stat_rx_packet_1519_1522_bytes;
  wire stat_rx_0_2_stat_rx_packet_1523_1548_bytes;
  wire stat_rx_0_2_stat_rx_packet_1549_2047_bytes;
  wire stat_rx_0_2_stat_rx_packet_2048_4095_bytes;
  wire stat_rx_0_2_stat_rx_packet_256_511_bytes;
  wire stat_rx_0_2_stat_rx_packet_4096_8191_bytes;
  wire stat_rx_0_2_stat_rx_packet_512_1023_bytes;
  wire stat_rx_0_2_stat_rx_packet_64_bytes;
  wire stat_rx_0_2_stat_rx_packet_65_127_bytes;
  wire stat_rx_0_2_stat_rx_packet_8192_9215_bytes;
  wire stat_rx_0_2_stat_rx_packet_bad_fcs;
  wire stat_rx_0_2_stat_rx_packet_large;
  wire stat_rx_0_2_stat_rx_packet_small;
  wire stat_rx_0_2_stat_rx_received_local_fault;
  wire stat_rx_0_2_stat_rx_remote_fault;
  wire [1:0]stat_rx_0_2_stat_rx_stomped_fcs;
  wire stat_rx_0_2_stat_rx_test_pattern_mismatch;
  wire stat_rx_0_2_stat_rx_toolong;
  wire [3:0]stat_rx_0_2_stat_rx_total_bytes;
  wire [13:0]stat_rx_0_2_stat_rx_total_good_bytes;
  wire stat_rx_0_2_stat_rx_total_good_packets;
  wire [1:0]stat_rx_0_2_stat_rx_total_packets;
  wire stat_rx_0_2_stat_rx_truncated;
  wire stat_rx_0_2_stat_rx_undersize;
  wire stat_rx_0_2_stat_rx_unicast;
  wire stat_rx_0_2_stat_rx_valid_ctrl_code;
  wire stat_rx_0_2_stat_rx_vlan;
  wire stat_rx_0_3_stat_rx_bad_code;
  wire [1:0]stat_rx_0_3_stat_rx_bad_fcs;
  wire stat_rx_0_3_stat_rx_bad_preamble;
  wire stat_rx_0_3_stat_rx_bad_sfd;
  wire stat_rx_0_3_stat_rx_block_lock;
  wire stat_rx_0_3_stat_rx_broadcast;
  wire stat_rx_0_3_stat_rx_fragment;
  wire stat_rx_0_3_stat_rx_framing_err;
  wire stat_rx_0_3_stat_rx_framing_err_valid;
  wire stat_rx_0_3_stat_rx_got_signal_os;
  wire stat_rx_0_3_stat_rx_hi_ber;
  wire stat_rx_0_3_stat_rx_inrangeerr;
  wire stat_rx_0_3_stat_rx_internal_local_fault;
  wire stat_rx_0_3_stat_rx_jabber;
  wire stat_rx_0_3_stat_rx_local_fault;
  wire stat_rx_0_3_stat_rx_multicast;
  wire stat_rx_0_3_stat_rx_oversize;
  wire stat_rx_0_3_stat_rx_packet_1024_1518_bytes;
  wire stat_rx_0_3_stat_rx_packet_128_255_bytes;
  wire stat_rx_0_3_stat_rx_packet_1519_1522_bytes;
  wire stat_rx_0_3_stat_rx_packet_1523_1548_bytes;
  wire stat_rx_0_3_stat_rx_packet_1549_2047_bytes;
  wire stat_rx_0_3_stat_rx_packet_2048_4095_bytes;
  wire stat_rx_0_3_stat_rx_packet_256_511_bytes;
  wire stat_rx_0_3_stat_rx_packet_4096_8191_bytes;
  wire stat_rx_0_3_stat_rx_packet_512_1023_bytes;
  wire stat_rx_0_3_stat_rx_packet_64_bytes;
  wire stat_rx_0_3_stat_rx_packet_65_127_bytes;
  wire stat_rx_0_3_stat_rx_packet_8192_9215_bytes;
  wire stat_rx_0_3_stat_rx_packet_bad_fcs;
  wire stat_rx_0_3_stat_rx_packet_large;
  wire stat_rx_0_3_stat_rx_packet_small;
  wire stat_rx_0_3_stat_rx_received_local_fault;
  wire stat_rx_0_3_stat_rx_remote_fault;
  wire [1:0]stat_rx_0_3_stat_rx_stomped_fcs;
  wire stat_rx_0_3_stat_rx_test_pattern_mismatch;
  wire stat_rx_0_3_stat_rx_toolong;
  wire [3:0]stat_rx_0_3_stat_rx_total_bytes;
  wire [13:0]stat_rx_0_3_stat_rx_total_good_bytes;
  wire stat_rx_0_3_stat_rx_total_good_packets;
  wire [1:0]stat_rx_0_3_stat_rx_total_packets;
  wire stat_rx_0_3_stat_rx_truncated;
  wire stat_rx_0_3_stat_rx_undersize;
  wire stat_rx_0_3_stat_rx_unicast;
  wire stat_rx_0_3_stat_rx_valid_ctrl_code;
  wire stat_rx_0_3_stat_rx_vlan;
  wire stat_rx_status_0_0;
  wire stat_rx_status_0_1;
  wire stat_rx_status_0_2;
  wire stat_rx_status_0_3;
  wire stat_tx_0_0_stat_tx_bad_fcs;
  wire stat_tx_0_0_stat_tx_broadcast;
  wire stat_tx_0_0_stat_tx_frame_error;
  wire stat_tx_0_0_stat_tx_local_fault;
  wire stat_tx_0_0_stat_tx_multicast;
  wire stat_tx_0_0_stat_tx_packet_1024_1518_bytes;
  wire stat_tx_0_0_stat_tx_packet_128_255_bytes;
  wire stat_tx_0_0_stat_tx_packet_1519_1522_bytes;
  wire stat_tx_0_0_stat_tx_packet_1523_1548_bytes;
  wire stat_tx_0_0_stat_tx_packet_1549_2047_bytes;
  wire stat_tx_0_0_stat_tx_packet_2048_4095_bytes;
  wire stat_tx_0_0_stat_tx_packet_256_511_bytes;
  wire stat_tx_0_0_stat_tx_packet_4096_8191_bytes;
  wire stat_tx_0_0_stat_tx_packet_512_1023_bytes;
  wire stat_tx_0_0_stat_tx_packet_64_bytes;
  wire stat_tx_0_0_stat_tx_packet_65_127_bytes;
  wire stat_tx_0_0_stat_tx_packet_8192_9215_bytes;
  wire stat_tx_0_0_stat_tx_packet_large;
  wire stat_tx_0_0_stat_tx_packet_small;
  wire [3:0]stat_tx_0_0_stat_tx_total_bytes;
  wire [13:0]stat_tx_0_0_stat_tx_total_good_bytes;
  wire stat_tx_0_0_stat_tx_total_good_packets;
  wire stat_tx_0_0_stat_tx_total_packets;
  wire stat_tx_0_0_stat_tx_unicast;
  wire stat_tx_0_0_stat_tx_vlan;
  wire stat_tx_0_1_stat_tx_bad_fcs;
  wire stat_tx_0_1_stat_tx_broadcast;
  wire stat_tx_0_1_stat_tx_frame_error;
  wire stat_tx_0_1_stat_tx_local_fault;
  wire stat_tx_0_1_stat_tx_multicast;
  wire stat_tx_0_1_stat_tx_packet_1024_1518_bytes;
  wire stat_tx_0_1_stat_tx_packet_128_255_bytes;
  wire stat_tx_0_1_stat_tx_packet_1519_1522_bytes;
  wire stat_tx_0_1_stat_tx_packet_1523_1548_bytes;
  wire stat_tx_0_1_stat_tx_packet_1549_2047_bytes;
  wire stat_tx_0_1_stat_tx_packet_2048_4095_bytes;
  wire stat_tx_0_1_stat_tx_packet_256_511_bytes;
  wire stat_tx_0_1_stat_tx_packet_4096_8191_bytes;
  wire stat_tx_0_1_stat_tx_packet_512_1023_bytes;
  wire stat_tx_0_1_stat_tx_packet_64_bytes;
  wire stat_tx_0_1_stat_tx_packet_65_127_bytes;
  wire stat_tx_0_1_stat_tx_packet_8192_9215_bytes;
  wire stat_tx_0_1_stat_tx_packet_large;
  wire stat_tx_0_1_stat_tx_packet_small;
  wire [3:0]stat_tx_0_1_stat_tx_total_bytes;
  wire [13:0]stat_tx_0_1_stat_tx_total_good_bytes;
  wire stat_tx_0_1_stat_tx_total_good_packets;
  wire stat_tx_0_1_stat_tx_total_packets;
  wire stat_tx_0_1_stat_tx_unicast;
  wire stat_tx_0_1_stat_tx_vlan;
  wire stat_tx_0_2_stat_tx_bad_fcs;
  wire stat_tx_0_2_stat_tx_broadcast;
  wire stat_tx_0_2_stat_tx_frame_error;
  wire stat_tx_0_2_stat_tx_local_fault;
  wire stat_tx_0_2_stat_tx_multicast;
  wire stat_tx_0_2_stat_tx_packet_1024_1518_bytes;
  wire stat_tx_0_2_stat_tx_packet_128_255_bytes;
  wire stat_tx_0_2_stat_tx_packet_1519_1522_bytes;
  wire stat_tx_0_2_stat_tx_packet_1523_1548_bytes;
  wire stat_tx_0_2_stat_tx_packet_1549_2047_bytes;
  wire stat_tx_0_2_stat_tx_packet_2048_4095_bytes;
  wire stat_tx_0_2_stat_tx_packet_256_511_bytes;
  wire stat_tx_0_2_stat_tx_packet_4096_8191_bytes;
  wire stat_tx_0_2_stat_tx_packet_512_1023_bytes;
  wire stat_tx_0_2_stat_tx_packet_64_bytes;
  wire stat_tx_0_2_stat_tx_packet_65_127_bytes;
  wire stat_tx_0_2_stat_tx_packet_8192_9215_bytes;
  wire stat_tx_0_2_stat_tx_packet_large;
  wire stat_tx_0_2_stat_tx_packet_small;
  wire [3:0]stat_tx_0_2_stat_tx_total_bytes;
  wire [13:0]stat_tx_0_2_stat_tx_total_good_bytes;
  wire stat_tx_0_2_stat_tx_total_good_packets;
  wire stat_tx_0_2_stat_tx_total_packets;
  wire stat_tx_0_2_stat_tx_unicast;
  wire stat_tx_0_2_stat_tx_vlan;
  wire stat_tx_0_3_stat_tx_bad_fcs;
  wire stat_tx_0_3_stat_tx_broadcast;
  wire stat_tx_0_3_stat_tx_frame_error;
  wire stat_tx_0_3_stat_tx_local_fault;
  wire stat_tx_0_3_stat_tx_multicast;
  wire stat_tx_0_3_stat_tx_packet_1024_1518_bytes;
  wire stat_tx_0_3_stat_tx_packet_128_255_bytes;
  wire stat_tx_0_3_stat_tx_packet_1519_1522_bytes;
  wire stat_tx_0_3_stat_tx_packet_1523_1548_bytes;
  wire stat_tx_0_3_stat_tx_packet_1549_2047_bytes;
  wire stat_tx_0_3_stat_tx_packet_2048_4095_bytes;
  wire stat_tx_0_3_stat_tx_packet_256_511_bytes;
  wire stat_tx_0_3_stat_tx_packet_4096_8191_bytes;
  wire stat_tx_0_3_stat_tx_packet_512_1023_bytes;
  wire stat_tx_0_3_stat_tx_packet_64_bytes;
  wire stat_tx_0_3_stat_tx_packet_65_127_bytes;
  wire stat_tx_0_3_stat_tx_packet_8192_9215_bytes;
  wire stat_tx_0_3_stat_tx_packet_large;
  wire stat_tx_0_3_stat_tx_packet_small;
  wire [3:0]stat_tx_0_3_stat_tx_total_bytes;
  wire [13:0]stat_tx_0_3_stat_tx_total_good_bytes;
  wire stat_tx_0_3_stat_tx_total_good_packets;
  wire stat_tx_0_3_stat_tx_total_packets;
  wire stat_tx_0_3_stat_tx_unicast;
  wire stat_tx_0_3_stat_tx_vlan;
  wire tx_core_clk_0_0;
  wire tx_core_clk_0_1;
  wire tx_core_clk_0_2;
  wire tx_core_clk_0_3;
  wire [55:0]tx_preamblein_0_0;
  wire [55:0]tx_preamblein_0_1;
  wire [55:0]tx_preamblein_0_2;
  wire [55:0]tx_preamblein_0_3;
  wire tx_reset_0_0;
  wire tx_reset_0_1;
  wire tx_reset_0_2;
  wire tx_reset_0_3;
  wire tx_resetdone_out_0_0;
  wire tx_resetdone_out_0_1;
  wire tx_resetdone_out_0_2;
  wire tx_resetdone_out_0_3;
  wire tx_unfout_0_0;
  wire tx_unfout_0_1;
  wire tx_unfout_0_2;
  wire tx_unfout_0_3;
  wire tx_usrclk_0;
  wire tx_usrclk_1;
  wire tx_usrclk_2;
  wire tx_usrclk_3;
  wire [31:0]user_reg0_0_0;
  wire [31:0]user_reg0_0_1;
  wire [31:0]user_reg0_0_2;
  wire [31:0]user_reg0_0_3;

generate
    if (C_GT_MAC_ENABLE_RSFEC == 1) begin

        xbtest_sub_xxv_gt_0 xbtest_sub_xxv_gt_i
            (.FEC_stat_0_0_stat_fec_inc_cant_correct_count(FEC_stat_0_0_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_0_stat_fec_inc_correct_count(FEC_stat_0_0_stat_fec_inc_correct_count),
                .FEC_stat_0_0_stat_fec_lock_error(FEC_stat_0_0_stat_fec_lock_error),
                .FEC_stat_0_0_stat_fec_rx_lock(FEC_stat_0_0_stat_fec_rx_lock),
                .FEC_stat_0_1_stat_fec_inc_cant_correct_count(FEC_stat_0_1_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_1_stat_fec_inc_correct_count(FEC_stat_0_1_stat_fec_inc_correct_count),
                .FEC_stat_0_1_stat_fec_lock_error(FEC_stat_0_1_stat_fec_lock_error),
                .FEC_stat_0_1_stat_fec_rx_lock(FEC_stat_0_1_stat_fec_rx_lock),
                .FEC_stat_0_2_stat_fec_inc_cant_correct_count(FEC_stat_0_2_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_2_stat_fec_inc_correct_count(FEC_stat_0_2_stat_fec_inc_correct_count),
                .FEC_stat_0_2_stat_fec_lock_error(FEC_stat_0_2_stat_fec_lock_error),
                .FEC_stat_0_2_stat_fec_rx_lock(FEC_stat_0_2_stat_fec_rx_lock),
                .FEC_stat_0_3_stat_fec_inc_cant_correct_count(FEC_stat_0_3_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_3_stat_fec_inc_correct_count(FEC_stat_0_3_stat_fec_inc_correct_count),
                .FEC_stat_0_3_stat_fec_lock_error(FEC_stat_0_3_stat_fec_lock_error),
                .FEC_stat_0_3_stat_fec_rx_lock(FEC_stat_0_3_stat_fec_rx_lock),
                .RS_FEC_stat_0_0_stat_rx_rsfec_corrected_cw_inc(RS_FEC_stat_0_0_stat_rx_rsfec_corrected_cw_inc),
                .RS_FEC_stat_0_0_stat_rx_rsfec_err_count0_inc(RS_FEC_stat_0_0_stat_rx_rsfec_err_count0_inc),
                .RS_FEC_stat_0_0_stat_rx_rsfec_hi_ser(RS_FEC_stat_0_0_stat_rx_rsfec_hi_ser),
                .RS_FEC_stat_0_0_stat_rx_rsfec_lane_alignment_status(RS_FEC_stat_0_0_stat_rx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_0_stat_rx_rsfec_uncorrected_cw_inc(RS_FEC_stat_0_0_stat_rx_rsfec_uncorrected_cw_inc),
                .RS_FEC_stat_0_0_stat_tx_rsfec_lane_alignment_status(RS_FEC_stat_0_0_stat_tx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_1_stat_rx_rsfec_corrected_cw_inc(RS_FEC_stat_0_1_stat_rx_rsfec_corrected_cw_inc),
                .RS_FEC_stat_0_1_stat_rx_rsfec_err_count0_inc(RS_FEC_stat_0_1_stat_rx_rsfec_err_count0_inc),
                .RS_FEC_stat_0_1_stat_rx_rsfec_hi_ser(RS_FEC_stat_0_1_stat_rx_rsfec_hi_ser),
                .RS_FEC_stat_0_1_stat_rx_rsfec_lane_alignment_status(RS_FEC_stat_0_1_stat_rx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_1_stat_rx_rsfec_uncorrected_cw_inc(RS_FEC_stat_0_1_stat_rx_rsfec_uncorrected_cw_inc),
                .RS_FEC_stat_0_1_stat_tx_rsfec_lane_alignment_status(RS_FEC_stat_0_1_stat_tx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_2_stat_rx_rsfec_corrected_cw_inc(RS_FEC_stat_0_2_stat_rx_rsfec_corrected_cw_inc),
                .RS_FEC_stat_0_2_stat_rx_rsfec_err_count0_inc(RS_FEC_stat_0_2_stat_rx_rsfec_err_count0_inc),
                .RS_FEC_stat_0_2_stat_rx_rsfec_hi_ser(RS_FEC_stat_0_2_stat_rx_rsfec_hi_ser),
                .RS_FEC_stat_0_2_stat_rx_rsfec_lane_alignment_status(RS_FEC_stat_0_2_stat_rx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_2_stat_rx_rsfec_uncorrected_cw_inc(RS_FEC_stat_0_2_stat_rx_rsfec_uncorrected_cw_inc),
                .RS_FEC_stat_0_2_stat_tx_rsfec_lane_alignment_status(RS_FEC_stat_0_2_stat_tx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_3_stat_rx_rsfec_corrected_cw_inc(RS_FEC_stat_0_3_stat_rx_rsfec_corrected_cw_inc),
                .RS_FEC_stat_0_3_stat_rx_rsfec_err_count0_inc(RS_FEC_stat_0_3_stat_rx_rsfec_err_count0_inc),
                .RS_FEC_stat_0_3_stat_rx_rsfec_hi_ser(RS_FEC_stat_0_3_stat_rx_rsfec_hi_ser),
                .RS_FEC_stat_0_3_stat_rx_rsfec_lane_alignment_status(RS_FEC_stat_0_3_stat_rx_rsfec_lane_alignment_status),
                .RS_FEC_stat_0_3_stat_rx_rsfec_uncorrected_cw_inc(RS_FEC_stat_0_3_stat_rx_rsfec_uncorrected_cw_inc),
                .RS_FEC_stat_0_3_stat_tx_rsfec_lane_alignment_status(RS_FEC_stat_0_3_stat_tx_rsfec_lane_alignment_status),
                .apb3clk_quad(apb3clk_quad),
                .axis_rx_0_0_tdata(axis_rx_0_0_tdata),
                .axis_rx_0_0_tkeep(axis_rx_0_0_tkeep),
                .axis_rx_0_0_tlast(axis_rx_0_0_tlast),
                .axis_rx_0_0_tuser(axis_rx_0_0_tuser),
                .axis_rx_0_0_tvalid(axis_rx_0_0_tvalid),
                .axis_rx_0_1_tdata(axis_rx_0_1_tdata),
                .axis_rx_0_1_tkeep(axis_rx_0_1_tkeep),
                .axis_rx_0_1_tlast(axis_rx_0_1_tlast),
                .axis_rx_0_1_tuser(axis_rx_0_1_tuser),
                .axis_rx_0_1_tvalid(axis_rx_0_1_tvalid),
                .axis_rx_0_2_tdata(axis_rx_0_2_tdata),
                .axis_rx_0_2_tkeep(axis_rx_0_2_tkeep),
                .axis_rx_0_2_tlast(axis_rx_0_2_tlast),
                .axis_rx_0_2_tuser(axis_rx_0_2_tuser),
                .axis_rx_0_2_tvalid(axis_rx_0_2_tvalid),
                .axis_rx_0_3_tdata(axis_rx_0_3_tdata),
                .axis_rx_0_3_tkeep(axis_rx_0_3_tkeep),
                .axis_rx_0_3_tlast(axis_rx_0_3_tlast),
                .axis_rx_0_3_tuser(axis_rx_0_3_tuser),
                .axis_rx_0_3_tvalid(axis_rx_0_3_tvalid),
                .axis_tx_0_0_tdata(axis_tx_0_0_tdata),
                .axis_tx_0_0_tkeep(axis_tx_0_0_tkeep),
                .axis_tx_0_0_tlast(axis_tx_0_0_tlast),
                .axis_tx_0_0_tready(axis_tx_0_0_tready),
                .axis_tx_0_0_tuser(axis_tx_0_0_tuser),
                .axis_tx_0_0_tvalid(axis_tx_0_0_tvalid),
                .axis_tx_0_1_tdata(axis_tx_0_1_tdata),
                .axis_tx_0_1_tkeep(axis_tx_0_1_tkeep),
                .axis_tx_0_1_tlast(axis_tx_0_1_tlast),
                .axis_tx_0_1_tready(axis_tx_0_1_tready),
                .axis_tx_0_1_tuser(axis_tx_0_1_tuser),
                .axis_tx_0_1_tvalid(axis_tx_0_1_tvalid),
                .axis_tx_0_2_tdata(axis_tx_0_2_tdata),
                .axis_tx_0_2_tkeep(axis_tx_0_2_tkeep),
                .axis_tx_0_2_tlast(axis_tx_0_2_tlast),
                .axis_tx_0_2_tready(axis_tx_0_2_tready),
                .axis_tx_0_2_tuser(axis_tx_0_2_tuser),
                .axis_tx_0_2_tvalid(axis_tx_0_2_tvalid),
                .axis_tx_0_3_tdata(axis_tx_0_3_tdata),
                .axis_tx_0_3_tkeep(axis_tx_0_3_tkeep),
                .axis_tx_0_3_tlast(axis_tx_0_3_tlast),
                .axis_tx_0_3_tready(axis_tx_0_3_tready),
                .axis_tx_0_3_tuser(axis_tx_0_3_tuser),
                .axis_tx_0_3_tvalid(axis_tx_0_3_tvalid),
                .ctl_tx_0_0_ctl_tx_send_idle(ctl_tx_0_0_ctl_tx_send_idle),
                .ctl_tx_0_0_ctl_tx_send_lfi(ctl_tx_0_0_ctl_tx_send_lfi),
                .ctl_tx_0_0_ctl_tx_send_rfi(ctl_tx_0_0_ctl_tx_send_rfi),
                .ctl_tx_0_1_ctl_tx_send_idle(ctl_tx_0_1_ctl_tx_send_idle),
                .ctl_tx_0_1_ctl_tx_send_lfi(ctl_tx_0_1_ctl_tx_send_lfi),
                .ctl_tx_0_1_ctl_tx_send_rfi(ctl_tx_0_1_ctl_tx_send_rfi),
                .ctl_tx_0_2_ctl_tx_send_idle(ctl_tx_0_2_ctl_tx_send_idle),
                .ctl_tx_0_2_ctl_tx_send_lfi(ctl_tx_0_2_ctl_tx_send_lfi),
                .ctl_tx_0_2_ctl_tx_send_rfi(ctl_tx_0_2_ctl_tx_send_rfi),
                .ctl_tx_0_3_ctl_tx_send_idle(ctl_tx_0_3_ctl_tx_send_idle),
                .ctl_tx_0_3_ctl_tx_send_lfi(ctl_tx_0_3_ctl_tx_send_lfi),
                .ctl_tx_0_3_ctl_tx_send_rfi(ctl_tx_0_3_ctl_tx_send_rfi),
                .gt_rxn_in_0(gt_rxn_in_0),
                .gt_rxp_in_0(gt_rxp_in_0),
                .gt_txn_out_0(gt_txn_out_0),
                .gt_txp_out_0(gt_txp_out_0),
                .gtwiz_reset_clk_freerun_in_0_0(gtwiz_reset_clk_freerun_in_0_0),
                .gtwiz_reset_clk_freerun_in_0_1(gtwiz_reset_clk_freerun_in_0_1),
                .gtwiz_reset_clk_freerun_in_0_2(gtwiz_reset_clk_freerun_in_0_2),
                .gtwiz_reset_clk_freerun_in_0_3(gtwiz_reset_clk_freerun_in_0_3),
                .pm_tick_0_0(pm_tick_0_0),
                .pm_tick_0_1(pm_tick_0_1),
                .pm_tick_0_2(pm_tick_0_2),
                .pm_tick_0_3(pm_tick_0_3),
                .gt_refclk_clk_n(gt_refclk_clk_n),
                .gt_refclk_clk_p(gt_refclk_clk_p),
                .rx_core_clk_0_0(rx_core_clk_0_0),
                .rx_core_clk_0_1(rx_core_clk_0_1),
                .rx_core_clk_0_2(rx_core_clk_0_2),
                .rx_core_clk_0_3(rx_core_clk_0_3),
                .rx_preambleout_0_0(rx_preambleout_0_0),
                .rx_preambleout_0_1(rx_preambleout_0_1),
                .rx_preambleout_0_2(rx_preambleout_0_2),
                .rx_preambleout_0_3(rx_preambleout_0_3),
                .rx_reset_0_0(rx_reset_0_0),
                .rx_reset_0_1(rx_reset_0_1),
                .rx_reset_0_2(rx_reset_0_2),
                .rx_reset_0_3(rx_reset_0_3),
                .rx_resetdone_out_0_0(rx_resetdone_out_0_0),
                .rx_resetdone_out_0_1(rx_resetdone_out_0_1),
                .rx_resetdone_out_0_2(rx_resetdone_out_0_2),
                .rx_resetdone_out_0_3(rx_resetdone_out_0_3),
                .rx_serdes_clk_0_0(rx_serdes_clk_0_0),
                .rx_serdes_clk_0_1(rx_serdes_clk_0_1),
                .rx_serdes_clk_0_2(rx_serdes_clk_0_2),
                .rx_serdes_clk_0_3(rx_serdes_clk_0_3),
                .rx_serdes_reset_0_0(rx_serdes_reset_0_0),
                .rx_serdes_reset_0_1(rx_serdes_reset_0_1),
                .rx_serdes_reset_0_2(rx_serdes_reset_0_2),
                .rx_serdes_reset_0_3(rx_serdes_reset_0_3),
                .rx_usrclk_0(rx_usrclk_0),
                .rx_usrclk_1(rx_usrclk_1),
                .rx_usrclk_2(rx_usrclk_2),
                .rx_usrclk_3(rx_usrclk_3),
                .s_axi_0_0_araddr(s_axi_0_0_araddr),
                .s_axi_0_0_arready(s_axi_0_0_arready),
                .s_axi_0_0_arvalid(s_axi_0_0_arvalid),
                .s_axi_0_0_awaddr(s_axi_0_0_awaddr),
                .s_axi_0_0_awready(s_axi_0_0_awready),
                .s_axi_0_0_awvalid(s_axi_0_0_awvalid),
                .s_axi_0_0_bready(s_axi_0_0_bready),
                .s_axi_0_0_bresp(s_axi_0_0_bresp),
                .s_axi_0_0_bvalid(s_axi_0_0_bvalid),
                .s_axi_0_0_rdata(s_axi_0_0_rdata),
                .s_axi_0_0_rready(s_axi_0_0_rready),
                .s_axi_0_0_rresp(s_axi_0_0_rresp),
                .s_axi_0_0_rvalid(s_axi_0_0_rvalid),
                .s_axi_0_0_wdata(s_axi_0_0_wdata),
                .s_axi_0_0_wready(s_axi_0_0_wready),
                .s_axi_0_0_wstrb(s_axi_0_0_wstrb),
                .s_axi_0_0_wvalid(s_axi_0_0_wvalid),
                .s_axi_0_1_araddr(s_axi_0_1_araddr),
                .s_axi_0_1_arready(s_axi_0_1_arready),
                .s_axi_0_1_arvalid(s_axi_0_1_arvalid),
                .s_axi_0_1_awaddr(s_axi_0_1_awaddr),
                .s_axi_0_1_awready(s_axi_0_1_awready),
                .s_axi_0_1_awvalid(s_axi_0_1_awvalid),
                .s_axi_0_1_bready(s_axi_0_1_bready),
                .s_axi_0_1_bresp(s_axi_0_1_bresp),
                .s_axi_0_1_bvalid(s_axi_0_1_bvalid),
                .s_axi_0_1_rdata(s_axi_0_1_rdata),
                .s_axi_0_1_rready(s_axi_0_1_rready),
                .s_axi_0_1_rresp(s_axi_0_1_rresp),
                .s_axi_0_1_rvalid(s_axi_0_1_rvalid),
                .s_axi_0_1_wdata(s_axi_0_1_wdata),
                .s_axi_0_1_wready(s_axi_0_1_wready),
                .s_axi_0_1_wstrb(s_axi_0_1_wstrb),
                .s_axi_0_1_wvalid(s_axi_0_1_wvalid),
                .s_axi_0_2_araddr(s_axi_0_2_araddr),
                .s_axi_0_2_arready(s_axi_0_2_arready),
                .s_axi_0_2_arvalid(s_axi_0_2_arvalid),
                .s_axi_0_2_awaddr(s_axi_0_2_awaddr),
                .s_axi_0_2_awready(s_axi_0_2_awready),
                .s_axi_0_2_awvalid(s_axi_0_2_awvalid),
                .s_axi_0_2_bready(s_axi_0_2_bready),
                .s_axi_0_2_bresp(s_axi_0_2_bresp),
                .s_axi_0_2_bvalid(s_axi_0_2_bvalid),
                .s_axi_0_2_rdata(s_axi_0_2_rdata),
                .s_axi_0_2_rready(s_axi_0_2_rready),
                .s_axi_0_2_rresp(s_axi_0_2_rresp),
                .s_axi_0_2_rvalid(s_axi_0_2_rvalid),
                .s_axi_0_2_wdata(s_axi_0_2_wdata),
                .s_axi_0_2_wready(s_axi_0_2_wready),
                .s_axi_0_2_wstrb(s_axi_0_2_wstrb),
                .s_axi_0_2_wvalid(s_axi_0_2_wvalid),
                .s_axi_0_3_araddr(s_axi_0_3_araddr),
                .s_axi_0_3_arready(s_axi_0_3_arready),
                .s_axi_0_3_arvalid(s_axi_0_3_arvalid),
                .s_axi_0_3_awaddr(s_axi_0_3_awaddr),
                .s_axi_0_3_awready(s_axi_0_3_awready),
                .s_axi_0_3_awvalid(s_axi_0_3_awvalid),
                .s_axi_0_3_bready(s_axi_0_3_bready),
                .s_axi_0_3_bresp(s_axi_0_3_bresp),
                .s_axi_0_3_bvalid(s_axi_0_3_bvalid),
                .s_axi_0_3_rdata(s_axi_0_3_rdata),
                .s_axi_0_3_rready(s_axi_0_3_rready),
                .s_axi_0_3_rresp(s_axi_0_3_rresp),
                .s_axi_0_3_rvalid(s_axi_0_3_rvalid),
                .s_axi_0_3_wdata(s_axi_0_3_wdata),
                .s_axi_0_3_wready(s_axi_0_3_wready),
                .s_axi_0_3_wstrb(s_axi_0_3_wstrb),
                .s_axi_0_3_wvalid(s_axi_0_3_wvalid),
                .s_axi_aclk_0_0(s_axi_aclk_0_0),
                .s_axi_aclk_0_1(s_axi_aclk_0_1),
                .s_axi_aclk_0_2(s_axi_aclk_0_2),
                .s_axi_aclk_0_3(s_axi_aclk_0_3),
                .s_axi_aresetn_0_0(s_axi_aresetn_0_0),
                .s_axi_aresetn_0_1(s_axi_aresetn_0_1),
                .s_axi_aresetn_0_2(s_axi_aresetn_0_2),
                .s_axi_aresetn_0_3(s_axi_aresetn_0_3),
                .stat_rx_0_0_stat_rx_bad_code(stat_rx_0_0_stat_rx_bad_code),
                .stat_rx_0_0_stat_rx_bad_fcs(stat_rx_0_0_stat_rx_bad_fcs),
                .stat_rx_0_0_stat_rx_bad_preamble(stat_rx_0_0_stat_rx_bad_preamble),
                .stat_rx_0_0_stat_rx_bad_sfd(stat_rx_0_0_stat_rx_bad_sfd),
                .stat_rx_0_0_stat_rx_block_lock(stat_rx_0_0_stat_rx_block_lock),
                .stat_rx_0_0_stat_rx_broadcast(stat_rx_0_0_stat_rx_broadcast),
                .stat_rx_0_0_stat_rx_fragment(stat_rx_0_0_stat_rx_fragment),
                .stat_rx_0_0_stat_rx_framing_err(stat_rx_0_0_stat_rx_framing_err),
                .stat_rx_0_0_stat_rx_framing_err_valid(stat_rx_0_0_stat_rx_framing_err_valid),
                .stat_rx_0_0_stat_rx_got_signal_os(stat_rx_0_0_stat_rx_got_signal_os),
                .stat_rx_0_0_stat_rx_hi_ber(stat_rx_0_0_stat_rx_hi_ber),
                .stat_rx_0_0_stat_rx_inrangeerr(stat_rx_0_0_stat_rx_inrangeerr),
                .stat_rx_0_0_stat_rx_internal_local_fault(stat_rx_0_0_stat_rx_internal_local_fault),
                .stat_rx_0_0_stat_rx_jabber(stat_rx_0_0_stat_rx_jabber),
                .stat_rx_0_0_stat_rx_local_fault(stat_rx_0_0_stat_rx_local_fault),
                .stat_rx_0_0_stat_rx_multicast(stat_rx_0_0_stat_rx_multicast),
                .stat_rx_0_0_stat_rx_oversize(stat_rx_0_0_stat_rx_oversize),
                .stat_rx_0_0_stat_rx_packet_1024_1518_bytes(stat_rx_0_0_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_0_stat_rx_packet_128_255_bytes(stat_rx_0_0_stat_rx_packet_128_255_bytes),
                .stat_rx_0_0_stat_rx_packet_1519_1522_bytes(stat_rx_0_0_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_0_stat_rx_packet_1523_1548_bytes(stat_rx_0_0_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_0_stat_rx_packet_1549_2047_bytes(stat_rx_0_0_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_0_stat_rx_packet_2048_4095_bytes(stat_rx_0_0_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_0_stat_rx_packet_256_511_bytes(stat_rx_0_0_stat_rx_packet_256_511_bytes),
                .stat_rx_0_0_stat_rx_packet_4096_8191_bytes(stat_rx_0_0_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_0_stat_rx_packet_512_1023_bytes(stat_rx_0_0_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_0_stat_rx_packet_64_bytes(stat_rx_0_0_stat_rx_packet_64_bytes),
                .stat_rx_0_0_stat_rx_packet_65_127_bytes(stat_rx_0_0_stat_rx_packet_65_127_bytes),
                .stat_rx_0_0_stat_rx_packet_8192_9215_bytes(stat_rx_0_0_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_0_stat_rx_packet_bad_fcs(stat_rx_0_0_stat_rx_packet_bad_fcs),
                .stat_rx_0_0_stat_rx_packet_large(stat_rx_0_0_stat_rx_packet_large),
                .stat_rx_0_0_stat_rx_packet_small(stat_rx_0_0_stat_rx_packet_small),
                .stat_rx_0_0_stat_rx_received_local_fault(stat_rx_0_0_stat_rx_received_local_fault),
                .stat_rx_0_0_stat_rx_remote_fault(stat_rx_0_0_stat_rx_remote_fault),
                .stat_rx_0_0_stat_rx_stomped_fcs(stat_rx_0_0_stat_rx_stomped_fcs),
                .stat_rx_0_0_stat_rx_test_pattern_mismatch(stat_rx_0_0_stat_rx_test_pattern_mismatch),
                .stat_rx_0_0_stat_rx_toolong(stat_rx_0_0_stat_rx_toolong),
                .stat_rx_0_0_stat_rx_total_bytes(stat_rx_0_0_stat_rx_total_bytes),
                .stat_rx_0_0_stat_rx_total_good_bytes(stat_rx_0_0_stat_rx_total_good_bytes),
                .stat_rx_0_0_stat_rx_total_good_packets(stat_rx_0_0_stat_rx_total_good_packets),
                .stat_rx_0_0_stat_rx_total_packets(stat_rx_0_0_stat_rx_total_packets),
                .stat_rx_0_0_stat_rx_truncated(stat_rx_0_0_stat_rx_truncated),
                .stat_rx_0_0_stat_rx_undersize(stat_rx_0_0_stat_rx_undersize),
                .stat_rx_0_0_stat_rx_unicast(stat_rx_0_0_stat_rx_unicast),
                .stat_rx_0_0_stat_rx_valid_ctrl_code(stat_rx_0_0_stat_rx_valid_ctrl_code),
                .stat_rx_0_0_stat_rx_vlan(stat_rx_0_0_stat_rx_vlan),
                .stat_rx_0_1_stat_rx_bad_code(stat_rx_0_1_stat_rx_bad_code),
                .stat_rx_0_1_stat_rx_bad_fcs(stat_rx_0_1_stat_rx_bad_fcs),
                .stat_rx_0_1_stat_rx_bad_preamble(stat_rx_0_1_stat_rx_bad_preamble),
                .stat_rx_0_1_stat_rx_bad_sfd(stat_rx_0_1_stat_rx_bad_sfd),
                .stat_rx_0_1_stat_rx_block_lock(stat_rx_0_1_stat_rx_block_lock),
                .stat_rx_0_1_stat_rx_broadcast(stat_rx_0_1_stat_rx_broadcast),
                .stat_rx_0_1_stat_rx_fragment(stat_rx_0_1_stat_rx_fragment),
                .stat_rx_0_1_stat_rx_framing_err(stat_rx_0_1_stat_rx_framing_err),
                .stat_rx_0_1_stat_rx_framing_err_valid(stat_rx_0_1_stat_rx_framing_err_valid),
                .stat_rx_0_1_stat_rx_got_signal_os(stat_rx_0_1_stat_rx_got_signal_os),
                .stat_rx_0_1_stat_rx_hi_ber(stat_rx_0_1_stat_rx_hi_ber),
                .stat_rx_0_1_stat_rx_inrangeerr(stat_rx_0_1_stat_rx_inrangeerr),
                .stat_rx_0_1_stat_rx_internal_local_fault(stat_rx_0_1_stat_rx_internal_local_fault),
                .stat_rx_0_1_stat_rx_jabber(stat_rx_0_1_stat_rx_jabber),
                .stat_rx_0_1_stat_rx_local_fault(stat_rx_0_1_stat_rx_local_fault),
                .stat_rx_0_1_stat_rx_multicast(stat_rx_0_1_stat_rx_multicast),
                .stat_rx_0_1_stat_rx_oversize(stat_rx_0_1_stat_rx_oversize),
                .stat_rx_0_1_stat_rx_packet_1024_1518_bytes(stat_rx_0_1_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_1_stat_rx_packet_128_255_bytes(stat_rx_0_1_stat_rx_packet_128_255_bytes),
                .stat_rx_0_1_stat_rx_packet_1519_1522_bytes(stat_rx_0_1_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_1_stat_rx_packet_1523_1548_bytes(stat_rx_0_1_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_1_stat_rx_packet_1549_2047_bytes(stat_rx_0_1_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_1_stat_rx_packet_2048_4095_bytes(stat_rx_0_1_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_1_stat_rx_packet_256_511_bytes(stat_rx_0_1_stat_rx_packet_256_511_bytes),
                .stat_rx_0_1_stat_rx_packet_4096_8191_bytes(stat_rx_0_1_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_1_stat_rx_packet_512_1023_bytes(stat_rx_0_1_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_1_stat_rx_packet_64_bytes(stat_rx_0_1_stat_rx_packet_64_bytes),
                .stat_rx_0_1_stat_rx_packet_65_127_bytes(stat_rx_0_1_stat_rx_packet_65_127_bytes),
                .stat_rx_0_1_stat_rx_packet_8192_9215_bytes(stat_rx_0_1_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_1_stat_rx_packet_bad_fcs(stat_rx_0_1_stat_rx_packet_bad_fcs),
                .stat_rx_0_1_stat_rx_packet_large(stat_rx_0_1_stat_rx_packet_large),
                .stat_rx_0_1_stat_rx_packet_small(stat_rx_0_1_stat_rx_packet_small),
                .stat_rx_0_1_stat_rx_received_local_fault(stat_rx_0_1_stat_rx_received_local_fault),
                .stat_rx_0_1_stat_rx_remote_fault(stat_rx_0_1_stat_rx_remote_fault),
                .stat_rx_0_1_stat_rx_stomped_fcs(stat_rx_0_1_stat_rx_stomped_fcs),
                .stat_rx_0_1_stat_rx_test_pattern_mismatch(stat_rx_0_1_stat_rx_test_pattern_mismatch),
                .stat_rx_0_1_stat_rx_toolong(stat_rx_0_1_stat_rx_toolong),
                .stat_rx_0_1_stat_rx_total_bytes(stat_rx_0_1_stat_rx_total_bytes),
                .stat_rx_0_1_stat_rx_total_good_bytes(stat_rx_0_1_stat_rx_total_good_bytes),
                .stat_rx_0_1_stat_rx_total_good_packets(stat_rx_0_1_stat_rx_total_good_packets),
                .stat_rx_0_1_stat_rx_total_packets(stat_rx_0_1_stat_rx_total_packets),
                .stat_rx_0_1_stat_rx_truncated(stat_rx_0_1_stat_rx_truncated),
                .stat_rx_0_1_stat_rx_undersize(stat_rx_0_1_stat_rx_undersize),
                .stat_rx_0_1_stat_rx_unicast(stat_rx_0_1_stat_rx_unicast),
                .stat_rx_0_1_stat_rx_valid_ctrl_code(stat_rx_0_1_stat_rx_valid_ctrl_code),
                .stat_rx_0_1_stat_rx_vlan(stat_rx_0_1_stat_rx_vlan),
                .stat_rx_0_2_stat_rx_bad_code(stat_rx_0_2_stat_rx_bad_code),
                .stat_rx_0_2_stat_rx_bad_fcs(stat_rx_0_2_stat_rx_bad_fcs),
                .stat_rx_0_2_stat_rx_bad_preamble(stat_rx_0_2_stat_rx_bad_preamble),
                .stat_rx_0_2_stat_rx_bad_sfd(stat_rx_0_2_stat_rx_bad_sfd),
                .stat_rx_0_2_stat_rx_block_lock(stat_rx_0_2_stat_rx_block_lock),
                .stat_rx_0_2_stat_rx_broadcast(stat_rx_0_2_stat_rx_broadcast),
                .stat_rx_0_2_stat_rx_fragment(stat_rx_0_2_stat_rx_fragment),
                .stat_rx_0_2_stat_rx_framing_err(stat_rx_0_2_stat_rx_framing_err),
                .stat_rx_0_2_stat_rx_framing_err_valid(stat_rx_0_2_stat_rx_framing_err_valid),
                .stat_rx_0_2_stat_rx_got_signal_os(stat_rx_0_2_stat_rx_got_signal_os),
                .stat_rx_0_2_stat_rx_hi_ber(stat_rx_0_2_stat_rx_hi_ber),
                .stat_rx_0_2_stat_rx_inrangeerr(stat_rx_0_2_stat_rx_inrangeerr),
                .stat_rx_0_2_stat_rx_internal_local_fault(stat_rx_0_2_stat_rx_internal_local_fault),
                .stat_rx_0_2_stat_rx_jabber(stat_rx_0_2_stat_rx_jabber),
                .stat_rx_0_2_stat_rx_local_fault(stat_rx_0_2_stat_rx_local_fault),
                .stat_rx_0_2_stat_rx_multicast(stat_rx_0_2_stat_rx_multicast),
                .stat_rx_0_2_stat_rx_oversize(stat_rx_0_2_stat_rx_oversize),
                .stat_rx_0_2_stat_rx_packet_1024_1518_bytes(stat_rx_0_2_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_2_stat_rx_packet_128_255_bytes(stat_rx_0_2_stat_rx_packet_128_255_bytes),
                .stat_rx_0_2_stat_rx_packet_1519_1522_bytes(stat_rx_0_2_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_2_stat_rx_packet_1523_1548_bytes(stat_rx_0_2_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_2_stat_rx_packet_1549_2047_bytes(stat_rx_0_2_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_2_stat_rx_packet_2048_4095_bytes(stat_rx_0_2_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_2_stat_rx_packet_256_511_bytes(stat_rx_0_2_stat_rx_packet_256_511_bytes),
                .stat_rx_0_2_stat_rx_packet_4096_8191_bytes(stat_rx_0_2_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_2_stat_rx_packet_512_1023_bytes(stat_rx_0_2_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_2_stat_rx_packet_64_bytes(stat_rx_0_2_stat_rx_packet_64_bytes),
                .stat_rx_0_2_stat_rx_packet_65_127_bytes(stat_rx_0_2_stat_rx_packet_65_127_bytes),
                .stat_rx_0_2_stat_rx_packet_8192_9215_bytes(stat_rx_0_2_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_2_stat_rx_packet_bad_fcs(stat_rx_0_2_stat_rx_packet_bad_fcs),
                .stat_rx_0_2_stat_rx_packet_large(stat_rx_0_2_stat_rx_packet_large),
                .stat_rx_0_2_stat_rx_packet_small(stat_rx_0_2_stat_rx_packet_small),
                .stat_rx_0_2_stat_rx_received_local_fault(stat_rx_0_2_stat_rx_received_local_fault),
                .stat_rx_0_2_stat_rx_remote_fault(stat_rx_0_2_stat_rx_remote_fault),
                .stat_rx_0_2_stat_rx_stomped_fcs(stat_rx_0_2_stat_rx_stomped_fcs),
                .stat_rx_0_2_stat_rx_test_pattern_mismatch(stat_rx_0_2_stat_rx_test_pattern_mismatch),
                .stat_rx_0_2_stat_rx_toolong(stat_rx_0_2_stat_rx_toolong),
                .stat_rx_0_2_stat_rx_total_bytes(stat_rx_0_2_stat_rx_total_bytes),
                .stat_rx_0_2_stat_rx_total_good_bytes(stat_rx_0_2_stat_rx_total_good_bytes),
                .stat_rx_0_2_stat_rx_total_good_packets(stat_rx_0_2_stat_rx_total_good_packets),
                .stat_rx_0_2_stat_rx_total_packets(stat_rx_0_2_stat_rx_total_packets),
                .stat_rx_0_2_stat_rx_truncated(stat_rx_0_2_stat_rx_truncated),
                .stat_rx_0_2_stat_rx_undersize(stat_rx_0_2_stat_rx_undersize),
                .stat_rx_0_2_stat_rx_unicast(stat_rx_0_2_stat_rx_unicast),
                .stat_rx_0_2_stat_rx_valid_ctrl_code(stat_rx_0_2_stat_rx_valid_ctrl_code),
                .stat_rx_0_2_stat_rx_vlan(stat_rx_0_2_stat_rx_vlan),
                .stat_rx_0_3_stat_rx_bad_code(stat_rx_0_3_stat_rx_bad_code),
                .stat_rx_0_3_stat_rx_bad_fcs(stat_rx_0_3_stat_rx_bad_fcs),
                .stat_rx_0_3_stat_rx_bad_preamble(stat_rx_0_3_stat_rx_bad_preamble),
                .stat_rx_0_3_stat_rx_bad_sfd(stat_rx_0_3_stat_rx_bad_sfd),
                .stat_rx_0_3_stat_rx_block_lock(stat_rx_0_3_stat_rx_block_lock),
                .stat_rx_0_3_stat_rx_broadcast(stat_rx_0_3_stat_rx_broadcast),
                .stat_rx_0_3_stat_rx_fragment(stat_rx_0_3_stat_rx_fragment),
                .stat_rx_0_3_stat_rx_framing_err(stat_rx_0_3_stat_rx_framing_err),
                .stat_rx_0_3_stat_rx_framing_err_valid(stat_rx_0_3_stat_rx_framing_err_valid),
                .stat_rx_0_3_stat_rx_got_signal_os(stat_rx_0_3_stat_rx_got_signal_os),
                .stat_rx_0_3_stat_rx_hi_ber(stat_rx_0_3_stat_rx_hi_ber),
                .stat_rx_0_3_stat_rx_inrangeerr(stat_rx_0_3_stat_rx_inrangeerr),
                .stat_rx_0_3_stat_rx_internal_local_fault(stat_rx_0_3_stat_rx_internal_local_fault),
                .stat_rx_0_3_stat_rx_jabber(stat_rx_0_3_stat_rx_jabber),
                .stat_rx_0_3_stat_rx_local_fault(stat_rx_0_3_stat_rx_local_fault),
                .stat_rx_0_3_stat_rx_multicast(stat_rx_0_3_stat_rx_multicast),
                .stat_rx_0_3_stat_rx_oversize(stat_rx_0_3_stat_rx_oversize),
                .stat_rx_0_3_stat_rx_packet_1024_1518_bytes(stat_rx_0_3_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_3_stat_rx_packet_128_255_bytes(stat_rx_0_3_stat_rx_packet_128_255_bytes),
                .stat_rx_0_3_stat_rx_packet_1519_1522_bytes(stat_rx_0_3_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_3_stat_rx_packet_1523_1548_bytes(stat_rx_0_3_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_3_stat_rx_packet_1549_2047_bytes(stat_rx_0_3_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_3_stat_rx_packet_2048_4095_bytes(stat_rx_0_3_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_3_stat_rx_packet_256_511_bytes(stat_rx_0_3_stat_rx_packet_256_511_bytes),
                .stat_rx_0_3_stat_rx_packet_4096_8191_bytes(stat_rx_0_3_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_3_stat_rx_packet_512_1023_bytes(stat_rx_0_3_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_3_stat_rx_packet_64_bytes(stat_rx_0_3_stat_rx_packet_64_bytes),
                .stat_rx_0_3_stat_rx_packet_65_127_bytes(stat_rx_0_3_stat_rx_packet_65_127_bytes),
                .stat_rx_0_3_stat_rx_packet_8192_9215_bytes(stat_rx_0_3_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_3_stat_rx_packet_bad_fcs(stat_rx_0_3_stat_rx_packet_bad_fcs),
                .stat_rx_0_3_stat_rx_packet_large(stat_rx_0_3_stat_rx_packet_large),
                .stat_rx_0_3_stat_rx_packet_small(stat_rx_0_3_stat_rx_packet_small),
                .stat_rx_0_3_stat_rx_received_local_fault(stat_rx_0_3_stat_rx_received_local_fault),
                .stat_rx_0_3_stat_rx_remote_fault(stat_rx_0_3_stat_rx_remote_fault),
                .stat_rx_0_3_stat_rx_stomped_fcs(stat_rx_0_3_stat_rx_stomped_fcs),
                .stat_rx_0_3_stat_rx_test_pattern_mismatch(stat_rx_0_3_stat_rx_test_pattern_mismatch),
                .stat_rx_0_3_stat_rx_toolong(stat_rx_0_3_stat_rx_toolong),
                .stat_rx_0_3_stat_rx_total_bytes(stat_rx_0_3_stat_rx_total_bytes),
                .stat_rx_0_3_stat_rx_total_good_bytes(stat_rx_0_3_stat_rx_total_good_bytes),
                .stat_rx_0_3_stat_rx_total_good_packets(stat_rx_0_3_stat_rx_total_good_packets),
                .stat_rx_0_3_stat_rx_total_packets(stat_rx_0_3_stat_rx_total_packets),
                .stat_rx_0_3_stat_rx_truncated(stat_rx_0_3_stat_rx_truncated),
                .stat_rx_0_3_stat_rx_undersize(stat_rx_0_3_stat_rx_undersize),
                .stat_rx_0_3_stat_rx_unicast(stat_rx_0_3_stat_rx_unicast),
                .stat_rx_0_3_stat_rx_valid_ctrl_code(stat_rx_0_3_stat_rx_valid_ctrl_code),
                .stat_rx_0_3_stat_rx_vlan(stat_rx_0_3_stat_rx_vlan),
                .stat_rx_status_0_0(stat_rx_status_0_0),
                .stat_rx_status_0_1(stat_rx_status_0_1),
                .stat_rx_status_0_2(stat_rx_status_0_2),
                .stat_rx_status_0_3(stat_rx_status_0_3),
                .stat_tx_0_0_stat_tx_bad_fcs(stat_tx_0_0_stat_tx_bad_fcs),
                .stat_tx_0_0_stat_tx_broadcast(stat_tx_0_0_stat_tx_broadcast),
                .stat_tx_0_0_stat_tx_frame_error(stat_tx_0_0_stat_tx_frame_error),
                .stat_tx_0_0_stat_tx_local_fault(stat_tx_0_0_stat_tx_local_fault),
                .stat_tx_0_0_stat_tx_multicast(stat_tx_0_0_stat_tx_multicast),
                .stat_tx_0_0_stat_tx_packet_1024_1518_bytes(stat_tx_0_0_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_0_stat_tx_packet_128_255_bytes(stat_tx_0_0_stat_tx_packet_128_255_bytes),
                .stat_tx_0_0_stat_tx_packet_1519_1522_bytes(stat_tx_0_0_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_0_stat_tx_packet_1523_1548_bytes(stat_tx_0_0_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_0_stat_tx_packet_1549_2047_bytes(stat_tx_0_0_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_0_stat_tx_packet_2048_4095_bytes(stat_tx_0_0_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_0_stat_tx_packet_256_511_bytes(stat_tx_0_0_stat_tx_packet_256_511_bytes),
                .stat_tx_0_0_stat_tx_packet_4096_8191_bytes(stat_tx_0_0_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_0_stat_tx_packet_512_1023_bytes(stat_tx_0_0_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_0_stat_tx_packet_64_bytes(stat_tx_0_0_stat_tx_packet_64_bytes),
                .stat_tx_0_0_stat_tx_packet_65_127_bytes(stat_tx_0_0_stat_tx_packet_65_127_bytes),
                .stat_tx_0_0_stat_tx_packet_8192_9215_bytes(stat_tx_0_0_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_0_stat_tx_packet_large(stat_tx_0_0_stat_tx_packet_large),
                .stat_tx_0_0_stat_tx_packet_small(stat_tx_0_0_stat_tx_packet_small),
                .stat_tx_0_0_stat_tx_total_bytes(stat_tx_0_0_stat_tx_total_bytes),
                .stat_tx_0_0_stat_tx_total_good_bytes(stat_tx_0_0_stat_tx_total_good_bytes),
                .stat_tx_0_0_stat_tx_total_good_packets(stat_tx_0_0_stat_tx_total_good_packets),
                .stat_tx_0_0_stat_tx_total_packets(stat_tx_0_0_stat_tx_total_packets),
                .stat_tx_0_0_stat_tx_unicast(stat_tx_0_0_stat_tx_unicast),
                .stat_tx_0_0_stat_tx_vlan(stat_tx_0_0_stat_tx_vlan),
                .stat_tx_0_1_stat_tx_bad_fcs(stat_tx_0_1_stat_tx_bad_fcs),
                .stat_tx_0_1_stat_tx_broadcast(stat_tx_0_1_stat_tx_broadcast),
                .stat_tx_0_1_stat_tx_frame_error(stat_tx_0_1_stat_tx_frame_error),
                .stat_tx_0_1_stat_tx_local_fault(stat_tx_0_1_stat_tx_local_fault),
                .stat_tx_0_1_stat_tx_multicast(stat_tx_0_1_stat_tx_multicast),
                .stat_tx_0_1_stat_tx_packet_1024_1518_bytes(stat_tx_0_1_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_1_stat_tx_packet_128_255_bytes(stat_tx_0_1_stat_tx_packet_128_255_bytes),
                .stat_tx_0_1_stat_tx_packet_1519_1522_bytes(stat_tx_0_1_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_1_stat_tx_packet_1523_1548_bytes(stat_tx_0_1_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_1_stat_tx_packet_1549_2047_bytes(stat_tx_0_1_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_1_stat_tx_packet_2048_4095_bytes(stat_tx_0_1_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_1_stat_tx_packet_256_511_bytes(stat_tx_0_1_stat_tx_packet_256_511_bytes),
                .stat_tx_0_1_stat_tx_packet_4096_8191_bytes(stat_tx_0_1_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_1_stat_tx_packet_512_1023_bytes(stat_tx_0_1_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_1_stat_tx_packet_64_bytes(stat_tx_0_1_stat_tx_packet_64_bytes),
                .stat_tx_0_1_stat_tx_packet_65_127_bytes(stat_tx_0_1_stat_tx_packet_65_127_bytes),
                .stat_tx_0_1_stat_tx_packet_8192_9215_bytes(stat_tx_0_1_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_1_stat_tx_packet_large(stat_tx_0_1_stat_tx_packet_large),
                .stat_tx_0_1_stat_tx_packet_small(stat_tx_0_1_stat_tx_packet_small),
                .stat_tx_0_1_stat_tx_total_bytes(stat_tx_0_1_stat_tx_total_bytes),
                .stat_tx_0_1_stat_tx_total_good_bytes(stat_tx_0_1_stat_tx_total_good_bytes),
                .stat_tx_0_1_stat_tx_total_good_packets(stat_tx_0_1_stat_tx_total_good_packets),
                .stat_tx_0_1_stat_tx_total_packets(stat_tx_0_1_stat_tx_total_packets),
                .stat_tx_0_1_stat_tx_unicast(stat_tx_0_1_stat_tx_unicast),
                .stat_tx_0_1_stat_tx_vlan(stat_tx_0_1_stat_tx_vlan),
                .stat_tx_0_2_stat_tx_bad_fcs(stat_tx_0_2_stat_tx_bad_fcs),
                .stat_tx_0_2_stat_tx_broadcast(stat_tx_0_2_stat_tx_broadcast),
                .stat_tx_0_2_stat_tx_frame_error(stat_tx_0_2_stat_tx_frame_error),
                .stat_tx_0_2_stat_tx_local_fault(stat_tx_0_2_stat_tx_local_fault),
                .stat_tx_0_2_stat_tx_multicast(stat_tx_0_2_stat_tx_multicast),
                .stat_tx_0_2_stat_tx_packet_1024_1518_bytes(stat_tx_0_2_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_2_stat_tx_packet_128_255_bytes(stat_tx_0_2_stat_tx_packet_128_255_bytes),
                .stat_tx_0_2_stat_tx_packet_1519_1522_bytes(stat_tx_0_2_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_2_stat_tx_packet_1523_1548_bytes(stat_tx_0_2_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_2_stat_tx_packet_1549_2047_bytes(stat_tx_0_2_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_2_stat_tx_packet_2048_4095_bytes(stat_tx_0_2_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_2_stat_tx_packet_256_511_bytes(stat_tx_0_2_stat_tx_packet_256_511_bytes),
                .stat_tx_0_2_stat_tx_packet_4096_8191_bytes(stat_tx_0_2_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_2_stat_tx_packet_512_1023_bytes(stat_tx_0_2_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_2_stat_tx_packet_64_bytes(stat_tx_0_2_stat_tx_packet_64_bytes),
                .stat_tx_0_2_stat_tx_packet_65_127_bytes(stat_tx_0_2_stat_tx_packet_65_127_bytes),
                .stat_tx_0_2_stat_tx_packet_8192_9215_bytes(stat_tx_0_2_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_2_stat_tx_packet_large(stat_tx_0_2_stat_tx_packet_large),
                .stat_tx_0_2_stat_tx_packet_small(stat_tx_0_2_stat_tx_packet_small),
                .stat_tx_0_2_stat_tx_total_bytes(stat_tx_0_2_stat_tx_total_bytes),
                .stat_tx_0_2_stat_tx_total_good_bytes(stat_tx_0_2_stat_tx_total_good_bytes),
                .stat_tx_0_2_stat_tx_total_good_packets(stat_tx_0_2_stat_tx_total_good_packets),
                .stat_tx_0_2_stat_tx_total_packets(stat_tx_0_2_stat_tx_total_packets),
                .stat_tx_0_2_stat_tx_unicast(stat_tx_0_2_stat_tx_unicast),
                .stat_tx_0_2_stat_tx_vlan(stat_tx_0_2_stat_tx_vlan),
                .stat_tx_0_3_stat_tx_bad_fcs(stat_tx_0_3_stat_tx_bad_fcs),
                .stat_tx_0_3_stat_tx_broadcast(stat_tx_0_3_stat_tx_broadcast),
                .stat_tx_0_3_stat_tx_frame_error(stat_tx_0_3_stat_tx_frame_error),
                .stat_tx_0_3_stat_tx_local_fault(stat_tx_0_3_stat_tx_local_fault),
                .stat_tx_0_3_stat_tx_multicast(stat_tx_0_3_stat_tx_multicast),
                .stat_tx_0_3_stat_tx_packet_1024_1518_bytes(stat_tx_0_3_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_3_stat_tx_packet_128_255_bytes(stat_tx_0_3_stat_tx_packet_128_255_bytes),
                .stat_tx_0_3_stat_tx_packet_1519_1522_bytes(stat_tx_0_3_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_3_stat_tx_packet_1523_1548_bytes(stat_tx_0_3_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_3_stat_tx_packet_1549_2047_bytes(stat_tx_0_3_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_3_stat_tx_packet_2048_4095_bytes(stat_tx_0_3_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_3_stat_tx_packet_256_511_bytes(stat_tx_0_3_stat_tx_packet_256_511_bytes),
                .stat_tx_0_3_stat_tx_packet_4096_8191_bytes(stat_tx_0_3_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_3_stat_tx_packet_512_1023_bytes(stat_tx_0_3_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_3_stat_tx_packet_64_bytes(stat_tx_0_3_stat_tx_packet_64_bytes),
                .stat_tx_0_3_stat_tx_packet_65_127_bytes(stat_tx_0_3_stat_tx_packet_65_127_bytes),
                .stat_tx_0_3_stat_tx_packet_8192_9215_bytes(stat_tx_0_3_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_3_stat_tx_packet_large(stat_tx_0_3_stat_tx_packet_large),
                .stat_tx_0_3_stat_tx_packet_small(stat_tx_0_3_stat_tx_packet_small),
                .stat_tx_0_3_stat_tx_total_bytes(stat_tx_0_3_stat_tx_total_bytes),
                .stat_tx_0_3_stat_tx_total_good_bytes(stat_tx_0_3_stat_tx_total_good_bytes),
                .stat_tx_0_3_stat_tx_total_good_packets(stat_tx_0_3_stat_tx_total_good_packets),
                .stat_tx_0_3_stat_tx_total_packets(stat_tx_0_3_stat_tx_total_packets),
                .stat_tx_0_3_stat_tx_unicast(stat_tx_0_3_stat_tx_unicast),
                .stat_tx_0_3_stat_tx_vlan(stat_tx_0_3_stat_tx_vlan),
                .tx_core_clk_0_0(tx_core_clk_0_0),
                .tx_core_clk_0_1(tx_core_clk_0_1),
                .tx_core_clk_0_2(tx_core_clk_0_2),
                .tx_core_clk_0_3(tx_core_clk_0_3),
                .tx_preamblein_0_0(tx_preamblein_0_0),
                .tx_preamblein_0_1(tx_preamblein_0_1),
                .tx_preamblein_0_2(tx_preamblein_0_2),
                .tx_preamblein_0_3(tx_preamblein_0_3),
                .tx_reset_0_0(tx_reset_0_0),
                .tx_reset_0_1(tx_reset_0_1),
                .tx_reset_0_2(tx_reset_0_2),
                .tx_reset_0_3(tx_reset_0_3),
                .tx_resetdone_out_0_0(tx_resetdone_out_0_0),
                .tx_resetdone_out_0_1(tx_resetdone_out_0_1),
                .tx_resetdone_out_0_2(tx_resetdone_out_0_2),
                .tx_resetdone_out_0_3(tx_resetdone_out_0_3),
                .tx_unfout_0_0(tx_unfout_0_0),
                .tx_unfout_0_1(tx_unfout_0_1),
                .tx_unfout_0_2(tx_unfout_0_2),
                .tx_unfout_0_3(tx_unfout_0_3),
                .tx_usrclk_0(tx_usrclk_0),
                .tx_usrclk_1(tx_usrclk_1),
                .tx_usrclk_2(tx_usrclk_2),
                .tx_usrclk_3(tx_usrclk_3),
                .user_reg0_0_0(user_reg0_0_0),
                .user_reg0_0_1(user_reg0_0_1),
                .user_reg0_0_2(user_reg0_0_2),
                .user_reg0_0_3(user_reg0_0_3));
    end

    if (C_GT_MAC_ENABLE_RSFEC == 0) begin
        xbtest_sub_xxv_gt_0 xbtest_sub_xxv_gt_i
            (.FEC_stat_0_0_stat_fec_inc_cant_correct_count(FEC_stat_0_0_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_0_stat_fec_inc_correct_count(FEC_stat_0_0_stat_fec_inc_correct_count),
                .FEC_stat_0_0_stat_fec_lock_error(FEC_stat_0_0_stat_fec_lock_error),
                .FEC_stat_0_0_stat_fec_rx_lock(FEC_stat_0_0_stat_fec_rx_lock),
                .FEC_stat_0_1_stat_fec_inc_cant_correct_count(FEC_stat_0_1_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_1_stat_fec_inc_correct_count(FEC_stat_0_1_stat_fec_inc_correct_count),
                .FEC_stat_0_1_stat_fec_lock_error(FEC_stat_0_1_stat_fec_lock_error),
                .FEC_stat_0_1_stat_fec_rx_lock(FEC_stat_0_1_stat_fec_rx_lock),
                .FEC_stat_0_2_stat_fec_inc_cant_correct_count(FEC_stat_0_2_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_2_stat_fec_inc_correct_count(FEC_stat_0_2_stat_fec_inc_correct_count),
                .FEC_stat_0_2_stat_fec_lock_error(FEC_stat_0_2_stat_fec_lock_error),
                .FEC_stat_0_2_stat_fec_rx_lock(FEC_stat_0_2_stat_fec_rx_lock),
                .FEC_stat_0_3_stat_fec_inc_cant_correct_count(FEC_stat_0_3_stat_fec_inc_cant_correct_count),
                .FEC_stat_0_3_stat_fec_inc_correct_count(FEC_stat_0_3_stat_fec_inc_correct_count),
                .FEC_stat_0_3_stat_fec_lock_error(FEC_stat_0_3_stat_fec_lock_error),
                .FEC_stat_0_3_stat_fec_rx_lock(FEC_stat_0_3_stat_fec_rx_lock),
                .apb3clk_quad(apb3clk_quad),
                .axis_rx_0_0_tdata(axis_rx_0_0_tdata),
                .axis_rx_0_0_tkeep(axis_rx_0_0_tkeep),
                .axis_rx_0_0_tlast(axis_rx_0_0_tlast),
                .axis_rx_0_0_tuser(axis_rx_0_0_tuser),
                .axis_rx_0_0_tvalid(axis_rx_0_0_tvalid),
                .axis_rx_0_1_tdata(axis_rx_0_1_tdata),
                .axis_rx_0_1_tkeep(axis_rx_0_1_tkeep),
                .axis_rx_0_1_tlast(axis_rx_0_1_tlast),
                .axis_rx_0_1_tuser(axis_rx_0_1_tuser),
                .axis_rx_0_1_tvalid(axis_rx_0_1_tvalid),
                .axis_rx_0_2_tdata(axis_rx_0_2_tdata),
                .axis_rx_0_2_tkeep(axis_rx_0_2_tkeep),
                .axis_rx_0_2_tlast(axis_rx_0_2_tlast),
                .axis_rx_0_2_tuser(axis_rx_0_2_tuser),
                .axis_rx_0_2_tvalid(axis_rx_0_2_tvalid),
                .axis_rx_0_3_tdata(axis_rx_0_3_tdata),
                .axis_rx_0_3_tkeep(axis_rx_0_3_tkeep),
                .axis_rx_0_3_tlast(axis_rx_0_3_tlast),
                .axis_rx_0_3_tuser(axis_rx_0_3_tuser),
                .axis_rx_0_3_tvalid(axis_rx_0_3_tvalid),
                .axis_tx_0_0_tdata(axis_tx_0_0_tdata),
                .axis_tx_0_0_tkeep(axis_tx_0_0_tkeep),
                .axis_tx_0_0_tlast(axis_tx_0_0_tlast),
                .axis_tx_0_0_tready(axis_tx_0_0_tready),
                .axis_tx_0_0_tuser(axis_tx_0_0_tuser),
                .axis_tx_0_0_tvalid(axis_tx_0_0_tvalid),
                .axis_tx_0_1_tdata(axis_tx_0_1_tdata),
                .axis_tx_0_1_tkeep(axis_tx_0_1_tkeep),
                .axis_tx_0_1_tlast(axis_tx_0_1_tlast),
                .axis_tx_0_1_tready(axis_tx_0_1_tready),
                .axis_tx_0_1_tuser(axis_tx_0_1_tuser),
                .axis_tx_0_1_tvalid(axis_tx_0_1_tvalid),
                .axis_tx_0_2_tdata(axis_tx_0_2_tdata),
                .axis_tx_0_2_tkeep(axis_tx_0_2_tkeep),
                .axis_tx_0_2_tlast(axis_tx_0_2_tlast),
                .axis_tx_0_2_tready(axis_tx_0_2_tready),
                .axis_tx_0_2_tuser(axis_tx_0_2_tuser),
                .axis_tx_0_2_tvalid(axis_tx_0_2_tvalid),
                .axis_tx_0_3_tdata(axis_tx_0_3_tdata),
                .axis_tx_0_3_tkeep(axis_tx_0_3_tkeep),
                .axis_tx_0_3_tlast(axis_tx_0_3_tlast),
                .axis_tx_0_3_tready(axis_tx_0_3_tready),
                .axis_tx_0_3_tuser(axis_tx_0_3_tuser),
                .axis_tx_0_3_tvalid(axis_tx_0_3_tvalid),
                .ctl_tx_0_0_ctl_tx_send_idle(ctl_tx_0_0_ctl_tx_send_idle),
                .ctl_tx_0_0_ctl_tx_send_lfi(ctl_tx_0_0_ctl_tx_send_lfi),
                .ctl_tx_0_0_ctl_tx_send_rfi(ctl_tx_0_0_ctl_tx_send_rfi),
                .ctl_tx_0_1_ctl_tx_send_idle(ctl_tx_0_1_ctl_tx_send_idle),
                .ctl_tx_0_1_ctl_tx_send_lfi(ctl_tx_0_1_ctl_tx_send_lfi),
                .ctl_tx_0_1_ctl_tx_send_rfi(ctl_tx_0_1_ctl_tx_send_rfi),
                .ctl_tx_0_2_ctl_tx_send_idle(ctl_tx_0_2_ctl_tx_send_idle),
                .ctl_tx_0_2_ctl_tx_send_lfi(ctl_tx_0_2_ctl_tx_send_lfi),
                .ctl_tx_0_2_ctl_tx_send_rfi(ctl_tx_0_2_ctl_tx_send_rfi),
                .ctl_tx_0_3_ctl_tx_send_idle(ctl_tx_0_3_ctl_tx_send_idle),
                .ctl_tx_0_3_ctl_tx_send_lfi(ctl_tx_0_3_ctl_tx_send_lfi),
                .ctl_tx_0_3_ctl_tx_send_rfi(ctl_tx_0_3_ctl_tx_send_rfi),
                .gt_rxn_in_0(gt_rxn_in_0),
                .gt_rxp_in_0(gt_rxp_in_0),
                .gt_txn_out_0(gt_txn_out_0),
                .gt_txp_out_0(gt_txp_out_0),
                .gtwiz_reset_clk_freerun_in_0_0(gtwiz_reset_clk_freerun_in_0_0),
                .gtwiz_reset_clk_freerun_in_0_1(gtwiz_reset_clk_freerun_in_0_1),
                .gtwiz_reset_clk_freerun_in_0_2(gtwiz_reset_clk_freerun_in_0_2),
                .gtwiz_reset_clk_freerun_in_0_3(gtwiz_reset_clk_freerun_in_0_3),
                .pm_tick_0_0(pm_tick_0_0),
                .pm_tick_0_1(pm_tick_0_1),
                .pm_tick_0_2(pm_tick_0_2),
                .pm_tick_0_3(pm_tick_0_3),
                .gt_refclk_clk_n(gt_refclk_clk_n),
                .gt_refclk_clk_p(gt_refclk_clk_p),
                .rx_core_clk_0_0(rx_core_clk_0_0),
                .rx_core_clk_0_1(rx_core_clk_0_1),
                .rx_core_clk_0_2(rx_core_clk_0_2),
                .rx_core_clk_0_3(rx_core_clk_0_3),
                .rx_preambleout_0_0(rx_preambleout_0_0),
                .rx_preambleout_0_1(rx_preambleout_0_1),
                .rx_preambleout_0_2(rx_preambleout_0_2),
                .rx_preambleout_0_3(rx_preambleout_0_3),
                .rx_reset_0_0(rx_reset_0_0),
                .rx_reset_0_1(rx_reset_0_1),
                .rx_reset_0_2(rx_reset_0_2),
                .rx_reset_0_3(rx_reset_0_3),
                .rx_resetdone_out_0_0(rx_resetdone_out_0_0),
                .rx_resetdone_out_0_1(rx_resetdone_out_0_1),
                .rx_resetdone_out_0_2(rx_resetdone_out_0_2),
                .rx_resetdone_out_0_3(rx_resetdone_out_0_3),
                .rx_serdes_clk_0_0(rx_serdes_clk_0_0),
                .rx_serdes_clk_0_1(rx_serdes_clk_0_1),
                .rx_serdes_clk_0_2(rx_serdes_clk_0_2),
                .rx_serdes_clk_0_3(rx_serdes_clk_0_3),
                .rx_serdes_reset_0_0(rx_serdes_reset_0_0),
                .rx_serdes_reset_0_1(rx_serdes_reset_0_1),
                .rx_serdes_reset_0_2(rx_serdes_reset_0_2),
                .rx_serdes_reset_0_3(rx_serdes_reset_0_3),
                .rx_usrclk_0(rx_usrclk_0),
                .rx_usrclk_1(rx_usrclk_1),
                .rx_usrclk_2(rx_usrclk_2),
                .rx_usrclk_3(rx_usrclk_3),
                .s_axi_0_0_araddr(s_axi_0_0_araddr),
                .s_axi_0_0_arready(s_axi_0_0_arready),
                .s_axi_0_0_arvalid(s_axi_0_0_arvalid),
                .s_axi_0_0_awaddr(s_axi_0_0_awaddr),
                .s_axi_0_0_awready(s_axi_0_0_awready),
                .s_axi_0_0_awvalid(s_axi_0_0_awvalid),
                .s_axi_0_0_bready(s_axi_0_0_bready),
                .s_axi_0_0_bresp(s_axi_0_0_bresp),
                .s_axi_0_0_bvalid(s_axi_0_0_bvalid),
                .s_axi_0_0_rdata(s_axi_0_0_rdata),
                .s_axi_0_0_rready(s_axi_0_0_rready),
                .s_axi_0_0_rresp(s_axi_0_0_rresp),
                .s_axi_0_0_rvalid(s_axi_0_0_rvalid),
                .s_axi_0_0_wdata(s_axi_0_0_wdata),
                .s_axi_0_0_wready(s_axi_0_0_wready),
                .s_axi_0_0_wstrb(s_axi_0_0_wstrb),
                .s_axi_0_0_wvalid(s_axi_0_0_wvalid),
                .s_axi_0_1_araddr(s_axi_0_1_araddr),
                .s_axi_0_1_arready(s_axi_0_1_arready),
                .s_axi_0_1_arvalid(s_axi_0_1_arvalid),
                .s_axi_0_1_awaddr(s_axi_0_1_awaddr),
                .s_axi_0_1_awready(s_axi_0_1_awready),
                .s_axi_0_1_awvalid(s_axi_0_1_awvalid),
                .s_axi_0_1_bready(s_axi_0_1_bready),
                .s_axi_0_1_bresp(s_axi_0_1_bresp),
                .s_axi_0_1_bvalid(s_axi_0_1_bvalid),
                .s_axi_0_1_rdata(s_axi_0_1_rdata),
                .s_axi_0_1_rready(s_axi_0_1_rready),
                .s_axi_0_1_rresp(s_axi_0_1_rresp),
                .s_axi_0_1_rvalid(s_axi_0_1_rvalid),
                .s_axi_0_1_wdata(s_axi_0_1_wdata),
                .s_axi_0_1_wready(s_axi_0_1_wready),
                .s_axi_0_1_wstrb(s_axi_0_1_wstrb),
                .s_axi_0_1_wvalid(s_axi_0_1_wvalid),
                .s_axi_0_2_araddr(s_axi_0_2_araddr),
                .s_axi_0_2_arready(s_axi_0_2_arready),
                .s_axi_0_2_arvalid(s_axi_0_2_arvalid),
                .s_axi_0_2_awaddr(s_axi_0_2_awaddr),
                .s_axi_0_2_awready(s_axi_0_2_awready),
                .s_axi_0_2_awvalid(s_axi_0_2_awvalid),
                .s_axi_0_2_bready(s_axi_0_2_bready),
                .s_axi_0_2_bresp(s_axi_0_2_bresp),
                .s_axi_0_2_bvalid(s_axi_0_2_bvalid),
                .s_axi_0_2_rdata(s_axi_0_2_rdata),
                .s_axi_0_2_rready(s_axi_0_2_rready),
                .s_axi_0_2_rresp(s_axi_0_2_rresp),
                .s_axi_0_2_rvalid(s_axi_0_2_rvalid),
                .s_axi_0_2_wdata(s_axi_0_2_wdata),
                .s_axi_0_2_wready(s_axi_0_2_wready),
                .s_axi_0_2_wstrb(s_axi_0_2_wstrb),
                .s_axi_0_2_wvalid(s_axi_0_2_wvalid),
                .s_axi_0_3_araddr(s_axi_0_3_araddr),
                .s_axi_0_3_arready(s_axi_0_3_arready),
                .s_axi_0_3_arvalid(s_axi_0_3_arvalid),
                .s_axi_0_3_awaddr(s_axi_0_3_awaddr),
                .s_axi_0_3_awready(s_axi_0_3_awready),
                .s_axi_0_3_awvalid(s_axi_0_3_awvalid),
                .s_axi_0_3_bready(s_axi_0_3_bready),
                .s_axi_0_3_bresp(s_axi_0_3_bresp),
                .s_axi_0_3_bvalid(s_axi_0_3_bvalid),
                .s_axi_0_3_rdata(s_axi_0_3_rdata),
                .s_axi_0_3_rready(s_axi_0_3_rready),
                .s_axi_0_3_rresp(s_axi_0_3_rresp),
                .s_axi_0_3_rvalid(s_axi_0_3_rvalid),
                .s_axi_0_3_wdata(s_axi_0_3_wdata),
                .s_axi_0_3_wready(s_axi_0_3_wready),
                .s_axi_0_3_wstrb(s_axi_0_3_wstrb),
                .s_axi_0_3_wvalid(s_axi_0_3_wvalid),
                .s_axi_aclk_0_0(s_axi_aclk_0_0),
                .s_axi_aclk_0_1(s_axi_aclk_0_1),
                .s_axi_aclk_0_2(s_axi_aclk_0_2),
                .s_axi_aclk_0_3(s_axi_aclk_0_3),
                .s_axi_aresetn_0_0(s_axi_aresetn_0_0),
                .s_axi_aresetn_0_1(s_axi_aresetn_0_1),
                .s_axi_aresetn_0_2(s_axi_aresetn_0_2),
                .s_axi_aresetn_0_3(s_axi_aresetn_0_3),
                .stat_rx_0_0_stat_rx_bad_code(stat_rx_0_0_stat_rx_bad_code),
                .stat_rx_0_0_stat_rx_bad_fcs(stat_rx_0_0_stat_rx_bad_fcs),
                .stat_rx_0_0_stat_rx_bad_preamble(stat_rx_0_0_stat_rx_bad_preamble),
                .stat_rx_0_0_stat_rx_bad_sfd(stat_rx_0_0_stat_rx_bad_sfd),
                .stat_rx_0_0_stat_rx_block_lock(stat_rx_0_0_stat_rx_block_lock),
                .stat_rx_0_0_stat_rx_broadcast(stat_rx_0_0_stat_rx_broadcast),
                .stat_rx_0_0_stat_rx_fragment(stat_rx_0_0_stat_rx_fragment),
                .stat_rx_0_0_stat_rx_framing_err(stat_rx_0_0_stat_rx_framing_err),
                .stat_rx_0_0_stat_rx_framing_err_valid(stat_rx_0_0_stat_rx_framing_err_valid),
                .stat_rx_0_0_stat_rx_got_signal_os(stat_rx_0_0_stat_rx_got_signal_os),
                .stat_rx_0_0_stat_rx_hi_ber(stat_rx_0_0_stat_rx_hi_ber),
                .stat_rx_0_0_stat_rx_inrangeerr(stat_rx_0_0_stat_rx_inrangeerr),
                .stat_rx_0_0_stat_rx_internal_local_fault(stat_rx_0_0_stat_rx_internal_local_fault),
                .stat_rx_0_0_stat_rx_jabber(stat_rx_0_0_stat_rx_jabber),
                .stat_rx_0_0_stat_rx_local_fault(stat_rx_0_0_stat_rx_local_fault),
                .stat_rx_0_0_stat_rx_multicast(stat_rx_0_0_stat_rx_multicast),
                .stat_rx_0_0_stat_rx_oversize(stat_rx_0_0_stat_rx_oversize),
                .stat_rx_0_0_stat_rx_packet_1024_1518_bytes(stat_rx_0_0_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_0_stat_rx_packet_128_255_bytes(stat_rx_0_0_stat_rx_packet_128_255_bytes),
                .stat_rx_0_0_stat_rx_packet_1519_1522_bytes(stat_rx_0_0_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_0_stat_rx_packet_1523_1548_bytes(stat_rx_0_0_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_0_stat_rx_packet_1549_2047_bytes(stat_rx_0_0_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_0_stat_rx_packet_2048_4095_bytes(stat_rx_0_0_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_0_stat_rx_packet_256_511_bytes(stat_rx_0_0_stat_rx_packet_256_511_bytes),
                .stat_rx_0_0_stat_rx_packet_4096_8191_bytes(stat_rx_0_0_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_0_stat_rx_packet_512_1023_bytes(stat_rx_0_0_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_0_stat_rx_packet_64_bytes(stat_rx_0_0_stat_rx_packet_64_bytes),
                .stat_rx_0_0_stat_rx_packet_65_127_bytes(stat_rx_0_0_stat_rx_packet_65_127_bytes),
                .stat_rx_0_0_stat_rx_packet_8192_9215_bytes(stat_rx_0_0_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_0_stat_rx_packet_bad_fcs(stat_rx_0_0_stat_rx_packet_bad_fcs),
                .stat_rx_0_0_stat_rx_packet_large(stat_rx_0_0_stat_rx_packet_large),
                .stat_rx_0_0_stat_rx_packet_small(stat_rx_0_0_stat_rx_packet_small),
                .stat_rx_0_0_stat_rx_received_local_fault(stat_rx_0_0_stat_rx_received_local_fault),
                .stat_rx_0_0_stat_rx_remote_fault(stat_rx_0_0_stat_rx_remote_fault),
                .stat_rx_0_0_stat_rx_stomped_fcs(stat_rx_0_0_stat_rx_stomped_fcs),
                .stat_rx_0_0_stat_rx_test_pattern_mismatch(stat_rx_0_0_stat_rx_test_pattern_mismatch),
                .stat_rx_0_0_stat_rx_toolong(stat_rx_0_0_stat_rx_toolong),
                .stat_rx_0_0_stat_rx_total_bytes(stat_rx_0_0_stat_rx_total_bytes),
                .stat_rx_0_0_stat_rx_total_good_bytes(stat_rx_0_0_stat_rx_total_good_bytes),
                .stat_rx_0_0_stat_rx_total_good_packets(stat_rx_0_0_stat_rx_total_good_packets),
                .stat_rx_0_0_stat_rx_total_packets(stat_rx_0_0_stat_rx_total_packets),
                .stat_rx_0_0_stat_rx_truncated(stat_rx_0_0_stat_rx_truncated),
                .stat_rx_0_0_stat_rx_undersize(stat_rx_0_0_stat_rx_undersize),
                .stat_rx_0_0_stat_rx_unicast(stat_rx_0_0_stat_rx_unicast),
                .stat_rx_0_0_stat_rx_valid_ctrl_code(stat_rx_0_0_stat_rx_valid_ctrl_code),
                .stat_rx_0_0_stat_rx_vlan(stat_rx_0_0_stat_rx_vlan),
                .stat_rx_0_1_stat_rx_bad_code(stat_rx_0_1_stat_rx_bad_code),
                .stat_rx_0_1_stat_rx_bad_fcs(stat_rx_0_1_stat_rx_bad_fcs),
                .stat_rx_0_1_stat_rx_bad_preamble(stat_rx_0_1_stat_rx_bad_preamble),
                .stat_rx_0_1_stat_rx_bad_sfd(stat_rx_0_1_stat_rx_bad_sfd),
                .stat_rx_0_1_stat_rx_block_lock(stat_rx_0_1_stat_rx_block_lock),
                .stat_rx_0_1_stat_rx_broadcast(stat_rx_0_1_stat_rx_broadcast),
                .stat_rx_0_1_stat_rx_fragment(stat_rx_0_1_stat_rx_fragment),
                .stat_rx_0_1_stat_rx_framing_err(stat_rx_0_1_stat_rx_framing_err),
                .stat_rx_0_1_stat_rx_framing_err_valid(stat_rx_0_1_stat_rx_framing_err_valid),
                .stat_rx_0_1_stat_rx_got_signal_os(stat_rx_0_1_stat_rx_got_signal_os),
                .stat_rx_0_1_stat_rx_hi_ber(stat_rx_0_1_stat_rx_hi_ber),
                .stat_rx_0_1_stat_rx_inrangeerr(stat_rx_0_1_stat_rx_inrangeerr),
                .stat_rx_0_1_stat_rx_internal_local_fault(stat_rx_0_1_stat_rx_internal_local_fault),
                .stat_rx_0_1_stat_rx_jabber(stat_rx_0_1_stat_rx_jabber),
                .stat_rx_0_1_stat_rx_local_fault(stat_rx_0_1_stat_rx_local_fault),
                .stat_rx_0_1_stat_rx_multicast(stat_rx_0_1_stat_rx_multicast),
                .stat_rx_0_1_stat_rx_oversize(stat_rx_0_1_stat_rx_oversize),
                .stat_rx_0_1_stat_rx_packet_1024_1518_bytes(stat_rx_0_1_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_1_stat_rx_packet_128_255_bytes(stat_rx_0_1_stat_rx_packet_128_255_bytes),
                .stat_rx_0_1_stat_rx_packet_1519_1522_bytes(stat_rx_0_1_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_1_stat_rx_packet_1523_1548_bytes(stat_rx_0_1_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_1_stat_rx_packet_1549_2047_bytes(stat_rx_0_1_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_1_stat_rx_packet_2048_4095_bytes(stat_rx_0_1_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_1_stat_rx_packet_256_511_bytes(stat_rx_0_1_stat_rx_packet_256_511_bytes),
                .stat_rx_0_1_stat_rx_packet_4096_8191_bytes(stat_rx_0_1_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_1_stat_rx_packet_512_1023_bytes(stat_rx_0_1_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_1_stat_rx_packet_64_bytes(stat_rx_0_1_stat_rx_packet_64_bytes),
                .stat_rx_0_1_stat_rx_packet_65_127_bytes(stat_rx_0_1_stat_rx_packet_65_127_bytes),
                .stat_rx_0_1_stat_rx_packet_8192_9215_bytes(stat_rx_0_1_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_1_stat_rx_packet_bad_fcs(stat_rx_0_1_stat_rx_packet_bad_fcs),
                .stat_rx_0_1_stat_rx_packet_large(stat_rx_0_1_stat_rx_packet_large),
                .stat_rx_0_1_stat_rx_packet_small(stat_rx_0_1_stat_rx_packet_small),
                .stat_rx_0_1_stat_rx_received_local_fault(stat_rx_0_1_stat_rx_received_local_fault),
                .stat_rx_0_1_stat_rx_remote_fault(stat_rx_0_1_stat_rx_remote_fault),
                .stat_rx_0_1_stat_rx_stomped_fcs(stat_rx_0_1_stat_rx_stomped_fcs),
                .stat_rx_0_1_stat_rx_test_pattern_mismatch(stat_rx_0_1_stat_rx_test_pattern_mismatch),
                .stat_rx_0_1_stat_rx_toolong(stat_rx_0_1_stat_rx_toolong),
                .stat_rx_0_1_stat_rx_total_bytes(stat_rx_0_1_stat_rx_total_bytes),
                .stat_rx_0_1_stat_rx_total_good_bytes(stat_rx_0_1_stat_rx_total_good_bytes),
                .stat_rx_0_1_stat_rx_total_good_packets(stat_rx_0_1_stat_rx_total_good_packets),
                .stat_rx_0_1_stat_rx_total_packets(stat_rx_0_1_stat_rx_total_packets),
                .stat_rx_0_1_stat_rx_truncated(stat_rx_0_1_stat_rx_truncated),
                .stat_rx_0_1_stat_rx_undersize(stat_rx_0_1_stat_rx_undersize),
                .stat_rx_0_1_stat_rx_unicast(stat_rx_0_1_stat_rx_unicast),
                .stat_rx_0_1_stat_rx_valid_ctrl_code(stat_rx_0_1_stat_rx_valid_ctrl_code),
                .stat_rx_0_1_stat_rx_vlan(stat_rx_0_1_stat_rx_vlan),
                .stat_rx_0_2_stat_rx_bad_code(stat_rx_0_2_stat_rx_bad_code),
                .stat_rx_0_2_stat_rx_bad_fcs(stat_rx_0_2_stat_rx_bad_fcs),
                .stat_rx_0_2_stat_rx_bad_preamble(stat_rx_0_2_stat_rx_bad_preamble),
                .stat_rx_0_2_stat_rx_bad_sfd(stat_rx_0_2_stat_rx_bad_sfd),
                .stat_rx_0_2_stat_rx_block_lock(stat_rx_0_2_stat_rx_block_lock),
                .stat_rx_0_2_stat_rx_broadcast(stat_rx_0_2_stat_rx_broadcast),
                .stat_rx_0_2_stat_rx_fragment(stat_rx_0_2_stat_rx_fragment),
                .stat_rx_0_2_stat_rx_framing_err(stat_rx_0_2_stat_rx_framing_err),
                .stat_rx_0_2_stat_rx_framing_err_valid(stat_rx_0_2_stat_rx_framing_err_valid),
                .stat_rx_0_2_stat_rx_got_signal_os(stat_rx_0_2_stat_rx_got_signal_os),
                .stat_rx_0_2_stat_rx_hi_ber(stat_rx_0_2_stat_rx_hi_ber),
                .stat_rx_0_2_stat_rx_inrangeerr(stat_rx_0_2_stat_rx_inrangeerr),
                .stat_rx_0_2_stat_rx_internal_local_fault(stat_rx_0_2_stat_rx_internal_local_fault),
                .stat_rx_0_2_stat_rx_jabber(stat_rx_0_2_stat_rx_jabber),
                .stat_rx_0_2_stat_rx_local_fault(stat_rx_0_2_stat_rx_local_fault),
                .stat_rx_0_2_stat_rx_multicast(stat_rx_0_2_stat_rx_multicast),
                .stat_rx_0_2_stat_rx_oversize(stat_rx_0_2_stat_rx_oversize),
                .stat_rx_0_2_stat_rx_packet_1024_1518_bytes(stat_rx_0_2_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_2_stat_rx_packet_128_255_bytes(stat_rx_0_2_stat_rx_packet_128_255_bytes),
                .stat_rx_0_2_stat_rx_packet_1519_1522_bytes(stat_rx_0_2_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_2_stat_rx_packet_1523_1548_bytes(stat_rx_0_2_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_2_stat_rx_packet_1549_2047_bytes(stat_rx_0_2_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_2_stat_rx_packet_2048_4095_bytes(stat_rx_0_2_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_2_stat_rx_packet_256_511_bytes(stat_rx_0_2_stat_rx_packet_256_511_bytes),
                .stat_rx_0_2_stat_rx_packet_4096_8191_bytes(stat_rx_0_2_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_2_stat_rx_packet_512_1023_bytes(stat_rx_0_2_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_2_stat_rx_packet_64_bytes(stat_rx_0_2_stat_rx_packet_64_bytes),
                .stat_rx_0_2_stat_rx_packet_65_127_bytes(stat_rx_0_2_stat_rx_packet_65_127_bytes),
                .stat_rx_0_2_stat_rx_packet_8192_9215_bytes(stat_rx_0_2_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_2_stat_rx_packet_bad_fcs(stat_rx_0_2_stat_rx_packet_bad_fcs),
                .stat_rx_0_2_stat_rx_packet_large(stat_rx_0_2_stat_rx_packet_large),
                .stat_rx_0_2_stat_rx_packet_small(stat_rx_0_2_stat_rx_packet_small),
                .stat_rx_0_2_stat_rx_received_local_fault(stat_rx_0_2_stat_rx_received_local_fault),
                .stat_rx_0_2_stat_rx_remote_fault(stat_rx_0_2_stat_rx_remote_fault),
                .stat_rx_0_2_stat_rx_stomped_fcs(stat_rx_0_2_stat_rx_stomped_fcs),
                .stat_rx_0_2_stat_rx_test_pattern_mismatch(stat_rx_0_2_stat_rx_test_pattern_mismatch),
                .stat_rx_0_2_stat_rx_toolong(stat_rx_0_2_stat_rx_toolong),
                .stat_rx_0_2_stat_rx_total_bytes(stat_rx_0_2_stat_rx_total_bytes),
                .stat_rx_0_2_stat_rx_total_good_bytes(stat_rx_0_2_stat_rx_total_good_bytes),
                .stat_rx_0_2_stat_rx_total_good_packets(stat_rx_0_2_stat_rx_total_good_packets),
                .stat_rx_0_2_stat_rx_total_packets(stat_rx_0_2_stat_rx_total_packets),
                .stat_rx_0_2_stat_rx_truncated(stat_rx_0_2_stat_rx_truncated),
                .stat_rx_0_2_stat_rx_undersize(stat_rx_0_2_stat_rx_undersize),
                .stat_rx_0_2_stat_rx_unicast(stat_rx_0_2_stat_rx_unicast),
                .stat_rx_0_2_stat_rx_valid_ctrl_code(stat_rx_0_2_stat_rx_valid_ctrl_code),
                .stat_rx_0_2_stat_rx_vlan(stat_rx_0_2_stat_rx_vlan),
                .stat_rx_0_3_stat_rx_bad_code(stat_rx_0_3_stat_rx_bad_code),
                .stat_rx_0_3_stat_rx_bad_fcs(stat_rx_0_3_stat_rx_bad_fcs),
                .stat_rx_0_3_stat_rx_bad_preamble(stat_rx_0_3_stat_rx_bad_preamble),
                .stat_rx_0_3_stat_rx_bad_sfd(stat_rx_0_3_stat_rx_bad_sfd),
                .stat_rx_0_3_stat_rx_block_lock(stat_rx_0_3_stat_rx_block_lock),
                .stat_rx_0_3_stat_rx_broadcast(stat_rx_0_3_stat_rx_broadcast),
                .stat_rx_0_3_stat_rx_fragment(stat_rx_0_3_stat_rx_fragment),
                .stat_rx_0_3_stat_rx_framing_err(stat_rx_0_3_stat_rx_framing_err),
                .stat_rx_0_3_stat_rx_framing_err_valid(stat_rx_0_3_stat_rx_framing_err_valid),
                .stat_rx_0_3_stat_rx_got_signal_os(stat_rx_0_3_stat_rx_got_signal_os),
                .stat_rx_0_3_stat_rx_hi_ber(stat_rx_0_3_stat_rx_hi_ber),
                .stat_rx_0_3_stat_rx_inrangeerr(stat_rx_0_3_stat_rx_inrangeerr),
                .stat_rx_0_3_stat_rx_internal_local_fault(stat_rx_0_3_stat_rx_internal_local_fault),
                .stat_rx_0_3_stat_rx_jabber(stat_rx_0_3_stat_rx_jabber),
                .stat_rx_0_3_stat_rx_local_fault(stat_rx_0_3_stat_rx_local_fault),
                .stat_rx_0_3_stat_rx_multicast(stat_rx_0_3_stat_rx_multicast),
                .stat_rx_0_3_stat_rx_oversize(stat_rx_0_3_stat_rx_oversize),
                .stat_rx_0_3_stat_rx_packet_1024_1518_bytes(stat_rx_0_3_stat_rx_packet_1024_1518_bytes),
                .stat_rx_0_3_stat_rx_packet_128_255_bytes(stat_rx_0_3_stat_rx_packet_128_255_bytes),
                .stat_rx_0_3_stat_rx_packet_1519_1522_bytes(stat_rx_0_3_stat_rx_packet_1519_1522_bytes),
                .stat_rx_0_3_stat_rx_packet_1523_1548_bytes(stat_rx_0_3_stat_rx_packet_1523_1548_bytes),
                .stat_rx_0_3_stat_rx_packet_1549_2047_bytes(stat_rx_0_3_stat_rx_packet_1549_2047_bytes),
                .stat_rx_0_3_stat_rx_packet_2048_4095_bytes(stat_rx_0_3_stat_rx_packet_2048_4095_bytes),
                .stat_rx_0_3_stat_rx_packet_256_511_bytes(stat_rx_0_3_stat_rx_packet_256_511_bytes),
                .stat_rx_0_3_stat_rx_packet_4096_8191_bytes(stat_rx_0_3_stat_rx_packet_4096_8191_bytes),
                .stat_rx_0_3_stat_rx_packet_512_1023_bytes(stat_rx_0_3_stat_rx_packet_512_1023_bytes),
                .stat_rx_0_3_stat_rx_packet_64_bytes(stat_rx_0_3_stat_rx_packet_64_bytes),
                .stat_rx_0_3_stat_rx_packet_65_127_bytes(stat_rx_0_3_stat_rx_packet_65_127_bytes),
                .stat_rx_0_3_stat_rx_packet_8192_9215_bytes(stat_rx_0_3_stat_rx_packet_8192_9215_bytes),
                .stat_rx_0_3_stat_rx_packet_bad_fcs(stat_rx_0_3_stat_rx_packet_bad_fcs),
                .stat_rx_0_3_stat_rx_packet_large(stat_rx_0_3_stat_rx_packet_large),
                .stat_rx_0_3_stat_rx_packet_small(stat_rx_0_3_stat_rx_packet_small),
                .stat_rx_0_3_stat_rx_received_local_fault(stat_rx_0_3_stat_rx_received_local_fault),
                .stat_rx_0_3_stat_rx_remote_fault(stat_rx_0_3_stat_rx_remote_fault),
                .stat_rx_0_3_stat_rx_stomped_fcs(stat_rx_0_3_stat_rx_stomped_fcs),
                .stat_rx_0_3_stat_rx_test_pattern_mismatch(stat_rx_0_3_stat_rx_test_pattern_mismatch),
                .stat_rx_0_3_stat_rx_toolong(stat_rx_0_3_stat_rx_toolong),
                .stat_rx_0_3_stat_rx_total_bytes(stat_rx_0_3_stat_rx_total_bytes),
                .stat_rx_0_3_stat_rx_total_good_bytes(stat_rx_0_3_stat_rx_total_good_bytes),
                .stat_rx_0_3_stat_rx_total_good_packets(stat_rx_0_3_stat_rx_total_good_packets),
                .stat_rx_0_3_stat_rx_total_packets(stat_rx_0_3_stat_rx_total_packets),
                .stat_rx_0_3_stat_rx_truncated(stat_rx_0_3_stat_rx_truncated),
                .stat_rx_0_3_stat_rx_undersize(stat_rx_0_3_stat_rx_undersize),
                .stat_rx_0_3_stat_rx_unicast(stat_rx_0_3_stat_rx_unicast),
                .stat_rx_0_3_stat_rx_valid_ctrl_code(stat_rx_0_3_stat_rx_valid_ctrl_code),
                .stat_rx_0_3_stat_rx_vlan(stat_rx_0_3_stat_rx_vlan),
                .stat_rx_status_0_0(stat_rx_status_0_0),
                .stat_rx_status_0_1(stat_rx_status_0_1),
                .stat_rx_status_0_2(stat_rx_status_0_2),
                .stat_rx_status_0_3(stat_rx_status_0_3),
                .stat_tx_0_0_stat_tx_bad_fcs(stat_tx_0_0_stat_tx_bad_fcs),
                .stat_tx_0_0_stat_tx_broadcast(stat_tx_0_0_stat_tx_broadcast),
                .stat_tx_0_0_stat_tx_frame_error(stat_tx_0_0_stat_tx_frame_error),
                .stat_tx_0_0_stat_tx_local_fault(stat_tx_0_0_stat_tx_local_fault),
                .stat_tx_0_0_stat_tx_multicast(stat_tx_0_0_stat_tx_multicast),
                .stat_tx_0_0_stat_tx_packet_1024_1518_bytes(stat_tx_0_0_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_0_stat_tx_packet_128_255_bytes(stat_tx_0_0_stat_tx_packet_128_255_bytes),
                .stat_tx_0_0_stat_tx_packet_1519_1522_bytes(stat_tx_0_0_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_0_stat_tx_packet_1523_1548_bytes(stat_tx_0_0_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_0_stat_tx_packet_1549_2047_bytes(stat_tx_0_0_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_0_stat_tx_packet_2048_4095_bytes(stat_tx_0_0_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_0_stat_tx_packet_256_511_bytes(stat_tx_0_0_stat_tx_packet_256_511_bytes),
                .stat_tx_0_0_stat_tx_packet_4096_8191_bytes(stat_tx_0_0_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_0_stat_tx_packet_512_1023_bytes(stat_tx_0_0_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_0_stat_tx_packet_64_bytes(stat_tx_0_0_stat_tx_packet_64_bytes),
                .stat_tx_0_0_stat_tx_packet_65_127_bytes(stat_tx_0_0_stat_tx_packet_65_127_bytes),
                .stat_tx_0_0_stat_tx_packet_8192_9215_bytes(stat_tx_0_0_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_0_stat_tx_packet_large(stat_tx_0_0_stat_tx_packet_large),
                .stat_tx_0_0_stat_tx_packet_small(stat_tx_0_0_stat_tx_packet_small),
                .stat_tx_0_0_stat_tx_total_bytes(stat_tx_0_0_stat_tx_total_bytes),
                .stat_tx_0_0_stat_tx_total_good_bytes(stat_tx_0_0_stat_tx_total_good_bytes),
                .stat_tx_0_0_stat_tx_total_good_packets(stat_tx_0_0_stat_tx_total_good_packets),
                .stat_tx_0_0_stat_tx_total_packets(stat_tx_0_0_stat_tx_total_packets),
                .stat_tx_0_0_stat_tx_unicast(stat_tx_0_0_stat_tx_unicast),
                .stat_tx_0_0_stat_tx_vlan(stat_tx_0_0_stat_tx_vlan),
                .stat_tx_0_1_stat_tx_bad_fcs(stat_tx_0_1_stat_tx_bad_fcs),
                .stat_tx_0_1_stat_tx_broadcast(stat_tx_0_1_stat_tx_broadcast),
                .stat_tx_0_1_stat_tx_frame_error(stat_tx_0_1_stat_tx_frame_error),
                .stat_tx_0_1_stat_tx_local_fault(stat_tx_0_1_stat_tx_local_fault),
                .stat_tx_0_1_stat_tx_multicast(stat_tx_0_1_stat_tx_multicast),
                .stat_tx_0_1_stat_tx_packet_1024_1518_bytes(stat_tx_0_1_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_1_stat_tx_packet_128_255_bytes(stat_tx_0_1_stat_tx_packet_128_255_bytes),
                .stat_tx_0_1_stat_tx_packet_1519_1522_bytes(stat_tx_0_1_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_1_stat_tx_packet_1523_1548_bytes(stat_tx_0_1_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_1_stat_tx_packet_1549_2047_bytes(stat_tx_0_1_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_1_stat_tx_packet_2048_4095_bytes(stat_tx_0_1_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_1_stat_tx_packet_256_511_bytes(stat_tx_0_1_stat_tx_packet_256_511_bytes),
                .stat_tx_0_1_stat_tx_packet_4096_8191_bytes(stat_tx_0_1_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_1_stat_tx_packet_512_1023_bytes(stat_tx_0_1_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_1_stat_tx_packet_64_bytes(stat_tx_0_1_stat_tx_packet_64_bytes),
                .stat_tx_0_1_stat_tx_packet_65_127_bytes(stat_tx_0_1_stat_tx_packet_65_127_bytes),
                .stat_tx_0_1_stat_tx_packet_8192_9215_bytes(stat_tx_0_1_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_1_stat_tx_packet_large(stat_tx_0_1_stat_tx_packet_large),
                .stat_tx_0_1_stat_tx_packet_small(stat_tx_0_1_stat_tx_packet_small),
                .stat_tx_0_1_stat_tx_total_bytes(stat_tx_0_1_stat_tx_total_bytes),
                .stat_tx_0_1_stat_tx_total_good_bytes(stat_tx_0_1_stat_tx_total_good_bytes),
                .stat_tx_0_1_stat_tx_total_good_packets(stat_tx_0_1_stat_tx_total_good_packets),
                .stat_tx_0_1_stat_tx_total_packets(stat_tx_0_1_stat_tx_total_packets),
                .stat_tx_0_1_stat_tx_unicast(stat_tx_0_1_stat_tx_unicast),
                .stat_tx_0_1_stat_tx_vlan(stat_tx_0_1_stat_tx_vlan),
                .stat_tx_0_2_stat_tx_bad_fcs(stat_tx_0_2_stat_tx_bad_fcs),
                .stat_tx_0_2_stat_tx_broadcast(stat_tx_0_2_stat_tx_broadcast),
                .stat_tx_0_2_stat_tx_frame_error(stat_tx_0_2_stat_tx_frame_error),
                .stat_tx_0_2_stat_tx_local_fault(stat_tx_0_2_stat_tx_local_fault),
                .stat_tx_0_2_stat_tx_multicast(stat_tx_0_2_stat_tx_multicast),
                .stat_tx_0_2_stat_tx_packet_1024_1518_bytes(stat_tx_0_2_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_2_stat_tx_packet_128_255_bytes(stat_tx_0_2_stat_tx_packet_128_255_bytes),
                .stat_tx_0_2_stat_tx_packet_1519_1522_bytes(stat_tx_0_2_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_2_stat_tx_packet_1523_1548_bytes(stat_tx_0_2_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_2_stat_tx_packet_1549_2047_bytes(stat_tx_0_2_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_2_stat_tx_packet_2048_4095_bytes(stat_tx_0_2_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_2_stat_tx_packet_256_511_bytes(stat_tx_0_2_stat_tx_packet_256_511_bytes),
                .stat_tx_0_2_stat_tx_packet_4096_8191_bytes(stat_tx_0_2_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_2_stat_tx_packet_512_1023_bytes(stat_tx_0_2_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_2_stat_tx_packet_64_bytes(stat_tx_0_2_stat_tx_packet_64_bytes),
                .stat_tx_0_2_stat_tx_packet_65_127_bytes(stat_tx_0_2_stat_tx_packet_65_127_bytes),
                .stat_tx_0_2_stat_tx_packet_8192_9215_bytes(stat_tx_0_2_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_2_stat_tx_packet_large(stat_tx_0_2_stat_tx_packet_large),
                .stat_tx_0_2_stat_tx_packet_small(stat_tx_0_2_stat_tx_packet_small),
                .stat_tx_0_2_stat_tx_total_bytes(stat_tx_0_2_stat_tx_total_bytes),
                .stat_tx_0_2_stat_tx_total_good_bytes(stat_tx_0_2_stat_tx_total_good_bytes),
                .stat_tx_0_2_stat_tx_total_good_packets(stat_tx_0_2_stat_tx_total_good_packets),
                .stat_tx_0_2_stat_tx_total_packets(stat_tx_0_2_stat_tx_total_packets),
                .stat_tx_0_2_stat_tx_unicast(stat_tx_0_2_stat_tx_unicast),
                .stat_tx_0_2_stat_tx_vlan(stat_tx_0_2_stat_tx_vlan),
                .stat_tx_0_3_stat_tx_bad_fcs(stat_tx_0_3_stat_tx_bad_fcs),
                .stat_tx_0_3_stat_tx_broadcast(stat_tx_0_3_stat_tx_broadcast),
                .stat_tx_0_3_stat_tx_frame_error(stat_tx_0_3_stat_tx_frame_error),
                .stat_tx_0_3_stat_tx_local_fault(stat_tx_0_3_stat_tx_local_fault),
                .stat_tx_0_3_stat_tx_multicast(stat_tx_0_3_stat_tx_multicast),
                .stat_tx_0_3_stat_tx_packet_1024_1518_bytes(stat_tx_0_3_stat_tx_packet_1024_1518_bytes),
                .stat_tx_0_3_stat_tx_packet_128_255_bytes(stat_tx_0_3_stat_tx_packet_128_255_bytes),
                .stat_tx_0_3_stat_tx_packet_1519_1522_bytes(stat_tx_0_3_stat_tx_packet_1519_1522_bytes),
                .stat_tx_0_3_stat_tx_packet_1523_1548_bytes(stat_tx_0_3_stat_tx_packet_1523_1548_bytes),
                .stat_tx_0_3_stat_tx_packet_1549_2047_bytes(stat_tx_0_3_stat_tx_packet_1549_2047_bytes),
                .stat_tx_0_3_stat_tx_packet_2048_4095_bytes(stat_tx_0_3_stat_tx_packet_2048_4095_bytes),
                .stat_tx_0_3_stat_tx_packet_256_511_bytes(stat_tx_0_3_stat_tx_packet_256_511_bytes),
                .stat_tx_0_3_stat_tx_packet_4096_8191_bytes(stat_tx_0_3_stat_tx_packet_4096_8191_bytes),
                .stat_tx_0_3_stat_tx_packet_512_1023_bytes(stat_tx_0_3_stat_tx_packet_512_1023_bytes),
                .stat_tx_0_3_stat_tx_packet_64_bytes(stat_tx_0_3_stat_tx_packet_64_bytes),
                .stat_tx_0_3_stat_tx_packet_65_127_bytes(stat_tx_0_3_stat_tx_packet_65_127_bytes),
                .stat_tx_0_3_stat_tx_packet_8192_9215_bytes(stat_tx_0_3_stat_tx_packet_8192_9215_bytes),
                .stat_tx_0_3_stat_tx_packet_large(stat_tx_0_3_stat_tx_packet_large),
                .stat_tx_0_3_stat_tx_packet_small(stat_tx_0_3_stat_tx_packet_small),
                .stat_tx_0_3_stat_tx_total_bytes(stat_tx_0_3_stat_tx_total_bytes),
                .stat_tx_0_3_stat_tx_total_good_bytes(stat_tx_0_3_stat_tx_total_good_bytes),
                .stat_tx_0_3_stat_tx_total_good_packets(stat_tx_0_3_stat_tx_total_good_packets),
                .stat_tx_0_3_stat_tx_total_packets(stat_tx_0_3_stat_tx_total_packets),
                .stat_tx_0_3_stat_tx_unicast(stat_tx_0_3_stat_tx_unicast),
                .stat_tx_0_3_stat_tx_vlan(stat_tx_0_3_stat_tx_vlan),
                .tx_core_clk_0_0(tx_core_clk_0_0),
                .tx_core_clk_0_1(tx_core_clk_0_1),
                .tx_core_clk_0_2(tx_core_clk_0_2),
                .tx_core_clk_0_3(tx_core_clk_0_3),
                .tx_preamblein_0_0(tx_preamblein_0_0),
                .tx_preamblein_0_1(tx_preamblein_0_1),
                .tx_preamblein_0_2(tx_preamblein_0_2),
                .tx_preamblein_0_3(tx_preamblein_0_3),
                .tx_reset_0_0(tx_reset_0_0),
                .tx_reset_0_1(tx_reset_0_1),
                .tx_reset_0_2(tx_reset_0_2),
                .tx_reset_0_3(tx_reset_0_3),
                .tx_resetdone_out_0_0(tx_resetdone_out_0_0),
                .tx_resetdone_out_0_1(tx_resetdone_out_0_1),
                .tx_resetdone_out_0_2(tx_resetdone_out_0_2),
                .tx_resetdone_out_0_3(tx_resetdone_out_0_3),
                .tx_unfout_0_0(tx_unfout_0_0),
                .tx_unfout_0_1(tx_unfout_0_1),
                .tx_unfout_0_2(tx_unfout_0_2),
                .tx_unfout_0_3(tx_unfout_0_3),
                .tx_usrclk_0(tx_usrclk_0),
                .tx_usrclk_1(tx_usrclk_1),
                .tx_usrclk_2(tx_usrclk_2),
                .tx_usrclk_3(tx_usrclk_3),
                .user_reg0_0_0(user_reg0_0_0),
                .user_reg0_0_1(user_reg0_0_1),
                .user_reg0_0_2(user_reg0_0_2),
                .user_reg0_0_3(user_reg0_0_3));

    end
endgenerate


endmodule
