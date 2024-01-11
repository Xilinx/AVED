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

import os
import sys

INTERNAL_RELEASE = False

SCRIPT_VERSION = '1.0'
SCRIPT_FILE    = os.path.basename(__file__)
SCRIPT_DIR     = os.path.dirname(os.path.realpath(__file__))

SW_DIR = os.path.abspath(os.path.join(os.path.split(SCRIPT_DIR)[0], 'sw'))

BUILD_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, 'build'))
sys.path.insert(0, BUILD_DIR)

from pkg import *

DEFAULT_PKG_RELEASE = 4064028

class Options(object):

    def PrintVersion(self):
        log_info('GEN_RPM-5', 'Script ' + SCRIPT_FILE + ' version: ' + SCRIPT_VERSION)

    def printHelp(self):
        log_info('GEN_RPM-6', 'Usage: $ python3 ' + SCRIPT_FILE + ' [options]')
        log_info('GEN_RPM-6', '\t--help              / -h: Display this message')
        log_info('GEN_RPM-6', '\t--version           / -v: Display version')
        log_info('GEN_RPM-6', '\t--verbose           / -V: Turn on verbosity')
        log_info('GEN_RPM-6', '\t--force             / -f: Override output directory if already existing')
        log_info('GEN_RPM-6', '\t--output_dir        / -o: Path to the output directory. Default ./output/<date>_<time>/')
        log_info('GEN_RPM-6', '\t--pkg_release       / -r: Package release. Default ' + str(DEFAULT_PKG_RELEASE))
        log_info('GEN_RPM-6', '\t--g_code_coverage   / -C: Compile host application with --code_coverage flag')
        log_info('GEN_RPM-6', '\t--g_debug           / -d: Compile host application with debug flag to include symboles for debugger')
        log_info('GEN_RPM-6', '\t--use_xrt           / -X: Generate xbtest which use XRT. (use AMI by default)')
        log_info('GEN_RPM-6', 'Examples:')
        log_info('GEN_RPM-6', '\t python3 ' + SCRIPT_FILE)
        log_info('GEN_RPM-6', 'Expects XRT or AMI installed on host')

    def __init__(self):
        self.help = False
        self.version = False
        self.verbose = False
        self.force = False
        self.output_dir = None
        self.pkg_release = str(DEFAULT_PKG_RELEASE)
        self.g_code_coverage = False
        self.g_debug = False
        self.use_xrt = False

    def getOptions(self, argv):
        log_info('GEN_RPM-53', 'Command line provided: $ ' + str(sys.executable) + ' ' + ' '.join(argv))
        try:
            options, remainder = getopt.gnu_getopt(
                argv[1:],
                'hvVfo:r:CdX',
                [
                    'help',
                    'version',
                    'verbose',
                    'force',
                    'output_dir=',
                    'pkg_release=',
                    'g_code_coverage',
                    'g_debug',
                    'use_xrt',
                ]
            )
        except getopt.GetoptError as e:
            self.printHelp()
            exit_error('GEN_RPM-1', str(e))

        log_info('GEN_RPM-54', 'Parsing command line options')
        for opt, arg in options:
            msg = '\t' + str(opt)
            if arg is not None:
                msg += ' ' + str(arg)
            log_info('GEN_RPM-54', msg)

            if opt in ('--help', '-h'):
                self.printHelp()
                self.help = True
            elif opt in ('--version', '-v'):
                self.PrintVersion()
                self.version = True
            elif opt in ('--verbose', '-V'):
                setup_verbose()
                self.verbose = True
            elif opt in ('--force', '-f'):
                self.force = True
            elif opt in ('--output_dir', '-o'):
                self.output_dir = str(arg)
                self.output_dir = os.path.abspath(self.output_dir)
            elif opt in ('--pkg_release', '-r'):
                self.pkg_release = str(arg)
            elif opt in ('--g_code_coverage', '-C'):
                self.g_code_coverage = True
            elif opt in ('--g_debug', '-d'):
                self.g_debug = True
            elif opt in ('--use_xrt', '-X'):
                self.use_xrt = True
            else:
                exit_error('GEN_RPM-2', 'Command line option not handled: ' + str(opt))

        if len(remainder) > 0:
            self.printHelp()
            exit_error('GEN_RPM-3', 'Unknown command line options: ' + ' '.join(remainder))

        if self.help or self.version:
            exit_info('GEN_RPM-4', 'Script terminating as help/version option provided')

def main(args):
    opt = Options()
    opt.getOptions(args)

    env = os.environ.copy()

    try:
        script_start_time = start('GEN_RPM-7', SCRIPT_FILE)

        # Create output directory
        if opt.output_dir is not None:
            output_dir = opt.output_dir
        else:
            now = datetime.datetime.now()
            output_dir =        str(now.year)          + '-' + str(now.month).zfill(2)  + '-' + str(now.day).zfill(2)
            output_dir += '_' + str(now.hour).zfill(2) + '-' + str(now.minute).zfill(2) + '-' + str(now.second).zfill(2)
            output_dir = os.path.abspath(os.path.join(CWD, 'output', output_dir))

        if os.path.isdir(output_dir):
            if not opt.force:
                exit_error('GEN_RPM-11', 'Output directory already exists (see --force to override): ' + output_dir)
            else:
                log_info('GEN_RPM-51', 'Removing output directory already existing as --force option is provided: ' + output_dir)
                try:
                    shutil.rmtree(output_dir)
                except OSError as e:
                    exit_error('GEN_RPM-52', 'Failed to remove output directory: ' + output_dir + '. Exception caught: ' + str(e))

                if os.path.isdir(output_dir):
                    exit_error('GEN_RPM-52', 'Failed to remove output directory: ' + output_dir + '. Directory still exists')

        log_info('GEN_RPM-12', 'Creating output directory: ' + output_dir)
        os.makedirs(output_dir)
        setup_logfile(os.path.abspath(os.path.join(output_dir, os.path.splitext(SCRIPT_FILE)[0] + '.log')))

        os.chdir(output_dir)

        # Get date, distribution ID, release, architecture
        build_date=get_date_long()
        build_date_short=get_date_short()

        # Get distribution_id
        step = 'get distribution ID'
        start_time = start_step('GEN_RPM-14', step)
        cmd = [ 'lsb_release', '-is']
        log_file_name = os.path.abspath(os.path.join(output_dir, 'lsb_release_is.log'))
        exec_step_cmd('GEN_RPM-15', step, cmd, log_file_name)
        log_file = open(log_file_name, mode='r')
        for line in log_file:
            distribution_id = line.split('\n')[0]
            break
        log_file.close()

        if distribution_id not in SUPPORTED_DIST_ID:
            exit_error('GEN_RPM-13', 'Invalid Distribution ID: ' + distribution_id + '. Supported values are ' + str(SUPPORTED_DIST_ID))

        end_step('GEN_RPM-16', start_time)

        # Get distribution_release
        step = 'get distribution release'
        start_time = start_step('GEN_RPM-14', step)
        cmd = [ 'lsb_release', '-rs']
        log_file_name = os.path.abspath(os.path.join(output_dir, 'lsb_release_rs.log'))
        exec_step_cmd('GEN_RPM-15', step, cmd, log_file_name)
        log_file = open(log_file_name, mode='r')
        for line in log_file:
            distribution_release = line.split('\n')[0]
            break
        log_file.close()

        # Only use the major release number of CentOS and RedHat
        if distribution_id in [DIST_ID_CENTOS, DIST_ID_REDHAT, DIST_ID_REDHAT2]:
            distribution_release_split = distribution_release.split('.')
            distribution_release       = distribution_release_split[0] + '.x'

        end_step('GEN_RPM-16', start_time)

        # architecture
        step = 'get architecture'
        start_time = start_step('GEN_RPM-14', step)
        if distribution_id in DIST_DEB:
            cmd = [ 'dpkg', '--print-architecture']
            log_file_name = os.path.abspath(os.path.join(output_dir, 'dpkg_print_architecture.log'))
            exec_step_cmd('GEN_RPM-15', step, cmd, log_file_name)
            log_file = open(log_file_name, mode='r')
            for line in log_file:
                architecture = line.split('\n')[0]
                break
            log_file.close()
        else:
            architecture = platform.machine() # 'x86_64'

        if architecture not in SUPPORTED_ARCH:
            exit_error('GEN_RPM-17', 'Invalid architecture: ' + architecture + '. Supported values are ' + str(SUPPORTED_ARCH))

        end_step('GEN_RPM-16', start_time)

        log_info('GEN_RPM-18', 'System:')
        log_info('GEN_RPM-18', '\t - Distribution ID: ' + distribution_id)
        log_info('GEN_RPM-18', '\t - Distribution release: ' + distribution_release)
        log_info('GEN_RPM-18', '\t - Architecture: ' + architecture)

        # Copy source code
        sw_dest_base = os.path.abspath(os.path.join(output_dir, 'sw'))
        copy_source_file('GEN_RPM-19', os.path.abspath(os.path.join(SW_DIR, 'makefile')), sw_dest_base)

        source_code_dir_list = [
            ['include', 'api'],
            ['include', 'share'],
            ['include', 'xbtest_common'],
            ['include', 'xbtest_sw'],
            ['src',     'api'],
            ['src',     'share'],
            ['src',     'xbtest_common'],
            ['src',     'xbtest_sw'],
        ]

        for dir_list in source_code_dir_list:
            sw_dest_dir = sw_dest_base
            sw_src_dir  = SW_DIR
            for dir in dir_list:
                sw_dest_dir = os.path.abspath(os.path.join(sw_dest_dir, dir))
                sw_src_dir  = os.path.abspath(os.path.join(sw_src_dir,  dir))
                if not os.path.isdir(sw_dest_dir):
                    os.makedirs(sw_dest_dir)
            copy_source_dir('GEN_RPM-19', sw_src_dir, sw_dest_dir)

        # Generate host application executable
        step = 'compile xbtest host application executable'
        start_time = start_step('GEN_RPM-20', step)

        exe = os.path.abspath(os.path.join(sw_dest_base, 'output', 'xbtest'))

        log_info('GEN_RPM-22', 'Compilation parameters of xbtest host application:')
        log_info('GEN_RPM-22', '\t - Build date: ' + build_date)
        log_info('GEN_RPM-22', '\t - Internal release: ' + str(INTERNAL_RELEASE))

        log_info('GEN_RPM-21', 'xbtest host application executables will be generated: ' + exe)

        cmd  = ['make']
        cmd += ['-j']
        cmd += ['build_version=' + str(opt.pkg_release)]
        cmd += ['build_date=' + build_date]
        if opt.g_code_coverage:
            cmd += ['code_coverage=true']
        if opt.g_debug:
            cmd += ['debug=true']
        if INTERNAL_RELEASE:
            cmd += ['internal_release=true']
        if opt.use_xrt:
            cmd += ['xrt=true']

        os.chdir(sw_dest_base)

        log_file_name = os.path.abspath(os.path.join(output_dir, 'compile.log'))
        exec_step_cmd('GEN_RPM-23', step, cmd, log_file_name=log_file_name, use_console=opt.verbose)

        if not os.path.isfile(exe):
            exit_error('GEN_RPM-24', 'xbtest host application does not exist: ' + exe)
        else:
            log_info('GEN_RPM-25', 'xbtest host application successfully generated: ' + exe)

        os.chdir(output_dir)

        end_step('GEN_RPM-26', start_time)

        # Get the host application major/minor versions
        step = 'execute host application to get major/minor version'
        start_time = start_step('GEN_RPM-27', step)
        cmd = exe + ' -v'
        log_file_name = os.path.abspath(os.path.join(output_dir, 'get_xbtest_version.log'))
        env['XILINX_XBTEST'] = os.path.abspath(os.path.join(sw_dest_base, 'output'))
        exec_step_cmd('GEN_RPM-28', step, cmd, log_file_name=log_file_name, shell = True, ignore_error = True, env = env)
        # Get version in
        # INFO      :: GEN_003 :: COMMAND_LINE :: <exe> host code version <major>.<minor>.<patch>
        major = None
        minor = None
        log_file = open(log_file_name, mode='r')
        for line in log_file:
            if 'GEN_003' in line:
                line = line.split('\n')[0]
                line = line.split('host code version ')[-1]
                version = line.split('.')
                major = version[0]
                minor = version[1]
                break
        log_file.close()

        if major is None:
            exit_error('GEN_RPM-29', 'Failed to find host application major version')
        if minor is None:
            exit_error('GEN_RPM-30', 'Failed to find host application minor version')

        log_info('GEN_RPM-31', 'Found host application version:')
        log_info('GEN_RPM-31', '\t - Major version: ' + major)
        log_info('GEN_RPM-31', '\t - Minor version: ' + minor)
        end_step('GEN_RPM-32', start_time)

        # Get the package metadata define in the xbtest_cfg package
        package = {}
        package[NAME]           = 'xbtest'
        package[VERSION]        = str(major) + '.' + str(minor)
        package[RELEASE]        = str(opt.pkg_release)
        if INTERNAL_RELEASE:
            package[RELEASE] += '.INTERNAL'
        package[SUMMARY]        = 'xbtest package'

        package[DESCRIPTION] = []
        if INTERNAL_RELEASE:
            package[DESCRIPTION] += [ '**INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL**' ]
        package[DESCRIPTION] += [ 'Xilinx Inc xbtest v' + str(major) + '.' + str(minor) + ' xbtest package.' ]
        package[DESCRIPTION] += [ 'Built on ' + str(build_date_short) + '.' ]
        package[DESCRIPTION] += [ 'Built from source CL ' + str(opt.pkg_release) + '.' ]
        package[DESCRIPTION] += [ 'Built with ' + distribution_id + ' version ' + distribution_release + ' and architecture ' + architecture + '.' ]
        if opt.use_xrt:
            package[DESCRIPTION] += [ 'Generated to use with XRT.' ]
        else:
            package[DESCRIPTION] += [ 'Generated to use with AMI.' ]
        if INTERNAL_RELEASE:
            package[DESCRIPTION] += [ '**INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL ** INTERNAL**' ]
        package[DESCRIPTION] = '\n'.join(package[DESCRIPTION])

        package[CHANGELOG]      = 'Release of xbtest v' + str(major) + '.' + str(minor)

        package[PRE_INST_MSG] = []
        package[PRE_INST_MSG] += [ '' ]
        package[PRE_INST_MSG] += [ 'Installing xbtest package']
        package[PRE_INST_MSG] += [ '' ]
        package[PRE_INST_MSG] = '\n'.join(        package[PRE_INST_MSG])

        package[POST_INST_MSG] = []
        package[POST_INST_MSG] += [ '' ]
        package[POST_INST_MSG] += [ 'xbtest package installed successfully' ]
        package[POST_INST_MSG] += [ '' ]
        package[POST_INST_MSG] += [ 'To run xbtest:' ]
        package[POST_INST_MSG] += [ '' ]
        package[POST_INST_MSG] += [ '\t1) Setup environment:' ]
        package[POST_INST_MSG] += [ '\t\t* If using csh, refresh your environment by running rehash. Otherwise, refer to the manual for your shell or start a new session.' ]
        package[POST_INST_MSG] += [ '' ]
        package[POST_INST_MSG] += [ '\t2) Run verify test on card with BDF <bdf>:' ]
        package[POST_INST_MSG] += [ '\t\txbtest -d <BDF> -c verify' ]
        package[POST_INST_MSG] += [ '' ]
        package[POST_INST_MSG] += [ '\t3) For more info, try:' ]
        package[POST_INST_MSG] += [ '\t\txbtest -h' ]
        package[POST_INST_MSG] = '\n'.join(package[POST_INST_MSG])

        log_info('GEN_RPM-33', 'xbtest package metadata:')
        for key, value in package.items():
            log_info('GEN_RPM-33', '\t- ' + key + ' : ' + str(value))

        # Create the file necessary to generate the packages
        if distribution_id in DIST_RPM:
            for dirname in ['BUILDROOT', 'RPMS', 'SOURCES', 'SPECS', 'SRPMS', 'BUILD']:
                dir = os.path.abspath(os.path.join(output_dir, dirname))
                os.makedirs(dir)
                os.chmod(dir, 493); # octal 0755

            dest_base = os.path.abspath(os.path.join(output_dir, 'BUILD'))

            # create spec file
            spec_file_name = os.path.abspath(os.path.join(output_dir, 'SPECS', 'specfile.spec'))
            log_info('GEN_RPM-34', 'Creating xbtest spec file: ' + spec_file_name)

            SPEC_FILE = []
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '# Generated by xbtest ' + SCRIPT_FILE ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ 'Name: ' + package[NAME] ]
            SPEC_FILE += [ 'Version: ' + package[VERSION] ]
            SPEC_FILE += [ 'Release: ' + package[RELEASE] ]
            SPEC_FILE += [ 'Vendor: Xilinx Inc' ]
            SPEC_FILE += [ 'License: Apache' ]; # TODO check license OK
            SPEC_FILE += [ 'Summary: ' + package[SUMMARY] ]
            SPEC_FILE += [ 'BuildArchitectures: ' + architecture ]
            SPEC_FILE += [ 'Buildroot: %{_topdir}' ]
            if opt.use_xrt:
                SPEC_FILE += [ 'Requires: xrt' ]
            SPEC_FILE += [ 'Requires: ncurses' ]
            SPEC_FILE += [ 'Requires: json-glib' ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ 'Conflicts: xbtest-common' ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%description' ]
            SPEC_FILE += [ package[DESCRIPTION] ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%pre' ]
            SPEC_FILE += [ 'echo -e "' + package[PRE_INST_MSG] + '"' ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%post' ]
            SPEC_FILE += [ 'echo -e "' + package[POST_INST_MSG] + '"' ]
            SPEC_FILE += [ '' ]

            SPEC_FILE += [ '%install' ]
            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + OPT_INSTALL_DIR + '/license' ]
            SPEC_FILE += [ 'install -m 0644 ' + OPT_INSTALL_DIR + '/license/LICENSE     %{buildroot}/' + OPT_INSTALL_DIR + '/license/LICENSE' ]
            SPEC_FILE += [ 'install -m 0644 ' + OPT_INSTALL_DIR + '/README.md           %{buildroot}/' + OPT_INSTALL_DIR + '/README.md' ]

            # xbtest exe
            SPEC_FILE += [ 'mkdir -p %{buildroot}/' + EXE_INSTALL_DIR ]
            SPEC_FILE += [ 'install -m 0755 ' + EXE_INSTALL_DIR + '/xbtest %{buildroot}/' + EXE_INSTALL_DIR + '/xbtest' ]

            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%files' ]
            SPEC_FILE += [ '%defattr(-,root,root)' ]
            SPEC_FILE += [  '/' + OPT_INSTALL_DIR ]
            SPEC_FILE += [  '/' + EXE_INSTALL_DIR ]
            SPEC_FILE += [ '' ]
            SPEC_FILE += [ '%changelog' ]
            SPEC_FILE += [ '* ' + build_date_short + ' Xilinx Inc <support@xilinx.com> - ' + package[VERSION] + '-' + package[RELEASE] ]
            SPEC_FILE += [ '- ' + package[CHANGELOG] ]

            with open(spec_file_name, mode='w') as outfile:
                outfile.write('\n'.join(SPEC_FILE))

        else:

            deb_name = package[NAME] + '_' + package[VERSION] + '-' + package[RELEASE] + '_' + architecture
            dest_base  = os.path.abspath(os.path.join(output_dir, deb_name))
            os.makedirs(dest_base)
            os.chmod(dest_base, 493); # octal 0755
            debian_dir = os.path.abspath(os.path.join(dest_base, 'DEBIAN'))
            os.makedirs(debian_dir)

            # create control file
            control_file_name = os.path.abspath(os.path.join(debian_dir, 'control'))
            log_info('GEN_RPM-35', 'Creating xbtest control file: ' + control_file_name)

            # Replace \n with \n+SPACE to be compatible with deb package format
            package[DESCRIPTION] = package[DESCRIPTION].replace('\n', '\n ')

            CONTROL = []
            CONTROL += [ 'Package: ' + package[NAME] ]
            CONTROL += [ 'Architecture: ' + architecture ]
            CONTROL += [ 'Version: ' + package[VERSION] + '-' + package[RELEASE] ]
            CONTROL += [ 'Priority: optional' ]
            CONTROL += [ 'Description: ' + package[DESCRIPTION] ]
            CONTROL += [ 'Maintainer: Xilinx Inc' ]
            CONTROL += [ 'Section: devel' ]
            depends = ['libncurses5', 'libjson-glib-1.0-0']
            if opt.use_xrt:
                depends += ['xrt']
            CONTROL += [ 'Depends: ' + ', '.join(depends) ]
            CONTROL += [ 'Conflicts: xbtest-common' ]
            CONTROL += [ '' ]

            with open(control_file_name, mode='w') as outfile:
                outfile.write('\n'.join(CONTROL))

            # create preinst file
            preinst_file_name = os.path.abspath(os.path.join(debian_dir, 'preinst'))
            log_info('GEN_RPM-36', 'Creating xbtest preinst file: ' + preinst_file_name)

            PRE_INST = []
            PRE_INST += [ 'echo -e "' + package[PRE_INST_MSG] + '"' ]

            with open(preinst_file_name, mode='w') as outfile:
                outfile.write('\n'.join(PRE_INST))

            os.chmod(preinst_file_name, 509); # octal 775

            # create postinst file
            postinst_file_name = os.path.abspath(os.path.join(debian_dir, 'postinst'))
            log_info('GEN_RPM-37', 'Creating xbtest postinst file: ' + postinst_file_name)

            POST_INST = []
            POST_INST += [ 'echo -e "' + package[POST_INST_MSG] + '"' ]

            with open(postinst_file_name, mode='w') as outfile:
                outfile.write('\n'.join(POST_INST))

            os.chmod(postinst_file_name, 509); # octal 775

            # create changelog file
            changelog_dir = dest_base
            for subdir in ['usr', 'share', 'doc', package[NAME]]:
                changelog_dir = os.path.abspath(os.path.join(changelog_dir, subdir))
                os.makedirs(changelog_dir)

            changelog_file_name = os.path.abspath(os.path.join(changelog_dir, 'changelog.Debian'))
            log_info('GEN_RPM-38', 'Creating xbtest changelog file: ' + changelog_file_name)

            CHANGE_LOG = []
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ package[NAME] + ' (' + package[VERSION] + '-' + package[RELEASE] + ') xilinx; urgency=medium' ]
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ '  * ' + package[CHANGELOG] ]
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ '-- Xilinx Inc <support@xilinx.com> ' + build_date_short + ' 00:00:00 +0000' ]
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ '' ]
            CHANGE_LOG += [ '' ]

            with open(changelog_file_name, mode='w') as outfile:
                outfile.write('\n'.join(CHANGE_LOG))

            changelog_tar_name = os.path.abspath(os.path.join(changelog_dir, 'changelog.Debian.gz'))
            with tarfile.open(changelog_tar_name, "w:gz") as tar:
                tar.add(changelog_file_name)
            os.remove(changelog_file_name)

        # copy pkg files
        log_info('GEN_RPM-39', 'Copying packaged files')

        SRC_DEST_LIST = [
            {SRC: exe,                                                              DST: EXE_INSTALL_DIR},
            {SRC: os.path.abspath(os.path.join(BUILD_DIR, 'license', 'LICENSE')),   DST: os.path.join(OPT_INSTALL_DIR, 'license')}
        ]

        for src_dest in SRC_DEST_LIST:
            src = src_dest[SRC]
            dst = dest_base
            if src_dest[DST] != '':
                dst = os.path.abspath(os.path.join(dst, src_dest[DST]))
            log_debug('GEN_RPM-40', 'src: ' + src)
            log_debug('GEN_RPM-40', 'dst: ' + dst)
            copy_source_file('GEN_RPM-40', src, dst)

        # README.md
        README = []
        README += ['<!--']
        with open(os.path.abspath(os.path.join(BUILD_DIR, 'license', 'LICENSE'))) as f:
            README += f.read().splitlines()
        README += ['-->']

        README += ['']

        README += ['# ' + package[SUMMARY]]
        README += ['']
        desc_split = package[DESCRIPTION].split('\n')
        for desc in desc_split:
            README += [desc]
            README += ['']
        README += ['']
        README += ['## Package info']
        README += ['']
        README += ['The following table presents xclbin package information:']
        README += ['']
        README += ['| Version | Release number | Distribution ID | Distribution release | Architecture | Build date |']
        README += ['|---|---|---|---|---|---|']
        README += ['| ' + package[VERSION] + ' | ' + package[RELEASE] + ' | ' + distribution_id + ' | ' + distribution_release + ' | ' + architecture + ' | '  + build_date_short + ' |']
        README += ['']
        README += ['## Host application info']
        README += ['']
        README += ['The following table presents host application build information:']
        README += ['']
        README += ['| Build number | Build date |']
        README += ['|---|---|']
        README += ['| ' + str(opt.pkg_release) + ' | ' + build_date + ' |']
        README += ['']
        if opt.use_xrt:
            README += ['Generated to use with XRT.']
        else:
            README += ['Generated to use with AMI.']
        README += ['']

        readme_md = os.path.abspath(os.path.join(dest_base, OPT_INSTALL_DIR, 'README.md'))
        log_info('GEN_RPM-32', 'Creating ' + readme_md)
        with open(readme_md, 'w') as outfile:
            outfile.write('\n'.join(README))

        # generate pkg
        if distribution_id in DIST_RPM:
            step = 'generate RPM package'
            start_time = start_step('GEN_RPM-43', step)
            cmd = [
                'rpmbuild',
                '--verbose',
                '--define', '_topdir ' + output_dir,
                '-bb', spec_file_name
            ]
            log_file_name = os.path.abspath(os.path.join(output_dir, 'rpmbuild.log'))
            exec_step_cmd('GEN_RPM-44', step, cmd, log_file_name)
            end_step('GEN_RPM-45', start_time)

            pkg  = package[NAME] + '-' + package[VERSION] + '-' + package[RELEASE] + '.' + architecture
            src  = os.path.abspath(os.path.join(output_dir, 'RPMS', architecture, pkg + '.rpm'))
            dst  = os.path.abspath(os.path.join(output_dir,                       pkg + '_' + distribution_release + '.rpm'))
            dst2 = os.path.abspath(os.path.join(CWD,                             pkg + '_' + distribution_release + '.rpm'))
        else:
            step = 'generate DEB package'
            start_time = start_step('GEN_RPM-46', step)
            cmd = [
                'dpkg-deb',
                '--build', deb_name
            ]
            log_file_name = os.path.abspath(os.path.join(output_dir, 'dpkg-deb.log'))
            exec_step_cmd('GEN_RPM-47', step, cmd, log_file_name)
            end_step('GEN_RPM-48', start_time)

            pkg = package[NAME] + '_' + package[VERSION] + '-' + package[RELEASE] + '_' + architecture
            src  = os.path.abspath(os.path.join(output_dir, pkg + '.deb'))
            dst  = os.path.abspath(os.path.join(output_dir, pkg + '_' + distribution_release + '.deb'))
            dst2 = os.path.abspath(os.path.join(CWD,        pkg + '_' + distribution_release + '.deb'))

        log_info('GEN_RPM-49', 'Package generated successfully: ' + dst)
        log_info('GEN_RPM-50', 'Moving output package: ' + dst2)
        shutil.copy(src, dst)
        os.remove(src)
        shutil.copy(dst, dst2)

        tear_down('GEN_RPM-8', SCRIPT_FILE, script_start_time)

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
