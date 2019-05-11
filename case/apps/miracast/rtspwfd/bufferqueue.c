/**
 * @file: bufferqueue.c
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
#include "bufferqueue.h"

BOOL WFD_QueueAllocate(WFD_CBufferQueue* p, void* hRtsp, unsigned long nSize, unsigned long nCount)
{
	unsigned long i;

	for (i=0; i < nCount; i++) {
		WFD_NODE* pNode = (WFD_NODE*)calloc(nSize + sizeof(WFD_NODE), 1);
        if ( pNode == NULL ) {
            WFD_RTSP_LOG(hRtsp, RTSP_LOG_ERR, "%s - unable to calloc()", __FUNCTION__);
            return FALSE;
        }
		pNode->nAllocated = nSize;

		if (pNode)
			WFD_QueuePutBuffer(p, hRtsp, pNode);
		else
			return FALSE;
	}

	return TRUE;
}

void WFD_QueueFree(WFD_CBufferQueue* p, void* hRtsp)
{
	while (p->m_pHead) {
		WFD_NODE* pNode = p->m_pHead;

		p->m_pHead = p->m_pHead->pNext;
		p->m_nBuffersQueued--;

		if (pNode)
			free(pNode);
	}
}

WFD_NODE* WFD_QueueGetBuffer(WFD_CBufferQueue* p, void* hRtsp)
{
	CRTSPParser* p2=(CRTSPParser*)hRtsp;

	while (0 == p->m_nBuffersQueued && !p->m_bShutdown)
		p->m_pSleep(10, p2);

	if (p->m_bShutdown)
		return NULL;

	if (p->m_pHead) {
		WFD_NODE* pNode = p->m_pHead;

		p->m_pHead = p->m_pHead->pNext;
		p->m_nBuffersQueued--;

		if (p->m_pHead) {
			p->m_pHead->pPrev = NULL;
		} else {
			// if head is NULL queue is empty, so set tail to NULL
			p->m_pTail = NULL;
		}

		pNode->bKeepalive = FALSE;

		return pNode;
	} else {
		WFD_RTSP_LOG(hRtsp, RTSP_LOG_ERR, "%s - Did not expect to get empty buffer", __FUNCTION__);
		return NULL;
	}
}

void WFD_QueuePutBuffer(WFD_CBufferQueue* p, void* hRtsp, WFD_NODE* pNode)
{
	pNode->pNext = NULL;

	if (p->m_pTail) {
		pNode->pPrev = p->m_pTail;
		p->m_pTail->pNext = pNode;
	} else {
		pNode->pPrev = NULL;
		p->m_pHead = pNode;
	}

	p->m_nBuffersQueued++;
	p->m_pTail = pNode;
}

unsigned long WFD_QueuedBufferCount(WFD_CBufferQueue* p, void* hRtsp)
{
	return p->m_nBuffersQueued;
}

void WFD_QueueStopWaiting(WFD_CBufferQueue* p, void* hRtsp) 
{
	p->m_bShutdown = TRUE;
}

BOOL WFD_QueueShutdown(WFD_CBufferQueue* p, void* hRtsp) 
{
	return p->m_bShutdown;
}

