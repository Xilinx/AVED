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

module memtest_reg_array #(
    parameter integer C_MAJOR_VERSION       = 0,    // Major version
    parameter integer C_MINOR_VERSION       = 0,    // Minor version
    parameter integer C_BUILD_VERSION       = 0,    // Build version
    parameter integer C_CLOCK0_FREQ         = 300,  // Frequency for clock0 (ap_clk)
    parameter integer C_CLOCK1_FREQ         = 500,  // Frequency for clock1 (ap_clk_2)
    parameter integer DEST_SYNC_FF          = 4,
    parameter integer C_BLOCK_ID            = 1,    // Block_ID (POWER = 0, MEMORY = 1)
    parameter integer C_MEM_KRNL_INST       = 0,    // Memory kernel instance
    parameter integer C_MEM_KRNL_CORE_IDX   = 0,    // M_AXI Port index
    parameter integer C_NUM_MAX_M_AXI       = 32,   // Maximum number of M_AXI ports (for memory kernel)
    parameter integer C_NUM_USED_M_AXI      = 1,    // Number of M_AXI ports
    parameter integer C_MEM_TYPE            = 0,    // 1 single-channel 2 multi-channel
    parameter integer C_USE_AXI_ID          = 0,    // 1 use axi id, 0 disable
    parameter integer C_M_AXI_DATA_WIDTH    = 0,
    parameter integer C_NUM_XFER_SIZE_WIDTH = 28,   // Width of the ctrl_xfer_cnt,
    parameter integer C_M_AXI_ADDR_WIDTH    = 64,
    parameter integer C_STAT_MEAS_SIZE      = 32,    // Max 32
    parameter integer C_STAT_TOTAL_SIZE     = 64,    // Max 64
    parameter integer C_STAT_INST_SIZE      = 32,    // Max 32
    parameter integer C_STAT_ERR_CNT_SIZE   = 8     // Max 32

) (
    input  wire                                 clk,
    input  wire                                 rst,

    input  wire                                 cs,
    input  wire                                 we,
    input  wire  [20:0]                         addr,
    input  wire  [31:0]                         wdata,
    output logic [31:0]                         rdata = '0,
    output logic                                cmd_cmplt = '0,

    input  wire                                 watchdog_alarm,

    output logic                                ctrl_stop_tog = '0,
    output logic                                ctrl_update_cfg_tog = '0,
    output logic [1:0]                          ctrl_test_mode = '0,

    output logic                                ctrl_reset = '0,
    output logic                                ctrl_clear_err_tog  = '0,

    output wire [63:0]                          ctrl_wr_start_addr,
    output wire [63:0]                          ctrl_rd_start_addr,

    output logic [8-1:0]                        ctrl_wr_burst_size = '0,
    output logic [8-1:0]                        ctrl_rd_burst_size = '0,

    output logic [C_NUM_XFER_SIZE_WIDTH-1:0]    ctrl_wr_num_xfer = '0,
    output logic [C_NUM_XFER_SIZE_WIDTH-1:0]    ctrl_rd_num_xfer = '0,

    output logic                                ctrl_wr_burst_req_rate_en = '0,
    output logic [31:0]                         ctrl_wr_burst_req_rate = '0,
    output logic                                ctrl_rd_burst_req_rate_en = '0,
    output logic [31:0]                         ctrl_rd_burst_req_rate = '0,

    output logic                                ctrl_wr_outstanding_en = '0,
    output logic [8:0]                          ctrl_wr_outstanding = '0,
    output logic                                ctrl_rd_outstanding_en = '0,
    output logic [8:0]                          ctrl_rd_outstanding = '0,

    output logic                                ctrl_axi_id_en = C_USE_AXI_ID[0],
    output logic                                ctrl_user_pattern_en = '0,
    output logic [31:0]                         ctrl_user_pattern_a = '0,
    output logic [31:0]                         ctrl_user_pattern_b = '0,

    input  wire                                 stat_cfg_updated_tog,

    input  wire                                 stat_toggle_1_sec,
    input  wire  [15:0]                         stat_timestamp_1_sec,

    input  wire                                 stat_gen_seed_err_latch,
    input  wire                                 stat_term_seed_err_latch,
    input  wire                                 stat_term_err_latch,
    input  wire [C_STAT_ERR_CNT_SIZE-1:0]       stat_term_err_cnt,

    input  wire [C_M_AXI_ADDR_WIDTH-1:0]        stat_axi_addr_ptr,

    input wire [31:0]                           stat_timestamp,

    input wire [C_STAT_TOTAL_SIZE-1:0]          stat_wr_burst_time_total,
    input wire [C_STAT_INST_SIZE-1:0]           stat_wr_burst_time_inst,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_wr_burst_time_min,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_wr_burst_time_max,

    input wire [C_STAT_TOTAL_SIZE-1:0]          stat_rd_burst_time_total,
    input wire [C_STAT_INST_SIZE-1:0]           stat_rd_burst_time_inst,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_rd_burst_time_min,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_rd_burst_time_max,

    input wire [C_STAT_TOTAL_SIZE-1:0]          stat_wr_burst_latency_total,
    input wire [C_STAT_INST_SIZE-1:0]           stat_wr_burst_latency_inst,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_wr_burst_latency_min,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_wr_burst_latency_max,

    input wire [C_STAT_TOTAL_SIZE-1:0]          stat_rd_burst_latency_total,
    input wire [C_STAT_INST_SIZE-1:0]           stat_rd_burst_latency_inst,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_rd_burst_latency_min,
    input wire [C_STAT_MEAS_SIZE-1:0]           stat_rd_burst_latency_max
);

function [3:0] f_data_width_div;
    input integer C_M_AXI_DATA_WIDTH;

    case (C_M_AXI_DATA_WIDTH)
        512 : begin
                f_data_width_div = 'h1;
            end
        256 : begin
                f_data_width_div = 'h2;
            end
        128 : begin
                f_data_width_div = 'h4;
            end
        64 : begin
                f_data_width_div = 'h8;
            end
        default: begin
                f_data_width_div = 'h0;
        end
    endcase

endfunction

wire [18:0] addr_core_reg   = addr[18:0];
wire        sel_core_reg    = addr[20]; // used to separate BI/status from all other registers
wire        sel_core_stat   = addr[19];
wire        sel_bi_ctrl     = addr[4];

wire        we_bi           = (~sel_core_reg & ~sel_bi_ctrl & cs & we) ? 'b1 : 'b0;
wire        we_ctrl_status  = (~sel_core_reg &  sel_bi_ctrl & cs & we) ? 'b1 : 'b0;
// when reading, use the address from the read pipeline
wire [2:0]  addr_bi_ctrl_status  = (cs & we)? addr[2:0]: read_pipe[2].addr[2:0];
wire [31:0] rdata_bi;
wire [31:0] rdata_ctrl_status;

wire [15: 0] Info_3 = {12'b0, f_data_width_div(C_M_AXI_DATA_WIDTH) };

// read pipeling for 32 registers (5 bits of addresses)
typedef struct {
    logic           cs;
    logic [20:0]    addr;
    logic [31:0]    rdata_ctrl[0:31];
    logic [31:0]    rdata_stat[0:31];
} Read_Pipe_Type;

Read_Pipe_Type C_RST_READ_PIPE = '{
    cs              : '0,
    addr            : '0,
    rdata_ctrl      : '{default:'0},
    rdata_stat      : '{default:'0}
};
localparam integer READ_PIPE_SIZE = 3;
Read_Pipe_Type read_pipe [0:READ_PIPE_SIZE-1] = '{default:C_RST_READ_PIPE};

// designed for 2 times 32 registers
wire [31:0]    rdata_stat[0:31];
wire [31:0]    rdata_ctrl[0:31];

//########################################
//### build info instantiation
//########################################
build_info_v4_0 #(
    .C_MAJOR_VERSION    ( C_MAJOR_VERSION       ),
    .C_MINOR_VERSION    ( C_MINOR_VERSION       ),
    .C_BUILD_VERSION    ( C_BUILD_VERSION       ),
    .C_BLOCK_ID         ( C_BLOCK_ID            )
) u_build_info (
    .Clk        ( clk                                           ),
    .Rst        ( rst                                           ),
    .Info_1     ( 16'b0                                         ), // Info 1 reserved for future use
    .Info_2     ( {C_MEM_KRNL_INST[7:0], 8'b0}                  ),
    .Info_3     ( Info_3                                        ),
    .Info_4     ( {C_NUM_MAX_M_AXI[7:0], C_NUM_USED_M_AXI[7:0]} ),
    .Info_5     ( {12'b0, C_USE_AXI_ID[1:0], C_MEM_TYPE[1:0]}   ),
    .Info_6     ( {8'b0, C_MEM_KRNL_CORE_IDX[7:0]}              ),
    .Info_7     ( C_CLOCK0_FREQ[15:0]                           ),
    .Info_8     ( C_CLOCK1_FREQ[15:0]                           ),
    .We         ( we_bi                                         ),
    .Addr       ( addr_bi_ctrl_status                           ),
    .Data_In    ( wdata                                         ),
    .Data_Out   ( rdata_bi                                      )
);


common_ctrl_status #(
    .C_CLOCK_FREQ               ( C_CLOCK0_FREQ ),
    .DEST_SYNC_FF               ( DEST_SYNC_FF  ),
    .C_CLK_TROTTLE_DETECT_EN    ( 0             ),
    .C_WATCHDOG_ENABLE          ( 0             ),
    .C_EXT_TOGGLE_1_SEC         ( 0             )
) u_common_ctrl_status (
    .ap_clk         ( clk               ),
    .ap_clk_cont    ( 1'b0              ),
    .ap_rst         ( rst               ),

    .ap_clk_2       ( 1'b0              ),
    .ap_clk_2_cont  ( 1'b0              ),
    .ap_rst_2       ( 1'b0              ),

    .toggle_1sec    ( 1'b0              ),
    .rst_watchdog   ( 1'b0              ),
    .watchdog_alarm (                   ),

    .We             ( we_ctrl_status        ),
    .Addr           ( addr_bi_ctrl_status   ),
    .Data_In        ( wdata                 ),
    .User_Status_1  ( 32'b0                 ),
    .Data_Out       ( rdata_ctrl_status     )
);

//########################################
//### Registers
//########################################

logic           stat_cfg_updated_latch = '0;
logic [31:0]    ctrl_wr_start_addr_lsb = '0;
logic [31:0]    ctrl_wr_start_addr_msb = '0;

logic [31:0]    ctrl_rd_start_addr_lsb = '0;
logic [31:0]    ctrl_rd_start_addr_msb = '0;

// there is a false path on the watchdog alarm input
wire   watchdog_alarm_cdc;
(*dont_touch ="true"*) logic watchdog_alarm_d = '0;

xpm_cdc_single #(
    .DEST_SYNC_FF   ( DEST_SYNC_FF  ),
    .INIT_SYNC_FF   ( 0             ),
    .SRC_INPUT_REG  ( 0             ),
    .SIM_ASSERT_CHK ( 0             )
)
xpm_cdc_watchdog (
    .src_clk  ( 'b0         ),
    .src_in   ( watchdog_alarm      ),
    .dest_out ( watchdog_alarm_cdc  ),
    .dest_clk ( clk                 )
);

    // 0x100000
assign  rdata_ctrl[0] [5:4]  = ctrl_test_mode;
assign  rdata_ctrl[0] [8]    = ctrl_reset;
assign  rdata_ctrl[0] [16]   = ctrl_wr_burst_req_rate_en;
assign  rdata_ctrl[0] [17]   = ctrl_rd_burst_req_rate_en;
assign  rdata_ctrl[0] [18]   = ctrl_wr_outstanding_en;
assign  rdata_ctrl[0] [19]   = ctrl_rd_outstanding_en;
assign  rdata_ctrl[0] [20]   = (C_USE_AXI_ID == 1)? ctrl_axi_id_en: 'b0;
assign  rdata_ctrl[0] [21]   = ctrl_user_pattern_en;
    // 0x100001
assign  rdata_ctrl[1] = ctrl_wr_start_addr_lsb;
    // 0x100002
assign  rdata_ctrl[2] = ctrl_wr_start_addr_msb;
    // 0x100003
assign  rdata_ctrl[3] = ctrl_rd_start_addr_lsb;
    // 0x100004
assign  rdata_ctrl[4] = ctrl_rd_start_addr_msb;
    // 0x100005
assign  rdata_ctrl[5][8-1:0] = ctrl_wr_burst_size;
    // 0x100006
assign  rdata_ctrl[6][8-1:0] = ctrl_rd_burst_size;
    // 0x100007
assign  rdata_ctrl[7][C_NUM_XFER_SIZE_WIDTH-1:0] = ctrl_wr_num_xfer;
    // 0x100008
assign  rdata_ctrl[8][C_NUM_XFER_SIZE_WIDTH-1:0] = ctrl_rd_num_xfer;
    // 0x100009
assign  rdata_ctrl[9] = ctrl_wr_burst_req_rate;
    // 0x10000A
assign  rdata_ctrl[10] = ctrl_rd_burst_req_rate;
    // 0x10000B
assign  rdata_ctrl[11][24:16] = ctrl_wr_outstanding;
assign  rdata_ctrl[11][8:0]   = ctrl_rd_outstanding;
    // 0x10000C
assign  rdata_ctrl[12] = ctrl_user_pattern_a;
    // 0x10000D
assign  rdata_ctrl[13] = ctrl_user_pattern_b;

    // 0x180000
assign  rdata_stat[0] [0]        = stat_cfg_updated_latch;
assign  rdata_stat[0] [1]        = stat_term_err_latch;
assign  rdata_stat[0] [2]        = stat_gen_seed_err_latch;
assign  rdata_stat[0] [3]        = stat_term_seed_err_latch;
assign  rdata_stat[0] [14:4]     = 'b0; // remove X' in simu
assign  rdata_stat[0] [15]       = stat_toggle_1_sec;
assign  rdata_stat[0] [31:16]    = stat_timestamp_1_sec;
    // 0x180001
assign  rdata_stat[1] [C_STAT_ERR_CNT_SIZE-1:0] = stat_term_err_cnt;
assign  rdata_stat[1] [31:C_STAT_ERR_CNT_SIZE]  = 'b0; // remove X' in simu
    // 0x180002
assign  rdata_stat[2] = stat_axi_addr_ptr[31:0];
    // 0x180003
assign  rdata_stat[3] = stat_axi_addr_ptr[63:32];
    // 0x180004
assign  rdata_stat[4] = (C_STAT_TOTAL_SIZE >= 32)? stat_wr_burst_latency_total[31:0] : stat_wr_burst_latency_total;
    // 0x180005
assign  rdata_stat[5] [C_STAT_TOTAL_SIZE-32-1:0] = (C_STAT_TOTAL_SIZE >= 32)? stat_wr_burst_latency_total[C_STAT_TOTAL_SIZE-1:32] : 'h0 ;
     // 0x180006
assign  rdata_stat[6] [C_STAT_INST_SIZE-1:0] = stat_wr_burst_latency_inst ;
    // 0x180007
assign  rdata_stat[7] = (C_STAT_TOTAL_SIZE >= 32)? stat_rd_burst_latency_total[31:0] : stat_rd_burst_latency_total;
    // 0x180008
assign  rdata_stat[8] [C_STAT_TOTAL_SIZE-32-1:0] = (C_STAT_TOTAL_SIZE >= 32)? stat_rd_burst_latency_total[C_STAT_TOTAL_SIZE-1:32] : 'h0;
    // 0x180009
assign  rdata_stat[9] [C_STAT_INST_SIZE-1:0] = stat_rd_burst_latency_inst;
     // 0x18000A
assign  rdata_stat[10] = stat_timestamp;
    // 0x18000B
assign  rdata_stat[11] = 'h0;
    // 0x18000C
assign  rdata_stat[12] = 'h0;
    // 0x18000D
assign  rdata_stat[13] = 'h0;
    // 0x18000E
assign  rdata_stat[14] = 'h0;
    // 0x18000F
assign  rdata_stat[15] = 'h0;
    // 0x180010
assign  rdata_stat[16] = 'h0;
   // 0x180011
assign  rdata_stat[17] = 'h0;
    // 0x180012
assign  rdata_stat[18] = 'h0;
    // 0x180013
assign  rdata_stat[19] = 'h0;
    // 0x180014
assign  rdata_stat[20] = 'h0;
    // 0x180015
assign  rdata_stat[21] = 'h0;
    // 0x180016
assign  rdata_stat[22] = 'h0;
    // 0x180017
assign  rdata_stat[23] = 'h0;
    // 0x180018
assign  rdata_stat[24] = 'h0;


wire  cfg_updated_tog;
logic cfg_updated_tog_d1;
logic cfg_updated_tog_d2;

wire  stat_cfg_updated_pulse;
assign  stat_cfg_updated_pulse = cfg_updated_tog_d1 ^ cfg_updated_tog_d2;

always_ff @(posedge clk) begin
    cfg_updated_tog_d1 <= cfg_updated_tog;
    cfg_updated_tog_d2 <= cfg_updated_tog_d1;
end

// use XPM to create false path automatically
xpm_cdc_single #(
    .DEST_SYNC_FF   ( 2  ),
    .INIT_SYNC_FF   ( 0  ),
    .SRC_INPUT_REG  ( 0  ),
    .SIM_ASSERT_CHK ( 0  )
)
xpm_cfg_updated(
    .src_clk  ( 1'h0                    ),
    .src_in   ( stat_cfg_updated_tog    ),
    .dest_out ( cfg_updated_tog         ),
    .dest_clk ( clk                     )
);

always_ff @(posedge clk) begin


    watchdog_alarm_d <= watchdog_alarm_cdc;

    if (watchdog_alarm_cdc != watchdog_alarm_d) begin

        ctrl_stop_tog  <= ~ctrl_stop_tog;; // stop the memory CU channel

    end
    if (stat_cfg_updated_pulse) begin

        stat_cfg_updated_latch <= 1'b1;

    end

    // only WRITE access to the control register
    if (cs && sel_core_reg && ~sel_core_stat && we ) begin

        case (addr_core_reg[3:0]) // 0x100000 -> 0x1FFFFF = Core control registers
            'h00 : begin     // 0x100000

                if (wdata[0]) begin
                    ctrl_stop_tog           <= ~ctrl_stop_tog;
                end
                if (wdata[1]) begin
                    ctrl_update_cfg_tog     <= ~ctrl_update_cfg_tog;
                end

                if (wdata[1] | wdata[0]) begin
                    stat_cfg_updated_latch <= 1'b0; // clear status just before we update config or stop CU as SW will detect its asserted in both case
                end
                ctrl_test_mode              <= wdata[5:4];
                ctrl_reset                  <= wdata[8];

                if (wdata[12]) begin
                    ctrl_clear_err_tog        <= ~ctrl_clear_err_tog;
                end

                ctrl_wr_burst_req_rate_en   <= wdata[16];
                ctrl_rd_burst_req_rate_en   <= wdata[17];
                ctrl_wr_outstanding_en      <= wdata[18];
                ctrl_rd_outstanding_en      <= wdata[19];
                if (C_USE_AXI_ID == 1) begin
                    ctrl_axi_id_en          <= wdata[20];
                end
                ctrl_user_pattern_en        <= wdata[21];

            end
            'h01 : begin     // 0x100001
                ctrl_wr_start_addr_lsb <= wdata;
            end
            'h02 : begin     // 0x100002
                ctrl_wr_start_addr_msb <=  wdata;
            end
            'h03 : begin     // 0x100003
                ctrl_rd_start_addr_lsb <= wdata;
            end
            'h04 : begin     // 0x100004
                ctrl_rd_start_addr_msb <= wdata;
            end
            'h05 : begin     // 0x100005
                ctrl_wr_burst_size  <= wdata[8-1:0];
            end
            'h06 : begin     // 0x100006
                ctrl_rd_burst_size  <= wdata[8-1:0];
            end
            'h07 : begin     // 0x100007
                ctrl_wr_num_xfer <= wdata[C_NUM_XFER_SIZE_WIDTH-1:0];
            end
            'h08 : begin     // 0x100008
                ctrl_rd_num_xfer <= wdata[C_NUM_XFER_SIZE_WIDTH-1:0];
            end
            'h09 : begin     // 0x100009
                ctrl_wr_burst_req_rate <= wdata;
            end
            'h0A : begin     // 0x10000A
                ctrl_rd_burst_req_rate <= wdata;
            end
            'h0B : begin // 0x10000B
                ctrl_wr_outstanding <= wdata[24:16];
                ctrl_rd_outstanding <= wdata[8:0];
            end
            'h0C : begin     // 0x10000C
                ctrl_user_pattern_a <= wdata;
            end
            'h0D : begin     // 0x10000D
                ctrl_user_pattern_b <= wdata;
            end
            /*
            'h0E : begin // 0x10000E
            end
            ...
            'hFFFFF : begin // 0x1FFFFF
            end
            */
            default : $display("Illegal address sent to memtest_reg_array");
        endcase

    end

    // pipeline the address decoding to ensure timing closure for ap_clk > 300MHz
    // sel_core_reg & sel_core_stat are used to select BI, Stat or ctrl
    // so far, max 5 bits addresses are used to select registers within a group (stat or ctrl)
    // create

    // drive the pipeline
    read_pipe[1:READ_PIPE_SIZE-1] <= read_pipe[0:READ_PIPE_SIZE-2];
    read_pipe[0].cs   <= cs & ~we;
    read_pipe[0].addr <= addr;

    // selection based on the 2 LSB
    // 0,1,2,3 goes into => 0; 4,5,6,7 => 1; 8,9,10,11 => 2; 12,13,14,15 => 3; 16,17,18,19 => 4; 20,21,22,23 => 5
    for (int i = 0; i < 8; i = i + 1) begin
        case (addr[1:0])
            'b00 : begin
                read_pipe[0].rdata_ctrl[i] <= rdata_ctrl[i*4];
                read_pipe[0].rdata_stat[i] <= rdata_stat[i*4];
            end
            'b01 : begin
                read_pipe[0].rdata_ctrl[i] <= rdata_ctrl[i*4 + 1];
                read_pipe[0].rdata_stat[i] <= rdata_stat[i*4 + 1];
            end
            'b10 : begin
                read_pipe[0].rdata_ctrl[i] <= rdata_ctrl[i*4 + 2];
                read_pipe[0].rdata_stat[i] <= rdata_stat[i*4 + 2];
            end
            default : begin
                read_pipe[0].rdata_ctrl[i] <= rdata_ctrl[i*4 + 3];
                read_pipe[0].rdata_stat[i] <= rdata_stat[i*4 + 3];
            end
        endcase
    end

    // Stage 1
    for (int i = 0; i < 2; i = i + 1) begin
        case (read_pipe[0].addr[3:2])
            'b00 : begin
                read_pipe[1].rdata_ctrl[i] <= read_pipe[0].rdata_ctrl[i*4];
                read_pipe[1].rdata_stat[i] <= read_pipe[0].rdata_stat[i*4];
            end
            'b01 : begin
                read_pipe[1].rdata_ctrl[i] <= read_pipe[0].rdata_ctrl[i*4 + 1];
                read_pipe[1].rdata_stat[i] <= read_pipe[0].rdata_stat[i*4 + 1];
            end
            'b10 : begin
                read_pipe[1].rdata_ctrl[i] <= read_pipe[0].rdata_ctrl[i*4 + 2];
                read_pipe[1].rdata_stat[i] <= read_pipe[0].rdata_stat[i*4 + 2];
            end
            default : begin
                read_pipe[1].rdata_ctrl[i] <= read_pipe[0].rdata_ctrl[i*4 + 3];
                read_pipe[1].rdata_stat[i] <= read_pipe[0].rdata_stat[i*4 + 3];
            end
        endcase
    end


    // Stage 2
    case ({read_pipe[1].addr[19], read_pipe[1].addr[4]}) // addr[19] = sel_core_stat
        'b00 : begin
            read_pipe[2].rdata_stat[0] <= read_pipe[1].rdata_ctrl[0];
        end
        'b01 : begin
            read_pipe[2].rdata_stat[0] <= read_pipe[1].rdata_ctrl[1];
        end
        'b10 : begin
            read_pipe[2].rdata_stat[0] <= read_pipe[1].rdata_stat[0];
        end
        default : begin
            read_pipe[2].rdata_stat[0] <= read_pipe[1].rdata_stat[1];
        end
    endcase


    // Stage 3
    rdata             <= '0;
    if (read_pipe[2].cs) begin
        if (~read_pipe[2].addr[20]) begin // addr[20] = sel_core_reg
            if (read_pipe[2].addr[4]) begin    // addr[4] = sel_bi_ctrl   //  0x10 -> 0x1F = common ctrl and status
                rdata <= rdata_ctrl_status;
            end else begin                                              //  0x8 -> 0xF  = Unused
                rdata <= rdata_bi;                                      //  0x0 -> 0x7  = build info
            end
        end else begin
            rdata <= read_pipe[2].rdata_stat[0];
        end
    end

    //automatically acknowledge write command
    cmd_cmplt <= (cs && we) || read_pipe[2].cs;


    if (rst) begin

        ctrl_reset                  <= '0;
        stat_cfg_updated_latch      <= '0;
        cmd_cmplt                   <= '0;

    end
end

assign ctrl_wr_start_addr = { ctrl_wr_start_addr_msb, ctrl_wr_start_addr_lsb };
assign ctrl_rd_start_addr = { ctrl_rd_start_addr_msb, ctrl_rd_start_addr_lsb };

endmodule : memtest_reg_array
`default_nettype wire
