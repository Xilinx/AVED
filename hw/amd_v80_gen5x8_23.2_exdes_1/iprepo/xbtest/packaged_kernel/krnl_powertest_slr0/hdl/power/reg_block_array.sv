
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

module reg_block_array #(
    parameter integer C_THROTTLE_MODE   = 1, // 1: clock throttling with BUFG, 0: throttle clock enable of sites FF input oscillator
    parameter integer N_SLICES          = 2
) (
    input  wire        clk,
    input  wire        reset,
    input  wire        enable,
    input  wire        tog_en
);

    generate
        if (N_SLICES > 0) begin

            wire ff_en;
            if (C_THROTTLE_MODE == 1) begin
                assign ff_en = enable;
            end else begin
                assign ff_en = tog_en & enable;
            end

            (*dont_touch ="true"*) wire [15:0] wire1 [N_SLICES-1:0];
            for (genvar ii = 0; ii < N_SLICES; ii++) begin : loop_slice_flop
                if (ii == 0) begin
                    // The FFs of one slice are oscillator. Outputs are driving the other FF of the power CU
                    for (genvar jj = 0; jj < 16; jj++) begin : genblk_fdre
                        FDRE #(
                            .INIT         ( 0    ), // Initial value of register, 1'b0, 1'b1
                            .IS_C_INVERTED( 1'b0 ), // Optional inversion for C
                            .IS_D_INVERTED( 1'b0 ), // Optional inversion for D
                            .IS_R_INVERTED( 1'b0 )  // Optional inversion for R
                        ) FDRE_inst (
                            .C  ( clk               ),  // 1-bit input: Clock
                            .R  ( reset             ),  // 1-bit input: Synchronous reset
                            .CE ( ff_en             ),  // 1-bit input: Clock enable
                            .D  ( ~wire1[ii][jj]    ),  // 1-bit input: Data
                            .Q  ( wire1[ii][jj]     )   // 1-bit output: Data
                        );
                    end: genblk_fdre

                end else begin
                    // Other FF of the other slices of the CR are chained, the floorplan of the chain is defined in gen_power_floorplan.tcl
                    for (genvar jj = 0; jj < 16; jj++) begin : genblk_fdre
                        // (*dont_touch ="true"*) wire lut_inv;
                        // // instantiate directly the LUT to be able to define a LOC constraint
                        // LUT1 #(
                        //     .INIT(2'h1) // INV
                        // )
                        // LUT_inst (
                        //     .O  (lut_inv),
                        //     .I0 (wire1[ii][jj])
                        // );

                        FDRE #(
                            .INIT         ( 0    ), // Initial value of register, 1'b0, 1'b1
                            .IS_C_INVERTED( 1'b0 ), // Optional inversion for C
                            .IS_D_INVERTED( 1'b0 ), // Optional inversion for D
                            .IS_R_INVERTED( 1'b0 )  // Optional inversion for R
                        ) FDRE_inst (
                            .C  ( clk               ),  // 1-bit input: Clock
                            .R  ( reset             ),  // 1-bit input: Synchronous reset
                            .CE ( enable            ),  // 1-bit input: Clock enable
                            // .D  ( lut_inv           ),  // 1-bit input: Data
                            .D  ( wire1[ii-1][jj]   ),  // 1-bit input: Data
                            .Q  ( wire1[ii][jj]     )   // 1-bit output: Data
                        );
                    end: genblk_fdre
                end
            end: loop_slice_flop
        end
    endgenerate


/*
generate
    if (N_SLICES > 0) begin

        (*dont_touch ="true"*) wire [15:0] wire1 [N_SLICES:0];
        (*dont_touch ="true"*) logic tog_en_d [4:0];

        always_ff @(posedge clk) begin
            tog_en_d[0] <= tog_en & enable;
            tog_en_d[1] <= tog_en_d[0];
            tog_en_d[2] <= tog_en_d[1];
            tog_en_d[3] <= tog_en_d[2];
            tog_en_d[4] <= tog_en_d[3];
        end

        for (genvar ii = 0; ii < N_SLICES; ii++) begin : loop_slice_flop

            // the first FF inverts its ouput when it's enabled
            // all other FF of the chain repeats the output of the previous FF
            if (ii == 0) begin
                for (genvar jj = 0; jj < 16; jj++) begin : genblk_fdre
                    FDRE #(
                        .INIT         ( 0    ),
                        .IS_C_INVERTED( 1'b0 ),
                        .IS_D_INVERTED( 1'b0 ),
                        .IS_R_INVERTED( 1'b0 )
                    ) FDRE_inst (
                        .C  ( clk               ),
                        .R  ( reset             ),
                        .CE ( tog_en_d[4]       ),
                        .D  ( ~wire1[ii][jj]    ),
                        .Q  ( wire1[ii][jj]     )
                    );
                end: genblk_fdre

            end else begin

                for (genvar jj = 0; jj < 16; jj++) begin : genblk_fdre

                    //  (*dont_touch ="true"*) wire lut_xor;
                    //
                    //  // instantiate directly the LUT as a XOR to be able to define a LOC constraint
                    //  LUT2 #(
                    //      .INIT(4'h6) // XOR : O=I0 & !I1 + !I0 & I1
                    //  )
                    //  LUT2_inst (
                    //      .O  (lut_xor),
                    //      .I0 (wire1[ii][jj]),
                    //      .I1 (wire1[ii-1][jj])
                    //  );

                    FDRE #(
                        .INIT         ( 0    ),
                        .IS_C_INVERTED( 1'b0 ),
                        .IS_D_INVERTED( 1'b0 ),
                        .IS_R_INVERTED( 1'b0 )
                    ) FDRE_inst (
                        .C  ( clk               ),
                        .R  ( reset             ),
                        .CE ( enable            ),
                        .D  ( wire1[ii-1][jj]  ),
                        // .D  ( lut_xor  ),
                        .Q  ( wire1[ii][jj]     )
                    );
                end: genblk_fdre
            end

        end: loop_slice_flop
    end
endgenerate
*/

endmodule : reg_block_array
