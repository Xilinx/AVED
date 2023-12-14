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

#ifndef _DEVICEINTERFACE_PEEKPOKE_H
#define _DEVICEINTERFACE_PEEKPOKE_H

#if defined(USE_AMI) || defined(USE_NO_DRIVER)

#include "deviceinterface.h"

namespace xbtest
{

class DeviceInterface_PeekPoke : public DeviceInterface_Base {

    public:

        DeviceInterface_PeekPoke                    ( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target );
        ~DeviceInterface_PeekPoke                   ();

        //ABSTRACT OVERRIDE
        bool                WriteGtmacCuCmd             ( const uint & cu_idx, const uint & value )                                         override;
        bool                WriteGtmacCuTrafficCfg      ( const uint & cu_idx, uint32_t * traffic_cfg )                                     override;
        bool                ReadGtmacCuTrafficCfg       ( const uint & cu_idx, uint32_t * read_buffer )                                     override;

        virtual bool        MemRead                     ( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t  * peekpoke_data )   = 0;
        virtual bool        MemWrite                    ( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t  * peekpoke_data )   = 0;
        bool                GetEndPointConfig           ( const std::string & ep_name, uint64_t & ep_range, uint64_t & ep_peekpoke_addr, uint8_t & ep_peekpoke_bar_idx, uint64_t & ep_axi_addr );
        virtual uint64_t    SetPeekPokeAddr             ( const uint64_t & addr, const uint8_t & bar )                                                                                                      = 0;

    protected:

        //ABSTRACT OVERRIDE
        bool        InitStruct                      (const Xbtest_Compute_Units_t & compute_unit, Cl_Compute_Unit_t & cl_compute_unit)                                  override;
        bool        ConfigureM00AxiBuffers          ( std::vector<Cl_Compute_Unit_t> & cl_compute_units )                                                               override;
        bool        CreateCuMxxAxiBuffers           ( std::vector<Cl_Compute_Unit_t> & cl_compute_units )                                                               override;
        bool        WriteComputeUnit                ( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, const uint & value )                                   override;
        bool        SetCuM00AxiPtrArgument          ( std::vector<Cl_Compute_Unit_t> & cl_compute_units )                                                               override;
        bool        ReadMemoryCuStatusInner         ( Cl_Compute_Unit_t & cl_compute_unit, uint32_t * read_buffer, const uint64_t & size)                               override;
        bool        ReadMultiGtStatus               ( Cl_Compute_Unit_t & cl_compute_unit, const uint & core_address, uint32_t * read_buffer, const uint64_t & size)    override;
        bool        ReadComputeUnit                 ( Cl_Compute_Unit_t & cl_compute_unit, const uint & address, uint & read_data )                                     override;
        bool        CheckCUsNotDeadlocked           ()                                                                                                                  override;

    private:

        bool        SetCuScalarArg                  ( const Cl_Compute_Unit_t & cl_compute_unit, const uint32_t & arg_idx, uint32_t & arg_data );
        bool        ExecuteCu                       ( const Cl_Compute_Unit_t & cl_compute_unit );
        bool        WaitCuIdle                      ( const Cl_Compute_Unit_t & cl_compute_unit );
        bool        SetCuAxiPtrArg                  ( const Cl_Compute_Unit_t & cl_compute_unit, const uint32_t & axi_idx, const uint64_t & axi_ptr_addr );
        bool        CheckCuIdle                     ( const Cl_Compute_Unit_t & cl_compute_unit, bool & idle );
        bool        CheckNoCuDeadlock               ( const std::vector<Cl_Compute_Unit_t> & cl_compute_units );

};

} // namespace

#endif

#endif /* _DEVICEINTERFACE_PEEKPOKE_H */