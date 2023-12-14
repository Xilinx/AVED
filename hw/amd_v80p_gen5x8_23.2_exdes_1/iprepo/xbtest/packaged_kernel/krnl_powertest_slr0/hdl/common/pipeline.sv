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

module pipeline #(
    parameter integer C_NUM_PIPELINE = 0,
    parameter integer C_DATA_WIDTH   = 1
) (
    input  wire                    clk,
    input  wire                    rst,
    input  wire [C_DATA_WIDTH-1:0] data_in,
    output wire [C_DATA_WIDTH-1:0] data_out
);

    generate
        if (C_NUM_PIPELINE == 0) begin : gen_no_pipe
            assign data_out = data_in;
        end : gen_no_pipe
        if (C_NUM_PIPELINE > 0) begin : gen_pipe
            // Use dont_touch attribute to prevent use of SRL
            (*dont_touch ="true"*) logic [C_DATA_WIDTH-1:0] pipe [C_NUM_PIPELINE:1] = '{C_NUM_PIPELINE{'b0}};
            always_ff @(posedge clk)
            begin
                if (rst) begin
                    pipe <= '{C_NUM_PIPELINE{'b0}};
                end else begin
                    pipe <= {pipe[C_NUM_PIPELINE-1:1], data_in};
                end
            end
            assign data_out = pipe[C_NUM_PIPELINE];
        end : gen_pipe
    endgenerate

endmodule: pipeline
`default_nettype wire
