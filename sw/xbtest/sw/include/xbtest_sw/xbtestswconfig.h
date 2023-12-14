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

#ifndef _XBTESTSWCONFIG_H
#define _XBTESTSWCONFIG_H

#include "xbtestswpackage.h"

#ifdef USE_NO_DRIVER
#include "deviceinfoparser_nodrv.h"
#endif
#ifdef USE_AMI
#include "deviceinfoparser_ami.h"
#endif
#ifdef USE_XRT
#include "deviceinfoparser_xrt.h"
#endif

namespace xbtest
{

class XbtestSwConfig
{

public:

    XbtestSwConfig( Logging * log, std::atomic<bool> * abort, const std::string & id );
    ~XbtestSwConfig();

    uint                                ParseCommandLine    ( int argc, char** argv );
    void                                SetCommandLine      ( const Xbtest_Command_Line_Sw_t & command_line );
    Xbtest_Command_Line_Sw_t            GetCommandLine();
    Xbtest_CL_option_Str_t              GetCommandLineTestJson();
    Xbtest_CL_option_Str_t              GetCommandLineXclbin();
    Xbtest_CL_option_Str_t              GetCommandLineP2PXclbin();
    Xbtest_CL_option_Str_t              GetCommandLineDeviceBdf();
    Xbtest_CL_option_Str_t              GetCommandLineP2pTargetBdf();
    Xbtest_CL_option_Str_t              GetCommandLineP2pNvmePath();
    Xbtest_CL_option_Str_t              GetCommandLineLogDir();
    Xbtest_CL_option_Str_t              GetCommandLineGuide();
    Xbtest_CL_option_Int_t              GetCommandLineVerbosity();
    Xbtest_CL_option_Str_t              GetCommandLineDesignPdi();
    bool                                GetCommandLineLogDisable();
    bool                                GetCommandLineConsoleDetailedFormat();
    bool                                GetCommandLineForce();
    std::string                         GetBdf();
    std::string                         GetXbtestpfmdef();
    std::string                         GetXclbin();
    std::string                         GetDesignPdi();
    bool                                ParseXclbin();
    bool                                ParseXbtestPfmDef();
    std::string                         GetLogDir();
    void                                ParseSystemInfo();
    bool                                ParseDeviceInfoWithoutBdf();
    void                                SetSessionTime( const bool & start_n_end );
    std::string                         GetSessionStartTime();
    std::string                         GetSessionEndTime();
    std::string                         GetSystemUsername();
    std::string                         GetSystemSysname();
    std::string                         GetSystemNodename();
    std::string                         GetSystemRelease();
    std::string                         GetSystemVersion();
    std::string                         GetSystemMachine();
    std::string                         GetHostInfoVersion();
    DeviceInfoParser *                  GetDeviceInfoParser();
    bool                                ParseDeviceInfoWithBdf();
    std::string                         GetNameInfo();
    bool                                GetNodmaInfo();
    bool                                GetIsMmioTestSupported();
    bool                                GetIsDmaSupported();
    bool                                ParseDeviceInfoPostXclbinDownload();
    void                                PrintDeviceInfo();
    bool                                GetIsP2PSupported();
    bool                                IsP2PEnabled();
    bool                                GenerateConfigPreXclbinLoad();
    bool                                GenerateConfigPostXclbinLoad();
    bool                                ComputeNewHostMemSize   ( const uint64_t & total_plram_buffer_size );
    std::vector<uint>                   GetAvailablePowerSLRs();
    std::vector<uint>                   GetAvailableGTMACs();
    std::vector<uint>                   GetAvailableGTLpbks();
    std::vector<uint>                   GetAvailableGTPrbss();
    std::vector<uint>                   GetAvailableGTFPrbss();
    std::vector<uint>                   GetAvailableGTMPrbss();
    std::vector<uint>                   GetAvailableGTYPPrbss();
    std::vector<uint>                   GetAvailableMultiGTPrbss( const std::string & multi_gt_name );
    std::vector<std::string>            GetAllAvailableMemories();
    std::vector<std::string>            GetAvailableBoardMemories();
    std::vector<std::string>            GetAvailableHostMemories();
    std::vector<std::string>            GetAvailableBoardTag();
    std::vector<std::string>            GetAvailableBoardMemoriesAndTags();
    std::vector<std::string>            GetAvailableTag                 ( const std::string & memory_name );
    std::vector<std::string>            GetAvailableSpTag               ( const std::string & memory_name );
    uint                                GetMemoryNumCU                  ( const std::string & memory_name );
    uint                                GetMemoryNumChannel             ( const std::string & memory_name );
    Xbtest_Memory_t                     GetMemorySettings               ( const uint & idx );
    bool                                IsOneMemCuSkipped               ( const std::string & memory_name );
    uint                                GetMemCUNumChannels             ( const std::string & cu_name );
    std::string                         GetMemCUTarget                  ( const std::string & cu_name );
    std::string                         GetCUConnectionVppSptag         ( const std::string & cu_name, const int & ch_idx );
    std::vector<std::string>            GetCUConnectionMemTags          ( const std::string & cu_name, const int & ch_idx );
    std::vector<uint>                   GetCUConnectionMemDataIndexes   ( const std::string & cu_name, const int & ch_idx );
    std::vector<uint64_t>               GetCuConnectionDeviceInfoSizes  ( const std::string & cu_name, const int & ch_idx );
    std::vector<uint64_t>               GetCuConnectionActualSizes      ( const std::string & cu_name, const int & ch_idx );
    uint64_t                            GetCuConnectionActualBaseAddress( const std::string & cu_name, const int & ch_idx );
    std::string                         GetPwrCUThrottleMode            ( const std::string & cu_name );
    uint                                GetPwrUseAIE                    ( const std::string & cu_name );
    bool                                GetPwrSupportAIE();
    bool                                GetPwrAIEInfoExists             ( const std::string & cu_name );
    uint                                GetPwrAIEFrequency              ( const std::string & cu_name );
    std::string                         GetPwrAIEType                   ( const std::string & cu_name );
    std::string                         GetPwrAIEControl                ( const std::string & cu_name );
    std::string                         GetPwrAIEStatus                 ( const std::string & cu_name );
    Xbtest_GT_t                         GetGTSettings                   ( const uint & gt_index );
    uint                                GetNumGT();
    uint                                GetNumGTPfm();
    Design_Info_t                       GetDesignInfo();
    Pcie_Info_t                         GetPcieInfo();
    bool                                IsExpectedPcieSpeedWidth();
    std::vector<Design_Mac_Info_t>      GetPlatformMacsInfo();
    Device_State_Info_t                 GetDeviceStateInfo();
    Xbtest_Pfm_Def_Mmio_t               GetMmioDefinitions();
    std::vector<Xbtest_Memory_t>        GetMemoryDefinitions();
    std::vector<Xbtest_Compute_Units_t> GetComputeUnitDefinitions();
    uint                                GetDownloadTime();
    bool                                GetInMemTopology                ( const std::string & mem_tag,      Xclbin_Memory_Data_t & memory_data, const bool & expect_used );
    bool                                GetInMemTopology                ( const uint        & mem_data_idx, Xclbin_Memory_Data_t & memory_data, const bool & expect_used );
    bool                                GetDeviceMemInfo                ( const uint & mem_data_idx, Mem_Info_t & mem_info );
    uint                                GetClockFrequency               ( const uint & clock_idx );
    Sensor_t                            GetSensorUsed();
    bool                                SensorOverrideJson              ( const Device_Mgmt_Type_Parameters_t & device_mgmt_param );
    std::vector<std::string>            GetAllMechanicalSourcesIDs();
    std::vector<std::string>            GetAllElectricalSourcesIDs();
    std::vector<std::string>            GetAllThermalSourcesIDs();

    uint                                GetHwVersionMajor();
    uint                                GetHwVersionMinor();
    double                              GetHwVersionMajorMinor();
    bool                                IsMemoryPatternSupported();

    void PrintActualConfig();

    std::vector<PCIe_Bar_Ep_t>  GetPcieBars();
    bool                        GetPcieBar( const uint & pcie_physical_function, const uint & pcie_base_address_register, PCIe_Bar_Ep_t & pcie_bar );
    bool                        GetPcieEp( const std::string & inst, PCIe_Bar_Ep_t & pcie_ep );

private:

    Logging *               m_log = nullptr;
    std::atomic<bool> *     m_abort;
    std::string             m_xbtest_sw_config_id;

    Xbtest_Configuration_t  m_xbtest_configuration;
    DeviceInfoParser *      m_device_info_parser        = nullptr;
    Sensor_t                m_sensor_all                = RST_SENSOR;
    Sensor_t                m_sensor_used               = RST_SENSOR;
    Xclbin_Metadata_t       m_xclbin_metadata;
    Xbtest_Pfm_Def_t        m_xbtest_pfm_def;
    std::string             m_name_info                 = RST_NAME_INFO;
    std::string             m_interface_uuid            = RST_INTERFACE_UUID;
    Device_State_Info_t     m_device_state_info         = RST_DEVICE_STATE_INFO;
    std::string             m_logic_uuid                = RST_LOGIC_UUID;
    bool                    m_nodma_info                = RST_NODMA_INFO; //set at the end of ParseDeviceInfoWithBdf()
    bool                    m_is_mmio_test_supported    = false; //set at the end of ParseDeviceInfoWithBdf()
    bool                    m_is_dma_supported          = false; //set at the end of ParseDeviceInfoWithBdf()
    bool                    m_is_p2p_supported          = false;
    Dynamic_Regions_Info_t  m_dynamic_regions_info      = RST_DYNAMIC_REGIONS_INFO;
    Pcie_Info_t             m_pcie_info                 = RST_PCIE_INFO;
    Host_Info_t             m_host_info                 = RST_HOST_INFO;
    Memory_Info_t           m_memory_info               = RST_MEMORY_INFO;
    Design_Info_t           m_design_info               = RST_DESIGN_INFO;
    std::string             m_log_header                = LOG_HEADER_XBT_SW_CFG;

    bool                    m_is_expected_pcie_speed_width = false;
    bool                    m_is_memory_pattern_supported = false;

    void                        LogMessage                  ( const Message_t & message, const std::vector<std::string> & arg_list );
    void                        LogMessage                  ( const Message_t & message );
    void                        SetLogDir                   ( const std::string & log_dir );
    void                        SetLogDirCreated();
    Xbtest_CL_option_Str_t      GetCommandLineXbtestpfmdef();
    Xbtest_CL_option_Str_t      GetCommandLineP2PXbtestpfmdef();
    void                        SetBdf                      ( const std::string & bdf );
    void                        SetXbtestpfmdef             ( const std::string & xbtest_pfm_def );
    void                        SetXclbin                   ( const std::string & xclbin );
    void                        SetDesignPdi                ( const std::string & design_pdi );
    bool                        ParseDefaultSensor();
    bool                        SensorOverridePfmDef();
    bool                        CheckSensorThresholds       ( const std::string & sensor_name, const std::string & threshold_name, const Sensor_Limit_t & threshold );
    std::vector<std::string>    GetAllPowerRailSourcesIDs();
    std::vector<std::string>    GetAllPowerConsumptionSourceIDs();
    void                        PrintSensorLimit            ( const std::string & limit_str, const std::string & unit_str, const Sensor_Limit_t & sensor_limit );
    void                        SetIsP2PSupported();
    void                        SetIsMmioTestSupported();
    void                        SetIsDmaSupported();

};

} // namespace

#endif /* _XBTESTSWCONFIG_H */
