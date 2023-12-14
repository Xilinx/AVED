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

#ifndef _GTMACMGMT_H
#define _GTMACMGMT_H

#include <random>

#include "testinterface.h"
#include "devicemgt.h"
#include "gtmacpackage.h"

namespace xbtest
{

class GTMACMgmt : public TestInterface
{

public:
    GTMACMgmt(
        Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device,
        const Tasks_Type_Parameters_t & task_parameters, const std::map<uint, Gt_Parameters_t> & test_parameters
    );
    ~GTMACMgmt();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;
    bool GetGTMACTestConfig ( const uint & gt_index, GTMACTestcaseCfg_t & gtmac_cfg );
    bool PopCommQueue       ( const uint & gt_index, GTMACMessage_t & value );
    void PushCommQueue      ( const uint & gt_index, const GTMACMessage_t & value );
    void SemaphoreWait      ( const uint & gt_index );
    bool GenConfig();
    std::string MacAddrOctetToStr   ( const uint32_t & addr_slice, const uint32_t & octet_idx );
    bool MacAddrHexToStr            ( const uint64_t & mac_addr_hex, std::string & mac_addr_str );

private:

    // Used to overwrite severity for some messages
    Message_t MSG_ETH_026_ERROR  = MSG_ETH_026; // Error by default in message definition
    Message_t MSG_ETH_026_WARN   = MSG_ETH_026;
    Message_t MSG_ETH_027_ERROR  = MSG_ETH_027; // Error by default in message definition
    Message_t MSG_ETH_027_WARN   = MSG_ETH_027;
    Message_t MSG_ETH_028_ERROR  = MSG_ETH_028; // Error by default in message definition
    Message_t MSG_ETH_028_WARN   = MSG_ETH_028;
    Message_t MSG_ETH_029_ERROR  = MSG_ETH_029; // Error by default in message definition
    Message_t MSG_ETH_029_WARN   = MSG_ETH_029;

    const std::string REP = "%";

    Tasks_Type_Parameters_t         m_task_parameters;
    std::map<uint, Gt_Parameters_t> m_test_parameters;

    bool m_overall_task_failure ;

    std::vector<std::string>                    m_board_mac_addr_list;
    std::map<uint32_t, GTMACTestcaseCfg_t>      m_gtmac_cfg_map;
    std::map<uint32_t, XQueue<GTMACMessage_t>*> m_gtmac_queue_map;
    std::map<uint32_t, Semaphore*>              m_gtmac_semaphore_map;
    std::map<uint32_t, TestItCnt_t>             m_test_it_map;
    std::map<uint32_t, MACStatusCnt_t>          m_mac_status_map;
    std::vector<GT_MAC_Addr_t>                  m_src_mac_addr;
    std::vector<MAC_Addr_t>                     m_mac_addr;
    std::vector<MAC_Addr_t>                     m_source_addr;

    int RunTask();
    void SemaphoreNotify            ( const uint & gt_index );
    bool GetMACAddress              ( const uint & gt_index, GT_MAC_Addr_t & mac_addr );
    void FetchMACAddress();
    bool AllocateSourceMACAddress();
    bool GetParamPacketCfg          ( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, const uint32_t & param_nom, uint32_t & param, std::string & param_cfg, const std::string & param_cfg_default );
    void set_log_header             ( const uint & gt_index );
    bool FetchGTMACTestConfig();
    bool OverwriteCfgPairedGT();
    bool CreateLaneMapping();
    bool OverwriteSourceMACAddress();
    bool CreateDestinationMACAddress();
    bool CheckLanesEnabled();
    bool FetchTestSequence          ( const uint & gt_index, std::vector<GTMAC_Test_Sequence_Parameters_t> & test_sequence );
    bool MacAddrOctetStrToHex       ( const bool & err_n_warn, const std::string & mac_addr_str, const uint & index, uint32_t & octet );
    bool MacAddrStrToHex            ( const bool & err_n_warn, const std::string & mac_addr_str, uint64_t & mac_addr_hex );
    bool CheckMacAddrRange          ( const uint64_t & mac_addr_hex );
    bool GetTestIt                  ( const uint & gt_index, TestItCnt_t & test_it );
    bool GetMACStatus               ( const uint & gt_index, MACStatusCnt_t & MAC_status_cnt );
    bool StartCU() override; // not used
    bool EnableWatchdogClkThrotDetection() override; // not used
    bool StopCU() override; // not used
    bool CheckWatchdogAndClkThrottlingAlarms() override; // not used
    int  RunTest() override; // not used
};

} // namespace

#endif /* _GTMACMGMT_H */
