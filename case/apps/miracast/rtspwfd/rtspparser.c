/**
 * @file: rtspparser.c
 * @brief Part of the RDS library.
 *
 * INTEL CONFIDENTIAL
 * Copyright 2010 Intel Corporation All Rights Reserved.
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

#include <assert.h>
#include "stdafx.h"
#include "wfdparser.h"
#include <unistd.h>
#define Declare_String
#include "method_d.h"
#define Declare_String
#include "general-header_d.h"
#define Declare_String
#include "request-header_d.h"
#define Declare_String
#include "response-header_d.h"
#define Declare_String
#include "entity-header_d.h"

#define WFD_RTSP_STATE_TRANSITION_TIMEOUT	6000
#define WFD_RTSP_STATE_EXTRA_TRANSITION_TIMEOUT	9000
#define WIFI_DIRECT_JSON 1

char g_cAtoH[255];

int wfd_snprintf_(char *buffer, size_t sizeOfBuffer, const char *format, ...)
{
	va_list ap;
	int ret;

	va_start(ap, format);
#ifdef WIN32
	ret = _vsnprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, ap);
#elif _POSIX_SOURCE
	ret = vsnprintf(buffer, sizeOfBuffer, format, ap);
#else
#error Please implement _snprintf for this platform
#endif
	va_end(ap);

	return ret;
}

void WFD_RTSP_LOG(CRTSPParser* p, int nLevel, char* szFmt,...)
{
	char buffer[1024];
	va_list ap;
	int nRet=0;

	va_start(ap, szFmt);
#ifdef WIN32
	nRet = _vsnprintf_s(buffer, 1024, _TRUNCATE, szFmt, ap);
#elif _POSIX_SOURCE
	nRet = vsnprintf(buffer, 1024, szFmt, ap);
#else
#error Please implement _snprintf for this platform
#endif
	va_end(ap);

	if (0 < nRet && p->m_pLog)
		p->m_pLog(nLevel, buffer, p->m_pContext);
}

CRTSPParser* WFD_RTSP_Alloc(BOOL bServer)
{
	CRTSPParser* p = calloc(1, sizeof(CRTSPParser));
	if (!p)
		return NULL;

	p->m_bServer = bServer;
	p->m_nSendCSeq = 1;
	p->m_usServerRtcpPort = 0;

	memset(p->m_szSession, 0, WFD_RTSP_SESSION_ID_LEN+1);

	if (bServer) {
		p->m_bMySupportedMethods[MGET_PARAMETER] = 
		p->m_bMySupportedMethods[MPLAY] = 
		p->m_bMySupportedMethods[MPAUSE] = 
		p->m_bMySupportedMethods[MSETUP] = 
		p->m_bMySupportedMethods[MSET_PARAMETER] = 
		p->m_bMySupportedMethods[MTEARDOWN] = TRUE;
	} else {
		p->m_bMySupportedMethods[MGET_PARAMETER] = 
		p->m_bMySupportedMethods[MSET_PARAMETER] = TRUE;
	}
	return p;
}

void WFD_RTSP_Free(CRTSPParser* p)
{
	if (p) {
		WFD_QueueFree(&p->m_bqSent, p);
		WFD_QueueFree(&p->m_bqToSend, p);

		free(p);
	}
}

long WFD_GetBufferOrCheckWhetherToBail(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;

	if (0 < p->nToken || 2 <= p->nStrikes) {
		if (p->m_nLineLen) {
			p->m_nLineLen = 0;
		}

		lRet = WFDReadNextLine(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			return lRet;

		p->nStrikes = 0;
		if (0 == p->m_nLineLen)
			return lRet; // empty line, no error but need to check on return

		p->pToken = strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext));

	} else
		p->nStrikes++;

	return lRet;
}

static long GetNext(CRTSPParser* p, char* pchar)
{
	long lRet = WFD_RTSP_NO_ERROR;
	long nBytesRead=0;

	if (p->m_bUnget) {

		*pchar = p->m_cUngetChar;
		p->m_bUnget = FALSE;

		return WFD_RTSP_NO_ERROR;

	} else if (p->m_nBytesRead == p->m_nPos) {

		// need to read more
		long nTimeout=30000;

		do {
			if (p->m_bRtspConnected) {
				// keepalive timeout logic
	
				if (p->m_bServer) {

					unsigned long CurTime = p->m_pGetTick(p->m_pContext);
					WFD_RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - CurTime: %ld - m_nLastKeepalive: %ld = %ld, m_nKeepalivePeriod: %ld", __FUNCTION__, CurTime, p->m_nLastKeepalive, (long)(CurTime - p->m_nLastKeepalive), p->m_nKeepalivePeriod);

					if (((long)(CurTime - p->m_nLastKeepalive) >= p->m_nKeepalivePeriod - RESPONSE_TIMEOUT)) {

						// if we're a server and we're connected and 60 seconds has passed, we need to send out a keepalive message

						WFD_RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - Send Keepalive NOW - CurTime: %ld, m_nLastKeepalive: %ld = %ld, m_nKeepalivePeriod: %ld", __FUNCTION__, CurTime, p->m_nLastKeepalive, (long)(CurTime - p->m_nLastKeepalive), p->m_nKeepalivePeriod);
						WFD_RTSPGetParam(p, "");
						p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);
				
					} else {
						nTimeout = p->m_nKeepalivePeriod - (CurTime - p->m_nLastKeepalive) - RESPONSE_TIMEOUT;
					}

				} else {
					unsigned long t=p->m_pGetTick(p->m_pContext);
				#if 0
					// add 10000 msec to make the keepalive timout 40 seconds on the snk side because the 30 sec is too tight
					if (((long)(t - p->m_nLastKeepalive) > (p->m_nKeepalivePeriod + 10000))) {

						// we're a client/sink, so if we haven't gotten a keepalive, we need to end the session
						WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - snk keepalive timeout, curr:%d, last:%d, delta:%d, keepalive period:%d", __FUNCTION__, 
							t, p->m_nLastKeepalive, (t- p->m_nLastKeepalive), p->m_nKeepalivePeriod);
				
						WFD_RTSPTeardown(p);

						return WFD_RTSP_KEEPALIVE_TIMEOUT;
				
					} else {
						// add 10000 msec to make the keepalive timout 40 seconds on the snk side because the 30 sec is too tight
						nTimeout = (p->m_nKeepalivePeriod+10000) - (p->m_pGetTick(p->m_pContext) - p->m_nLastKeepalive);
					}
				#else
					nTimeout = (p->m_nKeepalivePeriod+10000) - (p->m_pGetTick(p->m_pContext) - p->m_nLastKeepalive);
				#endif
				}
			}

			if (p->m_bServer)
				nTimeout = MIN(RESPONSE_TIMEOUT, nTimeout);

			if (0 < p->m_bqSent.m_nBuffersQueued) {
				// if we are waiting for a response for a sent packet, set the recv timeout to expire 5 seconds after the packet was sent
				nTimeout = MIN((long)(p->m_nResponseTimeout - (p->m_pGetTick(p->m_pContext) - p->m_bqSent.m_pHead->nTimeSent)), nTimeout);
				if (0 > nTimeout) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for response from peer", __FUNCTION__);
					p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
					nTimeout = 5000; // we don't want a negative number
				}
			} else {
			
				// override timeouts in these cases
				if (!p->m_bServer && WFD_RTSP_M00 == p->m_eState) {
					nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M1
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (OPTIONS) from source", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				} else if (p->m_bServer && WFD_RTSP_M01 == p->m_eState) {
					nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // source and waiting for M2
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (OPTIONS) from sink", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				} else if (!p->m_bServer && WFD_RTSP_M02 == p->m_eState) {
					nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M3
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (GET_PARAMETER) from source", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				} else if (!p->m_bServer && WFD_RTSP_M03 == p->m_eState) {
					nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M4
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (SET_PARAMETER) from source", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				} else if (!p->m_bServer && WFD_RTSP_M04 == p->m_eState) {
					nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M5
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (SETUP trigger) from source", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				} else if (p->m_bServer && !p->m_bRtspConnected && WFD_RTSP_M05 == p->m_eState) {
					nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M6
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (SETUP) from sink", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				} else if (p->m_bServer && WFD_RTSP_M06 == p->m_eState) {
					if (p->m_bHDCP)
						nTimeout = WFD_RTSP_STATE_EXTRA_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M7
					else
						nTimeout = WFD_RTSP_STATE_TRANSITION_TIMEOUT - (p->m_pGetTick(p->m_pContext) - p->m_ulStartTime[p->m_eState]); // sink and waiting for M7
					
					if (0 > nTimeout) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M%d (PLAY) from sink", __FUNCTION__, p->m_eState+1);
						p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
						return WFD_RTSP_SOCKET_RECV_TIMEOUT;
					}
				}

			}

            assert( p->m_pRecv );
			lRet = p->m_pRecv(p->m_pReadBuffer, RTSP_RECV_BUFFER, &nBytesRead, nTimeout, p->m_pContext);
		
			//if (WFD_RTSP_SOCKET_RECV_TIMEOUT == lRet) {
			//	if (!p->m_bServer && WFD_RTSP_M00 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M1 (OPTIONS) from source", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	} else if (p->m_bServer && WFD_RTSP_M01 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M2 (OPTIONS) from sink", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	} else if (!p->m_bServer && WFD_RTSP_M02 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M3 (GET_PARAMETER) from source", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	} else if (!p->m_bServer && WFD_RTSP_M03 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M4 (SET_PARAMETER) from source", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	} else if (!p->m_bServer && WFD_RTSP_M04 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M5 (SETUP trigger) from source", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	} else if (p->m_bServer && WFD_RTSP_M05 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M6 (SETUP) from sink", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	} else if (p->m_bServer && WFD_RTSP_M06 == p->m_eState) {
			//		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for M6 (PLAY) from sink", __FUNCTION__);
			//		p->m_pNotify(WFD_RTSP_TIMEOUT, NULL, p->m_pContext);
			//		return lRet;
			//	}
			//}

		} while (WFD_RTSP_SOCKET_RECV_TIMEOUT == lRet);

		if (WFD_RTSP_NO_ERROR != lRet)
			return lRet;

		p->m_nBytesRead = nBytesRead;
		p->m_nPos = 0;
	}

	*pchar = p->m_pReadBuffer[p->m_nPos++];

	return WFD_RTSP_NO_ERROR;
}

static void UnGet(CRTSPParser* p, char c)
{
	p->m_cUngetChar = c;
	p->m_bUnget = TRUE;
}

long WFDReadNextLine(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	long nPos=0;
	char c, c2;

	do {
		lRet = GetNext(p, &c);
		if (WFD_RTSP_NO_ERROR != lRet)
			break;

		if (0 == c) {
			p->m_pInputBuffer[nPos] = 0;
			p->m_nLineLen = nPos;

			// legacy, treat null char as eol/end of message
			// WFD_RTSP_NO_ERROR
			break;
		}

		if ('\r' == c || '\n' == c) {

			switch (p->m_eolType) {
			case EOL_CR:
				if ('\r' != c) {
					lRet = WFD_RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

			case EOL_LF:
				if ('\n' != c) {
					lRet = WFD_RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

			case EOL_CRLF:
				if ('\r' != c) {
					lRet = WFD_RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				lRet = GetNext(p, &c2);
				if (WFD_RTSP_NO_ERROR != lRet)
					return lRet;

				if ('\n' != c2) {
					lRet = WFD_RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

			case EOL_LFCR:
				if ('\n' != c) {
					lRet = WFD_RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				lRet = GetNext(p, &c2);
				if (WFD_RTSP_NO_ERROR != lRet)
					goto cleanup;

				if ('\r' != c2) {
					lRet = WFD_RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

            case EOL_UNKNOWN:
            case CountOfWFD_EOLTypes:

				lRet = GetNext(p, &c2);
				if (WFD_RTSP_NO_ERROR != lRet)
					goto cleanup;

				if ('\r' == c && '\n' == c2)
					p->m_eolType = EOL_CRLF;
				else if ('\n' == c && '\r' == c2)
					p->m_eolType = EOL_LFCR;
				else if ('\r' == c) {
					p->m_eolType = EOL_CR;
					UnGet(p, c2);
				} else {
					p->m_eolType = EOL_LF;
					UnGet(p, c2);
				}
                break;
			}

			p->m_pInputBuffer[nPos] = 0;
			p->m_nLineLen = nPos;

			// WFD_RTSP_NO_ERROR
			break;

		}

		if (RTSP_MAX_INPUT_BUFFER-1 <= nPos) {
			lRet = WFD_RTSP_INSUFFICIENT_BUF;
			break;
		}

		// add character and go back for another
		p->m_pInputBuffer[nPos++] = c;

	} while (TRUE);

cleanup:

	return lRet;
}

long WFD_RTSP_SetSrcAddress(CRTSPParser* p, char* szIpAddr)
{
	if (szIpAddr)
		strncpy(p->m_szIpAddress, szIpAddr, RTSP_MAX_IPADDR_BUFFER);
	else
		strncpy(p->m_szIpAddress, "0.0.0.0", RTSP_MAX_IPADDR_BUFFER);

	return WFD_RTSP_NO_ERROR;
}

long WFD_RTSP_Init(CRTSPParser* p, unsigned short usRtpPort, unsigned short usServerRtcpPort, unsigned long ulKeepaliveTimeout, char* pInitialGetParams, FN_WFD_RTSP_NOTIFICATIONS pfnCallback, FN_WFD_RTSP_RECV pfnRecv, FN_WFD_RTSP_SEND pfnSend, FN_WFD_RTSP_SLEEP pfnSleep, FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, FN_WFD_RTSP_LOG pfnLog, void* pContext)
{
	int nLen;

	g_cAtoH['0'] = 0;
	g_cAtoH['1'] = 1;
	g_cAtoH['2'] = 2;
	g_cAtoH['3'] = 3;
	g_cAtoH['4'] = 4;
	g_cAtoH['5'] = 5;
	g_cAtoH['6'] = 6;
	g_cAtoH['7'] = 7;
	g_cAtoH['8'] = 8;
	g_cAtoH['9'] = 9;
	g_cAtoH['A'] = 10;
	g_cAtoH['a'] = 10;
	g_cAtoH['B'] = 11;
	g_cAtoH['b'] = 11;
	g_cAtoH['C'] = 12;
	g_cAtoH['c'] = 12;
	g_cAtoH['D'] = 13;
	g_cAtoH['d'] = 13;
	g_cAtoH['E'] = 14;
	g_cAtoH['e'] = 14;
	g_cAtoH['F'] = 15;
	g_cAtoH['f'] = 15;

	// specifies which wfd commands are supported in this implementation
	p->m_bSupportedWFDCommands[wfd_audio_codecs] = TRUE;
	p->m_bSupportedWFDCommands[wfd_video_formats] = TRUE;
	p->m_bSupportedWFDCommands[wfd_3d_video_formats] = TRUE;
	p->m_bSupportedWFDCommands[wfd_content_protection] = TRUE;
	p->m_bSupportedWFDCommands[wfd_display_edid] = TRUE;
	p->m_bSupportedWFDCommands[wfd_coupled_sink] = TRUE;
	p->m_bSupportedWFDCommands[wfd_trigger_method] = TRUE;
	p->m_bSupportedWFDCommands[wfd_presentation_URL] = TRUE;
	p->m_bSupportedWFDCommands[wfd_client_rtp_ports] = TRUE;
	p->m_bSupportedWFDCommands[wfd_route] = TRUE;
	p->m_bSupportedWFDCommands[wfd_I2C] = TRUE;
	p->m_bSupportedWFDCommands[wfd_av_format_change_timing] = TRUE;
	p->m_bSupportedWFDCommands[wfd_preferred_display_mode] = TRUE;
	p->m_bSupportedWFDCommands[wfd_uibc_capability] = TRUE;
	p->m_bSupportedWFDCommands[wfd_uibc_setting] = TRUE;
	p->m_bSupportedWFDCommands[wfd_standby_resume_capability] = TRUE;
	p->m_bSupportedWFDCommands[wfd_standby] = TRUE;
	p->m_bSupportedWFDCommands[wfd_connector_type] = TRUE;
	p->m_bSupportedWFDCommands[wfd_idr_request] = TRUE;

	p->m_pNotify = pfnCallback;
	p->m_pRecv = pfnRecv;
	p->m_pSend = pfnSend;
	p->m_pSleep = pfnSleep;
	p->m_pGetTick = pfnGetTick;
	p->m_pLog = pfnLog;
	p->m_pContext = pContext;
	p->m_bqSent.m_pSleep = pfnSleep;
	p->m_bqToSend.m_pSleep = pfnSleep;
	if (p->m_bServer)
		p->m_usServerRtcpPort = usRtpPort;
	else
		p->m_usRTPPort = usRtpPort;
	p->m_usServerRtcpPort = usServerRtcpPort;
	p->m_nKeepalivePeriod = ulKeepaliveTimeout;
	p->m_nResponseTimeout = RESPONSE_TIMEOUT;

	nLen = pInitialGetParams?strlen(pInitialGetParams):0;
	if (nLen) {
		p->m_pInitialGetParams = malloc(nLen+1);
		if (p->m_pInitialGetParams)
			memcpy(p->m_pInitialGetParams, pInitialGetParams, nLen+1);
	} else {
		p->m_pInitialGetParams = malloc(1);
		if (p->m_pInitialGetParams)
			p->m_pInitialGetParams[0] = 0;
	}

	if (!WFD_QueueAllocate(&p->m_bqToSend, p, RTSP_SEND_BUFFER, RTSP_SEND_COUNT)) {
		// failed to allocate send buffers
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - WFD_QueueAllocate(%d, %d) failed", __FUNCTION__, RTSP_SEND_BUFFER, RTSP_SEND_COUNT);
		return WFD_RTSP_MEMORY;
	}

	p->m_pReadBuffer = calloc(1, RTSP_RECV_BUFFER);
	if (!p->m_pReadBuffer) {
		// failed to allocate input buffer
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - recv buffer allocation of %d bytes failed", __FUNCTION__, RTSP_RECV_BUFFER);
		return WFD_RTSP_MEMORY;
	}

	p->m_pInputBuffer = calloc(1, RTSP_MAX_INPUT_BUFFER);
	if (!p->m_pInputBuffer) {
		// failed to allocate input buffer
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - input buffer allocation of %d bytes failed", __FUNCTION__, RTSP_MAX_INPUT_BUFFER);
		return WFD_RTSP_MEMORY;
	}

	p->m_pSendBuffer = calloc(1, RTSP_MAX_OUTPUT_BUFFER);
	if (!p->m_pSendBuffer) {
		// failed to allocate work buffer
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - input buffer allocation of %d bytes failed", __FUNCTION__, RTSP_MAX_OUTPUT_BUFFER);
		return WFD_RTSP_MEMORY;
	}

	p->m_pOutputBuffer = calloc(1, RTSP_MAX_OUTPUT_BUFFER);
	if (!p->m_pOutputBuffer) {
		// failed to allocate output buffer
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - input buffer allocation of %d bytes failed", __FUNCTION__, RTSP_MAX_OUTPUT_BUFFER);
		return WFD_RTSP_MEMORY;
	}

	p->pParsed = calloc(1, sizeof(WFD_NODE) + RTSP_MAX_INPUT_BUFFER);
	if (!p->pParsed) {
		// failed to allocate parsed buffer
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - parsed buffer allocation of %d bytes failed", __FUNCTION__, sizeof(WFD_NODE) + RTSP_MAX_INPUT_BUFFER);
		return WFD_RTSP_MEMORY;
	}
	p->pParsed->nAllocated = RTSP_MAX_INPUT_BUFFER;
	p->pParsed->nUsed = 0;

	return WFD_RTSP_NO_ERROR;
}

long WFD_RTSP_SetParam(CRTSPParser* p, char* szParams)
{
	if (0 == strcmp(szParams, "wfd_standby"))
		p->m_bStandbyMode = TRUE;
	return WFD_SendSetParameters(p, szParams);
}

long WFD_RTSP_GetParam(CRTSPParser* p, char* szParams)
{
	return WFD_SendGetParameters(p, szParams);
}

long WFD_RTSP_SetParamBinary(CRTSPParser* p, WFDParams id, void* pData, long nDataLen)
{
	long lRet = WFD_RTSP_NO_ERROR;
	WFD_PARAMETER_STRUCT ps;
	long lLen=0;

	if (CountOfWFDParams <= id)
		return WFD_RTSP_INVALID_ID;

	memset(&ps, 0, sizeof(WFD_PARAMETER_STRUCT));

	ps.nId = id;

	lRet = WFD_RTSPEncodeMIME(p, NULL, &lLen, (unsigned char*)pData, nDataLen);
	if (WFD_RTSP_NO_ERROR != lRet)
		return lRet;

	ps.szValue = (char*)calloc(lLen+1, 1);
	lRet = WFD_RTSPEncodeMIME(p, ps.szValue, &lLen, (unsigned char*)pData, nDataLen);
	if (WFD_RTSP_NO_ERROR != lRet) {
		free(ps.szValue);
		return lRet;
	}

	ps.nCount = lLen;

	lRet = WFD_RTSP_SetParamStruct(p, &ps, 1);

	free(ps.szValue);
	return lRet;
}

long WFD_RTSP_SetParamStruct(CRTSPParser* p, WFD_PARAMETER_STRUCT* pStruct, int nCount)
{
	long lRet = WFD_RTSP_NO_ERROR;
	int nLen=0;
	int i;

	p->m_nOutputBufferUsed = 0;

	for (i=0; i < nCount; i++) {
		if (pStruct[i].nId > CountOfWFDParams) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Invalid wfd_ command id (%d) in structure element[zero based %d of %d]", __FUNCTION__, pStruct[i].nId, i, nCount);
			return WFD_RTSP_INTERNAL_ERROR;
		} else if (pStruct[i].nId == CountOfWFDParams && pStruct[i].szValue && pStruct[i].szValue[0]) {
			// this is a proprietary command
			lRet = WFD_BuildPayloadFromStruct(p, pStruct[i].szParam, &pStruct[i]);
			if (WFD_RTSP_NO_ERROR != lRet) {
				return lRet;
			}
		} else {
			lRet = WFD_BuildPayloadFromStruct(p, g_szWFDParams[pStruct[i].nId], &pStruct[i]);
			if (WFD_RTSP_NO_ERROR != lRet) {
				return lRet;
			}
		}
	}

	lRet = WFD_SendSetParameters(p, (char*)p->m_pOutputBuffer);
	if (WFD_RTSP_NO_ERROR != lRet) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendGetParameters(%s)", __FUNCTION__, lRet, p->m_pOutputBuffer);
		return WFD_RTSP_INTERNAL_ERROR;
	}

	return lRet;
}

long WFD_RTSP_Shutdown(CRTSPParser* p)
{
	if (p->m_bInParser)
		return WFD_RTSP_PARSER_ACTIVE; // this means the socket must be closed first, so the parser can exit

	if (p->m_pInitialGetParams) {
		free(p->m_pInitialGetParams);
		p->m_pInitialGetParams = NULL;
	}
	if (p->m_pReadBuffer) {
		free(p->m_pReadBuffer);
		p->m_pReadBuffer = NULL;
	}
	if (p->m_pSendBuffer) {
		free(p->m_pSendBuffer);
		p->m_pSendBuffer = NULL;
	}
	if (p->m_pInputBuffer) {
		free(p->m_pInputBuffer);
		p->m_pInputBuffer = NULL;
	}
	if (p->m_pOutputBuffer) {
		free(p->m_pOutputBuffer);
		p->m_pOutputBuffer = NULL;
	}
	if (p->pParsed) {
		free(p->pParsed);
		p->pParsed = NULL;
	}

	return WFD_RTSP_NO_ERROR;
}

long WFD_RTSP_State(CRTSPParser* p)
{
	if(p == NULL){
		return -1;
	}

	return p->m_eState;
}


long WFD_RTSP_Parser(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	BOOL bResponse = FALSE;

	// this is stuff to initialize at the start of each connection
	p->m_bInParser = TRUE;
	p->pToken=NULL;
	p->nMethod=CountOfWFD_MethodTokens;
	p->m_eolType = EOL_UNKNOWN;
	p->m_nRecvCSeq = -1;
	p->m_bRtspConnected = FALSE;
	p->m_bPreferredDisplayModeSupported = FALSE;
	p->m_bPreferredDisplayModeSent = FALSE;
	p->m_bStandbyMode = FALSE;
	memset(p->m_szSession, 0, WFD_RTSP_SESSION_ID_LEN+1);
	memset(p->m_ulStartTime, 0, sizeof(p->m_ulStartTime));
	p->m_bHDCP = FALSE;
	p->m_eState = WFD_RTSP_M00;
	p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

	if (p->m_bServer) {
		// a client is connected, so send it options
		lRet = WFD_SendOptions(p);

		if (WFD_RTSP_NO_ERROR != lRet)
			goto send_error;

		p->m_eState = WFD_RTSP_M01;
		p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);
	}

top:
	do {
		p->pBuf = NULL;
		p->nCSeq=-1;

		lRet = WFDReadNextLine(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto end_thread; // end thread

		// got a buffer, start parsing it
		p->m_nContentLength = 0;
		bResponse = FALSE;
		p->m_nRTSPReponseCode = 200;
		p->m_szErrorMessage[0] = 0;

		if (!strncmp((char*)p->m_pInputBuffer, "RTSP/", 5)) {
			// this is a response
			bResponse = TRUE;
			lRet = WFD_ParseMessageResponse(p);

			// this code if applicable will prompt the app to send the M4 message in the case that preferred display mode is supported
			if (p->m_nCSeqAfterwhichToGoToM4 == p->nCSeq && p->m_bServer) {

				// if preferred display mode is supported, and we haven't requested it, request it
				if (p->m_bPreferredDisplayModeSupported && p->m_bPreferredDisplayModeSent && 200 == p->m_nRTSPReponseCode) {
				
					// this prompts the app to send the M4 message for preferred display mode
					p->m_eState = WFD_RTSP_M04;
					p->m_pNotify(WFD_RTSP_PREF_DISP_CAP_EXCHANGE, 0, p->m_pContext);

				}
			}

			if (WFD_RTSP_NO_ERROR != lRet)
				break;

		} else {
            // Request      =       Request-Line          ; Section 6.1
            //              *(      general-header        ; Section 5
            //              |       request-header        ; Section 6.2
            //              |       entity-header )       ; Section 8.1
            //                      CRLF
            //                      [ message-body ]      ; Section 4.3

			p->pToken=strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext));

			p->nToken=-1;
			p->nMethod = WFD_MatchToken(g_szWFD_MethodTokens, CountOfWFD_MethodTokens, p->pToken);
			//if (!p->m_bServer)
			//	p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);

			if (0 <= p->nMethod) {
				// first make sure this is an RTSP method we handle
				if (MOPTIONS != p->nMethod && !p->m_bMySupportedMethods[p->nMethod]) {
					// we don't support this method
					p->m_nRTSPReponseCode = 405;
					wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Method (%s) is not one of the methods allowed as specified in the OPTIONS response");
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				}

				// Don't accept messages other than M7, M8 or M12 while in standby mode
				if (p->m_bStandbyMode && !(MPLAY == p->nMethod || MSET_PARAMETER == p->nMethod || MTEARDOWN == p->nMethod ))
				{
					// means "not acceptable"
					p->m_nRTSPReponseCode = 406;
					wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "in-standby-mode", p->pToken);
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				}
				// Request-Line = Method SP Request-URI SP RTSP-Version CRLF   ; Section 6.1

				p->pToken=strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // Request-URI
				if (p->pToken) {
					if (MOPTIONS == p->nMethod) {
						if (strncmp("*", p->pToken, 1)) {
							p->m_nRTSPReponseCode = 400;
							wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "OPTION method requires *, got (%s) instead", p->pToken);
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						}
					} else {
						if (strncmp("rtsp://", p->pToken, 7)) {
							p->m_nRTSPReponseCode = 400;
							wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Request-Line requires Request-URI, got (%s) instead", p->pToken);
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						}
					}
				} else {
					p->m_nRTSPReponseCode = 400;
					wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Request-Line requires Request-URI but none was found");
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				}

				p->pToken=strtok_s(NULL,  "\r\n", &(p->m_pStrtokContext)); // RTSP-Version
				if (p->pToken) {
					if (strncmp("RTSP/", p->pToken, 5)) {
						p->m_nRTSPReponseCode = 400;
						wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Request-Line requires RTSP/1.0, got (%s) instead", p->pToken);
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
					} else
						p->m_dServerVersion = atof((char*)p->pToken+5); // RTSP/m.n
				} else {
					p->m_nRTSPReponseCode = 400;
					wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Request-Line requires RTSP/1.0 but none was found");
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				}


				// Finished parsing message header

				lRet = WFDReadNextLine(p);
				if (WFD_RTSP_NO_ERROR != lRet)
					goto end_thread; // end thread

				p->pToken = strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext));

				p->nStrikes=0;

				// next there should be 0 or more general-header|request-header|entity-header
				do {
					// we will bail out of this loop when we get an empty line
					if (0 == p->m_nLineLen)
						break;

					// first check for general-headers
					lRet = WFD_CheckForGeneralHeader(p);
					if (WFD_RTSP_NO_ERROR != lRet)
						goto send_error;

					// make sure CSeq is greater than last one received
					if (GCSeq == p->nToken) {
						if ((unsigned long)-1 == p->nCSeq) {
							p->m_nRecvCSeq = p->nCSeq;
						} else if (p->m_nRecvCSeq >= p->nCSeq) {
							p->m_nRTSPReponseCode = 400;
							wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected CSeq: greater than %d, but got: %d instead", p->m_nRecvCSeq, p->nCSeq);
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						} else
							p->m_nRecvCSeq = p->nCSeq;
					}

					lRet = WFD_GetBufferOrCheckWhetherToBail(p);
					if (WFD_RTSP_NO_ERROR != lRet)
						goto end_thread;
					else if (0 == p->m_nLineLen)
						break;

					// next check for request-headers
					lRet = WFD_CheckForRequestHeader(p);
					if (WFD_RTSP_NO_ERROR != lRet)
						goto send_error;

					lRet = WFD_GetBufferOrCheckWhetherToBail(p);
					if (WFD_RTSP_NO_ERROR != lRet)
						goto end_thread;
					else if (0 == p->m_nLineLen)
						break;

					// next check for entity-headers
					lRet = WFD_CheckForEntityHeader(p);
					if (WFD_RTSP_NO_ERROR != lRet)
						goto send_error;

					lRet = WFD_GetBufferOrCheckWhetherToBail(p);
					if (WFD_RTSP_NO_ERROR != lRet)
						goto end_thread;
					else if (0 == p->m_nLineLen)
						break;

					/** for OPTIONS we need the server information */
					if (MOPTIONS == p->nMethod) {
						lRet = WFD_CheckForResponseHeader(p);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto send_error;

						lRet = WFD_GetBufferOrCheckWhetherToBail(p);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto end_thread;
						else if (0 == p->m_nLineLen)
							break;
					}

				} while (TRUE);

				p->m_nLineLen = 0;
			
				if (WFD_RTSP_NO_ERROR == lRet && 200 == p->m_nRTSPReponseCode) {

					switch (p->nMethod) {
					case MDESCRIBE:              // Section 10.2
					case MANNOUNCE:              // Section 10.3
						p->m_nRTSPReponseCode = 501;
						wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "RTSP message (%s) not implemented", g_szWFD_MethodTokens[p->nMethod]);
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						break;
					case MGET_PARAMETER:         // Section 10.8
						if (!p->m_bServer && 0 == p->m_nContentLength) {
							// sink receiving keepalive
							p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);
						}

						if (!p->m_bServer && WFD_RTSP_M02 == p->m_eState) {
							// if we're a client in M2 state, then this is the transistion to M3
							p->m_eState = WFD_RTSP_M03;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

							WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from source in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
						}
						break;
					case MOPTIONS:               // Section 10.1
						if (p->m_bServer) {
							p->m_eState = WFD_RTSP_M02;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

							WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from sink in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
						} else {
							p->m_eState = WFD_RTSP_M01;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

							WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from source in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
						}

						lRet = WFD_SendOptionsResponse(p, p->nCSeq);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto send_error;

						if (!p->m_bServer) {
							// if this is the client, we need to send out our options
							lRet = WFD_SendOptions(p);
							if (WFD_RTSP_NO_ERROR != lRet)
								goto send_error;

							p->m_eState = WFD_RTSP_M02;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);
						}

						if (p->m_bServer) {
							// if this is the server, after we get client options, we send GET_PARAMETERS
							lRet = WFD_SendGetParameters(p, p->m_pInitialGetParams);
							if (WFD_RTSP_NO_ERROR != lRet)
								goto send_error;

							p->m_eState = WFD_RTSP_M03;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

							// we save this because when we get this reponse, we send the setup trigger
							p->m_nCSeqAfterwhichToGoToM4 = p->m_nSendCSeq-1;
						}
						break;
					case MPAUSE:                 // Section 10.6
						lRet = WFD_SendResponse(p, p->nCSeq, 200, "OK", NULL, NULL);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto send_error;

						lRet = p->m_pNotify(WFD_RTSP_PAUSE, NULL, p->m_pContext);
						if (WFD_RTSP_NO_ERROR != lRet) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto send_error;
						}
						break;
					case MPLAY:                  // Section 10.5
						if (p->m_bServer) {
							p->m_eState = WFD_RTSP_M07;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);
							p->m_bStandbyMode = FALSE;

							WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from sink in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
						}

						lRet = WFD_SendPlayResponse(p, p->nCSeq);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto send_error;

						//p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);
						p->m_bRtspConnected = TRUE;
						memcpy(p->m_playStruct.m_szSession, p->m_szSession, sizeof(p->m_playStruct.m_szSession));

						lRet = p->m_pNotify(WFD_RTSP_PLAY, &p->m_playStruct, p->m_pContext);
						if (WFD_RTSP_NO_ERROR != lRet) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto send_error;
						}

						break;
					case MRECORD:                // Section 10.11
					case MREDIRECT:              // Section 10.10
						p->m_nRTSPReponseCode = 501;
						wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "RTSP message (%s) not implemented", g_szWFD_MethodTokens[p->nMethod]);
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						break;
					case MSETUP:                 // Section 10.4
						if (p->m_bServer) {
							p->m_eState = WFD_RTSP_M06;
							p->m_nLastKeepalive = p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

							WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from sink in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
						}

						lRet = WFD_SendSetupResponse(p, p->nCSeq);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto send_error;

						lRet = p->m_pNotify(WFD_RTSP_SETUP, NULL, p->m_pContext);
						if (WFD_RTSP_NO_ERROR != lRet) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto send_error;
						}
						break;
					case MSET_PARAMETER:         // Section 10.9
						//WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - Set param", __FUNCTION__);
						if (!p->m_bServer && WFD_RTSP_M03 == p->m_eState) {
							// if we're a client in M3 state, then this is the transistion to M4
							p->m_eState = WFD_RTSP_M04;
							p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

							WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from source in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
						}
						break;
					case MTEARDOWN:              // Section 10.7
						
						p->m_bRtspConnected = FALSE;

						lRet = WFD_SendTeardownResponse(p, p->nCSeq);
						if (WFD_RTSP_NO_ERROR != lRet)
							goto send_error;

						p->m_bRtspConnected = FALSE;

						lRet = p->m_pNotify(WFD_RTSP_TEARDOWN, 0, p->m_pContext);
						if (WFD_RTSP_NO_ERROR != lRet) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto send_error;
						}
						break;
					}

					if (WFD_RTSP_NO_ERROR != lRet)
						goto send_error;
				}
			}
		}

		if (p->m_nContentLength && WFD_RTSP_NO_ERROR == lRet && 200 == p->m_nRTSPReponseCode) {
			// get content
			if (10*1024*1024 < p->m_nContentLength && p->m_bServer) {
				// arbitrarily limit wfd content length to 10 MB to satisfy Klokworks
				
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Content length of %d is too large to handle on device", p->m_nContentLength);
				p->m_nRTSPReponseCode = 250;
				lRet = WFD_RTSP_MEMORY;
				break;
			}

			lRet = WFD_GetContents(p, p->m_nContentLength);
			if (WFD_RTSP_NO_ERROR != lRet)
				break;

			p->pParsed->nCSeq = p->nCSeq;
			p->pParsed->nMethod = (int)p->nMethod;

			lRet = WFD_Parser(p);
			if (WFD_RTSP_NO_ERROR != lRet)
				break;
		} else if (0 == p->m_nContentLength) {
			if (MGET_PARAMETER == p->nMethod) {
				lRet = WFD_SendGetParametersResponse(p, p->nCSeq, (char*)"");
				if (WFD_RTSP_NO_ERROR != lRet) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendGetParametersResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq, p->m_pOutputBuffer);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto end_thread;
				}
			} else if (MSET_PARAMETER == p->nMethod) {
				lRet = WFD_SendSetParametersResponse(p, p->nCSeq);
				if (WFD_RTSP_NO_ERROR != lRet) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendSetParametersResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq, p->m_pOutputBuffer);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto end_thread;
				}
			}
		}

	} while (200 == p->m_nRTSPReponseCode);

send_error:

	if (WFD_RTSP_NO_ERROR != lRet || 200 != p->m_nRTSPReponseCode) {

		if (!bResponse && WFD_RTSP_SOCKET_RECV_FAILED != lRet) {
			// if this was not a response to one of our messages, and the socket wasn't closed, send an error response
			if (WFD_RTSP_UNEXPECTED_EOLCHAR == lRet)

				WFD_SendResponse(p, p->nCSeq, 400, "Inconsistent use of CRLF characters", NULL, NULL);

			else if (WFD_RTSP_INSUFFICIENT_BUF == lRet || WFD_RTSP_MEMORY == lRet)

				WFD_SendResponse(p, p->nCSeq, 413, "Insufficient buffer to parse incoming message", NULL, NULL);

			else if (WFD_RTSP_STANDBY_MODE == lRet)
			
				WFD_SendResponse(p, p->nCSeq, 406, "in-standby-mode", NULL, NULL);

			else if (p->m_szErrorMessage[0] && 200 != p->m_nRTSPReponseCode) {

				WFD_SendResponse(p, p->nCSeq, p->m_nRTSPReponseCode, p->m_szErrorMessage, NULL, NULL);

			} else if (CountOfWFD_RTSPReturnCode > lRet) {

				char szMessage[100];

				wfd_snprintf_(szMessage, 100, "Error encountered while parsing message (%s)", g_szWFD_RTSPReturnCode[lRet]);
				WFD_SendResponse(p, p->nCSeq, (p->m_nRTSPReponseCode!=200)?p->m_nRTSPReponseCode:400, szMessage, NULL, NULL);

			} else {

				WFD_SendResponse(p, p->nCSeq, (p->m_nRTSPReponseCode!=200)?p->m_nRTSPReponseCode:400, "Error encountered while parsing message", NULL, NULL);

			}
		} else if (WFD_RTSP_NO_ERROR != lRet) {

			if (CountOfWFD_RTSPReturnCode > lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, g_szWFD_RTSPReturnCode[lRet]);
			}
			
			if (p->m_szErrorMessage[0] && 200 != p->m_nRTSPReponseCode) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
			}

			WFD_RTSPTeardown(p);
		}

		if (WFD_RTSP_NO_ERROR == lRet) {
			lRet = WFD_RTSP_UNSUPPORTED;
		}
	}

	goto top;

end_thread:
	// flush queues and reset pointers
	while (0 < p->m_bqSent.m_nBuffersQueued) {
		WFD_NODE* pBuf = WFD_QueueGetBuffer(&p->m_bqSent, p);
		WFD_QueuePutBuffer(&p->m_bqToSend, p, pBuf);
	}

	p->m_nPos = p->m_nBytesRead = p->m_nOutputBufferUsed = 0;
	p->m_bInParser = FALSE;

	return lRet;
}

long WFD_Send(CRTSPParser* p, char* szMessage)
{
	// LEGACY: required messages sent over the wire to be null terminated
	int nLen = (int)strlen(szMessage);
	long lRet = WFD_RTSP_NO_ERROR;

	lRet = p->m_pSend((unsigned char*)szMessage, nLen, p->m_pContext);
	if (lRet) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - send failed, Error = %d (%s)", __FUNCTION__, lRet, ((CountOfWFD_RTSPReturnCode > lRet)?g_szWFD_RTSPReturnCode[lRet]:""));
		return WFD_RTSP_SEND_ERROR;
	}

	return WFD_RTSP_NO_ERROR;
}

long WFD_SendResponse(CRTSPParser* p, int nCSeq, int nResponse, char* szRespMsg, char* szHeaders, char* szBody)
{
	long lRet=WFD_RTSP_NO_ERROR;
	char* szBuf= (char *)(p->m_pSendBuffer);
	long nContentLen = (long)(szBody?strlen(szBody):0);
	int nLen=(int)(strlen(RTSP_VERSION)+9+6+10+(szHeaders?strlen(szHeaders):0)+(nContentLen?(nContentLen+62 /* Content-Type/Content-Length */):0));
	int nCurrLen = 0;
	int nActualAllocation = RTSP_MAX_OUTPUT_BUFFER;

	if (RTSP_MAX_OUTPUT_BUFFER < nLen) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - unexpectedly, the buffer requirements exceed (%d), the pre-allocated buffer size", __FUNCTION__, nLen);
		szBuf = calloc(1, nLen+10);

		if (!szBuf) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed allocation of (%d) bytes", __FUNCTION__, nLen+10);
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		nActualAllocation = nLen+10;
	}

	nCurrLen = wfd_snprintf_(szBuf, nActualAllocation,
		"%s %d %s\r\n"
		"CSeq: %d\r\n",
		RTSP_VERSION, nResponse, szRespMsg,
		nCSeq);
	if (-1 == nCurrLen)
	{
		lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
		goto error;
	}

	if (p->m_szSession[0]) {
		lRet = wfd_snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen, "Session: %s\r\n", p->m_szSession);
		if (-1 == lRet)
		{
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		nCurrLen += lRet;
	}

	if (szHeaders && szHeaders[0]) {
		lRet = wfd_snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen, "%s", szHeaders);
		if (-1 == lRet)
		{
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		nCurrLen += lRet;
	}

	if (nContentLen) {
		if (0 > wfd_snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen,
			"Content-Type: text/parameters\r\n"
			"Content-Length: %ld\r\n\r\n"
			"%s",
			nContentLen,
			szBody))
		{
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}
	} else
		strcat(szBuf, "\r\n");

	lRet = WFD_Send(p, szBuf);

error:
	if (szBuf != (char*)p->m_pSendBuffer)
		free(szBuf);

	return lRet;
}

long WFD_SendRequest(CRTSPParser* p, char* szHeaders, char* szBody)
{
	long lRet = WFD_RTSP_NO_ERROR;
	long nContentLen = (long)(szBody?strlen(szBody):0);
	long nHdrLen = (long)strlen(szHeaders);
	long nLen = nHdrLen + (nContentLen?(nContentLen+62 /* Content-Type/Content-Length */):0);
	char* szBuf= (char *)(p->m_pSendBuffer);
	int nActualAllocation = RTSP_MAX_OUTPUT_BUFFER;

	if (RTSP_MAX_OUTPUT_BUFFER < nLen) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - unexpectedly, the buffer requirements exceed (%d), the pre-allocated buffer size", __FUNCTION__, nLen);
		szBuf = calloc(1, nLen+10);

		if (!szBuf) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed allocation of (%d) bytes", __FUNCTION__, nLen+10);
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		nActualAllocation = nLen+10;
	}

	if (nContentLen) {
		if (0 > wfd_snprintf_(szBuf, nActualAllocation,
			"%s" 
			"Content-Type: text/parameters\r\n"
			"Content-Length: %ld\r\n\r\n"
			"%s",
            szHeaders,
			nContentLen,
			szBody))
		{
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}
	} else {
		if (0 > wfd_snprintf_(szBuf, nActualAllocation, "%s\r\n", szHeaders))
		{
			lRet = WFD_RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}
	}

	lRet = WFD_Send(p, szBuf);

error:
	if (szBuf != (char*)p->m_pSendBuffer)
		free(szBuf);

	return lRet;
}

long WFD_MatchToken(char* apTokens[], int nCount, char* szToken)
{
	int i;

	if (!szToken) {
		//WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - szToken to match is NULL", __FUNCTION__);
		return -1;
	}

	for (i=0; i < nCount; i++) {
		if (0 == strcmp(apTokens[i], szToken))
			return i;
		else {
			size_t nLen = strlen(szToken);

			if ((szToken[nLen-1] == ':')
				&& (strlen(apTokens[i])+1 == nLen)
				&& (0 == strncmp(apTokens[i], szToken, nLen-1)))
			{
				return i;
			}
		}
	}

	return -1;
}

long WFD_CheckForRequestHeader(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;

	p->nToken = WFD_MatchToken(g_szWFD_ReqTokens, CountOfWFD_ReqTokens, p->pToken);
	switch (p->nToken) {
		case RAccept:
		case RAccept_Encoding:
		case RAccept_Language:
		case RAuthorization:
		case RBandwidth:
		case RBlocksize:
		case RConference:
		case RFrom:
		case RIf_Modified_Since:
		case RProxy_Require:
		case RRange:
		case RReferer:
			break;
		case RRequire:
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext));
			if (!p->pToken || strcmp(p->pToken, RTSP_OPTION_TAG)) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - WFD version mismatch, '%s' required, but client supports '%s'", __FUNCTION__, RTSP_OPTION_TAG, p->pToken);
				lRet = WFD_RTSP_VERSION_MISMATCH;
				p->m_nRTSPReponseCode = 505;
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "WFD version mismatch, '%s' required, but client supports '%s'", RTSP_OPTION_TAG, p->pToken?p->pToken:"");
			}
			break;
		case RScale:
			break;
		case RSession:
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext));
			//if (/*!p->pToken || */(p->m_szSession[0] && strncmp(p->m_szSession, p->pToken, WFD_RTSP_SESSION_ID_LEN))) {
			/**
			* For Google Nexus 4, it will omit the timeout=xx when GET_PARAMETER with Session,
			* but the session ID is correct. We must handle this situation.
			*/
			if ( (!p->pToken) || (strlen(p->pToken)>WFD_RTSP_SESSION_ID_LEN) || (p->m_szSession[0] && strncmp(p->m_szSession, p->pToken, strlen(p->pToken)))) {
				lRet = WFD_RTSP_SESSION_MISMATCH;
				p->m_nRTSPReponseCode = 454;
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Unexpected session name: '%s', expected: '%s'", p->pToken?p->pToken:"", p->m_szSession);
			}
			
			break;
		case RSpeed:
			break;
		case RTransport:
			{
				memset(&p->m_playStruct, 0, sizeof(p->m_playStruct));

				p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
				if (p->pToken) {
					char* pTok;

					strncpy(p->m_playStruct.szValue, p->pToken, WFD_RTSP_MAX_TRANSPORT_LEN);
					p->m_playStruct.szValue[WFD_RTSP_MAX_TRANSPORT_LEN-1] = 0; // make sure it's null terminated

					pTok=strstr(p->pToken, "client_port=");
					if (pTok) {
						p->m_usRTPPort = p->m_playStruct.usRTPPort = atoi(pTok+12);
						pTok=strstr(pTok, "-");
						if (pTok)
							p->m_playStruct.usRTCPPort = atoi(pTok+1);
						else
							p->m_playStruct.usRTCPPort = p->m_playStruct.usRTPPort+1;
					}
					
					pTok=strstr(p->pToken, "mode=");
					if (pTok) {
						if (strcmp(pTok, "mode=play")) {
							p->m_nRTSPReponseCode = 406;
							wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "in-standby-mode", pTok);
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						}
					}
				}
			}
			break;
		case RUser_Agent :
			break;
	}

	return lRet;
}

int WFD_ExportSessionId(CRTSPParser* p)
{
	char *ps;
	char id[64]={0};
	int i;
	FILE *fp;
	
	if(p == NULL){
		return -1;
	}

	/**
	* session_id: 1*(ALPHA | DIGIT | safe)
	* safe = "\$" | "-" | "_" | "." | "+"
	*/

	ps = p->m_szSession;
	for(i=0;i<64;i++){
		if((*(ps+i)>='0' && *(ps+i)<='9') || (*(ps+i)>='a' && *(ps+i)<='z') || \
			(*(ps+i)>='A' && *(ps+i)<='Z') || (*(ps+i)=='$') || (*(ps+i)=='-') || (*(ps+i)=='_') || (*(ps+i)=='.') || (*(ps+i)=='+') ){
			id[i] = *(ps+i);
		}
		else{
			break;
		}
	}

	if(i>=64){
		printf(">>> export session id error:len too large >64\n");
		return -1;
	}

	//printf(">>> export session id:%s\n",id);
	fp = fopen("/tmp/wfdsessionid","w");
	if(fp){
		fputs(id, fp);
		fclose(fp);
	}

	return 0;
	
}

long WFD_CheckForResponseHeader(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	BOOL bRds = FALSE;
	char szTemp[50];
#ifdef WIFI_DIRECT_JSON
	FILE *fd = NULL;
#endif
	p->nToken = WFD_MatchToken(g_szWFD_RespTokens, CountOfWFD_RespTokens, p->pToken);
	switch (p->nToken) {
		case rAllow:
		case rLocation:
		case rProxy_Authenticate:
			break;
		case rPublic:
			// following are the RTSP methods supported
			while ((p->pToken = strtok_s(NULL, ", ", &(p->m_pStrtokContext)))) {
				int nMethod = WFD_MatchToken(g_szWFD_MethodTokens, CountOfWFD_MethodTokens, p->pToken);

				if (0 <= nMethod)
					p->m_bPeerSupportedMethods[nMethod] = TRUE;
				else if (!strncmp(p->pToken, RTSP_OPTION_TAG, 13)) {
					strncpy(szTemp, p->pToken, 50);

					if (!strcmp(p->pToken, RTSP_OPTION_TAG)) {
						bRds = TRUE;
					} else {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - WFD version mismatch while parsing '%s' method, '%s' required, but client supports '%s'", __FUNCTION__, g_szWFD_MethodTokens[p->nToken], RTSP_OPTION_TAG, p->pToken);
						lRet = WFD_RTSP_VERSION_MISMATCH;
					}
				} else {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Supported method (%s) specified in Public: header is not valid", __FUNCTION__, p->pToken);
					lRet = WFD_RTSP_INVALID_METHOD;
				}
			}

			if (!bRds) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - ERROR while parsing '%s' method, '%s' required, but not found", __FUNCTION__, g_szWFD_MethodTokens[p->nToken], RTSP_OPTION_TAG);
				lRet = WFD_RTSP_VERSION_MISMATCH;
			}
			break;
		case rRange:
		case rRetry_After:
		case rRTP_Info:
		case rScale:
			break;
		case rSession:
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext));
			strncpy(p->m_szSession, p->pToken, WFD_RTSP_SESSION_ID_LEN);
			p->m_szSession[WFD_RTSP_SESSION_ID_LEN] = 0;
			{
				char* pTok=strstr(p->pToken, "timeout");
				if (pTok) {
					char* pContext;

					pTok = strtok_s(pTok, "= ", &pContext);
					if (pTok) {
						pTok = strtok_s(NULL, "= ", &pContext);
						if (pTok)
							p->m_nKeepalivePeriod = 1000*atoi(pTok);
					}
				}
			}

			{
				/** remove the timeout from session id */
				char *ptr = strstr(p->m_szSession,";timeout");
				if(ptr){
					*ptr=0;
					//printf(">>>>>>>>>> after remove session is:%s\n",p->m_szSession);
				}
			}
			if(p->m_eState == WFD_RTSP_M06){
				WFD_ExportSessionId(p);
			}
			break;
		case rServer:
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext));
			WFD_RTSP_LOG(p, RTSP_LOG_INFO, "Server is %s",p->pToken);
#ifdef WIFI_DIRECT_JSON
			if(p->pToken != NULL&& access("/tmp/rtsp_servername.txt",F_OK)!=0){
				fd = fopen("/tmp/rtsp_servername.txt","w");
				if(fd !=NULL){
					fprintf(fd,"%s",p->pToken);
					fflush(fd);
					fsync(fileno(fd));
					fclose(fd);
					//printf("--%s---%d---write rtsp_servername.txt ok!-------\n",__FUNCTION__,__LINE__);
				}
				else{
					//perror("open rtsp_severname.txt error!\n");
				}
			}
#endif
			if(strstr(p->pToken,"AllShareCast")){
				/** Samsung's AllShareCast */
				p->m_bHDCP = TRUE;
				WFD_RTSP_LOG(p, RTSP_LOG_INFO, "Samsung AllShareCast, enable HDCP");
			}
			break;
			
		case rSpeed:
			break;
		case rTransport:
			{
				memset(&p->m_playStruct, 0, sizeof(p->m_playStruct));

				p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
				if (p->pToken) {
					char* pTok;

					strncpy(p->m_playStruct.szValue, p->pToken, WFD_RTSP_MAX_TRANSPORT_LEN);
					p->m_playStruct.szValue[WFD_RTSP_MAX_TRANSPORT_LEN-1] = 0; // make sure it's null terminated

					pTok=strstr(p->pToken, "server_port=");
					if (pTok) {
						p->m_playStruct.usRTPPort = atoi(pTok+12);
						pTok=strstr(pTok, "-");
						if (pTok)
							p->m_playStruct.usRTCPPort = atoi(pTok+1);
						else
							p->m_playStruct.usRTCPPort = p->m_playStruct.usRTPPort+1;
					}
				}
			}
			break;
		case rUnsupported:
		case rVary:
		case rWWW_Authenticate:
			break;
	}

	return lRet;
}

long WFD_CheckForGeneralHeader(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;

	p->nToken = WFD_MatchToken(g_szWFD_GenTokens, CountOfWFD_GenTokens, p->pToken);
	switch (p->nToken) {
		case GCache_Control:
		case GConnection:
			break;
		case GCSeq:
			{
				p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // CSeq number
				if (!p->pToken) {
					p->m_nRTSPReponseCode = 400;
					wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected length while parsing CSeq: but got none");
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
					break;
				}

				p->nCSeq = atoi(p->pToken);
			}
			break;
		case GDate:
		case GVia:
			break;
	}

	return lRet;
}

long WFD_CheckForEntityHeader(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;

	p->nToken = WFD_MatchToken(g_szWFD_EntTokens, CountOfWFD_EntTokens, p->pToken);
	switch (p->nToken) {
		case EContent_Base:
		case EContent_Encoding:
		case EContent_Language:
			break;
		case EContent_Length:
			{
				p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // length
				if (!p->pToken) {
					p->m_nRTSPReponseCode = 411;
					wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected length while parsing Content-Length: but got none");
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				} else
					p->m_nContentLength = atoi(p->pToken);
			}
			break;
		case EContent_Location:
			break;
		case EContent_Type:
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // type

			if (!p->pToken) {
				p->m_nRTSPReponseCode = 400;
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected type while parsing Content-Type: but got none");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
			} else if (strcmp(p->pToken, "text/parameters")) {
				p->m_nRTSPReponseCode = 400;
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected test/parameters while parsing Content-Type: but got (%s)", p->pToken);
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
			}
			break;
		case EExpires:
		case ELast_Modified:
			break;
	}

	return lRet;
}

