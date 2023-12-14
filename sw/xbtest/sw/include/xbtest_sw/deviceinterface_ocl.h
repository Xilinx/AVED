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

#ifndef _DEVICEINTERFACE_OCL_H
#define _DEVICEINTERFACE_OCL_H

#ifdef USE_XRT

// XRT includes
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>

#include "deviceinterface.h"

namespace xbtest
{

class DeviceInterface : public DeviceInterface_Base {

    public:

        DeviceInterface                             ( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const bool & p2p_target );
        ~DeviceInterface                            ();

        //PUBLIC OVERRIDES
        bool        CheckXclbinDownloadTime         ()                                                                                      override;
        //ABSTRACT
        bool        SelectDevice                    ()                                                                                      override;
        bool        SetupDevice                     ()                                                                                      override;
        bool        WriteGtmacCuCmd                 ( const uint & cu_idx, const uint & value )                                             override;
        bool        WriteGtmacCuTrafficCfg          ( const uint & cu_idx, uint32_t * traffic_cfg )                                         override;
        bool        ReadGtmacCuTrafficCfg           ( const uint & cu_idx, uint32_t * read_buffer )                                         override;

        //Public XRT Exclusive
        cl::CommandQueue *  GetCmdQueueInstance     ();
        cl::Context *       GetContextInstance      ();
        xrt::run *  GetPowertestPsRun               ( std::string cu_name );

    protected:

        bool        SetupPsComputeUnits             ()                                                                                      override;
        bool        ConfigurePlramShadowBuffers     ()                                                                                      override;
        bool        CreateAllM00AXIBuffers          ()                                                                                      override;
        bool        InitCUArgs                      ()                                                                                      override;


        //ABSTRACT OVERRIDES
        bool        InitStruct                      (const Xbtest_Compute_Units_t & compute_unit, Cl_Compute_Unit_t & cl_compute_unit)                                  override;
        bool        ConfigureM00AxiBuffers          ( std::vector<Cl_Compute_Unit_t> & cl_compute_units )                                                               override;
        bool        SetCuM00AxiPtrArgument          ( std::vector<Cl_Compute_Unit_t> & cl_compute_units )                                                               override;
        bool        CreateCuMxxAxiBuffers           ( std::vector<Cl_Compute_Unit_t> & cl_compute_units )                                                               override;
        bool        ReadComputeUnit                 ( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, uint & read_data )                                     override;
        bool        ReadMemoryCuStatusInner         ( Cl_Compute_Unit_t & cl_compute_unit, uint32_t * read_buffer, const uint64_t & size)                               override;
        bool        ReadMultiGtStatus               ( Cl_Compute_Unit_t & cl_compute_unit, const uint & core_address, uint32_t * read_buffer, const uint64_t & size)    override;
        bool        WriteComputeUnit                ( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, const uint & value )                                   override;



    private:

        void        ReleaseDevice                   ();
        void        ReleaseXRTDevice                ();
        bool        InitCuScalarArgs                ( std::vector<Cl_Compute_Unit_t> & cl_compute_units );
        bool        CreateM00AxiBuffers             ();
        bool        CreateCuM00AxiSubBuffers        ( std::vector<Cl_Compute_Unit_t> & cl_compute_units );
        bool        CreateXrtDevice                 ();
        bool        LoadXrtDeviceXclbin             ();
        bool        CreateXrtDeviceCu               ( std::string cu_name );

        cl::CommandQueue                                        m_cl_cmd_queue;
        cl::Context                                             m_cl_context;

        using Xrt_Compute_Unit_t = struct Xrt_Compute_Unit_t
        {
            std::string name;
            xrt::kernel xrt_kernel;
            xrt::run    xrt_run;
        };

        std::vector<cl::Platform>       m_cl_platforms;
        std::vector<cl::Device>         m_cl_devices;
        cl::Device                      m_cl_device;
        cl::Program                     m_cl_program;

        std::vector<xrt::device>        m_xrt_device;
        xrt::uuid                       m_uuid;
        std::vector<Xrt_Compute_Unit_t> m_xrt_compute_units;
};

} // namespace

#endif

#endif /* _DEVICEINTERFACE_OCL_H */