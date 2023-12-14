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

// this module wraps the exdes functionalities by using the same naming convention than the other XXV IP (when GT is included into the XXV)

module xbtest_sub_xxv_gt_wrapper #(
    parameter integer DEST_SYNC_FF = 4,
    parameter integer C_GT_MAC_ENABLE_RSFEC = 0
) (
//  // GT Signals
    gt_rxp_in,
    gt_rxn_in,
    gt_txp_out,
    gt_txn_out,


    tx_clk_out_0,
    rx_core_clk_0,
    rx_clk_out_0,

//  // AXI4-Lite_0 Signals
    s_axi_aclk_0,
    s_axi_aresetn_0,
    s_axi_awaddr_0,
    s_axi_awvalid_0,
    s_axi_awready_0,
    s_axi_wdata_0,
    s_axi_wstrb_0,
    s_axi_wvalid_0,
    s_axi_wready_0,
    s_axi_bresp_0,
    s_axi_bvalid_0,
    s_axi_bready_0,
    s_axi_araddr_0,
    s_axi_arvalid_0,
    s_axi_arready_0,
    s_axi_rdata_0,
    s_axi_rresp_0,
    s_axi_rvalid_0,
    s_axi_rready_0,
    pm_tick_0,
//  // RX_0 Signals
    rx_reset_0,
    user_rx_reset_0,
    rxrecclkout_0,
//  // RX_0 User Interface  Signals
    rx_axis_tvalid_0,
    rx_axis_tdata_0,
    rx_axis_tlast_0,
    rx_axis_tkeep_0,
    rx_axis_tuser_0,
    rx_preambleout_0,


//  // RX_0 Control Signals



//  // RX_0 Stats Signals
    stat_rx_block_lock_0,
    stat_rx_framing_err_valid_0,
    stat_rx_framing_err_0,
    stat_rx_hi_ber_0,
    stat_rx_valid_ctrl_code_0,
    stat_rx_bad_code_0,
    stat_rx_total_packets_0,
    stat_rx_total_good_packets_0,
    stat_rx_total_bytes_0,
    stat_rx_total_good_bytes_0,
    stat_rx_packet_small_0,
    stat_rx_jabber_0,
    stat_rx_packet_large_0,
    stat_rx_oversize_0,
    stat_rx_undersize_0,
    stat_rx_toolong_0,
    stat_rx_fragment_0,
    stat_rx_packet_64_bytes_0,
    stat_rx_packet_65_127_bytes_0,
    stat_rx_packet_128_255_bytes_0,
    stat_rx_packet_256_511_bytes_0,
    stat_rx_packet_512_1023_bytes_0,
    stat_rx_packet_1024_1518_bytes_0,
    stat_rx_packet_1519_1522_bytes_0,
    stat_rx_packet_1523_1548_bytes_0,
    stat_rx_bad_fcs_0,
    stat_rx_packet_bad_fcs_0,
    stat_rx_stomped_fcs_0,
    stat_rx_packet_1549_2047_bytes_0,
    stat_rx_packet_2048_4095_bytes_0,
    stat_rx_packet_4096_8191_bytes_0,
    stat_rx_packet_8192_9215_bytes_0,
    stat_rx_unicast_0,
    stat_rx_multicast_0,
    stat_rx_broadcast_0,
    stat_rx_vlan_0,
    stat_rx_inrangeerr_0,
    stat_rx_bad_preamble_0,
    stat_rx_bad_sfd_0,
    stat_rx_got_signal_os_0,
    stat_rx_test_pattern_mismatch_0,
    stat_rx_truncated_0,
    stat_rx_local_fault_0,
    stat_rx_remote_fault_0,
    stat_rx_internal_local_fault_0,
    stat_rx_received_local_fault_0,
    stat_rx_status_0,



//  // TX_0 Signals
    tx_reset_0,
    user_tx_reset_0,

//  // TX_0 User Interface  Signals
    tx_axis_tready_0,
    tx_axis_tvalid_0,
    tx_axis_tdata_0,
    tx_axis_tlast_0,
    tx_axis_tkeep_0,
    tx_axis_tuser_0,
    tx_unfout_0,
    tx_preamblein_0,

//  // TX_0 Control Signals
    ctl_tx_send_lfi_0,
    ctl_tx_send_rfi_0,
    ctl_tx_send_idle_0,


//  // TX_0 Stats Signals
    stat_tx_total_packets_0,
    stat_tx_total_bytes_0,
    stat_tx_total_good_packets_0,
    stat_tx_total_good_bytes_0,
    stat_tx_packet_64_bytes_0,
    stat_tx_packet_65_127_bytes_0,
    stat_tx_packet_128_255_bytes_0,
    stat_tx_packet_256_511_bytes_0,
    stat_tx_packet_512_1023_bytes_0,
    stat_tx_packet_1024_1518_bytes_0,
    stat_tx_packet_1519_1522_bytes_0,
    stat_tx_packet_1523_1548_bytes_0,
    stat_tx_packet_small_0,
    stat_tx_packet_large_0,
    stat_tx_packet_1549_2047_bytes_0,
    stat_tx_packet_2048_4095_bytes_0,
    stat_tx_packet_4096_8191_bytes_0,
    stat_tx_packet_8192_9215_bytes_0,
    stat_tx_unicast_0,
    stat_tx_multicast_0,
    stat_tx_broadcast_0,
    stat_tx_vlan_0,
    stat_tx_bad_fcs_0,
    stat_tx_frame_error_0,
    stat_tx_local_fault_0,

    user_reg0_0,


//  // RS-FEC Stats Signals
    stat_rx_rsfec_hi_ser_0,
    stat_rx_rsfec_lane_alignment_status_0,
    stat_rx_rsfec_corrected_cw_inc_0,
    stat_rx_rsfec_uncorrected_cw_inc_0,
    stat_rx_rsfec_err_count0_inc_0,
    stat_tx_rsfec_lane_alignment_status_0,


//  // GT Debug Signals
    gt_dmonitorout_0,
    gt_eyescandataerror_0,
    gt_eyescanreset_0,
    gt_eyescantrigger_0,
    gt_pcsrsvdin_0,
    gt_rxbufreset_0,
    gt_rxbufstatus_0,
    gt_rxcdrhold_0,
    gt_rxcommadeten_0,
    gt_rxdfeagchold_0,
    gt_rxdfelpmreset_0,
    gt_rxlatclk_0,
    gt_rxlpmen_0,
    gt_rxpcsreset_0,
    gt_rxpmareset_0,
    gt_rxpolarity_0,
    gt_rxprbscntreset_0,
    gt_rxprbserr_0,
    gt_rxprbssel_0,
    gt_rxrate_0,
    gt_rxslide_in_0,
    gt_rxstartofseq_0,
    gt_txbufstatus_0,
    gt_txinhibit_0,
    gt_txlatclk_0,
    gt_txmaincursor_0,
    gt_txpcsreset_0,
    gt_txpmareset_0,
    gt_txpolarity_0,
    gt_txpostcursor_0,
    gt_txprbsforceerr_0,
    gt_txelecidle_0,
    gt_txprbssel_0,
    gt_txprecursor_0,
    gt_txdiffctrl_0,
//  // GT DRP Signals
    gt_drpdo_0,
    gt_drprdy_0,
    gt_drpen_0,
    gt_drpwe_0,
    gt_drpaddr_0,
    gt_drpdi_0,
    gt_drpclk_0,
    gt_drprst_0,

    gt_drp_done_0,

    ctl_rx_rate_10g_25gn_0,
    txpllclksel_in_0,
    rxpllclksel_in_0,
    txsysclksel_in_0,
    rxsysclksel_in_0,

    axi_ctl_core_mode_switch_0,
    rxdfecfokfcnum_in_0,
    rxafecfoken_in_0,
    gtwiz_reset_tx_datapath_0,
    gtwiz_reset_rx_datapath_0,
    gtpowergood_out_0,
    txoutclksel_in_0,
    rxoutclksel_in_0,


    tx_clk_out_1,
    rx_core_clk_1,
    rx_clk_out_1,

//  // AXI4-Lite_1 Signals
    s_axi_aclk_1,
    s_axi_aresetn_1,
    s_axi_awaddr_1,
    s_axi_awvalid_1,
    s_axi_awready_1,
    s_axi_wdata_1,
    s_axi_wstrb_1,
    s_axi_wvalid_1,
    s_axi_wready_1,
    s_axi_bresp_1,
    s_axi_bvalid_1,
    s_axi_bready_1,
    s_axi_araddr_1,
    s_axi_arvalid_1,
    s_axi_arready_1,
    s_axi_rdata_1,
    s_axi_rresp_1,
    s_axi_rvalid_1,
    s_axi_rready_1,
    pm_tick_1,
//  // RX_1 Signals
    rx_reset_1,
    user_rx_reset_1,
    rxrecclkout_1,
//  // RX_1 User Interface  Signals
    rx_axis_tvalid_1,
    rx_axis_tdata_1,
    rx_axis_tlast_1,
    rx_axis_tkeep_1,
    rx_axis_tuser_1,
    rx_preambleout_1,


//  // RX_1 Control Signals



//  // RX_1 Stats Signals
    stat_rx_block_lock_1,
    stat_rx_framing_err_valid_1,
    stat_rx_framing_err_1,
    stat_rx_hi_ber_1,
    stat_rx_valid_ctrl_code_1,
    stat_rx_bad_code_1,
    stat_rx_total_packets_1,
    stat_rx_total_good_packets_1,
    stat_rx_total_bytes_1,
    stat_rx_total_good_bytes_1,
    stat_rx_packet_small_1,
    stat_rx_jabber_1,
    stat_rx_packet_large_1,
    stat_rx_oversize_1,
    stat_rx_undersize_1,
    stat_rx_toolong_1,
    stat_rx_fragment_1,
    stat_rx_packet_64_bytes_1,
    stat_rx_packet_65_127_bytes_1,
    stat_rx_packet_128_255_bytes_1,
    stat_rx_packet_256_511_bytes_1,
    stat_rx_packet_512_1023_bytes_1,
    stat_rx_packet_1024_1518_bytes_1,
    stat_rx_packet_1519_1522_bytes_1,
    stat_rx_packet_1523_1548_bytes_1,
    stat_rx_bad_fcs_1,
    stat_rx_packet_bad_fcs_1,
    stat_rx_stomped_fcs_1,
    stat_rx_packet_1549_2047_bytes_1,
    stat_rx_packet_2048_4095_bytes_1,
    stat_rx_packet_4096_8191_bytes_1,
    stat_rx_packet_8192_9215_bytes_1,
    stat_rx_unicast_1,
    stat_rx_multicast_1,
    stat_rx_broadcast_1,
    stat_rx_vlan_1,
    stat_rx_inrangeerr_1,
    stat_rx_bad_preamble_1,
    stat_rx_bad_sfd_1,
    stat_rx_got_signal_os_1,
    stat_rx_test_pattern_mismatch_1,
    stat_rx_truncated_1,
    stat_rx_local_fault_1,
    stat_rx_remote_fault_1,
    stat_rx_internal_local_fault_1,
    stat_rx_received_local_fault_1,
    stat_rx_status_1,



//  // TX_1 Signals
    tx_reset_1,
    user_tx_reset_1,

//  // TX_1 User Interface  Signals
    tx_axis_tready_1,
    tx_axis_tvalid_1,
    tx_axis_tdata_1,
    tx_axis_tlast_1,
    tx_axis_tkeep_1,
    tx_axis_tuser_1,
    tx_unfout_1,
    tx_preamblein_1,

//  // TX_1 Control Signals
    ctl_tx_send_lfi_1,
    ctl_tx_send_rfi_1,
    ctl_tx_send_idle_1,


//  // TX_1 Stats Signals
    stat_tx_total_packets_1,
    stat_tx_total_bytes_1,
    stat_tx_total_good_packets_1,
    stat_tx_total_good_bytes_1,
    stat_tx_packet_64_bytes_1,
    stat_tx_packet_65_127_bytes_1,
    stat_tx_packet_128_255_bytes_1,
    stat_tx_packet_256_511_bytes_1,
    stat_tx_packet_512_1023_bytes_1,
    stat_tx_packet_1024_1518_bytes_1,
    stat_tx_packet_1519_1522_bytes_1,
    stat_tx_packet_1523_1548_bytes_1,
    stat_tx_packet_small_1,
    stat_tx_packet_large_1,
    stat_tx_packet_1549_2047_bytes_1,
    stat_tx_packet_2048_4095_bytes_1,
    stat_tx_packet_4096_8191_bytes_1,
    stat_tx_packet_8192_9215_bytes_1,
    stat_tx_unicast_1,
    stat_tx_multicast_1,
    stat_tx_broadcast_1,
    stat_tx_vlan_1,
    stat_tx_bad_fcs_1,
    stat_tx_frame_error_1,
    stat_tx_local_fault_1,


    user_reg0_1,



//  // RS-FEC Stats Signals
    stat_rx_rsfec_hi_ser_1,
    stat_rx_rsfec_lane_alignment_status_1,
    stat_rx_rsfec_corrected_cw_inc_1,
    stat_rx_rsfec_uncorrected_cw_inc_1,
    stat_rx_rsfec_err_count0_inc_1,
    stat_tx_rsfec_lane_alignment_status_1,


//  // GT Debug Signals
    gt_dmonitorout_1,
    gt_eyescandataerror_1,
    gt_eyescanreset_1,
    gt_eyescantrigger_1,
    gt_pcsrsvdin_1,
    gt_rxbufreset_1,
    gt_rxbufstatus_1,
    gt_rxcdrhold_1,
    gt_rxcommadeten_1,
    gt_rxdfeagchold_1,
    gt_rxdfelpmreset_1,
    gt_rxlatclk_1,
    gt_rxlpmen_1,
    gt_rxpcsreset_1,
    gt_rxpmareset_1,
    gt_rxpolarity_1,
    gt_rxprbscntreset_1,
    gt_rxprbserr_1,
    gt_rxprbssel_1,
    gt_rxrate_1,
    gt_rxslide_in_1,
    gt_rxstartofseq_1,
    gt_txbufstatus_1,
    gt_txinhibit_1,
    gt_txlatclk_1,
    gt_txmaincursor_1,
    gt_txpcsreset_1,
    gt_txpmareset_1,
    gt_txpolarity_1,
    gt_txpostcursor_1,
    gt_txprbsforceerr_1,
    gt_txelecidle_1,
    gt_txprbssel_1,
    gt_txprecursor_1,
    gt_txdiffctrl_1,
//  // GT DRP Signals
    gt_drpdo_1,
    gt_drprdy_1,
    gt_drpen_1,
    gt_drpwe_1,
    gt_drpaddr_1,
    gt_drpdi_1,
    gt_drpclk_1,
    gt_drprst_1,

    gt_drp_done_1,

    ctl_rx_rate_10g_25gn_1,
    txpllclksel_in_1,
    rxpllclksel_in_1,
    txsysclksel_in_1,
    rxsysclksel_in_1,

    axi_ctl_core_mode_switch_1,
    rxdfecfokfcnum_in_1,
    rxafecfoken_in_1,
    gtwiz_reset_tx_datapath_1,
    gtwiz_reset_rx_datapath_1,
    gtpowergood_out_1,
    txoutclksel_in_1,
    rxoutclksel_in_1,


    tx_clk_out_2,
    rx_core_clk_2,
    rx_clk_out_2,

//  // AXI4-Lite_2 Signals
    s_axi_aclk_2,
    s_axi_aresetn_2,
    s_axi_awaddr_2,
    s_axi_awvalid_2,
    s_axi_awready_2,
    s_axi_wdata_2,
    s_axi_wstrb_2,
    s_axi_wvalid_2,
    s_axi_wready_2,
    s_axi_bresp_2,
    s_axi_bvalid_2,
    s_axi_bready_2,
    s_axi_araddr_2,
    s_axi_arvalid_2,
    s_axi_arready_2,
    s_axi_rdata_2,
    s_axi_rresp_2,
    s_axi_rvalid_2,
    s_axi_rready_2,
    pm_tick_2,
//  // RX_2 Signals
    rx_reset_2,
    user_rx_reset_2,
    rxrecclkout_2,
//  // RX_2 User Interface  Signals
    rx_axis_tvalid_2,
    rx_axis_tdata_2,
    rx_axis_tlast_2,
    rx_axis_tkeep_2,
    rx_axis_tuser_2,
    rx_preambleout_2,


//  // RX_2 Control Signals



//  // RX_2 Stats Signals
    stat_rx_block_lock_2,
    stat_rx_framing_err_valid_2,
    stat_rx_framing_err_2,
    stat_rx_hi_ber_2,
    stat_rx_valid_ctrl_code_2,
    stat_rx_bad_code_2,
    stat_rx_total_packets_2,
    stat_rx_total_good_packets_2,
    stat_rx_total_bytes_2,
    stat_rx_total_good_bytes_2,
    stat_rx_packet_small_2,
    stat_rx_jabber_2,
    stat_rx_packet_large_2,
    stat_rx_oversize_2,
    stat_rx_undersize_2,
    stat_rx_toolong_2,
    stat_rx_fragment_2,
    stat_rx_packet_64_bytes_2,
    stat_rx_packet_65_127_bytes_2,
    stat_rx_packet_128_255_bytes_2,
    stat_rx_packet_256_511_bytes_2,
    stat_rx_packet_512_1023_bytes_2,
    stat_rx_packet_1024_1518_bytes_2,
    stat_rx_packet_1519_1522_bytes_2,
    stat_rx_packet_1523_1548_bytes_2,
    stat_rx_bad_fcs_2,
    stat_rx_packet_bad_fcs_2,
    stat_rx_stomped_fcs_2,
    stat_rx_packet_1549_2047_bytes_2,
    stat_rx_packet_2048_4095_bytes_2,
    stat_rx_packet_4096_8191_bytes_2,
    stat_rx_packet_8192_9215_bytes_2,
    stat_rx_unicast_2,
    stat_rx_multicast_2,
    stat_rx_broadcast_2,
    stat_rx_vlan_2,
    stat_rx_inrangeerr_2,
    stat_rx_bad_preamble_2,
    stat_rx_bad_sfd_2,
    stat_rx_got_signal_os_2,
    stat_rx_test_pattern_mismatch_2,
    stat_rx_truncated_2,
    stat_rx_local_fault_2,
    stat_rx_remote_fault_2,
    stat_rx_internal_local_fault_2,
    stat_rx_received_local_fault_2,
    stat_rx_status_2,



//  // TX_2 Signals
    tx_reset_2,
    user_tx_reset_2,

//  // TX_2 User Interface  Signals
    tx_axis_tready_2,
    tx_axis_tvalid_2,
    tx_axis_tdata_2,
    tx_axis_tlast_2,
    tx_axis_tkeep_2,
    tx_axis_tuser_2,
    tx_unfout_2,
    tx_preamblein_2,

//  // TX_2 Control Signals
    ctl_tx_send_lfi_2,
    ctl_tx_send_rfi_2,
    ctl_tx_send_idle_2,


//  // TX_2 Stats Signals
    stat_tx_total_packets_2,
    stat_tx_total_bytes_2,
    stat_tx_total_good_packets_2,
    stat_tx_total_good_bytes_2,
    stat_tx_packet_64_bytes_2,
    stat_tx_packet_65_127_bytes_2,
    stat_tx_packet_128_255_bytes_2,
    stat_tx_packet_256_511_bytes_2,
    stat_tx_packet_512_1023_bytes_2,
    stat_tx_packet_1024_1518_bytes_2,
    stat_tx_packet_1519_1522_bytes_2,
    stat_tx_packet_1523_1548_bytes_2,
    stat_tx_packet_small_2,
    stat_tx_packet_large_2,
    stat_tx_packet_1549_2047_bytes_2,
    stat_tx_packet_2048_4095_bytes_2,
    stat_tx_packet_4096_8191_bytes_2,
    stat_tx_packet_8192_9215_bytes_2,
    stat_tx_unicast_2,
    stat_tx_multicast_2,
    stat_tx_broadcast_2,
    stat_tx_vlan_2,
    stat_tx_bad_fcs_2,
    stat_tx_frame_error_2,
    stat_tx_local_fault_2,


    user_reg0_2,



//  // RS-FEC Stats Signals
    stat_rx_rsfec_hi_ser_2,
    stat_rx_rsfec_lane_alignment_status_2,
    stat_rx_rsfec_corrected_cw_inc_2,
    stat_rx_rsfec_uncorrected_cw_inc_2,
    stat_rx_rsfec_err_count0_inc_2,
    stat_tx_rsfec_lane_alignment_status_2,


//  // GT Debug Signals
    gt_dmonitorout_2,
    gt_eyescandataerror_2,
    gt_eyescanreset_2,
    gt_eyescantrigger_2,
    gt_pcsrsvdin_2,
    gt_rxbufreset_2,
    gt_rxbufstatus_2,
    gt_rxcdrhold_2,
    gt_rxcommadeten_2,
    gt_rxdfeagchold_2,
    gt_rxdfelpmreset_2,
    gt_rxlatclk_2,
    gt_rxlpmen_2,
    gt_rxpcsreset_2,
    gt_rxpmareset_2,
    gt_rxpolarity_2,
    gt_rxprbscntreset_2,
    gt_rxprbserr_2,
    gt_rxprbssel_2,
    gt_rxrate_2,
    gt_rxslide_in_2,
    gt_rxstartofseq_2,
    gt_txbufstatus_2,
    gt_txinhibit_2,
    gt_txlatclk_2,
    gt_txmaincursor_2,
    gt_txpcsreset_2,
    gt_txpmareset_2,
    gt_txpolarity_2,
    gt_txpostcursor_2,
    gt_txprbsforceerr_2,
    gt_txelecidle_2,
    gt_txprbssel_2,
    gt_txprecursor_2,
    gt_txdiffctrl_2,
//  // GT DRP Signals
    gt_drpdo_2,
    gt_drprdy_2,
    gt_drpen_2,
    gt_drpwe_2,
    gt_drpaddr_2,
    gt_drpdi_2,
    gt_drpclk_2,
    gt_drprst_2,

    gt_drp_done_2,

    ctl_rx_rate_10g_25gn_2,
    txpllclksel_in_2,
    rxpllclksel_in_2,
    txsysclksel_in_2,
    rxsysclksel_in_2,

    axi_ctl_core_mode_switch_2,
    rxdfecfokfcnum_in_2,
    rxafecfoken_in_2,
    gtwiz_reset_tx_datapath_2,
    gtwiz_reset_rx_datapath_2,
    gtpowergood_out_2,
    txoutclksel_in_2,
    rxoutclksel_in_2,


    tx_clk_out_3,
    rx_core_clk_3,
    rx_clk_out_3,

//  // AXI4-Lite_3 Signals
    s_axi_aclk_3,
    s_axi_aresetn_3,
    s_axi_awaddr_3,
    s_axi_awvalid_3,
    s_axi_awready_3,
    s_axi_wdata_3,
    s_axi_wstrb_3,
    s_axi_wvalid_3,
    s_axi_wready_3,
    s_axi_bresp_3,
    s_axi_bvalid_3,
    s_axi_bready_3,
    s_axi_araddr_3,
    s_axi_arvalid_3,
    s_axi_arready_3,
    s_axi_rdata_3,
    s_axi_rresp_3,
    s_axi_rvalid_3,
    s_axi_rready_3,
    pm_tick_3,
//  // RX_3 Signals
    rx_reset_3,
    user_rx_reset_3,
    rxrecclkout_3,
//  // RX_3 User Interface  Signals
    rx_axis_tvalid_3,
    rx_axis_tdata_3,
    rx_axis_tlast_3,
    rx_axis_tkeep_3,
    rx_axis_tuser_3,
    rx_preambleout_3,


//  // RX_3 Control Signals



//  // RX_3 Stats Signals
    stat_rx_block_lock_3,
    stat_rx_framing_err_valid_3,
    stat_rx_framing_err_3,
    stat_rx_hi_ber_3,
    stat_rx_valid_ctrl_code_3,
    stat_rx_bad_code_3,
    stat_rx_total_packets_3,
    stat_rx_total_good_packets_3,
    stat_rx_total_bytes_3,
    stat_rx_total_good_bytes_3,
    stat_rx_packet_small_3,
    stat_rx_jabber_3,
    stat_rx_packet_large_3,
    stat_rx_oversize_3,
    stat_rx_undersize_3,
    stat_rx_toolong_3,
    stat_rx_fragment_3,
    stat_rx_packet_64_bytes_3,
    stat_rx_packet_65_127_bytes_3,
    stat_rx_packet_128_255_bytes_3,
    stat_rx_packet_256_511_bytes_3,
    stat_rx_packet_512_1023_bytes_3,
    stat_rx_packet_1024_1518_bytes_3,
    stat_rx_packet_1519_1522_bytes_3,
    stat_rx_packet_1523_1548_bytes_3,
    stat_rx_bad_fcs_3,
    stat_rx_packet_bad_fcs_3,
    stat_rx_stomped_fcs_3,
    stat_rx_packet_1549_2047_bytes_3,
    stat_rx_packet_2048_4095_bytes_3,
    stat_rx_packet_4096_8191_bytes_3,
    stat_rx_packet_8192_9215_bytes_3,
    stat_rx_unicast_3,
    stat_rx_multicast_3,
    stat_rx_broadcast_3,
    stat_rx_vlan_3,
    stat_rx_inrangeerr_3,
    stat_rx_bad_preamble_3,
    stat_rx_bad_sfd_3,
    stat_rx_got_signal_os_3,
    stat_rx_test_pattern_mismatch_3,
    stat_rx_truncated_3,
    stat_rx_local_fault_3,
    stat_rx_remote_fault_3,
    stat_rx_internal_local_fault_3,
    stat_rx_received_local_fault_3,
    stat_rx_status_3,



//  // TX_3 Signals
    tx_reset_3,
    user_tx_reset_3,

//  // TX_3 User Interface  Signals
    tx_axis_tready_3,
    tx_axis_tvalid_3,
    tx_axis_tdata_3,
    tx_axis_tlast_3,
    tx_axis_tkeep_3,
    tx_axis_tuser_3,
    tx_unfout_3,
    tx_preamblein_3,

//  // TX_3 Control Signals
    ctl_tx_send_lfi_3,
    ctl_tx_send_rfi_3,
    ctl_tx_send_idle_3,


//  // TX_3 Stats Signals
    stat_tx_total_packets_3,
    stat_tx_total_bytes_3,
    stat_tx_total_good_packets_3,
    stat_tx_total_good_bytes_3,
    stat_tx_packet_64_bytes_3,
    stat_tx_packet_65_127_bytes_3,
    stat_tx_packet_128_255_bytes_3,
    stat_tx_packet_256_511_bytes_3,
    stat_tx_packet_512_1023_bytes_3,
    stat_tx_packet_1024_1518_bytes_3,
    stat_tx_packet_1519_1522_bytes_3,
    stat_tx_packet_1523_1548_bytes_3,
    stat_tx_packet_small_3,
    stat_tx_packet_large_3,
    stat_tx_packet_1549_2047_bytes_3,
    stat_tx_packet_2048_4095_bytes_3,
    stat_tx_packet_4096_8191_bytes_3,
    stat_tx_packet_8192_9215_bytes_3,
    stat_tx_unicast_3,
    stat_tx_multicast_3,
    stat_tx_broadcast_3,
    stat_tx_vlan_3,
    stat_tx_bad_fcs_3,
    stat_tx_frame_error_3,
    stat_tx_local_fault_3,


    user_reg0_3,



//  // RS-FEC Stats Signals
    stat_rx_rsfec_hi_ser_3,
    stat_rx_rsfec_lane_alignment_status_3,
    stat_rx_rsfec_corrected_cw_inc_3,
    stat_rx_rsfec_uncorrected_cw_inc_3,
    stat_rx_rsfec_err_count0_inc_3,
    stat_tx_rsfec_lane_alignment_status_3,


//  // GT Debug Signals
    gt_dmonitorout_3,
    gt_eyescandataerror_3,
    gt_eyescanreset_3,
    gt_eyescantrigger_3,
    gt_pcsrsvdin_3,
    gt_rxbufreset_3,
    gt_rxbufstatus_3,
    gt_rxcdrhold_3,
    gt_rxcommadeten_3,
    gt_rxdfeagchold_3,
    gt_rxdfelpmreset_3,
    gt_rxlatclk_3,
    gt_rxlpmen_3,
    gt_rxpcsreset_3,
    gt_rxpmareset_3,
    gt_rxpolarity_3,
    gt_rxprbscntreset_3,
    gt_rxprbserr_3,
    gt_rxprbssel_3,
    gt_rxrate_3,
    gt_rxslide_in_3,
    gt_rxstartofseq_3,
    gt_txbufstatus_3,
    gt_txinhibit_3,
    gt_txlatclk_3,
    gt_txmaincursor_3,
    gt_txpcsreset_3,
    gt_txpmareset_3,
    gt_txpolarity_3,
    gt_txpostcursor_3,
    gt_txprbsforceerr_3,
    gt_txelecidle_3,
    gt_txprbssel_3,
    gt_txprecursor_3,
    gt_txdiffctrl_3,
//  // GT DRP Signals
    gt_drpdo_3,
    gt_drprdy_3,
    gt_drpen_3,
    gt_drpwe_3,
    gt_drpaddr_3,
    gt_drpdi_3,
    gt_drpclk_3,
    gt_drprst_3,

    gt_drp_done_3,

    ctl_rx_rate_10g_25gn_3,
    txpllclksel_in_3,
    rxpllclksel_in_3,
    txsysclksel_in_3,
    rxsysclksel_in_3,

    axi_ctl_core_mode_switch_3,
    rxdfecfokfcnum_in_3,
    rxafecfoken_in_3,
    gtwiz_reset_tx_datapath_3,
    gtwiz_reset_rx_datapath_3,
    gtpowergood_out_3,
    txoutclksel_in_3,
    rxoutclksel_in_3,

    gt_refclk_p,
    gt_refclk_n,
    gt_refclk_out,
    qpllreset_in_0,
    sys_reset,
    dclk
);

    input  wire [3 : 0] gt_rxp_in;
    input  wire [3 : 0] gt_rxn_in;
    output wire [3 : 0] gt_txp_out;
    output wire [3 : 0] gt_txn_out;
    output wire tx_clk_out_0;
    input  wire rx_core_clk_0;
    output wire rx_clk_out_0;

//  // AXI_0 interface ports
    input  wire s_axi_aclk_0;
    input  wire s_axi_aresetn_0;
    input  wire [31:0] s_axi_awaddr_0;
    input  wire s_axi_awvalid_0;
    output wire s_axi_awready_0;
    input  wire [31:0] s_axi_wdata_0;
    input  wire [3:0] s_axi_wstrb_0;
    input  wire s_axi_wvalid_0;
    output wire s_axi_wready_0;
    output wire [1:0] s_axi_bresp_0;
    output wire s_axi_bvalid_0;
    input  wire s_axi_bready_0;
    input  wire [31:0] s_axi_araddr_0;
    input  wire s_axi_arvalid_0;
    output wire s_axi_arready_0;
    output wire [31:0] s_axi_rdata_0;
    output wire [1:0] s_axi_rresp_0;
    output wire s_axi_rvalid_0;
    input  wire s_axi_rready_0;
    input  wire pm_tick_0;
//  // RX_0 Signals
    input  wire rx_reset_0;
    output wire user_rx_reset_0;
    output wire rxrecclkout_0;
//  // RX_0 User Interface Signals
    output wire rx_axis_tvalid_0;
    output wire [63:0] rx_axis_tdata_0;
    output wire rx_axis_tlast_0;
    output wire [7:0] rx_axis_tkeep_0;
    output wire rx_axis_tuser_0;
    output wire [55:0] rx_preambleout_0;


    input  wire ctl_rx_rate_10g_25gn_0;
    input  wire gt_drp_done_0;
    input  wire [1:0] txpllclksel_in_0;
    input  wire [1:0] rxpllclksel_in_0;
    input  wire [1:0] txsysclksel_in_0;
    input  wire [1:0] rxsysclksel_in_0;

    output  wire axi_ctl_core_mode_switch_0;
    input  wire [3:0] rxdfecfokfcnum_in_0;
    input  wire rxafecfoken_in_0;
//  // RX_0 Control Signals



//  // RX_0 Stats Signals
    output wire stat_rx_block_lock_0;
    output wire stat_rx_framing_err_valid_0;
    output wire stat_rx_framing_err_0;
    output wire stat_rx_hi_ber_0;
    output wire stat_rx_valid_ctrl_code_0;
    output wire stat_rx_bad_code_0;
    output wire [1:0] stat_rx_total_packets_0;
    output wire stat_rx_total_good_packets_0;
    output wire [3:0] stat_rx_total_bytes_0;
    output wire [13:0] stat_rx_total_good_bytes_0;
    output wire stat_rx_packet_small_0;
    output wire stat_rx_jabber_0;
    output wire stat_rx_packet_large_0;
    output wire stat_rx_oversize_0;
    output wire stat_rx_undersize_0;
    output wire stat_rx_toolong_0;
    output wire stat_rx_fragment_0;
    output wire stat_rx_packet_64_bytes_0;
    output wire stat_rx_packet_65_127_bytes_0;
    output wire stat_rx_packet_128_255_bytes_0;
    output wire stat_rx_packet_256_511_bytes_0;
    output wire stat_rx_packet_512_1023_bytes_0;
    output wire stat_rx_packet_1024_1518_bytes_0;
    output wire stat_rx_packet_1519_1522_bytes_0;
    output wire stat_rx_packet_1523_1548_bytes_0;
    output wire [1:0] stat_rx_bad_fcs_0;
    output wire stat_rx_packet_bad_fcs_0;
    output wire [1:0] stat_rx_stomped_fcs_0;
    output wire stat_rx_packet_1549_2047_bytes_0;
    output wire stat_rx_packet_2048_4095_bytes_0;
    output wire stat_rx_packet_4096_8191_bytes_0;
    output wire stat_rx_packet_8192_9215_bytes_0;
    output wire stat_rx_unicast_0;
    output wire stat_rx_multicast_0;
    output wire stat_rx_broadcast_0;
    output wire stat_rx_vlan_0;
    output wire stat_rx_inrangeerr_0;
    output wire stat_rx_bad_preamble_0;
    output wire stat_rx_bad_sfd_0;
    output wire stat_rx_got_signal_os_0;
    output wire stat_rx_test_pattern_mismatch_0;
    output wire stat_rx_truncated_0;
    output wire stat_rx_local_fault_0;
    output wire stat_rx_remote_fault_0;
    output wire stat_rx_internal_local_fault_0;
    output wire stat_rx_received_local_fault_0;
    output wire  stat_rx_status_0;


//  // TX_0 Signals
    input  wire tx_reset_0;
    output wire user_tx_reset_0;

//  // TX_0 User Interface Signals
    output wire tx_axis_tready_0;
    input  wire tx_axis_tvalid_0;
    input  wire [63:0] tx_axis_tdata_0;
    input  wire tx_axis_tlast_0;
    input  wire [7:0] tx_axis_tkeep_0;
    input  wire tx_axis_tuser_0;
    output wire tx_unfout_0;
    input  wire [55:0] tx_preamblein_0;

//  // TX_0 Control Signals
    input  wire ctl_tx_send_lfi_0;
    input  wire ctl_tx_send_rfi_0;
    input  wire ctl_tx_send_idle_0;


//  // TX_0 Stats Signals
    output wire stat_tx_total_packets_0;
    output wire [3:0] stat_tx_total_bytes_0;
    output wire stat_tx_total_good_packets_0;
    output wire [13:0] stat_tx_total_good_bytes_0;
    output wire stat_tx_packet_64_bytes_0;
    output wire stat_tx_packet_65_127_bytes_0;
    output wire stat_tx_packet_128_255_bytes_0;
    output wire stat_tx_packet_256_511_bytes_0;
    output wire stat_tx_packet_512_1023_bytes_0;
    output wire stat_tx_packet_1024_1518_bytes_0;
    output wire stat_tx_packet_1519_1522_bytes_0;
    output wire stat_tx_packet_1523_1548_bytes_0;
    output wire stat_tx_packet_small_0;
    output wire stat_tx_packet_large_0;
    output wire stat_tx_packet_1549_2047_bytes_0;
    output wire stat_tx_packet_2048_4095_bytes_0;
    output wire stat_tx_packet_4096_8191_bytes_0;
    output wire stat_tx_packet_8192_9215_bytes_0;
    output wire stat_tx_unicast_0;
    output wire stat_tx_multicast_0;
    output wire stat_tx_broadcast_0;
    output wire stat_tx_vlan_0;
    output wire stat_tx_bad_fcs_0;
    output wire stat_tx_frame_error_0;
    output wire stat_tx_local_fault_0;


    output wire [31:0]  user_reg0_0;


//  // FEC Stats Signals
    output wire stat_rx_rsfec_hi_ser_0;
    output wire stat_rx_rsfec_lane_alignment_status_0;
    output wire stat_rx_rsfec_corrected_cw_inc_0;
    output wire stat_rx_rsfec_uncorrected_cw_inc_0;
    output wire [2:0] stat_rx_rsfec_err_count0_inc_0;
    output wire stat_tx_rsfec_lane_alignment_status_0;

//  // GT Debug interface ports
    output wire [16:0] gt_dmonitorout_0;
    output wire [0:0] gt_eyescandataerror_0;
    input  wire [0:0] gt_eyescanreset_0;
    input  wire [0:0] gt_eyescantrigger_0;
    input  wire [15:0] gt_pcsrsvdin_0;
    input  wire [0:0] gt_rxbufreset_0;
    output wire [2:0] gt_rxbufstatus_0;
    input  wire [0:0] gt_rxcdrhold_0;
    input  wire [0:0] gt_rxcommadeten_0;
    input  wire [0:0] gt_rxdfeagchold_0;
    input  wire [0:0] gt_rxdfelpmreset_0;
    input  wire [0:0] gt_rxlatclk_0;
    input  wire [0:0] gt_rxlpmen_0;
    input  wire [0:0] gt_rxpcsreset_0;
    input  wire [0:0] gt_rxpmareset_0;
    input  wire [0:0] gt_rxpolarity_0;
    input  wire [0:0] gt_rxprbscntreset_0;
    output wire [0:0] gt_rxprbserr_0;
    input  wire [3:0] gt_rxprbssel_0;
    input  wire [2:0] gt_rxrate_0;
    input  wire [0:0] gt_rxslide_in_0;
    output wire [1:0] gt_rxstartofseq_0;
    output wire [1:0] gt_txbufstatus_0;
    input  wire [0:0] gt_txinhibit_0;
    input  wire [0:0] gt_txlatclk_0;
    input  wire [6:0] gt_txmaincursor_0;
    input  wire [0:0] gt_txpcsreset_0;
    input  wire [0:0] gt_txpmareset_0;
    input  wire [0:0] gt_txpolarity_0;
    input  wire [4:0] gt_txpostcursor_0;
    input  wire [0:0] gt_txprbsforceerr_0;
    input  wire [0:0] gt_txelecidle_0;
    input  wire [3:0] gt_txprbssel_0;
    input  wire [4:0] gt_txprecursor_0;
    input wire [4:0] gt_txdiffctrl_0;
//  // GT DRP interface ports
    output wire [15:0] gt_drpdo_0;
    output wire [0:0] gt_drprdy_0;
    input  wire [0:0] gt_drpen_0;
    input  wire [0:0] gt_drpwe_0;
    input  wire [9:0] gt_drpaddr_0;
    input  wire [15:0] gt_drpdi_0;
    input wire gt_drpclk_0;
    input wire gt_drprst_0;
    input wire gtwiz_reset_tx_datapath_0;
    input wire gtwiz_reset_rx_datapath_0;
    output wire gtpowergood_out_0;
    input wire [2:0] txoutclksel_in_0;
    input wire [2:0] rxoutclksel_in_0;

    output wire tx_clk_out_1;
    input  wire rx_core_clk_1;
    output wire rx_clk_out_1;

//  // AXI_1 interface ports
    input  wire s_axi_aclk_1;
    input  wire s_axi_aresetn_1;
    input  wire [31:0] s_axi_awaddr_1;
    input  wire s_axi_awvalid_1;
    output wire s_axi_awready_1;
    input  wire [31:0] s_axi_wdata_1;
    input  wire [3:0] s_axi_wstrb_1;
    input  wire s_axi_wvalid_1;
    output wire s_axi_wready_1;
    output wire [1:0] s_axi_bresp_1;
    output wire s_axi_bvalid_1;
    input  wire s_axi_bready_1;
    input  wire [31:0] s_axi_araddr_1;
    input  wire s_axi_arvalid_1;
    output wire s_axi_arready_1;
    output wire [31:0] s_axi_rdata_1;
    output wire [1:0] s_axi_rresp_1;
    output wire s_axi_rvalid_1;
    input  wire s_axi_rready_1;
    input  wire pm_tick_1;
//  // RX_1 Signals
    input  wire rx_reset_1;
    output wire user_rx_reset_1;
    output wire rxrecclkout_1;
//  // RX_1 User Interface Signals
    output wire rx_axis_tvalid_1;
    output wire [63:0] rx_axis_tdata_1;
    output wire rx_axis_tlast_1;
    output wire [7:0] rx_axis_tkeep_1;
    output wire rx_axis_tuser_1;
    output wire [55:0] rx_preambleout_1;




    input  wire ctl_rx_rate_10g_25gn_1;
    input  wire gt_drp_done_1;
    input  wire [1:0] txpllclksel_in_1;
    input  wire [1:0] rxpllclksel_in_1;
    input  wire [1:0] txsysclksel_in_1;
    input  wire [1:0] rxsysclksel_in_1;

    output  wire axi_ctl_core_mode_switch_1;
    input  wire [3:0] rxdfecfokfcnum_in_1;
    input  wire rxafecfoken_in_1;
//  // RX_1 Control Signals



//  // RX_1 Stats Signals
    output wire stat_rx_block_lock_1;
    output wire stat_rx_framing_err_valid_1;
    output wire stat_rx_framing_err_1;
    output wire stat_rx_hi_ber_1;
    output wire stat_rx_valid_ctrl_code_1;
    output wire stat_rx_bad_code_1;
    output wire [1:0] stat_rx_total_packets_1;
    output wire stat_rx_total_good_packets_1;
    output wire [3:0] stat_rx_total_bytes_1;
    output wire [13:0] stat_rx_total_good_bytes_1;
    output wire stat_rx_packet_small_1;
    output wire stat_rx_jabber_1;
    output wire stat_rx_packet_large_1;
    output wire stat_rx_oversize_1;
    output wire stat_rx_undersize_1;
    output wire stat_rx_toolong_1;
    output wire stat_rx_fragment_1;
    output wire stat_rx_packet_64_bytes_1;
    output wire stat_rx_packet_65_127_bytes_1;
    output wire stat_rx_packet_128_255_bytes_1;
    output wire stat_rx_packet_256_511_bytes_1;
    output wire stat_rx_packet_512_1023_bytes_1;
    output wire stat_rx_packet_1024_1518_bytes_1;
    output wire stat_rx_packet_1519_1522_bytes_1;
    output wire stat_rx_packet_1523_1548_bytes_1;
    output wire [1:0] stat_rx_bad_fcs_1;
    output wire stat_rx_packet_bad_fcs_1;
    output wire [1:0] stat_rx_stomped_fcs_1;
    output wire stat_rx_packet_1549_2047_bytes_1;
    output wire stat_rx_packet_2048_4095_bytes_1;
    output wire stat_rx_packet_4096_8191_bytes_1;
    output wire stat_rx_packet_8192_9215_bytes_1;
    output wire stat_rx_unicast_1;
    output wire stat_rx_multicast_1;
    output wire stat_rx_broadcast_1;
    output wire stat_rx_vlan_1;
    output wire stat_rx_inrangeerr_1;
    output wire stat_rx_bad_preamble_1;
    output wire stat_rx_bad_sfd_1;
    output wire stat_rx_got_signal_os_1;
    output wire stat_rx_test_pattern_mismatch_1;
    output wire stat_rx_truncated_1;
    output wire stat_rx_local_fault_1;
    output wire stat_rx_remote_fault_1;
    output wire stat_rx_internal_local_fault_1;
    output wire stat_rx_received_local_fault_1;
    output wire  stat_rx_status_1;


//  // TX_1 Signals
    input  wire tx_reset_1;
    output wire user_tx_reset_1;

//  // TX_1 User Interface Signals
    output wire tx_axis_tready_1;
    input  wire tx_axis_tvalid_1;
    input  wire [63:0] tx_axis_tdata_1;
    input  wire tx_axis_tlast_1;
    input  wire [7:0] tx_axis_tkeep_1;
    input  wire tx_axis_tuser_1;
    output wire tx_unfout_1;
    input  wire [55:0] tx_preamblein_1;

//  // TX_1 Control Signals
    input  wire ctl_tx_send_lfi_1;
    input  wire ctl_tx_send_rfi_1;
    input  wire ctl_tx_send_idle_1;


//  // TX_1 Stats Signals
    output wire stat_tx_total_packets_1;
    output wire [3:0] stat_tx_total_bytes_1;
    output wire stat_tx_total_good_packets_1;
    output wire [13:0] stat_tx_total_good_bytes_1;
    output wire stat_tx_packet_64_bytes_1;
    output wire stat_tx_packet_65_127_bytes_1;
    output wire stat_tx_packet_128_255_bytes_1;
    output wire stat_tx_packet_256_511_bytes_1;
    output wire stat_tx_packet_512_1023_bytes_1;
    output wire stat_tx_packet_1024_1518_bytes_1;
    output wire stat_tx_packet_1519_1522_bytes_1;
    output wire stat_tx_packet_1523_1548_bytes_1;
    output wire stat_tx_packet_small_1;
    output wire stat_tx_packet_large_1;
    output wire stat_tx_packet_1549_2047_bytes_1;
    output wire stat_tx_packet_2048_4095_bytes_1;
    output wire stat_tx_packet_4096_8191_bytes_1;
    output wire stat_tx_packet_8192_9215_bytes_1;
    output wire stat_tx_unicast_1;
    output wire stat_tx_multicast_1;
    output wire stat_tx_broadcast_1;
    output wire stat_tx_vlan_1;
    output wire stat_tx_bad_fcs_1;
    output wire stat_tx_frame_error_1;
    output wire stat_tx_local_fault_1;


    output wire [31:0]  user_reg0_1;


//  // FEC Stats Signals
    output wire stat_rx_rsfec_hi_ser_1;
    output wire stat_rx_rsfec_lane_alignment_status_1;
    output wire stat_rx_rsfec_corrected_cw_inc_1;
    output wire stat_rx_rsfec_uncorrected_cw_inc_1;
    output wire [2:0] stat_rx_rsfec_err_count0_inc_1;
    output wire stat_tx_rsfec_lane_alignment_status_1;

//  // GT Debug interface ports
    output wire [16:0] gt_dmonitorout_1;
    output wire [0:0] gt_eyescandataerror_1;
    input  wire [0:0] gt_eyescanreset_1;
    input  wire [0:0] gt_eyescantrigger_1;
    input  wire [15:0] gt_pcsrsvdin_1;
    input  wire [0:0] gt_rxbufreset_1;
    output wire [2:0] gt_rxbufstatus_1;
    input  wire [0:0] gt_rxcdrhold_1;
    input  wire [0:0] gt_rxcommadeten_1;
    input  wire [0:0] gt_rxdfeagchold_1;
    input  wire [0:0] gt_rxdfelpmreset_1;
    input  wire [0:0] gt_rxlatclk_1;
    input  wire [0:0] gt_rxlpmen_1;
    input  wire [0:0] gt_rxpcsreset_1;
    input  wire [0:0] gt_rxpmareset_1;
    input  wire [0:0] gt_rxpolarity_1;
    input  wire [0:0] gt_rxprbscntreset_1;
    output wire [0:0] gt_rxprbserr_1;
    input  wire [3:0] gt_rxprbssel_1;
    input  wire [2:0] gt_rxrate_1;
    input  wire [0:0] gt_rxslide_in_1;
    output wire [1:0] gt_rxstartofseq_1;
    output wire [1:0] gt_txbufstatus_1;
    input  wire [0:0] gt_txinhibit_1;
    input  wire [0:0] gt_txlatclk_1;
    input  wire [6:0] gt_txmaincursor_1;
    input  wire [0:0] gt_txpcsreset_1;
    input  wire [0:0] gt_txpmareset_1;
    input  wire [0:0] gt_txpolarity_1;
    input  wire [4:0] gt_txpostcursor_1;
    input  wire [0:0] gt_txprbsforceerr_1;
    input  wire [0:0] gt_txelecidle_1;
    input  wire [3:0] gt_txprbssel_1;
    input  wire [4:0] gt_txprecursor_1;
    input wire [4:0] gt_txdiffctrl_1;
//  // GT DRP interface ports
    output wire [15:0] gt_drpdo_1;
    output wire [0:0] gt_drprdy_1;
    input  wire [0:0] gt_drpen_1;
    input  wire [0:0] gt_drpwe_1;
    input  wire [9:0] gt_drpaddr_1;
    input  wire [15:0] gt_drpdi_1;
    input wire gt_drpclk_1;
    input wire gt_drprst_1;
    input wire gtwiz_reset_tx_datapath_1;
    input wire gtwiz_reset_rx_datapath_1;
    output wire gtpowergood_out_1;
    input wire [2:0] txoutclksel_in_1;
    input wire [2:0] rxoutclksel_in_1;

    output wire tx_clk_out_2;
    input  wire rx_core_clk_2;
    output wire rx_clk_out_2;

//  // AXI_2 interface ports
    input  wire s_axi_aclk_2;
    input  wire s_axi_aresetn_2;
    input  wire [31:0] s_axi_awaddr_2;
    input  wire s_axi_awvalid_2;
    output wire s_axi_awready_2;
    input  wire [31:0] s_axi_wdata_2;
    input  wire [3:0] s_axi_wstrb_2;
    input  wire s_axi_wvalid_2;
    output wire s_axi_wready_2;
    output wire [1:0] s_axi_bresp_2;
    output wire s_axi_bvalid_2;
    input  wire s_axi_bready_2;
    input  wire [31:0] s_axi_araddr_2;
    input  wire s_axi_arvalid_2;
    output wire s_axi_arready_2;
    output wire [31:0] s_axi_rdata_2;
    output wire [1:0] s_axi_rresp_2;
    output wire s_axi_rvalid_2;
    input  wire s_axi_rready_2;
    input  wire pm_tick_2;
//  // RX_2 Signals
    input  wire rx_reset_2;
    output wire user_rx_reset_2;
    output wire rxrecclkout_2;
//  // RX_2 User Interface Signals
    output wire rx_axis_tvalid_2;
    output wire [63:0] rx_axis_tdata_2;
    output wire rx_axis_tlast_2;
    output wire [7:0] rx_axis_tkeep_2;
    output wire rx_axis_tuser_2;
    output wire [55:0] rx_preambleout_2;




    input  wire ctl_rx_rate_10g_25gn_2;
    input  wire gt_drp_done_2;
    input  wire [1:0] txpllclksel_in_2;
    input  wire [1:0] rxpllclksel_in_2;
    input  wire [1:0] txsysclksel_in_2;
    input  wire [1:0] rxsysclksel_in_2;

    output  wire axi_ctl_core_mode_switch_2;
    input  wire [3:0] rxdfecfokfcnum_in_2;
    input  wire rxafecfoken_in_2;
//  // RX_2 Control Signals



//  // RX_2 Stats Signals
    output wire stat_rx_block_lock_2;
    output wire stat_rx_framing_err_valid_2;
    output wire stat_rx_framing_err_2;
    output wire stat_rx_hi_ber_2;
    output wire stat_rx_valid_ctrl_code_2;
    output wire stat_rx_bad_code_2;
    output wire [1:0] stat_rx_total_packets_2;
    output wire stat_rx_total_good_packets_2;
    output wire [3:0] stat_rx_total_bytes_2;
    output wire [13:0] stat_rx_total_good_bytes_2;
    output wire stat_rx_packet_small_2;
    output wire stat_rx_jabber_2;
    output wire stat_rx_packet_large_2;
    output wire stat_rx_oversize_2;
    output wire stat_rx_undersize_2;
    output wire stat_rx_toolong_2;
    output wire stat_rx_fragment_2;
    output wire stat_rx_packet_64_bytes_2;
    output wire stat_rx_packet_65_127_bytes_2;
    output wire stat_rx_packet_128_255_bytes_2;
    output wire stat_rx_packet_256_511_bytes_2;
    output wire stat_rx_packet_512_1023_bytes_2;
    output wire stat_rx_packet_1024_1518_bytes_2;
    output wire stat_rx_packet_1519_1522_bytes_2;
    output wire stat_rx_packet_1523_1548_bytes_2;
    output wire [1:0] stat_rx_bad_fcs_2;
    output wire stat_rx_packet_bad_fcs_2;
    output wire [1:0] stat_rx_stomped_fcs_2;
    output wire stat_rx_packet_1549_2047_bytes_2;
    output wire stat_rx_packet_2048_4095_bytes_2;
    output wire stat_rx_packet_4096_8191_bytes_2;
    output wire stat_rx_packet_8192_9215_bytes_2;
    output wire stat_rx_unicast_2;
    output wire stat_rx_multicast_2;
    output wire stat_rx_broadcast_2;
    output wire stat_rx_vlan_2;
    output wire stat_rx_inrangeerr_2;
    output wire stat_rx_bad_preamble_2;
    output wire stat_rx_bad_sfd_2;
    output wire stat_rx_got_signal_os_2;
    output wire stat_rx_test_pattern_mismatch_2;
    output wire stat_rx_truncated_2;
    output wire stat_rx_local_fault_2;
    output wire stat_rx_remote_fault_2;
    output wire stat_rx_internal_local_fault_2;
    output wire stat_rx_received_local_fault_2;
    output wire  stat_rx_status_2;


//  // TX_2 Signals
    input  wire tx_reset_2;
    output wire user_tx_reset_2;

//  // TX_2 User Interface Signals
    output wire tx_axis_tready_2;
    input  wire tx_axis_tvalid_2;
    input  wire [63:0] tx_axis_tdata_2;
    input  wire tx_axis_tlast_2;
    input  wire [7:0] tx_axis_tkeep_2;
    input  wire tx_axis_tuser_2;
    output wire tx_unfout_2;
    input  wire [55:0] tx_preamblein_2;

//  // TX_2 Control Signals
    input  wire ctl_tx_send_lfi_2;
    input  wire ctl_tx_send_rfi_2;
    input  wire ctl_tx_send_idle_2;


//  // TX_2 Stats Signals
    output wire stat_tx_total_packets_2;
    output wire [3:0] stat_tx_total_bytes_2;
    output wire stat_tx_total_good_packets_2;
    output wire [13:0] stat_tx_total_good_bytes_2;
    output wire stat_tx_packet_64_bytes_2;
    output wire stat_tx_packet_65_127_bytes_2;
    output wire stat_tx_packet_128_255_bytes_2;
    output wire stat_tx_packet_256_511_bytes_2;
    output wire stat_tx_packet_512_1023_bytes_2;
    output wire stat_tx_packet_1024_1518_bytes_2;
    output wire stat_tx_packet_1519_1522_bytes_2;
    output wire stat_tx_packet_1523_1548_bytes_2;
    output wire stat_tx_packet_small_2;
    output wire stat_tx_packet_large_2;
    output wire stat_tx_packet_1549_2047_bytes_2;
    output wire stat_tx_packet_2048_4095_bytes_2;
    output wire stat_tx_packet_4096_8191_bytes_2;
    output wire stat_tx_packet_8192_9215_bytes_2;
    output wire stat_tx_unicast_2;
    output wire stat_tx_multicast_2;
    output wire stat_tx_broadcast_2;
    output wire stat_tx_vlan_2;
    output wire stat_tx_bad_fcs_2;
    output wire stat_tx_frame_error_2;
    output wire stat_tx_local_fault_2;


    output wire [31:0]  user_reg0_2;


//  // FEC Stats Signals
    output wire stat_rx_rsfec_hi_ser_2;
    output wire stat_rx_rsfec_lane_alignment_status_2;
    output wire stat_rx_rsfec_corrected_cw_inc_2;
    output wire stat_rx_rsfec_uncorrected_cw_inc_2;
    output wire [2:0] stat_rx_rsfec_err_count0_inc_2;
    output wire stat_tx_rsfec_lane_alignment_status_2;

//  // GT Debug interface ports
    output wire [16:0] gt_dmonitorout_2;
    output wire [0:0] gt_eyescandataerror_2;
    input  wire [0:0] gt_eyescanreset_2;
    input  wire [0:0] gt_eyescantrigger_2;
    input  wire [15:0] gt_pcsrsvdin_2;
    input  wire [0:0] gt_rxbufreset_2;
    output wire [2:0] gt_rxbufstatus_2;
    input  wire [0:0] gt_rxcdrhold_2;
    input  wire [0:0] gt_rxcommadeten_2;
    input  wire [0:0] gt_rxdfeagchold_2;
    input  wire [0:0] gt_rxdfelpmreset_2;
    input  wire [0:0] gt_rxlatclk_2;
    input  wire [0:0] gt_rxlpmen_2;
    input  wire [0:0] gt_rxpcsreset_2;
    input  wire [0:0] gt_rxpmareset_2;
    input  wire [0:0] gt_rxpolarity_2;
    input  wire [0:0] gt_rxprbscntreset_2;
    output wire [0:0] gt_rxprbserr_2;
    input  wire [3:0] gt_rxprbssel_2;
    input  wire [2:0] gt_rxrate_2;
    input  wire [0:0] gt_rxslide_in_2;
    output wire [1:0] gt_rxstartofseq_2;
    output wire [1:0] gt_txbufstatus_2;
    input  wire [0:0] gt_txinhibit_2;
    input  wire [0:0] gt_txlatclk_2;
    input  wire [6:0] gt_txmaincursor_2;
    input  wire [0:0] gt_txpcsreset_2;
    input  wire [0:0] gt_txpmareset_2;
    input  wire [0:0] gt_txpolarity_2;
    input  wire [4:0] gt_txpostcursor_2;
    input  wire [0:0] gt_txprbsforceerr_2;
    input  wire [0:0] gt_txelecidle_2;
    input  wire [3:0] gt_txprbssel_2;
    input  wire [4:0] gt_txprecursor_2;
    input wire [4:0] gt_txdiffctrl_2;
//  // GT DRP interface ports
    output wire [15:0] gt_drpdo_2;
    output wire [0:0] gt_drprdy_2;
    input  wire [0:0] gt_drpen_2;
    input  wire [0:0] gt_drpwe_2;
    input  wire [9:0] gt_drpaddr_2;
    input  wire [15:0] gt_drpdi_2;
    input wire gt_drpclk_2;
    input wire gt_drprst_2;
    input wire gtwiz_reset_tx_datapath_2;
    input wire gtwiz_reset_rx_datapath_2;
    output wire gtpowergood_out_2;
    input wire [2:0] txoutclksel_in_2;
    input wire [2:0] rxoutclksel_in_2;

    output wire tx_clk_out_3;
    input  wire rx_core_clk_3;
    output wire rx_clk_out_3;

//  // AXI_3 interface ports
    input  wire s_axi_aclk_3;
    input  wire s_axi_aresetn_3;
    input  wire [31:0] s_axi_awaddr_3;
    input  wire s_axi_awvalid_3;
    output wire s_axi_awready_3;
    input  wire [31:0] s_axi_wdata_3;
    input  wire [3:0] s_axi_wstrb_3;
    input  wire s_axi_wvalid_3;
    output wire s_axi_wready_3;
    output wire [1:0] s_axi_bresp_3;
    output wire s_axi_bvalid_3;
    input  wire s_axi_bready_3;
    input  wire [31:0] s_axi_araddr_3;
    input  wire s_axi_arvalid_3;
    output wire s_axi_arready_3;
    output wire [31:0] s_axi_rdata_3;
    output wire [1:0] s_axi_rresp_3;
    output wire s_axi_rvalid_3;
    input  wire s_axi_rready_3;
    input  wire pm_tick_3;
//  // RX_3 Signals
    input  wire rx_reset_3;
    output wire user_rx_reset_3;
    output wire rxrecclkout_3;
//  // RX_3 User Interface Signals
    output wire rx_axis_tvalid_3;
    output wire [63:0] rx_axis_tdata_3;
    output wire rx_axis_tlast_3;
    output wire [7:0] rx_axis_tkeep_3;
    output wire rx_axis_tuser_3;
    output wire [55:0] rx_preambleout_3;




    input  wire ctl_rx_rate_10g_25gn_3;
    input  wire gt_drp_done_3;
    input  wire [1:0] txpllclksel_in_3;
    input  wire [1:0] rxpllclksel_in_3;
    input  wire [1:0] txsysclksel_in_3;
    input  wire [1:0] rxsysclksel_in_3;

    output  wire axi_ctl_core_mode_switch_3;
    input  wire [3:0] rxdfecfokfcnum_in_3;
    input  wire rxafecfoken_in_3;
//  // RX_3 Control Signals



//  // RX_3 Stats Signals
    output wire stat_rx_block_lock_3;
    output wire stat_rx_framing_err_valid_3;
    output wire stat_rx_framing_err_3;
    output wire stat_rx_hi_ber_3;
    output wire stat_rx_valid_ctrl_code_3;
    output wire stat_rx_bad_code_3;
    output wire [1:0] stat_rx_total_packets_3;
    output wire stat_rx_total_good_packets_3;
    output wire [3:0] stat_rx_total_bytes_3;
    output wire [13:0] stat_rx_total_good_bytes_3;
    output wire stat_rx_packet_small_3;
    output wire stat_rx_jabber_3;
    output wire stat_rx_packet_large_3;
    output wire stat_rx_oversize_3;
    output wire stat_rx_undersize_3;
    output wire stat_rx_toolong_3;
    output wire stat_rx_fragment_3;
    output wire stat_rx_packet_64_bytes_3;
    output wire stat_rx_packet_65_127_bytes_3;
    output wire stat_rx_packet_128_255_bytes_3;
    output wire stat_rx_packet_256_511_bytes_3;
    output wire stat_rx_packet_512_1023_bytes_3;
    output wire stat_rx_packet_1024_1518_bytes_3;
    output wire stat_rx_packet_1519_1522_bytes_3;
    output wire stat_rx_packet_1523_1548_bytes_3;
    output wire [1:0] stat_rx_bad_fcs_3;
    output wire stat_rx_packet_bad_fcs_3;
    output wire [1:0] stat_rx_stomped_fcs_3;
    output wire stat_rx_packet_1549_2047_bytes_3;
    output wire stat_rx_packet_2048_4095_bytes_3;
    output wire stat_rx_packet_4096_8191_bytes_3;
    output wire stat_rx_packet_8192_9215_bytes_3;
    output wire stat_rx_unicast_3;
    output wire stat_rx_multicast_3;
    output wire stat_rx_broadcast_3;
    output wire stat_rx_vlan_3;
    output wire stat_rx_inrangeerr_3;
    output wire stat_rx_bad_preamble_3;
    output wire stat_rx_bad_sfd_3;
    output wire stat_rx_got_signal_os_3;
    output wire stat_rx_test_pattern_mismatch_3;
    output wire stat_rx_truncated_3;
    output wire stat_rx_local_fault_3;
    output wire stat_rx_remote_fault_3;
    output wire stat_rx_internal_local_fault_3;
    output wire stat_rx_received_local_fault_3;
    output wire  stat_rx_status_3;


//  // TX_3 Signals
    input  wire tx_reset_3;
    output wire user_tx_reset_3;

//  // TX_3 User Interface Signals
    output wire tx_axis_tready_3;
    input  wire tx_axis_tvalid_3;
    input  wire [63:0] tx_axis_tdata_3;
    input  wire tx_axis_tlast_3;
    input  wire [7:0] tx_axis_tkeep_3;
    input  wire tx_axis_tuser_3;
    output wire tx_unfout_3;
    input  wire [55:0] tx_preamblein_3;

//  // TX_3 Control Signals
    input  wire ctl_tx_send_lfi_3;
    input  wire ctl_tx_send_rfi_3;
    input  wire ctl_tx_send_idle_3;


//  // TX_3 Stats Signals
    output wire stat_tx_total_packets_3;
    output wire [3:0] stat_tx_total_bytes_3;
    output wire stat_tx_total_good_packets_3;
    output wire [13:0] stat_tx_total_good_bytes_3;
    output wire stat_tx_packet_64_bytes_3;
    output wire stat_tx_packet_65_127_bytes_3;
    output wire stat_tx_packet_128_255_bytes_3;
    output wire stat_tx_packet_256_511_bytes_3;
    output wire stat_tx_packet_512_1023_bytes_3;
    output wire stat_tx_packet_1024_1518_bytes_3;
    output wire stat_tx_packet_1519_1522_bytes_3;
    output wire stat_tx_packet_1523_1548_bytes_3;
    output wire stat_tx_packet_small_3;
    output wire stat_tx_packet_large_3;
    output wire stat_tx_packet_1549_2047_bytes_3;
    output wire stat_tx_packet_2048_4095_bytes_3;
    output wire stat_tx_packet_4096_8191_bytes_3;
    output wire stat_tx_packet_8192_9215_bytes_3;
    output wire stat_tx_unicast_3;
    output wire stat_tx_multicast_3;
    output wire stat_tx_broadcast_3;
    output wire stat_tx_vlan_3;
    output wire stat_tx_bad_fcs_3;
    output wire stat_tx_frame_error_3;
    output wire stat_tx_local_fault_3;


    output wire [31:0]  user_reg0_3;


//  // FEC Stats Signals
    output wire stat_rx_rsfec_hi_ser_3;
    output wire stat_rx_rsfec_lane_alignment_status_3;
    output wire stat_rx_rsfec_corrected_cw_inc_3;
    output wire stat_rx_rsfec_uncorrected_cw_inc_3;
    output wire [2:0] stat_rx_rsfec_err_count0_inc_3;
    output wire stat_tx_rsfec_lane_alignment_status_3;

//  // GT Debug interface ports
    output wire [16:0] gt_dmonitorout_3;
    output wire [0:0] gt_eyescandataerror_3;
    input  wire [0:0] gt_eyescanreset_3;
    input  wire [0:0] gt_eyescantrigger_3;
    input  wire [15:0] gt_pcsrsvdin_3;
    input  wire [0:0] gt_rxbufreset_3;
    output wire [2:0] gt_rxbufstatus_3;
    input  wire [0:0] gt_rxcdrhold_3;
    input  wire [0:0] gt_rxcommadeten_3;
    input  wire [0:0] gt_rxdfeagchold_3;
    input  wire [0:0] gt_rxdfelpmreset_3;
    input  wire [0:0] gt_rxlatclk_3;
    input  wire [0:0] gt_rxlpmen_3;
    input  wire [0:0] gt_rxpcsreset_3;
    input  wire [0:0] gt_rxpmareset_3;
    input  wire [0:0] gt_rxpolarity_3;
    input  wire [0:0] gt_rxprbscntreset_3;
    output wire [0:0] gt_rxprbserr_3;
    input  wire [3:0] gt_rxprbssel_3;
    input  wire [2:0] gt_rxrate_3;
    input  wire [0:0] gt_rxslide_in_3;
    output wire [1:0] gt_rxstartofseq_3;
    output wire [1:0] gt_txbufstatus_3;
    input  wire [0:0] gt_txinhibit_3;
    input  wire [0:0] gt_txlatclk_3;
    input  wire [6:0] gt_txmaincursor_3;
    input  wire [0:0] gt_txpcsreset_3;
    input  wire [0:0] gt_txpmareset_3;
    input  wire [0:0] gt_txpolarity_3;
    input  wire [4:0] gt_txpostcursor_3;
    input  wire [0:0] gt_txprbsforceerr_3;
    input  wire [0:0] gt_txelecidle_3;
    input  wire [3:0] gt_txprbssel_3;
    input  wire [4:0] gt_txprecursor_3;
    input wire [4:0] gt_txdiffctrl_3;
//  // GT DRP interface ports
    output wire [15:0] gt_drpdo_3;
    output wire [0:0] gt_drprdy_3;
    input  wire [0:0] gt_drpen_3;
    input  wire [0:0] gt_drpwe_3;
    input  wire [9:0] gt_drpaddr_3;
    input  wire [15:0] gt_drpdi_3;
    input wire gt_drpclk_3;
    input wire gt_drprst_3;
    input wire gtwiz_reset_tx_datapath_3;
    input wire gtwiz_reset_rx_datapath_3;
    output wire gtpowergood_out_3;
    input wire [2:0] txoutclksel_in_3;
    input wire [2:0] rxoutclksel_in_3;

    input  wire sys_reset;
    input  wire dclk;

    input wire qpllreset_in_0;
    input  wire [3 : 0] gt_refclk_p;
    input  wire [3 : 0] gt_refclk_n;
    output wire [3 : 0] gt_refclk_out;


    wire          user_rx_reset_int_0;
    wire          user_tx_reset_int_0;
    wire          rx_resetdone_out_0;
    wire          tx_resetdone_out_0;

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_rx_rst_0 (
        .clk          (rx_clk_out_0 ),
        .signal_in    (user_rx_reset_int_0),
        .signal_out   (user_rx_reset_0)
    );

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_tx_rst_0 (
        .clk          (tx_clk_out_0 ),
        .signal_in    (user_tx_reset_int_0),
        .signal_out   (user_tx_reset_0)
    );

    assign user_rx_reset_int_0 = ~(rx_resetdone_out_0);
    assign user_tx_reset_int_0 = ~(rx_resetdone_out_0);


    wire          user_rx_reset_int_1;
    wire          user_tx_reset_int_1;
    wire          rx_resetdone_out_1;
    wire          tx_resetdone_out_1;

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_rx_rst_1 (
        .clk          (rx_clk_out_1 ),
        .signal_in    (user_rx_reset_int_1),
        .signal_out   (user_rx_reset_1)
    );

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_tx_rst_1 (
        .clk          (tx_clk_out_1 ),
        .signal_in    (user_tx_reset_int_1),
        .signal_out   (user_tx_reset_1)
    );

    assign user_rx_reset_int_1 = ~(rx_resetdone_out_1);
    assign user_tx_reset_int_1 = ~(rx_resetdone_out_1);


    wire          user_rx_reset_int_2;
    wire          user_tx_reset_int_2;
    wire          rx_resetdone_out_2;
    wire          tx_resetdone_out_2;

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_rx_rst_2 (
        .clk          (rx_clk_out_2 ),
        .signal_in    (user_rx_reset_int_2),
        .signal_out   (user_rx_reset_2)
    );

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_tx_rst_2 (
        .clk          (tx_clk_out_2 ),
        .signal_in    (user_tx_reset_int_2),
        .signal_out   (user_tx_reset_2)
    );

    assign user_rx_reset_int_2 = ~(rx_resetdone_out_2);
    assign user_tx_reset_int_2 = ~(rx_resetdone_out_2);


    wire          user_rx_reset_int_3;
    wire          user_tx_reset_int_3;
    wire          rx_resetdone_out_3;
    wire          tx_resetdone_out_3;

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_rx_rst_3 (
        .clk          (rx_clk_out_3 ),
        .signal_in    (user_rx_reset_int_3),
        .signal_out   (user_rx_reset_3)
    );

    cdc_sync_2stage
    #(
        .WIDTH        (1)
    ) cdc_tx_rst_3 (
        .clk          (tx_clk_out_3 ),
        .signal_in    (user_tx_reset_int_3),
        .signal_out   (user_tx_reset_3)
    );

    assign user_rx_reset_int_3 = ~(rx_resetdone_out_3);
    assign user_tx_reset_int_3 = ~(rx_resetdone_out_3);

 xbtest_sub_xxv_gt_wrapper_versal_gty_4lanes #(
    .C_GT_MAC_ENABLE_RSFEC (C_GT_MAC_ENABLE_RSFEC)
 ) mac_versal_core (

    .RS_FEC_stat_0_0_stat_rx_rsfec_corrected_cw_inc (stat_rx_rsfec_corrected_cw_inc_0),
    .RS_FEC_stat_0_0_stat_rx_rsfec_err_count0_inc (stat_rx_rsfec_err_count0_inc_0),
    .RS_FEC_stat_0_0_stat_rx_rsfec_hi_ser (stat_rx_rsfec_hi_ser_0),
    .RS_FEC_stat_0_0_stat_rx_rsfec_lane_alignment_status (stat_rx_rsfec_lane_alignment_status_0),
    .RS_FEC_stat_0_0_stat_rx_rsfec_uncorrected_cw_inc (stat_rx_rsfec_uncorrected_cw_inc_0),
    .RS_FEC_stat_0_0_stat_tx_rsfec_lane_alignment_status (stat_tx_rsfec_lane_alignment_status_0),
    .RS_FEC_stat_0_1_stat_rx_rsfec_corrected_cw_inc (stat_rx_rsfec_corrected_cw_inc_1),
    .RS_FEC_stat_0_1_stat_rx_rsfec_err_count0_inc (stat_rx_rsfec_err_count0_inc_1),
    .RS_FEC_stat_0_1_stat_rx_rsfec_hi_ser (stat_rx_rsfec_hi_ser_1),
    .RS_FEC_stat_0_1_stat_rx_rsfec_lane_alignment_status (stat_rx_rsfec_lane_alignment_status_1),
    .RS_FEC_stat_0_1_stat_rx_rsfec_uncorrected_cw_inc (stat_rx_rsfec_uncorrected_cw_inc_1),
    .RS_FEC_stat_0_1_stat_tx_rsfec_lane_alignment_status (stat_tx_rsfec_lane_alignment_status_1),
    .RS_FEC_stat_0_2_stat_rx_rsfec_corrected_cw_inc (stat_rx_rsfec_corrected_cw_inc_2),
    .RS_FEC_stat_0_2_stat_rx_rsfec_err_count0_inc (stat_rx_rsfec_err_count0_inc_2),
    .RS_FEC_stat_0_2_stat_rx_rsfec_hi_ser (stat_rx_rsfec_hi_ser_2),
    .RS_FEC_stat_0_2_stat_rx_rsfec_lane_alignment_status (stat_rx_rsfec_lane_alignment_status_2),
    .RS_FEC_stat_0_2_stat_rx_rsfec_uncorrected_cw_inc (stat_rx_rsfec_uncorrected_cw_inc_2),
    .RS_FEC_stat_0_2_stat_tx_rsfec_lane_alignment_status (stat_tx_rsfec_lane_alignment_status_2),
    .RS_FEC_stat_0_3_stat_rx_rsfec_corrected_cw_inc (stat_rx_rsfec_corrected_cw_inc_3),
    .RS_FEC_stat_0_3_stat_rx_rsfec_err_count0_inc (stat_rx_rsfec_err_count0_inc_3),
    .RS_FEC_stat_0_3_stat_rx_rsfec_hi_ser (stat_rx_rsfec_hi_ser_3),
    .RS_FEC_stat_0_3_stat_rx_rsfec_lane_alignment_status (stat_rx_rsfec_lane_alignment_status_3),
    .RS_FEC_stat_0_3_stat_rx_rsfec_uncorrected_cw_inc (stat_rx_rsfec_uncorrected_cw_inc_3),
    .RS_FEC_stat_0_3_stat_tx_rsfec_lane_alignment_status (stat_tx_rsfec_lane_alignment_status_3),

    .apb3clk_quad (dclk),

    .axis_rx_0_0_tdata  (rx_axis_tdata_0),
    .axis_rx_0_0_tkeep  (rx_axis_tkeep_0),
    .axis_rx_0_0_tlast  (rx_axis_tlast_0),
    .axis_rx_0_0_tuser  (rx_axis_tuser_0),
    .axis_rx_0_0_tvalid (rx_axis_tvalid_0),
    .axis_rx_0_1_tdata  (rx_axis_tdata_1),
    .axis_rx_0_1_tkeep  (rx_axis_tkeep_1),
    .axis_rx_0_1_tlast  (rx_axis_tlast_1),
    .axis_rx_0_1_tuser  (rx_axis_tuser_1),
    .axis_rx_0_1_tvalid (rx_axis_tvalid_1),
    .axis_rx_0_2_tdata  (rx_axis_tdata_2),
    .axis_rx_0_2_tkeep  (rx_axis_tkeep_2),
    .axis_rx_0_2_tlast  (rx_axis_tlast_2),
    .axis_rx_0_2_tuser  (rx_axis_tuser_2),
    .axis_rx_0_2_tvalid (rx_axis_tvalid_2),
    .axis_rx_0_3_tdata  (rx_axis_tdata_3),
    .axis_rx_0_3_tkeep  (rx_axis_tkeep_3),
    .axis_rx_0_3_tlast  (rx_axis_tlast_3),
    .axis_rx_0_3_tuser  (rx_axis_tuser_3),
    .axis_rx_0_3_tvalid (rx_axis_tvalid_3),
    .axis_tx_0_0_tdata  (tx_axis_tdata_0),
    .axis_tx_0_0_tkeep  (tx_axis_tkeep_0),
    .axis_tx_0_0_tlast  (tx_axis_tlast_0),
    .axis_tx_0_0_tready (tx_axis_tready_0),
    .axis_tx_0_0_tuser  (tx_axis_tuser_0),
    .axis_tx_0_0_tvalid (tx_axis_tvalid_0),
    .axis_tx_0_1_tdata  (tx_axis_tdata_1),
    .axis_tx_0_1_tkeep  (tx_axis_tkeep_1),
    .axis_tx_0_1_tlast  (tx_axis_tlast_1),
    .axis_tx_0_1_tready (tx_axis_tready_1),
    .axis_tx_0_1_tuser  (tx_axis_tuser_1),
    .axis_tx_0_1_tvalid (tx_axis_tvalid_1),
    .axis_tx_0_2_tdata  (tx_axis_tdata_2),
    .axis_tx_0_2_tkeep  (tx_axis_tkeep_2),
    .axis_tx_0_2_tlast  (tx_axis_tlast_2),
    .axis_tx_0_2_tready (tx_axis_tready_2),
    .axis_tx_0_2_tuser  (tx_axis_tuser_2),
    .axis_tx_0_2_tvalid (tx_axis_tvalid_2),
    .axis_tx_0_3_tdata  (tx_axis_tdata_3),
    .axis_tx_0_3_tkeep  (tx_axis_tkeep_3),
    .axis_tx_0_3_tlast  (tx_axis_tlast_3),
    .axis_tx_0_3_tready (tx_axis_tready_3),
    .axis_tx_0_3_tuser  (tx_axis_tuser_3),
    .axis_tx_0_3_tvalid (tx_axis_tvalid_3),

    .ctl_tx_0_0_ctl_tx_send_idle (ctl_tx_send_idle_0),
    .ctl_tx_0_0_ctl_tx_send_lfi  (ctl_tx_send_lfi_0),
    .ctl_tx_0_0_ctl_tx_send_rfi  (ctl_tx_send_rfi_0),
    .ctl_tx_0_1_ctl_tx_send_idle (ctl_tx_send_idle_1),
    .ctl_tx_0_1_ctl_tx_send_lfi  (ctl_tx_send_lfi_1),
    .ctl_tx_0_1_ctl_tx_send_rfi  (ctl_tx_send_rfi_1),
    .ctl_tx_0_2_ctl_tx_send_idle (ctl_tx_send_idle_2),
    .ctl_tx_0_2_ctl_tx_send_lfi  (ctl_tx_send_lfi_2),
    .ctl_tx_0_2_ctl_tx_send_rfi  (ctl_tx_send_rfi_2),
    .ctl_tx_0_3_ctl_tx_send_idle (ctl_tx_send_idle_3),
    .ctl_tx_0_3_ctl_tx_send_lfi  (ctl_tx_send_lfi_3),
    .ctl_tx_0_3_ctl_tx_send_rfi  (ctl_tx_send_rfi_3),

    .gt_rxn_in_0  (gt_rxn_in ),
    .gt_rxp_in_0  (gt_rxp_in ),
    .gt_txn_out_0 (gt_txn_out),
    .gt_txp_out_0 (gt_txp_out),

    .gtwiz_reset_clk_freerun_in_0_0 (dclk),
    .gtwiz_reset_clk_freerun_in_0_1 (dclk),
    .gtwiz_reset_clk_freerun_in_0_2 (dclk),
    .gtwiz_reset_clk_freerun_in_0_3 (dclk),

    .gt_refclk_clk_n (gt_refclk_n),
    .gt_refclk_clk_p (gt_refclk_p),

    .pm_tick_0_0 (pm_tick_0),
    .pm_tick_0_1 (pm_tick_1),
    .pm_tick_0_2 (pm_tick_2),
    .pm_tick_0_3 (pm_tick_3),

    .rx_core_clk_0_0 (rx_core_clk_0),
    .rx_core_clk_0_1 (rx_core_clk_1),
    .rx_core_clk_0_2 (rx_core_clk_2),
    .rx_core_clk_0_3 (rx_core_clk_3),
    .rx_preambleout_0_0 (rx_preambleout_0),
    .rx_preambleout_0_1 (rx_preambleout_1),
    .rx_preambleout_0_2 (rx_preambleout_2),
    .rx_preambleout_0_3 (rx_preambleout_3),
    .rx_reset_0_0 (rx_reset_0),
    .rx_reset_0_1 (rx_reset_1),
    .rx_reset_0_2 (rx_reset_2),
    .rx_reset_0_3 (rx_reset_3),
    .rx_resetdone_out_0_0 (rx_resetdone_out_0),
    .rx_resetdone_out_0_1 (rx_resetdone_out_1),
    .rx_resetdone_out_0_2 (rx_resetdone_out_2),
    .rx_resetdone_out_0_3 (rx_resetdone_out_3),
    .rx_serdes_clk_0_0 (rx_clk_out_0),
    .rx_serdes_clk_0_1 (rx_clk_out_1),
    .rx_serdes_clk_0_2 (rx_clk_out_2),
    .rx_serdes_clk_0_3 (rx_clk_out_3),
    .rx_serdes_reset_0_0 (user_rx_reset_0),
    .rx_serdes_reset_0_1 (user_rx_reset_1),
    .rx_serdes_reset_0_2 (user_rx_reset_2),
    .rx_serdes_reset_0_3 (user_rx_reset_3),
    .rx_usrclk_0 (rx_clk_out_0),
    .rx_usrclk_1 (rx_clk_out_1),
    .rx_usrclk_2 (rx_clk_out_2),
    .rx_usrclk_3 (rx_clk_out_3),

    .s_axi_0_0_araddr (s_axi_araddr_0),
    .s_axi_0_0_arready(s_axi_arready_0),
    .s_axi_0_0_arvalid(s_axi_arvalid_0),
    .s_axi_0_0_awaddr (s_axi_awaddr_0),
    .s_axi_0_0_awready(s_axi_awready_0),
    .s_axi_0_0_awvalid(s_axi_awvalid_0),
    .s_axi_0_0_bready (s_axi_bready_0),
    .s_axi_0_0_bresp  (s_axi_bresp_0),
    .s_axi_0_0_bvalid (s_axi_bvalid_0),
    .s_axi_0_0_rdata  (s_axi_rdata_0),
    .s_axi_0_0_rready (s_axi_rready_0),
    .s_axi_0_0_rresp  (s_axi_rresp_0),
    .s_axi_0_0_rvalid (s_axi_rvalid_0),
    .s_axi_0_0_wdata  (s_axi_wdata_0),
    .s_axi_0_0_wready (s_axi_wready_0),
    .s_axi_0_0_wstrb  (s_axi_wstrb_0),
    .s_axi_0_0_wvalid (s_axi_wvalid_0),
    .s_axi_0_1_araddr (s_axi_araddr_1),
    .s_axi_0_1_arready(s_axi_arready_1),
    .s_axi_0_1_arvalid(s_axi_arvalid_1),
    .s_axi_0_1_awaddr (s_axi_awaddr_1),
    .s_axi_0_1_awready(s_axi_awready_1),
    .s_axi_0_1_awvalid(s_axi_awvalid_1),
    .s_axi_0_1_bready (s_axi_bready_1),
    .s_axi_0_1_bresp  (s_axi_bresp_1),
    .s_axi_0_1_bvalid (s_axi_bvalid_1),
    .s_axi_0_1_rdata  (s_axi_rdata_1),
    .s_axi_0_1_rready (s_axi_rready_1),
    .s_axi_0_1_rresp  (s_axi_rresp_1),
    .s_axi_0_1_rvalid (s_axi_rvalid_1),
    .s_axi_0_1_wdata  (s_axi_wdata_1),
    .s_axi_0_1_wready (s_axi_wready_1),
    .s_axi_0_1_wstrb  (s_axi_wstrb_1),
    .s_axi_0_1_wvalid (s_axi_wvalid_1),
    .s_axi_0_2_araddr (s_axi_araddr_2),
    .s_axi_0_2_arready(s_axi_arready_2),
    .s_axi_0_2_arvalid(s_axi_arvalid_2),
    .s_axi_0_2_awaddr (s_axi_awaddr_2),
    .s_axi_0_2_awready(s_axi_awready_2),
    .s_axi_0_2_awvalid(s_axi_awvalid_2),
    .s_axi_0_2_bready (s_axi_bready_2),
    .s_axi_0_2_bresp  (s_axi_bresp_2),
    .s_axi_0_2_bvalid (s_axi_bvalid_2),
    .s_axi_0_2_rdata  (s_axi_rdata_2),
    .s_axi_0_2_rready (s_axi_rready_2),
    .s_axi_0_2_rresp  (s_axi_rresp_2),
    .s_axi_0_2_rvalid (s_axi_rvalid_2),
    .s_axi_0_2_wdata  (s_axi_wdata_2),
    .s_axi_0_2_wready (s_axi_wready_2),
    .s_axi_0_2_wstrb  (s_axi_wstrb_2),
    .s_axi_0_2_wvalid (s_axi_wvalid_2),
    .s_axi_0_3_araddr (s_axi_araddr_3),
    .s_axi_0_3_arready(s_axi_arready_3),
    .s_axi_0_3_arvalid(s_axi_arvalid_3),
    .s_axi_0_3_awaddr (s_axi_awaddr_3),
    .s_axi_0_3_awready(s_axi_awready_3),
    .s_axi_0_3_awvalid(s_axi_awvalid_3),
    .s_axi_0_3_bready (s_axi_bready_3),
    .s_axi_0_3_bresp  (s_axi_bresp_3),
    .s_axi_0_3_bvalid (s_axi_bvalid_3),
    .s_axi_0_3_rdata  (s_axi_rdata_3),
    .s_axi_0_3_rready (s_axi_rready_3),
    .s_axi_0_3_rresp  (s_axi_rresp_3),
    .s_axi_0_3_rvalid (s_axi_rvalid_3),
    .s_axi_0_3_wdata  (s_axi_wdata_3),
    .s_axi_0_3_wready (s_axi_wready_3),
    .s_axi_0_3_wstrb  (s_axi_wstrb_3),
    .s_axi_0_3_wvalid (s_axi_wvalid_3),
    .s_axi_aclk_0_0 (s_axi_aclk_0),
    .s_axi_aclk_0_1 (s_axi_aclk_1),
    .s_axi_aclk_0_2 (s_axi_aclk_2),
    .s_axi_aclk_0_3 (s_axi_aclk_3),
    .s_axi_aresetn_0_0 (s_axi_aresetn_0),
    .s_axi_aresetn_0_1 (s_axi_aresetn_1),
    .s_axi_aresetn_0_2 (s_axi_aresetn_2),
    .s_axi_aresetn_0_3 (s_axi_aresetn_3),

    .stat_rx_0_0_stat_rx_bad_code (stat_rx_bad_code_0),
    .stat_rx_0_0_stat_rx_bad_fcs (stat_rx_bad_fcs_0),
    .stat_rx_0_0_stat_rx_bad_preamble (stat_rx_bad_preamble_0),
    .stat_rx_0_0_stat_rx_bad_sfd (stat_rx_bad_sfd_0),
    .stat_rx_0_0_stat_rx_block_lock (stat_rx_block_lock_0),
    .stat_rx_0_0_stat_rx_broadcast (stat_rx_broadcast_0),
    .stat_rx_0_0_stat_rx_fragment (stat_rx_fragment_0),
    .stat_rx_0_0_stat_rx_framing_err (stat_rx_framing_err_0),
    .stat_rx_0_0_stat_rx_framing_err_valid (stat_rx_framing_err_valid_0),
    .stat_rx_0_0_stat_rx_got_signal_os (stat_rx_got_signal_os_0),
    .stat_rx_0_0_stat_rx_hi_ber (stat_rx_hi_ber_0),
    .stat_rx_0_0_stat_rx_inrangeerr (stat_rx_inrangeerr_0),
    .stat_rx_0_0_stat_rx_internal_local_fault (stat_rx_internal_local_fault_0),
    .stat_rx_0_0_stat_rx_jabber (stat_rx_jabber_0),
    .stat_rx_0_0_stat_rx_local_fault (stat_rx_local_fault_0),
    .stat_rx_0_0_stat_rx_multicast (stat_rx_multicast_0),
    .stat_rx_0_0_stat_rx_oversize (stat_rx_oversize_0),
    .stat_rx_0_0_stat_rx_packet_1024_1518_bytes (stat_rx_packet_1024_1518_bytes_0),
    .stat_rx_0_0_stat_rx_packet_128_255_bytes (stat_rx_packet_128_255_bytes_0),
    .stat_rx_0_0_stat_rx_packet_1519_1522_bytes (stat_rx_packet_1519_1522_bytes_0),
    .stat_rx_0_0_stat_rx_packet_1523_1548_bytes (stat_rx_packet_1523_1548_bytes_0),
    .stat_rx_0_0_stat_rx_packet_1549_2047_bytes (stat_rx_packet_1549_2047_bytes_0),
    .stat_rx_0_0_stat_rx_packet_2048_4095_bytes (stat_rx_packet_2048_4095_bytes_0),
    .stat_rx_0_0_stat_rx_packet_256_511_bytes (stat_rx_packet_256_511_bytes_0),
    .stat_rx_0_0_stat_rx_packet_4096_8191_bytes (stat_rx_packet_4096_8191_bytes_0),
    .stat_rx_0_0_stat_rx_packet_512_1023_bytes (stat_rx_packet_512_1023_bytes_0),
    .stat_rx_0_0_stat_rx_packet_64_bytes (stat_rx_packet_64_bytes_0),
    .stat_rx_0_0_stat_rx_packet_65_127_bytes (stat_rx_packet_65_127_bytes_0),
    .stat_rx_0_0_stat_rx_packet_8192_9215_bytes (stat_rx_packet_8192_9215_bytes_0),
    .stat_rx_0_0_stat_rx_packet_bad_fcs (stat_rx_packet_bad_fcs_0),
    .stat_rx_0_0_stat_rx_packet_large (stat_rx_packet_large_0),
    .stat_rx_0_0_stat_rx_packet_small (stat_rx_packet_small_0),
    .stat_rx_0_0_stat_rx_received_local_fault (stat_rx_received_local_fault_0),
    .stat_rx_0_0_stat_rx_remote_fault (stat_rx_remote_fault_0),
    .stat_rx_0_0_stat_rx_stomped_fcs (stat_rx_stomped_fcs_0),
    .stat_rx_0_0_stat_rx_test_pattern_mismatch (stat_rx_test_pattern_mismatch_0),
    .stat_rx_0_0_stat_rx_toolong (stat_rx_toolong_0),
    .stat_rx_0_0_stat_rx_total_bytes (stat_rx_total_bytes_0),
    .stat_rx_0_0_stat_rx_total_good_bytes (stat_rx_total_good_bytes_0),
    .stat_rx_0_0_stat_rx_total_good_packets (stat_rx_total_good_packets_0),
    .stat_rx_0_0_stat_rx_total_packets (stat_rx_total_packets_0),
    .stat_rx_0_0_stat_rx_truncated (stat_rx_truncated_0),
    .stat_rx_0_0_stat_rx_undersize (stat_rx_undersize_0),
    .stat_rx_0_0_stat_rx_unicast (stat_rx_unicast_0),
    .stat_rx_0_0_stat_rx_valid_ctrl_code (stat_rx_valid_ctrl_code_0),
    .stat_rx_0_0_stat_rx_vlan (stat_rx_vlan_0),
    .stat_rx_0_1_stat_rx_bad_code (stat_rx_bad_code_1),
    .stat_rx_0_1_stat_rx_bad_fcs (stat_rx_bad_fcs_1),
    .stat_rx_0_1_stat_rx_bad_preamble (stat_rx_bad_preamble_1),
    .stat_rx_0_1_stat_rx_bad_sfd (stat_rx_bad_sfd_1),
    .stat_rx_0_1_stat_rx_block_lock (stat_rx_block_lock_1),
    .stat_rx_0_1_stat_rx_broadcast (stat_rx_broadcast_1),
    .stat_rx_0_1_stat_rx_fragment (stat_rx_fragment_1),
    .stat_rx_0_1_stat_rx_framing_err (stat_rx_framing_err_1),
    .stat_rx_0_1_stat_rx_framing_err_valid (stat_rx_framing_err_valid_1),
    .stat_rx_0_1_stat_rx_got_signal_os (stat_rx_got_signal_os_1),
    .stat_rx_0_1_stat_rx_hi_ber (stat_rx_hi_ber_1),
    .stat_rx_0_1_stat_rx_inrangeerr (stat_rx_inrangeerr_1),
    .stat_rx_0_1_stat_rx_internal_local_fault (stat_rx_internal_local_fault_1),
    .stat_rx_0_1_stat_rx_jabber (stat_rx_jabber_1),
    .stat_rx_0_1_stat_rx_local_fault (stat_rx_local_fault_1),
    .stat_rx_0_1_stat_rx_multicast (stat_rx_multicast_1),
    .stat_rx_0_1_stat_rx_oversize (stat_rx_oversize_1),
    .stat_rx_0_1_stat_rx_packet_1024_1518_bytes (stat_rx_packet_1024_1518_bytes_1),
    .stat_rx_0_1_stat_rx_packet_128_255_bytes (stat_rx_packet_128_255_bytes_1),
    .stat_rx_0_1_stat_rx_packet_1519_1522_bytes (stat_rx_packet_1519_1522_bytes_1),
    .stat_rx_0_1_stat_rx_packet_1523_1548_bytes (stat_rx_packet_1523_1548_bytes_1),
    .stat_rx_0_1_stat_rx_packet_1549_2047_bytes (stat_rx_packet_1549_2047_bytes_1),
    .stat_rx_0_1_stat_rx_packet_2048_4095_bytes (stat_rx_packet_2048_4095_bytes_1),
    .stat_rx_0_1_stat_rx_packet_256_511_bytes (stat_rx_packet_256_511_bytes_1),
    .stat_rx_0_1_stat_rx_packet_4096_8191_bytes (stat_rx_packet_4096_8191_bytes_1),
    .stat_rx_0_1_stat_rx_packet_512_1023_bytes (stat_rx_packet_512_1023_bytes_1),
    .stat_rx_0_1_stat_rx_packet_64_bytes (stat_rx_packet_64_bytes_1),
    .stat_rx_0_1_stat_rx_packet_65_127_bytes (stat_rx_packet_65_127_bytes_1),
    .stat_rx_0_1_stat_rx_packet_8192_9215_bytes (stat_rx_packet_8192_9215_bytes_1),
    .stat_rx_0_1_stat_rx_packet_bad_fcs (stat_rx_packet_bad_fcs_1),
    .stat_rx_0_1_stat_rx_packet_large (stat_rx_packet_large_1),
    .stat_rx_0_1_stat_rx_packet_small (stat_rx_packet_small_1),
    .stat_rx_0_1_stat_rx_received_local_fault (stat_rx_received_local_fault_1),
    .stat_rx_0_1_stat_rx_remote_fault (stat_rx_remote_fault_1),
    .stat_rx_0_1_stat_rx_stomped_fcs (stat_rx_stomped_fcs_1),
    .stat_rx_0_1_stat_rx_test_pattern_mismatch (stat_rx_test_pattern_mismatch_1),
    .stat_rx_0_1_stat_rx_toolong (stat_rx_toolong_1),
    .stat_rx_0_1_stat_rx_total_bytes (stat_rx_total_bytes_1),
    .stat_rx_0_1_stat_rx_total_good_bytes (stat_rx_total_good_bytes_1),
    .stat_rx_0_1_stat_rx_total_good_packets (stat_rx_total_good_packets_1),
    .stat_rx_0_1_stat_rx_total_packets (stat_rx_total_packets_1),
    .stat_rx_0_1_stat_rx_truncated (stat_rx_truncated_1),
    .stat_rx_0_1_stat_rx_undersize (stat_rx_undersize_1),
    .stat_rx_0_1_stat_rx_unicast (stat_rx_unicast_1),
    .stat_rx_0_1_stat_rx_valid_ctrl_code (stat_rx_valid_ctrl_code_1),
    .stat_rx_0_1_stat_rx_vlan (stat_rx_vlan_1),
    .stat_rx_0_2_stat_rx_bad_code (stat_rx_bad_code_2),
    .stat_rx_0_2_stat_rx_bad_fcs (stat_rx_bad_fcs_2),
    .stat_rx_0_2_stat_rx_bad_preamble (stat_rx_bad_preamble_2),
    .stat_rx_0_2_stat_rx_bad_sfd (stat_rx_bad_sfd_2),
    .stat_rx_0_2_stat_rx_block_lock (stat_rx_block_lock_2),
    .stat_rx_0_2_stat_rx_broadcast (stat_rx_broadcast_2),
    .stat_rx_0_2_stat_rx_fragment (stat_rx_fragment_2),
    .stat_rx_0_2_stat_rx_framing_err (stat_rx_framing_err_2),
    .stat_rx_0_2_stat_rx_framing_err_valid (stat_rx_framing_err_valid_2),
    .stat_rx_0_2_stat_rx_got_signal_os (stat_rx_got_signal_os_2),
    .stat_rx_0_2_stat_rx_hi_ber (stat_rx_hi_ber_2),
    .stat_rx_0_2_stat_rx_inrangeerr (stat_rx_inrangeerr_2),
    .stat_rx_0_2_stat_rx_internal_local_fault (stat_rx_internal_local_fault_2),
    .stat_rx_0_2_stat_rx_jabber (stat_rx_jabber_2),
    .stat_rx_0_2_stat_rx_local_fault (stat_rx_local_fault_2),
    .stat_rx_0_2_stat_rx_multicast (stat_rx_multicast_2),
    .stat_rx_0_2_stat_rx_oversize (stat_rx_oversize_2),
    .stat_rx_0_2_stat_rx_packet_1024_1518_bytes (stat_rx_packet_1024_1518_bytes_2),
    .stat_rx_0_2_stat_rx_packet_128_255_bytes (stat_rx_packet_128_255_bytes_2),
    .stat_rx_0_2_stat_rx_packet_1519_1522_bytes (stat_rx_packet_1519_1522_bytes_2),
    .stat_rx_0_2_stat_rx_packet_1523_1548_bytes (stat_rx_packet_1523_1548_bytes_2),
    .stat_rx_0_2_stat_rx_packet_1549_2047_bytes (stat_rx_packet_1549_2047_bytes_2),
    .stat_rx_0_2_stat_rx_packet_2048_4095_bytes (stat_rx_packet_2048_4095_bytes_2),
    .stat_rx_0_2_stat_rx_packet_256_511_bytes (stat_rx_packet_256_511_bytes_2),
    .stat_rx_0_2_stat_rx_packet_4096_8191_bytes (stat_rx_packet_4096_8191_bytes_2),
    .stat_rx_0_2_stat_rx_packet_512_1023_bytes (stat_rx_packet_512_1023_bytes_2),
    .stat_rx_0_2_stat_rx_packet_64_bytes (stat_rx_packet_64_bytes_2),
    .stat_rx_0_2_stat_rx_packet_65_127_bytes (stat_rx_packet_65_127_bytes_2),
    .stat_rx_0_2_stat_rx_packet_8192_9215_bytes (stat_rx_packet_8192_9215_bytes_2),
    .stat_rx_0_2_stat_rx_packet_bad_fcs (stat_rx_packet_bad_fcs_2),
    .stat_rx_0_2_stat_rx_packet_large (stat_rx_packet_large_2),
    .stat_rx_0_2_stat_rx_packet_small (stat_rx_packet_small_2),
    .stat_rx_0_2_stat_rx_received_local_fault (stat_rx_received_local_fault_2),
    .stat_rx_0_2_stat_rx_remote_fault (stat_rx_remote_fault_2),
    .stat_rx_0_2_stat_rx_stomped_fcs (stat_rx_stomped_fcs_2),
    .stat_rx_0_2_stat_rx_test_pattern_mismatch (stat_rx_test_pattern_mismatch_2),
    .stat_rx_0_2_stat_rx_toolong (stat_rx_toolong_2),
    .stat_rx_0_2_stat_rx_total_bytes (stat_rx_total_bytes_2),
    .stat_rx_0_2_stat_rx_total_good_bytes (stat_rx_total_good_bytes_2),
    .stat_rx_0_2_stat_rx_total_good_packets (stat_rx_total_good_packets_2),
    .stat_rx_0_2_stat_rx_total_packets (stat_rx_total_packets_2),
    .stat_rx_0_2_stat_rx_truncated (stat_rx_truncated_2),
    .stat_rx_0_2_stat_rx_undersize (stat_rx_undersize_2),
    .stat_rx_0_2_stat_rx_unicast (stat_rx_unicast_2),
    .stat_rx_0_2_stat_rx_valid_ctrl_code (stat_rx_valid_ctrl_code_2),
    .stat_rx_0_2_stat_rx_vlan (stat_rx_vlan_2),
    .stat_rx_0_3_stat_rx_bad_code (stat_rx_bad_code_3),
    .stat_rx_0_3_stat_rx_bad_fcs (stat_rx_bad_fcs_3),
    .stat_rx_0_3_stat_rx_bad_preamble (stat_rx_bad_preamble_3),
    .stat_rx_0_3_stat_rx_bad_sfd (stat_rx_bad_sfd_3),
    .stat_rx_0_3_stat_rx_block_lock (stat_rx_block_lock_3),
    .stat_rx_0_3_stat_rx_broadcast (stat_rx_broadcast_3),
    .stat_rx_0_3_stat_rx_fragment (stat_rx_fragment_3),
    .stat_rx_0_3_stat_rx_framing_err (stat_rx_framing_err_3),
    .stat_rx_0_3_stat_rx_framing_err_valid (stat_rx_framing_err_valid_3),
    .stat_rx_0_3_stat_rx_got_signal_os (stat_rx_got_signal_os_3),
    .stat_rx_0_3_stat_rx_hi_ber (stat_rx_hi_ber_3),
    .stat_rx_0_3_stat_rx_inrangeerr (stat_rx_inrangeerr_3),
    .stat_rx_0_3_stat_rx_internal_local_fault (stat_rx_internal_local_fault_3),
    .stat_rx_0_3_stat_rx_jabber (stat_rx_jabber_3),
    .stat_rx_0_3_stat_rx_local_fault (stat_rx_local_fault_3),
    .stat_rx_0_3_stat_rx_multicast (stat_rx_multicast_3),
    .stat_rx_0_3_stat_rx_oversize (stat_rx_oversize_3),
    .stat_rx_0_3_stat_rx_packet_1024_1518_bytes (stat_rx_packet_1024_1518_bytes_3),
    .stat_rx_0_3_stat_rx_packet_128_255_bytes (stat_rx_packet_128_255_bytes_3),
    .stat_rx_0_3_stat_rx_packet_1519_1522_bytes (stat_rx_packet_1519_1522_bytes_3),
    .stat_rx_0_3_stat_rx_packet_1523_1548_bytes (stat_rx_packet_1523_1548_bytes_3),
    .stat_rx_0_3_stat_rx_packet_1549_2047_bytes (stat_rx_packet_1549_2047_bytes_3),
    .stat_rx_0_3_stat_rx_packet_2048_4095_bytes (stat_rx_packet_2048_4095_bytes_3),
    .stat_rx_0_3_stat_rx_packet_256_511_bytes (stat_rx_packet_256_511_bytes_3),
    .stat_rx_0_3_stat_rx_packet_4096_8191_bytes (stat_rx_packet_4096_8191_bytes_3),
    .stat_rx_0_3_stat_rx_packet_512_1023_bytes (stat_rx_packet_512_1023_bytes_3),
    .stat_rx_0_3_stat_rx_packet_64_bytes (stat_rx_packet_64_bytes_3),
    .stat_rx_0_3_stat_rx_packet_65_127_bytes (stat_rx_packet_65_127_bytes_3),
    .stat_rx_0_3_stat_rx_packet_8192_9215_bytes (stat_rx_packet_8192_9215_bytes_3),
    .stat_rx_0_3_stat_rx_packet_bad_fcs (stat_rx_packet_bad_fcs_3),
    .stat_rx_0_3_stat_rx_packet_large (stat_rx_packet_large_3),
    .stat_rx_0_3_stat_rx_packet_small (stat_rx_packet_small_3),
    .stat_rx_0_3_stat_rx_received_local_fault (stat_rx_received_local_fault_3),
    .stat_rx_0_3_stat_rx_remote_fault (stat_rx_remote_fault_3),
    .stat_rx_0_3_stat_rx_stomped_fcs (stat_rx_stomped_fcs_3),
    .stat_rx_0_3_stat_rx_test_pattern_mismatch (stat_rx_test_pattern_mismatch_3),
    .stat_rx_0_3_stat_rx_toolong (stat_rx_toolong_3),
    .stat_rx_0_3_stat_rx_total_bytes (stat_rx_total_bytes_3),
    .stat_rx_0_3_stat_rx_total_good_bytes (stat_rx_total_good_bytes_3),
    .stat_rx_0_3_stat_rx_total_good_packets (stat_rx_total_good_packets_3),
    .stat_rx_0_3_stat_rx_total_packets (stat_rx_total_packets_3),
    .stat_rx_0_3_stat_rx_truncated (stat_rx_truncated_3),
    .stat_rx_0_3_stat_rx_undersize (stat_rx_undersize_3),
    .stat_rx_0_3_stat_rx_unicast (stat_rx_unicast_3),
    .stat_rx_0_3_stat_rx_valid_ctrl_code (stat_rx_valid_ctrl_code_3),
    .stat_rx_0_3_stat_rx_vlan (stat_rx_vlan_3),

    .stat_rx_status_0_0 (stat_rx_status_0),
    .stat_rx_status_0_1 (stat_rx_status_1),
    .stat_rx_status_0_2 (stat_rx_status_2),
    .stat_rx_status_0_3 (stat_rx_status_3),

    .stat_tx_0_0_stat_tx_bad_fcs (stat_tx_bad_fcs_0),
    .stat_tx_0_0_stat_tx_broadcast (stat_tx_broadcast_0),
    .stat_tx_0_0_stat_tx_frame_error (stat_tx_frame_error_0),
    .stat_tx_0_0_stat_tx_local_fault (stat_tx_local_fault_0),
    .stat_tx_0_0_stat_tx_multicast (stat_tx_multicast_0),
    .stat_tx_0_0_stat_tx_packet_1024_1518_bytes (stat_tx_packet_1024_1518_bytes_0),
    .stat_tx_0_0_stat_tx_packet_128_255_bytes (stat_tx_packet_128_255_bytes_0),
    .stat_tx_0_0_stat_tx_packet_1519_1522_bytes (stat_tx_packet_1519_1522_bytes_0),
    .stat_tx_0_0_stat_tx_packet_1523_1548_bytes (stat_tx_packet_1523_1548_bytes_0),
    .stat_tx_0_0_stat_tx_packet_1549_2047_bytes (stat_tx_packet_1549_2047_bytes_0),
    .stat_tx_0_0_stat_tx_packet_2048_4095_bytes (stat_tx_packet_2048_4095_bytes_0),
    .stat_tx_0_0_stat_tx_packet_256_511_bytes (stat_tx_packet_256_511_bytes_0),
    .stat_tx_0_0_stat_tx_packet_4096_8191_bytes (stat_tx_packet_4096_8191_bytes_0),
    .stat_tx_0_0_stat_tx_packet_512_1023_bytes (stat_tx_packet_512_1023_bytes_0),
    .stat_tx_0_0_stat_tx_packet_64_bytes (stat_tx_packet_64_bytes_0),
    .stat_tx_0_0_stat_tx_packet_65_127_bytes (stat_tx_packet_65_127_bytes_0),
    .stat_tx_0_0_stat_tx_packet_8192_9215_bytes (stat_tx_packet_8192_9215_bytes_0),
    .stat_tx_0_0_stat_tx_packet_large (stat_tx_packet_large_0),
    .stat_tx_0_0_stat_tx_packet_small (stat_tx_packet_small_0),
    .stat_tx_0_0_stat_tx_total_bytes (stat_tx_total_bytes_0),
    .stat_tx_0_0_stat_tx_total_good_bytes (stat_tx_total_good_bytes_0),
    .stat_tx_0_0_stat_tx_total_good_packets (stat_tx_total_good_packets_0),
    .stat_tx_0_0_stat_tx_total_packets (stat_tx_total_packets_0),
    .stat_tx_0_0_stat_tx_unicast (stat_tx_unicast_0),
    .stat_tx_0_0_stat_tx_vlan (stat_tx_vlan_0),
    .stat_tx_0_1_stat_tx_bad_fcs (stat_tx_bad_fcs_1),
    .stat_tx_0_1_stat_tx_broadcast (stat_tx_broadcast_1),
    .stat_tx_0_1_stat_tx_frame_error (stat_tx_frame_error_1),
    .stat_tx_0_1_stat_tx_local_fault (stat_tx_local_fault_1),
    .stat_tx_0_1_stat_tx_multicast (stat_tx_multicast_1),
    .stat_tx_0_1_stat_tx_packet_1024_1518_bytes (stat_tx_packet_1024_1518_bytes_1),
    .stat_tx_0_1_stat_tx_packet_128_255_bytes (stat_tx_packet_128_255_bytes_1),
    .stat_tx_0_1_stat_tx_packet_1519_1522_bytes (stat_tx_packet_1519_1522_bytes_1),
    .stat_tx_0_1_stat_tx_packet_1523_1548_bytes (stat_tx_packet_1523_1548_bytes_1),
    .stat_tx_0_1_stat_tx_packet_1549_2047_bytes (stat_tx_packet_1549_2047_bytes_1),
    .stat_tx_0_1_stat_tx_packet_2048_4095_bytes (stat_tx_packet_2048_4095_bytes_1),
    .stat_tx_0_1_stat_tx_packet_256_511_bytes (stat_tx_packet_256_511_bytes_1),
    .stat_tx_0_1_stat_tx_packet_4096_8191_bytes (stat_tx_packet_4096_8191_bytes_1),
    .stat_tx_0_1_stat_tx_packet_512_1023_bytes (stat_tx_packet_512_1023_bytes_1),
    .stat_tx_0_1_stat_tx_packet_64_bytes (stat_tx_packet_64_bytes_1),
    .stat_tx_0_1_stat_tx_packet_65_127_bytes (stat_tx_packet_65_127_bytes_1),
    .stat_tx_0_1_stat_tx_packet_8192_9215_bytes (stat_tx_packet_8192_9215_bytes_1),
    .stat_tx_0_1_stat_tx_packet_large (stat_tx_packet_large_1),
    .stat_tx_0_1_stat_tx_packet_small (stat_tx_packet_small_1),
    .stat_tx_0_1_stat_tx_total_bytes (stat_tx_total_bytes_1),
    .stat_tx_0_1_stat_tx_total_good_bytes (stat_tx_total_good_bytes_1),
    .stat_tx_0_1_stat_tx_total_good_packets (stat_tx_total_good_packets_1),
    .stat_tx_0_1_stat_tx_total_packets (stat_tx_total_packets_1),
    .stat_tx_0_1_stat_tx_unicast (stat_tx_unicast_1),
    .stat_tx_0_1_stat_tx_vlan (stat_tx_vlan_1),
    .stat_tx_0_2_stat_tx_bad_fcs (stat_tx_bad_fcs_2),
    .stat_tx_0_2_stat_tx_broadcast (stat_tx_broadcast_2),
    .stat_tx_0_2_stat_tx_frame_error (stat_tx_frame_error_2),
    .stat_tx_0_2_stat_tx_local_fault (stat_tx_local_fault_2),
    .stat_tx_0_2_stat_tx_multicast (stat_tx_multicast_2),
    .stat_tx_0_2_stat_tx_packet_1024_1518_bytes (stat_tx_packet_1024_1518_bytes_2),
    .stat_tx_0_2_stat_tx_packet_128_255_bytes (stat_tx_packet_128_255_bytes_2),
    .stat_tx_0_2_stat_tx_packet_1519_1522_bytes (stat_tx_packet_1519_1522_bytes_2),
    .stat_tx_0_2_stat_tx_packet_1523_1548_bytes (stat_tx_packet_1523_1548_bytes_2),
    .stat_tx_0_2_stat_tx_packet_1549_2047_bytes (stat_tx_packet_1549_2047_bytes_2),
    .stat_tx_0_2_stat_tx_packet_2048_4095_bytes (stat_tx_packet_2048_4095_bytes_2),
    .stat_tx_0_2_stat_tx_packet_256_511_bytes (stat_tx_packet_256_511_bytes_2),
    .stat_tx_0_2_stat_tx_packet_4096_8191_bytes (stat_tx_packet_4096_8191_bytes_2),
    .stat_tx_0_2_stat_tx_packet_512_1023_bytes (stat_tx_packet_512_1023_bytes_2),
    .stat_tx_0_2_stat_tx_packet_64_bytes (stat_tx_packet_64_bytes_2),
    .stat_tx_0_2_stat_tx_packet_65_127_bytes (stat_tx_packet_65_127_bytes_2),
    .stat_tx_0_2_stat_tx_packet_8192_9215_bytes (stat_tx_packet_8192_9215_bytes_2),
    .stat_tx_0_2_stat_tx_packet_large (stat_tx_packet_large_2),
    .stat_tx_0_2_stat_tx_packet_small (stat_tx_packet_small_2),
    .stat_tx_0_2_stat_tx_total_bytes (stat_tx_total_bytes_2),
    .stat_tx_0_2_stat_tx_total_good_bytes (stat_tx_total_good_bytes_2),
    .stat_tx_0_2_stat_tx_total_good_packets (stat_tx_total_good_packets_2),
    .stat_tx_0_2_stat_tx_total_packets (stat_tx_total_packets_2),
    .stat_tx_0_2_stat_tx_unicast (stat_tx_unicast_2),
    .stat_tx_0_2_stat_tx_vlan (stat_tx_vlan_2),
    .stat_tx_0_3_stat_tx_bad_fcs (stat_tx_bad_fcs_3),
    .stat_tx_0_3_stat_tx_broadcast (stat_tx_broadcast_3),
    .stat_tx_0_3_stat_tx_frame_error (stat_tx_frame_error_3),
    .stat_tx_0_3_stat_tx_local_fault (stat_tx_local_fault_3),
    .stat_tx_0_3_stat_tx_multicast (stat_tx_multicast_3),
    .stat_tx_0_3_stat_tx_packet_1024_1518_bytes (stat_tx_packet_1024_1518_bytes_3),
    .stat_tx_0_3_stat_tx_packet_128_255_bytes (stat_tx_packet_128_255_bytes_3),
    .stat_tx_0_3_stat_tx_packet_1519_1522_bytes (stat_tx_packet_1519_1522_bytes_3),
    .stat_tx_0_3_stat_tx_packet_1523_1548_bytes (stat_tx_packet_1523_1548_bytes_3),
    .stat_tx_0_3_stat_tx_packet_1549_2047_bytes (stat_tx_packet_1549_2047_bytes_3),
    .stat_tx_0_3_stat_tx_packet_2048_4095_bytes (stat_tx_packet_2048_4095_bytes_3),
    .stat_tx_0_3_stat_tx_packet_256_511_bytes (stat_tx_packet_256_511_bytes_3),
    .stat_tx_0_3_stat_tx_packet_4096_8191_bytes (stat_tx_packet_4096_8191_bytes_3),
    .stat_tx_0_3_stat_tx_packet_512_1023_bytes (stat_tx_packet_512_1023_bytes_3),
    .stat_tx_0_3_stat_tx_packet_64_bytes (stat_tx_packet_64_bytes_3),
    .stat_tx_0_3_stat_tx_packet_65_127_bytes (stat_tx_packet_65_127_bytes_3),
    .stat_tx_0_3_stat_tx_packet_8192_9215_bytes (stat_tx_packet_8192_9215_bytes_3),
    .stat_tx_0_3_stat_tx_packet_large (stat_tx_packet_large_3),
    .stat_tx_0_3_stat_tx_packet_small (stat_tx_packet_small_3),
    .stat_tx_0_3_stat_tx_total_bytes (stat_tx_total_bytes_3),
    .stat_tx_0_3_stat_tx_total_good_bytes (stat_tx_total_good_bytes_3),
    .stat_tx_0_3_stat_tx_total_good_packets (stat_tx_total_good_packets_3),
    .stat_tx_0_3_stat_tx_total_packets (stat_tx_total_packets_3),
    .stat_tx_0_3_stat_tx_unicast (stat_tx_unicast_3),
    .stat_tx_0_3_stat_tx_vlan (stat_tx_vlan_3),

    .tx_core_clk_0_0 (tx_clk_out_0),
    .tx_core_clk_0_1 (tx_clk_out_1),
    .tx_core_clk_0_2 (tx_clk_out_2),
    .tx_core_clk_0_3 (tx_clk_out_3),
    .tx_preamblein_0_0 (tx_preamblein_0),
    .tx_preamblein_0_1 (tx_preamblein_1),
    .tx_preamblein_0_2 (tx_preamblein_2),
    .tx_preamblein_0_3 (tx_preamblein_3),
    .tx_reset_0_0 (user_tx_reset_0),
    .tx_reset_0_1 (user_tx_reset_1),
    .tx_reset_0_2 (user_tx_reset_2),
    .tx_reset_0_3 (user_tx_reset_3),
    .tx_resetdone_out_0_0 (tx_resetdone_out_0),
    .tx_resetdone_out_0_1 (tx_resetdone_out_1),
    .tx_resetdone_out_0_2 (tx_resetdone_out_2),
    .tx_resetdone_out_0_3 (tx_resetdone_out_3),
    .tx_unfout_0_0 (tx_unfout_0),
    .tx_unfout_0_1 (tx_unfout_1),
    .tx_unfout_0_2 (tx_unfout_2),
    .tx_unfout_0_3 (tx_unfout_3),
    .tx_usrclk_0 (tx_clk_out_0),
    .tx_usrclk_1 (tx_clk_out_1),
    .tx_usrclk_2 (tx_clk_out_2),
    .tx_usrclk_3 (tx_clk_out_3),
    .user_reg0_0_0 (user_reg0_0),
    .user_reg0_0_1 (user_reg0_1),
    .user_reg0_0_2 (user_reg0_2),
    .user_reg0_0_3 (user_reg0_3)

);

endmodule : xbtest_sub_xxv_gt_wrapper

(* DowngradeIPIdentifiedWarnings="yes" *)
module cdc_sync_2stage
#(
 parameter WIDTH  = 1
)
(
 input  clk,
 input  [WIDTH-1:0] signal_in,
 output wire [WIDTH-1:0]  signal_out
);

                          wire [WIDTH-1:0] sig_in_cdc_from;
 (* ASYNC_REG = "TRUE" *) reg  [WIDTH-1:0] s_out_d2_cdc_to;
 (* ASYNC_REG = "TRUE" *) reg  [WIDTH-1:0] data_out_d3;

assign sig_in_cdc_from = signal_in;
assign signal_out      = data_out_d3;

always @(posedge clk)
begin
  s_out_d2_cdc_to  <= sig_in_cdc_from;
  data_out_d3      <= s_out_d2_cdc_to;
end

endmodule


`default_nettype wire
