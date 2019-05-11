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
			QueuePutBuffer(&queue, buf);       \
                                        \
		buf = NULL;                     \
	}                                   \
}

#define GET_AND_CHECK(variable,statement) \
{                                         \
	variable=statement;                   \
	if (!variable) {                      \
		return RTSP_INTERNAL_ERROR;       \
	}                                     \
}

typedef struct _tagNode {
	struct _tagNode* pPrev;
	struct _tagNode* pNext;
	unsigned long	 nAllocated;
	unsigned long	 nUsed;
	unsigned long    nCSeq;
	unsigned long    nClientAddr;
	unsigned long    nTimeSent;
	float            fRTSPVer;
	float            fRDSVer;
	BOOL             bClientConnected;
	BOOL             bKeepalive;
	int              nMethod; // positive for method, negative for response
	unsigned char    aBuffer[4]; // buffer space is allocated off the end of the structure
} NODE;

typedef struct 
{
	NODE* m_pHead;
	NODE* m_pTail;
	unsigned long m_nBuffersQueued;
	BOOL m_bShutdown;
	FN_RTSP_SLEEP m_pSleep;
} CBufferQueue;

BOOL QueueAllocate(CBufferQueue* p, void* hRtsp, unsigned long nSize, unsigned long nCount);
NODE* QueueReallocate(CBufferQueue* p, void* hRtsp, NODE* pBuf, int nSize);
void QueueFree(CBufferQueue* p, void* hRtsp);
NODE* QueueGetBuffer(CBufferQueue* p, void* hRtsp);
void QueuePutBuffer(CBufferQueue* p, void* hRtsp, NODE* pNode);
unsigned long QueueQueuedBufferCount(CBufferQueue* p, void* hRtsp);
void QueueStopWaiting(CBufferQueue* p, void* hRtsp);
BOOL QueueShutdown(CBufferQueue* p, void* hRtsp);

#endif //__CBUFFERQUEUE_H__
