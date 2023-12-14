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

#include "xqueue.h"

namespace xbtest
{

template<typename T> void XQueue<T>::Push( const T & value )
{
    std::unique_lock<std::mutex> lock(this->m_mutex);
    this->m_queue.push(value);
}

template<typename T> bool XQueue<T>::Pop( T & value )
{
    std::unique_lock<std::mutex> lock(this->m_mutex);
    if (this->m_queue.empty())
    {
        return false;
    }
    value = this->m_queue.front();
    this->m_queue.pop();
    return true;
}

template<typename T> bool XQueue<T>::Empty()
{
    return this->m_queue.empty();
}

template class XQueue<Message_t>;
template class XQueue<Message_Stat_Queue_Value_t>;
template class XQueue<Sensor_Queue_Value_t>;
template class XQueue<GTMACMessage_t>;

// xbtest SW
template class XQueue<Testcase_Queue_Value_t>;
// xbtest common
template class XQueue<Card_Test_Queue_Value_t>;

} // namespace
