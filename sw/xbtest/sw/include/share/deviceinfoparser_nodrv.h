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

#ifndef _DEVICEINFOPARSERNODRV_H
#define _DEVICEINFOPARSERNODRV_H

#ifdef USE_NO_DRIVER

#include "deviceinfoparser_base.h"

namespace xbtest
{

class DeviceInfoParser: public DeviceInfoParserBase
{

public:
    DeviceInfoParser( Logging * log, const std::string & bdf, std::atomic<bool> * abort );
    ~DeviceInfoParser();

    bool            CreateDevice();
    void            ReleaseDevice();

    bool            CheckDriverAccess       () override;
    bool            GetNameInfo             ( std::string & name_info ) override;
    bool            GetDeviceState          ( Device_State_Info_t & device_state ) override;
    bool            GetLogicUuid            ( std::string & logic_uuid ) override;
    bool            GetInterfaceUuid        ( std::string & interface_uuid ) override;
    bool            GetNodmaInfo            ( bool & nodma_info ) override;
    bool            GetElectricalInfoInner  ( Electrical_Info_t & electrical_info ) override;
    bool            GetThermalInfoInner     ( Thermal_Info_t & thermal_info ) override;
    bool            GetMechanicalInfoInner  ( Mechanical_Info_t & mechanical_info ) override;
    bool            GetMemoryInfo           ( Memory_Info_t & memory_info ) override;
    bool            GetDesignInfo           ( Design_Info_t & design_info ) override;
    bool            GetHostInfo             ( Host_Info_t & host_info ) override;
    bool            GetPcieInfo             ( Pcie_Info_t & pcie_info ) override;
    bool            GetDynamicRegionsInfo   ( Dynamic_Regions_Info_t & dynamic_regions_info ) override;

private:


};

} // namespace

#endif

#endif /* _DEVICEINFOPARSERNODRV_H */
