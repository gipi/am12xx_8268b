/**
 * @file: play.c
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

long WFD_RTSP_Pause(CRTSPParser* p)
{
	if (p->m_bServer)
		return WFD_SendPauseTrigger(p);
	else
		return WFD_SendPause(p);
}

long WFD_SendPause(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	WFD_NODE* pBuf = NULL;

	GET_AND_CHECK(pBuf,WFD_QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MPAUSE;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
		"PAUSE rtsp://%s/%s/streamid=0 %s\r\n"
		"CSeq: %d\r\n"
		"Session: %s\r\n",
		p->m_szServerIpAddress, RTSP_SHORT_TAG, RTSP_VERSION, 
		p->m_nSendCSeq++,
		p->m_szSession))
	{
		return WFD_RTSP_INTERNAL_ERROR; // this should never occur
	}

	lRet = WFD_SendRequest(p, (char*)pBuf->aBuffer, NULL);

	if (WFD_RTSP_NO_ERROR == lRet)
		WFD_QueuePutBuffer(&p->m_bqSent, p, pBuf);

	// sink transition to M10
	p->m_eState = WFD_RTSP_M10;
	p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

	return lRet;
}

long SendPauseResponse(CRTSPParser* p, int nCSeq)
{
	long lRet = WFD_SendResponse(p, nCSeq, 200, "OK", NULL, NULL);

	if (WFD_RTSP_NO_ERROR == lRet) {
		// sink transition to M10
		p->m_eState = WFD_RTSP_M10;
		p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);
	}

	return lRet;
}

long WFD_SendPauseTrigger(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	char szBody[RTSP_SEND_BUFFER];
	int nLen = wfd_snprintf_(szBody, RTSP_SEND_BUFFER, "wfd_trigger_method: PAUSE\r\n");

	WFD_NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,WFD_QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MSET_PARAMETER;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
		"SET_PARAMETER rtsp://%s/%s %s\r\n"
		"CSeq: %d\r\n"
		"Content-Type: text/parameters\r\n"
		"Content-Length: %d\r\n"
		"Session: %s\r\n\r\n"
		"%s",
		"localhost", RTSP_SHORT_TAG, RTSP_VERSION, 
		p->m_nSendCSeq++,
		nLen,
		p->m_szSession,
		szBody
		))
		return WFD_RTSP_INTERNAL_ERROR; // this should never occur

	lRet = WFD_Send(p, (char*)pBuf->aBuffer);

	if (WFD_RTSP_NO_ERROR == lRet)
		WFD_QueuePutBuffer(&p->m_bqSent, p, pBuf);

	// trigger is M5
	p->m_eState = WFD_RTSP_M05;
	p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

	return lRet;
}

long WFD_SendPauseTriggerResponse(CRTSPParser* p, int nCSeq)
{
	return WFD_SendResponse(p, nCSeq, 200, "OK", NULL, NULL);
}

