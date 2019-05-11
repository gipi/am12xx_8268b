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
#include "wfdparser.h"

long WFD_SendOptions(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	WFD_NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,WFD_QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MOPTIONS;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (0 > wfd_snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated, "OPTIONS * %s\r\nCSeq: %d\r\nRequire: %s\r\n\r\n", RTSP_VERSION, p->m_nSendCSeq++, RTSP_OPTION_TAG))
		return WFD_RTSP_INTERNAL_ERROR; // this should never occur

	lRet = WFD_Send(p, (char*)pBuf->aBuffer);

	if (WFD_RTSP_NO_ERROR == lRet)
		WFD_QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long WFD_SendOptionsResponse(CRTSPParser* p, int nCSeq)
{
	char szResp[RTSP_SEND_BUFFER];
	int nUsed=0;
	int nRet=0;
	int i;
	time_t tmNow;
	struct tm* ptm;
	size_t nLen;

	tmNow = time(&tmNow);
	ptm = gmtime(&tmNow);

	strftime(szResp, RTSP_SEND_BUFFER, "Date: %a, %b %d %Y %X GMT\r\n", ptm);
	nLen = strlen(szResp);

	nUsed = wfd_snprintf_(szResp+nLen, RTSP_SEND_BUFFER-nLen, "Public: %s", RTSP_OPTION_TAG);

	if (0 > nUsed)
		return WFD_RTSP_INTERNAL_ERROR; // this should never occur

	nUsed += nLen;

	for (i=0; i < CountOfWFD_MethodTokens; i++) {
		if (p->m_bMySupportedMethods[i]) {
			nRet = wfd_snprintf_(szResp+nUsed, RTSP_SEND_BUFFER-nUsed, ", %s", g_szWFD_MethodTokens[i]);

			if (0 > nUsed)
				return WFD_RTSP_INTERNAL_ERROR; // this should never occur

			nUsed += nRet;
			if (RTSP_SEND_BUFFER-4 < nUsed) // -4 for \r\n\0
				return WFD_RTSP_INTERNAL_ERROR; // this should never occur
		}
	}

	nRet = wfd_snprintf_(szResp+nUsed, RTSP_SEND_BUFFER-nUsed, "\r\n");

	if (0 > nUsed)
		return WFD_RTSP_INTERNAL_ERROR; // this should never occur

	return WFD_SendResponse(p, nCSeq, 200, "OK", szResp, NULL);
}


