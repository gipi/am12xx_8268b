#if  !define    __ACT_EZWIFI_CONFIG_H__
#define    __ACT_EZWIFI_CONFIG_H__


#define		EZWIFI_CONFIG_VENDOR_NAME		EZCAST	//Vendor name
#define		EZWIFI_CONFIG_MODEL_NAME		EZCAST	//Model name
#define		EZWIFI_CONFIG_SYSTEM_FONT		1	//Enable/disable system font
#define		EZWIFI_CONFIG_OSD_VIDEO_STATUS		0	//Enable/disable video status OSD
#define		EZWIFI_CONFIG_OSD_VIDEO_BUFFER		1	//Enable/disable video buffer status OSD
#define		EZWIFI_CONFIG_OSD_VIDEO_BUFFER_TEXT		0	//Enable/disable video buffer (text mode) status OSD
#define		EZWIFI_CONFIG_OSD_MIRACAST_STATUS		0	//Enable/disable Miracast status OSD
#define		EZWIFI_CONFIG_OSD_RESOLUTION		0	//Enable/disable resolution status OSD
#define		EZWIFI_CONFIG_MEDIA_STREAMING		1	//Enable/disable media streaming
#define		EZWIFI_CONFIG_MEDIA_STREAM_AUDIO		1	//Enable/disable audio media streaming
#define		EZWIFI_CONFIG_HTTPS_STREAMING		1	//Enable/disable Curl streaming instead of Wget
#define		EZWIFI_CONFIG_MMSX_STREAMING		0	//Enable/disable Mmsx streaming
#define		EZWIFI_CONFIG_RTMP_STREAMING		0	//Enable/disable Rtmp streaming
#define		EZWIFI_CONFIG_SUBTITLE		1	//Enable/disable Subtitle feature
#define		EZWIFI_CONFIG_UCLIBC		0	//Use uclibc to compile or not
#define		EZWIFI_CONFIG_SPLIT_SCREEN		0	//Enable/disable split screen
#define		EZWIFI_CONFIG_SPLIT_NUMBER		4	//Maximum split size, 1~4
#define		EZWIFI_CONFIG_HOMESCREEN_REWORK		0	//Enable/disable Home screen rework (by EZView)
#define		EZWIFI_CONFIG_PASSCODE_LOGIN		0	//Enable/disable password login
#define		EZWIFI_CONFIG_APP_PHOTO_VIEWER		1	//Enable/disable photo viewer app.
#define		EZWIFI_CONFIG_APP_LIVE_CAM		1	//Enable/disable live cam app.
#define		EZWIFI_CONFIG_APP_STREAMING_DOC		1	//Enable/disable streaming doc app.
#define		EZWIFI_CONFIG_APP_DROPBOX		0	//Enable/disable dropbox app.
#define		EZWIFI_CONFIG_APP_WEB_VIEWER		1	//Enable/disable web viewer app.
#define		EZWIFI_CONFIG_APP_QUALITY_MODE		1	//Enable/disable quality mode app.
#define		EZWIFI_CONFIG_APP_HTTP_STREAMING		1	//Enable/disable Http streaming app.
#define		EZWIFI_CONFIG_APP_REMOTE_CONTROL		0	//Enable/disable remote control app.
#define		EZWIFI_CONFIG_HID		0	//Enable/disable HID feature
#define		EZWIFI_CONFIG_EZDISPLAY_V1		1	//Enable/disable EZDisplay-v1 protocol, auto disabled by v2
#define		EZWIFI_CONFIG_EZANALYTICS		1	//Enable/disable EZAnalytics feature
#define		EZWIFI_CONFIG_EZCHANNEL_JS		0	//Enable/disable EZChannel feature
#define		EZWIFI_CONFIG_EZCHANNEL_JS_PATH		"https://www.iezvu.com/view"	//EZChannel duktape-js location
#define		EZWIFI_CONFIG_EZCHANNEL_JSDEBUG_PATH		"http://test.iezvu.com/view"	//EZChannel duktape-js-for-debug location
#define		EZWIFI_CONFIG_EZCHANNEL_PATH		"https://channel.iezvu.com/ez/channel/playlist"	//EZChannel playlist location
#define		EZWIFI_CONFIG_CLOUD_CONTROL		0	//Enable/disable Cloud Control feature
#define		EZWIFI_CONFIG_EZCLOUDCTL_PATH		"wss://eznode.iezvu.com/ws/cloudControl"	//Cloud Control server location
#define		EZWIFI_CONFIG_EZNEIGHBOUR_CAST		0	//Enable/disable Neighbour-cast feature
#define		EZWIFI_CONFIG_AUDIO_LINEIN		0	//Enable/disable Audio linein feature
#define		EZWIFI_CONFIG_EZCAST_APP		0	//Enable/disable EZCast(-app) conectivity mode
#define		EZWIFI_CONFIG_EZCAST_PRO		0	//Enable/disable EZCast(-pro) protocol
#define		EZWIFI_CONFIG_EZCAST_MIRASCREEN		1	//Enable/disable EZCast(-miracast)
#define		EZWIFI_CONFIG_EZCAST_TYPE		dongle-win	//box,projector,tv,dongle,music,car,lite
#define		EZWIFI_CONFIG_AIRPLAY_SERVER		1	//Enable/disable EZAir (Airplay receiver)
#define		EZWIFI_CONFIG_AIRPLAY_MIRROR		1	//Enable/disable EZAir-mirror (Airplay mirror)
#define		EZWIFI_CONFIG_DLNA_DMR		1	//Enable/disable EZDlna (DMR service)
#define		EZWIFI_CONFIG_MIRACAST_RECEIVER		1	//Enable/disable Miracast (receiver)
#define		EZWIFI_CONFIG_INTERNET_WALLPAPER		0	//Enable/disable Internet Wallpaper
#define		EZWIFI_CONFIG_INTERNET_WALLPAPER_JS_PATH		"https://cloudscreen.ezcast.com/slideshow"	//Internet Wallpaper slides location


#endif  //__ACT_EZWIFI_CONFIG_H__
