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

#include <csignal>
#include <iostream>

#include "xbtest.h"

void signal_handler( int signum )
{
    switch (signum)
    {
        case SIGINT:
        {
            xbtest::abort();
            break;
        }
        case SIGBUS:
        {
            xbtest::terminate();
            break;
        }
        default: break;
    }
}

int main ( int argc, char** argv )
{
    if (std::signal(SIGINT, signal_handler) == SIG_ERR) // for ctrl+c
    {
        std::cerr << "Setup of signal handler failed for SIGINT\n";
        return EXIT_FAILURE;
    }
    if (std::signal(SIGBUS, signal_handler) == SIG_ERR)
    {
        std::cerr << "Setup of signal handler failed for SIGBUS\n";
        return EXIT_FAILURE;
    }
    return xbtest::run(argc,argv);
}