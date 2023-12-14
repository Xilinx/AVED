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

#include "sk_types.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "adf/adf_api/XRTConfig.h"
#include "xrt/experimental/xrt_aie.h"
#include "xrt/experimental/xrt_kernel.h"
#include "xrt/xrt_bo.h"
#include <xaiengine.h>

#define NUM_DATA          1
#define DATA_SIZE_BYTES   sizeof(uint)
#define BUFFER_SIZE_BYTES NUM_DATA*DATA_SIZE_BYTES

#ifdef __cplusplus
extern "C" {
#endif

class xrtHandles : public pscontext {
   public:
    xrt::device dhdl;
    xrtHandles(xclDeviceHandle dhdl_in, const xuid_t xclbin_uuid) : dhdl(dhdl_in) {}
};

__attribute__((visibility("default")))
xrtHandles* krnl_powertest_ps_init(xclDeviceHandle dhdl, const xuid_t xclbin_uuid) {
    xrtHandles* handles = new xrtHandles(dhdl, xclbin_uuid);

    return handles;
}

__attribute__((visibility("default")))
int krnl_powertest_ps(uint toggle_rate, uint duration, xrtHandles* handles) {
    if (duration <= 0)
    {
        return 1;
    }

    auto in_bohdl = xrt::aie::bo(handles->dhdl, BUFFER_SIZE_BYTES, 0, 0);

    uint in_buff_data[NUM_DATA];
    in_buff_data[0] = toggle_rate;
    in_bohdl.write(in_buff_data, BUFFER_SIZE_BYTES, 0);

    for (uint i=0; i<duration; i++)
    {
        in_bohdl.sync("G.in0", XCL_BO_SYNC_BO_GMIO_TO_AIE, BUFFER_SIZE_BYTES, 0);
    }

    return 0;
}

__attribute__((visibility("default")))
int krnl_powertest_ps_fini(xrtHandles* handles) {
    // delete handles; // Do not delete handles to workaround issue of PS kernel cannot not be changed
    return 0;
}

#ifdef __cplusplus
}
#endif
