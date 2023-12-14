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

#include <json-glib/json-glib.h>
#include <fstream>

#include "messagesparser.h"

namespace xbtest
{

#define PARSE_MESSAGE(message) do { \
    if (ret == RET_SUCCESS) \
    { \
        ret = ParseMessage(message); \
    } \
} while(false); \

MessagesParser::MessagesParser( Logging * log, std::atomic<bool> * abort )
{
    this->m_log             = log;
    this->m_abort           = abort;
}

MessagesParser::~MessagesParser() = default;

bool MessagesParser::ParseMessages()
{
    auto ret = RET_SUCCESS;
    PARSE_MESSAGES(); // Central message definition to variables
    if (ret == RET_FAILURE)
    {
        LogMessage(MSG_DEF_001);
    }
    return ret;
}

bool MessagesParser::ParseMessage( const Message_t & message )
{
    // Check id
    if (message.id.empty())
    {
        LogMessage(MSG_DEF_004, {"id"});
        return RET_FAILURE;
    }
    // Check message ID not already defined
    if (this->m_messages.find(message.id) != this->m_messages.end())
    {
        LogMessage(MSG_DEF_005, {message.id});
        return RET_FAILURE;
    }
    // Check state
    if (message.state.empty())
    {
        LogMessage(MSG_DEF_004, {"state"});
        return RET_FAILURE;
    }
    if (!FindInVect(message.state, MESSAGE_STATES))
    {
        LogMessage(MSG_DEF_007, {message.state});
        return RET_FAILURE;
    }

    if (message.state == USED) // Process further only messages that are used
    {
        // Check log_level
        if (!FindInVect(message.log_level, SUPPORTED_LOGLEVEL))
        {
            LogMessage(MSG_DEF_006, {message.log_level + " (ID = " + message.id + ")"});
            return RET_FAILURE;
        }
        // Check msg
        if (message.msg.empty())
        {
            LogMessage(MSG_DEF_004, {"msg (ID = " + message.id + ")"});
            return RET_FAILURE;
        }
        // Check detail
        if (message.detail.empty())
        {
            LogMessage(MSG_DEF_004, {"detail (ID = " + message.id + ")"});
            return RET_FAILURE;
        }

        // Check resolution depending on severity
        if (((message.log_level == LOG_STATUS) || (message.log_level == LOG_INFO) || (message.log_level == LOG_PASS)) && !message.resolution.empty())
        {
            LogMessage(MSG_DEF_008, {LogLevelToSeverity(message.log_level), message.id}); // No resolution for STATUS, INFO and PASS messages
            return RET_FAILURE;
        }
        if (((message.log_level == LOG_WARN) || (message.log_level == LOG_CRIT_WARN) || (message.log_level == LOG_ERROR) || (message.log_level == LOG_FAILURE)) && message.resolution.empty())
        {
            LogMessage(MSG_DEF_009, {LogLevelToSeverity(message.log_level), message.id}); // Resolution required for WARNING, CRIT_WARN, ERROR and FAILURE messages
            return RET_FAILURE;
        }
    }

    // Save message
    this->m_messages.emplace(message.id, message);

    return RET_SUCCESS;
}

bool MessagesParser::WriteMessagesJson( const std::string & file_name, const bool & force )
{
    GError * error = nullptr;

    auto msg_array = json_array_new();

    for (const auto & msg : this->m_messages)
    {
        auto msg_obj = json_object_new();
        json_object_set_string_member (msg_obj, "id", msg.second.id.c_str());
        if (msg.second.state != "obsolete")
        {
            json_object_set_string_member(msg_obj, "severity", LogLevelToSeverity(msg.second.log_level).c_str());
            json_object_set_string_member(msg_obj, "msg", msg.second.msg.c_str());
            if (!msg.second.msg_edit.empty())
            {
                json_object_set_string_member(msg_obj, "msg_edit", msg.second.msg_edit.c_str());
            }
            json_object_set_string_member(msg_obj, "detail", msg.second.detail.c_str());
            if (!msg.second.resolution.empty())
            {
                json_object_set_string_member(msg_obj, "resolution", msg.second.resolution.c_str());
            }
            json_object_set_boolean_member(msg_obj, "display_classic_console", static_cast<gboolean>(msg.second.display_classic_console));
            json_object_set_boolean_member(msg_obj, "display_dynamic_console", static_cast<gboolean>(msg.second.display_dynamic_console));
            json_object_set_int_member(msg_obj, "num_display_dynamic_console",  msg.second.num_display_dynamic_console);
        }
        json_object_set_string_member (msg_obj, "state", msg.second.state.c_str());
        json_array_add_object_element(msg_array, msg_obj);
    }

    auto root_json_node = json_node_new(JSON_NODE_ARRAY);
    json_node_take_array(root_json_node, msg_array);

    // Write message.json in log directory using the JsonGenerator
    auto my_json_generator = json_generator_new();
    json_generator_set_root(my_json_generator, root_json_node);
    json_generator_set_pretty(my_json_generator, 1);
    json_generator_set_indent_char(my_json_generator, ' ');
    json_generator_set_indent(my_json_generator, 2);

    LogMessage(MSG_CMN_020, {file_name});
    if (FileExists(file_name))
    {
        if (force)
        {
            LogMessage(MSG_CMN_019, {file_name}); // warning
        }
        else
        {
            LogMessage(MSG_CMN_053, {file_name});
            return RET_FAILURE;
        }
        if (!IsFileWritable(file_name))
        {
            LogMessage(MSG_GEN_080, {"write", file_name});
            return RET_FAILURE;
        }
    }

    // it looks like json_generator_to_file can write a file even if the file has not write flag
    auto ret = json_generator_to_file(my_json_generator, file_name.c_str(), &error);
    if ((error != nullptr) || (ret == 0))
    {
        LogMessage(MSG_CMN_018, {file_name});
        if (error != nullptr)
        {
            g_error_free(error);
        }
        g_object_unref(my_json_generator);
        return RET_FAILURE;
    }
    g_object_unref(my_json_generator);
    return RET_SUCCESS;
}

bool MessagesParser::GetMessage( const std::string & id, Message_t & message )
{
    auto it = this->m_messages.find(id);
    if (it != this->m_messages.end())
    {
        message = it->second;
        return true;
    }
    return false;
}

void MessagesParser::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, message, arg_list);
}
void MessagesParser::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, message);
}

} // namespace
