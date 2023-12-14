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

module common_ctrl_status #(
    parameter integer C_CLOCK_FREQ              = 1, // Frequency in MHz of the common_ctrl_status clock
    parameter integer DEST_SYNC_FF              = 4,
    parameter integer C_CLK_TROTTLE_DETECT_EN   = 2,
    parameter integer C_WATCHDOG_ENABLE         = 1,
    parameter integer C_EXT_TOGGLE_1_SEC        = 0  // 1: use toggle_1_sec input, 0: generate internally

) (
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,

    input  wire         ap_clk_2,
    input  wire         ap_clk_2_cont,
    input  wire         ap_rst_2,

    input  wire         toggle_1sec,
    input  wire         rst_watchdog,    // pulse
    output wire         watchdog_alarm,

    input  wire         We,
    input  wire  [ 2:0] Addr,
    input  wire  [31:0] Data_In,
    input  wire  [31:0] User_Status_1,

    output logic  [31:0] Data_Out
);

logic       Scratch    = 1'b0;
logic [1:0] Rst_Detect = 'h0;

logic CU_Start              = 1'b0;
logic CU_Already_Started    = 1'b0;
logic watchdog_enable       = 1'b0;
logic [1:0] watchdog_cfg    = 'h0;
logic watchdog_rst          = 1'b0;
logic sw_watchdog_rst       = 1'b0;

wire alarm_watchdog;
// false patg
(*dont_touch ="true"*)  logic watchdog_alarm_latch  = 1'b0;
(*dont_touch ="true"*)  logic watchdog_alarm_proc   = 1'b0;

logic [31:0] Status_Live_1  = 'h0;
logic [31:0] Status_Latch_1 = 'h0;

wire  ap_clk_throt_alarm;
wire  ap_clk_2_throt_alarm;
logic ap_clk_throt_alarm_clear_tog = 1'b0;
logic ap_clk_2_throt_alarm_clear_tog = 1'b0;

logic throttle_detection_enable = 1'b0;
wire  ap_clk_throttle_detection_avail;
wire  ap_clk_2_throttle_detection_avail;

assign watchdog_alarm = watchdog_alarm_latch;

generate
    if (C_WATCHDOG_ENABLE == 1) begin : watchdog
        watchdog #(
            .C_CLOCK_FREQ       ( C_CLOCK_FREQ          ),
            .C_EXT_TOGGLE_1_SEC ( C_EXT_TOGGLE_1_SEC    )
        ) u_watchdog (
            .clk_cont       ( ap_clk_cont       ),
            .clk            ( ap_clk            ),
            .rst            ( ap_rst            ),
            .enable         ( watchdog_enable   ),
            .cfg            ( watchdog_cfg      ),
            .toggle_1sec    ( toggle_1sec       ),
            .alive          ( watchdog_rst      ),
            .alarm          ( alarm_watchdog    ) // pulse
        );

    end else begin : no_watchdog
        assign alarm_watchdog = 1'b0;
    end

    if (C_CLK_TROTTLE_DETECT_EN >= 1) begin : ap_clk_throt_detect
        clk_throttle_detect #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF )
        ) u_clk_throttle_detect(
            .clk                ( ap_clk                        ),
            .clk_cont           ( ap_clk_cont                   ),

            // on clk domain
            .enable             ( throttle_detection_enable     ),
            .alarm_clear_tog    ( ap_clk_throt_alarm_clear_tog  ),
            .alarm_latch        ( ap_clk_throt_alarm            )
        );

        logic detection_avail = 1'b0;
        always_ff @(posedge ap_clk_cont) begin
            detection_avail <= 1'b1;
        end

        xpm_cdc_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF ),
            .INIT_SYNC_FF   ( 0            ),
            .SRC_INPUT_REG  ( 0            ),
            .SIM_ASSERT_CHK ( 0            )
        ) u_cdc_detection_avail (
            .src_clk  ( ap_clk_cont                     ),
            .src_in   ( detection_avail                 ),
            .dest_clk ( ap_clk                          ),
            .dest_out ( ap_clk_throttle_detection_avail )
        );

    end : ap_clk_throt_detect

    if (C_CLK_TROTTLE_DETECT_EN >= 2) begin : ap_clk_2_throt_detect
        wire alarm_clear_cdc;
        wire alarm_latch;
        wire ap_clk_2_throttle_detection_enable;

        xpm_cdc_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
            .INIT_SYNC_FF   ( 0             ),
            .SRC_INPUT_REG  ( 1             ),
            .SIM_ASSERT_CHK ( 0             )
        ) u_cdc_enable (
            .src_clk  ( ap_clk           ),
            .src_in   ( throttle_detection_enable ),
            .dest_clk ( ap_clk_2         ),
            .dest_out ( ap_clk_2_throttle_detection_enable  )
        );

        clk_throttle_detect #(
            .DEST_SYNC_FF    (DEST_SYNC_FF)
        ) u_clk_throttle_detect (
            .clk                ( ap_clk_2      ),
            .clk_cont           ( ap_clk_2_cont ),

            // on clk domain
            .enable             ( ap_clk_2_throttle_detection_enable),
            .alarm_clear_tog    ( alarm_clear_cdc ),
            .alarm_latch        ( alarm_latch )
        );

        logic detection_avail = 1'b0;
        always_ff @(posedge ap_clk_2_cont) begin
            detection_avail <= 1'b1;
        end

        xpm_cdc_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
            .INIT_SYNC_FF   ( 0             ),
            .SRC_INPUT_REG  ( 0             ),
            .SIM_ASSERT_CHK ( 0             )
        ) u_cdc_detection_avail (
            .src_clk  ( ap_clk_2_cont                     ),
            .src_in   ( detection_avail                   ),
            .dest_clk ( ap_clk                            ),
            .dest_out ( ap_clk_2_throttle_detection_avail )
        );

        xpm_cdc_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
            .INIT_SYNC_FF   ( 0             ),
            .SRC_INPUT_REG  ( 0             ),
            .SIM_ASSERT_CHK ( 0             )
        ) u_cdc_clear_top (
            .src_clk  ( ap_clk                          ),
            .src_in   ( ap_clk_2_throt_alarm_clear_tog  ),
            .dest_clk ( ap_clk_2                        ),
            .dest_out ( alarm_clear_cdc                 )
        );

        xpm_cdc_single #(
            .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
            .INIT_SYNC_FF   ( 0             ),
            .SRC_INPUT_REG  ( 0             ),
            .SIM_ASSERT_CHK ( 0             )
        ) u_cdc_alarm (
            .src_clk  ( ap_clk_2                ),
            .src_in   ( alarm_latch             ),
            .dest_clk ( ap_clk                  ),
            .dest_out ( ap_clk_2_throt_alarm    )
        );
    end : ap_clk_2_throt_detect
endgenerate

always_comb begin
    Data_Out = 'h0;
    // only 4 registers are used so far, but keep room for 4 more
    case (Addr[2:0])
        3'b000: Data_Out      = {{16{~Scratch}}, {16{Scratch}}};
        3'b001: Data_Out[1:0] = Rst_Detect;
        3'b010: begin
                    Data_Out[0] = CU_Start;
                    Data_Out[4] = CU_Already_Started;
                end
        3'b011: begin
                    if (C_WATCHDOG_ENABLE == 1) begin
                        Data_Out[0] = 1'b1;
                    end
                    Data_Out[4]     = watchdog_enable;
                    Data_Out[6:5]   = watchdog_cfg;
                    Data_Out[8]     = watchdog_alarm_proc;

                    if (C_CLK_TROTTLE_DETECT_EN >= 1) begin
                        if (ap_clk_throttle_detection_avail) begin
                            Data_Out[17] = 1'b1;
                        end

                        Data_Out[18] = ap_clk_throt_alarm;
                    end

                    if (C_CLK_TROTTLE_DETECT_EN >= 2) begin
                        if (ap_clk_2_throttle_detection_avail) begin
                            Data_Out[19] = 1'b1;
                        end
                        Data_Out[20] = ap_clk_2_throt_alarm;
                    end

                    Data_Out[21] = throttle_detection_enable;

                end
        3'b100:  Data_Out = Status_Live_1;
        3'b101:  Data_Out = Status_Latch_1;
        default: Data_Out = 'h0;
    endcase
end

always_ff @(posedge ap_clk) begin
    Status_Live_1 <= User_Status_1;
    foreach (User_Status_1[i]) begin
        if (User_Status_1[i]) begin
            Status_Latch_1[i] <= 1'b1;
        end
    end

    if (alarm_watchdog) begin
        watchdog_alarm_latch <= 1'b1;
        watchdog_alarm_proc  <= 1'b1;
        CU_Start             <= 1'b0;
        CU_Already_Started   <= 1'b0;
    end

    watchdog_rst <= rst_watchdog || sw_watchdog_rst;

    sw_watchdog_rst <= 1'b0;

    if (We) begin
        case (Addr[2:0])
            3'b000: Scratch <= Data_In[0];
            3'b001: Rst_Detect <= Data_In[1:0];
            3'b010: begin
                        CU_Start <= Data_In[0];
                        // if the another app try to start, save the event
                        //      prior stopping, the SW will check if this bit is set and
                        //          report error as another app came and may have created error
                        if (CU_Start && Data_In[0])  begin
                            CU_Already_Started <= 1'b1;
                        end
                        // clear when the CU is stopped
                        if (~Data_In[0]) begin
                            CU_Already_Started <= 1'b0;
                        end
                    end

            3'b011: begin
                        watchdog_enable <= Data_In[4];
                        watchdog_cfg    <= Data_In[6:5];
                        // clear on write 1 or disabled
                        if (Data_In[8] || (Data_In[4] == 'b0)) begin
                            watchdog_alarm_latch  <= 1'b0;
                            watchdog_alarm_proc   <= 1'b0;
                        end
                        sw_watchdog_rst <= Data_In[12];

                        if (C_CLK_TROTTLE_DETECT_EN >= 1) begin
                            // clear on write 1
                            if (Data_In[16]) begin
                                ap_clk_throt_alarm_clear_tog   <= ~ap_clk_throt_alarm_clear_tog;
                            end
                        end
                        if (C_CLK_TROTTLE_DETECT_EN >=2) begin
                            // clear on write 1
                            if (Data_In[16]) begin
                                ap_clk_2_throt_alarm_clear_tog <= ~ap_clk_2_throt_alarm_clear_tog;
                            end
                        end
                        throttle_detection_enable <= Data_In[21];
                    end

            3'b101: Status_Latch_1 <= 'h0; // write to clear all latches
            default: $display("Illegal address");
        endcase
    end

    if (ap_rst) begin
        Rst_Detect                      <= 'h0;
        watchdog_enable                 <= 1'b0;
        watchdog_cfg                    <= 'h0;
        CU_Start                        <= 1'b0;
        CU_Already_Started              <= 1'b0;

        Status_Live_1                   <= 'h0;
        Status_Latch_1                  <= 'h0;

        throttle_detection_enable       <= 1'b0;
        ap_clk_throt_alarm_clear_tog    <= 1'b0;
        ap_clk_2_throt_alarm_clear_tog  <= 1'b0;
    end
end

endmodule: common_ctrl_status
`default_nettype wire
