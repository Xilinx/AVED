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

#ifdef USE_XRT

#include "deviceinterface_ocl.h"
#include "clpackage.h"

#define READ_CU_MIGRATE         false
#define USE_SUB_BUFFER          true

namespace xbtest
{

DeviceInterface::DeviceInterface(
    Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target
) : DeviceInterface_Base( log, xbtest_sw_config, xbtest_sw_config_p2p, abort, p2p_target ) {}

//use origianl
DeviceInterface::~DeviceInterface() { ReleaseDevice(); }

bool DeviceInterface::SelectDevice() {

    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    // Get platforms
    LogMessage(MSG_DEBUG_SETUP, {"Get OpenCL platform"});
    cl_err = cl::Platform::get(&(this->m_cl_platforms));
    CheckClPlatformGet(cl_err, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
    LogMessage(MSG_DEBUG_SETUP, {"Number of platforms found: " + std::to_string(this->m_cl_platforms.size())});

    // Select platform by name
    auto found_platform = false;
    cl::Platform cl_platform;
    for (uint ii = 0; ii < this->m_cl_platforms.size(); ii++)
    {
        cl_platform = this->m_cl_platforms[ii];
        auto cl_platformName = cl_platform.getInfo<CL_PLATFORM_NAME>(&cl_err);
        CheckClPlatformGetInfo(cl_err, "CL_PLATFORM_NAME", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
        LogMessage(MSG_DEBUG_SETUP, {"Platform name = " + cl_platformName + " for OpenCL platform at index: " + std::to_string(ii)});
        if (cl_platformName == XILINX_CL_PLATFORM_NAME)
        {
            found_platform = true;
            break;
        }
    }
    if (!found_platform)
    {
        LogMessage(MSG_ITF_005);
        return RET_FAILURE;
    }

    // Get all the devices on the "Xilinx" platform
    LogMessage(MSG_DEBUG_SETUP, {"Get all OpenCL devices"});
    this->m_cl_devices.clear();
    cl_err = cl_platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &(this->m_cl_devices));
    CheckClPlatformGetDevices(cl_err, "CL_DEVICE_TYPE_ACCELERATOR", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
    LogMessage(MSG_DEBUG_SETUP, {"Total OpenCL devices found: " + std::to_string(this->m_cl_devices.size())});

    // Select device for provided BDF
    std::string bdf;
    auto device_found = false;
    for (uint ii = 0; ii < this->m_cl_devices.size(); ii++)
    {
        this->m_cl_device = this->m_cl_devices[ii];
        cl_err = this->m_cl_device.getInfo<std::string>(CL_DEVICE_PCIE_BDF, &bdf);
        CheckClDeviceGetInfo(cl_err, "CL_DEVICE_PCIE_BDF", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
        LogMessage(MSG_DEBUG_SETUP, {"BDF = " + bdf + " for OpenCL device at index: " + std::to_string(ii)});

        if (CompareBDF(this->m_device_bdf, bdf))
        {
            device_found = true;
            LogMessage(MSG_DEBUG_SETUP, {"Found OpenCL device BDF at index: " + std::to_string(ii)});
            break;
        }
    }
    if (!device_found)
    {
        LogMessage(MSG_ITF_086, {this->m_device_bdf});
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

void DeviceInterface::ReleaseDevice() {
    this->m_xrt_compute_units.clear();
    ReleaseXRTDevice();
}

bool DeviceInterface::SetupDevice() {

    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    // OpenCL
    // Note: XRT sends SIGBUS to xbtest only if cl::Context/cl::CommandQueue is created
    this->m_cl_context = cl::Context(this->m_cl_device, nullptr, nullptr, nullptr, &cl_err); // Create a context
    CheckClContextConstructor(cl_err, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

    this->m_cl_cmd_queue = cl::CommandQueue(this->m_cl_context, this->m_cl_device, CL_QUEUE_PROFILING_ENABLE, &cl_err); // Create a command queue
    CheckClCommandQueueConstructor(cl_err, "CL_QUEUE_PROFILING_ENABLE", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

    LogMessage(MSG_ITF_009, {std::to_string(this->m_xbtest_sw_config->GetDownloadTime())});
    auto xclbin = this->m_xbtest_sw_config->GetXclbin();
    std::ifstream bin_file(xclbin, std::ifstream::binary); // Read the xclbin
    bin_file.seekg (0, bin_file.end);
    auto nb = bin_file.tellg();
    bin_file.seekg (0, bin_file.beg);
    char buf[nb];
    bin_file.read(buf, nb);
    cl::Program::Binaries bins{{buf,nb}}; // creating Program from Binary File

    auto begin = std::chrono::steady_clock::now();
    this->m_cl_program = cl::Program(this->m_cl_context, {this->m_cl_device}, bins, nullptr, &cl_err); // Download the xclbin on the device
    auto end = std::chrono::steady_clock::now();

    if (cl_err != CL_SUCCESS)
    {
        LogMessage(MSG_ITF_106, {this->m_xbtest_sw_config->GetNameInfo(), this->m_device_bdf, ProgrammConstructorClErrToStr(cl_err)});
        return RET_FAILURE;
    }
    auto ticks = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
    this->m_xclbin_download_time = (double)(ticks.count()) / (double)(1000) / (double)(1000);
    LogMessage(MSG_ITF_084, {Float_to_String(this->m_xclbin_download_time,1)});

    return RET_SUCCESS;
}

bool DeviceInterface::WriteGtmacCuCmd( const uint & cu_idx, const uint & value )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    //Rename variables to remove this
    const Cl_Compute_Unit_t cl_compute_unit = this->m_cl_gt_mac_compute_units[cu_idx];

    std::vector<cl::Event> waitEvent;
    cl::Event Event;
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header  = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Write GT_MAC xbtest HW IP command " + std::to_string(value)});

    // Arg 0
    uint arg_data = value & 0xffffffff; // set bits for data
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Set argument 0 : 0x" + NumToStrHex(arg_data)});
    cl_err = cl_compute_unit.cl_kernel.setArg(0, arg_data); // Set argument 0 with write data
    CheckClKernelSetArg(cl_err, msg_header, "0", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Enqueue a command to execute the xbtest HW IP
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    cl_err = this->m_cl_cmd_queue.enqueueTask(cl_compute_unit.cl_kernel, nullptr, &Event);
    CheckClCommandQueueEnqueueTask(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Wait for the xbtest HW IP to finish the execution
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    waitEvent.emplace_back(Event);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    cl_err = this->m_cl_cmd_queue.finish(); // Wait for the xbtest HW IP to finish the execution
    CheckClCommandQueueFinish(cl_err, msg_header + " execute xbtest HW IP", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully wrote data"});
    return RET_SUCCESS;
}

bool DeviceInterface::WriteGtmacCuTrafficCfg( const uint & cu_idx, uint32_t * traffic_cfg )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    //to be changed
    const Cl_Compute_Unit_t cl_compute_unit = this->m_cl_gt_mac_compute_units[cu_idx];

    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Writing GT_MAC xbtest HW IP traffic configuration (" + std::to_string(cl_compute_unit.cl_m00_axi_buffer_size) + " bytes)"});

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Write data in memory"});
    cl_err = this->m_cl_cmd_queue.enqueueWriteBuffer(cl_compute_unit.cl_m00_axi_buffer, CL_TRUE, 0, cl_compute_unit.cl_m00_axi_buffer_size, traffic_cfg, nullptr, nullptr);
    CheckClCommandQueueEnqueueWriteBuffer(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully wrote memory"});
    return RET_SUCCESS;
}

bool DeviceInterface::ReadGtmacCuTrafficCfg( const uint & cu_idx, uint32_t * read_buffer)
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    //change when fully tested
    const Cl_Compute_Unit_t cl_compute_unit = this->m_cl_gt_mac_compute_units[cu_idx];

    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading GT_MAC xbtest HW IP traffic configuration (" + std::to_string(GT_MAC_STATUS_SIZE * (uint64_t)(sizeof(uint32_t))) + " bytes)"});

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Read data in memory"});
    cl_err = this->m_cl_cmd_queue.enqueueReadBuffer(cl_compute_unit.cl_m00_axi_buffer, CL_TRUE, 0, GT_MAC_STATUS_SIZE * (uint64_t)(sizeof(uint32_t)), read_buffer, nullptr, nullptr);
    CheckClCommandQueueEnqueueReadBuffer(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully read memory"});
    return RET_SUCCESS;
}

bool DeviceInterface::SetCuM00AxiPtrArgument( std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    for (auto & cl_compute_unit : cl_compute_units)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Setting xbtest HW IP " + cl_compute_unit.name + " m00_axi xbtest HW IP argument"});
        cl_err = cl_compute_unit.cl_kernel.setArg(4, cl_compute_unit.cl_m00_axi_buffer);
        CheckClKernelSetArg(cl_err, cl_compute_unit.name, "4", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
    }

    return RET_SUCCESS;
}

bool DeviceInterface::ConfigureM00AxiBuffers( std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{
    for (auto & cl_compute_unit : cl_compute_units)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Configuring m00_axi buffer for xbtest HW IP " + cl_compute_unit.name + ": size = " + std::to_string(cl_compute_unit.cl_m00_axi_buffer_size) + " Bytes / memory data index = " + std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx)});

        auto found = false;
        for (auto & cl_m00_axi_buffer : this->cl_m00_axi_buffers)
        {
            if (cl_m00_axi_buffer.mem_data_idx == cl_compute_unit.cl_m00_axi_mem_data_idx)
            {
                cl_compute_unit.cl_m00_axi_buffer_origin = cl_m00_axi_buffer.buffer_size;
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

            auto plram_mem_info = RST_MEM_INFO;
            if (this->m_xbtest_sw_config->GetDeviceMemInfo(cl_m00_axi_buffer.mem_data_idx, plram_mem_info) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            cl_m00_axi_buffer.available_size = plram_mem_info.range_bytes;

            this->cl_m00_axi_buffers.emplace_back(cl_m00_axi_buffer);

            cl_compute_unit.cl_m00_axi_buffer_origin = 0;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::CheckXclbinDownloadTime()
{
    auto expected_download_time = (double)(this->m_xbtest_sw_config->GetDownloadTime());
    if (this->m_xclbin_download_time > expected_download_time)
    {
        if (this->m_xbtest_sw_config->IsExpectedPcieSpeedWidth())
        {
            LogMessage(MSG_ITF_017_ERROR, {Float_to_String(this->m_xclbin_download_time,1), Float_to_String(expected_download_time,0)});
            return RET_FAILURE;
        }
        LogMessage(MSG_ITF_017_WARN, {Float_to_String(this->m_xclbin_download_time,1), Float_to_String(expected_download_time,0)});
        return RET_SUCCESS;
    }
    LogMessage(MSG_ITF_018, {Float_to_String(this->m_xclbin_download_time,1), Float_to_String(expected_download_time,0)});
    return RET_SUCCESS;
}

bool DeviceInterface::InitStruct(const Xbtest_Compute_Units_t & compute_unit, Cl_Compute_Unit_t & cl_compute_unit) {
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    // Init
    cl_compute_unit.name    = compute_unit.name;
    cl_compute_unit.slr     = compute_unit.SLR;
    cl_compute_unit.cu_mtx  = new std::mutex;

    // Do not create cl::Kernel for companion xbtest HW IP as it is not used. Also AP_CTRL_NONE is only supported by XRT native API, not supported in XRT OpenCL API
    if (!(compute_unit.companion))
    {
        cl_compute_unit.cl_kernel = cl::Kernel(this->m_cl_program, cl_compute_unit.name.c_str(), &cl_err);
        CheckClKernelConstructor(cl_err, cl_compute_unit.name, chk_cl_err);

        CHK_CL_ERR_RETURN(chk_cl_err);
    }
    return RET_SUCCESS;
}

bool DeviceInterface::InitCUArgs() {
    // Initialize xbtest HW IP arguments
    if (InitCuScalarArgs(this->m_cl_verify_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_pwr_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_gt_mac_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_gt_lpbk_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_gt_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_gtf_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_gtm_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (InitCuScalarArgs(this->m_cl_gtyp_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        if (InitCuScalarArgs(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        if (InitCuScalarArgs(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::InitCuScalarArgs( std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    for (auto & cl_compute_unit : cl_compute_units)
    {
        for (uint arg_idx=0; arg_idx < 4; arg_idx++)
        {
            cl_err = cl_compute_unit.cl_kernel.setArg(arg_idx, 0);
            CheckClKernelSetArg(cl_err, cl_compute_unit.name, std::to_string(arg_idx), chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::CreateAllM00AXIBuffers()
{
    // Create M00_AXI buffers
    if (CreateM00AxiBuffers() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // Create M00_AXI sub-buffers
    if (CreateCuM00AxiSubBuffers(this->m_cl_verify_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_pwr_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_gt_mac_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_gt_lpbk_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_gt_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_gtf_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_gtm_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateCuM00AxiSubBuffers(this->m_cl_gtyp_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        if (CreateCuM00AxiSubBuffers(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        if (CreateCuM00AxiSubBuffers(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::CreateM00AxiBuffers()
{
    #if (USE_SUB_BUFFER)

    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    #endif

    LogMessage(MSG_DEBUG_SETUP, {"Creating kernels m00_axi buffers"});
    for (auto & cl_m00_axi_buffer : this->cl_m00_axi_buffers)
    {
        #if (USE_SUB_BUFFER)

        LogMessage(MSG_DEBUG_SETUP, {"Creating buffer of size " + std::to_string(cl_m00_axi_buffer.buffer_size) + " Bytes for memory data index: " + std::to_string(cl_m00_axi_buffer.mem_data_idx)});
        cl_mem_ext_ptr_t cl_mem_ext_ptr;
        cl_mem_ext_ptr.param    = nullptr;
        cl_mem_ext_ptr.obj      = nullptr;
        cl_mem_ext_ptr.flags    = cl_m00_axi_buffer.mem_data_idx | XCL_MEM_TOPOLOGY;

        cl_m00_axi_buffer.cl_buffer = cl::Buffer(
            this->m_cl_context,
            static_cast<cl_mem_flags>(CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX),
            cl_m00_axi_buffer.buffer_size,
            &cl_mem_ext_ptr,
            &cl_err
        );
        CheckClBufferConstructor(cl_err, "m00_axi buffer (memory data index = " + std::to_string(cl_m00_axi_buffer.mem_data_idx) + ")", "CL_MEM_READ_WRITE", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

        #endif
    }
    return RET_SUCCESS;
}

bool DeviceInterface::CreateCuM00AxiSubBuffers( std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{

    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    for (auto & cl_compute_unit : cl_compute_units)
    {
        #if (USE_SUB_BUFFER)

        // By design we should find the buffer here
        bool found = false;
        for (auto & cl_m00_axi_buffer : this->cl_m00_axi_buffers)
        {
            if (cl_m00_axi_buffer.mem_data_idx != cl_compute_unit.cl_m00_axi_mem_data_idx)
            {
                continue;
            }
            found = true;

            LogMessage(MSG_DEBUG_SETUP, {"Creating m00_axi sub-buffer for xbtest HW IP " + cl_compute_unit.name + ": size = " + std::to_string(cl_compute_unit.cl_m00_axi_buffer_size) + " Bytes / memory data index = " + std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx)});

            cl_buffer_region cl_m00_axi_buffer_region;
            cl_m00_axi_buffer_region.size    = cl_compute_unit.cl_m00_axi_buffer_size;
            cl_m00_axi_buffer_region.origin  = cl_compute_unit.cl_m00_axi_buffer_origin;
            cl_compute_unit.cl_m00_axi_buffer = cl_m00_axi_buffer.cl_buffer.createSubBuffer(CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &cl_m00_axi_buffer_region, &cl_err);
            CheckClCreateSubBuffer(cl_err, cl_compute_unit.name + " m00_axi sub-buffer (memory data index = " + std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx) + ")", "CL_MEM_READ_WRITE", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
        }
        // Should be found by design
        if (!found)
        {
            LogMessage(MSG_ITF_144, {cl_compute_unit.name, std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx)});
            return RET_FAILURE;
        }

        #else

        LogMessage(MSG_DEBUG_SETUP, {"Creating m00_axi buffer for xbtest HW IP " + cl_compute_unit.name + ": size = " + std::to_string(cl_compute_unit.cl_m00_axi_buffer_size) + " Bytes / memory data index = " + std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx)});

        cl_mem_ext_ptr_t cl_mem_ext_ptr;
        cl_mem_ext_ptr.param    = nullptr;
        cl_mem_ext_ptr.obj      = nullptr;
        cl_mem_ext_ptr.flags    = cl_compute_unit.cl_m00_axi_mem_data_idx | XCL_MEM_TOPOLOGY;

        cl_compute_unit.cl_m00_axi_buffer = cl::Buffer(
            this->m_cl_context,
            static_cast<cl_mem_flags>(CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX),
            cl_compute_unit.cl_m00_axi_buffer_size,
            &cl_mem_ext_ptr,
            &cl_err
        );
        CheckClBufferConstructor(cl_err, cl_compute_unit.name + "m00_axi buffer (memory data index = " + std::to_string(cl_compute_unit.cl_m00_axi_mem_data_idx) + ")", "CL_MEM_READ_WRITE", chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

        #endif

    }
    return RET_SUCCESS;
}

bool DeviceInterface::CreateCuMxxAxiBuffers( std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{

    // function should be called only for Memory xbtest HW IPs
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    for (auto & cl_compute_unit : cl_compute_units)
    {
        std::vector<cl::Buffer> cl_buffers;

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
            // For XRT, need to manage OpenCL buffers to get Mxx_AXI base address set with correct values.
            auto cu_connection_mem_tags             = this->m_xbtest_sw_config->GetCUConnectionMemTags(cl_compute_unit.name, ch_idx);
            auto cu_connection_mem_data_indexes     = this->m_xbtest_sw_config->GetCUConnectionMemDataIndexes(cl_compute_unit.name, ch_idx);
            auto cu_connection_actual_sizes         = this->m_xbtest_sw_config->GetCuConnectionActualSizes(cl_compute_unit.name, ch_idx);
            auto cu_connection_dev_info_sizes       = this->m_xbtest_sw_config->GetCuConnectionDeviceInfoSizes(cl_compute_unit.name, ch_idx);
            auto num_cu_connection_mem_tags         = cu_connection_mem_tags.size();

            // Check if not case 4
            auto use_mem_data_index = false; // when multiple tags are connected to Memory xbtest HW IP channel, the default memory topology index is used
            if (num_cu_connection_mem_tags == 1)
            {
                use_mem_data_index = true;
            }

            for (uint tag_idx = 0; tag_idx < num_cu_connection_mem_tags; tag_idx++)
            {
                auto mem_tag        = cu_connection_mem_tags[tag_idx];
                auto mem_data_index = cu_connection_mem_data_indexes[tag_idx];
                auto actual_size    = cu_connection_actual_sizes[tag_idx];
                auto dev_info_size  = cu_connection_dev_info_sizes[tag_idx];

                auto buff_info = "for memory data index " + std::to_string(mem_data_index) + " (" + mem_tag + ") for channel " + std::to_string(ch_idx) + " of xbtest HW IP " + cl_compute_unit.name;

                // CL_MEM_READ_WRITE: This flag specifies that the memory object will be read and written by a xbtest HW IP. This is the default.
                auto        mem_flags     =  CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX;
                std::string mem_flags_str = "CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX";

                cl_mem_ext_ptr_t cl_mem_ext_ptr;
                cl_mem_ext_ptr.param    = nullptr;
                cl_mem_ext_ptr.obj      = nullptr;
                cl_mem_ext_ptr.flags    = 0;
                if (use_mem_data_index)
                {
                    cl_mem_ext_ptr.flags |=                        mem_data_index      | XCL_MEM_TOPOLOGY;
                    mem_flags_str        += " | " + std::to_string(mem_data_index) + " | XCL_MEM_TOPOLOGY";
                }
                if (memory_target == HOST)
                {
                    cl_mem_ext_ptr.flags |=     XCL_MEM_EXT_HOST_ONLY;
                    mem_flags_str        += " | XCL_MEM_EXT_HOST_ONLY";
                }
                else
                {
                    // CL_MEM_HOST_NO_ACCESS: This flag specifies that the host will not read or write the memory object.
                    mem_flags     |=     CL_MEM_HOST_NO_ACCESS;
                    mem_flags_str += " | CL_MEM_HOST_NO_ACCESS";
                }
                // Create buffer in the first tag to be used by the xbtest HW IP
                if (tag_idx == 0)
                {
                    if (!use_mem_data_index)
                    {
                        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Leave default memory data index instead of selecting between: " + NumVectToStr<uint>(cu_connection_mem_data_indexes, ", ") + " (" + StrVectToStr(cu_connection_mem_tags, ", ") + ")"});
                    }
                    cl_buffers.emplace_back(cl::Buffer(
                        this->m_cl_context,
                        static_cast<cl_mem_flags>(mem_flags),
                        M_AXI_BASE_SIZE_BYTES,
                        &cl_mem_ext_ptr,
                        &cl_err
                    ));
                    CheckClBufferConstructor(cl_err, buff_info, mem_flags_str, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

                    cl_err = cl_compute_unit.cl_kernel.setArg(5+ch_idx, cl_buffers.back());
                    CheckClKernelSetArg(cl_err, cl_compute_unit.name, std::to_string(5+ch_idx), chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
                }

                // The Memory xbtest HW IP channel does not have access to all memory (e.g. case 3), create extra buffer so other Memory xbtest HW IP channel target correct offset in memory
                if (dev_info_size != actual_size)
                {
                    LogMessage(MSG_DEBUG_SETUP, {"\t\t* Creating extra buffers for the remaining of channel actual memory size " + std::to_string((double)(actual_size)/(double)(1024)/(double)(1024))});
                    auto remaining_size_bytes = actual_size;
                    if (tag_idx == 0) // in case 3, use full memory size for tag_idx > 0
                    {
                        remaining_size_bytes -= M_AXI_BASE_SIZE_BYTES;
                    }
                    uint64_t num_extra_buffer = 0; // Need to create multiple extra buffer as the max buffer size is MAX_OCL_BUFFER_SIZE_MB
                    while (remaining_size_bytes > 0)
                    {
                        auto buffer_size_bytes = remaining_size_bytes;
                        if (buffer_size_bytes > MAX_OCL_BUFFER_SIZE_MB * (uint64_t)(1024) * (uint64_t)(1024))
                        {
                            buffer_size_bytes = MAX_OCL_BUFFER_SIZE_MB * (uint64_t)(1024) * (uint64_t)(1024);
                        }
                        cl_buffers.emplace_back(cl::Buffer(
                            this->m_cl_context,
                            static_cast<cl_mem_flags>(mem_flags),
                            buffer_size_bytes,
                            &cl_mem_ext_ptr,
                            &cl_err
                        ));
                        if (cl_err != CL_SUCCESS)
                        {
                            CheckClBufferConstructor(cl_err, buff_info, mem_flags_str, chk_cl_err);
                            LogMessage(MSG_DEBUG_EXCEPTION, {chk_cl_err.msg});

                            double percent = (double)(remaining_size_bytes) / (double)(actual_size) * 100.0;
                            auto idx_str = "default (" + NumVectToStr<uint>(cu_connection_mem_data_indexes, ", ") + ")";
                            auto tag_str = "default (" + StrVectToStr(cu_connection_mem_tags, ", ") + ")";
                            if (use_mem_data_index)
                            {
                                idx_str = std::to_string(mem_data_index);
                                tag_str = mem_tag;
                            }
                            LogMessage(MSG_ITF_105, {tag_str, idx_str, std::to_string(buffer_size_bytes), BufferConstructorClErrToStr(cl_err), std::to_string(remaining_size_bytes), std::to_string(percent)});
                            cl_buffers.clear(); // This would free all extra buffers that were created to allocate 100% memory tested by the Memory xbtest HW IPs
                            break;
                        }

                        remaining_size_bytes -= buffer_size_bytes;
                        num_extra_buffer++;
                    }
                    LogMessage(MSG_DEBUG_SETUP, {"\t\t* Total number of extra buffers created: " + std::to_string(num_extra_buffer)});
                }
            }
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::ConfigurePlramShadowBuffers()
{
    // Applicable for NoDMA Vitis Platform use with XRT: the host memory is used as shadow buffer for the OpenCL regular buffers (PLRAM)
    if (this->m_xbtest_sw_config->GetNodmaInfo())
    {
        // We need to get the total PLRAM buffer sizes
        uint64_t total_plram_buffer_size = 0;
        for (const auto & cl_m00_axi_buffer : this->cl_m00_axi_buffers)
        {
            total_plram_buffer_size += cl_m00_axi_buffer.buffer_size;
        }
        // In the Host memory we make sure the memory area corresponding to the shadow buffer of PLRAM buffer is not used in the HOST memory TC
        if (this->m_xbtest_sw_config->ComputeNewHostMemSize(total_plram_buffer_size) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::ReadComputeUnit( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, uint & read_data )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    read_data = 0; // 0 read_data to match previous behaviour when ReadComputeUnit fails and return code not checked

    std::vector<cl::Event> waitEvent;
    cl::Event Event;
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading xbtest HW IP register 0x" + NumToStrHex(address)});

    // Set argument 0 with register address and read flag
    uint arg_data = (address << 4) & 0xfffffff0; // set bits for address
    arg_data |= 0x1; // set bit for read command
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Set argument 0: 0x" + NumToStrHex(arg_data)});
    cl_err = cl_compute_unit.cl_kernel.setArg(0, arg_data);
    CheckClKernelSetArg(cl_err, msg_header, "0", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Enqueue a command to execute the xbtest HW IP
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    cl_err = this->m_cl_cmd_queue.enqueueTask(cl_compute_unit.cl_kernel, nullptr, &Event);
    CheckClCommandQueueEnqueueTask(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Wait for the xbtest HW IP to finish the execution
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    waitEvent.emplace_back(Event);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + " execute xbtest HW IP", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();

    // Once the xbtest HW IP finishes the execution, the 32-bit read data is available at the offset 0x0 of the OpenCL buffer (m00_axi)
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Read data in memory"});

    #if (READ_CU_MIGRATE)
    cl::Event mapEvent;
    auto host_ptr = this->m_cl_cmd_queue.enqueueMapBuffer(cl_compute_unit.cl_m00_axi_buffer, CL_TRUE, CL_MAP_READ, 0, BUFF_SIZE, nullptr, &mapEvent, &cl_err);
    CheckClEnqueueMapBuffer(cl_err, msg_header, "CL_MAP_READ | CL_MAP_WRITE", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    waitEvent.emplace_back(mapEvent);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + " migrate data", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();

    cl::Event migrateEvent;
    auto host_buff = reinterpret_cast<unsigned char*>(host_ptr);

    cl_err = this->m_cl_cmd_queue.enqueueMigrateMemObjects({cl_compute_unit.cl_m00_axi_buffer}, CL_MIGRATE_MEM_OBJECT_HOST, nullptr, &migrateEvent);
    CheckClEnqueueMigrateMemObjects(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    waitEvent.emplace_back(migrateEvent);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + " migrate data", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();

    #else
    auto host_buff = (unsigned char*)malloc(BUFF_SIZE);
    cl_err = this->m_cl_cmd_queue.enqueueReadBuffer(cl_compute_unit.cl_m00_axi_buffer, CL_TRUE, 0, BUFF_SIZE, host_buff, nullptr, nullptr);
    CheckClCommandQueueEnqueueReadBuffer(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);
    #endif

    // Retrieve 4B read value from host buffer
    auto value = (uint)(host_buff[0]);
    value     |= (uint)(host_buff[1]) << 8;
    value     |= (uint)(host_buff[2]) << 16;
    value     |= (uint)(host_buff[3]) << 24;

    #if (READ_CU_MIGRATE)
    cl_err = this->m_cl_cmd_queue.enqueueUnmapMemObject(cl_compute_unit.cl_m00_axi_buffer, host_ptr);
    CheckClEnqueueUnmapBuffer(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + " unmap memory object", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);
    #else
    free(host_buff);
    #endif

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully read xbtest HW IP register @ address: 0x" + NumToStrHex(address) + ", value 0x" + NumToStrHex(value)});
    read_data = value;
    return RET_SUCCESS;
}

bool DeviceInterface::WriteComputeUnit( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, const uint & value )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::vector<cl::Event> waitEvent;
    cl::Event Event;
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Writing xbtest HW IP register 0x" + NumToStrHex(address) + " to 0x" + NumToStrHex(value)});

    // Set argument 0 with register address and write flag
    uint arg_data = (address << 4) & 0xfffffff0; // set bits for address
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Set argument 0: 0x" + NumToStrHex(arg_data)});
    cl_err = cl_compute_unit.cl_kernel.setArg(0, arg_data);
    CheckClKernelSetArg(cl_err, msg_header, "0", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Set argument 1 with register value
    arg_data = value & 0xffffffff; // set bits for data
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Set argument 1: 0x" + NumToStrHex(arg_data)});
    cl_err = cl_compute_unit.cl_kernel.setArg(1, arg_data);
    CheckClKernelSetArg(cl_err, msg_header, "1", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Enqueue a command to execute the xbtest HW IP
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    cl_err = this->m_cl_cmd_queue.enqueueTask(cl_compute_unit.cl_kernel, nullptr, &Event);
    CheckClCommandQueueEnqueueTask(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Wait for the xbtest HW IP to finish the execution
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    waitEvent.emplace_back(Event);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + " execute xbtest HW IP", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully wrote xbtest HW IP register @ address: 0x" + NumToStrHex(address) + ", value 0x" + NumToStrHex(value)});
    return RET_SUCCESS;
}

bool DeviceInterface::ReadMemoryCuStatusInner( Cl_Compute_Unit_t & cl_compute_unit, uint32_t * read_buffer, const uint64_t & size )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::vector<cl::Event> waitEvent;
    cl::Event Event;
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading Memory xbtest HW IP status (" + std::to_string(size) + " bytes)"});

    // Set argument 0 with read status flag
    uint arg_data = 0x3;
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Set argument 0: 0x" + NumToStrHex(arg_data)});
    cl_err = cl_compute_unit.cl_kernel.setArg(0, arg_data);
    CheckClKernelSetArg(cl_err, msg_header, "0", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Enqueue a command to execute the xbtest HW IP
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    cl_err = this->m_cl_cmd_queue.enqueueTask(cl_compute_unit.cl_kernel, nullptr, &Event);
    CheckClCommandQueueEnqueueTask(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Wait for the xbtest HW IP to finish the execution
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    waitEvent.emplace_back(Event);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + "\t - Execute xbtest HW IP", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();

    // Once the xbtest HW IP finishes the execution, the data buffer is available
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Read data in memory"});
    cl_err = this->m_cl_cmd_queue.enqueueReadBuffer(cl_compute_unit.cl_m00_axi_buffer, CL_TRUE, 0, size, read_buffer, nullptr, nullptr);
    CheckClCommandQueueEnqueueReadBuffer(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully read memory"});
    return RET_SUCCESS;
}


bool DeviceInterface::ReadMultiGtStatus( Cl_Compute_Unit_t & cl_compute_unit, const uint & core_address, uint32_t * read_buffer, const uint64_t & size )
{
    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);

    std::vector<cl::Event> waitEvent;
    cl::Event Event;
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;
    std::string msg_header = cl_compute_unit.name + ": ";
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "Reading Multi GT xbtest HW IP status (" + std::to_string(size) + " bytes)"});

    // Set argument 0 with core address
    uint arg_data = (core_address << 4) & 0xfffffff0; // set bits for address
    arg_data |= 0x3; // set bit for multi read command
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Set argument 0: 0x" + NumToStrHex(arg_data)});
    cl_err = cl_compute_unit.cl_kernel.setArg(0, arg_data);
    CheckClKernelSetArg(cl_err, msg_header, "0", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Enqueue a command to execute the xbtest HW IP
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Execute xbtest HW IP"});
    cl_err = this->m_cl_cmd_queue.enqueueTask(cl_compute_unit.cl_kernel, nullptr, &Event);
    CheckClCommandQueueEnqueueTask(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    // Wait for the xbtest HW IP to finish the execution
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Wait for xbtest HW IP to complete"});
    waitEvent.emplace_back(Event);
    cl_err = cl::WaitForEvents(waitEvent);
    CheckClWaitForEvents(cl_err, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    CHK_ABORT_RETURN(this->m_abort_dev_access_msg, this->m_abort, this->m_cl_fail);
    cl_err = this->m_cl_cmd_queue.finish();
    CheckClCommandQueueFinish(cl_err, msg_header + "\t - Execute xbtest HW IP", chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    waitEvent.clear();

    // Once the xbtest HW IP finishes the execution, the data buffer is available
    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Read data in Multi GT"});
    cl_err = this->m_cl_cmd_queue.enqueueReadBuffer(cl_compute_unit.cl_m00_axi_buffer, CL_TRUE, 0, size, read_buffer, nullptr, nullptr);
    CheckClCommandQueueEnqueueReadBuffer(cl_err, msg_header, chk_cl_err); CHK_CL_ERR_ABORT_RETURN(chk_cl_err, this->m_abort, this->m_cl_fail);

    LogMessage(MSG_DEBUG_OPENCL, {msg_header + "\t - Successfully read Multi GT"});
    return RET_SUCCESS;
}

void DeviceInterface::ReleaseXRTDevice()
{
    if (!(this->m_xrt_device.empty())) // release only if previously created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Releasing xrt::device for card at " + this->m_device_bdf});
        this->m_xrt_device.clear();
    }
}

bool DeviceInterface::CreateXrtDevice()
{
    if (this->m_xrt_device.empty()) // Create only if not already created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Creating xrt::device for card at " + this->m_device_bdf});
        // Create device inside the try-catch block to able to catch exceptions (e.g. when card is reset)
        // Note if device is not released while a xbutil reset is in progress, then xbutil reset cannot terminate
        try
        {
            this->m_xrt_device.emplace_back(xrt::device(this->m_device_bdf));
        }
        catch (const std::exception& e)
        {
            LogMessage(MSG_ITF_134, {this->m_device_bdf, e.what()});
            ReleaseXRTDevice();
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInterface::LoadXrtDeviceXclbin()
{
    LogMessage(MSG_DEBUG_SETUP, {"Loading xclbin with xrt::device for card"});
    try
    {
        this->m_uuid = this->m_xrt_device[0].load_xclbin(this->m_xbtest_sw_config->GetXclbin());
    }
    catch (const std::exception& e)
    {
        LogMessage(MSG_ITF_153, {e.what()});
        ReleaseXRTDevice();
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface::CreateXrtDeviceCu( std::string cu_name )
{
    LogMessage(MSG_DEBUG_SETUP, {"Creating xrt::kernel " + cu_name});
    try
    {
        Xrt_Compute_Unit_t xrt_compute_unit;
        xrt_compute_unit.name       = cu_name;
        xrt_compute_unit.xrt_kernel = xrt::kernel(this->m_xrt_device[0], this->m_uuid.get(), cu_name);
        xrt_compute_unit.xrt_run    = xrt::run(xrt_compute_unit.xrt_kernel);
        this->m_xrt_compute_units.emplace_back(std::move(xrt_compute_unit));
    }
    catch (const std::exception& e)
    {
        LogMessage(MSG_ITF_154, {cu_name, e.what()});
        ReleaseXRTDevice();
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface::SetupPsComputeUnits()
{
    std::vector<std::string> ps_cu_names;
    for (const auto & compute_unit : this->m_xbtest_sw_config->GetComputeUnitDefinitions())
    {
        if (compute_unit.mode == BI_PWR_HW_COMPONENT_ID)
        {
            if ((compute_unit.cu_type_configuration.use_aie == 1) && (StrMatchNoCase(compute_unit.cu_type_configuration.aie.control, AIE_CONTROL_PS)))
            {
                ps_cu_names.emplace_back(PS_KERNEL_NAME_POWER);
            }
        }
    }
    if (!ps_cu_names.empty())
    {
        this->m_use_ps_kernels = true;
    }
    else
    {
        return RET_SUCCESS;
    }

    if (this->CreateXrtDevice()     == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (this->LoadXrtDeviceXclbin() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    for (const auto & ps_cu_name: ps_cu_names)
    {
        if (this->CreateXrtDeviceCu(ps_cu_name) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

cl::CommandQueue * DeviceInterface::GetCmdQueueInstance()
{
    return &(this->m_cl_cmd_queue);
}
cl::Context * DeviceInterface::GetContextInstance()
{
    return &(this->m_cl_context);
}

xrt::run * DeviceInterface::GetPowertestPsRun( std::string cu_name )
{
    for (auto & xrt_compute_unit: this->m_xrt_compute_units)
    {
        if (xrt_compute_unit.name == cu_name)
        {
            return &(xrt_compute_unit.xrt_run);
        }
    }
    return nullptr;
}

} // namespace

#endif