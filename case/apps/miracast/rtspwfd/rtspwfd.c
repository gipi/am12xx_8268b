/**
 * @file: rdsrtsp.c
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

#ifdef MEMCHK
#define MAX_MEMCHK	1000
FN_WFD_RTSP_LOG g_pfnLog=NULL;
void* g_pContext=NULL;
typedef struct {
	void* ptr;
	char szText[256];
} MEM_NODE;

MEM_NODE g_MemArr[MAX_MEMCHK];
unsigned long g_Used=0;
#endif

/*
 * @brief Initializes an RTSP sink.
 * @param[out] phRtsp       Pointer to an object created by the 
 *                          initialization function and passed to all
 *                          subsequent RTSP function calls that take
 *                          a void* hRtsp parameter.
 * @param[in]  usRtpPort    RTP port number to be sent to the source
 *                          in the SETUP method.
 * @param[in]  pfnCallback  Pointer to a function that will receive
 *                          asynchronous notifications from the stack.
 * @param[in]  pfnRecv      Pointer to a function that will receive a
 *                          TCP message from the RTSP source.
 * @param[in]  pfnSend      Pointer to a function that will send the
 *                          specified payload via TCP to the RTSP
 *                          source.
 * @param[in]  pfnSleep     Pointer to a function that will sleep for
 *                          the specified number of milliseconds.
 * @param[in]  pfnGetTick   Pointer to a function that returns the 
 *                          number of milliseconds that have elapsed
 *                          since the system was started.
 * @param[in]  pfnLog       Pointer to a function that that takes a
 *                          string that the rtsp code would like logged.
 * @param[in]  pContext     Pointer provided by the caller that will
 *                          be passed back to the callback functions
 *                          (pfnCallback, pfnRecv, and pfnSend).  The
 *                          library does not use this value for
 *                          anything, it is provided solely as a
 *                          convenience for the caller.
 */
long CALLING_CONVENTION WFD_RTSPSnkInit(
	void** phRtsp, 
	unsigned short usRtpPort,
	FN_WFD_RTSP_NOTIFICATIONS pfnCallback, 
	FN_WFD_RTSP_RECV pfnRecv, 
	FN_WFD_RTSP_SEND pfnSend, 
	FN_WFD_RTSP_SLEEP pfnSleep, 
	FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, 
	FN_WFD_RTSP_LOG pfnLog, 
	void* pContext)
{
#ifdef MEMCHK
	{
		g_pfnLog = pfnLog;
		g_pContext = pContext;
		memset(g_MemArr, 0, sizeof(g_MemArr));
		g_Used = 0;
	}
	{
#endif
	CRTSPParser* h = WFD_RTSP_Alloc(FALSE);
	if (!h) {
		return WFD_RTSP_MEMORY;
	}

	*phRtsp = (void*)h;
	return WFD_RTSP_Init(h, usRtpPort, 0, WFD_SESSION_TIMEOUT_DEFAULT, NULL, pfnCallback, pfnRecv, pfnSend, pfnSleep, pfnGetTick, pfnLog, pContext);
#ifdef MEMCHK
	}
#endif
}

/*
 * @brief Initializes an RTSP source.
 * @param[out] phRtsp       Pointer to an object created by the 
 *                          initialization function and passed to all
 *                          subsequent RTSP function calls that take
 *                          a void* hRtsp parameter.
 * @param[in]  usServerRtcpPort
 *                          RTCP port number, passed to the receiver
 *                          so it can send RTCP messages back to the 
 *                          sender.  RTCP port number must be odd.
 * @param[in]  ulKeepaliveTimeout    Keepalive period in msec.
 * @param[in]  pfnCallback  Pointer to a function that will receive
 *                          asynchronous notifications from the stack.
 * @param[in]  pfnRecv      Pointer to a function that will receive a
 *                          TCP message from the RTSP source.
 * @param[in]  pfnSend      Pointer to a function that will send the
 *                          specified payload via TCP to the RTSP
 *                          source.
 * @param[in]  pfnSleep     Pointer to a function that will sleep for
 *                          the specified number of milliseconds.
 * @param[in]  pfnGetTick   Pointer to a function that returns the 
 *                          number of milliseconds that have elapsed
 *                          since the system was started.
 * @param[in]  pfnLog       Pointer to a function that that takes a
 *                          string that the rtsp code would like logged.
 * @param[in]  pContext     Pointer provided by the caller that will
 *                          be passed back to the callback functions
 *                          (pfnCallback, pfnRecv, and pfnSend).  The
 *                          library does not use this value for
 *                          anything, it is provided solely as a
 *                          convenience for the caller.
 */
long CALLING_CONVENTION WFD_RTSPSrcInit(
	void** phRtsp,
	unsigned short usServerRtcpPort,
	unsigned long ulKeepaliveTimeout,
	char* pInitialGetParams, 
	FN_WFD_RTSP_NOTIFICATIONS pfnCallback, 
	FN_WFD_RTSP_RECV pfnRecv, 
	FN_WFD_RTSP_SEND pfnSend, 
	FN_WFD_RTSP_SLEEP pfnSleep, 
	FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, 
	FN_WFD_RTSP_LOG pfnLog, 
	void* pContext)
{
#ifdef MEMCHK
	{
		g_pfnLog = pfnLog;
		g_pContext = pContext;
		memset(g_MemArr, 0, sizeof(g_MemArr));
		g_Used = 0;
	}
	{
#endif
	
	CRTSPParser* h = WFD_RTSP_Alloc(TRUE);
	if (!h) {
		return WFD_RTSP_MEMORY;
	}

	if (10000 > ulKeepaliveTimeout) {
		return WFD_RTSP_KEEPALIVE_PERIOD_TOO_SHORT;
	}

	*phRtsp = (void*)h;

	return WFD_RTSP_Init(h, 0, usServerRtcpPort, ulKeepaliveTimeout, pInitialGetParams, pfnCallback, pfnRecv, pfnSend, pfnSleep, pfnGetTick, pfnLog, pContext);
#ifdef MEMCHK
	}
#endif
}

long CALLING_CONVENTION WFD_RTSPSrcAddress(void* hRtsp, char* szIpAddr)
{
	return WFD_RTSP_SetSrcAddress(hRtsp, szIpAddr);
}

long CALLING_CONVENTION WFD_RTSPParser(void* hRtsp)
{
	long lRet = WFD_RTSP_Parser(hRtsp);

	return lRet;
}

long CALLING_CONVENTION WFD_RTSPGetState(void* hRtsp)
{
	long lRet = WFD_RTSP_State(hRtsp);

	return lRet;
}

long CALLING_CONVENTION WFD_RTSPShutdown(void* hRtsp)
{
	long lRet = WFD_RTSP_Shutdown(hRtsp);

	if (WFD_RTSP_NO_ERROR == lRet)
		WFD_RTSP_Free(hRtsp);

#ifdef MEMCHK
	{
		int i;

		for (i=0; i < g_Used; i++) {
			if (g_MemArr[i].ptr) {
				char szTmp[400];
				sprintf(szTmp, "LEAK: mem=0x%x, [%s]\n", g_MemArr[i].ptr, g_MemArr[i].szText);
				g_pfnLog(RTSP_LOG_DEBUG, szTmp, g_pContext);
			}
		}
	}
#endif
	return lRet;
}

long CALLING_CONVENTION WFD_RTSPSetParam(void* hRtsp, char* szParams)
{
	return WFD_RTSP_SetParam(hRtsp, szParams);
}

long CALLING_CONVENTION WFD_RTSPGetParam(void* hRtsp, char* szParams)
{
	return WFD_RTSP_GetParam(hRtsp, szParams);
}

long CALLING_CONVENTION WFD_RTSPSetParamBinary(void* hRtsp, WFDParams id, void* pData, long nDataLen)
{
	return WFD_RTSP_SetParamBinary(hRtsp, id, pData, nDataLen);
}

long CALLING_CONVENTION WFD_RTSPSetParamStruct(void* hRtsp, WFD_PARAMETER_STRUCT* pStruct, int nCount)
{
	return WFD_RTSP_SetParamStruct(hRtsp, pStruct, nCount);
}

long CALLING_CONVENTION WFD_RTSPPlay(void* hRtsp)
{
	return WFD_RTSP_Play(hRtsp);
}

long CALLING_CONVENTION WFD_RTSPPause(void* hRtsp)
{
	return WFD_RTSP_Pause(hRtsp);
}

long CALLING_CONVENTION WFD_RTSPTeardown(void* hRtsp)
{
	return WFD_RTSP_Teardown(hRtsp);
}

long CALLING_CONVENTION WFD_RTSPResetIncomingCSeq(void* hRtsp)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;

	p->m_nRecvCSeq = -1;

	return WFD_RTSP_NO_ERROR;
}

long CALLING_CONVENTION WFD_RTSPDecodeMIME(void* hRtsp, unsigned char* pDst, long* pDstLen, char* pSrc, long nSrcLen)
{
	return WFD_DecodeMIME((CRTSPParser*)hRtsp, pDst, pDstLen, (unsigned char*)pSrc, nSrcLen);
}

long CALLING_CONVENTION WFD_RTSPEncodeMIME(void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen)
{
	return WFD_EncodeMIME((CRTSPParser*)hRtsp, (unsigned char*)pDst, pDstLen, pSrc, nSrcLen);
}

long CALLING_CONVENTION WFD_RTSPSetResponseCodeAndMessage(void* hRtsp, unsigned long ulResponseCode, char* szFormat, ...)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;
	size_t nLen;
	va_list args;
	va_start (args, szFormat);

	p->m_nRTSPReponseCode = ulResponseCode;
	if (szFormat) {
		nLen = vsnprintf(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, szFormat, args);

		if (nLen > 0) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
		}
	} else
		p->m_szErrorMessage[0] = 0;

	return WFD_RTSP_NO_ERROR;
}

#ifdef MEMCHK

#undef malloc
#undef calloc
#undef realloc
#undef free

void* wfdmalloc(char* file, char* func, int line, unsigned long size)
{
	int i;
	void* p=malloc(size);

	for (i=0; i < g_Used; i++) {
		if (!g_MemArr[i].ptr) {
			sprintf(g_MemArr[i].szText, "*** MALLOC: %s: %s: %d, mem=0x%p, size=%d", file, func, line, p, size);
			g_pfnLog(RTSP_LOG_DEBUG, g_MemArr[i].szText, g_pContext);
			g_MemArr[i].ptr = p;
			break;
		}
	}
	if (i == g_Used && g_Used < MAX_MEMCHK) {
		g_MemArr[g_Used].ptr = p;
		sprintf(g_MemArr[g_Used].szText, "*** MALLOC: %s: %s: %d, mem=0x%p, size=%d", file, func, line, p, size);
		g_pfnLog(RTSP_LOG_DEBUG, g_MemArr[i].szText, g_pContext);
		g_Used++;
	}

	return p;
}

void* wfdcalloc(char* file, char* func, int line, unsigned long size, unsigned int cnt)
{
	int i;
	void* p=calloc(size, cnt);

	for (i=0; i < g_Used; i++) {
		if (!g_MemArr[i].ptr) {
			sprintf(g_MemArr[g_Used].szText, "*** CALLOC: %s: %s: %d, mem=0x%p, size=%d", file, func, line, p, size*cnt);
			g_pfnLog(RTSP_LOG_DEBUG, g_MemArr[i].szText, g_pContext);
			g_MemArr[i].ptr = p;
			break;
		}
	}
	if (i == g_Used && g_Used < MAX_MEMCHK) {
		g_MemArr[g_Used].ptr = p;
		sprintf(g_MemArr[g_Used].szText, "*** CALLOC: %s: %s: %d, mem=0x%p, size=%d", file, func, line, p, size*cnt);
		g_pfnLog(RTSP_LOG_DEBUG, g_MemArr[i].szText, g_pContext);
		g_Used++;
	}

	return p;
}

void* wfdrealloc(char* file, char* func, int line, void* ptr, unsigned long size)
{
	int i;
	void* p=realloc(ptr, size);
	for (i=0; i < g_Used; i++) {
		if (g_MemArr[i].ptr == ptr) {
			sprintf(g_MemArr[i].szText, "*** REALLOC: %s: %s: %d, mem=0x%p, size=%d", file, func, line, p, size);
			g_pfnLog(RTSP_LOG_DEBUG, g_MemArr[i].szText, g_pContext);
			g_MemArr[i].ptr = p;
			break;
		}
	}
	return p;
}

void wfdfree(char* file, char* func, int line, void* ptr)
{
	int i;
	for (i=0; i < g_Used; i++) {
		if (g_MemArr[i].ptr == ptr) {
			char szTmp[400];
			g_MemArr[i].ptr = NULL;
			sprintf(szTmp, "*** FREE: %s: %s: %d, mem=0x%p, [%s]\n", file, func, line, ptr, g_MemArr[i].szText);
			g_pfnLog(RTSP_LOG_DEBUG, szTmp, g_pContext);
			break;
		}
	}
	free(ptr);
}

#endif
