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

module build_info_v4_0 # (
    parameter integer C_MAJOR_VERSION     = '0,
    parameter integer C_MINOR_VERSION     = '0,
    parameter integer C_BUILD_VERSION     = '0,
    parameter integer C_BLOCK_ID          = '0
) (
    input  wire Clk,
    input  wire Rst,

    input  wire [15:0] Info_1,
    input  wire [15:0] Info_2,
    input  wire [15:0] Info_3,
    input  wire [15:0] Info_4,
    input  wire [15:0] Info_5,
    input  wire [15:0] Info_6,
    input  wire [15:0] Info_7,
    input  wire [15:0] Info_8,

    input  wire        We,
    input  wire [ 2:0] Addr,
    input  wire [31:0] Data_In,
    output logic [31:0] Data_Out
);

always_comb begin
    Data_Out = 'h0;
    case (Addr)
        3'b000:   Data_Out      = {C_MAJOR_VERSION[15:0], C_MINOR_VERSION[15:0]};
        3'b001:   Data_Out      = C_BUILD_VERSION[31:0];
        3'b010:   Data_Out      = C_BLOCK_ID[31:0];
        3'b011:   Data_Out      = 'h0;
        3'b100:   Data_Out      = {Info_2, Info_1};
        3'b101:   Data_Out      = {Info_4, Info_3};
        3'b110:   Data_Out      = {Info_6, Info_5};
        3'b111:   Data_Out      = {Info_8, Info_7};
        default : Data_Out      = 'h0;
    endcase
end

endmodule
