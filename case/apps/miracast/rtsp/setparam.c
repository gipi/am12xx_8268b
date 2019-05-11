/**
 * @file: setparam.c
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

long SendSetParameters(CRTSPParser* p, char* szParams)
{
	long lRet = RTSP_NO_ERROR;
	NODE* pBuf = NULL;

	if (p->m_bLegacy && !p->m_bServer && p->m_bRtspConnected) {
		// LEGACY: stack does not support receiving SET_PARAMETER from snk
		return RTSP_NO_ERROR;
	}

	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MSET_PARAMETER;
	pBuf->bKeepalive = FALSE;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (strstr(szParams, g_szRDSParams[rds_keepalive])) {
		pBuf->bKeepalive = TRUE;
	}

	if (p->m_szSession[0]) {
		if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
			"SET_PARAMETER rtsp://%s/%s %s\r\n"
			"CSeq: %d\r\n"
			"Content-Type: text/parameters\r\n"
			"Content-Length: %d\r\n"
			"Session: %s\r\n\r\n"
			"%s", 
			p->m_bLegacy?p->m_szIpAddress:"localhost", RDS_SHORT_VERSION, RTSP_VERSION, 
			p->m_nSendCSeq++, 
			strlen(szParams), 
			p->m_szSession,
			szParams))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}
	} else {
		if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
			"SET_PARAMETER rtsp://%s/%s %s\r\n"
			"CSeq: %d\r\n"
			"Content-Type: text/parameters\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s", 
			p->m_bLegacy?p->m_szIpAddress:"localhost", RDS_SHORT_VERSION, RTSP_VERSION, 
			p->m_nSendCSeq++, 
			strlen(szParams), szParams))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}
	}

	lRet = _Send(p, (char*)pBuf->aBuffer);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendSetParameter(CRTSPParser* p, RDSParams rdsParam, char* szParams)
{
	long lRet = RTSP_NO_ERROR;
	NODE* pBuf = NULL;

	if (p->m_bLegacy && !p->m_bServer && p->m_bRtspConnected) {
		// LEGACY: stack does not support receiving SET_PARAMETER from snk, so don't send it
		return RTSP_NO_ERROR;
	}

	GET_AND_CHECK(pBuf,QueueGetBuffer(&p->m_bqToSend, p));

	SET_CSEQ;
	pBuf->nMethod = MSET_PARAMETER;
	pBuf->bKeepalive = FALSE;
	pBuf->nTimeSent = p->m_pGetTick(p->m_pContext);

	if (strstr(szParams, g_szRDSParams[rds_keepalive])) {
		pBuf->bKeepalive = TRUE;
	}

	if (p->m_szSession[0]) {
		if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
			"SET_PARAMETER rtsp://%s/%s %s\r\n"
			"CSeq: %d\r\n"
			"Content-Type: text/parameters\r\n"
			"Content-Length: %d\r\n"
			"Session: %s\r\n\r\n"
			"%s: %s", 
			p->m_bLegacy?p->m_szIpAddress:"localhost", RDS_SHORT_VERSION, RTSP_VERSION, 
			p->m_nSendCSeq++, 
			strlen(g_szRDSParams[rdsParam]) + 2 + strlen(szParams), 
			p->m_szSession,
			g_szRDSParams[rdsParam], szParams))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}
	} else {
		if (0 > _snprintf_((char*)pBuf->aBuffer, pBuf->nAllocated,
			"SET_PARAMETER rtsp://%s/%s %s\r\n"
			"CSeq: %d\r\n"
			"Content-Type: text/parameters\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s: %s", 
			p->m_bLegacy?p->m_szIpAddress:"localhost", RDS_SHORT_VERSION, RTSP_VERSION, 
			p->m_nSendCSeq++, 
			strlen(g_szRDSParams[rdsParam]) + 2 + strlen(szParams), 
			g_szRDSParams[rdsParam], szParams))
		{
			return RTSP_INTERNAL_ERROR; // this should never occur
		}
	}

	lRet = _Send(p, (char*)pBuf->aBuffer);

	if (RTSP_NO_ERROR == lRet)
		QueuePutBuffer(&p->m_bqSent, p, pBuf);

	return lRet;
}

long SendSetParametersResponse(CRTSPParser* p, int nCSeq)
{
	return SendResponse(p, nCSeq, 200, "OK", NULL, NULL);
}

