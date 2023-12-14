
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

module memtest_burst_latency #(
    parameter integer C_USE_AXI_ID      = 0,
    parameter integer C_ID_WIDTH        = 1,
    parameter integer C_STAT_MEAS_SIZE  = 32,
    parameter integer C_STAT_TOTAL_SIZE = 64,
    parameter integer C_STAT_INST_SIZE  = 32
) (
    input  wire                             clk,
    input  wire                             rst,
    input  wire                             toggle_1sec,
    input  wire [31:0]                      timestamp,
    input  wire                             start,
    input  wire                             stop,
    input  wire  [C_ID_WIDTH-1:0]           id_start,
    input  wire  [C_ID_WIDTH-1:0]           id_stop,
    output logic [C_STAT_TOTAL_SIZE-1:0]    stat_latency_total,
    output logic [C_STAT_INST_SIZE-1:0]     stat_latency_inst,
    output logic [C_STAT_MEAS_SIZE-1:0]     stat_latency_min,
    output logic [C_STAT_MEAS_SIZE-1:0]     stat_latency_max
);

timeunit 1ps;
timeprecision 1ps;

localparam integer C_NUM_ID = 2**(C_ID_WIDTH);

////////////////////////////////////////////////////////////
// Start timestamps FIF0s
////////////////////////////////////////////////////////////

localparam integer C_FIFO_DEPTH = 64; // Need to be greater than GEN_PIPE_SIZE
localparam integer C_FIFO_PTR_SIZE = $clog2(C_FIFO_DEPTH);

logic [C_FIFO_PTR_SIZE:0]       start_ts_FIFO_rd_ptr [0:C_NUM_ID-1] = '{default:-1};
logic [0:C_FIFO_DEPTH-1] [31:0] start_ts_FIFO        [0:C_NUM_ID-1] = '{default:'0};
logic [31:0]                    start_ts_FIFO_out    [0:C_NUM_ID-1] = '{default:'0};


generate
    for (genvar n = 0; n < C_NUM_ID; n++) begin

        always_ff @(posedge clk) begin

            if ((start) && (id_start == n)) begin

                start_ts_FIFO[n] <= {timestamp, start_ts_FIFO[n][0:C_FIFO_DEPTH-2]};

                if ((~stop) || (id_stop != n)) begin

                    start_ts_FIFO_rd_ptr[n] <= start_ts_FIFO_rd_ptr[n] + 1;

                end

            end else if ((stop) && (id_stop == n)) begin

                start_ts_FIFO_rd_ptr[n] <= start_ts_FIFO_rd_ptr[n] - 1;

            end

            start_ts_FIFO_out[n] <= start_ts_FIFO[n][start_ts_FIFO_rd_ptr[n]];

            // Sync reset
            if (rst) begin

                start_ts_FIFO_rd_ptr[n] <= -1;

            end

        end
    end
endgenerate

////////////////////////////////////////////////////////////
// Inputs to stat pipeline
////////////////////////////////////////////////////////////

logic                           stop_d          = '0;
logic [C_ID_WIDTH-1:0]          id_stop_d       = '0;

logic                           timestamp_valid = '0;
logic [31:0]                    timestamp_start = '0;
logic [31:0]                    timestamp_stop  = '0;

logic                           meas_valid      = '0;
logic [C_STAT_MEAS_SIZE-1:0]    meas            = '0;

always_ff @(posedge clk) begin

    if (C_USE_AXI_ID == 1) begin

        stop_d <= stop;
        if (stop) begin

            id_stop_d   <= id_stop;

        end

        timestamp_valid <= stop_d;
        if (stop_d) begin

            timestamp_stop  <= timestamp - 1;
            timestamp_start <= start_ts_FIFO_out[id_stop_d];

        end

    end else begin

        timestamp_valid <= stop;

        if (stop) begin

            timestamp_stop  <= timestamp;
            timestamp_start <= start_ts_FIFO[0][start_ts_FIFO_rd_ptr[0]];

        end

    end

    meas_valid <= timestamp_valid;
    if (timestamp_valid) begin

        meas  <= timestamp_stop - timestamp_start;

    end

    if (rst) begin

        stop_d          <= '0;
        timestamp_valid <= '0;
        meas_valid      <= '0;

    end
end

memtest_burst_stat_pipe #(
    .C_STAT_MEAS_SIZE   ( C_STAT_MEAS_SIZE  ),
    .C_STAT_TOTAL_SIZE  ( C_STAT_TOTAL_SIZE ),
    .C_STAT_INST_SIZE   ( C_STAT_INST_SIZE  )
) u_stat_pipe (
    .clk                ( clk                   ),
    .rst                ( rst                   ),
    .toggle_1sec        ( toggle_1sec           ),
    .meas_valid         ( meas_valid            ),
    .meas               ( meas                  ),
    .stat_total         ( stat_latency_total    ),
    .stat_inst          ( stat_latency_inst     ),
    .stat_min           ( stat_latency_min      ),
    .stat_max           ( stat_latency_max      )
);

endmodule : memtest_burst_latency

`default_nettype wire
