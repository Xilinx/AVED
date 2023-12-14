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

`define PORT_AXI_PTR0(INDEX)                        \
    output logic [64-1:0] axi``INDEX``_ptr0 = 'h0,  \

module krnl_xbtest_control_s_axi #(
    parameter integer C_KRNL_MODE       = 0,    // Validation kernel mode (POWER = 0, MEMORY = 1)
    parameter integer C_NUM_USED_M_AXI  = 1,    // Number of used M_AXI ports 1..32 (enables M01_AXI .. M32_AXI for memory kernel)
    parameter integer C_ADDR_WIDTH      = 12,
    parameter integer C_DATA_WIDTH      = 32
) (
    input  wire                      aclk,
    input  wire                      areset,
    input  wire                      awvalid,
    output logic                     awready,
    input  wire [C_ADDR_WIDTH-1:0]   awaddr,
    input  wire                      wvalid,
    output logic                     wready,
    input  wire [C_DATA_WIDTH-1:0]   wdata,
    input  wire [C_DATA_WIDTH/8-1:0] wstrb,
    input  wire                      arvalid,
    output logic                     arready,
    input  wire [C_ADDR_WIDTH-1:0]   araddr,
    output logic                     rvalid,
    input  wire                      rready,
    output logic [C_DATA_WIDTH-1:0]  rdata,
    output wire [2-1:0]              rresp,
    output logic                     bvalid,
    input  wire                      bready,
    output wire [2-1:0]              bresp,

    output wire [64-1:0] axi00_ptr0     , // use wire for false path definition

    `PORT_AXI_PTR0(01)
    `PORT_AXI_PTR0(02)
    `PORT_AXI_PTR0(03)
    `PORT_AXI_PTR0(04)
    `PORT_AXI_PTR0(05)
    `PORT_AXI_PTR0(06)
    `PORT_AXI_PTR0(07)
    `PORT_AXI_PTR0(08)
    `PORT_AXI_PTR0(09)
    `PORT_AXI_PTR0(10)
    `PORT_AXI_PTR0(11)
    `PORT_AXI_PTR0(12)
    `PORT_AXI_PTR0(13)
    `PORT_AXI_PTR0(14)
    `PORT_AXI_PTR0(15)
    `PORT_AXI_PTR0(16)
    `PORT_AXI_PTR0(17)
    `PORT_AXI_PTR0(18)
    `PORT_AXI_PTR0(19)
    `PORT_AXI_PTR0(20)
    `PORT_AXI_PTR0(21)
    `PORT_AXI_PTR0(22)
    `PORT_AXI_PTR0(23)
    `PORT_AXI_PTR0(24)
    `PORT_AXI_PTR0(25)
    `PORT_AXI_PTR0(26)
    `PORT_AXI_PTR0(27)
    `PORT_AXI_PTR0(28)
    `PORT_AXI_PTR0(29)
    `PORT_AXI_PTR0(30)
    `PORT_AXI_PTR0(31)
    `PORT_AXI_PTR0(32)
    `PORT_AXI_PTR0(33)
    `PORT_AXI_PTR0(34)
    `PORT_AXI_PTR0(35)
    `PORT_AXI_PTR0(36)
    `PORT_AXI_PTR0(37)
    `PORT_AXI_PTR0(38)
    `PORT_AXI_PTR0(39)
    `PORT_AXI_PTR0(40)
    `PORT_AXI_PTR0(41)
    `PORT_AXI_PTR0(42)
    `PORT_AXI_PTR0(43)
    `PORT_AXI_PTR0(44)
    `PORT_AXI_PTR0(45)
    `PORT_AXI_PTR0(46)
    `PORT_AXI_PTR0(47)
    `PORT_AXI_PTR0(48)
    `PORT_AXI_PTR0(49)
    `PORT_AXI_PTR0(50)
    `PORT_AXI_PTR0(51)
    `PORT_AXI_PTR0(52)
    `PORT_AXI_PTR0(53)
    `PORT_AXI_PTR0(54)
    `PORT_AXI_PTR0(55)
    `PORT_AXI_PTR0(56)
    `PORT_AXI_PTR0(57)
    `PORT_AXI_PTR0(58)
    `PORT_AXI_PTR0(59)
    `PORT_AXI_PTR0(60)
    `PORT_AXI_PTR0(61)
    `PORT_AXI_PTR0(62)
    `PORT_AXI_PTR0(63)
    `PORT_AXI_PTR0(64)

    output logic                     interrupt,
    output logic                     ap_start,
    input  wire                      ap_idle,
    input  wire                      ap_done,
    output logic [32-1:0]            scalar00   = 'h0,
    output logic [32-1:0]            scalar01   = 'h0,
    output logic [32-1:0]            scalar02   = 'h0,
    output logic [32-1:0]            scalar03   = 'h0
);

    //------------------------Address Info-------------------
    // 0x000 : Control signals
    //         bit 0  - ap_start (Read/Write/COH)
    //         bit 1  - ap_done (Read/COR)
    //         bit 2  - ap_idle (Read)
    //         others - reserved
    // 0x004 : Global Interrupt Enable Register
    //         bit 0  - Global Interrupt Enable (Read/Write)
    //         others - reserved
    // 0x008 : IP Interrupt Enable Register (Read/Write)
    //         bit 0  - Channel 0 (ap_done)
    //         others - reserved
    // 0x00c : IP Interrupt Status Register (Read/TOW)
    //         bit 0  - Channel 0 (ap_done)
    //         others - reserved
    // 0x010 : Data signal of scalar00
    //         bit 31~0 - scalar00[31:0] (Read/Write)
    // 0x014 : reserved
    // 0x018 : Data signal of scalar01
    //         bit 31~0 - scalar01[31:0] (Read/Write)
    // 0x01c : reserved
    // 0x020 : Data signal of scalar02
    //         bit 31~0 - scalar02[31:0] (Read/Write)
    // 0x024 : reserved
    // 0x028 : Data signal of scalar03
    //         bit 31~0 - scalar03[31:0] (Read/Write)
    // 0x02c : reserved
    // 0x030 : Data signal of axi00_ptr0
    //         bit 31~0 - axi00_ptr0[31:0] (Read/Write)
    // 0x034 : Data signal of axi00_ptr0
    //         bit 31~0 - axi00_ptr0[63:32] (Read/Write)
    // 0x038 : Data signal of axi01_ptr0
    //         bit 31~0 - axi01_ptr0[31:0] (Read/Write)
    // 0x03c : Data signal of axi01_ptr0
    //         bit 31~0 - axi01_ptr0[63:32] (Read/Write)
    // ...
    // 0x130 : Data signal of axi32_ptr0
    //         bit 31~0 - axi32_ptr0[31:0] (Read/Write)
    // 0x134 : Data signal of axi32_ptr0
    //         bit 31~0 - axi32_ptr0[63:32] (Read/Write)
    // (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

    localparam [C_ADDR_WIDTH-1:0]
        LP_ADDR_AP_CTRL         = 12'h000,
        LP_ADDR_GIE             = 12'h004,
        LP_ADDR_IER             = 12'h008,
        LP_ADDR_ISR             = 12'h00c,
        LP_ADDR_SCALAR00_0      = 12'h010,
        LP_ADDR_SCALAR01_0      = 12'h018,
        LP_ADDR_SCALAR02_0      = 12'h020,
        LP_ADDR_SCALAR03_0      = 12'h028,
        LP_ADDR_AXI00_PTR0_0    = 12'h030,
        LP_ADDR_AXI00_PTR0_1    = 12'h034,
        LP_ADDR_AXI01_PTR0_0    = 12'h038,
        LP_ADDR_AXI01_PTR0_1    = 12'h03c,
        LP_ADDR_AXI02_PTR0_0    = 12'h040,
        LP_ADDR_AXI02_PTR0_1    = 12'h044,
        LP_ADDR_AXI03_PTR0_0    = 12'h048,
        LP_ADDR_AXI03_PTR0_1    = 12'h04c,
        LP_ADDR_AXI04_PTR0_0    = 12'h050,
        LP_ADDR_AXI04_PTR0_1    = 12'h054,
        LP_ADDR_AXI05_PTR0_0    = 12'h058,
        LP_ADDR_AXI05_PTR0_1    = 12'h05c,
        LP_ADDR_AXI06_PTR0_0    = 12'h060,
        LP_ADDR_AXI06_PTR0_1    = 12'h064,
        LP_ADDR_AXI07_PTR0_0    = 12'h068,
        LP_ADDR_AXI07_PTR0_1    = 12'h06c,
        LP_ADDR_AXI08_PTR0_0    = 12'h070,
        LP_ADDR_AXI08_PTR0_1    = 12'h074,
        LP_ADDR_AXI09_PTR0_0    = 12'h078,
        LP_ADDR_AXI09_PTR0_1    = 12'h07c,
        LP_ADDR_AXI10_PTR0_0    = 12'h080,
        LP_ADDR_AXI10_PTR0_1    = 12'h084,
        LP_ADDR_AXI11_PTR0_0    = 12'h088,
        LP_ADDR_AXI11_PTR0_1    = 12'h08c,
        LP_ADDR_AXI12_PTR0_0    = 12'h090,
        LP_ADDR_AXI12_PTR0_1    = 12'h094,
        LP_ADDR_AXI13_PTR0_0    = 12'h098,
        LP_ADDR_AXI13_PTR0_1    = 12'h09c,
        LP_ADDR_AXI14_PTR0_0    = 12'h0a0,
        LP_ADDR_AXI14_PTR0_1    = 12'h0a4,
        LP_ADDR_AXI15_PTR0_0    = 12'h0a8,
        LP_ADDR_AXI15_PTR0_1    = 12'h0ac,
        LP_ADDR_AXI16_PTR0_0    = 12'h0b0,
        LP_ADDR_AXI16_PTR0_1    = 12'h0b4,
        LP_ADDR_AXI17_PTR0_0    = 12'h0b8,
        LP_ADDR_AXI17_PTR0_1    = 12'h0bc,
        LP_ADDR_AXI18_PTR0_0    = 12'h0c0,
        LP_ADDR_AXI18_PTR0_1    = 12'h0c4,
        LP_ADDR_AXI19_PTR0_0    = 12'h0c8,
        LP_ADDR_AXI19_PTR0_1    = 12'h0cc,
        LP_ADDR_AXI20_PTR0_0    = 12'h0d0,
        LP_ADDR_AXI20_PTR0_1    = 12'h0d4,
        LP_ADDR_AXI21_PTR0_0    = 12'h0d8,
        LP_ADDR_AXI21_PTR0_1    = 12'h0dc,
        LP_ADDR_AXI22_PTR0_0    = 12'h0e0,
        LP_ADDR_AXI22_PTR0_1    = 12'h0e4,
        LP_ADDR_AXI23_PTR0_0    = 12'h0e8,
        LP_ADDR_AXI23_PTR0_1    = 12'h0ec,
        LP_ADDR_AXI24_PTR0_0    = 12'h0f0,
        LP_ADDR_AXI24_PTR0_1    = 12'h0f4,
        LP_ADDR_AXI25_PTR0_0    = 12'h0f8,
        LP_ADDR_AXI25_PTR0_1    = 12'h0fc,
        LP_ADDR_AXI26_PTR0_0    = 12'h100,
        LP_ADDR_AXI26_PTR0_1    = 12'h104,
        LP_ADDR_AXI27_PTR0_0    = 12'h108,
        LP_ADDR_AXI27_PTR0_1    = 12'h10c,
        LP_ADDR_AXI28_PTR0_0    = 12'h110,
        LP_ADDR_AXI28_PTR0_1    = 12'h114,
        LP_ADDR_AXI29_PTR0_0    = 12'h118,
        LP_ADDR_AXI29_PTR0_1    = 12'h11c,
        LP_ADDR_AXI30_PTR0_0    = 12'h120,
        LP_ADDR_AXI30_PTR0_1    = 12'h124,
        LP_ADDR_AXI31_PTR0_0    = 12'h128,
        LP_ADDR_AXI31_PTR0_1    = 12'h12c,
        LP_ADDR_AXI32_PTR0_0    = 12'h130,
        LP_ADDR_AXI32_PTR0_1    = 12'h134,

        LP_ADDR_AXI33_PTR0_0    = 12'h138,
        LP_ADDR_AXI33_PTR0_1    = 12'h13c,
        LP_ADDR_AXI34_PTR0_0    = 12'h140,
        LP_ADDR_AXI34_PTR0_1    = 12'h144,
        LP_ADDR_AXI35_PTR0_0    = 12'h148,
        LP_ADDR_AXI35_PTR0_1    = 12'h14c,
        LP_ADDR_AXI36_PTR0_0    = 12'h150,
        LP_ADDR_AXI36_PTR0_1    = 12'h154,
        LP_ADDR_AXI37_PTR0_0    = 12'h158,
        LP_ADDR_AXI37_PTR0_1    = 12'h15c,
        LP_ADDR_AXI38_PTR0_0    = 12'h160,
        LP_ADDR_AXI38_PTR0_1    = 12'h164,
        LP_ADDR_AXI39_PTR0_0    = 12'h168,
        LP_ADDR_AXI39_PTR0_1    = 12'h16c,
        LP_ADDR_AXI40_PTR0_0    = 12'h170,
        LP_ADDR_AXI40_PTR0_1    = 12'h174,
        LP_ADDR_AXI41_PTR0_0    = 12'h178,
        LP_ADDR_AXI41_PTR0_1    = 12'h17c,
        LP_ADDR_AXI42_PTR0_0    = 12'h180,
        LP_ADDR_AXI42_PTR0_1    = 12'h184,
        LP_ADDR_AXI43_PTR0_0    = 12'h188,
        LP_ADDR_AXI43_PTR0_1    = 12'h18c,
        LP_ADDR_AXI44_PTR0_0    = 12'h190,
        LP_ADDR_AXI44_PTR0_1    = 12'h194,
        LP_ADDR_AXI45_PTR0_0    = 12'h198,
        LP_ADDR_AXI45_PTR0_1    = 12'h19c,
        LP_ADDR_AXI46_PTR0_0    = 12'h1a0,
        LP_ADDR_AXI46_PTR0_1    = 12'h1a4,
        LP_ADDR_AXI47_PTR0_0    = 12'h1a8,
        LP_ADDR_AXI47_PTR0_1    = 12'h1ac,
        LP_ADDR_AXI48_PTR0_0    = 12'h1b0,
        LP_ADDR_AXI48_PTR0_1    = 12'h1b4,
        LP_ADDR_AXI49_PTR0_0    = 12'h1b8,
        LP_ADDR_AXI49_PTR0_1    = 12'h1bc,
        LP_ADDR_AXI50_PTR0_0    = 12'h1c0,
        LP_ADDR_AXI50_PTR0_1    = 12'h1c4,
        LP_ADDR_AXI51_PTR0_0    = 12'h1c8,
        LP_ADDR_AXI51_PTR0_1    = 12'h1cc,
        LP_ADDR_AXI52_PTR0_0    = 12'h1d0,
        LP_ADDR_AXI52_PTR0_1    = 12'h1d4,
        LP_ADDR_AXI53_PTR0_0    = 12'h1d8,
        LP_ADDR_AXI53_PTR0_1    = 12'h1dc,
        LP_ADDR_AXI54_PTR0_0    = 12'h1e0,
        LP_ADDR_AXI54_PTR0_1    = 12'h1e4,
        LP_ADDR_AXI55_PTR0_0    = 12'h1e8,
        LP_ADDR_AXI55_PTR0_1    = 12'h1ec,
        LP_ADDR_AXI56_PTR0_0    = 12'h1f0,
        LP_ADDR_AXI56_PTR0_1    = 12'h1f4,
        LP_ADDR_AXI57_PTR0_0    = 12'h1f8,
        LP_ADDR_AXI57_PTR0_1    = 12'h1fc,
        LP_ADDR_AXI58_PTR0_0    = 12'h200,
        LP_ADDR_AXI58_PTR0_1    = 12'h204,
        LP_ADDR_AXI59_PTR0_0    = 12'h208,
        LP_ADDR_AXI59_PTR0_1    = 12'h20c,
        LP_ADDR_AXI60_PTR0_0    = 12'h210,
        LP_ADDR_AXI60_PTR0_1    = 12'h214,
        LP_ADDR_AXI61_PTR0_0    = 12'h218,
        LP_ADDR_AXI61_PTR0_1    = 12'h21c,
        LP_ADDR_AXI62_PTR0_0    = 12'h220,
        LP_ADDR_AXI62_PTR0_1    = 12'h224,
        LP_ADDR_AXI63_PTR0_0    = 12'h228,
        LP_ADDR_AXI63_PTR0_1    = 12'h22c,
        LP_ADDR_AXI64_PTR0_0    = 12'h230,
        LP_ADDR_AXI64_PTR0_1    = 12'h234;

    localparam integer LP_SM_WIDTH = 2;

    // Write State Machine
    localparam [LP_SM_WIDTH-1:0]
        SM_WRIDLE   = 2'd0,
        SM_WRDATA   = 2'd1,
        SM_WRRESP   = 2'd2,
        SM_WRRESET  = 2'd3;
    logic [LP_SM_WIDTH-1:0] wstate = SM_WRRESET;

    // Read State Machine
    localparam [LP_SM_WIDTH-1:0]
        SM_RDIDLE   = 2'd0,
        SM_RDDATA   = 2'd1,
        SM_RDRESET  = 2'd3;
    logic [LP_SM_WIDTH-1:0] rstate = SM_RDRESET;

    logic [C_ADDR_WIDTH-1:0]    waddr = 'h0;
    wire  [C_DATA_WIDTH-1:0]    wmask = { {8{wstrb[3]}}, {8{wstrb[2]}}, {8{wstrb[1]}}, {8{wstrb[0]}} };
    // internal registers
    logic                       ap_done_latch   = 1'b0;
    logic                       int_gie         = 1'b0;
    logic                       int_ier         = 1'b0;
    logic                       int_isr         = 1'b0;

    logic [64-1:0] axi00_ptr = 'h0; // use axi00_ptr instead of axi00_ptr0 for false path definition
    assign axi00_ptr0 = axi00_ptr;

    //------------------------AXI WRITE FSM------------------
    // awready = (wstate == SM_WRIDLE);
    // wready  = (wstate == SM_WRDATA);
    // bvalid  = (wstate == SM_WRRESP);
    assign bresp = 2'b00; // OKAY

    // wstate
    always_ff @(posedge aclk) begin
        case (wstate)
            SM_WRIDLE: begin
                if (awvalid) begin
                    awready <= 1'b0;
                    wready  <= 1'b1;
                    waddr   <= awaddr;
                    wstate  <= SM_WRDATA;
                end
            end
            SM_WRDATA: begin
                if (wvalid) begin
                    wready  <= 1'b0;
                    bvalid  <= 1'b1;
                    wstate  <= SM_WRRESP;
                end
            end
            SM_WRRESP: begin
                if (bready) begin
                    awready <= 1'b1;
                    bvalid  <= 1'b0;
                    wstate  <= SM_WRIDLE;
                end
            end
            default: begin // SM_WRRESET
                awready <= 1'b1;
                wready  <= 1'b0;
                bvalid  <= 1'b0;
                wstate  <= SM_WRIDLE;
            end
        endcase

        if (areset) begin
            awready <= 1'b0;
            wready  <= 1'b0;
            bvalid  <= 1'b0;
            wstate  <= SM_WRRESET;
        end
    end

    //------------------------AXI READ FSM-------------------
    // arready = (rstate == SM_RDIDLE);
    // rvalid  = (rstate == SM_RDDATA);
    assign rresp   = 2'b00;  // OKAY
    // rstate
    always_ff @(posedge aclk) begin
        case (rstate)
            SM_RDIDLE: begin
                if (arvalid) begin
                    arready <= 1'b0;
                    rvalid  <= 1'b1;
                    rstate  <= SM_RDDATA;
                end
            end
            SM_RDDATA: begin
                if (rready) begin
                    arready <= 1'b1;
                    rvalid  <= 1'b0;
                    rstate  <= SM_RDIDLE;
                end
            end
            // SM_RDRESET:
            default: begin
                arready <= 1'b1;
                rvalid  <= 1'b0;
                rstate  <= SM_RDIDLE;
            end
        endcase

        if (areset) begin
            arready <= 1'b0;
            rvalid  <= 1'b0;
            rstate  <= SM_RDRESET;
        end
    end

`define CASE_RD_AXI_PTR0(INDEX)                                       \
    LP_ADDR_AXI``INDEX``_PTR0_0: rdata <= axi``INDEX``_ptr0[0+:32];   \
    LP_ADDR_AXI``INDEX``_PTR0_1: rdata <= axi``INDEX``_ptr0[32+:32];

always_ff @(posedge aclk) begin
    if (arvalid & arready) begin
        rdata <= 'h0;
        case (araddr)
            LP_ADDR_AP_CTRL: begin
                rdata[0] <= ap_start;
                rdata[1] <= ap_done_latch;
                rdata[2] <= ap_idle;

                ap_done_latch <= 1'b0; // clear on read
            end
            LP_ADDR_GIE:        rdata[0] <= int_gie;
            LP_ADDR_IER:        rdata[0] <= int_ier;
            LP_ADDR_ISR:        rdata[0] <= int_isr;
            LP_ADDR_SCALAR00_0: rdata <= scalar00[0+:32];
            LP_ADDR_SCALAR01_0: rdata <= scalar01[0+:32];
            LP_ADDR_SCALAR02_0: rdata <= scalar02[0+:32];
            LP_ADDR_SCALAR03_0: rdata <= scalar03[0+:32];

            LP_ADDR_AXI00_PTR0_0: rdata <= axi00_ptr[0+:32];
            LP_ADDR_AXI00_PTR0_1: rdata <= axi00_ptr[32+:32];

            // remove read back to opimize ressource
            // `CASE_RD_AXI_PTR0(01 -> 64)

            default: rdata <= 'h0;
        endcase
    end

    if (ap_done) begin
        ap_done_latch <= 1'b1;
    end

    if (areset) begin
        ap_done_latch <= 1'b0;
    end
end

//------------------------Register logic-----------------


// Generate other axi*_ptr0 for memory kernel (C_KRNL_MODE = 1) depending on number of m_axi ports used  C_NUM_USED_M_AXI
`define CASE_WR_AXI_PTR0(INDEX)                                                                                         \
    LP_ADDR_AXI``INDEX``_PTR0_0: begin                                                                                  \
        if ((C_KRNL_MODE == 1) && (INDEX <= C_NUM_USED_M_AXI)) begin                                  \
            axi``INDEX``_ptr0[0+:32]    <= (wdata[0+:32] & wmask[0+:32]) | (axi``INDEX``_ptr0[0+:32] & ~wmask[0+:32]);  \
        end                                                                                                             \
    end                                                                                                                 \
    LP_ADDR_AXI``INDEX``_PTR0_1: begin                                                                                  \
        if ((C_KRNL_MODE == 1) && (INDEX <= C_NUM_USED_M_AXI)) begin                                  \
            axi``INDEX``_ptr0[32+:32]   <= (wdata[0+:32] & wmask[0+:32]) | (axi``INDEX``_ptr0[32+:32] & ~wmask[0+:32]); \
        end                                                                                                             \
    end

always_ff @(posedge aclk) begin
    interrupt = int_gie & (|int_isr);

    // ap_start
    if (ap_done) begin
        ap_start <= 1'b0;
    end

    // scalar00
    if (wvalid & wready) begin
        case (waddr)
            LP_ADDR_AP_CTRL: begin
                 if (wstrb[0] & wdata[0]) begin
                    ap_start <= 1'b1;
                end
            end
            LP_ADDR_GIE: begin
                 if (wstrb[0]) begin
                    int_gie <= wdata[0];
                end
            end
            LP_ADDR_IER: begin
                 if (wstrb[0]) begin
                    int_ier <= wdata[0];
                end
            end
            LP_ADDR_ISR: begin
                 if (wstrb[0]) begin
                    int_isr <= int_isr ^ wdata[0];
                end
            end
            LP_ADDR_SCALAR00_0: begin
                scalar00[0+:32] <= (wdata[0+:32] & wmask[0+:32]) | (scalar00[0+:32] & ~wmask[0+:32]);
            end
            LP_ADDR_SCALAR01_0: begin
                scalar01[0+:32] <= (wdata[0+:32] & wmask[0+:32]) | (scalar01[0+:32] & ~wmask[0+:32]);
            end
            LP_ADDR_SCALAR02_0: begin
                scalar02[0+:32] <= (wdata[0+:32] & wmask[0+:32]) | (scalar02[0+:32] & ~wmask[0+:32]);
            end
            LP_ADDR_SCALAR03_0: begin
                scalar03[0+:32] <= (wdata[0+:32] & wmask[0+:32]) | (scalar03[0+:32] & ~wmask[0+:32]);
            end

            LP_ADDR_AXI00_PTR0_0: begin
                axi00_ptr[0+:32]    <= (wdata[0+:32] & wmask[0+:32]) | (axi00_ptr[0+:32] & ~wmask[0+:32]);
            end
            LP_ADDR_AXI00_PTR0_1: begin
                axi00_ptr[32+:32]   <= (wdata[0+:32] & wmask[0+:32]) | (axi00_ptr[32+:32] & ~wmask[0+:32]);
            end

            `CASE_WR_AXI_PTR0(01)
            `CASE_WR_AXI_PTR0(02)
            `CASE_WR_AXI_PTR0(03)
            `CASE_WR_AXI_PTR0(04)
            `CASE_WR_AXI_PTR0(05)
            `CASE_WR_AXI_PTR0(06)
            `CASE_WR_AXI_PTR0(07)
            `CASE_WR_AXI_PTR0(08)
            `CASE_WR_AXI_PTR0(09)
            `CASE_WR_AXI_PTR0(10)
            `CASE_WR_AXI_PTR0(11)
            `CASE_WR_AXI_PTR0(12)
            `CASE_WR_AXI_PTR0(13)
            `CASE_WR_AXI_PTR0(14)
            `CASE_WR_AXI_PTR0(15)
            `CASE_WR_AXI_PTR0(16)
            `CASE_WR_AXI_PTR0(17)
            `CASE_WR_AXI_PTR0(18)
            `CASE_WR_AXI_PTR0(19)
            `CASE_WR_AXI_PTR0(20)
            `CASE_WR_AXI_PTR0(21)
            `CASE_WR_AXI_PTR0(22)
            `CASE_WR_AXI_PTR0(23)
            `CASE_WR_AXI_PTR0(24)
            `CASE_WR_AXI_PTR0(25)
            `CASE_WR_AXI_PTR0(26)
            `CASE_WR_AXI_PTR0(27)
            `CASE_WR_AXI_PTR0(28)
            `CASE_WR_AXI_PTR0(29)
            `CASE_WR_AXI_PTR0(30)
            `CASE_WR_AXI_PTR0(31)
            `CASE_WR_AXI_PTR0(32)
            `CASE_WR_AXI_PTR0(33)
            `CASE_WR_AXI_PTR0(34)
            `CASE_WR_AXI_PTR0(35)
            `CASE_WR_AXI_PTR0(36)
            `CASE_WR_AXI_PTR0(37)
            `CASE_WR_AXI_PTR0(38)
            `CASE_WR_AXI_PTR0(39)
            `CASE_WR_AXI_PTR0(40)
            `CASE_WR_AXI_PTR0(41)
            `CASE_WR_AXI_PTR0(42)
            `CASE_WR_AXI_PTR0(43)
            `CASE_WR_AXI_PTR0(44)
            `CASE_WR_AXI_PTR0(45)
            `CASE_WR_AXI_PTR0(46)
            `CASE_WR_AXI_PTR0(47)
            `CASE_WR_AXI_PTR0(48)
            `CASE_WR_AXI_PTR0(49)
            `CASE_WR_AXI_PTR0(50)
            `CASE_WR_AXI_PTR0(51)
            `CASE_WR_AXI_PTR0(52)
            `CASE_WR_AXI_PTR0(53)
            `CASE_WR_AXI_PTR0(54)
            `CASE_WR_AXI_PTR0(55)
            `CASE_WR_AXI_PTR0(56)
            `CASE_WR_AXI_PTR0(57)
            `CASE_WR_AXI_PTR0(58)
            `CASE_WR_AXI_PTR0(59)
            `CASE_WR_AXI_PTR0(60)
            `CASE_WR_AXI_PTR0(61)
            `CASE_WR_AXI_PTR0(62)
            `CASE_WR_AXI_PTR0(63)
            `CASE_WR_AXI_PTR0(64)

            default : $display("Illegal write address received in krnl_xbtest_control_s_axi");
        endcase
    end

    // int_isr
    if (int_ier & ap_done) begin
        int_isr <= 1'b1;
    end

    if (areset) begin
        ap_start    <= 1'b0;
        int_gie     <= 1'b0;
        int_ier     <= 1'b0;
        int_isr     <= 1'b0;
    end
end

endmodule: krnl_xbtest_control_s_axi
`default_nettype wire
