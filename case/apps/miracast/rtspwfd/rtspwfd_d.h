
#include "common_d.h"

HeaderDecl(WFD_RTSPReturnCode)
	ItemDecl(WFD_RTSP_NO_ERROR)
	ItemDecl(WFD_RTSP_MEMORY)
	ItemDecl(WFD_RTSP_UNEXPECTD_TOKEN)
	ItemDecl(WFD_RTSP_VERSION_MISMATCH)
	ItemDecl(WFD_RTSP_CSEQ_MISMATCH)
	ItemDecl(WFD_RTSP_CONTENT_LENGTH_MISMATCH)
	ItemDecl(WFD_RTSP_INSUFFICIENT_BUF)
	ItemDecl(WFD_RTSP_INTERNAL_ERROR)
	ItemDecl(WFD_RTSP_THREAD_CREATE_FAILED)
	ItemDecl(WFD_RTSP_SOCKET_RECV_TIMEOUT)
	ItemDecl(WFD_RTSP_SOCKET_RECV_FAILED)
	ItemDecl(WFD_RTSP_SOCKET_SEND_FAILED)
	ItemDecl(WFD_RTSP_SOCKET_BIND_FAILED)
	ItemDecl(WFD_RTSP_SOCKET_CREATE_FAILED)
	ItemDecl(WFD_RTSP_SOCKET_NOT_CONNECTED)
	ItemDecl(WFD_RTSP_SEND_ERROR)
	ItemDecl(WFD_RTSP_UNEXPECTED_EOLCHAR)
	ItemDecl(WFD_RTSP_PARSER_ACTIVE)
	ItemDecl(WFD_RTSP_ERROR_RESPONSE)
	ItemDecl(WFD_RTSP_SESSION_MISMATCH)
	ItemDecl(WFD_RTSP_KEEPALIVE_TIMEOUT)
	ItemDecl(WFD_RTSP_UNSUPPORTED)
	ItemDecl(WFD_RTSP_INVALID_METHOD)
	ItemDecl(WFD_RTSP_INVALID_ID)
	ItemDecl(WFD_RTSP_SOCKET_CLOSED)
	ItemDecl(WFD_RTSP_INVALID_WFD_PARAMS)
	ItemDecl(WFD_RTSP_NO_TERMCAP_INTERSECTION)
	ItemDecl(WFD_RTSP_KEEPALIVE_PERIOD_TOO_SHORT)
	ItemDecl(WFD_RTSP_PAYLOAD_BUILT)
	ItemDecl(WFD_RTSP_STANDBY_MODE)
TailDecl(WFD_RTSPReturnCode)

HeaderDecl(WFD_RTSPNotifications)
	ItemDecl(WFD_RTSP_PLAY)                  //Client ready to receive media
	ItemDecl(WFD_RTSP_PAUSE)                 //Client wants to suspend receiving media
	ItemDecl(WFD_RTSP_SETUP)                 //Client sent setup message
	ItemDecl(WFD_RTSP_TEARDOWN)              //Client shutdown RTSP
	ItemDecl(WFD_RTSP_TIMEOUT)               //timeout waiting for response from peer
	ItemDecl(WFD_RTSP_GET_PARAMETER)         //get parameter, szValue=&WFD_PARAMETER_STRUCT
	ItemDecl(WFD_RTSP_GET_PARAMETER_RESP)    //get parameter response, szValue=&WFD_PARAMETER_STRUCT
	ItemDecl(WFD_RTSP_SET_PARAMETER)         //set parameter, szValue=&WFD_PARAMETER_STRUCT	
	ItemDecl(WFD_RTSP_PREF_DISP_EXCHANGE)    //notification to send SET_PARAMETER to start preferred display mode exchange	
	ItemDecl(WFD_RTSP_PREF_DISP_CAP_EXCHANGE)//notification to send SET_PARAMETER to complete preferred display mode capability exchange	
	ItemDecl(WFD_RTSP_CAP_EXCHANGE)          //notification to send SET_PARAMETER to complete capbility exchange without preferred display mode
TailDecl(WFD_RTSPNotifications)

HeaderDecl(WFDParams)
	ItemDecl(wfd_audio_codecs)             // 6.1.2
	ItemDecl(wfd_video_formats)            // 6.1.3
	ItemDecl(wfd_3d_video_formats)         // 6.1.4
	ItemDecl(wfd_content_protection)       // 6.1.5
	ItemDecl(wfd_display_edid)             // 6.1.6
	ItemDecl(wfd_coupled_sink)             // 6.1.7
	ItemDecl(wfd_trigger_method)           // 6.1.8
	ItemDecl(wfd_presentation_URL)         // 6.1.9
	ItemDecl(wfd_client_rtp_ports)         // 6.1.10
	ItemDecl(wfd_route)                    // 6.1.11
	ItemDecl(wfd_I2C)                      // 6.1.12
	ItemDecl(wfd_av_format_change_timing)  // 6.1.13
	ItemDecl(wfd_preferred_display_mode)   // 6.1.14
	ItemDecl(wfd_uibc_capability)          // 6.1.15
	ItemDecl(wfd_uibc_setting)             // 6.1.16 
	ItemDecl(wfd_standby_resume_capability)// 6.1.17
	ItemDecl(wfd_standby)                  // 6.1.18
	ItemDecl(wfd_connector_type)           // 6.1.19
	ItemDecl(wfd_idr_request)              // 6.1.20
TailDecl(WFDParams)

HeaderDecl(WFD_AudioCodecs)                 // Section 6.1.2
	ItemDecl2(WFD_LPCM,LPCM)
	ItemDecl2(WFD_AAC,AAC)
	ItemDecl2(WFD_AC3,AC3)
TailDecl(WFD_AudioCodecs)

HeaderDecl(WFD_RtpMode)                     // Section 6.1.10
	ItemDecl2(WFD_RTP_PLAY,mode=play)
TailDecl(WFD_RtpMode)

HeaderDecl(WFD_Route)                     // Section 6.1.11
	ItemDecl2(WFD_PRIMARY_ROUTE,primary_route)
	ItemDecl2(WFD_SECONDARY_ROUTE,secondary_route)
TailDecl(WFD_Route)

HeaderDecl(WFD_InputCategory)                     // Section 6.1.15
	ItemDecl2(WFD_GENERIC,GENERIC)
	ItemDecl2(WFD_HIDC,HIDC)
TailDecl(WFD_InputCategory)

HeaderDecl(WFD_InputType)                     // Section 6.1.15
	ItemDecl2(WFD_KEYBOARD,Keyboard)
	ItemDecl2(WFD_MOUSE,Mouse)
	ItemDecl2(WFD_SINGLETOUCH,SingleTouch)
	ItemDecl2(WFD_MULTITOUCH,MultiTouch)
	ItemDecl2(WFD_JOYSTICK,Joystick)
	ItemDecl2(WFD_CAMERA,Camera)
	ItemDecl2(WFD_GESTURE,Gesture)
	ItemDecl2(WFD_REMOTECONTROL,RemoteControl)
TailDecl(WFD_InputType)

HeaderDecl(WFD_InputPath)                     // Section 6.1.15
	ItemDecl2(WFD_INFRARED,Infrared)
	ItemDecl2(WFD_USB,USB)
	ItemDecl2(WFD_BT,BT)
	ItemDecl2(WFD_ZIGBEE,Zigbee)
	ItemDecl2(WFD_WIFI,Wi-Fi)
	ItemDecl2(WFD_NOSP,No-SP)
TailDecl(WFD_InputPath)

HeaderDecl(WFD_ConnectorTypes)                     // Table 6-6
	ItemDecl2(WFD_VGA,VGA)
	ItemDecl2(WFD_SVIDEO,S-Video)
	ItemDecl2(WFD_COMPOSITE,Composite)
	ItemDecl2(WFD_COMPONENT,Comopnent)
	ItemDecl2(WFD_DVI,DVI)
	ItemDecl2(WFD_HDMI,HDMI)
	ItemDecl2(WFD_CONNECTOR_TYPE_RESERVED6,Reserved6)
	ItemDecl2(WFD_WIFI_DISPLAY,WiFi-Display)
	ItemDecl2(WFD_JAPANESE_D,Japanese-D)
	ItemDecl2(WFD_SDI,SDI)
	ItemDecl2(WFD_DP,DisplayPort)
	ItemDecl2(WFD_CONNECTOR_TYPE_RESERVED11,Reserved11)
	ItemDecl2(WFD_UDI,UDI)
	ItemDecl2(WFD_UNKNOWN_CONNECTOR,255)
TailDecl(WFD_ConnectorTypes)

