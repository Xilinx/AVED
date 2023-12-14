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

`timescale 1fs/1fs
`default_nettype none
module gtf_wiz_10_raw_example_top # (
  parameter  integer     NUM_CHANNEL = 4,
  parameter integer DEST_SYNC_FF = 4
)
(

     // System Signals
    input  wire         refclk_in,
    input  wire         freerun_clk,

    output wire [3 : 0]   gtf_ch_gtftxn,
    output wire [3 : 0]   gtf_ch_gtftxp,
    input  wire [3 : 0]   gtf_ch_gtfrxn,
    input  wire [3 : 0]   gtf_ch_gtfrxp,

    output wire [0 : 0]   gt_recov_clk_n,
    output wire [0 : 0]   gt_recov_clk_p,
    output wire           gt_recov_clk,

    input wire [11 : 0]   loopback,
    input wire [3 : 0]    rxlpmen,
    input wire [3 : 0]    rxpolarity,
    input wire [3 : 0]    txpolarity,
    input wire [19 : 0]   txdiffctrl,
    input wire [27 : 0]   txmaincursor,
    input wire [19 : 0]   txpostcursor,
    input wire [19 : 0]   txprecursor,

    input wire [3 : 0]    gtwiz_reset_all,
    input wire [3 : 0]    gtwiz_reset_tx_pll_and_datapath,
    input wire [3 : 0]    gtwiz_reset_rx_pll_and_datapath,
    input wire [3 : 0]    gtwiz_reset_tx_datapath,
    input wire [3 : 0]    gtwiz_reset_rx_datapath,

    output wire             gtf_qpll0_lock,
    output wire [3 : 0]     gt_power_good,
    output wire [3 : 0]     gtwiz_reset_tx_done,
    output wire [3 : 0]     gtwiz_reset_rx_done,
    output wire [3 : 0]     gtwiz_bufferbypass_tx_done,
    output wire [3 : 0]     gtwiz_bufferbypass_rx_done,

    output wire [3 : 0]     gtf_ch_txresetdone,
    output wire [3 : 0]     gtf_ch_txsyncdone,
    output wire [3 : 0]     gtf_ch_rxresetdone,
    output wire [3 : 0]     gtf_ch_rxsyncdone,

    output  wire [3 : 0]    gtf_txusrclk2_out,
    output  wire [3 : 0]    gtf_rxusrclk2_out,

    input   wire [39 : 0]   tx_raw_data_0,
    input   wire [39 : 0]   tx_raw_data_1,
    input   wire [39 : 0]   tx_raw_data_2,
    input   wire [39 : 0]   tx_raw_data_3,

    output  wire [39 : 0]   rx_raw_data_0,
    output  wire [39 : 0]   rx_raw_data_1,
    output  wire [39 : 0]   rx_raw_data_2,
    output  wire [39 : 0]   rx_raw_data_3

);

  `include "gtf_wiz_10_raw_rules_output.vh"


  wire      [1:0]   gtf_cm_rxrecclk0sel;
  wire  [3:0]       gtf_ch_rxoutclk;

  wire [39:0]     tx_raw_data [0:3];
  wire [39:0]     rx_raw_data [0:3];

  //assign tx_raw_data = {tx_raw_data_0, tx_raw_data_1, tx_raw_data_2, tx_raw_data_3};
  assign tx_raw_data[0] = tx_raw_data_0;
  assign tx_raw_data[1] = tx_raw_data_1;
  assign tx_raw_data[2] = tx_raw_data_2;
  assign tx_raw_data[3] = tx_raw_data_3;
  //assign {rx_raw_data_0, rx_raw_data_1, rx_raw_data_2, rx_raw_data_3 } = rx_raw_data;
  assign rx_raw_data_0 = rx_raw_data[0];
  assign rx_raw_data_1 = rx_raw_data[1];
  assign rx_raw_data_2 = rx_raw_data[2];
  assign rx_raw_data_3 = rx_raw_data[3];

wire  [2:0]       gtf_ch_loopback    [0:3];
wire  [3:0]       gtf_ch_rxlpmen;
wire  [3:0]       gtf_ch_rxpolarity;
wire  [3:0]       gtf_ch_txpolarity;
wire  [4:0]       gtf_ch_txdiffctrl  [0:3];
wire  [6:0]       gtf_ch_txmaincursor[0:3];
wire  [4:0]       gtf_ch_txpostcursor[0:3];
wire  [4:0]       gtf_ch_txprecursor [0:3];

assign gtf_ch_loopback[0] = loopback[2:0];
assign gtf_ch_loopback[1] = loopback[5:3];
assign gtf_ch_loopback[2] = loopback[8:6];
assign gtf_ch_loopback[3] = loopback[11:9];

assign gtf_ch_rxpolarity    = rxpolarity;
assign gtf_ch_txpolarity    = txpolarity;

localparam integer FIX_GTF_ANALOG_CFG = 0;

assign gtf_ch_rxlpmen       = (FIX_GTF_ANALOG_CFG == 1)? {4{RXLPMEN}} : rxlpmen;

assign gtf_ch_txdiffctrl[0] = (FIX_GTF_ANALOG_CFG == 1)? A_TXDIFFCTRL : txdiffctrl[4:0];
assign gtf_ch_txdiffctrl[1] = (FIX_GTF_ANALOG_CFG == 1)? A_TXDIFFCTRL : txdiffctrl[9:5];
assign gtf_ch_txdiffctrl[2] = (FIX_GTF_ANALOG_CFG == 1)? A_TXDIFFCTRL : txdiffctrl[14:10];
assign gtf_ch_txdiffctrl[3] = (FIX_GTF_ANALOG_CFG == 1)? A_TXDIFFCTRL : txdiffctrl[19:15];

assign gtf_ch_txmaincursor[0] = (FIX_GTF_ANALOG_CFG == 1)? TXMAINCURSOR : txmaincursor[6:0];
assign gtf_ch_txmaincursor[1] = (FIX_GTF_ANALOG_CFG == 1)? TXMAINCURSOR : txmaincursor[13:7];
assign gtf_ch_txmaincursor[2] = (FIX_GTF_ANALOG_CFG == 1)? TXMAINCURSOR : txmaincursor[20:14];
assign gtf_ch_txmaincursor[3] = (FIX_GTF_ANALOG_CFG == 1)? TXMAINCURSOR : txmaincursor[27:21];

assign gtf_ch_txpostcursor[0] = (FIX_GTF_ANALOG_CFG == 1)? TXPOSTCURSOR : txpostcursor[4:0];
assign gtf_ch_txpostcursor[1] = (FIX_GTF_ANALOG_CFG == 1)? TXPOSTCURSOR : txpostcursor[9:5];
assign gtf_ch_txpostcursor[2] = (FIX_GTF_ANALOG_CFG == 1)? TXPOSTCURSOR : txpostcursor[14:10];
assign gtf_ch_txpostcursor[3] = (FIX_GTF_ANALOG_CFG == 1)? TXPOSTCURSOR : txpostcursor[19:15];

assign gtf_ch_txprecursor[0] = (FIX_GTF_ANALOG_CFG == 1)? TXPRECURSOR : txprecursor[4:0];
assign gtf_ch_txprecursor[1] = (FIX_GTF_ANALOG_CFG == 1)? TXPRECURSOR : txprecursor[9:5];
assign gtf_ch_txprecursor[2] = (FIX_GTF_ANALOG_CFG == 1)? TXPRECURSOR : txprecursor[14:10];
assign gtf_ch_txprecursor[3] = (FIX_GTF_ANALOG_CFG == 1)? TXPRECURSOR : txprecursor[19:15];

wire [NUM_CHANNEL-1:0] gtwiz_reset_all_in;
wire [NUM_CHANNEL-1:0] hb_gtwiz_reset_all_in;
assign hb_gtwiz_reset_all_in = gtwiz_reset_all;

wire [NUM_CHANNEL-1:0] hb_gtf_ch_txdp_reset_in;
assign hb_gtf_ch_txdp_reset_in = gtwiz_reset_tx_datapath;
wire [NUM_CHANNEL-1:0] hb_gtf_ch_rxdp_reset_in;
assign hb_gtf_ch_rxdp_reset_in = gtwiz_reset_rx_datapath;

  wire              gtf_cm_qpll0lock;
  assign gtf_qpll0_lock = gtf_cm_qpll0lock;

    // divide clock by 4: UG974. Specifies the value to divide the clock. Divide value is value provided plus 1.
    // For instance, setting 3'b000 will provide a divide value of 1 and 3'b111 will be a divide value of 8.
    BUFG_GT u_rxrecovclk_inst (
      .CE      (1'b1),
      .CEMASK  (1'b0),
      .CLR     (1'b0),
      .CLRMASK (1'b0),
      .DIV     (3'd3),
      .I       (gtf_ch_rxoutclk[0]),
      .O       (gt_recov_clk)
    );

wire [3: 0]             gtf_ch_rxrecclkout;

  OBUFDS_GTE4_ADV #(
        .REFCLK_EN_TX_PATH    (1'b1),
        .REFCLK_ICNTL_TX      (5'b00111)    // CR-1136599
    ) u_recclk_obufds_gte4 (
        .RXRECCLK_SEL     (gtf_cm_rxrecclk0sel),
        .CEB              (1'b0),
        .I                (gtf_ch_rxrecclkout),
        .O                (gt_recov_clk_p),
        .OB               (gt_recov_clk_n)
    );


wire [NUM_CHANNEL-1:0] gtf_wiz_reset_tx_datapath_init_i;
wire [NUM_CHANNEL-1:0] gtf_wiz_reset_rx_datapath_init_i;
wire [NUM_CHANNEL-1:0] gtf_wiz_reset_tx_datapath_init_sync;
wire [NUM_CHANNEL-1:0] gtf_wiz_reset_rx_datapath_init_sync;
wire [NUM_CHANNEL-1:0] gtwiz_reset_tx_done_out_init_sync;
wire [NUM_CHANNEL-1:0] gtwiz_reset_rx_done_out_init_sync;
wire [NUM_CHANNEL-1:0] gtwiz_buffbypass_tx_done_out_i;
wire [NUM_CHANNEL-1:0] gtwiz_buffbypass_rx_done_out_i;
reg  [NUM_CHANNEL-1:0] tx_init_done_in_r;
reg  [NUM_CHANNEL-1:0] rx_init_done_in_r;

wire [NUM_CHANNEL-1:0] gtwiz_reset_tx_sync;
wire [NUM_CHANNEL-1:0] gtwiz_reset_rx_sync;

  wire  [NUM_CHANNEL-1:0] gtwiz_reset_rx_done_out;
  wire  [NUM_CHANNEL-1:0] gtwiz_reset_tx_done_out;
  wire [NUM_CHANNEL-1:0] gtf_ch_gtpowergood;

  assign gt_power_good = gtf_ch_gtpowergood;
  assign gtwiz_reset_rx_done = gtwiz_reset_rx_done_out;
  assign gtwiz_reset_tx_done = gtwiz_reset_tx_done_out;
  assign gtwiz_bufferbypass_tx_done = gtwiz_buffbypass_tx_done_out_i;
  assign gtwiz_bufferbypass_rx_done = gtwiz_buffbypass_rx_done_out_i;

  wire   gtwiz_reset_clk_freerun_in = freerun_clk;
  wire   gtf_cm_qpll0lockdetclk     = freerun_clk;
  wire   gtf_cm_qpll1lockdetclk     = freerun_clk;
  wire   gtf_ch_cplllockdetclk      = freerun_clk;
  wire   gtf_ch_drpclk              = freerun_clk;
  wire   gtf_cm_drpclk              = freerun_clk;
  wire   gtf_ch_dmonitorclk         = freerun_clk;

  //---Reset controller ports ---{
  wire  [NUM_CHANNEL-1:0] gtwiz_reset_tx_pll_and_datapath_in;
  wire  [NUM_CHANNEL-1:0] gtwiz_reset_tx_datapath_in;
  wire  [NUM_CHANNEL-1:0] gtwiz_reset_rx_pll_and_datapath_in;
  wire  [NUM_CHANNEL-1:0] gtwiz_reset_rx_datapath_in;
  wire  [NUM_CHANNEL-1:0] gtwiz_reset_rx_cdr_stable_out;
  wire  [NUM_CHANNEL-1:0] gtwiz_pllreset_rx_out;
  wire  [NUM_CHANNEL-1:0] gtwiz_pllreset_tx_out;
  wire  [NUM_CHANNEL-1:0] gtf_ch_txdp_reset_in;
  wire  [NUM_CHANNEL-1:0] gtf_ch_rxdp_reset_in;
  //---Reset controller ports ---}

  //---GT Reference clock inputs ---{
  wire              gtf_ch_gtgrefclk = 1'b0;
  wire  [2:0]       gtf_ch_cpllrefclksel = CPLLREFCLKSEL;
  wire      [2:0]   gtf_cm_qpll0refclksel = QPLL0REFCLKSEL;
  wire      [2:0]   gtf_cm_qpll1refclksel = QPLL1REFCLKSEL;
  wire              gtf_ch_gtnorthrefclk0 = 0;
  wire              gtf_ch_gtnorthrefclk1 = 0;
  wire              gtf_ch_gtrefclk0 = refclk_in;
  wire              gtf_ch_gtrefclk1 = 0;
  wire              gtf_ch_gtsouthrefclk0 = 0;
  wire              gtf_ch_gtsouthrefclk1 = 0;
  wire              gtf_cm_gtgrefclk0 = 0;
  wire              gtf_cm_gtgrefclk1 = 0;
  wire              gtf_cm_gtnorthrefclk00 = 0;
  wire              gtf_cm_gtnorthrefclk01 = 0;
  wire              gtf_cm_gtnorthrefclk10 = 0;
  wire              gtf_cm_gtnorthrefclk11 = 0;
  wire              gtf_cm_gtrefclk00 = refclk_in;
  wire              gtf_cm_gtrefclk01 = 0;
  wire              gtf_cm_gtrefclk10 = 0;
  wire              gtf_cm_gtrefclk11 = 0;
  wire              gtf_cm_gtsouthrefclk00 = 0;
  wire              gtf_cm_gtsouthrefclk01 = 0;
  wire              gtf_cm_gtsouthrefclk10 = 0;
  wire              gtf_cm_gtsouthrefclk11 = 0;
  //---GT Reference clock inputs ---}

  //--- CPLL/QPLL0/QPLL1 dynamic sel ---{
  wire              gtf_cm_qpll0clkrsvd0     =  QPLL0CLKRSVD0 ;
  wire              gtf_cm_qpll0clkrsvd1     =  QPLL0CLKRSVD1 ;
  wire              gtf_cm_qpll0fbclklost;
//  wire              gtf_cm_qpll0lock;
  wire              gtf_cm_qpll0locken = 1'b1;
  wire              gtf_cm_qpll0outclk;
  wire              gtf_cm_qpll0outrefclk;
  wire              gtf_cm_qpll0pd           =  QPLL0PD       ;
  wire              gtf_cm_qpll0refclklost;
  wire              gtf_cm_qpll0reset        = gtwiz_pllreset_tx_out[0];// QPLL0RESET    ;
  wire              gtf_cm_qpll1clkrsvd0     =  QPLL1CLKRSVD0 ;
  wire              gtf_cm_qpll1clkrsvd1     =  QPLL1CLKRSVD1 ;
  wire              gtf_cm_qpll1fbclklost;
  wire              gtf_cm_qpll1lock;
  wire              gtf_cm_qpll1locken = 1'b1;
  wire              gtf_cm_qpll1outclk;
  wire              gtf_cm_qpll1outrefclk;
  wire              gtf_cm_qpll1pd           =  QPLL1PD       ;
  wire              gtf_cm_qpll1refclklost;
  wire              gtf_cm_qpll1reset        =  QPLL1RESET    ;
  wire      [4:0]   gtf_cm_qpllrsvd2         =  QPLLRSVD2     ;
  wire      [4:0]   gtf_cm_qpllrsvd3         =  QPLLRSVD3     ;
  wire      [7:0]   gtf_cm_qpll0fbdiv        =  QPLL0FBDIV    ;
  wire      [7:0]   gtf_cm_qpll1fbdiv        =  QPLL1FBDIV    ;
  wire      [7:0]   gtf_cm_qplldmonitor0;
  wire      [7:0]   gtf_cm_qplldmonitor1;
  wire      [7:0]   gtf_cm_qpllrsvd1         =  QPLLRSVD1     ;
  wire      [7:0]   gtf_cm_qpllrsvd4         =  QPLLRSVD4     ;
  wire              gtf_ch_cpllfbclklost;
  wire              gtf_ch_cpllfreqlock = CPLLFREQLOCK;
  wire              gtf_ch_cplllock;
  wire              gtf_ch_cplllocken =1'b1;
  wire              gtf_ch_cpllpd = 1'b0;
  wire              gtf_ch_cpllrefclklost;
  wire              gtf_ch_cpllreset = 1'b0;
  wire              gtf_ch_qpll0clk = gtf_cm_qpll0outclk;
  wire              gtf_ch_qpll0freqlock = QPLL0FREQLOCK;
  wire              gtf_ch_qpll0refclk = gtf_cm_qpll0outrefclk;
  wire              gtf_ch_qpll1clk = gtf_cm_qpll1outclk;
  wire              gtf_ch_qpll1freqlock = QPLL1FREQLOCK;
  wire              gtf_ch_qpll1refclk = gtf_cm_qpll1outrefclk;
  wire              plllock_tx_in = gtf_cm_qpll0lock;
  wire              plllock_rx_in = gtf_cm_qpll0lock;

  wire              gtf_cm_sdm0reset     =  SDM0RESET  ;
  wire              gtf_cm_sdm0toggle    =  SDM0TOGGLE ;
  wire      [1:0]   gtf_cm_sdm0width     =  SDM0WIDTH  ;
  wire      [3:0]   gtf_cm_sdm0finalout;
  wire     [14:0]   gtf_cm_sdm0testdata;
  wire     [24:0]   gtf_cm_sdm0data      =  SDM0DATA   ;
  wire              gtf_cm_sdm1reset     =  SDM1RESET  ;
  wire              gtf_cm_sdm1toggle    =  SDM1TOGGLE ;
  wire      [1:0]   gtf_cm_sdm1width     =  SDM1WIDTH  ;
  wire      [3:0]   gtf_cm_sdm1finalout;
  wire     [14:0]   gtf_cm_sdm1testdata;
  wire     [24:0]   gtf_cm_sdm1data      =  SDM1DATA   ;
  //--- CPLL/QPLL0/QPLL1 dynamic sel ---}

  //--- DRP ports ---{
  wire  [NUM_CHANNEL-1:0]       gtf_ch_drpen;
  wire  [NUM_CHANNEL-1:0]       gtf_ch_drprst;
  wire  [NUM_CHANNEL-1:0]       gtf_ch_drpwe;
  wire  [16*NUM_CHANNEL-1:0]    gtf_ch_drpdi;
  wire  [10*NUM_CHANNEL-1:0]    gtf_ch_drpaddr;
  wire  [NUM_CHANNEL-1:0]       gtf_ch_drprdy;
  wire  [16*NUM_CHANNEL-1:0]    gtf_ch_drpdo;
  wire         gtf_cm_drpen = 0;
  wire         gtf_cm_drpwe =0;
  wire  [15:0] gtf_cm_drpaddr =0;
  wire  [15:0] gtf_cm_drpdi =0;
  wire         gtf_cm_drprdy;
  wire  [15:0] gtf_cm_drpdo;
  //--- DRP ports ---}

  wire  [NUM_CHANNEL-1:0] gtwiz_buffbypass_rx_reset;
  wire  [NUM_CHANNEL-1:0] gtf_ch_am_switch;
  wire  [NUM_CHANNEL-1:0] gtf_ch_drp_reconfig_rdy;
  wire  [NUM_CHANNEL-1:0] gtf_ch_drp_reconfig_done;

  //---Port tie offs--{
  wire  [8:0]       gtf_ch_ctlrxpauseack = CTLRXPAUSEACK;
  wire  [8:0]       gtf_ch_ctltxpausereq = CTLTXPAUSEREQ;
  wire              gtf_ch_ctltxresendpause = CTLTXRESENDPAUSE;
  wire              gtf_ch_ctltxsendidle = CTLTXSENDIDLE;
  wire              gtf_ch_ctltxsendlfi = CTLTXSENDLFI;
  wire              gtf_ch_ctltxsendrfi = CTLTXSENDRFI;
  wire              gtf_ch_cdrstepdir = CDRSTEPDIR;
  wire              gtf_ch_cdrstepsq = CDRSTEPSQ;
  wire              gtf_ch_cdrstepsx = CDRSTEPSX;
  wire              gtf_ch_cfgreset = CFGRESET;
  wire              gtf_ch_clkrsvd0 = CLKRSVD0;
  wire              gtf_ch_clkrsvd1 = CLKRSVD1;
  wire              gtf_ch_dmonfiforeset = DMONFIFORESET;
  wire              gtf_ch_eyescanreset = EYESCANRESET;
  wire              gtf_ch_eyescantrigger = EYESCANTRIGGER;
  wire              gtf_ch_freqos = FREQOS;
  wire              gtf_ch_gtrxreset = GTRXRESET;
  wire              gtf_ch_gtrxresetsel = GTRXRESETSEL;
  wire              gtf_ch_gttxreset = GTTXRESET;
  wire              gtf_ch_gttxresetsel = GTTXRESETSEL;
  wire              gtf_ch_incpctrl = INCPCTRL;
  wire              gtf_ch_resetovrd = RESETOVRD;
  wire              gtf_ch_rxafecfoken = RXAFECFOKEN;
  wire              gtf_ch_rxcdrfreqreset = RXCDRFREQRESET;
  wire              gtf_ch_rxcdrhold = RXCDRHOLD;
  wire              gtf_ch_rxcdrovrden = RXCDROVRDEN;
  wire              gtf_ch_rxcdrreset = RXCDRRESET;
  wire              gtf_ch_rxckcalreset = RXCKCALRESET;
  wire              gtf_ch_rxdfeagchold = RXDFEAGCHOLD;
  wire              gtf_ch_rxdfeagcovrden = RXDFEAGCOVRDEN;
  wire              gtf_ch_rxdfecfokfen = RXDFECFOKFEN;
  wire              gtf_ch_rxdfecfokfpulse = RXDFECFOKFPULSE;
  wire              gtf_ch_rxdfecfokhold = RXDFECFOKHOLD;
  wire              gtf_ch_rxdfecfokovren = RXDFECFOKOVREN;
  wire              gtf_ch_rxdfekhhold = RXDFEKHHOLD;
  wire              gtf_ch_rxdfekhovrden = RXDFEKHOVRDEN;
  wire              gtf_ch_rxdfelfhold = RXDFELFHOLD;
  wire              gtf_ch_rxdfelfovrden = RXDFELFOVRDEN;
  wire              gtf_ch_rxdfelpmreset      = RXDFELPMRESET      ;
  wire              gtf_ch_rxdfetap10hold     = RXDFETAP10HOLD     ;
  wire              gtf_ch_rxdfetap10ovrden   = RXDFETAP10OVRDEN   ;
  wire              gtf_ch_rxdfetap11hold     = RXDFETAP11HOLD     ;
  wire              gtf_ch_rxdfetap11ovrden   = RXDFETAP11OVRDEN   ;
  wire              gtf_ch_rxdfetap12hold     = RXDFETAP12HOLD     ;
  wire              gtf_ch_rxdfetap12ovrden   = RXDFETAP12OVRDEN   ;
  wire              gtf_ch_rxdfetap13hold     = RXDFETAP13HOLD     ;
  wire              gtf_ch_rxdfetap13ovrden   = RXDFETAP13OVRDEN   ;
  wire              gtf_ch_rxdfetap14hold     = RXDFETAP14HOLD     ;
  wire              gtf_ch_rxdfetap14ovrden   = RXDFETAP14OVRDEN   ;
  wire              gtf_ch_rxdfetap15hold     = RXDFETAP15HOLD     ;
  wire              gtf_ch_rxdfetap15ovrden   = RXDFETAP15OVRDEN   ;
  wire              gtf_ch_rxdfetap2hold      = RXDFETAP2HOLD      ;
  wire              gtf_ch_rxdfetap2ovrden    = RXDFETAP2OVRDEN    ;
  wire              gtf_ch_rxdfetap3hold      = RXDFETAP3HOLD      ;
  wire              gtf_ch_rxdfetap3ovrden    = RXDFETAP3OVRDEN    ;
  wire              gtf_ch_rxdfetap4hold      = RXDFETAP4HOLD      ;
  wire              gtf_ch_rxdfetap4ovrden    = RXDFETAP4OVRDEN    ;
  wire              gtf_ch_rxdfetap5hold      = RXDFETAP5HOLD      ;
  wire              gtf_ch_rxdfetap5ovrden    = RXDFETAP5OVRDEN    ;
  wire              gtf_ch_rxdfetap6hold      = RXDFETAP6HOLD      ;
  wire              gtf_ch_rxdfetap6ovrden    = RXDFETAP6OVRDEN    ;
  wire              gtf_ch_rxdfetap7hold      = RXDFETAP7HOLD      ;
  wire              gtf_ch_rxdfetap7ovrden    = RXDFETAP7OVRDEN    ;
  wire              gtf_ch_rxdfetap8hold      = RXDFETAP8HOLD      ;
  wire              gtf_ch_rxdfetap8ovrden    = RXDFETAP8OVRDEN    ;
  wire              gtf_ch_rxdfetap9hold      = RXDFETAP9HOLD      ;
  wire              gtf_ch_rxdfetap9ovrden    = RXDFETAP9OVRDEN    ;
  wire              gtf_ch_rxdfeuthold        = RXDFEUTHOLD        ;
  wire              gtf_ch_rxdfeutovrden      = RXDFEUTOVRDEN      ;
  wire              gtf_ch_rxdfevphold        = RXDFEVPHOLD        ;
  wire              gtf_ch_rxdfevpovrden      = RXDFEVPOVRDEN      ;
  wire              gtf_ch_rxdfexyden         = RXDFEXYDEN         ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxdlybypass;//     = RXDLYBYPASS        ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxdlyen;//         = RXDLYEN            ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxdlyovrden;//     = RXDLYOVRDEN        ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxdlysreset;//     = RXDLYSRESET        ;
//  wire              gtf_ch_rxlpmen            = RXLPMEN            ;
  wire              gtf_ch_rxlpmgchold        = RXLPMGCHOLD        ;
  wire              gtf_ch_rxlpmgcovrden      = RXLPMGCOVRDEN      ;
  wire              gtf_ch_rxlpmhfhold        = RXLPMHFHOLD        ;
  wire              gtf_ch_rxlpmhfovrden      = RXLPMHFOVRDEN      ;
  wire              gtf_ch_rxlpmlfhold        = RXLPMLFHOLD        ;
  wire              gtf_ch_rxlpmlfklovrden    = RXLPMLFKLOVRDEN    ;
  wire              gtf_ch_rxlpmoshold        = RXLPMOSHOLD        ;
  wire              gtf_ch_rxlpmosovrden      = RXLPMOSOVRDEN      ;
  wire              gtf_ch_rxoscalreset       = RXOSCALRESET    ;
  wire              gtf_ch_rxoshold           = RXOSHOLD        ;
  wire              gtf_ch_rxosovrden         = RXOSOVRDEN      ;
  wire              gtf_ch_rxpcsreset         = RXPCSRESET      ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxphalign;//       = RXPHALIGN       ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxphalignen;//     = RXPHALIGNEN     ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxphdlypd;//       = RXPHDLYPD       ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxphdlyreset;//    = RXPHDLYRESET    ;
  wire              gtf_ch_rxpmareset         = RXPMARESET      ;
//  wire              gtf_ch_rxpolarity         = RXPOLARITY      ;
  wire              gtf_ch_rxprbscntreset     = RXPRBSCNTRESET  ;
  wire              gtf_ch_rxprogdivreset     = RXPROGDIVRESET  ;
  wire              gtf_ch_rxslipoutclk       = RXSLIPOUTCLK    ;
  wire              gtf_ch_rxslippma          = RXSLIPPMA       ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxsyncallin;//     = RXSYNCALLIN     ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxsyncin;//        = RXSYNCIN        ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxsyncmode;//      = RXSYNCMODE      ;
  wire              gtf_ch_rxtermination      = RXTERMINATION   ;
  wire              gtf_ch_rxuserrdy         = RXUSERRDY;
  wire              gtf_ch_txdccforcestart   =  TXDCCFORCESTART;
  wire              gtf_ch_txdccreset        =  TXDCCRESET     ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlybypass;//    =  TXDLYBYPASS    ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlyen;//        =  TXDLYEN        ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlyhold;//      =  TXDLYHOLD      ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlyovrden;//    =  TXDLYOVRDEN    ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlysreset;//    =  TXDLYSRESET    ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlyupdown;//    =  TXDLYUPDOWN    ;
  wire              gtf_ch_txelecidle        =  1'b0;
  wire              gtf_ch_txgbseqsync       =  TXGBSEQSYNC    ;
  wire              gtf_ch_txmuxdcdexhold    =  TXMUXDCDEXHOLD ;
  wire              gtf_ch_txmuxdcdorwren    =  TXMUXDCDORWREN ;
  wire              gtf_ch_txpcsreset        =  TXPCSRESET     ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphalign;//      =  TXPHALIGN      ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphalignen;//    =  TXPHALIGNEN    ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphdlypd;//      =  TXPHDLYPD      ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphdlyreset;//   =  TXPHDLYRESET   ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphdlytstclk;//  =  TXPHDLYTSTCLK  ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphinit;//       =  TXPHINIT       ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphovrden;//     =  TXPHOVRDEN     ;
  wire              gtf_ch_txpippmen         =  TXPIPPMEN      ;
  wire              gtf_ch_txpippmovrden     =  TXPIPPMOVRDEN  ;
  wire              gtf_ch_txpippmpd         =  TXPIPPMPD      ;
  wire              gtf_ch_txpippmsel        =  TXPIPPMSEL     ;
  wire              gtf_ch_txpisopd          =  TXPISOPD       ;
  wire              gtf_ch_txpmareset        =  TXPMARESET     ;
//  wire              gtf_ch_txpolarity        =  TXPOLARITY     ;
  wire              gtf_ch_txprbsforceerr    =  TXPRBSFORCEERR ;
  wire              gtf_ch_txprogdivreset    =  TXPROGDIVRESET ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txsyncallin;//    =  TXSYNCALLIN    ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txsyncin;//       =  TXSYNCIN       ;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txsyncmode;//     =  TXSYNCMODE     ;
  wire              gtf_ch_txuserrdy         =  TXUSERRDY      ;
  wire  [19:0]      gtf_ch_tstin = TSTIN;
  wire  [1:0]       gtf_ch_rxelecidlemode     = RXELECIDLEMODE ;
  wire  [1:0]       gtf_ch_rxmonitorsel = RXMONITORSEL;
  wire  [1:0]       gtf_ch_rxpd = RXPD;
  wire  [1:0]       gtf_ch_rxpllclksel = RXPLLCLKSEL;
  wire  [1:0]       gtf_ch_rxsysclksel       =  RXSYSCLKSEL;
  wire  [1:0]       gtf_ch_txpd = TXPD;
  wire  [1:0]       gtf_ch_txpllclksel = TXPLLCLKSEL;
  wire  [1:0]       gtf_ch_txsysclksel       =  TXSYSCLKSEL    ;
//  wire  [2:0]       gtf_ch_loopback = LOOPBACK;
  wire  [2:0]       gtf_ch_rxoutclksel = RXOUTCLKSEL;
  wire  [2:0]       gtf_ch_txoutclksel = TXOUTCLKSEL;
  wire  [3:0]       gtf_ch_rxdfecfokfcnum = RXDFECFOKFCNUM;
  wire  [3:0]       gtf_ch_rxprbssel = RXPRBSSEL;
  wire  [3:0]       gtf_ch_txprbssel = TXPRBSSEL;
//  wire  [4:0]       gtf_ch_txdiffctrl        =  A_TXDIFFCTRL     ;
  wire  [4:0]       gtf_ch_txpippmstepsize   =  TXPIPPMSTEPSIZE;
//  wire  [4:0]       gtf_ch_txpostcursor      =  TXPOSTCURSOR   ;
//  wire  [4:0]       gtf_ch_txprecursor       =  TXPRECURSOR    ;
  wire  [6:0]       gtf_ch_rxckcalstart = RXCKCALSTART;
//  wire  [6:0]       gtf_ch_txmaincursor      =  TXMAINCURSOR   ;
  wire              gtf_ch_dmonitoroutclk;
  wire              gtf_ch_eyescandataerror;
//  wire [NUM_CHANNEL-1:0]             gtf_ch_gtpowergood;
  wire              gtf_ch_gtrefclkmonitor;
  wire              gtf_ch_resetexception;
  wire              gtf_ch_rxbitslip;
  wire              gtf_ch_rxslippmardy;
  wire              gtf_ch_statrxblocklock;
  wire              gtf_ch_statrxinternallocalfault;
  wire              gtf_ch_statrxlocalfault;
  wire              gtf_ch_statrxreceivedlocalfault;
  wire              gtf_ch_statrxremotefault;
  wire  [15:0]      gtf_ch_gtrsvd = GTRSVD;
  wire  [15:0]      gtf_ch_pcsrsvdin = PCSRSVDIN;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxcdrlock;
  wire              gtf_ch_rxcdrphdone;
  wire              gtf_ch_rxckcaldone;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxdlysresetdone;
  wire              gtf_ch_rxelecidle;
  wire              gtf_ch_rxosintdone;
  wire              gtf_ch_rxosintstarted;
  wire              gtf_ch_rxosintstrobedone;
  wire              gtf_ch_rxosintstrobestarted;
//  wire              gtf_ch_rxoutclk;
  wire              gtf_ch_rxoutclkfabric;
  wire              gtf_ch_rxoutclkpcs;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxphovrden;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxphaligndone;
  wire              gtf_ch_rxphalignerr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxpmaresetdone;
  wire              gtf_ch_rxprbserr;
  wire              gtf_ch_rxprbslocked;
  wire              gtf_ch_rxprgdivresetdone;
//  wire              gtf_ch_rxrecclkout;
//  wire [NUM_CHANNEL-1:0]             gtf_ch_rxresetdone;
  wire              gtf_ch_rxslipdone;
  wire              gtf_ch_rxslipoutclkrdy;
//  wire [NUM_CHANNEL-1:0]             gtf_ch_rxsyncdone;
  wire [NUM_CHANNEL-1:0]             gtf_ch_rxsyncout;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxbadcode;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxbadfcs;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxbadpreamble;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxbadsfd;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxbroadcast;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxfcserr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxframingerr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxgotsignalos;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxhiber;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxinrangeerr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxmulticast;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxpkt;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxpkterr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxstatus;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxstompedfcs;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxtestpatternmismatch;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxtruncated;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxunicast;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxvalidctrlcode;
  wire [NUM_CHANNEL-1:0]             gtf_ch_statrxvlan;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxbadfcs;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxbroadcast;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxfcserr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxmulticast;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxpkt;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxpkterr;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxunicast;
  wire [NUM_CHANNEL-1:0]             gtf_ch_stattxvlan;
  wire              gtf_ch_txdccdone;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txdlysresetdone;
  wire              gtf_ch_txoutclk;
  wire              gtf_ch_txoutclkfabric;
  wire              gtf_ch_txoutclkpcs;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphaligndone;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txphinitdone;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txpmaresetdone;
  wire              gtf_ch_txprgdivresetdone;
//  wire [NUM_CHANNEL-1:0]             gtf_ch_txresetdone;
//  wire [NUM_CHANNEL-1:0]             gtf_ch_txsyncdone;
  wire [NUM_CHANNEL-1:0]             gtf_ch_txsyncout;
  wire [16*NUM_CHANNEL-1:0]          gtf_ch_dmonitorout;
  wire   [15:0]     gtf_ch_pinrsrvdas;
  wire   [7:0]      gtf_ch_rxmonitorout;
  wire [9*NUM_CHANNEL-1:0]           gtf_ch_statrxpausequanta;
  wire [9*NUM_CHANNEL-1:0]           gtf_ch_statrxpausereq;
  wire [9*NUM_CHANNEL-1:0]           gtf_ch_statrxpausevalid;
  wire [9*NUM_CHANNEL-1:0]           gtf_ch_stattxpausevalid;
  wire              gtf_cm_bgbypassb         =  BGBYPASSB    ;
  wire              gtf_cm_bgmonitorenb      =  BGMONITORENB ;
  wire              gtf_cm_bgpdb             =  BGPDB        ;
  wire              gtf_cm_bgrcalovrdenb     =  BGRCALOVRDENB;
  wire              gtf_cm_rcalenb           =  RCALENB      ;
  wire      [4:0]   gtf_cm_bgrcalovrd        =  BGRCALOVRD;
  wire      [7:0]   gtf_cm_pmarsvd0          =  PMARSVD0  ;
  wire      [7:0]   gtf_cm_pmarsvd1          =  PMARSVD1  ;
  wire              gtf_cm_refclkoutmonitor0;
  wire              gtf_cm_refclkoutmonitor1;
//  wire      [1:0]   gtf_cm_rxrecclk0sel;
  wire      [1:0]   gtf_cm_rxrecclk1sel;
  wire      [7:0]   gtf_cm_pmarsvdout0;
  wire      [7:0]   gtf_cm_pmarsvdout1;
  //---Port tie offs--}

  wire  [40*NUM_CHANNEL-1:0]         gtf_ch_txrawdata;

  wire  [NUM_CHANNEL-1:0]            gtf_ch_txaxisterr = {NUM_CHANNEL{1'b0}};
  wire  [NUM_CHANNEL-1:0]            gtf_ch_txaxistpoison = {NUM_CHANNEL{1'b0}};
  wire  [NUM_CHANNEL-1:0]            gtf_ch_txaxistready;
  wire  [NUM_CHANNEL-1:0]            gtf_ch_txaxistvalid = {NUM_CHANNEL{1'b1}};
  wire  [2*NUM_CHANNEL-1:0]          gtf_ch_txaxistsof = {NUM_CHANNEL{2'd0}};
  wire  [5*NUM_CHANNEL-1:0]          gtf_ch_txaxistterm = {NUM_CHANNEL{5'd0}};
  wire  [64*NUM_CHANNEL-1:0]         gtf_ch_txaxistdata = {NUM_CHANNEL{64'd0}};
  wire  [8*NUM_CHANNEL-1:0]          gtf_ch_txaxistlast = {NUM_CHANNEL{8'd1}};
  wire  [8*NUM_CHANNEL-1:0]          gtf_ch_txaxistpre  = {NUM_CHANNEL{8'd0}};

  wire   [40*NUM_CHANNEL-1:0]     gtf_ch_rxrawdata;

  wire              gtf_ch_rxaxisterr;
  wire   [NUM_CHANNEL-1:0]        gtf_ch_rxaxistvalid;
  wire   [1:0]      gtf_ch_rxaxistsof;
  wire   [4:0]      gtf_ch_rxaxistterm;
  wire   [64*NUM_CHANNEL-1:0]     gtf_ch_rxaxistdata;
  wire   [8*NUM_CHANNEL-1:0]      gtf_ch_rxaxistlast;
  wire   [7:0]      gtf_ch_rxaxistpre;
  wire  [4*NUM_CHANNEL-1:0]          gtf_ch_statrxbytes;
  wire   [NUM_CHANNEL-1:0]        gtf_ch_gttxreset_out;

  wire [NUM_CHANNEL-1:0]    status_int_freerun_sync = 'hF;
  wire [16*NUM_CHANNEL-1:0] tx_prbs_data;
  wire [16*NUM_CHANNEL-1:0] rx_prbs_data;
  wire [16*NUM_CHANNEL-1:0] prbs_any_chk_error_int;
  wire [NUM_CHANNEL-1:0]    status_int_prbs;


genvar i;
generate
for (i=0; i<NUM_CHANNEL; i=i+1) begin : gen_blk_multi_ch

  xpm_cdc_async_rst # (
    .DEST_SYNC_FF (5),
    .RST_ACTIVE_HIGH(1)
  ) reset_all_in_sync (
    .dest_clk  (freerun_clk),
    .src_arst  (hb_gtwiz_reset_all_in[i]),
    .dest_arst (gtwiz_reset_all_in[i])
  );

  xpm_cdc_async_rst # (
    .DEST_SYNC_FF (DEST_SYNC_FF),
    .RST_ACTIVE_HIGH(1)
  ) reset_tx_in_sync (
    .dest_clk  (freerun_clk),
    .src_arst  (gtwiz_reset_tx_pll_and_datapath[i]),
    .dest_arst (gtwiz_reset_tx_pll_and_datapath_in[i])
  );

  xpm_cdc_async_rst # (
    .DEST_SYNC_FF (DEST_SYNC_FF),
    .RST_ACTIVE_HIGH(1)
  ) reset_rx_in_sync (
    .dest_clk  (freerun_clk),
    .src_arst  (gtwiz_reset_rx_pll_and_datapath[i]),
    .dest_arst (gtwiz_reset_rx_pll_and_datapath_in[i])
  );

  xpm_cdc_async_rst # (
    .DEST_SYNC_FF (5),
    .RST_ACTIVE_HIGH(1)
  ) reset_tx_dp_in_sync (
    .dest_clk  (freerun_clk),
    .src_arst  (hb_gtf_ch_txdp_reset_in[i]),
    .dest_arst (gtf_ch_txdp_reset_in[i])
  );

  xpm_cdc_async_rst # (
    .DEST_SYNC_FF (5),
    .RST_ACTIVE_HIGH(1)
  ) reset_rx_dp_in_sync (
    .dest_clk  (freerun_clk),
    .src_arst  (hb_gtf_ch_rxdp_reset_in[i]),
    .dest_arst (gtf_ch_rxdp_reset_in[i])
  );

  assign gtf_ch_txrawdata[40*(i+1)-1:40*i] = tx_raw_data[i];
  assign rx_raw_data[i] = gtf_ch_rxrawdata[40*(i+1)-1:40*i];

  assign gtf_ch_drprst[i]             =  gtwiz_reset_all_in[i];
  assign gtwiz_buffbypass_rx_reset[i] = ~gtf_ch_rxresetdone[i];

  xpm_cdc_single #(
   .DEST_SYNC_FF(DEST_SYNC_FF),   // DECIMAL; range: 2-10
   .INIT_SYNC_FF(0),   // DECIMAL; 0=disable simulation init values, 1=enable simulation init values
   .SIM_ASSERT_CHK(0), // DECIMAL; 0=disable simulation messages, 1=enable simulation messages
   .SRC_INPUT_REG(0)   // DECIMAL; 0=do not register input, 1=register input
   ) u_reset_tx_done_out_init_done_inst (
     .dest_out (gtwiz_reset_tx_done_out_init_sync[i]),
     .dest_clk (gtf_ch_drpclk),
     .src_clk  (1'b0),
     .src_in   (gtwiz_reset_tx_done_out[i])
   );

  always @ (posedge gtf_ch_drpclk)
  begin
    tx_init_done_in_r[i]  <=  gtwiz_reset_tx_done_out_init_sync[i] && gtwiz_buffbypass_tx_done_out_i[i];
  end

  xpm_cdc_single #(
   .DEST_SYNC_FF(3),   // DECIMAL; range: 2-10
   .INIT_SYNC_FF(0),   // DECIMAL; 0=disable simulation init values, 1=enable simulation init values
   .SIM_ASSERT_CHK(0), // DECIMAL; 0=disable simulation messages, 1=enable simulation messages
   .SRC_INPUT_REG(0)   // DECIMAL; 0=do not register input, 1=register input
   ) u_reset_rx_done_out_init_done_inst (
     .dest_out (gtwiz_reset_rx_done_out_init_sync[i]),
     .dest_clk (gtf_ch_drpclk),
     .src_clk  (1'b0),
     .src_in   (gtwiz_reset_rx_done_out[i])
   );

  always @ (posedge gtf_ch_drpclk)
  begin
    rx_init_done_in_r[i]  <=  gtwiz_reset_rx_done_out_init_sync[i] && gtwiz_buffbypass_rx_done_out_i[i];
  end


gtf_wiz_10_raw u_gtf_wiz_ip_top (
.gtwiz_reset_clk_freerun_in                (gtwiz_reset_clk_freerun_in                ),
.gtwiz_reset_all_in                        (gtwiz_reset_all_in[i]                     ),
.gtwiz_reset_tx_pll_and_datapath_in        (gtwiz_reset_tx_pll_and_datapath_in[i]     ),
.gtwiz_reset_rx_pll_and_datapath_in        (gtwiz_reset_rx_pll_and_datapath_in[i]     ),
.gtwiz_reset_tx_datapath_in                (gtf_ch_txdp_reset_in[i]                   ),
.gtwiz_reset_rx_datapath_in                (gtf_ch_rxdp_reset_in[i]                   ),
.gtwiz_reset_rx_cdr_stable_out             (gtwiz_reset_rx_cdr_stable_out[i]          ),
.gtwiz_reset_tx_done_out                   (gtwiz_reset_tx_done_out[i]                ),
.gtwiz_reset_rx_done_out                   (gtwiz_reset_rx_done_out[i]                ),
.gtwiz_pllreset_rx_out                     (gtwiz_pllreset_rx_out[i]                  ),
.gtwiz_pllreset_tx_out                     (gtwiz_pllreset_tx_out[i]                  ),
.plllock_tx_in                             (plllock_tx_in                             ),
.plllock_rx_in                             (plllock_rx_in                             ),
.gtf_ch_cdrstepdir                         (gtf_ch_cdrstepdir                         ),
.gtf_ch_cdrstepsq                          (gtf_ch_cdrstepsq                          ),
.gtf_ch_cdrstepsx                          (gtf_ch_cdrstepsx                          ),
.gtf_ch_cfgreset                           (gtf_ch_cfgreset                           ),
.gtf_ch_clkrsvd0                           (gtf_ch_clkrsvd0                           ),
.gtf_ch_clkrsvd1                           (gtf_ch_clkrsvd1                           ),
.gtf_ch_cpllfreqlock                       (gtf_ch_cpllfreqlock                       ),
.gtf_ch_cplllockdetclk                     (gtf_ch_cplllockdetclk                     ),
.gtf_ch_cplllocken                         (gtf_ch_cplllocken                         ),
.gtf_ch_cpllpd                             (gtf_ch_cpllpd                             ),
.gtf_ch_cpllreset                          (gtf_ch_cpllreset                          ),
.gtf_ch_ctltxresendpause                   (gtf_ch_ctltxresendpause                   ),
.gtf_ch_ctltxsendlfi                       (gtf_ch_ctltxsendlfi                       ),
.gtf_ch_ctltxsendrfi                       (gtf_ch_ctltxsendrfi                       ),
.gtf_ch_ctltxsendidle                      (gtf_ch_ctltxsendidle                      ),
.gtf_ch_dmonfiforeset                      (gtf_ch_dmonfiforeset                      ),
.gtf_ch_dmonitorclk                        (gtf_ch_dmonitorclk                        ),
.gtf_ch_drpclk                             (gtf_ch_drpclk                             ),
.gtf_ch_drprst                             (gtf_ch_drprst[i]                          ),
.gtf_ch_eyescanreset                       (gtf_ch_eyescanreset                       ),
.gtf_ch_eyescantrigger                     (gtf_ch_eyescantrigger                     ),
.gtf_ch_freqos                             (gtf_ch_freqos                             ),
.gtf_ch_gtfrxn                             (gtf_ch_gtfrxn[i]                          ),
.gtf_ch_gtfrxp                             (gtf_ch_gtfrxp[i]                          ),
.gtf_ch_gtgrefclk                          (gtf_ch_gtgrefclk                          ),
.gtf_ch_gtnorthrefclk0                     (gtf_ch_gtnorthrefclk0                     ),
.gtf_ch_gtnorthrefclk1                     (gtf_ch_gtnorthrefclk1                     ),
.gtf_ch_gtrefclk0                          (gtf_ch_gtrefclk0                          ),
.gtf_ch_gtrefclk1                          (gtf_ch_gtrefclk1                          ),
.gtf_ch_gtrxresetsel                       (gtf_ch_gtrxresetsel                       ),
.gtf_ch_gtsouthrefclk0                     (gtf_ch_gtsouthrefclk0                     ),
.gtf_ch_gtsouthrefclk1                     (gtf_ch_gtsouthrefclk1                     ),
.gtf_ch_gttxresetsel                       (gtf_ch_gttxresetsel                       ),
.gtf_ch_incpctrl                           (gtf_ch_incpctrl                           ),
.gtf_ch_qpll0clk                           (gtf_ch_qpll0clk                           ),
.gtf_ch_qpll0freqlock                      (gtf_ch_qpll0freqlock                      ),
.gtf_ch_qpll0refclk                        (gtf_ch_qpll0refclk                        ),
.gtf_ch_qpll1clk                           (gtf_ch_qpll1clk                           ),
.gtf_ch_qpll1freqlock                      (gtf_ch_qpll1freqlock                      ),
.gtf_ch_qpll1refclk                        (gtf_ch_qpll1refclk                        ),
.gtf_ch_resetovrd                          (gtf_ch_resetovrd                          ),
.gtf_ch_rxafecfoken                        (gtf_ch_rxafecfoken                        ),
.gtf_ch_rxcdrfreqreset                     (gtf_ch_rxcdrfreqreset                     ),
.gtf_ch_rxcdrhold                          (gtf_ch_rxcdrhold                          ),
.gtf_ch_rxcdrovrden                        (gtf_ch_rxcdrovrden                        ),
.gtf_ch_rxcdrreset                         (gtf_ch_rxcdrreset                         ),
.gtf_ch_rxckcalreset                       (gtf_ch_rxckcalreset                       ),
.gtf_ch_rxdfeagchold                       (gtf_ch_rxdfeagchold                       ),
.gtf_ch_rxdfeagcovrden                     (gtf_ch_rxdfeagcovrden                     ),
.gtf_ch_rxdfecfokfen                       (gtf_ch_rxdfecfokfen                       ),
.gtf_ch_rxdfecfokfpulse                    (gtf_ch_rxdfecfokfpulse                    ),
.gtf_ch_rxdfecfokhold                      (gtf_ch_rxdfecfokhold                      ),
.gtf_ch_rxdfecfokovren                     (gtf_ch_rxdfecfokovren                     ),
.gtf_ch_rxdfekhhold                        (gtf_ch_rxdfekhhold                        ),
.gtf_ch_rxdfekhovrden                      (gtf_ch_rxdfekhovrden                      ),
.gtf_ch_rxdfelfhold                        (gtf_ch_rxdfelfhold                        ),
.gtf_ch_rxdfelfovrden                      (gtf_ch_rxdfelfovrden                      ),
.gtf_ch_rxdfelpmreset                      (gtf_ch_rxdfelpmreset                      ),
.gtf_ch_rxdfetap10hold                     (gtf_ch_rxdfetap10hold                     ),
.gtf_ch_rxdfetap10ovrden                   (gtf_ch_rxdfetap10ovrden                   ),
.gtf_ch_rxdfetap11hold                     (gtf_ch_rxdfetap11hold                     ),
.gtf_ch_rxdfetap11ovrden                   (gtf_ch_rxdfetap11ovrden                   ),
.gtf_ch_rxdfetap12hold                     (gtf_ch_rxdfetap12hold                     ),
.gtf_ch_rxdfetap12ovrden                   (gtf_ch_rxdfetap12ovrden                   ),
.gtf_ch_rxdfetap13hold                     (gtf_ch_rxdfetap13hold                     ),
.gtf_ch_rxdfetap13ovrden                   (gtf_ch_rxdfetap13ovrden                   ),
.gtf_ch_rxdfetap14hold                     (gtf_ch_rxdfetap14hold                     ),
.gtf_ch_rxdfetap14ovrden                   (gtf_ch_rxdfetap14ovrden                   ),
.gtf_ch_rxdfetap15hold                     (gtf_ch_rxdfetap15hold                     ),
.gtf_ch_rxdfetap15ovrden                   (gtf_ch_rxdfetap15ovrden                   ),
.gtf_ch_rxdfetap2hold                      (gtf_ch_rxdfetap2hold                      ),
.gtf_ch_rxdfetap2ovrden                    (gtf_ch_rxdfetap2ovrden                    ),
.gtf_ch_rxdfetap3hold                      (gtf_ch_rxdfetap3hold                      ),
.gtf_ch_rxdfetap3ovrden                    (gtf_ch_rxdfetap3ovrden                    ),
.gtf_ch_rxdfetap4hold                      (gtf_ch_rxdfetap4hold                      ),
.gtf_ch_rxdfetap4ovrden                    (gtf_ch_rxdfetap4ovrden                    ),
.gtf_ch_rxdfetap5hold                      (gtf_ch_rxdfetap5hold                      ),
.gtf_ch_rxdfetap5ovrden                    (gtf_ch_rxdfetap5ovrden                    ),
.gtf_ch_rxdfetap6hold                      (gtf_ch_rxdfetap6hold                      ),
.gtf_ch_rxdfetap6ovrden                    (gtf_ch_rxdfetap6ovrden                    ),
.gtf_ch_rxdfetap7hold                      (gtf_ch_rxdfetap7hold                      ),
.gtf_ch_rxdfetap7ovrden                    (gtf_ch_rxdfetap7ovrden                    ),
.gtf_ch_rxdfetap8hold                      (gtf_ch_rxdfetap8hold                      ),
.gtf_ch_rxdfetap8ovrden                    (gtf_ch_rxdfetap8ovrden                    ),
.gtf_ch_rxdfetap9hold                      (gtf_ch_rxdfetap9hold                      ),
.gtf_ch_rxdfetap9ovrden                    (gtf_ch_rxdfetap9ovrden                    ),
.gtf_ch_rxdfeuthold                        (gtf_ch_rxdfeuthold                        ),
.gtf_ch_rxdfeutovrden                      (gtf_ch_rxdfeutovrden                      ),
.gtf_ch_rxdfevphold                        (gtf_ch_rxdfevphold                        ),
.gtf_ch_rxdfevpovrden                      (gtf_ch_rxdfevpovrden                      ),
.gtf_ch_rxdfexyden                         (gtf_ch_rxdfexyden                         ),
.gtf_ch_rxlpmen                            (gtf_ch_rxlpmen[i]                         ),
.gtf_ch_rxlpmgchold                        (gtf_ch_rxlpmgchold                        ),
.gtf_ch_rxlpmgcovrden                      (gtf_ch_rxlpmgcovrden                      ),
.gtf_ch_rxlpmhfhold                        (gtf_ch_rxlpmhfhold                        ),
.gtf_ch_rxlpmhfovrden                      (gtf_ch_rxlpmhfovrden                      ),
.gtf_ch_rxlpmlfhold                        (gtf_ch_rxlpmlfhold                        ),
.gtf_ch_rxlpmlfklovrden                    (gtf_ch_rxlpmlfklovrden                    ),
.gtf_ch_rxlpmoshold                        (gtf_ch_rxlpmoshold                        ),
.gtf_ch_rxlpmosovrden                      (gtf_ch_rxlpmosovrden                      ),
.gtf_ch_rxoscalreset                       (gtf_ch_rxoscalreset                       ),
.gtf_ch_rxoshold                           (gtf_ch_rxoshold                           ),
.gtf_ch_rxosovrden                         (gtf_ch_rxosovrden                         ),
.gtf_ch_rxpcsreset                         (gtf_ch_rxpcsreset                         ),
.gtf_ch_rxpmareset                         (gtf_ch_rxpmareset                         ),
.gtf_ch_rxpolarity                         (gtf_ch_rxpolarity[i]                      ),
.gtf_ch_rxprbscntreset                     (gtf_ch_rxprbscntreset                     ),
.gtf_ch_rxslipoutclk                       (gtf_ch_rxslipoutclk                       ),
.gtf_ch_rxslippma                          (gtf_ch_rxslippma                          ),
.gtf_ch_rxtermination                      (gtf_ch_rxtermination                      ),
.gtf_ch_rxuserrdy                          (gtf_ch_rxuserrdy                          ),
.gtf_ch_txaxisterr                         (gtf_ch_txaxisterr[i]                      ),
.gtf_ch_txaxistpoison                      (gtf_ch_txaxistpoison[i]                   ),
.gtf_ch_txaxistvalid                       (gtf_ch_txaxistvalid[i]                    ),
.gtf_ch_txdccforcestart                    (gtf_ch_txdccforcestart                    ),
.gtf_ch_txdccreset                         (gtf_ch_txdccreset                         ),
.gtf_ch_txelecidle                         (gtf_ch_txelecidle                         ),
.gtf_ch_txgbseqsync                        (gtf_ch_txgbseqsync                        ),
.gtf_ch_txmuxdcdexhold                     (gtf_ch_txmuxdcdexhold                     ),
.gtf_ch_txmuxdcdorwren                     (gtf_ch_txmuxdcdorwren                     ),
.gtf_ch_txpcsreset                         (gtf_ch_txpcsreset                         ),
.gtf_ch_txpippmen                          (gtf_ch_txpippmen                          ),
.gtf_ch_txpippmovrden                      (gtf_ch_txpippmovrden                      ),
.gtf_ch_txpippmpd                          (gtf_ch_txpippmpd                          ),
.gtf_ch_txpippmsel                         (gtf_ch_txpippmsel                         ),
.gtf_ch_txpisopd                           (gtf_ch_txpisopd                           ),
.gtf_ch_txpmareset                         (gtf_ch_txpmareset                         ),
.gtf_ch_txpolarity                         (gtf_ch_txpolarity[i]                      ),
.gtf_ch_txprbsforceerr                     (gtf_ch_txprbsforceerr                     ),
.gtf_ch_txuserrdy                          (gtf_ch_txuserrdy                          ),
.gtf_ch_gtrsvd                             (gtf_ch_gtrsvd                             ),
.gtf_ch_pcsrsvdin                          (gtf_ch_pcsrsvdin                          ),
.gtf_ch_tstin                              (gtf_ch_tstin                              ),
.gtf_ch_rxelecidlemode                     (gtf_ch_rxelecidlemode                     ),
.gtf_ch_rxmonitorsel                       (gtf_ch_rxmonitorsel                       ),
.gtf_ch_rxpd                               (gtf_ch_rxpd                               ),
.gtf_ch_rxpllclksel                        (gtf_ch_rxpllclksel                        ),
.gtf_ch_rxsysclksel                        (gtf_ch_rxsysclksel                        ),
.gtf_ch_txaxistsof                         (gtf_ch_txaxistsof[2*(i+1)-1:2*i]          ),
.gtf_ch_txpd                               (gtf_ch_txpd                               ),
.gtf_ch_txpllclksel                        (gtf_ch_txpllclksel                        ),
.gtf_ch_txsysclksel                        (gtf_ch_txsysclksel                        ),
.gtf_ch_cpllrefclksel                      (gtf_ch_cpllrefclksel                      ),
.gtf_ch_loopback                           (gtf_ch_loopback[i]                           ),
.gtf_ch_rxoutclksel                        (gtf_ch_rxoutclksel                        ),
.gtf_ch_txoutclksel                        (gtf_ch_txoutclksel                        ),
.gtf_ch_txrawdata                          (gtf_ch_txrawdata[40*(i+1)-1:40*i]         ),
.gtf_ch_rxdfecfokfcnum                     (gtf_ch_rxdfecfokfcnum                     ),
.gtf_ch_rxprbssel                          (gtf_ch_rxprbssel                          ),
.gtf_ch_txprbssel                          (gtf_ch_txprbssel                          ),
.gtf_ch_txaxistterm                        (gtf_ch_txaxistterm[5*(i+1)-1:5*i]         ),
.gtf_ch_txdiffctrl                         (gtf_ch_txdiffctrl[i]                         ),
.gtf_ch_txpippmstepsize                    (gtf_ch_txpippmstepsize                    ),
.gtf_ch_txpostcursor                       (gtf_ch_txpostcursor[i]                       ),
.gtf_ch_txprecursor                        (gtf_ch_txprecursor[i]                        ),
.gtf_ch_txaxistdata                        (gtf_ch_txaxistdata[64*(i+1)-1:64*i]       ),
.gtf_ch_rxckcalstart                       (gtf_ch_rxckcalstart                       ),
.gtf_ch_txmaincursor                       (gtf_ch_txmaincursor[i]                       ),
.gtf_ch_txaxistlast                        (gtf_ch_txaxistlast[8*(i+1)-1:8*i]         ),
.gtf_ch_txaxistpre                         (gtf_ch_txaxistpre[8*(i+1)-1:8*i]          ),
.gtf_ch_ctlrxpauseack                      (gtf_ch_ctlrxpauseack                      ),
.gtf_ch_ctltxpausereq                      (gtf_ch_ctltxpausereq                      ),
.gtf_ch_cpllfbclklost                      (                                          ),
.gtf_ch_cplllock                           (                                          ),
.gtf_ch_cpllrefclklost                     (                                          ),
.gtf_ch_dmonitoroutclk                     (                                          ),
.gtf_ch_eyescandataerror                   (                                          ),
.gtf_ch_gtftxn                             (gtf_ch_gtftxn[i]                          ),
.gtf_ch_gtftxp                             (gtf_ch_gtftxp[i]                          ),
.gtf_ch_gtpowergood                        (gtf_ch_gtpowergood[i]                     ),
.gtf_ch_gtrefclkmonitor                    (                                          ),
.gtf_ch_resetexception                     (                                          ),
.gtf_ch_rxaxisterr                         (                                          ),
.gtf_ch_rxaxistvalid                       (gtf_ch_rxaxistvalid[i]                    ),
.gtf_ch_rxbitslip                          (                                          ),
.gtf_ch_rxcdrlock                          (gtf_ch_rxcdrlock[i]                       ),
.gtf_ch_rxcdrphdone                        (                                          ),
.gtf_ch_rxckcaldone                        (                                          ),
.gtf_ch_rxelecidle                         (                                          ),
.gtf_ch_rxgbseqstart                       (                                          ),
.gtf_ch_rxosintdone                        (                                          ),
.gtf_ch_rxosintstarted                     (                                          ),
.gtf_ch_rxosintstrobedone                  (                                          ),
.gtf_ch_rxosintstrobestarted               (                                          ),
.gtf_ch_rxoutclk                           ( gtf_ch_rxoutclk [i]                      ),
.gtf_ch_rxoutclkfabric                     (                                          ),
.gtf_ch_rxoutclkpcs                        (                                          ),
.gtf_ch_rxphalignerr                       (                                          ),
.gtf_ch_rxpmaresetdone                     (gtf_ch_rxpmaresetdone[i]                  ),
.gtf_ch_rxprbserr                          (                                          ),
.gtf_ch_rxprbslocked                       (                                          ),
.gtf_ch_rxprgdivresetdone                  (                                          ),
.gtf_ch_rxptpsop                           (                                          ),
.gtf_ch_rxptpsoppos                        (                                          ),
.gtf_ch_rxrecclkout                        (gtf_ch_rxrecclkout[i]                     ),
.gtf_ch_rxresetdone                        (gtf_ch_rxresetdone[i]                     ),
.gtf_ch_rxslipdone                         (                                          ),
.gtf_ch_rxslipoutclkrdy                    (                                          ),
.gtf_ch_rxslippmardy                       (                                          ),
.gtf_ch_rxsyncdone                         (gtf_ch_rxsyncdone[i]                      ),
.gtf_ch_statrxbadcode                      (                                          ),
.gtf_ch_statrxbadfcs                       (                                          ),
.gtf_ch_statrxbadpreamble                  (                                          ),
.gtf_ch_statrxbadsfd                       (                                          ),
.gtf_ch_statrxblocklock                    (                                          ),
.gtf_ch_statrxbroadcast                    (                                          ),
.gtf_ch_statrxfcserr                       (                                          ),
.gtf_ch_statrxframingerr                   (                                          ),
.gtf_ch_statrxgotsignalos                  (                                          ),
.gtf_ch_statrxhiber                        (                                          ),
.gtf_ch_statrxinrangeerr                   (                                          ),
.gtf_ch_statrxinternallocalfault           (                                          ),
.gtf_ch_statrxlocalfault                   (                                          ),
.gtf_ch_statrxmulticast                    (                                          ),
.gtf_ch_statrxpkt                          (                                          ),
.gtf_ch_statrxpkterr                       (                                          ),
.gtf_ch_statrxreceivedlocalfault           (                                          ),
.gtf_ch_statrxremotefault                  (                                          ),
.gtf_ch_statrxstatus                       (                                          ),
.gtf_ch_statrxstompedfcs                   (                                          ),
.gtf_ch_statrxtestpatternmismatch          (                                          ),
.gtf_ch_statrxtruncated                    (                                          ),
.gtf_ch_statrxunicast                      (                                          ),
.gtf_ch_statrxvalidctrlcode                (                                          ),
.gtf_ch_statrxvlan                         (                                          ),
.gtf_ch_stattxbadfcs                       (                                          ),
.gtf_ch_stattxbroadcast                    (                                          ),
.gtf_ch_stattxfcserr                       (                                          ),
.gtf_ch_stattxmulticast                    (                                          ),
.gtf_ch_stattxpkt                          (                                          ),
.gtf_ch_stattxpkterr                       (                                          ),
.gtf_ch_stattxunicast                      (                                          ),
.gtf_ch_stattxvlan                         (                                          ),
.gtf_ch_txaxistready                       (gtf_ch_txaxistready[i]                    ),
.gtf_ch_txdccdone                          (                                          ),
.gtf_ch_txgbseqstart                       (                       ),
.gtf_ch_txoutclk                           (                           ),
.gtf_ch_txoutclkfabric                     (                     ),
.gtf_ch_txoutclkpcs                        (                        ),
.gtf_ch_txpmaresetdone                     (gtf_ch_txpmaresetdone[i]                  ),
.gtf_ch_txprgdivresetdone                  (                  ),
.gtf_ch_txptpsop                           (                           ),
.gtf_ch_txptpsoppos                        (                        ),
.gtf_ch_txresetdone                        (gtf_ch_txresetdone[i]                     ),
.gtf_ch_txsyncdone                         (gtf_ch_txsyncdone[i]                      ),
.gtf_ch_txunfout                           (                                          ),
.gtf_ch_txaxistcanstart                    (                                          ),
.gtf_ch_rxinvalidstart                     (                           ),
.gtf_ch_pcsrsvdout                         (        ),
.gtf_ch_pinrsrvdas                         (                         ),
.gtf_ch_rxaxistsof                         (                         ),
.gtf_ch_rxrawdata                          (gtf_ch_rxrawdata[40*(i+1)-1:40*i]         ),
.gtf_ch_statrxbytes                        (gtf_ch_statrxbytes[4*(i+1)-1:4*i]         ),
.gtf_ch_stattxbytes                        (         ),
.gtf_ch_rxaxistterm                        (                        ),
.gtf_ch_rxaxistdata                        (gtf_ch_rxaxistdata[64*(i+1)-1:64*i]       ),
.gtf_ch_rxaxistlast                        (gtf_ch_rxaxistlast[8*(i+1)-1:8*i]         ),
.gtf_ch_rxaxistpre                         (                                          ),
.gtf_ch_rxmonitorout                       (                                          ),
.gtf_ch_statrxpausequanta                  (                                          ),
.gtf_ch_statrxpausereq                     (                                          ),
.gtf_ch_statrxpausevalid                   (                                          ),
.gtf_ch_stattxpausevalid                   (                                          ),
.gtf_ch_gttxreset_out                      (gtf_ch_gttxreset_out[i]                   ),
 .gtwiz_buffbypass_tx_done_out             (gtwiz_buffbypass_tx_done_out_i[i]         ),
 .gtwiz_buffbypass_rx_done_out         (gtwiz_buffbypass_rx_done_out_i[i]        ),
.gtf_txusrclk2_out                         (gtf_txusrclk2_out[i]                      ),
.gtf_rxusrclk2_out                         (gtf_rxusrclk2_out[i]                      )
);

end
endgenerate

gtf_wiz_10_raw_example_gtf_common # (
.AEN_QPLL0_FBDIV             (AEN_QPLL0_FBDIV             ),
.AEN_QPLL1_FBDIV             (AEN_QPLL1_FBDIV             ),
.AEN_SDM0TOGGLE              (AEN_SDM0TOGGLE              ),
.AEN_SDM1TOGGLE              (AEN_SDM1TOGGLE              ),
.A_SDM0TOGGLE                (A_SDM0TOGGLE                ),
.A_SDM1DATA_HIGH             (A_SDM1DATA_HIGH             ),
.A_SDM1DATA_LOW              (A_SDM1DATA_LOW              ),
.A_SDM1TOGGLE                (A_SDM1TOGGLE                ),
.BIAS_CFG0                   (BIAS_CFG0                   ),
.BIAS_CFG1                   (BIAS_CFG1                   ),
.BIAS_CFG2                   (BIAS_CFG2                   ),
.BIAS_CFG3                   (BIAS_CFG3                   ),
.BIAS_CFG4                   (BIAS_CFG4                   ),
.BIAS_CFG_RSVD               (BIAS_CFG_RSVD               ),
.COMMON_CFG0                 (COMMON_CFG0                 ),
.COMMON_CFG1                 (COMMON_CFG1                 ),
.POR_CFG                     (POR_CFG                     ),
.PPF0_CFG                    (PPF0_CFG                    ),
.PPF1_CFG                    (PPF1_CFG                    ),
.QPLL0CLKOUT_RATE            (QPLL0CLKOUT_RATE            ),
.QPLL0_CFG0                  (QPLL0_CFG0                  ),
.QPLL0_CFG1                  (QPLL0_CFG1                  ),
.QPLL0_CFG1_G3               (QPLL0_CFG1_G3               ),
.QPLL0_CFG2                  (QPLL0_CFG2                  ),
.QPLL0_CFG2_G3               (QPLL0_CFG2_G3               ),
.QPLL0_CFG3                  (QPLL0_CFG3                  ),
.QPLL0_CFG4                  (QPLL0_CFG4                  ),
.QPLL0_CP                    (QPLL0_CP                    ),
.QPLL0_CP_G3                 (QPLL0_CP_G3                 ),
.QPLL0_FBDIV                 (QPLL0_FBDIV                 ),
.QPLL0_FBDIV_G3              (QPLL0_FBDIV_G3              ),
.QPLL0_INIT_CFG0             (QPLL0_INIT_CFG0             ),
.QPLL0_INIT_CFG1             (QPLL0_INIT_CFG1             ),
.QPLL0_LOCK_CFG              (QPLL0_LOCK_CFG              ),
.QPLL0_LOCK_CFG_G3           (QPLL0_LOCK_CFG_G3           ),
.QPLL0_LPF                   (QPLL0_LPF                   ),
.QPLL0_LPF_G3                (QPLL0_LPF_G3                ),
.QPLL0_PCI_EN                (QPLL0_PCI_EN                ),
.QPLL0_RATE_SW_USE_DRP       (QPLL0_RATE_SW_USE_DRP       ),
.QPLL0_REFCLK_DIV            (QPLL0_REFCLK_DIV            ),
.QPLL0_SDM_CFG0              (QPLL0_SDM_CFG0              ),
.QPLL0_SDM_CFG1              (QPLL0_SDM_CFG1              ),
.QPLL0_SDM_CFG2              (QPLL0_SDM_CFG2              ),
.QPLL1CLKOUT_RATE            (QPLL1CLKOUT_RATE            ),
.QPLL1_CFG0                  (QPLL1_CFG0                  ),
.QPLL1_CFG1                  (QPLL1_CFG1                  ),
.QPLL1_CFG1_G3               (QPLL1_CFG1_G3               ),
.QPLL1_CFG2                  (QPLL1_CFG2                  ),
.QPLL1_CFG2_G3               (QPLL1_CFG2_G3               ),
.QPLL1_CFG3                  (QPLL1_CFG3                  ),
.QPLL1_CFG4                  (QPLL1_CFG4                  ),
.QPLL1_CP                    (QPLL1_CP                    ),
.QPLL1_CP_G3                 (QPLL1_CP_G3                 ),
.QPLL1_FBDIV                 (QPLL1_FBDIV                 ),
.QPLL1_FBDIV_G3              (QPLL1_FBDIV_G3              ),
.QPLL1_INIT_CFG0             (QPLL1_INIT_CFG0             ),
.QPLL1_INIT_CFG1             (QPLL1_INIT_CFG1             ),
.QPLL1_LOCK_CFG              (QPLL1_LOCK_CFG              ),
.QPLL1_LOCK_CFG_G3           (QPLL1_LOCK_CFG_G3           ),
.QPLL1_LPF                   (QPLL1_LPF                   ),
.QPLL1_LPF_G3                (QPLL1_LPF_G3                ),
.QPLL1_PCI_EN                (QPLL1_PCI_EN                ),
.QPLL1_RATE_SW_USE_DRP       (QPLL1_RATE_SW_USE_DRP       ),
.QPLL1_REFCLK_DIV            (QPLL1_REFCLK_DIV            ),
.QPLL1_SDM_CFG0              (QPLL1_SDM_CFG0              ),
.QPLL1_SDM_CFG1              (QPLL1_SDM_CFG1              ),
.QPLL1_SDM_CFG2              (QPLL1_SDM_CFG2              ),
.RSVD_ATTR0                  (RSVD_ATTR0                  ),
.RSVD_ATTR1                  (RSVD_ATTR1                  ),
.RSVD_ATTR2                  (RSVD_ATTR2                  ),
.RSVD_ATTR3                  (RSVD_ATTR3                  ),
.RXRECCLKOUT0_SEL            (RXRECCLKOUT0_SEL            ),
.RXRECCLKOUT1_SEL            (RXRECCLKOUT1_SEL            ),
.SARC_ENB                    (SARC_ENB                    ),
.SARC_SEL                    (SARC_SEL                    ),
.SDM0INITSEED0_0             (SDM0INITSEED0_0             ),
.SDM0INITSEED0_1             (SDM0INITSEED0_1             ),
.SDM1INITSEED0_0             (SDM1INITSEED0_0             ),
.SDM1INITSEED0_1             (SDM1INITSEED0_1             ),
.SIM_MODE                    (SIM_MODE                    ),
.SIM_RESET_SPEEDUP           (SIM_RESET_SPEEDUP           )
) example_gtf_common_inst(
  .gtf_cm_bgbypassb                 (gtf_cm_bgbypassb                 ),
  .gtf_cm_bgmonitorenb              (gtf_cm_bgmonitorenb              ),
  .gtf_cm_bgpdb                     (gtf_cm_bgpdb                     ),
  .gtf_cm_bgrcalovrdenb             (gtf_cm_bgrcalovrdenb             ),
  .gtf_cm_drpclk                    (gtf_cm_drpclk                    ),
  .gtf_cm_drpen                     (gtf_cm_drpen                     ),
  .gtf_cm_drpwe                     (gtf_cm_drpwe                     ),
  .gtf_cm_gtgrefclk0                (gtf_cm_gtgrefclk0                ),
  .gtf_cm_gtgrefclk1                (gtf_cm_gtgrefclk1                ),
  .gtf_cm_gtnorthrefclk00           (gtf_cm_gtnorthrefclk00           ),
  .gtf_cm_gtnorthrefclk01           (gtf_cm_gtnorthrefclk01           ),
  .gtf_cm_gtnorthrefclk10           (gtf_cm_gtnorthrefclk10           ),
  .gtf_cm_gtnorthrefclk11           (gtf_cm_gtnorthrefclk11           ),
  .gtf_cm_gtrefclk00                (gtf_cm_gtrefclk00                ),
  .gtf_cm_gtrefclk01                (gtf_cm_gtrefclk01                ),
  .gtf_cm_gtrefclk10                (gtf_cm_gtrefclk10                ),
  .gtf_cm_gtrefclk11                (gtf_cm_gtrefclk11                ),
  .gtf_cm_gtsouthrefclk00           (gtf_cm_gtsouthrefclk00           ),
  .gtf_cm_gtsouthrefclk01           (gtf_cm_gtsouthrefclk01           ),
  .gtf_cm_gtsouthrefclk10           (gtf_cm_gtsouthrefclk10           ),
  .gtf_cm_gtsouthrefclk11           (gtf_cm_gtsouthrefclk11           ),
  .gtf_cm_qpll0clkrsvd0             (gtf_cm_qpll0clkrsvd0             ),
  .gtf_cm_qpll0clkrsvd1             (gtf_cm_qpll0clkrsvd1             ),
  .gtf_cm_qpll0lockdetclk           (gtf_cm_qpll0lockdetclk           ),
  .gtf_cm_qpll0locken               (gtf_cm_qpll0locken               ),
  .gtf_cm_qpll0pd                   (gtf_cm_qpll0pd                   ),
  .gtf_cm_qpll0reset                (gtf_cm_qpll0reset                ),
  .gtf_cm_qpll1clkrsvd0             (gtf_cm_qpll1clkrsvd0             ),
  .gtf_cm_qpll1clkrsvd1             (gtf_cm_qpll1clkrsvd1             ),
  .gtf_cm_qpll1lockdetclk           (gtf_cm_qpll1lockdetclk           ),
  .gtf_cm_qpll1locken               (gtf_cm_qpll1locken               ),
  .gtf_cm_qpll1pd                   (gtf_cm_qpll1pd                   ),
  .gtf_cm_qpll1reset                (gtf_cm_qpll1reset                ),
  .gtf_cm_rcalenb                   (gtf_cm_rcalenb                   ),
  .gtf_cm_sdm0reset                 (gtf_cm_sdm0reset                 ),
  .gtf_cm_sdm0toggle                (gtf_cm_sdm0toggle                ),
  .gtf_cm_sdm1reset                 (gtf_cm_sdm1reset                 ),
  .gtf_cm_sdm1toggle                (gtf_cm_sdm1toggle                ),
  .gtf_cm_drpaddr                   (gtf_cm_drpaddr                   ),
  .gtf_cm_drpdi                     (gtf_cm_drpdi                     ),
  .gtf_cm_sdm0width                 (gtf_cm_sdm0width                 ),
  .gtf_cm_sdm1width                 (gtf_cm_sdm1width                 ),
  .gtf_cm_sdm0data                  (gtf_cm_sdm0data                  ),
  .gtf_cm_sdm1data                  (gtf_cm_sdm1data                  ),
  .gtf_cm_qpll0refclksel            (gtf_cm_qpll0refclksel            ),
  .gtf_cm_qpll1refclksel            (gtf_cm_qpll1refclksel            ),
  .gtf_cm_bgrcalovrd                (gtf_cm_bgrcalovrd                ),
  .gtf_cm_qpllrsvd2                 (gtf_cm_qpllrsvd2                 ),
  .gtf_cm_qpllrsvd3                 (gtf_cm_qpllrsvd3                 ),
  .gtf_cm_pmarsvd0                  (gtf_cm_pmarsvd0                  ),
  .gtf_cm_pmarsvd1                  (gtf_cm_pmarsvd1                  ),
  .gtf_cm_qpll0fbdiv                (gtf_cm_qpll0fbdiv                ),
  .gtf_cm_qpll1fbdiv                (gtf_cm_qpll1fbdiv                ),
  .gtf_cm_qpllrsvd1                 (gtf_cm_qpllrsvd1                 ),
  .gtf_cm_qpllrsvd4                 (gtf_cm_qpllrsvd4                 ),
  .gtf_cm_drprdy                    (gtf_cm_drprdy                    ),
  .gtf_cm_qpll0fbclklost            (gtf_cm_qpll0fbclklost            ),
  .gtf_cm_qpll0lock                 (gtf_cm_qpll0lock                 ),
  .gtf_cm_qpll0outclk               (gtf_cm_qpll0outclk               ),
  .gtf_cm_qpll0outrefclk            (gtf_cm_qpll0outrefclk            ),
  .gtf_cm_qpll0refclklost           (gtf_cm_qpll0refclklost           ),
  .gtf_cm_qpll1fbclklost            (gtf_cm_qpll1fbclklost            ),
  .gtf_cm_qpll1lock                 (gtf_cm_qpll1lock                 ),
  .gtf_cm_qpll1outclk               (gtf_cm_qpll1outclk               ),
  .gtf_cm_qpll1outrefclk            (gtf_cm_qpll1outrefclk            ),
  .gtf_cm_qpll1refclklost           (gtf_cm_qpll1refclklost           ),
  .gtf_cm_refclkoutmonitor0         (gtf_cm_refclkoutmonitor0         ),
  .gtf_cm_refclkoutmonitor1         (gtf_cm_refclkoutmonitor1         ),
  .gtf_cm_sdm0testdata              (gtf_cm_sdm0testdata              ),
  .gtf_cm_sdm1testdata              (gtf_cm_sdm1testdata              ),
  .gtf_cm_drpdo                     (gtf_cm_drpdo                     ),
  .gtf_cm_rxrecclk0sel              (gtf_cm_rxrecclk0sel              ),
  .gtf_cm_rxrecclk1sel              (gtf_cm_rxrecclk1sel              ),
  .gtf_cm_sdm0finalout              (gtf_cm_sdm0finalout              ),
  .gtf_cm_sdm1finalout              (gtf_cm_sdm1finalout              ),
  .gtf_cm_pmarsvdout0               (gtf_cm_pmarsvdout0               ),
  .gtf_cm_pmarsvdout1               (gtf_cm_pmarsvdout1               ),
  .gtf_cm_qplldmonitor0             (gtf_cm_qplldmonitor0             ),
  .gtf_cm_qplldmonitor1             (gtf_cm_qplldmonitor1             )
);


// assign gtf_cm_qpll0_lock = plllock_tx_in;


endmodule
`default_nettype wire
