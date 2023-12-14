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

module memtest_config #(
    parameter integer C_PLRAM_ADDR_WIDTH    = 64,
    parameter integer C_PLRAM_DATA_WIDTH    = 32,
    parameter integer C_NUM_USED_M_AXI      = 32, // Number of used M_AXI ports 1..32 (enables M01_AXI .. M32_AXI for memory kernel)
    parameter integer C_NUM_CU_REGISTER     = 11  // In multi-reg read mode. Memory CU status I/F has 16 registers (addr in [0x300000..0x300018))
) (
    // System Signals
    input  wire                             clk,
    input  wire                             rst,

    // PLRAM AXI4 master write interface
    output logic                            plram_awvalid = 'h0,
    input  wire                             plram_awready,
    output logic [C_PLRAM_ADDR_WIDTH-1:0]   plram_awaddr = 'h0,
    output wire  [8-1:0]                    plram_awlen,
    output logic                            plram_wvalid = 'h0,
    input  wire                             plram_wready,
    output logic [C_PLRAM_DATA_WIDTH-1:0]   plram_wdata = 'h0,
    output wire  [C_PLRAM_DATA_WIDTH/8-1:0] plram_wstrb,
    output wire                             plram_wlast,
    input  wire                             plram_bvalid,
    output wire                             plram_bready,
    // PLRAM AXI4 master read interface - NOT USED
    output logic                            plram_arvalid = 'h0,
    input  wire                             plram_arready,
    output logic[C_PLRAM_ADDR_WIDTH-1:0]    plram_araddr  = 'h0,
    output logic[8-1:0]                     plram_arlen,
    input  wire                             plram_rvalid,
    output logic                            plram_rready  = 'h0,
    input  wire [C_PLRAM_DATA_WIDTH-1:0]    plram_rdata,
    input  wire                             plram_rlast,

    // SDx Control Signals
    input  wire [C_PLRAM_ADDR_WIDTH-1:0]    axi00_ptr0,
    input  wire [32-1:0]                    scalar00,
    input  wire [32-1:0]                    scalar01,
    input  wire [32-1:0]                    scalar02, // NOT USED
    input  wire [32-1:0]                    scalar03, // NOT USED
    input  wire                             start_pulse,
    output logic                            done_pulse = 'h0,

    output logic                            cs = 'h0,
    output logic                            wr_en = 'h0,
    output logic [27:0]                     addr = 'h0,
    output logic [31:0]                     wdata = 'h0,
    input  wire  [31:0]                     rdata,
    input  wire                             cmd_cmplt
);

/////////////////////////////////////////////////////////////////////////////
// Local Parameters
/////////////////////////////////////////////////////////////////////////////
localparam integer LP_PLRAM_DATA_WIDTH_BYTES    = C_PLRAM_DATA_WIDTH/8;
localparam integer LP_LOG_NUM_CU_REGISTER       = $clog2(C_NUM_CU_REGISTER);
localparam integer LP_PLRAM_FIFO_DEPTH          = 2 ** (LP_LOG_NUM_CU_REGISTER); // FIFO depth equals the PLRAM write burst size
localparam integer LP_LOG_PLRAM_FIFO_DEPTH      = $clog2(LP_PLRAM_FIFO_DEPTH);
localparam integer LP_PLRAM_CORE_ADDR_OFFS      = LP_LOG_PLRAM_FIFO_DEPTH+2;
localparam integer LP_PLRAM_AWLEN               = LP_PLRAM_FIFO_DEPTH-1;
localparam integer LP_LOG_PLRAM_AWLEN           = $clog2(LP_PLRAM_AWLEN);
localparam integer LP_CNT_WIDTH                 = 25;

localparam [LP_CNT_WIDTH-1:0] LP_ONE                = {{LP_CNT_WIDTH-1{1'b0}},1'b1};
localparam [LP_CNT_WIDTH-1:0] LP_TWO                = {{LP_CNT_WIDTH-2{1'b0}},2'b10};

/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////
// FIFO write
logic  [5:0]                        memory_core_id_last     = 'h0;
logic  [LP_CNT_WIDTH-1:0]           memory_core_id          = 'h0;
logic  [5:0]                        memory_core_id_d1       = 'h0;
logic  [LP_CNT_WIDTH-1:0]           plram_FIFO_wr_ptr       = 'h0;
logic                               plram_write_start       = 1'b0; // this trigger a data burst to PLRAM
logic                               plram_write_start_d1    = 1'b0; // Write addr after this pulse
logic                               plram_write_start_d2    = 1'b0; // Write data after this pulse
logic                               plram_write_done        = 1'b0;
// FIFO read
wire                                plram_wxfer;
wire                                plram_final_wxfer;
logic [LP_CNT_WIDTH-1:0]            plram_FIFO_rd_ptr       = 'h0;
logic [C_PLRAM_DATA_WIDTH-1:0]      plram_FIFO [LP_PLRAM_FIFO_DEPTH-1:0]; // FIFO is never read and written at same time
logic [C_PLRAM_ADDR_WIDTH-1:0]      dbg_plram_awaddr = 'h0;

// State Machine
localparam logic [2:0]
    ST_IDLE             = 3'b000,
    ST_WAIT_WR_CU       = 3'b001,
    ST_WAIT_RD_CU       = 3'b010,
    ST_NEXT_RD_CU       = 3'b011,
    ST_WAIT_WR_PLRAM    = 3'b100;
logic [2:0] current_state = ST_IDLE;

/////////////////////////////////////////////////////////////////////////////
// FIFO write data
/////////////////////////////////////////////////////////////////////////////
always_ff @(posedge clk) begin
    // defaults
    cs                  <= 1'b0;
    wr_en               <= 1'b0;
    plram_write_start   <= 1'b0;
    done_pulse          <= 1'b0;

    if (cmd_cmplt) begin
        plram_FIFO[plram_FIFO_wr_ptr[LP_LOG_NUM_CU_REGISTER-1:0]] <= rdata;
    end

    case(current_state)
        ST_IDLE: begin
            if (start_pulse) begin

                case (scalar00[1:0])
                    2'b00: begin // Write 1 reg
                        cs                  <= 1'b1;
                        wr_en               <= 1'b1;
                        addr                <= scalar00[31:4];
                        wdata               <= scalar01;
                        current_state       <= ST_WAIT_WR_CU;
                    end
                    2'b01: begin // Read 1 reg
                        cs                  <= 1'b1;
                        wr_en               <= 1'b0;
                        addr                <= scalar00[31:4];
                        plram_FIFO_wr_ptr   <= 0;
                        memory_core_id_last <= 0;
                        memory_core_id      <= 0;
                        current_state       <= ST_WAIT_RD_CU;
                    end
                    default : begin // Read multiple reg
                        cs                  <= 1'b1;
                        wr_en               <= 1'b0;
                        addr                <= 'h0;
                        addr[27]            <= 1'b1;                        // memory CU cores
                        addr[26:21]         <= 'h0;                         // memory CU core index
                        addr[20]            <= 1'b1;                        // memory CU core register select
                        addr[19]            <= 1'b1;                        // memory CU core register select
                        addr[18:0]          <= C_NUM_CU_REGISTER-1; // memory CU core register offset
                        plram_FIFO_wr_ptr   <= C_NUM_CU_REGISTER-1; // Init to max to read all reg
                        memory_core_id_last <= C_NUM_USED_M_AXI-1;
                        memory_core_id      <= 0;                   // Init for next read
                        current_state       <= ST_WAIT_RD_CU;
                    end
                endcase
            end
        end // ST_IDLE

        ////////////////////////////////////////////////////////////////
        ST_WAIT_WR_CU: begin
            if (cmd_cmplt) begin
                done_pulse              <= 1'b1;
                current_state           <= ST_IDLE;
            end
        end // ST_WAIT_WR_CU

        ////////////////////////////////////////////////////////////////
        ST_WAIT_RD_CU: begin
            if (cmd_cmplt) begin
                plram_FIFO_wr_ptr       <= plram_FIFO_wr_ptr - LP_ONE;
                current_state           <= ST_NEXT_RD_CU;
            end
        end // ST_WAIT_RD_CU

        ////////////////////////////////////////////////////////////////
        ST_NEXT_RD_CU: begin
            if (plram_FIFO_wr_ptr[$high(plram_FIFO_wr_ptr)]) begin
                plram_write_start       <= 1'b1;
                memory_core_id          <= memory_core_id + LP_ONE;
                memory_core_id_d1       <= memory_core_id[5:0];
                current_state           <= ST_WAIT_WR_PLRAM;
            end else begin
                cs                      <= 1'b1;
                addr[18:0]              <= plram_FIFO_wr_ptr[18:0];
                current_state           <= ST_WAIT_RD_CU;
            end
        end // ST_NEXT_RD_CU

        ////////////////////////////////////////////////////////////////
        ST_WAIT_WR_PLRAM: begin
            if (plram_write_done) begin
                if (memory_core_id_d1 == memory_core_id_last) begin
                    done_pulse          <= 1'b1;
                    current_state       <= ST_IDLE;
                end else begin
                    cs                  <= 1'b1;
                    addr                <= 'h0;
                    addr[27]            <= 1'b1;                        // memory CU cores
                    addr[26:21]         <= memory_core_id[5:0];         // memory CU core index
                    addr[20]            <= 1'b1;                        // memory CU core register select
                    addr[19]            <= 1'b1;                        // memory CU core register select
                    addr[18:0]          <= C_NUM_CU_REGISTER-1;         // memory CU core register offset
                    plram_FIFO_wr_ptr   <= C_NUM_CU_REGISTER-1;
                    current_state       <= ST_WAIT_RD_CU;
                end
            end
        end // ST_WAIT_WR_PLRAM

        ////////////////////////////////////////////////////////////////
        default: begin
            current_state <= ST_IDLE;
        end // default
    endcase

    if (rst) begin
        current_state <= ST_IDLE;
    end
end

/////////////////////////////////////////////////////////////////////////////
// FIFO read data and PLRAM write control logic
/////////////////////////////////////////////////////////////////////////////
assign plram_wlast          = plram_FIFO_rd_ptr[LP_LOG_PLRAM_FIFO_DEPTH];
assign plram_wxfer          = plram_wvalid & plram_wready;
assign plram_final_wxfer    = plram_wlast & plram_wxfer;
assign plram_wstrb          = {LP_PLRAM_DATA_WIDTH_BYTES{1'b1}};
assign plram_awlen          = LP_PLRAM_AWLEN; // PLRAM write burst length is the same when 1 register is read and multiple registers are read
assign plram_bready         = 1'b1;

always @(posedge clk) begin

    if (plram_write_start) begin
        plram_awvalid       <= 1'b1;
        plram_awaddr        <= axi00_ptr0 + (memory_core_id_d1 << LP_PLRAM_CORE_ADDR_OFFS);
        dbg_plram_awaddr    <= axi00_ptr0 + (memory_core_id_d1 << LP_PLRAM_CORE_ADDR_OFFS);
    end else if (plram_awready) begin
        plram_awvalid       <= 1'b0;
    end

    plram_write_start_d1 <= plram_write_start;

    if (plram_write_start_d1 | plram_wxfer) begin
        plram_wdata         <= plram_FIFO[plram_FIFO_rd_ptr[LP_LOG_PLRAM_FIFO_DEPTH-1:0]];
        plram_FIFO_rd_ptr   <= plram_FIFO_rd_ptr + LP_ONE;
        dbg_plram_awaddr    <= dbg_plram_awaddr + 4;
    end

    if (plram_write_start_d1) begin
        plram_wvalid        <= 1'b1;
    end else if (plram_final_wxfer) begin
        plram_wvalid        <= 1'b0;
        plram_FIFO_rd_ptr   <= 0;
    end

    plram_write_done <= plram_bready & plram_bvalid;

    if (rst) begin
        plram_awvalid       <= 1'b0;
        plram_wvalid        <= 1'b0;
        plram_FIFO_rd_ptr   <= 0;
    end
end


// TEMP, NOT TESTED just there to prevent logic optimization and keep read interface towards PLRAM
//(*dont_touch ="true"*) logic [C_PLRAM_DATA_WIDTH-1:0]   read_data;
//always @(posedge clk) begin
//    plram_rready <= ~ plram_rready;
//    if (plram_write_start & plram_arready) begin
//        plram_araddr        <= axi00_ptr0;
//        plram_arlen         <= 'h0;
//        plram_arvalid       <= 1'b1;
//    end else if (plram_awready) begin
//        plram_arvalid   <= 1'b0;
//    end
//    if (plram_rvalid & plram_rlast) begin
//        read_data        <= plram_rdata;
//    end
//    if (rst) begin
//        plram_arvalid       <= 1'b0;
//        plram_rready        <= 1'b0;
//    end
//end

endmodule : memtest_config
`default_nettype wire