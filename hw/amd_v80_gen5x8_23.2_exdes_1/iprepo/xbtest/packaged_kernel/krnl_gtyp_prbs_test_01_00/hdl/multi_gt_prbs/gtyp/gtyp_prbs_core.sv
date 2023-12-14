
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

module gtyp_prbs_core #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_GT_NUM_LANE         = 4,
    parameter integer C_GT_RATE             = 1,
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

    input  wire         gt_refclk_in,
    output wire         gt_refclk_out,

    input wire [11 : 0] loopback,
    input wire [3 : 0]  rxlpmen,
    input wire [3 : 0]  rxpolarity,
    input wire [3 : 0]  txpolarity,
    input wire [19 : 0] txdiffctrl,
    input wire [27 : 0] txmaincursor,
    input wire [19 : 0] txpostcursor,
    input wire [19 : 0] txprecursor,

    input wire          bridge_reset_all,
    input wire          reset_tx_pll_and_datapath,
    input wire          reset_rx_pll_and_datapath,
    input wire          reset_tx_datapath,
    input wire          reset_rx_datapath,

    output wire         gt_power_good,
    output wire         gt_power_good_f,
    output wire         gt_power_good_r,
    output wire         lcpll_lock,
    output wire         lcpll_lock_f,
    output wire         lcpll_lock_r,

    output wire         bridge_reset_tx_done,
    output wire         bridge_reset_tx_done_f,
    output wire         bridge_reset_tx_done_r,
    output wire         bridge_reset_rx_done,
    output wire         bridge_reset_rx_done_f,
    output wire         bridge_reset_rx_done_r,

    input wire          Tx_Enable, // n/c, forced to 1 in PRBS generator
    input wire          Rx_Enable,
    input wire          clear_status,
    input wire          Disable_PRBS_Ref,
    input wire  [3 : 0] Tx_Error_Inject,

    output wire [3 : 0]    Rx_PRBS_Error,
    output wire [3 : 0]    Rx_Seed_Zero,
    output wire [3 : 0]    Tx_Seed_Zero,

    output wire [47 : 0]   rx_word_cnt[3:0],
    output wire [47 : 0]   rx_err_bit_cnt[3:0],
    output wire [47 : 0]   tx_word_cnt[3:0],

    input  wire            latch_status,
    input  wire            toggle_1_sec

);
    timeunit 1ps;
    timeprecision 1ps;

    localparam DATA_SIZE = 64;
    localparam SIM_PRBS
    // synthesis translate_off
                        = 1;
    localparam DUMMY_SIM
    // synthesis translate_on
                        = 0;

    wire [3 : 0]  rx_polarity;
    wire [3 : 0]  tx_polarity;

    wire          Tx_User_Clk;
    wire          Rx_User_Clk;
    wire [DATA_SIZE-1:0]    tx_raw_data[0 : 3];
    wire [1:0]              tx_raw_header[0 : 3];
    wire [DATA_SIZE-1:0]    rx_raw_data[0 : 3];
    wire [1:0]              rx_raw_header[0 : 3];

    wire power_good;
    wire [1 : 0] lcpll_lock_i;
    wire [1 : 0] rpll_lock_i;
    wire reset_tx_done;
    wire reset_rx_done;

    wire link_status;
    wire [3:0] rate_sel = 'h0;


    wire  bridge_reset_all_cdc;
    wire  reset_tx_pll_and_datapath_cdc;
    wire  reset_rx_pll_and_datapath_cdc;
    wire  reset_tx_datapath_cdc;
    wire  reset_rx_datapath_cdc;

    localparam RST_QTY = 5;
    wire [RST_QTY-1 : 0] rst_in, rst_cdc;
    assign rst_in = {bridge_reset_all, reset_tx_pll_and_datapath, reset_rx_pll_and_datapath, reset_tx_datapath, reset_rx_datapath};
    assign {bridge_reset_all_cdc, reset_tx_pll_and_datapath_cdc, reset_rx_pll_and_datapath_cdc, reset_tx_datapath_cdc, reset_rx_datapath_cdc} = rst_cdc;
    generate
    for (genvar rst_idx = 0; rst_idx < RST_QTY; rst_idx++) begin : rst_resync

        wire reset_cdc;
        xpm_cdc_async_rst #(
            .DEST_SYNC_FF    ( DEST_SYNC_FF ),
            .RST_ACTIVE_HIGH ( 1            )  // integer; 0=active low reset, 1=active high reset
        ) xpm_cdc_async_reset (
            .src_arst   ( rst_in[rst_idx]   ),
            .dest_clk   ( drp_clk           ),
            .dest_arst  ( reset_cdc         )
        );

        logic [4:0] reset_cnt = 'h0;

        always_ff @(posedge drp_clk) begin
            if (reset_cdc) begin
                reset_cnt <= {$size(reset_cnt){1'b1}};
            end
            if (reset_cnt[$high(reset_cnt)] != 'h0) begin
                reset_cnt <= reset_cnt - 'b1;
            end
        end
        assign rst_cdc[rst_idx] = reset_cnt[$high(reset_cnt)];

    end
    endgenerate

    generate
        if (SIM_PRBS == 1) begin : debug
            wire [65:0] loopback_raw_data;
            gtyp_raw_prbs_data_gen_term #(
                .DEST_SYNC_FF (DEST_SYNC_FF)
            ) prbs_debug (
                .Rx_Usr_Clk         ( ap_clk             ),
                .Tx_Usr_Clk         ( ap_clk             ),

                .Tx_Error_Inject    ( Tx_Error_Inject[0] ),

                .Rx_Enable          ( Rx_Enable           ),
                .clear_status       ( clear_status        ),

                .Disable_PRBS_Ref   ( Disable_PRBS_Ref  ),
                .toggle_1_sec       ( toggle_1_sec      ),

                .Rx_Data            ( loopback_raw_data[63:0] ),   // loopabck data
                .Rx_Header          ( loopback_raw_data[65:64] ),   // loopabck data

                .Rx_PRBS_Error      ( ),
                .Rx_Seed_Zero       ( ),
                .Rx_Word_Cnt        ( ),
                .Rx_Err_Bit_Cnt     ( ),
                .Tx_Word_Cnt        ( ),

                .Tx_Data            ( loopback_raw_data[63:0]  ),
                .Tx_Header          ( loopback_raw_data[65:64]  ),
                .Tx_Seed_Zero       (  )
            );
        end
    endgenerate


    // detect rising and falling edge into ap_clk domain
    (* dont_touch = "true" *) wire [3:0]  mon_async_in, mon_async_cdc, mon_async_r, mon_async_f;
    assign mon_async_in = { power_good, lcpll_lock_i[0], reset_rx_done, reset_tx_done };

    assign { gt_power_good,   lcpll_lock,   bridge_reset_rx_done,   bridge_reset_tx_done  } = mon_async_cdc;
    assign { gt_power_good_r, lcpll_lock_r, bridge_reset_rx_done_r, bridge_reset_tx_done_r} = mon_async_r;
    assign { gt_power_good_f, lcpll_lock_f, bridge_reset_rx_done_f, bridge_reset_tx_done_f} = mon_async_f;

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .WIDTH          ( $size(mon_async_in) )
    )
    cdc_async_mon (
        .src_clk  (  1'h0           ),
        .src_in   ( mon_async_in    ),
        .dest_out ( mon_async_cdc   ),
        .dest_clk ( ap_clk          )
    );

    monitor_signal_r_f_edges  #(
        .WIDTH                  ( $size(mon_async_in) ),
        .DEST_SYNC_FF           ( DEST_SYNC_FF )
    ) mon_async (
        .clk_in                 ( ap_clk ),
        .async_clear            ( clear_status  ),
        .sig_in                 ( mon_async_cdc ),

        .sig_r                  ( mon_async_r),
        .sig_f                  ( mon_async_f)
    );

    generate

        genvar i;
        for (i=0; i <= 3; i=i+1) begin : mon_rst_done

            xpm_cdc_single #(
                .DEST_SYNC_FF   ( DEST_SYNC_FF ),
                .INIT_SYNC_FF   ( 0            ),
                .SRC_INPUT_REG  ( 0            )
            )
            cdc_rx_pol (
                .src_clk  (  1'h0           ),
                .src_in   ( rxpolarity[i]   ),
                .dest_out ( rx_polarity[i]  ),
                .dest_clk ( Rx_User_Clk     )
            );

            xpm_cdc_single #(
                .DEST_SYNC_FF   ( DEST_SYNC_FF ),
                .INIT_SYNC_FF   ( 0            ),
                .SRC_INPUT_REG  ( 0            )
            )
            cdc_tx_pol (
                .src_clk  (  1'h0           ),
                .src_in   ( txpolarity[i]   ),
                .dest_out ( tx_polarity[i]  ),
                .dest_clk ( Tx_User_Clk     )
            );

        end
    endgenerate

    wire [$size(loopback)-1 : 0] loopback_cdc;

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .WIDTH          ( $size(loopback) )
    )
    cdc_lpbk (
        .src_clk  (  1'h0           ),
        .src_in   ( loopback        ),
        .dest_out ( loopback_cdc    ),
        .dest_clk ( drp_clk         )
    );


    wire  [2:0]       lane_loopback    [0:3];
    wire  [4:0]       lane_txdiffctrl  [0:3];
    wire  [6:0]       lane_txmaincursor[0:3];
    wire  [4:0]       lane_txpostcursor[0:3];
    wire  [4:0]       lane_txprecursor [0:3];

    assign lane_loopback[0] = loopback_cdc[2:0];
    assign lane_loopback[1] = loopback_cdc[5:3];
    assign lane_loopback[2] = loopback_cdc[8:6];
    assign lane_loopback[3] = loopback_cdc[11:9];
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

    wire [127:0] ch0_rxdata_ext;
    wire [127:0] ch1_rxdata_ext;
    wire [127:0] ch2_rxdata_ext;
    wire [127:0] ch3_rxdata_ext;

    wire [5:0] ch0_rxheader_ext;
    wire [5:0] ch1_rxheader_ext;
    wire [5:0] ch2_rxheader_ext;
    wire [5:0] ch3_rxheader_ext;

    assign rx_raw_data[0] = ch0_rxdata_ext[63:0];
    assign rx_raw_data[1] = ch1_rxdata_ext[63:0];
    assign rx_raw_data[2] = ch2_rxdata_ext[63:0];
    assign rx_raw_data[3] = ch3_rxdata_ext[63:0];

    assign rx_raw_header[0] = ch0_rxheader_ext[1:0];
    assign rx_raw_header[1] = ch1_rxheader_ext[1:0];
    assign rx_raw_header[2] = ch2_rxheader_ext[1:0];
    assign rx_raw_header[3] = ch3_rxheader_ext[1:0];


    gtyp_xbtest_sub_gt_0_wrapper  u_gtyp_quad (
        .GT_Serial_grx_n                            (QSFP_RX_N),
        .GT_Serial_grx_p                            (QSFP_RX_P),
        .GT_Serial_gtx_n                            (QSFP_TX_N),
        .GT_Serial_gtx_p                            (QSFP_TX_P),
        .apb3clk_gt_bridge_ip_0                     (drp_clk),
        .apb3clk_quad                               (drp_clk),
        .ch0_rxdata_ext                             (ch0_rxdata_ext),
        .ch0_rxheader_ext                           (ch0_rxheader_ext),
        .ch0_txdata_ext                             ({64'b0,tx_raw_data[0]}),
        .ch0_txheader_ext                           ({4'b0,tx_raw_header[0]}),
        .ch0_txsequence_ext                         ('b0),
        .ch1_rxdata_ext                             (ch1_rxdata_ext),
        .ch1_rxheader_ext                           (ch1_rxheader_ext),
        .ch1_txdata_ext                             ({64'b0,tx_raw_data[1]}),
        .ch1_txheader_ext                           ({4'b0,tx_raw_header[1]}),
        .ch1_txsequence_ext                         ('b0),
        .ch2_rxdata_ext                             (ch2_rxdata_ext),
        .ch2_rxheader_ext                           (ch2_rxheader_ext),
        .ch2_txdata_ext                             ({64'b0,tx_raw_data[2]}),
        .ch2_txheader_ext                           ({4'b0,tx_raw_header[2]}),
        .ch2_txsequence_ext                         ('b0),
        .ch3_rxdata_ext                             (ch3_rxdata_ext),
        .ch3_rxheader_ext                           (ch3_rxheader_ext),
        .ch3_txdata_ext                             ({64'b0,tx_raw_data[3]}),
        .ch3_txheader_ext                           ({4'b0,tx_raw_header[3]}),
        .ch3_txsequence_ext                         ('b0),
        .gt_bridge_ip_0_diff_gt_ref_clock_clk_n     (QSFP_CK_N),
        .gt_bridge_ip_0_diff_gt_ref_clock_clk_p     (QSFP_CK_P),
        .gt_powergood                               (power_good),
        .gt_refclk_in                               (gt_refclk_in),
        .gt_refclk_out                              (gt_refclk_out),
        .gt_reset_gt_bridge_ip_0                    (bridge_reset_all_cdc),
        .hsclk0_lcplllock_0                         (lcpll_lock_i[0]),
        .hsclk0_rplllock_0                          (rpll_lock_i[0]),
        .hsclk1_lcplllock_0                         (lcpll_lock_i[1]),
        .hsclk1_rplllock_0                          (rpll_lock_i[1]),
        .lane_loopback_0                            (lane_loopback[0]    ),
        .lane_loopback_1                            (lane_loopback[1]    ),
        .lane_loopback_2                            (lane_loopback[2]    ),
        .lane_loopback_3                            (lane_loopback[3]    ),
        .lane_rxlpmen_0                             (rxlpmen[0]     ),
        .lane_rxlpmen_1                             (rxlpmen[1]     ),
        .lane_rxlpmen_2                             (rxlpmen[2]     ),
        .lane_rxlpmen_3                             (rxlpmen[3]     ),
        .lane_rxpolarity_0                          (rx_polarity[0]  ),
        .lane_rxpolarity_1                          (rx_polarity[1]  ),
        .lane_rxpolarity_2                          (rx_polarity[2]  ),
        .lane_rxpolarity_3                          (rx_polarity[3]  ),
        .lane_txdiffctrl_0                          (lane_txdiffctrl[0]  ),
        .lane_txdiffctrl_1                          (lane_txdiffctrl[1]  ),
        .lane_txdiffctrl_2                          (lane_txdiffctrl[2]  ),
        .lane_txdiffctrl_3                          (lane_txdiffctrl[3]  ),
        .lane_txmaincursor_0                        (lane_txmaincursor[0]),
        .lane_txmaincursor_1                        (lane_txmaincursor[1]),
        .lane_txmaincursor_2                        (lane_txmaincursor[2]),
        .lane_txmaincursor_3                        (lane_txmaincursor[3]),
        .lane_txpolarity_0                          (tx_polarity[0]  ),
        .lane_txpolarity_1                          (tx_polarity[1]  ),
        .lane_txpolarity_2                          (tx_polarity[2]  ),
        .lane_txpolarity_3                          (tx_polarity[3]  ),
        .lane_txpostcursor_0                        (lane_txpostcursor[0]),
        .lane_txpostcursor_1                        (lane_txpostcursor[1]),
        .lane_txpostcursor_2                        (lane_txpostcursor[2]),
        .lane_txpostcursor_3                        (lane_txpostcursor[3]),
        .lane_txprecursor_0                         (lane_txprecursor[0] ),
        .lane_txprecursor_1                         (lane_txprecursor[1] ),
        .lane_txprecursor_2                         (lane_txprecursor[2] ),
        .lane_txprecursor_3                         (lane_txprecursor[3] ),
        .link_status_gt_bridge_ip_0                 (link_status),
        .rate_sel_gt_bridge_ip_0                    (rate_sel),
        .reset_rx_datapath_in                       (reset_rx_datapath_cdc),
        .reset_rx_pll_and_datapath_in               (reset_rx_pll_and_datapath_cdc),
        .reset_tx_datapath_in                       (reset_tx_datapath_cdc),
        .reset_tx_pll_and_datapath_in               (reset_tx_pll_and_datapath_cdc),
        .rx_resetdone_out_gt_bridge_ip_0            (reset_rx_done),
        .rxusrclk_gt_bridge_ip_0                    (Rx_User_Clk),
        .tx_resetdone_out_gt_bridge_ip_0            (reset_tx_done),
        .txusrclk_gt_bridge_ip_0                    (Tx_User_Clk)
    );

    genvar index;
    generate
        for (index=0; index < C_GT_NUM_LANE; index=index+1)
        begin : lane_raw_prbs
            gtyp_raw_prbs_data_gen_term #(
                .DEST_SYNC_FF ( DEST_SYNC_FF    )
            ) u_prbs_gen_term (
                .Rx_Usr_Clk             ( Rx_User_Clk        ),
                .Tx_Usr_Clk             ( Tx_User_Clk        ),

                .Tx_Error_Inject        ( Tx_Error_Inject[index]    ),

                .Rx_Enable              ( Rx_Enable                 ),
                .clear_status           ( clear_status              ),

                .Disable_PRBS_Ref       ( Disable_PRBS_Ref          ),
                .latch_status           ( latch_status              ),
                .toggle_1_sec           ( toggle_1_sec              ),

                .Rx_Data                ( rx_raw_data[index]        ),
                .Rx_Header              ( rx_raw_header[index]      ),

                .Rx_PRBS_Error          ( Rx_PRBS_Error[index]      ),
                .Rx_Seed_Zero           ( Rx_Seed_Zero[index]       ),
                .Rx_Word_Cnt            ( rx_word_cnt[index]        ),
                .Rx_Err_Bit_Cnt         ( rx_err_bit_cnt[index]     ),
                .Tx_Word_Cnt            ( tx_word_cnt[index]        ),

                .Tx_Data                ( tx_raw_data[index]        ),
                .Tx_Header              ( tx_raw_header[index]      ),
                .Tx_Seed_Zero           ( Tx_Seed_Zero[index]       )
            );

        end
    endgenerate

endmodule : gtyp_prbs_core
`default_nettype wire
