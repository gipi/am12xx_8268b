/**
 * @file: rtsp_abstraction.h
 * @brief Prototypes for functions exported by rtsp_abstraction.c.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010-2011 Intel Corporation All Rights Reserved.
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
 * delivery of the Materials,  either expressly, by implication, inducement,
 * estoppel or otherwise.  Any license under such intellectual property
 * rights must be express and approved by Intel in writing.
 */

#ifndef _RTSP_ABSTRACTION_H
#define _RTSP_ABSTRACTION_H

// {{{ Includes
#include <stdio.h>
#include <syslog.h>
#include <strings.h> // strcasecmp

#include "rtsp/rdsrtsp.h"
#include "rtspwfd/rtspwfd.h"
#include "rtspwfd/intelrtspwfd.h"
// }}}

#define RTSPLIB_VENDOR_EXTENSION "VENDOR_EXTENSION"

#define RTSP_MODE_RDS               1
#define RTSP_MODE_WFD               2

#define WFD_VENDOR_EXTENSION CountOfWFDParams

typedef long (*rtsp_fn_notification_callback)( int id, void* szValue, void* pContext );

long rtsplib_snkinit( int mode, void** phRtsp, unsigned short usRtpPort, rtsp_fn_notification_callback pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext);
long rtsplib_parser( int mode, void* hRtsp);
long rtsplib_shutdown( int mode, void* hRtsp);
long rtsplib_play( int mode, void* hRtsp );
long rtsplib_pause( int mode, void* hRtsp );
long rtsplib_teardown( int mode, void* hRtsp );
long rtsplib_setparam( int mode, void* hRtsp, char* szParams);
long rtsplib_getparam( int mode, void* hRtsp, char* szParams);
long rtsplib_setparamstruct( int mode, void* hRtsp, PARAMETER_STRUCT* pStruct);
long rtsplib_encodemime( int mode, void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, int nSrcLen);
long rtsplib_error_code( const int mode, const char* label );
char* rtsplib_param_name( const int mode, const unsigned long id );
char* rtsplib_notification_name( const int mode, const unsigned long id );
int rtsplib_param_is_vendor_extension( const int mode, const unsigned long id );
long rtsplib_notification_code( const int mode, const char* label );

#if 0
// These functions need to be abstracted; but the sample code isn't
// using them yet
long CALLING_CONVENTION rtsplib_srcinit( int mode, void** phRtsp, unsigned short usServerRtcpPort, int nOverscanComp, char* pOptRdsSetupParams, FN_RTSP_NOTIFICATIONS pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext);
long CALLING_CONVENTION rtsplib_srcupdateparams( int mode, void* hRtsp, int nOverscanComp, char* pOptRdsSetupParams);
long CALLING_CONVENTION rtsplib_srcaddress( int mode, void* phRtsp, char* szIpAddr);
long CALLING_CONVENTION rtsplib_setlegacy( int mode, void* hRtsp, int bLegacy);
long CALLING_CONVENTION rtsplib_getlegacy( int mode, void* hRtsp, int* pbLegacy);
long CALLING_CONVENTION rtsplib_setparambinary( int mode, void* hRtsp, RDSParams id, void* pData, long nDataLen);
long CALLING_CONVENTION rtsplib_teardown( int mode, void* hRtsp);
long CALLING_CONVENTION rtsplib_resetincomingcseq( int mode, void* hRtsp);
long CALLING_CONVENTION rtsplib_decodemime( int mode, void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen);
#endif

#endif
