# (c) Copyright 2022-2023, Advanced Micro Devices, Inc.
# 
# Permission is hereby granted, free of charge, to any person obtaining a 
# copy of this software and associated documentation files (the "Software"), 
# to deal in the Software without restriction, including without limitation 
# the rights to use, copy, modify, merge, publish, distribute, sublicense, 
# and/or sell copies of the Software, and to permit persons to whom the 
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
# DEALINGS IN THE SOFTWARE.
############################################################

set script_path     [file normalize [info script]]
set script_dir   [file dirname $script_path]

proc write_file { filename data } {
    set fp [open $filename w]; puts $fp $data; close $fp
}
proc read_file { filename } {
    set fp [open $filename r]; set data [read $fp]; close $fp
    return $data
}

# Procedure to convert JSON data to TCL dictionary
proc json2dict { txt } {
    package require json

    # Remove space and new lines before processing with ::json::json2dict
    set data    {}
    set trim    true

    foreach c [split $txt ""] {
        if {[string match {\"} $c]} {
            set trim [expr ! $trim]
        }
        if {(![string match { } $c] && ![string match "\n" $c] && ![string match "\t" $c]) || !$trim} {
            append data $c
        }
    }
    return [::json::json2dict $data]
}

proc check_file_exists { name } {
    if {! [file exist $name]} {
        common::send_msg_id {BUILD_HW-1} {ERROR} "$name does not exist"
    }
}
proc check_dir_exists { name } {
    if {! [file isdirectory $name]} {
        common::send_msg_id {BUILD_HW-1} {ERROR} "$name does not exist"
    }
}

proc print_help {} {
  global static_xsa
  global example_application_metadata_dir
  global conf_json
  global default_ip_repo
  global gui_mode
  global xbtest_example

    puts {Usage:}
    puts {  vivado -mode tcl -source <path to>/build_hw.tcl -tclargs -s <static_xsa> ...}
    puts {}
    puts {Where:}
    puts "  -s <static_xsa>            : Path where static XSA will be generated.                Default: $static_xsa"
    puts "  -m <example_application_metadata_dir>      : Directory where application metadata will be generated. Default: $example_application_metadata_dir"
    puts "  -c <conf_json>             : Configuration json file.                                Default: $conf_json"
    puts "  -i <ip_repo_paths>         : Path(s) to IP repositories.                             Default: $default_ip_repo"
    puts "  -g \[0|1]                  : GUI mode. Generate design and stop.                     Default: $gui_mode"
    puts "  -x <string>                : Override xbtest example from conf.json                  Default: $xbtest_example"
    puts "                             :   none - No xbtest example                                                      "
    puts "                             :   json - Use configuration json value."
    puts "                             :   xbtest_verify - Lightweight xbtest."
    puts "                             :   xbtest_stress - Full xbtest suite."
    puts {}
}

# Check arguments exists
if {![info exists argv]} {
    print_help
    common::send_msg_id {BUILD_HW-2} {ERROR} {argv input is not defined}
}
if {![info exists argc]} {
    print_help
    common::send_msg_id {BUILD_HW-3} {ERROR} {argc input is not defined}
}

set static_xsa                       [pwd]
set example_application_metadata_dir [file normalize [file join [pwd] xbtest metadata]]
set conf_json                        [file normalize [file join .. conf.json]]
set default_ip_repo                  [file normalize [file join [pwd] iprepo]]
set gui_mode                         0
set example_application              {json}

set ii 0
while {$ii < $argc} {
    set arg [lindex $argv $ii]
    set val [lindex $argv [incr ii]]

    switch -exact -- $arg {
        -s {
            set static_xsa $val
        }
        -c {
            set conf_json $val
        }
        -i {
            lappend ip_repo_paths $val
        }
        -g {
            set gui_mode $val
        }
        -x {
            set example_application $val
        }
        -m {
            set example_application_metadata_dir $val
        }
        default {
            print_help
            common::send_msg_id {BUILD_HW-4} {ERROR} {Wrong inputs}
        }
    }
    incr ii
}

# Check command line options
if {($static_xsa == {}) || ($example_application_metadata_dir == {}) || ($conf_json == {}) || ($ip_repo_paths == {})} {
    print_help
    common::send_msg_id {BUILD_HW-4} {ERROR} {Wrong inputs}
}

if {[llength $ip_repo_paths] == 0} { 
  set ip_repo_paths       $default_ip_repo
}
common::send_msg_id {BUILD_HW-7} {INFO} {Arguments provided:}
common::send_msg_id {BUILD_HW-7} {INFO} "\t - static_xsa          : $static_xsa"
common::send_msg_id {BUILD_HW-7} {INFO} "\t - example_application_metadata_dir    : $example_application_metadata_dir"
common::send_msg_id {BUILD_HW-7} {INFO} "\t - conf_json           : $conf_json"
common::send_msg_id {BUILD_HW-7} {INFO} "\t - ip_repo_paths       : [join $ip_repo_paths {, }]"
common::send_msg_id {BUILD_HW-7} {INFO} "\t - gui_mode            : $gui_mode"
common::send_msg_id {BUILD_HW-7} {INFO} "\t - example_application : $example_application"

check_file_exists $conf_json
foreach {repo} $ip_repo_paths {
    check_dir_exists $repo
}

# Load configuration JSON file
set config_data [read_file $conf_json]
set design_cfg  [json2dict $config_data]

if {[string equal -nocase {json} $example_application]} { 
  set example_application [dict get $design_cfg design application]
}
if {$gui_mode} { 
  start_gui
}
# Create the project targeting its part
create_project prj ./prj -part [dict get $design_cfg board device_part] -force


# Set project IP repositories
set_property ip_repo_paths $ip_repo_paths [current_project]
update_ip_catalog

# Create block diagram
create_bd_design  top
current_bd_design top

# Add base to block diagram
source $script_dir/bd/create_bd_design.tcl
create_root_design ""

if {[string match -nocase {xbtest*} $example_application]} {
  # Update the project to add xbtest
  source $script_dir/bd/customize_bd_xbtest.tcl
  customize_bd_xbtest $design_cfg "" $example_application

  source $script_dir/metadata/generate_xbtest_metadata.tcl
  generate_xbtest_metadata $design_cfg $example_application_metadata_dir

  # Validate and save the full design
  validate_bd_design
  save_bd_design

  import_files -fileset constrs_1 -norecurse $script_dir/constraints/impl.xbtest.xdc
  set_property -dict { used_in_synthesis false    processing_order LATE   } [get_files *impl.xbtest.xdc]
}

# Write the block diagram wrapper and set it as design top
add_files -norecurse [make_wrapper -files [get_files top.bd] -top]
set_property top top_wrapper [current_fileset]
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

# Add constraint and hook files
import_files -fileset constrs_1 -norecurse $script_dir/constraints/impl.xdc
import_files -fileset constrs_1 -norecurse $script_dir/constraints/impl.pins.xdc
import_files -fileset utils_1   -norecurse $script_dir/constraints/opt.post.tcl
import_files -fileset utils_1   -norecurse $script_dir/constraints/place.pre.tcl
import_files -fileset utils_1   -norecurse $script_dir/constraints/write_device_image.pre.tcl

set_property -dict { used_in_synthesis false    processing_order NORMAL } [get_files *impl.xdc]
set_property -dict { used_in_synthesis false    processing_order NORMAL } [get_files *impl.pins.xdc]

set_property STEPS.OPT_DESIGN.TCL.POST         [get_files *opt.post.tcl]                [get_runs impl_1]
set_property STEPS.PLACE_DESIGN.TCL.PRE        [get_files *place.pre.tcl]               [get_runs impl_1]
set_property STEPS.WRITE_DEVICE_IMAGE.TCL.PRE  [get_files *write_device_image.pre.tcl]  [get_runs impl_1]

proc do_aved_build {} { 
  global example_application 
  global design_cfg
  global example_application_metadata_dir 
  global static_xsa

  # Generate all output products
  generate_target all [get_files top.bd]

  # Write design bd out.
  file mkdir bd_gen
  write_bd_tcl -force -no_ip_version -hier_blks [get_bd_cells .] ./bd_gen/create_bd_design_final.tcl
  # Run all synthesis and implementation steps
  launch_runs synth_1 -jobs 4 
  wait_on_runs synth_1
  launch_runs impl_1 -to_step write_device_image -jobs 4
  wait_on_runs impl_1

  # Implementation has terminated, check it has completed successfully.
  if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    common::send_msg_id {BUILD_HW-6} {ERROR} "Implementation failed"
  }

  if {[string match -nocase {xbtest*} $example_application]} { 
    # Adding UUID to xbtest metadata
    set uuid_file [open [file join [get_property DIRECTORY [current_run]] "pfm_uuid_manifest.dict"] r]
    set uuid_dict [read $uuid_file]
    close $uuid_file
    add_xbtest_metadata_uuid $design_cfg $example_application_metadata_dir $uuid_dict
  }

  # Write XSA is needed to build VMR and to generate xsabin with Vitis Packager
  open_run impl_1
  write_hw_platform -force -fixed -hw -static $static_xsa

  common::send_msg_id {BUILD_HW-8} {INFO} {Done!}
}

if {$gui_mode == 0} {
  do_aved_build
  exit 0
}
