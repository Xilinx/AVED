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

#include <curses.h>
#include <iostream>
#include <thread>

#include "consolemgmtbase.h"

namespace xbtest
{

ConsoleMgmtBase::ConsoleMgmtBase( Logging * log, Timer * timer )
{
    this->m_log = log;
    this->m_timer = timer;

    m_message_freq.clear();

    m_message_stat_queue_value_last.msg_count.clear();
    m_message_stat_queue_value_last.msg_count[LOG_WARN]       = 0;
    m_message_stat_queue_value_last.msg_count[LOG_CRIT_WARN]  = 0;
    m_message_stat_queue_value_last.msg_count[LOG_PASS]       = 0;
    m_message_stat_queue_value_last.msg_count[LOG_ERROR]      = 0;
    m_message_stat_queue_value_last.msg_count[LOG_FAILURE]    = 0;
}

ConsoleMgmtBase::~ConsoleMgmtBase() = default;


void ConsoleMgmtBase::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, message, arg_list);
}

void ConsoleMgmtBase::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, message);
}

void ConsoleMgmtBase::Start()
{
    LogMessage(MSG_DEBUG_SETUP, {"Start"});
    this->m_start = true;
}

void ConsoleMgmtBase::Stop()
{
    LogMessage(MSG_DEBUG_SETUP, {"Stop"});
    this->m_stop = true;
}

void ConsoleMgmtBase::DisableOutput()
{
    LogMessage(MSG_DEBUG_SETUP, {"Disable output"});
    this->m_enable_output = false;
}

void ConsoleMgmtBase::SetAllTcEnded()
{
    this->m_all_tc_ended = true;
}

void ConsoleMgmtBase::StoreDisplayLightMessages( const Message_t & message )
{
    if (message.display_dynamic_console)
    {
        auto msg_freq_key = message.header + message.header_2 + message.header_3 + message.id;
        int count = 0;
        auto it = this->m_message_freq.find(msg_freq_key);
        if (it != this->m_message_freq.end())
        {
            count = it->second;
        }

        if ((message.num_display_dynamic_console < 0) || (count < message.num_display_dynamic_console))
        {
            if (this->m_all_tc_ended)
            {
                this->m_messages_after_test.emplace_back(message.log_msg);
            }
            else if (!(this->m_table_exists))
            {
                this->m_messages_before_test.emplace_back(message.log_msg);
            }
            else
            {
                this->m_messages_during_test.emplace_back(message.log_msg);
            }
        }
        auto result = this->m_message_freq.emplace(msg_freq_key, 1);
        if (!result.second) // Element already exists
        {
            (result.first)->second++;
        }
    }
}

void ConsoleMgmtBase::RunRefreshDataThread()
{
	while (!(this->m_stop))
	{
        RefreshData();
        // Wait 1 second before next refresh
        this->m_timer->WaitFor1sTick();
        this->m_elapsed = this->m_timer->GetSecCnt();
	}
}

bool ConsoleMgmtBase::ConsoleTask()
{
    // Start-up phase (not dynamic)
    Message_t message_tmp;
	while (!(this->m_start) && !(this->m_stop))
	{
        while (this->m_log->PopMessage(message_tmp)) // Check if new messages
        {
            // Need to store all messages as we don't know yet if Light console will be used
            StoreDisplayLightMessages(message_tmp);
            // Display as if we were with -F

            if (this->m_enable_output)
            {
                if ((message_tmp.display_classic_console) || (this->m_log->GetVerbosity() < LOG_STATUS))
                {
                    std::cout << message_tmp.log_msg << std::endl << std::flush;
                }
            }
        }
	}
    if (this->m_stop)
    {
        return RET_SUCCESS;
    }

    // Get the console configuration
    // The isatty() function tests whether fd is an open file descriptor referring to a terminal.
    // returns 1 if fd is an open file descriptor referring to a terminal; otherwise 0 is returned
    if (isatty(fileno(stdout)) == 0)
    {
        this->m_console_batch = true;
    }

    // FILE * f = nullptr;
    // SCREEN * screen = nullptr;
    if (this->m_enable_output)
    {
        if (!(this->m_detailed_format))
        {
            // Setup console for dynamic display
            if (!(this->m_console_batch))
            {
                std::cout << "Starting dynamic display mode..." << std::endl << std::flush;
                /*
                // We could have used this instead of isatty(fileno(stdout)) and have the latest display in the log in case of stdout redirection
                f = fopen("/dev/tty", "r+");
                screen = newterm(nullptr, f, f);
                set_term(screen);
                */

                initscr(); // Start curses mode
                cbreak();  // Line buffering disabled, Pass on every thing to me
                noecho();  // switch off echoing of characters typed by the user to the terminal.
                // clearok(stdscr, 1); // when refresh() is called, erase() the screen before redrawing
           }
            else
            {
                std::cout << "Starting dynamic display mode in batch mode..." << std::endl << std::flush;
            }
        }
    }

    std::thread refresh_data_thread;
    if (this->m_detailed_format)
    {
        refresh_data_thread = std::thread(&ConsoleMgmtBase::RunRefreshDataThread, this); // Refresh data in separate thread in -F mode
    }
	while (!(this->m_stop))
	{
        this->m_runnning = true;

        // Write console with new content
        if (this->m_detailed_format)
        {
            while (this->m_log->PopMessage(message_tmp)) // Check if new messages
            {
                // In detailed mode, no need to save messages
                // No filter when the verbosity is debug
                if (this->m_enable_output)
                {
                    if ((message_tmp.display_classic_console) || (this->m_log->GetVerbosity() < LOG_STATUS))
                    {
                        std::cout << message_tmp.log_msg << std::endl << std::flush;
                    }
                }
            }
        }
        else
        {
            RefreshData(); // Refresh data before refresh console in dynamic console mode

            // Init console
            if (this->m_enable_output)
            {
                if (this->m_console_batch)
                {
                    std::cout << GetBatchSeparator() << std::endl << std::flush;
                }
                else
                {
                    // erase(); // Erase content of the window. Note using this we can still see stderr, but we don't know where it is and we could write over it (and stderr msg disappear after enwin()...)
                    clear(); // Clear content of the window
                    move(0,0); // move cursor to top left of the window
                }
                // Display new table
                for (const auto & line : UpdateContent(false))
                {
                    if (this->m_console_batch)
                    {
                        std::cout << line << std::endl << std::flush;
                    }
                    else
                    {
                        printw("%s\n", line.c_str());
                    }
                }
                // Refresh with new content
                if (!(this->m_console_batch))
                {
                    refresh(); // Update screen
                }
            }
            // Wait 1 second for next display
            this->m_timer->WaitFor1sTick();
            this->m_elapsed = this->m_timer->GetSecCnt();
        }
	}
    if (this->m_detailed_format)
    {
        refresh_data_thread.join();
    }
    this->m_runnning = false;

    if (this->m_enable_output)
    {
        if (!(this->m_detailed_format))
        {
            // end of dynamic display
            if (!(this->m_console_batch))
            {
                endwin(); // End curses mode
                // delscreen(screen);
                // fclose(f);
            }
            // Display last table so console contains final log after xbtest terminates
            // Init
            if (this->m_console_batch)
            {
                std::cout << GetLastBatchSeparator() << std::endl << std::flush;
            }
            else
            {
                std::cout << std::endl << std::endl << "Repeating last content of dynamic display mode:" << std::endl << std::flush;
            }
            // Display
            for (const auto & line : UpdateContent(true))
            {
                std::cout << line << std::endl << std::flush;
            }
        }
    }
    return RET_SUCCESS;
}

std::string ConsoleMgmtBase::GetBatchSeparator()
{
    return "------- IT: " + std::to_string(this->m_elapsed) + " -------";
}

std::string ConsoleMgmtBase::GetLastBatchSeparator()
{
    return "------- Final IT -------";
}

} // namespace
