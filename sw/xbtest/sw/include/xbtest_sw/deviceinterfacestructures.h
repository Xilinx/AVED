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

#ifndef _DEVICEINTERFACESTRUCTURES_H
#define _DEVICEINTERFACESTRUCTURES_H

#include <fcntl.h>

#ifdef USE_XRT

    #define CL_HPP_CL_1_2_DEFAULT_BUILD
    #define CL_HPP_TARGET_OPENCL_VERSION                                120
    #define CL_HPP_MINIMUM_OPENCL_VERSION                               120
    #define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
    #define CL_USE_DEPRECATED_OPENCL_1_2_APIS

    // OpenCL includes
    #if __has_include(<CL/opencl.hpp>)
        #include <CL/opencl.hpp>
    #else
        #include <CL/cl2.hpp>
    #endif

    // XRT includes
    #include <xrt/xrt_device.h>
    #include <xrt/xrt_kernel.h>

#endif

namespace xbtest
{

struct Build_Info {
        std::string cu_name;
        uint        major_version;
        uint        minor_version;
        uint        build_version;
        uint        component_id;
        uint        scratch_pad;
        uint        slr;
        uint        throttle_mode;
        uint        use_aie;
        uint        mem_num_channels;
        uint        mem_type;
        uint        mem_cu_inst;
        int         mem_ch_idx;
        uint        mem_axi_datasize_divider;
        uint        gt_index;
        uint        gt_enable;
        uint        gt_rate;
        uint        gt_ip_sel;
        uint        gt_num_lane;
        uint        gt_num_gt;
        uint        gt_type;
        uint        gt_rsfec_en;
        uint        num_slice;
        uint        num_dsp;
        uint        num_bram;
        uint        num_uram;
        uint        rst_detection;
        uint        clock0_freq;
        uint        clock1_freq;
};

// m00_axi buffer pointers
class Cl_Buffer_Base {
    public:
        uint        mem_data_idx;
        size_t      buffer_size;
        size_t      available_size;
};

class Cl_Buffer_t : public Cl_Buffer_Base {
    public:
        #if defined(USE_AMI) || defined(USE_NO_DRIVER)
        // peek-poke
        uint8_t     peekpoke_bar_idx;   // PCIe BAR index of PLRAM
        uint64_t    peekpoke_addr;      // Address offset in PCIe BAR of PLRAM
        uint64_t    peekpoke_range;     // Address range in PCIe BAR of PLRAM, not used currently
        uint64_t    axi_addr;
        #endif
        //USE_XRT
        #ifdef USE_XRT
        cl::Buffer  cl_buffer;
        #endif
};

// xbtest HW IPs
class Cl_Compute_Unit_Base {
    public:
        std::string             name;
        size_t                  cl_m00_axi_buffer_size;
        size_t                  cl_m00_axi_buffer_origin;
        uint                    cl_m00_axi_mem_data_idx;
        uint                    slr; // power
        uint                    gt_index; // gt_mac/lpbk/prbs
        std::vector<uint>       gtf_indexes; // gtf_prbs
        std::vector<uint>       gtm_indexes; // gtm_prbs
        std::vector<uint>       gtyp_indexes; // gtyp_prbs
        bool                    dna_read;
        Build_Info              build_info;
        std::vector<Build_Info> ch_build_info; // memory/gtf/gtm/gtyp
        std::mutex              * cu_mtx;
};

class Cl_Compute_Unit_t : public Cl_Compute_Unit_Base {
    public:
        #ifdef USE_XRT
        mutable cl::Kernel              cl_kernel;//MODIFIED
        cl::Buffer              cl_m00_axi_buffer;
        #endif

        #if defined(USE_AMI) || defined(USE_NO_DRIVER)
        uint8_t                 cu_peekpoke_bar_idx;    // PCIe BAR index of xbtest HW IP, used to peek poke xbtest HW IP
        uint64_t                cu_peekpoke_addr;       // Address offset in PCIe BAR of xbtest HW IP, used to peek poke xbtest HW IP
        uint64_t                cu_peekpoke_range;      // Address range in PCIe BAR of xbtest HW IP, not used currently
        uint64_t                cu_axi_addr;            // Address offset in AXI of xbtest HW IP, used to peek poke xbtest HW IP. UNUSED FOR NOW.

        uint8_t                 cl_m00_peekpoke_bar_idx; // PCIe BAR index of PLRAM associated to xbtest HW IP, used to peek poke PLRAM associated to xbtest HW IP
        uint64_t                cl_m00_peekpoke_addr;    // Address offset in PCIe BAR of PLRAM associated to xbtest HW IP, used to peek poke PLRAM associated to xbtest HW IP
        uint64_t                cl_m00_axi_addr;         // Address offset in AXI      of PLRAM associated to xbtest HW IP, used to set xbtest HW IP argument with PLRAM address xbtest HW IP will write to.
        #endif
};

} // namespace

#endif /* _DEVICEINTERFACESTRUCTURES_H */