/**
 * @file: parse_response.c
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

long WFD_ParseMessageResponse(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
	WFD_NODE* pSent = NULL;
	
	p->pToken=NULL;
	p->nStrikes=0;
	p->nToken=0;
	p->nCSeq=-1;

	GET_AND_CHECK(pSent,WFD_QueueGetBuffer(&p->m_bqSent, p));
	pSent->nUsed = 0;
	p->nMethod = -pSent->nMethod;

	pSent->fRTSPVer = (float)atof((char*)p->m_pInputBuffer+5);

	p->pToken = strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext)); // RTSP/m.n
	
	p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext)); // next token
#if 1
	/**
	* Modify by simon Lee as i think the original version
	* could not work.
	*/
	if (p->pToken) {
		if (!strcmp(p->pToken, "200")) {
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext));
			if (!p->pToken || strcmp(p->pToken, "OK")) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Response received from peer '%s'", __FUNCTION__, p->pToken?p->pToken:"");
			} else {
				p->m_nRTSPReponseCode = 200;
			}
		} else {
			p->m_nRTSPReponseCode = atoi(p->pToken);
 
			p->pToken = strtok_s(NULL,  " ", &(p->m_pStrtokContext));
			if (!p->pToken || strcmp(p->pToken, "OK")) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - ERROR Response received from peer '%s'", __FUNCTION__, p->pToken?p->pToken:"");
			}
		}
		//lRet = WFD_RTSP_ERROR_RESPONSE;
		//goto done;
	}
#else
	if (!p->pToken) {
		if (!strcmp(p->pToken, "200")) {
			p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
			if (!p->pToken || strcmp(p->pToken, "OK")) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Response received from peer '%s'", __FUNCTION__, p->pToken?p->pToken:"");
			} else {
				p->m_nRTSPReponseCode = 200;
			}
		} else {
			p->m_nRTSPReponseCode = atoi(p->pToken);
 
			p->pToken = strtok_s(NULL,  "", &(p->m_pStrtokContext));
			if (!p->pToken || strcmp(p->pToken, "OK")) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - ERROR Response received from peer '%s'", __FUNCTION__, p->pToken?p->pToken:"");
			}
		}
		//lRet = WFD_RTSP_ERROR_RESPONSE;
		//goto done;
	}
#endif

	lRet = WFDReadNextLine(p);
	if (WFD_RTSP_NO_ERROR != lRet)
		goto done;

	p->pToken = strtok_s((char*)p->m_pInputBuffer,  " ", &(p->m_pStrtokContext));

	// next there should be 0 or more general-header|request-header|entity-header
	do {
		// we will bail out of this loop when we get an empty line
		if (0 == p->m_nLineLen)
			break;

		// first check for general-headers
		lRet = WFD_CheckForGeneralHeader(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto done;

		if ((int)GCSeq == p->nToken && pSent->nCSeq != p->nCSeq) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected CSeq: %d in response, but got: %d instead", __FUNCTION__, pSent->nCSeq, p->nCSeq);
			lRet = WFD_RTSP_CSEQ_MISMATCH;
			goto done;
		}

		lRet = WFD_GetBufferOrCheckWhetherToBail(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto done;
		else if (0 == p->m_nLineLen)
			break;

		// next check for response-headers
		lRet = WFD_CheckForResponseHeader(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto done;

		lRet = WFD_GetBufferOrCheckWhetherToBail(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto done;
		else if (0 == p->m_nLineLen)
			break;

		// next check for entity-headers
		lRet = WFD_CheckForEntityHeader(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto done;

		lRet = WFD_GetBufferOrCheckWhetherToBail(p);
		if (WFD_RTSP_NO_ERROR != lRet)
			goto done;
		else if (0 == p->m_nLineLen)
			break;

	} while (TRUE);

	// if we need to take action on a response, we can do it here
	switch (pSent->nMethod) {
	case MDESCRIBE:              // Section 10.2
	case MANNOUNCE:              // Section 10.3
	case MGET_PARAMETER:         // Section 10.8
		break;
	case MOPTIONS:               // Section 10.1
		break;
	case MPAUSE:                 // Section 10.6
		memcpy(p->m_playStruct.m_szSession, p->m_szSession, sizeof(p->m_playStruct.m_szSession));
		lRet = p->m_pNotify(WFD_RTSP_PAUSE, &p->m_playStruct, p->m_pContext);
		if (WFD_RTSP_NO_ERROR != lRet) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
		}
		break;
	case MPLAY:                  // Section 10.5
		p->m_bRtspConnected = TRUE;
		memcpy(p->m_playStruct.m_szSession, p->m_szSession, sizeof(p->m_playStruct.m_szSession));
		lRet = p->m_pNotify(WFD_RTSP_PLAY, &p->m_playStruct, p->m_pContext);
		if (WFD_RTSP_NO_ERROR != lRet) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
		}
		break;
	case MRECORD:                // Section 10.11
	case MREDIRECT:              // Section 10.10
		break;
	case MSETUP:                 // Section 10.4
		p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);
		lRet = WFD_SendPlay(p);
		if (WFD_RTSP_NO_ERROR != lRet) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received error (%d) from sending play", __FUNCTION__, lRet);
		}
		break;
	case MSET_PARAMETER:         // Section 10.9
		if (p->m_nCSeqAfterwhichToGoToM5 == p->nCSeq && p->m_bServer) {
			// this is the response for the M4 SET_PARAMETER message after which we send the setup trigger
			lRet = WFD_SendSetupTrigger(p);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received error (%d) from sending setup trigger", __FUNCTION__, lRet);
			}
		}
		break;
	case MTEARDOWN:              // Section 10.7
		// got teardown response
		p->m_bRtspConnected = FALSE;

		lRet = p->m_pNotify(WFD_RTSP_TEARDOWN, 0, p->m_pContext);
		if (WFD_RTSP_NO_ERROR != lRet) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
		}

		break;
	}

done:
	p->m_nLineLen = 0;

	WFD_QueuePutBuffer(&p->m_bqToSend, p, pSent);

	return lRet;
}

long WFD_GetContents(CRTSPParser* p, unsigned long nContentLen)
{
	long nBytes=0;
	long lRet = WFD_RTSP_NO_ERROR;

	p->pParsed->nUsed = 0;

	if (p->pParsed->nAllocated < (unsigned long)nContentLen+1) {
		// reallocate
		// this shouldn't happen
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - unexpected content length (%d) exceeds initial buffer size", __FUNCTION__, nContentLen);

		p->pParsed = realloc(p->pParsed, sizeof(WFD_NODE) + nContentLen+1);
		if (!p->pParsed) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed allocating large buffer", __FUNCTION__);
			return WFD_RTSP_MEMORY;
		}
		p->pParsed->nAllocated = nContentLen+1;
		p->pParsed->nUsed = 0;
	}

	// first see if there's some bytes in the read buffer we need to get
	if (p->m_bUnget) {
		p->pParsed->aBuffer[0] = p->m_cUngetChar;
		p->m_bUnget = FALSE;
		p->pParsed->nUsed = 1;
	}

	if (p->m_nPos) {
		while (p->m_nBytesRead > p->m_nPos && ('\r' == p->m_pReadBuffer[p->m_nPos] || '\n' == p->m_pInputBuffer[p->m_nPos]))
			p->m_nPos++;

		nBytes = MIN((long)nContentLen, p->m_nBytesRead-p->m_nPos);

		memcpy(p->pParsed->aBuffer+p->pParsed->nUsed, p->m_pReadBuffer+p->m_nPos, nBytes);
		p->pParsed->nUsed += nBytes;
		p->m_nPos+= nBytes;
	}

	if (p->pParsed->nUsed < nContentLen) {
		
		nBytes=0;

		// read until we read all the message body
		do {
			lRet = p->m_pRecv(p->pParsed->aBuffer+p->pParsed->nUsed, nContentLen-p->pParsed->nUsed, &nBytes, 100, p->m_pContext);
			if (WFD_RTSP_NO_ERROR != lRet)
				return lRet;

			p->pParsed->nUsed += nBytes;
		} while (p->pParsed->nUsed < nContentLen);

		p->m_nBytesRead = p->m_nPos = 0;
	}

	p->pParsed->aBuffer[p->pParsed->nUsed] = 0;

	return WFD_RTSP_NO_ERROR;
}

