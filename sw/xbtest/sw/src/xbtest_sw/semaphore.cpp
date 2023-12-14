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

#include "semaphore.h"

namespace xbtest
{

void Semaphore::Notify()
{
    std::unique_lock<std::mutex> lock(this->m_mtx);
    this->m_count++;
    this->m_cv.notify_one();
}

void Semaphore::Wait()
{
    std::unique_lock<std::mutex> lock(this->m_mtx);

    while (this->m_count == 0)
    {
        this->m_cv.wait(lock);
    }
    this->m_count--;
}

} // namespace