
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

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include "logging.h"
#include "messagesparser.h"

namespace xbtest
{

static MessagesParser * s_messages_parser  = nullptr;

Logging::Logging( std::atomic<bool> * abort, const uint & level )
{
    this->m_abort = abort;
    this->m_level = level;

    this->m_msg_count[LOG_WARN]       = 0;
    this->m_msg_count[LOG_CRIT_WARN]  = 0;
    this->m_msg_count[LOG_PASS]       = 0;
    this->m_msg_count[LOG_ERROR]      = 0;
    this->m_msg_count[LOG_FAILURE]    = 0;

    this->m_full_log_rec.clear();
    this->m_summary_log_rec.clear();

    this->m_message_queue = new XQueue<Message_t>;
    this->m_message_stat_queue = new XQueue<Message_Stat_Queue_Value_t>;

    ParseMessages();
}

Logging::~Logging()
{
    delete this->m_message_queue;
    delete this->m_message_stat_queue;
}

void Logging::SetUseConsole( const bool & use_console )
{
    this->m_use_console = use_console;
}

void Logging::LogMessage ( const std::string & header, const Message_t & message, const std::vector<std::string> & arg_list )
{
    auto message_2      = message;
    message_2.header    = header;
    message_2.arg_list  = arg_list;
    LogMessage(message_2);
}

void Logging::LogMessage ( const std::string & header, const Message_t & message )
{
    auto message_2      = message;
    message_2.header    = header;
    LogMessage(message_2);
}

void Logging::LogMessage ( const std::string & header, const std::string & header_2, const std::string & header_3, const LogLevel & Level, const Message_t & message, const std::vector<std::string> & arg_list )
{
    auto message_2      = message;
    message_2.log_level = Level;
    message_2.header    = header;
    message_2.header_2  = header_2;
    message_2.header_3  = header_3;
    message_2.arg_list  = arg_list;
    LogMessage(message_2);
}

void Logging::LogMessage ( const std::string & header, const std::string & header_2, const std::string & header_3, const LogLevel & Level, const Message_t & message )
{
    auto message_2      = message;
    message_2.log_level = Level;
    message_2.header    = header;
    message_2.header_2  = header_2;
    message_2.header_3  = header_3;
    LogMessage(message_2);
}

void Logging::LogMessage ( const std::string & header, const std::string & header_2, const std::string & header_3, const Message_t & message, const std::vector<std::string> & arg_list )
{
    auto message_2 = message;
    message_2.header    = header;
    message_2.header_2  = header_2;
    message_2.header_3  = header_3;
    message_2.arg_list  = arg_list;
    LogMessage(message_2);
}

void Logging::LogMessage ( const std::string & header, const std::string & header_2, const std::string & header_3, const Message_t & message )
{
    auto message_2      = message;
    message_2.header    = header;
    message_2.header_2  = header_2;
    message_2.header_3  = header_3;
    LogMessage(message_2);
}

void Logging::LogMessage( const Message_t & message_in )
{
    if (message_in.log_level < this->m_verbosity)
    {
        return;
    }
    auto message = message_in;
    message.log_msg = "";

    if (this->m_timestamp_mode_en)
    {
        this->m_timestamp_curr = GetTimestamp();
        message.timestamp_abs  = this->m_timestamp_curr;
        message.timestamp_diff = this->m_timestamp_curr - this->m_timestamp_last;
        if (!(this->m_timestamp_first))
        {
            message.timestamp_diff = 0;
        }

        message.timestamp_str = "[";
        if (this->m_timestamp_mode_abs_n_diff)
        {
            message.timestamp_str = Float_to_String<double>((double)(message.timestamp_abs) / (double)(1000000.0), 6);
        }
        else
        {
            if (!(this->m_timestamp_first))
            {
                message.timestamp_str += " ";
            }
            else
            {
                message.timestamp_str += "+";
            }
            message.timestamp_str += Float_to_String<double>((double)(message.timestamp_diff) / (double)(1000000.0), 6);
        }
        message.timestamp_str += "] ";

        this->m_timestamp_last = this->m_timestamp_curr;
        this->m_timestamp_first = true;
        message.log_msg += message.timestamp_str;
    }

    message.severity = LogLevelToSeverity(message.log_level);
    message.log_msg += LogLevelToString(message.log_level) + LOG_COL_DELIMITER;
    message.log_msg += message.id + LOG_COL_DELIMITER;

    if (!message.header.empty())
    {
        message.log_msg += message.header + LOG_COL_DELIMITER;
    }
    if (!message.header_2.empty())
    {
        message.log_msg += message.header_2 + LOG_COL_DELIMITER;
    }
    if (!message.header_3.empty())
    {
        message.log_msg += message.header_3 + LOG_COL_DELIMITER;
    }

    // Message content (editable or constant)
    message.content = "";
    const std::string REP = "%s";
    if (message.msg_edit.empty()) // msg_edit not defined, use constant message "msg" and add all arg as <others>
    {
        message.content = message.msg;
        if (!message.arg_list.empty())
        {
            message.content += "; <others> " + StrVectToStr(message.arg_list, ", ");
        }
    }
    else // msg_edit is defined, use constant message "msg_edit" and add all arg. Too much arg => <others>, not enough arg => <NULL>
    {
        message.content = message.msg_edit;
        size_t start_pos;
        auto end_param = false;
        for (uint ii = 0; ii < message.arg_list.size(); ii++ )
        {
            start_pos = message.content.find(REP);
            if (start_pos != std::string::npos)
            {
                message.content.replace(start_pos, REP.length(), message.arg_list[ii]); // Replace "%s" with arg
            }
            else
            {
                if (!end_param)
                {
                    message.content += "; <others> ";
                }
                message.content += message.arg_list[ii];
                if (ii < message.arg_list.size() - 1)
                {
                    message.content += ", ";
                }
                end_param = true;
            }
        }
        // When there are less arg than "%s"
        do
        {
            start_pos = message.content.find(REP);
            if (start_pos != std::string::npos)
            {
                message.content.replace(start_pos, REP.length(), "<NULL>");
            }
        } while (start_pos != std::string::npos);
    }

    message.log_msg += message.content;

    this->m_logging_mtx.lock(); // protect access of log files at attributes

    // Count number of message per severity
    if (this->m_msg_count.find(message.log_level) != this->m_msg_count.end())
    {
        this->m_msg_count[message.log_level]++;
    }

    if (this->m_use_console) // Console mgmt is not used, display messages here
    {
        // No filter when the verbosity is debug
        if ((message.display_classic_console) || (this->m_verbosity < LOG_STATUS))
        {
            std::cout << message.log_msg << std::endl << std::flush;
        }
    }
    else // Console mgmt is not used, push message into the console mgmt message queue
    {
        // Send to queue
        this->m_message_queue->Push(message);
        // Send to queue
        Message_Stat_Queue_Value_t message_stat_queue_value;
        message_stat_queue_value.msg_count = this->m_msg_count;
        this->m_message_stat_queue->Push(message_stat_queue_value);
    }
    // record first error
    if (!(this->m_first_error_exists))
    {
        if ((message.log_level == LOG_FAILURE) || (message.log_level == LOG_ERROR))
        {
            this->m_first_error_exists = true;
            auto msg_found = GetMessage(message.id, this->m_first_error);
            if (!msg_found)
            {
                this->m_first_error.id            = message.id;
                this->m_first_error.severity      = LogLevelToSeverity(message.log_level);
                this->m_first_error.detail        = "Not available";
                this->m_first_error.resolution    = "Not available";
                this->m_first_error.log_level     = message.log_level;
            }
            this->m_first_error.msg = message.log_msg; // full log message with headers, id, severity
        }
    }
    // all message in full log
    if (this->m_logfile_wr_en)
    {
        this->m_full_log_file << message.log_msg << "\n";
        this->m_full_log_file.flush();
    }
    else if (!(this->m_disable_log))
    {
        std::vector<std::string> log_msg_vec;
        log_msg_vec.emplace_back(message.log_msg);
        this->m_full_log_rec.emplace_back(log_msg_vec);
    }
    // Only display_classic_console (i.e -F) in summary.log
    if (message.display_classic_console)
    {
        if (this->m_logfile_wr_en)
        {
            this->m_summary_log_file << message.log_msg << "\n";
            this->m_summary_log_file.flush();
        }
        else if (!(this->m_disable_log))
        {
            this->m_summary_log_rec.emplace_back(message.log_msg);
        }
    }

    this->m_logging_mtx.unlock();
}

Message_t Logging::GetFirstError()
{
    return this->m_first_error;
}

bool Logging::GetReportMsgCount( std::string & msg_cnt_msg )
{
    msg_cnt_msg = "";
    msg_cnt_msg += std::to_string(this->m_msg_count[LOG_WARN])      + " Warnings, ";
    msg_cnt_msg += std::to_string(this->m_msg_count[LOG_CRIT_WARN]) + " Critical Warnings, ";
    msg_cnt_msg += std::to_string(this->m_msg_count[LOG_PASS])      + " Passes, ";
    msg_cnt_msg += std::to_string(this->m_msg_count[LOG_ERROR])     + " Errors, ";
    msg_cnt_msg += std::to_string(this->m_msg_count[LOG_FAILURE])   + " Failures encountered";

    if (this->m_msg_count[LOG_ERROR] + this->m_msg_count[LOG_FAILURE] > 0)
    {
        return RET_FAILURE;
    }
    if (this->m_msg_count[LOG_PASS] == 0)
    {
        LogMessage(LOG_HEADER_GENERAL, MSG_GEN_026);
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void Logging::SetVerbosity( const LogLevel & verbosity )
{
    // only authorise overwrite to display debug messages
    if (verbosity < LOG_STATUS)
    {
        this->m_verbosity = verbosity;
    }
}

LogLevel Logging::GetVerbosity()
{
    return this->m_verbosity;
}

void Logging::SetTimestampMode( const std::string & timestamp_mode )
{
    this->m_timestamp_mode = timestamp_mode;
    if (!StrMatchNoCase(timestamp_mode, TIMESTAMP_MODE_NONE))
    {
        this->m_timestamp_mode_en = true;
        if (StrMatchNoCase(timestamp_mode, TIMESTAMP_MODE_ABSOLUTE))
        {
            this->m_timestamp_mode_abs_n_diff = true;
        }
    }
}

bool Logging::GenerateLogDir( const std::string & log_dir, const bool & disable_log, const bool & force )
{
    this->m_disable_log = disable_log;
    if (this->m_disable_log)
    {
        return RET_SUCCESS;
    }
    if (CreateDirectory(log_dir, force) == RET_FAILURE) // do not create if directory already exists
    {
        return RET_FAILURE;
    }
    // create the detailed log file
    std::string summary_log = "summary.log";
    std::string full_log    = "xbtest.log";
    if (this->m_level == XBTEST_COMMON)
    {
        summary_log = "common_summary.log";
        full_log    = "common.log";
    }
    if (CreateLogFile(log_dir, summary_log, force, this->m_summary_log_file) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (CreateLogFile(log_dir, full_log, force, this->m_full_log_file) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Create the folder for result CSV files used for the RT display tool
    auto RT_log_dir  = log_dir + "/RT";
    if (CreateDirectory(RT_log_dir, force) == RET_FAILURE) // do not create if directory already exists
    {
        return RET_FAILURE;
    }

    // Write previously saved log messages
    this->m_logging_mtx.lock(); // Stop logging while previous message are saved
    // summary
    for (const auto & log_msg: this->m_summary_log_rec)
    {
        this->m_summary_log_file << log_msg << "\n";
    }
    this->m_summary_log_file.flush();
    this->m_summary_log_rec.clear();
    // full
    for (const auto & log_msg: this->m_full_log_rec)
    {
        this->m_full_log_file << log_msg.at(0) << "\n";
    }
    this->m_full_log_file.flush();
    this->m_full_log_rec.clear();

    this->m_logfile_wr_en = true; // This authorize writing to log file after it is created
    this->m_logging_mtx.unlock(); // Restart logging

    return RET_SUCCESS;
}

bool Logging::WriteMessagesJson( const std::string & file_name, const bool & force )
{
    return s_messages_parser->WriteMessagesJson(file_name, force);
}

bool Logging::GetMessage( const std::string & id, Message_t & message )
{
    return s_messages_parser->GetMessage(id, message);
}

bool Logging::GetMessagesParserRet()
{
    return this->m_messages_parser_ret;
}

bool Logging::PopMessage( Message_t & value )
{
    return this->m_message_queue->Pop(value);
}

bool Logging::PopMessageStat( Message_Stat_Queue_Value_t & value )
{
    return this->m_message_stat_queue->Pop(value);
}

bool Logging::RemoveDirectoryContent( const std::string & dir )
{
    // Remove the directory recuresively
    auto contents = ListDirectoryContent(dir+"/*");
    for (const auto & content : contents)
    {
        std::string content_path;
        content_path += dir;
        content_path += "/";
        content_path += content;
        // Check if full path of provided directory exists
        struct stat stat_info;
        if (stat(content_path.c_str(), &stat_info) == 0) // file/directory exists
        {
            if (S_ISDIR(stat_info.st_mode))
            {
                if (RemoveDirectoryContent(content_path) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
                if (rmdir(content_path.c_str()) != 0)
                {
                    LogMessage(this->m_log_header, MSG_ITF_186, {content_path, strerror(errno)}); // warning
                    return RET_FAILURE;
                }
            }
            else
            {
                if (remove(content_path.c_str()) != 0)
                {
                    LogMessage(this->m_log_header, MSG_ITF_186, {content_path, strerror(errno)}); // warning
                    return RET_FAILURE;
                }
            }
        }
    }
    return RET_SUCCESS;
}

bool Logging::CreateDirectory( const std::string & log_dir, const bool & force )
{
    struct stat stat_info;

    if (this->m_disable_log)
    {
        return RET_SUCCESS;
    }

    LogMessage(this->m_log_header, MSG_ITF_078, {log_dir});

    // Check if full path of provided directory exists
    if (stat(log_dir.c_str(), &stat_info) == 0) // file/directory exists
    {
        if (S_ISDIR(stat_info.st_mode))
        {
            if (force)
            {
                LogMessage(this->m_log_header, MSG_ITF_146, {log_dir}); // warning
                if (RemoveDirectoryContent(log_dir) == RET_FAILURE)
                {
                    return RET_FAILURE;
                }
            }
            else
            {
                LogMessage(this->m_log_header, MSG_ITF_147, {log_dir}); // directory already exists
                return RET_FAILURE;
            }
        }
        else
        {
            LogMessage(this->m_log_header, MSG_ITF_148, {log_dir}); // path already exists and is not a directory
            return RET_FAILURE;
        }
    }

    // Create directory recursively
    std::string tmp;
    for (const auto & dir : split(log_dir, '/'))
    {
        tmp += dir + "/";
        if (stat(tmp.c_str(), &stat_info) == 0) // directory exists
        {
            continue; // Do not try to create directory if it already exists
        }
        if (mkdir(tmp.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
        {
            if (errno != EEXIST) // ignore exist error
            {
                LogMessage(this->m_log_header, MSG_ITF_079, {log_dir, strerror(errno)});
                return RET_FAILURE;
            }
        }
    }
    return RET_SUCCESS;
}

bool Logging::CreateLogFile( const std::string & log_dir, const std::string & filename, const bool & force, std::ofstream & ofs )
{
    auto fullname = log_dir + "/" + filename;
    LogMessage(this->m_log_header, MSG_CMN_020, {fullname});
    if (FileExists(fullname))
    {
        if (force)
        {
            LogMessage(this->m_log_header, MSG_CMN_019, {fullname}); // warning
        }
        else
        {
            LogMessage(this->m_log_header, MSG_CMN_053, {fullname});
            return RET_FAILURE;
        }
    }

    if (!OpenFile(fullname, ofs))
    {
        LogMessage(this->m_log_header, MSG_ITF_083, {fullname});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

void Logging::ParseMessages()
{
    if (s_messages_parser == nullptr)
    {
        s_messages_parser = new MessagesParser(this, this->m_abort);
        this->m_messages_parser_ret = s_messages_parser->ParseMessages();
    }
}

std::string Logging::LogLevelToString( const LogLevel & level )
{
    switch (level)
    {
        case LOG_FAILURE:           return "FAILURE  "; break;
        case LOG_ERROR:             return "ERROR    "; break;
        case LOG_PASS:              return "PASS     "; break;
        case LOG_CRIT_WARN:         return "CRIT WARN"; break;
        case LOG_WARN:              return "WARNING  "; break;
        case LOG_INFO:              return "INFO     "; break;
        case LOG_STATUS:            return "STATUS   "; break;
        default:                    return "DEBUG    "; break; // LOG_DEBUG*
    }
}

} // namespace
