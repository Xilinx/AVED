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

# Note to add/remove a parameter definition of wizard configuration JSON file, or update its behaviour, you must:
# 1) Define an autoconfigure_ procedure in param_autoconfig.tcl: platform info or simply default value
# 2) Define a merge_ procedure in param_merge.tcl: this will define the behaviour of merge between auto-configured and user provided values
# 3) Define a validate_ procedure in param_validate.tcl: check the actual (merged) value against possible values
# 4) Add your parameter to gen_wizard_config.tcl: see procedures get_wizard_config_json_def, fn_wizard_configure, log_wizard_config

############################################################################################################
# Top function to generate and validate the xbtest wizard configuration
############################################################################################################
proc gen_wizard_config { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    dict set config build_date [exec date]
    dict set config tool_version_long [join [split [version -verbose] "\n"] ". "]
    dict set config tool_version [version_short_to_num [version -short]]

    # Get IP user parameter values
    load_user_parameters config

    # Get parameters available on the IPDEF
    load_ipdef_parameters config
    log_config config [list platforminfo Component_Name MESSAGES_DICT C_DYNAMIC_GEOMETRY] [file join [dict get $config run_dir] base_config.log]

    # Extract platform metadata
    load_platforminfo config

    # Get wizard config provided by user
    load_wizard_config_json config

    # Auto-configure parameters. Log the auto-configured values, and Remove invalid auto-configured values
    fn_wizard_configure config autoconfigure    wizard_auto_config
    log_wizard_config   $config                 wizard_auto_config
    fn_wizard_configure config validate         wizard_auto_config

    if {[dict get $config C_INIT]} {
        # Create template directories
        dict set config cfg_template_dir        [file join [dict get $config run_dir]           cfg_template];  file mkdir [dict get $config cfg_template_dir]
        dict set config pwr_cfg_template_dir    [file join [dict get $config cfg_template_dir]  pwr_cfg];       file mkdir [dict get $config pwr_cfg_template_dir]
        dict set config vpp_cfg_template_dir    [file join [dict get $config cfg_template_dir]  vpp_cfg];       file mkdir [dict get $config vpp_cfg_template_dir]

        # Set user wizard configuration for init mode
        set_wizard_config_init  config
        # Generate wizard configuration JSON file template
        gen_wizard_cfg_template config
        # Generate the Vitis configuration templates
        gen_vpp_cfg_template    config
    }

    # Merge valid auto-configured values with value provided by user. Log actual (merged) configuration and Validate the actual values
    fn_wizard_configure config merge        wizard_actual_config
    log_wizard_config   $config             wizard_actual_config
    fn_wizard_configure config validate     wizard_actual_config
    log_wizard_config   $config             wizard_actual_config; # log a 2nd time after validate


    set req_param_list {}
    lappend req_param_list {wizard_actual_config platform name}
    lappend req_param_list {wizard_actual_config platform interface_uuid}
    foreach param_path $req_param_list {
        set param_name      [join $param_path {.}]
        set param_value     [dict_get_quiet $config $param_path]
        if {$param_value == {NOT DEFINED}} {
            log_message $config {XBTEST_WIZARD-38} [list $param_name]; # WARNING: value not found in platform info.
        }
    }

    # Load the power floorplan JSON files
    load_power_floorplan_json   config
    # Configure the verify compute unit
    setup_verify_cu             config
}
############################################################################################################
# Support when vivado version is not numerical value
############################################################################################################
proc version_short_to_num { ver } {
    set res ""
    foreach c [split $ver ""] {
        if {[regexp -all {[0-9.]} $c]} {
            append res $c
        }
    }
    if {$res == {}} {
        set res "0"
    }
    if {[lindex $res 0] == "."} {
        set res "0"
    }
    set res_split [split $res "."]
    if {[llength $res_split] == 1} {
        append res ".0"
    } elseif {[llength $res_split] > 2} {
        set res_split    [lrange $res_split 0 1]
        set res         [join $res_split "."]
    }
    return $res
}
############################################################################################################
# Set user wizard configuration for init mode
############################################################################################################
proc set_wizard_config_init { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    # Create a configuration without selecting any CU
    # The verify CU is always inserted, configure it without DNA

    set wizard_config_name {init}
    dict set config wizard_config_name  $wizard_config_name

    dict set config wizard_config $wizard_config_name [dict create \
        cu_configuration [dict create \
            verify [dict create \
                dna_read false \
            ] \
        ] \
        cu_selection [dict create] \
    ]
}
############################################################################################################
# Generate a template of wizard configuration JSON file
############################################################################################################
proc gen_wizard_cfg_template { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set auto_cu_configuration_gt_mac    [dict_get_quiet $config {wizard_auto_config cu_configuration gt_mac}]
    set auto_cu_configuration_gt_prbs   [dict_get_quiet $config {wizard_auto_config cu_configuration gt_prbs}]
    set auto_cu_configuration_gtf_prbs  [dict_get_quiet $config {wizard_auto_config cu_configuration gtf_prbs}]
    set auto_cu_configuration_gtm_prbs  [dict_get_quiet $config {wizard_auto_config cu_configuration gtm_prbs}]
    set auto_cu_configuration_gtyp_prbs [dict_get_quiet $config {wizard_auto_config cu_configuration gtyp_prbs}]
    set auto_cu_configuration_memory    [dict_get_quiet $config {wizard_auto_config cu_configuration memory}]
    set auto_cu_configuration_power     [dict_get_quiet $config {wizard_auto_config cu_configuration power}]

    set gt_mac      {}
    set gt_prbs     {}
    set gtf_prbs    {}
    set gtm_prbs    {}
    set gtyp_prbs   {}
    set memory      {}
    set power       {}

    if {$auto_cu_configuration_gt_mac != {}} {
        set gt_mac [lsort -dictionary [dict keys $auto_cu_configuration_gt_mac]]
    }
    if {$auto_cu_configuration_gt_prbs != {}} {
        set gt_prbs [lsort -dictionary [dict keys $auto_cu_configuration_gt_prbs]]
    }
    if {$auto_cu_configuration_gtf_prbs != {}} {
        set gtf_prbs [lsort -dictionary [dict keys $auto_cu_configuration_gtf_prbs]]
    }
    if {$auto_cu_configuration_gtm_prbs != {}} {
        set gtm_prbs [lsort -dictionary [dict keys $auto_cu_configuration_gtm_prbs]]
    }
    if {$auto_cu_configuration_gtyp_prbs != {}} {
        set gtyp_prbs [lsort -dictionary [dict keys $auto_cu_configuration_gtyp_prbs]]
    }
    if {$auto_cu_configuration_memory != {}} {
        set memory [lsort -dictionary [dict keys $auto_cu_configuration_memory]]
    }
    if {$auto_cu_configuration_power != {}} {
        set power  [lsort -dictionary [dict keys $auto_cu_configuration_power]]
    }
    set wizard_config_name {xbtest_stress}
    set config_template [dict create \
        default [dict create \
            build [dict create \
                pwr_floorplan_dir {../pwr_cfg} \
                vpp_options_dir   {../vpp_cfg} \
            ] \
        ] \
        xbtest_stress [dict create \
            cu_selection [dict create \
                gt_mac      $gt_mac \
                gt_prbs     $gt_prbs \
                gtf_prbs    $gtf_prbs \
                gtm_prbs    $gtm_prbs \
                gtyp_prbs   $gtyp_prbs \
                memory      $memory \
                power       $power \
            ] \
        ] \
        xbtest_power_fp [dict create \
            build [dict create \
                display_pwr_floorplan true \
            ] \
            cu_selection [dict create \
                power   $power \
            ] \
        ] \
    ]
    set wizard_cfg_json [file join [dict get $config cfg_template_dir] wizard_cfg.json]
    log_wizard_config $config_template {} [dict keys $config_template] $wizard_cfg_json
    log_message $config {XBTEST_WIZARD-40} [list $wizard_cfg_json]
}
############################################################################################################
# Generate the vpp_cfg templates
############################################################################################################
proc gen_vpp_cfg_template { config_ref } {
    # Dictionary passed as ref.
    upvar 1 $config_ref config

    set     POSTSYS_LINK_TCL {}
    lappend POSTSYS_LINK_TCL "#########################################################################################"
    lappend POSTSYS_LINK_TCL "# Post system linker TCL hook"
    lappend POSTSYS_LINK_TCL "# This is a generated template file"
    lappend POSTSYS_LINK_TCL "# How to use. In procedure postsys_link_body:"
    lappend POSTSYS_LINK_TCL "#    1) Update continuous clocks connectivity: UCS name and version"
    lappend POSTSYS_LINK_TCL "#          => Select one call of procedure connect_continuous_clocks"
    lappend POSTSYS_LINK_TCL "#########################################################################################"
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL "###################################### Update here ######################################"
    lappend POSTSYS_LINK_TCL "proc postsys_link_body \{\} \{"
    lappend POSTSYS_LINK_TCL "  #### Continuous clocks connectivity"
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL "  # connect_continuous_clocks ulp_ucs 0; # No UCS present, continuous clock is not supported"
    lappend POSTSYS_LINK_TCL "  # connect_continuous_clocks ulp_ucs 2; # UCS subsystem version v2"
    lappend POSTSYS_LINK_TCL "  connect_continuous_clocks ulp_ucs 3; # UCS subsystem version v3"
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL "\}"
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL ""
    lappend POSTSYS_LINK_TCL "##################################### DO NOT EDIT #######################################"
    lappend POSTSYS_LINK_TCL "proc connect_continuous_clocks \{ ucs_name ucs_version\} \{"
    lappend POSTSYS_LINK_TCL "  # Set names depending on UCS version"
    lappend POSTSYS_LINK_TCL "  if \{\$ucs_version == 0\} \{"
    lappend POSTSYS_LINK_TCL "    connect_bd_net  \[get_bd_pins krnl_*/ap_clk\]     \[get_bd_pins krnl_*/ap_clk_cont\]"
    lappend POSTSYS_LINK_TCL "    connect_bd_net  \[get_bd_pins krnl_*/ap_clk_2\]   \[get_bd_pins krnl_*/ap_clk_2_cont\]"
    lappend POSTSYS_LINK_TCL "    return"
    lappend POSTSYS_LINK_TCL "  \} elseif \{\$ucs_version == 2\} \{"
    lappend POSTSYS_LINK_TCL "    set clk_prop_name_0   ENABLE_KERNEL_CONT_CLOCK"
    lappend POSTSYS_LINK_TCL "    set clk_prop_val_0    true"
    lappend POSTSYS_LINK_TCL "    set clk_port_0        clk_kernel_cont"
    lappend POSTSYS_LINK_TCL "    set clk_prop_name_1   ENABLE_KERNEL2_CONT_CLOCK"
    lappend POSTSYS_LINK_TCL "    set clk_prop_val_1    true"
    lappend POSTSYS_LINK_TCL "    set clk_port_1        clk_kernel2_cont"
    lappend POSTSYS_LINK_TCL "  \} elseif \{\$ucs_version == 3\} \{"
    lappend POSTSYS_LINK_TCL "    set clk_prop_name_0   ENABLE_CONT_KERNEL_CLOCK_00"
    lappend POSTSYS_LINK_TCL "    set clk_prop_val_0    true"
    lappend POSTSYS_LINK_TCL "    set clk_port_0        aclk_kernel_00_cont"
    lappend POSTSYS_LINK_TCL "    set clk_prop_name_1   ENABLE_CONT_KERNEL_CLOCK_01"
    lappend POSTSYS_LINK_TCL "    set clk_prop_val_1    true"
    lappend POSTSYS_LINK_TCL "    set clk_port_1        aclk_kernel_01_cont"
    lappend POSTSYS_LINK_TCL "  \} else \{"
    lappend POSTSYS_LINK_TCL "    common::send_msg_id \{XBTEST_POSTSYS_LINK-1\} \{ERROR\} \"Failed to connect continuous clocks. UCS version (\$ucs_version) not defined in connect_continuous_clocks in your postsys_link.tcl\""
    lappend POSTSYS_LINK_TCL "  \}"
    lappend POSTSYS_LINK_TCL "  # Check the UCS cell exists"
    lappend POSTSYS_LINK_TCL "  if \{\[get_bd_cells \$ucs_name\] == \{\}\} \{"
    lappend POSTSYS_LINK_TCL "    common::send_msg_id \{XBTEST_POSTSYS_LINK-2\} \{ERROR\} \"Failed to connect continuous clocks. UCS cell (\$ucs_name) not found. Check cell name in BD\""
    lappend POSTSYS_LINK_TCL "  \}"
    lappend POSTSYS_LINK_TCL "  # Enable UCS kernel continuous clocks outputs"
    lappend POSTSYS_LINK_TCL "  foreach \{prop val\} \[dict create \$clk_prop_name_0 \$clk_prop_val_0 \$clk_prop_name_1 \$clk_prop_val_0\] \{"
    lappend POSTSYS_LINK_TCL "    # Check property exists"
    lappend POSTSYS_LINK_TCL "    if \{!\[regexp -nocase -- \".*CONFIG.\$\{prop\}.*\" \[list_property \[get_bd_cells \$ucs_name\]\]\]\} \{"
    lappend POSTSYS_LINK_TCL "      common::send_msg_id \{XBTEST_POSTSYS_LINK-3\} \{ERROR\} \"Failed to connect continuous clocks. UCS cell property (CONFIG.\$prop) does not exists. Check UCS susbsystem (\$ucs_name) version\""
    lappend POSTSYS_LINK_TCL "    \}"
    lappend POSTSYS_LINK_TCL "    set_property CONFIG.\$prop \$val \[get_bd_cells \$ucs_name\]"
    lappend POSTSYS_LINK_TCL "  \}"
    lappend POSTSYS_LINK_TCL "  # Connect UCS continuous clocks outputs to clock inputs of all xbtest compute units continuous"
    lappend POSTSYS_LINK_TCL "  foreach \{src dst\} \[dict create \$clk_port_0 ap_clk_cont \$clk_port_1 ap_clk_2_cont\] \{"
    lappend POSTSYS_LINK_TCL "    if \{\[get_bd_pins \$ucs_name/\$src\] == \{\}\} \{"
    lappend POSTSYS_LINK_TCL "      common::send_msg_id \{XBTEST_POSTSYS_LINK-4\} \{ERROR\} \"Failed to connect continuous clocks. UCS cell pin (\$ucs_name/\$src) not found. Check cell pin name in BD\""
    lappend POSTSYS_LINK_TCL "    \}"
    lappend POSTSYS_LINK_TCL "    connect_bd_net \[get_bd_pins \$ucs_name/\$src\] \[get_bd_pins krnl_*/\$dst\]"
    lappend POSTSYS_LINK_TCL "  \}"
    lappend POSTSYS_LINK_TCL "\}"
    lappend POSTSYS_LINK_TCL "# Execute body"
    lappend POSTSYS_LINK_TCL "postsys_link_body"

    set postsys_link_tcl [file join [dict get $config vpp_cfg_template_dir] postsys_link.tcl]
    write_file $postsys_link_tcl [join $POSTSYS_LINK_TCL "\n"]
    log_message $config {XBTEST_WIZARD-40} [list $postsys_link_tcl]

    # Empty place holders
    set place_design_pre_tcl [file join [dict get $config vpp_cfg_template_dir] place_design_pre.tcl]
    write_file $place_design_pre_tcl ""
    log_message $config {XBTEST_WIZARD-40} [list $place_design_pre_tcl]

    # Empty place holders
    set route_design_pre_tcl [file join [dict get $config vpp_cfg_template_dir] route_design_pre.tcl]
    write_file $route_design_pre_tcl ""
    log_message $config {XBTEST_WIZARD-40} [list $route_design_pre_tcl]

    set     VPP_INI {}
    lappend VPP_INI "remote_ip_cache=../../../remote_ip_cache"
    lappend VPP_INI ""
    lappend VPP_INI "# TCL hooks"
    lappend VPP_INI "\[advanced\]"
    lappend VPP_INI "param=compiler.userPostSysLinkOverlayTcl=../../vpp_cfg/postsys_link.tcl"
    lappend VPP_INI ""
    lappend VPP_INI "\[vivado\]"
    lappend VPP_INI "prop=run.impl_1.STEPS.PLACE_DESIGN.TCL.PRE=../../vpp_cfg/place_design_pre.tcl"
    lappend VPP_INI "prop=run.impl_1.STEPS.ROUTE_DESIGN.TCL.PRE=../../vpp_cfg/route_design_pre.tcl"
    lappend VPP_INI ""
    lappend VPP_INI "# Build strategy/directives"
    lappend VPP_INI "# \[vivado\]"
    lappend VPP_INI "# prop=run.impl_1.strategy=Performance_EarlyBlockPlacement"
    lappend VPP_INI "# prop=run.impl_1.STEPS.POST_ROUTE_PHYS_OPT_DESIGN.IS_ENABLED=true"
    lappend VPP_INI "# prop=run.impl_1.STEPS.ROUTE_DESIGN.ARGS.DIRECTIVE=Explore"
    lappend VPP_INI "# prop=run.impl_1.STEPS.POST_ROUTE_PHYS_OPT_DESIGN.ARGS.DIRECTIVE=Explore"
    lappend VPP_INI "# prop=run.impl_1.STEPS.PHYS_OPT_DESIGN.ARGS.DIRECTIVE=Explore"
    lappend VPP_INI "# prop=run.impl_1.STEPS.PHYS_OPT_DESIGN.IS_ENABLED=true"

    set vpp_ini [file join [dict get $config vpp_cfg_template_dir] vpp.ini]
    write_file $vpp_ini [join $VPP_INI "\n"]
    log_message $config {XBTEST_WIZARD-40} [list $vpp_ini]
}
############################################################################################################
# Get all the IPDEF user parameters
############################################################################################################
proc load_user_parameters { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    # First make sure MESSAGES_DICT is loaded, needed to use log_message
    dict set config MESSAGES_DICT [get_property CONFIG.MESSAGES_DICT [dict get $config ip]]

    # Start of wizard example design
    log_message $config {XBTEST_WIZARD-1}

    set user_params {}
    foreach property [list_property [dict get $config ip] CONFIG.*] {
        # Removing the CONFIG. prefix from the property
        regsub {^CONFIG\.} $property {} user_param_name

        # Add non-empty strings to config array
        if {(![string equal [string trim $user_param_name] {}]) && ($user_param_name != {MESSAGES_DICT}) && ([regexp -- {INSERT_VIP} $user_param_name] == 0)} {
            lappend user_params $user_param_name
            dict set config $user_param_name [get_property $property [dict get $config ip]]
        }
    }
    foreach user_param_name $user_params {
        set autoconfigure_proc autoconfigure_$user_param_name
        if {[info proc $autoconfigure_proc] != {}} {
            $autoconfigure_proc config
        }
    }
    foreach user_param_name $user_params {
        set validate_proc validate_$user_param_name
        if {[info proc $validate_proc] != {}} {
            $validate_proc config
        }
    }
}
############################################################################################################
# Get some of the IPDEF properties
############################################################################################################
proc load_ipdef_parameters { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set ipdef [get_ipdefs [get_property -quiet IPDEF [dict get $config ip]]]

    dict set config ip_vlnv     [get_property -quiet VLNV    $ipdef]
    dict set config ip_catalog  [file dirname [file dirname [get_property xml_file_name $ipdef]]]

    set vlnv_split [split [dict get $config ip_vlnv] {:}]

    dict set config ip_vendor           [lindex $vlnv_split 0]
    dict set config ip_library          [lindex $vlnv_split 1]
    dict set config ip_name             [lindex $vlnv_split 2]
    dict set config ip_version          [lindex $vlnv_split 3]

    set version_split [split [dict get $config ip_version] {.}]

    dict set config ip_version_major    [lindex $version_split 0]
    dict set config ip_version_minor    [lindex $version_split 1]
    dict set config ip_name_v           [dict get $config ip_name]_v[dict get $config ip_version_major]_[dict get $config ip_version_minor]

    log_message $config {XBTEST_WIZARD-3} [list [dict get $config ip_name_v]]
}
############################################################################################################
# Find the xpfm and queries the platform info, stores it into a dictionary of config variable.
############################################################################################################
proc load_platforminfo { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set xpfm [dict_get_quiet $config xpfm]
    if {$xpfm != {}} {
        set xpfm_ext [string tolower [file extension $xpfm]]

        log_message $config {XBTEST_WIZARD-15} [list {platform XPFM} $xpfm]; #Reading platform .xpfm
        load librdi_platformtcl[info sharedlibextension]

        if {$xpfm_ext == {.xsa}} {
            # For XSA, the platforminfo returned = value of hardwarePlatform node
            log_message $config {XBTEST_WIZARD-35} [list {XSA} {::platform::query -xpfm <xsa>}]
            set platforminfo_json [::platform::query -xpfm $xpfm]
        } else {
            # For xpfm, the value of hardwarePlatform node is in platforminfo returned at :
            #       - for Flat platform: hardwarePlatforms.hardwarePlatform node
            #       - for DFX platform: hardwarePlatforms.reconfigurablePartitions[0].hardwarePlatform node
            # hardwarePlatform node is found by the tool and its value is returned when providing option:: -key_path hardwarePlatform
            log_message $config {XBTEST_WIZARD-35} [list {XPFM} {::platform::query -xpfm <xpfm> -key_path hardwarePlatform}]
            set platforminfo_json [::platform::query -xpfm $xpfm -key_path hardwarePlatform]
        }

        write_file [file join [dict get $config run_dir] platforminfo.json] $platforminfo_json; # Save platforminfo JSON

        if {$platforminfo_json == {}} {
            log_message $config {XBTEST_WIZARD-34} [list $xpfm {empty platform metadata}]; # critical warning: unable to auto-configure
        }

        set platforminfo [json2dict $platforminfo_json]

        # Check if the raptor extension is found
        if {![dict_exist $platforminfo {extensions raptor2}]} {
            log_message $config {XBTEST_WIZARD-34} [list $xpfm {extensions.raptor2 section not found in platform metadata}]; # critical warning: unable to auto-configure
        }

        dict set config platforminfo $platforminfo
    }
}
############################################################################################################
# Find the wizard_config_json and read its content, stores it into a dictionary of config variable.
############################################################################################################
proc load_wizard_config_json { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set wizard_config_json [dict get $config wizard_config_json]
    if {$wizard_config_json == {<path>}} {
        return 1; # Do not load if not provided
    }

    log_message $config {XBTEST_WIZARD-15} [list {wizard configuration JSON file} $wizard_config_json]; # Reading wizard_config_json

    set wizard_config_data [read_file $wizard_config_json]

    # Save user configuration
    write_file [file join [dict get $config run_dir] wizard_user_config.json] $wizard_config_data

    dict set config wizard_config [json2dict $wizard_config_data]

    check_wizard_config_json config

    set wizard_config_name [dict get $config wizard_config_name]
    if {$wizard_config_name == {default}} {
        return 1; # Do check config name if not provided
    }
    set expected_values [lsort -dictionary [dict keys [dict get $config wizard_config]]]
    if {$wizard_config_name ni $expected_values} {
        log_message $config {XBTEST_WIZARD-24} [list $wizard_config_name {wizard_config_name} "\{$expected_values\}"]; # ERROR invalid value
        return 0
    }
}
############################################################################################################
# Wizard configuration JSON file definition
############################################################################################################
proc get_wizard_config_json_def {} {
    set any_key {<key>}
    set any_value {<value>}

    # Use $any_key accept any key
    # Use $any_value to accept any value
    # Actual $any_key and $any_value provided should be check later in autoconfigure and validate procedures

    set wizard_config_json_def [dict create \
        platform [dict create \
            fpga_part               $any_value \
            fpga_family             $any_value \
            name                    $any_value \
            interface_uuid          $any_value \
            mmio_support            $any_value \
            is_nodma                $any_value \
            dma_support             $any_value \
            p2p_support             $any_value \
            mac_addresses_available $any_value \
            aie [dict create \
                freq $any_value \
                control $any_value \
                status $any_value \
            ] \
            gt [dict create \
                $any_key [dict create \
                    slr            $any_value \
                    type           $any_value \
                    group_select   $any_value \
                    refclk_sel     $any_value \
                ] \
            ] \
        ] \
        build [dict create \
            pwr_floorplan_dir       $any_value \
            vpp_options_dir         $any_value \
            display_pwr_floorplan   $any_value \
            vpp_link_output         $any_value \
        ] \
        cu_configuration [dict create \
            clock [dict create \
                $any_key [dict create \
                    freq $any_value \
                ] \
            ] \
            plram_selection [dict create \
                $any_key $any_value \
            ] \
            verify [dict create \
                slr      $any_value \
                dna_read $any_value \
            ] \
            power [dict create \
                $any_key [dict create \
                    throttle_mode $any_value \
                ] \
            ] \
            gt [dict create \
                $any_key [dict create \
                    diff_clocks  $any_value \
                    serial_port  $any_value \
                ] \
            ] \
            gt_mac [dict create \
                $any_key [dict create \
                    ip_sel       $any_value \
                    enable_rsfec $any_value \
                ] \
            ] \
            gt_prbs [dict create \
                $any_key [dict create \
                    ip_sel       $any_value \
                ] \
            ] \
            gtf_prbs [dict create \
                $any_key [dict create \
                    ip_sel       $any_value \
                ] \
            ] \
            gtm_prbs [dict create \
                $any_key [dict create \
                    ip_sel       $any_value \
                ] \
            ] \
            gtyp_prbs [dict create \
                $any_key [dict create \
                    ip_sel       $any_value \
                ] \
            ] \
            memory [dict create \
                $any_key [dict create \
                    global [dict create \
                        target          $any_value \
                        axi_data_size   $any_value \
                        axi_id_threads  $any_value \
                        axi_outstanding $any_value \
                    ] \
                    specific [dict create \
                        $any_key [dict create \
                            slr      $any_value \
                            sptag    $any_value \
                        ] \
                    ] \
                ] \
            ] \
        ] \
        cu_selection [dict create \
            power      $any_value \
            gt_mac     $any_value \
            gt_lpbk    $any_value \
            gt_prbs    $any_value \
            gtf_prbs   $any_value \
            gtm_prbs   $any_value \
            gtyp_prbs  $any_value \
            memory     $any_value \
        ] \
    ]
    return $wizard_config_json_def
}
############################################################################################################
# Top function to check the node names in the wizard configuration JSON file for each config name
############################################################################################################
proc check_wizard_config_json { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set wizard_config_json_def  [get_wizard_config_json_def]
    set wizard_config           [dict_get_quiet $config wizard_config]

    foreach {config_name config_value} $wizard_config {
        check_wizard_config_json_next config [list $config_name] $config_value $wizard_config_json_def
    }
}
############################################################################################################
# Recursive function to check the node names in the wizard configuration JSON file
############################################################################################################
proc check_wizard_config_json_next { config_ref path_in config_value config_def} {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set any_key     {<key>}
    set any_value   {<value>}

    if {$config_def == $any_value} {
        return
    }

    set valid_keys [dict keys $config_def]

    foreach {key value} $config_value {
        set path $path_in
        lappend path $key
        if {$valid_keys == $any_key} {
            if {[llength $value] > 1} {
                check_wizard_config_json_next config $path $value [dict get $config_def $any_key]
            }
        } elseif {$key in $valid_keys} {
            if {[llength $value] > 1} {
                check_wizard_config_json_next config $path $value [dict get $config_def $key]
            }
        } else {
            log_message $config {XBTEST_WIZARD-30} [list {wizard configuration JSON file} [join $path {.}]]
        }
    }
}
############################################################################################################
# Wizard_config autoconfigure/merge/validate
############################################################################################################
proc fn_wizard_configure { config_ref fn cfg_name } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    # Define here in what order parameter in the wizard configuration JSON file are autoconfigured/validated
    # Autoconfigure procedures should add the parameter to the wizard_actual_config dictionary (based on wizard_config_name, default wizard_config and platform info)
    # Validate procedures should check the parameter in the wizard_actual_config dictionary

    ${fn}_platform_fpga_part                config $cfg_name [list platform fpga_part]
    ${fn}_platform_fpga_family              config $cfg_name [list platform fpga_family]
    ${fn}_platform_name                     config $cfg_name [list platform name]
    ${fn}_platform_interface_uuid           config $cfg_name [list platform interface_uuid]
    ${fn}_platform_mmio_support             config $cfg_name [list platform mmio_support]
    ${fn}_platform_is_nodma                 config $cfg_name [list platform is_nodma]
    ${fn}_platform_dma_support              config $cfg_name [list platform dma_support]
    ${fn}_platform_p2p_support              config $cfg_name [list platform p2p_support]
    ${fn}_platform_mac_addresses_available  config $cfg_name [list platform mac_addresses_available]
    ${fn}_platform_gt                       config $cfg_name [list platform gt]
    ${fn}_platform_aie_freq                 config $cfg_name [list platform aie freq]
    ${fn}_platform_aie_control              config $cfg_name [list platform aie control]
    ${fn}_platform_aie_status               config $cfg_name [list platform aie status]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name platform gt]] {
        ${fn}_platform_gt_slr           config $cfg_name [list platform gt $gt_idx slr]
        ${fn}_platform_gt_type          config $cfg_name [list platform gt $gt_idx type]
        ${fn}_platform_gt_group_select  config $cfg_name [list platform gt $gt_idx group_select]
        ${fn}_platform_gt_refclk_sel    config $cfg_name [list platform gt $gt_idx refclk_sel]
    }
    ${fn}_build_pwr_floorplan_dir       config $cfg_name [list build pwr_floorplan_dir]
    ${fn}_build_vpp_options_dir         config $cfg_name [list build vpp_options_dir]
    ${fn}_build_display_pwr_floorplan   config $cfg_name [list build display_pwr_floorplan]
    ${fn}_build_vpp_link_output         config $cfg_name [list build vpp_link_output]

    # Clock
    ${fn}_cu_configuration_clock config $cfg_name [list cu_configuration clock]
    foreach {clk_idx clk_def} [dict_get_quiet $config [list $cfg_name cu_configuration clock]] {
        ${fn}_cu_configuration_clock_freq config $cfg_name [list cu_configuration clock $clk_idx freq]
    }
    # PLRAM
    ${fn}_cu_configuration_plram_selection  config $cfg_name [list cu_configuration plram_selection]

    # Verify
    ${fn}_cu_configuration_verify_slr       config $cfg_name [list cu_configuration verify slr]
    ${fn}_cu_configuration_verify_dna_read  config $cfg_name [list cu_configuration verify dna_read]

    # Power
    ${fn}_cu_configuration_power config $cfg_name [list cu_configuration power]
    foreach {slr_idx power_def} [dict_get_quiet $config [list $cfg_name cu_configuration power]] {
        ${fn}_cu_configuration_power_throttle_mode config $cfg_name [list cu_configuration power $slr_idx throttle_mode]
    }
    # GT
    ${fn}_cu_configuration_gt config $cfg_name [list cu_configuration gt]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name cu_configuration gt]] {
        ${fn}_cu_configuration_gt_diff_clocks   config $cfg_name [list cu_configuration gt $gt_idx diff_clocks]
        ${fn}_cu_configuration_gt_serial_port   config $cfg_name [list cu_configuration gt $gt_idx serial_port]
    }
    # GT_MAC
    ${fn}_cu_configuration_gt_mac config $cfg_name [list cu_configuration gt_mac]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name cu_configuration gt_mac]] {
        ${fn}_cu_configuration_gt_mac_ip_sel        config $cfg_name [list cu_configuration gt_mac $gt_idx ip_sel]
        ${fn}_cu_configuration_gt_mac_enable_rsfec  config $cfg_name [list cu_configuration gt_mac $gt_idx enable_rsfec]
    }
    # GT_PRBS
    ${fn}_cu_configuration_gt_prbs config $cfg_name [list cu_configuration gt_prbs]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name cu_configuration gt_prbs]] {
        ${fn}_cu_configuration_gt_prbs_ip_sel       config $cfg_name [list cu_configuration gt_prbs $gt_idx ip_sel]
    }
    # GTF_PRBS
    ${fn}_cu_configuration_gtf_prbs config $cfg_name [list cu_configuration gtf_prbs]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name cu_configuration gtf_prbs]] {
        ${fn}_cu_configuration_gtf_prbs_ip_sel  config $cfg_name [list cu_configuration gtf_prbs $gt_idx ip_sel]
    }
    # GTM_PRBS
    ${fn}_cu_configuration_gtm_prbs config $cfg_name [list cu_configuration gtm_prbs]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name cu_configuration gtm_prbs]] {
        ${fn}_cu_configuration_gtm_prbs_ip_sel  config $cfg_name [list cu_configuration gtm_prbs $gt_idx ip_sel]
    }
    # GTYP_PRBS
    ${fn}_cu_configuration_gtyp_prbs config $cfg_name [list cu_configuration gtyp_prbs]
    foreach {gt_idx gt_def} [dict_get_quiet $config [list $cfg_name cu_configuration gtyp_prbs]] {
        ${fn}_cu_configuration_gtyp_prbs_ip_sel  config $cfg_name [list cu_configuration gtyp_prbs $gt_idx ip_sel]
    }
    # Memory
    ${fn}_cu_configuration_memory config $cfg_name [list cu_configuration memory]
    foreach {memory_type memory_def} [dict_get_quiet $config [list $cfg_name cu_configuration memory]] {
        ${fn}_cu_configuration_memory_global_target             config $cfg_name [list cu_configuration memory $memory_type global target]
        ${fn}_cu_configuration_memory_global_axi_data_size      config $cfg_name [list cu_configuration memory $memory_type global axi_data_size]
        ${fn}_cu_configuration_memory_global_axi_id_threads     config $cfg_name [list cu_configuration memory $memory_type global axi_id_threads]
        ${fn}_cu_configuration_memory_global_axi_outstanding    config $cfg_name [list cu_configuration memory $memory_type global axi_outstanding]
        ${fn}_cu_configuration_memory_specific                  config $cfg_name [list cu_configuration memory $memory_type specific]
        foreach {cu_idx cu_def} [dict_get_quiet $config [list $cfg_name cu_configuration memory $memory_type specific]] {
            ${fn}_cu_configuration_memory_specific_slr          config $cfg_name [list cu_configuration memory $memory_type specific $cu_idx slr]
            ${fn}_cu_configuration_memory_specific_sptag        config $cfg_name [list cu_configuration memory $memory_type specific $cu_idx sptag]
        }
    }
    ${fn}_cu_selection_power     config $cfg_name [list cu_selection power]
    ${fn}_cu_selection_gt_mac    config $cfg_name [list cu_selection gt_mac]
    ${fn}_cu_selection_gt_lpbk   config $cfg_name [list cu_selection gt_lpbk]
    ${fn}_cu_selection_gt_prbs   config $cfg_name [list cu_selection gt_prbs]
    ${fn}_cu_selection_gtf_prbs  config $cfg_name [list cu_selection gtf_prbs]
    ${fn}_cu_selection_gtm_prbs  config $cfg_name [list cu_selection gtm_prbs]
    ${fn}_cu_selection_gtyp_prbs config $cfg_name [list cu_selection gtyp_prbs]
    ${fn}_cu_selection_memory    config $cfg_name [list cu_selection memory]
}
############################################################################################################
# Wizard_config autoconfigure/merge/validate
############################################################################################################
proc log_wizard_config { config cfg_name {wizard_config_names_in {}} {filename_in {}} } {

    set TAB_1 "  "
    set TAB_2 "$TAB_1$TAB_1"
    set TAB_3 "$TAB_2$TAB_1"
    set TAB_4 "$TAB_3$TAB_1"
    set TAB_5 "$TAB_4$TAB_1"
    set TAB_6 "$TAB_5$TAB_1"
    set TAB_7 "$TAB_6$TAB_1"

    if {$wizard_config_names_in == {}} {
        if {$cfg_name == {wizard_actual_config}} {
            set wizard_config_names [list [dict get $config wizard_config_name]]
        } else {
            set wizard_config_names {{default}}
        }
    } else {
        set wizard_config_names $wizard_config_names_in
    }

    set     CONFIG_JSON {}
    lappend CONFIG_JSON "\{"

    set hh 0; set hh_max [expr [llength $wizard_config_names] - 1]

    foreach wizard_config_name $wizard_config_names {

        if {$wizard_config_names_in == {}} {
            set cfg [dict_get_quiet $config [list $cfg_name]]
        } else {
            set cfg [dict_get_quiet $config [list $wizard_config_name]]
        }

        set platform_gt                         [dict_get_quiet $cfg {platform gt}]
        set platform_fpga_part                  [dict_get_quiet $cfg {platform fpga_part}]
        set platform_fpga_family                [dict_get_quiet $cfg {platform fpga_family}]
        set platform_name                       [dict_get_quiet $cfg {platform name}]
        set platform_interface_uuid             [dict_get_quiet $cfg {platform interface_uuid}]
        set platform_mmio_support               [dict_get_quiet $cfg {platform mmio_support}]
        set platform_is_nodma                   [dict_get_quiet $cfg {platform is_nodma}]
        set platform_dma_support                [dict_get_quiet $cfg {platform dma_support}]
        set platform_p2p_support                [dict_get_quiet $cfg {platform p2p_support}]
        set platform_mac_addresses_available    [dict_get_quiet $cfg {platform mac_addresses_available}]
        set platform_aie_freq                   [dict_get_quiet $cfg {platform aie freq}]
        set platform_aie_control                [dict_get_quiet $cfg {platform aie control}]
        set platform_aie_status                 [dict_get_quiet $cfg {platform aie status}]

        set build_pwr_floorplan_dir             [dict_get_quiet $cfg {build pwr_floorplan_dir}]
        set build_vpp_options_dir               [dict_get_quiet $cfg {build vpp_options_dir}]
        set build_display_pwr_floorplan         [dict_get_quiet $cfg {build display_pwr_floorplan}]
        set build_vpp_link_output               [dict_get_quiet $cfg {build vpp_link_output}]

        set cu_configuration_clock              [dict_get_quiet $cfg {cu_configuration clock}]
        set cu_configuration_plram_selection    [dict_get_quiet $cfg {cu_configuration plram_selection}]
        set cu_configuration_verify             [dict_get_quiet $cfg {cu_configuration verify}]
        set cu_configuration_gt                 [dict_get_quiet $cfg {cu_configuration gt}]
        set cu_configuration_gt_mac             [dict_get_quiet $cfg {cu_configuration gt_mac}]
        set cu_configuration_gt_prbs            [dict_get_quiet $cfg {cu_configuration gt_prbs}]
        set cu_configuration_gtf_prbs           [dict_get_quiet $cfg {cu_configuration gtf_prbs}]
        set cu_configuration_gtm_prbs           [dict_get_quiet $cfg {cu_configuration gtm_prbs}]
        set cu_configuration_gtyp_prbs          [dict_get_quiet $cfg {cu_configuration gtyp_prbs}]
        set cu_configuration_memory             [dict_get_quiet $cfg {cu_configuration memory}]
        set cu_configuration_power              [dict_get_quiet $cfg {cu_configuration power}]

        set cu_selection_power                  [dict_get_quiet $cfg {cu_selection power}]
        set cu_selection_gt_mac                 [dict_get_quiet $cfg {cu_selection gt_mac}]
        set cu_selection_gt_lpbk                [dict_get_quiet $cfg {cu_selection gt_lpbk}]
        set cu_selection_gt_prbs                [dict_get_quiet $cfg {cu_selection gt_prbs}]
        set cu_selection_gtf_prbs               [dict_get_quiet $cfg {cu_selection gtf_prbs}]
        set cu_selection_gtm_prbs               [dict_get_quiet $cfg {cu_selection gtm_prbs}]
        set cu_selection_gtyp_prbs              [dict_get_quiet $cfg {cu_selection gtyp_prbs}]
        set cu_selection_memory                 [dict_get_quiet $cfg {cu_selection memory}]

        if {($platform_fpga_part != {}) || ($platform_fpga_family != {}) || ($platform_name != {}) || ($platform_interface_uuid != {}) || ($platform_mmio_support != {}) || ($platform_is_nodma != {}) || ($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} {
            set is_platform_section true
        } else {
            set is_platform_section false
        }
        if {($build_pwr_floorplan_dir != {}) || ($build_vpp_options_dir != {}) || ($build_display_pwr_floorplan != {}) || ($build_vpp_link_output != {})} {
            set is_build_section true
        } else {
            set is_build_section false
        }
        if {($cu_configuration_clock != {}) || ($cu_configuration_plram_selection != {}) || ($cu_configuration_verify != {}) || ($cu_configuration_gt != {})
         || ($cu_configuration_gt_mac != {}) || ($cu_configuration_gt_prbs != {}) || ($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})        } {
            set is_cu_configuration_section true
        } else {
            set is_cu_configuration_section false
        }
        if {($cu_selection_power != {}) || ($cu_selection_gt_mac != {}) || ($cu_selection_gt_lpbk != {}) || ($cu_selection_gt_prbs != {}) || ($cu_selection_gtf_prbs != {}) || ($cu_selection_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {})|| ($cu_selection_memory != {})} {
            set is_cu_selection_section true
        } else {
            set is_cu_selection_section false
        }

    lappend CONFIG_JSON "$TAB_1[string2json $wizard_config_name] : \{"

        if {$is_platform_section} {
    lappend CONFIG_JSON     "$TAB_2[string2json {platform}] : \{"
            if {$platform_fpga_part != {}} {
                if {($platform_fpga_family != {}) || ($platform_name != {}) || ($platform_interface_uuid != {}) || ($platform_mmio_support != {})  || ($platform_is_nodma != {}) || ($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {fpga_part}] : [string2json $platform_fpga_part]$next"
            }
            if {$platform_fpga_family != {}} {
                if {($platform_name != {}) || ($platform_interface_uuid != {}) || ($platform_mmio_support != {}) || ($platform_is_nodma != {}) || ($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {fpga_family}] : [string2json $platform_fpga_family]$next"
            }
            if {$platform_name != {}} {
                if {($platform_interface_uuid != {}) || ($platform_mmio_support != {})  || ($platform_is_nodma != {}) || ($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {name}] : [string2json $platform_name]$next"
            }
            if {$platform_interface_uuid != {}} {
                if {($platform_mmio_support != {}) || ($platform_is_nodma != {}) || ($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {interface_uuid}] : [string2json $platform_interface_uuid]$next"
            }
            if {$platform_mmio_support != {}} {
                if {($platform_is_nodma != {}) || ($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {mmio_support}] : $platform_mmio_support$next"
            }
            if {$platform_is_nodma != {}} {
                if {($platform_dma_support != {}) || ($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {is_nodma}] : $platform_is_nodma$next"
            }
            if {$platform_dma_support != {}} {
                if {($platform_p2p_support != {}) || ($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {dma_support}] : $platform_dma_support$next"
            }
            if {$platform_p2p_support != {}} {
                if {($platform_mac_addresses_available != {}) || ($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {p2p_support}] : $platform_p2p_support$next"
            }
            if {$platform_mac_addresses_available != {}} {
                if {($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {}) || ($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {mac_addresses_available}] : $platform_mac_addresses_available$next"
            }
            if {($platform_aie_freq != {}) || ($platform_aie_control != {}) || ($platform_aie_status != {})} {
    lappend CONFIG_JSON         "$TAB_3[string2json {aie}] : \{"
                if {$platform_aie_freq != {}} {
                    if {($platform_aie_control != {}) || ($platform_aie_status != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON                "$TAB_4[string2json {freq}] : $platform_aie_freq$next"
                }
                if {$platform_aie_control != {}} {
                    if {$platform_aie_status != {}} { set next "," } else { set next "" }
    lappend CONFIG_JSON             "$TAB_4[string2json {control}] : $platform_aie_control$next"
                }
                if {$platform_aie_status != {}} {
    lappend CONFIG_JSON             "$TAB_4[string2json {status}] : $platform_aie_status"
                }
                if {($platform_gt != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$platform_gt != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gt}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $platform_gt]] - 1]
                foreach {gt_idx gt_def} $platform_gt {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {slr}] : [string2json [dict_get_quiet $gt_def {slr}]],"
    lappend CONFIG_JSON                 "$TAB_5[string2json {type}] : [string2json [dict_get_quiet $gt_def {type}]],"
    lappend CONFIG_JSON                 "$TAB_5[string2json {group_select}] : [strlist2json [dict_get_quiet $gt_def {group_select}]],"
    lappend CONFIG_JSON                 "$TAB_5[string2json {refclk_sel}] :  [dict_get_quiet $gt_def {refclk_sel}]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
    lappend CONFIG_JSON         "$TAB_3\}"
            }
            if {$is_build_section || $is_cu_configuration_section || $is_cu_selection_section} { set next "," } else { set next "" }
    lappend CONFIG_JSON     "$TAB_2\}$next"
        }

        if {$is_build_section} {
    lappend CONFIG_JSON     "$TAB_2[string2json {build}] : \{"
            if {$build_pwr_floorplan_dir != {}} {
                if {($build_vpp_options_dir != {}) || ($build_display_pwr_floorplan != {}) || ($build_vpp_link_output != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {pwr_floorplan_dir}] : [string2json $build_pwr_floorplan_dir]$next"
            }
            if {$build_vpp_options_dir != {}} {
                if {($build_display_pwr_floorplan != {}) || ($build_vpp_link_output != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {vpp_options_dir}] : [string2json $build_vpp_options_dir]$next"
            }
            if {$build_display_pwr_floorplan != {}} {
                if {$build_vpp_link_output != {}} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {display_pwr_floorplan}] : ${build_display_pwr_floorplan}$next"
            }
            if {$build_vpp_link_output != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {vpp_link_output}] : [string2json $build_vpp_link_output]"
            }
            if {$is_cu_configuration_section || $is_cu_selection_section} { set next "," } else { set next "" }
    lappend CONFIG_JSON     "$TAB_2\}$next"
        }

        if {$is_cu_configuration_section} {
    lappend CONFIG_JSON     "$TAB_2[string2json {cu_configuration}] : \{"
            if {$cu_configuration_clock != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {clock}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_clock]] - 1]
                foreach {clock_idx clock_def} $cu_configuration_clock {
    lappend CONFIG_JSON             "$TAB_4[string2json $clock_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {freq}] : [dict_get_quiet $clock_def {freq}]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_plram_selection != {}) || ($cu_configuration_verify != {}) || ($cu_configuration_gt != {}) || ($cu_configuration_gt_mac != {}) || ($cu_configuration_gt_prbs != {}) || ($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_plram_selection != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {plram_selection}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_plram_selection]] - 1]
                foreach {slr sptag} $cu_configuration_plram_selection {
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4[string2json $slr] : [string2json $sptag]$next"
                }
                if {($cu_configuration_verify != {}) || ($cu_configuration_gt != {}) || ($cu_configuration_gt_mac != {}) || ($cu_configuration_gt_prbs != {}) || ($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }

            if {$cu_configuration_verify != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {verify}] : \{"
    lappend CONFIG_JSON             "$TAB_4[string2json {slr}] : [string2json [dict_get_quiet $cu_configuration_verify {slr}] ],"
                set dna_read [dict_get_quiet $cu_configuration_verify {dna_read}]
                if {$dna_read == {auto}} {
    lappend CONFIG_JSON             "$TAB_4[string2json {dna_read}] : [string2json $dna_read]"
                } else {
    lappend CONFIG_JSON             "$TAB_4[string2json {dna_read}] : $dna_read"
                }
                if {($cu_configuration_gt != {}) || ($cu_configuration_gt_mac != {}) || ($cu_configuration_gt_prbs != {}) || ($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }

            if {$cu_configuration_gt != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gt}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_gt]] - 1]
                foreach {gt_idx gt_def} $cu_configuration_gt {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {diff_clocks}]  : [strlist2json [dict_get_quiet $gt_def {diff_clocks}]],"
    lappend CONFIG_JSON                 "$TAB_5[string2json {serial_port}]  : [strlist2json [dict_get_quiet $gt_def {serial_port}]]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_gt_mac != {}) || ($cu_configuration_gt_prbs != {}) || ($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_gt_mac != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gt_mac}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_gt_mac]] - 1]
                foreach {gt_idx gt_def} $cu_configuration_gt_mac {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {ip_sel}] : [string2json [dict_get_quiet $gt_def {ip_sel}]],"
    lappend CONFIG_JSON                 "$TAB_5[string2json {enable_rsfec}] : [dict_get_quiet $gt_def {enable_rsfec}]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_gt_prbs != {}) || ($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_gt_prbs != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gt_prbs}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_gt_prbs]] - 1]
                foreach {gt_idx gt_def} $cu_configuration_gt_prbs {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {ip_sel}] : [string2json [dict_get_quiet $gt_def {ip_sel}]]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_gtf_prbs != {}) || ($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_gtf_prbs != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gtf_prbs}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_gtf_prbs]] - 1]
                foreach {gt_idx gt_def} $cu_configuration_gtf_prbs {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {ip_sel}] : [string2json [dict_get_quiet $gt_def {ip_sel}]]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_gtm_prbs != {}) || ($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_gtm_prbs != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gtm_prbs}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_gtm_prbs]] - 1]
                foreach {gt_idx gt_def} $cu_configuration_gtm_prbs {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {ip_sel}] : [string2json [dict_get_quiet $gt_def {ip_sel}]]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_gtyp_prbs != {}) || ($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_gtyp_prbs != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {gtyp_prbs}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_gtyp_prbs]] - 1]
                foreach {gt_idx gt_def} $cu_configuration_gtyp_prbs {
    lappend CONFIG_JSON             "$TAB_4[string2json $gt_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {ip_sel}] : [string2json [dict_get_quiet $gt_def {ip_sel}]]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {($cu_configuration_memory != {}) || ($cu_configuration_power != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_memory != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {memory}] : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_memory]] - 1]
                foreach {memory_type memory_def} $cu_configuration_memory {
    lappend CONFIG_JSON             "$TAB_4[string2json $memory_type] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {global}] : \{"
    lappend CONFIG_JSON                     "$TAB_6[string2json {target}] : [string2json [dict_get_quiet $memory_def {global target}]],"
    lappend CONFIG_JSON                     "$TAB_6[string2json {axi_data_size}] : [dict_get_quiet $memory_def {global axi_data_size}],"
    lappend CONFIG_JSON                     "$TAB_6[string2json {axi_id_threads}] : [dict_get_quiet $memory_def {global axi_id_threads}],"
    lappend CONFIG_JSON                     "$TAB_6[string2json {axi_outstanding}] : [dict_get_quiet $memory_def {global axi_outstanding}]"
    lappend CONFIG_JSON                 "$TAB_5\},"
    lappend CONFIG_JSON                 "$TAB_5[string2json {specific}] : \{"
                    set cu_configuration_memory_specific [dict_get_quiet $memory_def {specific}]
                    set jj 0; set jj_max [expr [llength [dict keys $cu_configuration_memory_specific]] - 1]
                    foreach {cu_idx cu_def} $cu_configuration_memory_specific {
    lappend CONFIG_JSON                     "$TAB_6[string2json $cu_idx] : \{"
    lappend CONFIG_JSON                         "$TAB_7[string2json {slr}] : [string2json [dict_get_quiet $cu_def {slr}]],"
    lappend CONFIG_JSON                         "$TAB_7[string2json {sptag}] : [strlist2json [dict_get_quiet $cu_def {sptag}]]"
                        if {$jj < $jj_max} { set next "," } else { set next "" }; incr jj
    lappend CONFIG_JSON                     "$TAB_6\}$next"
                    }
    lappend CONFIG_JSON                 "$TAB_5\}"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
                if {$cu_configuration_power != {}} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3\}$next"
            }
            if {$cu_configuration_power != {}} {
    lappend CONFIG_JSON         "$TAB_3\"power\" : \{"
                set ii 0; set ii_max [expr [llength [dict keys $cu_configuration_power]] - 1]
                foreach {slr_idx power_def} $cu_configuration_power {
    lappend CONFIG_JSON             "$TAB_4[string2json $slr_idx] : \{"
    lappend CONFIG_JSON                 "$TAB_5[string2json {throttle_mode}] : [string2json [dict_get_quiet $power_def {throttle_mode}]]"
                    if {$ii < $ii_max} { set next "," } else { set next "" }; incr ii
    lappend CONFIG_JSON             "$TAB_4\}$next"
                }
    lappend CONFIG_JSON         "$TAB_3\}"
            }
            if {$is_cu_selection_section} { set next "," } else { set next "" }
    lappend CONFIG_JSON     "$TAB_2\}$next"
        }

        if {$is_cu_selection_section} {
    lappend CONFIG_JSON     "$TAB_2[string2json {cu_selection}] : \{"
            if {$cu_selection_power != {}} {
                if {($cu_selection_gt_mac != {}) || ($cu_selection_gt_lpbk != {}) || ($cu_selection_gt_prbs != {}) || ($cu_selection_gtf_prbs != {}) || ($cu_selection_gtm_prbs != {}) || ($cu_selection_gtyp_prbs != {}) || ($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {power}] : [list2json $cu_selection_power]$next"
            }
            if {$cu_selection_gt_mac != {}} {
                if {($cu_selection_gt_lpbk != {}) || ($cu_selection_gt_prbs != {}) || ($cu_selection_gtf_prbs != {}) || ($cu_selection_gtm_prbs != {}) || ($cu_selection_gtyp_prbs != {}) || ($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {gt_mac}] : [list2json $cu_selection_gt_mac]$next"
            }
            if {$cu_selection_gt_lpbk != {}} {
                if {($cu_selection_gt_prbs != {}) || ($cu_selection_gtf_prbs != {}) || ($cu_selection_gtm_prbs != {}) || ($cu_selection_gtyp_prbs != {}) || ($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {gt_lpbk}] : [list2json $cu_selection_gt_lpbk]$next"
            }
            if {$cu_selection_gt_prbs != {}} {
                if {($cu_selection_gtf_prbs != {}) || ($cu_selection_gtm_prbs != {}) || ($cu_selection_gtyp_prbs != {}) || ($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {gt_prbs}] : [list2json $cu_selection_gt_prbs]$next"
            }
            if {$cu_selection_gtf_prbs != {}} {
                if {($cu_selection_gtm_prbs != {}) || ($cu_selection_gtyp_prbs != {}) || ($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {gtf_prbs}] : [list2json $cu_selection_gtf_prbs]$next"
            }
            if {$cu_selection_gtm_prbs != {}} {
                if {($cu_selection_gtyp_prbs != {}) ||($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {gtm_prbs}] : [list2json $cu_selection_gtm_prbs]$next"
            }
            if {$cu_selection_gtyp_prbs != {}} {
                if {($cu_selection_memory != {})} { set next "," } else { set next "" }
    lappend CONFIG_JSON         "$TAB_3[string2json {gtyp_prbs}] : [list2json $cu_selection_gtyp_prbs]$next"
            }
            if {$cu_selection_memory != {}} {
    lappend CONFIG_JSON         "$TAB_3[string2json {memory}] : [strlist2json $cu_selection_memory]"
            }
    lappend CONFIG_JSON     "$TAB_2\}"
        }
        if {$hh < $hh_max} { set next "," } else { set next "" }; incr hh
    lappend CONFIG_JSON "$TAB_1\}$next"
    }
    lappend CONFIG_JSON "\}"

    # Save configuration
    if {$filename_in != {}} {
        set filename $filename_in
    } else {
        set filename [file join [dict get $config run_dir] ${cfg_name}.json]
    }
    write_file $filename [join $CONFIG_JSON "\n"]
}
############################################################################################################
# Report if parameter is auto-configured and store value
############################################################################################################
proc set_autoconfigured_param { config_ref cfg_name param_path0 param_value msg } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set param_path [concat [list $cfg_name] $param_path0]
    set param_name [join $param_path {.}]
    if {$param_value == {}} {
        log_message $config {XBTEST_WIZARD-9} [list $param_name $msg]
    } else {
        log_message $config {XBTEST_WIZARD-10} [list $param_name $param_value]; # report auto-configured value
    }
    dict_set config $param_path $param_value
}
############################################################################################################
# Check if parameter was provided in the <wizard_config_name> and "default" confugartion
############################################################################################################
proc merge_param_from_config { config_ref cfg_name config_path param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set param_path      [concat $config_path $param_path0]
    set param_name      [join $param_path {.}]
    set param_value     [dict_get_quiet $config $param_path]
    set param_path_cfg  [concat [list $cfg_name] $param_path0]
    if {[dict_exist $config $param_path]} {
        log_message $config {XBTEST_WIZARD-29} [list $param_name $param_value]
        dict_set config $param_path_cfg $param_value
        return 1
    }
    return 0
}
proc merge_param { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set wizard_config_name [dict_get_quiet $config wizard_config_name]
    if {[merge_param_from_config config $cfg_name [list wizard_config $wizard_config_name] $param_path0]} {
        return
    }
    if {[merge_param_from_config config $cfg_name {wizard_config default} $param_path0]} {
        return
    }
    if {[merge_param_from_config config $cfg_name {wizard_auto_config} $param_path0]} {
        return
    }
    set param_name [join $param_path0 {.}]
    log_message $config {XBTEST_WIZARD-31} [list $param_name]
}
proc merge_dict { config_ref cfg_name param_path0 } {
    upvar 1 $config_ref config; # Dictionary passed as ref.
    set wizard_config_name [dict_get_quiet $config wizard_config_name]
    # autoconfigured value
    set auto_param_path         [concat {wizard_auto_config} $param_path0]
    set auto_param_value        [dict_get_quiet $config $auto_param_path]
    # default configuration provided by user
    set default_param_path     [concat {wizard_config default} $param_path0]
    set default_param_value    [dict_get_quiet $config $default_param_path]
    # configuration selected by user
    set selected_param_path    [concat [list wizard_config $wizard_config_name] $param_path0]
    set selected_param_value   [dict_get_quiet $config $selected_param_path]

    # Merge the different configurations
    set param_value {}
    if {$auto_param_value != {}} {
        set param_value [dict merge $param_value $auto_param_value]
    }
    if {$default_param_value != {}} {
        set param_value [dict merge $param_value $default_param_value]
    }
    if {$selected_param_value != {}} {
        set param_value [dict merge $param_value $selected_param_value]
    }
    # Set merged value
    if {$param_value != {}} {
        set param_path  [concat [list $cfg_name] $param_path0]
        dict_set config $param_path $param_value
    }
}
############################################################################################################
# Get memory definition (sptag vs slr) in platform metadata
############################################################################################################
proc get_memory_xpfm_def { config_ref param_path0 memory_type } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set ret_val             {}
    set param_name          [join $param_path0 {.}]
    set platforminfo_path   [list platforminfo extensions raptor2 resources $memory_type]
    set sptag_key           {sptag}
    set slr_key             {slr}
    set index_key           {index}

    set hier0 [dict_get_quiet $config $platforminfo_path]
    if {$hier0 == {}} {
        log_message $config {XBTEST_WIZARD-28} [list $param_name $memory_type "path \{$platforminfo_path\} not found in platform metadata"]
        return $ret_val
    }

    foreach {hier0_key hier0_val} $hier0 {
        set sptag   [dict_get_quiet $hier0_val $sptag_key]
        set slr     [dict_get_quiet $hier0_val $slr_key]
        set index   [dict_get_quiet $hier0_val $index_key]

        if {$sptag == {}} {
            log_message $config {XBTEST_WIZARD-28} [list $param_name $memory_type "key ($sptag_key) not found in platform metadata at path \{$platforminfo_path $hier0_key\}"]
            set ret_val {}
            break
        }
        if {$slr == {}} {
            log_message $config {XBTEST_WIZARD-28} [list $param_name $memory_type "key ($slr_key) not found in platform metadata at path \{$platforminfo_path $hier0_key\}"]
            set ret_val {}
            break
        }
        if {$index != {}} {
            set sptag "$sptag\[$index\]"
        }
        if {![dict exist $ret_val $sptag]} {
            dict set ret_val $sptag $slr
        }
    }

    if {$ret_val == {}} {
        log_message $config {XBTEST_WIZARD-28} [list $param_name $memory_type "no $memory_type was found in platform metadata at path \{$platforminfo_path\}"]
    }
    return $ret_val
}

############################################################################################################
# Load the power floorplan JSON file. Do this only if a power CU is selected as it takes some time to load JSON files with json2dict.
############################################################################################################
proc load_power_floorplan_json { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    # if dynamic_geometry.json or utilization.json file is missing, use only 1 slice in power CU and stop after the opt phase so a DCP is available
    # Default
    dict set config C_DYNAMIC_GEOMETRY  {}
    dict set config C_UTILIZATION       {}
    dict set config C_INVALID           {PL_INVALID {}}

    set verify_slr      [dict_get_quiet $config {wizard_actual_config cu_configuration verify slr}]
    set verify_dna_read [dict_get_quiet $config {wizard_actual_config cu_configuration verify dna_read}]
    set power           [dict get $config wizard_actual_config cu_selection power]

    if {($power == {}) && ($verify_slr != {auto}) && ($verify_dna_read != {auto})} {
        return; # No power CU selected and no verify CU autoconfiguration needed: do not try to load the dynamic geometry JSON files
    }

    set dynamic_geometry_json [file join [dict get $config run_dir] pwr_floorplan dynamic_geometry.json]
    if {![file exists $dynamic_geometry_json]} {
        log_message $config {XBTEST_WIZARD-36} [list "Dynamic geometry JSON file" $dynamic_geometry_json]; # WARNING: file not found.
    } else {
        log_message $config {XBTEST_WIZARD-37} [list "dynamic geometry JSON file" $dynamic_geometry_json]
        set C_DYNAMIC_GEOMETRY_JSON [read_file $dynamic_geometry_json]
        dict set config C_DYNAMIC_GEOMETRY [json2dict $C_DYNAMIC_GEOMETRY_JSON]

        # Check only keys present at top level
        set supported_keys {COMMENT PART PBLOCKS SLR CLOCK_REGIONS SITES_TYPES CONFIG_SITES PL_DYNAMIC_GEOMETRY AIE_TYPE AIE_GEOMETRY}
        foreach key [dict keys [dict get $config C_DYNAMIC_GEOMETRY]] {
            set KEY [string toupper $key]
            if {$KEY ni $supported_keys} {
                log_message $config {XBTEST_WIZARD-30} [list {dynamic_geometry.json} $key]
            }
        }
    }

    if {$power == {}} {
        return; # No power CU selected: do not try to load the other power floorplan JSON files
    }

    set utilization_json [file join [dict get $config run_dir] pwr_floorplan utilization.json]
    if {![file exists $utilization_json]} {
        log_message $config {XBTEST_WIZARD-36} [list "Power CU utilization JSON file" $utilization_json]; # WARNING: file not found.
    } else {
        log_message $config {XBTEST_WIZARD-37} [list "power CU utilization JSON file" $utilization_json]
        set C_UTILIZATION_JSON [read_file $utilization_json]
        dict set config C_UTILIZATION [json2dict $C_UTILIZATION_JSON]

        # Check only keys present at top level
        set supported_keys {COMMENT PL_UTILIZATION AIE_UTILIZATION}
        foreach key [dict keys [dict get $config C_UTILIZATION]] {
            set KEY [string toupper $key]
            if {$KEY ni $supported_keys} {
                log_message $config {XBTEST_WIZARD-30} [list {utilization.json} $key]
            }
        }
    }
    # Optional
    set invalid_json [file join [dict get $config run_dir] pwr_floorplan invalid.json]
    if {[file exists $invalid_json]} {
        log_message $config {XBTEST_WIZARD-37} [list "power CU invalid sites JSON file" $invalid_json]
        set C_INVALID_JSON [read_file $invalid_json]
        dict set config C_INVALID [json2dict $C_INVALID_JSON]

        # Check only keys present at top level
        set supported_keys {COMMENT PL_INVALID}
        foreach key [dict keys [dict get $config C_INVALID]] {
            set KEY [string toupper $key]
            if {$KEY ni $supported_keys} {
                log_message $config {XBTEST_WIZARD-30} [list {invalid.json} $key]
            }
        }
    }
}

############################################################################################################
# Set-up verify CU
############################################################################################################
proc setup_verify_cu { config_ref } {
    upvar 1 $config_ref config; # Dictionary passed as ref.

    set slr             [dict_get_quiet $config {wizard_actual_config cu_configuration verify slr}]
    set dna_read        [dict_get_quiet $config {wizard_actual_config cu_configuration verify dna_read}]
    set CONFIG_SITES    [dict_get_quiet $config {C_DYNAMIC_GEOMETRY CONFIG_SITES}]

    if {$CONFIG_SITES == {}} {
        set slr_indexes {}
    } else {
        set slr_indexes [dict keys $CONFIG_SITES]
    }

    if {$slr == {auto}} {
        if {[llength $slr_indexes] > 0} {
            set slr SLR[lindex $slr_indexes 0]
            log_message $config {XBTEST_WIZARD-41} [list $slr {a CONFIG_SITE was found in dynamic_geometry.json during auto-configuraton}]
        } else {
            set slr SLR0
            log_message $config {XBTEST_WIZARD-41} [list $slr {no CONFIG_SITE was found in dynamic_geometry.json during auto-configuraton}]
        }

        if {$dna_read == {auto}} {

            if {[llength $slr_indexes] == 0} {
                log_message $config {XBTEST_WIZARD-42} [list {disabled} {no CONFIG_SITE was found in dynamic_geometry.json during auto-configuration}]
                set dna_read false
            } else {
                log_message $config {XBTEST_WIZARD-42} [list {enabled} {a CONFIG_SITE was found in dynamic_geometry.json during auto-configuration}]
                set dna_read true
            }

        } elseif {$dna_read} {

            log_message $config {XBTEST_WIZARD-42} [list {enabled} {configuration was provided by user}]

            if {[llength $slr_indexes] == 0} {
                log_message $config {XBTEST_WIZARD-43} [list "for any SLR"]; # CRITICAL WARNING: DNA read enabled but no CONFIG_SITE
            }

        } else {

            log_message $config {XBTEST_WIZARD-42} [list {disabled} {configuration was provided by user}]

        }
    } else {
        log_message $config {XBTEST_WIZARD-41} [list $slr {SLR location was provided by user}]

        regexp {^SLR([0-9])$} $slr -> slr_idx
        if {$dna_read == {auto}} {

            if {$slr_idx ni $slr_indexes} {
                log_message $config {XBTEST_WIZARD-42} [list {disabled} "no CONFIG_SITE was found in dynamic_geometry.json during auto-configuration for SLR provided by user: $slr"]
                set dna_read false
            } else {
                log_message $config {XBTEST_WIZARD-42} [list {enabled} "a CONFIG_SITE was found in dynamic_geometry.json during auto-configuration for SLR provided by user: $slr"]
                set dna_read true
            }

        } elseif {$dna_read} {

            log_message $config {XBTEST_WIZARD-42} [list {enabled} {configuration was provided by user}]

            if {$slr_idx ni $slr_indexes} {
                log_message $config {XBTEST_WIZARD-43} [list "for SLR provided by user: $slr"]; # CRITICAL WARNING: DNA read enabled but no CONFIG_SITE
            }

        } else {

            log_message $config {XBTEST_WIZARD-42} [list {disabled} {configuration was provided by user}]

        }
    }
    dict set config wizard_actual_config cu_configuration verify slr        $slr
    dict set config wizard_actual_config cu_configuration verify dna_read   $dna_read
}
