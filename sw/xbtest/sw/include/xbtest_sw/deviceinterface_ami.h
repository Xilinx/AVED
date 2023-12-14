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

#ifndef _DEVICEINTERFACE_AMI_H
#define _DEVICEINTERFACE_AMI_H

#ifdef USE_AMI

#include <ami.h>
#include <ami_device.h>
#include <ami_mem_access.h>
#include <ami_program.h>
#include <ami_sensor.h>

#include <deviceinterface_peekpoke.h>

namespace xbtest
{

class DeviceInterface : public DeviceInterface_PeekPoke {

    public:
        DeviceInterface                          ( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target );
        ~DeviceInterface                         ();

        //ABSTRACT OVERRIDE
        bool     SelectDevice                    ()                                                                                                                                                 override;
        bool     SetupDevice                     ()                                                                                                                                                 override;
        bool     MemRead                         ( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )   override;
        bool     MemWrite                        ( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )   override;
        uint64_t SetPeekPokeAddr                 ( const uint64_t & addr, const uint8_t & bar )                                                                                                     override;

    private:

        std::vector<ami_device*> m_ami_device;

        bool     CreateAmiDevice                ();
        void     ReleaseDevice                  ();

};

} // namespace

#endif

#endif /* _DEVICEINTERFACE_AMI_H */