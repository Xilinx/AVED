# Copyright (C) 2022 Xilinx, Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

############################################################################################################
# platform.fpga_part
############################################################################################################
proc merge_platform_fpga_part { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.fpga_family
############################################################################################################
proc merge_platform_fpga_family { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.name
############################################################################################################
proc merge_platform_name { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.interface_uuid
############################################################################################################
proc merge_platform_interface_uuid { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.mmio_support
############################################################################################################
proc merge_platform_mmio_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.is_nodma
############################################################################################################
proc merge_platform_is_nodma { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.dma_support
############################################################################################################
proc merge_platform_dma_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.p2p_support
############################################################################################################
proc merge_platform_p2p_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.mac_addresses_available
############################################################################################################
proc merge_platform_mac_addresses_available { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.aie.freq
############################################################################################################
proc merge_platform_aie_freq { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.aie.control
############################################################################################################
proc merge_platform_aie_control { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.aie.status
############################################################################################################
proc merge_platform_aie_status { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.gt
############################################################################################################
proc merge_platform_gt { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# platform.gt.<gt_idx>.slr
############################################################################################################
proc merge_platform_gt_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.gt.<gt_idx>.type
############################################################################################################
proc merge_platform_gt_type { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.gt.<gt_idx>.group_select
############################################################################################################
proc merge_platform_gt_group_select { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# platform.gt.<gt_idx>.refclk_sel
############################################################################################################
proc merge_platform_gt_refclk_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# build.pwr_floorplan_dir
############################################################################################################
proc merge_build_pwr_floorplan_dir { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# build.vpp_options_dir
############################################################################################################
proc merge_build_vpp_options_dir { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# build.display_pwr_floorplan
############################################################################################################
proc merge_build_display_pwr_floorplan { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# build.vpp_link_output
############################################################################################################
proc merge_build_vpp_link_output { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.clock
############################################################################################################
proc merge_cu_configuration_clock { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.clock.<idx>.freq
############################################################################################################
proc merge_cu_configuration_clock_freq { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.plram_selection
############################################################################################################
proc merge_cu_configuration_plram_selection { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.verify
############################################################################################################
proc merge_cu_configuration_verify_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
proc merge_cu_configuration_verify_dna_read { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.power
############################################################################################################
proc merge_cu_configuration_power { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.power.<slr_idx>.throttle_mode
############################################################################################################
proc merge_cu_configuration_power_throttle_mode { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt
############################################################################################################
proc merge_cu_configuration_gt { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt.<gt_idx>.diff_clocks
############################################################################################################
proc merge_cu_configuration_gt_diff_clocks { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt.<gt_idx>.serial_port
############################################################################################################
proc merge_cu_configuration_gt_serial_port { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt_mac
############################################################################################################
proc merge_cu_configuration_gt_mac { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt_mac.<gt_idx>.ip_sel
############################################################################################################
proc merge_cu_configuration_gt_mac_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt_mac.<gt_idx>.enable_rsfec
############################################################################################################
proc merge_cu_configuration_gt_mac_enable_rsfec { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt_prbs
############################################################################################################
proc merge_cu_configuration_gt_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gt_prbs.<gt_idx>.ip_sel
############################################################################################################
proc merge_cu_configuration_gt_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gtf_prbs
############################################################################################################
proc merge_cu_configuration_gtf_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gtf_prbs.<gt_idx>.ip_sel
############################################################################################################
proc merge_cu_configuration_gtf_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gtm_prbs
############################################################################################################
proc merge_cu_configuration_gtm_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gtm_prbs.<gt_idx>.ip_sel
############################################################################################################
proc merge_cu_configuration_gtm_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gtyp_prbs
############################################################################################################
proc merge_cu_configuration_gtyp_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.gtyp_prbs.<gt_idx>.ip_sel
############################################################################################################
proc merge_cu_configuration_gtyp_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory
############################################################################################################
proc merge_cu_configuration_memory { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_dict config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory.<name>.global.target
############################################################################################################
proc merge_cu_configuration_memory_global_target { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_data_size
############################################################################################################
proc merge_cu_configuration_memory_global_axi_data_size { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_id_threads
############################################################################################################
proc merge_cu_configuration_memory_global_axi_id_threads { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_outstanding
############################################################################################################
proc merge_cu_configuration_memory_global_axi_outstanding { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory.<name>.specific
############################################################################################################
proc merge_cu_configuration_memory_specific { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0; # Do not merge as dict, if user override the CU configuration we use the provided dict
}
############################################################################################################
# cu_configuration.memory.<name>.specific.<cu_idx>.slr
############################################################################################################
proc merge_cu_configuration_memory_specific_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_configuration.memory.<name>.specific.<cu_idx>.sptag
############################################################################################################
proc merge_cu_configuration_memory_specific_sptag { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.power
############################################################################################################
proc merge_cu_selection_power { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.gt_mac
############################################################################################################
proc merge_cu_selection_gt_mac { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.gt_lpbk
############################################################################################################
proc merge_cu_selection_gt_lpbk { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.gt_prbs
############################################################################################################
proc merge_cu_selection_gt_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.gtf_prbs
############################################################################################################
proc merge_cu_selection_gtf_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.gtm_prbs
############################################################################################################
proc merge_cu_selection_gtm_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.gtyp_prbs
############################################################################################################
proc merge_cu_selection_gtyp_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
############################################################################################################
# cu_selection.memory
############################################################################################################
proc merge_cu_selection_memory { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    merge_param config $cfg_name $param_path0
}
