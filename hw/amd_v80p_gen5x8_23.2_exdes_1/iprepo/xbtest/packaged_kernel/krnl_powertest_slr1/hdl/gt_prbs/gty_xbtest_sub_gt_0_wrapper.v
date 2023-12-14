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


// HOW TO create this file
//
// open vivado with VCK5000 board, use the following command
//      create_bd_design "xbtest_sub_gt_0"
//      use bd.tcl and port.tcl for versal and VCK5000
//
//  save the project
//      right click on the bd in the Sources window => create HDL wrapper (select let vivado name wrapper and auto-update)

`timescale 1 ps / 1 ps

module gt_1bridge_4lanes_wrapper
   (GT_Serial_grx_n,
    GT_Serial_grx_p,
    GT_Serial_gtx_n,
    GT_Serial_gtx_p,
    apb3clk_gt_bridge_ip_0,
    apb3clk_quad,
    ch0_rxdata_ext,
    ch0_rxdatavalid_ext,
    ch0_rxgearboxslip_ext,
    ch0_rxheader_ext,
    ch0_rxheadervalid_ext,
    ch0_rxstartofseq_ext,
    ch0_txdata_ext,
    ch0_txheader_ext,
    ch0_txsequence_ext,
    ch1_rxdata_ext,
    ch1_rxdatavalid_ext,
    ch1_rxgearboxslip_ext,
    ch1_rxheader_ext,
    ch1_rxheadervalid_ext,
    ch1_rxstartofseq_ext,
    ch1_txdata_ext,
    ch1_txheader_ext,
    ch1_txsequence_ext,
    ch2_rxdata_ext,
    ch2_rxdatavalid_ext,
    ch2_rxgearboxslip_ext,
    ch2_rxheader_ext,
    ch2_rxheadervalid_ext,
    ch2_rxstartofseq_ext,
    ch2_txdata_ext,
    ch2_txheader_ext,
    ch2_txsequence_ext,
    ch3_rxdata_ext,
    ch3_rxdatavalid_ext,
    ch3_rxgearboxslip_ext,
    ch3_rxheader_ext,
    ch3_rxheadervalid_ext,
    ch3_rxstartofseq_ext,
    ch3_txdata_ext,
    ch3_txheader_ext,
    ch3_txsequence_ext,
    gt_bridge_ip_0_diff_gt_ref_clock_clk_n,
    gt_bridge_ip_0_diff_gt_ref_clock_clk_p,
    gt_powergood,
    gt_reset_gt_bridge_ip_0,
    hsclk0_lcplllock_0,
    hsclk0_rplllock_0,
    hsclk1_lcplllock_0,
    hsclk1_rplllock_0,
    lane_loopback_0,
    lane_loopback_1,
    lane_loopback_2,
    lane_loopback_3,
    lane_rxlpmen_0,
    lane_rxlpmen_1,
    lane_rxlpmen_2,
    lane_rxlpmen_3,
    lane_txdiffctrl_0,
    lane_txdiffctrl_1,
    lane_txdiffctrl_2,
    lane_txdiffctrl_3,
    lane_txmaincursor_0,
    lane_txmaincursor_1,
    lane_txmaincursor_2,
    lane_txmaincursor_3,
    lane_txpolarity_0,
    lane_txpolarity_1,
    lane_txpolarity_2,
    lane_txpolarity_3,
    lane_txpostcursor_0,
    lane_txpostcursor_1,
    lane_txpostcursor_2,
    lane_txpostcursor_3,
    lane_txprecursor_0,
    lane_txprecursor_1,
    lane_txprecursor_2,
    lane_txprecursor_3,
    link_status_gt_bridge_ip_0,
    rate_sel_gt_bridge_ip_0,
    reset_rx_datapath_in,
    reset_rx_pll_and_datapath_in,
    reset_tx_datapath_in,
    reset_tx_pll_and_datapath_in,
    rx_resetdone_out_gt_bridge_ip_0,
    rxusrclk_gt_bridge_ip_0,
    tx_resetdone_out_gt_bridge_ip_0,
    txusrclk_gt_bridge_ip_0);
  input [3:0]GT_Serial_grx_n;
  input [3:0]GT_Serial_grx_p;
  output [3:0]GT_Serial_gtx_n;
  output [3:0]GT_Serial_gtx_p;
  input apb3clk_gt_bridge_ip_0;
  input apb3clk_quad;
  output [127:0]ch0_rxdata_ext;
  output [1:0]ch0_rxdatavalid_ext;
  input ch0_rxgearboxslip_ext;
  output [5:0]ch0_rxheader_ext;
  output [1:0]ch0_rxheadervalid_ext;
  output [1:0]ch0_rxstartofseq_ext;
  input [127:0]ch0_txdata_ext;
  input [5:0]ch0_txheader_ext;
  input [6:0]ch0_txsequence_ext;
  output [127:0]ch1_rxdata_ext;
  output [1:0]ch1_rxdatavalid_ext;
  input ch1_rxgearboxslip_ext;
  output [5:0]ch1_rxheader_ext;
  output [1:0]ch1_rxheadervalid_ext;
  output [1:0]ch1_rxstartofseq_ext;
  input [127:0]ch1_txdata_ext;
  input [5:0]ch1_txheader_ext;
  input [6:0]ch1_txsequence_ext;
  output [127:0]ch2_rxdata_ext;
  output [1:0]ch2_rxdatavalid_ext;
  input ch2_rxgearboxslip_ext;
  output [5:0]ch2_rxheader_ext;
  output [1:0]ch2_rxheadervalid_ext;
  output [1:0]ch2_rxstartofseq_ext;
  input [127:0]ch2_txdata_ext;
  input [5:0]ch2_txheader_ext;
  input [6:0]ch2_txsequence_ext;
  output [127:0]ch3_rxdata_ext;
  output [1:0]ch3_rxdatavalid_ext;
  input ch3_rxgearboxslip_ext;
  output [5:0]ch3_rxheader_ext;
  output [1:0]ch3_rxheadervalid_ext;
  output [1:0]ch3_rxstartofseq_ext;
  input [127:0]ch3_txdata_ext;
  input [5:0]ch3_txheader_ext;
  input [6:0]ch3_txsequence_ext;
  input [0:0]gt_bridge_ip_0_diff_gt_ref_clock_clk_n;
  input [0:0]gt_bridge_ip_0_diff_gt_ref_clock_clk_p;
  output gt_powergood;
  input gt_reset_gt_bridge_ip_0;
  output hsclk0_lcplllock_0;
  output hsclk0_rplllock_0;
  output hsclk1_lcplllock_0;
  output hsclk1_rplllock_0;
  input [2:0]lane_loopback_0;
  input [2:0]lane_loopback_1;
  input [2:0]lane_loopback_2;
  input [2:0]lane_loopback_3;
  input lane_rxlpmen_0;
  input lane_rxlpmen_1;
  input lane_rxlpmen_2;
  input lane_rxlpmen_3;
  input [4:0]lane_txdiffctrl_0;
  input [4:0]lane_txdiffctrl_1;
  input [4:0]lane_txdiffctrl_2;
  input [4:0]lane_txdiffctrl_3;
  input [6:0]lane_txmaincursor_0;
  input [6:0]lane_txmaincursor_1;
  input [6:0]lane_txmaincursor_2;
  input [6:0]lane_txmaincursor_3;
  input lane_txpolarity_0;
  input lane_txpolarity_1;
  input lane_txpolarity_2;
  input lane_txpolarity_3;
  input [4:0]lane_txpostcursor_0;
  input [4:0]lane_txpostcursor_1;
  input [4:0]lane_txpostcursor_2;
  input [4:0]lane_txpostcursor_3;
  input [4:0]lane_txprecursor_0;
  input [4:0]lane_txprecursor_1;
  input [4:0]lane_txprecursor_2;
  input [4:0]lane_txprecursor_3;
  output link_status_gt_bridge_ip_0;
  input [3:0]rate_sel_gt_bridge_ip_0;
  input reset_rx_datapath_in;
  input reset_rx_pll_and_datapath_in;
  input reset_tx_datapath_in;
  input reset_tx_pll_and_datapath_in;
  output rx_resetdone_out_gt_bridge_ip_0;
  output rxusrclk_gt_bridge_ip_0;
  output tx_resetdone_out_gt_bridge_ip_0;
  output txusrclk_gt_bridge_ip_0;

  wire [3:0]GT_Serial_grx_n;
  wire [3:0]GT_Serial_grx_p;
  wire [3:0]GT_Serial_gtx_n;
  wire [3:0]GT_Serial_gtx_p;
  wire apb3clk_gt_bridge_ip_0;
  wire apb3clk_quad;
  wire [127:0]ch0_rxdata_ext;
  wire [1:0]ch0_rxdatavalid_ext;
  wire ch0_rxgearboxslip_ext;
  wire [5:0]ch0_rxheader_ext;
  wire [1:0]ch0_rxheadervalid_ext;
  wire [1:0]ch0_rxstartofseq_ext;
  wire [127:0]ch0_txdata_ext;
  wire [5:0]ch0_txheader_ext;
  wire [6:0]ch0_txsequence_ext;
  wire [127:0]ch1_rxdata_ext;
  wire [1:0]ch1_rxdatavalid_ext;
  wire ch1_rxgearboxslip_ext;
  wire [5:0]ch1_rxheader_ext;
  wire [1:0]ch1_rxheadervalid_ext;
  wire [1:0]ch1_rxstartofseq_ext;
  wire [127:0]ch1_txdata_ext;
  wire [5:0]ch1_txheader_ext;
  wire [6:0]ch1_txsequence_ext;
  wire [127:0]ch2_rxdata_ext;
  wire [1:0]ch2_rxdatavalid_ext;
  wire ch2_rxgearboxslip_ext;
  wire [5:0]ch2_rxheader_ext;
  wire [1:0]ch2_rxheadervalid_ext;
  wire [1:0]ch2_rxstartofseq_ext;
  wire [127:0]ch2_txdata_ext;
  wire [5:0]ch2_txheader_ext;
  wire [6:0]ch2_txsequence_ext;
  wire [127:0]ch3_rxdata_ext;
  wire [1:0]ch3_rxdatavalid_ext;
  wire ch3_rxgearboxslip_ext;
  wire [5:0]ch3_rxheader_ext;
  wire [1:0]ch3_rxheadervalid_ext;
  wire [1:0]ch3_rxstartofseq_ext;
  wire [127:0]ch3_txdata_ext;
  wire [5:0]ch3_txheader_ext;
  wire [6:0]ch3_txsequence_ext;
  wire [0:0]gt_bridge_ip_0_diff_gt_ref_clock_clk_n;
  wire [0:0]gt_bridge_ip_0_diff_gt_ref_clock_clk_p;
  wire gt_powergood;
  wire gt_reset_gt_bridge_ip_0;
  wire hsclk0_lcplllock_0;
  wire hsclk0_rplllock_0;
  wire hsclk1_lcplllock_0;
  wire hsclk1_rplllock_0;
  wire [2:0]lane_loopback_0;
  wire [2:0]lane_loopback_1;
  wire [2:0]lane_loopback_2;
  wire [2:0]lane_loopback_3;
  wire lane_rxlpmen_0;
  wire lane_rxlpmen_1;
  wire lane_rxlpmen_2;
  wire lane_rxlpmen_3;
  wire [4:0]lane_txdiffctrl_0;
  wire [4:0]lane_txdiffctrl_1;
  wire [4:0]lane_txdiffctrl_2;
  wire [4:0]lane_txdiffctrl_3;
  wire [6:0]lane_txmaincursor_0;
  wire [6:0]lane_txmaincursor_1;
  wire [6:0]lane_txmaincursor_2;
  wire [6:0]lane_txmaincursor_3;
  wire lane_txpolarity_0;
  wire lane_txpolarity_1;
  wire lane_txpolarity_2;
  wire lane_txpolarity_3;
  wire [4:0]lane_txpostcursor_0;
  wire [4:0]lane_txpostcursor_1;
  wire [4:0]lane_txpostcursor_2;
  wire [4:0]lane_txpostcursor_3;
  wire [4:0]lane_txprecursor_0;
  wire [4:0]lane_txprecursor_1;
  wire [4:0]lane_txprecursor_2;
  wire [4:0]lane_txprecursor_3;
  wire link_status_gt_bridge_ip_0;
  wire [3:0]rate_sel_gt_bridge_ip_0;
  wire reset_rx_datapath_in;
  wire reset_rx_pll_and_datapath_in;
  wire reset_tx_datapath_in;
  wire reset_tx_pll_and_datapath_in;
  wire rx_resetdone_out_gt_bridge_ip_0;
  wire rxusrclk_gt_bridge_ip_0;
  wire tx_resetdone_out_gt_bridge_ip_0;
  wire txusrclk_gt_bridge_ip_0;

  gty_xbtest_sub_gt_0 gty_xbtest_sub_gt_0_i
       (.GT_Serial_grx_n(GT_Serial_grx_n),
        .GT_Serial_grx_p(GT_Serial_grx_p),
        .GT_Serial_gtx_n(GT_Serial_gtx_n),
        .GT_Serial_gtx_p(GT_Serial_gtx_p),
        .apb3clk_gt_bridge_ip_0(apb3clk_gt_bridge_ip_0),
        .apb3clk_quad(apb3clk_quad),
        .ch0_rxdata_ext(ch0_rxdata_ext),
        .ch0_rxdatavalid_ext(ch0_rxdatavalid_ext),
        .ch0_rxgearboxslip_ext(ch0_rxgearboxslip_ext),
        .ch0_rxheader_ext(ch0_rxheader_ext),
        .ch0_rxheadervalid_ext(ch0_rxheadervalid_ext),
        .ch0_rxstartofseq_ext(ch0_rxstartofseq_ext),
        .ch0_txdata_ext(ch0_txdata_ext),
        .ch0_txheader_ext(ch0_txheader_ext),
        .ch0_txsequence_ext(ch0_txsequence_ext),
        .ch1_rxdata_ext(ch1_rxdata_ext),
        .ch1_rxdatavalid_ext(ch1_rxdatavalid_ext),
        .ch1_rxgearboxslip_ext(ch1_rxgearboxslip_ext),
        .ch1_rxheader_ext(ch1_rxheader_ext),
        .ch1_rxheadervalid_ext(ch1_rxheadervalid_ext),
        .ch1_rxstartofseq_ext(ch1_rxstartofseq_ext),
        .ch1_txdata_ext(ch1_txdata_ext),
        .ch1_txheader_ext(ch1_txheader_ext),
        .ch1_txsequence_ext(ch1_txsequence_ext),
        .ch2_rxdata_ext(ch2_rxdata_ext),
        .ch2_rxdatavalid_ext(ch2_rxdatavalid_ext),
        .ch2_rxgearboxslip_ext(ch2_rxgearboxslip_ext),
        .ch2_rxheader_ext(ch2_rxheader_ext),
        .ch2_rxheadervalid_ext(ch2_rxheadervalid_ext),
        .ch2_rxstartofseq_ext(ch2_rxstartofseq_ext),
        .ch2_txdata_ext(ch2_txdata_ext),
        .ch2_txheader_ext(ch2_txheader_ext),
        .ch2_txsequence_ext(ch2_txsequence_ext),
        .ch3_rxdata_ext(ch3_rxdata_ext),
        .ch3_rxdatavalid_ext(ch3_rxdatavalid_ext),
        .ch3_rxgearboxslip_ext(ch3_rxgearboxslip_ext),
        .ch3_rxheader_ext(ch3_rxheader_ext),
        .ch3_rxheadervalid_ext(ch3_rxheadervalid_ext),
        .ch3_rxstartofseq_ext(ch3_rxstartofseq_ext),
        .ch3_txdata_ext(ch3_txdata_ext),
        .ch3_txheader_ext(ch3_txheader_ext),
        .ch3_txsequence_ext(ch3_txsequence_ext),
        .gt_bridge_ip_0_diff_gt_ref_clock_clk_n(gt_bridge_ip_0_diff_gt_ref_clock_clk_n),
        .gt_bridge_ip_0_diff_gt_ref_clock_clk_p(gt_bridge_ip_0_diff_gt_ref_clock_clk_p),
        .gt_powergood(gt_powergood),
        .gt_reset_gt_bridge_ip_0(gt_reset_gt_bridge_ip_0),
        .hsclk0_lcplllock_0(hsclk0_lcplllock_0),
        .hsclk0_rplllock_0(hsclk0_rplllock_0),
        .hsclk1_lcplllock_0(hsclk1_lcplllock_0),
        .hsclk1_rplllock_0(hsclk1_rplllock_0),
        .lane_loopback_0(lane_loopback_0),
        .lane_loopback_1(lane_loopback_1),
        .lane_loopback_2(lane_loopback_2),
        .lane_loopback_3(lane_loopback_3),
        .lane_rxlpmen_0(lane_rxlpmen_0),
        .lane_rxlpmen_1(lane_rxlpmen_1),
        .lane_rxlpmen_2(lane_rxlpmen_2),
        .lane_rxlpmen_3(lane_rxlpmen_3),
        .lane_txdiffctrl_0(lane_txdiffctrl_0),
        .lane_txdiffctrl_1(lane_txdiffctrl_1),
        .lane_txdiffctrl_2(lane_txdiffctrl_2),
        .lane_txdiffctrl_3(lane_txdiffctrl_3),
        .lane_txmaincursor_0(lane_txmaincursor_0),
        .lane_txmaincursor_1(lane_txmaincursor_1),
        .lane_txmaincursor_2(lane_txmaincursor_2),
        .lane_txmaincursor_3(lane_txmaincursor_3),
        .lane_txpolarity_0(lane_txpolarity_0),
        .lane_txpolarity_1(lane_txpolarity_1),
        .lane_txpolarity_2(lane_txpolarity_2),
        .lane_txpolarity_3(lane_txpolarity_3),
        .lane_txpostcursor_0(lane_txpostcursor_0),
        .lane_txpostcursor_1(lane_txpostcursor_1),
        .lane_txpostcursor_2(lane_txpostcursor_2),
        .lane_txpostcursor_3(lane_txpostcursor_3),
        .lane_txprecursor_0(lane_txprecursor_0),
        .lane_txprecursor_1(lane_txprecursor_1),
        .lane_txprecursor_2(lane_txprecursor_2),
        .lane_txprecursor_3(lane_txprecursor_3),
        .link_status_gt_bridge_ip_0(link_status_gt_bridge_ip_0),
        .rate_sel_gt_bridge_ip_0(rate_sel_gt_bridge_ip_0),
        .reset_rx_datapath_in(reset_rx_datapath_in),
        .reset_rx_pll_and_datapath_in(reset_rx_pll_and_datapath_in),
        .reset_tx_datapath_in(reset_tx_datapath_in),
        .reset_tx_pll_and_datapath_in(reset_tx_pll_and_datapath_in),
        .rx_resetdone_out_gt_bridge_ip_0(rx_resetdone_out_gt_bridge_ip_0),
        .rxusrclk_gt_bridge_ip_0(rxusrclk_gt_bridge_ip_0),
        .tx_resetdone_out_gt_bridge_ip_0(tx_resetdone_out_gt_bridge_ip_0),
        .txusrclk_gt_bridge_ip_0(txusrclk_gt_bridge_ip_0));
endmodule
