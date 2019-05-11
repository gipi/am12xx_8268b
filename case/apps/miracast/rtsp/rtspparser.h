/**
 * @file: rtspparser.h
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

#ifndef __RTSPPARSER_H__
#define __RTSPPARSER_H__

#include "bufferqueue.h"

#include "rtspparser_d.h"

#define Declare_Extern
#include "rtspparser_d.h"

#include "method_d.h"

#define Declare_Extern
#include "method_d.h"

#include "general-header_d.h"

#define Declare_Extern
#include "general-header_d.h"

#include "request-header_d.h"

#define Declare_Extern
#include "request-header_d.h"

#include "response-header_d.h"

#define Declare_Extern
#include "response-header_d.h"

#include "entity-header_d.h"

#define Declare_Extern
#include "entity-header_d.h"

#define RTSP_VERSION "RTSP/1.0"
#define RDS_VERSION "com.intel.rds1.0"
#define RDS_SHORT_VERSION "rds1.0"

#define RTSP_MAX_INPUT_BUFFER	(100*1024) // this should be large enough to handle MIME encoded 160x120 png
#define RTSP_MAX_OUTPUT_BUFFER	(100*1024) // this should be large enough to handle MIME encoded 160x120 png
#define RTSP_MAX_IPADDR_BUFFER	16 // this should handle IP4 format address: xxx.xxx.xxx.xxx
#define RTSP_RECV_BUFFER 1500
#define RTSP_SEND_COUNT 20
#define RTSP_SEND_BUFFER 1500
#define RESPONSE_TIMEOUT 5000
#define RTSP_MAX_ERROR_MSG_LEN	200
#define RDS_SESSION_TIMEOUT_DEFAULT	30000 // 30000 msec or 30 sec

#define MIN(a, b) ((a < b)?(a):(b))
#define SET_CSEQ \
{\
	if (p->m_bLegacy) { \
		if (p->m_nSendCSeq <= p->m_nRecvCSeq) \
			p->m_nSendCSeq = p->m_nRecvCSeq+1; \
	} \
	pBuf->nCSeq = p->m_nSendCSeq; \
}

typedef struct
{
	BOOL m_bServer;
	BOOL m_bLegacy;

	// RTSP parsing variables
	MethodTokens m_Trigger;
	CBufferQueue m_bqToSend;
	CBufferQueue m_bqSent;
	unsigned short m_usRTPPort;
	unsigned short m_usServerRtcpPort;
	unsigned char* m_pReadBuffer;
	unsigned char* m_pInputBuffer;
	unsigned char* m_pOutputBuffer;
	unsigned char* m_pSendBuffer;
	unsigned char* m_pLine;
	PLAY_STRUCT m_playStruct;
	double m_dServerVersion;
	BOOL m_bRtspConnected;
	BOOL m_bUnget;
	BOOL m_bInParser;
	BOOL m_bMySupportedMethods[CountOfMethodTokens];
	BOOL m_bPeerSupportedMethods[CountOfMethodTokens];
	long m_nOutputBufferUsed;
	long m_nBytesRead;
	long m_nPos;
	long m_nLineLen;
	long m_nLastKeepalive;
	char m_szSession[RTSP_SESSION_ID_LEN+1];
	char m_szServerIpAddress[RTSP_MAX_IPADDR_BUFFER];
	char m_szIpAddress[RTSP_MAX_IPADDR_BUFFER];
	char m_szErrorMessage[RTSP_MAX_ERROR_MSG_LEN];
	char m_cUngetChar;
	int m_nSendCSeq;
	int m_nRecvCSeq;
	int m_nCSeqForGetEdid;
	int m_nOverscanComp;
	int m_nContentLength;
	int m_nKeepalivePeriod;
	int m_nResponseTimeout;
	int m_nRTSPReponseCode;
	FN_RTSP_NOTIFICATIONS m_pNotify;
	FN_RTSP_RECV m_pRecv;
	FN_RTSP_SEND m_pSend;
	FN_RTSP_SLEEP m_pSleep;
	FN_RTSP_GET_MSEC_TICK m_pGetTick;
	FN_RTSP_LOG m_pLog;
	EOLTypes m_eolType;
	char* m_pStrtokContext;
	char* m_pOptRdsSetupParams;
	void* m_pContext;
	unsigned long m_ulStartTime;

	// RDS vairiables
	VERSION_STRUCT m_verSoftware;
	VERSION_STRUCT m_verHardware;
	unsigned char m_ucDeviecId[RDS_DEVICE_ID_LEN];
	unsigned char* m_pucDeviecLogo;
	char m_szFriendlyName[RDS_FRIENDLY_NAME_LEN+1];
	char m_szModelName[RDS_MODEL_NAME_LEN+1];
	char m_szManufacturerName[RDS_MANUFACTURER_NAME_LEN+1];
	char m_szManufacturerUrl[RDS_MANUFACTURER_URL_LEN+1];
	char m_szProductId[RDS_PRODUCT_ID_LEN+1];

	NODE* pBuf;
	NODE* pParsed;
	char* pToken;
	int nMethod;
	int nStrikes;
	int nToken;
	int nCSeq;
} CRTSPParser;


CRTSPParser* RTSP_Alloc(BOOL bServer);
void RTSP_Free(CRTSPParser* p);
long GetBufferOrCheckWhetherToBail(CRTSPParser* p);

long ReadNextLine(CRTSPParser* p);
long RTSP_SetSrcAddress(CRTSPParser* p, char* szIpAddr);
long RTSP_Init(CRTSPParser* p, unsigned short usRtpPort, unsigned short usServerRtcpPort, int nOverscanComp, char* pOptRdsSetupParams, FN_RTSP_NOTIFICATIONS pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext);
long RTSP_UpdateParams(CRTSPParser* p, int nOverscanComp, char* pOptRdsSetupParams);
long RTSP_StartSession(CRTSPParser* p, char* szTargAddr, unsigned short nTargPort);
long RTSP_SetParam(CRTSPParser* p, char* szParams);
long RTSP_GetParam(CRTSPParser* p, char* szParams);
long RTSP_SetParamBinary(CRTSPParser* p, RDSParams id, void* pData, long nDataLen);
long RTSP_SetParamStruct(CRTSPParser* p, PARAMETER_STRUCT* pStruct);
long RTSP_Shutdown(CRTSPParser* p);
long RTSP_Teardown(CRTSPParser* p);
long RTSP_Parser(CRTSPParser* p);
void RTSP_LOG(CRTSPParser* p, int nLevel, char* szFmt,...);

long ParseMessageResponse(CRTSPParser* p);
long _Send(CRTSPParser* p, char* szMessage);
long SendRequest(CRTSPParser* p, char* szHeaders, char* szBody);
long SendResponse(CRTSPParser* p, int nCSeq, int nResponse, char* szRespMsg, char* szHeaders, char* szBody);
long GetContents(CRTSPParser* p, unsigned long nContentLen);
long MatchToken(char* apTokens[], int nCount, char* szToken);
long CheckForEntityHeader(CRTSPParser* p);
long CheckForGeneralHeader(CRTSPParser* p);
long CheckForRequestHeader(CRTSPParser* p);
long CheckForResponseHeader(CRTSPParser* p);
long GetBufferOrCheckWhetherToBail(CRTSPParser* p);

long SendOptions(CRTSPParser* p);
long SendSetupTrigger(CRTSPParser* p);
long SendSetup(CRTSPParser* p);
long SendPlay(CRTSPParser* p);
long SendTeardownTrigger(CRTSPParser* p);
long SendTeardown(CRTSPParser* p);
long SendGetParameters(CRTSPParser* p, char* szParams);
long SendSetParameters(CRTSPParser* p, char* szParams);
long SendSetParameter(CRTSPParser* p, RDSParams rdsParam, char* szParams);

long SendOptionsResponse(CRTSPParser* p, int nCSeq);
long SendSetupTriggerResponse(CRTSPParser* p, int nCSeq);
long SendSetupResponse(CRTSPParser* p, int nCSeq);
long SendPlayResponse(CRTSPParser* p, int nCSeq);
long SendTeardownTriggerResponse(CRTSPParser* p, int nCSeq);
long SendTeardownResponse(CRTSPParser* p, int nCSeq);
long SendGetParametersResponse(CRTSPParser* p, int nCSeq, char* szParams);
long SendSetParametersResponse(CRTSPParser* p, int nCSeq);

int _snprintf_(char *buffer, size_t sizeOfBuffer, const char *format, ...);
long GetNext(CRTSPParser* p, char* pchar);

#endif // __RTSPPARSER_H__
