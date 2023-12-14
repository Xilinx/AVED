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

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <adf.h>
#include "user_parameters.h"

void compute (
    input_stream<int32>  * restrict sin0
);

void control (
    input_stream<uint32> * restrict sin0,
    output_stream<int32> * restrict sout0
);

#endif // __KERNEL_H__
