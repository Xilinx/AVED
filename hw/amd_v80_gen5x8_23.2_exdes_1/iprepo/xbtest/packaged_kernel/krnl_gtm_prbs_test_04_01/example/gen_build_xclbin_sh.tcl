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

proc connect_power_cu_throttle { config } {
    variable C_KRNL_MODE_POWER
    set pwr_cu_cfg { \
        INTERNAL_CLK   {} \
        INTERNAL_MACRO {} \
        EXTERNAL_CLK   {} \
        EXTERNAL_MACRO {} \
    }
    foreach cu_cfg [dict get $config cu_config] {
        if {[dict get $cu_cfg krnl_mode] == $C_KRNL_MODE_POWER} {
            dict lappend pwr_cu_cfg [dict get $cu_cfg throttle_mode] [dict get $cu_cfg kernel_inst]
        }
    }
    # Check if inter-power CU connections are needed
    if {([dict get $pwr_cu_cfg EXTERNAL_CLK] == {}) && ([dict get $pwr_cu_cfg EXTERNAL_MACRO] == {})} {
        return {}
    }
    # Find master CU
    set master_inst {}
    set master_mode {}
    foreach throttle_mode {INTERNAL_CLK INTERNAL_MACRO} {
        set inst_list [dict get $pwr_cu_cfg $throttle_mode]
        if {$inst_list != {}} {
            set master_inst [lindex $inst_list 0]
            set master_mode $throttle_mode
            break
        }
    }
    # No master power CU found for connection to slave power CU
    if {$master_inst == {}} {
        log_message $config {GEN_BUILD_XCLBIN_SH-14}
        return {}
    }
    # Connect master power CU to all slave power CU
    set SP_OPTION {}
    lappend SP_OPTION "# Power CU throttle connections"
    foreach slave_mode {EXTERNAL_CLK EXTERNAL_MACRO} {
        foreach slave_inst [dict get $pwr_cu_cfg $slave_mode] {
            # Check if mixed mode was used
            if {(($slave_mode == {EXTERNAL_CLK}) && ($master_mode == {INTERNAL_MACRO})) || (($slave_mode == {EXTERNAL_MACRO}) && ($master_mode == {INTERNAL_CLK}))} {
                log_message $config {GEN_BUILD_XCLBIN_SH-15} [list $slave_inst $slave_mode $master_inst $master_mode]
            }
            lappend SP_OPTION "connect=${master_inst}/pwr_clk_out:${slave_inst}/pwr_clk_in"
            lappend SP_OPTION "connect=${master_inst}/pwr_throttle_out:${slave_inst}/pwr_throttle_in"
            lappend SP_OPTION "connect=${master_inst}/pwr_FF_en_out:${slave_inst}/pwr_FF_en_in"
            lappend SP_OPTION "connect=${master_inst}/pwr_DSP_en_out:${slave_inst}/pwr_DSP_en_in"
            lappend SP_OPTION "connect=${master_inst}/pwr_BRAM_en_out:${slave_inst}/pwr_BRAM_en_in"
            lappend SP_OPTION "connect=${master_inst}/pwr_URAM_en_out:${slave_inst}/pwr_URAM_en_in"
        }
    }
    return $SP_OPTION
}
proc check_vpp_ini { config vpp_ini } {
    log_message $config {GEN_BUILD_XCLBIN_SH-6} [list $vpp_ini]
    set infile [open $vpp_ini r]
    set postsys_link_ref false
    while {![eof $infile]} {
        set line [gets $infile]

        foreach gt_itf {gt_serial_port gt_refclk} {
            if {[regexp -- $gt_itf $line]} {
                log_message $config {GEN_BUILD_XCLBIN_SH-7} [list $gt_itf]
            }
        }

        if {[regexp -- {postsys_link.tcl} $line]} {
            set postsys_link_ref true
        }
   }
    if {!$postsys_link_ref} {
        log_message $config {GEN_BUILD_XCLBIN_SH-13} [list {postsys_link.tcl}]
    }
    close $infile
}

# Script that generates build_xclbin.sh from input configuration file
proc gen_build_xclbin_sh { config_ref } {
    variable C_KRNL_MODE_VERIFY
    variable C_KRNL_MODE_POWER
    variable C_KRNL_MODE_MEMORY
    variable C_KRNL_MODE_GT_MAC
    variable C_KRNL_MODE_GT_LPBK
    variable C_KRNL_MODE_GT_PRBS
    variable C_KRNL_MODE_GTF_PRBS
    variable C_KRNL_MODE_GTM_PRBS
    variable C_KRNL_MODE_GTYP_PRBS

    # Dictionary passed as ref.
    upvar 1 $config_ref config

    set build_xclbin_sh [file join [dict get $config run_dir] build_xclbin.sh]
    log_message $config {GEN_BUILD_XCLBIN_SH-1} [list $build_xclbin_sh]; # Start

    set xpfm                [dict_get_quiet $config xpfm]
    set clock0_freq         [dict get $config wizard_actual_config cu_configuration clock 0 freq]
    set clock1_freq         [dict get $config wizard_actual_config cu_configuration clock 1 freq]

    set use_aie                 0
    set num_aie_used            0
    set aie_type                {}
    set aie_constraints_json    {}

    ## Vitis inputs and options
    set KERNEL_XO   {}
    set NK_OPTION   {}
    set SLR_OPTION  {}
    set SP_OPTION   {}
    set SC_OPTION   {}

    foreach cu_cfg [dict get $config cu_config] {
        # Get config
        set kernel_name [dict get $cu_cfg kernel_name]
        set kernel_inst [dict get $cu_cfg kernel_inst]
        set kernel_xo   [dict get $cu_cfg kernel_xo]
        set slr_idx     [dict get $cu_cfg slr_idx]
        set krnl_mode   [dict get $cu_cfg krnl_mode]

        set krnl_use_aie 0
        if {$krnl_mode == $C_KRNL_MODE_POWER} {
            set krnl_use_aie [dict get $cu_cfg use_aie]
            if {$krnl_use_aie} {
                set use_aie                 1
                set num_aie_used            [dict get $cu_cfg num_aie_used]
                set aie_type                [dict get $cu_cfg aie_type]
                set aie_freq                [dict get $cu_cfg aie_freq]
                set aie_control             [dict get $cu_cfg aie_control]
                set aie_status              [dict get $cu_cfg aie_status]
                set aie_constraints_json    [dict get $cu_cfg aie_constraints_json]
                if {$aie_control == {pl}} {
                    lappend SC_OPTION "stream_connect=${kernel_inst}.m_axis_aie0:ai_engine_0.in0"
                }
                if {$aie_status == {pl}} {
                    lappend SC_OPTION "stream_connect=ai_engine_0.out0:${kernel_inst}.s_axis_aie0"
                }
            }
        }

        # Input XOs
        if {[file exists $kernel_xo]} {
            lappend KERNEL_XO "\${XBTEST_RUN_DIR}/kernel_xo/${kernel_name}.xo \\"
        } else {
            log_message $config {GEN_BUILD_XCLBIN_SH-2} [list $kernel_xo]; # ERROR file not found
        }

        # NK options
        lappend NK_OPTION "nk=${kernel_name}:1:${kernel_inst}"; # Set nk option

        # SP options
        # Find number of axi ports
        set connection_indexes [list]
        foreach key [dict keys $cu_cfg sp_m*_axi] {
            regexp {sp_m0*([0-9]+)_axi} $key -> connection_idx
            lappend connection_indexes $connection_idx
        }
        set num_connections [llength $connection_indexes]

        for {set ii 0} {$ii < $num_connections} {incr ii} {
            set connection_idx  [lindex $connection_indexes $ii]
            set port            [format {m%02d_axi} $connection_idx]
            set sptag           [dict get $cu_cfg [format {sp_m%02d_axi} $connection_idx]]

            log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name $port $sptag]
            lappend SP_OPTION "sp=${kernel_inst}.${port}:${sptag}"; # Set sp option
        }

        if {($krnl_mode == $C_KRNL_MODE_GT_LPBK) || ($krnl_mode == $C_KRNL_MODE_GT_MAC) || ($krnl_mode == $C_KRNL_MODE_GT_PRBS)} {
            set serial_port  [dict get $cu_cfg serial_port]
            set diff_clocks  [dict get $cu_cfg diff_clocks]

            log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name gt_serial_port $serial_port]
            lappend SP_OPTION "connect=${kernel_inst}/gt_serial_port:${serial_port}"
            if {[llength $diff_clocks] == 1} {
                log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name gt_refclk $diff_clocks]
                lappend SP_OPTION "connect=${diff_clocks}:${kernel_inst}/gt_refclk"
            } else {
                set ct 0
                foreach {io} $diff_clocks {
                    # for backward compatibility, gt_refclk_0 doesn't exist
                    if {$ct == 0} {
                        log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name gt_refclk $io]
                        lappend SP_OPTION "connect=${io}:${kernel_inst}/gt_refclk"
                    } else {
                        log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name gt_refclk_${ct} $io]
                        lappend SP_OPTION "connect=${io}:${kernel_inst}/gt_refclk_${ct}"
                    }
                    incr ct
                }
            }
        } elseif {($krnl_mode == $C_KRNL_MODE_GTF_PRBS) || ($krnl_mode == $C_KRNL_MODE_GTM_PRBS) || ($krnl_mode == $C_KRNL_MODE_GTYP_PRBS)} {
            foreach {gt_idx cfg} [dict get $cu_cfg gt_cfg] {
                set enable       [dict get $cfg enable]
                if {$enable} {
                    set serial_port     [dict get $cfg serial_port]
                    set diff_clocks     [dict get $cfg diff_clocks]

                    set gt_refclk_name      gt_refclk_[format "%02d" $gt_idx]
                    set gt_serial_port_name gt_serial_port_[format "%02d" $gt_idx]

                    log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name ${gt_serial_port_name} $serial_port]
                    # lappend SP_OPTION "connect=${kernel_inst}/${gt_serial_port_name}:${serial_port}"


                    if {[llength $diff_clocks] == 1} {
                        log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name $gt_refclk_name $diff_clocks]
                        # lappend SP_OPTION "connect=${diff_clocks}:${kernel_inst}/${gt_refclk_name}"
                    } else {
                        set ct 0
                        foreach {io} $diff_clocks {
                            set gt_refclk_ct_name   gt_refclk_${ct}_[format "%02d" $gt_idx]
                            # for backward compatibility, gt_refclk_0 doesn't exist
                            if {$ct == 0} {
                                log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name ${gt_refclk_name} $io]
                                # lappend SP_OPTION "connect=${io}:${kernel_inst}/${gt_refclk_name}"
                            } else {
                                log_message $config {GEN_BUILD_XCLBIN_SH-3} [list $kernel_name ${gt_refclk_ct_name} $io]
                                # lappend SP_OPTION "connect=${io}:${kernel_inst}/${gt_refclk_ct_name}"
                            }
                            incr ct
                        }
                    }
                }
            }
        }

        # Connect watchdog alarm from verify to all other CUs
        if {($krnl_mode != $C_KRNL_MODE_VERIFY) } {
            lappend SP_OPTION "connect=krnl_verify_1/watchdog_alarm_out:${kernel_inst}/watchdog_alarm_in"
            lappend SP_OPTION "connect=krnl_verify_1/ap_clk_div2_out:${kernel_inst}/ap_clk_div2_in"
            lappend SP_OPTION "connect=krnl_verify_1/ap_clk_div4_out:${kernel_inst}/ap_clk_div4_in"
        }

        # SLR options
        log_message $config {GEN_BUILD_XCLBIN_SH-4} [list $kernel_name $slr_idx]
        lappend SLR_OPTION "slr=${kernel_inst}:SLR${slr_idx}"; # Set slr option
    }

    # Power CU throttle connections
    set SP_OPTION [concat $SP_OPTION [connect_power_cu_throttle $config]]

    ## Set build script
    set     BUILD_XCLBIN_SH {}
    lappend BUILD_XCLBIN_SH "#!/bin/bash"
    lappend BUILD_XCLBIN_SH "# This is a generated file. Use and modify at your own risk."
    set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]
    if {$xpfm != {}} {
        lappend BUILD_XCLBIN_SH "XPFM=$xpfm"
    }
    set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_env_var_bash XPFM "Run build_xclbin.sh script"]]
    set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]

    lappend BUILD_XCLBIN_SH "XBTEST_RUN_DIR=[dict get $config run_dir]"
    lappend BUILD_XCLBIN_SH "XBTEST_OUTPUT_DIR=[dict get $config output_dir]"
    lappend BUILD_XCLBIN_SH "XBTEST_IP_CATALOG=[dict get $config ip_catalog]"
    lappend BUILD_XCLBIN_SH "XBTEST_IPDEF=\${XBTEST_IP_CATALOG}/[dict get $config ip_name_v]"

    if {$use_aie} {
        set XBTEST_AIE "[dict get $config ip_catalog]/[dict get $config ip_name_v]"
        if {$aie_type == {AIE_ML_CORE}} {
            set XBTEST_AIE "${XBTEST_AIE}/aie_ml"
        } else {
            set XBTEST_AIE "${XBTEST_AIE}/aie"
        }
        lappend BUILD_XCLBIN_SH "XBTEST_AIE=${XBTEST_AIE}"
    }
    set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]

    ## Set v++ aie compiler command
    #   - Currently does not support if AIE in more than one SLR
    #   - Optional open AIE compiler result in Vitis Analyzer. See graph, array, constraints etc
    #              vitis_analyzer ${XBTEST_RUN_DIR}/vpp_aie/graph.aiecompile_summary
    #   - Optional run simulation output generated in [dict get $config run_dir]/aeisimulator_output/data/output.txt and must match $aie_data/golden.txt
    #              aiesimulator --pkg-dir ${XBTEST_RUN_DIR}/vpp_aie --simulation-cycle-timeout=10000
    if {$use_aie} {
        set sh_vpp_aie_out  "\${XBTEST_RUN_DIR}/libadf.a"

        lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} {Executing Vitis AIE compiler}]
        lappend BUILD_XCLBIN_SH "v++ -c \\"
        lappend BUILD_XCLBIN_SH "--mode         aie \\";
        lappend BUILD_XCLBIN_SH "--platform     \${XPFM} \\"; # Specify path to a platform specification(XPFM) or hardware specification(XSA)
        lappend BUILD_XCLBIN_SH "--config       \${XBTEST_RUN_DIR}/vpp_aie.ini \\"
        lappend BUILD_XCLBIN_SH "--output       $sh_vpp_aie_out \\"; # Specify Vitis AIE compiler output on command line to allow reuse of vpp_aie.ini
        lappend BUILD_XCLBIN_SH "\${XBTEST_AIE}/graph.cpp"
        lappend BUILD_XCLBIN_SH ""; # end of command

        # Check the license is set
        lappend BUILD_XCLBIN_SH "if \[ -z \"\$\{XILINXD_LICENSE_FILE\}\" \]; then"
        lappend BUILD_XCLBIN_SH "\t[log_message_bash {CRITICAL WARNING} {XBTEST_WIZARD-14} {Environment variable XILINXD_LICENSE_FILE not set for AIE compiler}]"
        lappend BUILD_XCLBIN_SH "fi"

        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_ret_code_bash   {Vitis AIE compiler}]]
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_file_bash       $sh_vpp_aie_out]]
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]
    }

    ## Set v++ link command
    #   - if vpp_link_output = xclbin, an xclbin is generated by Vitis linker and Vitis packager is not run
    #   - if vpp_link_output = xsa,    an xsa is generated by Vitis linker and an xclbin is generated by Vitis packager
    set vpp_link_output [dict_get_quiet $config {wizard_actual_config build vpp_link_output}]
    if {$vpp_link_output == {default}} {
        set vpp_link_output xclbin
        # If we target versal board with vitis >= 2022.1, then the vitis linker output is XSA and use vitis packager to get the xclbin
        if {([dict get $config tool_version] >= 2022.1) && ([regexp {versal} [dict_get_quiet $config {wizard_actual_config platform fpga_family}]] == 1)} {
            set vpp_link_output xsa
        }
    }

    # Vitis Compiler Configuration File documentation (2021.1)
    # Switches are read in the order they are encountered.
    # If the same switch is repeated with conflicting information, the first switch read is used.
    # The order of precedence for switches is as follows, where item one takes highest precedence:
    #    1) Command line switches.
    #    2) Config files (on command line) from left-to-right.
    #    3) Within a config file, precedence is from top-to-bottom.

    set vpp_link_out    [file join [dict get $config run_dir] vpp_link.$vpp_link_output]
    set sh_vpp_link_out "\${XBTEST_RUN_DIR}/vpp_link.$vpp_link_output"

    lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} {Executing Vitis linker}]
    lappend BUILD_XCLBIN_SH "v++ \\"
    lappend BUILD_XCLBIN_SH "--platform             \${XPFM} \\"; # Specify path to a platform specification(XPFM) or hardware specification(XSA)
    lappend BUILD_XCLBIN_SH "--user_ip_repo_paths   \${XBTEST_IP_CATALOG} \\"; # path to xbtest_sub_xxv_gt
    set vpp_options_dir [dict get $config wizard_actual_config build vpp_options_dir]; # Vitis ini options directory
    if {$vpp_options_dir != {}} {
        set vpp_ini [file join $vpp_options_dir vpp.ini]
        log_message $config {GEN_BUILD_XCLBIN_SH-5} [list $vpp_ini]
        check_vpp_ini $config $vpp_ini
        lappend BUILD_XCLBIN_SH "--config               $vpp_ini \\"
    }
    # give the lowest priority to option present in this file, so user vpp can be used to overwrite anything
    lappend BUILD_XCLBIN_SH "--config               \${XBTEST_RUN_DIR}/vpp_link.ini \\"
    if {$use_aie} {
        lappend BUILD_XCLBIN_SH "$sh_vpp_aie_out \\"
    }
    set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH $KERNEL_XO]
    lappend BUILD_XCLBIN_SH ""; # end of command
    set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_ret_code_bash   {Vitis linker}]]

    if {![dict get $config STOP_POST_OPT]} {
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_file_bash       $sh_vpp_link_out]]
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]
    } else {
        if {[dict get $config C_INIT]} {
            lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "No xclbin generated as Vitis was stopped at step: vpl.impl.opt_design. A DCP should be available for power CU floorplan inputs"]
            lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "Initialization completed"]
            lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "Done!"]
            lappend BUILD_XCLBIN_SH "exit 0"; # Return 0 if success
        } else {
            lappend BUILD_XCLBIN_SH [log_message_bash {ERROR} {XBTEST_WIZARD-27} "No xclbin generated as Vitis was stopped at step: vpl.impl.opt_design. A DCP should be available for power CU floorplan inputs"]
            lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "Done!"]
            lappend BUILD_XCLBIN_SH "exit 1"; # Return 1 if error
        }
    }

    ## Package for AIE
    if {$use_aie || ($vpp_link_output == {xsa})} {
        set tmp_xclbin      [file join [dict get $config run_dir] vpp_package.xclbin]
        set sh_tmp_xclbin   "\${XBTEST_RUN_DIR}/vpp_package.xclbin"
    } else {
        set tmp_xclbin       $vpp_link_out
        set sh_tmp_xclbin    $sh_vpp_link_out
    }

    if {![dict get $config STOP_POST_OPT]} {
        ## Package for AIE
        if {$use_aie || ($vpp_link_output == {xsa})} {
            lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} {Executing Vitis packager}]
            lappend BUILD_XCLBIN_SH "v++ \\"
            lappend BUILD_XCLBIN_SH "--platform \${XPFM} \\"; # Specify path to a platform specification(XPFM) or hardware specification(XSA)
            lappend BUILD_XCLBIN_SH "--config   \${XBTEST_RUN_DIR}/vpp_package.ini \\"
            if {$use_aie} {
                lappend BUILD_XCLBIN_SH "$sh_vpp_aie_out \\"
            }
            lappend BUILD_XCLBIN_SH "$sh_vpp_link_out \n"; # end of command
            set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_ret_code_bash   {Vitis packager}]]
            set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_file_bash       $sh_tmp_xclbin]]
            set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [remove_temp_file_bash $sh_vpp_link_out]]
            set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]
        }

        ## Set USER_METADATA command
        set output_xclbin [file join [dict get $config output_dir] xbtest_stress.xclbin]
        set sh_output_xclbin "\${XBTEST_OUTPUT_DIR}/xbtest_stress.xclbin"
        log_message $config {GEN_BUILD_XCLBIN_SH-8} [list $output_xclbin]
        lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} {Adding USER_METADATA section to xclbin}]
        lappend BUILD_XCLBIN_SH "xclbinutil \\"
        lappend BUILD_XCLBIN_SH "--add-section  USER_METADATA:RAW:\${XBTEST_RUN_DIR}/user_metadata.json \\"
        lappend BUILD_XCLBIN_SH "--input        $sh_tmp_xclbin \\"
        lappend BUILD_XCLBIN_SH "--output       $sh_output_xclbin"
        lappend BUILD_XCLBIN_SH ""; # end of command
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_ret_code_bash   xclbinutil]]
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [check_file_bash       $sh_output_xclbin]]
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [remove_temp_file_bash $sh_tmp_xclbin]]
        set     BUILD_XCLBIN_SH [concat $BUILD_XCLBIN_SH [get_bash_delimiter]]

        ## End of build_xclbin.sh
        lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "xclbin successfully generated: $sh_output_xclbin"]
        lappend BUILD_XCLBIN_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "Done!"]
        lappend BUILD_XCLBIN_SH "exit 0"; # Return 0 if success
    }

    log_message $config {GEN_BUILD_XCLBIN_SH-10} [list $build_xclbin_sh]
    write_file $build_xclbin_sh [join $BUILD_XCLBIN_SH "\n"]
    file attribute $build_xclbin_sh -permissions {a+x}; # Add executable permission to everyone (raptor)

    ## Set INI file for Vitis AIE compiler
    if {$use_aie} {
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "Create vpp_aie.ini"]
        set     VPP_AIE_INI {}
        lappend VPP_AIE_INI "target=hw"
        lappend VPP_AIE_INI ""
        # V++ other options
        lappend VPP_AIE_INI "log_dir=vpp_aie/logs"
        lappend VPP_AIE_INI "report_dir=vpp_aie/reports"
        lappend VPP_AIE_INI ""

        # All the include files needed to build the graph
        lappend VPP_AIE_INI "include=${XBTEST_AIE}"
        lappend VPP_AIE_INI "include=${XBTEST_AIE}/kernels"
        lappend VPP_AIE_INI "include=${XBTEST_AIE}/data"
        lappend VPP_AIE_INI ""

        # V++ AIE options
        lappend VPP_AIE_INI "\[aie\]"
        lappend VPP_AIE_INI "workdir=vpp_aie";                                      # The location of where the Work directory will be created
        lappend VPP_AIE_INI "constraints=$aie_constraints_json";                    # Specify constraints files in JSON format (zero or more)
        lappend VPP_AIE_INI ""

        lappend VPP_AIE_INI "pl-freq=$clock0_freq"
        lappend VPP_AIE_INI "heapsize=16000"
        lappend VPP_AIE_INI "stacksize=16000"
        lappend VPP_AIE_INI "enable-ecc-scrubbing=true"
        lappend VPP_AIE_INI "xlopt=0"
        lappend VPP_AIE_INI ""

        lappend VPP_AIE_INI "#verbose=true"
        lappend VPP_AIE_INI "kernel-linting=true"
        lappend VPP_AIE_INI "no-init=true"
        lappend VPP_AIE_INI "nodot-graph=true"
        lappend VPP_AIE_INI ""

        lappend VPP_AIE_INI "Xpreproc=-DPL_FREQ_MHZ=$clock0_freq";                  # The PL freq to be passed to PLIO constructor
        lappend VPP_AIE_INI "Xpreproc=-DAIE_FREQ_MHZ=$aie_freq";                    # AIE frequency used for AIE control by PS; TODO can this be obtained based on --platform?
        if {$aie_control == {ps}} {
        lappend VPP_AIE_INI "Xpreproc=-DAIE_CONTROL=0"
        } elseif {$aie_control == {pl}} {
        lappend VPP_AIE_INI "Xpreproc=-DAIE_CONTROL=1"
        }
        if {$aie_status == {none}} {
        lappend VPP_AIE_INI "Xpreproc=-DAIE_STATUS=0"
        } elseif {$aie_status == {pl}} {
        lappend VPP_AIE_INI "Xpreproc=-DAIE_STATUS=1"
        }
        lappend VPP_AIE_INI "Xpreproc=-DNUMCORES=$num_aie_used";                    # # The number of kernel to be generated in graph
        lappend VPP_AIE_INI ""

        set vpp_aie_ini [file join [dict get $config run_dir] vpp_aie.ini]
        log_message $config {GEN_BUILD_XCLBIN_SH-10} [list $vpp_aie_ini]
        write_file $vpp_aie_ini [join $VPP_AIE_INI "\n"]
    }

    ## Set INI file for Vitis link
    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "Create vpp_link.ini"]
    set     VPP_LINK_INI {}
    lappend VPP_LINK_INI "link=1"
    if {[dict get $config STOP_POST_OPT]} {
        lappend VPP_LINK_INI "to_step=vpl.impl.opt_design"
    }
    lappend VPP_LINK_INI "output=$vpp_link_out"
    lappend VPP_LINK_INI "target=hw"
    lappend VPP_LINK_INI "kernel_frequency=0:${clock0_freq}|1:${clock1_freq}"
    lappend VPP_LINK_INI ""
    # V++ other options
    lappend VPP_LINK_INI "temp_dir=vpp_link"
    lappend VPP_LINK_INI "log_dir=vpp_link/logs"
    lappend VPP_LINK_INI "report_dir=vpp_link/reports"
    lappend VPP_LINK_INI "messageDb=vpp_link.mdb"
    lappend VPP_LINK_INI "save-temps=1"
    lappend VPP_LINK_INI "report_level=2"; # The -R 2 specification turns on everything -R1 does, plus it adds report_failfast post-route_design.
    lappend VPP_LINK_INI ""
    # V++ connectivity
    lappend VPP_LINK_INI "\[connectivity\]"
    set     VPP_LINK_INI [concat $VPP_LINK_INI $NK_OPTION]
    set     VPP_LINK_INI [concat $VPP_LINK_INI $SLR_OPTION]
    set     VPP_LINK_INI [concat $VPP_LINK_INI $SP_OPTION]
    set     VPP_LINK_INI [concat $VPP_LINK_INI $SC_OPTION]
    lappend VPP_LINK_INI ""

    # V++ advanced
    lappend VPP_LINK_INI "\[advanced\]"
    lappend VPP_LINK_INI "misc=solution_name=link"
    lappend VPP_LINK_INI "param=compiler.enableAutoFrequencyScaling=false"; # disable auto kernel clk freq scaling, note user can still use compiler.skipTimingCheckAndFrequencyScaling=true
    lappend VPP_LINK_INI "param=compiler.addOutputTypes=hw_export"; # TODO this might not be needed for our use case. It specifies the creation of a new XSA for the target platform. The exported XSA has the name of the output file specified by the -o option, with the file extension of .xsa.
    lappend VPP_LINK_INI ""

    set vpp_link_ini [file join [dict get $config run_dir] vpp_link.ini]
    log_message $config {GEN_BUILD_XCLBIN_SH-10} [list $vpp_link_ini]
    write_file $vpp_link_ini [join $VPP_LINK_INI "\n"]

    ## Set INI file for Vitis package
    if {![dict get $config STOP_POST_OPT]} {
        if {$use_aie || ($vpp_link_output == {xsa})} {
            log_message $config {GEN_BUILD_XCLBIN_SH-12} [list {Create vpp_package.ini}]
            set     VPP_PACKAGE_INI {}
            lappend VPP_PACKAGE_INI "package=1"
            lappend VPP_PACKAGE_INI "output=$tmp_xclbin"
            lappend VPP_PACKAGE_INI "target=hw"
            lappend VPP_PACKAGE_INI ""
            # V++ other options
            lappend VPP_PACKAGE_INI "temp_dir=vpp_package"
            lappend VPP_PACKAGE_INI "log_dir=vpp_package/logs"
            lappend VPP_PACKAGE_INI "report_dir=vpp_package/reports"
            lappend VPP_PACKAGE_INI "messageDb=vpp_package.mdb"
            lappend VPP_PACKAGE_INI "save-temps=1"
            lappend VPP_PACKAGE_INI "report_level=2"; # The -R 2 specification turns on everything -R1 does, plus it adds report_failfast post-route_design.
            lappend VPP_PACKAGE_INI ""
            # V++ package
            lappend VPP_PACKAGE_INI "\[package\]"
            lappend VPP_PACKAGE_INI "boot_mode=ospi"; # For Versal datacentre
            # Do not use "defer_aie_run=1"
            # https://www.xilinx.com/html_docs/xilinx2020_2/vitis_doc/vitiscommandcompiler.html?hl=defer_aie#bng1586024970608__section_edk_sr3_glb
            #       => Where this option specifies that the Versal AI Engine cores will be enabled by an embedded processor (PS) application.
            #          When not specified, the tool will generate CDO commands to enable the AI Engine cores during PDI load instead
            lappend VPP_PACKAGE_INI ""

            # V++ advanced
            lappend VPP_PACKAGE_INI "\[advanced\]"
            lappend VPP_PACKAGE_INI "misc=solution_name=package"
            lappend VPP_PACKAGE_INI ""

            set vpp_package_ini [file join [dict get $config run_dir] vpp_package.ini]
            log_message $config {GEN_BUILD_XCLBIN_SH-10} [list $vpp_package_ini]
            write_file $vpp_package_ini [join $VPP_PACKAGE_INI "\n"]
        }
    }

    ## Write user_metadata.json
    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "Create user_metadata.json"]

    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    # json::dict2json seems to be broken, need to defined manually in the json format
    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-Board info"]
    set     USER_METADATA_JSON {}
    lappend USER_METADATA_JSON "\{"
    lappend USER_METADATA_JSON "$TAB_1[string2json {build_info}]: \{"

    lappend USER_METADATA_JSON      "$TAB_2[string2json {xbtest}]: \{"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {version}]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {major}]: [dict get $config ip_version_major],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {minor}]: [dict get $config ip_version_minor]"
    lappend USER_METADATA_JSON          "$TAB_3\},"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {build}]: [dict get $config C_BUILD_VERSION],"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {date}]: [string2json [dict get $config build_date]],"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {internal_release}]: [dict get $config C_INTERNAL_RELEASE]"
    lappend USER_METADATA_JSON      "$TAB_2\},"

    lappend USER_METADATA_JSON      "$TAB_2[string2json {board}]: \{"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {name}]: [string2json [dict get $config wizard_actual_config platform name]],"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {interface_uuid}]: [string2json [dict get $config wizard_actual_config platform interface_uuid]]"
    lappend USER_METADATA_JSON      "$TAB_2\},"
    lappend USER_METADATA_JSON      "$TAB_2[string2json {tool}]: \{"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {version}]: [dict get $config tool_version],"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {version_long}]: [string2json [dict get $config tool_version_long]]"
    lappend USER_METADATA_JSON      "$TAB_2\}"

    lappend USER_METADATA_JSON "$TAB_1\},"

    lappend USER_METADATA_JSON "$TAB_1[string2json {definition}]: \{"

    if {$use_aie} {
    lappend USER_METADATA_JSON      "$TAB_2[string2json {aie}]: \{"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {freq}]: $aie_freq,"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {type}]: [string2json $aie_type],"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {control}]: [string2json $aie_control],"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {status}]: [string2json $aie_status]"
    lappend USER_METADATA_JSON      "$TAB_2\},"
    }
    lappend USER_METADATA_JSON      "$TAB_2[string2json {clocks}]: \{"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {0}]: $clock0_freq,"
    lappend USER_METADATA_JSON          "$TAB_3[string2json {1}]: $clock1_freq"
    lappend USER_METADATA_JSON      "$TAB_2\},"

    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-Memory definition"]
    if {[dict exists $config memory_definition]} {
    lappend USER_METADATA_JSON      "$TAB_2[string2json {memory}]: \{"
        set memory_definition_list [dict get $config memory_definition]
        set ii_max [expr [llength $memory_definition_list] - 1]
        for {set ii 0} {$ii <= $ii_max} {incr ii} {
            set def [lindex $memory_definition_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3\"$ii\": \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {name}]: [string2json [dict get $def memory_name]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {target}]: [string2json [dict get $def target]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {axi_data_size}]: [dict get $def axi_data_size],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {num_axi_thread}]: [dict get $def axi_id_threads],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {num_axi_outstanding}]: [dict get $def axi_outstanding]"; # last param
            if {$ii < $ii_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
        }
    lappend USER_METADATA_JSON      "$TAB_2\},"
    }
    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-GT_MAC definition"]
    if {[dict exists $config gt_mac_definition] || [dict exists $config gt_lpbk_definition] || [dict exists $config gt_prbs_definition] || [dict exists $config gtf_prbs_definition] || [dict exists $config gtm_prbs_definition] || [dict exists $config gtyp_prbs_definition]} {
                lappend USER_METADATA_JSON      "$TAB_2[string2json {gt}]: \{"
        set gt_mac_def_list     [dict_get_quiet $config gt_mac_definition]
        set gt_lpbk_def_list    [dict_get_quiet $config gt_lpbk_definition]
        set gt_prbs_def_list    [dict_get_quiet $config gt_prbs_definition]
        set gtf_prbs_def_list   [dict_get_quiet $config gtf_prbs_definition]
        set gtm_prbs_def_list   [dict_get_quiet $config gtm_prbs_definition]
        set gtyp_prbs_def_list  [dict_get_quiet $config gtyp_prbs_definition]
        set gt_idx              0
        set gt_idx_max          [expr [llength $gt_mac_def_list] + [llength $gt_lpbk_def_list] + [llength $gt_prbs_def_list] + [llength $gtf_prbs_def_list] + [llength $gtm_prbs_def_list] + [llength $gtyp_prbs_def_list] - 1]
        if {[dict exists $config gt_mac_definition]} {
            for {set ii 0} {$ii < [llength $gt_mac_def_list]} {incr ii} {
                log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-GT\[$ii\]"]

                set def [lindex $gt_mac_def_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $gt_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_index}]: [dict get $def gt_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_mac_ip_sel}]: [string2json [dict get $def ip_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_mac_enable_rsfec}]: [dict get $def enable_rsfec],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_group_select}]: [string2json [dict get $def group_select]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_serial_port}]: [string2json [dict get $def serial_port]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_diff_clocks}]: [string2json [dict get $def diff_clocks]]"; # last param
                if {$gt_idx < $gt_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
                incr gt_idx
            }
        }
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-GT_LPBK definition"]
        if {[dict exists $config gt_lpbk_definition]} {
            set gt_lpbk_def_list [dict get $config gt_lpbk_definition]
            for {set ii 0} {$ii < [llength $gt_lpbk_def_list]} {incr ii} {
                log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-GT\[$ii\]"]
                set def [lindex $gt_lpbk_def_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $gt_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_index}]: [dict get $def gt_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_group_select}]: [string2json [dict get $def group_select]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_serial_port}]: [string2json [dict get $def serial_port]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_diff_clocks}]: [string2json [dict get $def diff_clocks]]"; # last param
                if {$gt_idx < $gt_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
                incr gt_idx
            }
        }
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-GT_PRBS definition"]
        if {[dict exists $config gt_prbs_definition]} {
            set gt_prbs_def_list [dict get $config gt_prbs_definition]
            for {set ii 0} {$ii < [llength $gt_prbs_def_list]} {incr ii} {
                log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-GT\[$ii\]"]
                set def [lindex $gt_prbs_def_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $gt_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_index}]: [dict get $def gt_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_prbs_ip_sel}]: [string2json [dict get $def ip_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_group_select}]: [string2json [dict get $def group_select]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_serial_port}]: [string2json [dict get $def serial_port]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_diff_clocks}]: [string2json [dict get $def diff_clocks]]"; # last param
                if {$gt_idx < $gt_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
                incr gt_idx
            }
        }
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-GTF_PRBS definition"]
        if {[dict exists $config gtf_prbs_definition]} {
            set gtf_prbs_def_list [dict get $config gtf_prbs_definition]
            for {set ii 0} {$ii < [llength $gtf_prbs_def_list]} {incr ii} {
                log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-GT\[$ii\]"]
                set def [lindex $gtf_prbs_def_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $gt_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_index}]: [dict get $def gt_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gtf_prbs_ip_sel}]: [string2json [dict get $def ip_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_group_select}]: [string2json [dict get $def group_select]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_refclk_sel}]: [string2json [dict get $def refclk_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_serial_port}]: [string2json [dict get $def serial_port]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_diff_clocks}]: [string2json [dict get $def diff_clocks]]"; # last param
                if {$gt_idx < $gt_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
                incr gt_idx
            }
        }
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-GTM_PRBS definition"]
        if {[dict exists $config gtm_prbs_definition]} {
            set gtm_prbs_def_list [dict get $config gtm_prbs_definition]
            for {set ii 0} {$ii < [llength $gtm_prbs_def_list]} {incr ii} {
                log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-GT\[$ii\]"]
                set def [lindex $gtm_prbs_def_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $gt_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_index}]: [dict get $def gt_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gtm_prbs_ip_sel}]: [string2json [dict get $def ip_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_group_select}]: [string2json [dict get $def group_select]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_refclk_sel}]: [string2json [dict get $def refclk_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_serial_port}]: [string2json [dict get $def serial_port]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_diff_clocks}]: [string2json [dict get $def diff_clocks]]"; # last param
                if {$gt_idx < $gt_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
                incr gt_idx
            }
        }
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-GTYP_PRBS definition"]
        if {[dict exists $config gtyp_prbs_definition]} {
            set gtyp_prbs_def_list [dict get $config gtyp_prbs_definition]
            for {set ii 0} {$ii < [llength $gtyp_prbs_def_list]} {incr ii} {
                log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-GT\[$ii\]"]
                set def [lindex $gtyp_prbs_def_list $ii]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $gt_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_index}]: [dict get $def gt_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_type}]: [string2json [dict get $def type]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gtyp_prbs_ip_sel}]: [string2json [dict get $def ip_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_group_select}]: [string2json [dict get $def group_select]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_refclk_sel}]: [string2json [dict get $def refclk_sel]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_serial_port}]: [string2json [dict get $def serial_port]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {gt_diff_clocks}]: [string2json [dict get $def diff_clocks]]"; # last param
                if {$gt_idx < $gt_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
                incr gt_idx
            }
        }
    lappend USER_METADATA_JSON      "$TAB_2\},"
    }

    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-Compute units definition"]
    lappend USER_METADATA_JSON      "$TAB_2[string2json {compute_units}]: \{"
    set cu_cfg_list [dict get $config cu_config]
    set cu_idx_max  [expr [llength $cu_cfg_list] - 1]
    for {set cu_idx 0} {$cu_idx <= $cu_idx_max} {incr cu_idx} {
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-CU\[$cu_idx\]"]
        set cu_cfg      [lindex $cu_cfg_list $cu_idx]
        set krnl_mode   [dict get $cu_cfg krnl_mode]
    lappend USER_METADATA_JSON          "$TAB_3[string2json $cu_idx]: \{"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {name}]: [string2json [dict get $cu_cfg kernel_name]],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {SLR}]: [dict get $cu_cfg slr_idx],"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {mode}]: $krnl_mode,"
        # Connectivity

        # Find number of axi ports
        set connection_indexes {}
        foreach key [dict keys $cu_cfg sp_m*_axi] {
            regexp {sp_m0*([0-9]+)_axi} $key -> connection_idx
            lappend connection_indexes $connection_idx
        }
        set num_connections [llength $connection_indexes]
    lappend USER_METADATA_JSON              "$TAB_4[string2json {num_connections}]: $num_connections,"
    lappend USER_METADATA_JSON              "$TAB_4[string2json {connectivity}]: \{"
        set ii_max [expr $num_connections - 1]
        for {set ii 0} {$ii <= $ii_max} {incr ii} {
            set sptag [dict get $cu_cfg [format {sp_m%02d_axi} [lindex $connection_indexes $ii]]]
            if {$ii < $ii_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON                  "$TAB_5[string2json $ii]: [string2json $sptag]$next"
        }
    lappend USER_METADATA_JSON              "$TAB_4\},"

        # CU configuration specific for CU type
    lappend USER_METADATA_JSON              "$TAB_4[string2json {cu_type_configuration}]: \{"
        if {$krnl_mode == $C_KRNL_MODE_VERIFY} {
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {dna_read}]: [dict get $cu_cfg dna_read]"
        } elseif {$krnl_mode == $C_KRNL_MODE_POWER} {
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {SLR}]: [dict get $cu_cfg slr_idx],"
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {throttle_mode}]: [string2json [dict get $cu_cfg throttle_mode]],"
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {use_aie}]: [dict get $cu_cfg use_aie]"
        } elseif {$krnl_mode == $C_KRNL_MODE_MEMORY} {
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {memory_type}]: [string2json [dict get $cu_cfg memory_name]]"
        } elseif {($krnl_mode == $C_KRNL_MODE_GT_LPBK) || ($krnl_mode == $C_KRNL_MODE_GT_MAC) || ($krnl_mode == $C_KRNL_MODE_GT_PRBS)} {
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {gt_index}]: [dict get $cu_cfg gt_idx]"
        } elseif {($krnl_mode == $C_KRNL_MODE_GTF_PRBS) || ($krnl_mode == $C_KRNL_MODE_GTM_PRBS) || ($krnl_mode == $C_KRNL_MODE_GTYP_PRBS)} {
    lappend USER_METADATA_JSON                  "$TAB_5[string2json {gt_index}]: \{";
            set jj      0
            set jj_max [expr [llength [dict keys [dict get $cu_cfg gt_cfg]]] - 1]
            foreach {gt_idx cfg} [dict get $cu_cfg gt_cfg] {
                set enable       [dict get $cfg enable]
                if {$jj < $jj_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON                      "$TAB_6[string2json $gt_idx]: $enable$next"
                incr jj
            }
    lappend USER_METADATA_JSON                  "$TAB_5\}"; # last param
        }
    lappend USER_METADATA_JSON              "$TAB_4\}"; # last param
        if {$cu_idx < $cu_idx_max} { set next "," } else { set next "" }
    lappend USER_METADATA_JSON          "$TAB_3\}$next"
    }
    lappend USER_METADATA_JSON      "$TAB_2\}"
    lappend USER_METADATA_JSON "$TAB_1\},"

    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-Pre-canned test JSON"]
    lappend USER_METADATA_JSON "$TAB_1[string2json {pre_canned}]: \{"
    set ii      0
    set ii_max [expr [llength [dict keys [dict get $config pre_canned]]] - 1]
    foreach {test TEST_JSON} [dict get $config pre_canned] {
        log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t\t-test: $test"]
        set jj      0
        set jj_max [expr [llength $TEST_JSON] - 1]
        foreach line $TEST_JSON {
            if {$ii < $ii_max} { set next "," } else { set next "" }
            if {$jj < $jj_max} { set next ""  } else { set next $next }
            if {$jj == 0} {
    lappend USER_METADATA_JSON      "$TAB_2[string2json $test]: $line"
            } else {
    lappend USER_METADATA_JSON      "$TAB_2$line$next"
            }
            incr jj
        }
        incr ii
    }
    lappend USER_METADATA_JSON "$TAB_1\},"

    log_message $config {GEN_BUILD_XCLBIN_SH-12} [list "\t-Platform definition JSON"]
        set jj      0
        set jj_max [expr [llength [dict get $config xbtest_pfm_def]] - 1]
        foreach line [dict get $config xbtest_pfm_def] {
            if {$jj == 0} {
    lappend USER_METADATA_JSON "$TAB_1[string2json {xbtest_pfm_def}]: $line"
            } else {
    lappend USER_METADATA_JSON "$TAB_1$line"
            }
            incr jj
        }

    lappend USER_METADATA_JSON "\}"

    set user_metadata_json [file join [dict get $config run_dir] user_metadata.json]
    log_message $config {GEN_BUILD_XCLBIN_SH-10} [list $user_metadata_json]
    write_file $user_metadata_json [join $USER_METADATA_JSON "\n"]
}
