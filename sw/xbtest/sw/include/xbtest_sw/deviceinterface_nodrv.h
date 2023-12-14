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

#ifndef _DEVICEINTERFACE_NODRV_H
#define _DEVICEINTERFACE_NODRV_H

#ifdef USE_NO_DRIVER

#include <deviceinterface_peekpoke.h>

namespace xbtest
{

class DeviceInterface : public DeviceInterface_PeekPoke {

    public:
        DeviceInterface( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const bool & p2p_target );
        ~DeviceInterface();

        //ABSTRACT OVERRIDE
        bool        SelectDevice                ()                                                                                                                                                  override;
        bool        SetupDevice                 ()                                                                                                                                                  override;
        bool        MemRead                     ( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )    override;
        bool        MemWrite                    ( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )    override;
        uint64_t    SetPeekPokeAddr             ( const uint64_t & addr, const uint8_t & bar )                                                                                                      override;

    private:

        bool        GetUserBarInfo              ();

        using Bar_Info_t = struct Bar_Info_t {
            uint        index;
            long long   start_addr;
            long long   end_addr;
            long long   size;
            long long   meta;
        };

        std::string m_pci_dev_resource_file;
        std::string m_dev_mem_fname = "/dev/mem";
        std::vector<Bar_Info_t> m_bars_info;
        int m_page_size = 0;

        const uint MMIO_CU_READ     = 0; // Read xbtest HW IP status/arguments
        const uint MMIO_PLRAM_READ  = 1; // Read PLRAM
        const uint MMIO_CU_WRITE    = 2; // Write xbtest HW IP control/arguments
        const uint MMIO_PLRAM_WRITE = 3; // Write PLRAM

        using Mmap_Info_t = struct Mmap_Info_t {
            int             fd;
            volatile void * mem;
            uint64_t        address;
            uint64_t        offset;
            uint64_t        length;
        };

        bool        Open_Mmap                   ( Mmap_Info_t & mmap_info, const uint & access_type, const uint64_t & size );
        bool        Close_Mmap                  ( Mmap_Info_t & mmap_info );
        bool        MmioAccess                  ( const std::string & endpoint_name, const uint & access_type, const uint64_t & address, const uint64_t & size, uint32_t * data );

        std::mutex  m_dev_mem_mtx;

};

} // namespace

#endif

#endif /* _DEVICEINTERFACE_AMI_H */