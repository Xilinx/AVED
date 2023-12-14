
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

module gt_prbs_25_core #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer DISABLE_SIM_ASSERT    = 0
) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_rst,
    input  wire         drp_clk,

    input  wire         QSFP_CK_P,
    input  wire         QSFP_CK_N,
    input  wire [3 : 0] QSFP_RX_N,
    input  wire [3 : 0] QSFP_RX_P,
    output wire [3 : 0] QSFP_TX_N,
    output wire [3 : 0] QSFP_TX_P,

    input wire [11 : 0] loopback,
    input wire [3 : 0]  rxlpmen,
    input wire [3 : 0]  txpolarity,
    input wire [19 : 0] txdiffctrl,
    input wire [27 : 0] txmaincursor,
    input wire [19 : 0] txpostcursor,
    input wire [19 : 0] txprecursor,

    input  wire         ctrl_gt_reset,
    output wire [3:0]   gt_power_good,
    output wire         gt_reset_tx_done,
    output wire         gt_reset_rx_done,

    input wire          Tx_Enable,
    input wire          Rx_Enable,
    input wire          Clear_Rx_PRBS_Error,
    input wire          Clear_Rx_Seed_Zero,
    input wire          Clear_Tx_Seed_Zero,
    input wire          Disable_PRBS_Ref,
    input wire  [3 : 0] Tx_Error_Inject,

    output wire [3 : 0]    Rx_PRBS_Error,
    output wire [3 : 0]    Rx_Seed_Zero,
    output wire [3 : 0]    Tx_Seed_Zero,

    output wire [47 : 0]   rx_word_cnt[3:0],
    output wire [47 : 0]   rx_err_word_cnt[3:0]

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
wire [27 : 0]   txsequence_in = 'b0;
wire [7:0]      rxdatavalid_out;
wire [23 : 0]   rxheader_out;
wire [7 : 0]    rxheadervalid_out;
wire [3 : 0]    eyescanreset_in;
wire [11 : 0]   rxrate_in;
wire [0 : 0]    gtrefclk00_in;
wire            Tx_Buf_Clk;
wire            gt_reset;

// un-used control signals
wire [3 : 0]    txpolarity_in;

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

xpm_cdc_async_rst #(
    .DEST_SYNC_FF       (DEST_SYNC_FF),
    .INIT_SYNC_FF       (0),
    .RST_ACTIVE_HIGH    (1) // DECIMAL; 0=active low reset, 1=active high reset
)
xpm_cdc_async_rst_inst (
    .src_arst   (ctrl_gt_reset),
    .dest_clk   (drp_clk),
    .dest_arst  (gt_reset)
);

// DO NOT CONNECT txmaincursor_in, it kills the traffic even if left to default value
gt_wiz_25 gt_wiz_inst (
    .gtrefclk00_in                      (gtrefclk00_in          ),

    .rxoutclk_out                       (rxoutclk_out           ),
    .txoutclk_out                       (txoutclk_out           ),
    .rxusrclk_in                        (rxusrclk_in            ),
    .rxusrclk2_in                       (rxusrclk2_in           ),
    .txusrclk_in                        (txusrclk_in            ),
    .txusrclk2_in                       (txusrclk2_in           ),

    .gtwiz_reset_clk_freerun_in         (drp_clk                ),
    .gtwiz_reset_all_in                 (gt_reset               ),
    .gtpowergood_out                    (gt_power_good          ),
    .gtwiz_reset_tx_done_out            (gt_reset_tx_done       ),
    .gtwiz_reset_rx_done_out            (gt_reset_rx_done       ),

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

    .rxdatavalid_out                    (rxdatavalid_out        ),
    .rxheader_out                       (rxheader_out           ),
    .rxheadervalid_out                  (rxheadervalid_out      ),
    .txheader_in                        (txheader_in            ),
    .txsequence_in                      (txsequence_in          ),

    .loopback_in                        (loopback               ),
    .rxlpmen_in                         (rxlpmen                ),
    .txpolarity_in                      (txpolarity_in          ),
    .txdiffctrl_in                      (txdiffctrl             ),
//    .txmaincursor_in                    (txmaincursor           ),
    .txpostcursor_in                    (txpostcursor           ),
    .txprecursor_in                     (txprecursor            )

);


    ///////////////////////////////////////////////////////////////////////////////
    //      Generate the 4 Channels
    ///////////////////////////////////////////////////////////////////////////////

genvar index;
generate
for (index=0; index < 4; index=index+1) begin : CH

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

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF    ),
        .INIT_SYNC_FF   ( 0               ),
        .SRC_INPUT_REG  ( 0               ),
        .SIM_ASSERT_CHK ( 0               )
    )
    xpm_cdc_tx_pol (
        .src_clk  ( ap_clk                ),
        .src_in   ( txpolarity[index]     ),
        .dest_out ( txpolarity_in[index]  ),
        .dest_clk ( Tx_Buf_Clk            )
    );

    ///////////////////////////////////////////////////////////////////////////////
    //      Process the Traffic
    ///////////////////////////////////////////////////////////////////////////////


    gt_prbs_25 #(
        .DEST_SYNC_FF (DEST_SYNC_FF)
    ) chan_prbs_i (
        .Rx_Usr_Clk     (Rx_Buf_Clk),
        .Tx_Usr_Clk     (Tx_Buf_Clk),

        .Tx_Enable          ( Tx_Enable              ),
        .Tx_Error_Inject    ( Tx_Error_Inject[index] ),

        .Rx_Enable           ( Rx_Enable           ),
        .Clear_Rx_PRBS_Error ( Clear_Rx_PRBS_Error ),
        .Clear_Rx_Seed_Zero  ( Clear_Rx_Seed_Zero  ),
        .Clear_Tx_Seed_Zero  ( Clear_Tx_Seed_Zero  ),

        .Disable_PRBS_Ref    ( Disable_PRBS_Ref  ),

        .Rx_Data        (gtwiz_userdata_rx_out[index * 64 +: 64]   ),
        .Rx_Header      (rxheader_out[index * 6 +: 2]              ),
        .Rx_Slip        (rxgearboxslip_in[index]                   ),

        .Rx_PRBS_Error      ( Rx_PRBS_Error[index]  ),
        .Rx_Seed_Zero       ( Rx_Seed_Zero[index]   ),
        .Rx_Word_Cnt        ( rx_word_cnt[index]    ),
        .Rx_Err_Word_Cnt    ( rx_err_word_cnt[index]),

        .Tx_Data        (gtwiz_userdata_tx_in[index * 64 +: 64]    ),
        .Tx_Header      (txheader_in[index * 6 +: 2]               ),
        .Tx_Seed_Zero   ( Tx_Seed_Zero[index] )
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

endmodule : gt_prbs_25_core
`default_nettype wire
