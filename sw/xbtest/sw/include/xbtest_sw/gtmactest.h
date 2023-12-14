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

#ifndef _GTMACTEST_H
#define _GTMACTEST_H

#include <list>

#include "testinterface.h"
#include "gtmacmgmt.h"

namespace xbtest
{

#define MAC_NUM_STATS           (43)

class GTMACTest : public TestInterface
{

public:
    GTMACTest(
        Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, GTMACMgmt * gtmacmgmt, DeviceMgt * device_mgt,
        const Gt_Parameters_t & test_parameters, const uint & cu_idx
    );
    ~GTMACTest();

    bool                                            PreSetup() override;
    void                                            Run() override;
    void                                            PostTeardown() override;
    void                                            Abort() override;
    std::vector<GTMAC_Test_Sequence_Parameters_t>   GetTestSequence();

private:

    const std::string REP = "%";

    using Supported_Rate_t = enum Supported_Rate_t {
        SWITCH_RATE_10_25GBE,
        FIX_RATE_10GBE,
        FIX_RATE_25GBE
    };

    using Rate_t = struct Rate_t {
        uint32_t    packet_size;
        float       byte_rate;
        float       packet_rate;
    };

    // from measurements, need to be sorted by packet_size order
    const std::vector<Rate_t> BYTE_RATE_25GBE    = {{   64, 2220216818.0, 34690888.0},
                                                    {  128, 2704184653.0, 21126443.0},
                                                    {  256, 2900140178.0, 11328673.0},
                                                    {  512, 3009168226.0,  5877282.0},
                                                    { 1024, 3067183036.0,  2995296.0},
                                                    { 1535, 3082871352.0,  2210894.0},
                                                    { 9000, 3120527858.0,   328477.0},
                                                    {10000, 3120862552.0,   311743.0}};

    const std::vector<Rate_t> BYTE_RATE_10GBE    = {{   64,  952599220.0, 14884363.0},
                                                    {  128, 1081328858.0,  8447882.0},
                                                    {  256, 1159686071.0,  4530024.0},
                                                    {  512, 1203283371.0,  2350163.0},
                                                    { 1024, 1226327518.0,  1197585.0},
                                                    { 1535, 1234198586.0,   804038.0},
                                                    { 9000, 1247655900.0,   131332.0},
                                                    {10000, 1247789394.0,   124642.0}};

    const std::vector<std::string> MAC_STAT_NAMES =  {
        "CYCLE_COUNT                ",  //  0
        "RX_FRAMING_ERR             ",  //  1
        "RX_BAD_CODE                ",  //  2
        "RX_RSFEC_CORRECTED_CW_INC  ",  //  3
        "RX_RSFEC_UNCORRECTED_CW_INC",  //  4
        "RX_RSFEC_ERR_COUNT0_INC    ",  //  5
        "RX_TOTAL_PACKETS           ",  //  6
        "RX_TOTAL_GOOD_PACKETS      ",  //  7
        "RX_TOTAL_BYTES             ",  //  8
        "RX_TOTAL_GOOD_BYTES        ",  //  9
        "RX_PACKET_64_BYTES         ",  // 10
        "RX_PACKET_65_127_BYTES     ",  // 11
        "RX_PACKET_128_255_BYTES    ",  // 12
        "RX_PACKET_256_511          ",  // 13
        "RX_PACKET_512_1023_BYTES   ",  // 14
        "RX_PACKET_1024_1518_BYTES  ",  // 15
        "RX_PACKET_1519_1522_BYTES  ",  // 16
        "RX_PACKET_1523_1548_BYTES  ",  // 17
        "RX_PACKET_1549_2047_BYTES  ",  // 18
        "RX_PACKET_2048_4095_BYTES  ",  // 19
        "RX_PACKET_4096_8191_BYTES  ",  // 20
        "RX_PACKET_8192_9215_BYTES  ",  // 21
        "RX_PACKET_LARGE            ",  // 22
        "RX_PACKET_SMALL            ",  // 23
        "RX_UNDERSIZE               ",  // 24
        "RX_FRAGMENT                ",  // 25
        "RX_OVERSIZE                ",  // 26
        "RX_TOOLONG                 ",  // 27
        "RX_JABBER                  ",  // 28
        "RX_BAD_FCS                 ",  // 29
        "RX_PACKET_BAD_FCS          ",  // 30
        "RX_STOMPED_FCS             ",  // 31
        "RX_UNICAST                 ",  // 32
        "RX_MULTICAST               ",  // 33
        "RX_BROADCAST               ",  // 34
        "RX_VLAN                    ",  // 35
        "RX_INRANGEERR              ",  // 36
        "RX_TRUNCATED               ",  // 37
        "RX_TEST_PATTERN_MISMATCH   ",  // 38
        "FEC_INC_CORRECT_COUNT      ",  // 39
        "FEC_INC_CANT_CORRECT_COUNT ",  // 40
        "TX_TOTAL_PACKETS           ",  // 41
        "TX_TOTAL_BYTES             "   // 42
    };
    const std::vector<bool> MAC_STAT_ERR_TRIG =  {
        FALSE,  // "CYCLE_COUNT                ",
        TRUE,   // "RX_FRAMING_ERR             ",
        TRUE,   // "RX_BAD_CODE                ",
        TRUE,   // "RX_RSFEC_CORRECTED_CW_INC  ",
        TRUE,   // "RX_RSFEC_UNCORRECTED_CW_INC",
        TRUE,   // "RX_RSFEC_ERR_COUNT0_INC    ",
        FALSE,  // "RX_TOTAL_PACKETS           ",
        FALSE,  // "RX_TOTAL_GOOD_PACKETS      ",
        FALSE,  // "RX_TOTAL_BYTES             ",
        FALSE,  // "RX_TOTAL_GOOD_BYTES        ",
        FALSE,  // "RX_PACKET_64_BYTES         ",
        FALSE,  // "RX_PACKET_65_127_BYTES     ",
        FALSE,  // "RX_PACKET_128_255_BYTES    ",
        FALSE,  // "RX_PACKET_256_511          ",
        FALSE,  // "RX_PACKET_512_1023_BYTES   ",
        FALSE,  // "RX_PACKET_1024_1518_BYTES  ",
        FALSE,  // "RX_PACKET_1519_1522_BYTES  ",
        FALSE,  // "RX_PACKET_1523_1548_BYTES  ",
        FALSE,  // "RX_PACKET_1549_2047_BYTES  ",
        FALSE,  // "RX_PACKET_2048_4095_BYTES  ",
        FALSE,  // "RX_PACKET_4096_8191_BYTES  ",
        FALSE,  // "RX_PACKET_8192_9215_BYTES  ",
        FALSE,  // "RX_PACKET_LARGE            ",
        TRUE,   // "RX_PACKET_SMALL            ",
        TRUE,   // "RX_UNDERSIZE               ",
        TRUE,   // "RX_FRAGMENT                ",
        FALSE,  // "RX_OVERSIZE                ",
        FALSE,  // "RX_TOOLONG                 ",
        TRUE,   // "RX_JABBER                  ",
        TRUE,   // "RX_BAD_FCS                 ",
        TRUE,   // "RX_PACKET_BAD_FCS          ",
        TRUE,   // "RX_STOMPED_FCS             ",
        FALSE,  // "RX_UNICAST                 ",
        FALSE,  // "RX_MULTICAST               ",
        FALSE,  // "RX_BROADCAST               ",
        FALSE,  // "RX_VLAN                    ",
        TRUE,   // "RX_INRANGEERR              ",
        FALSE,  // "RX_TRUNCATED               ",
        TRUE,   // "RX_TEST_PATTERN_MISMATCH   ",
        FALSE,  // "FEC_INC_CORRECT_COUNT      ",
        TRUE,   // "FEC_INC_CANT_CORRECT_COUNT ",
        FALSE,  // "TX_TOTAL_PACKETS           ",
        FALSE   // "TX_TOTAL_BYTES             "
    };
   const std::vector<bool> MAC_STAT_WARNING_TRIG =  {
        FALSE,  // "CYCLE_COUNT                ",
        FALSE,  // "RX_FRAMING_ERR             ",
        FALSE,  // "RX_BAD_CODE                ",
        FALSE,  // "RX_RSFEC_CORRECTED_CW_INC  ",
        FALSE,  // "RX_RSFEC_UNCORRECTED_CW_INC",
        FALSE,  // "RX_RSFEC_ERR_COUNT0_INC    ",
        FALSE,  // "RX_TOTAL_PACKETS           ",
        FALSE,  // "RX_TOTAL_GOOD_PACKETS      ",
        FALSE,  // "RX_TOTAL_BYTES             ",
        FALSE,  // "RX_TOTAL_GOOD_BYTES        ",
        FALSE,  // "RX_PACKET_64_BYTES         ",
        FALSE,  // "RX_PACKET_65_127_BYTES     ",
        FALSE,  // "RX_PACKET_128_255_BYTES    ",
        FALSE,  // "RX_PACKET_256_511          ",
        FALSE,  // "RX_PACKET_512_1023_BYTES   ",
        FALSE,  // "RX_PACKET_1024_1518_BYTES  ",
        FALSE,  // "RX_PACKET_1519_1522_BYTES  ",
        FALSE,  // "RX_PACKET_1523_1548_BYTES  ",
        FALSE,  // "RX_PACKET_1549_2047_BYTES  ",
        FALSE,  // "RX_PACKET_2048_4095_BYTES  ",
        FALSE,  // "RX_PACKET_4096_8191_BYTES  ",
        FALSE,  // "RX_PACKET_8192_9215_BYTES  ",
        FALSE,  // "RX_PACKET_LARGE            ",
        FALSE,  // "RX_PACKET_SMALL            ",
        FALSE,  // "RX_UNDERSIZE               ",
        FALSE,  // "RX_FRAGMENT                ",
        TRUE ,  // "RX_OVERSIZE                ",
        TRUE ,  // "RX_TOOLONG                 ",
        FALSE,  // "RX_JABBER                  ",
        FALSE,  // "RX_BAD_FCS                 ",
        FALSE,  // "RX_PACKET_BAD_FCS          ",
        FALSE,  // "RX_STOMPED_FCS             ",
        FALSE,  // "RX_UNICAST                 ",
        FALSE,  // "RX_MULTICAST               ",
        FALSE,  // "RX_BROADCAST               ",
        FALSE,  // "RX_VLAN                    ",
        FALSE,  // "RX_INRANGEERR              ",
        TRUE ,  // "RX_TRUNCATED               ",
        FALSE,  // "RX_TEST_PATTERN_MISMATCH   ",
        TRUE,   // "FEC_INC_CORRECT_COUNT      ",
        FALSE,  // "FEC_INC_CANT_CORRECT_COUNT ",
        FALSE,  // "TX_TOTAL_PACKETS           ",
        FALSE   // "TX_TOTAL_BYTES             "
    };

    const uint MAC_IDX_RX_GOOD_PAC           = 7;
    const uint MAC_IDX_TX_SENT_PAC           = 41;
    const uint MAC_IDX_TX_SENT_BYTE          = 42;

    const uint STATUS_USR_TX_RST_0           = (0x1 << 0);
    const uint STATUS_USR_TX_RST_1           = (0x1 << 1);
    const uint STATUS_USR_TX_RST_2           = (0x1 << 2);
    const uint STATUS_USR_TX_RST_3           = (0x1 << 3);
    const uint STATUS_USR_TX_RST_0_3         = (0xF << 0);
    const uint STATUS_USR_RX_RST_0           = (0x1 << 4);
    const uint STATUS_USR_RX_RST_1           = (0x1 << 5);
    const uint STATUS_USR_RX_RST_2           = (0x1 << 6);
    const uint STATUS_USR_RX_RST_3           = (0x1 << 7);
    const uint STATUS_USR_RX_RST_0_3         = (0xF << 4);
    const uint STATUS_RX_LINK_0              = (0x1 << 8);
    const uint STATUS_RX_LINK_1              = (0x1 << 9);
    const uint STATUS_RX_LINK_2              = (0x1 << 10);
    const uint STATUS_RX_LINK_3              = (0x1 << 11);
    const uint STATUS_RX_LINK_0_3            = (0xF << 8);

    const uint STATUS_LANE_RATE_10_GBE       = (0xF << 12);
    const uint STATUS_LANE_RATE_25_GBE       = (0x0 << 12);
    const uint STATUS_LANE_RATE_0_3          = (0xF << 12);

    const uint TC_GOOD_PACKET_0_LSB_ADDR     = 0x0040;
    const uint TC_GOOD_PACKET_1_LSB_ADDR     = 0x0041;
    const uint TC_GOOD_PACKET_2_LSB_ADDR     = 0x0042;
    const uint TC_GOOD_PACKET_3_LSB_ADDR     = 0x0043;
    const uint TC_GOOD_PACKET_0_MSB_ADDR     = 0x0044;
    const uint TC_GOOD_PACKET_1_MSB_ADDR     = 0x0045;
    const uint TC_GOOD_PACKET_2_MSB_ADDR     = 0x0046;
    const uint TC_GOOD_PACKET_3_MSB_ADDR     = 0x0047;

    const uint TC_GOOD_BYTE_0_LSB_ADDR       = 0x0048;
    const uint TC_GOOD_BYTE_1_LSB_ADDR       = 0x0049;
    const uint TC_GOOD_BYTE_2_LSB_ADDR       = 0x004A;
    const uint TC_GOOD_BYTE_3_LSB_ADDR       = 0x004B;
    const uint TC_GOOD_BYTE_0_MSB_ADDR       = 0x004C;
    const uint TC_GOOD_BYTE_1_MSB_ADDR       = 0x004D;
    const uint TC_GOOD_BYTE_2_MSB_ADDR       = 0x004E;
    const uint TC_GOOD_BYTE_3_MSB_ADDR       = 0x004F;

    const uint TC_TOTAL_PACKET_0_LSB_ADDR    = 0x0050;
    const uint TC_TOTAL_PACKET_1_LSB_ADDR    = 0x0051;
    const uint TC_TOTAL_PACKET_2_LSB_ADDR    = 0x0052;
    const uint TC_TOTAL_PACKET_3_LSB_ADDR    = 0x0053;
    const uint TC_TOTAL_PACKET_0_MSB_ADDR    = 0x0054;
    const uint TC_TOTAL_PACKET_1_MSB_ADDR    = 0x0055;
    const uint TC_TOTAL_PACKET_2_MSB_ADDR    = 0x0056;
    const uint TC_TOTAL_PACKET_3_MSB_ADDR    = 0x0057;

    const uint TC_BAD_DEST_ADDR_0_LSB_ADDR   = 0x0058;
    const uint TC_BAD_DEST_ADDR_1_LSB_ADDR   = 0x0059;
    const uint TC_BAD_DEST_ADDR_2_LSB_ADDR   = 0x005A;
    const uint TC_BAD_DEST_ADDR_3_LSB_ADDR   = 0x005B;
    const uint TC_BAD_DEST_ADDR_0_MSB_ADDR   = 0x005C;
    const uint TC_BAD_DEST_ADDR_1_MSB_ADDR   = 0x005D;
    const uint TC_BAD_DEST_ADDR_2_MSB_ADDR   = 0x005E;
    const uint TC_BAD_DEST_ADDR_3_MSB_ADDR   = 0x005F;

    const uint LAST_BAD_DEST_ADDR_0_LSB_ADDR = 0x0060;
    const uint LAST_BAD_DEST_ADDR_1_LSB_ADDR = 0x0061;
    const uint LAST_BAD_DEST_ADDR_2_LSB_ADDR = 0x0062;
    const uint LAST_BAD_DEST_ADDR_3_LSB_ADDR = 0x0063;
    const uint LAST_BAD_DEST_ADDR_0_MSB_ADDR = 0x0064;
    const uint LAST_BAD_DEST_ADDR_1_MSB_ADDR = 0x0065;
    const uint LAST_BAD_DEST_ADDR_2_MSB_ADDR = 0x0066;
    const uint LAST_BAD_DEST_ADDR_3_MSB_ADDR = 0x0067;

    const uint TC_BAD_SRC_ADDR_0_LSB_ADDR    = 0x0068;
    const uint TC_BAD_SRC_ADDR_1_LSB_ADDR    = 0x0069;
    const uint TC_BAD_SRC_ADDR_2_LSB_ADDR    = 0x006A;
    const uint TC_BAD_SRC_ADDR_3_LSB_ADDR    = 0x006B;
    const uint TC_BAD_SRC_ADDR_0_MSB_ADDR    = 0x006C;
    const uint TC_BAD_SRC_ADDR_1_MSB_ADDR    = 0x006D;
    const uint TC_BAD_SRC_ADDR_2_MSB_ADDR    = 0x006E;
    const uint TC_BAD_SRC_ADDR_3_MSB_ADDR    = 0x006F;

    const uint LAST_BAD_SRC_ADDR_0_LSB_ADDR  = 0x0070;
    const uint LAST_BAD_SRC_ADDR_1_LSB_ADDR  = 0x0071;
    const uint LAST_BAD_SRC_ADDR_2_LSB_ADDR  = 0x0072;
    const uint LAST_BAD_SRC_ADDR_3_LSB_ADDR  = 0x0073;
    const uint LAST_BAD_SRC_ADDR_0_MSB_ADDR  = 0x0074;
    const uint LAST_BAD_SRC_ADDR_1_MSB_ADDR  = 0x0075;
    const uint LAST_BAD_SRC_ADDR_2_MSB_ADDR  = 0x0076;
    const uint LAST_BAD_SRC_ADDR_3_MSB_ADDR  = 0x0077;

    const uint FREQ_CNT_0_LSB_ADDR           = 0x0078;
    const uint FREQ_CNT_1_LSB_ADDR           = 0x0079;
    const uint FREQ_CNT_2_LSB_ADDR           = 0x007A;
    const uint FREQ_CNT_3_LSB_ADDR           = 0x007B;
    const uint FREQ_CNT_0_MSB_ADDR           = 0x007C;
    const uint FREQ_CNT_1_MSB_ADDR           = 0x007D;
    const uint FREQ_CNT_2_MSB_ADDR           = 0x007E;
    const uint FREQ_CNT_3_MSB_ADDR           = 0x007F;

    const uint32_t IPG_DEFAULT               = 12;

    // Used to overwrite severity for some messages
    Message_t MSG_ETH_004_ERROR  = MSG_ETH_004; // Error by default in message definition
    Message_t MSG_ETH_004_WARN   = MSG_ETH_004;

    GTMACMgmt *                                     m_gtmacmgmt = nullptr;
    DeviceMgt *                                     m_devicemgt = nullptr;
    Gt_Parameters_t                                 m_test_parameters;
    uint                                            m_cu_idx;
    uint                                            m_gt_index;
    std::string                                     m_outputfile_name;
    bool                                            m_use_outputfile = false;
    std::map<uint, std::ofstream>                   m_outputfile;
    std::map<uint, std::ofstream>                   m_RT_outputfile;
    std::map<uint, std::string   >                  m_RT_outputfile_name;
    std::map<uint, std::string   >                  m_RT_outputfile_head;
    std::vector<GTMAC_Test_Sequence_Parameters_t>   m_test_sequence;
    std::list<TestItConfig_t>                       m_test_it_list;
    GTMACTestcaseCfg_t                              m_TC_Cfg;
    MACStatus_t                                     m_MAC_Status;
    MACTrafficCnt_t                                 m_traffic_counters;
    std::vector<MAC_Addr_t>                         m_mac_addr;
    uint                                            m_run_duration = 0;
    Supported_Rate_t                                m_CU_Rate;
    bool                                            m_rsfec_available = false;

    bool        WriteGtmacCu                ( const uint & address, const uint & value);
    bool        ReadGtmacCu                 ( const uint & address, uint & read_data );
    bool        ReadGtmacCu64b              ( const uint & addr_lsb, const uint & addr_msb, uint64_t & read_data_64 );
    bool        WriteGtmacCuCmd             ( const uint & value );
    bool        WriteGtmacCuTrafficCfg      ( uint32_t * traffic_cfg );
    bool        ReadGtmacCuTrafficCfg       ( uint32_t * status );
    uint32_t    CalcScript                  ( const std::string & rate, const uint32_t & packet_size, const float & utilisation );
    bool        ParseMACStatus              ( uint32_t * status, const uint & lane_idx);
    bool        CheckTrafficCounters        ( uint32_t * status_tx, const uint & lane_idx, const bool & Check_Tx_Rx );
    void        SendGTMACCmd                ( const MACCmd_t & cmd );
    bool        CheckLinkDown               ( const bool & expected_link_down );
    bool        CheckLinkSpeed              ( const std::string & lane_rate );
    void        ClearLatchStatus();
    void        GetTrafficCounters();
    Rate_t      GetExpectedRates            ( const std::string & rate, const uint32_t & packet_size, const float & utilisation );
    bool        CheckMACBytesPacketsCounters( const uint & lane_idx, const uint64_t & mac_tx_byte_sent, const uint64_t & mac_tx_packet_sent );
    bool        CreateMACConfig             ( uint32_t Traffic_Cfg[], std::map<uint, MAC_Config_t> & MAC_Config );
    bool        CheckMACStatus              ( std::map<uint, MAC_Config_t> & MAC_Config, std::map<uint, bool> & lane_failure );
    int         RunTest() override;
    bool        StartCU() override;
    bool        EnableWatchdogClkThrotDetection() override;
    bool        StopCU() override;
    bool        CheckWatchdogAndClkThrottlingAlarms() override;
    bool        GetCUConfig();
    bool        InitTestSequence();
    bool        ParseTestSequenceSettings   ( std::list<TestItConfig_t> & test_list );
    bool        TrafficTypeParam2Setting    ( const uint & lane_idx, const std::string & param, Traffic_Type_t & setting );
    void        PrintLaneParam              ( const LaneParam_t & lane_param );
    void        WriteOutputLine             ( const uint & lane_idx, const bool & test_failure, const bool & test_it_failure, uint32_t *status );
    std::string GetFirstOutputLine          ();

    std::string MacAddrToStr(uint64_t mac_addr);

};

} // namespace

#endif /* _GTMACTEST_H */
