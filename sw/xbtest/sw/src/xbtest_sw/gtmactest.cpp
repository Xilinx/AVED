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

#include <thread>

#include "gtmactest.h"

namespace xbtest
{

#define READ_BUFFER_64(Buffer, Idx_64)   ( (uint64_t)( (uint64_t)(Buffer[(Idx_64 * 2)+1]) << 32 ) | (uint64_t)(Buffer[Idx_64 * 2]) )

GTMACTest::GTMACTest(
    Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, GTMACMgmt * gtmacmgmt, DeviceMgt * device_mgt,
    const Gt_Parameters_t & test_parameters, const uint & cu_idx
) : TestInterface::TestInterface(log, xbtest_sw_config, console_mgmt, timer, device)
{
    MSG_ETH_004_WARN.log_level = LOG_WARN;

    this->m_gt_index = device->GetGtmacCuGTIndex(cu_idx);

    this->m_log_header = LOG_HEADER_GTMAC;
    this->m_log_header.replace(LOG_HEADER_GTMAC.find(REP), REP.length(), std::to_string(device->GetGtmacCuGTIndex(cu_idx)));

    this->m_gtmacmgmt         = gtmacmgmt;
    this->m_devicemgt         = device_mgt;
    this->m_test_parameters   = test_parameters;
    this->m_cu_idx            = cu_idx;

    this->m_queue_testcase  = GT_MAC_MEMBER.name;
    this->m_queue_thread    = "GT_MAC[" + std::to_string(device->GetGtmacCuGTIndex(cu_idx)) + "]";
}

GTMACTest::~GTMACTest() = default;

bool GTMACTest::PreSetup()
{
    auto global_settings_failure = RET_SUCCESS;
    this->m_state = TestState::TS_PRE_SETUP;

    LogMessage(MSG_CMN_004);

    if (!(this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat()))
    {
        this->m_console_mgmt->AllocateTestcaseQueue(this->m_queue_testcase, this->m_queue_thread, this->m_testcase_queue_value);
    }

    global_settings_failure |= this->m_gtmacmgmt->GetGTMACTestConfig(this->m_gt_index, this->m_TC_Cfg);
    global_settings_failure |= InitTestSequence();

    if (!(this->m_xbtest_sw_config->GetCommandLineLogDisable()))
    {
        this->m_outputfile_name = "gtmac";
        this->m_use_outputfile  = true;
        auto first_line = GetFirstOutputLine();

        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            std::ofstream of;
            this->m_outputfile[lane_idx] = std::move(of);
            global_settings_failure |= OpenOutputFile(this->m_outputfile_name + "_gt_" + std::to_string(this->m_gt_index) + "_lane_" + std::to_string(lane_idx) + ".csv", this->m_outputfile[lane_idx] );

            std::ofstream RT_of;
            this->m_RT_outputfile[lane_idx] = std::move(RT_of);
            this->m_RT_outputfile_name[lane_idx] = this->m_outputfile_name + "_gt_" + std::to_string(this->m_gt_index) + "_lane_" + std::to_string(lane_idx) + ".csv";
            global_settings_failure |= OpenRTOutputFile(this->m_RT_outputfile_name[lane_idx], this->m_RT_outputfile[lane_idx] );

            if (global_settings_failure == RET_SUCCESS)
            {
                this->m_outputfile[lane_idx] << first_line << "\n";
                this->m_outputfile[lane_idx].flush();

                this->m_RT_outputfile_head[lane_idx] = first_line;
                this->m_RT_outputfile[lane_idx].close();
            }
        }
    }

    LogMessage(MSG_CMN_021, {"Test configuration:"});
    LogMessage(MSG_CMN_021, {"- Global configuration parameters"});
    PrintLaneParam(this->m_TC_Cfg.global_param);

    // Display here all lane parameters including hidden in DEBUG
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        LogMessage(MSG_CMN_021, {"- Lane " + std::to_string(lane_idx) + " configuration parameters"});
        PrintLaneParam(this->m_TC_Cfg.lane_param[lane_idx]);
        LogMessage(MSG_CMN_021, {"\t- " + TX_MAPPING_MEMBER.name + " : " + std::to_string(this->m_TC_Cfg.lane_param[lane_idx].tx_mapping)});
    }

    if (global_settings_failure == RET_SUCCESS)
    {
        global_settings_failure |= ParseTestSequenceSettings(this->m_test_it_list);
    }
    if (global_settings_failure == RET_FAILURE)
    {
        Abort();
    }
    return global_settings_failure;
}

void GTMACTest::Run()
{
    this->m_state     = TestState::TS_RUNNING;
    this->m_result    = TestResult::TR_FAILED;

    auto test_state = TEST_STATE_FAILURE;

    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_008, {GetCurrentTime()});
        test_state = RunTest();
    }

    if ((test_state < TEST_STATE_PASS) || (this->m_abort)) // any negatif state is an abort code
    {
        LogMessage(MSG_CMN_009, {GetCurrentTime()});
        this->m_result = TestResult::TR_ABORTED;
    }
    else if (test_state > TEST_STATE_PASS) // any positive state is an error code
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

void GTMACTest::PostTeardown()
{
    LogMessage(MSG_CMN_005);
    this->m_state = TestState::TS_POST_TEARDOWN;
}

void GTMACTest::Abort()
{
    if (!(this->m_abort))
    {
        LogMessage(MSG_CMN_006);
        this->m_abort = true;
    }
}

std::vector<GTMAC_Test_Sequence_Parameters_t> GTMACTest::GetTestSequence()
{
    return this->m_test_sequence;
}

bool GTMACTest::WriteGtmacCu( const uint & address, const uint & value)
{
    return this->m_device->WriteGtmacCu(this->m_cu_idx, address, value);
}

bool GTMACTest::ReadGtmacCu( const uint & address, uint & read_data )
{
    return this->m_device->ReadGtmacCu(this->m_cu_idx, address, read_data);
}

bool GTMACTest::ReadGtmacCu64b( const uint & addr_lsb, const uint & addr_msb, uint64_t & read_data_64 )
{
    read_data_64 = 0;
    uint read_data;

    if (ReadGtmacCu(addr_msb, read_data) == RET_FAILURE) { return RET_FAILURE; }
    read_data_64 = (uint64_t)(read_data);
    read_data_64 = (read_data_64 << 32);

    if (ReadGtmacCu(addr_lsb, read_data) == RET_FAILURE) { return RET_FAILURE; }
    read_data_64 |= (uint64_t)(read_data);

    return RET_SUCCESS;
}

bool GTMACTest::WriteGtmacCuCmd ( const uint & value )
{
    return this->m_device->WriteGtmacCuCmd(this->m_cu_idx, value);
}

bool GTMACTest::WriteGtmacCuTrafficCfg( uint32_t * traffic_cfg )
{
    return this->m_device->WriteGtmacCuTrafficCfg(this->m_cu_idx, traffic_cfg);
}

bool GTMACTest::ReadGtmacCuTrafficCfg ( uint32_t * status )
{
    return this->m_device->ReadGtmacCuTrafficCfg(this->m_cu_idx, status);
}

uint32_t GTMACTest::CalcScript( const std::string & rate, const uint32_t & packet_size, const float & utilisation )
{
    uint32_t        ret         = 0xFFFFF800;       //set the delay to max (21 MSB) and size to 0 (11 LSB)
    const double    FUDGE       = 0.996;            // Adjustment to match rate on Viavi analyser
    auto            enc_size    = SMALL_PACKET_SIZE_MIN;

    // Check the packet size is an allowed value, and convert to the hw range of 0..2047
    // Zero and invalid returns no packet and maximum delay
    if ((packet_size >= SMALL_PACKET_SIZE_MIN) && (packet_size <= SMALL_PACKET_SIZE_MAX))
    {
        enc_size = packet_size;
    }
    else if ((packet_size >= BIG_PACKET_SIZE_MIN) && (packet_size <= BIG_PACKET_SIZE_MAX))
    {
        enc_size = ((packet_size - BIG_PACKET_SIZE_MIN) / 2) + 1536;
    }

    // For max utilisation, set the delay to 0
    if (utilisation >= (float)100.0)
    {
        ret = enc_size & 0x000007FF;
    }
    else
    {
        if (utilisation <= (float)0.0)
        {
            ret = 0xFFFFF800; // Set the delay to max and size to 0
        }
        else
        {
            double bytes_per_clk = ( (rate == LANE_RATE_10GBE) ? 10000.0 : 25000.0 ) / (300.0 * 8.0);

            // Add Preamble (8 bytes) and IPG (Inter packet Gap), round up to the next 4 byte boundary, Scale by 100 as utilisation is %
            // Calculate the number of clock cycles delay required, turnaround is 3 cycles, use 2.5 to allow truncation to integer
            uint32_t bytes = (((packet_size + 8 + IPG_DEFAULT + 3) / 4) * 4) * 100;
            double   delay = (((double)(bytes) * FUDGE) / ((double)(utilisation) * bytes_per_clk)) - 2.5;

            // Check if the required utilisation exceeds the counter range, if it does simply set the counter to Max
            if (delay >= (1 << 21))
            {
                ret =  0xFFFFF800 | (enc_size & 0x000007FF);
            }
            else
            {
                ret = ((uint32_t(delay) << 11) & 0xFFFFF800) | (enc_size & 0x000007FF);
            }
        }
    }

    return ret;
}

bool  GTMACTest::ParseMACStatus( uint32_t * status, const uint & lane_idx )
{
    auto ret = RET_SUCCESS;

    for (uint32_t n = 0; n < MAC_NUM_STATS; n++)
    {
        auto Stats_var = READ_BUFFER_64(status, n);
        if (MAC_STAT_ERR_TRIG[n]) // Check if this is a Counter that should fail the test
        {
            if (Stats_var > 0)
            {
                LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_004_ERROR, {std::to_string(Stats_var), MAC_STAT_NAMES[n]});
                ret |= RET_FAILURE;
            }
            else
            {
                LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_005, {std::to_string(Stats_var), MAC_STAT_NAMES[n]});
            }
        }
        else
        {
            if (MAC_STAT_WARNING_TRIG[n]) // Check if this is a Counter that should create a warning
            {
                if (Stats_var > 0)
                {
                    LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_004_WARN, {std::to_string(Stats_var), MAC_STAT_NAMES[n]});
                }
                else
                {
                    LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_005, {std::to_string(Stats_var), MAC_STAT_NAMES[n]});
                }
            }
            else
            {
                LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_006, {MAC_STAT_NAMES[n], std::to_string(Stats_var)});
            }
        }
    }

    if (0 == READ_BUFFER_64(status, MAC_IDX_RX_GOOD_PAC))
    {
        LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_007);
        ret |= RET_FAILURE;
    }

    auto mac_tx_byte_sent   = READ_BUFFER_64(status, MAC_IDX_TX_SENT_BYTE);
    auto mac_tx_packet_sent = READ_BUFFER_64(status, MAC_IDX_TX_SENT_PAC);

    ret |= CheckMACBytesPacketsCounters(lane_idx, mac_tx_byte_sent, mac_tx_packet_sent);

    return ret;
}

bool GTMACTest::CheckTrafficCounters( uint32_t * status_tx, const uint & lane_idx, const bool & Check_Tx_Rx )
{
    auto ret = RET_SUCCESS;
    uint64_t Stats_tx_var;

    if (Check_Tx_Rx)
    {
        // Check if Rx filtered Packets = Tx Packets
        Stats_tx_var = READ_BUFFER_64(status_tx, MAC_IDX_TX_SENT_PAC);
        if (Stats_tx_var != this->m_traffic_counters[lane_idx].Good_Packet)
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_024, {std::to_string(this->m_traffic_counters[lane_idx].Good_Packet), std::to_string(Stats_tx_var)});
            ret |= RET_FAILURE;
        }
        else
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_022, {std::to_string(Stats_tx_var)});
        }

        // Check if Rx filtered Bytes = Tx Bytes
        Stats_tx_var = READ_BUFFER_64(status_tx, MAC_IDX_TX_SENT_BYTE);
        if (Stats_tx_var != this->m_traffic_counters[lane_idx].Good_Byte)
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_025, {std::to_string(this->m_traffic_counters[lane_idx].Good_Byte), std::to_string(Stats_tx_var)});
            ret |= RET_FAILURE;
        }
        else
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_023, {std::to_string(Stats_tx_var)});
        }

    }

    return ret;
}

void GTMACTest::SendGTMACCmd( const MACCmd_t & cmd )
{
    uint MACCmd = 0x0001 << 19;
    // clear status latches them first, so "clear" or "check" are the same command
    if ( (cmd == CLEAR_STATUS) || (cmd == CHECK_STATUS) )
    {
        MACCmd |= 0x0001 << 16;
    }
    else if (cmd == CONFIG_SEND)
    {
        MACCmd |= 0x0001 << 17;
    }
    else if (cmd == CU_RUN)
    {
        MACCmd |= 0x0001 << 18;
    }
    else if (cmd == TX_RST)
    {
        MACCmd |= 0x0001 << 21;
    }
    else if (cmd == RX_RST)
    {
        MACCmd |= 0x0002 << 21;
    }
    else if (cmd == TX_RX_RST)
    {
        MACCmd |= 0x0003 << 21;
    }
    // if (cmd == STOP) nothing to do
    WriteGtmacCuCmd(MACCmd);
}

bool GTMACTest::CheckLinkDown( const bool & expected_link_down )
{
    auto link_failure = RET_SUCCESS;
    uint read_data;

    ReadGtmacCu(CMN_STATUS_LATCH_ADDR, read_data);

    if (expected_link_down)
    {
        // a reset had probably occured, so it's not an issue to detect link down
        // report INFO
        link_failure = RET_SUCCESS;

        if ( !(this->m_TC_Cfg.lane_param[0].disable_lane) && ((read_data & STATUS_RX_LINK_0) == STATUS_RX_LINK_0) )
        {
            LogMessageHeader2(GetHeader2(0), MSG_ETH_015);
        }
        if ( !(this->m_TC_Cfg.lane_param[1].disable_lane) && ((read_data & STATUS_RX_LINK_1) == STATUS_RX_LINK_1) )
        {
            LogMessageHeader2(GetHeader2(1), MSG_ETH_015);
        }
        if ( !(this->m_TC_Cfg.lane_param[2].disable_lane) && ((read_data & STATUS_RX_LINK_2) == STATUS_RX_LINK_2) )
        {
            LogMessageHeader2(GetHeader2(2), MSG_ETH_015);
        }
        if ( !(this->m_TC_Cfg.lane_param[3].disable_lane) && ((read_data & STATUS_RX_LINK_3) == STATUS_RX_LINK_3) )
        {
            LogMessageHeader2(GetHeader2(3), MSG_ETH_015);
        }

        if ( !(this->m_TC_Cfg.lane_param[0].disable_lane) && ((read_data & STATUS_USR_TX_RST_0) == STATUS_USR_TX_RST_0) )
        {
            LogMessageHeader2(GetHeader2(0), MSG_ETH_017);
        }
        if ( !(this->m_TC_Cfg.lane_param[1].disable_lane) && ((read_data & STATUS_USR_TX_RST_1) == STATUS_USR_TX_RST_1) )
        {
            LogMessageHeader2(GetHeader2(1), MSG_ETH_017);
        }
        if ( !(this->m_TC_Cfg.lane_param[2].disable_lane) && ((read_data & STATUS_USR_TX_RST_2) == STATUS_USR_TX_RST_2) )
        {
            LogMessageHeader2(GetHeader2(2), MSG_ETH_017);
        }
        if ( !(this->m_TC_Cfg.lane_param[3].disable_lane) && ((read_data & STATUS_USR_TX_RST_3) == STATUS_USR_TX_RST_3) )
        {
            LogMessageHeader2(GetHeader2(3), MSG_ETH_017);
        }

        if ( !(this->m_TC_Cfg.lane_param[0].disable_lane) && ((read_data & STATUS_USR_RX_RST_0) == STATUS_USR_RX_RST_0) )
        {
            LogMessageHeader2(GetHeader2(0), MSG_ETH_019);
        }
        if ( !(this->m_TC_Cfg.lane_param[1].disable_lane) && ((read_data & STATUS_USR_RX_RST_1) == STATUS_USR_RX_RST_1) )
        {
            LogMessageHeader2(GetHeader2(1), MSG_ETH_019);
        }
        if ( !(this->m_TC_Cfg.lane_param[2].disable_lane) && ((read_data & STATUS_USR_RX_RST_2) == STATUS_USR_RX_RST_2) )
        {
            LogMessageHeader2(GetHeader2(2), MSG_ETH_019);
        }
        if ( !(this->m_TC_Cfg.lane_param[3].disable_lane) && ((read_data & STATUS_USR_RX_RST_3) == STATUS_USR_RX_RST_3) )
        {
            LogMessageHeader2(GetHeader2(3), MSG_ETH_019);
        }
    }
    else
    {
        // it's an issue to get a link down or detect TX/RX reset when a reset is NOT performed
        if ( !(this->m_TC_Cfg.lane_param[0].disable_lane) && ((read_data & STATUS_RX_LINK_0) == STATUS_RX_LINK_0) )
        {
            LogMessageHeader2(GetHeader2(0), MSG_ETH_014);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[1].disable_lane) && ((read_data & STATUS_RX_LINK_1) == STATUS_RX_LINK_1) )
        {
            LogMessageHeader2(GetHeader2(1), MSG_ETH_014);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[2].disable_lane) && ((read_data & STATUS_RX_LINK_2) == STATUS_RX_LINK_2) )
        {
            LogMessageHeader2(GetHeader2(2), MSG_ETH_014);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[3].disable_lane) && ((read_data & STATUS_RX_LINK_3) == STATUS_RX_LINK_3) )
        {
            LogMessageHeader2(GetHeader2(3), MSG_ETH_014);
            link_failure = RET_FAILURE;
        }

        if ( !(this->m_TC_Cfg.lane_param[0].disable_lane) && ((read_data & STATUS_USR_TX_RST_0) == STATUS_USR_TX_RST_0) )
        {
            LogMessageHeader2(GetHeader2(0), MSG_ETH_016);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[1].disable_lane) && ((read_data & STATUS_USR_TX_RST_1) == STATUS_USR_TX_RST_1) )
        {
            LogMessageHeader2(GetHeader2(1), MSG_ETH_016);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[2].disable_lane) && ((read_data & STATUS_USR_TX_RST_2) == STATUS_USR_TX_RST_2) )
        {
            LogMessageHeader2(GetHeader2(2), MSG_ETH_016);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[3].disable_lane) && ((read_data & STATUS_USR_TX_RST_3) == STATUS_USR_TX_RST_3) )
        {
            LogMessageHeader2(GetHeader2(3), MSG_ETH_016);
            link_failure = RET_FAILURE;
        }

        if ( !(this->m_TC_Cfg.lane_param[0].disable_lane) && ((read_data & STATUS_USR_RX_RST_0) == STATUS_USR_RX_RST_0) )
        {
            LogMessageHeader2(GetHeader2(0), MSG_ETH_018);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[1].disable_lane) && ((read_data & STATUS_USR_RX_RST_1) == STATUS_USR_RX_RST_1) )
        {
            LogMessageHeader2(GetHeader2(1), MSG_ETH_018);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[2].disable_lane) && ((read_data & STATUS_USR_RX_RST_2) == STATUS_USR_RX_RST_2) )
        {
            LogMessageHeader2(GetHeader2(2), MSG_ETH_018);
            link_failure = RET_FAILURE;
        }
        if ( !(this->m_TC_Cfg.lane_param[3].disable_lane) && ((read_data & STATUS_USR_RX_RST_3) == STATUS_USR_RX_RST_3) )
        {
            LogMessageHeader2(GetHeader2(3), MSG_ETH_018);
            link_failure = RET_FAILURE;
        }
    }

    return link_failure;
}

bool GTMACTest::CheckLinkSpeed( const std::string & lane_rate )
{
    auto rate_failure = RET_SUCCESS;
    uint read_data;

    ReadGtmacCu(CMN_STATUS_LIVE_ADDR, read_data);
    if (lane_rate == LANE_RATE_10GBE)
    {
        if ( (read_data & STATUS_LANE_RATE_0_3) != STATUS_LANE_RATE_10_GBE)
        {
            LogMessage(MSG_ETH_040, {LANE_RATE_10GBE, LANE_RATE_25GBE, std::to_string(read_data & STATUS_LANE_RATE_0_3)});
            rate_failure = RET_FAILURE;
        }
    }
    else
    {
        if ( (read_data & STATUS_LANE_RATE_0_3) != STATUS_LANE_RATE_25_GBE)
        {
            LogMessage(MSG_ETH_040, {LANE_RATE_25GBE, LANE_RATE_10GBE, std::to_string(read_data & STATUS_LANE_RATE_0_3)});
            rate_failure = RET_FAILURE;
        }
    }
    return rate_failure;
}

void GTMACTest::ClearLatchStatus()
{
    WriteGtmacCu(CMN_STATUS_LATCH_ADDR, 0x0);
}

std::string GTMACTest::MacAddrToStr(uint64_t mac_addr)
{
    // reserve byte ordering before converting
    uint64_t addr;
    addr = (uint64_t)((mac_addr & 0xFF) << 40);
    addr |= (uint64_t)((mac_addr & 0xFF00) << 24);
    addr |= (uint64_t)((mac_addr & 0xFF0000) << 8);
    addr |= (uint64_t)((mac_addr & 0xFF000000) >> 8);
    addr |= (uint64_t)((mac_addr & 0xFF00000000) >> 24);
    addr |= (uint64_t)((mac_addr & 0xFF0000000000) >> 40);
    std::string temp_str;
    this->m_gtmacmgmt->MacAddrHexToStr(addr, temp_str);
    return temp_str;
}

void GTMACTest::GetTrafficCounters()
{
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        this->m_traffic_counters[lane_idx] = RST_TRAFFICCOUNTERS;
    }
    ReadGtmacCu64b(TC_GOOD_PACKET_0_LSB_ADDR,     TC_GOOD_PACKET_0_MSB_ADDR,        this->m_traffic_counters[0].Good_Packet         );
    ReadGtmacCu64b(TC_GOOD_BYTE_0_LSB_ADDR,       TC_GOOD_BYTE_0_MSB_ADDR,          this->m_traffic_counters[0].Good_Byte           );
    ReadGtmacCu64b(TC_TOTAL_PACKET_0_LSB_ADDR,    TC_TOTAL_PACKET_0_MSB_ADDR,       this->m_traffic_counters[0].Total_Packet        );
    ReadGtmacCu64b(TC_BAD_DEST_ADDR_0_LSB_ADDR,   TC_BAD_DEST_ADDR_0_MSB_ADDR,      this->m_traffic_counters[0].Bad_Dest_Addr       );
    ReadGtmacCu64b(LAST_BAD_DEST_ADDR_0_LSB_ADDR, LAST_BAD_DEST_ADDR_0_MSB_ADDR,    this->m_traffic_counters[0].Last_Bad_Dest_Addr  );
    ReadGtmacCu64b(TC_BAD_SRC_ADDR_0_LSB_ADDR,    TC_BAD_SRC_ADDR_0_MSB_ADDR,       this->m_traffic_counters[0].Bad_Src_Addr        );
    ReadGtmacCu64b(LAST_BAD_SRC_ADDR_0_LSB_ADDR,  LAST_BAD_SRC_ADDR_0_MSB_ADDR,     this->m_traffic_counters[0].Last_Bad_Src_Addr   );
    ReadGtmacCu64b(FREQ_CNT_0_LSB_ADDR,           FREQ_CNT_0_MSB_ADDR,              this->m_traffic_counters[0].Freq_Cnt            );

    ReadGtmacCu64b(TC_GOOD_PACKET_1_LSB_ADDR,     TC_GOOD_PACKET_1_MSB_ADDR,        this->m_traffic_counters[1].Good_Packet         );
    ReadGtmacCu64b(TC_GOOD_BYTE_1_LSB_ADDR,       TC_GOOD_BYTE_1_MSB_ADDR,          this->m_traffic_counters[1].Good_Byte           );
    ReadGtmacCu64b(TC_TOTAL_PACKET_1_LSB_ADDR,    TC_TOTAL_PACKET_1_MSB_ADDR,       this->m_traffic_counters[1].Total_Packet        );
    ReadGtmacCu64b(TC_BAD_DEST_ADDR_1_LSB_ADDR,   TC_BAD_DEST_ADDR_1_MSB_ADDR,      this->m_traffic_counters[1].Bad_Dest_Addr       );
    ReadGtmacCu64b(LAST_BAD_DEST_ADDR_1_LSB_ADDR, LAST_BAD_DEST_ADDR_1_MSB_ADDR,    this->m_traffic_counters[1].Last_Bad_Dest_Addr  );
    ReadGtmacCu64b(TC_BAD_SRC_ADDR_1_LSB_ADDR,    TC_BAD_SRC_ADDR_1_MSB_ADDR,       this->m_traffic_counters[1].Bad_Src_Addr        );
    ReadGtmacCu64b(LAST_BAD_SRC_ADDR_1_LSB_ADDR,  LAST_BAD_SRC_ADDR_1_MSB_ADDR,     this->m_traffic_counters[1].Last_Bad_Src_Addr   );
    ReadGtmacCu64b(FREQ_CNT_1_LSB_ADDR,           FREQ_CNT_1_MSB_ADDR,              this->m_traffic_counters[1].Freq_Cnt            );

    ReadGtmacCu64b(TC_GOOD_PACKET_2_LSB_ADDR,     TC_GOOD_PACKET_2_MSB_ADDR,        this->m_traffic_counters[2].Good_Packet         );
    ReadGtmacCu64b(TC_GOOD_BYTE_2_LSB_ADDR,       TC_GOOD_BYTE_2_MSB_ADDR,          this->m_traffic_counters[2].Good_Byte           );
    ReadGtmacCu64b(TC_TOTAL_PACKET_2_LSB_ADDR,    TC_TOTAL_PACKET_2_MSB_ADDR,       this->m_traffic_counters[2].Total_Packet        );
    ReadGtmacCu64b(TC_BAD_DEST_ADDR_2_LSB_ADDR,   TC_BAD_DEST_ADDR_2_MSB_ADDR,      this->m_traffic_counters[2].Bad_Dest_Addr       );
    ReadGtmacCu64b(LAST_BAD_DEST_ADDR_2_LSB_ADDR, LAST_BAD_DEST_ADDR_2_MSB_ADDR,    this->m_traffic_counters[2].Last_Bad_Dest_Addr  );
    ReadGtmacCu64b(TC_BAD_SRC_ADDR_2_LSB_ADDR,    TC_BAD_SRC_ADDR_2_MSB_ADDR,       this->m_traffic_counters[2].Bad_Src_Addr        );
    ReadGtmacCu64b(LAST_BAD_SRC_ADDR_2_LSB_ADDR,  LAST_BAD_SRC_ADDR_2_MSB_ADDR,     this->m_traffic_counters[2].Last_Bad_Src_Addr   );
    ReadGtmacCu64b(FREQ_CNT_2_LSB_ADDR,           FREQ_CNT_2_MSB_ADDR,              this->m_traffic_counters[2].Freq_Cnt            );

    ReadGtmacCu64b(TC_GOOD_PACKET_3_LSB_ADDR,     TC_GOOD_PACKET_3_MSB_ADDR,        this->m_traffic_counters[3].Good_Packet         );
    ReadGtmacCu64b(TC_GOOD_BYTE_3_LSB_ADDR,       TC_GOOD_BYTE_3_MSB_ADDR,          this->m_traffic_counters[3].Good_Byte           );
    ReadGtmacCu64b(TC_TOTAL_PACKET_3_LSB_ADDR,    TC_TOTAL_PACKET_3_MSB_ADDR,       this->m_traffic_counters[3].Total_Packet        );
    ReadGtmacCu64b(TC_BAD_DEST_ADDR_3_LSB_ADDR,   TC_BAD_DEST_ADDR_3_MSB_ADDR,      this->m_traffic_counters[3].Bad_Dest_Addr       );
    ReadGtmacCu64b(LAST_BAD_DEST_ADDR_3_LSB_ADDR, LAST_BAD_DEST_ADDR_3_MSB_ADDR,    this->m_traffic_counters[3].Last_Bad_Dest_Addr  );
    ReadGtmacCu64b(TC_BAD_SRC_ADDR_3_LSB_ADDR,    TC_BAD_SRC_ADDR_3_MSB_ADDR,       this->m_traffic_counters[3].Bad_Src_Addr        );
    ReadGtmacCu64b(LAST_BAD_SRC_ADDR_3_LSB_ADDR,  LAST_BAD_SRC_ADDR_3_MSB_ADDR,     this->m_traffic_counters[3].Last_Bad_Src_Addr   );
    ReadGtmacCu64b(FREQ_CNT_3_LSB_ADDR,           FREQ_CNT_3_MSB_ADDR,              this->m_traffic_counters[3].Freq_Cnt            );

    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Traffic Counter for lane: " + std::to_string(lane_idx)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Total Packet         : " + std::to_string(this->m_traffic_counters[lane_idx].Total_Packet)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Good Packet          : " + std::to_string(this->m_traffic_counters[lane_idx].Good_Packet)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Good byte            : " + std::to_string(this->m_traffic_counters[lane_idx].Good_Byte)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Bad Dest Addr Packet : " + std::to_string(this->m_traffic_counters[lane_idx].Bad_Dest_Addr)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Last Bad Dest Addr   : " + MacAddrToStr(this->m_traffic_counters[lane_idx].Last_Bad_Dest_Addr)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Bad Src Addr Packet  : " + std::to_string(this->m_traffic_counters[lane_idx].Bad_Src_Addr)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - Last Bad Src Addr    : " + MacAddrToStr(this->m_traffic_counters[lane_idx].Last_Bad_Src_Addr)});
        LogMessage(MSG_DEBUG_TESTCASE, {"\t - tx_clk_out freq (Hz) : " + std::to_string(this->m_traffic_counters[lane_idx].Freq_Cnt)});
    }
}

GTMACTest::Rate_t GTMACTest::GetExpectedRates( const std::string & rate, const uint32_t & packet_size, const float & utilisation )
{
    Rate_t              ret;
    std::vector<Rate_t> rate_array;

    float byte_rate   = 0.0;
    float packet_rate = 0.0;
    float slope       = 0.0;

    if (rate == LANE_RATE_10GBE)
    {
        rate_array = BYTE_RATE_10GBE;
    }
    else
    {
        rate_array = BYTE_RATE_25GBE;
    }

    for (std::size_t idx = 0; idx != rate_array.size(); ++idx)
    {
        //LogMessage(MSG_DEBUG_TESTCASE, {"\t - Index: " + std::to_string(idx)});
        if (packet_size == rate_array[idx].packet_size)
        {
            byte_rate = rate_array[idx].byte_rate;
            packet_rate = rate_array[idx].packet_rate;
            break;
        }
        // use linear interpolation in between each point.
        // find in between each set of point the packet size belong:e.g. 112:   64 < 112 < 128
        if (packet_size < rate_array[idx].packet_size)
        {
            slope = (rate_array[idx].byte_rate - rate_array[idx-1].byte_rate)/(rate_array[idx].packet_size-rate_array[idx-1].packet_size);
            //LogMessage(MSG_DEBUG_TESTCASE, {"\t - Byte slope: " + std::to_string(slope)});
            byte_rate = rate_array[idx-1].byte_rate + (packet_size-rate_array[idx-1].packet_size) * slope;

            slope = (rate_array[idx].packet_rate - rate_array[idx-1].packet_rate)/(rate_array[idx].packet_size-rate_array[idx-1].packet_size);
            //LogMessage(MSG_DEBUG_TESTCASE, {"\t - Packet slope: " + std::to_string(slope)});
            packet_rate = rate_array[idx-1].packet_rate + (packet_size-rate_array[idx-1].packet_size) * slope;
            break;
        }
    }

    //LogMessage(MSG_DEBUG_TESTCASE, "\t - Packet size: " + std::to_string(packet_size)});
    //LogMessage(MSG_DEBUG_TESTCASE, "\t - Byte rate: "   + std::to_string(byte_rate)});
    //LogMessage(MSG_DEBUG_TESTCASE, "\t - Packet rate: " + std::to_string(packet_rate)});

    ret.packet_size = packet_size;
    ret.byte_rate   = byte_rate   * (utilisation / 100.0);
    ret.packet_rate = packet_rate * (utilisation / 100.0);
    LogMessage(MSG_DEBUG_TESTCASE, {"Expected rates for a packet size of " + std::to_string(packet_size) + " and an utilisation of " + Float_to_String<float>(utilisation,0) + " %"});
    LogMessage(MSG_DEBUG_TESTCASE, {"\t - Byte rate (B/s): " + Float_to_String<float>(ret.byte_rate,0)});
    LogMessage(MSG_DEBUG_TESTCASE, {"\t - Packet rate (p/s): " + Float_to_String<float>(ret.packet_rate,0)});

    return ret;
}

bool GTMACTest::CheckMACBytesPacketsCounters( const uint & lane_idx, const uint64_t & mac_tx_byte_sent, const uint64_t & mac_tx_packet_sent )
{
    auto ret = RET_SUCCESS;
    Rate_t lane_rate;
    float exp_cnt;
    float min_exp_cnt;
    float max_exp_cnt;

    if (StrMatchNoCase(this->m_TC_Cfg.lane_param[lane_idx].packet_cfg, PACKET_CFG_SWEEP))
    {
        // sweep value are similar to a 768 packet size
        lane_rate = GetExpectedRates(this->m_TC_Cfg.global_param.lane_rate, 768, this->m_TC_Cfg.lane_param[lane_idx].utilisation);
    }
    else
    {
        lane_rate = GetExpectedRates(this->m_TC_Cfg.global_param.lane_rate, this->m_TC_Cfg.lane_param[lane_idx].packet_size, this->m_TC_Cfg.lane_param[lane_idx].utilisation);
    }

    if (!(this->m_TC_Cfg.lane_param[lane_idx].disable_lane))
    {
        exp_cnt = (float)this->m_run_duration * lane_rate.packet_rate;
        // takes 20% margin
        min_exp_cnt = (exp_cnt / 10) * 8;
        max_exp_cnt = (exp_cnt / 10) * 12;
        if ( (mac_tx_packet_sent >= min_exp_cnt) && (mac_tx_packet_sent <= max_exp_cnt) )
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_041, {"packets", std::to_string(mac_tx_packet_sent),  Float_to_String<float>(min_exp_cnt,0), Float_to_String<float>(max_exp_cnt,0)});
        }
        else
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_042, {"packets", std::to_string(mac_tx_packet_sent),  Float_to_String<float>(min_exp_cnt,0), Float_to_String<float>(max_exp_cnt,0)});
        }

        if ( mac_tx_packet_sent == 0)
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_048, {"packets"});
            ret = RET_FAILURE;
        }

        exp_cnt = (float)this->m_run_duration * lane_rate.byte_rate;
        // takes 20% margin
        min_exp_cnt = (exp_cnt / 10 ) * 8;
        max_exp_cnt = (exp_cnt / 10 ) * 12;
        if ( (mac_tx_byte_sent >= min_exp_cnt) && (mac_tx_byte_sent <= max_exp_cnt) )
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_041, {"bytes", std::to_string(mac_tx_byte_sent), Float_to_String<float>(min_exp_cnt,0), Float_to_String<float>(max_exp_cnt,0)});
        }
        else
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_042, {"bytes", std::to_string(mac_tx_byte_sent), Float_to_String<float>(min_exp_cnt,0), Float_to_String<float>(max_exp_cnt,0)});
        }

        if (mac_tx_byte_sent == 0)
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_048, {"bytes"});
            ret = RET_FAILURE;
        }

    }
    return ret;
}

bool GTMACTest::CreateMACConfig( uint32_t Traffic_Cfg[], std::map<uint, MAC_Config_t> & MAC_Config )
{
    auto config_falure = RET_SUCCESS;
    uint32_t traffic_gen_cfg;

    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        // Fixed MAC configuration
        MAC_Config[lane_idx].EtherType     = 0x88b5;
        MAC_Config[lane_idx].Set_LFI       = FALSE;
        MAC_Config[lane_idx].Set_RFI       = FALSE;
        MAC_Config[lane_idx].Set_Idle      = FALSE;
        MAC_Config[lane_idx].Lcl_Loopback  = FALSE;
        MAC_Config[lane_idx].Script_Base   = 1904 + (lane_idx * 36);
        MAC_Config[lane_idx].Script_Length = 1;

        // MAC configuration JSON override
        MAC_Config[lane_idx].Active        = !(this->m_TC_Cfg.lane_param[lane_idx].disable_lane);

        MAC_Config[lane_idx].Dest_Addr     = this->m_TC_Cfg.lane_param[lane_idx].Dest_MAC_Addr;
        MAC_Config[lane_idx].Source_Addr   = this->m_TC_Cfg.lane_param[lane_idx].Src_MAC_Addr;
        MAC_Config[lane_idx].Match_Tx_Rx   = this->m_TC_Cfg.lane_param[lane_idx].match_tx_rx;
        // Transceiver Configuration
        MAC_Config[lane_idx].GT_Tx_Diff         = this->m_TC_Cfg.lane_param[lane_idx].gt_tx_diffctrl;
        MAC_Config[lane_idx].GT_Tx_Main_Cursor  = this->m_TC_Cfg.lane_param[lane_idx].gt_tx_main_cursor;
        MAC_Config[lane_idx].GT_Tx_Pre          = this->m_TC_Cfg.lane_param[lane_idx].gt_tx_pre_emph;
        MAC_Config[lane_idx].GT_Tx_Post         = this->m_TC_Cfg.lane_param[lane_idx].gt_tx_post_emph;
        MAC_Config[lane_idx].GT_Rx_Eq_Sel       = this->m_TC_Cfg.lane_param[lane_idx].gt_rx_use_lpm;

        MAC_Config[lane_idx].tx_polarity   = false;
        if (StrMatchNoCase(this->m_TC_Cfg.lane_param[lane_idx].tx_polarity, TX_POLARITY_INVERTED))
        {
            MAC_Config[lane_idx].tx_polarity   = true;
        }

        // Set up conditions for Packet Sweep: only use global settings (utilisation)
        if (StrMatchNoCase(this->m_TC_Cfg.lane_param[lane_idx].packet_cfg, PACKET_CFG_SWEEP))
        {
            MAC_Config[lane_idx].Script_Base   = 448;
            MAC_Config[lane_idx].Script_Length = 1518 - 64 + 1;
            MAC_Config[lane_idx].Utilisation   = (float)this->m_TC_Cfg.global_param.utilisation;
            config_falure |= TrafficTypeParam2Setting(lane_idx, this->m_TC_Cfg.global_param.traffic_type, MAC_Config[lane_idx].Traffic_Type);
        }
        else
        {
            // Set up single channel scripts
            MAC_Config[lane_idx].Utilisation   = (float)this->m_TC_Cfg.lane_param[lane_idx].utilisation;
            config_falure |= TrafficTypeParam2Setting(lane_idx, this->m_TC_Cfg.lane_param[lane_idx].traffic_type, MAC_Config[lane_idx].Traffic_Type);
            traffic_gen_cfg = CalcScript(this->m_TC_Cfg.global_param.lane_rate, this->m_TC_Cfg.lane_param[lane_idx].packet_size, MAC_Config[lane_idx].Utilisation);
            Traffic_Cfg[MAC_Config[lane_idx].Script_Base] = traffic_gen_cfg;
            LogMessage(MSG_DEBUG_TESTCASE, {"Traffic config for lane " + std::to_string(lane_idx) + ": delay " + std::to_string( ((traffic_gen_cfg & 0xFFFFF800) >> 11) ) + ", size " + std::to_string(traffic_gen_cfg & 0x000007FF)});
        }
        // Adjust the MTU to match Tx frame generation
        if (StrMatchNoCase(this->m_TC_Cfg.lane_param[this->m_TC_Cfg.lane_param[lane_idx].rx_mapping].packet_cfg, PACKET_CFG_SWEEP))
        {
            MAC_Config[lane_idx].MTU = 1518;
        }
        else
        {
            if (this->m_TC_Cfg.lane_param[this->m_TC_Cfg.lane_param[lane_idx].rx_mapping].packet_size > 9600)
            {
                MAC_Config[lane_idx].MTU   = 10000;
            }
            else if (this->m_TC_Cfg.lane_param[this->m_TC_Cfg.lane_param[lane_idx].rx_mapping].packet_size > 1518)
            {
                MAC_Config[lane_idx].MTU   = 9600;
            }
            else
            {
                MAC_Config[lane_idx].MTU   = 1518;
            }
        }
        // For Inactive channels, set the Script length to zero (Disable Packet Generator)
        if (!MAC_Config[lane_idx].Active)
        {
            MAC_Config[lane_idx].Script_Length = 0;
        }
        // Internal Registers
        Traffic_Cfg[(lane_idx*16)+ 1]  = ((MAC_Config[lane_idx].Script_Length & 0x0000FFFF) << 16) | (MAC_Config[lane_idx].Script_Base & 0x0000FFFF);    // Script Length / Base
        Traffic_Cfg[(lane_idx*16)+ 2]  = ((MAC_Config[lane_idx].Dest_Addr   <<  8) & 0xFF000000) | ((MAC_Config[lane_idx].Dest_Addr   >>  8) & 0x00FF0000) | ((MAC_Config[lane_idx].Dest_Addr   >> 24) & 0x0000FF00) | ((MAC_Config[lane_idx].Dest_Addr   >> 40) & 0x000000FF);
        Traffic_Cfg[(lane_idx*16)+ 3]  = ((MAC_Config[lane_idx].Source_Addr >>  8) & 0xFF000000) | ((MAC_Config[lane_idx].Source_Addr >> 24) & 0x00FF0000) | ((MAC_Config[lane_idx].Dest_Addr   <<  8) & 0x0000FF00) | ((MAC_Config[lane_idx].Dest_Addr   >>  8) & 0x000000FF);
        Traffic_Cfg[(lane_idx*16)+ 4]  = ((MAC_Config[lane_idx].Source_Addr << 24) & 0xFF000000) | ((MAC_Config[lane_idx].Source_Addr <<  8) & 0x00FF0000) | ((MAC_Config[lane_idx].Source_Addr >>  8) & 0x0000FF00) | ((MAC_Config[lane_idx].Source_Addr >> 24) & 0x000000FF);
        // Test Traffic Type / EtherType
        Traffic_Cfg[(lane_idx*16)+ 5]  = ((MAC_Config[lane_idx].Traffic_Type & 0x00000003) << 16) | ((MAC_Config[lane_idx].EtherType   <<  8) & 0x0000FF00) | ((MAC_Config[lane_idx].EtherType   >>  8) & 0x000000FF);
        Traffic_Cfg[(lane_idx*16)+ 6]  = ((MAC_Config[lane_idx].tx_polarity ? 0x1 : 0x0) << 22) | ((MAC_Config[lane_idx].GT_Tx_Post & 0x001F) << 17) | ((MAC_Config[lane_idx].GT_Tx_Main_Cursor & 0x007F) << 10) | ((MAC_Config[lane_idx].GT_Tx_Pre & 0x001F) << 5) | ((MAC_Config[lane_idx].GT_Tx_Diff & 0x001F) << 0);
        Traffic_Cfg[(lane_idx*16)+ 7]  = MAC_Config[lane_idx].GT_Rx_Eq_Sel ? 0x00000001 : 0x00000000;
        // XXV Ethernet configuration Registers
        Traffic_Cfg[(lane_idx*16)+ 8]  = MAC_Config[lane_idx].Lcl_Loopback ? 0x80000000 : 0x00000000;                     // 0x0008 : MODE_REG
        Traffic_Cfg[(lane_idx*16)+ 9]  = 0x00000003 | ((IPG_DEFAULT & 0x0000000F) << 10) | (MAC_Config[lane_idx].Set_LFI ? 0x00000008 : 0x00000000) | (MAC_Config[lane_idx].Set_RFI ? 0x00000010 : 0x00000000) | (MAC_Config[lane_idx].Set_Idle ? 0x00000020 : 0x00000000);       // TX REG1
        Traffic_Cfg[(lane_idx*16)+10]  = 0x00000033 ;                                                              // 0x0014 : RX REG1
        Traffic_Cfg[(lane_idx*16)+11]  = 0x00000040 | ((MAC_Config[lane_idx].MTU & 0x00007FFF) << 16);                    // 0x0018 : RX MTU
        Traffic_Cfg[(lane_idx*16)+12]  = 0x4FFF4FFF;                                                               // 0x001C : VL Length
        // common settings for all lanes
        Traffic_Cfg[(lane_idx*16)+13]  = (this->m_TC_Cfg.global_param.fec_mode == FEC_MODE_RS_FEC) ? 0x0000000D : 0x00000000;      // 0x00D0 : RSFEC REG   - RS-FEC Enabled
        Traffic_Cfg[(lane_idx*16)+14]  = (this->m_TC_Cfg.global_param.fec_mode == FEC_MODE_CLAUSE_74) ? 0x00000007 : 0x00000000;   // 0x00D4 : FEC REG     -- Clause 74 FEC Enable
        Traffic_Cfg[(lane_idx*16)+15]  = (this->m_TC_Cfg.global_param.lane_rate == LANE_RATE_10GBE) ? 0x00000001 : 0x00000000;     // 0x0138 : SWITCH SPEED REG (10G)
    }
    // Generate a sweep of all packets between 64 and 1536 bytes, based ONLY on global settings
    for (uint32_t n = 64; n <= 1518; n++)
    {
        Traffic_Cfg[448 + n - 64] = CalcScript(this->m_TC_Cfg.global_param.lane_rate, n, this->m_TC_Cfg.global_param.utilisation);
    }
    return config_falure;
}

bool GTMACTest::CheckMACStatus( std::map<uint, MAC_Config_t> & MAC_Config, std::map<uint, bool> & lane_failure )
{
    // global result of all lanes
    auto ret = RET_SUCCESS;
    // current result for a single lane
    bool lane_error;

    // latch the status
    SendGTMACCmd(CHECK_STATUS);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Read the Status from the PLRAM
    ReadGtmacCuTrafficCfg(&(this->m_MAC_Status[0]));
    GetTrafficCounters();

    MACStatus_t tx_MAC_Status;
    std::copy(this->m_MAC_Status, this->m_MAC_Status + GT_MAC_STATUS_SIZE, tx_MAC_Status);

    if (this->m_TC_Cfg.global_param.gt_mapping.enable)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Send MAC status to Mgmt"});
        GTMACMessage_t gtmac_msg;
        gtmac_msg.header = GTMACMSG_HEADER_MAC_STATUS;
        std::copy(this->m_MAC_Status, this->m_MAC_Status + GT_MAC_STATUS_SIZE, gtmac_msg.mac_status);
        this->m_gtmacmgmt->PushCommQueue(this->m_gt_index, gtmac_msg);

        // wait semaphore telling that stats from the paired GT are available in the comm queue
        this->m_gtmacmgmt->SemaphoreWait(this->m_gt_index);

        GTMACMessage_t mgmt_msg;
        if (this->m_gtmacmgmt->PopCommQueue(this->m_gt_index, mgmt_msg))
        {
            if (mgmt_msg.header == GTMACMSG_HEADER_MAC_STATUS)
            {
                LogMessage(MSG_DEBUG_TESTCASE, {"Received MAC status from the paired GTMAC"});
                std::copy(mgmt_msg.mac_status, mgmt_msg.mac_status + GT_MAC_STATUS_SIZE, tx_MAC_Status);
            }
            else
            {
                LogMessage(MSG_ETH_055);
                ret |= RET_FAILURE;
            }
        }
        else
        {
            LogMessage(MSG_ETH_055);
            ret |= RET_FAILURE;
        }
    }

    // And Parse each of lane the MAC results
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        lane_error = RET_SUCCESS;
        if (MAC_Config[lane_idx].Active)
        {
            LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_002);

            // Fail if we had bad counters, Only check Tx vs Rx if enabled for this MAC
            if ((ParseMACStatus(&(this->m_MAC_Status[64 + (lane_idx*96)]), lane_idx) == RET_FAILURE) ||
                (CheckTrafficCounters(&tx_MAC_Status[64 + (this->m_TC_Cfg.lane_param[lane_idx].tx_mapping*96)], lane_idx, MAC_Config[lane_idx].Match_Tx_Rx) == RET_FAILURE))
            {
                ret = RET_FAILURE;
                lane_error = RET_FAILURE;
            }

            if (lane_error == RET_SUCCESS)
            {
                LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_010);
            }
            else
            {
                LogMessageHeader2(GetHeader2(lane_idx), MSG_ETH_011);
            }
            // save the current lane result into the array of results
            lane_failure[lane_idx] |= lane_error;

            WriteOutputLine(lane_idx, lane_failure[lane_idx], lane_error, &(this->m_MAC_Status[64 + (lane_idx * 96)]));
        }
    }
    return ret;
}

int GTMACTest::RunTest()
{
    uint test_it_cnt     = 1;
    auto test_failure    = RET_SUCCESS;
    auto test_it_failure = RET_SUCCESS;

    uint32_t Traffic_Cfg[GT_MAC_BUF_SIZE];
    std::map<uint, MAC_Config_t> MAC_Config;
    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        MAC_Config[lane_idx] = RST_MAC_CONFIG;
    }

    this->m_run_duration = 0;

    if (!(this->m_abort))
    {
        LogMessage(MSG_DEBUG_TESTCASE, {"Number of test iterations: " + std::to_string(this->m_test_it_list.size())});
        this->m_testcase_queue_value.pending = this->m_test_it_list.size();
        PushTestcaseQueue();
    }

    GTMACMessage_t gtmac_msg;
    //gtmac_msg.header = GTMACMSG_HEADER_STRING;
    //gtmac_msg.msg = GTMACMSG_CFG_DONE;
    //this->m_gtmacmgmt->PushCommQueue(this->m_gt_index, gtmac_msg);

    gtmac_msg.header = GTMACMSG_HEADER_TEST_IT_CNT;
    gtmac_msg.test_it_cnt = test_it_cnt;
    this->m_gtmacmgmt->PushCommQueue(this->m_gt_index, gtmac_msg);
    this->m_gtmacmgmt->SemaphoreWait(this->m_gt_index);

    for (auto & test_it : this->m_test_it_list)
    {
        if (this->m_abort)
        {
            break;
        }

        this->m_testcase_queue_value.remaining_time = std::to_string(test_it.duration);
        this->m_testcase_queue_value.parameters = StrVectToTest(test_it.test_strvect);
        PushTestcaseQueue();

        test_it_failure = RET_SUCCESS;
        std::map<uint, bool> lane_failure;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            lane_failure[lane_idx] = RET_SUCCESS;
        }

        LogMessage(MSG_CMN_032, {std::to_string(test_it_cnt), StrVectToTest(test_it.test_strvect)}); // start test
        LogMessage(MSG_DEBUG_TESTCASE, {"\t- Extrapolated test sequence: " + StrVectToTest(test_it.test_strvect_dbg)});

        if ( (test_it.mac_cmd == CONFIG_10GBE) || (test_it.mac_cmd == CONFIG_10GBE_C74_FEC) || (test_it.mac_cmd == CONFIG_25GBE) || (test_it.mac_cmd == CONFIG_25GBE_C74_FEC) ||(test_it.mac_cmd == CONFIG_25GBE_RS_FEC))
        {
            // update the config done purely based on test json with the actual mode selected
            // allow live reconfiguratin
            if ( (test_it.mac_cmd == CONFIG_10GBE) || (test_it.mac_cmd == CONFIG_10GBE_C74_FEC) )
            {
                this->m_TC_Cfg.global_param.lane_rate = LANE_RATE_10GBE;

                if (test_it.mac_cmd == CONFIG_10GBE_C74_FEC)
                {
                   this->m_TC_Cfg.global_param.fec_mode = FEC_MODE_CLAUSE_74;
                }
                else
                {
                    this->m_TC_Cfg.global_param.fec_mode = FEC_MODE_NONE;
                }
            }
            else
            {
                this->m_TC_Cfg.global_param.lane_rate = LANE_RATE_25GBE;
                if (test_it.mac_cmd == CONFIG_25GBE_C74_FEC)
                {
                    this->m_TC_Cfg.global_param.fec_mode = FEC_MODE_CLAUSE_74;
                }
                else if (test_it.mac_cmd == CONFIG_25GBE_RS_FEC)
                {
                    this->m_TC_Cfg.global_param.fec_mode = FEC_MODE_RS_FEC;
                }
                else
                {
                    this->m_TC_Cfg.global_param.fec_mode = FEC_MODE_NONE;
                }
            }

            // create config and sent it to PLRAM
            test_it_failure |= CreateMACConfig(Traffic_Cfg, MAC_Config);
            LogMessage(MSG_ETH_001);
            WriteGtmacCuTrafficCfg(&Traffic_Cfg[0]);

            // send a confing command
            SendGTMACCmd(CONFIG_SEND);

            // send a reset command
            LogMessage(MSG_DEBUG_TESTCASE, {"Send GT reset"});
            SendGTMACCmd(TX_RX_RST);
            test_it_failure |= CheckLinkDown(true);
            test_it_failure |= CheckLinkSpeed(this->m_TC_Cfg.global_param.lane_rate);
        }
        else if ( (test_it.mac_cmd == CLEAR_STATUS) || (test_it.mac_cmd == CHECK_STATUS) )
        {
            // do nothing, wait first
        }
        else
        {
            // send any other MAC command
            SendGTMACCmd(test_it.mac_cmd);
            if (test_it.mac_cmd == CU_RUN)
            {
                this->m_run_duration = test_it.duration;
            }
        }

        uint duration_divider = test_it.duration/10;
        if (duration_divider == 0)
        {
            duration_divider = 1;
        }

        // Loop until done or abort
        for (uint i = test_it.duration; (i >= 1) && (!(this->m_abort)); i--)
        {
            if (((i % duration_divider == 0) || (i == test_it.duration)) && (!(this->m_abort)))
            {
                LogMessage(MSG_CMN_048, {std::to_string(i), ""});
            }
            WaitSecTick(1);

            this->m_testcase_queue_value.remaining_time = std::to_string(i);
            PushTestcaseQueue();
        }
        LogMessage(MSG_CMN_049); // test duration reached

        if (test_it.mac_cmd == CLEAR_STATUS)
        {
            ClearLatchStatus();
            SendGTMACCmd(CLEAR_STATUS);
            test_it_failure |= CheckLinkSpeed(this->m_TC_Cfg.global_param.lane_rate);
        }
        else if (test_it.mac_cmd == CHECK_STATUS)
        {
            test_it_failure |= CheckMACStatus(MAC_Config, lane_failure);
            test_it_failure |= CheckLinkDown(false);
            test_it_failure |= CheckLinkSpeed(this->m_TC_Cfg.global_param.lane_rate);
        }
        else if (test_it.mac_cmd == CU_RUN)
        {
            SendGTMACCmd(CU_STOP);
            WaitSecTick(2);
        }

        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if (this->m_TC_Cfg.lane_param[lane_idx].disable_lane)
            {
                this->m_test_it_lane_results[lane_idx][test_it_cnt-1] = TITR_DISABLED;
            }
            else if (lane_failure[lane_idx] == RET_SUCCESS)
            {
                this->m_test_it_lane_results[lane_idx][test_it_cnt-1] = TITR_PASSED;
            }
            else
            {
                this->m_test_it_lane_results[lane_idx][test_it_cnt-1] = TITR_FAILED;
            }
        }

        if (test_it_failure == RET_SUCCESS)
        {
            LogMessage(MSG_ETH_012);
        }
        else
        {
            LogMessage(MSG_ETH_013);
        }

        if (this->m_abort)
        {
            test_failure = RET_FAILURE;
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            // Set all remaining test result to aborted
            for (uint i = test_it_cnt - 1; i < this->m_test_sequence.size(); i++)
            {
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    this->m_test_it_lane_results[lane_idx][i] = TITR_ABORTED;
                }
                this->m_testcase_queue_value.completed++;
                this->m_testcase_queue_value.pending--;
                this->m_testcase_queue_value.failed++;
            }
        }
        else if (test_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_033_ERROR, {std::to_string(test_it_cnt)});
            test_failure = RET_FAILURE;
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.failed++;
        }
        else
        {
            LogMessage(MSG_CMN_033_PASS, {std::to_string(test_it_cnt)});
            this->m_testcase_queue_value.completed++;
            this->m_testcase_queue_value.pending--;
            this->m_testcase_queue_value.passed++;
        }
        if (this->m_testcase_queue_value.pending == 0)
        {
            this->m_testcase_queue_value.remaining_time    = NOT_APPLICABLE;
            this->m_testcase_queue_value.parameters        = NOT_APPLICABLE;
        }
        PushTestcaseQueue();
        test_it_cnt++;

        gtmac_msg.header = GTMACMSG_HEADER_TEST_IT_CNT;
        gtmac_msg.test_it_cnt = test_it_cnt;
        this->m_gtmacmgmt->PushCommQueue(this->m_gt_index, gtmac_msg);
        if (this->m_TC_Cfg.global_param.gt_mapping.enable)
        {
            this->m_gtmacmgmt->SemaphoreWait(this->m_gt_index);
        }
    }

    // Stop Traffic Running
    SendGTMACCmd(CU_STOP);

    // check for overall test failure
    if (this->m_abort)
    {
        return TEST_STATE_ABORT;
    }
    if (test_failure == RET_FAILURE)
    {
        return TEST_STATE_FAILURE;
    }
   return TEST_STATE_PASS;
}

bool GTMACTest::StartCU()
{
    return RET_SUCCESS;
}

bool GTMACTest::EnableWatchdogClkThrotDetection()
{
    return RET_SUCCESS;
}

bool GTMACTest::StopCU()
{
    return RET_SUCCESS;
}

bool GTMACTest::CheckWatchdogAndClkThrottlingAlarms()
{
    return RET_SUCCESS;
}

bool GTMACTest::GetCUConfig()
{
    auto ret = RET_SUCCESS;
    auto build_info = this->m_device->GetGtmacCuBI(this->m_cu_idx);

    switch (build_info.gt_rate)
    {
        case 0  :
                    this->m_CU_Rate = SWITCH_RATE_10_25GBE;
                    break;
        case 1  :
                    this->m_CU_Rate = FIX_RATE_10GBE;
                    break;
        case 2  :
                    this->m_CU_Rate = FIX_RATE_25GBE;
                    break;
        default :
                    LogMessage(MSG_ETH_050);
                    ret = RET_FAILURE;
                    break;
    }
    if (build_info.gt_rsfec_en == 1)
    {
        this->m_rsfec_available = true;
    }
    return ret;
}

bool GTMACTest::InitTestSequence()
{
    this->m_test_sequence = this->m_TC_Cfg.test_sequence;

    for (const auto & lane_idx : GT_LANE_INDEXES)
    {
        this->m_test_it_lane_results[lane_idx].clear();
        for (uint ii=0; ii<this->m_test_sequence.size(); ii++)
        {
            this->m_test_it_lane_results[lane_idx].emplace_back(TITR_NOT_TESTED);
        }
    }
    return RET_SUCCESS;
}

bool GTMACTest::ParseTestSequenceSettings( std::list<TestItConfig_t> & test_list )
{
    auto parse_failure = RET_SUCCESS;
    uint parse_error_cnt = 0;
    int  test_cnt = 0;
    auto lane_rate = LANE_RATE_25GBE; // use the most constraining rate

    LogMessage(MSG_CMN_022);
    std::vector<std::string> test_seq_strvect;
    std::vector<std::string> test_sequence_dbg;

    for (const auto & test_seq_param : this->m_test_sequence)
    {
        if (this->m_abort)
        {
            break;
        }

        auto parse_it_failure = GetCUConfig();
        TestItConfig_t test_it_cfg;

        test_cnt++;

        test_it_cfg.test_strvect = {}; // Displays what user entered
        test_it_cfg.test_strvect_dbg = {}; // Displays with extrapolated parameters

        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure |= CheckParam<uint>(DURATION_TEST_SEQ_MEMBER.name, test_seq_param.duration.value, MIN_DURATION, MAX_DURATION);
            test_it_cfg.duration = test_seq_param.duration.value;
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + DURATION_TEST_SEQ_MEMBER.name + "\": " + std::to_string(test_seq_param.duration.value));
        if (parse_it_failure == RET_SUCCESS)
        {
            parse_it_failure = CheckStringInSet(MODE_TEST_SEQ_MEMBER.name, test_seq_param.mode.value, SUPPORTED_GT_MAC_SEQUENCE_MODE);
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            // mode already checked in input parser
            test_it_cfg.mac_cmd_str = test_seq_param.mode.value;

            if      (StrMatchNoCase(test_seq_param.mode.value, CHECK_STATUS_STR))
            {
                test_it_cfg.mac_cmd = CHECK_STATUS;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CLEAR_STATUS_STR))
            {
                test_it_cfg.mac_cmd = CLEAR_STATUS;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_10GBE_NO_FEC_STR))
            {
                if (this->m_CU_Rate == FIX_RATE_25GBE)
                {
                    LogMessage(MSG_ETH_049, {CONF_10GBE_NO_FEC_STR, "25GbE"});
                    parse_it_failure |= RET_FAILURE;
                }
                else
                {
                    test_it_cfg.mac_cmd = CONFIG_10GBE;
                    lane_rate           = LANE_RATE_10GBE;
                }
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_10GBE_C74_FEC_STR))
            {
                if (this->m_CU_Rate == FIX_RATE_25GBE)
                {
                    LogMessage(MSG_ETH_049, {CONF_10GBE_C74_FEC_STR, "25GbE"});
                    parse_it_failure |= RET_FAILURE;
                }
                else
                {
                    test_it_cfg.mac_cmd = CONFIG_10GBE_C74_FEC;
                    lane_rate           = LANE_RATE_10GBE;
                }
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_25GBE_NO_FEC_STR))
            {
                if (this->m_CU_Rate == FIX_RATE_10GBE)
                {
                    LogMessage(MSG_ETH_049, {CONF_25GBE_NO_FEC_STR, "10GbE"});
                    parse_it_failure |= RET_FAILURE;
                }
                else
                {
                    test_it_cfg.mac_cmd = CONFIG_25GBE;
                    lane_rate           = LANE_RATE_25GBE;
                }
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_25GBE_C74_FEC_STR))
            {
                if (this->m_CU_Rate == FIX_RATE_10GBE)
                {
                    LogMessage(MSG_ETH_049, {CONF_25GBE_C74_FEC_STR, "10GbE"});
                    parse_it_failure |= RET_FAILURE;
                }
                else
                {
                    test_it_cfg.mac_cmd = CONFIG_25GBE_C74_FEC;
                    lane_rate           = LANE_RATE_25GBE;
                }
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, CONF_25GBE_RS_FEC_STR))
            {
                if (this->m_CU_Rate == FIX_RATE_10GBE)
                {
                    LogMessage(MSG_ETH_049, {CONF_25GBE_RS_FEC_STR, "10GbE"});
                    parse_it_failure |= RET_FAILURE;
                }
                else if (!(this->m_rsfec_available))
                {
                    LogMessage(MSG_ETH_056);
                    parse_it_failure |= RET_FAILURE;
                }
                else
                {
                    test_it_cfg.mac_cmd = CONFIG_25GBE_RS_FEC;
                    lane_rate           = LANE_RATE_25GBE;
                }
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, RUN_STR))
            {
                test_it_cfg.mac_cmd = CU_RUN;

                // 48bits MAC bytes counter will saturate if duration is too long, force user to split the test sequence
                if (lane_rate == LANE_RATE_10GBE)
                {
                    if (test_it_cfg.duration > MAX_DURATION_10GBE)
                    {
                        LogMessage(MSG_ETH_043, { std::to_string(test_it_cfg.duration), lane_rate, std::to_string(MAX_DURATION_10GBE) } );
                        parse_it_failure |= RET_FAILURE;
                    }
                }
                else
                {
                    if (test_it_cfg.duration > MAX_DURATION_25GBE)
                    {
                        LogMessage(MSG_ETH_043, { std::to_string(test_it_cfg.duration), lane_rate, std::to_string(MAX_DURATION_25GBE) } );
                        parse_it_failure |= RET_FAILURE;
                    }
                }

            }
            else if (StrMatchNoCase(test_seq_param.mode.value, TX_RST_STR))
            {
                test_it_cfg.mac_cmd = TX_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, RX_RST_STR))
            {
                test_it_cfg.mac_cmd = RX_RST;
            }
            else if (StrMatchNoCase(test_seq_param.mode.value, TX_RX_RST_STR))
            {
                test_it_cfg.mac_cmd = TX_RX_RST;
            }
        }
        test_it_cfg.test_strvect.emplace_back(    "\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");
        test_it_cfg.test_strvect_dbg.emplace_back("\"" + MODE_TEST_SEQ_MEMBER.name + "\": \"" + test_seq_param.mode.value + "\"");

        parse_failure |= parse_it_failure;

        if (parse_it_failure == RET_FAILURE)
        {
            LogMessage(MSG_CMN_025,{std::to_string(test_cnt)});

            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + " contains erroneous parameters");
            parse_error_cnt ++;
            if (parse_error_cnt > MAX_NUM_PARSER_ERROR)
            {
                break;
            }
        }
        else
        {
            // the test is valid; add it to the test_list
            test_list.emplace_back(test_it_cfg);
            test_sequence_dbg.emplace_back("\t- Test " + std::to_string(test_cnt) + ": " + StrVectToTest(test_it_cfg.test_strvect_dbg) + ". " + std::to_string(test_seq_param.num_param) + " parameters provided");
        }

        test_seq_strvect.emplace_back(StrVectToTest(test_it_cfg.test_strvect));
    }

    // display the entire test sequence
    LogMessage(MSG_DEBUG_TESTCASE, {"Extrapolated test sequence:"});
    for (const auto & test : test_sequence_dbg)
    {
        LogMessage(MSG_DEBUG_TESTCASE, {test});
    }

    if (parse_failure == RET_SUCCESS)
    {
        LogMessage(MSG_CMN_047, {std::to_string(test_seq_strvect.size()), "[ " + StrVectToStr(test_seq_strvect, ", ") + " ]"});
        LogMessage(MSG_CMN_023);
    }
    else
    {
        LogMessage(MSG_CMN_024);
    }

    return parse_failure;
}

bool GTMACTest::TrafficTypeParam2Setting ( const uint & lane_idx, const std::string & param, Traffic_Type_t & setting )
{
    if (StrMatchNoCase(param, TRAFFIC_TYPE_0X00))
    {
        setting = TT_ZERO;
    }
    else if (StrMatchNoCase(param, TRAFFIC_TYPE_0XFF))
    {
        setting = TT_FF;
    }
    else if (StrMatchNoCase(param, TRAFFIC_TYPE_COUNT))
    {
        setting = TT_CNT;
    }
    else if (StrMatchNoCase(param, TRAFFIC_TYPE_PATTERN))
    {
        setting = TT_4CYC;
    }
    else
    {
        auto param_name       = TRAFFIC_TYPE_MEMBER.name + "_" + std::to_string(lane_idx);
        auto supported_values = TRAFFIC_TYPE_0X00 + ", " + TRAFFIC_TYPE_0XFF + ", " + TRAFFIC_TYPE_COUNT + ", " + TRAFFIC_TYPE_PATTERN;
        LogMessage(MSG_CMN_001, {param, param_name, supported_values});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void GTMACTest::PrintLaneParam ( const LaneParam_t & lane_param )
{
    LogMessage(MSG_CMN_021, {"\t- " + DISABLE_LANE_MEMBER.name          + " : " +      BoolToStr(lane_param.disable_lane)});
    LogMessage(MSG_CMN_021, {"\t- " + UTILISATION_MEMBER.name           + " : " + std::to_string(lane_param.utilisation)});
    LogMessage(MSG_CMN_021, {"\t- " + TRAFFIC_TYPE_MEMBER.name          + " : " +                lane_param.traffic_type});
    LogMessage(MSG_CMN_021, {"\t- " + MAC_TO_MAC_CONNECTION_MEMBER.name + " : " + std::to_string(lane_param.mac_to_mac_connection)});

    auto tmp = lane_param.packet_cfg;
    if (!StrMatchNoCase(lane_param.packet_cfg, PACKET_CFG_SWEEP))
    {
        tmp = std::to_string(lane_param.packet_size);
    }
    LogMessage(MSG_CMN_021, {"\t- " + PACKET_CFG_MEMBER.name        + " : " + tmp});
    LogMessage(MSG_CMN_021, {"\t- " + MATCH_TX_RX_MEMBER.name       + " : " +      BoolToStr(lane_param.match_tx_rx)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_SETTINGS_MEMBER.name       + " : " +                lane_param.gt_settings});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_DIFFCTRL_MEMBER.name    + " : " + std::to_string(lane_param.gt_tx_diffctrl)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_MAIN_CURSOR_MEMBER.name + " : " + std::to_string(lane_param.gt_tx_main_cursor)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_PRE_EMPH_MEMBER.name    + " : " + std::to_string(lane_param.gt_tx_pre_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POST_EMPH_MEMBER.name   + " : " + std::to_string(lane_param.gt_tx_post_emph)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_RX_USE_LPM_MEMBER.name     + " : " +      BoolToStr(lane_param.gt_rx_use_lpm)});
    LogMessage(MSG_CMN_021, {"\t- " + GT_TX_POLARITY_MEMBER.name    + " : " +                lane_param.tx_polarity});
}

void GTMACTest::WriteOutputLine ( const uint & lane_idx, const bool & test_failure, const bool & test_it_failure, uint32_t * status )
{
    if (this->m_use_outputfile)
    {
        std::vector<std::string> str_vect;
        if (test_failure == RET_FAILURE)
        {
            str_vect.emplace_back("FAIL");
        }
        else
        {
            str_vect.emplace_back("PASS");
        }
        if (test_it_failure == RET_FAILURE)
        {
            str_vect.emplace_back("FAIL");
        }
        else
        {
            str_vect.emplace_back("PASS");
        }
        for (uint32_t n = 0; n < MAC_NUM_STATS; n++)
        {
            auto Stats_var = READ_BUFFER_64(status, n);
            str_vect.emplace_back(std::to_string( Stats_var ));
        }
        this->m_outputfile[lane_idx] << StrVectToStr(str_vect, ",") << "\n";
        this->m_outputfile[lane_idx].flush();

        (void)OpenRTOutputFile(this->m_RT_outputfile_name[lane_idx], this->m_RT_outputfile[lane_idx] );   // Don't check if open worked..
        this->m_RT_outputfile[lane_idx] << this->m_RT_outputfile_head[lane_idx] + "\n" + StrVectToStr(str_vect, ",");
        this->m_RT_outputfile[lane_idx].flush();
        this->m_RT_outputfile[lane_idx].close();
    }
}

std::string GTMACTest::GetFirstOutputLine ()
{
    if (this->m_use_outputfile)
    {
        std::vector<std::string> str_vect;
        str_vect.emplace_back("Overall result");
        str_vect.emplace_back("Test result");
        for (auto name : MAC_STAT_NAMES)
        {
            // Remove blank spaces after name
            // std::string name = MAC_STAT_NAMES[n];
            std::string name_trim;
            auto end_param = false;
            for (std::string::reverse_iterator rit = name.rbegin(); rit != name.rend(); ++rit)
            {
                if (*rit != ' ')
                {
                    end_param = true;
                }
                if (end_param)
                {
                    name_trim.insert(name_trim.begin(), *rit);
                }
            }
            str_vect.emplace_back(name_trim);
        }
        return StrVectToStr(str_vect, ",");
    }
    return "";
}

} // namespace

