#ifndef __RTSPSRC_H__
#define __RTSPSRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rdsrtsp_d.h"

#define Declare_Extern
#include "rdsrtsp_d.h"

#ifdef _WIN32
#define CALLING_CONVENTION              __stdcall
#elif __GNUC__
#define CALLING_CONVENTION
#else
#error Please define CALLING_CONVENTION for this compiler
#endif

#define RDS_FRIENDLY_NAME_LEN		18
#define RDS_MODEL_NAME_LEN			32
#define RDS_MANUFACTURER_NAME_LEN	32
#define RDS_MANUFACTURER_URL_LEN	256
#define RDS_DEVICE_ID_LEN			12
#define RDS_PRODUCT_ID_LEN			16
#define RTSP_SESSION_ID_LEN			8
#define RDS_STATIC_LEN				512
#define RDS_MAX_OVERSCAN_COMP		15
#define RTSP_MAX_TRANSPORT_LEN		256

typedef struct {
	unsigned char ucMajor;
	unsigned char ucMinor;
	unsigned char ucSku;
	unsigned char usBuild;
} VERSION_STRUCT;

typedef struct {
	RDSParams nId; // rds id which will determine which field in the union is applicable
	char* szParam; // null terminated string
	char* szValue; // null terminated string
	int nCount; // when copying into a pre-allocated szValue, this is the length of character array
	char szStaticBuffer[RDS_STATIC_LEN]; // null terminated string
	union {
		struct { // rds_content_protection
			unsigned short usPort;
			float fHDCPVersion;
		} contentProtection;
		struct { // rds_sink_version
			VERSION_STRUCT verHardware;
			VERSION_STRUCT verSoftware;
			char szProductId[RDS_PRODUCT_ID_LEN+1];
		} version;
		struct { // rds_status
			int nBusy;
			int nDisplayConnected;
		} status;
		struct { // rds_overscan_comp
			int nX;
			int nY;
		} overscanComp;
		struct { // rds_sigma_pipeline_params
			int nPlaybackDelay;
			int nPositiveMaxStcPCR;
			int nNegativeMaxStcPCR;
		} sigmaPipeline;
		unsigned long ulFormats; // rds_audio_formats|audio_video_formats
		RDSProfiles profile; // rds_rtp_profile
	} u;
} PARAMETER_STRUCT;

typedef struct {
	char szValue[RTSP_MAX_TRANSPORT_LEN]; // null terminated string
	unsigned short usRTPPort;
	unsigned short usRTCPPort;
} PLAY_STRUCT;

#ifndef _FN_RTSP_NOTIFICATIONS
#define _FN_RTSP_NOTIFICATIONS
typedef long (CALLING_CONVENTION *FN_RTSP_NOTIFICATIONS)(RDSRTSPNotifications id, void* pValue, void* pContext);
#endif

#ifndef _FN_RTSP_RECV
#define _FN_RTSP_RECV
typedef long (CALLING_CONVENTION *FN_RTSP_RECV)(unsigned char* pData, long nBufSize, long* pnBytesRead, long nTimeout, void* pContext);
#endif

#ifndef _FN_RTSP_SEND
#define _FN_RTSP_SEND
typedef long (CALLING_CONVENTION *FN_RTSP_SEND)(unsigned char* pData, long nBufSize, void* pContext);
#endif

#ifndef _FN_RTSP_SLEEP
#define _FN_RTSP_SLEEP
typedef void (CALLING_CONVENTION *FN_RTSP_SLEEP)(int nMsec, void* pContext);
#endif

#ifndef _FN_RTSP_GET_MSEC_TICK
#define _FN_RTSP_GET_MSEC_TICK
typedef unsigned long (CALLING_CONVENTION *FN_RTSP_GET_MSEC_TICK)(void* pContext);
#endif

#define RTSP_LOG_EMERG   0   /* system is unusable */
#define RTSP_LOG_ALERT   1   /* action must be taken immediately */
#define RTSP_LOG_CRIT    2   /* critical conditions */
#define RTSP_LOG_ERR     3   /* error conditions */
#define RTSP_LOG_WARNING 4   /* warning conditions */
#define RTSP_LOG_NOTICE  5   /* normal but significant condition */
#define RTSP_LOG_INFO    6   /* informational */
#define RTSP_LOG_DEBUG   7   /* debug-level messages */

#ifndef _FN_RTSP_LOG
#define _FN_RTSP_LOG
typedef void (CALLING_CONVENTION *FN_RTSP_LOG)(int nLevel, char* szString, void* pContext);
#endif

long CALLING_CONVENTION RTSPSnkInit(void** phRtsp, unsigned short usRtpPort, FN_RTSP_NOTIFICATIONS pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext);
long CALLING_CONVENTION RTSPSrcInit(void** phRtsp, unsigned short usServerRtcpPort, int nOverscanComp, char* pOptRdsSetupParams, FN_RTSP_NOTIFICATIONS pfnCallback, FN_RTSP_RECV pfnRecv, FN_RTSP_SEND pfnSend, FN_RTSP_SLEEP pfnSleep, FN_RTSP_GET_MSEC_TICK pfnGetTick, FN_RTSP_LOG pfnLog, void* pContext);
long CALLING_CONVENTION RTSPSrcUpdateParams(void* hRtsp, int nOverscanComp, char* pOptRdsSetupParams);
long CALLING_CONVENTION RTSPSrcAddress(void* phRtsp, char* szIpAddr);
long CALLING_CONVENTION RTSPSetLegacy(void* hRtsp, int bLegacy);
long CALLING_CONVENTION RTSPGetLegacy(void* hRtsp, int* pbLegacy);
long CALLING_CONVENTION RTSPParser(void* hRtsp);
long CALLING_CONVENTION RTSPShutdown(void* hRtsp);
long CALLING_CONVENTION RTSPSetParam(void* hRtsp, char* szParams);
long CALLING_CONVENTION RTSPGetParam(void* hRtsp, char* szParams);
long CALLING_CONVENTION RTSPSetParamBinary(void* hRtsp, RDSParams id, void* pData, long nDataLen);
long CALLING_CONVENTION RTSPSetParamStruct(void* hRtsp, PARAMETER_STRUCT* pStruct);
long CALLING_CONVENTION RTSPTeardown(void* hRtsp);
long CALLING_CONVENTION RTSPResetIncomingCSeq(void* hRtsp);
long CALLING_CONVENTION RTSPDecodeMIME(void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen);
long CALLING_CONVENTION RTSPEncodeMIME(void* hRtsp, char* pDst, int* pDstLen, unsigned char* pSrc, int nSrcLen);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // __RTSPSRC_H__
