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

v32int16 rdmem0[MEM_SIZE];

v16int16  wrmem0[MEM_SIZE];
v16int16  wrmem1[MEM_SIZE];
v16int16  wrmem2[MEM_SIZE];

void compute (
    input_stream<int32>  * restrict sin0
)
{
    uint32 i;

    chess_protect_access v32int16 chess_storage(DM_bankA) * restrict pt_rdmem0 = (v32int16 chess_storage(DM_bankA) *) rdmem0;

    chess_protect_access v16int16  chess_storage(DM_bankB) * restrict pt_wrmem0 = (v16int16  chess_storage(DM_bankB) *) wrmem0;
    chess_protect_access v16int16  chess_storage(DM_bankC) * restrict pt_wrmem1 = (v16int16  chess_storage(DM_bankC) *) wrmem1;
    chess_protect_access v16int16  chess_storage(DM_bankD) * restrict pt_wrmem2 = (v16int16  chess_storage(DM_bankD) *) wrmem2;

    chess_protect_access int32_t * restrict tmp0 = (int32_t *) pt_rdmem0;
    for(i = 0; i < (16*16*MEM_SIZE/32); i++) chess_prepare_for_pipelining {
        tmp0[i]   = 0xFFFFFFFF;
    }

    const int shft = 4;
    const int sign = 0;

    v16int32 xbuff0;
    v16int32 xbuff1;

    int32 data0 = readincr(sin0);
    int32 data1 = readincr(sin0);

    for (i = 0; i < 16; i++) {
        xbuff0 = upd_elem (xbuff0, i, data0);
        xbuff1 = upd_elem (xbuff1, i, data1);
    }

    v32int16 zbuff = *(pt_rdmem0);

    v16acc64 acc0_0 = undef_v16acc64();
    v16acc64 acc0_1 = undef_v16acc64();
    v16acc64 acc0_2 = undef_v16acc64();

    v16acc64 acc1_0 = undef_v16acc64();
    v16acc64 acc1_1 = undef_v16acc64();
    v16acc64 acc1_2 = undef_v16acc64();

    int sgn_x_0    = 0+0;
    int sgn_x_1    = 0+1;
    int sgn_y_0    = 0+0;
    int sgn_y_1    = 0+1;
    int zero_acc1  = 0;
    int shift16_0  = 0+0;
    int shift16_1  = 0+1;
    int sub_mul    = 0;
    int sub_acc1   = 0;

    while (1) chess_prepare_for_pipelining chess_loop_range(8,) {
        {
            for (i = 0; i < ITERATIONS; i++) chess_prepare_for_pipelining chess_loop_range(8,) {
                data0 = readincr(sin0);
                xbuff0 = upd_elem (xbuff0, 0, data0);
                xbuff0 = upd_elem (xbuff0, 1, data0);
                xbuff0 = upd_elem (xbuff0, 2, data0);
                acc0_0 = negmul_4x2_2x4_conf (xbuff0, sgn_x_0, zbuff, sgn_y_0, sub_mul);                                            *pt_wrmem0 = ssrs(acc0_0,shft,sign);
                acc0_1 = negmul_4x2_2x4_conf (xbuff0, sgn_x_1, zbuff, sgn_y_0, sub_mul);
                acc0_1 = msc_4x2_2x4_conf    (xbuff0, sgn_x_0, zbuff, sgn_y_0, acc0_1, zero_acc1, shift16_0, sub_mul, sub_acc1);    *pt_wrmem1 = ssrs(acc0_1,shft,sign);
                acc0_2 = negmul_4x2_2x4_conf (xbuff0, sgn_x_0, zbuff, sgn_y_1, sub_mul);                                            *pt_wrmem2 = ssrs(acc0_2,shft,sign);
                shift16_0+=2;


                data1 = readincr(sin0);
                xbuff1 = upd_elem (xbuff1, 0, data1);
                xbuff1 = upd_elem (xbuff1, 1, data1);
                xbuff1 = upd_elem (xbuff1, 2, data1);
                acc1_0 = negmul_4x2_2x4_conf (xbuff1, sgn_x_0, zbuff, sgn_y_0, sub_mul);                                            *pt_wrmem0 = ssrs(acc1_0,shft,sign);
                acc1_1 = negmul_4x2_2x4_conf (xbuff1, sgn_x_1, zbuff, sgn_y_0, sub_mul);
                acc1_1 = msc_4x2_2x4_conf    (xbuff1, sgn_x_0, zbuff, sgn_y_0, acc1_1, zero_acc1, shift16_1, sub_mul, sub_acc1);    *pt_wrmem1 = ssrs(acc1_1,shft,sign);
                acc1_2 = negmul_4x2_2x4_conf (xbuff1, sgn_x_0, zbuff, sgn_y_1, sub_mul);                                            *pt_wrmem2 = ssrs(acc1_2,shft,sign);
                shift16_1+=2;
            }
        }
    }
}
