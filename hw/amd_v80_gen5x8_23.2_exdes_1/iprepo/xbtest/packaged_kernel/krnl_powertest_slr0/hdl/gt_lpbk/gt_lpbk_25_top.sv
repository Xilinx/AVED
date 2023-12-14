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

module gt_lpbk_25_top #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer C_BLOCK_ID            = 3,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_GT_INDEX               ,
    parameter integer C_PLRAM_ADDR_WIDTH    = 64,
    parameter integer C_PLRAM_DATA_WIDTH    = 32,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer DISABLE_SIM_ASSERT    = 0
) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,

    input  wire         QSFP_CK_P,
    input  wire         QSFP_CK_N,
    input  wire [3 : 0] QSFP_RX_N,
    input  wire [3 : 0] QSFP_RX_P,
    output wire [3 : 0] QSFP_TX_N,
    output wire [3 : 0] QSFP_TX_P,


    output wire         plram_awvalid,
    input  wire         plram_awready,
    output wire [63:0]  plram_awaddr,
    output wire [7:0]   plram_awlen,
    output wire         plram_wvalid,
    input  wire         plram_wready,
    output wire [31:0]  plram_wdata,
    output wire [3:0]   plram_wstrb,
    output wire         plram_wlast,
    input  wire         plram_bvalid,
    output wire         plram_bready,
    output wire         plram_arvalid,
    input  wire         plram_arready,
    output wire [63:0]  plram_araddr,
    output wire [7:0]   plram_arlen,
    input  wire         plram_rvalid,
    output wire         plram_rready,
    input  wire [31:0]  plram_rdata,
    input  wire         plram_rlast,


    input  wire [C_PLRAM_ADDR_WIDTH-1:0]  axi00_ptr0,
    input  wire [32-1:0]                  scalar00,
    input  wire [32-1:0]                  scalar01,
    input  wire [32-1:0]                  scalar02,
    input  wire [32-1:0]                  scalar03,
    input  wire                           start_pulse,
    output logic                          done_pulse

);

    wire         cs;
    wire         we;
    wire  [11:0] addr;
    wire  [31:0] wdata;
    logic [31:0] rdata;
    logic        cmd_cmplt;

    wire [3 : 0]    rxlpmen;
    wire [3 : 0]    txpolarity;
    wire [19 : 0]   txdiffctrl;
    wire [19 : 0]   txpostcursor;
    wire [19 : 0]   txprecursor;
    wire            gt_reset;

    wire          gt_drpclk;

    cu_config  # (
        .C_PLRAM_ADDR_WIDTH ( C_PLRAM_ADDR_WIDTH    ),
        .C_PLRAM_DATA_WIDTH ( C_PLRAM_DATA_WIDTH    )
    ) u_cu_config (
        .clk              ( ap_clk             ),
        .rst              ( ap_rst             ),

        .plram_awvalid    ( plram_awvalid    ),
        .plram_awready    ( plram_awready    ),
        .plram_awaddr     ( plram_awaddr     ),
        .plram_awlen      ( plram_awlen      ),
        .plram_wvalid     ( plram_wvalid     ),
        .plram_wready     ( plram_wready     ),
        .plram_wdata      ( plram_wdata      ),
        .plram_wstrb      ( plram_wstrb      ),
        .plram_wlast      ( plram_wlast      ),
        .plram_bvalid     ( plram_bvalid     ),
        .plram_bready     ( plram_bready     ),
        .plram_arvalid    ( plram_arvalid    ),
        .plram_arready    ( plram_arready    ),
        .plram_araddr     ( plram_araddr     ),
        .plram_arlen      ( plram_arlen      ),
        .plram_rvalid     ( plram_rvalid     ),
        .plram_rready     ( plram_rready     ),
        .plram_rdata      ( plram_rdata      ),
        .plram_rlast      ( plram_rlast      ),

        .axi00_ptr0       ( axi00_ptr0       ),
        .scalar00         ( scalar00         ),
        .scalar01         ( scalar01         ),
        .scalar02         ( scalar02         ),
        .scalar03         ( scalar03         ),
        .start_pulse      ( start_pulse      ),
        .done_pulse       ( done_pulse       ),

        .cs               ( cs               ),
        .wr_en            ( we               ),
        .addr             ( addr             ),
        .wdata            ( wdata            ),
        .rdata            ( rdata            ),
        .cmd_cmplt        ( cmd_cmplt        )
    );


    BUFGCE_DIV #(
        .BUFGCE_DIVIDE(2)
    ) DRP_CK (
        .I      (ap_clk     ),
        .CE     (1'b1       ),
        .CLR    (1'b0       ),
        .O      (gt_drpclk  )
    );

    gt_lpbk_25_reg_array #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
        .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ         ),
        .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ         ),
        .C_GT_INDEX         ( C_GT_INDEX            ),
        .DEST_SYNC_FF       ( DEST_SYNC_FF          ),
        .C_BLOCK_ID         ( C_BLOCK_ID            )
    ) u_reg_array (
        .ap_clk             ( ap_clk        ),
        .ap_clk_cont        ( ap_clk_cont   ),
        .ap_rst             ( ap_rst        ),

        .rxlpmen            ( rxlpmen       ),
        .txpolarity         ( txpolarity    ),
        .txdiffctrl         ( txdiffctrl    ),
        .txpostcursor       ( txpostcursor  ),
        .txprecursor        ( txprecursor   ),
        .gt_reset           ( gt_reset      ),

        .cs                 ( cs            ),
        .we                 ( we            ),
        .addr               ( addr          ),
        .wdata              ( wdata         ),
        .rdata              ( rdata         ),
        .cmd_cmplt          ( cmd_cmplt     )
    );


    gt_lpbk_25_core #(

        .DEST_SYNC_FF       (DEST_SYNC_FF   )

    ) gt_lpbk_25_core  (

        .ap_clk             ( ap_clk        ),
        .ap_rst             ( ap_rst        ),
        .drp_clk            ( gt_drpclk     ),

        .QSFP_CK_P          ( QSFP_CK_P ),
        .QSFP_CK_N          ( QSFP_CK_N ),
        .QSFP_RX_N          ( QSFP_RX_N ),
        .QSFP_RX_P          ( QSFP_RX_P ),
        .QSFP_TX_N          ( QSFP_TX_N ),
        .QSFP_TX_P          ( QSFP_TX_P ),

        .rxlpmen            ( rxlpmen     ),
        .txpolarity         ( txpolarity  ),
        .txdiffctrl         ( txdiffctrl  ),
        .txpostcursor       ( txpostcursor),
        .txprecursor        ( txprecursor ),

        .ctrl_gt_reset      ( gt_reset )
    );


endmodule : gt_lpbk_25_top
`default_nettype wire
