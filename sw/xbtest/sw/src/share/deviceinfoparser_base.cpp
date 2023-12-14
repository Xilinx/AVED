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

#include <cmath>
#include <iostream>

#include "deviceinfoparser_base.h"

namespace xbtest
{

DeviceInfoParserBase::DeviceInfoParserBase( Logging * log, const std::string & bdf, std::atomic<bool> * abort )
{
    this->m_log           = log;
    this->m_bdf           = bdf;
    this->m_abort         = abort;

    InitApiFailCnt();
}

DeviceInfoParserBase::~DeviceInfoParserBase()
{
    ReleaseDevice();
}

void DeviceInfoParserBase::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
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

void DeviceInfoParserBase::LogMessage ( const Message_t & message )
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

void DeviceInfoParserBase::UpdateMessageHistory ( const Message_t & message )
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

void DeviceInfoParserBase::SetReportLevel( const LogLevel & level )
{
    this->m_level          = level;
    this->m_default_report = false;
}

uint64_t DeviceInfoParserBase::GetApiFailTotal()
{
    return this->m_api_fail_total;
}

void DeviceInfoParserBase::InitApiFailCnt()
{
    this->m_api_fail_cnt = 0;
    this->m_message_history.clear();
    if (API_FAIL_ERROR_LIMIT == 0)
    {
        SetReportLevel(LOG_ERROR);
    }
    else
    {
        SetReportLevel(LOG_CRIT_WARN);
    }
}

bool DeviceInfoParserBase::RetFailure()
{
    this->m_api_fail_cnt++;
    this->m_api_fail_total++;
    if (this->m_api_fail_cnt == API_FAIL_ERROR_LIMIT)
    {
        SetReportLevel(LOG_DEBUG_EXCEPTION);
        for (const auto & message : this->m_message_history)
        {
            if (message.first != MSG_ITF_145.id)
            {
                LogMessage(MSG_ITF_145, {LogLevelToSeverity(message.second), message.first});
            }
        }
        this->m_message_history.clear();
    }
    return RET_FAILURE;
}

bool DeviceInfoParserBase::RetSuccess()
{
    InitApiFailCnt();
    return RET_SUCCESS;
}

bool DeviceInfoParserBase::CreateDevice() { return RET_SUCCESS; }

void DeviceInfoParserBase::ReleaseDevice() {}

bool DeviceInfoParserBase::GetElectricalInfo( Electrical_Info_t & electrical_info )
{
    if (GetElectricalInfoInner(electrical_info) == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}
bool DeviceInfoParserBase::GetThermalInfo( Thermal_Info_t & thermal_info )
{
    if (GetThermalInfoInner(thermal_info) == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}

bool DeviceInfoParserBase::GetMechanicalInfo( Mechanical_Info_t & mechanical_info )
{
    if (GetMechanicalInfoInner(mechanical_info) == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}

// Wrap the Sensor calls to increment counter only once per second
bool DeviceInfoParserBase::GetSensorsInfo( Electrical_Info_t & electrical_info, Thermal_Info_t & thermal_info, Mechanical_Info_t & mechanical_info )
{
    auto ret = RET_SUCCESS;

    ret |= GetElectricalInfoInner(electrical_info);
    ret |= GetThermalInfoInner   (thermal_info);
    ret |= GetMechanicalInfoInner(mechanical_info);
    if (ret == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}

bool DeviceInfoParserBase::GetSensorsInfo( Electrical_Info_t & electrical_info, Thermal_Info_t & thermal_info )
{
// only used by common, no update required for now
    auto ret = RET_SUCCESS;
    ret |= GetElectricalInfoInner(electrical_info);
    ret |= GetThermalInfoInner   (thermal_info);
    if (ret == RET_FAILURE)
    {
        return RetFailure();
    }
    return RetSuccess();
}

} // namespace
