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
#include "rdsparser.h"

#define Declare_String
#include "rdsrtsp_d.h"

void ParseVersion(VERSION_STRUCT* pVer, char* szVer)
{
	char szNum[5];
	int i,j;

	// major
	for (i=j=0; j < 2 && szVer[i] > '0' && szVer[i] < '9'; i++,j++)
		szNum[j] = szVer[i];
	szNum[j] = 0;
	pVer->ucMajor = atoi(szNum);

	i++; // inc past '.'

	// minor
	for (j=0; j < 2 && szVer[i] != '.'; i++,j++)
		szNum[j] = szVer[i];
	szNum[j] = 0;
	pVer->ucMinor = atoi(szNum);

	i++; // inc past '.'

	// sku
	for (j=0; j < 2 && szVer[i] != '.'; i++,j++)
		szNum[j] = szVer[i];
	szNum[j] = 0;
	pVer->ucSku = atoi(szNum);

	i++; // inc past '.'

	// build
	for (j=0; j < 4 && szVer[i] != '.'; i++,j++)
		szNum[j] = szVer[i];
	szNum[j] = 0;
	pVer->usBuild = atoi(szNum);
}

int GetIndex(char* szName, int nCount, char* strArr[])
{
	int i;
	int nLen = (int)strlen(szName);

	if (!strncmp(szName, "rds_", 4) && ':' == szName[nLen-1]) {
		// we want to ignore the colon if there is one
		szName[nLen-1] = 0;
	}

	for (i=0; i < nCount; i++)
		if (!strcmp(szName, strArr[i]))
			return i;

	return nCount;
}

long RDSParser(CRTSPParser* p)
{
	long lRet = RTSP_NO_ERROR;

	if (MSET_PARAMETER == -p->pParsed->nMethod) {

		RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - SET_PARAMETER (response): %s", __FUNCTION__, p->pParsed->aBuffer);

	} else if (MGET_PARAMETER == -p->pParsed->nMethod) {

		if (200 > p->pParsed->nUsed) {
			RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - GET_PARAMETER (response): %s", __FUNCTION__, p->pParsed->aBuffer);
		} else {
			RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - GET_PARAMETER (response): %d byte buffer", __FUNCTION__, p->pParsed->nUsed);
		}

		lRet = RDSParseParametersAndNotify(p, RTSP_GET_PARAMETER_RESP);
		if (RTSP_NO_ERROR != lRet) {
			return RTSP_INTERNAL_ERROR;
		}

	} else if (MSET_PARAMETER == p->pParsed->nMethod) {

		// RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - SET_PARAMETER: %s", __FUNCTION__, p->pParsed->aBuffer);

		lRet = RDSParseParametersAndNotify(p, RTSP_SET_PARAMETER);
		if (RTSP_NO_ERROR != lRet) {
			return lRet;
		}

		if (MSETUP == p->m_Trigger) {

			lRet = SendSetupTriggerResponse(p, p->pParsed->nCSeq);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendSetupTriggerResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return RTSP_INTERNAL_ERROR;
			}

			lRet = SendSetup(p);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendSetup(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return RTSP_INTERNAL_ERROR;
			}

		} else if (MTEARDOWN == p->m_Trigger) {
			lRet = SendTeardownTriggerResponse(p, p->pParsed->nCSeq);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendSetupTriggerResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return RTSP_INTERNAL_ERROR;
			}

			lRet = SendTeardown(p);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendSetup(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return RTSP_INTERNAL_ERROR;
			}
		} else {
			lRet = SendSetParametersResponse(p, p->pParsed->nCSeq);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq);
				return RTSP_INTERNAL_ERROR;
			}
		}

	} else if (MGET_PARAMETER == p->pParsed->nMethod) {
		// See what parameters we need and then respond with them
		char* pToken=strtok_s((char*)p->pParsed->aBuffer, ", ", &(p->m_pStrtokContext));
		long nLen=0;
		long nAdditionalLen=0;

		p->m_nOutputBufferUsed = 0;
		//RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - GET_PARAMETER: %s", __FUNCTION__, p->pParsed->aBuffer);

		while (pToken) {
			PARAMETER_STRUCT buf;

			memset(&buf, 0, sizeof(buf));

			buf.nId = (RDSParams)GetIndex(pToken, CountOfRDSParams, g_szRDSParams);
			buf.szValue = buf.szStaticBuffer;
			buf.nCount = RDS_STATIC_LEN;

			if (rds_audio_formats == buf.nId
				|| rds_video_formats == buf.nId
				|| rds_rtp_profile == buf.nId
				|| rds_rtp_profile_setting == buf.nId
				|| rds_session_timeout == buf.nId)
			{
				nAdditionalLen = (long)(strlen(pToken))+2 + 13; // depracated\r\n

				if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, buf.szValue);
					return RTSP_INSUFFICIENT_BUF;
				}

				nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: deprecated\r\n", pToken);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;
			} else if (RTSP_INSUFFICIENT_BUF == p->m_pNotify(RTSP_GET_PARAMETER, &buf, p->m_pContext)) {
				// first get the required size

				// we receive the value as a string
				buf.nCount++;
				buf.szValue = calloc(1, buf.nCount);
				lRet = p->m_pNotify(RTSP_GET_PARAMETER, &buf, p->m_pContext);
				if (RTSP_NO_ERROR != lRet) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
					if (buf.szValue != buf.szStaticBuffer)
						free(buf.szValue);
					return RTSP_INTERNAL_ERROR;
				}

				nAdditionalLen = (long)(strlen(pToken))+2 + (long)(strlen(buf.szValue))+2;

				if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %c%c%c%c%c...)", __FUNCTION__, pToken, 
						buf.szValue[0], buf.szValue[1], buf.szValue[2], buf.szValue[3], buf.szValue[4]);
					return RTSP_INSUFFICIENT_BUF;
				}

				nLen = _snprintf_((char*)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: %s\r\n", pToken, buf.szValue);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

				if (buf.szValue != buf.szStaticBuffer)
					free(buf.szValue);

			} else {
				if (rds_rtp_profile == buf.nId || rds_rtp_profile_setting == buf.nId) {

					if (CountOfRDSProfiles <= buf.u.profile) {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected profile=%d, max=%d", __FUNCTION__, buf.nId, CountOfRDSProfiles);
						return RTSP_INTERNAL_ERROR;
					}

					nAdditionalLen = (long)(strlen(pToken))+2 + (long)(strlen(g_szRDSProfiles[buf.u.profile]))+2;

					if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, buf.szValue);
						return RTSP_INSUFFICIENT_BUF;
					}

					nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: %s\r\n", pToken, g_szRDSProfiles[buf.u.profile]);

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;

				} else if (rds_status == buf.nId) {

					nAdditionalLen = (long)(strlen(pToken))+2 + 29; // busy=1, display_connected=1\r\n

					if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, buf.szValue);
						return RTSP_INSUFFICIENT_BUF;
					}

					nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: busy=%d, display_connected=%d\r\n", pToken, buf.u.status.nBusy, buf.u.status.nDisplayConnected);

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;

				} else if (rds_content_protection == buf.nId) {

					nAdditionalLen = (long)(strlen(pToken))+2 + 21; // HDCPX.Y port=65536\r\n

					if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, buf.szValue);
						return RTSP_INSUFFICIENT_BUF;
					}

					if (buf.u.contentProtection.usPort)
						nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
							"%s: HDCP%.1f port=%d\r\n", pToken, buf.u.contentProtection.fHDCPVersion, buf.u.contentProtection.usPort);
					else
						nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
							"%s: none\r\n", pToken);

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;

				} else if (rds_sink_version == buf.nId) {

					nAdditionalLen = (long)(strlen(pToken))+2 + 81; // product_ID=xxxxxxxxxxxxxxxx, hw_version=00.00.00.0000, sw_version=00.00.00.0000\r\n

					if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, buf.szValue);
						return RTSP_INSUFFICIENT_BUF;
					}

					nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: product_ID=%s, hw_version=%d.%d.%d.%d, "
						"sw_version=%d.%d.%d.%d\r\n", 
						pToken, 
						buf.u.version.szProductId,
						buf.u.version.verHardware.ucMajor, buf.u.version.verHardware.ucMinor, 
						buf.u.version.verHardware.ucSku, buf.u.version.verHardware.usBuild,
						buf.u.version.verSoftware.ucMajor, buf.u.version.verSoftware.ucMinor, 
						buf.u.version.verSoftware.ucSku, buf.u.version.verSoftware.usBuild);

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;

				} else {

					if (p->m_bLegacy && rds_display_edid == buf.nId && !strcmp(buf.szValue, "none")) {
						// LEGACY: if edid is none, we need to send some encoded stuff
						strcpy(buf.szValue, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=");
					}

					nAdditionalLen = (long)(strlen(pToken))+2 + (long)(strlen(buf.szValue))+2;

					if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
						RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, pToken, buf.szValue);
						return RTSP_INSUFFICIENT_BUF;
					}

					nLen = _snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						"%s: %s\r\n", pToken, buf.szValue);

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;

					if (buf.szValue != buf.szStaticBuffer)
						free(buf.szValue);

				}
			}

			pToken = strtok_s(NULL, ", ", &(p->m_pStrtokContext));
		}

		lRet = SendGetParametersResponse(p, p->pParsed->nCSeq,
			(char*)p->m_pOutputBuffer);
		if (RTSP_NO_ERROR != lRet) {
			RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from SendGetParametersResponse(%d, %s)", __FUNCTION__, lRet, p->pParsed->nCSeq, p->m_pOutputBuffer);
			return RTSP_INTERNAL_ERROR;
		}

	}

	return 0;
}

long RDSParseParametersAndNotify(CRTSPParser* p, int nNotifyMsg)
{
	long lRet = RTSP_NO_ERROR;
	char* pTok=NULL;
	int i=0,j=0,k=0;
	BOOL bStatus=FALSE;
	int nBusy=0;
	int nDisplayConnected=0;
	PARAMETER_STRUCT buf;
	BOOL bEdid=FALSE;
	BOOL bHDCP=FALSE;

	// Parse response from our request for paraemters
	char* pToken=strtok_s((char*)p->pParsed->aBuffer, ", ", &(p->m_pStrtokContext));

	// this value means no trigger
	p->m_Trigger = CountOfRDSRTSPNotifications;

	while (pToken) {
		memset(&buf, 0, sizeof(buf));
		buf.nId = (RDSParams)GetIndex(pToken, CountOfRDSParams, g_szRDSParams);
		if (CountOfRDSParams == buf.nId) {
			RTSP_LOG(p, RTSP_LOG_ERR, "%s - Unrecognized value (%s) for RDS Param", __FUNCTION__, pToken?pToken:"NULL");
			buf.szParam = pToken;
			buf.szValue = strtok_s(NULL,  "", &(p->m_pStrtokContext));

			lRet = p->m_pNotify((RDSRTSPNotifications)nNotifyMsg, &buf, p->m_pContext);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
				return RTSP_INTERNAL_ERROR;
			}

			return RTSP_NO_ERROR;
		}

		buf.szValue = strtok_s(NULL,  "\r\n", &(p->m_pStrtokContext));
		buf.nCount = (int)(buf.szValue?strlen(buf.szValue):0);

		if (rds_sigma_pipeline_params == buf.nId) {
			if (RTSP_SET_PARAMETER == nNotifyMsg) {
				char* pContext=NULL;
				char* szTmp=NULL;
				char* pTok2=NULL;
				char* pTok3=NULL;

				szTmp = strdup(buf.szValue);
				if (!szTmp) {
					RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed allocating (%d) bytes", __FUNCTION__, buf.nCount+1);
					return RTSP_MEMORY;
				}

				pTok2 = strtok_s(szTmp, "= ", &pContext);
				while (pTok2) {
					if (!strcmp(pTok2, g_szRDSSigmaPipelineParams[PlaybackDelay])) {
						// PlaybackDelay
						pTok3 = strtok_s(NULL, "; ", &pContext);
						if (pTok3) buf.u.sigmaPipeline.nPlaybackDelay = atoi(pTok3);
					} else if (!strcmp(pTok2, g_szRDSSigmaPipelineParams[PositiveMaxStcPCR])) {
						// PositiveMaxStcPCR
						pTok3 = strtok_s(NULL, "; ", &pContext);
						if (pTok3) buf.u.sigmaPipeline.nPositiveMaxStcPCR = atoi(pTok3);
					} else if (!strcmp(pTok2, g_szRDSSigmaPipelineParams[NegativeMaxStcPCR])) {
						// NegativeMaxStcPCR
						pTok3 = strtok_s(NULL, "; ", &pContext);
						if (pTok3) buf.u.sigmaPipeline.nNegativeMaxStcPCR = atoi(pTok3);
					}

					if (pTok3)
						pTok2 = strtok_s(NULL, "= ", &pContext);
				}

				free(szTmp);
			}


		} else if (rds_rtp_profile == buf.nId || rds_rtp_profile_setting == buf.nId) {
			buf.u.profile = (RDSProfiles)GetIndex(buf.szValue, CountOfRDSProfiles, g_szRDSProfiles);

		} else if (rds_keepalive == buf.nId) {

			if (!p->m_bServer && RTSP_SET_PARAMETER == nNotifyMsg)
				p->m_nLastKeepalive = p->m_pGetTick(p->m_pContext);

		} else if (rds_display_edid == buf.nId) {

			// got EDID
			bEdid = TRUE;

			if (p->m_bLegacy) {
				// LEGACY: when no HDMI is connected, we are getting AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA instead of none
				if (256 > buf.nCount && !strncmp("AAAAAAAAAAAAAAAAAAAAAAAAA", buf.szValue, 25)) {
					// replace with "none"
					strcpy(buf.szValue, "none");
					buf.nCount = 4;
				}
			}

		} else if (rds_overscan_comp == buf.nId) {

			buf.u.overscanComp.nX = buf.u.overscanComp.nY = 0;

			if (2 != sscanf(buf.szValue, "x=%d, y=%d", &buf.u.overscanComp.nX, &buf.u.overscanComp.nY)) {
				p->m_nRTSPReponseCode = 400;
				_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Malformed rds_overscan_comp: parameters '%s', expected 'x=#, y=#'", buf.szValue);
				RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return RTSP_INVALID_RDS_PARAMS;
			}
		} else if (rds_session_timeout == buf.nId) {

			//if (!p->m_bServer && RTSP_SET_PARAMETER == nNotifyMsg && buf.szValue)
			//	p->m_nKeepalivePeriod = 1000*atoi(buf.szValue);

		} else if (rds_trigger_method == buf.nId) {

			p->m_Trigger = (MethodTokens)GetIndex(buf.szValue, CountOfMethodTokens, g_szMethodTokens);
		
		} else if (rds_presentation_URL == buf.nId) {
			if (RTSP_SET_PARAMETER == nNotifyMsg) {
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

		} else if (rds_friendly_name == buf.nId) {
			strncpy(p->m_szFriendlyName, buf.szValue, RDS_FRIENDLY_NAME_LEN+1);

		} else if (rds_sink_model_name == buf.nId) {
			strncpy(p->m_szModelName, buf.szValue, RDS_MODEL_NAME_LEN+1);

		} else if (rds_sink_manufacturer_name == buf.nId) {
			strncpy(p->m_szManufacturerName, buf.szValue, RDS_MANUFACTURER_NAME_LEN+1);

		} else if (rds_sink_device_URL == buf.nId) {
			strncpy(p->m_szManufacturerUrl, buf.szValue, RDS_MANUFACTURER_URL_LEN+1);

		} else if (rds_sink_version == buf.nId) {
			// parse adapter version info
			pTok=strstr(buf.szValue, "product_ID=");
			if (pTok) {
				int i;
				for (i=0; i < RDS_PRODUCT_ID_LEN; i++) {
					p->m_szProductId[i] = *(pTok+11+i);
					if (',' == p->m_szProductId[i])
						break;
				}
				p->m_szProductId[i] = 0;
				strncpy(buf.u.version.szProductId, p->m_szProductId, RDS_PRODUCT_ID_LEN+1);
			}

			pTok=strstr(buf.szValue, "hw_version=");
			if (pTok) {
				ParseVersion(&p->m_verHardware, pTok+11);
				buf.u.version.verHardware = p->m_verHardware;
			}

			pTok=strstr(buf.szValue, "sw_version=");
			if (pTok) {
				ParseVersion(&p->m_verSoftware, pTok+11);
				buf.u.version.verSoftware = p->m_verSoftware;
			}

		} else if (rds_status == buf.nId) {
			nBusy = nDisplayConnected = 0;

			if (buf.szValue && 2 == sscanf(buf.szValue, "busy=%d, display_connected=%d", &nBusy, &nDisplayConnected)) {
				if (p->m_bRtspConnected)
					bStatus = TRUE;
			} else {
				p->m_nRTSPReponseCode = 400;
				_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Malformed rds_status: parameters '%s', expected 'busy=#, display_connected=#'", buf.szValue?buf.szValue:"");
				RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return RTSP_INVALID_RDS_PARAMS;
			}
		} else if (rds_content_protection == buf.nId) {
			buf.u.contentProtection.fHDCPVersion = 0;
			buf.u.contentProtection.usPort = 0;

			// got HDCP message
			bHDCP = TRUE;

			if (!strncmp(buf.szValue, "HDCP", 4)) {
				buf.u.contentProtection.fHDCPVersion = (float)atof(buf.szValue+4);
				pTok=strstr(buf.szValue, "port=");

				if (pTok)
					buf.u.contentProtection.usPort = atoi(pTok+5);
			}
		}

		if (!(bStatus && rds_status == buf.nId)) {
			// don't give status callback because we want to make sure that the rest of the stuff like 
			// rds_display_edid and rds_content_protection get parsed before the app handles the status change.
			// If the status indicates an HDMI hot plug/unplug event, the spec will require the rds_display_edid 
			// and rds_content_protection to be sent in the same message

			lRet = p->m_pNotify((RDSRTSPNotifications)nNotifyMsg, &buf, p->m_pContext);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
				return RTSP_INTERNAL_ERROR;
			}
		}

skip_to_next:
		pToken = strtok_s(NULL,  " \r\n", &(p->m_pStrtokContext));
		if (pToken) RTSP_LOG(p, RTSP_LOG_NOTICE, "%s - Next rds token = %s", __FUNCTION__, pToken);
	}

	if (bStatus) {
		memset(&buf, 0, sizeof(buf));
		buf.nId = rds_status;
		buf.u.status.nBusy = nBusy;
		buf.u.status.nDisplayConnected = nDisplayConnected;

		if (!bEdid || !bHDCP) {
			p->m_nRTSPReponseCode = 400;
			_snprintf_(p->m_szErrorMessage, RTSP_MAX_ERROR_MSG_LEN, "Received rds_status but didn't get one or both of rds_display_edid, rds_content_protection");
			RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

			return RTSP_INVALID_RDS_PARAMS;
		} else {
			lRet = p->m_pNotify((RDSRTSPNotifications)nNotifyMsg, &buf, p->m_pContext);
			if (RTSP_NO_ERROR != lRet) {
				RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from client notify callback", __FUNCTION__, lRet);
				return RTSP_INTERNAL_ERROR;
			}
		}
	}

	return RTSP_NO_ERROR;
}

long DecodeMIME(CRTSPParser* p, unsigned char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen)
{
	int i,len=0;
	unsigned char in[4], out[3],v;
	static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";
	long ipbufPtr = 0;
	long opbufPtr=0;

	memset(in,'\0',4); //KW fix

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
				RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - ERROR! RTSP Source: MIME Decoder seeds 4 bytes of Encoded characters to decode, %d < 3", __FUNCTION__, len);
				return RTSP_INTERNAL_ERROR;
			}        	  
			out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
			out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
			out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);	    

			if (opbufPtr+3 > *pDstLen && pDst)
				return RTSP_INSUFFICIENT_BUF;

			if (pDst)
				memcpy(pDst+opbufPtr,out,3);
			opbufPtr+=3;
		}			
	}
	
	*pDstLen = opbufPtr;

	return RTSP_NO_ERROR;	
}

long EncodeMIME(CRTSPParser* p, unsigned char* pDst, int* pDstLen, unsigned char* pSrc, int nSrcLen)
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

	RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - Length of the buffer to be encoded %ld", __FUNCTION__, nSrcLen);

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
			ogroup[3] = s_cMimeTable[igroup[2] & 0x3F]; //Replace characters in output stream with ”=” pad characters if fewer than three characters were read
			if (charIndex < 3) { 
				ogroup[3] = '=';
				if (charIndex < 2) { 
					ogroup[2] = '=';
				}
			}

			if (opbufPtr+5 > *pDstLen && pDst)
				return RTSP_INSUFFICIENT_BUF;

			if (pDst)
				memcpy(pDst+opbufPtr,ogroup,4);
			opbufPtr = opbufPtr + 4;
		}
	}     
	if (pDst)
		pDst[opbufPtr]='\0';

	// return the real length if actually decoding, or padd the requested len
	*pDstLen = pDst?opbufPtr:opbufPtr+4;

	RTSP_LOG(p, RTSP_LOG_DEBUG, "%s - Encoded buffer len %ld", __FUNCTION__, opbufPtr);

	return RTSP_NO_ERROR;
}

