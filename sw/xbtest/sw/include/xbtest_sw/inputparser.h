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

#ifndef _INPUTPARSER_H
#define _INPUTPARSER_H

#include "xjsonparser.h"
#include "xbtestswconfig.h"

namespace xbtest
{

class InputParser : public XJsonParser
{

public:
    InputParser( Logging * log, XbtestSwConfig * xbtest_sw_config, std::atomic<bool> * abort );
    ~InputParser();

    bool                    Parse() override;
    Tasks_Parameters_t      GetTasksParameters();
    Testcases_Parameters_t  GetTestcasesParameters();
    void                    PrintJsonParameters             ( const bool & info_n_debug, const std::string & base_name, const std::string & param_type, const Json_Params_Def_t & parameters_definition, Json_Parameters_t & json_parameters, const bool & print_test_seq = true, const uint & level = 0 );
    void                    PrintJsonTestSeqAndResult       ( const bool & info_n_debug, const std::string & base_name, const Json_Params_Def_t & parameters_definition, Json_Parameters_t & json_parameters, const std::vector<TestIterationResult> & test_it_results );
    void                    PrintJsonTestSeqAndResultThreads( const bool & info_n_debug, const std::string & base_name, const Json_Params_Def_t & parameters_definition, Json_Parameters_t & json_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map );
    void                    SetLogHeader                    ( const std::string & log_header );
    bool                    PrintGuide                      ( const std::string & testcase_task_name );
    Json_Params_Def_t       GetPowerGlobalParametersDef();
    Json_Params_Def_t       GetMemoryTagChanParametersDef   ( const std::string & memory_name );
    Json_Params_Def_t       GetMemoryGlobalParametersDef    ( const std::string & memory_name );

private:
    const std::set<std::string>     EMPTY_SET                   = {};
    const std::vector<std::string>  EMPTY_VECT                  = {};
    const std::string               TEST_JSON_EXAMPLE_COMMENT   = R"(
    "comment": "This is an example of test JSON file",
    "comment": "You can use this example as template for your own tests",
    "comment": "Please refer to the User Guide for how to define or add/remove testcases",
    "comment": "Comments can be added or removed anywhere in test JSON file",
)";

    XbtestSwConfig *        m_xbtest_sw_config = nullptr;
    Tasks_Parameters_t      m_tasks_param;
    Testcases_Parameters_t  m_testcases_param;
    Json_Definition_t       m_device_mgmt_sensor_json_def;

    std::vector<std::string> m_supported_gt_prbs_node_names;
    std::vector<std::string> m_supported_gtf_prbs_node_names;
    std::vector<std::string> m_supported_gtm_prbs_node_names;
    std::vector<std::string> m_supported_gtyp_prbs_node_names;

    Json_Params_Def_t       m_power_global_config_parameters_def_full   = POWER_GLOBAL_CONFIG_PARAMETERS_DEF;
    Json_Params_Def_t       m_memory_global_config_parameters_def_full  = MEMORY_GLOBAL_CONFIG_PARAMETERS_DEF;
    Json_Params_Def_t       m_memory_tag_chan_parameters_def_full       = MEMORY_TAG_CHAN_PARAMETERS_DEF;
    Json_Params_Def_t       m_testcases_parameters_def_supported        = TESTCASES_PARAMETERS_DEF;

    std::string         TestIterationResultToString             ( const TestIterationResult & Result );
    void                AppendJsonDefinition                    ( const std::vector<std::string> & base_title, const Json_Params_Def_t & parameters_definition, const Hidden_t & top_hidden, const  bool & visible_only, Json_Definition_t & json_definition );
    Json_Definition_t   GetJsonDefinition                       ( const bool & visible_only );
    bool                CheckBothProvided                       ( const std::vector<std::string> & node_title_in, const std::string & name_1, const bool & exists_1, const std::string & name_2, const bool & exists_2 );
    bool                CheckWriteReadOnly                      ( const std::string & direction, const std::vector<std::string> & node_title_in,const  std::string & mode );
    void                PrintRequiredNotFound                   ( const std::vector<std::string> & node_title_in );
    bool                ParseSensor                             ( const std::vector<std::string> & base_title, std::vector<Device_Mgmt_Sensor_t> & sensors );
    bool                ParseTasksParameters();
    void                PrintJson();
    bool                ParseTestcasesParameters();
    bool                CheckCuForTestcase();

    bool                ParseGtPrbsIndexJsonParameters          ( const std::string & base_name, std::vector<std::string> & base_title, const std::string & name, Gt_Parameters_t & gt_parameters );
    bool                ParseMultiGtPrbsIndexJsonParameters     ( const std::string & base_name, std::vector<std::string> & base_title, const std::string & name, Gt_Parameters_t & gt_parameters );

    bool                ParseJsonParameters                     ( const std::string & base_name, const std::vector<std::string> & base_title, const Json_Params_Def_t & json_params_def, Json_Parameters_t & param );
    bool                ParseMmioTestSequenceObject             ( const std::string & test_sequence_name,                           Mmio_Test_Sequence_Parameters_t &        test_seq_param );
    bool                ParseMmioTestSequenceArray              ( const std::string & test_sequence_name, const uint & param_index, Mmio_Test_Sequence_Parameters_t &        test_seq_param );
    bool                ParseDMATestSequenceObject              ( const std::string & test_sequence_name,                           DMA_Test_Sequence_Parameters_t &         test_seq_param );
    bool                ParseDMATestSequenceArray               ( const std::string & test_sequence_name, const uint & param_index, DMA_Test_Sequence_Parameters_t &         test_seq_param );
    bool                ParseP2PCardTestSequenceObject          ( const std::string & test_sequence_name,                           P2P_Card_Test_Sequence_Parameters_t &    test_seq_param );
    bool                ParseP2PCardTestSequenceArray           ( const std::string & test_sequence_name, const uint & param_index, P2P_Card_Test_Sequence_Parameters_t &    test_seq_param );
    bool                ParseP2PNvmeTestSequenceObject          ( const std::string & test_sequence_name,                           P2P_Nvme_Test_Sequence_Parameters_t &    test_seq_param );
    bool                ParseP2PNvmeTestSequenceArray           ( const std::string & test_sequence_name, const uint & param_index, P2P_Nvme_Test_Sequence_Parameters_t &    test_seq_param );
    bool                ParseMemoryTestSequenceObject           ( const std::string & test_sequence_name,                           Memory_Test_Sequence_Parameters_t &      test_seq_param );
    bool                ParseMemoryTestSequenceArray            ( const std::string & test_sequence_name, const uint & param_index, Memory_Test_Sequence_Parameters_t &      test_seq_param );
    bool                ParsePowerTestSequenceObject            ( const std::string & test_sequence_name,                           Power_Test_Sequence_Parameters_t &       test_seq_param );
    bool                ParsePowerTestSequenceArray             ( const std::string & test_sequence_name, const uint & param_index, Power_Test_Sequence_Parameters_t &       test_seq_param );
    bool                ParseGTMACTestSequenceObject            ( const std::string & test_sequence_name,                           GTMAC_Test_Sequence_Parameters_t &       test_seq_param );
    bool                ParseGTMACTestSequenceArray             ( const std::string & test_sequence_name, const uint & param_index, GTMAC_Test_Sequence_Parameters_t &       test_seq_param );
    bool                ParseGTLpbkTestSequenceObject           ( const std::string & test_sequence_name,                           GTLpbk_Test_Sequence_Parameters_t &      test_seq_param );
    bool                ParseGTLpbkTestSequenceArray            ( const std::string & test_sequence_name, const uint & param_index, GTLpbk_Test_Sequence_Parameters_t &      test_seq_param );
    bool                ParseGTPrbsTestSequenceObject           ( const std::string & test_sequence_name,                           GTPrbs_Test_Sequence_Parameters_t &      test_seq_param );
    bool                ParseGTPrbsTestSequenceArray            ( const std::string & test_sequence_name, const uint & param_index, GTPrbs_Test_Sequence_Parameters_t &      test_seq_param );
    bool                ParseMultiGTPrbsTestSequenceObject      ( const std::string & test_sequence_name,                           MultiGTPrbs_Test_Sequence_Parameters_t & test_seq_param );
    bool                ParseMultiGTPrbsTestSequenceArray       ( const std::string & test_sequence_name, const uint & param_index, MultiGTPrbs_Test_Sequence_Parameters_t & test_seq_param );
    bool                GetNodeTestSeqUint                      ( const std::string & name, Test_Seq_Param_Uint_t & test_seq_param );
    bool                GetNodeTestSeqStr                       ( const std::string & name, Test_Seq_Param_Str_t &  test_seq_param );
    bool                ExtractNodeTestSeqUint                  ( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Uint_t &   test_seq_param, uint & num_param );
    bool                ExtractNodeTestSeqUint8                 ( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Uint8_t &  test_seq_param, uint & num_param );
    bool                ExtractNodeTestSeqUint64                ( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Uint64_t & test_seq_param, uint & num_param );
    bool                ExtractNodeTestSeqStr                   ( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Str_t &    test_seq_param, uint & num_param );
    bool                CheckStringInSet                        ( const std::string & param_name, const std::string & value, const std::set<std::string> &     test_set, const std::set<std::string> &    hidden_test_set );
    bool                CheckStringInVect                       ( const std::string & param_name, const std::string & value, const std::vector<std::string> & test_vect, const std::vector<std::string> & hidden_test_vect );
    bool                ParseJsonParamStr                       ( const std::vector<std::string> & node_title, const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_parameters );
    bool                ParseJsonParamBool                      ( const std::vector<std::string> & node_title, const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_parameters );
    bool                ParseTestSequence                       ( const std::string & testcase_name, const std::vector<std::string> & testsequence_node_title, Json_Parameters_t & json_parameters );
    void                PrintJsonDefintion                      ( const Json_Definition_t & json_definition );
    void                PrintSensorLimit                        ( const bool & info_n_debug, const std::string & limit_str, const Sensor_Limit_t & sensor_limit );
    std::string         GetUintTestSequenceString               ( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Uint_t &   test_seq_param );
    std::string         GetUint8TestSequenceString              ( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Uint8_t &  test_seq_param );
    std::string         GetUint64TestSequenceString             ( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Uint64_t & test_seq_param );
    std::string         GetStrTestSequenceString                ( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Str_t &    test_seq_param );
    std::string         GetMmioTestSequenceString               ( const uint & test_seq_param_idx, const Mmio_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintMmioTestSequence                   ( const bool & info_n_debug, const std::vector<Mmio_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintMmioTestSequenceAndResult          ( const bool & info_n_debug, const std::vector<Mmio_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results );
    std::string         GetDMATestSequenceString                ( const uint & test_seq_param_idx, const DMA_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintDMATestSequence                    ( const bool & info_n_debug, const std::vector<DMA_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintDMATestSequenceAndResult           ( const bool & info_n_debug, const std::vector<DMA_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results );
    std::string         GetP2PCardTestSequenceString            ( const uint & test_seq_param_idx, const P2P_Card_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintP2PCardTestSequence                ( const bool & info_n_debug, const std::vector<P2P_Card_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintP2PCardTestSequenceAndResult       ( const bool & info_n_debug, const std::vector<P2P_Card_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results );
    std::string         GetP2PNvmeTestSequenceString            ( const uint & test_seq_param_idx, const P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintP2PNvmeTestSequence                ( const bool & info_n_debug, const std::vector<P2P_Nvme_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintP2PNvmeTestSequenceAndResult       ( const bool & info_n_debug, const std::vector<P2P_Nvme_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results );
    std::string         GetMemoryTestSequenceString             ( const uint & test_seq_param_idx, const Memory_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintMemoryTestSequence                 ( const bool & info_n_debug, const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintMemorySCTestSequenceAndResult      ( const bool & info_n_debug, const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map );
    void                PrintMemoryMCTestSequenceAndResult      ( const bool & info_n_debug, const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results );
    std::string         GetPowerTestSequenceString              ( const uint & test_seq_param_idx, const Power_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintPowerTestSequence                  ( const bool & info_n_debug, const std::vector<Power_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintPowerTestSequenceAndResult         ( const bool & info_n_debug, const std::vector<Power_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results );
    std::string         GetGTMACTestSequenceString              ( const uint & test_seq_param_idx, const GTMAC_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintGTMACTestSequence                  ( const bool & info_n_debug, const std::vector<GTMAC_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintGTMACTestSequenceAndResult         ( const bool & info_n_debug, const std::vector<GTMAC_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map );
    std::string         GetGTLpbkTestSequenceString             ( const uint & test_seq_param_idx, const GTLpbk_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintGTLpbkTestSequence                 ( const bool & info_n_debug, const std::vector<GTLpbk_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintGTLpbkTestSequenceAndResult        ( const bool & info_n_debug, const std::vector<GTLpbk_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map );
    std::string         GetGTPrbsTestSequenceString             ( const uint & test_seq_param_idx, const GTPrbs_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintGTPrbsTestSequence                 ( const bool & info_n_debug, const std::vector<GTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintGTPrbsTestSequenceAndResult        ( const bool & info_n_debug, const std::vector<GTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map );
    std::string         GetMultiGTPrbsTestSequenceString        ( const uint & test_seq_param_idx, const MultiGTPrbs_Test_Sequence_Parameters_t & test_seq_param );
    void                PrintMultiGTPrbsTestSequence            ( const bool & info_n_debug, const std::vector<MultiGTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters );
    void                PrintMultiGTPrbsTestSequenceAndResult   ( const bool & info_n_debug, const std::vector<MultiGTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map );
    void                LogMessageIf                            ( const bool & info_n_debug, const Message_t & message, const std::string & msg );
    void                PrintJsonParamsGuide                    ( const std::string & tabs, const Json_Params_Def_t & json_params_def );
    void                PrintMmioTestSequenceGuide();
    void                PrintDMATestSequenceGuide();
    void                PrintP2PCardTestSequenceGuide();
    void                PrintP2PNvmeTestSequenceGuide();
    void                PrintMemoryTestSequenceGuide();
    void                PrintPowerTestSequenceGuide();
    void                PrintGTMACTestSequenceGuide();
    void                PrintGTLpbkTestSequenceGuide();
    void                PrintGTPrbsTestSequenceGuide();
    void                PrintMultiGTPrbsTestSequenceGuide( const std::string & multi_gt_name );
    bool                PrintDeviceMmgmtSensorGuide();
    void                PrintMmioJSONExample();
    void                PrintDMAJSONExample();
    void                PrintP2PCardJSONExample();
    void                PrintP2P2NvmJSONExample();
    void                PrintMemoryJSONExample();
    void                PrintPowerJSONExample();
    void                PrintGTMACJSONExample();
    void                PrintGTLpbkJSONExample();
    void                PrintGTPrbsJSONExample();
    void                PrintMultiGTPrbsJSONExample( const std::string & multi_gt_name );
    void                PrintDeviceMmgmtJSONExample();

    template<typename T> bool ParseJsonParamInt( const std::vector<std::string> & node_title, const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_parameters )
    {
        T param;
        if (ExtractNodeValueInt<T>(node_title, param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        InsertJsonParam<T>(json_parameters, json_val_def, param);
        return RET_SUCCESS;
    }
    template<typename T> bool ParseJsonParamDouble( const std::vector<std::string> & node_title, const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_parameters )
    {
        T param;
        if (ExtractNodeValueDouble<T>(node_title, param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        InsertJsonParam<T>(json_parameters, json_val_def, param);
        return RET_SUCCESS;
    }

    template<typename T> void AppendTestSequenceParameters( Json_Parameters_t & json_parameters, const T & test_seq_param )
    {
        std::vector<T> tmp_val;
        auto it = FindJsonParam(json_parameters, TEST_SEQUENCE_MEMBER);
        if (it != json_parameters.end())
        {
            tmp_val = TestcaseParamCast<std::vector<T>>(it->second);
        }
        tmp_val.emplace_back(test_seq_param);
        EraseJsonParam(json_parameters, TEST_SEQUENCE_MEMBER);
        InsertJsonParam<std::vector<T>>(json_parameters, TEST_SEQUENCE_MEMBER, tmp_val);
    }

};

} // namespace

#endif /* _INPUTPARSER_H */
