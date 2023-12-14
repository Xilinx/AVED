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
proc autoconfigure_platform_fpga_part { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set platforminfo_path   {platforminfo extensions raptor2 part}
    set param_value         [dict_get_quiet $config $platforminfo_path]
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no value found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# platform.fpga_family
############################################################################################################
proc autoconfigure_platform_fpga_family { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "parameter will be auto-configured based on actual fpga_part"
}
############################################################################################################
# platform.name
############################################################################################################
proc autoconfigure_platform_name { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set platforminfo_path   {platforminfo uniqueName}
    set param_value         [dict_get_quiet $config $platforminfo_path]
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no value found in platform info at \{$platforminfo_path\}"

    if {$param_value == {}} {
        set param_value {NOT DEFINED}
        set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
    }
}
############################################################################################################
# platform.interface_uuid
############################################################################################################
proc autoconfigure_platform_interface_uuid { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set platforminfo_path   {platforminfo extensions xclbin append_sections partition_metadata interfaces}
    set interfaces_list     [dict_get_quiet $config $platforminfo_path]
    set param_value         {}
    if {$interfaces_list != {}} {
        if {[llength $interfaces_list] > 0} {
            set interface [lindex $interfaces_list 0]
            set param_value [dict_get_quiet $interface {interface_uuid}]
        }
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no value found in platform info at \{$platforminfo_path\}"

    if {$param_value == {}} {
        set param_value {NOT DEFINED}
        set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
    }
}
############################################################################################################
# platform.mmio_support
############################################################################################################
proc autoconfigure_platform_mmio_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value true
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.is_nodma
############################################################################################################
proc autoconfigure_platform_is_nodma { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value false
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.dma_support
############################################################################################################
proc autoconfigure_platform_dma_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value true
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.p2p_support
############################################################################################################
proc autoconfigure_platform_p2p_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value true
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.mac_addresses_available
############################################################################################################
proc autoconfigure_platform_mac_addresses_available { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value default
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.aie.freq
############################################################################################################
proc autoconfigure_platform_aie_freq { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value default
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.aie.control
############################################################################################################
proc autoconfigure_platform_aie_control { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value default
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.aie.status
############################################################################################################
proc autoconfigure_platform_aie_status { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value default
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# platform.gt
############################################################################################################
proc autoconfigure_platform_gt { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value         {}
    set platforminfo_path   {platforminfo extensions raptor2 resources gts}
    foreach {gt_idx gt_def} [dict_get_quiet $config $platforminfo_path] {
        dict set param_value $gt_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# platform.gt.<gt_idx>.slr
############################################################################################################
proc autoconfigure_platform_gt_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path          [concat [list $cfg_name] $param_path0]
    set param_name          [join $param_path {.}]
    set gt_idx              [lindex $param_path0 2]
    set platforminfo_path   [list platforminfo extensions raptor2 resources gts $gt_idx slr_assignment]
    set param_value         [dict_get_quiet $config $platforminfo_path]
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "GT $gt_idx SLR not found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# platform.gt.<gt_idx>.type
############################################################################################################
proc autoconfigure_platform_gt_type { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path          [concat [list $cfg_name] $param_path0]
    set param_name          [join $param_path {.}]
    set gt_idx              [lindex $param_path0 2]
    set platforminfo_path   [list platforminfo extensions raptor2 resources gts $gt_idx gt_type]
    set param_value         [dict_get_quiet $config $platforminfo_path]
    if {$param_value == {}} {
        set param_value {GTY}
        log_message $config {XBTEST_WIZARD-9} [list $param_name "GT $gt_idx gt_type not found in platform info at \{$platforminfo_path\}. Defaults to: $param_value"]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "GT $gt_idx gt_type not found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# platform.gt.<gt_idx>.refclk_sel
############################################################################################################
proc autoconfigure_platform_gt_refclk_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path          [concat [list $cfg_name] $param_path0]
    set param_name          [join $param_path {.}]
    set gt_idx              [lindex $param_path0 2]
    set param_value         $gt_idx
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "GT selects its own refclk by default"
}
############################################################################################################
# platform.gt.<gt_idx>.group_select
############################################################################################################
proc autoconfigure_platform_gt_group_select { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set gt_idx              [lindex $param_path0 2]
    set platforminfo_path   [list platforminfo extensions raptor2 resources gts $gt_idx gt_group_select]
    set param_value         [dict_get_quiet $config $platforminfo_path]
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "GT $gt_idx group_select not found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# build.pwr_floorplan_dir
############################################################################################################
proc autoconfigure_build_pwr_floorplan_dir { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# build.vpp_options_dir
############################################################################################################
proc autoconfigure_build_vpp_options_dir { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# build.display_pwr_floorplan
############################################################################################################
proc autoconfigure_build_display_pwr_floorplan { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value false
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# build.vpp_link_output
############################################################################################################
proc autoconfigure_build_vpp_link_output { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value default
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.clock
############################################################################################################
proc autoconfigure_cu_configuration_clock { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    foreach {clk_idx} {0 1} {
        dict set param_value $clk_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.clock.<idx>.freq
############################################################################################################
proc autoconfigure_cu_configuration_clock_freq { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set clk_idx     [lindex $param_path0 2]
    set param_value 300
    if {$clk_idx == 1} {
        set param_value 500
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.plram_selection
############################################################################################################
proc autoconfigure_cu_configuration_plram_selection { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value     {}
    set memory_type     {PLRAM}
    set memory_xpfm_def [get_memory_xpfm_def config $param_path0 $memory_type]
    foreach sptag [lsort -dictionary [dict keys $memory_xpfm_def]] {
        set slr [dict get $memory_xpfm_def $sptag]
        if {[dict exist $param_value $slr] == 0} {
            dict set param_value $slr $sptag
        }
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no $memory_type was found in platforminfo"
}
############################################################################################################
# cu_configuration.verify.slr
############################################################################################################
proc autoconfigure_cu_configuration_verify_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value auto
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}

############################################################################################################
# cu_configuration.verify.dna_read
############################################################################################################
proc autoconfigure_cu_configuration_verify_dna_read { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value auto
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.power
############################################################################################################
proc autoconfigure_cu_configuration_power { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path          [concat [list $cfg_name] $param_path0]
    set param_name          [join $param_path {.}]
    set param_value         {}
    set platforminfo_path   {platforminfo extensions raptor2 slrs}
    foreach {slr} [dict_get_quiet $config $platforminfo_path] {
        # We only understand SLRx syntax
        if {[regexp {^SLR([0-9])$} $slr -> slr_idx]} {
            dict set param_value $slr_idx [dict create]
        } else {
            log_message $config {XBTEST_WIZARD-9} [list $param_name "cannot get SLR index in platform info at \{$platforminfo_path\}: $slr. Expected format is: SLRx"]
            set param_value {}
            break
        }
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no SLR found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# cu_configuration.power.<slr_idx>.throttle_mode
############################################################################################################
proc autoconfigure_cu_configuration_power_throttle_mode { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value default
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.gt
############################################################################################################
proc autoconfigure_cu_configuration_gt { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value         {}
    set wizard_path         [list $cfg_name platform gt]
    foreach {gt_idx} [dict keys [dict_get_quiet $config $wizard_path]] {
        dict set param_value $gt_idx {}
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in wizard configuration at \{$wizard_path\}"
}
############################################################################################################
# cu_configuration.gt.<gt_idx>.diff_clocks
############################################################################################################
proc autoconfigure_cu_configuration_gt_diff_clocks { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value         {}
    set gt_idx              [lindex $param_path0 2]
    set platforminfo_path   [list platforminfo extensions raptor2 resources gts $gt_idx diff_clocks]
    set diff_clocks         [dict_get_quiet $config $platforminfo_path]
    if {[llength $diff_clocks] > 0} {
        set param_value [lindex $diff_clocks 0]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "GT $gt_idx diff_clocks not found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# cu_configuration.gt.<gt_idx>.serial_port
############################################################################################################
proc autoconfigure_cu_configuration_gt_serial_port { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set gt_idx              [lindex $param_path0 2]
    set platforminfo_path   [list platforminfo extensions raptor2 resources gts $gt_idx gt_serial_port]
    set param_value         [dict_get_quiet $config $platforminfo_path]
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "GT $gt_idx gt_serial_port not found in platform info at \{$platforminfo_path\}"
}
############################################################################################################
# cu_configuration.gt_mac
############################################################################################################
proc autoconfigure_cu_configuration_gt_mac { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set wizard_path [list $cfg_name platform gt]
    foreach {gt_idx} [dict keys [dict_get_quiet $config $wizard_path]] {
        dict set param_value $gt_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in wizard configuration at \{$wizard_path\}"
}
############################################################################################################
# cu_configuration.gt_mac.<gt_idx>.ip_sel
############################################################################################################
proc autoconfigure_cu_configuration_gt_mac_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value xxv
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.gt_mac.<gt_idx>.enable_rsfec
############################################################################################################
proc autoconfigure_cu_configuration_gt_mac_enable_rsfec { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value false
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.gt_prbs
############################################################################################################
proc autoconfigure_cu_configuration_gt_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set wizard_path [list $cfg_name platform gt]
    foreach {gt_idx} [dict keys [dict_get_quiet $config $wizard_path]] {
        dict set param_value $gt_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in wizard configuration at \{$wizard_path\}"
}
############################################################################################################
# cu_configuration.gt_prbs.<gt_idx>.ip_sel
############################################################################################################
proc autoconfigure_cu_configuration_gt_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value gt_wiz
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.gtf_prbs
############################################################################################################
proc autoconfigure_cu_configuration_gtf_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set wizard_path [list $cfg_name platform gt]
    foreach {gt_idx} [dict keys [dict_get_quiet $config $wizard_path]] {
        dict set param_value $gt_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in wizard configuration at \{$wizard_path\}"
}
############################################################################################################
# cu_configuration.gtf_prbs.<gt_idx>.ip_sel
############################################################################################################
proc autoconfigure_cu_configuration_gtf_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value gtf_wiz
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.gtm_prbs
############################################################################################################
proc autoconfigure_cu_configuration_gtm_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set wizard_path [list $cfg_name platform gt]
    foreach {gt_idx} [dict keys [dict_get_quiet $config $wizard_path]] {
        dict set param_value $gt_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in wizard configuration at \{$wizard_path\}"
}
############################################################################################################
# cu_configuration.gtm_prbs.<gt_idx>.ip_sel
############################################################################################################
proc autoconfigure_cu_configuration_gtm_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value xbtest_sub_gt
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.gtyp_prbs
############################################################################################################
proc autoconfigure_cu_configuration_gtyp_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set wizard_path [list $cfg_name platform gt]
    foreach {gt_idx} [dict keys [dict_get_quiet $config $wizard_path]] {
        dict set param_value $gt_idx [dict create]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no GT found in wizard configuration at \{$wizard_path\}"
}
############################################################################################################
# cu_configuration.gtyp_prbs.<gt_idx>.ip_sel
############################################################################################################
proc autoconfigure_cu_configuration_gtyp_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value xbtest_sub_gt
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.memory
############################################################################################################
proc autoconfigure_cu_configuration_memory { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set memory_types {DDR HBM HOST}
    foreach {memory_type} $memory_types {
        set memory_xpfm_def [get_memory_xpfm_def config $param_path0 $memory_type]
        if {[llength [dict keys $memory_xpfm_def]] > 0} {
            dict set param_value $memory_type {}
        }
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no [join $memory_types "/"] memory found in platform metadata"
}
############################################################################################################
# cu_configuration.memory.<name>.global.target
############################################################################################################
proc autoconfigure_cu_configuration_memory_global_target { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set memory_type [lindex $param_path0 2]
    set param_value board
    if {$memory_type == {HOST}} {
        set param_value host
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_data_size
############################################################################################################
proc autoconfigure_cu_configuration_memory_global_axi_data_size { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value 512
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_id_threads
############################################################################################################
proc autoconfigure_cu_configuration_memory_global_axi_id_threads { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value 1
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_outstanding
############################################################################################################
proc autoconfigure_cu_configuration_memory_global_axi_outstanding { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value 32
    set_autoconfigured_param config $cfg_name $param_path0 $param_value ""
}
############################################################################################################
# cu_configuration.memory.<name>.specific
############################################################################################################
proc autoconfigure_cu_configuration_memory_specific { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value     {}
    set memory_type     [lindex $param_path0 2]
    set memory_xpfm_def [get_memory_xpfm_def config $param_path0 $memory_type]
    set sptag_list      [lsort -dictionary [dict keys $memory_xpfm_def]]

    if {($memory_type == {DDR}) || ($memory_type == {HOST})} {
        # DDR/host: one tag => one CU
        for {set cu_idx 0} {$cu_idx < [llength $sptag_list]} {incr cu_idx} {
            dict set param_value $cu_idx {}
        }
    } elseif {$memory_type == {HBM}} {
        # HBM: one cu => all tag
        if {[llength $sptag_list] > 0} {
            set cu_idx 0
            dict set param_value $cu_idx {}
        }
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no $memory_type memory found in platform metadata"
}
############################################################################################################
# cu_configuration.memory.<name>.specific.<cu_idx>.slr
############################################################################################################
proc autoconfigure_cu_configuration_memory_specific_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value     {}
    set memory_type     [lindex $param_path0 2]
    set cu_idx          [lindex $param_path0 4]
    set memory_xpfm_def [get_memory_xpfm_def config $param_path0 $memory_type]
    set sptag_list      [lsort -dictionary [dict keys $memory_xpfm_def]]
    if {[llength $sptag_list] > $cu_idx} {
        set sptag           [lindex $sptag_list $cu_idx]
        set param_value     [dict_get_quiet $memory_xpfm_def $sptag]
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no $memory_type memory found in platform metadata"
}
############################################################################################################
# cu_configuration.memory.<name>.specific.<cu_idx>.sptag
############################################################################################################
proc autoconfigure_cu_configuration_memory_specific_sptag { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value     {}
    set memory_type     [lindex $param_path0 2]
    set cu_idx          [lindex $param_path0 4]
    set memory_xpfm_def [get_memory_xpfm_def config $param_path0 $memory_type]
    set sptag_list      [lsort -dictionary [dict keys $memory_xpfm_def]]
    if {($memory_type == {DDR}) || ($memory_type == {HOST})} {
        if {[llength $sptag_list] > $cu_idx} {
            set param_value [list [lindex $sptag_list $cu_idx]]
        }
    } elseif {$memory_type == {HBM}} {
        if {[llength $sptag_list] > 0} {
            set param_value $sptag_list
        }
    }
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no $memory_type memory found in platform metadata"
}
############################################################################################################
# cu_selection.power
############################################################################################################
proc autoconfigure_cu_selection_power { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.gt_mac
############################################################################################################
proc autoconfigure_cu_selection_gt_mac { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.gt_lpbk
############################################################################################################
proc autoconfigure_cu_selection_gt_lpbk { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.gt_prbs
############################################################################################################
proc autoconfigure_cu_selection_gt_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.gtf_prbs
############################################################################################################
proc autoconfigure_cu_selection_gtf_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.gtm_prbs
############################################################################################################
proc autoconfigure_cu_selection_gtm_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.gtyp_prbs
############################################################################################################
proc autoconfigure_cu_selection_gtyp_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
############################################################################################################
# cu_selection.memory
############################################################################################################
proc autoconfigure_cu_selection_memory { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_value {}
    set_autoconfigured_param config $cfg_name $param_path0 $param_value "no auto-configuration supported for this parameter"
}
