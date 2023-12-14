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

#ifndef _XCLBINPARSER_H
#define _XCLBINPARSER_H

#ifdef USE_XRT
// include XRT xclbin header
#include "xclbin.h"
#endif

#include "xbtestswpackage.h"
#include "xjsonparser.h"

namespace xbtest
{

class XclbinParser: public XJsonParser
{

public:

    XclbinParser( Logging * log, const std::string & xclbin, const std::string & design_pdi, std::atomic<bool> * abort );
    ~XclbinParser();

    bool                Parse() override;

#ifdef USE_XRT
    bool                ParseIpLayout();
    bool                ParseMemoryTopology();
    bool                ParseConnectivity();
    bool                ParseClockFreqTopology();
    bool                ParseUserMetadata();
#endif

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    bool                ParsePartitionMetadataJson();
    bool                ParseMemoryTopologyJson();
    bool                ParseConnectivityJson();
    bool                ParseClockFreqTopologyJson();
    bool                ParseUserMetadataJson();
#endif

    bool                ParseUserMetadataJsonContent( const std::string & user_metadata_json );

    Xclbin_Metadata_t   GetXclbinMetadata();

private:
    // SECTION NAMES
    const std::string PCIE                          = "pcie";
    const std::string BARS                          = "bars";
    const std::string ADDRESSABLE_ENDPOINTS         = "addressable_endpoints";
    const std::string PCIE_BASE_ADDRESS_REGISTER    = "pcie_base_address_register";
    const std::string PCIE_PHYSICAL_FUNCTION        = "pcie_physical_function";
    const std::string OFFSET2                       = "offset";
    const std::string RANGE                         = "range";

    const std::string PARTITION_METADATA        = "PARTITION_METADATA";
    const std::string IP_LAYOUT                 = "IP_LAYOUT";
    const std::string MEM_TOPOLOGY              = "MEM_TOPOLOGY";
    const std::string CONNECTIVITY              = "CONNECTIVITY";
    const std::string CLOCK_FREQ_TOPOLOGY       = "CLOCK_FREQ_TOPOLOGY";

    const std::string PARTITION_METADATA_L      = "partition_metadata";
    const std::string IP_LAYOUT_L               = "ip_layout";
    const std::string MEM_TOPOLOGY_L            = "mem_topology";
    const std::string CONNECTIVITY_L            = "connectivity";
    const std::string CLOCK_FREQ_TOPOLOGY_L     = "clock_freq_topology";

    const std::string IP_NAME                   = "ip_name";
    const std::string M_BASE_ADDRESS            = "m_base_address";
    const std::string M_MEM_DATA                = "m_mem_data";
    const std::string M_USED                    = "m_used";
    const std::string M_SIZEKB                  = "m_sizekb";
    const std::string M_IP_NAME                 = "m_ip_name";
    const std::string M_TAG                     = "m_tag";
    const std::string M_CONNECTION              = "m_connection";
    const std::string ARG_INDEX                 = "arg_index";
    const std::string M_IP_LAYOUT_INDEX         = "m_ip_layout_index";
    const std::string MEM_DATA_INDEX            = "mem_data_index";
    const std::string DATA                      = "DATA";
    const std::string KERNEL                    = "KERNEL";
    const std::string M_CLOCK_FREQ              = "m_clock_freq";
    const std::string M_FREQ_MHZ                = "m_freq_Mhz";
    const std::string M_TYPE                    = "m_type";

    const std::string USER_METADATA             = "USER_METADATA";
    const std::string BUILD_INFO                = "build_info";
    const std::string XBTEST                    = "xbtest";
    const std::string MAJOR                     = "major";
    const std::string MINOR                     = "minor";
    const std::string BUILD                     = "build";
    const std::string DATE                      = "date";
    const std::string INTERNALRELEASE           = "internal_release";
    const std::string NAME                      = "name";
    const std::string INTERFACE_UUID            = "interface_uuid";
    const std::string DEFINITION                = "definition";
    const std::string AIE                       = "aie";
    const std::string FREQ                      = "freq";
    const std::string TYPE                      = "type";
    const std::string CONTROL                   = "control";
    const std::string STATUS                    = "status";
    const std::string CLOCKS                    = "clocks";
    const std::string MEMORY                    = "memory";
    const std::string TARGET                    = "target";
    const std::string AXI_DATA_SIZE             = "axi_data_size";
    const std::string NUM_AXI_THREAD            = "num_axi_thread";
    const std::string GT                        = "gt";
    const std::string GT_INDEX                  = "gt_index";
    const std::string GT_GROUP_SELECT           = "gt_group_select";
    const std::string GT_SERIAL_PORT            = "gt_serial_port";
    const std::string GT_DIFF_CLOCKS            = "gt_diff_clocks";
    const std::string COMPUTE_UNITS             = "compute_units";
    const std::string SLR                       = "SLR";
    const std::string MODE                      = "mode";
    const std::string CU_TYPE_CONFIGURATION     = "cu_type_configuration";
    const std::string MEMORY_TYPE               = "memory_type";
    const std::string THROTTLE_MODE             = "throttle_mode";
    const std::string USE_AIE                   = "use_aie";
    const std::string DNA_READ                  = "dna_read";

    char *              m_buffer        = nullptr;
#ifdef USE_XRT
    axlf *              m_xclbin_axlf   = nullptr;
#endif
    std::string         m_xclbin;
    std::string         m_design_pdi;
    std::string         m_metadata_dir;
    Xclbin_Metadata_t   m_xclbin_metadata;
    std::string         m_section;
    std::string         m_filename;

#if defined(USE_AMI) || defined(USE_NO_DRIVER)
    void PrintPartitionMetadata();
#endif
    void PrintIpLayout();
    void PrintMemoryTopology();
    void PrintConnectivity();
    void PrintClockFreqTopology();
    void PrintUserMetadata();
    void PrintRequiredNotFound( const std::vector<std::string> & node_title_in );

};

} // namespace

#endif /* _XCLBINPARSER_H */
