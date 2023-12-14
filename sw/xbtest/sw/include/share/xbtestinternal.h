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

#ifndef XBTESTINTERNAL_H
#define XBTESTINTERNAL_H

namespace xbtest
{

#define XBTEST_VERSION_MAJOR    (int)(7)
#define XBTEST_VERSION_MINOR    (int)(0)
#define XBTEST_VERSION_PATCH    (int)(0)
#define XBTEST_VERSION_STR      std::to_string(XBTEST_VERSION_MAJOR) + "." + std::to_string(XBTEST_VERSION_MINOR) + "." + std::to_string(XBTEST_VERSION_PATCH)

// Preprocessor marco defined at compilation time
#ifndef BUILD_VERSION
#define BUILD_VERSION 0
#endif
#ifndef BUILD_DATE
#define BUILD_DATE std::string("")
#endif

#define XBTEST_BUILD_VERSION (int)(BUILD_VERSION)
#define preprop_xstr(s) preprop_str(s)
#define preprop_str(s) #s
#define XBTEST_BUILD_DATE std::string(preprop_xstr(BUILD_DATE))

// Functions to handle signals from EXE

void set_sig_abort( bool val );
bool get_sig_abort();

void set_sig_terminate( bool val );
bool get_sig_terminate();

// Functions for xbtest common to set xbtest SW options without argc/argv

void set_multitest_mode( bool val );
bool get_multitest_mode();

int run_core_inner ( int argc, char** argv );

} // namespace

#endif  /* XBTESTINTERNAL_H */