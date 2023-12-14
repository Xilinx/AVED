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

proc gen_kernel_xml { config } {
    variable C_KRNL_MODE_POWER

    foreach {cu_cfg} [dict get $config cu_config] {

        set kernel_name             [dict get $cu_cfg kernel_name]
        set krnl_mode               [dict get $cu_cfg krnl_mode]
        log_message $config {GEN_KERNEL_XML-2} [list $kernel_name]; # Start

        set use_s_axi       true
        set has_interrupt   true
        if {$krnl_mode == $C_KRNL_MODE_POWER} {
            set throttle_mode [dict get $cu_cfg throttle_mode]
            # Disable kernel control for external throttle_mode
            if {($throttle_mode == {EXTERNAL_MACRO}) || ($throttle_mode == {EXTERNAL_CLK})} {
                set use_s_axi       false
                set has_interrupt   false
            }
        }

        set kernel_vendor           [dict get $config ip_vendor]
        set kernel_library          [dict get $config ip_library]
        set kernel_version_major    [dict get $config ip_version_major]
        set kernel_version_minor    [dict get $config ip_version_minor]
        set vlnv                    ${kernel_vendor}:${kernel_library}:${kernel_name}:${kernel_version_major}.${kernel_version_minor}

        set use_aie 0
        set aie_type {}
        set aie_control {}
        set aie_status {}
        if {$krnl_mode == $C_KRNL_MODE_POWER} {
            set use_aie [dict get $cu_cfg use_aie]
            if {$use_aie} {
                set aie_type    [dict get $cu_cfg aie_type]
                set aie_control [dict get $cu_cfg aie_control]
                set aie_status  [dict get $cu_cfg aie_status]
            }
        }

        # Find axi ports
        set connection_indexes [list]
        foreach key [dict keys $cu_cfg sp_m*_axi] {
            regexp {sp_m0*([0-9]+)_axi} $key -> connection_idx
            lappend connection_indexes $connection_idx
        }
        set num_connections [llength $connection_indexes]

        set ports           {}
        set args            {}
        set id              0
        set offset          16

        # Add s_axi_control ports
        if {$use_s_axi} {
            lappend ports {      <port name="s_axi_control" mode="slave" range="0x1000" dataWidth="32" portType="addressable" base="0x0"/>}
        }
        # Add memory mapped ports
        for {set i 0} {$i < $num_connections} {incr i} {
            set connection_idx  [lindex $connection_indexes $i]
            set port            [format {m%02d_axi} $connection_idx]
            lappend ports [subst {      <port name="$port" mode="master" range="0xFFFFFFFFFFFFFFFF" dataWidth="512" portType="addressable" base="0x0"/>}]
        }
        # Add stream ports for power CU
        if {$use_aie} {
            set C_AXIS_AIE_DATA_WIDTH 128
            if {$aie_type == {AIE_ML_CORE}} {
                set C_AXIS_AIE_DATA_WIDTH 32
            }

            if {$aie_control == {pl}} {
                lappend ports [subst {      <port name="m_axis_aie0" mode="write_only" dataWidth="$C_AXIS_AIE_DATA_WIDTH" portType="stream"/>}]
            }
            if {$aie_status == {pl}} {
                lappend ports [subst {      <port name="s_axis_aie0" mode="read_only" dataWidth="$C_AXIS_AIE_DATA_WIDTH" portType="stream"/>}]
            }
        }

        # Add arguments. Don't add any arguments if CU does not have s_axi_control port
        if {$use_s_axi} {
            # Add scalar arguments
            for {set i 0} {$i < 4} {incr i} {
                set name                [format {scalar%02d} $i]
                set offset              [format {0x%03x} $offset]

                lappend args [subst {      <arg id="$id" name="$name" type="uint" addressQualifier="0" port="s_axi_control" size="0x4" offset="$offset" hostSize="0x4" hostOffset="0x0"/>}]
                set offset [expr {$offset + 8}]
                incr id
            }
            # Add global memory arguments
            for {set i 0} {$i < $num_connections} {incr i} {
                set connection_idx      [lindex $connection_indexes $i]
                set name                [format {axi%02d_ptr0}  $connection_idx]
                set port                [format {m%02d_axi}     $connection_idx]
                set offset              [format {0x%03x}        $offset]

                lappend args [subst {      <arg id="$id" name="$name" type="int*" addressQualifier="1" port="$port" size="0x8" offset="$offset" hostSize="0x8" hostOffset="0x0"/>}]
                set offset [expr {$offset + 8}]
                incr id
            }
            if {$use_aie} {
                if {$aie_control == {pl}} {
                    lappend args [subst {      <arg id="$id" name="arg_m_axis_aie0" type="stream" addressQualifier="4" port="m_axis_aie0" size="0x0" offset="0x0" hostSize="0x8" hostOffset="0x0" memSize="0x20" origName="arg_m_axis_aie0" origUse="variable"/>}]; incr id
                }
                if {$aie_status == {pl}} {
                    lappend args [subst {      <arg id="$id" name="arg_s_axis_aie0" type="stream" addressQualifier="4" port="s_axis_aie0" size="0x0" offset="0x0" hostSize="0x8" hostOffset="0x0" memSize="0x20" origName="arg_s_axis_aie0" origUse="variable"/>}]; incr id
                }
            }
        }

        # Output xml
        set KERNEL_XML [subst {<?xml version="1.0" encoding="UTF-8"?>
<root versionMajor="1" versionMinor="6">
  <kernel name="$kernel_name" language="ip_c" vlnv="$vlnv" attributes="" preferredWorkGroupSizeMultiple="0" workGroupSize="1" interrupt="$has_interrupt">
    <ports>
[join $ports "\n"]
    </ports>
    <args>
[join $args "\n"]
    </args>
  </kernel>
</root>
}]

        write_file [dict get $cu_cfg kernel_xml] $KERNEL_XML
        log_message $config {GEN_KERNEL_XML-1} [list [dict get $cu_cfg kernel_xml]]; # End of generation
    }
}
