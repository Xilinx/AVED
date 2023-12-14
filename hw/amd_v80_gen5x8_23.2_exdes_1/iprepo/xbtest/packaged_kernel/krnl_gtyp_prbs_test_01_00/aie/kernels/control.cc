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

v8int32  mem0[MEM_SIZE];
v8int32  mem1[MEM_SIZE];
v8int32  mem2[MEM_SIZE];
v8int32  mem3[MEM_SIZE];

void control (
    input_stream<uint32> * restrict sin0,
    output_stream<int32> * restrict sout0
)
{
    int32 cnt_rep=0;

    chess_protect_access v8int32  chess_storage(DM_bankA) * restrict pt_mem0 = (v8int32  chess_storage(DM_bankA) *) mem0;
    chess_protect_access v8int32  chess_storage(DM_bankB) * restrict pt_mem1 = (v8int32  chess_storage(DM_bankB) *) mem1;
    chess_protect_access v8int32  chess_storage(DM_bankC) * restrict pt_mem2 = (v8int32  chess_storage(DM_bankC) *) mem2;
    chess_protect_access v8int32  chess_storage(DM_bankD) * restrict pt_mem3 = (v8int32  chess_storage(DM_bankD) *) mem3;

    auto wrval0 = null_v8int32();
    auto wrval1 = null_v8int32();
    auto wrval2 = null_v8int32();
    auto wrval3 = null_v8int32();

    auto spat_0 = null_v4int32();
    auto spat_1 = null_v4int32();
    spat_0 = upd_elem (spat_0,0, 0);
    spat_0 = upd_elem (spat_0,1,-1);
    spat_0 = upd_elem (spat_0,2, 0);
    spat_0 = upd_elem (spat_0,3,-1);

    spat_1 = upd_elem (spat_1,0,-1);
    spat_1 = upd_elem (spat_1,1, 0);
    spat_1 = upd_elem (spat_1,2,-1);
    spat_1 = upd_elem (spat_1,3, 0);

    uint32 toggle_rate;

    while (1) chess_prepare_for_pipelining chess_loop_range(8,) {

        toggle_rate = readincr(sin0);

        for (uint32 i = 0; i < NUM_REP; i++) chess_prepare_for_pipelining chess_loop_range(8,) {
            for (uint32 j = toggle_rate; j > 0; j--)
            {
                // Write stream for 8 cycles
                writeincr_v4(sout0, spat_0);
                writeincr_v4(sout0, spat_1);
            }
            for (uint32 k = 100-toggle_rate; k > 0; k--)
            {
                // Do other stuff so stream is not written for 8 cycles
                *(pt_mem0++) = upd_elem (wrval0, 0, cnt_rep);
                *(pt_mem1++) = upd_elem (wrval1, 1, cnt_rep);
                *(pt_mem2++) = upd_elem (wrval2, 2, cnt_rep);
                *(pt_mem3++) = upd_elem (wrval3, 3, cnt_rep);

                *(pt_mem0--) = upd_elem (wrval0, 0, cnt_rep);
                *(pt_mem1--) = upd_elem (wrval1, 1, cnt_rep);
                *(pt_mem2--) = upd_elem (wrval2, 2, cnt_rep);
                *(pt_mem3--) = upd_elem (wrval3, 3, cnt_rep);
            }
        }
        cnt_rep++;
    }
}
