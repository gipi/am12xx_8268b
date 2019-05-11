#ifndef SWFEXT_WIFI_ENGINE_H
#define SWFEXT_WIFI_ENGINE_H

#ifdef MODULE_CONFIG_NETWORK
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "wpa_cli.h"
#include "mnavi_fop.h"
#include <net/if.h>
#include <netinet/in.h>
#include <time.h>
#if 0
	#define wifidbg_info(fmt, arg...) printf("WIFIINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define wifidbg_info(fmt, arg...)
#endif

#define wifidbg_err(fmt,arg...) printf("WIFIERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define AM_CASE_SCRIPTS_DIR         	"/am7x/case/scripts/"

typedef enum authen_type_t{
	WPA,
	WEP,
	OPEN,
}authen_type;

typedef enum status_t{
	WIFI_DISCONNECTED = -1,		//WPA_DISCONNECTED - Disconnected state
	WIFI_COMPLETED,				//WPA_COMPLETED - All authentication completed
	WIFI_INACTIVE,				//WPA_INACTIVE - Inactive state (wpa_supplicant disabled)
	WIFI_SCANNING,				//WPA_SCANNING - Scanning for a network
	WIFI_ASSOCIATING,			//WPA_ASSOCIATING - Trying to associate with a BSS/SSID
	WIFI_ASSOCIATED,			//WPA_ASSOCIATED - Association completed
	WIFI_4WAY_HANDSHAKE,		//WPA_4WAY_HANDSHAKE - WPA 4-Way Key Handshake in progress
	WIFI_GROUP_HANDSHAKE,		//WPA_GROUP_HANDSHAKE - WPA Group Key Handshake in progress
	WIFI_AUTO_IP_ERROR,
	WIFI_AUTO_IP_SUCCESS,
	WIFI_CONNECT_FAILED,
	WIFI_DISCONNECTED_MANUAL,
}status_enum;

typedef enum pwd_t{
	INDEX_OUTRANGE = 1,
	WPA_PWD_ERROR,
	WEP_PWD_ERROR,
	PORT_ERROR,
}pwd_emun;
typedef enum wifi_entire_stutas{
	WIFI_INIT_STATUS=-1,  // init status for wifi dongle   
	WIFI_DONGLE_PLUG_IN = 0 ,  //wifi dongle plug in
	WIFI_DONGLE_PLUG_OUT =1, //wifi dongle plug out
	
	WIFI_CLIENT_MODE_ENABLE = 2,      //enable wifi client    
	WIFI_CLIENT_GETAUTOIP_OK = 3,   //get auto ip 
	WIFI_CLIENT_GETAUTOIP_ERR = 13,   //get auto ip fail ,may be occur on some ap disabe DHCP
	WIFI_SOFTAP_ENABLE = 4 ,        //enable soft ap mode

	WIFI_DIRECT_ANABLE = 5 ,     //wifi direct able
	WIFI_DIRECT_CLIENT_OK = 6 ,   //DPF connect another peer device as client
	WIFI_DIRECT_SOFTAP_OK = 7 ,   //DPF connected by another peer device as softap

	WIFI_CONCURRENT_CLIENT_ENABLE = 8 ,    //anable client while concurrent mode
	WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK =9 ,	  //connect successfully as client while concurrent mode
	WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR=14,   //get auto ip fail ,may be occur on some ap disabe DHCP
	WIFI_CONCURRENT_SOFTAP_ENABLE = 10 ,	  //enable successfully as softap  concurrent mode

	WIFI_DISABLE_MANUAL =11 ,         //user disable manually
	WIFI_MODE_CHANGEING_FOR_RALINK = 12 ,  //special status for ralink wifi dongle when it's swtitch mode 

	
}wifi_entire_stutas_emun;

typedef enum{
	INIT_DONGLE_TYPE=0,
	REALTEK_dongle,
	RALINK_dongle,
}wifi_dongle_type_emun;

typedef enum{
	REALTEK_CU,
	REALTEK_DU,
	REALTEK_8188EU,
	REALTEK_8192EU,
	REALTEK_8811AU,
	REALTEK_8821CU,
	REALTEK_8189ES,
	REALTEK_8188FU
}realtek_wifi_dongle_type_emun;

typedef enum{
	LAN_ROUTING=1,
	WIFI_ROUTING,
}route_type_emun;


typedef enum{
	SWITCH_TO_CLIENT=1,     //swtich to client wifi
	SWITCH_TO_SOFTAP=2,    //switch to softap
	CLOSE_WIFI_PROCESS=3,  //close wifi function
	RESTART_WIFI_PROCESS=4, //restart wifi function
}wifi_function_switch_direction;

#define MAX_CONFFILE 30

#define WEP_KEY_NUM_ 4
#define WEP_KEY_MAXLEN 16
#define WIFI_IOARRAY_LEN 20

#define MAX_SSID_LEN 33
#define MAX_STATE_LEN 20


typedef struct AP_info_t{
	int isvalid;
	char ssid[MAX_SSID_LEN];	///< access point ssid
	char mac_address[18];		///< access point's mac address
	int frequency;				///< access point's frequency
	int singal_level;			///< access point's singal level
	authen_type authen;			///< authen type
	int key_index;
	char ap_password[63];		///< access point's password
	int ip[4];					///< wlan port IP address
	//struct AP_info * ap_info_p;
}AP_info;

typedef struct current_status_t{
	char bssid[18];
	char ssid[MAX_SSID_LEN];
	char ip_address[18];
	char wpa_state[MAX_STATE_LEN];
	char key_mgmt[50];
	char pairwise_cipher[50];
	char group_cipher[50];
	char mode[50];
	char router[18];
	char DNS_server[50];
	char passphrase[63];
	char current_status_value;
	char signal_level[4];
}current_status;

typedef struct wifi_semi_t{
	sem_t semi_req;
	sem_t semi_start;
	sem_t semi_end;
	sem_t semi_apentry;
}wifi_semi;

typedef struct wifi_info_t{
	int AP_num;
	AP_info * ap_entry;
	wifi_semi syn_lock;
	pthread_t thread_id;
	unsigned is_thread_run:1;	///< note whether the thread is run
	int req_start_idx; 			///< the req start idx
	sem_t semi_wpa_cli;
	pthread_t autoconnect_thread_id;
}wifi_info;

typedef enum{
	WIFI_CMD_SCAN,
	WIFI_CMD_STOPSCAN,
	WIFI_CMD_CONNECT,
	WIFI_CMD_STOPCONNECT,
}wifi_ioctl_cmd;

typedef enum{
	WIFI_REQ_INVALID,
	WIFI_REQ_ENQUEUE,
	WIFI_REQ_DEQUEUE,
	WIFI_REQ_DONE,
}wifi_req_status;

typedef struct wifi_ioctrl_t{
	wifi_req_status active;
	unsigned long timestamp;
	wifi_ioctl_cmd iocmd;
	int para_size;
}wifi_ioctrl;

#define WIFI_BUF_LEN 256

int realtek_softap_func(int dongle_dir,void *data);
void __reset_connect_ap(char *connect_ap);


char *dongle_get_device();
int dongle_get_ctrl_iface(char*ctl_iface);
void dongle_in(unsigned int vidpid);
void dongle_out();

void enable_route_function(char *pub_interface,char *private_interface,char *private_subnet,int routing_type);
void disable_route_function(int disable_type);
char * get_soft_ap_entry(int dongle_type,char *key);
void putdown_softap_info(char *softap_ssid,char *softap_psk,char *softap_channel);

/*bq:add a flage to identy wps connect */
extern int wps_flag ;
/*add for realtek channel band*/
enum band_type_t{
	RTL_BAND_DEF=0x00,
	RTL_BAND_5G=0x05,    // realtek channel band 5GHZ
	RTL_BAND_24G=0x02   //realtek channel band 2.4GHZ
};
enum probox_led_on{  //probox led  
	LED24G_ON=0,
	LED24G_OFF,	
	LED5G_ON,
	LED5G_OFF, 
	LEDLAN_ON, 
	LEDLAN_OFF
};

typedef struct channel_region
{
	char *region_name;		///the device name ,such as the realtek dongle is wlan0 and the ralink is ra0
	unsigned int  region_code;
	unsigned int  total_channel_2g;
	unsigned int  total_channel_5g;
	unsigned int  channel_plan[48];	
	unsigned int default_channel_5g;
}channel_region_t;

extern int rtl_band_use;

typedef enum{
	WIFI_BRIDGE_ADD_BR0 = 1,
	WIFI_BRIDGE_ENABLE_BR0,
	WIFI_BRIDGE_ADD_WLAN1,
	WIFI_BRIDGE_DEL_WLAN1,
	WIFI_BRIDGE_DEL_BR0,

	WIFI_BRIDGE_ADD_ETH0_AND_WLAN0,
	WIFI_BRIDGE_ADD_ETH0_AND_WLAN1,
	WIFI_BRIDGE_DEL_ETH0_AND_WLAN0,
	WIFI_BRIDGE_DEL_ETH0_AND_WLAN1,
	WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3,
	
}wifi_bridge_process_t;

int wifiBridgeProcess(wifi_bridge_process_t processChoose);
int getNetRunningFlagByIoctl(char *interfaceName);
char * getIpAddressByIoctl(char *interfaceName);

struct dyn_lease {
    /* "nip": IP in network order */
    /* Unix time when lease expires. Kept in memory in host order.
     * When written to file, converted to network order
     * and adjusted (current time subtracted) */
    unsigned int expires;
    unsigned int lease_nip;
    /* We use lease_mac[6], since e.g. ARP probing uses
     * only 6 first bytes anyway. We check received dhcp packets
     * that their hlen == 6 and thus chaddr has only 6 significant bytes
     * (dhcp packet has chaddr[16], not [6])
     */
    unsigned char lease_mac[6];
    char hostname[20];
    unsigned char pad[2];
    /* total size is a multiply of 4 */
};

typedef struct lease_info_t{
	int item_nu;
	time_t last_change;
	char latest_hostName[20];
	struct dyn_lease *glease;
}lease_info;

#endif	/** MODULE_CONFIG_NETWORK */

#endif
