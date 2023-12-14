
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

module AXIS_Controller #(
    parameter integer C_MEM_KRNL_CORE_IDX       = 0,    // Memory kernel core index
    parameter integer C_XFER_SIZE_WIDTH         = 15,   // Width of the ctrl_xfer_size_in_bytes
    parameter integer C_NUM_XFER_SIZE_WIDTH     = 28,   // Width of the ctrl_xfer_cnt
    parameter integer C_M_AXI_THREAD_ID_WIDTH   = 2,    // Set to the address width of the interface
    parameter integer C_M_AXI_ADDR_WIDTH        = 64,   // Set to the address width of the interface
    parameter integer C_M_AXI_DATA_WIDTH        = 512
)(
    input  wire                                 clk,
    input  wire                                 rst,
    input  wire  [64-1:0]                       axi_addr_ptr,

    output logic [C_M_AXI_THREAD_ID_WIDTH-1:0]  Gen_ID_Out = '0,
    output wire                                 Gen_Valid_Out,
    output wire                                 Gen_Last_Out,
    input  wire                                 Gen_Ready_In,

    output logic                                suppress_error_o,

    output logic [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_awid_o = '0,
    output wire                                 axi_awvalid_o,
    input  wire                                 axi_awready_i,
    output wire  [C_M_AXI_ADDR_WIDTH-1:0]       axi_awaddr_o,
    output logic [8-1:0]                        axi_awlen_o = '0,
    input  wire                                 axi_bvalid_i,


    output logic [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_arid_o = '0,
    output wire                                 axi_arvalid_o,
    input  wire                                 axi_arready_i,
    output wire  [C_M_AXI_ADDR_WIDTH-1:0]       axi_araddr_o,
    output logic [8-1:0]                        axi_arlen_o = '0,
    input  wire  [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_rid_i,
    input  wire                                 axi_rvalid_i,
    input  wire                                 axi_rlast_i,

    // initialise or abort test and specify what type
    input  wire                                 ctrl_stop_tog,
    input  wire                                 ctrl_update_cfg_tog,
    input  wire [1:0]                           ctrl_test_mode,
    // Write control
    input  wire [C_NUM_XFER_SIZE_WIDTH-1:0]     ctrl_wr_num_xfer,
    input  wire [C_M_AXI_ADDR_WIDTH-1:0]        ctrl_wr_start_addr,
    input  wire [8-1:0]                         ctrl_wr_burst_size,
    // Read control
    input  wire [C_NUM_XFER_SIZE_WIDTH-1:0]     ctrl_rd_num_xfer,
    input  wire [C_M_AXI_ADDR_WIDTH-1:0]        ctrl_rd_start_addr,
    input  wire [8-1:0]                         ctrl_rd_burst_size,

    input  wire                                 ctrl_wr_burst_req_rate_en,
    input  wire [31:0]                          ctrl_wr_burst_req_rate,
    input  wire                                 ctrl_rd_burst_req_rate_en,
    input  wire [31:0]                          ctrl_rd_burst_req_rate,

    input wire                                  ctrl_wr_outstanding_en,
    input wire [8:0]                            ctrl_wr_outstanding,
    input wire                                  ctrl_rd_outstanding_en,
    input wire [8:0]                            ctrl_rd_outstanding,

    input wire                                  ctrl_axi_id_en,

    output logic                                stat_cfg_updated_tog = 'b0
);


///////////////////////////////////////////////////////////////////////////////
// functions
///////////////////////////////////////////////////////////////////////////////
function integer f_max ( input integer a, input integer b );
    f_max = (a < b) ? b : a;
endfunction


////////////////////////////////////////////////////////////
// Local parameters
////////////////////////////////////////////////////////////
localparam integer C_NUM_ID = 2**(C_M_AXI_THREAD_ID_WIDTH);

// Write FSM
localparam logic [1:0]  ST_WRITE_IDLE                   = 2'b01;
localparam logic [1:0]  ST_WRITE_CYCLE                  = 2'b10;
// Read FSM
localparam logic [1:0]  ST_READ_IDLE                    = 2'b01;
localparam logic [1:0]  ST_READ_CYCLE                   = 2'b10;

localparam logic [1:0]  C_TEST_MODE_ALTERNATE_WR_RD     = 2'b00;
localparam logic [1:0]  C_TEST_MODE_ONLY_WR             = 2'b01;
localparam logic [1:0]  C_TEST_MODE_ONLY_RD             = 2'b10;
localparam logic [1:0]  C_TEST_MODE_SIMULTANEOUS_WR_RD  = 2'b11;

localparam integer      LP_DW_BYTES                     = C_M_AXI_DATA_WIDTH/8;
localparam integer      LP_LOG_DW_BYTES                 = $clog2(LP_DW_BYTES);
localparam integer      LP_TOTAL_LEN_WIDTH              = C_XFER_SIZE_WIDTH-LP_LOG_DW_BYTES;

localparam integer      LP_XFER_IN_BURST_CNT_WIDTH      = f_max(LP_TOTAL_LEN_WIDTH,     24);
localparam integer      LP_TOTAL_XFER_CNT_WIDTH         = f_max(C_NUM_XFER_SIZE_WIDTH,  24);
localparam integer      LP_AXI_ADDR_CNT_WIDTH           = f_max(C_M_AXI_ADDR_WIDTH,     24);

localparam logic [1:0]  LP_BURST_CNT_ZERO               = {LP_XFER_IN_BURST_CNT_WIDTH{'0}};
localparam logic [1:0]  LP_BURST_CNT_ONE                = {{LP_XFER_IN_BURST_CNT_WIDTH-1{'0}}, 1'b1};
localparam logic [1:0]  LP_BURST_CNT_MAX                = {LP_XFER_IN_BURST_CNT_WIDTH{'1}};

localparam logic [1:0]  LP_NUM_XFER_CNT_ZERO            = {LP_TOTAL_XFER_CNT_WIDTH{'0}};
localparam logic [1:0]  LP_NUM_XFER_CNT_ONE             = {{LP_TOTAL_XFER_CNT_WIDTH-1{'0}}, 1'b1};
localparam logic [1:0]  LP_NUM_XFER_CNT_MAX             = {LP_TOTAL_XFER_CNT_WIDTH{'1}};

localparam logic [1:0]  LP_AXI_ADDR_CNT_ZERO            = {LP_AXI_ADDR_CNT_WIDTH{'0}};
localparam logic [1:0]  LP_AXI_ADDR_CNT_ONE             = {{LP_AXI_ADDR_CNT_WIDTH-1{'0}}, 1'b1};
localparam logic [1:0]  LP_AXI_ADDR_CNT_MAX             = {LP_AXI_ADDR_CNT_WIDTH{'1}};

localparam integer      LP_WR_REQ_RATE_WIDTH            = 13;
localparam integer      LP_RD_REQ_RATE_WIDTH            = 13;

////////////////////////////////////////////////////////////
// Configuration
////////////////////////////////////////////////////////////

logic                               ctrl_stop_latch                 = '0;
logic                               update_wr_cfg_req               = '0;
logic                               update_rd_cfg_req               = '0;
logic                               update_cfg_req                  = '0;
logic                               update_cfg_req_d1               = '0;
logic                               update_cfg_pulse                = '0;
logic                               update_cfg_pulse_d1             = '0;

logic                               start_wr_cycle                  = '0;
logic                               start_rd_cycle                  = '0;

logic                               ctrl_enable_test_pulse          = '0;

// Write control
logic [C_NUM_XFER_SIZE_WIDTH-1:0]   ctrl_wr_num_xfer_r_minus2bs     = 'h0;
logic [C_NUM_XFER_SIZE_WIDTH-1:0]   ctrl_wr_num_xfer_r              = 'h0;
logic [8-1:0]                       ctrl_wr_burst_size_r            = 'h0; // Supported in SW from 8 AXI beats to 4kBytes AXI burst (depending on AXI data size)
logic [8-1:0]                       ctrl_wr_burst_size_r_plus1      = 'h0;
logic [8-1:0]                       ctrl_wr_burst_size_r_minus1     = 'h0;
// Read control
logic [C_NUM_XFER_SIZE_WIDTH-1:0]   ctrl_rd_num_xfer_r_minus2bs     = 'h0;
logic [C_NUM_XFER_SIZE_WIDTH-1:0]   ctrl_rd_num_xfer_r              = 'h0;
logic [8-1:0]                       ctrl_rd_burst_size_r            = 'h0;
logic [8-1:0]                       ctrl_rd_burst_size_r_plus1      = '0;

logic                               ctrl_wr_burst_req_rate_en_r     = '0;
logic [31:0]                        ctrl_wr_burst_req_rate_r        = '0;
logic                               ctrl_rd_burst_req_rate_en_r     = '0;
logic [31:0]                        ctrl_rd_burst_req_rate_r        = '0;

logic                               ctrl_wr_outstanding_en_r        = '0;
logic [8:0]                         ctrl_wr_outstanding_r           = '0;
logic                               ctrl_rd_outstanding_en_r        = '0;
logic [8:0]                         ctrl_rd_outstanding_r           = '0;

logic                               ctrl_axi_id_en_r                = '0;

logic                               alternate_wr_rd_mode_en         = '0;
logic                               only_wr_mode_en                 = '0;
logic                               only_rd_mode_en                 = '0;
logic                               simultaneous_wr_rd_mode_en      = '0;

(* extract_enable = "no" *) logic [1:0] current_wr_state = ST_WRITE_IDLE;
(* extract_enable = "no" *) logic [1:0] current_rd_state = ST_READ_IDLE;

///////////////////////////////////////////////////////////////////////
// Outputs
///////////////////////////////////////////////////////////////////////
wire  awxfer;
logic axi_awvalid_s = '0;

wire  wxfer;
logic axi_wvalid_s = '0;

wire  last_wxfer;

wire  arxfer;
logic axi_arvalid_s = '0;

wire  rxfer;
wire  last_rxfer;

///////////////////////////////////////////////////////////////////////
// WRITE
///////////////////////////////////////////////////////////////////////

logic [7:0]                             wr_req                      = '0;
logic                                   wr_req_stop                 = '0;
logic [LP_WR_REQ_RATE_WIDTH:0]          wr_req_synth                = '0;
logic                                   wr_cycle_toggle             = '0;

// Compute AWID
wire  [C_M_AXI_THREAD_ID_WIDTH-1:0]     axi_awid_load_value         = {C_M_AXI_THREAD_ID_WIDTH{'0}};
wire  [C_M_AXI_THREAD_ID_WIDTH-1:0]     axi_awid_inc                = {1'b0, 1'b1};

// Compute AWADDR
logic [LP_AXI_ADDR_CNT_WIDTH-1:0]       axi_awaddr_load_value       = '0;
wire  [LP_AXI_ADDR_CNT_WIDTH-1:0]       axi_awaddr_inc              = {1'b0, ctrl_wr_burst_size_r_plus1, {LP_LOG_DW_BYTES{'0}}};
logic [LP_AXI_ADDR_CNT_WIDTH-1:0]       axi_awaddr_s                = '0;

// Compute WID
wire  [C_M_AXI_THREAD_ID_WIDTH-1:0]     axi_wid_load_value          = {C_M_AXI_THREAD_ID_WIDTH{'0}};
wire  [C_M_AXI_THREAD_ID_WIDTH-1:0]     axi_wid_inc                 = {1'b0, 1'b1};;

// Count number of AXI write burst commands
wire  [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_awvalid_cnt_load        = {1'b0, ctrl_wr_num_xfer_r_minus2bs};
wire  [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_awvalid_cnt_dec         = {1'b0, ctrl_wr_burst_size_r_plus1};
logic [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_awvalid_cnt_s           = '0;

// Count number of AXI write burst responses
wire  [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_bvalid_cnt_load         = {1'b0, ctrl_wr_num_xfer_r_minus2bs};
wire  [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_bvalid_cnt_dec          = {1'b0, ctrl_wr_burst_size_r_plus1};
logic [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_bvalid_cnt_s            = '0;

// Count number of AXI write burst sent
wire  [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_wlast_cnt_load          = {1'b0, ctrl_wr_num_xfer_r_minus2bs};
wire  [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_wlast_cnt_dec           = {1'b0, ctrl_wr_burst_size_r_plus1};
logic [LP_TOTAL_XFER_CNT_WIDTH-1:0]     axi_wlast_cnt_s             = '0;

// Count number of write transfers in current AXI burst
wire  [LP_XFER_IN_BURST_CNT_WIDTH-1:0]  wxfer_in_burst_cnt_load     = {1'b0, ctrl_wr_burst_size_r_minus1};
logic [LP_XFER_IN_BURST_CNT_WIDTH-1:0]  wxfer_in_burst_cnt_s        = '0;

// Count number of AXI outstanding write burst requests
wire  [8:0]                             axi_wr_outstanding_cnt_load = ctrl_wr_outstanding_r; // -1 means max number of outstanding = 1
logic [8:0]                             axi_wr_outstanding_cnt_s    = '0;
logic                                   axi_bvalid_latch            = '0;

///////////////////////////////////////////////////////////////////////
// READ
///////////////////////////////////////////////////////////////////////

logic [7:0]                         rd_req                      = '0;
logic                               rd_req_stop                 = '0;
logic [LP_RD_REQ_RATE_WIDTH:0]      rd_req_synth                = '0;
logic                               rd_cycle_toggle             = '0;
logic [C_NUM_ID-1:0]                first_rd_valid_id           = '0;

// Compute ARID
wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_arid_load_value         = {C_M_AXI_THREAD_ID_WIDTH{'0}};
wire [C_M_AXI_THREAD_ID_WIDTH-1:0]  axi_arid_inc                = {1'b0, 1'b1};

// Compute ARADDR
logic [LP_AXI_ADDR_CNT_WIDTH-1:0]   axi_araddr_load_value       = '0;
wire  [LP_AXI_ADDR_CNT_WIDTH-1:0]   axi_araddr_inc              = {1'b0, ctrl_rd_burst_size_r_plus1, {LP_LOG_DW_BYTES{'0}}};
logic [LP_AXI_ADDR_CNT_WIDTH-1:0]   axi_araddr_s                = '0;

// Count number of AXI read burst commands
wire  [LP_TOTAL_XFER_CNT_WIDTH:0]   axi_arvalid_cnt_load        = {1'b0, ctrl_rd_num_xfer_r_minus2bs};
wire  [LP_TOTAL_XFER_CNT_WIDTH:0]   axi_arvalid_cnt_dec         = {1'b0, ctrl_rd_burst_size_r_plus1};
logic [LP_TOTAL_XFER_CNT_WIDTH:0]   axi_arvalid_cnt_s           = '0;

// Count number of AXI read burst received (decrement)
wire  [LP_TOTAL_XFER_CNT_WIDTH:0]   axi_rlast_cnt_load          = {1'b0, ctrl_rd_num_xfer_r_minus2bs};
wire  [LP_TOTAL_XFER_CNT_WIDTH:0]   axi_rlast_cnt_dec           = {1'b0, ctrl_rd_burst_size_r_plus1};
logic [LP_TOTAL_XFER_CNT_WIDTH:0]   axi_rlast_cnt_s             = '0;

// Count number of AXI outstanding read burst requests
wire  [8:0]                         axi_rd_outstanding_cnt_load = ctrl_rd_outstanding_r; // -1 means max number of outstanding = 1
logic [8:0]                         axi_rd_outstanding_cnt_s    = '0;
logic                               last_rxfer_latch            = '0;


wire    stop_tog;
logic   stop_tog_d1;
logic   stop_tog_d2;
logic   ctrl_stop_pulse;
wire    update_cfg_tog;
logic   update_cfg_tog_d1;
logic   update_cfg_tog_d2;
logic   ctrl_update_cfg_pulse;

// use XPM to create false path automatically
xpm_cdc_array_single #(
    .DEST_SYNC_FF   ( 2 ),
    .INIT_SYNC_FF   ( 0 ),
    .SRC_INPUT_REG  ( 0 ),
    .WIDTH          ( 2)
)
xpm_pulse (
    .src_clk  (  1'h0         ),
    .src_in   ( {ctrl_stop_tog, ctrl_update_cfg_tog } ),
    .dest_out ( {stop_tog, update_cfg_tog} ),
    .dest_clk ( clk           )
);

always_ff @(posedge clk) begin
    stop_tog_d1 <= stop_tog;
    stop_tog_d2 <= stop_tog_d1;
    ctrl_stop_pulse <= stop_tog_d1 ^ stop_tog_d2;

    update_cfg_tog_d1 <= update_cfg_tog;
    update_cfg_tog_d2 <= update_cfg_tog_d1;
    ctrl_update_cfg_pulse <= update_cfg_tog_d1 ^ update_cfg_tog_d2;

end

///////////////////////////////////////////////////////////////////////
// Outputs
///////////////////////////////////////////////////////////////////////
// Write requests
assign awxfer           = axi_awvalid_s & axi_awready_i;
assign axi_awvalid_o    = axi_awvalid_s;
assign axi_awaddr_o     = axi_awaddr_s[$high(axi_awaddr_o):0];
// Write data transfers
assign wxfer            = axi_wvalid_s & Gen_Ready_In;
assign last_wxfer       = wxfer_in_burst_cnt_s[$high(wxfer_in_burst_cnt_s)] & wxfer;
assign Gen_Valid_Out    = wxfer; // Do not output axi_wvalid_s directly
assign Gen_Last_Out     = last_wxfer;
// Read requests
assign arxfer           = axi_arvalid_s & axi_arready_i;
assign axi_arvalid_o    = axi_arvalid_s;
assign axi_araddr_o     = axi_araddr_s[$high(axi_araddr_o):0];
// Read data transfers
assign rxfer            = axi_rvalid_i;
assign last_rxfer       = axi_rlast_i & rxfer;
assign suppress_error_o = first_rd_valid_id[axi_rid_i] & rxfer;

always_ff @(posedge clk) begin

    /////////////////////////////////////////////////////////
    // WRITE

    if (~ctrl_wr_burst_req_rate_en_r) begin
        wr_req_synth <= {1'b1, wr_req_synth[$high(wr_req_synth)-1:0]};
    end else if (~wr_req[$high(wr_req)]) begin
        wr_req_synth <= {1'b0, wr_req_synth[$high(wr_req_synth)-1:0]} + {1'b0, ctrl_wr_burst_req_rate_r[$high(wr_req_synth)-1:0]};
    end else begin
        wr_req_synth <= {1'b0, wr_req_synth[$high(wr_req_synth)-1:0]};
    end

    // Write FSM
    unique case(1'b1) // use unique state to optimize logic

        ////////////////////////////////////////////////////////////
        // ST_WRITE_IDLE

        current_wr_state[0]: begin
            axi_awvalid_cnt_s           <= axi_awvalid_cnt_load;
            axi_awaddr_s                <= axi_awaddr_load_value;
            wxfer_in_burst_cnt_s        <= wxfer_in_burst_cnt_load;
            axi_wlast_cnt_s             <= axi_wlast_cnt_load;
            axi_bvalid_cnt_s            <= axi_bvalid_cnt_load;
            wr_req                      <= 'h0;
            axi_wr_outstanding_cnt_s    <= axi_wr_outstanding_cnt_load;
            // wait for the start of next write cycle. block if new config received
            if (update_wr_cfg_req) begin
                update_wr_cfg_req   <= 1'b0;
                // Do not reset synth between cycles to keep same delay between final req of prev cycle and first of next cycle
                wr_req_synth        <= {~ctrl_wr_burst_req_rate_en_r, ctrl_wr_burst_req_rate_r[$high(wr_req_synth)-1:0]};
                // Do not reset ID between cycles
                axi_awid_o          <= axi_awid_load_value;
                Gen_ID_Out          <= axi_wid_load_value;
            end else if (start_wr_cycle) begin
                // Set all values and use what is necessary for the test requested
                current_wr_state    <= ST_WRITE_CYCLE;
                axi_awvalid_s       <= 1'b1;
                wr_req_stop         <= 1'b0;
                wr_req              <= {1'b0,1'b1};
                axi_wvalid_s        <= 1'b1;
                axi_awlen_o         <= ctrl_wr_burst_size_r;
            end
        end

        ////////////////////////////////////////////////////////////
        // ST_WRITE_CYCLE

        current_wr_state[1]: begin
            // Write burst commands
            if (~wr_req_stop) begin
                if (~wr_req[$high(wr_req)] & wr_req_synth[$high(wr_req_synth)]) begin
                    if (~awxfer) begin
                        wr_req <= wr_req + 1;
                    end
                end else begin
                    if (awxfer) begin
                        wr_req <= wr_req - 1;
                    end
                end

                if (ctrl_wr_outstanding_en_r) begin
                    if (awxfer) begin
                        if (~axi_bvalid_i) begin
                            axi_wr_outstanding_cnt_s <= axi_wr_outstanding_cnt_s - 1;
                        end
                    end else begin
                        if (axi_bvalid_i) begin
                            axi_wr_outstanding_cnt_s <= axi_wr_outstanding_cnt_s + 1;
                        end
                    end
                end

                // We use this latch to send as soon as we can when maximum number of outstand transaction is limited to 1
                if (axi_bvalid_i) begin
                    axi_bvalid_latch <= 1'b1;
                end

                // If we need to send a new request based on the rate : ((wr_req != 0) | wr_req_synth[$high(wr_req_synth)])
                // And we can do it because we are not limited by maximum outstanding: (~axi_wr_outstanding_cnt_s[$high(axi_wr_outstanding_cnt_s)] | axi_bvalid_latch)
                if ( ((wr_req != 0) | wr_req_synth[$high(wr_req_synth)])
                   & (~axi_wr_outstanding_cnt_s[$high(axi_wr_outstanding_cnt_s)] | axi_bvalid_latch)) begin
                    axi_awvalid_s <= 1'b1;
                    axi_bvalid_latch <= 1'b0;
                end

            end

            if (awxfer) begin
                if (ctrl_axi_id_en_r) begin
                    axi_awid_o <= axi_awid_o + axi_awid_inc;
                end
                axi_awvalid_cnt_s   <= axi_awvalid_cnt_s - axi_awvalid_cnt_dec;
                axi_awaddr_s        <= axi_awaddr_s      + axi_awaddr_inc;

                // slow request rate
                if ((wr_req == 'h1) & ~wr_req_synth[$high(wr_req_synth)]) begin // Last being sent
                    axi_awvalid_s <= 1'b0;
                end
                // too many request outstanding
                if (axi_wr_outstanding_cnt_s[$high(axi_wr_outstanding_cnt_s)]) begin
                    axi_awvalid_s <= 1'b0;
                end
                // last AXI burst command sent
                if (axi_awvalid_cnt_s[$high(axi_awvalid_cnt_s)]) begin
                    wr_req_stop   <= 1'b1;
                    axi_awvalid_s <= 1'b0;
                end
            end

            // Write burst xfers
            if (last_wxfer) begin
                axi_wlast_cnt_s <= axi_wlast_cnt_s - axi_wlast_cnt_dec;

                if (ctrl_axi_id_en_r) begin
                    Gen_ID_Out <= Gen_ID_Out + axi_wid_inc;
                end
            end

            if (wxfer) begin
                wxfer_in_burst_cnt_s <= wxfer_in_burst_cnt_s - 1;
                // last AXI Beat of current burst sent
                if (wxfer_in_burst_cnt_s[$high(wxfer_in_burst_cnt_s)]) begin
                    wxfer_in_burst_cnt_s <= wxfer_in_burst_cnt_load;

                    // last AXI Beat of last burst sent, stop write xfers
                    if (axi_wlast_cnt_s[$high(axi_wlast_cnt_s)]) begin
                        axi_wvalid_s <= 1'b0;
                    end
                end
            end

            // End of cycle
            // TODO we could start next cycle one clock cycle before if we move last bvalid detection out the state machine
            if (axi_bvalid_i) begin
                axi_bvalid_cnt_s <= axi_bvalid_cnt_s - axi_bvalid_cnt_dec;

                // BRESP of last AXI burst received
                if (axi_bvalid_cnt_s[$high(axi_bvalid_cnt_s)]) begin

                    // End of cycle, the transfer type is switched in alternate test mode
                    if (alternate_wr_rd_mode_en) begin
                        start_wr_cycle      <= 1'b0;
                        start_rd_cycle      <= 1'b1;
                    end
                    current_wr_state    <= ST_WRITE_IDLE;

                    wr_cycle_toggle     <= ~wr_cycle_toggle; // DEBUG
                end
            end
        end

    endcase

    /////////////////////////////////////////////////////////
    // READ

    if (~ctrl_rd_burst_req_rate_en_r) begin
        rd_req_synth <= {1'b1, rd_req_synth[$high(rd_req_synth)-1:0]};
    end else if (~rd_req[$high(rd_req)]) begin
        rd_req_synth <= {1'b0, rd_req_synth[$high(rd_req_synth)-1:0]} + {1'b0, ctrl_rd_burst_req_rate_r[$high(rd_req_synth)-1:0]};
    end else begin
        rd_req_synth <= {1'b0, rd_req_synth[$high(rd_req_synth)-1:0]};
    end

    // Read FSM
    unique case(1'b1) // use unique state to optimize logic

        ////////////////////////////////////////////////////////////
        // ST_READ_IDLE

        current_rd_state[0]: begin
            axi_arvalid_cnt_s           <= axi_arvalid_cnt_load;
            axi_araddr_s                <= axi_araddr_load_value;
            axi_rlast_cnt_s             <= axi_rlast_cnt_load;
            rd_req                      <= {$size(rd_req){1'b0}};
            axi_rd_outstanding_cnt_s    <= axi_rd_outstanding_cnt_load;
            // wait for the start of next read cycle
            // block if new config received
            if (update_rd_cfg_req) begin
                update_rd_cfg_req   <= 1'b0;
                // Do not reset synth between cycles to keep same delay between final req of prev cycle and first of next cycle
                rd_req_synth        <= {~ctrl_rd_burst_req_rate_en_r, ctrl_rd_burst_req_rate_r[$high(rd_req_synth)-1:0]};
                // Do not reset ID between cycles
                axi_arid_o          <= axi_arid_load_value;
                first_rd_valid_id   <= {$size(first_rd_valid_id){1'b1}};
            end else if (start_rd_cycle) begin
                current_rd_state    <= ST_READ_CYCLE;
                axi_arvalid_s       <= 1'b1;
                rd_req_stop         <= 1'b0;
                rd_req              <= {1'b0, 1'b1};
                axi_arlen_o         <= ctrl_rd_burst_size_r;
            end
        end

        ////////////////////////////////////////////////////////////
        // ST_READ_CYCLE

        current_rd_state[1]: begin
            // Read burst commands

            if (~rd_req_stop) begin

                if (~rd_req[$high(rd_req)] & rd_req_synth[$high(rd_req_synth)]) begin
                    if (~arxfer) begin
                        rd_req <= rd_req + 1;
                    end
                end else begin
                    if (arxfer) begin
                        rd_req <= rd_req - 1;
                    end
                end

                if (ctrl_rd_outstanding_en_r) begin
                    if (arxfer) begin
                        if (~last_rxfer) begin
                            axi_rd_outstanding_cnt_s <= axi_rd_outstanding_cnt_s - 1;
                        end
                    end else begin
                        if (last_rxfer) begin
                            axi_rd_outstanding_cnt_s <= axi_rd_outstanding_cnt_s + 1;
                        end
                    end
                end

                // We use this latch to send as soon as we can when maximum number of outstand transaction is limited to 1
                if (last_rxfer) begin
                    last_rxfer_latch <= 1'b1;
                end

                // If we need to send a new request based on the rate : ((rd_req != 0) | rd_req_synth[$high(rd_req_synth)])
                // And we can do it because we are not limited by maximum outstanding: (~axi_rd_outstanding_cnt_s[$high(axi_rd_outstanding_cnt_s)] | last_rxfer_latch)
                if ( ((rd_req != 0) | rd_req_synth[$high(rd_req_synth)])
                   & (~axi_rd_outstanding_cnt_s[$high(axi_rd_outstanding_cnt_s)] | last_rxfer_latch)) begin
                    axi_arvalid_s       <= 1'b1;
                    last_rxfer_latch    <= 1'b0;
                end

            end

            if (arxfer) begin
                if (ctrl_axi_id_en_r) begin
                    axi_arid_o <= axi_arid_o + axi_arid_inc;
                end
                axi_arvalid_cnt_s   <= axi_arvalid_cnt_s - axi_arvalid_cnt_dec;
                axi_araddr_s        <= axi_araddr_s      + axi_araddr_inc;

                // slow request rate
                if ((rd_req == 'h1) & ~rd_req_synth[$high(rd_req_synth)] ) begin
                    axi_arvalid_s <= 1'b0;
                end
                // too many request outstanding
                if (axi_rd_outstanding_cnt_s[$high(axi_rd_outstanding_cnt_s)]) begin
                    axi_arvalid_s <= 1'b0;
                end
                // last AXI burst command sent
                if (axi_arvalid_cnt_s[$high(axi_arvalid_cnt_s)]) begin
                    rd_req_stop   <= 1'b1;
                    axi_arvalid_s <= 1'b0;
                end
            end

            // Read transfers: can be interleaved for each ID (e.g NOC) or not (e.g Mem-subsystem)
            if (rxfer) begin
                first_rd_valid_id[axi_rid_i] <= 1'b0;
            end

            // End of cycle
            if (last_rxfer) begin
                axi_rlast_cnt_s <= axi_rlast_cnt_s - axi_rlast_cnt_dec;

                // last AXI Beat of last burst received
                if (axi_rlast_cnt_s[$high(axi_rlast_cnt_s)]) begin
                    if (alternate_wr_rd_mode_en) begin
                        start_wr_cycle  <= 1'b1;
                        start_rd_cycle  <= 1'b0;
                    end else begin
                        first_rd_valid_id <= {$size(first_rd_valid_id){1'b1}};
                    end
                    current_rd_state <= ST_READ_IDLE;

                    rd_cycle_toggle     <= ~rd_cycle_toggle; // DEBUG
                end
            end
        end

    endcase

    /////////////////////////////////////////////////////////
    // Configuration
    /////////////////////////////////////////////////////////
    // Configuration logic written after FSMs in process as it has priority for start_wr_cycle and start_rd_cycle

    // Test will run until a stop command is received
    if (ctrl_stop_pulse) begin
        ctrl_stop_latch <= 1'b1;
    end

    if (update_cfg_pulse) begin
        axi_awaddr_load_value           <= axi_addr_ptr + ctrl_wr_start_addr;
        ctrl_wr_num_xfer_r_minus2bs     <= ctrl_wr_num_xfer - {1'b0,ctrl_wr_burst_size,1'b0};
        ctrl_wr_num_xfer_r              <= ctrl_wr_num_xfer;
        ctrl_wr_burst_size_r            <= ctrl_wr_burst_size;
        ctrl_wr_burst_size_r_plus1      <= ctrl_wr_burst_size + {1'b0, 1'b1};
        ctrl_wr_burst_size_r_minus1     <= ctrl_wr_burst_size - {1'b0, 1'b1};

        axi_araddr_load_value           <= axi_addr_ptr + ctrl_rd_start_addr;
        ctrl_rd_num_xfer_r_minus2bs     <= ctrl_rd_num_xfer - {1'b0,ctrl_rd_burst_size,1'b0};
        ctrl_rd_num_xfer_r              <= ctrl_rd_num_xfer;
        ctrl_rd_burst_size_r            <= ctrl_rd_burst_size;
        ctrl_rd_burst_size_r_plus1      <= ctrl_rd_burst_size + {1'b0, 1'b1};

        ctrl_wr_burst_req_rate_en_r     <= ctrl_wr_burst_req_rate_en;
        ctrl_wr_burst_req_rate_r        <= ctrl_wr_burst_req_rate;
        ctrl_rd_burst_req_rate_en_r     <= ctrl_rd_burst_req_rate_en;
        ctrl_rd_burst_req_rate_r        <= ctrl_rd_burst_req_rate;

        ctrl_wr_outstanding_en_r        <= ctrl_wr_outstanding_en;
        ctrl_wr_outstanding_r           <= ctrl_wr_outstanding;
        ctrl_rd_outstanding_en_r        <= ctrl_rd_outstanding_en;
        ctrl_rd_outstanding_r           <= ctrl_rd_outstanding;

        ctrl_axi_id_en_r                <= ctrl_axi_id_en;

        // pre-compute to reduce logic level
        alternate_wr_rd_mode_en         <= ~ctrl_stop_latch && (ctrl_test_mode == C_TEST_MODE_ALTERNATE_WR_RD   ) ? 1'b1 : 1'b0;
        only_wr_mode_en                 <= ~ctrl_stop_latch && (ctrl_test_mode == C_TEST_MODE_ONLY_WR           ) ? 1'b1 : 1'b0;
        only_rd_mode_en                 <= ~ctrl_stop_latch && (ctrl_test_mode == C_TEST_MODE_ONLY_RD           ) ? 1'b1 : 1'b0;
        simultaneous_wr_rd_mode_en      <= ~ctrl_stop_latch && (ctrl_test_mode == C_TEST_MODE_SIMULTANEOUS_WR_RD) ? 1'b1 : 1'b0;
        ctrl_stop_latch                 <= 1'b0;
    end

    if (ctrl_stop_pulse | ctrl_update_cfg_pulse) begin
        // Latch update configuration request
        update_cfg_req      <= 1'b1;
        // Send configuration request to write/read FSM
        update_wr_cfg_req   <= 1'b1;
        update_rd_cfg_req   <= 1'b1;
    end

    update_cfg_pulse <= 1'b0;
    // wait for both write/read FSM to arrive IDLE state
    if (update_cfg_req & ~update_wr_cfg_req & ~update_rd_cfg_req) begin
        update_cfg_req      <= 1'b0;
        update_cfg_pulse    <= 1'b1;
    end

    update_cfg_pulse_d1 <= update_cfg_pulse; // One cycle to update config

    // Enable FSM one clock cycle after
    ctrl_enable_test_pulse <= 1'b0;
    // once the config has been updated, allow the start of test.
    if (update_cfg_pulse_d1) begin
        ctrl_enable_test_pulse <= 1'b1;
        stat_cfg_updated_tog <= ~stat_cfg_updated_tog;
    end

    if (update_wr_cfg_req | update_rd_cfg_req) begin
        // hold as waiting for the config to be propagated
        start_wr_cycle <= 1'b0;
        start_rd_cycle <= 1'b0;
    end else if (ctrl_enable_test_pulse) begin
        start_wr_cycle <= 1'b0;
        start_rd_cycle <= 1'b0;
        // Start a test
        if (only_rd_mode_en) begin
            start_rd_cycle <= 1'b1;
        end else if (simultaneous_wr_rd_mode_en) begin
            start_wr_cycle <= 1'b1;
            start_rd_cycle <= 1'b1;
        end else if (alternate_wr_rd_mode_en | only_wr_mode_en) begin
            start_wr_cycle <= 1'b1;
        end
    end

    /////////////////////////////////////////////////////////
    // Reset
    /////////////////////////////////////////////////////////

    if (rst) begin
        current_wr_state    <= ST_WRITE_IDLE;
        current_rd_state    <= ST_READ_IDLE;

        start_wr_cycle      <= 1'b0;
        start_rd_cycle      <= 1'b0;

        ctrl_stop_latch     <= 1'b0;

        update_cfg_req      <= 1'b0;
        update_wr_cfg_req   <= 1'b0;
        update_rd_cfg_req   <= 1'b0;
        update_cfg_pulse    <= 1'b0;

        wr_req              <= {$size(wr_req){1'b0}};
        wr_req_synth        <= {$size(wr_req_synth){1'b0}};
        axi_bvalid_latch    <= '0;

        rd_req              <= {$size(rd_req){1'b0}};
        rd_req_synth        <= {$size(rd_req_synth){1'b0}};

        first_rd_valid_id   <= '0;

        axi_awlen_o         <= '0;
        axi_awid_o          <= '0;
        axi_awaddr_s        <= '0;
        axi_awvalid_s       <= '0;

        Gen_ID_Out          <= '0;
        axi_wvalid_s        <= '0;

        axi_arlen_o         <= '0;
        axi_arid_o          <= '0;
        axi_araddr_s        <= '0;
        axi_arvalid_s       <= '0;
    end
end

endmodule: AXIS_Controller
`default_nettype wire
