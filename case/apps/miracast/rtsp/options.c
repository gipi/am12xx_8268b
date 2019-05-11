/**
 * @file: options.c
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

long SendOptions(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MOPTIONS;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated, "OPTIONS * %s\r\nCSeq: %d\r\nRequire: %s\r\n\r\n", RTSP_VERSION, p->m_nSendCSeq++, RDS_VERSION))
		return RTSP_INTERNAL_ERROR; // this should never occur

	lRet = _Send(p, (char*)pBuf->aBuffer);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendOptionsResponse(CRTSPParser* p, int nCSeq)
{
	char szResp[RTSP_SEND_BUFFER];
	int nUsed=0;
	int nRet=0;
	int i;

	nUsed = _snprintf_(szResp, RTSP_SEND_BUFFER, "Public: %s", RDS_VERSION);

	if (0 > nUsed)
		return RTSP_INTERNAL_ERROR; // this should never occur

	for (i=0; i < CountOfMethodTokens; i++) {
		if (p->m_bMySupportedMethods[i]) {
			nRet = _snprintf_(szResp+nUsed, RTSP_SEND_BUFFER-nUsed, ", %s", g_szMethodTokens[i]);

			if (0 > nUsed)
				return RTSP_INTERNAL_ERROR; // this should never occur

			nUsed += nRet;
			if (RTSP_SEND_BUFFER-4 < nUsed) // -4 for \r\n\0
				return RTSP_INTERNAL_ERROR; // this should never occur
		}
	}

	nRet = _snprintf_(szResp+nUsed, RTSP_SEND_BUFFER-nUsed, "\r\n");

	if (0 > nUsed)
		return RTSP_INTERNAL_ERROR; // this should never occur

	return SendResponse(p, nCSeq, 200, "OK", szResp, NULL);
}


