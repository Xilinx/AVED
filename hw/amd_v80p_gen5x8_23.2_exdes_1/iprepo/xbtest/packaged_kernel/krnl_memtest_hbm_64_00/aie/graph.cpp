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

#include "graph.h"

using namespace adf;

TopGraph G;

#if defined(__AIESIM__) || defined(__X86SIM__)
int main()
{
    return_code ret;
    G.init();
#if (AIE_CONTROL == AIE_CONTROL_PS)
    uint* dinArray=(uint*)GMIO::malloc(GMIO_SIZE_BYTES);

    dinArray[0]=0;
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    dinArray[0]=10;
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    dinArray[0]=20;
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);

    dinArray[0]=100;
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);

    dinArray[0]=50;
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
    G.in0.gm2aie(dinArray,GMIO_SIZE_BYTES);
#endif
    ret = G.run(10);
    if (ret != ok) {
        printf("Run failed\n");
        return ret;
    }
#if (AIE_CONTROL == AIE_CONTROL_PS)
    GMIO::free(dinArray);
#endif
    ret = G.end();
    if (ret != ok)
    {
        printf("End failed\n");
        return ret;
    }
    return 0;
}
#endif
