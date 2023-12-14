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

#ifndef __USER_PARAMETERS_H__
#define __USER_PARAMETERS_H__

#define ITERATIONS  1024
#define MEM_SIZE    32
#define FIFO_DEPTH  16

// Set number of repetitions of inner loops so that the duration of the kernel execution is 1 second
// Reduce to 0.915s to account for overhead. When set to 1s, the output rate is 91.5% of maximum rate
#ifndef NUM_REP
#define NUM_REP (double)(AIE_FREQ_MHZ)*(double)(1000)*(double)(1000)/(double)(100)/(double)(8)*(double)(0.915)
#endif

#define GMIO_SIZE_BYTES 4

#define AIE_CONTROL_PS   0
#define AIE_CONTROL_PL   1

#define AIE_STATUS_NONE 0
#define AIE_STATUS_PL   1

#endif // __USER_PARAMETERS_H__
