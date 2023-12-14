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

####################################################################################################################
# This is a generated file. Use and modify at your own risk.
####################################################################################################################

proc add_hdl_parameter { config_ref kernel_name param_name param_value } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set_property value $param_value [ipx::add_user_parameter  $param_name [ipx::current_core]]
    set_property value $param_value [ipx::get_user_parameters $param_name -of_objects [ipx::current_core]]
    set_property value $param_value [ipx::get_hdl_parameters  $param_name -of_objects [ipx::current_core]]

    log_message $config {PACKAGE_KERNEL-4} [list $param_name $kernel_name $param_value]
}
proc add_parameter_quiet { config_ref kernel_name param_name param_value } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set_property value $param_value [ipx::add_user_parameter  $param_name [ipx::current_core]]
    set_property value $param_value [ipx::get_user_parameters $param_name -of_objects [ipx::current_core]]
}
proc add_parameter { config_ref kernel_name param_name param_value } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    add_parameter_quiet config $kernel_name $param_name $param_value
    log_message $config {PACKAGE_KERNEL-4} [list $param_name $kernel_name $param_value]
}
proc set_bus_parameter { config_ref kernel_name bus_if_name param_name param_value } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set bus_if      [ipx::get_bus_interfaces $bus_if_name -of_objects [ipx::current_core]]
    set bus_param   [ipx::get_bus_parameters $param_name -of_objects $bus_if]
    if {$bus_param == {}} {
        set bus_param [ipx::add_bus_parameter $param_name $bus_if]
    }
    set_property value $param_value $bus_param
    log_message $config {PACKAGE_KERNEL-4} [list $bus_if_name->$param_name $kernel_name $param_value]
}

proc add_bus_interface { config_ref kernel_name bus_if_name interface_mode abstraction_type_vlnv bus_type_vlnv {master_address_space_ref {}} {slave_memory_map_ref {}}} {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    log_message $config {PACKAGE_KERNEL-6} [list $bus_type_vlnv $kernel_name $bus_if_name]
    set bus_if [ipx::add_bus_interface $bus_if_name [ipx::current_core]]
    set_property interface_mode         $interface_mode         $bus_if
    set_property abstraction_type_vlnv  $abstraction_type_vlnv  $bus_if
    set_property bus_type_vlnv          $bus_type_vlnv          $bus_if
    if {$master_address_space_ref != {}} {
        set_property master_address_space_ref $master_address_space_ref $bus_if
    }
    if {$slave_memory_map_ref != {}} {
        set_property slave_memory_map_ref $slave_memory_map_ref $bus_if
    }
}
proc set_port_map { bus_if_name port_map_name physical_name } {
    set port_map [ipx::add_port_map $port_map_name [ipx::get_bus_interfaces $bus_if_name -of_objects [ipx::current_core]]]
    set_property physical_name $physical_name $port_map
}
proc set_obj_property { prop val obj obj_desc } {
    set obj_prop [list_property $obj]
    if {[lsearch -nocase $obj_prop $prop] != -1} {
        common::send_msg_id {PACKAGE_KERNEL-4} {INFO} "Set property $prop with value ($val) for $obj_desc"
        set_property $prop $val $obj
    }
}


proc package_one_kernel { config_ref packaged_kernel_dir cu_cfg } {
    variable C_KRNL_MODE_VERIFY
    variable C_KRNL_MODE_POWER
    variable C_KRNL_MODE_MEMORY
    variable C_KRNL_MODE_GT_MAC
    variable C_KRNL_MODE_GT_LPBK
    variable C_KRNL_MODE_GT_PRBS
    variable C_KRNL_MODE_GTF_PRBS
    variable C_KRNL_MODE_GTM_PRBS
    variable C_KRNL_MODE_GTYP_PRBS
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set kernel_name [dict get $cu_cfg kernel_name]
    log_message $config {PACKAGE_KERNEL-3} [list $kernel_name]

    set path_to_tmp_project [file join [dict get $config run_dir] tmp_kernel_pack]

    create_project -force kernel_pack $path_to_tmp_project

    set ip_rep_dir  [file join $packaged_kernel_dir ip_rep]
    set ip_name_v   [dict get $config ip_name_v]
    set ip_catalog  [dict get $config ip_catalog]

    file delete -force -- $ip_rep_dir; file mkdir $ip_rep_dir
    file copy -force [file join $ip_catalog $ip_name_v] $ip_rep_dir

    set_property ip_repo_paths $ip_rep_dir [current_project]
    update_ip_catalog

    set component_xml   [file join $ip_rep_dir $ip_name_v component.xml]
    set packaged_dir    [file join $packaged_kernel_dir $kernel_name]
    set packaged_zip    [file join $packaged_kernel_dir ${kernel_name}.zip]
    ipx::edit_ip_in_project -upgrade true -name tmp_edit_project -directory $packaged_dir $component_xml

    # Common parameters
    # Set messages by default to be sure they will be loaded.
    add_parameter_quiet config $kernel_name MESSAGES_DICT [dict get $config MESSAGES_DICT]
    add_hdl_parameter   config $kernel_name C_CLOCK0_FREQ [dict get $config wizard_actual_config cu_configuration clock 0 freq]
    add_hdl_parameter   config $kernel_name C_CLOCK1_FREQ [dict get $config wizard_actual_config cu_configuration clock 1 freq]

    set krnl_mode [dict get $cu_cfg krnl_mode]

    add_hdl_parameter config $kernel_name C_KRNL_MODE       $krnl_mode
    add_hdl_parameter config $kernel_name C_KRNL_SLR        [dict get $cu_cfg slr_idx]
    add_parameter     config $kernel_name C_STOP_POST_OPT   [dict get $config STOP_POST_OPT]

    # Common interfaces
    set ap_clk_associated_busif [list]

    # s_axi_control
    add_bus_interface config $kernel_name s_axi_control slave xilinx.com:interface:aximm_rtl:1.0 xilinx.com:interface:aximm:1.0 {} s_axi_control
    set_port_map s_axi_control ARADDR  s_axi_control_araddr
    set_port_map s_axi_control ARREADY s_axi_control_arready
    set_port_map s_axi_control ARVALID s_axi_control_arvalid
    set_port_map s_axi_control AWADDR  s_axi_control_awaddr
    set_port_map s_axi_control AWREADY s_axi_control_awready
    set_port_map s_axi_control AWVALID s_axi_control_awvalid
    set_port_map s_axi_control BREADY  s_axi_control_bready
    set_port_map s_axi_control BRESP   s_axi_control_bresp
    set_port_map s_axi_control BVALID  s_axi_control_bvalid
    set_port_map s_axi_control RDATA   s_axi_control_rdata
    set_port_map s_axi_control RREADY  s_axi_control_rready
    set_port_map s_axi_control RRESP   s_axi_control_rresp
    set_port_map s_axi_control RVALID  s_axi_control_rvalid
    set_port_map s_axi_control WDATA   s_axi_control_wdata
    set_port_map s_axi_control WSTRB   s_axi_control_wstrb
    set_port_map s_axi_control WREADY  s_axi_control_wready
    set_port_map s_axi_control WVALID  s_axi_control_wvalid

    set_bus_parameter config $kernel_name s_axi_control PROTOCOL                 AXI4LITE
    set_bus_parameter config $kernel_name s_axi_control READ_WRITE_MODE          READ_WRITE
    set_bus_parameter config $kernel_name s_axi_control DATA_WIDTH               32
    set_bus_parameter config $kernel_name s_axi_control ADDR_WIDTH               12

    set addr_space [ipx::add_address_space s_axi_control [ipx::current_core]]
    set_obj_property range                  "0xFFFFFFFFFFFFFFFF"                                                $addr_space "address space s_axi_control"
    set_obj_property range_dependency       "pow(2,spirit:decode(id('MODELPARAM_VALUE.C_M00_AXI_ADDR_WIDTH')))" $addr_space "address space s_axi_control"
    set_obj_property range_resolve_type     "dependent"                                                         $addr_space "address space s_axi_control"
    set_obj_property width                  "32"                                                                $addr_space "address space s_axi_control"
    set_obj_property width_dependency       "spirit:decode(id('MODELPARAM_VALUE.C_M00_AXI_DATA_WIDTH'))"        $addr_space "address space s_axi_control"
    set_obj_property width_resolve_type     "dependent"                                                         $addr_space "address space s_axi_control"

    set mem_map [ipx::add_memory_map s_axi_control [ipx::current_core]]
    set_obj_property name           "s_axi_control"                 $mem_map "memory map s_axi_control"
    set_obj_property description    "Memory Map for s_axi_control"  $mem_map "memory map s_axi_control"

    set addr_block [ipx::add_address_block reg0 $mem_map]
    set_obj_property name                           "reg0"                                                                      $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property description                    "s_axi_control reg0 Registers"                                              $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property base_address_bit_string_length "32"                                                                        $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property base_address_format            "bitString"                                                                 $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property base_address_resolve_type      "user"                                                                      $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property range                          "0x1000"                                                                    $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property range_dependency               "pow(2,spirit:decode(id('MODELPARAM_VALUE.C_S_AXI_CONTROL_ADDR_WIDTH')))"   $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property range_resolve_type             "dependent"                                                                 $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property usage                          "register"                                                                  $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property width                          "32"                                                                        $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property width_dependency               "spirit:decode(id('MODELPARAM_VALUE.C_S_AXI_CONTROL_DATA_WIDTH'))"          $addr_block "address block reg0 of memory map s_axi_control"
    set_obj_property width_resolve_type             "dependent"                                                                 $addr_block "address block reg0 of memory map s_axi_control"

    set address_offset 0

    set reg [ipx::add_register Control $addr_block]
    set_obj_property name           "Control"                               $reg "register Control of address block reg0"
    set_obj_property address_offset "[format {0x%03x} $address_offset]"     $reg "register Control of address block reg0"
    set_obj_property size           "1"                                     $reg "register Control of address block reg0"
    set address_offset [expr {$address_offset + 16}]

    for {set ii 0} {$ii < 4} {incr ii} {
        set reg_name scalar0$ii
        set reg [ipx::add_register $reg_name $addr_block]
        set_obj_property name           "$reg_name"                             $reg "register $reg_name of address block reg0"
        set_obj_property address_offset "[format {0x%03x} $address_offset]"     $reg "register $reg_name of address block reg0"
        set_obj_property size           "4"                                     $reg "register $reg_name of address block reg0"
        set address_offset [expr {$address_offset + 8}]
    }

    for {set ii 0} {$ii <= 64} {incr ii} {
        set reg_name axi[format "%02d" $ii]_ptr0
        set reg [ipx::add_register $reg_name $addr_block]
        set_obj_property name           "$reg_name"                             $reg "register $reg_name of address block reg0"
        set_obj_property address_offset "[format {0x%03x} $address_offset]"     $reg "register $reg_name of address block reg0"
        set_obj_property size           "8"                                     $reg "register $reg_name of address block reg0"
        set address_offset [expr {$address_offset + 8}]
    }

    lappend ap_clk_associated_busif s_axi_control

    # m00_axi
    add_bus_interface config $kernel_name m00_axi master xilinx.com:interface:aximm_rtl:1.0 xilinx.com:interface:aximm:1.0 m00_axi
    set_port_map m00_axi AWVALID m00_axi_awvalid
    set_port_map m00_axi AWREADY m00_axi_awready
    set_port_map m00_axi AWADDR  m00_axi_awaddr
    set_port_map m00_axi AWLEN   m00_axi_awlen
    set_port_map m00_axi WVALID  m00_axi_wvalid
    set_port_map m00_axi WREADY  m00_axi_wready
    set_port_map m00_axi WDATA   m00_axi_wdata
    set_port_map m00_axi WSTRB   m00_axi_wstrb
    set_port_map m00_axi WLAST   m00_axi_wlast
    set_port_map m00_axi BVALID  m00_axi_bvalid
    set_port_map m00_axi BREADY  m00_axi_bready
    set_port_map m00_axi ARVALID m00_axi_arvalid
    set_port_map m00_axi ARREADY m00_axi_arready
    set_port_map m00_axi ARADDR  m00_axi_araddr
    set_port_map m00_axi ARLEN   m00_axi_arlen
    set_port_map m00_axi RVALID  m00_axi_rvalid
    set_port_map m00_axi RREADY  m00_axi_rready
    set_port_map m00_axi RDATA   m00_axi_rdata
    set_port_map m00_axi RLAST   m00_axi_rlast

    set_bus_parameter config $kernel_name m00_axi PROTOCOL                 AXI4
    set_bus_parameter config $kernel_name m00_axi READ_WRITE_MODE          READ_WRITE
    set_bus_parameter config $kernel_name m00_axi ADDR_WIDTH               64
    set_bus_parameter config $kernel_name m00_axi MAX_BURST_LENGTH         64
    set_bus_parameter config $kernel_name m00_axi NUM_WRITE_THREADS        1
    set_bus_parameter config $kernel_name m00_axi NUM_READ_THREADS         1
    set_bus_parameter config $kernel_name m00_axi NUM_WRITE_OUTSTANDING    64
    set_bus_parameter config $kernel_name m00_axi NUM_READ_OUTSTANDING     64

    set addr_space [ipx::add_address_space m00_axi [ipx::current_core]]
    set_obj_property range                  "0xFFFFFFFFFFFFFFFF"                                                $addr_space "address space m00_axi"
    set_obj_property range_dependency       "pow(2,spirit:decode(id('MODELPARAM_VALUE.C_M00_AXI_ADDR_WIDTH')))" $addr_space "address space m00_axi"
    set_obj_property range_resolve_type     "dependent"                                                         $addr_space "address space m00_axi"
    set_obj_property width                  "32"                                                                $addr_space "address space m00_axi"
    set_obj_property width_dependency       "spirit:decode(id('MODELPARAM_VALUE.C_M00_AXI_DATA_WIDTH'))"        $addr_space "address space m00_axi"
    set_obj_property width_resolve_type     "dependent"                                                         $addr_space "address space m00_axi"

    lappend ap_clk_associated_busif m00_axi

    # ap_rst_n
    add_bus_interface config $kernel_name ap_rst_n slave xilinx.com:signal:reset_rtl:1.0 xilinx.com:signal:reset:1.0
    set_port_map ap_rst_n RST ap_rst_n
    set_bus_parameter config $kernel_name ap_rst_n POLARITY ACTIVE_LOW

    # ap_rst_n_2
    add_bus_interface config $kernel_name ap_rst_n_2 slave xilinx.com:signal:reset_rtl:1.0 xilinx.com:signal:reset:1.0
    set_port_map ap_rst_n_2 RST ap_rst_n_2
    set_bus_parameter config $kernel_name ap_rst_n_2 POLARITY ACTIVE_LOW
    set_property enablement_value false [ipx::get_bus_interfaces ap_rst_n_2 -of_objects [ipx::current_core]]

    # ap_clk
    add_bus_interface config $kernel_name ap_clk slave xilinx.com:signal:clock_rtl:1.0 xilinx.com:signal:clock:1.0
    set_port_map ap_clk CLK ap_clk
    set_bus_parameter config $kernel_name ap_clk ASSOCIATED_RESET ap_rst_n

    # ap_clk_2
    add_bus_interface config $kernel_name ap_clk_2 slave xilinx.com:signal:clock_rtl:1.0 xilinx.com:signal:clock:1.0
    set_port_map ap_clk_2 CLK ap_clk_2
    set_bus_parameter config $kernel_name ap_clk_2 ASSOCIATED_RESET ap_rst_n_2
    set_property enablement_value false [ipx::get_bus_interfaces ap_clk_2 -of_objects [ipx::current_core]]

    # Vitis will connect both ap_clk*_cont to ap_clk by default if they are declared as clock
    # ap_clk_cont
    # add_bus_interface config $kernel_name ap_clk_cont slave xilinx.com:signal:clock_rtl:1.0 xilinx.com:signal:clock:1.0
    # set_port_map ap_clk_cont CLK ap_clk_cont
    # set_bus_parameter config $kernel_name ap_clk_cont ASSOCIATED_RESET ap_rst_n

    # ap_clk_2_cont
    # add_bus_interface config $kernel_name ap_clk_2_cont slave xilinx.com:signal:clock_rtl:1.0 xilinx.com:signal:clock:1.0
    # set_port_map ap_clk_2_cont CLK ap_clk_2_cont
    # set_bus_parameter config $kernel_name ap_clk_2_cont ASSOCIATED_RESET ap_rst_n_2
    # set_property enablement_value false [ipx::get_bus_interfaces ap_clk_2_cont -of_objects [ipx::current_core]]
    set_property enablement_value false [ipx::get_ports ap_clk_2_cont -of_objects [ipx::current_core]]

    # interrupt
    add_bus_interface config $kernel_name interrupt master xilinx.com:signal:interrupt_rtl:1.0 xilinx.com:signal:interrupt:1.0
    set_port_map interrupt INTERRUPT interrupt
    set_bus_parameter config $kernel_name interrupt SENSITIVITY LEVEL_HIGH

    # watchdog_alarm
    set_property enablement_value false [ipx::get_ports watchdog_alarm_out -of_objects [ipx::current_core]]
    set_property enablement_value true  [ipx::get_ports watchdog_alarm_in  -of_objects [ipx::current_core]]

    # ap_clk_div2/4
    # Vitis will connect both ap_clk_div* to ap_clk by default if they are declared as clock
    set_property enablement_value false [ipx::get_ports ap_clk_div2_out -of_objects [ipx::current_core]]
    set_property enablement_value false [ipx::get_ports ap_clk_div4_out -of_objects [ipx::current_core]]

    # Memory interfaces
    # mXX_axi
    for {set ii 1} {$ii <= 64} {incr ii} {
        set m_axi m[format "%02d" $ii]_axi

        add_bus_interface config $kernel_name $m_axi master xilinx.com:interface:aximm_rtl:1.0 xilinx.com:interface:aximm:1.0 $m_axi
        set_port_map $m_axi AWID    ${m_axi}_awid
        set_port_map $m_axi AWVALID ${m_axi}_awvalid
        set_port_map $m_axi AWREADY ${m_axi}_awready
        set_port_map $m_axi AWADDR  ${m_axi}_awaddr
        set_port_map $m_axi AWLEN   ${m_axi}_awlen
        set_port_map $m_axi WVALID  ${m_axi}_wvalid
        set_port_map $m_axi WREADY  ${m_axi}_wready
        set_port_map $m_axi WDATA   ${m_axi}_wdata
        set_port_map $m_axi WSTRB   ${m_axi}_wstrb
        set_port_map $m_axi WLAST   ${m_axi}_wlast
        set_port_map $m_axi BID     ${m_axi}_bid
        set_port_map $m_axi BVALID  ${m_axi}_bvalid
        set_port_map $m_axi BREADY  ${m_axi}_bready
        set_port_map $m_axi ARID    ${m_axi}_arid
        set_port_map $m_axi ARVALID ${m_axi}_arvalid
        set_port_map $m_axi ARREADY ${m_axi}_arready
        set_port_map $m_axi ARADDR  ${m_axi}_araddr
        set_port_map $m_axi ARLEN   ${m_axi}_arlen
        set_port_map $m_axi RID     ${m_axi}_rid
        set_port_map $m_axi RVALID  ${m_axi}_rvalid
        set_port_map $m_axi RREADY  ${m_axi}_rready
        set_port_map $m_axi RDATA   ${m_axi}_rdata
        set_port_map $m_axi RLAST   ${m_axi}_rlast
        set_property enablement_value false [ipx::get_bus_interfaces $m_axi -of_objects [ipx::current_core]]

        lappend ap_clk_associated_busif $m_axi

        set addr_space [ipx::add_address_space $m_axi [ipx::current_core]]
        set_obj_property range                  "0xFFFFFFFFFFFFFFFF"                                                                    $addr_space "address space $m_axi"
        set_obj_property range_dependency       "pow(2,spirit:decode(id('MODELPARAM_VALUE.C_M[format "%02d" $ii]_AXI_ADDR_WIDTH')))"    $addr_space "address space $m_axi"
        set_obj_property range_resolve_type     "dependent"                                                                             $addr_space "address space $m_axi"
        set_obj_property width                  "32"                                                                                    $addr_space "address space $m_axi"
        set_obj_property width_dependency       "spirit:decode(id('MODELPARAM_VALUE.C_M[format "%02d" $ii]_AXI_DATA_WIDTH'))"           $addr_space "address space $m_axi"
        set_obj_property width_resolve_type     "dependent"                                                                             $addr_space "address space $m_axi"
    }

    # Power interfaces
    # pwr cascade ports
    set pwr_cascade_ports_in [list \
        pwr_clk_in \
        pwr_throttle_in \
        pwr_FF_en_in \
        pwr_DSP_en_in \
        pwr_BRAM_en_in \
        pwr_URAM_en_in \
    ]
    foreach port $pwr_cascade_ports_in {
        set_property enablement_value false [ipx::get_ports $port -of_objects [ipx::current_core]]
    }

    set pwr_cascade_ports_out [list \
        pwr_clk_out \
        pwr_throttle_out \
        pwr_FF_en_out \
        pwr_DSP_en_out \
        pwr_BRAM_en_out \
        pwr_URAM_en_out \
    ]
    foreach port $pwr_cascade_ports_out {
        set_property enablement_value false [ipx::get_ports $port -of_objects [ipx::current_core]]
    }

    # m_axis_aie0
    add_bus_interface config $kernel_name m_axis_aie0 master xilinx.com:interface:axis_rtl:1.0 xilinx.com:interface:axis:1.0 m_axis_aie0
    set_port_map m_axis_aie0 TDATA  m_axis_aie0_tdata
    set_port_map m_axis_aie0 TVALID m_axis_aie0_tvalid
    set_port_map m_axis_aie0 TREADY m_axis_aie0_tready
    set_port_map m_axis_aie0 TKEEP  m_axis_aie0_tkeep
    set_port_map m_axis_aie0 TLAST  m_axis_aie0_tlast
    set_property enablement_value false [ipx::get_bus_interfaces m_axis_aie0 -of_objects [ipx::current_core]]

    lappend ap_clk_associated_busif m_axis_aie0

    set addr_space [ipx::add_address_space m_axis_aie0 [ipx::current_core]]
    set_obj_property width                  "32"                                                                $addr_space "address space m_axis_aie0"
    set_obj_property width_dependency       "spirit:decode(id('MODELPARAM_VALUE.C_AXIS_AIE_DATA_WIDTH'))"       $addr_space "address space m_axis_aie0"
    set_obj_property width_resolve_type     "dependent"                                                         $addr_space "address space m_axis_aie0"

    # s_axis_aie0
    add_bus_interface config $kernel_name s_axis_aie0 slave xilinx.com:interface:axis_rtl:1.0 xilinx.com:interface:axis:1.0
    set_port_map s_axis_aie0 TDATA  s_axis_aie0_tdata
    set_port_map s_axis_aie0 TVALID s_axis_aie0_tvalid
    set_port_map s_axis_aie0 TREADY s_axis_aie0_tready
    set_port_map s_axis_aie0 TKEEP  s_axis_aie0_tkeep
    set_port_map s_axis_aie0 TLAST  s_axis_aie0_tlast
    set_property enablement_value false [ipx::get_bus_interfaces s_axis_aie0 -of_objects [ipx::current_core]]

    lappend ap_clk_associated_busif s_axis_aie0

    # GT interfaces
    # gt_refclk
    add_bus_interface config $kernel_name gt_refclk slave xilinx.com:interface:diff_clock_rtl:1.0 xilinx.com:interface:diff_clock:1.0
    set_port_map gt_refclk CLK_P QSFP_CK_P
    set_port_map gt_refclk CLK_N QSFP_CK_N
    set_property enablement_value false [ipx::get_bus_interfaces gt_refclk -of_objects [ipx::current_core]]

    # gt_serial_port
    add_bus_interface config $kernel_name gt_serial_port master xilinx.com:interface:gt_rtl:1.0 xilinx.com:interface:gt:1.0
    set_port_map gt_serial_port GRX_P QSFP_RX_P
    set_port_map gt_serial_port GRX_N QSFP_RX_N
    set_port_map gt_serial_port GTX_P QSFP_TX_P
    set_port_map gt_serial_port GTX_N QSFP_TX_N
    set_property enablement_value false [ipx::get_bus_interfaces gt_serial_port -of_objects [ipx::current_core]]

    # gt_recov_clk
    # recov clocks are defined as ports in pfm currently

    for {set ii 0} {$ii < 32} {incr ii} {
        set gt_refclk_name      gt_refclk_[format "%02d" $ii]
        set gt_serial_port_name gt_serial_port_[format "%02d" $ii]

        # gt_refclk_*
        add_bus_interface config $kernel_name $gt_refclk_name slave xilinx.com:interface:diff_clock_rtl:1.0 xilinx.com:interface:diff_clock:1.0
        set_port_map $gt_refclk_name CLK_P QSFP_CK_P_[format "%02d" $ii]
        set_port_map $gt_refclk_name CLK_N QSFP_CK_N_[format "%02d" $ii]
        set_property enablement_value false [ipx::get_bus_interfaces $gt_refclk_name -of_objects [ipx::current_core]]

        # gt_serial_port_*
        add_bus_interface config $kernel_name $gt_serial_port_name master xilinx.com:interface:gt_rtl:1.0 xilinx.com:interface:gt:1.0
        set_port_map $gt_serial_port_name GRX_P QSFP_RX_P_[format "%02d" $ii]
        set_port_map $gt_serial_port_name GRX_N QSFP_RX_N_[format "%02d" $ii]
        set_port_map $gt_serial_port_name GTX_P QSFP_TX_P_[format "%02d" $ii]
        set_port_map $gt_serial_port_name GTX_N QSFP_TX_N_[format "%02d" $ii]
        set_property enablement_value false [ipx::get_bus_interfaces $gt_serial_port_name -of_objects [ipx::current_core]]

        # gt_recov_clk_*
        # recov clocks are defined as ports in pfm currently
        set_property enablement_value false [ipx::get_ports gt_recov_clk_p_[format "%02d" $ii] -of_objects [ipx::current_core]]
        set_property enablement_value false [ipx::get_ports gt_recov_clk_n_[format "%02d" $ii] -of_objects [ipx::current_core]]
        set_property enablement_value false [ipx::get_ports gt_recov_clk_[format "%02d" $ii] -of_objects [ipx::current_core]]
    }

    # Clocks associated bus interfaces
    # ap_clk
    set_bus_parameter config $kernel_name ap_clk ASSOCIATED_BUSIF [join $ap_clk_associated_busif {:}]

    # Customize parameters
    if {$krnl_mode == $C_KRNL_MODE_POWER} {
        set throttle_mode    [dict get $cu_cfg throttle_mode]
        set c_use_aie        [dict get $cu_cfg use_aie]
        if {$throttle_mode == {EXTERNAL_MACRO}} {
            set c_throttle_mode 2
        } elseif {$throttle_mode == {EXTERNAL_CLK}} {
            set c_throttle_mode 3
        } elseif {$throttle_mode == {INTERNAL_MACRO}} {
            set c_throttle_mode 0
        } else {
            set c_throttle_mode 1; # default mode INTERNAL_CLK
        }

        add_hdl_parameter   config $kernel_name C_THROTTLE_MODE     $c_throttle_mode
        add_hdl_parameter   config $kernel_name C_USE_AIE           $c_use_aie
        add_parameter_quiet config $kernel_name C_DYNAMIC_GEOMETRY  [dict get $config C_DYNAMIC_GEOMETRY]
        add_parameter_quiet config $kernel_name C_INVALID           [dict get $config C_INVALID]
        add_parameter_quiet config $kernel_name C_UTILIZATION       [dict get $config C_UTILIZATION]

        set_property enablement_value true [ipx::get_bus_interfaces ap_rst_n_2  -of_objects [ipx::current_core]]
        set_property enablement_value true [ipx::get_bus_interfaces ap_clk_2    -of_objects [ipx::current_core]]
        set_property enablement_value true [ipx::get_ports ap_clk_2_cont        -of_objects [ipx::current_core]]

        if {(($c_throttle_mode == 2) || ($c_throttle_mode == 3))} {
            foreach port $pwr_cascade_ports_in {
                set_property enablement_value true [ipx::get_ports $port -of_objects [ipx::current_core]]
            }
            # Disable kernel control and plram interfaces for external throttle_mode
            set_property enablement_value false [ipx::get_bus_interfaces s_axi_control -of_objects [ipx::current_core]]
            set_property enablement_value false [ipx::get_bus_interfaces interrupt     -of_objects [ipx::current_core]]
            set_property enablement_value false [ipx::get_bus_interfaces m00_axi       -of_objects [ipx::current_core]]
        } else {
            foreach port $pwr_cascade_ports_out {
                set_property enablement_value true [ipx::get_ports $port -of_objects [ipx::current_core]]
            }
        }
        if {$c_use_aie} {
            set aie_type    [dict get $cu_cfg aie_type]
            set aie_control [dict get $cu_cfg aie_control]
            set aie_status  [dict get $cu_cfg aie_status]
            if {$aie_type == {AIE_ML_CORE}} {
                add_hdl_parameter config $kernel_name C_AXIS_AIE_DATA_WIDTH 32
            }

            if {$aie_control == {pl}} {
                set_property enablement_value true [ipx::get_bus_interfaces m_axis_aie0 -of_objects [ipx::current_core]]
                set_bus_parameter config $kernel_name m_axis_aie0 LAYERED_METADATA   generated
                set_bus_parameter config $kernel_name m_axis_aie0 TUSER_WIDTH        0
            }
            if {$aie_status == {pl}} {
                set_property enablement_value true [ipx::get_bus_interfaces s_axis_aie0 -of_objects [ipx::current_core]]
                set_bus_parameter config $kernel_name s_axis_aie0 LAYERED_METADATA   generated
                set_bus_parameter config $kernel_name s_axis_aie0 TUSER_WIDTH        0
            }
        }

    } elseif {$krnl_mode == $C_KRNL_MODE_MEMORY} {
        set num_port [dict get $cu_cfg num_port]

        add_hdl_parameter config $kernel_name C_NUM_USED_M_AXI  $num_port
        add_hdl_parameter config $kernel_name C_MEM_KRNL_INST   [dict get $cu_cfg inst]

        set type [dict get $cu_cfg type]
        if {$type == {single_channel}} {
            set c_mem_type 1
        } else {
            set c_mem_type 2
        }
        add_hdl_parameter config $kernel_name C_MEM_TYPE $c_mem_type

        set axi_data_size   [dict get $cu_cfg axi_data_size]
        set axi_id_threads  [dict get $cu_cfg axi_id_threads]
        set axi_outstanding [dict get $cu_cfg axi_outstanding]
        if {$axi_id_threads == 1} {
            set c_use_axi_id 0
            set c_axi_thread_id_width 1
        } else {
            set c_use_axi_id 1
            set c_axi_thread_id_width [expr int( log($axi_id_threads) / log(2) )]
        }
        add_hdl_parameter config $kernel_name C_USE_AXI_ID $c_use_axi_id

        for {set ii 1} {$ii <= $num_port} {incr ii} {
            add_hdl_parameter config $kernel_name C_M[format "%02d" $ii]_AXI_DATA_WIDTH         $axi_data_size
            add_hdl_parameter config $kernel_name C_M[format "%02d" $ii]_AXI_THREAD_ID_WIDTH    $c_axi_thread_id_width
        }

        for {set ii 1} {$ii <= $num_port} {incr ii} {
            set m_axi m[format "%02d" $ii]_axi

            set_property enablement_value true [ipx::get_bus_interfaces $m_axi -of_objects [ipx::current_core]]
            set_bus_parameter config $kernel_name $m_axi PROTOCOL                 AXI4
            set_bus_parameter config $kernel_name $m_axi READ_WRITE_MODE          READ_WRITE
            set_bus_parameter config $kernel_name $m_axi ADDR_WIDTH               64
            set_bus_parameter config $kernel_name $m_axi MAX_BURST_LENGTH         64
            set_bus_parameter config $kernel_name $m_axi NUM_WRITE_THREADS        $axi_id_threads
            set_bus_parameter config $kernel_name $m_axi NUM_READ_THREADS         $axi_id_threads
            set_bus_parameter config $kernel_name $m_axi NUM_WRITE_OUTSTANDING    $axi_outstanding
            set_bus_parameter config $kernel_name $m_axi NUM_READ_OUTSTANDING     $axi_outstanding
        }

    } elseif {($krnl_mode == $C_KRNL_MODE_GT_LPBK) || ($krnl_mode == $C_KRNL_MODE_GT_MAC) || ($krnl_mode == $C_KRNL_MODE_GT_PRBS)} {

        ## config common to all GT CU

        set gt_idx  [dict get $cu_cfg gt_idx]
        set gt_type [dict get $cu_cfg type]
        add_hdl_parameter config $kernel_name C_GT_INDEX $gt_idx

        add_parameter config $kernel_name GT_${gt_idx}_GROUP_SELECT [dict get $cu_cfg group_select]
        add_parameter config $kernel_name GT_TYPE                   $gt_type

        set_property enablement_value true [ipx::get_bus_interfaces gt_refclk      -of_objects [ipx::current_core]]
        set_property enablement_value true [ipx::get_bus_interfaces gt_serial_port -of_objects [ipx::current_core]]
    }

    if {($krnl_mode == $C_KRNL_MODE_GTF_PRBS) || ($krnl_mode == $C_KRNL_MODE_GTM_PRBS) || ($krnl_mode == $C_KRNL_MODE_GTYP_PRBS)} {

        foreach {gt_idx cfg} [dict get $cu_cfg gt_cfg] {
            set gt_refclk_name          gt_refclk_[format "%02d" $gt_idx]
            set gt_serial_port_name     gt_serial_port_[format "%02d" $gt_idx]
            set c_quad_en_name          C_QUAD_EN_[format "%02d" $gt_idx]
            set gt_type_name            GT_TYPE_[format "%02d" $gt_idx]
            set c_gt_refclk_sel_name    C_GT_REFCLK_SEL_[format "%02d" $gt_idx]

            set enable  [dict get $cfg enable]
            set gt_type [dict get $cfg type]
            set gt_refclk_sel [dict get $cfg refclk_sel]

            if {$enable} {
                set c_quad_en 1
                set_property enablement_value true [ipx::get_bus_interfaces $gt_refclk_name      -of_objects [ipx::current_core]]
                set_property enablement_value true [ipx::get_bus_interfaces $gt_serial_port_name -of_objects [ipx::current_core]]
            } else {
                set c_quad_en 0
                set gt_refclk_sel $gt_idx
            }
            if {$gt_refclk_sel != $gt_idx} {
                set_property enablement_value false [ipx::get_bus_interfaces $gt_refclk_name      -of_objects [ipx::current_core]]
            }
            add_hdl_parameter   config $kernel_name $c_quad_en_name         $c_quad_en
            add_hdl_parameter   config $kernel_name $c_gt_refclk_sel_name   $gt_refclk_sel
            add_parameter       config $kernel_name $gt_type_name           $gt_type
        }
    }


    if {[dict exists $cu_cfg gt_rate]} {
        set gt_rate [dict get $cu_cfg gt_rate]
        if {$gt_rate == {10G_25G}} {
            set C_GT_RATE 0
        } elseif {$gt_rate == {10G}} {
            set C_GT_RATE 1
        } elseif {$gt_rate == {25G}} {
            set C_GT_RATE 2
        }
    }

    if {$krnl_mode == $C_KRNL_MODE_GT_MAC} {
        # default value: 1 GTY with 4 lanes, 10/25GbE
        set C_GT_TYPE             0
        set C_GT_NUM_GT           1
        set C_GT_NUM_LANE         4
        set C_GT_MAC_ENABLE_RSFEC 0

        set ip_sel       [dict get $cu_cfg ip_sel]
        set enable_rsfec [dict get $cu_cfg enable_rsfec]

        #set_property value $gt_mac_ip_sel [ipx::get_user_parameters GT_MAC_IP_SEL -of_objects [ipx::current_core]]
        # convert string into integer
        # Only two values possible for gt_mac_ip_sel (see validate_gt_mac_ip_sel)
        if {$ip_sel == {xbtest_sub_xxv_gt}} {
            set C_GT_MAC_IP_SEL 1
            # versal: 1 GTY with 4 lanes, fix 25GbE
            set C_GT_TYPE       0
            set C_GT_NUM_GT     1
            set C_GT_NUM_LANE   4
        } elseif {$ip_sel == {xxv}} {
            set C_GT_MAC_IP_SEL 0
        }

        if {$enable_rsfec} {
            set C_GT_MAC_ENABLE_RSFEC 1
        } else {
            set C_GT_MAC_ENABLE_RSFEC 0
        }

        add_parameter     config $kernel_name GT_MAC_IP_SEL         $ip_sel
        add_parameter     config $kernel_name ENABLE_RSFEC          $enable_rsfec

        add_hdl_parameter config $kernel_name C_GT_MAC_IP_SEL       $C_GT_MAC_IP_SEL
        add_hdl_parameter config $kernel_name C_GT_TYPE             $C_GT_TYPE
        add_hdl_parameter config $kernel_name C_GT_NUM_GT           $C_GT_NUM_GT
        add_hdl_parameter config $kernel_name C_GT_NUM_LANE         $C_GT_NUM_LANE
        add_hdl_parameter config $kernel_name C_GT_RATE             $C_GT_RATE
        add_hdl_parameter config $kernel_name C_GT_MAC_ENABLE_RSFEC $C_GT_MAC_ENABLE_RSFEC
    }

    if {$krnl_mode == $C_KRNL_MODE_VERIFY} {
        set dna_read [dict get $cu_cfg dna_read]
        if {$dna_read} {
            set C_DNA_READ 1
        } else {
            set C_DNA_READ 0
        }

        add_hdl_parameter   config $kernel_name C_DNA_READ     $C_DNA_READ

        set_property enablement_value true [ipx::get_bus_interfaces ap_rst_n_2  -of_objects [ipx::current_core]]
        set_property enablement_value true [ipx::get_bus_interfaces ap_clk_2    -of_objects [ipx::current_core]]
        set_property enablement_value true [ipx::get_ports ap_clk_2_cont        -of_objects [ipx::current_core]]

        set_property enablement_value true  [ipx::get_ports watchdog_alarm_out -of_objects [ipx::current_core]]
        set_property enablement_value false [ipx::get_ports watchdog_alarm_in  -of_objects [ipx::current_core]]

        set_property enablement_value true  [ipx::get_ports ap_clk_div2_out -of_objects [ipx::current_core]]
        set_property enablement_value true  [ipx::get_ports ap_clk_div4_out -of_objects [ipx::current_core]]
        set_property enablement_value false [ipx::get_ports ap_clk_div2_in  -of_objects [ipx::current_core]]
        set_property enablement_value false [ipx::get_ports ap_clk_div4_in  -of_objects [ipx::current_core]]
    }

    if {$krnl_mode == $C_KRNL_MODE_GT_PRBS} {
        set C_GT_NUM_GT     1
        set C_GT_NUM_LANE   4

        set ip_sel  [dict get $cu_cfg ip_sel]
        if {$ip_sel == {xbtest_sub_gt}} {
            set C_GT_IP_SEL 1
        } elseif {$ip_sel == {gt_wiz}} {
            set C_GT_IP_SEL 0
        }

        add_parameter     config $kernel_name GT_TYPE       "GTY"
        add_parameter     config $kernel_name GT_IP_SEL     $ip_sel
        add_hdl_parameter config $kernel_name C_GT_IP_SEL   $C_GT_IP_SEL
        add_hdl_parameter config $kernel_name C_GT_RATE     $C_GT_RATE
        add_hdl_parameter config $kernel_name C_GT_NUM_GT   $C_GT_NUM_GT
        add_hdl_parameter config $kernel_name C_GT_NUM_LANE $C_GT_NUM_LANE
    }

    if {$krnl_mode == $C_KRNL_MODE_GTF_PRBS} {
        set C_GT_NUM_GT     32
        set C_GT_NUM_LANE   4

        foreach {gt_idx cfg} [dict get $cu_cfg gt_cfg] {
            set enable [dict get $cfg enable]
            if {$enable} {
                set gt_rate [dict get $cfg gt_rate]
                # 10G
                set C_GT_RATE 1

                set ip_sel [dict get $cfg ip_sel]
                # gtf_wiz
                set C_GT_IP_SEL 2

                set gt_ip_sel_name      GT_IP_SEL_[format "%02d" $gt_idx]
                set c_gt_ip_sel_name    C_GT_IP_SEL_[format "%02d" $gt_idx]
                set c_gt_rate_name      C_GT_RATE_[format "%02d" $gt_idx]

                add_parameter     config $kernel_name $gt_ip_sel_name       $ip_sel
                add_hdl_parameter config $kernel_name $c_gt_ip_sel_name     $C_GT_IP_SEL
                add_hdl_parameter config $kernel_name $c_gt_rate_name       $C_GT_RATE

                # Enable for all enabled GTF quad as recov clock not defined in pfm metadata
                set_property enablement_value true [ipx::get_ports gt_recov_clk_p_[format "%02d" $gt_idx] -of_objects [ipx::current_core]]
                set_property enablement_value true [ipx::get_ports gt_recov_clk_n_[format "%02d" $gt_idx] -of_objects [ipx::current_core]]
                set_property enablement_value true [ipx::get_ports gt_recov_clk_[format "%02d" $gt_idx] -of_objects [ipx::current_core]]
            }
        }
        add_hdl_parameter config $kernel_name C_GT_NUM_GT    $C_GT_NUM_GT
        add_hdl_parameter config $kernel_name C_GT_NUM_LANE  $C_GT_NUM_LANE
    }

    if {$krnl_mode == $C_KRNL_MODE_GTM_PRBS} {
        set C_GT_NUM_GT     32
        set C_GT_NUM_LANE   4

        foreach {gt_idx cfg} [dict get $cu_cfg gt_cfg] {
            set enable [dict get $cfg enable]
            if {$enable} {
                set gt_rate [dict get $cfg gt_rate]
                #if {$gt_rate == {56G}} {
                    set C_GT_RATE 3
                #}
                set ip_sel [dict get $cfg ip_sel]
                #if {$ip_sel == {xbtest_sub_gt}} {
                    set C_GT_IP_SEL 1
                #}

                set gt_ip_sel_name      GT_IP_SEL_[format "%02d" $gt_idx]
                set c_gt_ip_sel_name    C_GT_IP_SEL_[format "%02d" $gt_idx]
                set c_gt_rate_name      C_GT_RATE_[format "%02d" $gt_idx]

                add_parameter     config $kernel_name GT_TYPE               "GTM"
                add_parameter     config $kernel_name $gt_ip_sel_name       $ip_sel
                add_hdl_parameter config $kernel_name $c_gt_ip_sel_name     $C_GT_IP_SEL
                add_hdl_parameter config $kernel_name $c_gt_rate_name       $C_GT_RATE

            }
        }
        add_hdl_parameter config $kernel_name C_GT_NUM_GT    $C_GT_NUM_GT
        add_hdl_parameter config $kernel_name C_GT_NUM_LANE  $C_GT_NUM_LANE
    }

    if {$krnl_mode == $C_KRNL_MODE_GTYP_PRBS} {
        set C_GT_NUM_GT     32
        set C_GT_NUM_LANE   4

        foreach {gt_idx cfg} [dict get $cu_cfg gt_cfg] {
            set enable [dict get $cfg enable]
            if {$enable} {
                set gt_rate [dict get $cfg gt_rate]
                #if {$gt_rate == {32gbs}} {
                    set C_GT_RATE 4
                #}
                set ip_sel [dict get $cfg ip_sel]
                #if {$ip_sel == {xbtest_sub_gt}} {
                    set C_GT_IP_SEL 1
                #}

                set gt_ip_sel_name      GT_IP_SEL_[format "%02d" $gt_idx]
                set c_gt_ip_sel_name    C_GT_IP_SEL_[format "%02d" $gt_idx]
                set c_gt_rate_name      C_GT_RATE_[format "%02d" $gt_idx]

                add_parameter     config $kernel_name GT_TYPE               "GTYP"
                add_parameter     config $kernel_name $gt_ip_sel_name       $ip_sel
                add_hdl_parameter config $kernel_name $c_gt_ip_sel_name     $C_GT_IP_SEL
                add_hdl_parameter config $kernel_name $c_gt_rate_name       $C_GT_RATE

            }
        }
        add_hdl_parameter config $kernel_name C_GT_NUM_GT    $C_GT_NUM_GT
        add_hdl_parameter config $kernel_name C_GT_NUM_LANE  $C_GT_NUM_LANE
    }

    ####################################################################################################################
    # set new vlnv
    ####################################################################################################################
    set build_date [dict get $config build_date]

    set_property name           $kernel_name                                                                [ipx::current_core]
    set_property display_name   "xbtest $kernel_name compute unit"                                          [ipx::current_core]
    set_property description    "xbtest RTL kernel automatically generated from $ip_name_v on $build_date"  [ipx::current_core]

    # Overwrite value from wizard example design
    set_property supports_ooc singular [ipx::current_core]

    ipx::create_xgui_files  [ipx::current_core]
    ipx::update_checksums   [ipx::current_core]
    ipx::check_integrity    [ipx::current_core]

    ipx::archive_core $packaged_zip [ipx::current_core]

    ####################################################################################################################
    # Remove project
    ####################################################################################################################

    # delete $packaged_kernel_dir project
    close_project -delete
    # delete $path_to_tmp_project/kernel_pack project
    close_project -delete

    file delete -force $packaged_dir
    file mkdir $packaged_dir
    exec unzip $packaged_zip -d $packaged_dir
    file delete -force $packaged_zip
    file delete -force $path_to_tmp_project
    file delete -force $ip_rep_dir
}

proc package_kernel { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    log_message $config {PACKAGE_KERNEL-1}

    set packaged_kernel_dir [dict get $config packaged_kernel_dir]

    foreach {cu_cfg} [dict get $config cu_config] {
        set kernel_name [dict get $cu_cfg kernel_name]
        set kernel_xml  [dict get $cu_cfg kernel_xml]
        set kernel_xo   [dict get $cu_cfg kernel_xo]

        package_one_kernel config $packaged_kernel_dir $cu_cfg

        if {[file exists $kernel_xo]} {
            file delete -force $kernel_xo
        }
        package_xo -xo_path $kernel_xo -kernel_name $kernel_name -ip_directory [file join $packaged_kernel_dir $kernel_name] -kernel_xml [dict get $cu_cfg kernel_xml]
    }

    log_message $config {PACKAGE_KERNEL-2}
}
