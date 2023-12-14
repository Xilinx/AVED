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

#include <cmath>

#include "deviceinfoparser_ami.h"

namespace xbtest
{

DeviceInfoParser::DeviceInfoParser( Logging * log, const std::string & bdf, std::atomic<bool> * abort ) : DeviceInfoParserBase::DeviceInfoParserBase(log, bdf, abort)
{
    this->m_ami_sensors.clear();
}

DeviceInfoParser::~DeviceInfoParser() = default;

// Note: AMI device object used for to get device info is different than AMI device object used to communicate with card (xbtest HW, PLRAM, DMA, etc)
bool DeviceInfoParser::CreateDevice()
{
    if (this->m_ami_device.empty()) // Create only if not already created
    {
        ami_device *dev = nullptr;

        if (this->m_bdf == "0")
        {
            // Do not discover sensors without BDF
            LogMessage(MSG_DEBUG_SETUP, {"Creating ami_device without BDF (for first card found)"});
            if (ami_dev_find_next(&dev, AMI_ANY_DEV, AMI_ANY_DEV, AMI_ANY_DEV, nullptr) != AMI_STATUS_OK)
            {
                LogMessage(MSG_ITF_178, {std::string(ami_get_last_error())});
                return RET_FAILURE;
            }
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"Creating ami_device for card at " + this->m_bdf});
            if (ami_dev_bringup(this->m_bdf.c_str(), &dev) != AMI_STATUS_OK)
            {
                LogMessage(MSG_ITF_178, {std::string(ami_get_last_error())});
                return RET_FAILURE;
            }
        }
        if (dev == nullptr)
        {
            LogMessage(MSG_ITF_178, {"No AMI error message available"});
            return RET_FAILURE;
        }
        this->m_ami_device.emplace_back(dev);

        if (this->m_bdf != "0")
        {
            if (SetAmiSensorRefreshRate() == RET_FAILURE)
            {
                ReleaseDevice();
                return RET_FAILURE;
            }
        }
    }
    return RET_SUCCESS;
}

void DeviceInfoParser::ReleaseDevice()
{
    if (!(this->m_ami_device.empty())) // release only if previously created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Releasing ami_device for card at " + this->m_bdf});
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

bool DeviceInfoParser::SetAmiSensorRefreshRate()
{
    uint16_t refresh;
    if (ami_sensor_get_refresh(this->m_ami_device[0], &refresh) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_184, {"get", std::string(ami_get_last_error())});
        return RET_FAILURE;
    }
    LogMessage(MSG_DEBUG_SETUP, {"Current sensor refresh rate " + std::to_string(refresh)});

    if (refresh != SENSOR_REFRESH_RATE)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Overwriting sensor refresh rate from " + std::to_string(refresh) + " ms to " + std::to_string(SENSOR_REFRESH_RATE) + " second"});
        refresh = SENSOR_REFRESH_RATE;
        if (ami_sensor_set_refresh(this->m_ami_device[0], refresh) != AMI_STATUS_OK)
        {
            LogMessage(MSG_ITF_184, {"set", std::string(ami_get_last_error())});
            return RET_FAILURE;
        }
    }

    return RET_SUCCESS;
}

bool DeviceInfoParser::CheckDriverAccess()
{
    Host_Info_t host_info;
    if (GetHostInfo(host_info) == RET_FAILURE)
    {
        SetReportLevel(LOG_ERROR);
        LogMessage(MSG_ITF_185);
        SetReportLevel(LOG_CRIT_WARN);
        return RET_FAILURE;
    }
    SetReportLevel(LOG_CRIT_WARN);
    return RET_SUCCESS;
}

bool DeviceInfoParser::GetNameInfo( std::string & name_info )
{
    name_info = RST_NAME_INFO;

    char buf[AMI_DEV_NAME_SIZE];
    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }
    if (ami_dev_get_name(this->m_ami_device[0], buf) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"device name", std::string(ami_get_last_error())});
        return RetFailure();
    }
    name_info = std::string(buf);
    LogMessage(MSG_DEBUG_SETUP, {"AMI ami_dev_get_name " + name_info});
    return RetSuccess();
}

bool DeviceInfoParser::GetDeviceState( Device_State_Info_t & device_state )
{
    device_state = RST_DEVICE_STATE_INFO;

    device_state.expected_state = AMI_DEV_READY_STR;

    char buf[AMI_DEV_STATE_SIZE];
    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }
    if (ami_dev_get_state(this->m_ami_device[0], buf) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"device state", std::string(ami_get_last_error())});
        return RetFailure();
    }

    device_state.current_state = std::string(buf);
    return RetSuccess();
}

// Logic UUID is used for AVED
bool DeviceInfoParser::GetLogicUuid( std::string & logic_uuid )
{
    logic_uuid = RST_LOGIC_UUID;

    char buf[AMI_LOGIC_UUID_SIZE];
    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }
    if (ami_dev_read_uuid(this->m_ami_device[0], buf) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"logic UUID", std::string(ami_get_last_error())});
        return RetFailure();
    }

    logic_uuid = std::string(buf);
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

    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }

    // AMI does not support NoDMA, use RST_NODMA_INFO

    return RetSuccess();
}

bool DeviceInfoParser::GetAMISensorType( const AMI_Sensor_Cfg_t & sensor_cfg, uint32_t & type )
{
    type = (uint32_t)(AMI_SENSOR_TYPE_INVALID);
    // ami_sensor_get_type returns a sensor type value which may contain multiple AMI_SENSOR_TYPE
    if (ami_sensor_get_type(this->m_ami_device[0], sensor_cfg.name.c_str(), &type) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_173, {"type", sensor_cfg.name, std::string(ami_get_last_error())} );
        return RET_FAILURE;
    }
    if (type == AMI_SENSOR_TYPE_INVALID)
    {
        LogMessage(MSG_ITF_174, {sensor_cfg.name} );
    }
    return RET_SUCCESS;
}

bool DeviceInfoParser::GetAMISensorUnit( AMI_Sensor_Cfg_t & sensor_cfg, const enum ami_sensor_type & type )
{
    switch (type)
    {
        case AMI_SENSOR_TYPE_TEMP:
        {
            if (ami_sensor_get_temp_unit_mod(this->m_ami_device[0], sensor_cfg.name.c_str(), &(sensor_cfg.unit_temperature)) != AMI_STATUS_OK)
            {
                LogMessage(MSG_ITF_173, {"temperature unit multiplier", sensor_cfg.name, std::string(ami_get_last_error())} );
                return RET_FAILURE;
            }
            break;
        }
        case AMI_SENSOR_TYPE_CURRENT:
        {
            if (ami_sensor_get_current_unit_mod(this->m_ami_device[0], sensor_cfg.name.c_str(), &(sensor_cfg.unit_current)) != AMI_STATUS_OK)
            {
                LogMessage(MSG_ITF_173, {"current unit multiplier", sensor_cfg.name, std::string(ami_get_last_error())} );
                return RET_FAILURE;
            }
            break;
        }
        case AMI_SENSOR_TYPE_VOLTAGE:
        {
            if (ami_sensor_get_voltage_unit_mod(this->m_ami_device[0], sensor_cfg.name.c_str(), &(sensor_cfg.unit_voltage)) != AMI_STATUS_OK)
            {
                LogMessage(MSG_ITF_173, {"voltage unit multiplier", sensor_cfg.name, std::string(ami_get_last_error())} );
                return RET_FAILURE;
            }
            break;
        }
        case AMI_SENSOR_TYPE_POWER:
        {
            if (ami_sensor_get_power_unit_mod(this->m_ami_device[0], sensor_cfg.name.c_str(), &(sensor_cfg.unit_power)) != AMI_STATUS_OK)
            {
                LogMessage(MSG_ITF_173, {"power unit multiplier", sensor_cfg.name, std::string(ami_get_last_error())} );
                return RET_FAILURE;
            }
            break;
        }
        default:
        {
            // By design, function is not called for other senor types
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInfoParser::GetAMISensorValue( AMI_Sensor_Cfg_t & sensor_cfg, const enum ami_sensor_type & type, int64_t & value, enum ami_sensor_status & status )
{
    value  = 0;
    status = AMI_SENSOR_STATUS_INVALID;
    switch (type)
    {
        case AMI_SENSOR_TYPE_TEMP:
        {
            if (ami_sensor_get_temp_value(this->m_ami_device[0], sensor_cfg.name.c_str(), &value, &status) != AMI_STATUS_OK)
            {
                m_sensor_error_cnt++;
                if (m_sensor_error_cnt <= SENSOR_ERROR_REPORT_LIMIT)
                {
                    LogMessage(MSG_ITF_172, {"temperature value & status", sensor_cfg.name, std::string(ami_get_last_error())} );
                }
                return RET_FAILURE;
            }
            break;
        }
        case AMI_SENSOR_TYPE_CURRENT:
        {
            if (ami_sensor_get_current_value(this->m_ami_device[0], sensor_cfg.name.c_str(), &value, &status) != AMI_STATUS_OK)
            {
                m_sensor_error_cnt++;
                if (m_sensor_error_cnt <= SENSOR_ERROR_REPORT_LIMIT)
                {
                    LogMessage(MSG_ITF_172, {"current value & status", sensor_cfg.name, std::string(ami_get_last_error())} );
                }
                return RET_FAILURE;
            }
            break;
        }
        case AMI_SENSOR_TYPE_VOLTAGE:
        {
            if (ami_sensor_get_voltage_value(this->m_ami_device[0], sensor_cfg.name.c_str(), &value, &status) != AMI_STATUS_OK)
            {
                m_sensor_error_cnt++;
                if (m_sensor_error_cnt <= SENSOR_ERROR_REPORT_LIMIT)
                {
                    LogMessage(MSG_ITF_172, {"voltage value & status", sensor_cfg.name, std::string(ami_get_last_error())} );
                }
                return RET_FAILURE;
            }
            break;
        }
        case AMI_SENSOR_TYPE_POWER:
        {
            if (ami_sensor_get_power_value(this->m_ami_device[0], sensor_cfg.name.c_str(), &value, &status) != AMI_STATUS_OK)
            {
                m_sensor_error_cnt++;
                if (m_sensor_error_cnt <= SENSOR_ERROR_REPORT_LIMIT)
                {
                    LogMessage(MSG_ITF_172, {"power value & status", sensor_cfg.name, std::string(ami_get_last_error())} );
                }
                return RET_FAILURE;
            }
            break;
        }
        default:
        {
            // By design, function is not called for other senor types
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool DeviceInfoParser::GetAMISensorList()
{
    if (!(this->m_ami_sensors.empty())) // Get AMI Sensor list only if not already done
    {
        return RET_SUCCESS;
    }

    auto ret = RET_SUCCESS;
    int dummy = 0;
    struct ami_sensor *sensor_link_list = nullptr;
    if (ami_sensor_get_sensors(this->m_ami_device[0], &sensor_link_list, &dummy) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_179, {std::string(ami_get_last_error())});
        return RET_FAILURE;
    }

    if (sensor_link_list == nullptr)
    {
        LogMessage(MSG_ITF_177);
    }
    // recursively go through the link list of sensor
    while (sensor_link_list != nullptr)
    {
        auto sensor_cfg = RST_AMI_SENSOR_CFG;
        // Transfer the name into our internal structure
        sensor_cfg.name = std::string(sensor_link_list->name);

        auto    type    = (uint32_t)(AMI_SENSOR_TYPE_INVALID);
        auto    status  = AMI_SENSOR_STATUS_INVALID;
        int64_t value   = 0;

        if (GetAMISensorType(sensor_cfg, type) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (type != AMI_SENSOR_TYPE_INVALID)
        {
            // check for all sensor type, remove the type if detected
            // check at the end there is no other bit set high.
            //      if there are, it means that not all types have been decoded
            for (const auto & supported_ami_sensor_type : SUPPORTED_AMI_SENSOR_TYPES)
            {
                if ((type & supported_ami_sensor_type) == supported_ami_sensor_type)
                {
                    type &= (~supported_ami_sensor_type);
                    switch (supported_ami_sensor_type)
                    {
                        case AMI_SENSOR_TYPE_TEMP:      sensor_cfg.is_temperature   = true; break;
                        case AMI_SENSOR_TYPE_CURRENT:   sensor_cfg.is_current       = true; break;
                        case AMI_SENSOR_TYPE_VOLTAGE:   sensor_cfg.is_voltage       = true; break;
                        case AMI_SENSOR_TYPE_POWER:     sensor_cfg.is_power         = true; break;
                        default:                        break; // By design, default case should not be reached
                    }
                    if (GetAMISensorUnit(sensor_cfg, supported_ami_sensor_type) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    // check for status, use get_value as it checks status
                    if (GetAMISensorValue(sensor_cfg, supported_ami_sensor_type, value, status) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                }
            }

            if (type != 0)
            {
                LogMessage(MSG_ITF_176, {sensor_cfg.name, NumToStrHex(type,8)} );
            }

            this->m_ami_sensors.push_back(sensor_cfg);
        }
        sensor_link_list = sensor_link_list->next;
    }

    return ret;
}

// Do not return if a sensor does not exists, try to find other sensor
// The electrical/thermal/mechanical "get" sensor can report failure during xbtest test case but xbtest will abort if the first call does not return success during setup.

bool DeviceInfoParser::GetElectricalInfoInner( Electrical_Info_t & electrical_info )
{
    auto ret = RET_SUCCESS;
    electrical_info = RST_ELECTRICAL_INFO;

    // go through the list and check for power sensor. There are 2 type of power sensors
    //      1) pure power sensor
    //      2) power rails: current AND/OR voltage
    if (CreateDevice() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (GetAMISensorList() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    for (auto & sensor : this->m_ami_sensors)
    {
        int64_t ami_sensor_value = 0;
        enum ami_sensor_status ami_sensor_status_enum; // Assume AMI set sensor status to invalid when AMI API returns error code
        if (sensor.is_power)
        {
            auto info = RST_POWER_CONSUMPTION_INFO;
            info.id = sensor.name;

            if (GetAMISensorValue(sensor, AMI_SENSOR_TYPE_POWER, ami_sensor_value, ami_sensor_status_enum) == RET_FAILURE)
            {
                ret = RET_FAILURE;
            }
            info.is_present                 = true;
            info.sensor_status              = ConvertAmiStatusEnum2Str(ami_sensor_status_enum);
            info.power_consumption_watts    = (double)ami_sensor_value * (double)pow(10,sensor.unit_power);
            electrical_info.power_consumptions.emplace_back(info);
        }

        if (sensor.is_current || sensor.is_voltage)
        {
            auto info = RST_POWER_RAIL_INFO;
            info.id = sensor.name;
            if (sensor.is_current)
            {
                if (GetAMISensorValue(sensor, AMI_SENSOR_TYPE_CURRENT, ami_sensor_value, ami_sensor_status_enum) == RET_FAILURE)
                {
                    ret = RET_FAILURE;
                }
                info.current.is_present     = true;
                info.current.sensor_status  = ConvertAmiStatusEnum2Str(ami_sensor_status_enum);
                info.current.amps           = (double)ami_sensor_value * (double)pow(10,sensor.unit_current);
            }
            if (sensor.is_voltage)
            {
                if (GetAMISensorValue(sensor, AMI_SENSOR_TYPE_VOLTAGE, ami_sensor_value, ami_sensor_status_enum) == RET_FAILURE)
                {
                    ret = RET_FAILURE;
                }
                info.voltage.is_present     = true;
                info.voltage.sensor_status  = ConvertAmiStatusEnum2Str(ami_sensor_status_enum);
                info.voltage.volts          = (double)ami_sensor_value * (double)pow(10,sensor.unit_voltage);
            }
            electrical_info.power_rails.emplace_back(info);
        }
    }

    return ret;
}

bool DeviceInfoParser::GetThermalInfoInner( Thermal_Info_t & thermal_info )
{
    auto ret = RET_SUCCESS;
    thermal_info = RST_THERMAL_INFO;

    if (CreateDevice() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (GetAMISensorList() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    for (auto & sensor : this->m_ami_sensors)
    {
        int64_t ami_sensor_value = 0;
        enum ami_sensor_status ami_sensor_status_enum;
        if (sensor.is_temperature)
        {
            auto info = RST_THERMAL_EL_INFO;
            info.location_id = sensor.name;

            if (GetAMISensorValue(sensor, AMI_SENSOR_TYPE_TEMP, ami_sensor_value, ami_sensor_status_enum) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            info.is_present     = true;
            info.sensor_status  = ConvertAmiStatusEnum2Str(ami_sensor_status_enum);
            info.temp_c         = (double)ami_sensor_value * (double)pow(10,sensor.unit_temperature);
            thermal_info.thermals.emplace_back(info);
        }
    }

    return ret;
}

bool DeviceInfoParser::GetMechanicalInfoInner( Mechanical_Info_t & mechanical_info )
{
    auto ret = RET_SUCCESS;
    mechanical_info = RST_MECHANICAL_INFO;

    if (CreateDevice() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (GetAMISensorList() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // no mechanical info currently supported by AMI

    return ret;
}

bool DeviceInfoParser::GetMemoryInfo( Memory_Info_t & memory_info )
{
    memory_info = RST_MEMORY_INFO;

    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }

    // Not used

    return RetSuccess();
}

bool DeviceInfoParser::GetDesignInfo( Design_Info_t & design_info )
{
    design_info = RST_DESIGN_INFO;

    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }

    struct amc_version version;
    if (ami_dev_get_amc_version(this->m_ami_device[0], &version) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"AMC version", std::string(ami_get_last_error())});
        return RetFailure();
    }

    design_info.controller.amc.version = std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);
    design_info.controller.amc.expected_version = design_info.controller.amc.version; // Expected version reporting not supported yet in AMI so force expected to disable warning

    // use default value for other design info

    return RetSuccess();
}

bool DeviceInfoParser::GetHostInfo( Host_Info_t & host_info )
{
    host_info = RST_HOST_INFO;

    struct ami_version version;
    if (ami_get_driver_version(&version) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"driver version", std::string(ami_get_last_error())});
        return RetFailure();
    }
    host_info.version = std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);

    return RetSuccess();
}

bool DeviceInfoParser::GetPcieInfo( Pcie_Info_t & pcie_info )
{
    pcie_info = RST_PCIE_INFO;

    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }
    if (ami_dev_get_pci_link_speed(this->m_ami_device[0], (uint8_t *)(&(pcie_info.link_speed_gbit_sec)), (uint8_t *)(&(pcie_info.expected_link_speed_gbit_sec))) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"PCIe link speed", std::string(ami_get_last_error())});
        return RetFailure();
    }
    if (ami_dev_get_pci_link_width(this->m_ami_device[0], (uint8_t *)(&(pcie_info.express_lane_width_count)), (uint8_t *)(&(pcie_info.expected_express_lane_width_count))) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"PCIe link width", std::string(ami_get_last_error())});
        return RetFailure();
    }
    char buf[AMI_PCI_CPULIST_SIZE];
    if (ami_dev_get_pci_cpulist(this->m_ami_device[0], buf) != AMI_STATUS_OK)
    {
        LogMessage(MSG_ITF_180, {"PCIe CPU list", std::string(ami_get_last_error())});
        return RetFailure();
    }
    pcie_info.cpu_affinity = std::string(buf);

    return RetSuccess();
}

bool DeviceInfoParser::GetDynamicRegionsInfo( Dynamic_Regions_Info_t & dynamic_regions_info )
{
    dynamic_regions_info = RST_DYNAMIC_REGIONS_INFO;

    if (CreateDevice() == RET_FAILURE)
    {
        return RetFailure();
    }

    // No dynamic region info supported yet, use default value
    // used only for display

    return RetSuccess();
}

} // namespace

#endif