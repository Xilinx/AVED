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

#include "xbtestcommonpackage.h"
#include "configparser.h"

namespace xbtest
{

ConfigParser::ConfigParser( Logging * log, const std::string & filename, std::atomic<bool> * abort ) : XJsonParser::XJsonParser(log, abort)
{
    this->m_log_header    = LOG_HEADER_CONFIG_PARSER;
    this->m_filename      = filename;
    this->m_content_name  = CONFIG_JSON;
}

ConfigParser::~ConfigParser()
{
    ClearParser();
}

bool ConfigParser::Parse()
{
    // Check file exists
    if (!FileExists(this->m_filename))
    {
        LogMessage(MSG_ITF_003, {this->m_content_name, this->m_filename});
        return RET_FAILURE;
    }

    // Initialize json parser and reader
    this->m_json_parser = json_parser_new();

    GError * error = nullptr;
    json_parser_load_from_file (this->m_json_parser, this->m_filename.c_str(), &error);
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_013, {this->m_content_name, "<file name>:<line>:<character>: <error message>", std::string(error->message)});
        g_error_free(error);
        g_object_unref(this->m_json_parser);
        return RET_FAILURE;
    }

    this->m_json_root_node = json_parser_get_root(this->m_json_parser);
    this->m_json_reader = json_reader_new(nullptr);
    set_json_root(this->m_json_root_node);
    this->m_json_parsed = true;

    return RET_SUCCESS;
}

} // namespace
