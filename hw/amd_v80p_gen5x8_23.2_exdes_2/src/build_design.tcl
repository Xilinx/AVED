# (c) Copyright 2024, Advanced Micro Devices, Inc.
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

set src_dir        [file dirname [file normalize [info script]]]
set design_name     "amd_v80p_gen5x8_23.2_exdes_2"
set bd_name         "top"

proc do_aved_build {} { 
  global design_name
  global src_dir
  global bd_name
  set build_dir [get_property DIRECTORY [current_project]]

  # Generate all output products
  generate_target all [get_files "${bd_name}.bd"]

  # Write xsa
  write_hw_platform -force -fixed -minimal "${build_dir}/${design_name}.xsa"

  # Write design bd out (optional)
  file mkdir ${build_dir}/bd_gen
  write_bd_tcl -force -no_ip_version -hier_blks [get_bd_cells /] "${build_dir}/bd_gen/create_bd_design_final.tcl"

  # Run all synthesis and implementation steps
  launch_runs synth_1 -jobs 4 
  wait_on_runs synth_1
  launch_runs impl_1 -to_step write_device_image -jobs 4
  wait_on_runs impl_1

  # Implementation has terminated, check it has completed successfully.
  if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    common::send_msg_id {BUILD_HW-6} {ERROR} "Implementation failed"
  }

  common::send_msg_id {BUILD_HW-8} {INFO} {Done!}
}

do_aved_build
