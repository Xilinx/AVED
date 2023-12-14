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

# Script that generates compile_ps_kernel.sh from input configuration file
proc gen_compile_ps_kernel_sh { config_ref } {
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

    set compile_ps_kernel_sh [file join [dict get $config run_dir] compile_ps_kernel.sh]
    log_message $config {GEN_COMPILE_PS_KERNEL_SH-1} [list $compile_ps_kernel_sh]; # Start

    set PS_CU_NAMES {}

    foreach cu_cfg [dict get $config cu_config] {
        set krnl_mode   [dict get $cu_cfg krnl_mode]

        if {$krnl_mode == $C_KRNL_MODE_POWER} {
            set use_aie [dict get $cu_cfg use_aie]
            if {$use_aie} {
                set aie_control [dict get $cu_cfg aie_control]
                if {$aie_control == {ps}} {
                    lappend PS_CU_NAMES krnl_powertest_ps
                }
            }
        }
    }

    if {[llength $PS_CU_NAMES] == 0} {
        log_message $config {GEN_COMPILE_PS_KERNEL_SH-2}; # Not applicable
        return
    }

    set     COMPILE_PS_KERNEL_SH {}
    lappend COMPILE_PS_KERNEL_SH "#!/bin/bash"
    lappend COMPILE_PS_KERNEL_SH "# This is a generated file. Use and modify at your own risk."
    lappend COMPILE_PS_KERNEL_SH "XBTEST_RUN_DIR=[dict get $config run_dir]"
    lappend COMPILE_PS_KERNEL_SH "XBTEST_OUTPUT_DIR=[dict get $config output_dir]"
    lappend COMPILE_PS_KERNEL_SH "XBTEST_IP_CATALOG=[dict get $config ip_catalog]"
    lappend COMPILE_PS_KERNEL_SH "XBTEST_IPDEF=\${XBTEST_IP_CATALOG}/[dict get $config ip_name_v]"
    lappend COMPILE_PS_KERNEL_SH "XBTEST_PS_SRC=\${XBTEST_IPDEF}/ps"

    lappend COMPILE_PS_KERNEL_SH "# Environment setup"
    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_env_var_bash        XILINX_VITIS            "Compile PS kernels"]]
    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_dir_bash        "\${XILINX_VITIS}" true     "Compile PS kernels"]]
    lappend COMPILE_PS_KERNEL_SH "source \${XILINX_VITIS}/settings64.sh"

    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [set_default_env_var_bash           XILINX_VITIS_AIETOOLS "\${XILINX_VITIS}/aietools"]]
    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_dir_bash                 "\${XILINX_VITIS_AIETOOLS}" true "Compile AIE kernels"]]

    # TODO: upon release, switch from /proj/xbuilds/ to public GitHub repo
    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_env_var_bash REL "Compile PS kernels"]]
    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [set_default_env_var_bash        SYSROOT "/proj/xbuilds/\${REL}/internal_platforms/sw/versal/xilinx-versal/sysroots/aarch64-xilinx-linux"]]
    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_dir_bash              "\${SYSROOT}" true "Compile AIE kernels"]]

    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [set_default_env_var_bash CROSSCXX "\${XILINX_VITIS}/gnu/aarch64/lin/aarch64-linux/bin/aarch64-linux-gnu-g++"]]

    lappend COMPILE_PS_KERNEL_SH "CROSSCFLAGS=\"-I\${SYSROOT}/usr/include/xrt -I\${SYSROOT} -I\${XILINX_VITIS_AIETOOLS}/include -Wall -O2 -g -fPIC -fvisibility=hidden -lrt --sysroot=\${SYSROOT}\""
    lappend COMPILE_PS_KERNEL_SH "CROSSLDFLAGS=\"-shared -lxrt_core -L\${SYSROOT}/usr/lib -lxilinxopencl -lpthread -lrt -ldl -lcrypt -lstdc++ --sysroot=\${SYSROOT}\""

    set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [get_bash_delimiter]]

    #############################################################################################################
    # Set PS kernels cross-compilation commands
    #############################################################################################################

    foreach ps_cu_name $PS_CU_NAMES {
        set ps_cu_o [file join [dict get $config run_dir] ${ps_cu_name}.o]
        set ps_cu_so [file join [dict get $config output_dir] ${ps_cu_name}.so]
        set sh_ps_cu_c  "\${XBTEST_PS_SRC}/${ps_cu_name}/src/${ps_cu_name}.c"
        set sh_ps_cu_o  "\${XBTEST_OUTPUT_DIR}/${ps_cu_name}.o"
        set sh_ps_cu_so "\${XBTEST_OUTPUT_DIR}/${ps_cu_name}.so"

        log_message $config {GEN_COMPILE_PS_KERNEL_SH-3} [list $ps_cu_name $ps_cu_so]


        lappend COMPILE_PS_KERNEL_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "Generating PS kernel $ps_cu_name"]
        lappend COMPILE_PS_KERNEL_SH "\${CROSSCXX} -c -I\${XBTEST_PS_SRC}/${ps_cu_name}/src \${CROSSCFLAGS} -o $sh_ps_cu_o $sh_ps_cu_c"
        set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_ret_code_bash   "\${CROSSCXX} -c"]]
        set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_file_bash       $sh_ps_cu_o]]

        lappend COMPILE_PS_KERNEL_SH "\${CROSSCXX} $sh_ps_cu_o \${CROSSLDFLAGS} -o $sh_ps_cu_so"
        set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_ret_code_bash   "\${CROSSCXX}"]]
        set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [check_file_bash       $sh_ps_cu_so]]

        lappend COMPILE_PS_KERNEL_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "PS kernel $ps_cu_name compiled successfully: $sh_ps_cu_so"]

        set     COMPILE_PS_KERNEL_SH [concat $COMPILE_PS_KERNEL_SH [get_bash_delimiter]]
    }

    #############################################################################################################
    # End of build_xclbin.sh
    #############################################################################################################
    lappend COMPILE_PS_KERNEL_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "All PS kernels compiled successfully"]
    lappend COMPILE_PS_KERNEL_SH [log_message_bash {INFO} {XBTEST_WIZARD-27} "Done!"]
    lappend COMPILE_PS_KERNEL_SH "exit 0"; # Return 0 if success

    log_message $config {GEN_COMPILE_PS_KERNEL_SH-4} [list $compile_ps_kernel_sh]
    write_file $compile_ps_kernel_sh [join $COMPILE_PS_KERNEL_SH "\n"]
    file attribute $compile_ps_kernel_sh -permissions {a+x}; # Add executable permission to everyone (raptor)


}
