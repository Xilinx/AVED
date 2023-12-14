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

#ifndef _TIMER_H
#define _TIMER_H

#include <mutex>
#include <condition_variable>

#include "xbtestpackage.h"
#include "logging.h"

namespace xbtest
{

class Timer
{

public:
    using Timer_Resolution_t = enum Timer_Resolution_t {
        RES_1s,
        RES_333ms
    };

    Timer( Logging * log, const Timer_Resolution_t & timer_res );
    ~Timer();

    void Stop();
    bool RunTask();
    void WaitFor1sTick();
    void WaitFor333msTick();
    uint64_t GetSecCnt();

private:

    std::string             m_log_header = LOG_HEADER_TIMER;
    Logging *               m_log = nullptr;
    bool                    m_stop = false;
    std::mutex              m_Mutex_333ms;
    std::condition_variable m_CV_333ms;
    std::mutex              m_Mutex_1s;
    std::condition_variable m_CV_1s;
    std::mutex              m_Mutex_sec_cnt;
    uint64_t                m_sec_cnt = 0;
    Timer_Resolution_t      m_timer_res = RES_1s;

    void LogMessage( const Message_t & message, const std::vector<std::string> & arg_list );
    void LogMessage( const Message_t & message );
    void Signal1sTick();
    void Signal333msTick();

};

} // namespace

#endif /* _TIMER_H */
