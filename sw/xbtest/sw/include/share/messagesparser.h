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

#ifndef _MESSAGESPARSER_H
#define _MESSAGESPARSER_H

#include "xbtestpackage.h"
#include "logging.h"

namespace xbtest
{

class MessagesParser
{

public:
    MessagesParser( Logging * log, std::atomic<bool> * abort );
    ~MessagesParser();

    bool        ParseMessages();
    bool        ParseMessage        ( const Message_t & message );
    bool        WriteMessagesJson   ( const std::string & file_name, const bool & force );
    bool        GetMessage          ( const std::string & id, Message_t & message );

private:
    const std::string USED      = "used";
    const std::string RESERVED  = "reserved";
    const std::string OBSOLETE  = "obsolete";
    const std::vector<std::string> MESSAGE_STATES = {
        USED,
        RESERVED,
        OBSOLETE
    };

    std::string                         m_log_header = LOG_HEADER_MSG_PARSER;
    Logging *                           m_log = nullptr;
    std::atomic<bool> *                 m_abort;
    std::map<std::string, Message_t>    m_messages;

    void LogMessage( const Message_t & message, const std::vector<std::string> & arg_list );
    void LogMessage( const Message_t & message );

};

} // namespace

#endif /* _MESSAGESPARSER_H */
