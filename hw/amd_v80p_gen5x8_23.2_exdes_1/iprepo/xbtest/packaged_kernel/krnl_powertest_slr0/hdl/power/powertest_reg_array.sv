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

module powertest_reg_array #(
    parameter integer C_MAJOR_VERSION       = 0,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_BLOCK_ID            = 0,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_KRNL_SLR            = 0,    // Kernel SLR (SLR0 = 0, SLR1 = 1, SLR2 = 2, SLR3 = 3)
    parameter integer C_THROTTLE_MODE       = 1,    // 1: clock throttling with BUFG, 0: throttle clock enable of sites FF input oscillator
    parameter integer C_USE_AIE             = 0,    // 0: Disable AIE, 1: Enable AIE
    parameter integer C_REG_BLOCK_SIZE_ARRAY        [0:19][0:19],
    parameter integer C_DSP48E2_COL_SIZES_ARRRAY    [0:199],
    parameter integer C_RAMB36_COL_SIZES_ARRRAY     [0:199],
    parameter integer C_URAM288_COL_SIZES_ARRRAY    [0:199]
) (
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         ap_clk_2,
    input  wire         ap_clk_2_cont,
    input  wire         ap_rst_2,
    input  wire         watchdog_alarm_in,
    input  wire         cs,
    input  wire         we,
    input  wire  [11:0] addr,
    input  wire  [31:0] wdata,
    output logic [31:0] rdata                   = 'h0,
    output logic        cmd_cmplt               = 'h0,

    output logic [9:0]  ctrl_tog_pattern        = 'h0,
    output logic        ctrl_tog_update         = 'h0,
    output logic        ctrl_pwr_FF_en          = 'h0,
    output logic        ctrl_pwr_DSP_en         = 'h0,
    output logic        ctrl_pwr_BRAM_en        = 'h0,
    output logic        ctrl_pwr_URAM_en        = 'h0,
    output logic        ctrl_pwr_AIE_en         = 'h0,

    input wire [31:0]   m_aie0_cycle_cnt,
    input wire [31:0]   m_aie0_tready_cnt,
    input wire [31:0]   m_aie0_txfer_cnt,
    input wire [31:0]   s_aie0_txfer_cnt
);

    function integer f_cnt_reg (input integer size_array [0:19][0:19] );
        integer cnt;
        cnt = 0;
        for (integer ii = 0; ii < 20; ii++) begin
            for (integer jj = 0; jj < 20; jj++) begin
                cnt = cnt + size_array[ii][jj];
            end
        end
        return cnt;
    endfunction: f_cnt_reg
    function integer f_cnt_col (input integer size_array [0:199] );
        integer cnt;
        cnt = 0;
        for (integer ii = 0; ii < 200; ii++) begin
            cnt = cnt + size_array[ii];
        end
        return cnt;
    endfunction: f_cnt_col

    localparam integer C_NUM_REG        = f_cnt_reg(C_REG_BLOCK_SIZE_ARRAY);     // Total number of SLICE sites in power kernel
    localparam integer C_NUM_DSP48E2    = f_cnt_col(C_DSP48E2_COL_SIZES_ARRRAY); // Total number of DSP48E2 sites in power kernel
    localparam integer C_NUM_RAMB36     = f_cnt_col(C_RAMB36_COL_SIZES_ARRRAY);  // Total number of RAMB36 sites in power kernel
    localparam integer C_NUM_URAM288    = f_cnt_col(C_URAM288_COL_SIZES_ARRRAY); // Total number of URAM288 sites in power kernel

    wire [2:0]  addr_2_0    = addr[2:0];
    wire        addr_5      = addr[5];
    wire        addr_4      = addr[4];
    wire        addr_5_4    = addr[5:4];
    wire        we_bi           = (addr_5_4 == 2'b00) ? cs & we : 'b0;
    wire        we_ctrl_status  = (addr_5_4 == 2'b01) ? cs & we : 'b0;
    wire [31:0] rdata_bi;
    wire [31:0] rdata_ctrl_status;

    logic ctrl_tog_upd       = 'b0;
    logic ctrl_tog_upd_d1    = 'b0;
    logic ctrl_tog_upd_d2    = 'b0;
    logic ctrl_tog_upd_d3    = 'b0;
    logic ctrl_tog_upd_d4    = 'b0;

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
        .src_clk  ( 'b0         ),
        .src_in   ( watchdog_alarm_in   ),
        .dest_out ( watchdog_alarm_cdc  ),
        .dest_clk ( ap_clk              )
    );

    build_info_v4_0 #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
        .C_BLOCK_ID         ( C_BLOCK_ID            )
    ) u_build_info (
        .Clk        ( ap_clk                                                        ),
        .Rst        ( ap_rst                                                        ),
        .Info_1     ( 16'b0                                                         ),  // Info 1 reserved for future use
        .Info_2     ( {C_USE_AIE[3:0], 4'b0, C_KRNL_SLR[3:0], C_THROTTLE_MODE[3:0]} ),
        .Info_3     ( C_NUM_REG[15:0]                                               ),
        .Info_4     ( C_NUM_DSP48E2[15:0]                                           ),
        .Info_5     ( C_NUM_RAMB36[15:0]                                            ),
        .Info_6     ( C_NUM_URAM288[15:0]                                           ),
        .Info_7     ( C_CLOCK0_FREQ[15:0]           ),
        .Info_8     ( C_CLOCK1_FREQ[15:0]           ),

        .We         ( we_bi                                                         ),
        .Addr       ( addr[2:0]                                                     ),
        .Data_In    ( wdata                                                         ),
        .Data_Out   ( rdata_bi                                                      )
    );

    common_ctrl_status #(
        .C_CLOCK_FREQ               ( C_CLOCK0_FREQ ),
        .DEST_SYNC_FF               ( DEST_SYNC_FF  ),
        .C_CLK_TROTTLE_DETECT_EN    ( 0             ),
        .C_WATCHDOG_ENABLE          ( 0             ),
        .C_EXT_TOGGLE_1_SEC         ( 0             )
    ) u_common_ctrl_status (
        .ap_clk         ( ap_clk            ),
        .ap_clk_cont    ( ap_clk_cont       ),
        .ap_rst         ( ap_rst            ),
        .ap_clk_2       ( ap_clk_2          ),
        .ap_clk_2_cont  ( ap_clk_2_cont     ),
        .ap_rst_2       ( ap_rst_2          ),

        .toggle_1sec    ( 1'b0              ),
        .rst_watchdog   ( 1'b0              ),
        .watchdog_alarm (                   ),
        .We             ( we_ctrl_status    ),
        .Addr           ( addr[2:0]         ),
        .Data_In        ( wdata             ),
        .User_Status_1  ( 32'b0             ),
        .Data_Out       ( rdata_ctrl_status )
    );

    //########################################
    //### Write registers
    //########################################
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin
            ctrl_tog_pattern       <= 'h0;
            ctrl_tog_update        <= 'b0;
            ctrl_tog_upd           <= 'b0;
            ctrl_tog_upd_d1        <= 'b0;
            ctrl_tog_upd_d2        <= 'b0;
            ctrl_tog_upd_d3        <= 'b0;
            ctrl_tog_upd_d4        <= 'b0;

            ctrl_pwr_FF_en         <= 'b0;
            ctrl_pwr_DSP_en        <= 'b0;
            ctrl_pwr_BRAM_en       <= 'b0;
            ctrl_pwr_URAM_en       <= 'b0;
            ctrl_pwr_AIE_en        <= 'b0;

            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

        end else begin
            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            watchdog_alarm_d <= watchdog_alarm_cdc;

            if (watchdog_alarm_cdc != watchdog_alarm_d) begin
                // stop the kernel
                ctrl_tog_pattern    <= 'h0;
                ctrl_tog_upd        <= ~ctrl_tog_upd;
                ctrl_pwr_FF_en      <= 'b0;
                ctrl_pwr_DSP_en     <= 'b0;
                ctrl_pwr_BRAM_en    <= 'b0;
                ctrl_pwr_URAM_en    <= 'b0;
                ctrl_pwr_AIE_en     <= 'b0;
            end

            //delay the update for the pattern to cross clock domain
            ctrl_tog_upd_d1  <= ctrl_tog_upd;
            ctrl_tog_upd_d2  <= ctrl_tog_upd_d1;
            ctrl_tog_upd_d3  <= ctrl_tog_upd_d2;
            ctrl_tog_upd_d4  <= ctrl_tog_upd_d3;
            ctrl_tog_update  <= ctrl_tog_upd_d4;

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
                        case (addr_2_0)
                            // default : begin
                            3'b000 : begin // 0x0
                                if (we) begin
                                    ctrl_tog_pattern <= wdata[ 9: 0];
                                    ctrl_tog_upd     <= ~ctrl_tog_upd;  // invert to signal "new config available"
                                    ctrl_pwr_FF_en   <= wdata[16];
                                    ctrl_pwr_DSP_en  <= wdata[17];
                                    ctrl_pwr_BRAM_en <= wdata[18];
                                    ctrl_pwr_URAM_en <= wdata[19];
                                    ctrl_pwr_AIE_en  <= wdata[20];
                                end

                                rdata[ 9: 0]     <= ctrl_tog_pattern;
                                rdata[ 16  ]     <= ctrl_pwr_FF_en;
                                rdata[ 17  ]     <= ctrl_pwr_DSP_en;
                                rdata[ 18  ]     <= ctrl_pwr_BRAM_en;
                                rdata[ 19  ]     <= ctrl_pwr_URAM_en;
                                rdata[ 20  ]     <= ctrl_pwr_AIE_en;

                                cmd_cmplt     <= 'h1;
                            end
                            3'b001 : begin
                                rdata       <= m_aie0_cycle_cnt;
                                cmd_cmplt   <= 'h1;
                            end
                            3'b010 : begin
                                rdata       <= m_aie0_tready_cnt;
                                cmd_cmplt   <= 'h1;
                            end
                            3'b011 : begin
                                rdata       <= m_aie0_txfer_cnt;
                                cmd_cmplt   <= 'h1;
                            end
                            default : begin
                                rdata       <= s_aie0_txfer_cnt;
                                cmd_cmplt   <= 'h1;
                            end
                        endcase
                    end
                endcase
            end
        end
    end

endmodule : powertest_reg_array
`default_nettype wire
