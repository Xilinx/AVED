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

module gt_prbs_25_reg_array #(
    parameter integer C_MAJOR_VERSION,
    parameter integer C_MINOR_VERSION,
    parameter integer C_BUILD_VERSION,
    parameter integer C_CLOCK0_FREQ,
    parameter integer C_CLOCK1_FREQ,
    parameter integer C_GT_INDEX,
    parameter integer C_GT_RATE,
    parameter integer C_GT_IP_SEL,
    parameter integer DEST_SYNC_FF,
    parameter integer C_BLOCK_ID
) (
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         watchdog_alarm_in,

    output wire [11 : 0]   loopback     ,
    output wire [3 : 0]    rxlpmen      ,
    output wire [3 : 0]    rxpolarity,
    output wire [3 : 0]    txpolarity   ,
    output wire [19 : 0]   txdiffctrl   ,
    output wire [27 : 0]   txmaincursor ,
    output wire [19 : 0]   txpostcursor ,
    output wire [19 : 0]   txprecursor  ,

    output logic           gt_reset     ,
    output logic           gt_tx_reset,
    output logic           gt_rx_reset,
    input  wire [3:0]      gt_power_good,
    input  wire            gt_reset_tx_done,
    input  wire            gt_reset_rx_done,
    input  wire            pll_lock,

    output logic           Tx_Enable,
    output logic           Rx_Enable,
    output logic           Clear_Rx_PRBS_Error,
    output logic           Clear_Rx_Seed_Zero,
    output logic           Clear_Tx_Seed_Zero,
    output logic           Disable_PRBS_Ref,
    output logic [3 :0]    Tx_Error_Inject,

    input  wire [3 : 0]    Rx_PRBS_Error,
    input  wire [3 : 0]    Rx_Seed_Zero,
    input  wire [3 : 0]    Tx_Seed_Zero,

    input  wire [47 : 0]   rx_word_cnt[3:0],
    input  wire [47 : 0]   rx_err_word_cnt[3:0],

    input  wire [47 : 0]   tx_word_cnt[3:0],

    input  wire         cs,
    input  wire         we,
    input  wire  [11:0] addr,
    input  wire  [31:0] wdata,
    output logic [31:0] rdata           = 'h0,
    output logic        cmd_cmplt       = 'h0
);


    wire [4:0]  addr_4_0    = addr[4:0];
    wire        addr_5      = addr[5];
    wire        addr_4      = addr[4];
    wire        addr_5_4    = addr[5:4];
    wire        we_bi           = (addr_5_4 == 2'b00) ? cs & we : 'b0;
    wire        we_ctrl_status  = (addr_5_4 == 2'b01) ? cs & we : 'b0;
    wire [31:0] rdata_bi;
    wire [31:0] rdata_ctrl_status;

    build_info_v4_0 #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
        .C_BLOCK_ID         ( C_BLOCK_ID            )
    ) u_build_info (
        .Clk        ( ap_clk                        ),
        .Rst        ( ap_rst                        ),
        .Info_1     ( 16'b0                         ),  // Info 1 reserved for future use
        .Info_2     ( {C_GT_INDEX[7:0], 4'b0, C_GT_RATE[1:0], C_GT_IP_SEL[1:0]}       ),
        .Info_3     ( 16'h0                         ),
        .Info_4     ( 16'h0                         ),
        .Info_5     ( 16'h0                         ),
        .Info_6     ( 16'h0                         ),
        .Info_7     ( C_CLOCK0_FREQ[15:0]           ),
        .Info_8     ( C_CLOCK1_FREQ[15:0]           ),

        .We         ( we_bi                         ),
        .Addr       ( addr[2:0]                     ),
        .Data_In    ( wdata                         ),
        .Data_Out   ( rdata_bi                      )
    );

    common_ctrl_status #(
        .C_CLOCK_FREQ               ( C_CLOCK0_FREQ ),
        .DEST_SYNC_FF               ( DEST_SYNC_FF  ),
        .C_CLK_TROTTLE_DETECT_EN    ( 0             ),
        .C_WATCHDOG_ENABLE          ( 0             ),
        .C_EXT_TOGGLE_1_SEC         ( 0             )  // 1: use toggle_1_sec input, 0: generate internally
    ) u_common_ctrl_status (
        .ap_clk         ( ap_clk            ),
        .ap_clk_cont    ( ap_clk_cont       ),
        .ap_rst         ( ap_rst            ),

        .ap_clk_2       ( 1'b0              ),
        .ap_clk_2_cont  ( 1'b0              ),
        .ap_rst_2       ( 1'b0              ),

        .toggle_1sec    ( 1'b0              ),
        .rst_watchdog   ( 1'b0              ),
        .watchdog_alarm (                   ),

        .We             ( we_ctrl_status    ),
        .Addr           ( addr[2:0]         ),
        .Data_In        ( wdata             ),
        .User_Status_1  ( 32'b0             ),
        .Data_Out       ( rdata_ctrl_status )
    );


    // VUp: from XXV example design trans_debug
    // Versal from gt_quad_base example design (gt_bridge ip)
    // GTF: from wizard rules_output.vh files

    logic [2:0]  gt_loopback     [3:0]  = '{default:'0};
    logic [3:0]  gt_rxlpmen             = 'h0;
    logic [3:0]  gt_rxpolarity          = 'h0;
    logic [3:0]  gt_txpolarity          = 'h0;
    logic [4:0]  gt_txpostcursor [3:0]  = '{default:'0};
    logic [4:0]  gt_txprecursor  [3:0]  = '{default:'0};
    logic [4:0]  gt_txdiffctrl   [3:0]  = (C_GT_IP_SEL == 2) ? '{default: 5'b01100} :'{default:'0};
    logic [6:0]  gt_txmaincursor [3:0]  = (C_GT_IP_SEL == 2) ? '{default: 7'h50} : ((C_GT_IP_SEL == 1) ? '{default: 7'h0} : '{default: 7'h50});

    wire [2:0]  lane_loopback;
    wire        lane_rxlpmen;
    wire        lane_rxpolarity;
    wire        lane_txpolarity;
    wire [4:0]  lane_txpostcursor;
    wire [4:0]  lane_txprecursor ;
    wire [4:0]  lane_txdiffctrl  ;
    wire [6:0]  lane_txmaincursor;

    assign lane_loopback     = wdata[30:28];
    assign lane_rxlpmen      = wdata[24];
    assign lane_rxpolarity   = wdata[23];
    assign lane_txpolarity   = wdata[22];
    assign lane_txpostcursor = wdata[21:17];
    assign lane_txmaincursor = wdata[16:10];
    assign lane_txprecursor  = wdata[9:5];
    assign lane_txdiffctrl   = wdata[4:0];

    assign loopback = {gt_loopback[3],gt_loopback[2],gt_loopback[1],gt_loopback[0] };
    assign rxlpmen = {gt_rxlpmen[3],gt_rxlpmen[2],gt_rxlpmen[1],gt_rxlpmen[0] };
    assign rxpolarity = {gt_rxpolarity[3],gt_rxpolarity[2],gt_rxpolarity[1],gt_rxpolarity[0] };
    assign txpolarity = {gt_txpolarity[3],gt_txpolarity[2],gt_txpolarity[1],gt_txpolarity[0] };
    assign txdiffctrl = {gt_txdiffctrl[3],gt_txdiffctrl[2],gt_txdiffctrl[1],gt_txdiffctrl[0] };
    assign txpostcursor = {gt_txpostcursor[3],gt_txpostcursor[2],gt_txpostcursor[1],gt_txpostcursor[0] };
    assign txprecursor = {gt_txprecursor[3],gt_txprecursor[2],gt_txprecursor[1],gt_txprecursor[0] };
    assign txmaincursor = {gt_txmaincursor[3],gt_txmaincursor[2],gt_txmaincursor[1],gt_txmaincursor[0] };

    wire [31:0]  cfg_lane [3:0];
    assign cfg_lane[0] = {1'b0, gt_loopback[0], 3'b0, gt_rxlpmen[0], gt_rxpolarity[0], gt_txpolarity[0], gt_txpostcursor[0], gt_txmaincursor[0], gt_txprecursor[0], gt_txdiffctrl[0]} ;
    assign cfg_lane[1] = {1'b0, gt_loopback[1], 3'b0, gt_rxlpmen[1], gt_rxpolarity[1], gt_txpolarity[1], gt_txpostcursor[1], gt_txmaincursor[1], gt_txprecursor[1], gt_txdiffctrl[1]} ;
    assign cfg_lane[2] = {1'b0, gt_loopback[2], 3'b0, gt_rxlpmen[2], gt_rxpolarity[2], gt_txpolarity[2], gt_txpostcursor[2], gt_txmaincursor[2], gt_txprecursor[2], gt_txdiffctrl[2]} ;
    assign cfg_lane[3] = {1'b0, gt_loopback[3], 3'b0, gt_rxlpmen[3], gt_rxpolarity[3], gt_txpolarity[3], gt_txpostcursor[3], gt_txmaincursor[3], gt_txprecursor[3], gt_txdiffctrl[3]} ;

    wire [3:0]    Rx_PRBS_Error_cdc;
    wire [3:0]    Rx_Seed_Zero_cdc;
    wire [3:0]    Tx_Seed_Zero_cdc;
    wire [3:0]    gt_power_good_cdc;
    wire          gt_reset_tx_done_cdc;
    wire          gt_reset_rx_done_cdc;
    wire          pll_lock_cdc;

    wire [47 : 0]   rx_word_cnt_cdc[3:0];
    wire [47 : 0]   rx_err_word_cnt_cdc[3:0];
    wire [47 : 0]   tx_word_cnt_cdc[3:0];

    wire [18:0]  status_cdc_in, status_cdc_out;
    assign status_cdc_in = {pll_lock, gt_reset_rx_done, gt_reset_tx_done, gt_power_good, Tx_Seed_Zero, Rx_Seed_Zero, Rx_PRBS_Error};

    assign {   pll_lock_cdc,
                gt_reset_rx_done_cdc,
                gt_reset_tx_done_cdc,
                gt_power_good_cdc,
                Tx_Seed_Zero_cdc,
                Rx_Seed_Zero_cdc,
                Rx_PRBS_Error_cdc
    } = status_cdc_out;

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF         ),
        .INIT_SYNC_FF   ( 0                    ),
        .SRC_INPUT_REG  ( 0                    ),
        .WIDTH          ( $size(status_cdc_in) )
    )
    xpm_cdc_status (
        .src_clk  (  1'h0            ),
        .src_in   ( status_cdc_in    ),
        .dest_out ( status_cdc_out   ),
        .dest_clk ( ap_clk           )
    );

    // there is a false path on the watchdog alarm input
    wire   watchdog_alarm_cdc;
    (*dont_touch ="true"*) logic watchdog_alarm_d = '0;

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .SIM_ASSERT_CHK ( 0             )
    )
    xpm_cdc_watchdog (
        .src_clk  ( 1'h0                ),
        .src_in   ( watchdog_alarm_in   ),
        .dest_out ( watchdog_alarm_cdc  ),
        .dest_clk ( ap_clk              )
    );

    for (genvar ln_idx=0; ln_idx < 4; ln_idx=ln_idx+1)
    begin: rx_word_cdc
        xpm_cdc_array_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF               ),
            .INIT_SYNC_FF   ( 0                          ),
            .SRC_INPUT_REG  ( 0                          ),
            .WIDTH          ( $size(tx_word_cnt[ln_idx]) )
        )
        cdc_tx_wd_cnt (
            .src_clk  (  1'h0                   ),
            .src_in   ( tx_word_cnt[ln_idx]     ),
            .dest_out ( tx_word_cnt_cdc[ln_idx] ),
            .dest_clk ( ap_clk                  )
        );

        xpm_cdc_array_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF               ),
            .INIT_SYNC_FF   ( 0                          ),
            .SRC_INPUT_REG  ( 0                          ),
            .WIDTH          ( $size(rx_word_cnt[ln_idx]) )
        )
        cdc_rx_wd_cnt (
            .src_clk  (  1'h0                   ),
            .src_in   ( rx_word_cnt[ln_idx]     ),
            .dest_out ( rx_word_cnt_cdc[ln_idx] ),
            .dest_clk ( ap_clk                  )
        );

        xpm_cdc_array_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF                   ),
            .INIT_SYNC_FF   ( 0                              ),
            .SRC_INPUT_REG  ( 0                              ),
            .WIDTH          ( $size(rx_err_word_cnt[ln_idx]) )
        )
        cdc_rx_err_wd_cnt (
            .src_clk  (  1'h0                       ),
            .src_in   ( rx_err_word_cnt[ln_idx]     ),
            .dest_out ( rx_err_word_cnt_cdc[ln_idx] ),
            .dest_clk ( ap_clk                      )
        );


    end

    logic           gt_power_good_all;


    //########################################
    //### registers
    //########################################
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin
            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            gt_reset        <= 'h0;
            gt_tx_reset     <= 'h0;
            gt_rx_reset     <= 'h0;
            Tx_Enable       <= 'h0;
            Rx_Enable       <= 'h0;
            Clear_Rx_PRBS_Error <= 'h0;
            Clear_Rx_Seed_Zero  <= 'h0;
            Clear_Tx_Seed_Zero  <= 'h0;
            Disable_PRBS_Ref    <= 'h0;
            Tx_Error_Inject     <= 'h0;

        end else begin

            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            gt_reset    <= 'h0;
            gt_tx_reset <= 'h0;
            gt_rx_reset <= 'h0;

            watchdog_alarm_d <= watchdog_alarm_cdc;

            if (watchdog_alarm_d != watchdog_alarm_cdc) begin
                Tx_Enable <= 'h0;
                Rx_Enable <= 'h0;
            end

            gt_power_good_all <= &gt_power_good_cdc;

            if (cs) begin

                case (addr_5)
                    'h0 : begin
                        if (addr_4) begin
                            // common ctrl and status
                            rdata       <= rdata_ctrl_status;
                        end else begin
                            // build info
                            rdata       <= rdata_bi;
                        end
                        cmd_cmplt   <= 'h1;
                    end
                    default : begin
                        case (addr_4_0)
                            // default : begin
                            5'b00000 : begin // 0x0

                                if (we) begin
                                    gt_reset <= wdata[0];
                                    gt_tx_reset <= wdata[2];
                                    gt_rx_reset <= wdata[3];
                                    Tx_Enable <= wdata[4];
                                    Rx_Enable <= wdata[5];

                                    if (wdata[8]) begin
                                        Clear_Rx_PRBS_Error <= ~ Clear_Rx_PRBS_Error;
                                    end
                                    if (wdata[9]) begin
                                        Clear_Rx_Seed_Zero <= ~ Clear_Rx_Seed_Zero;
                                    end
                                    if (wdata[10]) begin
                                        Clear_Tx_Seed_Zero <= ~ Clear_Tx_Seed_Zero;
                                    end

                                    if (wdata[24]) begin
                                        Tx_Error_Inject[0] <= ~ Tx_Error_Inject[0];
                                    end
                                    if (wdata[25]) begin
                                        Tx_Error_Inject[1] <= ~ Tx_Error_Inject[1];
                                    end
                                    if (wdata[26]) begin
                                        Tx_Error_Inject[2] <= ~ Tx_Error_Inject[2];
                                    end
                                    if (wdata[27]) begin
                                        Tx_Error_Inject[3] <= ~ Tx_Error_Inject[3];
                                    end

                                    Disable_PRBS_Ref <= wdata[28];

                                end

                                rdata[1]        <= gt_power_good_all;
                                rdata[2]        <= gt_reset_tx_done_cdc;
                                rdata[3]        <= gt_reset_rx_done_cdc;

                                rdata[4]        <= Tx_Enable;
                                rdata[5]        <= Rx_Enable;

                                rdata[6]        <= pll_lock_cdc;

                                rdata[15:12]    <= Rx_PRBS_Error_cdc;
                                rdata[19:16]    <= Rx_Seed_Zero_cdc;
                                rdata[23:20]    <= Tx_Seed_Zero_cdc;

                                rdata[28]       <= Disable_PRBS_Ref;

                                cmd_cmplt   <= 'h1;
                            end
                            5'b00001 : begin
                                if (we) begin
                                    gt_loopback[0]      <= lane_loopback;
                                    gt_rxlpmen[0]       <= lane_rxlpmen;
                                    gt_rxpolarity[0]    <= lane_rxpolarity;
                                    gt_txpolarity[0]    <= lane_txpolarity;
                                    gt_txpostcursor[0]  <= lane_txpostcursor;
                                    gt_txmaincursor[0]  <= lane_txmaincursor;
                                    gt_txprecursor[0]   <= lane_txprecursor ;
                                    gt_txdiffctrl[0]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b00010 : begin
                                if (we) begin
                                    gt_loopback[1]      <= lane_loopback;
                                    gt_rxlpmen[1]       <= lane_rxlpmen;
                                    gt_rxpolarity[1]    <= lane_rxpolarity;
                                    gt_txpolarity[1]    <= lane_txpolarity;
                                    gt_txpostcursor[1]  <= lane_txpostcursor;
                                    gt_txmaincursor[1]  <= lane_txmaincursor;
                                    gt_txprecursor[1]   <= lane_txprecursor ;
                                    gt_txdiffctrl[1]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[1];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b00011 : begin
                                if (we) begin
                                    gt_loopback[2]      <= lane_loopback;
                                    gt_rxlpmen[2]       <= lane_rxlpmen;
                                    gt_rxpolarity[2]    <= lane_rxpolarity;
                                    gt_txpolarity[2]    <= lane_txpolarity;
                                    gt_txpostcursor[2]  <= lane_txpostcursor;
                                    gt_txmaincursor[2]  <= lane_txmaincursor;
                                    gt_txprecursor[2]   <= lane_txprecursor ;
                                    gt_txdiffctrl[2]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[2];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b00100 : begin
                                if (we) begin
                                    gt_loopback[3]      <= lane_loopback;
                                    gt_rxlpmen[3]       <= lane_rxlpmen;
                                    gt_rxpolarity[3]    <= lane_rxpolarity;
                                    gt_txpolarity[3]    <= lane_txpolarity;
                                    gt_txpostcursor[3]  <= lane_txpostcursor;
                                    gt_txmaincursor[3]  <= lane_txmaincursor;
                                    gt_txprecursor[3]   <= lane_txprecursor ;
                                    gt_txdiffctrl[3]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[3];
                                cmd_cmplt   <= 'h1;
                            end

                            // 5'b00101 => 5'b01111 not used
                            5'b01000 : begin
                                rdata       <= tx_word_cnt_cdc[0][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01001 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[0][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01010 : begin
                                rdata       <= tx_word_cnt_cdc[1][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01011 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[1][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01100 : begin
                                rdata       <= tx_word_cnt_cdc[2][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01101 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[2][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01110 : begin
                                rdata       <= tx_word_cnt_cdc[3][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b01111 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[3][47:32];
                                cmd_cmplt   <= 'h1;
                            end


                            5'b10000 : begin
                                rdata       <= rx_word_cnt_cdc[0][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b10001 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[0][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b10010 : begin
                                rdata       <= rx_err_word_cnt_cdc[0][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b10011 : begin
                                rdata[15:0] <= rx_err_word_cnt_cdc[0][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            5'b10100 : begin
                                rdata       <= rx_word_cnt_cdc[1][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b10101 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[1][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b10110 : begin
                                rdata       <= rx_err_word_cnt_cdc[1][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b10111 : begin
                                rdata[15:0] <= rx_err_word_cnt_cdc[1][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            5'b11000 : begin
                                rdata       <= rx_word_cnt_cdc[2][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b11001 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[2][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b11010 : begin
                                rdata       <= rx_err_word_cnt_cdc[2][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b11011 : begin
                                rdata[15:0] <= rx_err_word_cnt_cdc[2][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            5'b11100 : begin
                                rdata       <= rx_word_cnt_cdc[3][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b11101 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[3][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b11110 : begin
                                rdata       <= rx_err_word_cnt_cdc[3][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            5'b11111 : begin
                                rdata[15:0] <= rx_err_word_cnt_cdc[3][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            default : begin
                                rdata       <= 'h0;
                                cmd_cmplt   <= 'h1;
                            end
                        endcase
                    end
                endcase
            end

        end
    end

endmodule : gt_prbs_25_reg_array
`default_nettype wire
