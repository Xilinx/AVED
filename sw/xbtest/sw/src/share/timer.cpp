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

#include <thread>

#include "timer.h"

namespace xbtest
{

Timer::Timer( Logging * log, const Timer_Resolution_t & timer_res )
{
    this->m_log         = log;
    this->m_timer_res   = timer_res;
}

Timer::~Timer() = default;

void Timer::Stop()
{
    this->m_stop = true;
}

bool Timer::RunTask()
{
    if (this->m_timer_res == RES_333ms)
    {
        LogMessage(MSG_MGT_001);
    }
    else
    {
        LogMessage(MSG_MGT_002);
    }

    while (!(this->m_stop))
    {
        if (this->m_timer_res == RES_333ms)
        {
            for (uint i=0; i<3; i++)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(333333));
                Signal333msTick();
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        Signal1sTick();
    }
    return RET_SUCCESS;
}

void Timer::WaitFor1sTick()
{
    std::unique_lock<std::mutex> lk(this->m_Mutex_1s);
    if (this->m_CV_1s.wait_for(lk,std::chrono::milliseconds(1100)) == std::cv_status::timeout)
    {
        if (!(this->m_stop))
        {
            LogMessage(MSG_MGT_004);
        }
    }
}

void Timer::WaitFor333msTick()
{
    std::unique_lock<std::mutex> lk(this->m_Mutex_333ms);
    if (this->m_timer_res == RES_333ms)
    {
        if (this->m_CV_333ms.wait_for(lk,std::chrono::milliseconds(350)) == std::cv_status::timeout)
        {
            if (!(this->m_stop))
            {
                LogMessage(MSG_MGT_003);
            }
        }
    }
}

uint64_t Timer::GetSecCnt()
{
    std::unique_lock<std::mutex> lk(this->m_Mutex_sec_cnt);
    return this->m_sec_cnt;
}

void Timer::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, message, arg_list);
}

void Timer::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, message);
}

void Timer::Signal1sTick()
{
    std::unique_lock<std::mutex> lk(this->m_Mutex_sec_cnt);
    this->m_sec_cnt++;
    this->m_CV_1s.notify_all();
}

void Timer::Signal333msTick()
{
    this->m_CV_333ms.notify_all();
}

} // namespace
