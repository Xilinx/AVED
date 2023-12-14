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

#ifndef _GTLPBKTEST_H
#define _GTLPBKTEST_H

#include <list>

#include "testinterface.h"
#include "devicemgt.h"
#include "gtmacpackage.h"

namespace xbtest
{

class GTLpbkTest : public TestInterface
{

public:
    GTLpbkTest(
        Logging * log, XbtestSwConfig * xbtest_sw_config, ConsoleMgmtSw * console_mgmt, Timer * timer, DeviceInterface * device, DeviceMgt * device_mgt,
        const Gt_Parameters_t & test_parameters, const uint & cu_idx
    );
    ~GTLpbkTest();

    bool PreSetup() override;
    void Run() override;
    void PostTeardown() override;
    void Abort() override;

private:
    const std::string REP = "%";

    const uint RESET_REG_ADDR           = 0x20;
    const uint GT_LANE_0_CFG_REG_ADDR   = 0x21;
    const uint GT_LANE_1_CFG_REG_ADDR   = 0x22;
    const uint GT_LANE_2_CFG_REG_ADDR   = 0x23;
    const uint GT_LANE_3_CFG_REG_ADDR   = 0x24;
    const std::array<uint,4> GT_LANES_CFG_REG_ADDR = {GT_LANE_0_CFG_REG_ADDR, GT_LANE_1_CFG_REG_ADDR, GT_LANE_2_CFG_REG_ADDR, GT_LANE_3_CFG_REG_ADDR};

    using LaneParam_t = struct LaneParam_t {
        std::string gt_settings;
        uint32_t    gt_tx_diffctrl;
        uint32_t    gt_tx_pre_emph;
        uint32_t    gt_tx_post_emph;
        bool        gt_rx_use_lpm;
        std::string tx_polarity;
    };
    const LaneParam_t RSTC_LANEPARAM = {
        .gt_settings        = "",
        .gt_tx_diffctrl     = 0,
        .gt_tx_pre_emph     = 0,
        .gt_tx_post_emph    = 0,
        .gt_rx_use_lpm      = false,
        .tx_polarity        = ""
    };

    using GTLPBKTestcaseCfg_t = struct GTLPBKTestcaseCfg_t {
        LaneParam_t                 global_param;
        std::map<uint, LaneParam_t> lane_param;
    };

    using Cmd_t = enum Cmd_t {
        RUN,
        CONFIG_10GBE,
        CONFIG_25GBE,
        TX_RX_RST
    };

    using GTLpbkTestItConfig_t = struct GTLpbkTestItConfig_t {
        uint                        duration;
        Cmd_t                       cmd;
        std::string                 cmd_str;
        std::vector<std::string>    test_strvect;
        std::vector<std::string>    test_strvect_dbg;
    };

    DeviceMgt *                                     m_devicemgt = nullptr;
    Gt_Parameters_t                                 m_test_parameters;
    uint                                            m_cu_idx;
    uint                                            m_gt_index;
    std::vector<GTLpbk_Test_Sequence_Parameters_t>  m_test_sequence;
    GTLPBKTestcaseCfg_t                             m_TC_Cfg;
    std::list<GTLpbkTestItConfig_t>                 m_test_it_list;

    bool    WriteGtlpbkCu               ( const uint & address, const uint & value);
    bool    ReadGtlpbkCu                ( const uint & address, uint & read_data );
    void    ResetGT();
    void    SendGTCfg();
    int     RunTest() override;
    bool    StartCU() override;
    bool    EnableWatchdogClkThrotDetection() override;
    bool    StopCU() override;
    bool    CheckWatchdogAndClkThrottlingAlarms() override;
    bool    GetTestSequence();
    bool    ParseTestSequenceSettings   ( std::list<GTLpbkTestItConfig_t> & test_list );
    void    PrintLaneParam              ( const LaneParam_t & lane_param );

};

} // namespace

#endif /* _GTLPBKTEST_H */
