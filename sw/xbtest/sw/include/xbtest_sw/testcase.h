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

#ifndef _TESTCASE_H
#define _TESTCASE_H

#include <future>

#include "testinterface.h"

namespace xbtest
{

class TestCase
{

public:

    TestCase( Logging * log, const ThreadType & thread_type, const std::string & type_name, const std::string & thread_name, TestInterface * base );
    ~TestCase();
    void                                SignalAbortTest();
    void                                SetTestComplete();
    bool                                GetTestComplete();
    bool                                CheckTestAborted();
    ThreadType                          GetTestCaseType();
    std::string                         GetTypeName();
    std::string                         GetThreadName();
    std::string                         GetThreadID();
    std::vector<TestIterationResult>    GetItResult();
    std::vector<TestIterationResult>    GetItLaneResult( const uint & idx );
    TestInterface *                     GetTestInterfaceBase();
    TestCaseThreadResult                SpawnTest();
    void                                StartThread();
    TestCaseThreadResult                GetThreadResult();
    bool                                WaitThread( uint duration );

private:
    std::string                         m_log_header = LOG_HEADER_GENERAL;
    Logging *                           m_log  = nullptr;
    TestInterface *                     m_base;
    ThreadType                          m_thread_type;
    std::string                         m_type_name;
    std::string                         m_thread_name;
    std::string                         m_thread_id;
    bool                                m_complete = false;
    std::future<TestCaseThreadResult>   m_future_result;

    void LogMessage( const Message_t & message, const std::vector<std::string> & arg_list );
    void LogMessage( const Message_t & message );

};

} // namespace

#endif /* _TESTCASE_H */
