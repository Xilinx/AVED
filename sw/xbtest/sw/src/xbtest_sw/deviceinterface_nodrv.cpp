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

// Low level Peek-poke functions

#ifdef USE_NO_DRIVER

#include <deviceinterface_nodrv.h>

namespace xbtest
{

DeviceInterface::DeviceInterface (
    Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target
)  : DeviceInterface_PeekPoke( log, xbtest_sw_config, xbtest_sw_config_p2p, abort, p2p_target ) {
    this->m_pci_dev_resource_file = "/sys/bus/pci/devices/" + ExtendBDF(this->m_device_bdf) + "/resource";
}

//use origianl
DeviceInterface::~DeviceInterface() {}

bool DeviceInterface::SelectDevice() {
    return RET_SUCCESS;
}

bool DeviceInterface::SetupDevice()
{
    if (GetUserBarInfo() ==  RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_SETUP, {"FAILURE: Failed to get user BAR info"});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

uint64_t DeviceInterface::SetPeekPokeAddr( const uint64_t & addr, const uint8_t & bar ) {
    return addr + this->m_bars_info[bar].start_addr;
}

// No need to specify peekpoke_bar_idx to MmioAccess as Bar offset included in peekpoke_addr by SetPeekPokeAddr

bool DeviceInterface::MemWrite( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )
{
    auto num_bytes = (uint64_t)(num) * (uint64_t)(sizeof(uint32_t));
    // std::cout << target << " - MemWrite( peekpoke_bar_idx = " << std::to_string(peekpoke_bar_idx) << ", peekpoke_addr = 0x" << NumToStrHex(peekpoke_addr) << ", num = " << std::to_string(num) << " )" << std::endl << std::flush;
    if (MmioAccess(target, MMIO_CU_WRITE, peekpoke_addr, num_bytes, peekpoke_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_161, {"write", std::to_string(num_bytes), "to", NumToStrHex(peekpoke_addr), std::to_string(peekpoke_bar_idx), target, " MMIO access"});
        *(this->m_abort) = true;
        this->m_cl_fail = true;
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface::MemRead( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )
{
    auto num_bytes = (uint64_t)(num) * (uint64_t)(sizeof(uint32_t));
    // std::cout << target << " - MemRead( peekpoke_bar_idx = " << std::to_string(peekpoke_bar_idx) << ", peekpoke_addr = 0x" << NumToStrHex(peekpoke_addr) << ", num = " << std::to_string(num) << " )" << std::endl << std::flush;
    if (MmioAccess(target, MMIO_CU_READ, peekpoke_addr, (num_bytes), peekpoke_data) == RET_FAILURE)
    {
        LogMessage(MSG_ITF_161, {"read", std::to_string(num_bytes), "from", NumToStrHex(peekpoke_addr), std::to_string(peekpoke_bar_idx), target, "MMIO access"});
        *(this->m_abort) = true;
        this->m_cl_fail = true;
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface::GetUserBarInfo()
{
    std::string line;
    Bar_Info_t bar_info;

    // file "resource" exists if xrt is not installed
    std::ifstream resource_ifs(this->m_pci_dev_resource_file);
    if (!resource_ifs.good())
    {
        LogMessage(MSG_ITF_159, {"PCIe device resource file", this->m_pci_dev_resource_file, "failed to open node"});
        return RET_FAILURE;
    }

    this->m_bars_info.clear();
    while (std::getline(resource_ifs, line))
    {
        if (sscanf(line.c_str(), "0x%llx 0x%llx 0x%llx", &(bar_info.start_addr), &(bar_info.end_addr), &(bar_info.meta)) != 3)
        {
            LogMessage(MSG_ITF_165, {"PCIe device resource file", this->m_pci_dev_resource_file});
            return RET_FAILURE;
        }

        bar_info.size   = bar_info.end_addr - bar_info.start_addr + 1;
        bar_info.index  = this->m_bars_info.size();

        this->m_bars_info.emplace_back(bar_info);
    }

    if (!resource_ifs.eof())
    {
        LogMessage(MSG_ITF_165, {"PCIe device resource file", this->m_pci_dev_resource_file});
        return RET_FAILURE;
    }

    this->m_page_size = getpagesize();
    return RET_SUCCESS;
}

bool DeviceInterface::Open_Mmap ( DeviceInterface::Mmap_Info_t & mmap_info, const uint & access_type, const uint64_t & size )
{
    // address must be a multiple of the page size
    // mmap_info.offset = mmap_info.address & ((uint64_t)(this->m_page_size) - 1);
    // mmap_info.address &= ~ ((uint64_t)(this->m_page_size) - 1);
    mmap_info.offset   = mmap_info.address % (uint64_t)(this->m_page_size);
    mmap_info.address -= mmap_info.offset;
    mmap_info.length   = size + mmap_info.offset;

    int flags;
    if ((access_type == MMIO_CU_WRITE) || (access_type == MMIO_PLRAM_WRITE))
    {
        flags = O_RDWR;
    }
    else
    {
        flags = O_RDONLY | O_SYNC;
    }

    this->m_dev_mem_mtx.lock();

	mmap_info.fd = open64(m_dev_mem_fname.c_str(), flags);
	if (mmap_info.fd < 0)
    {
        std::string flags_str;
        if ((access_type == MMIO_CU_WRITE) || (access_type == MMIO_PLRAM_WRITE))
        {
            flags_str = "O_RDWR";
        }
        else
        {
            flags_str = "O_RDONLY | O_SYNC";
        }
        LogMessage(MSG_ITF_159, {"system's physical memory", m_dev_mem_fname + "(" + flags_str + ")", strerror(errno)});
        Close_Mmap(mmap_info);
		return RET_FAILURE;
	}

	int protection = 0;
	if ((flags&O_ACCMODE) == O_RDWR)
    {
		protection = PROT_READ | PROT_WRITE;
	}
    else if ((flags & O_ACCMODE) == O_RDONLY)
    {
		protection = PROT_READ;
	}
    else if  ((flags & O_ACCMODE) == O_WRONLY)
    {
		protection = PROT_WRITE;
	}

    // see MAP_HUGETLB
	mmap_info.mem = mmap(nullptr, mmap_info.length, protection, MAP_SHARED, mmap_info.fd, mmap_info.address);

	if (mmap_info.mem == MAP_FAILED)
    {
        LogMessage(MSG_ITF_160, {NumToStrHex(mmap_info.address), strerror(errno)});
		Close_Mmap(mmap_info);
		return RET_FAILURE;
	}

	return RET_SUCCESS;
}

bool DeviceInterface::Close_Mmap ( DeviceInterface::Mmap_Info_t & mmap_info )
{
    auto ret = RET_SUCCESS;
    if ((mmap_info.mem != nullptr) && (mmap_info.mem != MAP_FAILED))
    {
        if (munmap((void *)mmap_info.mem, mmap_info.length) != 0)
        {
            LogMessage(MSG_ITF_169, {NumToStrHex(mmap_info.address), strerror(errno)});
            ret = RET_FAILURE;
        }
	}
    if (mmap_info.fd >= 0)
    {
        if (close(mmap_info.fd) != 0)
        {
            LogMessage(MSG_ITF_170, {strerror(errno)});
            ret = RET_FAILURE;
        }
    }
    this->m_dev_mem_mtx.unlock();
    return ret;
}

bool DeviceInterface::MmioAccess ( const std::string & endpoint_name, const uint & access_type, const uint64_t & address, const uint64_t & size, uint32_t * data )
{
	Mmap_Info_t mmap_info;

	mmap_info.fd        = -1;
	mmap_info.mem       = nullptr;
	mmap_info.address   = address;

	if (Open_Mmap(mmap_info, access_type, size) == RET_FAILURE)
    {
        Close_Mmap(mmap_info);
        return RET_FAILURE;
	}

    // 32-bits accesses
    if ((access_type == MMIO_CU_WRITE))
    {
        for (uint64_t i = 0; i < size / sizeof(uint32_t); i++)
        {
            *((uint32_t *)((char*)(mmap_info.mem)+mmap_info.offset) + i) = data[i];
        }
    }
    else
    {
        for (uint64_t i = 0; i < size / sizeof(uint32_t); i++)
        {
            data[i] = *((uint32_t *)((char*)(mmap_info.mem)+mmap_info.offset) + i);
        }
    }

    if ((access_type == MMIO_CU_WRITE)) {
        LogMessage(MSG_DEBUG_OPENCL, {endpoint_name + " - MmioAccess write address 0x" + NumToStrHex(address) + ", data[0] = 0x" + NumToStrHex(data[0])});
    } else {
        LogMessage(MSG_DEBUG_OPENCL, {endpoint_name + " - MmioAccess read address 0x" + NumToStrHex(address) + ", data[0] = 0x" + NumToStrHex(data[0])});
    }

	return Close_Mmap(mmap_info);
}

} // namespace

#endif