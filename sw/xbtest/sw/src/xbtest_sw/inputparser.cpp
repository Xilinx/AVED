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

#include "inputparser.h"

namespace xbtest
{

InputParser::InputParser( Logging * log, XbtestSwConfig * xbtest_sw_config, std::atomic<bool> * abort ) : XJsonParser::XJsonParser(log, abort)
{
    this->m_xbtest_sw_config  = xbtest_sw_config;
    this->m_log_header        = LOG_HEADER_INPUTPARSER;
    this->m_content_name      = TEST_JSON;

    if (this->m_xbtest_sw_config->GetPwrSupportAIE())
    {
        // disable_aie is added to the definition only if the feature is present in HW design (USE_AIE in USER_METADATA)
        this->m_power_global_config_parameters_def_full.emplace_back(DISABLE_AIE_MEMBER);
    }
    this->m_memory_global_config_parameters_def_full.emplace_back(SINGLE_AXI_THREAD_MEMBER); // just to print the guide
    this->m_memory_global_config_parameters_def_full.emplace_back(PATTERN_MEMBER); // just to print the guide
    this->m_memory_tag_chan_parameters_def_full.emplace_back(SINGLE_AXI_THREAD_MEMBER); // just to print the guide
    this->m_memory_tag_chan_parameters_def_full.emplace_back(PATTERN_MEMBER); // just to print the guide

    this->m_testcases_parameters_def_supported.clear();
    if (this->m_xbtest_sw_config->GetIsMmioTestSupported())
    {
        this->m_testcases_parameters_def_supported.emplace_back(MMIO_MEMBER);
    }
    if (this->m_xbtest_sw_config->GetIsDmaSupported())
    {
        this->m_testcases_parameters_def_supported.emplace_back(DMA_MEMBER);
    }
    if (this->m_xbtest_sw_config->GetIsP2PSupported())
    {
        this->m_testcases_parameters_def_supported.emplace_back(P2P_CARD_MEMBER);
        this->m_testcases_parameters_def_supported.emplace_back(P2P_NVME_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailablePowerSLRs().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(POWER_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAllAvailableMemories().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(MEMORY_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailableGTMACs().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(GT_MAC_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailableGTLpbks().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(GT_LPBK_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailableGTPrbss().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(GT_PRBS_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailableGTFPrbss().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(GTF_PRBS_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailableGTMPrbss().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(GTM_PRBS_MEMBER);
    }
    if (!this->m_xbtest_sw_config->GetAvailableGTYPPrbss().empty())
    {
        this->m_testcases_parameters_def_supported.emplace_back(GTYP_PRBS_MEMBER);
    }
    for (const auto & sensor_val_def : DEVICE_MGMT_SENSOR_DEF)
    {
        this->m_device_mgmt_sensor_json_def.insert( Definition_t({sensor_val_def.name}, sensor_val_def.node_type) );
    }
    for (const auto & thresh_member : {SENSOR_WARNING_THRESHOLD_MEMBER, SENSOR_ERROR_THRESHOLD_MEMBER, SENSOR_ABORT_THRESHOLD_MEMBER})
    {
        for (const auto & thresh_val_def : DEVICE_MGMT_THRESHOLD_DEF)
        {
            this->m_device_mgmt_sensor_json_def.insert( Definition_t({thresh_member.name, thresh_val_def.name}, thresh_val_def.node_type) );
        }
    }

    this->m_supported_gt_prbs_node_names.clear();
    this->m_supported_gt_prbs_node_names.emplace_back(DEFAULT_MEMBER.name);
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTPrbss())
    {
        this->m_supported_gt_prbs_node_names.emplace_back(std::to_string(gt_index));
    }
    this->m_supported_gtf_prbs_node_names.clear();
    this->m_supported_gtf_prbs_node_names.emplace_back(DEFAULT_MEMBER.name);
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTFPrbss())
    {
        this->m_supported_gtf_prbs_node_names.emplace_back(std::to_string(gt_index));
    }
    this->m_supported_gtm_prbs_node_names.clear();
    this->m_supported_gtm_prbs_node_names.emplace_back(DEFAULT_MEMBER.name);
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMPrbss())
    {
        this->m_supported_gtm_prbs_node_names.emplace_back(std::to_string(gt_index));
    }
    this->m_supported_gtyp_prbs_node_names.clear();
    this->m_supported_gtyp_prbs_node_names.emplace_back(DEFAULT_MEMBER.name);
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTYPPrbss())
    {
        this->m_supported_gtyp_prbs_node_names.emplace_back(std::to_string(gt_index));
    }
}

InputParser::~InputParser()
{
    ClearParser();
}

bool InputParser::Parse()
{
    std::vector<std::string> node_title;

    this->m_tasks_param.timer.exists = false;
    this->m_tasks_param.timer.param.clear();

    this->m_tasks_param.device_mgmt.exists = false;
    this->m_tasks_param.device_mgmt.param.clear();
    this->m_tasks_param.device_mgmt.sensor.clear();

    this->m_tasks_param.debug.exists = false;
    this->m_tasks_param.debug.param.clear();

    this->m_testcases_param.mmio.exists = false;
    this->m_testcases_param.mmio.global_config.clear();

    this->m_testcases_param.dma.exists = false;
    this->m_testcases_param.dma.global_config.clear();
    this->m_testcases_param.dma.memory_config.clear();

    this->m_testcases_param.p2p_card.exists = false;
    this->m_testcases_param.p2p_card.global_config.clear();
    this->m_testcases_param.p2p_card.memory_config.clear();

    this->m_testcases_param.p2p_nvme.exists = false;
    this->m_testcases_param.p2p_nvme.global_config.clear();
    this->m_testcases_param.p2p_nvme.memory_config.clear();

    this->m_testcases_param.memory.exists = false;
    this->m_testcases_param.memory.memory_config.clear();

    this->m_testcases_param.power.exists = false;
    this->m_testcases_param.power.global_config.clear();

    this->m_testcases_param.gt_mac.clear();
    this->m_testcases_param.gt_lpbk.clear();
    this->m_testcases_param.gt_prbs.clear();
    this->m_testcases_param.gtf_prbs.clear();
    this->m_testcases_param.gtm_prbs.clear();
    this->m_testcases_param.gtyp_prbs.clear();

    LogMessage(MSG_DEBUG_PARSING, {"Parse: " + this->m_content_name});

    // check json configuration file exists
    auto test_json = this->m_xbtest_sw_config->GetCommandLineTestJson().value;
    // Initialize json parser and reader
    LogMessage(MSG_ITF_045, {this->m_content_name, test_json});
    this->m_json_parser = json_parser_new();

    GError * error = nullptr;
    json_parser_load_from_file(this->m_json_parser, test_json.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_content_name, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        g_object_unref(this->m_json_parser);
        return RET_FAILURE;
    }
    this->m_json_root_node  = json_parser_get_root(this->m_json_parser);
    this->m_json_reader     = json_reader_new(nullptr);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    // Check if "device" node exists (with the xbtest_pfm_def location) to print a clear error message
    if (NodeExists({"device"}))
    {
        LogMessage(MSG_ITF_093);
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_PARSING, {"Check xbtest HW IP is present for provided testcases"});
    if (CheckCuForTestcase() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_085);
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_PARSING, {"Check the json file content"});
    if (CheckMembers(GetJsonDefinition(false)) == RET_FAILURE)
    {
        // Print visible only
        PrintJsonDefintion(GetJsonDefinition(true));
        LogMessage(MSG_ITF_085);
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_PARSING, {"Parse the tasks parameters"});
    if (ParseTasksParameters() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_085);
        return RET_FAILURE;
    }
    LogMessage(MSG_DEBUG_PARSING, {"Parse the testcases parameters"});
    if (ParseTestcasesParameters() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_085);
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_PARSING, {"Print JSON parameter after parsing"});
    PrintJson();
    return RET_SUCCESS;
}

Tasks_Parameters_t InputParser::GetTasksParameters()
{
    return m_tasks_param;
}

Testcases_Parameters_t InputParser::GetTestcasesParameters()
{
    return m_testcases_param;
}

void InputParser::PrintJsonParameters( const bool & info_n_debug, const std::string & base_name, const std::string & param_type, const Json_Params_Def_t & parameters_definition, Json_Parameters_t & json_parameters, const bool & print_test_seq, const uint & level )
{
    if (level == 0)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, param_type + " parameters:");
    }
    else
    {
        std::string tab;
        for (uint i=0; i<level; i++)
        {
            tab += "\t";
        }
        LogMessageIf(info_n_debug, MSG_GEN_018, tab + " - " + param_type + ":");
    }
    for (const auto & json_val_def : parameters_definition)
    {
        for (auto & json_param : json_parameters)
        {
            if (json_val_def.name == json_param.first)
            {
                std::string tab = "\t";
                for (uint i=0; i<level; i++)
                {
                    tab += "\t";
                }
                auto msg = tab + "- " + json_val_def.name + ": ";
                if (json_val_def.node_type == JSON_NODE_VALUE)
                {
                    switch (json_val_def.typeId)
                    {
                        case TYPE_ID_INT:       msg += std::to_string(TestcaseParamCast<int>        (json_param.second)); break;
                        case TYPE_ID_UINT:      msg += std::to_string(TestcaseParamCast<uint>       (json_param.second)); break;
                        case TYPE_ID_UINT64_T:  msg += std::to_string(TestcaseParamCast<uint64_t>   (json_param.second)); break;
                        case TYPE_ID_FLOAT:     msg += std::to_string(TestcaseParamCast<float>      (json_param.second)); break;
                        case TYPE_ID_DOUBLE:    msg += std::to_string(TestcaseParamCast<double>     (json_param.second)); break;
                        case TYPE_ID_BOOL:      msg +=      BoolToStr(TestcaseParamCast<bool>       (json_param.second)); break;
                        case TYPE_ID_STRING:    msg +=                TestcaseParamCast<std::string>(json_param.second);  break;
                        default: break;
                    }
                }
                else if (json_val_def.node_type == JSON_NODE_ARRAY)
                {
                    switch (json_val_def.typeId)
                    {
                        case TYPE_ID_STRING:    msg +=  StrVectToStr(TestcaseParamCast<std::vector<std::string>>(json_param.second), ", ");  break;
                        case TYPE_ID_TEST_SEQUENCE:
                            if (print_test_seq)
                            {
                                if (StrMatchNoCase(base_name, MMIO_MEMBER.name))
                                {
                                    PrintMmioTestSequence(info_n_debug, TestcaseParamCast<std::vector<Mmio_Test_Sequence_Parameters_t>>   (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, DMA_MEMBER.name))
                                {
                                    PrintDMATestSequence(info_n_debug, TestcaseParamCast<std::vector<DMA_Test_Sequence_Parameters_t>>   (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, P2P_CARD_MEMBER.name))
                                {
                                    PrintP2PCardTestSequence(info_n_debug, TestcaseParamCast<std::vector<P2P_Card_Test_Sequence_Parameters_t>>   (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, P2P_NVME_MEMBER.name))
                                {
                                    PrintP2PNvmeTestSequence(info_n_debug, TestcaseParamCast<std::vector<P2P_Nvme_Test_Sequence_Parameters_t>>   (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, MEMORY_MEMBER.name))
                                {
                                    PrintMemoryTestSequence(info_n_debug, TestcaseParamCast<std::vector<Memory_Test_Sequence_Parameters_t>>(json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, POWER_MEMBER.name))
                                {
                                    PrintPowerTestSequence(info_n_debug, TestcaseParamCast<std::vector<Power_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, GT_MAC_MEMBER.name))
                                {
                                    PrintGTMACTestSequence(info_n_debug, TestcaseParamCast<std::vector<GTMAC_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, GT_LPBK_MEMBER.name))
                                {
                                    PrintGTLpbkTestSequence(info_n_debug, TestcaseParamCast<std::vector<GTLpbk_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, GT_PRBS_MEMBER.name))
                                {
                                    PrintGTPrbsTestSequence(info_n_debug, TestcaseParamCast<std::vector<GTPrbs_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, GTF_PRBS_MEMBER.name))
                                {
                                    PrintMultiGTPrbsTestSequence(info_n_debug, TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, GTM_PRBS_MEMBER.name))
                                {
                                    PrintMultiGTPrbsTestSequence(info_n_debug, TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                                else if (StrMatchNoCase(base_name, GTYP_PRBS_MEMBER.name))
                                {
                                    PrintMultiGTPrbsTestSequence(info_n_debug, TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>> (json_param.second));
                                }
                            }
                            break;
                        default: break;
                    }
                }
                // Test sequence is printing depending on the base name
                if (json_val_def.typeId != TYPE_ID_TEST_SEQUENCE)
                {
                    LogMessageIf(info_n_debug, MSG_GEN_018, msg);
                }
            }
        }
    }
    if (StrMatchNoCase(base_name, DEVICE_MGMT_MEMBER.name))
    {
        for (const auto & source : m_tasks_param.device_mgmt.sensor)
        {
            LogMessageIf(info_n_debug, MSG_GEN_018, "\t - Sensor " + source.id + " configuration:");
            LogMessageIf(info_n_debug, MSG_GEN_018, "\t\t - " + SENSOR_ID_MEMBER.name   + ": " + source.id);
            LogMessageIf(info_n_debug, MSG_GEN_018, "\t\t - " + SENSOR_TYPE_MEMBER.name + ": " + source.type);
            PrintSensorLimit(info_n_debug, "Warning", source.warning_threshold);
            PrintSensorLimit(info_n_debug, "Error",   source.error_threshold);
            PrintSensorLimit(info_n_debug, "Abort",   source.abort_threshold);
        }
    }
}

void InputParser::PrintJsonTestSeqAndResult( const bool & info_n_debug, const std::string & base_name, const Json_Params_Def_t & parameters_definition, Json_Parameters_t & json_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    for (const auto & json_val_def : parameters_definition)
    {
        for (auto & json_param : json_parameters)
        {
            if ((json_val_def.name == json_param.first) && (json_val_def.typeId == TYPE_ID_TEST_SEQUENCE))
            {
                if (base_name == MMIO_MEMBER.name)
                {
                    PrintMmioTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<Mmio_Test_Sequence_Parameters_t>>(json_param.second), test_it_results);
                }
                else if (base_name == DMA_MEMBER.name)
                {
                    PrintDMATestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<DMA_Test_Sequence_Parameters_t>>(json_param.second), test_it_results);
                }
                else if (base_name == P2P_CARD_MEMBER.name)
                {
                    PrintP2PCardTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<P2P_Card_Test_Sequence_Parameters_t>>(json_param.second), test_it_results);
                }
                else if (base_name == P2P_NVME_MEMBER.name)
                {
                    PrintP2PNvmeTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<P2P_Nvme_Test_Sequence_Parameters_t>>(json_param.second), test_it_results);
                }
                else if (base_name == MEMORY_MEMBER.name)
                {
                    PrintMemoryMCTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<Memory_Test_Sequence_Parameters_t>>(json_param.second), test_it_results);
                }
                else if (base_name == POWER_MEMBER.name)
                {
                    PrintPowerTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<Power_Test_Sequence_Parameters_t>>(json_param.second), test_it_results);
                }
            }
        }
    }
}

void InputParser::PrintJsonTestSeqAndResultThreads( const bool & info_n_debug, const std::string & base_name, const Json_Params_Def_t & parameters_definition, Json_Parameters_t & json_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map )
{
    for (const auto & json_val_def : parameters_definition)
    {
        for (auto & json_param : json_parameters)
        {
            if ((json_val_def.name == json_param.first) && (json_val_def.typeId == TYPE_ID_TEST_SEQUENCE))
            {
                // Memory SC test runs multiple threads with same configuration
                // GT_MAC/LPBK test runs in one thread but for multiple lanes
                if (base_name == MEMORY_MEMBER.name)
                {
                    PrintMemorySCTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<Memory_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
                else if (base_name == GT_MAC_MEMBER.name)
                {
                    PrintGTMACTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<GTMAC_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
                else if (base_name == GT_LPBK_MEMBER.name)
                {
                    PrintGTLpbkTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<GTLpbk_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
                else if (base_name == GT_PRBS_MEMBER.name)
                {
                    PrintGTPrbsTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<GTPrbs_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
                else if (base_name == GTF_PRBS_MEMBER.name)
                {
                    PrintMultiGTPrbsTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
                else if (base_name == GTM_PRBS_MEMBER.name)
                {
                    PrintMultiGTPrbsTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
                else if (base_name == GTYP_PRBS_MEMBER.name)
                {
                    PrintMultiGTPrbsTestSequenceAndResult(info_n_debug, TestcaseParamCast<std::vector<MultiGTPrbs_Test_Sequence_Parameters_t>>(json_param.second), test_it_results_map);
                }
            }
        }
    }
}

void InputParser::SetLogHeader( const std::string & log_header )
{
    m_log_header = log_header;
}

bool InputParser::PrintGuide( const std::string & testcase_task_name )
{
    // Check testcase_task_name in supported values
    std::set<std::string> supported_testcase_task_names;
    for (const auto & task_def : TASKS_PARAMETERS_DEF)
    {
        if (task_def.hidden == HIDDEN_FALSE)
        {
            supported_testcase_task_names.insert(task_def.name);
        }
    }

    for (const auto & tescase_def : TESTCASES_PARAMETERS_DEF)
    {
        if (tescase_def.hidden == HIDDEN_FALSE)
        {
            supported_testcase_task_names.insert(tescase_def.name);
        }
    }

    if (CheckStringInSet("\"-g\" option", testcase_task_name, supported_testcase_task_names, EMPTY_SET) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Start printing the guide for the requested testcase_task_name
    auto ret = RET_SUCCESS;

    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"Displaying xbtest options available for the HW design running on targeted card"});
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});

    // Check if the testcase is supported
    if (StrMatchNoCase(testcase_task_name, GT_MAC_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableGTMACs().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, GT_LPBK_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableGTLpbks().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, GT_PRBS_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableGTPrbss().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, GTF_PRBS_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableGTFPrbss().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, GTM_PRBS_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableGTMPrbss().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, GTYP_PRBS_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableGTYPPrbss().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, MMIO_MEMBER.name))
    {
        if (!(this->m_xbtest_sw_config->GetIsMmioTestSupported()))
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, DMA_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableBoardMemories().empty() || !(this->m_xbtest_sw_config->GetIsDmaSupported()))
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, P2P_CARD_MEMBER.name) || StrMatchNoCase(testcase_task_name, P2P_NVME_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAvailableBoardMemories().empty() || !(this->m_xbtest_sw_config->GetIsP2PSupported()))
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }
    if (StrMatchNoCase(testcase_task_name, MEMORY_MEMBER.name))
    {
        if (this->m_xbtest_sw_config->GetAllAvailableMemories().empty())
        {
            LogMessage(MSG_ITF_077, {"The HW design running on targeted card does not support " + testcase_task_name + " " + TESTCASES_MEMBER.name});
            return ret;
        }
    }

    for (const auto & task_def : TASKS_PARAMETERS_DEF)
    {
        if (StrMatchNoCase(testcase_task_name, task_def.name))
        {
            LogMessage(MSG_ITF_077, {"The following " + m_content_name + " options are available in " + task_def.name + " " + TASKS_MEMBER.name + ":"});
        }
    }
    for (const auto & tescase_def : TESTCASES_PARAMETERS_DEF)
    {
        if (StrMatchNoCase(testcase_task_name, tescase_def.name))
        {
            LogMessage(MSG_ITF_077, {"The following " + m_content_name + " options are available in " + tescase_def.name + " " + TESTCASES_MEMBER.name + ":"});
        }
    }
    // Parameter guide
    if (StrMatchNoCase(testcase_task_name, DEVICE_MGMT_MEMBER.name))
    {
        PrintJsonParamsGuide("\t", TASKS_DEVICE_MGMT_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, DEBUG_MEMBER.name))
    {
        PrintJsonParamsGuide("\t", TASKS_DEBUG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, MMIO_MEMBER.name))
    {
        LogMessage(MSG_ITF_077, {"\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t", MMIO_GLOBAL_CONFIG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, DMA_MEMBER.name))
    {
        LogMessage(MSG_ITF_077, {"\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t", DMA_GLOBAL_CONFIG_PARAMETERS_DEF);

        LogMessage(MSG_ITF_077, {"\t- " + StrVectToStr(QuoteStrVect(this->m_xbtest_sw_config->GetAvailableBoardMemories()), "/") + " memory options:"});
        PrintJsonParamsGuide("\t\t", DMA_MEMORY_TYPE_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, P2P_CARD_MEMBER.name))
    {
        LogMessage(MSG_ITF_077, {"\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t", P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF);

        LogMessage(MSG_ITF_077, {"\t- " + StrVectToStr(QuoteStrVect(this->m_xbtest_sw_config->GetAvailableBoardMemories()), "/") + " memory options:"});
        PrintJsonParamsGuide("\t\t", P2P_CARD_MEMORY_TYPE_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, P2P_NVME_MEMBER.name))
    {
        LogMessage(MSG_ITF_077, {"\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t", P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF);

        LogMessage(MSG_ITF_077, {"\t- " + StrVectToStr(QuoteStrVect(this->m_xbtest_sw_config->GetAvailableBoardMemories()), "/") + " memory options:"});
        PrintJsonParamsGuide("\t\t", P2P_NVME_MEMORY_TYPE_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, MEMORY_MEMBER.name))
    {
        LogMessage(MSG_ITF_077, {"\t- " + StrVectToStr(QuoteStrVect(this->m_xbtest_sw_config->GetAllAvailableMemories()), "/") + " memory options:"});
        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});

        PrintJsonParamsGuide("\t\t\t", this->m_memory_global_config_parameters_def_full);
    }
    else if (StrMatchNoCase(testcase_task_name, POWER_MEMBER.name))
    {
        LogMessage(MSG_ITF_077, {"\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t", this->m_power_global_config_parameters_def_full);
    }
    else if (StrMatchNoCase(testcase_task_name, GT_MAC_MEMBER.name))
    {
        std::vector<std::string> quoted_str;
        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMACs())
        {
            quoted_str.emplace_back("\"" + std::to_string(gt_index) + "\"");
        }
        LogMessage(MSG_ITF_077, {"\t- Options for GT index(es) " + StrVectToStr(quoted_str, "/") + ":"});

        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t\t", GT_MAC_GLOBAL_CFG_PARAMETERS_DEF);

        // Lane configurations are hidden parameters

        LogMessage(MSG_ITF_077, {"\t\t- " + LANE_CONFIG_MEMBER.name + " options:"});
        std::vector<std::string> quoted_str2;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            quoted_str2.emplace_back(std::to_string(lane_idx));
        }
        LogMessage(MSG_ITF_077, {"\t\t\t- " + StrVectToStr(quoted_str2, "/") + " lanes options:"});
        PrintJsonParamsGuide("\t\t\t\t", GT_MAC_LANE_CFG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, GT_LPBK_MEMBER.name))
    {
        std::vector<std::string> quoted_str;
        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTLpbks())
        {
            quoted_str.emplace_back("\"" + std::to_string(gt_index) + "\"");
        }
        LogMessage(MSG_ITF_077, {"\t- Options for GT index(es) " + StrVectToStr(quoted_str, "/") + ":"});

        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t\t", GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF);

        // Lane configurations are hidden parameters

        LogMessage(MSG_ITF_077, {"\t\t- " + LANE_CONFIG_MEMBER.name + " options:"});
        std::vector<std::string> quoted_str2;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            quoted_str2.emplace_back(std::to_string(lane_idx));
        }
        LogMessage(MSG_ITF_077, {"\t\t\t- " + StrVectToStr(quoted_str2, "/") + " lanes options:"});
        PrintJsonParamsGuide("\t\t\t\t", GT_LPBK_LANE_CFG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, GT_PRBS_MEMBER.name))
    {
        std::vector<std::string> quoted_str;
        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTPrbss())
        {
            quoted_str.emplace_back("\"" + std::to_string(gt_index) + "\"");
        }
        LogMessage(MSG_ITF_077, {"\t- Options for GT index(es) " + StrVectToStr(quoted_str, "/") + ":"});

        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t\t", GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF);

        // Lane configurations are hidden parameters

        LogMessage(MSG_ITF_077, {"\t\t- " + LANE_CONFIG_MEMBER.name + " options:"});
        std::vector<std::string> quoted_str2;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            quoted_str2.emplace_back(std::to_string(lane_idx));
        }
        LogMessage(MSG_ITF_077, {"\t\t\t- " + StrVectToStr(quoted_str2, "/") + " lanes options:"});
        PrintJsonParamsGuide("\t\t\t\t", GT_PRBS_LANE_CFG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, GTF_PRBS_MEMBER.name))
    {
        std::vector<std::string> quoted_str;
        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTFPrbss())
        {
            quoted_str.emplace_back("\"" + std::to_string(gt_index) + "\"");
        }
        LogMessage(MSG_ITF_077, {"\t- Options for GTF index(es) " + StrVectToStr(quoted_str, "/") + ":"});

        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t\t", MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF);

        // Lane configurations are hidden parameters

        LogMessage(MSG_ITF_077, {"\t\t- " + LANE_CONFIG_MEMBER.name + " options:"});
        std::vector<std::string> quoted_str2;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            quoted_str2.emplace_back(std::to_string(lane_idx));
        }
        LogMessage(MSG_ITF_077, {"\t\t\t- " + StrVectToStr(quoted_str2, "/") + " lanes options:"});
        PrintJsonParamsGuide("\t\t\t\t", GT_PRBS_LANE_CFG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, GTM_PRBS_MEMBER.name))
    {
        std::vector<std::string> quoted_str;
        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMPrbss())
        {
            quoted_str.emplace_back("\"" + std::to_string(gt_index) + "\"");
        }
        LogMessage(MSG_ITF_077, {"\t- Options for GTM index(es) " + StrVectToStr(quoted_str, "/") + ":"});

        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t\t", MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF);

        // Lane configurations are hidden parameters

        LogMessage(MSG_ITF_077, {"\t\t- " + LANE_CONFIG_MEMBER.name + " options:"});
        std::vector<std::string> quoted_str2;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            quoted_str2.emplace_back(std::to_string(lane_idx));
        }
        LogMessage(MSG_ITF_077, {"\t\t\t- " + StrVectToStr(quoted_str2, "/") + " lanes options:"});
        PrintJsonParamsGuide("\t\t\t\t", GT_PRBS_LANE_CFG_PARAMETERS_DEF);
    }
    else if (StrMatchNoCase(testcase_task_name, GTYP_PRBS_MEMBER.name))
    {
        std::vector<std::string> quoted_str;
        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTYPPrbss())
        {
            quoted_str.emplace_back("\"" + std::to_string(gt_index) + "\"");
        }
        LogMessage(MSG_ITF_077, {"\t- Options for GTYP index(es) " + StrVectToStr(quoted_str, "/") + ":"});

        LogMessage(MSG_ITF_077, {"\t\t- " + GLOBAL_CONFIG_MEMBER.name + " options:"});
        PrintJsonParamsGuide("\t\t\t", MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF);

        // Lane configurations are hidden parameters

        LogMessage(MSG_ITF_077, {"\t\t- " + LANE_CONFIG_MEMBER.name + " options:"});
        std::vector<std::string> quoted_str2;
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            quoted_str2.emplace_back(std::to_string(lane_idx));
        }
        LogMessage(MSG_ITF_077, {"\t\t\t- " + StrVectToStr(quoted_str2, "/") + " lanes options:"});
        PrintJsonParamsGuide("\t\t\t\t", GT_PRBS_LANE_CFG_PARAMETERS_DEF);
    }
    LogMessage(MSG_ITF_077, {"The default value of each option is displayed at the beginning of the testcase and in xbtest UG"});

    // Test sequence guide
    // JSON example
    if (StrMatchNoCase(testcase_task_name, MMIO_MEMBER.name))
    {
        PrintMmioTestSequenceGuide();
        PrintMmioJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, DMA_MEMBER.name))
    {
        PrintDMATestSequenceGuide();
        PrintDMAJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, P2P_CARD_MEMBER.name))
    {
        PrintP2PCardTestSequenceGuide();
        PrintP2PCardJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, P2P_NVME_MEMBER.name))
    {
        PrintP2PNvmeTestSequenceGuide();
        PrintP2P2NvmJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, MEMORY_MEMBER.name))
    {
        PrintMemoryTestSequenceGuide();
        PrintMemoryJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, POWER_MEMBER.name))
    {
        PrintPowerTestSequenceGuide();
        PrintPowerJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, GT_MAC_MEMBER.name))
    {
        PrintGTMACTestSequenceGuide();
        PrintGTMACJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, GT_LPBK_MEMBER.name))
    {
        PrintGTLpbkTestSequenceGuide();
        PrintGTLpbkJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, GT_PRBS_MEMBER.name))
    {
        PrintGTPrbsTestSequenceGuide();
        PrintGTPrbsJSONExample();
    }
    else if (StrMatchNoCase(testcase_task_name, GTF_PRBS_MEMBER.name))
    {
        PrintMultiGTPrbsTestSequenceGuide(GTF_PRBS_MEMBER.name);
        PrintMultiGTPrbsJSONExample(GTF_PRBS_MEMBER.name);
    }
    else if (StrMatchNoCase(testcase_task_name, GTM_PRBS_MEMBER.name))
    {
        PrintMultiGTPrbsTestSequenceGuide(GTM_PRBS_MEMBER.name);
        PrintMultiGTPrbsJSONExample(GTM_PRBS_MEMBER.name);
    }
    else if (StrMatchNoCase(testcase_task_name, GTYP_PRBS_MEMBER.name))
    {
        PrintMultiGTPrbsTestSequenceGuide(GTYP_PRBS_MEMBER.name);
        PrintMultiGTPrbsJSONExample(GTYP_PRBS_MEMBER.name);
    }
    else if (StrMatchNoCase(testcase_task_name, DEVICE_MGMT_MEMBER.name))
    {
        ret |= PrintDeviceMmgmtSensorGuide();
        PrintDeviceMmgmtJSONExample();
    }

    return ret;
}

Json_Params_Def_t InputParser::GetPowerGlobalParametersDef()
{
    return this->m_power_global_config_parameters_def_full;
}
// single_axi_thread is added to the definition only if the feature is present in HW design (NUM_AXI_THREAD in USER_METADATA)
Json_Params_Def_t InputParser::GetMemoryTagChanParametersDef( const std::string & memory_name )
{
    auto json_params_def = MEMORY_TAG_CHAN_PARAMETERS_DEF;
    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        if ((memory.name == memory_name) && (memory.num_axi_thread > 1))
        {
            json_params_def.emplace_back(SINGLE_AXI_THREAD_MEMBER);
        }
        if (this->m_xbtest_sw_config->IsMemoryPatternSupported())
        {
            json_params_def.emplace_back(PATTERN_MEMBER);
        }
    }
    return json_params_def;
}

Json_Params_Def_t InputParser::GetMemoryGlobalParametersDef( const std::string & memory_name )
{
    auto json_params_def = MEMORY_GLOBAL_CONFIG_PARAMETERS_DEF;
    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        if ((memory.name == memory_name) && (memory.num_axi_thread > 1))
        {
            json_params_def.emplace_back(SINGLE_AXI_THREAD_MEMBER);
        }
        if (this->m_xbtest_sw_config->IsMemoryPatternSupported())
        {
            json_params_def.emplace_back(PATTERN_MEMBER);
        }
    }
    return json_params_def;
}

std::string InputParser::TestIterationResultToString( const TestIterationResult & Result )
{
    switch (Result)
    {
        case TITR_DISABLED: return "DISABLED";   break;
        case TITR_PASSED:   return "PASSED";     break;
        case TITR_FAILED:   return "FAILED";     break;
        case TITR_ABORTED:  return "ABORTED";    break;
        default:            return "NOT_TESTED"; break;
    }
}

void InputParser::AppendJsonDefinition( const std::vector<std::string> & base_title, const Json_Params_Def_t & parameters_definition, const Hidden_t & top_hidden, const bool & visible_only, Json_Definition_t & json_definition )
{
    auto node_title = base_title;
    if ((top_hidden == HIDDEN_FALSE) || ((!visible_only) && (top_hidden == HIDDEN_TRUE)))
    {
        for (const auto & json_val_def: parameters_definition)
        {
            if ((json_val_def.hidden == HIDDEN_FALSE) || ((!visible_only) && (json_val_def.hidden == HIDDEN_TRUE)))
            {
                node_title.emplace_back(json_val_def.name);
                json_definition.insert( Definition_t(node_title, json_val_def.node_type) );
                node_title.pop_back();
            }
        }
    }
}

Json_Definition_t InputParser::GetJsonDefinition( const bool & visible_only )
{
    Json_Definition_t json_definition;
    // base parameterS
    AppendJsonDefinition({}, BASE_PARAMETERS_DEF, HIDDEN_FALSE, visible_only, json_definition);

    // "tasks" parameters
    AppendJsonDefinition({TASKS_MEMBER.name}, TASKS_PARAMETERS_DEF, TASKS_MEMBER.hidden, visible_only, json_definition);
    AppendJsonDefinition({TASKS_MEMBER.name, DEVICE_MGMT_MEMBER.name},  TASKS_DEVICE_MGMT_PARAMETERS_DEF, DEVICE_MGMT_MEMBER.hidden, visible_only, json_definition);
    AppendJsonDefinition({TASKS_MEMBER.name, DEBUG_MEMBER.name},        TASKS_DEBUG_PARAMETERS_DEF, DEBUG_MEMBER.hidden, visible_only, json_definition);

    // "testcases" parameters
    AppendJsonDefinition({TESTCASES_MEMBER.name}, this->m_testcases_parameters_def_supported, TESTCASES_MEMBER.hidden, visible_only, json_definition);

    // "testcases"."mmio" parameters
    if (this->m_xbtest_sw_config->GetIsMmioTestSupported())
    {
        AppendJsonDefinition({TESTCASES_MEMBER.name, MMIO_MEMBER.name}, MMIO_SUB_PARAMETERS_DEF, MMIO_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, MMIO_MEMBER.name, GLOBAL_CONFIG_MEMBER.name}, MMIO_GLOBAL_CONFIG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
    }
    // "testcases"."dma" parameters
    if (this->m_xbtest_sw_config->GetIsDmaSupported())
    {
        AppendJsonDefinition({TESTCASES_MEMBER.name, DMA_MEMBER.name}, DMA_SUB_PARAMETERS_DEF, DMA_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, DMA_MEMBER.name, GLOBAL_CONFIG_MEMBER.name}, DMA_GLOBAL_CONFIG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            if ((MEMORY_TYPE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (MEMORY_TYPE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, DMA_MEMBER.name, MEMORY_TYPE_CONFIG_MEMBER.name, memory.name}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, DMA_MEMBER.name, MEMORY_TYPE_CONFIG_MEMBER.name, memory.name}, DMA_MEMORY_TYPE_PARAMETERS_DEF, MEMORY_TYPE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."p2p_card" parameters
    if (this->m_xbtest_sw_config->GetIsP2PSupported())
    {
        AppendJsonDefinition({TESTCASES_MEMBER.name, P2P_CARD_MEMBER.name}, P2P_CARD_SUB_PARAMETERS_DEF, P2P_CARD_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, P2P_CARD_MEMBER.name, GLOBAL_CONFIG_MEMBER.name}, P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            if ((MEMORY_TYPE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (MEMORY_TYPE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, P2P_CARD_MEMBER.name, MEMORY_TYPE_CONFIG_MEMBER.name, memory.name}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, P2P_CARD_MEMBER.name, MEMORY_TYPE_CONFIG_MEMBER.name, memory.name}, P2P_CARD_MEMORY_TYPE_PARAMETERS_DEF, MEMORY_TYPE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."p2p_nvme" parameters
    if (this->m_xbtest_sw_config->GetIsP2PSupported())
    {
        AppendJsonDefinition({TESTCASES_MEMBER.name, P2P_NVME_MEMBER.name}, P2P_NVME_SUB_PARAMETERS_DEF, P2P_NVME_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, P2P_NVME_MEMBER.name, GLOBAL_CONFIG_MEMBER.name}, P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            if ((MEMORY_TYPE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (MEMORY_TYPE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, P2P_NVME_MEMBER.name, MEMORY_TYPE_CONFIG_MEMBER.name, memory.name}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, P2P_NVME_MEMBER.name, MEMORY_TYPE_CONFIG_MEMBER.name, memory.name}, P2P_NVME_MEMORY_TYPE_PARAMETERS_DEF, MEMORY_TYPE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."memory" parameters
    if (!(this->m_xbtest_sw_config->GetAllAvailableMemories().empty()))
    {
        // json_definition.insert( Definition_t({TESTCASES_MEMBER.name, MEMORY_MEMBER.name}, JSON_NODE_OBJECT) );

        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name}, JSON_NODE_OBJECT) );
            if (memory.type == SINGLE_CHANNEL)
            {
                AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name}, MEMORY_SC_SUB_PARAMETERS_DEF, MEMORY_MEMBER.hidden, visible_only, json_definition);
                AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, GLOBAL_CONFIG_MEMBER.name}, GetMemoryGlobalParametersDef(memory.name), GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
                if (this->m_xbtest_sw_config->IsMemoryPatternSupported())
                {
                    AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, GLOBAL_CONFIG_MEMBER.name, PATTERN_MEMBER.name}, MEMORY_PATTERN_DEF, PATTERN_MEMBER.hidden, visible_only, json_definition);
                }
                for (const auto & tag : this->m_xbtest_sw_config->GetAvailableSpTag(memory.name))
                {
                    if ((MEMORY_TAG_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (MEMORY_TAG_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
                    {
                        json_definition.insert( Definition_t({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, MEMORY_TAG_CONFIG_MEMBER.name, tag}, JSON_NODE_OBJECT) );
                    }
                    AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, MEMORY_TAG_CONFIG_MEMBER.name, tag}, GetMemoryTagChanParametersDef(memory.name), MEMORY_TAG_CONFIG_MEMBER.hidden, visible_only, json_definition);
                    if (this->m_xbtest_sw_config->IsMemoryPatternSupported())
                    {
                        AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, MEMORY_TAG_CONFIG_MEMBER.name, tag, PATTERN_MEMBER.name}, MEMORY_PATTERN_DEF, PATTERN_MEMBER.hidden, visible_only, json_definition);
                    }
                }
            }
            else
            {
                AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name}, MEMORY_MC_SUB_PARAMETERS_DEF, MEMORY_MEMBER.hidden, visible_only, json_definition);
                AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, GLOBAL_CONFIG_MEMBER.name}, GetMemoryGlobalParametersDef(memory.name), GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
                AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, GLOBAL_CONFIG_MEMBER.name, PATTERN_MEMBER.name}, MEMORY_PATTERN_DEF, PATTERN_MEMBER.hidden, visible_only, json_definition);
                for (uint ch_idx = 0; ch_idx < this->m_xbtest_sw_config->GetMemoryNumChannel(memory.name); ch_idx++)
                {
                    if ((MEMORY_CHANNEL_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (MEMORY_CHANNEL_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
                    {
                        json_definition.insert( Definition_t({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, MEMORY_CHANNEL_CONFIG_MEMBER.name, std::to_string(ch_idx)}, JSON_NODE_OBJECT) );
                    }
                    AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, MEMORY_CHANNEL_CONFIG_MEMBER.name, std::to_string(ch_idx)}, GetMemoryTagChanParametersDef(memory.name), MEMORY_CHANNEL_CONFIG_MEMBER.hidden, visible_only, json_definition);
                    if (this->m_xbtest_sw_config->IsMemoryPatternSupported())
                    {
                        AppendJsonDefinition({TESTCASES_MEMBER.name, MEMORY_MEMBER.name, memory.name, MEMORY_CHANNEL_CONFIG_MEMBER.name, std::to_string(ch_idx), PATTERN_MEMBER.name}, MEMORY_PATTERN_DEF, PATTERN_MEMBER.hidden, visible_only, json_definition);
                    }
                }
            }
        }
    }
    // "testcases"."power" parameters
    if (!this->m_xbtest_sw_config->GetAvailablePowerSLRs().empty())
    {
        AppendJsonDefinition({TESTCASES_MEMBER.name, POWER_MEMBER.name}, POWER_SUB_PARAMETERS_DEF, POWER_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, POWER_MEMBER.name, GLOBAL_CONFIG_MEMBER.name}, this->m_power_global_config_parameters_def_full, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
    }

    // "testcases"."gt_mac" parameters
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMACs())
    {
        if ((GT_MAC_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (GT_MAC_MEMBER.hidden == HIDDEN_TRUE)))
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GT_MAC_MEMBER.name, std::to_string(gt_index)}, JSON_NODE_OBJECT) );
        }
        AppendJsonDefinition({TESTCASES_MEMBER.name, GT_MAC_MEMBER.name, std::to_string(gt_index)}, GT_INDEX_PARAMETERS_DEF, GT_MAC_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, GT_MAC_MEMBER.name, std::to_string(gt_index), GLOBAL_CONFIG_MEMBER.name},  GT_MAC_GLOBAL_CFG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if ((LANE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (LANE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GT_MAC_MEMBER.name, std::to_string(gt_index), LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, GT_MAC_MEMBER.name, std::to_string(gt_index), LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, GT_MAC_LANE_CFG_PARAMETERS_DEF, LANE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."gt_lpbk" parameters
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTLpbks())
    {
        if ((GT_LPBK_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (GT_LPBK_MEMBER.hidden == HIDDEN_TRUE)))
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GT_LPBK_MEMBER.name, std::to_string(gt_index)}, JSON_NODE_OBJECT) );
        }
        AppendJsonDefinition({TESTCASES_MEMBER.name, GT_LPBK_MEMBER.name, std::to_string(gt_index)}, GT_INDEX_PARAMETERS_DEF, GT_LPBK_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, GT_LPBK_MEMBER.name, std::to_string(gt_index), GLOBAL_CONFIG_MEMBER.name},  GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if ((LANE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (LANE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GT_LPBK_MEMBER.name, std::to_string(gt_index), LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, GT_LPBK_MEMBER.name, std::to_string(gt_index), LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, GT_LPBK_LANE_CFG_PARAMETERS_DEF, LANE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."gt_prbs" parameters
    for (const auto & node_name : this->m_supported_gt_prbs_node_names)
    {
        if ((GT_PRBS_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (GT_PRBS_MEMBER.hidden == HIDDEN_TRUE)))
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GT_PRBS_MEMBER.name, node_name}, JSON_NODE_OBJECT) );
        }
        AppendJsonDefinition({TESTCASES_MEMBER.name, GT_PRBS_MEMBER.name, node_name}, GT_INDEX_PARAMETERS_DEF, GT_PRBS_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, GT_PRBS_MEMBER.name, node_name, GLOBAL_CONFIG_MEMBER.name},  GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if ((LANE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (LANE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GT_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, GT_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, GT_PRBS_LANE_CFG_PARAMETERS_DEF, LANE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."gtf_prbs" parameters
    for (const auto & node_name : this->m_supported_gtf_prbs_node_names)
    {
        if ((GTF_PRBS_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (GTF_PRBS_MEMBER.hidden == HIDDEN_TRUE)))
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GTF_PRBS_MEMBER.name, node_name}, JSON_NODE_OBJECT) );
        }
        AppendJsonDefinition({TESTCASES_MEMBER.name, GTF_PRBS_MEMBER.name, node_name}, GT_INDEX_PARAMETERS_DEF, GTF_PRBS_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, GTF_PRBS_MEMBER.name, node_name, GLOBAL_CONFIG_MEMBER.name},  MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if ((LANE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (LANE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GTF_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, GTF_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, GT_PRBS_LANE_CFG_PARAMETERS_DEF, LANE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."gtm_prbs" parameters
    for (const auto & node_name : this->m_supported_gtm_prbs_node_names)
    {
        if ((GTM_PRBS_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (GTM_PRBS_MEMBER.hidden == HIDDEN_TRUE)))
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GTM_PRBS_MEMBER.name, node_name}, JSON_NODE_OBJECT) );
        }
        AppendJsonDefinition({TESTCASES_MEMBER.name, GTM_PRBS_MEMBER.name, node_name}, GT_INDEX_PARAMETERS_DEF, GTM_PRBS_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, GTM_PRBS_MEMBER.name, node_name, GLOBAL_CONFIG_MEMBER.name},  MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if ((LANE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (LANE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GTM_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, GTM_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, GT_PRBS_LANE_CFG_PARAMETERS_DEF, LANE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    // "testcases"."gtyp_prbs" parameters
    for (const auto & node_name : this->m_supported_gtyp_prbs_node_names)
    {
        if ((GTYP_PRBS_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (GTYP_PRBS_MEMBER.hidden == HIDDEN_TRUE)))
        {
            json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GTYP_PRBS_MEMBER.name, node_name}, JSON_NODE_OBJECT) );
        }
        AppendJsonDefinition({TESTCASES_MEMBER.name, GTYP_PRBS_MEMBER.name, node_name}, GT_INDEX_PARAMETERS_DEF, GTYP_PRBS_MEMBER.hidden, visible_only, json_definition);
        AppendJsonDefinition({TESTCASES_MEMBER.name, GTYP_PRBS_MEMBER.name, node_name, GLOBAL_CONFIG_MEMBER.name},  MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, GLOBAL_CONFIG_MEMBER.hidden, visible_only, json_definition);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            if ((LANE_CONFIG_MEMBER.hidden == HIDDEN_FALSE) || ((!visible_only) && (LANE_CONFIG_MEMBER.hidden == HIDDEN_TRUE)))
            {
                json_definition.insert( Definition_t({TESTCASES_MEMBER.name, GTYP_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, JSON_NODE_OBJECT) );
            }
            AppendJsonDefinition({TESTCASES_MEMBER.name, GTYP_PRBS_MEMBER.name, node_name, LANE_CONFIG_MEMBER.name, std::to_string(lane_idx)}, GT_PRBS_LANE_CFG_PARAMETERS_DEF, LANE_CONFIG_MEMBER.hidden, visible_only, json_definition);
        }
    }
    return json_definition;
}

bool InputParser::CheckBothProvided( const std::vector<std::string> & node_title_in, const std::string & name_1, const bool & exists_1, const std::string & name_2, const bool & exists_2 )
{
    if (exists_1 != exists_2)
    {
        LogMessage(MSG_ITF_127, {name_1 + " and " + name_2 + " in " + StrVectToStr(node_title_in, ".")});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool InputParser::CheckWriteReadOnly( const std::string & direction, const std::vector<std::string> & node_title_in, const std::string & mode )
{
    auto ret = RET_SUCCESS;
    if (direction == "write")
    {
        if ((!StrMatchNoCase(mode, ALTERNATE_WR_RD)) && (!StrMatchNoCase(mode, ONLY_WR)) && (!StrMatchNoCase(mode, SIMULTANEOUS_WR_RD)))
        {
            ret = RET_FAILURE;
        }
    }
    else
    {
        if ((!StrMatchNoCase(mode, ALTERNATE_WR_RD)) && (!StrMatchNoCase(mode, ONLY_RD)) && (!StrMatchNoCase(mode, SIMULTANEOUS_WR_RD)))
        {
            ret = RET_FAILURE;
        }
    }
    if (ret == RET_FAILURE)
    {
        LogMessage(MSG_ITF_126, {direction, StrVectToStr(node_title_in, "."), mode});
    }
    return ret;
}

void InputParser::PrintRequiredNotFound( const std::vector<std::string> & node_title_in )
{
    LogMessage(MSG_ITF_044, {this->m_content_name, StrVectToStr(node_title_in, ".")});
}

bool InputParser::ParseSensor( const std::vector<std::string> & base_title, std::vector<Device_Mgmt_Sensor_t> & sensors )
{
    auto parse_failure = RET_SUCCESS;
    std::vector<std::string> node_title;
    sensors.clear();

    uint num_node_read = 0;
    ExtractNode(base_title, num_node_read); // Move cursor to sensors array

    auto sensors_array_size = count_json_elements();
    if (sensors_array_size == 0)
    {
        LogMessage(MSG_ITF_115, {StrVectToStr(base_title, ".")});
        parse_failure = RET_FAILURE;
    }
    else if (sensors_array_size > (gint)(MAX_SENSORS))
    {
        LogMessage(MSG_ITF_119, {std::to_string(sensors_array_size), StrVectToStr(base_title, "."), std::to_string(MAX_SENSORS)});
        parse_failure = RET_FAILURE;
    }
    std::vector<std::string> source_ids;
    for (gint j = 0; (j < sensors_array_size) && (parse_failure == RET_SUCCESS); j++) // For each element in tests array
    {
        auto base_title_id = base_title;
        base_title_id.emplace_back(std::to_string(j));

        // Check content of tests element
        read_json_element(j); // Move cursor to array element
        if (is_json_object() == 0)
        {
            LogMessage(MSG_ITF_104, {StrVectToStr(base_title_id, "."), "object"});
            parse_failure = RET_FAILURE;
            end_json_element(); // Move back cursor from array element
            break;
        }
        if (CheckMembers(base_title_id, this->m_device_mgmt_sensor_json_def, true) == RET_FAILURE)
        {
            parse_failure = RET_FAILURE;
            end_json_element(); // Move back cursor from array element
            break;
        }
        // Parse content of tests element
        auto sensor = RST_DEVICE_MGMT_SENSOR;

        // ID
        node_title = {SENSOR_ID_MEMBER.name};
        if (ExtractNodeValueStr(node_title, sensor.id) == RET_FAILURE)
        {
            parse_failure = RET_FAILURE;
            end_json_element(); // Move back cursor from array element
            break;
        }
        if (FindStringInVect(sensor.id, source_ids))
        {
            LogMessage(MSG_ITF_129, {this->m_content_name, sensor.id});
            parse_failure = RET_FAILURE;
            end_json_element(); // Move back cursor from array element
            break;
        }
        source_ids.emplace_back(sensor.id);
        // TYPE
        node_title = {SENSOR_TYPE_MEMBER.name};
        if (ExtractNodeValueStr(node_title, sensor.type) == RET_FAILURE)
        {
            parse_failure = RET_FAILURE;
            end_json_element(); // Move back cursor from array element
            break;
        }
        if (CheckStringInSet(SENSOR_TYPE_MEMBER.name, sensor.type, SUPPORTED_SENSOR_TYPES, EMPTY_SET) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // WARNING_THRESHOLD.MIN
        node_title = {SENSOR_WARNING_THRESHOLD_MEMBER.name, SENSOR_THRESHOLD_MIN_MEMBER.name};
        if (NodeExists(node_title))
        {
            if (ExtractNodeValueDouble<double>(node_title, sensor.warning_threshold.min) == RET_FAILURE)
            {
                parse_failure = RET_FAILURE;
                end_json_element(); // Move back cursor from array element
                break;
            }
            sensor.warning_threshold.min_exists = true;
        }
        // WARNING_THRESHOLD.MAX
        node_title = {SENSOR_WARNING_THRESHOLD_MEMBER.name, SENSOR_THRESHOLD_MAX_MEMBER.name};
        if (NodeExists(node_title))
        {
            if (ExtractNodeValueDouble<double>(node_title, sensor.warning_threshold.max) == RET_FAILURE)
            {
                parse_failure = RET_FAILURE;
                end_json_element(); // Move back cursor from array element
                break;
            }
            sensor.warning_threshold.max_exists = true;
        }
        // ERROR_THRESHOLD.MIN
        node_title = {SENSOR_ERROR_THRESHOLD_MEMBER.name, SENSOR_THRESHOLD_MIN_MEMBER.name};
        if (NodeExists(node_title))
        {
            if (ExtractNodeValueDouble<double>(node_title, sensor.error_threshold.min) == RET_FAILURE)
            {
                parse_failure = RET_FAILURE;
                end_json_element(); // Move back cursor from array element
                break;
            }
            sensor.error_threshold.min_exists = true;
        }
        // ERROR_THRESHOLD.MAX
        node_title = {SENSOR_ERROR_THRESHOLD_MEMBER.name, SENSOR_THRESHOLD_MAX_MEMBER.name};
        if (NodeExists(node_title))
        {
            if (ExtractNodeValueDouble<double>(node_title, sensor.error_threshold.max) == RET_FAILURE)
            {
                parse_failure = RET_FAILURE;
                end_json_element(); // Move back cursor from array element
                break;
            }
            sensor.error_threshold.max_exists = true;
        }
        // ABORT_THRESHOLD.MIN
        node_title = {SENSOR_ABORT_THRESHOLD_MEMBER.name, SENSOR_THRESHOLD_MIN_MEMBER.name};
        if (NodeExists(node_title))
        {
            if (ExtractNodeValueDouble<double>(node_title, sensor.abort_threshold.min) == RET_FAILURE)
            {
                parse_failure = RET_FAILURE;
                end_json_element(); // Move back cursor from array element
                break;
            }
            sensor.abort_threshold.min_exists = true;
        }
        // ABORT_THRESHOLD.MAX
        node_title = {SENSOR_ABORT_THRESHOLD_MEMBER.name, SENSOR_THRESHOLD_MAX_MEMBER.name};
        if (NodeExists(node_title))
        {
            if (ExtractNodeValueDouble<double>(node_title, sensor.abort_threshold.max) == RET_FAILURE)
            {
                parse_failure = RET_FAILURE;
                end_json_element(); // Move back cursor from array element
                break;
            }
            sensor.abort_threshold.max_exists = true;
        }

        sensors.emplace_back(sensor);

        end_json_element(); // Move back cursor to tests array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from tests array
    {
        end_json_element();
    }
    return parse_failure;
}

bool InputParser::ParseTasksParameters()
{
    std::string                 base_name;
    std::vector<std::string>    base_title;
    // DEVICE_MGMT
    base_name   = DEVICE_MGMT_MEMBER.name;
    base_title  = {TASKS_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_tasks_param.device_mgmt.exists = true;
        if (ParseJsonParameters(base_name, base_title, TASKS_DEVICE_MGMT_PARAMETERS_DEF, this->m_tasks_param.device_mgmt.param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
        base_title = {TASKS_MEMBER.name, base_name, SENSOR_MEMBER.name};
        if (NodeExists(base_title))
        {
            if (ParseSensor(base_title, this->m_tasks_param.device_mgmt.sensor) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
        }
    }
    // DEBUG
    base_name   = DEBUG_MEMBER.name;
    base_title  = {TASKS_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_tasks_param.debug.exists = true;
        if (ParseJsonParameters(base_name, base_title, TASKS_DEBUG_PARAMETERS_DEF, this->m_tasks_param.debug.param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    return RET_SUCCESS;
}

void InputParser::PrintJson()
{
    std::string                 base_name;
    std::vector<std::string>    node_title;

    //Print "task" device_mgmt
    base_name   = DEVICE_MGMT_MEMBER.name;
    node_title  = {TASKS_MEMBER.name, base_name};
    if (this->m_tasks_param.device_mgmt.exists)
    {
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), TASKS_DEVICE_MGMT_PARAMETERS_DEF, this->m_tasks_param.device_mgmt.param);
    }
    //Print "task" debug
    base_name   = DEBUG_MEMBER.name;
    node_title  = {TASKS_MEMBER.name, base_name};
    if (this->m_tasks_param.debug.exists)
    {
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), TASKS_DEBUG_PARAMETERS_DEF, this->m_tasks_param.debug.param);
    }
    // Print MMIO parameters
    base_name   = MMIO_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    if (this->m_testcases_param.mmio.exists)
    {
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), MMIO_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.mmio.global_config);
        node_title.pop_back();
    }
    // Print DMA parameters
    base_name   = DMA_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    if (this->m_testcases_param.dma.exists)
    {
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), DMA_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.dma.global_config);
        node_title.pop_back();

        node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
        for (auto & cfg : this->m_testcases_param.dma.memory_config)
        {
            if (cfg.second.exists)
            {
                node_title.emplace_back(this->m_xbtest_sw_config->GetMemorySettings(cfg.first).name);
                PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), DMA_MEMORY_TYPE_PARAMETERS_DEF, cfg.second.param);
                node_title.pop_back();
            }
        }
        node_title.pop_back();
    }
    // Print p2p_card parameters
    base_name   = P2P_CARD_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    if (this->m_testcases_param.p2p_card.exists)
    {
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.p2p_card.global_config);
        node_title.pop_back();

        node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
        for (auto & cfg : this->m_testcases_param.p2p_card.memory_config)
        {
            if (cfg.second.exists)
            {
                node_title.emplace_back(this->m_xbtest_sw_config->GetMemorySettings(cfg.first).name);
                PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), P2P_CARD_MEMORY_TYPE_PARAMETERS_DEF, cfg.second.param);
                node_title.pop_back();
            }
        }
        node_title.pop_back();
    }
    // Print p2p_nvme parameters
    base_name   = P2P_NVME_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    if (this->m_testcases_param.p2p_nvme.exists)
    {
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.p2p_nvme.global_config);
        node_title.pop_back();

        node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
        for (auto & cfg : this->m_testcases_param.p2p_nvme.memory_config)
        {
            if (cfg.second.exists)
            {
                node_title.emplace_back(this->m_xbtest_sw_config->GetMemorySettings(cfg.first).name);
                PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), P2P_NVME_MEMORY_TYPE_PARAMETERS_DEF, cfg.second.param);
                node_title.pop_back();
            }
        }
        node_title.pop_back();
    }

    // Print Memory parameters
    base_name   = MEMORY_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    if (this->m_testcases_param.memory.exists)
    {
        // Print memory type specific config
        for (auto & mem_cfg : this->m_testcases_param.memory.memory_config)
        {
            if (mem_cfg.second.exists)
            {
                auto memory_name = this->m_xbtest_sw_config->GetMemorySettings(mem_cfg.first).name;
                auto memory_type = this->m_xbtest_sw_config->GetMemorySettings(mem_cfg.first).type;

                node_title.emplace_back(memory_name);

                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GetMemoryGlobalParametersDef(memory_name), mem_cfg.second.global_config);
                if (!mem_cfg.second.global_config_pattern.empty())
                {
                    PrintJsonParameters(false, base_name, PATTERN_MEMBER.name, MEMORY_PATTERN_DEF, mem_cfg.second.global_config_pattern, true, 1);
                }
                node_title.pop_back();

                if (memory_type == SINGLE_CHANNEL)
                {
                    node_title.emplace_back(MEMORY_TAG_CONFIG_MEMBER.name);
                    for (auto & cfg : mem_cfg.second.memory_tag_config)
                    {
                        node_title.emplace_back(cfg.first);
                        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GetMemoryTagChanParametersDef(memory_name), cfg.second);
                        for (auto & cfg_pattern : mem_cfg.second.memory_tag_config_pattern)
                        {
                            if (cfg.first == cfg_pattern.first)
                            {
                                PrintJsonParameters(false, base_name, PATTERN_MEMBER.name, MEMORY_PATTERN_DEF, cfg_pattern.second, true, 1);
                            }
                        }
                        node_title.pop_back();
                    }
                    node_title.pop_back();
                }
                else
                {
                    node_title.emplace_back(MEMORY_CHANNEL_CONFIG_MEMBER.name);
                    for (auto & cfg : mem_cfg.second.memory_chan_config)
                    {
                        node_title.emplace_back(std::to_string(cfg.first));
                        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GetMemoryTagChanParametersDef(memory_name), cfg.second);
                        for (auto & cfg_pattern : mem_cfg.second.memory_chan_config_pattern)
                        {
                            if (cfg.first == cfg_pattern.first)
                            {
                                PrintJsonParameters(false, base_name, PATTERN_MEMBER.name, MEMORY_PATTERN_DEF, cfg_pattern.second, true, 1);
                            }
                        }
                        node_title.pop_back();
                    }
                    node_title.pop_back();
                }
                node_title.pop_back();
            }
        }
    }

    // Print Power parameters
    base_name   = POWER_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    if (this->m_testcases_param.power.exists)
    {
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), this->m_power_global_config_parameters_def_full, this->m_testcases_param.power.global_config);
        node_title.pop_back();
    }
    // Print GT_MAC parameters
    base_name   = GT_MAC_MEMBER.name;
    node_title  = {TESTCASES_MEMBER.name, base_name};
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMACs())
    {
        if (this->m_testcases_param.gt_mac[gt_index].exists)
        {
            node_title.emplace_back(std::to_string(gt_index));

            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_MAC_GLOBAL_CFG_PARAMETERS_DEF, this->m_testcases_param.gt_mac[gt_index].global_config);
            node_title.pop_back();

            node_title.emplace_back(LANE_CONFIG_MEMBER.name);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                if (this->m_testcases_param.gt_mac[gt_index].lane_config[lane_idx].exists)
                {
                    node_title.emplace_back(std::to_string(lane_idx));
                    PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_MAC_LANE_CFG_PARAMETERS_DEF, this->m_testcases_param.gt_mac[gt_index].lane_config[lane_idx].param);
                    node_title.pop_back();
                }
            }
            node_title.pop_back();

            node_title.pop_back();
        }
    }
    // Print GT_LPBK parameters
    base_name = GT_LPBK_MEMBER.name;
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTLpbks())
    {
        if (this->m_testcases_param.gt_lpbk[gt_index].exists)
        {
            node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};

            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF, this->m_testcases_param.gt_lpbk[gt_index].global_config);
            node_title.pop_back();

            node_title.emplace_back(LANE_CONFIG_MEMBER.name);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                if (this->m_testcases_param.gt_lpbk[gt_index].lane_config[lane_idx].exists)
                {
                    node_title.emplace_back(std::to_string(lane_idx));
                    PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_LPBK_LANE_CFG_PARAMETERS_DEF, this->m_testcases_param.gt_lpbk[gt_index].lane_config[lane_idx].param);
                    node_title.pop_back();
                }
            }
        }
    }
    // Print GT_PRBS parameters
    base_name = GT_PRBS_MEMBER.name;
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTPrbss())
    {
        if (this->m_testcases_param.gt_prbs[gt_index].exists)
        {
            node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};

            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, this->m_testcases_param.gt_prbs[gt_index].global_config);
            node_title.pop_back();

            node_title.emplace_back(LANE_CONFIG_MEMBER.name);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                if (this->m_testcases_param.gt_prbs[gt_index].lane_config[lane_idx].exists)
                {
                    node_title.emplace_back(std::to_string(lane_idx));
                    PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_PRBS_LANE_CFG_PARAMETERS_DEF, this->m_testcases_param.gt_prbs[gt_index].lane_config[lane_idx].param);
                    node_title.pop_back();
                }
            }
        }
    }
    // Print GTF_PRBS parameters
    base_name = GTF_PRBS_MEMBER.name;
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTFPrbss())
    {
        if (this->m_testcases_param.gtf_prbs[gt_index].exists)
        {
            node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};

            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, this->m_testcases_param.gtf_prbs[gt_index].global_config);
            node_title.pop_back();

            node_title.emplace_back(LANE_CONFIG_MEMBER.name);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                if (this->m_testcases_param.gtf_prbs[gt_index].lane_config[lane_idx].exists)
                {
                    node_title.emplace_back(std::to_string(lane_idx));
                    PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_PRBS_LANE_CFG_PARAMETERS_DEF, this->m_testcases_param.gtf_prbs[gt_index].lane_config[lane_idx].param);
                    node_title.pop_back();
                }
            }
        }
    }
    // Print GTM_PRBS parameters
    base_name = GTM_PRBS_MEMBER.name;
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMPrbss())
    {
        if (this->m_testcases_param.gtm_prbs[gt_index].exists)
        {
            node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};

            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, this->m_testcases_param.gtm_prbs[gt_index].global_config);
            node_title.pop_back();

            node_title.emplace_back(LANE_CONFIG_MEMBER.name);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                if (this->m_testcases_param.gtm_prbs[gt_index].lane_config[lane_idx].exists)
                {
                    node_title.emplace_back(std::to_string(lane_idx));
                    PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_PRBS_LANE_CFG_PARAMETERS_DEF, this->m_testcases_param.gtm_prbs[gt_index].lane_config[lane_idx].param);
                    node_title.pop_back();
                }
            }
        }
    }
    // Print GTYP_PRBS parameters
    base_name = GTYP_PRBS_MEMBER.name;
    for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTYPPrbss())
    {
        if (this->m_testcases_param.gtyp_prbs[gt_index].exists)
        {
            node_title = {TESTCASES_MEMBER.name, base_name, std::to_string(gt_index)};

            node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
            PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, this->m_testcases_param.gtyp_prbs[gt_index].global_config);
            node_title.pop_back();

            node_title.emplace_back(LANE_CONFIG_MEMBER.name);
            for (const auto & lane_idx : GT_LANE_INDEXES)
            {
                if (this->m_testcases_param.gtyp_prbs[gt_index].lane_config[lane_idx].exists)
                {
                    node_title.emplace_back(std::to_string(lane_idx));
                    PrintJsonParameters(false, base_name, StrVectToStr(node_title, "."), GT_PRBS_LANE_CFG_PARAMETERS_DEF, this->m_testcases_param.gtyp_prbs[gt_index].lane_config[lane_idx].param);
                    node_title.pop_back();
                }
            }
        }
    }
}

bool InputParser::CheckCuForTestcase()
{
    std::string                 base_name;
    std::vector<std::string>    base_title;
    // Try and parse all, if xbtest HW IP is not available, and then a testcase not supported, then it is not allowed in Test JSON definition (see InputParser::GetJsonDefinition)
    base_name   = DMA_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        if (this->m_xbtest_sw_config->GetAvailableBoardMemories().empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no xbtest HW IP targeting on-board memories found in the HW design"});
            return RET_FAILURE;
        }
    }
    base_name   = P2P_CARD_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        if (this->m_xbtest_sw_config->GetAvailableBoardMemories().empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no xbtest HW IP targeting on-board memories found in the HW design"});
            return RET_FAILURE;
        }
        if (!(this->m_xbtest_sw_config->GetIsP2PSupported()))
        {
            LogMessage(MSG_ITF_150, {base_name, "P2P is not supported on source card"});
            return RET_FAILURE;
        }
    }
    base_name   = P2P_NVME_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        if (this->m_xbtest_sw_config->GetAvailableBoardMemories().empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no xbtest HW IP targeting on-board memories found in the HW design"});
            return RET_FAILURE;
        }
        if (!(this->m_xbtest_sw_config->GetIsP2PSupported()))
        {
            LogMessage(MSG_ITF_150, {base_name, "P2P is not supported on card"});
            return RET_FAILURE;
        }
    }
    base_name   = MEMORY_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto all_available_memory = this->m_xbtest_sw_config->GetAllAvailableMemories();
        if (all_available_memory.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no Memory xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        if (node_names.empty())
        {
            base_title.emplace_back("<memory name> with <memory name> in {" + StrVectToStr(all_available_memory, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        all_available_memory.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, all_available_memory))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no Memory xbtest HW IP targeting " + node_name + " found in the HW design"});
                return RET_FAILURE;
            }
        }
    }
    base_name   = POWER_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        if (this->m_xbtest_sw_config->GetAvailablePowerSLRs().empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no Power xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }
    }
    base_name   = GT_MAC_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto available_gt_macs = this->m_xbtest_sw_config->GetAvailableGTMACs();
        if (available_gt_macs.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no GT_MAC xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        std::vector<std::string> supported_node_names;
        supported_node_names.reserve(available_gt_macs.size()+1);
        for (const auto & gt_index : available_gt_macs)
        {
            supported_node_names.emplace_back(std::to_string(gt_index));
        }
        if (node_names.empty())
        {
            base_title.emplace_back("<gt index> with <gt index> in {" + StrVectToStr(supported_node_names, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        supported_node_names.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, supported_node_names))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no GT_MAC xbtest HW IP targeting GT[" + node_name + "] found in the HW design"});
                return RET_FAILURE;
            }
        }
    }
    base_name   = GT_LPBK_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto available_gt_lpbks = this->m_xbtest_sw_config->GetAvailableGTLpbks();
        if (available_gt_lpbks.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no GT_LPBK xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        std::vector<std::string> supported_node_names;
        supported_node_names.reserve(available_gt_lpbks.size()+1);
        for (const auto & gt_index : available_gt_lpbks)
        {
            supported_node_names.emplace_back(std::to_string(gt_index));
        }
        if (node_names.empty())
        {
            base_title.emplace_back("<gt index> with <gt index> in {" + StrVectToStr(supported_node_names, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        supported_node_names.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, supported_node_names))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no GT_LPBK xbtest HW IP targeting GT[" + node_name + "] found in the HW design"});
                return RET_FAILURE;
            }
        }
    }

    base_name   = GT_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto available_gt_prbss = this->m_xbtest_sw_config->GetAvailableGTPrbss();
        if (available_gt_prbss.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no GT_PRBS xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        auto supported_node_names = this->m_supported_gt_prbs_node_names;
        if (node_names.empty())
        {
            base_title.emplace_back("<gt index> with <gt index> in {" + StrVectToStr(supported_node_names, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        supported_node_names.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, supported_node_names))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no GT_PRBS xbtest HW IP targeting GT[" + node_name + "] found in the HW design"});
                return RET_FAILURE;
            }
        }
    }
    base_name   = GTF_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto available_gtf_prbss = this->m_xbtest_sw_config->GetAvailableGTFPrbss();
        if (available_gtf_prbss.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no GTF_PRBS xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        auto supported_node_names = this->m_supported_gtf_prbs_node_names;
        if (node_names.empty())
        {
            base_title.emplace_back("<gt index> with <gt index> in {" + StrVectToStr(supported_node_names, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        supported_node_names.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, supported_node_names))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no GTF_PRBS xbtest HW IP targeting GT[" + node_name + "] found in the HW design"});
                return RET_FAILURE;
            }
        }
    }
    base_name   = GTM_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto available_gtm_prbss = this->m_xbtest_sw_config->GetAvailableGTMPrbss();
        if (available_gtm_prbss.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no GTM_PRBS xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        auto supported_node_names = this->m_supported_gtm_prbs_node_names;
        if (node_names.empty())
        {
            base_title.emplace_back("<gt index> with <gt index> in {" + StrVectToStr(supported_node_names, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        supported_node_names.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, supported_node_names))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no GTM_PRBS xbtest HW IP targeting GT[" + node_name + "] found in the HW design"});
                return RET_FAILURE;
            }
        }
    }
    base_name   = GTYP_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        auto available_gtyp_prbss = this->m_xbtest_sw_config->GetAvailableGTYPPrbss();
        if (available_gtyp_prbss.empty())
        {
            LogMessage(MSG_ITF_103, {base_name, "no GTYP_PRBS xbtest HW IP found in the HW design"});
            return RET_FAILURE;
        }

        // List nodes, this should contains indexes from 0 to N
        std::vector<std::string> node_names;
        if (ListNodeMembers(base_title, node_names) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        auto supported_node_names = this->m_supported_gtyp_prbs_node_names;
        if (node_names.empty())
        {
            base_title.emplace_back("<gt index> with <gt index> in {" + StrVectToStr(supported_node_names, ", ") + "}");
            PrintRequiredNotFound(base_title);
            return RET_FAILURE;
        }
        supported_node_names.emplace_back("comment"); // nodes "comment" are allowed anywhere in test JSON
        for (const auto & node_name : node_names)
        {
            if (!FindStringInVect(node_name, supported_node_names))
            {
                LogMessage(MSG_ITF_103, {base_name + "." + node_name + ".", "no GTYP_PRBS xbtest HW IP targeting GT[" + node_name + "] found in the HW design"});
                return RET_FAILURE;
            }
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseTestcasesParameters()
{
    std::string                 base_name;
    std::vector<std::string>    base_title;
    // Get MMIO parameters
    base_name   = MMIO_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_testcases_param.mmio.exists = true;
        auto node_title = base_title;

        // Get global_config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, MMIO_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.mmio.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();
    }

    // Get DMA parameters
    base_name   = DMA_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_testcases_param.dma.exists = true;
        auto node_title = base_title;

        // Get global_config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, DMA_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.dma.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();

        // Get memory type specific config
        node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            if (memory.target == BOARD)
            {
                node_title.emplace_back(memory.name);
                Dma_Type_Parameters_t dma_type_parameters;
                dma_type_parameters.exists = false;
                dma_type_parameters.param.clear();
                if (NodeExists(node_title))
                {
                    LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                    dma_type_parameters.exists = true;
                    if (ParseJsonParameters(base_name, node_title, DMA_MEMORY_TYPE_PARAMETERS_DEF, dma_type_parameters.param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                }
                this->m_testcases_param.dma.memory_config[memory.idx] = dma_type_parameters;
                node_title.pop_back();
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }

    // Get p2p_card parameters
    base_name   = P2P_CARD_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_testcases_param.p2p_card.exists = true;
        auto node_title = base_title;

        // Get global_config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, P2P_CARD_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.p2p_card.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();

        // Get memory type specific config
        node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            if (memory.target == BOARD)
            {
                node_title.emplace_back(memory.name);
                P2P_Card_Type_Parameters_t p2p_card_type_parameters;
                p2p_card_type_parameters.exists = false;
                p2p_card_type_parameters.param.clear();
                if (NodeExists(node_title))
                {
                    LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                    p2p_card_type_parameters.exists = true;
                    if (ParseJsonParameters(base_name, node_title, P2P_CARD_MEMORY_TYPE_PARAMETERS_DEF, p2p_card_type_parameters.param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                }
                this->m_testcases_param.p2p_card.memory_config[memory.idx] = p2p_card_type_parameters;
                node_title.pop_back();
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }

    // Get p2p_nvme parameters
    base_name   = P2P_NVME_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_testcases_param.p2p_nvme.exists = true;
        auto node_title = base_title;

        // Get global_config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, P2P_NVME_GLOBAL_CONFIG_PARAMETERS_DEF, this->m_testcases_param.p2p_nvme.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();

        // Get memory type specific config
        node_title.emplace_back(MEMORY_TYPE_CONFIG_MEMBER.name);
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            if (memory.target == BOARD)
            {
                node_title.emplace_back(memory.name);
                P2P_Nvme_Type_Parameters_t p2p_nvme_type_parameters;
                p2p_nvme_type_parameters.exists = false;
                p2p_nvme_type_parameters.param.clear();
                if (NodeExists(node_title))
                {
                    LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                    p2p_nvme_type_parameters.exists = true;
                    if (ParseJsonParameters(base_name, node_title, P2P_NVME_MEMORY_TYPE_PARAMETERS_DEF, p2p_nvme_type_parameters.param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                }
                this->m_testcases_param.p2p_nvme.memory_config[memory.idx] = p2p_nvme_type_parameters;
                node_title.pop_back();
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }

    // Get Memory parameters
    base_name   = MEMORY_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        auto node_title = base_title;
        this->m_testcases_param.memory.exists = true;
        // Get memory type specific config
        for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
        {
            Memory_Type_Parameters_t memory_type_parameters;
            memory_type_parameters.exists = false;
            memory_type_parameters.global_config.clear();
            memory_type_parameters.memory_chan_config.clear();
            memory_type_parameters.memory_tag_config.clear();
            memory_type_parameters.global_config_pattern.clear();
            memory_type_parameters.memory_chan_config_pattern.clear();
            memory_type_parameters.memory_tag_config_pattern.clear();

            node_title.emplace_back(memory.name);
            if (NodeExists(node_title))
            {
                LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});

                memory_type_parameters.exists = true;

                // Get global_config
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                if (ParseJsonParameters(base_name, node_title, GetMemoryGlobalParametersDef(memory.name), memory_type_parameters.global_config) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }

                // Get pattern
                node_title.emplace_back(PATTERN_MEMBER.name);
                if (NodeExists(node_title))
                {
                    LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                    if (ParseJsonParameters(base_name, node_title, MEMORY_PATTERN_DEF, memory_type_parameters.global_config_pattern) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                }
                node_title.pop_back();

                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                node_title.pop_back();

                if (memory.type == SINGLE_CHANNEL)
                {
                    // Get memory_tag_config
                    node_title.emplace_back(MEMORY_TAG_CONFIG_MEMBER.name);
                    if (NodeExists(node_title))
                    {
                        for (const auto & tag : this->m_xbtest_sw_config->GetAvailableSpTag(memory.name))
                        {
                            node_title.emplace_back(tag);
                            if (NodeExists(node_title))
                            {
                                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                                Json_Parameters_t param;
                                if (ParseJsonParameters(base_name, node_title, GetMemoryTagChanParametersDef(memory.name), param) == RET_FAILURE)
                                {
                                    return RET_FAILURE;
                                }
                                memory_type_parameters.memory_tag_config.emplace(tag, param);
                                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                            }

                            // Get pattern
                            node_title.emplace_back(PATTERN_MEMBER.name);
                            if (NodeExists(node_title))
                            {
                                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                                Json_Parameters_t param;
                                if (ParseJsonParameters(base_name, node_title, MEMORY_PATTERN_DEF, param) == RET_FAILURE)
                                {
                                    return RET_FAILURE;
                                }
                                memory_type_parameters.memory_tag_config_pattern.emplace(tag, param);
                                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                            }
                            node_title.pop_back();

                            node_title.pop_back();
                        }
                    }
                    node_title.pop_back();
                }
                else
                {
                    // Get memory_tag_config
                    node_title.emplace_back(MEMORY_CHANNEL_CONFIG_MEMBER.name);
                    if (NodeExists(node_title))
                    {
                        for (uint ch_idx = 0; ch_idx < this->m_xbtest_sw_config->GetMemoryNumChannel(memory.name); ch_idx++)
                        {
                            node_title.emplace_back(std::to_string(ch_idx));
                            if (NodeExists(node_title))
                            {
                                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                                Json_Parameters_t param;
                                if (ParseJsonParameters(base_name, node_title, GetMemoryTagChanParametersDef(memory.name), param) == RET_FAILURE)
                                {
                                    return RET_FAILURE;
                                }
                                memory_type_parameters.memory_chan_config.emplace(ch_idx, param);
                                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                            }

                            // Get pattern
                            node_title.emplace_back(PATTERN_MEMBER.name);
                            if (NodeExists(node_title))
                            {
                                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                                Json_Parameters_t param;
                                if (ParseJsonParameters(base_name, node_title, MEMORY_PATTERN_DEF, param) == RET_FAILURE)
                                {
                                    return RET_FAILURE;
                                }
                                memory_type_parameters.memory_chan_config_pattern.emplace(ch_idx, param);
                                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                            }
                            node_title.pop_back();

                            node_title.pop_back();
                        }
                    }
                    node_title.pop_back();
                }

                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
            }
            this->m_testcases_param.memory.memory_config[memory.idx] = memory_type_parameters;
            node_title.pop_back();
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }

    // Get Power parameters
    base_name   = POWER_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        this->m_testcases_param.power.exists = true;
        auto node_title = base_title;

        // Get global_config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, this->m_power_global_config_parameters_def_full, this->m_testcases_param.power.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();
    }

    // Get GT_MAC parameters
    base_name   = GT_MAC_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});

        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMACs())
        {
            Gt_Parameters_t gt_parameters;
            gt_parameters.exists = false;
            gt_parameters.global_config.clear();
            gt_parameters.lane_config.clear();

            base_title.emplace_back(std::to_string(gt_index));
            if (NodeExists(base_title))
            {
                LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
                auto node_title = base_title;

                gt_parameters.exists = true;

                // Get global config
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                if (ParseJsonParameters(base_name, node_title, GT_MAC_GLOBAL_CFG_PARAMETERS_DEF, gt_parameters.global_config) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                node_title.pop_back();

                // Get lane specific config
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    Gt_Lane_Parameters_t gt_lane_parameters;
                    gt_lane_parameters.exists = false;
                    gt_lane_parameters.param.clear();

                    node_title.emplace_back(std::to_string(lane_idx));
                    if (NodeExists(node_title))
                    {
                        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                        gt_lane_parameters.exists = true;
                        if (ParseJsonParameters(base_name, node_title, GT_MAC_LANE_CFG_PARAMETERS_DEF, gt_lane_parameters.param) == RET_FAILURE)
                        {
                            return RET_FAILURE;
                        }
                        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                    }
                    gt_parameters.lane_config[lane_idx] = gt_lane_parameters;
                    node_title.pop_back();
                }
                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
            }
            this->m_testcases_param.gt_mac[gt_index] = gt_parameters;
            base_title.pop_back();
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }

    // Get GT_LPBK parameters
    base_name   = GT_LPBK_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});

        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTLpbks())
        {
            Gt_Parameters_t gt_parameters;
            gt_parameters.exists = false;
            gt_parameters.global_config.clear();
            gt_parameters.lane_config.clear();

            base_title.emplace_back(std::to_string(gt_index));
            if (NodeExists(base_title))
            {
                LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
                auto node_title = base_title;

                gt_parameters.exists = true;

                // Get global config
                node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
                LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
                if (ParseJsonParameters(base_name, node_title, GT_LPBK_GLOBAL_CFG_PARAMETERS_DEF, gt_parameters.global_config) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                node_title.pop_back();

                // Get lane specific config
                node_title.emplace_back(LANE_CONFIG_MEMBER.name);
                for (const auto & lane_idx : GT_LANE_INDEXES)
                {
                    Gt_Lane_Parameters_t gt_lane_parameters;
                    gt_lane_parameters.exists = false;
                    gt_lane_parameters.param.clear();
                    node_title.emplace_back(std::to_string(lane_idx));
                    if (NodeExists(node_title))
                    {
                        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                        gt_lane_parameters.exists = true;
                        if (ParseJsonParameters(base_name, node_title, GT_LPBK_LANE_CFG_PARAMETERS_DEF, gt_lane_parameters.param) == RET_FAILURE)
                        {
                            return RET_FAILURE;
                        }
                        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
                    }
                    gt_parameters.lane_config[lane_idx] = gt_lane_parameters;
                    node_title.pop_back();
                }
                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
            }
            this->m_testcases_param.gt_lpbk[gt_index] = gt_parameters;
            base_title.pop_back();
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    // Get GT_PRBS parameters
    base_name   = GT_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});

        Gt_Parameters_t default_gt_parameters;
        if (ParseGtPrbsIndexJsonParameters(base_name, base_title, DEFAULT_MEMBER.name, default_gt_parameters) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTPrbss())
        {
            Gt_Parameters_t gt_parameters;
            if (ParseGtPrbsIndexJsonParameters(base_name, base_title, std::to_string(gt_index), gt_parameters) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!gt_parameters.exists && default_gt_parameters.exists)
            {
                this->m_testcases_param.gt_prbs[gt_index] = default_gt_parameters;
            }
            else
            {
                this->m_testcases_param.gt_prbs[gt_index] = gt_parameters;
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }

    // Get GTF_PRBS parameters
    base_name   = GTF_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});

        Gt_Parameters_t default_gt_parameters;
        if (ParseMultiGtPrbsIndexJsonParameters(base_name, base_title, DEFAULT_MEMBER.name, default_gt_parameters) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTFPrbss())
        {
            Gt_Parameters_t gt_parameters;
            if (ParseMultiGtPrbsIndexJsonParameters(base_name, base_title, std::to_string(gt_index), gt_parameters) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!gt_parameters.exists && default_gt_parameters.exists)
            {
                this->m_testcases_param.gtf_prbs[gt_index] = default_gt_parameters;
            }
            else
            {
                this->m_testcases_param.gtf_prbs[gt_index] = gt_parameters;
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    // Get GTM_PRBS parameters
    base_name   = GTM_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});

        Gt_Parameters_t default_gt_parameters;
        if (ParseMultiGtPrbsIndexJsonParameters(base_name, base_title, DEFAULT_MEMBER.name, default_gt_parameters) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTMPrbss())
        {
            Gt_Parameters_t gt_parameters;
            if (ParseMultiGtPrbsIndexJsonParameters(base_name, base_title, std::to_string(gt_index), gt_parameters) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!gt_parameters.exists && default_gt_parameters.exists)
            {
                this->m_testcases_param.gtm_prbs[gt_index] = default_gt_parameters;
            }
            else
            {
                this->m_testcases_param.gtm_prbs[gt_index] = gt_parameters;
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    // Get GTYP_PRBS parameters
    base_name   = GTYP_PRBS_MEMBER.name;
    base_title  = {TESTCASES_MEMBER.name, base_name};
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});

        Gt_Parameters_t default_gt_parameters;
        if (ParseMultiGtPrbsIndexJsonParameters(base_name, base_title, DEFAULT_MEMBER.name, default_gt_parameters) == RET_FAILURE)
        {
            return RET_FAILURE;
        }

        for (const auto & gt_index : this->m_xbtest_sw_config->GetAvailableGTYPPrbss())
        {
            Gt_Parameters_t gt_parameters;
            if (ParseMultiGtPrbsIndexJsonParameters(base_name, base_title, std::to_string(gt_index), gt_parameters) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!gt_parameters.exists && default_gt_parameters.exists)
            {
                this->m_testcases_param.gtyp_prbs[gt_index] = default_gt_parameters;
            }
            else
            {
                this->m_testcases_param.gtyp_prbs[gt_index] = gt_parameters;
            }
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    return RET_SUCCESS;
}

bool InputParser::ParseGtPrbsIndexJsonParameters( const std::string & base_name, std::vector<std::string> & base_title, const std::string & name, Gt_Parameters_t & gt_parameters )
{
    gt_parameters.exists = false;
    gt_parameters.global_config.clear();
    gt_parameters.lane_config.clear();

    base_title.emplace_back(name);
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        auto node_title = base_title;

        gt_parameters.exists = true;

        // Get global config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, gt_parameters.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();

        // Get lane specific config
        node_title.emplace_back(LANE_CONFIG_MEMBER.name);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            Gt_Lane_Parameters_t gt_lane_parameters;
            gt_lane_parameters.exists = false;
            gt_lane_parameters.param.clear();
            node_title.emplace_back(std::to_string(lane_idx));
            if (NodeExists(node_title))
            {
                LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                gt_lane_parameters.exists = true;
                if (ParseJsonParameters(base_name, node_title, GT_PRBS_LANE_CFG_PARAMETERS_DEF, gt_lane_parameters.param) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
            }
            gt_parameters.lane_config[lane_idx] = gt_lane_parameters;
            node_title.pop_back();
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    base_title.pop_back();
    return RET_SUCCESS;
}

bool InputParser::ParseMultiGtPrbsIndexJsonParameters( const std::string & base_name, std::vector<std::string> & base_title, const std::string & name, Gt_Parameters_t & gt_parameters )
{
    gt_parameters.exists = false;
    gt_parameters.global_config.clear();
    gt_parameters.lane_config.clear();

    base_title.emplace_back(name);
    if (NodeExists(base_title))
    {
        LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(base_title, ".") + " parameters. Parsing..."});
        auto node_title = base_title;

        gt_parameters.exists = true;

        // Get global config
        node_title.emplace_back(GLOBAL_CONFIG_MEMBER.name);
        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + StrVectToStr(node_title, ".") + " parameters..."});
        if (ParseJsonParameters(base_name, node_title, MULTI_GT_PRBS_GLOBAL_CFG_PARAMETERS_DEF, gt_parameters.global_config) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
        node_title.pop_back();

        // Get lane specific config
        node_title.emplace_back(LANE_CONFIG_MEMBER.name);
        for (const auto & lane_idx : GT_LANE_INDEXES)
        {
            Gt_Lane_Parameters_t gt_lane_parameters;
            gt_lane_parameters.exists = false;
            gt_lane_parameters.param.clear();
            node_title.emplace_back(std::to_string(lane_idx));
            if (NodeExists(node_title))
            {
                LogMessage(MSG_DEBUG_PARSING, {"Found " + StrVectToStr(node_title, ".") + " parameters. Parsing..."});
                gt_lane_parameters.exists = true;
                if (ParseJsonParameters(base_name, node_title, GT_PRBS_LANE_CFG_PARAMETERS_DEF, gt_lane_parameters.param) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(node_title, ".") + " parameters"});
            }
            gt_parameters.lane_config[lane_idx] = gt_lane_parameters;
            node_title.pop_back();
        }
        LogMessage(MSG_DEBUG_PARSING, {"Successfully parsed " + StrVectToStr(base_title, ".") + " parameters"});
    }
    base_title.pop_back();
    return RET_SUCCESS;
}

bool InputParser::ParseJsonParameters( const std::string & base_name, const std::vector<std::string> & base_title, const Json_Params_Def_t & json_params_def, Json_Parameters_t & param )
{
    // Parse all test parameter except test_sequence
    for (const auto & json_val_def : json_params_def)
    {
        auto node_title = base_title;
        node_title.emplace_back(json_val_def.name);
        LogMessage(MSG_DEBUG_PARSING, {"\t - Get " + StrVectToStr(node_title, ".") + " parameters"});

        if ((!NodeExists(node_title)) && (json_val_def.required == REQUIRED_TRUE))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (!NodeExists(node_title))
        {
            continue;
        }
        if (json_val_def.node_type == JSON_NODE_VALUE)
        {
            // Parse only values, other types are parsed specifically like test_sequence after other parameters
            switch (json_val_def.typeId)
            {
                case TYPE_ID_INT:
                    if (ParseJsonParamInt<int>(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                case TYPE_ID_UINT:
                    if (ParseJsonParamInt<uint>(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                case TYPE_ID_UINT64_T:
                    if (ParseJsonParamInt<uint64_t>(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                case TYPE_ID_FLOAT:
                    if (ParseJsonParamDouble<float>(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                case TYPE_ID_DOUBLE:
                    if (ParseJsonParamDouble<double>(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                case TYPE_ID_BOOL:
                    if (ParseJsonParamBool(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                case TYPE_ID_STRING:
                    if (ParseJsonParamStr(node_title, json_val_def, param) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    break;
                default: break;
            }
        }
        else if (json_val_def.typeId == TYPE_ID_TEST_SEQUENCE)
        {
            if (ParseTestSequence(base_name, node_title, param) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
        }
        else if (json_val_def.node_type == JSON_NODE_ARRAY)
        {
            switch (json_val_def.typeId)
            {
                case TYPE_ID_STRING:
                {
                    std::vector<std::string> node_array_value;
                    if (ExtractNodeArrayStr(node_title, node_array_value) == RET_FAILURE)
                    {
                        return RET_FAILURE;
                    }
                    InsertJsonParam<std::vector<std::string>>(param, json_val_def, node_array_value);
                    break;
                }
                default: break;
            }
        }
    }
    PrintJsonParameters(false, base_name, StrVectToStr(base_title, "."), json_params_def, param, true);
    return RET_SUCCESS;
}

bool InputParser::ParseMmioTestSequenceObject( const std::string & test_sequence_name, Mmio_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> node_title;
    Json_Definition_t json_definition;
    for (const auto & json_val_def : MMIO_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // BAR
    json_val_def    = BAR_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint8(test_sequence_name, node_title, test_seq_param.bar, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // offset
    json_val_def    = OFFSET_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint64(test_sequence_name, node_title, test_seq_param.offset, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // BUFFER_SIZE
    json_val_def    = BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint64(test_sequence_name, node_title, test_seq_param.buffer_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseMmioTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, Mmio_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseDMATestSequenceObject( const std::string & test_sequence_name, DMA_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> node_title;
    Json_Definition_t json_definition;
    for (const auto & json_val_def : DMA_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // TARGET
    json_val_def    = TARGET_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.target, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // BUFFER_SIZE
    json_val_def    = BUFFER_SIZE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint64(test_sequence_name, node_title, test_seq_param.buffer_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseDMATestSequenceArray( const std::string & test_sequence_name, const uint & param_index, DMA_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // TARGET
            name += TARGET_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.target) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseP2PCardTestSequenceObject( const std::string & test_sequence_name, P2P_Card_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> node_title;
    Json_Definition_t json_definition;
    for (const auto & json_val_def : P2P_CARD_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // SOURCE
    json_val_def    = SOURCE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.source, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // TARGET
    json_val_def    = TARGET_TEST_SEQ_MEMBER_OPT;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.target, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // BUFFER_SIZE
    json_val_def    = BUFFER_SIZE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint64(test_sequence_name, node_title, test_seq_param.buffer_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseP2PCardTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, P2P_Card_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // SOURCE
            name += SOURCE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.source) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseP2PNvmeTestSequenceObject( const std::string & test_sequence_name, P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> node_title;
    Json_Definition_t json_definition;
    for (const auto & json_val_def : P2P_NVME_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // SOURCE
    json_val_def    = SOURCE_TEST_SEQ_MEMBER_OPT;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.source, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // TARGET
    json_val_def    = TARGET_TEST_SEQ_MEMBER_OPT;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.target, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // BUFFER_SIZE
    json_val_def    = BUFFER_SIZE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint64(test_sequence_name, node_title, test_seq_param.buffer_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseP2PNvmeTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // SOURCE
            name += SOURCE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.source) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseMemoryTestSequenceObject( const std::string & test_sequence_name, Memory_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> node_title;
    Json_Definition_t json_definition;
    for (const auto & json_val_def : MEMORY_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    node_title = {test_sequence_name};
    if (CheckMembers(node_title, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // MODE
    json_val_def   = MODE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.mode, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (CheckStringInSet(test_sequence_name, test_seq_param.mode.value, SUPPORTED_MEMORY_TC_MODE, EMPTY_SET) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // WR_RATE
    json_val_def   = WR_RATE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("write", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.write.rate, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // WR_BANDWIDTH
    json_val_def   = WR_BANDWIDTH_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("write", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.write.bandwidth, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // WR_START_ADDR
    json_val_def   = WR_START_ADDR_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("write", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.write.start_addr, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // WR_BURST_SIZE
    json_val_def   = WR_BURST_SIZE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("write", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.write.burst_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // WR_BLOCK_SIZE
    json_val_def   = WR_BLOCK_SIZE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("write", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.write.block_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // WR_OUTSTANDING
    json_val_def   = WR_OUTSTANDING_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("write", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.write.outstanding, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // RD_RATE
    json_val_def   = RD_RATE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("read", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.read.rate, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // RD_BANDWIDTH
    json_val_def   = RD_BANDWIDTH_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("read", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.read.bandwidth, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // RD_START_ADDR
    json_val_def   = RD_START_ADDR_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("read", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.read.start_addr, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // RD_BURST_SIZE
    json_val_def   = RD_BURST_SIZE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("read", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.read.burst_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // RD_BLOCK_SIZE
    json_val_def   = RD_BLOCK_SIZE_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("read", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.read.block_size, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // RD_OUTSTANDING
    json_val_def   = RD_OUTSTANDING_TEST_SEQ_MEMBER;
    node_title     = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (CheckWriteReadOnly("read", {test_sequence_name, node_title[0]}, test_seq_param.mode.value) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.read.outstanding, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // Other checks
    if (CheckBothProvided({test_sequence_name}, WR_START_ADDR_TEST_SEQ_MEMBER.name, test_seq_param.write.start_addr.exists, WR_BLOCK_SIZE_TEST_SEQ_MEMBER.name, test_seq_param.write.block_size.exists) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CheckBothProvided({test_sequence_name}, RD_START_ADDR_TEST_SEQ_MEMBER.name, test_seq_param.read.start_addr.exists, RD_BLOCK_SIZE_TEST_SEQ_MEMBER.name, test_seq_param.read.block_size.exists) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseMemoryTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, Memory_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // MODE
            name += MODE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.mode) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (CheckStringInSet(name, test_seq_param.mode.value, SUPPORTED_MEMORY_TC_MODE, EMPTY_SET) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParsePowerTestSequenceObject( const std::string & test_sequence_name, Power_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string>    node_title;
    Json_Definition_t           json_definition;
    for (const auto & json_val_def : POWER_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // TOGGLE_TEST_SEQ
    json_val_def    = TOGGLE_RATE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.toggle_rate, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParsePowerTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, Power_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // TOGGLE_RATE
            name += TOGGLE_RATE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.toggle_rate) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseGTMACTestSequenceObject( const std::string & test_sequence_name, GTMAC_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> node_title;
    Json_Definition_t json_definition;
    for (const auto & json_val_def : GT_MAC_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // MODE
    json_val_def    = MODE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.mode, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseGTMACTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, GTMAC_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // MODE
            name += MODE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.mode) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::ParseGTLpbkTestSequenceObject( const std::string & test_sequence_name, GTLpbk_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string>    node_title;
    Json_Definition_t           json_definition;
    for (const auto & json_val_def : GT_LPBK_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // MODE
    json_val_def    = MODE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.mode, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseGTLpbkTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, GTLpbk_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // MODE
            name += MODE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.mode) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}
bool InputParser::ParseGTPrbsTestSequenceObject( const std::string & test_sequence_name, GTPrbs_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string>    node_title;
    Json_Definition_t           json_definition;
    for (const auto & json_val_def : GT_PRBS_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // MODE
    json_val_def    = MODE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.mode, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}
bool InputParser::ParseMultiGTPrbsTestSequenceObject( const std::string & test_sequence_name, MultiGTPrbs_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string>    node_title;
    Json_Definition_t           json_definition;
    for (const auto & json_val_def : MULTI_GT_PRBS_TEST_SEQ_DEF)
    {
        json_definition.insert( Definition_t({json_val_def.name}, json_val_def.node_type));
    }
    if (CheckMembers({test_sequence_name}, json_definition, true) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // DURATION
    auto json_val_def   = DURATION_TEST_SEQ_MEMBER;
    node_title          = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqUint(test_sequence_name, node_title, test_seq_param.duration, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    // MODE
    json_val_def    = MODE_TEST_SEQ_MEMBER;
    node_title      = {json_val_def.name};
    if ((json_val_def.required == REQUIRED_TRUE) || (NodeExists(node_title)))
    {
        if (ExtractNodeTestSeqStr(test_sequence_name, node_title, test_seq_param.mode, test_seq_param.num_param) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseGTPrbsTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, GTPrbs_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // MODE
            name += MODE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.mode) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}
bool InputParser::ParseMultiGTPrbsTestSequenceArray( const std::string & test_sequence_name, const uint & param_index, MultiGTPrbs_Test_Sequence_Parameters_t & test_seq_param )
{
    auto name = test_sequence_name + ".";
    switch (param_index)
    {
        case 0: // DURATION
            name += DURATION_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqUint(name, test_seq_param.duration) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
        case 1: // MODE
            name += MODE_TEST_SEQ_MEMBER.name;
            if (GetNodeTestSeqStr(name, test_seq_param.mode) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            test_seq_param.num_param++;
            break;
    }
    return RET_SUCCESS;
}

bool InputParser::GetNodeTestSeqUint( const std::string & name, Test_Seq_Param_Uint_t & test_seq_param )
{
    if (GetNodeValueInt<uint>(name, test_seq_param.value) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    test_seq_param.exists = true;
    return RET_SUCCESS;
}

bool InputParser::GetNodeTestSeqStr( const std::string & name, Test_Seq_Param_Str_t & test_seq_param )
{
    if (GetNodeValueStr(name, test_seq_param.value) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    test_seq_param.exists = true;
    return RET_SUCCESS;
}

bool InputParser::ExtractNodeTestSeqUint( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Uint_t & test_seq_param, uint & num_param )
{
    if (ExtractNodeValueInt<uint>(node_title, test_seq_param.value) == RET_FAILURE)
    {
        PrintRequiredNotFound({test_sequence_name, StrVectToStr(node_title, ".")});
        return RET_FAILURE;
    }
    test_seq_param.exists = true;
    num_param++;
    return RET_SUCCESS;
}

bool InputParser::ExtractNodeTestSeqUint8( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Uint8_t & test_seq_param, uint & num_param )
{
    if (ExtractNodeValueInt<uint8_t>(node_title, test_seq_param.value) == RET_FAILURE)
    {
        PrintRequiredNotFound({test_sequence_name, StrVectToStr(node_title, ".")});
        return RET_FAILURE;
    }
    test_seq_param.exists = true;
    num_param++;
    return RET_SUCCESS;
}

bool InputParser::ExtractNodeTestSeqUint64( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Uint64_t & test_seq_param, uint & num_param )
{
    if (ExtractNodeValueInt<uint64_t>(node_title, test_seq_param.value) == RET_FAILURE)
    {
        PrintRequiredNotFound({test_sequence_name, StrVectToStr(node_title, ".")});
        return RET_FAILURE;
    }
    test_seq_param.exists = true;
    num_param++;
    return RET_SUCCESS;
}

bool InputParser::ExtractNodeTestSeqStr( const std::string & test_sequence_name, const std::vector<std::string> & node_title, Test_Seq_Param_Str_t & test_seq_param, uint & num_param )
{
    if (ExtractNodeValueStr(node_title, test_seq_param.value) == RET_FAILURE)
    {
        PrintRequiredNotFound({test_sequence_name, StrVectToStr(node_title, ".")});
        return RET_FAILURE;
    }
    test_seq_param.exists = true;
    num_param++;
    return RET_SUCCESS;
}

bool InputParser::CheckStringInSet ( const std::string & param_name, const std::string & value, const std::set<std::string> & test_set, const std::set<std::string> & hidden_test_set )
{
    if (!FindStringInSet(value, test_set))
    {
        if (!FindStringInSet(value, hidden_test_set))
        {
            LogMessage(MSG_CMN_001, {value, param_name, StrSetToStr(QuoteStrSet(test_set), ", ")});
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::CheckStringInVect ( const std::string & param_name, const std::string & value, const std::vector<std::string> & test_vect, const std::vector<std::string> & hidden_test_vect )
{
    if (!FindStringInVect(value, test_vect))
    {
        if (!FindStringInVect(value, hidden_test_vect))
        {
            LogMessage(MSG_CMN_001, {value, param_name, StrVectToStr(QuoteStrVect(test_vect), ", ")});
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool InputParser::ParseJsonParamStr( const std::vector<std::string> & node_title, const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_parameters )
{
    std::string param;
    if (ExtractNodeValueStr(node_title, param) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    InsertJsonParam<std::string>(json_parameters, json_val_def, param);
    return RET_SUCCESS;
}

bool InputParser::ParseJsonParamBool( const std::vector<std::string> & node_title, const Json_Val_Def_t & json_val_def, Json_Parameters_t & json_parameters )
{
    bool param;
    if (ExtractNodeValueBool(node_title, param) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    InsertJsonParam<bool>(json_parameters, json_val_def, param);
    return RET_SUCCESS;
}

bool InputParser::ParseTestSequence( const std::string & testcase_name, const std::vector<std::string> & testsequence_node_title, Json_Parameters_t & json_parameters )
{
    auto parse_failure      = RET_SUCCESS;
    auto stop_parsing       = false;
    uint parse_error_cnt    = 0;
    uint num_node_read      = 0;
    ExtractNode(testsequence_node_title, num_node_read); // Move cursor to test sequence array

    auto test_sequence_size = count_json_elements();
    if (test_sequence_size == 0)
    {
        LogMessage(MSG_ITF_094, {StrVectToStr(testsequence_node_title, ".")});
        parse_failure = RET_FAILURE;
    }

    for (gint j = 0; (j < test_sequence_size) && (!stop_parsing); j++) // For each element in test sequence array
    {
        read_json_element(j); // Move cursor to test sequence element

        auto test_sequence_name         = StrVectToStr(testsequence_node_title, ".") + "[" + std::to_string(j) + "]";
        auto parse_it_failure           = RET_SUCCESS;
        auto test_seq_param_mmio        = RST_MMIO_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_dma         = RST_DMA_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_p2p_card    = RST_P2P_CARD_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_p2p_nvme    = RST_P2P_NVME_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_memory      = RST_MEMORY_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_power       = RST_POWER_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_gt_mac      = RST_GTMAC_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_gt_lpbk     = RST_GTLPBK_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_gt_prbs     = RST_GTPRBS_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_gtf_prbs    = RST_MULTI_GTPRBS_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_gtm_prbs    = RST_MULTI_GTPRBS_TEST_SEQUENCE_PARAMETERS;
        auto test_seq_param_gtyp_prbs   = RST_MULTI_GTPRBS_TEST_SEQUENCE_PARAMETERS;

        LogMessage(MSG_DEBUG_PARSING, {"Parsing " + test_sequence_name + "..."});

        // Check test sequence iteration parameters is a JSON array
        if ((is_json_array() == 0) && (is_json_object() == 0))
        {
            LogMessage(MSG_ITF_104, {test_sequence_name, "object or array"});
            parse_it_failure = RET_FAILURE;
        }
        if ((is_json_object() == 1) && (parse_it_failure == RET_SUCCESS)) // Test sequence element is an object
        {
            if (StrMatchNoCase(testcase_name, MMIO_MEMBER.name))
            {
                parse_it_failure = ParseMmioTestSequenceObject   (test_sequence_name, test_seq_param_mmio);
            }
            else if (StrMatchNoCase(testcase_name, DMA_MEMBER.name))
            {
                parse_it_failure = ParseDMATestSequenceObject   (test_sequence_name, test_seq_param_dma);
            }
            else if (StrMatchNoCase(testcase_name, P2P_CARD_MEMBER.name))
            {
                parse_it_failure = ParseP2PCardTestSequenceObject   (test_sequence_name, test_seq_param_p2p_card);
            }
            else if (StrMatchNoCase(testcase_name, P2P_NVME_MEMBER.name))
            {
                parse_it_failure = ParseP2PNvmeTestSequenceObject   (test_sequence_name, test_seq_param_p2p_nvme);
            }
            else if (StrMatchNoCase(testcase_name, MEMORY_MEMBER.name))
            {
                parse_it_failure = ParseMemoryTestSequenceObject(test_sequence_name, test_seq_param_memory);
            }
            else if (StrMatchNoCase(testcase_name, POWER_MEMBER.name))
            {
                parse_it_failure = ParsePowerTestSequenceObject (test_sequence_name, test_seq_param_power);
            }
            else if (StrMatchNoCase(testcase_name, GT_MAC_MEMBER.name))
            {
                parse_it_failure = ParseGTMACTestSequenceObject (test_sequence_name, test_seq_param_gt_mac);
            }
            else if (StrMatchNoCase(testcase_name, GT_LPBK_MEMBER.name))
            {
                parse_it_failure = ParseGTLpbkTestSequenceObject (test_sequence_name, test_seq_param_gt_lpbk);
            }
            else if (StrMatchNoCase(testcase_name, GT_PRBS_MEMBER.name))
            {
                parse_it_failure = ParseGTPrbsTestSequenceObject (test_sequence_name, test_seq_param_gt_prbs);
            }
            else if (StrMatchNoCase(testcase_name, GTF_PRBS_MEMBER.name))
            {
                parse_it_failure = ParseMultiGTPrbsTestSequenceObject (test_sequence_name, test_seq_param_gtf_prbs);
            }
            else if (StrMatchNoCase(testcase_name, GTM_PRBS_MEMBER.name))
            {
                parse_it_failure = ParseMultiGTPrbsTestSequenceObject (test_sequence_name, test_seq_param_gtm_prbs);
            }
            else if (StrMatchNoCase(testcase_name, GTYP_PRBS_MEMBER.name))
            {
                parse_it_failure = ParseMultiGTPrbsTestSequenceObject (test_sequence_name, test_seq_param_gtyp_prbs);
            }
        }
        else if ((is_json_array() == 1) && (parse_it_failure == RET_SUCCESS)) // Test sequence element is an array
        {
            LogMessage(MSG_ITF_128, {test_sequence_name});
            // Check number of parameters
            auto count_elements = (uint)count_json_elements();
            if (count_elements != TEST_SEQ_PARAM_ARRAY_SIZE)
            {
                LogMessage(MSG_ITF_067, {test_sequence_name, std::to_string(count_elements), std::to_string(TEST_SEQ_PARAM_ARRAY_SIZE)});
                parse_it_failure = RET_FAILURE;
            }
            for (uint i = 0; (i < count_elements) && (parse_it_failure == RET_SUCCESS); i++) // For each element in test sequence array
            {
                read_json_element(i); // Move cursor to test parameter element
                if (StrMatchNoCase(testcase_name, MMIO_MEMBER.name))
                {
                    parse_it_failure = ParseMmioTestSequenceArray   (test_sequence_name, i, test_seq_param_mmio);
                }
                else if (StrMatchNoCase(testcase_name, DMA_MEMBER.name))
                {
                    parse_it_failure = ParseDMATestSequenceArray   (test_sequence_name, i, test_seq_param_dma);
                }
                else if (StrMatchNoCase(testcase_name, P2P_CARD_MEMBER.name))
                {
                    parse_it_failure = ParseP2PCardTestSequenceArray   (test_sequence_name, i, test_seq_param_p2p_card);
                }
                else if (StrMatchNoCase(testcase_name, P2P_NVME_MEMBER.name))
                {
                    parse_it_failure = ParseP2PNvmeTestSequenceArray   (test_sequence_name, i, test_seq_param_p2p_nvme);
                }
                else if (StrMatchNoCase(testcase_name, MEMORY_MEMBER.name))
                {
                    parse_it_failure = ParseMemoryTestSequenceArray(test_sequence_name, i, test_seq_param_memory);
                }
                else if (StrMatchNoCase(testcase_name, POWER_MEMBER.name))
                {
                    parse_it_failure = ParsePowerTestSequenceArray (test_sequence_name, i, test_seq_param_power);
                }
                else if (StrMatchNoCase(testcase_name, GT_MAC_MEMBER.name))
                {
                    parse_it_failure = ParseGTMACTestSequenceArray (test_sequence_name, i, test_seq_param_gt_mac);
                }
                else if (StrMatchNoCase(testcase_name, GT_LPBK_MEMBER.name))
                {
                    parse_it_failure = ParseGTLpbkTestSequenceArray (test_sequence_name, i, test_seq_param_gt_lpbk);
                }
                else if (StrMatchNoCase(testcase_name, GT_PRBS_MEMBER.name))
                {
                    parse_it_failure = ParseGTPrbsTestSequenceArray (test_sequence_name, i, test_seq_param_gt_prbs);
                }
                else if (StrMatchNoCase(testcase_name, GTF_PRBS_MEMBER.name))
                {
                    parse_it_failure = ParseMultiGTPrbsTestSequenceArray (test_sequence_name, i, test_seq_param_gtf_prbs);
                }
                else if (StrMatchNoCase(testcase_name, GTM_PRBS_MEMBER.name))
                {
                    parse_it_failure = ParseMultiGTPrbsTestSequenceArray (test_sequence_name, i, test_seq_param_gtm_prbs);
                }
                else if (StrMatchNoCase(testcase_name, GTYP_PRBS_MEMBER.name))
                {
                    parse_it_failure = ParseMultiGTPrbsTestSequenceArray (test_sequence_name, i, test_seq_param_gtyp_prbs);
                }
                end_json_element();; // Move back cursor from test parameter array
            }
        }
        if (parse_it_failure == RET_SUCCESS)
        {
            if (StrMatchNoCase(testcase_name, MMIO_MEMBER.name))
            {
                AppendTestSequenceParameters<Mmio_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_mmio);
            }
            else if (StrMatchNoCase(testcase_name, DMA_MEMBER.name))
            {
                AppendTestSequenceParameters<DMA_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_dma);
            }
            else if (StrMatchNoCase(testcase_name, P2P_CARD_MEMBER.name))
            {
                AppendTestSequenceParameters<P2P_Card_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_p2p_card);
            }
            else if (StrMatchNoCase(testcase_name, P2P_NVME_MEMBER.name))
            {
                AppendTestSequenceParameters<P2P_Nvme_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_p2p_nvme);
            }
            else if (StrMatchNoCase(testcase_name, MEMORY_MEMBER.name))
            {
                AppendTestSequenceParameters<Memory_Test_Sequence_Parameters_t>(json_parameters, test_seq_param_memory);
            }
            else if (StrMatchNoCase(testcase_name, POWER_MEMBER.name))
            {
                AppendTestSequenceParameters<Power_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_power);
            }
            else if (StrMatchNoCase(testcase_name, GT_MAC_MEMBER.name))
            {
                AppendTestSequenceParameters<GTMAC_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_gt_mac);
            }
            else if (StrMatchNoCase(testcase_name, GT_LPBK_MEMBER.name))
            {
                AppendTestSequenceParameters<GTLpbk_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_gt_lpbk);
            }
            else if (StrMatchNoCase(testcase_name, GT_PRBS_MEMBER.name))
            {
                AppendTestSequenceParameters<GTPrbs_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_gt_prbs);
            }
            else if (StrMatchNoCase(testcase_name, GTF_PRBS_MEMBER.name))
            {
                AppendTestSequenceParameters<MultiGTPrbs_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_gtf_prbs);
            }
            else if (StrMatchNoCase(testcase_name, GTM_PRBS_MEMBER.name))
            {
                AppendTestSequenceParameters<MultiGTPrbs_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_gtm_prbs);
            }
            else if (StrMatchNoCase(testcase_name, GTYP_PRBS_MEMBER.name))
            {
                AppendTestSequenceParameters<MultiGTPrbs_Test_Sequence_Parameters_t> (json_parameters, test_seq_param_gtyp_prbs);
            }
        }
        else
        {
            LogMessage(MSG_ITF_073, {test_sequence_name});
            parse_error_cnt++;
            if (parse_error_cnt >= MAX_NUM_PARSER_ERROR)
            {
                stop_parsing = true;
            }
        }
        parse_failure |= parse_it_failure;

        end_json_element();; // Move back cursor to test sequence array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from test sequence array
    {
        end_json_element();;
    }
    if (parse_failure == RET_FAILURE)
    {
        LogMessage(MSG_ITF_075, {StrVectToStr(testsequence_node_title, ".")});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void InputParser::PrintJsonDefintion( const Json_Definition_t & json_definition )
{
    LogMessage(MSG_DEBUG_SETUP, {"Supported " + m_content_name + " parameters:"});
    for (const auto & def : json_definition)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - " + StrVectToStr(def.first, ".") + " (" + JsonNodeTypeToString(def.second) + ")"});
    }
}

void InputParser::PrintSensorLimit( const bool & info_n_debug, const std::string & limit_str, const Sensor_Limit_t & sensor_limit )
{
    if (sensor_limit.min_exists)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, "\t\t - " + limit_str + " limit min: " + std::to_string(sensor_limit.min));
    }
    if (sensor_limit.max_exists)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, "\t\t - " + limit_str + " limit max: " + std::to_string(sensor_limit.max));
    }
}

std::string InputParser::GetUintTestSequenceString( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Uint_t & test_seq_param )
{
    return "\"" + json_val_def.name + "\": " + std::to_string(test_seq_param.value);
}

std::string InputParser::GetUint8TestSequenceString( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Uint8_t & test_seq_param )
{
    return "\"" + json_val_def.name + "\": " + std::to_string(test_seq_param.value);
}

std::string InputParser::GetUint64TestSequenceString( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Uint64_t & test_seq_param )
{
    return "\"" + json_val_def.name + "\": " + std::to_string(test_seq_param.value);
}

std::string InputParser::GetStrTestSequenceString( const Json_Val_Def_t & json_val_def, const Test_Seq_Param_Str_t & test_seq_param )
{
    return "\"" + json_val_def.name + "\": \"" + test_seq_param.value + "\"";
}

std::string InputParser::GetMmioTestSequenceString( const uint & test_seq_param_idx, const Mmio_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.bar.exists)
    {
        str_vect.emplace_back(GetUint8TestSequenceString(BAR_TEST_SEQ_MEMBER, test_seq_param.bar));
    }
    if (test_seq_param.offset.exists)
    {
        str_vect.emplace_back(GetUint64TestSequenceString(OFFSET_TEST_SEQ_MEMBER, test_seq_param.offset));
    }
    if (test_seq_param.buffer_size.exists)
    {
        str_vect.emplace_back(GetUint64TestSequenceString(BUFFER_SIZE_BYTES_TEST_SEQ_MEMBER, test_seq_param.buffer_size));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintMmioTestSequence( const bool & info_n_debug, const std::vector<Mmio_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint ii = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetMmioTestSequenceString(ii, test_seq_param));
        ii++;
    }
}

void InputParser::PrintMmioTestSequenceAndResult( const bool & info_n_debug, const std::vector<Mmio_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetMmioTestSequenceString(ii + 1, test_sequence_parameters[ii]) + "; Result: " +  TestIterationResultToString(test_it_results[ii]));
    }
}

std::string InputParser::GetDMATestSequenceString( const uint & test_seq_param_idx, const DMA_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.target.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER, test_seq_param.target));
    }
    if (test_seq_param.buffer_size.exists)
    {
        str_vect.emplace_back(GetUint64TestSequenceString(BUFFER_SIZE_TEST_SEQ_MEMBER, test_seq_param.buffer_size));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintDMATestSequence( const bool & info_n_debug, const std::vector<DMA_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint ii = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetDMATestSequenceString(ii, test_seq_param));
        ii++;
    }
}

void InputParser::PrintDMATestSequenceAndResult( const bool & info_n_debug, const std::vector<DMA_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetDMATestSequenceString(ii + 1, test_sequence_parameters[ii]) + "; Result: " +  TestIterationResultToString(test_it_results[ii]));
    }
}

std::string InputParser::GetP2PCardTestSequenceString( const uint & test_seq_param_idx, const P2P_Card_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.source.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER, test_seq_param.source));
    }
    if (test_seq_param.target.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER_OPT, test_seq_param.target));
    }
    if (test_seq_param.buffer_size.exists)
    {
        str_vect.emplace_back(GetUint64TestSequenceString(BUFFER_SIZE_TEST_SEQ_MEMBER, test_seq_param.buffer_size));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintP2PCardTestSequence( const bool & info_n_debug, const std::vector<P2P_Card_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint ii = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetP2PCardTestSequenceString(ii, test_seq_param));
        ii++;
    }
}

void InputParser::PrintP2PCardTestSequenceAndResult( const bool & info_n_debug, const std::vector<P2P_Card_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetP2PCardTestSequenceString(ii + 1, test_sequence_parameters[ii]) + "; Result: " +  TestIterationResultToString(test_it_results[ii]));
    }
}

std::string InputParser::GetP2PNvmeTestSequenceString( const uint & test_seq_param_idx, const P2P_Nvme_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.source.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER_OPT, test_seq_param.source));
    }
    if (test_seq_param.target.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER_OPT, test_seq_param.target));
    }
    if (test_seq_param.buffer_size.exists)
    {
        str_vect.emplace_back(GetUint64TestSequenceString(BUFFER_SIZE_TEST_SEQ_MEMBER, test_seq_param.buffer_size));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintP2PNvmeTestSequence( const bool & info_n_debug, const std::vector<P2P_Nvme_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint ii = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetP2PNvmeTestSequenceString(ii, test_seq_param));
        ii++;
    }
}

void InputParser::PrintP2PNvmeTestSequenceAndResult( const bool & info_n_debug, const std::vector<P2P_Nvme_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetP2PNvmeTestSequenceString(ii + 1, test_sequence_parameters[ii]) + "; Result: " +  TestIterationResultToString(test_it_results[ii]));
    }
}

std::string InputParser::GetMemoryTestSequenceString( const uint & test_seq_param_idx, const Memory_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.mode.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER, test_seq_param.mode));
    }
    if (test_seq_param.write.rate.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(WR_RATE_TEST_SEQ_MEMBER, test_seq_param.write.rate));
    }
    if (test_seq_param.write.bandwidth.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(WR_BANDWIDTH_TEST_SEQ_MEMBER, test_seq_param.write.bandwidth));
    }
    if (test_seq_param.write.start_addr.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(WR_START_ADDR_TEST_SEQ_MEMBER, test_seq_param.write.start_addr));
    }
    if (test_seq_param.write.burst_size.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(WR_BURST_SIZE_TEST_SEQ_MEMBER, test_seq_param.write.burst_size));
    }
    if (test_seq_param.write.block_size.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(WR_BLOCK_SIZE_TEST_SEQ_MEMBER, test_seq_param.write.block_size));
    }
    if (test_seq_param.write.outstanding.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(WR_OUTSTANDING_TEST_SEQ_MEMBER, test_seq_param.write.outstanding));
    }
    if (test_seq_param.read.rate.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(RD_RATE_TEST_SEQ_MEMBER, test_seq_param.read.rate));
    }
    if (test_seq_param.read.bandwidth.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(RD_BANDWIDTH_TEST_SEQ_MEMBER, test_seq_param.read.bandwidth));
    }
    if (test_seq_param.read.start_addr.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(RD_START_ADDR_TEST_SEQ_MEMBER, test_seq_param.read.start_addr));
    }
    if (test_seq_param.read.burst_size.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(RD_BURST_SIZE_TEST_SEQ_MEMBER, test_seq_param.read.burst_size));
    }
    if (test_seq_param.read.block_size.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(RD_BLOCK_SIZE_TEST_SEQ_MEMBER, test_seq_param.read.block_size));
    }
    if (test_seq_param.read.outstanding.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(RD_OUTSTANDING_TEST_SEQ_MEMBER, test_seq_param.read.outstanding));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintMemoryTestSequence( const bool & info_n_debug, const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint ii = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetMemoryTestSequenceString(ii, test_seq_param));
        ii++;
    }
}

void InputParser::PrintMemorySCTestSequenceAndResult( const bool & info_n_debug, const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        msg = GetMemoryTestSequenceString(ii + 1, test_sequence_parameters[ii]);
        msg += "; Result: ";
        std::vector<std::string> result_str_vect;
        result_str_vect.reserve(test_it_results_map.size());
        for (const auto & test_result : test_it_results_map)
        {
            result_str_vect.emplace_back(test_result.first + " " +  TestIterationResultToString(test_result.second[ii]));
        }
        msg += StrVectToStr(result_str_vect, " / ");
        LogMessageIf(info_n_debug, MSG_GEN_018, msg);
    }
}

void InputParser::PrintMemoryMCTestSequenceAndResult( const bool & info_n_debug, const std::vector<Memory_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetMemoryTestSequenceString(ii + 1, test_sequence_parameters[ii]) + "; Result: " +  TestIterationResultToString(test_it_results[ii]));
    }
}

std::string InputParser::GetPowerTestSequenceString( const uint & test_seq_param_idx, const Power_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.toggle_rate.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(TOGGLE_RATE_TEST_SEQ_MEMBER, test_seq_param.toggle_rate));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintPowerTestSequence( const bool & info_n_debug, const std::vector<Power_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint test_seq_param_idx = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetPowerTestSequenceString(test_seq_param_idx, test_seq_param));
        test_seq_param_idx++;
    }
}

void InputParser::PrintPowerTestSequenceAndResult( const bool & info_n_debug, const std::vector<Power_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::vector<TestIterationResult> & test_it_results )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetPowerTestSequenceString(ii + 1, test_sequence_parameters[ii]) + "; Result: " +  TestIterationResultToString(test_it_results[ii]));
    }
}

std::string InputParser::GetGTMACTestSequenceString( const uint & test_seq_param_idx, const GTMAC_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.mode.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER, test_seq_param.mode));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintGTMACTestSequence( const bool & info_n_debug, const std::vector<GTMAC_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint ii = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetGTMACTestSequenceString(ii, test_seq_param));
        ii++;
    }
}

void InputParser::PrintGTMACTestSequenceAndResult( const bool & info_n_debug, const std::vector<GTMAC_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint ii = 0; ii < test_sequence_parameters.size(); ii++)
    {
        msg = GetGTMACTestSequenceString(ii + 1, test_sequence_parameters[ii]);
        msg += "; Result: ";
        std::vector<std::string> result_str_vect;
        result_str_vect.reserve(test_it_results_map.size());
        for (const auto & test_result : test_it_results_map)
        {
            result_str_vect.emplace_back(test_result.first + " " + TestIterationResultToString(test_result.second[ii]));
        }
        msg += StrVectToStr(result_str_vect, " / ");
        LogMessageIf(info_n_debug, MSG_GEN_018, msg);
    }
}

std::string InputParser::GetGTLpbkTestSequenceString( const uint & test_seq_param_idx, const GTLpbk_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.mode.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER, test_seq_param.mode));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintGTLpbkTestSequence( const bool & info_n_debug, const std::vector<GTLpbk_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint test_seq_param_idx = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetGTLpbkTestSequenceString(test_seq_param_idx, test_seq_param));
        test_seq_param_idx++;
    }
}

void InputParser::PrintGTLpbkTestSequenceAndResult( const bool & info_n_debug, const std::vector<GTLpbk_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint test_seq_param_idx = 0; test_seq_param_idx < test_sequence_parameters.size(); test_seq_param_idx++)
    {
        msg = GetGTLpbkTestSequenceString(test_seq_param_idx + 1, test_sequence_parameters[test_seq_param_idx]);
        msg += "; Result: ";
        std::vector<std::string> result_str_vect;
        result_str_vect.reserve(test_it_results_map.size());
        for (const auto & test_result : test_it_results_map)
        {
            result_str_vect.emplace_back(test_result.first + " " + TestIterationResultToString(test_result.second[test_seq_param_idx]));
        }
        msg += StrVectToStr(result_str_vect, " / ");
        LogMessageIf(info_n_debug, MSG_GEN_018, msg);
    }
}
std::string InputParser::GetGTPrbsTestSequenceString( const uint & test_seq_param_idx, const GTPrbs_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.mode.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER, test_seq_param.mode));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintGTPrbsTestSequence( const bool & info_n_debug, const std::vector<GTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint test_seq_param_idx = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetGTPrbsTestSequenceString(test_seq_param_idx, test_seq_param));
        test_seq_param_idx++;
    }
}

void InputParser::PrintGTPrbsTestSequenceAndResult( const bool & info_n_debug, const std::vector<GTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint test_seq_param_idx = 0; test_seq_param_idx < test_sequence_parameters.size(); test_seq_param_idx++)
    {
        msg = GetGTPrbsTestSequenceString(test_seq_param_idx + 1, test_sequence_parameters[test_seq_param_idx]);
        msg += "; Result: ";
        std::vector<std::string> result_str_vect;
        result_str_vect.reserve(test_it_results_map.size());
        for (const auto & test_result : test_it_results_map)
        {
            result_str_vect.emplace_back(test_result.first + " " + TestIterationResultToString(test_result.second[test_seq_param_idx]));
        }
        msg += StrVectToStr(result_str_vect, " / ");
        LogMessageIf(info_n_debug, MSG_GEN_018, msg);
    }
}

std::string InputParser::GetMultiGTPrbsTestSequenceString( const uint & test_seq_param_idx, const MultiGTPrbs_Test_Sequence_Parameters_t & test_seq_param )
{
    std::vector<std::string> str_vect;
    if (test_seq_param.duration.exists)
    {
        str_vect.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
    }
    if (test_seq_param.mode.exists)
    {
        str_vect.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER, test_seq_param.mode));
    }
    return "\t\t * Test " + std::to_string(test_seq_param_idx) + ": " + StrVectToStr(str_vect, ", ") + " ";
}

void InputParser::PrintMultiGTPrbsTestSequence( const bool & info_n_debug, const std::vector<MultiGTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    uint test_seq_param_idx = 1;
    for (const auto & test_seq_param : test_sequence_parameters)
    {
        LogMessageIf(info_n_debug, MSG_GEN_018, GetMultiGTPrbsTestSequenceString(test_seq_param_idx, test_seq_param));
        test_seq_param_idx++;
    }
}

void InputParser::PrintMultiGTPrbsTestSequenceAndResult( const bool & info_n_debug, const std::vector<MultiGTPrbs_Test_Sequence_Parameters_t> & test_sequence_parameters, const std::map<std::string, std::vector<TestIterationResult>> & test_it_results_map )
{
    auto msg = "\t- " + TEST_SEQUENCE_MEMBER.name + ":";
    LogMessageIf(info_n_debug, MSG_GEN_018, msg);

    for (uint test_seq_param_idx = 0; test_seq_param_idx < test_sequence_parameters.size(); test_seq_param_idx++)
    {
        msg = GetMultiGTPrbsTestSequenceString(test_seq_param_idx + 1, test_sequence_parameters[test_seq_param_idx]);
        msg += "; Result: ";
        std::vector<std::string> result_str_vect;
        result_str_vect.reserve(test_it_results_map.size());
        for (const auto & test_result : test_it_results_map)
        {
            result_str_vect.emplace_back(test_result.first + " " + TestIterationResultToString(test_result.second[test_seq_param_idx]));
        }
        msg += StrVectToStr(result_str_vect, " / ");
        LogMessageIf(info_n_debug, MSG_GEN_018, msg);
    }
}
void InputParser::LogMessageIf( const bool & info_n_debug, const Message_t & message, const std::string & msg )
{
    if (info_n_debug)
    {
        LogMessage(message, {msg});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {msg});
    }
}

void InputParser::PrintJsonParamsGuide( const std::string & tabs, const Json_Params_Def_t & json_params_def )
{
    // Get pad size automatically
    uint max_pad_size = 10; // min
    for (const auto & json_val_def : json_params_def)
    {
        if (json_val_def.name.size() > max_pad_size)
        {
            max_pad_size = json_val_def.name.size();
        }
    }
    for (const auto & json_val_def : json_params_def)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {tabs + "- " + pad(json_val_def.name, ' ', max_pad_size, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {tabs + "- " + pad(json_val_def.name, ' ', max_pad_size, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.description == DESCRIPTION_NOT_SET)
        {
            LogMessage(MSG_DEBUG_EXCEPTION, {"Description is not set for parameter " + json_val_def.name});
        }
    }
}

void InputParser::PrintMmioTestSequenceGuide()
{
    const uint MMIO_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + MMIO_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : MMIO_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', MMIO_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', MMIO_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
    }
}

void InputParser::PrintDMATestSequenceGuide()
{
    const uint DMA_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + DMA_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : DMA_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', DMA_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', DMA_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == TARGET_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: "});
            for (const auto & memory : m_xbtest_sw_config->GetMemoryDefinitions())
            {
                if (memory.target == BOARD)
                {
                    LogMessage(MSG_ITF_077, {"\t\t\t+ memory type: \"" + memory.name + "\""});
                    LogMessage(MSG_ITF_077, {"\t\t\t\t- memory tags for this memory type: " + StrVectToStr(QuoteStrVect(m_xbtest_sw_config->GetAvailableTag(memory.name)), ", ")});
                }
            }
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + DMA_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param             = RST_DMA_TEST_SEQUENCE_PARAMETERS;
    auto available_board_memories   = m_xbtest_sw_config->GetAvailableBoardMemories();
    if (!available_board_memories.empty())
    {
        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 1;
        test_seq_param.target.value     = available_board_memories[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER,      test_seq_param.target));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " second test of all memory tags of type " + test_seq_param.target.value + ":"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});
    }

    auto all_available_board_tag = m_xbtest_sw_config->GetAvailableBoardTag();
    if (!all_available_board_tag.empty())
    {
        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 10;
        test_seq_param.target.value     = all_available_board_tag[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER,      test_seq_param.target));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test of memory tag " + all_available_board_tag[0] + ":"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});
    }
}

void InputParser::PrintP2PCardTestSequenceGuide()
{
    const uint P2P_CARD_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + P2P_CARD_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : P2P_CARD_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', P2P_CARD_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', P2P_CARD_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == SOURCE_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: "});
            for (const auto & memory : m_xbtest_sw_config->GetMemoryDefinitions())
            {
                if (memory.target == BOARD)
                {
                    LogMessage(MSG_ITF_077, {"\t\t\t+ memory type: \"" + memory.name + "\""});
                    LogMessage(MSG_ITF_077, {"\t\t\t\t- memory tags for this memory type: " + StrVectToStr(QuoteStrVect(m_xbtest_sw_config->GetAvailableTag(memory.name)), ", ")});
                }
            }
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + P2P_CARD_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param             = RST_P2P_CARD_TEST_SEQUENCE_PARAMETERS;
    auto available_board_memories   = m_xbtest_sw_config->GetAvailableBoardMemories();
    if (!available_board_memories.empty())
    {
        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 1;
        test_seq_param.source.value     = available_board_memories[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER,      test_seq_param.source));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " second test of all memory tags of type " + test_seq_param.source.value + ":"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});
    }

    auto all_available_board_tag = m_xbtest_sw_config->GetAvailableBoardTag();
    if (!all_available_board_tag.empty())
    {
        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 10;
        test_seq_param.source.value     = all_available_board_tag[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER,      test_seq_param.source));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test of memory tag " + all_available_board_tag[0] + ":"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});
    }
}

void InputParser::PrintP2PNvmeTestSequenceGuide()
{
    const uint P2P_NVME_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + P2P_NVME_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : P2P_NVME_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', P2P_NVME_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', P2P_NVME_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if ((json_val_def.name == SOURCE_TEST_SEQ_MEMBER_OPT.name) || (json_val_def.name == TARGET_TEST_SEQ_MEMBER_OPT.name))
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: "});
            for (const auto & memory : m_xbtest_sw_config->GetMemoryDefinitions())
            {
                if (memory.target == BOARD)
                {
                    LogMessage(MSG_ITF_077, {"\t\t\t+ memory type: \"" + memory.name + "\""});
                    LogMessage(MSG_ITF_077, {"\t\t\t\t- memory tags for this memory type: " + StrVectToStr(QuoteStrVect(m_xbtest_sw_config->GetAvailableTag(memory.name)), ", ")});
                }
            }
        }
    }
    LogMessage(MSG_ITF_077, {"Parameters " + TARGET_TEST_SEQ_MEMBER_OPT.name + " and " + SOURCE_TEST_SEQ_MEMBER_OPT.name + " cannot be combined"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + P2P_NVME_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param             = RST_P2P_NVME_TEST_SEQUENCE_PARAMETERS;
    auto available_board_memories   = m_xbtest_sw_config->GetAvailableBoardMemories();
    if (!available_board_memories.empty())
    {
        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 1;
        test_seq_param.source.value     = available_board_memories[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER,      test_seq_param.source));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " second test of all memory tags of type " + test_seq_param.source.value + ":"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});
    }

    auto all_available_board_tag = m_xbtest_sw_config->GetAvailableBoardTag();
    if (!all_available_board_tag.empty())
    {
        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 10;
        test_seq_param.source.value     = all_available_board_tag[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER_OPT,  test_seq_param.source));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test of memory tag " + all_available_board_tag[0] + " (the NVMe is the P2P target):"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});

        testseq_example.clear();
        test_ex.clear();
        test_seq_param.duration.value   = 10;
        test_seq_param.target.value     = all_available_board_tag[0];
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER_OPT,  test_seq_param.target));
        testseq_example.emplace_back(StrVectToTest(test_ex));

        LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test of memory tag " + all_available_board_tag[0] + " (the NVMe is the P2P source):"});
        LogMessage(MSG_ITF_077, {""});
        LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
        LogMessage(MSG_ITF_077, {""});
    }
}

void InputParser::PrintMemoryTestSequenceGuide()
{
    const uint MEM_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + MEMORY_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : MEMORY_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', MEM_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', MEM_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == MODE_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: " + StrSetToStr(QuoteStrSet(SUPPORTED_MEMORY_TC_MODE), ", ")});
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + MEMORY_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param =  RST_MEMORY_TEST_SEQUENCE_PARAMETERS;
    for (const auto & mode : SUPPORTED_MEMORY_TC_MODE)
    {
        test_seq_param.duration.value   = 10;
        test_seq_param.mode.value       = mode;
        std::vector<std::string> test_ex;
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
        testseq_example.emplace_back(StrVectToTest(test_ex));
    }
    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test in each " + MODE_TEST_SEQ_MEMBER.name + ", using default read/write rate:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": ["});
    for (const auto & test : testseq_example)
    {
        LogMessage(MSG_ITF_077, {"\t\t\t" + test});
    }
    LogMessage(MSG_ITF_077, {"\t\t]"});
    LogMessage(MSG_ITF_077, {""});

    testseq_example.clear();
    test_ex.clear();
    test_seq_param.duration.value   = 20;
    test_seq_param.mode.value       = ALTERNATE_WR_RD;
    test_seq_param.write.rate.value = 100;
    test_seq_param.read.rate.value  = 75;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    test_ex.emplace_back(GetUintTestSequenceString(WR_RATE_TEST_SEQ_MEMBER,    test_seq_param.write.rate));
    test_ex.emplace_back(GetUintTestSequenceString(RD_RATE_TEST_SEQ_MEMBER,    test_seq_param.read.rate));
    testseq_example.emplace_back(StrVectToTest(test_ex));
    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test in " + test_seq_param.mode.value + " mode with " + WR_MEM_RATE + " set to " + std::to_string(test_seq_param.write.rate.value) + "% and " + RD_MEM_RATE + " set to " + std::to_string(test_seq_param.read.rate.value) + "%:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
    LogMessage(MSG_ITF_077, {""});

    testseq_example.clear();
    test_ex.clear();
    test_seq_param.duration.value   = 20;
    test_seq_param.mode.value       = ONLY_WR;
    test_seq_param.write.rate.value = 10;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    test_ex.emplace_back(GetUintTestSequenceString(WR_RATE_TEST_SEQ_MEMBER,    test_seq_param.write.rate));
    testseq_example.emplace_back(StrVectToTest(test_ex));
    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test in " + test_seq_param.mode.value + " mode with " + WR_MEM_RATE + " set to " + std::to_string(test_seq_param.write.rate.value) + "%:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
    LogMessage(MSG_ITF_077, {""});

    testseq_example.clear();
    test_ex.clear();
    test_seq_param.duration.value   = 10;
    test_seq_param.mode.value       = ONLY_RD;
    test_seq_param.read.rate.value  = 15;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    test_ex.emplace_back(GetUintTestSequenceString(RD_RATE_TEST_SEQ_MEMBER,    test_seq_param.read.rate));
    testseq_example.emplace_back(StrVectToTest(test_ex));
    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test in " + test_seq_param.mode.value + " mode with " + RD_MEM_RATE + " set to " + std::to_string(test_seq_param.read.rate.value) + "%:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
    LogMessage(MSG_ITF_077, {""});

    testseq_example.clear();
    test_ex.clear();
    test_seq_param.duration.value   = 30;
    test_seq_param.mode.value       = SIMULTANEOUS_WR_RD;
    test_seq_param.write.rate.value = 65;
    test_seq_param.read.rate.value  = 100;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    test_ex.emplace_back(GetUintTestSequenceString(WR_RATE_TEST_SEQ_MEMBER,    test_seq_param.write.rate));
    test_ex.emplace_back(GetUintTestSequenceString(RD_RATE_TEST_SEQ_MEMBER,    test_seq_param.read.rate));
    testseq_example.emplace_back(StrVectToTest(test_ex));
    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test in " + test_seq_param.mode.value + " mode with " + WR_MEM_RATE + " set to " + std::to_string(test_seq_param.write.rate.value) + "% and " + RD_MEM_RATE + " set to " + std::to_string(test_seq_param.read.rate.value) + "%:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
    LogMessage(MSG_ITF_077, {""});

    testseq_example.clear();
    test_ex.clear();
    test_seq_param.duration.value   = 30;
    test_seq_param.mode.value       = SIMULTANEOUS_WR_RD;
    test_seq_param.write.bandwidth.value = 6500;
    test_seq_param.read.bandwidth.value  = 6500;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    test_ex.emplace_back(GetUintTestSequenceString(WR_BANDWIDTH_TEST_SEQ_MEMBER,    test_seq_param.write.bandwidth));
    test_ex.emplace_back(GetUintTestSequenceString(RD_BANDWIDTH_TEST_SEQ_MEMBER,    test_seq_param.read.bandwidth));
    testseq_example.emplace_back(StrVectToTest(test_ex));
    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test in " + test_seq_param.mode.value + " mode requesting " + WR_MEM_BANDWIDTH + " set to " + std::to_string(test_seq_param.write.bandwidth.value) + "MBps and " + RD_MEM_BANDWIDTH + " set to " + std::to_string(test_seq_param.read.bandwidth.value) + "MBps:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
    LogMessage(MSG_ITF_077, {""});
}

void InputParser::PrintPowerTestSequenceGuide()
{
    const uint PWR_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + POWER_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : POWER_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', PWR_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', PWR_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + POWER_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_POWER_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value       = 40;
    test_seq_param.toggle_rate.value    = 30;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,       test_seq_param.duration));
    test_ex.emplace_back(GetUintTestSequenceString(TOGGLE_RATE_TEST_SEQ_MEMBER,    test_seq_param.toggle_rate));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    LogMessage(MSG_ITF_077, {"\t- " + std::to_string(test_seq_param.duration.value) + " seconds test at " + std::to_string(test_seq_param.toggle_rate.value) + "% toggle rate:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": [" + StrVectToStr(testseq_example, ",") + "]"});
    LogMessage(MSG_ITF_077, {""});
}

void InputParser::PrintGTMACTestSequenceGuide()
{
    const uint GT_MAC_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + GT_MAC_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : GT_MAC_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', GT_MAC_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', GT_MAC_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == MODE_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: " + StrSetToStr(QuoteStrSet(SUPPORTED_GT_MAC_SEQUENCE_MODE), ", ")});
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + GT_MAC_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_GTMAC_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_25GBE_RS_FEC_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CLEAR_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CHECK_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));
    LogMessage(MSG_ITF_077, {"\t- 60 seconds test with xbtest HW IP configured at 25 GbE:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": ["});
    for (uint i = 0; i < testseq_example.size(); i++)
    {
        auto str = "\t\t\t" + testseq_example[i];
        if (i < testseq_example.size() - 1)
        {
            str += ",";
        }
        LogMessage(MSG_ITF_077, {str});
    }
    LogMessage(MSG_ITF_077, {"\t\t]"});
    LogMessage(MSG_ITF_077, {""});
}

void InputParser::PrintGTLpbkTestSequenceGuide()
{
    const uint GT_LPBK_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + GT_LPBK_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : GT_LPBK_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', GT_LPBK_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', GT_LPBK_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == MODE_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: " + StrSetToStr(QuoteStrSet(SUPPORTED_GT_LPBK_SEQUENCE_MODE), ", ")});
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + GT_LPBK_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_GTLPBK_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_25GBE_NO_FEC_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    LogMessage(MSG_ITF_077, {"\t- 60 seconds test with xbtest HW IP:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": ["});
    for (uint i = 0; i < testseq_example.size(); i++)
    {
        auto str = "\t\t\t" + testseq_example[i];
        if (i < testseq_example.size() - 1)
        {
            str += ",";
        }
        LogMessage(MSG_ITF_077, {str});
    }
    LogMessage(MSG_ITF_077, {"\t\t]"});
    LogMessage(MSG_ITF_077, {""});
}

void InputParser::PrintGTPrbsTestSequenceGuide()
{
    const uint GT_PRBS_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + GT_PRBS_MEMBER.name + " testcase is defined by:"});
    for (const auto & json_val_def : GT_PRBS_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', GT_PRBS_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', GT_PRBS_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == MODE_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: " + StrSetToStr(QuoteStrSet(SUPPORTED_GT_PRBS_SEQUENCE_MODE), ", ")});
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + GT_PRBS_MEMBER.name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_GTPRBS_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_25GBE_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CLEAR_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CHECK_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    LogMessage(MSG_ITF_077, {"\t- 60 seconds test with xbtest HW IP:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": ["});
    for (uint i = 0; i < testseq_example.size(); i++)
    {
        auto str = "\t\t\t" + testseq_example[i];
        if (i < testseq_example.size() - 1)
        {
            str += ",";
        }
        LogMessage(MSG_ITF_077, {str});
    }
    LogMessage(MSG_ITF_077, {"\t\t]"});
    LogMessage(MSG_ITF_077, {""});
}

void InputParser::PrintMultiGTPrbsTestSequenceGuide( const std::string & multi_gt_name )
{
    const uint MULTI_GT_PRBS_GUIDE_PAD = 15;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + TEST_SEQUENCE_MEMBER.name + " parameter of the " + multi_gt_name + " testcase is defined by:"});
    for (const auto & json_val_def : MULTI_GT_PRBS_TEST_SEQ_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', MULTI_GT_PRBS_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', MULTI_GT_PRBS_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == MODE_TEST_SEQ_MEMBER.name)
        {
            LogMessage(MSG_ITF_077, {"\t\t* Possible values: " + StrSetToStr(QuoteStrSet(SUPPORTED_MULTI_GT_PRBS_SEQUENCE_MODE), ", ")});
        }
    }
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"Examples of " + TEST_SEQUENCE_MEMBER.name + " for the " + multi_gt_name + " testcase:"});

    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_MULTI_GTPRBS_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_MULTI_GT_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CLEAR_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CHECK_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    LogMessage(MSG_ITF_077, {"\t- 60 seconds test with xbtest HW IP:"});
    LogMessage(MSG_ITF_077, {""});
    LogMessage(MSG_ITF_077, {"\t\t\"" + TEST_SEQUENCE_MEMBER.name + "\": ["});
    for (uint i = 0; i < testseq_example.size(); i++)
    {
        auto str = "\t\t\t" + testseq_example[i];
        if (i < testseq_example.size() - 1)
        {
            str += ",";
        }
        LogMessage(MSG_ITF_077, {str});
    }
    LogMessage(MSG_ITF_077, {"\t\t]"});
    LogMessage(MSG_ITF_077, {""});
}

bool InputParser::PrintDeviceMmgmtSensorGuide()
{
    const uint DEVICE_MGMT_GUIDE_PAD  = 17;
    const uint DEVICE_MGMT_GUIDE_PAD2 = 3;
    auto ret = RET_SUCCESS;
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + SENSOR_MEMBER.name + " parameter of the " + DEVICE_MGMT_MEMBER.name + " task is defined by:"});
    for (const auto & json_val_def : DEVICE_MGMT_SENSOR_DEF)
    {
        if (json_val_def.hidden == HIDDEN_FALSE)
        {
            LogMessage(MSG_ITF_077, {"\t- " + pad(json_val_def.name, ' ', DEVICE_MGMT_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t- " + pad(json_val_def.name, ' ', DEVICE_MGMT_GUIDE_PAD, PAD_ON_RIGHT) + ": " + json_val_def.description});
        }
        if (json_val_def.name == SENSOR_ID_MEMBER.name)
        {
            auto electrical_ids = m_xbtest_sw_config->GetAllElectricalSourcesIDs();
            if (!electrical_ids.empty())
            {
                LogMessage(MSG_ITF_077, {"\t\t* Supported \"" + XRT_INFO_DEVICE_ELECTRICAL + "\" sensor IDs: " + StrVectToStr(QuoteStrVect(electrical_ids), ", ")});
            }
            else
            {
                LogMessage(MSG_ITF_077, {"\t\t* No \"" + XRT_INFO_DEVICE_ELECTRICAL + "\" sensor IDs supported"});
            }
            auto thermal_ids = m_xbtest_sw_config->GetAllThermalSourcesIDs();
            if (!thermal_ids.empty())
            {
                LogMessage(MSG_ITF_077, {"\t\t* Supported \"" + XRT_INFO_DEVICE_THERMAL + "\" sensor IDs: " + StrVectToStr(QuoteStrVect(thermal_ids), ", ")});
            }
            else
            {
                LogMessage(MSG_ITF_077, {"\t\t* No \"" + XRT_INFO_DEVICE_THERMAL + "\" sensor IDs supported"});
            }
            auto mechanical_ids = m_xbtest_sw_config->GetAllMechanicalSourcesIDs();
            if (!mechanical_ids.empty())
            {
                LogMessage(MSG_ITF_077, {"\t\t* Supported \"" + XRT_INFO_DEVICE_MECHANICAL + "\" sensor IDs: " + StrVectToStr(QuoteStrVect(mechanical_ids), ", ")});
            }
            else
            {
                LogMessage(MSG_ITF_077, {"\t\t* No \"" + XRT_INFO_DEVICE_MECHANICAL + "\" sensor IDs supported"});
            }
            LogMessage(MSG_ITF_077, {"\t\t* Here are the IDs of sensors recorded by default for each sensor type:"});
            LogMessage(MSG_ITF_077, {"\t\t\t+ \"" + XRT_INFO_DEVICE_ELECTRICAL + "\": " + StrVectToStr(QuoteStrVect(DEFAULT_ELECTRICAL_SOURCES), ", ")});
            LogMessage(MSG_ITF_077, {"\t\t\t+ \"" + XRT_INFO_DEVICE_THERMAL    + "\": " + StrVectToStr(QuoteStrVect(DEFAULT_THERMAL_SOURCES), ", ")});
            LogMessage(MSG_ITF_077, {"\t\t\t+ \"" + XRT_INFO_DEVICE_MECHANICAL + "\": " + StrVectToStr(QuoteStrVect(DEFAULT_MECHANICAL_SOURCES), ", ")});
        }
        if ((json_val_def.name == SENSOR_WARNING_THRESHOLD_MEMBER.name) || (json_val_def.name == SENSOR_ERROR_THRESHOLD_MEMBER.name) || (json_val_def.name == SENSOR_ABORT_THRESHOLD_MEMBER.name))
        {
            for (const auto & json_val_def2 : DEVICE_MGMT_THRESHOLD_DEF)
            {
                if (json_val_def2.hidden == HIDDEN_FALSE)
                {
                    LogMessage(MSG_ITF_077, {"\t\t* " + pad(json_val_def2.name, ' ', DEVICE_MGMT_GUIDE_PAD2, PAD_ON_RIGHT) + ": " + json_val_def2.description});
                }
                else
                {
                    LogMessage(MSG_DEBUG_SETUP, {"\t\t* " + pad(json_val_def2.name, ' ', DEVICE_MGMT_GUIDE_PAD2, PAD_ON_RIGHT) + ": " + json_val_def2.description});
                }
            }
        }
    }
    return ret;
}

void InputParser::PrintMmioJSONExample()
{
    std::vector<std::string> testseq_example;

    auto test_seq_param = RST_MMIO_TEST_SEQUENCE_PARAMETERS;
    test_seq_param.duration.value   = 10;
    std::vector<std::string> test_ex;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    testseq_example.emplace_back(StrVectToTest(test_ex));


    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "mmio": {
            "global_config": {
)";
test_json_example += "                \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                \"test_sequence\": [\n";
test_json_example += "                    " + StrVectToStr(testseq_example, ",\n                    ") + "\n";
test_json_example += "                ]";
test_json_example += R"(
            }
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + MMIO_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintDMAJSONExample()
{
    std::vector<std::string> testseq_example;
    for (const auto & available_board_memory : m_xbtest_sw_config->GetAvailableBoardMemories())
    {
        auto test_seq_param = RST_DMA_TEST_SEQUENCE_PARAMETERS;
        test_seq_param.duration.value   = 10;
        test_seq_param.target.value     = available_board_memory;
        std::vector<std::string> test_ex;
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER,      test_seq_param.target));
        testseq_example.emplace_back(StrVectToTest(test_ex));
    }

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "dma": {
            "global_config": {
)";
test_json_example += "                \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                \"test_sequence\": [\n";
test_json_example += "                    " + StrVectToStr(testseq_example, ",\n                    ") + "\n";
test_json_example += "                ]";
test_json_example += R"(
            }
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + DMA_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintP2PCardJSONExample()
{
    std::vector<std::string> testseq_example;
    for (const auto & available_board_memory : m_xbtest_sw_config->GetAvailableBoardMemories())
    {
        auto test_seq_param = RST_P2P_CARD_TEST_SEQUENCE_PARAMETERS;
        test_seq_param.duration.value   = 10;
        test_seq_param.source.value     = available_board_memory;
        std::vector<std::string> test_ex;
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER, test_seq_param.source));
        testseq_example.emplace_back(StrVectToTest(test_ex));
    }

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "p2p_card": {
            "global_config": {
)";
test_json_example += "                \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                \"test_sequence\": [\n";
test_json_example += "                    " + StrVectToStr(testseq_example, ",\n                    ") + "\n";
test_json_example += "                ]";
test_json_example += R"(
            }
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + P2P_CARD_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintP2P2NvmJSONExample()
{
    std::vector<std::string> testseq_example;
    for (const auto & available_board_memory : m_xbtest_sw_config->GetAvailableBoardMemories())
    {
        auto test_seq_param = RST_P2P_NVME_TEST_SEQUENCE_PARAMETERS;
        test_seq_param.duration.value   = 10;
        test_seq_param.source.value     = available_board_memory;
        test_seq_param.target.value     = available_board_memory;
        std::vector<std::string> test_ex;
        if (this->m_xbtest_sw_config->GetIsDmaSupported())
        {
            test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
            test_ex.emplace_back(GetStrTestSequenceString(SOURCE_TEST_SEQ_MEMBER_OPT, test_seq_param.source));
            testseq_example.emplace_back(StrVectToTest(test_ex));
        }
        test_ex.clear();
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER, test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(TARGET_TEST_SEQ_MEMBER_OPT, test_seq_param.target));
        testseq_example.emplace_back(StrVectToTest(test_ex));
    }

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "p2p_nvme": {
            "global_config": {
)";
test_json_example += "                \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                \"test_sequence\": [\n";
test_json_example += "                    " + StrVectToStr(testseq_example, ",\n                    ") + "\n";
test_json_example += "                ]";
test_json_example += R"(
            }
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + P2P_NVME_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintMemoryJSONExample()
{
    std::vector<std::string> testseq_example;
    for (const auto & mode : SUPPORTED_MEMORY_TC_MODE)
    {
        auto test_seq_param = RST_MEMORY_TEST_SEQUENCE_PARAMETERS;
        test_seq_param.duration.value   = 30;
        test_seq_param.mode.value       = mode;
        std::vector<std::string> test_ex;
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
        test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
        testseq_example.emplace_back(StrVectToTest(test_ex));
    }

    std::string test_json_example_on_board = "\n{\n";
test_json_example_on_board += TEST_JSON_EXAMPLE_COMMENT;
test_json_example_on_board += R"(
    "testcases": {
        "memory": {
)";
    uint ii=0;
    for (const auto & available_memory : m_xbtest_sw_config->GetAvailableBoardMemories())
    {
test_json_example_on_board += "            \""+ available_memory + "\": {\n";
test_json_example_on_board += "                \"global_config\": {\n";
test_json_example_on_board += "                    \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example_on_board += "                    \"test_sequence\": [\n";
test_json_example_on_board += "                       " + StrVectToStr(testseq_example, ",\n                       ") + "\n";
test_json_example_on_board += "                    ]";
test_json_example_on_board += R"(
                }
            })";
        if (ii < m_xbtest_sw_config->GetAllAvailableMemories().size() - 1)
        {
            test_json_example_on_board += ",\n";
        }
        ii++;
    }
        test_json_example_on_board += R"(
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + MEMORY_MEMBER.name + " testcase targeting on-board memory may look like: " + test_json_example_on_board});

    std::string test_json_example_host = "\n{\n";
test_json_example_host += TEST_JSON_EXAMPLE_COMMENT;
test_json_example_host += R"(
    "testcases": {
        "memory": {
)";
    uint jj=0;
    for (const auto & available_memory : m_xbtest_sw_config->GetAvailableHostMemories())
    {
test_json_example_host+=  "            \"" + available_memory + "\": {\n";
test_json_example_host += "                 \"global_config\": {\n";
test_json_example_host += "                     \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example_host += "                     \"test_sequence\": [\n";
test_json_example_host += "                         " + StrVectToStr(testseq_example, ",\n                         ") + "\n";
test_json_example_host += "                     ]";
test_json_example_host+=R"(
                }
            })";
        if (jj < m_xbtest_sw_config->GetAllAvailableMemories().size() - 1)
        {
            test_json_example_host += ",\n";
        }
        jj++;
    }
        test_json_example_host += R"(
        }
    }
})";
    if (!m_xbtest_sw_config->GetAvailableHostMemories().empty())
    {
        LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + MEMORY_MEMBER.name + " testcase targeting host memory may look like: " + test_json_example_host});
    }
}

void InputParser::PrintPowerJSONExample()
{
    std::vector<std::string> testseq_example;
    for (const auto & toggle_rate : {15, 20})
    {
        auto test_seq_param = RST_POWER_TEST_SEQUENCE_PARAMETERS;
        test_seq_param.duration.value       = 60;
        test_seq_param.toggle_rate.value    = toggle_rate;
        std::vector<std::string> test_ex;
        test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,       test_seq_param.duration));
        test_ex.emplace_back(GetUintTestSequenceString(TOGGLE_RATE_TEST_SEQ_MEMBER,    test_seq_param.toggle_rate));
        testseq_example.emplace_back(StrVectToTest(test_ex));
    }

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "power": {
            "comment"      : "Use comment to detail your test if necessary (you can also remove this line) ",
            "global_config": {
)";
test_json_example += "                \"test_sequence\": [\n";
test_json_example += "                    " + StrVectToStr(testseq_example, ",\n                    ") + "\n";
test_json_example += "                ]";
test_json_example += R"(
            }
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + POWER_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintGTMACJSONExample()
{
    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_GTMAC_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_25GBE_RS_FEC_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CLEAR_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CHECK_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "gt_mac": {
)";
test_json_example += "            \"comment\" : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
    uint ii=0;
    for (const auto & gt_index : m_xbtest_sw_config->GetAvailableGTMACs())
    {
test_json_example+=  "            \"" + std::to_string(gt_index) + "\": {\n";
test_json_example += "                \"global_config\": {\n";
test_json_example += "                    \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                    \"test_sequence\": [\n";
test_json_example += "                        " + StrVectToStr(testseq_example, ",\n                        ") + "\n";
test_json_example += "                    ]";
test_json_example += R"(
                }
            })";
        if (ii < m_xbtest_sw_config->GetAvailableGTMACs().size()-1)
        {
            test_json_example += ",\n";
        }
        ii++;
    }
    test_json_example += R"(
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + GT_MAC_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintGTLpbkJSONExample()
{
    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_GTLPBK_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_25GBE_NO_FEC_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "gt_lpbk": {
)";
test_json_example += "            \"comment\" : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
    uint ii=0;
    for (const auto & gt_index : m_xbtest_sw_config->GetAvailableGTLpbks())
    {
test_json_example+=  "            \"" + std::to_string(gt_index) + "\": {\n";
test_json_example += "                \"global_config\": {\n";
test_json_example += "                    \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                    \"test_sequence\": [\n";
test_json_example += "                        " + StrVectToStr(testseq_example, ",\n                        ") + "\n";
test_json_example += "                    ]";
test_json_example += R"(
                }
            })";
        if (ii < m_xbtest_sw_config->GetAvailableGTLpbks().size()-1)
        {
            test_json_example += ",\n";
        }
        ii++;
    }
    test_json_example += R"(
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + GT_LPBK_MEMBER.name + " testcase may look like: " + test_json_example});
}


void InputParser::PrintGTPrbsJSONExample()
{
    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_GTPRBS_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_25GBE_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CLEAR_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CHECK_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "testcases": {
        "gt_prbs": {
)";
test_json_example += "            \"comment\" : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
    uint ii=0;
    for (const auto & gt_index : m_xbtest_sw_config->GetAvailableGTPrbss())
    {
test_json_example+=  "            \"" + std::to_string(gt_index) + "\": {\n";
test_json_example += "                \"global_config\": {\n";
test_json_example += "                    \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                    \"test_sequence\": [\n";
test_json_example += "                        " + StrVectToStr(testseq_example, ",\n                        ") + "\n";
test_json_example += "                    ]";
test_json_example += R"(
                }
            })";
        if (ii < m_xbtest_sw_config->GetAvailableGTPrbss().size()-1)
        {
            test_json_example += ",\n";
        }
        ii++;
    }
    test_json_example += R"(
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + GT_PRBS_MEMBER.name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintMultiGTPrbsJSONExample( const std::string & multi_gt_name )
{
    std::vector<std::string> testseq_example;
    std::vector<std::string> test_ex;
    auto test_seq_param = RST_MULTI_GTPRBS_TEST_SEQUENCE_PARAMETERS;

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CONF_MULTI_GT_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CLEAR_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 60;
    test_seq_param.mode.value     = RUN_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    test_ex.clear();
    test_seq_param.duration.value = 1;
    test_seq_param.mode.value     = CHECK_STATUS_STR;
    test_ex.emplace_back(GetUintTestSequenceString(DURATION_TEST_SEQ_MEMBER,   test_seq_param.duration));
    test_ex.emplace_back(GetStrTestSequenceString(MODE_TEST_SEQ_MEMBER,        test_seq_param.mode));
    testseq_example.emplace_back(StrVectToTest(test_ex));

    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
/*
test_json_example += R"(
    "testcases": {
        "gtf_prbs": {
)";
*/
test_json_example += R"(
    "testcases": {
        ")" + multi_gt_name + R"(": {
)";

test_json_example += "            \"comment\" : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
    uint ii=0;
    for (const auto & gt_index : m_xbtest_sw_config->GetAvailableMultiGTPrbss(multi_gt_name))
    {
test_json_example+=  "            \"" + std::to_string(gt_index) + "\": {\n";
test_json_example += "                \"global_config\": {\n";
test_json_example += "                    \"comment\"      : \"Use comment to detail your test if necessary (you can also remove this line)\",\n";
test_json_example += "                    \"test_sequence\": [\n";
test_json_example += "                        " + StrVectToStr(testseq_example, ",\n                        ") + "\n";
test_json_example += "                    ]";
test_json_example += R"(
                }
            })";
        if (ii < m_xbtest_sw_config->GetAvailableMultiGTPrbss(multi_gt_name).size()-1)
        {
            test_json_example += ",\n";
        }
        ii++;
    }
    test_json_example += R"(
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + multi_gt_name + " testcase may look like: " + test_json_example});
}

void InputParser::PrintDeviceMmgmtJSONExample()
{
    std::string test_json_example = "\n{\n";
test_json_example += TEST_JSON_EXAMPLE_COMMENT;
test_json_example += R"(
    "tasks": {
        "device_mgmt": {
            "comment": "Use comment to detail your test if necessary (you can also remove this line) ",
            "sensor" : [
                {
)";
test_json_example += "                    \"" + SENSOR_ID_MEMBER.name + "\"   : \"" + ID_12V_PEX + "\",\n";
test_json_example += "                    \"" + SENSOR_TYPE_MEMBER.name + "\" : \"" + XRT_INFO_DEVICE_ELECTRICAL + "\",";
test_json_example += R"(
                    "warning_threshold" : {
                        "min": 1.0,
                        "max": 65.0
                    },
                    "error_threshold" : {
                        "min": 0.1,
                        "max": 70.0
                    },
                    "abort_threshold" : {
                        "min": 0.0,
                        "max": 75.0
                    }
                }
            ]
        }
    }
})";
    LogMessage(MSG_ITF_077, {LOG_LINE_DELIMITER});
    LogMessage(MSG_ITF_077, {"The " + m_content_name + " defining a basic " + DEVICE_MGMT_MEMBER.name + " task may look like: " + test_json_example});
}

} // namespace
