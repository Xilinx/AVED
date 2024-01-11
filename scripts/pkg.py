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

import json
import os
import sys
import shutil
import types
import glob
import subprocess
import signal
import copy
import datetime
import re
import inspect
from functools import reduce
import logging
import zipfile
import tarfile
from io import StringIO
import textwrap
import argparse

# Define in this list the names of different steps of the build flow
SUPPORTED_STEPS = ['hw', 'fw', 'pdi', 'deploy']

# Define list of supported OS used for creating the output RPM or DEB package
DIST_ID_CENTOS  = 'CentOS'
DIST_ID_REDHAT  = 'RedHatEnterprise'
DIST_ID_REDHAT2 = 'RedHatEnterpriseWorkstation'
DIST_ID_SLES    = 'SUSE'
DIST_ID_UBUNTU  = 'Ubuntu'
DIST_RPM = [DIST_ID_CENTOS, DIST_ID_REDHAT, DIST_ID_REDHAT2, DIST_ID_SLES]
DIST_DEB = [DIST_ID_UBUNTU]
SUPPORTED_DIST_ID = DIST_RPM + DIST_DEB

# Format the command line option parser display in console to allow lin jumps.
class ArgParseFormatter(argparse.HelpFormatter):
    def _fill_text(self, text, width, indent):
        return "\n".join([textwrap.fill(line, width) for line in textwrap.indent(textwrap.dedent(text), indent).splitlines()])

# Setup logging class: message formatting, default severity level is INFO.
logger = logging.getLogger('')
logger.setLevel(logging.INFO)
log_stream = StringIO()
string_handler = logging.StreamHandler(stream=log_stream)
string_handler.setLevel(logging.INFO)
console_handler = logging.StreamHandler(stream=sys.stdout)
console_handler.setLevel(logging.INFO)
formatter = logging.Formatter('%(levelname)s: %(message)s')
string_handler.setFormatter(formatter)
console_handler.setFormatter(formatter)
logger.addHandler(string_handler)
logger.addHandler(console_handler)

# Change the logging severity to DEBUG when verbose option specified in command line
def setup_verbose():
    logger.setLevel(logging.DEBUG)
    string_handler.setLevel(logging.DEBUG)
    console_handler.setLevel(logging.DEBUG)

# After the output directory is created, create log file and dump previous log messages into it
def setup_logfile(log_file):
    with open(log_file, 'w') as fd:
        log_stream.seek(0)
        shutil.copyfileobj(log_stream, fd)
    file_handler = logging.FileHandler(log_file, 'a')
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    logger.removeHandler(string_handler)

# Terminate the script and set script return code depending on the severity of the message
# TODO instead of using the severity of exit message, set return code based on error present in previous log message
def exit_msg(level, id, msg):
    log_msg(level, id, msg)
    if level == logging.ERROR:
        sys.exit(1)
    else:
        sys.exit(0)

# Terminate script with INFO log message
def exit_info(id, msg): exit_msg(logging.INFO, id, msg)

# Terminate script with ERROR log message
def exit_error(id, msg): exit_msg(logging.ERROR, id, msg)

# Format date and time for log messages
def format_time_str(time_in): return time_in.strftime('%Y-%m-%d, %H:%M:%S')

# Format log message ID and actual content
def format_msg(id, msg): return '[' + id + '] ' + msg

# Wrapper procedures to log messages with different severity
def log_debug(id, msg):         logger.debug(format_msg(id,msg))
def log_info(id, msg):          logger.info(format_msg(id,msg))
def log_warning(id, msg):       logger.warning(format_msg(id,msg))
def log_error(id, msg):         logger.error(format_msg(id,msg))
def log_msg(level, id, msg):    logger.log(level, format_msg(id,msg))

# Regularly store the current content the main config dictionary
def dump_config(config):
    config_dump = copy.deepcopy(config)

    # Format times to string to be valid JSON values
    for key in ['now', 'script_start_time', 'script_end_time', 'current_step_start_time', 'current_step_end_time']:
        if key in config_dump.keys():
            config_dump[key] = format_time_str(config_dump[key])

    with open(os.path.abspath(os.path.join(config['output_dir'], 'config_dump.json')), 'w') as f:
        json.dump(config_dump, f, sort_keys=False, indent=2)

# Procedure called at the beginning of build flow
def start(config):
    config['script_start_time'] = datetime.datetime.now().replace(microsecond=0)
    log_info('GEN_DESIGN-20', '[' + format_time_str(config['script_start_time']) + '] Starting ' + config['script_name'])
    print('\n')

# Procedure called at the end of build flow
# TODO update to display the total elapsed time also in case of error.
def tear_down(config):
    config['script_end_time']       = datetime.datetime.now().replace(microsecond=0)
    config['script_elapsed_time']   = str(config['script_end_time'] - config['script_start_time'])
    print('\n')
    log_info('GEN_DESIGN-21', '[' + format_time_str(config['script_end_time']) + '] ' + config['script_name'] + ' END. Total Elapsed Time: ' + config['script_elapsed_time'])
    sys.exit(0)

# Copy entire directory and set all files as readable/writable/executable
def copy_source_dir(src_dir, dest_dir):
    if not os.path.isdir(src_dir):
        exit_error('GEN_DESIGN-23', 'Source directory does not exist: ' + src_dir)
    log_debug('GEN_DESIGN-22', 'Copying directory to: ' + dest_dir)
    if not os.path.isdir(dest_dir):
        os.makedirs(dest_dir)
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            shutil.copy(os.path.abspath(os.path.join(src_dir, file)), dest_dir)
            os.chmod(os.path.abspath(os.path.join(dest_dir, file)), 0o777)
        for dir in dirs:
            dst = os.path.abspath(os.path.join(dest_dir, dir))
            shutil.copytree(os.path.abspath(os.path.join(src_dir, dir)), dst)
            os.chmod(dst, 0o777)
        break # prevent descending into sub-folders

# Copy file to destination directory. Create destination directory if it does not exist
def copy_source_file(src_file, dest_dir):
    if not os.path.isfile(src_file):
        exit_error('GEN_DESIGN-23', 'Source file does not exist: ' + src_file)
    log_debug('GEN_DESIGN-24', 'Copying file to: ' + os.path.abspath(os.path.join(dest_dir, os.path.basename(src_file))))
    if not os.path.isdir(dest_dir):
        os.makedirs(dest_dir)
    shutil.copy(src_file, dest_dir)

# Look for any message starting with error pattern in a log file. When found, consider the command failed and display the error message found.
def check_log_error(log_file_name):
    # check for error pattern in log
    pattern = 'ERROR: ['
    error = False
    with open(log_file_name, mode='r') as f:
        for line in f:
            if pattern == line[0:len(pattern)]:
                error = True
                print(line)
    if error:
        exit_error('GEN_DESIGN-25', 'Message(s) containing pattern "' + pattern + '" found in log file: ' + log_file_name)

def is_current_step_run(config):
    return config['steps'][config['current_step']]['run_step']

# Procedure called at the beginning of each step
def start_step(opt, config, current_step):
    if current_step not in SUPPORTED_STEPS:
       exit_error('GEN_DESIGN-13', 'Unknown step: ' + current_step)

    config['current_step']              = current_step
    config['current_step_start_time']   = datetime.datetime.now().replace(microsecond=0)
    print('\n')
    log_info('GEN_DESIGN-26', '[' + format_time_str(config['current_step_start_time']) + '] Starting step: ' + config['current_step'])

    if 'steps' not in config.keys():
        config['steps'] = {}
    config['steps'][current_step] = {}
    config['steps'][current_step]['step_start_time'] = format_time_str(config['current_step_start_time'])

    run_step = True
    step_dir = os.path.abspath(os.path.join(config['output_dir'], current_step))

    # Check if the current step should be run. If --from_step option is provided, do not re-run steps before provided step. If --single_step is provided, re-run only provided step.
    # If restarting or re-running a later step, check the current step build directory exists
    if opt.from_step is not None:
        if SUPPORTED_STEPS.index(opt.from_step) > SUPPORTED_STEPS.index(current_step):
            run_step = False
        if SUPPORTED_STEPS.index(opt.from_step) > SUPPORTED_STEPS.index(current_step):
            if not os.path.isdir(step_dir):
               exit_error('GEN_DESIGN-13', 'Cannot restart from provided step (' + opt.from_step + ') as build directory of earlier step (' + current_step + ') does not exist: ' + step_dir)
    elif opt.single_step is not None:
        if SUPPORTED_STEPS.index(opt.single_step) != SUPPORTED_STEPS.index(current_step):
            run_step = False
        if SUPPORTED_STEPS.index(opt.single_step) > SUPPORTED_STEPS.index(current_step):
            if not os.path.isdir(step_dir):
               exit_error('GEN_DESIGN-13', 'Cannot rerun provided step (' + opt.single_step + ') as build directory of earlier step (' + current_step + ') does not exist: ' + step_dir)

    # If current step build directory already exist while the step should be rerun, the --force option must be provided
    if os.path.isdir(step_dir) and run_step:
        force_remove_dir(step_dir)
    if not os.path.isdir(step_dir):
        os.makedirs(step_dir)
    os.chdir(step_dir)

    config['steps'][current_step]['run_step']   = run_step
    config['steps'][current_step]['dir']        = step_dir

    dump_config(config)

# Procedure called at the end of each step
def end_step(opt, config):
    config['current_step_end_time']     = datetime.datetime.now().replace(microsecond=0)
    config['current_step_elapsed_time'] = str(config['current_step_end_time'] - config['current_step_start_time'])

    config['steps'][config['current_step']]['step_end_time']        = format_time_str(config['current_step_end_time'])
    config['steps'][config['current_step']]['step_elapsed_time']    = config['current_step_elapsed_time']
    log_info('GEN_DESIGN-27', 'End of step ' + config['current_step'] + '. Elapsed time: ' + config['current_step_elapsed_time'])
    print('\n')

    if opt.single_step is not None:
        if SUPPORTED_STEPS.index(opt.single_step) == SUPPORTED_STEPS.index(config['current_step']):
            log_info('GEN_DESIGN-27', 'Script ending after step ' + config['current_step'] + ' as --single_step option was provided')
            tear_down(config)

# Execute external command, maanage log and check for errors
def exec_step_cmd(opt, config, cmd, log_file_name = None, shell = False, ignore_error = False, env = None, expect_fail = False):
    if not shell:
        cmd_str = ' '.join(cmd)
    else:
        cmd_str = cmd

    log_info('GEN_DESIGN-28', 'Executing: $ ' + cmd_str)

    proc = subprocess.Popen(cmd, shell=shell, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, env=env)

    if log_file_name is not None:
        log_info('GEN_DESIGN-29', 'Log file: ' + log_file_name)
        log_file = open(log_file_name, mode='w')

    for line in proc.stdout:
        if opt.verbose:
            sys.stdout.write(line)
            sys.stdout.flush()
        if log_file_name is not None:
            log_file.write(line)
            log_file.flush()

    proc.wait()

    if not ignore_error:
        err_log_msg = ''
        if log_file_name is not None:
            log_file.close()
            check_log_error(log_file_name)
            err_log_msg = '. Check log for more details: ' + log_file_name

        if not expect_fail and proc.returncode != 0:
            exit_error('GEN_DESIGN-30', 'Step ' + config['current_step'] + ' failed: Unexpected non-zero return code (' + str(proc.returncode) + ') for command: ' + cmd_str + err_log_msg)
        elif expect_fail and proc.returncode == 0:
            exit_error('GEN_DESIGN-31', 'Step ' + config['current_step'] + ' failed: Unexpected zero return code for command: ' + cmd_str + err_log_msg)
    return proc.returncode

# Try to log JSON file and report if content is not valid
def json_load(name, file):
    json_data = {}
    if not os.path.isfile(file):
        exit_error('GEN_DESIGN-32', 'Failed to load ' + name + '. File does not exist: ' + file)
    with open(file) as infile:
        try:
            json_data = json.load(infile)
        except ValueError as e:
            exit_error('GEN_DESIGN-33', 'Failed to load ' + name + '. File contains invalid JSON content: ' + file + '. JSON parser error: ' + str(e))
    return json_data

# Procedures called at the beginning or at the end of each action of each steps to check if the inputs or outputs of the actions exist
def check_input_file_exists(fname):
    if not os.path.isfile(fname):
        exit_error('GEN_DESIGN-34', 'Input file does not exist: ' + fname)
def check_input_dir_exists(fname):
    if not os.path.isdir(fname):
        exit_error('GEN_DESIGN-35', 'Input directory does not exist: ' + fname)
def check_output_file_exists(fname):
    if not os.path.isfile(fname):
        exit_error('GEN_DESIGN-36', 'Output file does not exist: ' + fname)
    log_info('GEN_DESIGN-37', 'Successfully generated: ' + fname)
def check_output_dir_exists(fname):
    if not os.path.isdir(fname):
        exit_error('GEN_DESIGN-38', 'Output directory does not exist: ' + fname)
    log_info('GEN_DESIGN-39', 'Successfully generated: ' + fname)

# Try to remove a directory and check it worked
def force_remove_dir(output_dir):
    try:
       shutil.rmtree(output_dir)
    except OSError as e:
       exit_error('GEN_DESIGN-40', 'Failed to remove output directory: ' + output_dir + '. Exception caught: ' + str(e))

    if os.path.isdir(output_dir):
       exit_error('GEN_DESIGN-41', 'Failed to remove output directory: ' + output_dir + '. Directory still exists')

# Try to get value from nested directory and report empty dictionary when the item is not found in dictionary
def dict_get(mydict, keys, default={}): return reduce(lambda c, k: c.get(k, default), keys, mydict)
