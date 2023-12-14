
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

module gt_test_25_top #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 1,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_BLOCK_ID            = 0,    // Block_ID (POWER = 0, MEMORY = 1, GT = 2 or 3)
    parameter integer C_GT_INDEX            = 0,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer DISABLE_SIM_ASSERT    = 0
) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_rst,
    input  wire         ap_clk_2,
    input  wire         ap_rst_2,

    input  wire         QSFP_CK_P,
    input  wire         QSFP_CK_N,
    input  wire [3 : 0] QSFP_RX_N,
    input  wire [3 : 0] QSFP_RX_P,
    output wire [3 : 0] QSFP_TX_N,
    output wire [3 : 0] QSFP_TX_P,

    output wire         XTAL_OE,
    output wire         XTAL_FS,

    input  wire         cs,
    input  wire         we,
    input  wire [11:0]  addr,
    input  wire [31:0]  wdata,
    output wire [31:0]  rdata,
    output wire         cmd_cmplt
);
    timeunit 1ps;
    timeprecision 1ps;

wire [3 : 0]    rxoutclk_out;
wire [3 : 0]    txoutclk_out;
wire [3 : 0]    rxusrclk_in;
wire [3 : 0]    rxusrclk2_in;
wire [3 : 0]    txusrclk_in;
wire [3 : 0]    txusrclk2_in;
wire [255 : 0]  gtwiz_userdata_tx_in;
wire [255 : 0]  gtwiz_userdata_rx_out;
wire [3 : 0]    rxgearboxslip_in;
wire [23 : 0]   txheader_in;
wire [27 : 0]   txsequence_in;
wire [23 : 0]   rxheader_out;
wire [7 : 0]    rxheadervalid_out;
wire [39 : 0]   drpaddr_in;
wire [3 : 0]    drpclk_in;
wire [63 : 0]   drpdi_in;
wire [3 : 0]    drpen_in;
wire [3 : 0]    drpwe_in;
wire [3 : 0]    eyescanreset_in;
wire [3 : 0]    rxlpmen_in;
wire [11 : 0]   rxrate_in;
wire [19 : 0]   txdiffctrl_in;
wire [19 : 0]   txpostcursor_in;
wire [19 : 0]   txprecursor_in;
wire [63 : 0]   drpdo_out;
wire [3 : 0]    drprdy_out;
wire [0 : 0]    gtrefclk00_in;
wire            Tx_Buf_Clk;

wire [ 11: 0]   ctrl_gt_loopback;
wire [ 0: 0]    ctrl_gt_reset;

wire            ctrl_scramb_dis;
wire            ctrl_retime_dis;
wire            ctrl_align_dis;

///////////////////////////////////////////////////////////////////////////////
//
//      Generate a 100 MHz
//
///////////////////////////////////////////////////////////////////////////////
wire         ibert_clk;
BUFGCE_DIV #(
        .BUFGCE_DIVIDE(3)
    ) DRP_CK (
        .I      (ap_clk     ),
        .CE     (1'b1       ),
        .CLR    (1'b0       ),
        .O      (ibert_clk  )
    );

IBUFDS_GTE4 #(
    .REFCLK_EN_TX_PATH  (1'b0),
    .REFCLK_HROW_CK_SEL (2'b00),
    .REFCLK_ICNTL_RX    (2'b00)
) GTC (
    .I      (QSFP_CK_P),
    .IB     (QSFP_CK_N),
    .CEB    (1'b0),
    .O      (gtrefclk00_in)
);

gt_core_25 gt_inst (
    .gtrefclk00_in                      (gtrefclk00_in          ),

    .rxoutclk_out                       (rxoutclk_out           ),
    .txoutclk_out                       (txoutclk_out           ),
    .rxusrclk_in                        (rxusrclk_in            ),
    .rxusrclk2_in                       (rxusrclk2_in           ),
    .txusrclk_in                        (txusrclk_in            ),
    .txusrclk2_in                       (txusrclk2_in           ),

    .gtwiz_reset_clk_freerun_in         (ap_clk                 ),
    .gtwiz_reset_all_in                 (ap_rst | ctrl_gt_reset ),

    .loopback_in                        (ctrl_gt_loopback       ),

    .gtwiz_userclk_tx_active_in         (1'b1                   ),
    .gtwiz_userclk_rx_active_in         (1'b1                   ),
    .gtwiz_reset_tx_pll_and_datapath_in (1'b0                   ),
    .gtwiz_reset_tx_datapath_in         (1'b0                   ),
    .gtwiz_reset_rx_pll_and_datapath_in (1'b0                   ),
    .gtwiz_reset_rx_datapath_in         (1'b0                   ),

    .gtwiz_userdata_tx_in               (gtwiz_userdata_tx_in   ),
    .gtwiz_userdata_rx_out              (gtwiz_userdata_rx_out  ),

    .gtyrxn_in                          (QSFP_RX_N              ),
    .gtyrxp_in                          (QSFP_RX_P              ),
    .gtytxn_out                         (QSFP_TX_N              ),
    .gtytxp_out                         (QSFP_TX_P              ),

    .rxcdrhold_in                       (4'b0000                ),
    .rxgearboxslip_in                   (rxgearboxslip_in       ),

    .rxheader_out                       (rxheader_out           ),
    .rxheadervalid_out                  (rxheadervalid_out      ),
    .rxlpmen_in                         (rxlpmen_in             ),
    .rxrate_in                          (rxrate_in              ),

    .txheader_in                        (txheader_in            ),
    .txsequence_in                      (txsequence_in          ),
    .txdiffctrl_in                      (txdiffctrl_in          ),
    .txpostcursor_in                    (txpostcursor_in        ),
    .txprecursor_in                     (txprecursor_in         ),

    .eyescanreset_in                    (eyescanreset_in        ),

    .drpaddr_in                         (drpaddr_in             ),
    .drpclk_in                          (drpclk_in              ),
    .drpdi_in                           (drpdi_in               ),
    .drpen_in                           (drpen_in               ),
    .drpwe_in                           (drpwe_in               ),
    .drpdo_out                          (drpdo_out              ),
    .drprdy_out                         (drprdy_out             )
);


ibert isib_inst (
  .drpclk_o                             (drpclk_in),

  .gt0_drpen_o                          (drpen_in[0]),
  .gt0_drpwe_o                          (drpwe_in[0]),
  .gt0_drpaddr_o                        (drpaddr_in[9:0]),
  .gt0_drpdi_o                          (drpdi_in[15:0]),
  .gt0_drprdy_i                         (drprdy_out[0]),
  .gt0_drpdo_i                          (drpdo_out[15:0]),

  .gt1_drpen_o                          (drpen_in[1]),
  .gt1_drpwe_o                          (drpwe_in[1]),
  .gt1_drpaddr_o                        (drpaddr_in[19:10]),
  .gt1_drpdi_o                          (drpdi_in[31:16]),
  .gt1_drprdy_i                         (drprdy_out[1]),
  .gt1_drpdo_i                          (drpdo_out[31:16]),

  .gt2_drpen_o                          (drpen_in[2]),
  .gt2_drpwe_o                          (drpwe_in[2]),
  .gt2_drpaddr_o                        (drpaddr_in[29:20]),
  .gt2_drpdi_o                          (drpdi_in[47:32]),
  .gt2_drprdy_i                         (drprdy_out[2]),
  .gt2_drpdo_i                          (drpdo_out[47:32]),

  .gt3_drpen_o                          (drpen_in[3]),
  .gt3_drpwe_o                          (drpwe_in[3]),
  .gt3_drpaddr_o                        (drpaddr_in[39:30]),
  .gt3_drpdi_o                          (drpdi_in[63:48]),
  .gt3_drprdy_i                         (drprdy_out[3]),
  .gt3_drpdo_i                          (drpdo_out[63:48]),

  .eyescanreset_o                       (eyescanreset_in),
  .rxrate_o                             (rxrate_in),
  .txdiffctrl_o                         (txdiffctrl_in),
  .txprecursor_o                        (txprecursor_in),
  .txpostcursor_o                       (txpostcursor_in),
  .rxlpmen_o                            (rxlpmen_in),
  .rxoutclk_i                           (rxusrclk2_in),
  .clk                                  (ibert_clk)
);

gt_test_reg_array #(
    .C_MAJOR_VERSION           ( C_MAJOR_VERSION            ),
    .C_MINOR_VERSION           ( C_MINOR_VERSION            ),
    .C_BUILD_VERSION           ( C_BUILD_VERSION            ),
    .C_BLOCK_ID                ( C_BLOCK_ID                 ),
    .C_GT_INDEX                ( C_GT_INDEX                 )
) u_reg_array (
    .clk                       ( ap_clk                     ),
    .rst                       ( ap_rst                     ),
    .cs                        ( cs                         ),
    .we                        ( we                         ),
    .addr                      ( addr                       ),
    .wdata                     ( wdata                      ),
    .rdata                     ( rdata                      ),
    .cmd_cmplt                 ( cmd_cmplt                  ),

    .ctrl_gt_loopback          ( ctrl_gt_loopback           ),
    .ctrl_gt_reset             ( ctrl_gt_reset              ),
    .ctrl_scramb_dis           ( ctrl_scramb_dis            ),
    .ctrl_retime_dis           ( ctrl_retime_dis            ),
    .ctrl_align_dis            ( ctrl_align_dis             )
);


    ///////////////////////////////////////////////////////////////////////////////
    //      Generate the 4 Channels
    ///////////////////////////////////////////////////////////////////////////////

genvar index;
generate
for (index=0; index < 4; index=index+1)
  begin

    wire    Rx_Buf_Clk;

    ///////////////////////////////////////////////////////////////////////////////
    //      Create the individual Rx Clocks
    ///////////////////////////////////////////////////////////////////////////////

    BUFG_GT Rx_BUFG_i (
        .I  (rxoutclk_out[index]),

        .CE         (1'b1   ),
        .CEMASK     (1'b0   ),
        .CLR        (1'b0   ),
        .CLRMASK    (1'b0   ),
        .DIV        (3'b000 ),

        .O  (Rx_Buf_Clk)
    );

    assign rxusrclk_in[index]   = Rx_Buf_Clk;
    assign rxusrclk2_in[index]  = Rx_Buf_Clk;

    assign  txusrclk_in[index]  = Tx_Buf_Clk;
    assign  txusrclk2_in[index] = Tx_Buf_Clk;

    ///////////////////////////////////////////////////////////////////////////////
    //      Process the Traffic
    ///////////////////////////////////////////////////////////////////////////////

    gt_channel_25 chan_i (
        .Rx_Usr_Clk  (Rx_Buf_Clk),
        .Tx_Usr_Clk  (Tx_Buf_Clk),

        .ctrl_scramb_dis    ( ctrl_scramb_dis    ),
        .ctrl_retime_dis    ( ctrl_retime_dis    ),
        .ctrl_align_dis     ( ctrl_align_dis     ),

        .Rx_Data     (gtwiz_userdata_rx_out[index * 64 +: 64]   ),
        .Rx_Header   (rxheader_out[index * 6 +: 2]              ),
        .Rx_Slip     (rxgearboxslip_in[index]                   ),

        .Tx_Data     (gtwiz_userdata_tx_in[index * 64 +: 64]    ),
        .Tx_Header   (txheader_in[index * 6 +: 2]               )
    );

  end
endgenerate

    ///////////////////////////////////////////////////////////////////////////////
    //      Create the common Tx Clocks
    ///////////////////////////////////////////////////////////////////////////////

    BUFG_GT Tx_BUFG_i (
        .I  (txoutclk_out[0]),

        .CE         (1'b1   ),
        .CEMASK     (1'b0   ),
        .CLR        (1'b0   ),
        .CLRMASK    (1'b0   ),
        .DIV        (3'b000 ),

        .O  (Tx_Buf_Clk)
    );

endmodule : gt_test_25_top
`default_nettype wire
