#ifndef __WFD_RTSPSRC_H__
#define __WFD_RTSPSRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rtspwfd_d.h"

#define Declare_Extern
#include "rtspwfd_d.h"

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

#define WFD_FRIENDLY_NAME_LEN		18
#define WFD_MODEL_NAME_LEN			32
#define WFD_MANUFACTURER_NAME_LEN	32
#define WFD_PRESENTATION_URL_LEN	256
#define WFD_DEVICE_ID_LEN			12
#define WFD_PRODUCT_ID_LEN			16
#define WFD_RTSP_SESSION_ID_LEN		64
#define WFD_STATIC_LEN				512
#define WFD_RTSP_MAX_TRANSPORT_LEN	256
#define WFD_IP_ADDR_LEN				16

typedef struct {
	uint64_t S3D_Capability;	// 8*HEXDIG ; in network byte order, see Table 5-19 [wfd_3d_video_formats only]
	unsigned long CEA_Support;		// 8*HEXDIG ; in network byte order, see Table 5-10 [wfd_video_formats only]
	unsigned long VESA_Support;		// 8*HEXDIG ; in network byte order, see Table 5-11 [wfd_video_formats only]
	unsigned long HH_Support;		// 8*HEXDIG ; in network byte order, see Table 5-12 [wfd_video_formats only]
	unsigned char latency;			// 2*HEXDIG ; decoder latency in units of 5 msecs
	unsigned short min_slice_size;	// 4*HEXDIG ; number of macroblocks
	unsigned short slice_enc_params;// 4*HEXDIG ; in network byte order, see Table 5-16
	unsigned char frame_rate_control_support;	// 2*HEXDIG ; Table 5-17
} WFD_H264_MISC_PARAMS;

typedef struct {
	unsigned char profile;
	unsigned char level;
	unsigned short max_hres;
	unsigned short max_vres;
	WFD_H264_MISC_PARAMS misc_params;
} WFD_H264_PROFILE;

typedef struct {
	WFD_AudioCodecs	audio_format;
	unsigned long mode;
	unsigned char latency;
} WFD_AUDIO_CODEC;

typedef struct {
	WFD_InputType eType;
	WFD_InputPath ePath;
} WFD_DETAILED_CAP_PAIR;

typedef struct {
	WFDParams nId; // rds id which will determine which field in the union is applicable
	char* szParam; // null terminated string
	char* szValue; // null terminated string
	long nCount; // when copying into a pre-allocated szValue, this is the length of character array
	char szStaticBuffer[WFD_STATIC_LEN]; // null terminated string
	union {
		struct { // wfd_display_edid
			unsigned short usLength;
			unsigned char* pucEdid;
		} displayEdid;
		struct { // wfd_content_protection
			unsigned short usPort;
			float fHDCPVersion;
		} contentProtection;
		struct { // wfd_coupled_sink
			unsigned char ucSupported;
			unsigned char ucStatus;
			unsigned char aucMACAddr[6];
		} coupledSink;
		struct { // wfd_client_rtp_ports
			char* szProfile;
			unsigned short usPort0;
			unsigned short usPort1;
			WFD_RtpMode eMode;
		} clientPorts;
		struct { // wfd_video_formats/wfd_3d_video_formats
			unsigned char supported;
			unsigned char native;
			unsigned char preferred_display_mode_supported;
			unsigned short num_profiles;
			WFD_H264_PROFILE* aProfiles;
		} videoFormat;
		struct { // wfd_presentation_URL
			char szStream0Addr[WFD_IP_ADDR_LEN];
			char szStream1Addr[WFD_IP_ADDR_LEN];
		} presentationURL;
		WFD_Route route;
		struct { // wfd_audio_codec
			unsigned char supported;
			unsigned short num_codecs;
			WFD_AUDIO_CODEC* aCodecs;
		} audioCodecs;
		struct { // wfd_I2C
			int nSupported;
			int nPort;
		} i2c;
		struct { // wfd_connector_type
			unsigned char ucSupported;
			unsigned char ucDisconnected;
			unsigned char ucType;
		} connectorType;
		struct { // wfd_av_format_change_timing
			uint64_t ullDTS;
			uint64_t ullPTS;
		} avTiming;
		struct { // wfd_uibc_capability
			unsigned char ucSupported;
			unsigned char aucCategory[CountOfWFD_InputCategory]; // used as array of bool's 0/non-zero
			unsigned char aucType[CountOfWFD_InputType]; // used as array of bool's 0/non-zero
			unsigned short usCountOfCapPairs;
			WFD_DETAILED_CAP_PAIR* pCapPairs;
			unsigned short usPort;
		} uibcCapability;
		struct { // wfd_uibc_setting
			unsigned char ucEnabled;
		} uibcSetting;
		unsigned short standby_resume_supported; // wfd_standby_resume_capability
		struct { // wfd_preferred_display_mode
			unsigned char supported;
			unsigned long pixel_clock_in_10khz_units;
			unsigned short horizontal_active_resolution;
			unsigned short horizontal_blanking_period;
			short horizontal_sync_offset;
			unsigned short horizontal_sync_width;
			unsigned short vertical_active_resolution;
			unsigned short vertical_blanking_period;
			short vertical_sync_offset;
			unsigned short vertical_sync_width;
			unsigned char _3d_vertical_blanking_interval;
			unsigned char _2d_s3d_modes;
			unsigned char pixel_depth;
			WFD_H264_PROFILE h264_profile;
		} prefDispMode;
	} u;
} WFD_PARAMETER_STRUCT;

typedef struct {
	char szValue[WFD_RTSP_MAX_TRANSPORT_LEN]; // null terminated string
	unsigned short usRTPPort;
	unsigned short usRTCPPort;
	char m_szSession[WFD_RTSP_SESSION_ID_LEN+1];
} WFD_PLAY_STRUCT;

typedef long (CALLING_CONVENTION *FN_WFD_RTSP_NOTIFICATIONS)(WFD_RTSPNotifications id, void* pValue, void* pContext);
typedef long (CALLING_CONVENTION *FN_WFD_RTSP_RECV)(unsigned char* pData, long nBufSize, long* pnBytesRead, long nTimeout, void* pContext);
typedef long (CALLING_CONVENTION *FN_WFD_RTSP_SEND)(unsigned char* pData, long nBufSize, void* pContext);
typedef void (CALLING_CONVENTION *FN_WFD_RTSP_SLEEP)(int nMsec, void* pContext);
typedef unsigned long (CALLING_CONVENTION *FN_WFD_RTSP_GET_MSEC_TICK)(void* pContext);

#define RTSP_LOG_EMERG   0   /* system is unusable */
#define RTSP_LOG_ALERT   1   /* action must be taken immediately */
#define RTSP_LOG_CRIT    2   /* critical conditions */
#define RTSP_LOG_ERR     3   /* error conditions */
#define RTSP_LOG_WARNING 4   /* warning conditions */
#define RTSP_LOG_NOTICE  5   /* normal but significant condition */
#define RTSP_LOG_INFO    6   /* informational */
#define RTSP_LOG_DEBUG   7   /* debug-level messages */

typedef void (CALLING_CONVENTION *FN_WFD_RTSP_LOG)(int nLevel, char* szString, void* pContext);

long CALLING_CONVENTION WFD_RTSPSnkInit(void** phRtsp, unsigned short usRtpPort, FN_WFD_RTSP_NOTIFICATIONS pfnCallback, FN_WFD_RTSP_RECV pfnRecv, FN_WFD_RTSP_SEND pfnSend, FN_WFD_RTSP_SLEEP pfnSleep, FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, FN_WFD_RTSP_LOG pfnLog, void* pContext);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSnkInit)(void** phRtsp, unsigned short usRtpPort, FN_WFD_RTSP_NOTIFICATIONS pfnCallback, FN_WFD_RTSP_RECV pfnRecv, FN_WFD_RTSP_SEND pfnSend, FN_WFD_RTSP_SLEEP pfnSleep, FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, FN_WFD_RTSP_LOG pfnLog, void* pContext);
long CALLING_CONVENTION WFD_RTSPSrcInit(void** phRtsp, unsigned short usServerRtcpPort, unsigned long ulKeepaliveTimeout, char* pInitialGetParams, FN_WFD_RTSP_NOTIFICATIONS pfnCallback, FN_WFD_RTSP_RECV pfnRecv, FN_WFD_RTSP_SEND pfnSend, FN_WFD_RTSP_SLEEP pfnSleep, FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, FN_WFD_RTSP_LOG pfnLog, void* pContext);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSrcInit)(void** phRtsp, unsigned short usServerRtcpPort, unsigned long ulKeepaliveTimeout, char* pInitialGetParams, char* pOptRdsSetupParams, FN_WFD_RTSP_NOTIFICATIONS pfnCallback, FN_WFD_RTSP_RECV pfnRecv, FN_WFD_RTSP_SEND pfnSend, FN_WFD_RTSP_SLEEP pfnSleep, FN_WFD_RTSP_GET_MSEC_TICK pfnGetTick, FN_WFD_RTSP_LOG pfnLog, void* pContext);
long CALLING_CONVENTION WFD_RTSPSrcAddress(void* phRtsp, char* szIpAddr);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSrcAddress)(void* phRtsp, char* szIpAddr);
long CALLING_CONVENTION WFD_RTSPSetLegacy(void* hRtsp, int bLegacy);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSetLegacy)(void* hRtsp, int bLegacy);
long CALLING_CONVENTION WFD_RTSPGetLegacy(void* hRtsp, int* pbLegacy);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPGetLegacy)(void* hRtsp, int* pbLegacy);
long CALLING_CONVENTION WFD_RTSPParser(void* hRtsp);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPParser)(void* hRtsp);
long CALLING_CONVENTION WFD_RTSPShutdown(void* hRtsp);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPShutdown)(void* hRtsp);
long CALLING_CONVENTION WFD_RTSPSetParam(void* hRtsp, char* szParams);
typedef long (CALLING_CONVENTION *FN_RTSPSetParam)(void* hRtsp, char* szParams);
long CALLING_CONVENTION WFD_RTSPGetParam(void* hRtsp, char* szParams);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPGetParam)(void* hRtsp, char* szParams);
long CALLING_CONVENTION WFD_RTSPSetParamBinary(void* hRtsp, WFDParams id, void* pData, long nDataLen);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSetParamBinary)(void* hRtsp, WFDParams id, void* pData, long nDataLen);
long CALLING_CONVENTION WFD_RTSPSetParamStruct(void* hRtsp, WFD_PARAMETER_STRUCT* pStruct, int nCount);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSetParamStruct)(void* hRtsp, WFD_PARAMETER_STRUCT* pStruct, int nCount);
long CALLING_CONVENTION WFD_RTSPPlay(void* hRtsp);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPPlay)(void* hRtsp);
long CALLING_CONVENTION WFD_RTSPPause(void* hRtsp);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPPause)(void* hRtsp);
long CALLING_CONVENTION WFD_RTSPTeardown(void* hRtsp);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPTeardown)(void* hRtsp);
long CALLING_CONVENTION WFD_RTSPResetIncomingCSeq(void* hRtsp);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPResetIncomingCSeq)(void* hRtsp);
long CALLING_CONVENTION WFD_RTSPDecodeMIME(void* hRtsp, unsigned char* pDst, long* pDstLen, char* pSrc, long nSrcLen);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPDecodeMIME)(void* hRtsp, unsigned char* pDst, long* pDstLen, char* pSrc, long nSrcLen);
long CALLING_CONVENTION WFD_RTSPEncodeMIME(void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPEncodeMIME)(void* hRtsp, char* pDst, long* pDstLen, unsigned char* pSrc, long nSrcLen);
long CALLING_CONVENTION WFD_RTSPSetResponseCodeAndMessage(void* hRtsp, unsigned long ulResponseCode, char* szFormat, ...);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPSetResponseCodeAndMessage)(void* hRtsp, unsigned long ulResponseCode, char* szFormat, ...);

typedef struct {
	FN_WFD_RTSPSnkInit				WFD_RTSPSnkInit;
	FN_WFD_RTSPSrcInit				WFD_RTSPSrcInit;
	FN_WFD_RTSPSrcAddress			WFD_RTSPSrcAddress;
	FN_WFD_RTSPSetLegacy			WFD_RTSPSetLegacy;
	FN_WFD_RTSPGetLegacy			WFD_RTSPGetLegacy;
	FN_WFD_RTSPParser				WFD_RTSPParser;
	FN_WFD_RTSPShutdown				WFD_RTSPShutdown;
	FN_RTSPSetParam					WFD_RTSPSetParam;
	FN_WFD_RTSPGetParam				WFD_RTSPGetParam;
	FN_WFD_RTSPSetParamBinary		WFD_RTSPSetParamBinary;
	FN_WFD_RTSPSetParamStruct		WFD_RTSPSetParamStruct;
	FN_WFD_RTSPPlay					WFD_RTSPPlay;
	FN_WFD_RTSPPause				WFD_RTSPPause;
	FN_WFD_RTSPTeardown				WFD_RTSPTeardown;
	FN_WFD_RTSPResetIncomingCSeq	WFD_RTSPResetIncomingCSeq;
	FN_WFD_RTSPDecodeMIME			WFD_RTSPDecodeMIME;
	FN_WFD_RTSPEncodeMIME			WFD_RTSPEncodeMIME;
	FN_WFD_RTSPSetResponseCodeAndMessage	WFD_RTSPSetResponseCodeAndMessage;
} RTSP_INTERFACE;

long CALLING_CONVENTION WFD_RTSPGetInterface(RTSP_INTERFACE* pApi);
typedef long (CALLING_CONVENTION *FN_WFD_RTSPGetInterface)(RTSP_INTERFACE* pApi);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // __WFD_RTSPSRC_H__
