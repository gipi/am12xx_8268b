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
 * @param[in]  pContext     Pointer provided by the caller that will
 *                          be passed back to the callback functions
 *                          (pfnCallback, pfnRecv, and pfnSend).  The
 *                          library does not use this value for
 *                          anything, it is provided solely as a
 *                          convenience for the caller.
 */
long CALLING_CONVENTION RTSPSnkInit(
	void** phRtsp, 
	unsigned short usRtpPort, 
	FN_RTSP_NOTIFICATIONS pfnCallback, 
	FN_RTSP_RECV pfnRecv, 
	FN_RTSP_SEND pfnSend, 
	FN_RTSP_SLEEP pfnSleep, 
	FN_RTSP_GET_MSEC_TICK pfnGetTick, 
	FN_RTSP_LOG pfnLog, 
	void* pContext)
{
	CRTSPParser* h = RTSP_Alloc(FALSE);
	if (!h) {
		RTSP_LOG(h, RTSP_LOG_ERR, "%s - CRDSParser_Alloc() failed");
		return RTSP_MEMORY;
	}

	*phRtsp = (void*)h;

	return RTSP_Init(h, usRtpPort, 0, 0, NULL, pfnCallback, pfnRecv, pfnSend, pfnSleep, pfnGetTick, pfnLog, pContext);
}

/*
 * @brief Initializes an RTSP sink.
 * @param[out] phRtsp       Pointer to an object created by the 
 *                          initialization function and passed to all
 *                          subsequent RTSP function calls that take
 *                          a void* hRtsp parameter.
 * @param[in]  usServerRtcpPort
 *                          RTCP port number, passed to the receiver
 *                          so it can send RTCP messages back to the 
 *                          sender.  RTCP port number must be odd.
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
 * @param[in]  pContext     Pointer provided by the caller that will
 *                          be passed back to the callback functions
 *                          (pfnCallback, pfnRecv, and pfnSend).  The
 *                          library does not use this value for
 *                          anything, it is provided solely as a
 *                          convenience for the caller.
 */
long CALLING_CONVENTION RTSPSrcInit(
	void** phRtsp,
	unsigned short usServerRtcpPort, 
	int nOverscanComp,
	char* pOptRdsSetupParams, 
	FN_RTSP_NOTIFICATIONS pfnCallback, 
	FN_RTSP_RECV pfnRecv, 
	FN_RTSP_SEND pfnSend, 
	FN_RTSP_SLEEP pfnSleep, 
	FN_RTSP_GET_MSEC_TICK pfnGetTick, 
	FN_RTSP_LOG pfnLog, 
	void* pContext)
{
	CRTSPParser* h = RTSP_Alloc(TRUE);
	if (!h) {
		RTSP_LOG(h, RTSP_LOG_ERR, "%s - CRDSParser_Alloc() failed");
		return RTSP_MEMORY;
	}

	*phRtsp = (void*)h;

	return RTSP_Init(h, 0, usServerRtcpPort, nOverscanComp, pOptRdsSetupParams, pfnCallback, pfnRecv, pfnSend, pfnSleep, pfnGetTick, pfnLog, pContext);
}

long CALLING_CONVENTION RTSPSrcUpdateParams(void* hRtsp, int nOverscanComp, char* pOptRdsSetupParams)
{
	return RTSP_UpdateParams(hRtsp, nOverscanComp, pOptRdsSetupParams);
}

long CALLING_CONVENTION RTSPSrcAddress(void* hRtsp, char* szIpAddr)
{
	return RTSP_SetSrcAddress(hRtsp, szIpAddr);
}


long CALLING_CONVENTION RTSPSetLegacy(void* hRtsp, int bLegacy)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;

	p->m_bLegacy = bLegacy;

	return RTSP_NO_ERROR;
}

long CALLING_CONVENTION RTSPGetLegacy(void* hRtsp, int* pbLegacy)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;

	*pbLegacy = p->m_bLegacy;

	return RTSP_NO_ERROR;
}

long CALLING_CONVENTION RTSPParser(void* hRtsp)
{
	long lRet = RTSP_Parser(hRtsp);

	return lRet;
}

long CALLING_CONVENTION RTSPShutdown(void* hRtsp)
{
	long lRet = RTSP_Shutdown(hRtsp);

	if (RTSP_NO_ERROR == lRet)
		RTSP_Free(hRtsp);

	return lRet;
}

long CALLING_CONVENTION RTSPSetParam(void* hRtsp, char* szParams)
{
	return RTSP_SetParam(hRtsp, szParams);
}

long CALLING_CONVENTION RTSPGetParam(void* hRtsp, char* szParams)
{
	return RTSP_GetParam(hRtsp, szParams);
}

long CALLING_CONVENTION RTSPSetParamBinary(void* hRtsp, RDSParams id, void* pData, long nDataLen)
{
	return RTSP_SetParamBinary(hRtsp, id, pData, nDataLen);
}

long CALLING_CONVENTION RTSPSetParamStruct(void* hRtsp, PARAMETER_STRUCT* pStruct)
{
	return RTSP_SetParamStruct(hRtsp, pStruct);
}

long CALLING_CONVENTION RTSPTeardown(void* hRtsp)
{
	return RTSP_Teardown(hRtsp);
}

long CALLING_CONVENTION RTSPResetIncomingCSeq(void* hRtsp)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;

	p->m_nRecvCSeq = -1;

	return RTSP_NO_ERROR;
}

long CALLING_CONVENTION RTSPDecodeMIME(void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen)
{
	return DecodeMIME(hRtsp, (unsigned char*)pDst, pDstLen, pSrc, nSrcLen);
}

long CALLING_CONVENTION RTSPEncodeMIME(void* hRtsp, char* pDst, int* pDstLen, unsigned char* pSrc, int nSrcLen)
{
	return EncodeMIME(hRtsp, (unsigned char*)pDst, pDstLen, pSrc, nSrcLen);
}


