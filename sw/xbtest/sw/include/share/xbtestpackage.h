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

#ifndef _XBTESTPACKAGE_H
#define _XBTESTPACKAGE_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <unistd.h>
#include <cmath>
#include <map>
#include <set>
#include <json-glib/json-glib.h>
#include <fstream>
#include <glob.h>

#ifdef USE_AMI
    // AMI includes
    #include <ami_sensor.h>
#endif

#include "xbtestinternal.h"

namespace xbtest
{

#define XBTEST_SW           (uint)(1)
#define XBTEST_COMMON       (uint)(0)

// Check USE_NO_DRIVER / USE_AMI and USE_XRT definition
#if defined(USE_AMI) && defined(USE_XRT)
#  error "USE_AMI and USE_XRT cannot be both defined"
#endif
#if defined(USE_AMI) && defined(USE_NO_DRIVER)
#  error "USE_AMI and USE_NO_DRIVER cannot be both defined"
#endif
#if defined(USE_XRT) && defined(USE_NO_DRIVER)
#  error "USE_NO_DRIVER and USE_XRT cannot be both defined"
#endif

#if !defined(USE_NO_DRIVER) && !defined(USE_AMI) && !defined(USE_XRT)
#  error "USE_NO_DRIVER or USE_AMI or USE_XRT must be defined"
#endif

#define RET_FAILURE         true
#define RET_SUCCESS         false

inline bool IsAdmin()
{
  return (getuid() == 0) || (geteuid() == 0);
}

// any negative value is an abort code, any positive value is an error code
#define TEST_STATE_PASS     (int)(0)
#define TEST_STATE_ABORT    (int)(-1)
#define TEST_STATE_FAILURE  (int)(1)

#define XBTEST_METADATA     std::string("xbtest metadata")
#define CONFIG_JSON         std::string("xbtest package configuration JSON file")

#define NOT_APPLICABLE      std::string("n/a")
#define OVR                 std::string("OVR")

// Command line parameters names
#define ARGS                        std::string("arguments")
#define UNKNOWN_OPTION              std::string("unknown option")

#define HELP                        std::string("help")
#define VERSION                     std::string("version")
#define MSG_ID                      std::string("message ID")
#define GUIDE                       std::string("guide")

#define DEVICE_BDF                  std::string("card BDF")
#define P2P_TARGET_BDF              std::string("P2P target card BDF")
#define P2P_NVME_PATH               std::string("P2P NVME path")
#define PRE_CANNED                  std::string("pre-canned test")
#define TEST_JSON                   std::string("test JSON file")
#define LOG_DIR                     std::string("log directory")
#define DISABLE_LOG                 std::string("disable logging")
#define FORCE                       std::string("force")
#define DESIGN_PDI                  std::string("design PDI")
#define CONSOLE_DETAILED_FORMAT     std::string("classic display mode")

#define XBTEST_PFM_DEF              std::string("card definition JSON file")
#define P2P_XBTEST_PFM_DEF          std::string("card definition JSON file of P2P target BDF")
#define P2P_DESIGN_PDI              std::string("design PDI of P2P target BDF")
#define XCLBIN                      std::string("xclbin")
#define P2P_XCLBIN                  std::string("xclbin of P2P target BDF")
#define VERBOSITY                   std::string("verbosity")

#define COMMAND_LINE_EXIT_SUCCESS   (uint)(0)
#define COMMAND_LINE_RET_SUCCESS    (uint)(1)
#define COMMAND_LINE_RET_FAILURE    (uint)(2)

using Xbtest_CL_option_Str_t = struct Xbtest_CL_option_Str_t {
    bool        exists;
    std::string value;
};
using Xbtest_CL_option_Uint_t = struct Xbtest_CL_option_Uint_t {
    bool        exists;
    uint        value;
};
using Xbtest_CL_option_Int_t = struct Xbtest_CL_option_Int_t {
    bool        exists;
    int         value;
};

using Xbtest_System_t = struct Xbtest_System_t {
    std::string start_time;
    std::string start_time2;
    std::string end_time;
    std::string username;
    std::string sysname;
    std::string nodename;
    std::string release;
    std::string version;
    std::string machine;
};

// Parameter required or not (error message when not found in JSON)
using Required_t = enum Required_t {
    REQUIRED_TRUE,
    REQUIRED_FALSE
};
// Parameter hidden (no message displayed) or visible
using Hidden_t = enum Hidden_t {
    HIDDEN_TRUE,
    HIDDEN_FALSE
};
// Type IDs of JSON parameter values.
// TYPE_ID_TEST_SEQUENCE is used for testcase TEST_SEQUENCE array of a structure different for each testcase
using Type_ID_t = enum Type_ID_t {
    TYPE_ID_INT,
    TYPE_ID_UINT,
    TYPE_ID_UINT64_T,
    TYPE_ID_FLOAT,
    TYPE_ID_DOUBLE,
    TYPE_ID_BOOL,
    TYPE_ID_STRING,
    TYPE_ID_TEST_SEQUENCE,
    TYPE_ID_OBJECT,
    TYPE_ID_NOT_APPLICABLE
};
// Structure defining a JSON parameter value
struct Json_Val_Def_t {
    Required_t      required;
    Hidden_t        hidden;
    JsonNodeType    node_type;
    Type_ID_t       typeId;
    std::string     name;
    std::string     description;
};
using Json_Params_Def_t = std::vector<Json_Val_Def_t>;
#define DESCRIPTION_NOT_SET std::string("No description set")

// Type used to check JSON parameter definition (node type, not node value type!)
using Definition_t      = std::pair<std::vector<std::string>, JsonNodeType>;
using Json_Definition_t = std::map<std::vector<std::string>, JsonNodeType>;

#define TIMESTAMP_MODE_NONE         std::string("none")
#define TIMESTAMP_MODE_ABSOLUTE     std::string("absolute")
#define TIMESTAMP_MODE_DIFFERENTIAL std::string("differential")
#define SUPPORTED_TIMESTAMP_MODE    std::set<std::string>({ \
    TIMESTAMP_MODE_NONE, \
    TIMESTAMP_MODE_ABSOLUTE, \
    TIMESTAMP_MODE_DIFFERENTIAL \
}) \

inline std::string JsonNodeTypeToString( const JsonNodeType & node_type )
{
    if (node_type == JSON_NODE_OBJECT)
    {
        return "object";
    }
    if (node_type == JSON_NODE_ARRAY)
    {
        return "array";
    }
    if (node_type == JSON_NODE_VALUE)
    {
        return "value";
    }
    if (node_type == JSON_NODE_NULL)
    {
        return "null";
    }
    return "UNKNOWN";
}


#define PAD_ON_RIGHT false
#define PAD_ON_LEFT  true

inline std::string pad( const std::string & input_string, const char & pad_char, const uint & max_pad_size, const bool & left_n_right )
{
    std::stringstream ss;
    ss << std::setfill(pad_char);
    ss << std::setw(max_pad_size);
    if (left_n_right == PAD_ON_LEFT) // Pad character to be inserted on the left (right fill)
    {
        ss << std::right;
    }
    else // Pad character to be inserted on the right (Left fill)
    {
        ss << std::left;
    }
    ss << input_string;
    return ss.str();
}

inline bool StrMatchNoCase( const std::string & str1_in, const std::string & str2_in )
{
    auto str1 = str1_in;
    auto str2 = str2_in;
    std::transform(str1.begin(), str1.end(), str1.begin(), tolower);
    std::transform(str2.begin(), str2.end(), str2.begin(), tolower);
    return (str1 == str2);
}

inline bool StrMatchNoCase( const std::string & str1_in, const int & begin, const int & size, const std::string & str2_in )
{
    auto str1 = str1_in;
    auto str2 = str2_in;
    // Compares "size" characters from "begin" index of str1 with string str2
    std::transform(str1.begin(), str1.end(), str1.begin(), tolower);
    std::transform(str2.begin(), str2.end(), str2.begin(), tolower);
    return (str1.compare(begin, size,  str2) == 0);
}

template<typename T> inline bool ConvString2Num( const std::string & str, T & value )
{
    std::stringstream ss;
    ss << str;
    ss >> value;
    if (ss.fail())
    {
        value = 0;
        return RET_FAILURE; // non-numeric string
    }
    return RET_SUCCESS; // string to numeric conversion is successful
}

template<typename T> inline bool ConvStringHex2Num( const std::string & str, T & value )
{
    std::stringstream ss;
    ss << str;
    ss >> std::hex >> value;
    if (ss.fail())
    {
        value = 0;
        return RET_FAILURE; // non-numeric string
    }
    return RET_SUCCESS; // string to numeric conversion is successful
}

template<typename T> inline std::string NumToStrHex( const T & value, const int hex_size = -1 )
{
    std::stringstream hex_sstream;
    hex_sstream << std::hex << value;
    auto hex_str = hex_sstream.str();
    if (hex_size > 0)
    {
        hex_str = pad(hex_str, '0', hex_size, PAD_ON_LEFT);
    }
    return hex_str;
}

inline bool FindStringInSet ( const std::string & value_in, const std::set<std::string> & test_set )
{
    std::set<std::string> test_set_lowercase;
    for (auto test : test_set)
    {
        std::transform(test.begin(), test.end(), test.begin(), tolower);
        test_set_lowercase.insert(test);
    }

    auto value = value_in;
    std::transform(value.begin(), value.end(), value.begin(), tolower);

    return (test_set_lowercase.find(value) != test_set_lowercase.end());
}

template<typename T> inline bool FindInVect ( const T & value, const std::vector<T> & test_vect )
{
    return (std::find(test_vect.begin(), test_vect.end(), value) != test_vect.end());
}

inline bool FindStringInVect ( const std::string & value_in, const std::vector<std::string> & test_vect )
{
    std::vector<std::string> test_vect_lowercase;
    test_vect_lowercase.reserve(test_vect.size());
    for (auto test : test_vect)
    {
        std::transform(test.begin(), test.end(), test.begin(), tolower);
        test_vect_lowercase.emplace_back(test);
    }

    auto value = value_in;
    std::transform(value.begin(), value.end(), value.begin(), tolower);

    return FindInVect(value, test_vect_lowercase);
}

inline std::vector<std::string> QuoteStrVect( const std::vector<std::string> & str_vect_in )
{
    std::vector<std::string> quoted_str;
    quoted_str.reserve(str_vect_in.size());
    for (const auto & str : str_vect_in)
    {
        quoted_str.emplace_back("\"" + str + "\"");
    }
    return quoted_str;
}

inline std::set<std::string> QuoteStrSet( const std::set<std::string> & str_set_in )
{
    std::set<std::string> quoted_str;
    for (const auto & str : str_set_in)
    {
        quoted_str.insert("\"" + str + "\"");
    }
    return quoted_str;
}

inline std::string StrVectToStr( const std::vector<std::string> & str_vect, const std::string & delimiter )
{
    auto it = str_vect.begin();
    std::string str;
    if (it != str_vect.end())
    {
        str = *it++;
        for (; it != str_vect.end(); it++)
        {
            str += delimiter + *it;
        }
    }
    return str;
}

inline std::string StrVectToTest( const std::vector<std::string> & str_vect )
{
    return "{" + StrVectToStr(str_vect, ", ") + "}";
}

inline std::string StrSetToStr( const std::set<std::string> & str_set, const std::string & delimiter )
{
    auto it = str_set.begin();
    std::string str;
    if (it != str_set.end())
    {
        str = *it++;
        for (; it != str_set.end(); it++)
        {
            str += delimiter + *it;
        }
    }
    return str;
}

template<typename T> inline std::string NumVectToStr( const std::vector<T> & num_vect, const std::string & delimiter )
{
    auto it = num_vect.begin();
    std::string str;
    if (it != num_vect.end())
    {
        str = std::to_string(*it++);
        for (; it != num_vect.end(); it++)
        {
            str += delimiter + std::to_string(*it);
        }
    }
    return str;
}

#define BOOL_TRUE_STR     std::string("true")
#define BOOL_FALSE_STR    std::string("false")

inline std::string BoolToStr( const bool & bool_in )
{
    if (bool_in)
    {
        return BOOL_TRUE_STR;
    }
    return BOOL_FALSE_STR;
}

inline bool StrToBool( const std::string & bool_in, bool & bool_out )
{
    if (StrMatchNoCase(bool_in, BOOL_TRUE_STR))
    {
        bool_out = true;
        return RET_SUCCESS;
    }
    if (StrMatchNoCase(bool_in, BOOL_FALSE_STR))
    {
        bool_out = false;
        return RET_SUCCESS;
    }
    return RET_FAILURE;
}

template<typename T> inline std::string Float_to_String( const T & num, const int & precision )
{
    auto prec = precision;
    if (precision < 0)
    {
        prec = 0;
    }
    std::stringstream stream;
    stream << std::fixed << std::setprecision(prec) << num;
    return stream.str();
}

template<typename T> inline std::string Float_to_Scientific_String( const T & num, const int & precision )
{
    auto prec = precision;
    if (precision < 0)
    {
        prec = 0;
    }
    std::stringstream stream;
    stream << std::scientific << std::setprecision(prec) << num;
    return stream.str();
}

template<typename T> inline std::string Float_to_String_Ceil( const T & num, const int & precision )
{
    auto tmp = (double)(num);
    if (precision > 0)
    {
        auto a = (double)(std::pow(10,precision));
        tmp *= a;
        tmp = std::ceil(tmp);
        tmp /= a;
    }
    return Float_to_String<T>(tmp,precision);
}

inline uint64_t GetTimestamp()
{
    struct timespec sample;
    clock_gettime(CLOCK_MONOTONIC, &sample);
    auto secs       = (uint64_t)(sample.tv_sec)  * (uint64_t)(1000000); // conv sec to us
    auto u_secs     = (uint64_t)(sample.tv_nsec) / (uint64_t)(1000);    // conv ns to us
    return (secs + u_secs);
}

using LogLevel = int;

const LogLevel LOG_FAILURE          = 6;
const LogLevel LOG_ERROR            = 5;
const LogLevel LOG_PASS             = 4;
const LogLevel LOG_CRIT_WARN        = 3;
const LogLevel LOG_WARN             = 2;
const LogLevel LOG_INFO             = 1;
const LogLevel LOG_STATUS           = 0;
const LogLevel LOG_DEBUG_EXCEPTION  = -1;
const LogLevel LOG_DEBUG_TESTCASE   = -1;
const LogLevel LOG_DEBUG_SETUP      = -2;
const LogLevel LOG_DEBUG_OPENCL     = -3;
const LogLevel LOG_DEBUG_PARSING    = -4;

#define SUPPORTED_LOGLEVEL std::vector<LogLevel>({ \
    LOG_FAILURE, \
    LOG_ERROR, \
    LOG_PASS, \
    LOG_CRIT_WARN, \
    LOG_WARN, \
    LOG_INFO, \
    LOG_STATUS, \
    LOG_DEBUG_EXCEPTION, \
    LOG_DEBUG_TESTCASE, \
    LOG_DEBUG_SETUP, \
    LOG_DEBUG_OPENCL, \
    LOG_DEBUG_PARSING \
}) \


#define MIN_VERBOSITY LOG_DEBUG_PARSING
#define MAX_VERBOSITY LOG_STATUS

// Line HEADER
#define LOG_HEADER_GENERAL          std::string("GENERAL     ")
#define LOG_HEADER_MSG_PARSER       std::string("MSG_PARSER  ")
#define LOG_HEADER_LOGGING          std::string("LOGGING     ")
#define LOG_HEADER_DEVICE_INFO      std::string("DEVICE INFO ")
#define LOG_HEADER_COMMAND_LINE     std::string("COMMAND_LINE")
#define LOG_HEADER_TIMER            std::string("TIMER       ")
#define LOG_HEADER_CONSOLE_MGMT     std::string("CONSOLE_MGMT")

#define LOG_COL_DELIMITER           std::string(" :: ")

#define LOG_LINE_DELIMITER          std::string("#####################################################################################################")
#define LOG_LINE_COMMON             std::string("########################################## COMMON SUMMARY ###########################################")
#define LOG_LINE_SUMMARY            std::string("############################################## SUMMARY ##############################################")
#define LOG_LINE_DONE               std::string("############################################### DONE ################################################")

// Messages definition
using Message_t = struct Message_t {
    std::string                 id;
    std::string                 state;
    LogLevel                    log_level                   = LOG_DEBUG_EXCEPTION;
    std::string                 msg                         = "";
    std::string                 msg_edit                    = "";
    std::string                 detail                      = "";
    std::string                 resolution                  = "";
    bool                        display_classic_console     = true;
    bool                        display_dynamic_console     = false;
    int                         num_display_dynamic_console = -1;
    std::string                 severity                    = "";           // set by logging
    std::string                 header                      = "";           // set by logging
    std::string                 header_2                    = "";           // set by logging
    std::string                 header_3                    = "";           // set by logging
    std::vector<std::string>    arg_list                    = {};           // set by logging
    uint64_t                    timestamp_abs               = 0;            // set by logging
    uint64_t                    timestamp_diff              = 0;            // set by logging
    std::string                 timestamp_str               = "";           // set by logging
    std::string                 content                     = "";           // set by logging
    std::string                 log_msg                     = "";           // set by logging
};

using Testcase_Queue_Value_t = struct Testcase_Queue_Value_t {
    uint        pending;
    uint        completed;
    uint        passed;
    uint        failed;
    uint        errors;
    uint        warnings;
    std::string remaining_time; // use string as this can be n/a
    std::string parameters;
};
#define RESET_TESTCASE_QUEUE_VALUE  Testcase_Queue_Value_t({ \
    .pending         = 0, \
    .completed       = 0, \
    .passed          = 0, \
    .failed          = 0, \
    .errors          = 0, \
    .warnings        = 0, \
    .remaining_time  = NOT_APPLICABLE, \
    .parameters      = NOT_APPLICABLE \
}) \

using Card_Test_Queue_Value_t = struct Card_Test_Queue_Value_t {
    uint        pending;
    uint        completed;
    uint        passed;
    uint        failed;
    std::string elapsed_time; // use string as this can be n/a
    std::string test;
};
#define RESET_CARD_TEST_QUEUE_VALUE Card_Test_Queue_Value_t({ \
    .pending        = 0, \
    .completed      = 0, \
    .passed         = 0, \
    .failed         = 0, \
    .elapsed_time   = NOT_APPLICABLE, \
    .test           = NOT_APPLICABLE \
}) \

using Sensor_Queue_Value_t = struct Sensor_Queue_Value_t {
    bool        dump_valid;
    uint64_t    dump_cnt;
    double      temperature;
    double      power;
    bool        temperature_found;
    bool        power_found;
};

#define RESET_SENSOR_QUEUE_VALUE    Sensor_Queue_Value_t({ \
    .dump_valid          = false, \
    .dump_cnt            = 0, \
    .temperature         = 0.0, \
    .power               = 0.0, \
    .temperature_found   = false, \
    .power_found         = false \
}) \

using Message_Stat_Queue_Value_t = struct Message_Stat_Queue_Value_t {
    std::map<LogLevel, uint64_t> msg_count;
};

inline std::string LogLevelToSeverity( const LogLevel & level )
{
    switch (level)
    {
        case LOG_PASS:              return "PASS";       break;
        case LOG_FAILURE:           return "FAILURE";    break;
        case LOG_ERROR:             return "ERROR";      break;
        case LOG_CRIT_WARN:         return "CRIT_WARN";  break;
        case LOG_WARN:              return "WARNING";    break;
        case LOG_INFO:              return "INFO";       break;
        case LOG_STATUS:            return "STATUS";     break;
        default:                    return "DEBUG";      break; // LOG_DEBUG*
    }
}

inline std::vector<std::string> split( const std::string & input_string, const char & delimiter )
{
    std::vector<std::string> string_list;
    size_t current_pos = 0;
    auto next_pos = input_string.find(delimiter);
    while (next_pos != std::string::npos)
    {
        auto current_substring = input_string.substr(current_pos, next_pos-current_pos);
        string_list.emplace_back(current_substring);
        current_pos = next_pos + 1;

        next_pos = input_string.find( delimiter, current_pos );
    }
    string_list.emplace_back(input_string.substr( current_pos, std::min(next_pos, input_string.size()) - current_pos + 1 ));

    return string_list;
}

inline std::vector<std::string> SplitNoEmpty( const std::string & input_string, const char & delimiter )
{
    auto string_list = split(input_string, delimiter);
    auto it = string_list.begin();
    // Remove all empty strings from vector
    while (it != string_list.end())
    {
        if (it->empty()) {
            it = string_list.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return string_list;
}

inline bool FileExists( const std::string & filename )
{
    return (access(filename.c_str(), F_OK) == 0);
}

inline bool IsFileWritable( const std::string & filename )
{
    return (access(filename.c_str(), W_OK) == 0);
}

inline bool IsFileReadable( const std::string & filename )
{
    return (access(filename.c_str(), R_OK) == 0);
}

inline bool OpenFile( const std::string & filename, std::ofstream & ofs )
{
    ofs.open(filename, std::ofstream::out);
    auto created = !ofs.fail();
    return created;
}

inline bool CheckCardBDF( const std::string & bdf, int & dom, int & b, int & d, int & f )
{
    dom = -1;
    b = -1;
    d = -1;
    f = -1;

    auto n = std::count(bdf.begin(), bdf.end(), ':');
    if ((n != 1) && (n != 2))
    {
        return RET_FAILURE;
    }

    std::string b_str;
    std::string df_str;
    auto dom_bdf_split = split(bdf, ':');

    if (n == 1)
    {
        b_str  = dom_bdf_split[0];
        df_str = dom_bdf_split[1];
    }
    else if (n == 2)
    {
        auto dom_str = dom_bdf_split[0];
        b_str        = dom_bdf_split[1];
        df_str       = dom_bdf_split[2];
        if (ConvStringHex2Num<int>(dom_str, dom) == RET_FAILURE)
        {
            return RET_FAILURE;
        }
    }
    else
    {
        return RET_FAILURE;
    }

    if (ConvStringHex2Num<int>(b_str, b) == RET_FAILURE)
    {
        return RET_FAILURE;
    }

    n = std::count(df_str.begin(), df_str.end(), '.');
    if (n != 1)
    {
        return RET_FAILURE;
    }

    auto df_split = split(df_str, '.');
    auto d_str    = df_split[0];
    auto f_str    = df_split[1];

    if (ConvStringHex2Num<int>(d_str, d) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    if (ConvStringHex2Num<int>(f_str, f) == RET_FAILURE)
    {
        return RET_FAILURE;
    }
    return RET_SUCCESS;
}

inline bool CheckCardBDF( const std::string & bdf )
{
    int dom;
    int b;
    int d;
    int f;
    return CheckCardBDF(bdf, dom, b, d, f);
}

inline bool CompareBDF( const std::string & bdf1, const std::string & bdf2 )
{
    auto n1 = std::count(bdf1.begin(), bdf1.end(), ':');
    auto n2 = std::count(bdf2.begin(), bdf2.end(), ':');

    std::string dom1_str;
    std::string bdf1_str;
    std::string dom2_str;
    std::string bdf2_str;
    auto dom_bdf1_split = split(bdf1, ':');
    auto dom_bdf2_split = split(bdf2, ':');

    if (n1 == 1)
    {
        bdf1_str = dom_bdf1_split[0] + ":" + dom_bdf1_split[1];
    }
    else // n1 == 2
    {
        dom1_str = dom_bdf1_split[0];
        bdf1_str = dom_bdf1_split[1] + ":" + dom_bdf1_split[2];
    }
    if (n2 == 1)
    {
        bdf2_str = dom_bdf2_split[0] + ":" + dom_bdf2_split[1];
    }
    else // n1 == 2
    {
        dom2_str = dom_bdf2_split[0];
        bdf2_str = dom_bdf2_split[1] + ":" + dom_bdf2_split[2];
    }

    auto ret = StrMatchNoCase(bdf1_str, bdf2_str);
    if ((n1 == 2) && (n2 == 2))
    {
        ret &= StrMatchNoCase(dom1_str, dom2_str);
    }
    return ret;
}

inline std::string ExtendBDF( const std::string & bdf )
{
    if (std::count(bdf.begin(), bdf.end(), ':') == 1)
    {
        return "0000:" + bdf;
    }
    return bdf;
}

inline std::string FormatTime( const std::time_t & now, const std::string & format )
{
    char tmp_arr[100];
    std::strftime(tmp_arr, sizeof(tmp_arr), format.c_str(), std::localtime(&now));
    std::string tmp_s(tmp_arr);
    return tmp_s;
}

inline std::string GetCurrentTime()
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return FormatTime(now, "%a %b %d %H:%M:%S %Y %Z"); // Sun Jul 26 17:26:07 2020 BST
}

inline std::string FormatName ( const std::string & name, const std::string & rep )
{
    auto format_name = name;
    // Use lower cases
    std::transform(format_name.begin(), format_name.end(), format_name.begin(), tolower);
    // Replace special characters with provided character
    std::string rep_chars = "{}*-+.<>~@,; :";
    for(const char & c : rep_chars)
    {
        std::string rep_char(c,1);
        auto pos = format_name.find(rep_char);
        while (pos != std::string::npos)
        {
            format_name.replace(pos, rep_char.length(), rep);
            pos = format_name.find(rep_char);
        }
    }
    // Remove "rep" before
    auto valid = false;
    std::string format_name_no_pad;
    for (const auto & c : format_name)
    {
        if (std::string(c,1) != rep)
        {
            valid = true;
        }
        if (valid)
        {
            format_name_no_pad.insert(format_name_no_pad.end(), c);
        }
    }
    format_name = format_name_no_pad;

    // Remove "rep" after
    valid = false;
    format_name_no_pad = "";
    for (auto rit=format_name.rbegin(); rit!=format_name.rend(); ++rit)
    {
        if (std::string(*rit,1) != rep)
        {
            valid = true;
        }
        if (valid)
        {
            format_name_no_pad.insert(format_name_no_pad.begin(), *rit);
        }
    }
    format_name = format_name_no_pad;
    return format_name;
}

// device info
#define MAX_SENSORS                     (uint)(50)
 // device info fails => crit warn. if fails = 3 times consecutively, report error and next fail = debug. reset to crit warn if pass
#define API_FAIL_ERROR_LIMIT            (uint64_t)(3)
#define PERCENT_API_FAIL_ERROR_LIMIT    (double)(10.0)
#define PERCENT_API_FAIL_WARNING_LIMIT  (double)(0.0)
#define DEVICE_INFO_API_JSON            std::string("xrt::device::get_info API JSON output for card at ")

#define LOGIC_UUID          std::string("logic_uuid")

#define XRT_INFO_DEVICE_BDF                     std::string("bdf")
#define XRT_INFO_DEVICE_INTERFACE_UUID          std::string("interface_uuid")
#define XRT_INFO_DEVICE_KDMA                    std::string("kdma")
#define XRT_INFO_DEVICE_MAX_CLOCK_FREQUENCY_MHZ std::string("max_clock_frequency_mhz")
#define XRT_INFO_DEVICE_M2M                     std::string("m2m")
#define XRT_INFO_DEVICE_NAME                    std::string("name")
#define XRT_INFO_DEVICE_NODMA                   std::string("nodma")
#define XRT_INFO_DEVICE_OFFLINE                 std::string("offline")
#define XRT_INFO_DEVICE_ELECTRICAL              std::string("electrical")
#define XRT_INFO_DEVICE_THERMAL                 std::string("thermal")
#define XRT_INFO_DEVICE_MECHANICAL              std::string("mechanical")
#define XRT_INFO_DEVICE_MEMORY                  std::string("memory")
#define XRT_INFO_DEVICE_PLATFORM                std::string("design")
#define XRT_INFO_DEVICE_PCIE_INFO               std::string("pcie_info")
#define XRT_INFO_DEVICE_HOST                    std::string("host")
#define XRT_INFO_DEVICE_DYNAMIC_REGIONS         std::string("dynamic_regions")

#define SUPPORTED_SENSOR_TYPES  std::set<std::string>({ \
    XRT_INFO_DEVICE_ELECTRICAL, \
    XRT_INFO_DEVICE_THERMAL, \
    XRT_INFO_DEVICE_MECHANICAL \
}) \

// Default power rails sources
#ifdef USE_XRT
#define ID_12V_PEX                  std::string("12v_pex")
#define ID_12V_AUX                  std::string("12v_aux")
#define ID_3V3_PEX                  std::string("3v3_pex")
#define ID_VCCINT                   std::string("vccint")
#endif
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
#define ID_12V_PEX                  std::string("12V_PEX")
#define ID_12V_AUX                  std::string("12V_AUX")
#define ID_3V3_PEX                  std::string("3V3_PEX")
#define ID_VCCINT                   std::string("VCCINT")
#endif
#define DEFAULT_POWER_RAIL_SOURCES  std::vector<std::string>({ \
    ID_3V3_PEX, \
    ID_12V_PEX, \
    ID_12V_AUX, \
    ID_VCCINT \
}) \

// Default power_consumption sources
#ifdef USE_XRT
#define ID_POWER_CONSUMPTION    std::string("power_consumption")
#endif
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
#define ID_POWER_CONSUMPTION    std::string("Total_Power")
#endif
#define DEFAULT_POWER_CONSUMPTION_SOURCES   std::vector<std::string>({ \
    ID_POWER_CONSUMPTION \
}) \
// Default electrical sources parsed by default, must concatenate all power sources
//      => DEFAULT_POWER_RAIL_SOURCES & DEFAULT_POWER_CONSUMPTION_SOURCES
#define DEFAULT_ELECTRICAL_SOURCES          std::vector<std::string>({ \
    ID_12V_PEX, \
    ID_12V_AUX, \
    ID_3V3_PEX, \
    ID_VCCINT, \
    ID_POWER_CONSUMPTION \
}) \

// Default thermal sources
#ifdef USE_XRT
#define ID_FPGA0            std::string("fpga0")
#endif
#if defined(USE_AMI) || defined(USE_NO_DRIVER)
#define ID_FPGA0            std::string("Device")
#endif
#define DEFAULT_THERMAL_SOURCES     std::vector<std::string>({ \
    ID_FPGA0 \
}) \
// Default mechanical sources
#define ID_FPGA_FAN_1               std::string("fpga_fan_1")
#define DEFAULT_MECHANICAL_SOURCES  std::vector<std::string>({ \
    ID_FPGA_FAN_1 \
}) \

#define DEFAULT_CARD_POWER_ID        ID_POWER_CONSUMPTION
#define DEFAULT_CARD_TEMPERATURE_ID  ID_FPGA0

#define RST_NAME_INFO       std::string("UNKNOWN")
#define RST_LOGIC_UUID      std::string("UNKNOWN")
#define RST_INTERFACE_UUID  std::string("UNKNOWN")
#define RST_NODMA_INFO      false

#define STATUS_SENSOR_INVALID           std::string("INVALID")
#define STATUS_SENSOR_NOT_PRESENT       std::string("NOT_PRESENT")
#define STATUS_SENSOR_OK                std::string("OK")
#define STATUS_SENSOR_NO_DATA           std::string("NO_DATA")
#define STATUS_SENSOR_OK_CACHED         std::string("OK_CACHED")
#define STATUS_SENSOR_NOT_APPLICABLE    std::string("N/A")
#define STATUS_SENSOR_UNKNOWN           std::string("UNKNOWN")

#ifdef USE_AMI
inline std::string ConvertAmiStatusEnum2Str( const enum ami_sensor_status & status )
{
    switch (status)
    {
                                                   // Used in CSV, do not use "," in string values here
        case AMI_SENSOR_STATUS_INVALID:     return STATUS_SENSOR_INVALID;           break;
        case AMI_SENSOR_STATUS_NOT_PRESENT: return STATUS_SENSOR_NOT_PRESENT;       break;
        case AMI_SENSOR_STATUS_OK:          return STATUS_SENSOR_OK;                break;
        case AMI_SENSOR_STATUS_NO_DATA:     return STATUS_SENSOR_NO_DATA;           break;
        case AMI_SENSOR_STATUS_OK_CACHED:   return STATUS_SENSOR_OK_CACHED;         break;
        case AMI_SENSOR_STATUS_NA:          return STATUS_SENSOR_NOT_APPLICABLE;    break;
        default:                            return "Unknown AMI status (" + std::to_string(status) + ")"; break;
    }
}
#endif

// electrical
struct Voltage_Info_t {
    double                  volts;
    std::string             sensor_status;
    bool                    is_present;
};
#define RST_VOLTAGE_INFO Voltage_Info_t({ \
    .volts          = 0.0, \
    .sensor_status  = STATUS_SENSOR_UNKNOWN, \
    .is_present     = false \
})

struct Current_Info_t {
    double                  amps;
    std::string             sensor_status;
    bool                    is_present;
};
#define RST_CURRENT_INFO Current_Info_t({ \
    .amps           = 0.0, \
    .sensor_status  = STATUS_SENSOR_UNKNOWN, \
    .is_present     = false \
})

struct Power_Rail_Info_t {
    std::string     id;
    Voltage_Info_t  voltage;
    Current_Info_t  current;
};
#define RST_POWER_RAIL_INFO Power_Rail_Info_t({ \
    .id         = "", \
    .voltage    = RST_VOLTAGE_INFO, \
    .current    = RST_CURRENT_INFO \
}) \

struct Power_Consumption_Info_t {
    std::string             id;
    double                  power_consumption_watts;
    std::string             sensor_status;
    bool                    is_present;
};
#define RST_POWER_CONSUMPTION_INFO Power_Consumption_Info_t({ \
    .id                         = "", \
    .power_consumption_watts    = 0.0, \
    .sensor_status              = STATUS_SENSOR_UNKNOWN, \
    .is_present                 = false \
})

struct Electrical_Info_t {
    std::string                             info_json;
    std::vector<Power_Rail_Info_t>          power_rails;
    std::vector<Power_Consumption_Info_t>   power_consumptions;
};
#define RST_ELECTRICAL_INFO Electrical_Info_t({ \
    .info_json          = "", \
    .power_rails        = {}, \
    .power_consumptions = {} \
}) \

// thermal
struct Thermal_El_Info_t {
    std::string             location_id;
    double                  temp_c;
    std::string             sensor_status;
    bool                    is_present;
};
#define RST_THERMAL_EL_INFO Thermal_El_Info_t({ \
    .location_id    = "", \
    .temp_c         = 0.0, \
    .sensor_status  = STATUS_SENSOR_UNKNOWN, \
    .is_present     = false \
})

struct Thermal_Info_t {
    std::string                     info_json;
    std::vector<Thermal_El_Info_t>  thermals;
};
#define RST_THERMAL_INFO Thermal_Info_t({ \
    .info_json = "", \
    .thermals = {} \
}) \

// mechanical
struct Fan_Info_t {
    std::string             location_id;
    double                  speed_rpm;
    std::string             sensor_status;
    bool                    is_present;
};
#define RST_FAN_INFO Fan_Info_t({ \
    .location_id                = "", \
    .speed_rpm                  = 0.0, \
    .sensor_status              = STATUS_SENSOR_UNKNOWN, \
    .is_present                 = false \
})

struct Mechanical_Info_t {
    std::string             info_json;
    std::vector<Fan_Info_t> fans;
};
#define RST_MECHANICAL_INFO Mechanical_Info_t({ \
    .info_json  = "", \
    .fans       = {} \
}) \

struct Mem_Info_t {
    uint64_t    base_address;
    uint64_t    range_bytes;
    uint        mem_data_idx;
};
#define RST_MEM_INFO Mem_Info_t({ \
    .base_address   = 0, \
    .range_bytes    = 0, \
    .mem_data_idx   = 0 \
}) \

struct Mem_Infos_t {
    std::vector<Mem_Info_t> memories;
};
#define RST_MEM_INFOS Mem_Infos_t({ \
    .memories = {} \
}) \

struct Memory_Info_t {
    std::string info_json;
    Mem_Infos_t memory;
};
#define RST_MEMORY_INFO Memory_Info_t({ \
    .info_json  = "", \
    .memory     = RST_MEM_INFOS \
}) \

// design
struct Design_Status_Info_t {
    std::string p2p_status;
};
#define RST_PLATFORM_STATUS_INFO Design_Status_Info_t({ \
    .p2p_status = "not supported" \
}) \

struct Design_Satellite_Controller_Info_t {
    std::string version;
    std::string expected_version;
};
#define RST_PLATFORM_SATELLITE_CONTROLLER_INFO Design_Satellite_Controller_Info_t({ \
    .version            = "", \
    .expected_version   = "" \
}) \

struct Design_Amc_Info_t {
    std::string version;
    std::string expected_version;
};
#define RST_PLATFORM_AMC_INFO Design_Amc_Info_t({ \
    .version            = "", \
    .expected_version   = "" \
}) \

struct Design_Controller_Info_t {
    Design_Satellite_Controller_Info_t  satellite_controller;
    Design_Amc_Info_t                   amc;
};
#define RST_DESIGN_CONTROLLER_INFO Design_Controller_Info_t({ \
    .satellite_controller = RST_PLATFORM_SATELLITE_CONTROLLER_INFO, \
    .amc                  = RST_PLATFORM_AMC_INFO \
}) \

struct Design_Mac_Info_t {
    std::string address;
};
#define RST_DESIGN_MAC_INFO Design_Mac_Info_t({ \
    .address = "" \
}) \

struct Design_Info_t {
    std::string                         info_json;
    Design_Status_Info_t                status;
    Design_Controller_Info_t            controller;
    std::vector<Design_Mac_Info_t>      macs;
};
#define RST_DESIGN_INFO Design_Info_t({ \
    .info_json      = "", \
    .status         = RST_PLATFORM_STATUS_INFO, \
    .controller     = RST_DESIGN_CONTROLLER_INFO, \
    .macs           = {} \
}) \

// host
struct Host_Info_t {
    std::string info_json;
    std::string version;
    std::string build_date;
};
#define RST_HOST_INFO Host_Info_t({ \
    .info_json  = "", \
    .version    = "", \
    .build_date = "" \
}) \

// pcie_info
struct Pcie_Info_t {
    std::string info_json;
    uint        link_speed_gbit_sec;
    uint        expected_link_speed_gbit_sec;
    uint        express_lane_width_count;
    uint        expected_express_lane_width_count;
    std::string cpu_affinity;
};
#define RST_PCIE_INFO Pcie_Info_t({ \
    .info_json                          = "", \
    .link_speed_gbit_sec                = 0, \
    .expected_link_speed_gbit_sec       = 0, \
    .express_lane_width_count           = 0, \
    .expected_express_lane_width_count  = 0, \
    .cpu_affinity                       = "" \
}) \

// dynamic_regions
struct Dynamic_Regions_Info_t {
    std::string info_json;
};
#define RST_DYNAMIC_REGIONS_INFO Dynamic_Regions_Info_t({ \
    .info_json = "" \
}) \

// device_state
struct Device_State_Info_t {
    std::string current_state;
    std::string expected_state;
};
#define RST_DEVICE_STATE_INFO Device_State_Info_t({ \
    .current_state  = "UNKNOWN", \
    .expected_state = "UNKNOWN" \
}) \

// Memory xbtest HW IP is doing 64 Bytes burst (16 words of 32b)
// Verify/Power/gt_lpbk/gt_mac xbtest HW IPs => 1 word of 32b (=4 Bytes)
// => To avoid AXI 4kB boundary violation /Verifypower/gt_lpbk/gt_mac xbtest HW IP buffer size must be multiple of 128 Bytes
#define BUF_SIZE_MULTIPLE    (uint64_t)(128)

#define VERIFY_BUF_SIZE      (uint64_t)(4)
#define POWER_BUF_SIZE       (uint64_t)(4)
// next power of 2 of the quantity of register transferred: currently 11 register => 16 => 16*4 = 64
#define MEMORY_CHAN_BUF_SIZE (uint64_t)(64)
// next power of 2 of the quantity of register transferred: currently 13 register => 16 => 16*4 = 64
#define MULTI_GT_BUFFER_SIZE (uint64_t)(64)

 // 2048 x 32 bits words
#define GT_MAC_BUF_SIZE      (uint64_t)(2048)
#define GT_LPBK_BUF_SIZE     (uint64_t)(4)
#define GT_PRBS_BUF_SIZE     (uint64_t)(4)
#define GTF_PRBS_BUF_SIZE    (uint64_t)(4)
#define GTM_PRBS_BUF_SIZE    (uint64_t)(4)
#define GTYP_PRBS_BUF_SIZE   (uint64_t)(4)

// host buffer size = 64B to support nodma
#define BUFF_SIZE            (uint64_t)(64)

// Buffer for status report from MAC
#define GT_MAC_STATUS_SIZE (uint64_t)(448)
using MACStatus_t = uint32_t[GT_MAC_STATUS_SIZE];

using GTMACMessage_t = struct GTMACMessage_t {
    uint            header;
    std::string     msg;
    uint32_t        test_it_cnt;
    uint64_t        counter;
    MACStatus_t     mac_status;
};

inline std::vector<std::string> ListDirectoryContent( const std::string & dir )
{
    std::vector<std::string> file_list;
    glob_t glob_res;
    glob(dir.c_str(), GLOB_TILDE, nullptr, &glob_res);
    for (uint i = 0; i < glob_res.gl_pathc; ++i)
    {
        auto path       = std::string(glob_res.gl_pathv[i]);
        auto path_split = split(path, '/');
        file_list.emplace_back(path_split.back());
    }
    return file_list;
}

} // namespace

#endif /* _XBTESTPACKAGE_H */
