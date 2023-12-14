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

#include "xjsonparser.h"

namespace xbtest
{

XJsonParser::XJsonParser( Logging * log, std::atomic<bool> * abort )
{
    this->m_log   = log;
    this->m_abort = abort;
}

XJsonParser::~XJsonParser() = default;

void XJsonParser::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    if (this->m_default_report || (message.log_level != LOG_FAILURE))
    {
        this->m_log->LogMessage(this->m_log_header, "", "", message, arg_list);
    }
    else
    {
        this->m_log->LogMessage(this->m_log_header, "", "", this->m_level, message, arg_list);
    }
    UpdateMessageHistory(message);
}

void XJsonParser::LogMessage ( const Message_t & message )
{
    if (!(this->m_default_report) && (message.log_level == LOG_FAILURE))
    {
        this->m_log->LogMessage(this->m_log_header, "", "", this->m_level, message);
    }
    else
    {
        this->m_log->LogMessage(this->m_log_header, "", "", message);
    }
    UpdateMessageHistory(message);
}

void XJsonParser::UpdateMessageHistory ( const Message_t & message )
{
    if (message.log_level > LOG_WARN)
    {
        auto log_level = message.log_level;
        if (!(this->m_default_report) && (message.log_level == LOG_FAILURE))
        {
            log_level = this->m_level;
        }
        this->m_message_history.emplace(message.id, log_level);
    }
}

void XJsonParser::ClearParser()
{
    if (this->m_json_reader != nullptr)
    {
        g_clear_object(&(this->m_json_reader));
    }
    if (this->m_json_parser != nullptr)
    {
        g_clear_object(&(this->m_json_parser));
    }
    this->m_json_parsed = false;
}

void XJsonParser::SetReportLevel( const LogLevel & level )
{
    this->m_level          = level;
    this->m_default_report = false;
}

bool XJsonParser::CheckReaderError()
{
    const auto error = get_json_error();
    if (error != nullptr)
    {
        LogMessage(MSG_JPR_001, {this->m_content_name, std::string(error->message)});
        return RET_FAILURE;
    }
   return RET_SUCCESS;
}

bool XJsonParser::ReadMemberNoCase( const std::string & node_title_in )
{
    // Check that JSON content is parsed before trying to read content
    if (!(this->m_json_parsed))
    {
        return RET_FAILURE;
    }
    gchar **member_list;
    std::string node_name = "ThisNameIsNotAllowedInJSONs"; // This means we expected this name not allowed in all parsed jsons
    if (ListNodeMembers(node_title_in, &member_list) == RET_SUCCESS)
    {
        if (count_json_members() > 0)
        {
            for (int j = 0; member_list[j] != nullptr; j++)
            {
                // Check that node is defined, without case sensitivity
                if (StrMatchNoCase(node_title_in, std::string(member_list[j])))
                {
                    node_name = std::string(member_list[j]);
                    break;
                }
            }
        }
    }
    g_strfreev (member_list);
    // Get node at next hierarchy level
    if (read_json_member((const gchar *)node_name.c_str()) == 0)
    {
        LogMessage(MSG_DEBUG_PARSING, {"ReadMemberNoCase: Failed to read: " + node_title_in});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool XJsonParser::NodeExists( const std::vector<std::string> & node_title_in )
{
    auto node_exists = true;
    LogMessage(MSG_DEBUG_PARSING, {"NodeExists: Checking node exists: " + StrVectToStr(node_title_in, ".")});
    if (node_title_in.empty())
    {
        return node_exists; // Nothing to extract
    }

    uint num_node_read = 0;
    for (const auto & node_name : node_title_in)
    {
        num_node_read++;
        if (ReadMemberNoCase(node_name) == RET_FAILURE) // Failed to get node
        {
            node_exists = false;
            break;
        }
    }
    if (this->m_json_parsed)
    {
        for (uint ii = 0; ii < num_node_read; ii++)
        {
            end_json_element();
        }
    }
    return node_exists;
}

bool XJsonParser::ExtractNode( const std::vector<std::string> & node_title_in, uint & num_node_read )
{
    auto ret = RET_SUCCESS;
    if (node_title_in.empty())
    {
        return ret; // Nothing to extract
    }

    LogMessage(MSG_DEBUG_PARSING, {"ExtractNode: Extracting node: " + StrVectToStr(node_title_in, ".")});
    num_node_read = 0;
    for (const auto & node_name : node_title_in)
    {
        num_node_read++;
        ret = ReadMemberNoCase(node_name);
        if (ret == RET_FAILURE)
        {
            break;
        }
    }

    if (ret == RET_FAILURE)
    {
        LogMessage(MSG_JPR_003, {this->m_content_name, StrVectToStr(node_title_in, ".")});
    }
    return ret;
}

bool XJsonParser::GetNodeValueBool( const std::string & name, bool & node_value )
{
    // Get value inside node
    auto ret = RET_SUCCESS;
    if (is_json_value() == 1)
    {
        auto node = get_json_value();
        if (json_node_get_value_type(node) == G_TYPE_BOOLEAN)
        {
            node_value = (get_json_boolean_value() != 0);
        }
        else
        {
            LogMessage(MSG_JPR_004, {this->m_content_name, name, "boolean"});
            ret = RET_FAILURE;
        }
        ret |= CheckReaderError();
    }
    else
    {
        LogMessage(MSG_JPR_005, {this->m_content_name, name});
        ret = RET_FAILURE;
    }
    return ret;
}

bool XJsonParser::GetNodeValueStr( const std::string & name, std::string & node_value )
{
    // Get value inside node
    auto ret = RET_SUCCESS;
    if (is_json_value() == 1)
    {
        auto node = get_json_value();
        if (json_node_get_value_type(node) == G_TYPE_STRING)
        {
            node_value = get_json_string_value();
        }
        else
        {
            LogMessage(MSG_JPR_004, {this->m_content_name, name, "string"});
            ret = RET_FAILURE;
        }
        ret |= CheckReaderError();
    }
    else
    {
        LogMessage(MSG_JPR_005, {this->m_content_name, name});
        ret = RET_FAILURE;
    }
    return ret;
}

bool XJsonParser::ExtractNodeValueBool( const std::vector<std::string> & node_title_in, bool & node_value )
{
    uint num_node_read = 0;
    auto ret = ExtractNode(node_title_in, num_node_read);
    if (ret == RET_SUCCESS)
    {
        auto full_node_title = StrVectToStr(node_title_in, ".");
        ret = GetNodeValueBool(full_node_title, node_value);
        if (ret == RET_FAILURE)
        {
            LogMessage(MSG_JPR_006, {this->m_content_name, "boolean", full_node_title});
        }
    }
    if (this->m_json_parsed)
    {
        for (uint ii = 0; ii < num_node_read; ii++)
        {
            end_json_element();
        }
    }
    return ret;
}

bool XJsonParser::ExtractNodeValueStr( const std::vector<std::string> & node_title_in, std::string & node_value )
{
    uint num_node_read = 0;
    auto ret = ExtractNode(node_title_in, num_node_read);
    if (ret == RET_SUCCESS)
    {
        auto full_node_title = StrVectToStr(node_title_in, ".");
        ret = GetNodeValueStr(full_node_title, node_value);
        if (ret == RET_FAILURE)
        {
            LogMessage(MSG_JPR_006, {this->m_content_name, "string", full_node_title});
        }
    }
    if (this->m_json_parsed)
    {
        for (uint ii = 0; ii < num_node_read; ii++)
        {
            end_json_element();
        }
    }
    return ret;
}

bool XJsonParser::ExtractNodeArrayStr( const std::vector<std::string> & node_title_in, std::vector<std::string> & node_array_value )
{
    node_array_value.clear();
    uint num_node_read = 0;
    auto ret = ExtractNode(node_title_in, num_node_read);
    if (ret == RET_SUCCESS)
    {
        if (is_json_array() == 1)
        {
            for (gint j = 0; j < count_json_elements(); j++)
            {
                read_json_element(j);
                std::string node_value;
                auto full_node_title = StrVectToStr(node_title_in, ".") + "[" + std::to_string(j) + "]";
                ret = GetNodeValueStr(full_node_title, node_value);
                if (ret == RET_FAILURE)
                {
                    break;
                }
                node_array_value.emplace_back(node_value);
                end_json_element();
            }
        }
        else
        {
            ret = RET_FAILURE;
        }
    }
    if (this->m_json_parsed)
    {
        for (uint ii = 0; ii < num_node_read; ii++ )
        {
            end_json_element();
        }
    }
    return ret;
}

JsonNodeType XJsonParser::GetJsonNodeType()
{
    auto node_type = JSON_NODE_NULL;
    if (is_json_object() == 1)
    {
        node_type = JSON_NODE_OBJECT;
    }
    else if (is_json_array() == 1)
    {
        node_type = JSON_NODE_ARRAY;
    }
    else if (is_json_value() == 1)
    {
        node_type = JSON_NODE_VALUE;
    }
    return node_type;
}

bool XJsonParser::ListNodeMembers( const std::string & node_title_in, gchar *** member_list )
{
    *member_list = list_json_members();
    if (member_list == nullptr)
    {
        end_json_member();
        LogMessage(MSG_JPR_009, {this->m_content_name, node_title_in});
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

bool XJsonParser::ListNodeMembers( const std::vector<std::string> & node_title_in, std::vector<std::string> & member_list )
{
    member_list.clear();
    gchar **g_member_list;
    uint num_node_read = 0;
    ExtractNode(node_title_in, num_node_read); // Move cursor to testcases array
    auto node_name_in = StrVectToStr(node_title_in, ".");
    auto ret = ListNodeMembers(node_name_in, &g_member_list);
    if (ret == RET_SUCCESS)
    {
        if (count_json_members() > 0)
        {
            for (int j = 0; g_member_list[j] != nullptr; j++)
            {
                member_list.emplace_back(std::string(g_member_list[j]));
            }
        }
    }
    g_strfreev (g_member_list);
    if (this->m_json_parsed)
    {
        for (uint ii = 0; ii < num_node_read; ii++) // Move cursor back from testcases array
        {
            end_json_element();
        }
    }
    return ret;
}

bool XJsonParser::CheckMembers( const Json_Definition_t & json_definition )
{
    return CheckMembers({}, json_definition, true);
}

bool XJsonParser::CheckMembers( const std::vector<std::string> & base_node_title, const Json_Definition_t & json_definition, const bool & next_level )
{
    auto ret = RET_SUCCESS;
    gchar **member_list;
    auto base_node_name = StrVectToStr(base_node_title, ".");
    if (ListNodeMembers(base_node_name, &member_list) == RET_FAILURE)
    {
        ret = RET_FAILURE;
    }

    if (count_json_members() > 0)
    {
        for (int j = 0; (member_list[j] != nullptr) && (ret == RET_SUCCESS); j++)
        {
            // Check that nodes in first level title+type are defined
            std::string node_name(member_list[j]);
            if (StrMatchNoCase(node_name, "comment"))
            {
                continue;
            }

            std::vector<std::string> node_title = {node_name};

            LogMessage(MSG_DEBUG_PARSING, {"Checking node: " + StrVectToStr(node_title, ".")});
            if (ReadMemberNoCase(node_name) == RET_FAILURE)
            {
                ret = RET_FAILURE;
            }
            JsonNodeType node_type;
            if (ret == RET_SUCCESS)
            {
                node_type = GetJsonNodeType();
                if (CheckMemberDefinition(base_node_title, json_definition, node_title, node_type) == RET_FAILURE)
                {
                    ret = RET_FAILURE;
                }
                else if ((node_type == JSON_NODE_OBJECT) && (next_level))
                {
                    if (CheckMembersNextLevel(base_node_title, json_definition, node_title) == RET_FAILURE)
                    {
                        ret = RET_FAILURE;
                    }
                }
            }
            end_json_member();
        }
    }
    g_strfreev (member_list);
    return ret;
}

bool XJsonParser::CheckMembersNextLevel( const std::vector<std::string> & base_node_title, const Json_Definition_t & json_definition, const std::vector<std::string> & node_title_in )
{
    auto ret = RET_SUCCESS;
    gchar **member_list;
    auto full_node_title = base_node_title;
    full_node_title.insert(full_node_title.end(), node_title_in.begin(), node_title_in.end());
    auto full_node_name = StrVectToStr(full_node_title, ".");
    if (ListNodeMembers(full_node_name, &member_list) == RET_FAILURE)
    {
        ret = RET_FAILURE;
    }

    if (count_json_members() > 0)
    {
        for (int j = 0; (member_list[j] != nullptr) && (ret == RET_SUCCESS); j++)
        {
            // Recursively check that all nodes title+type are defined
            std::string node_name(member_list[j]);
            if (StrMatchNoCase(node_name, "comment"))
            {
                continue;
            }

            auto node_title = node_title_in;
            node_title.emplace_back(node_name);
            full_node_title.emplace_back(node_name);

            LogMessage(MSG_DEBUG_PARSING, {"Checking node: " + StrVectToStr(full_node_title, ".")});
            if (ReadMemberNoCase(node_name) == RET_FAILURE)
            {
                ret = RET_FAILURE;
            }
            JsonNodeType node_type;
            if (ret == RET_SUCCESS)
            {
                node_type = GetJsonNodeType();
                if (CheckMemberDefinition(base_node_title, json_definition, node_title, node_type) == RET_FAILURE)
                {
                    ret = RET_FAILURE;
                }
                if (node_type == JSON_NODE_OBJECT)
                {
                    if (CheckMembersNextLevel(base_node_title, json_definition, node_title) == RET_FAILURE)
                    {
                        ret = RET_FAILURE;
                    }
                }
            }
            end_json_member();
        }
    }
    g_strfreev (member_list);
    return ret;
}

bool XJsonParser::CheckMemberDefinition( const std::vector<std::string> & base_node_title, const Json_Definition_t & json_definition, const std::vector<std::string> & node_title_in, const JsonNodeType & node_type_in )
{
    auto ret = RET_SUCCESS;
    auto name_correct = false;
    auto type_correct = false;
    auto expected_type = JSON_NODE_NULL;
    auto full_node_title = base_node_title;
    full_node_title.insert(full_node_title.end(), node_title_in.begin(), node_title_in.end());

    for (const auto & def : json_definition)
    {
        name_correct = false;
        type_correct = false;
        if (def.second == node_type_in)
        {
            type_correct = true;
        }
        if (StrMatchNoCase(StrVectToStr(def.first, "."), StrVectToStr(node_title_in, ".")))
        {
            name_correct = true;
            expected_type = def.second;
            break;
        }
    }
    if (name_correct)
    {
        if (!type_correct)
        {
            ret = RET_FAILURE;
            LogMessage(MSG_JPR_012, {this->m_content_name, JsonNodeTypeToString(node_type_in), StrVectToStr(full_node_title, "."), JsonNodeTypeToString(expected_type)});
        }
    }
    else
    {
        ret = RET_FAILURE;
        LogMessage(MSG_JPR_011, {this->m_content_name, StrVectToStr(full_node_title, ".")});
    }

    return ret;
}

bool XJsonParser::GetJsonNodeValueType( const std::string & name, GType & node_value_type )
{
    // Get value inside node
    auto ret = RET_SUCCESS;
    if (is_json_value() == 1)
    {
        auto node = get_json_value();
        node_value_type = json_node_get_value_type(node);
        ret |= CheckReaderError();
    }
    else
    {
        LogMessage(MSG_JPR_005, {this->m_content_name, name});
        ret = RET_FAILURE;
    }
    return ret;
}

const GError *  XJsonParser::get_json_error()                               { return json_reader_get_error          (this->m_json_reader);              }
void            XJsonParser::set_json_root( JsonNode * root )               {        json_reader_set_root           (this->m_json_reader, root);        }
gchar**         XJsonParser::list_json_members()                            { return json_reader_list_members       (this->m_json_reader);              }
gint            XJsonParser::count_json_members()                           { return json_reader_count_members      (this->m_json_reader);              }
gboolean        XJsonParser::read_json_member( const gchar * member_name )  { return json_reader_read_member        (this->m_json_reader, member_name); }
void            XJsonParser::end_json_member()                              {        json_reader_end_member         (this->m_json_reader);              }
gboolean        XJsonParser::read_json_element( guint index )               { return json_reader_read_element       (this->m_json_reader, index);       }
void            XJsonParser::end_json_element()                             {        json_reader_end_element        (this->m_json_reader);              }
gboolean        XJsonParser::is_json_value()                                { return json_reader_is_value           (this->m_json_reader);              }
gboolean        XJsonParser::is_json_object()                               { return json_reader_is_object          (this->m_json_reader);              }
gboolean        XJsonParser::is_json_array()                                { return json_reader_is_array           (this->m_json_reader);              }
JsonNode *      XJsonParser::get_json_value()                               { return json_reader_get_value          (this->m_json_reader);              }
gint64          XJsonParser::get_json_int_value()                           { return json_reader_get_int_value      (this->m_json_reader);              }
const gchar *   XJsonParser::get_json_string_value()                        { return json_reader_get_string_value   (this->m_json_reader);              }
gboolean        XJsonParser::get_json_boolean_value()                       { return json_reader_get_boolean_value  (this->m_json_reader);              }
gint            XJsonParser::count_json_elements()                          { return json_reader_count_elements     (this->m_json_reader);              }

} // namespace
