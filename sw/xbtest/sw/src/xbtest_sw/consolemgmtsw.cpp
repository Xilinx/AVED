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

#include "consolemgmtsw.h"

namespace xbtest
{

ConsoleMgmtSw::ConsoleMgmtSw( Logging * log, Timer * timer, XbtestSwConfig * xbtest_sw_config ) : ConsoleMgmtBase::ConsoleMgmtBase(log, timer)
{
    this->m_xbtest_sw_config = xbtest_sw_config;
}

ConsoleMgmtSw::~ConsoleMgmtSw() = default;

bool ConsoleMgmtSw::SetUp()
{
    LogMessage(MSG_DEBUG_SETUP, {"Set-up"});
    if (this->m_xbtest_sw_config->GetCommandLineConsoleDetailedFormat())
    {
        this->m_detailed_format = true;
        this->m_messages_before_test.clear();
    }
    return RET_SUCCESS;
}

void ConsoleMgmtSw::RefreshData()
{
    if (this->m_sensor_queues_exists)
    {
        while (this->m_sensor_queues->Pop(this->m_sensor_queue_value_last))
        {
        }
    }
    for (const auto & testcase_stat : this->m_testcase_queues)
    {
        for (const auto & thread_stat : testcase_stat.second)
        {
            // flush queue and get the last data available
            while (this->m_testcase_queues[testcase_stat.first][thread_stat.first]->Pop(this->m_testcase_queue_value_last[testcase_stat.first][thread_stat.first]))
            {
            }
        }
    }
    this->m_data_exists = true;
}

void ConsoleMgmtSw::AllocateTestcaseQueue( const std::string & testcase, const std::string & thread, const Testcase_Queue_Value_t & value )
{
    auto * queue = new XQueue<Testcase_Queue_Value_t>;
    this->m_testcase_queues[testcase][thread] = queue;
    PushTestcaseQueue(testcase, thread, value);
    this->m_testcase_queue_value_last[testcase][thread] = value;
}

void ConsoleMgmtSw::PushTestcaseQueue( const std::string & testcase, const std::string & thread, const Testcase_Queue_Value_t & value )
{
    this->m_testcase_queues[testcase][thread]->Push(value);
}

void ConsoleMgmtSw::AllocateSensorQueue( const Sensor_Queue_Value_t & value )
{
    auto * queue = new XQueue<Sensor_Queue_Value_t>;
    this->m_sensor_queues = queue;
    PushSensorQueue(value);
    this->m_sensor_queues_exists = true;
}

void ConsoleMgmtSw::PushSensorQueue( const Sensor_Queue_Value_t & value )
{
    this->m_sensor_queues->Push(value);
}

std::vector<std::string> ConsoleMgmtSw::UpdateContent( const bool & last )
{
    std::vector<std::string> console_lines;

    // Check if new messages
    Message_t message;
    while (this->m_log->PopMessage(message))
    {
        StoreDisplayLightMessages(message);
    }

    // Add the message that appeared before first TC started (dma or verify)
    console_lines.insert(console_lines.end(), this->m_messages_before_test.begin(), this->m_messages_before_test.end());
    console_lines.emplace_back(""); // space

    // Display table
    if (!(this->m_testcase_queues.empty()))
    {
        auto table = UpdateConsoleTable();
        console_lines.insert(console_lines.end(), table.begin(), table.end());
        console_lines.emplace_back(""); // space
        this->m_table_exists = true;
    }

    // Live sensor info
    //      Live: Power: 31 W; Temperature: 50 C; Qty of measurements: 7
    if (this->m_sensor_queues_exists)
    {
        std::string live_msg = "Card status: ";
        live_msg += "Power: ";
        if (this->m_sensor_queue_value_last.power_found)
        {
            live_msg += Float_to_String<double>(this->m_sensor_queue_value_last.power, 1) + " W";
        }
        else
        {
            live_msg += NOT_APPLICABLE;
        }
        live_msg += "; ";
        live_msg += "Temperature: ";
        if (this->m_sensor_queue_value_last.temperature_found)
        {
            live_msg += Float_to_String<double>(this->m_sensor_queue_value_last.temperature, 0) + " C";
        }
        else
        {
            live_msg += NOT_APPLICABLE;
        }
        live_msg += "; ";
        live_msg += "Qty of measurements: " + std::to_string(this->m_sensor_queue_value_last.dump_cnt);
        console_lines.emplace_back(live_msg);
        console_lines.emplace_back(""); // space
    }

    // Message stat
    Message_Stat_Queue_Value_t message_stat_queue_value;
    while (this->m_log->PopMessageStat(message_stat_queue_value))
    {
        this->m_message_stat_queue_value_last = message_stat_queue_value;
    }
    std::string msg_stat_line;
    msg_stat_line += "Messages stats: ";
    msg_stat_line += std::to_string(this->m_message_stat_queue_value_last.msg_count[LOG_WARN])      + " Warnings, ";
    msg_stat_line += std::to_string(this->m_message_stat_queue_value_last.msg_count[LOG_CRIT_WARN]) + " Critical Warnings, ";
    msg_stat_line += std::to_string(this->m_message_stat_queue_value_last.msg_count[LOG_PASS])      + " Passes, ";
    msg_stat_line += std::to_string(this->m_message_stat_queue_value_last.msg_count[LOG_ERROR])     + " Errors, ";
    msg_stat_line += std::to_string(this->m_message_stat_queue_value_last.msg_count[LOG_FAILURE])   + " Failures encountered";
    console_lines.emplace_back(msg_stat_line);
    console_lines.emplace_back(""); // space

    // Add the message that appeared after after first TC started (dma or verify) and before the last testcase end
    if (!(this->m_messages_during_test.empty()))
    {
        console_lines.emplace_back("Message history (limited to the " + std::to_string(MAX_MSG_DURING_TEST) + " last ones)");
        auto limit = MAX_MSG_DURING_TEST;
        if (limit > this->m_messages_during_test.size())
        {
            limit = this->m_messages_during_test.size();
        }
        console_lines.insert(console_lines.end(), this->m_messages_during_test.end() - limit, this->m_messages_during_test.end());
        console_lines.emplace_back(""); // space
    }

    // Total elapsed: 21 seconds
    // or
    // Loading... Total elapsed: 1 seconds
    std::string elapsed_msg;
    if (this->m_messages_before_test.empty())
    {
        elapsed_msg += "Loading... ";
    }
    elapsed_msg += "Total elapsed: " + std::to_string(this->m_elapsed) + " s";
    console_lines.emplace_back(elapsed_msg);

    // Add "  " for last content
    if (last)
    {
        std::vector<std::string> console_lines_last;
        console_lines_last.reserve(console_lines.size());
        for (const auto & line : console_lines)
        {
            console_lines_last.emplace_back("  " + line);
        }
        console_lines = console_lines_last;
    }
    if (!(this->m_messages_after_test.empty()))
    {
        console_lines.emplace_back("");
        console_lines.emplace_back("");
    }
    // Add the message that appeared after the last testcase end
    console_lines.insert(console_lines.end(), this->m_messages_after_test.begin(), this->m_messages_after_test.end());
    return console_lines;
}

std::vector<std::string> ConsoleMgmtSw::UpdateConsoleTable()
{
    std::vector<std::string> table;
    std::vector<std::string> col_vect;

// +------------------+ +-----------------------------------------------------------+ +---------------------------------------------------------------------+
// |                  | |                          STATUS                           | |                            ONGOING TEST                             |
// |     TESTCASE     | |-----------------------------------------------------------| |---------------------------------------------------------------------|
// |                  | | Pending | Completed | Passed | Failed | Errors | Warnings | | Remaining time (s) | Parameters                                     |
// |------------------| |-----------------------------------------------------------| |---------------------------------------------------------------------|
// | DMA              | |       0 |         5 |      3 |      2 |     10 |        1 | |                n/a | n/a                                            |
// | Power            | |       3 |         2 |      2 |      0 |      0 |        0 | |                 67 | [150,70]                                       |
// | HBM              | |       2 |         1 |      0 |      1 |      5 |        0 | |                  5 | [20, "Only_Wr"]                                |
// | DDR[1]           | |       1 |         0 |      0 |      0 |      0 |        2 | |                  5 | [200, "Alternate_Wr_Rd"]                       |
// | DDR[0]           | |       1 |         0 |      0 |      0 |      0 |        0 | |                  5 | [200, "Alternate_Wr_Rd"]                       |
// | GT_MAC[1]        | |       5 |         0 |      0 |      0 |      0 |        8 | |                100 | [200, "run"]                                   |
// | GT_MAC[0]        | |       5 |         0 |      0 |      0 |      0 |        0 | |                100 | [200, "run"]                                   |
// +------------------+ +-----------------------------------------------------------+ +---------------------------------------------------------------------+

    uint STAT_COL_SIZE_0 = STAT_COL_NAME_0.size();
    uint STAT_COL_SIZE_1 = STAT_COL_NAME_1.size();
    uint STAT_COL_SIZE_2 = STAT_COL_NAME_2.size();
    uint STAT_COL_SIZE_3 = STAT_COL_NAME_3.size();
    uint STAT_COL_SIZE_4 = STAT_COL_NAME_4.size();
    uint STAT_COL_SIZE_5 = STAT_COL_NAME_5.size();

    uint ONGO_COL_SIZE_0 = ONGO_COL_NAME_0.size(); // long enough!
    uint ONGO_COL_SIZE_1 = 46; // Don't know what will be test parameters, this should be enough

    uint TOP_COL_SIZE_0 = 16; // Don't know what will be testcase name, this should be enough
    uint TOP_COL_SIZE_1 = STAT_COL_SIZE_0+3+STAT_COL_SIZE_1+3+STAT_COL_SIZE_2+3+STAT_COL_SIZE_3+3+STAT_COL_SIZE_4+3+STAT_COL_SIZE_5; // + 3 each time for " | "
    uint TOP_COL_SIZE_2 = ONGO_COL_SIZE_0+3+ONGO_COL_SIZE_1;

    uint TESTCASE_SIZE  = TOP_COL_SIZE_0+5+TOP_COL_SIZE_1+5+TOP_COL_SIZE_2; // + 5 each time for "-+ +-"

    col_vect.clear();
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_0, PAD_ON_RIGHT) + "-|"); // +1 for extra  + " |"
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_1, PAD_ON_RIGHT) + "-|"); // Add "|" for end of STAT and remove 1 to the size before
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_2, PAD_ON_RIGHT) + "-|"); // Add "|" for end of STAT and remove 1 to the size before
    auto row_delimiter = StrVectToStr(col_vect, " ");

    col_vect.clear();
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_0, PAD_ON_RIGHT) + "-+"); // +2 for extra  + " |"
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_1, PAD_ON_RIGHT) + "-+"); // Add "|" for end of STAT and remove 1 to the size before
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_2, PAD_ON_RIGHT) + "-+"); // Add "|" for end of STAT and remove 1 to the size before
    auto table_delimiter = StrVectToStr(col_vect, " ");

    // first row delimitation
    table.emplace_back(table_delimiter);

    // top row 0, center names
    col_vect.clear();
    col_vect.emplace_back("| " + pad("",                                                                              ' ', TOP_COL_SIZE_0, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_1, ' ', TOP_COL_SIZE_1/2+TOP_COL_NAME_1.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_1, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_2, ' ', TOP_COL_SIZE_2/2+TOP_COL_NAME_2.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_2, PAD_ON_RIGHT) + " |");
    table.emplace_back(StrVectToStr(col_vect, " "));

    // top row 1, center names
    col_vect.clear();
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_0, ' ', TOP_COL_SIZE_0/2+TOP_COL_NAME_0.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_0, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("|-" + pad("",                                                                              '-', TOP_COL_SIZE_1, PAD_ON_RIGHT) + "-|");
    col_vect.emplace_back("|-" + pad("",                                                                              '-', TOP_COL_SIZE_2, PAD_ON_RIGHT) + "-|");
    table.emplace_back(StrVectToStr(col_vect, " "));

    // header row
    col_vect.clear();
    col_vect.emplace_back("| " + pad("",              ' ', TOP_COL_SIZE_0,  PAD_ON_LEFT) + " |");

    col_vect.emplace_back("| " + pad(STAT_COL_NAME_0, ' ', STAT_COL_SIZE_0, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(STAT_COL_NAME_1, ' ', STAT_COL_SIZE_1, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(STAT_COL_NAME_2, ' ', STAT_COL_SIZE_2, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(STAT_COL_NAME_3, ' ', STAT_COL_SIZE_3, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(STAT_COL_NAME_4, ' ', STAT_COL_SIZE_4, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(STAT_COL_NAME_5, ' ', STAT_COL_SIZE_5, PAD_ON_LEFT) + " |");

    col_vect.emplace_back("| " + pad(ONGO_COL_NAME_0, ' ', ONGO_COL_SIZE_0, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(ONGO_COL_NAME_1, ' ', ONGO_COL_SIZE_1, PAD_ON_RIGHT) + " |");

    table.emplace_back(StrVectToStr(col_vect, " "));

    // third row delimitation
    // table.emplace_back(row_delimiter);

    // table content rows
    for (const auto & testcase_stat : this->m_testcase_queues)
    {
        auto testcase = testcase_stat.first;
        auto testcase_row_present = true;
        if ((testcase == VERIFY_STR) || (testcase == POWER_MEMBER.name) || (testcase == MMIO_MEMBER.name) || (testcase == DMA_MEMBER.name) || (testcase == P2P_CARD_MEMBER.name) || (testcase == P2P_NVME_MEMBER.name))
        {
            testcase_row_present = false;
        }

        table.emplace_back(row_delimiter);

        // testcase name row
        if (testcase_row_present)
        {
            table.emplace_back("| " + pad(testcase, ' ', TESTCASE_SIZE, PAD_ON_RIGHT) + " |");
            // table.emplace_back(row_delimiter);
        }

        for (const auto & thread_stat : testcase_stat.second)
        {
            auto thread     = thread_stat.first;
            auto thread_pad = PAD_ON_RIGHT;
            if (testcase_row_present)
            {
                thread_pad = PAD_ON_LEFT;
            }
            col_vect.clear();
            col_vect.emplace_back("| " + pad(               thread,                                                       ' ', TOP_COL_SIZE_0,  thread_pad) + " |");
            col_vect.emplace_back("| " + pad(std::to_string(this->m_testcase_queue_value_last[testcase][thread].pending),       ' ', STAT_COL_SIZE_0, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_testcase_queue_value_last[testcase][thread].completed),     ' ', STAT_COL_SIZE_1, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_testcase_queue_value_last[testcase][thread].passed),        ' ', STAT_COL_SIZE_2, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_testcase_queue_value_last[testcase][thread].failed),        ' ', STAT_COL_SIZE_3, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_testcase_queue_value_last[testcase][thread].errors),        ' ', STAT_COL_SIZE_4, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_testcase_queue_value_last[testcase][thread].warnings),      ' ', STAT_COL_SIZE_5, PAD_ON_LEFT)  + " |"); // Add " |" for end of STAT
            col_vect.emplace_back("| " + pad(               this->m_testcase_queue_value_last[testcase][thread].remaining_time, ' ', ONGO_COL_SIZE_0, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(               this->m_testcase_queue_value_last[testcase][thread].parameters,     ' ', ONGO_COL_SIZE_1, PAD_ON_RIGHT) + " |");
            table.emplace_back(StrVectToStr(col_vect, " "));
        }
    }

    // last row delimitation
    table.emplace_back(table_delimiter);

    return table;
}

} // namespace
