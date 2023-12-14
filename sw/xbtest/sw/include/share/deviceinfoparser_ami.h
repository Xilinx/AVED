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

#ifndef _DEVICEINFOPARSERAMI_H
#define _DEVICEINFOPARSERAMI_H

#ifdef USE_AMI

// AMI includes
#include <ami.h>
#include <ami_device.h>
#include <ami_sensor.h>

#include "deviceinfoparser_base.h"

namespace xbtest
{

class DeviceInfoParser: public DeviceInfoParserBase
{

public:
    DeviceInfoParser( Logging * log, const std::string & bdf, std::atomic<bool> * abort );
    ~DeviceInfoParser();

    // List supported AMI sensor types: when adding new type here, also add new cases in switch(type)
    const std::vector<enum ami_sensor_type> SUPPORTED_AMI_SENSOR_TYPES = {AMI_SENSOR_TYPE_TEMP, AMI_SENSOR_TYPE_CURRENT, AMI_SENSOR_TYPE_VOLTAGE, AMI_SENSOR_TYPE_POWER};

    using AMI_Sensor_Cfg_t = struct AMI_Sensor_Cfg_t {
        std::string                 name;
        bool                        is_temperature;
        bool                        is_current;
        bool                        is_voltage;
        bool                        is_power;
        enum ami_sensor_unit_mod    unit_temperature;
        enum ami_sensor_unit_mod    unit_current;
        enum ami_sensor_unit_mod    unit_voltage;
        enum ami_sensor_unit_mod    unit_power;
    };

    const AMI_Sensor_Cfg_t RST_AMI_SENSOR_CFG = {
        .name                = "",
        .is_temperature      = false,
        .is_current          = false,
        .is_voltage          = false,
        .is_power            = false,
        .unit_temperature    = AMI_SENSOR_UNIT_MOD_NONE,
        .unit_current        = AMI_SENSOR_UNIT_MOD_NONE,
        .unit_voltage        = AMI_SENSOR_UNIT_MOD_NONE,
        .unit_power          = AMI_SENSOR_UNIT_MOD_NONE,
    };

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

    const uint16_t              SENSOR_REFRESH_RATE = 1000;

    std::vector<ami_device*>    m_ami_device;

    std::vector<AMI_Sensor_Cfg_t>   m_ami_sensors;

    const uint32_t              SENSOR_ERROR_REPORT_LIMIT = 5;
    uint32_t                    m_sensor_error_cnt = 0;

    bool            SetAmiSensorRefreshRate();
    bool            GetAMISensorType            ( const AMI_Sensor_Cfg_t & sensor_cfg, uint32_t & type );
    bool            GetAMISensorUnit            ( AMI_Sensor_Cfg_t & sensor_cfg, const enum ami_sensor_type & type );
    bool            GetAMISensorValue           ( AMI_Sensor_Cfg_t & sensor_cfg, const enum ami_sensor_type & type, int64_t & value, enum ami_sensor_status & status );
    bool            GetAMISensorList();

};

} // namespace

#endif

#endif /* _DEVICEINFOPARSERAMI_H */
