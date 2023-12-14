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

module reg_chain_top #(
    parameter integer C_THROTTLE_MODE   = 1, // 1: clock throttling with BUFG, 0: throttle clock enable of sites FF input oscillator
    parameter integer C_REG_BLOCK_SIZE_ARRAY [0:19][0:19]
) (
    input  wire        clk,
    input  wire        reset,
    input  wire        enable,
    input  wire        tog_en
);

    generate
        for (genvar yy = 0; yy < 20; yy++) begin : CR_Y
            for (genvar xx = 0; xx < 20; xx++) begin : CR_X
                reg_block_array #(
                    .C_THROTTLE_MODE    ( C_THROTTLE_MODE                   ),
                    .N_SLICES           ( C_REG_BLOCK_SIZE_ARRAY[yy][xx]    )
                ) reg_block_array_inst (
                    .clk    ( clk       ),
                    .reset  ( reset     ),
                    .enable ( enable    ) ,
                    .tog_en ( tog_en    )
                );
            end
        end
    endgenerate

endmodule : reg_chain_top
`default_nettype wire
