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

#ifndef _CONSOLEMGMTSW_H
#define _CONSOLEMGMTSW_H

#include "xbtestswconfig.h"
#include "consolemgmtbase.h"
#include "xqueue.h"

namespace xbtest
{

class ConsoleMgmtSw : public ConsoleMgmtBase
{

public:
    ConsoleMgmtSw( Logging * log, Timer * timer, XbtestSwConfig * xbtest_sw_config );
    ~ConsoleMgmtSw();

    void AllocateTestcaseQueue( const std::string & testcase, const std::string & thread, const Testcase_Queue_Value_t & value );
    void PushTestcaseQueue    ( const std::string & testcase, const std::string & thread, const Testcase_Queue_Value_t & value );
    void AllocateSensorQueue  ( const Sensor_Queue_Value_t & value );
    void PushSensorQueue      ( const Sensor_Queue_Value_t & value );
    bool SetUp() override;
    void RefreshData() override;

private:
    XbtestSwConfig * m_xbtest_sw_config = nullptr;

    std::map<std::string,std::map<std::string,XQueue<Testcase_Queue_Value_t>*>>     m_testcase_queues;
    std::map<std::string,std::map<std::string,Testcase_Queue_Value_t>>              m_testcase_queue_value_last;
    bool                                                                            m_sensor_queues_exists = false;
    XQueue<Sensor_Queue_Value_t> *                                                  m_sensor_queues;
    Sensor_Queue_Value_t                                                            m_sensor_queue_value_last;

    const std::string TOP_COL_NAME_0    = "TESTCASE";
    const std::string TOP_COL_NAME_1    = "STATUS";
    const std::string TOP_COL_NAME_2    = "ONGOING TEST";
    const std::string STAT_COL_NAME_0   = "Pending";
    const std::string STAT_COL_NAME_1   = "Completed";
    const std::string STAT_COL_NAME_2   = "Passed";
    const std::string STAT_COL_NAME_3   = "Failed";
    const std::string STAT_COL_NAME_4   = "Errors";
    const std::string STAT_COL_NAME_5   = "Warnings";
    const std::string ONGO_COL_NAME_0   = "Remaining time (s)";
    const std::string ONGO_COL_NAME_1   = "Parameters";

    std::vector<std::string> UpdateContent( const bool & last ) override;
    std::vector<std::string> UpdateConsoleTable() override;

};

} // namespace

#endif /* _CONSOLEMGMTSW_H */
