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

module verify_reg_array #(
    parameter integer C_MAJOR_VERSION       = 0,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer C_DNA_READ            = 0,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_BLOCK_ID            = 0
) (
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         ap_clk_2,
    input  wire         ap_clk_2_cont,
    input  wire         ap_rst_2,
    input  wire         cs,
    input  wire         we,
    input  wire  [11:0] addr,
    input  wire  [31:0] wdata,
    output logic [31:0] rdata           = 'h0,
    output logic        cmd_cmplt       = 'h0,
    output wire         watchdog_alarm_latch,

    output logic        dna_read_rq_tog = 'h0,
    input  wire         dna_rd_done_tog,
    input  wire  [95:0] dna
);

    wire [2:0]  addr_2_0    = addr[2:0];
    wire        addr_5      = addr[5];
    wire        addr_4      = addr[4];
    wire [1:0]  addr_5_4    = addr[5:4];
    wire        we_bi           = (addr_5_4 == 2'b00) ? cs & we : 'b0;
    wire        we_ctrl_status  = (addr_5_4 == 2'b01) ? cs & we : 'b0;
    wire [31:0] rdata_bi;
    wire [31:0] rdata_ctrl_status;

    wire   watchdog_alarm;

    logic  dna_rd_done_tog_d1;
    logic  dna_rd_done_latch;


    build_info_v4_0 #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
        .C_BLOCK_ID         ( C_BLOCK_ID            )
    ) u_build_info (
        .Clk        ( ap_clk                        ),
        .Rst        ( ap_rst                        ),

        .Info_1     ( 16'b0                         ),  // Info 1 reserved for future use
        .Info_2     ( 16'b0                         ),
        .Info_3     ( 16'b0                         ),
        .Info_4     ( 16'b0                         ),
        .Info_5     ( 16'b0                         ),
        .Info_6     ( 16'b0                         ),
        .Info_7     ( {7'b0, C_CLOCK0_FREQ[8:0]}    ),
        .Info_8     ( {7'b0, C_CLOCK1_FREQ[8:0]}    ),

        .We         ( we_bi                         ),
        .Addr       ( addr[2:0]                     ),
        .Data_In    ( wdata                         ),
        .Data_Out   ( rdata_bi                      )
    );

    common_ctrl_status #(
        .C_CLOCK_FREQ               ( C_CLOCK0_FREQ ),
        .DEST_SYNC_FF               ( DEST_SYNC_FF  ),
        .C_CLK_TROTTLE_DETECT_EN    ( 2             ),
        .C_WATCHDOG_ENABLE          ( 1             ),
        .C_EXT_TOGGLE_1_SEC         ( 0             )
    ) u_common_ctrl_status (
        .ap_clk         ( ap_clk            ),
        .ap_clk_cont    ( ap_clk_cont       ),
        .ap_rst         ( ap_rst            ),
        .ap_clk_2       ( ap_clk_2          ),
        .ap_clk_2_cont  ( ap_clk_2_cont     ),
        .ap_rst_2       ( ap_rst_2          ),

        .toggle_1sec            ( 1'b0                  ),
        .rst_watchdog           ( cs                    ),
        .watchdog_alarm         ( watchdog_alarm_latch  ),

        .We             ( we_ctrl_status    ),
        .Addr           ( addr[2:0]         ),
        .Data_In        ( wdata             ),
        .User_Status_1  ( 32'b0             ),
        .Data_Out       ( rdata_ctrl_status )
    );

    //########################################
    //### Write registers
    //########################################
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin

            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;
            dna_rd_done_latch <= 'h0;

        end else begin
            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            dna_rd_done_tog_d1 <= dna_rd_done_tog;

            if (dna_rd_done_tog != dna_rd_done_tog_d1)  begin
                dna_rd_done_latch <= 'h1;
            end

            if (cs) begin

                case (addr_5)
                    'h0 : begin
                        if (addr_4) begin
                            // common ctrl and status
                            rdata       <= rdata_ctrl_status;
                        end else begin
                            // build info
                            rdata       <= rdata_bi;
                        end
                        cmd_cmplt   <= 'h1;
                    end
                    default : begin
                        case (addr_2_0)
                            // default : begin
                            3'b000 : begin // 0x0

                                if (we) begin
                                    if (wdata[1]) begin
                                        dna_read_rq_tog     <= ~dna_read_rq_tog;
                                        dna_rd_done_latch   <= 'h0;
                                    end
                                end

                                rdata[0] <= C_DNA_READ;
                                // rdata[1] <= not used ;
                                rdata[2] <= dna_rd_done_latch;

                                cmd_cmplt     <= 'h1;
                            end
                            3'b001 : begin
                                rdata <= dna [31:0];
                                cmd_cmplt   <= 'h1;
                            end
                            3'b010 : begin
                                rdata <= dna [63:32];
                                cmd_cmplt   <= 'h1;
                            end
                            3'b011 : begin
                                rdata <= dna [95:64];
                                cmd_cmplt   <= 'h1;
                            end
                            default : begin
                                rdata       <= 'h0;
                                cmd_cmplt   <= 'h1;
                            end
                        endcase
                    end
                endcase
            end
        end
    end

endmodule : verify_reg_array
`default_nettype wire
