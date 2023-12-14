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

proc get_xbtest_pci_ip_from_seg { design_cfg seg } {
    # xbtest IP segments are called SEG_krnl_*_reg0 where krnl_* is the xbtest IP instance name
    # xbtest status RAM segments are called SEG_krnl_*_ctrl_Mem0 where krnl_* is the xbtest IP instance name
    set seg_name    [get_property NAME $seg]
    set cell_name   [string map {{SEG_} {}} $seg_name]

    if {[string first {krnl_} $cell_name] == 0} {
        set cell_name   [string map {{_reg0} {}} $cell_name]
    } elseif {[string first {cfg_stat_mem_slr} $cell_name] == 0} {
        set cell_name   [string map {{_Mem0} {}} $cell_name]
    } else {
        return {}
    }

    set axi_offset  [get_property OFFSET $seg]

    set pcie_pf     {}
    set pcie_bar    {}

    foreach {bar_name bar_def} [dict get $design_cfg design pcie_bars] {
        set pcie_bar_axi_offset [dict get $bar_def offset]
        set pcie_bar_axi_range  [dict get $bar_def range]
        if {($axi_offset >= $pcie_bar_axi_offset) && ($axi_offset <= [expr $pcie_bar_axi_offset + $pcie_bar_axi_range])} {
            set pcie_pf     [dict get $bar_def physical_function]
            set pcie_bar    [dict get $bar_def bar]
            break
        }
    }

    if {$pcie_bar == {}} {
        return {}
    }

    set pcie_offset [expr $axi_offset - $pcie_bar_axi_offset]
    set pcie_range  [get_property RANGE  $seg]

    return [dict create \
        cell_name   $cell_name \
        pf          0x[format {%x} $pcie_pf] \
        bar         0x[format {%x} $pcie_bar] \
        offset      0x[format {%x} $pcie_offset] \
        range       0x[format {%x} $pcie_range] \
    ]
}

proc generate_xbtest_metadata { design_cfg app_metadata_dir } {

    common::send_msg_id {BUILD_HW-20} {INFO} {Generating xbtest metadata JSON file}

    ## partition_metadata

    set XBTEST_METADATA_JSON {}
    lappend XBTEST_METADATA_JSON "\{"
    lappend XBTEST_METADATA_JSON "  \"partition_metadata\": \{"
    lappend XBTEST_METADATA_JSON "    \"logic_uuid\": \"NOT_DEFINED\","

    lappend XBTEST_METADATA_JSON "    \"pcie\": \{"
    lappend XBTEST_METADATA_JSON "      \"bars\": \["
    foreach {bar_name bar_def} [dict get $design_cfg design pcie_bars] {
        lappend XBTEST_METADATA_JSON "        \{"
        lappend XBTEST_METADATA_JSON "          \"name\"                       : \"$bar_name\","
        lappend XBTEST_METADATA_JSON "          \"pcie_physical_function\"     : \"[format {0x%X} [dict get $bar_def physical_function]]\","
        lappend XBTEST_METADATA_JSON "          \"pcie_base_address_register\" : \"[format {0x%X} [dict get $bar_def bar]]\","
        lappend XBTEST_METADATA_JSON "          \"offset\"                     : \"[dict get $bar_def offset]\","
        lappend XBTEST_METADATA_JSON "          \"range\"                      : \"[dict get $bar_def range]\""
        lappend XBTEST_METADATA_JSON "        \},"
    }
    set XBTEST_METADATA_JSON [lrange $XBTEST_METADATA_JSON 0 end-1]
    lappend XBTEST_METADATA_JSON "        \}"
    lappend XBTEST_METADATA_JSON "      \]"
    lappend XBTEST_METADATA_JSON "    \},"

    lappend XBTEST_METADATA_JSON "    \"addressable_endpoints\": \{"

    set found_seg_names {}
    set     segments [ get_bd_addr_segs -quiet cips/CPM_PCIE_NOC_?/SEG_* ]
    lappend segments [ get_bd_addr_segs -quiet cips/CPM_PL_AXI?/SEG_* ]

    foreach seg $segments {
        set seg_name [get_property NAME $seg]

        if {[lsearch -exact $found_seg_names $seg_name] < 0} {

            set xbtest_pci_ip [get_xbtest_pci_ip_from_seg $design_cfg $seg]

            if {$xbtest_pci_ip != {}} {
                lappend XBTEST_METADATA_JSON "      \"[dict get $xbtest_pci_ip cell_name]\": \{"
                lappend XBTEST_METADATA_JSON "        \"offset\"                     : \"[dict get $xbtest_pci_ip offset]\","
                lappend XBTEST_METADATA_JSON "        \"range\"                      : \"[dict get $xbtest_pci_ip range]\","
                lappend XBTEST_METADATA_JSON "        \"pcie_physical_function\"     : \"[dict get $xbtest_pci_ip pf]\","
                lappend XBTEST_METADATA_JSON "        \"pcie_base_address_register\" : \"[dict get $xbtest_pci_ip bar]\""
                lappend XBTEST_METADATA_JSON "      \},"

                lappend found_seg_names $seg_name

            }
        }
    }
    set XBTEST_METADATA_JSON [lreplace $XBTEST_METADATA_JSON end end [string map {{,} {}} [lindex $XBTEST_METADATA_JSON end]]]; # Remove "," of last line

    lappend XBTEST_METADATA_JSON "    \}"
    lappend XBTEST_METADATA_JSON "  \},"

    ## mem_topology

    set mem_topology {}
    foreach seg [get_bd_addr_segs /xbtest/krnl_memtest_*/m*_axi/SEG_*] {
        set path        [get_property PATH $seg]
        set base_addr   [get_property OFFSET $seg]
        set size        [get_property RANGE  $seg]

        if {[string first {/m00_axi/} $path] < 0} {

            foreach name [split $path {/}] {
                if {[string first {krnl_memtest_} $name] == 0} {

                    set mem_type [string toupper [join [lrange [split $name {_}] 2 end-3] {_}]]

                    set found false

                    if {![dict exists $mem_topology $mem_type]} {
                       dict set mem_topology $mem_type {}
                    }

                    foreach {type mem_topology_type} $mem_topology {
                        foreach {tag topology} $mem_topology_type {
                            if {[dict get $topology base_addr] == $base_addr} {
                                set found true
                            }
                        }
                    }

                    if {!$found} {
                        set mem_idx [llength [dict keys [dict get $mem_topology $mem_type]]]
                        set mem_tag "$mem_type\[$mem_idx\]"

                        dict set mem_topology $mem_type $mem_tag base_addr $base_addr
                        dict set mem_topology $mem_type $mem_tag size      $size
                    }
                }
            }
        }
    }
    foreach seg [get_bd_addr_segs /xbtest/krnl_*/m00_axi/SEG_*] {
        set path        [get_property PATH $seg]
        set base_addr   [get_property OFFSET $seg]
        set size        [get_property RANGE  $seg]

        foreach name [split $path {/}] {
            if {[string first {cfg_stat_mem} $name] >= 0} {

                set mem_type [string toupper cfg_stat_mem]

                set found false

                if {![dict exists $mem_topology $mem_type]} {
                   dict set mem_topology $mem_type {}
                }

                foreach {type mem_topology_type} $mem_topology {
                    foreach {tag topology} $mem_topology_type {
                        if {[dict get $topology base_addr] == $base_addr} {
                            set found true
                        }
                    }
                }

                if {!$found} {
                    set mem_tag [join [lrange [split $name {_}] 1 end-1] {_}]

                    dict set mem_topology $mem_type $mem_tag base_addr $base_addr
                    dict set mem_topology $mem_type $mem_tag size      $size
                }
            }
        }
    }

    set mem_topology_list {}

    lappend XBTEST_METADATA_JSON "  \"mem_topology\": \{"
    lappend XBTEST_METADATA_JSON "    \"m_mem_data\": \["
    foreach {type mem_topology_type} $mem_topology {
        foreach {tag topology} $mem_topology_type {
            set base_addr   [dict get $topology base_addr]
            set size        [dict get $topology size]

            set base_addr   [string map {{_} {}} $base_addr]

            if {[string tolower [string range $size 0 1]] == {0x}} {
                scan [string range $size 2 end] %x decimal
                set size_kb $decimal
            }

            set size_unit [string index $size end]
            if {$size_unit == {G}} {
                set size_kb [string range $size 0 end-1]
                set size_kb [expr $size_kb * 1024 * 1024]
            } elseif {$size_unit == {M}} {
                set size_kb [string range $size 0 end-1]
                set size_kb [expr $size_kb * 1024]
            }  elseif {$size_unit == {K}} {
                set size_kb [string range $size 0 end-1]
            }  else {
                set size_kb [expr $size_kb / 1024]
            }
            set size_kb 0x[format %X $size_kb]

            lappend XBTEST_METADATA_JSON "      \{"
            lappend XBTEST_METADATA_JSON "        \"m_used\"        : \"1\","
            lappend XBTEST_METADATA_JSON "        \"m_sizeKB\"      : \"$size_kb\","
            lappend XBTEST_METADATA_JSON "        \"m_tag\"         : \"$tag\","
            lappend XBTEST_METADATA_JSON "        \"m_base_address\": \"$base_addr\""
            lappend XBTEST_METADATA_JSON "      \},"

            lappend mem_topology_list [dict create \
                m_sizeKB        $size_kb \
                m_tag           $tag \
                m_base_address  $base_addr \
            ]


        }
    }
    set XBTEST_METADATA_JSON [lreplace $XBTEST_METADATA_JSON end end [string map {{,} {}} [lindex $XBTEST_METADATA_JSON end]]]; # Remove "," of last line

    lappend XBTEST_METADATA_JSON "    \]"
    lappend XBTEST_METADATA_JSON "  \},"

    ## connectivity

    lappend XBTEST_METADATA_JSON "  \"connectivity\": \{"
    lappend XBTEST_METADATA_JSON "    \"m_connection\": \["

    foreach cell [get_bd_cells /xbtest/krnl*] {
        set cell_name   [get_property NAME [get_bd_cells $cell]]

        set mxx_axi [get_bd_intf_pins -quiet [lsort -dictionary [get_bd_intf_pins -quiet $cell/m*_axi]]]
        set arg_index 4

        foreach m_axi $mxx_axi {
            set mem_data_index -1
            for {set ii 0} {$ii < [llength $mem_topology_list]} {incr ii} {
                set seg_offset      [get_property OFFSET [get_bd_addr_segs -of $m_axi]]
                set mem_base_addr   [dict get [lindex $mem_topology_list $ii] m_base_address]
                if {$seg_offset == $mem_base_addr} {
                    set mem_data_index $ii
                    break
                }
            }
            if {$mem_data_index == -1} {
                common::send_msg_id {BUILD_HW-21} {ERROR} "Did not find target in memory topology for interface: $m_axi"
            }

            lappend XBTEST_METADATA_JSON "      \{"
            lappend XBTEST_METADATA_JSON "        \"ip_name\"       : \"$cell_name\","
            lappend XBTEST_METADATA_JSON "        \"arg_index\"     : \"$arg_index\","
            lappend XBTEST_METADATA_JSON "        \"mem_data_index\": \"$mem_data_index\""
            lappend XBTEST_METADATA_JSON "      \},"

            incr arg_index
        }
    }
    set XBTEST_METADATA_JSON [lreplace $XBTEST_METADATA_JSON end end [string map {{,} {}} [lindex $XBTEST_METADATA_JSON end]]]; # Remove "," of last line

    lappend XBTEST_METADATA_JSON "    \]"
    lappend XBTEST_METADATA_JSON "  \},"

    ## clock_freq_topology

    lappend XBTEST_METADATA_JSON "  \"clock_freq_topology\": \{"
    lappend XBTEST_METADATA_JSON "    \"m_clock_freq\": \["

    set m_clk_types     {DATA KERNEL}
    set m_clk_freq_Mhz  [get_property CONFIG.CLKOUT_REQUESTED_OUT_FREQUENCY [get_bd_cells /clock_reset/usr_clk_wiz]]
    set m_clk_freq_Mhz  [split $m_clk_freq_Mhz {,}]

    for {set ii 0} {$ii < 2} {incr ii} {
        lappend XBTEST_METADATA_JSON "      \{"
        lappend XBTEST_METADATA_JSON "        \"m_type\"    : \"[lindex $m_clk_types    $ii]\","
        lappend XBTEST_METADATA_JSON "        \"m_freq_Mhz\": \"[lindex $m_clk_freq_Mhz $ii]\""
        lappend XBTEST_METADATA_JSON "      \},"
    }
    set XBTEST_METADATA_JSON [lreplace $XBTEST_METADATA_JSON end end [string map {{,} {}} [lindex $XBTEST_METADATA_JSON end]]]; # Remove "," of last line

    lappend XBTEST_METADATA_JSON "    \]"
    lappend XBTEST_METADATA_JSON "  \}"
    lappend XBTEST_METADATA_JSON "\}"

    # puts "XBTEST_METADATA_JSON = [join $XBTEST_METADATA_JSON "\n"]"
    write_file [file join $app_metadata_dir "xbtest_metadata.json"] [join $XBTEST_METADATA_JSON "\n"]
}

proc add_xbtest_metadata_uuid { design_cfg app_metadata_dir uuid_dict } {

    common::send_msg_id {BUILD_HW-20} {INFO} {Adding UUID to xbtest metadata JSON file}

    # Load xbtest_metadata previously generated and edit partition_metadata.logic_uuid

    # set uuid [dict get $uuid_dict top_i]
    set uuid [lindex $uuid_dict 1]

    set XBTEST_METADATA_JSON {}
    foreach line [split [read_file [file join $app_metadata_dir "xbtest_metadata.json"]] "\n"] {
        if {[string first {logic_uuid} $line] >= 0} {
            lappend XBTEST_METADATA_JSON [string map [list {NOT_DEFINED} $uuid ] $line]
        } else {
            lappend XBTEST_METADATA_JSON $line
        }
    }

    write_file [file join $app_metadata_dir "xbtest_metadata.json"] [join $XBTEST_METADATA_JSON "\n"]
}

