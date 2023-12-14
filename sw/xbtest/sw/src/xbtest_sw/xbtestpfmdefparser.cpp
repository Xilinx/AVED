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

#include <cmath>

#include "xbtestpfmdefparser.h"

namespace xbtest
{

XbtestPfmDefParser::XbtestPfmDefParser(
    Logging * log, const std::string & filename, const bool & is_nodma, const Pcie_Info_t & pcie_info, bool is_p2p_supported, bool is_mmio_test_supported, std::atomic<bool> * abort
) : XJsonParser::XJsonParser(log, abort)
{
    this->m_log_header    = LOG_HEADER_PFM_DEF;

    this->m_filename                = filename;
    this->m_is_nodma                = is_nodma;
    this->m_pcie_info               = pcie_info;
    this->m_is_p2p_supported        = is_p2p_supported;
    this->m_is_mmio_test_supported  = is_mmio_test_supported;

    this->m_content_name  = XBTEST_PFM_DEF;
}

XbtestPfmDefParser::~XbtestPfmDefParser()
{
    ClearParser();
}

bool XbtestPfmDefParser::Parse()
{
    // Initialize json parser and reader
    LogMessage(MSG_ITF_045, {this->m_content_name, this->m_filename});
    this->m_json_parser = json_parser_new();

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_content_name, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free (error);
        g_object_unref (this->m_json_parser);
        return RET_FAILURE;
    }

    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    this->m_json_reader = json_reader_new(nullptr);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    // Check the json file content against the defined members
    LogMessage(MSG_DEBUG_PARSING, {"Check the JSON file content"});
    if (CheckMembers(GetJsonDefinition()) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    // Get JSON parameters
    LogMessage(MSG_DEBUG_PARSING, {"Get " + XBTEST_PFM_DEF + " parameters"});
    std::vector<std::string> node_title;
    auto found = false;

    // Download time
    node_title = {DEVICE, RUNTIME, DOWNLOAD_TIME};
    LogMessage(MSG_DEBUG_PARSING, {"Get " + StrVectToStr(node_title, ".") + " parameters"});
    if (ExtractRequiredUint(node_title, {}, this->m_xbtest_pfm_def.runtime.download_time) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Sensor
    node_title = {DEVICE, SENSOR};
    LogMessage(MSG_DEBUG_PARSING, {"Get " + StrVectToStr(node_title, ".") + " parameters"});

    this->m_xbtest_pfm_def.sensor.clear();
    std::vector<std::string> source_ids;

    auto card_power_found = false;
    auto card_temperature_found = false;

    if (NodeExists(node_title))
    {
        // Check if first index exists
        node_title = {DEVICE, SENSOR, "0"};
        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Get sensor sources
        auto parse_array_end = false;
        for (uint idx = 0; idx < MAX_SENSORS; idx++)
        {
            node_title = {DEVICE, SENSOR, std::to_string(idx)};
            if (!NodeExists(node_title))
            {
                parse_array_end = true;
                continue; // Finished get sensor sources
            }
            if (parse_array_end)
            {
                LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEVICE, SENSOR}, ".") });
                return RET_FAILURE;
            }

            LogMessage(MSG_DEBUG_PARSING, {"\t - Get sensor sources[" + std::to_string(idx) + "]"});
            Xbtest_Pfm_Def_Sensor_Source_t source;
            source.card_power = false;
            source.card_temperature = false;

            // ID
            if (ExtractRequiredString(node_title, {ID}, source.id) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (FindStringInVect(source.id, source_ids))
            {
                LogMessage(MSG_ITF_129, {this->m_content_name, source.id});
                return RET_FAILURE;
            }
            source_ids.emplace_back(source.id);
            // TYPE
            if (ExtractRequiredString(node_title, {TYPE}, source.type) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!FindStringInSet(source.type, SUPPORTED_SENSOR_TYPES))
            {
                LogMessage(MSG_CMN_001, {source.type, TYPE, StrSetToStr(QuoteStrSet(SUPPORTED_SENSOR_TYPES), ", ")});
                return RET_FAILURE;
            }
            // CARD_POWER
            if (ExtractOptionalBool(node_title, {CARD_POWER}, source.card_power, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (found && card_power_found)
            {
                LogMessage(MSG_CMN_054, {"power"});
                return RET_FAILURE;
            }
            // CARD_TEMPERATURE
            if (ExtractOptionalBool(node_title, {CARD_TEMPERATURE}, source.card_temperature, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (found && card_temperature_found)
            {
                LogMessage(MSG_CMN_054, {"temperature"});
                return RET_FAILURE;
            }
            this->m_xbtest_pfm_def.sensor.emplace_back(source);
        }
    }

    // GT
    node_title = {DEVICE, GT};
    LogMessage(MSG_DEBUG_PARSING, {"Get " + StrVectToStr(node_title, ".") + " parameters"});
    this->m_xbtest_pfm_def.gt.num_gt = 0;
    if (NodeExists(node_title))
    {
        // Check if first index exists
        node_title = {DEVICE, GT, "0"};
        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        // Get GT
        // GT indexing must start from 0
        auto parse_array_end = false;
        for (uint idx = 0; idx < MAX_GT_QTY; idx++)
        {
            Xbtest_Pfm_Def_GT_Settings_t gt_settings;

            node_title = {DEVICE, GT , std::to_string(idx)};
            if (!NodeExists(node_title))
            {
                parse_array_end = true;
                continue; // Finished get GTs
            }

            if (parse_array_end)
            {
                LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEVICE, GT}, ".") });
                return RET_FAILURE;
            }
            // NAME
            gt_settings.name = "none";
            if (ExtractOptionalString(node_title, {NAME}, gt_settings.name, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            LogMessage(MSG_DEBUG_PARSING, {"Get GT[" + std::to_string(idx) + "] transceiver settings"});
            node_title = {DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS};

            LogMessage(MSG_DEBUG_PARSING, {"\t - Get GT[" + std::to_string(idx) + "] cable settings"});
            node_title.emplace_back(GT_TRANS_SET_CABLE);

            // TX_POLARITY
            gt_settings.transceivers.cable.tx_polarity = TX_POLARITY_NORMAL;
            if (ExtractOptionalString(node_title, {TX_POLARITY}, gt_settings.transceivers.cable.tx_polarity, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_MAIN_CURSOR
            gt_settings.transceivers.cable.tx_main_cursor = DEFAULT_GT_TX_MAIN_CURSOR;
            if (ExtractOptionalUint(node_title, {TX_MAIN_CURSOR}, gt_settings.transceivers.cable.tx_main_cursor, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_DIFF_CTRL
            if (ExtractRequiredUint(node_title, {TX_DIFF_CTRL}, gt_settings.transceivers.cable.tx_differential_swing_control) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_PRE_EMPH
            if (ExtractRequiredUint(node_title, {TX_PRE_EMPH},  gt_settings.transceivers.cable.tx_pre_emphasis) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_POST_EMPH
            if (ExtractRequiredUint(node_title, {TX_POST_EMPH}, gt_settings.transceivers.cable.tx_post_emphasis) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // RX_EQUALISER
            if (ExtractRequiredString(node_title, {RX_EQUALISER}, gt_settings.transceivers.cable.rx_equalizer) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // RX_POLARITY
            gt_settings.transceivers.cable.rx_polarity = RX_POLARITY_NORMAL;
            if (ExtractOptionalString(node_title, {RX_POLARITY}, gt_settings.transceivers.cable.rx_polarity, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            node_title.pop_back(); // remove "gt_trans_set_cable"

            LogMessage(MSG_DEBUG_PARSING, {"\t- Get GT[" + std::to_string(idx) + "] module settings"});
            node_title.emplace_back(GT_TRANS_SET_MODULE);
            // TX_POLARITY
            gt_settings.transceivers.module.tx_polarity = TX_POLARITY_NORMAL;
            if (ExtractOptionalString(node_title, {TX_POLARITY}, gt_settings.transceivers.module.tx_polarity, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_MAIN_CURSOR
            gt_settings.transceivers.module.tx_main_cursor = DEFAULT_GT_TX_MAIN_CURSOR;
            if (ExtractOptionalUint(node_title, {TX_MAIN_CURSOR}, gt_settings.transceivers.module.tx_main_cursor, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_DIFF_CTRL
            if (ExtractRequiredUint(node_title, {TX_DIFF_CTRL}, gt_settings.transceivers.module.tx_differential_swing_control) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_PRE_EMPH
            if (ExtractRequiredUint(node_title, {TX_PRE_EMPH},  gt_settings.transceivers.module.tx_pre_emphasis) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // TX_POST_EMPH
            if (ExtractRequiredUint(node_title, {TX_POST_EMPH}, gt_settings.transceivers.module.tx_post_emphasis) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // RX_EQUALISER
            if (ExtractRequiredString(node_title, {RX_EQUALISER}, gt_settings.transceivers.module.rx_equalizer) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // RX_POLARITY
            gt_settings.transceivers.module.rx_polarity = RX_POLARITY_NORMAL;
            if (ExtractOptionalString(node_title, {RX_POLARITY}, gt_settings.transceivers.module.rx_polarity, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            node_title.pop_back(); // remove "gt_trans_set_module"

            this->m_xbtest_pfm_def.gt.settings.emplace_back(gt_settings);
            this->m_xbtest_pfm_def.gt.num_gt++;
        }
    }

    // MMIO
    if (this->m_is_mmio_test_supported)
    {
        Xbtest_Pfm_Def_Mmio_t mmio;

        node_title = {DEVICE, MMIO};
        LogMessage(MSG_DEBUG_PARSING, {"Get " + StrVectToStr(node_title, ".") + " parameters"});

        // mmio.bw. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
        node_title = {DEVICE, MMIO, BW};
        mmio.bw_exists    = false;

        bool average_found;

        // Compute default MMIO test BW thresholds
        auto default_mmio_thresh_high   = MAX_PCIE_BW_1_1_x_1 * (double)((1<<(this->m_pcie_info.expected_link_speed_gbit_sec-1))) * (double)(this->m_pcie_info.expected_express_lane_width_count);
        auto default_mmio_thresh_low    = (double)(1);
#ifdef USE_NO_DRIVER
        default_mmio_thresh_high    = (double)(MAX_UINT_VAL);
        default_mmio_thresh_low     = (double)(1);
#endif
        auto default_mmio_thresh_average = (default_mmio_thresh_low + default_mmio_thresh_high) / (double)(2);

        LogMessage(MSG_DEBUG_SETUP, {"Based on expected PCIe link speed " + std::to_string(this->m_pcie_info.expected_link_speed_gbit_sec) + " and lane width " + std::to_string(this->m_pcie_info.expected_express_lane_width_count) + ", default MMIO bandwidth thresholds (MBps): [" + Float_to_String(default_mmio_thresh_low,0) + ";" + Float_to_String(default_mmio_thresh_high,0) + "]"});

        // WRITE,AVERAGE
        mmio.bw.write.average = default_mmio_thresh_average;
        if (ExtractOptionalUint(node_title, {WRITE,AVERAGE}, mmio.bw.write.average, average_found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.bw_exists |= average_found;
        // WRITE,HIGH
        mmio.bw.write.high = default_mmio_thresh_high;
        if (average_found)
        {
            mmio.bw.write.high = std::ceil((double)(mmio.bw.write.average) + MMIO_AVG_TO_HI_LO_PERCENT * (double)(mmio.bw.write.average) / (double)(100));
        }
        if (ExtractOptionalUint(node_title, {WRITE,HIGH}, mmio.bw.write.high, found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.bw_exists |= found;
        // WRITE,LOW
        mmio.bw.write.low = default_mmio_thresh_low;
        if (average_found)
        {
            mmio.bw.write.low = std::ceil((double)(mmio.bw.write.average) - MMIO_AVG_TO_HI_LO_PERCENT * (double)(mmio.bw.write.average) / (double)(100));
        }
        if (ExtractOptionalUint(node_title, {WRITE,LOW}, mmio.bw.write.low, found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.bw_exists |= found;
        // READ,AVERAGE
        mmio.bw.read.average = default_mmio_thresh_average;
        if (ExtractOptionalUint(node_title, {READ,AVERAGE}, mmio.bw.read.average, average_found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.bw_exists |= average_found;
        // READ,HIGH
        mmio.bw.read.high = default_mmio_thresh_high;
        if (average_found)
        {
            mmio.bw.read.high = std::ceil((double)(mmio.bw.read.average) + MMIO_AVG_TO_HI_LO_PERCENT * (double)(mmio.bw.read.average) / (double)(100));
        }
        if (ExtractOptionalUint(node_title, {READ,HIGH}, mmio.bw.read.high, found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.bw_exists |= found;
        // READ,LOW
        mmio.bw.read.low = default_mmio_thresh_low;
        if (average_found)
        {
            mmio.bw.read.low = std::ceil((double)(mmio.bw.read.average) - MMIO_AVG_TO_HI_LO_PERCENT * (double)(mmio.bw.read.average) / (double)(100));
        }
        if (ExtractOptionalUint(node_title, {READ,LOW}, mmio.bw.read.low, found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.bw_exists |= found;

        // mmio.config. Get if exist
        node_title = {DEVICE, MMIO, CONFIG};
        mmio.config = RST_MMIO_CONFIG;

        // TOTAL_SIZE
        if (ExtractOptionalUint64(node_title, {TOTAL_SIZE}, mmio.config.total_size, mmio.config.total_size_exists) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // BUFFERSIZE
        if (ExtractOptionalUint64(node_title, {BUFFERSIZE}, mmio.config.buffer_size, mmio.config.buffer_size_exists) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // BAR
        if (ExtractOptionalUint8(node_title, {BAR}, mmio.config.bar, mmio.config.bar_exists) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // OFFSET
        if (ExtractOptionalUint64(node_title, {OFFSET}, mmio.config.offset, mmio.config.offset_exists) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        mmio.config_exists = mmio.config.total_size_exists || mmio.config.buffer_size_exists || mmio.config.bar_exists || mmio.config.offset_exists;

        this->m_xbtest_pfm_def.mmio = mmio;
    }

    node_title = {DEVICE, MEMORY};
    LogMessage(MSG_DEBUG_PARSING, {"Get " + StrVectToStr(node_title, ".") + " parameters"});

    this->m_xbtest_pfm_def.memory.clear();
    if (NodeExists(node_title))
    {
        // Check if first index exists
        node_title = {DEVICE, MEMORY, "0"};
        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        std::set<std::string> xbtest_pfm_def_memory_names;
        bool average_found;

        // Compute default DMA BW thresholds
        auto default_dma_thresh_high        = MAX_PCIE_BW_1_1_x_1 * (double)((1<<(this->m_pcie_info.expected_link_speed_gbit_sec-1))) * (double)(this->m_pcie_info.expected_express_lane_width_count);
        auto default_dma_thresh_low         = default_dma_thresh_high - (DMA_DEFAULT_HI_TO_LO_PERCENT * default_dma_thresh_high) / (double)(100);

        // Compute default P2P_CARD BW thresholds
        auto default_p2p_card_thresh_high   = MAX_PCIE_BW_1_1_x_1 * (double)((1<<(this->m_pcie_info.expected_link_speed_gbit_sec-1))) * (double)(this->m_pcie_info.expected_express_lane_width_count);
        auto default_p2p_card_thresh_low    = (double)(1);

        // Compute default P2P_NVME BW thresholds
        auto default_p2p_nvme_thresh_high   = MAX_PCIE_BW_1_1_x_1 * (double)((1<<(this->m_pcie_info.expected_link_speed_gbit_sec-1))) * (double)(this->m_pcie_info.expected_express_lane_width_count);
        auto default_p2p_nvme_thresh_low    = (double)(1);
#ifdef USE_NO_DRIVER
        default_dma_thresh_high         = (double)(MAX_UINT_VAL);
        default_dma_thresh_low          = (double)(1);

        default_p2p_card_thresh_high    = (double)(MAX_UINT_VAL);
        default_p2p_card_thresh_low     = (double)(1);

        default_p2p_nvme_thresh_high    = (double)(MAX_UINT_VAL);
        default_p2p_nvme_thresh_low     = (double)(1);
#endif
        auto default_dma_thresh_average         = (default_dma_thresh_low + default_dma_thresh_high) / (double)(2);
        auto default_p2p_card_thresh_average    = (default_p2p_card_thresh_low + default_p2p_card_thresh_high) / (double)(2);
        auto default_p2p_nvme_thresh_average    = (default_p2p_nvme_thresh_low + default_p2p_nvme_thresh_high) / (double)(2);

        if (!(this->m_is_nodma))
        {
            LogMessage(MSG_DEBUG_SETUP, {"Based on expected PCIe link speed " + std::to_string(this->m_pcie_info.expected_link_speed_gbit_sec) + " and lane width " + std::to_string(this->m_pcie_info.expected_express_lane_width_count) + ", default DMA bandwidth thresholds (MBps): [" + Float_to_String(default_dma_thresh_low,0) + ";" + Float_to_String(default_dma_thresh_high,0) + "]"});
        }
        if (this->m_is_p2p_supported)
        {
            LogMessage(MSG_DEBUG_SETUP, {"Based on expected PCIe link speed " + std::to_string(this->m_pcie_info.expected_link_speed_gbit_sec) + " and lane width " + std::to_string(this->m_pcie_info.expected_express_lane_width_count) + ", default P2P_CARD bandwidth thresholds (MBps): [" + Float_to_String(default_p2p_card_thresh_low,0) + ";" + Float_to_String(default_p2p_card_thresh_high,0) + "]"});
            LogMessage(MSG_DEBUG_SETUP, {"Based on expected PCIe link speed " + std::to_string(this->m_pcie_info.expected_link_speed_gbit_sec) + " and lane width " + std::to_string(this->m_pcie_info.expected_express_lane_width_count) + ", default P2P_NVME bandwidth thresholds (MBps): [" + Float_to_String(default_p2p_nvme_thresh_low,0) + ";" + Float_to_String(default_p2p_nvme_thresh_high,0) + "]"});
        }
        // Get Memory settings
        auto parse_array_end = false;
        for (uint idx = 0; idx < MAX_MEM_TYPE_QTY; idx++)
        {
            Xbtest_Pfm_Def_Memory_t memory;

            node_title = {DEVICE, MEMORY, std::to_string(idx)};
            if (!NodeExists(node_title))
            {
                parse_array_end = true;
                continue; // Finished get memory settings
            }
            if (parse_array_end)
            {
                LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEVICE, MEMORY}, ".") });
                return RET_FAILURE;
            }

            // NAME
            if (ExtractRequiredString(node_title, {NAME}, memory.name) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            LogMessage(MSG_DEBUG_PARSING, {"Get memory parameters for " + memory.name});

            // check this memory was not already defined
            if (xbtest_pfm_def_memory_names.count(memory.name) > 0)
            {
                LogMessage(MSG_ITF_098, {"memory name", memory.name});
                return RET_FAILURE;
            }
            xbtest_pfm_def_memory_names.insert(memory.name);

            // dma_bw. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
            node_title = {DEVICE, MEMORY, std::to_string(idx), DMA_BW};
            memory.dma_bw_exists    = false;

            // WRITE,AVERAGE
            memory.dma_bw.write.average = default_dma_thresh_average;
            if (ExtractOptionalUint(node_title, {WRITE,AVERAGE}, memory.dma_bw.write.average, average_found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_bw_exists |= average_found;
            // WRITE,HIGH
            memory.dma_bw.write.high = default_dma_thresh_high;
            if (average_found)
            {
                memory.dma_bw.write.high = std::ceil((double)(memory.dma_bw.write.average) + DMA_AVG_TO_HI_LO_PERCENT * (double)(memory.dma_bw.write.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {WRITE,HIGH}, memory.dma_bw.write.high, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_bw_exists |= found;
            // WRITE,LOW
            memory.dma_bw.write.low = default_dma_thresh_low;
            if (average_found)
            {
                memory.dma_bw.write.low = std::ceil((double)(memory.dma_bw.write.average) - DMA_AVG_TO_HI_LO_PERCENT * (double)(memory.dma_bw.write.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {WRITE,LOW}, memory.dma_bw.write.low, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_bw_exists |= found;
            // READ,AVERAGE
            memory.dma_bw.read.average = default_dma_thresh_average;
            if (ExtractOptionalUint(node_title, {READ,AVERAGE}, memory.dma_bw.read.average, average_found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_bw_exists |= average_found;
            // READ,HIGH
            memory.dma_bw.read.high = default_dma_thresh_high;
            if (average_found)
            {
                memory.dma_bw.read.high = std::ceil((double)(memory.dma_bw.read.average) + DMA_AVG_TO_HI_LO_PERCENT * (double)(memory.dma_bw.read.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {READ,HIGH}, memory.dma_bw.read.high, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_bw_exists |= found;
            // READ,LOW
            memory.dma_bw.read.low = default_dma_thresh_low;
            if (average_found)
            {
                memory.dma_bw.read.low = std::ceil((double)(memory.dma_bw.read.average) - DMA_AVG_TO_HI_LO_PERCENT * (double)(memory.dma_bw.read.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {READ,LOW}, memory.dma_bw.read.low, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_bw_exists |= found;

            // dma_config. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
            node_title = {DEVICE, MEMORY, std::to_string(idx), DMA_CONFIG};
            // TOTAL_SIZE
            if (ExtractOptionalUint(node_title, {TOTAL_SIZE}, memory.dma_config.total_size, memory.dma_config.total_size_exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // BUFFERSIZE
            if (ExtractOptionalUint(node_title, {BUFFERSIZE}, memory.dma_config.buffer_size, memory.dma_config.buffer_size_exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.dma_config_exists = memory.dma_config.total_size_exists || memory.dma_config.buffer_size_exists;

            // p2p_card_bw. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
            node_title = {DEVICE, MEMORY, std::to_string(idx), P2P_CARD_BW};
            memory.p2p_card_bw_exists    = false;

            // WRITE,AVERAGE
            memory.p2p_card_bw.write.average = default_p2p_card_thresh_average;
            if (ExtractOptionalUint(node_title, {WRITE,AVERAGE}, memory.p2p_card_bw.write.average, average_found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_bw_exists |= average_found;
            // WRITE,HIGH
            memory.p2p_card_bw.write.high = default_p2p_card_thresh_high;
            if (average_found)
            {
                memory.p2p_card_bw.write.high = std::ceil((double)(memory.p2p_card_bw.write.average) + P2P_CARD_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_card_bw.write.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {WRITE,HIGH}, memory.p2p_card_bw.write.high, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_bw_exists |= found;
            // WRITE,LOW
            memory.p2p_card_bw.write.low = default_p2p_card_thresh_low;
            if (average_found)
            {
                memory.p2p_card_bw.write.low = std::ceil((double)(memory.p2p_card_bw.write.average) - P2P_CARD_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_card_bw.write.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {WRITE,LOW}, memory.p2p_card_bw.write.low, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_bw_exists |= found;
            // READ,AVERAGE
            memory.p2p_card_bw.read.average = default_p2p_card_thresh_average;
            if (ExtractOptionalUint(node_title, {READ,AVERAGE}, memory.p2p_card_bw.read.average, average_found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_bw_exists |= average_found;
            // READ,HIGH
            memory.p2p_card_bw.read.high = default_p2p_card_thresh_high;
            if (average_found)
            {
                memory.p2p_card_bw.read.high = std::ceil((double)(memory.p2p_card_bw.read.average) + P2P_CARD_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_card_bw.read.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {READ,HIGH}, memory.p2p_card_bw.read.high, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_bw_exists |= found;
            // READ,LOW
            memory.p2p_card_bw.read.low = default_p2p_card_thresh_low;
            if (average_found)
            {
                memory.p2p_card_bw.read.low = std::ceil((double)(memory.p2p_card_bw.read.average) - P2P_CARD_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_card_bw.read.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {READ,LOW}, memory.p2p_card_bw.read.low, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_bw_exists |= found;

            // p2p_card_config. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
            node_title = {DEVICE, MEMORY, std::to_string(idx), P2P_CARD_CONFIG};
            // TOTAL_SIZE
            if (ExtractOptionalUint(node_title, {TOTAL_SIZE}, memory.p2p_card_config.total_size, memory.p2p_card_config.total_size_exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // BUFFERSIZE
            if (ExtractOptionalUint(node_title, {BUFFERSIZE}, memory.p2p_card_config.buffer_size, memory.p2p_card_config.buffer_size_exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_card_config_exists = memory.p2p_card_config.total_size_exists || memory.p2p_card_config.buffer_size_exists;

            // p2p_nvme_bw. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
            node_title = {DEVICE, MEMORY, std::to_string(idx), P2P_NVME_BW};
            memory.p2p_nvme_bw_exists    = false;

            // WRITE,AVERAGE
            memory.p2p_nvme_bw.write.average = default_p2p_nvme_thresh_average;
            if (ExtractOptionalUint(node_title, {WRITE,AVERAGE}, memory.p2p_nvme_bw.write.average, average_found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_bw_exists |= average_found;
            // WRITE,HIGH
            memory.p2p_nvme_bw.write.high = default_p2p_nvme_thresh_high;
            if (average_found)
            {
                memory.p2p_nvme_bw.write.high = std::ceil((double)(memory.p2p_nvme_bw.write.average) + P2P_NVME_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_nvme_bw.write.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {WRITE,HIGH}, memory.p2p_nvme_bw.write.high, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_bw_exists |= found;
            // WRITE,LOW
            memory.p2p_nvme_bw.write.low = default_p2p_nvme_thresh_low;
            if (average_found)
            {
                memory.p2p_nvme_bw.write.low = std::ceil((double)(memory.p2p_nvme_bw.write.average) - P2P_NVME_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_nvme_bw.write.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {WRITE,LOW}, memory.p2p_nvme_bw.write.low, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_bw_exists |= found;
            // READ,AVERAGE
            memory.p2p_nvme_bw.read.average = default_p2p_nvme_thresh_average;
            if (ExtractOptionalUint(node_title, {READ,AVERAGE}, memory.p2p_nvme_bw.read.average, average_found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_bw_exists |= average_found;
            // READ,HIGH
            memory.p2p_nvme_bw.read.high = default_p2p_nvme_thresh_high;
            if (average_found)
            {
                memory.p2p_nvme_bw.read.high = std::ceil((double)(memory.p2p_nvme_bw.read.average) + P2P_NVME_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_nvme_bw.read.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {READ,HIGH}, memory.p2p_nvme_bw.read.high, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_bw_exists |= found;
            // READ,LOW
            memory.p2p_nvme_bw.read.low = default_p2p_nvme_thresh_low;
            if (average_found)
            {
                memory.p2p_nvme_bw.read.low = std::ceil((double)(memory.p2p_nvme_bw.read.average) - P2P_NVME_AVG_TO_HI_LO_PERCENT * (double)(memory.p2p_nvme_bw.read.average) / (double)(100));
            }
            if (ExtractOptionalUint(node_title, {READ,LOW}, memory.p2p_nvme_bw.read.low, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_bw_exists |= found;

            // p2p_nvme_config. Get if exist, xbtest_sw_config will check this is defined only for board memory and not host
            node_title = {DEVICE, MEMORY, std::to_string(idx), P2P_NVME_CONFIG};
            // TOTAL_SIZE
            if (ExtractOptionalUint(node_title, {TOTAL_SIZE}, memory.p2p_nvme_config.total_size, memory.p2p_nvme_config.total_size_exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // BUFFERSIZE
            if (ExtractOptionalUint(node_title, {BUFFERSIZE}, memory.p2p_nvme_config.buffer_size, memory.p2p_nvme_config.buffer_size_exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            memory.p2p_nvme_config_exists = memory.p2p_nvme_config.total_size_exists || memory.p2p_nvme_config.buffer_size_exists;

            // cu_bw
            node_title = {DEVICE, MEMORY, std::to_string(idx), CU_BW};
            // ONLY_WR
            if (ExtractRequiredThreshold(node_title, {ONLY_WR,WRITE}, CU_BW_AVG_TO_HI_LO_PERCENT, memory.cu_bw.only_wr.write) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ONLY_RD
            if (ExtractRequiredThreshold(node_title, {ONLY_RD,READ}, CU_BW_AVG_TO_HI_LO_PERCENT, memory.cu_bw.only_rd.read) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ALT_WR_RD
            memory.cu_bw.alt_wr_rd.write.high = ((double)(memory.cu_bw.only_wr.write.high) + (double)(memory.cu_bw.only_rd.read.high)) / (double)(4);
            memory.cu_bw.alt_wr_rd.write.low  = ((double)(memory.cu_bw.only_wr.write.low)  + (double)(memory.cu_bw.only_rd.read.low))  / (double)(4);
            memory.cu_bw.alt_wr_rd.read       = memory.cu_bw.alt_wr_rd.write;
            if (ExtractOptionalThreshold(node_title, {ALT_WR_RD,WRITE}, CU_BW_AVG_TO_HI_LO_PERCENT, memory.cu_bw.alt_wr_rd.write) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalThreshold(node_title, {ALT_WR_RD,READ},  CU_BW_AVG_TO_HI_LO_PERCENT, memory.cu_bw.alt_wr_rd.read) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // SIMUL_WR_RD
            memory.cu_bw.simul_wr_rd.write.high = ((double)(memory.cu_bw.only_wr.write.high) + (double)(memory.cu_bw.only_rd.read.high)) / (double)(4);
            memory.cu_bw.simul_wr_rd.write.low  = ((double)(memory.cu_bw.only_wr.write.low)  + (double)(memory.cu_bw.only_rd.read.low))  / (double)(4);
            memory.cu_bw.simul_wr_rd.read       = memory.cu_bw.simul_wr_rd.write;
            if (ExtractOptionalThreshold(node_title, {SIMUL_WR_RD,WRITE}, CU_BW_AVG_TO_HI_LO_PERCENT, memory.cu_bw.simul_wr_rd.write) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalThreshold(node_title, {SIMUL_WR_RD,READ},  CU_BW_AVG_TO_HI_LO_PERCENT, memory.cu_bw.simul_wr_rd.read) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            // cu_latency
            node_title = {DEVICE, MEMORY, std::to_string(idx), CU_LATENCY};
            // ONLY_WR
            if (ExtractRequiredThreshold(node_title, {ONLY_WR,WRITE}, CU_LAT_AVG_TO_HI_LO_PERCENT, memory.cu_latency.only_wr.write) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ONLY_RD
            if (ExtractRequiredThreshold(node_title, {ONLY_RD,READ},  CU_LAT_AVG_TO_HI_LO_PERCENT, memory.cu_latency.only_rd.read) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ALT_WR_RD
            memory.cu_latency.alt_wr_rd.write = memory.cu_latency.only_wr.write;
            memory.cu_latency.alt_wr_rd.read  = memory.cu_latency.only_rd.read;
            if (ExtractOptionalThreshold(node_title, {ALT_WR_RD,WRITE}, CU_LAT_AVG_TO_HI_LO_PERCENT, memory.cu_latency.alt_wr_rd.write) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalThreshold(node_title, {ALT_WR_RD,READ},  CU_LAT_AVG_TO_HI_LO_PERCENT, memory.cu_latency.alt_wr_rd.read) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // SIMUL_WR_RD
            memory.cu_latency.simul_wr_rd.write = memory.cu_latency.only_wr.write;
            memory.cu_latency.simul_wr_rd.read  = memory.cu_latency.only_rd.read;
            if (ExtractOptionalThreshold(node_title, {SIMUL_WR_RD,WRITE}, CU_LAT_AVG_TO_HI_LO_PERCENT, memory.cu_latency.simul_wr_rd.write) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalThreshold(node_title, {SIMUL_WR_RD,READ},  CU_LAT_AVG_TO_HI_LO_PERCENT, memory.cu_latency.simul_wr_rd.read) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            // cu_rate
            node_title = {DEVICE, MEMORY, std::to_string(idx), CU_RATE};
            // ONLY_WR
            memory.cu_rate.only_wr.write.nominal = DEFAULT_CU_RATE;
            if (ExtractOptionalUint(node_title, {ONLY_WR,WRITE,NOMINAL}, memory.cu_rate.only_wr.write.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ONLY_RD
            memory.cu_rate.only_rd.read.nominal = DEFAULT_CU_RATE;
            if (ExtractOptionalUint(node_title, {ONLY_RD,READ,NOMINAL}, memory.cu_rate.only_rd.read.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ALT_WR_RD
            memory.cu_rate.alt_wr_rd.write = memory.cu_rate.only_wr.write;
            memory.cu_rate.alt_wr_rd.read  = memory.cu_rate.only_rd.read;
            if (ExtractOptionalUint(node_title, {ALT_WR_RD,WRITE,NOMINAL}, memory.cu_rate.alt_wr_rd.write.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalUint(node_title, {ALT_WR_RD,READ,NOMINAL}, memory.cu_rate.alt_wr_rd.read.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // SIMUL_WR_RD
            memory.cu_rate.simul_wr_rd.write = memory.cu_rate.only_wr.write;
            memory.cu_rate.simul_wr_rd.read  = memory.cu_rate.only_rd.read;
            if (ExtractOptionalUint(node_title, {SIMUL_WR_RD,WRITE,NOMINAL}, memory.cu_rate.simul_wr_rd.write.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalUint(node_title, {SIMUL_WR_RD,READ,NOMINAL}, memory.cu_rate.simul_wr_rd.read.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            // cu_outstanding
            node_title = {DEVICE, MEMORY, std::to_string(idx), CU_OUTSTANDING};
            // ONLY_WR
            memory.cu_outstanding.only_wr.write.nominal = DEFAULT_CU_OUTSTANDING;
            if (ExtractOptionalUint(node_title, {ONLY_WR,WRITE,NOMINAL}, memory.cu_outstanding.only_wr.write.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ONLY_RD
            memory.cu_outstanding.only_rd.read.nominal = DEFAULT_CU_OUTSTANDING;
            if (ExtractOptionalUint(node_title, {ONLY_RD,READ,NOMINAL}, memory.cu_outstanding.only_rd.read.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ALT_WR_RD
            memory.cu_outstanding.alt_wr_rd.write = memory.cu_outstanding.only_wr.write;
            memory.cu_outstanding.alt_wr_rd.read  = memory.cu_outstanding.only_rd.read;
            if (ExtractOptionalUint(node_title, {ALT_WR_RD,WRITE,NOMINAL}, memory.cu_outstanding.alt_wr_rd.write.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalUint(node_title, {ALT_WR_RD,READ,NOMINAL}, memory.cu_outstanding.alt_wr_rd.read.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // SIMUL_WR_RD
            memory.cu_outstanding.simul_wr_rd.write = memory.cu_outstanding.only_wr.write;
            memory.cu_outstanding.simul_wr_rd.read  = memory.cu_outstanding.only_rd.read;
            if (ExtractOptionalUint(node_title, {SIMUL_WR_RD,WRITE,NOMINAL}, memory.cu_outstanding.simul_wr_rd.write.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (ExtractOptionalUint(node_title, {SIMUL_WR_RD,READ,NOMINAL}, memory.cu_outstanding.simul_wr_rd.read.nominal, found) == RET_FAILURE)
            {
                return RET_FAILURE;
            }

            // cu_burst_size
            node_title = {DEVICE, MEMORY, std::to_string(idx), CU_BURST_SIZE};
            // ONLY_WR
            if (ExtractOptionalUint(node_title, {ONLY_WR,WRITE,NOMINAL}, memory.cu_burst_size.only_wr.write.nominal, memory.cu_burst_size.only_wr.write.exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ONLY_RD
            if (ExtractOptionalUint(node_title, {ONLY_RD,READ,NOMINAL}, memory.cu_burst_size.only_rd.read.nominal, memory.cu_burst_size.only_rd.read.exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            // ALT_WR_RD
            if (ExtractOptionalUint(node_title, {ALT_WR_RD,WRITE,NOMINAL}, memory.cu_burst_size.alt_wr_rd.write.nominal, memory.cu_burst_size.alt_wr_rd.write.exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!memory.cu_burst_size.alt_wr_rd.write.exists)
            {
                memory.cu_burst_size.alt_wr_rd.write = memory.cu_burst_size.only_wr.write;
            }
            if (ExtractOptionalUint(node_title, {ALT_WR_RD,READ,NOMINAL}, memory.cu_burst_size.alt_wr_rd.read.nominal, memory.cu_burst_size.alt_wr_rd.read.exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!memory.cu_burst_size.alt_wr_rd.read.exists)
            {
                memory.cu_burst_size.alt_wr_rd.read = memory.cu_burst_size.only_rd.read;
            }
            // SIMUL_WR_RD
            if (ExtractOptionalUint(node_title, {SIMUL_WR_RD,WRITE,NOMINAL}, memory.cu_burst_size.simul_wr_rd.write.nominal, memory.cu_burst_size.simul_wr_rd.write.exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!memory.cu_burst_size.simul_wr_rd.write.exists)
            {
                memory.cu_burst_size.simul_wr_rd.write = memory.cu_burst_size.only_wr.write;
            }
            if (ExtractOptionalUint(node_title, {SIMUL_WR_RD,READ,NOMINAL}, memory.cu_burst_size.simul_wr_rd.read.nominal, memory.cu_burst_size.simul_wr_rd.read.exists) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            if (!memory.cu_burst_size.simul_wr_rd.read.exists)
            {
                memory.cu_burst_size.simul_wr_rd.read = memory.cu_burst_size.only_rd.read;
            }

            this->m_xbtest_pfm_def.memory.emplace_back(memory);
        }
    }
    PrintPlatformDef(this->m_xbtest_pfm_def);
    return RET_SUCCESS;
}

void XbtestPfmDefParser::PrintPlatformDef( const Xbtest_Pfm_Def_t & xbtest_pfm_def )
{
    LogMessage(MSG_DEBUG_SETUP, {"Download time (s): " + std::to_string(xbtest_pfm_def.runtime.download_time)});

    for (const auto & source : xbtest_pfm_def.sensor)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Sensor source " + source.id + " configuration:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t - ID     : " + source.id});
        LogMessage(MSG_DEBUG_SETUP, {"\t - Type   : " + source.type});
    }

    for (uint gt_index = 0; gt_index < xbtest_pfm_def.gt.settings.size(); gt_index++)
    {
        LogMessage(MSG_DEBUG_SETUP, {"GT [" + std::to_string(gt_index) + "] configuration:"});
        if (xbtest_pfm_def.gt.settings[gt_index].name != "none")
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t - GT Name             : " + xbtest_pfm_def.gt.settings[gt_index].name});
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t - Transceiver settings:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Cable:}"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX polarity                  : " +                xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.tx_polarity});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX main cursor               : " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.tx_main_cursor)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX differential swing control: " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.tx_differential_swing_control)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX pre emphasis              : " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.tx_pre_emphasis)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX post emphasis             : " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.tx_post_emphasis)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + RX equaliser                 : " +                xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.rx_equalizer});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + RX polarity                  : " +                xbtest_pfm_def.gt.settings[gt_index].transceivers.cable.rx_polarity});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Module: "});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX polarity                  : " +                xbtest_pfm_def.gt.settings[gt_index].transceivers.module.tx_polarity});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX tx main cursor            : " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.module.tx_main_cursor)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX differential swing control: " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.module.tx_differential_swing_control)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX pre emphasis              : " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.module.tx_pre_emphasis)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + TX post emphasis             : " + std::to_string(xbtest_pfm_def.gt.settings[gt_index].transceivers.module.tx_post_emphasis)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + RX equaliser                 : " +                xbtest_pfm_def.gt.settings[gt_index].transceivers.module.rx_equalizer});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + RX polarity                  : " +                xbtest_pfm_def.gt.settings[gt_index].transceivers.module.rx_polarity});
    }

    if (this->m_is_mmio_test_supported)
    {
        LogMessage(MSG_DEBUG_SETUP, {"MMIO test BW thresholds (kBps):"});
        LogMessage(MSG_DEBUG_SETUP, {"\t * Write: [" + std::to_string(xbtest_pfm_def.mmio.bw.write.low) + ";" + std::to_string(xbtest_pfm_def.mmio.bw.write.high)   + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t * Read : [" + std::to_string(xbtest_pfm_def.mmio.bw.read.low)  + ";" + std::to_string(xbtest_pfm_def.mmio.bw.read.high)    + "]"});

        if (xbtest_pfm_def.mmio.config_exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"MMIO test configuration:"});
            if (xbtest_pfm_def.mmio.config.total_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t - Total size (Bytes): " + std::to_string(xbtest_pfm_def.mmio.config.total_size)});
            }
            if (xbtest_pfm_def.mmio.config.buffer_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t - Buffer size (Bytes): " + std::to_string(xbtest_pfm_def.mmio.config.buffer_size)});
            }
            if (xbtest_pfm_def.mmio.config.bar_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t - BAR: " + std::to_string(xbtest_pfm_def.mmio.config.bar)});
            }
            if (xbtest_pfm_def.mmio.config.offset_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t - Offset: " + std::to_string(xbtest_pfm_def.mmio.config.offset)});
            }
        }
    }

    for (const auto & memory : xbtest_pfm_def.memory)
    {
        LogMessage(MSG_DEBUG_SETUP, {"Memory " + memory.name + " configuration:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t - BW thresholds (MBps) for each tag in DMA test:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Write: [" + std::to_string(memory.dma_bw.write.low) + ";" + std::to_string(memory.dma_bw.write.high)   + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Read : [" + std::to_string(memory.dma_bw.read.low)  + ";" + std::to_string(memory.dma_bw.read.high)    + "]"});

        if (memory.dma_config_exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t - Configuration for each tag in DMA test:"});
            if (memory.dma_config.total_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t * Total size (MB): " + std::to_string(memory.dma_config.total_size)});
            }
            if (memory.dma_config.buffer_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t * Buffer size (MB): " + std::to_string(memory.dma_config.buffer_size)});
            }
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t - BW thresholds (MBps) for each tag in P2P_CARD test:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Write: [" + std::to_string(memory.p2p_card_bw.write.low) + ";" + std::to_string(memory.p2p_card_bw.write.high)   + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Read : [" + std::to_string(memory.p2p_card_bw.read.low)  + ";" + std::to_string(memory.p2p_card_bw.read.high)    + "]"});

        if (memory.p2p_card_config_exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t - Configuration for each tag in P2P_CARD test:"});
            if (memory.p2p_card_config.total_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t * Total size (MB): " + std::to_string(memory.p2p_card_config.total_size)});
            }
            if (memory.p2p_card_config.buffer_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t * Buffer size (MB): " + std::to_string(memory.p2p_card_config.buffer_size)});
            }
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t - BW thresholds (MBps) for each tag in P2P_NVME test:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Write: [" + std::to_string(memory.p2p_nvme_bw.write.low) + ";" + std::to_string(memory.p2p_nvme_bw.write.high)   + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Read : [" + std::to_string(memory.p2p_nvme_bw.read.low)  + ";" + std::to_string(memory.p2p_nvme_bw.read.high)    + "]"});

        if (memory.p2p_nvme_config_exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t - Configuration for each tag in P2P_NVME test:"});
            if (memory.p2p_nvme_config.total_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t * Total size (MB): " + std::to_string(memory.p2p_nvme_config.total_size)});
            }
            if (memory.p2p_nvme_config.buffer_size_exists)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t * Buffer size (MB): " + std::to_string(memory.p2p_nvme_config.buffer_size)});
            }
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t - xbtest HW IP BW thresholds (MBps):"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: [" + std::to_string(memory.cu_bw.only_wr.write.low)     + ";" + std::to_string(memory.cu_bw.only_wr.write.high)      + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : [" + std::to_string(memory.cu_bw.only_rd.read.low)      + ";" + std::to_string(memory.cu_bw.only_rd.read.high)       + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: [" + std::to_string(memory.cu_bw.alt_wr_rd.write.low)   + ";" + std::to_string(memory.cu_bw.alt_wr_rd.write.high)    + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : [" + std::to_string(memory.cu_bw.alt_wr_rd.read.low)    + ";" + std::to_string(memory.cu_bw.alt_wr_rd.read.high)     + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: [" + std::to_string(memory.cu_bw.simul_wr_rd.write.low) + ";" + std::to_string(memory.cu_bw.simul_wr_rd.write.high)  + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : [" + std::to_string(memory.cu_bw.simul_wr_rd.read.low)  + ";" + std::to_string(memory.cu_bw.simul_wr_rd.read.high)   + "]"});

        LogMessage(MSG_DEBUG_SETUP, {"\t - xbtest HW IP latency thresholds (ns):"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: [" + std::to_string(memory.cu_latency.only_wr.write.low)     + ";" + std::to_string(memory.cu_latency.only_wr.write.high)      + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : [" + std::to_string(memory.cu_latency.only_rd.read.low)      + ";" + std::to_string(memory.cu_latency.only_rd.read.high)       + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: [" + std::to_string(memory.cu_latency.alt_wr_rd.write.low)   + ";" + std::to_string(memory.cu_latency.alt_wr_rd.write.high)    + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : [" + std::to_string(memory.cu_latency.alt_wr_rd.read.low)    + ";" + std::to_string(memory.cu_latency.alt_wr_rd.read.high)     + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: [" + std::to_string(memory.cu_latency.simul_wr_rd.write.low) + ";" + std::to_string(memory.cu_latency.simul_wr_rd.write.high)  + "]"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : [" + std::to_string(memory.cu_latency.simul_wr_rd.read.low)  + ";" + std::to_string(memory.cu_latency.simul_wr_rd.read.high)   + "]"});

        LogMessage(MSG_DEBUG_SETUP, {"\t - xbtest HW IP rate (%):"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: " + std::to_string(memory.cu_rate.only_wr.write.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : " + std::to_string(memory.cu_rate.only_rd.read.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: " + std::to_string(memory.cu_rate.alt_wr_rd.write.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : " + std::to_string(memory.cu_rate.alt_wr_rd.read.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: " + std::to_string(memory.cu_rate.simul_wr_rd.write.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : " + std::to_string(memory.cu_rate.simul_wr_rd.read.nominal)});

        LogMessage(MSG_DEBUG_SETUP, {"\t - xbtest HW IP maximum number of outstanding:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_WR + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Writes: " + std::to_string(memory.cu_outstanding.only_wr.write.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Reads : " + std::to_string(memory.cu_outstanding.only_rd.read.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Writes: " + std::to_string(memory.cu_outstanding.alt_wr_rd.write.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Reads : " + std::to_string(memory.cu_outstanding.alt_wr_rd.read.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Writes: " + std::to_string(memory.cu_outstanding.simul_wr_rd.write.nominal)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Reads : " + std::to_string(memory.cu_outstanding.simul_wr_rd.read.nominal)});

        LogMessage(MSG_DEBUG_SETUP, {"\t - xbtest HW IP burst size (Bytes):"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_WR + " mode:"});
        if (memory.cu_burst_size.only_wr.write.exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: " + std::to_string(memory.cu_burst_size.only_wr.write.nominal)});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: see memory TC"});
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ONLY_RD + " mode:"});
        if (memory.cu_burst_size.only_rd.read.exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : " + std::to_string(memory.cu_burst_size.only_rd.read.nominal)});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : see memory TC"});
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + ALTERNATE_WR_RD + " mode:"});
        if (memory.cu_burst_size.alt_wr_rd.write.exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: " + std::to_string(memory.cu_burst_size.alt_wr_rd.write.nominal)});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: see memory TC"});
        }
        if (memory.cu_burst_size.alt_wr_rd.read.exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : " + std::to_string(memory.cu_burst_size.alt_wr_rd.read.nominal)});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : see memory TC"});
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * " + SIMULTANEOUS_WR_RD + " mode:"});
        if (memory.cu_burst_size.simul_wr_rd.write.exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: " + std::to_string(memory.cu_burst_size.simul_wr_rd.write.nominal)});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Write: see memory TC"});
        }
        if (memory.cu_burst_size.simul_wr_rd.read.exists)
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : " + std::to_string(memory.cu_burst_size.simul_wr_rd.read.nominal)});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Read : see memory TC"});
        }
    }
}

Json_Definition_t XbtestPfmDefParser::GetJsonDefinition()
{
    Json_Definition_t json_definition;
    json_definition.insert( Definition_t({DEVICE},  JSON_NODE_OBJECT));

    json_definition.insert( Definition_t({DEVICE, RUNTIME},                 JSON_NODE_OBJECT));
    json_definition.insert( Definition_t({DEVICE, RUNTIME, DOWNLOAD_TIME},  JSON_NODE_VALUE));

    json_definition.insert( Definition_t({DEVICE, SENSOR}, JSON_NODE_OBJECT));
    for (uint idx = 0; idx < MAX_SENSORS; idx++)
    {
        json_definition.insert( Definition_t({DEVICE, SENSOR, std::to_string(idx)},                     JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, SENSOR, std::to_string(idx), ID},                 JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, SENSOR, std::to_string(idx), TYPE},               JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, SENSOR, std::to_string(idx), CARD_POWER},         JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, SENSOR, std::to_string(idx), CARD_TEMPERATURE},   JSON_NODE_VALUE));
    }

    json_definition.insert( Definition_t({DEVICE, GT}, JSON_NODE_OBJECT));
    for (uint idx = 0; idx < MAX_GT_QTY; idx++)
    {
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx)},         JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), NAME},   JSON_NODE_VALUE));

        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS},   JSON_NODE_OBJECT));

        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE},                  JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,TX_POLARITY},      JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,TX_MAIN_CURSOR},   JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,TX_DIFF_CTRL},     JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,TX_PRE_EMPH },     JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,TX_POST_EMPH},     JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,RX_EQUALISER},     JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_MODULE,RX_POLARITY},      JSON_NODE_VALUE));

        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE},                   JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,TX_POLARITY},       JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,TX_MAIN_CURSOR},    JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,TX_DIFF_CTRL},      JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,TX_PRE_EMPH },      JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,TX_POST_EMPH},      JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,RX_EQUALISER},      JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, GT, std::to_string(idx), TRANSCEIVER_SETTINGS, GT_TRANS_SET_CABLE,RX_POLARITY},       JSON_NODE_VALUE));

    }

    if (this->m_is_mmio_test_supported)
    {
        json_definition.insert( Definition_t({DEVICE, MMIO}, JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, MMIO, BW}, JSON_NODE_OBJECT));
        for (const auto & direction : {WRITE, READ})
        {
            json_definition.insert( Definition_t({DEVICE, MMIO, BW, direction},          JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MMIO, BW, direction, AVERAGE}, JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MMIO, BW, direction, HIGH},    JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MMIO, BW, direction, LOW},     JSON_NODE_VALUE));
        }

        json_definition.insert( Definition_t({DEVICE, MMIO, CONFIG},                 JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, MMIO, CONFIG, TOTAL_SIZE},     JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, MMIO, CONFIG, BUFFERSIZE},     JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, MMIO, CONFIG, BAR},            JSON_NODE_VALUE));
        json_definition.insert( Definition_t({DEVICE, MMIO, CONFIG, OFFSET},         JSON_NODE_VALUE));
    }

    json_definition.insert( Definition_t({DEVICE, MEMORY}, JSON_NODE_OBJECT));
    for (uint idx = 0; idx < MAX_MEM_TYPE_QTY; idx++)
    {
        json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx)},         JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), NAME},   JSON_NODE_VALUE));

        if (!(this->m_is_nodma))
        {
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_BW}, JSON_NODE_OBJECT));
            for (const auto & direction : {WRITE, READ})
            {
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_BW, direction},          JSON_NODE_OBJECT));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_BW, direction, AVERAGE}, JSON_NODE_VALUE));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_BW, direction, HIGH},    JSON_NODE_VALUE));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_BW, direction, LOW},     JSON_NODE_VALUE));
            }

            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_CONFIG},                 JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_CONFIG, TOTAL_SIZE},     JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), DMA_CONFIG, BUFFERSIZE},     JSON_NODE_VALUE));
        }
        if (this->m_is_p2p_supported)
        {
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_BW}, JSON_NODE_OBJECT));
            for (const auto & direction : {WRITE, READ})
            {
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_BW, direction},          JSON_NODE_OBJECT));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_BW, direction, AVERAGE}, JSON_NODE_VALUE));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_BW, direction, HIGH},    JSON_NODE_VALUE));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_BW, direction, LOW},     JSON_NODE_VALUE));
            }

            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_CONFIG},                 JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_CONFIG, TOTAL_SIZE},     JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_CARD_CONFIG, BUFFERSIZE},     JSON_NODE_VALUE));

            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_BW}, JSON_NODE_OBJECT));
            for (const auto & direction : {WRITE, READ})
            {
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_BW, direction},          JSON_NODE_OBJECT));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_BW, direction, AVERAGE}, JSON_NODE_VALUE));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_BW, direction, HIGH},    JSON_NODE_VALUE));
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_BW, direction, LOW},     JSON_NODE_VALUE));
            }

            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_CONFIG},                 JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_CONFIG, TOTAL_SIZE},     JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), P2P_NVME_CONFIG, BUFFERSIZE},     JSON_NODE_VALUE));
        }
        for (const auto & config : {CU_BW, CU_LATENCY})
        {
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config}, JSON_NODE_OBJECT));
            for (const auto & mode : {ALT_WR_RD, SIMUL_WR_RD})
            {
                json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode}, JSON_NODE_OBJECT));
                for (const auto & direction : {WRITE, READ})
                {
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode, direction},            JSON_NODE_OBJECT));
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode, direction, AVERAGE},   JSON_NODE_VALUE));
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode, direction, HIGH},      JSON_NODE_VALUE));
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode, direction, LOW},       JSON_NODE_VALUE));
                }
            }
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR},                    JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR, WRITE},             JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR, WRITE, AVERAGE},    JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR, WRITE, HIGH},       JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR, WRITE, LOW},        JSON_NODE_VALUE));

            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD},                JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD, READ},          JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD, READ, AVERAGE}, JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD, READ, HIGH},    JSON_NODE_VALUE));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD, READ, LOW},     JSON_NODE_VALUE));
        }

        for (const auto & config : {CU_RATE, CU_OUTSTANDING, CU_BURST_SIZE})
        {
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config}, JSON_NODE_OBJECT));
            for (const auto & mode : {ALT_WR_RD, SIMUL_WR_RD})
            {
                for (const auto & direction : {WRITE, READ})
                {
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode},                       JSON_NODE_OBJECT));
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode, direction},            JSON_NODE_OBJECT));
                    json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, mode, direction, NOMINAL},   JSON_NODE_VALUE));
                }
            }
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR},                    JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR, WRITE},             JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_WR, WRITE, NOMINAL},    JSON_NODE_VALUE));

            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD},                    JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD, READ},              JSON_NODE_OBJECT));
            json_definition.insert( Definition_t({DEVICE, MEMORY, std::to_string(idx), config, ONLY_RD, READ, NOMINAL},     JSON_NODE_VALUE));
        }
    }
    return json_definition;
}

Xbtest_Pfm_Def_t XbtestPfmDefParser::GetPlatformDef()
{
    return this->m_xbtest_pfm_def;
}

void XbtestPfmDefParser::PrintRequiredNotFound( const std::vector<std::string> & node_title )
{
    LogMessage(MSG_ITF_044, {this->m_content_name, StrVectToStr(node_title, ".")});
}

void XbtestPfmDefParser::PrintOptionalNotFound( const std::vector<std::string> & node_title )
{
    LogMessage(MSG_ITF_125, {this->m_content_name, StrVectToStr(node_title, ".")});
}

bool XbtestPfmDefParser::ExtractRequiredString( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, std::string & value )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    if (ExtractNodeValueStr(node_title, value) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + value});
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractOptionalString( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, std::string & value, bool & found )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    found = false;
    if (NodeExists(node_title))
    {
        if (ExtractNodeValueStr(node_title, value) == RET_FAILURE)
        {
            PrintOptionalNotFound(node_title);
            return RET_FAILURE;
        }
        found = true;
    }
    if (found)
    {
        LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + value});
    }
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractOptionalBool( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, bool & value, bool & found )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    found = false;
    if (NodeExists(node_title))
    {
        if (ExtractNodeValueBool(node_title, value) == RET_FAILURE)
        {
            PrintOptionalNotFound(node_title);
            return RET_FAILURE;
        }
        found = true;
    }
    if (found)
    {
        LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + BoolToStr(value)});
    }
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractRequiredUint( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint & value )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    if (ExtractNodeValueInt<uint>(node_title, value) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + std::to_string(value)});
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractOptionalUint8( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint8_t & value, bool & found )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    found = false;
    if (NodeExists(node_title))
    {
        if (ExtractNodeValueInt<uint8_t>(node_title, value) == RET_FAILURE)
        {
            PrintOptionalNotFound(node_title);
            return RET_FAILURE;
        }
        found = true;
        LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + std::to_string(value)});
    }
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractOptionalUint( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint & value, bool & found )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    found = false;
    if (NodeExists(node_title))
    {
        if (ExtractNodeValueInt<uint>(node_title, value) == RET_FAILURE)
        {
            PrintOptionalNotFound(node_title);
            return RET_FAILURE;
        }
        found = true;
        LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + std::to_string(value)});
    }
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractOptionalUint64( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint64_t & value, bool & found )
{
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    found = false;
    if (NodeExists(node_title))
    {
        if (ExtractNodeValueInt<uint64_t>(node_title, value) == RET_FAILURE)
        {
            PrintOptionalNotFound(node_title);
            return RET_FAILURE;
        }
        found = true;
        LogMessage(MSG_DEBUG_PARSING, {"Extracted " + StrVectToStr(node_title, ".") + " = " + std::to_string(value)});
    }
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractRequiredThreshold( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, const double & percent, Mem_Thresh_t & thresh )
{
    auto found = false;
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    if (ExtractOptionalUint(node_title, {AVERAGE}, thresh.average, found) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (found)
    {
        thresh.high = std::ceil((double)(thresh.average) + percent * (double)(thresh.average) / (double)(100));
        thresh.low  = std::ceil((double)(thresh.average) - percent * (double)(thresh.average) / (double)(100));
        // HIGH
        if (ExtractOptionalUint(node_title, {HIGH}, thresh.high, found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // LOW
        if (ExtractOptionalUint(node_title, {LOW}, thresh.low, found) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    else
    {
        // HIGH
        if (ExtractRequiredUint(node_title, {HIGH}, thresh.high) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        // LOW
        if (ExtractRequiredUint(node_title, {LOW},  thresh.low) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

bool XbtestPfmDefParser::ExtractOptionalThreshold( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, const double & percent, Mem_Thresh_t & thresh )
{
    auto found      = false;
    auto node_title = base_title;
    node_title.insert(node_title.end(), rel_node_title.begin(), rel_node_title.end());
    if (ExtractOptionalUint(node_title, {AVERAGE}, thresh.average, found) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (found)
    {
        thresh.high = std::ceil((double)(thresh.average) + percent * (double)(thresh.average) / (double)(100));
        thresh.low  = std::ceil((double)(thresh.average) - percent * (double)(thresh.average) / (double)(100));
    }
    if (ExtractOptionalUint(node_title, {HIGH}, thresh.high, found) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ExtractOptionalUint(node_title, {LOW}, thresh.low, found) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

} // namespace
