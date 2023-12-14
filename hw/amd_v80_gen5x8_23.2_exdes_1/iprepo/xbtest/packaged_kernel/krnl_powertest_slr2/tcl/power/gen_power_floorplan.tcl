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

set_msg_config -id {GEN_POWER_FLOORPLAN 18} -limit 1500

#########################################################################################
# Global variables
#########################################################################################
set FF_BEL_DEF     {AFF AFF2 BFF BFF2 CFF CFF2 DFF DFF2 EFF EFF2 FFF FFF2 GFF GFF2 HFF HFF2}
set LUT_BEL_DEF    {A5LUT A6LUT B5LUT B6LUT C5LUT C6LUT D5LUT D6LUT E5LUT E6LUT F5LUT F6LUT G5LUT G6LUT H5LUT H6LUT}

# TODO is PWR_SITES the same for DSP48E2_XxYy and DSP_XxYy sites
set PWR_SITES [dict create \
    SLICE 0.0011596527 \
    DSP   0.0091391509 \
    BRAM  0.0402298851 \
    URAM  0.0532894737 \
]

set N_SITE_PL_DYNAMIC_GEOMETRY_CR   {}
set N_SITE_AVAILABLE_CR             {}
set N_SITE_ACTUAL_CR                {}
set N_SITE_PL_DYNAMIC_GEOMETRY      {}
set N_SITE_AVAILABLE                {}
set N_SITE_ACTUAL                   {}

set f_log       {}
set f_dbg       {}
set STAT_MSG    {}

set GEN_CONST_DEBUG  0
set GEN_CONST_OUTPUT 0

#########################################################################################
# Runtime functions
#########################################################################################
proc log_message_line {id {arg_in {}}} {
    variable messages_config
    log_message_file [ log_message $messages_config $id $arg_in ]
}
#----------------------------------------------------------------------------------------
proc Print_Stat {SITES_KEY_LIST n_dynamic_geom_type n_available_type n_actual_type} {
    variable PWR_SITES
    log_message_line {GEN_POWER_FLOORPLAN-18} {{\t\t-------------------------------------------------------------------------------}}
    log_message_line {GEN_POWER_FLOORPLAN-18} [list [format {\t\t| %10s | %10s | %10s | %10s | %10s | %10s |} Resource Dynamic Available Usage {Usage %} {Est Pwr W}]]
    log_message_line {GEN_POWER_FLOORPLAN-18} {{\t\t-------------------------------------------------------------------------------}}

    set power_estimated_tot 0.0
    foreach site_key $SITES_KEY_LIST {
        set n_dynamic_geom  [dict get $n_dynamic_geom_type  $site_key ]
        set n_available     [dict get $n_available_type     $site_key ]
        set n_actual        [dict get $n_actual_type        $site_key ]
        set power_estimated     [expr [dict get $PWR_SITES $site_key ] * $n_actual * 1.2]
        set power_estimated_tot [expr $power_estimated_tot + $power_estimated]
        if {$n_available == 0}  { set percent [format {%10s} {/}] } \
        else                    { set percent [format {%10.2f} [expr double(100*$n_actual)/double($n_available)]] }
        log_message_line {GEN_POWER_FLOORPLAN-18} [list [format {\t\t| %10s | %10d | %10d | %10d | %s | %10.2f |} $site_key $n_dynamic_geom $n_available $n_actual $percent $power_estimated]]
        log_message_line {GEN_POWER_FLOORPLAN-18} {{\t\t-------------------------------------------------------------------------------}}
    }
    log_message_line {GEN_POWER_FLOORPLAN-18} [list [format {\t\tTotal estimated power: %.2f W} $power_estimated_tot]]
}
#----------------------------------------------------------------------------------------
proc Report_Stat {SLR} {
    variable N_SITE_PL_DYNAMIC_GEOMETRY; variable N_SITE_AVAILABLE; variable N_SITE_ACTUAL
    # Display total statistics
    log_message_line {GEN_POWER_FLOORPLAN-18} {{Total utilization:}}
    set n_pl_geom_total_type    {}
    set n_available_total_type  {}
    set n_actual_total_type     {}
    foreach {site_key N_SITE_PL_DYNAMIC_GEOMETRY_SITE} $N_SITE_PL_DYNAMIC_GEOMETRY {
        dict set n_pl_geom_total_type   $site_key 0
        dict set n_available_total_type $site_key 0
        dict set n_actual_total_type    $site_key 0
        foreach idx_slr $SLR {
            dict incr n_pl_geom_total_type      $site_key [dict get $N_SITE_PL_DYNAMIC_GEOMETRY_SITE    $idx_slr]
            dict incr n_available_total_type    $site_key [dict get $N_SITE_AVAILABLE                   $site_key $idx_slr]
            dict incr n_actual_total_type       $site_key [dict get $N_SITE_ACTUAL                      $site_key $idx_slr]
        }
    }
    Print_Stat [dict keys $N_SITE_PL_DYNAMIC_GEOMETRY] $n_pl_geom_total_type $n_available_total_type $n_actual_total_type

    # Display statistics per SLR
    foreach idx_slr $SLR {
        log_message_line {GEN_POWER_FLOORPLAN-18} [list "SLR$idx_slr utilization:"]
        set n_pl_geom_type      {}
        set n_available_type    {}
        set n_actual_type       {}
        foreach {site_key N_SITE_PL_DYNAMIC_GEOMETRY_SITE} $N_SITE_PL_DYNAMIC_GEOMETRY {
            dict set n_pl_geom_type     $site_key [dict get $N_SITE_PL_DYNAMIC_GEOMETRY_SITE    $idx_slr]
            dict set n_available_type   $site_key [dict get $N_SITE_AVAILABLE                   $site_key $idx_slr]
            dict set n_actual_type      $site_key [dict get $N_SITE_ACTUAL                      $site_key $idx_slr]
        }
        Print_Stat [dict keys $N_SITE_PL_DYNAMIC_GEOMETRY] $n_pl_geom_type $n_available_type $n_actual_type
    }
}
#----------------------------------------------------------------------------------------
proc report_utilization { resource area } {
    variable SLR; variable CLOCK_REGIONS
    variable N_SITE_PL_DYNAMIC_GEOMETRY_CR; variable N_SITE_AVAILABLE_CR; variable N_SITE_ACTUAL_CR;
    variable N_SITE_PL_DYNAMIC_GEOMETRY; variable N_SITE_AVAILABLE; variable N_SITE_ACTUAL;

    if {$resource == {help}} {
        puts {report_utilization                                                                    }
        puts {                                                                                      }
        puts {Description:                                                                          }
        puts {Report the utilization of the power kernel                                            }
        puts {                                                                                      }
        puts {Syntax:                                                                               }
        puts {report_utilization <resource> <area>                                                  }
        puts {                                                                                      }
        puts {Usage:                                                                                }
        puts {  Name         Description                                                            }
        puts {  ------------------------                                                            }
        puts {  <resource>   (Optional) Defines the resource to be reported e.g all SLICE or DSP    }
        puts {  <area>       (Optional) Defines the area to be reported:                            }
        puts {                  SLR: The utilization will be reported per SLR                       }
        puts {                  CR : The utilization will be reported per Clock Region              }
        puts {                                                                                      }
        puts {Examples:                                                                             }
        puts {                                                                                      }
        puts {  The following are some examples utilization report:                                 }
        puts {                                                                                      }
        puts {    report_utilization all    SLR                                                     }
        puts {    report_utilization SLICE  SLR                                                     }
        puts {    report_utilization DSP    CR                                                      }
    } else {
        set failure 1; # Check resource
        foreach site_key [dict keys $N_SITE_PL_DYNAMIC_GEOMETRY] {
            if {$site_key == $resource} {set failure 0}
        }
        if {$resource == {all}} {set failure 0}
        if {$failure} {
            log_message_line {GEN_POWER_FLOORPLAN-16} [list {<resource>} $resource]
        }

        set failure 1; # Check area
        foreach area_tmp {SLR CR valid} {
            if {$area_tmp == $area} {set failure 0}
        }
        if {$failure} {
            log_message_line {GEN_POWER_FLOORPLAN-16} [list {<area>} $area]
        }

        # Display
        if {$resource == {all}} { set SITE_KEY_LIST [dict keys $N_SITE_PL_DYNAMIC_GEOMETRY]} \
        else { set SITE_KEY_LIST [list $resource] }

        if {$area == {SLR}} {
            # Display statistics per SLR
            foreach idx_slr $SLR {
                log_message_line {GEN_POWER_FLOORPLAN-18} [list "SLR$idx_slr utilization:"]
                set n_dynamic_geom_type {}
                set n_available_type    {}
                set n_actual_type       {}
                foreach site_key $SITE_KEY_LIST {
                    if {[dict exists $N_SITE_PL_DYNAMIC_GEOMETRY $site_key $idx_slr]} {
                        dict set n_dynamic_geom_type $site_key [dict get $N_SITE_PL_DYNAMIC_GEOMETRY $site_key $idx_slr]
                    } else {
                        dict set n_dynamic_geom_type $site_key 0
                    }
                    if {[dict exists $N_SITE_AVAILABLE $site_key $idx_slr]} {
                        dict set n_available_type $site_key [dict get $N_SITE_AVAILABLE $site_key $idx_slr]
                    } else {
                        dict set n_available_type $site_key 0
                    }
                    if {[dict exists $N_SITE_ACTUAL $site_key $idx_slr]} {
                        dict set n_actual_type $site_key [dict get $N_SITE_ACTUAL $site_key $idx_slr]
                    } else {
                        dict set n_actual_type $site_key 0
                    }
                }
                Print_Stat $SITE_KEY_LIST $n_dynamic_geom_type $n_valid_type $n_available_type $n_actual_type
            }
        } elseif {$area == {CR}} {
            # Display statistics per SLR
            foreach {idx_slr CLOCK_REGIONS_SLR} $CLOCK_REGIONS {
                foreach {cr_y CLOCK_REGIONS_SLR_Y} $CLOCK_REGIONS_SLR {
                    foreach cr_x $CLOCK_REGIONS_SLR_Y {
                        log_message_line {GEN_POWER_FLOORPLAN-18} [list "CR X${cr_x}Y${cr_y} (SLR$idx_slr) utilization:"]
                        set n_dynamic_geom_type {}
                        set n_available_type    {}
                        set n_actual_type       {}
                        foreach site_key $SITE_KEY_LIST {
                            if {[dict exists $N_SITE_PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x]} {
                                dict set n_dynamic_geom_type $site_key [dict get $N_SITE_PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x]
                            } else {
                                dict set n_dynamic_geom_type $site_key 0
                            }
                            if {[dict exists $N_SITE_AVAILABLE $site_key $idx_slr $cr_y $cr_x]} {
                                dict set n_available_type $site_key [dict get $N_SITE_AVAILABLE $site_key $idx_slr $cr_y $cr_x]
                            } else {
                                dict set n_available_type $site_key 0
                            }
                            if {[dict exists $N_SITE_ACTUAL $site_key $idx_slr $cr_y $cr_x]} {
                                dict set n_actual_type $site_key [dict get $N_SITE_ACTUAL $site_key $idx_slr $cr_y $cr_x]
                            } else {
                                dict set n_actual_type $site_key 0
                            }
                        }
                        Print_Stat $report_resource_tmp $n_dynamic_geom_type $n_valid_type $n_available_type $n_actual_type
                    }
                }
            }
        }
    }
}
#----------------------------------------------------------------------------------------
proc log_message_file { msg } {
    variable f_log; variable GEN_CONST_OUTPUT

    if {$GEN_CONST_OUTPUT} {
        if {$f_log == {}} { set f_log [open [file join output gen_constraints.log] w] }
        puts $f_log $msg; flush $f_log
    }
}
#----------------------------------------------------------------------------------------
proc Close_Log {} {
    variable f_log
    if {$f_log != {}} { close $f_log; set f_log {} }
}
#----------------------------------------------------------------------------------------
proc Close_Dbg {} {
    variable f_dbg
    if {$f_dbg != {}} { close $f_dbg; set f_dbg {} }
}
#----------------------------------------------------------------------------------------
proc Close_Files {} {
    Close_Log
    Close_Dbg
}
#----------------------------------------------------------------------------------------
proc Setup_Path {} {
    variable GEN_CONST_DEBUG; variable GEN_CONST_OUTPUT

    # delete directory along with everything in that directory, recursively
    if {$GEN_CONST_OUTPUT} {
        set dir_name output; file delete -force -- $dir_name; file mkdir $dir_name
    }
    if {$GEN_CONST_DEBUG} {
        set dir_name debug; file delete -force -- $dir_name; file mkdir $dir_name
    }
}
#########################################################################################
# Debug functions
#########################################################################################
proc Dbg_Write_List {LIST LISTE_NAME_STR} {
    variable GEN_CONST_DEBUG; variable f_dbg

    if {$GEN_CONST_DEBUG} {
        set f_dbg [open [file join debug ${LISTE_NAME_STR}.dbg] w]
        set DATA {}
        if {[catch {dict with LIST {}}]} {
            lappend DATA $LIST
        } else {
            foreach key0 [lsort [dict keys $LIST]] {
                lappend DATA {#########################################}
                lappend DATA $key0
                if {[catch {dict with [dict get $LIST $key0] {}}]} {
                    lappend DATA "\t[dict get $LIST $key0]"
                } else {
                    foreach key1 [lsort [dict keys [dict get $LIST $key0]]] {
                        lappend DATA {---------------------------------------}
                        lappend DATA "\t$key1"
                        foreach BLOCK [dict get $LIST $key0 $key1] {
                            if {[catch {dict with BLOCK {}}]} {
                                lappend DATA "\t\t$BLOCK"
                            } else {
                                foreach key2 [lsort [dict keys $BLOCK]] {
                                    lappend DATA "\t\t$key2 [dict get $BLOCK $key2]"
                                }
                            }
                            lappend DATA {}
                        }
                    }
                }
            }
        }
        puts $f_dbg [join $DATA "\n"]; flush $f_dbg; Close_Dbg
    }
}
#----------------------------------------------------------------------------------------
proc Dbg_Mark_Site {SITES_TYPES LIST LIST_STR_NAME site_key} {
    variable GEN_CONST_DEBUG; variable f_dbg

    set site_type [dict get $SITES_TYPES $site_key]

    if     {$site_key == {SLICE}}   { set color blue   } \
    elseif {$site_key == {DSP}}     { set color green  } \
    elseif {$site_key == {BRAM}}    { set color orange } \
    elseif {$site_key == {URAM}}    { set color yellow }

    if {($GEN_CONST_DEBUG) && ([llength $LIST] > 0)} {
        set f_dbg  [open [file join debug ${LIST_STR_NAME}.dbg] a]
        set     DATA {}
        lappend DATA "mark_objects -color $color \[get_sites \{ \\"
        foreach {x COL_X} $LIST {
            set tmp {}
            foreach y $COL_X {
                lappend tmp ${site_type}_X${x}Y${y}
            }
            lappend DATA "[join $tmp { }] \\"
        }
        lappend DATA "\}\]"
        puts $f_dbg [join $DATA "\n"]; flush $f_dbg; Close_Dbg
    }
}
#----------------------------------------------------------------------------------------
proc display_pwr_fp { {resource {all}} {sites {actual}} } {
    variable C_DYNAMIC_GEOMETRY

    if {$resource == {help}} {
        puts {display_pwr_fp                                                                            }
        puts {                                                                                          }
        puts {Description:                                                                              }
        puts {Display the utilization of the power kernel using mark_objects commands.                  }
        puts {The sites of the power kernel will be displayed in different color depending on the type: }
        puts {  - SLICE   : blue                                                                        }
        puts {  - DSP48E2 : green                                                                       }
        puts {  - RAMB36  : orange                                                                      }
        puts {  - URAM288 : yellow                                                                      }
        puts {                                                                                          }
        puts {Syntax:                                                                                   }
        puts {display_pwr_fp <resource>                                                                 }
        puts {                                                                                          }
        puts {Usage:                                                                                    }
        puts {  Name         Description                                                                }
        puts {  ------------------------                                                                }
        puts {  <resource>   (Optional) Defines the resource to be displayed e.g all SLICE or DSP       }
        puts {                                                                                          }
        puts {Examples:                                                                                 }
        puts {                                                                                          }
        puts {  The following are some examples utilization report:                                     }
        puts {                                                                                          }
        puts {    display_pwr_fp all                                                                    }
        puts {    display_pwr_fp SLICE                                                                  }
    } else {

        set SLR             [dict get $C_DYNAMIC_GEOMETRY SLR]
        set CLOCK_REGIONS   [dict get $C_DYNAMIC_GEOMETRY CLOCK_REGIONS]
        set SITES_TYPES     [dict get $C_DYNAMIC_GEOMETRY SITES_TYPES]

        set failure 1; # Check resource
        foreach site_key [dict keys $SITES_TYPES] {
            if {$site_key == $resource} {set failure 0}
        }
        if {$resource == {all}} {set failure 0}
        if {$failure} {
            log_message_line {GEN_POWER_FLOORPLAN-17} [list {<resource>} $resource]
        }

        set failure 1; # Check sites
        foreach sites_tmp {geom valid available actual} {
            if {$sites_tmp == $sites} {set failure 0}
        }
        if {$failure} {
            log_message_line {GEN_POWER_FLOORPLAN-17} [list {<sites>} $sites]
        }

        # Display
        if {$resource == {all}} { set SITE_KEY_LIST [dict keys $SITES_TYPES]} \
        else { set SITE_KEY_LIST [list $resource] }

        foreach site_key $SITE_KEY_LIST {
            foreach idx_slr $SLR {
                set CLOCK_REGIONS_SLR [dict get $CLOCK_REGIONS $idx_slr]
                foreach {cr_y CLOCK_REGIONS_SLR_Y} $CLOCK_REGIONS_SLR {
                    foreach cr_x $CLOCK_REGIONS_SLR_Y {
                        set filename [file join debug mark_slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_${sites}.dbg]
                        if {[file exists $filename]} {
                            source -notrace $filename
                        }
                    }
                }
            }
        }
    }
}
#########################################################################################
# Check function
#########################################################################################
proc Is_Site_In_Col_List {x y COL_LIST} {
    foreach {cr_y COL_LIST_Y} $COL_LIST {
        foreach {cr_x COL_LIST_Y_X} $COL_LIST_Y {
            foreach {col_x COL_LIST_Y_X_COL_X} $COL_LIST_Y_X {
                foreach col_y $COL_LIST_Y_X_COL_X {
                    if {($x == $col_x) && ($y == $col_y)} {
                        return 1
                    }
                }
            }
        }
    }
    return 0
}
#----------------------------------------------------------------------------------------
proc Is_Site_In_Block_List {x y BLOCK_LIST} {
    if {[llength $BLOCK_LIST] ne 0}  {
        foreach BLOCK $BLOCK_LIST {
            if { ($x >= [dict get $BLOCK X_MIN]) && ($x <= [dict get $BLOCK X_MAX])
              && ($y >= [dict get $BLOCK Y_MIN]) && ($y <= [dict get $BLOCK Y_MAX]) } {
                return 1
            }
        }
    }
    return 0
}
#----------------------------------------------------------------------------------------
proc Is_Site_Available {x y site_type idx_slr PL_INVALID} {
    if {[dict exists $PL_INVALID $site_type $idx_slr]} {; # Must fall outside invalid range
        if {[ Is_Site_In_Block_List $x $y [ dict get $PL_INVALID $site_type $idx_slr ] ]} {
            return 0
        }
    }
    return 1
}
#----------------------------------------------------------------------------------------
proc Check_PL_Invalid {SITES_TYPES SLR PL_DYNAMIC_GEOMETRY PL_INVALID} {
    set PL_INVALID_ret {}; # Reformat dictionary
    foreach {slr PL_INVALID_SLR} $PL_INVALID {
        if {[string tolower $slr] == {comment}} { continue }; # Ignore comments

        if {[scan $slr {SLR_%d} idx_slr] != 1} {
            log_message_line {GEN_POWER_FLOORPLAN-10} [list "Cannot extract SLR index from key ($slr) provided"]; #ERROR
        }
        if {[lsearch -exact $SLR $idx_slr] == -1} {
            log_message_line {GEN_POWER_FLOORPLAN-10} [list "Invalid SLR index ${idx_slr} provided in key ($slr). Valid SLR indexes are [join $SLR ", "]"]; #ERROR
        }
        foreach BLOCK $PL_INVALID_SLR {
            if {![dict exists $BLOCK location]} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Location not defined in dictionary ($BLOCK) provided"]; #ERROR
            }

            set location    [dict get $BLOCK location]
            set ret         [scan $location {%[^:]:%s} location_min location_max]
            if {($ret != 1) && ($ret != 2)} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Cannot extract min and max locations from location ($location) provided"]; #ERROR
            } elseif {$ret == 1} {
                set location_max $location_min
            }
            if {[scan $location_min {%[^_X]_X%dY%d} site_type_min x_min y_min] != 3} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Cannot extract site type, X and Y coordinates from location ($location_min) provided"]; #ERROR
            }
            if {[scan $location_max {%[^_X]_X%dY%d} site_type_max x_max y_max] != 3} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Cannot extract site type, X and Y coordinates from location ($location_min) provided"]; #ERROR
            }
            if {$site_type_min != $site_type_max} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Site type for location min ($site_type_min) does not match location max ($site_type_max) for $location"]; #ERROR
            }
            set site_type   $site_type_min
            set site_key    {}
            foreach {k v} $SITES_TYPES {
                if {$site_type == $v} {
                    set site_key $k
                    break
                }
            }
            if {$site_key == {}} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Invalid site type ${site_type} provided for $location. Valid sites are [join [dict values $SITES_TYPES] ", "]"]; #ERROR
            }
            if {$y_min > $y_max} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Incorrect definition of $site_key rectangle $location: Y location min ($y_min) > Y location max ($y_max) in SLR$idx_slr"]; #ERROR
            }
            if {$x_min > $x_max} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Incorrect definition of $site_key rectangle $location: X location min ($x_min) > X location max ($x_max) in SLR$idx_slr"]; #ERROR
            }
            for {set y $y_min} {$y <= $y_max} {incr y} {
                for {set x $x_min} {$x <= $x_max} {incr x} {
                    set valid 0
                    if {[dict exists $PL_DYNAMIC_GEOMETRY $site_key $idx_slr]} {
                        set valid [Is_Site_In_Col_List $x $y [dict get $PL_DYNAMIC_GEOMETRY $site_key $idx_slr]]
                    }
                    if {!$valid} {
                        log_message_line {GEN_POWER_FLOORPLAN-10} [list "$site_key site ${site_type}_X${x}Y${y} not found in FPGA geometry for rectangle ($location) provided in SLR$idx_slr"]; #ERROR
                    }
                }
            }
            if {![dict exists $PL_INVALID_ret $site_key]} {
                dict set PL_INVALID_ret $site_key {}
            }
            if {![dict exists $PL_INVALID_ret $site_key $idx_slr]} {
                dict set PL_INVALID_ret $site_key $idx_slr [list]
            }
            dict with PL_INVALID_ret $site_key  {
                lappend $idx_slr [dict create X_MIN $x_min Y_MIN $y_min X_MAX $x_max Y_MAX $y_max]
            }
        }
    }
    return $PL_INVALID_ret
}
#----------------------------------------------------------------------------------------
proc Check_PL_Utilization {SITES_TYPES SLR CLOCK_REGIONS PL_DYNAMIC_GEOMETRY PL_UTILIZATION} {
    set PL_UTILIZATION_ret {}; # Reformat dictionary
    # Check all utilization corresponds to sites existing in geometry
    foreach {slr PL_UTILIZATION_SLR} $PL_UTILIZATION {
        if {[string tolower $slr] == {comment}} { continue }; # Ignore comments

        if {[scan $slr {SLR_%d} idx_slr] != 1} {
            log_message_line {GEN_POWER_FLOORPLAN-11} [list "Cannot extract SLR index from key ($slr) provided"]; #ERROR
        }
        if {[lsearch -exact $SLR $idx_slr] == -1} {
            log_message_line {GEN_POWER_FLOORPLAN-11} [list "Invalid SLR index ${idx_slr} provided in key ($slr). Valid SLR indexes are [join $SLR ", "]"]; #ERROR
        }

        dict set PL_UTILIZATION_ret $idx_slr {}
        foreach {CR_Y PL_UTILIZATION_SLR_Y} $PL_UTILIZATION_SLR {
            if {[string tolower $CR_Y] == {comment}} { continue }; # Ignore comments

            if {[scan $CR_Y {CR_Y_%d} cr_y] != 1} {
                log_message_line {GEN_POWER_FLOORPLAN-10} [list "Cannot extract CR Y index from key ($CR_Y) provided"]; #ERROR
            }
            if {![dict exists $CLOCK_REGIONS $idx_slr $cr_y]} {
                log_message_line {GEN_POWER_FLOORPLAN-11} [list "Invalid CR Y index ($cr_y) in SLR$idx_slr provided in key ($CR_Y). Valid CR Y indexes for SLR$idx_slr are [join [dict keys [dict get $CLOCK_REGIONS $idx_slr]] ", "]"]; #ERROR
            }

            dict set PL_UTILIZATION_ret $idx_slr $cr_y {}
            set REQUIRED_KEYS   [concat [dict keys $SITES_TYPES] {CR_X}]
            set SUPPORTED_KEYS  [concat $REQUIRED_KEYS {comment}]
            set expected_size   [llength [dict get $CLOCK_REGIONS $idx_slr $cr_y]]

            foreach key $REQUIRED_KEYS {
                if {![dict exists $PL_UTILIZATION_SLR_Y $key]} {
                    log_message_line {GEN_POWER_FLOORPLAN-11} [list "Required list ($key) not defined for CR Y index ($cr_y) in SLR$idx_slr"]; #ERROR
                }
                if {[llength [dict get $PL_UTILIZATION_SLR_Y $key]] != $expected_size} {
                    log_message_line {GEN_POWER_FLOORPLAN-11} [list "Size of provided list ($key) for CR Y index ($cr_y) in SLR$idx_slr does not match expected ($expected_size)"]; #ERROR
                }
            }
            foreach key [dict keys $PL_UTILIZATION_SLR_Y] {
                if {[lsearch -exact $SUPPORTED_KEYS $key] == -1} {
                    log_message_line {GEN_POWER_FLOORPLAN-11} [list "Invalid key ${key} for CR Y index ($cr_y) in SLR$idx_slr provided. Valid keys are [join $SUPPORTED_KEYS ", "]"]; #ERROR
                }
            }
            for {set i 0} {$i < $expected_size} {incr i} {
                set cr_x    [lindex [dict get $PL_UTILIZATION_SLR_Y CR_X] $i]
                if {[lsearch -exact [dict get $CLOCK_REGIONS $idx_slr $cr_y] $cr_x] == -1} {
                    log_message_line {GEN_POWER_FLOORPLAN-11} [list "Invalid CR X index ($cr_x) for CR Y index ($cr_y) in SLR$idx_slr provided. Valid CR X indexes for CR index Y ($cr_y) in SLR$idx_slr are [join [dict get $CLOCK_REGIONS $idx_slr $cr_y] ", "]"]; #ERROR
                }

                dict set PL_UTILIZATION_ret $idx_slr $cr_y $cr_x {}

                foreach site_key [dict keys $SITES_TYPES] {
                    set utilization [lindex [dict get $PL_UTILIZATION_SLR_Y $site_key] $i]
                    if {($utilization > 100) || ($utilization < 0)} {
                        log_message_line {GEN_POWER_FLOORPLAN-11} [list "Utilization ($utilization) not in \\\[0;100\\\] for site $site_key in CR X${cr_x}Y${cr_y} (SLR$idx_slr)"]; #ERROR
                    }
                    dict set PL_UTILIZATION_ret $idx_slr $cr_y $cr_x $site_key $utilization
                }
            }
        }
    }
    # Check an utilization is defined for all sites in geometry
    foreach {site_key PL_DYNAMIC_GEOMETRY_SITE} $PL_DYNAMIC_GEOMETRY {
        foreach {idx_slr PL_DYNAMIC_GEOMETRY_SITE_SLR} $PL_DYNAMIC_GEOMETRY_SITE {
            foreach {cr_y PL_DYNAMIC_GEOMETRY_SITE_SLR_Y} $PL_DYNAMIC_GEOMETRY_SITE_SLR {
                foreach {cr_x PL_DYNAMIC_GEOMETRY_SITE_SLR_Y_X} $PL_DYNAMIC_GEOMETRY_SITE_SLR_Y {
                    if {![dict exists $PL_UTILIZATION_ret $idx_slr $cr_y $cr_x $site_key]} {
                        log_message_line {GEN_POWER_FLOORPLAN-11} [list "No utilization defined for site type ${site_key} in CR X${cr_x}Y${cr_y} (SLR${idx_slr})"]; #ERROR
                    }
                }
            }
        }
    }
    return $PL_UTILIZATION_ret
}
#----------------------------------------------------------------------------------------
proc Get_Site_SLR {site_type_in col_in row_in PL_DYNAMIC_GEOMETRY} {
    set site_slr -1
    if {[dict exists $PL_DYNAMIC_GEOMETRY $site_type_in]} {
        set PL_DYNAMIC_GEOMETRY_SITE [dict get $PL_DYNAMIC_GEOMETRY $site_type_in]
        foreach {idx_slr PL_DYNAMIC_GEOMETRY_SITE_SLR} $PL_DYNAMIC_GEOMETRY_SITE {
            if {[Is_Site_In_Col_List $col_in $row_in $PL_DYNAMIC_GEOMETRY_SITE_SLR]} {
                set site_slr $idx_slr; break
            }
        }
    }
    return $site_slr
}
#########################################################################################
# Generate Actual Sites for DSP, BRAM && URAM
#########################################################################################
proc Generate_Actual_Geom_List {SITES_TYPES SLR PL_DYNAMIC_GEOMETRY PL_UTILIZATION PL_INVALID} {
    variable N_SITE_PL_DYNAMIC_GEOMETRY_CR; variable N_SITE_AVAILABLE_CR; variable N_SITE_ACTUAL_CR
    variable N_SITE_PL_DYNAMIC_GEOMETRY; variable N_SITE_AVAILABLE; variable N_SITE_ACTUAL

    # ACTUAL_GEOM_LIST[site_key][idx_slr][cr_y][cr_x][site_x]={site_y}
    set ACTUAL_GEOM_LIST {}

    foreach {site_key PL_DYNAMIC_GEOMETRY_SITE} $PL_DYNAMIC_GEOMETRY {
        log_message_line {GEN_POWER_FLOORPLAN-12} [list $site_key]

        dict set ACTUAL_GEOM_LIST               $site_key {}
        dict set N_SITE_PL_DYNAMIC_GEOMETRY_CR  $site_key {}
        dict set N_SITE_AVAILABLE_CR            $site_key {}
        dict set N_SITE_ACTUAL_CR               $site_key {}
        dict set N_SITE_PL_DYNAMIC_GEOMETRY     $site_key {}
        dict set N_SITE_AVAILABLE               $site_key {}
        dict set N_SITE_ACTUAL                  $site_key {}

        foreach idx_slr $SLR {; # for all SLR
            log_message_line {GEN_POWER_FLOORPLAN-13} [list $idx_slr]

            # Number of sites
            dict set N_SITE_PL_DYNAMIC_GEOMETRY_CR  $site_key $idx_slr {}
            dict set N_SITE_AVAILABLE_CR            $site_key $idx_slr {}
            dict set N_SITE_ACTUAL_CR               $site_key $idx_slr {}
            dict set N_SITE_PL_DYNAMIC_GEOMETRY     $site_key $idx_slr 0
            dict set N_SITE_AVAILABLE               $site_key $idx_slr 0
            dict set N_SITE_ACTUAL                  $site_key $idx_slr 0

            # Init
            if {![dict exists $PL_DYNAMIC_GEOMETRY_SITE $idx_slr]} {
                log_message_line {GEN_POWER_FLOORPLAN-21} [list $site_key $idx_slr]
                continue
            }
            set PL_DYNAMIC_GEOMETRY_SITE_SLR [dict get $PL_DYNAMIC_GEOMETRY_SITE $idx_slr]
            dict set ACTUAL_GEOM_LIST  $site_key $idx_slr {}

            foreach {cr_y DYNAMIC_GEOMETRY_SITE_SLR_Y} $PL_DYNAMIC_GEOMETRY_SITE_SLR {
                dict set ACTUAL_GEOM_LIST               $site_key $idx_slr $cr_y {}
                dict set N_SITE_PL_DYNAMIC_GEOMETRY_CR  $site_key $idx_slr $cr_y {}
                dict set N_SITE_AVAILABLE_CR            $site_key $idx_slr $cr_y {}
                dict set N_SITE_ACTUAL_CR               $site_key $idx_slr $cr_y {}

                foreach {cr_x DYNAMIC_GEOMETRY_SITE_SLR_Y_X} $DYNAMIC_GEOMETRY_SITE_SLR_Y {
                    dict set ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x {}
                    set SITE_DYNAMIC_GEOMETRY_LIST  {}; # list of all site columns in CR
                    set SITE_AVAILABLE_LIST         {}; # list of all valid site columns in CR (SITE_DYNAMIC_GEOMETRY_LIST minus PL_INVALID)
                    set col_found 0

                    foreach {x COL_X} $DYNAMIC_GEOMETRY_SITE_SLR_Y_X {
                        dict set SITE_DYNAMIC_GEOMETRY_LIST $x $COL_X; # All Site List
                        foreach y $COL_X {
                            set col_found 1
                            # Available Site List
                            if {[Is_Site_Available $x $y $site_key $idx_slr $PL_INVALID]} {
                                if {[dict exists $SITE_AVAILABLE_LIST $x]} {
                                    dict with SITE_AVAILABLE_LIST {
                                        lappend $x $y
                                    }
                                } else {
                                    dict set SITE_AVAILABLE_LIST $x [list $y]
                                }
                            }
                        }
                    }
                    Dbg_Write_List                       $SITE_DYNAMIC_GEOMETRY_LIST   slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_geom
                    Dbg_Write_List                       $SITE_AVAILABLE_LIST          slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_available
                    Dbg_Mark_Site $SITES_TYPES $SITE_DYNAMIC_GEOMETRY_LIST   mark_slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_geom      $site_key
                    Dbg_Mark_Site $SITES_TYPES $SITE_AVAILABLE_LIST          mark_slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_available $site_key

                    set ii 0; # Sort site from bottom to top
                    set SORTED_SITE_AVAILABLE_LIST  {}; # list of all site columns in CR
                    foreach x [lsort -integer -increasing [dict keys $SITE_AVAILABLE_LIST]] {
                        if {($ii%2 == 0) && ($site_key == {SLICE})} { # for SLICE, sort keys in the reverted order 1 column / 2
                            set SORTED_SITE_AVAILABLE_LIST_Y [lsort -integer -decreasing [dict get $SITE_AVAILABLE_LIST $x]];
                        } else {
                            set SORTED_SITE_AVAILABLE_LIST_Y [lsort -integer -increasing [dict get $SITE_AVAILABLE_LIST $x]];
                        }
                        dict set SORTED_SITE_AVAILABLE_LIST $x $SORTED_SITE_AVAILABLE_LIST_Y
                        incr ii
                    }

                    set percent [dict get $PL_UTILIZATION $idx_slr $cr_y $cr_x $site_key]

                    set SITE_ACTUAL_LIST {}; # list of all actually used site columns in CR
                    if {$col_found} {; # Continue if at least one column is found in Clock region
                        if {$site_key == {SLICE}} {
                            set jj 0; # Create Actual Block List, leaving "gaps" in SLICE
                            foreach {x COL_X} $SORTED_SITE_AVAILABLE_LIST {
                                foreach y $COL_X {
                                    if {$jj < $percent} {
                                        if {[dict exists $ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x $x]} {
                                            dict with ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x {
                                                lappend $x $y
                                            }
                                        } else {
                                            dict set ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x $x [list $y]
                                        }
                                    }
                                    if {$jj == 99} { set jj 0 } else { incr jj }
                                }
                            }

                        } else {
                            foreach {x COL_X} $SORTED_SITE_AVAILABLE_LIST {
                                set n_max [expr int(ceil(double([llength $COL_X]) * double($percent) / 100.0))]
                                if  {$n_max > [llength $COL_X]} { set n_max [llength $COL_X] }

                                set jj 0; # Create Actual Block List, leaving "gaps" in SLICE
                                foreach y $COL_X {
                                    if {$jj == $n_max} { break } else { incr jj }
                                    if {[dict exists $ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x $x]} {
                                        dict with ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x {
                                            lappend $x $y
                                        }
                                    } else {
                                        dict set ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x $x [list $y]
                                    }
                                }
                            }
                        }
                        Dbg_Write_List                        [dict get $ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x] slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_actual
                        Dbg_Mark_Site  $SITES_TYPES [dict get $ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x] mark_slr${idx_slr}_CR_X${cr_x}Y${cr_y}_${site_key}_actual $site_key
                    }

                    set n_dynamic_geometry 0
                    set n_available 0
                    set n_actual 0
                    foreach {x COL_X} $SITE_DYNAMIC_GEOMETRY_LIST   { incr n_dynamic_geometry   [llength $COL_X] }
                    foreach {x COL_X} $SITE_AVAILABLE_LIST          { incr n_available          [llength $COL_X] }
                    foreach {x COL_X} [dict get $ACTUAL_GEOM_LIST $site_key $idx_slr $cr_y $cr_x] {
                        incr n_actual [llength $COL_X]
                    }
                    dict set N_SITE_PL_DYNAMIC_GEOMETRY_CR  $site_key $idx_slr $cr_y $cr_x $n_dynamic_geometry
                    dict set N_SITE_AVAILABLE_CR            $site_key $idx_slr $cr_y $cr_x $n_available
                    dict set N_SITE_ACTUAL_CR               $site_key $idx_slr $cr_y $cr_x $n_actual

                    dict with N_SITE_PL_DYNAMIC_GEOMETRY    $site_key {incr $idx_slr $n_dynamic_geometry}
                    dict with N_SITE_AVAILABLE              $site_key {incr $idx_slr $n_available}
                    dict with N_SITE_ACTUAL                 $site_key {incr $idx_slr $n_actual}

                    lappend STAT_MSG {#----------------------------------------------------}
                    lappend STAT_MSG "# Clock Region X${cr_x}Y${cr_y} (SLR${idx_slr}) stats for site ${site_key}:"
                    lappend STAT_MSG "#\t- Valid       : $n_dynamic_geometry"
                    lappend STAT_MSG "#\t- Available   : $n_available"
                    lappend STAT_MSG "#\t- Actual      : $n_actual"
                    lappend STAT_MSG "#\t- Percent     : $percent %%"
                }
            }
        }
    }
    Report_Stat $SLR; # Print Statistics

    Dbg_Write_List $ACTUAL_GEOM_LIST ACTUAL_GEOM_LIST
    return $ACTUAL_GEOM_LIST
}
#########################################################################################
# Generate column constraints
#########################################################################################
proc Generate_Col_Constraints { SITES_TYPES ACTUAL_GEOM_LIST } {
    variable STAT_MSG

    set COL_CONSTRAINTS_LIST  {}
    lappend STAT_MSG {############################### Column Stats ###############################}
    foreach {site_key ACTUAL_GEOM_LIST_SITE} $ACTUAL_GEOM_LIST {
        if {$site_key == {SLICE}} { continue }; # Do not process SLICE sites as columns

        set site_type [dict get $SITES_TYPES $site_key]
        dict set COL_CONSTRAINTS_LIST  $site_key {}

        foreach {idx_slr ACTUAL_GEOM_LIST_SITE_SLR} $ACTUAL_GEOM_LIST_SITE {
            dict set COL_CONSTRAINTS_LIST $site_key $idx_slr {}

            set idx 0
            foreach {cr_y ACTUAL_GEOM_LIST_SITE_SLR_Y} $ACTUAL_GEOM_LIST_SITE_SLR {
                foreach {cr_x ACTUAL_GEOM_LIST_SITE_SLR_Y_X} $ACTUAL_GEOM_LIST_SITE_SLR_Y {
                    foreach {x COL_X} $ACTUAL_GEOM_LIST_SITE_SLR_Y_X {
                        lappend STAT_MSG {#----------------------------------------------------}
                        lappend STAT_MSG "# Clock Region X${cr_x}Y${cr_y} (SLR${idx_slr}) stats for site ${site_key}:"
                        lappend STAT_MSG "# Column  : ${site_type}_X${x}Y*"
                        lappend STAT_MSG "# Index   : ${idx}"
                        lappend STAT_MSG "# Actual  : [llength $COL_X]"

                        if {[llength $COL_X] > 0} {
                            set path_base   [format "inst_krnl_core/inst_powertest_top/inst_%s_top/genblk_col\[%d\].%s_col_inst" [string tolower $site_key] $idx [string tolower $site_key]]
                            set col_constraints {}
                            # ff is present for all type DSP, BRAM & URAM
                            lappend col_constraints [format "set_false_path -from \[get_pins %s/ff_reg/C\]" $path_base]; # TODO see if we should remove this
                            # sleep only exists for BRAM/URAM
                            if {($site_key == {BRAM}) || $site_key == {URAM}} {
                                lappend col_constraints [format "set_false_path -from \[get_pins %s/sleep_reg/C\]" $path_base]
                            }

                            set idx_site 0
                            set SITE_LIST [list]
                            foreach y $COL_X {
                                set site            "${site_type}_X${x}Y${y}"
                                lappend SITE_LIST   "$site"

                                # TODO see here if same SystemVerilog macro can be used for DSP48E2_XxYy and DSP_XxYy
                                if     {$site_key == {DSP}}   { set path_site [format "%s/genblk_dsp\[%d\].u_dsp_el/dsp_inst"       $path_base $idx_site] } \
                                elseif {$site_key == {BRAM}}  { set path_site [format "%s/genblk_bram\[%d\].u_bram_el/bram_inst"    $path_base $idx_site] } \
                                elseif {$site_key == {URAM}}  { set path_site [format "%s/genblk_uram\[%d\].u_uram_el/uram_inst"    $path_base $idx_site] }

                                lappend col_constraints  [format "set_property LOC %s \[get_cells \{%s\}\]" $site $path_site]; # Write LOC property for site
                                incr idx_site
                            }
                            lappend STAT_MSG "# Sites  : [join $SITE_LIST ", "]"

                            dict set COL_CONSTRAINTS_LIST $site_key $idx_slr [concat [dict get $COL_CONSTRAINTS_LIST $site_key $idx_slr] $col_constraints]
                            incr idx
                        }
                        lappend STAT_MSG {#----------------------------------------------------}
                    }
                }
            }
        }
    }
    Dbg_Write_List $COL_CONSTRAINTS_LIST COL_CONSTRAINTS_LIST
    return $COL_CONSTRAINTS_LIST
}
#########################################################################################
# Generate SV column parameters
#########################################################################################
proc Generate_Col_Sizes_Array_List {SITES_TYPES ACTUAL_GEOM_LIST} {
    set COL_SIZES_ARRRAY_LIST {}

    foreach {site_key ACTUAL_GEOM_LIST_SITE} $ACTUAL_GEOM_LIST {
        if {$site_key == {SLICE}} { continue }; # Do not process SLICE sites as columns

        set site_type [dict get $SITES_TYPES $site_key]
        dict set COL_SIZES_ARRRAY_LIST $site_key {}

        foreach {idx_slr ACTUAL_GEOM_LIST_SITE_SLR} $ACTUAL_GEOM_LIST_SITE {
            dict set COL_SIZES_ARRRAY_LIST $site_key $idx_slr {}

            set idx 0
            foreach {cr_y ACTUAL_GEOM_LIST_SITE_SLR_Y} $ACTUAL_GEOM_LIST_SITE_SLR {
                dict set COL_SIZES_ARRRAY_LIST $site_key $idx_slr $cr_y {}

                foreach {cr_x ACTUAL_GEOM_LIST_SITE_SLR_Y_X} $ACTUAL_GEOM_LIST_SITE_SLR_Y {
                    dict set COL_SIZES_ARRRAY_LIST $site_key $idx_slr $cr_y $cr_x {}

                    foreach {x COL_X} $ACTUAL_GEOM_LIST_SITE_SLR_Y_X {
                        if {[llength $COL_X] > 0} {
                            # Get column data
                            set col_name "CR X${cr_x}Y${cr_y}: ${site_type}_X${x}Y*"
                            dict set COL_SIZES_ARRRAY_LIST $site_key $idx_slr $cr_y $cr_x $col_name [list $idx [llength $COL_X]]
                            incr idx
                        }
                    }
                }
            }
        }
    }
    return $COL_SIZES_ARRRAY_LIST
}
#########################################################################################
proc Generate_Pkg_Col_Sizes_Array_List {COL_SIZES_ARRRAY_LIST} {
    set PKG_COL_SIZES_ARRRAY_LIST {}

    foreach {site_key COL_SIZES_ARRRAY_LIST_TYPE} $COL_SIZES_ARRRAY_LIST {
        dict set PKG_COL_SIZES_ARRRAY_LIST $site_key {}
        # TODO rename SystemVerilog C_%s_COL_SIZES_ARRRAY with site_key
        if     {$site_key == {DSP}}  { set tmp DSP48E2 } \
        elseif {$site_key == {BRAM}} { set tmp RAMB36  } \
        elseif {$site_key == {URAM}} { set tmp URAM288 }

        foreach {idx_slr COL_SIZES_ARRRAY_LIST_TYPE_SLR} $COL_SIZES_ARRRAY_LIST_TYPE {
            set col_sizes_array {}
            lappend col_sizes_array [format "\tlocalparam integer C_%s_COL_SIZES_ARRRAY \[0:199\] = '\{ // Set to large size but only lower indexes used" $tmp]

            foreach {cr_y COL_SIZES_ARRRAY_LIST_TYPE_SLR_Y} $COL_SIZES_ARRRAY_LIST_TYPE_SLR {
                foreach {cr_x COL_SIZES_ARRRAY_LIST_TYPE_SLR_Y_X} $COL_SIZES_ARRRAY_LIST_TYPE_SLR_Y {
                    foreach {col_name COL_SIZES_ARRRAY_LIST_BLOCK} $COL_SIZES_ARRRAY_LIST_TYPE_SLR_Y_X {; # For all column of DSP, BRAM and URAM in geometry
                        set idx         [lindex $COL_SIZES_ARRRAY_LIST_BLOCK 0]
                        set num_site    [lindex $COL_SIZES_ARRRAY_LIST_BLOCK 1]
                        lappend col_sizes_array [format "\t\t%d:%d, // %s" $idx $num_site $col_name]; # Get column data
                    }
                }
            }
            lappend col_sizes_array "\t\tdefault:0 // set unused indexes to 0"
            lappend col_sizes_array "\t\};"
            dict set PKG_COL_SIZES_ARRRAY_LIST $site_key $idx_slr $col_sizes_array
        }
    }
    return $PKG_COL_SIZES_ARRRAY_LIST
}
#########################################################################################
# Generate Slice constraints
#########################################################################################
proc Generate_Slice_Constraints_List { SITES_TYPES ACTUAL_SITE_LIST } {
    variable FF_BEL_DEF; variable LUT_BEL_DEF; variable STAT_MSG
    set site_key    "SLICE"
    set site_type   [dict get $SITES_TYPES $site_key]

    set SLICE_CONSTRAINTS_LIST {}
    dict set SLICE_CONSTRAINTS_LIST $site_key {}
    set path_base {inst_krnl_core/inst_powertest_top}

    set ACTUAL_SITE_LIST_TYPE [dict get $ACTUAL_SITE_LIST $site_key]
    foreach {idx_slr ACTUAL_GEOM_LIST_SITE_SLR} $ACTUAL_SITE_LIST_TYPE {; # for all SLR
        # Write false path constraints
        set slice_constraints {}
        foreach {cr_y ACTUAL_GEOM_LIST_SITE_SLR_Y} $ACTUAL_GEOM_LIST_SITE_SLR {
            foreach {cr_x ACTUAL_GEOM_LIST_SITE_SLR_Y_X} $ACTUAL_GEOM_LIST_SITE_SLR_Y {
                set idx 0
                foreach {x COL_X} $ACTUAL_GEOM_LIST_SITE_SLR_Y_X {
                    lappend STAT_MSG {#----------------------------------------------------}
                    lappend STAT_MSG "# Clock Region X${cr_x}Y${cr_y} (SLR${idx_slr}) stats for site ${site_key}:"
                    lappend STAT_MSG "# Column  : ${site_type}_X${x}Y*"
                    lappend STAT_MSG "# Index   : ${idx}"
                    lappend STAT_MSG "# Actual  : [llength $COL_X]"

                    # Write FF & LUT loc constraints
                    if {[llength $COL_X] > 0} {
                        set SITE_LIST [list]

                        foreach y $COL_X {
                            set path_slice      [format "%s/inst_reg_chain_top/CR_Y\[%d\].CR_X\[%d\].reg_block_array_inst" $path_base $cr_y $cr_x]
                            set site            "${site_type}_X${x}Y${y}"
                            lappend SITE_LIST   "$site"

                            # A LOC constraint is for a primary site in the FPGA die, like a slice, a bram, or a dsp48, or a MMCM/PLL.
                            # a BEL (which stands for Basic ELement) is a subset of a site

                            for {set idx_fdre 0} {$idx_fdre < 16} {incr idx_fdre} {
                                # select the subset of SLICE
                                lappend slice_constraints [format "set_property BEL %4s \[get_cells \{%s/loop_slice_flop\[%d\].genblk_fdre\[%d\].FDRE_inst\}\]" [lindex $FF_BEL_DEF $idx_fdre] $path_slice $idx $idx_fdre]
                                # lappend slice_constraints [format "set_property BEL %s \[get_cells \{%s/loop_slice_flop\[%d\].genblk_fdre\[%d\].LUT_inst\}\]" [lindex $LUT_BEL_DEF $idx_fdre] $path_slice $idx $idx_fdre]
                            }
                            for {set idx_fdre 0} {$idx_fdre < 16} {incr idx_fdre} {
                                # select the SLICE
                                lappend slice_constraints [format "set_property LOC %4s \[get_cells \{%s/loop_slice_flop\[%d\].genblk_fdre\[%d\].FDRE_inst\}\]" $site $path_slice $idx $idx_fdre]
                                # lappend slice_constraints [format "set_property LOC %s \[get_cells \{%s/loop_slice_flop\[%d\].genblk_fdre\[%d\].LUT_inst\}\]" $site $path_slice $idx $idx_fdre]
                            }
                            incr idx; # Increment index for each site in the CR
                        }
                        lappend STAT_MSG "# Sites  : [join $SITE_LIST ", "]"
                    }
                    lappend STAT_MSG {#----------------------------------------------------}
                }
            }
        }
        dict set SLICE_CONSTRAINTS_LIST $site_key $idx_slr $slice_constraints
    }
    Dbg_Write_List $SLICE_CONSTRAINTS_LIST SLICE_CONSTRAINTS_LIST
    return $SLICE_CONSTRAINTS_LIST
}
#########################################################################################
# Generate SV Slice parameters
#########################################################################################
proc Generate_Slice_Sizes_Array_List { ACTUAL_SITE_LIST } {
    set SLICE_SIZES_ARRRAY_LIST {}
    set ACTUAL_SITE_LIST_TYPE   [dict get $ACTUAL_SITE_LIST SLICE]

    foreach {idx_slr ACTUAL_SITE_LIST_TYPE_SLR} $ACTUAL_SITE_LIST_TYPE {; # for all SLR
        dict set SLICE_SIZES_ARRRAY_LIST $idx_slr {}

        foreach {cr_y ACTUAL_SITE_LIST_TYPE_SLR_Y} $ACTUAL_SITE_LIST_TYPE_SLR {; # for CR on Y
            dict set SLICE_SIZES_ARRRAY_LIST $idx_slr $cr_y {}

            foreach {cr_x ACTUAL_SITE_LIST_TYPE_SLR_Y_X} $ACTUAL_SITE_LIST_TYPE_SLR_Y {; # for CR on Y
                set n_site 0

                foreach {x COL_X} $ACTUAL_SITE_LIST_TYPE_SLR_Y_X {
                    incr n_site [llength $COL_X]
                }
                dict set SLICE_SIZES_ARRRAY_LIST $idx_slr $cr_y $cr_x $n_site
            }
        }
    }
    return $SLICE_SIZES_ARRRAY_LIST
}
#########################################################################################
proc Generate_Pkg_Slice_Sizes_Array_List { SLICE_SIZES_ARRRAY_LIST } {
    set PKG_SLICE_SIZES_ARRRAY_LIST {}

    foreach {idx_slr SLICE_SIZES_ARRRAY_LIST_SLR} $SLICE_SIZES_ARRRAY_LIST {; # for all SLR
        set slice_sizes_array {}
        lappend slice_sizes_array  "\tlocalparam integer C_REG_BLOCK_SIZE_ARRAY \[0:19\]\[0:19\] = '\{ // Set to large size but only lower indexes used"

        foreach {cr_y SLICE_SIZES_ARRRAY_LIST_Y} $SLICE_SIZES_ARRRAY_LIST_SLR {; # for CR on Y
            lappend slice_sizes_array [format "\t\t%d:'\{" $cr_y]

            foreach {cr_x SLICE_SIZES_ARRRAY_LIST_Y_X} $SLICE_SIZES_ARRRAY_LIST_Y {; # for CR on Y
                set str_block   [format {X%dY%d} $cr_x $cr_y];  # "X3Y4"
                set str_param   [format {%d:%d,} $cr_x $SLICE_SIZES_ARRRAY_LIST_Y_X]
                lappend slice_sizes_array [format "\t\t\t%-10s // %s" $str_param $str_block]
            }
            lappend slice_sizes_array "\t\t\tdefault:0  //set unused indexes to 0"
            lappend slice_sizes_array "\t\t\},"
        }
        lappend slice_sizes_array "\t\tdefault:'\{20\{0\}\} //set unused indexes to 0"
        lappend slice_sizes_array "\t\};"
        dict set PKG_SLICE_SIZES_ARRRAY_LIST $idx_slr $slice_sizes_array
    }
    return $PKG_SLICE_SIZES_ARRRAY_LIST
}
#########################################################################################
# Write constraints
#########################################################################################
proc Write_Constraints {CONSTRAINTS_LIST} {
    variable GEN_CONST_OUTPUT; variable f_dbg

    if {$GEN_CONST_OUTPUT} {
        set f_sdxloc_name [file join output sdx_loc.tcl]
        set f_dbg [open $f_sdxloc_name a];
        set DATA {}
        lappend DATA {#// This is a generated file. Use and modify at your own risk.}
        lappend DATA {#////////////////////////////////////////////////////////////////////////////////}

        foreach {site_key CONSTRAINTS_LIST_SITE} $CONSTRAINTS_LIST {
            foreach {idx_slr CONSTRAINTS_LIST_SITE_SLR} $CONSTRAINTS_LIST_SITE {
                lappend DATA {#############################################################}
                lappend DATA "# Constraints for site ${site_key} in SLR${idx_slr}"
                lappend DATA {#############################################################}
                set DATA [concat $DATA $CONSTRAINTS_LIST_SITE_SLR]
            }
            log_message_line {GEN_POWER_FLOORPLAN-14} [list $site_key $f_sdxloc_name]; # File written
        }
        puts $f_dbg [join $DATA "\n"]; flush $f_dbg; Close_Dbg
    }
}
#########################################################################################
# Write SV package
#########################################################################################
proc Write_Pkg {SLR PKG_COL_SIZES_ARRRAY_LIST PKG_SLICE_SIZES_ARRRAY_LIST} {
    variable GEN_CONST_OUTPUT; variable f_dbg
    variable GEN_CONST_OUTPUT; variable f_dbg

    if {$GEN_CONST_OUTPUT} {
        foreach idx_slr $SLR {; # for all SLR
            set f_dbg_name [file join output powertest_param_slr${idx_slr}.sv]
            set f_dbg [open $f_dbg_name w]
            set DATA {}
            lappend DATA {////////////////////////////////////////////////////////////////////////////////}
            lappend DATA {// This is a generated file. Use and modify at your own risk.}
            lappend DATA {////////////////////////////////////////////////////////////////////////////////}
            lappend DATA {package powertest_param;}

            foreach {site_key PKG_COL_SIZES_ARRRAY_LIST_SITE} $PKG_COL_SIZES_ARRRAY_LIST {
                if {[dict exists $PKG_COL_SIZES_ARRRAY_LIST_SITE $idx_slr]} {
                    set DATA [concat $DATA [dict get $PKG_COL_SIZES_ARRRAY_LIST_SITE $idx_slr]]
                }
            }
            if {[dict exists $PKG_SLICE_SIZES_ARRRAY_LIST $idx_slr]} {
                set DATA [concat $DATA [dict get $PKG_SLICE_SIZES_ARRRAY_LIST $idx_slr]]
            }
            lappend DATA {endpackage}
            puts $f_dbg [join $DATA "\n"]; flush $f_dbg; Close_Dbg
            log_message_line {GEN_POWER_FLOORPLAN-15} [list $idx_slr $f_dbg_name]; # File written
        }
    }
}
#########################################################################################
# Main
#########################################################################################
proc gen_power_floorplan {SLR_LIST_IN {GEN_CONST_DEBUG_IN 0} {GEN_CONST_OUTPUT_IN 0}} {
    variable C_DYNAMIC_GEOMETRY; # dynamic_geometry.json
    variable C_INVALID; # invalid.json
    variable C_UTILIZATION; # utilization.json
    variable STAT_MSG
    variable GEN_CONST_DEBUG; variable GEN_CONST_OUTPUT

    set GEN_CONST_DEBUG  $GEN_CONST_DEBUG_IN
    set GEN_CONST_OUTPUT $GEN_CONST_OUTPUT_IN

    puts "gen_power_floorplan C_UTILIZATION: $C_UTILIZATION"
    puts "gen_power_floorplan C_INVALID: $C_INVALID"

    set SLR                 [dict get $C_DYNAMIC_GEOMETRY SLR]
    set CLOCK_REGIONS       [dict get $C_DYNAMIC_GEOMETRY CLOCK_REGIONS]
    set SITES_TYPES         [dict get $C_DYNAMIC_GEOMETRY SITES_TYPES]
    set PL_DYNAMIC_GEOMETRY [dict get $C_DYNAMIC_GEOMETRY PL_DYNAMIC_GEOMETRY]
    set PL_INVALID          [dict get $C_INVALID          PL_INVALID]
    set PL_UTILIZATION      [dict get $C_UTILIZATION      PL_UTILIZATION]

    # Use catch block to handle file resources deallocation
    set catch_ret [catch {
        Setup_Path
        log_message_line {GEN_POWER_FLOORPLAN-1} [list $SLR_LIST_IN]; # Start

        foreach idx_slr $SLR_LIST_IN {; # Check current SLRs are valid
            if {[lsearch -exact $SLR $idx_slr] == -1} {
                log_message_line {GEN_POWER_FLOORPLAN-2} [list $idx_slr [join $SLR ", "]]; # ERROR SLR not found
            }
        }
        log_message_line {GEN_POWER_FLOORPLAN-3} {{PL_INVALID}}; # Start param check
        set PL_INVALID [Check_PL_Invalid $SITES_TYPES $SLR $PL_DYNAMIC_GEOMETRY $PL_INVALID]

        log_message_line {GEN_POWER_FLOORPLAN-3} {{PL_UTILIZATION}}; # Start param check
        set PL_UTILIZATION [Check_PL_Utilization $SITES_TYPES $SLR $CLOCK_REGIONS $PL_DYNAMIC_GEOMETRY $PL_UTILIZATION]

        log_message_line {GEN_POWER_FLOORPLAN-4}; # Start actual generation
        set ACTUAL_GEOM_LIST [Generate_Actual_Geom_List $SITES_TYPES $SLR_LIST_IN $PL_DYNAMIC_GEOMETRY $PL_UTILIZATION $PL_INVALID]

        log_message_line {GEN_POWER_FLOORPLAN-5}; # Start physical constraints
        set COL_CONSTRAINTS_LIST    [Generate_Col_Constraints           $SITES_TYPES $ACTUAL_GEOM_LIST]
        set SLICE_CONSTRAINTS_LIST  [Generate_Slice_Constraints_List    $SITES_TYPES $ACTUAL_GEOM_LIST]

        log_message_line {GEN_POWER_FLOORPLAN-6}; # Start SV parameters
        set COL_SIZES_ARRRAY_LIST   [Generate_Col_Sizes_Array_List      $SITES_TYPES $ACTUAL_GEOM_LIST]
        set SLICE_SIZES_ARRRAY_LIST [Generate_Slice_Sizes_Array_List    $ACTUAL_GEOM_LIST]

        set PKG_COL_SIZES_ARRRAY_LIST   [Generate_Pkg_Col_Sizes_Array_List      $COL_SIZES_ARRRAY_LIST]
        set PKG_SLICE_SIZES_ARRRAY_LIST [Generate_Pkg_Slice_Sizes_Array_List    $SLICE_SIZES_ARRRAY_LIST]

        Write_Constraints   $COL_CONSTRAINTS_LIST
        Write_Constraints   $SLICE_CONSTRAINTS_LIST
        Write_Pkg           $SLR_LIST_IN $PKG_COL_SIZES_ARRRAY_LIST $PKG_SLICE_SIZES_ARRRAY_LIST

        log_message_file [join $STAT_MSG "\n"]

    } catch_result_txt]

    if {$catch_ret == 0} {
        log_message_line {GEN_POWER_FLOORPLAN-7}; # End success
        Close_Files
        return [list $PKG_COL_SIZES_ARRRAY_LIST $PKG_SLICE_SIZES_ARRRAY_LIST $COL_CONSTRAINTS_LIST $SLICE_CONSTRAINTS_LIST]
    } else {
        set error_info $::errorInfo
        set error_code $::errorCode
        [catch {log_message_line {GEN_POWER_FLOORPLAN-8}} res_txt]; # End ERROR
        Close_Files
        return -code $catch_ret -errorcode $error_code -errorinfo $error_info $catch_result_txt
    }
}
