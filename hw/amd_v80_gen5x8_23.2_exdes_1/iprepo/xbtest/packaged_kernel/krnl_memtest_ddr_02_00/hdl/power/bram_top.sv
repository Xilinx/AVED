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

module bram_top #(
    parameter         C_FAMILY                   = "no_family",
    parameter integer C_THROTTLE_MODE            = 1, // 1: clock throttling with BUFG, 0: throttle clock enable of sites FF input oscillator
    parameter integer C_COL_SIZES_ARRRAY [0:199],
    parameter integer DISABLE_SIM_ASSERT    = 0
) (
    input wire clk,
    input wire reset,
    input wire enable,
    input wire tog_en
);

    generate
        for (genvar ii = 0; ii < 200; ii++) begin : genblk_col
            bram_col #(
                .C_FAMILY           ( C_FAMILY                  ),
                .C_THROTTLE_MODE    ( C_THROTTLE_MODE           ),
                .N_BRAM             ( C_COL_SIZES_ARRRAY[ii]    ),
                .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT        )
            ) bram_col_inst (
                .clk    ( clk       ),
                .reset  ( reset     ),
                .enable ( enable    ),
                .tog_en ( tog_en    )
            );
        end : genblk_col
    endgenerate

endmodule: bram_top
`default_nettype wire
