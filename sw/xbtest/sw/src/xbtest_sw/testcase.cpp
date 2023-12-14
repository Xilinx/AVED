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

#include <functional>

#include "testcase.h"

namespace xbtest
{

TestCase::TestCase( Logging * log, const ThreadType & thread_type, const std::string & type_name, const std::string & thread_name, TestInterface * base )
{
    this->m_log           = log;
    this->m_base          = base;
    this->m_thread_type   = thread_type;
    this->m_type_name     = type_name;
    this->m_thread_name   = thread_name;
    this->m_thread_id     = type_name;
    if (!this->m_thread_name.empty()) // This is used in multi-thread (SC memory)
    {
        this->m_thread_id += " " + this->m_thread_name;
    }
}

TestCase::~TestCase() = default;

void TestCase::SignalAbortTest()
{
    this->m_base->Abort(); //signal the atomic abort
}

void TestCase::SetTestComplete()
{
    this->m_complete = true;
}

bool TestCase::GetTestComplete()
{
    return this->m_complete;
}

bool TestCase::CheckTestAborted()
{
    return (this->m_base->GetResult() == TestInterface::TR_ABORTED);
}

ThreadType TestCase::GetTestCaseType()
{
    return this->m_thread_type;
}

std::string  TestCase::GetTypeName()
{
    return this->m_type_name;
}

std::string  TestCase::GetThreadName()
{
    return this->m_thread_name;
}

std::string  TestCase::GetThreadID()
{
    return this->m_thread_id;
}

std::vector<TestIterationResult> TestCase::GetItResult()
{
    return this->m_base->GetItResult();
}

std::vector<TestIterationResult> TestCase::GetItLaneResult( const uint & idx )
{
    return this->m_base->GetItLaneResult(idx);
}

TestInterface * TestCase::GetTestInterfaceBase()
{
    return this->m_base;
}

TestCaseThreadResult TestCase::SpawnTest()
{
    auto ret = TC_FAIL;
    LogMessage(MSG_DEBUG_TESTCASE, {this->m_thread_id + ": Starting thread"});
    this->m_base->PreSetup();
    this->m_base->Run(); // run tests - block until complete or abort/fails
    switch (this->m_base->GetResult())
    {
        default:                        ret = TC_FAIL;      break;
        case TestInterface::TR_PASSED:  ret = TC_PASS;      break;
        case TestInterface::TR_ABORTED: ret = TC_ABORTED;   break;
    }
    this->m_base->PostTeardown(); // tear down required even on failure
    LogMessage(MSG_DEBUG_TESTCASE, {this->m_thread_id + ": Ending thread"});
    return ret;
}

void TestCase::StartThread()
{
    this->m_future_result = std::async(std::launch::async, std::bind(&TestCase::SpawnTest, this));
}

TestCaseThreadResult TestCase::GetThreadResult()
{
    return this->m_future_result.get();
}

bool TestCase::WaitThread( uint duration )
{
    return (this->m_future_result.wait_for(std::chrono::milliseconds(duration)) == std::future_status::ready); // true => thread has completed
}

void TestCase::LogMessage ( const Message_t & message, const std::vector<std::string> & arg_list )
{
    this->m_log->LogMessage(this->m_log_header, message, arg_list);
}

void TestCase::LogMessage ( const Message_t & message )
{
    this->m_log->LogMessage(this->m_log_header, message);
}

} // namespace
