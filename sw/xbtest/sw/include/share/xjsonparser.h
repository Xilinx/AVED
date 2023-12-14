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

#ifndef _XJSONPARSER_H
#define _XJSONPARSER_H

#include "xbtestpackage.h"
#include "logging.h"

namespace xbtest
{

class XJsonParser
{

public:
    XJsonParser( Logging * log, std::atomic<bool> * abort );
    ~XJsonParser();

    std::string                         m_log_header;
    std::string                         m_content_name;
    std::map<std::string, LogLevel>     m_message_history;
    Logging *                           m_log = nullptr;
    std::atomic<bool> *                 m_abort;
    JsonParser *                        m_json_parser = nullptr;
    JsonNode   *                        m_json_root_node;
    JsonReader *                        m_json_reader = nullptr;
    bool                                m_json_parsed = false;
    bool                                m_default_report = true; // If false, then Message_t with log_level != LOG_FAILURE are displayed with m_level
    LogLevel                            m_level;

    void            LogMessage              ( const Message_t & message, const std::vector<std::string> & arg_list );
    void            LogMessage              ( const Message_t & message );
    void            UpdateMessageHistory    ( const Message_t & message );
    void            ClearParser();
    void            SetReportLevel          ( const LogLevel & level );
    bool            CheckReaderError();
    bool            ReadMemberNoCase        ( const std::string & node_title_in );
    bool            NodeExists              ( const std::vector<std::string> & node_title_in );
    bool            ExtractNode             ( const std::vector<std::string> & node_title_in, uint & num_node_read );
    bool            GetNodeValueBool        ( const std::string & name, bool & node_value );
    bool            GetNodeValueStr         ( const std::string & name, std::string & node_value );
    bool            ExtractNodeValueBool    ( const std::vector<std::string> & node_title_in, bool & node_value );
    bool            ExtractNodeValueStr     ( const std::vector<std::string> & node_title_in, std::string & node_value );
    bool            ExtractNodeArrayStr     ( const std::vector<std::string> & node_title_in, std::vector<std::string> & node_array_value );
    JsonNodeType    GetJsonNodeType();
    bool            ListNodeMembers         ( const std::string & node_title_in, gchar *** member_list );
    bool            ListNodeMembers         ( const std::vector<std::string> & node_title_in, std::vector<std::string> & member_list );
    bool            CheckMembers            ( const Json_Definition_t & json_definition );
    bool            CheckMembers            ( const std::vector<std::string> & base_node_title, const Json_Definition_t & json_definition, const bool & next_level );
    bool            CheckMembersNextLevel   ( const std::vector<std::string> & base_node_title, const Json_Definition_t & json_definition, const std::vector<std::string> & node_title_in );
    bool            CheckMemberDefinition   ( const std::vector<std::string> & base_node_title, const Json_Definition_t & json_definition, const std::vector<std::string> & node_title_in, const JsonNodeType & node_type_in );

    const GError *  get_json_error();
    void            set_json_root( JsonNode * root );
    gchar**         list_json_members();
    gint            count_json_members();
    gboolean        read_json_member( const gchar  *member_name );
    void            end_json_member();
    gboolean        read_json_element( guint index );
    void            end_json_element();
    gboolean        is_json_value();
    gboolean        is_json_object();
    gboolean        is_json_array();
    JsonNode *      get_json_value();
    gint64          get_json_int_value();
    const gchar *   get_json_string_value();
    gboolean        get_json_boolean_value();
    gint            count_json_elements();

    template<typename T> bool GetNodeValueInt( const std::string & name, T & node_value )
    {
        // Get value inside node
        auto ret = RET_SUCCESS;
        if (is_json_value() == 1)
        {
            auto node = get_json_value();
            if (json_node_get_value_type(node) == G_TYPE_INT64)
            {
                auto json_value = get_json_int_value();
                node_value = (T)(json_value);
                if (std::to_string(node_value) != std::to_string(json_value))
                {

                    LogMessage(MSG_JPR_007, {this->m_content_name, std::to_string(json_value), name, "integer"});
                    ret = RET_FAILURE;
                }
            }
            else
            {
                LogMessage(MSG_JPR_004, {this->m_content_name, name, "integer"});
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

    template<typename T> bool ExtractNodeValueInt( const std::vector<std::string> & node_title_in, T & node_value )
    {
        uint num_node_read = 0;
        auto ret = ExtractNode(node_title_in, num_node_read);
        if (ret == RET_SUCCESS)
        {
            auto full_node_title = StrVectToStr(node_title_in, ".");
            ret = GetNodeValueInt(full_node_title, node_value);
            if (ret == RET_FAILURE)
            {
                LogMessage(MSG_JPR_006, {this->m_content_name, "integer", full_node_title});
            }
        }
        for (uint ii = 0; ii < num_node_read; ii++)
        {
            end_json_element();
        }
        return ret;
    }

    template<typename T> bool GetNodeValueDouble( const std::string & name, T & node_value )
    {
        // Get value inside node
        auto ret = RET_SUCCESS;
        if (is_json_value() == 1)
        {
            auto node = get_json_value();
            if (json_node_get_value_type(node) == G_TYPE_DOUBLE)
            {
                auto json_value = json_reader_get_double_value(this->m_json_reader);
                node_value = (T)(json_value);
                if (std::to_string(node_value) != std::to_string(json_value))
                {
                    LogMessage(MSG_JPR_007, {this->m_content_name, std::to_string(json_value), name, "double"});
                    ret = RET_FAILURE;
                }
            }
            else
            {
                LogMessage(MSG_JPR_004, {this->m_content_name, name, "double"});
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

    template<typename T> bool ExtractNodeValueDouble( const std::vector<std::string> & node_title_in, T & node_value )
    {
        uint num_node_read = 0;
        auto ret = ExtractNode(node_title_in, num_node_read);
        if (ret == RET_SUCCESS)
        {
            auto full_node_title = StrVectToStr(node_title_in, ".");
            ret = GetNodeValueDouble(full_node_title, node_value);
            if (ret == RET_FAILURE)
            {
                LogMessage(MSG_JPR_006, {this->m_content_name, "double", full_node_title});
            }
        }

        for (uint ii = 0; ii < num_node_read; ii++)
        {
            end_json_element();
        }
        return ret;
    }

    bool GetJsonNodeValueType( const std::string & name, GType & node_value_type );

    virtual bool Parse() = 0;
};

} // namespace

#endif /* _XJSONPARSER_H */
