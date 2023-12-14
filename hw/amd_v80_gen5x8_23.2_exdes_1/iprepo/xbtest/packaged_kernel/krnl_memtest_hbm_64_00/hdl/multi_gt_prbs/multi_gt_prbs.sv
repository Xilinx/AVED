
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


`define MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(INDEX) \
    parameter integer C_QUAD_EN_``INDEX``       = 0, \
    parameter integer C_GT_IP_SEL_``INDEX``     = 0, \
    parameter integer C_GT_RATE_``INDEX``       = 0, \
    parameter integer C_GT_REFCLK_SEL_``INDEX`` = ``INDEX``,

`define MULTI_GT_PRBS_WRAPPER_QSFP_PORT(INDEX) \
    input  wire             QSFP_CK_N_``INDEX``, \
    input  wire             QSFP_CK_P_``INDEX``, \
    input  wire [3 : 0]     QSFP_RX_N_``INDEX``, \
    input  wire [3 : 0]     QSFP_RX_P_``INDEX``, \
    output wire [3 : 0]     QSFP_TX_N_``INDEX``, \
    output wire [3 : 0]     QSFP_TX_P_``INDEX``, \
    output wire [0 : 0]     gt_recov_clk_n_``INDEX``, \
    output wire [0 : 0]     gt_recov_clk_p_``INDEX``, \
    output wire             gt_recov_clk_``INDEX``,

module multi_gt_prbs_wrapper #(
    parameter integer C_MAJOR_VERSION       = 1,    // Major version
    parameter integer C_MINOR_VERSION       = 2,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer C_KRNL_MODE           = 0,

    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(00)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(01)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(02)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(03)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(04)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(05)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(06)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(07)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(08)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(09)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(10)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(11)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(12)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(13)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(14)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(15)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(16)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(17)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(18)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(19)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(20)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(21)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(22)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(23)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(24)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(25)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(26)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(27)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(28)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(29)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(30)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PARAM(31)

    parameter integer C_PLRAM_ADDR_WIDTH    = 64,
    parameter integer C_PLRAM_DATA_WIDTH    = 32,
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_NUM_PIPELINE_PROC   = 2,
    parameter integer DISABLE_SIM_ASSERT    = 0
) (
    // System Signals
    input  wire         ap_clk,
    input  wire         ap_clk_cont,
    input  wire         ap_rst,
    input  wire         ap_clk_2,
    input  wire         ap_clk_2_cont,
    input  wire         ap_rst_2,

    input  wire         ap_clk_div2_in,

    input  wire         watchdog_alarm_in,

    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(00)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(01)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(02)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(03)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(04)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(05)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(06)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(07)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(08)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(09)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(10)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(11)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(12)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(13)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(14)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(15)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(16)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(17)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(18)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(19)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(20)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(21)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(22)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(23)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(24)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(25)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(26)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(27)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(28)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(29)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(30)
    `MULTI_GT_PRBS_WRAPPER_QSFP_PORT(31)

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
    timeunit 1ps;
    timeprecision 1ps;

    localparam integer MAX_GT_SUPPORTED      = 32;
    localparam integer C_GT_NUM_LANE         = 4;
    localparam integer SIM_DIVIDER
    // synthesis translate_off
                                    = 1000000; // 1us sec toggle in simulation
    localparam integer DUMMY_DIVIDER
    // synthesis translate_on
                                    = 1;

    // due to tool routing issue, 1 GTF top requires that its drp clock is created locally
    localparam GENERATE_DRP_CLK =  (C_KRNL_MODE == 7)? 32'b00000000000000001000000000000000 : 32'b0;
    localparam integer AP_CLK_2_DRP_CLK = 0;

    // Proc pipeling: design to suport up to 64 GT, but address decoding only support 32
    localparam integer C_NUM_MAX_GT = 64;

    wire toggle_1_sec;

    wire drp_clk;
    wire drp_clk_sel;
    assign drp_clk_sel = (AP_CLK_2_DRP_CLK == 1) ? ap_clk_2_cont : ap_clk_cont;

    BUFGCE_DIV #(
        .SIM_DEVICE(xbtest_pkg::SIM_DEVICE_BUFGCE_DIV),
        .BUFGCE_DIVIDE(2)
    ) ap_clk_div2_buf (
        .I      (drp_clk_sel),
        .CE     (1'b1       ),
        .CLR    (1'b0       ),
        .O      (drp_clk    )
    );

    wire [C_NUM_MAX_GT-1:0] gt_refclk;

    wire         cs;
    wire         we;
    wire  [27:0] addr;
    wire  [31:0] wdata;
    logic [31:0] rdata;
    logic        cmd_cmplt;

    typedef struct {
        logic                           cs;
        logic                           we;
        logic                           we_bi;
        logic                           we_ctrl_status;
        logic [C_NUM_MAX_GT-1:0]        cs_test;
        logic [27:0]                    addr;
        logic [31:0]                    rdata[C_NUM_MAX_GT/6 : 0];
        logic [C_NUM_MAX_GT/6 : 0]      cmd_cmplt;
        logic [31:0]                    wdata;
    } Proc_Pipe_Type;

    Proc_Pipe_Type C_RST_PROC_PIPE = '{
        cs              : '0,
        we              : '0,
        we_bi           : '0,
        we_ctrl_status  : '0,
        cs_test         : '0,
        addr            : '0,
        rdata           : '{default:'0},
        cmd_cmplt       : '0,
        wdata           : '0
    };
    localparam integer PROC_PIPE_SIZE = 3;
    Proc_Pipe_Type proc_pipe [0:PROC_PIPE_SIZE-1] = '{default:C_RST_PROC_PIPE};

    multi_gt_prbs_config  #(
        .C_PLRAM_ADDR_WIDTH     ( C_PLRAM_ADDR_WIDTH    ),
        .C_PLRAM_DATA_WIDTH     ( C_PLRAM_DATA_WIDTH    ),
        .C_NUM_CU_REGISTER      ( 13 ),
        .C_NUM_CU_REG_OFFSET    ( 28 )
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

    ///////////////////////////////////////////////////////////////////////////////
    // Manage proc interface
    ///////////////////////////////////////////////////////////////////////////////

    logic        cs_d = 'b0;
    logic        we_d = 'b0;
    logic [27:0] addr_d = 'b0;
    logic [31:0] wdata_d = 'b0;

    wire        sel_wrapper_bi_ctrl = addr[4];
    wire        sel_wrapper_cores   = addr[27];
    wire [4:0]  sel_core_idx        = addr[26:22];

    wire        sel_wrapper_bi_ctrl_d   = proc_pipe[0].addr[4];
    wire        sel_wrapper_cores_d     = proc_pipe[0].addr[27];
    wire [4:0]  sel_core_idx_d          = proc_pipe[0].addr[26:22];

    logic       we_bi = 'b0;
    logic       we_ctrl_status = 'b0;
    wire [31:0] rdata_bi;
    wire [31:0] rdata_ctrl_status;

    wire  [31:0]                rdata_test [C_NUM_MAX_GT-1:0];
    wire  [C_NUM_MAX_GT-1:0]    cmd_cmplt_test;
    logic                       cmd_cmplt_OR_reduce;
    logic  [31:0]               rdata_OR_reduce;

    wire [MAX_GT_SUPPORTED-1:0] sel;
    wire [MAX_GT_SUPPORTED-1:0] sel_d;

    logic [31:0] rdata_v = 'b0;
    logic        cmd_cmplt_v = 'b0;

    generate
        for (genvar INDEX = 0; INDEX < MAX_GT_SUPPORTED; INDEX++) begin : gt_sel
            assign sel[INDEX]     = (sel_core_idx     == INDEX) ? 1'b1 : 1'b0;
            assign sel_d[INDEX]   = (sel_core_idx_d   == INDEX) ? 1'b1 : 1'b0;
        end
    endgenerate


    always_ff @(posedge ap_clk) begin
        // drive the pipeline
        proc_pipe[1:PROC_PIPE_SIZE-1] <= proc_pipe[0:PROC_PIPE_SIZE-2];

        ///////////////////////////////////////////////////////////////////////////////
        // Stage 0
        ///////////////////////////////////////////////////////////////////////////////
        proc_pipe[0].cs    <= cs;
        proc_pipe[0].we    <= we;
        proc_pipe[0].addr  <= addr;
        proc_pipe[0].wdata <= wdata;

        proc_pipe[0].we_bi           <= 1'b0;
        proc_pipe[0].we_ctrl_status  <= 1'b0;

        proc_pipe[0].cs_test  <= 'b0;

        if (cs) begin
            case (sel_wrapper_cores)
                'h0 : begin // wrapper build info
                    if (sel_wrapper_bi_ctrl) begin
                        proc_pipe[0].we_ctrl_status <= we;
                    end else begin
                        proc_pipe[0].we_bi          <= we;
                    end
                end
                default : begin // cores registers
                    for (int INDEX = 0; INDEX < MAX_GT_SUPPORTED; INDEX++) begin
                        if (sel[INDEX] == 1'b1) begin
                            proc_pipe[0].cs_test[INDEX] <= 1'b1;
                        end
                    end
                end
            endcase
        end

        // OR reduce per group of 6; from max 64 to 11
        rdata_OR_reduce = '0;
        cmd_cmplt_OR_reduce = '0;
        for (int i = MAX_GT_SUPPORTED-1; i >= 0; i--) begin
            rdata_OR_reduce = rdata_OR_reduce | rdata_test[i];
            cmd_cmplt_OR_reduce = cmd_cmplt_OR_reduce | cmd_cmplt_test[i];
            if ((i % 6) == 0) begin
                proc_pipe[0].rdata[i/6] <= rdata_OR_reduce;
                proc_pipe[0].cmd_cmplt[i/6] <= cmd_cmplt_OR_reduce;
                rdata_OR_reduce = '0;
                cmd_cmplt_OR_reduce = '0;
            end
        end


        ///////////////////////////////////////////////////////////////////////////////
        // Stage 1: OR reduction from max 11 to  2
        ///////////////////////////////////////////////////////////////////////////////

        rdata_OR_reduce = '0;
        cmd_cmplt_OR_reduce = '0;
        for (int i = MAX_GT_SUPPORTED/6; i >= 0; i--) begin
            rdata_OR_reduce = rdata_OR_reduce | proc_pipe[0].rdata[i];
            cmd_cmplt_OR_reduce = cmd_cmplt_OR_reduce | proc_pipe[0].cmd_cmplt[i];
            if ((i % 6) == 0) begin
                proc_pipe[1].rdata[i/6] <= rdata_OR_reduce;
                proc_pipe[1].cmd_cmplt[i/6] <= cmd_cmplt_OR_reduce;
                rdata_OR_reduce = '0;
                cmd_cmplt_OR_reduce = '0;
            end
        end


        ///////////////////////////////////////////////////////////////////////////////
        // Stage 2: combine everytying
        ///////////////////////////////////////////////////////////////////////////////

        rdata_v     = proc_pipe[1].rdata[0] | proc_pipe[1].rdata[1];
        cmd_cmplt_v = proc_pipe[1].cmd_cmplt[0] | proc_pipe[1].cmd_cmplt[1];

        rdata       <= rdata_v;
        cmd_cmplt   <= cmd_cmplt_v;

        // wrapper level access
        if ((proc_pipe[0].cs == 1'b1) && (sel_wrapper_cores_d == 1'b0)) begin
            if (sel_wrapper_bi_ctrl_d) begin
                rdata   <= rdata_ctrl_status;
            end else begin
                rdata   <= rdata_bi;
            end

            cmd_cmplt   <= 1'b1;
        end
    end



    ///////////////////////////////////////////////////////////////////////////////
    // build info instantiation
    ///////////////////////////////////////////////////////////////////////////////
    build_info_v4_0 #(
        .C_MAJOR_VERSION    ( C_MAJOR_VERSION ),
        .C_MINOR_VERSION    ( C_MINOR_VERSION ),
        .C_BUILD_VERSION    ( C_BUILD_VERSION ),
        .C_BLOCK_ID         ( C_KRNL_MODE     )
    ) wrapper_build_info (
        .Clk        ( ap_clk                                        ),
        .Rst        ( ap_rst                                        ),
        .Info_1     ( 16'b0                                         ),
        .Info_2     ( 16'b0                                         ),
        .Info_3     ( 16'h0                                         ),
        .Info_4     ( {MAX_GT_SUPPORTED[7:0], C_GT_NUM_LANE[7:0]}   ),
        .Info_5     ( 16'h0                                         ),
        .Info_6     ( 16'h0                                         ),
        .Info_7     ( C_CLOCK0_FREQ[15:0]                           ),
        .Info_8     ( C_CLOCK1_FREQ[15:0]                           ),
        .We         ( proc_pipe[0].we_bi                            ),
        .Addr       ( proc_pipe[0].addr[2:0]                        ),
        .Data_In    ( proc_pipe[0].wdata                            ),
        .Data_Out   ( rdata_bi                                      )
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

        .We             ( proc_pipe[0].we_ctrl_status   ),
        .Addr           ( proc_pipe[0].addr[2:0]        ),
        .Data_In        ( proc_pipe[0].wdata            ),
        .User_Status_1  ( 'h0               ),
        .Data_Out       ( rdata_ctrl_status )
    );

    ///////////////////////////////////////////////////////////////////////////////
    // Instantiate multiple of GT tops IPs
    ///////////////////////////////////////////////////////////////////////////////

    timer #(
        .C_CLOCK_FREQ       ( C_CLOCK0_FREQ     ),
        .C_1_SEC_TIMER_EN   ( 1                 ),
        .DEST_SYNC_FF       ( DEST_SYNC_FF      ),
        .C_SIM_DIVIDER      ( SIM_DIVIDER       )
    ) u_timer (
        .clk_1              ( ap_clk_cont       ),
        .clk_2              ( ap_clk            ),

        .toggle_1_sec_1     (                   ),
        .toggle_1_sec_2     ( toggle_1_sec      ),

        .timer_rst          ( 1'b0              ),
        .timer_end_1        (                   ),
        .timer_end_2        (                   )
    );

    `define MULTI_GT_PRBS_TOP_INSTANCE(INDEX) \
        generate \
            if (C_QUAD_EN_``INDEX`` == 1) begin : top_``INDEX`` \
           \
            parameter integer PIPE_IF_WIDTH_``INDEX`` = 1 + 1 + 1 + 1 + 22 + 32 + 32 + 1 + 1; \
            \
            wire                                    watchdog_alarm_d_``INDEX``; \
            wire                                    ap_rst_d_``INDEX``;         \
            wire                                    cs_d_``INDEX``;             \
            wire                                    we_d_``INDEX``;             \
            wire [21:0]                             addr_d_``INDEX``;           \
            wire [31:0]                             wdata_d_``INDEX``;          \
            wire [31:0]                             rdata_``INDEX``;            \
            wire                                    cmd_cmplt_``INDEX``;        \
            wire                                    toggle_1_sec_d_``INDEX``;   \
            \
            wire [PIPE_IF_WIDTH_``INDEX``-1:0] pipeline_in_``INDEX``, pipeline_out_``INDEX``; \
            \
            assign pipeline_in_``INDEX`` = {  watchdog_alarm_in,  ap_rst, proc_pipe[0].cs_test[INDEX], proc_pipe[0].we,  \
                                    proc_pipe[0].addr[21:0],                                                   \
                                    proc_pipe[0].wdata,                                                        \
                                    rdata_``INDEX``,                                                \
                                    cmd_cmplt_``INDEX``,                                            \
                                    toggle_1_sec                                              };     \
            \
            (* keep_hierarchy = "yes" *) pipeline #(                                                 \
                .C_NUM_PIPELINE ( C_NUM_PIPELINE_PROC ),                \
                .C_DATA_WIDTH   ( PIPE_IF_WIDTH_``INDEX``)              \
            ) pipeline_if_``INDEX`` (                                   \
                .clk        ( ap_clk        ),                          \
                .rst        ( 1'b0          ),                          \
                .data_in    ( pipeline_in_``INDEX``   ),                \
                .data_out   ( pipeline_out_``INDEX``  )                 \
            );                                                          \
            \
            assign {    watchdog_alarm_d_``INDEX``, ap_rst_d_``INDEX``, cs_d_``INDEX``, we_d_``INDEX``, \
                        addr_d_``INDEX``,                                                               \
                        wdata_d_``INDEX``,                                                              \
                        rdata_test[INDEX],                                                              \
                        cmd_cmplt_test[INDEX],                                                          \
                        toggle_1_sec_d_``INDEX``             } = pipeline_out_``INDEX``;  \
            \
                if (C_KRNL_MODE == 9) begin : gtyp \
                    (* keep_hierarchy = "yes" *)  gtyp_prbs_top # ( \
                        .C_BUILD_VERSION    ( C_BUILD_VERSION           ), \
                        .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ             ), \
                        .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ             ), \
                        .C_BLOCK_ID         ( C_KRNL_MODE               ), \
                        .C_GT_INDEX         ( ``INDEX``                 ), \
                        .C_GT_NUM_LANE      ( C_GT_NUM_LANE             ), \
                        .DEST_SYNC_FF       ( DEST_SYNC_FF              ), \
                        .C_QUAD_EN          ( C_QUAD_EN_``INDEX``       ), \
                        .C_GT_IP_SEL        ( 1                         ), \
                        .C_GT_RATE          ( 4                         ), \
                        .C_GEN_DRP_CLK      ( GENERATE_DRP_CLK[``INDEX``]), \
                        .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT        ) \
                    ) u_prbs_top ( \
                        .ap_clk             ( ap_clk                    ), \
                        .ap_clk_cont        ( ap_clk_cont               ), \
                        .ap_rst             ( ap_rst_d_``INDEX``        ), \
                        .drp_clk_in         ( drp_clk                   ), \
                        .drp_clk_gen        ( drp_clk_sel               ), \
                        .QSFP_CK_N          ( QSFP_CK_N_``INDEX``       ), \
                        .QSFP_CK_P          ( QSFP_CK_P_``INDEX``       ), \
                        .QSFP_RX_N          ( QSFP_RX_N_``INDEX``       ), \
                        .QSFP_RX_P          ( QSFP_RX_P_``INDEX``       ), \
                        .QSFP_TX_N          ( QSFP_TX_N_``INDEX``       ), \
                        .QSFP_TX_P          ( QSFP_TX_P_``INDEX``       ), \
                                                                           \
                        .gt_refclk_in       ( gt_refclk[C_GT_REFCLK_SEL_``INDEX``] ), \
                        .gt_refclk_out      ( gt_refclk[``INDEX``]      ), \
                                                                           \
                        .toggle_1_sec       ( toggle_1_sec_d_``INDEX``  ), \
                        .watchdog_alarm     ( watchdog_alarm_d_``INDEX``), \
                                                                           \
                        .cs                 ( cs_d_``INDEX``            ), \
                        .we                 ( we_d_``INDEX``            ), \
                        .addr               ( addr_d_``INDEX``          ), \
                        .wdata              ( wdata_d_``INDEX``         ), \
                        .rdata              ( rdata_``INDEX``           ), \
                        .cmd_cmplt          ( cmd_cmplt_``INDEX``       )  \
                    ); \
                end else if (C_KRNL_MODE == 8) begin : gtm \
                    (* keep_hierarchy = "yes" *)  gtm_prbs_top # ( \
                        .C_BUILD_VERSION    ( C_BUILD_VERSION           ), \
                        .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ             ), \
                        .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ             ), \
                        .C_BLOCK_ID         ( C_KRNL_MODE               ), \
                        .C_GT_INDEX         ( ``INDEX``                 ), \
                        .C_GT_NUM_LANE      ( C_GT_NUM_LANE             ), \
                        .DEST_SYNC_FF       ( DEST_SYNC_FF              ), \
                        .C_QUAD_EN          ( C_QUAD_EN_``INDEX``       ), \
                        .C_GT_IP_SEL        ( 1                         ), \
                        .C_GT_RATE          ( 3                         ), \
                        .C_GEN_DRP_CLK      ( GENERATE_DRP_CLK[``INDEX``]), \
                        .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT        ) \
                    ) u_prbs_top ( \
                        .ap_clk             ( ap_clk                    ), \
                        .ap_clk_cont        ( ap_clk_cont               ), \
                        .ap_rst             ( ap_rst_d_``INDEX``        ), \
                        .drp_clk_in         ( drp_clk                   ), \
                        .drp_clk_gen        ( drp_clk_sel               ), \
                        .QSFP_CK_N          ( QSFP_CK_N_``INDEX``       ), \
                        .QSFP_CK_P          ( QSFP_CK_P_``INDEX``       ), \
                        .QSFP_RX_N          ( QSFP_RX_N_``INDEX``       ), \
                        .QSFP_RX_P          ( QSFP_RX_P_``INDEX``       ), \
                        .QSFP_TX_N          ( QSFP_TX_N_``INDEX``       ), \
                        .QSFP_TX_P          ( QSFP_TX_P_``INDEX``       ), \
                                                                           \
                        .gt_refclk_in       ( gt_refclk[C_GT_REFCLK_SEL_``INDEX``] ), \
                        .gt_refclk_out      ( gt_refclk[``INDEX``]      ), \
                                                                           \
                        .toggle_1_sec       ( toggle_1_sec_d_``INDEX``  ), \
                        .watchdog_alarm     ( watchdog_alarm_d_``INDEX``), \
                                                                           \
                        .cs                 ( cs_d_``INDEX``            ), \
                        .we                 ( we_d_``INDEX``            ), \
                        .addr               ( addr_d_``INDEX``          ), \
                        .wdata              ( wdata_d_``INDEX``         ), \
                        .rdata              ( rdata_``INDEX``           ), \
                        .cmd_cmplt          ( cmd_cmplt_``INDEX``       )  \
                    ); \
                end else begin: gtf \
                    (* keep_hierarchy = "yes" *) gtf_prbs_top # ( \
                        .C_BUILD_VERSION    ( C_BUILD_VERSION           ), \
                        .C_CLOCK0_FREQ      ( C_CLOCK0_FREQ             ), \
                        .C_CLOCK1_FREQ      ( C_CLOCK1_FREQ             ), \
                        .C_BLOCK_ID         ( C_KRNL_MODE               ), \
                        .C_GT_INDEX         ( ``INDEX``                 ), \
                        .C_GT_NUM_LANE      ( C_GT_NUM_LANE             ), \
                        .DEST_SYNC_FF       ( DEST_SYNC_FF              ), \
                        .C_QUAD_EN          ( C_QUAD_EN_``INDEX``       ), \
                        .C_GT_IP_SEL        ( 2                         ), \
                        .C_GT_RATE          ( 1                         ), \
                        .C_GEN_DRP_CLK      ( GENERATE_DRP_CLK[``INDEX``]), \
                        .DISABLE_SIM_ASSERT ( DISABLE_SIM_ASSERT        ) \
                    ) u_prbs_top ( \
                        .ap_clk             ( ap_clk                    ), \
                        .ap_clk_cont        ( ap_clk_cont               ), \
                        .ap_rst             ( ap_rst_d_``INDEX``        ), \
                        .drp_clk_in         ( drp_clk                   ), \
                        .drp_clk_gen        ( drp_clk_sel               ), \
                        .QSFP_CK_N          ( QSFP_CK_N_``INDEX``       ), \
                        .QSFP_CK_P          ( QSFP_CK_P_``INDEX``       ), \
                        .QSFP_RX_N          ( QSFP_RX_N_``INDEX``       ), \
                        .QSFP_RX_P          ( QSFP_RX_P_``INDEX``       ), \
                        .QSFP_TX_N          ( QSFP_TX_N_``INDEX``       ), \
                        .QSFP_TX_P          ( QSFP_TX_P_``INDEX``       ), \
                        .gt_recov_clk_n     ( gt_recov_clk_n_``INDEX``  ), \
                        .gt_recov_clk_p     ( gt_recov_clk_p_``INDEX``  ), \
                        .gt_recov_clk       ( gt_recov_clk_``INDEX``    ), \
                                                                           \
                        .toggle_1_sec       ( toggle_1_sec_d_``INDEX``  ), \
                        .watchdog_alarm     ( watchdog_alarm_d_``INDEX``), \
                                                                           \
                        .cs                 ( cs_d_``INDEX``            ), \
                        .we                 ( we_d_``INDEX``            ), \
                        .addr               ( addr_d_``INDEX``          ), \
                        .wdata              ( wdata_d_``INDEX``         ), \
                        .rdata              ( rdata_``INDEX``           ), \
                        .cmd_cmplt          ( cmd_cmplt_``INDEX``       )  \
                    ); \
                end \
            end else begin \
                assign rdata_test[INDEX]      = '0; \
                assign cmd_cmplt_test[INDEX]  = '0; \
            end \
        endgenerate

        `MULTI_GT_PRBS_TOP_INSTANCE(00)
        `MULTI_GT_PRBS_TOP_INSTANCE(01)
        `MULTI_GT_PRBS_TOP_INSTANCE(02)
        `MULTI_GT_PRBS_TOP_INSTANCE(03)
        `MULTI_GT_PRBS_TOP_INSTANCE(04)
        `MULTI_GT_PRBS_TOP_INSTANCE(05)
        `MULTI_GT_PRBS_TOP_INSTANCE(06)
        `MULTI_GT_PRBS_TOP_INSTANCE(07)
        `MULTI_GT_PRBS_TOP_INSTANCE(08)
        `MULTI_GT_PRBS_TOP_INSTANCE(09)
        `MULTI_GT_PRBS_TOP_INSTANCE(10)
        `MULTI_GT_PRBS_TOP_INSTANCE(11)
        `MULTI_GT_PRBS_TOP_INSTANCE(12)
        `MULTI_GT_PRBS_TOP_INSTANCE(13)
        `MULTI_GT_PRBS_TOP_INSTANCE(14)
        `MULTI_GT_PRBS_TOP_INSTANCE(15)
        `MULTI_GT_PRBS_TOP_INSTANCE(16)
        `MULTI_GT_PRBS_TOP_INSTANCE(17)
        `MULTI_GT_PRBS_TOP_INSTANCE(18)
        `MULTI_GT_PRBS_TOP_INSTANCE(19)
        `MULTI_GT_PRBS_TOP_INSTANCE(20)
        `MULTI_GT_PRBS_TOP_INSTANCE(21)
        `MULTI_GT_PRBS_TOP_INSTANCE(22)
        `MULTI_GT_PRBS_TOP_INSTANCE(23)
        `MULTI_GT_PRBS_TOP_INSTANCE(24)
        `MULTI_GT_PRBS_TOP_INSTANCE(25)
        `MULTI_GT_PRBS_TOP_INSTANCE(26)
        `MULTI_GT_PRBS_TOP_INSTANCE(27)
        `MULTI_GT_PRBS_TOP_INSTANCE(28)
        `MULTI_GT_PRBS_TOP_INSTANCE(29)
        `MULTI_GT_PRBS_TOP_INSTANCE(30)
        `MULTI_GT_PRBS_TOP_INSTANCE(31)

endmodule : multi_gt_prbs_wrapper
`default_nettype wire
