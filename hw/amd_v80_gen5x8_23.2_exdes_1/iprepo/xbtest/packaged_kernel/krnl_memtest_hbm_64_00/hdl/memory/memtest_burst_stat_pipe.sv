
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

module memtest_burst_stat_pipe  #(
    parameter integer C_STAT_MEAS_SIZE  = 32,
    parameter integer C_STAT_TOTAL_SIZE = 64,
    parameter integer C_STAT_INST_SIZE  = 32,
    parameter integer C_REPORT_MIN_MAX  = 0
) (
    input  wire                             clk,
    input  wire                             rst,
    input  wire                             toggle_1sec,
    input  wire                             meas_valid,
    input  wire  [C_STAT_MEAS_SIZE-1:0]     meas,
    output logic [C_STAT_TOTAL_SIZE-1:0]    stat_total,
    output logic [C_STAT_INST_SIZE-1:0]     stat_inst,
    output logic [C_STAT_MEAS_SIZE-1:0]     stat_min,
    output logic [C_STAT_MEAS_SIZE-1:0]     stat_max
);

timeunit 1ps;
timeprecision 1ps;

(*dont_touch ="true"*) logic toggle_1sec_d = '0;
(*dont_touch ="true"*) logic toggle_1sec_trigger = '0;
logic toggle_1sec_trigger_d = '0; // no dont_touch to allow replication

typedef struct {
    logic                           valid;
    logic [C_STAT_MEAS_SIZE-1:0]    meas;
    logic [C_STAT_TOTAL_SIZE-1:0]   total;
    logic [C_STAT_INST_SIZE-1:0]    inst;
    logic [C_STAT_MEAS_SIZE:0]      min_chk;
    logic [C_STAT_MEAS_SIZE:0]      max_chk;
    logic [C_STAT_MEAS_SIZE:0]      meas_d_chk;
    logic [C_STAT_MEAS_SIZE-1:0]    min;
    logic [C_STAT_MEAS_SIZE-1:0]    max;
} Pipe_Type;

Pipe_Type C_RST_PIPE = '{
    valid           : '0,
    meas            : '0,
    total           : '0,
    inst            : '0,
    min_chk         : '0,
    max_chk         : '0,
    meas_d_chk      : '0,
    min             : '{C_STAT_MEAS_SIZE{1'b1}},
    max             : '0
};
localparam integer PIPE_SIZE = 3;
Pipe_Type pipe [0:PIPE_SIZE-1] = '{default:C_RST_PIPE};


/*
(*dont_touch ="true"*) logic [C_STAT_TOTAL_SIZE-1:0]   total_c;
(*dont_touch ="true"*) logic [C_STAT_TOTAL_SIZE-1:0]   total_r;

logic [C_STAT_TOTAL_SIZE-1:0]   previous_total_c;
logic [C_STAT_TOTAL_SIZE-1:0]   new_total_c;

assign previous_total_c = (toggle_1sec_trigger)? 'b0 : total_r;
assign new_total_c = (meas_valid)? {1'b0, meas}: 'b0 ;
assign total_c = previous_total_c + new_total_c;

always_ff @(posedge clk) begin

    total_r <= total_c;
end
*/

////////////////////////////////////////////////////////////
// Pipeline
////////////////////////////////////////////////////////////

// Measures are not reset between tests (when update cfg is issued) as the AXI controller FSM should ensure the AXI burst are always fully sent/received
// We also expect the SW to wait at least one sec before reading the first measures

always_ff @(posedge clk) begin

    // drive the pipeline
    pipe[1:PIPE_SIZE-1] <= pipe[0:PIPE_SIZE-2];

    toggle_1sec_d <= toggle_1sec;

    if (toggle_1sec_d != toggle_1sec) begin
        toggle_1sec_trigger <= '1;
    end else begin
        toggle_1sec_trigger <= '0;
    end
    toggle_1sec_trigger_d <= toggle_1sec_trigger;

    ////////////////////////////////////////////////////////////
    // Stage 0 - Accumulate time. Prepare to check min/max update
    ////////////////////////////////////////////////////////////
    pipe[0].meas    <= meas;
    pipe[0].valid   <= meas_valid;

    if (meas_valid) begin

        pipe[0].min_chk     <= {1'b0,  meas}        - {1'b0, pipe[2].min};
        pipe[0].max_chk     <= {1'b0,  pipe[2].max} - {1'b0, meas};
        pipe[0].meas_d_chk  <= {1'b0,  meas}        - {1'b0, pipe[0].meas};  // Need to check back to back meas

    end

    // Accumulator
    pipe[0].total  <=  pipe[0].total;
    pipe[0].inst   <=  pipe[0].inst;

    if (meas_valid) begin

        pipe[0].total <=  pipe[0].total + {1'b0, meas};
        pipe[0].inst  <=  pipe[0].inst + 1;

    end

    // If toggle occurs while meas_valid, ignore it.
    if (toggle_1sec_trigger) begin

        pipe[0].total <= 'b0;
        pipe[0].inst  <= 'b0;

    end

    if (rst) begin

        pipe[0].valid   <= 1'b0;

    end

    ////////////////////////////////////////////////////////////
    // Stage 1
    ////////////////////////////////////////////////////////////
    // reset pipe
    if (rst || toggle_1sec_trigger) begin

        pipe[1].valid <= 1'b0;

    end

    ////////////////////////////////////////////////////////////
    // Stage 2 - Check min/max update
    ////////////////////////////////////////////////////////////

    pipe[2].min <= pipe[2].min;
    pipe[2].max <= pipe[2].max;

    if (pipe[1].valid) begin

        if (pipe[1].min_chk[C_STAT_MEAS_SIZE]) begin

            if (pipe[0].valid & (pipe[0].meas_d_chk[C_STAT_MEAS_SIZE])) begin

                pipe[2].min <= pipe[0].meas;

            end else begin

                pipe[2].min <= pipe[1].meas;

            end
        end

        if (pipe[1].max_chk[C_STAT_MEAS_SIZE]) begin

            if (pipe[0].valid & (~pipe[0].meas_d_chk[C_STAT_MEAS_SIZE])) begin

                pipe[2].max <= pipe[0].meas;

            end else begin

                pipe[2].max <= pipe[1].meas;

            end
        end
    end

    if (toggle_1sec_trigger) begin

        pipe[2].min   <= {C_STAT_MEAS_SIZE{1'b1}};
        pipe[2].max   <= {C_STAT_MEAS_SIZE{1'b0}};

    end

    ////////////////////////////////////////////////////////////
    // Latch measures of last second to be read by SW
    ////////////////////////////////////////////////////////////

    if (toggle_1sec_trigger) begin

        stat_total <= pipe[0].total;
        stat_inst  <= pipe[0].inst;
        stat_min   <= (C_REPORT_MIN_MAX)? pipe[2].min : 'h0;
        stat_max   <= (C_REPORT_MIN_MAX)? pipe[2].max : 'h0;

    end

end

endmodule : memtest_burst_stat_pipe

`default_nettype wire
