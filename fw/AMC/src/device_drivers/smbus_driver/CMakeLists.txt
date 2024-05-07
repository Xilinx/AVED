# Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required( VERSION 3.5.0 )

project( smbus-driver )

add_subdirectory( src )

if( TEST_ENABLE )
    add_subdirectory( ./ext/cmocka )
    add_subdirectory( ./test/unittest )
endif()
