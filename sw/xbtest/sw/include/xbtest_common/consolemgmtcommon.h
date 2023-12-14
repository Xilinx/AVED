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

#ifndef _CONSOLEMGMTCOMMON_H
#define _CONSOLEMGMTCOMMON_H

#include "consolemgmtbase.h"
#include "xbtestcommonpackage.h"
#include "xbtestcommonconfig.h"
#include "xqueue.h"

namespace xbtest
{

class ConsoleMgmtCommon : public ConsoleMgmtBase
{

public:

    ConsoleMgmtCommon( Logging * log, Timer * timer, XbtestCommonConfig * xbtest_config );
    ~ConsoleMgmtCommon();

    bool SetUp() override;
    void RefreshData() override;
    void AllocateCardTestQueue( const std::string & dsa_name, const std::string & bdf, const Card_Test_Queue_Value_t & value );
    void PushCardTestQueue    ( const std::string & dsa_name, const std::string & bdf, const Card_Test_Queue_Value_t & value );
    bool CardTestQueueEmpty   ( const std::string & dsa_name, const std::string & bdf );
    void AllocateSensorQueue  ( const std::string & dsa_name, const std::string & bdf, const Sensor_Queue_Value_t & value );
    void PushSensorQueue      ( const std::string & dsa_name, const std::string & bdf, const Sensor_Queue_Value_t & value );


private:

    XbtestCommonConfig *                                                            m_xbtest_config = nullptr;
    std::map<std::string,std::map<std::string,XQueue<Card_Test_Queue_Value_t>*>>    m_card_test_queues;
    std::map<std::string,std::map<std::string,Card_Test_Queue_Value_t>>             m_card_test_queue_value_last;
    std::map<std::string,std::map<std::string,XQueue<Sensor_Queue_Value_t>*>>       m_sensor_queues;
    std::map<std::string,std::map<std::string,Sensor_Queue_Value_t>>                m_sensor_queue_value_last;
    std::ofstream                                                                   m_card_status_csv_file;
    std::ofstream                                                                   m_RT_card_status_csv_file;
    std::string                                                                     m_RT_card_status_csv_file_name;
    std::string                                                                     m_RT_card_status_csv_file_head;

    std::vector<std::string>    UpdateContent( const bool & last ) override;
    std::vector<std::string>    UpdateConsoleTable() override;

};

} // namespace

#endif /* _CONSOLEMGMTCOMMON_H */
