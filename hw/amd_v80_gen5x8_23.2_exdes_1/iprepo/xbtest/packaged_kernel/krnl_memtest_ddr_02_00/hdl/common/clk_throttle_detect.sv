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

module clk_throttle_detect #(
    parameter integer DEST_SYNC_FF  = 4
) (
    input  wire     clk,
    input  wire     clk_cont,

    // on clk domain
    input  wire     enable,
    input  wire     alarm_clear_tog,
    output wire     alarm_latch
);

timeunit 1ps;
timeprecision 1ps;

wire  enable_cdc_cont;

logic [6:0] cont_counter = 'h0;
logic [6:0] counter      = 'h0;
wire  [6:0] counter_cdc ;
logic [6:0] cnt_diff     = 'h0;

logic cont_counter_msb_d    = 'h0;
logic request_tog           = 'h0;
wire  request_tog_cdc;
logic request_tog_cdc_d = 'h0;

logic cnt_send = 'h0;
wire  cnt_rcv;
logic cnt_rcv_d = 'h0;

wire alarm_ack;
logic alarm_ack_d = 'h0;
logic throttle_alarm = 'h0;

logic dbg_alarm_live ='h0;

/////////////////////////////////////////////////////////////////////////////
// Begin RTL
/////////////////////////////////////////////////////////////////////////////

always_ff @(posedge clk) begin

    counter <= counter + 1;
    cnt_send <= 'b0;

    request_tog_cdc_d <= request_tog_cdc;
    if (request_tog_cdc_d ^ request_tog_cdc) begin
        cnt_send <= 'b1;
    end

    if (~enable) begin
        counter <= 'h0;
    end

end

xpm_cdc_single #(
    .DEST_SYNC_FF   ( DEST_SYNC_FF ),
    .INIT_SYNC_FF   ( 0            ),
    .SRC_INPUT_REG  ( 0            ),
    .SIM_ASSERT_CHK ( 0            )
)
u_cdc_enable_cont (
    .src_clk  ( clk             ),
    .src_in   ( enable          ),
    .dest_clk ( clk_cont        ),
    .dest_out ( enable_cdc_cont )
);

always_ff @(posedge clk_cont) begin

    cont_counter <= cont_counter + 1;

    cnt_rcv_d <= cnt_rcv;
    alarm_ack_d <= alarm_ack;

    cont_counter_msb_d <= cont_counter[6];

    // falling edge of the counter, request
    if (cont_counter_msb_d & ~cont_counter[6]) begin
        request_tog  <= ~request_tog;
    end

    if (cnt_rcv) begin
        cnt_diff  <= cont_counter - counter_cdc;
    end

    if (alarm_ack ^ alarm_ack_d) begin
        throttle_alarm <= 'h0;
    end

    dbg_alarm_live <= 'h0;

    if (cnt_rcv_d) begin
        if (cnt_diff[6]) begin
            throttle_alarm <= 'h1;
            dbg_alarm_live <= 'h1;
        end
    end

    if (~enable_cdc_cont) begin
        cont_counter        <= 'h0;
        cont_counter_msb_d  <= 'h0;
        cnt_diff            <= 'h0;
        throttle_alarm      <= 'h0;
    end
end

    cdc_static_bus #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF ),
        .WIDTH          (7)
    )
    u_cdc_counter (
        .src_clk        (clk           ),
        .src_send       (cnt_send      ),
        .src_in         (counter       ),

        .dest_clk       (clk_cont      ),
        .dest_ack       (cnt_rcv       ),
        .dest_out       (counter_cdc   )
    );

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF ),
        .INIT_SYNC_FF   ( 0            ),
        .SRC_INPUT_REG  ( 0            ),
        .SIM_ASSERT_CHK ( 0            )
    )
    u_cdc_req_tog (
        .src_clk  ( clk_cont        ),
        .src_in   ( request_tog     ),
        .dest_clk ( clk             ),
        .dest_out ( request_tog_cdc )
    );

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF ),
        .INIT_SYNC_FF   ( 0            ),
        .SRC_INPUT_REG  ( 0            ),
        .SIM_ASSERT_CHK ( 0            )
    )
    u_cdc_ack (
        .src_clk  ( clk              ),
        .src_in   ( alarm_clear_tog  ),
        .dest_clk ( clk_cont         ),
        .dest_out ( alarm_ack        )
    );


    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF ),
        .INIT_SYNC_FF   ( 0            ),
        .SRC_INPUT_REG  ( 0            ),
        .SIM_ASSERT_CHK ( 0            )
    )
    u_cdc_alarm (
        .src_clk  ( clk_cont        ),
        .src_in   ( throttle_alarm  ),
        .dest_clk ( clk             ),
        .dest_out ( alarm_latch     )
    );


endmodule : clk_throttle_detect
`default_nettype wire