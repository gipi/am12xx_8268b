
#include "common_d.h"

HeaderDecl(RDSRTSPReturnCode)
	ItemDecl(RTSP_NO_ERROR)
	ItemDecl(RTSP_MEMORY)
	ItemDecl(RTSP_UNEXPECTD_TOKEN)
	ItemDecl(RTSP_RDS_VERSION_MISMATCH)
	ItemDecl(RTSP_CSEQ_MISMATCH)
	ItemDecl(RTSP_CONTENT_LENGTH_MISMATCH)
	ItemDecl(RTSP_INSUFFICIENT_BUF)
	ItemDecl(RTSP_INTERNAL_ERROR)
	ItemDecl(RTSP_THREAD_CREATE_FAILED)
	ItemDecl(RTSP_SOCKET_RECV_TIMEOUT)
	ItemDecl(RTSP_SOCKET_RECV_FAILED)
	ItemDecl(RTSP_SOCKET_SEND_FAILED)
	ItemDecl(RTSP_SOCKET_BIND_FAILED)
	ItemDecl(RTSP_SOCKET_CREATE_FAILED)
	ItemDecl(RTSP_SOCKET_NOT_CONNECTED)
	ItemDecl(RTSP_SEND_ERROR)
	ItemDecl(RTSP_UNEXPECTED_EOLCHAR)
	ItemDecl(RTSP_PARSER_ACTIVE)
	ItemDecl(RTSP_ERROR_RESPONSE)
	ItemDecl(RTSP_SESSION_MISMATCH)
	ItemDecl(RTSP_KEEPALIVE_TIMEOUT)
	ItemDecl(RTSP_UNSUPPORTED)
	ItemDecl(RTSP_INVALID_METHOD)
	ItemDecl(RTSP_INVALID_ID)
	ItemDecl(RTSP_SOCKET_CLOSED)
	ItemDecl(RTSP_INVALID_RDS_PARAMS)
TailDecl(RDSRTSPReturnCode)

HeaderDecl(RDSRTSPNotifications)
	ItemDecl(RTSP_PLAY)                  //Client ready to receive media
	ItemDecl(RTSP_TEARDOWN)              //Client shutdown RTSP
	ItemDecl(RTSP_TIMEOUT)               //timeout waiting for response from peer
	ItemDecl(RTSP_GET_PARAMETER)         //get parameter, szValue=&PARAMETER_STRUCT
	ItemDecl(RTSP_GET_PARAMETER_RESP)    //get parameter response, szValue=&PARAMETER_STRUCT
	ItemDecl(RTSP_SET_PARAMETER)         //set parameter, szValue=&PARAMETER_STRUCT	
TailDecl(RDSRTSPNotifications)

HeaderDecl(RDSParams)
	ItemDecl(rds_friendly_name)           // 
	ItemDecl(rds_sink_manufacturer_name)  // 
	ItemDecl(rds_sink_model_name)         // 
	ItemDecl(rds_sink_manufacturer_logo)  // 
	ItemDecl(rds_sink_version)            // 
	ItemDecl(rds_sink_device_URL)         // 
	ItemDecl(rds_unique_device_id)        // 
	ItemDecl(rds_status)                  // 
	ItemDecl(rds_audio_formats)           // (deprecated)
	//ItemDecl(rds_audio_format_setting)  // (deprecated)
	ItemDecl(rds_video_formats)           // (deprecated)
	//ItemDecl(rds_video_format_setting)  // (deprecated)
	ItemDecl(rds_rtp_profile)             // (deprecated)
	ItemDecl(rds_rtp_profile_setting)     // (deprecated)
	//ItemDecl(rds_languages)             // (deprecated)
	//ItemDecl(rds_language_setting)      // (deprecated)
	ItemDecl(rds_content_protection)      // 
	ItemDecl(rds_display_edid)            // 
	//ItemDecl(rds_opt_in_pairing)        // (deprecated)
	//ItemDecl(rds_sink_capabilities)     // (deprecated)
	ItemDecl(rds_trigger_method)          // 
	ItemDecl(rds_presentation_URL)        // 
	//ItemDecl(rds_description)           // (deprecated)
	ItemDecl(rds_session_timeout)         // (deprecated)
	//ItemDecl(rds_max_session_timeout)   // (deprecated)
	ItemDecl(rds_keepalive)               // 
	ItemDecl(rds_overscan_comp)           // 
	ItemDecl(rds_HDMI_DIP)                // 
	//ItemDecl(rds_CEC)                   // (deprecated)
	//ItemDecl(rds_DDC)                   // (deprecated)
	ItemDecl(rds_enable_widi_rtcp)        // requests adapter to send WIDI App defined RTCP messages
	ItemDecl(rds_init_intel_hdshk)        // parameter is (Rtx) 32 byte array with each byte represented as two hex digits with no spaces between bytes 
	ItemDecl(rds_resp_intel_hdshk)        // parameters are two (Rrx H') 32 byte arrays represented as above 
	ItemDecl(rds_cfrm_intel_hdshk)        // parameter is (H") 32 byte array represented as above 
	ItemDecl(rds_fail_intel_hdshk)        // parameter is optional reason string 
	ItemDecl(rds_sigma_pipeline_params)   // parameter: PlaybackDelay=<integer>; PositiveMaxSrcPCR=<integer>; NegativeMaxSrcPCR=<integer>
	ItemDecl(rds_fast_cursor_disabled)      // parameter: 1 - disabled, else enabled
TailDecl(RDSParams)

HeaderDecl(RDSAudioFormats)               // Section 4.3.9
	ItemDecl2(AAC_LC_128,AAC-LC-128)
	ItemDecl2(LPCM16_48_x2,LPCM16-48-x2)
	ItemDecl(AC3)
	ItemDecl2(DTS5_1,DTS5.1)
	ItemDecl2(LPCM16_48_x6,LPCM16-48-x6)
	ItemDecl2(E_AC_3,E-AC-3)
	ItemDecl(TrueHD)
	ItemDecl2(DTS_HD_MASTER,DTS-HD-MASTER)
TailDecl(RDSAudioFormats)

HeaderDecl(RDSVideoFormats)               // Section 4.3.11
	ItemDecl2(MPEG_2_MP_HL,MPEG-2-MP@HL)
	ItemDecl2(RDS_CLASS1,RDS-CLASS1)
	ItemDecl2(RDS_CLASS1b,RDS-CLASS1b)
	ItemDecl2(RDS_CLASS2,RDS-CLASS2)
	ItemDecl2(RDS_CLASS3,RDS-CLASS3)
TailDecl(RDSVideoFormats)

HeaderDecl(RDSProfiles)                   // Section 4.3.13
	ItemDecl2(RTP_AVPF,RTP/AVPF)
	ItemDecl2(RTP_AVP,RTP/AVP)
TailDecl(RDSProfiles)

HeaderDecl(RDSSigmaPipelineParams)
	ItemDecl(PlaybackDelay)
	ItemDecl(PositiveMaxStcPCR)
	ItemDecl(NegativeMaxStcPCR)
TailDecl(RDSSigmaPipelineParams)

