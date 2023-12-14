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

#ifndef _MEMORYTEST_H
#define _MEMORYTEST_H

#include <list>

#include "testinterface.h"
#include "devicemgt.h"

namespace xbtest
{

class MemoryTest : public TestInterface
{

public:
    MemoryTest(
        Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
        const Memory_Type_Parameters_t & test_parameters, const bool & power_tc_exists, const uint & memory_idx, const uint & cu_idx
    );
    ~MemoryTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

private:

    using Mem_Test_Mode_t = enum Mem_Test_Mode_t {
        ALTERNATE_WR_RD_MODE_VAL     = 0,
        ONLY_WR_MODE_VAL             = 1,
        ONLY_RD_MODE_VAL             = 2,
        SIMULTANEOUS_WR_RD_MODE_VAL  = 3,
        UNKNOWN_MODE_VAL             = 4
    };

    using MemoryCoreCfg_t = struct MemoryCoreCfg_t {
        bool                        disable;
        Mem_Thresh_CU_t             cu_bw;
        Mem_Thresh_CU_t             cu_latency;
        bool                        disable_prewrite;
        bool                        check_bw_exists;
        bool                        check_bw;
        bool                        check_latency_exists;
        bool                        check_latency;
        bool                        check_data_integrity_exists;
        bool                        check_data_integrity;
        uint64_t                    mem_size_bytes;
        bool                        single_axi_thread;
        std::string                 pattern_ctrl;
        std::vector<std::string>    pattern_list_str;
        std::vector<uint>           pattern_list;
    };

    using MemoryTestcaseCfg_t = struct MemoryTestcaseCfg_t {
        MemoryCoreCfg_t                     chan_config_global;
        std::map<uint, MemoryCoreCfg_t>     chan_config;
    };

    using Test_Cfg_Wr_Rd_t = struct Test_Cfg_Wr_Rd_t {
        double rate;        // via test JSON
        uint bandwidth;     // via test JSON
        uint start_addr;    // via test JSON
        uint end_addr;      // end address base on the block size
        uint burst_size;    // via test JSON
        uint block_size;    // via test JSON
        uint num_burst;     // number of bursts based on burst and block size
        uint burst_xfer;    // number of transfer per burst based on burst size and AXI data size
        uint total_xfer;    // total number of transfer per cycle based on block size and AXI data size
        uint outstanding;   // via test JSON
    };

    using TestItConfig_t = struct TestItConfig_t {
        uint                                mode;
        uint                                duration;
        Test_Cfg_Wr_Rd_t                    write;
        Test_Cfg_Wr_Rd_t                    read;
        std::map<uint, Mem_Thresh_WrRd_t>   bw_thresh;
        std::map<uint, Mem_Thresh_WrRd_t>   lat_thresh;
        uint64_t                            cfg_update_time_us;
        bool                                is_write;
        bool                                is_read;
        std::vector<std::string>            test_strvect;
        std::vector<std::string>            test_strvect_dbg;
    };

    using Stat_Burst_t = struct Stat_Burst_t {
        // register values
        uint        reg_inst;               // number of measures
        uint64_t    reg_total;              // HW accumulator of measures in clk cycle
        double      live_inst;              // Number of measures in double
        double      live_total_ns;          // HW accumulator of measures in ns
        double      live_average_ns;        // total/inst in ns
        double      live_average_mbps;      // for debug, applicable for burst time, equivalent to burst time (live_average_ns) in MBps
        double      live_bw_mbps;           // Applicable for burst time, live bandwidth in MBps based on inst
        // Accumulated each sec over test
        double      average_inst;
        double      average_ns;
        double      average_mbps;     // for debug,  Applicable for burst time, equivalent to burst time (average_ns) in MBps
        double      average_bw_mbps;  // Applicable for burst time, average bandwidth in MBps based on average_inst
        double      acc_inst;
        double      acc_ns;
    };
    const Stat_Burst_t RESET_STAT_BURST = {
        .reg_inst           = 0,
        .reg_total          = 0,
        .live_inst          = 0.0,
        .live_total_ns      = 0.0,
        .live_average_ns    = 0.0,
        .live_average_mbps  = 0.0,
        .live_bw_mbps       = 0.0,
        .average_inst       = 0.0,
        .average_ns         = 0.0,
        .average_mbps       = 0.0,
        .average_bw_mbps    = 0.0,
        .acc_inst           = 0.0,
        .acc_ns             = 0.0
    };

    using Chan_Status_t = struct Chan_Status_t {
        uint                configuration;
        uint                term_err_cnt;
        uint64_t            axi_addr_ptr;
        Stat_Burst_t        wr_burst_latency;
        Stat_Burst_t        rd_burst_latency;
        uint                timestamp;
        uint                cfg_updated;
        uint                term_err;
        uint                gen_seed_err;
        uint                term_seed_err;
        uint                toggle_1_sec;
        uint                timestamp_1_sec;
    }; // register values
    const Chan_Status_t RESET_CHAN_STATUS = {
        .configuration      = 0,
        .term_err_cnt       = 0,
        .axi_addr_ptr       = 0,
        .wr_burst_latency   = RESET_STAT_BURST,
        .rd_burst_latency   = RESET_STAT_BURST,
        .timestamp          = 0,
        .cfg_updated        = 0,
        .term_err           = 0,
        .gen_seed_err       = 0,
        .term_seed_err      = 0,
        .toggle_1_sec       = 0,
        .timestamp_1_sec    = 0
    };

    using Data_Integrity_t = struct Data_Integrity_t {
        std::string live;
        std::string result;
    }; // register values
    const Data_Integrity_t RESET_DATA_INTEGRITY = {
        .live   = DATA_INTEG_OK,
        .result = DATA_INTEG_OK,
    };

    const uint       REG_CTRL_CONFIGURATION              = 0x100000;
    const uint       REG_CTRL_WR_START_ADDR_LSB          = 0x100001;
    const uint       REG_CTRL_WR_START_ADDR_MSB          = 0x100002;
    const uint       REG_CTRL_RD_START_ADDR_LSB          = 0x100003;
    const uint       REG_CTRL_RD_START_ADDR_MSB          = 0x100004;
    const uint       REG_CTRL_WR_BURST_SIZE              = 0x100005;
    const uint       REG_CTRL_RD_BURST_SIZE              = 0x100006;
    const uint       REG_CTRL_WR_NUM_XFER                = 0x100007;
    const uint       REG_CTRL_RD_NUM_XFER                = 0x100008;
    const uint       REG_CTRL_WR_MEM_RATE                = 0x100009;
    const uint       REG_CTRL_RD_MEM_RATE                = 0x10000A;
    const uint       REG_CTRL_OUTSTANDING                = 0x10000B;
    const uint       REG_CTRL_PATTERN_A                  = 0x10000C;
    const uint       REG_CTRL_PATTERN_B                  = 0x10000D;
    const uint       REG_STAT_CONFIGURATION              = 0x180000;
    // const uint       REG_STAT_ERROR_COUNT                = 0x180001;
    const uint       REG_STAT_AXI_ADDR_PTR_LSB           = 0x180002;
    const uint       REG_STAT_AXI_ADDR_PTR_MSB           = 0x180003;
    // const uint       REG_STAT_WR_BURST_LATENCY_TOTAL_LSB = 0x180004;
    // const uint       REG_STAT_WR_BURST_LATENCY_TOTAL_MSB = 0x180005;
    // const uint       REG_STAT_WR_BURST_LATENCY_INST      = 0x180006;
    // const uint       REG_STAT_RD_BURST_LATENCY_TOTAL_LSB = 0x180007;
    // const uint       REG_STAT_RD_BURST_LATENCY_TOTAL_MSB = 0x180009;
    // const uint       REG_STAT_RD_BURST_LATENCY_INST      = 0x180009;
    // const uint       REG_STAT_TIMESTAMP                  = 0x18000A;
    const uint       MEM_CTRL_STOP                       = (0x1 << 0);
    const uint       MEM_CTRL_UPDATE_CFG                 = (0x1 << 1);
    const uint       MEM_CTRL_RESET                      = (0x1 << 8);
    const uint       MEM_TEST_MODE_MASK                  = 0x00000030;
    const uint       MEM_CTRL_CLEAR_ERR                  = (0x1 << 12);
    const uint       WR_MEM_RATE_EN                      = (0x1 << 16);
    const uint       RD_MEM_RATE_EN                      = (0x1 << 17);
    const uint       OUTSTANDING_WR_EN                   = (0x1 << 18);
    const uint       OUTSTANDING_RD_EN                   = (0x1 << 19);
    const uint       MULTI_THREAD_EN                     = (0x1 << 20);
    const uint       PATTERN_EN                          = (0x1 << 21);
    const uint       OUTSTANDING_WR_MASK                 = 0x01FF0000;
    const uint       OUTSTANDING_RD_MASK                 = 0x000001FF;
    const double            BPNS_TO_MBPS                        = (double)(1000) * (double)(1000) * (double)(1000) / (double)(1024) / (double)(1024);
    const uint       MIN_RESULTS_CHECK_DURATION          = 20;
    const uint       MIN_START_ADDR                      = 0;
    const uint       MIN_BURST_XFER                      = 2;  //  2 * 512 bits = 128 Bytes
    const uint       MAX_BURST_XFER_512b                 = 64; // 64 * 512 bits = 4 KBytes
    const uint       MAX_BURST_XFER_BELOW_512b           = 128;
    const uint       MIN_BLOCK_SIZE                      = 1; // 1 MB
    const double     MIN_MEM_RATE                        = 1.0;
    const double     MAX_MEM_RATE                        = 100.0;
    const uint       MIN_MEM_BANDWIDTH                   = 1;
    const uint       MAX_MEM_BANDWIDTH                   = MAX_UINT_VAL;
    const uint64_t   HW_SEC_CNT_TOLERANCE                = 10; // 10 % with min - 1 sec
    const uint       MIN_LO_THRESH                       = 1;
    const uint       MIN_OUTSTANDING                     = 0; // 0 means not limited
    const uint       MAX_OUTSTANDING                     = 255;

    DeviceMgt                * m_devicemgt = nullptr;
    Memory_Type_Parameters_t   m_test_parameters;
    bool                       m_power_tc_exists;
    uint                       m_memory_idx;
    uint                       m_cu_idx;

    Xbtest_Memory_t m_memory_settings;
    std::map<uint, Json_Parameters_t> m_mem_chan_config;
    std::map<uint, Json_Parameters_t> m_mem_chan_config_pattern;

    std::vector<Memory_Test_Sequence_Parameters_t> m_global_test_sequence;

    int                                         m_num_channels;
    uint64_t                                    m_buffer_size;
    std::string                                 m_cu_name;
    std::vector<uint>                           m_enabled_ch_indexes;
    std::string                                 m_outputfile_name;
    bool                                        m_use_outputfile = false;
    std::map<uint, std::ofstream>               m_outputfile_detail;
    std::map<uint, std::ofstream>               m_RT_outputfile_detail;
    std::map<uint, std::string>                 m_RT_outputfile_detail_name;
    std::map<uint, std::string>                 m_RT_outputfile_detail_head;
    std::map<uint, std::ofstream>               m_outputfile_result;
    std::ofstream                               m_outputfile_combined_detail;
    std::ofstream                               m_RT_outputfile_combined_detail;
    std::string                                 m_RT_outputfile_combined_detail_name;
    std::string                                 m_RT_outputfile_combined_detail_head;
    std::ofstream                               m_outputfile_combined_result;
    std::ofstream                               m_outputfile_mc_summary;
    std::ofstream                               m_outputfile_power;
    MemoryTestcaseCfg_t                         m_TC_Cfg;
    std::list<TestItConfig_t>                   m_global_test_it_list;
    std::map<uint, std::list<TestItConfig_t>>   m_chan_test_it_list;
    std::map<uint, TestItConfig_t>              m_chan_test_it;
    std::map<uint, bool>                        m_chan_seq_override;
    bool                                        m_seq_override = false;
    std::map<uint, Chan_Status_t>               m_chan_status;
    std::map<uint, uint64_t>                    m_stat_axi_addr_ptr;
    uint                                        m_tog_ch_idx = 0;
    uint                                        m_tog_1sec = 0;
    uint                                        m_tog_1sec_last = 1;
    uint                                        m_toggle_error_cnt = 0;
    std::map<uint,uint>                         m_timestamp_1_sec_expected;
    uint                                        m_hw_sec_cnt = 0;
    uint                                        m_sw_sec_cnt_start = 0;
    uint                                        m_sw_sec_cnt = 0;
    bool                                        m_mem_048_display = true;
    Data_Integrity_t                            m_combined_data_integrity = RESET_DATA_INTEGRITY;
    std::map<uint, Data_Integrity_t>            m_data_integrity;
    // Burst time, BW and burst latency results per channel
    std::map<uint, Stat_Burst_t>                m_bw_wr_plus_rd; // Only used to show wr + plus rd BW
    // combined_bw_wr_plus_rd will have the sum of wr + rd BW for all channel. No burst time info as not applicable for wr+rd
    Stat_Burst_t                                m_combined_bw_wr_plus_rd;
    // combined_burst_latency_and_bw_wr/rd will have the sum of BW and the average of burst time and min/max of all channels
    Stat_Burst_t                                m_combined_burst_latency_and_bw_wr;
    Stat_Burst_t                                m_combined_burst_latency_and_bw_rd;
    bool                                        m_combined_is_write;
    bool                                        m_combined_is_read;
    bool                                        m_combined_check_data_integrity;
    std::vector<uint>                           m_is_write_ch_idx;
    std::vector<uint>                           m_is_read_ch_idx;
    Test_Cfg_Wr_Rd_t                            m_min;
    Test_Cfg_Wr_Rd_t                            m_max;
    Mem_Rate_CU_t                               m_cu_rate;
    Mem_Outstanding_CU_t                        m_cu_outstanding;
    Mem_Burst_Size_CU_t                         m_cu_burst_size;
    uint                                        m_AXI_num_data_bytes;
    uint                                        m_num_axi_thread;

    void            PrintTestcaseConfig         ( const MemoryCoreCfg_t & chan_config );
    std::string     GetCUConnectionVppSptag     ( const int & ch_idx );
    uint64_t        GetAxiBaseAddress           ( const int & ch_idx );
    bool            ReadMemoryCu                ( const int & ch_idx, const uint & address, uint & read_data );
    bool            WriteMemoryCu               ( const int & ch_idx, const uint & address, const uint & value );
    void            ReadMemoryCuStatus();
    void            ComputeStatBurst();
    double          StatBurstValToNs            ( const double & val );
    double          ComputeAverageMbps          ( const uint & burst_size, const double & average_ns );
    double          ComputeBwMbps               ( const uint & burst_size, const double & inst );
    void            GetLiveStatBurst            ( Stat_Burst_t & stat_burst, const uint & burst_size );
    void            AccumulateBurstStat         ( Stat_Burst_t & stat_burst, const uint & burst_size );
    void            WaitHw1SecToggle();
    void            CheckTimestamp1Sec();
    void            PrintRegStatBurst           ( const std::string & stat_name, const Stat_Burst_t & stat_burst );
    void            PrintStatus();
    void            ClearError                  ( const int & ch_idx );
    void            SetRateReg                  ( const int & ch_idx, const double & value_percent, const uint & burst_xfer, const uint & rate_ctrl_en, const uint & rate_reg_addr );
    void            SetWrRate                   ( const int & ch_idx, const double & value_percent, const uint & burst_xfer );
    void            SetRdRate                   ( const int & ch_idx, const double & value_percent, const uint & burst_xfer );
    void            EnablePattern               ( const int & ch_idx );
    void            DisablePattern              ( const int & ch_idx );
    void            SetPatternA                 ( const int & ch_idx, const uint & pattern );
    void            SetPatternB                 ( const int & ch_idx, const uint & pattern );
    void            SetAxiMultiThread           ( const int & ch_idx );
    void            SetAxiSingleThread          ( const int & ch_idx );
    void            SetOutstandingWrites        ( const int & ch_idx, const uint & value );
    void            SetOutstandingReads         ( const int & ch_idx, const uint & value );
    void            SetMode                     ( const int & ch_idx, const uint & value );
    bool            StartCU() override;
    bool            EnableWatchdogClkThrotDetection() override;
    bool            StopCU() override;
    bool            CheckWatchdogAndClkThrottlingAlarms() override;
    bool            StopCuCores();
    void            ActivateResetCores();
    void            ClearResetCores();
    void            UpdateCuConfig              ( const int & ch_idx );
    bool            WaitCuConfigUpdated         ( const std::vector<uint> & ch_indexes );
    void            SetWrCtrlAddr               ( const int & ch_idx, const uint & addr_mb );
    void            SetWrCtrlXferBytes          ( const int & ch_idx, const uint & value );
    void            SetWrCtrlNumXfer            ( const int & ch_idx, const uint & value );
    void            SetRdCtrlAddr               ( const int & ch_idx, const uint & addr_mb );
    void            SetRdCtrlXferBytes          ( const int & ch_idx, const uint & value );
    void            SetRdCtrlNumXfer            ( const int & ch_idx, const uint & value );
    void            CheckDataIntegrity();
    void            GetCombinedDataIntegrity();
    void            GetWrPlusRdBw();
    void            GetCombinedWrPlusRdBw();
    Stat_Burst_t    GetCombinedBurstStat        ( const std::string & stat );
    bool            CheckPrbsErrorLatches       ( const int & ch_idx );
    uint            GetHw1secToggle             ( const int & ch_idx );
    bool            ReadMemoryCu64b             ( const int & ch_idx, const uint & addr_lsb, const uint & addr_msb, uint64_t & read_data_64 );
    uint64_t        GetAxiAddrPtr               ( const int & ch_idx );
    bool            SetSequenceAndUpdateCores();
    void            SetSequence                 ( const int & ch_idx, const TestItConfig_t & test_it );

    void WriteToMeasurementFileDetail(
        const std::string &         global_time,
        std::ofstream &             measurement_file,
        std::ofstream &             RT_measurement_file,
        std::string &               RT_measurement_file_name,
        std::string &               RT_measurement_file_head,
        const uint &                test_idx,
        const bool &                seq_override,
        const bool &                is_write,
        const bool &                is_read,
        const TestItConfig_t &      test_it,
        const uint &                timestamp_1_sec,
        const Data_Integrity_t &    data_integrity,
        const Stat_Burst_t &        burst_latency_and_bw_wr,
        const Stat_Burst_t &        burst_latency_and_bw_rd,
        const Stat_Burst_t &        bw_wr_plus_rd
    );
    void WriteToMeasurementFilePower(
        const std::string &     global_time,
        std::ofstream &         measurement_file,
        const uint &            test_idx,
        const bool &            seq_override,
        const bool &            is_write,
        const bool &            is_read,
        const TestItConfig_t &  test_it
    );
    void WriteToMeasurementFileResult(
        std::ofstream &             measurement_file,
        const uint &                test_idx,
        const bool &                seq_override,
        const bool &                is_write,
        const bool &                is_read,
        const TestItConfig_t &      test_it,
        const Data_Integrity_t &    data_integrity,
        const Stat_Burst_t &        burst_latency_and_bw_wr,
        const Stat_Burst_t &        burst_latency_and_bw_rd,
        const Stat_Burst_t &        bw_wr_plus_rd
    );
    void WriteToMeasurementFileMcSummary(
        std::ofstream &                     measurement_file,
        const uint &                        test_idx,
        const bool &                        seq_override,
        std::map<uint, TestItConfig_t> &    chan_test_it,
        std::map<uint, Chan_Status_t> &     chan_status,
        const bool &                        combined_is_write,
        const bool &                        combined_is_read,
        const TestItConfig_t &              combined_test_it,
        const Data_Integrity_t &            combined_data_integrity,
        const Stat_Burst_t &                combined_burst_latency_and_bw_wr,
        const Stat_Burst_t &                combined_burst_latency_and_bw_rd,
        const Stat_Burst_t &                combined_bw_wr_plus_rd
    );


    std::string GetMessageTagChan           ( const int & ch_idx );
    std::string GetMessageTagChan2          ( const int & ch_idx );
    std::string GetMessageChan              ( const int & ch_idx );
    std::string GetPassMessageChan();
    std::string GetFailMessageChan();
    void        DisplayStartExtraParam      ( const TestItConfig_t & test_it, const std::string & opt_chan_info );
    bool        PreWriteStep();
    int         RunTest() override;
    bool        CheckAxiAddrPtr();
    bool        CheckTestConfigWrRd         ( const std::string & direction, Test_Cfg_Wr_Rd_t & test_it_wr_rd );
    bool        CheckWrRdMemoryAreas        ( const TestItConfig_t & test_it );
    bool        CheckTestItConfig           ( TestItConfig_t & test_it );
    bool        ParseTestSequenceSettings   ( const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence, std::list<TestItConfig_t> & test_list );
    bool        GetTestSequence();
    bool        CheckTestItResults();
    std::string TestModeEnumToString        ( const uint & type );
    uint        TestModeStringToEnum        ( const std::string & type );
    bool        IsTestModeWrite             ( const uint & mode_in );
    bool        IsTestModeRead              ( const uint & mode_in );

};

} // namespace

#endif /* _MEMORYTEST_H */
