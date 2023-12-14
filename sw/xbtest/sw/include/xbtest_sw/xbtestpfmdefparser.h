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

#ifndef _XBTESTPFMDEFPARSER_H
#define _XBTESTPFMDEFPARSER_H

#include "xjsonparser.h"
#include "xbtestswpackage.h"
#include "gtmacpackage.h"

namespace xbtest
{

class XbtestPfmDefParser : public XJsonParser
{

public:
    XbtestPfmDefParser( Logging * log, const std::string & filename, const bool & is_nodma, const Pcie_Info_t & pcie_info, bool is_p2p_supported, bool is_mmio_test_supported, std::atomic<bool> * abort );
    ~XbtestPfmDefParser();

    bool                Parse() override;
    void                PrintPlatformDef( const Xbtest_Pfm_Def_t & xbtest_pfm_def );
    Json_Definition_t   GetJsonDefinition();
    Xbtest_Pfm_Def_t    GetPlatformDef();

private:
    Xbtest_Pfm_Def_t    m_xbtest_pfm_def;
    std::string         m_filename;
    bool                m_is_nodma;
    Pcie_Info_t         m_pcie_info = RST_PCIE_INFO;
    bool                m_is_p2p_supported;
    bool                m_is_mmio_test_supported;

    const std::string DEVICE            = "device";
    const std::string RUNTIME           = "runtime";
    const std::string DOWNLOAD_TIME     = "download_time";
    const std::string SENSOR            = "sensor";
    const std::string ID                = "id";
    const std::string TYPE              = "type";
    const std::string CARD_POWER        = "card_power";
    const std::string CARD_TEMPERATURE  = "card_temperature";
    const std::string MEMORY            = "memory";
    const std::string MMIO              = "mmio";
    const std::string BW                = "bw";
    const std::string CONFIG            = "config";
    const std::string DMA_BW            = "dma_bw";
    const std::string DMA_CONFIG        = "dma_config";
    const std::string P2P_CARD_BW       = "p2p_card_bw";
    const std::string P2P_CARD_CONFIG   = "p2p_card_config";
    const std::string P2P_NVME_BW       = "p2p_nvme_bw";
    const std::string P2P_NVME_CONFIG   = "p2p_nvme_config";
    const std::string TOTAL_SIZE        = "total_size";
    const std::string BUFFERSIZE        = "buffer_size";
    const std::string CU_BW             = "cu_bw";
    const std::string CU_LATENCY        = "cu_latency";
    const std::string CU_RATE           = "cu_rate";
    const std::string CU_OUTSTANDING    = "cu_outstanding";
    const std::string CU_BURST_SIZE     = "cu_burst_size";
    const std::string ALT_WR_RD         = "alt_wr_rd";
    const std::string SIMUL_WR_RD       = "simul_wr_rd";
    const std::string WRITE             = "write";
    const std::string READ              = "read";
    const std::string AVERAGE           = "average";
    const std::string HIGH              = "high";
    const std::string LOW               = "low";
    const std::string NOMINAL           = "nominal";

    const std::string GT                    = "GT";
    const std::string NAME                  = "name";
    const std::string TRANSCEIVER_SETTINGS  = "transceiver_settings";
    const std::string TX_POLARITY           = "tx_polarity";
    const std::string TX_MAIN_CURSOR        = "tx_main_cursor";
    const std::string TX_DIFF_CTRL          = "tx_differential_swing_control";
    const std::string TX_PRE_EMPH           = "tx_pre_emphasis";
    const std::string TX_POST_EMPH          = "tx_post_emphasis";
    const std::string RX_EQUALISER          = "rx_equaliser";
    const std::string RX_POLARITY           = "rx_polarity";
    const std::string GT_TRANS_SET_CABLE    = "cable";
    const std::string GT_TRANS_SET_MODULE   = "module";

    const uint MAX_GT_QTY               = 127; // dummy limits
    const uint MAX_MEM_TYPE_QTY         = 127; // dummy limits
    const uint DEFAULT_CU_RATE          = 100;
    const uint DEFAULT_CU_OUTSTANDING   = 0;

    // PCIe Generation 1.1, one lane max BW is 256 MBps
    const double MAX_PCIE_BW_1_1_x_1                = 256.0; // MBps
    const double MMIO_AVG_TO_HI_LO_PERCENT          = 25.0;  // +/- 25%
    const double DMA_DEFAULT_HI_TO_LO_PERCENT       = 40.0;  //   - 40%
    const double DMA_AVG_TO_HI_LO_PERCENT           = 25.0;  // +/- 25%
    const double P2P_CARD_AVG_TO_HI_LO_PERCENT      = 25.0;  // +/- 25%
    const double P2P_NVME_AVG_TO_HI_LO_PERCENT      = 25.0;  // +/- 25%
    const double CU_BW_AVG_TO_HI_LO_PERCENT         = 10.0;  // +/- 25%
    const double CU_LAT_AVG_TO_HI_LO_PERCENT        = 10.0;  // +/- 25%

    void PrintRequiredNotFound      ( const std::vector<std::string> & node_title );
    void PrintOptionalNotFound      ( const std::vector<std::string> & node_title );
    bool ExtractRequiredString      ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, std::string & value );
    bool ExtractOptionalString      ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, std::string & value, bool & found );
    bool ExtractOptionalBool        ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, bool & value, bool & found );
    bool ExtractRequiredUint        ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint &        value );
    bool ExtractOptionalUint8       ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint8_t &     value, bool & found );
    bool ExtractOptionalUint        ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint &        value, bool & found );
    bool ExtractOptionalUint64      ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, uint64_t &    value, bool & found );
    bool ExtractRequiredThreshold   ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, const double & percent, Mem_Thresh_t & thresh );
    bool ExtractOptionalThreshold   ( const std::vector<std::string> & base_title, const std::vector<std::string> & rel_node_title, const double & percent, Mem_Thresh_t & thresh );

};

} // namespace

#endif /* _XBTESTPFMDEFPARSER_H */
