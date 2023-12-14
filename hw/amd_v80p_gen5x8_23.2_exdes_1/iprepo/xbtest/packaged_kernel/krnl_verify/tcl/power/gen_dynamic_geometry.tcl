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

set_msg_config -id {GEN_DYNAMIC_GEOMETRY 7} -limit 1500
set_msg_config -id {GEN_DYNAMIC_GEOMETRY 12} -limit 1500

#########################################################################################
# Get SLR: SLR={idx_slr}
#########################################################################################
proc get_SLR {} {
    set SLR {}
    set idx_slr 0
    set end_fpga false
    while {$end_fpga == false} {
        set slr [get_slrs -quiet -filter SLR_INDEX==$idx_slr]
        if {$slr != ""} {
            lappend SLR $idx_slr
            incr idx_slr
        } else {
            set end_fpga true
        }
    }
    return $SLR
}
#########################################################################################
# Create temporary pblock
#########################################################################################
proc create_tmp_pblocks { SITES_TYPES aie_type } {
    delete_pblock -quiet [get_pblocks -quiet device]
    set tmp_pblock [create_pblock device]
    foreach {site_key site_type} $SITES_TYPES {
        resize_pblock $tmp_pblock -add [get_sites $site_type*]
    }
    if {$aie_type != {}} {
        resize_pblock $tmp_pblock -add [get_sites $aie_type*]
    }
    return [list $tmp_pblock]
}

#proc create_tmp_pblocks { SITES_TYPES } {
#    set tmp_pblocks {}
#    foreach idx_slr $SLR {
#        set tmp_pblock [get_pblocks -quiet SLR$idx_slr]
#        if {$tmp_pblock != {}} {
#            remove_tmp_pblocks $tmp_pblock
#        }
#        set tmp_pblock [create_pblock SLR$idx_slr]
#        resize_pblock $tmp_pblock -add SLR$idx_slr
#        lappend tmp_pblocks $tmp_pblock
#    }
#    return $tmp_pblocks
#}
#########################################################################################
# Create temporary pblock
#########################################################################################
proc remove_tmp_pblocks { tmp_pblocks } {
    foreach tmp_pblock $tmp_pblocks {
        delete_pblock [get_pblocks $tmp_pblock]
    }
}
#########################################################################################
# Get CLOCK_REGIONS: CLOCK_REGIONS[idx_slr][cr_y]={cr_x}
#########################################################################################
proc get_CLOCK_REGIONS { SLR } {
    set CLOCK_REGIONS {}
    set cr_y 0
    foreach idx_slr $SLR {
        set end_slr false
        while {!$end_slr} {
            set cr_x_list [get_clock_regions -quiet -filter [subst {ROW_INDEX==$cr_y}] -of_objects [get_slrs -filter [subst {SLR_INDEX==$idx_slr}]]]
            if {[llength $cr_x_list] > 0} {
                dict set CLOCK_REGIONS $idx_slr $cr_y {}
                foreach cr $cr_x_list {
                    set cr_split_y [split $cr                    {Y}]
                    set cr_split_x [split [lindex $cr_split_y 0] {X}]
                    scan [lindex $cr_split_x 1] %d cr_x
                    dict with CLOCK_REGIONS $idx_slr {
                        lappend $cr_y $cr_x
                    }
                }
                incr cr_y
            } else {
                set end_slr true
            }
        }
    }
    return $CLOCK_REGIONS
}

#########################################################################################
# Get sites in each CR and order sites by column:
# CONFIG_SITES[idx_slr][cr_y][cr_x][site_x]={site_y}
#########################################################################################
proc get_CONFIG_SITES { CLOCK_REGIONS dynamic_pblocks} {
    set CONFIG_SITES {}

    set site_type CONFIG_SITE
    set found false

    foreach ulp_pblock $dynamic_pblocks {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-6} {INFO} "Get sites ($site_type) in ${ulp_pblock} for all CRs in all SLRs"
        set sites_pblock [get_sites -quiet -filter [subst {NAME =~ ${site_type}_X* && !PROHIBIT}] -of [get_pblocks $ulp_pblock]]

        foreach {idx_slr CLOCK_REGIONS_SLR} $CLOCK_REGIONS {
            foreach {cr_y CLOCK_REGIONS_SLR_Y} $CLOCK_REGIONS_SLR {
                foreach cr_x $CLOCK_REGIONS_SLR_Y {
                    set sites_cr [get_sites -quiet -filter [subst {NAME =~ ${site_type}_X* && !PROHIBIT}] -of_objects [get_clock_regions -filter [subst {ROW_INDEX==$cr_y && COLUMN_INDEX==$cr_x}]]]
                    if {[llength $sites_cr] > 0} {
                        # First create a dictionary to store sites by column (based on their x location)
                        foreach site $sites_cr {
                            if { [lsearch -exact $sites_pblock $site] != -1 } {
                                # Save columns
                                if {![dict exists $CONFIG_SITES $idx_slr]} {
                                    dict set CONFIG_SITES $idx_slr {}
                                }
                                if {![dict exists $CONFIG_SITES $idx_slr $cr_y]} {
                                    dict set CONFIG_SITES $idx_slr $cr_y {}
                                }
                                if {![dict exists $CONFIG_SITES $idx_slr $cr_y $cr_x]} {
                                    dict set CONFIG_SITES $idx_slr $cr_y $cr_x {}
                                }

                                scan $site "${site_type}_X%dY%d" x y

                                if {[dict exists $CONFIG_SITES $idx_slr $cr_y $cr_x $x]} {
                                    dict with CONFIG_SITES $idx_slr $cr_y $cr_x {
                                        lappend $x $y
                                    }
                                } else {
                                    dict set CONFIG_SITES $idx_slr $cr_y $cr_x $x [list $y]
                                }
                                set found true
                            }
                        }
                    }
                }
            }
        }
    }
    if {$found} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-8} {INFO} "Found one or more sites ($site_type) in ULP pblock(s)"
    }
    return $CONFIG_SITES
}

#########################################################################################
# Get sites in each CR and order sites by column:
# PL_DYNAMIC_GEOMETRY[site_key][idx_slr][cr_y][cr_x][site_x]={site_y}
#########################################################################################
proc get_PL_DYNAMIC_GEOMETRY { SITES_TYPES CLOCK_REGIONS dynamic_pblocks} {
    set PL_DYNAMIC_GEOMETRY {}
    set N_SITE              {}

    foreach {site_key site_type} $SITES_TYPES {
        foreach ulp_pblock $dynamic_pblocks {
            common::send_msg_id {GEN_DYNAMIC_GEOMETRY-6} {INFO} "Get $site_key sites ($site_type) in ${ulp_pblock} for all CRs in all SLRs"
            set sites_pblock [get_sites -quiet -filter [subst {NAME =~ ${site_type}_X* && !PROHIBIT}] -of [get_pblocks $ulp_pblock]]

            foreach {idx_slr CLOCK_REGIONS_SLR} $CLOCK_REGIONS {
                set n_sites_slr 0
                foreach {cr_y CLOCK_REGIONS_SLR_Y} $CLOCK_REGIONS_SLR {
                    set n_sites_cr_y 0
                    foreach cr_x $CLOCK_REGIONS_SLR_Y {
                        set n_sites_cr_x 0
                        set sites_cr [get_sites -quiet -filter [subst {NAME =~ ${site_type}_X* && !PROHIBIT}] -of_objects [get_clock_regions -filter [subst {ROW_INDEX==$cr_y && COLUMN_INDEX==$cr_x}]]]
                        if {[llength $sites_cr] > 0} {
                            # First create a dictionary to store sites by column (based on their x location)
                            foreach site $sites_cr {
                                if { [lsearch -exact $sites_pblock $site] != -1 } {
                                    # Save columns
                                    if {![dict exists $PL_DYNAMIC_GEOMETRY $site_key]} {
                                        dict set PL_DYNAMIC_GEOMETRY $site_key {}
                                    }
                                    if {![dict exists $PL_DYNAMIC_GEOMETRY $site_key $idx_slr]} {
                                        dict set PL_DYNAMIC_GEOMETRY $site_key $idx_slr {}
                                    }
                                    if {![dict exists $PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y]} {
                                        dict set PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y {}
                                    }
                                    if {![dict exists $PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x]} {
                                        dict set PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x {}
                                    }

                                    scan $site "${site_type}_X%dY%d" x y

                                    if {[dict exists $PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x $x]} {
                                        dict with PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x {
                                            lappend $x $y
                                        }
                                    } else {
                                        dict set PL_DYNAMIC_GEOMETRY $site_key $idx_slr $cr_y $cr_x $x [list $y]
                                    }
                                    incr n_sites_cr_x
                                }
                            }
                        }
                        incr n_sites_cr_y $n_sites_cr_x
                        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-7} {INFO} "\t- Total number of $site_key sites ($site_type) found in $ulp_pblock for CR X${cr_x}Y${cr_y}: $n_sites_cr_x"
                    }
                    incr n_sites_slr $n_sites_cr_y
                }
                # Accumulate number of sites
                if {![dict exists $N_SITE $site_key]} {
                    dict set N_SITE $site_key {}
                }
                if {![dict exists $N_SITE $site_key $idx_slr]} {
                    dict set N_SITE $site_key $idx_slr 0
                }
                dict set N_SITE $site_key $idx_slr [expr [dict get $N_SITE $site_key $idx_slr] + $n_sites_cr_x]
                common::send_msg_id {GEN_DYNAMIC_GEOMETRY-8} {INFO} "Total number of $site_key sites ($site_type) found in $ulp_pblock for SLR$idx_slr: $n_sites_slr"
            }
        }
    }
    return $PL_DYNAMIC_GEOMETRY
}

#########################################################################################
# Get sites in each CR and order sites by column:
# AIE_GEOMETRY[idx_slr][site_x]={site_y}
#########################################################################################
proc get_AIE_GEOMETRY { site_key site_type CLOCK_REGIONS dynamic_pblocks} {
    set AIE_GEOMETRY    {}
    set N_SITE          {}

    foreach ulp_pblock $dynamic_pblocks {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-11} {INFO} "Get $site_key sites ($site_type) in ${ulp_pblock} for all CRs in all SLRs"
        set sites_pblock [get_sites -quiet -filter [subst {NAME =~ ${site_type}_X* && !PROHIBIT}] -of [get_pblocks $ulp_pblock]]

        foreach {idx_slr CLOCK_REGIONS_SLR} $CLOCK_REGIONS {
            set n_sites_slr 0
            foreach {cr_y CLOCK_REGIONS_SLR_Y} $CLOCK_REGIONS_SLR {
                set n_sites_cr_y 0
                foreach cr_x $CLOCK_REGIONS_SLR_Y {
                    set n_sites_cr_x 0
                    set sites_cr [get_sites -quiet -filter [subst {NAME =~ ${site_type}_X* && !PROHIBIT}] -of_objects [get_clock_regions -filter [subst {ROW_INDEX==$cr_y && COLUMN_INDEX==$cr_x}]]]
                    if {[llength $sites_cr] > 0} {
                        # First create a dictionary to store sites by column (based on their x location)
                        foreach site $sites_cr {
                            if { [lsearch -exact $sites_pblock $site] != -1 } {
                                # Save columns
                                if {![dict exists $AIE_GEOMETRY $idx_slr]} {
                                    dict set AIE_GEOMETRY $idx_slr {}
                                }

                                scan $site "${site_type}_X%dY%d" x y

                                if {[dict exists $AIE_GEOMETRY $idx_slr $y]} {
                                    dict with AIE_GEOMETRY $idx_slr {
                                        lappend $y $x
                                    }
                                } else {
                                    dict set AIE_GEOMETRY $idx_slr $y [list $x]
                                }
                                incr n_sites_cr_x
                            }
                        }
                    }
                    incr n_sites_cr_y $n_sites_cr_x
                    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-12} {INFO} "\t- Total number of $site_key sites ($site_type) found in $ulp_pblock for CR X${cr_x}Y${cr_y}: $n_sites_cr_x"
                }
                incr n_sites_slr $n_sites_cr_y
            }
            # Accumulate number of sites
            if {![dict exists $N_SITE $idx_slr]} {
                dict set N_SITE $idx_slr 0
            }
            dict set N_SITE $idx_slr [expr [dict get $N_SITE $idx_slr] + $n_sites_cr_x]
            common::send_msg_id {GEN_DYNAMIC_GEOMETRY-13} {INFO} "Total number of $site_key sites ($site_type) found in $ulp_pblock for SLR$idx_slr: $n_sites_slr"
        }
    }
    return $AIE_GEOMETRY
}

proc string2json {str} {
    return "\"$str\""
}
proc list2json {listVal} {
    return "\[[join $listVal ,]\]"
}
proc strlist2json {strListVal} {
    set tmp {}
    foreach val $strListVal {
        lappend tmp [string2json $val]
    }
    return [list2json $tmp]
}
proc d4list2json {listVal} {
    set tmp {}
    foreach val $listVal {
        lappend tmp [format {%4s} $val]
    }

    return "\[[join $tmp ,]\]"
}

#########################################################################################
# Main function. Open DCP in Vivado, source this script and run the gen_dynamic_geometry command
# The dynamic_geometry.json file, required to build xbtest power CU, and a template of utilization.json and invalid.json will be generated.
#########################################################################################
proc gen_dynamic_geometry { pwr_floorplan_dir dynamic_pblocks_in } {

    set SITES_TYPE_SLICE    SLICE
    set SITES_TYPE_DSP      DSP
    set SITES_TYPE_DSP48E2  DSP48E2
    set SITES_TYPE_BRAM     RAMB36
    set SITES_TYPE_URAM     URAM288
    set SITES_TYPE_AIE      AIE_CORE
    set SITES_TYPE_AIE_ML   AIE_ML_CORE

    set SITE_KEY_SLICE      SLICE
    set SITE_KEY_DSP        DSP
    set SITE_KEY_BRAM       BRAM
    set SITE_KEY_URAM       URAM
    set SITE_KEY_AIE        AIE

    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"

    # Check inputs
    if {![file exists $pwr_floorplan_dir]} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-1} {ERROR} "Directory provided does not exists: $pwr_floorplan_dir"
    }
    if {![file isdirectory $pwr_floorplan_dir]} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-1} {ERROR} "Directory provided is not a directory: $pwr_floorplan_dir"
    }
    foreach ulp_pblock $dynamic_pblocks_in {
        if { [llength [lsearch -all $dynamic_pblocks_in $ulp_pblock]] > 1 } {
            common::send_msg_id {GEN_DYNAMIC_GEOMETRY-2} {ERROR} "Pblock name ($ulp_pblock) provided more than once in input pblock list"
        }
        if {[get_pblocks -quiet $ulp_pblock] == {}} {
            common::send_msg_id {GEN_DYNAMIC_GEOMETRY-3} {ERROR} "Pblock name ($ulp_pblock) provided not found in design. Available pblocks: [join [get_pblocks] {, }]"
        }
    }

    ##################################################################################################################################################

    set dynamic_geometry_json [file join $pwr_floorplan_dir dynamic_geometry.json]
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-5} {INFO} "Starting generation of dynamic_geometry.json file: $dynamic_geometry_json"
    if {[file exists $dynamic_geometry_json]} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-4} {WARNING} "File dynamic_geometry.json already exists and will be overwritten: $dynamic_geometry_json"
    }

    # Determine list of SLRs and Clock Regions
    set SLR                 [get_SLR]
    set CLOCK_REGIONS       [get_CLOCK_REGIONS $SLR]

    # Determine supported type of DSP sites
    set dsp48e2_found 0
    if {[llength [get_sites -quiet -filter [subst {NAME =~ ${SITES_TYPE_DSP48E2}_X* && !PROHIBIT}] ]] > 0} {
        set dsp48e2_found 1
    }

    # Determine if AIE is supported
    set aie_type {}
    if {[llength [get_sites -quiet -filter [subst {NAME =~ ${SITES_TYPE_AIE}_X* && !PROHIBIT}] ]] > 0} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-10} {INFO} {Platform supports AIE}
        set aie_type $SITES_TYPE_AIE
    } elseif {[llength [get_sites -quiet -filter [subst {NAME =~ ${SITES_TYPE_AIE_ML}_X* && !PROHIBIT}] ]] > 0} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-10} {INFO} {Platform supports AIE-ML}
        set aie_type $SITES_TYPE_AIE_ML
    }

    # Determine supported site types
    set SITES_TYPES {}
    dict set SITES_TYPES $SITE_KEY_SLICE $SITES_TYPE_SLICE
    if {$dsp48e2_found} {
        dict set SITES_TYPES $SITE_KEY_DSP $SITES_TYPE_DSP48E2
    } else {
        dict set SITES_TYPES $SITE_KEY_DSP $SITES_TYPE_DSP
    }
    dict set SITES_TYPES $SITE_KEY_BRAM  $SITES_TYPE_BRAM
    dict set SITES_TYPES $SITE_KEY_URAM  $SITES_TYPE_URAM

    if {$dynamic_pblocks_in == {}} {
        set dynamic_pblocks [create_tmp_pblocks $SITES_TYPES $aie_type]
    } else {
        set dynamic_pblocks $dynamic_pblocks_in
    }

    set CONFIG_SITES        [get_CONFIG_SITES        $CLOCK_REGIONS $dynamic_pblocks]
    set PL_DYNAMIC_GEOMETRY [get_PL_DYNAMIC_GEOMETRY $SITES_TYPES $CLOCK_REGIONS $dynamic_pblocks]
    if {$aie_type != {}} {
        set AIE_GEOMETRY [get_AIE_GEOMETRY $SITE_KEY_AIE $aie_type $CLOCK_REGIONS $dynamic_pblocks]
    }

    # Generate dynamic_geometry in JSON format

    set     DYNAMIC_GEOMETRY_JSON {}
    lappend DYNAMIC_GEOMETRY_JSON "\{"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json {#########################################################################################}],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json {xbtest dynamic region geometry}],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json {This is a generated file. Use and modify at your own risk.}],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json {#########################################################################################}],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {PART}]    : [string2json [get_property part [current_design]]],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {PBLOCKS}] : [strlist2json $dynamic_pblocks_in],"
    # Write SLR
    set slr_format {SLR definition (SLR): [ <SLR idx> ]}
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $slr_format],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {SLR}] : [list2json $SLR],"
    # Write CLOCK_REGIONS
    set clock_regions_format {}
    lappend clock_regions_format "Clock Regions definition (CLOCK_REGIONS):"
    lappend clock_regions_format "<SLR index> : \{"
    lappend clock_regions_format    "$TAB_1 <CR Y> : \[ <CR X> \]"
    lappend clock_regions_format "\}"
    foreach line $clock_regions_format {
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $line],"
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {CLOCK_REGIONS}] : \{"
    set ii_last [expr [llength [dict keys $CLOCK_REGIONS]] - 1]
    set ii      0
    foreach {idx_slr CLOCK_REGIONS_SLR} $CLOCK_REGIONS {
    lappend DYNAMIC_GEOMETRY_JSON       "$TAB_2[string2json $idx_slr] : \{"
        set jj_last [expr [llength [dict keys $CLOCK_REGIONS_SLR]] - 1]
        set jj      0
        foreach {cr_y CLOCK_REGIONS_SLR_Y} $CLOCK_REGIONS_SLR {
            if {$jj < $jj_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON           "$TAB_3[string2json $cr_y] : [list2json $CLOCK_REGIONS_SLR_Y]$next"
            incr jj
        }
        if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON       "$TAB_2\}$next"
        incr ii
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1\},"

    # Write SITES_TYPES
    set sites_types_format {Sites types supported in PL (SITES_TYPES): <Site key> : <Site type>}
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $sites_types_format],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {SITES_TYPES}] : \{"
    set ii_last [expr [llength [dict keys $SITES_TYPES]] - 1]
    set ii      0
    foreach {site_key site_type} $SITES_TYPES {
        if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON       "$TAB_2[string2json $site_key] : [string2json $site_type]$next"
        incr ii
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1\},"


    # Write AIE_GEOMETRY
    if {$aie_type != {}} {
        # Write AIE_TYPE
        set aie_type_format {Supported type of AIE sites (AIE_TYPE): <AIE type>}
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $aie_type_format],"
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {AIE_TYPE}] : [string2json $aie_type],"

        set aie_geometry_format {}
        lappend aie_geometry_format "AIE sites definition (AIE_GEOMETRY):"
        lappend aie_geometry_format "<SLR index> : \{"
        lappend aie_geometry_format    "$TAB_1 <Site Y> : \[ <Site X> \]"
        lappend aie_geometry_format "\}"
        foreach line $aie_geometry_format {
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $line],"
        }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {AIE_GEOMETRY}] : \{"
        set ii_last [expr [llength [dict keys $AIE_GEOMETRY]] - 1]
        set ii      0
        foreach {idx_slr AIE_GEOMETRY_SLR} $AIE_GEOMETRY {
    lappend DYNAMIC_GEOMETRY_JSON       "$TAB_2[string2json $idx_slr] : \{"
            set jj_last [expr [llength [dict keys $AIE_GEOMETRY_SLR]] - 1]
            set jj      0
            foreach {y AIE_GEOMETRY_SLR_Y} $AIE_GEOMETRY_SLR {
                if {$jj < $jj_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON           "$TAB_3[string2json $y] : [list2json $AIE_GEOMETRY_SLR_Y]$next"
                incr jj
            }
            if {$ii < $ii_last} { set next "," } else { set next "" }
            lappend DYNAMIC_GEOMETRY_JSON "$TAB_2\}$next"
            incr ii
        }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1\},"
    }

    # Write CONFIG_SITES
    set config_sites_format {}
    lappend config_sites_format "CONFIG_SITE sites definition (CONFIG_SITES):"
    lappend config_sites_format "<SLR index> : \{"
    lappend config_sites_format "$TAB_1 <CR Y> : \{"
    lappend config_sites_format     "$TAB_2 <CR X> : \{"
    lappend config_sites_format         "$TAB_3 <Site X> : \[ <Site Y> \]"
    lappend config_sites_format     "$TAB_2 \}"
    lappend config_sites_format "$TAB_1 \}"
    lappend config_sites_format "\}"
    foreach line $config_sites_format {
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $line],"
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {CONFIG_SITES}] : \{"
    set ii_last [expr [llength [dict keys $CONFIG_SITES]] - 1]
    set ii      0
    foreach {idx_slr CONFIG_SITES_SLR} $CONFIG_SITES {
    lappend DYNAMIC_GEOMETRY_JSON           "$TAB_2[string2json $idx_slr] : \{"
        set jj_last [expr [llength [dict keys $CONFIG_SITES_SLR]] - 1]
        set jj      0
        foreach {cr_y CONFIG_SITES_SLR_Y} $CONFIG_SITES_SLR {
    lappend DYNAMIC_GEOMETRY_JSON               "$TAB_3[string2json $cr_y] : \{"
            set kk_last [expr [llength [dict keys $CONFIG_SITES_SLR_Y]] - 1]
            set kk      0
            foreach {cr_x CONFIG_SITES_SLR_Y_X} $CONFIG_SITES_SLR_Y {
    lappend DYNAMIC_GEOMETRY_JSON                   "$TAB_4[string2json $cr_x] : \{"
                set ll_last [expr [llength [dict keys $CONFIG_SITES_SLR_Y_X]] - 1]
                set ll      0
                foreach {x COL_X} $CONFIG_SITES_SLR_Y_X {
                    if {$ll < $ll_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON                       "$TAB_5[string2json $x] : [list2json $COL_X]$next"
                    incr ll
                }
                if {$kk < $kk_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON                   "$TAB_4\}$next"
                incr kk
            }
            if {$jj < $jj_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON               "$TAB_3\}$next"
            incr jj
        }
        if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON           "$TAB_2\}$next"
        incr ii
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1\},"
    # Write PL_DYNAMIC_GEOMETRY
    set pl_dynamic_geometry_format {}
    lappend pl_dynamic_geometry_format "PL sites definition (PL_DYNAMIC_GEOMETRY):"
    lappend pl_dynamic_geometry_format "<Site key> : \{"
    lappend pl_dynamic_geometry_format "$TAB_1 <SLR index>  : \{"
    lappend pl_dynamic_geometry_format      "$TAB_2 <CR Y> : \{"
    lappend pl_dynamic_geometry_format          "$TAB_3 <CR X> : \{"
    lappend pl_dynamic_geometry_format              "$TAB_4 <Site X> : \[ <Site Y> \]"
    lappend pl_dynamic_geometry_format          "$TAB_3 \}"
    lappend pl_dynamic_geometry_format      "$TAB_2 \}"
    lappend pl_dynamic_geometry_format "$TAB_1 \}"
    lappend pl_dynamic_geometry_format "\}"
    foreach line $pl_dynamic_geometry_format {
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {comment}] : [string2json $line],"
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1[string2json {PL_DYNAMIC_GEOMETRY}] : \{"
    set ii_last [expr [llength [dict keys $PL_DYNAMIC_GEOMETRY]] - 1]
    set ii      0
    foreach {site_key DYNAMIC_GEOMETRY_SITE} $PL_DYNAMIC_GEOMETRY {
    lappend DYNAMIC_GEOMETRY_JSON       "$TAB_2[string2json $site_key] : \{"
        set jj_last [expr [llength [dict keys $DYNAMIC_GEOMETRY_SITE]] - 1]
        set jj      0
        foreach {idx_slr DYNAMIC_GEOMETRY_SITE_SLR} $DYNAMIC_GEOMETRY_SITE {
    lappend DYNAMIC_GEOMETRY_JSON           "$TAB_3[string2json $idx_slr] : \{"
            set kk_last [expr [llength [dict keys $DYNAMIC_GEOMETRY_SITE_SLR]] - 1]
            set kk      0
            foreach {cr_y DYNAMIC_GEOMETRY_SITE_SLR_Y} $DYNAMIC_GEOMETRY_SITE_SLR {
    lappend DYNAMIC_GEOMETRY_JSON               "$TAB_4[string2json $cr_y] : \{"
                set ll_last [expr [llength [dict keys $DYNAMIC_GEOMETRY_SITE_SLR_Y]] - 1]
                set ll      0
                foreach {cr_x DYNAMIC_GEOMETRY_SITE_SLR_Y_X} $DYNAMIC_GEOMETRY_SITE_SLR_Y {
    lappend DYNAMIC_GEOMETRY_JSON                   "$TAB_5[string2json $cr_x] : \{"
                    set mm_last [expr [llength [dict keys $DYNAMIC_GEOMETRY_SITE_SLR_Y_X]] - 1]
                    set mm      0
                    foreach {x COL_X} $DYNAMIC_GEOMETRY_SITE_SLR_Y_X {
                        if {$mm < $mm_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON                       "$TAB_6[string2json $x] : [list2json $COL_X]$next"
                        incr mm
                    }
                    if {$ll < $ll_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON                   "$TAB_5\}$next"
                    incr ll
                }
                if {$kk < $kk_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON               "$TAB_4\}$next"
                incr kk
            }
            if {$jj < $jj_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON           "$TAB_3\}$next"
            incr jj
        }
        if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend DYNAMIC_GEOMETRY_JSON       "$TAB_2\}$next"
        incr ii
    }
    lappend DYNAMIC_GEOMETRY_JSON "$TAB_1\}"
    lappend DYNAMIC_GEOMETRY_JSON "\}"
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-9} {INFO} "Successfully generated dynamic_geometry.json file: $dynamic_geometry_json"
    set fp [open $dynamic_geometry_json w]; puts $fp [join $DYNAMIC_GEOMETRY_JSON "\n"]; close $fp

    ##################################################################################################################################################

    set utilization_template_json [file join $pwr_floorplan_dir utilization_template.json]
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-5} {INFO} "Starting generation of utilization_template.json file: $utilization_template_json"
    if {[file exists $utilization_template_json]} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-4} {WARNING} "File utilization_template.json already exists and will be overwritten: $utilization_template_json"
    }

    set     UTILIZATION_TEMPLATE_JSON {}
    lappend UTILIZATION_TEMPLATE_JSON "\{"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {#########################################################################################}],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {xbtest sites utilization definition}],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {This is a generated template file}],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {How to use:}],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {    1. Rename to utilization.json}],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {    2. Modify according to your requirements}],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {#########################################################################################}],"

    # Write AIE_UTILIZATION
    if {$aie_type != {}} {
    set aie_utilization_format {Utilization of AIE sites (AIE_UTILIZATION) is defined per SLR in percentage of sites in dynamic geometry: SLR_<SLR idx> : <AIE utilization>}
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json $aie_utilization_format],"
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {AIE_UTILIZATION}] : \{"
        set ii_last [expr [llength [dict keys $AIE_GEOMETRY]] - 1]
        set ii      0
        foreach {idx_slr AIE_GEOMETRY_SLR} $AIE_GEOMETRY {
            if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend UTILIZATION_TEMPLATE_JSON        "$TAB_2[string2json SLR_$idx_slr] : 0$next"
            incr ii
        }
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1\},"
    }
    # Write PL_UTILIZATION
    set pl_utilization_format {}
    lappend pl_utilization_format "Utilization of PL sites (PL_UTILIZATION) is defined, per clock region (CR) and per site type, in percentage \[0;100\] of (number of valid location in dynamic geometry - number of invalid locations):"
    lappend pl_utilization_format "SLR_<SLR idx> : \{"
    lappend pl_utilization_format   "$TAB_1 CR_Y_<CR_Y idx> : \{"
    lappend pl_utilization_format       "$TAB_2 [format {%-7s} CR_X] : \[ <list of CR_X idx>\ ]"
    foreach site_key [dict keys $SITES_TYPES] {
    lappend pl_utilization_format       "$TAB_2 [format {%-7s} $site_key] : \[ <list of $site_key utilizations for each CR_X idx> \]"
    }
    lappend pl_utilization_format   "$TAB_1 \}"
    lappend pl_utilization_format "\}"
    foreach line $pl_utilization_format {
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json $line],"
    }
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1[string2json {PL_UTILIZATION}] : \{"
    set ii_last [expr [llength [dict keys $CLOCK_REGIONS]] - 1]
    set ii      0
    foreach idx_slr [lsort -decreasing -dictionary [dict keys $CLOCK_REGIONS]] {
        set CLOCK_REGIONS_SLR [dict get $CLOCK_REGIONS $idx_slr]
    lappend UTILIZATION_TEMPLATE_JSON        "$TAB_2[string2json SLR_$idx_slr] : \{"
        set jj_last [expr [llength [dict keys $CLOCK_REGIONS_SLR]] - 1]
        set jj      0
        foreach cr_y [lsort -decreasing -dictionary [dict keys $CLOCK_REGIONS_SLR]] {
    lappend UTILIZATION_TEMPLATE_JSON            "$TAB_3[string2json CR_Y_$cr_y] : \{"
            set CR_X    {}
            set UTIL_0  {}
            foreach cr_x [lsort -dictionary [dict get $CLOCK_REGIONS_SLR $cr_y]] {
                lappend CR_X    $cr_x
                lappend UTIL_0  0
            }
    lappend UTILIZATION_TEMPLATE_JSON                "$TAB_4[format {%-7s} [string2json {CR_X}]] : [d4list2json $CR_X],"
        set kk_last [expr [llength [dict keys $SITES_TYPES]] - 1]
        set kk      0
            foreach site_key [dict keys $SITES_TYPES] {
                if {$kk < $kk_last} { set next "," } else { set next "" }
    lappend UTILIZATION_TEMPLATE_JSON                "$TAB_4[format {%-7s} [string2json $site_key]] : [d4list2json $UTIL_0]$next"
                incr kk
            }
            if {$jj < $jj_last} { set next "," } else { set next "" }
    lappend UTILIZATION_TEMPLATE_JSON            "$TAB_3\}$next"
            incr jj
        }
        if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend UTILIZATION_TEMPLATE_JSON        "$TAB_2\}$next"
        incr ii
    }
    lappend UTILIZATION_TEMPLATE_JSON "$TAB_1\}"
    lappend UTILIZATION_TEMPLATE_JSON "\}"

    set fp [open $utilization_template_json w]; puts $fp [join $UTILIZATION_TEMPLATE_JSON "\n"]; close $fp
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-9} {INFO} "Successfully generated utilization_template.json file: $utilization_template_json"

    ##################################################################################################################################################

    set invalid_template_json [file join $pwr_floorplan_dir invalid_template.json]
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-5} {INFO} "Starting generation of invalid_template.json file: $invalid_template_json"
    if {[file exists $invalid_template_json]} {
        common::send_msg_id {GEN_DYNAMIC_GEOMETRY-4} {WARNING} "File invalid_template.json already exists and will be overwritten: $invalid_template_json"
    }

    set     INVALID_TEMPLATE_JSON {}
    lappend INVALID_TEMPLATE_JSON "\{"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {#########################################################################################}],"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {xbtest optional invalid sites definition}],"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {This is a generated template file}],"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {How to use:}],"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {    1. Rename to invalid.json}],"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {    2. Modify according to your requirements}],"
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json {#########################################################################################}],"

    # Write PL_INVALID
    set pl_invalid_format {}
    lappend pl_invalid_format "Locations of sites not used in power CU floorplan (PL_INVALID) are defined per site type and per SLR."
    lappend pl_invalid_format "Provide a single site (<x0>,<y0>) or a site rectangle defined by bottom left (<x1>,<y1>) and top right (<x2>,<y2>) corners:"
    lappend pl_invalid_format "See dynamic_geometry for supported <site type>"
    lappend pl_invalid_format "SLR_<SLR idx> : \["
    lappend pl_invalid_format "  \{ location: <site type>_X<x0>Y<y0> \}"
    lappend pl_invalid_format "  \{ location: <site type>_X<x1>Y<y1>:<site type>_X<x2>Y<y2> \}"
    lappend pl_invalid_format "\]"
    lappend pl_invalid_format "Here is an example of one single site and one rectangle per site type per available SLR"
    foreach line $pl_invalid_format {
    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {comment}] : [string2json $line],"
    }

    lappend INVALID_TEMPLATE_JSON "$TAB_1[string2json {PL_INVALID}] : \{"
    set ii_last [expr [llength $SLR] - 1]
    set ii      0
    foreach idx_slr $SLR {
    lappend INVALID_TEMPLATE_JSON       "$TAB_2[string2json SLR_$idx_slr] : \["

        set example_locations {}
        foreach {site_key DYNAMIC_GEOMETRY_SITE} $PL_DYNAMIC_GEOMETRY {
            if {[dict exists $DYNAMIC_GEOMETRY_SITE $idx_slr]} {
                set DYNAMIC_GEOMETRY_SITE_SLR [dict get $DYNAMIC_GEOMETRY_SITE $idx_slr]
                set cr_y [lindex [lsort -dictionary [dict keys $DYNAMIC_GEOMETRY_SITE_SLR]] 0]

                set DYNAMIC_GEOMETRY_SITE_SLR_Y [dict get $DYNAMIC_GEOMETRY_SITE_SLR $cr_y]
                set cr_x [lindex [lsort -dictionary [dict keys $DYNAMIC_GEOMETRY_SITE_SLR_Y]] 0]

                set DYNAMIC_GEOMETRY_SITE_SLR_Y_X [dict get $DYNAMIC_GEOMETRY_SITE_SLR_Y $cr_x]
                set x [lindex [lsort -dictionary [dict keys $DYNAMIC_GEOMETRY_SITE_SLR_Y_X]] 0]

                set COL_X  [lsort -dictionary [dict get $DYNAMIC_GEOMETRY_SITE_SLR_Y_X $x]]
                set y0 [lindex $COL_X 0]; set y1 [lindex $COL_X 1]; set y2 [lindex $COL_X 2]
                set site_type [dict get $SITES_TYPES $site_key]
                lappend example_locations "${site_type}_X${x}Y${y0}"
                lappend example_locations "${site_type}_X${x}Y${y1}:${site_type}_X${x}Y${y2}"
            }
        }

        set jj_last [expr [llength $example_locations] - 1]
        set jj      0
        foreach loc $example_locations {
            if {$jj < $jj_last} { set next "," } else { set next "" }
    lappend INVALID_TEMPLATE_JSON "$TAB_3\{ [string2json {location}] : [string2json $loc] \}$next"
            incr jj
        }

        if {$ii < $ii_last} { set next "," } else { set next "" }
    lappend INVALID_TEMPLATE_JSON       "$TAB_2\]$next"
        incr ii
    }

    lappend INVALID_TEMPLATE_JSON "$TAB_1\}"
    lappend INVALID_TEMPLATE_JSON "\}"

    set fp [open $invalid_template_json w]; puts $fp [join $INVALID_TEMPLATE_JSON "\n"]; close $fp
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-9} {INFO} "Successfully generated invalid_template.json file: $invalid_template_json"


    if {$dynamic_pblocks_in == {}} {
        remove_tmp_pblocks $dynamic_pblocks
    }
}

# vivado -mode tcl -source <path>/xbtest_wizard_vx_x/tcl/power/gen_dynamic_geometry.tcl -tclargs <path>/xbtest_wizard_vx_x/xclbin_generate/output/xilinx_u55c_gen3x16_xdma_2_202110_1/1103_release/u_1103_release_ex/run/vpp_link/link/vivado/vpl/prj/prj.runs/impl_1/level0_wrapper_opt.dcp ./pwr_cfg/ pblock_dynamic_SLR0 pblock_dynamic_SLR1

proc print_gen_dynamic_geometry_help {} {
    puts {Usage:}
    puts {  * Run script from Linux console in batch mode:}
    puts {      vivado -mode tcl -source <xbtest>/tcl/power/gen_dynamic_geometry.tcl -tclargs <dcp_name> <pwr_floorplan_dir> <dynamic_pblocks>}
    puts {}
    puts {  * Run script from Vivado TCL console with DCP already opened:}
    puts {      set DCP_OPENED true}
    puts {      set argv \{<dcp_name> <pwr_floorplan_dir> <dynamic_pblocks>\}; set argc \[llength $argv\]}
    puts {      source <xbtest>/tcl/power/gen_dynamic_geometry.tcl}
    puts {}
    puts {Where:}
    puts {  <dcp_name>          : Path to DCP}
    puts {  <pwr_floorplan_dir> : Path to output directory where dynamic_geometry.json, utilization_template.json and invalid_template.json will be generated}
    puts {  <dynamic_pblocks>   : Names of the dynamic region pblocks depending on your platform. For example:}
    puts {    * xilinx_u25_gen3x8_xdma_1_202010_1       : pblock_dynamic_SLR0}
    puts {    * xilinx_u50_gen3x16_xdma_4_202020_1      : pblock_dynamic_SLR0 pblock_dynamic_SLR1}
    puts {    * xilinx_u55c_gen3x16_xdma_2_202110_1     : pblock_dynamic_SLR0 pblock_dynamic_SLR1 pblock_dynamic_SLR2}
    puts {    * xilinx_u250_gen3x16_xdma_4_1_202020_1   : pblock_dynamic_SLR0 pblock_dynamic_SLR1 pblock_dynamic_SLR2 pblock_dynamic_SLR3}
    puts {    * xilinx_vck5000_gen3x16_xdma_1_202110_1  : pblock_level0_ulp}
    puts {}
    puts {Example commands:}
    puts {  * From Linux console in batch mode:}
    puts {      vivado -mode    tcl \\}
    puts {             -source  <xbtest>/tcl/power/gen_dynamic_geometry.tcl \\}
    puts {             -tclargs <xbtest>/xclbin_generate/output/<platform>/init/u_init_ex/run/vpp_link/link/vivado/vpl/prj/prj.runs/impl_1/level0_wrapper_opt.dcp <xbtest>/xclbin_generate/cfg/<platform>/pwr_cfg/ pblock_dynamic_SLR0 pblock_dynamic_SLR1 pblock_dynamic_SLR2}
    puts {}
    puts {  * Equivalent but from Vivado TCL console with DCP already opened:}
    puts {      set DCP_OPENED true}
    puts {      set argv \{<xbtest>/xclbin_generate/output/<platform>/init/u_init_ex/run/vpp_link/link/vivado/vpl/prj/prj.runs/impl_1/level0_wrapper_opt.dcp <xbtest>/xclbin_generate/cfg/<platform>/pwr_cfg/ pblock_dynamic_SLR0 pblock_dynamic_SLR1 pblock_dynamic_SLR2\}; set argc \[llength $argv\]}
    puts {      source <xbtest>/tcl/power/gen_dynamic_geometry.tcl}
    puts {}
}

# DCP_OPENED can be set to true to run this script in Vivado TCL console with a DCP already opened
if {![info exists DCP_OPENED]} {
    set DCP_OPENED false
}
# Check arguments exists
if {![info exists argv]} {
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-14} {ERROR} {argv input is not defined}
}
if {![info exists argc]} {
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-15} {ERROR} {argc input is not defined}
}

if {$argc >= 2} {
    set dcp_name            [lindex $argv 0]
    set pwr_floorplan_dir   [lindex $argv 1]
    set dynamic_pblocks     {}
    for {set ii 2} {$ii < $argc} {incr ii} {
        lappend dynamic_pblocks [lindex $argv $ii]
    }
} else {
    print_gen_dynamic_geometry_help
    common::send_msg_id {GEN_DYNAMIC_GEOMETRY-16} {ERROR} {Wrong arguments}
}

if {!$DCP_OPENED} {
    open_checkpoint $dcp_name
}

gen_dynamic_geometry $pwr_floorplan_dir $dynamic_pblocks

common::send_msg_id {GEN_DYNAMIC_GEOMETRY-17} {INFO} {Done!}

if {!$DCP_OPENED} {
    exit 0
}