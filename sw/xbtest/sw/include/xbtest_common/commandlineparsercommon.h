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

#ifndef _COMMANDLINEPARSERCOMMON_H
#define _COMMANDLINEPARSERCOMMON_H

#include "commandlineparserbase.h"
#include "xbtestcommonpackage.h"

namespace xbtest
{

class CommandLineParserCommon : public CommandLineParserBase
{

public:
    CommandLineParserCommon(
        Logging * log, std::atomic<bool> * abort, int argc, char ** argv, const std::vector<std::string> & amd_xbtest_libs,
        const std::map<std::string, HW_Lib_Config_t> & map_lib_config
    );
    ~CommandLineParserCommon();

    uint                            ParseCommandLine() override;
    Xbtest_Command_Line_Common_t    GetCommandLine();

private:
    std::vector<std::string>                m_amd_xbtest_libs;
    std::map<std::string, HW_Lib_Config_t>  m_map_lib_config;
    Xbtest_Command_Line_Common_t            m_command_line;

    void            ResetCommandLineTest();
    std::string     OptionCharToName    ( const char & option_char ) override;
    bool            MapNewTests();
    void            PrintConfig();
    void            PrintUsage() override;
    void            PrintVersion() override;
    void            PrintInvalidLib();
    uint            CountOpts           ( const std::vector<std::string> & args, const char & char_opt );

};

} // namespace

#endif /* _COMMANDLINEPARSERCOMMON_H */