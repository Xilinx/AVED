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

#ifndef _MMIOTEST_H
#define _MMIOTEST_H

#if defined(USE_AMI) || defined(USE_NO_DRIVER)

#include <list>

#include "testinterface.h"

namespace xbtest
{

class MmioTest: public TestInterface
{

public:
    MmioTest( Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, const Mmio_Parameters_t & test_parameters );
    ~MmioTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

private:
    const uint       MIN_RESULTS_CHECK_DURATION = 10;

    const uint8_t    MIN_BAR = 0;
    const uint8_t    MAX_BAR = MAX_UINT8_VAL; // BAR index is check against valid BARs from metadata

    const uint64_t   MIN_TOTAL_SIZE = sizeof(uint32_t); // Bytes - Limit due to API limitation
    const uint64_t   MAX_TOTAL_SIZE = MAX_UINT64_VAL; // Bytes - PCIe BAR range is used to saturate maximum buffer size

    const uint64_t   MIN_BUFFER_SIZE = sizeof(uint32_t); // Bytes - Limit due to API limitation
    const uint64_t   MAX_BUFFER_SIZE = MAX_UINT64_VAL; // Bytes - total size is used to saturate maximum buffer size

    const uint64_t   MIN_OFFSET = 0; // Max offset depend on total size and buffer size

    const uint       MIN_LO_THRESH_WR = 1;
    const uint       MAX_LO_THRESH_WR = MAX_UINT_VAL;
    const uint       MIN_HI_THRESH_WR = 1;
    const uint       MAX_HI_THRESH_WR = MAX_UINT_VAL;
    const uint       MIN_LO_THRESH_RD = 1;
    const uint       MAX_LO_THRESH_RD = MAX_UINT_VAL;
    const uint       MIN_HI_THRESH_RD = 1;
    const uint       MAX_HI_THRESH_RD = MAX_UINT_VAL;

    using MmioTcCfg_t = struct MmioTcCfg_t {
        uint64_t    total_size;
        uint        lo_thresh_wr;
        uint        hi_thresh_wr;
        uint        lo_thresh_rd;
        uint        hi_thresh_rd;
        bool        check_bw_exists;
        bool        check_bw;
        bool        check_data_integrity_exists;
        bool        check_data_integrity;
        bool        stop_on_error;
        uint8_t     nom_bar;
        uint64_t    nom_total_size;
        uint64_t    nom_buffer_size;
        uint64_t    nom_offset;
    };
    const MmioTcCfg_t RST_MMIOTCCFG = {
        .total_size                     = 0,
        .lo_thresh_wr                   = 0,
        .hi_thresh_wr                   = 0,
        .lo_thresh_rd                   = 0,
        .hi_thresh_rd                   = 0,
        .check_bw_exists                = false,
        .check_bw                       = false,
        .check_data_integrity_exists    = false,
        .check_data_integrity           = false,
        .stop_on_error                  = false,
        .nom_bar                        = 0,
        .nom_total_size                 = 0,
        .nom_buffer_size                = 0,
        .nom_offset                     = 0
    };

    using Test_It_Stat_t = struct Test_It_Stat_t {
        uint64_t wr_bw_pass_cnt;
        uint64_t rd_bw_pass_cnt;
        uint64_t wr_bw_fail_cnt;
        uint64_t rd_bw_fail_cnt;
    };
    const Test_It_Stat_t RST_TEST_IT_STAT = {
        .wr_bw_pass_cnt = 0,
        .rd_bw_pass_cnt = 0,
        .wr_bw_fail_cnt = 0,
        .rd_bw_fail_cnt = 0
    };

    using TestItConfig_t = struct TestItConfig_t {
        uint                        duration; // sec
        uint8_t                     bar;
        uint64_t                    total_size;
        uint64_t                    buffer_size; // Bytes
        uint64_t                    offset;
        uint64_t                    actual_offset;
        uint64_t                    t_start; // Start timestamp
        uint64_t                    elapsed; // Elapsed duration of test (us)
        uint64_t                    wr_rd_cycle_cnt;
        uint64_t                    buff_size_int; // Number of int
        uint64_t                    buffer_count;

        uint64_t                    max_total_size;
        uint64_t                    nom_buffer_size;
        uint64_t                    max_buffer_size;
        uint64_t                    nom_offset;
        uint64_t                    max_offset;

        Test_It_Stat_t              test_it_stat;
        uint                        parent_test_idx; // This is used to differentiate test index fron sequence in JSON, and extended test sequence when test sequence mode "all" is present
        std::vector<std::string>    test_strvect;
        std::vector<std::string>    test_strvect_dbg;
    };

    using Rate_t = struct Rate_t {
        bool   valid;
        double inst;
        double min;
        double max;
        double acc;
        double avg;
    };
    const Rate_t RST_RATE = {
        false, // bool   valid;
        0.0,   // double inst;
        0.0,   // double min;
        0.0,   // double max;
        0.0,   // double acc;
        0.0    // double avg;
    };

    Mmio_Parameters_t                               m_test_parameters;
    std::vector<Mmio_Test_Sequence_Parameters_t>    m_test_sequence;
    int                                             m_page_size = 0;
    MmioTcCfg_t                                     m_TC_Cfg = RST_MMIOTCCFG;
    std::list<TestItConfig_t>                       m_test_it_list;
    std::string                                     m_outputfile_name;
    bool                                            m_use_outputfile = false;
    std::ofstream                                   m_outputfile_detail;
    std::ofstream                                   m_RT_outputfile_detail;
    std::string                                     m_RT_outputfile_detail_name;
    std::string                                     m_RT_outputfile_detail_head;
    std::ofstream                                   m_outputfile_result;

    bool    GetTestSequence();
    void    SetTestSequenceStr              ( const Mmio_Test_Sequence_Parameters_t & test_seq_param, TestItConfig_t & test_it_cfg );
    void    SetTestSequenceStrDbg           ( TestItConfig_t & test_it_cfg );
    bool    ParseTestSequenceSettings       ( std::list<TestItConfig_t> & test_list );
    void    StopOnError                     ( const uint & test_it_cnt, const bool & test_it_failure, bool & stop_test_it, const std::string & msg );
    int     RunTest() override;
    bool    CheckDataIntegrity              ( uint32_t * host_buf, uint32_t * ref_data_buf, const uint64_t & buff_size_bytes, const uint64_t & buff_size_int );
    bool    ComputeRate                     ( const TestItConfig_t & test_it_cfg, const uint64_t & t_1, const uint64_t & t_0, Rate_t & rate );
    bool    CheckRate                       ( TestItConfig_t & test_it_cfg, const Rate_t & rate_wr, const Rate_t & rate_rd, const bool & report_pass_fail );
    void    PrintResults                    ( const bool & info_n_debug, const TestItConfig_t & test_it_cfg, const Rate_t & rate_wr, const Rate_t & rate_rd );
    void    WriteToMeasurementFileResult    ( const uint & test_it_cnt, const TestItConfig_t & test_it_cfg, const std::string & data_integ_str, const Rate_t & rate_wr, const Rate_t & rate_rd );
    void    WriteToMeasurementFileDetail    ( const uint & test_it_cnt, const TestItConfig_t & test_it_cfg, const std::string & data_integ_str, const Rate_t & rate_wr, const Rate_t & rate_rd );

    bool    StartCU() override; // not used
    bool    EnableWatchdogClkThrotDetection() override; // not used
    bool    StopCU() override; // not used
    bool    CheckWatchdogAndClkThrottlingAlarms() override; // not used

};

} // namespace

#endif

#endif /* _MMIOTEST_H */
