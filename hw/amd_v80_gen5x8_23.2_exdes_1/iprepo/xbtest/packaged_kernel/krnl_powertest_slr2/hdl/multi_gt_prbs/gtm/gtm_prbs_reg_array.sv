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

module gtm_prbs_reg_array #(
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

    output wire [11 : 0]    loopback,
    output wire [3 : 0]     rxlpmen,
    output wire [3 : 0]     rxpolarity,
    output wire [3 : 0]     txpolarity,
    output wire [19 : 0]    txdiffctrl,
    output wire [27 : 0]    txmaincursor,
    output wire [23 : 0]    txpostcursor,
    output wire [23 : 0]    txprecursor,

    output logic    bridge_reset_all,
    output logic    reset_tx_pll_and_datapath,
    output logic    reset_rx_pll_and_datapath,
    output logic    reset_tx_datapath,
    output logic    reset_rx_datapath,

    input  wire     gt_power_good,
    input  wire     gt_power_good_f,
    input  wire     gt_power_good_r,
    input  wire     lcpll_lock,
    input  wire     lcpll_lock_f,
    input  wire     lcpll_lock_r,

    input  wire     bridge_reset_tx_done,
    input  wire     bridge_reset_tx_done_f,
    input  wire     bridge_reset_tx_done_r,
    input  wire     bridge_reset_rx_done,
    input  wire     bridge_reset_rx_done_f,
    input  wire     bridge_reset_rx_done_r,

    output logic           Tx_Enable,
    output logic           Rx_Enable,
    output logic           clear_status,
    output logic           latch_status,
    output logic           Disable_PRBS_Ref,
    output logic [3 :0]    Tx_Error_Inject,

    input  wire [3 : 0]    Rx_PRBS_Error,
    input  wire [3 : 0]    Rx_Seed_Zero,
    input  wire [3 : 0]    Tx_Seed_Zero,

    input  wire [30 : 0]   sec_cnt,
    input  wire [31 : 0]   stopwatch,
    input  wire [47 : 0]   rx_word_cnt[3:0],
    input  wire [47 : 0]   rx_err_bit_cnt[3:0],
    input  wire [47 : 0]   tx_word_cnt[3:0],

    input  wire            toggle_1_sec,

    input  wire         cs,
    input  wire         we,
    input  wire  [21:0] addr,
    input  wire  [31:0] wdata,
    output logic [31:0] rdata           = 'h0,
    output logic        cmd_cmplt       = 'h0
);

    wire        sel_core_reg    = addr[6]; // used to separate BI/status from all other registers
    wire        sel_bi_ctrl     = addr[4];
    wire [5:0]  addr_5_0        = addr[5:0];

    wire        we_bi           = (~sel_core_reg & ~sel_bi_ctrl & cs & we) ? 'b1 : 'b0;
    wire        we_ctrl_status  = (~sel_core_reg &  sel_bi_ctrl & cs & we) ? 'b1 : 'b0;
    // when reading, use the address from the read pipeline
    wire [2:0]  addr_bi_ctrl_status  = (cs & we)? addr[2:0]: read_pipe[2].addr[2:0];
    wire [31:0] rdata_bi;
    wire [31:0] rdata_ctrl_status;

    // read pipeling for 64 registers (6 bits of addresses)
    wire [31:0]    rdata_array[0:63];
    typedef struct {
        logic           cs;
        logic [6:0]     addr;
        logic [31:0]    rdata[0:63];
    } Read_Pipe_Type;

    Read_Pipe_Type C_RST_READ_PIPE = '{
        cs          : '0,
        addr        : '0,
        rdata       : '{default:'0}
    };
    localparam integer READ_PIPE_SIZE = 3;
    Read_Pipe_Type read_pipe [0:READ_PIPE_SIZE-1] = '{default:C_RST_READ_PIPE};


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
        .Addr       ( addr_bi_ctrl_status           ),
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

        .We             ( we_ctrl_status        ),
        .Addr           ( addr_bi_ctrl_status   ),
        .Data_In        ( wdata                 ),
        .User_Status_1  ( 32'b0                 ),
        .Data_Out       ( rdata_ctrl_status     )
    );


    // Configuration

    // VUp: from XXV example design trans_debug
    // Versal from gt_quad_base example design (gt_bridge ip)
    // GTF: from wizard rules_output.vh files

    logic [2:0]  gt_loopback     [3:0]  = '{default:'0};
    logic [3:0]  gt_rxlpmen             = 'h0;
    logic [3:0]  gt_rxpolarity          = 'h0;
    logic [3:0]  gt_txpolarity          = 'h0;

    logic [5:0]  gt_txpostcursor [3:0]  = '{default:'0};
    logic [5:0]  gt_txprecursor  [3:0]  = '{default:'0};
    logic [4:0]  gt_txdiffctrl   [3:0]  = '{default: 5'b00000};  // not used by GTM
    logic [6:0]  gt_txmaincursor [3:0]  = '{default: 7'b1001101}; // extracted from GTM example design

    wire [2:0]  lane_loopback;
    wire        lane_rxlpmen;
    wire        lane_rxpolarity;
    wire        lane_txpolarity;
    wire [5:0]  lane_txpostcursor;
    wire [5:0]  lane_txprecursor ;
    wire [4:0]  lane_txdiffctrl  ;
    wire [6:0]  lane_txmaincursor;

    assign lane_loopback     = wdata[31:29];
    assign lane_rxlpmen      = wdata[26];
    assign lane_rxpolarity   = wdata[25];
    assign lane_txpolarity   = wdata[24];
    assign lane_txpostcursor = wdata[23:18];
    assign lane_txmaincursor = wdata[17:11];
    assign lane_txprecursor  = wdata[10:5];
    assign lane_txdiffctrl   = wdata[4:0];

    assign loopback     = {gt_loopback[3],      gt_loopback[2],     gt_loopback[1],     gt_loopback[0] };
    assign rxlpmen      = {gt_rxlpmen[3],       gt_rxlpmen[2],      gt_rxlpmen[1],      gt_rxlpmen[0] };
    assign rxpolarity   = {gt_rxpolarity[3],    gt_rxpolarity[2],   gt_rxpolarity[1],   gt_rxpolarity[0] };
    assign txpolarity   = {gt_txpolarity[3],    gt_txpolarity[2],   gt_txpolarity[1],   gt_txpolarity[0] };
    assign txdiffctrl   = {gt_txdiffctrl[3],    gt_txdiffctrl[2],   gt_txdiffctrl[1],   gt_txdiffctrl[0] };
    assign txpostcursor = {gt_txpostcursor[3],  gt_txpostcursor[2], gt_txpostcursor[1], gt_txpostcursor[0] };
    assign txprecursor  = {gt_txprecursor[3],   gt_txprecursor[2],  gt_txprecursor[1],  gt_txprecursor[0] };
    assign txmaincursor = {gt_txmaincursor[3],  gt_txmaincursor[2], gt_txmaincursor[1], gt_txmaincursor[0] };

    wire [31:0] cfg_lane [3:0];
    generate
    for (genvar LANE = 0; LANE < 4; LANE++) begin
        assign cfg_lane[LANE] = {   gt_loopback[LANE],      // 31:29
                                    2'b0,                   // 28:27
                                    gt_rxlpmen[LANE],       // 26
                                    gt_rxpolarity[LANE],    // 25
                                    gt_txpolarity[LANE],    // 24
                                    gt_txpostcursor[LANE],  // 23:18
                                    gt_txmaincursor[LANE],  // 17:11
                                    gt_txprecursor[LANE],   // 10:5
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
        cdc_rx_err_bit_cnt (
            .src_clk  (  1'h0                       ),
            .src_in   ( rx_err_bit_cnt[ln_idx]     ),
            .dest_out ( rx_err_bit_cnt_cdc[ln_idx] ),
            .dest_clk ( ap_clk                      )
        );

    end


// 6'b000000
assign rdata_array[0][3:0]      = 'b0; // remove X' in simu
assign rdata_array[0][4]        = Tx_Enable;
assign rdata_array[0][5]        = Rx_Enable;
assign rdata_array[0][27:6]     = 'b0; // remove X' in simu
assign rdata_array[0][28]       = Disable_PRBS_Ref;
assign rdata_array[0][31:29]    = 'b0; // remove X' in simu
// b000001
assign rdata_array[1] = cfg_lane[0];
// b000010
assign rdata_array[2] = cfg_lane[1];
// b000011
assign rdata_array[3] = cfg_lane[2];
// b000100 : begin
assign rdata_array[4] = cfg_lane[3];
// 6'b000101 => 6'b000111 not used
// b001000
assign rdata_array[8]           = tx_word_cnt_cdc[0][31:0];
// b001001
assign rdata_array[9][15:0]     = tx_word_cnt_cdc[0][47:32];
assign rdata_array[9][31:16]    = tx_word_cnt_cdc[1][15:0];
// b001010
assign rdata_array[10]          = tx_word_cnt_cdc[1][47:16];
// b001011
assign rdata_array[11]          = tx_word_cnt_cdc[2][31:0];
// b001100
assign rdata_array[12][15:0]    = tx_word_cnt_cdc[2][47:32];
assign rdata_array[12][31:16]   = tx_word_cnt_cdc[3][15:0];
// b001101
assign rdata_array[13]          = tx_word_cnt_cdc[3][47:16];
// b001110
assign rdata_array[14]          = stopwatch;
// b001111
assign rdata_array[15]          = 'b0;
// b010000
assign rdata_array[16]          = rx_word_cnt_cdc[0][31:0];
// b010001
assign rdata_array[17][15:0]    = rx_word_cnt_cdc[0][47:32];
assign rdata_array[17][31:16]   = rx_err_bit_cnt_cdc[0][15:0];
// b010010
assign rdata_array[18]          = rx_err_bit_cnt_cdc[0][47:16];

// b010011
assign rdata_array[19]          = rx_word_cnt_cdc[1][31:0];
// b010100
assign rdata_array[20][15:0]    = rx_word_cnt_cdc[1][47:32];
assign rdata_array[20][31:16]   = rx_err_bit_cnt_cdc[1][15:0];
// b010101
assign rdata_array[21]          = rx_err_bit_cnt_cdc[1][47:16];

// b010110
assign rdata_array[22]          = rx_word_cnt_cdc[2][31:0];
// b010111
assign rdata_array[23][15:0]    = rx_word_cnt_cdc[2][47:32];
assign rdata_array[23][31:16]   = rx_err_bit_cnt_cdc[2][15:0];
// b011000
assign rdata_array[24]          = rx_err_bit_cnt_cdc[2][47:16];

// b011001
assign rdata_array[25]          = rx_word_cnt_cdc[3][31:0];
// b011010
assign rdata_array[26][15:0]    = rx_word_cnt_cdc[3][47:32];
assign rdata_array[26][31:16]   = rx_err_bit_cnt_cdc[3][15:0];
// b011011
assign rdata_array[27]          = rx_err_bit_cnt_cdc[3][47:16];

// b011100
assign rdata_array[28]          = {toggle_1_sec, sec_cnt[30 : 0]};

// b011101
assign rdata_array[29][0]       = gt_power_good;
assign rdata_array[29][1]       = gt_power_good_f;
assign rdata_array[29][2]       = gt_power_good_r;
assign rdata_array[29][3]       = 'b0; // remove X' in simu
assign rdata_array[29][4]       = lcpll_lock;
assign rdata_array[29][5]       = lcpll_lock_f;
assign rdata_array[29][6]       = lcpll_lock_r;
assign rdata_array[29][7]       = 'b0; // remove X' in simu
assign rdata_array[29][8]       = bridge_reset_tx_done;
assign rdata_array[29][9]       = bridge_reset_tx_done_f;
assign rdata_array[29][10]      = bridge_reset_tx_done_r;
assign rdata_array[29][11]      = 'b0; // remove X' in simu
assign rdata_array[29][12]      = bridge_reset_rx_done;
assign rdata_array[29][13]      = bridge_reset_rx_done_f;
assign rdata_array[29][14]      = bridge_reset_rx_done_r;
assign rdata_array[29][19:15]   = 'b0; // remove X' in simu
assign rdata_array[29][23:20]   = Rx_PRBS_Error_cdc;
assign rdata_array[29][27:24]   = Rx_Seed_Zero_cdc;
assign rdata_array[29][31:28]   = Tx_Seed_Zero_cdc;
// b011110
assign rdata_array[30]      = 'b0;
// b011111
assign rdata_array[31]      = 'b0;
// b100000
assign rdata_array[32]      = 'b0;


//########################################
//### registers
//########################################
always_ff @(posedge ap_clk) begin

    // set it back to '0': this creates a pulse
    bridge_reset_all          <= 'h0;
    reset_tx_pll_and_datapath <= 'h0;
    reset_rx_pll_and_datapath <= 'h0;
    reset_tx_datapath         <= 'h0;
    reset_rx_datapath         <= 'h0;

    watchdog_alarm_d <= watchdog_alarm_cdc;

    if (watchdog_alarm_d != watchdog_alarm_cdc) begin
        Tx_Enable <= 'h0;
        Rx_Enable <= 'h0;
    end

    if (cs && we && sel_core_reg) begin

        case (addr_5_0)
            // default : begin
            6'b000000 : begin // 0x0
                bridge_reset_all          <= wdata[0];
                reset_tx_pll_and_datapath <= wdata[2];
                reset_rx_pll_and_datapath <= wdata[3];
                Tx_Enable   <= wdata[4];
                Rx_Enable   <= wdata[5];
                reset_tx_datapath <= wdata[6];
                reset_rx_datapath <= wdata[7];

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
            6'b000001 : begin
                gt_loopback[0]      <= lane_loopback;
                gt_rxlpmen[0]       <= lane_rxlpmen;
                gt_rxpolarity[0]    <= lane_rxpolarity;
                gt_txpolarity[0]    <= lane_txpolarity;
                gt_txpostcursor[0]  <= lane_txpostcursor;
                gt_txmaincursor[0]  <= lane_txmaincursor;
                gt_txprecursor[0]   <= lane_txprecursor;
                gt_txdiffctrl[0]    <= lane_txdiffctrl;
            end
            6'b000010 : begin
                gt_loopback[1]      <= lane_loopback;
                gt_rxlpmen[1]       <= lane_rxlpmen;
                gt_rxpolarity[1]    <= lane_rxpolarity;
                gt_txpolarity[1]    <= lane_txpolarity;
                gt_txpostcursor[1]  <= lane_txpostcursor;
                gt_txmaincursor[1]  <= lane_txmaincursor;
                gt_txprecursor[1]   <= lane_txprecursor;
                gt_txdiffctrl[1]    <= lane_txdiffctrl;
            end
            6'b000011 : begin
                gt_loopback[2]      <= lane_loopback;
                gt_rxlpmen[2]       <= lane_rxlpmen;
                gt_rxpolarity[2]    <= lane_rxpolarity;
                gt_txpolarity[2]    <= lane_txpolarity;
                gt_txpostcursor[2]  <= lane_txpostcursor;
                gt_txmaincursor[2]  <= lane_txmaincursor;
                gt_txprecursor[2]   <= lane_txprecursor;
                gt_txdiffctrl[2]    <= lane_txdiffctrl;
            end
            6'b000100 : begin
                gt_loopback[3]      <= lane_loopback;
                gt_rxlpmen[3]       <= lane_rxlpmen;
                gt_rxpolarity[3]    <= lane_rxpolarity;
                gt_txpolarity[3]    <= lane_txpolarity;
                gt_txpostcursor[3]  <= lane_txpostcursor;
                gt_txmaincursor[3]  <= lane_txmaincursor;
                gt_txprecursor[3]   <= lane_txprecursor;
                gt_txdiffctrl[3]    <= lane_txdiffctrl;
            end

            // 6'b000111 => 6'b001111 not used
            // 6'b001000 => 6'b010001: word, err, second counters, status register
            6'b001000 : begin
            end
            6'b001001 : begin
            end
            6'b001010 : begin
            end
            6'b001011 : begin
            end
            6'b001100 : begin
            end
            6'b001101 : begin
            end
            6'b001110 : begin
            end
            6'b001111 : begin
            end
            6'b010000 : begin
            end
            6'b010001 : begin
            end

            6'b010010 : begin
            end
            6'b010011 : begin
            end
            6'b010100 : begin
            end
            6'b010101 : begin
            end
            6'b010110 : begin
            end
            6'b010111 : begin
            end
            6'b011000 : begin
            end
            6'b011001 : begin
            end
            6'b011010 : begin
            end
            6'b011011 : begin
            end
            6'b011100 : begin
            end
            6'b011101 : begin
            end
            6'b011110 : begin
            end
            6'b011111 : begin
            end
            6'b100000 : begin
            end
            6'b100001 : begin
            end

            default : begin
                $display("Illegal address sent to gtm_reg_array");
            end
        endcase

    end

    // pipeline the address decoding to ensure timing closure for ap_clk > 300MHz
    // sel_core_reg is used to select BI/Status or core register
    // max 6 bits addresses are supported to select registers

    // drive the pipeline
    read_pipe[1:READ_PIPE_SIZE-1] <= read_pipe[0:READ_PIPE_SIZE-2];
    read_pipe[0].cs   <= cs & ~we;
    read_pipe[0].addr <= addr;

    // stage 0:
    // selection based on the 2 LSB
    // 0,1,2,3 goes into => 0; 4,5,6,7 => 1; 8,9,10,11 => 2; 12,13,14,15 => 3; 16,17,18,19 => 4; 20,21,22,23 => 5
    for (int i = 0; i < 16; i = i + 1) begin
        case (addr[1:0])
            'b00 : begin
                read_pipe[0].rdata[i] <= rdata_array[i*4];
            end
            'b01 : begin
                read_pipe[0].rdata[i] <= rdata_array[i*4 + 1];
            end
            'b10 : begin
                read_pipe[0].rdata[i] <= rdata_array[i*4 + 2];
            end
            default : begin
                read_pipe[0].rdata[i] <= rdata_array[i*4 + 3];
            end
        endcase
    end

    // Stage 1:
    //  0,1,2,3 goes into => 0; 4,5,6,7 => 1; 8,9,10,11 => 2; 12,13,14,15 => 3
    for (int i = 0; i < 4; i = i + 1) begin
        case (read_pipe[0].addr[3:2])
            'b00 : begin
                read_pipe[1].rdata[i] <= read_pipe[0].rdata[i*4];
            end
            'b01 : begin
                read_pipe[1].rdata[i] <= read_pipe[0].rdata[i*4 + 1];
            end
            'b10 : begin
                read_pipe[1].rdata[i] <= read_pipe[0].rdata[i*4 + 2];
            end
            default : begin
                read_pipe[1].rdata[i] <= read_pipe[0].rdata[i*4 + 3];
            end
        endcase
    end

    // Stage 2
    case (read_pipe[1].addr[5:4])
        'b00 : begin
            read_pipe[2].rdata[0] <= read_pipe[1].rdata[0];
        end
        'b01 : begin
            read_pipe[2].rdata[0] <= read_pipe[1].rdata[1];
        end
        'b10 : begin
            read_pipe[2].rdata[0] <= read_pipe[1].rdata[2];
        end
        default : begin
            read_pipe[2].rdata[0] <= read_pipe[1].rdata[3];
        end
    endcase

    // Stage 3
    rdata             <= '0;
    if (read_pipe[2].cs) begin
        if (~read_pipe[2].addr[6]) begin    // sel_core_reg
            if (read_pipe[2].addr[4]) begin    // sel_bi_ctrl
                rdata <= rdata_ctrl_status;
            end else begin
                rdata <= rdata_bi;
            end
        end else begin
            rdata <= read_pipe[2].rdata[0];
        end
    end

    //  immediately acknowledge write
    cmd_cmplt <= (cs && we) || read_pipe[2].cs;

    if (ap_rst) begin

        bridge_reset_all          <= 'h0;
        reset_tx_pll_and_datapath <= 'h0;
        reset_rx_pll_and_datapath <= 'h0;
        reset_tx_datapath         <= 'h0;
        reset_rx_datapath         <= 'h0;
        Tx_Enable                 <= 'h0;
        Rx_Enable                 <= 'h0;
        clear_status              <= 'h0;
        latch_status              <= 'h0;
        Disable_PRBS_Ref          <= 'h0;
        Tx_Error_Inject           <= 'h0;

        cmd_cmplt                 <= '0;

    end

end

endmodule : gtm_prbs_reg_array
`default_nettype wire
