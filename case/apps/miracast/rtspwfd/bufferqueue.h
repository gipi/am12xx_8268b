/**
 * @file: bufferqueue.h
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

#ifndef __CBUFFERQUEUE_H__
#define __CBUFFERQUEUE_H__

#define FREE_DYNAMIC_OR_PUT(buf, queue) \
{                                       \
	if (buf) {                          \
		if (buf->bDynamicAlloc)         \
			free(buf);                  \
		else                            \
			WFD_QueuePutBuffer(&queue, buf);       \
                                        \
		buf = NULL;                     \
	}                                   \
}

#define GET_AND_CHECK(variable,statement) \
{                                         \
	variable=statement;                   \
	if (!variable) {                      \
		return WFD_RTSP_INTERNAL_ERROR;       \
	}                                     \
}

typedef struct _tagWFDNode {
	struct _tagWFDNode* pPrev;
	struct _tagWFDNode* pNext;
	unsigned long	 nAllocated;
	unsigned long	 nUsed;
	unsigned long    nCSeq;
	unsigned long    nClientAddr;
	unsigned long    nTimeSent;
	float            fRTSPVer;
	float            fRDSVer;
	BOOL             bKeepalive;
	int              nMethod; // positive for method, negative for response
	unsigned char    aBuffer[4]; // buffer space is allocated off the end of the structure
} WFD_NODE;

typedef struct 
{
	WFD_NODE* m_pHead;
	WFD_NODE* m_pTail;
	unsigned long m_nBuffersQueued;
	BOOL m_bShutdown;
	FN_WFD_RTSP_SLEEP m_pSleep;
} WFD_CBufferQueue;

BOOL WFD_QueueAllocate(WFD_CBufferQueue* p, void* hRtsp, unsigned long nSize, unsigned long nCount);
WFD_NODE* WFD_QueueReallocate(WFD_CBufferQueue* p, void* hRtsp, WFD_NODE* pBuf, int nSize);
void WFD_QueueFree(WFD_CBufferQueue* p, void* hRtsp);
WFD_NODE* WFD_QueueGetBuffer(WFD_CBufferQueue* p, void* hRtsp);
void WFD_QueuePutBuffer(WFD_CBufferQueue* p, void* hRtsp, WFD_NODE* pNode);
unsigned long WFD_QueuedBufferCount(WFD_CBufferQueue* p, void* hRtsp);
void WFD_QueueStopWaiting(WFD_CBufferQueue* p, void* hRtsp);
BOOL WFD_QueueShutdown(WFD_CBufferQueue* p, void* hRtsp);

#endif //__CBUFFERQUEUE_H__
