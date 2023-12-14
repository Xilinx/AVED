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


`timescale 1ps/1ps
(* DowngradeIPIdentifiedWarnings="yes" *)
module mac_core_trans_debug
(

  input  wire [15:0]  gt_drpdo,
  input  wire [0:0]   gt_drprdy,
  output reg [0:0]    gt_drpen,
  output reg [0:0]    gt_drpwe,
  output reg [9:0]    gt_drpaddr,
  output reg [15:0]   gt_drpdi,
  input   wire         mode_change,
  output               gt_drp_done,
  output  wire         ctl_rx_rate_10g_25gn,  ////1'b1=10G, 1'b0=25G
  input   wire         axi_ctl_core_mode_switch,   // Input port from AXI reg Map

  output  reg  [1:0]   txpllclksel,       //// Channel PLL TX clock select. (QPLL0=2'b11) (QPLL1=2'b10)
  output  reg  [1:0]   rxpllclksel,       //// Channel PLL TX clock select. (QPLL0=2'b11) (QPLL1=2'b10)
  output  reg  [1:0]   txsysclksel,       //// Channel PLL TX Ref clock select. (QPLL0=2'b10) (QPLL1=2'b11)
  output  reg  [1:0]   rxsysclksel,       //// Channel PLL TX Ref clock select. (QPLL0=2'b10) (QPLL1=2'b11)
  output reg           rxafecfoken,
  output reg [3:0]     rxdfecfokfcnum,
  input                reset,            //// Reset for this module
  input                drp_clk          //// DRP clock, connect to same clock that goes to gt drp clock.
    );
    localparam  IDLE_STATE         = 8'b00000001;
    localparam  WR_STATE           = 8'b00000010;
    localparam  WR_RDY_STATE       = 8'b00000100;
    localparam  READ_STATE         = 8'b00001000;
    localparam  RMW_RDY_STATE      = 8'b00010000;
    localparam  RD_MODIFY_WR_STATE = 8'b00100000;
    localparam  DONE_STATE         = 8'b01000000;
    localparam  MODE_SWITCH_STATE  = 8'b10000000;



    localparam  INDEX_MAX1    = 5'd26;   /// Max count value indexing the write of 16-bit DRP registers
    localparam  INDEX_MAX2    = 5'd7;    //// Max count value indexing the read-modify-write DRP registers
    wire [9:0]  drp_addr;
    wire [15:0] drp_di;
    reg         ch_drp_en_r;
    reg         ch_drp_we_r;
    wire        ch_drp_rdy;

    reg         mode        = 1'b0;
    reg         mode_reg    = 1'b0;
    reg         mode_reg2   = 1'b0;
    reg         mode_switch = 1'b0;   ////25GE

    reg [47:0]  rom_data = 48'h000000000000;
    reg  [4:0]  wr_index;
    reg  [4:0]  rd_index;

    wire        ten_to_twentyfive;
    wire        twentyfive_to_ten;

    reg         done;
    reg [7:0]   td_fsm;
    reg         speed;
    reg         rdy_reg;
    reg         rd_flag;

    reg [4:0]   done_cnt;
    wire        mode_change_sync;
    wire        mode_change_int;
    reg         mode_change_d1;
    reg         mode_change_d2;
    reg         mode_change_d3;





    always @(posedge drp_clk)
    begin
        if  (reset == 1'b1)
        begin
            gt_drpaddr  <=  10'b0;
            gt_drpen    <=  1'b0;
            gt_drpdi    <=  16'b0;
            gt_drpwe    <=  1'b0;
        end
        else
        begin
            gt_drpaddr  <=  drp_addr;
            gt_drpen    <=  ch_drp_en_r;
            gt_drpdi    <=  drp_di;
            gt_drpwe    <=  ch_drp_we_r;
         end
    end

    always @(posedge drp_clk)
    begin
        if (mode_switch == 1'b0)
		begin
		  txpllclksel     <=  2'b11;  //QPLL0 (25G)
		  rxpllclksel     <=  2'b11;

		  txsysclksel     <=  2'b10;  //QPLL0 (25G)
		  rxsysclksel     <=  2'b10;
                  rxafecfoken     <=  1'b0;
                  rxdfecfokfcnum  <=  4'b0000;
		end
		else
		begin
		  txpllclksel     <=  2'b10; //QPLL1 (10G)
		  rxpllclksel     <=  2'b10;

		  txsysclksel     <=  2'b11; //QPLL1 (10G)
		  rxsysclksel     <=  2'b11;
                  rxafecfoken     <=  1'b1;
                  rxdfecfokfcnum  <=  4'b1101;
		end
    end

    always @(posedge drp_clk)
    begin
        if (reset == 1'b1)
        begin
            rdy_reg      <=  1'b0;
        end
        else
        begin
            rdy_reg      <=  ch_drp_rdy;
        end
    end


    //////////////////////////////////////////////////
    //// State machine flow for mode switching
    //////////////////////////////////////////////////
    always @ (posedge drp_clk)
    begin
        if (reset == 1'b1)
        begin
            td_fsm           <= IDLE_STATE;
            wr_index      <= 5'd0;
            rd_index      <= 5'd0;
            speed         <= 1'b0;
            done          <= 1'b0;
            ch_drp_en_r   <= 1'b0;
            ch_drp_we_r   <= 1'b0;
            rom_data      <= 48'h000000000000;
        end
        else
        begin
            case (td_fsm)
                IDLE_STATE:
                        begin
                            if (ten_to_twentyfive == 1'b1)
                            begin
                                td_fsm           <=  WR_STATE;
                                speed         <=  1'b0;  //// Switching to 25G
                                wr_index      <=  5'd0;
                                rd_index      <=  5'd0;
                                done          <=  1'b0;
                                ch_drp_en_r   <=  1'b0;
                            end
                            else if (twentyfive_to_ten == 1'b1)
                            begin
                                td_fsm           <=  WR_STATE;
                                speed         <=  1'b1;  //// Switching to 10G
                                wr_index      <=  5'd0;
                                rd_index      <=  5'd0;
                                done          <=  1'b0;
                                ch_drp_en_r   <=  1'b0;
                            end
                            else
                            begin
                                wr_index      <=  5'd0;
                                rd_index      <=  5'd0;
                                done          <=  1'b0;
                                ch_drp_en_r   <=  1'b0;
                            end
                        end

                WR_STATE:
                        begin
                            case(wr_index)
                                                                        //     rom_data[41:32] == drp_addr
                                                                        //     rom_data[31:16] == 10G setting
                                                                        //     rom_data[15:0]  == 25G settin
                                5'd0 : begin
                                          rom_data <= 48'h811640409090;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd1 : begin
                                          rom_data <= 48'h80F7C0C04040;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd2: begin
                                          rom_data <= 48'h80F810C01040;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd3 : begin
                                          rom_data <= 48'h80F9C0C04040;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd4 : begin
                                          rom_data <= 48'h810180C00040;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd5 : begin
                                          rom_data <= 48'h8010026901E9;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd6 : begin
                                          rom_data <= 48'h801100120010;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd7 : begin
                                          rom_data <= 48'h80A500120010;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd8 : begin
                                          rom_data <= 48'h810A00000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd9 : begin
                                          rom_data <= 48'h810800000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd10 : begin
                                          rom_data <= 48'h810900000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd11 : begin
                                          rom_data <= 48'h810E00000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd12 : begin
                                          rom_data <= 48'h810B00000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd13 : begin
                                          rom_data <= 48'h810D00000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd14 : begin
                                          rom_data <= 48'h810C00000004;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd15 : begin
                                          rom_data <= 48'h803DFFD0FFE0;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd16 : begin
                                          rom_data <= 48'h807501023006;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd17 : begin
                                          rom_data <= 48'h80D200540000;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd18 : begin
                                          rom_data <= 48'h80A703003000;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd19 : begin
                                          rom_data <= 48'h80A810000000;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd20 : begin
                                          rom_data <= 48'h809D03C203C6;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd21 : begin
                                          rom_data <= 48'h80A16C00F800;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd22 : begin
                                          rom_data <= 48'h80536C00F800;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd23 : begin
                                          rom_data <= 48'h80546C00F800;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd24 : begin
                                          rom_data <= 48'h811C00120010;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd25 : begin
                                          rom_data <= 48'h80B10200281C;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd26 : begin
                                          rom_data <= 48'h80B041014120;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                               default: begin
                                          rom_data <= 48'h000000000000;
                                          ch_drp_en_r <= 1'b0;
                                          ch_drp_we_r <= 1'b0;
                                         end
                             endcase

                             rd_flag         <= 1'b0;
                             td_fsm             <= WR_RDY_STATE;
                       end

                WR_RDY_STATE:
                        begin
                             ch_drp_en_r     <= 1'b0;
                             ch_drp_we_r     <= 1'b0;
                             if (rdy_reg == 1'b1)
                             begin
                                if (wr_index == INDEX_MAX1)
                                begin
                                   td_fsm       <= READ_STATE;
                                   wr_index  <= 5'd0;
                                end
                                else if (wr_index < INDEX_MAX1)
                                begin
                                   td_fsm       <= WR_STATE;
                                   wr_index  <= wr_index + 1;
                                end
                             end
                             else
                                td_fsm          <= WR_RDY_STATE;
                        end

                READ_STATE:
                        begin
                             case(rd_index)
                                5'd0 : begin
                                          rom_data[47:32] <= 16'h80FB;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd1 : begin
                                          rom_data[47:32] <= 16'h80DD;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd2 : begin
                                          rom_data[47:32] <= 16'h8135;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd3 : begin
                                          rom_data[47:32] <= 16'h809B;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd4 : begin
                                          rom_data[47:32] <= 16'h808C;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd5 : begin
                                          rom_data[47:32] <= 16'h8066;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd6 : begin
                                          rom_data[47:32] <= 16'h80D3;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                                5'd7 : begin
                                          rom_data[47:32] <= 16'h80FA;
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b0;
                                       end
                               default: begin
                                          rom_data <= 48'h000000000000;
                                          ch_drp_en_r <= 1'b0;
                                          ch_drp_we_r <= 1'b0;
                                         end
                             endcase
                             rd_flag         <= 1'b0;
                             td_fsm             <= RMW_RDY_STATE;
                        end

               RMW_RDY_STATE:
                        begin
                             ch_drp_en_r     <= 1'b0;
                             ch_drp_we_r     <= 1'b0;
                             if (rdy_reg == 1'b1)
                             begin
                                if (wr_index == INDEX_MAX2 && rd_flag == 1'b1)
                                begin
                                   td_fsm          <= DONE_STATE;
                                   wr_index     <= 5'd0;
                                   rd_index     <= 5'd0;
                                end
                                else if (rd_index <= INDEX_MAX2 && rd_flag == 1'b0)
                                begin
                                   td_fsm       <= RD_MODIFY_WR_STATE;
                                   rd_index  <= rd_index + 1;
                                   rd_flag   <= 1'b1;
                                end
                                else if (wr_index < INDEX_MAX2 && rd_flag == 1'b1)
                                begin
                                   td_fsm       <= READ_STATE;
                                   wr_index  <= wr_index + 1;
                                   rd_flag   <= 1'b0;
                                end

                             end
                             else
                                td_fsm          <= RMW_RDY_STATE;
                        end
                RD_MODIFY_WR_STATE:
                        begin
                             case(wr_index)
                                5'd0 : begin
                                          rom_data <= {16'h80FB,gt_drpdo[15:6],2'b01,gt_drpdo[3],2'b01,gt_drpdo[0],
                                                                gt_drpdo[15:6],2'b11,gt_drpdo[3],2'b11,gt_drpdo[0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd1 : begin
                                          rom_data <= {16'h80DD,gt_drpdo[15:5],2'b00,3'b011,
                                                                gt_drpdo[15:5],2'b11,3'b111};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd2 : begin
                                          rom_data <= {16'h8135,6'b010010,gt_drpdo[9:0],
                                                                6'b010000,gt_drpdo[9:0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd3 : begin
                                          rom_data <= {16'h809B,gt_drpdo[15:10],2'b11,gt_drpdo[7:0],
                                                                gt_drpdo[15:10],2'b01,gt_drpdo[7:0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd4 : begin
                                          rom_data <= {16'h808C,gt_drpdo[15:10],2'b11,gt_drpdo[7:0],
                                                                gt_drpdo[15:10],2'b01,gt_drpdo[7:0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end

                                5'd5 : begin
                                          rom_data <= {16'h8066,gt_drpdo[15:4],2'b01,gt_drpdo[1:0],
                                                                gt_drpdo[15:4],2'b10,gt_drpdo[1:0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd6 : begin
                                          rom_data <= {16'h80D3,gt_drpdo[15:2],1'b1,gt_drpdo[0],
                                                                gt_drpdo[15:2],1'b0,gt_drpdo[0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                5'd7 : begin
                                          rom_data <= {16'h80FA,gt_drpdo[15:11],2'b00,gt_drpdo[8:7],1'b0,gt_drpdo[5:0],
                                                                gt_drpdo[15:11],2'b11,gt_drpdo[8:7],1'b1,gt_drpdo[5:0]};
                                          ch_drp_en_r <= 1'b1;
                                          ch_drp_we_r <= 1'b1;
                                       end
                                default: begin
                                          rom_data <= 48'h000000000000;
                                          ch_drp_en_r <= 1'b0;
                                          ch_drp_we_r <= 1'b0;
                                         end
                             endcase
                             rd_flag         <= 1'b1;
                             td_fsm             <= RMW_RDY_STATE;
                        end

                DONE_STATE:
                        begin
                             ch_drp_en_r     <= 1'b0;
                             ch_drp_we_r     <= 1'b0;
                             rd_flag         <= 1'b0;
                             wr_index        <= 5'd0;
                             rd_index        <= 5'd0;
                             if (done_cnt[4] == 1'b1)
                             begin
                                 td_fsm    <=  MODE_SWITCH_STATE;
                                 done        <=  1'b0;
                             end
                             else
                             begin
                                 td_fsm    <= DONE_STATE;
                                  done  <=  1'b1;
                             end
                        end

                MODE_SWITCH_STATE:
                        begin
                             td_fsm          <=  IDLE_STATE;
                             mode_switch  <=  ~mode_switch;   ////1'b1=10G, 1'b0=25G
                        end
                endcase
        end
    end

    //////////////////////////////////////////////////
    //// done_cnt signal generation
    //////////////////////////////////////////////////
    always @(posedge drp_clk)
    begin
        if (reset == 1'b1)
            done_cnt  <= 5'd0;
        else
        begin
            if (done == 1'b1)
               done_cnt  <= done_cnt + 5'd1;
            else
               done_cnt  <= 5'd0;
        end
    end

    //////////////////////////////////////////////////
    //// mode_change registering
    //////////////////////////////////////////////////
    always @(posedge drp_clk)
    begin
        if (reset == 1'b1)
        begin
            mode_change_d1  <= 1'b0;
            mode_change_d2  <= 1'b0;
            mode_change_d3  <= 1'b0;
        end
        else
        begin
            mode_change_d1  <= mode_change_sync;
            mode_change_d2  <= mode_change_d1;
            mode_change_d3  <= mode_change_d2;
        end
    end
    //////////////////////////////////////////////////
    //// mode signal generation on rising edge of mode_change
    //////////////////////////////////////////////////
    always @(posedge drp_clk)
    begin
        if ((td_fsm == IDLE_STATE) && ((mode_change_d3 == 1'b0) && (mode_change_d2 == 1'b1)))
            mode   <= ~mode;
    end

    always @(posedge drp_clk)
    begin
        mode_reg   <= mode;
        mode_reg2  <= mode_reg;
    end

    mac_core_td_cdc_sync i_mac_core_cdc_sync
    (
     .clk         (drp_clk),
     .signal_in   (mode_change_int),
     .signal_out  (mode_change_sync)
    );
    assign mode_change_int = mode_change | axi_ctl_core_mode_switch;

    assign twentyfive_to_ten = mode_reg & (~mode_reg2);
    assign ten_to_twentyfive = (~mode_reg) & (mode_reg2);

    assign drp_addr                 = rom_data[41:32];
    assign ch_drp_rdy               = gt_drprdy;
    assign drp_di                   = speed ? rom_data[31:16] : rom_data[15:0];
    assign gt_drp_done              = done;
    assign ctl_rx_rate_10g_25gn     = mode_switch; ////1'b1=10G, 1'b0=25G

endmodule


(* DowngradeIPIdentifiedWarnings="yes" *)
module mac_core_td_cdc_sync (
 input clk,
 input signal_in,
 output wire signal_out
);

                          wire sig_in_cdc_from;
 (* ASYNC_REG = "TRUE" *) reg  s_out_d2_cdc_to;
 (* ASYNC_REG = "TRUE" *) reg  s_out_d3;
 (* ASYNC_REG = "TRUE" *) reg  s_out_d4;
 (* ASYNC_REG = "TRUE" *) reg  s_out_d5;

assign sig_in_cdc_from = signal_in;
assign signal_out      = s_out_d5;

always @(posedge clk) begin
  s_out_d2_cdc_to <= sig_in_cdc_from;
  s_out_d3        <= s_out_d2_cdc_to;
  s_out_d4        <= s_out_d3;
  s_out_d5        <= s_out_d4;
end

endmodule
