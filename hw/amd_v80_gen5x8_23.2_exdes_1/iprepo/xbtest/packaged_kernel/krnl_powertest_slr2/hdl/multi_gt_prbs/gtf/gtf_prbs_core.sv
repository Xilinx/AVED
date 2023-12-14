
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

module gtf_prbs_core #(
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

    output wire [0 : 0] gt_recov_clk_n,
    output wire [0 : 0] gt_recov_clk_p,
    output wire         gt_recov_clk,

    input wire [11 : 0] loopback,
    input wire [3 : 0]  rxlpmen,
    input wire [3 : 0]  rxpolarity,
    input wire [3 : 0]  txpolarity,
    input wire [19 : 0] txdiffctrl,
    input wire [27 : 0] txmaincursor,
    input wire [19 : 0] txpostcursor,
    input wire [19 : 0] txprecursor,

    input wire [3 : 0]  gtwiz_reset_all,
    input wire [3 : 0]  gtwiz_reset_tx_pll_and_datapath,
    input wire [3 : 0]  gtwiz_reset_rx_pll_and_datapath,
    input wire [3 : 0]  gtwiz_reset_tx_datapath,
    input wire [3 : 0]  gtwiz_reset_rx_datapath,

    output wire [3 : 0] gt_power_good,
    output wire [3 : 0] gt_power_good_f,
    output wire [3 : 0] gt_power_good_r,
    output wire         gtf_qpll0_lock,
    output wire         gtf_qpll0_lock_f,
    output wire         gtf_qpll0_lock_r,

    output wire [3 : 0] gt_reset_tx_done,
    output wire [3 : 0] gt_reset_tx_done_f,
    output wire [3 : 0] gt_reset_tx_done_r,
    output wire [3 : 0] gt_bufferbypass_tx_done,
    output wire [3 : 0] gt_bufferbypass_tx_done_f,
    output wire [3 : 0] gt_bufferbypass_tx_done_r,
    output wire [3 : 0] gtf_ch_txresetdone,
    output wire [3 : 0] gtf_ch_txresetdone_f,
    output wire [3 : 0] gtf_ch_txresetdone_r,
    output wire [3 : 0] gtf_ch_txsyncdone,
    output wire [3 : 0] gtf_ch_txsyncdone_f,
    output wire [3 : 0] gtf_ch_txsyncdone_r,

    output wire [3 : 0] gt_reset_rx_done,
    output wire [3 : 0] gt_reset_rx_done_f,
    output wire [3 : 0] gt_reset_rx_done_r,
    output wire [3 : 0] gt_bufferbypass_rx_done,
    output wire [3 : 0] gt_bufferbypass_rx_done_f,
    output wire [3 : 0] gt_bufferbypass_rx_done_r,
    output wire [3 : 0] gtf_ch_rxresetdone,
    output wire [3 : 0] gtf_ch_rxresetdone_f,
    output wire [3 : 0] gtf_ch_rxresetdone_r,
    output wire [3 : 0] gtf_ch_rxsyncdone,
    output wire [3 : 0] gtf_ch_rxsyncdone_f,
    output wire [3 : 0] gtf_ch_rxsyncdone_r,

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

    localparam DATA_SIZE = ( C_GT_RATE == 2 ) ? 40 : 16;

    wire [3 : 0]  rx_polarity;
    wire [3 : 0]  tx_polarity;

    wire [3 : 0]            Tx_Buf_Clk;
    wire [3 : 0]            Rx_Buf_Clk;
    wire [DATA_SIZE-1:0]    tx_raw_data[0 : 3];
    wire [DATA_SIZE-1:0]    rx_raw_data[0 : 3];
    wire [0 : 0]            gtrefclk00_in;
    wire                    gt_wiz_reset_start;
    wire [3 : 0]            gtf_power_good;
    wire                    qpll0_lock;
    wire [3 : 0]            reset_tx_done;
    wire [3 : 0]            reset_rx_done;
    wire [3 : 0]            bufferbypass_tx_done;
    wire [3 : 0]            bufferbypass_rx_done;
    wire [3 : 0]            gtf_txresetdone;
    wire [3 : 0]            gtf_txsyncdone;
    wire [3 : 0]            gtf_rxresetdone;
    wire [3 : 0]            gtf_rxsyncdone;

    localparam SIM_PRBS
    // synthesis translate_off
                        = 1;
    localparam DUMMY_SIM
    // synthesis translate_on
                        = 0;

    generate
        if (SIM_PRBS == 1) begin : debug
            wire [DATA_SIZE-1:0] loopback_raw_data;
            gtf_raw_prbs_data_gen_term #(
                .DEST_SYNC_FF (DEST_SYNC_FF),
                .IO_DATA_SIZE (DATA_SIZE )
            ) prbs_debug (
                .Rx_Usr_Clk         ( ap_clk             ),
                .Tx_Usr_Clk         ( ap_clk             ),

                .Tx_Error_Inject    ( Tx_Error_Inject[0] ),

                .Rx_Enable          ( Rx_Enable           ),
                .clear_status       ( clear_status        ),

                .Disable_PRBS_Ref   ( Disable_PRBS_Ref  ),
                .latch_status       ( latch_status      ),
                .toggle_1_sec       ( toggle_1_sec      ),

                .Rx_Data            ( loopback_raw_data ),   // loopabck data

                .Rx_PRBS_Error      ( ),
                .Rx_Seed_Zero       ( ),
                .Rx_Word_Cnt        ( ),
                .Rx_Err_Bit_Cnt     ( ),
                .Tx_Word_Cnt        ( ),

                .Tx_Data            ( loopback_raw_data  ),
                .Tx_Seed_Zero       (  )
            );
        end
    endgenerate

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


    wire [39:0]     tx_raw_data_int[0 : 3];
    wire [39:0]     rx_raw_data_int[0 : 3];


    (* dont_touch = "true" *) wire [36:0]  mon_async_in, mon_async_cdc, mon_async_r, mon_async_f;
    assign mon_async_in = { gtf_power_good, qpll0_lock, gtf_rxsyncdone, gtf_txsyncdone, bufferbypass_rx_done, bufferbypass_tx_done,
                            reset_rx_done, reset_tx_done, gtf_rxresetdone, gtf_txresetdone };

    assign { gt_power_good,   gtf_qpll0_lock,   gtf_ch_rxsyncdone,   gtf_ch_txsyncdone,   gt_bufferbypass_rx_done,   gt_bufferbypass_tx_done,
             gt_reset_rx_done, gt_reset_tx_done, gtf_ch_rxresetdone, gtf_ch_txresetdone } = mon_async_cdc;
    assign { gt_power_good_r, gtf_qpll0_lock_r, gtf_ch_rxsyncdone_r, gtf_ch_txsyncdone_r, gt_bufferbypass_rx_done_r, gt_bufferbypass_tx_done_r,
             gt_reset_rx_done_r, gt_reset_tx_done_r, gtf_ch_rxresetdone_r, gtf_ch_txresetdone_r } = mon_async_r;
    assign { gt_power_good_f, gtf_qpll0_lock_f, gtf_ch_rxsyncdone_f, gtf_ch_txsyncdone_f, gt_bufferbypass_rx_done_f, gt_bufferbypass_tx_done_f,
             gt_reset_rx_done_f, gt_reset_tx_done_f, gtf_ch_rxresetdone_f, gtf_ch_txresetdone_f } = mon_async_f;

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

        // detect rising and falling edge into their respective clock domains
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
                .dest_clk ( Rx_Buf_Clk[i]   )
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
                .dest_clk ( Tx_Buf_Clk[i]   )
            );

        end
    endgenerate

//            gtfwizard_0_example_top #(
    gtf_wiz_10_raw_example_top #(
//            gtf_quad_example_top #(
        .DEST_SYNC_FF ( DEST_SYNC_FF    )
    ) u_gtf_quad (
        .refclk_in          (   gtrefclk00_in       ),
        .freerun_clk        (   drp_clk             ),

        .gtf_ch_gtftxn      (   QSFP_TX_N           ),
        .gtf_ch_gtftxp      (   QSFP_TX_P           ),
        .gtf_ch_gtfrxn      (   QSFP_RX_N           ),
        .gtf_ch_gtfrxp      (   QSFP_RX_P           ),

        .gt_recov_clk_n     (   gt_recov_clk_n      ),
        .gt_recov_clk_p     (   gt_recov_clk_p      ),
        .gt_recov_clk       (   gt_recov_clk        ),

        .loopback           (   loopback            ),
        .rxlpmen            (   rxlpmen             ),
        .rxpolarity         (   rx_polarity         ),
        .txpolarity         (   tx_polarity         ),
        .txdiffctrl         (   txdiffctrl          ),
        .txmaincursor       (   txmaincursor        ),
        .txpostcursor       (   txpostcursor        ),
        .txprecursor        (   txprecursor         ),

        .gtwiz_reset_all                 (gtwiz_reset_all),
        .gtwiz_reset_tx_pll_and_datapath (gtwiz_reset_tx_pll_and_datapath),
        .gtwiz_reset_rx_pll_and_datapath (gtwiz_reset_rx_pll_and_datapath),
        .gtwiz_reset_tx_datapath         (gtwiz_reset_tx_datapath),
        .gtwiz_reset_rx_datapath         (gtwiz_reset_rx_datapath),

        .gt_power_good      (   gtf_power_good      ),
        .gtf_qpll0_lock     (   qpll0_lock          ),

        .gtwiz_reset_tx_done        ( reset_tx_done     ),
        .gtwiz_reset_rx_done        ( reset_rx_done     ),
        .gtwiz_bufferbypass_tx_done ( bufferbypass_tx_done ),
        .gtwiz_bufferbypass_rx_done ( bufferbypass_rx_done ),

        .gtf_ch_txresetdone     ( gtf_txresetdone  ),
        .gtf_ch_txsyncdone      ( gtf_txsyncdone   ),
        .gtf_ch_rxresetdone     ( gtf_rxresetdone  ),
        .gtf_ch_rxsyncdone      ( gtf_rxsyncdone   ),

        .gtf_txusrclk2_out  (   Tx_Buf_Clk          ),
        .gtf_rxusrclk2_out  (   Rx_Buf_Clk          ),

        .tx_raw_data_0      (   tx_raw_data_int[0]  ),
        .tx_raw_data_1      (   tx_raw_data_int[1]  ),
        .tx_raw_data_2      (   tx_raw_data_int[2]  ),
        .tx_raw_data_3      (   tx_raw_data_int[3]  ),

        .rx_raw_data_0      (   rx_raw_data_int[0]  ),
        .rx_raw_data_1      (   rx_raw_data_int[1]  ),
        .rx_raw_data_2      (   rx_raw_data_int[2]  ),
        .rx_raw_data_3      (   rx_raw_data_int[3]  )
    );

/*

    logic [3 : 0]    link_down_latched_reset_in;
    wire [3 : 0]    link_status_out;
    logic clear_status_cdc;
    logic clear_status_d;
    assign Rx_Seed_Zero = 'h0;

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF ),
        .INIT_SYNC_FF   ( 0            ),
        .SRC_INPUT_REG  ( 0            )
    )
    cdc_clear_rx (
        .src_clk  (  1'h0           ),
        .src_in   ( clear_status     ),
        .dest_out ( clear_status_cdc ),
        .dest_clk ( drp_clk          )
    );

    always @(posedge drp_clk) begin
        clear_status_d <= clear_status_cdc;
        if (clear_status_d != clear_status_cdc) begin
            link_down_latched_reset_in <= 'hF;
        end else begin
            link_down_latched_reset_in <= 'h0;
        end

    end

    gtf_wiz_10_raw_example_top_quad u_gtf_quad (
        .refclk_in          (   gtrefclk00_in       ),
        .freerun_clk        (   drp_clk             ),

        .gtf_ch_gtftxn      (   QSFP_TX_N           ),
        .gtf_ch_gtftxp      (   QSFP_TX_P           ),
        .gtf_ch_gtfrxn      (   QSFP_RX_N           ),
        .gtf_ch_gtfrxp      (   QSFP_RX_P           ),

        .rxpolarity         (   rx_polarity         ),
        .Tx_Error_Inject    ( Tx_Error_Inject       ),

        .hb_gtwiz_reset_all_in       ( gtwiz_reset_all ),
        .link_down_latched_reset_in  ( link_down_latched_reset_in ),
        .link_status_out             ( link_status_out ),
        .link_down_latched_out       ( Rx_PRBS_Error  ),

        .gtf_txusrclk2_out  (   Tx_Buf_Clk          ),
        .gtf_rxusrclk2_out  (   Rx_Buf_Clk          ),

        .gt_recov_clk_n     (   gt_recov_clk_n      ),
        .gt_recov_clk_p     (   gt_recov_clk_p      ),
        .gt_recov_clk       (   gt_recov_clk        ),

        .gt_power_good      (   gtf_power_good      ),
        .gtf_qpll0_lock     (   qpll0_lock          ),

        .gtwiz_reset_tx_done        ( reset_tx_done     ),
        .gtwiz_reset_rx_done        ( reset_rx_done     ),
        .gtwiz_bufferbypass_tx_done ( bufferbypass_tx_done ),
        .gtwiz_bufferbypass_rx_done ( bufferbypass_rx_done ),

        .gtf_ch_txresetdone     ( gtf_txresetdone  ),
        .gtf_ch_txsyncdone      ( gtf_txsyncdone   ),
        .gtf_ch_rxresetdone     ( gtf_rxresetdone  ),
        .gtf_ch_rxsyncdone      ( gtf_rxsyncdone   ),

        .rx_raw_data_0      (   rx_raw_data_int[0]  ),
        .rx_raw_data_1      (   rx_raw_data_int[1]  ),
        .rx_raw_data_2      (   rx_raw_data_int[2]  ),
        .rx_raw_data_3      (   rx_raw_data_int[3]  )

    );
*/
    generate
        if ( C_GT_RATE == 2) begin

            assign tx_raw_data_int = tx_raw_data;
            assign rx_raw_data = rx_raw_data_int;

        end else begin

            // raw data bus always on 40bits

            assign tx_raw_data_int[0] = {24'd0, tx_raw_data[0]};
            assign tx_raw_data_int[1] = {24'd0, tx_raw_data[1]};
            assign tx_raw_data_int[2] = {24'd0, tx_raw_data[2]};
            assign tx_raw_data_int[3] = {24'd0, tx_raw_data[3]};

            assign rx_raw_data[0] = rx_raw_data_int[0][15:0];
            assign rx_raw_data[1] = rx_raw_data_int[1][15:0];
            assign rx_raw_data[2] = rx_raw_data_int[2][15:0];
            assign rx_raw_data[3] = rx_raw_data_int[3][15:0];

        end
    endgenerate

    genvar index;
    generate
        for (index=0; index < C_GT_NUM_LANE; index=index+1)
        begin : lane_raw_prbs
            gtf_raw_prbs_data_gen_term #(
                .DEST_SYNC_FF ( DEST_SYNC_FF    ),
                .IO_DATA_SIZE ( DATA_SIZE       )
            ) u_prbs_gen_term (
                .Rx_Usr_Clk             ( Rx_Buf_Clk[index]         ),
                .Tx_Usr_Clk             ( Tx_Buf_Clk[index]         ),

                .Tx_Error_Inject        ( Tx_Error_Inject[index]    ),

                .Rx_Enable              ( Rx_Enable                 ),
                .clear_status           ( clear_status              ),

                .Disable_PRBS_Ref       ( Disable_PRBS_Ref          ),
                .latch_status           ( latch_status              ),
                .toggle_1_sec           ( toggle_1_sec              ),

                .Rx_Data                ( rx_raw_data[index]        ),

                .Rx_PRBS_Error          ( Rx_PRBS_Error[index]      ),
                .Rx_Seed_Zero           ( Rx_Seed_Zero[index]       ),
                .Rx_Word_Cnt            ( rx_word_cnt[index]        ),
                .Rx_Err_Bit_Cnt         ( rx_err_bit_cnt[index]     ),
                .Tx_Word_Cnt            ( tx_word_cnt[index]        ),

                .Tx_Data                ( tx_raw_data[index]        ),
                .Tx_Seed_Zero           ( Tx_Seed_Zero[index]       )
            );

        end
    endgenerate

endmodule : gtf_prbs_core
`default_nettype wire
