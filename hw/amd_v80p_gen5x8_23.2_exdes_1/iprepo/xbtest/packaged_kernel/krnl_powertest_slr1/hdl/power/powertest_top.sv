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

module powertest_top #(
    parameter         C_FAMILY              = "no_family",
    parameter integer C_MAJOR_VERSION       = 3,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer C_BLOCK_ID            = 0,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_KRNL_SLR            = 0,    // Kernel SLR (SLR0 = 0, SLR1 = 1, SLR2 = 2, SLR3 = 3)
    parameter integer C_THROTTLE_MODE       = 1,
    parameter integer C_USE_AIE             = 0,    // 0: Disable AIE, 1: Enable AIE
    parameter integer C_AXIS_AIE_DATA_WIDTH = 128,
    parameter integer C_PLRAM_ADDR_WIDTH    = 64,
    parameter integer C_PLRAM_DATA_WIDTH    = 32,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer DISABLE_SIM_ASSERT    = 0
    ) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         ap_clk_2,
    input  wire         ap_clk_2_cont,
    input  wire         ap_rst_2,

    input  wire         watchdog_alarm_in,

    input  wire         pwr_clk_in,
    output wire         pwr_clk_out,
    input  wire         pwr_throttle_in,
    output wire         pwr_throttle_out,
    input  wire         pwr_FF_en_in,
    input  wire         pwr_DSP_en_in,
    input  wire         pwr_BRAM_en_in,
    input  wire         pwr_URAM_en_in,
    output wire         pwr_FF_en_out,
    output wire         pwr_DSP_en_out,
    output wire         pwr_BRAM_en_out,
    output wire         pwr_URAM_en_out,

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

    output logic [C_AXIS_AIE_DATA_WIDTH-1:0]    m_axis_aie0_tdata,
    output logic                                m_axis_aie0_tvalid,
    input  wire                                 m_axis_aie0_tready,
    output wire  [C_AXIS_AIE_DATA_WIDTH/8-1:0]  m_axis_aie0_tkeep,
    output wire                                 m_axis_aie0_tlast,

    input  wire [C_AXIS_AIE_DATA_WIDTH-1:0]     s_axis_aie0_tdata,
    input  wire                                 s_axis_aie0_tvalid,
    output wire                                 s_axis_aie0_tready,
    input  wire  [C_AXIS_AIE_DATA_WIDTH/8-1:0]  s_axis_aie0_tkeep,
    input  wire                                 s_axis_aie0_tlast,

    input  wire [C_PLRAM_ADDR_WIDTH-1:0]  axi00_ptr0,
    input  wire [32-1:0]                  scalar00,
    input  wire [32-1:0]                  scalar01,
    input  wire [32-1:0]                  scalar02,
    input  wire [32-1:0]                  scalar03,
    input  wire                           start_pulse,
    output logic                          done_pulse

);

    localparam integer THROTTLE_MODE = ( C_THROTTLE_MODE >= 2 ) ? C_THROTTLE_MODE-2 : C_THROTTLE_MODE; // when clk comes from outside, selected use of clk or throttle

    wire         cs;
    wire         we;
    wire  [11:0] addr;
    wire  [31:0] wdata;
    logic [31:0] rdata;
    logic        cmd_cmplt;

    wire            pwr_clk, throttle;
    wire            ctrl_pwr_FF_en, ctrl_pwr_DSP_en, ctrl_pwr_BRAM_en, ctrl_pwr_URAM_en, ctrl_pwr_AIE_en;
    wire            ctrl_FF_en, ctrl_DSP_en, ctrl_BRAM_en, ctrl_URAM_en;
    wire            FF_en, DSP_en, BRAM_en, URAM_en;
    wire            ctrl_tog_update;
    wire    [9:0 ]  ctrl_tog_pattern;

    wire    [31:0]  m_aie0_cycle_cnt;
    wire    [31:0]  m_aie0_tready_cnt;
    wire    [31:0]  m_aie0_txfer_cnt;
    wire    [31:0]  s_aie0_txfer_cnt;

    ///////////////////////////////////////////////////////////////////////////////
    // Power CU host interface
    ///////////////////////////////////////////////////////////////////////////////

    generate

        if ( (C_THROTTLE_MODE <= 1) || (C_USE_AIE == 1) ) begin: cu_cfg

            cu_config  # (
                .C_ADDR_WIDTH       ( $size(addr)           ),
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

            powertest_reg_array #(
                .C_MAJOR_VERSION            ( C_MAJOR_VERSION                           ),
                .C_MINOR_VERSION            ( C_MINOR_VERSION                           ),
                .C_BUILD_VERSION            ( C_BUILD_VERSION                           ),
                .C_CLOCK0_FREQ              ( C_CLOCK0_FREQ                             ),
                .C_CLOCK1_FREQ              ( C_CLOCK1_FREQ                             ),
                .DEST_SYNC_FF               ( DEST_SYNC_FF                              ),
                .C_BLOCK_ID                 ( C_BLOCK_ID                                ),
                .C_KRNL_SLR                 ( C_KRNL_SLR                                ),
                .C_THROTTLE_MODE            ( C_THROTTLE_MODE                           ),
                .C_USE_AIE                  ( C_USE_AIE                                 ),
                .C_REG_BLOCK_SIZE_ARRAY     ( xbtest_pkg::C_REG_BLOCK_SIZE_ARRAY        ),
                .C_DSP48E2_COL_SIZES_ARRRAY ( xbtest_pkg::C_DSP48E2_COL_SIZES_ARRRAY    ),
                .C_RAMB36_COL_SIZES_ARRRAY  ( xbtest_pkg::C_RAMB36_COL_SIZES_ARRRAY     ),
                .C_URAM288_COL_SIZES_ARRRAY ( xbtest_pkg::C_URAM288_COL_SIZES_ARRRAY    )
            ) u_reg_array (
                .ap_clk             ( ap_clk           ),
                .ap_clk_cont        ( ap_clk_cont      ),
                .ap_rst             ( ap_rst           ),
                .ap_clk_2           ( ap_clk_2         ),
                .ap_clk_2_cont      ( ap_clk_2_cont    ),
                .ap_rst_2           ( ap_rst_2         ),
                .watchdog_alarm_in  ( watchdog_alarm_in),

                .cs                 ( cs               ),
                .we                 ( we               ),
                .addr               ( addr             ),
                .wdata              ( wdata            ),
                .rdata              ( rdata            ),
                .cmd_cmplt          ( cmd_cmplt        ),

                .ctrl_tog_pattern   ( ctrl_tog_pattern ),
                .ctrl_tog_update    ( ctrl_tog_update  ),
                .ctrl_pwr_FF_en     ( ctrl_pwr_FF_en   ),
                .ctrl_pwr_DSP_en    ( ctrl_pwr_DSP_en  ),
                .ctrl_pwr_BRAM_en   ( ctrl_pwr_BRAM_en ),
                .ctrl_pwr_URAM_en   ( ctrl_pwr_URAM_en ),
                .ctrl_pwr_AIE_en    ( ctrl_pwr_AIE_en  ),

                .m_aie0_cycle_cnt   ( m_aie0_cycle_cnt  ),
                .m_aie0_tready_cnt  ( m_aie0_tready_cnt ),
                .m_aie0_txfer_cnt   ( m_aie0_txfer_cnt  ),
                .s_aie0_txfer_cnt   ( s_aie0_txfer_cnt  )
            );

            wire [3:0] xpm_cdc_in,xpm_cdc_out;

            assign xpm_cdc_in = { ctrl_pwr_FF_en, ctrl_pwr_DSP_en, ctrl_pwr_BRAM_en, ctrl_pwr_URAM_en };

            xpm_cdc_array_single #(
                .DEST_SYNC_FF   ( DEST_SYNC_FF      ),
                .INIT_SYNC_FF   ( 0                 ),
                .SRC_INPUT_REG  ( 1                 ),
                .SIM_ASSERT_CHK ( 1                 ),
                .WIDTH          ( $size(xpm_cdc_in) )
            )
            xpm_cdc_control (
                .src_in   ( xpm_cdc_in  ),
                .src_clk  ( ap_clk      ),
                .dest_out ( xpm_cdc_out ),
                .dest_clk ( pwr_clk   )
            );

            assign { ctrl_FF_en, ctrl_DSP_en, ctrl_BRAM_en, ctrl_URAM_en } = xpm_cdc_out;

            //------------------------------------
            // dont_touch ="true" for false path constraints defined in gen_power_floorplan.tcl
            //  these signals are going directly to resources
            (*dont_touch ="true"*) logic  ctrl_enable_reg, ctrl_enable_dsp, ctrl_enable_bram, ctrl_enable_uram;

            always_ff @(posedge pwr_clk) begin
                ctrl_enable_reg     <= ctrl_FF_en;
                ctrl_enable_dsp     <= ctrl_DSP_en;
                ctrl_enable_bram    <= ctrl_BRAM_en;
                ctrl_enable_uram    <= ctrl_URAM_en;
            end
            assign FF_en    = ctrl_enable_reg;
            assign DSP_en   = ctrl_enable_dsp;
            assign BRAM_en  = ctrl_enable_bram;
            assign URAM_en  = ctrl_enable_uram;

        end else begin

            assign FF_en    = pwr_FF_en_in;
            assign DSP_en   = pwr_DSP_en_in;
            assign BRAM_en  = pwr_BRAM_en_in;
            assign URAM_en  = pwr_URAM_en_in;

        end
    endgenerate

    assign pwr_FF_en_out   = FF_en  ;
    assign pwr_DSP_en_out  = DSP_en ;
    assign pwr_BRAM_en_out = BRAM_en;
    assign pwr_URAM_en_out = URAM_en;

    ///////////////////////////////////////////////////////////////////////////////
    // Power CU DSP/BRAM/URAM/FF throttle control on ap_clk_2 using C_THROTTLE_MODE
    ///////////////////////////////////////////////////////////////////////////////
    generate

        if (C_THROTTLE_MODE >= 2) begin: external_pwr_clk

            assign pwr_clk  = pwr_clk_in;
            assign throttle = pwr_throttle_in;

        end else begin: pwr_ctrl

            clk_throttling #(
                .C_THROTTLE_MODE    ( C_THROTTLE_MODE ),
                .SYNTH_SIZE         ( 10              ),
                .DEST_SYNC_FF       ( DEST_SYNC_FF    ),
                .RST_ACTIVE_HIGH    ( 1               ),
                .SIM_DEVICE         (xbtest_pkg::SIM_DEVICE_BUFGCE_DIV)
            ) clk_throttle (
                .Clk_In         ( ap_clk_2          ),
                .Rst_In         ( ap_rst_2          ),
                .Enable         ( 1'b1              ),

                .Rate_Upd_Tog   ( ctrl_tog_update   ),
                .Rate_In        ( ctrl_tog_pattern  ),

                .Clk_Out        ( pwr_clk           ),
                .Throttle       ( throttle          )
            );

        end

    endgenerate

    assign pwr_clk_out = pwr_clk;
    assign pwr_throttle_out = throttle;


    reg_chain_top #(
        .C_THROTTLE_MODE        ( THROTTLE_MODE                         ),
        .C_REG_BLOCK_SIZE_ARRAY ( xbtest_pkg::C_REG_BLOCK_SIZE_ARRAY    )
    ) inst_reg_chain_top (
        .clk        ( pwr_clk   ),
        .reset      ( 1'b0      ),
        .enable     ( FF_en     ),
        .tog_en     ( throttle  )
    );

    dsp_top #(
        .C_FAMILY           ( C_FAMILY                                  ),
        .C_THROTTLE_MODE    ( THROTTLE_MODE                             ),
        .C_COL_SIZES_ARRRAY ( xbtest_pkg::C_DSP48E2_COL_SIZES_ARRRAY    )
    ) inst_dsp_top (
        .clk        ( pwr_clk   ),
        .reset      ( 1'b0      ),
        .enable     ( DSP_en    ),
        .tog_en     ( throttle  )
    );

    bram_top #(
        .C_FAMILY           ( C_FAMILY                              ),
        .C_THROTTLE_MODE    ( THROTTLE_MODE                         ),
        .C_COL_SIZES_ARRRAY ( xbtest_pkg::C_RAMB36_COL_SIZES_ARRRAY ),
        .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT )
    ) inst_bram_top (
        .clk        ( pwr_clk   ),
        .reset      ( 1'b0      ),
        .enable     ( BRAM_en   ),
        .tog_en     ( throttle  )
    );

    uram_top #(
        .C_FAMILY           ( C_FAMILY                                  ),
        .C_THROTTLE_MODE    ( THROTTLE_MODE                             ),
        .C_COL_SIZES_ARRRAY ( xbtest_pkg::C_URAM288_COL_SIZES_ARRRAY    ),
        .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT                        )
    ) inst_uram_top (
        .clk        ( pwr_clk   ),
        .reset      ( 1'b0      ),
        .enable     ( URAM_en   ),
        .tog_en     ( throttle  )
    );

    generate
        if (C_USE_AIE == 1) begin
            wire throttle_aie;

            // Power CU AIE throttle control on ap_clk, always using CE mode
            clk_throttling #(
                .C_THROTTLE_MODE        ( 0     ),
                .SYNTH_SIZE             ( 10    ),
                .DEST_SYNC_FF           ( 2     ), // Lowest DEST_SYNC_FF as CDC not needed
                .RST_ACTIVE_HIGH        ( 1     ),
                .SIM_DEVICE(xbtest_pkg::SIM_DEVICE_BUFGCE_DIV)
            ) clk_throttle_aie (
                .Clk_In         ( ap_clk            ),
                .Rst_In         ( ap_rst            ),
                .Enable         ( 1'b1              ),

                .Rate_Upd_Tog   ( ctrl_tog_update   ),
                .Rate_In        ( ctrl_tog_pattern  ),

                .Clk_Out        (                   ), // Here, Clk_Out = Clk_In  !
                .Throttle       ( throttle_aie      )
            );

            // Power CU AIE AXI stream interfaces
            toggle_axis #(
                .C_CLOCK0_FREQ        ( C_CLOCK0_FREQ           ),
                .DEST_SYNC_FF         ( DEST_SYNC_FF            ),
                .C_AXIS_DATA_WIDTH    ( C_AXIS_AIE_DATA_WIDTH   )
            ) inst_toggle_aie_axis0 (
                .clk                ( ap_clk                ),
                .clk_cont           ( ap_clk_cont           ),
                .rst                ( ap_rst                ),
                .enable             ( ctrl_pwr_AIE_en       ),
                .tog_en             ( throttle_aie          ),

                .m_axis_tdata       ( m_axis_aie0_tdata     ),
                .m_axis_tvalid      ( m_axis_aie0_tvalid    ),
                .m_axis_tready      ( m_axis_aie0_tready    ),
                .m_axis_tkeep       ( m_axis_aie0_tkeep     ),
                .m_axis_tlast       ( m_axis_aie0_tlast     ),

                .s_axis_tdata       ( s_axis_aie0_tdata     ),
                .s_axis_tvalid      ( s_axis_aie0_tvalid    ),
                .s_axis_tready      ( s_axis_aie0_tready    ),
                .s_axis_tkeep       ( s_axis_aie0_tkeep     ),
                .s_axis_tlast       ( s_axis_aie0_tlast     ),

                .cycle_cnt          ( m_aie0_cycle_cnt      ),
                .m_tready_cnt       ( m_aie0_tready_cnt     ),
                .m_txfer_cnt        ( m_aie0_txfer_cnt      ),
                .s_txfer_cnt        ( s_aie0_txfer_cnt      )
            );
        end
    endgenerate

endmodule : powertest_top
`default_nettype wire
