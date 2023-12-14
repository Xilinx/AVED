#!/usr/bin/env python3

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

# TODO check the tools needed during build flow are available: bootgen, cdoutil, Vivado, v++, xclbinutil, etc
# TODO change all message IDs to AVED-XXX in all sources (python,tcl,etc)

import os
import sys

SCRIPT_VERSION = '1.0'
SCRIPT_NAME    = os.path.basename(__file__)
SCRIPT_DIR     = os.path.dirname(os.path.realpath(__file__))

SRC_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, 'scripts'))
sys.path.insert(0, SRC_DIR)

# Get current working directory
CWD = os.getcwd()

# Import package containing utility functions.
from pkg import *

# Import the main function used to initialize the build flow.
from setup import set_up

# Import the main function of each step of the build flow.
from step_hw                import run_step_hw
from step_fw                import run_step_fw
from step_pdi               import run_step_pdi
#from step_sw                import run_step_sw
from step_deploy            import run_step_deploy

# Parse and check command line options
def get_options():
    DEFAULT_LSF_CMD = 'bsub -I -R "select[type=X86_64 && (osdistro=rhel || osdistro=centos) && (osver == ws7 || osver == cent7)] rusage[mem=256000]" -N -q long -W 48:00'

    program_descripton  = []
    program_descripton += ['This script generates Alveo Versal Example Design.']
    program_descripton += ['']
    program_descripton += ['It is divided in a series of steps (<step>) run in following sequence:']
    program_descripton += ['    # | Step']
    program_descripton += ['  ----|---------------------']
    for ii in range(0,len(SUPPORTED_STEPS)):
        program_descripton += ['   %2d | %s' % ((ii+1), SUPPORTED_STEPS[ii])]

    parser = argparse.ArgumentParser(
        prog            = SCRIPT_NAME,
        description     = '\n'.join(program_descripton),
        usage           = '%(prog)s [options]',
        allow_abbrev    = False,
        formatter_class = ArgParseFormatter
    )
    parser.add_argument('-o', '--output_dir',  metavar='<output_dir>',   action='store',                                                                  help='Path to the output directory (default = ./output/<VBNV>/<date>_<time> where VBNV is <Vendor name>_<Board name>_<design Name>_<design Version>. Example: amd_v80_gen5x8_23.2_exdes_1)')
    parser.add_argument('-V', '--verbose',                               action='store_true',                                                             help='Turn on verbosity.' )
    parser.add_argument('-f', '--force',                                 action='store_true',                                                             help='Override output directory if already existing.' )
    parser.add_argument('-c', '--conf_json',   metavar='<conf_json>',    action='store',                                                                  help='Path to the configuration JSON file. (Required)' )
    parser.add_argument('-s', '--from_step',   metavar='<step>',         action='store',                                      choices=SUPPORTED_STEPS,    help='Restart previous <output_dir> build from given step <step>. All outputs of any preceding step in sequence must exist in previous build at expected location. Step sequence is specified in help message.' )
    parser.add_argument('-S', '--single_step', metavar='<step>',         action='store',                                      choices=SUPPORTED_STEPS,    help='Rerun previous <output_dir> build executing only given step <step>. All outputs of any preceding step in sequence must exist in previous build at expected location. Step sequence is specified in help message.' )
    parser.add_argument('-l', '--use_lsf',                               action='store_true',                                                             help='Use LSF to run some of the steps. Default LSF command is: ' + DEFAULT_LSF_CMD)
    parser.add_argument('-L', '--lsf_cmd',     metavar='<lsf_cmd>',      action='store',                                                                  help='Use LSF to run some of the steps and specify the LSF command.')
    parser.add_argument('-v', '--version',                               action='version',                                                                help='Display version', version='%(prog)s version ' + SCRIPT_VERSION )
    parser.add_argument('-g', '--gui',                                   action='store_true',                                                             help='Run vivado in GUI mode, create bd design, and drop into interactive mode.')
    parser.add_argument('-d', '--deploy',                                action='store_true',                                                             help='Run additional step to generate AVED deployment archive')

    opt = parser.parse_args()

    # LSF
    # TODO remove support of LSF command line options?
    # TODO or simplify usage, e.g. merge --lsf_cmd wit --use_lsf?
    if opt.lsf_cmd is not None:
        opt.use_lsf = True
    elif opt.use_lsf:
        opt.lsf_cmd = DEFAULT_LSF_CMD

    # Get absolute paths
    if opt.conf_json is None:
        exit_error('GEN_DESIGN-16', 'A configuration json file must be specified with the -c or --conf_json option.')
    else:
        opt.conf_json = os.path.abspath(opt.conf_json)

    if opt.output_dir is not None:
        opt.output_dir = os.path.abspath(opt.output_dir)

    # Enable verbose mode, print all debug messages and also print output of each commands in console in addition to their own log file
    if opt.verbose:
        setup_verbose()

    if ((opt.from_step is not None) and (opt.single_step is not None)):
        exit_error('GEN_DESIGN-12', 'Options --from_step and --single_step cannot be combined')

    # An output directory of previous build is required to restart from a step or rerun a single step.
    if ((opt.from_step is not None) or (opt.single_step is not None)):
        if opt.output_dir is None:
            exit_error('GEN_DESIGN-10', '--output_dir option must be provided with option --from_step or --single_step')
        if not os.path.isdir(opt.output_dir):
           exit_error('GEN_DESIGN-13', 'Cannot restart from or rerun provided step as output directory does not exist: ' + opt.output_dir)

    # Check the configuration JSON file (default location or provided in command line) exists. This is not needed when reusing a previous build.
    if (opt.from_step is None) and (opt.single_step is None):
        if not os.path.isfile(opt.conf_json):
            exit_error('GEN_DESIGN-12', 'Configuration JSON file does not exist: ' + opt.conf_json)

    if opt.from_step is not None:
        log_info('GEN_DESIGN-11', 'Script will resume from step (' + opt.from_step + ') using design sources previously saved in output directory')
    if opt.single_step is not None:
        log_info('GEN_DESIGN-11', 'Script will rerun only step (' + opt.single_step + ') using design sources previously saved in output directory')

    if (opt.from_step == "deploy") and not(opt.deploy):
            exit_error('GEN_DESIGN-15', 'Option "--from_step deploy" can only be run with --deploy option')



    return opt

def main(args):

    if sys.version_info[0] < 3:
        exit_error('AVED-1', 'Must be using Python 3')

    opt = get_options()

    config = {}
    config['script_dir']        = SCRIPT_DIR
    config['script_name']       = SCRIPT_NAME
    config['user_cwd']          = CWD
    config['now']               = datetime.datetime.now()
    config['build_date_short']  = config['now'].replace(microsecond=0).strftime('%a %b %d %Y')

    try:
        start(config)

        # Set-up build flow, e.g. create output directory, back-up sources, load configuration
        set_up(opt, config)

        # Run Vivado to create BD design, and generate PDI
        run_step_hw(opt, config)

        # Build FW executable from sources to run on the RPU
        run_step_fw(opt, config)

        # This step combines the device image (PDI) created by Vivado in hw step, the FW image and Extension FPT in a new PDI
        run_step_pdi(opt, config)

        if opt.deploy:
            # This step pulls the output products from the various other steps and creates a deployment archive for installation on the Host
            run_step_deploy(opt, config)

        tear_down(config)

    except OSError as o:
        print(o)
        raise RuntimeError(o.errno)
    except AssertionError as a:
        print(a)
        raise RuntimeError(1)
    except Exception as e:
        print(e)
        raise RuntimeError(1)
    finally:
        print('')

    sys.exit(1)

if __name__ == '__main__':
    main(sys.argv)
