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

proc format_name_cu { name } {
    set name_rep [string tolower $name]
    foreach rep_char {\[ \] \{ \} * - + . < > ~ @ , ; { } } {
        set pos [string first $rep_char $name_rep]
        while {$pos != -1} {
            set name_rep [string replace $name_rep $pos $pos "_"]
            set pos [string first $rep_char $name_rep]
        }
    }
    set name_rep [string trim $name_rep {_}]
    return $name_rep
}


#Generates dictionary of MEMORY compute unit configuration
proc configure_memory_cu { config_ref } {
    variable C_KRNL_MODE_MEMORY
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set cu_cfg_list [list]

    foreach {memory_name} [dict get $config wizard_actual_config cu_selection memory] {

        set target              [dict get $config wizard_actual_config cu_configuration memory $memory_name global target]
        set axi_data_size       [dict get $config wizard_actual_config cu_configuration memory $memory_name global axi_data_size]
        set axi_id_threads      [dict get $config wizard_actual_config cu_configuration memory $memory_name global axi_id_threads]
        set axi_outstanding     [dict get $config wizard_actual_config cu_configuration memory $memory_name global axi_outstanding]
        set specific_def        [dict get $config wizard_actual_config cu_configuration memory $memory_name specific]

        set num_cu [llength [dict keys $specific_def]]

        # Detect if the configuration correspond to SC or MC memory
        # If the number of CU is 1 and the number of sptags is > 1 => multi-channel
        set type {single_channel}
        if {$num_cu == 1} {
            set num_port [llength [dict get $specific_def 0 sptag]]
            if {$num_port > 1} {
                set type {multi_channel}
            }
        }

        # Save memory definition
        if {![dict exists $config memory_definition]} {
            dict set config memory_definition [list]
        }
        set memory_definition_list [dict get $config memory_definition]
        set def {}
        dict set def memory_name        $memory_name
        dict set def target             $target
        dict set def type               $type
        dict set def axi_data_size      $axi_data_size
        dict set def axi_id_threads     $axi_id_threads
        dict set def axi_outstanding    $axi_outstanding
        lappend memory_definition_list $def
        dict set config memory_definition $memory_definition_list

        foreach {cu_idx cu_def} $specific_def {

            set sp_tag_list [dict get $cu_def sptag]
            set slr         [dict get $cu_def slr]

            set num_port    [llength $sp_tag_list]
            set plram       [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
            regexp {^SLR([0-9])$} $slr -> slr_idx

            # Save CU config
            set cu_cfg {}
            dict set cu_cfg kernel_name         [format {krnl_memtest_%s_%02d_%02d} [format_name_cu $memory_name] $num_port $cu_idx]
            dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
            dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
            dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
            dict set cu_cfg slr_idx             $slr_idx
            dict set cu_cfg krnl_mode           $C_KRNL_MODE_MEMORY
            dict set cu_cfg sp_m00_axi          $plram
            for {set ii 0} {$ii < $num_port} {incr ii} {
                set sp_tag      [lindex $sp_tag_list $ii]
                set sp_tag_name [format {sp_m%02d_axi} [expr $ii + 1]]
                dict set cu_cfg $sp_tag_name $sp_tag
            }
            dict set cu_cfg memory_name         $memory_name
            dict set cu_cfg target              $target
            dict set cu_cfg type                $type
            dict set cu_cfg num_port            $num_port
            dict set cu_cfg inst                $cu_idx
            dict set cu_cfg axi_data_size       $axi_data_size
            dict set cu_cfg axi_id_threads      $axi_id_threads
            dict set cu_cfg axi_outstanding     $axi_outstanding
            log_message $config {GEN_CU_CONFIG-2} [list {MEMORY} $cu_cfg]; # Report configuration
            lappend cu_cfg_list $cu_cfg
        }
    }


    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {MEMORY}]; # Report no CU
    } else {
        log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {MEMORY}]; # Report number of configurations
        if {[dict exists $config cu_config]} {
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}

proc set_power_throttle_mode { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    # Determine how the power CU will be cascaded
    set pwr_cu_cfg { \
        default        {} \
        INTERNAL_CLK   {} \
        INTERNAL_MACRO {} \
        EXTERNAL_CLK   {} \
        EXTERNAL_MACRO {} \
    }
    foreach {slr_idx} [lsort -integer [dict get $config wizard_actual_config cu_selection power]] {
        set throttle_mode [dict_get_quiet $config [list wizard_actual_config cu_configuration power $slr_idx throttle_mode]]
        if {$throttle_mode == {}} {
            set throttle_mode default
        }
        dict lappend pwr_cu_cfg $throttle_mode $slr_idx
    }

    # Check if default throttle mode are provided
    if {[llength [dict get $pwr_cu_cfg default]] == 0} {
        return; # Nothing to do
    }

    # Find master CU
    set master_inst {}
    set master_mode {}
    foreach throttle_mode {INTERNAL_CLK INTERNAL_MACRO} {
        set inst_list   [dict get $pwr_cu_cfg $throttle_mode]
        if {$inst_list != {}} {
            set master_inst [lindex $inst_list 0]
            set master_mode $throttle_mode
            break
        }
    }
    if {$master_mode == {INTERNAL_MACRO}} {
        set slave_mode EXTERNAL_MACRO
    } else {
        set slave_mode  EXTERNAL_CLK
    }

    # Set default throttle_mode
    if {$master_inst == {}} {
        # No master power CU found for connection to slave power CU
        # Set first default found as master and others as slave
        set inst_list   [dict get $pwr_cu_cfg default]
        set master_inst [lindex $inst_list 0]
        set master_mode INTERNAL_CLK
        dict set config wizard_actual_config cu_configuration power $master_inst throttle_mode $master_mode
        log_message $config {GEN_CU_CONFIG-12} [list $master_inst $master_mode]
    }
    # Set other default as slave
    foreach {inst} [dict get $pwr_cu_cfg default] {
        if {$inst != $master_inst} {
            # Ignore master inst as it could be set from default
            dict set config wizard_actual_config cu_configuration power $inst throttle_mode $slave_mode
            log_message $config {GEN_CU_CONFIG-12} [list $inst $slave_mode]
        }
    }
}

#Generates dictionary of Power compute unit configuration
proc configure_power_cu { config_ref } {
    variable C_KRNL_MODE_POWER
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set cu_cfg_list [list]

    set use_aie 0

    # Determine how the power CU will be cascaded
    set_power_throttle_mode config

    foreach {slr_idx} [dict get $config wizard_actual_config cu_selection power] {
        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection SLR${slr_idx}]
        set throttle_mode   [dict get $config wizard_actual_config cu_configuration power $slr_idx throttle_mode]

        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_powertest_slr$slr_idx
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg krnl_mode           $C_KRNL_MODE_POWER
        dict set cu_cfg throttle_mode       $throttle_mode
        dict set cu_cfg use_aie             0; # AIE disabled by default
        dict set cu_cfg num_aie_used        0
        set aie_geometry_slr [dict_get_quiet $config [list C_DYNAMIC_GEOMETRY AIE_GEOMETRY $slr_idx]]
        if {$aie_geometry_slr != {}} {
            set aie_type [dict_get_quiet $config [list C_DYNAMIC_GEOMETRY AIE_TYPE]]
            if {$aie_type == {}} {
                set aie_type AIE_CORE; # Default to AIE1
            }
            set aie_utilization_slr [dict_get_quiet $config [list C_UTILIZATION AIE_UTILIZATION SLR_$slr_idx]]
            if {$aie_utilization_slr == {}} {
                log_message $config {GEN_CU_CONFIG-8} [list $slr_idx]; # A required user_parameter is not defined.
            } elseif {$aie_utilization_slr > 0} {
                if {$use_aie} {
                    log_message $config {GEN_CU_CONFIG-11}; # support only 1 SLR
                } else {
                    set use_aie 1
                    dict set cu_cfg use_aie 1
                    log_message $config {GEN_CU_CONFIG-10} [list $slr_idx [file join [dict get $config wizard_actual_config build pwr_floorplan_dir] utilization.tcl]]
                    gen_aie_config config cu_cfg $aie_type $aie_geometry_slr $aie_utilization_slr
                }
            }
        }
        # if the power control is coming from another CU, so there is no need of PLRAM connection, except if the AIE are used
        if { (($throttle_mode != {EXTERNAL_MACRO}) && ($throttle_mode != {EXTERNAL_CLK})) || ([dict get $cu_cfg use_aie]) } {
            dict set cu_cfg sp_m00_axi          $plram
        }

        log_message $config {GEN_CU_CONFIG-2} [list {POWER} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {POWER}]; # Report no CU
    } else {
        log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {POWER}]; # Report number of configurations
        if {[dict exists $config cu_config]} {
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}

proc set_gt_rate { krnl_mode ip_sel gt_type } {
    variable C_KRNL_MODE_GT_MAC
    variable C_KRNL_MODE_GT_LPBK
    variable C_KRNL_MODE_GT_PRBS
    variable C_KRNL_MODE_GTF_PRBS
    variable C_KRNL_MODE_GTM_PRBS
    variable C_KRNL_MODE_GTYP_PRBS

    if {$krnl_mode == $C_KRNL_MODE_GT_MAC} {
        set gt_rate {10G_25G}; # Default value. (10G_25G only valid for GT_MAC)
        if {$ip_sel == {xbtest_sub_xxv_gt}} {
            set gt_rate {25G}; # versal: fix rate 25GbE
        }
        if {$gt_type == {GTM}} {
            set gt_rate {25G}; #  GTM: fix rate 25GbE
        }
    } elseif {$krnl_mode == $C_KRNL_MODE_GT_LPBK} {
        set gt_rate {25G}; #  fix rate 25GbE
    } elseif {$krnl_mode == $C_KRNL_MODE_GT_PRBS} {
        if {$ip_sel == {xbtest_sub_gt}} {
            set gt_rate {25G}; #  fix rate 25GbE
        } elseif {$ip_sel == {gt_wiz}} {
            set gt_rate {25G}; #  fix rate 25GbE
        }
    } elseif {$krnl_mode == $C_KRNL_MODE_GTF_PRBS} {
        if {$ip_sel == {gtf_wiz}} {
            set gt_rate {10G}; #  fix rate 25GbE
        }
    } elseif {$krnl_mode == $C_KRNL_MODE_GTM_PRBS} {
        if {$ip_sel == {xbtest_sub_gt}} {
            set gt_rate {56G}; #  fix rate 56GbE
        }
    } elseif {$krnl_mode == $C_KRNL_MODE_GTYP_PRBS} {
        if {$ip_sel == {xbtest_sub_gt}} {
            set gt_rate {32G}; #  fix rate 32Gbs
        }
    }
    return $gt_rate
}

#Generates dictionary of GT MAC compute unit configuration
proc configure_gt_mac_cu { config_ref } {
    variable C_KRNL_MODE_GT_MAC
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set krnl_mode $C_KRNL_MODE_GT_MAC
    set cu_cfg_list [list]

    foreach {gt_idx} [dict get $config wizard_actual_config cu_selection gt_mac] {
        set slr             [dict get $config wizard_actual_config platform            gt      $gt_idx slr]
        set type            [dict get $config wizard_actual_config platform            gt      $gt_idx type]
        set group_select    [dict get $config wizard_actual_config platform            gt      $gt_idx group_select]
        set serial_port     [dict get $config wizard_actual_config cu_configuration    gt      $gt_idx serial_port]
        set diff_clocks     [dict get $config wizard_actual_config cu_configuration    gt      $gt_idx diff_clocks]
        set enable_rsfec    [dict get $config wizard_actual_config cu_configuration    gt_mac  $gt_idx enable_rsfec]
        set ip_sel          [dict get $config wizard_actual_config cu_configuration    gt_mac  $gt_idx ip_sel]
        set gt_rate         [set_gt_rate $krnl_mode $ip_sel $type]

        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
        regexp {^SLR([0-9])$} $slr -> slr_idx

        # Save gt_mac definition
        if {![dict exists $config gt_mac_definition]} {
            dict set config gt_mac_definition [list]
        }
        set gt_mac_def_list [dict get $config gt_mac_definition]
        set def {}
        dict set def gt_idx          $gt_idx
        dict set def type            $type
        dict set def ip_sel          $ip_sel
        dict set def gt_rate         $gt_rate
        dict set def enable_rsfec    $enable_rsfec
        dict set def group_select    $group_select
        dict set def serial_port     $serial_port
        dict set def diff_clocks     $diff_clocks
        lappend gt_mac_def_list $def
        dict set config gt_mac_definition $gt_mac_def_list

        # Save CU config
        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_gt_mac_test$gt_idx
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg krnl_mode           $krnl_mode
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg sp_m00_axi          $plram
        dict set cu_cfg gt_idx              $gt_idx
        dict set cu_cfg type                $type
        dict set cu_cfg ip_sel              $ip_sel
        dict set cu_cfg gt_rate             $gt_rate
        dict set cu_cfg enable_rsfec        $enable_rsfec
        dict set cu_cfg group_select        $group_select
        dict set cu_cfg serial_port         $serial_port
        dict set cu_cfg diff_clocks         $diff_clocks
        log_message $config {GEN_CU_CONFIG-2} [list {GT_MAC} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {GT_MAC}]; # Report no CU
    } else {
        if {[dict exists $config cu_config]} {
            log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {GT_MAC}]; # Report number of configurations
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}


#Generates dictionary of GT LPBK compute unit configuration
proc configure_gt_lpbk_cu { config_ref } {
    variable C_KRNL_MODE_GT_LPBK
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set krnl_mode $C_KRNL_MODE_GT_LPBK
    set cu_cfg_list [list]

    foreach {gt_idx} [dict get $config wizard_actual_config cu_selection gt_lpbk] {
        set slr             [dict get $config wizard_actual_config platform            gt $gt_idx slr]
        set type            [dict get $config wizard_actual_config platform            gt $gt_idx type]
        set group_select    [dict get $config wizard_actual_config platform            gt $gt_idx group_select]
        set serial_port     [dict get $config wizard_actual_config cu_configuration    gt $gt_idx serial_port]
        set diff_clocks     [dict get $config wizard_actual_config cu_configuration    gt $gt_idx diff_clocks]
        set gt_rate         [set_gt_rate $krnl_mode {} $type]

        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
        regexp {^SLR([0-9])$} $slr -> slr_idx


        # Save gt_lpbk definition
        if {![dict exists $config gt_lpbk_definition]} {
            dict set config gt_lpbk_definition [list]
        }

        set gt_lpbk_def_list [dict get $config gt_lpbk_definition]
        set def {}
        dict set def gt_idx         $gt_idx
        dict set def type           $type
        dict set def gt_rate        $gt_rate
        dict set def group_select   $group_select
        dict set def serial_port    $serial_port
        dict set def diff_clocks    $diff_clocks
        lappend gt_lpbk_def_list $def
        dict set config gt_lpbk_definition $gt_lpbk_def_list

        # Save CU config
        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_gt_lpbk_test$gt_idx
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg krnl_mode           $krnl_mode
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg sp_m00_axi          $plram
        dict set cu_cfg gt_idx              $gt_idx
        dict set cu_cfg type                $type
        dict set cu_cfg gt_rate             $gt_rate
        dict set cu_cfg group_select        $group_select
        dict set cu_cfg serial_port         $serial_port
        dict set cu_cfg diff_clocks         $diff_clocks
        log_message $config {GEN_CU_CONFIG-2} [list {GT_LPBK} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {GT_LPBK}]; # Report no CU
    } else {
        if {[dict exists $config cu_config]} {
            log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {GT_LPBK}]; # Report number of configurations
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}


#Generates dictionary of GT PRBS compute unit configuration
proc configure_gt_prbs_cu { config_ref } {
    variable C_KRNL_MODE_GT_PRBS
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set krnl_mode $C_KRNL_MODE_GT_PRBS
    set cu_cfg_list [list]

    foreach {gt_idx} [dict get $config wizard_actual_config cu_selection gt_prbs] {
        set slr             [dict get $config wizard_actual_config platform            gt       $gt_idx slr]
        set type            [dict get $config wizard_actual_config platform            gt       $gt_idx type]
        set group_select    [dict get $config wizard_actual_config platform            gt       $gt_idx group_select]
        set serial_port     [dict get $config wizard_actual_config cu_configuration    gt       $gt_idx serial_port]
        set diff_clocks     [dict get $config wizard_actual_config cu_configuration    gt       $gt_idx diff_clocks]
        set ip_sel          [dict get $config wizard_actual_config cu_configuration    gt_prbs  $gt_idx ip_sel]
        set gt_rate         [set_gt_rate $krnl_mode $ip_sel $type]

        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
        regexp {^SLR([0-9])$} $slr -> slr_idx


        # Save gt_prbs definition
        if {![dict exists $config gt_prbs_definition]} {
            dict set config gt_prbs_definition [list]
        }

        set gt_prbs_def_list [dict get $config gt_prbs_definition]
        set def {}
        dict set def gt_idx         $gt_idx
        dict set def type           $type
        dict set def ip_sel         $ip_sel
        dict set def gt_rate        $gt_rate
        dict set def group_select   $group_select
        dict set def serial_port    $serial_port
        dict set def diff_clocks    $diff_clocks
        lappend gt_prbs_def_list $def
        dict set config gt_prbs_definition $gt_prbs_def_list

        # Save CU config
        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_gt_prbs_test$gt_idx
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg krnl_mode           $krnl_mode
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg sp_m00_axi          $plram
        dict set cu_cfg gt_idx              $gt_idx
        dict set cu_cfg type                $type
        dict set cu_cfg ip_sel              $ip_sel
        dict set cu_cfg gt_rate             $gt_rate
        dict set cu_cfg group_select        $group_select
        dict set cu_cfg serial_port         $serial_port
        dict set cu_cfg diff_clocks         $diff_clocks
        log_message $config {GEN_CU_CONFIG-2} [list {GT_PRBS} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {GT_PRBS}]; # Report no CU
    } else {
        if {[dict exists $config cu_config]} {
            log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {GT_PRBS}]; # Report number of configurations
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}

#Generates dictionary of GTF PRBS compute unit configuration
proc configure_gtf_prbs_cu { config_ref } {
    variable C_KRNL_MODE_GTF_PRBS
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set krnl_mode $C_KRNL_MODE_GTF_PRBS
    set cu_cfg_list [list]

    set slr_gt_idx {}

    foreach {gt_idx} [dict get $config wizard_actual_config cu_selection gtf_prbs] {
        set slr [dict get $config wizard_actual_config platform gt $gt_idx slr]

        set gt_idx_list [list]
        if {[dict exists $slr_gt_idx $slr]} {
            set gt_idx_list [dict get $slr_gt_idx $slr]
        }
        lappend gt_idx_list $gt_idx
        dict set slr_gt_idx $slr $gt_idx_list
    }

    foreach {slr gt_idx_list} $slr_gt_idx {
        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
        regexp {^SLR([0-9])$} $slr -> slr_idx

        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_gtf_prbs_test_[format "%02d" [llength $gt_idx_list]]_[format "%02d" $slr_idx]
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg krnl_mode           $krnl_mode
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg sp_m00_axi          $plram
        dict set cu_cfg gt_cfg              {}

        for {set gt_idx 0} {$gt_idx < 32} {incr gt_idx} {

            dict set cu_cfg gt_cfg $gt_idx {}

            if {$gt_idx ni $gt_idx_list} {
                set enable          false
                set type            {}
                set group_select    {}
                set serial_port     {}
                set diff_clocks     {}
                set ip_sel          {}
                set gt_rate         {}
            } else {
                set enable          true
                set type            [dict get $config wizard_actual_config platform            gt       $gt_idx type]
                set group_select    [dict get $config wizard_actual_config platform            gt       $gt_idx group_select]
                set serial_port     [dict get $config wizard_actual_config cu_configuration    gt       $gt_idx serial_port]
                set diff_clocks     [dict get $config wizard_actual_config cu_configuration    gt       $gt_idx diff_clocks]
                set refclk_sel      [dict get $config wizard_actual_config platform            gt       $gt_idx refclk_sel]
                set ip_sel          [dict get $config wizard_actual_config cu_configuration    gtf_prbs $gt_idx ip_sel]
                set gt_rate         [set_gt_rate $krnl_mode $ip_sel $type]

                # Save gtf_prbs definition
                if {![dict exists $config gtf_prbs_definition]} {
                    dict set config gtf_prbs_definition [list]
                }

                set gtf_prbs_def_list [dict get $config gtf_prbs_definition]
                set def {}
                dict set def gt_idx         $gt_idx
                dict set def type           $type
                dict set def ip_sel         $ip_sel
                dict set def gt_rate        $gt_rate
                dict set def group_select   $group_select
                dict set def serial_port    $serial_port
                dict set def diff_clocks    $diff_clocks
                dict set def refclk_sel     $refclk_sel
                lappend gtf_prbs_def_list $def
                dict set config gtf_prbs_definition $gtf_prbs_def_list
            }
            # Save GT config
            dict set cu_cfg gt_cfg $gt_idx enable       $enable
            dict set cu_cfg gt_cfg $gt_idx type         $type
            dict set cu_cfg gt_cfg $gt_idx ip_sel       $ip_sel
            dict set cu_cfg gt_cfg $gt_idx gt_rate      $gt_rate
            dict set cu_cfg gt_cfg $gt_idx group_select $group_select
            dict set cu_cfg gt_cfg $gt_idx serial_port  $serial_port
            dict set cu_cfg gt_cfg $gt_idx diff_clocks  $diff_clocks
            dict set cu_cfg gt_cfg $gt_idx refclk_sel  $refclk_sel
        }
        # Save CU config
        log_message $config {GEN_CU_CONFIG-2} [list {GTF_PRBS} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {GTF_PRBS}]; # Report no CU
    } else {
        if {[dict exists $config cu_config]} {
            log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {GTF_PRBS}]; # Report number of configurations
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}

proc configure_gtm_prbs_cu { config_ref } {
    variable C_KRNL_MODE_GTM_PRBS
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set krnl_mode $C_KRNL_MODE_GTM_PRBS
    set cu_cfg_list [list]

    set slr_gt_idx {}

    foreach {gt_idx} [dict get $config wizard_actual_config cu_selection gtm_prbs] {
        set slr [dict get $config wizard_actual_config platform gt $gt_idx slr]

        set gt_idx_list [list]
        if {[dict exists $slr_gt_idx $slr]} {
            set gt_idx_list [dict get $slr_gt_idx $slr]
        }
        lappend gt_idx_list $gt_idx
        dict set slr_gt_idx $slr $gt_idx_list
    }

    foreach {slr gt_idx_list} $slr_gt_idx {
        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
        regexp {^SLR([0-9])$} $slr -> slr_idx

        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_gtm_prbs_test_[format "%02d" [llength $gt_idx_list]]_[format "%02d" $slr_idx]
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg krnl_mode           $krnl_mode
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg sp_m00_axi          $plram
        dict set cu_cfg gt_cfg              {}

        for {set gt_idx 0} {$gt_idx < 32} {incr gt_idx} {

            dict set cu_cfg gt_cfg $gt_idx {}

            if {$gt_idx ni $gt_idx_list} {
                set enable          false
                set type            {}
                set group_select    {}
                set serial_port     {}
                set diff_clocks     {}
                set refclk_sel      {}
                set ip_sel          {}
                set gt_rate         {}
            } else {
                set enable          true
                set type            [dict get $config wizard_actual_config platform            gt       $gt_idx type]
                set group_select    [dict get $config wizard_actual_config platform            gt       $gt_idx group_select]
                set serial_port     [dict get $config wizard_actual_config cu_configuration    gt       $gt_idx serial_port]
                set diff_clocks     [dict get $config wizard_actual_config cu_configuration    gt       $gt_idx diff_clocks]
                set refclk_sel      [dict get $config wizard_actual_config platform            gt       $gt_idx refclk_sel]
                set ip_sel          [dict get $config wizard_actual_config cu_configuration    gtm_prbs $gt_idx ip_sel]
                set gt_rate         [set_gt_rate $krnl_mode $ip_sel $type]

                # Save gtm_prbs definition
                if {![dict exists $config gtm_prbs_definition]} {
                    dict set config gtm_prbs_definition [list]
                }

                set gtm_prbs_def_list [dict get $config gtm_prbs_definition]
                set def {}
                dict set def gt_idx         $gt_idx
                dict set def type           $type
                dict set def ip_sel         $ip_sel
                dict set def gt_rate        $gt_rate
                dict set def group_select   $group_select
                dict set def serial_port    $serial_port
                dict set def diff_clocks    $diff_clocks
                dict set def refclk_sel     $refclk_sel
                lappend gtm_prbs_def_list $def
                dict set config gtm_prbs_definition $gtm_prbs_def_list
            }
            # Save GT config
            dict set cu_cfg gt_cfg $gt_idx enable       $enable
            dict set cu_cfg gt_cfg $gt_idx type         $type
            dict set cu_cfg gt_cfg $gt_idx ip_sel       $ip_sel
            dict set cu_cfg gt_cfg $gt_idx gt_rate      $gt_rate
            dict set cu_cfg gt_cfg $gt_idx group_select $group_select
            dict set cu_cfg gt_cfg $gt_idx serial_port  $serial_port
            dict set cu_cfg gt_cfg $gt_idx diff_clocks  $diff_clocks
            dict set cu_cfg gt_cfg $gt_idx refclk_sel   $refclk_sel
        }
        # Save CU config
        log_message $config {GEN_CU_CONFIG-2} [list {GTM_PRBS} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {GTM_PRBS}]; # Report no CU
    } else {
        if {[dict exists $config cu_config]} {
            log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {GTM_PRBS}]; # Report number of configurations
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}

proc configure_gtyp_prbs_cu { config_ref } {
    variable C_KRNL_MODE_GTYP_PRBS
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set krnl_mode $C_KRNL_MODE_GTYP_PRBS
    set cu_cfg_list [list]

    set slr_gt_idx {}

    foreach {gt_idx} [dict get $config wizard_actual_config cu_selection gtyp_prbs] {
        set slr [dict get $config wizard_actual_config platform gt $gt_idx slr]

        set gt_idx_list [list]
        if {[dict exists $slr_gt_idx $slr]} {
            set gt_idx_list [dict get $slr_gt_idx $slr]
        }
        lappend gt_idx_list $gt_idx
        dict set slr_gt_idx $slr $gt_idx_list
    }

    foreach {slr gt_idx_list} $slr_gt_idx {
        set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
        regexp {^SLR([0-9])$} $slr -> slr_idx

        set cu_cfg {}
        dict set cu_cfg kernel_name         krnl_gtyp_prbs_test_[format "%02d" [llength $gt_idx_list]]_[format "%02d" $slr_idx]
        dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
        dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
        dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
        dict set cu_cfg krnl_mode           $krnl_mode
        dict set cu_cfg slr_idx             $slr_idx
        dict set cu_cfg sp_m00_axi          $plram
        dict set cu_cfg gt_cfg              {}

        for {set gt_idx 0} {$gt_idx < 32} {incr gt_idx} {

            dict set cu_cfg gt_cfg $gt_idx {}

            if {$gt_idx ni $gt_idx_list} {
                set enable          false
                set type            {}
                set group_select    {}
                set serial_port     {}
                set diff_clocks     {}
                set refclk_sel      {}
                set ip_sel          {}
                set gt_rate         {}
            } else {
                set enable          true
                set type            [dict get $config wizard_actual_config platform            gt        $gt_idx type]
                set group_select    [dict get $config wizard_actual_config platform            gt        $gt_idx group_select]
                set serial_port     [dict get $config wizard_actual_config cu_configuration    gt        $gt_idx serial_port]
                set diff_clocks     [dict get $config wizard_actual_config cu_configuration    gt        $gt_idx diff_clocks]
                set refclk_sel      [dict get $config wizard_actual_config platform            gt        $gt_idx refclk_sel]
                set ip_sel          [dict get $config wizard_actual_config cu_configuration    gtyp_prbs $gt_idx ip_sel]
                set gt_rate         [set_gt_rate $krnl_mode $ip_sel $type]

                # Save gtyp_prbs definition
                if {![dict exists $config gtyp_prbs_definition]} {
                    dict set config gtyp_prbs_definition [list]
                }

                set gtyp_prbs_def_list [dict get $config gtyp_prbs_definition]
                set def {}
                dict set def gt_idx         $gt_idx
                dict set def type           $type
                dict set def ip_sel         $ip_sel
                dict set def gt_rate        $gt_rate
                dict set def group_select   $group_select
                dict set def serial_port    $serial_port
                dict set def diff_clocks    $diff_clocks
                dict set def refclk_sel     $refclk_sel
                lappend gtyp_prbs_def_list $def
                dict set config gtyp_prbs_definition $gtyp_prbs_def_list
            }
            # Save GT config
            dict set cu_cfg gt_cfg $gt_idx enable       $enable
            dict set cu_cfg gt_cfg $gt_idx type         $type
            dict set cu_cfg gt_cfg $gt_idx ip_sel       $ip_sel
            dict set cu_cfg gt_cfg $gt_idx gt_rate      $gt_rate
            dict set cu_cfg gt_cfg $gt_idx group_select $group_select
            dict set cu_cfg gt_cfg $gt_idx serial_port  $serial_port
            dict set cu_cfg gt_cfg $gt_idx diff_clocks  $diff_clocks
            dict set cu_cfg gt_cfg $gt_idx refclk_sel   $refclk_sel
        }
        # Save CU config
        log_message $config {GEN_CU_CONFIG-2} [list {GTYP_PRBS} $cu_cfg]; # Report configuration
        lappend cu_cfg_list $cu_cfg
    }

    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {GTYP_PRBS}]; # Report no CU
    } else {
        if {[dict exists $config cu_config]} {
            log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {GTYP_PRBS}]; # Report number of configurations
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}

#Generates dictionary of Verify compute unit configuration
proc configure_verify_cu { config_ref } {
    variable C_KRNL_MODE_VERIFY
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set cu_cfg_list [list]

    set dna_read   [dict get $config wizard_actual_config cu_configuration verify dna_read]
    set slr        [dict get $config wizard_actual_config cu_configuration verify slr]
    #set dna_read 1
    #set slr SLR1

    set plram           [dict get $config wizard_actual_config cu_configuration plram_selection $slr]
    regexp {^SLR([0-9])$} $slr -> slr_idx

    set cu_cfg {}
    dict set cu_cfg kernel_name         krnl_verify
    dict set cu_cfg kernel_inst         [dict get $cu_cfg kernel_name]_1
    dict set cu_cfg kernel_xo           [file join [dict get $config kernel_xo_dir]  [dict get $cu_cfg kernel_name].xo]
    dict set cu_cfg kernel_xml          [file join [dict get $config kernel_xml_dir] [dict get $cu_cfg kernel_name].xml]
    dict set cu_cfg krnl_mode           $C_KRNL_MODE_VERIFY
    dict set cu_cfg slr_idx             $slr_idx
    dict set cu_cfg sp_m00_axi          $plram
    dict set cu_cfg dna_read            $dna_read

    log_message $config {GEN_CU_CONFIG-2} [list {VERIFY} $cu_cfg]; # Report configuration
    lappend cu_cfg_list $cu_cfg


    if {[llength $cu_cfg_list] == 0} {
        log_message $config {GEN_CU_CONFIG-3} [list {VERIFY}]; # Report no CU
    } else {
        log_message $config {GEN_CU_CONFIG-4} [list [llength $cu_cfg_list] {VERIFY}]; # Report number of configurations
        if {[dict exists $config cu_config]} {
            set cu_cfg_list [concat $cu_cfg_list [dict get $config cu_config]]
        }
        dict set config cu_config $cu_cfg_list
    }
}


proc gen_cu_config { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    log_message $config {GEN_CU_CONFIG-5}; # Start
    configure_verify_cu     config
    configure_memory_cu     config
    configure_power_cu      config
    configure_gt_mac_cu     config
    configure_gt_lpbk_cu    config
    configure_gt_prbs_cu    config
    configure_gtf_prbs_cu   config
    configure_gtm_prbs_cu   config
    configure_gtyp_prbs_cu  config

    set num_cu_config [llength [dict_get_quiet $config {cu_config}]]
    if {([dict get $config C_INIT]) && ($num_cu_config == 0)} {
        log_message $config {GEN_CU_CONFIG-6} [list [dict get $config wizard_config_name]]; # ERROR: 0 cu selected
    } elseif {([dict get $config C_INIT] == 0) && ($num_cu_config == 1)} {
        log_message $config {GEN_CU_CONFIG-6} [list [dict get $config wizard_config_name]]; # ERROR: 0 cu selected
    } else {
        log_message $config {GEN_CU_CONFIG-7} [list $num_cu_config]; # Report number of config
    }
}


proc gen_aie_config { config_ref cfg_ref aie_type aie_geometry_slr aie_utilization_slr } {
    # TODO add support of AIE tiles in INVALID

    upvar 1 $config_ref config; # Dictionary passed as ref.
    upvar 1 $cfg_ref    cu_cfg

    set aie_freq [dict_get_quiet $config {wizard_actual_config platform aie freq}]
    if {$aie_freq == {default}} {
        if {$aie_type == {AIE_ML_CORE}} {
            set aie_freq 1050
        } else {
            set aie_freq 1250
        }
    }
    set aie_control [dict_get_quiet $config {wizard_actual_config platform aie control}]
    if {$aie_control == {default}} {
        set aie_control pl
    }

    set aie_status [dict_get_quiet $config {wizard_actual_config platform aie status}]
    if {$aie_status == {default}} {
        set aie_status pl
    }

    if {$aie_type == {AIE_ML_CORE}} {
        if {$aie_control == {pl}} {
            set aie_status none; # PL status not supported for AIE-ML when control from PL
        }
    }

    set slr_idx [dict get $cu_cfg slr_idx]

    set aie_krnl_cnt        0
    set util_cnt            0
    set x_dir               {-increasing}
    set TILE_CONSTRAINTS    {}

    # When AIE control is in PS, make sure 2 AIE tiles are used if utilisation is set to 1% (1 tile for control)
    set Y_SORTED            [lsort -integer [dict keys $aie_geometry_slr]]
    foreach {y} $Y_SORTED {
        set X_SORTED [lsort $x_dir -integer [dict get $aie_geometry_slr $y]]
        foreach {x} $X_SORTED {
            if {$util_cnt < $aie_utilization_slr} {
                lappend TILE_CONSTRAINTS "\t\t\"G.krnl\[$aie_krnl_cnt\]\":\{\"tile\":\{\"column\": $x,\"row\":$y\}\}"
                incr aie_krnl_cnt
            }
            incr util_cnt
            if {$util_cnt == 100} {
                set util_cnt 0
            }
        }
        if {$x_dir == {-increasing}} {
            set x_dir {-decreasing}
        } elseif {$x_dir == {-decreasing}} {
            set x_dir {-increasing}
        }
    }

    set     AIE_CONSTRAINTS_JSON {}
    lappend AIE_CONSTRAINTS_JSON "\{"
    lappend AIE_CONSTRAINTS_JSON "\t\"NodeConstraints\" : \{"
    for {set ii 0} {$ii < [llength $TILE_CONSTRAINTS]} {incr ii} {
        set tmp [lindex $TILE_CONSTRAINTS $ii]
        if {$ii < [expr [llength $TILE_CONSTRAINTS] - 1]} {
            append tmp ","
        }
        lappend AIE_CONSTRAINTS_JSON $tmp
    }
    lappend AIE_CONSTRAINTS_JSON "\t\}"
    lappend AIE_CONSTRAINTS_JSON "\}"

    set aie_constraints_json [file join [dict get $config run_dir] aie_constraints_SLR${slr_idx}.json]
    log_message $config {GEN_CU_CONFIG-9} [list $aie_constraints_json]
    write_file $aie_constraints_json [join $AIE_CONSTRAINTS_JSON "\n"]

    dict set cu_cfg aie_constraints_json   $aie_constraints_json
    dict set cu_cfg num_aie_used           [llength $TILE_CONSTRAINTS]
    dict set cu_cfg aie_type               $aie_type
    dict set cu_cfg aie_freq               $aie_freq
    dict set cu_cfg aie_control            $aie_control
    dict set cu_cfg aie_status             $aie_status
}
