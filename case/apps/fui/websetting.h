#ifndef __WEBSETTING_H__
#define __WEBSETTING_H__
struct SysCGI_priv_s{
	char * ssid_string_for_cgi;
	char * ssid_signal_string;
	char * ssid_index_string;
	char ssid_str[500][128];
	int language_index;
	int ssid_signal[500];
	int ssid_index[500];
	char connect_status[5];
	/*
	struct wifiListNode ssid_str;
	struct wifiListNode ssid_signal;
	struct wifiListNode ssid_index;
	*/
	
	unsigned int  CONNECTIONS_OK_LED_twinkle_flag:1,
			CONNECTIONS_OK_LED_twinkle_tick:1,
			WIFI_ON_LED_flag:1,
			WIFI_CLIENT_OK_LED_flag:1,
			WIFI_CLIENT_OK_LED_twinkle_tick:1,
			start_audio_flag:1,
			start_audio_once:1,
			play_warning_tone_flag:1,
			connect_router_waiting_lan_flag:1,
			connect_router_waiting_flag:1,
			connect_router_success_flag:1,
			connect_router_failed_flag:1,
			connected_router_useful_flag:1,
			not_connected_router_useful_flag:1,
			otaupgrade_waiting_lan_flag:1,
			otaupgrade_waiting_lan_once:1,
			otaupgrade_waiting_flag:1,
			otaupgrade_successful_flag:1,
			otaupgrade_failed_flag:1,
			otaupgrade_download_finished:1,
			play_connect_router_waiting_start:1,
			play_connect_router_waiting_end:1,
			play_otaupgrade_waiting_start:1,
			play_otaupgrade_waiting_end:1,
			connected_ssid_once:1,
			router_disconnect_flag:1,
			new_firmware_version_flag:1,	
			Bye_bye_flag:1,
			mainswf_skip_get_keycode_flag:1,
			start_ota_upgrade_flag:1,
			otaupgrade_failed_and_stop:1,
			stream_play_status:1;
	int enter_factory_test_video;
	int connections_num;
	struct itimerval WIFI_CLIENT_OK_LED_twinkle_timer;
	struct itimerval CONNECTIONS_OK_LED_twinkle_timer;	
	struct itimerval get_warn_music_status_twinkle_timer;	
};

struct wifiListNode
{
	char *  ssidList_data;
	struct node *ssidList_next;
};

struct websetting_funLink_s{
	char name[128];
	int (*func)(char *cmd,char *val);
};
struct SysCGI_priv_s SysCGI_priv;
#define SOCKET_ERROR        -1

#define PICO_QUERY_INFO				5
#define PICO_CONTROL				10

#define TCPCLIENT_HDR_SIZE			8 //Sequence(4)+TotalBytes(4)
#define TCPCOMMON_HDR_SIZE			24 //TCP frame common header
#define TCPPACKET_HDR_SIZE			(TCPCLIENT_HDR_SIZE+TCPCOMMON_HDR_SIZE) // TCP frame complete header

#define TCP_HDR_QUERYINFO_OFFSET	(TCPCLIENT_HDR_SIZE+8)
#define TCP_HDR_QUERYISIZE_OFFSET	(TCPCLIENT_HDR_SIZE+12)
#define TCP_HDR_QUERYRESULT_OFFSET	(TCPCLIENT_HDR_SIZE+16)

#define QUERY_INFO_OK				0x1

#define INFO_TCP_CONNECTIONS		2

#define TCP_HDR_CTRLRESULT_OFFSET	(TCPCLIENT_HDR_SIZE+20)

#define CONTROL_STOP				1
#define CONTROL_CHANGE				2
#define CONTROL_DISCONNECT			3
#define CONTROL_DISCONNECT_ALL		4
#define CONTROL_ASSIGN_HOST			5


#define FLAG_DPF_TO_DPF				0x2

#define DEFAULT_HOST_NAME	"127.0.0.1"
#define DEFAULT_HOST_PORT	0x0979

#define BUF_LEN 			1024*1024
typedef struct ezWifiPkt_s ezWifiPkt_t;
struct ezWifiPkt_s 
{
	unsigned long seqNo;
	unsigned long totalSize;
	unsigned long tag;
	unsigned char flag;
	unsigned char len;
	unsigned char reserve[2];
	unsigned char cdb[16];
};

typedef struct ezWifiQueryCdb_s ezWifiQueryCdb_t;
struct ezWifiQueryCdb_s 
{
	unsigned long info;
	unsigned long iSize;
	unsigned long result;
	unsigned long reserve;
};

typedef struct ezWifiReqCdb_s ezWifiReqCdb_t;
struct ezWifiReqCdb_s 
{
	unsigned long command;
	unsigned long splitSize;
	unsigned long displayId;
	union
	{
		unsigned long result;
		unsigned long iSize;
	};
};

typedef struct recv_ip_info_s
{
		
       char projective_ip[32];
	int position;
} recv_ip_info_t;

int ezcast_wifi_open(void);
int ezcast_wifi_start(void);
int ezcast_wifi_get_dongle_type(void);
int ezcast_wifi_startscan(void);
int ezcast_wifi_getscanresults(void);
char* ezcast_wifi_getssid(int index);
int ezcast_wifi_getsingal(int index);
char *ezcast_wifi_getIP(void);
char  * ezcast_wifi_get_softap_psk(void);
void ezcast_change_psk(char *psk);
int ezcast_wifi_getAuthenType(int index);
int ezcast_wifi_disconnect_ap(void);
int ezcast_wifi_setAuthenType(int index,int auth_type);
int ezcast_wifi_setssid(int index,char * AP_ssid);
int ezcast_wifi_setpassword(int index,char * ap_password);
int ezcast_wifi_saveconf(void);
int ezcast_wifi_connectAP(void);
int ezcast_wifi_getconfexist(int index);
char * ezcast_wifi_get_connected_ssid();
int ezcast_wifi_deleteconf(int index);
int ezcast_sysinfo_get_wifi_mode(void);
void open_start_wifi();
void wifi_start_scan();
void get_scanresults(void);
int ssid_connect_ok_show();
int ezcast_wifi_getStatus(void);
int ezcast_wifi_addnetwork(void);
int ezcast_wifi_saveaddedAPconf(void);
int ezcast_sysinfo_ota_upgrade(int mode,char *url);
int ezcast_sysinfo_get_ota_check_status(void );
char * ezcast_sysinfo_get_ota_server_version(void);
char * ezcast_sysinfo_get_ota_local_version(void);
int ezcast_sysinfo_get_ota_status(void);
void ezcast_sysinfo_set_local_language(int lang);
void ezcast_sysinfo_set_disauto_status(int status);
void ezcast_sysinfo_store_config(void );
char * ezcast_get_string(char * ids);
int ezcast_sysinfo_json_set_value(int op,char *val);
int ezcast_set_language(char *lang);
void ezcast_sysinfo_Set_language_index(int lan_index);
int ezcast_sysinfo_get_local_language(void );
int ezcast_Get_language_index(void);
char* ezcast_sysinfo_langautoget(void );
int ezcast_sysinfo_get_disauto_status(void );
int ezcast_set_language_swf_name(char *lang,char *name);
int ezcast_sysinfo_read_edid_bin(int read_index);
int ezcast_sysinfo_creat_edid_bin(int hdmi_valid,int hdmi_format,int vga_valid,int vga_format);
void ezcast_sysinfo_set_HDMI_mode(int hmode);
void ezcast_sysinfo_set_last_ui(int last_ui,int changed_by_user);
int ezcast_sysinfo_get_last_ui(void);
char * ezcast_wifi_get_connect_mac_address(int port);
char * ezcast_wifi_get_softap_ssid(void);
int ezcast_wifi_count_accessDevices(void);
int ezcast_sysinfo_get_cpu_frequency(void);
void ezcast_ae_play(void);
void ezcast_ae_close(void);
int ezcast_ae_set_file(char * file);
int ezcast_ae_stop(void);
int ezcast_ae_get_state(void);
int ezcast_ae_open(void);
void ezcast_sysinfo_set_router_ctl(int val,int storage);
int ezcast_sysinfo_get_router_ctl();
char* ezcast_sysinfo_get_ip(int port);
void ezcast_sysinfo_StopNetDisplay(void);
void ezcast_wifi_stop(void);
void ezcast_wifi_close(void);
int ezcast_get_device_name(char *devName, int len);
 void ezcast_set_device_name(char *devName);
 void ezmusic_am7x_dlna_start_work(void);
void ezmusic_am7x_dlna_start_DMS(void);
void ezmusic_am7x_dlna_stop_work(void);
void  websetting_set_internet_access_control_state(void);
void create_websetting_server(void);
#endif
