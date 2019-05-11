/**
 * @file rtsp_abstraction.c
 *
 * Provides abstraction layer for calls to RDS-RTSP and WFD-RTSP.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2012 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors.  Title to the Material remains with Intel
 * Corporation or its suppliers and licensors.  The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors.  The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or
 * delivery of the Materials, either expressly, by implication, inducement,
 * estoppel or otherwise.  Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "rtsp_abstraction.h"

// {{{ Debug helpers
#ifndef _WIDI_DEBUG_MACROS
#define _WIDI_DEBUG_MACROS
#define DEBUG(...)    logger( __func__, __LINE__, LOG_DEBUG, __VA_ARGS__ )  ///< Messages used for development; they aren't shown for production builds
#define NOTICE(...)   logger( __func__, __LINE__, LOG_NOTICE, __VA_ARGS__ ) ///< Informational messages that do not indicate an abnormal situation
#define ERROR(...)    logger( __func__, __LINE__, LOG_ERR, __VA_ARGS__ )    ///< Messages that indicate an abnormal situation, but program operation can continue
#define CRITICAL(...) logger( __func__, __LINE__, LOG_CRIT, __VA_ARGS__ )   ///< Messages that indicate a severe situation where normal program operation cannot continue
#endif // _WIDI_DEBUG_MACROS
// }}}

/**
 * Internal helper function to print debug messages. Function name, file name,
 * and line number are included automatically. Outputs to syslog.
 *
 * @returns None
 */
static void logger( const char* func, int line, int priority, char* format, ... )
    __attribute__ ((format (printf, 4, 5)));
static void logger( const char* func, int line, int priority, char* format, ... )
{
    char new_format[255];
    assert( strlen(format) < sizeof(new_format) );
    snprintf( new_format, sizeof(new_format), "%s: %s (%s:%d)", func, format, __FILE__, line );

    va_list args;
    va_start( args, format );
    vsyslog( priority, new_format, args );
    va_end( args );
}
// }}}

long rtsplib_snkinit( int mode, void** phRtsp,
        unsigned short usRtpPort, rtsp_fn_notification_callback pfnCallback,
        FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP
        pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog,
        void* pContext )
{
    assert( phRtsp );
    assert( pfnCallback );
    assert( pfnRecv );
    assert( pfnSend );
    assert( pfnSleep );
    assert( pfnGetTick );
    assert( pfnLog );

    if ( mode == RTSP_MODE_RDS ) {
        return RTSPSnkInit( phRtsp, usRtpPort,
                (FN_RTSP_NOTIFICATIONS)pfnCallback, pfnRecv, pfnSend,
                pfnSleep, pfnGetTick, pfnLog, pContext );
    }

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPSnkInit( phRtsp, usRtpPort,
                (FN_WFD_RTSP_NOTIFICATIONS)pfnCallback, pfnRecv, pfnSend,
                (FN_WFD_RTSP_SLEEP)pfnSleep, pfnGetTick, pfnLog, pContext
                );
    }

    DEBUG( "unsupported RTSP library mode: %d", mode );
    assert( 0 );
    return 1;
}

long rtsplib_parser( int mode, void* hRtsp )
{
    assert( hRtsp );

    DEBUG( "mode=%d", mode );

    if ( mode == RTSP_MODE_RDS ) {
		return RTSPParser( hRtsp );
    }

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPParser( hRtsp );
    }

    DEBUG( "unsupported RTSP library mode: %d", mode );
    assert( 0 );
    return 1;
}

long rtsplib_get_state(int mode, void* hRtsp)
{
	if ( mode == RTSP_MODE_RDS ) {
		return -1;
    }

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPGetState( hRtsp );
    }

    return -1;
}

long rtsplib_shutdown( int mode, void* hRtsp )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_RDS ) {
		return RTSPShutdown( hRtsp );
    }

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPShutdown( hRtsp );
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

long rtsplib_play( const int mode, void* hRtsp )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPPlay( hRtsp );
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

long rtsplib_teardown( const int mode, void* hRtsp )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPTeardown( hRtsp );
    }

    if ( mode == RTSP_MODE_RDS ) {
        return RTSPTeardown( hRtsp );
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

long rtsplib_pause( const int mode, void* hRtsp )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPPause( hRtsp );
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

long rtsplib_setparam( const int mode, void* hRtsp, char* szParams )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_RDS ) {
		return RTSPSetParam( hRtsp, szParams );
    }

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPSetParam( hRtsp, szParams );
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

long rtsplib_encodemime( int mode, void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, int nSrcLen )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_RDS ) {
		return RTSPEncodeMIME( hRtsp, pDst, (int*)pDstLen, pSrc, nSrcLen );
    }

    if ( mode == RTSP_MODE_WFD ) {
        return WFD_RTSPEncodeMIME( hRtsp, pDst, pDstLen, pSrc, nSrcLen );
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

/* @brief This method is only needed for the affinity code, which is
 * only supported on RDS. Thus the WFD version is unsupported.
 */
long rtsplib_setparamstruct( int mode, void* hRtsp, PARAMETER_STRUCT* pStruct )
{
    assert( hRtsp );

    if ( mode == RTSP_MODE_RDS ) {
        return RTSPSetParamStruct( hRtsp, pStruct ) ;
    }

    if ( mode == RTSP_MODE_WFD ) {
        CRITICAL( "unsupported for WFD, this should not have been called" );
        return 1;
    }

    CRITICAL( "unsupported RTSP library mode: %d", mode );
    return 1;
}

long rtsplib_error_code( const int mode, const char* label )
{
    assert( label );
    assert( mode == 1 || mode == 2 );

    struct node {
        char* label;
        long value[2]; // [0]=RDS, [1]=WFD
    } nodes[20] = {
        { "NO_ERROR",             { RTSP_NO_ERROR,             WFD_RTSP_NO_ERROR } },
        { "INSUFFICIENT_BUF",     { RTSP_INSUFFICIENT_BUF,     WFD_RTSP_INSUFFICIENT_BUF } },
        { "SOCKET_RECV_FAILED",   { RTSP_SOCKET_RECV_FAILED,   WFD_RTSP_SOCKET_RECV_FAILED } },
        { "SOCKET_RECV_TIMEOUT",  { RTSP_SOCKET_RECV_TIMEOUT,  WFD_RTSP_SOCKET_RECV_TIMEOUT } },
        { "SOCKET_SEND_FAILED",   { RTSP_SOCKET_SEND_FAILED,   WFD_RTSP_SOCKET_SEND_FAILED } },
        { "SOCKET_NOT_CONNECTED", { RTSP_SOCKET_NOT_CONNECTED, WFD_RTSP_SOCKET_NOT_CONNECTED } },
        { NULL,                   { 0, 0 } }
    };
    struct node *ptr = nodes;

    while ( ptr->label != NULL ) {
        if ( strcasecmp( label, ptr->label ) == 0 ) {
            return ptr->value[ mode-1 ];
        }
        ptr++;
    }

    CRITICAL( "should not get here - unknown label %s for mode %d", label, mode );
    return 1;
}

long rtsplib_notification_code( const int mode, const char* label )
{
    assert( label );

    struct node {
        int mode;
        char* label;
        long value;
    } nodes[] = {
        { RTSP_MODE_RDS, "PLAY",               RTSP_PLAY },
        { RTSP_MODE_RDS, "TEARDOWN",           RTSP_TEARDOWN },
        { RTSP_MODE_RDS, "TIMEOUT",            RTSP_TIMEOUT },
        { RTSP_MODE_RDS, "GET_PARAMETER",      RTSP_GET_PARAMETER },
        { RTSP_MODE_RDS, "GET_PARAMETER_RESP", RTSP_GET_PARAMETER_RESP },
        { RTSP_MODE_RDS, "SET_PARAMETER",      RTSP_SET_PARAMETER },
        { RTSP_MODE_WFD, "PLAY",               WFD_RTSP_PLAY },
        { RTSP_MODE_WFD, "TEARDOWN",           WFD_RTSP_TEARDOWN },
        { RTSP_MODE_WFD, "TIMEOUT",            WFD_RTSP_TIMEOUT },
        { RTSP_MODE_WFD, "GET_PARAMETER",      WFD_RTSP_GET_PARAMETER },
        { RTSP_MODE_WFD, "GET_PARAMETER_RESP", WFD_RTSP_GET_PARAMETER_RESP },
        { RTSP_MODE_WFD, "SET_PARAMETER",      WFD_RTSP_SET_PARAMETER },
        { RTSP_MODE_WFD, "PAUSE",              WFD_RTSP_PAUSE },
        { -1, NULL, 0 }
    }, *ptr = nodes;

    while ( ptr->label != NULL ) {
        int mode_matches = ptr->mode == mode;
        int label_matches = 0 == strcasecmp( label, ptr->label );
        if ( mode_matches && label_matches ) {
            return ptr->value;
        }
        ptr++;
    }

    CRITICAL( "unable to find match for mode=%d label=%s", mode, label );
    return -1;
}

int rtsplib_param_is_vendor_extension( const int mode, const unsigned long id )
{
    if ( mode == RTSP_MODE_WFD ) {
        return id >= CountOfWFDParams;
    }

    if ( mode == RTSP_MODE_RDS ) {
        return id >= CountOfRDSParams;
    }

    CRITICAL( "should not get here" );
    return 1;
}

char* rtsplib_param_name( const int mode, const unsigned long id )
{
    if ( rtsplib_param_is_vendor_extension( mode, id ) ) {
        DEBUG( "%s: id %ld is outside of range supported by RTSP library", __func__, id );
        return RTSPLIB_VENDOR_EXTENSION;
    }

    if ( mode == RTSP_MODE_WFD ) {
        if ( id < CountOfWFDParams ) {
            return g_szWFDParams[ id ];
        }
    }

    if ( mode == RTSP_MODE_RDS ) {
        if ( id < CountOfRDSParams ) {
            return g_szRDSParams[ id ];
        }
    }

    CRITICAL( "unknown mode %d, should never get here", mode );
    return RTSPLIB_VENDOR_EXTENSION;
}

char* rtsplib_notification_name( const int mode, const unsigned long id )
{
    if ( mode == RTSP_MODE_WFD ) {
        return g_szWFD_RTSPNotifications[ id ];
    } else if ( mode == RTSP_MODE_RDS ) {
        return g_szRDSRTSPNotifications[ id ];
    } else {
        CRITICAL( "unknown mode %d, should never get here", mode );
        assert( 0 );
        return "INVALID";
    }
}

#if 0
// These functions need to be abstracted; but the sample code isn't
// using them yet
long CALLING_CONVENTION rtsplib_srcinit( int mode, void** phRtsp, unsigned short usServerRtcpPort, int nOverscanComp, char* pOptRdsSetupParams, FN_RTSP_NOTIFICATIONS pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext );
long CALLING_CONVENTION rtsplib_srcupdateparams( int mode, void* hRtsp, int nOverscanComp, char* pOptRdsSetupParams);
long CALLING_CONVENTION rtsplib_srcaddress( int mode, void* phRtsp, char* szIpAddr);
long CALLING_CONVENTION rtsplib_setlegacy( int mode, void* hRtsp, int bLegacy);
long CALLING_CONVENTION rtsplib_getlegacy( int mode, void* hRtsp, int* pbLegacy);
long CALLING_CONVENTION rtsplib_resetincomingcseq( int mode, void* hRtsp);
long CALLING_CONVENTION rtsplib_getparam( int mode, void* hRtsp, char* szParams);
long CALLING_CONVENTION rtsplib_decodemime( int mode, void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen);
long CALLING_CONVENTION rtsplib_teardown( int mode, void* hRtsp);
long CALLING_CONVENTION rtsplib_setparambinary( int mode, void* hRtsp, RDSParams id, void* pData, long nDataLen);
#endif
