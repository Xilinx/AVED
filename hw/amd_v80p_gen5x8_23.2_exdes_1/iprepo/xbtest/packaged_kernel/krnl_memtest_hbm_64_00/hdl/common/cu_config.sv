
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

module cu_config #(
    parameter integer C_ADDR_WIDTH          = 28, // Max is 28
    parameter integer C_PLRAM_ADDR_WIDTH    = 64,
    parameter integer C_PLRAM_DATA_WIDTH    = 32
) (
    // System Signals
    input  wire                              clk,
    input  wire                              rst,

    // PLRAM AXI4 master write interface
    output logic                            plram_awvalid = 'h0,
    input  wire                             plram_awready,
    output logic [C_PLRAM_ADDR_WIDTH-1:0]   plram_awaddr = 'h0,
    output logic [8-1:0]                    plram_awlen = 'h0,
    output logic                            plram_wvalid = 'h0,
    input  wire                             plram_wready,
    output logic [C_PLRAM_DATA_WIDTH-1:0]   plram_wdata = 'h0,
    output wire [C_PLRAM_DATA_WIDTH/8-1:0]  plram_wstrb,
    output logic                            plram_wlast = 'h0,
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
    output logic                            done_pulse,

    output logic                        cs,
    output logic                        wr_en,
    output logic [C_ADDR_WIDTH-1:0]     addr,
    output logic [31:0]                 wdata,
    input  wire  [31:0]                 rdata,
    input  wire                         cmd_cmplt
);

/////////////////////////////////////////////////////////////////////////////
// Variables
/////////////////////////////////////////////////////////////////////////////
logic                                 plram_write_start     = 1'b0; // this trigger a data burst to PLRAM
logic  [C_PLRAM_DATA_WIDTH-1:0]       plram_write_data      = 'h0;
logic                                 plram_write_done      = 1'b0;
wire                                  plram_wxfer;      // Unregistered write data transfer

/////////////////////////////////////////////////////////////////////////////
// State Machine
/////////////////////////////////////////////////////////////////////////////

localparam logic [2:0] ST_IDLE             = 4'b001;
localparam logic [2:0] ST_RD_ONE_REG_CU    = 4'b010;
localparam logic [2:0] ST_WR_ONE_REG_CU    = 4'b100;
logic [2:0] current_state = ST_IDLE;

always_ff @(posedge clk) begin

    cs                  <= 1'b0;
    wr_en               <= 1'b0;
    plram_write_start   <= 1'b0;
    done_pulse          <= 1'b0;

    unique case(1'b1)

        current_state[0]: begin // ST_IDLE
            if (start_pulse) begin
                cs                  <= 1'b1;
                wr_en               <= 1'b0;
                addr                <= scalar00[C_ADDR_WIDTH+4-1:4];

                case (scalar00[3:0])
                    4'b0000: begin // Write 1 reg
                            wr_en               <= 1'b1;
                            wdata               <= scalar01;
                            current_state       <= ST_WR_ONE_REG_CU;
                        end
                    // 4'b0001: begin
                    default : begin  // Read 1 reg
                            current_state       <= ST_RD_ONE_REG_CU;
                        end
                endcase
            end
        end // ST_IDLE

        current_state[1]: begin // ST_RD_ONE_REG_CU
            if (plram_write_done) begin
                done_pulse      <= 1'b1;
                current_state   <= ST_IDLE;
            end
            if (cmd_cmplt) begin
                plram_write_start   <= 1'b1;
                plram_write_data    <= rdata;
            end

        end // ST_RD_ONE_REG_CU

        current_state[2]: begin // ST_WR_ONE_REG_CU
            if (cmd_cmplt) begin
                done_pulse      <= 1'b1;
                current_state   <= ST_IDLE;
            end
        end // ST_WR_ONE_REG_CU

    endcase


    if (rst) begin
        cs                  <= 1'b0;
        wr_en               <= 1'b0;
        plram_write_start   <= 1'b0;
        plram_write_data    <= 'h0;
        done_pulse          <= 1'b0;
        current_state       <= ST_IDLE;
    end

end

/////////////////////////////////////////////////////////////////////////////
// PLRAM write control logic
/////////////////////////////////////////////////////////////////////////////

assign plram_wxfer          = plram_wvalid & plram_wready;
assign plram_wstrb          = {(C_PLRAM_DATA_WIDTH/8){1'b1}};
assign plram_bready         = 1'b1;

always @(posedge clk) begin
    plram_write_done    <= plram_bready & plram_bvalid;

    if (plram_write_start) begin
        plram_awaddr        <= axi00_ptr0;
        plram_awlen         <= 'h0;
        plram_awvalid       <= 1'b1;
        plram_wdata         <= plram_write_data;
        plram_wvalid        <= 1'b1;
        plram_wlast         <= 1'b1;
    end else if (plram_awready) begin
        plram_awvalid <= 1'b0;
    end

    if (plram_wxfer) begin
        plram_wvalid        <= 1'b0;
    end

    if (rst) begin
        plram_awvalid       <= 1'b0;
        plram_wvalid        <= 1'b0;
        plram_wlast         <= 1'b0;
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

endmodule : cu_config
