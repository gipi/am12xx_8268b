/**
 * @file: wfdparser.h
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
#define RTSP_OPTION_TAG "org.wfa.wfd1.0"
#define RTSP_SHORT_TAG "wfd1.0"

#define RTSP_MAX_INPUT_BUFFER	(100*1024) // this should be large enough to handle MIME encoded 160x120 png
#define RTSP_MAX_OUTPUT_BUFFER	(100*1024) // this should be large enough to handle MIME encoded 160x120 png
#define RTSP_MAX_IPADDR_BUFFER	16 // this should handle IP4 format address: xxx.xxx.xxx.xxx
#define RTSP_RECV_BUFFER 1500
#define RTSP_SEND_COUNT 20
#define RTSP_SEND_BUFFER 1500
#define RESPONSE_TIMEOUT 5000
#define RTSP_MAX_ERROR_MSG_LEN	400
#define WFD_SESSION_TIMEOUT_DEFAULT	60000 // 60000 msec or 60 sec

#define MIN(a, b) ((a < b)?(a):(b))
#define SET_CSEQ \
{\
	pBuf->nCSeq = p->m_nSendCSeq; \
}

typedef enum {
	WFD_RTSP_M00,
	WFD_RTSP_M01,
	WFD_RTSP_M02,
	WFD_RTSP_M03,
	WFD_RTSP_M04,
	WFD_RTSP_M05,
	WFD_RTSP_M06,
	WFD_RTSP_M07,
	WFD_RTSP_M08,
	WFD_RTSP_M09,
	WFD_RTSP_M10,
	WFD_RTSP_M11,
	WFD_RTSP_M12,
	WFD_RTSP_M13,
	WFD_RTSP_M14,
	WFD_RTSP_M15,
	WFD_RTSP_M16,
	WFD_RTSP_M17,
	WFD_RTSP_M_COUNT
} WFD_RTSP_STATES;

typedef struct
{
	BOOL m_bServer;
	//BOOL m_bLegacy;

	WFD_RTSP_STATES	m_eState;

	// RTSP parsing variables
	WFD_MethodTokens m_Trigger;
	WFD_CBufferQueue m_bqToSend;
	WFD_CBufferQueue m_bqSent;
	unsigned short m_usRTPPort;
	unsigned short m_usServerRtcpPort;
	unsigned char* m_pReadBuffer;
	unsigned char* m_pInputBuffer;
	unsigned char* m_pOutputBuffer;
	unsigned char* m_pSendBuffer;
	unsigned char* m_pLine;
	WFD_PLAY_STRUCT m_playStruct;
	double m_dServerVersion;
	BOOL m_bRtspConnected;
	BOOL m_bUnget;
	BOOL m_bInParser;
	BOOL m_bHDCP;
	BOOL m_bPreferredDisplayModeSupported;
	BOOL m_bPreferredDisplayModeSent;
	BOOL m_bStandbyMode;
	BOOL m_bMySupportedMethods[CountOfWFD_MethodTokens];
	BOOL m_bPeerSupportedMethods[CountOfWFD_MethodTokens];
	BOOL m_bSupportedWFDCommands[CountOfWFDParams];
	long m_nOutputBufferUsed;
	long m_nBytesRead;
	long m_nPos;
	long m_nLineLen;
	long m_nLastKeepalive;
	char m_szSession[WFD_RTSP_SESSION_ID_LEN+1];
	char m_szServerIpAddress[RTSP_MAX_IPADDR_BUFFER];
	char m_szIpAddress[RTSP_MAX_IPADDR_BUFFER];
	char m_szErrorMessage[RTSP_MAX_ERROR_MSG_LEN];
	char m_cUngetChar;
	int m_nSendCSeq;
	int m_nRecvCSeq;
	int m_nCSeqAfterwhichToGoToM4;
	int m_nCSeqAfterwhichToGoToM5;
	int m_nContentLength;
	int m_nKeepalivePeriod;
	int m_nResponseTimeout;
	int m_nRTSPReponseCode;
	FN_WFD_RTSP_NOTIFICATIONS m_pNotify;
	FN_WFD_RTSP_RECV m_pRecv;
	FN_WFD_RTSP_SEND m_pSend;
	FN_WFD_RTSP_SLEEP m_pSleep;
	FN_WFD_RTSP_GET_MSEC_TICK m_pGetTick;
	FN_WFD_RTSP_LOG m_pLog;
	WFD_EOLTypes m_eolType;
	char* m_pStrtokContext;
	char* m_pInitialGetParams;
	void* m_pContext;
	unsigned long m_ulStartTime[WFD_RTSP_M_COUNT];

	// RDS vairiables
	unsigned char m_ucDeviecId[WFD_DEVICE_ID_LEN];
	unsigned char* m_pucDeviecLogo;
	char m_szPresentationUrl0[WFD_PRESENTATION_URL_LEN+1];
	char m_szPresentationUrl1[WFD_PRESENTATION_URL_LEN+1];

	WFD_NODE* pBuf;
	WFD_NODE* pParsed;
	char* pToken;
	int nMethod;
	int nStrikes;
	int nToken;
	int nCSeq;
} CRTSPParser;


CRTSPParser* WFD_RTSP_Alloc(BOOL bServer);
void WFD_RTSP_Free(CRTSPParser* p);
long WFD_GetBufferOrCheckWhetherToBail(CRTSPParser* p);

long WFDReadNextLine(CRTSPParser* p);
long WFD_RTSP_SetSrcAddress(CRTSPParser* p, char* szIpAddr);
long WFD_RTSP_Init(CRTSPParser* p, unsigned short usRtpPort, unsigned short usServerRtcpPort, unsigned long ulKeepaliveTimeout, char* pInitialGetParams, FN_WFD_RTSP_NOTIFICATIONS pfnCallback, FN_WFD_RTSP_RECV pfnRecv, FN_WFD_RTSP_SEND pfnSend, FN_WFD_RTSP_SLEEP pfnSleep, FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, FN_WFD_RTSP_LOG pfnLog, void* pContext);
long WFD_RTSP_UpdateParams(CRTSPParser* p, char* pOptRdsSetupParams);
long WFD_RTSP_StartSession(CRTSPParser* p, char* szTargAddr, unsigned short nTargPort);
long WFD_RTSP_SetParam(CRTSPParser* p, char* szParams);
long WFD_RTSP_GetParam(CRTSPParser* p, char* szParams);
long WFD_RTSP_SetParamBinary(CRTSPParser* p, WFDParams id, void* pData, long nDataLen);
long WFD_RTSP_SetParamStruct(CRTSPParser* p, WFD_PARAMETER_STRUCT* pStruct, int nCount);
long WFD_RTSP_Shutdown(CRTSPParser* p);
long WFD_RTSP_Play(CRTSPParser* p);
long WFD_RTSP_Pause(CRTSPParser* p);
long WFD_RTSP_Teardown(CRTSPParser* p);
long WFD_RTSP_Parser(CRTSPParser* p);
void WFD_RTSP_LOG(CRTSPParser* p, int nLevel, char* szFmt,...);

long WFD_ParseMessageResponse(CRTSPParser* p);
long WFD_Send(CRTSPParser* p, char* szMessage);
long WFD_SendRequest(CRTSPParser* p, char* szHeaders, char* szBody);
long WFD_SendResponse(CRTSPParser* p, int nCSeq, int nResponse, char* szRespMsg, char* szHeaders, char* szBody);
long WFD_GetContents(CRTSPParser* p, unsigned long nContentLen);
long WFD_MatchToken(char* apTokens[], int nCount, char* szToken);
long WFD_CheckForEntityHeader(CRTSPParser* p);
long WFD_CheckForGeneralHeader(CRTSPParser* p);
long WFD_CheckForRequestHeader(CRTSPParser* p);
long WFD_CheckForResponseHeader(CRTSPParser* p);
long WFD_GetBufferOrCheckWhetherToBail(CRTSPParser* p);

long WFD_SendOptions(CRTSPParser* p);
long WFD_SendSetupTrigger(CRTSPParser* p);
long WFD_SendPauseTrigger(CRTSPParser* p);
long WFD_SendPlayTrigger(CRTSPParser* p);
long WFD_SendSetup(CRTSPParser* p);
long WFD_SendPlay(CRTSPParser* p);
long WFD_SendPause(CRTSPParser* p);
long WFD_SendTeardownTrigger(CRTSPParser* p);
long WFD_SendTeardown(CRTSPParser* p);
long WFD_SendGetParameters(CRTSPParser* p, char* szParams);
long WFD_SendSetParameters(CRTSPParser* p, char* szParams);
long WFD_SendSetParameter(CRTSPParser* p, WFDParams rdsParam, char* szParams);

long WFD_SendOptionsResponse(CRTSPParser* p, int nCSeq);
long WFD_SendSetupTriggerResponse(CRTSPParser* p, int nCSeq);
long WFD_SendPauseTriggerResponse(CRTSPParser* p, int nCSeq);
long WFD_SendPlayTriggerResponse(CRTSPParser* p, int nCSeq);
long WFD_SendSetupResponse(CRTSPParser* p, int nCSeq);
long WFD_SendPlayResponse(CRTSPParser* p, int nCSeq);
long WFD_SendTeardownTriggerResponse(CRTSPParser* p, int nCSeq);
long WFD_SendTeardownResponse(CRTSPParser* p, int nCSeq);
long WFD_SendGetParametersResponse(CRTSPParser* p, int nCSeq, char* szParams);
long WFD_SendSetParametersResponse(CRTSPParser* p, int nCSeq);
long WFD_SetResponseCodeAndMessage(CRTSPParser* p, unsigned long ulResponseCode, char* szMsg);

int wfd_snprintf_(char *buffer, size_t sizeOfBuffer, const char *format, ...);

#endif // __RTSPPARSER_H__
