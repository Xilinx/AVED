// Copyright (C) 2022 Xilinx, Inc.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XBTEST_H
#define XBTEST_H

namespace xbtest
{

/**
* struct version_type -structure to hold the xbtest version information
* @major: Major software version
* @minor: Minor software version
* @patch: Patch number (if applicable)
* @build: Build number
*/

struct version_type {
	int major;
	int minor;
	int patch;
	int build;
};

/**
 * get_version() - Get version info
 * @version: Structure with version information
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */

int get_version ( struct version_type *version );

/**
 * abort() - Send abort signal (e.g. in case of SIGINT)
 */

void abort();

/**
 * terminate() - Send terminate signal (e.g. in case of SIGBUS)
 */

void terminate();

/**
 * xbtest_run() - Run
 * @argc: Argument count.
 * @argv: Argument vector.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */

int run ( int argc, char** argv );

/**
 * run_core() - Run core lower (for debug usage only)
 * @argc: Argument count.
 * @argv: Argument vector.
 *
 * Return: EXIT_SUCCESS or EXIT_FAILURE.
 */

int run_core ( int argc, char** argv );

} // namespace

#endif  /* XBTEST_H */