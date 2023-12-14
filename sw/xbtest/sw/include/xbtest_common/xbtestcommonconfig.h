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

#ifndef _XBTESTCOMMONCONFIG_H
#define _XBTESTCOMMONCONFIG_H

#include "xbtestcommonpackage.h"
#include "logging.h"
#include "configparser.h"
#include "cardjsonparser.h"
#include "commandlineparsercommon.h"

namespace xbtest
{

class XbtestCommonConfig
{

public:

    XbtestCommonConfig( Logging * log, std::atomic<bool> * abort );
    ~XbtestCommonConfig();

    bool                                    ScanHostSetup();
    bool                                    CheckNumLibValid();
    std::map<std::string, Card_Config_t>    GetCardConfigMap();
    Card_Config_t                           GetCardConfig           ( const std::string & bdf );
    void                                    ParseSystemInfo();
    void                                    SetStartSessionTime();
    void                                    SetEndSessionTime();
    std::string                             GetSessionStartTime();
    std::string                             GetSessionEndTime();
    std::string                             GetSystemUsername();
    std::string                             GetSystemSysname();
    std::string                             GetSystemNodename();
    std::string                             GetSystemRelease();
    std::string                             GetSystemVersion();
    std::string                             GetSystemMachine();
    uint                                    ParseCommandLine        ( int argc, char ** argv );
    bool                                    ParseCardJson();
    void                                    FormatUuid              ( std::string & uuid );
    bool                                    GetActualLibConfig      ( const std::string & bdf, std::string & uuid, std::string & dsa_name, HW_Lib_Config_t & lib );
    bool                                    GenerateActualCardConfig();
    void                                    PrintConfig();
    bool                                    GetMultitestMode();
    std::string                             GetCommonLogDir();
    std::string                             GetCommonRTLogDir();
    void                                    SetCommonLogDir         ( const std::string & common_log_dir );
    bool                                    GetUseCommonLogDir();
    void                                    SetUseCommonLogDir      ( const bool & use_common_log_dir );
    bool                                    GetCommandLineConsoleDetailedFormat();
    bool                                    GetCommandLineForce();

private:

    const std::string BUILD_INFO            = "build_info";
    const std::string XBTEST                = "xbtest";
    const std::string MAJOR                 = "major";
    const std::string MINOR                 = "minor";
    const std::string DATE                  = "date";
    const std::string INTERNALRELEASE       = "internal_release";
    const std::string BOARD                 = "board";
    const std::string NAME                  = "name";
    const std::string INTERFACE_UUID        = "interface_uuid";
    const std::string SW_VERSION_MAX        = "sw_version_max";

    const std::string PARTITION_METADATA    = "partition_metadata";

    const std::string DEVICE            = "device";
    const std::string SENSOR            = "sensor";
    const std::string ID                = "id";
    const std::string CARD_POWER        = "card_power";
    const std::string CARD_TEMPERATURE  = "card_temperature";

    Logging *                   m_log = nullptr;
    Xbtest_Common_Config_t      m_xbtest_common_config;
    std::atomic<bool> *         m_abort;
    bool                        m_command_line_parsed;
    uint                        m_num_lib_valid;
    std::string                 m_log_header = LOG_HEADER_GENERAL;
    std::string                 m_amd_xbtest_libs_str;
    std::vector<std::string>    m_amd_xbtest_libs;

    void                        LogMessage                      ( const Message_t & message, const std::vector<std::string> & arg_list );
    void                        LogMessage                      ( const Message_t & message );
    void                        LogMessageBdf                   ( const Message_t & message, const std::string & bdf, const std::vector<std::string> & arg_list );
    void                        LogMessageBdf                   ( const Message_t & message, const std::string & bdf );
    void                        PrintRequiredNotFound           ( const std::string & file_type, const std::vector<std::string> & node_title_in );
    void                        PrintHWConfigRequiredNotFound   ( const std::string & lib_name, const std::string & file_type, const std::vector<std::string> & node_title_in );
    std::vector<std::string>    ListPreCannedTests              ( const std::string & lib_path );
    bool                        GetHWBuildInfo                  ( const std::string & lib_name, const std::string & config_filename, HW_Build_Info_t & hw_build_info );
    bool                        GetHWBuildInfo2                 ( const std::string & lib_name, const std::string & config_filename, HW_Build_Info_t & hw_build_info );
    bool                        GetPlatformDefinition           ( const std::string & lib_name, const std::string & config_filename, HW_Lib_Config_t & hw_lib_config );
    bool                        CheckFileExists                 ( const std::string & source_file, const std::string & filename, const bool & log_error );
    bool                        ScanLibraries();
    bool                        InsertOptions                   ( const std::string & bdf, const uint & test_idx, std::vector<std::string> & arg_dst, const std::vector<std::string> & arg_src );
    std::vector<std::string>    GetOpts                         ( const std::vector<std::string> & args );
    void                        GetNameExt                      ( const std::string & file, std::string & name, std::string & ext );

};

} // namespace

#endif /* _XBTESTCOMMONCONFIG_H */
