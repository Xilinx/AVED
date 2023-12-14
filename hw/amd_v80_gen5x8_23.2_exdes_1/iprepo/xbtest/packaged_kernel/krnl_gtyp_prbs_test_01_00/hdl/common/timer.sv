
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

module timer # (
    parameter integer C_CLOCK_FREQ          = 300,      // Frequency in MHz of the timer clock
    parameter integer C_TIMER_RELOAD        = 2999999,  // Example value for a 10ms timer at 300MHz
    parameter integer C_1_SEC_TIMER_EN      = 0,        // 1: Enable 1 sec timer; 0: disable
    parameter integer C_USER_TIMER_EN       = 0,        // 1: Enable User-defined timer; 0: disable
    parameter integer C_CLK_2_EN            = 1,        // 1: Enable clk_2 CDC; 0: disable
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_SIM_DIVIDER         = 1
) (
    input  wire         clk_1,
    input  wire         clk_2,

    // 1 sec timer
    output wire         toggle_1_sec_1,
    output wire         toggle_1_sec_2,

    // User-defined timer
    input  wire         timer_rst,
    output wire         timer_end_1,
    output wire         timer_end_2  // Extra cycles to recreate pulse in clk_2 domain.
);

// 100*10ms timer, it requires C_CLOCK_FREQ * 1.000.000 clock cycles at C_CLOCK_FREQ MHz
//with clk_1 @ 300Mhz, 300.000.000 cycles per second, it's below < 2^29. Use 30 bits counter to catch the carry (at C_1S_TIMER_SIZE bit)
localparam integer C_1S_TIMER_RELOAD = C_CLOCK_FREQ * 1000000 - 1;
localparam integer C_1S_TIMER_SIZE   = $clog2(C_1S_TIMER_RELOAD);

localparam integer C_TIMER_SIZE = $clog2(C_TIMER_RELOAD);

logic [C_1S_TIMER_SIZE:0]   timer_1sec_cnt = 0; // add 1 extra bit to catch roll-over
logic [31:0]                toggle_1_sec_cnt = 0;

logic [C_TIMER_SIZE:0]      timer_cnt = 0; // add 1 extra bit to catch roll-over
logic                       timer_cnt_ovfl_1 = 0;

/////////////////////////////////////////////////////////////////////////////
// Begin RTL
/////////////////////////////////////////////////////////////////////////////

generate
    if (C_1_SEC_TIMER_EN == 1) begin : timer_1_sec
        /////////////////////////////////////////////////////////////////////////////
        // 1 sec timer
        /////////////////////////////////////////////////////////////////////////////
        (*dont_touch ="true"*) logic toggle_1_sec_1_int = 1'b0;
        assign toggle_1_sec_1 = toggle_1_sec_1_int;

        always_ff @(posedge clk_1) begin
            if (timer_1sec_cnt[C_1S_TIMER_SIZE]) begin
                toggle_1_sec_1_int  <= ~toggle_1_sec_1_int;

                timer_1sec_cnt      <= C_1S_TIMER_RELOAD/C_SIM_DIVIDER;
            end else begin
                timer_1sec_cnt      <= timer_1sec_cnt - 1;
            end
        end
        if (C_CLK_2_EN == 1) begin : timer_1_sec_cdc
            xpm_cdc_single #(
                .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
                .INIT_SYNC_FF   ( 0             ),
                .SRC_INPUT_REG  ( 1             ),
                .SIM_ASSERT_CHK ( 0             )
            ) xpm_cdc_toggle_1_sec (
                .src_clk  ( clk_1               ),
                .src_in   ( toggle_1_sec_1_int  ),
                .dest_clk ( clk_2               ),
                .dest_out ( toggle_1_sec_2      )
            );
        end : timer_1_sec_cdc
    end : timer_1_sec
endgenerate

generate
    if (C_USER_TIMER_EN == 1) begin : timer_user
        /////////////////////////////////////////////////////////////////////////////
        // User-defined timer
        /////////////////////////////////////////////////////////////////////////////
        (*dont_touch ="true"*) logic timer_end_1_int = 1'b0;
        assign timer_end_1 = timer_end_1_int;

        always_ff @(posedge clk_1) begin
            timer_end_1_int <= 1'b0;
            if (timer_cnt[C_TIMER_SIZE] & ~timer_cnt_ovfl_1) begin
                timer_end_1_int   <= 1'b1;
                timer_cnt_ovfl_1  <= 1'b1;
            end else begin
                timer_cnt <= timer_cnt - 1;
            end

            if (timer_rst) begin
                timer_end_1_int     <= 1'b0; // keep previous value
                timer_cnt_ovfl_1    <= 1'b0;
                timer_cnt           <= C_TIMER_RELOAD/C_SIM_DIVIDER;
            end
        end
        if (C_CLK_2_EN == 1) begin : timer_user_cdc
           wire timer_cnt_ovfl_2;

            xpm_cdc_single #(
                .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
                .INIT_SYNC_FF   ( 0             ),
                .SRC_INPUT_REG  ( 1             ),
                .SIM_ASSERT_CHK ( 0             )
            ) xpm_cdc_timer_end (
                .src_clk  ( clk_1             ),
                .src_in   ( timer_cnt_ovfl_1  ),
                .dest_clk ( clk_2             ),
                .dest_out ( timer_cnt_ovfl_2  )
            );

           (*dont_touch ="true"*) logic timer_cnt_ovfl_2_d = 1'b0;
           (*dont_touch ="true"*) logic timer_end_2_int = 1'b0;
            assign timer_end_2 = timer_end_2_int;

            always_ff @(posedge clk_2) begin
                timer_cnt_ovfl_2_d <= timer_cnt_ovfl_2;
                timer_end_2_int    <= 1'b0;
                if (timer_cnt_ovfl_2 & ~timer_cnt_ovfl_2_d) begin
                    timer_end_2_int   <= 1'b1;
                end
            end

        end : timer_user_cdc
    end : timer_user
endgenerate

endmodule : timer
`default_nettype wire