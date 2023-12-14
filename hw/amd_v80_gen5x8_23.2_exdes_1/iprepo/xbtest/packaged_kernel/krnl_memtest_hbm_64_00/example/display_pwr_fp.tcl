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

proc display_pwr_fp_top { config_ref display_pwr_fp_dir } {
    # Dictionary passed as ref.
    upvar 1 $config_ref config

    set cwd [pwd]
    cd $display_pwr_fp_dir

    log_message $config {DISPLAY_PWR_FP-1}

    set part                [dict_get_quiet $config {wizard_actual_config platform fpga_part}]
    set pwr_floorplan_dir   [dict_get_quiet $config {wizard_actual_config build pwr_floorplan_dir}]
    set POWER               [dict_get_quiet $config {wizard_actual_config cu_selection power}]

    set ip_catalog   [dict get $config ip_catalog]
    set ip_name_v    [dict get $config ip_name_v]

    log_message $config {DISPLAY_PWR_FP-3} [list "display_pwr_fp" $part $display_pwr_fp_dir]
    create_project -force display_pwr_fp $display_pwr_fp_dir -part $part
    set_property design_mode PinPlanning [current_fileset]
    open_io_design -name io_1

    global C_DYNAMIC_GEOMETRY
    global C_INVALID
    global C_UTILIZATION

    set C_DYNAMIC_GEOMETRY  [dict get $config C_DYNAMIC_GEOMETRY]
    set C_INVALID           [dict get $config C_INVALID]
    set C_UTILIZATION       [dict get $config C_UTILIZATION]

    # Get list of SLR provided
    global SLR_LIST_IN
    set SLR_LIST_IN {}
    foreach idx_slr $POWER {
        lappend SLR_LIST_IN $idx_slr
    }

    set GEN_CONST_DEBUG  1
    set GEN_CONST_OUTPUT 1

    global messages_config
    set messages_config [dict create MESSAGES_DICT [dict get $config MESSAGES_DICT]]; # Needed in gen_power_floorplan

    gen_power_floorplan $SLR_LIST_IN $GEN_CONST_DEBUG $GEN_CONST_OUTPUT

    log_message $config {DISPLAY_PWR_FP-4} {{Mark actual sites depending on site type using for example:}}
    log_message $config {DISPLAY_PWR_FP-4} {{\t - display_pwr_fp SLICE}}
    log_message $config {DISPLAY_PWR_FP-4} {{Mark all actual sites using:}}
    log_message $config {DISPLAY_PWR_FP-4} {{\t - display_pwr_fp}}

    log_message $config {DISPLAY_PWR_FP-2}

    display_pwr_fp

    set    SETUP_TCL [list]
    lappend SETUP_TCL "open_project [file join $display_pwr_fp_dir display_pwr_fp.xpr]"
    lappend SETUP_TCL "source [file join $ip_catalog $ip_name_v example common.tcl]"
    lappend SETUP_TCL "source [file join $ip_catalog $ip_name_v tcl power gen_power_floorplan.tcl]"
    lappend SETUP_TCL "source [file join $ip_catalog $ip_name_v tcl messages.tcl]"
    lappend SETUP_TCL "set messages_config \[dict create MESSAGES_DICT \[load_messages_json [file join $ip_catalog $ip_name_v data messages.json]\]\]"
    lappend SETUP_TCL "set C_DYNAMIC_GEOMETRY_JSON \[read_file [file join $pwr_floorplan_dir dynamic_geometry.json]\]"
    lappend SETUP_TCL "set C_DYNAMIC_GEOMETRY      \[json2dict \$C_DYNAMIC_GEOMETRY_JSON\]"
    lappend SETUP_TCL "display_pwr_fp help"
    lappend SETUP_TCL "cd $display_pwr_fp_dir"

    write_file [file join $display_pwr_fp_dir setup.tcl] [join $SETUP_TCL "\n"]

    cd $cwd
    close_project
}