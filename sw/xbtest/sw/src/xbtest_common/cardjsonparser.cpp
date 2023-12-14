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

#include "cardjsonparser.h"

namespace xbtest
{

CardJsonParser::CardJsonParser( Logging * log, const std::string & filename, std::atomic<bool> * abort ) : XJsonParser::XJsonParser(log, abort)
{
    this->m_log_header    = LOG_HEADER_CARD_PARSER;
    this->m_filename      = filename;
    this->m_content_name  = CARD_JSON_FILE;
}

CardJsonParser::~CardJsonParser()
{
    ClearParser();
}

bool CardJsonParser::Parse()
{
    // Parse JSON file
    LogMessage(MSG_ITF_045, {this->m_content_name, this->m_filename});

    // Check provided multicard json file exists
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {CARD_JSON_FILE, this->m_filename});
        return RET_FAILURE;
    }

    // Initialize json parser and reader
    this->m_json_parser = json_parser_new();

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_content_name, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free (error);
        g_object_unref (this->m_json_parser);
        return RET_FAILURE;
    }

    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    this->m_json_reader = json_reader_new(nullptr);
    set_json_root (this->m_json_root_node);
    this->m_json_parsed = true;

    this->m_tests_config_json_def = GetTestsConfigJsonDefinition();

    // Check the json file content against the defined members at root level only
    LogMessage(MSG_DEBUG_PARSING, {"Check the JSON file content at root level"});
    this->m_root_json_def = GetRootJsonDefinition();
    std::vector<std::string> node_title;
    if (CheckMembers(node_title, this->m_root_json_def, false) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Parse the json file content against the defined members at global_config
    LogMessage(MSG_DEBUG_PARSING, {"Check the JSON file content at global_config level"});
    if (ParseGlobalConfig() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // Parse the json file content against the defined members at global_config
    LogMessage(MSG_DEBUG_PARSING, {"Check the JSON file content at card_config level"});
    if (ParseCardConfig() == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    PrintConfig();
    return RET_SUCCESS;
}

std::map<std::string,Card_Config_t> CardJsonParser::GetCardConfigMap()
{
    return this->m_multicard_config.card;
}

bool CardJsonParser::ParseGlobalConfig()
{
    auto parse_failure = RET_SUCCESS;
    std::vector<std::string> node_title;

    // Check the json file content against the defined members at global_config level and below
    uint num_node_read = 0;
    node_title = {GLOBAL_CONFIG};
    ExtractNode(node_title, num_node_read); // Move cursor to GLOBAL_CONFIG node
    this->m_global_config_json_def = GetGlobalConfigJsonDefinition();
    if (CheckMembers(node_title, this->m_global_config_json_def, true) == RET_FAILURE)
    {
        parse_failure = RET_FAILURE;
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from tests array
    {
        end_json_element();
    }
    if (parse_failure == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    // CARDS
    node_title = {GLOBAL_CONFIG, CARDS};
    if (ExtractNodeArrayStr(node_title, this->m_multicard_config.global.cards) == RET_FAILURE)
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    if (this->m_multicard_config.global.cards.size() > MAX_NUM_TEST)
    {
        LogMessage(MSG_ITF_114, {std::to_string(this->m_multicard_config.global.cards.size()), StrVectToStr(node_title, "."), std::to_string(MAX_NUM_CARD)});
        return RET_FAILURE;
    }
    // Check if bdf provided is correct
    for (const auto & bdf : this->m_multicard_config.global.cards)
    {
        if (CheckCardBDF(bdf) == RET_FAILURE)
        {
            LogMessage(MSG_ITF_109, {bdf, StrVectToStr(node_title, "."), "Incorrect format"});
            return RET_FAILURE;
        }
    }
    // ARGS
    node_title = {GLOBAL_CONFIG, ARGS};
    if (NodeExists(node_title))
    {
        std::string tmp;
        if (ExtractNodeValueStr(node_title, tmp) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
        this->m_multicard_config.global.args = SplitNoEmpty(tmp, ' ');
    }
    // Parse the json file content against the defined members at tests level
    node_title = {GLOBAL_CONFIG, TESTS};
    if (!NodeExists(node_title))
    {
        PrintRequiredNotFound(node_title);
        return RET_FAILURE;
    }
    if (ParseTestsConfig(node_title, this->m_multicard_config.global.tests, this->m_multicard_config.global.args) == RET_FAILURE)
    {
        LogMessage(MSG_DEBUG_PARSING, {"Failed to parse JSON file content at " + StrVectToStr(node_title, ".") + " level"});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool CardJsonParser::ParseTestsConfig( const std::vector<std::string> & base_node_title_in, std::vector<Test_Config_t> & tests, const std::vector<std::string> & default_args )
{
    auto parse_failure      = RET_SUCCESS;
    auto base_node_title    = base_node_title_in;
    std::vector<std::string> node_title;

    tests.clear();

    LogMessage(MSG_DEBUG_PARSING, {"Check the JSON file content at " + StrVectToStr(base_node_title, ".") + " level"});
    uint num_node_read = 0;
    ExtractNode(base_node_title, num_node_read); // Move cursor to tests array

    auto tests_array_size = count_json_elements();
    if (tests_array_size == 0)
    {
        LogMessage(MSG_ITF_115, {StrVectToStr(base_node_title, ".")});
        parse_failure = RET_FAILURE;
    }
    else if (tests_array_size > MAX_NUM_TEST)
    {
        LogMessage(MSG_ITF_114, {std::to_string(tests_array_size), StrVectToStr(base_node_title, "."), std::to_string(MAX_NUM_TEST)});
        parse_failure = RET_FAILURE;
    }
    for (gint j = 0; (j < tests_array_size) && (parse_failure == RET_SUCCESS); j++) // For each element in tests array
    {
        base_node_title.emplace_back(std::to_string(j));

        // Check content of tests element
        read_json_element(j); // Move cursor to tests array element
        if (is_json_object() == 0)
        {
            LogMessage(MSG_ITF_104, {StrVectToStr(base_node_title, "."), "object"});
            parse_failure = RET_FAILURE;
        }
        if (parse_failure == RET_SUCCESS)
        {
            parse_failure = CheckMembers(base_node_title, this->m_tests_config_json_def, false);
        }

        // Parse content of tests element
        auto test = RESET_TEST_CONFIG;

        // ARGS
        if (parse_failure == RET_SUCCESS)
        {
            node_title = {ARGS};
            test.args = default_args; // Defaults to higher level
            if (NodeExists(node_title))
            {
                std::string tmp;
                if (ExtractNodeValueStr(node_title, tmp) == RET_FAILURE)
                {
                    parse_failure = RET_FAILURE;
                }
                test.args = SplitNoEmpty(tmp, ' ');
            }
        }
        // PRECANNED
        if (parse_failure == RET_SUCCESS)
        {
            node_title = {PRECANNED};
            if (NodeExists(node_title))
            {
                test.is_pre_canned = true;
                if (ExtractNodeValueStr(node_title, test.pre_canned) == RET_FAILURE)
                {
                    parse_failure = RET_FAILURE;
                }
            }
        }
        // TESTJSON
        if (parse_failure == RET_SUCCESS)
        {
            node_title = {TESTJSON};
            if (NodeExists(node_title))
            {
                if (test.is_pre_canned) // Check parameters are not combined
                {
                    LogMessage(MSG_ITF_110, {StrVectToStr(base_node_title, "."), TESTJSON, PRECANNED});
                    parse_failure = RET_FAILURE;
                }
                else
                {
                    test.is_test_json = true;
                    if (ExtractNodeValueStr(node_title, test.test_json) == RET_FAILURE)
                    {
                        parse_failure = RET_FAILURE;
                    }
                }
            }
        }

        if (parse_failure == RET_SUCCESS)
        {
            tests.emplace_back(test);
        }

        base_node_title.pop_back();

        end_json_element(); // Move back cursor to tests array
    }
    for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from tests array
    {
        end_json_element();
    }
    return parse_failure;
}

bool CardJsonParser::ParseCardConfig()
{
    auto parse_failure = RET_SUCCESS;
    std::vector<std::string> node_title;

    // Check the json file content against the defined members at card_config level and below
    uint num_node_read = 0;
    node_title = {CARD_CONFIG};
    if (NodeExists(node_title)) // Don't return now if it does not exist as we will populate default value after
    {
        ExtractNode(node_title, num_node_read); // Move cursor to CARD_CONFIG node
        this->m_card_config_json_def = GetCardConfigJsonDefinition();

        parse_failure = CheckMembers(node_title, this->m_card_config_json_def, true);

        for (uint ii = 0; ii < num_node_read; ii++ ) // Move cursor back from tests array
        {
            end_json_element();
        }
        if (parse_failure == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }

    for (const auto & bdf : this->m_multicard_config.global.cards)
    {
        Card_Config_t card_config;

        // ARGS
        node_title = {CARD_CONFIG, bdf, ARGS};
        card_config.args = this->m_multicard_config.global.args; // Defaults to global config
        if (NodeExists(node_title))
        {
            std::string tmp;
            if (ExtractNodeValueStr(node_title, tmp) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
            card_config.args = SplitNoEmpty(tmp, ' ');
        }

        // Parse the json file content against the defined members at tests level
        node_title = {CARD_CONFIG, bdf, TESTS};
        card_config.tests = this->m_multicard_config.global.tests; // Defaults to global config
        if (NodeExists(node_title))
        {
            if (ParseTestsConfig(node_title, card_config.tests, card_config.args) == RET_FAILURE)
            {
                return RET_FAILURE;
            }
        }
        this->m_multicard_config.card.emplace(bdf, card_config);
    }

    if (this->m_multicard_config.card.empty())
    {
        LogMessage(MSG_ITF_112);
        return RET_FAILURE;
    }
    return parse_failure;
}

void CardJsonParser::PrintConfig()
{
    LogMessage(MSG_DEBUG_SETUP, {"Global configuration:"});
    if (!(this->m_multicard_config.global.args.empty()))
    {
        LogMessage(MSG_DEBUG_SETUP, {"\t - " + ARGS  + ": " + StrVectToStr(this->m_multicard_config.global.args, " ")});
    }
    LogMessage(MSG_DEBUG_SETUP, {"\t - " + CARDS + ": " + StrVectToStr(this->m_multicard_config.global.cards, ", ")});
    uint i = 1;
    for (const auto & test : this->m_multicard_config.global.tests)
    {
        std::vector<std::string> tmp;
        if (test.is_pre_canned)
        {
            tmp.emplace_back(PRECANNED + ": " + test.pre_canned);
        }
        else if (test.is_test_json)
        {
            tmp.emplace_back(TESTJSON + ": " + test.test_json);
        }
        if (!test.args.empty())
        {
            tmp.emplace_back(ARGS + ": " + StrVectToStr(test.args, " "));
        }
        LogMessage(MSG_DEBUG_SETUP, {"\t - Test " + std::to_string(i) + ") " + StrVectToStr(tmp, " / ")});
        i++;
    }
    for (const auto & cfg : this->m_multicard_config.card)
    {
        auto card_config = cfg.second;

        LogMessage(MSG_DEBUG_SETUP, {"Card " + cfg.first + " configuration:"});
        if (!card_config.args.empty())
        {
            LogMessage(MSG_DEBUG_SETUP, {"\t - " + ARGS + ": " + StrVectToStr(card_config.args, " ")});
        }
        i = 1;
        for (const auto & test : card_config.tests)
        {
            std::vector<std::string> tmp;
            if (test.is_pre_canned)
            {
                tmp.emplace_back(PRECANNED + ": " + test.pre_canned);
            }
            else if (test.is_test_json)
            {
                tmp.emplace_back(TESTJSON + ": " + test.test_json);
            }
            if (!test.args.empty())
            {
                tmp.emplace_back(ARGS + ": " + StrVectToStr(test.args, " "));
            }
            LogMessage(MSG_DEBUG_SETUP, {"\t - Test " + std::to_string(i+1) + ") " + StrVectToStr(tmp, " / ")});
            i++;
        }
    }
}

Json_Definition_t CardJsonParser::GetRootJsonDefinition()
{
    Json_Definition_t json_definition;
    json_definition.insert( Definition_t({GLOBAL_CONFIG},   JSON_NODE_OBJECT));
    json_definition.insert( Definition_t({CARD_CONFIG},     JSON_NODE_OBJECT));
    return json_definition;
}

Json_Definition_t CardJsonParser::GetGlobalConfigJsonDefinition()
{
    Json_Definition_t json_definition;
    json_definition.insert( Definition_t({ARGS},   JSON_NODE_VALUE));
    json_definition.insert( Definition_t({CARDS},  JSON_NODE_ARRAY));
    json_definition.insert( Definition_t({TESTS},  JSON_NODE_ARRAY));
    return json_definition;
}

Json_Definition_t CardJsonParser::GetCardConfigJsonDefinition()
{
    Json_Definition_t json_definition;
    for (const auto & bdf : this->m_multicard_config.global.cards)
    {
        json_definition.insert( Definition_t({bdf},         JSON_NODE_OBJECT));
        json_definition.insert( Definition_t({bdf, ARGS},   JSON_NODE_VALUE));
        json_definition.insert( Definition_t({bdf, TESTS},  JSON_NODE_ARRAY));
    }
    return json_definition;
}

Json_Definition_t CardJsonParser::GetTestsConfigJsonDefinition()
{
    Json_Definition_t json_definition;
    json_definition.insert( Definition_t({ARGS},       JSON_NODE_VALUE));
    json_definition.insert( Definition_t({TESTJSON},  JSON_NODE_VALUE));
    json_definition.insert( Definition_t({PRECANNED},  JSON_NODE_VALUE));
    return json_definition;
}

void CardJsonParser::PrintRequiredNotFound( const std::vector<std::string> & node_title_in )
{
    LogMessage(MSG_ITF_044, {this->m_content_name, StrVectToStr(node_title_in, ".")});
}

} // namespace
