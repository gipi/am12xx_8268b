#ifndef __INTELRTSPWFD_H__
#define __INTELRTSPWFD_H__

#include "rtspwfd.h"

#ifdef WIN32
#include "winsock2.h"
#endif

#ifdef _POSIX_SOURCE
#include <arpa/inet.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "intelrtspwfd_d.h"

#define Declare_Extern
#include "intelrtspwfd_d.h"

#ifdef _WIN32
#define CALLING_CONVENTION              __stdcall
#define uint64_t unsigned __int64
#define int64_t __int64
#define strtoull _strtoui64
#elif __GNUC__
#define CALLING_CONVENTION
#include <stdint.h>
#else
#error Please define CALLING_CONVENTION for this compiler
#endif

#define INTELWFD_FRIENDLY_NAME_LEN		18
#define INTELWFD_MODEL_NAME_LEN			64
#define INTELWFD_MANUFACTURER_NAME_LEN	32
#define INTELWFD_DEVICE_URL_LEN			256
#define INTELWFD_PRODUCT_ID_LEN			16

#define INTELWFD_MAX_OVERSCAN_COMP		15

#define INTELWFD_STATIC_LEN				512

#ifdef WIN32
#pragma pack(1)
#endif
typedef struct {
	unsigned char ucMajor;
	unsigned char ucMinor;
	unsigned char ucSku;
	unsigned char usBuild;
} INTELWFD_VERSION_STRUCT;

typedef struct { // intel_sink_version
	INTELWFD_VERSION_STRUCT verHardware;
	INTELWFD_VERSION_STRUCT verSoftware;
	char szProductId[INTELWFD_PRODUCT_ID_LEN+1];
} INTELWFD_VERSION;

typedef struct { // intel_sigma_pipeline_params
	int nPlaybackDelay;
	int nPositiveMaxStcPCR;
	int nNegativeMaxStcPCR;
} INTELWFD_SIGMA_PARAMS;

typedef struct { // intel_overscan_comp
	int nX;
	int nY;
} INTELWFD_OVERSCAN_COMP;

typedef struct { // intel_sink_manufacturer_logo
	int nLen;
	unsigned char* pucPng;
} INTELWFD_MANUFACTURER_LOGO;

typedef struct { // intel_usboip
	unsigned char ucSupported;
	unsigned char ucActive;
	float fVersion;
	struct in_addr inaddrClient;
	struct in_addr inaddrHost;
	unsigned short usDiscoveryPort;
} INTELWFD_USBOIP;

typedef struct {
	IntelWFDParams nId; // rds id which will determine which field in the union is applicable

	char szStaticBuffer[WFD_STATIC_LEN]; // null terminated string
	
	union {
		INTELWFD_VERSION version; // intel_sink_version

		char szFriendlyName[INTELWFD_FRIENDLY_NAME_LEN+1]; // intel_friendly_name

		char szManufacturerName[INTELWFD_MANUFACTURER_NAME_LEN+1]; // intel_sink_manufacturer_name
		
		char szModelName[INTELWFD_MODEL_NAME_LEN+1]; // intel_sink_model_name
		
		char szDeviceURL[INTELWFD_DEVICE_URL_LEN+1]; // intel_sink_device_URL
		
		INTELWFD_MANUFACTURER_LOGO manufacturerLogo; // intel_sink_manufacturer_logo
		
		int nLowerBandwidth; // intel_lower_bandwidth
		
		INTELWFD_SIGMA_PARAMS sigmaLatencyParams; // intel_sigma_pipeline_params
		
		INTELWFD_OVERSCAN_COMP overscanComp; // intel_overscan_comp

		INTELWFD_USBOIP usboip; // intel_usboip

		unsigned short usRTCPPort; // intel_enable_widi_rtcp
	} u;
} INTELWFD_PARAMETER_STRUCT;
#ifdef WIN32
#pragma pack()
#endif

long INTELWFD_SetParamStruct(void* hRtsp, INTELWFD_PARAMETER_STRUCT* pStruct, int nCount);
long BuildPayloadFromStruct(void* hRtsp, INTELWFD_PARAMETER_STRUCT* pBuf);
long ParseParameter(void* hRtsp, INTELWFD_PARAMETER_STRUCT* pIntel, WFD_PARAMETER_STRUCT* pWfd);
int GetId(char* szName, int nCount, char* strArr[]);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // __INTELRTSPWFD_H__
