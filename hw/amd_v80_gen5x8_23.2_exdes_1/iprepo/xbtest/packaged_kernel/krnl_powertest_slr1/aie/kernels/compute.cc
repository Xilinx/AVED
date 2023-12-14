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

v16int16 rdmem0[MEM_SIZE];

v8int32  wrmem0[MEM_SIZE];
v8int32  wrmem1[MEM_SIZE];
v8int32  wrmem2[MEM_SIZE];

void compute (
    input_stream<int32>  * restrict sin0,
    output_stream<int32> * restrict sout0
)
{
    chess_protect_access v16int16 chess_storage(DM_bankA) * restrict pt_rdmem0 = (v16int16 chess_storage(DM_bankA) *) rdmem0;

    chess_protect_access v8int32  chess_storage(DM_bankB) * restrict pt_wrmem0 = (v8int32  chess_storage(DM_bankB) *) wrmem0;
    chess_protect_access v8int32  chess_storage(DM_bankC) * restrict pt_wrmem1 = (v8int32  chess_storage(DM_bankC) *) wrmem1;
    chess_protect_access v8int32  chess_storage(DM_bankD) * restrict pt_wrmem2 = (v8int32  chess_storage(DM_bankD) *) wrmem2;

    chess_protect_access int32_t * restrict tmp0 = (int32_t *) pt_rdmem0;
    for(int32 i = 0; i < (16*16*MEM_SIZE/32); i++) chess_prepare_for_pipelining {
        tmp0[i]   = 0xFFFFFFFF;
    }

    int                   xstart0     = 0+0;
    const int             xstart1     = 0+1;
    const int             xstart2     = 0+2;
    const unsigned int    xoffsets    = 0x76543210;
    int                   ystart0     = 8+0;
    const int             ystart1     = 8+1;
    const int             ystart2     = 8+2;
    const int             mtap        = 16;
    const int             zstart      = 0;
    const unsigned int    zoffsets    = 0x76543210;
    const int             zstep       = 8;
    const int             shft        = 4;

    v4int32  ibuff0 = readincr_v4(sin0);
    v4int32  ibuff1 = readincr_v4(sin0);
    v16int32 xbuff0 = concat(ibuff0, ibuff0, ibuff0, ibuff0);
    v16int32 xbuff1 = concat(ibuff1, ibuff1, ibuff1, ibuff1);

    v16int16 zbuff = *(pt_rdmem0);

    v8acc80 acc0_0 = undef_v8acc80();
    v8acc80 acc0_1 = undef_v8acc80();
    v8acc80 acc0_2 = undef_v8acc80();

    v8acc80 acc1_0 = undef_v8acc80();
    v8acc80 acc1_1 = undef_v8acc80();
    v8acc80 acc1_2 = undef_v8acc80();

    while (1) chess_prepare_for_pipelining chess_loop_range(8,) {
        for (int32 i = 0; i < ITERATIONS; i++) chess_prepare_for_pipelining chess_loop_range(8,) {
            xbuff0 = upd_v (xbuff0, 0, readincr_v4(sin0));
            acc0_0 = lnegmul8_sym_ct(xbuff0, xstart1, xoffsets, ystart1, mtap, zbuff, zstart, zoffsets, zstep);      *pt_wrmem0 = srs(acc0_0,shft);
            acc0_1 = lnegmul8_sym_ct     (xbuff0, xstart2, xoffsets, ystart2, mtap, zbuff, zstart, zoffsets, zstep);
            acc0_1 = lmsc8_sym_ct(acc0_1, xbuff0, xstart2, xoffsets, ystart2, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem1 = srs(acc0_1,shft);
            acc0_2 = lnegmul8_sym_ct     (xbuff0, xstart0, xoffsets, ystart0, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem2 = srs(acc0_2,shft);
            xstart0+=2; ystart0+=2; writeincr_v4(sout0, srs(ext_lo(acc0_2),shft));

            //////////////////////////

            xbuff1 = upd_v (xbuff1, 0, readincr_v4(sin0));
            acc1_0 = lnegmul8_sym_ct(xbuff1, xstart2, xoffsets, ystart2, mtap, zbuff, zstart, zoffsets, zstep);      *pt_wrmem0 = srs(acc1_0,shft);
            acc1_1 = lnegmul8_sym_ct     (xbuff1, xstart1, xoffsets, ystart1, mtap, zbuff, zstart, zoffsets, zstep);
            acc1_1 = lmsc8_sym_ct(acc1_1, xbuff1, xstart1, xoffsets, ystart1, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem1 = srs(acc1_1,shft);
            acc1_2 = lnegmul8_sym_ct     (xbuff1, xstart0, xoffsets, ystart0, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem2 = srs(acc1_2,shft);
            xstart0+=2; ystart0+=2; writeincr_v4(sout0, srs(ext_lo(acc1_2),shft));
        }
    }
}

void compute_end (
    input_stream<int32>  * restrict sin0
)
{
    chess_protect_access v16int16 chess_storage(DM_bankA) * restrict pt_rdmem0 = (v16int16 chess_storage(DM_bankA) *) rdmem0;

    chess_protect_access v8int32  chess_storage(DM_bankB) * restrict pt_wrmem0 = (v8int32  chess_storage(DM_bankB) *) wrmem0;
    chess_protect_access v8int32  chess_storage(DM_bankC) * restrict pt_wrmem1 = (v8int32  chess_storage(DM_bankC) *) wrmem1;
    chess_protect_access v8int32  chess_storage(DM_bankD) * restrict pt_wrmem2 = (v8int32  chess_storage(DM_bankD) *) wrmem2;

    chess_protect_access int32_t * restrict tmp0 = (int32_t *) pt_rdmem0;
    for(int32 i = 0; i < (16*16*MEM_SIZE/32); i++) chess_prepare_for_pipelining {
        tmp0[i]   = 0xFFFFFFFF;
    }

    int                   xstart0     = 0+0;
    const int             xstart1     = 0+1;
    const int             xstart2     = 0+2;
    const unsigned int    xoffsets    = 0x76543210;
    int                   ystart0     = 8+0;
    const int             ystart1     = 8+1;
    const int             ystart2     = 8+2;
    const int             mtap        = 16;
    const int             zstart      = 0;
    const unsigned int    zoffsets    = 0x76543210;
    const int             zstep       = 8;
    const int             shft        = 4;

    v4int32  ibuff0 = readincr_v4(sin0);
    v4int32  ibuff1 = readincr_v4(sin0);
    v16int32 xbuff0 = concat(ibuff0, ibuff0, ibuff0, ibuff0);
    v16int32 xbuff1 = concat(ibuff1, ibuff1, ibuff1, ibuff1);

    v16int16 zbuff = *(pt_rdmem0);

    v8acc80 acc0_0 = undef_v8acc80();
    v8acc80 acc0_1 = undef_v8acc80();
    v8acc80 acc0_2 = undef_v8acc80();

    v8acc80 acc1_0 = undef_v8acc80();
    v8acc80 acc1_1 = undef_v8acc80();
    v8acc80 acc1_2 = undef_v8acc80();

    while (1) chess_prepare_for_pipelining chess_loop_range(8,) {
        for (int32 i = 0; i < ITERATIONS; i++) chess_prepare_for_pipelining chess_loop_range(8,) {
            xbuff0 = upd_v (xbuff0, 0, readincr_v4(sin0));
            acc0_0 = lnegmul8_sym_ct(xbuff0, xstart1, xoffsets, ystart1, mtap, zbuff, zstart, zoffsets, zstep);      *pt_wrmem0 = srs(acc0_0,shft);
            acc0_1 = lnegmul8_sym_ct     (xbuff0, xstart2, xoffsets, ystart2, mtap, zbuff, zstart, zoffsets, zstep);
            acc0_1 = lmsc8_sym_ct(acc0_1, xbuff0, xstart2, xoffsets, ystart2, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem1 = srs(acc0_1,shft);
            acc0_2 = lnegmul8_sym_ct     (xbuff0, xstart0, xoffsets, ystart0, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem2 = srs(acc0_2,shft);
            xstart0+=2; ystart0+=2;

            xbuff1 = upd_v (xbuff1, 0, readincr_v4(sin0));
            acc1_0 = lnegmul8_sym_ct(xbuff1, xstart2, xoffsets, ystart2, mtap, zbuff, zstart, zoffsets, zstep);      *pt_wrmem0 = srs(acc1_0,shft);
            acc1_1 = lnegmul8_sym_ct     (xbuff1, xstart1, xoffsets, ystart1, mtap, zbuff, zstart, zoffsets, zstep);
            acc1_1 = lmsc8_sym_ct(acc1_1, xbuff1, xstart1, xoffsets, ystart1, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem1 = srs(acc1_1,shft);
            acc1_2 = lnegmul8_sym_ct     (xbuff1, xstart0, xoffsets, ystart0, mtap, zbuff, zstart, zoffsets, zstep); *pt_wrmem2 = srs(acc1_2,shft);
            xstart0+=2; ystart0+=2;
        }
    }
}
