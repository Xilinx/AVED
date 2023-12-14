/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#include "evl.h"

/******************************************************************************/
/* Defines                                                                    */
/******************************************************************************/

#define EVL_TEST_PRINTF printf( "[EVL_TEST::%d] ", __LINE__ ); printf


/******************************************************************************/
/* Test events                                                                */
/******************************************************************************/

/**
 * @enum    EVL_TEST_EVENTS
 * @brief   Dummy events for testing the EVL
 */
typedef enum
{
    EVL_TEST_EVENT_0 = 0x00,
    EVL_TEST_EVENT_1,
    EVL_TEST_EVENT_2,
    EVL_TEST_EVENT_3,

    MAX_EVL_TEST_EVENT

} EVL_TEST_EVENTS;


/******************************************************************************/
/* Local functions                                                            */
/******************************************************************************/

/**
 * @brief   Test against a single binding
 *
 * @return  N/A
 * 
 */
int iSingleBinding( void );

/**
 * @brief   Test against multiple bindings
 *
 * @return  N/A
 * 
 */
int iMultiBinding( void );

/**
 * @brief   Test against all possible bindings
 *
 * @return  N/A
 * 
 */
int iAllBindings( void );

/**
 * @brief   Test error cases
 *
 * @return  N/A
 * 
 */
int iBindingErrors( void );
/**
 * @brief   Test functions
 *
 * @param   ucModuleId  Unique ID of this module
 * @param   ucEventId   Event being raised
 *
 * @return  OK in all cases
 * 
 */
int test_binding1( uint8_t ucModuleId, uint8_t ucEventId );
int test_binding2( uint8_t ucModuleId, uint8_t ucEventId );
int test_binding3( uint8_t ucModuleId, uint8_t ucEventId );
int test_binding4( uint8_t ucModuleId, uint8_t ucEventId );
int test_binding5( uint8_t ucModuleId, uint8_t ucEventId );


/**
 * @brief   Test initialisation API
 *
 * @param   ucUniqueId  Unique ID of the module
 *
 * @return  OK if test module initialised
 *          ERROR if test module not initalised
 * 
 */
int iTEST_MODULE_Initialise( uint8_t ucUniqueId );

/**
 * @brief   Test bind in callback API
 *
 * @param   pxCallback  Pointer to callback to bind in
 *
 * @return  OK if callback bound
 *          ERROR if callback not bound
 * 
 */
int iTEST_MODULE_BindCallback( EVL_CALLBACK *pxCallback );

/**
 * @brief   Trigger a test event (normally this would happen internally)
 *
 * @return  N/A
 * 
 */
int iTEST_MODULE_DoSomething( void );

/******************************************************************************/
/* Test functions                                                             */
/******************************************************************************/

/**
 * @brief   Main test function
 */
int main( void )
{
    int     iStatus = ERROR;
    uint8_t ucId    = 0;

    printf( "Enter unique ID (00-FF): 0x" );
    scanf( " %X", &ucId );
    EVL_TEST_PRINTF( "Test ID set to 0x%02X\r\n", ucId );

    if( OK == iTEST_MODULE_Initialise( ucId ) )
    {
        EVL_TEST_PRINTF( "Starting test\r\n\r\n" );
        iStatus = OK;

        if( OK == iSingleBinding() )
        {
            EVL_TEST_PRINTF( "Single binding test passed\r\n\r\n" );
        }
        else
        {
            EVL_TEST_PRINTF( "Single binding test Failed\r\n\r\n" );
            iStatus = ERROR;
        }
        
        if( OK == iMultiBinding() )
        {
            EVL_TEST_PRINTF( "Multi binding test passed\r\n\r\n" );
        }
        else
        {
            EVL_TEST_PRINTF( "Multi binding test failed\r\n\r\n" );
            iStatus = ERROR;
        }
        
        if( OK == iAllBindings() )
        {
            EVL_TEST_PRINTF( "All binding test passed\r\n\r\n" );
        }
        else
        {
            EVL_TEST_PRINTF( "All binding test failed\r\n\r\n" );
            iStatus = ERROR;
        }
        
        if( OK == iBindingError() )
        {
            EVL_TEST_PRINTF( "Binding errors test passed\r\n\r\n" );
        }
        else
        {
            EVL_TEST_PRINTF( "Binding errors test failed\r\n\r\n" );
            iStatus = ERROR;
        }

        if( OK == iStatus )
        {
            EVL_TEST_PRINTF( "All tests passed - OK\r\n\r\n" );
        }
        else
        {
            EVL_TEST_PRINTF( "Tests failed - ERROR\r\n\r\n" );
        }
    }
    else
    {
        EVL_TEST_PRINTF( "Error initialising test module\r\n" );
    }
    
    return iStatus;
}

/**
 * @brief   Test against a single binding
 */
int iSingleBinding( void )
{
    int iStatus = ERROR;

    EVL_TEST_PRINTF( "Single binding\r\n" );

    if( OK == iTEST_MODULE_BindCallback( &test_binding1 ) )
    {
        EVL_TEST_PRINTF( "test1 bound OK\r\n" );
        iStatus = iTEST_MODULE_DoSomething();
    }
    else
    {
        EVL_TEST_PRINTF( "Error binding test1\r\n" );
        iStatus = ERROR;
    }

    return iStatus;
}

/**
 * @brief   Test against multiple bindings
 */
int iMultiBinding( void )
{
    int iStatus = ERROR;
    int i       = 0;

    EVL_TEST_PRINTF( "Multi binding\r\n" );

    if( OK == iTEST_MODULE_BindCallback( &test_binding2 ) )
    {
        EVL_TEST_PRINTF( "test2 bound OK\r\n" );
        iStatus = iTEST_MODULE_DoSomething();
    }
    else
    {
        EVL_TEST_PRINTF( "Error binding test2\r\n" );
        iStatus = ERROR;
    }

    return iStatus;
}

/**
 * @brief   Test against all possible bindings
 */
int iAllBindings( void )
{
    int iStatus = ERROR;

    EVL_TEST_PRINTF( "All bindings\r\n" );

    if( OK == iTEST_MODULE_BindCallback( &test_binding3 ) )
    {
        EVL_TEST_PRINTF( "test3 bound OK\r\n" );

        if( OK == iTEST_MODULE_BindCallback( &test_binding4 ) )
        {
            EVL_TEST_PRINTF( "test4 bound OK\r\n" );
            iStatus = iTEST_MODULE_DoSomething();
        }
        else
        {
            EVL_TEST_PRINTF( "Error binding test4\r\n" );
            iStatus = ERROR;
        }
    }
    else
    {
        EVL_TEST_PRINTF( "Error binding test3\r\n" );
        iStatus = ERROR;
    }

    return iStatus;
}

/**
 * @brief   Test against multiple bindings
 */
int iBindingError( void )
{
    int iStatus = ERROR;

    EVL_TEST_PRINTF( "Binding errors\r\n" );

    if( ERROR == iTEST_MODULE_BindCallback( &test_binding5 ) )
    {
        EVL_TEST_PRINTF( "test5 not bound\r\n" );
        iStatus = OK;
    }
    else
    {
        EVL_TEST_PRINTF( "test5 erroneously bound\r\n" );
        iStatus = ERROR;
    }

    return iStatus;
}

/******************************************************************************/
/* Helper functions                                                           */
/******************************************************************************/

/**
 * @brief   Test function 1
 */
int test_binding1( uint8_t ucModuleId, uint8_t ucEventId )
{
    EVL_TEST_PRINTF( "Function 1 - %02X:%02X\r\n", ucModuleId, ucEventId );

    return OK;
}

/**
 * @brief   Test function 2
 */
int test_binding2( uint8_t ucModuleId, uint8_t ucEventId )
{
    EVL_TEST_PRINTF( "Function 2 - %02X:%02X\r\n", ucModuleId, ucEventId );

    return OK;
}

/**
 * @brief   Test function 3
 */
int test_binding3( uint8_t ucModuleId, uint8_t ucEventId )
{
    EVL_TEST_PRINTF( "Function 3 - %02X:%02X\r\n", ucModuleId, ucEventId );

    return OK;
}

/**
 * @brief   Test function 4
 */
int test_binding4( uint8_t ucModuleId, uint8_t ucEventId )
{
    EVL_TEST_PRINTF( "Function 4 - %02X:%02X\r\n", ucModuleId, ucEventId );

    return OK;
}

/**
 * @brief   Test function 5
 */
int test_binding5( uint8_t ucModuleId, uint8_t ucEventId )
{
    EVL_TEST_PRINTF( "Function 5 - %02X:%02X\r\n", ucModuleId, ucEventId );

    return OK;
}


/******************************************************************************/
/* Test module                                                                */
/******************************************************************************/

/**
 * @struct  TEST_MODULE_DATA
 * @brief   Example test data structure
 */
typedef struct
{
    int         iInitialised;
    uint8_t     ucMyId;
    EVL_RECORD  xRecord;

} TEST_MODULE_DATA;

static TEST_MODULE_DATA xMyData =
{ 
    FALSE,
    0x00,
    { { 0 } }
};
static TEST_MODULE_DATA *pxThis = &xMyData;

/**
 * @brief Test initialisation API
 */
int iTEST_MODULE_Initialise( uint8_t ucUniqueId )
{
    int iStatus = ERROR;
        
    if( FALSE == pxThis->iInitialised )
    {
        pxThis->ucMyId = ucUniqueId;
        pxThis->iInitialised = TRUE;
        iStatus = OK;

        iEVL_GetStats( &pxThis->xRecord );
    }

    return iStatus;
}

/**
 * @brief Test bind in callback API
 */
int iTEST_MODULE_BindCallback( EVL_CALLBACK *pxCallback )
{
    int iStatus = ERROR;

    if( ( TRUE == pxThis->iInitialised ) && ( NULL != pxCallback ) )
    {
        if( OK == iEVL_BindCallback( &pxThis->xRecord, pxCallback ) )
        {
            iStatus = OK;
        }
    }

    return iStatus;
}

/**
 * @brief Trigger a test event (normally this would happen internally)
 */
int iTEST_MODULE_DoSomething( void )
{
    int iStatus = ERROR;
    int i       = 0;

    if( TRUE == pxThis->iInitialised )
    {
        iStatus = OK;

        for( i = 0; i < MAX_EVL_TEST_EVENT; i++ )
        {
            if( OK != iEVL_RaiseEvent( &pxThis->xRecord, pxThis->ucMyId, i ) )
            {
                iStatus = ERROR;
                break;
            }

#ifdef EVL_LOGGING
            if( OK != iEVL_GetStats( &pxThis->xRecord ) )
            {
                EVL_TEST_PRINTF( "Error getting stats\r\n" );
                iStatus = ERROR;
            }
#endif
            printf( "\r\n" );
        }

    }

    return iStatus;
}
