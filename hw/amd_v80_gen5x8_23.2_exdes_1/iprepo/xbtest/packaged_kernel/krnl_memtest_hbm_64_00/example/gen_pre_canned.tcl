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

# Script that generates pre-canned tests

proc gen_pre_canned_file_path { config test } {
    return [file join [dict get $config pre_canned_dir] ${test}.json]
}

proc append_top_comment { TEST_JSON_REF TAB next } {
    upvar 1 $TEST_JSON_REF TEST_JSON
    set TAB_1 "  "
    lappend TEST_JSON "$TAB[string2json {comment}]: \["
    lappend TEST_JSON "$TAB$TAB_1[string2json {This is an example of test JSON file}],"
    lappend TEST_JSON "$TAB$TAB_1[string2json {You can use this example as template for your own tests}],"
    lappend TEST_JSON "$TAB$TAB_1[string2json {Please refer to the User Guide for how to define or add/remove testcases}],"
    lappend TEST_JSON "$TAB$TAB_1[string2json {Comments can be added or removed anywhere in test JSON file}]"
    lappend TEST_JSON "$TAB\]$next"
}

proc append_multi_gt_comment { TEST_JSON_REF TAB next } {
    upvar 1 $TEST_JSON_REF TEST_JSON
    set TAB_1 "  "
    lappend TEST_JSON "$TAB[string2json {comment}]: \["
    lappend TEST_JSON "$TAB$TAB_1[string2json {Make sure power duration is bigger than the total duration gt tests}],"
    lappend TEST_JSON "$TAB$TAB_1[string2json {reset gt only after power toggle rate is stable}]"
    lappend TEST_JSON "$TAB\]$next"
}
############################################################################################################

proc gen_pre_canned_verify { config_ref } {
    # Dictionary passed as ref.
    upvar 1 $config_ref config

    set TAB_1 "  "

    set test    verify
    set file    [gen_pre_canned_file_path $config $test]

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {}
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_power { config_ref } {
    variable C_KRNL_MODE_POWER
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"

    set test    power
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set is_power_cu false
    set use_aie 0
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_POWER} {
            set is_power_cu true
            set krnl_use_aie [dict get $cu_cfg use_aie]
            if {$krnl_use_aie} {
                set use_aie 1
            }
        }
    }
    if {!$is_power_cu} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no power CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set duration 100
    set toggle_rates {5 10 15 20}

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {power}]: \{"
    if {$use_aie} {
    lappend TEST_JSON           "$TAB_3[string2json {comment}]: [string2json {Power CU uses resources in PL and AIE}],"
    }
    lappend TEST_JSON           "$TAB_3[string2json {global_config}]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {test_sequence}]: \["
    set ii      0
    set ii_max [expr [llength $toggle_rates] - 1]
    foreach toggle_rate $toggle_rates {
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: $duration,"
    lappend TEST_JSON                       "$TAB_6[string2json {toggle_rate}]: $toggle_rate"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
        incr ii
    }
    lappend TEST_JSON               "$TAB_4\]"
    lappend TEST_JSON           "$TAB_3\}"
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_memory { config_ref target } {
    variable C_KRNL_MODE_MEMORY
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test memory
    if {$target == {host}} {
        set test ${test}_host
    }
    set file [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set memory_names {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_MEMORY} {
            set memory_name [dict get $cu_cfg memory_name]
            if {[dict get $cu_cfg target] == $target} {
                if {$memory_name ni $memory_names} {
                    lappend memory_names $memory_name
                }
            }
        }
    }

    if {$memory_names == {}} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no $target memory CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set duration 20
    set modes {alternate_wr_rd only_wr only_rd simultaneous_wr_rd}

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {memory}]: \{"
    set ii      0
    set ii_max [expr [llength $memory_names] - 1]
    foreach memory_name $memory_names {
    lappend TEST_JSON           "$TAB_3[string2json $memory_name]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
    set jj      0
    set jj_max [expr [llength $modes] - 1]
    foreach mode $modes {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
        if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
        incr jj
    }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_mmio { config_ref } {
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"

    set test    mmio
    set file    [gen_pre_canned_file_path $config $test]

    if {![dict get $config wizard_actual_config platform mmio_support]} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "platform does not support MMIO test"]; # n/a
        return
    }
    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set duration 10

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {mmio}]: \{"
    lappend TEST_JSON           "$TAB_3[string2json {global_config}]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {test_sequence}]: \["
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: $duration"
    lappend TEST_JSON                   "$TAB_5\}"
    lappend TEST_JSON               "$TAB_4\]"
    lappend TEST_JSON           "$TAB_3\}"
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_dma { config_ref } {
    variable C_KRNL_MODE_MEMORY
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"

    set test    dma
    set file    [gen_pre_canned_file_path $config $test]

    if {![dict get $config wizard_actual_config platform dma_support]} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "platform does not support DMA"]; # n/a
        return
    }
    if {[dict get $config wizard_actual_config platform is_nodma]} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "targeting NoDMA platform"]; # n/a
        return
    }

    set target  board

    # Check is test applicable
    set memory_names {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_MEMORY} {
            set memory_name [dict get $cu_cfg memory_name]
            if {[dict get $cu_cfg target] == $target} {
                if {$memory_name ni $memory_names} {
                    lappend memory_names $memory_name
                }
            }
        }
    }

    if {$memory_names == {}} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no $target memory CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set duration 10

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {dma}]: \{"
    lappend TEST_JSON           "$TAB_3[string2json {global_config}]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {test_sequence}]: \["
    set ii      0
    set ii_max [expr [llength $memory_names] - 1]
    foreach memory_name $memory_names {
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: $duration,"
    lappend TEST_JSON                       "$TAB_6[string2json {target}]: [string2json $memory_name]"
    lappend TEST_JSON                   "$TAB_5\}$next"
        incr ii
    }
    lappend TEST_JSON               "$TAB_4\]"
    lappend TEST_JSON           "$TAB_3\}"
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_p2p_card { config_ref } {
    variable C_KRNL_MODE_MEMORY
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"

    set test    p2p_card
    set file    [gen_pre_canned_file_path $config $test]

    if {![dict get $config wizard_actual_config platform p2p_support]} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "platform does not support P2P"]; # n/a
        return
    }
    if {[dict get $config wizard_actual_config platform is_nodma]} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "targeting NoDMA platform"]; # n/a
        return
    }

    set target  board

    # Check is test applicable
    set memory_names {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_MEMORY} {
            set memory_name [dict get $cu_cfg memory_name]
            if {[dict get $cu_cfg target] == $target} {
                if {$memory_name ni $memory_names} {
                    lappend memory_names $memory_name
                }
            }
        }
    }

    if {$memory_names == {}} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no $target memory CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set duration 10

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {p2p_card}]: \{"
    lappend TEST_JSON           "$TAB_3[string2json {global_config}]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {test_sequence}]: \["
    set ii      0
    set ii_max [expr [llength $memory_names] - 1]
    foreach memory_name $memory_names {
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: $duration,"
    lappend TEST_JSON                       "$TAB_6[string2json {source}]: [string2json $memory_name]"
    lappend TEST_JSON                   "$TAB_5\}$next"
        incr ii
    }
    lappend TEST_JSON               "$TAB_4\]"
    lappend TEST_JSON           "$TAB_3\}"
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_p2p_nvme { config_ref } {
    variable C_KRNL_MODE_MEMORY
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"

    set test    p2p_nvme
    set file    [gen_pre_canned_file_path $config $test]

    if {![dict get $config wizard_actual_config platform p2p_support]} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "platform does not support P2P"]; # n/a
        return
    }

    set target  board

    # Check is test applicable
    set memory_names {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_MEMORY} {
            set memory_name [dict get $cu_cfg memory_name]
            if {[dict get $cu_cfg target] == $target} {
                if {$memory_name ni $memory_names} {
                    lappend memory_names $memory_name
                }
            }
        }
    }

    if {$memory_names == {}} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no $target memory CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set duration 10

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {p2p_nvme}]: \{"
    lappend TEST_JSON           "$TAB_3[string2json {global_config}]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {test_sequence}]: \["
    set ii      0
    set ii_max [expr [llength $memory_names] - 1]
    foreach memory_name $memory_names {
        if {$ii < $ii_max} { set next {,} } else { set next {} }

            if {![dict get $config wizard_actual_config platform is_nodma]} {
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: $duration,"
    lappend TEST_JSON                       "$TAB_6[string2json {source}]: [string2json $memory_name]"
    lappend TEST_JSON                   "$TAB_5\},"
            }
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: $duration,"
    lappend TEST_JSON                       "$TAB_6[string2json {target}]: [string2json $memory_name]"
    lappend TEST_JSON                   "$TAB_5\}$next"
        incr ii
    }
    lappend TEST_JSON               "$TAB_4\]"
    lappend TEST_JSON           "$TAB_3\}"
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_gt_mac { config_ref } {
    variable C_KRNL_MODE_GT_MAC
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gt_mac
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gt_macs {}
    set lanes_config {}
    set durations_modes {}
    set gt_rate_comment {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GT_MAC} {
            set gt_idx  [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {![dict exists $gt_macs $gt_idx]} {
                dict set gt_macs $gt_idx {}
                dict set gt_macs $gt_idx gt_rate $gt_rate

                dict set lanes_config $gt_idx {}
                for {set lane_idx 0} {$lane_idx < 4} {incr lane_idx} {
                    dict set lanes_config $gt_idx $lane_idx {}
                    dict set lanes_config $gt_idx $lane_idx source_addr [string2json {test_address}]; # Use test address for loopback config
                }

                if {$gt_rate == {10G_25G}} {
                    dict set durations_modes $gt_idx {1 conf_25gbe_c74_fec 1 clear_status 10 run 1 check_status 1 conf_10gbe_c74_fec 1 clear_status 10 run 1 check_status}
                } elseif {$gt_rate == {10G}} {
                    dict set gt_rate_comment $gt_idx "GT\[$gt_idx\] only supports 10GbE rate"
                    dict set durations_modes $gt_idx {1 conf_10gbe_c74_fec 1 clear_status 10 run 1 check_status}
                } elseif {$gt_rate == {25G}} {
                    dict set gt_rate_comment $gt_idx "GT\[$gt_idx\] only supports 25GbE rate"
                    dict set durations_modes $gt_idx {1 conf_25gbe_c74_fec 1 clear_status 10 run 1 check_status}
                }
            }
        }
    }
    if {[dict size $gt_macs] == 0} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GT_MAC CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gt_mac}]: \{"
    set ii      0
    set ii_max [expr [dict size $gt_macs] - 1]
    foreach gt_idx [dict keys $gt_macs] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
        if {[dict exists $gt_rate_comment $gt_idx]} {
    lappend TEST_JSON               "$TAB_4[string2json {comment}]: [string2json [dict get $gt_rate_comment $gt_idx]],"
        }
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {match_tx_rx}]: true,"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength [dict get $durations_modes $gt_idx]] / 2 - 1]
    foreach {duration mode} [dict get $durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
        if {$lanes_config != {}} { set next {,} } else { set next {} }
    lappend TEST_JSON               "$TAB_4\}$next"
        if {$lanes_config != {}} {
    lappend TEST_JSON               "$TAB_4[string2json {lane_config}]: \{"
            set lane_config [dict get $lanes_config $gt_idx]
            set kk      0
            set kk_max [expr [llength [dict keys $lane_config]] - 1]
            foreach {lane_idx cfg} $lane_config {
    lappend TEST_JSON                   "$TAB_5[string2json $lane_idx]: \{"
                set ll      0
                set ll_max [expr [llength [dict keys $cfg]] - 1]
                foreach {key val} $cfg {
                    if {$ll < $ll_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6[string2json $key]: $val$next"
                    incr ll
                }
                if {$kk < $kk_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
                incr kk
            }
    lappend TEST_JSON               "$TAB_4\}"
        }
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_switch { config_ref rate } {
    variable C_KRNL_MODE_GT_MAC
    # Dictionary passed as ref.
    upvar 1 $config_ref config

    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    if {($rate != {10gbe}) && ($rate != {25gbe})} {
        return
    }

    set test    switch_$rate
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gt_macs {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GT_MAC} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {$gt_idx ni $gt_macs} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_macs $gt_idx
                } elseif {($gt_rate == {10G}) && ($rate == {10gbe})} {
                    lappend gt_macs $gt_idx
                } elseif {($gt_rate == {25G}) && ($rate == {25gbe})} {
                    lappend gt_macs $gt_idx
                }
            }
        }
    }
    if {$gt_macs == {}} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GT_MAC CU found"]; # n/a
        return
    }

    set mac_addr_comment [list]
    set mac_addresses_available_all [dict_get_quiet $config {wizard_actual_config platform mac_addresses_available}]
    set mac_addresses_available $mac_addresses_available_all
    if {$mac_addresses_available != {default}} {
        if {$mac_addresses_available == 0} {
            log_message $config {GEN_PRE_CANNED-3} [list $test "no MAC address available"]; # n/a
            return
        } elseif {$mac_addresses_available < [expr [llength $gt_macs] * 4]} {
            set num_pairs               [expr $mac_addresses_available / 2]
            set mac_addresses_available [expr $num_pairs * 2]
            if {$num_pairs == 0} {
                log_message $config {GEN_PRE_CANNED-3} [list $test "at least 2 MAC addresses are required"]; # n/a
                return
            }
            lappend mac_addr_comment {Not enough board MAC addresses are available. Some GT lanes are disabled in this pre-canned test.}
			set gt_macs_unused {}
            if {$num_pairs < [llength $gt_macs]} {
                # Need at least 2 MAC addresses per GT, reduce number of GTs used
                set gt_macs_unused	[lrange $gt_macs $num_pairs end]
                set gt_macs         [lrange $gt_macs 0 [expr $num_pairs - 1]]
            }
			set gt_macs_str {}
			foreach gt_idx $gt_macs {
				lappend gt_macs_str "GT\[$gt_idx\]"
			}
			set gt_macs_unused_str {}
			foreach gt_idx $gt_macs_unused {
				lappend gt_macs_unused_str "GT\[$gt_idx\]"
			}
			lappend mac_addr_comment "GTs tested with switch: [join $gt_macs_str ", "]."
			if {[llength $gt_macs_unused_str] > 0} {
				lappend mac_addr_comment "GTs not tested: [join $gt_macs_unused_str ", "]."
			}
            if {$mac_addresses_available_all != $mac_addresses_available} {
                lappend mac_addr_comment "Using $mac_addresses_available of $mac_addresses_available_all MAC address(es) available on card."
            }
        } elseif {$mac_addresses_available >= [expr [llength $gt_macs] * 4]} {
            set mac_addresses_available {default}; # Enough mac addresses for all lanes of all GTs, do not change the default behaviour
        }
    }

    set lanes_config {}
    # Set tx mapping for switch config by default
    foreach gt_idx $gt_macs {
        dict set lanes_config $gt_idx { \
            0 {tx_mapping 1} \
            1 {tx_mapping 0} \
            2 {tx_mapping 3} \
            3 {tx_mapping 2} \
        }
    }
    if {$mac_addresses_available != {default}} {

        set addr_idx 0
        foreach gt_idx $gt_macs {
            for {set lane_idx 0} {$lane_idx < 2} {incr lane_idx} {
                if {$addr_idx < $mac_addresses_available} {
                    dict set lanes_config $gt_idx $lane_idx source_addr [string2json board_mac_addr_$addr_idx]; # Manually assign the board MAC address evenly between GTs
                } else {
                    dict set lanes_config $gt_idx $lane_idx [dict create disable_lane true]; # disable lanes that cannot be assigned with mac address, remove tx_mapping
                }
                incr addr_idx
            }
        }
        foreach gt_idx $gt_macs {
            for {set lane_idx 2} {$lane_idx < 4} {incr lane_idx} {
                if {$addr_idx < $mac_addresses_available} {
                    dict set lanes_config $gt_idx $lane_idx source_addr [string2json board_mac_addr_$addr_idx]; # Manually assign the board MAC address evenly between GTs
                } else {
                    dict set lanes_config $gt_idx $lane_idx [dict create disable_lane true]; # disable lanes that cannot be assigned with mac address, remove tx_mapping
                }
                incr addr_idx
            }
        }
    }

    if {$rate == {10gbe}} {
        set durations_modes {1 conf_10gbe_no_fec 1 clear_status 60 run 1 check_status}
    } elseif {$rate == {25gbe}} {
        set durations_modes {1 conf_25gbe_c74_fec 1 clear_status 60 run 1 check_status}
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gt_mac}]: \{"
    if {[llength $mac_addr_comment] > 0} {
        foreach comment $mac_addr_comment {
    lappend TEST_JSON           "$TAB_3[string2json {comment}]: [string2json $comment],"
        }
    }
    set ii      0
    set ii_max [expr [llength $gt_macs] - 1]
    foreach gt_idx $gt_macs {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {match_tx_rx}]: true,"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength $durations_modes] / 2 - 1]
        foreach {duration mode} $durations_modes {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
        if {$lanes_config != {}} { set next {,} } else { set next {} }
    lappend TEST_JSON               "$TAB_4\}$next"
        if {$lanes_config != {}} {
    lappend TEST_JSON               "$TAB_4[string2json {lane_config}]: \{"
            set lane_config [dict get $lanes_config $gt_idx]
            set kk      0
            set kk_max [expr [llength [dict keys $lane_config]] - 1]
            foreach {lane_idx cfg} $lane_config {
    lappend TEST_JSON                   "$TAB_5[string2json $lane_idx]: \{"
                set ll      0
                set ll_max [expr [llength [dict keys $cfg]] - 1]
                foreach {key val} $cfg {
                    if {$ll < $ll_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6[string2json $key]: $val$next"
                    incr ll
                }
                if {$kk < $kk_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
                incr kk
            }
    lappend TEST_JSON               "$TAB_4\}"
        }
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_gt_mac_port_to_port { config_ref } {
    variable C_KRNL_MODE_GT_MAC
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gt_mac_port_to_port
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gt_macs_25g {}
    set gt_macs_10g {}
    set lanes_config {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GT_MAC} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {$gt_idx ni $gt_macs_25g} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_macs_25g $gt_idx
                } elseif {$gt_rate == {25G}} {
                    lappend gt_macs_25g $gt_idx
                }
            }
            if {$gt_idx ni $gt_macs_10g} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_macs_10g $gt_idx
                } elseif {$gt_rate == {10G}} {
                    lappend gt_macs_10g $gt_idx
                }
            }

            dict set lanes_config $gt_idx {}
            for {set lane_idx 0} {$lane_idx < 4} {incr lane_idx} {
                dict set lanes_config $gt_idx $lane_idx [dict create source_addr [string2json {test_address}]]; # Use test address for loopback config
            }
        }
    }

    set rate 25gbe
    set gt_macs $gt_macs_25g
    if {[llength $gt_macs] < 2} {
        # Not enough GT_MAC CU supporting 25GbE, look for GT_MAC CU supporting 10GbE
        set rate 10gbe
        set gt_macs $gt_macs_10g
    }

    if {[llength $gt_macs] < 2} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "number of GT_MAC CUs found supporting same rate is less than 2"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    # Only use 2 GT
    set gt_macs [lrange $gt_macs 0 1]

    set gt_mac_port_map {}
    dict set gt_mac_port_map [lindex $gt_macs 0] [lindex $gt_macs 1]

    if {$rate == {10gbe}} {
        set durations_modes {1 conf_10gbe_no_fec 1 clear_status 60 run 1 check_status}
    } elseif {$rate == {25gbe}} {
        set durations_modes {1 conf_25gbe_c74_fec 1 clear_status 60 run 1 check_status}
    }


    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gt_mac}]: \{"
    set ii      0
    set ii_max [expr [llength [dict keys $gt_mac_port_map]] - 1]
    foreach {src dst} $gt_mac_port_map {
    lappend TEST_JSON           "$TAB_3[string2json $src]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {match_tx_rx}]: true,"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength $durations_modes] / 2 - 1]
        foreach {duration mode} $durations_modes {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
        if {$lanes_config != {}} { set next {,} } else { set next {} }
    lappend TEST_JSON               "$TAB_4\}$next"
        if {$lanes_config != {}} {
    lappend TEST_JSON               "$TAB_4[string2json {lane_config}]: \{"
            set lane_config [dict get $lanes_config $src]
            set kk      0
            set kk_max [expr [llength [dict keys $lane_config]] - 1]
            foreach {lane_idx cfg} $lane_config {
    lappend TEST_JSON                   "$TAB_5[string2json $lane_idx]: \{"
                set ll      0
                set ll_max [expr [llength [dict keys $cfg]] - 1]
                foreach {key val} $cfg {
                    if {$ll < $ll_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6[string2json $key]: $val$next"
                    incr ll
                }
                if {$kk < $kk_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
                incr kk
            }
    lappend TEST_JSON               "$TAB_4\}"
        }
    lappend TEST_JSON           "$TAB_3\},"
    lappend TEST_JSON           "$TAB_3[string2json $dst]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {mac_to_mac_connection}]: $src"
        if {$lanes_config != {}} { set next {,} } else { set next {} }
    lappend TEST_JSON               "$TAB_4\}$next"
        if {$lanes_config != {}} {
    lappend TEST_JSON               "$TAB_4[string2json {lane_config}]: \{"
            set lane_config [dict get $lanes_config $dst]
            set kk      0
            set kk_max [expr [llength [dict keys $lane_config]] - 1]
            foreach {lane_idx cfg} $lane_config {
    lappend TEST_JSON                   "$TAB_5[string2json $lane_idx]: \{"
                set ll      0
                set ll_max [expr [llength [dict keys $cfg]] - 1]
                foreach {key val} $cfg {
                    if {$ll < $ll_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6[string2json $key]: $val$next"
                    incr ll
                }
                if {$kk < $kk_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
                incr kk
            }
    lappend TEST_JSON               "$TAB_4\}"
        }
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_gt_mac_lpbk { config_ref } {
    variable C_KRNL_MODE_GT_MAC
    variable C_KRNL_MODE_GT_LPBK
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gt_mac_lpbk
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    # By default, look for GT_MAC/GT_LPBK CU supporting 25GbE
    set gt_macs_25g {}
    set gt_macs_10g {}
    set gt_lpbks_25g {}
    set gt_lpbks_10g {}
    set lanes_config {}
    foreach cu_cfg [dict get $config cu_config] {
        set krnl_mode [dict get $cu_cfg krnl_mode]
        if {$krnl_mode == $C_KRNL_MODE_GT_LPBK} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {$gt_idx ni $gt_lpbks_25g} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_lpbks_25g $gt_idx
                } elseif {$gt_rate == {25G}} {
                    lappend gt_lpbks_25g $gt_idx
                }
            }
            if {$gt_idx ni $gt_lpbks_10g} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_lpbks_10g $gt_idx
                } elseif {$gt_rate == {10G}} {
                    lappend gt_lpbks_10g $gt_idx
                }
            }
        } elseif {$krnl_mode == $C_KRNL_MODE_GT_MAC} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {$gt_idx ni $gt_macs_25g} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_macs_25g $gt_idx
                } elseif {$gt_rate == {25G}} {
                    lappend gt_macs_25g $gt_idx
                }
            }
            if {$gt_idx ni $gt_macs_10g} {
                if {$gt_rate == {10G_25G}} {
                    lappend gt_macs_10g $gt_idx
                } elseif {$gt_rate == {10G}} {
                    lappend gt_macs_10g $gt_idx
                }
            }
            dict set lanes_config $gt_idx {}
            for {set lane_idx 0} {$lane_idx < 4} {incr lane_idx} {
                dict set lanes_config $gt_idx $lane_idx {}
                dict set lanes_config $gt_idx $lane_idx source_addr [string2json {test_address}]; # Use test address for loopback config
            }
        }
    }

    set rate 25gbe
    set gt_macs $gt_macs_25g
    set gt_lpbks $gt_lpbks_25g
    if {($gt_macs == {}) || ($gt_lpbks == {})} {
        # Not enough GT_MAC/GT_LPBK CU supporting 25GbE, look for GT_MAC/GT_LPBK CU supporting 10GbE
        set rate 10gbe
        set gt_macs $gt_macs_10g
        set gt_lpbks $gt_lpbks_10g
    }

    if {($gt_macs == {}) || ($gt_lpbks == {})} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GT_MAC CU and GT_LPBK CU found supporting same rate"]; # n/a
        return
    }

    # Only use 2 GT if more than 2 GT
    set gt_macs  [lindex $gt_macs 0]
    set gt_lpbks [lindex $gt_lpbks 0]

    if {$rate == {10gbe}} {
        set gt_macs_durations_modes     {1 conf_10gbe_no_fec 1 clear_status 60 run 1 check_status}
        set gt_lpbks_durations_modes    {1 conf_10gbe_no_fec}
    } elseif {$rate == {25gbe}} {
        set gt_macs_durations_modes     {1 conf_25gbe_no_fec 1 clear_status 60 run 1 check_status}
        set gt_lpbks_durations_modes    {1 conf_25gbe_no_fec}
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gt_mac}]: \{"
    set ii      0
    set ii_max [expr [llength $gt_macs] - 1]
    foreach gt_idx $gt_macs {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {match_tx_rx}]: true,"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength $gt_macs_durations_modes] / 2 - 1]
        foreach {duration mode} $gt_macs_durations_modes {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
        if {$lanes_config != {}} { set next {,} } else { set next {} }
    lappend TEST_JSON               "$TAB_4\}$next"
        if {$lanes_config != {}} {
    lappend TEST_JSON               "$TAB_4[string2json {lane_config}]: \{"
            set lane_config [dict get $lanes_config $gt_idx]
            set kk      0
            set kk_max [expr [llength [dict keys $lane_config]] - 1]
            foreach {lane_idx cfg} $lane_config {
    lappend TEST_JSON                   "$TAB_5[string2json $lane_idx]: \{"
                set ll      0
                set ll_max [expr [llength [dict keys $cfg]] - 1]
                foreach {key val} $cfg {
                    if {$ll < $ll_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6[string2json $key]: $val$next"
                    incr ll
                }
                if {$kk < $kk_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
                incr kk
            }
    lappend TEST_JSON               "$TAB_4\}"
        }
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\},"

    lappend TEST_JSON       "$TAB_2[string2json {gt_lpbk}]: \{"
    set ii      0
    set ii_max [expr [llength $gt_lpbks] - 1]
    foreach gt_idx $gt_lpbks {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength $gt_lpbks_durations_modes] / 2 - 1]
        foreach {duration mode} $gt_lpbks_durations_modes {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

proc gen_pre_canned_gt_prbs { config_ref } {
    variable C_KRNL_MODE_GT_PRBS
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gt_prbs
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gt_prbss {}
    set durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GT_PRBS} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {![dict exists $gt_prbss $gt_idx]} {
                dict set gt_prbss $gt_idx {}
                dict set gt_prbss $gt_idx gt_rate $gt_rate

                if {$gt_rate == {10G_25G}} {
                    dict set durations_modes $gt_idx {1 conf_25gbe 1 tx_rx_rst 1 clear_status 60 run 1 check_status 1 conf_10gbe 1 tx_rx_rst 1 clear_status 60 run 1 check_status}
                } elseif {$gt_rate == {10G}} {
                    dict set durations_modes $gt_idx {1 conf_10gbe 1 tx_rx_rst 1 clear_status 60 run 1 check_status}
                } elseif {$gt_rate == {25G}} {
                    dict set durations_modes $gt_idx {1 conf_25gbe 1 tx_rx_rst 1 clear_status 60 run 1 check_status}
                }
            }
        }
    }
    if {[dict size $gt_prbss] == 0} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GT_PRBS CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gt_prbs}]: \{"
    set ii      0
    set ii_max [expr [dict size $gt_prbss] - 1]
    foreach gt_idx [dict keys $gt_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength [dict get $durations_modes $gt_idx]] / 2 - 1]
        foreach {duration mode} [dict get $durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

proc gen_pre_canned_gtf_prbs { config_ref } {
    variable C_KRNL_MODE_GTF_PRBS
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gtf_prbs
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gtf_prbss {}
    set gtf_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GTF_PRBS} {
            dict set gtf_prbss "default" {}
            dict set gtf_prbss "default" gt_rate "10G"
            dict set gtf_prbss_durations_modes "default" [list 20 conf_gt 1 tx_rx_rst 1 clear_status 60 run 1 check_status]
            break
        }
    }

    if {[dict size $gtf_prbss] == 0} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GTF_PRBS CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gtf_prbs}]: \{"
    set ii      0
    set ii_max [expr [dict size $gtf_prbss] - 1]
    foreach gt_idx [dict keys $gtf_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength [dict get $gtf_prbss_durations_modes $gt_idx]] / 2 - 1]
        foreach {duration mode} [dict get $gtf_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

proc gen_pre_canned_gtm_prbs { config_ref } {
    variable C_KRNL_MODE_GTM_PRBS
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gtm_prbs
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gtm_prbss {}
    set gtm_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GTM_PRBS} {
            dict set gtm_prbss "default" {}
            dict set gtm_prbss "default" gt_rate "56G"
            dict set gtm_prbss_durations_modes "default" [list 1 conf_gt 1 tx_rx_rst 1 rx_datapath_rst 1 clear_status 60 run 1 check_status]
            break
        }
    }

    if {[dict size $gtm_prbss] == 0} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GTM_PRBS CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gtm_prbs}]: \{"
    set ii      0
    set ii_max [expr [dict size $gtm_prbss] - 1]
    foreach gt_idx [dict keys $gtm_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength [dict get $gtm_prbss_durations_modes $gt_idx]] / 2 - 1]
        foreach {duration mode} [dict get $gtm_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

proc gen_pre_canned_gtyp_prbs { config_ref } {
    variable C_KRNL_MODE_GTYP_PRBS
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    gtyp_prbs
    set file    [gen_pre_canned_file_path $config $test]

    # Check is test applicable
    set gtyp_prbss {}
    set gtyp_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GTYP_PRBS} {
            dict set gtyp_prbss "default" {}
            dict set gtyp_prbss "default" gt_rate "32G"
            dict set gtyp_prbss_durations_modes "default" [list 1 conf_gt 1 tx_rx_rst 1 rx_datapath_rst 1 clear_status 60 run 1 check_status]
            break
        }
    }

    if {[dict size $gtyp_prbss] == 0} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GTYP_PRBS CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    lappend TEST_JSON       "$TAB_2[string2json {gtyp_prbs}]: \{"
    set ii      0
    set ii_max [expr [dict size $gtyp_prbss] - 1]
    foreach gt_idx [dict keys $gtyp_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
        set jj      0
        set jj_max [expr [llength [dict get $gtyp_prbss_durations_modes $gt_idx]] / 2 - 1]
        foreach {duration mode} [dict get $gtyp_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
        }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
        if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
        incr ii
    }
    lappend TEST_JSON       "$TAB_2\}"
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned_stress { config_ref } {
    variable C_KRNL_MODE_POWER
    variable C_KRNL_MODE_MEMORY
    variable C_KRNL_MODE_GT_MAC
    variable C_KRNL_MODE_GT_PRBS
    variable C_KRNL_MODE_GTF_PRBS
    variable C_KRNL_MODE_GTM_PRBS
    variable C_KRNL_MODE_GTYP_PRBS
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set test    stress
    set file    [gen_pre_canned_file_path $config $test]

    set target  board

    # Check is test applicable
    set is_power_cu false
    set use_aie 0
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_POWER} {
            set is_power_cu true
            set krnl_use_aie [dict get $cu_cfg use_aie]
            if {$krnl_use_aie} {
                set use_aie 1
            }
        }
    }

    set memory_names {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_MEMORY} {
            set memory_name [dict get $cu_cfg memory_name]
            if {[dict get $cu_cfg target] == $target} {
                if {$memory_name ni $memory_names} {
                    lappend memory_names $memory_name
                }
            }
        }
    }

    set gt_macs {}
    set gt_macs_durations_modes {}
    set gt_rate_comment {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GT_MAC} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {![dict exists $gt_macs $gt_idx]} {
                dict set gt_macs $gt_idx {}
                dict set gt_macs $gt_idx gt_rate $gt_rate

                if {$gt_rate == {10G_25G}} {
                    dict set gt_macs_durations_modes $gt_idx {1 conf_25gbe_c74_fec 1 clear_status 300 run 1 check_status}
                } elseif {$gt_rate == {10G}} {
                    dict set gt_rate_comment $gt_idx "GT\[$gt_idx\] only supports 10GbE rate"
                    dict set gt_macs_durations_modes $gt_idx {1 conf_10gbe_no_fec 1 clear_status 300 run 1 check_status}
                } elseif {$gt_rate == {25G}} {
                    dict set gt_rate_comment $gt_idx "GT\[$gt_idx\] only supports 25GbE rate"
                    dict set gt_macs_durations_modes $gt_idx {1 conf_25gbe_c74_fec 1 clear_status 300 run 1 check_status}
                }
            }
        }
    }

    set gt_macs_switch {}
    set gt_macs_lpbk   {}
    set mac_addr_comment [list]
    set mac_addresses_available_all [dict_get_quiet $config {wizard_actual_config platform mac_addresses_available}]
    set mac_addresses_available $mac_addresses_available_all
    if {$mac_addresses_available != {default}} {
        if {$mac_addresses_available < [expr [dict size $gt_macs] * 4]} {
            set num_pairs               [expr $mac_addresses_available / 2]
            set mac_addresses_available [expr $num_pairs * 2]

            lappend mac_addr_comment {Not enough board MAC addresses are available. Some GT lanes are disabled or used with loopback in this pre-canned test.}

            set i 0
            foreach gt_idx [dict keys $gt_macs] {
                # Need at least 2 MAC address per GT, set other GT without board MAC address available to test address for loopback
                if {$i < $num_pairs} {
                    lappend gt_macs_switch $gt_idx
                } else {
                    lappend gt_macs_lpbk $gt_idx
                }
                incr i
            }
			set gt_macs_switch_str {}
			foreach gt_idx $gt_macs_switch {
				lappend gt_macs_switch_str "GT\[$gt_idx\]"
			}
			set gt_macs_lpbk_str {}
			foreach gt_idx $gt_macs_lpbk {
				lappend gt_macs_lpbk_str "GT\[$gt_idx\]"
			}
            lappend mac_addr_comment "GTs tested with switch: [join $gt_macs_switch_str ", "]."
			if {[llength $gt_macs_lpbk_str] > 0} {
				lappend mac_addr_comment "GTs tested with loopback: [join $gt_macs_lpbk_str ", "]."
			}
            if {$mac_addresses_available_all != $mac_addresses_available} {
                lappend mac_addr_comment "Using $mac_addresses_available of $mac_addresses_available_all MAC address(es) available on card."
            }
        } elseif {$mac_addresses_available >= [expr [dict size $gt_macs] * 4]} {
            set mac_addresses_available {default}; # Enough mac addresses for all lanes of all GTs, do not change the default behaviour
        }
    }

    set lanes_config {}
    # default lane config when board address is used
    foreach gt_idx [dict keys $gt_macs] {
        dict set lanes_config $gt_idx {}
        dict set lanes_config $gt_idx 0 {tx_mapping 1}
        dict set lanes_config $gt_idx 1 {tx_mapping 0}
        dict set lanes_config $gt_idx 2 {tx_mapping 3}
        dict set lanes_config $gt_idx 3 {tx_mapping 2}
    }
    if {$mac_addresses_available != {default}} {
        set addr_idx 0
        foreach gt_idx $gt_macs_switch {
            # Use board address for lane 0 and 1 if possible
            for {set lane_idx 0} {$lane_idx < 2} {incr lane_idx} {
                if {$addr_idx < $mac_addresses_available} {
                    dict set lanes_config $gt_idx $lane_idx source_addr [string2json board_mac_addr_$addr_idx]; # Manually assign the board MAC address evenly between GTs
                } else {
                    dict set lanes_config $gt_idx $lane_idx {}
                    dict set lanes_config $gt_idx $lane_idx disable_lane true; # disable lanes that cannot be assigned with mac address
                }
                incr addr_idx
            }
        }

        # Use board address for lane 2 and 3 if possible
        foreach gt_idx $gt_macs_switch {
            for {set lane_idx 2} {$lane_idx < 4} {incr lane_idx} {
                if {$addr_idx < $mac_addresses_available} {
                    dict set lanes_config $gt_idx $lane_idx source_addr [string2json board_mac_addr_$addr_idx]; # Manually assign the board MAC address evenly between GTs
                } else {
                    dict set lanes_config $gt_idx $lane_idx [dict create disable_lane true]; # disable lanes that cannot be assigned with mac address
                }
                incr addr_idx
            }
        }
        # Use test address for GT without board MAC addresses
        foreach gt_idx $gt_macs_lpbk {
            for {set lane_idx 0} {$lane_idx < 4} {incr lane_idx} {
                dict set lanes_config $gt_idx $lane_idx [dict create source_addr [string2json {test_address}]]; # Use test address for loopback config
            }
        }
    }

    set gt_prbss {}
    set gt_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GT_PRBS} {
            set gt_idx [dict get $cu_cfg gt_idx]
            set gt_rate [dict get $cu_cfg gt_rate]
            if {![dict exists $gt_prbss $gt_idx]} {
                dict set gt_prbss $gt_idx {}
                dict set gt_prbss $gt_idx gt_rate $gt_rate

                if {$gt_rate == {10G_25G}} {
                    dict set gt_prbss_durations_modes $gt_idx {1 conf_25gbe 1 tx_rx_rst 1 clear_status 300 run 1 check_status}
                } elseif {$gt_rate == {10G}} {
                    dict set gt_prbss_durations_modes $gt_idx {1 conf_10gbe 1 tx_rx_rst 1 clear_status 300 run 1 check_status}
                } elseif {$gt_rate == {25G}} {
                    dict set gt_prbss_durations_modes $gt_idx {1 conf_25gbe 1 tx_rx_rst 1 clear_status 300 run 1 check_status}
                }
            }
        }
    }

    set gtf_prbss {}
    set gtf_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GTF_PRBS} {
            dict set gtf_prbss "default" {}
            dict set gtf_prbss "default" gt_rate "10G"
            dict set gtf_prbss_durations_modes "default" [list 20 conf_gt 1 tx_rx_rst 1 clear_status 250 run 1 check_status]
            break
        }
    }

    set gtm_prbss {}
    set gtm_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GTM_PRBS} {
            dict set gtm_prbss "default" {}
            dict set gtm_prbss "default" gt_rate "56G"
            dict set gtm_prbss_durations_modes "default" [list 1 conf_gt 1 tx_rx_rst 1 rx_datapath_rst 1 clear_status 250 run 1 check_status]
            break
        }
    }

    set gtyp_prbss {}
    set gtyp_prbss_durations_modes {}
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_GTYP_PRBS} {
            dict set gtyp_prbss "default" {}
            dict set gtyp_prbss "default" gt_rate "32G"
            dict set gtyp_prbss_durations_modes "default" [list 1 conf_gt 1 tx_rx_rst 1 rx_datapath_rst 1 clear_status 250 run 1 check_status]
            break
        }
    }

    if {!$is_power_cu && ($memory_names == {}) && ([dict size $gt_macs] == 0) && ([dict size $gt_prbss] == 0) && ([dict size $gtf_prbss] == 0) && ([dict size $gtm_prbss] == 0) && ([dict size $gtyp_prbss] == 0)} {
        log_message $config {GEN_PRE_CANNED-3} [list $test "no GT_MAC CU, no GT_PRBS CU, no GTF_PRBS CU, no GTM_PRBS CU, no GTYP_PRBS CU and no board memory CU and no power CU found"]; # n/a
        return
    }

    log_message $config {GEN_PRE_CANNED-2} [list $test $file]; # Start

    set     TEST_JSON {}
    lappend TEST_JSON "\{"
    append_top_comment TEST_JSON $TAB_1 {,}
    lappend TEST_JSON "$TAB_1[string2json {testcases}]: \{"
    if {$is_power_cu} {
    lappend TEST_JSON       "$TAB_2[string2json {power}]: \{"
    if {$use_aie} {
    lappend TEST_JSON           "$TAB_3[string2json {comment}]: [string2json {Power CU uses resources in PL and AIE}],"
    }
    lappend TEST_JSON           "$TAB_3[string2json {comment}]: [string2json {Update toggle rate according to your test environment}],"
    lappend TEST_JSON           "$TAB_3[string2json {global_config}]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {test_sequence}]: \["
    lappend TEST_JSON                   "$TAB_5\{"
    lappend TEST_JSON                       "$TAB_6[string2json {duration}]: 300,"
    lappend TEST_JSON                       "$TAB_6[string2json {toggle_rate}]: 10"
    lappend TEST_JSON                   "$TAB_5\}"
    lappend TEST_JSON               "$TAB_4\]"
    lappend TEST_JSON           "$TAB_3\}"
        if {([llength $memory_names] > 0) || ([dict size $gt_macs] > 0) || ([dict size $gt_prbss] > 0) || ([dict size $gtf_prbss] > 0) || ([dict size $gtm_prbss] > 0) || ([dict size $gtyp_prbss] > 0)} { set next {,} } else { set next {} }
    lappend TEST_JSON       "$TAB_2\}$next"
    }

    if { [llength $memory_names] > 0} {
    lappend TEST_JSON       "$TAB_2[string2json {memory}]: \{"
        set ii      0
        set ii_max [expr [llength $memory_names] - 1]
        foreach memory_name $memory_names {
    lappend TEST_JSON           "$TAB_3[string2json $memory_name]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: 300,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json {alternate_wr_rd}]"
    lappend TEST_JSON                       "$TAB_6\}"
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
            if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
            incr ii
        }
        if {([dict size $gt_macs] > 0) || ([dict size $gt_prbss] > 0) || ([dict size $gtf_prbss] > 0) || ([dict size $gtm_prbss] > 0) || ([dict size $gtyp_prbss] > 0)} { set next {,} } else { set next {} }
    lappend TEST_JSON       "$TAB_2\}$next"
    }


    if { [dict size $gt_prbss] > 0} {

    lappend TEST_JSON       "$TAB_2[string2json {gt_prbs}]: \{"
        set ii      0
        set ii_max [expr [dict size $gt_prbss] - 1]
        foreach gt_idx [dict keys $gt_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
            set jj      0
            set jj_max [expr [llength [dict get $gt_prbss_durations_modes $gt_idx]] / 2 - 1]
            foreach {duration mode} [dict get $gt_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
            }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
            if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
            incr ii
        }
        if {([dict size $gt_macs] > 0) || ([dict size $gtf_prbss] > 0)} { set next {,} } else { set next {} }
    lappend TEST_JSON       "$TAB_2\}$next"
    }

    if { [dict size $gtf_prbss] > 0} {
    lappend TEST_JSON       "$TAB_2[string2json {gtf_prbs}]: \{"
    append_multi_gt_comment TEST_JSON $TAB_3 {,}
        set ii      0
        set ii_max [expr [dict size $gtf_prbss] - 1]
        foreach gt_idx [dict keys $gtf_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
            set jj      0
            set jj_max [expr [llength [dict get $gtf_prbss_durations_modes $gt_idx]] / 2 - 1]
            foreach {duration mode} [dict get $gtf_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
            }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
            if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
            incr ii
        }
        if {([dict size $gt_macs] > 0) || ([dict size $gtm_prbss] > 0) || ([dict size $gtyp_prbss] > 0)} { set next {,} } else { set next {} }
    lappend TEST_JSON       "$TAB_2\}$next"
    }

    if { [dict size $gtm_prbss] > 0} {
    lappend TEST_JSON       "$TAB_2[string2json {gtm_prbs}]: \{"
    append_multi_gt_comment TEST_JSON $TAB_3 {,}
        set ii      0
        set ii_max [expr [dict size $gtm_prbss] - 1]
        foreach gt_idx [dict keys $gtm_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
            set jj      0
            set jj_max [expr [llength [dict get $gtm_prbss_durations_modes $gt_idx]] / 2 - 1]
            foreach {duration mode} [dict get $gtm_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
            }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
            if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
            incr ii
        }
        if {([dict size $gt_macs] > 0) || ([dict size $gtyp_prbss] > 0)} { set next {,} } else { set next {} }
    lappend TEST_JSON       "$TAB_2\}$next"
    }

    if { [dict size $gtyp_prbss] > 0} {
    lappend TEST_JSON       "$TAB_2[string2json {gtyp_prbs}]: \{"
    append_multi_gt_comment TEST_JSON $TAB_3 {,}
        set ii      0
        set ii_max [expr [dict size $gtyp_prbss] - 1]
        foreach gt_idx [dict keys $gtyp_prbss] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
            set jj      0
            set jj_max [expr [llength [dict get $gtyp_prbss_durations_modes $gt_idx]] / 2 - 1]
            foreach {duration mode} [dict get $gtyp_prbss_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
            if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
            incr jj
            }
    lappend TEST_JSON                   "$TAB_5\]"
    lappend TEST_JSON               "$TAB_4\}"
            if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
            incr ii
        }
        if {([dict size $gt_macs] > 0)} { set next {,} } else { set next {} }
    lappend TEST_JSON       "$TAB_2\}$next"
    }

    if { [dict size $gt_macs] > 0} {

    lappend TEST_JSON       "$TAB_2[string2json {gt_mac}]: \{"
    if {[llength $mac_addr_comment] > 0} {
        foreach comment $mac_addr_comment {
    lappend TEST_JSON           "$TAB_3[string2json {comment}]: [string2json $comment],"
        }
    }
        set ii      0
        set ii_max [expr [dict size $gt_macs] - 1]
        foreach gt_idx [dict keys $gt_macs] {
    lappend TEST_JSON           "$TAB_3[string2json $gt_idx]: \{"
        if {[dict exists $gt_rate_comment $gt_idx]} {
    lappend TEST_JSON               "$TAB_4[string2json {comment}]: [string2json [dict get $gt_rate_comment $gt_idx]],"
        }
    lappend TEST_JSON               "$TAB_4[string2json {global_config}]: \{"
    lappend TEST_JSON                   "$TAB_5[string2json {match_tx_rx}]: true,"
    lappend TEST_JSON                   "$TAB_5[string2json {test_sequence}]: \["
            set jj      0
            set jj_max [expr [llength [dict get $gt_macs_durations_modes $gt_idx]] / 2 - 1]
            foreach {duration mode} [dict get $gt_macs_durations_modes $gt_idx] {
    lappend TEST_JSON                       "$TAB_6\{"
    lappend TEST_JSON                           "$TAB_7[string2json {duration}]: $duration,"
    lappend TEST_JSON                           "$TAB_7[string2json {mode}]: [string2json $mode]"
                if {$jj < $jj_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6\}$next"
                incr jj
            }
    lappend TEST_JSON                   "$TAB_5\]"
            if {$lanes_config != {}} { set next {,} } else { set next {} }
    lappend TEST_JSON               "$TAB_4\}$next"
            if {$lanes_config != {}} {
    lappend TEST_JSON               "$TAB_4[string2json {lane_config}]: \{"
                set lane_config [dict get $lanes_config $gt_idx]
                set kk      0
                set kk_max [expr [llength [dict keys $lane_config]] - 1]
                foreach {lane_idx cfg} $lane_config {
    lappend TEST_JSON                   "$TAB_5[string2json $lane_idx]: \{"
                    set ll      0
                    set ll_max [expr [llength [dict keys $cfg]] - 1]
                    foreach {key val} $cfg {
                        if {$ll < $ll_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                       "$TAB_6[string2json $key]: $val$next"
                        incr ll
                    }
                    if {$kk < $kk_max} { set next {,} } else { set next {} }
    lappend TEST_JSON                   "$TAB_5\}$next"
                    incr kk
                }
    lappend TEST_JSON               "$TAB_4\}"
            }
            if {$ii < $ii_max} { set next {,} } else { set next {} }
    lappend TEST_JSON           "$TAB_3\}$next"
            incr ii
        }
    lappend TEST_JSON       "$TAB_2\}"
    }
    lappend TEST_JSON "$TAB_1\}"
    lappend TEST_JSON "\}"

    write_file $file [join $TEST_JSON "\n"]
    dict set config pre_canned $test $TEST_JSON
}

############################################################################################################

proc gen_pre_canned { config_ref } {
    # Dictionary passed as ref.
    upvar 1 $config_ref config
    log_message $config {GEN_PRE_CANNED-1}; # Start

    dict set config pre_canned {}

    gen_pre_canned_verify               config
    gen_pre_canned_power                config
    gen_pre_canned_memory               config board
    gen_pre_canned_memory               config host
    gen_pre_canned_mmio                 config
    gen_pre_canned_dma                  config
    gen_pre_canned_p2p_card             config
    gen_pre_canned_p2p_nvme             config
    gen_pre_canned_gt_mac               config
    gen_pre_canned_switch               config {10gbe}
    gen_pre_canned_switch               config {25gbe}
    gen_pre_canned_gt_mac_port_to_port  config
    gen_pre_canned_gt_mac_lpbk          config
    gen_pre_canned_gt_prbs              config
    gen_pre_canned_gtf_prbs             config
    gen_pre_canned_gtm_prbs             config
    gen_pre_canned_gtyp_prbs            config
    gen_pre_canned_stress               config
}
