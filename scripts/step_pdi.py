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

# Write the Boot Image Format configuration file needed by bootgen tool to set the content of the PDI.
# If applicable, also include the AIE image (CDOs) extracted directly from Vitis AIE Compiler output directory (not from libadf.a).
def write_pdi_combine_bif(opt, config):
    check_input_file_exists(config['steps']['hw']['hw_pdi'])
    check_input_file_exists(config['steps']['fw']['amc_elf'])

    config['steps']['pdi']['pdi_combine_bif'] = os.path.abspath(os.path.join(config['steps']['pdi']['dir'], 'pdi_combine.bif'))

    if is_current_step_run(config):
        PDI_COMBINE_BIF = [
            'all:',
            '{',
            '    image {',
            '        { type=bootimage, file=' + config['steps']['hw']['hw_pdi'] + ' }',
            '    }',
            '    image {',
            '        id = 0x1c000000, name=rpu_subsystem, delay_handoff',
            '        { core=r5-0, file=' + config['steps']['fw']['amc_elf'] + ' }',
            '    }',
            '}',
        ]

        log_info('GEN_DESIGN-51', 'Create Boot Image Format file: ' + config['steps']['pdi']['pdi_combine_bif'])
        with open(config['steps']['pdi']['pdi_combine_bif'], 'w') as f:
            f.write('\n'.join(PDI_COMBINE_BIF))

    check_output_file_exists(config['steps']['pdi']['pdi_combine_bif'])

# Use bootgen tool to combine the different design images in a new PDI
def generate_design_pdi(opt, config):
    check_input_file_exists(config['steps']['pdi']['pdi_combine_bif'])

    config['steps']['pdi']['design_pdi'] = os.path.abspath(os.path.join(config['steps']['pdi']['dir'], 'design.pdi'))

    if is_current_step_run(config):
        cmd = [ 'bootgen',  '-arch',    'versal',
                            '-image',   config['steps']['pdi']['pdi_combine_bif'],
                            '-w',
                            '-o',       config['steps']['pdi']['design_pdi']
        ]
        log_info('GEN_DESIGN-52', 'Generate Programmable Device Image file: ' + config['steps']['pdi']['design_pdi'])
        exec_step_cmd(opt, config, cmd, log_file_name=os.path.abspath(os.path.join(config['log_dir'], 'bootgen_design_pdi.log')))

    check_output_file_exists(config['steps']['pdi']['design_pdi'])

def generate_fpt_setup_pdi(opt, config):
    check_input_file_exists(config['steps']['pdi']['design_pdi'])
    check_input_file_exists(config['steps']['fw']['fpt_bin'])    

    config['steps']['pdi']['fpt_pdi_gen_py'] = os.path.abspath(os.path.join(config['hw_src_dir'], 'fpt', 'fpt_pdi_gen.py'))
    config['steps']['pdi']['fpt_release']    = str(config['now'].year) + str(config['now'].month).zfill(2) + str(config['now'].day).zfill(2)
    
    config['steps']['pdi']['fpt_name']   = 'fpt_setup_' + config['design']['vbnv'] + '_' + config['steps']['pdi']['fpt_release'] + '.pdi'
    
    config['steps']['pdi']['fpt_setup_pdi']     = os.path.abspath(os.path.join(config['steps']['pdi']['dir'], config['steps']['pdi']['fpt_name']))

    if is_current_step_run(config):
        cmd = [ 'python3', config['steps']['pdi']['fpt_pdi_gen_py'],
                                                    '--fpt',            config['steps']['fw']['fpt_bin'],
                                                    '--pdi',            config['steps']['pdi']['design_pdi'],
                                                    '--output',         config['steps']['pdi']['fpt_setup_pdi']
              ]
              
        log_info('GEN_DESIGN-53', 'Generate FPT Setup Programmable Device Image file: ' + config['steps']['pdi']['fpt_setup_pdi'])
        exec_step_cmd(opt, config, cmd, log_file_name=os.path.abspath(os.path.join(config['log_dir'], 'fpt_setup_pdi_gen.log')))

    check_output_file_exists(config['steps']['pdi']['fpt_setup_pdi'])    

# Main function of pdi step
def run_step_pdi(opt, config):
    start_step(opt, config, 'pdi')

    write_pdi_combine_bif(opt, config)
    generate_design_pdi(opt, config)
    generate_fpt_setup_pdi(opt, config)

    end_step(opt, config)
