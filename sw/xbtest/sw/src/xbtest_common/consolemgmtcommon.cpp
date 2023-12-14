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

#include "consolemgmtcommon.h"

namespace xbtest
{

ConsoleMgmtCommon::ConsoleMgmtCommon( Logging * log, Timer * timer, XbtestCommonConfig * xbtest_config ) : ConsoleMgmtBase::ConsoleMgmtBase(log, timer)
{
    this->m_xbtest_config = xbtest_config;
}

ConsoleMgmtCommon::~ConsoleMgmtCommon() = default;

bool ConsoleMgmtCommon::SetUp()
{
    LogMessage(MSG_DEBUG_SETUP, {"Set-up"});
    // We don't want to use the xbtest-common dynamic display when a single test is run (it will be xbtest-sw) or when user disable it (-F)
    if (!(this->m_xbtest_config->GetMultitestMode()) || this->m_xbtest_config->GetCommandLineConsoleDetailedFormat())
    {
        this->m_detailed_format = true;
        this->m_messages_before_test.clear();
    }

    if (this->m_xbtest_config->GetUseCommonLogDir())
    {
        auto filename = this->m_xbtest_config->GetCommonLogDir() + "/card_status.csv";
        LogMessage(MSG_CMN_020, {filename});
        if (FileExists(filename))
        {
            if (this->m_xbtest_config->GetCommandLineForce())
            {
                LogMessage(MSG_CMN_019, {filename}); // warning
            }
            else
            {
                LogMessage(MSG_CMN_053, {filename});
                return RET_FAILURE;
            }
        }
        if (!OpenFile(filename, this->m_card_status_csv_file))
        {
            LogMessage(MSG_CMN_018, {filename});
            return RET_FAILURE;
        }

        filename = this->m_xbtest_config->GetCommonRTLogDir() + "/card_status.csv";
        LogMessage(MSG_CMN_020, {filename});
        if (FileExists(filename))
        {
            if (this->m_xbtest_config->GetCommandLineForce())
            {
                LogMessage(MSG_CMN_019, {filename}); // warning
            }
            else
            {
                LogMessage(MSG_CMN_053, {filename});
                return RET_FAILURE;
            }
        }
        this->m_RT_card_status_csv_file_name = filename;
        if (!OpenFile(this->m_RT_card_status_csv_file_name, this->m_RT_card_status_csv_file))
        {
            LogMessage(MSG_CMN_018, {filename});
            return RET_FAILURE;
        }
    }
    return RET_SUCCESS;
}

void ConsoleMgmtCommon::RefreshData()
{
    std::vector<std::string> card_status_csv_row;

    // Write CSV file header
    if (this->m_xbtest_config->GetUseCommonLogDir())
    {
        if (!(this->m_data_exists))
        {
            // write csv header
            card_status_csv_row.clear();
            card_status_csv_row.emplace_back("Global Time (s)");
            for (const auto & dsa_stat : this->m_card_test_queues)
            {
                for (const auto & card_stat : dsa_stat.second)
                {
                    card_status_csv_row.emplace_back(card_stat.first + " ongoing test name");
                    card_status_csv_row.emplace_back(card_stat.first + " ongoing test time (s)");
                    card_status_csv_row.emplace_back(card_stat.first + " measurement ID");
                    card_status_csv_row.emplace_back(card_stat.first + " measurement valid");
                    card_status_csv_row.emplace_back(card_stat.first + " power (W)");
                    card_status_csv_row.emplace_back(card_stat.first + " temperature (C)");
                }
            }
            // Write row in CSV file
            this->m_card_status_csv_file << StrVectToStr(card_status_csv_row, ",") << "\n";
            this->m_card_status_csv_file.flush();

            this->m_RT_card_status_csv_file_head = StrVectToStr(card_status_csv_row, ",");
            this->m_RT_card_status_csv_file.close();
        }

        // prepare row for data, insert time in first column
        card_status_csv_row.clear();
        card_status_csv_row.emplace_back(std::to_string(this->m_elapsed));
    }

    for (const auto & dsa_stat : this->m_card_test_queues)
    {
        for (const auto & card_stat : dsa_stat.second)
        {
            // flush both queues and get the last data available
            while (this->m_card_test_queues[dsa_stat.first][card_stat.first]->Pop(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first]))
            {
            }

            while (this->m_sensor_queues[dsa_stat.first][card_stat.first]->Pop(this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first]))
            {
            }

            if (this->m_xbtest_config->GetUseCommonLogDir())
            {
                std::string power_str       = "0.0";
                std::string temperature_str = "0.0";
                if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].dump_cnt > 0)
                {
                    if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].power_found)
                    {
                        power_str = Float_to_String<double>(this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].power, 1);
                    }
                    if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].temperature_found)
                    {
                        temperature_str = Float_to_String<double>(this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].temperature, 0);
                    }
                }

                card_status_csv_row.emplace_back(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].test);
                card_status_csv_row.emplace_back(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].elapsed_time);
                card_status_csv_row.emplace_back(std::to_string(this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].dump_cnt));
                if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].dump_valid)
                {
                    card_status_csv_row.emplace_back("OK");
                }
                else
                {
                    card_status_csv_row.emplace_back("KO");
                }
                card_status_csv_row.emplace_back(power_str);
                card_status_csv_row.emplace_back(temperature_str);
            }
        }
    }
    if (this->m_xbtest_config->GetUseCommonLogDir() && (this->m_elapsed > 0))
    {
        this->m_card_status_csv_file  << StrVectToStr(card_status_csv_row,    ",") << std::endl << std::flush;

        (void)OpenFile(this->m_RT_card_status_csv_file_name, this->m_RT_card_status_csv_file); // Don't check if open worked..
        this->m_RT_card_status_csv_file  << this->m_RT_card_status_csv_file_head + "\n" + StrVectToStr(card_status_csv_row,    ",");
        this->m_RT_card_status_csv_file.flush();
        this->m_RT_card_status_csv_file.close();
    }
    this->m_data_exists = true;
}

void ConsoleMgmtCommon::AllocateCardTestQueue( const std::string & dsa_name, const std::string & bdf, const Card_Test_Queue_Value_t & value )
{
    auto * queue = new XQueue<Card_Test_Queue_Value_t>;
    this->m_card_test_queues[dsa_name][bdf] = queue;
    PushCardTestQueue(dsa_name, bdf, value);
    this->m_card_test_queue_value_last[dsa_name][bdf] = value;
}

void ConsoleMgmtCommon::PushCardTestQueue( const std::string & dsa_name, const std::string & bdf, const Card_Test_Queue_Value_t & value )
{
    this->m_card_test_queues[dsa_name][bdf]->Push(value);
}

bool ConsoleMgmtCommon::CardTestQueueEmpty( const std::string & dsa_name, const std::string & bdf )
{
    return this->m_card_test_queues[dsa_name][bdf]->Empty();
}

void ConsoleMgmtCommon::AllocateSensorQueue( const std::string & dsa_name, const std::string & bdf, const Sensor_Queue_Value_t & value )
{
    auto * queue = new XQueue<Sensor_Queue_Value_t>;
    this->m_sensor_queues[dsa_name][bdf] = queue;
    PushSensorQueue(dsa_name, bdf, value);
    this->m_sensor_queue_value_last[dsa_name][bdf] = value;
}

void ConsoleMgmtCommon::PushSensorQueue( const std::string & dsa_name, const std::string & bdf, const Sensor_Queue_Value_t & value )
{
    this->m_sensor_queues[dsa_name][bdf]->Push(value);
}

std::vector<std::string> ConsoleMgmtCommon::UpdateContent( const bool & last )
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
    if (!(this->m_card_test_queues.empty()))
    {
        auto table = UpdateConsoleTable();
        console_lines.insert(console_lines.end(), table.begin(), table.end());
        console_lines.emplace_back(""); // space
        this->m_table_exists = true;
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
        console_lines.insert(console_lines.end(), this->m_messages_during_test.end()-limit, this->m_messages_during_test.end());
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

std::vector<std::string> ConsoleMgmtCommon::UpdateConsoleTable()
{
    std::vector<std::string> table;
    std::vector<std::string> col_vect;

// +------------------------------------------+ +-----------------------------+ +-------------------------------------------+ +---------------------------------------+
// |                                          | |           STATUS            | |               ONGOING TEST                | |             TEST SUMMARY              |
// |                   CARD                   | |-----------------------------| |-------------------------------------------| |---------------------------------------|
// |                                          | | Power (W) | Temperature (C) | | Name                           | Time (s) | | Pending | Completed | Passed | Failed |
// |------------------------------------------| |-----------------------------| |-------------------------------------------| |---------------------------------------|
// | xilinx_u25_gen3x8_xdma_base_1                                                                                                                                    |
// |------------------------------------------| |-----------------------------| |-------------------------------------------| |---------------------------------------|
// |                             0000:5e:00.1 | |        32 |              45 | | 003_power                      |       38 | |       3 |         2 |      2 |      0 |
// |------------------------------------------| |-----------------------------| |-------------------------------------------| |---------------------------------------|
// | xilinx_u50lv_gen3x4_xdma_base_2                                                                                                                                  |
// |------------------------------------------| |-----------------------------| |-------------------------------------------| |---------------------------------------|
// |                             0000:86:00.1 | |        44 |              62 | | n/a                            |      n/a | |       0 |         4 |      2 |      2 |
// |                             0000:a7:00.1 | |        47 |              61 | | 001_stress                     |      100 | |       1 |         0 |      0 |      0 |
// |------------------------------------------| |-----------------------------| |-------------------------------------------| |---------------------------------------|
// | xilinx_u50_gen3x16_xdma_base_4                                                                                                                                   |
// |------------------------------------------| |-----------------------------| |-------------------------------------------| |---------------------------------------|
// |                             0000:d9:00.1 | |        29 |              50 | | 002_mytest                     |       15 | |       1 |         1 |      1 |      0 |
// +------------------------------------------+ +-----------------------------+ +-------------------------------------------+ +---------------------------------------+

    const std::string TOP_COL_NAME_0 = "CARD";
    const std::string TOP_COL_NAME_1 = "STATUS";
    const std::string TOP_COL_NAME_2 = "ONGOING TEST";
    const std::string TOP_COL_NAME_3 = "TEST SUMMARY";

    const std::string CARD_STAT_COL_NAME_0 = "Power (W)";
    const std::string CARD_STAT_COL_NAME_1 = "Temperature (C)";

    const std::string ONGO_COL_NAME_0 = "Name";
    const std::string ONGO_COL_NAME_1 = "Time (s)";

    const std::string TEST_STAT_COL_NAME_0 = "Pending";
    const std::string TEST_STAT_COL_NAME_1 = "Completed";
    const std::string TEST_STAT_COL_NAME_2 = "Passed";
    const std::string TEST_STAT_COL_NAME_3 = "Failed";

    uint CARD_STAT_COL_SIZE_0 = CARD_STAT_COL_NAME_0.size();
    uint CARD_STAT_COL_SIZE_1 = CARD_STAT_COL_NAME_1.size();

    uint ONGO_COL_SIZE_0 = 30; // Don't know what will be test name, this should be enough
    uint ONGO_COL_SIZE_1 = ONGO_COL_NAME_1.size(); // long enough!

    uint TEST_STAT_COL_SIZE_0 = TEST_STAT_COL_NAME_0.size();
    uint TEST_STAT_COL_SIZE_1 = TEST_STAT_COL_NAME_1.size();
    uint TEST_STAT_COL_SIZE_2 = TEST_STAT_COL_NAME_2.size();
    uint TEST_STAT_COL_SIZE_3 = TEST_STAT_COL_NAME_3.size();

    uint TOP_COL_SIZE_0 = 40; // Don't know what will be dsa_name, this should be enough
    uint TOP_COL_SIZE_1 = CARD_STAT_COL_SIZE_0+3+CARD_STAT_COL_SIZE_1;
    uint TOP_COL_SIZE_2 = ONGO_COL_SIZE_0+3+ONGO_COL_SIZE_1;
    uint TOP_COL_SIZE_3 = TEST_STAT_COL_SIZE_0+3+TEST_STAT_COL_SIZE_1+3+TEST_STAT_COL_SIZE_2+3+TEST_STAT_COL_SIZE_3; // + 3 each time for " | "

    uint DSA_NAME_SIZE = TOP_COL_SIZE_0+5+TOP_COL_SIZE_1+5+TOP_COL_SIZE_2+5+TOP_COL_SIZE_3; // + 5 each time for "-+ +-"

    col_vect.clear();
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_0, PAD_ON_RIGHT) + "-|");
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_1, PAD_ON_RIGHT) + "-|");
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_2, PAD_ON_RIGHT) + "-|");
    col_vect.emplace_back("|-" + pad("", '-', TOP_COL_SIZE_3, PAD_ON_RIGHT) + "-|");
    auto row_delimiter = StrVectToStr(col_vect, " ");

    col_vect.clear();
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_0, PAD_ON_RIGHT) + "-+");
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_1, PAD_ON_RIGHT) + "-+");
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_2, PAD_ON_RIGHT) + "-+");
    col_vect.emplace_back("+-" + pad("", '-', TOP_COL_SIZE_3, PAD_ON_RIGHT) + "-+");
    auto table_delimiter = StrVectToStr(col_vect, " ");

    // first row delimitation
    table.emplace_back(table_delimiter);

    // top row 0, center names
    col_vect.clear();
    col_vect.emplace_back("| " + pad("",                                                                              ' ', TOP_COL_SIZE_0, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_1, ' ', TOP_COL_SIZE_1/2+TOP_COL_NAME_1.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_1, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_2, ' ', TOP_COL_SIZE_2/2+TOP_COL_NAME_2.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_2, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_3, ' ', TOP_COL_SIZE_3/2+TOP_COL_NAME_3.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_3, PAD_ON_RIGHT) + " |");
    table.emplace_back(StrVectToStr(col_vect, " "));

    // top row 1, center names
    col_vect.clear();
    col_vect.emplace_back("| " + pad(pad(TOP_COL_NAME_0, ' ', TOP_COL_SIZE_0/2+TOP_COL_NAME_0.size()/2, PAD_ON_LEFT), ' ', TOP_COL_SIZE_0, PAD_ON_RIGHT) + " |");
    col_vect.emplace_back("|-" + pad("",                                                                              '-', TOP_COL_SIZE_1, PAD_ON_RIGHT) + "-|");
    col_vect.emplace_back("|-" + pad("",                                                                              '-', TOP_COL_SIZE_2, PAD_ON_RIGHT) + "-|");
    col_vect.emplace_back("|-" + pad("",                                                                              '-', TOP_COL_SIZE_3, PAD_ON_RIGHT) + "-|");
    table.emplace_back(StrVectToStr(col_vect, " "));

    // top row 2
    col_vect.clear();
    col_vect.emplace_back("| " + pad("",                   ' ', TOP_COL_SIZE_0,       PAD_ON_LEFT) + " |");

    col_vect.emplace_back("| " + pad(CARD_STAT_COL_NAME_0, ' ', CARD_STAT_COL_SIZE_0, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(CARD_STAT_COL_NAME_1, ' ', CARD_STAT_COL_SIZE_1, PAD_ON_LEFT) + " |");

    col_vect.emplace_back("| " + pad(ONGO_COL_NAME_0,      ' ', ONGO_COL_SIZE_0,      PAD_ON_RIGHT));
    col_vect.emplace_back("| " + pad(ONGO_COL_NAME_1,      ' ', ONGO_COL_SIZE_1,      PAD_ON_LEFT) + " |");

    col_vect.emplace_back("| " + pad(TEST_STAT_COL_NAME_0, ' ', TEST_STAT_COL_SIZE_0, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(TEST_STAT_COL_NAME_1, ' ', TEST_STAT_COL_SIZE_1, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(TEST_STAT_COL_NAME_2, ' ', TEST_STAT_COL_SIZE_2, PAD_ON_LEFT));
    col_vect.emplace_back("| " + pad(TEST_STAT_COL_NAME_3, ' ', TEST_STAT_COL_SIZE_3, PAD_ON_LEFT) + " |");
    table.emplace_back(StrVectToStr(col_vect, " "));

    // table content rows
    for (const auto & dsa_stat : this->m_card_test_queues)
    {
        // DSA name row
        table.emplace_back(row_delimiter);
        table.emplace_back("| " + pad(dsa_stat.first, ' ', DSA_NAME_SIZE, PAD_ON_RIGHT) + " |");
        table.emplace_back(row_delimiter);

        for (const auto & card_stat : dsa_stat.second)
        {
            auto power_str       = NOT_APPLICABLE;
            auto temperature_str = NOT_APPLICABLE;
            if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].dump_cnt > 0)
            {
                if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].power_found)
                {
                    power_str = Float_to_String<double>(this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].power, 1);
                }
                if (this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].temperature_found)
                {
                    temperature_str = Float_to_String<double>(this->m_sensor_queue_value_last[dsa_stat.first][card_stat.first].temperature, 0);
                }
            }

            col_vect.clear();
            col_vect.emplace_back("| " + pad(               card_stat.first,                                                                  ' ', TOP_COL_SIZE_0,       PAD_ON_LEFT) + " |");

            col_vect.emplace_back("| " + pad(               power_str,                                                                        ' ', CARD_STAT_COL_SIZE_0, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(               temperature_str,                                                                  ' ', CARD_STAT_COL_SIZE_1, PAD_ON_LEFT) + " |"); // Add " |" for end of STAT

            col_vect.emplace_back("| " + pad(               this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].test,         ' ', ONGO_COL_SIZE_0,      PAD_ON_RIGHT));
            col_vect.emplace_back("| " + pad(               this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].elapsed_time, ' ', ONGO_COL_SIZE_1,      PAD_ON_LEFT) + " |");

            col_vect.emplace_back("| " + pad(std::to_string(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].pending),     ' ', TEST_STAT_COL_SIZE_0, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].completed),   ' ', TEST_STAT_COL_SIZE_1, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].passed),      ' ', TEST_STAT_COL_SIZE_2, PAD_ON_LEFT));
            col_vect.emplace_back("| " + pad(std::to_string(this->m_card_test_queue_value_last[dsa_stat.first][card_stat.first].failed),      ' ', TEST_STAT_COL_SIZE_3, PAD_ON_LEFT) + " |");

            table.emplace_back(StrVectToStr(col_vect, " "));
        }
    }

    // last row delimitation
    table.emplace_back(table_delimiter);

    return table;
}

} // namespace
