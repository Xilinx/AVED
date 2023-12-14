/**
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * This file contains the public API of the Printing and Logging Library (PLL)
 *
 * @file pll.h
 *
 */
#ifndef _PLL_H_
#define _PLL_H_

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "standard.h"
#include "osal.h"


/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#ifndef PRINT_BUFFER_SIZE
#define PRINT_BUFFER_SIZE    ( 256 )
#endif

#define PLL_LOG_ENTRY_SIZE   ( 96 )
#define PLL_LOG_MAX_RECS     ( 50 )

#ifdef DEBUG_PRINT
#define PLL_PRINTF( l, t, m, ...) vPLL_Output( l, "[" t "] " m, ##__VA_ARGS__)
#else
#define PLL_PRINTF( m )
#endif

#define PLL_INF( t, m, ... ) vPLL_Output( PLL_OUTPUT_LEVEL_INFO,    "[" t "] " m, ##__VA_ARGS__ )  /* used for stats - always print */
#define PLL_ERR( t, m, ... ) vPLL_Output( PLL_OUTPUT_LEVEL_ERROR,   "[" t "] " m, ##__VA_ARGS__ )  /* used for errors               */
#define PLL_WRN( t, m, ... ) vPLL_Output( PLL_OUTPUT_LEVEL_WARNING, "[" t "] " m, ##__VA_ARGS__ )  /* used for warnings             */
#define PLL_LOG( t, m, ... ) vPLL_Output( PLL_OUTPUT_LEVEL_LOGGING, "[" t "] " m, ##__VA_ARGS__ )  /* used for general printing     */
#define PLL_DBG( t, m, ... ) vPLL_Output( PLL_OUTPUT_LEVEL_DEBUG,   "[" t "] " m, ##__VA_ARGS__ )  /* used for debug prints         */

#define PLL_DAL( t, m, ... ) vPLL_Printf( "[" t "] " m, ##__VA_ARGS__ )  /* used for DAL - ignores verbosity */


/******************************************************************************/
/* Structs                                                                    */
/******************************************************************************/

/**
 * @struct PLL_LOG_MSG
 * @brief  Structure to hold the log message.
 */
typedef struct PLL_LOG_MSG
{
	char	pcBuff[ PLL_LOG_ENTRY_SIZE ];
} PLL_LOG_MSG;

#define PLL_LOG_BUF_LEN ( sizeof( struct PLL_LOG_MSG ) * PLL_LOG_MAX_RECS )


/******************************************************************************/
/* Enums                                                                      */
/******************************************************************************/

/**
 * @enum    PLL_OUTPUT_LEVEL
 * @brief   Output verbosity levels for the PLL   
 */
typedef enum PLL_OUTPUT_LEVEL
{
    PLL_OUTPUT_LEVEL_INFO = 0,  /* used for stats - always print */
    PLL_OUTPUT_LEVEL_ERROR,     /* used for errors               */ 
    PLL_OUTPUT_LEVEL_WARNING,   /* used for warnings             */ 
    PLL_OUTPUT_LEVEL_LOGGING,   /* used for general printing     */ 
    PLL_OUTPUT_LEVEL_DEBUG,     /* used for debug prints         */ 

    MAX_PLL_OUTPUT_LEVEL

} PLL_OUTPUT_LEVEL;


/******************************************************************************/
/* Public function declarations                                               */
/******************************************************************************/

/**
 * @brief   Main initialisation point for the PLL
 *
 * @param   xOutputLevel    Output verbosity level
 *
 * @return  OK     if PLL is initialised and running successfully
 *          ERROR  if the PLL is not initialised
 */ 
int iPLL_Initialise( PLL_OUTPUT_LEVEL xOutputLevel );

/**
 * @brief   Globally sets the PLL verbosity level
 *
 * @param   xOutputLevel    Output verbosity level
 *
 * @return  OK     if Level set correctly
 *          ERROR  if level not set
 */
int iPLL_SetLevel( PLL_OUTPUT_LEVEL xOutputLevel );

/**
 * @brief   Gets current PLL output level
 *
 * @param   pxOutputLevel   Pointer to output level
 * 
 * @return  OK     if level retrieved successful
 *          ERROR  if level not retrieved successful
 */
int iPLL_GetLevel( PLL_OUTPUT_LEVEL *pxOutputLevel );

/**
 * @brief   Function for task/thread safe prints.
 *
 * @param   xOutputLevel  Verbosity level of the message.
 * 
 * @param   pcFormat      C string that contains the text to be written.
 */
void vPLL_Output( PLL_OUTPUT_LEVEL xOutputLevel, const char *pcFormat, ... );

/**
 * @brief   Function for task/thread safe prints.
 *
 * @param   pcFormat  C string that contains the text to be written.
 * 
 * @note    Print function that overwrites the set PLL_OUTPUT_LEVEL, used ...
 *          for the likes of the DAL.
 */
void vPLL_Printf( const char *pcFormat, ... );

/**
 * @brief    Dumps logs from shared memory.
 * 
 * @return   OK if successful
 *           ERROR if not successful. 
 */
int iPLL_DumpLog( void );

/**
 * @brief    Clears shared memory log buffer.
 * 
 * @return   OK if successful
 *           ERROR if not successful. 
 */
int iPLL_ClearLog( void );

/**
 * @brief   Print all the stats gathered by the library
 *
 * @return  OK     Stats retrieved from the PLL successfully
 *          ERROR  Stats not retrieved successfully
 *
 */
int iPLL_PrintStatistics( void );

/**
 * @brief   Clear all the stats in the library
 *
 * @return  OK     Stats cleared successfully
 *          ERROR  Stats not cleared successfully
 *
 */
int iPLL_ClearStatistics( void );

#endif
