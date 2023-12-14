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

# Script that generates platform definition JSON file

proc gen_xbtest_pfm_def { config_ref } {
    # Dictionary passed as ref.
    upvar 1 $config_ref config

    # set config {}
    # dict set config output_dir "."
    # dict set config wizard_actual_config platform name toto
    # dict set config wizard_actual_config platform is_nodma false
    # set memory_definition {{memory_name DDR target board} {memory_name HBM target board} {memory_name HOST target host}}
    # dict set config memory_definition $memory_definition
    # set gt_mac_definition {{gt_idx 0} {gt_idx 1}}
    # dict set config gt_mac_definition $gt_mac_definition
    # set gt_lpbk_definition {{gt_idx 2}}
    # dict set config gt_lpbk_definition $gt_lpbk_definition

    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    set xbtest_pfm_def_json [file join [dict get $config sw_dir] xbtest_pfm_def_template.json]

    log_message $config {GEN_XBTEST_PFM_DEF-1} [list $xbtest_pfm_def_json]; # Start

    set is_nodma [dict get $config wizard_actual_config platform is_nodma]

    set memories {}
    if {[dict exists $config memory_definition]} {
        foreach def [dict get $config memory_definition] {
            dict set memories [dict get $def memory_name] [dict get $def target]
        }
    }
    set gt_indexes {}
    foreach gt_definition {gt_mac_definition gt_lpbk_definition gt_prbs_definition gtf_prbs_definition gtm_prbs_definition gtyp_prbs_definition} {
        if {[dict exists $config $gt_definition]} {
            foreach def [dict get $config $gt_definition] {
                set gt_idx [dict get $def gt_idx]
                if {$gt_idx ni $gt_indexes} {
                    lappend gt_indexes $gt_idx
                }
            }
        }
    }

    set     XBTEST_PFM_DEF_JSON {}
    lappend XBTEST_PFM_DEF_JSON "\{"
    lappend XBTEST_PFM_DEF_JSON "$TAB_1[string2json {comment}]: \["
    set tmp "xbtest platform definition JSON file for: [dict get $config wizard_actual_config platform name]"
    lappend XBTEST_PFM_DEF_JSON "$TAB_2[string2json $tmp],"
    lappend XBTEST_PFM_DEF_JSON "$TAB_2[string2json {This is a generated template file}],"
    lappend XBTEST_PFM_DEF_JSON "$TAB_2[string2json {Please refer to the checklist for how to update/define its content}]"
    lappend XBTEST_PFM_DEF_JSON "$TAB_1\],"

    lappend XBTEST_PFM_DEF_JSON "$TAB_1[string2json {device}]: \{"

    lappend XBTEST_PFM_DEF_JSON     "$TAB_2[string2json {runtime}]: \{"
    lappend XBTEST_PFM_DEF_JSON         "$TAB_3[string2json {download_time}]: 1"
    if {([llength [dict keys $memories]] > 0) || ([llength $gt_indexes] > 0)} { set next "," } else { set next "" }
    lappend XBTEST_PFM_DEF_JSON     "$TAB_2\}$next"

    if {[llength $gt_indexes] > 0} {
    lappend XBTEST_PFM_DEF_JSON     "$TAB_2[string2json {gt}]: \{"
        set ii      0
        set ii_max [expr [llength $gt_indexes] - 1]
        foreach gt_idx $gt_indexes {
    lappend XBTEST_PFM_DEF_JSON         "$TAB_3[string2json $gt_idx]: \{"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4[string2json {transceiver_settings}]: \{"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {module}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {tx_differential_swing_control}]: 0,"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {tx_pre_emphasis}]: 0,"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {tx_post_emphasis}]: 0,"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {rx_equaliser}]: [string2json {DFE}]"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {cable}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {tx_differential_swing_control}]: 0,"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {tx_pre_emphasis}]: 0,"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {tx_post_emphasis}]: 0,"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {rx_equaliser}]: [string2json {DFE}]"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\}"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4\}"
            if {$ii < $ii_max} { set next "," } else { set next "" }
    lappend XBTEST_PFM_DEF_JSON           "$TAB_3\}$next"
            incr ii
        }
        if {[llength [dict keys $memories]] > 0} { set next "," } else { set next "" }
    lappend XBTEST_PFM_DEF_JSON       "$TAB_2\}$next"
    }

    if {[llength [dict keys $memories]] > 0} {
    lappend XBTEST_PFM_DEF_JSON     "$TAB_2[string2json {memory}]: \{"
        set ii      0
        set ii_max [expr [llength [dict keys $memories]] - 1]
        foreach {memory_name target} $memories {
    lappend XBTEST_PFM_DEF_JSON         "$TAB_3[string2json $ii]: \{"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4[string2json {name}]: [string2json $memory_name],"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4[string2json {cu_rate}]: \{"
            if {$target == {host}} {
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_wr}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 50"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\}"
            } else {
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_wr}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 100"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 100"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {simul_wr_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 100"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\},"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 100"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\}"
            }
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4\},"
            if {$target == {board}} {
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4[string2json {cu_outstanding}]: \{"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_wr}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 0"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 0"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {simul_wr_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 0"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\},"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {nominal}]: 0"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\}"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4\},"
            }
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4[string2json {cu_bw}]: \{"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_wr}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {simul_wr_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\},"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\}"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4\},"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4[string2json {cu_latency}]: \{"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_wr}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {only_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\},"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5[string2json {simul_wr_rd}]: \{"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {write}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\},"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6[string2json {read}]: \{"
    lappend XBTEST_PFM_DEF_JSON                         "$TAB_7[string2json {average}]: 2"
    lappend XBTEST_PFM_DEF_JSON                     "$TAB_6\}"
    lappend XBTEST_PFM_DEF_JSON                 "$TAB_5\}"
    lappend XBTEST_PFM_DEF_JSON             "$TAB_4\}"
            if {$ii < $ii_max} { set next "," } else { set next "" }
    lappend XBTEST_PFM_DEF_JSON           "$TAB_3\}$next"
            incr ii
        }
    lappend XBTEST_PFM_DEF_JSON       "$TAB_2\}"
    }
    lappend XBTEST_PFM_DEF_JSON "$TAB_1\}"
    lappend XBTEST_PFM_DEF_JSON "\}"

    write_file $xbtest_pfm_def_json [join $XBTEST_PFM_DEF_JSON "\n"]
    dict set config xbtest_pfm_def $XBTEST_PFM_DEF_JSON
}
