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


# Build the FW image (ELF) to run on RPU
def build_amc(opt, config):
    config['steps']['fw']['amc_build_dir']      = os.path.abspath(os.path.join(config['fw_src_dir'], 'AMC'))
    config['steps']['fw']['amc_build_scripts']  = os.path.abspath(os.path.join(config['steps']['fw']['amc_build_dir'], 'scripts'))    
    config['steps']['fw']['amc_build_sh']       = os.path.abspath(os.path.join(config['steps']['fw']['amc_build_scripts'], 'build.sh'))
    config['steps']['fw']['amc_build_output']   = os.path.abspath(os.path.join(config['steps']['fw']['amc_build_dir'], 'build', 'amc.elf'))

    check_input_file_exists(config['steps']['fw']['amc_build_sh'])
    check_input_file_exists(config['steps']['hw']['static_xsa'])

    config['steps']['fw']['amc_elf'] = os.path.abspath(os.path.join(config['steps']['fw']['dir'], 'amc.elf'))

    if "v80p" in config['board']['name']:
        amc_profile = "v80"
    else:
        amc_profile = config['board']['name']

    if is_current_step_run(config):
        # Change build directory to ./fw
        os.chdir(os.path.abspath(os.path.join(config['steps']['fw']['amc_build_dir'])))
      
        # Command ./scripts/build.sh 
        #   -os freertos10_xilinx 
        #   -profile <board profile e.g. v80> 
        #   -xsa <absolute_path_to_xsa>
       
        cmd = [ 'bash', config['steps']['fw']['amc_build_sh'],  '-os',        'freertos10_xilinx',
                                                                '-profile',    amc_profile,
                                                                '-xsa',        config['steps']['hw']['static_xsa']
        ]

        log_info('GEN_DESIGN-48', 'Build AMC ELF: ' + config['steps']['fw']['amc_elf'])
        exec_step_cmd(opt, config, cmd, log_file_name=os.path.abspath(os.path.join(config['log_dir'], 'build_amc.log')))

        check_output_file_exists(config['steps']['fw']['amc_build_output'])

        shutil.copyfile(config['steps']['fw']['amc_build_output'], config['steps']['fw']['amc_elf'])

    check_output_file_exists(config['steps']['fw']['amc_elf'])


def build_fpt_bin(opt, config):
    config['steps']['fw']['gen_fpt_py']     = os.path.abspath(os.path.join(config['steps']['fw']['amc_build_scripts'], 'gen_fpt.py'))
    config['steps']['fw']['fpt_json']       = os.path.abspath(os.path.join(config['steps']['fw']['amc_build_scripts'], 'fpt.json'))
    
    check_input_file_exists(config['steps']['fw']['gen_fpt_py'])
    check_input_file_exists(config['steps']['fw']['fpt_json'])    

    config['steps']['fw']['fpt_bin'] = os.path.abspath(os.path.join(config['steps']['fw']['dir'], 'fpt.bin'))

    if is_current_step_run(config):
        os.chdir(os.path.abspath(os.path.join(config['steps']['fw']['dir'])))
              
        # Command ./scripts/gen_fpt.py 
        #   -f <absolute_path_to_fpt_json>
       
        cmd = [ 'python3', config['steps']['fw']['gen_fpt_py'], '-f', config['steps']['fw']['fpt_json']
        ]

        log_info('GEN_DESIGN-49', 'Build FPT Binary: ' + config['steps']['fw']['fpt_bin'])
        exec_step_cmd(opt, config, cmd, log_file_name=os.path.abspath(os.path.join(config['log_dir'], 'build_fpt_bin.log')))

    check_output_file_exists(config['steps']['fw']['fpt_bin'])    



def run_step_fw(opt, config):
    start_step(opt, config, 'fw')

    build_amc(opt, config)
    build_fpt_bin(opt, config)

    end_step(opt, config)
