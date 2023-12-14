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

#ifndef _XBTESTCOMMONPACKAGE_H
#define _XBTESTCOMMONPACKAGE_H

#include "xbtestpackage.h"

namespace xbtest
{

// Line HEADER
#define LOG_HEADER_CONFIG_PARSER    std::string("CONF_PARSER ")
#define LOG_HEADER_CARD_PARSER      std::string("CARD_PARSER ")

using Version_t = struct Version_t {
    uint major;
    uint minor;
};

using HW_Build_Info_Xbtest_t = struct HW_Build_Info_Xbtest_t {
    Version_t   version;
    std::string date;
    bool        internal_release;
};

using HW_Build_Info_Board_t = struct HW_Build_Info_Board_t {
    std::string name;
    std::string logic_uuid;
    std::string interface_uuid;
};

using HW_Build_Info_t = struct HW_Build_Info_t {
    HW_Build_Info_Xbtest_t  xbtest;
    HW_Build_Info_Board_t   board;
};

using HW_Lib_Config_t = struct HW_Lib_Config_t {
    HW_Build_Info_t             build_info;
    std::vector<std::string>    pre_canned_test_list;
    std::string                 lib_name;
    std::string                 lib_path;
    std::string                 config_filename;        // Point to user_metadata.json for AMI (AVED package), and to config.json for XRT (xclbin package)
    std::string                 xbtest_pfm_def;
    std::string                 xclbin;
    std::string                 pdi;                            // Used for AMI
    std::string                 metadata_path;                  // Used for AMI
    std::string                 xbtest_metadata_filename;       // Used for AMI (logic UUID)
    std::string                 card_power_id;
    std::string                 card_temperature_id;
    bool                        valid;
};

// Max number of tests in card JSON or max number of times -c and -j present in command line
# define MAX_NUM_TEST          gint(999)
# define MAX_NUM_CARD          uint(100)
# define CARD_JSON_FILE        std::string("card configuration JSON file")
# define HW_CONFIG_JSON        std::string("HW config JSON")
# define SW_CONFIG_JSON        std::string("SW config JSON")
# define COMPATIBILITY_JSON    std::string("Compatibility JSON")
# define HOST_CODE             std::string("host code")
# define ALL                   std::string("all")

using Test_Config_t = struct Test_Config_t {
    std::vector<std::string>    sys_cmd;
    std::vector<std::string>    args;
    std::string                 test_json;
    std::string                 pre_canned;
    bool                        is_pre_canned;
    bool                        is_test_json;
    bool                        use_log_dir;
    std::string                 log_dir;
    std::string                 test_id_name;
};
#define RESET_TEST_CONFIG Test_Config_t({\
    .sys_cmd        = {},    \
    .args           = {},    \
    .test_json      = "",    \
    .pre_canned     = "",    \
    .is_pre_canned  = false, \
    .is_test_json   = false, \
    .use_log_dir    = false, \
    .log_dir        = "",    \
    .test_id_name   = ""     \
})\

using Card_Config_t = struct Card_Config_t {
    std::string                 dsa_name;
    std::string                 interface_uuid;
    std::vector<std::string>    args;
    std::vector<Test_Config_t>  tests;
    HW_Lib_Config_t             lib;
};

using Xbtest_Command_Line_Common_t = struct Xbtest_Command_Line_Common_t {
    std::vector<std::string>            data;
    // Arg with no opt
    bool                                help;
    bool                                version;
    bool                                disable_log;
    bool                                console_detailed_format;
    bool                                force;
    // Arg with opt
    Xbtest_CL_option_Int_t              verbosity;
    Xbtest_CL_option_Str_t              card_json;
    Xbtest_CL_option_Str_t              p2p_target_bdf;
    Xbtest_CL_option_Str_t              p2p_nvme_path;
    Xbtest_CL_option_Str_t              log_dir;
    Xbtest_CL_option_Str_t              msg_id;
    Xbtest_CL_option_Str_t              guide;

    bool                                singlecard_mode; // -d provided only once
    bool                                multicard_mode;  // -d provided more than once
    bool                                multitest_mode;  // Indicate multiple test will be run. E.g single card + multiple tests OR multiple card +single/multiple tests
    bool                                cardjson_mode;   // -D provided
    std::vector<std::string>            all_cards;       // List of all BDFs provided with -d for all configurations
    std::vector<std::string>            cards;
    std::vector<Test_Config_t>          tests;
    std::vector<std::string>            args;   // Use to pass other args to the SW level
    std::map<std::string,Card_Config_t> card_config_map;
};

// Multicard parameters
using Card_JSON_Global_Config_t = struct Card_JSON_Global_Config_t {
    std::vector<std::string>    args;
    std::vector<std::string>    cards;
    std::vector<Test_Config_t>  tests;
};

using Card_JSON_Config_t = struct Card_JSON_Config_t {
    Card_JSON_Global_Config_t             global;
    std::map<std::string,Card_Config_t>   card;
};

using Xbtest_Common_Config_t = struct Xbtest_Common_Config_t {
    Xbtest_System_t                         system;
    std::vector<std::string>                lib_name_list;
    std::vector<std::string>                sw_major_list;
    std::map<std::string, HW_Lib_Config_t>  map_lib_config;
    Xbtest_Command_Line_Common_t            command_line;
    std::map<std::string,Card_Config_t>     card_config_map;
    bool                                    multicard_mode; // -d provided more than once
    bool                                    multitest_mode; // Indicate multiple test will be run. E.g single card + multiple tests OR multiple card +single/multiple tests
    bool                                    use_common_log_dir;
    std::string                             common_log_dir;
};

} // namespace

#endif /* _XBTESTCOMMONPACKAGE_H */
