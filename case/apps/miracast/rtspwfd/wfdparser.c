/**
* @file: rdsparser.c
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
#include <errno.h>

#define Declare_String
#include "rtspwfd_d.h"

#define WIFI_DIRECT_JSON 1
extern char g_cAtoH[255];

static int GetIndex(char* szName, int nCount, char* strArr[])
{
	int i;
	int nLen=0;
	
	// if szName is null or it is a null pointer, immediately return value for not found
	if (!szName || !szName[0])
		return nCount;

	nLen = (int)strlen(szName);

	if (!strncmp(szName, "wfd_", 4) && ':' == szName[nLen-1]) {
		// we want to ignore the colon if there is one
		szName[nLen-1] = 0;
	}

	for (i=0; i < nCount; i++)
		if (!strcmp(szName, strArr[i]))
			return i;

	return nCount;
}

long WFD_Parser(CRTSPParser* p)
{
	long lRet = WFD_RTSP_NO_ERROR;
#ifdef WIFI_DIRECT_JSON
	FILE *fd = NULL;	
#endif
	if (MSET_PARAMETER == -p->pParsed->nMethod) {

		WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - SET_PARAMETER (response): %s", __FUNCTION__, p->pParsed->aBuffer);

	} else if (MGET_PARAMETER == -p->pParsed->nMethod) {

		if (200 > p->pParsed->nUsed) {
			WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - GET_PARAMETER (response): %s", __FUNCTION__, p->pParsed->aBuffer);
		} else {
			WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - GET_PARAMETER (response): %d byte buffer", __FUNCTION__, p->pParsed->nUsed);
		}

		lRet = WFD_ParseParametersAndNotify(p, WFD_RTSP_GET_PARAMETER_RESP);
		if (WFD_RTSP_NO_ERROR != lRet) {
			return WFD_RTSP_INTERNAL_ERROR;
		}

		if (p->m_nCSeqAfterwhichToGoToM4 == p->nCSeq && p->m_bServer) {

			// if preferred display mode is supported, and we haven't requested it, request it
			if (p->m_bPreferredDisplayModeSupported && !p->m_bPreferredDisplayModeSent) {
				
				// this prompts the app to send the preferred siaply mode M4 message
				WFD_PARAMETER_STRUCT buf;
				memset(&buf, 0, sizeof(buf));

				buf.nId = wfd_preferred_display_mode;

				p->m_pNotify(WFD_RTSP_PREF_DISP_EXCHANGE, &buf, p->m_pContext);

				if (buf.u.prefDispMode.supported) {
					lRet = WFD_RTSP_SetParamStruct(p, &buf, 1);
					if (WFD_RTSP_NO_ERROR != lRet) {
						return WFD_RTSP_INTERNAL_ERROR;
					}

					p->m_bPreferredDisplayModeSent = TRUE;

					// change so we will send M4 after getting the wfd_preferred_display_mode reponse
					p->m_nCSeqAfterwhichToGoToM4 = p->m_nSendCSeq-1;
				} else {
				
					// this prompts the app to send the M4 message
					p->m_eState = WFD_RTSP_M04;
					p->m_pNotify(WFD_RTSP_CAP_EXCHANGE, 0, p->m_pContext);

				}
			} else {
				
				// this prompts the app to send the M4 message
				p->m_eState = WFD_RTSP_M04;
				p->m_pNotify(WFD_RTSP_CAP_EXCHANGE, 0, p->m_pContext);

			}
		}
	} else if (MSET_PARAMETER == p->pParsed->nMethod) {

		// WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - SET_PARAMETER: %s", __FUNCTION__, p->pParsed->aBuffer);
		//printf("set parameter>>>>>>>>>>>>>:\n%s\n",(char*)p->pParsed->aBuffer);
		lRet = WFD_ParseParametersAndNotify(p, WFD_RTSP_SET_PARAMETER);
		if (WFD_RTSP_NO_ERROR != lRet) {
			return lRet;
		}

		if (MSETUP == p->m_Trigger) {

			lRet = WFD_SendSetupTriggerResponse(p, p->pParsed->nCSeq);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendSetupTriggerResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

			lRet = WFD_SendSetup(p);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendSetup(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

		} else if (MPLAY == p->m_Trigger) {

			lRet = WFD_SendPlayTriggerResponse(p, p->pParsed->nCSeq);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendPlayTriggerResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

			lRet = WFD_SendPlay(p);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendPlay(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

		} else if (MPAUSE == p->m_Trigger) {

			lRet = WFD_SendPauseTriggerResponse(p, p->pParsed->nCSeq);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendPauseTriggerResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

			lRet = WFD_SendPause(p);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendPause(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

		} else if (MTEARDOWN == p->m_Trigger) {
			lRet = WFD_SendTeardownTriggerResponse(p, p->pParsed->nCSeq);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendSetupTriggerResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}

			lRet = WFD_SendTeardown(p);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendSetup(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}
		} else {
#ifdef WIFI_DIRECT_JSON
			fd = fopen("/tmp/rtsp_parameter.txt","w");
			if(fd !=NULL){
				fprintf(fd,"%s",(char*)p->m_pReadBuffer);
				fflush(fd);
				fsync(fileno(fd));
				fclose(fd);		
				//printf("---%s--%d---write rtsp_parameter.txt ok!--------\n",__FUNCTION__,__LINE__);
			}
			else{
				//perror("open /tmp/rtsp_parameter.txt error!\n");
			}
#endif			
			lRet = WFD_SendSetParametersResponse(p, p->pParsed->nCSeq);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return WFD_RTSP_INTERNAL_ERROR;
			}
		}

	} else if (MGET_PARAMETER == p->pParsed->nMethod) {
		// See what parameters we need and then respond with them
		char* pToken=strtok_s((char*)p->pParsed->aBuffer, ", \r\n", &(p->m_pStrtokContext));
		long nLen=0;
		long nAdditionalLen=0;

		p->m_nOutputBufferUsed = 0;
		//WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - GET_PARAMETER: %s", __FUNCTION__, p->pParsed->aBuffer);

		while (pToken) {
			WFD_PARAMETER_STRUCT buf;

			memset(&buf, 0, sizeof(buf));

			buf.nId = (WFDParams)GetIndex(pToken, CountOfWFDParams, g_szWFDParams);
			buf.szValue = buf.szStaticBuffer;
			buf.nCount = WFD_STATIC_LEN;
			if (CountOfWFDParams == buf.nId) {
				wfd_snprintf_(buf.szStaticBuffer, WFD_STATIC_LEN, "%s", pToken);
				buf.szParam = buf.szStaticBuffer;
			} else if (!p->m_bSupportedWFDCommands[buf.nId]) {
				p->m_nRTSPReponseCode = 551; // option not supported
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "%s not supported", g_szWFDParams[buf.nId]);
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				return WFD_RTSP_INTERNAL_ERROR;
			}

			// first get the required size
			// we always start by providing our static buffer which has length WFD_STATIC_LEN
			// it will be long enough in most cases. If not, the app will communicate the
			// required length in the nCount member. We should then allocate the required 
			// buffer size. In either case the buffer for collecting the requested parameter
			// is szValue.
			switch (buf.nId) {
			case wfd_audio_codecs:
				buf.u.audioCodecs.aCodecs = (void*)buf.szValue;
				break;
			case wfd_video_formats:
			case wfd_3d_video_formats:
				buf.u.videoFormat.aProfiles = (void*)buf.szValue;
				break;
			case wfd_display_edid:
				buf.u.displayEdid.pucEdid = (void*)buf.szValue;
				break;
			case wfd_client_rtp_ports:
				buf.u.clientPorts.szProfile = (void*)buf.szValue;
				break;
			case wfd_uibc_capability:
				buf.u.uibcCapability.pCapPairs = (void*)buf.szValue;
				break;
			default:
				// client will directly use szValue
				break;
			}

			lRet = p->m_pNotify(WFD_RTSP_GET_PARAMETER, &buf, p->m_pContext);

			if (WFD_RTSP_INSUFFICIENT_BUF == lRet) {

				// to allow for null string termination
				buf.nCount++;
				buf.szValue = calloc(1, buf.nCount);
				
				switch (buf.nId) {
				case wfd_audio_codecs:
					buf.u.audioCodecs.aCodecs = (void*)buf.szValue;
					break;
				case wfd_video_formats:
				case wfd_3d_video_formats:
					buf.u.videoFormat.aProfiles = (void*)buf.szValue;
					break;
				case wfd_display_edid:
					buf.u.displayEdid.pucEdid = (void*)buf.szValue;
					break;
				case wfd_client_rtp_ports:
					buf.u.clientPorts.szProfile = (void*)buf.szValue;
					break;
				case wfd_uibc_capability:
					buf.u.uibcCapability.pCapPairs = (void*)buf.szValue;
					break;
				default:
					// client will directly use szValue
					break;
				}

				lRet = p->m_pNotify(WFD_RTSP_GET_PARAMETER, &buf, p->m_pContext);
				if (WFD_RTSP_NO_ERROR != lRet) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
					if (buf.szValue != buf.szStaticBuffer)
						free(buf.szValue);
					return WFD_RTSP_INTERNAL_ERROR;
				}

				nAdditionalLen = (long)(strlen(pToken))+2 + (long)(strlen(buf.szValue))+2;

				if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %c%c%c%c%c...)", __FUNCTION__, pToken, 
						buf.szValue[0], buf.szValue[1], buf.szValue[2], buf.szValue[3], buf.szValue[4]);
					return WFD_RTSP_INSUFFICIENT_BUF;
				}

				nLen = wfd_snprintf_((char*)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: %s\r\n", pToken, buf.szValue);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

				if (buf.szValue != buf.szStaticBuffer)
					free(buf.szValue);

			} else if (WFD_RTSP_PAYLOAD_BUILT != lRet) {
				lRet = WFD_BuildPayloadFromStruct(p, pToken, &buf);
				if (buf.szValue != buf.szStaticBuffer)
					free(buf.szValue);

				if (WFD_RTSP_NO_ERROR != lRet) {
					return lRet;
				}
			}

			pToken = strtok_s(NULL, ", \r\n", &(p->m_pStrtokContext));
		}

		lRet = WFD_SendGetParametersResponse(p, p->pParsed->nCSeq,
			(char*)p->m_pOutputBuffer);
		if (WFD_RTSP_NO_ERROR != lRet) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendGetParametersResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq, p->m_pOutputBuffer);
			return WFD_RTSP_INTERNAL_ERROR;
		}

	}

	return 0;
}

long WFD_BuildPayloadFromStruct(CRTSPParser* p, char* pToken, WFD_PARAMETER_STRUCT* pBuf)
{
	long nAdditionalLen=0;
	int nLen=0;

	if (CountOfWFDParams > pBuf->nId && !p->m_bSupportedWFDCommands[pBuf->nId]) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s not supported", __FUNCTION__, g_szWFDParams[pBuf->nId]);
		return WFD_RTSP_INTERNAL_ERROR;
	} else if (wfd_content_protection == pBuf->nId) {

		nAdditionalLen = (long)(strlen(pToken))+2 + 21; // HDCPX.Y port=65536\r\n

		if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, pBuf->szValue);
			return WFD_RTSP_INSUFFICIENT_BUF;
		}

		if (pBuf->u.contentProtection.usPort) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: HDCP%.1f port=%d\r\n", pToken, pBuf->u.contentProtection.fHDCPVersion, pBuf->u.contentProtection.usPort);
		} else {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: none\r\n", pToken);
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_route == pBuf->nId) {

		if (WFD_PRIMARY_ROUTE == pBuf->u.route) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: primary\r\n", g_szWFDParams[pBuf->nId]);
		} else if (WFD_SECONDARY_ROUTE == pBuf->u.route) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: secondary\r\n", g_szWFDParams[pBuf->nId]);
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_connector_type == pBuf->nId) {

		if (pBuf->u.connectorType.ucSupported) {
			if (pBuf->u.connectorType.ucDisconnected) {
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: none\r\n", g_szWFDParams[pBuf->nId]);
			} else if (WFD_UNKNOWN_CONNECTOR == pBuf->u.connectorType.ucType) {
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: 255\r\n", g_szWFDParams[pBuf->nId]);
			} else {
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: %.2x\r\n", g_szWFDParams[pBuf->nId], (int)(pBuf->u.connectorType.ucType));
			}
		} else {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: none\r\n", g_szWFDParams[pBuf->nId]);
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_standby_resume_capability == pBuf->nId) {

		if (pBuf->u.standby_resume_supported) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: supported\r\n", g_szWFDParams[pBuf->nId]);
		} else {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: none\r\n", g_szWFDParams[pBuf->nId]);
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_I2C == pBuf->nId) {

		if (pBuf->u.i2c.nSupported) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %d\r\n", g_szWFDParams[pBuf->nId], pBuf->u.i2c.nPort);
		} else {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: none\r\n", g_szWFDParams[pBuf->nId]);
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_client_rtp_ports == pBuf->nId) {

		if (CountOfWFD_RtpMode <= pBuf->u.clientPorts.eMode) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Invalid mode index(%d) for %s", __FUNCTION__, pBuf->u.clientPorts.eMode, g_szWFDParams[pBuf->nId]);
			return WFD_RTSP_INVALID_ID;
		}

		nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: %s %d %d %s\r\n", 
					g_szWFDParams[pBuf->nId], 
					pBuf->u.clientPorts.szProfile,
					(int)pBuf->u.clientPorts.usPort0,
					(int)pBuf->u.clientPorts.usPort1,
					(CountOfWFD_RtpMode > pBuf->u.clientPorts.eMode)?g_szWFD_RtpMode[pBuf->u.clientPorts.eMode]:"UNKNOWN"
					);

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_coupled_sink == pBuf->nId) {
		if (pBuf->u.coupledSink.ucSupported) {
			if (0x1 == pBuf->u.coupledSink.ucStatus) {
				// coupled
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: %.2x %.2x%.2x%.2x%.2x%.2x%.2x\r\n", 
						g_szWFDParams[pBuf->nId], 
						(int)pBuf->u.coupledSink.ucStatus,
						(int)pBuf->u.coupledSink.aucMACAddr[0],
						(int)pBuf->u.coupledSink.aucMACAddr[1],
						(int)pBuf->u.coupledSink.aucMACAddr[2],
						(int)pBuf->u.coupledSink.aucMACAddr[3],
						(int)pBuf->u.coupledSink.aucMACAddr[4],
						(int)pBuf->u.coupledSink.aucMACAddr[5]);
			} else {
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: %.2x none\r\n", 
						g_szWFDParams[pBuf->nId], 
						(int)pBuf->u.coupledSink.ucStatus);
			}
		} else {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: none\r\n", g_szWFDParams[pBuf->nId]);
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_display_edid == pBuf->nId) {

		int i;

		if (pBuf->u.displayEdid.usLength)
			nAdditionalLen = (long)(strlen(pToken))+2*pBuf->u.displayEdid.usLength+9; // +9 for ': xxxx ' and CRLF
		else
			nAdditionalLen = (long)(strlen(pToken))+4+4; // "none" +4 for ': ' and CRLF

		if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, pBuf->szValue);
			return WFD_RTSP_INSUFFICIENT_BUF;
		}

		if (pBuf->u.displayEdid.usLength) {
			p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %.4x ", g_szWFDParams[pBuf->nId], pBuf->u.displayEdid.usLength/128);

			for (i=0; i < pBuf->u.displayEdid.usLength; i++) {
				p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%.2x", pBuf->u.displayEdid.pucEdid[i]);
			}
		} else {
			p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: none", g_szWFDParams[pBuf->nId]);
		}

		p->m_nOutputBufferUsed +=
			wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed, "\r\n");

	} else if (wfd_audio_codecs == pBuf->nId) {

		BOOL bFirst=TRUE;
		int i;

		nAdditionalLen = (long)(strlen(pToken))+4; // +4 for ': ' and CRLF

		if (!pBuf->u.audioCodecs.supported) {
			p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: none", g_szWFDParams[pBuf->nId]);
		} else {
			for (i=0; i < pBuf->u.audioCodecs.num_codecs; i++) {
				if (bFirst) {
					p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: %s %.8x %.2x", g_szWFDParams[pBuf->nId], g_szWFD_AudioCodecs[pBuf->u.audioCodecs.aCodecs[i].audio_format],
						pBuf->u.audioCodecs.aCodecs[i].mode, pBuf->u.audioCodecs.aCodecs[i].latency);
					bFirst = FALSE;
				} else
					p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						", %s %.8x %.2x", g_szWFD_AudioCodecs[pBuf->u.audioCodecs.aCodecs[i].audio_format],
						pBuf->u.audioCodecs.aCodecs[i].mode, pBuf->u.audioCodecs.aCodecs[i].latency);
			}
		}

		p->m_nOutputBufferUsed +=
			wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed, "\r\n");
	} else if (wfd_video_formats == pBuf->nId || wfd_3d_video_formats == pBuf->nId) {

		BOOL bFirst=TRUE;
		int i;

		nAdditionalLen = (long)(strlen(pToken))+4; // +4 for ': ' and CRLF

		if (!pBuf->u.videoFormat.supported) {
			p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: none", g_szWFDParams[pBuf->nId]);
		} else {
			for (i=0; i < pBuf->u.videoFormat.num_profiles; i++) {
				char szRes[10];

				if (pBuf->u.videoFormat.preferred_display_mode_supported) {
					wfd_snprintf_(szRes, 10, "%.4x %.4x", 
						(int)pBuf->u.videoFormat.aProfiles[i].max_hres,
						(int)pBuf->u.videoFormat.aProfiles[i].max_vres);
				} else {
					strcpy(szRes, "none none");
				}

				if (bFirst) {
					p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: %.2x %.2x %.2x %.2x", 
						g_szWFDParams[pBuf->nId], 
						(int)pBuf->u.videoFormat.native,
						(int)pBuf->u.videoFormat.preferred_display_mode_supported,

						(int)pBuf->u.videoFormat.aProfiles[i].profile,
						(int)pBuf->u.videoFormat.aProfiles[i].level
						);
					bFirst = FALSE;
				} else
					p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						", %.2x %.2x", 
						(int)pBuf->u.videoFormat.aProfiles[i].profile,
						(int)pBuf->u.videoFormat.aProfiles[i].level
						);

				if (wfd_video_formats == pBuf->nId) {
					p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						" %.8x %.8x %.8x", 
						(int)pBuf->u.videoFormat.aProfiles[i].misc_params.CEA_Support,
						(int)pBuf->u.videoFormat.aProfiles[i].misc_params.VESA_Support,
						(int)pBuf->u.videoFormat.aProfiles[i].misc_params.HH_Support
						);
				} else {
					p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						" %.16llx", 
						(uint64_t)pBuf->u.videoFormat.aProfiles[i].misc_params.S3D_Capability
						);
				}

				p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					" %.2x %.4x %.4x %.2x %s", 
					(int)pBuf->u.videoFormat.aProfiles[i].misc_params.latency,
					(int)pBuf->u.videoFormat.aProfiles[i].misc_params.min_slice_size,
					(int)pBuf->u.videoFormat.aProfiles[i].misc_params.slice_enc_params,
					(int)pBuf->u.videoFormat.aProfiles[i].misc_params.frame_rate_control_support,
					szRes);
			}
		}

		p->m_nOutputBufferUsed +=
			wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed, "\r\n");

	} else if (wfd_preferred_display_mode == pBuf->nId && pBuf->u.prefDispMode.supported) {

		BOOL bFirst=TRUE;

		nAdditionalLen = (long)(strlen(pToken))+4; // +4 for ': ' and CRLF

		p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
			"%s: %.6lx %.4lx %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.2x %.2x %.2x %.2x %.2x", 
			g_szWFDParams[pBuf->nId], 
			(unsigned int)pBuf->u.prefDispMode.pixel_clock_in_10khz_units,
			(unsigned int)pBuf->u.prefDispMode.horizontal_active_resolution,
			(unsigned int)pBuf->u.prefDispMode.horizontal_blanking_period,
			(unsigned int)((0 <= pBuf->u.prefDispMode.horizontal_sync_offset)
				?(0x800+pBuf->u.prefDispMode.horizontal_sync_offset)
				:(-pBuf->u.prefDispMode.horizontal_sync_offset)),
			(unsigned int)pBuf->u.prefDispMode.horizontal_sync_width,
			(unsigned int)pBuf->u.prefDispMode.vertical_active_resolution,
			(unsigned int)pBuf->u.prefDispMode.vertical_blanking_period,
			(unsigned int)((0 <= pBuf->u.prefDispMode.vertical_sync_offset)
				?(0x800+pBuf->u.prefDispMode.vertical_sync_offset)
				:(-pBuf->u.prefDispMode.vertical_sync_offset)),
			(unsigned int)pBuf->u.prefDispMode.vertical_sync_width,
			(unsigned int)pBuf->u.prefDispMode._3d_vertical_blanking_interval,
			(unsigned int)pBuf->u.prefDispMode._2d_s3d_modes,
			(unsigned int)pBuf->u.prefDispMode.pixel_depth,

			(unsigned int)pBuf->u.prefDispMode.h264_profile.profile,
			(unsigned int)pBuf->u.prefDispMode.h264_profile.level
			);

		if (0x1e & pBuf->u.prefDispMode._2d_s3d_modes) {
			p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				" %.16llx", 
				(unsigned int)0 // pBuf->u.prefDispMode.h264_profile.misc_params.S3D_Capability,
				);
		} else {
			p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				" %.8x %.8x %.8x", 
				(unsigned int)0, //pBuf->u.prefDispMode.h264_profile.misc_params.CEA_Support,
				(unsigned int)0, //pBuf->u.prefDispMode.h264_profile.misc_params.VESA_Support,
				(unsigned int)0 //pBuf->u.prefDispMode.h264_profile.misc_params.HH_Support,
				);
		}

		p->m_nOutputBufferUsed += wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
			" %.2x %.4x %.4x %.2x %.4x %.4x", 
			(unsigned int)pBuf->u.prefDispMode.h264_profile.misc_params.latency,
			(unsigned int)pBuf->u.prefDispMode.h264_profile.misc_params.min_slice_size,
			(unsigned int)pBuf->u.prefDispMode.h264_profile.misc_params.slice_enc_params,
			(unsigned int)pBuf->u.prefDispMode.h264_profile.misc_params.frame_rate_control_support,
			(unsigned int)pBuf->u.prefDispMode.h264_profile.max_hres,
			(unsigned int)pBuf->u.prefDispMode.h264_profile.max_vres
			);

		p->m_nOutputBufferUsed +=
			wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed, "\r\n");

	} else if (wfd_av_format_change_timing == pBuf->nId) {
		nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: %.10llx %.10llx\r\n", 
					g_szWFDParams[pBuf->nId], 
					((uint64_t)pBuf->u.avTiming.ullPTS)<<7,
					((uint64_t)pBuf->u.avTiming.ullDTS)<<7);

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_uibc_capability == pBuf->nId) {

		if (!pBuf->u.uibcCapability.ucSupported) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: none\r\n", g_szWFDParams[pBuf->nId]);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
		} else {
			if (pBuf->u.uibcCapability.aucCategory[WFD_GENERIC] || pBuf->u.uibcCapability.aucCategory[WFD_HIDC]) {
				BOOL bFirst=TRUE;
				int i;

				for (i=0; i < CountOfWFD_InputCategory; i++) {
					if (pBuf->u.uibcCapability.aucCategory[i]) {
						if (bFirst) {
							bFirst = FALSE;
							nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
									"%s: input_category_list=%s", g_szWFDParams[pBuf->nId], g_szWFD_InputCategory[i]);
						} else {
							nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
									", %s", g_szWFD_InputCategory[i]);
						}

						if (0 < nLen)
							p->m_nOutputBufferUsed += nLen;
					}
				}

				bFirst = TRUE;
				for (i=0; i < CountOfWFD_InputType; i++) {
					if (pBuf->u.uibcCapability.aucType[i]) {
						if (bFirst) {
							bFirst = FALSE;
							nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
									";generic_cap_list=%s", g_szWFD_InputType[i]);
						} else {
							nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
									", %s", g_szWFD_InputType[i]);
						}

						if (0 < nLen)
							p->m_nOutputBufferUsed += nLen;
					}
				}

				// No generic capabilities
				if (bFirst)
				{
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
							";generic_cap_list=none");
					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;
				}

				bFirst = TRUE;
				for (i=0; i < pBuf->u.uibcCapability.usCountOfCapPairs; i++) {
					if (bFirst) {
						bFirst = FALSE;
						nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
								";hidc_cap_list=%s/%s", 
								g_szWFD_InputType[pBuf->u.uibcCapability.pCapPairs[i].eType], 
								g_szWFD_InputPath[pBuf->u.uibcCapability.pCapPairs[i].ePath]);
					} else {
						nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
								", %s/%s", 
								g_szWFD_InputType[pBuf->u.uibcCapability.pCapPairs[i].eType], 
								g_szWFD_InputPath[pBuf->u.uibcCapability.pCapPairs[i].ePath]);
					}

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;
				}

				// No hidc capabilities
				if (bFirst)
				{
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
							";hidc_cap_list=none", g_szWFD_InputCategory[i]);
					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;
				}

				if (pBuf->u.uibcCapability.usPort)
				{
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						";port=%d\r\n", (int)pBuf->u.uibcCapability.usPort);
				}
				else
				{
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						";port=none\r\n");
				}

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

			} else {
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: input_category_list= none; generic_cap_list= none; hidc_cap_list= none; port= none\r\n", 
						g_szWFDParams[pBuf->nId]);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;
			}
		}
	}  else if (wfd_uibc_setting == pBuf->nId) {

		nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %s\r\n", g_szWFDParams[pBuf->nId], pBuf->u.uibcSetting.ucEnabled ? "enable" : "disable");
		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else if (wfd_presentation_URL == pBuf->nId) {
		if (pBuf->u.presentationURL.szStream1Addr[0]) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: rtsp://%s/wfd1.0/streamid=0 rtsp://%s/wfd1.0/streamid=1\r\n", 
						g_szWFDParams[pBuf->nId], 
						pBuf->u.presentationURL.szStream0Addr,
						pBuf->u.presentationURL.szStream1Addr);
		} else if (pBuf->u.presentationURL.szStream0Addr[0]) {
			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: rtsp://%s/wfd1.0/streamid=0 none\r\n", 
						g_szWFDParams[pBuf->nId], 
						pBuf->u.presentationURL.szStream0Addr);
		} else {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Invalid values in presentationUrl union member", __FUNCTION__);
			return WFD_RTSP_INVALID_WFD_PARAMS;
		}

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;

	} else {

		//nAdditionalLen = (long)(strlen(pToken))+2 + (long)(strlen(pBuf->szValue))+2;
		nAdditionalLen = (long)(strlen(pToken))+2 + (long)(strlen("none"))+2;

		if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, pBuf->szValue);
			return WFD_RTSP_INSUFFICIENT_BUF;
		}

		//nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
		//	"%s: %s\r\n", pToken, pBuf->szValue);

		nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
			"%s: %s\r\n", pToken, "none");

		if (0 < nLen)
			p->m_nOutputBufferUsed += nLen;
	}

	return WFD_RTSP_NO_ERROR;
}

BOOL GetNextHexByte(CRTSPParser* p, unsigned char* ucRes, char* szText, int* pnContext)
{
	unsigned char ucHex=0;
	int i, j;

	for (i=*pnContext; ' ' == szText[i] && 0 != szText[i]; i++)
		;

	if (0 == szText[i])
		return FALSE;

	for (j=0; j < 2 && (('0' <= szText[i] && szText[i] <= '9') || ('A' <= szText[i] && szText[i] <= 'F') || ('a' <= szText[i] && szText[i] <= 'f')); j++,i++) {
		ucHex <<= 4;
		ucHex |= g_cAtoH[(int)szText[i]];
	}

	if (2 != j) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected (2) bytes, but read (%d) bytes, then read invalid character (%c)", __FUNCTION__, j, szText[i]);
		return FALSE;
	}

	*pnContext = i;
	*ucRes = ucHex;

	return TRUE;
}

BOOL GetNextHexShort(CRTSPParser* p, unsigned short* usRes, char* szText, int* pnContext)
{
	unsigned short usHex=0;
	int i, j;

	for (i=*pnContext; ' ' == szText[i] && 0 != szText[i]; i++)
		;

	if (0 == szText[i])
		return FALSE;

	for (j=0; j < 4 && (('0' <= szText[i] && szText[i] <= '9') || ('A' <= szText[i] && szText[i] <= 'F') || ('a' <= szText[i] && szText[i] <= 'f')); j++,i++) {
		usHex <<= 4;
		usHex |= g_cAtoH[(int)szText[i]];
	}

	if (4 != j) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected (4) bytes, but read (%d) bytes, then read invalid character (%c)", __FUNCTION__, j, szText[i]);
		return FALSE;
	}

	*pnContext = i;
	*usRes = usHex;

	return TRUE;
}

BOOL GetNextHex6(CRTSPParser* p, unsigned long* ulRes, char* szText, int* pnContext)
{
	unsigned long ulHex=0;
	int i, j;

	for (i=*pnContext; ' ' == szText[i] && 0 != szText[i]; i++)
		;

	if (0 == szText[i])
		return FALSE;

	for (j=0; j < 6 && (('0' <= szText[i] && szText[i] <= '9') || ('A' <= szText[i] && szText[i] <= 'F') || ('a' <= szText[i] && szText[i] <= 'f')); j++,i++) {
		ulHex <<= 4;
		ulHex |= g_cAtoH[(int)szText[i]];
	}

	if (6 != j) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected (6) bytes, but read (%d) bytes, then read invalid character (%c)", __FUNCTION__, j, szText[i]);
		return FALSE;
	}

	*pnContext = i;
	*ulRes = ulHex;

	return TRUE;
}

BOOL GetNextHexLong(CRTSPParser* p, unsigned long* ulRes, char* szText, int* pnContext)
{
	unsigned long ulHex=0;
	int i, j;

	for (i=*pnContext; ' ' == szText[i] && 0 != szText[i]; i++)
		;

	if (0 == szText[i])
		return FALSE;

	for (j=0; j < 8 && (('0' <= szText[i] && szText[i] <= '9') || ('A' <= szText[i] && szText[i] <= 'F') || ('a' <= szText[i] && szText[i] <= 'f')); j++,i++) {
		ulHex <<= 4;
		ulHex |= g_cAtoH[(int)szText[i]];
	}

	if (8 != j) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected (8) bytes, but read (%d) bytes, then read invalid character (%c)", __FUNCTION__, j, szText[i]);
		return FALSE;
	}

	*pnContext = i;
	*ulRes = ulHex;

	return TRUE;
}

BOOL GetNextHexLongLong(CRTSPParser* p, uint64_t* ullRes, char* szText, int* pnContext)
{
	uint64_t ullHex=0;
	int i, j;

	for (i=*pnContext; ' ' == szText[i] && 0 != szText[i]; i++)
		;

	if (0 == szText[i])
		return FALSE;

	for (j=0; j < 16 && (('0' <= szText[i] && szText[i] <= '9') || ('A' <= szText[i] && szText[i] <= 'F') || ('a' <= szText[i] && szText[i] <= 'f')); j++,i++) {
		ullHex <<= 4;
		ullHex |= g_cAtoH[(int)szText[i]];
	}

	if (16 != j) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected (16) bytes, but read (%d) bytes, then read invalid character (%c)", __FUNCTION__, j, szText[i]);
		return FALSE;
	}

	*pnContext = i;
	*ullRes = ullHex;

	return TRUE;
}

BOOL GetNextHex10(CRTSPParser* p, uint64_t* ullRes, char* szText, int* pnContext)
{
	uint64_t ullHex=0;
	int i, j;

	for (i=*pnContext; ' ' == szText[i] && 0 != szText[i]; i++)
		;

	if (0 == szText[i])
		return FALSE;

	for (j=0; j < 10 && (('0' <= szText[i] && szText[i] <= '9') || ('A' <= szText[i] && szText[i] <= 'F') || ('a' <= szText[i] && szText[i] <= 'f')); j++,i++) {
		ullHex <<= 4;
		ullHex |= g_cAtoH[(int)szText[i]];
	}

	if (10 != j) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Expected (10) bytes, but read (%d) bytes, then read invalid character (%c)", __FUNCTION__, j, szText[i]);
		return FALSE;
	}

	*pnContext = i;
	*ullRes = ullHex;

	return TRUE;
}

long WFD_ParseParametersAndNotify(CRTSPParser* p, int nNotifyMsg)
{
	long lRet = WFD_RTSP_NO_ERROR;
	char* pTok=NULL;
	char* pContext2;
	int i=0,j=0,k=0;
	BOOL bStatus=FALSE;
	int nBusy=0;
	int nDisplayConnected=0;
	WFD_PARAMETER_STRUCT buf;
	BOOL bEdid=FALSE;
	BOOL bHDCP=FALSE;
	int iCnt;

	// Parse response from our request for paraemters
	char* pToken=strtok_s((char*)p->pParsed->aBuffer, "\r\n", &(p->m_pStrtokContext));

	// this value means no trigger
	p->m_Trigger = CountOfWFD_MethodTokens;

	while (pToken) {
		memset(&buf, 0, sizeof(buf));
		pTok = strtok_s(pToken, ": ", &pContext2);

		// get id of wfd command
		buf.nId = (WFDParams)GetIndex(pToken, CountOfWFDParams, g_szWFDParams);
		if (CountOfWFDParams <= buf.nId) {
			// Unrecognized command, send to app as a proprietary command
			WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - Unrecognized value (%s) for WFD Param", __FUNCTION__, pToken?pToken:"NULL");
			buf.szParam = pToken;
			buf.szValue = strtok_s(NULL,  "", &pContext2);
			// eat up leading spaces
			while (buf.szValue && ' ' == *buf.szValue)
				buf.szValue++;

			buf.nCount = (int)(buf.szValue?strlen(buf.szValue):0);

			lRet = p->m_pNotify((WFD_RTSPNotifications)nNotifyMsg, &buf, p->m_pContext);
			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
				lRet = WFD_RTSP_INTERNAL_ERROR;
				goto cleanup;
			}

		} 
		else if (p->m_bStandbyMode && wfd_standby != buf.nId)
		{
			// Only message accepted while in standby mode is M12 (wfd_standby)
			lRet = WFD_RTSP_STANDBY_MODE;
			goto cleanup;
		}
		else {

			// szValue will point to command so app can look at it even if it has been parsed
			buf.szValue = strtok_s(NULL,  "", &pContext2);
			// eat up leading spaces
			while (buf.szValue && ' ' == *buf.szValue)
				buf.szValue++;

			buf.nCount = (int)(buf.szValue?strlen(buf.szValue):0);

			if (!p->m_bSupportedWFDCommands[buf.nId]) {
				p->m_nRTSPReponseCode = 551; // option not supported
				wfd_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "%s not supported", g_szWFDParams[buf.nId]);
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %s", __FUNCTION__, p->m_szErrorMessage);
				lRet = WFD_RTSP_INTERNAL_ERROR;
				goto cleanup;

			} else if (wfd_trigger_method == buf.nId) {

				p->m_Trigger = (WFD_MethodTokens)GetIndex(buf.szValue, CountOfWFD_MethodTokens, g_szWFD_MethodTokens);

				// any trigger puts us in state M5
				p->m_eState = WFD_RTSP_M05;
				p->m_ulStartTime[p->m_eState] = p->m_pGetTick(p->m_pContext);

				if (MSETUP == p->m_Trigger) {
					WFD_RTSP_LOG(p, RTSP_LOG_INFO, "%s - M%d received from source in %d msec, %d msec from start", __FUNCTION__, p->m_eState, p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[p->m_eState-1], p->m_ulStartTime[p->m_eState]-p->m_ulStartTime[0]);
				}
			} else if (wfd_presentation_URL == buf.nId) {
				strncpy(p->m_szPresentationUrl0, buf.szValue, WFD_PRESENTATION_URL_LEN+1);
				iCnt=sscanf(buf.szValue, "%s %s", p->m_szPresentationUrl0, p->m_szPresentationUrl1);
				if (1 > iCnt) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_presentation_URL", __FUNCTION__);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				if (WFD_RTSP_SET_PARAMETER == nNotifyMsg) {
					// parse server IP address from url
					char* pContext=NULL;
					char szTmp[50];
					char* pTok2=NULL;

					strncpy(szTmp, buf.szValue, 50);
					szTmp[49] = 0;

					pTok2 = strtok_s(szTmp, "/", &pContext);
					if (pTok2) {
						pTok2 = strtok_s(NULL, "/", &pContext);

						if (pTok2)
							strncpy(p->m_szServerIpAddress, pTok2, RTSP_MAX_IPADDR_BUFFER);
					}
				}

			} else if (wfd_content_protection == buf.nId) {

				buf.u.contentProtection.fHDCPVersion = 0;
				buf.u.contentProtection.usPort = 0;

				// got HDCP message
				bHDCP = TRUE;

				if (strcmp(buf.szValue, "none")) {
					iCnt=sscanf(buf.szValue, "HDCP%f port=%hd", &buf.u.contentProtection.fHDCPVersion, &buf.u.contentProtection.usPort);
					if (2 != iCnt) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_content_protection", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					} else if (buf.u.contentProtection.usPort) {
						p->m_bHDCP = TRUE;
					}
				}
			} else if (wfd_route == buf.nId) {

				// got route message
				if (strstr(buf.szValue, "primary"))
					buf.u.route = WFD_PRIMARY_ROUTE;
				else if (strstr(buf.szValue, "secondary"))
					buf.u.route = WFD_SECONDARY_ROUTE;
				else {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_route: %s", __FUNCTION__, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}
			} else if (wfd_standby_resume_capability == buf.nId) {

				// got standby resume capability
				if (strstr(buf.szValue, "supported"))
					buf.u.standby_resume_supported = 1;
				else if (strstr(buf.szValue, "none"))
					buf.u.standby_resume_supported = 0;
				else {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_route: %s", __FUNCTION__, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}
			} else if (wfd_standby == buf.nId) {
				// Don't set m_bStandbyMode to TRUE, because this message just indicates that the SINK
				// is going into standby mode. The source's mode should not change.
				//p->m_bStandbyMode = TRUE;
			}
			else if (wfd_connector_type == buf.nId) {
				int nContext=0;

				// got connector type
				if (strstr(buf.szValue, "none")) {
					buf.u.connectorType.ucSupported = 1;
					buf.u.connectorType.ucDisconnected = 1;
				} else {
					unsigned char ucByte;

					buf.u.connectorType.ucSupported = 1;
					buf.u.connectorType.ucDisconnected = 0;
					if (!GetNextHexByte(p, &ucByte, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Unexpected character (%c) while looking for connector type at position %d", __FUNCTION__, buf.szValue[nContext], nContext);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}

					if (255 == ucByte)
						buf.u.connectorType.ucType = WFD_UNKNOWN_CONNECTOR;
					else
						buf.u.connectorType.ucType = ucByte;
				}
			} else if (wfd_I2C == buf.nId) {

				// got I2C message
				if (!strncmp(buf.szValue, "none", 4)) {
					buf.u.i2c.nSupported = 0;
				} else {
					buf.u.i2c.nSupported = 1;
					iCnt=sscanf(buf.szValue, "%d", &buf.u.i2c.nPort);
					if (1 != iCnt) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_I2C", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}
				}
			} else if (wfd_client_rtp_ports == buf.nId) {
				char szTmp[50];

				buf.u.clientPorts.szProfile = buf.szStaticBuffer;

				iCnt=sscanf(buf.szValue, "%s %hd %hd %s", buf.u.clientPorts.szProfile, &buf.u.clientPorts.usPort0, &buf.u.clientPorts.usPort1, szTmp);
				if (4 == iCnt) {
					buf.u.clientPorts.eMode = (WFD_RtpMode)GetIndex(szTmp, CountOfWFD_RtpMode, g_szWFD_RtpMode);
				}

				if (4 != iCnt || CountOfWFD_RtpMode == buf.u.clientPorts.eMode) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_client_rtp_ports", __FUNCTION__);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

			} else if (wfd_coupled_sink == buf.nId) {
				if (!strncmp(buf.szValue, "none", 4)) {
					buf.u.coupledSink.ucSupported = 0;
				} else {
					int nContext=0;
	
					buf.u.coupledSink.ucSupported = 1;

					if (!GetNextHexByte(p, &buf.u.coupledSink.ucStatus, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Unexpected character (%c) while looking for coupled sink status at position %d", __FUNCTION__, buf.szValue[nContext], nContext);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}

					if (buf.u.coupledSink.ucStatus) {
						// sink
						for (i=0; i < 6; i++) {
							if (!GetNextHexByte(p, buf.u.coupledSink.aucMACAddr+i, buf.szValue, &nContext)) {
								WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Unexpected character (%c) while looking for mac address at position %d", __FUNCTION__, pToken[nContext], nContext);
								lRet = WFD_RTSP_INTERNAL_ERROR;
								goto cleanup;
							}
						}
					} else {
						memset(buf.u.coupledSink.aucMACAddr, 0, 6);
					}
				}
			} else if (wfd_display_edid == buf.nId) {
				int i;
				int nContext=0;

				bEdid = TRUE;

				pToken = buf.szValue;
				
				if (!strcmp(pToken, "none")) {
					buf.u.displayEdid.usLength = 0;
					buf.u.displayEdid.pucEdid = NULL;
				} else {
					if (!GetNextHexShort(p, &buf.u.displayEdid.usLength, pToken, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing block size or after col %d in %s", __FUNCTION__, nContext, pToken);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}

					buf.u.displayEdid.usLength *= 128;
					buf.u.displayEdid.pucEdid = (unsigned char*)malloc(buf.u.displayEdid.usLength);

					for (i=0; i < buf.u.displayEdid.usLength; i++) {
						if (!GetNextHexByte(p, buf.u.displayEdid.pucEdid+i, pToken, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Unexpected character (%c) while looking for edid hex digits at position %d", __FUNCTION__, pToken[nContext], nContext);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}
					}
				}

			} else if (wfd_audio_codecs == buf.nId) {
				int nContext=0;
				unsigned short ulProfile=0;
				char* pContext;

				buf.u.audioCodecs.num_codecs = 0;
				buf.u.audioCodecs.aCodecs = NULL;

				if (!strncmp(buf.szValue, "none", 4))
					buf.u.audioCodecs.supported = 0;
				else {
					buf.u.audioCodecs.supported = 1;

					buf.u.audioCodecs.num_codecs = 0;
					buf.u.audioCodecs.aCodecs = (WFD_AUDIO_CODEC*)malloc(1); // we just do this allocation so we can realloc in the loop

					pToken = strtok_s(buf.szValue,  " \r\n", &pContext);
					
					while (pToken) {
						// if this succeeds, then we have a codec
						buf.u.audioCodecs.aCodecs = (WFD_AUDIO_CODEC*)realloc(buf.u.audioCodecs.aCodecs, (buf.u.audioCodecs.num_codecs+1)*sizeof(WFD_AUDIO_CODEC));
						if (!buf.u.audioCodecs.aCodecs) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failure allocating (%d) bytes", __FUNCTION__, (buf.u.audioCodecs.num_codecs+1)*sizeof(WFD_AUDIO_CODEC));
							lRet = WFD_RTSP_MEMORY;
							goto cleanup;
						}

						buf.u.audioCodecs.aCodecs[buf.u.audioCodecs.num_codecs].audio_format = GetIndex(pToken, CountOfWFD_AudioCodecs, g_szWFD_AudioCodecs);

						nContext = strlen(pToken)+1;
						
						if (!GetNextHexLong(p, &buf.u.audioCodecs.aCodecs[buf.u.audioCodecs.num_codecs].mode, pToken, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing format on or after col %d in %s %s", __FUNCTION__, nContext, pToken, pContext);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (!GetNextHexByte(p, &buf.u.audioCodecs.aCodecs[buf.u.audioCodecs.num_codecs].latency, pToken, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing latency on or after col %d in %s %s", __FUNCTION__, nContext, pToken, pContext);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						pToken = strtok_s(pToken+nContext,  " ,\r\n", &pContext);
					
						buf.u.audioCodecs.num_codecs++;
					}
				}

			} else if (wfd_video_formats == buf.nId || wfd_3d_video_formats == buf.nId) {
				int nContext=0;
				unsigned char usProfile=0;
				buf.u.videoFormat.num_profiles = 0;

				if (!strncmp(buf.szValue, "none", 4))
					buf.u.videoFormat.supported = 0;
				else {
					buf.u.videoFormat.supported = 1;

					// native
					if (!GetNextHexByte(p, &buf.u.videoFormat.native, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'native' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}

					if (!GetNextHexByte(p, &buf.u.videoFormat.preferred_display_mode_supported, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'preferred_display_mode_supported' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}
					p->m_bPreferredDisplayModeSupported = buf.u.videoFormat.preferred_display_mode_supported;
					p->m_bPreferredDisplayModeSent = FALSE;

					buf.u.videoFormat.aProfiles = (WFD_H264_PROFILE*)malloc(1); // we just do this allocation so we can realloc in the loop

					while (GetNextHexByte(p, &usProfile, buf.szValue, &nContext)) {
						// if this succeeds, then we have a profile
						buf.u.videoFormat.aProfiles = (WFD_H264_PROFILE*)realloc(buf.u.videoFormat.aProfiles, (buf.u.videoFormat.num_profiles+1)*sizeof(WFD_H264_PROFILE));
						if (!buf.u.videoFormat.aProfiles) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failure allocating (%d) bytes", __FUNCTION__, (buf.u.videoFormat.num_profiles+1)*sizeof(WFD_H264_PROFILE));
							lRet = WFD_RTSP_MEMORY;
							goto cleanup;
						}

						buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].profile = usProfile;

						if (!GetNextHexByte(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].level, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'level' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (wfd_video_formats == buf.nId) {
							if (!GetNextHexLong(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.CEA_Support, buf.szValue, &nContext)) {
								WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'CEA_Support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
								lRet = WFD_RTSP_INTERNAL_ERROR;
								goto cleanup;
							}

							if (!GetNextHexLong(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.VESA_Support, buf.szValue, &nContext)) {
								WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'VESA_Support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
								lRet = WFD_RTSP_INTERNAL_ERROR;
								goto cleanup;
							}

							if (!GetNextHexLong(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.HH_Support, buf.szValue, &nContext)) {
								WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'HH_Support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
								lRet = WFD_RTSP_INTERNAL_ERROR;
								goto cleanup;
							}
						} else {
							if (!GetNextHexLongLong(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.S3D_Capability, buf.szValue, &nContext)) {
								WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing '3D_Capability' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
								lRet = WFD_RTSP_INTERNAL_ERROR;
								goto cleanup;
							}
						}

						if (!GetNextHexByte(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.latency, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'latency' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (!GetNextHexShort(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.min_slice_size, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'min_slice_size' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (!GetNextHexShort(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.slice_enc_params, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'slice_enc_params' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (!GetNextHexByte(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].misc_params.frame_rate_control_support, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'frame_rate_control_support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (!strncmp(&buf.szValue[nContext+1], "none", 4)) {
							buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].max_hres = 0;
							nContext += 5;
						} else if (!GetNextHexShort(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].max_hres, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'max_hres' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						if (!strncmp(&buf.szValue[nContext+1], "none", 4)) {
							buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].max_vres = 0;
							nContext += 5;
						} else if (!GetNextHexShort(p, &buf.u.videoFormat.aProfiles[buf.u.videoFormat.num_profiles].max_vres, buf.szValue, &nContext)) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'max_vres' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						buf.u.videoFormat.num_profiles++;
						while (buf.szValue[nContext] == ' ' || buf.szValue[nContext] == ',')
							nContext++;
					}
				}
			} else if (wfd_preferred_display_mode == buf.nId) {
				int nContext=0;
				unsigned char usProfile=0;
				unsigned short us=0;

				buf.u.videoFormat.num_profiles = 0;
				buf.u.prefDispMode.supported = 1;

				// pixel_clock_in_10khz_units
				if (!GetNextHex6(p, &buf.u.prefDispMode.pixel_clock_in_10khz_units, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'pixel_clock_in_10khz_units' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// horizontal_active_resolution
				if (!GetNextHexShort(p, &buf.u.prefDispMode.horizontal_active_resolution, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'horizontal_active_resolution' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// horizontal_blanking_period
				if (!GetNextHexShort(p, &buf.u.prefDispMode.horizontal_blanking_period, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'horizontal_blanking_period' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// horizontal_sync_offset
				if (!GetNextHexShort(p, &us, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'horizontal_sync_offset' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}
				if ((0x8000 & us) == 0) {
					// negative
					buf.u.prefDispMode.horizontal_sync_offset = -us;
				} else {
					// positive
					buf.u.prefDispMode.horizontal_sync_offset = (0x7fff & us);
				}

				// horizontal_sync_width
				if (!GetNextHexShort(p, &buf.u.prefDispMode.horizontal_sync_width, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'horizontal_sync_width' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// vertical_active_resolution
				if (!GetNextHexShort(p, &buf.u.prefDispMode.vertical_active_resolution, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'vertical_active_resolution' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// vertical_blanking_period
				if (!GetNextHexShort(p, &buf.u.prefDispMode.vertical_blanking_period, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'vertical_blanking_period' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// vertical_sync_offset
				if (!GetNextHexShort(p, &us, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'vertical_sync_offset' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}
				if ((0x8000 & us) == 0) {
					// negative
					buf.u.prefDispMode.vertical_sync_offset = -us;
				} else {
					// positive
					buf.u.prefDispMode.vertical_sync_offset = (0x7fff & us);
				}

				// vertical_sync_width
				if (!GetNextHexShort(p, &buf.u.prefDispMode.vertical_sync_width, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'vertical_sync_width' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// 3d_vertical_blanking_interval
				if (!GetNextHexByte(p, &buf.u.prefDispMode._3d_vertical_blanking_interval, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing '3d_vertical_blanking_interval' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// 2d_s3d_modes
				if (!GetNextHexByte(p, &buf.u.prefDispMode._2d_s3d_modes, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing '2d_s3d_modes' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// pixel_depth
				if (!GetNextHexByte(p, &buf.u.prefDispMode.pixel_depth, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'pixel_depth' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.profile
				if (!GetNextHexByte(p, &buf.u.prefDispMode.h264_profile.profile, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.profile' on or after col %d in %s", __FUNCTION__, nContext, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.level
				if (!GetNextHexByte(p, &buf.u.prefDispMode.h264_profile.level, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.level' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				if (0x1e & buf.u.prefDispMode._2d_s3d_modes) {
					// h264_profile.misc_params.CEA_Support
					if (!GetNextHexLongLong(p, &buf.u.prefDispMode.h264_profile.misc_params.S3D_Capability, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.S3D_Capability' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}
				} else {
					// h264_profile.misc_params.CEA_Support
					if (!GetNextHexLong(p, &buf.u.prefDispMode.h264_profile.misc_params.CEA_Support, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.CEA_Support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}

					// h264_profile.misc_params.VESA_Support
					if (!GetNextHexLong(p, &buf.u.prefDispMode.h264_profile.misc_params.VESA_Support, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.VESA_Support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}

					// h264_profile.misc_params.HH_Support
					if (!GetNextHexLong(p, &buf.u.prefDispMode.h264_profile.misc_params.HH_Support, buf.szValue, &nContext)) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.HH_Support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}
				}

				// h264_profile.misc_params.latency
				if (!GetNextHexByte(p, &buf.u.prefDispMode.h264_profile.misc_params.latency, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.latency' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.misc_params.min_slice_size
				if (!GetNextHexShort(p, &buf.u.prefDispMode.h264_profile.misc_params.min_slice_size, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.min_slice_size' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.misc_params.slice_enc_params
				if (!GetNextHexShort(p, &buf.u.prefDispMode.h264_profile.misc_params.slice_enc_params, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.slice_enc_params' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.misc_params.frame_rate_control_support
				if (!GetNextHexByte(p, &buf.u.prefDispMode.h264_profile.misc_params.frame_rate_control_support, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.misc_params.frame_rate_control_support' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.max_hres
				if (!strncmp(&buf.szValue[nContext+1], "none", 4)) {
					buf.u.prefDispMode.h264_profile.max_hres = 0;
					nContext += 5;
				} else if (!GetNextHexShort(p, &buf.u.prefDispMode.h264_profile.max_hres, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.max_hres' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

				// h264_profile.max_vres
				if (!strncmp(&buf.szValue[nContext+1], "none", 4)) {
					buf.u.prefDispMode.h264_profile.max_vres = 0;
					nContext += 5;
				} else if (!GetNextHexShort(p, &buf.u.prefDispMode.h264_profile.max_vres, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'h264_profile.max_vres' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

			} else if (wfd_uibc_capability == buf.nId) {
				char* szToken;
				char* pContext;
				char* pInputCatVal = NULL;
				char* pGenericCapVal = NULL;
				char* pHIDCCapVal = NULL;
				char* pTcpPort = NULL;
				int id;
				unsigned char ucSupported = 0;

				if (!strncmp(buf.szValue, "none", 4))
					buf.u.uibcCapability.ucSupported = 0;
				else {
					pInputCatVal = strstr(buf.szValue, "input_category_list=");
					if (!pInputCatVal) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_uibc_capability, looking for input-category-val;", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					} else
						pInputCatVal += 20;

					pGenericCapVal = strstr(buf.szValue, "generic_cap_list=");
					if (!pGenericCapVal) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_uibc_capability, looking for generic-cap-val;", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					} else
						pGenericCapVal += 17;
						
					pHIDCCapVal = strstr(buf.szValue, "hidc_cap_list=");
					if (!pHIDCCapVal) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_uibc_capability, looking for hidc-cap-val;", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					} else
						pHIDCCapVal += 14;
						
					pTcpPort = strstr(buf.szValue, "port=");
					if (!pTcpPort) {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_uibc_capability, looking for tcp-port", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					} else
						pTcpPort += 5;

					szToken = strtok_s(pInputCatVal, " ,=;", &pContext);
					while (szToken) {

						if (!strncmp(szToken, "generic_cap_list", 16)) {
							break;
						}

						if (!strcmp(szToken, "none")) {
							break;
						}

						id = (WFD_InputCategory)GetIndex(szToken, CountOfWFD_InputCategory, g_szWFD_InputCategory);

						if (CountOfWFD_InputCategory == id) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected token (%s) while parsing input-cat in wfd_uibc_capability", __FUNCTION__, szToken);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						buf.u.uibcCapability.aucCategory[id] = 1;

						szToken = strtok_s(NULL, " ,;=", &pContext);
					}

					szToken = strtok_s(pGenericCapVal, " ,=;", &pContext);
					while (szToken) {

						if (!strncmp(szToken, "hidc_cap_list", 13)) {
							break;
						}

						if (!strcmp(szToken, "none")) {
							memset(buf.u.uibcCapability.aucType, 0, CountOfWFD_InputType*sizeof(buf.u.uibcCapability.aucType[0]));
							break;
						}

						id = (WFD_InputType)GetIndex(szToken, CountOfWFD_InputType, g_szWFD_InputType);

						if (CountOfWFD_InputType == i) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected token (%s) while parsing inp-typ in wfd_uibc_capability", __FUNCTION__, szToken);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						buf.u.uibcCapability.aucType[id] = 1;
						ucSupported = 1;

						szToken = strtok_s(NULL, " ,;=", &pContext);
					}

					szToken = strtok_s(pHIDCCapVal, " ,=;", &pContext);
					buf.u.uibcCapability.usCountOfCapPairs = 0;
					buf.u.uibcCapability.pCapPairs = malloc(0);

					while (szToken) {
						char* pContext2;
						char* szInputType;
						char* szInputPath;
						int nTypeId;
						int nPathId;

						if (!strncmp(szToken, "port", 4)) {
							break;
						}

						if (!strcmp(szToken, "none")) {
							buf.u.uibcCapability.usCountOfCapPairs = 0;
							break;
						}

						szInputType = strtok_s(szToken, "/", &pContext2);
						if (!szInputType) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error splitting input type/path (%s) while parsing detailed-cap in wfd_uibc_capability", __FUNCTION__, szToken);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						szInputPath = strtok_s(NULL, "", &pContext2);
						if (!szInputPath) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error splitting input type/path (%s) while parsing detailed-cap in wfd_uibc_capability", __FUNCTION__, szToken);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						nTypeId = (WFD_InputType)GetIndex(szInputType, CountOfWFD_InputType, g_szWFD_InputType);

						if (CountOfWFD_InputType == nTypeId) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected token (%s) while parsing inp-type from detailed-cap in wfd_uibc_capability", __FUNCTION__, szToken);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						nPathId = (WFD_InputPath)GetIndex(szInputPath, CountOfWFD_InputPath, g_szWFD_InputPath);

						if (CountOfWFD_InputPath == nPathId) {
							WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected token (%s) while parsing inp-path from detailed-cap in wfd_uibc_capability", __FUNCTION__, szToken);
							lRet = WFD_RTSP_INTERNAL_ERROR;
							goto cleanup;
						}

						buf.u.uibcCapability.usCountOfCapPairs++;
						buf.u.uibcCapability.pCapPairs = realloc(buf.u.uibcCapability.pCapPairs, sizeof(WFD_DETAILED_CAP_PAIR)*buf.u.uibcCapability.usCountOfCapPairs);
						buf.u.uibcCapability.pCapPairs[buf.u.uibcCapability.usCountOfCapPairs-1].eType = nTypeId;
						buf.u.uibcCapability.pCapPairs[buf.u.uibcCapability.usCountOfCapPairs-1].ePath = nPathId;
						ucSupported = 1;

						szToken = strtok_s(NULL, " ,;=", &pContext);
					}

					szToken = strtok_s(pTcpPort, " =", &pContext);
					if (szToken)
					{
						if (0 == strcmp(szToken, "none"))
						{
							buf.u.uibcCapability.usPort = 0;
						}
						else
						{
							buf.u.uibcCapability.usPort = atoi(szToken);
						}

					}
					else {
						WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received error trying to parse port for wfd_uibc_capability", __FUNCTION__);
						lRet = WFD_RTSP_INTERNAL_ERROR;
						goto cleanup;
					}
					
					// This will be true as long as we found at least 1
					// UIBC Generic or HIDC input capability
					buf.u.uibcCapability.ucSupported = ucSupported;
				}
			} else if (wfd_uibc_setting == buf.nId) {

				if (!strncmp(buf.szValue, "enable", 6))
					buf.u.uibcSetting.ucEnabled = 1;
				else if (!strncmp(buf.szValue, "disable", 6))
					buf.u.uibcSetting.ucEnabled = 0;
				else
				{
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error parsing wfd_uibc_setting", __FUNCTION__);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}

			} else if (wfd_av_format_change_timing == buf.nId) {
                int nContext=0;

                // Parse DTS
				if (!GetNextHex10(p, &buf.u.avTiming.ullDTS, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'DTS' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}
				buf.u.avTiming.ullDTS >>= 7;

                // Parse PTS
				if (!GetNextHex10(p, &buf.u.avTiming.ullPTS, buf.szValue, &nContext)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Error parsing 'PTS' on or after col %d in %s", __FUNCTION__, nContext+1, buf.szValue);
					lRet = WFD_RTSP_INTERNAL_ERROR;
					goto cleanup;
				}
				buf.u.avTiming.ullPTS >>= 7;
			}

			lRet = p->m_pNotify((WFD_RTSPNotifications)nNotifyMsg, &buf, p->m_pContext);

			// free memory
			if (wfd_audio_codecs == buf.nId) {
				if (buf.u.audioCodecs.aCodecs) {
					free(buf.u.audioCodecs.aCodecs);
					buf.u.audioCodecs.aCodecs = NULL;
				}
			} else if (wfd_video_formats == buf.nId || wfd_3d_video_formats == buf.nId) {
				if (buf.u.videoFormat.aProfiles) {
					free(buf.u.videoFormat.aProfiles);
					buf.u.videoFormat.aProfiles = NULL;
				}
			} else if (wfd_display_edid == buf.nId) {
				if (buf.u.displayEdid.pucEdid) {
					free(buf.u.displayEdid.pucEdid);
					buf.u.displayEdid.pucEdid = NULL;
				}
			} else if (wfd_uibc_capability == buf.nId) {
				if (buf.u.uibcCapability.pCapPairs)  {
					free(buf.u.uibcCapability.pCapPairs);
					buf.u.uibcCapability.pCapPairs = NULL;
				}
			}

			if (WFD_RTSP_NO_ERROR != lRet) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
				lRet = WFD_RTSP_INTERNAL_ERROR;
				goto cleanup;
			}
		}

		pToken = strtok_s(NULL,  "\r\n", &(p->m_pStrtokContext));
		if (pToken) WFD_RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - Next rds token = %s", __FUNCTION__, pToken);
	}

cleanup:
	// free memory
	if (wfd_audio_codecs == buf.nId) {
		if (buf.u.audioCodecs.aCodecs)
			free(buf.u.audioCodecs.aCodecs);
	} else if (wfd_video_formats == buf.nId || wfd_3d_video_formats == buf.nId) {
		if (buf.u.videoFormat.aProfiles)
			free(buf.u.videoFormat.aProfiles);
	} else if (wfd_display_edid == buf.nId) {
		if (buf.u.displayEdid.pucEdid)
			free(buf.u.displayEdid.pucEdid);
	} else if (wfd_uibc_capability == buf.nId) {
		if (buf.u.uibcCapability.pCapPairs)
			free(buf.u.uibcCapability.pCapPairs);
	}

	return lRet;
}

long WFD_DecodeMIME(CRTSPParser* p, unsigned char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen)
{
	int i,len=0;
	unsigned char in[4], out[3],v;
	static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
	long ipbufPtr = 0;
	long opbufPtr=0;

	memset(in,0,4);

	while (ipbufPtr < nSrcLen) { 

		for (len=0,i = 0; i < 4; i++) { 						
			v = 0;
			while( (ipbufPtr < nSrcLen) && v == 0 ) {           
				//v = (unsigned char) *((*encodedData) + ipbufPtr);
				v = pSrc[ipbufPtr];
				ipbufPtr++;
				v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if( v ) {
					v = (unsigned char) ((v == '$') ? 0 : v - 61);
				}
			}

			if( ipbufPtr <= nSrcLen ) {
				len++;
				if( v ) {
					in[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				in[i] = 0;
			}
		}

		if ( len ) {
			if (len < 3)
			{
				WFD_RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - ERROR! RTSP Source: MIME Decoder seeds 4 bytes of Encoded characters to decode, %d < 3", __FUNCTION__, len);
				return WFD_RTSP_INTERNAL_ERROR;
			}        	  
			out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
			out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
			out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);	    

			if (opbufPtr+3 > *pDstLen && pDst)
				return WFD_RTSP_INSUFFICIENT_BUF;

			if (pDst)
				memcpy(pDst+opbufPtr,out,3);
			opbufPtr+=3;
		}			
	}
	
	*pDstLen = opbufPtr;

	return WFD_RTSP_NO_ERROR;	
}

long WFD_EncodeMIME(CRTSPParser* p, unsigned char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen)
{
	static BOOL s_bInitilized=FALSE;
	static char s_cMimeTable[64];
	unsigned char igroup[3], ogroup[4];
	int eof = FALSE;
	int ch, charIndex;
	long lenInputFile;
	long ipbufPtr = 0;
	long opbufPtr=0;

	igroup[0] = igroup[1] = igroup[2] = 0;
	if (!s_bInitilized) {
		int i;
		for (i = 0; i < 9; i++) { 
			s_cMimeTable [i] = 'A' + i;
			s_cMimeTable [i + 9] = 'J' + i;
			s_cMimeTable [26 + i] = 'a' + i;	
			s_cMimeTable [26 + i + 9] = 'j' + i;
		}

		for (i = 0; i < 8; i++) { 
			s_cMimeTable [i + 18] = 'S' + i;
			s_cMimeTable [26 + i + 18] = 's' + i;
		}

		for (i = 0; i < 10; i++) { 
			s_cMimeTable [52 + i] = '0' + i;
		}

		s_cMimeTable [62] = '+';
		s_cMimeTable [63] = '/';

		s_bInitilized = TRUE;
	}

	WFD_RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - Length of the buffer to be encoded %ld", __FUNCTION__, nSrcLen);

	lenInputFile = nSrcLen;

	while (!eof ) {         
		igroup[0] = igroup[1] = igroup[2] = 0;
		for (charIndex = 0; charIndex < 3; charIndex++) {           
			if(ipbufPtr < lenInputFile)
			{
				ch =  *(pSrc + ipbufPtr);
				ipbufPtr++;
			}else {                 
				eof = TRUE;
				break;
			}
			igroup[charIndex] = (unsigned char) ch;
		}
		if (charIndex > 0) { 
			ogroup[0] = s_cMimeTable[igroup[0] >> 2];
			ogroup[1] = s_cMimeTable[((igroup[0] & 0x03) << 4) | (igroup[1] >> 4)];      
			ogroup[2] = s_cMimeTable[((igroup[1] & 0x0F) << 2) | (igroup[2] >> 6)];
			ogroup[3] = s_cMimeTable[igroup[2] & 0x3F]; //Replace characters in output stream with ??pad characters if fewer than three characters were read
			if (charIndex < 3) { 
				ogroup[3] = '=';
				if (charIndex < 2) { 
					ogroup[2] = '=';
				}
			}

			if (opbufPtr+5 > *pDstLen && pDst)
				return WFD_RTSP_INSUFFICIENT_BUF;

			if (pDst)
				memcpy(pDst+opbufPtr,ogroup,4);
			opbufPtr = opbufPtr + 4;
		}
	}     
	if (pDst)
		pDst[opbufPtr]='\0';

	// return the real length if actually decoding, or padd the requested len
	*pDstLen = pDst?opbufPtr:opbufPtr+4;

	WFD_RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - Encoded buffer len %ld", __FUNCTION__, opbufPtr);

	return WFD_RTSP_NO_ERROR;
}

