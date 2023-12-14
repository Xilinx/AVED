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

#include "deviceinterface.h"
#include "clpackage.h"

namespace xbtest
{

// READ_CU_MIGRATE:
//      - when set to true, use migrate for PLRAM read in ReadComputeUnit(), when false, use EnqueueRead,
//      - Not implemented for GT_MAC xbtest HW IP status/config and Memory xbtest HW IP status

DeviceInterface_Base::DeviceInterface_Base( Logging * log, XbtestSwConfig * xbtest_sw_config, XbtestSwConfig * xbtest_sw_config_p2p, std::atomic<bool> * abort, const bool & p2p_target )
{
    MSG_ITF_017_WARN.log_level = LOG_WARN;

    this->m_abort             = abort;
    this->m_log               = log;
    if (p2p_target)
    {
        this->m_xbtest_sw_config = xbtest_sw_config_p2p;
    }
    else
    {
        this->m_xbtest_sw_config = xbtest_sw_config;
    }
    this->m_p2p_target = p2p_target;
    this->m_device_bdf = this->m_xbtest_sw_config->GetBdf();
}

DeviceInterface_Base::~DeviceInterface_Base()
{
    if (GetNumUnknownCu() > 0)
    {
        for (const auto & cl_compute_unit : this->m_cl_unknown_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumSkippedCu() > 0)
    {
        for (const auto & cl_compute_unit : this->m_cl_skipped_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumCompanionCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_companion_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumVerifyCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_verify_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumPowerCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_pwr_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumGtmacCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_gt_mac_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumGtlpbkCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_gt_lpbk_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumGtPrbsCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_gt_prbs_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumGtfPrbsCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_gtf_prbs_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumGtmPrbsCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_gtm_prbs_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    if (GetNumGtypPrbsCu() > 0)
    {
        for (auto & cl_compute_unit : this->m_cl_gtyp_prbs_compute_units)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        for (auto & cl_compute_unit : name_cu.second)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        for (const auto & cl_compute_unit : name_cu.second)
        {
            delete cl_compute_unit.cu_mtx;
        }
    }
    this->m_cl_unknown_compute_units.clear();
    this->m_cl_skipped_compute_units.clear();
    this->m_cl_companion_compute_units.clear();
    this->m_cl_verify_compute_units.clear();
    this->m_cl_pwr_compute_units.clear();
    this->m_cl_gt_mac_compute_units.clear();
    this->m_cl_gt_lpbk_compute_units.clear();
    this->m_cl_gt_prbs_compute_units.clear();
    this->m_cl_gtf_prbs_compute_units.clear();
    this->m_cl_gtm_prbs_compute_units.clear();
    this->m_cl_gtyp_prbs_compute_units.clear();
    this->m_cl_mem_sc_compute_units.clear();
    this->m_cl_mem_mc_compute_units.clear();
}

// PDI download not supported when running with AMI or without driver, return succes by default
bool DeviceInterface_Base::CheckXclbinDownloadTime() { return RET_SUCCESS; }

bool DeviceInterface_Base::CheckDeviceInfo()
{
    // Check Device state
    auto device_state_info = this->m_xbtest_sw_config->GetDeviceStateInfo();
    if (device_state_info.current_state != device_state_info.expected_state)
    {
        LogMessage(MSG_ITF_138, {"Device state", device_state_info.current_state, device_state_info.expected_state});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"Device state (" + device_state_info.current_state + ") matches expected (" + device_state_info.expected_state + ")"});
    }

    // Check Design info
    auto design_info = this->m_xbtest_sw_config->GetDesignInfo();

#ifdef USE_XRT
    // Satellite Controller
    if (design_info.controller.satellite_controller.version != design_info.controller.satellite_controller.expected_version)
    {
        LogMessage(MSG_ITF_138, {"Satellite controller version", design_info.controller.satellite_controller.version, design_info.controller.satellite_controller.expected_version});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"Satellite controller version (" + design_info.controller.satellite_controller.version + ") matches expected (" + design_info.controller.satellite_controller.expected_version + ")"});
    }
#endif
/*
    // AMC: No expected version is available
    if (design_info.controller.amc.version != design_info.controller.amc.expected_version)
    {
        LogMessage(MSG_ITF_138, {"AMC version", design_info.controller.amc.version, design_info.controller.amc.expected_version});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"AMC version (" + design_info.controller.amc.version + ") matches expected (" + design_info.controller.amc.expected_version + ")"});
    }
*/
    // Check PCIe info
    auto pcie_info = this->m_xbtest_sw_config->GetPcieInfo();
    if (pcie_info.link_speed_gbit_sec != pcie_info.expected_link_speed_gbit_sec)
    {
        LogMessage(MSG_ITF_138, {"PCIe link speed", std::to_string(pcie_info.link_speed_gbit_sec), std::to_string(pcie_info.expected_link_speed_gbit_sec)});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"PCIe link speed (" + std::to_string(pcie_info.link_speed_gbit_sec) + ") matches expected (" + std::to_string(pcie_info.expected_link_speed_gbit_sec) + ")"});
    }
    if (pcie_info.express_lane_width_count != pcie_info.expected_express_lane_width_count)
    {
        LogMessage(MSG_ITF_138, {"PCIe lane width", std::to_string(pcie_info.express_lane_width_count), std::to_string(pcie_info.expected_express_lane_width_count)});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"PCIe lane width (" + std::to_string(pcie_info.express_lane_width_count) + ") matches expected (" + std::to_string(pcie_info.expected_express_lane_width_count) + ")"});
    }
    return RET_SUCCESS;
}

uint64_t DeviceInterface_Base::RoundUpToMultiple( const uint64_t & value, const uint64_t & multiple )
{
    // this function rounds up the input value to the next multiple
    if (value <= multiple)
    {
        return multiple;
    }
    uint64_t remainder = value % multiple;
    if (remainder == 0)
    {
        return value;
    }
    return value + multiple - remainder;
}

// Low level xbtest HW IP access not supported when running with XRT, return succes by default
bool DeviceInterface_Base::CheckCUsNotDeadlocked() { return RET_SUCCESS; }
bool DeviceInterface_Base::InitCUArgs() { return RET_SUCCESS; }

// PLRAM host buffer creation not applicable running with AMI or without driver, return succes by default
bool DeviceInterface_Base::CreateAllM00AXIBuffers() { return RET_SUCCESS; }
bool DeviceInterface_Base::ConfigurePlramShadowBuffers() { return RET_SUCCESS; }

// PS kernel not supported when running with AMI or without driver, return succes by default
bool DeviceInterface_Base::SetupPsComputeUnits() { return RET_SUCCESS; }

bool DeviceInterface_Base::SetupComputeUnits() {

    LogMessage(MSG_ITF_023);

    for (const auto & compute_unit : this->m_xbtest_sw_config->GetComputeUnitDefinitions())
    {
        Cl_Compute_Unit_t cl_compute_unit;

        if (InitStruct(compute_unit, cl_compute_unit) != RET_SUCCESS)
        {
            return RET_FAILURE;
        }

        if (compute_unit.skipped)
        {
            this->m_cl_skipped_compute_units.emplace_back(std::move(cl_compute_unit));
            continue;
        }
        if (compute_unit.companion)
        {
            this->m_cl_companion_compute_units.emplace_back(std::move(cl_compute_unit));
            continue;
        }

        switch (compute_unit.mode)
        {
            case BI_VERIFY_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(VERIFY_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                cl_compute_unit.dna_read                = compute_unit.cu_type_configuration.dna_read;
                this->m_cl_verify_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_PWR_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(POWER_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                this->m_cl_pwr_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_MEM_HW_COMPONENT_ID:
                if (compute_unit.cu_type_configuration.memory->type == SINGLE_CHANNEL)
                {
                    cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(MEMORY_CHAN_BUF_SIZE, BUF_SIZE_MULTIPLE);
                    cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                    auto search = this->m_cl_mem_sc_compute_units.find(compute_unit.cu_type_configuration.memory->name);
                    if (search != this->m_cl_mem_sc_compute_units.end())
                    {
                        search->second.emplace_back(std::move(cl_compute_unit));
                    }
                    else
                    {
                        std::vector<Cl_Compute_Unit_t> cl_compute_units;
                        cl_compute_units.emplace_back(std::move(cl_compute_unit));
                        this->m_cl_mem_sc_compute_units.emplace(compute_unit.cu_type_configuration.memory->name, std::move(cl_compute_units));
                    }
                }
                else
                {
                    cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(MEMORY_CHAN_BUF_SIZE * (uint64_t)(compute_unit.cu_type_configuration.memory->num_channels), BUF_SIZE_MULTIPLE);
                    cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                    auto search = this->m_cl_mem_mc_compute_units.find(compute_unit.cu_type_configuration.memory->name);
                    if (search != this->m_cl_mem_mc_compute_units.end())
                    {
                        search->second.emplace_back(std::move(cl_compute_unit));
                    }
                    else
                    {
                        std::vector<Cl_Compute_Unit_t> cl_compute_units;
                        cl_compute_units.emplace_back(std::move(cl_compute_unit));
                        this->m_cl_mem_mc_compute_units.emplace(compute_unit.cu_type_configuration.memory->name, std::move(cl_compute_units));
                    }
                }
                break;
            case BI_GT_MAC_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(GT_MAC_BUF_SIZE * (uint64_t)(sizeof(uint32_t)), BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                cl_compute_unit.gt_index                = compute_unit.cu_type_configuration.gt->gt_index;
                this->m_cl_gt_mac_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_GT_LPBK_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(GT_LPBK_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                cl_compute_unit.gt_index                = compute_unit.cu_type_configuration.gt->gt_index;
                this->m_cl_gt_lpbk_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_GT_PRBS_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(GT_PRBS_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                cl_compute_unit.gt_index                = compute_unit.cu_type_configuration.gt->gt_index;
                this->m_cl_gt_prbs_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_GTF_PRBS_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(GTF_PRBS_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                for (auto gt : compute_unit.cu_type_configuration.gts)
                {
                    cl_compute_unit.gtf_indexes.emplace_back(gt->gt_index);
                }
                this->m_cl_gtf_prbs_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_GTM_PRBS_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(GTM_PRBS_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                for (auto gt : compute_unit.cu_type_configuration.gts)
                {
                    cl_compute_unit.gtm_indexes.emplace_back(gt->gt_index);
                }
                this->m_cl_gtm_prbs_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            case BI_GTYP_PRBS_HW_COMPONENT_ID:
                cl_compute_unit.cl_m00_axi_buffer_size  = RoundUpToMultiple(GTYP_PRBS_BUF_SIZE, BUF_SIZE_MULTIPLE);
                cl_compute_unit.cl_m00_axi_mem_data_idx = compute_unit.connectivity[0].mem_data_indexes[0];
                for (auto gt : compute_unit.cu_type_configuration.gts)
                {
                    cl_compute_unit.gtyp_indexes.emplace_back(gt->gt_index);
                }
                this->m_cl_gtyp_prbs_compute_units.emplace_back(std::move(cl_compute_unit));
                break;
            default:
                this->m_cl_unknown_compute_units.emplace_back(std::move(cl_compute_unit));
                break; // unknown mode, should not happen by definition (see wizard)
        }
    }
    // Report number of xbtest HW IPs
    if (GetNumSkippedCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumSkippedCu()), "ignored (skipped)"});
    }
    if (GetNumCompanionCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumCompanionCu()), "ignored (companion)"});
    }
    // the HW design must contain a Verify xbtest HW IP
    if (GetNumVerifyCu() == 0)
    {
        LogMessage(MSG_ITF_149);
        return RET_FAILURE;
    }
    LogMessage(MSG_ITF_030, {std::to_string(GetNumVerifyCu()), "verify"});

    if (GetNumPowerCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumPowerCu()), "power"});
    }
    if (GetNumGtmacCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumGtmacCu()), "GT MAC"});
    }
    if (GetNumGtlpbkCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumGtlpbkCu()), "GT LPBK"});
    }
    if (GetNumGtPrbsCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumGtPrbsCu()), "GT PRBS"});
    }
    if (GetNumGtfPrbsCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumGtfPrbsCu()), "GTF PRBS"});
    }
    if (GetNumGtmPrbsCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumGtmPrbsCu()), "GTM PRBS"});
    }
    if (GetNumGtypPrbsCu() != 0)
    {
        LogMessage(MSG_ITF_030, {std::to_string(GetNumGtypPrbsCu()), "GTYP PRBS"});
    }
    for (const auto & memory : this->m_xbtest_sw_config->GetMemoryDefinitions())
    {
        if (GetNumMemoryCu(memory.type, memory.name) != 0)
        {
            LogMessage(MSG_ITF_030, {std::to_string(GetNumMemoryCu(memory.type, memory.name)), memory.name + " memory (" + memory.type + ")"});
        }
    }
    // Report unknown xbtest HW IP names
    std::vector<std::string> cu_names;
    if (GetNumUnknownCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_unknown_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_ITF_031, {StrVectToStr(cu_names, ", ")});
    }
    // Report other xbtest HW IP names for debug
    if (GetNumSkippedCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_skipped_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"Ignored xbtest HW IPs (skipped): " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumCompanionCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_companion_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"Ignored xbtest HW IPs (companion): " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumVerifyCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_verify_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"Verify xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumPowerCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_pwr_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"Power xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumGtmacCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_gt_mac_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"GT MAC xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumGtlpbkCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_gt_lpbk_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"GT LPBK xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumGtPrbsCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_gt_prbs_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"GT PRBS xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumGtfPrbsCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_gtf_prbs_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"GTF PRBS xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumGtmPrbsCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_gtm_prbs_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"GTM PRBS xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    if (GetNumGtypPrbsCu() > 0)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : this->m_cl_gtyp_prbs_compute_units)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {"GTYP PRBS xbtest HW IPs: " + StrVectToStr(cu_names, ", ")});
    }
    for (const auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : name_cu.second)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {name_cu.first + " Memory xbtest HW IPs (" + SINGLE_CHANNEL + "): " + StrVectToStr(cu_names, ", ")});
    }
    for (const auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        cu_names.clear();
        for (const auto & cl_compute_unit : name_cu.second)
        {
            cu_names.emplace_back(cl_compute_unit.name);
        }
        LogMessage(MSG_DEBUG_SETUP, {name_cu.first + " Memory xbtest HW IPs (" + MULTI_CHANNEL + "): " + StrVectToStr(cu_names, ", ")});
    }

    //set for OCl (override) not for peekpoke (will always be true)
    if (InitCUArgs() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Configure the buffers for m00_axi interfaces
    this->cl_m00_axi_buffers.clear();
    if (ConfigureM00AxiBuffers(this->m_cl_verify_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_pwr_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_gt_mac_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_gt_lpbk_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_gt_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_gtf_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_gtm_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConfigureM00AxiBuffers(this->m_cl_gtyp_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        if (ConfigureM00AxiBuffers(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        if (ConfigureM00AxiBuffers(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }

    // Check the PLRAM size if enough for all xbtest HW IPs
    for (auto & cl_m00_axi_buffer : this->cl_m00_axi_buffers)
    {
        if (cl_m00_axi_buffer.available_size < cl_m00_axi_buffer.buffer_size)
        {
            LogMessage(MSG_ITF_120, {std::to_string(cl_m00_axi_buffer.available_size), std::to_string(cl_m00_axi_buffer.mem_data_idx), std::to_string(cl_m00_axi_buffer.buffer_size)});
            return RET_FAILURE;
        }
    }

    //create for OCl (override) not for peekpoke (will always be true)
    if (CreateAllM00AXIBuffers() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Set M00_AXI pointer argument
    if (SetCuM00AxiPtrArgument(this->m_cl_verify_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_pwr_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_gt_mac_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_gt_lpbk_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_gt_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_gtf_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_gtm_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (SetCuM00AxiPtrArgument(this->m_cl_gtyp_prbs_compute_units) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        if (SetCuM00AxiPtrArgument(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        if (SetCuM00AxiPtrArgument(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }

    //Check will do nothing for AMI and will call a function that will always return true
    //XRT will override
    if (ConfigurePlramShadowBuffers() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Setup MXX_AXI pointers arguments for all Memory xbtest HW IPs
    for (auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        if (CreateCuMxxAxiBuffers(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        if (CreateCuMxxAxiBuffers(name_cu.second) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }

    //Check xbtest HW IPs are not in deadlock, will return true as default for xrt, ami overrides
    if (CheckCUsNotDeadlocked() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Once the xbtest HW IPs are set-up, read their build info
    // Create M00_AXI sub-buffers
    for (uint cu_idx = 0; cu_idx < this->m_cl_verify_compute_units.size(); cu_idx++)
    {
        if (ReadVerifyCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_pwr_compute_units.size(); cu_idx++)
    {
        if (ReadPowerCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_gt_mac_compute_units.size(); cu_idx++)
    {
        if (ReadGtmacCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_gt_lpbk_compute_units.size(); cu_idx++)
    {
        if (ReadGTLpbkCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_gt_prbs_compute_units.size(); cu_idx++)
    {
        if (ReadGTPrbsCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_gtf_prbs_compute_units.size(); cu_idx++)
    {
        if (ReadGTFPrbsCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_gtm_prbs_compute_units.size(); cu_idx++)
    {
        if (ReadGTMPrbsCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (uint cu_idx = 0; cu_idx < this->m_cl_gtyp_prbs_compute_units.size(); cu_idx++)
    {
        if (ReadGTYPPrbsCuBI(cu_idx) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    for (const auto & name_cu : this->m_cl_mem_sc_compute_units)
    {
        for (uint cu_idx = 0; cu_idx < name_cu.second.size(); cu_idx++)
        {
            if (ReadMemoryCuBI(SINGLE_CHANNEL, name_cu.first, cu_idx) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
        }
    }
    for (const auto & name_cu : this->m_cl_mem_mc_compute_units)
    {
        for (uint cu_idx = 0; cu_idx < name_cu.second.size(); cu_idx++)
        {
            if (ReadMemoryCuBI(MULTI_CHANNEL, name_cu.first, cu_idx) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
        }
    }

    if (SetupPsComputeUnits() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    LogMessage(MSG_DEBUG_SETUP, {"Done setting up xbtest HW IP"});
    return RET_SUCCESS;
}

uint DeviceInterface_Base::GetNumSkippedCu()
{
    return this->m_cl_skipped_compute_units.size();
}
uint DeviceInterface_Base::GetNumCompanionCu()
{
    return this->m_cl_companion_compute_units.size();
}
uint DeviceInterface_Base::GetNumVerifyCu()
{
    return this->m_cl_verify_compute_units.size();
}
uint DeviceInterface_Base::GetNumPowerCu()
{
    return this->m_cl_pwr_compute_units.size();
}
uint DeviceInterface_Base::GetNumGtmacCu()
{
    return this->m_cl_gt_mac_compute_units.size();
}
uint DeviceInterface_Base::GetNumGtlpbkCu()
{
    return this->m_cl_gt_lpbk_compute_units.size();
}
uint DeviceInterface_Base::GetNumGtPrbsCu()
{
    return this->m_cl_gt_prbs_compute_units.size();
}
uint DeviceInterface_Base::GetNumGtfPrbsCu()
{
    return this->m_cl_gtf_prbs_compute_units.size();
}
uint DeviceInterface_Base::GetNumGtmPrbsCu()
{
    return this->m_cl_gtm_prbs_compute_units.size();
}
uint DeviceInterface_Base::GetNumGtypPrbsCu()
{
    return this->m_cl_gtyp_prbs_compute_units.size();
}
uint DeviceInterface_Base::GetNumUnknownCu()
{
    return this->m_cl_unknown_compute_units.size();
}
uint DeviceInterface_Base::GetNumMemoryCu( const  std::string & memory_type, const std::string & memory_name )
{
    if (memory_type == SINGLE_CHANNEL)
    {
        return GetNumMemoryScCu(memory_name);
    }
    return GetNumMemoryMcCu(memory_name);
}

std::string DeviceInterface_Base::GetSkippedCuName( const uint & cu_idx )
{
    return this->m_cl_skipped_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetCompanionCuName( const uint & cu_idx )
{
    return this->m_cl_companion_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetVerifyCuName( const uint & cu_idx )
{
    return this->m_cl_verify_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetPwrCuName( const uint & cu_idx )
{
    return this->m_cl_pwr_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetGTMACCuName( const uint & cu_idx )
{
    return this->m_cl_gt_mac_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetGTLpbkCuName( const uint & cu_idx )
{
    return this->m_cl_gt_lpbk_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetGTPrbsCuName( const uint & cu_idx )
{
    return this->m_cl_gt_prbs_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetGTFPrbsCuName( const uint & cu_idx )
{
    return this->m_cl_gtf_prbs_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetGTMPrbsCuName( const uint & cu_idx )
{
    return this->m_cl_gtm_prbs_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetGTYPPrbsCuName( const uint & cu_idx )
{
    return this->m_cl_gtyp_prbs_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetUnknownCuName( const uint & cu_idx )
{
    return this->m_cl_unknown_compute_units[cu_idx].name;
}
std::string DeviceInterface_Base::GetMemoryCuName( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx )
{
    if (memory_type == SINGLE_CHANNEL)
    {
        return GetMemSCCuName(memory_name, cu_idx);
    }
    return GetMemMCCuName(memory_name, cu_idx);
}
uint DeviceInterface_Base::GetVerifyCuDnaRead( const uint & cu_idx )
{
    return this->m_cl_verify_compute_units[cu_idx].dna_read;
}
uint DeviceInterface_Base::GetPowerCuSlrIndex( const uint & cu_idx )
{
    return this->m_cl_pwr_compute_units[cu_idx].slr;
}
uint DeviceInterface_Base::GetGtmacCuGTIndex( const uint & cu_idx )
{
    return this->m_cl_gt_mac_compute_units[cu_idx].gt_index;
}
uint DeviceInterface_Base::GetGtlpbkCuGTIndex( const uint & cu_idx )
{
    return this->m_cl_gt_lpbk_compute_units[cu_idx].gt_index;
}
uint DeviceInterface_Base::GetGtPrbsCuGTIndex( const uint & cu_idx )
{
    return this->m_cl_gt_prbs_compute_units[cu_idx].gt_index;
}
std::vector<uint> DeviceInterface_Base::GetGtfPrbsCuGTFIndexes( const uint & cu_idx )
{
    return this->m_cl_gtf_prbs_compute_units[cu_idx].gtf_indexes;
}
std::vector<uint> DeviceInterface_Base::GetGtmPrbsCuGTMIndexes( const uint & cu_idx )
{
    return this->m_cl_gtm_prbs_compute_units[cu_idx].gtm_indexes;
}
std::vector<uint> DeviceInterface_Base::GetGtypPrbsCuGTYPIndexes( const uint & cu_idx )
{
    return this->m_cl_gtyp_prbs_compute_units[cu_idx].gtyp_indexes;
}

bool DeviceInterface_Base::ReadVerifyCu( const uint & cu_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_verify_compute_units[cu_idx], address, read_data);
}
bool DeviceInterface_Base::ReadPowerCu( const uint & cu_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_pwr_compute_units[cu_idx], address, read_data);
}
bool DeviceInterface_Base::ReadGtmacCu( const uint & cu_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_gt_mac_compute_units[cu_idx], address, read_data);
}
bool DeviceInterface_Base::ReadGtlpbkCu( const uint & cu_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_gt_lpbk_compute_units[cu_idx], address, read_data);
}
bool DeviceInterface_Base::ReadGtPrbsCu( const uint & cu_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_gt_prbs_compute_units[cu_idx], address, read_data);
}
bool DeviceInterface_Base::ReadGtfPrbsCu( const uint & cu_idx, const int & gt_idx, const uint & address, uint & read_data )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtf_prbs_compute_units[cu_idx].cu_mtx));
    return ReadComputeUnit(this->m_cl_gtf_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx) | address, read_data);
}
bool DeviceInterface_Base::ReadGtmPrbsCu( const uint & cu_idx, const int & gt_idx, const uint & address, uint & read_data )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtm_prbs_compute_units[cu_idx].cu_mtx));
    return ReadComputeUnit(this->m_cl_gtm_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx) | address, read_data);
}
bool DeviceInterface_Base::ReadGtypPrbsCu( const uint & cu_idx, const int & gt_idx, const uint & address, uint & read_data )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtyp_prbs_compute_units[cu_idx].cu_mtx));
    return ReadComputeUnit(this->m_cl_gtyp_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx) | address, read_data);
}

bool DeviceInterface_Base::ReadGtfPrbsCuStatus( const uint & cu_idx, const int & gt_idx, uint32_t * read_buffer, const uint64_t & size )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtf_prbs_compute_units[cu_idx].cu_mtx));
    return ReadMultiGtStatus(this->m_cl_gtf_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx), read_buffer, size);
}
bool DeviceInterface_Base::ReadGtmPrbsCuStatus( const uint & cu_idx, const int & gt_idx, uint32_t * read_buffer, const uint64_t & size )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtm_prbs_compute_units[cu_idx].cu_mtx));
    return ReadMultiGtStatus(this->m_cl_gtm_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx), read_buffer, size);
}
bool DeviceInterface_Base::ReadGtypPrbsCuStatus( const uint & cu_idx, const int & gt_idx, uint32_t * read_buffer, const uint64_t & size )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtyp_prbs_compute_units[cu_idx].cu_mtx));
    return ReadMultiGtStatus(this->m_cl_gtyp_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx), read_buffer, size);
}

bool DeviceInterface_Base::ReadMemoryCu( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, uint & read_data )
{
    if (memory_type == SINGLE_CHANNEL)
    {
        return ReadMemoryScCu(memory_name, cu_idx, ch_idx, address, read_data);
    }
    return ReadMemoryMcCu(memory_name, cu_idx, ch_idx, address, read_data);
}

bool DeviceInterface_Base::ReadMemoryCuStatus( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, uint32_t * read_buffer, const uint64_t & size )
{
    if (memory_type == SINGLE_CHANNEL)
    {
        return ReadMemoryScCuStatus(memory_name, cu_idx, read_buffer, size);
    }
    return ReadMemoryMcCuStatus(memory_name, cu_idx, read_buffer, size);
}

bool DeviceInterface_Base::WriteVerifyCu( const uint & cu_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_verify_compute_units[cu_idx], address, value);
}
bool DeviceInterface_Base::WritePowerCu( const uint & cu_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_pwr_compute_units[cu_idx], address, value);
}
bool DeviceInterface_Base::WriteGtmacCu( const uint & cu_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_gt_mac_compute_units[cu_idx], address, value);
}
bool DeviceInterface_Base::WriteGtlpbkCu( const uint & cu_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_gt_lpbk_compute_units[cu_idx], address, value);
}
bool DeviceInterface_Base::WriteGtPrbsCu( const uint & cu_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_gt_prbs_compute_units[cu_idx], address, value);
}
bool DeviceInterface_Base::WriteGtfPrbsCu( const uint & cu_idx, const int & gt_idx, const uint & address, const uint & value )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtf_prbs_compute_units[cu_idx].cu_mtx));
    return WriteComputeUnit(this->m_cl_gtf_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx) | address, value);
}
bool DeviceInterface_Base::WriteGtmPrbsCu( const uint & cu_idx, const int & gt_idx, const uint & address, const uint & value )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtm_prbs_compute_units[cu_idx].cu_mtx));
    return WriteComputeUnit(this->m_cl_gtm_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx) | address, value);
}
bool DeviceInterface_Base::WriteGtypPrbsCu( const uint & cu_idx, const int & gt_idx, const uint & address, const uint & value )
{
    std::lock_guard<std::mutex> guard(*(this->m_cl_gtyp_prbs_compute_units[cu_idx].cu_mtx));
    return WriteComputeUnit(this->m_cl_gtyp_prbs_compute_units[cu_idx], GetMultiGtCuAddrOffset(gt_idx) | address, value);
}
bool DeviceInterface_Base::WriteMemoryCu( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint &address, const uint & value )
{
    if (memory_type == SINGLE_CHANNEL)
    {
        return WriteMemoryScCu(memory_name, cu_idx, ch_idx, address, value);
    }
    return WriteMemoryMcCu(memory_name, cu_idx, ch_idx, address, value);
}

Build_Info DeviceInterface_Base::GetVerifyCuBI( const uint & cu_idx )
{
    return this->m_cl_verify_compute_units[cu_idx].build_info;
}
Build_Info DeviceInterface_Base::GetPowerCuBI( const uint & cu_idx )
{
    return this->m_cl_pwr_compute_units[cu_idx].build_info;
}
Build_Info DeviceInterface_Base::GetGtmacCuBI( const uint & cu_idx )
{
    return this->m_cl_gt_mac_compute_units[cu_idx].build_info;
}
Build_Info DeviceInterface_Base::GetGtlpbkCuBI( const uint & cu_idx )
{
    return this->m_cl_gt_lpbk_compute_units[cu_idx].build_info;
}
Build_Info DeviceInterface_Base::GetGtPrbsCuBI( const uint & cu_idx )
{
    return this->m_cl_gt_prbs_compute_units[cu_idx].build_info;
}
Build_Info DeviceInterface_Base::GetGtfPrbsCuBI( const uint & cu_idx, const int & gt_idx )
{
    if (gt_idx == CU_WRAPPER_IDX)
    {
        return this->m_cl_gtf_prbs_compute_units[cu_idx].build_info;
    }
    return this->m_cl_gtf_prbs_compute_units[cu_idx].ch_build_info[gt_idx];

}
Build_Info DeviceInterface_Base::GetGtmPrbsCuBI( const uint & cu_idx, const int & gt_idx )
{
    if (gt_idx == CU_WRAPPER_IDX)
    {
        return this->m_cl_gtm_prbs_compute_units[cu_idx].build_info;
    }
    return this->m_cl_gtm_prbs_compute_units[cu_idx].ch_build_info[gt_idx];

}
Build_Info DeviceInterface_Base::GetGtypPrbsCuBI( const uint & cu_idx, const int & gt_idx )
{
    if (gt_idx == CU_WRAPPER_IDX)
    {
        return this->m_cl_gtyp_prbs_compute_units[cu_idx].build_info;
    }
    return this->m_cl_gtyp_prbs_compute_units[cu_idx].ch_build_info[gt_idx];

}
Build_Info DeviceInterface_Base::GetMemoryCuBI( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx )
{
    if (memory_type == SINGLE_CHANNEL)
    {
        return GetMemoryScCuBI(memory_name, cu_idx, ch_idx);
    }
    return GetMemoryMcCuBI(memory_name, cu_idx, ch_idx);
}

void DeviceInterface_Base::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    if (this->m_xbtest_sw_config->GetCommandLineP2pTargetBdf().exists)
    {
        this->m_log->LogMessage(this->m_log_header, this->m_device_bdf, "", message, arg_list);
    }
    else
    {
        this->m_log->LogMessage(this->m_log_header, "", "", message, arg_list);
    }
}
void DeviceInterface_Base::LogMessage ( const Message_t & message )
{
    if (this->m_xbtest_sw_config->GetCommandLineP2pTargetBdf().exists)
    {
        this->m_log->LogMessage(this->m_log_header, this->m_device_bdf, "", message);
    }
    else
    {
        this->m_log->LogMessage(this->m_log_header, "", "", message);
    }
}

/*
bool DeviceInterface_Base::CreateCuMxxAxiBuffersNoOffset( std::vector<Cl_Compute_Unit_t> & cl_compute_units )
{
    // function should be called only for Memory xbtest HW IPs
    auto cl_err     = CL_SUCCESS;
    auto chk_cl_err = CHK_CL_ERR_SUCCESS;

    for (auto & cl_compute_unit : cl_compute_units)
    {
        for (uint ch_idx = 0; ch_idx < this->m_xbtest_sw_config->GetMemCUNumChannels(cl_compute_unit.name); ch_idx++)
        {
            // Create one buffer per Memory xbtest HW IP channel, not used in HW or SW but needed to execute the xbtest HW IP
            // Base address for each channel of the Memory xbtest HW IP is computed based on device info and metadata and is written by SW to Memory xbtest HW IP register during memory TC

            auto cu_connection_mem_tags         = this->m_xbtest_sw_config->GetCUConnectionMemTags(cl_compute_unit.name, ch_idx);
            auto cu_connection_mem_data_indexes = this->m_xbtest_sw_config->GetCUConnectionMemDataIndexes(cl_compute_unit.name, ch_idx);

            auto mem_tag        = cu_connection_mem_tags[0];
            auto mem_data_index = cu_connection_mem_data_indexes[0];

            auto buff_info = "buffer for channel (" + std::to_string(ch_idx) + ") of Memory xbtest HW IP (" + cl_compute_unit.name + "): memory data index: " + std::to_string(mem_data_index) + " / memory tag " + mem_tag;
            LogMessage(MSG_DEBUG_SETUP, {"Creating " + buff_info});

            // Create buffer in the first tag to be used by the xbtest HW IP
            cl_mem_ext_ptr_t cl_mem_ext_ptr;
            cl_mem_ext_ptr.param    = nullptr;
            cl_mem_ext_ptr.obj      = nullptr;
            cl_mem_ext_ptr.flags    = ((unsigned)mem_data_index) | XCL_MEM_TOPOLOGY;
            auto mem_flags_str = std::to_string(mem_data_index) + " | XCL_MEM_TOPOLOGY";

            // CL_MEM_READ_WRITE: This flag specifies that the memory object will be read and written by a xbtest HW IP. This is the default.
            auto mem_flags = CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX;
            mem_flags_str += "CL_MEM_READ_WRITE | CL_MEM_EXT_PTR_XILINX";
            if (m_xbtest_sw_config->GetMemCUTarget(cl_compute_unit.name) == HOST)
            {
                cl_mem_ext_ptr.flags |= XCL_MEM_EXT_HOST_ONLY ;
                mem_flags_str += " | XCL_MEM_EXT_HOST_ONLY";
            }
            else
            {
                // CL_MEM_HOST_NO_ACCESS: This flag specifies that the host will not read or write the memory object.
                mem_flags |= CL_MEM_HOST_NO_ACCESS;
                mem_flags_str += " | CL_MEM_HOST_NO_ACCESS";
            }

            auto mem_cu_buffer = cl::Buffer(
                this->m_cl_context,
                static_cast<cl_mem_flags>(mem_flags),
                M_AXI_BASE_SIZE_BYTES,
                &cl_mem_ext_ptr,
                &cl_err
            );
            CheckClBufferConstructor(cl_err, buff_info, mem_flags_str, chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);

            cl_err = cl_compute_unit.cl_kernel.setArg(5+ch_idx, mem_cu_buffer);
            CheckClKernelSetArg(cl_err, cl_compute_unit.name, std::to_string(5+ch_idx), chk_cl_err); CHK_CL_ERR_RETURN(chk_cl_err);
        }
    }
    return RET_SUCCESS;
}
*/

uint DeviceInterface_Base::GetNumMemoryScCu( const std::string & memory_name )
{
    return this->m_cl_mem_sc_compute_units[memory_name].size();
}
uint DeviceInterface_Base::GetNumMemoryMcCu( const std::string & memory_name )
{
    return this->m_cl_mem_mc_compute_units[memory_name].size();
}

std::string DeviceInterface_Base::GetMemSCCuName( const std::string & memory_name, const uint & cu_idx )
{
    return this->m_cl_mem_sc_compute_units[memory_name][cu_idx].name;
}
std::string DeviceInterface_Base::GetMemMCCuName( const std::string & memory_name, const uint & cu_idx )
{
    return this->m_cl_mem_mc_compute_units[memory_name][cu_idx].name;
}

Build_Info DeviceInterface_Base::GetMemoryScCuBI( const std::string & memory_name, const uint & cu_idx, const int & ch_idx  )
{
    if (ch_idx == CU_WRAPPER_IDX)
    {
        return this->m_cl_mem_sc_compute_units[memory_name][cu_idx].build_info;
    }
    return this->m_cl_mem_sc_compute_units[memory_name][cu_idx].ch_build_info[ch_idx];
}
Build_Info DeviceInterface_Base::GetMemoryMcCuBI( const std::string & memory_name, const uint & cu_idx, const int & ch_idx  )
{
    if (ch_idx == CU_WRAPPER_IDX)
    {
        return this->m_cl_mem_mc_compute_units[memory_name][cu_idx].build_info;
    }
    return this->m_cl_mem_mc_compute_units[memory_name][cu_idx].ch_build_info[ch_idx];
}

uint DeviceInterface_Base::GetMemoryCuAddrOffset( const int & ch_idx )
{
    // b27 = 0 => Wrapper level (build info, ctrl status)
    //     = 1 => Core level (build info, ctrl status, registers)
    // b26:b21 core idx selection: 64 channels supported

    uint addr_offset = 0;
    if (ch_idx != CU_WRAPPER_IDX)
    {
        addr_offset = 0x8000000;
        addr_offset |= (((uint)ch_idx & 0x3F) << 21);
    }
    return addr_offset;
}

uint DeviceInterface_Base::GetMultiGtCuAddrOffset( const int & gt_idx )
{
    // b27 = 0 => Wrapper level (build info, ctrl status)
    //     = 1 => Core level (build info, ctrl status, registers)
    // b26:b22 core idx selection: [0x00,0x1F] <=> [0x0000000,0x7C00000]

    uint addr_offset = 0;
    if (gt_idx != CU_WRAPPER_IDX)
    {
        addr_offset = 0x8000000;
        addr_offset |= (((uint)gt_idx & 0x1F) << 22);
    }
    return addr_offset;
}

bool DeviceInterface_Base::ReadMemoryScCu( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_mem_sc_compute_units[memory_name][cu_idx], GetMemoryCuAddrOffset(ch_idx) | address, read_data);
}
bool DeviceInterface_Base::ReadMemoryMcCu( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, uint & read_data )
{
    return ReadComputeUnit(this->m_cl_mem_mc_compute_units[memory_name][cu_idx], GetMemoryCuAddrOffset(ch_idx) | address, read_data);
}

bool DeviceInterface_Base::ReadMemoryScCuStatus( const std::string & memory_name, const uint & cu_idx, uint32_t * read_buffer, const uint64_t & size )
{
    return ReadMemoryCuStatusInner(this->m_cl_mem_sc_compute_units[memory_name][cu_idx], read_buffer, size);
}
bool DeviceInterface_Base::ReadMemoryMcCuStatus( const std::string & memory_name, const uint & cu_idx, uint32_t * read_buffer, const uint64_t & size )
{
    return ReadMemoryCuStatusInner(this->m_cl_mem_mc_compute_units[memory_name][cu_idx], read_buffer, size);
}

bool DeviceInterface_Base::WriteMemoryScCu( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_mem_sc_compute_units[memory_name][cu_idx], GetMemoryCuAddrOffset(ch_idx) | address, value);
}
bool DeviceInterface_Base::WriteMemoryMcCu( const std::string & memory_name, const uint & cu_idx, const int & ch_idx, const uint & address, const uint & value )
{
    return WriteComputeUnit(this->m_cl_mem_mc_compute_units[memory_name][cu_idx], GetMemoryCuAddrOffset(ch_idx) | address, value);
}

bool DeviceInterface_Base::ReadVerifyCuBI( const uint & cu_idx )
{
    uint read_data;
    this->m_cl_verify_compute_units[cu_idx].build_info.cu_name = this->m_cl_verify_compute_units[cu_idx].name;

    if (ReadVerifyCu(cu_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_verify_compute_units[cu_idx].build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    this->m_cl_verify_compute_units[cu_idx].build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadVerifyCu(cu_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_verify_compute_units[cu_idx].build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadVerifyCu(cu_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_verify_compute_units[cu_idx].build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadVerifyCu(cu_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_verify_compute_units[cu_idx].build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadVerifyCu(cu_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_verify_compute_units[cu_idx].build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    this->m_cl_verify_compute_units[cu_idx].build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadVerifyCu(cu_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_verify_compute_units[cu_idx].build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintVerifyCuBI(this->m_cl_verify_compute_units[cu_idx].build_info);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadPowerCuBI( const uint & cu_idx )
{
    uint read_data;
    this->m_cl_pwr_compute_units[cu_idx].build_info.cu_name = this->m_cl_pwr_compute_units[cu_idx].name;

    if (ReadPowerCu(cu_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    this->m_cl_pwr_compute_units[cu_idx].build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadPowerCu(cu_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadPowerCu(cu_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadPowerCu(cu_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadPowerCu(cu_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // (read_data >> 0)  & 0x0000FFFF;   // Info 1 reserved for future use
    this->m_cl_pwr_compute_units[cu_idx].build_info.slr             = (read_data >> 20) & 0x0000000F;   // Info 2
    this->m_cl_pwr_compute_units[cu_idx].build_info.throttle_mode   = (read_data >> 16) & 0x0000000F;   // Info 2
    this->m_cl_pwr_compute_units[cu_idx].build_info.use_aie         = (read_data >> 28) & 0x0000000F;   // Info 2

    if (ReadPowerCu(cu_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.num_slice         = (read_data >> 0)  & 0x0000FFFF;   // Info 3
    this->m_cl_pwr_compute_units[cu_idx].build_info.num_dsp     = (read_data >> 16) & 0x0000FFFF;   // Info 4

    if (ReadPowerCu(cu_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.num_bram      = (read_data >> 0)  & 0x0000FFFF;   // Info 5
    this->m_cl_pwr_compute_units[cu_idx].build_info.num_uram     = (read_data >> 16) & 0x0000FFFF;   // Info 6

    if (ReadPowerCu(cu_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    this->m_cl_pwr_compute_units[cu_idx].build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadPowerCu(cu_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_pwr_compute_units[cu_idx].build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintPowerCuBI(this->m_cl_pwr_compute_units[cu_idx].build_info);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadGtmacCuBI( const uint & cu_idx )
{
    uint read_data;
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.cu_name = this->m_cl_gt_mac_compute_units[cu_idx].name;

    if (ReadGtmacCu(cu_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.major_version    = (read_data >> 16) & 0x0000FFFF;
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.minor_version    = (read_data >> 0)  & 0x0000FFFF;

    if (ReadGtmacCu(cu_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.build_version    = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtmacCu(cu_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.component_id     = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtmacCu(cu_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.scratch_pad      = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtmacCu(cu_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // (read_data >> 0)  & 0x0000FFFF;   // Info 1 reserved for future use
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.gt_index         = (read_data >> 24) & 0x000000FF;  // Info 2

    if (ReadGtmacCu(cu_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.gt_type          = (read_data >> 0)  & 0x0000003;
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.gt_num_gt        = (read_data >> 2)  & 0x0000003;
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.gt_num_lane      = (read_data >> 4)  & 0x0000007;
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.gt_rate          = (read_data >> 7)  & 0x0000003;
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.gt_rsfec_en      = (read_data >> 9)  & 0x0000001;

    // if (ReadGtmacCu(cu_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }

    if (ReadGtmacCu(cu_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.clock0_freq      = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.clock1_freq      = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadGtmacCu(cu_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_mac_compute_units[cu_idx].build_info.rst_detection    = (read_data >> 0)  & 0x00000003;

    PrintGtmacCuBI(this->m_cl_gt_mac_compute_units[cu_idx].build_info);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadGTLpbkCuBI( const uint & cu_idx )
{
    uint read_data;
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.cu_name     = this->m_cl_gt_lpbk_compute_units[cu_idx].name;

    if (ReadGtlpbkCu(cu_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadGtlpbkCu(cu_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtlpbkCu(cu_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtlpbkCu(cu_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtlpbkCu(cu_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // (read_data >> 0)  & 0x0000FFFF;   // Info 1 reserved for future use
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.gt_index        = (read_data >> 24) & 0x000000FF;  // Info 2

    if (ReadGtlpbkCu(cu_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.gt_type         = (read_data >> 0)  & 0x0000003;
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.gt_num_gt       = (read_data >> 2)  & 0x0000003;
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.gt_num_lane     = (read_data >> 4)  & 0x0000007;
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.gt_rate         = (read_data >> 7)  & 0x0000003;

    // if (ReadGtlpbkCu(cu_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }

    if (ReadGtlpbkCu(cu_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadGtlpbkCu(cu_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_lpbk_compute_units[cu_idx].build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintGTLpbkCuBI(this->m_cl_gt_lpbk_compute_units[cu_idx].build_info);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadGTPrbsCuBI( const uint & cu_idx )
{
    uint read_data;
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.cu_name     = this->m_cl_gt_prbs_compute_units[cu_idx].name;

    if (ReadGtPrbsCu(cu_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadGtPrbsCu(cu_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtPrbsCu(cu_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtPrbsCu(cu_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadGtPrbsCu(cu_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // (read_data >> 0)  & 0x0000FFFF;   // Info 1 reserved for future use
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.gt_index        = (read_data >> 24) & 0x000000FF;  // Info 2

    if (ReadGtPrbsCu(cu_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.gt_type         = (read_data >> 0)  & 0x0000003;
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.gt_num_gt       = (read_data >> 2)  & 0x0000003;
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.gt_rate         = (read_data >> 7)  & 0x0000003;

    // if (ReadGtPrbsCu(cu_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }

    if (ReadGtPrbsCu(cu_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadGtPrbsCu(cu_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    this->m_cl_gt_prbs_compute_units[cu_idx].build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintGTPrbsCuBI(this->m_cl_gt_prbs_compute_units[cu_idx].build_info);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadGTFPrbsCuBI( const uint & cu_idx )
{
    if (ReadGTFPrbsCuInternBI(cu_idx, CU_WRAPPER_IDX, this->m_cl_gtf_prbs_compute_units[cu_idx].build_info) == RET_FAILURE) { return RET_FAILURE; }

    this->m_cl_gtf_prbs_compute_units[cu_idx].ch_build_info.clear();
    for (const auto & gt_idx : this->m_cl_gtf_prbs_compute_units[cu_idx].gtf_indexes)
    {
        Build_Info build_info;
        if (ReadGTFPrbsCuInternBI(cu_idx, gt_idx, build_info) == RET_FAILURE) { return RET_FAILURE; }
        this->m_cl_gtf_prbs_compute_units[cu_idx].ch_build_info.emplace_back(build_info);
    }
    return RET_SUCCESS;
}
bool DeviceInterface_Base::ReadGTFPrbsCuInternBI( const uint & cu_idx, const int & gt_idx, Build_Info & build_info )
{
    uint read_data;
    build_info.cu_name     = this->m_cl_gtf_prbs_compute_units[cu_idx].name;
    // LogMessage(MSG_DEBUG_SETUP, {"xbtest HW IP name = " + build_info.cu_name});
    // LogMessage(MSG_DEBUG_SETUP, {"gt_idx = " + std::to_string(gt_idx)});

    if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_MAJOR_MINOR_VERSION_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"BI_BUILD_VERSION_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"BI_COMPONENT_ID_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtfPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"CMN_SCRATCH_PAD_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_1_2_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.gt_index        = (read_data >> 24) & 0x000000FF;
    build_info.gt_enable       = (read_data >> 21) & 0x0000001;
    build_info.gt_rate         = (read_data >> 18) & 0x0000007;
    build_info.gt_ip_sel       = (read_data >> 16) & 0x0000003;

    if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_3_4_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.gt_num_gt       = (read_data >> 24)  & 0x0000FF;
    build_info.gt_num_lane     = (read_data >> 16)  & 0x0000FF;

    // if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_5_6_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtfPrbsCu(cu_idx, gt_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_7_8_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadGtfPrbsCu(cu_idx, gt_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"CMN_RESET_DETECTION_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintGTFPrbsCuBI(build_info, gt_idx);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadGTMPrbsCuBI( const uint & cu_idx )
{
    if (ReadGTMPrbsCuInternBI(cu_idx, CU_WRAPPER_IDX, this->m_cl_gtm_prbs_compute_units[cu_idx].build_info) == RET_FAILURE) { return RET_FAILURE; }

    this->m_cl_gtm_prbs_compute_units[cu_idx].ch_build_info.clear();
    for (const auto & gt_idx : this->m_cl_gtm_prbs_compute_units[cu_idx].gtm_indexes)
    {
        Build_Info build_info;
        if (ReadGTMPrbsCuInternBI(cu_idx, gt_idx, build_info) == RET_FAILURE) { return RET_FAILURE; }
        this->m_cl_gtm_prbs_compute_units[cu_idx].ch_build_info.emplace_back(build_info);
    }
    return RET_SUCCESS;
}
bool DeviceInterface_Base::ReadGTMPrbsCuInternBI( const uint & cu_idx, const int & gt_idx, Build_Info & build_info )
{
    uint read_data;
    build_info.cu_name     = this->m_cl_gtm_prbs_compute_units[cu_idx].name;
    // LogMessage(MSG_DEBUG_SETUP, {"xbtest HW IP name = " + build_info.cu_name});
    // LogMessage(MSG_DEBUG_SETUP, {"gt_idx = " + std::to_string(gt_idx)});

    if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_MAJOR_MINOR_VERSION_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"BI_BUILD_VERSION_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"BI_COMPONENT_ID_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtmPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"CMN_SCRATCH_PAD_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_1_2_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.gt_index        = (read_data >> 24) & 0x000000FF;
    build_info.gt_enable       = (read_data >> 21) & 0x0000001;
    build_info.gt_rate         = (read_data >> 18) & 0x0000007;
    build_info.gt_ip_sel       = (read_data >> 16) & 0x0000003;

    if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_3_4_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.gt_num_gt       = (read_data >> 24)  & 0x0000FF;
    build_info.gt_num_lane     = (read_data >> 16)  & 0x0000FF;

    // if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_5_6_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtmPrbsCu(cu_idx, gt_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_7_8_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadGtmPrbsCu(cu_idx, gt_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"CMN_RESET_DETECTION_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintGTMPrbsCuBI(build_info, gt_idx);
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadGTYPPrbsCuBI( const uint & cu_idx )
{
    if (ReadGTYPPrbsCuInternBI(cu_idx, CU_WRAPPER_IDX, this->m_cl_gtyp_prbs_compute_units[cu_idx].build_info) == RET_FAILURE) { return RET_FAILURE; }

    this->m_cl_gtyp_prbs_compute_units[cu_idx].ch_build_info.clear();
    for (const auto & gt_idx : this->m_cl_gtyp_prbs_compute_units[cu_idx].gtyp_indexes)
    {
        Build_Info build_info;
        if (ReadGTYPPrbsCuInternBI(cu_idx, gt_idx, build_info) == RET_FAILURE) { return RET_FAILURE; }
        this->m_cl_gtyp_prbs_compute_units[cu_idx].ch_build_info.emplace_back(build_info);
    }
    return RET_SUCCESS;
}
bool DeviceInterface_Base::ReadGTYPPrbsCuInternBI( const uint & cu_idx, const int & gt_idx, Build_Info & build_info )
{
    uint read_data;
    build_info.cu_name     = this->m_cl_gtyp_prbs_compute_units[cu_idx].name;
    // LogMessage(MSG_DEBUG_SETUP, {"xbtest HW IP name = " + build_info.cu_name});
    // LogMessage(MSG_DEBUG_SETUP, {"gt_idx = " + std::to_string(gt_idx)});

    if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_MAJOR_MINOR_VERSION_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.major_version   = (read_data >> 16) & 0x0000FFFF;
    build_info.minor_version   = (read_data >> 0)  & 0x0000FFFF;

    if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.build_version   = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"BI_BUILD_VERSION_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.component_id    = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"BI_COMPONENT_ID_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtypPrbsCu(cu_idx, gt_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.scratch_pad     = (read_data >> 0)  & 0xFFFFFFFF;
    // LogMessage(MSG_DEBUG_SETUP, {"CMN_SCRATCH_PAD_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_1_2_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.gt_index        = (read_data >> 24) & 0x000000FF;
    build_info.gt_enable       = (read_data >> 21) & 0x0000001;
    build_info.gt_rate         = (read_data >> 18) & 0x0000007;
    build_info.gt_ip_sel       = (read_data >> 16) & 0x0000003;

    if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_3_4_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.gt_num_gt       = (read_data >> 24)  & 0x0000FF;
    build_info.gt_num_lane     = (read_data >> 16)  & 0x0000FF;

    // if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_5_6_ADDR read_data = " + NumToStrHex(read_data)});

    if (ReadGtypPrbsCu(cu_idx, gt_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"BI_INFO_7_8_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.clock0_freq     = (read_data >> 0)  & 0x0000FFFF;   // Info 7
    build_info.clock1_freq     = (read_data >> 16) & 0x0000FFFF;   // Info 8

    if (ReadGtypPrbsCu(cu_idx, gt_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    // LogMessage(MSG_DEBUG_SETUP, {"CMN_RESET_DETECTION_ADDR read_data = " + NumToStrHex(read_data)});
    build_info.rst_detection   = (read_data >> 0)  & 0x00000003;

    PrintGTYPPrbsCuBI(build_info, gt_idx);
    return RET_SUCCESS;
}


bool DeviceInterface_Base::ReadMemoryCuBI( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx )
{
    Cl_Compute_Unit_t * cl_compute_unit;

    if (memory_type == SINGLE_CHANNEL)
    {
        cl_compute_unit = &(this->m_cl_mem_sc_compute_units[memory_name][cu_idx]);
    }
    else
    {
        cl_compute_unit = &(this->m_cl_mem_mc_compute_units[memory_name][cu_idx]);
    }

    if (ReadMemoryCuInternBI(memory_type, memory_name, cu_idx, CU_WRAPPER_IDX, cl_compute_unit->build_info) == RET_FAILURE) { return RET_FAILURE; }

    cl_compute_unit->ch_build_info.clear();
    for (uint ch_idx = 0; ch_idx < this->m_xbtest_sw_config->GetMemCUNumChannels(cl_compute_unit->name); ch_idx++)
    {
        Build_Info build_info;
        if (ReadMemoryCuInternBI(memory_type, memory_name, cu_idx, ch_idx, build_info) == RET_FAILURE) { return RET_FAILURE; }
        cl_compute_unit->ch_build_info.emplace_back(build_info);
    }
    return RET_SUCCESS;
}

bool DeviceInterface_Base::ReadMemoryCuInternBI( const std::string & memory_type, const std::string & memory_name, const uint & cu_idx, const int & ch_idx, Build_Info & build_info )
{
    uint read_data;
    build_info.cu_name = GetMemoryCuName(memory_type, memory_name, cu_idx);

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_MAJOR_MINOR_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.major_version               = (read_data >> 16) & 0x0000FFFF;
    build_info.minor_version               = (read_data >> 0)  & 0x0000FFFF;

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_BUILD_VERSION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.build_version               = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_COMPONENT_ID_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.component_id                = (read_data >> 0)  & 0xFFFFFFFF;

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, CMN_SCRATCH_PAD_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.scratch_pad                 = (read_data >> 0)  & 0xFFFFFFFF;

    // if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_INFO_1_2_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; });

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_INFO_3_4_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.mem_axi_datasize_divider     = (read_data >> 0)  & 0x0000000F;
    build_info.mem_num_channels             = (read_data >> 16) & 0x000000FF;

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_INFO_5_6_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.mem_type = (read_data >> 0) & 0x00000003;
    if (ch_idx != CU_WRAPPER_IDX)
    {
        build_info.mem_ch_idx     = (read_data >> 16) & 0x000000FF;
    }

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, BI_INFO_7_8_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.clock0_freq                 = (read_data >> 0)  & 0x0000FFFF;
    build_info.clock1_freq                 = (read_data >> 16) & 0x0000FFFF;

    if (ReadMemoryCu(memory_type, memory_name, cu_idx, ch_idx, CMN_RESET_DETECTION_ADDR, read_data) == RET_FAILURE) { return RET_FAILURE; }
    build_info.rst_detection               = (read_data >> 0)  & 0x00000003;

    PrintMemoryCuBI(build_info, ch_idx);
    return RET_SUCCESS;
}

void DeviceInterface_Base::PrintCommonCuBI( const Build_Info & build_info, const int & ch_idx )
{
    if (ch_idx != CU_WRAPPER_IDX)
    {
        LogMessage(MSG_DEBUG_SETUP, {"xbtest HW IP (" + build_info.cu_name + ") build info (Core " + std::to_string(ch_idx) + "):"});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"xbtest HW IP (" + build_info.cu_name + ") build info:"});
    }
    LogMessage(MSG_DEBUG_SETUP, {"\t - Major version           : "     +    std::to_string(build_info.major_version)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Minor version           : "     +    std::to_string(build_info.minor_version)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Build version           : "     +    std::to_string(build_info.build_version)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Component ID            : "     +    std::to_string(build_info.component_id)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Scratch pad             : 0x"   + NumToStrHex<uint>(build_info.scratch_pad)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Reset detection         : "     +    std::to_string(build_info.rst_detection)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Clock0 frequency        : "     +    std::to_string(build_info.clock0_freq)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Clock1 frequency        : "     +    std::to_string(build_info.clock1_freq)});
}

void DeviceInterface_Base::PrintVerifyCuBI( const Build_Info & build_info )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
}

void DeviceInterface_Base::PrintPowerCuBI( const Build_Info & build_info )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    LogMessage(MSG_DEBUG_SETUP, {"\t - SLR                     : " + std::to_string(build_info.slr)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Throttle mode           : " + std::to_string(build_info.throttle_mode)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Number of Slices        : " + std::to_string(build_info.num_slice)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Number of DSP           : " + std::to_string(build_info.num_dsp)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Number of BRAM          : " + std::to_string(build_info.num_bram)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Number of URAM          : " + std::to_string(build_info.num_uram)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - AIE used                : " + std::to_string(build_info.use_aie)});
}

void DeviceInterface_Base::PrintMemoryCuBI( const Build_Info & build_info, const int & ch_idx )
{
    PrintCommonCuBI(build_info, ch_idx);
    LogMessage(MSG_DEBUG_SETUP, {"\t - Memory type             : " + std::to_string(build_info.mem_type)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - Number of channels      : " + std::to_string(build_info.mem_num_channels)});
    if (ch_idx != CU_WRAPPER_IDX)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - Channel index           : " + std::to_string(build_info.mem_ch_idx)});
    }
}

void DeviceInterface_Base::PrintGtmacCuBI( const Build_Info & build_info )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT index                : " + std::to_string(build_info.gt_index)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT type                 : " + std::to_string(build_info.gt_type)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num lane             : " + std::to_string(build_info.gt_num_lane)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT rate                 : " + std::to_string(build_info.gt_rate)});
}

void DeviceInterface_Base::PrintGTLpbkCuBI( const Build_Info & build_info )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT index                : " + std::to_string(build_info.gt_index)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT type                 : " + std::to_string(build_info.gt_type)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num lane             : " + std::to_string(build_info.gt_num_lane)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT rate                 : " + std::to_string(build_info.gt_rate)});
}

void DeviceInterface_Base::PrintGTPrbsCuBI( const Build_Info & build_info )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT index                : " + std::to_string(build_info.gt_index)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT type                 : " + std::to_string(build_info.gt_type)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT rate                 : " + std::to_string(build_info.gt_rate)});
}

void DeviceInterface_Base::PrintGTFPrbsCuBI( const Build_Info & build_info, const int & gt_idx )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    if (gt_idx == CU_WRAPPER_IDX)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT index                : " + std::to_string(build_info.gt_index)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT type                 : " + std::to_string(build_info.gt_enable)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT rate                 : " + std::to_string(build_info.gt_rate)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT ip sel               : " + std::to_string(build_info.gt_ip_sel)});
    }
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num lane             : " + std::to_string(build_info.gt_num_lane)});
}

void DeviceInterface_Base::PrintGTMPrbsCuBI( const Build_Info & build_info, const int & gt_idx )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    if (gt_idx == CU_WRAPPER_IDX)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT index                : " + std::to_string(build_info.gt_index)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT type                 : " + std::to_string(build_info.gt_enable)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT rate                 : " + std::to_string(build_info.gt_rate)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT ip sel               : " + std::to_string(build_info.gt_ip_sel)});
    }
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num lane             : " + std::to_string(build_info.gt_num_lane)});
}
void DeviceInterface_Base::PrintGTYPPrbsCuBI( const Build_Info & build_info, const int & gt_idx )
{
    PrintCommonCuBI(build_info, CU_WRAPPER_IDX);
    if (gt_idx == CU_WRAPPER_IDX)
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
    }
    else
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT num gt               : " + std::to_string(build_info.gt_num_gt)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT index                : " + std::to_string(build_info.gt_index)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT type                 : " + std::to_string(build_info.gt_enable)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT rate                 : " + std::to_string(build_info.gt_rate)});
        LogMessage(MSG_DEBUG_SETUP, {"\t - GT ip sel               : " + std::to_string(build_info.gt_ip_sel)});
    }
    LogMessage(MSG_DEBUG_SETUP, {"\t - GT num lane             : " + std::to_string(build_info.gt_num_lane)});
}

uint DeviceInterface_Base::GetGTFNumLanes( const uint & cu_idx, const int & gt_idx)
{
    return this->m_cl_gtf_prbs_compute_units[cu_idx].ch_build_info[gt_idx].gt_num_lane;
}
uint DeviceInterface_Base::GetGTMNumLanes( const uint & cu_idx, const int & gt_idx)
{
    return this->m_cl_gtm_prbs_compute_units[cu_idx].ch_build_info[gt_idx].gt_num_lane;
}
uint DeviceInterface_Base::GetGTYPNumLanes( const uint & cu_idx, const int & gt_idx)
{
    return this->m_cl_gtyp_prbs_compute_units[cu_idx].ch_build_info[gt_idx].gt_num_lane;
}
void DeviceInterface_Base::LockGTRstMtx(const int & gt_idx)
{
    this->m_gt_rst_mtx.lock();
    LogMessage(MSG_DEBUG_TESTCASE, {"GT[" + std::to_string(gt_idx) + "]: taking gt reset mutex in lock"});

}
void DeviceInterface_Base::UnLockGTRstMtx(const int & gt_idx)
{
    this->m_gt_rst_mtx.unlock();
    LogMessage(MSG_DEBUG_TESTCASE, {"GT[" + std::to_string(gt_idx) + "]: releasing gt reset mutex"});
}

} // namespace
