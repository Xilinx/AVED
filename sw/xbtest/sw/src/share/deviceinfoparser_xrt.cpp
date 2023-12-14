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

#ifdef USE_XRT

#include <cmath>

#include "deviceinfoparser_xrt.h"

namespace xbtest
{

DeviceInfoParser::DeviceInfoParser( Logging * log, const std::string & bdf, std::atomic<bool> * abort ) : DeviceInfoParserBase::DeviceInfoParserBase(log, bdf, abort) , XJsonParser::XJsonParser(log, abort)
{
    this->m_content_name  = DEVICE_INFO_API_JSON + bdf;
}

DeviceInfoParser::~DeviceInfoParser()
{
    ClearParser();
}

// Reimplement as both parent have them defined
void DeviceInfoParser::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    DeviceInfoParserBase::LogMessage(message, arg_list);
}

void DeviceInfoParser::LogMessage ( const Message_t & message )
{
    DeviceInfoParserBase::LogMessage(message);
}

// not used
bool DeviceInfoParser::Parse()
{
    return RET_SUCCESS;
}

bool DeviceInfoParser::CreateDevice()
{
    if (this->m_xrt_device.empty()) // Create only if not already created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Creating xrt::device for card at " + this->m_bdf});
        // Create device inside the try-catch block to able to catch exceptions (e.g. when card is reset)
        // Note if device is not released while a xbutil reset is in progress, then xbutil reset cannot terminate
        try
        {
            this->m_xrt_device.emplace_back(xrt::device(this->m_bdf));
        }
        catch (const std::exception& e)
        {
            LogMessage(MSG_ITF_134, {this->m_bdf, e.what()});
            ReleaseDevice();
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

void DeviceInfoParser::ReleaseDevice()
{
    if (!(this->m_xrt_device.empty())) // release only if previously created
    {
        LogMessage(MSG_DEBUG_SETUP, {"Releasing xrt::device for card at " + this->m_bdf});
        this->m_xrt_device.clear();
    }
}

bool DeviceInfoParser::CheckDriverAccess()
{
    Host_Info_t host_info;
    if (GetHostInfo(host_info) == RET_FAILURE)
    {
        DeviceInfoParserBase::SetReportLevel(LOG_ERROR);
        LogMessage(MSG_ITF_185);
        DeviceInfoParserBase::SetReportLevel(LOG_CRIT_WARN);
        return RET_FAILURE;
    }
    DeviceInfoParserBase::SetReportLevel(LOG_CRIT_WARN);
    return RET_SUCCESS;
}

bool DeviceInfoParser::GetNameInfo( std::string & name_info )
{
    name_info = RST_NAME_INFO;

    const auto info_enum = xrt::info::device::name;
    if (GetDeviceInfo<info_enum, std::string>(name_info) == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}

// Device state not supported
bool DeviceInfoParser::GetDeviceState( Device_State_Info_t & device_state )
{
    device_state = RST_DEVICE_STATE_INFO;
    return RetSuccess();
}

// Logic UUID not supported
bool DeviceInfoParser::GetLogicUuid( std::string & logic_uuid )
{
    logic_uuid = RST_LOGIC_UUID;

    return RetSuccess();
}

// Interface UUID is used for Vitis platform
bool DeviceInfoParser::GetInterfaceUuid( std::string & interface_uuid )
{
    interface_uuid = RST_INTERFACE_UUID;

    const auto info_enum = xrt::info::device::interface_uuid;
    xrt::uuid interface_uuid_info;
    if (GetDeviceInfo<info_enum, xrt::uuid>(interface_uuid_info) == RET_FAILURE)
    {
        return RetFailure();
    }
    interface_uuid = interface_uuid_info.to_string();

    return RetSuccess();
}

bool DeviceInfoParser::GetNodmaInfo( bool & nodma_info )
{
    nodma_info = RST_NODMA_INFO;

    const auto info_enum = xrt::info::device::nodma;
    if (GetDeviceInfo<info_enum, bool>(nodma_info) == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}

// Do not return if a sensor does not exists, try to find other sensor
// The electrical/thermal/mechanical "get" sensor can report failure during xbtest test case but xbtest will abort if the first call does not return success during setup.

bool DeviceInfoParser::GetElectricalInfoInner( Electrical_Info_t & electrical_info )
{
    auto ret = RET_SUCCESS;
    electrical_info = RST_ELECTRICAL_INFO;

    const auto info_enum    = xrt::info::device::electrical;
    auto info_str           = XrtInfoDeviceEnumToStr(info_enum);
    std::vector<std::string> node_title;
    std::string tmp_str;
    std::string info_json;
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    electrical_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    /*
        example of return:
        {
            "power_rails":
            [
                {
                    "id": "12v_aux",
                    "description": "12 Volts Auxillary",
                    "voltage":
                    {
                        "volts": "0.000",
                        "is_present": "false"
                    },
                    "current":
                    {
                        "amps": "0.000",
                        "is_present": "false"
                    }
                },
                ...
                {
                    ...
                }
            ],
            "power_consumption_max_watts": "75",
            "power_consumption_watts": "20.989838",
            "power_consumption_warning": "false"
        }
    */

    // power_rails info

    node_title = {POWER_RAILS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        ret = RET_FAILURE;
    }

    if (ret == RET_SUCCESS)
    {
        uint num_node_read = 0;
        ExtractNode(node_title, num_node_read); // Move cursor to power_rails array
        if (GetJsonNodeType() == JSON_NODE_ARRAY)
        {
            for (gint j = 0; j < count_json_elements(); j++) // For each element in POWER_RAILS array
            {
                read_json_element(j); // Move cursor to array j element

                auto sensor_ret = RET_SUCCESS;
                auto info = RST_POWER_RAIL_INFO;
                // Initialize sensor status to not present by default
                info.voltage.is_present = false;
                info.current.is_present = false;
                info.voltage.sensor_status = STATUS_SENSOR_NOT_APPLICABLE;
                info.current.sensor_status = STATUS_SENSOR_NOT_APPLICABLE;

                node_title = {ID};
                if (!NodeExists(node_title))
                {
                    PrintRequiredNotFound(true, info_str, node_title, info_json); // debug
                    ret = RET_FAILURE;
                    end_json_element(); // Move back cursor to array
                    continue;
                }
                if (ExtractNodeValueStr(node_title, info.id) == RET_FAILURE)
                {
                    PrintRequiredNotFound(true, info_str, node_title, info_json); // debug
                    ret = RET_FAILURE;
                    end_json_element(); // Move back cursor to array
                    continue;
                }

                node_title = {VOLTAGE};
                if (NodeExists(node_title))
                {
                    node_title = {VOLTAGE, IS_PRESENT};
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                        sensor_ret = RET_FAILURE;
                    }
                    if (sensor_ret == RET_SUCCESS)
                    {
                        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                        {
                            PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                            sensor_ret = RET_FAILURE;
                        }
                    }
                    if (sensor_ret == RET_SUCCESS)
                    {
                        if (StrToBool(tmp_str, info.voltage.is_present) == RET_FAILURE)
                        {
                            PrintFailedToConvert(true, info.id + " " + info_str, node_title, info_json); // debug
                            sensor_ret = RET_FAILURE;
                        }
                    }

                    node_title = {VOLTAGE, VOLTS};
                    if (info.voltage.is_present)
                    {
                        info.voltage.sensor_status = STATUS_SENSOR_INVALID; // Sensor is reported present, initialize status to invalid
                        if (sensor_ret == RET_SUCCESS)
                        {
                            if (!NodeExists(node_title))
                            {
                                PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                                sensor_ret = RET_FAILURE;
                            }
                        }
                        if (sensor_ret == RET_SUCCESS)
                        {
                            if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                            {
                                PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                                sensor_ret = RET_FAILURE;
                            }
                        }
                        if (sensor_ret == RET_SUCCESS)
                        {
                            if (ConvString2Num<double>(tmp_str, info.voltage.volts) == RET_FAILURE)
                            {
                                PrintFailedToConvert(true, info.id + " " + info_str, node_title, info_json); // debug
                                sensor_ret = RET_FAILURE;
                            }
                        }
                        if (sensor_ret == RET_SUCCESS)
                        {
                            info.voltage.sensor_status = STATUS_SENSOR_OK;
                        }
                    }
                    else
                    {
                        if (sensor_ret == RET_SUCCESS)
                        {
                            info.voltage.sensor_status = STATUS_SENSOR_NOT_PRESENT; // Sensor is valid but reported as not present
                        }
                    }

                    if (sensor_ret == RET_FAILURE)
                    {
                        ret = RET_FAILURE;
                    }
                }

                sensor_ret = RET_SUCCESS;

                node_title = {CURRENT};
                if (NodeExists(node_title))
                {
                    node_title = {CURRENT, IS_PRESENT};
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                        sensor_ret = RET_FAILURE;
                    }
                    if (sensor_ret == RET_SUCCESS)
                    {
                        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                        {
                            PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                            sensor_ret = RET_FAILURE;
                        }
                    }
                    if (sensor_ret == RET_SUCCESS)
                    {
                        if (StrToBool(tmp_str, info.current.is_present) == RET_FAILURE)
                        {
                            PrintFailedToConvert(true, info.id + " " + info_str, node_title, info_json); // debug
                            sensor_ret = RET_FAILURE;
                        }
                    }
                    node_title = {CURRENT, AMPS};
                    if (info.current.is_present)
                    {
                        info.current.sensor_status = STATUS_SENSOR_INVALID; // Sensor is reported present, initialize status to invalid
                        if (sensor_ret == RET_SUCCESS)
                        {
                            if (!NodeExists(node_title))
                            {
                                PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                                sensor_ret = RET_FAILURE;
                            }
                        }
                        if (sensor_ret == RET_SUCCESS)
                        {
                            if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                            {
                                PrintRequiredNotFound(true, info.id + " " + info_str, node_title, info_json); // debug
                                sensor_ret = RET_FAILURE;
                            }
                        }
                        if (sensor_ret == RET_SUCCESS)
                        {
                            if (ConvString2Num<double>(tmp_str, info.current.amps) == RET_FAILURE)
                            {
                                PrintFailedToConvert(true, info.id + " " + info_str, node_title, info_json); // debug
                                sensor_ret = RET_FAILURE;
                            }
                        }
                        if (sensor_ret == RET_SUCCESS)
                        {
                            info.current.sensor_status = STATUS_SENSOR_OK;
                        }
                    }
                    else
                    {
                        if (sensor_ret == RET_SUCCESS)
                        {
                            info.current.sensor_status = STATUS_SENSOR_NOT_PRESENT; // Sensor is valid but reported as not present
                        }
                    }

                    if (sensor_ret == RET_FAILURE)
                    {
                        ret = RET_FAILURE;
                    }
                }
                electrical_info.power_rails.emplace_back(info);

                end_json_element(); // Move back cursor to array
            }
        }
        for (uint ii = 0; ii < num_node_read; ii++) // Move cursor back from array
        {
            end_json_element();
        }
    }

    // power_consumption info
    auto sensor_ret = RET_SUCCESS;
    auto info = RST_POWER_CONSUMPTION_INFO;
    info.id = ID_POWER_CONSUMPTION;
    // Initialize sensor status to not present by default
    info.is_present = true;
    info.sensor_status = STATUS_SENSOR_NOT_PRESENT;

    node_title = {POWER_CONSUMPTION_WATTS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        sensor_ret = RET_FAILURE;
    }
    if (sensor_ret == RET_SUCCESS)
    {
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(false, info_str, node_title, info_json);
            sensor_ret = RET_FAILURE;
        }
    }
    if (sensor_ret == RET_SUCCESS)
    {
        info.sensor_status = STATUS_SENSOR_INVALID; // Sensor is is found, initialize status to invalid
        if (ConvString2Num<double>(tmp_str, info.power_consumption_watts) == RET_FAILURE)
        {
            PrintFailedToConvert(false, info_str, node_title, info_json);
            sensor_ret = RET_FAILURE;
        }
    }
    if (sensor_ret == RET_SUCCESS)
    {
        info.sensor_status = STATUS_SENSOR_OK;
    }
    else
    {
        ret = RET_FAILURE;
    }
    electrical_info.power_consumptions.emplace_back(info);

    return ret;
}

bool DeviceInfoParser::GetThermalInfoInner( Thermal_Info_t & thermal_info )
{
    auto ret = RET_SUCCESS;
    thermal_info = RST_THERMAL_INFO;

    const auto info_enum    = xrt::info::device::thermal;
    auto info_str           = XrtInfoDeviceEnumToStr(info_enum);
    std::vector<std::string> node_title;
    std::string tmp_str;
    std::string info_json;
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    thermal_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    /*
        example of return:
        {
            "thermals":
            [
                {
                    "location_id": "pcb_top_front",
                    "description": "PCB Top Front",
                    "temp_C": "40",
                    "is_present": "true"
                },
                ...
                {
                    ...
                }
            ]
        }
    */
    node_title = {THERMALS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RET_FAILURE;
    }

    uint num_node_read = 0;
    ExtractNode(node_title, num_node_read); // Move cursor to thermals array

    if (GetJsonNodeType() == JSON_NODE_ARRAY)
    {
        for (gint j = 0; j < count_json_elements(); j++) // For each element in thermals array
        {
            read_json_element(j); // Move cursor to array j element

            auto sensor_ret = RET_SUCCESS;
            auto info = RST_THERMAL_EL_INFO;
            // Initialize sensor status to not present by default
            info.is_present = false;
            info.sensor_status = STATUS_SENSOR_NOT_APPLICABLE;

            node_title = {LOCATION_ID};
            if (!NodeExists(node_title))
            {
                PrintRequiredNotFound(true, info_str, node_title, info_json); // debug
                ret = RET_FAILURE;
                end_json_element(); // Move back cursor to array
                continue;
            }
            if (sensor_ret == RET_SUCCESS)
            {
                if (ExtractNodeValueStr(node_title, info.location_id) == RET_FAILURE)
                {
                    PrintRequiredNotFound(true, info_str, node_title, info_json); // debug
                    ret = RET_FAILURE;
                    end_json_element(); // Move back cursor to array
                    continue;
                }
            }

            node_title = {IS_PRESENT};
            if (sensor_ret == RET_SUCCESS)
            {
                if (!NodeExists(node_title))
                {
                    PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json); // debug
                    sensor_ret = RET_FAILURE;
                }
            }
            if (sensor_ret == RET_SUCCESS)
            {
                if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                {
                    PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json); // debug
                    sensor_ret = RET_FAILURE;
                }
            }
            if (sensor_ret == RET_SUCCESS)
            {
                if (StrToBool(tmp_str, info.is_present) == RET_FAILURE)
                {
                    PrintFailedToConvert(true, info.location_id + " " + info_str, node_title, info_json); // debug
                    sensor_ret = RET_FAILURE;
                }
            }

            node_title = {TEMP_C};
            if (info.is_present)
            {
                info.sensor_status = STATUS_SENSOR_INVALID; // Sensor is reported present, initialize status to invalid

                if (sensor_ret == RET_SUCCESS)
                {
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json);
                        sensor_ret = RET_FAILURE;
                    }
                }
                if (sensor_ret == RET_SUCCESS)
                {
                    if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                    {
                        PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json);
                        sensor_ret = RET_FAILURE;
                    }
                }
                if (sensor_ret == RET_SUCCESS)
                {
                    if (ConvString2Num<double>(tmp_str, info.temp_c) == RET_FAILURE)
                    {
                        PrintFailedToConvert(true, info.location_id + " " + info_str, node_title, info_json);
                        sensor_ret = RET_FAILURE;
                    }
                }
                if (sensor_ret == RET_SUCCESS)
                {
                    info.sensor_status = STATUS_SENSOR_OK;
                }
            }
            else
            {
                if (sensor_ret == RET_SUCCESS)
                {
                    info.sensor_status = STATUS_SENSOR_NOT_PRESENT; // Sensor is valid but reported as not present
                }
            }

            if (sensor_ret == RET_FAILURE)
            {
                ret = RET_FAILURE;
            }

            thermal_info.thermals.emplace_back(info);

            end_json_element(); // Move back cursor to array
        }
    }
    for (uint ii = 0; ii < num_node_read; ii++) // Move cursor back from array
    {
        end_json_element();
    }

    return ret;
}

bool DeviceInfoParser::GetMechanicalInfoInner( Mechanical_Info_t & mechanical_info )
{
    auto ret = RET_SUCCESS;
    mechanical_info = RST_MECHANICAL_INFO;

    const auto info_enum    = xrt::info::device::mechanical;
    auto info_str           = XrtInfoDeviceEnumToStr(info_enum);
    std::vector<std::string> node_title;
    std::string tmp_str;
    std::string info_json;
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    mechanical_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    /*
        example of return:
        {
            "fans":
            [
                {
                    "location_id": "fpga_fan_1",
                    "description": "FPGA Fan 1",
                    "critical_trigger_temp_C": "39",
                    "speed_rpm": "0",
                    "is_present": "false"
                }
            ]
        }
    */

    node_title = {FANS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RET_FAILURE;
    }

    uint num_node_read = 0;
    ExtractNode(node_title, num_node_read); // Move cursor to fans array

    if (GetJsonNodeType() == JSON_NODE_ARRAY)
    {
        for (gint j = 0; j < count_json_elements(); j++) // For each element in fans array
        {
            read_json_element(j); // Move cursor to array j element

            auto sensor_ret = RET_SUCCESS;
            auto info = RST_FAN_INFO;
            // Initialize sensor status to not present by default
            info.is_present = false;
            info.sensor_status = STATUS_SENSOR_NOT_APPLICABLE;

            node_title = {LOCATION_ID};
            if (!NodeExists(node_title))
            {
                PrintRequiredNotFound(true, info_str, node_title, info_json); // debug
                ret = RET_FAILURE;
                end_json_element(); // Move back cursor to array
                continue;
            }
            if (ExtractNodeValueStr(node_title, info.location_id) == RET_FAILURE)
            {
                PrintRequiredNotFound(true, info_str, node_title, info_json); // debug
                ret = RET_FAILURE;
                end_json_element(); // Move back cursor to array
                continue;
            }

            node_title = {IS_PRESENT};
            if (sensor_ret == RET_SUCCESS)
            {
                if (!NodeExists(node_title))
                {
                    PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json); // debug
                    sensor_ret = RET_FAILURE;
                }
            }
            if (sensor_ret == RET_SUCCESS)
            {
                if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                {
                    PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json); // debug
                    sensor_ret = RET_FAILURE;
                }
            }
            if (sensor_ret == RET_SUCCESS)
            {
                if (StrToBool(tmp_str, info.is_present) == RET_FAILURE)
                {
                    PrintFailedToConvert(true, info.location_id + " " + info_str, node_title, info_json);
                    sensor_ret = RET_FAILURE;
                }
            }

            node_title = {SPEED_RPM};
            if (info.is_present)
            {
                info.sensor_status = STATUS_SENSOR_INVALID; // Sensor is reported present, initialize status to invalid

                if (sensor_ret == RET_SUCCESS)
                {
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json); // debug
                        sensor_ret = RET_FAILURE;
                    }
                }
                if (sensor_ret == RET_SUCCESS)
                {
                    if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
                    {
                        PrintRequiredNotFound(true, info.location_id + " " + info_str, node_title, info_json); // debug
                        sensor_ret = RET_FAILURE;
                    }
                }
                if (sensor_ret == RET_SUCCESS)
                {
                    if (ConvString2Num<double>(tmp_str, info.speed_rpm) == RET_FAILURE)
                    {
                        PrintFailedToConvert(true, info.location_id + " " + info_str, node_title, info_json); // debug
                        sensor_ret = RET_FAILURE;
                    }
                }

                if (sensor_ret == RET_SUCCESS)
                {
                    info.sensor_status = STATUS_SENSOR_OK;
                }
            }
            else
            {
                if (sensor_ret == RET_SUCCESS)
                {
                    info.sensor_status = STATUS_SENSOR_NOT_PRESENT; // Sensor is valid but reported as not present
                }
            }

            if (sensor_ret == RET_FAILURE)
            {
                ret = RET_FAILURE;
            }

            mechanical_info.fans.emplace_back(info);

            end_json_element(); // Move back cursor to array
        }
    }
    for (uint ii = 0; ii < num_node_read; ii++) // Move cursor back from array
    {
        end_json_element();
    }
    return ret;
}

bool DeviceInfoParser::GetMemoryInfo( Memory_Info_t & memory_info )
{
    memory_info = RST_MEMORY_INFO;
    auto info_str = XRT_INFO_DEVICE_MEMORY;
    std::vector<std::string> node_title;
    std::string tmp_str;
    std::string info_json;

    const auto info_enum    = xrt::info::device::memory;
    // auto info_str           = XrtInfoDeviceEnumToStr(info_enum);
    info_str           = XrtInfoDeviceEnumToStr(info_enum);
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RetFailure();
    }

    memory_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RetFailure();
    }
    /*
        example of return:
        {
            "board":
            {
                "direct_memory_accesses":
                {
                    "type": "pcie xdma",
                    "metrics":
                    [
                        {
                            "channel_id": "0",
                            "host_to_card_bytes": "0x1b480",
                            "card_to_host_bytes": "0x31140"
                        },
                        {
                            "channel_id": "1",
                            "host_to_card_bytes": "0x0",
                            "card_to_host_bytes": "0x0"
                        }
                    ]
                },
                "memory":
                {
                    "data_streams": "",
                    "memories":
                    [
                        {
                            "error_msg": "Failed to find subdirectory for HBM[0] under \/sys\/bus\/pci\/devices\/0000:86:00.1\n",
                            "type": "MEM_HBM",
                            "tag": "HBM[0]",
                            "enabled": "true",
                            "base_address": "0x0",
                            "range_bytes": "0x10000000",
                            "extended_info":
                            {
                                "usage":
                                {
                                    "allocated_bytes": "0",
                                    "buffer_objects_count": "0"
                                },
                                "temperature_C": "47"
                            }
                        },
                        ...
                        {
                            "type": "MEM_DRAM",
                            "tag": "PLRAM[3]",
                            "enabled": "false",
                            "base_address": "0x201400000",
                            "range_bytes": "0x20000",
                            "extended_info":
                            {
                                "usage":
                                {
                                    "allocated_bytes": "0",
                                    "buffer_objects_count": "0"
                                }
                            }
                        }
                    ]
                }
            },
            "error_msg": "Failed to open \/sys\/bus\/pci\/devices\/0000:86:00.1\/\/mig_cache_update for writing: Permission denied\n"
        }
    */

    // memory
    node_title = {board, MEMORY, MEMORIES};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }

    uint num_node_read = 0;
    ExtractNode(node_title, num_node_read); // Move cursor to memories array

    if (GetJsonNodeType() == JSON_NODE_ARRAY)
    {
        for (gint j = 0; j < count_json_elements(); j++) // For each element in memories array
        {
            read_json_element(j); // Move cursor to array j element

            auto info = RST_MEM_INFO;

            info.mem_data_idx = j;

            node_title = {BASE_ADDRESS};
            if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
            {
                PrintRequiredNotFound(false, info_str, node_title, info_json);
                return RetFailure();
            }
            if (ConvStringHex2Num<uint64_t>(tmp_str, info.base_address) == RET_FAILURE)
            {
                PrintFailedToConvert(false, info_str, node_title, info_json);
                return RetFailure();
            }
            node_title = {RANGE_BYTES};
            if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
            {
                PrintRequiredNotFound(false, info_str, node_title, info_json);
                return RetFailure();
            }
            if (ConvStringHex2Num<uint64_t>(tmp_str, info.range_bytes) == RET_FAILURE)
            {
                PrintFailedToConvert(false, info_str, node_title, info_json);
                return RetFailure();
            }
            memory_info.memory.memories.emplace_back(info);

            end_json_element(); // Move back cursor to array
        }
    }
    for (uint ii = 0; ii < num_node_read; ii++) // Move cursor back from array
    {
        end_json_element();
    }

    return RetSuccess();
}

bool DeviceInfoParser::GetDesignInfo( Design_Info_t & design_info )
{
    design_info = RST_DESIGN_INFO;
    auto info_str = XRT_INFO_DEVICE_PLATFORM;
    std::vector<std::string> node_title;
    std::string info_json;

    const auto info_enum    = xrt::info::device::platform;
    // auto info_str           = XrtInfoDeviceEnumToStr(info_enum);
    info_str           = XrtInfoDeviceEnumToStr(info_enum);
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RetFailure();
    }

    design_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RetFailure();
    }
    /*
        example of return:
        {
            "platforms":
            [
                {
                    "static_region":
                    {
                        "vbnv": "xilinx_u50lv_gen3x4_xdma_base_2",
                        "interface_uuid": "05A5E9D4-E079-740E-76C7-499FEEC81DB3",
                        "jtag_idcode": "0x14b77093",
                        "fpga_name": ""
                    },
                    "off_chip_board_info":
                    {
                        "ddr_size_bytes": "0",
                        "ddr_count": "0"
                    },
                    "status":
                    {
                        "mig_calibrated": "true",
                        "p2p_status": "not supported" / "p2p_status": "enabled"
                    },
                    "controller":
                    {
                        "satellite_controller":
                        {
                            "version": "5.2.15",
                            "expected_version": "5.2.15"
                        },
                        "card_mgmt_controller":
                        {
                            "version": "201392160",
                            "serial_number": "501211101A46",
                            "oem_id": "Xilinx"
                        }
                    },
                    "clocks":
                    [
                        {
                            "id": "DATA_CLK",
                            "description": "Data",
                            "freq_mhz": "250"
                        },
                        {
                            "id": "KERNEL_CLK",
                            "description": "Kernel",
                            "freq_mhz": "500"
                        },
                        {
                            "id": "hbm_aclk",
                            "description": "N\/A",
                            "freq_mhz": "400"
                        }
                    ],
                    "macs":
                    [
                        {
                            "address": "00:0A:35:06:7B:2E"
                        },
                        {
                            "address": "00:0A:35:06:7B:2F"
                        },
                        {
                            "address": "00:0A:35:06:7B:30"
                        },
                        {
                            "address": "00:0A:35:06:7B:31"
                        }
                    ]
                }
            ]
        }
    */

    node_title = {PLATFORMS};
    if (NodeExists(node_title))
    {
        uint num_node_read_0 = 0;
        ExtractNode(node_title, num_node_read_0); // Move cursor to platforms array

        if (GetJsonNodeType() == JSON_NODE_ARRAY)
        {
            if (count_json_elements() > 0)
            {
                read_json_element(0); // Move cursor to array 0 element

                // p2p_status info
                node_title = {STATUS, P2P_STATUS};
                if (ExtractNodeValueStr(node_title, design_info.status.p2p_status) == RET_FAILURE)
                {
                    PrintRequiredNotFound(false, info_str, node_title, info_json);
                    return RetFailure();
                }
                // satellite_controller info
                node_title = {CONTROLLER, SATELLITE_CONTROLLER, VERSION};
                if (ExtractNodeValueStr(node_title, design_info.controller.satellite_controller.version) == RET_FAILURE)
                {
                    PrintRequiredNotFound(false, info_str, node_title, info_json);
                    return RetFailure();
                }
                node_title = {CONTROLLER, SATELLITE_CONTROLLER, EXPECTED_VERSION};
                if (ExtractNodeValueStr(node_title, design_info.controller.satellite_controller.expected_version) == RET_FAILURE)
                {
                    PrintRequiredNotFound(false, info_str, node_title, info_json);
                    return RetFailure();
                }
                // macs info
                node_title = {MACS};
                if (NodeExists(node_title))
                {
                    uint num_node_read = 0;
                    ExtractNode(node_title, num_node_read); // Move cursor to macs array

                    for (gint j = 0; j < count_json_elements(); j++) // For each element in macs array
                    {
                        read_json_element(j); // Move cursor to array j element

                        auto info = RST_DESIGN_MAC_INFO;

                        node_title = {ADDRESS};
                        if (ExtractNodeValueStr(node_title, info.address) == RET_FAILURE)
                        {
                            PrintRequiredNotFound(false, info_str, node_title, info_json);
                            return RetFailure();
                        }

                        design_info.macs.emplace_back(info);

                        end_json_element(); // Move back cursor to array
                    }
                    for (uint ii = 0; ii < num_node_read; ii++) // Move cursor back from array
                    {
                        end_json_element();
                    }
                }

                end_json_element(); // Move back cursor to array
            }
        }
        for (uint ii = 0; ii < num_node_read_0; ii++) // Move cursor back from array
        {
            end_json_element();
        }
    }
    return RetSuccess();
}

bool DeviceInfoParser::GetHostInfo( Host_Info_t & host_info )
{
    host_info = RST_HOST_INFO;

    std::vector<std::string> node_title;

    const auto  info_enum = xrt::info::device::host;
    auto        info_str  = XrtInfoDeviceEnumToStr(info_enum);
    std::string info_json;
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RetFailure();
    }

    host_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RetFailure();
    }
    /*
        example of return:
        {
            "version": "2.12.275",
            "branch": "master",
            "hash": "50eca7496740f38a299393838abb207bceb2e519",
            "build_date": "2021-08-24 06:39:05"
        }
    */

    node_title = {VERSION};
    if (ExtractNodeValueStr(node_title, host_info.version) == RET_FAILURE)
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }

    return RetSuccess();
}

bool DeviceInfoParser::GetPcieInfo( Pcie_Info_t & pcie_info )
{
    pcie_info = RST_PCIE_INFO;

    std::vector<std::string> node_title;
    std::string tmp_str;

    const auto  info_enum = xrt::info::device::pcie_info;
    auto        info_str = XrtInfoDeviceEnumToStr(info_enum);
    std::string info_json;
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RetFailure();
    }

    pcie_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RetFailure();
    }
    /*
        example of return:
        {
            "vendor": "0x10ee",
            "device": "0x5061",
            "sub_device": "0x000e",
            "sub_vendor": "0x10ee",
            "link_speed_gbit_sec": "3",
            "expected_link_speed_gbit_sec": "3",
            "express_lane_width_count": "4",
            "expected_express_lane_width_count": "4",
            "dma_thread_count": "2",
            "cpu_affinity": "1,3,5,7,9,11",
            "max_shared_host_mem_aperture_bytes": "0"
        }
    */

    node_title = {LINK_SPEED_GBIT_SEC};
    if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }
    if (ConvString2Num<uint>(tmp_str, pcie_info.link_speed_gbit_sec) == RET_FAILURE)
    {
        PrintFailedToConvert(false, info_str, node_title, info_json);
        return RetFailure();
    }
    node_title = {EXPECTED_LINK_SPEED_GBIT_SEC};
    if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }
    if (ConvString2Num<uint>(tmp_str, pcie_info.expected_link_speed_gbit_sec) == RET_FAILURE)
    {
        PrintFailedToConvert(false, info_str, node_title, info_json);
        return RetFailure();
    }
    node_title = {EXPRESS_LANE_WIDTH_COUNT};
    if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }
    if (ConvString2Num<uint>(tmp_str, pcie_info.express_lane_width_count) == RET_FAILURE)
    {
        PrintFailedToConvert(false, info_str, node_title, info_json);
        return RetFailure();
    }
    node_title = {EXPECTED_EXPRESS_LANE_WIDTH_COUNT};
    if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }
    if (ConvString2Num<uint>(tmp_str, pcie_info.expected_express_lane_width_count) == RET_FAILURE)
    {
        PrintFailedToConvert(false, info_str, node_title, info_json);
        return RetFailure();
    }
    node_title = {CPU_AFFINITY};
    if (ExtractNodeValueStr(node_title, pcie_info.cpu_affinity) == RET_FAILURE)
    {
        PrintRequiredNotFound(false, info_str, node_title, info_json);
        return RetFailure();
    }

    return RetSuccess();
}

bool DeviceInfoParser::GetDynamicRegionsInfo( Dynamic_Regions_Info_t & dynamic_regions_info )
{
    dynamic_regions_info = RST_DYNAMIC_REGIONS_INFO;
    auto info_str = XRT_INFO_DEVICE_DYNAMIC_REGIONS;
    std::vector<std::string> node_title;
    std::string info_json;

    const auto info_enum    = xrt::info::device::dynamic_regions;
    // auto info_str           = XrtInfoDeviceEnumToStr(info_enum);
    info_str           = XrtInfoDeviceEnumToStr(info_enum);
    if (GetDeviceInfo<info_enum, std::string>(info_json) == RET_FAILURE)
    {
        return RetFailure();
    }

    dynamic_regions_info.info_json = info_json;
    if (LoadStringJSON(info_str, info_json) == RET_FAILURE)
    {
        return RetFailure();
    }
    /*
        example of return:
        {
            "xclbin_uuid": "10003F59-5328-684F-2D6B-C1AD9ED9DEAC"
        }
    */

    return RetSuccess();
}


std::string DeviceInfoParser::XrtInfoDeviceEnumToStr( const xrt::info::device info_enum )
{
    auto info_str = UNKNOWN;
    switch (info_enum)
    {
        case xrt::info::device::bdf                     : info_str = XRT_INFO_DEVICE_BDF;                       break;
        case xrt::info::device::interface_uuid          : info_str = XRT_INFO_DEVICE_INTERFACE_UUID;            break;
        case xrt::info::device::kdma                    : info_str = XRT_INFO_DEVICE_KDMA;                      break;
        case xrt::info::device::max_clock_frequency_mhz : info_str = XRT_INFO_DEVICE_MAX_CLOCK_FREQUENCY_MHZ;   break;
        case xrt::info::device::m2m                     : info_str = XRT_INFO_DEVICE_M2M;                       break;
        case xrt::info::device::name                    : info_str = XRT_INFO_DEVICE_NAME;                      break;
        case xrt::info::device::nodma                   : info_str = XRT_INFO_DEVICE_NODMA;                     break;
        case xrt::info::device::offline                 : info_str = XRT_INFO_DEVICE_OFFLINE;                   break;
        case xrt::info::device::electrical              : info_str = XRT_INFO_DEVICE_ELECTRICAL;                break;
        case xrt::info::device::thermal                 : info_str = XRT_INFO_DEVICE_THERMAL;                   break;
        case xrt::info::device::mechanical              : info_str = XRT_INFO_DEVICE_MECHANICAL;                break;
        case xrt::info::device::memory                  : info_str = XRT_INFO_DEVICE_MEMORY;                    break;
        case xrt::info::device::platform                : info_str = XRT_INFO_DEVICE_PLATFORM;                  break;
        case xrt::info::device::pcie_info               : info_str = XRT_INFO_DEVICE_PCIE_INFO;                 break;
        case xrt::info::device::host                    : info_str = XRT_INFO_DEVICE_HOST;                      break;
        case xrt::info::device::dynamic_regions         : info_str = XRT_INFO_DEVICE_DYNAMIC_REGIONS;           break;
        default: break;
    }
    return info_str;
}

void DeviceInfoParser::PrintRequiredNotFound( bool debug, const std::string & info_str, const std::vector<std::string> & node_title, const std::string & info_json )
{
    if (debug)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Required parameter not found in " + this->m_content_name + " while parsing " + info_str + " member: " + StrVectToStr(node_title, ".")});
        LogMessage(MSG_DEBUG_SETUP, {"Reporting xrt::device::get_info<xrt::info::device::" + info_str + "> API JSON output for card at " + this->m_bdf + " below:\n" +  info_json});
    }
    else
    {
        LogMessage(MSG_ITF_044, {this->m_content_name + " " + info_str, StrVectToStr(node_title, ".")});
        LogMessage(MSG_ITF_046, {"xrt::info::device::" + info_str, this->m_bdf, info_json});
    }
}

void DeviceInfoParser::PrintFailedToConvert( bool debug, const std::string & info_str, const std::vector<std::string> & node_title, const std::string & info_json )
{
    if (debug)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Failed to convert " + this->m_content_name + " while parsing " + info_str + " member " + StrVectToStr(node_title, ".")});
        LogMessage(MSG_DEBUG_SETUP, {"Reporting xrt::device::get_info<xrt::info::device::" + info_str + "> API JSON output for card at " + this->m_bdf + " below:\n" +  info_json});
    }
    else
    {
        LogMessage(MSG_ITF_013, {this->m_content_name + " " + info_str, StrVectToStr(node_title, ".")});
        LogMessage(MSG_ITF_046, {"xrt::info::device::" + info_str, this->m_bdf, info_json});
    }
}

bool DeviceInfoParser::LoadStringJSON( const std::string & info_str, const std::string & info_json )
{
    // Check output size
    if (info_json.empty())
    {
        LogMessage(MSG_ITF_131, {this->m_bdf});
        return RET_FAILURE;
    }
    // always clear parser
    ClearParser();

    // Initialize json parser and reader
    this->m_json_parser = json_parser_new();
    GError * error = nullptr;

    json_parser_load_from_data(this->m_json_parser, info_json.c_str(), info_json.size(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_content_name + " " + info_str, "<data>:<line>:<character>: <error message>", std::string(error->message)});
        LogMessage(MSG_ITF_046, {"xrt::info::device::" + info_str, this->m_bdf, info_json});
        g_error_free (error);
        g_object_unref (this->m_json_parser);
        return RET_FAILURE;
    }

    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    this->m_json_reader = json_reader_new(nullptr);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    return RET_SUCCESS;
}

} // namespace

#endif