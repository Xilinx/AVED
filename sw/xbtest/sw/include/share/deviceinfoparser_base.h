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

#ifndef _DEVICEINFOPARSERBASE_H
#define _DEVICEINFOPARSERBASE_H

#include "logging.h"

namespace xbtest
{

class DeviceInfoParserBase
{

public:

    DeviceInfoParserBase( Logging * log, const std::string & bdf, std::atomic<bool> * abort );
    ~DeviceInfoParserBase();

    void        LogMessage              ( const Message_t & message, const std::vector<std::string> & arg_list );
    void        LogMessage              ( const Message_t & message );
    void        UpdateMessageHistory    ( const Message_t & message );

    void        SetReportLevel( const LogLevel & level );
    uint64_t    GetApiFailTotal();
    void        InitApiFailCnt();
    bool        RetFailure();
    bool        RetSuccess();

    bool CreateDevice();
    void ReleaseDevice();

    virtual bool CheckDriverAccess       () = 0;
    virtual bool GetNameInfo             ( std::string & name_info ) = 0;
    virtual bool GetDeviceState          ( Device_State_Info_t & device_state ) = 0;
    virtual bool GetLogicUuid            ( std::string & logic_uuid ) = 0;
    virtual bool GetInterfaceUuid        ( std::string & interface_uuid ) = 0;
    virtual bool GetNodmaInfo            ( bool & nodma_info ) = 0;
    virtual bool GetElectricalInfoInner  ( Electrical_Info_t & electrical_info ) = 0;
    bool         GetElectricalInfo       ( Electrical_Info_t & electrical_info );
    virtual bool GetThermalInfoInner     ( Thermal_Info_t & thermal_info ) = 0;
    bool         GetThermalInfo          ( Thermal_Info_t & thermal_info );
    virtual bool GetMechanicalInfoInner  ( Mechanical_Info_t & mechanical_info ) = 0;
    bool         GetMechanicalInfo       ( Mechanical_Info_t & mechanical_info );
    bool         GetSensorsInfo          ( Electrical_Info_t & electrical_info, Thermal_Info_t & thermal_info, Mechanical_Info_t & mechanical_info );
    bool         GetSensorsInfo          ( Electrical_Info_t & electrical_info, Thermal_Info_t & thermal_info );
    virtual bool GetMemoryInfo           ( Memory_Info_t & memory_info ) = 0;
    virtual bool GetDesignInfo           ( Design_Info_t & design_info ) = 0;
    virtual bool GetHostInfo             ( Host_Info_t & host_info ) = 0;
    virtual bool GetPcieInfo             ( Pcie_Info_t & pcie_info ) = 0;
    virtual bool GetDynamicRegionsInfo   ( Dynamic_Regions_Info_t & dynamic_regions_info ) = 0;

    Logging *           m_log = nullptr;
    std::string         m_bdf;
    std::atomic<bool> * m_abort;

    std::string         m_log_header = LOG_HEADER_DEVICE_INFO;

    uint64_t                        m_api_fail_cnt    = 0;
    uint64_t                        m_api_fail_total  = 0;
    std::map<std::string, LogLevel> m_message_history;
    bool                            m_default_report = true; // If false, then Message_t with log_level != LOG_FAILURE are displayed with m_level
    LogLevel                        m_level;


};

} // namespace

#endif /* _DEVICEINFOPARSERBASE_H */
