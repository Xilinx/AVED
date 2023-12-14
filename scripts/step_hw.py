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

# Use Vivado to generated the PDI. From HW sources, the build_hw.tcl script will:
#    - Create a BD design
#    - Run Synthesis and implementation
#    - Write the device image (PDI)
#    - Finally generate a static XSA needed to build the FW image.
def build_hw(opt, config):
    config['steps']['hw']['build_hw_tcl']    = os.path.abspath(os.path.join(config['hw_src_dir'], 'build_hw.tcl'))
    config['steps']['hw']['hw_ip_repo']      = os.path.abspath(os.path.join(config['hw_src_dir'], 'iprepo'))

    check_input_file_exists(config['steps']['hw']['build_hw_tcl'])
    check_input_dir_exists(config['steps']['hw']['hw_ip_repo'])

    config['steps']['hw']['hw_pdi']                     = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'prj', 'prj.runs', 'impl_1', 'top_wrapper.pdi'))
    config['steps']['hw']['uuid_dict']                  = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'prj', 'prj.runs', 'impl_1', 'pfm_uuid_manifest.dict'))
    config['steps']['hw']['static_xsa']                 = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'static.xsa'))

    # Create directories for xbtest/test & xbtest/metadata so file copies at end of hw build stage can be performed
    config['steps']['hw']['xbtest_test_json_dir']          = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'xbtest', 'test'))
    config['steps']['hw']['xbtest_test_metadata_json_dir'] = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'xbtest', 'metadata'))

    if is_current_step_run(config):
        # Only create these directories if the hw step is being run/reran.  If running --from_step <fw, pdi, etc>, do create these directories 
        os.makedirs(config['steps']['hw']['xbtest_test_json_dir'])
        os.makedirs(config['steps']['hw']['xbtest_test_metadata_json_dir'])

        vivado_jou = os.path.abspath(os.path.join(config['log_dir'], 'build_hw_vivado.jou'))
        vivado_log = os.path.abspath(os.path.join(config['log_dir'], 'build_hw_vivado.log'))

        cmd = [ 'vivado',   '-notrace',
                            '-mode',        'batch',
                            '-journal',     vivado_jou,
                            '-log',         vivado_log,
                            '-source',      config['steps']['hw']['build_hw_tcl'],
                            '-tclargs',     '-s', config['steps']['hw']['static_xsa'],
                                            '-m', config['steps']['hw']['xbtest_test_metadata_json_dir'],
                                            '-c', config['conf_json'],
                                            '-i', config['steps']['hw']['hw_ip_repo'],
        ]
        if opt.gui:
            cmd.extend(['-g', '1'])
        cmd = ' '.join(cmd)
        if opt.lsf_cmd is not None:
            cmd = opt.lsf_cmd + ' "' + cmd + '"'
    
        log_info('GEN_DESIGN-43', 'Run vivado to build hardware PDI: ' + config['steps']['hw']['hw_pdi'])
        exec_step_cmd(opt, config, cmd, log_file_name=os.path.abspath(os.path.join(config['log_dir'], 'build_hw.log')), shell=True)

    check_output_file_exists(config['steps']['hw']['hw_pdi'])
    check_output_file_exists(config['steps']['hw']['uuid_dict'])
    check_output_file_exists(config['steps']['hw']['static_xsa'])

    with open(config['steps']['hw']['uuid_dict'], 'r') as file:
        uuid_dict = file.read()

    config['steps']['hw']['logic_uuid']     = uuid_dict.split(" ")[1]


# Use Vivado to generated the PDI. From HW sources, the build_hw.tcl script will:
#    - Create a BD design
#    - Run Synthesis and implementation
#    - Write the device image (PDI)
#    - Finally generate a static XSA needed to build the FW image.
def get_application_metadata(opt, config):

    if "xbtest" in config['design']['application']:

        config['steps']['hw']['xbtest_user_metadata_json']     = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'xbtest', 'metadata', 'user_metadata.json'))
        config['steps']['hw']['xbtest_pfm_def_metadata_json']  = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'xbtest', 'metadata', 'xbtest_pfm_def.json'))
        config['steps']['hw']['xbtest_metadata_json']          = os.path.abspath(os.path.join(config['steps']['hw']['dir'], 'xbtest', 'metadata', 'xbtest_metadata.json'))
        
        # Copy xbtest test/*.json and xbtest metadata files
        copy_source_dir(os.path.abspath(os.path.join(config['hw_src_dir'], 'iprepo', 'xbtest', 'test')), config['steps']['hw']['xbtest_test_json_dir'])
        shutil.copyfile(os.path.abspath(os.path.join(config['hw_src_dir'], 'iprepo', 'xbtest', 'user_metadata.json')), config['steps']['hw']['xbtest_user_metadata_json'])
        shutil.copyfile(os.path.abspath(os.path.join(config['hw_src_dir'], 'iprepo', 'xbtest', 'xbtest_pfm_def.json')), config['steps']['hw']['xbtest_pfm_def_metadata_json'])




# Main function of hw step
def run_step_hw(opt, config):
    start_step(opt, config, 'hw')

    build_hw(opt, config)

    get_application_metadata(opt, config)

    end_step(opt, config)
