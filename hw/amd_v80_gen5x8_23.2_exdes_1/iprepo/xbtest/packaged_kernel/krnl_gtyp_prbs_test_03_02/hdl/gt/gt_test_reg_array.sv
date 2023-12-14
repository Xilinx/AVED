
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

module gt_test_reg_array #(
    parameter integer C_MAJOR_VERSION,      // Major version
    parameter integer C_MINOR_VERSION,      // Minor version
    parameter integer C_BUILD_VERSION,      // Build version
    parameter integer C_BLOCK_ID,           // Block_ID (POWER = 0, MEMORY = 1, GT = 2,3)
    parameter integer C_GT_INDEX            // GT index 0 or 1

) (
    input  logic        clk,
    input  logic        rst,
    input  logic        cs,
    input  logic        we,
    input  logic [11:0] addr,
    input  logic [31:0] wdata,
    output logic [31:0] rdata                   = 'h0,
    output logic        cmd_cmplt               = 'h0,

    output logic [11:0] ctrl_gt_loopback        = 'h0,
    output logic        ctrl_gt_reset           = 'h0,

    output logic        ctrl_scramb_dis         = 1'b0,
    output logic        ctrl_retime_dis         = 1'b0,
    output logic        ctrl_align_dis          = 1'b0

);

    wire [1:0]  addr_1_0    = addr[1:0];
    wire        addr_4      = addr[4];
    wire        we_bi       = (addr_4 == 'h0) ? cs & we : 'h0;
    wire [31:0] rdata_bi;

    //########################################
    //### build info instantiation
    //########################################
    build_info_v3_0 #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
        .C_BLOCK_ID         ( C_BLOCK_ID            )
    ) powertest_build_info (
        .Clk        ( clk                       ),
        .Rst        ( rst                       ),
        .Info_1     ( 16'b0                     ),
        .Info_2     ( {C_GT_INDEX[7:0], 8'b0}   ),
        .Info_3     ( 16'h0                     ),
        .Info_4     ( 16'h0                     ),
        .Info_5     ( 16'h0                     ),
        .Info_6     ( 16'h0                     ),
        .We         ( we_bi                     ),
        .Addr       ( addr[2:0]                 ),
        .Data_In    ( wdata                     ),
        .Data_Out   ( rdata_bi                  )
    );

    //########################################
    //### Write registers
    //########################################
    always_ff @(posedge clk) begin
        if (rst) begin
            ctrl_gt_loopback    <= 'h0;
            ctrl_gt_reset       <= 'h0;

            ctrl_scramb_dis     <= 1'b0;
            ctrl_retime_dis     <= 1'b0;
            ctrl_align_dis      <= 1'b0;


            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

        end else begin
            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            if (cs) begin
                case (addr_4)
                    'h0 : begin
                        // build info
                        rdata       <= rdata_bi;
                        cmd_cmplt   <= 'h1;
                    end

                    default : begin
                        case (addr_1_0)

                            'h2 : begin
                                if (we) begin
                                    ctrl_gt_reset <= wdata[0];
                                end
                                rdata[0]  <= ctrl_gt_reset;
                                cmd_cmplt <= 'h1;
                            end

                            default : begin
                                if (we) begin
                                    ctrl_gt_loopback  <= wdata[11:0];

                                    ctrl_scramb_dis   <= wdata[20];
                                    ctrl_retime_dis   <= wdata[21];
                                    ctrl_align_dis    <= wdata[22];

                                end
                                rdata[11: 0]  <= ctrl_gt_loopback;

                                rdata[20]     <= ctrl_scramb_dis;
                                rdata[21]     <= ctrl_retime_dis;
                                rdata[22]     <= ctrl_align_dis;

                                cmd_cmplt <= 'h1;
                            end

                        endcase

                    end

                endcase
            end
        end
    end

endmodule : gt_test_reg_array
