#ifndef _REALTEK_P2P_FUNC_
#define _REALTEK_P2P_FUNC_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

//#define	P2P_AUTO	1
//#define DHCP	1       //bq open
#define CMD_SZ 100
#define SSID_SZ	32

#define SEC 1000000
#define SCAN_POOL_NO	10
#define WIDI_MSG_MAX       10
#define NEGO_RETRY_INTERVAL 10 * SEC
#define NEGO_QUERY_INTERVAL 0.5 * SEC
#define PRE_NEGO_INTERVAL 0.5 * SEC
#define MAX_PROV_RETRY 15
#define PROV_RETRY_INTERVAL 5
#define PROV_WAIT_TIME	1 * SEC
#define MAX_NEGO_RETRY 60
#define NEGO_WAIT_TIME	0.5 * SEC
#define	WPS_RETRY 5
#define SUPPLICANT_INIT_TIME 1 * SEC
#define HOSTAPD_INIT_TIME	1 * SEC
#define SUPPLICANT_INTERVAL 1 * SEC
#define HOSTAPD_INTERVAL 1 * SEC
#define POLLING_INTERVAL	1 * SEC
#define _TRUE 1
#define _FALSE 0

#define WPS_CONFIG_METHOD_LABEL		0x0004
#define WPS_CONFIG_METHOD_DISPLAY	0x0008
#define WPS_CONFIG_METHOD_E_NFC		0x0010
#define WPS_CONFIG_METHOD_I_NFC		0x0020
#define WPS_CONFIG_METHOD_NFC		0x0040
#define WPS_CONFIG_METHOD_PBC		0x0080
#define WPS_CONFIG_METHOD_KEYPAD	0x0100
#define WPS_CONFIG_METHOD_VPBC		0x0280
#define WPS_CONFIG_METHOD_PPBC		0x0480
#define WPS_CONFIG_METHOD_VDISPLAY	0x2008
#define WPS_CONFIG_METHOD_PDISPLAY	0x4008

//ssid softap id  flag
#define WIDI_SSID_FLAGE   1
//#define P2P_DEBUG
#if 0
	#define p2pdbg_info(fmt, arg...) printf("P2P[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define p2pdbg_info(fmt, arg...)  do{}while(0);
#endif

#define p2p_err(fmt,arg...) printf("P2P[%s %d]:"fmt"\n",__func__,__LINE__,##arg)
#define p2p_info(fmt,arg...) printf("P2P[%s %d]:"fmt"\n",__func__,__LINE__,##arg)

typedef enum widi_status_t{
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
}widi_status_enum;
typedef struct widi_current_status_t{
	char bssid[18];
	char ssid[32];
	char ip_address[18];
	char wpa_state[20];
	char key_mgmt[50];
	char pairwise_cipher[50];
	char group_cipher[50];
	char mode[50];
	char router[18];
	char DNS_server[50];
}widi_current_status;

typedef enum{
	P2P_REQ_INVALID,
	P2P_REQ_ENQUEUE,
	P2P_REQ_DEQUEUE,
	P2P_REQ_DONE,
}p2p_req_status;

typedef enum{
	WIDI_CMD_DEFAULT =0,
	WIDI_CMD_SCAN,
	WIDI_CMD_PROV,
	WIDI_CMD_WPA_SCAN,
	WIDI_CMD_CONNECT,
	WIDI_CMD_STOPCONNECT
}p2p_ioctl_cmd;

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



typedef struct p2p_ioctrl_t{
	p2p_req_status active;
	unsigned long timestamp;
	p2p_ioctl_cmd iocmd;
	int para_size;
}p2p_ioctrl;

typedef struct p2p_sem_t{
	sem_t sem_role;
	sem_t sem_status;
}p2p_sem;

/*
enum thread_trigger{
	THREAD_NONE = 0,
	THREAD_DEVICE = 1,
	THREAD_GO = 2,
};
*/
enum P2P_ROLE {
	P2P_ROLE_DISABLE = 0,
	P2P_ROLE_DEVICE = 1,
	P2P_ROLE_CLIENT = 2,
	P2P_ROLE_GO = 3	
};
/*
enum P2P_STATE {
	P2P_STATE_NONE = 0,					//	P2P disable
	P2P_STATE_IDLE = 1,						//	P2P had enabled and do nothing
	P2P_STATE_LISTEN = 2,					//	In pure listen state
	P2P_STATE_SCAN = 3,					//	In scan phase
	P2P_STATE_FIND_PHASE_LISTEN = 4,		//	In the listen state of find phase
	P2P_STATE_FIND_PHASE_SEARCH = 5,		//	In the search state of find phase
	P2P_STATE_TX_PROVISION_DIS_REQ = 6,	//	In P2P provisioning discovery
	P2P_STATE_RX_PROVISION_DIS_RSP = 7,
	P2P_STATE_RX_PROVISION_DIS_REQ = 8,	
	P2P_STATE_GONEGO_ING = 9,				//	Doing the group owner negoitation handshake
	P2P_STATE_GONEGO_OK = 10,				//	finish the group negoitation handshake with success
	P2P_STATE_GONEGO_FAIL = 11,			//	finish the group negoitation handshake with failure
	P2P_STATE_RECV_INVITE_REQ = 12,		//	receiving the P2P Inviation request
	P2P_STATE_PROVISIONING_ING = 13,		//	Doing the P2P WPS
	P2P_STATE_PROVISIONING_DONE = 14,	//	Finish the P2P WPS
};
*/
enum P2P_STATE {
	P2P_STATE_NONE = 0,							//	P2P disable
	P2P_STATE_IDLE = 1,								//	P2P had enabled and do nothing
	P2P_STATE_LISTEN = 2,							//	In pure listen state
	P2P_STATE_SCAN = 3,							//	In scan phase
	P2P_STATE_FIND_PHASE_LISTEN = 4,				//	In the listen state of find phase
	P2P_STATE_FIND_PHASE_SEARCH = 5,				//	In the search state of find phase
	P2P_STATE_TX_PROVISION_DIS_REQ = 6,			//	In P2P provisioning discovery
	P2P_STATE_RX_PROVISION_DIS_RSP = 7,
	P2P_STATE_RX_PROVISION_DIS_REQ = 8,	
	P2P_STATE_GONEGO_ING = 9,						//	Doing the group owner negoitation handshake
	P2P_STATE_GONEGO_OK = 10,						//	finish the group negoitation handshake with success
	P2P_STATE_GONEGO_FAIL = 11,					//	finish the group negoitation handshake with failure
	P2P_STATE_RECV_INVITE_REQ_MATCH = 12,		//	receiving the P2P Inviation request and match with the profile.
	P2P_STATE_PROVISIONING_ING = 13,				//	Doing the P2P WPS
	P2P_STATE_PROVISIONING_DONE = 14,			//	Finish the P2P WPS
	P2P_STATE_TX_INVITE_REQ = 15,					//	Transmit the P2P Invitation request
	P2P_STATE_RX_INVITE_RESP = 16,				//	Receiving the P2P Invitation response
	P2P_STATE_RECV_INVITE_REQ_DISMATCH = 17,	//	receiving the P2P Inviation request and dismatch with the profile.
	P2P_STATE_RECV_INVITE_REQ_GO = 18,			//	receiving the P2P Inviation request and this wifi is GO.
	P2P_STATE_RECV_INVITE_REQ_JOIN = 19,			//	receiving the P2P Inviation request to join an existing P2P Group.
	P2P_STATE_RX_INVITE_RESP_FAIL = 20,         // receiving the P2P inviataion response with failure
	P2P_STATE_RX_INFOR_NOREADY = 21,            // receiving p2p negotiation response with information is not available
	P2P_STATE_TX_INFOR_NOREADY = 22,            // sending p2p negotiation response with information is not available. 
};
enum P2P_WPSINFO {
	P2P_NO_WPSINFO						= 0,
	P2P_GOT_WPSINFO_PEER_DISPLAY_PIN	= 1,
	P2P_GOT_WPSINFO_SELF_DISPLAY_PIN	        = 2,
	P2P_GOT_WPSINFO_PBC					= 3,
};

//define p2p mode type for identy 
enum P2P_TYPE_MODE{
	P2P_DEFAULT  = 0,
	P2P_DEVICE     = 1,
	P2P_CLIENT     = 2,
	P2P_SOFTAP    = 3
};


//wifi direct connect flag
typedef enum{
	WIDI_CONNECT_DEFAULT =0,
	WIDI_CONNECT_CLIENT_ING,
	WIDI_CONNECT_CLIENT_OK,
	WIDI_CONNECT_SOFTAP_ING,
	WIDI_CONNECT_SOFTAP_OK,
	WIDI_STATUS_DISCONNECT,
	WIDI_STATUS_REJECT,
	WIDI_STATUS_PROV_FAILURE
}wifi_direct_connect;

typedef enum{
	WIDI_PIN_PBC_DEFAULT =0, // for push button
	WIDI_PIN_PBC_INPUT=1,      // for keypad
	WIDI_PIN_PBC_DISPLAY=2,    // for display or label
	WIDI_PIN_PBC_INVITED=3,    // get invited to join a group, for status 19
	WIDI_PIN_PBC_PERSISTENT_GP_INVITED=4,     // invited and directly as client, for status 12
	WIDI_PIN_PBC_NONE=1000        // not receive any request.

}wifi_direct_io_pin;
typedef enum p2p_authen_type_t{
	WPA,
	WPS,
	OPEN,
}p2p_authen_type;

struct cmd_msg{
	int cmd_type;
	sem_t cmd_sem;
	sem_t cmd_scan_sem;
	int widi_cmd_queue[WIDI_MSG_MAX];
};
struct wd_scan{
	char mac_addr[18]; //wifi direct device ap mac address
	char ssid[32];  //wifi direct device access point ssid
	//int widirect_ip[4];  //wifi dierct device ap ip address
	char widi_device_name[32];  //wifi direct  device name
	//int  ssid_flag; //identify ssid status
	//char widi_password[18];
	//int frequency;
	int singal_level;
	//p2p_authen_type authen;
	int go;
};

struct profile_device{
	char p_ssid[32];
	char p_macaddr[18];
};
/*
typedef struct widi_semi_t{
	sem_t semi_req;
	sem_t semi_start;
	sem_t semi_end;
	sem_t semi_apentry;
}widi_semi;
*/
struct p2p{
	char ifname[10];
	int status;
	int status_for_json;
	int skfd;		//add for socket description in the use of iwprv ioctl 
	char dev_name[33];
	int intent;
	int listen_ch;
	int wps_info;
	int wpsing;
	int peer_port;
	unsigned int pin;
	int role;  // the actual role.
	int start_role; // P2P role when started.
	char apd_ssid[SSID_SZ];
	char peer_devaddr[18];
	char peer_ifaddr[18];
	//char cmd[CMD_SZ];
	//char parse[CMD_SZ];
	char mac_connect_ok[SSID_SZ]; //define client connect flag
	int op_ch;	//operation channel
	int wpa_open;
	int ap_open;
	pthread_t pthread;
	pthread_t pthread_go;
	pthread_t pthread_msg;
	int pbc_flag;      ///display push button flag
	int pthread_runing;
	int pthread_go_runing;
	int phtread_msg_runing;
	int thread_poll_gc;
	int thread_go_gc;
	int thread_msg_gc;
	struct cmd_msg msg;
	//int res;	//check if thread is created; 1: disabled, 0: enabled
	//int res_go;	//created if p2p device becomes GO
	struct wd_scan scan_pool[SCAN_POOL_NO];
	struct profile_device p_device[SCAN_POOL_NO];//store ten device for connect
	int ap_total;             //ap total number
	int scan_ok_flag;
	int connect_go;
	int polling_status_flage;
	int no_sta_connected;
	int connect_flag; //define the widi connect flag
	//int req_start_idx;  //req start idex
	p2p_sem syn_lock;
	char req_cm[40];			//add for mitacast info collection as the request wps method
	int prov_req_cm; // the request config method from peer device.
	pthread_mutex_t locker_for_prov_req_cm; // locker for the prov_req_cm
	int op_ch_5G;
};

static int wifi_direct_device_mode_server(struct p2p *p);
static int wifi_direct_get_ifaddr(struct p2p *p);
static int wifi_direct_set_nego(struct p2p *p);
static void wifi_direct_client_mode(struct p2p *p);
static void wifi_direct_go_mode(struct p2p *p);
static int wifi_direct_set_device_name(struct p2p *p);
static int wifi_p2p_get_device_name(struct p2p *p, int ap_num );

#endif	//_P2P_UI_TEST_H_
