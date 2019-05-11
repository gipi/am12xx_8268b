/**
 * @file: getparam.c
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
#include "wfdparser.h"

long WFD_SendGetParameters(CRTSPParser* p, char* szParams)
{
	long lRet = WFD_RTSP_NO_ERROR;
	WFD_NODE* pBuf = NULL;
	long lLen = 0;

	if (szParams)
		lLen = strlen(szParams);

	GET_AND_CHECK(pBuf,WFD_QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MGET_PARAMETER;
	if (szParams && 0 == szParams[0])
		pBuf->bKeepalive = TRUE;
	else
		pBuf->bKeepalive = FALSE;

	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (p->m_szSession[0]) {
		if (lLen) {
			if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
				"GET_PARAMETER rtsp://%s/%s %s\r\n"
				"CSeq: %d\r\n"
				"Content-Type: text/parameters\r\n"
				"Content-Length: %d\r\n"
				"Session: %s\r\n\r\n"
				"%s", 
				"localhost", RTSP_SHORT_TAG, RTSP_VERSION, 
				p->m_nSendCSeq++,
				lLen, 
				p->m_szSession, 
				szParams))
			{
				return WFD_RTSP_INTERNAL_ERROR; // this should never occur
			}
		} else {
			if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
				"GET_PARAMETER rtsp://%s/%s %s\r\n"
				"CSeq: %d\r\n"
				"Session: %s\r\n\r\n",
				"localhost", RTSP_SHORT_TAG, RTSP_VERSION, 
				p->m_nSendCSeq++,
				p->m_szSession))
			{
				return WFD_RTSP_INTERNAL_ERROR; // this should never occur
			}
		}
	} else {
		if (lLen) {
			if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
				"GET_PARAMETER rtsp://%s/%s %s\r\n"
				"CSeq: %d\r\n"
				"Content-Type: text/parameters\r\n"
				"Content-Length: %d\r\n\r\n"
				"%s", 
				"localhost", RTSP_SHORT_TAG, RTSP_VERSION, 
				p->m_nSendCSeq++, 
				lLen, szParams))
			{
				return WFD_RTSP_INTERNAL_ERROR; // this should never occur
			}
		} else {
			if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
				"GET_PARAMETER rtsp://%s/%s %s\r\n"
				"CSeq: %d\r\n",
				"localhost", RTSP_SHORT_TAG, RTSP_VERSION, 
				p->m_nSendCSeq++))
			{
				return WFD_RTSP_INTERNAL_ERROR; // this should never occur
			}
		}
	}

	lRet = WFD_Send(p, (char*)pBuf->aBuffer);

	if (WFD_RTSP_NO_ERROR == lRet)
		WFD_QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long WFD_SendGetParametersResponse(CRTSPParser* p, int nCSeq, char* szGetParamsResp)
{
	return WFD_SendResponse(p, nCSeq, 200, "OK", "", szGetParamsResp);
}

