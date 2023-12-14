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

#include <csignal>
#include <cstdlib>
#include <atomic>

#include "xbtest.h"
#include "xbtestinternal.h"

namespace xbtest
{

/**
* Get xbtest version info
*/

int get_version ( struct version_type *version )
{
	if (version != nullptr)
    {
		version->major = XBTEST_VERSION_MAJOR;
		version->minor = XBTEST_VERSION_MINOR;
		version->patch = XBTEST_VERSION_PATCH;
		version->build = XBTEST_BUILD_VERSION;

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

std::atomic<bool> gSigAbort;
std::atomic<bool> gSigTerminate;
std::atomic<bool> gMultiTestMode;
std::atomic<bool> gIsCommonChild;

/**
 * abort() - Send abort signal to xbtest (e.g. in case of SIGINT)
 */

void abort()
{
    set_sig_abort(true);
}

/**
 * terminate() - Send terminate signal to xbtest (e.g. in case of SIGBUS)
 */

void terminate()
{
    set_sig_terminate(true);
}

/**
 * Internal functions
*/

void set_sig_abort( bool val )
{
    gSigAbort = val;
}

bool get_sig_abort()
{
    return gSigAbort;
}

void set_sig_terminate( bool val )
{
    gSigTerminate = val;
}
bool get_sig_terminate()
{
    return gSigTerminate;
}

void set_multitest_mode( bool val )
{
    gMultiTestMode = val;
}
bool get_multitest_mode()
{
    return gMultiTestMode;
}

} // namespace
