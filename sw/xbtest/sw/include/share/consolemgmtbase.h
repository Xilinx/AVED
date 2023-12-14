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

#ifndef _CONSOLEMGMTBASE_H
#define _CONSOLEMGMTBASE_H

#include "xbtestpackage.h"
#include "timer.h"
#include "logging.h"

namespace xbtest
{

class ConsoleMgmtBase
{

public:
    const uint MAX_MSG_DURING_TEST = 10;

    bool                        m_stop                  = false;
    bool                        m_start                 = false;
    bool                        m_enable_output         = true;
    bool                        m_runnning              = false;
    bool                        m_all_tc_ended          = false;
    Timer *                     m_timer                 = nullptr;
    Logging *                   m_log = nullptr;
    std::string                 m_log_header            = LOG_HEADER_CONSOLE_MGMT;
    bool                        m_console_batch         = false;
    bool                        m_detailed_format       = false;
    uint64_t                    m_elapsed               = 0;
    std::map<std::string, int>  m_message_freq;
    bool                        m_table_exists          = false;
    bool                        m_data_exists           = false;
    std::vector<std::string>    m_messages_before_test;
    std::vector<std::string>    m_messages_during_test;
    std::vector<std::string>    m_messages_after_test;
    Message_Stat_Queue_Value_t  m_message_stat_queue_value_last;

    ConsoleMgmtBase( Logging * log, Timer * timer );
    ~ConsoleMgmtBase();

    void        LogMessage                  ( const Message_t & message, const std::vector<std::string> & arg_list );
    void        LogMessage                  ( const Message_t & message );
    void        Start();
    void        Stop();
    void        DisableOutput();
    void        SetAllTcEnded();
    void        StoreDisplayLightMessages   ( const Message_t & message );
    void        RunRefreshDataThread();
    bool        ConsoleTask();
    std::string GetBatchSeparator();
    std::string GetLastBatchSeparator();

    virtual bool                        SetUp() = 0;
    virtual void                        RefreshData() = 0;
    virtual std::vector<std::string>    UpdateContent( const bool & last ) = 0;
    virtual std::vector<std::string>    UpdateConsoleTable() = 0;
};

} // namespace

#endif /* _CONSOLEMGMTBASE_H */
