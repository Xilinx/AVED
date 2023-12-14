/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the standard definitions for the OSAL
 *
 * @file standard.h
 *
 */

#ifndef _STANDARD_H_
#define _STANDARD_H_

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define OK          ( 0 )
#define ERROR       ( -1 )

#ifndef TRUE
#define TRUE        ( 1 )
#endif

#ifndef FALSE
#define FALSE       ( 0 )
#endif

#ifndef FOREVER
#define FOREVER     for( ;; )
#endif

#ifndef UNUSED
#define UNUSED( p ) ( void )( p )
#endif

#endif
