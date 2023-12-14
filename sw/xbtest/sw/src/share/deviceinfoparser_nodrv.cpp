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

#ifdef USE_NO_DRIVER

#include <cmath>

#include "deviceinfoparser_nodrv.h"

namespace xbtest
{

DeviceInfoParser::DeviceInfoParser( Logging * log, const std::string & bdf, std::atomic<bool> * abort ) : DeviceInfoParserBase::DeviceInfoParserBase(log, bdf, abort) {}

DeviceInfoParser::~DeviceInfoParser()= default;

bool DeviceInfoParser::CheckDriverAccess()
{
    // No driver mode must be run as sudo
    if (!IsAdmin())
    {
        SetReportLevel(LOG_ERROR);
        LogMessage(MSG_ITF_158);
        SetReportLevel(LOG_CRIT_WARN);
        return RET_FAILURE;
    }
    SetReportLevel(LOG_CRIT_WARN);
    return RET_SUCCESS;
}

bool DeviceInfoParser::GetNameInfo( std::string & name_info )
{
    name_info = RST_NAME_INFO;
    name_info = "not available";
    return RetSuccess();
}

// Device state not supported
bool DeviceInfoParser::GetDeviceState( Device_State_Info_t & device_state )
{
    device_state = RST_DEVICE_STATE_INFO;
    return RetSuccess();
}

// Logic UUID is used for AVED
bool DeviceInfoParser::GetLogicUuid( std::string & logic_uuid )
{
    logic_uuid = RST_LOGIC_UUID;
    logic_uuid = "not available";
    return RetSuccess();
}

// Interface UUID not supported
bool DeviceInfoParser::GetInterfaceUuid( std::string & interface_uuid )
{
    interface_uuid = RST_INTERFACE_UUID;
    return RetSuccess();
}

bool DeviceInfoParser::GetNodmaInfo( bool & nodma_info )
{
    nodma_info = RST_NODMA_INFO;

    // Not used

    return RetSuccess();
}


bool DeviceInfoParser::GetElectricalInfoInner( Electrical_Info_t & electrical_info )
{
    electrical_info = RST_ELECTRICAL_INFO;

    // Not used

    return RET_SUCCESS;
}

bool DeviceInfoParser::GetThermalInfoInner( Thermal_Info_t & thermal_info )
{
    thermal_info = RST_THERMAL_INFO;

    // Not used

    return RET_SUCCESS;
}

bool DeviceInfoParser::GetMechanicalInfoInner( Mechanical_Info_t & mechanical_info )
{
    mechanical_info = RST_MECHANICAL_INFO;

    // Not used

    return RET_SUCCESS;
}

bool DeviceInfoParser::GetMemoryInfo( Memory_Info_t & memory_info )
{
    memory_info = RST_MEMORY_INFO;

    // Not used

    return RetSuccess();
}

bool DeviceInfoParser::GetDesignInfo( Design_Info_t & design_info )
{
    design_info = RST_DESIGN_INFO;

    // No design info supported yet, used default value

    return RetSuccess();
}

bool DeviceInfoParser::GetHostInfo( Host_Info_t & host_info )
{
    host_info = RST_HOST_INFO;

    // Not used

    return RetSuccess();
}

bool DeviceInfoParser::GetPcieInfo( Pcie_Info_t & pcie_info )
{
    pcie_info = RST_PCIE_INFO;

    // Used to configure default threshold of DMA / P2P not supported for USE_NO_DRIVER

    return RetSuccess();
}

bool DeviceInfoParser::GetDynamicRegionsInfo( Dynamic_Regions_Info_t & dynamic_regions_info )
{
    dynamic_regions_info = RST_DYNAMIC_REGIONS_INFO;

    // No dynamic region info supported yet, use default value
    // used only for display

    return RetSuccess();
}

} // namespace

#endif