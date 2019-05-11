/**
 * @file: teardown.c
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

long RTSP_Teardown(CRTSPParser* p)
{
	if (p->m_bServer)
		return SendTeardownTrigger(p);
	else
		return SendTeardown(p);
}

long SendTeardownTrigger(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	char szBody[RTSP_SEND_BUFFER];
	int nLen = _snprintf_(szBody, RTSP_SEND_BUFFER, "rds_trigger_method: TEARDOWN\r\n");

	NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MSET_PARAMETER;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
		"SET_PARAMETER rtsp://%s/%s %s\r\n"
		"CSeq: %d\r\n"
		"Content-Type: text/parameters\r\n"
		"Content-Length: %d\r\n"
		"Session: %s\r\n\r\n"
		"%s",
		p->m_bLegacy?p->m_szIpAddress:"localhost", RDS_SHORT_VERSION, RTSP_VERSION, 
		p->m_nSendCSeq++,
		nLen,
		p->m_szSession,
		szBody
		))
		return RTSP_INTERNAL_ERROR; // this should never occur

	lRet = _Send(p, (char*)pBuf->aBuffer);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendTeardown(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MTEARDOWN;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
		"TEARDOWN rtsp://%s/%s/streamid=0 %s\r\n"
		"CSeq: %d\r\n"
		"Session: %s\r\n",
		p->m_szServerIpAddress, RDS_SHORT_VERSION, RTSP_VERSION, 
		p->m_nSendCSeq++,
		p->m_szSession))
	{
		return RTSP_INTERNAL_ERROR; // this should never occur
	}

	lRet = SendRequest(p, (char*)pBuf->aBuffer, NULL);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendTeardownTriggerResponse(CRTSPParser* p, int nCSeq)
{
	return SendResponse(p, nCSeq, 200, "OK", NULL, NULL);
}

long SendTeardownResponse(CRTSPParser* p, int nCSeq)
{
	long lRet = SendResponse(p, nCSeq, 200, "OK", NULL, NULL);

	// init session to null
	p->m_szSession[0] = 0;

	//RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - To Send = %d", __FUNCTION__, QueueQueuedBufferCount(&p->m_bqToSend));
	//RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - Sent = %d", __FUNCTION__, QueueQueuedBufferCount(&p->m_bqSent));

	return lRet;
}

