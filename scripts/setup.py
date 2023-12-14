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

def load_config_json(opt, config):
    log_debug('GEN_DESIGN-17', 'Loading configuration JSON file: ' + opt.conf_json)
    fdata = json_load('configuration JSON file', opt.conf_json)

    # Merge configuration JSOPN file content in main config dictionary
    for k,v in fdata.items():
        config[k] = v

    # TODO check conf.json required parameters defined

    config['design']['vbnv']  = '_'.join([config['vendor']['name'],config['board']['name'],config['design']['name'],config['design']['version']])

def create_output_dir(opt, config):
    # Get/define output directory
    if opt.output_dir is not None:
        config['output_dir'] = opt.output_dir
    else:
#        config['output_dir'] = str(config['design']['vbnv'])
#        config['output_dir'] += '_' + '-'.join([str(config['now'].year)         , str(config['now'].month).zfill(2) , str(config['now'].day).zfill(2)])
#        config['output_dir'] += '_' + '-'.join([str(config['now'].hour).zfill(2), str(config['now'].minute).zfill(2), str(config['now'].second).zfill(2)])
#        config['output_dir'] = os.path.abspath(os.path.join(config['user_cwd'], 'output', config['output_dir']))

        config['output_dir'] =        '-'.join([str(config['now'].year)         , str(config['now'].month).zfill(2) , str(config['now'].day).zfill(2)])
        config['output_dir'] += '_' + '-'.join([str(config['now'].hour).zfill(2), str(config['now'].minute).zfill(2), str(config['now'].second).zfill(2)])
        config['output_dir'] = os.path.abspath(os.path.join(config['user_cwd'], 'output', config['design']['vbnv'], config['output_dir']))


    if ((opt.from_step is None) and (opt.single_step is None)):
        # Create output directory, override it if force option is provided
        if os.path.isdir(config['output_dir']):
           if not opt.force:
               exit_error('GEN_DESIGN-14', 'Output directory already exists (see --force to override): ' + config['output_dir'])
           else:
               log_info('GEN_DESIGN-15', 'Removing output directory already existing as --force option is provided: ' + config['output_dir'])
               force_remove_dir(config['output_dir'])

        log_info('GEN_DESIGN-16', 'Creating output directory: ' + config['output_dir'])
        os.makedirs(config['output_dir'])


def start_logging(config):
    config['log_dir'] = os.path.abspath(os.path.join(config['output_dir'], 'log'))

    if not os.path.isdir(config['log_dir']):
        os.makedirs(config['log_dir'])

    setup_logfile(os.path.abspath(os.path.join(config['log_dir'], os.path.splitext(config['script_name'])[0] + '.log')))


def backup_sources(opt, config):
    config['bkp_design_dir']  = os.path.abspath(os.path.join(config['output_dir'], 'bkp_design'))

    config['script_file']       = os.path.abspath(os.path.join(config['bkp_design_dir'], config['script_name']))
    config['conf_json']         = os.path.abspath(os.path.join(config['bkp_design_dir'], 'conf.json'))
    config['scripts_src_dir']   = os.path.abspath(os.path.join(config['bkp_design_dir'], 'scripts'))
    config['hw_src_dir']        = os.path.abspath(os.path.join(config['bkp_design_dir'], 'hw'))
    config['fw_src_dir']        = os.path.abspath(os.path.join(config['bkp_design_dir'], 'fw'))
    config['deploy_src_dir']    = os.path.abspath(os.path.join(config['bkp_design_dir'], 'deploy'))    

    # Do not backup sources if from_step or step option provided
    if (opt.from_step is None) and (opt.single_step is None):
        log_info('GEN_DESIGN-18', 'Saving sources in directory: ' + config['bkp_design_dir'])

        os.makedirs(config['bkp_design_dir'])

        shutil.copyfile(os.path.abspath(os.path.join(config['script_dir'], config['script_name'])), config['script_file'])
        os.chmod(config['script_file'], 0o755)

        shutil.copyfile(opt.conf_json, config['conf_json'])

        os.makedirs(config['scripts_src_dir'])
        os.makedirs(config['hw_src_dir'])
        os.makedirs(config['fw_src_dir'])
        copy_source_dir(os.path.abspath(os.path.join(config['script_dir'],'hw',config['design']['vbnv'])), config['hw_src_dir'])
        copy_source_dir(os.path.abspath(os.path.join(config['script_dir'],'fw')), config['fw_src_dir'])
        copy_source_dir(os.path.abspath(os.path.join(config['script_dir'],'scripts')), config['scripts_src_dir'])
        copy_source_dir(os.path.abspath(os.path.join(config['script_dir'],'deploy')), config['deploy_src_dir'])

    check_output_file_exists(config['script_file'])
    check_output_file_exists(config['conf_json'])
    check_output_dir_exists(config['scripts_src_dir'])
    check_output_dir_exists(config['hw_src_dir'])
    check_output_dir_exists(config['fw_src_dir'])


def write_export_script(opt, config):
    export_sh = os.path.abspath(os.path.join(config['output_dir'], 'export.sh'))

    output_dir_base = os.path.basename(config['output_dir'])

    fdata = []
    fdata += ['#!/bin/bash']
    fdata += ['']
    fdata += ['EXPORT="export_' + output_dir_base + '"']
    fdata += ['BKP_DESIGN_DIR="' + config['bkp_design_dir'] + '"']
    fdata += ['OUTPUT_DIR="' + config['output_dir'] + '"']
    fdata += ['']
    fdata += ['echo "Exporting build run to directory: ./${EXPORT}"']
    fdata += ['mkdir -p ${EXPORT}']
    fdata += ['mkdir -p ${EXPORT}/' + output_dir_base]
    fdata += ['cp -rL ${BKP_DESIGN_DIR}/* ${EXPORT}']
    fdata += ['cp -rL ${OUTPUT_DIR}/*     ${EXPORT}/' + output_dir_base]

    fdata += ['chmod a+x ${EXPORT}/'+config['script_name']]

    fdata += ['']
    fdata += ['cat <<EOF > ./${EXPORT}/rerun.sh']
    fdata += ['#\!/bin/bash']
    fdata += ['']
    fdata += ['# Rerun ' + config['script_name']]
    fdata += ['']
    fdata += ['./' + config['script_name'] + ' \\']
    fdata += ['\t --output_dir "rerun_' + os.path.basename(config['output_dir']) + '"\\ ']
    if opt.verbose:
        fdata += ['    --verbose \\ ']
    if opt.lsf_cmd is not None:
        fdata += ['    --lsf_cmd "' + opt.lsf_cmd + '"\\ ']
    if opt.use_lsf:
        fdata += ['    --use_lsf \\ ']

    fdata += ['']
    fdata += ['EOF']
    fdata += ['chmod a+x ${EXPORT}/rerun.sh']
    fdata += ['']
    fdata += ['echo "Creating archive: ./${EXPORT}.tar.gz"']
    fdata += ['tar -czf ${EXPORT}.tar.gz ./${EXPORT}']
    fdata += ['']
    fdata += ['echo "Successfully exported: ./${EXPORT}.tar.gz"']
    fdata += ['']
    fdata += ['echo "Re-run using following commands:"']
    fdata += ['echo "    tar -xf ./${EXPORT}.tar.gz"']
    fdata += ['echo "    cd ${EXPORT}"']
    fdata += ['echo "    ./rerun.sh"']

    log_info('GEN_DESIGN-70', 'Writing: ' + export_sh)
    with open(export_sh, mode='w') as outfile:
        outfile.write('\n'.join(fdata))

    os.chmod(export_sh, 0o755)


def set_up(opt, config):
    load_config_json        (opt, config)
    create_output_dir       (opt, config)
    start_logging           (config)
    backup_sources          (opt, config)
    write_export_script     (opt, config)
