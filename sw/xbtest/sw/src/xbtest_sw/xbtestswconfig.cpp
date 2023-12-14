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

#include <fstream>
#include <sys/utsname.h>
#include <thread>

#include "xbtestswconfig.h"
#include "commandlineparsersw.h"
#include "xclbinparser.h"
#include "xbtestpfmdefparser.h"

namespace xbtest
{

XbtestSwConfig::XbtestSwConfig( Logging * log, std::atomic<bool> * abort, const std::string & id )
{
    this->m_log = log;
    this->m_abort = abort;
    this->m_xbtest_sw_config_id = id;
    this->m_xbtest_configuration.logging.log_dir_created = false;
    this->m_xbtest_configuration.logging.log_dir = "";

    SetSessionTime(true);
    ParseSystemInfo();
}

XbtestSwConfig::~XbtestSwConfig() = default;

uint XbtestSwConfig::ParseCommandLine( int argc, char** argv )
{
    CommandLineParserSw command_line_parser(this->m_log, this->m_abort, argc, argv);
    auto command_line_ret = command_line_parser.ParseCommandLine();
    if (command_line_ret != COMMAND_LINE_RET_SUCCESS)
    {
        return command_line_ret;
    }
    this->m_xbtest_configuration.command_line = command_line_parser.GetCommandLine();

    SetBdf(GetCommandLineDeviceBdf().value);
    SetXbtestpfmdef(GetCommandLineXbtestpfmdef().value);

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    SetXclbin("");
    SetDesignPdi(GetCommandLineDesignPdi().value);
#endif
#ifdef USE_XRT
    SetXclbin(GetCommandLineXclbin().value);
    SetDesignPdi("");
#endif

    // Create log dir
    // The default log dir name contains BDF
    // The log dir is not created if the command line is not validated or some options are used, e.g -h, -v, -m ...
    if (GetCommandLineLogDir().exists)
    {
        SetLogDir(GetCommandLineLogDir().value);
    }
    else
    {
        SetLogDir("./xbtest_logs/" + this->m_xbtest_configuration.system.start_time2 + "_" + FormatName(GetBdf(), "-"));
    }

    if (this->m_log->GenerateLogDir(GetLogDir(), GetCommandLineLogDisable(), GetCommandLineForce()) == RET_FAILURE)
    {
        return COMMAND_LINE_RET_FAILURE;
    }
    SetLogDirCreated();

    // Write test.json in log directory as is
    if (!GetCommandLineLogDisable())
    {
        auto dest_name = GetLogDir() + "/test.json";

        if (FileExists(dest_name))
        {
            if (GetCommandLineForce())
            {
                LogMessage(MSG_CMN_019, {dest_name}); // warning
            }
            else
            {
                LogMessage(MSG_CMN_053, {dest_name});
                return COMMAND_LINE_RET_FAILURE;
            }
        }

        // Copy
        std::ofstream dest;
        if (!OpenFile(dest_name, dest))
        {
            LogMessage(MSG_CMN_018, {dest_name});
            return COMMAND_LINE_RET_FAILURE;
        }
        std::ifstream src(GetCommandLineTestJson().value, std::ofstream::binary) ;
        dest << src.rdbuf();
    }
    return COMMAND_LINE_RET_SUCCESS;
}

void XbtestSwConfig::SetCommandLine( const Xbtest_Command_Line_Sw_t & command_line )
{
    this->m_xbtest_configuration.command_line = command_line;
    if (this->m_xbtest_sw_config_id == "p2p")
    {
        SetBdf(GetCommandLineP2pTargetBdf().value);
        SetXbtestpfmdef(GetCommandLineP2PXbtestpfmdef().value);
        SetXclbin(GetCommandLineP2PXclbin().value);
    }
}

Xbtest_Command_Line_Sw_t XbtestSwConfig::GetCommandLine()
{
    return this->m_xbtest_configuration.command_line;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineTestJson()
{
    return this->m_xbtest_configuration.command_line.test_json;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineXclbin()
{
    return this->m_xbtest_configuration.command_line.xclbin;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineP2PXclbin()
{
    return this->m_xbtest_configuration.command_line.p2p_xclbin;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineDeviceBdf()
{
    return this->m_xbtest_configuration.command_line.device_bdf;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineP2pTargetBdf()
{
    return this->m_xbtest_configuration.command_line.p2p_target_bdf;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineP2pNvmePath()
{
    return this->m_xbtest_configuration.command_line.p2p_nvme_path;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineLogDir()
{
    return this->m_xbtest_configuration.command_line.log_dir;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineGuide()
{
    return this->m_xbtest_configuration.command_line.guide;
}

Xbtest_CL_option_Int_t XbtestSwConfig::GetCommandLineVerbosity()
{
    return this->m_xbtest_configuration.command_line.verbosity;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineDesignPdi()
{
    return this->m_xbtest_configuration.command_line.design_pdi;
}

bool XbtestSwConfig::GetCommandLineLogDisable()
{
    return this->m_xbtest_configuration.command_line.disable_log;
}

bool XbtestSwConfig::GetCommandLineConsoleDetailedFormat()
{
    return this->m_xbtest_configuration.command_line.console_detailed_format;
}

bool XbtestSwConfig::GetCommandLineForce()
{
    return this->m_xbtest_configuration.command_line.force;
}

std::string XbtestSwConfig::GetBdf()
{
    return this->m_xbtest_configuration.bdf;
}

std::string XbtestSwConfig::GetXbtestpfmdef()
{
    return this->m_xbtest_configuration.xbtest_pfm_def;
}

std::string XbtestSwConfig::GetXclbin()
{
    return this->m_xbtest_configuration.xclbin;
}

std::string XbtestSwConfig::GetDesignPdi()
{
    return this->m_xbtest_configuration.design_pdi;
}

bool XbtestSwConfig::ParseXclbin()
{
    XclbinParser xclbin_parser(this->m_log, GetXclbin(), GetDesignPdi(), this->m_abort);
    if (xclbin_parser.Parse() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    this->m_xclbin_metadata = xclbin_parser.GetXclbinMetadata();

    if (GetHwVersionMajorMinor() >= MIN_HW_VERSION_PATTERN)
    {
        this->m_is_memory_pattern_supported = true;
    }
    return RET_SUCCESS;
}

bool XbtestSwConfig::ParseXbtestPfmDef()
{
    XbtestPfmDefParser xbtest_pfm_def_parser(this->m_log, GetXbtestpfmdef(), !(GetIsDmaSupported()), this->m_pcie_info, GetIsP2PSupported(), GetIsMmioTestSupported(), this->m_abort);
    if (xbtest_pfm_def_parser.Parse() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    this->m_xbtest_pfm_def = xbtest_pfm_def_parser.GetPlatformDef();
#ifdef USE_NO_DRIVER
        return RET_SUCCESS;
#endif
    return SensorOverridePfmDef(); // Override default sensor with sensor from pfm def
}

std::string XbtestSwConfig::GetLogDir()
{
    return this->m_xbtest_configuration.logging.log_dir;
}

void XbtestSwConfig::ParseSystemInfo()
{
    this->m_xbtest_configuration.system.username  = "UNKNOWN";
    this->m_xbtest_configuration.system.sysname   = "UNKNOWN";
    this->m_xbtest_configuration.system.nodename  = "UNKNOWN";
    this->m_xbtest_configuration.system.release   = "UNKNOWN";
    this->m_xbtest_configuration.system.version   = "UNKNOWN";
    this->m_xbtest_configuration.system.machine   = "UNKNOWN";

    char username[LOGIN_NAME_MAX];
    if (getlogin_r(username, LOGIN_NAME_MAX) == 0)
    {
        this->m_xbtest_configuration.system.username = std::string(username);
    }
    utsname tmp_utsname;
    if (uname(&tmp_utsname) == 0)
    {
        this->m_xbtest_configuration.system.sysname   = std::string(tmp_utsname.sysname);
        this->m_xbtest_configuration.system.nodename  = std::string(tmp_utsname.nodename);
        this->m_xbtest_configuration.system.release   = std::string(tmp_utsname.release);
        this->m_xbtest_configuration.system.version   = std::string(tmp_utsname.version);
        this->m_xbtest_configuration.system.machine   = std::string(tmp_utsname.machine);
    }
}

bool XbtestSwConfig::ParseDeviceInfoWithoutBdf()
{
    LogMessage(MSG_DEBUG_PARSING, {"Getting device info without BDF"});
    auto device_info_parser_0 = DeviceInfoParser(this->m_log, "0", this->m_abort); // Use first device found to get host info
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (device_info_parser_0.GetHostInfo(this->m_host_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {XRT_INFO_DEVICE_HOST});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {XRT_INFO_DEVICE_HOST + " information: " + this->m_host_info.info_json});
    return RET_SUCCESS;
}

void XbtestSwConfig::SetSessionTime( const bool & start_n_end )
{
    if (start_n_end)
    {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        this->m_xbtest_configuration.system.start_time  = FormatTime(now, "%a %b %d %H:%M:%S %Y %Z"); // Sun Jul 26 17:26:07 2020 BST
        this->m_xbtest_configuration.system.start_time2 = FormatTime(now, "%Y-%m-%d_%H-%M-%S");       // 2020-07-26_17-26-07
    }
    else
    {
        this->m_xbtest_configuration.system.end_time = GetCurrentTime();
    }
}

std::string XbtestSwConfig::GetSessionStartTime()
{
    return this->m_xbtest_configuration.system.start_time;
}

std::string XbtestSwConfig::GetSessionEndTime()
{
    return this->m_xbtest_configuration.system.end_time;
}

std::string XbtestSwConfig::GetSystemUsername()
{
    return this->m_xbtest_configuration.system.username;
}

std::string XbtestSwConfig::GetSystemSysname()
{
    return this->m_xbtest_configuration.system.sysname;
}

std::string XbtestSwConfig::GetSystemNodename()
{
    return this->m_xbtest_configuration.system.nodename;
}

std::string XbtestSwConfig::GetSystemRelease()
{
    return this->m_xbtest_configuration.system.release;
}

std::string XbtestSwConfig::GetSystemVersion()
{
    return this->m_xbtest_configuration.system.version;
}

std::string XbtestSwConfig::GetSystemMachine()
{
    return this->m_xbtest_configuration.system.machine;
}

std::string XbtestSwConfig::GetHostInfoVersion()
{
    return this->m_host_info.version;
}

DeviceInfoParser* XbtestSwConfig::GetDeviceInfoParser()
{
    return this->m_device_info_parser;
}

bool XbtestSwConfig::ParseDeviceInfoWithBdf()
{
    LogMessage(MSG_DEBUG_PARSING, {"Getting device information with BDF"});
    this->m_device_info_parser = new DeviceInfoParser(this->m_log, GetBdf(), this->m_abort);

    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetNameInfo(this->m_name_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"device name"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {"Device name: " + this->m_name_info});

#ifdef USE_AMI
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetDeviceState(this->m_device_state_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"device state"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {"Current device state : " + this->m_device_state_info.current_state});
    LogMessage(MSG_DEBUG_SETUP, {"Expected device state: " + this->m_device_state_info.expected_state});
#endif
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetLogicUuid(this->m_logic_uuid) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"logic UUID"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {"Logic UUID: " + this->m_logic_uuid});
#endif
#ifdef USE_XRT
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetInterfaceUuid(this->m_interface_uuid) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"interface UUID"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {"Interface UUID: " + this->m_interface_uuid});
#endif

    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetNodmaInfo(this->m_nodma_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"NoDMA information"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {"NoDMA information: " + BoolToStr(this->m_nodma_info)});
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetPcieInfo(this->m_pcie_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"PCIe information"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    LogMessage(MSG_DEBUG_SETUP, {"PCIe information: "});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Current PCIe speed: " + std::to_string(this->m_pcie_info.link_speed_gbit_sec)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Maximum PCIe speed: " + std::to_string(this->m_pcie_info.expected_link_speed_gbit_sec)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Current PCIe width: " + std::to_string(this->m_pcie_info.express_lane_width_count)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Maximum PCIe width: " + std::to_string(this->m_pcie_info.expected_express_lane_width_count)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - CPU Affinity      : " +                this->m_pcie_info.cpu_affinity});
#endif
#ifdef USE_XRT
    LogMessage(MSG_DEBUG_SETUP, {"PCIe information: " + this->m_pcie_info.info_json});
#endif

    if ((this->m_pcie_info.link_speed_gbit_sec      == this->m_pcie_info.expected_link_speed_gbit_sec)
     && (this->m_pcie_info.express_lane_width_count == this->m_pcie_info.expected_express_lane_width_count))
    {
        this->m_is_expected_pcie_speed_width = true;
    }

    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetDesignInfo(this->m_design_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {"design information"});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    SetIsP2PSupported();

    LogMessage(MSG_DEBUG_SETUP, {"Design information: " + this->m_design_info.info_json});

    if (ParseDefaultSensor() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    if (GetNodmaInfo())
    {
        LogMessage(MSG_ITF_123);
    }

    SetIsMmioTestSupported();
    SetIsDmaSupported();

    return RET_SUCCESS;
}

std::string XbtestSwConfig::GetNameInfo()
{
    return this->m_name_info;
}

bool XbtestSwConfig::GetNodmaInfo()
{
    return this->m_nodma_info;
}

void XbtestSwConfig::SetIsMmioTestSupported()
{
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    this->m_is_mmio_test_supported = true;
#endif
#ifdef USE_XRT
    this->m_is_mmio_test_supported = false;
#endif
}

bool XbtestSwConfig::GetIsMmioTestSupported()
{
    return this->m_is_mmio_test_supported;
}

void XbtestSwConfig::SetIsDmaSupported()
{
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    this->m_is_dma_supported = false;
    //LogMessage(MSG_DMA_033);
#endif
#ifdef USE_XRT
    this->m_is_dma_supported = !(this->m_nodma_info);
#endif
}

bool XbtestSwConfig::GetIsDmaSupported()
{
    return this->m_is_dma_supported;
}

bool XbtestSwConfig::ParseDeviceInfoPostXclbinDownload()
{
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetMemoryInfo(this->m_memory_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {XRT_INFO_DEVICE_MEMORY});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetDynamicRegionsInfo(this->m_dynamic_regions_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {XRT_INFO_DEVICE_DYNAMIC_REGIONS});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LogMessage(MSG_DEBUG_SETUP, {XRT_INFO_DEVICE_DYNAMIC_REGIONS + " information: " + this->m_dynamic_regions_info.info_json});
    return RET_SUCCESS;
}

void XbtestSwConfig::PrintDeviceInfo()
{
    LogMessage(MSG_ITF_015, {"Device name                : " + this->m_name_info});
    LogMessage(MSG_ITF_015, {"Current device state       : " + this->m_device_state_info.current_state});
    LogMessage(MSG_ITF_015, {"Expected device state      : " + this->m_device_state_info.expected_state});
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    LogMessage(MSG_ITF_015, {"Logic UUID                 : " + this->m_logic_uuid});
#endif
#ifdef USE_XRT
    LogMessage(MSG_ITF_015, {"Interface UUID             : " + this->m_interface_uuid});
#endif
    LogMessage(MSG_ITF_015, {"Nodma information          : " + BoolToStr(this->m_nodma_info)});
    LogMessage(MSG_ITF_015, {"Host information           : " + this->m_host_info.info_json});

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    LogMessage(MSG_ITF_015, {"PCIe information           : "});
    LogMessage(MSG_ITF_015, {"\t - Current PCIe speed: " + std::to_string(this->m_pcie_info.link_speed_gbit_sec)});
    LogMessage(MSG_ITF_015, {"\t - Maximum PCIe speed: " + std::to_string(this->m_pcie_info.expected_link_speed_gbit_sec)});
    LogMessage(MSG_ITF_015, {"\t - Current PCIe width: " + std::to_string(this->m_pcie_info.express_lane_width_count)});
    LogMessage(MSG_ITF_015, {"\t - Maximum PCIe width: " + std::to_string(this->m_pcie_info.expected_express_lane_width_count)});
    LogMessage(MSG_ITF_015, {"\t - CPU Affinity      : " +                this->m_pcie_info.cpu_affinity});
#endif
#ifdef USE_XRT
    LogMessage(MSG_DEBUG_SETUP, {"PCIe information       : " + this->m_pcie_info.info_json});
#endif

#ifdef USE_XRT
    LogMessage(MSG_ITF_015, {"Design information         : " + this->m_design_info.info_json});
    LogMessage(MSG_ITF_015, {"dynamic_regions information: " + this->m_dynamic_regions_info.info_json});
#endif
#ifdef USE_AMI
    LogMessage(MSG_ITF_015, {"Design information: " + this->m_design_info.info_json});
    LogMessage(MSG_ITF_015, {"\t - AMC version  : " + this->m_design_info.controller.amc.version});
#endif
}

bool XbtestSwConfig::GetIsP2PSupported()
{
    return this->m_is_p2p_supported;
}

void XbtestSwConfig::SetIsP2PSupported()
{
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
        this->m_is_p2p_supported = false;
        //LogMessage(MSG_P2P_041);
    #endif
    #ifdef USE_XRT
        this->m_is_p2p_supported = (this->m_design_info.status.p2p_status != "not supported");
    #endif
}

bool XbtestSwConfig::IsP2PEnabled()
{
    auto design_info = GetDesignInfo();
    return (design_info.status.p2p_status == "enabled");
}

bool XbtestSwConfig::GenerateConfigPreXclbinLoad()
{
    // Get device info using provided BDF once provided BDF is validated
    if (ParseDeviceInfoWithBdf() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    if (this->m_xbtest_sw_config_id == "p2p")
    {
        LogMessage(MSG_ITF_008, {"P2P target card", GetNameInfo()});
    }
    else if (GetCommandLineP2pTargetBdf().exists)
    {
        LogMessage(MSG_ITF_008, {"P2P source card", GetNameInfo()});
    }
    else
    {
        LogMessage(MSG_ITF_008, {"card", GetNameInfo() + " (" + this->m_xbtest_configuration.bdf + ")"});
    }


    // Parse xbtest metadata and card definition JSON file
    if (ParseXclbin() == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ParseXbtestPfmDef() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Merge GT definition from USER_METADATA and from xbtest_pfm_def.json
    this->m_xbtest_configuration.gt.clear();
    for (const auto & um_gt : this->m_xclbin_metadata.user_metadata.definition.gt)
    {
        Xbtest_GT_t gt;

        // From USER_METADATA
        gt.gt_index         = um_gt.gt_index;
        gt.gt_group_select  = um_gt.gt_group_select;
        gt.gt_serial_port   = um_gt.gt_serial_port;
        gt.gt_diff_clocks   = um_gt.gt_diff_clocks;

        // GT definition existing in USER_METADATA is not found in xbtest_pfm_def.json
        if (gt.gt_index >= this->m_xbtest_pfm_def.gt.settings.size())
        {
            LogMessage(MSG_ITF_097, {"GT index", std::to_string(gt.gt_index)});
            return RET_FAILURE;
        }
        // From xbtest_pfm_def.json
        gt.name                 = this->m_xbtest_pfm_def.gt.settings[gt.gt_index].name;
        gt.transceivers         = this->m_xbtest_pfm_def.gt.settings[gt.gt_index].transceivers;

        this->m_xbtest_configuration.gt.emplace_back(gt);
    }
    // GT existing in xbtest_pfm_def.json is not found USER_METADATA
    for (uint pfm_gt_index = 0; pfm_gt_index < this->m_xbtest_pfm_def.gt.settings.size(); pfm_gt_index++)
    {
        auto found = false;
        for (const auto & um_gt : this->m_xclbin_metadata.user_metadata.definition.gt)
        {
            if (pfm_gt_index == um_gt.gt_index)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            LogMessage(MSG_DEBUG_EXCEPTION, {"Ignoring the GT[" + std::to_string(pfm_gt_index) + "] definition found in xbtest_pfm_def as no definition was found in USER_METADATA"});
        }
    }

    // Merge Memory definition from USER_METADATA and from xbtest_pfm_def.json
    this->m_xbtest_configuration.memory.clear();
    for (const auto & um_memory : this->m_xclbin_metadata.user_metadata.definition.memory)
    {
        Xbtest_Memory_t memory;

        // Init
        memory.available_tag.clear();
        memory.vpp_sptags.clear();
        memory.num_channels = 0;
        memory.num_cu = 0;

        // From USER_METADATA
        memory.name             = um_memory.name;
        memory.type             = um_memory.type;
        memory.target           = um_memory.target;
        memory.axi_data_size    = um_memory.axi_data_size;
        memory.num_axi_thread   = um_memory.num_axi_thread;
        memory.idx              = um_memory.idx;

        // From xbtest_pfm_def.json
        auto found = false;
        for (const auto & pfm_def_memory : this->m_xbtest_pfm_def.memory)
        {
            if (memory.name == pfm_def_memory.name)
            {
                if (!(GetIsDmaSupported()) || (memory.target == HOST))
                {
                    if (pfm_def_memory.dma_bw_exists)
                    {
                        LogMessage(MSG_ITF_099, {"dma_bw", memory.name});
                        return RET_FAILURE;
                    }
                    if (pfm_def_memory.dma_config_exists)
                    {
                        LogMessage(MSG_ITF_099, {"dma_config", memory.name});
                        return RET_FAILURE;
                    }
                }
                if (!(GetIsP2PSupported()) || (memory.target == HOST))
                {
                    if (pfm_def_memory.p2p_card_bw_exists)
                    {
                        LogMessage(MSG_ITF_099, {"p2p_card_bw", memory.name});
                        return RET_FAILURE;
                    }
                    if (pfm_def_memory.p2p_card_config_exists)
                    {
                        LogMessage(MSG_ITF_099, {"p2p_card_config", memory.name});
                        return RET_FAILURE;
                    }
                    if (pfm_def_memory.p2p_nvme_bw_exists)
                    {
                        LogMessage(MSG_ITF_099, {"p2p_nvme_bw", memory.name});
                        return RET_FAILURE;
                    }
                    if (pfm_def_memory.p2p_nvme_config_exists)
                    {
                        LogMessage(MSG_ITF_099, {"p2p_nvme_config", memory.name});
                        return RET_FAILURE;
                    }
                }
                memory.dma_bw                   = pfm_def_memory.dma_bw;
                memory.dma_bw_exists            = pfm_def_memory.dma_bw_exists;
                memory.dma_config               = pfm_def_memory.dma_config;
                memory.dma_config_exists        = pfm_def_memory.dma_config_exists;
                memory.p2p_card_bw              = pfm_def_memory.p2p_card_bw;
                memory.p2p_card_bw_exists       = pfm_def_memory.p2p_card_bw_exists;
                memory.p2p_card_config          = pfm_def_memory.p2p_card_config;
                memory.p2p_card_config_exists   = pfm_def_memory.p2p_card_config_exists;
                memory.p2p_nvme_bw              = pfm_def_memory.p2p_nvme_bw;
                memory.p2p_nvme_bw_exists       = pfm_def_memory.p2p_nvme_bw_exists;
                memory.p2p_nvme_config          = pfm_def_memory.p2p_nvme_config;
                memory.p2p_nvme_config_exists   = pfm_def_memory.p2p_nvme_config_exists;
                memory.cu_bw                    = pfm_def_memory.cu_bw;
                memory.cu_latency               = pfm_def_memory.cu_latency;
                memory.cu_rate                  = pfm_def_memory.cu_rate;
                memory.cu_outstanding           = pfm_def_memory.cu_outstanding;
                memory.cu_burst_size            = pfm_def_memory.cu_burst_size;
                found = true;
                break;
            }
        }
        // Memory definition existing in USER_METADATA is not found in xbtest_pfm_def.json
        if (!found)
        {
            LogMessage(MSG_ITF_097, {"Memory name", memory.name});
            return RET_FAILURE;
        }

        this->m_xbtest_configuration.memory.emplace_back(memory);
    }
    // Memory existing in xbtest_pfm_def.json is not found USER_METADATA
    for (const auto & pfm_def_memory : this->m_xbtest_pfm_def.memory)
    {
        auto found = false;
        for (const auto & um_memory : this->m_xclbin_metadata.user_metadata.definition.memory)
        {
            if (pfm_def_memory.name == um_memory.name)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            LogMessage(MSG_DEBUG_EXCEPTION, {"Ignoring the Memory (" + pfm_def_memory.name + ") definition found in card definition JSON file as no definition was found in USER_METADATA"});
        }
    }

    // Merge xbtest HW IPs definition from USER_METADATA and from xbtest_pfm_def.json
    this->m_xbtest_configuration.compute_units.clear();
    for (const auto & um_compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        Xbtest_Compute_Units_t compute_unit;

        // From USER_METADATA
        compute_unit.name       = um_compute_unit.name;
        compute_unit.SLR        = um_compute_unit.SLR;
        compute_unit.mode       = um_compute_unit.mode;

        if (compute_unit.mode == BI_VERIFY_HW_COMPONENT_ID)
        {
            compute_unit.cu_type_configuration.dna_read         = um_compute_unit.cu_type_configuration.dna_read;
        }
        else if (compute_unit.mode == BI_PWR_HW_COMPONENT_ID)
        {
            compute_unit.cu_type_configuration.SLR              = um_compute_unit.cu_type_configuration.SLR;
            compute_unit.cu_type_configuration.throttle_mode    = um_compute_unit.cu_type_configuration.throttle_mode;
            compute_unit.cu_type_configuration.use_aie          = um_compute_unit.cu_type_configuration.use_aie;
            if (compute_unit.cu_type_configuration.use_aie)
            {
                compute_unit.cu_type_configuration.aie.exists   = this->m_xclbin_metadata.user_metadata.definition.aie.exists;
                compute_unit.cu_type_configuration.aie.freq     = this->m_xclbin_metadata.user_metadata.definition.aie.freq;
                compute_unit.cu_type_configuration.aie.type     = this->m_xclbin_metadata.user_metadata.definition.aie.type;
                compute_unit.cu_type_configuration.aie.control  = this->m_xclbin_metadata.user_metadata.definition.aie.control;
                compute_unit.cu_type_configuration.aie.status   = this->m_xclbin_metadata.user_metadata.definition.aie.status;
            }
        }
        else if (compute_unit.mode == BI_MEM_HW_COMPONENT_ID)
        {
            for (auto & memory : this->m_xbtest_configuration.memory)
            {
                if (um_compute_unit.cu_type_configuration.memory_type == memory.name)
                {
                    compute_unit.cu_type_configuration.memory = &memory;
                    break;
                }
            }
        }
        else if ((compute_unit.mode == BI_GT_MAC_HW_COMPONENT_ID) || (compute_unit.mode == BI_GT_LPBK_HW_COMPONENT_ID) || (compute_unit.mode == BI_GT_PRBS_HW_COMPONENT_ID))
        {
            for (auto & gt : this->m_xbtest_configuration.gt)
            {
                if (um_compute_unit.cu_type_configuration.gt_index == gt.gt_index)
                {
                    compute_unit.cu_type_configuration.gt = &gt;
                    break;
                }
            }
        }
        else if (compute_unit.mode == BI_GTF_PRBS_HW_COMPONENT_ID)
        {
            for (auto & gt : this->m_xbtest_configuration.gt)
            {
                for (uint gt_index = 0; gt_index < um_compute_unit.cu_type_configuration.gtf_indexes.size(); gt_index++)
                {
                    auto enable = um_compute_unit.cu_type_configuration.gtf_indexes[gt_index];
                    if ((gt_index == gt.gt_index) && (enable))
                    {
                        compute_unit.cu_type_configuration.gts.emplace_back(&gt);
                        break;
                    }
                }
            }
        }
        else if (compute_unit.mode == BI_GTM_PRBS_HW_COMPONENT_ID)
        {
            for (auto & gt : this->m_xbtest_configuration.gt)
            {
                for (uint gt_index = 0; gt_index < um_compute_unit.cu_type_configuration.gtm_indexes.size(); gt_index++)
                {
                    auto enable = um_compute_unit.cu_type_configuration.gtm_indexes[gt_index];
                    if ((gt_index == gt.gt_index) && (enable))
                    {
                        compute_unit.cu_type_configuration.gts.emplace_back(&gt);
                        break;
                    }
                }
            }
        }
        else if (compute_unit.mode == BI_GTYP_PRBS_HW_COMPONENT_ID)
        {
            for (auto & gt : this->m_xbtest_configuration.gt)
            {
                for (uint gt_index = 0; gt_index < um_compute_unit.cu_type_configuration.gtyp_indexes.size(); gt_index++)
                {
                    auto enable = um_compute_unit.cu_type_configuration.gtyp_indexes[gt_index];
                    if ((gt_index == gt.gt_index) && (enable))
                    {
                        compute_unit.cu_type_configuration.gts.emplace_back(&gt);
                        break;
                    }
                }
            }
        }

        // Mark xbtest HW IP as companion. This kind of xbtest HW IP is not controllable by the host application as it's not connected to PLRAM (e.g Power xbtest HW IP controlled by another Power xbtest HW IP)
        compute_unit.companion = false;
        if (um_compute_unit.connectivity.empty())
        {
            LogMessage(MSG_DEBUG_SETUP, {"Found a companion xbtest HW IP, not controllable by host application: " + compute_unit.name});
            compute_unit.companion = true;
        }
        // Do not look for xbtest HW IP connectivity for companion xbtest HW IP
        if (!(compute_unit.companion))
        {
            std::string name_cmp;
            auto ip_idx_found = false;

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
            name_cmp = compute_unit.name + "_1";
#endif
#ifdef USE_XRT
            name_cmp = compute_unit.name + ":";
#endif
            // From IP_LAYOUT
            for (const auto & ip_data : this->m_xclbin_metadata.ip_layout.m_ip_data)
            {
                // format: - ip_data.m_name : "krnl_memtest_ddr_01_00:krnl_memtest_ddr_01_00_1"
                //         - name_cmp       : "krnl_memtest_ddr_01_00:"
                if (ip_data.m_name.compare(0, name_cmp.size(), name_cmp) == 0)
                {
                    compute_unit.m_ip_layout_index = ip_data.m_ip_layout_index;
                    ip_idx_found = true;
                    break;
                }
            }

            // Do not fail if the xbtest HW IP defined in USER_METADATA was not included in HW design
            //      Note in opposite case, if xbtest HW IP present in HW design, not defined in USER_METADATA, it is ignored by xbtest SW
            if (!ip_idx_found)
            {
                continue;
            }

            int32_t cu_arg_index = 4; // Init argument index, account for the first 4 arguments which are scalars
            for (const auto & um_connection : um_compute_unit.connectivity)
            {
                Xbtest_Connection_t connection;

                // Init
                connection.mem_data_indexes.clear();
                connection.mem_tags.clear();

                // From USER_METADATA
                connection.vpp_sptag = um_connection;

                // From CONNECTIVITY: A xbtest HW IP port can be connected to multiple memory bank in connectivity (when not using GROUP_CONNECTIVITY)
                for (const auto & xclbin_connection : this->m_xclbin_metadata.connectivity.m_connection)
                {
                    if ((xclbin_connection.m_ip_layout_index == compute_unit.m_ip_layout_index) && (xclbin_connection.arg_index == cu_arg_index))
                    {
                        if (xclbin_connection.mem_data_index >= (int32_t)this->m_xclbin_metadata.memory_topology.size())
                        {
                            LogMessage(MSG_ITF_140, {std::to_string(xclbin_connection.mem_data_index), std::to_string(cu_arg_index), compute_unit.name});
                            return RET_FAILURE;
                        }
                        auto xclbin_memory_data = this->m_xclbin_metadata.memory_topology[xclbin_connection.mem_data_index];
                        connection.mem_tags.emplace_back(xclbin_memory_data.m_tag);
                        connection.mem_data_indexes.emplace_back(xclbin_connection.mem_data_index);
                    }
                }
                if (connection.mem_data_indexes.empty())
                {
                    LogMessage(MSG_ITF_141, {std::to_string(cu_arg_index), compute_unit.name, std::to_string(compute_unit.m_ip_layout_index)});
                    return RET_FAILURE;
                }
                compute_unit.connectivity.emplace_back(connection);
                cu_arg_index++;
            }
        }
        this->m_xbtest_configuration.compute_units.emplace_back(compute_unit);
    }

    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.mode != BI_MEM_HW_COMPONENT_ID)
        {
            continue;
        }

        compute_unit.cu_type_configuration.memory->num_channels = 0;
        for (uint idx = 1; idx < compute_unit.connectivity.size(); idx++) // ignore port_idx=0 which is PLRAM
        {
            // Create vector of unique tags
            for (const auto & tag : compute_unit.connectivity[idx].mem_tags)
            {
                if (!FindInVect(tag, compute_unit.cu_type_configuration.memory->available_tag))
                {
                    compute_unit.cu_type_configuration.memory->available_tag.emplace_back(tag); // insert
                }
            }
            // SP tag
            if (!FindInVect(compute_unit.connectivity[idx].vpp_sptag, compute_unit.cu_type_configuration.memory->vpp_sptags))
            {
                compute_unit.cu_type_configuration.memory->vpp_sptags.emplace_back(compute_unit.connectivity[idx].vpp_sptag); // insert
            }
            compute_unit.cu_type_configuration.memory->num_channels++;
        }
        compute_unit.cu_type_configuration.memory->num_cu++;
    }
    return RET_SUCCESS;
}

bool XbtestSwConfig::GenerateConfigPostXclbinLoad()
{
    // xbtest HW IPs

    // Get total memory size for each port based on tag from device info (XRT C++API) memory topology
    for (auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        // Init
        compute_unit.skipped = false;

        for (auto & cu_connection : compute_unit.connectivity)
        {
            // Init
            cu_connection.dev_info_sizes_bytes.clear();
            cu_connection.dev_info_total_sizes_bytes = 0;

            for (const auto & mem_data_index : cu_connection.mem_data_indexes)
            {
                if (mem_data_index >= this->m_xclbin_metadata.memory_topology.size())
                {
                    LogMessage(MSG_ITF_143, {std::to_string(mem_data_index), compute_unit.name});
                    return RET_FAILURE;
                }

                auto xclbin_memory_data = this->m_xclbin_metadata.memory_topology[mem_data_index];

                uint64_t range_bytes;
                uint64_t base_address;
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
                // Get build time memory size from metadata as runtime info not reported by AMI or when running without driver
                range_bytes  = xclbin_memory_data.size_bytes;
                base_address = xclbin_memory_data.m_base_address;
#endif
#ifdef USE_XRT
                // For XRT, get run time memory size
                if (mem_data_index >= this->m_memory_info.memory.memories.size())
                {
                    LogMessage(MSG_ITF_142, {std::to_string(mem_data_index), compute_unit.name});
                    return RET_FAILURE;
                }
                auto mem_info = this->m_memory_info.memory.memories[mem_data_index];
                range_bytes  = mem_info.range_bytes;
                base_address = mem_info.base_address;
#endif
                cu_connection.dev_info_base_addresses.emplace_back(base_address);
                cu_connection.dev_info_sizes_bytes.emplace_back(range_bytes);
                cu_connection.dev_info_total_sizes_bytes += range_bytes;
                // Check host memory is allocated for HOST Memory xbtest HW IP.
                if ((compute_unit.mode == BI_MEM_HW_COMPONENT_ID) && (compute_unit.cu_type_configuration.memory->target == HOST) && (range_bytes == 0))
                {
                    if (!(GetIsDmaSupported()))
                    {
                        LogMessage(MSG_GEN_068);
                        return RET_FAILURE;
                    }
                    compute_unit.skipped = true;
                    LogMessage(MSG_GEN_037, {compute_unit.name, xclbin_memory_data.m_tag});
                }
            }
        }
    }

    // Get actual total memory size for each port based on the number of port connected to same vpp_sptag
    // This is currently used in OpenCL buffer creation
    // Do this only for Memory xbtest HW IP, as for PLRAM we use sub-buffers
    for (auto & memory : this->m_xbtest_configuration.memory)
    {
        memory.size_mb = 0; // Compute actual memory size. This is currently used in dma/p2p testcase
        auto size_set = false;
        for (const auto & vpp_sptag : memory.vpp_sptags)
        {
            // Compute total number of times vpp_sptag is used, this will be used to compute memory channel actual size
            // Mark each connection with vpp_sptag occurrence used to compute connection base address
            uint vpp_sptag_count = 0;
            for (auto & compute_unit : this->m_xbtest_configuration.compute_units)
            {
                if (compute_unit.mode != BI_MEM_HW_COMPONENT_ID)
                {
                    continue;
                }
                if (compute_unit.cu_type_configuration.memory->name != memory.name)
                {
                    continue;
                }
                for (auto & cu_connection : compute_unit.connectivity)
                {
                    if (vpp_sptag == cu_connection.vpp_sptag)
                    {
                        cu_connection.vpp_sptag_occurrence = vpp_sptag_count;
                        vpp_sptag_count++;
                    }
                }
            }
            LogMessage(MSG_DEBUG_SETUP, {"BD tag " + vpp_sptag + " found " + std::to_string(vpp_sptag_count) + " times"});

            uint memory_size_mb = 0;

            // Divide all dump size by the number of times same vpp_sptag was found and compute base address
            for (auto & compute_unit : this->m_xbtest_configuration.compute_units)
            {
                for (auto & cu_connection : compute_unit.connectivity)
                {
                    if (vpp_sptag == cu_connection.vpp_sptag)
                    {
                        // Initialize
                        cu_connection.actual_sizes_bytes.clear();
                        cu_connection.actual_total_size_bytes = 0;

                        // actual_sizes_bytes is rounded to lower even MB so when memory size is not multiple of total xbtest HW IP sizes then entire memory is not tested
                        for (const auto & size : cu_connection.dev_info_sizes_bytes)
                        {
                            auto actual_sizes_bytes = (uint64_t)(size)/(uint64_t)(vpp_sptag_count);
                            if (!size_set)
                            {
                                memory_size_mb += (uint64_t)(size)/(uint64_t)(1024)/(uint64_t)(1024);
                            }

                            // Round to lower MB as memory testcase settings in MB
                            actual_sizes_bytes /= (uint64_t)(1024)*(uint64_t)(1024);
                            actual_sizes_bytes *= (uint64_t)(1024)*(uint64_t)(1024);

                            // Round to even MB as memory testcase simultaneous requires
                            if (actual_sizes_bytes % 2 != 0)
                            {
                                actual_sizes_bytes -= 1;
                            }

                            cu_connection.actual_sizes_bytes.emplace_back(actual_sizes_bytes);
                            cu_connection.actual_total_size_bytes += actual_sizes_bytes;
                        }
                        // Find lowest base address if xbtest HW IP is connected to multiple memory bank (and GROUP_CONNECTIVITY not used)
                        auto min_base_address = cu_connection.dev_info_base_addresses[0];
                        for (const auto & base_address : cu_connection.dev_info_base_addresses)
                        {
                            if (min_base_address > base_address)
                            {
                                min_base_address = base_address;
                            }
                        }
                        cu_connection.actual_base_address = min_base_address + cu_connection.vpp_sptag_occurrence*cu_connection.actual_total_size_bytes;
                    }
                }
            }
            if (!size_set)
            {
                memory_size_mb /= (uint64_t)(vpp_sptag_count);
                memory.size_mb += memory_size_mb;
            }
            if (memory.type == SINGLE_CHANNEL) // For single channel xbtest HW IP set with size of first xbtest HW IP. Check size of other xbtest HW IPs
            {
                size_set = true;
            }
        }
    }
    // Compute actual memory size. This is currently used in memory testcase
    for (auto & memory : this->m_xbtest_configuration.memory)
    {
        // Init
        memory.cu_size_mb = 0;
        auto size_set = false;
        for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
        {
            if (compute_unit.mode != BI_MEM_HW_COMPONENT_ID)
            {
                continue;
            }
            if (compute_unit.cu_type_configuration.memory->name != memory.name)
            {
                continue;
            }
            for (uint idx = 1; idx < compute_unit.connectivity.size(); idx++) // ignore port_idx=0 which is PLRAM
            {
                auto cu_size_mb = compute_unit.connectivity[idx].actual_total_size_bytes/(uint64_t)(1024)/(uint64_t)(1024);
                if (!size_set)
                {
                    memory.cu_size_mb += cu_size_mb;
                }
                if (memory.type == SINGLE_CHANNEL) // For single channel xbtest HW IP set with size of first xbtest HW IP. Check size of other xbtest HW IPs
                {
                    size_set = true;
                    if (size_set)
                    {
                        if (memory.cu_size_mb != cu_size_mb)
                        {
                            LogMessage(MSG_DEBUG_EXCEPTION, {"Memory size (" + std::to_string(cu_size_mb) + ") for single-channel Memory xbtest HW IP (" + compute_unit.name + ") does not match size of first xbtest HW IP (" + std::to_string(memory.cu_size_mb) + ")"});
                        }
                    }
                }
            }
        }
    }
    return RET_SUCCESS;
}

bool XbtestSwConfig::ComputeNewHostMemSize( const uint64_t & total_plram_buffer_size )
{
    // This is needed for nodma
    // In nodma platform, buffers for on-board memory are shadowed in the host memory to perform data transfer from device to host.
    // Meaning that the area used for PLRAM status can't be used in the Host Memory xbtest HW IP testcase
    // We simply reduce the size of each tag of each channel of each host Memory xbtest HW IP by total_plram_buffer_size

    // Round to next MB
    auto unusable_size_mb_double    = (double)(total_plram_buffer_size) / (double)(1024) / (double)(1024);
    auto unusable_size_mb           = (uint64_t)(unusable_size_mb_double);
    if (unusable_size_mb_double > (double)(unusable_size_mb))
    {
        unusable_size_mb++;
    }
    auto unusable_size_bytes = unusable_size_mb * (uint64_t)(1024) * (uint64_t)(1024);

    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory.target != HOST)
        {
            continue;
        }
        for (const auto & vpp_sptag : memory.vpp_sptags)
        {
            for (auto & compute_unit : this->m_xbtest_configuration.compute_units)
            {
                for (uint cu_port_idx = 1; cu_port_idx < compute_unit.connectivity.size(); cu_port_idx++) // Ignore first connection (PLRAM)
                {
                    if (vpp_sptag == compute_unit.connectivity[cu_port_idx].vpp_sptag)
                    {
                        if (unusable_size_mb > 0)
                        {
                            LogMessage(MSG_DEBUG_SETUP, {std::to_string(unusable_size_mb) + " MB will not be used by channel " + std::to_string(cu_port_idx-1) + " of Memory xbtest HW IP " + compute_unit.name});
                        }
                        for (auto & actual_size_bytes : compute_unit.connectivity[cu_port_idx].actual_sizes_bytes)
                        {
                            actual_size_bytes -= unusable_size_bytes;
                            compute_unit.connectivity[cu_port_idx].actual_total_size_bytes -= unusable_size_bytes;
                        }
                    }
                }
            }
        }

    }
    // Compute actual memory size. This is currently used in memory testcase
    for (auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory.target == HOST)
        {
            uint64_t new_size_mb = 0;

            for (auto & compute_unit : this->m_xbtest_configuration.compute_units)
            {
                if (compute_unit.mode != BI_MEM_HW_COMPONENT_ID)
                {
                    continue;
                }
                if (compute_unit.cu_type_configuration.memory->name != memory.name)
                {
                    continue;
                }
                for (uint cu_port_idx=1; cu_port_idx<compute_unit.connectivity.size(); cu_port_idx++) // Ignore first connection (PLRAM)
                {
                    new_size_mb += compute_unit.connectivity[cu_port_idx].actual_total_size_bytes/(uint64_t)(1024)/(uint64_t)(1024);
                }
                if (memory.type == SINGLE_CHANNEL)
                {
                    break; // Set size only for first xbtest HW IP found for SC Memory xbtest HW IP. Assume all SC Memory xbtest HW IP of same type accesse memory of same size
                }
            }
            if (memory.size_mb != new_size_mb)
            {
                LogMessage(MSG_ITF_121, {memory.name, std::to_string(memory.size_mb), std::to_string(new_size_mb)});
            }
            if (new_size_mb < 1)
            {
                LogMessage(MSG_ITF_122, {memory.name, std::to_string(new_size_mb)});
                return RET_FAILURE;
            }
            memory.size_mb = new_size_mb;
        }
    }
    return RET_SUCCESS;
}

std::vector<uint> XbtestSwConfig::GetAvailablePowerSLRs()
{
    std::vector<uint> available_power_slrs;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_PWR_HW_COMPONENT_ID)
        {
            available_power_slrs.emplace_back(compute_unit.cu_type_configuration.SLR);
        }
    }
    return available_power_slrs;
}

std::vector<uint> XbtestSwConfig::GetAvailableGTMACs()
{
    std::vector<uint> available_gts;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_GT_MAC_HW_COMPONENT_ID)
        {
            available_gts.emplace_back(compute_unit.cu_type_configuration.gt_index);
        }
    }
    return available_gts;
}

std::vector<uint> XbtestSwConfig::GetAvailableGTLpbks()
{
    std::vector<uint> available_gts;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_GT_LPBK_HW_COMPONENT_ID)
        {
            available_gts.emplace_back(compute_unit.cu_type_configuration.gt_index);
        }
    }
    return available_gts;
}

std::vector<uint> XbtestSwConfig::GetAvailableGTPrbss()
{
    std::vector<uint> available_gts;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_GT_PRBS_HW_COMPONENT_ID)
        {
            available_gts.emplace_back(compute_unit.cu_type_configuration.gt_index);
        }
    }
    return available_gts;
}

std::vector<uint> XbtestSwConfig::GetAvailableGTFPrbss()
{
    std::vector<uint> available_gts;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_GTF_PRBS_HW_COMPONENT_ID)
        {
            for (uint gt_index = 0; gt_index < compute_unit.cu_type_configuration.gtf_indexes.size(); gt_index++)
            {
                auto enable = compute_unit.cu_type_configuration.gtf_indexes[gt_index];
                if (enable)
                {
                    available_gts.emplace_back(gt_index);
                }
            }
        }
    }
    return available_gts;
}

std::vector<uint> XbtestSwConfig::GetAvailableGTMPrbss()
{
    std::vector<uint> available_gts;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_GTM_PRBS_HW_COMPONENT_ID)
        {
            for (uint gt_index = 0; gt_index < compute_unit.cu_type_configuration.gtm_indexes.size(); gt_index++)
            {
                auto enable = compute_unit.cu_type_configuration.gtm_indexes[gt_index];
                if (enable)
                {
                    available_gts.emplace_back(gt_index);
                }
            }
        }
    }
    return available_gts;
}

std::vector<uint> XbtestSwConfig::GetAvailableGTYPPrbss()
{
    std::vector<uint> available_gts;
    for (const auto & compute_unit : this->m_xclbin_metadata.user_metadata.definition.compute_units)
    {
        if (compute_unit.mode == BI_GTYP_PRBS_HW_COMPONENT_ID)
        {
            for (uint gt_index = 0; gt_index < compute_unit.cu_type_configuration.gtyp_indexes.size(); gt_index++)
            {
                auto enable = compute_unit.cu_type_configuration.gtyp_indexes[gt_index];
                if (enable)
                {
                    available_gts.emplace_back(gt_index);
                }
            }
        }
    }
    return available_gts;
}

std::vector<uint> XbtestSwConfig::GetAvailableMultiGTPrbss( const std::string & multi_gt_name )
{
    std::vector<uint> available_gts;
    if (multi_gt_name == GTM_PRBS_MEMBER.name)
    {
        available_gts = GetAvailableGTMPrbss();
    }
    else if (multi_gt_name == GTYP_PRBS_MEMBER.name)
    {
         available_gts = GetAvailableGTYPPrbss();
    }
    else
    {
        available_gts = GetAvailableGTFPrbss();
    }
    return available_gts;
}

// Not possible to have different memory with same name
// Not possible to find a tag in multiple memories.
std::vector<std::string> XbtestSwConfig::GetAllAvailableMemories()
{
    std::vector<std::string> all_available_memories;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        all_available_memories.emplace_back(memory.name);
    }
    return all_available_memories;
}

std::vector<std::string> XbtestSwConfig::GetAvailableBoardMemories()
{
    std::vector<std::string> available_board_memories;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory.target == BOARD)
        {
            available_board_memories.emplace_back(memory.name);
        }
    }
    return available_board_memories;
}

std::vector<std::string> XbtestSwConfig::GetAvailableHostMemories()
{
    std::vector<std::string> available_host_memories;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory.target == HOST)
        {
            available_host_memories.emplace_back(memory.name);
        }
    }
    return available_host_memories;
}

std::vector<std::string> XbtestSwConfig::GetAvailableBoardTag()
{
    std::vector<std::string> available_board_tag;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory.target == BOARD)
        {
            available_board_tag.insert(available_board_tag.end(), memory.available_tag.begin(), memory.available_tag.end());
        }
    }
    return available_board_tag;
}

std::vector<std::string> XbtestSwConfig::GetAvailableBoardMemoriesAndTags()
{
    std::vector<std::string> available_board_memories_and_tag;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory.target == BOARD)
        {
            available_board_memories_and_tag.emplace_back(memory.name);
            available_board_memories_and_tag.insert(available_board_memories_and_tag.end(), memory.available_tag.begin(), memory.available_tag.end());
        }
    }
    return available_board_memories_and_tag;
}

std::vector<std::string> XbtestSwConfig::GetAvailableTag( const std::string & memory_name )
{
    std::vector<std::string> available_tag;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory_name == memory.name)
        {
            available_tag = memory.available_tag;
            break;
        }
    }
    return available_tag;
}

std::vector<std::string> XbtestSwConfig::GetAvailableSpTag( const std::string & memory_name )
{
    std::vector<std::string> vpp_sptags;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory_name == memory.name)
        {
            vpp_sptags = memory.vpp_sptags;
            break;
        }
    }
    return vpp_sptags;
}

uint XbtestSwConfig::GetMemoryNumCU( const std::string & memory_name )
{
    uint num_cu = 0;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory_name == memory.name)
        {
            num_cu = memory.num_cu;
            break;
        }
    }
    return num_cu;
}

uint XbtestSwConfig::GetMemoryNumChannel( const std::string & memory_name )
{
    uint num_channels = 0;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (memory_name == memory.name)
        {
            num_channels = memory.num_channels;
            break;
        }
    }
    return num_channels;
}

Xbtest_Memory_t XbtestSwConfig::GetMemorySettings( const uint & idx )
{
    Xbtest_Memory_t memory_o;
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        if (idx == memory.idx)
        {
            memory_o = memory;
            break;
        }
    }
    return memory_o;
}

bool XbtestSwConfig::IsOneMemCuSkipped( const std::string & memory_name )
{
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_MEM_HW_COMPONENT_ID) && (compute_unit.cu_type_configuration.memory->name == memory_name) && (compute_unit.skipped))
        {
            return true;
        }
    }
    return false;
}

uint XbtestSwConfig::GetMemCUNumChannels( const std::string & cu_name )
{
    uint num_channels = 0;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            num_channels = compute_unit.connectivity.size() - 1; // minus 1 for PLRAM
            break;
        }
    }
    return num_channels;
}

std::string XbtestSwConfig::GetMemCUTarget( const std::string & cu_name )
{
    std::string target;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_MEM_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            target = compute_unit.cu_type_configuration.memory->target;
            break;
        }
    }
    return target;
}

std::string XbtestSwConfig::GetCUConnectionVppSptag( const std::string & cu_name, const int & ch_idx )
{
    std::string vpp_sptag;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            vpp_sptag = compute_unit.connectivity[ch_idx+1].vpp_sptag;  // 0 is for PLRAM
            break;
        }
    }
    return vpp_sptag;
}

std::vector<std::string> XbtestSwConfig::GetCUConnectionMemTags( const std::string & cu_name, const int & ch_idx )
{
    std::vector<std::string> mem_tags;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            mem_tags = compute_unit.connectivity[ch_idx+1].mem_tags;  // 0 is for PLRAM
            break;
        }
    }
    return mem_tags;
}

std::vector<uint> XbtestSwConfig::GetCUConnectionMemDataIndexes( const std::string & cu_name, const int & ch_idx )
{
    std::vector<uint> mem_data_indexes;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            mem_data_indexes = compute_unit.connectivity[ch_idx+1].mem_data_indexes; // 0 is for PLRAM
            break;
        }
    }
    return mem_data_indexes;
}

std::vector<uint64_t> XbtestSwConfig::GetCuConnectionDeviceInfoSizes( const std::string & cu_name, const int & ch_idx )
{
    std::vector<uint64_t> dev_info_sizes_bytes;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            dev_info_sizes_bytes = compute_unit.connectivity[ch_idx+1].dev_info_sizes_bytes; // 0 is for PLRAM
            break;
        }
    }
    return dev_info_sizes_bytes;
}

std::vector<uint64_t> XbtestSwConfig::GetCuConnectionActualSizes( const std::string & cu_name, const int & ch_idx )
{
    std::vector<uint64_t> actual_sizes_bytes;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            actual_sizes_bytes = compute_unit.connectivity[ch_idx+1].actual_sizes_bytes; // 0 is for PLRAM
            break;
        }
    }
    return actual_sizes_bytes;
}

uint64_t XbtestSwConfig::GetCuConnectionActualBaseAddress( const std::string & cu_name, const int & ch_idx )
{
    uint64_t actual_base_address = 0;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            actual_base_address = compute_unit.connectivity[ch_idx+1].actual_base_address;  // 0 is for PLRAM
            break;
        }
    }
    return actual_base_address;
}

std::string XbtestSwConfig::GetPwrCUThrottleMode( const std::string & cu_name )
{
    std::string throttle_mode;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if (compute_unit.name == cu_name)
        {
            throttle_mode = compute_unit.cu_type_configuration.throttle_mode;
            break;
        }
    }
    return throttle_mode;
}

uint XbtestSwConfig::GetPwrUseAIE( const std::string & cu_name )
{
    uint use_aie = 0;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            use_aie = compute_unit.cu_type_configuration.use_aie;
            break;
        }
    }
    return use_aie;
}

bool XbtestSwConfig::GetPwrSupportAIE()
{
    auto support_aie = false;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.cu_type_configuration.use_aie == 1))
        {
            support_aie = true;
            break;
        }
    }
    return support_aie;
}

bool XbtestSwConfig::GetPwrAIEInfoExists( const std::string & cu_name )
{
    bool aie_exists = false;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            aie_exists = compute_unit.cu_type_configuration.aie.exists;
            break;
        }
    }
    return aie_exists;
}


uint XbtestSwConfig::GetPwrAIEFrequency( const std::string & cu_name )
{
    uint aie_freq = 0;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            aie_freq = compute_unit.cu_type_configuration.aie.freq;
            break;
        }
    }
    return aie_freq;
}

std::string XbtestSwConfig::GetPwrAIEType( const std::string & cu_name )
{
    std::string aie_type;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            aie_type = compute_unit.cu_type_configuration.aie.type;
            break;
        }
    }
    return aie_type;
}

std::string XbtestSwConfig::GetPwrAIEControl( const std::string & cu_name )
{
    std::string aie_control;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            aie_control = compute_unit.cu_type_configuration.aie.control;
            break;
        }
    }
    return aie_control;
}

std::string XbtestSwConfig::GetPwrAIEStatus( const std::string & cu_name )
{
    std::string aie_status;
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        if ((compute_unit.mode == BI_PWR_HW_COMPONENT_ID) && (compute_unit.name == cu_name))
        {
            aie_status = compute_unit.cu_type_configuration.aie.status;
            break;
        }
    }
    return aie_status;
}

Xbtest_GT_t XbtestSwConfig::GetGTSettings( const uint & gt_index )
{
    Xbtest_GT_t gt_o;
    for (const auto & gt : this->m_xbtest_configuration.gt)
    {
        if (gt.gt_index == gt_index)
        {
            gt_o = gt;
            break;
        }
    }
    return gt_o;
}

uint XbtestSwConfig::GetNumGT()
{
    return this->m_xbtest_configuration.gt.size();
}

uint XbtestSwConfig::GetNumGTPfm()
{
    return this->m_xbtest_pfm_def.gt.settings.size();
}

Design_Info_t XbtestSwConfig::GetDesignInfo()
{
    return this->m_design_info;
}

Pcie_Info_t XbtestSwConfig::GetPcieInfo()
{
    return this->m_pcie_info;
}

bool XbtestSwConfig::IsExpectedPcieSpeedWidth()
{
    return this->m_is_expected_pcie_speed_width;
}

std::vector<Design_Mac_Info_t> XbtestSwConfig::GetPlatformMacsInfo()
{
    return this->m_design_info.macs;
}

Device_State_Info_t XbtestSwConfig::GetDeviceStateInfo()
{
    return this->m_device_state_info;
}

Xbtest_Pfm_Def_Mmio_t XbtestSwConfig::GetMmioDefinitions()
{
    return this->m_xbtest_pfm_def.mmio;
}

std::vector<Xbtest_Memory_t> XbtestSwConfig::GetMemoryDefinitions()
{
    return this->m_xbtest_configuration.memory;
}

std::vector<Xbtest_Compute_Units_t> XbtestSwConfig::GetComputeUnitDefinitions()
{
    return this->m_xbtest_configuration.compute_units;
}

uint XbtestSwConfig::GetDownloadTime()
{
    return this->m_xbtest_pfm_def.runtime.download_time;
}

bool XbtestSwConfig::GetInMemTopology ( const std::string & mem_tag, Xclbin_Memory_Data_t & memory_data, const bool & expect_used )
{
    for (const auto & xclbin_memory_data : this->m_xclbin_metadata.memory_topology)
    {
        if (StrMatchNoCase(xclbin_memory_data.m_tag, mem_tag))
        {
            if ((xclbin_memory_data.m_used == 0) && (expect_used))
            {
                LogMessage(MSG_ITF_037, {"tag", mem_tag});
                return RET_FAILURE;
            }
            memory_data = xclbin_memory_data;
            LogMessage(MSG_DEBUG_SETUP, {"Found " + mem_tag + " in memory topology at memory data index: " + std::to_string(xclbin_memory_data.mem_data_idx)});
            return RET_SUCCESS;
        }
    }
    LogMessage(MSG_ITF_036, {"tag", mem_tag});
    return RET_FAILURE;
}

bool XbtestSwConfig::GetInMemTopology ( const uint & mem_data_idx, Xclbin_Memory_Data_t & memory_data, const bool & expect_used )
{
    for (const auto & xclbin_memory_data : this->m_xclbin_metadata.memory_topology)
    {
        if (mem_data_idx == xclbin_memory_data.mem_data_idx)
        {
            if ((xclbin_memory_data.m_used == 0) && (expect_used))
            {
                LogMessage(MSG_ITF_037, {"index", std::to_string(mem_data_idx)});
                return RET_FAILURE;
            }
            memory_data = xclbin_memory_data;
            LogMessage(MSG_DEBUG_SETUP, {"Found " + xclbin_memory_data.m_tag + " in memory topology at memory data index: " + std::to_string(mem_data_idx)});
            return RET_SUCCESS;
        }
    }
    LogMessage(MSG_ITF_036, {"index", std::to_string(mem_data_idx)});
    return RET_FAILURE;
}

bool XbtestSwConfig::GetDeviceMemInfo ( const uint & mem_data_idx, Mem_Info_t & mem_info )
{
    mem_info = RST_MEM_INFO;
    for (const auto & info : this->m_memory_info.memory.memories)
    {
        if (mem_data_idx == info.mem_data_idx)
        {
            mem_info = info;
            LogMessage(MSG_DEBUG_SETUP, {"Found data in memory topology at memory data index: " + std::to_string(mem_data_idx)});
            return RET_SUCCESS;
        }
    }
    LogMessage(MSG_ITF_036, {"index", std::to_string(mem_data_idx)});
    return RET_FAILURE;
}

uint XbtestSwConfig::GetClockFrequency( const uint & clock_idx )
{
    if (this->m_xclbin_metadata.clock_freq_topology.m_clock_freq[clock_idx].found)
    {
        return this->m_xclbin_metadata.clock_freq_topology.m_clock_freq[clock_idx].freq_Mhz;
    }
    return this->m_xclbin_metadata.user_metadata.definition.clocks[clock_idx];
}

Sensor_t XbtestSwConfig::GetSensorUsed()
{
    return this->m_sensor_used;
}

bool XbtestSwConfig::SensorOverrideJson( const Device_Mgmt_Type_Parameters_t & device_mgmt_param )
{
    LogMessage(MSG_DEBUG_SETUP, {"Sensor override from " + TEST_JSON});
    for (const auto & usr_source : device_mgmt_param.sensor)
    {
        if (CheckSensorThresholds(usr_source.id, SENSOR_WARNING_THRESHOLD_MEMBER.name, usr_source.warning_threshold) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (CheckSensorThresholds(usr_source.id, SENSOR_ERROR_THRESHOLD_MEMBER.name, usr_source.error_threshold) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (CheckSensorThresholds(usr_source.id, SENSOR_ABORT_THRESHOLD_MEMBER.name, usr_source.abort_threshold) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        auto sensor_exists      = false;
        auto sensor_is_present  = false;
        auto threshold_exist    =  usr_source.warning_threshold.min_exists || usr_source.error_threshold.min_exists || usr_source.abort_threshold.min_exists ||
                                   usr_source.warning_threshold.max_exists || usr_source.error_threshold.max_exists || usr_source.abort_threshold.max_exists;

        if (StrMatchNoCase(usr_source.type, XRT_INFO_DEVICE_ELECTRICAL))
        {
            for (auto & source : this->m_sensor_used.power_rail_sources)
            {
                if (StrMatchNoCase(usr_source.id, source.info.id))
                {
                    sensor_exists       = true;
                    sensor_is_present   = source.info.current.is_present || source.info.voltage.is_present;

                    if (sensor_is_present)
                    {
                        source.warning_threshold = usr_source.warning_threshold;
                        source.error_threshold   = usr_source.error_threshold;
                        source.abort_threshold   = usr_source.abort_threshold;
                    }
                    break;
                }
            }
            for (auto & source : this->m_sensor_used.power_consumption_sources)
            {
                if (StrMatchNoCase(usr_source.id, source.info.id))
                {
                    sensor_exists       = true;
                    sensor_is_present   = true;

                    if (sensor_is_present)
                    {
                        source.warning_threshold = usr_source.warning_threshold;
                        source.error_threshold   = usr_source.error_threshold;
                        source.abort_threshold   = usr_source.abort_threshold;
                    }
                    break;
                }
            }
        }
        else if (StrMatchNoCase(usr_source.type, XRT_INFO_DEVICE_THERMAL))
        {
            for (auto & source : this->m_sensor_used.thermal_sources)
            {
                if (StrMatchNoCase(usr_source.id, source.info.location_id))
                {
                    sensor_exists       = true;
                    sensor_is_present   = source.info.is_present;

                    if (sensor_is_present)
                    {
                        source.warning_threshold = usr_source.warning_threshold;
                        source.error_threshold   = usr_source.error_threshold;
                        source.abort_threshold   = usr_source.abort_threshold;
                    }
                    break;
                }
            }
        }
        else if (StrMatchNoCase(usr_source.type, XRT_INFO_DEVICE_MECHANICAL))
        {
            for (auto & source : this->m_sensor_used.fan_sources)
            {
                if (StrMatchNoCase(usr_source.id, source.info.location_id))
                {
                    sensor_exists       = true;
                    sensor_is_present   = source.info.is_present;

                    if (sensor_is_present)
                    {
                        source.warning_threshold = usr_source.warning_threshold;
                        source.error_threshold   = usr_source.error_threshold;
                        source.abort_threshold   = usr_source.abort_threshold;
                    }
                    break;
                }
            }
        }
        if (sensor_exists)
        {
            LogMessage(MSG_ITF_124, {usr_source.type, usr_source.id});
        }
        else
        {
            if (StrMatchNoCase(usr_source.type, XRT_INFO_DEVICE_ELECTRICAL))
            {
                for (const auto & source : this->m_sensor_all.power_rail_sources)
                {
                    if (StrMatchNoCase(source.info.id, usr_source.id))
                    {
                        sensor_exists       = true;
                        sensor_is_present   = source.info.current.is_present || source.info.voltage.is_present;

                        auto new_source = source;
                        if (sensor_is_present)
                        {
                            new_source.warning_threshold    = usr_source.warning_threshold;
                            new_source.error_threshold      = usr_source.error_threshold;
                            new_source.abort_threshold      = usr_source.abort_threshold;
                        }
                        this->m_sensor_used.power_rail_sources.emplace_back(new_source);
                    }
                }
                for (const auto & source : this->m_sensor_all.power_consumption_sources)
                {
                    if (StrMatchNoCase(source.info.id, usr_source.id))
                    {
                        sensor_exists       = true;
                        sensor_is_present   = true;

                        auto new_source = source;
                        if (sensor_is_present)
                        {
                            new_source.warning_threshold            = usr_source.warning_threshold;
                            new_source.error_threshold              = usr_source.error_threshold;
                            new_source.abort_threshold              = usr_source.abort_threshold;
                        }
                        this->m_sensor_used.power_consumption_sources.emplace_back(new_source);
                    }
                }
            }
            else if (StrMatchNoCase(usr_source.type, XRT_INFO_DEVICE_THERMAL))
            {
                for (const auto & source : this->m_sensor_all.thermal_sources)
                {
                    if (StrMatchNoCase(source.info.location_id, usr_source.id))
                    {
                        sensor_exists       = true;
                        sensor_is_present   = source.info.is_present;

                        auto new_source = source;
                        if (sensor_is_present)
                        {
                            new_source.warning_threshold    = usr_source.warning_threshold;
                            new_source.error_threshold      = usr_source.error_threshold;
                            new_source.abort_threshold      = usr_source.abort_threshold;
                        }
                        this->m_sensor_used.thermal_sources.emplace_back(new_source);
                    }
                }
            }
            else if (StrMatchNoCase(usr_source.type, XRT_INFO_DEVICE_MECHANICAL))
            {
                for (const auto & source : this->m_sensor_all.fan_sources)
                {
                    if (StrMatchNoCase(source.info.location_id, usr_source.id))
                    {
                        sensor_exists       = true;
                        sensor_is_present   = source.info.is_present;

                        auto new_source = source;
                        if (sensor_is_present)
                        {
                            new_source.warning_threshold    = usr_source.warning_threshold;
                            new_source.error_threshold      = usr_source.error_threshold;
                            new_source.abort_threshold      = usr_source.abort_threshold;
                        }
                        this->m_sensor_used.fan_sources.emplace_back(new_source);
                    }
                }
            }
            if (sensor_exists)
            {
                LogMessage(MSG_ITF_132, {usr_source.type, usr_source.id});
            }
        }
        if (!sensor_exists)
        {
            LogMessage(MSG_ITF_133, {usr_source.type, usr_source.id, TEST_JSON});
            return RET_FAILURE;
        }
        if (threshold_exist && !sensor_is_present)
        {
            LogMessage(MSG_ITF_135, {usr_source.type, usr_source.id});
        }
    }
    return RET_SUCCESS;
}

std::vector<std::string> XbtestSwConfig::GetAllMechanicalSourcesIDs()
{
    std::vector<std::string> ret;
    for (const auto & source : this->m_sensor_all.fan_sources)
    {
        if (source.info.is_present)
        {
            ret.emplace_back(source.info.location_id);
        }
    }
    return ret;
}

std::vector<std::string> XbtestSwConfig::GetAllElectricalSourcesIDs()
{
    auto ret  = GetAllPowerRailSourcesIDs();
    auto ret2 = GetAllPowerConsumptionSourceIDs();
    ret.insert(ret.end(), ret2.begin(), ret2.end());
    return ret;
}

std::vector<std::string> XbtestSwConfig::GetAllThermalSourcesIDs()
{
    std::vector<std::string> ret;
    for (const auto & source : this->m_sensor_all.thermal_sources)
    {
        if (source.info.is_present)
        {
            ret.emplace_back(source.info.location_id);
        }
    }
    return ret;
}

uint XbtestSwConfig::GetHwVersionMajor()
{
    return this->m_xclbin_metadata.user_metadata.build_info.xbtest.version.major;
}
uint XbtestSwConfig::GetHwVersionMinor()
{
    return this->m_xclbin_metadata.user_metadata.build_info.xbtest.version.minor;
}
double XbtestSwConfig::GetHwVersionMajorMinor()
{
    return (double)(GetHwVersionMajor()) + (double)(GetHwVersionMinor())/(double)(10.0);
}
bool XbtestSwConfig::IsMemoryPatternSupported()
{
    return this->m_is_memory_pattern_supported;
}

void XbtestSwConfig::PrintActualConfig()
{
    if (this->m_xclbin_metadata.user_metadata.build_info.xbtest.internal_release)
    {
        LogMessage(MSG_ITF_137);
    }
    LogMessage(MSG_ITF_052, {"Download time (s): " + std::to_string(this->m_xbtest_pfm_def.runtime.download_time)});
    LogMessage(MSG_ITF_057, {"Available SLRs for power test: " + NumVectToStr<uint>(GetAvailablePowerSLRs(), ",")});

    for (const auto & source : this->m_sensor_used.power_rail_sources)
    {
        LogMessage(MSG_ITF_055, {source.type + " source " + source.info.id + " configuration:"});
        LogMessage(MSG_ITF_055, {"\t - ID                : " +           source.info.id});
        LogMessage(MSG_ITF_055, {"\t - Current is present: " + BoolToStr(source.info.current.is_present)});
        LogMessage(MSG_ITF_055, {"\t - Voltage is present: " + BoolToStr(source.info.voltage.is_present)});
        if (source.info.current.is_present && source.info.voltage.is_present)
        {
            PrintSensorLimit("Warning", source.unit_power, source.warning_threshold);
            PrintSensorLimit("Error",   source.unit_power, source.error_threshold);
            PrintSensorLimit("Abort",   source.unit_power, source.abort_threshold);
        }
        else if (source.info.current.is_present)
        {
            PrintSensorLimit("Warning", source.unit_current, source.warning_threshold);
            PrintSensorLimit("Error",   source.unit_current, source.error_threshold);
            PrintSensorLimit("Abort",   source.unit_current, source.abort_threshold);
        }
        else if (source.info.voltage.is_present)
        {
            PrintSensorLimit("Warning", source.unit_voltage, source.warning_threshold);
            PrintSensorLimit("Error",   source.unit_voltage, source.error_threshold);
            PrintSensorLimit("Abort",   source.unit_voltage, source.abort_threshold);
        }
    }
    for (const auto & source : this->m_sensor_used.power_consumption_sources)
    {
        LogMessage(MSG_ITF_055, {source.type + " source " + source.info.id + " configuration:"});
        LogMessage(MSG_ITF_055, {"\t - ID: " + source.info.id});
        PrintSensorLimit("Warning", source.unit, source.warning_threshold);
        PrintSensorLimit("Error",   source.unit, source.error_threshold);
        PrintSensorLimit("Abort",   source.unit, source.abort_threshold);
    }
    for (const auto & source : this->m_sensor_used.thermal_sources)
    {
        LogMessage(MSG_ITF_055, {source.type + " source " + source.info.location_id + " configuration:"});
        LogMessage(MSG_ITF_055, {"\t - ID        : " +           source.info.location_id});
        LogMessage(MSG_ITF_055, {"\t - Is present: " + BoolToStr(source.info.is_present)});
        if (source.info.is_present)
        {
            PrintSensorLimit("Warning", source.unit, source.warning_threshold);
            PrintSensorLimit("Error",   source.unit, source.error_threshold);
            PrintSensorLimit("Abort",   source.unit, source.abort_threshold);
        }
    }
    for (const auto & source : this->m_sensor_used.fan_sources)
    {
        LogMessage(MSG_ITF_055, {source.type + " source " + source.info.location_id + " configuration:"});
        LogMessage(MSG_ITF_055, {"\t - ID           : " +           source.info.location_id});
        LogMessage(MSG_ITF_055, {"\t - Is present   : " + BoolToStr(source.info.is_present)});
        if (source.info.is_present)
        {
            PrintSensorLimit("Warning", source.unit, source.warning_threshold);
            PrintSensorLimit("Error",   source.unit, source.error_threshold);
            PrintSensorLimit("Abort",   source.unit, source.abort_threshold);
        }
    }
    // From xbtest_pfm_def + USER_METADATA
    for (const auto & gt : this->m_xbtest_configuration.gt)
    {
        LogMessage(MSG_ITF_088, {"GT " + std::to_string(gt.gt_index) + " configuration:"});
        if (gt.name != "none")
        {
            LogMessage(MSG_ITF_088, {"\t - GT name             : " + gt.name});
        }
        LogMessage(MSG_ITF_088, {"\t - GT group select     : " + gt.gt_group_select});
        LogMessage(MSG_ITF_088, {"\t - GT serial port      : " + gt.gt_serial_port});
        LogMessage(MSG_ITF_088, {"\t - GT diff clock       : " + gt.gt_diff_clocks});

        LogMessage(MSG_ITF_088, {"\t - Transceiver settings:"});
        LogMessage(MSG_ITF_088, {"\t\t * Cable: "});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX polarity                  : " +                gt.transceivers.cable.tx_polarity});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX main cursor               : " + std::to_string(gt.transceivers.cable.tx_main_cursor)});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX differential swing control: " + std::to_string(gt.transceivers.cable.tx_differential_swing_control)});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX pre emphasis              : " + std::to_string(gt.transceivers.cable.tx_pre_emphasis)});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX post emphasis             : " + std::to_string(gt.transceivers.cable.tx_post_emphasis)});
        LogMessage(MSG_ITF_088, {"\t\t\t + RX equaliser                 : " +                gt.transceivers.cable.rx_equalizer});
        LogMessage(MSG_ITF_088, {"\t\t\t + RX polarity                  : " +                gt.transceivers.cable.rx_polarity});
        LogMessage(MSG_ITF_088, {"\t\t * Module: "});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX polarity                  : " +                gt.transceivers.module.tx_polarity});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX main cursor               : " + std::to_string(gt.transceivers.module.tx_main_cursor)});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX differential swing control: " + std::to_string(gt.transceivers.module.tx_differential_swing_control)});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX pre emphasis              : " + std::to_string(gt.transceivers.module.tx_pre_emphasis)});
        LogMessage(MSG_ITF_088, {"\t\t\t + TX post emphasis             : " + std::to_string(gt.transceivers.module.tx_post_emphasis)});
        LogMessage(MSG_ITF_088, {"\t\t\t + RX equaliser                 : " +                gt.transceivers.module.rx_equalizer});
        LogMessage(MSG_ITF_088, {"\t\t\t + RX polarity                  : " +                gt.transceivers.module.rx_polarity});
    }
    if (GetIsMmioTestSupported())
    {
        LogMessage(MSG_ITF_058, {"MMIO BW thresholds (MBps):"});
        LogMessage(MSG_ITF_058, {"\t * Write: [" + std::to_string(this->m_xbtest_pfm_def.mmio.bw.write.low) + ";" + std::to_string(this->m_xbtest_pfm_def.mmio.bw.write.high) + "]"});
        LogMessage(MSG_ITF_058, {"\t * Read : [" + std::to_string(this->m_xbtest_pfm_def.mmio.bw.read.low)  + ";" + std::to_string(this->m_xbtest_pfm_def.mmio.bw.read.high)  + "]"});
        if (this->m_xbtest_pfm_def.mmio.config_exists)
        {
            LogMessage(MSG_ITF_058, {"MMIO Configuration:"});
            if (this->m_xbtest_pfm_def.mmio.config.total_size_exists)
            {
                LogMessage(MSG_ITF_058, {"\t * Total size (MB): " + std::to_string(this->m_xbtest_pfm_def.mmio.config.total_size)});
            }
            if (this->m_xbtest_pfm_def.mmio.config.buffer_size_exists)
            {
                LogMessage(MSG_ITF_058, {"\t * Buffer size (MB): " + std::to_string(this->m_xbtest_pfm_def.mmio.config.buffer_size)});
            }
            if (this->m_xbtest_pfm_def.mmio.config.bar_exists)
            {
                LogMessage(MSG_ITF_058, {"\t * BAR: " + std::to_string(this->m_xbtest_pfm_def.mmio.config.bar)});
            }
            if (this->m_xbtest_pfm_def.mmio.config.offset_exists)
            {
                LogMessage(MSG_ITF_058, {"\t * Offset: " + std::to_string(this->m_xbtest_pfm_def.mmio.config.offset)});
            }
        }
    }
    for (const auto & memory : this->m_xbtest_configuration.memory)
    {
        LogMessage(MSG_ITF_058, {"Memory " + memory.name + " configuration:"});
        LogMessage(MSG_ITF_058, {"\t - Name                     : " +                memory.name});
        LogMessage(MSG_ITF_058, {"\t - Index                    : " + std::to_string(memory.idx)});
        // LogMessage(MSG_ITF_058, {"\t - Memory size (MB)         : " + std::to_string(memory.size)}); // Only available after step 2
        LogMessage(MSG_ITF_058, {"\t - Type                     : " +                memory.type});
        LogMessage(MSG_ITF_058, {"\t - Target                   : " +                memory.target});
        LogMessage(MSG_ITF_058, {"\t - Number of channels       : " + std::to_string(memory.num_channels)});
        LogMessage(MSG_ITF_058, {"\t - Number of xbtest HW IPs  : " + std::to_string(memory.num_cu)});
        LogMessage(MSG_ITF_058, {"\t - AXI data size (bits)     : " + std::to_string(memory.axi_data_size)});
        LogMessage(MSG_ITF_058, {"\t - Number of AXI Threads    : " + std::to_string(memory.num_axi_thread)});
        if (memory.target == BOARD)
        {
            LogMessage(MSG_ITF_058, {"\t - Tag                  : " + StrVectToStr(memory.available_tag, ", ")});
            if (GetIsDmaSupported())
            {
                LogMessage(MSG_ITF_058, {"\t - BW thresholds (MBps) for each tag in DMA test:"});
                LogMessage(MSG_ITF_058, {"\t\t * Write: [" + std::to_string(memory.dma_bw.write.low) + ";" + std::to_string(memory.dma_bw.write.high) + "]"});
                LogMessage(MSG_ITF_058, {"\t\t * Read : [" + std::to_string(memory.dma_bw.read.low)  + ";" + std::to_string(memory.dma_bw.read.high)  + "]"});
                if (memory.dma_config_exists)
                {
                    LogMessage(MSG_ITF_058, {"\t - Configuration for each tag in DMA test:"});
                    if (memory.dma_config.total_size_exists)
                    {
                        LogMessage(MSG_ITF_058, {"\t\t * Total size (MB): " + std::to_string(memory.dma_config.total_size)});
                    }
                    if (memory.dma_config.buffer_size_exists)
                    {
                        LogMessage(MSG_ITF_058, {"\t\t * Buffer size (MB): " + std::to_string(memory.dma_config.buffer_size)});
                    }
                }
            }
            if (GetIsP2PSupported())
            {
                LogMessage(MSG_ITF_058, {"\t - BW thresholds (MBps) for each tag in P2P_CARD test:"});
                LogMessage(MSG_ITF_058, {"\t\t * Write: [" + std::to_string(memory.p2p_card_bw.write.low) + ";" + std::to_string(memory.p2p_card_bw.write.high) + "]"});
                LogMessage(MSG_ITF_058, {"\t\t * Read : [" + std::to_string(memory.p2p_card_bw.read.low)  + ";" + std::to_string(memory.p2p_card_bw.read.high)  + "]"});
                if (memory.p2p_card_config_exists)
                {
                    LogMessage(MSG_ITF_058, {"\t - Configuration for each tag in P2P_CARD test:"});
                    if (memory.p2p_card_config.total_size_exists)
                    {
                        LogMessage(MSG_ITF_058, {"\t\t * Total size (MB): " + std::to_string(memory.p2p_card_config.total_size)});
                    }
                    if (memory.p2p_card_config.buffer_size_exists)
                    {
                        LogMessage(MSG_ITF_058, {"\t\t * Buffer size (MB): " + std::to_string(memory.p2p_card_config.buffer_size)});
                    }
                }
                LogMessage(MSG_ITF_058, {"\t - BW thresholds (MBps) for each tag in P2P_NVME test:"});
                LogMessage(MSG_ITF_058, {"\t\t * Write: [" + std::to_string(memory.p2p_nvme_bw.write.low) + ";" + std::to_string(memory.p2p_nvme_bw.write.high) + "]"});
                LogMessage(MSG_ITF_058, {"\t\t * Read : [" + std::to_string(memory.p2p_nvme_bw.read.low)  + ";" + std::to_string(memory.p2p_nvme_bw.read.high)  + "]"});
                if (memory.p2p_nvme_config_exists)
                {
                    LogMessage(MSG_ITF_058, {"\t - Configuration for each tag in P2P_NVME test:"});
                    if (memory.p2p_nvme_config.total_size_exists)
                    {
                        LogMessage(MSG_ITF_058, {"\t\t * Total size (MB): " + std::to_string(memory.p2p_nvme_config.total_size)});
                    }
                    if (memory.p2p_nvme_config.buffer_size_exists)
                    {
                        LogMessage(MSG_ITF_058, {"\t\t * Buffer size (MB): " + std::to_string(memory.p2p_nvme_config.buffer_size)});
                    }
                }
            }
        }

        LogMessage(MSG_ITF_058, {"\t - BW thresholds (MBps) for each xbtest HW IP channel in memory test:"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: [" + std::to_string(memory.cu_bw.alt_wr_rd.write.low)   + ";" + std::to_string(memory.cu_bw.alt_wr_rd.write.high)   + "]"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : [" + std::to_string(memory.cu_bw.alt_wr_rd.read.low)    + ";" + std::to_string(memory.cu_bw.alt_wr_rd.read.high)    + "]"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: [" + std::to_string(memory.cu_bw.only_wr.write.low)     + ";" + std::to_string(memory.cu_bw.only_wr.write.high)     + "]"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : [" + std::to_string(memory.cu_bw.only_rd.read.low)      + ";" + std::to_string(memory.cu_bw.only_rd.read.high)      + "]"});
        LogMessage(MSG_ITF_058, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: [" + std::to_string(memory.cu_bw.simul_wr_rd.write.low) + ";" + std::to_string(memory.cu_bw.simul_wr_rd.write.high) + "]"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : [" + std::to_string(memory.cu_bw.simul_wr_rd.read.low)  + ";" + std::to_string(memory.cu_bw.simul_wr_rd.read.high)  + "]"});

        LogMessage(MSG_ITF_058, {"\t - Latency thresholds (ns) for each xbtest HW IP channel in memory test:"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: [" + std::to_string(memory.cu_latency.alt_wr_rd.write.low)   + ";" + std::to_string(memory.cu_latency.alt_wr_rd.write.high)   + "]"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : [" + std::to_string(memory.cu_latency.alt_wr_rd.read.low)    + ";" + std::to_string(memory.cu_latency.alt_wr_rd.read.high)    + "]"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: [" + std::to_string(memory.cu_latency.only_wr.write.low)     + ";" + std::to_string(memory.cu_latency.only_wr.write.high)     + "]"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : [" + std::to_string(memory.cu_latency.only_rd.read.low)      + ";" + std::to_string(memory.cu_latency.only_rd.read.high)      + "]"});
        LogMessage(MSG_ITF_058, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: [" + std::to_string(memory.cu_latency.simul_wr_rd.write.low) + ";" + std::to_string(memory.cu_latency.simul_wr_rd.write.high) + "]"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : [" + std::to_string(memory.cu_latency.simul_wr_rd.read.low)  + ";" + std::to_string(memory.cu_latency.simul_wr_rd.read.high)  + "]"});

        LogMessage(MSG_ITF_058, {"\t - Nominal rate (%) for each xbtest HW IP channel in memory test:"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: " + std::to_string(memory.cu_rate.alt_wr_rd.write.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : " + std::to_string(memory.cu_rate.alt_wr_rd.read.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: " + std::to_string(memory.cu_rate.only_wr.write.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : " + std::to_string(memory.cu_rate.only_rd.read.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Write: " + std::to_string(memory.cu_rate.simul_wr_rd.write.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t\t + Read : " + std::to_string(memory.cu_rate.simul_wr_rd.read.nominal)});

        LogMessage(MSG_ITF_058, {"\t - Nominal maximum number of outstanding for each xbtest HW IP channel in memory test:"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Writes: " + std::to_string(memory.cu_outstanding.alt_wr_rd.write.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t\t + Reads : " + std::to_string(memory.cu_outstanding.alt_wr_rd.read.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Writes: " + std::to_string(memory.cu_outstanding.only_wr.write.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Reads : " + std::to_string(memory.cu_outstanding.only_rd.read.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_ITF_058, {"\t\t\t + Writes: " + std::to_string(memory.cu_outstanding.simul_wr_rd.write.nominal)});
        LogMessage(MSG_ITF_058, {"\t\t\t + Reads : " + std::to_string(memory.cu_outstanding.simul_wr_rd.read.nominal)});

        LogMessage(MSG_ITF_058, {"\t - Nominal burst size (Bytes) for each xbtest HW IP channel in memory test:"});
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_WR + " mode:"});
        if (memory.cu_burst_size.only_wr.write.exists)
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Write: " + std::to_string(memory.cu_burst_size.only_wr.write.nominal)});
        }
        else
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Write: see memory TC"});
        }
        LogMessage(MSG_ITF_058, {"\t\t * " + ONLY_RD + " mode:"});
        if (memory.cu_burst_size.only_rd.read.exists)
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Read : " + std::to_string(memory.cu_burst_size.only_rd.read.nominal)});
        }
        else
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Read : see memory TC"});
        }
        LogMessage(MSG_ITF_058, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        if (memory.cu_burst_size.alt_wr_rd.write.exists)
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Write: " + std::to_string(memory.cu_burst_size.alt_wr_rd.write.nominal)});
        }
        else
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Write: see memory TC"});
        }
        if (memory.cu_burst_size.alt_wr_rd.read.exists)
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Read : " + std::to_string(memory.cu_burst_size.alt_wr_rd.read.nominal)});
        }
        else
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Read : see memory TC"});
        }
        LogMessage(MSG_ITF_058, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        if (memory.cu_burst_size.simul_wr_rd.write.exists)
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Write: " + std::to_string(memory.cu_burst_size.simul_wr_rd.write.nominal)});
        }
        else
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Write: see memory TC"});
        }
        if (memory.cu_burst_size.simul_wr_rd.read.exists)
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Read : " + std::to_string(memory.cu_burst_size.simul_wr_rd.read.nominal)});
        }
        else
        {
            LogMessage(MSG_ITF_058, {"\t\t\t + Read : see memory TC"});
        }
    }
    for (const auto & compute_unit : this->m_xbtest_configuration.compute_units)
    {
        LogMessage(MSG_ITF_100, {"xbtest HW IP " + compute_unit.name + " configuration:"});
        LogMessage(MSG_ITF_100, {"\t - Name            : " +                compute_unit.name});
        LogMessage(MSG_ITF_100, {"\t - SLR             : " + std::to_string(compute_unit.SLR)});
        LogMessage(MSG_ITF_100, {"\t - Mode            : " + std::to_string(compute_unit.mode)});
        LogMessage(MSG_ITF_100, {"\t - IP layout index : " + std::to_string(compute_unit.m_ip_layout_index)});
        if (compute_unit.connectivity.empty())
        {
            LogMessage(MSG_ITF_100, {"\t - No port present in connectivity"});
        }
        else
        {
            for (uint port_idx = 0; port_idx < compute_unit.connectivity.size(); port_idx++)
            {
                LogMessage(MSG_ITF_100, {"\t - Port [" + std::to_string(port_idx) + "] configuration:"});
                LogMessage(MSG_ITF_100, {"\t\t * BD tag(s)                         : " +                        compute_unit.connectivity[port_idx].vpp_sptag});
                LogMessage(MSG_ITF_100, {"\t\t * Memory topology tag(s)            : " +           StrVectToStr(compute_unit.connectivity[port_idx].mem_tags, ", ")});
                LogMessage(MSG_ITF_100, {"\t\t * Memory topology index(es)         : " +     NumVectToStr<uint>(compute_unit.connectivity[port_idx].mem_data_indexes, ", ")});
                // LogMessage(MSG_ITF_100, {"\t\t * Memory topology size(s)    (Bytes): " + NumVectToStr<uint64_t>(compute_unit.connectivity[port_idx].xclbin_size, ", ")});
                // LogMessage(MSG_ITF_100, {"\t\t * Total memory topology size (Bytes): " +         std::to_string(compute_unit.connectivity[port_idx].xclbin_total_size)});
            }
        }

        LogMessage(MSG_ITF_100, {"\t - xbtest HW IP type specific configuration:"});
        switch (compute_unit.mode)
        {
            case BI_VERIFY_HW_COMPONENT_ID:
            {
                LogMessage(MSG_ITF_100, {"\t\t * dna_read     : " + BoolToStr(compute_unit.cu_type_configuration.dna_read)});
                break;
            }
            case BI_PWR_HW_COMPONENT_ID:
            {
                LogMessage(MSG_ITF_100, {"\t\t * SLR          : " + std::to_string(compute_unit.cu_type_configuration.SLR)});
                LogMessage(MSG_ITF_100, {"\t\t * Throttle mode: " +                compute_unit.cu_type_configuration.throttle_mode});
                if (compute_unit.cu_type_configuration.use_aie == 1)
                {
                    LogMessage(MSG_ITF_100, {"\t\t * AIE          : Used"});
                }
                if (compute_unit.cu_type_configuration.aie.exists)
                {
                    LogMessage(MSG_ITF_100, {"\t\t\t + Frequency (MHz): " + std::to_string(compute_unit.cu_type_configuration.aie.freq)});
                    LogMessage(MSG_ITF_100, {"\t\t\t + Type           : " +                compute_unit.cu_type_configuration.aie.type});
                    LogMessage(MSG_ITF_100, {"\t\t\t + Control        : " +                compute_unit.cu_type_configuration.aie.control});
                    LogMessage(MSG_ITF_100, {"\t\t\t + Status         : " +                compute_unit.cu_type_configuration.aie.status});
                }
                break;
            }
            case BI_MEM_HW_COMPONENT_ID:
            {
                LogMessage(MSG_ITF_100, {"\t\t * Memory name: " + compute_unit.cu_type_configuration.memory->name});
                break;
            }
            case BI_GT_MAC_HW_COMPONENT_ID:
            {
                LogMessage(MSG_ITF_100, {"\t\t * GT index: " + std::to_string(compute_unit.cu_type_configuration.gt->gt_index)});
                break;
            }
            case BI_GT_LPBK_HW_COMPONENT_ID:
            {
                LogMessage(MSG_ITF_100, {"\t\t * GT index: " + std::to_string(compute_unit.cu_type_configuration.gt->gt_index)});
                break;
            }
            case BI_GT_PRBS_HW_COMPONENT_ID:
            {
                LogMessage(MSG_ITF_100, {"\t\t * GT index: " + std::to_string(compute_unit.cu_type_configuration.gt->gt_index)});
                break;
            }
            case BI_GTF_PRBS_HW_COMPONENT_ID:
            {
                std::vector<uint> gtf_indexes;
                for (auto gt : compute_unit.cu_type_configuration.gts)
                {
                    gtf_indexes.emplace_back(gt->gt_index);
                }
                LogMessage(MSG_ITF_100, {"\t\t * GTF indexes: " + NumVectToStr<uint>(gtf_indexes, ",")});
                break;
            }
            default:
            {
                LogMessage(MSG_ITF_100, {"\t\t * No parameter defined"});
                break;
            }
        }
    }
}

void XbtestSwConfig::LogMessage( const Message_t & message, const std::vector<std::string> & arg_list )
{
    if (GetCommandLineP2pTargetBdf().exists)
    {
        this->m_log->LogMessage(this->m_log_header, this->m_xbtest_configuration.bdf, "", message, arg_list);
    }
    else
    {
        this->m_log->LogMessage(this->m_log_header, "", "", message, arg_list);
    }
}

void XbtestSwConfig::LogMessage( const Message_t & message )
{
    if (GetCommandLineP2pTargetBdf().exists)
    {
        this->m_log->LogMessage(this->m_log_header, this->m_xbtest_configuration.bdf, "", message);
    }
    else
    {
        this->m_log->LogMessage(this->m_log_header, "", "", message);
    }
}

void XbtestSwConfig::SetLogDir( const std::string & log_dir )
{
    this->m_xbtest_configuration.logging.log_dir = log_dir;
}

void XbtestSwConfig::SetLogDirCreated()
{
    this->m_xbtest_configuration.logging.log_dir_created = true;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineXbtestpfmdef()
{
    return this->m_xbtest_configuration.command_line.xbtest_pfm_def;
}

Xbtest_CL_option_Str_t XbtestSwConfig::GetCommandLineP2PXbtestpfmdef()
{
    return this->m_xbtest_configuration.command_line.p2p_xbtest_pfm_def;
}

void XbtestSwConfig::SetBdf( const std::string & bdf )
{
    this->m_xbtest_configuration.bdf = bdf;
}

void XbtestSwConfig::SetXbtestpfmdef( const std::string & xbtest_pfm_def )
{
    this->m_xbtest_configuration.xbtest_pfm_def = xbtest_pfm_def;
}

void XbtestSwConfig::SetXclbin( const std::string & xclbin )
{
    this->m_xbtest_configuration.xclbin = xclbin;
}

void XbtestSwConfig::SetDesignPdi( const std::string & design_pdi )
{
    this->m_xbtest_configuration.design_pdi = design_pdi;
}

bool XbtestSwConfig::ParseDefaultSensor()
{
    auto electrical_info    = RST_ELECTRICAL_INFO;
    auto thermal_info       = RST_THERMAL_INFO;
    auto mechanical_info    = RST_MECHANICAL_INFO;

    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetElectricalInfo(electrical_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {XRT_INFO_DEVICE_ELECTRICAL});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetThermalInfo(thermal_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {XRT_INFO_DEVICE_THERMAL});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    for (uint i = 0; i < API_FAIL_ERROR_LIMIT; i++)
    {
        if (this->m_device_info_parser->GetMechanicalInfo(mechanical_info) == RET_SUCCESS)
        {
            break;
        }
        if (i == API_FAIL_ERROR_LIMIT-1)
        {
            LogMessage(MSG_ITF_011, {XRT_INFO_DEVICE_MECHANICAL});
            return RET_FAILURE;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Parse present sensor (all)
    // store all sensor available
    LogMessage(MSG_DEBUG_PARSING, {"Get " + XRT_INFO_DEVICE_ELECTRICAL + " sources"});
    for (const auto & info : electrical_info.power_rails)
    {
        LogMessage(MSG_DEBUG_PARSING, {"\t - Found " + XRT_INFO_DEVICE_ELECTRICAL + " source " + info.id});
        auto source = RST_POWER_POWER_RAIL_SOURCE;
        source.display_name_power          = info.id + " power";
        source.display_name_voltage_value  = info.id + " voltage";
        source.display_name_voltage_status = info.id + " voltage status";
        source.display_name_current_value  = info.id + " current";
        source.display_name_current_status = info.id + " current status";
        source.info                        = info;
        this->m_sensor_all.power_rail_sources.emplace_back(source);
    }
    for (const auto & info : electrical_info.power_consumptions)
    {
        LogMessage(MSG_DEBUG_PARSING, {"\t - Found " + XRT_INFO_DEVICE_ELECTRICAL + " source " + info.id});
        auto source = RST_POWER_CONSUMPTION_SOURCE;
        source.display_name_value  = info.id + " power";
        source.display_name_status = info.id + " power status";
        source.info                = info;
        this->m_sensor_all.power_consumption_sources.emplace_back(source);
    }
    LogMessage(MSG_DEBUG_PARSING, {"Get " + XRT_INFO_DEVICE_THERMAL + " sources"});
    for (const auto & info : thermal_info.thermals)
    {
        LogMessage(MSG_DEBUG_PARSING, {"\t - Found " + XRT_INFO_DEVICE_THERMAL + " source " + info.location_id});
        auto source = RST_THERMAL_SOURCE;
        source.display_name_value  = info.location_id + " temperature";
        source.display_name_status = info.location_id + " temperature status";
        source.info                = info;
        this->m_sensor_all.thermal_sources.emplace_back(source);
    }
    LogMessage(MSG_DEBUG_PARSING, {"Get " + XRT_INFO_DEVICE_MECHANICAL + " sources"});
    for (const auto & info : mechanical_info.fans)
    {
        LogMessage(MSG_DEBUG_PARSING, {"\t - Found " + XRT_INFO_DEVICE_MECHANICAL + " source " + info.location_id});
        auto source = RST_FAN_SOURCE;
        source.display_name_value  = info.location_id + " fan";
        source.display_name_status = info.location_id + " fan status";
        source.info                = info;
        this->m_sensor_all.fan_sources.emplace_back(source);
    }

    // Parse default sensor
    // from all sensor available, store the default one
    LogMessage(MSG_DEBUG_PARSING, {"Get default " + XRT_INFO_DEVICE_ELECTRICAL + " sources"});
    for (const auto & source : this->m_sensor_all.power_rail_sources)
    {
        if (FindStringInVect(source.info.id, DEFAULT_POWER_RAIL_SOURCES))
        {
            LogMessage(MSG_DEBUG_PARSING, {"\t - Using default " + XRT_INFO_DEVICE_ELECTRICAL + " source " + source.info.id});
            this->m_sensor_used.power_rail_sources.emplace_back(source);
        }
    }
    for (const auto & source : this->m_sensor_all.power_consumption_sources)
    {
        if (FindStringInVect(source.info.id, DEFAULT_POWER_CONSUMPTION_SOURCES))
        {
            LogMessage(MSG_DEBUG_PARSING, {"\t - Using default " + XRT_INFO_DEVICE_ELECTRICAL + " source " + source.info.id});
            this->m_sensor_used.power_consumption_sources.emplace_back(source);
        }
    }
    LogMessage(MSG_DEBUG_PARSING, {"Get default " + XRT_INFO_DEVICE_THERMAL + " sources"});
    for (const auto & source : this->m_sensor_all.thermal_sources)
    {
        if (FindStringInVect(source.info.location_id, DEFAULT_THERMAL_SOURCES))
        {
            LogMessage(MSG_DEBUG_PARSING, {"\t - Using default " + XRT_INFO_DEVICE_THERMAL + " source " + source.info.location_id});
            this->m_sensor_used.thermal_sources.emplace_back(source);
        }
    }
    LogMessage(MSG_DEBUG_PARSING, {"Get default " + XRT_INFO_DEVICE_MECHANICAL + " sources"});
    for (const auto & source : this->m_sensor_all.fan_sources)
    {
        if (FindStringInVect(source.info.location_id, DEFAULT_MECHANICAL_SOURCES))
        {
            LogMessage(MSG_DEBUG_PARSING, {"\t - Using default " + XRT_INFO_DEVICE_MECHANICAL + " source " + source.info.location_id});
            this->m_sensor_used.fan_sources.emplace_back(source);
        }
    }
    return RET_SUCCESS;
}

bool XbtestSwConfig::SensorOverridePfmDef()
{
    // sensor request in pfm
    //  if sensor already monitored by default,
    //      do not add it.
    //  else
    //      check if the sensor is actually available, then add to the list of sensor to monitor
    //
    LogMessage(MSG_DEBUG_SETUP, {"Sensor override from " + XBTEST_PFM_DEF});
    for (const auto & pfm_source : this->m_xbtest_pfm_def.sensor)
    {
        auto sensor_exists = false;
        if (StrMatchNoCase(pfm_source.type, XRT_INFO_DEVICE_ELECTRICAL))
        {
            for (auto & source : this->m_sensor_used.power_rail_sources)
            {
                if (StrMatchNoCase(pfm_source.id, source.info.id))
                {
                    if (pfm_source.card_power)
                    {
                        source.card_power = true;
                    }
                    sensor_exists = true;
                    break;
                }
            }
            for (auto & source : this->m_sensor_used.power_consumption_sources)
            {
                if (StrMatchNoCase(pfm_source.id, source.info.id))
                {
                    if (pfm_source.card_power)
                    {
                        source.card_power = true;
                    }
                    sensor_exists = true;
                    break;
                }
            }
        }
        else if (StrMatchNoCase(pfm_source.type, XRT_INFO_DEVICE_THERMAL))
        {
            for (auto & source : this->m_sensor_used.thermal_sources)
            {
                if (StrMatchNoCase(pfm_source.id, source.info.location_id))
                {
                    if (pfm_source.card_temperature)
                    {
                        source.card_temperature = true;
                    }
                    sensor_exists = true;
                    break;
                }
            }
        }
        else if (StrMatchNoCase(pfm_source.type, XRT_INFO_DEVICE_MECHANICAL))
        {
            for (auto & source : this->m_sensor_used.fan_sources)
            {
                if (StrMatchNoCase(pfm_source.id, source.info.location_id))
                {
                    sensor_exists = true;
                    break;
                }
            }
        }
        if (sensor_exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"Definition already used for " + pfm_source.type + " source " + pfm_source.id + " provided in " + XBTEST_PFM_DEF});
        }
        else
        {
            // See if sensor from card definition is valid
            auto sensor_valid = false;
            if (StrMatchNoCase(pfm_source.type, XRT_INFO_DEVICE_ELECTRICAL))
            {
                for (auto & source : this->m_sensor_all.power_rail_sources)
                {
                    if (StrMatchNoCase(source.info.id, pfm_source.id))
                    {
                        if (pfm_source.card_power)
                        {
                            source.card_power = true;
                        }
                        this->m_sensor_used.power_rail_sources.emplace_back(source);
                        sensor_valid = true;
                    }
                }
                for (auto & source : this->m_sensor_all.power_consumption_sources)
                {
                    if (StrMatchNoCase(source.info.id, pfm_source.id))
                    {
                        if (pfm_source.card_power)
                        {
                            source.card_power = true;
                        }
                        this->m_sensor_used.power_consumption_sources.emplace_back(source);
                        sensor_valid = true;
                    }
                }
            }
            else if (StrMatchNoCase(pfm_source.type, XRT_INFO_DEVICE_THERMAL))
            {
                for (auto & source : this->m_sensor_all.thermal_sources)
                {
                    if (StrMatchNoCase(source.info.location_id, pfm_source.id))
                    {
                        if (pfm_source.card_temperature)
                        {
                            source.card_temperature = true;
                        }
                        this->m_sensor_used.thermal_sources.emplace_back(source);
                        sensor_valid = true;
                    }
                }
            }
            else if (StrMatchNoCase(pfm_source.type, XRT_INFO_DEVICE_MECHANICAL))
            {
                for (auto & source : this->m_sensor_all.fan_sources)
                {
                    if (StrMatchNoCase(source.info.location_id, pfm_source.id))
                    {
                        this->m_sensor_used.fan_sources.emplace_back(source);
                        sensor_valid = true;
                    }
                }
            }
            if (sensor_valid)
            {
                LogMessage(MSG_DEBUG_SETUP, {"Using new " + pfm_source.type + " source " + pfm_source.id + " using definition provided in " + XBTEST_PFM_DEF});
            }
            else
            {
                LogMessage(MSG_ITF_133, {pfm_source.type, pfm_source.id, XBTEST_PFM_DEF});
                return RET_FAILURE;
            }
        }
    }
    return RET_SUCCESS;
}

bool XbtestSwConfig::CheckSensorThresholds( const std::string & sensor_name, const std::string & threshold_name, const Sensor_Limit_t & threshold )
{
    if (threshold.min_exists && threshold.max_exists)
    {
        if (threshold.min > threshold.max)
        {
            LogMessage(MSG_ITF_118, {threshold_name, sensor_name, std::to_string(threshold.min), std::to_string(threshold.max)});
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

std::vector<std::string> XbtestSwConfig::GetAllPowerRailSourcesIDs()
{
    std::vector<std::string> ret;
    for (const auto & source : this->m_sensor_all.power_rail_sources)
    {
        if ((source.info.voltage.is_present) || (source.info.current.is_present))
        {
            ret.emplace_back(source.info.id);
        }
    }
    return ret;
}

std::vector<std::string> XbtestSwConfig::GetAllPowerConsumptionSourceIDs()
{
    std::vector<std::string> ret;
    for (const auto & source : this->m_sensor_all.power_consumption_sources)
    {
        if (source.info.is_present)
        {
            ret.emplace_back(source.info.id);
        }
    }
    return ret;
}

void XbtestSwConfig::PrintSensorLimit( const std::string & limit_str, const std::string & unit_str, const Sensor_Limit_t & sensor_limit )
{
    if (sensor_limit.min_exists)
    {
        LogMessage(MSG_ITF_055, {"\t - " + limit_str + " limit min (" + unit_str + "): " + std::to_string(sensor_limit.min)});
    }
    if (sensor_limit.max_exists)
    {
        LogMessage(MSG_ITF_055, {"\t - " + limit_str + " limit max (" + unit_str + "): " + std::to_string(sensor_limit.max)});
    }
}

std::vector<PCIe_Bar_Ep_t> XbtestSwConfig::GetPcieBars()
{
    return this->m_xclbin_metadata.partition_metadata.pcie_bars;
}

bool XbtestSwConfig::GetPcieBar( const uint & pcie_physical_function, const uint & pcie_base_address_register, PCIe_Bar_Ep_t & pcie_bar )
{
    auto found = false;
    for (const auto & bar : this->m_xclbin_metadata.partition_metadata.pcie_bars)
    {
        if ((bar.pcie_physical_function == pcie_physical_function) && (bar.pcie_base_address_register == pcie_base_address_register))
        {
            pcie_bar = bar;
            found = true;
        }
    }
    if (!found)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool XbtestSwConfig::GetPcieEp( const std::string & inst, PCIe_Bar_Ep_t & pcie_ep )
{
    auto found = false;
    for (const auto & ep : this->m_xclbin_metadata.partition_metadata.pcie_eps)
    {
        if (inst.compare(0, inst.size(), ep.name) == 0)
        {
            pcie_ep = ep;
            found = true;
        }
    }
    if (!found)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

} // namespace
