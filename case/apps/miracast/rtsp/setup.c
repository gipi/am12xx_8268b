/**
 * @file: setup.c
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

long SendSetupTrigger(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;
	char szBody[RTSP_SEND_BUFFER];
	int nLen=0;

	NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MSET_PARAMETER;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (p->m_bLegacy) {
		nLen = _snprintf_(szBody, RTSP_SEND_BUFFER, 
			"rds_trigger_method: SETUP\r\n" 
			"rds_overscan_comp: x=%d, y=%d\r\n"
			"rds_presentation_URL: rtsp://%s/%s/streamid=0\r\n"
			"Description:\r\n"
			"v= 0\r\n"
			"o= rds 0 0 IN IP4 %s\r\n"
			"s= WIDIFWUpdate\r\n"
			"m= video %d RTP/AVPF 33\r\n"
			"a= rtcp-fb:* nack sli a= control:streamid=0",
			RDS_MAX_OVERSCAN_COMP-p->m_nOverscanComp, RDS_MAX_OVERSCAN_COMP-p->m_nOverscanComp, 
			p->m_szIpAddress, RDS_SHORT_VERSION,
			p->m_szIpAddress,
			67890);
	} else {
		nLen = _snprintf_(szBody, RTSP_SEND_BUFFER, 
			"rds_trigger_method: SETUP\r\n" 
			"rds_overscan_comp: x=%d, y=%d\r\n"
			"rds_presentation_URL: rtsp://%s/%s/streamid=0\r\n"
			"%s",
			RDS_MAX_OVERSCAN_COMP-p->m_nOverscanComp, RDS_MAX_OVERSCAN_COMP-p->m_nOverscanComp, 
			p->m_szIpAddress, RDS_SHORT_VERSION,
			p->m_pOptRdsSetupParams);
	}

	if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated, 
		"SET_PARAMETER rtsp://%s/%s %s\r\n"
		"CSeq: %d\r\n"
		"Content-Type: text/parameters\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s",
		p->m_bLegacy?p->m_szIpAddress:"localhost", RDS_SHORT_VERSION, RTSP_VERSION, 
		p->m_nSendCSeq++,
		nLen,
		szBody
		))
		return RTSP_INTERNAL_ERROR; // this should never occur

	lRet = _Send(p, (char*)pBuf->aBuffer);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendSetup(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;

	NODE* pBuf = NULL;
	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MSETUP;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (p->m_bLegacy) {
		if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated, 
			"SETUP rtsp://%s/%s/streamid=0 %s\r\n"
			"Transport: RTP/AVPF/UDP;unicast;client_port=%d-%d;mode=play\r\n"
			"CSeq: %d\r\n\r\n",
			p->m_szServerIpAddress, RDS_SHORT_VERSION, RTSP_VERSION, 
			p->m_usRTPPort, p->m_usRTPPort+1,
			p->m_nSendCSeq++))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}
	} else {
		if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated, 
			"SETUP rtsp://%s/%s/streamid=0 %s\r\n"
			"Transport: RTP/AVPF/UDP;unicast;client_port=%d-%d;mode=play\r\n"
			"CSeq: %d\r\n\r\n",
			p->m_szServerIpAddress, RDS_SHORT_VERSION, RTSP_VERSION, 
			p->m_usRTPPort, p->m_usRTPPort+1,
			p->m_nSendCSeq++))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}
	}

	lRet = _Send(p, (char*)pBuf->aBuffer);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendSetupTriggerResponse(CRTSPParser* p, int nCSeq)
{
	return SendResponse(p, nCSeq, 200, "OK", NULL, NULL);
}

long SendSetupResponse(CRTSPParser* p, int nCSeq)
{
	char szResp[RTSP_SEND_BUFFER];

	// generate session id
	int i,randIndex;

	char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ$_-+.1234567890abcdefghijklmnopqrstuvwxyz";
	int lenCharSet = (int)strlen(charSet) -1;

	srand((unsigned int)time(NULL)); //seed for random number

	//Generate Upper/Lower case Alphabetic values
	for (i = 0; i < RTSP_SESSION_ID_LEN; i++) {
		randIndex = rand() % lenCharSet;
		p->m_szSession[i] = charSet[randIndex];
	}
	p->m_szSession[i] = '\0';

	if (p->m_bLegacy) {
		// LEGACY: requires message headers to be in a certain order
		if (0 > _snprintf_(szResp, RTSP_SEND_BUFFER,
			"%s 200 OK\r\n"
			"Transport: RTP/AVPF/UDP;unicast;client_port=%d-%d;server_port=0-%d;mode=play\r\n"
			"CSeq: %d\r\n"
			"Session: %s\r\n",
			RTSP_VERSION,
			p->m_usRTPPort, p->m_usRTPPort+1, 
			p->m_usServerRtcpPort, 
			nCSeq,
			p->m_szSession))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}

		return _Send(p, szResp);
	} else {
		if (0 > _snprintf_(szResp, RTSP_SEND_BUFFER,
			"Transport: RTP/AVPF/UDP;unicast;client_port=%d-%d;server_port=0-%d;mode=play\r\n"
			"Session: %s\r\n",
			p->m_usRTPPort, p->m_usRTPPort+1,
			p->m_usServerRtcpPort, 
			p->m_szSession))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}

		return SendResponse(p, nCSeq, 200, "OK", szResp, NULL);
	}
}


