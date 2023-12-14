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

module memtest_top #(
    parameter integer C_MAJOR_VERSION           = 0,    // Major version
    parameter integer C_MINOR_VERSION           = 0,    // Minor version
    parameter integer C_BUILD_VERSION           = 0,    // Build version
    parameter integer C_CLOCK0_FREQ             = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ             = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer DEST_SYNC_FF              = 4,
    parameter integer C_BLOCK_ID                = 1,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_MEM_KRNL_INST           = 0,    // Memory kernel instance
    parameter integer C_MEM_KRNL_CORE_IDX       = 0,    // Memory kernel core index
    parameter integer C_NUM_MAX_M_AXI           = 32,   // Maximum number of M_AXI ports (for memory kernel)
    parameter integer C_NUM_USED_M_AXI          = 1,    // Number of used M_AXI ports 1..32 (enables M01_AXI .. M32_AXI for memory kernel)
    parameter integer C_MEM_TYPE                = 0,    // 1 single-channel 2 multi-channel
    parameter integer C_USE_AXI_ID              = 1,    // 1 use axi id, 0 disable
    parameter integer C_M_AXI_THREAD_ID_WIDTH   = 2,
    parameter integer C_M_AXI_ADDR_WIDTH        = 64,
    parameter integer C_M_AXI_DATA_WIDTH        = 512,
    parameter integer C_EXTRA_CHIPSCOPE         = 0,
    parameter integer C_DISABLE_BURST_TIME_MEAS = 1
) (
    input  wire                                 ap_clk,
    input  wire                                 ap_clk_cont,
    input  wire                                 ap_rst,

    input  wire [C_M_AXI_ADDR_WIDTH-1:0]        axi_addr_ptr,

    // AXI4 master interface
    output wire [C_M_AXI_THREAD_ID_WIDTH-1:0]   m_axi_awid,
    output wire                                 m_axi_awvalid,
    input  wire                                 m_axi_awready,
    output wire [C_M_AXI_ADDR_WIDTH-1:0]        m_axi_awaddr,
    output wire [8-1:0]                         m_axi_awlen,
    output wire                                 m_axi_wvalid,
    input  wire                                 m_axi_wready,
    output wire [C_M_AXI_DATA_WIDTH-1:0]        m_axi_wdata,
    output wire [C_M_AXI_DATA_WIDTH/8-1:0]      m_axi_wstrb,
    output wire                                 m_axi_wlast,
    input  wire [C_M_AXI_THREAD_ID_WIDTH-1:0]   m_axi_bid,
    input  wire                                 m_axi_bvalid,
    output wire                                 m_axi_bready,

    output wire [C_M_AXI_THREAD_ID_WIDTH-1:0]   m_axi_arid,
    output wire                                 m_axi_arvalid,
    input  wire                                 m_axi_arready,
    output wire [C_M_AXI_ADDR_WIDTH-1:0]        m_axi_araddr,
    output wire [8-1:0]                         m_axi_arlen,
    input  wire [C_M_AXI_THREAD_ID_WIDTH-1:0]   m_axi_rid,
    input  wire                                 m_axi_rvalid,
    output wire                                 m_axi_rready,
    input  wire [C_M_AXI_DATA_WIDTH-1:0]        m_axi_rdata,
    input  wire                                 m_axi_rlast,

    input  wire                                 toggle_1_sec,
    input  wire                                 watchdog_alarm,

    input  wire                                 cs,
    input  wire                                 we,
    input  wire [20:0]                          addr,
    input  wire [31:0]                          wdata,
    output wire [31:0]                          rdata,
    output wire                                 cmd_cmplt
);

///////////////////////////////////////////////////////////////////////
// Local Parameters
///////////////////////////////////////////////////////////////////////

localparam integer C_NUM_ID = 2**(C_M_AXI_THREAD_ID_WIDTH);
localparam integer C_XFER_SIZE_WIDTH        = 15;
localparam integer C_NUM_XFER_SIZE_WIDTH    = 32; // TODO see if we want to make this depend on the memory size


localparam integer C_STAT_MEAS_SIZE     = 32; // Max 32
localparam integer C_STAT_TOTAL_SIZE    = 64; // Max 64
localparam integer C_STAT_INST_SIZE     = 32; // Max 32
localparam integer C_STAT_ERR_CNT_SIZE  = 8;  // Max 32

///////////////////////////////////////////////////////////////////////
// Wires and Logic
///////////////////////////////////////////////////////////////////////
wire                                ctrl_stop_tog;
wire                                ctrl_update_cfg_tog;
wire [1:0]                          ctrl_test_mode;
wire                                ctrl_reset;
wire                                ctrl_clear_err_tog;

wire [63:0]                         ctrl_wr_start_addr;
wire [63:0]                         ctrl_rd_start_addr;

wire [8-1:0]                        ctrl_wr_burst_size;
wire [8-1:0]                        ctrl_rd_burst_size;

wire [C_NUM_XFER_SIZE_WIDTH-1:0]    ctrl_wr_num_xfer;
wire [C_NUM_XFER_SIZE_WIDTH-1:0]    ctrl_rd_num_xfer;

wire                                ctrl_wr_burst_req_rate_en;
wire [31:0]                         ctrl_wr_burst_req_rate;
wire                                ctrl_rd_burst_req_rate_en;
wire [31:0]                         ctrl_rd_burst_req_rate;

wire                                ctrl_wr_outstanding_en;
wire [8:0]                          ctrl_wr_outstanding;
wire                                ctrl_rd_outstanding_en;
wire [8:0]                          ctrl_rd_outstanding;

wire                                ctrl_axi_id_en;

wire                                ctrl_user_pattern_en;
wire [31:0]                         ctrl_user_pattern_a;
wire [31:0]                         ctrl_user_pattern_b;

wire                                stat_cfg_updated_tog;

wire [C_STAT_TOTAL_SIZE-1:0]        stat_wr_burst_time_total;
wire [C_STAT_INST_SIZE-1:0]         stat_wr_burst_time_inst;
wire [C_STAT_MEAS_SIZE-1:0]         stat_wr_burst_time_min;
wire [C_STAT_MEAS_SIZE-1:0]         stat_wr_burst_time_max;

wire [C_STAT_TOTAL_SIZE-1:0]        stat_rd_burst_time_total;
wire [C_STAT_INST_SIZE-1:0]         stat_rd_burst_time_inst;
wire [C_STAT_MEAS_SIZE-1:0]         stat_rd_burst_time_min;
wire [C_STAT_MEAS_SIZE-1:0]         stat_rd_burst_time_max;

wire [C_STAT_TOTAL_SIZE-1:0]        stat_wr_burst_latency_total;
wire [C_STAT_INST_SIZE-1:0]         stat_wr_burst_latency_inst;
wire [C_STAT_MEAS_SIZE-1:0]         stat_wr_burst_latency_min;
wire [C_STAT_MEAS_SIZE-1:0]         stat_wr_burst_latency_max;

wire [C_STAT_TOTAL_SIZE-1:0]        stat_rd_burst_latency_total;
wire [C_STAT_INST_SIZE-1:0]         stat_rd_burst_latency_inst;
wire [C_STAT_MEAS_SIZE-1:0]         stat_rd_burst_latency_min;
wire [C_STAT_MEAS_SIZE-1:0]         stat_rd_burst_latency_max;

wire                                suppress_error;

wire                                stat_gen_seed_err_latch;
wire                                stat_term_seed_err_latch;
wire                                stat_term_err_latch;
wire [C_STAT_ERR_CNT_SIZE-1:0]      stat_term_err_cnt;

wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_wid;

wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  Gen_ID;
wire                                Gen_Valid;
wire                                Gen_Last;
wire                                Gen_Ready;

wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  m_axi_awid_int;
wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  m_axi_arid_int;

wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_wid_int;
wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  m_axi_bid_int;
wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  m_axi_rid_int;


///////////////////////////////////////////////////////////////////////
// Assign Output Signals
///////////////////////////////////////////////////////////////////////
generate
    if (C_USE_AXI_ID == 1) begin

        assign m_axi_awid       = m_axi_awid_int;
        assign m_axi_arid       = m_axi_arid_int;

        assign axi_wid_int      = axi_wid;
        assign m_axi_bid_int    = m_axi_bid;
        assign m_axi_rid_int    = m_axi_rid;

    end else begin

        assign m_axi_awid       = '0;
        assign m_axi_arid       = '0;

        assign axi_wid_int      = '0;
        assign m_axi_bid_int    = '0;
        assign m_axi_rid_int    = '0;

    end
endgenerate

assign m_axi_wstrb  = '{(C_M_AXI_DATA_WIDTH/8){1'b1}};
assign m_axi_bready = 1'b1;
assign m_axi_rready = 1'b1;


///////////////////////////////////////////////////////////////////////
// Timestamps
///////////////////////////////////////////////////////////////////////
logic           stat_toggle_1_sec       = '0;
logic [15:0]    stat_timestamp_1_sec    = '0;
logic [31:0]    stat_timestamp          = '0;

always_ff @(posedge ap_clk) begin

    stat_timestamp      <= stat_timestamp + 1;
    stat_toggle_1_sec   <= toggle_1_sec;

    if (stat_toggle_1_sec ^ toggle_1_sec) begin

        stat_timestamp_1_sec <= stat_timestamp_1_sec + 1;

    end

    if (ap_rst) begin
        stat_timestamp          <= '0;
        stat_toggle_1_sec       <= '0;
        stat_timestamp_1_sec    <= '0;
    end

end

///////////////////////////////////////////////////////////////////////
// Reset Logic
///////////////////////////////////////////////////////////////////////
logic cfg_reset = 1'b1;

always_ff @(posedge ap_clk) begin

    if (ap_rst) begin

        cfg_reset <= 1'b1;

    end else begin

        cfg_reset <= ctrl_reset;

    end

end

logic [2:0] cfg_reset_d;
pipeline #(
    .C_NUM_PIPELINE ( 2 ),
    .C_DATA_WIDTH   ( $size(cfg_reset_d))
) pipeline_reset (
    .clk        ( ap_clk        ),
    .rst        ( 'b0           ),
    .data_in    ( {$size(cfg_reset_d){cfg_reset}}),
    .data_out   ( cfg_reset_d  )
);

///////////////////////////////////////////////////////////////////////
// Register Array
///////////////////////////////////////////////////////////////////////

(* keep_hierarchy = "yes" *) memtest_reg_array #(
    .C_MAJOR_VERSION            ( C_MAJOR_VERSION           ),
    .C_MINOR_VERSION            ( C_MINOR_VERSION           ),
    .C_BUILD_VERSION            ( C_BUILD_VERSION           ),
    .C_CLOCK0_FREQ              ( C_CLOCK0_FREQ             ),
    .C_CLOCK1_FREQ              ( C_CLOCK1_FREQ             ),
    .C_BLOCK_ID                 ( C_BLOCK_ID                ),
    .C_MEM_KRNL_INST            ( C_MEM_KRNL_INST           ),
    .C_MEM_KRNL_CORE_IDX        ( C_MEM_KRNL_CORE_IDX       ),
    .C_NUM_MAX_M_AXI            ( C_NUM_MAX_M_AXI           ),
    .C_NUM_USED_M_AXI           ( C_NUM_USED_M_AXI          ),
    .C_MEM_TYPE                 ( C_MEM_TYPE                ),
    .C_USE_AXI_ID               ( C_USE_AXI_ID              ),
    .C_M_AXI_DATA_WIDTH         ( C_M_AXI_DATA_WIDTH        ),
    .C_NUM_XFER_SIZE_WIDTH      ( C_NUM_XFER_SIZE_WIDTH     ),
    .C_M_AXI_ADDR_WIDTH         ( C_M_AXI_ADDR_WIDTH        ),
    .C_STAT_MEAS_SIZE           ( C_STAT_MEAS_SIZE          ),
    .C_STAT_TOTAL_SIZE          ( C_STAT_TOTAL_SIZE         ),
    .C_STAT_INST_SIZE           ( C_STAT_INST_SIZE          ),
    .C_STAT_ERR_CNT_SIZE        ( C_STAT_ERR_CNT_SIZE       )
) u_reg_array(
    .clk                            ( ap_clk                        ),
    .rst                            ( ap_rst                        ),

    .cs                             ( cs                            ),
    .we                             ( we                            ),
    .addr                           ( addr                          ),
    .wdata                          ( wdata                         ),
    .rdata                          ( rdata                         ),
    .cmd_cmplt                      ( cmd_cmplt                     ),
    .watchdog_alarm                 ( watchdog_alarm                ),

    .ctrl_stop_tog                  ( ctrl_stop_tog                 ),
    .ctrl_update_cfg_tog            ( ctrl_update_cfg_tog           ),
    .ctrl_test_mode                 ( ctrl_test_mode                ),
    .ctrl_reset                     ( ctrl_reset                    ),
    .ctrl_clear_err_tog             ( ctrl_clear_err_tog            ),
    .ctrl_wr_start_addr             ( ctrl_wr_start_addr            ),
    .ctrl_rd_start_addr             ( ctrl_rd_start_addr            ),
    .ctrl_wr_burst_size             ( ctrl_wr_burst_size            ),
    .ctrl_rd_burst_size             ( ctrl_rd_burst_size            ),
    .ctrl_wr_num_xfer               ( ctrl_wr_num_xfer              ),
    .ctrl_rd_num_xfer               ( ctrl_rd_num_xfer              ),
    .ctrl_wr_burst_req_rate_en      ( ctrl_wr_burst_req_rate_en     ),
    .ctrl_wr_burst_req_rate         ( ctrl_wr_burst_req_rate        ),
    .ctrl_rd_burst_req_rate_en      ( ctrl_rd_burst_req_rate_en     ),
    .ctrl_rd_burst_req_rate         ( ctrl_rd_burst_req_rate        ),
    .ctrl_wr_outstanding_en         ( ctrl_wr_outstanding_en        ),
    .ctrl_wr_outstanding            ( ctrl_wr_outstanding           ),
    .ctrl_rd_outstanding_en         ( ctrl_rd_outstanding_en        ),
    .ctrl_rd_outstanding            ( ctrl_rd_outstanding           ),

    .ctrl_axi_id_en                 ( ctrl_axi_id_en                ),
    .ctrl_user_pattern_en           ( ctrl_user_pattern_en          ),
    .ctrl_user_pattern_a            ( ctrl_user_pattern_a           ),
    .ctrl_user_pattern_b            ( ctrl_user_pattern_b           ),

    .stat_cfg_updated_tog           ( stat_cfg_updated_tog          ),
    .stat_toggle_1_sec              ( stat_toggle_1_sec             ),
    .stat_timestamp_1_sec           ( stat_timestamp_1_sec          ),
    .stat_gen_seed_err_latch        ( stat_gen_seed_err_latch       ),
    .stat_term_seed_err_latch       ( stat_term_seed_err_latch      ),
    .stat_term_err_latch            ( stat_term_err_latch           ),
    .stat_term_err_cnt              ( stat_term_err_cnt             ),
    .stat_axi_addr_ptr              ( axi_addr_ptr                  ),

    .stat_timestamp                 ( stat_timestamp                ),

    .stat_wr_burst_time_total       ( stat_wr_burst_time_total      ),
    .stat_wr_burst_time_inst        ( stat_wr_burst_time_inst       ),
    .stat_wr_burst_time_min         ( stat_wr_burst_time_min        ),
    .stat_wr_burst_time_max         ( stat_wr_burst_time_max        ),

    .stat_rd_burst_time_total       ( stat_rd_burst_time_total      ),
    .stat_rd_burst_time_inst        ( stat_rd_burst_time_inst       ),
    .stat_rd_burst_time_min         ( stat_rd_burst_time_min        ),
    .stat_rd_burst_time_max         ( stat_rd_burst_time_max        ),

    .stat_wr_burst_latency_total    ( stat_wr_burst_latency_total   ),
    .stat_wr_burst_latency_inst     ( stat_wr_burst_latency_inst    ),
    .stat_wr_burst_latency_min      ( stat_wr_burst_latency_min     ),
    .stat_wr_burst_latency_max      ( stat_wr_burst_latency_max     ),

    .stat_rd_burst_latency_total    ( stat_rd_burst_latency_total   ),
    .stat_rd_burst_latency_inst     ( stat_rd_burst_latency_inst    ),
    .stat_rd_burst_latency_min      ( stat_rd_burst_latency_min     ),
    .stat_rd_burst_latency_max      ( stat_rd_burst_latency_max     )
);

///////////////////////////////////////////////////////////////////////
// PBRS31 Generation and Checking with error insertion
///////////////////////////////////////////////////////////////////////
(* keep_hierarchy = "yes" *) PRBS31_Gen_Term #(
    .C_USE_AXI_ID           ( C_USE_AXI_ID              ),
    .C_ID_WIDTH             ( C_M_AXI_THREAD_ID_WIDTH   ),
    .C_DATA_WIDTH           ( C_M_AXI_DATA_WIDTH        ),
    .C_STAT_ERR_CNT_SIZE    ( C_STAT_ERR_CNT_SIZE       )
) PRBS31_Gen_Term_0 (
    .clk                            ( ap_clk                         ),
    .rst                            ( cfg_reset_d[0]                 ),

    .Gen_ID_In                      ( Gen_ID                         ),
    .Gen_Valid_In                   ( Gen_Valid                      ),
    .Gen_Last_In                    ( Gen_Last                       ),
    .Gen_Ready_Out                  ( Gen_Ready                      ),

    .m_axi_wready                   ( m_axi_wready                   ),
    .axi_wid                        ( axi_wid                        ),
    .m_axi_wvalid                   ( m_axi_wvalid                   ),
    .m_axi_wdata                    ( m_axi_wdata                    ),
    .m_axi_wlast                    ( m_axi_wlast                    ),

    .Term_Data_In                   ( m_axi_rdata                    ),
    .Term_ID_In                     ( m_axi_rid_int                  ),
    .Term_Valid_In                  ( m_axi_rvalid                   ),

    .ctrl_user_pattern_en           ( ctrl_user_pattern_en          ),
    .ctrl_user_pattern_a            ( ctrl_user_pattern_a           ),
    .ctrl_user_pattern_b            ( ctrl_user_pattern_b           ),

    .ctrl_clear_err_tog             ( ctrl_clear_err_tog            ),
    .suppress_error                 ( suppress_error                ),

    .stat_gen_seed_err_latch        ( stat_gen_seed_err_latch        ),
    .stat_term_seed_err_latch       ( stat_term_seed_err_latch       ),
    .stat_term_err_latch            ( stat_term_err_latch            ),
    .stat_term_err_cnt              ( stat_term_err_cnt              )
);

///////////////////////////////////////////////////////////////////////
// Test/AXI Controller
///////////////////////////////////////////////////////////////////////

(* keep_hierarchy = "yes" *) AXIS_Controller #(
    .C_MEM_KRNL_CORE_IDX        ( C_MEM_KRNL_CORE_IDX       ),
    .C_XFER_SIZE_WIDTH          ( C_XFER_SIZE_WIDTH         ),
    .C_NUM_XFER_SIZE_WIDTH      ( C_NUM_XFER_SIZE_WIDTH     ),
    .C_M_AXI_THREAD_ID_WIDTH    ( C_M_AXI_THREAD_ID_WIDTH   ),
    .C_M_AXI_ADDR_WIDTH         ( C_M_AXI_ADDR_WIDTH        ),
    .C_M_AXI_DATA_WIDTH         ( C_M_AXI_DATA_WIDTH        )
) Test_Controller (
    .clk                            ( ap_clk                        ),
    .rst                            ( cfg_reset_d[1]                ),
    .axi_addr_ptr                   ( axi_addr_ptr                  ),

    .Gen_ID_Out                     ( Gen_ID                        ),
    .Gen_Valid_Out                  ( Gen_Valid                     ),
    .Gen_Last_Out                   ( Gen_Last                      ),
    .Gen_Ready_In                   ( Gen_Ready                     ),

    .suppress_error_o               ( suppress_error                ),

    .axi_awid_o                     ( m_axi_awid_int                ),
    .axi_awvalid_o                  ( m_axi_awvalid                 ),
    .axi_awready_i                  ( m_axi_awready                 ),
    .axi_awaddr_o                   ( m_axi_awaddr                  ),
    .axi_awlen_o                    ( m_axi_awlen                   ),
    .axi_bvalid_i                   ( m_axi_bvalid                  ),

    .axi_arid_o                     ( m_axi_arid_int                ),
    .axi_arvalid_o                  ( m_axi_arvalid                 ),
    .axi_arready_i                  ( m_axi_arready                 ),
    .axi_araddr_o                   ( m_axi_araddr                  ),
    .axi_arlen_o                    ( m_axi_arlen                   ),
    .axi_rid_i                      ( m_axi_rid_int                 ),
    .axi_rvalid_i                   ( m_axi_rvalid                  ),
    .axi_rlast_i                    ( m_axi_rlast                   ),

    .ctrl_stop_tog                  ( ctrl_stop_tog                 ),
    .ctrl_update_cfg_tog            ( ctrl_update_cfg_tog           ),
    .ctrl_test_mode                 ( ctrl_test_mode                ),

    .ctrl_wr_start_addr             ( ctrl_wr_start_addr            ),
    .ctrl_wr_burst_size             ( ctrl_wr_burst_size            ),
    .ctrl_wr_num_xfer               ( ctrl_wr_num_xfer              ),

    .ctrl_rd_start_addr             ( ctrl_rd_start_addr            ),
    .ctrl_rd_burst_size             ( ctrl_rd_burst_size            ),
    .ctrl_rd_num_xfer               ( ctrl_rd_num_xfer              ),
    .ctrl_wr_burst_req_rate_en      ( ctrl_wr_burst_req_rate_en     ),
    .ctrl_wr_burst_req_rate         ( ctrl_wr_burst_req_rate        ),
    .ctrl_rd_burst_req_rate_en      ( ctrl_rd_burst_req_rate_en     ),
    .ctrl_rd_burst_req_rate         ( ctrl_rd_burst_req_rate        ),
    .ctrl_wr_outstanding_en         ( ctrl_wr_outstanding_en        ),
    .ctrl_wr_outstanding            ( ctrl_wr_outstanding           ),
    .ctrl_rd_outstanding_en         ( ctrl_rd_outstanding_en        ),
    .ctrl_rd_outstanding            ( ctrl_rd_outstanding           ),

    .ctrl_axi_id_en                 ( ctrl_axi_id_en                ),

    .stat_cfg_updated_tog           ( stat_cfg_updated_tog          )
);

///////////////////////////////////////////////////////////////////////
// Generate wfirst for burst time measurement and synchronize AXI signals for burst time and latency measurements
///////////////////////////////////////////////////////////////////////
logic [C_M_AXI_THREAD_ID_WIDTH-1:0] m_axi_awid_d        = '0;
logic                               m_axi_awxfer_d      = '0;
logic [C_M_AXI_THREAD_ID_WIDTH-1:0] axi_wid_d           = '0;
logic                               m_axi_first_wxfer_d = '0;
logic                               m_axi_last_wxfer_d  = '0;
logic [C_NUM_ID-1:0]                m_axi_wlast_latch_d = '{C_NUM_ID{1'b1}};
logic [C_M_AXI_THREAD_ID_WIDTH-1:0] m_axi_bid_d        = '0;
logic                               m_axi_bxfer_d      = '0;

always_ff @(posedge ap_clk) begin

    m_axi_awid_d        <= m_axi_awid_int;
    m_axi_awxfer_d      <= m_axi_awready & m_axi_awvalid;
    axi_wid_d           <= axi_wid_int;
    m_axi_last_wxfer_d  <= m_axi_wready  & m_axi_wvalid & m_axi_wlast;

    m_axi_bid_d         <= m_axi_bid_int;
    m_axi_bxfer_d       <= m_axi_bready & m_axi_bvalid;

    if (m_axi_wvalid & m_axi_wready & m_axi_wlast) begin
        m_axi_wlast_latch_d[axi_wid_int] <= 1'b1;
    end

    m_axi_first_wxfer_d <= 1'b0;
    if (m_axi_wvalid & m_axi_wready & m_axi_wlast_latch_d[axi_wid_int]) begin
        m_axi_first_wxfer_d               <= 1'b1;
        m_axi_wlast_latch_d[axi_wid_int]  <= 1'b0;
    end

    if (cfg_reset_d[2]) begin
        m_axi_wlast_latch_d  <= '{C_NUM_ID{1'b1}};
    end
end

///////////////////////////////////////////////////////////////////////
// Generate rfirst for burst time measurement and synchronize AXI signals for burst time and latency measurements
///////////////////////////////////////////////////////////////////////
logic [C_M_AXI_THREAD_ID_WIDTH-1:0] m_axi_arid_d        = '0;
logic                               m_axi_arxfer_d      = '0;
logic [C_M_AXI_THREAD_ID_WIDTH-1:0] m_axi_rid_d         = '0;
logic                               m_axi_first_rxfer_d = '0;
logic                               m_axi_last_rxfer_d  = '0;
logic [C_NUM_ID-1:0]                m_axi_rlast_latch_d = '{C_NUM_ID{1'b1}};

always_ff @(posedge ap_clk) begin

    m_axi_arid_d        <= m_axi_arid_int;
    m_axi_arxfer_d      <= m_axi_arready & m_axi_arvalid;
    m_axi_rid_d         <= m_axi_rid_int;
    m_axi_last_rxfer_d  <= m_axi_rready  & m_axi_rvalid & m_axi_rlast;

    if (m_axi_rvalid & m_axi_rready & m_axi_rlast) begin
        m_axi_rlast_latch_d[m_axi_rid_int] <= 1'b1;
    end

    m_axi_first_rxfer_d <= 1'b0;
    if (m_axi_rvalid & m_axi_rready & m_axi_rlast_latch_d[m_axi_rid_int]) begin
        m_axi_first_rxfer_d                 <= 1'b1;
        m_axi_rlast_latch_d[m_axi_rid_int]  <= 1'b0;
    end

    if (cfg_reset_d[2]) begin
        m_axi_rlast_latch_d  <= '{C_NUM_ID{1'b1}};
    end
end

///////////////////////////////////////////////////////////////////////
// Burst time Measurements
///////////////////////////////////////////////////////////////////////

generate
    if (C_DISABLE_BURST_TIME_MEAS == 1) begin

        // jsut keep the inst value which represent the quantity of measurement
        assign stat_wr_burst_time_total = 'h0;
        assign stat_wr_burst_time_inst  = stat_wr_burst_latency_inst;
        assign stat_wr_burst_time_min   = 'h0;
        assign stat_wr_burst_time_max   = 'h0;
        assign stat_rd_burst_time_total = 'h0;
        assign stat_rd_burst_time_inst  = stat_rd_burst_latency_inst;
        assign stat_rd_burst_time_min   = 'h0;
        assign stat_rd_burst_time_max   = 'h0;

    end else begin

        memtest_burst_time #(
            .C_ID_WIDTH         ( C_M_AXI_THREAD_ID_WIDTH   ),
            .C_STAT_MEAS_SIZE   ( C_STAT_MEAS_SIZE          ),
            .C_STAT_TOTAL_SIZE  ( C_STAT_TOTAL_SIZE         ),
            .C_STAT_INST_SIZE   ( C_STAT_INST_SIZE          )
        ) u_burst_time_wr (
            .clk                ( ap_clk                    ),
            .rst                ( cfg_reset_d[2]            ),
            .toggle_1sec        ( stat_toggle_1_sec         ),
            .timestamp          ( stat_timestamp            ),
            .start              ( m_axi_first_wxfer_d       ),
            .stop               ( m_axi_last_wxfer_d        ),
            .id                 ( '0                        ), // write bursts are not interleaved
            .stat_time_total    ( stat_wr_burst_time_total  ),
            .stat_time_inst     ( stat_wr_burst_time_inst   ),
            .stat_time_min      ( stat_wr_burst_time_min    ),
            .stat_time_max      ( stat_wr_burst_time_max    )
        );

        memtest_burst_time #(
            .C_ID_WIDTH         ( C_M_AXI_THREAD_ID_WIDTH   ),
            .C_STAT_MEAS_SIZE   ( C_STAT_MEAS_SIZE          ),
            .C_STAT_TOTAL_SIZE  ( C_STAT_TOTAL_SIZE         ),
            .C_STAT_INST_SIZE   ( C_STAT_INST_SIZE          )
        ) u_burst_time_rd (
            .clk                ( ap_clk                    ),
            .rst                ( cfg_reset_d[2]            ),
            .toggle_1sec        ( stat_toggle_1_sec         ),
            .timestamp          ( stat_timestamp            ),
            .start              ( m_axi_first_rxfer_d       ),
            .stop               ( m_axi_last_rxfer_d        ),
            .id                 ( m_axi_rid_d               ),
            .stat_time_total    ( stat_rd_burst_time_total  ),
            .stat_time_inst     ( stat_rd_burst_time_inst   ),
            .stat_time_min      ( stat_rd_burst_time_min    ),
            .stat_time_max      ( stat_rd_burst_time_max    )
        );

    end

endgenerate

///////////////////////////////////////////////////////////////////////
// Burst latency Measurements
///////////////////////////////////////////////////////////////////////

memtest_burst_latency #(
    .C_USE_AXI_ID       ( C_USE_AXI_ID              ),
    .C_ID_WIDTH         ( C_M_AXI_THREAD_ID_WIDTH   ),
    .C_STAT_MEAS_SIZE   ( C_STAT_MEAS_SIZE          ),
    .C_STAT_TOTAL_SIZE  ( C_STAT_TOTAL_SIZE         ),
    .C_STAT_INST_SIZE   ( C_STAT_INST_SIZE          )
) u_burst_latency_wr (
    .clk                ( ap_clk                        ),
    .rst                ( cfg_reset_d[2]                ),
    .toggle_1sec        ( stat_toggle_1_sec             ),
    .timestamp          ( stat_timestamp                ),
    .start              ( m_axi_last_wxfer_d            ),
    .stop               ( m_axi_bxfer_d                 ),
    .id_start           ( axi_wid_d                     ),
    .id_stop            ( m_axi_bid_d                   ),
    .stat_latency_total ( stat_wr_burst_latency_total   ),
    .stat_latency_inst  ( stat_wr_burst_latency_inst    ),
    .stat_latency_min   ( stat_wr_burst_latency_min     ),
    .stat_latency_max   ( stat_wr_burst_latency_max     )
);

memtest_burst_latency #(
    .C_USE_AXI_ID       ( C_USE_AXI_ID              ),
    .C_ID_WIDTH         ( C_M_AXI_THREAD_ID_WIDTH   ),
    .C_STAT_MEAS_SIZE   ( C_STAT_MEAS_SIZE          ),
    .C_STAT_TOTAL_SIZE  ( C_STAT_TOTAL_SIZE         ),
    .C_STAT_INST_SIZE   ( C_STAT_INST_SIZE          )
) u_burst_latency_rd (
    .clk                ( ap_clk                        ),
    .rst                ( cfg_reset_d[2]                ),
    .toggle_1sec        ( stat_toggle_1_sec             ),
    .timestamp          ( stat_timestamp                ),
    .start              ( m_axi_arxfer_d                ),
    .stop               ( m_axi_first_rxfer_d           ),
    .id_start           ( m_axi_arid_d                  ),
    .id_stop            ( m_axi_rid_d                   ),
    .stat_latency_total ( stat_rd_burst_latency_total   ),
    .stat_latency_inst  ( stat_rd_burst_latency_inst    ),
    .stat_latency_min   ( stat_rd_burst_latency_min     ),
    .stat_latency_max   ( stat_rd_burst_latency_max     )
);

endmodule: memtest_top
`default_nettype wire
