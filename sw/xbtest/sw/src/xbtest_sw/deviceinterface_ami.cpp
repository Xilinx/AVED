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

#ifdef USE_AMI

#include <deviceinterface_ami.h>

namespace xbtest
{

DeviceInterface::DeviceInterface (
Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const  bool & p2p_target
) : DeviceInterface_PeekPoke( log, xbtest_sw_config, xbtest_sw_config_p2p, abort, p2p_target ) {}

//use origianl
DeviceInterface::~DeviceInterface() { ReleaseDevice(); }

// PDI download not supported when running with AMI or without driver, return succes by default
bool DeviceInterface::SetupDevice() { return RET_SUCCESS; }

uint64_t DeviceInterface::SetPeekPokeAddr( const uint64_t & addr, const uint8_t & bar )
{
    (void)bar; // The bar is not used to compute the address as this is handled by AMI
    return addr;
}

bool DeviceInterface::SelectDevice() {
    if (this->CreateAmiDevice() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface::MemWrite( const std::string & target, const uint8_t & peekpoke_bar_idx, const uint64_t & peekpoke_addr, const uint32_t & num, uint32_t * peekpoke_data )
{
    auto num_bytes = (uint64_t)(num) * (uint64_t)(sizeof(uint32_t));
    // std::cout << target << " - MemWrite( peekpoke_bar_idx = " << std::to_string(peekpoke_bar_idx) << ", peekpoke_addr = 0x" << NumToStrHex(peekpoke_addr) << ", num = " << std::to_string(num) << " )" << std::endl << std::flush;
    if (ami_mem_bar_write_range(this->m_ami_device[0], peekpoke_bar_idx, peekpoke_addr, num, peekpoke_data) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_161, {"write", std::to_string(num_bytes), "to", NumToStrHex(peekpoke_addr), std::to_string(peekpoke_bar_idx), target, "AMI. AMI error message: " + std::string(ami_get_last_error())});
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
    if (ami_mem_bar_read_range(this->m_ami_device[0], peekpoke_bar_idx, peekpoke_addr, num, peekpoke_data) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_161, {"read", std::to_string(num_bytes), "from", NumToStrHex(peekpoke_addr), std::to_string(peekpoke_bar_idx), target, "AMI. AMI error message: " + std::string(ami_get_last_error())});
        *(this->m_abort) = true;
        this->m_cl_fail = true;
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool DeviceInterface::CreateAmiDevice()
{
    if (this->m_ami_device.empty()) // Create only if not already created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Creating ami_device for card at " + this->m_device_bdf});

        ami_device *dev = nullptr;

        // Create AMI device without discovering sensors as another AMI device used for sensor is created in amother context
        if (ami_dev_find(this->m_device_bdf.c_str(), &dev) != AMI_STATUS_OK)
        {
            LogMessage(MSG_ITF_178, {std::string(ami_get_last_error())});
            return RET_FAILURE;
        }
        if (dev == nullptr)
        {
            LogMessage(MSG_ITF_178, {"No AMI error message available"});
            return RET_FAILURE;
        }

        // Following needed to run PCIe BAR peek poke AMI API without sudo
        if (ami_dev_request_access(dev) != AMI_STATUS_OK)
        {
            LogMessage(MSG_ITF_183, {std::string(ami_get_last_error())});
            return RET_FAILURE;
        }

        this->m_ami_device.emplace_back(dev);
    }
    return RET_SUCCESS;
}

void DeviceInterface::ReleaseDevice()
{
    if (!(this->m_ami_device.empty())) // release only if previously created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Releasing ami_device for card at " + this->m_device_bdf});
        for (auto & dev : this->m_ami_device)
        {
            if (dev != nullptr)
            {
                ami_dev_delete(&dev);
            }
        }
        this->m_ami_device.clear();
    }
}

} // namespace

#endif