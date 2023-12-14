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

# TODO Remove interface UUID once ART is available
# TODO should we also allow user to provide the UUID value, or other values to add in UUID ROM (e.g. version, etc)
# TODO should we also allow user to provide bitstream USR_ACCESS 32-bit register value (e.g. version, etc)?

   # For now, this script:
   # - Calculates a platform Logic-UUID from the synthesized checkpoint, and populates the UUID ROM with it;
   # - Calculates a platform Interface-UUID from the routed checkpoint;
   # - Inserts the Logic-UUID and Interface-UUID into a dictionary in a file used by write_hw_platform.
   # =========================================================================

   proc find_dir {base name} {
     set res [glob -nocomplain -types d -directory $base $name]
     if {$res != {}} {
       return $res
     }
     foreach dir [glob -nocomplain -types d -directory $base *] {
       set res [find_dir [file join $base $dir] $name]
       if {$res != {}} {
         return $res
       }
     }
     return {}
   }

   # Procedure to update the Logic UUID ROM
   # Input is 64 hex character string (256-bit UUID)

   proc update_logic_uuid_rom {uuid} {

      # Get the absolute directory path of the shel_utils_uuid_rom_v2_0 Tcl directory
      set scr_fname {}

      foreach ip_repo_path [get_property IP_REPO_PATHS [current_project]] {
        set scr_fname [find_dir $ip_repo_path shell_utils_uuid_rom_v2_0]
        if {$scr_fname != {}} {
          break
        }
      }
      set update_uuid_rom [file join $scr_fname tcl update_uuid_rom.tcl]

      # Source the update UUID ROM script, return an error if not found
      if {[file exists $update_uuid_rom]} {
         source $update_uuid_rom
      } else {
         return -code error "ERROR: update_uuid_rom.tcl script not found, Logic UUID not populated."
      }

      # Search for the BLP_LOGIC_UUID_ROM cell path in the netlist, return an error if not found
      set uuid_cell [get_cells -hier -filter {NAME =~ "*uuid_rom" && PARENT =~ "*base_logic"}]
      if {$uuid_cell eq ""} {
         return -code error "ERROR: BLP_LOGIC_UUID_ROM cell not found in netlist, Logic UUID not populated."
      }

      # Call the update_uuid_rom script to update the Logic UUID ROM, return the response
      return [update_uuid_rom $uuid $uuid_cell]
   }

   set top_name [get_property TOP [current_design]]

   # Code to generate a Logic-UUID from the synthesized checkpoint
   # Get the absolute directory path of the project synth_1 run
   set synth_fname [file normalize "../synth_1/"]

   # Use md5sum to calculate a Logic-UUID and then populate the ROM with it, or return an error if the checkpoint isn't found
   if {[file exists ${synth_fname}/${top_name}.dcp]} {
      set logic_uuid [lindex [exec md5sum ${synth_fname}/${top_name}.dcp] 0]
      puts "Logic-UUID is $logic_uuid"
      update_logic_uuid_rom $logic_uuid
   } else {
      return -code error "ERROR: synthesized checkpoint ${top_name}.dcp not found, cannot generate Logic-UUID."
   }

   # Create a dictionary of each cell that will require a generated UUID, starting with design top
   set design_top [lindex [get_cells] 0]
   set uuid_dict [dict create logic_uuid $logic_uuid]

   # Generate a UUID and add it to the dictionary for ulp cell
   set dfx_cells [list top_i/ulp]
   if {1 != [llength $dfx_cells]} {
      return -code error "ERROR: more than one reconfigurable partition found; this is not currently supported, so cannot generate Interface-UUID."
   }
   foreach {dfx_cell} $dfx_cells {
      # Code to generate an Interface-UUID from the routed checkpoint
      # Get the absolute directory path of the project impl_1 run
      set route_fname [file normalize "./"]

      # Use md5sum to calculate an Interface-UUID, or return an error if the checkpoint isn't found
      if {[file exists ${route_fname}/${top_name}_routed.dcp]} {
         set interface_uuid [lindex [exec md5sum ${route_fname}/${top_name}_routed.dcp] 0]
         puts "Interface-UUID is $interface_uuid"
         dict set uuid_dict interface_uuid ${interface_uuid}
      } else {
         return -code error "ERROR: routed checkpoint ${top_name}_routed.dcp not found, cannot generate Interface-UUID."
      }
   }

   # Now write the full dictionary, consisting of key-value pairs of the top cell and its Logic-UUID, and the reconfigurable module
   # and its Interface-UUID, to a file with predetermined name required for the write_hw_platform flow.
   set uuid_file [open [file join [get_property DIRECTORY [current_project]] "pfm_uuid_manifest.dict"] w]
   puts $uuid_file $uuid_dict
   close $uuid_file

# Enable automatic loading of bitstream USR_ACCESS 32-bit register with timestamp
# TODO why is this set here and not in build_hw.tcl?
set_property BITSTREAM.CONFIG.USR_ACCESS TIMESTAMP [current_design]
