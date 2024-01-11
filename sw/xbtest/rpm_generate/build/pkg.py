#!/usr/bin/env python3

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

import platform
import os
import sys
import shutil
import subprocess
import datetime
import getopt
import re
import json
import tarfile

import logging
from io import StringIO

# Get the root logger
logger = logging.getLogger('')
logger.setLevel(logging.INFO)
# stream
log_stream = StringIO()
string_handler = logging.StreamHandler(stream=log_stream)
string_handler.setLevel(logging.INFO)
# stream
console_handler = logging.StreamHandler(stream=sys.stdout)
console_handler.setLevel(logging.INFO)
# formatter
formatter = logging.Formatter('%(levelname)s: %(message)s')
# add
string_handler.setFormatter(formatter)
console_handler.setFormatter(formatter)
logger.addHandler(string_handler)
logger.addHandler(console_handler)

def setup_verbose():
    logger.setLevel(logging.DEBUG)
    string_handler.setLevel(logging.DEBUG)
    console_handler.setLevel(logging.DEBUG)

def setup_logfile(log_file):
    with open(log_file, 'w') as fd:
        log_stream.seek(0)
        shutil.copyfileobj(log_stream, fd)
    file_handler = logging.FileHandler(log_file, 'a')
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    logger.removeHandler(string_handler)

def exit_msg(level, id, msg):
    log_msg(level, id, msg)
    if level == logging.ERROR:
        sys.exit(1)
    else:
        sys.exit(0)

def exit_info(id, msg):
    exit_msg(logging.INFO, id, msg)

def exit_error(id, msg):
    exit_msg(logging.ERROR, id, msg)

def format_time_str(time_in):
    return time_in.strftime('%Y-%m-%d, %H:%M:%S')
def format_msg(id, msg):
    return '[' + id + '] ' + msg

def log_debug(id, msg):
    logger.debug(format_msg(id,msg))
def log_info(id, msg):
    logger.info(format_msg(id,msg))
def log_warning(id, msg):
    logger.warning(format_msg(id,msg))
def log_error(id, msg):
    logger.error(format_msg(id,msg))
def log_msg(level, id, msg):
    logger.log(level, format_msg(id,msg))

CWD = os.getcwd(); # get current working directory

DIST_ID_CENTOS  = 'CentOS'
DIST_ID_UBUNTU  = 'Ubuntu'
DIST_ID_REDHAT  = 'RedHatEnterprise'
DIST_ID_REDHAT2 = 'RedHatEnterpriseWorkstation'
DIST_ID_SLES    = 'SUSE'
SUPPORTED_DIST_ID = [
    DIST_ID_CENTOS,
    DIST_ID_UBUNTU,
    DIST_ID_REDHAT,
    DIST_ID_REDHAT2,
    DIST_ID_SLES,
]
DIST_RPM = [DIST_ID_CENTOS, DIST_ID_REDHAT, DIST_ID_REDHAT2, DIST_ID_SLES]
DIST_DEB = [DIST_ID_UBUNTU]

ARCH_X86_64     = 'x86_64'
ARCH_AMD64      = 'amd64'
ARCH_PPC64LE    = 'ppc64le'
ARCH_PPC64      = 'ppc64'
SUPPORTED_ARCH = [
    ARCH_X86_64,
    ARCH_AMD64,
    ARCH_PPC64LE,
    ARCH_PPC64,
]

OPT_INSTALL_DIR = 'opt/amd/xbtest'
EXE_INSTALL_DIR = 'usr/local/bin'

NAME                = 'name'
VERSION             = 'version'
RELEASE             = 'release'
SUMMARY             = 'summary'
DESCRIPTION         = 'description'
CHANGELOG           = 'changelog'
PRE_INST_MSG        = 'pre installation message'
POST_INST_MSG       = 'post installation message'
INSTALL_DIR         = 'installation directory'

SRC = 'source'
DST = 'destination'

MAJOR           = 'major'
MINOR           = 'minor'
INTERNALRELEASE = 'internal_release'

def get_date_long():
    # Thu Jul 15 09:35:39 BST 2021
    time_0 = datetime.datetime.now().replace(microsecond=0)
    return time_0.strftime('%a %b %d %H:%M:%S %Z %Y')
def get_date_short():
    # Thu Jul 15 2021
    time_0 = datetime.datetime.now().replace(microsecond=0)
    return time_0.strftime('%a %b %d %Y')

def start(id, file):
    start_time = datetime.datetime.now().replace(microsecond=0)
    log_info(id, '--------------------------------------------------------------------------------------')
    log_info(id, '[' + format_time_str(start_time) + '] Starting ' + file)
    log_info(id, '--------------------------------------------------------------------------------------')
    return start_time

def tear_down(id, file, start_time):
    end_time = datetime.datetime.now().replace(microsecond=0)
    elapsed_time = end_time - start_time
    log_info(id, '--------------------------------------------------------------------------------------')
    log_info(id, '[' + format_time_str(end_time) + '] ' + file + ' END. Total Elapsed Time: ' + str(elapsed_time))
    log_info(id, '--------------------------------------------------------------------------------------')
    sys.exit(0)

def copy_source_dir(id, src_dir, dest_dir):
    log_debug(id, 'Copying source directory: ' + src_dir)
    if not os.path.isdir(src_dir):
        exit_error(id, 'Source directory does not exist: ' + src_dir)
    if not os.path.isdir(dest_dir):
        os.makedirs(dest_dir)
    for root, dirs, files in os.walk(src_dir):
        for file in files:
            # log_debug(id, 'Copying source file: ' + file)
            shutil.copy(os.path.abspath(os.path.join(src_dir, file)), dest_dir)
            os.chmod(os.path.abspath(os.path.join(dest_dir, file)), 511); # octal 777
        for dir in dirs:
            # log_debug(id, 'Copying source directory: ' + dir)
            dst = os.path.abspath(os.path.join(dest_dir, dir))
            shutil.copytree(os.path.abspath(os.path.join(src_dir, dir)), dst)
            os.chmod(dst, 511); # octal 777
        break   #prevent descending into sub-folders

def copy_source_file(id, src_file, dest_dir):
    log_debug(id, 'Copying source file: ' + src_file)
    if not os.path.isfile(src_file):
        exit_error(id, 'Source file does not exist: ' + src_file)
    if not os.path.isdir(dest_dir):
        os.makedirs(dest_dir)
    shutil.copy(src_file, dest_dir)

def start_step(id, step):
    start_time = datetime.datetime.now().replace(microsecond=0)
    log_info(id, '*** [' + format_time_str(start_time) + '] Starting step: ' + step)
    return start_time

def end_step(id, start_time):
    elapsed_time = datetime.datetime.now().replace(microsecond=0) - start_time
    log_info(id, '************************** End of step. Elapsed time: ' + str(elapsed_time) + '\n\n')

def exec_step_cmd(id, step, cmd, log_file_name = None, use_console = False, shell = False, ignore_error = False, env = None, expect_fail = False):
    if not shell:
        cmd_str = ' '.join(cmd)
    else:
        cmd_str = cmd

    log_info(id, 'Executing: $ ' + cmd_str)

    proc = subprocess.Popen(cmd, shell=shell, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, env=env)

    if log_file_name is not None:
        log_info(id, 'Log file: ' + log_file_name)
        log_file = open(log_file_name, mode='w')

    for line in proc.stdout:
        if use_console:
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
            err_log_msg = '. Check log for more details: ' + log_file_name

        if not expect_fail and proc.returncode != 0:
            exit_error(id, 'Step ' + step + ' failed: Unexpected non-zero return code (' + str(proc.returncode) + ') for command: ' + cmd_str + err_log_msg)
        elif expect_fail and proc.returncode == 0:
            exit_error(id, 'Step ' + step + ' failed: Unexpected zero return code for command: ' + cmd_str + err_log_msg)
    return proc.returncode

def json_load(id, name, file):
    json_data = {}
    if not os.path.isfile(file):
        exit_error(id, 'Failed to load ' + name + '. File does not exist: ' + file)
    with open(file) as infile:
        try:
            json_data = json.load(infile)
        except ValueError as e:
            exit_error(id, 'Failed to load ' + name + '. File contains invalid JSON content: ' + file + '. JSON parser error: ' + str(e))
    return json_data
