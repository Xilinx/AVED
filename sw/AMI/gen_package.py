#!/usr/bin/env python3

# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2023-present Advanced Micro Devices, Inc. All rights reserved.

import re
import os
import sys

from os import walk
from os.path import join

SCRIPT_VERSION = '1.0'
SCRIPT_FILE    = os.path.basename(__file__)
SCRIPT_DIR     = os.path.dirname(os.path.realpath(__file__))

BUILD_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, 'build'))
sys.path.insert(0, BUILD_DIR)

from pkg import *

# Get date
build_date=get_date_long()
build_date_short=get_date_short()
now = datetime.datetime.now()

class Options(object):
    def PrintVersion(self):
        log_info('GEN_PACKAGE-5', 'Script ' + SCRIPT_FILE + ' version: ' + SCRIPT_VERSION)

    def printHelp(self):
        log_info('GEN_PACKAGE-6', 'Usage: $ python3 ' + SCRIPT_FILE + ' [options]')
        log_info('GEN_PACKAGE-6', '\t--help                 / -h: Display this message')
        log_info('GEN_PACKAGE-6', '\t--output_dir           / -o: Path to the output directory. Default: ./output/<date>_<time>')
        log_info('GEN_PACKAGE-6', '\t--pkg_release          / -r: Package release. Default <date>')
        log_info('GEN_PACKAGE-6', '\t--verbose              / -V: Turn on verbosity')
        log_info('GEN_PACKAGE-6', '\t--force                / -f: Override output directory if already existing')
        log_info('GEN_PACKAGE-6', '\t--version              / -v: Display version')
        log_info('GEN_PACKAGE-6', '\t--no_driver            / -n: No driver built during gen_package - docker build option only')
        log_info('GEN_PACKAGE-6', '\t--no_gen_version       / -g: No genVersion scripts run during gen_package - docker build option only')
        log_info('GEN_PACKAGE-6', '')

    def __init__(self):
        self.help = False
        self.output_dir = None
        self.pkg_release = None
        self.force = False
        self.verbose = False
        self.version = False
        self.no_driver = False
        self.no_gen_version = False

    def getOptions(self, argv):
        log_info('GEN_PACKAGE-62', 'Command line provided: $ ' + str(sys.executable) + ' ' + ' '.join(argv))
        try:
            options, remainder = getopt.gnu_getopt(
                argv[1:],
                'ho:r:Vfvng',
                [
                    'help',
                    'output_dir=',
                    'pkg_release=',
                    'verbose',
                    'force',
                    'version',
                    'no_driver',
                    'no_gen_version'
                ]
            )
        except getopt.GetoptError as e:
            self.printHelp()
            exit_error('GEN_PACKAGE-1', str(e))

        log_info('GEN_PACKAGE-63', 'Parsing command line options')
        for opt, arg in options:
            msg = '\t' + str(opt)
            if arg is not None:
                msg += ' ' + str(arg)
            log_info('GEN_PACKAGE-63', msg)

            if opt in ('--help', '-h'):
                self.printHelp()
                self.help = True
            elif opt in ('--output_dir', '-o'):
                self.output_dir = str(arg)
                self.output_dir = os.path.abspath(self.output_dir)
            elif opt in ('--pkg_release', '-r'):
                self.pkg_release = str(arg)
            elif opt in ('--verbose', '-V'):
                setup_verbose()
                self.verbose = True
            elif opt in ('--force', '-f'):
                self.force = True
            elif opt in ('--version', '-v'):
                self.PrintVersion()
                self.version = True
            elif opt in ('--no_driver', '-n'):
                self.no_driver = True
            elif opt in ('--no_gen_version', '-g'):
                self.no_gen_version = True
            else:
                exit_error('GEN_PACKAGE-2', 'Command line option not handled: ' + str(opt))

        if len(remainder) > 0:
            self.printHelp()
            exit_error('GEN_PACKAGE-3', 'Unknown command line options: ' + ' '.join(remainder))

        if self.help or self.version:
            exit_info('GEN_PACKAGE-4', 'Script terminating as help/version option provided')

        if self.pkg_release is None:
            self.pkg_release = str(now.year) + str(now.month).zfill(2) + str(now.day).zfill(2)


def find_longest_filter(path, filters):
    """
    find_longest_filter - find the longest matching filter for a pathname.

    :param path: Full path for which to find a matching filter
    :param filters: Dictionary of the format {'path': [files...]}
    :return: Matched filter or an empty string
    """
    longest_filter = ''

    for k in filters.keys():
        if path.startswith(k):
            if (len(k) > len(longest_filter)):
                longest_filter = k

    return longest_filter


def main(args):
    # Parse command line options
    opt = Options()
    opt.getOptions(args)

    config = {}

    # Driver and API sources - will look for them later, after we run
    # the getVersion.sh script (will miss the version headers otherwise)
    driver = []
    api_headers = []

    try:

        script_start_time = start('GEN_PACKAGE-7', SCRIPT_FILE)

        #######################################################################################################

        # Get/define output directory
        if opt.output_dir is not None:
            output_dir = opt.output_dir
        else:
            output_dir =        str(now.year)          + '-' + str(now.month).zfill(2)  + '-' + str(now.day).zfill(2)
            output_dir += '_' + str(now.hour).zfill(2) + '-' + str(now.minute).zfill(2) + '-' + str(now.second).zfill(2)
            output_dir = os.path.abspath(os.path.join(CWD, 'output', output_dir))

        # Define steps output directories and others
        tmp_dir         = os.path.abspath(os.path.join(output_dir, 'tmp'))
        log_dir         = os.path.abspath(os.path.join(output_dir, 'log'))
        bkp_design_dir  = os.path.abspath(os.path.join(output_dir, 'bkp_design'))

        # Create output directory
        if os.path.isdir(output_dir):
           if not opt.force:
               exit_error('GEN_PACKAGE-17', 'Output directory already exists (see --force to override): ' + output_dir)
           else:
               log_info('GEN_PACKAGE-60', 'Removing output directory already existing as --force option is provided: ' + output_dir)
               force_remove_dir('GEN_PACKAGE-61', output_dir)

        log_info('GEN_PACKAGE-18', 'Creating output directory: ' + output_dir)
        os.makedirs(output_dir)

        if not os.path.isdir(log_dir):
            os.makedirs(log_dir)

        setup_logfile(os.path.abspath(os.path.join(log_dir, os.path.splitext(SCRIPT_FILE)[0] + '.log')))

        if not os.path.isdir(tmp_dir):
            os.makedirs(tmp_dir)

        if not os.path.isdir(bkp_design_dir):
            os.makedirs(bkp_design_dir)

        #######################################################################################################

        step = 'get system info'
        start_time = start_step('GEN_PACKAGE-39', step)

        # Get System info
        config['system'] = {}

        # Get config['system']['distribution_id']
        cmd = [ 'lsb_release', '-is']
        log_file_name = os.path.abspath(os.path.join(output_dir, 'lsb_release_is.log'))
        log_info('GEN_PACKAGE-5', 'Get distribution ID')
        exec_step_cmd('GEN_PACKAGE-15', step, cmd, log_file_name)
        check_output_file_exists('GEN_PACKAGE-5', log_file_name)

        log_file = open(log_file_name, mode='r')
        for line in log_file:
            config['system']['distribution_id'] = line.split('\n')[0]
            break
        log_file.close()

        if config['system']['distribution_id'] not in SUPPORTED_DIST_ID:
            exit_error('GEN_PACKAGE-14', 'Invalid Distribution ID: ' + config['system']['distribution_id'] + '. Supported values are ' + str(SUPPORTED_DIST_ID))

        log_info('GEN_PACKAGE-16', 'Current distribution ID: ' + config['system']['distribution_id'])

        # Get config['system']['distribution_release']
        cmd = [ 'lsb_release', '-rs']
        log_file_name = os.path.abspath(os.path.join(output_dir, 'lsb_release_rs.log'))
        exec_step_cmd('GEN_PACKAGE-15', step, cmd, log_file_name)
        log_file = open(log_file_name, mode='r')
        for line in log_file:
            config['system']['distribution_release'] = line.split('\n')[0]
            break
        log_file.close()

        # Only use the major release number of CentOS and RedHat
        if config['system']['distribution_id'] in [DIST_ID_CENTOS, DIST_ID_REDHAT, DIST_ID_REDHAT2]:
            distribution_release_split = config['system']['distribution_release'].split('.')
            config['system']['distribution_release'] = distribution_release_split[0] + '.x'

        # architecture
        if config['system']['distribution_id'] in DIST_DEB:
            cmd = [ 'dpkg', '--print-architecture']
            log_file_name = os.path.abspath(os.path.join(output_dir, 'dpkg_print_architecture.log'))
            exec_step_cmd('GEN_PACKAGE-15', step, cmd, log_file_name)
            log_file = open(log_file_name, mode='r')
            for line in log_file:
                config['system']['architecture'] = line.split('\n')[0]
                break
            log_file.close()
        else:
            config['system']['architecture'] = platform.machine() # 'x86_64'

        if config['system']['architecture'] not in SUPPORTED_ARCH:
            exit_error('GEN_PACKAGE-17', 'Invalid architecture: ' + config['system']['architecture'] + '. Supported values are ' + str(SUPPORTED_ARCH))

        log_info('GEN_PACKAGE-18', 'System:')
        log_info('GEN_PACKAGE-18', '\t - Distribution ID: ' + config['system']['distribution_id'])
        log_info('GEN_PACKAGE-18', '\t - Distribution release: ' + config['system']['distribution_release'])
        log_info('GEN_PACKAGE-18', '\t - Architecture: ' + config['system']['architecture'])

        end_step('GEN_PACKAGE-38', start_time)

        #######################################################################################################

        # Init
        config['vendor']            = {}
        config['vendor']['name']    = 'xilinx'
        config['vendor']['full']    = 'Xilinx Inc'
        config['vendor']['email']   = 'support@xilinx.com'

        # for AMI build as part of aved_build flow - do not run genVersion for AMI or GCQ - these are run in advance
        if not opt.no_gen_version :
            # Get package version
            step = 'get AMI version'
            start_time = start_step('GET_VER', step)
            get_ver = './getVersion.sh ami'
            exec_step_cmd('GEN_VERSION', step, get_ver, shell=True)
            check_output_file_exists('GET_VER', join('api', 'include', 'ami_version.h'))
            end_step('GET_VER', start_time)

            # Get GCQ driver version
            step = 'get GCQ version'
            start_time = start_step('GET_VER', step)
            get_ver = './getVersion.sh gcq'
            exec_step_cmd('GEN_VERSION', step, get_ver, shell=True, cwd=join('driver', 'gcq-driver'))
            check_output_file_exists('GET_VER', join('driver', 'gcq-driver', 'src', 'gcq_version.h'))
            end_step('GET_VER', start_time)

        # If a directory is present in this dictionary, we will only include the
        # listed files in the driver sources. Useful if we don't want to bundle
        # all .c/.h files in a directory. Preference is given to the longest
        # directory match (i.e., a filter for 'driver/fal/gcq' takes priority
        # over 'driver/fal').
        driver_filter = {
            'driver/fal': ['fw_if.h', 'gcq/fw_if_gcq.h', 'gcq/fw_if_gcq_linux.c']
        }

        # Find driver sources
        for path, _, files in walk('driver'):
            src_filter = find_longest_filter(path, driver_filter)

            for name in files:
                include_file = False

                if src_filter:
                    for f in driver_filter[src_filter]:
                        if join(src_filter, f) == join(path, name):
                            include_file = True
                else:
                    include_file = True

                if not include_file or not name.endswith(('.c', '.h')):
                    continue

                driver.append(join(path, name))

        # Find API sources
        for path, _, files in walk(join('api', 'include')):
            for name in files:
                api_headers.append(join(path, name))

        config['package']                       = {}
        config['package']['name']               = 'ami'
        config['package']['release']            = opt.pkg_release
        config['package']['summary']            = config['package']['name'] + ' driver package'
        config['package']['description']        = [
            config['package']['name'] + ' driver package',
            'Built on ' + build_date_short + '.'
        ]
        config['package']['changelog']          = config['package']['name'] + ' driver package. Built on $build_date_short.'

        # Find version from generated header file
        with open(join('api', 'include', 'ami_version.h'), 'r') as fd:
                data = fd.read()

                v = re.findall(r'GIT_TAG.*?\"(\d+\.\d+\.\d+).*\"$', data, re.M)
                c = re.findall(r'GIT_TAG_VER_DEV_COMMITS.*?\((\d+)\)$', data, re.M)
                h = re.findall(r'GIT_HASH.*?\"(.*?)\"$', data, re.M)

                # Set version
                config['package']['version'] = v[0] if v else '0.0.0'

                # Set release
                config['package']['release'] = f'{c[0] if c else 0}.{h[0][:8] if h else ""}.{opt.pkg_release}'

        with open(os.path.abspath(os.path.join(SCRIPT_DIR, 'build', 'postinst.sh')), 'r') as infile:
            fdata = infile.read()
            fdata = fdata.replace('#!/bin/sh',                  '')
            fdata = fdata.replace('MODULE_NAME=$1',             'MODULE_NAME='+ config['package']['name'])
            fdata = fdata.replace('MODULE_VERSION_STRING=$2',   'MODULE_VERSION_STRING='+ config['package']['version'])
            config['package']['postinst'] = fdata.split('\n')
            with open(os.path.abspath(os.path.join(output_dir, 'postinst.sh')), 'w') as outfile:
                outfile.write('\n'.join(config['package']['postinst']))

        with open(os.path.abspath(os.path.join(SCRIPT_DIR, 'build', 'prerm.sh')), 'r') as infile:
            fdata = infile.read()
            fdata = fdata.replace('#!/bin/sh',                  '')
            fdata = fdata.replace('MODULE_NAME=$1',             'MODULE_NAME='+ config['package']['name'])
            fdata = fdata.replace('MODULE_VERSION_STRING=$2',   'MODULE_VERSION_STRING='+ config['package']['version'])
            config['package']['prerm'] = fdata.split('\n')
            with open(os.path.abspath(os.path.join(output_dir, 'prerm.sh')), 'w') as outfile:
                outfile.write('\n'.join(config['package']['prerm']))

        with open(os.path.abspath(os.path.join(SCRIPT_DIR, 'build', 'preinst.sh')), 'r') as infile:
            fdata = infile.read()
            fdata = fdata.replace('#!/bin/sh',                  '')
            fdata = fdata.replace('MODULE_NAME=$1',             'MODULE_NAME='+ config['package']['name'])
            fdata = fdata.replace('MODULE_VERSION_STRING=$2',   'MODULE_VERSION_STRING='+ config['package']['version'])
            config['package']['preinst'] = fdata.split('\n')
            with open(os.path.abspath(os.path.join(output_dir, 'preinst.sh')), 'w') as outfile:
                outfile.write('\n'.join(config['package']['preinst']))

        config['package']['dependency'] = {
            'rpm': ['glibc', 'gcc', 'make', 'dkms', 'grep', 'gawk', 'kernel-devel', 'kernel-headers'],
            'deb': ['libc6', 'gcc', 'make', 'dkms', 'grep', 'gawk', 'linux-headers']
        }

        # We want to conflict with XRT, however, we don't want the package manager to automatically
        # remove XRT and any packages that depend on it. This is handled correctly by `yum`` with RPM files,
        # but `apt`` tries to automatically uninstall XRT if it is detected. To get around this, we use a
        # 'preinst' scriptlet rather than the 'Conflicts' field in the control file.
        config['package']['conflicts'] = {}
        config['package']['conflicts']['rpm'] = ['xrt']
        config['package']['conflicts']['deb'] = []

        # for AMI build as part of aved_build flow - do not run driver generation within docker flow
        if not opt.no_driver :
            # Check that the driver will build
            step = 'driver compilation confidence check'
            start_time = start_step('BUILD_DRIVER', step)

            build_driver = 'cd driver && make clean && make'
            exec_step_cmd('BUILD_CHECK_DRIVER', step, build_driver, shell=True)
            check_output_file_exists('BUILD_CHECK_DRIVER', join('driver', 'ami.ko'))

            step = 'clean driver'
            clean_driver = 'cd driver && make clean'
            exec_step_cmd('CLEAN_DRIVER', step, clean_driver, shell=True)

            end_step('BUILD_DRIVER', start_time)

        # Build libami.a and ami_tool
        step = 'build AMI API and tools'
        start_time = start_step('BUILD_AMI', step)

        build_api = 'cd api && make clean && make'
        exec_step_cmd('BUILD_LIBAMI', step, build_api, shell=True)
        check_output_file_exists('BUILD_LIBAMI', join('api', 'build', 'libami.a'))

        build_ami_tool = 'cd app && make clean && make'
        exec_step_cmd('BUILD_AMI_TOOL', step, build_ami_tool, shell=True)
        check_output_file_exists('BUILD_AMI_TOOL', join('app', 'build', 'ami_tool'))

        end_step('BUILD_AMI', start_time)

        # Define package content paths
        config['package']['usr_src_dir']        = 'usr/src/' + config['package']['name'] + '-' + config['package']['version']
        config['package']['usr_src_Makefile']   = config['package']['usr_src_dir'] + '/driver/Makefile'
        config['package']['usr_src_files']      = [f'{config["package"]["usr_src_dir"]}/{f}' for f in driver]
        config['package']['usr_src_dkms_conf']  = config['package']['usr_src_dir'] + '/dkms.conf'

        config['package']['usr_include_dir']    = 'usr/include/' + config['package']['name']
        config['package']['usr_include_h']      = [f'{config["package"]["usr_include_dir"]}/{os.path.basename(f)}' for f in api_headers]

        config['package']['usr_bin_dir']        = 'usr/local/bin/'
        config['package']['usr_lib_dir']        = 'usr/local/lib/'

        config['package']['usr_bin'] = config['package']['usr_bin_dir'] + '/ami_tool'
        config['package']['usr_lib'] = config['package']['usr_lib_dir'] + '/libami.a'

        if config['system']['distribution_id'] in DIST_RPM:
            config['package']['pkg_config_dir'] = 'usr/lib64/pkgconfig'
        elif config['system']['distribution_id'] in DIST_DEB:
            config['package']['pkg_config_dir'] = 'usr/lib/pkgconfig'
        else:
            config['package']['pkg_config_dir'] = 'usr/share/pkgconfig'
        config['package']['pkg_config_pc']  = config['package']['pkg_config_dir'] + '/ami.pc'

        # Define package description metadata
        config['package']['description']  = [
            'Xilinx Inc ' + config['package']['name'] + ' driver package.',
            'Built on ' + str(build_date_short) + '.',
            'Built with ' + config['system']['distribution_id'] + ' version ' + config['system']['distribution_release'] + ' and architecture ' + config['system']['architecture'] + '.',
        ]

        # Create the file necessary to generate the packages
        if config['system']['distribution_id'] in DIST_RPM:
            for dirname in ['BUILDROOT', 'RPMS', 'SOURCES', 'SPECS', 'SRPMS', 'BUILD']:
                dir = os.path.abspath(os.path.join(output_dir, dirname))
                os.makedirs(dir)
                os.chmod(dir, 493); # octal 0755
                check_output_dir_exists('GEN_PACKAGE-5', dir)

            dest_base = os.path.abspath(os.path.join(output_dir, 'BUILD'))

            # Create specfile
            SPEC_FILE = []
            SPEC_FILE += [ '# Turn off the brp-python-bytecompile script' ]
            SPEC_FILE += [ "%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*\$!!g')" ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ 'Name: '                 + config['package']['name'] ]
            SPEC_FILE += [ 'Version: '              + config['package']['version'] ]
            SPEC_FILE += [ 'Release: '              + config['package']['release'] ]
            SPEC_FILE += [ 'Vendor: '               + config['vendor']['full'] ]
            SPEC_FILE += [ 'License: '              + config['vendor']['name'] ]
            SPEC_FILE += [ 'Summary: '              + config['package']['summary'] ]
            SPEC_FILE += [ 'BuildArchitectures: '   + config['system']['architecture'] ]
            SPEC_FILE += [ 'Buildroot: %{_topdir}' ]
            for dep in config['package']['dependency']['rpm']:
                SPEC_FILE += [ 'Requires: ' + dep ]
            SPEC_FILE += [ '' ]
            for conflict in config['package']['conflicts']['rpm']:
                SPEC_FILE += [ 'Conflicts: ' + conflict ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%description' ]
            SPEC_FILE += config['package']['description']
            SPEC_FILE += [ '' ]

            SPEC_FILE += [ '%install' ]
            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + config['package']['usr_src_dir'] + '/driver' ]
            SPEC_FILE += [ 'install -m 0644 ' + config['package']['usr_src_Makefile'] + ' %{buildroot}/' + config['package']['usr_src_Makefile'] ]

            dirs_done = []
            for file in config['package']['usr_src_files']:
                d = os.path.dirname(file)
                if d not in dirs_done:
                    SPEC_FILE += [ 'mkdir -p %{buildroot}/' + d]
                    dirs_done.append(d)

                SPEC_FILE += [ 'install -m 0644 ' + file + ' %{buildroot}/' + file ]

            SPEC_FILE += [ 'install -m 0644 ' + config['package']['usr_src_dkms_conf'] + ' %{buildroot}/' + config['package']['usr_src_dkms_conf'] ]

            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + config['package']['usr_include_dir'] ]

            for file in config['package']['usr_include_h']:
                SPEC_FILE += [ 'install -m 0644 ' + file  + ' %{buildroot}/' + file ]

            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + config['package']['usr_lib_dir'] ]
            SPEC_FILE += [ 'install -m 0644 ' + config['package']['usr_lib'] + ' %{buildroot}/' + config['package']['usr_lib'] ]

            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + config['package']['usr_bin_dir'] ]
            SPEC_FILE += [ 'install -m 0755 ' + config['package']['usr_bin'] + ' %{buildroot}/' + config['package']['usr_bin'] ]

            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + config['package']['pkg_config_dir'] ]
            SPEC_FILE += [ 'install -m 0644 ' + config['package']['pkg_config_pc'] + ' %{buildroot}/' + config['package']['pkg_config_pc'] ]

            SPEC_FILE += [ '' ]

            SPEC_FILE += [ '%posttrans' ]
            SPEC_FILE += config['package']['postinst']
            SPEC_FILE += [ '' ]

            SPEC_FILE += [ '%preun' ]
            SPEC_FILE += config['package']['prerm']
            SPEC_FILE += [ '' ]

            SPEC_FILE += [ '%files' ]
            SPEC_FILE += [ '%defattr(-,root,root)' ]
            SPEC_FILE += [ '/' + config['package']['usr_src_dir'] ]
            SPEC_FILE += [ '/' + config['package']['usr_include_dir'] ]
            SPEC_FILE += [ '/' + config['package']['pkg_config_pc'] ]
            SPEC_FILE += [ '/' + config['package']['usr_lib_dir'] ]
            SPEC_FILE += [ '/' + config['package']['usr_bin_dir'] ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%changelog' ]
            SPEC_FILE += [ '* ' + build_date_short + ' ' + config['vendor']['full'] + ' ' + config['vendor']['email'] + '> - ' + config['package']['version'] + '-' + config['package']['release'] ]
            SPEC_FILE += [ '- ' + config['package']['changelog'] ]

            spec_file_name = os.path.abspath(os.path.join(output_dir, 'SPECS', 'specfile.spec'))
            log_info('GEN_PACKAGE-25', 'Writing spec file: ' + spec_file_name)
            with open(spec_file_name, mode='w') as outfile:
                outfile.write('\n'.join(SPEC_FILE))
            check_output_file_exists('GEN_PACKAGE-5', spec_file_name)

        else:
            config['package']['description']    = '\n '.join(config['package']['description']); # DEB package format requires \n+SPACE

            deb_name = config['package']['name'] + '_' + config['package']['version'] + '-' + config['package']['release'] + '_' + config['system']['architecture']

            dest_base  = os.path.abspath(os.path.join(output_dir, deb_name))
            debian_dir = os.path.abspath(os.path.join(dest_base, 'DEBIAN'))
            os.makedirs(dest_base)
            os.chmod(dest_base, 493); # octal 0755
            os.makedirs(debian_dir)
            check_output_dir_exists('GEN_PACKAGE-5', dest_base)
            check_output_dir_exists('GEN_PACKAGE-5', debian_dir)

            # Create control file
            CONTROL = []
            CONTROL += [ 'Package: '        + config['package']['name'] ]
            CONTROL += [ 'Architecture: '   + config['system']['architecture'] ]
            CONTROL += [ 'Version: '        + config['package']['version'] + '-' + config['package']['release'] ]
            CONTROL += [ 'Priority: optional' ]
            CONTROL += [ 'Description: '    + config['package']['description'] ]
            CONTROL += [ 'Maintainer: '     + config['vendor']['full']]
            CONTROL += [ 'Section: devel' ]
            if len(config['package']['dependency']['deb']) > 0:
                CONTROL += [ 'Depends: ' + ', '.join(config['package']['dependency']['deb']) ]
            if len(config['package']['conflicts']['deb']) > 0:
                CONTROL += [ 'Conflicts: ' + ', '.join(config['package']['conflicts']['deb']) ]
            CONTROL += [ '' ]

            control_file_name = os.path.abspath(os.path.join(debian_dir, 'control'))
            log_info('GEN_PACKAGE-26', 'Writing control file: ' + control_file_name)
            with open(control_file_name, mode='w') as outfile:
                outfile.write('\n'.join(CONTROL))
            check_output_file_exists('GEN_PACKAGE-5', control_file_name)

            # Creating postinst file
            POSTINST = [
                '#!/bin/bash',
                'set -e',
            ]
            POSTINST += config['package']['postinst']

            postinst_file_name = os.path.abspath(os.path.join(debian_dir, 'postinst'))
            log_info('GEN_PACKAGE-28', 'Writing postinst file: ' + postinst_file_name)
            with open(postinst_file_name, mode='w') as outfile:
                outfile.write('\n'.join(POSTINST))
            os.chmod(postinst_file_name, 509); # octal 775
            check_output_file_exists('GEN_PACKAGE-5', postinst_file_name)

            # Create prerm file
            PRERM = [
                '#!/bin/bash',
                'set -e',
            ]
            PRERM += config['package']['prerm']

            prerm_file_name = os.path.abspath(os.path.join(debian_dir, 'prerm'))
            log_info('GEN_PACKAGE-28', 'Writing prerm file: ' + prerm_file_name)
            with open(prerm_file_name, mode='w') as outfile:
                outfile.write('\n'.join(PRERM))
            os.chmod(prerm_file_name, 509); # octal 775
            check_output_file_exists('GEN_PACKAGE-5', prerm_file_name)

             # Create preinst file
            PREINST = [
                '#!/bin/bash',
                'set -e',
            ]
            PREINST += config['package']['preinst']

            preinst_file_name = os.path.abspath(os.path.join(debian_dir, 'preinst'))
            log_info('GEN_PACKAGE-28', 'Writing preinst file: ' + preinst_file_name)
            with open(preinst_file_name, mode='w') as outfile:
                outfile.write('\n'.join(PREINST))
            os.chmod(preinst_file_name, 509); # octal 775
            check_output_file_exists('GEN_PACKAGE-5', preinst_file_name)

            # Create changelog
            CHANGE_LOG = []
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ config['package']['name'] + ' (' + config['package']['version'] + '-' + config['package']['release'] + ') xilinx; urgency=medium' ]
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ '  * ' + config['package']['changelog'] ]
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ '-- ' + config['vendor']['full']+' <' + config['vendor']['email'] + '> ' + build_date_short + ' 00:00:00 +0000' ]
            CHANGE_LOG += [ '' ]

            changelog_dir       = os.path.abspath(os.path.join(dest_base, 'usr', 'share', 'doc', config['package']['name']))
            changelog_file_name = os.path.abspath(os.path.join(changelog_dir, 'changelog.Debian'))
            changelog_tar_name  = os.path.abspath(os.path.join(changelog_dir, 'changelog.Debian.gz'))

            os.makedirs(changelog_dir)

            log_info('GEN_PACKAGE-29', 'Writing changelog file: ' + changelog_file_name)
            with open(changelog_file_name, mode='w') as outfile:
                outfile.write('\n'.join(CHANGE_LOG))

            with tarfile.open(changelog_tar_name, "w:gz") as tar:
                tar.add(changelog_file_name)
            os.remove(changelog_file_name)

            check_output_file_exists('GEN_PACKAGE-5', changelog_tar_name)

        driver_dest = [
            {
                'src': os.path.abspath(join(SCRIPT_DIR, f)),
                'dst': join(config['package']['usr_src_dir'], os.path.dirname(f))
            } for f in driver
        ]

        api_dest = [
            {
                'src': os.path.abspath(join(SCRIPT_DIR, f)),
                'dst': join(config['package']['usr_include_dir'])
            } for f in api_headers
        ]

        # Copying packaged files
        SRC_DEST_LIST = [
            {'src': os.path.abspath(join(SCRIPT_DIR, 'driver', 'Makefile')), 'dst': join(config['package']['usr_src_dir'], 'driver')},
            {'src': os.path.abspath(join(SCRIPT_DIR, 'build',  'dkms.conf')),'dst': config['package']['usr_src_dir']},
            {'src': os.path.abspath(join(SCRIPT_DIR, 'api',  'build', 'libami.a')),'dst': config['package']['usr_lib_dir']},
            {'src': os.path.abspath(join(SCRIPT_DIR, 'app',  'build', 'ami_tool')),'dst': config['package']['usr_bin_dir']},
            *api_dest,
            *driver_dest
        ]

        for src_dest in SRC_DEST_LIST:
            src = src_dest['src']
            dst = dest_base
            if src_dest['dst'] != '':
                dst = os.path.abspath(os.path.join(dst, src_dest['dst']))
            copy_source_file('GEN_PACKAGE-31', src, dst)
            check_output_file_exists('GEN_PACKAGE-5', os.path.abspath(os.path.join(dst, os.path.basename(src))))

            # Replace version in dkms.conf
            if src.endswith('dkms.conf'):
                with open(src, 'r') as dkms_src:
                    conf = dkms_src.read()
                    with open(join(dst, os.path.basename(src)), 'w') as dkms_dest:
                        new_conf = conf.replace('@PKGVER@', config['package']['version'])
                        dkms_dest.write(new_conf)

        # Create module PC file
        MODULE_PC = [
            'bindir=/'+config['package']['usr_bin_dir'],
            'libdir=/'+config['package']['usr_lib_dir'],
            'includedir=/'+config['package']['usr_include_dir'],
            '',
            'Name: ' + config['package']['name'],
            'Description: ' + config['package']['summary'],
            'Version: ' + config['package']['version'],
            'Libs: -L${libdir} -lami',
            'Cflags: -I${includedir}'

        ]
        os.makedirs(os.path.abspath(os.path.join(dest_base, config['package']['pkg_config_dir'])))
        module_pc_file_name = os.path.abspath(os.path.join(dest_base, config['package']['pkg_config_pc']))
        log_info('GEN_PACKAGE-28', 'Writing module PC file: ' + module_pc_file_name)
        with open(module_pc_file_name, mode='w') as outfile:
            outfile.write('\n'.join(MODULE_PC))
        check_output_file_exists('GEN_PACKAGE-5', module_pc_file_name)

        # Generate package
        if config['system']['distribution_id'] in DIST_RPM:
            cmd = [
                'rpmbuild', '--verbose',
                            '--define', '_topdir ' + output_dir,
                            '-bb',      spec_file_name
            ]
            log_file_name = os.path.abspath(os.path.join(log_dir, 'rpmbuild.log'))

            pkg_name  = config['package']['name'] + '-' + config['package']['version'] + '-' + config['package']['release'] + '.' + config['system']['architecture']
            pkg  = os.path.abspath(os.path.join(output_dir, 'RPMS', config['system']['architecture'], pkg_name + '.rpm'))
            pkg_cpy  = os.path.abspath(os.path.join(output_dir, pkg_name + '_' + config['system']['distribution_release'] + '.rpm'))
        else:
            cmd = [ 'dpkg-deb', '--build', '--root-owner-group', join(output_dir, deb_name) ]
            log_file_name = os.path.abspath(os.path.join(log_dir, 'dpkg-deb.log'))

            pkg_name = config['package']['name'] + '_' + config['package']['version'] + '-' + config['package']['release'] + '_' + config['system']['architecture']
            pkg     = os.path.abspath(os.path.join(output_dir, pkg_name + '.deb'))
            pkg_cpy = os.path.abspath(os.path.join(output_dir, pkg_name + '_' + config['system']['distribution_release'] + '.deb'))

        exec_step_cmd('GEN_PACKAGE-37', step, cmd, log_file_name)
        check_output_file_exists('GEN_PACKAGE-5', pkg)
        shutil.copyfile(pkg, pkg_cpy)
        os.remove(pkg)
        check_output_file_exists('GEN_PACKAGE-5', pkg_cpy)

        end_step('GEN_PACKAGE-38', start_time)

        tear_down('GEN_PACKAGE-8', SCRIPT_FILE, script_start_time)

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
