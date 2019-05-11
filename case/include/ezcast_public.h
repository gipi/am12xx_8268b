#ifndef _EZCAST_PUBLIC_H_
#define _EZCAST_PUBLIC_H_
#include <time.h>
#include <stdint.h>
#include "ezcastpro.h"
#include "ezcast_config.h"

#ifdef MODULE_CONFIG_EZCAST_ENABLE
#define EZCAST_ENABLE MODULE_CONFIG_EZCAST_ENABLE
#else
#define EZCAST_ENABLE 0
#endif

#ifdef MODULE_CONFIG_WEBSETTING_ENABLE
#define WEBSETTING_ENABLE MODULE_CONFIG_WEBSETTING_ENABLE
#else
#define WEBSETTING_ENABLE 0
#endif

#ifdef MODULE_CONFIG_AIRDISK_ENABLE
#define AIRDISK_ENABLE MODULE_CONFIG_AIRDISK_ENABLE
#else
#define AIRDISK_ENABLE 0
#endif

#ifdef MODULE_CONFIG_EZMUSIC_ENABLE
#define EZMUSIC_ENABLE MODULE_CONFIG_EZMUSIC_ENABLE
#else
#define EZMUSIC_ENABLE 0
#endif

#ifdef MODULE_CONFIG_WIFI_ON_LED
#define GPIO_WIFI_ON_LED MODULE_CONFIG_WIFI_ON_LED
#else
#define GPIO_WIFI_ON_LED 0
#endif

#ifdef MODULE_CONFIG_POWER_ON_LED
#define GPIO_POWER_ON_LED MODULE_CONFIG_POWER_ON_LED
#else
#define GPIO_POWER_ON_LED 0
#endif

#ifdef MODULE_CONFIG_EZWIRE_ENABLE
#define EZWIRE_ENABLE MODULE_CONFIG_EZWIRE_ENABLE
#else
#define EZWIRE_ENABLE 0
#endif

#ifdef MODULE_CONFIG_EZWIRE_TYPE
	#define EZWIRE_TYPE	MODULE_CONFIG_EZWIRE_TYPE
#else
	#define EZWIRE_TYPE	0
#endif

#ifdef MODULE_CONFIG_EZCASTPRO_MODE
	#define EZCASTPRO_MODE	MODULE_CONFIG_EZCASTPRO_MODE
#else
	#define EZCASTPRO_MODE	0
#endif

#ifdef MODULE_CONFIG_LAN_ONLY
	#define LAN_ONLY	MODULE_CONFIG_LAN_ONLY
#else
	#define LAN_ONLY	0
#endif

#ifdef MODULE_CONFIG_FLASH_TYPE
	#define FLASH_TYPE	MODULE_CONFIG_FLASH_TYPE
#else
	#define FLASH_TYPE	0
#endif

#define AM8251_EZWIRE		(0)		// 0: All system
#define AM8252B_MIRAWIRE	(1)		// 1: iOS only
#define AM8252C_MIRAPLUG	(2)		// 2: iOS & Android with linein
#define AM8252B_MIRAPLUG	(3)		// 3: iOS & Android
#define AM8251_EZDOCKING	(4)		// 4: All system
#define AM8252N_MIRAWIRE	(5)		// 5: iOS only with 16M snor
#define AM8252_MIRALINK		(6)		// 6: iOS by CDC/ECM
#define AM8252N_MIRALINK	(7)		// 7: iOS by CDC/ECM with 16M snor
#define AM8252N_MIRALINE	(8)		// 8: iOS only with 8M snor
#define AM8256_MIRALINE		(9)		// 9: iOS only with 8M snor and 64M ddr2
#define AM8252N_MIRAPLUG	(10)	// 10: iOS & Android with 16M snor

#define FLASH_TYPE_NAND		(0)		// 0: The storage is nand flash.
#define FLASH_TYPE_16M_SNOR	(1)		// 0: The storage is 16M snor flash.
#define FLASH_TYPE_8M_SNOR	(2)		// 0: The storage is 8M snor flash.

#if EZWIRE_ENABLE || EZCASTPRO_MODE==8075 || EZCASTPRO_MODE==8074
	#define FUN_WIRE_ENABLE	1
#else
	#define FUN_WIRE_ENABLE 0
#endif

#if (EZWIRE_TYPE==AM8251_EZWIRE || EZWIRE_TYPE==AM8251_EZDOCKING || EZWIRE_TYPE==AM8252_MIRALINK || EZWIRE_TYPE==AM8252N_MIRALINK)
	#define EZWIRE_USB_DEVICE_ENABLE 1
#else
	#define EZWIRE_USB_DEVICE_ENABLE 0
#endif

#if defined(MODULE_CONFIG_IOS_CAPTURE) && FUN_WIRE_ENABLE // (EZWIRE_TYPE==0 || EZWIRE_TYPE==1 || EZWIRE_TYPE==2 || EZWIRE_TYPE==3 || EZWIRE_TYPE==4 || EZWIRE_TYPE==5 || EZCASTPRO_MODE==8075)
	#define EZWIRE_CAPTURE_ENABLE 	MODULE_CONFIG_IOS_CAPTURE
#else
	#define EZWIRE_CAPTURE_ENABLE	0
#endif

#if (EZWIRE_TYPE==AM8251_EZWIRE || EZWIRE_TYPE==AM8252B_MIRAWIRE || EZWIRE_TYPE==AM8252C_MIRAPLUG || \
		EZWIRE_TYPE==AM8252B_MIRAPLUG || EZWIRE_TYPE==AM8251_EZDOCKING || EZWIRE_TYPE==AM8252N_MIRAWIRE || \
		EZWIRE_TYPE==AM8252N_MIRALINE || EZWIRE_TYPE==AM8256_MIRALINE || EZWIRE_TYPE==AM8252N_MIRAPLUG || EZCASTPRO_MODE==8075 || EZCASTPRO_MODE==8074)
	#define EZWIRE_PLUGPLAY_DEFAULT 1
#else
	#define EZWIRE_PLUGPLAY_DEFAULT	0
#endif

#if (EZWIRE_TYPE==AM8252B_MIRAWIRE || EZWIRE_TYPE==AM8252N_MIRAWIRE || EZWIRE_TYPE==AM8252_MIRALINK || \
		EZWIRE_TYPE==AM8252N_MIRALINK || EZWIRE_TYPE==AM8252N_MIRALINE || EZWIRE_TYPE==AM8256_MIRALINE)
	#define EZWIRE_ANDROID_ENABLE	0
#else
	#define EZWIRE_ANDROID_ENABLE	1
#endif

#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR || FLASH_TYPE==FLASH_TYPE_8M_SNOR)
	#define IS_SNOR_FLASH			1
#else
	#define IS_SNOR_FLASH			0
#endif

#if LAN_ONLY && EZCASTPRO_MODE == 8075
	#define DEVNAME         "Pro LB02_"
#elif EZWIRE_TYPE == AM8251_EZWIRE
	#define DEVNAME         "EZWire-"
#elif EZWIRE_TYPE == AM8252B_MIRAWIRE
	#define DEVNAME         "MiraWire-"
#elif EZWIRE_TYPE == AM8252C_MIRAPLUG
	#define DEVNAME         "MiraPlug-"
#elif EZWIRE_TYPE == AM8252B_MIRAPLUG
	#define DEVNAME         "MiraPlug-"
#elif EZWIRE_TYPE == AM8251_EZDOCKING
	#define DEVNAME         "EZDock-"
#elif EZWIRE_TYPE == AM8252N_MIRAWIRE
	#define DEVNAME         "MiraWire-"
#elif EZWIRE_TYPE == AM8252_MIRALINK
	#define DEVNAME         "Miralink-"
#elif EZWIRE_TYPE == AM8252N_MIRALINK
	#define DEVNAME 		"Miralink-"
#elif EZWIRE_TYPE == AM8252N_MIRALINE
	#define DEVNAME 		"MiraLine-"
#elif EZWIRE_TYPE == AM8256_MIRALINE
	#define DEVNAME 		"MiraLine-"
#elif (EZWIRE_TYPE == AM8252N_MIRAPLUG)
	#define DEVNAME 		"MiraPlug-"
#else
	#define DEVNAME			"AM-"
#endif

//#define PNP_USE_WIFIDISPLAY

#define EZWIRE_ANDROID_IF_PRE	"usb"

#define EZWIRE_IOS_PORT			"ieth0"
#define PROBOX_5G_CONFIG		"wlan0"
#define PROBOX_24G_CONFIG		"wlan2"
#if EZCAST_ENABLE
	//#define AM8252TO8251KEY_ENABLE
	#if FLASH_TYPE==FLASH_TYPE_16M_SNOR 
	#define AUTOPLAY_SET_ENABLE 	0
	#else
	#define AUTOPLAY_SET_ENABLE 	1
	#endif
	
	//#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE != 0)
		#define ROUTER_ONLY_ENABLE      1
	//#else
	//	#define ROUTER_ONLY_ENABLE      0
	//#endif
	
	#if defined(MODULE_CONFIG_BUSINESS_CUSTOMER) && (MODULE_CONFIG_BUSINESS_CUSTOMER==1)
	#define OTA_SERVER_SET_ENABLE	1
	#else
	#define OTA_SERVER_SET_ENABLE	0
	#endif
	
	//#if defined(MODULE_CONFIG_EZWILAN_ENABLE) && (MODULE_CONFIG_EZWILAN_ENABLE ==1)
	//	#define AUTO_DNS_ENABLE		0
	//#else
		#define AUTO_DNS_ENABLE		1
	//#endif
	
//	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
//		#define EZCAST_LITE_ENABLE	0
//	#else
		#define EZCAST_LITE_ENABLE	1
//	#endif
	#ifdef MODULE_CONFIG_EZCAST5G_ENABLE
		#define EZCAST5G_ENABLE MODULE_CONFIG_EZCAST5G_ENABLE
	#else
		#define EZCAST5G_ENABLE 	0
	#endif
#else
	#define EZCAST5G_ENABLE 		0
	#define AUTOPLAY_SET_ENABLE 	0
	#define EZCAST_LITE_ENABLE		0
	#define AUTO_DNS_ENABLE 		0
	#define OTA_SERVER_SET_ENABLE	0
	#define ROUTER_ONLY_ENABLE		0
#endif

#ifdef MODULE_CONFIG_EZCAST_MAC_LIMIT
	#define EZCAST_MAC_LIMIT MODULE_CONFIG_EZCAST_MAC_LIMIT
#else
	#define EZCAST_MAC_LIMIT 0
#endif

#define OP_GET_SSID			0
#define OP_GET_VERSION		1
#define OP_GET_MAC			2
#define USERINFO_JSONFILE  "/mnt/vram/ezcastuserinfo.json"
#define MIRACAST_DEVICEINFO_ISET	'y'
#define MIRACAST_HDCPDISABLE_PATH	"/tmp/MIRACASTDISABLE"
#define MIRACAST_DEVICEINFO_PATH	"/tmp/MIRACASTDEVICEINFO"
#define GETOTAURL_OTACONFFILE		"/tmp/ota_config.json"
#define GET_OTA_DOWNLOAD_NUM_CONF	"/tmp/ota_num_config.json"
#define GET_OTA_DOWNLOAD_NUM_RETURN	"/tmp/ota_download_num"
#define GET_OTA_DOWNLOAD_NUM_URL	"http://cloud.iezvu.com/API/OTA/log.php"
#define	PIC_PATH 					"/tmp/helpview.jpg"
#define SHOW_PIC_PATH				"/tmp/helptoview.jpg"
#define EZCAST_CONF					"/mnt/vram/ezcast/ezcast.conf"
#define AOA_DISABLE					"/mnt/vram/ezcast/AOA_DISABLE"

#define EZWIRE_PAIR_FLAG_FILE		"/tmp/PAIR_OK"
#define AUDIO_UNSUPPORT_FLAG		"/tmp/audio_time_not_support"

#define USB_AUDIO_SUPPORT			"/tmp/USB_AUDIO_SUPPORT"
#define USB_AUDIO_START				"/tmp/UAC_START"

#define CONFNAME_STANDBY			"standby time"
#define CONFNAME_NEIGHBOUR		"Neighbour"
#define CONFNAME_PSKHIDE			"PSK_hide"
#define CONFNAME_OTASERVER		"OTA server"
#define CONFNAME_CTLMODE			"ctl mode"
#define CONFNAME_EZAIRMODE		"EZAir mode v2"
//for host authority  by cxf 2015/11/26
#define CONFNAME_CONFERENT		"conferent"
#define CONFNAME_AIRVIEW			"airview"
#define CONFNAME_AIRSETTING	       "airoptions"
#define CONFNAME_INTERNET_EN		 "internetenables"
#define CONFNAME_PASSWORD_HIDE	 "passwordhide"
#define CONFNAME_REBOOT	  	        "reloadsystem"
#define CONFNAME_MIRACODE	  	        "miracodeables"
#define CONFNAME_DEVICEMANAGER	  "devicemanager"
#define CONFNAME_NETWORKSETUP	  "networksetup"
#define CONFNAME_AIRVIEW_RATE	  "airview_rate"


#define CONFNAME_INTERNET_CONTROL  	     "Internet Access Control"
#define  CONFNAME_PASSCODE  	     "PASSCODE"
#define  CONFNAME_PASSCODE_VAL	     "PASSCODE_VAL"
#define CONFNAME_SOFTAP_CHANNEL  	     "softap_channel_val"
#define CONFNAME_SOFTAP_5G_CHANNEL		"softap_5g_channel_val"
#define CONFNAME_SOFTAP_5G_COUNTRY		"softap_5g_channel_country"
#define CONFNAME_SNMP  	     "snmp"
#define CONFNAME_IGNORESSID 	     "ignore_ssid"
#define STR_NIL						"nil"
#define CONFNAME_BANDWIDTH_24G	"channel_bandwidth_24g"
#define CONFNAME_BANDWIDTH_5G	"channel_bandwidth_5g"

#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR)
#define HOSTAPD_CON_FILE	"/tmp/rtl_hostapd_01.conf"	
#else
#define HOSTAPD_CON_FILE	"/mnt/user1/softap/rtl_hostapd_01.conf"
#endif
#define HOSTAPD_BAK_CON_FILE	"/mnt/bak/rtl_hostapd_01.conf"
#define P2P_CON_FILE  "/etc/p2p_hostapd_01.conf"
#define TMP_QRCODE_FILE		"/tmp/tmp_qrcode.jpg"

enum{
	CTLMODE_START = -1,
	CTLMODE_DIRECTONLY,
	CTLMODE_ROUTERALLOW,
	CTLMODE_ROUTERONLY,
	CTLMODE_END
};

enum{
	JSON_SET_EZMIRROR_START = 0,
	JSON_SET_EZMIRROR_STOP,
	JSON_SET_INTE_SSID = 4,
	JSON_SET_LANGUAGE,
};

enum{
	CMDNUM_HELPVIEW = 1,
	CMDNUM_APPINFO,
	CMDNUM_GETDONGLEINFO,
	CMDNUM_KEY = 4,
	CMDNUM_TRIALKEY
};

enum{
	HOTSPOT_UNCONNECT = -1,
	HOTSPOT_READY = 0,
	HOTSPOT_SCAN,
	HOTSPOT_CONNECTING,
	HOTSPOT_FAIL,
	HOTSPOT_SUCCESS,
	HOTSPOT_TIMEOUT
};

enum{
	SET_AUTOPLAY_ENABLE = 0,
	SET_AUTOPLAY_VOLUME,
	SET_AUTOPLAY_WAITIME,
	SET_AUTOPLAY_PLAYLIST,
	SET_AUTOPLAY_HOST_AP,
	SET_AUTOPLAY_PROGRESSIVE
};

enum{
	EZWIRE_DEV_IGNORE = -1,		// Device not changed.
	EZWIRE_DEV_UNKNOWN = 0,
	EZWIRE_DEV_ANDROID,
	EZWIRE_DEV_IOS,
	EZWIRE_DEV_PC_MAC
};

enum{
	EZWIRE_STATUS_DISCONNECT = 0,
	EZWIRE_STATUS_READ,
	EZWIRE_STATUS_CONNECTING,
	EZWIRE_STATUS_CONNECTED,
	EZWIRE_STATUS_IP_OK
};

enum{
	MAC_AUTO,
	MAC_WLAN0,
	MAC_WLAN1,
	MAC_WLAN2,
	MAC_WLAN3,
	MAC_ETH0
};

typedef struct ota_num_s{
	char iSet;
	int num;
}ota_num_t;

typedef struct deviceMac_s{
	char iSet;
	char mac[18];
}deviceMac_t;

typedef struct EZCASTARGB_s
{
	uint8_t B;	/**< B component */
	uint8_t G;	/**< G component */
	uint8_t R;	/**< R component */
	uint8_t A;	/**< A component */
} EZCASTARGB_t;

typedef struct websetting_wifi_info_s{
	int websetting_authen_type;
	char websetting_ssid[128];
	char websetting_psk[128];
}websetting_wifi_info_t;

#define EZCASTLOG(fmt, args...) do { struct timespec __t; clock_gettime(CLOCK_MONOTONIC, &__t); printf("<%6lu.%03lu> [%s:%d] "fmt,__t.tv_sec,__t.tv_nsec/1000000,__FUNCTION__,__LINE__,##args); } while (0)
#define EZCASTWARN(fmt, args...) do { struct timespec __t; clock_gettime(CLOCK_MONOTONIC, &__t); printf("<%6lu.%03lu> WARNNING!!! [%s:%d] "fmt,__t.tv_sec,__t.tv_nsec/1000000,__FUNCTION__,__LINE__,##args); } while (0)
#define PROOFREED_COORDINATE(x, y) (x)>(y)?(y):(x)

#define EZMIRRORLITE_ITEM		(3)
#define HOTSPOT_CON_TIMEOUT 	(24)
#define HOTSPOT_READY_TIMEOUT	(20)
#define HOTSPOT_SCAN_TIMEOUT	(8)
#define HOTSPOT_SCAN_READY_TIME	(5)
#endif
