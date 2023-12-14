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

#include <adf.h>
#include "../user_parameters.h"

int32  mem0[MEM_SIZE];
int32  mem1[MEM_SIZE];
int32  mem2[MEM_SIZE];
int32  mem3[MEM_SIZE];

void control (
    input_stream<uint32> * restrict sin0,
    output_stream<int32> * restrict sout0
)
{
    int32 cnt_rep=0;

    chess_protect_access int32  chess_storage(DM_bankA) * restrict pt_mem0 = (int32  chess_storage(DM_bankA) *) mem0;
    chess_protect_access int32  chess_storage(DM_bankB) * restrict pt_mem1 = (int32  chess_storage(DM_bankB) *) mem1;
    chess_protect_access int32  chess_storage(DM_bankC) * restrict pt_mem2 = (int32  chess_storage(DM_bankC) *) mem2;
    chess_protect_access int32  chess_storage(DM_bankD) * restrict pt_mem3 = (int32  chess_storage(DM_bankD) *) mem3;

    int32 spat_0 = 0;
    int32 spat_1 = -1;

    uint32 toggle_rate;

    while (1) chess_prepare_for_pipelining chess_loop_range(8,) {

        toggle_rate = readincr(sin0);
        // printf("toggle rate: %c\n", toggle_rate);

        for (uint32 i = 0; i < NUM_REP; i++) chess_prepare_for_pipelining chess_loop_range(8,) {
            // auto start=get_cycles();
            for (uint32 j = toggle_rate; j > 0; j--)
            {
                // Write stream twice for 8 cycles
                writeincr(sout0, spat_0);
                *(pt_mem0++) = cnt_rep;
                *(pt_mem1++) = cnt_rep;
                *(pt_mem2++) = cnt_rep;

                writeincr(sout0, spat_1);
                *(pt_mem0--) = cnt_rep;
                *(pt_mem1--) = cnt_rep;
                *(pt_mem2--) = cnt_rep;
            }
            for (uint32 k = 100-toggle_rate; k > 0; k--)
            {
                // Do other stuff so stream is not written for 8 cycles
                *(pt_mem0++) = cnt_rep;
                *(pt_mem1++) = cnt_rep;
                *(pt_mem2++) = cnt_rep;
                *(pt_mem3++) = cnt_rep;

                *(pt_mem0--) = cnt_rep;
                *(pt_mem1--) = cnt_rep;
                *(pt_mem2--) = cnt_rep;
                *(pt_mem3--) = cnt_rep;
            }
            // auto end=get_cycles();
            // printf("number of cycles  : %llu\n", end-start);
        }
        cnt_rep++;
    }
}
