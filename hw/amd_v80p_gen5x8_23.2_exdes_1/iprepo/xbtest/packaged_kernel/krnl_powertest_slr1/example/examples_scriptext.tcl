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

source ./imports/messages.tcl
source ./imports/common.tcl
source ./imports/param_autoconfigure.tcl
source ./imports/param_merge.tcl
source ./imports/param_validate.tcl
source ./imports/gen_wizard_config.tcl
source ./imports/gen_cu_config.tcl
source ./imports/gen_kernel_xml.tcl
source ./imports/package_kernel.tcl
source ./imports/gen_pre_canned.tcl
source ./imports/gen_xbtest_pfm_def.tcl
source ./imports/gen_build_xclbin_sh.tcl
source ./imports/gen_compile_ps_kernel_sh.tcl

# Initialization
set config {}
dict set config STOP_POST_OPT   0
dict set config ip              [get_ips]; # xbtest_wizard is the only ip here

#############################################################################################################
# Setup output/build directories
#############################################################################################################
dict set config base_dir            [pwd]; # Example design current working directory: u_ex
dict set config run_dir             [file join [dict get $config base_dir] run];                 file mkdir [dict get $config run_dir]
dict set config output_dir          [file join [dict get $config run_dir]  output];              file mkdir [dict get $config output_dir]
dict set config sw_dir              [file join [dict get $config run_dir]  sw];                  file mkdir [dict get $config sw_dir]
dict set config pre_canned_dir      [file join [dict get $config sw_dir]   test];                file mkdir [dict get $config pre_canned_dir]
dict set config packaged_kernel_dir [file join [dict get $config run_dir]  packaged_kernel];     file mkdir [dict get $config packaged_kernel_dir]
dict set config kernel_xml_dir      [file join [dict get $config run_dir]  kernel_xml];          file mkdir [dict get $config kernel_xml_dir]
dict set config kernel_xo_dir       [file join [dict get $config run_dir]  kernel_xo];           file mkdir [dict get $config kernel_xo_dir]

#############################################################################################################
# Get wizard configuration
#############################################################################################################
gen_wizard_config config

set local_xbtest_catalog    [file join [dict get $config run_dir] xbtest_catalog]
set ip_name_v               [dict get $config ip_name_v]

set ip_catalog      [dict get $config ip_catalog]
set ipdef           [file join $ip_catalog $ip_name_v]
set ipdef_ex_src    [file join $ipdef example]

#############################################################################################################
# Check if Vitis will stop post opt step
#############################################################################################################
if {[dict get $config C_INIT]} {
    dict set config STOP_POST_OPT 1
} elseif {[dict get $config wizard_actual_config cu_selection power] != {}} {
    if {([dict get $config C_DYNAMIC_GEOMETRY] == {}) || ([dict get $config C_UTILIZATION] == {})} {
        dict set config STOP_POST_OPT 1
    }
}

#############################################################################################################
# Generate power floorplan and setup the project that will be used to visualize it
#############################################################################################################
if {[dict get $config wizard_actual_config build display_pwr_floorplan] && ([dict get $config wizard_actual_config cu_selection power] != {})} {
    set gen_power_floorplan_tcl [file join $ipdef tcl  power gen_power_floorplan.tcl]
    set messages_tcl            [file join $ipdef tcl  messages.tcl]
    set messages_json           [file join $ipdef data messages.json]
    set display_pwr_fp_dir      [file join [dict get $config run_dir] display_pwr_fp]
    move_file $gen_power_floorplan_tcl [file join $display_pwr_fp_dir gen_power_floorplan.tcl]
    move_file $messages_json           [file join $display_pwr_fp_dir messages.json]
    move_file $messages_tcl            [file join $display_pwr_fp_dir messages.tcl]
    source [file join $display_pwr_fp_dir   gen_power_floorplan.tcl]
    source [file join $ipdef_ex_src         display_pwr_fp.tcl]
    display_pwr_fp_top config $display_pwr_fp_dir
}

#############################################################################################################
# Get CU configuration
#############################################################################################################
gen_cu_config config

#############################################################################################################
# Generate kernel.xml, file to be included in XO
#############################################################################################################
gen_kernel_xml $config

#############################################################################################################
# Customize IP and Generate XO
#############################################################################################################
package_kernel config

#############################################################################################################
# Generate pre-canned test JSON files
#############################################################################################################
gen_pre_canned config

#############################################################################################################
# Generate platform definition JSON file
#############################################################################################################
gen_xbtest_pfm_def config

#############################################################################################################
# Generate Vitis build script
#############################################################################################################
gen_build_xclbin_sh config

#############################################################################################################
# Generate PS kernel compilation script
#############################################################################################################
gen_compile_ps_kernel_sh config

#############################################################################################################
# Save extended configuration
#############################################################################################################
log_config config [list platforminfo Component_Name MESSAGES_DICT C_DYNAMIC_GEOMETRY] [file join [dict get $config run_dir] extended_config.log]

#############################################################################################################
# End of wizard example design
#############################################################################################################
log_message $config {XBTEST_WIZARD-2}
