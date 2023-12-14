// SPDX-License-Identifier: GPL-2.0-only
/*
 * profile_fal.h - This file contains the fal profile for the Linux platform
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_PROFILE_FAL
#define AMI_PROFILE_FAL

#define PLL_INF( t, m, ... ) printk( "[" t "] " m, ##__VA_ARGS__ )
#define PLL_ERR( t, m, ... ) printk( "[" t "] " m, ##__VA_ARGS__ )
#define PLL_WRN( t, m, ... ) printk( "[" t "] " m, ##__VA_ARGS__ )
#define PLL_LOG( t, m, ... ) printk( "[" t "] " m, ##__VA_ARGS__ )

#ifdef FAL_DBG_PRINT
#define PLL_DBG( t, m, ... ) printk( "[" t "] " m, ##__VA_ARGS__ )
#else
#define PLL_DBG( t, m, ... ) ( ( void )( 0 ) )
#endif

#endif
