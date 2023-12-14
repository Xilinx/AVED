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

#ifndef _CLPACKAGE_H
#define _CLPACKAGE_H

#include <string>

#ifdef USE_XRT
    #define CL_HPP_CL_1_2_DEFAULT_BUILD
    #define CL_HPP_TARGET_OPENCL_VERSION                                120
    #define CL_HPP_MINIMUM_OPENCL_VERSION                               120
    #define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY 1
    #define CL_USE_DEPRECATED_OPENCL_1_2_APIS

    #if __has_include(<CL/opencl.hpp>)

        #include <CL/opencl.hpp>

    #else

        #include <CL/cl2.hpp>

    #endif
    #include <CL/cl_ext.h>
#endif

namespace xbtest
{

using ChkClErr_t = struct ChkClErr_t {
    bool        fail;
    std::string msg;
};

#define CHK_CL_ERR_SUCCESS ChkClErr_t({ \
    false, \
    "" \
})

#define CHK_CL_ERR_FAILURE ChkClErr_t({ \
    true, \
    "" \
})

#define CHK_ABORT_RETURN(m_abort_dev_access_msg, m_abort, m_cl_fail) \
do {                                                                 \
    if ((*m_abort) && m_cl_fail)                                     \
    {                                                                \
        if (m_abort_dev_access_msg)                                  \
        {                                                            \
            m_abort_dev_access_msg = false;                          \
            LogMessage(MSG_ITF_152);                                 \
        }                                                            \
        return RET_FAILURE;                                          \
    }                                                                \
} while(false)

#define CHK_CL_ERR_IT_FAILURE(chk_cl_err, test_it_failure)  \
do {                                                        \
    if (chk_cl_err.fail)                                    \
    {                                                       \
        LogMessage(MSG_ITF_038, {chk_cl_err.msg});          \
        test_it_failure = RET_FAILURE;                      \
    }                                                       \
} while(false)

#define CHK_CL_ERR_RETURN(chk_cl_err)                       \
do {                                                        \
    if (chk_cl_err.fail)                                    \
    {                                                       \
        LogMessage(MSG_ITF_038, {chk_cl_err.msg});          \
        return RET_FAILURE;                                 \
    }                                                       \
} while(false)

#define CHK_CL_ERR_ABORT_RETURN(chk_cl_err, m_abort, m_cl_fail)     \
do {                                                                \
    if (chk_cl_err.fail)                                            \
    {                                                               \
        LogMessage(MSG_ITF_038, {chk_cl_err.msg});                  \
        *m_abort = true;                                            \
        m_cl_fail = true;                                           \
        return RET_FAILURE;                                         \
    }                                                               \
} while(false)

#ifdef USE_XRT

#define CL_ERR_STR(chk_cl_err, cl_err)      case cl_err: chk_cl_err.msg += std::string(". OpenCL return code = ") + std::string(#cl_err);   break;
#define CL_ERR_DEFAULT(chk_cl_err, cl_err)  default:     chk_cl_err.msg += std::string(". OpenCL return code = ") + std::to_string(cl_err); break;

#define CL_ERR_SUCCESS(chk_cl_err, cl_err)                 \
do {                                                       \
    chk_cl_err = CHK_CL_ERR_FAILURE;                       \
    if (cl_err == CL_SUCCESS)                              \
    {                                                      \
        chk_cl_err = CHK_CL_ERR_SUCCESS;                   \
        return;                                            \
    }                                                      \
} while(false)                                             \

void inline CheckClPlatformGet( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Platform::get - Failed to get list of available platforms.";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}
void inline CheckclGetPlatformIDs( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clGetPlatformIDs - Failed to get list of available platforms";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClPlatformGetInfo( const cl_int & cl_err, const std::string & param_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Platform::getInfo - Failed to get OpenCL platform information " + param_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclGetPlatformInfo( const cl_int & cl_err, const std::string & param_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clGetPlatformInfo - Failed to get OpenCL platform information " + param_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClPlatformGetDevices( const cl_int & cl_err, const std::string & device_type_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Platform::getDevices - Failed to get list of devices " + device_type_name + " available on a platform";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE_TYPE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_DEVICE_NOT_FOUND)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclGetDeviceIDs( const cl_int & cl_err, const std::string & device_type_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clGetDeviceIDs - Failed to get list of devices " + device_type_name + " available on a platform";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE_TYPE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_DEVICE_NOT_FOUND)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClDeviceGetInfo( const cl_int & cl_err, const std::string & param_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Device::getInfo - Failed to get OpenCL device information " + param_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclGetDeviceInfo( const cl_int & cl_err, const std::string & param_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clGetDeviceInfo - Failed to get OpenCL device information " + param_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClContextConstructor( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Context - Failed to create an OpenCL context";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROPERTY)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_DEVICE_NOT_AVAILABLE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclCreateContext( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clCreateContext - Failed to create an OpenCL context";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PLATFORM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROPERTY)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_DEVICE_NOT_AVAILABLE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClCommandQueueConstructor( const cl_int & cl_err, const std::string & property_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue - Failed to create a host command-queue with property " + property_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_QUEUE_PROPERTIES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclCreateCommandQueue( const cl_int & cl_err, const std::string & property_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clCreateCommandQueue - Failed to create a host command-queue with property " + property_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_QUEUE_PROPERTIES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClCommandQueueEnqueueTask( const cl_int & cl_err, const std::string & cu_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::EnqueueTask - Failed to enqueue a command to execute kernel " + cu_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROGRAM_EXECUTABLE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL_ARGS)
        CL_ERR_STR(chk_cl_err, CL_INVALID_WORK_GROUP_SIZE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClCommandQueueFinish( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::Finish - Failed to block until command " + info + " in command-queue issued and completed";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}


void inline CheckclFinish( const cl_int & cl_err, const std::string & queue, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clFinish - Failed to block until all commands in command-queue " + queue + " are issued and have completed";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClCommandQueueEnqueueReadBuffer( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::EnqueueReadBuffer - Failed to enqueue command to read buffer for " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClCommandQueueEnqueueWriteBuffer( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::EnqueueWriteBuffer - Failed to enqueue command to write buffer for " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}
void inline CheckClKernelConstructor( const cl_int & cl_err, const std::string & kernel_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Kernel - Failed to create kernel in program: " + kernel_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROGRAM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROGRAM_EXECUTABLE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL_NAME)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL_DEFINITION)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}
void inline CheckClProgramCreateKernels( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Program::createKernels - Failed to create kernels in program";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROGRAM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROGRAM_EXECUTABLE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL_NAME)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL_DEFINITION)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

std::string inline ProgrammConstructorClErrToStr( const cl_int & cl_err )
{
    ChkClErr_t chk_cl_err;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_PROGRAM)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_BINARY)
        CL_ERR_STR(chk_cl_err, CL_INVALID_BUILD_OPTIONS)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_COMPILER_NOT_AVAILABLE)
        CL_ERR_STR(chk_cl_err, CL_BUILD_PROGRAM_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
    return chk_cl_err.msg;
}

std::string inline BufferConstructorClErrToStr( const cl_int & cl_err )
{
    ChkClErr_t chk_cl_err;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_BUFFER_SIZE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_HOST_PTR)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
    return chk_cl_err.msg;
}

void inline CheckClBufferConstructor( const cl_int & cl_err, const std::string & buffer_name, const std::string & flags_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Buffer - Failed to create buffer " + buffer_name + " with flags " + flags_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_BUFFER_SIZE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_HOST_PTR)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclCreateBuffer( const cl_int & cl_err, const std::string & buffer_name, const std::string & flags_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clCreateBuffer - Failed to create buffer " + buffer_name + " with flags " + flags_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_BUFFER_SIZE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_HOST_PTR)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClCreateSubBuffer( const cl_int & cl_err, const std::string & buffer_name, const std::string & flags_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Buffer::CreateSubBuffer - Failed to create sub-buffer " + buffer_name + " with flags " + flags_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_BUFFER_SIZE)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClKernelGetInfo( const cl_int & cl_err, const std::string & cu_name, const std::string & param_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Kernel::getInfo - Failed to get info " + param_name + " for kernel " + cu_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClKernelSetArg( const cl_int & cl_err, const std::string & cu_name, const std::string & arg_index, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::Kernel::setArg - Failed to set argument " + arg_index + " for kernel " + cu_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_KERNEL)
        CL_ERR_STR(chk_cl_err, CL_INVALID_ARG_INDEX)
        CL_ERR_STR(chk_cl_err, CL_INVALID_ARG_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_SAMPLER)
        CL_ERR_STR(chk_cl_err, CL_INVALID_ARG_SIZE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckxclGetMemObjectFromFd( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "xclGetMemObjectFromFd - Failed to get memory object from FD";
    switch (cl_err)
    {
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClEnqueueMapBuffer( const cl_int & cl_err, const std::string & name, const std::string & flags, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::enqueueMapBuffer - Failed to map memory object " + name + " with " + flags;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_STR(chk_cl_err, CL_MAP_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClEnqueueUnmapBuffer( const cl_int & cl_err, const std::string & name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::enqueueUnmapBuffer - Failed to unmap memory object " + name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClEnqueueMigrateMemObjects( const cl_int & cl_err, const std::string & param_name, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::enqueueMigrateMemObjects - Failed to migrate memory object " + param_name;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckEnqueueWriteBuffer( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::enqueueWriteBuffer - Failed to write buffer " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckEnqueueReadBuffer( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::CommandQueue::enqueueReadBuffer - Failed to read buffer " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_OPERATION)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclEnqueueCopyBuffer( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clEnqueueCopyBuffer - Failed to copy buffer " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_MISALIGNED_SUB_BUFFER_OFFSET)
        CL_ERR_STR(chk_cl_err, CL_MEM_COPY_OVERLAP)
        CL_ERR_STR(chk_cl_err, CL_MEM_OBJECT_ALLOCATION_FAILURE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckClWaitForEvents( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "cl::WaitForEvents - Failed";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclWaitForEvents( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clWaitForEvents - Failed";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_VALUE)
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_INVALID_EVENT)
        CL_ERR_STR(chk_cl_err, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclReleaseMemObject( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clReleaseMemObject - Failed to release memory object " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_MEM_OBJECT)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclReleaseContext( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clReleaseContext - Failed to release context " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_CONTEXT)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclReleaseCommandQueue( const cl_int & cl_err, const std::string & info, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clReleaseCommandQueue - Failed to release command queue " + info;
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_COMMAND_QUEUE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}

void inline CheckclReleaseDevice( const cl_int & cl_err, ChkClErr_t & chk_cl_err )
{
    CL_ERR_SUCCESS(chk_cl_err, cl_err);
    chk_cl_err.msg = "clReleaseDevice - Failed to release device";
    switch (cl_err)
    {
        CL_ERR_STR(chk_cl_err, CL_INVALID_DEVICE)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_RESOURCES)
        CL_ERR_STR(chk_cl_err, CL_OUT_OF_HOST_MEMORY)
        CL_ERR_DEFAULT(chk_cl_err, cl_err)
    }
}
#endif

} // namespace

#endif /* _CLPACKAGE_H */
