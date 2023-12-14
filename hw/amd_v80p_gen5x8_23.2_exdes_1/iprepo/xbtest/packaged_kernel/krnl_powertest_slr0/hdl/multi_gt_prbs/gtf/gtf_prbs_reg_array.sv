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

module gtf_prbs_reg_array #(
    parameter integer C_MAJOR_VERSION,
    parameter integer C_MINOR_VERSION,
    parameter integer C_BUILD_VERSION,
    parameter integer C_CLOCK0_FREQ,
    parameter integer C_CLOCK1_FREQ,
    parameter integer C_GT_INDEX,
    parameter integer C_GT_RATE,
    parameter integer C_GT_IP_SEL,
    parameter integer C_QUAD_EN,
    parameter integer C_GT_NUM_LANE,
    parameter integer DEST_SYNC_FF,
    parameter integer C_BLOCK_ID
) (
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         watchdog_alarm_in,

    output wire [11 : 0]   loopback,
    output wire [3 : 0]    rxlpmen,
    output wire [3 : 0]    rxpolarity,
    output wire [3 : 0]    txpolarity,
    output wire [19 : 0]   txdiffctrl,
    output wire [27 : 0]   txmaincursor,
    output wire [19 : 0]   txpostcursor,
    output wire [19 : 0]   txprecursor,

    output logic [3 : 0]    gtwiz_reset_all,
    output logic [3 : 0]    gtwiz_reset_tx_pll_and_datapath,
    output logic [3 : 0]    gtwiz_reset_rx_pll_and_datapath,
    output logic [3 : 0]    gtwiz_reset_tx_datapath,
    output logic [3 : 0]    gtwiz_reset_rx_datapath,

    input  wire [3 : 0]     gt_power_good,
    input  wire [3 : 0]     gt_power_good_f,
    input  wire [3 : 0]     gt_power_good_r,
    input  wire             gtf_qpll0_lock,
    input  wire             gtf_qpll0_lock_f,
    input  wire             gtf_qpll0_lock_r,

    input  wire [3 : 0]     gt_reset_tx_done,
    input  wire [3 : 0]     gt_reset_tx_done_f,
    input  wire [3 : 0]     gt_reset_tx_done_r,
    input  wire [3 : 0]     gt_bufferbypass_tx_done,
    input  wire [3 : 0]     gt_bufferbypass_tx_done_f,
    input  wire [3 : 0]     gt_bufferbypass_tx_done_r,
    input  wire [3 : 0]     gtf_ch_txresetdone,
    input  wire [3 : 0]     gtf_ch_txresetdone_f,
    input  wire [3 : 0]     gtf_ch_txresetdone_r,
    input  wire [3 : 0]     gtf_ch_txsyncdone,
    input  wire [3 : 0]     gtf_ch_txsyncdone_f,
    input  wire [3 : 0]     gtf_ch_txsyncdone_r,

    input  wire [3 : 0]     gt_reset_rx_done,
    input  wire [3 : 0]     gt_reset_rx_done_f,
    input  wire [3 : 0]     gt_reset_rx_done_r,
    input  wire [3 : 0]     gt_bufferbypass_rx_done,
    input  wire [3 : 0]     gt_bufferbypass_rx_done_f,
    input  wire [3 : 0]     gt_bufferbypass_rx_done_r,
    input  wire [3 : 0]     gtf_ch_rxresetdone,
    input  wire [3 : 0]     gtf_ch_rxresetdone_f,
    input  wire [3 : 0]     gtf_ch_rxresetdone_r,
    input  wire [3 : 0]     gtf_ch_rxsyncdone,
    input  wire [3 : 0]     gtf_ch_rxsyncdone_f,
    input  wire [3 : 0]     gtf_ch_rxsyncdone_r,

    output logic           Tx_Enable,
    output logic           Rx_Enable,
    output logic           clear_status,
    output logic           latch_status,
    output logic           Disable_PRBS_Ref,
    output logic [3 :0]    Tx_Error_Inject,

    input  wire [3 : 0]    Rx_PRBS_Error,
    input  wire [3 : 0]    Rx_Seed_Zero,
    input  wire [3 : 0]    Tx_Seed_Zero,

    input  wire [31 : 0]   sec_cnt,
    input  wire [47 : 0]   rx_word_cnt[3:0],
    input  wire [47 : 0]   rx_err_bit_cnt[3:0],
    input  wire [47 : 0]   tx_word_cnt[3:0],

    input  wire            stat_toggle_1_sec,

    input  wire         cs,
    input  wire         we,
    input  wire  [21:0] addr,
    input  wire  [31:0] wdata,
    output logic [31:0] rdata           = 'h0,
    output logic        cmd_cmplt       = 'h0
);


    wire [5:0]  addr_5_0    = addr[5:0]; // was addr 4..0
    wire        addr_6      = addr[6]; // was addr 5
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
        .Info_2     ( {C_GT_INDEX[7:0], 2'b0, C_QUAD_EN[0:0], C_GT_RATE[2:0], C_GT_IP_SEL[1:0]}       ),
        .Info_3     ( 16'h0                         ),
        .Info_4     ( {8'h0, C_GT_NUM_LANE[7:0]}    ),
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


    // Configuration

    // VUp: from XXV example design trans_debug
    // Versal from gt_quad_base example design (gt_bridge ip)
    // GTF: from wizard rules_output.vh files

    logic [2:0]  gt_loopback     [3:0]  = '{default:'0};
    logic [3:0]  gt_rxlpmen             = 'h0;
    logic [3:0]  gt_rxpolarity          = 'h0;
    logic [3:0]  gt_txpolarity          = 'h0;
    logic [4:0]  gt_txpostcursor [3:0]  = '{default:'0};
    logic [4:0]  gt_txprecursor  [3:0]  = '{default:'0};
    logic [4:0]  gt_txdiffctrl   [3:0]  = '{default: 5'b01100};
    logic [6:0]  gt_txmaincursor [3:0]  = '{default: 7'h5A}; // 'd90 from rules

    wire [2:0]  lane_loopback;
    wire        lane_rxlpmen;
    wire        lane_rxpolarity;
    wire        lane_txpolarity;
    wire [4:0]  lane_txpostcursor;
    wire [4:0]  lane_txprecursor ;
    wire [4:0]  lane_txdiffctrl  ;
    wire [6:0]  lane_txmaincursor;

    assign lane_loopback     = wdata[31:29];
    assign lane_rxlpmen      = wdata[26];
    assign lane_rxpolarity   = wdata[25];
    assign lane_txpolarity   = wdata[24];
    assign lane_txpostcursor = wdata[22:18];
    assign lane_txmaincursor = wdata[16:11];
    assign lane_txprecursor  = wdata[9:5];
    assign lane_txdiffctrl   = wdata[4:0];

    assign loopback     = {gt_loopback[3],      gt_loopback[2],     gt_loopback[1],     gt_loopback[0] };
    assign rxlpmen      = {gt_rxlpmen[3],       gt_rxlpmen[2],      gt_rxlpmen[1],      gt_rxlpmen[0] };
    assign rxpolarity   = {gt_rxpolarity[3],    gt_rxpolarity[2],   gt_rxpolarity[1],   gt_rxpolarity[0] };
    assign txpolarity   = {gt_txpolarity[3],    gt_txpolarity[2],   gt_txpolarity[1],   gt_txpolarity[0] };
    assign txdiffctrl   = {gt_txdiffctrl[3],    gt_txdiffctrl[2],   gt_txdiffctrl[1],   gt_txdiffctrl[0] };
    assign txpostcursor = {gt_txpostcursor[3],  gt_txpostcursor[2], gt_txpostcursor[1], gt_txpostcursor[0] };
    assign txprecursor  = {gt_txprecursor[3],   gt_txprecursor[2],  gt_txprecursor[1],  gt_txprecursor[0] };
    assign txmaincursor = {gt_txmaincursor[3],  gt_txmaincursor[2], gt_txmaincursor[1], gt_txmaincursor[0] };

    wire [31:0]  cfg_lane [3:0];
    generate
    for (genvar LANE = 0; LANE < 4; LANE++) begin
        assign cfg_lane[LANE] = {   gt_loopback[LANE],      // 31:29
                                    2'b0,                   // 28:27
                                    gt_rxlpmen[LANE],       // 26
                                    gt_rxpolarity[LANE],    // 25
                                    gt_txpolarity[LANE],    // 24
                                    1'b0,                   // 23
                                    gt_txpostcursor[LANE],  // 22:18
                                    gt_txmaincursor[LANE],  // 17:11
                                    1'b0,                   // 10
                                    gt_txprecursor[LANE],   // 9:5
                                    gt_txdiffctrl[LANE]     // 4:0
        };
    end
    endgenerate

    // Status

    wire [3:0]    Rx_PRBS_Error_cdc;
    wire [3:0]    Rx_Seed_Zero_cdc;
    wire [3:0]    Tx_Seed_Zero_cdc;

    wire [47 : 0]   rx_word_cnt_cdc[3:0];
    wire [47 : 0]   rx_err_bit_cnt_cdc[3:0];
    wire [47 : 0]   tx_word_cnt_cdc[3:0];

    wire [11:0]  status_cdc_in, status_cdc_out;

    assign status_cdc_in = {Tx_Seed_Zero, Rx_Seed_Zero, Rx_PRBS_Error};

    assign {Tx_Seed_Zero_cdc, Rx_Seed_Zero_cdc, Rx_PRBS_Error_cdc} = status_cdc_out;

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
            .WIDTH          ( $size(rx_err_bit_cnt[ln_idx]) )
        )
        cdc_rx_err_wd_cnt (
            .src_clk  (  1'h0                       ),
            .src_in   ( rx_err_bit_cnt[ln_idx]     ),
            .dest_out ( rx_err_bit_cnt_cdc[ln_idx] ),
            .dest_clk ( ap_clk                      )
        );


    end

    //########################################
    //### registers
    //########################################
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin
            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            gtwiz_reset_all                 <= 'h0;
            gtwiz_reset_tx_pll_and_datapath <= 'h0;
            gtwiz_reset_rx_pll_and_datapath <= 'h0;
            gtwiz_reset_tx_datapath         <= 'h0;
            gtwiz_reset_rx_datapath         <= 'h0;
            Tx_Enable           <= 'h0;
            Rx_Enable           <= 'h0;
            clear_status        <= 'h0;
            latch_status        <= 'h0;
            Disable_PRBS_Ref    <= 'h0;
            Tx_Error_Inject     <= 'h0;

        end else begin

            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            // set it back to '0': this creates a pulse
            gtwiz_reset_all                 <= 'h0;
            gtwiz_reset_tx_pll_and_datapath <= 'h0;
            gtwiz_reset_rx_pll_and_datapath <= 'h0;
            gtwiz_reset_tx_datapath         <= 'h0;
            gtwiz_reset_rx_datapath         <= 'h0;

            watchdog_alarm_d <= watchdog_alarm_cdc;

            if (watchdog_alarm_d != watchdog_alarm_cdc) begin
                Tx_Enable <= 'h0;
                Rx_Enable <= 'h0;
            end

            if (cs) begin

                case (addr_6)
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
                        case (addr_5_0)
                            // default : begin
                            6'b000000 : begin // 0x0

                                if (we) begin

                                    gtwiz_reset_all    <= {4{wdata[0]}};
                                    gtwiz_reset_tx_pll_and_datapath <= {4{wdata[2]}};
                                    gtwiz_reset_rx_pll_and_datapath <= {4{wdata[3]}};
                                    Tx_Enable   <= wdata[4];
                                    Rx_Enable   <= wdata[5];
                                    gtwiz_reset_tx_datapath <= {4{wdata[6]}};
                                    gtwiz_reset_rx_datapath <= {4{wdata[7]}};

                                    if (wdata[8]) begin
                                        clear_status <= ~ clear_status;
                                    end

                                    if (wdata[9]) begin
                                        latch_status <= ~ latch_status;
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

                                rdata[4]        <= Tx_Enable;
                                rdata[5]        <= Rx_Enable;

                                rdata[28]       <= Disable_PRBS_Ref;

                                cmd_cmplt   <= 'h1;
                            end
                            6'b000001 : begin
                                if (we) begin
                                    gt_loopback[0]      <= lane_loopback;
                                    gt_rxlpmen[0]       <= lane_rxlpmen;
                                    gt_rxpolarity[0]    <= lane_rxpolarity;
                                    gt_txpolarity[0]    <= lane_txpolarity;
                                    gt_txpostcursor[0]  <= lane_txpostcursor;
                                    gt_txmaincursor[0]  <= lane_txmaincursor;
                                    gt_txprecursor[0]   <= lane_txprecursor;
                                    gt_txdiffctrl[0]    <= lane_txdiffctrl;
                                end

                                rdata       <= cfg_lane[0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b000010 : begin
                                if (we) begin
                                    gt_loopback[1]      <= lane_loopback;
                                    gt_rxlpmen[1]       <= lane_rxlpmen;
                                    gt_rxpolarity[1]    <= lane_rxpolarity;
                                    gt_txpolarity[1]    <= lane_txpolarity;
                                    gt_txpostcursor[1]  <= lane_txpostcursor;
                                    gt_txmaincursor[1]  <= lane_txmaincursor;
                                    gt_txprecursor[1]   <= lane_txprecursor;
                                    gt_txdiffctrl[1]    <= lane_txdiffctrl;
                                end

                                rdata       <= cfg_lane[1];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b000011 : begin
                                if (we) begin
                                    gt_loopback[2]      <= lane_loopback;
                                    gt_rxlpmen[2]       <= lane_rxlpmen;
                                    gt_rxpolarity[2]    <= lane_rxpolarity;
                                    gt_txpolarity[2]    <= lane_txpolarity;
                                    gt_txpostcursor[2]  <= lane_txpostcursor;
                                    gt_txmaincursor[2]  <= lane_txmaincursor;
                                    gt_txprecursor[2]   <= lane_txprecursor;
                                    gt_txdiffctrl[2]    <= lane_txdiffctrl;
                                end

                                rdata       <= cfg_lane[2];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b000100 : begin
                                if (we) begin
                                    gt_loopback[3]      <= lane_loopback;
                                    gt_rxlpmen[3]       <= lane_rxlpmen;
                                    gt_rxpolarity[3]    <= lane_rxpolarity;
                                    gt_txpolarity[3]    <= lane_txpolarity;
                                    gt_txpostcursor[3]  <= lane_txpostcursor;
                                    gt_txmaincursor[3]  <= lane_txmaincursor;
                                    gt_txprecursor[3]   <= lane_txprecursor;
                                    gt_txdiffctrl[3]    <= lane_txdiffctrl;
                                end

                                rdata       <= cfg_lane[3];
                                cmd_cmplt   <= 'h1;
                            end

                            // 6'b000111 => 6'b001111 not used
                            6'b001000 : begin
                                rdata       <= tx_word_cnt_cdc[0][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b001001 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[0][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            6'b001010 : begin
                                rdata       <= tx_word_cnt_cdc[1][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b001011 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[1][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            6'b001100 : begin
                                rdata       <= tx_word_cnt_cdc[2][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b001101 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[2][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            6'b001110 : begin
                                rdata       <= tx_word_cnt_cdc[3][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b001111 : begin
                                rdata[15:0] <= tx_word_cnt_cdc[3][47:32];
                                cmd_cmplt   <= 'h1;
                            end


                            6'b010000 : begin
                                rdata       <= rx_word_cnt_cdc[0][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b010001 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[0][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b010010 : begin
                                rdata       <= rx_err_bit_cnt_cdc[0][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b010011 : begin
                                rdata[15:0] <= rx_err_bit_cnt_cdc[0][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            6'b010100 : begin
                                rdata       <= rx_word_cnt_cdc[1][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b010101 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[1][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b010110 : begin
                                rdata       <= rx_err_bit_cnt_cdc[1][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b010111 : begin
                                rdata[15:0] <= rx_err_bit_cnt_cdc[1][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            6'b011000 : begin
                                rdata       <= rx_word_cnt_cdc[2][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b011001 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[2][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b011010 : begin
                                rdata       <= rx_err_bit_cnt_cdc[2][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b011011 : begin
                                rdata[15:0] <= rx_err_bit_cnt_cdc[2][47:32];
                                cmd_cmplt   <= 'h1;
                            end

                            6'b011100 : begin
                                rdata       <= rx_word_cnt_cdc[3][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b011101 : begin
                                rdata[15:0] <= rx_word_cnt_cdc[3][47:32];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b011110 : begin
                                rdata       <= rx_err_bit_cnt_cdc[3][31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            6'b011111 : begin
                                rdata[15:0] <= rx_err_bit_cnt_cdc[3][47:32];
                                cmd_cmplt   <= 'h1;
                            end


                            6'b100000 : begin
                                rdata       <= sec_cnt;
                                cmd_cmplt   <= 'h1;
                            end

                            6'b100001 : begin

                                rdata[0]        <= & gt_power_good;
                                rdata[1]        <= | gt_power_good_f;
                                rdata[2]        <= | gt_power_good_r;

                                rdata[4]        <= gtf_qpll0_lock;
                                rdata[5]        <= gtf_qpll0_lock_f;
                                rdata[6]        <= gtf_qpll0_lock_r;

                                rdata[8]        <= & gt_reset_tx_done;
                                rdata[9]        <= | gt_reset_tx_done_f;
                                rdata[10]       <= | gt_reset_tx_done_r;

                                rdata[12]       <= & gt_reset_rx_done;
                                rdata[13]       <= | gt_reset_rx_done_f;
                                rdata[14]       <= | gt_reset_rx_done_r;

                                rdata[23:20]    <= Rx_PRBS_Error_cdc;
                                rdata[27:24]    <= Rx_Seed_Zero_cdc;
                                rdata[31:28]    <= Tx_Seed_Zero_cdc;

                                cmd_cmplt   <= 'h1;
                            end

                            6'b100010 : begin

                                if (we) begin

                                    gtwiz_reset_all[0]                  <= wdata[0];
                                    gtwiz_reset_tx_pll_and_datapath[0]  <= wdata[1];
                                    gtwiz_reset_rx_pll_and_datapath[0]  <= wdata[2];
                                    gtwiz_reset_tx_datapath[0]          <= wdata[3];
                                    gtwiz_reset_rx_datapath[0]          <= wdata[4];

                                end

                                rdata[0]     <=  gt_reset_tx_done[0];
                                rdata[1]     <=  gt_reset_tx_done_f[0];
                                rdata[2]     <=  gt_reset_tx_done_r[0];
                                rdata[3]     <=  gtf_ch_txresetdone[0];
                                rdata[4]     <=  gtf_ch_txresetdone_f[0];
                                rdata[5]     <=  gtf_ch_txresetdone_r[0];
                                rdata[6]     <=  gtf_ch_txsyncdone[0];
                                rdata[7]     <=  gtf_ch_txsyncdone_f[0];
                                rdata[8]     <=  gtf_ch_txsyncdone_r[0];

                                rdata[9]     <=  gt_reset_rx_done[0];
                                rdata[10]    <=  gt_reset_rx_done_f[0];
                                rdata[11]    <=  gt_reset_rx_done_r[0];
                                rdata[12]    <=  gtf_ch_rxresetdone[0];
                                rdata[13]    <=  gtf_ch_rxresetdone_f[0];
                                rdata[14]    <=  gtf_ch_rxresetdone_r[0];
                                rdata[15]    <=  gtf_ch_rxsyncdone[0];
                                rdata[16]    <=  gtf_ch_rxsyncdone_f[0];
                                rdata[17]    <=  gtf_ch_rxsyncdone_r[0];

                                rdata[18]    <=  gt_bufferbypass_tx_done[0];
                                rdata[19]    <=  gt_bufferbypass_tx_done_f[0];
                                rdata[20]    <=  gt_bufferbypass_tx_done_r[0];
                                rdata[21]    <=  gt_bufferbypass_rx_done[0];
                                rdata[22]    <=  gt_bufferbypass_rx_done_f[0];
                                rdata[23]    <=  gt_bufferbypass_rx_done_r[0];

                                rdata[24]    <= gt_power_good[0];
                                rdata[25]    <= gt_power_good_f[0];
                                rdata[26]    <= gt_power_good_r[0];

                                cmd_cmplt   <= 'h1;
                            end

                            6'b100011 : begin

                                if (we) begin

                                    gtwiz_reset_all[1]                  <= wdata[0];
                                    gtwiz_reset_tx_pll_and_datapath[1]  <= wdata[1];
                                    gtwiz_reset_rx_pll_and_datapath[1]  <= wdata[2];
                                    gtwiz_reset_tx_datapath[1]          <= wdata[3];
                                    gtwiz_reset_rx_datapath[1]          <= wdata[4];

                                end

                                rdata[0]     <=  gt_reset_tx_done[1];
                                rdata[1]     <=  gt_reset_tx_done_f[1];
                                rdata[2]     <=  gt_reset_tx_done_r[1];
                                rdata[3]     <=  gtf_ch_txresetdone[1];
                                rdata[4]     <=  gtf_ch_txresetdone_f[1];
                                rdata[5]     <=  gtf_ch_txresetdone_r[1];
                                rdata[6]     <=  gtf_ch_txsyncdone[1];
                                rdata[7]     <=  gtf_ch_txsyncdone_f[1];
                                rdata[8]     <=  gtf_ch_txsyncdone_r[1];

                                rdata[9]     <=  gt_reset_rx_done[1];
                                rdata[10]    <=  gt_reset_rx_done_f[1];
                                rdata[11]    <=  gt_reset_rx_done_r[1];
                                rdata[12]    <=  gtf_ch_rxresetdone[1];
                                rdata[13]    <=  gtf_ch_rxresetdone_f[1];
                                rdata[14]    <=  gtf_ch_rxresetdone_r[1];
                                rdata[15]    <=  gtf_ch_rxsyncdone[1];
                                rdata[16]    <=  gtf_ch_rxsyncdone_f[1];
                                rdata[17]    <=  gtf_ch_rxsyncdone_r[1];

                                rdata[18]    <=  gt_bufferbypass_tx_done[1];
                                rdata[19]    <=  gt_bufferbypass_tx_done_f[1];
                                rdata[20]    <=  gt_bufferbypass_tx_done_r[1];
                                rdata[21]    <=  gt_bufferbypass_rx_done[1];
                                rdata[22]    <=  gt_bufferbypass_rx_done_f[1];
                                rdata[23]    <=  gt_bufferbypass_rx_done_r[1];

                                rdata[24]    <= gt_power_good[1];
                                rdata[25]    <= gt_power_good_f[1];
                                rdata[26]    <= gt_power_good_r[1];

                                cmd_cmplt   <= 'h1;
                            end

                            6'b100100 : begin

                                if (we) begin

                                    gtwiz_reset_all[2]                  <= wdata[0];
                                    gtwiz_reset_tx_pll_and_datapath[2]  <= wdata[1];
                                    gtwiz_reset_rx_pll_and_datapath[2]  <= wdata[2];
                                    gtwiz_reset_tx_datapath[2]          <= wdata[3];
                                    gtwiz_reset_rx_datapath[2]          <= wdata[4];

                                end

                                rdata[0]     <=  gt_reset_tx_done[2];
                                rdata[1]     <=  gt_reset_tx_done_f[2];
                                rdata[2]     <=  gt_reset_tx_done_r[2];
                                rdata[3]     <=  gtf_ch_txresetdone[2];
                                rdata[4]     <=  gtf_ch_txresetdone_f[2];
                                rdata[5]     <=  gtf_ch_txresetdone_r[2];
                                rdata[6]     <=  gtf_ch_txsyncdone[2];
                                rdata[7]     <=  gtf_ch_txsyncdone_f[2];
                                rdata[8]     <=  gtf_ch_txsyncdone_r[2];

                                rdata[9]     <=  gt_reset_rx_done[2];
                                rdata[10]    <=  gt_reset_rx_done_f[2];
                                rdata[11]    <=  gt_reset_rx_done_r[2];
                                rdata[12]    <=  gtf_ch_rxresetdone[2];
                                rdata[13]    <=  gtf_ch_rxresetdone_f[2];
                                rdata[14]    <=  gtf_ch_rxresetdone_r[2];
                                rdata[15]    <=  gtf_ch_rxsyncdone[2];
                                rdata[16]    <=  gtf_ch_rxsyncdone_f[2];
                                rdata[17]    <=  gtf_ch_rxsyncdone_r[2];

                                rdata[18]    <=  gt_bufferbypass_tx_done[2];
                                rdata[19]    <=  gt_bufferbypass_tx_done_f[2];
                                rdata[20]    <=  gt_bufferbypass_tx_done_r[2];
                                rdata[21]    <=  gt_bufferbypass_rx_done[2];
                                rdata[22]    <=  gt_bufferbypass_rx_done_f[2];
                                rdata[23]    <=  gt_bufferbypass_rx_done_r[2];

                                rdata[24]    <= gt_power_good[2];
                                rdata[25]    <= gt_power_good_f[2];
                                rdata[26]    <= gt_power_good_r[2];

                                cmd_cmplt   <= 'h1;
                            end

                            6'b100101 : begin

                                if (we) begin

                                    gtwiz_reset_all[3]                  <= wdata[0];
                                    gtwiz_reset_tx_pll_and_datapath[3]  <= wdata[1];
                                    gtwiz_reset_rx_pll_and_datapath[3]  <= wdata[2];
                                    gtwiz_reset_tx_datapath[3]          <= wdata[3];
                                    gtwiz_reset_rx_datapath[3]          <= wdata[4];

                                end

                                rdata[0]     <=  gt_reset_tx_done[3];
                                rdata[1]     <=  gt_reset_tx_done_f[3];
                                rdata[2]     <=  gt_reset_tx_done_r[3];
                                rdata[3]     <=  gtf_ch_txresetdone[3];
                                rdata[4]     <=  gtf_ch_txresetdone_f[3];
                                rdata[5]     <=  gtf_ch_txresetdone_r[3];
                                rdata[6]     <=  gtf_ch_txsyncdone[3];
                                rdata[7]     <=  gtf_ch_txsyncdone_f[3];
                                rdata[8]     <=  gtf_ch_txsyncdone_r[3];

                                rdata[9]     <=  gt_reset_rx_done[3];
                                rdata[10]    <=  gt_reset_rx_done_f[3];
                                rdata[11]    <=  gt_reset_rx_done_r[3];
                                rdata[12]    <=  gtf_ch_rxresetdone[3];
                                rdata[13]    <=  gtf_ch_rxresetdone_f[3];
                                rdata[14]    <=  gtf_ch_rxresetdone_r[3];
                                rdata[15]    <=  gtf_ch_rxsyncdone[3];
                                rdata[16]    <=  gtf_ch_rxsyncdone_f[3];
                                rdata[17]    <=  gtf_ch_rxsyncdone_r[3];

                                rdata[18]    <=  gt_bufferbypass_tx_done[3];
                                rdata[19]    <=  gt_bufferbypass_tx_done_f[3];
                                rdata[20]    <=  gt_bufferbypass_tx_done_r[3];
                                rdata[21]    <=  gt_bufferbypass_rx_done[3];
                                rdata[22]    <=  gt_bufferbypass_rx_done_f[3];
                                rdata[23]    <=  gt_bufferbypass_rx_done_r[3];

                                rdata[24]    <= gt_power_good[3];
                                rdata[25]    <= gt_power_good_f[3];
                                rdata[26]    <= gt_power_good_r[3];

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

endmodule : gtf_prbs_reg_array
`default_nettype wire
