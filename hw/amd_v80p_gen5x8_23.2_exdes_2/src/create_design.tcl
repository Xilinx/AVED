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
set cwd            [pwd]
set design_name     "amd_v80p_gen5x8_23.2_exdes_2"
set bd_name         "top"

proc do_aved_create_design { } { 
  global bd_name
  global src_dir
  global design_name
  # Create the project targeting its part
  create_project prj "[pwd]/build" -part xcv80-lsva4737-2MHP-e-S -force


  # Set project IP repositories
  set_property ip_repo_paths "${src_dir}/iprepo" [current_project]
  update_ip_catalog

  # Create block diagram
  create_bd_design  ${bd_name}
  current_bd_design ${bd_name}

  # Add base to block diagram
  source "$src_dir/bd/create_bd_design.tcl"
  create_root_design ""

  # Write the block diagram wrapper and set it as design top
  add_files -norecurse [make_wrapper -files [get_files "${bd_name}.bd"] -top]
  update_compile_order -fileset sources_1
  update_compile_order -fileset sim_1
  set_property top top_wrapper [current_fileset]

  # Add constraint and hook files
  import_files -fileset constrs_1 -norecurse "$src_dir/constraints/impl.xdc"
  import_files -fileset constrs_1 -norecurse "$src_dir/constraints/impl.pins.xdc"
  import_files -fileset utils_1   -norecurse "$src_dir/constraints/opt.post.tcl"
  import_files -fileset utils_1   -norecurse "$src_dir/constraints/place.pre.tcl"
  import_files -fileset utils_1   -norecurse "$src_dir/constraints/write_device_image.pre.tcl"

  set_property -dict { used_in_synthesis false    processing_order NORMAL } [get_files *impl.xdc]
  set_property -dict { used_in_synthesis false    processing_order NORMAL } [get_files *impl.pins.xdc]

  set_property STEPS.OPT_DESIGN.TCL.POST         [get_files *opt.post.tcl]                [get_runs impl_1]
  set_property STEPS.PLACE_DESIGN.TCL.PRE        [get_files *place.pre.tcl]               [get_runs impl_1]
  set_property STEPS.WRITE_DEVICE_IMAGE.TCL.PRE  [get_files *write_device_image.pre.tcl]  [get_runs impl_1]
}

do_aved_create_design
