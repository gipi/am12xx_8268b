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

#include "stdafx.h"
#include "rtspparser.h"

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

#define RTSP_CONNECTION_TIMEOUT	15000

int _snprintf_(char *buffer, size_t sizeOfBuffer, const char *format, ...)
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

void RTSP_LOG(CRTSPParser* p, int nLevel, char* szFmt,...)
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

CRTSPParser* RTSP_Alloc(BOOL bServer)
{
	CRTSPParser* p = calloc(1, sizeof(CRTSPParser));
	if (!p)
		return NULL;

	p->m_bServer = bServer;
	p->m_nSendCSeq = 1;
	p->m_usServerRtcpPort = 0;
	p->m_bLegacy = FALSE;

	memset(p->m_szSession, 0, RTSP_SESSION_ID_LEN+1);

	if (bServer) {
		p->m_bMySupportedMethods[MGET_PARAMETER] = 
		p->m_bMySupportedMethods[MPLAY] = 
		p->m_bMySupportedMethods[MSETUP] = 
		p->m_bMySupportedMethods[MSET_PARAMETER] = 
		p->m_bMySupportedMethods[MTEARDOWN] = TRUE;
	} else {
		p->m_bMySupportedMethods[MGET_PARAMETER] = 
		p->m_bMySupportedMethods[MSET_PARAMETER] = TRUE;
	}
	return p;
}

void RTSP_Free(CRTSPParser* p)
{
	if (p) {
		QueueFree(&p->m_bqSent, p);
		QueueFree(&p->m_bqToSend, p);

		if (p->m_pOptRdsSetupParams) free(p->m_pOptRdsSetupParams);
		free(p);
	}
}

long GetBufferOrCheckWhetherToBail(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;

	if (0 < p->nToken || 2 <= p->nStrikes) {
		if (p->m_nLineLen) {
			p->m_nLineLen = 0;
		}

		lRet = ReadNextLine(p);
		if (RTSP_NO_ERROR != lRet)
			return lRet;

		p->nStrikes = 0;
		if (0 == p->m_nLineLen)
			return lRet; // empty line, no error but need to check on return

		p->pToken = strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext));

	} else
		p->nStrikes++;

	return lRet;
}

long GetNext(CRTSPParser* p, char* pchar)
{
	long lRet = RTSP_NO_ERROR;
	long nBytesRead=0;

	if (p->m_bUnget) {

		*pchar = p->m_cUngetChar;
		p->m_bUnget = FALSE;

		return RTSP_NO_ERROR;

	} else if (p->m_nBytesRead == p->m_nPos) {

		// need to read more
		long nTimeout=30000;
		unsigned long ulDt;

		do {
			if (p->m_bRtspConnected) {
	
				if (p->m_bServer) {

					unsigned long CurTime = p->m_pGetTick(p->m_pContext);
					if (((long)(CurTime - p->m_nLastKeepalive) >= (p->m_bLegacy?30000:p->m_nKeepalivePeriod) - RESPONSE_TIMEOUT)) {

						// if we're a server and we're connected and 30 seconds has passed, we need to send out a keepalive message

						RTSPSetParam(p, "rds_keepalive:");
						p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);
				
					} else {
						nTimeout = (p->m_bLegacy?30000:p->m_nKeepalivePeriod) - (CurTime - p->m_nLastKeepalive) - RESPONSE_TIMEOUT;
					}

				} else {
					unsigned long t=p->m_pGetTick(p->m_pContext);

					// add 10000 msec to make the keepalive timout 40 seconds on the snk side because the 30 sec is too tight
					if (((long)(t - p->m_nLastKeepalive) > ((p->m_bLegacy?30000:p->m_nKeepalivePeriod) + 10000))) {

						// we're a client/sink, so if we haven't gotten a keepalive, we need to end the session
						RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - snk keepalive timeout, curr:%d, last:%d, delta:%d, legacy:%d, keepalive period:%d", __FUNCTION__, 
							t, p->m_nLastKeepalive, (t- p->m_nLastKeepalive), p->m_bLegacy, p->m_bLegacy?30000:p->m_nKeepalivePeriod);
				
						RTSPTeardown(p);

						return RTSP_KEEPALIVE_TIMEOUT;

				
					} else {
						// add 10000 msec to make the keepalive timout 40 seconds on the snk side because the 30 sec is too tight
						nTimeout = ((p->m_bLegacy?30000:p->m_nKeepalivePeriod)+10000) - (p->m_pGetTick(p->m_pContext) - p->m_nLastKeepalive);
					}
				}

			}

			if (0 < p->m_bqSent.m_nBuffersQueued) {
				// if we are waiting for a response for a sent packet, set the recv timeout to expire 5 seconds after the packet was sent
				nTimeout = MIN((long)(p->m_nResponseTimeout - (p->m_pGetTick(p->m_pContext) - p->m_bqSent.m_pHead->nTimeSent)), nTimeout);
				if (0 > nTimeout) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for response from peer", __FUNCTION__);
					p->m_pNotify(RTSP_TIMEOUT, NULL, p->m_pContext);
					nTimeout = 5000; // we don't want a negative number
				}
			}

			ulDt = p->m_pGetTick(p->m_pContext) - p->m_ulStartTime;
			if (!p->m_bRtspConnected && RTSP_CONNECTION_TIMEOUT < ulDt) {
				// we're not connected yet, and it's been RTSP_CONNECTION_TIMEOUT msec since the rtsp stack was started, so abort with timeout
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - timeout waiting for response from peer, %lu msec elapsed since start %lu", __FUNCTION__, ulDt, p->m_ulStartTime);
				p->m_pNotify(RTSP_TIMEOUT, NULL, p->m_pContext);

				return RTSP_SOCKET_RECV_TIMEOUT;
			}

			if (p->m_bServer)
				nTimeout = MIN(RESPONSE_TIMEOUT, nTimeout);

			lRet = p->m_pRecv(p->m_pReadBuffer, RTSP_RECV_BUFFER, &nBytesRead, nTimeout, p->m_pContext);
		
		} while (RTSP_SOCKET_RECV_TIMEOUT == lRet);

		if (RTSP_NO_ERROR != lRet)
			return lRet;

		// LEGACY stack resets keepalive countdown whenever it sends out any message, so in legacy mode, we must do the same
		if (!p->m_bServer && p->m_bLegacy)
			p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);

		p->m_nBytesRead = nBytesRead;
		p->m_nPos = 0;
	}

	*pchar = p->m_pReadBuffer[p->m_nPos++];

	return RTSP_NO_ERROR;
}

void UnGet(CRTSPParser* p, char c)
{
	p->m_cUngetChar = c;
	p->m_bUnget = TRUE;
}

long ReadNextLine(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	long nPos=0;
	char c, c2;

	do {
		lRet = GetNext(p, &c);
		if (RTSP_NO_ERROR != lRet)
			break;

		if (0 == c) {
			p->m_bLegacy = TRUE;
			p->m_pInputBuffer[nPos] = 0;
			p->m_nLineLen = nPos;

			// legacy, treat null char as eol/end of message
			// RTSP_NO_ERROR
			break;
		}

		if ('\r' == c || '\n' == c) {

			switch (p->m_eolType) {
			case EOL_CR:
				if ('\r' != c) {
					lRet = RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

			case EOL_LF:
				if ('\n' != c) {
					lRet = RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

			case EOL_CRLF:
				if ('\r' != c) {
					if (p->m_bLegacy && 
						(
							!strncmp((char*)p->m_pInputBuffer,    "rds_friendly_name: Wireless XYZ", 31)
							|| !strncmp((char*)p->m_pInputBuffer, "rds_audio_formats: LPCM16/48/x2", 31)
							|| !strncmp((char*)p->m_pInputBuffer, "rds_video_formats: MPEG-2-MP@HL", 31)
						)) {
						lRet = GetNext(p, &c);
						if (RTSP_NO_ERROR != lRet)
							goto cleanup;

					} else {
						lRet = RTSP_UNEXPECTED_EOLCHAR;
						goto cleanup;
					}
				}

				lRet = GetNext(p, &c2);
				if (RTSP_NO_ERROR != lRet)
					return lRet;

				if ('\n' != c2) {
					lRet = RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

			case EOL_LFCR:
				if ('\n' != c) {
					lRet = RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				lRet = GetNext(p, &c2);
				if (RTSP_NO_ERROR != lRet)
					goto cleanup;

				if ('\r' != c2) {
					lRet = RTSP_UNEXPECTED_EOLCHAR;
					goto cleanup;
				}

				break;

            case EOL_UNKNOWN:
            case CountOfEOLTypes:

				lRet = GetNext(p, &c2);
				if (RTSP_NO_ERROR != lRet)
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

			// RTSP_NO_ERROR
			break;

		}

		if (RTSP_MAX_INPUT_BUFFER-1 <= nPos) {
			lRet = RTSP_INSUFFICIENT_BUF;
			break;
		}

		// add character and go back for another
		p->m_pInputBuffer[nPos++] = c;

	} while (TRUE);

cleanup:

	return lRet;
}

long RTSP_SetSrcAddress(CRTSPParser* p, char* szIpAddr)
{
	if (szIpAddr)
		strncpy(p->m_szIpAddress, szIpAddr, RTSP_MAX_IPADDR_BUFFER);
	else
		strncpy(p->m_szIpAddress, "0.0.0.0", RTSP_MAX_IPADDR_BUFFER);

	return RTSP_NO_ERROR;
}

long RTSP_Init(CRTSPParser* p, unsigned short usRtpPort, unsigned short usServerRtcpPort, int nOverscanComp, char* pOptRdsSetupParams, FN_RTSP_NOTIFICATIONS pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext)
{
	int nLen;

	p->m_pNotify = pfnCallback;
	p->m_pRecv = pfnRecv;
	p->m_pSend = pfnSend;
	p->m_pSleep = pfnSleep;
	p->m_pGetTick = pfnGetTick;
	p->m_pLog = pfnLog;
	p->m_pContext = pContext;
	p->m_bqSent.m_pSleep = pfnSleep;
	p->m_bqToSend.m_pSleep = pfnSleep;
	p->m_usRTPPort = usRtpPort;
	p->m_usServerRtcpPort = usServerRtcpPort;
	p->m_nKeepalivePeriod = RDS_SESSION_TIMEOUT_DEFAULT;
	p->m_nResponseTimeout = RESPONSE_TIMEOUT;
	p->m_nOverscanComp = nOverscanComp;

	nLen = pOptRdsSetupParams?strlen(pOptRdsSetupParams):0;
	if (nLen) {
		p->m_pOptRdsSetupParams = malloc(nLen+1);
		if (p->m_pOptRdsSetupParams)
			memcpy(p->m_pOptRdsSetupParams, pOptRdsSetupParams, nLen+1);
	} else {
		p->m_pOptRdsSetupParams = malloc(1);
		if (p->m_pOptRdsSetupParams)
			p->m_pOptRdsSetupParams[0] = 0;
	}

	if (!QueueAllocate(&p->m_bqToSend, p, RTSP_SEND_BUFFER, RTSP_SEND_COUNT)) {
		// failed to allocate send buffers
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - QueueAllocate(%d, %d) failed", __FUNCTION__, RTSP_SEND_BUFFER, RTSP_SEND_COUNT);
		return RTSP_MEMORY;
	}

	p->m_pReadBuffer = calloc(1, RTSP_RECV_BUFFER);
	if (!p->m_pReadBuffer) {
		// failed to allocate input buffer
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - recv buffer allocation of %d bytes failed", __FUNCTION__, RTSP_RECV_BUFFER);
		return RTSP_MEMORY;
	}

	p->m_pInputBuffer = calloc(1, RTSP_MAX_INPUT_BUFFER);
	if (!p->m_pInputBuffer) {
		// failed to allocate input buffer
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - input buffer allocation of %d bytes failed", __FUNCTION__, RTSP_MAX_INPUT_BUFFER);
		return RTSP_MEMORY;
	}

	p->m_pSendBuffer = calloc(1, RTSP_MAX_OUTPUT_BUFFER);
	if (!p->m_pSendBuffer) {
		// failed to allocate work buffer
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - input buffer allocation of %d bytes failed", __FUNCTION__, RTSP_MAX_OUTPUT_BUFFER);
		return RTSP_MEMORY;
	}

	p->m_pOutputBuffer = calloc(1, RTSP_MAX_OUTPUT_BUFFER);
	if (!p->m_pOutputBuffer) {
		// failed to allocate output buffer
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - input buffer allocation of %d bytes failed", __FUNCTION__, RTSP_MAX_OUTPUT_BUFFER);
		return RTSP_MEMORY;
	}

	p->pParsed = calloc(1, sizeof(NODE) + RTSP_MAX_INPUT_BUFFER);
	if (!p->pParsed) {
		// failed to allocate parsed buffer
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - parsed buffer allocation of %d bytes failed", __FUNCTION__, sizeof(NODE) + RTSP_MAX_INPUT_BUFFER);
		return RTSP_MEMORY;
	}
	p->pParsed->nAllocated = RTSP_MAX_INPUT_BUFFER;
	p->pParsed->nUsed = 0;

	return RTSP_NO_ERROR;
}

long RTSP_UpdateParams(CRTSPParser* p, int nOverscanComp, char* pOptRdsSetupParams)
{
	int nLen;

	p->m_nOverscanComp = nOverscanComp;

	nLen = pOptRdsSetupParams?strlen(pOptRdsSetupParams):0;
	if (nLen) {
		p->m_pOptRdsSetupParams = malloc(nLen+1);
		if (p->m_pOptRdsSetupParams)
			memcpy(p->m_pOptRdsSetupParams, pOptRdsSetupParams, nLen+1);
	} else {
		p->m_pOptRdsSetupParams = malloc(1);
		if (p->m_pOptRdsSetupParams)
			p->m_pOptRdsSetupParams[0] = 0;
	}

	return RTSP_NO_ERROR;
}

long RTSP_SetParam(CRTSPParser* p, char* szParams)
{
	return SendSetParameters(p, szParams);
}

long RTSP_GetParam(CRTSPParser* p, char* szParams)
{
	return SendGetParameters(p, szParams);
}

long RTSP_SetParamBinary(CRTSPParser* p, RDSParams id, void* pData, long nDataLen)
{
	long lRet = RTSP_NO_ERROR;
	PARAMETER_STRUCT ps;
	int lLen=0;

	if (CountOfRDSParams <= id || 0 > id)
		return RTSP_INVALID_ID;

	memset(&ps, 0, sizeof(PARAMETER_STRUCT));

	ps.nId = id;

	lRet = RTSPEncodeMIME(p, NULL, &lLen, pData, nDataLen);
	if (RTSP_NO_ERROR != lRet)
		return lRet;

	ps.szValue = calloc(lLen+1, 1);
	lRet = RTSPEncodeMIME(p, ps.szValue, &lLen, pData, nDataLen);
	if (RTSP_NO_ERROR != lRet) {
		free(ps.szValue);
		return lRet;
	}

	ps.nCount = lLen;

	lRet = RTSP_SetParamStruct(p, &ps);

	free(ps.szValue);
	return lRet;
}

long RTSP_SetParamStruct(CRTSPParser* p, PARAMETER_STRUCT* pStruct)
{
	char szMsg[RDS_STATIC_LEN];
	int nLen=0;
	
	if (p->m_bLegacy && pStruct->nId != rds_friendly_name && pStruct->nId != rds_overscan_comp && pStruct->nId != rds_keepalive)
		return RTSP_NO_ERROR; // don't send SET_PARAMETER to legacy except for these id's

	if (CountOfRDSParams <= pStruct->nId || 0 > pStruct->nId)
		return RTSP_INVALID_ID;

	if (rds_content_protection == pStruct->nId) {
		if (pStruct->u.contentProtection.usPort)
			nLen = _snprintf_(szMsg, RDS_STATIC_LEN, "HDCP%.1f port=%d", pStruct->u.contentProtection.fHDCPVersion, pStruct->u.contentProtection.usPort);
		else
			nLen = _snprintf_(szMsg, RDS_STATIC_LEN, "none");
	} else if (rds_sink_version == pStruct->nId) {
		nLen = _snprintf_(szMsg, RDS_STATIC_LEN, 
			"product_ID=%s, hw_version=%d.%d.%d.%d, "
			"sw_version=%d.%d.%d.%d", 
			pStruct->u.version.szProductId,
			pStruct->u.version.verHardware.ucMajor, pStruct->u.version.verHardware.ucMinor, 
			pStruct->u.version.verHardware.ucSku, pStruct->u.version.verHardware.usBuild,
			pStruct->u.version.verSoftware.ucMajor, pStruct->u.version.verSoftware.ucMinor, 
			pStruct->u.version.verSoftware.ucSku, pStruct->u.version.verSoftware.usBuild);
	} else if (rds_status == pStruct->nId) {
		nLen = _snprintf_(szMsg, RDS_STATIC_LEN,
			"busy=%d, display_connected=%d", pStruct->u.status.nBusy, pStruct->u.status.nDisplayConnected);
	} else if (rds_overscan_comp == pStruct->nId) {
		nLen = _snprintf_(szMsg, RDS_STATIC_LEN,
			"x=%d, y=%d", pStruct->u.overscanComp.nX, pStruct->u.overscanComp.nY);
	} else if (rds_rtp_profile == pStruct->nId) {
		if (CountOfRDSProfiles <= pStruct->u.profile)
			return RTSP_INVALID_ID;

		nLen = _snprintf_(szMsg, RDS_STATIC_LEN,
			"%s", g_szRDSProfiles[pStruct->u.profile]);
	} else if (rds_audio_formats == pStruct->nId) {
		int nAdditionalLen = (long)(strlen(g_szRDSParams[pStruct->nId]))+4; // +4 for ': ' and CRLF
		int nUsed=0;
		BOOL bFirst=TRUE;
		int i;

		for (i=0; i < (int)CountOfRDSAudioFormats; i++) {
			if (pStruct->u.ulFormats & (1<<i)) {

				nAdditionalLen += (long)(strlen(g_szRDSAudioFormats[i]))+2; // +2 for ', '

				if (RDS_STATIC_LEN < nUsed + nAdditionalLen) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer for audio formats)", __FUNCTION__);
					return RTSP_INSUFFICIENT_BUF;
				}

				if (bFirst) {
					nLen = _snprintf_(szMsg, RDS_STATIC_LEN, "%s", g_szRDSAudioFormats[i]);
					bFirst = FALSE;
				} else {
					nLen = _snprintf_(szMsg+nUsed, RDS_STATIC_LEN-nUsed, ", %s", g_szRDSAudioFormats[i]);
				}

				if (0 < nLen)
					nUsed += nLen;
			}
		}
	} else if (rds_video_formats == pStruct->nId) {
		int nAdditionalLen = (long)(strlen(g_szRDSParams[pStruct->nId]))+4; // +4 for ': ' and CRLF
		int nUsed=0;
		BOOL bFirst=TRUE;
		int i;

		for (i=0; i < (int)CountOfRDSVideoFormats; i++) {
			if (pStruct->u.ulFormats & (1<<i)) {

				nAdditionalLen += (long)(strlen(g_szRDSVideoFormats[i]))+2; // +2 for ', '

				if (RDS_STATIC_LEN < nUsed + nAdditionalLen) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer for video formats)", __FUNCTION__);
					return RTSP_INSUFFICIENT_BUF;
				}

				if (bFirst) {
					nLen = _snprintf_(szMsg, RDS_STATIC_LEN, "%s", g_szRDSVideoFormats[i]);
					bFirst = FALSE;
				} else {
					nLen = _snprintf_(szMsg+nUsed, RDS_STATIC_LEN-nUsed, ", %s", g_szRDSVideoFormats[i]);
				}

				if (0 < nLen)
					nUsed += nLen;
			}
		}
	} else if (pStruct->szValue && pStruct->szValue[0]) {
		// if the szValue member is set, we will use it
		return SendSetParameter(p, pStruct->nId, pStruct->szValue);
	} else {
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s, szValue memeber is either NULL or an empty string", __FUNCTION__, g_szRDSParams[pStruct->nId]);
		return RTSP_INVALID_ID;
	}

	if (0 > nLen)
		return RTSP_INSUFFICIENT_BUF;
	else
		return SendSetParameter(p, pStruct->nId, szMsg);
}

long RTSP_Shutdown(CRTSPParser* p)
{
	if (p->m_bInParser)
		return RTSP_PARSER_ACTIVE; // this means the socket must be closed first, so the parser can exit

	if (p->m_pReadBuffer) free(p->m_pReadBuffer);
	if (p->m_pSendBuffer) free(p->m_pSendBuffer);
	if (p->m_pInputBuffer) free(p->m_pInputBuffer);
	if (p->m_pOutputBuffer) free(p->m_pOutputBuffer);
	if (p->pParsed) free(p->pParsed);

	return RTSP_NO_ERROR;
}


long RTSP_Parser(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	BOOL bResponse = FALSE;

	// this is stuff to initialize at the start of each connection
	p->m_bInParser = TRUE;
	p->pToken=NULL;
	p->nMethod=CountOfMethodTokens;
	p->m_eolType = EOL_UNKNOWN;
	p->m_nRecvCSeq = -1;
	p->m_bRtspConnected = FALSE;
	memset(p->m_szSession, 0, RTSP_SESSION_ID_LEN+1);
	p->m_ulStartTime = p->m_pGetTick(p->m_pContext);
	RTSP_LOG(p, RTSP_LOG_INFO, "%s - Start time: %ld", __FUNCTION__, p->m_ulStartTime);

	if (p->m_bServer) {
		// a client is connected, so send it options
		lRet = SendOptions(p);

		if (RTSP_NO_ERROR != lRet)
			goto send_error;
	}

top:
	do {
		p->pBuf = NULL;
		p->nCSeq=-1;

		lRet = ReadNextLine(p);
		if (RTSP_NO_ERROR != lRet)
			goto end_thread; // end thread

		// got a buffer, start parsing it
		p->m_nContentLength = 0;
		bResponse = FALSE;
		p->m_nRTSPReponseCode = 200;
		p->m_szErrorMessage[0] = 0;

		if (!strncmp((char*)p->m_pInputBuffer, "RTSP/", 5)) {
			// this is a response
			bResponse = TRUE;
			if (RTSP_NO_ERROR != ParseMessageResponse(p))
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
			p->nMethod = MatchToken(g_szMethodTokens, CountOfMethodTokens, p->pToken);

			if (0 <= p->nMethod) {
				// first make sure this is an RTSP method we handle
				if (MOPTIONS != p->nMethod && !p->m_bMySupportedMethods[p->nMethod]) {
					// we don't support this method
					p->m_nRTSPReponseCode = 405;
					_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Method (%s) is not one of the methods allowed as specified in the OPTIONS response", p->pToken);
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				}

				// Request-Line = Method SP Request-URI SP RTSP-Version CRLF   ; Section 6.1

				p->pToken=strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // Request-URI

				p->pToken=strtok_s(NULL,  "\r\n", &(p->m_pStrtokContext)); // RTSP-Version

				p->m_dServerVersion = atof((char*)p->pToken+5); // RTSP/m.n

				// Finished parsing message header

				lRet = ReadNextLine(p);
				if (RTSP_NO_ERROR != lRet)
					goto end_thread; // end thread

				p->pToken = strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext));

				p->nStrikes=0;

				// next there should be 0 or more general-header|request-header|entity-header
				do {
					// we will bail out of this loop when we get an empty line
					if (0 == p->m_nLineLen)
						break;

					// first check for general-headers
					lRet = CheckForGeneralHeader(p);
					if (RTSP_NO_ERROR != lRet)
						goto send_error;

					// make sure CSeq is greater than last one received
					if (GCSeq == p->nToken) {
						if (-1 == p->nCSeq)
							p->m_nRecvCSeq = p->nCSeq;
						else if (p->m_nRecvCSeq >= p->nCSeq) {
							p->m_nRTSPReponseCode = 400;
							_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected CSeq: greater than %d, but got: %d instead", p->m_nRecvCSeq, p->nCSeq);
							RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						} else
							p->m_nRecvCSeq = p->nCSeq;
					}

					lRet = GetBufferOrCheckWhetherToBail(p);
					if (RTSP_NO_ERROR != lRet)
						goto end_thread;
					else if (0 == p->m_nLineLen)
						break;

					// next check for request-headers
					lRet = CheckForRequestHeader(p);
					if (RTSP_NO_ERROR != lRet)
						goto send_error;

					lRet = GetBufferOrCheckWhetherToBail(p);
					if (RTSP_NO_ERROR != lRet)
						goto end_thread;
					else if (0 == p->m_nLineLen)
						break;

					// next check for entity-headers
					lRet = CheckForEntityHeader(p);
					if (RTSP_NO_ERROR != lRet)
						goto send_error;

					lRet = GetBufferOrCheckWhetherToBail(p);
					if (RTSP_NO_ERROR != lRet)
						goto end_thread;
					else if (0 == p->m_nLineLen)
						break;

				} while (TRUE);

				p->m_nLineLen = 0;
			
				if (RTSP_NO_ERROR == lRet && 200 == p->m_nRTSPReponseCode) {

					switch (p->nMethod) {
					case MDESCRIBE:              // Section 10.2
					case MANNOUNCE:              // Section 10.3
						p->m_nRTSPReponseCode = 501;
						_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "RTSP message (%s) not implemented", g_szMethodTokens[p->nMethod]);
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						break;
					case MGET_PARAMETER:         // Section 10.8
						break;
					case MOPTIONS:               // Section 10.1
						lRet = SendOptionsResponse(p, p->nCSeq);
						if (RTSP_NO_ERROR != lRet)
							goto send_error;

						if (!p->m_bServer) {
							// if this is the client, we need to send out our options
							lRet = SendOptions(p);
							if (RTSP_NO_ERROR != lRet)
								goto send_error;
						}

						if (p->m_bServer) {
							// if this is the client, after we get client options, we send GET_PARAMETERS
							if (p->m_bLegacy) // LEGACY: only send rds_audio_format/rds_video_format to legacy adapter
								lRet = SendGetParameters(p, "rds_friendly_name, rds_unique_device_id, rds_status, rds_audio_formats, rds_video_formats, rds_rtp_profile, rds_content_protection, rds_sink_version");
							else
								lRet = SendGetParameters(p, "rds_friendly_name, rds_audio_formats, rds_unique_device_id, rds_status, rds_content_protection, rds_sink_version, rds_fast_cursor_disabled");
							if (RTSP_NO_ERROR != lRet)
								goto send_error;

							// next we get the edid
							lRet = SendGetParameters(p, "rds_display_edid");
							if (RTSP_NO_ERROR != lRet)
								goto send_error;

							// we save this because when we get this reponse, we send the setup trigger
							p->m_nCSeqForGetEdid = p->m_nSendCSeq-1;
						}
						break;
					case MPAUSE:                 // Section 10.6
						p->m_nRTSPReponseCode = 501;
						_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "RTSP message (%s) not implemented", g_szMethodTokens[p->nMethod]);
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						break;
					case MPLAY:                  // Section 10.5
						lRet = SendPlayResponse(p, p->nCSeq);
						if (RTSP_NO_ERROR != lRet)
							goto send_error;

						p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);
						p->m_bRtspConnected = TRUE;

						lRet = p->m_pNotify(RTSP_PLAY, &p->m_playStruct, p->m_pContext);
						if (RTSP_NO_ERROR != lRet) {
							RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
							lRet = RTSP_INTERNAL_ERROR;
							goto send_error;
						}

						break;
					case MRECORD:                // Section 10.11
					case MREDIRECT:              // Section 10.10
						p->m_nRTSPReponseCode = 501;
						_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "RTSP message (%s) not implemented", g_szMethodTokens[p->nMethod]);
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						break;
					case MSETUP:                 // Section 10.4
						lRet = SendSetupResponse(p, p->nCSeq);
						if (RTSP_NO_ERROR != lRet)
							goto send_error;

						break;
					case MSET_PARAMETER:         // Section 10.9
						//RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - Set param", __FUNCTION__);
						break;
					case MTEARDOWN:              // Section 10.7
						
						p->m_bRtspConnected = FALSE;

						lRet = SendTeardownResponse(p, p->nCSeq);
						if (RTSP_NO_ERROR != lRet)
							goto send_error;

						p->m_bRtspConnected = FALSE;

						lRet = p->m_pNotify(RTSP_TEARDOWN, 0, p->m_pContext);
						if (RTSP_NO_ERROR != lRet) {
							RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
							lRet = RTSP_INTERNAL_ERROR;
							goto send_error;
						}
						break;
					}

					if (RTSP_NO_ERROR != lRet)
						goto send_error;
				}
			}
		}

		if (p->m_nContentLength && RTSP_NO_ERROR == lRet && 200 == p->m_nRTSPReponseCode) {
			// get content
			lRet = GetContents(p, p->m_nContentLength);
			if (RTSP_NO_ERROR != lRet)
				break;

			p->pParsed->nCSeq = p->nCSeq;
			p->pParsed->nMethod = (int)p->nMethod;

			lRet = RDSParser(p);
			if (RTSP_NO_ERROR != lRet)
				break;
		} else if (0 == p->m_nContentLength) {
			if (MGET_PARAMETER == p->nMethod) {
				lRet = SendGetParametersResponse(p, p->nCSeq, (char*)"");
				if (RTSP_NO_ERROR != lRet) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendGetParametersResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq, p->m_pOutputBuffer);
					return RTSP_INTERNAL_ERROR;
				}
			} else if (MSET_PARAMETER == p->nMethod) {
				lRet = SendSetParametersResponse(p, p->nCSeq);
				if (RTSP_NO_ERROR != lRet) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendSetParametersResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq, p->m_pOutputBuffer);
					return RTSP_INTERNAL_ERROR;
				}
			}
		}

	} while (200 == p->m_nRTSPReponseCode);

send_error:

	if (RTSP_NO_ERROR != lRet || 200 != p->m_nRTSPReponseCode) {

		if (!bResponse && RTSP_SOCKET_RECV_FAILED != lRet) {
			// if this was not a response to one of our messages, and the socket wasn't closed, send an error response
			if (RTSP_UNEXPECTED_EOLCHAR == lRet)

				SendResponse(p, p->nCSeq, 400, "Inconsistent use of CRLF characters", NULL, NULL);

			else if (RTSP_INSUFFICIENT_BUF == lRet || RTSP_MEMORY == lRet)

				SendResponse(p, p->nCSeq, 413, "Insufficient buffer to parse incoming message", NULL, NULL);

			else if (p->m_szErrorMessage[0] && 200 != p->m_nRTSPReponseCode) {

				SendResponse(p, p->nCSeq, p->m_nRTSPReponseCode, p->m_szErrorMessage, NULL, NULL);

			} else if (CountOfRDSRTSPReturnCode > lRet) {

				char szMessage[100];

				_snprintf_(szMessage, 100, "Error encountered while parsing message (%s)", g_szRDSRTSPReturnCode[lRet]);
				SendResponse(p, p->nCSeq, (p->m_nRTSPReponseCode!=200)?p->m_nRTSPReponseCode:400, szMessage, NULL, NULL);

			} else {

				SendResponse(p, p->nCSeq, (p->m_nRTSPReponseCode!=200)?p->m_nRTSPReponseCode:400, "Error encountered while parsing message", NULL, NULL);

			}
		}

		if (RTSP_NO_ERROR == lRet) {
			lRet = RTSP_UNSUPPORTED;
		}
	}

	goto top;

end_thread:
	// flush queues and reset pointers
	while (0 < p->m_bqSent.m_nBuffersQueued) {
		NODE* pBuf = QueueGetBuffer(&p->m_bqSent, p);
		QueuePutBuffer(&p->m_bqToSend, p, pBuf);
	}

	p->m_nPos = p->m_nBytesRead = p->m_nOutputBufferUsed = 0;
	p->m_bInParser = FALSE;

	return lRet;
}

long _Send(CRTSPParser* p, char* szMessage)
{
	// LEGACY: required messages sent over the wire to be null terminated
	int nLen = (int)strlen(szMessage)+(p->m_bLegacy?1:0);
	long lRet = RTSP_NO_ERROR;

	lRet = p->m_pSend((unsigned char*)szMessage, nLen, p->m_pContext);
	if (lRet) {
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - send failed, Error = %d (%s)", __FUNCTION__, lRet, ((CountOfRDSRTSPReturnCode > lRet)?g_szRDSRTSPReturnCode[lRet]:""));
		return RTSP_SEND_ERROR;
	}

	return RTSP_NO_ERROR;
}

long SendResponse(CRTSPParser* p, int nCSeq, int nResponse, char* szRespMsg, char* szHeaders, char* szBody)
{
	long lRet=RTSP_NO_ERROR;
	char* szBuf= (char *)(p->m_pSendBuffer);
	long nContentLen = (long)(szBody?strlen(szBody):0);
	int nLen=(int)(strlen(RTSP_VERSION)+9+6+10+(szHeaders?strlen(szHeaders):0)+(nContentLen?(nContentLen+60 /* Conten-Type/Conten-Length */):0));
	int nCurrLen = 0;
	int nActualAllocation = RTSP_MAX_OUTPUT_BUFFER;

	if (RTSP_MAX_OUTPUT_BUFFER < nLen) {
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - unexpectedly, the buffer requirements exceed (%d), the pre-allocated buffer size", __FUNCTION__, nLen);
		szBuf = calloc(1, nLen+10);

		if (!szBuf) {
			RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed allocation of (%d) bytes", __FUNCTION__, nLen+10);
			lRet = RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		nActualAllocation = nLen+10;
	}

	if (p->m_bLegacy) {
		// LEGACY: Session: header must be last
		nCurrLen = _snprintf_(szBuf, nActualAllocation,
			"%s %d %s\r\n"
			"CSeq: %d\r\n",
			RTSP_VERSION, nResponse, szRespMsg,
			nCSeq);
		if (-1 == nCurrLen)
		{
			lRet = RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		if (nContentLen) {
			lRet = _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen,
				"Content-Type: text/parameters\r\n"
				"Content-Length: %ld\r\n",
				nContentLen);
			if (-1 == lRet)
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}

			nCurrLen += lRet;
		}

		if (p->m_szSession[0]) {
			lRet = _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen, "Session: %s\r\n", p->m_szSession);
			if (-1 == lRet)
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}

			nCurrLen += lRet;
		}

		if (szHeaders && szHeaders[0]) {
			lRet = _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen, "%s", szHeaders);
			if (-1 == lRet)
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}

			nCurrLen += lRet;
		}

		if (nContentLen) {
			if (0 > _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen,
				"\r\n%s",
				szBody))
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}
		} else
			strcat(szBuf, "\r\n");

		lRet = _Send(p, szBuf);
	} else {
		nCurrLen = _snprintf_(szBuf, nActualAllocation,
			"%s %d %s\r\n"
			"CSeq: %d\r\n",
			RTSP_VERSION, nResponse, szRespMsg,
			nCSeq);
		if (-1 == nCurrLen)
		{
			lRet = RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		if (p->m_szSession[0]) {
			lRet = _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen, "Session: %s\r\n", p->m_szSession);
			if (-1 == lRet)
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}

			nCurrLen += lRet;
		}

		if (szHeaders && szHeaders[0]) {
			lRet = _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen, "%s", szHeaders);
			if (-1 == lRet)
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}

			nCurrLen += lRet;
		}

		if (nContentLen) {
			if (0 > _snprintf_(szBuf+nCurrLen, nActualAllocation-nCurrLen,
				"Content-Type: text/parameters\r\n"
				"Content-Length: %ld\r\n\r\n"
				"%s",
				nContentLen,
				szBody))
			{
				lRet = RTSP_INTERNAL_ERROR; // this should never occur
				goto error;
			}
		} else
			strcat(szBuf, "\r\n");

		lRet = _Send(p, szBuf);
	}

error:
	if (szBuf != (char*)p->m_pSendBuffer)
		free(szBuf);

	return lRet;
}

long SendRequest(CRTSPParser* p, char* szHeaders, char* szBody)
{
	long lRet = RTSP_NO_ERROR;
	long nContentLen = (long)(szBody?strlen(szBody):0);
	long nHdrLen = (long)strlen(szHeaders);
	long nLen = nHdrLen + (nContentLen?(nContentLen+60 /* Conten-Type/Conten-Length */):0);
	char* szBuf= (char *)(p->m_pSendBuffer);
	int nActualAllocation = RTSP_MAX_OUTPUT_BUFFER;

	if (RTSP_MAX_OUTPUT_BUFFER < nLen) {
		RTSP_LOG(p, RTSP_LOG_ERR, "%s - unexpectedly, the buffer requirements exceed (%d), the pre-allocated buffer size", __FUNCTION__, nLen);
		szBuf = calloc(1, nLen+10);

		if (!szBuf) {
			RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed allocation of (%d) bytes", __FUNCTION__, nLen+10);
			lRet = RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}

		nActualAllocation = nLen+10;
	}

	if (nContentLen) {
		if (0 > _snprintf_(szBuf, nActualAllocation,
			"%s" 
			"Content-Type: text/parameters\r\n"
			"Content-Length: %ld\r\n\r\n"
			"%s",
            szHeaders,
			nContentLen,
			szBody))
		{
			lRet = RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}
	} else {
		if (0 > _snprintf_(szBuf, nActualAllocation, "%s\r\n", szHeaders))
		{
			lRet = RTSP_INTERNAL_ERROR; // this should never occur
			goto error;
		}
	}

	lRet = _Send(p, szBuf);

error:
	if (szBuf != (char*)p->m_pSendBuffer)
		free(szBuf);

	return lRet;
}

long MatchToken(char* apTokens[], int nCount, char* szToken)
{
	int i;

	if (!szToken) {
		//RTSP_LOG(p, RTSP_LOG_ERR, "%s - szToken to match is NULL", __FUNCTION__);
		return -1;
	}

	for (i=0; i < nCount; i++) {
		if (0 == strcmp(apTokens[i], szToken))
			return i;
		else {
			int nLen = (int)strlen(szToken);

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

long CheckForRequestHeader(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;

	p->nToken = MatchToken(g_szReqTokens, CountOfReqTokens, p->pToken);
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
			p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
			if (!p->pToken || strcmp(p->pToken, RDS_VERSION)) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - RDS version mismatch, '%s' required, but client supports '%s'", __FUNCTION__, RDS_VERSION, p->pToken);
				lRet = RTSP_RDS_VERSION_MISMATCH;
				p->m_nRTSPReponseCode = 505;
				_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "RDS version mismatch, '%s' required, but client supports '%s'", RDS_VERSION, p->pToken?p->pToken:"");
			}
			break;
		case RScale:
			break;
		case RSession:
			p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
			if (!p->pToken || (p->m_szSession[0] && strncmp(p->m_szSession, p->pToken, RTSP_SESSION_ID_LEN))) {
				lRet = RTSP_SESSION_MISMATCH;
				p->m_nRTSPReponseCode = 454;
				_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Unextected session name: '%s', expected: '%s'", p->pToken?p->pToken:"", p->m_szSession);
			}
			
			break;
		case RSpeed:
			break;
		case RTransport:
			{
				memset(&p->m_playStruct, 0, sizeof(p->m_playStruct));

				p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
				strncpy(p->m_playStruct.szValue, p->pToken, RTSP_MAX_TRANSPORT_LEN);
				p->m_playStruct.szValue[RTSP_MAX_TRANSPORT_LEN-1] = 0; // make sure it's null terminated

				if (p->pToken) {
					char* pTok=strstr(p->pToken, "client_port=");
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
							_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Received unsupported mode (%s) from rtsp sink", pTok);
							RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
						}
					} else {
						p->m_nRTSPReponseCode = 406;
						_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "mode required but not found in Transport header (%s) from client", p->pToken);
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
					}
				}
			}
			break;
		case RUser_Agent :
			break;
	}

	return lRet;
}

long CheckForResponseHeader(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	BOOL bRds = FALSE;
	char szTemp[50];

	p->nToken = MatchToken(g_szRespTokens, CountOfRespTokens, p->pToken);
	switch (p->nToken) {
		case rAllow:
		case rLocation:
		case rProxy_Authenticate:
			break;
		case rPublic:
			// following are the RTSP methods supported
			while ((p->pToken = strtok_s(NULL, ", ", &(p->m_pStrtokContext)))) {
				int nMethod = MatchToken(g_szMethodTokens, CountOfMethodTokens, p->pToken);

				if (0 <= nMethod)
					p->m_bPeerSupportedMethods[nMethod] = TRUE;
				else if (!strncmp(p->pToken, RDS_VERSION, 13)) {
					strncpy(szTemp, p->pToken, 50);

					if (!strcmp(p->pToken, RDS_VERSION)) {
						bRds = TRUE;
					} else {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - RDS version mismatch while parsing '%s' method, '%s' required, but client supports '%s'", __FUNCTION__, g_szMethodTokens[p->nToken], RDS_VERSION, p->pToken);
						lRet = RTSP_RDS_VERSION_MISMATCH;
					}
				} else {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Supported method (%s) specified in Public: header is not valid", __FUNCTION__, p->pToken);
					lRet = RTSP_INVALID_METHOD;
				}
			}

			if (!bRds) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - RDS version mismatch while parsing '%s' method, '%s' required, but client supports '%s'", __FUNCTION__, g_szMethodTokens[p->nToken], RDS_VERSION, szTemp);
				lRet = RTSP_RDS_VERSION_MISMATCH;
			}
			break;
		case rRange:
		case rRetry_After:
		case rRTP_Info:
		case rScale:
			break;
		case rSession:
			p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
			strncpy(p->m_szSession, p->pToken, RTSP_SESSION_ID_LEN);
			p->m_szSession[RTSP_SESSION_ID_LEN] = 0;
			break;
		case rServer:
		case rSpeed:
			break;
		case rTransport:
			{
				memset(&p->m_playStruct, 0, sizeof(p->m_playStruct));

				p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
				strncpy(p->m_playStruct.szValue, p->pToken, RTSP_MAX_TRANSPORT_LEN);
				p->m_playStruct.szValue[RTSP_MAX_TRANSPORT_LEN-1] = 0; // make sure it's null terminated

				if (p->pToken) {
					char* pTok=strstr(p->pToken, "server_port=");
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

long CheckForGeneralHeader(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;

	p->nToken = MatchToken(g_szGenTokens, CountOfGenTokens, p->pToken);
	switch (p->nToken) {
		case GCache_Control:
		case GConnection:
			break;
		case GCSeq:
			{
				p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // CSeq number
				if (!p->pToken) {
					p->m_nRTSPReponseCode = 400;
					_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected length while parsing CSeq: but got none");
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
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

long CheckForEntityHeader(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;

	p->nToken = MatchToken(g_szEntTokens, CountOfEntTokens, p->pToken);
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
					_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected length while parsing Content-Length: but got none");
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
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
				_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected type while parsing Content-Type: but got none");
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
			} else if (strcmp(p->pToken, "text/parameters")) {
				p->m_nRTSPReponseCode = 400;
				_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Expected test/parameters while parsing Content-Type: but got (%s)", p->pToken);
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
			}
			break;
		case EExpires:
		case ELast_Modified:
			break;
	}

	return lRet;
}

