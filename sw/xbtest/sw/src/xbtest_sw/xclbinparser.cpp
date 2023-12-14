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

#include <fstream>

#include "xclbinparser.h"

namespace xbtest
{

XclbinParser::XclbinParser(
    Logging * log, const std::string & xclbin, const std::string & design_pdi, std::atomic<bool> * abort
) : XJsonParser::XJsonParser(log, abort)
{
    this->m_log_header    = LOG_HEADER_XCLBIN_PARSER;
    this->m_xclbin        = xclbin;
    this->m_design_pdi    = design_pdi;

    this->m_content_name  = XBTEST_METADATA;

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    auto design_pdi_path_split = split(this->m_design_pdi, '/');
    design_pdi_path_split.pop_back();
    design_pdi_path_split.emplace_back("xbtest");
    design_pdi_path_split.emplace_back("metadata");
    this->m_metadata_dir = StrVectToStr(design_pdi_path_split, "/");
#endif
#ifdef USE_XRT
    std::ifstream is(this->m_xclbin, std::ifstream::binary);
    is.seekg (0, is.end);
    auto length = is.tellg();
    is.seekg (0, is.beg);
    this->m_buffer = new char [length];
    is.read(this->m_buffer, length);
    this->m_xclbin_axlf = reinterpret_cast<axlf*>(this->m_buffer);
#endif
}

XclbinParser::~XclbinParser()
{
    delete [] this->m_buffer;
    ClearParser();
}

Xclbin_Metadata_t XclbinParser::GetXclbinMetadata()
{
    return this->m_xclbin_metadata;
}

bool XclbinParser::Parse()
{
    // Initialize json parser and reader
    this->m_json_parser = json_parser_new();
    this->m_json_reader = json_reader_new(nullptr);

#ifdef USE_XRT
    if (ParseIpLayout() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {IP_LAYOUT});
        return RET_FAILURE;
    }
    if (ParseMemoryTopology() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {MEM_TOPOLOGY});
        return RET_FAILURE;
    }
    if (ParseConnectivity() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {CONNECTIVITY});
        return RET_FAILURE;
    }
    if (ParseClockFreqTopology() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {CLOCK_FREQ_TOPOLOGY});
        return RET_FAILURE;
    }
    if (ParseUserMetadata() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {USER_METADATA});
        return RET_FAILURE;
    }
#endif
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    if (ParsePartitionMetadataJson() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {IP_LAYOUT});
        return RET_FAILURE;
    }
    if (ParseMemoryTopologyJson() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {MEM_TOPOLOGY});
        return RET_FAILURE;
    }
    if (ParseConnectivityJson() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {CONNECTIVITY});
        return RET_FAILURE;
    }
    if (ParseClockFreqTopologyJson() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {CLOCK_FREQ_TOPOLOGY});
        return RET_FAILURE;
    }
    if (ParseUserMetadataJson() == RET_FAILURE)
    {
        LogMessage(MSG_ITF_032, {USER_METADATA});
        return RET_FAILURE;
    }
#endif
    return RET_SUCCESS;
}

#ifdef USE_XRT
bool XclbinParser::ParseIpLayout()
{
    this->m_section = IP_LAYOUT;
    LogMessage(MSG_ITF_039, {this->m_section});
    auto section_header = xclbin::get_axlf_section(this->m_xclbin_axlf, axlf_section_kind::IP_LAYOUT);
    char data [section_header->m_sectionSize];
    memcpy(data, this->m_buffer + section_header->m_sectionOffset, section_header->m_sectionSize);
    auto ipl = reinterpret_cast<ip_layout*>(data);

    for (int32_t i = 0; i < ipl->m_count; i++)
    {
        if (ipl->m_ip_data[i].m_type != IP_TYPE::IP_KERNEL)
        {
            continue;
        }

        Xclbin_Ip_Data_t ip_data;
        ip_data.m_name               = std::string((char*)ipl->m_ip_data[i].m_name);
        ip_data.m_ip_layout_index    = i;
        this->m_xclbin_metadata.ip_layout.m_ip_data.emplace_back(ip_data);
    }
    PrintIpLayout();
    return RET_SUCCESS;
}
#endif

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
bool XclbinParser::ParsePartitionMetadataJson()
{
    this->m_section = PARTITION_METADATA;
    this->m_filename = this->m_metadata_dir + "/xbtest_metadata.json";
    LogMessage(MSG_ITF_039, {this->m_section});
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {this->m_section, this->m_filename});
        return RET_FAILURE;
    }

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_section, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        return RET_FAILURE;
    }
    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    std::string tmp_str;

    // partition_metadata
    std::vector<std::string> node_title = {PARTITION_METADATA_L};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // bars
    std::vector<std::string> bars_title = {PARTITION_METADATA_L, PCIE, BARS};
    if (!NodeExists(bars_title))
    {
        PrintRequiredNotFound(bars_title);
        return RET_FAILURE;
    }
    uint num_node_read = 0;
    ExtractNode(bars_title, num_node_read); // Move cursor to BARS array

    m_xclbin_metadata.partition_metadata.pcie_bars.clear();
    for (uint j = 0; j < (uint)json_reader_count_elements(m_json_reader); j++) // For each element in BARS array
    {
        json_reader_read_element(m_json_reader, j); // Move cursor to BARS element

        PCIe_Bar_Ep_t pcie_bar;

        // pcie_base_address_register
        node_title = {PCIE_BASE_ADDRESS_REGISTER};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint>(tmp_str, pcie_bar.pcie_base_address_register) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        // pcie_physical_function
        node_title = {PCIE_PHYSICAL_FUNCTION};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint>(tmp_str, pcie_bar.pcie_physical_function) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        // offset
        node_title = {OFFSET2};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint64_t>(tmp_str, pcie_bar.offset) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        // range
        node_title = {RANGE};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint64_t>(tmp_str, pcie_bar.range) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }

        pcie_bar.name = "PF " + std::to_string(pcie_bar.pcie_physical_function) + " BAR" + std::to_string(pcie_bar.pcie_base_address_register);

        m_xclbin_metadata.partition_metadata.pcie_bars.emplace_back(pcie_bar);

        json_reader_end_element(m_json_reader); // Move back cursor to BARS array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from BARS array
    {
        json_reader_end_element(m_json_reader);
    }

    // addressable_endpoints
    node_title = {PARTITION_METADATA_L, ADDRESSABLE_ENDPOINTS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    std::vector<std::string> addressable_endpoints_names;

    if (ListNodeMembers(node_title, addressable_endpoints_names) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    m_xclbin_metadata.partition_metadata.pcie_eps.clear();
    for (const auto & addressable_endpoints_name : addressable_endpoints_names)
    {
        PCIe_Bar_Ep_t pcie_ep;
        pcie_ep.name = addressable_endpoints_name;

        node_title.emplace_back(addressable_endpoints_name);

        // pcie_base_address_register
        node_title.emplace_back(PCIE_BASE_ADDRESS_REGISTER);
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint>(tmp_str, pcie_ep.pcie_base_address_register) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        node_title.pop_back(); // remove pcie_base_address_register

        // pcie_physical_function
        node_title.emplace_back(PCIE_PHYSICAL_FUNCTION);
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint>(tmp_str, pcie_ep.pcie_physical_function) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        node_title.pop_back(); // remove pcie_physical_function

        // offset
        node_title.emplace_back(OFFSET2);
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint64_t>(tmp_str, pcie_ep.offset) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        node_title.pop_back(); // remove offset

        // range
        node_title.emplace_back(RANGE);
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint64_t>(tmp_str, pcie_ep.range) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        node_title.pop_back(); // remove range

        node_title.pop_back(); // remove addressable_endpoints_name

        m_xclbin_metadata.partition_metadata.pcie_eps.emplace_back(pcie_ep);
    }

    PrintPartitionMetadata();
    return RET_SUCCESS;
}
#endif

#ifdef USE_XRT
bool XclbinParser::ParseMemoryTopology()
{
    this->m_section = MEM_TOPOLOGY;
    LogMessage(MSG_ITF_039, {this->m_section});
    auto section_header = xclbin::get_axlf_section(this->m_xclbin_axlf, axlf_section_kind::MEM_TOPOLOGY);
    char data[section_header->m_sectionSize];
    memcpy(data, this->m_buffer + section_header->m_sectionOffset, section_header->m_sectionSize);
    auto mem_tpl = reinterpret_cast<mem_topology*>(data);

    for (int32_t i = 0; i < mem_tpl->m_count; i++)
    {
        Xclbin_Memory_Data_t memory_data;
        memory_data.m_used       = mem_tpl->m_mem_data[i].m_used;
        memory_data.m_tag        = std::string((char*)mem_tpl->m_mem_data[i].m_tag);
        memory_data.mem_data_idx = i;
        this->m_xclbin_metadata.memory_topology.emplace_back(memory_data);
    }
    PrintMemoryTopology();
    return RET_SUCCESS;
}
#endif

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
bool XclbinParser::ParseMemoryTopologyJson()
{
    this->m_section = MEM_TOPOLOGY;
    this->m_filename = this->m_metadata_dir + "/xbtest_metadata.json";
    LogMessage(MSG_ITF_039, {this->m_section});
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {this->m_section, this->m_filename});
        return RET_FAILURE;
    }

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_section, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        return RET_FAILURE;
    }
    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    std::vector<std::string> node_title = {MEM_TOPOLOGY_L};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    std::vector<std::string> memory_topology_title = {MEM_TOPOLOGY_L, M_MEM_DATA};
    if (!NodeExists(memory_topology_title))
    {
        PrintRequiredNotFound(memory_topology_title);
        return RET_FAILURE;
    }
    uint num_node_read = 0;
    ExtractNode(memory_topology_title, num_node_read); // Move cursor to testcases array

    m_xclbin_metadata.memory_topology.clear();
    for (uint j = 0; j < (uint)json_reader_count_elements(m_json_reader); j++) // For each element in memory_data array
    {
        json_reader_read_element(m_json_reader, j); // Move cursor to memory_data element

        std::string tmp_str;
        Xclbin_Memory_Data_t memory_data;

        // m_used
        node_title = {M_USED};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (ConvString2Num<uint8_t>(tmp_str, memory_data.m_used) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        uint16_t tmp_uint16;
        if (ConvString2Num<uint16_t>(tmp_str, tmp_uint16) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        memory_data.m_used = tmp_uint16;

        // m_sizeKB
        node_title = {M_SIZEKB};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // size_bytes
        if (StrHexToNum<uint64_t>(tmp_str, memory_data.size_bytes) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        memory_data.size_bytes *= 1024; // kB to Bytes

        // m_tag
        node_title = {M_TAG};
        if (ExtractNodeValueStr(node_title, memory_data.m_tag) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // m_ip_name
        //node_title = {M_IP_NAME};
        //if (ExtractNodeValueStr(node_title, memory_data.m_ip_name) == RET_FAILURE)
        //{
        //    PrintRequiredNotFound(node_title);
        //    return RET_FAILURE;
        //}
        // m_base_address
        node_title = {M_BASE_ADDRESS};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (StrHexToNum<uint64_t>(tmp_str, memory_data.m_base_address) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }

        memory_data.mem_data_idx=j;

        // Add parsed connection to mem_topology vector
        m_xclbin_metadata.memory_topology.emplace_back(memory_data);

        json_reader_end_element(m_json_reader); // Move back cursor to connection array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from connection array
    {
        json_reader_end_element(m_json_reader);
    }

    PrintMemoryTopology();
    return RET_SUCCESS;
}
#endif

#ifdef USE_XRT
bool XclbinParser::ParseConnectivity()
{
    this->m_section = CONNECTIVITY;
    LogMessage(MSG_ITF_039, {this->m_section});
    auto section_header = xclbin::get_axlf_section(this->m_xclbin_axlf, axlf_section_kind::CONNECTIVITY);
    char data [section_header->m_sectionSize];
    memcpy(data, this->m_buffer + section_header->m_sectionOffset, section_header->m_sectionSize);
    auto con = reinterpret_cast<connectivity*>(data);

    for (int32_t i = 0; i < con->m_count; i++)
    {
        Xclbin_Connection_t xclbin_connection;
        xclbin_connection.arg_index         = con->m_connection[i].arg_index;
        xclbin_connection.m_ip_layout_index = con->m_connection[i].m_ip_layout_index;
        xclbin_connection.mem_data_index    = con->m_connection[i].mem_data_index;
        this->m_xclbin_metadata.connectivity.m_connection.emplace_back(xclbin_connection);
    }
    PrintConnectivity();
    return RET_SUCCESS;
}
#endif

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
bool XclbinParser::ParseConnectivityJson()
{
    this->m_section = CONNECTIVITY;
    this->m_filename = this->m_metadata_dir + "/xbtest_metadata.json";
    LogMessage(MSG_ITF_039, {this->m_section});
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {this->m_section, this->m_filename});
        return RET_FAILURE;
    }

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_section, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        return RET_FAILURE;
    }
    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    std::string tmp_str;
    std::vector<std::string> ip_names;

    std::vector<std::string> node_title = {CONNECTIVITY_L};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    std::vector<std::string> connection_title = {CONNECTIVITY_L, M_CONNECTION};
    if (!NodeExists(connection_title))
    {
        PrintRequiredNotFound(connection_title);
        return RET_FAILURE;
    }
    uint num_node_read = 0;
    ExtractNode(connection_title, num_node_read); // Move cursor to testcases array

    m_xclbin_metadata.ip_layout.m_ip_data.clear();
    m_xclbin_metadata.connectivity.m_connection.clear();
    for (uint j = 0; j <(uint)json_reader_count_elements(m_json_reader); j++) // For each element in connection array
    {
        json_reader_read_element(m_json_reader, j); // Move cursor to connection element

        Xclbin_Ip_Data_t ip_data;

        // ip_name
        node_title = {IP_NAME};
        if (ExtractNodeValueStr(node_title, ip_data.m_name) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        ip_data.m_ip_layout_index = 0;
        for (const auto & ip_name : ip_names)
        {
            if (StrMatchNoCase(ip_data.m_name, ip_name))
            {
                break;
            }
            ip_data.m_ip_layout_index++;
        }

        if (ip_data.m_ip_layout_index == ip_names.size())
        {
            ip_names.emplace_back(ip_data.m_name);
            m_xclbin_metadata.ip_layout.m_ip_data.emplace_back(ip_data);
        }

        Xclbin_Connection_t connection;
        connection.m_ip_layout_index = ip_data.m_ip_layout_index;
        // arg_index
        node_title = {ARG_INDEX};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (ConvString2Num<int32_t>(tmp_str, connection.arg_index) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        // mem_data_index
        node_title = {MEM_DATA_INDEX};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (ConvString2Num<int32_t>(tmp_str, connection.mem_data_index) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }

        // Add parsed connection to connectivity vector
        m_xclbin_metadata.connectivity.m_connection.emplace_back(connection);

        json_reader_end_element(m_json_reader); // Move back cursor to connection array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from connection array
    {
        json_reader_end_element(m_json_reader);
    }

    PrintIpLayout();
    PrintConnectivity();
    return RET_SUCCESS;
}
#endif

#ifdef USE_XRT
bool XclbinParser::ParseClockFreqTopology()
{
    this->m_section = CLOCK_FREQ_TOPOLOGY;
    LogMessage(MSG_ITF_039, {this->m_section});
    auto section_header = xclbin::get_axlf_section(this->m_xclbin_axlf, axlf_section_kind::CLOCK_FREQ_TOPOLOGY);
    char data [section_header->m_sectionSize];
    memcpy(data, this->m_buffer + section_header->m_sectionOffset, section_header->m_sectionSize);
    auto cft = reinterpret_cast<clock_freq_topology*>(data);

    // Find data clock (clock0)
    Xclbin_Clock_Freq_t xclbin_clock_freq_data;
    xclbin_clock_freq_data.found = false;
    for (int32_t i = 0; i < cft->m_count; i++)
    {
        if (cft->m_clock_freq[i].m_type == CLOCK_TYPE::CT_DATA)
        {
            xclbin_clock_freq_data.freq_Mhz  = cft->m_clock_freq[i].m_freq_Mhz;
            xclbin_clock_freq_data.found     = true;
            break;
        }
    }
    if (!xclbin_clock_freq_data.found)
    {
        LogMessage(MSG_ITF_151, {"DATA (clock 0)"});
    }
    this->m_xclbin_metadata.clock_freq_topology.m_clock_freq.emplace_back(xclbin_clock_freq_data);

    // Find kernel clock (clock1)
    Xclbin_Clock_Freq_t xclbin_clock_freq_kernel;
    xclbin_clock_freq_kernel.found = false;
    for (int32_t i = 0; i < cft->m_count; i++)
    {
        if (cft->m_clock_freq[i].m_type == CLOCK_TYPE::CT_KERNEL)
        {
            xclbin_clock_freq_kernel.freq_Mhz  = cft->m_clock_freq[i].m_freq_Mhz;
            xclbin_clock_freq_kernel.found     = true;
            break;
        }
    }
    if (!xclbin_clock_freq_kernel.found)
    {
        LogMessage(MSG_ITF_151, {"KERNEL (clock 1)"});
    }
    this->m_xclbin_metadata.clock_freq_topology.m_clock_freq.emplace_back(xclbin_clock_freq_kernel);

    PrintClockFreqTopology();
    return RET_SUCCESS;
}
#endif

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
bool XclbinParser::ParseClockFreqTopologyJson()
{
    this->m_section = CLOCK_FREQ_TOPOLOGY;
    LogMessage(MSG_ITF_039, {this->m_section});
    this->m_filename = this->m_metadata_dir + "/xbtest_metadata.json";
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {this->m_section, this->m_filename});
        return RET_FAILURE;
    }

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_section, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        return RET_FAILURE;
    }

    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;


    Xclbin_Clock_Freq_t xclbin_clock_freq_data;
    Xclbin_Clock_Freq_t xclbin_clock_freq_kernel;

    xclbin_clock_freq_data.found = false;
    xclbin_clock_freq_kernel.found = false;

    std::vector<std::string> node_title = {CLOCK_FREQ_TOPOLOGY_L};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    std::vector<std::string> clock_freq_title = {CLOCK_FREQ_TOPOLOGY_L, M_CLOCK_FREQ};
    if (!NodeExists(clock_freq_title))
    {
        PrintRequiredNotFound(clock_freq_title);
        return RET_FAILURE;
    }
    uint num_node_read = 0;
    ExtractNode(clock_freq_title, num_node_read); // Move cursor to testcases array

    m_xclbin_metadata.clock_freq_topology.m_clock_freq.clear();
    for (uint j = 0; j <(uint)json_reader_count_elements(m_json_reader); j++) // For each element in clock_freq array
    {
        json_reader_read_element(m_json_reader, j); // Move cursor to clock_freq element

        std::string tmp_str;
        // m_freq_Mhz
        node_title = {M_FREQ_MHZ};
        if (ExtractNodeValueStr(node_title, tmp_str) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        uint m_freq_Mhz;
        if (ConvString2Num<uint>(tmp_str, m_freq_Mhz) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_013, {this->m_content_name + " " + m_section, StrVectToStr(node_title, ".")});
            return RET_FAILURE;
        }
        // m_type
        node_title = {M_TYPE};
        std::string m_type;
        if (ExtractNodeValueStr(node_title, m_type) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        if (m_type == DATA)
        {
            xclbin_clock_freq_data.freq_Mhz = m_freq_Mhz;
            xclbin_clock_freq_data.found    = true;
        }
        if (m_type == KERNEL)
        {
            xclbin_clock_freq_kernel.freq_Mhz = m_freq_Mhz;
            xclbin_clock_freq_kernel.found    = true;
        }

        json_reader_end_element(m_json_reader); // Move back cursor to clock_freq array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from clock_freq array
    {
        json_reader_end_element(m_json_reader);
    }

    if (!xclbin_clock_freq_data.found)
    {
        LogMessage(MSG_ITF_151, {"clock 0 (ap_clk)"});
    }
    this->m_xclbin_metadata.clock_freq_topology.m_clock_freq.emplace_back(xclbin_clock_freq_data);
    if (!xclbin_clock_freq_kernel.found)
    {
        LogMessage(MSG_ITF_151, {"clock 1 (ap_clk_2)"});
    }
    this->m_xclbin_metadata.clock_freq_topology.m_clock_freq.emplace_back(xclbin_clock_freq_kernel);

    PrintClockFreqTopology();
    return RET_SUCCESS;
}
#endif

#ifdef USE_XRT
bool XclbinParser::ParseUserMetadata()
{
    this->m_section = USER_METADATA;
    LogMessage(MSG_ITF_039, {this->m_section});
    auto section_header = xclbin::get_axlf_section(this->m_xclbin_axlf, axlf_section_kind::USER_METADATA);
    char data [section_header->m_sectionSize+1];
    memcpy(data, this->m_buffer + section_header->m_sectionOffset, section_header->m_sectionSize);
    data[section_header->m_sectionSize] = '\0'; // terminate array with null-terminator
    auto user_metadata_json = std::string((char*)data);
    return ParseUserMetadataJsonContent(user_metadata_json);
}
#endif

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
bool XclbinParser::ParseUserMetadataJson()
{
    this->m_section = USER_METADATA;
    LogMessage(MSG_ITF_039, {this->m_section});
    this->m_filename = this->m_metadata_dir + "/user_metadata.json";
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {this->m_section, this->m_filename});
        return RET_FAILURE;
    }
    std::ifstream ifs(this->m_filename);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    auto user_metadata_json = buffer.str();
    return ParseUserMetadataJsonContent(user_metadata_json);
}
#endif

bool XclbinParser::ParseUserMetadataJsonContent( const std::string & user_metadata_json )
{
    std::vector<std::string> node_title;

    GError * error = nullptr;
    json_parser_load_from_data(this->m_json_parser, user_metadata_json.c_str(), user_metadata_json.size(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_content_name + " " + this->m_section, "<data>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        return RET_FAILURE;
    }

    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    // Build info
    // xbtest
    node_title = {BUILD_INFO};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, VERSION, MAJOR};
    if (ExtractNodeValueInt<uint>(node_title, this->m_xclbin_metadata.user_metadata.build_info.xbtest.version.major) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, VERSION, MINOR};
    if (ExtractNodeValueInt<uint>(node_title, this->m_xclbin_metadata.user_metadata.build_info.xbtest.version.minor) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, BUILD};
    if (ExtractNodeValueInt<uint>(node_title, this->m_xclbin_metadata.user_metadata.build_info.xbtest.build) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, DATE};
    if (ExtractNodeValueStr(node_title, this->m_xclbin_metadata.user_metadata.build_info.xbtest.date) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, XBTEST, INTERNALRELEASE};
    if (ExtractNodeValueBool(node_title, this->m_xclbin_metadata.user_metadata.build_info.xbtest.internal_release) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    // board
    node_title = {BUILD_INFO, BOARD, NAME};
    if (ExtractNodeValueStr(node_title, this->m_xclbin_metadata.user_metadata.build_info.board.name) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    node_title = {BUILD_INFO, BOARD, INTERFACE_UUID};
    if (ExtractNodeValueStr(node_title, this->m_xclbin_metadata.user_metadata.build_info.board.interface_uuid) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // Definition
    node_title = {DEFINITION};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // AIE
    // Note, a AIE definition is defined by wizard in USER_METADATA only if AIE is present in xclbin
    LogMessage(MSG_DEBUG_PARSING, {"Get AIE parameters defined in USER_METADATA"});
    this->m_xclbin_metadata.user_metadata.definition.aie.exists     = false;
    this->m_xclbin_metadata.user_metadata.definition.aie.freq       = 0;
    this->m_xclbin_metadata.user_metadata.definition.aie.type       = "";
    this->m_xclbin_metadata.user_metadata.definition.aie.control    = "";
    this->m_xclbin_metadata.user_metadata.definition.aie.status     = "";
    node_title = {DEFINITION, AIE};
    if (NodeExists(node_title))
    {
        // Get freq
        node_title = {DEFINITION, AIE, FREQ};
        if (ExtractNodeValueInt<uint>(node_title, this->m_xclbin_metadata.user_metadata.definition.aie.freq) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Get type
        node_title = {DEFINITION, AIE, TYPE};
        if (ExtractNodeValueStr(node_title, this->m_xclbin_metadata.user_metadata.definition.aie.type) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Get control
        node_title = {DEFINITION, AIE, CONTROL};
        if (ExtractNodeValueStr(node_title, this->m_xclbin_metadata.user_metadata.definition.aie.control) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Get status
        node_title = {DEFINITION, AIE, STATUS};
        if (ExtractNodeValueStr(node_title, this->m_xclbin_metadata.user_metadata.definition.aie.status) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        this->m_xclbin_metadata.user_metadata.definition.aie.exists = true;
    }

    // Clocks
    LogMessage(MSG_DEBUG_PARSING, {"Get clock parameters defined in USER_METADATA"});
    this->m_xclbin_metadata.user_metadata.definition.clocks.clear();

    node_title = {DEFINITION, CLOCKS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // List memory nodes, this should contains indexes from 0 to N
    std::vector<std::string> clock_member_list;
    if (ListNodeMembers(node_title, clock_member_list) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // Check if first index exists
    node_title = {DEFINITION, CLOCKS, "0"};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // Use of parse_array_end to force user to use back-to-back indexes
    auto parse_array_end = false;
    for (uint idx = 0; idx < clock_member_list.size(); idx++)
    {
        uint clock;

        node_title = {DEFINITION, CLOCKS, std::to_string(idx)};
        if (!NodeExists(node_title))
        {
            parse_array_end = true;
            continue; // Finished get clocks
        }
        if (parse_array_end)
        {
            LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, CLOCKS}, ".") });
            return RET_FAILURE;
        }
        // Get clock
        node_title = {DEFINITION, CLOCKS, std::to_string(idx)};
        if (ExtractNodeValueInt<uint>(node_title, clock) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        this->m_xclbin_metadata.user_metadata.definition.clocks.emplace_back(clock);
    }

    // Memory
    // Note, a memory definition is defined by wizard in USER_METADATA only if xbtest HW IP is present in xclbin
    LogMessage(MSG_DEBUG_PARSING, {"Get memory parameters defined in USER_METADATA"});
    this->m_xclbin_metadata.user_metadata.definition.memory.clear();

    node_title = {DEFINITION, MEMORY};
    if (NodeExists(node_title))
    {
        // List memory nodes, this should contains indexes from 0 to N
        std::vector<std::string> memory_member_list;
        if (ListNodeMembers(node_title, memory_member_list) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        // Check if first index exists
        node_title = {DEFINITION, MEMORY, "0"};
        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Use of parse_array_end to force user to use back-to-back indexes
        parse_array_end = false;
        for (uint idx = 0; idx < memory_member_list.size(); idx++)
        {
            Xclbin_UM_Def_Memory_t xclbin_um_def_memory;

            node_title = {DEFINITION, MEMORY, std::to_string(idx)};
            if (!NodeExists(node_title))
            {
                parse_array_end = true;
                continue; // Finished get memory
            }
            if (parse_array_end)
            {
                LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, MEMORY}, ".") });
                return RET_FAILURE;
            }
            // Get name
            node_title = {DEFINITION, MEMORY, std::to_string(idx), NAME};
            if (ExtractNodeValueStr(node_title, xclbin_um_def_memory.name) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get target
            node_title = {DEFINITION, MEMORY, std::to_string(idx), TARGET};
            if (ExtractNodeValueStr(node_title, xclbin_um_def_memory.target) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get type
            node_title = {DEFINITION, MEMORY, std::to_string(idx), TYPE};
            if (ExtractNodeValueStr(node_title, xclbin_um_def_memory.type) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get axi_data_size
            node_title = {DEFINITION, MEMORY, std::to_string(idx), AXI_DATA_SIZE};
            if (ExtractNodeValueInt<uint>(node_title, xclbin_um_def_memory.axi_data_size) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get num_axi_thread
            node_title = {DEFINITION, MEMORY, std::to_string(idx), NUM_AXI_THREAD};
            if (ExtractNodeValueInt<uint>(node_title, xclbin_um_def_memory.num_axi_thread) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }

            xclbin_um_def_memory.idx = idx;

            this->m_xclbin_metadata.user_metadata.definition.memory.emplace_back(xclbin_um_def_memory);
        }
    }

    // GT
    LogMessage(MSG_DEBUG_PARSING, {"Get GT parameters defined in USER_METADATA"});
    this->m_xclbin_metadata.user_metadata.definition.gt.clear();

    node_title = {DEFINITION, GT};
    if (NodeExists(node_title))
    {
        // List gt nodes, this should contains indexes from 0 to N
        std::vector<std::string> gt_member_list;
        if (ListNodeMembers(node_title, gt_member_list) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        // Check if first index exists
        node_title = {DEFINITION, GT, "0"};
        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Use of parse_array_end to force user to use back-to-back indexes
        parse_array_end = false;
        for (uint idx = 0; idx < gt_member_list.size(); idx++)
        {
            Xclbin_UM_Def_GT_t xclbin_um_def_gt;

            node_title = {DEFINITION, GT, std::to_string(idx)};
            if (!NodeExists(node_title))
            {
                parse_array_end = true;
                continue; // Finished get gt
            }
            if (parse_array_end)
            {
                LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, GT}, ".") });
                return RET_FAILURE;
            }
            // Get gt_index
            node_title = {DEFINITION, GT, std::to_string(idx), GT_INDEX};
            if (ExtractNodeValueInt<uint>(node_title, xclbin_um_def_gt.gt_index) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get gt_group_select
            node_title = {DEFINITION, GT, std::to_string(idx), GT_GROUP_SELECT};
            if (ExtractNodeValueStr(node_title, xclbin_um_def_gt.gt_group_select) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get gt_serial_port
            node_title = {DEFINITION, GT, std::to_string(idx), GT_SERIAL_PORT};
            if (ExtractNodeValueStr(node_title, xclbin_um_def_gt.gt_serial_port) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            // Get gt_diff_clocks
            node_title = {DEFINITION, GT, std::to_string(idx), GT_DIFF_CLOCKS};
            if (ExtractNodeValueStr(node_title, xclbin_um_def_gt.gt_diff_clocks) == RET_FAILURE)
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }
            this->m_xclbin_metadata.user_metadata.definition.gt.emplace_back(xclbin_um_def_gt);
        }
    }

    // xbtest HW IPs
    LogMessage(MSG_DEBUG_PARSING, {"Get xbtest HW IPs parameters defined in USER_METADATA"});

    node_title = {DEFINITION, COMPUTE_UNITS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // List xbtest HW IPs nodes, this should contains indexes from 0 to N
    std::vector<std::string> cu_member_list;
    if (ListNodeMembers(node_title, cu_member_list) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // Check if first index exists
    node_title = {DEFINITION, COMPUTE_UNITS, "0"};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }

    // Use of parse_array_end to force user to use back-to-back indexes
    parse_array_end = false;
    for (uint idx = 0; idx < cu_member_list.size(); idx++)
    {
        Xclbin_UM_Def_Compute_Units_t compute_unit;

        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx)};
        if (!NodeExists(node_title))
        {
            parse_array_end = true;
            continue; // Finished get clocks
        }
        if (parse_array_end)
        {
            LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, COMPUTE_UNITS}, ".") });
            return RET_FAILURE;
        }
        // Get name
        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), NAME};
        if (ExtractNodeValueStr(node_title, compute_unit.name) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Get slr
        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), SLR};
        if (ExtractNodeValueInt<uint>(node_title, compute_unit.SLR) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        // Get mode
        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), MODE};
        if (ExtractNodeValueInt<uint>(node_title, compute_unit.mode) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        // Get connectivity
        compute_unit.connectivity.clear();
        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CONNECTIVITY_L};

        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }

        // List xbtest HW IPs nodes, this should contains indexes from 0 to N
        std::vector<std::string> connectivity_member_list;
        if (ListNodeMembers(node_title, connectivity_member_list) == RET_FAILURE)
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        if (!connectivity_member_list.empty())
        {
            // Check if first index exists
            node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CONNECTIVITY_L, "0"};
            if (!NodeExists(node_title))
            {
                PrintRequiredNotFound(node_title);
                return RET_FAILURE;
            }

            // Use of parse_subarray_end to force user to use back-to-back indexes
            auto parse_subarray_end = false;
            for (uint j = 0; j < connectivity_member_list.size(); j++)
            {
                std::string connectivity;

                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CONNECTIVITY_L, std::to_string(j)};
                if (!NodeExists(node_title))
                {
                    parse_subarray_end = true;
                    continue; // Finished get clocks
                }
                if (parse_subarray_end)
                {
                    LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, COMPUTE_UNITS, std::to_string(idx), CONNECTIVITY}, ".") });
                    return RET_FAILURE;
                }
                // Get connectivity
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CONNECTIVITY_L, std::to_string(j)};
                if (ExtractNodeValueStr(node_title, connectivity) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                compute_unit.connectivity.emplace_back(connectivity);
            }
        }

        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION};
        if (!NodeExists(node_title))
        {
            PrintRequiredNotFound(node_title);
            return RET_FAILURE;
        }
        switch (compute_unit.mode)
        {
            case BI_VERIFY_HW_COMPONENT_ID:
            {
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, DNA_READ};
                if (ExtractNodeValueBool(node_title, compute_unit.cu_type_configuration.dna_read) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                break;
            }
            case BI_PWR_HW_COMPONENT_ID:
            {
                // Get SLR
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, SLR};
                if (ExtractNodeValueInt<uint>(node_title, compute_unit.cu_type_configuration.SLR) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                // Get throttle_mode
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, THROTTLE_MODE};
                if (ExtractNodeValueStr(node_title, compute_unit.cu_type_configuration.throttle_mode) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                // Get use_aie
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, USE_AIE};
                if (ExtractNodeValueInt<uint>(node_title, compute_unit.cu_type_configuration.use_aie) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                break;
            }
            case BI_MEM_HW_COMPONENT_ID:
            {
                // Get memory type
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, MEMORY_TYPE};
                if (ExtractNodeValueStr(node_title, compute_unit.cu_type_configuration.memory_type) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                break;
            }
            case BI_GT_MAC_HW_COMPONENT_ID:
            {
                // Get gt_index
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX};
                if (ExtractNodeValueInt<uint>(node_title, compute_unit.cu_type_configuration.gt_index) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                break;
            }
            case BI_GT_LPBK_HW_COMPONENT_ID:
            {
                // Get gt_index
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX};
                if (ExtractNodeValueInt<uint>(node_title, compute_unit.cu_type_configuration.gt_index) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                break;
            }
            case BI_GT_PRBS_HW_COMPONENT_ID:
            {
                // Get gt_index
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX};
                if (ExtractNodeValueInt<uint>(node_title, compute_unit.cu_type_configuration.gt_index) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                break;
            }
            case BI_GTF_PRBS_HW_COMPONENT_ID:
            {
                // Get gt_index
                compute_unit.cu_type_configuration.gtf_indexes.clear();
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX};

                if (!NodeExists(node_title))
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }

                // List xbtest HW IPs nodes, this should contains indexes from 0 to 31
                std::vector<std::string> gt_index_member_list;
                if (ListNodeMembers(node_title, gt_index_member_list) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                if (!gt_index_member_list.empty())
                {
                    // Check if first index exists
                    node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, "0"};
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(node_title);
                        return RET_FAILURE;
                    }

                    // Use of parse_subarray_end to force user to use back-to-back indexes
                    auto parse_subarray_end = false;
                    for (uint j = 0; j < gt_index_member_list.size(); j++)
                    {
                        bool enable;

                        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, std::to_string(j)};
                        if (!NodeExists(node_title))
                        {
                            parse_subarray_end = true;
                            continue; // Finished get clocks
                        }
                        if (parse_subarray_end)
                        {
                            LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX}, ".") });
                            return RET_FAILURE;
                        }
                        // Get enable
                        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, std::to_string(j)};
                        if (ExtractNodeValueBool(node_title, enable) == RET_FAILURE)
                        {
                            PrintRequiredNotFound(node_title);
                            return RET_FAILURE;
                        }
                        compute_unit.cu_type_configuration.gtf_indexes.emplace_back(enable);
                    }
                }
                break;
            }
            case BI_GTM_PRBS_HW_COMPONENT_ID:
            {
                // Get gt_index
                compute_unit.cu_type_configuration.gtm_indexes.clear();
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX};

                if (!NodeExists(node_title))
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }

                // List xbtest HW IPs nodes, this should contains indexes from 0 to 31
                std::vector<std::string> gt_index_member_list;
                if (ListNodeMembers(node_title, gt_index_member_list) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                if (!gt_index_member_list.empty())
                {
                    // Check if first index exists
                    node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, "0"};
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(node_title);
                        return RET_FAILURE;
                    }

                    // Use of parse_subarray_end to force user to use back-to-back indexes
                    auto parse_subarray_end = false;
                    for (uint j = 0; j < gt_index_member_list.size(); j++)
                    {
                        bool enable;

                        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, std::to_string(j)};
                        if (!NodeExists(node_title))
                        {
                            parse_subarray_end = true;
                            continue; // Finished get clocks
                        }
                        if (parse_subarray_end)
                        {
                            LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX}, ".") });
                            return RET_FAILURE;
                        }
                        // Get enable
                        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, std::to_string(j)};
                        if (ExtractNodeValueBool(node_title, enable) == RET_FAILURE)
                        {
                            PrintRequiredNotFound(node_title);
                            return RET_FAILURE;
                        }
                        compute_unit.cu_type_configuration.gtm_indexes.emplace_back(enable);
                    }
                }
                break;
            }
            case BI_GTYP_PRBS_HW_COMPONENT_ID:
            {
                // Get gt_index
                compute_unit.cu_type_configuration.gtyp_indexes.clear();
                node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX};

                if (!NodeExists(node_title))
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }

                // List xbtest HW IPs nodes, this should contains indexes from 0 to 31
                std::vector<std::string> gt_index_member_list;
                if (ListNodeMembers(node_title, gt_index_member_list) == RET_FAILURE)
                {
                    PrintRequiredNotFound(node_title);
                    return RET_FAILURE;
                }
                if (!gt_index_member_list.empty())
                {
                    // Check if first index exists
                    node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, "0"};
                    if (!NodeExists(node_title))
                    {
                        PrintRequiredNotFound(node_title);
                        return RET_FAILURE;
                    }

                    // Use of parse_subarray_end to force user to use back-to-back indexes
                    auto parse_subarray_end = false;
                    for (uint j = 0; j < gt_index_member_list.size(); j++)
                    {
                        bool enable;

                        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, std::to_string(j)};
                        if (!NodeExists(node_title))
                        {
                            parse_subarray_end = true;
                            continue; // Finished get clocks
                        }
                        if (parse_subarray_end)
                        {
                            LogMessage(MSG_ITF_092, {std::to_string(idx), StrVectToStr({DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX}, ".") });
                            return RET_FAILURE;
                        }
                        // Get enable
                        node_title = {DEFINITION, COMPUTE_UNITS, std::to_string(idx), CU_TYPE_CONFIGURATION, GT_INDEX, std::to_string(j)};
                        if (ExtractNodeValueBool(node_title, enable) == RET_FAILURE)
                        {
                            PrintRequiredNotFound(node_title);
                            return RET_FAILURE;
                        }
                        compute_unit.cu_type_configuration.gtyp_indexes.emplace_back(enable);
                    }
                }
                break;
            }
            default: break;
        }
        this->m_xclbin_metadata.user_metadata.definition.compute_units.emplace_back(compute_unit);
    }

    PrintUserMetadata();
    return RET_SUCCESS;
}

void XclbinParser::PrintUserMetadata()
{
    LogMessage(MSG_DEBUG_SETUP, {"USER_METADATA:"});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Build Info:"});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t * xbtest:"});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Version:"});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # Major: " + std::to_string(this->m_xclbin_metadata.user_metadata.build_info.xbtest.version.major)});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # Minor: " + std::to_string(this->m_xclbin_metadata.user_metadata.build_info.xbtest.version.minor)});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Build: "   + std::to_string(this->m_xclbin_metadata.user_metadata.build_info.xbtest.build)});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Date : " +                 this->m_xclbin_metadata.user_metadata.build_info.xbtest.date});
    if (this->m_xclbin_metadata.user_metadata.build_info.xbtest.internal_release)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Release : INTERNAL"});
    }
    LogMessage(MSG_DEBUG_SETUP, {"\t\t * Board:"});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Name          : " + this->m_xclbin_metadata.user_metadata.build_info.board.name});
    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Interface UUID: " + this->m_xclbin_metadata.user_metadata.build_info.board.interface_uuid});


    LogMessage(MSG_DEBUG_SETUP, {"\t - Definition:"});
    if (this->m_xclbin_metadata.user_metadata.definition.aie.exists)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * AIE:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Frequency (MHz): " + std::to_string(this->m_xclbin_metadata.user_metadata.definition.aie.freq)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Type           : " +                this->m_xclbin_metadata.user_metadata.definition.aie.type});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Control        : " +                this->m_xclbin_metadata.user_metadata.definition.aie.control});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Status         : " +                this->m_xclbin_metadata.user_metadata.definition.aie.status});
    }
    for (uint i = 0; i < this->m_xclbin_metadata.user_metadata.definition.clocks.size(); i++)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Clock[" + std::to_string(i) + "]:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Frequency (MHz): " + std::to_string(this->m_xclbin_metadata.user_metadata.definition.clocks[i])});
    }

    for (const auto & gt : this->m_xclbin_metadata.user_metadata.definition.gt)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * GT[" + std::to_string(gt.gt_index) + "]:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + GT group select: " + gt.gt_group_select});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + GT serial port : " + gt.gt_serial_port});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + GT diff clock  : " + gt.gt_diff_clocks});
    }
    for (const auto & memory : this->m_xclbin_metadata.user_metadata.definition.memory)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * Memory[" + std::to_string(memory.idx) + "]:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Name                 : " +                memory.name});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Type                 : " +                memory.type});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Target               : " +                memory.target});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + AXI data size (bits) : " + std::to_string(memory.axi_data_size)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + Number of AXI Threads: " + std::to_string(memory.num_axi_thread)});
    }

    for (uint i = 0; i < this->m_xclbin_metadata.user_metadata.definition.compute_units.size(); i++)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t\t * xbtest HW IP[" + std::to_string(i) + "]:"});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + name           : " +                this->m_xclbin_metadata.user_metadata.definition.compute_units[i].name});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + SLR            : " + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].SLR)});
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + mode           : " + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].mode)});
        if (this->m_xclbin_metadata.user_metadata.definition.compute_units[i].connectivity.empty())
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + BD tag(s)      : NONE"});
        }
        else
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + BD tag(s)      :"});
            for (uint j = 0; j < this->m_xclbin_metadata.user_metadata.definition.compute_units[i].connectivity.size(); j++)
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # Port[" + std::to_string(j) + "] : " + this->m_xclbin_metadata.user_metadata.definition.compute_units[i].connectivity[j]});
            }
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t\t\t + xbtest HW IP type specific configuration:"});
        switch (this->m_xclbin_metadata.user_metadata.definition.compute_units[i].mode)
        {
            case BI_PWR_HW_COMPONENT_ID:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # SLR          : " + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.SLR)});
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # Throttle mode: " +                this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.throttle_mode});
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # AIE          : " + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.use_aie)});
                break;
            }
            case BI_MEM_HW_COMPONENT_ID:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # Memory type: "   +                this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.memory_type});
                break;
            }
            case BI_GT_MAC_HW_COMPONENT_ID:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # GT index: "      + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.gt_index)});
                break;
            }
            case BI_GT_LPBK_HW_COMPONENT_ID:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # GT index: "      + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.gt_index)});
                break;
            }
            case BI_GT_PRBS_HW_COMPONENT_ID:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # GT index: "      + std::to_string(this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.gt_index)});
                break;
            }
            case BI_GTF_PRBS_HW_COMPONENT_ID:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # GT indexes:"});
                for (uint gt_index=0; gt_index<this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.gtf_indexes.size(); gt_index++)
                {
                    auto enable = this->m_xclbin_metadata.user_metadata.definition.compute_units[i].cu_type_configuration.gtf_indexes[gt_index];
                    LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t\t > GT [" + std::to_string(gt_index) + "]: " + BoolToStr(enable)});
                }
                break;
            }
            default:
            {
                LogMessage(MSG_DEBUG_SETUP, {"\t\t\t\t # No parameter defined"});
                break;
            }
        }
    }
}

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
void XclbinParser::PrintPartitionMetadata()
{
    uint i;
    LogMessage(MSG_DEBUG_SETUP, {"PARTITION_METADATA:"});
    i=0;
    for (const auto & pcie_bar : this->m_xclbin_metadata.partition_metadata.pcie_bars)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - pcie_bar[" + std::to_string(i) + "]: name = " + pcie_bar.name + " / pcie_base_address_register = " + std::to_string(pcie_bar.pcie_base_address_register) + " / pcie_physical_function = " + std::to_string(pcie_bar.pcie_physical_function) + " / offset = 0x" + NumToStrHex(pcie_bar.offset, 16) + " / range = 0x" + NumToStrHex(pcie_bar.range, 16)});
        i++;
    }
    i=0;
    for (const auto & pcie_ep : this->m_xclbin_metadata.partition_metadata.pcie_eps)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - pcie_ep[" + std::to_string(i) + "]: name = " + pcie_ep.name + " / pcie_base_address_register = " + std::to_string(pcie_ep.pcie_base_address_register) + " / pcie_physical_function = " + std::to_string(pcie_ep.pcie_physical_function) + " / offset = 0x" + NumToStrHex(pcie_ep.offset, 16) + " / range = 0x" + NumToStrHex(pcie_ep.range, 16)});
        i++;
    }
}
#endif

void XclbinParser::PrintIpLayout()
{
    LogMessage(MSG_DEBUG_SETUP, {"IP_LAYOUT:"});
    for (const auto & ip_data : this->m_xclbin_metadata.ip_layout.m_ip_data)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - ip_data[" + std::to_string(ip_data.m_ip_layout_index) + "]: m_name = " + ip_data.m_name});
    }
}

void XclbinParser::PrintMemoryTopology()
{
    LogMessage(MSG_DEBUG_SETUP, {"MEMORY TOPOLOGY:"});
    for (const auto & memory_data: this->m_xclbin_metadata.memory_topology)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - memory_data[" + std::to_string(memory_data.mem_data_idx) + "]: m_used = " + std::to_string(memory_data.m_used) + " / memory_data.size_bytes = " + NumToStrHex(memory_data.size_bytes) + " / m_tag = " + memory_data.m_tag + " / m_ip_name = " + memory_data.m_ip_name});
    }
}

void XclbinParser::PrintConnectivity()
{
    LogMessage(MSG_DEBUG_SETUP, {"CONNECTIVITY:"});
    for (uint j = 0; j < this->m_xclbin_metadata.connectivity.m_connection.size(); j++)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - connection[" + std::to_string(j) + "]: arg_index = " + std::to_string(this->m_xclbin_metadata.connectivity.m_connection[j].arg_index) + " / m_ip_layout_index = " + std::to_string(this->m_xclbin_metadata.connectivity.m_connection[j].m_ip_layout_index) + " / mem_data_index = " + std::to_string(this->m_xclbin_metadata.connectivity.m_connection[j].mem_data_index)});
    }
}
void XclbinParser::PrintClockFreqTopology()
{
    LogMessage(MSG_DEBUG_SETUP, {"CLOCK_FREQ_TOPOLOGY:"});
    for (uint j = 0; j < this->m_xclbin_metadata.clock_freq_topology.m_clock_freq.size(); j++)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - clock_freq[" + std::to_string(j) + "]: freq_Mhz = " + std::to_string(this->m_xclbin_metadata.clock_freq_topology.m_clock_freq[j].freq_Mhz) + " / found = " + BoolToStr(this->m_xclbin_metadata.clock_freq_topology.m_clock_freq[j].found) });
    }
}

void XclbinParser::PrintRequiredNotFound( const std::vector<std::string> & node_title_in )
{
    LogMessage(MSG_ITF_044, {this->m_content_name + " " + this->m_section, StrVectToStr(node_title_in, ".")});
}

} // namespace

