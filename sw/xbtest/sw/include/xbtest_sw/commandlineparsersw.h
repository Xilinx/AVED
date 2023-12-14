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

#ifndef _COMMANDLINEPARSERSW_H
#define _COMMANDLINEPARSERSW_H

#include <mutex>

#include "commandlineparserbase.h"
#include "xbtestswpackage.h"

namespace xbtest
{

class CommandLineParserSw : public CommandLineParserBase
{

public:
    CommandLineParserSw( Logging * log, std::atomic<bool> * abort, int argc, char ** argv );
    ~CommandLineParserSw();

    uint                        ParseCommandLine() override;
    Xbtest_Command_Line_Sw_t    GetCommandLine();

    static std::mutex           m_mutex_getopt;

private:
    const uint GEN_002_PAD = 15;

    Xbtest_Command_Line_Sw_t    m_command_line;

    std::string     OptionCharToName( const char & option_char ) override;
    void            PrintUsage() override;
    void            PrintVersion() override;

};

} // namespace

#endif /* _COMMANDLINEPARSERSW_H */