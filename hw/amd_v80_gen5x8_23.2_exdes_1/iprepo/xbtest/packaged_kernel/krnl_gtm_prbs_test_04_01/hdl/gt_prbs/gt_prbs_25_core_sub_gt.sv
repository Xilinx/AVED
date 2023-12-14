
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

module gt_prbs_25_core_sub_gt #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer C_GT_IP_SEL           = 0,
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
    output wire         hsclk_lcplllock,

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

wire gt_powergood;

assign gt_power_good[0] = gt_powergood;
assign gt_power_good[1] = gt_powergood;
assign gt_power_good[2] = gt_powergood;
assign gt_power_good[3] = gt_powergood;

wire [1:0] lcpll_lock;
wire link_status;
wire [3:0] rate_sel;
wire [1:0] rpll_lock;

assign hsclk_lcplllock = &lcpll_lock;
// from pg331: gt_ilo_reset, gt_pll_reset, rpll_lock_out, lcpll_lock_out, pcie_rstb, and reset_mask output values
//      are not used and can be ignored.
// Recommend use hsclk0_lcplllock, hsclk1_lcplllock, hsclk0_rplllock, and hsclk1_rplllock signals
//      from respective gt_quad_base IP for monitoring purpose.

// rpll not selected when defining "Transceiver Config Protocol" in GUI
assign hsclk_rplllock = &rpll_lock;

wire [127:0]  rx_data [0:3];
wire [1:0]    rx_data_valid [0:3];
wire [5:0]    rx_header [0:3];
wire [1:0]    rx_header_valid [0:3];
wire [1:0]    rx_startofseq [0:3];

wire [127:0]  tx_data [0:3];
wire [5:0]    tx_header [0:3];
wire [6:0]    tx_sequence [0:3];

wire [3 : 0]    rxgearboxslip_in;

wire            gt_reset;
logic           gt_reset_d1;
logic           gt_reset_d2;
logic           gt_reset_d;
wire tx_user_clk;
wire rx_user_clk;


wire [2 : 0] lane_loopback [0:3];
wire [4 : 0] lane_txdiffctrl [0:3];
wire [6 : 0] lane_txmaincursor [0:3];
wire [4 : 0] lane_txpostcursor [0:3];
wire [4 : 0] lane_txprecursor [0:3];

assign lane_loopback[0] = loopback[2:0];
assign lane_loopback[1] = loopback[5:3];
assign lane_loopback[2] = loopback[8:6];
assign lane_loopback[3] = loopback[11:9];

assign lane_txdiffctrl[0] = txdiffctrl[4:0];
assign lane_txdiffctrl[1] = txdiffctrl[9:5];
assign lane_txdiffctrl[2] = txdiffctrl[14:10];
assign lane_txdiffctrl[3] = txdiffctrl[19:15];

assign lane_txmaincursor[0] = txmaincursor[6:0];
assign lane_txmaincursor[1] = txmaincursor[13:7];
assign lane_txmaincursor[2] = txmaincursor[20:14];
assign lane_txmaincursor[3] = txmaincursor[27:21];

assign lane_txpostcursor[0] = txpostcursor[4:0];
assign lane_txpostcursor[1] = txpostcursor[9:5];
assign lane_txpostcursor[2] = txpostcursor[14:10];
assign lane_txpostcursor[3] = txpostcursor[19:15];

assign lane_txprecursor[0] = txprecursor[4:0];
assign lane_txprecursor[1] = txprecursor[9:5];
assign lane_txprecursor[2] = txprecursor[14:10];
assign lane_txprecursor[3] = txprecursor[19:15];

// un-used control signals
wire [3 : 0]    lane_txpolarity;

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

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .WIDTH          ( 4            )
    )
    xpm_cdc_status (
        .src_clk  (  ap_clk         ),
        .src_in   ( txpolarity      ),
        .dest_out ( lane_txpolarity ),
        .dest_clk ( tx_user_clk     )
    );

// pg331: An active-High, asynchronous pulse of at least one apb3clk period in duration initializes the process.
// create a pulse long enough from the reset resync block
always_ff @(posedge drp_clk) begin
    gt_reset_d1  <= gt_reset;
    gt_reset_d2  <= gt_reset_d1;
    gt_reset_d   <= gt_reset_d1 | gt_reset_d2;
end

// axis_ila_gt_reset u_ila_gt_reset (
//   .clk(drp_clk),
//   .probe0(gt_reset),
//   .probe1(gt_powergood),
//   .probe2(gt_reset_rx_done),
//   .probe3(gt_reset_tx_done),
//   .probe4(lcpll_lock[0]),
//   .probe5(lcpll_lock[1]),
//   .probe6(rpll_lock[0]),
//   .probe7(rpll_lock[1])
// );

// axis_ila_gt_rxdata u_ila_gt_rxdata_0 (
//    .clk(rx_user_clk),
//    .probe0(rx_data_valid[0]),
//    .probe1(rx_data[0]),
//    .probe2(rx_header_valid[0] ),
//    .probe3(rx_header[0] & rx_header[0] ),
//    .probe4(Rx_Seed_Zero[0] ),
//    .probe5(Rx_PRBS_Error[0] )
// );


// from pg331: gt_ilo_reset, gt_pll_reset, rpll_lock_out, lcpll_lock_out, pcie_rstb, and reset_mask output values are not used and can be ignored. Recommend use hsclk0_lcplllock, hsclk1_lcplllock, hsclk0_rplllock, and hsclk1_rplllock signals from respective gt_quad_base IP for monitoring purpose.

gty_xbtest_sub_gt_0_wrapper u_gty_xbtest_sub_gt_0_wrapper (
    .GT_Serial_grx_n                                ( QSFP_RX_N      ),
    .GT_Serial_grx_p                                ( QSFP_RX_P      ),
    .GT_Serial_gtx_n                                ( QSFP_TX_N      ),
    .GT_Serial_gtx_p                                ( QSFP_TX_P      ),

    .apb3clk_gt_bridge_ip_0                         ( drp_clk      ),
    .apb3clk_quad                                   ( drp_clk      ),

    .ch0_rxdata_ext                                 ( rx_data[0]            ),
    .ch0_rxdatavalid_ext                            ( rx_data_valid[0]      ),
    .ch0_rxgearboxslip_ext                          ( rxgearboxslip_in[0]   ),
    .ch0_rxheader_ext                               ( rx_header[0]          ),
    .ch0_rxheadervalid_ext                          ( rx_header_valid[0]    ),
    .ch0_rxstartofseq_ext                           ( rx_startofseq[0]      ),
    .ch0_txdata_ext                                 ( tx_data[0]            ),
    .ch0_txheader_ext                               ( tx_header[0]          ),
    .ch0_txsequence_ext                             ( tx_sequence[0]        ),
    .ch1_rxdata_ext                                 ( rx_data[1]            ),
    .ch1_rxdatavalid_ext                            ( rx_data_valid[1]      ),
    .ch1_rxgearboxslip_ext                          ( rxgearboxslip_in[1]   ),
    .ch1_rxheader_ext                               ( rx_header[1]          ),
    .ch1_rxheadervalid_ext                          ( rx_header_valid[1]    ),
    .ch1_rxstartofseq_ext                           ( rx_startofseq[1]      ),
    .ch1_txdata_ext                                 ( tx_data[1]            ),
    .ch1_txheader_ext                               ( tx_header[1]          ),
    .ch1_txsequence_ext                             ( tx_sequence[1]        ),
    .ch2_rxdata_ext                                 ( rx_data[2]            ),
    .ch2_rxdatavalid_ext                            ( rx_data_valid[2]      ),
    .ch2_rxgearboxslip_ext                          ( rxgearboxslip_in[2]   ),
    .ch2_rxheader_ext                               ( rx_header[2]          ),
    .ch2_rxheadervalid_ext                          ( rx_header_valid[2]    ),
    .ch2_rxstartofseq_ext                           ( rx_startofseq[2]      ),
    .ch2_txdata_ext                                 ( tx_data[2]            ),
    .ch2_txheader_ext                               ( tx_header[2]          ),
    .ch2_txsequence_ext                             ( tx_sequence[2]        ),
    .ch3_rxdata_ext                                 ( rx_data[3]            ),
    .ch3_rxdatavalid_ext                            ( rx_data_valid[3]      ),
    .ch3_rxgearboxslip_ext                          ( rxgearboxslip_in[3]   ),
    .ch3_rxheader_ext                               ( rx_header[3]          ),
    .ch3_rxheadervalid_ext                          ( rx_header_valid[3]    ),
    .ch3_rxstartofseq_ext                           ( rx_startofseq[3]      ),
    .ch3_txdata_ext                                 ( tx_data[3]            ),
    .ch3_txheader_ext                               ( tx_header[3]          ),
    .ch3_txsequence_ext                             ( tx_sequence[3]        ),

    .gt_bridge_ip_0_diff_gt_ref_clock_clk_n         ( QSFP_CK_N             ),
    .gt_bridge_ip_0_diff_gt_ref_clock_clk_p         ( QSFP_CK_P             ),

    .gt_powergood                                   ( gt_powergood          ),
    .gt_reset_gt_bridge_ip_0                        ( gt_reset_d            ),

    .hsclk0_lcplllock_0                             ( lcpll_lock[0]         ),
    .hsclk0_rplllock_0                              ( rpll_lock[0]          ),
    .hsclk1_lcplllock_0                             ( lcpll_lock[1]         ),
    .hsclk1_rplllock_0                              ( rpll_lock[1]          ),

    .lane_loopback_0                                (lane_loopback[0]       ),
    .lane_loopback_1                                (lane_loopback[1]       ),
    .lane_loopback_2                                (lane_loopback[2]       ),
    .lane_loopback_3                                (lane_loopback[3]       ),
    .lane_rxlpmen_0                                 (rxlpmen[0]             ),
    .lane_rxlpmen_1                                 (rxlpmen[1]             ),
    .lane_rxlpmen_2                                 (rxlpmen[2]             ),
    .lane_rxlpmen_3                                 (rxlpmen[3]             ),
    .lane_txdiffctrl_0                              (lane_txdiffctrl[0]     ),
    .lane_txdiffctrl_1                              (lane_txdiffctrl[1]     ),
    .lane_txdiffctrl_2                              (lane_txdiffctrl[2]     ),
    .lane_txdiffctrl_3                              (lane_txdiffctrl[3]     ),
    .lane_txmaincursor_0                            (lane_txmaincursor[0]   ),
    .lane_txmaincursor_1                            (lane_txmaincursor[1]   ),
    .lane_txmaincursor_2                            (lane_txmaincursor[2]   ),
    .lane_txmaincursor_3                            (lane_txmaincursor[3]   ),
    .lane_txpolarity_0                              (lane_txpolarity[0]     ),
    .lane_txpolarity_1                              (lane_txpolarity[1]     ),
    .lane_txpolarity_2                              (lane_txpolarity[2]     ),
    .lane_txpolarity_3                              (lane_txpolarity[3]     ),
    .lane_txpostcursor_0                            (lane_txpostcursor[0]   ),
    .lane_txpostcursor_1                            (lane_txpostcursor[1]   ),
    .lane_txpostcursor_2                            (lane_txpostcursor[2]   ),
    .lane_txpostcursor_3                            (lane_txpostcursor[3]   ),
    .lane_txprecursor_0                             (lane_txprecursor[0]    ),
    .lane_txprecursor_1                             (lane_txprecursor[1]    ),
    .lane_txprecursor_2                             (lane_txprecursor[2]    ),
    .lane_txprecursor_3                             (lane_txprecursor[3]    ),

    .link_status_gt_bridge_ip_0                     (link_status        ),
    .rate_sel_gt_bridge_ip_0                        (rate_sel           ),
    .reset_rx_datapath_in                           ( 1'b0              ),
    .reset_rx_pll_and_datapath_in                   ( 1'b0              ),
    .reset_tx_datapath_in                           ( 1'b0              ),
    .reset_tx_pll_and_datapath_in                   ( 1'b0              ),
    .rx_resetdone_out_gt_bridge_ip_0                ( gt_reset_rx_done  ),
    .rxusrclk_gt_bridge_ip_0                        ( rx_user_clk       ),
    .tx_resetdone_out_gt_bridge_ip_0                ( gt_reset_tx_done  ),
    .txusrclk_gt_bridge_ip_0                        ( tx_user_clk       )
);

    ///////////////////////////////////////////////////////////////////////////////
    //      Generate the 4 Channels
    ///////////////////////////////////////////////////////////////////////////////

genvar index;
generate
for (index=0; index < 4; index=index+1)
    begin

    ///////////////////////////////////////////////////////////////////////////////
    //      Process the Traffic
    ///////////////////////////////////////////////////////////////////////////////


    gt_prbs_25 #(
        .C_GT_IP_SEL  (C_GT_IP_SEL),
        .DEST_SYNC_FF (DEST_SYNC_FF)
    ) chan_prbs_i (
        .Rx_Usr_Clk     (rx_user_clk),
        .Tx_Usr_Clk     (tx_user_clk),

        .Tx_Enable           ( Tx_Enable              ),
        .Tx_Error_Inject     ( Tx_Error_Inject[index] ),

        .Rx_Enable           ( Rx_Enable           ),
        .Clear_Rx_PRBS_Error ( Clear_Rx_PRBS_Error ),
        .Clear_Rx_Seed_Zero  ( Clear_Rx_Seed_Zero  ),
        .Clear_Tx_Seed_Zero  ( Clear_Tx_Seed_Zero  ),

        .Disable_PRBS_Ref    ( Disable_PRBS_Ref  ),

        .Rx_Data             (rx_data[index][63:0]      ),
        .Rx_Header           (rx_header[index][1:0]     ),
        .Rx_Slip             (rxgearboxslip_in[index]   ),

        .Rx_PRBS_Error      ( Rx_PRBS_Error[index]  ),
        .Rx_Seed_Zero       ( Rx_Seed_Zero[index]   ),
        .Rx_Word_Cnt        ( rx_word_cnt[index]    ),
        .Rx_Err_Word_Cnt    ( rx_err_word_cnt[index]),

        .Tx_Data            ( tx_data[index] [63:0]     ),
        .Tx_Header          ( tx_header[index] [1:0]    ),
        .Tx_Seed_Zero       ( Tx_Seed_Zero[index] )
    );


    end
endgenerate


endmodule : gt_prbs_25_core_sub_gt
`default_nettype wire
