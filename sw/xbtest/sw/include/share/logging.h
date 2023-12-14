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

#ifndef _LOGGING_H
#define _LOGGING_H

#include <atomic>
#include <mutex>
#include <fstream>

#include "xbtestpackage.h"
#include "messages.h"
#include "xqueue.h"

namespace xbtest
{

class Logging
{

public:
    Logging( std::atomic<bool> * abort, const uint & level );
    ~Logging();

    void                SetUseConsole           ( const bool & use_console );
    void                LogMessage              ( const std::string & header,                                                                                     const Message_t & message, const std::vector<std::string> & arg_list );
    void                LogMessage              ( const std::string & header,                                                                                     const Message_t & message );
    void                LogMessage              ( const std::string & header, const std::string & header_2, const std::string & header_3, const LogLevel & Level, const Message_t & message, const std::vector<std::string> & arg_list );
    void                LogMessage              ( const std::string & header, const std::string & header_2, const std::string & header_3, const LogLevel & Level, const Message_t & message );
    void                LogMessage              ( const std::string & header, const std::string & header_2, const std::string & header_3,                         const Message_t & message, const std::vector<std::string> & arg_list );
    void                LogMessage              ( const std::string & header, const std::string & header_2, const std::string & header_3,                         const Message_t & message );
    void                LogMessage              ( const Message_t & message_in );
    Message_t           GetFirstError();
    bool                GetReportMsgCount       ( std::string & msg_cnt_msg );
    void                SetVerbosity            ( const LogLevel & verbosity );
    LogLevel            GetVerbosity();
    void                SetTimestampMode        ( const std::string & timestamp_mode );
    bool                GenerateLogDir          ( const std::string & log_dir, const bool & disable_log, const bool & force );
    bool                WriteMessagesJson       ( const std::string & file_name, const bool & force );
    bool                GetMessage              ( const std::string & id, Message_t & message );
    bool                GetMessagesParserRet();
    bool                PopMessage              ( Message_t & value );
    bool                PopMessageStat          ( Message_Stat_Queue_Value_t & value );

private:

    std::string                             m_log_header                = LOG_HEADER_LOGGING;
    bool                                    m_messages_parser_ret       = RET_SUCCESS;
    uint                                    m_level;
    std::atomic<bool> *                     m_abort;
    std::mutex                              m_logging_mtx;
    std::map<LogLevel, uint64_t>            m_msg_count;
    Message_t                               m_first_error;
    bool                                    m_first_error_exists        = false;
    uint64_t                                m_timestamp_curr            = 0;
    uint64_t                                m_timestamp_last            = 0;
    std::string                             m_timestamp_mode            = TIMESTAMP_MODE_NONE;
    bool                                    m_timestamp_mode_en         = false;
    bool                                    m_timestamp_mode_abs_n_diff = false;
    bool                                    m_timestamp_first           = false;
    LogLevel                                m_verbosity                 = LOG_STATUS; // Default log level
    std::vector<std::vector<std::string>>   m_full_log_rec;
    std::vector<std::string>                m_summary_log_rec;
    bool                                    m_logfile_wr_en             = false;
    bool                                    m_disable_log               = false;
    std::ofstream                           m_full_log_file;
    std::ofstream                           m_summary_log_file;
    bool                                    m_use_console               = true; // This will signal that message are not displayed by the logging class anymore
    XQueue<Message_t> *                     m_message_queue;
    XQueue<Message_Stat_Queue_Value_t> *    m_message_stat_queue;

    bool        RemoveDirectoryContent  ( const std::string & log_dir );
    bool        CreateDirectory         ( const std::string & log_dir, const bool & force );
    bool        CreateLogFile           ( const std::string & log_dir, const std::string & filename, const bool & force, std::ofstream & ofs );
    void        ParseMessages();
    std::string LogLevelToString        ( const LogLevel & level );

};

} // namespace

#endif /* _LOGGING_H */
