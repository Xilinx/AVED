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

# Hierarchical cell: cfg_stat_mem_slr2
proc create_hier_cell_cfg_stat_mem_slr2 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_cfg_stat_mem_slr2() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_h2c


  # Create pins
  create_bd_pin -dir I -type clk clk
  create_bd_pin -dir I -type rst resetn_periph
  create_bd_pin -dir I -type rst resetn_ic

  # Create instance: cfg_stat_mem_slr2_sc, and set properties
  set cfg_stat_mem_slr2_sc [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect cfg_stat_mem_slr2_sc ]
  set_property -dict [list \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {1} \
  ] $cfg_stat_mem_slr2_sc


  # Create instance: cfg_stat_mem_slr2_ctrl, and set properties
  set cfg_stat_mem_slr2_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl cfg_stat_mem_slr2_ctrl ]
  set_property -dict [list \
    CONFIG.DATA_WIDTH {32} \
    CONFIG.SINGLE_PORT_BRAM {1} \
  ] $cfg_stat_mem_slr2_ctrl


  # Create instance: cfg_stat_mem_slr2_mem, and set properties
  set cfg_stat_mem_slr2_mem [ create_bd_cell -type ip -vlnv xilinx.com:ip:emb_mem_gen cfg_stat_mem_slr2_mem ]

  # Create interface connections
  connect_bd_intf_net -intf_net cfg_stat_mem_slr2_ctrl_BRAM_PORTA [get_bd_intf_pins cfg_stat_mem_slr2_ctrl/BRAM_PORTA] [get_bd_intf_pins cfg_stat_mem_slr2_mem/BRAM_PORTA]
  connect_bd_intf_net -intf_net cfg_stat_mem_slr2_sc_M00_AXI [get_bd_intf_pins cfg_stat_mem_slr2_sc/M00_AXI] [get_bd_intf_pins cfg_stat_mem_slr2_ctrl/S_AXI]
  connect_bd_intf_net -intf_net s_axi_h2c_1 [get_bd_intf_pins s_axi_h2c] [get_bd_intf_pins cfg_stat_mem_slr2_sc/S00_AXI]

  # Create port connections
  connect_bd_net -net clk_1 [get_bd_pins clk] [get_bd_pins cfg_stat_mem_slr2_sc/aclk] [get_bd_pins cfg_stat_mem_slr2_ctrl/s_axi_aclk]
  connect_bd_net -net resetn_ic_1 [get_bd_pins resetn_ic] [get_bd_pins cfg_stat_mem_slr2_sc/aresetn]
  connect_bd_net -net resetn_periph_1 [get_bd_pins resetn_periph] [get_bd_pins cfg_stat_mem_slr2_ctrl/s_axi_aresetn]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: cfg_stat_mem_slr1
proc create_hier_cell_cfg_stat_mem_slr1 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_cfg_stat_mem_slr1() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_h2c


  # Create pins
  create_bd_pin -dir I -type clk clk
  create_bd_pin -dir I -type rst resetn_periph
  create_bd_pin -dir I -type rst resetn_ic

  # Create instance: cfg_stat_mem_slr1_sc, and set properties
  set cfg_stat_mem_slr1_sc [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect cfg_stat_mem_slr1_sc ]
  set_property -dict [list \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {1} \
  ] $cfg_stat_mem_slr1_sc


  # Create instance: cfg_stat_mem_slr1_ctrl, and set properties
  set cfg_stat_mem_slr1_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl cfg_stat_mem_slr1_ctrl ]
  set_property -dict [list \
    CONFIG.DATA_WIDTH {32} \
    CONFIG.SINGLE_PORT_BRAM {1} \
  ] $cfg_stat_mem_slr1_ctrl


  # Create instance: cfg_stat_mem_slr1_mem, and set properties
  set cfg_stat_mem_slr1_mem [ create_bd_cell -type ip -vlnv xilinx.com:ip:emb_mem_gen cfg_stat_mem_slr1_mem ]

  # Create interface connections
  connect_bd_intf_net -intf_net cfg_stat_mem_slr1_ctrl_BRAM_PORTA [get_bd_intf_pins cfg_stat_mem_slr1_ctrl/BRAM_PORTA] [get_bd_intf_pins cfg_stat_mem_slr1_mem/BRAM_PORTA]
  connect_bd_intf_net -intf_net cfg_stat_mem_slr1_sc_M00_AXI [get_bd_intf_pins cfg_stat_mem_slr1_sc/M00_AXI] [get_bd_intf_pins cfg_stat_mem_slr1_ctrl/S_AXI]
  connect_bd_intf_net -intf_net s_axi_h2c_1 [get_bd_intf_pins s_axi_h2c] [get_bd_intf_pins cfg_stat_mem_slr1_sc/S00_AXI]

  # Create port connections
  connect_bd_net -net clk_1 [get_bd_pins clk] [get_bd_pins cfg_stat_mem_slr1_sc/aclk] [get_bd_pins cfg_stat_mem_slr1_ctrl/s_axi_aclk]
  connect_bd_net -net resetn_ic_1 [get_bd_pins resetn_ic] [get_bd_pins cfg_stat_mem_slr1_sc/aresetn]
  connect_bd_net -net resetn_periph_1 [get_bd_pins resetn_periph] [get_bd_pins cfg_stat_mem_slr1_ctrl/s_axi_aresetn]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: cfg_stat_mem_slr0
proc create_hier_cell_cfg_stat_mem_slr0 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_cfg_stat_mem_slr0() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_h2c


  # Create pins
  create_bd_pin -dir I -type clk clk
  create_bd_pin -dir I -type rst resetn_periph
  create_bd_pin -dir I -type rst resetn_ic

  # Create instance: cfg_stat_mem_slr0_sc, and set properties
  set cfg_stat_mem_slr0_sc [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect cfg_stat_mem_slr0_sc ]
  set_property -dict [list \
    CONFIG.NUM_MI {1} \
    CONFIG.NUM_SI {1} \
  ] $cfg_stat_mem_slr0_sc


  # Create instance: cfg_stat_mem_slr0_ctrl, and set properties
  set cfg_stat_mem_slr0_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl cfg_stat_mem_slr0_ctrl ]
  set_property -dict [list \
    CONFIG.DATA_WIDTH {32} \
    CONFIG.SINGLE_PORT_BRAM {1} \
  ] $cfg_stat_mem_slr0_ctrl


  # Create instance: cfg_stat_mem_slr0_mem, and set properties
  set cfg_stat_mem_slr0_mem [ create_bd_cell -type ip -vlnv xilinx.com:ip:emb_mem_gen cfg_stat_mem_slr0_mem ]

  # Create interface connections
  connect_bd_intf_net -intf_net cfg_stat_mem_slr0_ctrl_BRAM_PORTA [get_bd_intf_pins cfg_stat_mem_slr0_ctrl/BRAM_PORTA] [get_bd_intf_pins cfg_stat_mem_slr0_mem/BRAM_PORTA]
  connect_bd_intf_net -intf_net cfg_stat_mem_slr0_sc_M00_AXI [get_bd_intf_pins cfg_stat_mem_slr0_sc/M00_AXI] [get_bd_intf_pins cfg_stat_mem_slr0_ctrl/S_AXI]
  connect_bd_intf_net -intf_net s_axi_h2c_1 [get_bd_intf_pins s_axi_h2c] [get_bd_intf_pins cfg_stat_mem_slr0_sc/S00_AXI]

  # Create port connections
  connect_bd_net -net clk_1 [get_bd_pins clk] [get_bd_pins cfg_stat_mem_slr0_sc/aclk] [get_bd_pins cfg_stat_mem_slr0_ctrl/s_axi_aclk]
  connect_bd_net -net resetn_ic_1 [get_bd_pins resetn_ic] [get_bd_pins cfg_stat_mem_slr0_sc/aresetn]
  connect_bd_net -net resetn_periph_1 [get_bd_pins resetn_periph] [get_bd_pins cfg_stat_mem_slr0_ctrl/s_axi_aresetn]

  # Restore current instance
  current_bd_instance $oldCurInst
}

proc init_hier_cell { parentCell nameHier } {
    common::send_msg_id {BUILD_HW-12} {INFO}  "Creating new BD hier <$nameHier> in <$parentCell>."

    if { $parentCell eq "" || $nameHier eq "" } {
        common::send_msg_id {BUILD_HW-13} {ERROR} {init_hier_cell() - Empty argument(s)!}
    }

    # Get object for parentCell
    set parentObj [get_bd_cells $parentCell]
    if { $parentObj == "" } {
        common::send_msg_id {BUILD_HW-14} {ERROR}  "init_hier_cell() - Unable to find parent cell <$parentCell>"
    }

    # Make sure parentObj is hier blk
    set parentType [get_property TYPE $parentObj]
    if { $parentType ne "hier" } {
        common::send_msg_id {BUILD_HW-15} {ERROR}  "init_hier_cell() - Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."
    }

    # Save current instance; Restore later
    set oldCurInst [current_bd_instance .]

    # Set parent object as current
    current_bd_instance $parentObj

    # Create cell and set as current instance
    set hier_obj [create_bd_cell -type hier $nameHier]
    current_bd_instance $hier_obj

    return $oldCurInst
}

proc term_hier_cell { oldCurInst } {
    # Restore current instance
    current_bd_instance $oldCurInst
    save_bd_design
}

# Procedure to update design and add xbtest kernels

proc customize_bd_xbtest { design_cfg parentCell xbtest_example} {

    if { $parentCell eq "" } {
        set parentCell [get_bd_cells /]
    }
    set oldCurInst [init_hier_cell $parentCell xbtest]

    switch -nocase $xbtest_example {
      xbtest_verify { set xbtest_ipdefs [get_ipdefs -all {xilinx.com:ip:krnl_verify*:*}]}
      xbtest        { set xbtest_ipdefs [get_ipdefs -all {xilinx.com:ip:krnl_*} -filter {vlnv !~ xilinx.com:ip:krnl_powertest*:*}]}
      xbtest_stress { set xbtest_ipdefs [get_ipdefs -all {xilinx.com:ip:krnl_*:*}]}
      default       { common::send_msg_id {BUILD_HW-19} {ERROR} "XBTest application profile not recognized: $xbtest_example. Update json config"}
    }

    ## Instantiate kernels
    foreach ipdef $xbtest_ipdefs {
        set cell_name   [get_property NAME $ipdef]_1
        set ip_vlnv     [get_property VLNV $ipdef]
        create_bd_cell -type ip -vlnv $ip_vlnv $cell_name
    }

    ## Connect kernel clocks and resets
    connect_bd_net [get_bd_pins /clock_reset/clk_usr_0]      [get_bd_pins krnl_*/ap_clk] \
                                                             [get_bd_pins krnl_*/ap_clk_cont]
    connect_bd_net [get_bd_pins /clock_reset/clk_usr_1]      [get_bd_pins krnl_*/ap_clk_2] \
                                                             [get_bd_pins krnl_*/ap_clk_2_cont]

    connect_bd_net [get_bd_pins /clock_reset/resetn_usr_0_periph] [get_bd_pins krnl_*/ap_rst_n]
    connect_bd_net [get_bd_pins /clock_reset/resetn_usr_1_periph] [get_bd_pins krnl_*/ap_rst_n_2]

    # Instantiate cfg_stat_mems

    create_hier_cell_cfg_stat_mem_slr0 [current_bd_instance .] cfg_stat_mem_slr0
    create_hier_cell_cfg_stat_mem_slr1 [current_bd_instance .] cfg_stat_mem_slr1
    create_hier_cell_cfg_stat_mem_slr2 [current_bd_instance .] cfg_stat_mem_slr2

    connect_bd_net [get_bd_pins /clock_reset/clk_usr_0]                      [get_bd_pins cfg_stat_mem_slr*/clk]
    connect_bd_net [get_bd_pins /clock_reset/usr_0_psr/interconnect_aresetn] [get_bd_pins cfg_stat_mem_slr*/resetn_ic]
    connect_bd_net [get_bd_pins /clock_reset/resetn_usr_0_periph]            [get_bd_pins cfg_stat_mem_slr*/resetn_periph]

    set pcie_slr0_sc [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect pcie_slr0_sc ]
    set pcie_slr1_sc [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect pcie_slr1_sc ]
    set pcie_slr2_sc [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect pcie_slr2_sc ]

    set_property -dict [list \
      CONFIG.NUM_CLKS {2} \
      CONFIG.NUM_MI {1} \
      CONFIG.NUM_SI {1} \
    ] $pcie_slr0_sc

    set_property -dict [list \
      CONFIG.NUM_CLKS {2} \
      CONFIG.NUM_MI {1} \
      CONFIG.NUM_SI {1} \
    ] $pcie_slr1_sc

    set_property -dict [list \
      CONFIG.NUM_CLKS {2} \
      CONFIG.NUM_MI {1} \
      CONFIG.NUM_SI {1} \
    ] $pcie_slr2_sc

    connect_bd_net [get_bd_pins /cips/pl0_ref_clk]                        [get_bd_pins pcie_slr*_sc/aclk]
    connect_bd_net [get_bd_pins /clock_reset/clk_usr_0]                   [get_bd_pins pcie_slr*_sc/aclk1]
    connect_bd_net [get_bd_pins /clock_reset/pl_psr/interconnect_aresetn] [get_bd_pins pcie_slr*_sc/aresetn]

    connect_bd_intf_net [get_bd_intf_pins pcie_slr0_sc/M00_AXI] [get_bd_intf_pins cfg_stat_mem_slr0/s_axi_h2c]
    connect_bd_intf_net [get_bd_intf_pins pcie_slr1_sc/M00_AXI] [get_bd_intf_pins cfg_stat_mem_slr1/s_axi_h2c]
    connect_bd_intf_net [get_bd_intf_pins pcie_slr2_sc/M00_AXI] [get_bd_intf_pins cfg_stat_mem_slr2/s_axi_h2c]


    set_property -dict [list \
      CONFIG.NUM_MI {4} \
    ] [get_bd_cells /axi_noc_cips]

    set_property -dict [ list \
      CONFIG.DATA_WIDTH {32} \
      CONFIG.APERTURES {{0x201_0200_0000 0x200_0000}} \
      CONFIG.CATEGORY {pl} \
    ] [get_bd_intf_pins /axi_noc_cips/M01_AXI]

    set_property -dict [ list \
      CONFIG.DATA_WIDTH {32} \
      CONFIG.APERTURES {{0x201_0400_0000 0x200_0000}} \
      CONFIG.CATEGORY {pl} \
    ] [get_bd_intf_pins /axi_noc_cips/M02_AXI]

    set_property -dict [ list \
      CONFIG.DATA_WIDTH {32} \
      CONFIG.APERTURES {{0x201_0000_0000 1G}} \
      CONFIG.CATEGORY {pl} \
    ] [get_bd_intf_pins /axi_noc_cips/M03_AXI]

    set_property -dict [ list \
      CONFIG.CONNECTIONS {M00_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M01_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M02_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M03_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M00_INI { read_bw {800} write_bw {800} read_avg_burst {64} write_avg_burst {64} } M02_INI { read_bw {800} write_bw {800} read_avg_burst {64} write_avg_burst {64} }} \
      CONFIG.DEST_IDS {M00_AXI:0x100:M01_AXI:0x80:M02_AXI:0x40:M03_AXI:0xc0} \
    ] [get_bd_intf_pins /axi_noc_cips/S00_AXI]

    set_property -dict [ list \
      CONFIG.CONNECTIONS {M00_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M01_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M02_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M03_AXI { read_bw   {5} write_bw   {5} read_avg_burst {64} write_avg_burst {64} } M01_INI { read_bw {800} write_bw {800} read_avg_burst {64} write_avg_burst {64} } M03_INI { read_bw {800} write_bw {800} read_avg_burst {64} write_avg_burst {64} }} \
      CONFIG.DEST_IDS {M00_AXI:0x100:M01_AXI:0x80:M02_AXI:0x40:M03_AXI:0xc0} \
    ] [get_bd_intf_pins /axi_noc_cips/S01_AXI]

    connect_bd_intf_net [get_bd_intf_pins /axi_noc_cips/M01_AXI] [get_bd_intf_pins pcie_slr0_sc/S00_AXI]
    connect_bd_intf_net [get_bd_intf_pins /axi_noc_cips/M02_AXI] [get_bd_intf_pins pcie_slr1_sc/S00_AXI]
    connect_bd_intf_net [get_bd_intf_pins /axi_noc_cips/M03_AXI] [get_bd_intf_pins pcie_slr2_sc/S00_AXI]

    assign_bd_address -offset 0x020103000000 -range 0x00002000 -target_address_space [get_bd_addr_spaces cips/CPM_PCIE_NOC_0] [get_bd_addr_segs cfg_stat_mem_slr0/cfg_stat_mem_slr0_ctrl/S_AXI/Mem0] -force
    assign_bd_address -offset 0x020104000000 -range 0x00002000 -target_address_space [get_bd_addr_spaces cips/CPM_PCIE_NOC_0] [get_bd_addr_segs cfg_stat_mem_slr1/cfg_stat_mem_slr1_ctrl/S_AXI/Mem0] -force
    assign_bd_address -offset 0x020105000000 -range 0x00002000 -target_address_space [get_bd_addr_spaces cips/CPM_PCIE_NOC_0] [get_bd_addr_segs cfg_stat_mem_slr2/cfg_stat_mem_slr2_ctrl/S_AXI/Mem0] -force

    assign_bd_address -offset 0x020103000000 -range 0x00002000 -target_address_space [get_bd_addr_spaces cips/CPM_PCIE_NOC_1] [get_bd_addr_segs cfg_stat_mem_slr0/cfg_stat_mem_slr0_ctrl/S_AXI/Mem0] -force
    assign_bd_address -offset 0x020104000000 -range 0x00002000 -target_address_space [get_bd_addr_spaces cips/CPM_PCIE_NOC_1] [get_bd_addr_segs cfg_stat_mem_slr1/cfg_stat_mem_slr1_ctrl/S_AXI/Mem0] -force
    assign_bd_address -offset 0x020105000000 -range 0x00002000 -target_address_space [get_bd_addr_spaces cips/CPM_PCIE_NOC_1] [get_bd_addr_segs cfg_stat_mem_slr2/cfg_stat_mem_slr2_ctrl/S_AXI/Mem0] -force



    ## Kernel control and cfg_stat_mem
    # Note, some ranges in PCIe User are already used (see create_bd_design)
    #set pcie_user_offset [dict get $design_cfg design pcie_bars user offset]
    set pcie_mgmt_offset [dict get $design_cfg design pcie_bars mgmt offset]
    # TODO compute user IP offsets based on cfg_stat_mem addr/size
    set krnl_base_addr_slr [dict create \
        SLR0 [expr $pcie_mgmt_offset + 0x03010000] \
        SLR1 [expr $pcie_mgmt_offset + 0x04010000] \
        SLR2 [expr $pcie_mgmt_offset + 0x05010000] \
    ]
    set krnl_range 0x10000
    foreach cell [get_bd_cells krnl*] {
        set slr_idx [get_property CONFIG.C_KRNL_SLR $cell]

        # Connect kernel m00_axi to cfg_stat_mem
        set m00_axi [get_bd_intf_pins -quiet ${cell}/m00_axi]
        if {$m00_axi != {}} {
            set cfg_stat_mem_name cfg_stat_mem_slr${slr_idx}

            set num_si [get_property CONFIG.NUM_SI [get_bd_cells ${cfg_stat_mem_name}/${cfg_stat_mem_name}_sc]]
            set_property CONFIG.NUM_SI [expr $num_si + 1] [get_bd_cells ${cfg_stat_mem_name}/${cfg_stat_mem_name}_sc]

            set s_axi_name [format {S%02d_AXI} $num_si]
            connect_bd_intf_net $m00_axi [get_bd_intf_pins ${cfg_stat_mem_name}/${cfg_stat_mem_name}_sc/$s_axi_name]

            assign_bd_address -target_address_space [get_bd_addr_spaces $m00_axi] [get_bd_addr_segs ${cfg_stat_mem_name}/${cfg_stat_mem_name}_ctrl/S_AXI/Mem0] -force
        }

        # Connect kernel s_axi_control
        set s_axi_control [get_bd_intf_pins -quiet ${cell}/s_axi_control]
        if {$s_axi_control != {}} {
            set pcie_sc_name  pcie_slr${slr_idx}_sc

            set num_mi [get_property CONFIG.NUM_MI [get_bd_cells ${pcie_sc_name}]]
            set_property CONFIG.NUM_MI [expr $num_mi + 1] [get_bd_cells ${pcie_sc_name}]

            set m_axi_name [format {M%02d_AXI} $num_mi]
            connect_bd_intf_net [get_bd_intf_pins ${pcie_sc_name}/$m_axi_name] $s_axi_control

            set krnl_base_addr [dict get $krnl_base_addr_slr SLR${slr_idx}]

            for {set ii 0} {$ii < 2} {incr ii} {
                assign_bd_address -range $krnl_range -offset $krnl_base_addr -target_address_space [get_bd_addr_spaces /cips/CPM_PCIE_NOC_${ii}] [get_bd_addr_segs ${cell}/s_axi_control/reg0]
            }
            incr krnl_base_addr $krnl_range

            dict set krnl_base_addr_slr SLR${slr_idx} $krnl_base_addr
        }
    }

    ## Connect verify CU and other kernels
    connect_bd_net -quiet [get_bd_pins krnl_verify*/watchdog_alarm_out] [get_bd_pins krnl_*/watchdog_alarm_in]
    connect_bd_net -quiet [get_bd_pins krnl_verify*/ap_clk_div2_out]    [get_bd_pins krnl_*/ap_clk_div2_in]
    connect_bd_net -quiet [get_bd_pins krnl_verify*/ap_clk_div4_out]    [get_bd_pins krnl_*/ap_clk_div4_in]

    ## Connect power CUs together. Following power connectivity expects only one power CU with THROTTLE_MODE set to INTERNAL
    connect_bd_net -quiet [get_bd_pins -quiet krnl_powertest_slr*/pwr_clk_out]      [get_bd_pins -quiet krnl_powertest_slr*/pwr_clk_in]
    connect_bd_net -quiet [get_bd_pins -quiet krnl_powertest_slr*/pwr_throttle_out] [get_bd_pins -quiet krnl_powertest_slr*/pwr_throttle_in]
    connect_bd_net -quiet [get_bd_pins -quiet krnl_powertest_slr*/pwr_FF_en_out]    [get_bd_pins -quiet krnl_powertest_slr*/pwr_FF_en_in]
    connect_bd_net -quiet [get_bd_pins -quiet krnl_powertest_slr*/pwr_DSP_en_out]   [get_bd_pins -quiet krnl_powertest_slr*/pwr_DSP_en_in]
    connect_bd_net -quiet [get_bd_pins -quiet krnl_powertest_slr*/pwr_BRAM_en_out]  [get_bd_pins -quiet krnl_powertest_slr*/pwr_BRAM_en_in]
    connect_bd_net -quiet [get_bd_pins -quiet krnl_powertest_slr*/pwr_URAM_en_out]  [get_bd_pins -quiet krnl_powertest_slr*/pwr_URAM_en_in]

 
    ## Connect memory kernel to HBM
    if {[llength [get_bd_cells -hierarchical -filter {VLNV=~xilinx.com:ip:krnl_memtest_hbm_*:*}]] > 0} {
        set m00_axi [get_bd_intf_pins krnl_memtest_hbm_*/m00_axi]
        set mxx_axi {}
        foreach intf_pin [get_bd_intf_pins krnl_memtest_hbm_*/m*_axi] {
            if {$intf_pin != $m00_axi} {
                lappend mxx_axi $intf_pin
            }
        }

        set num_clks [get_property CONFIG.NUM_CLKS [get_bd_cells /axi_noc_cips]]
        set_property CONFIG.NUM_CLKS [expr $num_clks + 1] [get_bd_cells /axi_noc_cips]

        connect_bd_net [get_bd_pins /clock_reset/clk_usr_0] [get_bd_pins /axi_noc_cips/aclk${num_clks}]

        set hbm_ch_cfg [get_property CONFIG.HBM_CHNL0_CONFIG [get_bd_cells /axi_noc_cips]]
        dict set hbm_ch_cfg HBM_REFRESH_MODE                    {SINGLE_BANK_REFRESH}
        dict set hbm_ch_cfg HBM_PC0_PRE_DEFINED_ADDRESS_MAP     {USER_DEFINED_ADDRESS_MAP}
        dict set hbm_ch_cfg HBM_PC1_PRE_DEFINED_ADDRESS_MAP     {USER_DEFINED_ADDRESS_MAP}
        dict set hbm_ch_cfg HBM_PC0_USER_DEFINED_ADDRESS_MAP    {1BG-15RA-1SID-2BA-5CA-1BG}
        dict set hbm_ch_cfg HBM_PC1_USER_DEFINED_ADDRESS_MAP    {1BG-15RA-1SID-2BA-5CA-1BG}
        dict set hbm_ch_cfg HBM_PC0_ADDRESS_MAP                 {BA3,RA14,RA13,RA12,RA11,RA10,RA9,RA8,RA7,RA6,RA5,RA4,RA3,RA2,RA1,RA0,SID,BA1,BA0,CA5,CA4,CA3,CA2,CA1,BA2,NC,NA,NA,NA,NA}
        dict set hbm_ch_cfg HBM_PC1_ADDRESS_MAP                 {BA3,RA14,RA13,RA12,RA11,RA10,RA9,RA8,RA7,RA6,RA5,RA4,RA3,RA2,RA1,RA0,SID,BA1,BA0,CA5,CA4,CA3,CA2,CA1,BA2,NC,NA,NA,NA,NA}
        set_property CONFIG.HBM_CHNL0_CONFIG $hbm_ch_cfg [get_bd_cells /axi_noc_cips]

        set num_ch [llength $mxx_axi]
        if {$num_ch == 64} {
            set num_hbm_bli [get_property CONFIG.NUM_HBM_BLI [get_bd_cells /axi_noc_cips]]
            set_property CONFIG.NUM_HBM_BLI [expr $num_hbm_bli + $num_ch]  [get_bd_cells /axi_noc_cips]

            for {set ch_idx 0} {$ch_idx < $num_ch} {incr ch_idx} {
                set hbm_idx         [expr $ch_idx / 4]
                set hbm_port_idx    [expr $ch_idx % 4]

                set mxx_axi_name    [format   {m%02d_axi} [expr                $ch_idx + 1]]
                set mem_intf_name   [format {HBM%02d_AXI} [expr $num_hbm_bli + $ch_idx]]

                connect_bd_intf_net [get_bd_intf_pins krnl_memtest_hbm_*/${mxx_axi_name}]   [get_bd_intf_pins /axi_noc_cips/${mem_intf_name}]

                set_property -dict [ subst { CONFIG.CONNECTIONS { HBM${hbm_idx}_PORT${hbm_port_idx} { read_bw {12800} write_bw {12800} read_avg_burst {128} write_avg_burst {128}} }}] [get_bd_intf_pins /axi_noc_cips/${mem_intf_name}]
            }

            set hbm_addr 0x04000000000
            for {set ch_idx 0} {$ch_idx < $num_ch} {incr ch_idx} {
                set mxx_axi_name    [format   {m%02d_axi} [expr                $ch_idx + 1]]
                set mem_intf_name   [format {HBM%02d_AXI} [expr $num_hbm_bli + $ch_idx]]

                assign_bd_address -range 1G -offset $hbm_addr -target_address_space [get_bd_addr_spaces krnl_memtest_hbm_*/$mxx_axi_name] [get_bd_addr_segs /axi_noc_cips/${mem_intf_name}/HBM*_PC?]
                if {[expr $ch_idx % 2] == 1} {
                    incr hbm_addr 0x40000000
                }
            }

        } else {
            common::send_msg_id {BUILD_HW-18} {ERROR} "Connectivity not defined in customize_bd_xbtest.tcl for [llength $mxx_axi] connections of kernel krnl_memtest_hbm_*. Please update customize_bd_xbtest.tcl"
        }
    }

    ## Clock wizard configuration
    set clock_wiz_freq {}
    for {set ii 0} {$ii < 2} {incr ii} {
        lappend clock_wiz_freq [get_property CONFIG.C_CLOCK${ii}_FREQ [get_bd_cells krnl_verify_1]]
    }
    set_property CONFIG.CLKOUT_REQUESTED_OUT_FREQUENCY [join $clock_wiz_freq {,}] [get_bd_cells /clock_reset/usr_clk_wiz]

    term_hier_cell $oldCurInst
}

