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

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))

from pkg import *


# Generate bash script which will copy deployment files to the desired location on host
def create_aved_install_bash(opt, config):

    config['steps']['deploy']['aved_install_sh'] = os.path.abspath(os.path.join(config['steps']['deploy']['dir'], 'aved_install.sh'))

    if is_current_step_run(config):

        AVED_INSTALL_SH = []
        AVED_INSTALL_SH += ['#! /bin/bash']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['DEST_DIR="/opt/amd/aved/'+config['design']['vbnv']+'_'+config['design']['application']+'"']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['if [ $(grep \'^NAME\' /etc/os-release | grep -c "Ubuntu") -eq 1 ]; then']
        AVED_INSTALL_SH += ['    if [ $(grep \'^VERSION_ID\' /etc/os-release | grep -c "20.04") -eq 1 ]; then']
        AVED_INSTALL_SH += ['       PKG="20.04.deb"']
        AVED_INSTALL_SH += ['       PKG_MGR="apt"']
        AVED_INSTALL_SH += ['    elif [ $(grep \'^VERSION_ID\' /etc/os-release | grep -c "22.04")  -eq 1 ]; then']
        AVED_INSTALL_SH += ['       PKG="22.04.deb"']
        AVED_INSTALL_SH += ['       PKG_MGR="apt"']
        AVED_INSTALL_SH += ['    else']
        AVED_INSTALL_SH += ['      echo "Unsupported version of Ubuntu"']
        AVED_INSTALL_SH += ['    fi']
        AVED_INSTALL_SH += ['elif [ $(grep \'^NAME\' /etc/os-release | grep -c "Red Hat") -eq 1 ]; then']
        AVED_INSTALL_SH += ['   if [ $(grep \'^VERSION_ID\' /etc/os-release | grep -c "8.") -eq 1 ]; then']
        AVED_INSTALL_SH += ['       PKG="rpm"']
        AVED_INSTALL_SH += ['       PKG_MGR="yum"']
        AVED_INSTALL_SH += ['    else']
        AVED_INSTALL_SH += ['      echo "Unsupported version of Red Hat"; exit']
        AVED_INSTALL_SH += ['    fi']
        AVED_INSTALL_SH += ['else']
        AVED_INSTALL_SH += ['   echo "Unsupported OS"']
        AVED_INSTALL_SH += ['   echo "Only Ubuntu 20.04, Ubuntu 22.04 & Red Hat 8.* are supported"; exit']
        AVED_INSTALL_SH += ['fi']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo "Removing previous AVED installations"']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['if [ $(grep \'^NAME\' /etc/os-release | grep -c "Ubuntu") -eq 1 ]; then']
        AVED_INSTALL_SH += ['   if [ $(apt list | grep -c ^xbtest/now) -eq 1 ]; then']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo "Removing previous installation of XBTEST"']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       $PKG_MGR remove xbtest -y']
        AVED_INSTALL_SH += ['    fi']
        AVED_INSTALL_SH += ['   if [ $(apt list | grep -c ^ami/now) -eq 1 ]; then']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo "Removing previous installation of AMI"']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       $PKG_MGR remove ami -y']
        AVED_INSTALL_SH += ['    fi']
        AVED_INSTALL_SH += ['fi']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['if [ $(grep \'^NAME\' /etc/os-release | grep -c "Red Hat") -eq 1 ]; then']
        AVED_INSTALL_SH += ['   if [ $(yum list | grep -c ^xbtest.x86) -eq 1 ]; then']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo "Removing previous installation of XBTEST"']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       $PKG_MGR remove xbtest -y']
        AVED_INSTALL_SH += ['    fi']
        AVED_INSTALL_SH += ['   if [ $(yum list | grep -c ^ami.x86) -eq 1 ]; then']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo "Removing previous installation of AMI"']
        AVED_INSTALL_SH += ['       echo "=========================================================================="']
        AVED_INSTALL_SH += ['       echo " "']
        AVED_INSTALL_SH += ['       $PKG_MGR remove ami -y']
        AVED_INSTALL_SH += ['    fi']
        AVED_INSTALL_SH += ['fi']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo "Removing previous installation of AVED HW "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['rm -rf               $DEST_DIR']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo "Installing New AVED deployment"']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['mkdir -p             $DEST_DIR']
        AVED_INSTALL_SH += ['cp -L    *.pdi       $DEST_DIR']
        AVED_INSTALL_SH += ['cp -L    *.xsa       $DEST_DIR']
        AVED_INSTALL_SH += ['cp -L    *.json      $DEST_DIR']
        AVED_INSTALL_SH += ['cp -rL   flash_setup $DEST_DIR']
        if "xbtest" in config['design']['application']:
            AVED_INSTALL_SH += ['cp -rL   xbtest      $DEST_DIR']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo "AVED Deployment files copied to $DEST_DIR"']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo "Installing AMI & XBTEST"']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['$PKG_MGR install ./sw_packages/*$PKG -y']
        AVED_INSTALL_SH += ['']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo "AVED Installation Complete"']
        AVED_INSTALL_SH += ['echo "=========================================================================="']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo " "']
        AVED_INSTALL_SH += ['echo " "']

        log_info('GEN_DESIGN-51', 'Create AVED Install script: ' + config['steps']['deploy']['aved_install_sh'])
        with open(config['steps']['deploy']['aved_install_sh'], 'w') as f:
            f.write('\n'.join(AVED_INSTALL_SH))

        os.chmod(config['steps']['deploy']['aved_install_sh'], 0o755)

    check_output_file_exists(config['steps']['deploy']['aved_install_sh'])


def create_version_json(opt, config):

    config['steps']['deploy']['version_json'] = os.path.abspath(os.path.join(config['steps']['deploy']['dir'], 'version.json'))
    config['steps']['deploy']['deploy_release']  = str(config['now'].year) + str(config['now'].month).zfill(2) + str(config['now'].day).zfill(2)

    if is_current_step_run(config):

        VERSION_JSON  = ['{']
        VERSION_JSON += ['    "design": {']
        VERSION_JSON += ['        "name"        : "' + config['design']['vbnv'] + '",']
        VERSION_JSON += ['        "release"     : "' + config['steps']['deploy']['deploy_release'] + '",']
        VERSION_JSON += ['        "logic_uuid"  : "' + config['steps']['hw']['logic_uuid'] + '",']
        VERSION_JSON += ['        "application" : "' + config['design']['application'] + '"']
        VERSION_JSON += ['     }']
        VERSION_JSON += ['}']

        log_info('GEN_DESIGN-51', 'Create version.json: ' + config['steps']['deploy']['version_json'])
        with open(config['steps']['deploy']['version_json'], 'w') as f:
            f.write('\n'.join(VERSION_JSON))

        os.chmod(config['steps']['deploy']['version_json'], 0o755)

    check_output_file_exists(config['steps']['deploy']['version_json'])

def create_version_full(opt, config):

    base_folder = os.path.abspath(os.path.join(config['script_dir']))

    #Read build output version json, append and overwrite - check file exists - fill with empty if not
    with open(config['steps']['deploy']['version_json'], 'r') as f1:
        version_main = json.load(f1)

    hw_dir = os.path.abspath(os.path.join(base_folder, 'hw'))
    if os.path.isfile(f'{hw_dir}/hw_version.json'):
        with open(f'{hw_dir}/hw_version.json', 'r') as f2:
            hw_version = json.load(f2)
    else:
        version_string = f'{{"hw_cl" : "unknown"}}'
        hw_version = json.loads(version_string)

    xbtest_sw_dir = os.path.abspath(os.path.join(base_folder, 'sw', 'xbtest'))
    if os.path.isfile(f'{xbtest_sw_dir}/xbtest_sw_version.json'):
        with open(f'{xbtest_sw_dir}/xbtest_sw_version.json', 'r') as f3:
            xbtest_sw_version = json.load(f3)
    else:
        version_string = f'{{"xbtest_sw_cl" : "unknown"}}'
        xbtest_sw_version = json.loads(version_string)

    ami_dir = os.path.abspath(os.path.join(base_folder, 'sw', 'AMI'))
    if os.path.isfile(f'{ami_dir}/merge_ami_version.json'):
        with open(f'{ami_dir}/merge_ami_version.json', 'r') as f4:
            ami_json_data = json.load(f4)
    else:
        version_string = f'{{"ami_version" : "unknown"}}'
        ami_json_data = json.loads(version_string)

    amc_dir = os.path.abspath(os.path.join(base_folder, 'fw', 'AMC'))
    if os.path.isfile(f'{amc_dir}/merge_amc_version.json'):
        with open(f'{amc_dir}/merge_amc_version.json', 'r') as f5:
            amc_json_data = json.load(f5)
    else:
        version_string = f'{{"amc_version" : "unknown"}}'
        amc_json_data = json.loads(version_string)

    ami_json = {
        'ami_version':ami_json_data
    }

    amc_json = {
        'amc_version':amc_json_data
    }

    full_version = {**version_main, **hw_version, **xbtest_sw_version, **ami_json, **amc_json}

    with open('version.json', 'w') as file:
        json.dump(full_version, file, indent=4)
    os.chmod(config['steps']['deploy']['version_json'], 0o555)


# Gather outputs from different steps of AVED build and pull them together into an AVED deployment archive
def create_deployment_zip(opt, config):

    check_input_file_exists(config['steps']['pdi']['design_pdi'])
    check_input_file_exists(config['steps']['pdi']['fpt_setup_pdi'])
    check_input_file_exists(config['steps']['hw']['hw_pdi'])
    check_input_file_exists(config['steps']['hw']['static_xsa'])
    check_input_file_exists(config['steps']['deploy']['version_json'])

    if "xbtest" in config['design']['application']:
        check_input_file_exists(config['steps']['hw']['xbtest_metadata_json'])
        check_input_file_exists(config['steps']['hw']['xbtest_user_metadata_json'])
        check_input_file_exists(config['steps']['hw']['xbtest_pfm_def_metadata_json'])
        check_input_dir_exists(config['steps']['hw']['xbtest_test_json_dir'])


    config['steps']['deploy']['boot_mode_tcl']   = os.path.abspath(os.path.join(config['deploy_src_dir'], 'scripts', 'versal_change_boot_mode.tcl'))
    config['steps']['deploy']['sw_packages_dir'] = os.path.abspath(os.path.join(config['deploy_src_dir'], 'sw_packages'))

    config['steps']['deploy']['deploy_name']     = config['design']['vbnv'] + '_' + config['design']['application'] + '_' + config['steps']['deploy']['deploy_release']
    config['steps']['deploy']['deploy_zip_name'] = config['steps']['deploy']['deploy_name'] + '.zip'
    config['steps']['deploy']['deploy_zip']      = os.path.abspath(os.path.join(config['steps']['deploy']['dir'], config['steps']['deploy']['deploy_zip_name']))



    if is_current_step_run(config):

        with zipfile.ZipFile(config['steps']['deploy']['deploy_zip'], 'w') as zip_ref:
            # Add install shell script to the zip
            zip_ref.write(config['steps']['deploy']['aved_install_sh'], config['steps']['deploy']['deploy_name'] +"//"+ os.path.basename(config['steps']['deploy']['aved_install_sh']))
            # Add Design PDI to the zip
            zip_ref.write(config['steps']['pdi']['design_pdi'], config['steps']['deploy']['deploy_name'] +"//"+ os.path.basename(config['steps']['pdi']['design_pdi']))
            # Add Static XSA to the zip
            zip_ref.write(config['steps']['hw']['static_xsa'], config['steps']['deploy']['deploy_name'] +"//"+ os.path.basename(config['steps']['hw']['static_xsa']))
            # Add files needed for Flash setup to the zip
            zip_ref.write(config['steps']['pdi']['fpt_setup_pdi'], config['steps']['deploy']['deploy_name'] + "//flash_setup//"+os.path.basename(config['steps']['pdi']['fpt_setup_pdi']))
            zip_ref.write(config['steps']['hw']['hw_pdi'], config['steps']['deploy']['deploy_name'] + "//flash_setup//"+str(config['board']['name'])+"_initialization.pdi")
            zip_ref.write(config['steps']['deploy']['boot_mode_tcl'], config['steps']['deploy']['deploy_name'] + "//flash_setup//"+os.path.basename(config['steps']['deploy']['boot_mode_tcl']))
            # Add Version JSON file to zip
            zip_ref.write(config['steps']['deploy']['version_json'], config['steps']['deploy']['deploy_name'] +"//"+ os.path.basename(config['steps']['deploy']['version_json']))

            if "xbtest" in config['design']['application']:
                # Add metadata used by xbtest to the zip
                zip_ref.write(config['steps']['hw']['xbtest_metadata_json'], config['steps']['deploy']['deploy_name'] + "//xbtest//metadata//"+os.path.basename(config['steps']['hw']['xbtest_metadata_json']))
                zip_ref.write(config['steps']['hw']['xbtest_pfm_def_metadata_json'], config['steps']['deploy']['deploy_name'] + "//xbtest//metadata//"+os.path.basename(config['steps']['hw']['xbtest_pfm_def_metadata_json']))
                zip_ref.write(config['steps']['hw']['xbtest_user_metadata_json'], config['steps']['deploy']['deploy_name'] + "//xbtest//metadata//"+os.path.basename(config['steps']['hw']['xbtest_user_metadata_json']))

                for dirpath,dirs,files in os.walk(config['steps']['hw']['xbtest_test_json_dir']):
                    for f in files:
                        fn = os.path.join(dirpath, f)
                        zip_ref.write(fn, config['steps']['deploy']['deploy_name'] + "//xbtest//test//"+os.path.basename(fn))

            # Add any pre built SW packages to the zip
            for dirpath,dirs,files in os.walk(config['steps']['deploy']['sw_packages_dir']):
                for f in files:
                    fn = os.path.join(dirpath, f)
                    zip_ref.write(fn, config['steps']['deploy']['deploy_name'] + "//sw_packages//"+os.path.basename(fn))


        log_info('GEN_DESIGN-52', 'Generate AVED Deployment Archive: ' + config['steps']['deploy']['deploy_zip'])

    check_output_file_exists(config['steps']['deploy']['deploy_zip'])



# Main function of deploy step
def run_step_deploy(opt, config):
    start_step(opt, config, 'deploy')

    create_aved_install_bash(opt, config)
    create_version_json(opt, config)
    create_version_full(opt, config)
    create_deployment_zip(opt, config)

    end_step(opt, config)
