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

// default_nettype of none prevents implicit wire declaration.
`default_nettype none

module gtyp_prbs_top #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 2,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer C_BLOCK_ID            = 3,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_GT_INDEX            = 0,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_GT_NUM_LANE         = 4,
    parameter integer C_GT_IP_SEL           = 0,
    parameter integer C_GT_RATE             = 0,
    parameter integer C_QUAD_EN             = 0,
    parameter integer C_GEN_DRP_CLK         = 0,
    parameter integer DISABLE_SIM_ASSERT    = 0
) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,

    input  wire         drp_clk_in,
    input  wire         drp_clk_gen,

    input  wire         toggle_1_sec,
    input  wire         watchdog_alarm,

    input  wire         QSFP_CK_N,
    input  wire         QSFP_CK_P,
    input  wire [3 : 0] QSFP_RX_N,
    input  wire [3 : 0] QSFP_RX_P,
    output wire [3 : 0] QSFP_TX_N,
    output wire [3 : 0] QSFP_TX_P,

    input  wire         gt_refclk_in,
    output wire         gt_refclk_out,

    output wire [0 : 0]  gt_recov_clk_n,
    output wire [0 : 0]  gt_recov_clk_p,
    output wire          gt_recov_clk,

    input  wire         cs,
    input  wire         we,
    input  wire [21:0]  addr,
    input  wire [31:0]  wdata,
    output wire [31:0]  rdata,
    output wire         cmd_cmplt

);

    wire [11 : 0]   loopback;
    wire [3 : 0]    rxlpmen;
    wire [3 : 0]    rxpolarity;
    wire [3 : 0]    txpolarity;
    wire [19 : 0]   txdiffctrl;
    wire [27 : 0]   txmaincursor;
    wire [19 : 0]   txpostcursor;
    wire [19 : 0]   txprecursor;

    wire  bridge_reset_all         ;
    wire  reset_tx_pll_and_datapath;
    wire  reset_rx_pll_and_datapath;
    wire  reset_tx_datapath        ;
    wire  reset_rx_datapath        ;

    wire  gt_power_good;
    wire  gt_power_good_f;
    wire  gt_power_good_r;
    wire  lcpll_lock;
    wire  lcpll_lock_f;
    wire  lcpll_lock_r;

    wire  bridge_reset_tx_done;
    wire  bridge_reset_tx_done_f;
    wire  bridge_reset_tx_done_r;

    wire  bridge_reset_rx_done;
    wire  bridge_reset_rx_done_f;
    wire  bridge_reset_rx_done_r;

    wire            Tx_Enable;
    wire            Rx_Enable;
    wire            clear_status;
    logic           clear_status_d;
    wire            latch_status;
    logic           latch_status_d;
    wire            Disable_PRBS_Ref;
    wire [3 : 0]    Tx_Error_Inject;
    wire [3 : 0]    Rx_PRBS_Error;
    wire [3 : 0]    Rx_Seed_Zero;
    wire [3 : 0]    Tx_Seed_Zero;

    wire [47 : 0]   rx_word_cnt[3:0];
    wire [47 : 0]   rx_err_bit_cnt[3:0];
    wire [47 : 0]   tx_word_cnt[3:0];

    logic [30 : 0]  sec_cnt = 'h0;
    logic [31 : 0]  stopwatch = 'h0;
    logic [31 : 0]  stopwatch_latch = 'h0;

    wire            stat_toggle_1_sec;
    logic           stat_toggle_1_sec_d;

    ///////////////////////////////////////////////////////////////////////
    // Timestamp
    ///////////////////////////////////////////////////////////////////////

    // status can't be latched when 1sec toggles. because of all cdc, it's impossible to predict if status and toggle will be aligned
    // this is "protected" by SW: it computes the rate based on sec +/- 1
    assign stat_toggle_1_sec = toggle_1_sec;
    always_ff @(posedge ap_clk) begin
        stat_toggle_1_sec_d <= stat_toggle_1_sec;

        if (stat_toggle_1_sec_d != stat_toggle_1_sec) begin
            sec_cnt <= sec_cnt + 1;
            stopwatch <= stopwatch + 1;
        end

        clear_status_d <= clear_status;
        if (clear_status_d != clear_status) begin
            stopwatch <= '0;
        end

        latch_status_d <= latch_status;
        if (latch_status_d != latch_status) begin
            stopwatch_latch <= stopwatch;
        end

    end

    wire drp_clk;
    generate
        if (C_GEN_DRP_CLK == 1) begin : gen_drp_clk

            (* dont_touch = "true" *) wire clk_div2;

            (* dont_touch = "true" *) BUFGCE_DIV #(
                .BUFGCE_DIVIDE(2)
            ) clk_div2_buf (
                .I      (drp_clk_gen ),
                .CE     (1'b1        ),
                .CLR    (1'b0        ),
                .O      (clk_div2    )
            );

            assign drp_clk = clk_div2;

        end else begin : common_drp_clk
            assign drp_clk = drp_clk_in;
        end
    endgenerate

    ///////////////////////////////////////////////////////////////////////
    // Register Array
    ///////////////////////////////////////////////////////////////////////

    gtyp_prbs_reg_array #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION   ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION   ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION   ),
        .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ     ),
        .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ     ),
        .C_GT_INDEX         ( C_GT_INDEX        ),
        .C_GT_IP_SEL        ( C_GT_IP_SEL       ),
        .C_GT_RATE          ( C_GT_RATE         ),
        .C_QUAD_EN          ( C_QUAD_EN         ),
        .C_GT_NUM_LANE      ( C_GT_NUM_LANE     ),
        .DEST_SYNC_FF       ( DEST_SYNC_FF      ),
        .C_BLOCK_ID         ( C_BLOCK_ID        )
    ) u_reg_array (
        .ap_clk                 ( ap_clk                ),
        .ap_clk_cont            ( ap_clk_cont           ),
        .ap_rst                 ( ap_rst                ),

        .watchdog_alarm_in      ( watchdog_alarm        ),

        .loopback               ( loopback              ),
        .rxlpmen                ( rxlpmen               ),
        .rxpolarity             ( rxpolarity            ),
        .txpolarity             ( txpolarity            ),
        .txdiffctrl             ( txdiffctrl            ),
        .txmaincursor           ( txmaincursor          ),
        .txpostcursor           ( txpostcursor          ),
        .txprecursor            ( txprecursor           ),

        .bridge_reset_all           (bridge_reset_all),
        .reset_tx_pll_and_datapath  (reset_tx_pll_and_datapath),
        .reset_rx_pll_and_datapath  (reset_rx_pll_and_datapath),
        .reset_tx_datapath          (reset_tx_datapath),
        .reset_rx_datapath          (reset_rx_datapath),

        .gt_power_good      ( gt_power_good ),
        .gt_power_good_f    ( gt_power_good_f ),
        .gt_power_good_r    ( gt_power_good_r ),
        .lcpll_lock         ( lcpll_lock ),
        .lcpll_lock_f       ( lcpll_lock_f ),
        .lcpll_lock_r       ( lcpll_lock_r ),

        .bridge_reset_tx_done   ( bridge_reset_tx_done ),
        .bridge_reset_tx_done_f ( bridge_reset_tx_done_f ),
        .bridge_reset_tx_done_r ( bridge_reset_tx_done_r ),
        .bridge_reset_rx_done   ( bridge_reset_rx_done ),
        .bridge_reset_rx_done_f ( bridge_reset_rx_done_f ),
        .bridge_reset_rx_done_r ( bridge_reset_rx_done_r ),

        .Tx_Enable              ( Tx_Enable             ),
        .Rx_Enable              ( Rx_Enable             ),
        .clear_status           ( clear_status          ),
        .latch_status           ( latch_status          ),
        .Disable_PRBS_Ref       ( Disable_PRBS_Ref      ),
        .Tx_Error_Inject        ( Tx_Error_Inject       ),
        .Rx_PRBS_Error          ( Rx_PRBS_Error         ),
        .Rx_Seed_Zero           ( Rx_Seed_Zero          ),
        .Tx_Seed_Zero           ( Tx_Seed_Zero          ),

        .sec_cnt                ( sec_cnt               ),
        .stopwatch              ( stopwatch             ),
        .rx_word_cnt            ( rx_word_cnt           ),
        .rx_err_bit_cnt         ( rx_err_bit_cnt        ),
        .tx_word_cnt            ( tx_word_cnt           ),

        .toggle_1_sec           ( toggle_1_sec          ),

        .cs                     ( cs                    ),
        .we                     ( we                    ),
        .addr                   ( addr                  ),
        .wdata                  ( wdata                 ),
        .rdata                  ( rdata                 ),
        .cmd_cmplt              ( cmd_cmplt             )
    );

    gtyp_prbs_core #(
        .C_GT_RATE      ( C_GT_RATE     ),
        .C_GT_NUM_LANE  ( C_GT_NUM_LANE ),
        .DEST_SYNC_FF   ( DEST_SYNC_FF  )

    ) u_prbs_core  (

        .ap_clk                 ( ap_clk                ),
        .ap_rst                 ( ap_rst                ),
        .drp_clk                ( drp_clk               ),

        .QSFP_CK_N              ( QSFP_CK_N             ),
        .QSFP_CK_P              ( QSFP_CK_P             ),
        .QSFP_RX_N              ( QSFP_RX_N             ),
        .QSFP_RX_P              ( QSFP_RX_P             ),
        .QSFP_TX_N              ( QSFP_TX_N             ),
        .QSFP_TX_P              ( QSFP_TX_P             ),

        .gt_refclk_in           ( gt_refclk_in          ),
        .gt_refclk_out          ( gt_refclk_out         ),

        .loopback               ( loopback              ),
        .rxlpmen                ( rxlpmen               ),
        .rxpolarity             ( rxpolarity            ),
        .txpolarity             ( txpolarity            ),
        .txdiffctrl             ( txdiffctrl            ),
        .txmaincursor           ( txmaincursor          ),
        .txpostcursor           ( txpostcursor          ),
        .txprecursor            ( txprecursor           ),

        .bridge_reset_all           (bridge_reset_all),
        .reset_tx_pll_and_datapath  (reset_tx_pll_and_datapath),
        .reset_rx_pll_and_datapath  (reset_rx_pll_and_datapath),
        .reset_tx_datapath          (reset_tx_datapath),
        .reset_rx_datapath          (reset_rx_datapath),

        .gt_power_good      ( gt_power_good ),
        .gt_power_good_f    ( gt_power_good_f ),
        .gt_power_good_r    ( gt_power_good_r ),
        .lcpll_lock         ( lcpll_lock ),
        .lcpll_lock_f       ( lcpll_lock_f ),
        .lcpll_lock_r       ( lcpll_lock_r ),

        .bridge_reset_tx_done   ( bridge_reset_tx_done ),
        .bridge_reset_tx_done_f ( bridge_reset_tx_done_f ),
        .bridge_reset_tx_done_r ( bridge_reset_tx_done_r ),
        .bridge_reset_rx_done   ( bridge_reset_rx_done ),
        .bridge_reset_rx_done_f ( bridge_reset_rx_done_f ),
        .bridge_reset_rx_done_r ( bridge_reset_rx_done_r ),

        .Tx_Enable              ( Tx_Enable             ),
        .Rx_Enable              ( Rx_Enable             ),
        .clear_status           ( clear_status          ),
        .Disable_PRBS_Ref       ( Disable_PRBS_Ref      ),
        .Tx_Error_Inject        ( Tx_Error_Inject       ),
        .Rx_PRBS_Error          ( Rx_PRBS_Error         ),
        .Rx_Seed_Zero           ( Rx_Seed_Zero          ),
        .Tx_Seed_Zero           ( Tx_Seed_Zero          ),
        .rx_word_cnt            ( rx_word_cnt           ),
        .rx_err_bit_cnt         ( rx_err_bit_cnt        ),
        .tx_word_cnt            ( tx_word_cnt           ),

        .latch_status           ( latch_status          ),
        .toggle_1_sec           ( stat_toggle_1_sec     )

    );

endmodule : gtyp_prbs_top
`default_nettype wire