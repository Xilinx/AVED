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

#include "gtmacmgmt.h"

namespace xbtest
{

GTMACMgmt::GTMACMgmt(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device,
    const Tasks_Type_Parameters_t & task_parameters, const std::map<uint, Gt_Parameters_t> & test_parameters
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    MSG_ETH_026_WARN.log_level = LOG_WARN;
    MSG_ETH_027_WARN.log_level = LOG_WARN;
    MSG_ETH_028_WARN.log_level = LOG_WARN;
    MSG_ETH_029_WARN.log_level = LOG_WARN;

    this->m_log_header        = LOG_HEADER_GTMACMGMT;
    this->m_task_parameters   = task_parameters;
    this->m_test_parameters   = test_parameters;
}

GTMACMgmt::~GTMACMgmt() = default;

bool GTMACMgmt::PreSetup()
{
    this->m_state       = TestState::TS_PRE_SETUP;
    this->m_log_header  = LOG_HEADER_GTMACMGMT;
    return RET_SUCCESS;
}

void GTMACMgmt::Run()
{
    this->m_state   = TestState::TS_RUNNING;
    this->m_result  = TestResult::TR_FAILED;

    int task_state = 1;

    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});
        this->m_overall_task_failure = RET_SUCCESS;
        task_state = RunTask();
    }

    this->m_log_header  = LOG_HEADER_GTMACMGMT;
    if (task_state < 0)
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (task_state > 0)
    {
        LogMessage(MSG_CMN_010, {GetCurrentTime()});
        this->m_result = TestResult::TR_FAILED;
    }
    else
    {
        LogMessage(MSG_CMN_011, {GetCurrentTime()});
        this->m_result = TestResult::TR_PASSED;
    }
}

void GTMACMgmt::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;

}

void GTMACMgmt::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_007);
        this->m_abort = true;
    }
}

bool GTMACMgmt::GetGTMACTestConfig( const uint & gt_index, GTMACTestcaseCfg_t & gtmac_cfg )
{
    if (this->m_gtmac_cfg_map.count(gt_index) > 0)
    {
        gtmac_cfg = this->m_gtmac_cfg_map[gt_index];
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool GTMACMgmt::PopCommQueue( const uint & gt_index, GTMACMessage_t & value )
{
    return this->m_gtmac_queue_map[gt_index]->Pop(value);
}

void GTMACMgmt::PushCommQueue( const uint & gt_index, const GTMACMessage_t & value )
{
    this->m_gtmac_queue_map[gt_index]->Push(value);
}

void GTMACMgmt::SemaphoreWait( const uint & gt_index )
{
    this->m_gtmac_semaphore_map[gt_index]->Wait();
}

bool GTMACMgmt::GenConfig()
{
    auto global_settings_failure = RET_SUCCESS;

    FetchMACAddress();
    global_settings_failure |= AllocateSourceMACAddress();
    global_settings_failure |= FetchGTMACTestConfig();
    global_settings_failure |= OverwriteCfgPairedGT();
    global_settings_failure |= CreateLaneMapping();
    global_settings_failure |= OverwriteSourceMACAddress();
    global_settings_failure |= CreateDestinationMACAddress();
    global_settings_failure |= CheckLanesEnabled();

    return global_settings_failure;
}

int GTMACMgmt::RunTask()
{
    this->m_log_header  = LOG_HEADER_GTMACMGMT;
    // return -1(Abort), 0(Pass), 1(Fail)
    int ret = 0;

    GTMACMessage_t msg;

    while (!(this->m_overall_task_failure) && !(this->m_abort))
    {
        WaitSecTick(1);

        //check message queue for new test it counter
        for (const auto & cfg : this->m_gtmac_cfg_map)
        {
            auto gt_index = cfg.first;
            if (PopCommQueue(gt_index, msg))
            {
                if (msg.header == GTMACMSG_HEADER_TEST_IT_CNT)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"New test it counter received for GT[" + std::to_string(gt_index) + "]: " + std::to_string (msg.test_it_cnt)});
                    TestItCnt_t test_it;
                    this->m_overall_task_failure |= GetTestIt(gt_index, test_it);
                    test_it.received_cnt = msg.test_it_cnt;
                    this->m_test_it_map[gt_index] = test_it;
                }
                else if (msg.header == GTMACMSG_HEADER_MAC_STATUS)
                {
                    LogMessage(MSG_DEBUG_TESTCASE, {"New MAC status received for GT[" + std::to_string(gt_index) + "]"});
                    MACStatusCnt_t MAC_status;
                    MAC_status.new_status = false;
                    this->m_overall_task_failure |= GetMACStatus(gt_index, MAC_status);
                    MAC_status.new_status = true;
                    std::copy(msg.mac_status, msg.mac_status + GT_MAC_STATUS_SIZE, MAC_status.status);
                    this->m_mac_status_map[gt_index] = MAC_status;
                }
            }
        }
        // check both paired GT are at the same 'new' test_it, if yes release them by notifying their respective semaphore
        for (const auto & cfg : this->m_gtmac_cfg_map)
        {
            auto gt_index   = cfg.first;
            auto test_it_1  = RST_TESTITCNT;
            auto test_it_2  = RST_TESTITCNT;
            this->m_overall_task_failure |= GetTestIt(gt_index, test_it_1);

            // a new test_it has been received, check the paired GT
            if (test_it_1.current_cnt != test_it_1.received_cnt)
            {
                if (test_it_1.gt_pair_en)
                {
                    this->m_overall_task_failure |= GetTestIt(test_it_1.GT_paired, test_it_2);

                    // check for de-synchronisation between GT test sequence
                    if (test_it_1.current_cnt != test_it_2.current_cnt)
                    {
                        LogMessage(MSG_ETH_054, {std::to_string(gt_index),std::to_string(test_it_1.current_cnt),std::to_string(test_it_1.GT_paired),std::to_string(test_it_2.current_cnt) });
                        this->m_overall_task_failure |= RET_FAILURE;
                    }
                    else
                    {
                        // check if the paried GT has also sent a new test_it
                        if (test_it_2.current_cnt != test_it_2.received_cnt)
                        {
                            // both GTs can be now released if they are at the same point in their respective test sequence
                            if (test_it_1.received_cnt != test_it_2.received_cnt)
                            {
                                LogMessage(MSG_ETH_054, {std::to_string(gt_index),std::to_string(test_it_1.received_cnt),std::to_string(test_it_1.GT_paired),std::to_string(test_it_2.received_cnt) });
                                this->m_overall_task_failure |= RET_FAILURE;

                            }
                            else
                            {
                                LogMessage(MSG_DEBUG_TESTCASE, {"Semaphoring paired GT MAC: " + std::to_string(gt_index) + " & " + std::to_string(test_it_1.GT_paired) + " after test it sync"});
                                test_it_1.current_cnt = test_it_1.received_cnt;
                                test_it_2.current_cnt = test_it_2.received_cnt;
                                this->m_test_it_map[gt_index]= test_it_1;
                                this->m_test_it_map[test_it_1.GT_paired] =  test_it_2;
                                SemaphoreNotify(gt_index);
                                SemaphoreNotify(test_it_1.GT_paired);
                            }
                        }
                    }
                }
                else
                {
                    test_it_1.current_cnt = test_it_1.received_cnt;
                    this->m_test_it_map[gt_index]= test_it_1;
                    SemaphoreNotify(gt_index);
                }
            }
        }
        // check both paired GT if they both have a new MAC_status
        // if yes cross send status and then notify GT MAC that paired GT MAC status is available
        for (const auto & cfg : this->m_gtmac_cfg_map)
        {
            auto gt_index = cfg.first;
            MACStatusCnt_t MAC_status_1;
            MACStatusCnt_t MAC_status_2;

            // init
            MAC_status_1.new_status = false;
            MAC_status_2.new_status = false;

            this->m_overall_task_failure |= GetMACStatus(gt_index, MAC_status_1);
            if (MAC_status_1.new_status)
            {
                this->m_overall_task_failure |= GetMACStatus(MAC_status_1.GT_paired, MAC_status_2);
                if (MAC_status_2.new_status)
                {
                    // cross send status
                    GTMACMessage_t gtmac_msg;
                    gtmac_msg.header = GTMACMSG_HEADER_MAC_STATUS;
                    std::copy(MAC_status_2.status, MAC_status_2.status + GT_MAC_STATUS_SIZE, gtmac_msg.mac_status);
                    PushCommQueue(gt_index, gtmac_msg);
                    std::copy(MAC_status_1.status, MAC_status_1.status + GT_MAC_STATUS_SIZE, gtmac_msg.mac_status);
                    PushCommQueue(MAC_status_1.GT_paired, gtmac_msg);

                    // mark both status as old & semaphore both GT MAC xbtest HW IP that a message is ready to get pop
                    MAC_status_1.new_status = false;
                    MAC_status_2.new_status = false;
                    this->m_mac_status_map[gt_index]= MAC_status_1;
                    this->m_mac_status_map[MAC_status_1.GT_paired] =  MAC_status_2;
                    LogMessage(MSG_DEBUG_TESTCASE, {"Semaphoring paired GT MAC: " + std::to_string(gt_index) + " & " + std::to_string(MAC_status_1.GT_paired) + " after cross-sent mac status"});
                    SemaphoreNotify(gt_index);
                    SemaphoreNotify(MAC_status_1.GT_paired);
                }
            }
        }

    }
    // As this test is run as background task, the abort signal is not considered as a failure
    // An internal task failure is considered as an abort, so that all other tests will be aborted as a consequence
    if (!(this->m_overall_task_failure) && this->m_abort)
    {
        ret = 0;
    }
    else if (this->m_overall_task_failure)
    {
        ret = -1;
    }
    else
    {
        ret = 1;
    }
    return ret;
}

void GTMACMgmt::SemaphoreNotify( const uint & gt_index )
{
    LogMessage(MSG_DEBUG_TESTCASE, {"Semaphore: Notify GT[" + std::to_string(gt_index) + "]"});
    this->m_gtmac_semaphore_map[gt_index]->Notify();
}

bool GTMACMgmt::GetMACAddress( const uint & gt_index, GT_MAC_Addr_t & mac_addr )
{
    if (gt_index < this->m_src_mac_addr.size())
    {
        mac_addr = this->m_src_mac_addr[gt_index];
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

void GTMACMgmt::FetchMACAddress()
{
    this->m_log_header  = LOG_HEADER_GTMACMGMT;
    // grab all MAC Address and check for errors

    auto macs_info = this->m_xbtest_sw_config->GetPlatformMacsInfo();

    uint64_t    mac_addr_hex;
    std::string temp_str;
    uint32_t    valid_mac_addr = 0;
    uint32_t    addr_idx = 0;

    // for test
    // Platforthis->m_Mac_Info_t info1, info2, info3, info4, info5, info6, info7, info8;
    // info1.address = "00:0a:35:00:00:01";
    // info2.address = "00:0a:35:00:00:02";
    // info3.address = "00:00:35:00:00:03"; // out of range
    // info4.address = "00:0a:35:00:00:04";
    // info5.address = "00:5d:03:00:00:05";
    // info6.address = "00:5d:03:00:00:06";
    // info7.address = "00:5d:03:00:00:07";
    // info8.address = "00:5d:03:00:00:08";
    // macs_info.clear();
    // macs_info = {info1, info2, info3, info4, info5, info6, info7, info8}

    // based on what has been extracted from the card EEPROM, create a list of address with the indication if it's a valid one or not
    // first check how many addresses are available and if they are valid
    if (!macs_info.empty())
    {

        LogMessage(MSG_DEBUG_TESTCASE, {"MAC addresses extracted from the board:"});
        for (const auto & info : macs_info)
        {
            // check if the address can be converted into 6 octets
            auto mac_addr = RST_MAC_ADDR;
            mac_addr.addr = DEFAULT_TEST_MAC_ADDR;
            if (MacAddrStrToHex(false, info.address, mac_addr_hex) == RET_SUCCESS)
            {
                mac_addr.valid = true;
                mac_addr.addr = mac_addr_hex;
                this->m_mac_addr.emplace_back(mac_addr);
            }
            LogMessage(MSG_DEBUG_TESTCASE, {"\t - " + std::to_string(addr_idx) + ": " + info.address});

            // convert it back to string for test
            // MacAddrHexToStr(mac_addr_hex, temp_str);
            // LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * " + temp_str});
            addr_idx++;
        }

        // check that 2 addresses are not identical and mark the duplicate as invalid
        if (this->m_mac_addr.size() >= 2)
        {
            for (uint idx = 0; idx < this->m_mac_addr.size() - 1; idx++) // no need to check the last element of the list.
            {
                addr_idx = idx + 1;
                while (addr_idx < this->m_mac_addr.size())
                {
                    if (this->m_mac_addr[idx].addr == this->m_mac_addr[addr_idx].addr)
                    {
                        this->m_mac_addr[addr_idx].valid = false;
                        LogMessage(MSG_ETH_032, {std::to_string(idx), NumToStrHex<uint64_t>(this->m_mac_addr[idx].addr),std::to_string(addr_idx), NumToStrHex<uint64_t>(this->m_mac_addr[addr_idx].addr)});
                    }
                    addr_idx++;
                }
            }
        }

        // check if the addresses are in supported addresses range, if not mark then as invalid
        //  do NOT check if they are within Xilinx pool ones
        //  just for info, it doesn't stop anything
        valid_mac_addr = 0;
        for (auto & mac_addr : this->m_mac_addr)
        {
            if (mac_addr.valid)
            {
                if (CheckMacAddrRange(mac_addr.addr) == RET_SUCCESS)
                {
                    valid_mac_addr++;
                }
                else
                {
                    mac_addr.valid = false;
                }
            }
        }
    }
    else
    {
        valid_mac_addr = 0;
        LogMessage(MSG_ETH_038); // no HW MAC address detected
        // having no addresses defined is not an error as user can overwrite them in his test.json
    }

    auto min_mac_addr = GT_LANE_INDEXES.size() * this->m_xbtest_sw_config->GetNumGT();
    if (valid_mac_addr < min_mac_addr)
    {
        // there is NOT enough address listed in the device info for all GT lanes
        LogMessage(MSG_ETH_033, {std::to_string(valid_mac_addr), std::to_string(min_mac_addr)});
    }

    // remove all none valid address (use Erase-remove idiom over the vector of struct)
    this->m_mac_addr.erase(std::remove_if(this->m_mac_addr.begin(), this->m_mac_addr.end(), [](const MAC_Addr_t& t){return !t.valid;}), this->m_mac_addr.end());

    // create expected "board_mac_addr_'i'" string list: board_mac_addr_0, board_mac_addr_1, board_mac_addr_2 ...
    // length of the list is based on the quantity of lane available and board_mac_addr
    for (uint idx = 0; idx < this->m_mac_addr.size(); idx++)
    {
        this->m_board_mac_addr_list.emplace_back("board_mac_addr_" + std::to_string(idx));
    }

    if (valid_mac_addr > 0)
    {
        LogMessage(MSG_ETH_030); // list MAC address
        for (uint idx = 0; idx < this->m_mac_addr.size(); idx++)
        {
            MacAddrHexToStr(this->m_mac_addr[idx].addr, temp_str);
            LogMessage(MSG_ETH_036, {std::to_string(idx), temp_str});
        }
    }

}

bool GTMACMgmt::AllocateSourceMACAddress()
{
    // always allocate the address based on the quantity of GT defined in the card definition and not based on the quantity of GT under test
    // so GT will always get the same addresses what ever the test
    // round robin the valid address per GT and per lane
    //      e.g. 5 address for 2 GTs: A, B, C, D, E
    //      GT[0] will receive: address: A, C, E and remainig address will be not valid
    //      GT[1] will receive: address: B, D and remainig address will be not valid
    this->m_log_header = LOG_HEADER_GTMACMGMT;
    auto ret = RET_SUCCESS;
    std::string temp_str;
    auto tmp_mac_addr = RST_MAC_ADDR;

    for (uint gt_index = 0; gt_index < this->m_xbtest_sw_config->GetNumGTPfm(); gt_index++)
    {
        GT_MAC_Addr_t mac_addr;
        tmp_mac_addr.valid = false;
        tmp_mac_addr.addr  = NO_VALID_MAC_ADDRESS;

        // init the vector of addresses
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            mac_addr[lane_idx] = tmp_mac_addr;
        }

        if (!(this->m_mac_addr.empty()))
        {
            uint addr_idx = gt_index;
            uint lane_idx = 0;
            while ((addr_idx < this->m_mac_addr.size()) && (lane_idx < GT_LANE_INDEXES.size()))
            {
                if (this->m_mac_addr[addr_idx].valid)
                {
                    mac_addr[lane_idx]  = this->m_mac_addr[addr_idx];
                }
                addr_idx += this->m_xbtest_sw_config->GetNumGTPfm();
                lane_idx++;
            }
        }
        this->m_src_mac_addr.emplace_back(mac_addr);
    }

    LogMessage(MSG_DEBUG_TESTCASE, {"Default GT Mac Address allocation" + temp_str});
    for (uint gt_idx = 0; gt_idx < this->m_xbtest_sw_config->GetNumGTPfm(); gt_idx++)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - GT[" + std::to_string(gt_idx) + "]"});
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            MacAddrHexToStr(this->m_src_mac_addr[gt_idx][lane_idx].addr, temp_str);
            LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * Lane " +std::to_string(lane_idx) + ": " + temp_str});
        }
    }
    return ret;
}

bool GTMACMgmt::GetParamPacketCfg ( const Json_Val_Def_t & json_val_def, const std::string & name_cmplt, Json_Parameters_t & json_param, const uint32_t & param_nom, uint32_t & param, std::string & param_cfg, const std::string & param_cfg_default )
{
    param      = param_nom; // Default value
    param_cfg  = param_cfg_default; // Default value

    auto it = FindJsonParam(json_param, json_val_def);
    if (it != json_param.end())
    {
        // check first if it's an integer
        // if yes: check the range
        // if not check that it's the expect string sweep
        auto param_cfg_str = TestcaseParamCast<std::string>(it->second);
        if (ConvString2Num<uint32_t>(param_cfg_str, param) == RET_SUCCESS)
        {
            ConvertStringToNum<uint32_t>(json_val_def.name + name_cmplt, param_cfg_str, param);
            param_cfg = PACKET_CFG_NO_SWEEP;
            if ( (param < SMALL_PACKET_SIZE_MIN) || ( (param > SMALL_PACKET_SIZE_MAX) && (param < BIG_PACKET_SIZE_MIN) ) || (param > BIG_PACKET_SIZE_MAX) )
            {
                LogMessage(MSG_ETH_039, {std::to_string(param), json_val_def.name + name_cmplt, PACKET_CFG_SWEEP, std::to_string(SMALL_PACKET_SIZE_MIN), std::to_string(SMALL_PACKET_SIZE_MAX), std::to_string(BIG_PACKET_SIZE_MIN),std::to_string(BIG_PACKET_SIZE_MAX) });
                return RET_FAILURE;
            }
            // jumbo frame must be even size
            if ( (param >= BIG_PACKET_SIZE_MIN) && (param <= BIG_PACKET_SIZE_MAX) && ((param % 2) != 0) )
            {
                LogMessage(MSG_ETH_047, {std::to_string(param)});
                return RET_FAILURE;
            }

        }
        else
        {
            if (StrMatchNoCase(param_cfg_str, PACKET_CFG_SWEEP))
            {
                param_cfg = PACKET_CFG_SWEEP;
            }
            else
            {
                param_cfg = PACKET_CFG_SWEEP;
                LogMessage(MSG_ETH_039, {param_cfg_str, json_val_def.name + name_cmplt, PACKET_CFG_SWEEP, std::to_string(SMALL_PACKET_SIZE_MIN), std::to_string(SMALL_PACKET_SIZE_MAX), std::to_string(BIG_PACKET_SIZE_MIN),std::to_string(BIG_PACKET_SIZE_MAX) });
                return RET_FAILURE;
            }
        }
    }
    else
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_CMN_012, {json_val_def.name + name_cmplt, std::to_string(param)});
        }
    }
    return RET_SUCCESS;
}

void GTMACMgmt::set_log_header( const uint & gt_index )
{
    this->m_log_header = LOG_HEADER_GTMAC;
    this->m_log_header.replace(LOG_HEADER_GTMAC.find(REP), REP.length(), std::to_string(gt_index));
}

bool GTMACMgmt::FetchGTMACTestConfig()
{
    auto ret = RET_SUCCESS;

    for (uint cu_idx = 0; cu_idx < this->m_device->GetNumGtmacCu(); cu_idx++)
    {
        // Note cu_idx is the index of the xbtest HW IP in the array and is different from gt_index
        auto gt_index = this->m_device->GetGtmacCuGTIndex(cu_idx);
        // change log header so gt mac header is match the test json
        set_log_header(gt_index);

        GTMACTestcaseCfg_t TC_Cfg;
        TC_Cfg.exist = false;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            TC_Cfg.lane_param[lane_idx] = RST_LANEPARAM;
        }

        if (this->m_test_parameters[gt_index].exists)
        {
            TC_Cfg.exist = true;

            // check if the GT_MAC is connected to another GT_MAC xbtest HW IP
            ret |= GetJsonParamNum<uint32_t>(MAC_TO_MAC_CONNECTION_MEMBER, this->m_test_parameters[gt_index].global_config, 0, gt_index, this->m_xbtest_sw_config->GetNumGT(), TC_Cfg.global_param.mac_to_mac_connection);

            // only check test parameters when not in port to port mode
            // in port to port mode, parameters will come from another GT mac
            //
            if (TC_Cfg.global_param.mac_to_mac_connection == gt_index)
            {
                // so for it's not connected to another GT port, so it's a source of traffic to itself
                TC_Cfg.global_param.gt_mapping.enable       = false;
                TC_Cfg.global_param.gt_mapping.cfg_source   = SELF_TRAFFIC_SOURCE;
                TC_Cfg.global_param.gt_mapping.paired_gt    = gt_index; // not paired, traffic to itself

                auto test_it = RST_TESTITCNT;

                // only insert if not present as the paired GT could have created it before
                if (this->m_test_it_map.find(gt_index) == this->m_test_it_map.end())
                {
                    this->m_test_it_map.emplace(gt_index, test_it);
                }
                MACStatusCnt_t MAC_status;
                MAC_status.new_status = false;
                if (this->m_mac_status_map.find(gt_index) == this->m_mac_status_map.end())
                {
                    this->m_mac_status_map.emplace(gt_index, MAC_status);
                }

                ret |= FetchTestSequence(gt_index, TC_Cfg.test_sequence);

                // Get lane configurations from JSON. For each parameter:
                //      1) Get TC_Cfg.global_param, the configuration overwrite for all lines.
                //          Note TC_Cfg.global_param is then set as default/nominal configuration to the lane configuration overwrite.
                //      2) Get TC_Cfg.lane_param[lane_idx], lane_idx in GT_LANE_INDEXES, the configuration overwrite for lane "lane_idx"

                // configuration active mac / disable lane
                TC_Cfg.global_param.disable_lane = false; // this is not a global parameter
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= GetJsonParamBool(DISABLE_LANE_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, TC_Cfg.lane_param[lane_idx].disable_lane, false);
                }
                // configuration utilization
                ret |= GetJsonParamNum<uint32_t>(UTILISATION_MEMBER, this->m_test_parameters[gt_index].global_config, MIN_UTILISATION, NOM_UTILISATION, MAX_UTILISATION, TC_Cfg.global_param.utilisation);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= GetJsonParamNum<uint32_t>(UTILISATION_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, MIN_UTILISATION, TC_Cfg.global_param.utilisation, MAX_UTILISATION, TC_Cfg.lane_param[lane_idx].utilisation);
                }
                // configuration traffic_type
                ret |= GetJsonParamStr(TRAFFIC_TYPE_MEMBER, this->m_test_parameters[gt_index].global_config, SUPPORTED_TRAFFIC_TYPE, TC_Cfg.global_param.traffic_type,  TRAFFIC_TYPE_COUNT);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= GetJsonParamStr(TRAFFIC_TYPE_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, SUPPORTED_TRAFFIC_TYPE, TC_Cfg.lane_param[lane_idx].traffic_type, TC_Cfg.global_param.traffic_type);
                }
                // configuration packet_cfg
                ret |= GetParamPacketCfg(PACKET_CFG_MEMBER, "", this->m_test_parameters[gt_index].global_config, NOM_PACKET_SIZE, TC_Cfg.global_param.packet_size, TC_Cfg.global_param.packet_cfg, PACKET_CFG_SWEEP);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= GetParamPacketCfg(PACKET_CFG_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, TC_Cfg.global_param.packet_size, TC_Cfg.lane_param[lane_idx].packet_size, TC_Cfg.lane_param[lane_idx].packet_cfg, TC_Cfg.global_param.packet_cfg);
                }
                // configuration match_tx_rx
                ret |= GetJsonParamBool(MATCH_TX_RX_MEMBER, this->m_test_parameters[gt_index].global_config, TC_Cfg.global_param.match_tx_rx,  false);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= GetJsonParamBool(MATCH_TX_RX_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, TC_Cfg.lane_param[lane_idx].match_tx_rx, TC_Cfg.global_param.match_tx_rx);
                }

                // The following parameter has no global_config, only lane_config
                // configuration tx_mapping = to which lane the traffic is sent
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= GetJsonParamNum<uint32_t>(TX_MAPPING_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, 0, lane_idx, GT_LANE_INDEXES.size() - 1, TC_Cfg.lane_param[lane_idx].tx_mapping);
                }

                // create rx_mapping: from which lane the traffic is received
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    for (const auto & n : GT_LANE_INDEXES)
                    {
                        if (TC_Cfg.lane_param[n].tx_mapping == lane_idx)
                        {
                            TC_Cfg.lane_param[lane_idx].rx_mapping = n;
                        }
                    }
                }
            }
            else
            {
                TC_Cfg.global_param.gt_mapping.enable       = true;
                TC_Cfg.global_param.gt_mapping.cfg_source   = TC_Cfg.global_param.mac_to_mac_connection;
                TC_Cfg.global_param.gt_mapping.paired_gt    = TC_Cfg.global_param.mac_to_mac_connection;

                // create a cross map of the paired GT and their current test_it
                TestItCnt_t test_it;
                test_it.gt_pair_en      = true;
                test_it.GT_paired       = TC_Cfg.global_param.mac_to_mac_connection;
                test_it.current_cnt     = 0;
                test_it.received_cnt    = 0;
                this->m_test_it_map.emplace(gt_index, test_it);

                // paired GT: add to map or replace if already existing
                test_it.GT_paired = gt_index;
                if (this->m_test_it_map.find(TC_Cfg.global_param.mac_to_mac_connection) == this->m_test_it_map.end())
                {
                    this->m_test_it_map.emplace(TC_Cfg.global_param.mac_to_mac_connection, test_it);
                }
                else
                {
                    this->m_test_it_map[TC_Cfg.global_param.mac_to_mac_connection] = test_it;
                }

                MACStatusCnt_t MAC_status;
                MAC_status.new_status   = false;
                MAC_status.GT_paired    = TC_Cfg.global_param.mac_to_mac_connection;
                this->m_mac_status_map.emplace(gt_index, MAC_status);

                // paired GT: add to map or replace if already existing
                MAC_status.GT_paired = gt_index;
                if (this->m_mac_status_map.find(TC_Cfg.global_param.mac_to_mac_connection) == this->m_mac_status_map.end())
                {
                    this->m_mac_status_map.emplace(TC_Cfg.global_param.mac_to_mac_connection, MAC_status);
                }
                else
                {
                    this->m_mac_status_map[TC_Cfg.global_param.mac_to_mac_connection] = MAC_status;
                }

                // check that nothing related to traffic config is present
                ret |= CheckJsonParamNotDefined(OPTIONAL_TEST_SEQUENCE_MEMBER, this->m_test_parameters[gt_index].global_config, "GT port to port connection enabled");
                ret |= CheckJsonParamNotDefined(           UTILISATION_MEMBER, this->m_test_parameters[gt_index].global_config, "GT port to port connection enabled");
                ret |= CheckJsonParamNotDefined(          TRAFFIC_TYPE_MEMBER, this->m_test_parameters[gt_index].global_config, "GT port to port connection enabled");
                ret |= CheckJsonParamNotDefined(            PACKET_CFG_MEMBER, this->m_test_parameters[gt_index].global_config, "GT port to port connection enabled");
                ret |= CheckJsonParamNotDefined(           MATCH_TX_RX_MEMBER, this->m_test_parameters[gt_index].global_config, "GT port to port connection enabled");

                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    ret |= CheckJsonParamNotDefined(DISABLE_LANE_MEMBER, this->m_test_parameters[gt_index].lane_config[lane_idx].param, "GT port to port connection enabled for lane " + std::to_string(lane_idx));
                    ret |= CheckJsonParamNotDefined( UTILISATION_MEMBER, this->m_test_parameters[gt_index].lane_config[lane_idx].param, "GT port to port connection enabled for lane " + std::to_string(lane_idx));
                    ret |= CheckJsonParamNotDefined(TRAFFIC_TYPE_MEMBER, this->m_test_parameters[gt_index].lane_config[lane_idx].param, "GT port to port connection enabled for lane " + std::to_string(lane_idx));
                    ret |= CheckJsonParamNotDefined(  PACKET_CFG_MEMBER, this->m_test_parameters[gt_index].lane_config[lane_idx].param, "GT port to port connection enabled for lane " + std::to_string(lane_idx));
                    ret |= CheckJsonParamNotDefined( MATCH_TX_RX_MEMBER, this->m_test_parameters[gt_index].lane_config[lane_idx].param, "GT port to port connection enabled for lane " + std::to_string(lane_idx));
                    ret |= CheckJsonParamNotDefined(  TX_MAPPING_MEMBER, this->m_test_parameters[gt_index].lane_config[lane_idx].param, "GT port to port connection enabled for lane " + std::to_string(lane_idx));
                }
            }

            // source mac address overwrite
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamAnyStr(SOURCE_ADDR_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, TC_Cfg.lane_param[lane_idx].src_mac_addr, NO_MAC_ADDR_OVERWRITE);
            }

            // overwrite destination address
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamAnyStr(DEST_ADDR_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, TC_Cfg.lane_param[lane_idx].dest_mac_addr, NO_MAC_ADDR_OVERWRITE);
            }

            // default configuration of GT; all lanes configured identically
            ret |= GetJsonParamStr(GT_SETTINGS_MEMBER, this->m_test_parameters[gt_index].global_config, SUPPORTED_GT_SETTINGS, TC_Cfg.global_param.gt_settings, GT_SETTINGS_MODULE);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                TC_Cfg.lane_param[lane_idx].gt_settings = TC_Cfg.global_param.gt_settings;
            }

            std::string tx_polarity;
            uint32_t    tx_main_cursor;
            uint32_t    tx_diffctrl;
            uint32_t    tx_pre_emphasis;
            uint32_t    tx_post_emphasis;
            bool        rx_equaliser_lpm;

            auto gt_settings = this->m_xbtest_sw_config->GetGTSettings(gt_index);

            if (TC_Cfg.global_param.gt_settings == GT_SETTINGS_CABLE)
            {
                tx_polarity      = gt_settings.transceivers.cable.tx_polarity;
                tx_main_cursor   = gt_settings.transceivers.cable.tx_main_cursor;
                tx_diffctrl      = gt_settings.transceivers.cable.tx_differential_swing_control;
                tx_pre_emphasis  = gt_settings.transceivers.cable.tx_pre_emphasis;
                tx_post_emphasis = gt_settings.transceivers.cable.tx_post_emphasis;
                rx_equaliser_lpm = RX_EQUALISER_DFM_EN;

                if (StrMatchNoCase(gt_settings.transceivers.cable.rx_equalizer, RX_EQUALISER_LPM))
                {
                    rx_equaliser_lpm = RX_EQUALISER_LPM_EN;
                }
            }
            else
            {
                tx_polarity      = gt_settings.transceivers.module.tx_polarity;
                tx_main_cursor   = gt_settings.transceivers.module.tx_main_cursor;
                tx_diffctrl      = gt_settings.transceivers.module.tx_differential_swing_control;
                tx_pre_emphasis  = gt_settings.transceivers.module.tx_pre_emphasis;
                tx_post_emphasis = gt_settings.transceivers.module.tx_post_emphasis;
                rx_equaliser_lpm = RX_EQUALISER_DFM_EN;

                if (StrMatchNoCase(gt_settings.transceivers.module.rx_equalizer, RX_EQUALISER_LPM))
                {
                    rx_equaliser_lpm = RX_EQUALISER_LPM_EN;
                }
            }

            // overwrite each GT settings globally then allow a per lane overwrite
            // overwrite gt_tx_diffctrl
            ret |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, this->m_test_parameters[gt_index].global_config, MIN_GT_TX_DIFFCTRL, tx_diffctrl, MAX_GT_TX_DIFFCTRL, TC_Cfg.global_param.gt_tx_diffctrl);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamNum<uint32_t>(GT_TX_DIFFCTRL_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, MIN_GT_TX_DIFFCTRL, TC_Cfg.global_param.gt_tx_diffctrl, MAX_GT_TX_DIFFCTRL, TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl);
            }
            // overwrite gt_tx_main_cursor
            ret |= GetJsonParamNum<uint32_t>(GT_TX_MAIN_CURSOR_MEMBER, this->m_test_parameters[gt_index].global_config, MIN_GT_TX_MAIN_CURSOR, tx_main_cursor, MAX_GT_TX_MAIN_CURSOR, TC_Cfg.global_param.gt_tx_main_cursor);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamNum<uint32_t>(GT_TX_MAIN_CURSOR_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, MIN_GT_TX_MAIN_CURSOR, TC_Cfg.global_param.gt_tx_main_cursor, MAX_GT_TX_MAIN_CURSOR, TC_Cfg.lane_param[lane_idx].gt_tx_main_cursor);
            }
            // overwrite gt_tx_pre_emph
            ret |= GetJsonParamNum<uint32_t>(GT_TX_PRE_EMPH_MEMBER, this->m_test_parameters[gt_index].global_config, MIN_GT_TX_PRE_EMPH, tx_pre_emphasis, MAX_GT_TX_PRE_EMPH, TC_Cfg.global_param.gt_tx_pre_emph);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamNum<uint32_t>(GT_TX_PRE_EMPH_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, MIN_GT_TX_PRE_EMPH, TC_Cfg.global_param.gt_tx_pre_emph, MAX_GT_TX_PRE_EMPH, TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph);
            }
            // overwrite gt_tx_post_emph
            ret |= GetJsonParamNum<uint32_t>(GT_TX_POST_EMPH_MEMBER, this->m_test_parameters[gt_index].global_config, MIN_GT_TX_POST_EMPH, tx_post_emphasis, MAX_GT_TX_POST_EMPH, TC_Cfg.global_param.gt_tx_post_emph);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamNum<uint32_t>(GT_TX_POST_EMPH_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, MIN_GT_TX_POST_EMPH, TC_Cfg.global_param.gt_tx_post_emph, MAX_GT_TX_POST_EMPH, TC_Cfg.lane_param[lane_idx].gt_tx_post_emph);
            }
            // overwrite gt_rx_use_lpm
            ret |= GetJsonParamBool(GT_RX_USE_LPM_MEMBER, this->m_test_parameters[gt_index].global_config, TC_Cfg.global_param.gt_rx_use_lpm,  rx_equaliser_lpm);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamBool(GT_RX_USE_LPM_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm, TC_Cfg.global_param.gt_rx_use_lpm);
            }
            // overwrite tx_polarity
            ret |= GetJsonParamStr(GT_TX_POLARITY_MEMBER, this->m_test_parameters[gt_index].global_config, SUPPORTED_TX_POLARITY_TYPE, TC_Cfg.global_param.tx_polarity,  tx_polarity);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                ret |= GetJsonParamStr(GT_TX_POLARITY_MEMBER, " for lane " + std::to_string(lane_idx), this->m_test_parameters[gt_index].lane_config[lane_idx].param, SUPPORTED_TX_POLARITY_TYPE, TC_Cfg.lane_param[lane_idx].tx_polarity, TC_Cfg.global_param.tx_polarity);
            }

            this->m_gtmac_cfg_map.emplace(gt_index, TC_Cfg);
            auto * comm_queue = new XQueue<GTMACMessage_t>;
            this->m_gtmac_queue_map.emplace(gt_index, comm_queue);

            auto * gtmac_semaphore = new Semaphore();
            this->m_gtmac_semaphore_map.emplace(gt_index, gtmac_semaphore);
        }
    }
    this->m_log_header = LOG_HEADER_GTMACMGMT;
    return ret;
}

bool GTMACMgmt::OverwriteCfgPairedGT()
{
    auto ret = RET_SUCCESS;
    for (const auto & cfg : this->m_gtmac_cfg_map)
    {
        auto gt_index  = cfg.first;
        auto gtmac_cfg = cfg.second;
        set_log_header(gt_index);

        // if port to port is enabled, select the test sequence from the GT source
        if ( (gtmac_cfg.global_param.gt_mapping.enable) && (gtmac_cfg.global_param.gt_mapping.cfg_source != SELF_TRAFFIC_SOURCE) )
        {
            LogMessage(MSG_ETH_052, {std::to_string(gtmac_cfg.global_param.gt_mapping.cfg_source)});
            this->m_gtmac_cfg_map[gt_index].test_sequence = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].test_sequence;

            this->m_gtmac_cfg_map[gt_index].global_param.utilisation   = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].global_param.utilisation;
            this->m_gtmac_cfg_map[gt_index].global_param.match_tx_rx   = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].global_param.match_tx_rx;
            this->m_gtmac_cfg_map[gt_index].global_param.packet_size   = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].global_param.packet_size;
            this->m_gtmac_cfg_map[gt_index].global_param.packet_cfg    = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].global_param.packet_cfg;
            this->m_gtmac_cfg_map[gt_index].global_param.traffic_type  = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].global_param.traffic_type;

            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                this->m_gtmac_cfg_map[gt_index].lane_param[lane_idx].disable_lane  = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].lane_param[lane_idx].disable_lane;
                this->m_gtmac_cfg_map[gt_index].lane_param[lane_idx].utilisation   = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].lane_param[lane_idx].utilisation;
                this->m_gtmac_cfg_map[gt_index].lane_param[lane_idx].match_tx_rx   = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].lane_param[lane_idx].match_tx_rx;
                this->m_gtmac_cfg_map[gt_index].lane_param[lane_idx].packet_size   = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].lane_param[lane_idx].packet_size;
                this->m_gtmac_cfg_map[gt_index].lane_param[lane_idx].packet_cfg    = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].lane_param[lane_idx].packet_cfg;
                this->m_gtmac_cfg_map[gt_index].lane_param[lane_idx].traffic_type  = this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.cfg_source].lane_param[lane_idx].traffic_type;
            }
        }
    }
    this->m_log_header = LOG_HEADER_GTMACMGMT;
    return ret;
}

bool GTMACMgmt::CreateLaneMapping()
{
    // check which port is connected to which and overwrite source port tx mapping with destination one
    auto ret = RET_SUCCESS;
    this->m_log_header = LOG_HEADER_GTMACMGMT;

    // cross-map port to port connection, take tx_mapping from the source
    for (const auto & cfg : this->m_gtmac_cfg_map)
    {
        auto gt_index  = cfg.first;
        auto gtmac_cfg = cfg.second;

        // if port to port is enabled and the GT is paired (not sending traffic to itself)
        if ( (gtmac_cfg.global_param.gt_mapping.enable) && (gtmac_cfg.global_param.gt_mapping.cfg_source != SELF_TRAFFIC_SOURCE) )
        {
            GTMACTestcaseCfg_t src_gtmac_cfg;
            ret |= GetGTMACTestConfig(gtmac_cfg.global_param.gt_mapping.cfg_source, src_gtmac_cfg);

            // copy tx mapping
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                gtmac_cfg.lane_param[lane_idx].tx_mapping = src_gtmac_cfg.lane_param[lane_idx].tx_mapping;
            }

            // create rx_mapping: from which lane the traffic is received
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                for (const auto & n : GT_LANE_INDEXES)
                {
                    if (gtmac_cfg.lane_param[n].tx_mapping == lane_idx)
                    {
                        gtmac_cfg.lane_param[lane_idx].rx_mapping = n;
                    }
                }
            }
            // save the updates
            this->m_gtmac_cfg_map[gt_index] = gtmac_cfg;

            // modify the source settings to let it aware that it's connected to another port
            src_gtmac_cfg.global_param.gt_mapping.enable = true;
            src_gtmac_cfg.global_param.gt_mapping.cfg_source = SELF_TRAFFIC_SOURCE;
            src_gtmac_cfg.global_param.gt_mapping.paired_gt = gt_index;

            this->m_gtmac_cfg_map[gtmac_cfg.global_param.gt_mapping.paired_gt] = src_gtmac_cfg;
        }
    }

    LogMessage(MSG_DEBUG_TESTCASE, {"GT lane mapping"});
    for (const auto & cfg : this->m_gtmac_cfg_map)
    {
        // as this->m_gtmac_cfg_map is a map
        auto gt_index  = cfg.first;
        auto gtmac_cfg = cfg.second;
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - GT[" + std::to_string(gt_index) + "]"});
        for (const auto & n : GT_LANE_INDEXES)
        {
            LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * Lane " +std::to_string(n) + " to lane " + std::to_string(gtmac_cfg.lane_param[n].tx_mapping)});
        }
    }
    return ret;
}

bool GTMACMgmt::OverwriteSourceMACAddress()
{
    auto ret = RET_SUCCESS;

    GT_MAC_Addr_t      mac_addr;
    GTMACTestcaseCfg_t gtmac_cfg;

    for (const auto & cfg : this->m_gtmac_cfg_map)
    {
        auto gt_index  = cfg.first;
        auto gtmac_cfg = cfg.second;
        set_log_header(gt_index);
        LogMessage(MSG_ETH_034);

        ret |= GetMACAddress(gt_index, mac_addr);

        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if (gtmac_cfg.lane_param[lane_idx].src_mac_addr != NO_MAC_ADDR_OVERWRITE)
            {
                if (StrMatchNoCase(gtmac_cfg.lane_param[lane_idx].src_mac_addr, TEST_MAC_ADDR))
                {
                    mac_addr[lane_idx].addr  = DEFAULT_TEST_MAC_ADDR | uint64_t((gt_index<<4) + lane_idx);
                    mac_addr[lane_idx].valid = true;
                }
                else
                {
                    if (FindStringInVect(gtmac_cfg.lane_param[lane_idx].src_mac_addr, this->m_board_mac_addr_list))
                    {
                        auto addr_split = split(gtmac_cfg.lane_param[lane_idx].src_mac_addr, '_');
                        // take the last element of the vector and convert it integer: "board_mac_addr_'i'"
                        if (uint(stoi(addr_split.back())) < this->m_mac_addr.size())
                        {
                            mac_addr[lane_idx].addr  = this->m_mac_addr[stoi(addr_split.back())].addr;
                            mac_addr[lane_idx].valid = true;
                        }
                        else
                        {
                            ret |= RET_FAILURE;
                            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_045, {addr_split.back(), std::to_string(this->m_board_mac_addr_list.size())});
                        }
                    }
                    else
                    {
                        //hidden mode
                        if (StrMatchNoCase(gtmac_cfg.lane_param[lane_idx].src_mac_addr, ALVEO_RDM_MAC_ADDR))
                        {
                            // init random num generator
                            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                            std::default_random_engine generator (seed);
                            std::uniform_int_distribution<int> distribution(0,255);
                            uint32_t rdm_addr = uint32_t(distribution(generator)) + uint32_t(distribution(generator) << 8) + uint32_t(distribution(generator) << 16);

                            mac_addr[lane_idx].addr  = ALVEO_MAC_ADDR_POOL_1 | uint64_t(rdm_addr);
                            mac_addr[lane_idx].valid = true;
                        }
                        else
                        {
                            // if src_mac_addr string doesn't contain board_mac_addr_i nor request for random or test adress, then it's a failure
                            auto temp_str = TEST_MAC_ADDR + ", " + StrVectToStr(this->m_board_mac_addr_list, ", ") +  ", " + ALVEO_RDM_MAC_ADDR;
                            LogMessageHeader2(GetHeader2(lane_idx), MSG_CMN_001, {gtmac_cfg.lane_param[lane_idx].src_mac_addr, SOURCE_ADDR_MEMBER.name, temp_str});
                            ret |= RET_FAILURE;
                            mac_addr[lane_idx].valid = false;
                            mac_addr[lane_idx].addr  = NO_VALID_MAC_ADDRESS;
                        }
                    }
                }
            }
        }

        // save source addresses
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            gtmac_cfg.lane_param[lane_idx].Src_MAC_Addr_valid = false;
            if (mac_addr[lane_idx].valid)
            {
                gtmac_cfg.lane_param[lane_idx].Src_MAC_Addr         = mac_addr[lane_idx].addr;
                gtmac_cfg.lane_param[lane_idx].Src_MAC_Addr_valid   = true;
            }
        }
        this->m_gtmac_cfg_map[gt_index]   = gtmac_cfg;
        this->m_src_mac_addr[gt_index]    = mac_addr;
    }

    this->m_log_header = LOG_HEADER_GTMACMGMT;
    return ret;
}

bool GTMACMgmt::CreateDestinationMACAddress()
{
    // destination address is based on tx mapping
    //  then destination address can be overwritten
    auto ret = RET_SUCCESS;

    std::vector<uint64_t> dest_addr;
    dest_addr.reserve(GT_LANE_INDEXES.size());
    uint64_t mac_addr_hex;

    for (const auto & cfg : this->m_gtmac_cfg_map)
    {
        auto gt_index  = cfg.first;
        auto gtmac_cfg = cfg.second;
        set_log_header(gt_index);
        LogMessage(MSG_ETH_035); // list destination MAC address

        GT_MAC_Addr_t mac_addr;
        if (gtmac_cfg.global_param.gt_mapping.enable)
        {
            ret |= GetMACAddress(gtmac_cfg.global_param.gt_mapping.paired_gt, mac_addr);
            LogMessage(MSG_DEBUG_TESTCASE, {"GT [" + std::to_string(gt_index) + "] destination addr are source addr of GT[" + std::to_string(gtmac_cfg.global_param.gt_mapping.paired_gt) + "]"});
        }
        else
        {
            ret |= GetMACAddress(gt_index, mac_addr);
        }
        dest_addr.clear();
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if (mac_addr[gtmac_cfg.lane_param[lane_idx].tx_mapping].valid)
            {
                dest_addr.emplace_back(mac_addr[gtmac_cfg.lane_param[lane_idx].tx_mapping].addr);
            }
            else
            {
                dest_addr.emplace_back(NO_VALID_MAC_ADDRESS);
            }
        }

        // check if the string is valid: it must be
        //  - "board_mac_addr_'i'" : where 'i' represents one of the available address
        //  - 6 octet value e.g. "01:02:03:04:05:06"
        //  - "test_address"

        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if (gtmac_cfg.lane_param[lane_idx].dest_mac_addr != NO_MAC_ADDR_OVERWRITE)
            {
                if (StrMatchNoCase(gtmac_cfg.lane_param[lane_idx].dest_mac_addr, TEST_MAC_ADDR))
                {
                    dest_addr[lane_idx] = DEFAULT_TEST_MAC_ADDR | uint64_t((gt_index<<4) + lane_idx);
                }
                else
                {
                    if (FindStringInVect(gtmac_cfg.lane_param[lane_idx].dest_mac_addr, this->m_board_mac_addr_list))
                    {
                        auto addr_split = split(gtmac_cfg.lane_param[lane_idx].dest_mac_addr, '_');
                        // take the last element of the vector and convert it integer
                        if (uint(stoi(addr_split.back())) < this->m_mac_addr.size())
                        {
                            dest_addr[lane_idx] = this->m_mac_addr[stoi(addr_split.back())].addr;
                        }
                        else
                        {
                            ret |= RET_FAILURE;
                            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_045, {addr_split.back(), std::to_string(this->m_mac_addr.size())});
                        }
                    }
                    else
                    {
                        if (MacAddrStrToHex(true, gtmac_cfg.lane_param[lane_idx].dest_mac_addr, mac_addr_hex) == RET_SUCCESS)
                        {
                            // not selecting a board mac address, try to extract an address from the string
                            dest_addr[lane_idx] = mac_addr_hex;
                        }
                        else
                        {
                            // if dest_mac_addr string doesn't contain board_mac_addr_i nor a valid 6 octect address nor test address request, then it's a failure
                            auto temp_str = TEST_MAC_ADDR + ", " + StrVectToStr(this->m_board_mac_addr_list, ", " ) + ", a valid 6 octets address";
                            LogMessageHeader2(GetHeader2(lane_idx), MSG_CMN_001, {gtmac_cfg.lane_param[lane_idx].dest_mac_addr, DEST_ADDR_MEMBER.name, temp_str});
                            ret |= RET_FAILURE;
                        }
                    }
                }
            }
        }
        // save destination addresses
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            gtmac_cfg.lane_param[lane_idx].Dest_MAC_Addr_valid = false;
            if (dest_addr[lane_idx] != NO_VALID_MAC_ADDRESS)
            {
                gtmac_cfg.lane_param[lane_idx].Dest_MAC_Addr        = dest_addr[lane_idx];
                gtmac_cfg.lane_param[lane_idx].Dest_MAC_Addr_valid  = true;
            }
        }

        // save the updates
        this->m_gtmac_cfg_map[gt_index] = gtmac_cfg;
    }

    this->m_log_header = LOG_HEADER_GTMACMGMT;
    return ret;
}

bool GTMACMgmt::CheckLanesEnabled()
{
    auto ret = RET_SUCCESS;
    std::string temp_str;

    uint ETH_031_COL_0_PAD = 4;
    uint ETH_031_COL_1_PAD = 18;
    uint ETH_031_COL_2_PAD = 23;
    uint ETH_031_COL_3_PAD = 8;

    auto all_lanes_disabled = true;
    for (const auto & cfg : this->m_gtmac_cfg_map)
    {
        auto gt_index   = cfg.first;
        auto gtmac_cfg  = cfg.second;
        set_log_header(gt_index);

        // display source, destination addresses and resulting lane state
        std::vector<std::string> row_vect;
        row_vect.emplace_back(pad("Lane",                      ' ', ETH_031_COL_0_PAD, PAD_ON_RIGHT));
        row_vect.emplace_back(pad("Source MAC address",        ' ', ETH_031_COL_1_PAD, PAD_ON_RIGHT));
        row_vect.emplace_back(pad("Destination MAC address",   ' ', ETH_031_COL_2_PAD, PAD_ON_RIGHT));
        row_vect.emplace_back(pad("State",                     ' ', ETH_031_COL_3_PAD, PAD_ON_RIGHT));
        LogMessage(MSG_ETH_031, {StrVectToStr(row_vect, " | ")});

        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            row_vect.clear();
            row_vect.emplace_back(pad(std::to_string(lane_idx), ' ', ETH_031_COL_0_PAD, PAD_ON_LEFT));
            if (gtmac_cfg.lane_param[lane_idx].Src_MAC_Addr_valid)
            {
                MacAddrHexToStr(gtmac_cfg.lane_param[lane_idx].Src_MAC_Addr, temp_str);
                row_vect.emplace_back(pad(temp_str, ' ', ETH_031_COL_1_PAD, PAD_ON_LEFT));
            }
            else
            {
                gtmac_cfg.lane_param[lane_idx].disable_lane = true;
                row_vect.emplace_back(pad(NOT_APPLICABLE, ' ', ETH_031_COL_1_PAD, PAD_ON_LEFT));
            }
            if (gtmac_cfg.lane_param[lane_idx].Dest_MAC_Addr_valid)
            {
                MacAddrHexToStr(gtmac_cfg.lane_param[lane_idx].Dest_MAC_Addr, temp_str);
                row_vect.emplace_back(pad(temp_str, ' ', ETH_031_COL_2_PAD, PAD_ON_LEFT));
            }
            else
            {
                gtmac_cfg.lane_param[lane_idx].disable_lane = true;
                row_vect.emplace_back(pad(NOT_APPLICABLE, ' ', ETH_031_COL_2_PAD, PAD_ON_LEFT));
            }
            if (gtmac_cfg.lane_param[lane_idx].disable_lane)
            {
                row_vect.emplace_back(pad("DISABLED", ' ', ETH_031_COL_3_PAD, PAD_ON_LEFT));
                //LogMessage(MSG_DEBUG_TESTCASE, {"GT [" + std::to_string(gt_index)+ "] lane " + std::to_string(lane_idx) + " disabled"});
            }
            else
            {
                all_lanes_disabled = false;
                row_vect.emplace_back(pad("ENABLED", ' ', ETH_031_COL_3_PAD, PAD_ON_LEFT));
                //LogMessage(MSG_DEBUG_TESTCASE, {"GT [" + std::to_string(gt_index)+ "] lane " + std::to_string(lane_idx) + " enabled"});
            }
            LogMessage(MSG_ETH_031, {StrVectToStr(row_vect, " | ")});
        }

        // save the updates
        this->m_gtmac_cfg_map[gt_index] = gtmac_cfg;
    }

    // report failure if all lanes are disabled
    if (all_lanes_disabled)
    {
        ret |= RET_FAILURE;
        LogMessage(MSG_ETH_046);
    }

    this->m_log_header = LOG_HEADER_GTMACMGMT;
    return ret;
}

bool GTMACMgmt::FetchTestSequence( const uint & gt_index, std::vector<GTMAC_Test_Sequence_Parameters_t> & test_sequence )
{
    auto ret = RET_SUCCESS;
    set_log_header(gt_index);

    // a test sequence must be present
    auto it = FindJsonParam(this->m_test_parameters[gt_index].global_config, OPTIONAL_TEST_SEQUENCE_MEMBER);
    if (it != this->m_test_parameters[gt_index].global_config.end())
    {
        test_sequence = TestcaseParamCast<std::vector<GTMAC_Test_Sequence_Parameters_t>>(it->second);
    }
    else
    {
        LogMessage(MSG_ETH_053);
        ret = RET_FAILURE;
    }
    return ret;
}

bool GTMACMgmt::MacAddrOctetStrToHex( const bool & err_n_warn, const std::string & mac_addr_str, const uint & index, uint32_t & octet )
{
    auto ret        = RET_SUCCESS;
    auto addr_split = split(mac_addr_str, ':');
    auto octet_str  = addr_split[index];
    int  octet_int  = 0xFF;
    if (octet_str.length() == 2)
    {
        try
        {
            octet_int = std::stoi(octet_str, nullptr, 16);
        }
        catch(std::invalid_argument& e)
        {
            if (err_n_warn)
            {
                LogMessage(MSG_ETH_028_ERROR, {mac_addr_str, octet_str} );
            }
            else
            {
                LogMessage(MSG_ETH_028_WARN, {mac_addr_str, octet_str} );
            }
            ret = RET_FAILURE;
        }
        catch(std::out_of_range& e)
        {
            if (err_n_warn)
            {
                LogMessage(MSG_ETH_028_ERROR, {mac_addr_str, octet_str} );
            }
            else
            {
                LogMessage(MSG_ETH_028_WARN, {mac_addr_str, octet_str} );
            }
            ret = RET_FAILURE;
        }

        if ((octet_int < 0 ) || (octet_int > 256))
        {
            if (err_n_warn)
            {
                LogMessage(MSG_ETH_026_ERROR, {mac_addr_str, NumToStrHex(octet_int)} );
            }
            else
            {
                LogMessage(MSG_ETH_026_WARN, {mac_addr_str, NumToStrHex(octet_int)} );
            }
            ret = RET_FAILURE;
        }
    }
    else
    {
        // octet only 2 char
        if (err_n_warn)
        {
            LogMessage(MSG_ETH_029_ERROR, {mac_addr_str, octet_str} );
        }
        else
        {
            LogMessage(MSG_ETH_029_WARN, {mac_addr_str, octet_str} );
        }
    }

    octet = octet_int;
    //LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * Octet - " + NumToStrHex<uint32_t>(octet)});
    return ret;
}

bool GTMACMgmt::MacAddrStrToHex( const bool & err_n_warn, const std::string & mac_addr_str, uint64_t & mac_addr_hex )
{
    auto ret = RET_SUCCESS;
    auto addr_split = split(mac_addr_str, ':');
    uint32_t addr_octet   = 0;
    uint64_t addr_hex_msb = 0;
    uint64_t addr_hex_lsb = 0;

    mac_addr_hex = 0;

    if (addr_split.size() == 6)
    {
        // compiler doesn't like to play dynamically with 64bit vector shift; split into 2 32bits
        ret |= MacAddrOctetStrToHex(err_n_warn, mac_addr_str, 0, addr_octet);
        addr_hex_msb = addr_octet << 8;
        //LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * addr_hex_msb 0x" + NumToStrHex<uint32_t>(addr_hex_msb)});
        ret |= MacAddrOctetStrToHex(err_n_warn, mac_addr_str, 1, addr_octet);
        addr_hex_msb += addr_octet;
        //LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * addr_hex_msb 0x" + NumToStrHex<uint32_t>(addr_hex_msb)});

        for (int i = 0; i < 4; i++)
        {
            ret |=MacAddrOctetStrToHex(err_n_warn, mac_addr_str, i + 2, addr_octet);
            addr_hex_lsb += addr_octet << (3 - i) * 8;
            //LogMessage(MSG_DEBUG_TESTCASE, {"\t\t * addr_hex_lsb 0x" + NumToStrHex<uint32_t>(addr_hex_lsb)});
        }
        mac_addr_hex = (uint64_t)(addr_hex_msb << 32) + (uint64_t)addr_hex_lsb;
        //LogMessage(MSG_DEBUG_TESTCASE, {"\t - addr 0x" + NumToStrHex<uint64_t>(mac_addr_hex)});
    }
    else
    {
        if (err_n_warn)
        {
            LogMessage(MSG_ETH_027_ERROR, {mac_addr_str});
        }
        else
        {
            LogMessage(MSG_ETH_027_WARN, {mac_addr_str});
        }
        ret = RET_FAILURE;
    }
    return ret;
}

std::string GTMACMgmt::MacAddrOctetToStr( const uint32_t & addr_slice, const uint32_t & octet_idx )
{
    std::string octet_str_out;
    uint32_t temp_uint = (addr_slice >> (8 * octet_idx)) & 0xFF;
    if (temp_uint < 16)
    {
        octet_str_out =  "0" + NumToStrHex<uint32_t>(temp_uint);
    }
    else
    {
        octet_str_out = NumToStrHex<uint32_t>(temp_uint);
    }
    return octet_str_out;
}

bool GTMACMgmt::MacAddrHexToStr( const uint64_t & mac_addr_hex, std::string & mac_addr_str )
{
    auto addr           = (uint64_t)(mac_addr_hex >> 32);
    auto addr_hex_msb   = (uint32_t)addr;
    auto addr_hex_lsb   = (uint32_t)mac_addr_hex;
    if (addr_hex_msb > 0xFFFF)
    {
        LogMessage(MSG_ETH_037, {NumToStrHex<uint64_t>(mac_addr_hex)});
        //mac_addr_str = ERROR_MAC_ADDR_STR;
        return RET_FAILURE;
    }
    mac_addr_str  = MacAddrOctetToStr(addr_hex_msb,1) + ":" + MacAddrOctetToStr(addr_hex_msb,0) + ":";
    mac_addr_str += MacAddrOctetToStr(addr_hex_lsb,3) + ":" + MacAddrOctetToStr(addr_hex_lsb,2) + ":" + MacAddrOctetToStr(addr_hex_lsb,1) + ":" + MacAddrOctetToStr(addr_hex_lsb,0);
    return RET_SUCCESS;
}

bool GTMACMgmt::CheckMacAddrRange( const uint64_t & mac_addr_hex )
{
    auto addr_pool = mac_addr_hex & uint64_t(0xFFFFFFFFFF00);
    if ((addr_pool == INVALID_MAC_ADDR_POOL_1) || (mac_addr_hex == BROADCAST_MAC_ADDR))
    {
        std::string temp_str_1;
        MacAddrHexToStr(mac_addr_hex, temp_str_1);
        LogMessage(MSG_ETH_044, {temp_str_1});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool GTMACMgmt::GetTestIt( const uint & gt_index, TestItCnt_t & test_it )
{
    if (this->m_test_it_map.count(gt_index) > 0)
    {
        test_it = this->m_test_it_map[gt_index];
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool GTMACMgmt::GetMACStatus( const uint & gt_index, MACStatusCnt_t & MAC_status_cnt )
{
    if (this->m_mac_status_map.count(gt_index) > 0)
    {
        MAC_status_cnt = this->m_mac_status_map[gt_index];
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

bool GTMACMgmt::StartCU()
{
    return RET_FAILURE;
}
bool GTMACMgmt::EnableWatchdogClkThrotDetection()
{
    return RET_FAILURE;
}
bool GTMACMgmt::StopCU()
{
    return RET_FAILURE;
}
bool GTMACMgmt::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_FAILURE; // not a real xbtest HW IP, no watchdog
}
int GTMACMgmt::RunTest()
{
    return TEST_STATE_FAILURE;
}

} // namespace
