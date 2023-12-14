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

module gt_lpbk_25_reg_array #(
    parameter integer C_MAJOR_VERSION,
    parameter integer C_MINOR_VERSION,
    parameter integer C_BUILD_VERSION,
    parameter integer C_CLOCK0_FREQ,
    parameter integer C_CLOCK1_FREQ,
    parameter integer C_GT_INDEX,
    parameter integer DEST_SYNC_FF,
    parameter integer C_BLOCK_ID
) (
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,

    output wire [3 : 0]    rxlpmen      ,
    output wire [3 : 0]    txpolarity   ,
    output wire [19 : 0]   txdiffctrl   ,
    output wire [19 : 0]   txpostcursor ,
    output wire [19 : 0]   txprecursor  ,
    output logic           gt_reset     ,

    input  wire         cs,
    input  wire         we,
    input  wire  [11:0] addr,
    input  wire  [31:0] wdata,
    output logic [31:0] rdata           = 'h0,
    output logic        cmd_cmplt       = 'h0
);


    wire [2:0]  addr_2_0    = addr[2:0];
    wire        addr_5      = addr[5];
    wire        addr_4      = addr[4];
    wire        addr_5_4    = addr[5:4];
    wire        we_bi           = (addr_5_4 == 2'b00) ? cs & we : 'b0;
    wire        we_ctrl_status  = (addr_5_4 == 2'b01) ? cs & we : 'b0;
    wire [31:0] rdata_bi;
    wire [31:0] rdata_ctrl_status;

    wire   watchdog_alarm;

    build_info_v4_0 #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
        .C_BLOCK_ID         ( C_BLOCK_ID            )
    ) u_build_info (
        .Clk        ( ap_clk                        ),
        .Rst        ( ap_rst                        ),
        .Info_1     ( 16'b0                         ),  // Info 1 reserved for future use
        .Info_2     ( {C_GT_INDEX[7:0], 8'b0}       ),
        .Info_3     ( 16'h0                         ),
        .Info_4     ( 16'h0                         ),
        .Info_5     ( 16'h0                         ),
        .Info_6     ( 16'h0                         ),
        .Info_7     ( C_CLOCK0_FREQ[15:0]           ),
        .Info_8     ( C_CLOCK1_FREQ[15:0]           ),

        .We         ( we_bi                         ),
        .Addr       ( addr[2:0]                     ),
        .Data_In    ( wdata                         ),
        .Data_Out   ( rdata_bi                      )
    );

    common_ctrl_status #(
        .C_CLOCK_FREQ               ( C_CLOCK0_FREQ ),
        .DEST_SYNC_FF               ( DEST_SYNC_FF  ),
        .C_CLK_TROTTLE_DETECT_EN    ( 0             ),
        .C_WATCHDOG_ENABLE          ( 0             ),
        .C_EXT_TOGGLE_1_SEC         ( 0             )  // 1: use toggle_1_sec input, 0: generate internally
    ) u_common_ctrl_status (
        .ap_clk         ( ap_clk            ),
        .ap_clk_cont    ( ap_clk_cont       ),
        .ap_rst         ( ap_rst            ),

        .ap_clk_2       ( 1'b0              ),
        .ap_clk_2_cont  ( 1'b0              ),
        .ap_rst_2       ( 1'b0              ),

        .toggle_1sec    ( 1'b0              ),
        .rst_watchdog   ( 1'b0              ),
        .watchdog_alarm (                   ),

        .We             ( we_ctrl_status    ),
        .Addr           ( addr[2:0]         ),
        .Data_In        ( wdata             ),
        .User_Status_1  ( 32'b0             ),
        .Data_Out       ( rdata_ctrl_status )
    );


    logic [3:0]  gt_rxlpmen;
    logic [3:0]  gt_txpolarity;
    logic [4:0]  gt_txpostcursor [3:0];
    logic [4:0]  gt_txprecursor  [3:0];
    logic [4:0]  gt_txdiffctrl   [3:0];

    wire        lane_rxlpmen;
    wire        lane_txpolarity;
    wire [4:0]  lane_txpostcursor;
    wire [4:0]  lane_txprecursor ;
    wire [4:0]  lane_txdiffctrl  ;

    assign lane_rxlpmen      = wdata[24];
    assign lane_txpolarity   = wdata[22];
    assign lane_txpostcursor = wdata[21:17];
    assign lane_txprecursor  = wdata[9:5];
    assign lane_txdiffctrl   = wdata[4:0];

    assign rxlpmen = {gt_rxlpmen[3],gt_rxlpmen[2],gt_rxlpmen[1],gt_rxlpmen[0] };
    assign txpolarity = {gt_txpolarity[3],gt_txpolarity[2],gt_txpolarity[1],gt_txpolarity[0] };
    assign txdiffctrl = {gt_txdiffctrl[3],gt_txdiffctrl[2],gt_txdiffctrl[1],gt_txdiffctrl[0] };
    assign txpostcursor = {gt_txpostcursor[3],gt_txpostcursor[2],gt_txpostcursor[1],gt_txpostcursor[0] };
    assign txprecursor = {gt_txprecursor[3],gt_txprecursor[2],gt_txprecursor[1],gt_txprecursor[0] };

    wire [31:0]  cfg_lane [3:0];                                                       // txmaincursor
    assign cfg_lane[0] = {7'b0, gt_rxlpmen[0], 1'b0, gt_txpolarity[0], gt_txpostcursor[0], 7'b0, gt_txprecursor[0], gt_txdiffctrl[0]} ;
    assign cfg_lane[1] = {7'b0, gt_rxlpmen[1], 1'b0, gt_txpolarity[1], gt_txpostcursor[1], 7'b0, gt_txprecursor[1], gt_txdiffctrl[1]} ;
    assign cfg_lane[2] = {7'b0, gt_rxlpmen[2], 1'b0, gt_txpolarity[2], gt_txpostcursor[2], 7'b0, gt_txprecursor[2], gt_txdiffctrl[2]} ;
    assign cfg_lane[3] = {7'b0, gt_rxlpmen[3], 1'b0, gt_txpolarity[3], gt_txpostcursor[3], 7'b0, gt_txprecursor[3], gt_txdiffctrl[3]} ;

    //########################################
    //### registers
    //########################################
    always_ff @(posedge ap_clk) begin
        if (ap_rst) begin
            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            gt_reset       <= 'h0;

            gt_rxlpmen      <= 'h0;
            gt_txpolarity   <= 'h0;
            gt_txpostcursor <= '{default:'0};
            gt_txprecursor  <= '{default:'0};
            gt_txdiffctrl   <= '{default:'0};

        end else begin

            rdata       <= 'h0;
            cmd_cmplt   <= 'h0;

            gt_reset    <= 'h0;

            if (watchdog_alarm == 1'b1) begin
                // nothing to stop so far
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
                                    gt_reset <='h1;
                                end

                                rdata       <= 'h0;
                                cmd_cmplt   <= 'h1;
                            end
                            3'b001 : begin
                                if (we) begin
                                    gt_rxlpmen[0]       <= lane_rxlpmen;
                                    gt_txpolarity[0]    <= lane_txpolarity;
                                    gt_txpostcursor[0]  <= lane_txpostcursor;
                                    gt_txprecursor[0]   <= lane_txprecursor ;
                                    gt_txdiffctrl[0]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[0];
                                cmd_cmplt   <= 'h1;
                            end
                            3'b010 : begin
                                if (we) begin
                                    gt_rxlpmen[1]       <= lane_rxlpmen;
                                    gt_txpolarity[1]    <= lane_txpolarity;
                                    gt_txpostcursor[1]  <= lane_txpostcursor;
                                    gt_txprecursor[1]   <= lane_txprecursor ;
                                    gt_txdiffctrl[1]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[1];
                                cmd_cmplt   <= 'h1;
                            end
                            3'b011 : begin
                                if (we) begin
                                    gt_rxlpmen[2]       <= lane_rxlpmen;
                                    gt_txpolarity[2]    <= lane_txpolarity;
                                    gt_txpostcursor[2]  <= lane_txpostcursor;
                                    gt_txprecursor[2]   <= lane_txprecursor ;
                                    gt_txdiffctrl[2]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[2];
                                cmd_cmplt   <= 'h1;
                            end
                            3'b100 : begin
                                if (we) begin
                                    gt_rxlpmen[3]       <= lane_rxlpmen;
                                    gt_txpolarity[3]    <= lane_txpolarity;
                                    gt_txpostcursor[3]  <= lane_txpostcursor;
                                    gt_txprecursor[3]   <= lane_txprecursor ;
                                    gt_txdiffctrl[3]    <= lane_txdiffctrl  ;
                                end

                                rdata       <= cfg_lane[3];
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

endmodule : gt_lpbk_25_reg_array
`default_nettype wire
