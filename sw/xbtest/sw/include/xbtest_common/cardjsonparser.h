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

#ifndef _CARDJSONPARSER_H
#define _CARDJSONPARSER_H

#include "xjsonparser.h"
#include "xbtestcommonpackage.h"

namespace xbtest
{

class CardJsonParser : public XJsonParser
{

public:
    CardJsonParser( Logging * log, const std::string & filename, std::atomic<bool> * abort );
    ~CardJsonParser();

    bool                                Parse() override;
    std::map<std::string,Card_Config_t> GetCardConfigMap();

private:
    std::string         m_filename;
    Card_JSON_Config_t  m_multicard_config;
    Json_Definition_t   m_root_json_def;
    Json_Definition_t   m_global_config_json_def;
    Json_Definition_t   m_card_config_json_def;
    Json_Definition_t   m_tests_config_json_def;

    const std::string   GLOBAL_CONFIG = "global_config";
    const std::string   CARDS         = "cards";
    const std::string   TESTS         = "tests";
    const std::string   PRECANNED     = "pre_canned";
    const std::string   TESTJSON      = "test_json";
    const std::string   CARD_CONFIG   = "card_config";

    bool                ParseGlobalConfig();
    bool                ParseTestsConfig( const std::vector<std::string> & base_node_title_in, std::vector<Test_Config_t> & tests, const std::vector<std::string> & default_args );
    bool                ParseCardConfig();
    void                PrintConfig();
    Json_Definition_t   GetRootJsonDefinition();
    Json_Definition_t   GetGlobalConfigJsonDefinition();
    Json_Definition_t   GetCardConfigJsonDefinition();
    Json_Definition_t   GetTestsConfigJsonDefinition();
    void                PrintRequiredNotFound( const std::vector<std::string> & node_title_in );
};

} // namespace

#endif /* _CARDJSONPARSER_H */
