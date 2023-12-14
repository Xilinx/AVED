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

#if defined(USE_AMI) || defined(USE_NO_DRIVER)

#include "deviceinterface_peekpoke.h"
#include "clpackage.h"

namespace xbtest
{

DeviceInterface_PeekPoke::DeviceInterface_PeekPoke(
    Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target
) : DeviceInterface_Base( log, xbtest_sw_config, xbtest_sw_config_p2p, abort, p2p_target ) {}

//use origianl
DeviceInterface_PeekPoke::~DeviceInterface_PeekPoke() = default;

bool DeviceInterface_PeekPoke::SetCuScalarArg ( const Cl_Compute_Unit_t & cl_compute_unit, const uint32_t & arg_idx, uint32_t & arg_data )
{
    std::string msg_header = cl_compute_unit.name + ": ";
    uint64_t peekpoke_addr;
    uint32_t peekpoke_data[1];
    uint64_t scalar_addr = SCALAR_ARG_ADDR[arg_idx];

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Setting scalar argument " + std::to_string(arg_idx) + " at address 0x" + NumToStrHex(scalar_addr) + " to 0x" + NumToStrHex(arg_data)});

    peekpoke_addr       = cl_compute_unit.cu_peekpoke_addr + scalar_addr;
    peekpoke_data[0]    = arg_data;
    if (MemWrite(cl_compute_unit.name, cl_compute_unit.cu_peekpoke_bar_idx, peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"write", "to", cl_compute_unit.name});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::ExecuteCu ( const Cl_Compute_Unit_t & cl_compute_unit )
{
    uint64_t peekpoke_addr;
    uint32_t peekpoke_data[1];

    peekpoke_addr       = cl_compute_unit.cu_peekpoke_addr+CU_AP_CTRL_ADDR;
    peekpoke_data[0]    = CU_AP_CTRL_START;
    if (MemWrite(cl_compute_unit.name, cl_compute_unit.cu_peekpoke_bar_idx, peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"write", "to", cl_compute_unit.name});
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::WaitCuIdle ( const Cl_Compute_Unit_t & cl_compute_unit )
{
    uint64_t peekpoke_addr;
    uint32_t peekpoke_data[1];

    peekpoke_addr = cl_compute_unit.cu_peekpoke_addr+CU_AP_CTRL_ADDR;
    // Read xbtest HW IP status until the IP is IDLE and DONE status was set.
    do
    {
        if (MemRead(cl_compute_unit.name, cl_compute_unit.cu_peekpoke_bar_idx, peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
        {
            LogMessage(MSG_ITF_164, {"read", "from", cl_compute_unit.name});
            return RET_FAILURE;
        }

        if (((peekpoke_data[0] & CU_AP_CTRL_DONE) == CU_AP_CTRL_DONE) && ((peekpoke_data[0] & CU_AP_CTRL_IDLE) == CU_AP_CTRL_IDLE))
        {
            return RET_SUCCESS;
        }
    } while (!(*(this->m_abort)));

    LogMessage(MSG_DEBUG_OPENCL, {cl_compute_unit.name + ": \t - Stop waiting for xbtest HW IP to complete as abort was received"});
    return RET_FAILURE;
}

bool DeviceInterface_PeekPoke::WriteGtmacCuCmd ( const uint & cu_idx, const uint & value )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    //Rename variables to remove this
    const Cl_Compute_Unit_t cl_compute_unit = this->m_cl_gt_mac_compute_units[cu_idx];

    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Write GT_MAC xbtest HW IP command " + std::to_string(value)});
    uint32_t arg_data;

    // Set argument 0 with GT_MAC xbtest HW IP command
    arg_data = value & 0xffffffff;
    if (SetCuScalarArg(cl_compute_unit, 0, arg_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_162, {cl_compute_unit.name, "scalar", "0"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    if (ExecuteCu(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    if (WaitCuIdle(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading Memory xbtest HW IP status done."});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::WriteGtmacCuTrafficCfg ( const uint & cu_idx, uint32_t * traffic_cfg )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    //to be changed

    std::string msg_header = this->m_cl_gt_mac_compute_units[cu_idx].name + ": ";
    auto size = this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_axi_buffer_size;
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Writing GT_MAC xbtest HW IP traffic configuration (" + std::to_string(size) + " bytes)"});

    // Write PLRAM
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Writing " + std::to_string(size) + " bytes in PLRAM at address 0x" + NumToStrHex(this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_peekpoke_addr)});
    if (MemWrite(this->m_cl_gt_mac_compute_units[cu_idx].name, this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_peekpoke_bar_idx, this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_peekpoke_addr, size/sizeof(uint32_t), traffic_cfg) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"write", "to", this->m_cl_gt_mac_compute_units[cu_idx].name + " configuration and status memory"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Write GT_MAC xbtest HW IP traffic configuration done."});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::ReadGtmacCuTrafficCfg ( const uint & cu_idx, uint32_t * read_buffer )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::string msg_header = this->m_cl_gt_mac_compute_units[cu_idx].name + ": ";
    auto size = GT_MAC_STATUS_SIZE * (uint64_t)(sizeof(uint32_t));
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading GT_MAC xbtest HW IP traffic configuration (" + std::to_string(size) + " bytes)"});

    // Read PLRAM
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading " + std::to_string(size) + " bytes in PLRAM at address 0x" + NumToStrHex(this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_peekpoke_addr)});

    if (MemRead(this->m_cl_gt_mac_compute_units[cu_idx].name, this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_peekpoke_bar_idx, this->m_cl_gt_mac_compute_units[cu_idx].cl_m00_peekpoke_addr, size/sizeof(uint32_t), read_buffer) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"read", "from", this->m_cl_gt_mac_compute_units[cu_idx].name + " configuration and status memory"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading GT_MAC xbtest HW IP traffic configuration done."});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::SetCuAxiPtrArg ( const Cl_Compute_Unit_t & cl_compute_unit, const uint32_t & axi_idx, const uint64_t & axi_ptr_addr )
{
    std::string msg_header = cl_compute_unit.name + ": ";
    uint64_t peekpoke_addr;
    uint32_t peekpoke_data[1];

    uint64_t axi_ptr_addr_0     = (axi_ptr_addr >> 0) & 0xFFFFFFFF;
    uint64_t axi_ptr_addr_1     = (axi_ptr_addr >> 32) & 0xFFFFFFFF;
    uint64_t axi_ptr_arg_addr_0 = AXI_PTR0_ARG_ADDR[axi_idx][0];
    uint64_t axi_ptr_arg_addr_1 = AXI_PTR0_ARG_ADDR[axi_idx][1];

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Setting AXI " + std::to_string(axi_idx) + " ptr argument at addresses: 0x" + NumToStrHex(axi_ptr_arg_addr_0) + " to 0x" + NumToStrHex(axi_ptr_addr_0) + " and 0x" + NumToStrHex(axi_ptr_arg_addr_1) + " to 0x" + NumToStrHex(axi_ptr_addr_1)});

    peekpoke_addr       = cl_compute_unit.cu_peekpoke_addr+axi_ptr_arg_addr_0;
    peekpoke_data[0]    = axi_ptr_addr_0;

    if (MemWrite(cl_compute_unit.name, cl_compute_unit.cu_peekpoke_bar_idx, peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"write", "to", cl_compute_unit.name});
        return RET_FAILURE;
    }

    peekpoke_addr       = cl_compute_unit.cu_peekpoke_addr+axi_ptr_arg_addr_1;
    peekpoke_data[0]    = axi_ptr_addr_1;

    if (MemWrite(cl_compute_unit.name, cl_compute_unit.cu_peekpoke_bar_idx, peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"write", "to", cl_compute_unit.name});
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::ConfigureM00AxiBuffers( std::vector<Cl_Compute_Unit_t> & cl_compute_units ) {
    for (auto & cl_compute_unit : cl_compute_units)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Configuring m00_axi buffer for xbtest HW IP " + cl_compute_unit.name + ": size = " + std::to_string(cl_compute_unit.cl_m00_axi_buffer_size) + " Bytes / memory data index = " + std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx)});

        auto found = false;
        for (auto & cl_m00_axi_buffer : this->cl_m00_axi_buffers)
        {
            if (cl_m00_axi_buffer.mem_data_idx == cl_compute_unit.cl_m00_axi_mem_data_idx)
            {
                cl_compute_unit.cl_m00_axi_buffer_origin = cl_m00_axi_buffer.buffer_size;

                // To perform peek-poke, AMI requires address offset of the PLRAM inside PCIe BAR and PCIe BAR index
                cl_compute_unit.cl_m00_peekpoke_bar_idx = cl_m00_axi_buffer.peekpoke_bar_idx;
                cl_compute_unit.cl_m00_peekpoke_addr    = cl_m00_axi_buffer.peekpoke_addr + cl_compute_unit.cl_m00_axi_buffer_origin;

                // xbtest HW IP needs PLRAM AXI address
                cl_compute_unit.cl_m00_axi_addr         = cl_m00_axi_buffer.axi_addr      + cl_compute_unit.cl_m00_axi_buffer_origin;

                cl_m00_axi_buffer.buffer_size += cl_compute_unit.cl_m00_axi_buffer_size;

                found = true;
                break;
            }
        }
        if (!found)
        {
            Cl_Buffer_t cl_m00_axi_buffer;
            cl_m00_axi_buffer.mem_data_idx  = cl_compute_unit.cl_m00_axi_mem_data_idx;
            cl_m00_axi_buffer.buffer_size   = cl_compute_unit.cl_m00_axi_buffer_size;

            // No memory information available when running with AMI or without driver so get memory size from metadata (build time)
            Xclbin_Memory_Data_t plram_memory_data;
            if (this->m_xbtest_sw_config->GetInMemTopology(cl_m00_axi_buffer.mem_data_idx, plram_memory_data, true) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            cl_m00_axi_buffer.available_size = plram_memory_data.size_bytes;

            std::string ep_name = "cfg_stat_mem_slr" + std::to_string(cl_compute_unit.slr) + "_ctrl";
            if (GetEndPointConfig(ep_name, cl_m00_axi_buffer.peekpoke_range, cl_m00_axi_buffer.peekpoke_addr, cl_m00_axi_buffer.peekpoke_bar_idx, cl_m00_axi_buffer.axi_addr) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            cl_m00_axi_buffer.peekpoke_addr = SetPeekPokeAddr(cl_m00_axi_buffer.peekpoke_addr, cl_m00_axi_buffer.peekpoke_bar_idx);

            this->cl_m00_axi_buffers.emplace_back(cl_m00_axi_buffer);

            cl_compute_unit.cl_m00_axi_buffer_origin = 0;

            // To perform peek-poke, AMI requires address offset of the PLRAM inside PCIe BAR and PCIe BAR index
            cl_compute_unit.cl_m00_peekpoke_bar_idx = cl_m00_axi_buffer.peekpoke_bar_idx;
            cl_compute_unit.cl_m00_peekpoke_addr    = cl_m00_axi_buffer.peekpoke_addr + cl_compute_unit.cl_m00_axi_buffer_origin;

            // xbtest HW IP needs PLRAM AXI address
            cl_compute_unit.cl_m00_axi_addr         = cl_m00_axi_buffer.axi_addr      + cl_compute_unit.cl_m00_axi_buffer_origin;
        }
        LogMessage(MSG_DEBUG_SETUP, {"Got " + cl_compute_unit.name + " xbtest HW IP m00_axi buffer:" });
        LogMessage(MSG_DEBUG_SETUP, {"\t- Peek-poke address   = 0x" + NumToStrHex(cl_compute_unit.cl_m00_peekpoke_addr)});
        LogMessage(MSG_DEBUG_SETUP, {"\t- Peek-poke BAR index = " + std::to_string(cl_compute_unit.cl_m00_peekpoke_bar_idx)});
        LogMessage(MSG_DEBUG_SETUP, {"\t- AXI address         = 0x" + NumToStrHex(cl_compute_unit.cl_m00_axi_addr)});
    }
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::SetCuM00AxiPtrArgument( std::vector<Cl_Compute_Unit_t> & cl_compute_units ) {
    for (auto & cl_compute_unit : cl_compute_units)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Setting xbtest HW IP " + cl_compute_unit.name + " m00_axi argument"});
        if (SetCuAxiPtrArg(cl_compute_unit, 0, cl_compute_unit.cl_m00_axi_addr) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_162, {cl_compute_unit.name, "AXI pointer", "0"});
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::CreateCuMxxAxiBuffers( std::vector<Cl_Compute_Unit_t> & cl_compute_units ) {
    for (auto & cl_compute_unit : cl_compute_units)
    {
        int  num_channels   = this->m_xbtest_sw_config->GetMemCUNumChannels(cl_compute_unit.name);
        auto memory_target  = this->m_xbtest_sw_config->GetMemCUTarget(cl_compute_unit.name);

        LogMessage(MSG_DEBUG_SETUP, {"Setting " + std::to_string(num_channels) + " arguments of Memory xbtest HW IP: " + cl_compute_unit.name});

        // Case 1: Single-channel different tag. Eg DDR
        //      - Each xbtest HW IP is connected to different memory bank
        //      - Create 1 small buffer to set xbtest HW IP base address
        // Case 2: Multi-channel one different tag. E.g HBM 32 channels: 1 PC per channel
        //      - Each channel is connected to a different memory bank
        //      - Create 1 small buffer to set xbtest HW IP base address
        // Case 3: Multi-channel same tag: E.g PS_DDR
        //      - Each channel is connected to same memory bank
        //      - Create 1 small buffer to set xbtest HW IP base address
        //      - Create extra buffers for the remaining memory size available for the channel (actual_size), so all channel address is set at correct address offset
        // Case 4: Multi-channel multiple different tag. E.g HBM 16 channels: 2 PC per channel
        //      - Each channel is connected to multiple memory banks (merged)
        //      - Create 1 small buffer to set xbtest HW IP base address in first tag without use the memory data indexes of the banks

        for (int ch_idx = 0; ch_idx < num_channels; ch_idx++)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t - Setting argument of Memory xbtest HW IP channel: " + std::to_string(ch_idx)});
            // For AMI, correct Mxx_AXI base address already calculated by xbtest SW config.
            auto cu_connection_base_address = this->m_xbtest_sw_config->GetCuConnectionActualBaseAddress(cl_compute_unit.name, ch_idx);

            if (SetCuAxiPtrArg(cl_compute_unit, ch_idx+1, cu_connection_base_address) == RET_FAILURE)
            {
                LogMessage(MSG_ITF_162, {cl_compute_unit.name, "AXI pointer", std::to_string(ch_idx+1)});
                return RET_FAILURE;
            }
         }
    }
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::ReadComputeUnit ( Cl_Compute_Unit_t & cl_compute_unit, const uint & reg_address, uint & data )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading xbtest HW IP register 0x" + NumToStrHex(reg_address)});
    uint32_t arg_data;

    // Set argument 0 with register address and read flag
    arg_data   = (reg_address << 4) & 0xfffffff0;
    arg_data  |= 0x1; // set bit for read command
    if (SetCuScalarArg(cl_compute_unit, 0, arg_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_162, {cl_compute_unit.name, "scalar", "0"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    if (ExecuteCu(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    if (WaitCuIdle(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    // Read PLRAM
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading 4 bytes in PLRAM at address 0x" + NumToStrHex(cl_compute_unit.cl_m00_peekpoke_addr)});
    uint32_t peekpoke_data[1];

    if (MemRead(cl_compute_unit.name, cl_compute_unit.cl_m00_peekpoke_bar_idx, cl_compute_unit.cl_m00_peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"read", "from", cl_compute_unit.name + " configuration and status memory"});
        return RET_FAILURE;
    }

    data = peekpoke_data[0];
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading xbtest HW IP register 0x" + NumToStrHex<uint32_t>(reg_address) + " done. Data = 0x" + NumToStrHex<uint32_t>(data)});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::ReadMemoryCuStatusInner (Cl_Compute_Unit_t & cl_compute_unit, uint32_t * read_buffer, const uint64_t & size )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading Memory xbtest HW IP status (" + std::to_string(size) + " bytes)"});
    uint32_t arg_data;

    // Set argument 0 with read status flag
    arg_data = 0x3;
    if (SetCuScalarArg(cl_compute_unit, 0, arg_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_162, {cl_compute_unit.name, "scalar", "0"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    if (ExecuteCu(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    if (WaitCuIdle(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    // Read PLRAM
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading " + std::to_string(size) + " bytes in PLRAM at address 0x" + NumToStrHex(cl_compute_unit.cl_m00_peekpoke_addr)});

    if (MemRead(cl_compute_unit.name, cl_compute_unit.cl_m00_peekpoke_bar_idx, cl_compute_unit.cl_m00_peekpoke_addr, size/sizeof(uint32_t), read_buffer) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"read", "from", cl_compute_unit.name + " configuration and status memory"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading Memory xbtest HW IP status done."});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::ReadMultiGtStatus (Cl_Compute_Unit_t & cl_compute_unit, const uint & core_address, uint32_t * read_buffer, const uint64_t & size )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading Multi GT xbtest HW IP status (" + std::to_string(size) + " bytes)"});
    uint32_t arg_data;

    // Set argument 0 with core address
    arg_data   = (core_address << 4) & 0xfffffff0;
    arg_data  |= 0x3; // set bit for multi read command
    if (SetCuScalarArg(cl_compute_unit, 0, arg_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_162, {cl_compute_unit.name, "scalar", "0"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    if (ExecuteCu(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    if (WaitCuIdle(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    // Read PLRAM
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading " + std::to_string(size) + " bytes in PLRAM at address 0x" + NumToStrHex(cl_compute_unit.cl_m00_peekpoke_addr)});

    if (MemRead(cl_compute_unit.name, cl_compute_unit.cl_m00_peekpoke_bar_idx, cl_compute_unit.cl_m00_peekpoke_addr, size/sizeof(uint32_t), read_buffer) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"read", "from", cl_compute_unit.name + " configuration and status Multi GT"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Reading Multi GT xbtest HW IP status done."});
    return RET_SUCCESS;
}


bool DeviceInterface_PeekPoke::WriteComputeUnit ( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, const uint & value )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Writing xbtest HW IP register 0x" + NumToStrHex(address) + " to 0x" + NumToStrHex(value)});
    uint32_t arg_data;

    // Set argument 0 with register address and write flag
    arg_data   = (address << 4) & 0xfffffff0;
    if (SetCuScalarArg(cl_compute_unit, 0, arg_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_162, {cl_compute_unit.name, "scalar", "0"});
        return RET_FAILURE;
    }

    // Set argument 1 with register value
    arg_data = value;
    if (SetCuScalarArg(cl_compute_unit, 1, arg_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_162, {cl_compute_unit.name, "scalar", "1"});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    if (ExecuteCu(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    if (WaitCuIdle(cl_compute_unit) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_163, {cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Writing xbtest HW IP register 0x" + NumToStrHex(address) + " done"});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::GetEndPointConfig( const std::string & ep_name, uint64_t & ep_range, uint64_t & ep_peekpoke_addr, uint8_t & ep_peekpoke_bar_idx, uint64_t & ep_axi_addr )
{
    PCIe_Bar_Ep_t ep;
    PCIe_Bar_Ep_t bar;

    if (this->m_xbtest_sw_config->GetPcieEp(ep_name, ep) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_171, {ep_name});
        return RET_FAILURE;
    }

    if (this->m_xbtest_sw_config->GetPcieBar(ep.pcie_physical_function, ep.pcie_base_address_register, bar) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_171, {ep_name});
        return RET_FAILURE;
    }

    ep_peekpoke_addr    = ep.offset;
    ep_peekpoke_bar_idx = ep.pcie_base_address_register;
    ep_axi_addr         = ep.offset + bar.offset;
    ep_range            = ep.range;

    LogMessage(MSG_DEBUG_SETUP, {ep_name + " peek-poke address   = 0x" + NumToStrHex(ep_peekpoke_addr, 16)});
    LogMessage(MSG_DEBUG_SETUP, {ep_name + " peek-poke BAR index = " + std::to_string(ep_peekpoke_bar_idx)});
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::CheckCuIdle ( const Cl_Compute_Unit_t & cl_compute_unit, bool & idle )
{
    uint64_t peekpoke_addr;
    uint32_t peekpoke_data[1];

    idle = false;

    peekpoke_addr = cl_compute_unit.cu_peekpoke_addr+CU_AP_CTRL_ADDR;

    if (MemRead(cl_compute_unit.name, cl_compute_unit.cu_peekpoke_bar_idx, peekpoke_addr, 1, peekpoke_data) != RET_SUCCESS)
    {
        LogMessage(MSG_ITF_164, {"read", "from", cl_compute_unit.name});
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_OPENCL, {"xbtest HW IP control register 0x" + NumToStrHex(peekpoke_addr) + " = 0x" + NumToStrHex(peekpoke_data[0])});

    if ((peekpoke_data[0] & CU_AP_CTRL_IDLE) == CU_AP_CTRL_IDLE)
    {
        idle = true;
    }

    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::CheckNoCuDeadlock( const std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{
    bool idle;
    for ( const auto & cl_compute_unit : cl_compute_units)
    {
        if (CheckCuIdle(cl_compute_unit, idle) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_166, {cl_compute_unit.name});
            return RET_FAILURE;
        }
        if (!idle)
        {
            LogMessage(MSG_ITF_167, {cl_compute_unit.name});
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_OPENCL, {cl_compute_unit.name + " is in idle state as expected"});
    }
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::InitStruct(const Xbtest_Compute_Units_t & compute_unit, Cl_Compute_Unit_t & cl_compute_unit) {
    // Init
    cl_compute_unit.name    = compute_unit.name;
    cl_compute_unit.slr     = compute_unit.SLR;
    cl_compute_unit.cu_mtx  = new std::mutex;

    if (!(compute_unit.companion))
    {
        if (GetEndPointConfig(compute_unit.name + "_1", cl_compute_unit.cu_peekpoke_range, cl_compute_unit.cu_peekpoke_addr, cl_compute_unit.cu_peekpoke_bar_idx, cl_compute_unit.cu_axi_addr) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        cl_compute_unit.cu_peekpoke_addr = SetPeekPokeAddr(cl_compute_unit.cu_peekpoke_addr, cl_compute_unit.cu_peekpoke_bar_idx);
    }
    return RET_SUCCESS;
}

bool DeviceInterface_PeekPoke::CheckCUsNotDeadlocked() {
    // Check xbtest HW IPs are not deadlocked.
    LogMessage(MSG_DEBUG_SETUP, {"Checking all xbtest HW IPs are in idle state"});
    if (CheckNoCuDeadlock(this->m_cl_verify_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_pwr_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_gt_mac_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_gt_lpbk_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_gt_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_gtf_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_gtm_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckNoCuDeadlock(this->m_cl_gtyp_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        if (CheckNoCuDeadlock(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        if (CheckNoCuDeadlock(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }

    return RET_SUCCESS;
}

} // namespace

#endif