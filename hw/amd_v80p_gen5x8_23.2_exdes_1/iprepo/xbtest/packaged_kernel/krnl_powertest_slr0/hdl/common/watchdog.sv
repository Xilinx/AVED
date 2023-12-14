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

module watchdog #(
    parameter integer C_CLOCK_FREQ          = 300, // Frequency in MHz of the watchdog clock
    parameter integer C_EXT_TOGGLE_1_SEC    = 0  // 1: use toggle_1_sec input, 0: generate internally

) (
    input  wire         clk_cont,
    input  wire         clk,
    input  wire         rst,
    input  wire         enable,
    input  wire [1:0]   cfg,
    input  wire         toggle_1sec,
    input  wire         alive,
    output logic        alarm  // pulse
);

    wire        sec_toggle;

    logic       sec_toggle_d1;
    logic       toggle_sec;
    logic [7:0] watchdog_cnt = 'h0;

    logic cnt_alarm;

generate
    if (C_EXT_TOGGLE_1_SEC == 0) begin : internal_toggle
        timer #(
            .C_CLOCK_FREQ       ( C_CLOCK_FREQ  ),
            .C_1_SEC_TIMER_EN   ( 1             )
        ) u_timer (
            .clk_1              ( clk_cont      ),
            .clk_2              ( clk           ),

            .toggle_1_sec_1     (               ),
            .toggle_1_sec_2     ( sec_toggle    ),

            .timer_rst          ( 1'b0          ),
            .timer_end_1        (               ),
            .timer_end_2        (               )
        );
    end else begin : external_toggle
        assign sec_toggle = toggle_1sec;
    end
endgenerate

always_comb begin
    cnt_alarm = 'h0;
    case (cfg)
        2'b00:  cnt_alarm = watchdog_cnt[4];
        2'b01:  cnt_alarm = watchdog_cnt[5];
        2'b10:  cnt_alarm = watchdog_cnt[6];
        2'b11:  cnt_alarm = watchdog_cnt[7];
        default: cnt_alarm = watchdog_cnt[4];
    endcase
end

always_ff @(posedge clk) begin

    if (rst) begin

        toggle_sec      <= 'b0;
        watchdog_cnt    <= 'h0;
        alarm           <= 'b0;

    end else begin

        sec_toggle_d1 <= sec_toggle;

        // detect any edge
        toggle_sec <= 'b0;
        if (sec_toggle != sec_toggle_d1) begin

            toggle_sec <= 'b1;

        end

        // watchdog only active if enable is set
        alarm <= 'b0;
        if (enable) begin

            if (toggle_sec) begin

                if (cnt_alarm) begin

                    // the alarm pulse will be repeated every second
                    alarm <= 'b1;

                end else begin

                    watchdog_cnt <= watchdog_cnt + 1;

                end

            end

        end else begin

            watchdog_cnt <= 'h0;

        end

        if (alive) begin

            watchdog_cnt <= 'h0;

        end

    end
end

endmodule: watchdog
`default_nettype wire
