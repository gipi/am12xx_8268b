/**
* @file: intelrtspwfd.c
* @brief Intel extensions to the RTSP/WFD stack.
*
* INTEL CONFIDENTIAL
* Copyright 2012 Intel Corporation All Rights Reserved.
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
#include "intelrtspwfd.h"
#include <errno.h>

#define Declare_String
#include "intelrtspwfd_d.h"

#ifdef _POSIX_SOURCE
#define strncpy_s(dst, destsize, src, srcsize)	wfd_snprintf_(dst, destsize, "%s", src)
#endif

static void ParseVersion(INTELWFD_VERSION_STRUCT* pVer, char* szVer)
{
	char szNum[5];
	int i,j;

	// major
	for (i=j=0; j < 2 && szVer[i] >= '0' && szVer[i] <= '9'; i++,j++)
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

int GetId(char* szName, int nCount, char* strArr[])
{
	int i;
	int nLen=0;
	
	// if szName is null or it is a null pointer, immediately return value for not found
	if (!szName || !szName[0])
		return nCount;

	nLen = (int)strlen(szName);

	if (!strncmp(szName, "intel_", 4) && ':' == szName[nLen-1]) {
		// we want to ignore the colon if there is one
		szName[nLen-1] = 0;
	}

	for (i=0; i < nCount; i++)
		if (!strcmp(szName, strArr[i]))
			return i;

	return nCount;
}

long INTELWFD_SetParamStruct(void* hRtsp, INTELWFD_PARAMETER_STRUCT* pStruct, int nCount)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;
	long lRet = WFD_RTSP_NO_ERROR;
	int nLen=0;
	int i;

	p->m_nOutputBufferUsed = 0;

	for (i=0; i < nCount; i++) {
		if (pStruct[i].nId >= CountOfIntelWFDParams) {
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Invalid wfd_ command id (%d) in structure element[zero based %d of %d]", __FUNCTION__, pStruct[i].nId, i, nCount);
			return WFD_RTSP_INTERNAL_ERROR;
		} else {
			lRet = BuildPayloadFromStruct(p, &pStruct[i]);
			if (WFD_RTSP_NO_ERROR != lRet) {
				return lRet;
			}
		}
	}

	lRet = WFD_SendSetParameters(p, (char*)p->m_pOutputBuffer);
	if (WFD_RTSP_NO_ERROR != lRet) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Received unexpected error (%d) from WFD_SendGetParameters(%s)", __FUNCTION__, lRet, p->m_pOutputBuffer);
		return WFD_RTSP_INTERNAL_ERROR;
	}

	return lRet;
}


long ParseParameter(void* hRtsp, INTELWFD_PARAMETER_STRUCT* pIntel, WFD_PARAMETER_STRUCT* pWfd)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;
	char* pToken=NULL;

	memset(pIntel, 0, sizeof(INTELWFD_PARAMETER_STRUCT));
	pToken = pWfd->szValue;

	pIntel->nId = (IntelWFDParams)GetId(pWfd->szParam, CountOfIntelWFDParams, g_szIntelWFDParams);
	
	switch (pIntel->nId) {
	case intel_enable_widi_rtcp:
		if (pToken)
			pIntel->u.usRTCPPort = atoi(pToken);
		break;
	case intel_sink_version:
	{
		// parse adapter version info

		char* pTok=strstr(pToken, "product_ID=");
		if (pTok) {
			int i;
			for (i=0; i < INTELWFD_PRODUCT_ID_LEN; i++) {
				if (',' == *(pTok+11+i)) {
					pIntel->u.version.szProductId[i] = 0;
					break;
				}
				pIntel->u.version.szProductId[i] = *(pTok+11+i);
			}
		}

		pTok=strstr(pToken, "hw_version=");
		if (pTok) {
			ParseVersion(&pIntel->u.version.verHardware, pTok+11);
		}

		pTok=strstr(pToken, "sw_version=");
		if (pTok) {
			ParseVersion(&pIntel->u.version.verSoftware, pTok+11);
		}
		break;
	}
	case intel_friendly_name:
		if (pToken)
			strncpy_s(pIntel->u.szFriendlyName, sizeof(pIntel->u.szFriendlyName), pToken, sizeof(pIntel->u.szFriendlyName)-1);
		break;
	case intel_sink_manufacturer_name:
		if (pToken)
			strncpy_s(pIntel->u.szManufacturerName, sizeof(pIntel->u.szManufacturerName), pToken, sizeof(pIntel->u.szManufacturerName)-1);
		break;
	case intel_sink_model_name:
		if (pToken)
			strncpy_s(pIntel->u.szModelName, sizeof(pIntel->u.szModelName), pToken, sizeof(pIntel->u.szModelName)-1);
		break;
	case intel_sink_device_URL:
		if (pToken)
			strncpy_s(pIntel->u.szDeviceURL, sizeof(pIntel->u.szDeviceURL), pToken, sizeof(pIntel->u.szDeviceURL)-1);
		break;
	case intel_sink_manufacturer_logo:
		if (pToken) {
			long lenInput=strlen(pToken);
			long lenOutput=0;

			if (WFD_RTSP_NO_ERROR != WFD_RTSPDecodeMIME(p, NULL, &lenOutput, pToken, lenInput)) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed decoding %d byte buffer", __FUNCTION__, pToken, lenInput);
				return WFD_RTSP_INTERNAL_ERROR;
			}

			pIntel->u.manufacturerLogo.nLen = lenOutput;
			pIntel->u.manufacturerLogo.pucPng = (unsigned char*)malloc(lenOutput+1);

			if (WFD_RTSP_NO_ERROR != WFD_RTSPDecodeMIME(p, pIntel->u.manufacturerLogo.pucPng, &lenOutput, pToken, lenInput)) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed decoding %d byte png file", __FUNCTION__, pToken, lenInput);
				return WFD_RTSP_INTERNAL_ERROR;
			}
		}
		break;
	case intel_lower_bandwidth:
		pIntel->u.nLowerBandwidth = 0; // defatult to zero
		if (pToken)
			pIntel->u.nLowerBandwidth = atoi(pToken);
		break;
	case intel_sigma_pipeline_params:
		if (pToken) {
			if (3 != sscanf(pToken, "PlaybackDelay=%d; PositiveMaxStcPCR=%d; NegativeMaxStcPCR=%d;", &pIntel->u.sigmaLatencyParams.nPlaybackDelay, &pIntel->u.sigmaLatencyParams.nPositiveMaxStcPCR, &pIntel->u.sigmaLatencyParams.nNegativeMaxStcPCR)) {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed rds_overscan_comp: parameters '%s', expected 'PlaybackDelay=#; PositiveMaxStcPCR=#; NegativeMaxStcPCR=#;'", pToken);
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INVALID_WFD_PARAMS;
			}
		}
		break;
	case intel_overscan_comp:
		if (pToken) {
			int nX, nY;

			if (2 != sscanf(pToken, "x=%d, y=%d", &nX, &nY)) {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed rds_overscan_comp: parameters '%s', expected 'x=#, y=#'", pToken);
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INVALID_WFD_PARAMS;
			}
			pIntel->u.overscanComp.nX = INTELWFD_MAX_OVERSCAN_COMP-nX;
			pIntel->u.overscanComp.nY = INTELWFD_MAX_OVERSCAN_COMP-nY;
		}
		break;
	case intel_usboip:
		if (pToken) {
			char* pContext;
			char* pTok = strtok_s(pToken, " ", &pContext);

			memset(&pIntel->u.usboip, 0, sizeof(pIntel->u.usboip));

			if (!pTok) {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed intel_usboip: parameters '%s', expected none or type", pTok?pTok:"");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INVALID_WFD_PARAMS;
			}

			if (!strcmp(pTok, "none")) {
				break;
			}

			pIntel->u.usboip.ucSupported = 1;

			// get version=float
			if (strncmp(pTok, "version=", 8)) {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed intel_usboip: parameters '%s', expected version=floating point number", pTok?pTok:"");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INVALID_WFD_PARAMS;
			}

			if ('0' > pTok[8] || pTok[8] > '9') {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed intel_usboip: parameters '%s', expecting version number", pTok?pTok:"");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INTERNAL_ERROR;
			}
			pIntel->u.usboip.fVersion = atof(pTok+8);

			pTok = strtok_s(NULL, " ", &pContext);

			// get host/client=IPADDRESS
			if (!pTok) {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed intel_usboip: expecting host=/client=, no more tokens found");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INVALID_WFD_PARAMS;
			}

			if (!strncmp(pTok, "host=", 5) || !pTok[5])
				pIntel->u.usboip.inaddrHost.s_addr = inet_addr(pTok+5);
			else if (!strncmp(pTok, "client=", 7) || !pTok[7])
				pIntel->u.usboip.inaddrClient.s_addr = inet_addr(pTok+7);
			else {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed intel_usboip: parameters '%s', expecting host=/client=", pTok?pTok:"");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INTERNAL_ERROR;
			}

			// get next token
			pTok = strtok_s(NULL, " ", &pContext);

			if (!pTok) {
				WFD_RTSPSetResponseCodeAndMessage(p, 400, "Malformed intel_usboip: expecting active=, no more tokens found");
				WFD_RTSP_LOG(p, RTSP_LOG_ERR,  "%s - %s", __FUNCTION__, p->m_szErrorMessage);

				return WFD_RTSP_INVALID_WFD_PARAMS;
			}

			if (!strncmp(pTok, "active=", 7) || !pTok[7])
				pIntel->u.usboip.ucActive = ('0'==pTok[7])?0:1;

			if (!pTok)
				break;

			// get next token
			pTok = strtok_s(NULL, " ", &pContext);

			if (!strncmp(pTok, "discovery_port=", 15) || !pTok[15])
				pIntel->u.usboip.usDiscoveryPort = atoi(pTok+15);

		}
		break;
	default:
		return WFD_RTSP_INVALID_WFD_PARAMS;
	}

	return WFD_RTSP_NO_ERROR;
}

long BuildPayloadFromStruct(void* hRtsp, INTELWFD_PARAMETER_STRUCT* pBuf)
{
	CRTSPParser* p = (CRTSPParser*)hRtsp;
	long nAdditionalLen=0;
	int nLen=0;
	int i;

	if (CountOfIntelWFDParams <= pBuf->nId) {
		WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - %d not supported", __FUNCTION__, pBuf->nId);
		return WFD_RTSP_INVALID_WFD_PARAMS;
	} else {
		switch (pBuf->nId) {
		case intel_enable_widi_rtcp:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + 6; // strlen("max short") + 1

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s:)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %d\r\n", g_szIntelWFDParams[pBuf->nId], (int)pBuf->u.usRTCPPort);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_sink_version:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + 81; // product_ID=xxxxxxxxxxxxxxxx, hw_version=00.00.00.0000, sw_version=00.00.00.0000\r\n

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: product_ID=xxxxxxxxxxxxxxxx, hw_version=00.00.00.0000, sw_version=00.00.00.0000)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: product_ID=%s, hw_version=%d.%d.%d.%d, "
				"sw_version=%d.%d.%d.%d\r\n", 
				g_szIntelWFDParams[pBuf->nId], 
				pBuf->u.version.szProductId,
				(int)pBuf->u.version.verHardware.ucMajor, (int)pBuf->u.version.verHardware.ucMinor, 
				(int)pBuf->u.version.verHardware.ucSku, (int)pBuf->u.version.verHardware.usBuild,
				(int)pBuf->u.version.verSoftware.ucMajor, (int)pBuf->u.version.verSoftware.ucMinor, 
				(int)pBuf->u.version.verSoftware.ucSku, (int)pBuf->u.version.verSoftware.usBuild);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_friendly_name:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + INTELWFD_FRIENDLY_NAME_LEN;

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId], pBuf->u.szFriendlyName);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %s\r\n", g_szIntelWFDParams[pBuf->nId], pBuf->u.szFriendlyName);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_sink_manufacturer_name:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + INTELWFD_MANUFACTURER_NAME_LEN;

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId], pBuf->u.szManufacturerName);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %s\r\n", g_szIntelWFDParams[pBuf->nId], pBuf->u.szManufacturerName);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_sink_model_name:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + INTELWFD_MODEL_NAME_LEN;

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId], pBuf->u.szModelName);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %s\r\n", g_szIntelWFDParams[pBuf->nId], pBuf->u.szModelName);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_sink_device_URL:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + INTELWFD_DEVICE_URL_LEN;

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: %s)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId], pBuf->u.szDeviceURL);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %s\r\n", g_szIntelWFDParams[pBuf->nId], pBuf->u.szDeviceURL);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_sink_manufacturer_logo:

			if (0 == pBuf->u.manufacturerLogo.nLen) {
				nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + 5; // none\r\n

				if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: none)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
					return WFD_RTSP_INSUFFICIENT_BUF;
				}

				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: none\r\n", g_szIntelWFDParams[pBuf->nId]);
			} else {

				// get length first
				long len;

				if (WFD_RTSP_NO_ERROR != WFD_RTSPEncodeMIME(p, NULL, &len, pBuf->u.manufacturerLogo.pucPng, pBuf->u.manufacturerLogo.nLen)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed encoding %d byte png file", __FUNCTION__, g_szIntelWFDParams[pBuf->nId], pBuf->u.manufacturerLogo.nLen);
					return WFD_RTSP_INTERNAL_ERROR;
				}

				nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + len + 1;

				if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s: product_ID=xxxxxxxxxxxxxxxx, hw_version=00.00.00.0000, sw_version=00.00.00.0000)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
					return WFD_RTSP_INSUFFICIENT_BUF;
				}

				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: ", g_szIntelWFDParams[pBuf->nId]);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

				if (WFD_RTSP_NO_ERROR != WFD_RTSPEncodeMIME(p, (char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), &len, pBuf->u.manufacturerLogo.pucPng, pBuf->u.manufacturerLogo.nLen)) {
					WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Failed encoding %d byte png file", __FUNCTION__, g_szIntelWFDParams[pBuf->nId], pBuf->u.manufacturerLogo.nLen);
					return WFD_RTSP_INTERNAL_ERROR;
				}

			}

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_lower_bandwidth:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + 2; // 0|1\r\n

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s:)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: %d\r\n", g_szIntelWFDParams[pBuf->nId], pBuf->u.nLowerBandwidth?1:0);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_sigma_pipeline_params:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + 70; // PlaybackDelay=%d; PositiveMaxStcPCR=%d; NegativeMaxStcPCR=%d;\r\n

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s:)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: PlaybackDelay=%d; PositiveMaxStcPCR=%d; NegativeMaxStcPCR=%d;\r\n", g_szIntelWFDParams[pBuf->nId]
				, pBuf->u.sigmaLatencyParams.nPlaybackDelay
				, pBuf->u.sigmaLatencyParams.nPositiveMaxStcPCR
				, pBuf->u.sigmaLatencyParams.nNegativeMaxStcPCR
				);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_overscan_comp:

			nAdditionalLen = (long)(strlen(g_szIntelWFDParams[pBuf->nId]))+2 + 11; // x=(0..15), y=(0..15)\r\n

			if (RTSP_MAX_OUTPUT_BUFFER < p->m_nOutputBufferUsed + nAdditionalLen) {
				WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - Insufficient buffer to add (%s:)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
				return WFD_RTSP_INSUFFICIENT_BUF;
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"%s: x=%d, y=%d\r\n", g_szIntelWFDParams[pBuf->nId]
				, INTELWFD_MAX_OVERSCAN_COMP-pBuf->u.overscanComp.nX
				, INTELWFD_MAX_OVERSCAN_COMP-pBuf->u.overscanComp.nY
				);

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;
			break;

		case intel_usboip:

			if (!pBuf->u.usboip.ucSupported) {
				// not supported
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: none", g_szIntelWFDParams[pBuf->nId]
					);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;
			} else {
				// supported
				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					"%s: version=%3.1f", g_szIntelWFDParams[pBuf->nId], pBuf->u.usboip.fVersion
					);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

				if (pBuf->u.usboip.inaddrClient.s_addr) {
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						" client=%s", inet_ntoa(pBuf->u.usboip.inaddrClient)
						);
				} else if (pBuf->u.usboip.inaddrHost.s_addr) {
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						" host=%s", inet_ntoa(pBuf->u.usboip.inaddrHost)
						);
				}

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

				nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
					" active=%d", pBuf->u.usboip.ucActive
					);

				if (0 < nLen)
					p->m_nOutputBufferUsed += nLen;

				if (pBuf->u.usboip.ucActive) {
					nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
						" discovery_port=%d", pBuf->u.usboip.usDiscoveryPort
						);

					if (0 < nLen)
						p->m_nOutputBufferUsed += nLen;
				}
			}

			nLen = wfd_snprintf_((char *)(p->m_pOutputBuffer+p->m_nOutputBufferUsed), RTSP_MAX_OUTPUT_BUFFER-p->m_nOutputBufferUsed,
				"\r\n");

			if (0 < nLen)
				p->m_nOutputBufferUsed += nLen;

			break;
		default:
			WFD_RTSP_LOG(p, RTSP_LOG_ERR, "%s - unhandled (%s:)", __FUNCTION__, g_szIntelWFDParams[pBuf->nId]);
			return WFD_RTSP_INVALID_METHOD;
		}
	}

	return WFD_RTSP_NO_ERROR;
}

