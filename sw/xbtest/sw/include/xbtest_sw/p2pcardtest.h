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

#ifndef _P2PCARDTEST_H
#define _P2PCARDTEST_H

#ifdef USE_XRT

#include <list>

#include "testinterface.h"

namespace xbtest
{

class P2PCardTest: public TestInterface
{

public:
    P2PCardTest( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, const P2P_Card_Parameters_t & test_parameters );
    ~P2PCardTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

private:
    const uint       MIN_RESULTS_CHECK_DURATION = 10;
    const uint       MIN_TOTAL_SIZE = 1;  // MB, for all types, MAX_TOTAL_SIZE set to memory size
    const uint64_t   MIN_BUFFER_SIZE = 1; // MB
    const uint64_t   NOM_BUFFER_SIZE = 256; // MB used to saturate the nominal buffer size (depend on total size)
    const uint64_t   MAX_BUFFER_SIZE = 2048; // MB used to saturate the maximum buffer size (depend on total size)
    const uint       MIN_LO_THRESH_WR = 1;
    const uint       MAX_LO_THRESH_WR = MAX_UINT_VAL;
    const uint       MIN_HI_THRESH_WR = 1;
    const uint       MAX_HI_THRESH_WR = MAX_UINT_VAL;
    const uint       MIN_LO_THRESH_RD = 1;
    const uint       MAX_LO_THRESH_RD = MAX_UINT_VAL;
    const uint       MIN_HI_THRESH_RD = 1;
    const uint       MAX_HI_THRESH_RD = MAX_UINT_VAL;

    using P2PMemTypeCfg_t = struct P2PMemTypeCfg_t {
        std::string                 name;
        std::vector<std::string>    tag;
        uint                        nom_total_size;
        uint                        total_size;
        bool                        lo_thresh_wr_exists;
        bool                        hi_thresh_wr_exists;
        bool                        lo_thresh_rd_exists;
        bool                        hi_thresh_rd_exists;
        uint                        lo_thresh_wr;
        uint                        hi_thresh_wr;
        uint                        lo_thresh_rd;
        uint                        hi_thresh_rd;
        bool                        check_bw_exists;
        bool                        check_bw;
        bool                        check_data_integrity_exists;
        bool                        check_data_integrity;
        bool                        stop_on_error;
        uint64_t                    nom_buffer_size;
        uint64_t                    max_buffer_size;
    };
    const P2PMemTypeCfg_t RST_P2PMEMTYPECFG = {
        .name                           = "",
        .tag                            = {},
        .nom_total_size                 = 0,
        .total_size                     = 0,
        .lo_thresh_wr_exists            = false,
        .hi_thresh_wr_exists            = false,
        .lo_thresh_rd_exists            = false,
        .hi_thresh_rd_exists            = false,
        .lo_thresh_wr                   = 0,
        .hi_thresh_wr                   = 0,
        .lo_thresh_rd                   = 0,
        .hi_thresh_rd                   = 0,
        .check_bw_exists                = false,
        .check_bw                       = false,
        .check_data_integrity_exists    = false,
        .check_data_integrity           = false,
        .stop_on_error                  = false,
        .nom_buffer_size                = 0,
        .max_buffer_size                = 0
    };

    using P2PTestcaseCfg_t = struct P2PTestcaseCfg_t {
        std::map<uint, P2PMemTypeCfg_t> memory_src;
        std::map<uint, P2PMemTypeCfg_t> memory_tgt;
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

    using TestItMemConfig_t = struct TestItMemConfig_t {
        bool                    mem_type_used;
        bool                    mem_tag_used;
        bool                    mem_default_used;
        std::string             mem_type;
        std::string             mem_tag;
        uint                    mem_cfg_idx;
        Xclbin_Memory_Data_t    memory_data;
    };

    using TestItConfig_t = struct TestItConfig_t {
        uint                        duration; // sec
        TestItMemConfig_t           src;
        TestItMemConfig_t           tgt;
        uint64_t                    buffer_size; // MB
        uint                        total_size; // MB
        uint                        lo_thresh_wr;
        uint                        hi_thresh_wr;
        uint                        lo_thresh_rd;
        uint                        hi_thresh_rd;
        uint64_t                    t_start; // Start timestamp
        uint64_t                    elapsed; // Elapsed duration of test (us)
        uint64_t                    wr_rd_cycle_cnt;
        uint64_t                    total_size_bytes; // B
        uint64_t                    buff_size_bytes; // B
        uint64_t                    buff_size_int; // Number of int
        uint64_t                    buffer_count;
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

    const bool CREATE_P2P_BUFFER     = true;
    const bool CREATE_REGULAR_BUFFER = false;

    const bool MIGRATE_TO_CARD = true;
    const bool MIGRATE_TO_HOST = false;

    XbtestSwConfig *                                 m_xbtest_sw_config_p2p = nullptr;
    P2P_Card_Parameters_t                            m_test_parameters;
    std::string                                      m_bdf_src;
    std::string                                      m_bdf_tgt;
    std::vector<P2P_Card_Test_Sequence_Parameters_t> m_test_sequence;
    int                                              m_page_size = 0;
    P2PTestcaseCfg_t                                 m_TC_Cfg;
    std::list<TestItConfig_t>                        m_test_it_list;
    std::string                                      m_outputfile_name;
    bool                                             m_use_outputfile = false;
    std::ofstream                                    m_outputfile_detail;
    std::ofstream                                    m_RT_outputfile_detail;
    std::string                                      m_RT_outputfile_detail_name;
    std::string                                      m_RT_outputfile_detail_head;
    std::ofstream                                    m_outputfile_result;

    // OpenCL
    std::vector<cl_platform_id> m_cl_platform_ids;
    uint                        m_platform_idx;
    std::vector<cl_device_id>   m_cl_device_ids;
    uint                        m_dev_idx_src;
    uint                        m_dev_idx_tgt;
    cl_context                  m_cl_context_src;
    cl_context                  m_cl_context_tgt;
    cl_command_queue            m_cl_queue_src;
    cl_command_queue            m_cl_queue_tgt;

    bool    SetupMemoryConfig               ( XbtestSwConfig * xbtest_sw_config, std::map<uint, P2P_Card_Type_Parameters_t> & memory_config, std::map<uint, P2PMemTypeCfg_t> & mem_cfg, bool check_bw_global, bool check_data_integrity_global, bool stop_on_error_global, bool check_bw_exists_global, bool check_data_integrity_exists_global );
    bool    GetTestSequence();
    bool    GetConfigIndexFromType          ( const std::string & mem_type, XbtestSwConfig * xbtest_sw_config, std::map<uint, P2PMemTypeCfg_t> & mem_cfg, uint & mem_cfg_idx );
    bool    GetConfigIndexFromTag           ( const std::string & mem_tag,  XbtestSwConfig * xbtest_sw_config, std::map<uint, P2PMemTypeCfg_t> & mem_cfg, uint & mem_cfg_idx );
    void    SelectTestItThreshold           ( const uint64_t & test_idx, const std::string & thresh_name, const bool & thresh_src_exists, const uint & thresh_src, const uint & thresh_tgt, uint & thresh_val );
    bool    SetOtherTestSequenceItSettings  ( const P2P_Card_Test_Sequence_Parameters_t & test_seq_param, const std::list<TestItConfig_t> & test_list, TestItConfig_t & test_it_cfg );
    void    SetTestSequenceStr              ( const P2P_Card_Test_Sequence_Parameters_t & test_seq_param, TestItConfig_t & test_it_cfg );
    void    SetTestSequenceStrDbg           ( TestItConfig_t & test_it_cfg );
    void    AppendTestIt                    ( const P2P_Card_Test_Sequence_Parameters_t & test_seq_param, TestItConfig_t & test_it_cfg, std::list<TestItConfig_t> & test_list, std::vector<std::string> & test_seq_strvect, std::vector<std::string> & test_seq_strvect_dbg, bool & parse_it_failure );
    void    AppendTestItTarget              ( const P2P_Card_Test_Sequence_Parameters_t & test_seq_param, std::list<TestItConfig_t> & test_list, TestItConfig_t & test_it_cfg, std::vector<std::string> & test_seq_strvect, std::vector<std::string> & test_seq_strvect_dbg, bool & parse_it_failure );
    bool    ParseTestSequenceSettings       ( std::list<TestItConfig_t> & test_list );
    bool    CreateClBuffer                  ( const std::string & src_tgt, const uint64_t & buffer_count, const uint64_t & buff_size_bytes, const uint & mem_data_idx, cl_context & context, std::vector<cl_mem> & cl_buffers, std::vector<int *> & host_buf, const bool & use_p2p_buffer );
    bool    ImportClBuffer                  ( std::vector<cl_mem> & cl_buffers_tgt, std::vector<cl_mem> & cl_buffers_tgt_imported );
    bool    MigrateClBuffer                 ( const std::string & src_tgt, cl_command_queue & queue, std::vector<cl_mem> & cl_buffers, const bool & wr_n_rd );
    bool    ReleaseClBuffer                 ( const std::string & src_tgt, std::vector<cl_mem> & cl_buffers );
    void    ReleaseHostbuffer               ( std::vector<int *> & host_buf );
    void    StopOnError                     ( const TestItConfig_t & test_it, const uint & test_it_cnt, const bool & test_it_failure, bool & stop_test_it, const std::string & msg );
    int     RunTest() override;
    bool    GetInMemTopology                ( XbtestSwConfig * xbtest_sw_config, TestItMemConfig_t & test_it_mem_cfg );
    bool    CheckDataIntegrity              ( int * host_buf, int * ref_data_buf, const uint64_t & buff_size_bytes, const uint64_t & buff_size_int );
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

#endif /* _P2PCARDTEST_H */
