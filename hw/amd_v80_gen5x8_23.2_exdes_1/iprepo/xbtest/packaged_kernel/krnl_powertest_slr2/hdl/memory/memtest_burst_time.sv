
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

module memtest_burst_time #(
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
    input  wire  [C_ID_WIDTH-1:0]           id,
    output logic [C_STAT_TOTAL_SIZE-1:0]    stat_time_total,
    output logic [C_STAT_INST_SIZE-1:0]     stat_time_inst,
    output logic [C_STAT_MEAS_SIZE-1:0]     stat_time_min,
    output logic [C_STAT_MEAS_SIZE-1:0]     stat_time_max
);

timeunit 1ps;
timeprecision 1ps;

localparam integer C_NUM_ID = 2**(C_ID_WIDTH);

logic [31:0]                    start_ts_RAM [0:C_NUM_ID-1] = '{default:'0};

logic                           timestamp_valid = '0;
logic [31:0]                    timestamp_start = '0;
logic [31:0]                    timestamp_stop  = '0;
logic                           meas_valid      = '0;
logic [C_STAT_MEAS_SIZE-1:0]    meas            = '0;

always_ff @(posedge clk) begin

    // start and stop cannot occur at same cycle as we do not support single-beat bursts
    if (start) begin

        start_ts_RAM[id] <= timestamp;

    end

    timestamp_valid <= stop;
    if (stop) begin

        timestamp_start <= start_ts_RAM[id];
        timestamp_stop  <= timestamp + 1;

    end

    meas_valid <= timestamp_valid;
    if (timestamp_valid) begin

        meas  <= timestamp_stop - timestamp_start;

    end

    if (rst) begin

        timestamp_valid <= '0;

    end
end

memtest_burst_stat_pipe #(
    .C_STAT_MEAS_SIZE   ( C_STAT_MEAS_SIZE  ),
    .C_STAT_TOTAL_SIZE  ( C_STAT_TOTAL_SIZE ),
    .C_STAT_INST_SIZE   ( C_STAT_INST_SIZE  )
) u_stat_pipe (
    .clk                ( clk               ),
    .rst                ( rst               ),
    .toggle_1sec        ( toggle_1sec       ),
    .meas_valid         ( meas_valid        ),
    .meas               ( meas              ),
    .stat_total         ( stat_time_total   ),
    .stat_inst          ( stat_time_inst    ),
    .stat_min           ( stat_time_min     ),
    .stat_max           ( stat_time_max     )
);

endmodule : memtest_burst_time

`default_nettype wire
