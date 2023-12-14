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

#ifndef _DEVICEINFOPARSERXRT_H
#define _DEVICEINFOPARSERXRT_H

#ifdef USE_XRT

// XRT includes
#include <xrt/xrt_device.h>

#include "deviceinfoparser_base.h"
#include "xjsonparser.h"

namespace xbtest
{

class DeviceInfoParser: public DeviceInfoParserBase, public XJsonParser
{

public:
    DeviceInfoParser( Logging * log, const std::string & bdf, std::atomic<bool> * abort );
    ~DeviceInfoParser();

    void            LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list );
    void            LogMessage ( const Message_t & message );

    bool            Parse() override; // not used

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

    std::vector<xrt::device>    m_xrt_device; // use vector for device release

    const std::string UNKNOWN                               = "UNKNOWN";
    const std::string IS_PRESENT                            = "is_present";
    const std::string ID                                    = "id";
    const std::string LOCATION_ID                           = "location_id";
    const std::string DESCRIPTION                           = "description";
    const std::string FANS                                  = "fans";
    const std::string CRITICAL_TRIGGER_TEMP_C               = "critical_trigger_temp_C";
    const std::string SPEED_RPM                             = "speed_rpm";
    const std::string POWER_RAILS                           = "power_rails";
    const std::string VOLTAGE                               = "voltage";
    const std::string VOLTS                                 = "volts";
    const std::string CURRENT                               = "current";
    const std::string AMPS                                  = "amps";
    const std::string POWER_CONSUMPTION_MAX_WATTS           = "power_consumption_max_watts";
    const std::string POWER_CONSUMPTION_WATTS               = "power_consumption_watts";
    const std::string THERMALS                              = "thermals";
    const std::string TEMP_C                                = "temp_C";
    const std::string board                                 = "board";
    const std::string TYPE                                  = "type";
    const std::string MEMORY                                = "memory";
    const std::string MEMORIES                              = "memories";
    const std::string BASE_ADDRESS                          = "base_address";
    const std::string RANGE_BYTES                           = "range_bytes";
    const std::string PLATFORMS                             = "platforms";
    const std::string VBNV                                  = "vbnv";
    const std::string INTERFACE_UUID                        = "interface_uuid";
    const std::string STATUS                                = "status";
    const std::string P2P_STATUS                            = "p2p_status";
    const std::string CONTROLLER                            = "controller";
    const std::string SATELLITE_CONTROLLER                  = "satellite_controller";
    const std::string EXPECTED_VERSION                      = "expected_version";
    const std::string CLOCKS                                = "clocks";
    const std::string FREQ_MHZ                              = "freq_mhz";
    const std::string MACS                                  = "macs";
    const std::string ADDRESS                               = "address";
    const std::string LINK_SPEED_GBIT_SEC                   = "link_speed_gbit_sec";
    const std::string EXPECTED_LINK_SPEED_GBIT_SEC          = "expected_link_speed_gbit_sec";
    const std::string EXPRESS_LANE_WIDTH_COUNT              = "express_lane_width_count";
    const std::string EXPECTED_EXPRESS_LANE_WIDTH_COUNT     = "expected_express_lane_width_count";
    const std::string CPU_AFFINITY                          = "cpu_affinity";

    template <xrt::info::device param, typename T> bool GetDeviceInfo( T & device_info )
    {
        auto param_str = XrtInfoDeviceEnumToStr(param);
        if (CreateDevice() == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // Check if device is offline
        const auto info_offline_param = xrt::info::device::offline;
        try
        {
            auto offline_info = this->m_xrt_device[0].get_info<info_offline_param>();
            if (offline_info)
            {
                LogMessage(MSG_ITF_136, {"xrt::info::device::" + param_str, this->m_bdf});
                ReleaseDevice();
                return RET_FAILURE;
            }
        }
        catch (const std::exception & e)
        {
            LogMessage(MSG_ITF_130, {"xrt::info::device::" + XrtInfoDeviceEnumToStr(info_offline_param), this->m_bdf, e.what()});
            ReleaseDevice();
            return RET_FAILURE;
        }
        // Get device info
        try
        {
            device_info = this->m_xrt_device[0].get_info<param>();
        }
        catch (const std::exception & e)
        {
            LogMessage(MSG_ITF_130, {"xrt::info::device::" + param_str, this->m_bdf, e.what()});
            ReleaseDevice();
            return RET_FAILURE;
        }
        return RET_SUCCESS;
    }

    std::string XrtInfoDeviceEnumToStr( xrt::info::device info_enum );

    void        PrintRequiredNotFound ( bool debug, const std::string & info_str, const std::vector<std::string> & node_title, const std::string & info_json );
    void        PrintFailedToConvert  ( bool debug, const std::string & info_str, const std::vector<std::string> & node_title, const std::string & info_json );
    bool        LoadStringJSON        ( const std::string & info_str, const std::string & info_json );

};

} // namespace

#endif

#endif /* _DEVICEINFOPARSERXRT_H */
