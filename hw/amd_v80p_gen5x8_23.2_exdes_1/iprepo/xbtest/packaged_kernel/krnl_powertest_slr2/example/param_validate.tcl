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

############################################################################################################
# xpfm
############################################################################################################
proc validate_xpfm { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      {xpfm}
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {}

    if {$param_value != {}} {
        if {![file exists $param_value]} {
            log_message $config {XBTEST_WIZARD-14} [list $param_name $param_value]; # ERROR: file not found.
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list {user} $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# wizard_config_json
############################################################################################################
proc validate_wizard_config_json { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    if {[dict get $config C_INIT]} {
        return 1; # Ignore wizard_config_json in init mode
    }

    set param_path      {wizard_config_json}
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {<path>}

    if {($param_value == $default_value) || ($param_value == {})} {
        log_message $config {XBTEST_WIZARD-13} [list $param_name]; # ERROR: value not specified.
    }
    if {![file exists $param_value]} {
        log_message $config {XBTEST_WIZARD-14} [list $param_name $param_value]; # ERROR: file not found.
    }
    log_message $config {XBTEST_WIZARD-22} [list {user} $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.fpga_part
############################################################################################################
proc validate_platform_fpga_part { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    if {$param_value == {}} {
        set expected {non-empty value}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value is not $expected"]
            return 1; # Valid value
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.fpga_family
############################################################################################################
proc validate_platform_fpga_family { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]


    if {$param_value == {}} {
        set expected {non-empty value}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value is not $expected"]
            return 1; # Valid value
        } else {
            # FPGA family not provided in user config, try to autoconfigure based on actual fpga_part

            set fpga_part_path     [concat [lrange $param_path 0 end-1] [list fpga_part]]
            set fpga_part_value    [dict_get_quiet $config $fpga_part_path]

            set param_value  [get_property -quiet C_FAMILY [get_parts -quiet $fpga_part_value]]
            set_autoconfigured_param config $cfg_name $param_path0 $param_value "no value found from actual fpga_part \{$fpga_part_value\}"

            # Validate auto-configured FPGA family
            set param_value [dict_get_quiet $config $param_path]
            if {$param_value == {}} {
                log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.name
############################################################################################################
proc validate_platform_name { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {NOT DEFINED}

    if {$param_value == {}} {
        log_message $config {XBTEST_WIZARD-13} [list $param_name]; # ERROR: value not specified.
    }
    log_message $config {XBTEST_WIZARD-22} [list {user} $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.interface_uuid
############################################################################################################
proc validate_platform_interface_uuid { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {NOT DEFINED}

    if {$param_value == {}} {
        log_message $config {XBTEST_WIZARD-13} [list $param_name]; # ERROR: value not specified.
    }
    log_message $config {XBTEST_WIZARD-22} [list {user} $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.mmio_support
############################################################################################################
proc validate_platform_mmio_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {true false}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.is_nodma
############################################################################################################
proc validate_platform_is_nodma { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {true false}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.dma_support
############################################################################################################
proc validate_platform_dma_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {true false}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.p2p_support
############################################################################################################
proc validate_platform_p2p_support { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {true false}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.mac_addresses_available
############################################################################################################
proc validate_platform_mac_addresses_available { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {default}

    if {$param_value != $default_value} {
        if {(![string is integer $param_value]) || (([string is integer $param_value]) && ($param_value < 0))} {
            set expected "positive integer numbers"
            if {$cfg_name == {wizard_auto_config}} {
                set dict_path  [lrange $param_path 0 end-1]
                set dict_name  [join $dict_path {.}]
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.aie.freq
############################################################################################################
proc validate_platform_aie_freq { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {default}

    if {$param_value != $default_value} {
        if {(![string is integer $param_value]) || (([string is integer $param_value]) && ($param_value < 0))} {
            set expected "positive integer value"
            if {$cfg_name == {wizard_auto_config}} {
                set dict_path  [lrange $param_path 0 end-1]
                set dict_name  [join $dict_path {.}]
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.aie.control
############################################################################################################
proc validate_platform_aie_control { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {default pl}
    # set expected_values {default ps pl}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.aie.status
############################################################################################################
proc validate_platform_aie_status { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {default pl none}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.gt
############################################################################################################
proc validate_platform_gt { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GT0 config>}, "1": {<GT1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    foreach {key} [dict keys $param_value] {
        if {![string is integer $key]} {
            set expected {integer numbers}
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.gt.<gt_idx>.slr
############################################################################################################
proc validate_platform_gt_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set gt_idx      [lindex $param_path0 2]
    # We only understand SLRx syntax
    if {[regexp {^SLR([0-9])$} $param_value] == 0} {
        set expected {of format SLRx}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1; # Valid value
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid param_value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.gt.<gt_idx>.type
############################################################################################################
proc validate_platform_gt_type { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {GTY GTM GTYP}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1; # Valid value
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.gt.<gt_idx>.group_select
############################################################################################################
proc validate_platform_gt_group_select { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set gt_idx      [lindex $param_path0 2]
    if {$param_value == {}} {
        set expected {non-empty value}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1; # Valid value
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# platform.gt.<gt_idx>.refclk_sel
############################################################################################################
proc validate_platform_gt_refclk_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set gt_idx          [lindex $param_path0 2]

    if {$param_value == {}} {
        set expected {non-empty value}
        log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
        return 0
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# build.pwr_floorplan_dir
############################################################################################################
proc validate_build_pwr_floorplan_dir { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {}

    if {$param_value != $default_value} {
        set param_value [file normalize $param_value]

        if {![file isdir $param_value]} {
            log_message $config {XBTEST_WIZARD-14} [list $param_name $param_value]; # ERROR: param_value directory not found.
            return 0
        }
        dict_set config $param_path $param_value

        # Save input power floorplan source files
        foreach name {{dynamic_geometry} {utilization} {invalid}} {
            move_file [file join $param_value ${name}.json] [file join [dict get $config run_dir] pwr_floorplan ${name}.json]
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# build.vpp_options_dir
############################################################################################################
proc validate_build_vpp_options_dir { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set default_value   {}

    if {$param_value != $default_value} {
        set param_value [file normalize $param_value]

        if {![file isdir $param_value]} {
            log_message $config {XBTEST_WIZARD-14} [list $param_name $param_value]; # ERROR: param_value directory not found.
            return 0
        }
        dict_set config $param_path $param_value

        foreach name {{vpp.ini}} {
            set filename [file join $param_value $name]
            if {![file exists $filename]} {
                log_message $config {XBTEST_WIZARD-39} [list $filename]; # ERROR: file not found.
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# build.display_pwr_floorplan
############################################################################################################
proc validate_build_display_pwr_floorplan { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {true false}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# build.vpp_link_output
############################################################################################################
proc validate_build_vpp_link_output { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {default xclbin xsa}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.clock
############################################################################################################
proc validate_cu_configuration_clock { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<Clock 0 config>}, "1": {<Clock 1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    foreach {key} [dict keys $param_value] {
        if {![string is integer $key]} {
            set expected {integer numbers}
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "clock_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.clock.<idx>.freq
############################################################################################################
proc validate_cu_configuration_clock_freq { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set value_min   200
    set value_max   650
    set clk_idx     [lindex $param_path0 2]
    if {(![string is integer $param_value]) || ($param_value < $value_min) || ($param_value > $value_max)} {
        set expected "in range between $value_min and $value_max"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $clk_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1; # Valid value
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.plram_selection
############################################################################################################
proc validate_cu_configuration_plram_selection { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"SLR0" : "PLRAM[0]", "SLR1" : "PLRAM[1]"}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_set config $param_path {}
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    foreach {key} [dict keys $param_value] {
        # We only understand SLRx syntax
        if {[regexp {^SLR([0-9])$} $key] == 0} {
            set expected {of format SLRx}
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "SLR key ($key) is not $expected"]
                dict_unset config [concat $param_name [list $key]]
                return 1; # Valid value
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
}

############################################################################################################
# cu_configuration.verify.slr
############################################################################################################
proc validate_cu_configuration_verify_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {$param_value != {auto}} {
        # We only understand SLRx syntax
        if {[regexp {^SLR([0-9])$} $param_value] == 0} {
            set expected {of format SLRx}
            if {$cfg_name == {wizard_auto_config}} {
                set dict_path  [lrange $param_path 0 end-1]
                set dict_name  [join $dict_path {.}]
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.verify.dna_read
############################################################################################################
proc validate_cu_configuration_verify_dna_read { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {auto true false}

    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not $expected"]
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}

############################################################################################################
# cu_configuration.power
############################################################################################################
proc validate_cu_configuration_power { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<SLR0 config>}, "1": {<SLR1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    foreach {key} [dict keys $param_value] {
        if {![string is integer $key]} {
            set expected {integer numbers}
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "slr_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.power.<slr_idx>.throttle_mode
############################################################################################################
proc validate_cu_configuration_power_throttle_mode { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {default INTERNAL_MACRO INTERNAL_CLK EXTERNAL_MACRO EXTERNAL_CLK}
    set slr_idx         [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $slr_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt
############################################################################################################
proc validate_cu_configuration_gt { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GT0 config>}, "1": {<GT1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name platform gt]]]]

    foreach {key} [dict keys $param_value] {
        if {[lsearch -exact $expected_values $key] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt.<gt_idx>.diff_clocks
############################################################################################################
proc validate_cu_configuration_gt_diff_clocks { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set gt_idx      [lindex $param_path0 2]
    if {$param_value == {}} {
        set expected {non-empty value}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt.<gt_idx>.serial_port
############################################################################################################
proc validate_cu_configuration_gt_serial_port { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set gt_idx      [lindex $param_path0 2]
    if {$param_value == {}} {
        set expected {non-empty value}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt_mac
############################################################################################################
proc validate_cu_configuration_gt_mac { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GT_MAC0 config>}, "1": {<GT_MAC1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name platform gt]]]]

    foreach {key} [dict keys $param_value] {
        if {[lsearch -exact $expected_values $key] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt_mac.<gt_idx>.ip_sel
############################################################################################################
proc validate_cu_configuration_gt_mac_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {xxv xbtest_sub_xxv_gt}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not in $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt_mac.<gt_idx>.enable_rsfec
############################################################################################################
proc validate_cu_configuration_gt_mac_enable_rsfec { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {true false}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not in $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt_prbs
############################################################################################################
proc validate_cu_configuration_gt_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GT_PRBS0 config>}, "1": {<GT_PRBS1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name platform gt]]]]

    foreach {key} [dict keys $param_value] {
        if {[lsearch -exact $expected_values $key] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gt_prbs.<gt_idx>.ip_sel
############################################################################################################
proc validate_cu_configuration_gt_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {gt_wiz xbtest_sub_gt}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not in $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gtf_prbs
############################################################################################################
proc validate_cu_configuration_gtf_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GTF_PRBS0 config>}, "1": {<GTF_PRBS1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name platform gt]]]]

    foreach {key} [dict keys $param_value] {
        if {[lsearch -exact $expected_values $key] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gtm_prbs
############################################################################################################
proc validate_cu_configuration_gtm_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GTM_PRBS0 config>}, "1": {<GTM_PRBS1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name platform gt]]]]

    foreach {key} [dict keys $param_value] {
        if {[lsearch -exact $expected_values $key] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gtyp_prbs
############################################################################################################
proc validate_cu_configuration_gtyp_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"0": {<GTYP_PRBS0 config>}, "1": {<GTYP_PRBS1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # ERROR invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name platform gt]]]]

    foreach {key} [dict keys $param_value] {
        if {[lsearch -exact $expected_values $key] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-32} [list $key $param_name "gt_idx key ($key) is not $expected"]
                dict_unset config [concat $param_path [list $key]]
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gtf_prbs.<gt_idx>.ip_sel
############################################################################################################
proc validate_cu_configuration_gtf_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {gtf_wiz xbtest_sub_gt}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not in $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gtm_prbs.<gt_idx>.ip_sel
############################################################################################################
proc validate_cu_configuration_gtm_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {xbtest_sub_gt}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not in $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.gtyp_prbs.<gt_idx>.ip_sel
############################################################################################################
proc validate_cu_configuration_gtyp_prbs_ip_sel { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {xbtest_sub_gt}
    set gt_idx          [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-1]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $gt_idx $dict_name "parameter $param_name ($param_value) is not in $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory
############################################################################################################
proc validate_cu_configuration_memory { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]

    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"<memory_type_0>": {<memory_type_0 config>}, "<memory_type_1 name>": {<memory_type_1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($param_value) is not a valid dictionary. Expecting $expected"]; # invalid dict
            dict_unset config $param_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    # No check dict keys and values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.global.target
############################################################################################################
proc validate_cu_configuration_memory_global_target { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {board host}
    set memory_type     [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-2]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_data_size
############################################################################################################
proc validate_cu_configuration_memory_global_axi_data_size { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {512 256 128 64}
    set memory_type     [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-2]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_id_threads
############################################################################################################
proc validate_cu_configuration_memory_global_axi_id_threads { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {1 2 4 8 16}
    set memory_type     [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-2]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.global.axi_outstanding
############################################################################################################
proc validate_cu_configuration_memory_global_axi_outstanding { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values {1 2 4 8 16 32 64 128 256}
    set memory_type     [lindex $param_path0 2]
    if {[lsearch -exact $expected_values $param_value] == -1} {
        set expected "in \{$expected_values\}"
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-2]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid value
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.specific
############################################################################################################
proc validate_cu_configuration_memory_specific { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set memory_type [lindex $param_path0 2]
    if {[expr [llength $param_value] % 2] != 0} {
        set expected {{"<cu_idx_0>": {<cu_idx_0 config>}, "<cu_idx_1>": {<cu_idx_1 config>}}}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-3]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-33} [list $memory_type $dict_name "parameter $param_name ($param_value) is not a valid dictionary. Expecting $expected"]; # invalid dict
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-19} [list $param_name $expected]; # ERROR invalid dict
            return 0
        }
    }
    foreach {key} [dict keys $param_value] {
        if {![string is integer $key]} {
            set expected {integer numbers}
            if {$cfg_name == {wizard_auto_config}} {
                set dict_path  [lrange $param_path 0 end-3]
                set dict_name  [join $dict_path {.}]
                log_message $config {XBTEST_WIZARD-33} [list $memory_type $dict_name "parameter $param_name cu_idx key ($key) is not $expected"]
                dict_unset config $dict_path
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-20} [list $param_name $key $expected]; # ERROR invalid key
                return 0
            }
        }
    }
    set num_cu [llength [dict keys $param_value]]
    foreach {cu_idx} [dict keys $param_value] {
        set num_sptag [llength [dict get $param_value $cu_idx sptag]]
        if {($num_cu > 1) && ($num_sptag > 1)} {
            set expected {one sptag for one or more CU, or one or more sptag for one CU}
            if {$cfg_name == {wizard_auto_config}} {
                set dict_path  [lrange $param_path 0 end-3]
                set dict_name  [join $dict_path {.}]
                log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not valid. Expecting $expected"]; # invalid dict
                dict_unset config $dict_path
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $param_value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    # No check on dict values
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.specific.<cu_idx>.slr
############################################################################################################
proc validate_cu_configuration_memory_specific_slr { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set memory_type [lindex $param_path0 2]
    if {[regexp {^SLR([0-9])$} $param_value] == 0} {
        set expected {of format SLRx}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-3]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid key
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_configuration.memory.<name>.specific.<cu_idx>.sptag
############################################################################################################
proc validate_cu_configuration_memory_specific_sptag { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path  [concat [list $cfg_name] $param_path0]
    set param_name  [join $param_path {.}]
    set param_value [dict_get_quiet $config $param_path]
    set memory_type [lindex $param_path0 2]
    if {$param_value == {}} {
        set expected {non-empty value}
        if {$cfg_name == {wizard_auto_config}} {
            set dict_path  [lrange $param_path 0 end-2]
            set dict_name  [join $dict_path {.}]
            log_message $config {XBTEST_WIZARD-32} [list $memory_type $dict_name "parameter $param_name ($param_value) is not $expected"]
            dict_unset config $dict_path
            return 1
        } else {
            log_message $config {XBTEST_WIZARD-24} [list $param_value $param_name $expected]; # ERROR invalid key
            return 0
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.power
############################################################################################################
proc validate_cu_selection_power { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration power]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.gt_mac
############################################################################################################
proc validate_cu_selection_gt_mac { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration gt_mac]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.gt_lpbk
############################################################################################################
proc validate_cu_selection_gt_lpbk { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration gt]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.gt_prbs
############################################################################################################
proc validate_cu_selection_gt_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration gt]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.gtf_prbs
############################################################################################################
proc validate_cu_selection_gtf_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration gt]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.gtm_prbs
############################################################################################################
proc validate_cu_selection_gtm_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration gt]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.gtyp_prbs
############################################################################################################
proc validate_cu_selection_gtyp_prbs { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -integer [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration gt]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
############################################################################################################
# cu_selection.memory
############################################################################################################
proc validate_cu_selection_memory { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path      [concat [list $cfg_name] $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set expected_values [lsort -dictionary [dict keys [dict_get_quiet $config [list $cfg_name cu_configuration memory]]]]

    foreach {value} $param_value {
        if {[lsearch -exact $expected_values $value] == -1} {
            set expected "in \{$expected_values\}"
            if {$cfg_name == {wizard_auto_config}} {
                log_message $config {XBTEST_WIZARD-33} [list $param_name "value ($value) is not $expected"]
                dict_set config $param_path {}
                return 1
            } else {
                log_message $config {XBTEST_WIZARD-23} [list $value $param_name $expected]; # ERROR invalid value
                return 0
            }
        }
    }
    log_message $config {XBTEST_WIZARD-22} [list $cfg_name $param_name $param_value]; # validation successful, Display actual value
    return 1; # Valid value
}
