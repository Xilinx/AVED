/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the printing profile for the Linux platform
 *
 * @file profile_print.h
 *
 */

#ifndef _PROFILE_PRINT_H_
#define _PROFILE_PRINT_H_

#include <stdarg.h>

#ifndef PRINT
#define PRINT( ... ) ( printf( __VA_ARGS__ ) )
#endif

#endif
