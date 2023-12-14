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

#ifndef _COMMANDLINEPARSERBASE_H
#define _COMMANDLINEPARSERBASE_H

#ifdef USE_AMI
#include <ami.h>
#endif

#include "xbtestpackage.h"
#include "logging.h"

namespace xbtest
{

class CommandLineParserBase
{

public:

    CommandLineParserBase( Logging * log, std::atomic<bool> * abort, int argc, char ** argv );
    ~CommandLineParserBase();

    std::atomic<bool> * m_abort;
    std::string         m_log_header = LOG_HEADER_COMMAND_LINE;
    Logging *           m_log = nullptr;
    int                 m_argc;
    char **             m_argv;

#ifdef USE_AMI
    struct ami_version m_ami_version;
#endif

    void                LogMessage                          ( const Message_t & message, const std::vector<std::string> & arg_list );
    void                LogMessage                          ( const Message_t & message );
    bool                ParseOptionArgumentStr              ( Xbtest_CL_option_Str_t &   cl_option, const char & test_option, const std::string & optarg_str );
    bool                ParseOptionArgumentStr              ( Xbtest_CL_option_Str_t &   cl_option, const char & test_option, const std::string & optarg_str, const bool & unique );
    bool                ParseOptionArgumentStr              ( std::string &              cl_option, const char & test_option, const std::string & optarg_str );
    bool                ParseOptionArgumentInt              ( Xbtest_CL_option_Int_t &   cl_option, const char & test_option, const std::string & optarg_str );
    bool                ParseOptionArgumentInt              ( Xbtest_CL_option_Int_t &   cl_option, const char & test_option, const std::string & optarg_str, const bool & unique );
    bool                ParseOptionArgumentInt              ( int &                      cl_option, const char & test_option, const std::string & optarg_str );
    bool                ParseOptionArgumentUint             ( Xbtest_CL_option_Uint_t &  cl_option, const char & test_option, const std::string & optarg_str );
    bool                ParseOptionArgumentUint             ( Xbtest_CL_option_Uint_t &  cl_option, const char & test_option, const std::string & optarg_str, const bool & unique );
    bool                ParseOptionArgumentUint             (                    uint &  cl_option, const char & test_option, const std::string & optarg_str );
    bool                CheckFileExists                     ( const std::string & name, const std::string & file );
    bool                SetVerbosity                        ( const int & verbosity );
    void                PrintCommandLineOptionNotSupported  ( const char & test_option );
    void                PrintMessage                        ( const std::string & msg_id );
    bool                CheckTargetP2P                      ( const std::string & source_bdf, const Xbtest_CL_option_Str_t & p2p_target_bdf, const Xbtest_CL_option_Str_t & p2p_nvme_path );

    virtual uint        ParseCommandLine() = 0;
    virtual std::string OptionCharToName( const char & option_char ) = 0;
    virtual void        PrintUsage() = 0;
    virtual void        PrintVersion() = 0;

#ifdef USE_AMI
    bool                GetAMIBuildVersion();
#endif

};

} // namespace

#endif /* _COMMANDLINEPARSERBASE_H */