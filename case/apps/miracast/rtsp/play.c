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
#include "rtspparser.h"


long SendPlay(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	NODE* pBuf = NULL;

	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MPLAY;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
		"PLAY rtsp://%s/%s/streamid=0 %s\r\n"
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

long SendPlayResponse(CRTSPParser* p, int nCSeq)
{
	return SendResponse(p, nCSeq, 200, "OK", NULL, NULL);
}

