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

module monitor_signal_r_f_edges  #(
    parameter integer WIDTH                 = 1,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer DISABLE_SIM_ASSERT    = 0
)(
    input  wire                     clk_in,
    input  wire [WIDTH-1 : 0]       sig_in,

    input  wire                     async_clear,

    output logic [WIDTH-1 : 0]      sig_r = '0,
    output logic [WIDTH-1 : 0]      sig_f = '0,

    input  wire                     clk_cdc,
    output logic [WIDTH-1 : 0]      sig_in_cdc,
    output logic [WIDTH-1 : 0]      sig_r_cdc,
    output logic [WIDTH-1 : 0]      sig_f_cdc

);
 //  this module detects rising and falling edge of sig_in,
 //  output _r or _f stay high until async_clear is toggled (from 1->0 or from 0->1)

    wire                    async_clear_cdc;
    logic                   async_clear_cdc_d;
    logic [WIDTH-1 : 0]     sig_in_d = '0;

    xpm_cdc_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF ),
        .INIT_SYNC_FF   ( 0            ),
        .SRC_INPUT_REG  ( 0            )
    )
    cdc_clear_rx (
        .src_clk  (  1'h0           ),
        .src_in   ( async_clear     ),
        .dest_out ( async_clear_cdc ),
        .dest_clk ( clk_in          )
    );

    always_ff @(posedge clk_in) begin
        async_clear_cdc_d <= async_clear_cdc;
    end

generate
    genvar i;
    for (i=0; i <= WIDTH-1; i=i+1) begin : reset_done

        always_ff @(posedge clk_in) begin
            sig_in_d[i] <= sig_in[i];

            if (async_clear_cdc_d != async_clear_cdc) begin
                sig_r[i]  <= 1'b0;
                sig_f[i]  <= 1'b0;
            end

            if ((sig_in_d[i]  == 1'b0) && (sig_in[i]  == 1'b1)) begin
                sig_r[i]  <= 1'b1;
            end
            if ((sig_in_d[i]  == 1'b1) && (sig_in[i]  == 1'b0)) begin
                sig_f[i]  <= 1'b1;
            end

        end
    end
endgenerate

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .WIDTH          ( WIDTH         )
    )
    cdc_sig_in (
        .src_clk  (  1'h0       ),
        .src_in   ( sig_in      ),
        .dest_out ( sig_in_cdc  ),
        .dest_clk ( clk_cdc     )
    );

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .WIDTH          ( WIDTH         )
    )
    cdc_sig_r (
        .src_clk  (  1'h0       ),
        .src_in   ( sig_r       ),
        .dest_out ( sig_r_cdc   ),
        .dest_clk ( clk_cdc     )
    );

    xpm_cdc_array_single #(
        .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
        .INIT_SYNC_FF   ( 0             ),
        .SRC_INPUT_REG  ( 0             ),
        .WIDTH          ( WIDTH         )
    )
    cdc_sig_f (
        .src_clk  (  1'h0       ),
        .src_in   ( sig_f       ),
        .dest_out ( sig_f_cdc   ),
        .dest_clk ( clk_cdc     )
    );

endmodule : monitor_signal_r_f_edges
`default_nettype wire
