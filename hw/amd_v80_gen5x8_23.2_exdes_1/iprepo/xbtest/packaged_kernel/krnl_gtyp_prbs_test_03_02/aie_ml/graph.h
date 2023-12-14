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

#pragma once

#include "adf.h"
#include "kernels.h"

using namespace adf;

#if ((AIE_CONTROL == AIE_CONTROL_PL) && (AIE_STATUS == AIE_STATUS_NONE))
class TopGraph : public graph
{
    public:
        input_plio  in0;
        kernel      krnl [NUMCORES];

        TopGraph()
        {
            in0   = input_plio::create("in0",  plio_32_bits, "data/input0.txt",  PL_FREQ_MHZ);

            for (uint i=0; i<NUMCORES; i++)
            {
                krnl[i] = kernel::create(compute);
                runtime<ratio>(krnl[i]) = 1.0;
                source(krnl[i]) = "compute.cc";
            }

            for (uint i=0; i<NUMCORES; i++)
            {
                connect<stream> n0 (in0.out[0], krnl[i].in[0]);
                fifo_depth(n0) = FIFO_DEPTH;
            }
        }

};
#endif

#if ((AIE_CONTROL == AIE_CONTROL_PS) && (AIE_STATUS == AIE_STATUS_NONE))
class TopGraph : public graph
{
    public:
        input_gmio  in0;
        kernel      krnl [NUMCORES];

        TopGraph()
        {
            in0     =  input_gmio::create("in0", 64, 100);

            krnl[0] = kernel::create(control);
            runtime<ratio>(krnl[0]) = 1.0;
            source(krnl[0]) = "control.cc";

            for (uint i=1; i<NUMCORES; i++)
            {
                krnl[i] = kernel::create(compute);
                runtime<ratio>(krnl[i]) = 1.0;
                source(krnl[i]) = "compute.cc";
            }

            connect<stream> nin  (in0.out[0], krnl[0].in[0]);
            fifo_depth(nin)  = FIFO_DEPTH;

            for (uint i=1; i<NUMCORES; i++)
            {
                connect<stream> n0 (krnl[0].out[0], krnl[i].in[0]);
                fifo_depth(n0) = FIFO_DEPTH;
            }
        }

};
#endif

#if ((AIE_CONTROL == AIE_CONTROL_PS) && (AIE_STATUS == AIE_STATUS_PL))
class TopGraph : public graph
{
    public:
        input_gmio  in0;
        output_plio out0;
        kernel      krnl [NUMCORES];

        TopGraph()
        {
            in0  =  input_gmio::create("in0", 64, 100);
            out0 = output_plio::create("out0", plio_32_bits, "data/output0.txt", PL_FREQ_MHZ);

            krnl[0] = kernel::create(control);
            runtime<ratio>(krnl[0]) = 1.0;
            source(krnl[0]) = "control.cc";

            for (uint i=1; i<NUMCORES; i++)
            {
                krnl[i] = kernel::create(compute);
                runtime<ratio>(krnl[i]) = 1.0;
                source(krnl[i]) = "compute.cc";
            }

            connect<stream> nin  (in0.out[0], krnl[0].in[0]);
            fifo_depth(nin)  = FIFO_DEPTH;

            connect<stream> nout (krnl[0].out[0], out0.in[0]);
            fifo_depth(nout) = FIFO_DEPTH;

            for (uint i=1; i<NUMCORES; i++)
            {
                connect<stream> n0 (krnl[0].out[0], krnl[i].in[0]);
                fifo_depth(n0) = FIFO_DEPTH;
            }
        }

};
#endif
