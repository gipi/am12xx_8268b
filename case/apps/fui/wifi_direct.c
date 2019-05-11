/**
* realtek wifi direct functions
*/
#include "swf_ext.h"
#include "stdio.h"
#include "string.h"
#include <pthread.h>
#include <stdbool.h>
#include "wifi_direct.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "fcntl.h"
#include "unistd.h"
#include "signal.h"
#include <errno.h>
#include <time.h>
/*
*add for iwpriv ioctl by amos
*/
#include "iwlib.h"
#include "wireless.h"         
/*
*add for json file
*/
//#include "json.h"
#include "ez_json.h"
#include "ezcast_public.h"

#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1)
	#define WIFI_DIRECT_NEW_FLOW 1
#else
	#define WIFI_DIRECT_NEW_FLOW 0
#endif

#if WIFI_DIRECT_NEW_FLOW
#include "wpa_ctrl.h"
#include "ifc_actions.h"
#include <poll.h>
#include <sys/stat.h>
#endif

#define WIFI_DIRECT_JSON    1

#define WIFI_DIRECT_PROFILE_LIST "/tmp/profiles.list"
#define WIFI_DIRECT_PROFILE_LIST_TMP "/tmp/profilestmp.list"
#define WIFI_DIRECT_PROFILE_LIST_DISK "/mnt/user1/profiles.list"

#if 0 //EZCAST_ENABLE
	#define WIFI_DIRECT_CLIENT_ENABLE 1 		   //enable p2p+client mode or not
	#define WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT 1    //keep client working always in p2p+client mode or not
	#define WIFI_STA_P2PAUTOGO_ENABLE 1 	   //enable STA+P2P AUTO GO function
#else
	#define WIFI_DIRECT_CLIENT_ENABLE 0            //enable p2p+client mode or not
	#define WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT 0    //keep client working always in p2p+client mode or not
	#define WIFI_STA_P2PAUTOGO_ENABLE 0        //enable STA+P2P AUTO GO function
#endif
#define WIFI_DIRECT_SOFTAP_ENABLE 0              //enalbe p2p+softap mode or not

#define WIFI_DIRECT_HOSTAPD_DEBUG 0 			  //added for printing hostapd debug info
#define WPA_DEBUG 0
#define WIFI_DIRECT_PING 0
#define WIFI_DIRECT_TCPDUMP 0

#if MODULE_CONFIG_EZCAST5G_ENABLE
#define WIFI_DIRECT_INIT_INTENT 13
#define WIFI_DIRECT_INIT_LCH 11
#define WIFI_DIRECT_DNSMASQ 1
#else
#define WIFI_DIRECT_INIT_INTENT 0
#define WIFI_DIRECT_INIT_LCH 6
#define WIFI_DIRECT_DNSMASQ 0
#endif
//>>> P2P hostapd config file path.
#define WIFI_DIRECT_HOSTAPD_CONF "/tmp/wfd/rtl_hostapd.conf"
#if WIFI_DIRECT_DNSMASQ
#define WIFI_DIRECT_UDHCPD_CONF "/tmp/wfd/dnsmasq.conf"
#else
#define WIFI_DIRECT_UDHCPD_CONF "/tmp/wfd/udhcpd.conf"
#endif
#define WIDI_MODE_NORMAL		0
#define WIDI_MODE_P2PAUTOGO		1
#define WIDI_MODE_MIX			2

extern int wifi_stop_process();
extern int softap_start();

#if WIFI_DIRECT_CLIENT_ENABLE
#define WIFI_DIRECT_INTF "wlan1"
#else
#define WIFI_DIRECT_INTF "wlan0"
#endif

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
extern realtek_wifi_dongle_type_emun realtek_dongle_type_divided;

#if EZCAST_LITE_ENABLE
	enum{
		P2P_NULL = -1,
		P2P_WLAN0,
		P2P_WLAN1
	};

	enum{
		MIXED_STATUS_NULL = -1,
		MIXED_STATUS_CLIENT_SOFTAP,
		MIXED_STATUS_CLIENT_P2P,
		MIXED_STATUS_P2P_SOFTAP,
		MIXED_STATUS_P2P_WLAN0,
		MIXED_STATUS_P2P_WLAN1,
		MIXED_STATUS_MAX
	};

	enum{
		MIXED_MODE_NULL = -1,
		MIXED_MODE_CLIENT_SOFTAP,
		MIXED_MODE_CLIENT_P2P,
		MIXED_MODE_P2P_SOFTAP,
		MIXED_MODE_P2P,
		MIXED_MODE_P2P_CLOSE,
		MIXED_MODE_P2P_OPEN,
		MIXED_MODE_MAX
	};
#endif

#ifdef WIFI_DIRECT_JSON
//rtsp_info
#define RTSP_PARAMETER "/tmp/rtsp_parameter.txt"
#define RTSP_OUT    "/tmp/rc_out.txt"
#define RTSP_SERVERNAME "/tmp/rtsp_servername.txt"
#define RTSP_HDCP "/tmp/hdcp.txt"
#define RTSP_HDCP_DECL_SUPPORT "/tmp/hdcp_decl_support.txt"
#define RTSP_HDCP_NEGO_OVER "/tmp/hdcp_nego_over.txt"
#define RTSP_HDCP_KEY_SOURCE "/tmp/hdcp_key_source.txt"
#define	MIRACAST_H264DEC_ERROR_INFO	"/tmp/miracast_h264dec_error.info"
#define MPEGTS    "/tmp/mpegts.info"

#define WIFI_DIRECT_JSON_FILE "/mnt/user1/miracast.json"          //the path of json file
#define ARRAY_MAX 20
#define CODE_SIZE 60
int jsonWriteFlag = 0;
#endif

typedef enum{
	LAN_ROUTING=1,
	WIFI_ROUTING,
};
#define AP_MAX_NUM 16
#define WIDI_BUF_LEN 256
#define P2P_IOARRAY_LEN 16

#define SCAN_OK 1
#define SCAN_DEF 0

#if WIFI_DIRECT_NEW_FLOW
typedef enum __WIFI_DIRECT_JSON_STATE{
	JSON_OK = 0,
	JSON_ERR = -1,
    JSON_ERROR_OPEN_FILE = 5,
	JSON_ERROR_MALLOC_FAIL,
	JSON_ERROR_GET_JSON
}WIFI_DIRECT_JSON_STATE;

#define WIFI_P2P_IP_CONFIG_FILE		"/mnt/vram/wifi/p2p_ip_info.conf"
#define CONFIG_P2P_IP				"InetAddr"
#define CONFIG_P2P_MASK				"Mask"
#define CONFIG_P2P_GATEWAY			"Gateway"
#endif
#define debug_info(fmt,arg...) // printf("---%s---%d---"fmt"---\n",__FUNCTION__,__LINE__,##arg)

extern int bestChannelChosen;         //added for p2p host by amos
extern int channel_p2p;
static int _wifi_client(struct p2p *p,int save_profile);
static int wifi_direct_scan_results();
static int  wifi_direct_obtain_hostname(char *hostname);
static int _to_lower_case(char *str);
void wifi_direct_concurrent_close_client_softap_fun();
static int widi_connect_status();
#if WIFI_DIRECT_NEW_FLOW
static int mixed_mode_change(const int new_mode);
#endif

char wifi_direct_apbuf[256];
char widi_device_name[48];
struct p2p action_p2p;
int wpa_ap_num;
static int wifi_direct_mode = WIDI_MODE_NORMAL;
#if EZCAST_LITE_ENABLE
	#define MODE_CHANGE_WAIT_TIME	(2)
	
	int p2p_current_port = P2P_NULL;
	int mixed_current_status = MIXED_STATUS_NULL;
	int mixed_current_mode = MIXED_MODE_NULL;

	int change_to_client_softap = -1;
	static int mixed_mode_p2p_status = 0;
#endif
#if EZCAST_ENABLE
	static int getIpResult = -1;
	static int getPeerIpStart = 0;
	static int doNegoResult = -1;
	static int doNegoStart = 0;
#endif

struct  msg_buf   
{  
    long  mtype;  
    int   msg_data;  
};  
static int msg_id = -1;

static void __dump_p2p(struct p2p *p)
{


    printf("[%s:%s:%d]-[%s-%s]-[zhoudayuan-2015615]\n",__FILE__, __func__, __LINE__, __DATE__, __TIME__);
    printf("********************** begin dump ****************\n");
    printf("**ifname:%s\n",p->ifname);
    printf("**status:%d\n",p->status);
    printf("**dev_name:%s\n",p->dev_name);
    printf("**intent:%d\n",p->intent);
    printf("**listen_ch:%d\n",p->listen_ch);
    printf("**wps_info:%d\n",p->wps_info);
    printf("**wpsing:%d\n",p->wpsing);
    printf("**pin:%d\n",p->pin);
    printf("**role:%d\n",p->role);
    printf("**start_role:%d\n",p->start_role);
    printf("**apd_ssid:%s\n",p->apd_ssid);
    printf("**peer_devaddr:%s\n",p->peer_devaddr);
    printf("**peer_ifaddr:%s\n",p->peer_ifaddr);
    printf("**mac_connect_ok:%s\n",p->mac_connect_ok);
    printf("**op_ch:%d\n",p->op_ch);
    printf("**wpa_open:%d\n",p->wpa_open);
    printf("**ap_open:%d\n",p->ap_open);
    printf("**pthread:%d\n",p->pthread);
    printf("**pthread_go:%d\n",p->pthread_go);
    printf("**pthread_msg:%d\n",p->pthread_msg);
    printf("**pbc_flag:%d\n",p->pbc_flag);
    printf("**pthread_runing:%d\n",p->pthread_runing);
    printf("**pthread_go_runing:%d\n",p->pthread_go_runing);
    printf("**phtread_msg_runing:%d\n",p->phtread_msg_runing);
    printf("**thread_poll_gc:%d\n",p->thread_poll_gc);
    printf("**thread_go_gc:%d\n",p->thread_go_gc);
    printf("**thread_msg_gc:%d\n",p->thread_msg_gc);
    printf("**ap_total:%d\n",p->ap_total);
    printf("**scan_ok_flag:%d\n",p->scan_ok_flag);
    printf("**connect_go:%d\n",p->connect_go);
    printf("**polling_status_flage:%d\n",p->polling_status_flage);
    printf("**no_sta_connected:%d\n",p->no_sta_connected);
    printf("**connect_flag:%d\n",p->connect_flag);
    printf("**prov_req_cm:%d\n",p->prov_req_cm);
    printf("********************** end dump ****************\n");
}

static void __wifi_direct_sleep(int sec)
{
    int err;
    struct timespec req,rem;

    req.tv_sec=sec;
    req.tv_nsec=0;

__re_sleep:
    err = nanosleep(&req,&rem);
    if(err){
         if(errno == EINTR){
              req.tv_sec=rem.tv_sec;
              req.tv_nsec=rem.tv_nsec;
              goto __re_sleep;
         }
         else{
              printf(">> %s error:%d\n",__FUNCTION__,err);
         }
    }

    return;
}

//#define WIFI_DIRECT_USE_VFORK_FOR_SYSTEM

#ifdef WIFI_DIRECT_USE_VFORK_FOR_SYSTEM
pthread_mutex_t _wifi_direct_sys_locker;
#endif

static int __wifi_direct_system(const char *cmdstring)
{
#ifdef WIFI_DIRECT_USE_VFORK_FOR_SYSTEM
    pid_t pid,wpid;
    int status;
    static int flag=0;

    //pthread_mutex_lock(&_wifi_direct_sys_locker);
    
    if(cmdstring == NULL){
         return 1;
    }
    
    if((pid = vfork()) < 0){
         status = -1;
    }
    else if(pid == 0){
         /** child process */
         execl("/bin/sh","sh","-c",cmdstring,(char *)0);
         _exit(127);
    }
    else{
         printf("*****:%s,%d,f=%d\n",cmdstring,pid,flag);
         while((wpid = waitpid(pid,&status,0)) < 0){
              if(errno != EINTR){
                   printf(">>>> errno is %d\n",errno);
                   status = -1;
                   break;
              }
         }
         printf("====%d,f=%d\n",wpid,flag);
         flag++;
    }
    //pthread_mutex_unlock(&_wifi_direct_sys_locker);
    return(status);
#else
    return system(cmdstring);
#endif
}

static void wifi_direct_sem_wait(sem_t * sem)
{
    int err;

__PEND_REWAIT:
    err = sem_wait(sem);
    if(err == -1){
         int errsv = errno;
         if(errsv == EINTR){
              //Restart if interrupted by handler
              goto __PEND_REWAIT;     
         }
         else{
              //printf("work_sem_pend: errno:%d\n",errsv);
              return;
         }
    }

    return;
    
}

static void wifi_direct_sem_post(sem_t * sem)
{
    int err;
    err = sem_post(sem);
}

static int APMac_Hash (char *str)
{
    unsigned int hash = 0;
    int i;
    
    for (i = 0; *str; i++){
         if ((i & 1) == 0){
              hash ^= ((hash << 7) ^(*str++) ^ (hash >> 3));
         } 
         else {
              hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
         }
    }
    return (hash & 0x7FFFFFFF);
}

#ifdef WIFI_DIRECT_JSON
/**
*write the info got from driver into the json file
*/
static int createJsonFile(struct p2p *p,JSON *json,char *error){
         JSON *json_device = NULL;
         char *json_buf;
         FILE *json_fd = NULL;
         char *json_string = NULL;
         int string_size;
         int ret;
         FILE *fd = NULL;
         char rtsp_ser[50] = {0};          //servername
         char rtsp_para[1024] = {0};     //parameter
         char rtsp_out[256] = {0};          //exit code
         char hdcp_s[4] = {0};          //hdcp used or not
         char hdcp_decl_support_s[4] = {0};     //support hdcp or not
         char hdcp_nego_over_s[4] = {0};          //hdcp authen or not
         char hdcp_key_source[20] = {0};          //hdcp key source
         char mpegts_info[256] = {0};          //mpegts info
         int hdcp = 0;                    //hdcp
         int hdcp_decl_support = 0;
         int hdcp_nego_over = 0;
         int array_size = 0;               
                   
//         printf("---%s---%d----\n",__FUNCTION__,__LINE__);
         
         json_device = JSON_CreateObject();          //create a json device

         JSON_AddNumberToObject(json_device,"p2p_role",p->role);                    //add role into the item
         
         JSON_AddNumberToObject(json_device,"p2p_request_status",p->status_for_json);     //add status into the item
         
         JSON_AddNumberToObject(json_device,"p2p_op_channel",p->op_ch);          //add op_ch into the item
         
         JSON_AddStringToObject(json_device,"p2p_device_mac",p->peer_ifaddr);          //add mac into the item
         
#if EZCAST_ENABLE
         if(access(MIRACAST_DEVICEINFO_PATH, F_OK)==0){
              fd = fopen(MIRACAST_DEVICEINFO_PATH, "r");
              if(fd != NULL){
                   deviceMac_t macAddr;
                   memset(&macAddr, 0, sizeof(macAddr));
                   fread(&macAddr, 1, sizeof(macAddr), fd);
                   fclose(fd);
                   if(macAddr.iSet == MIRACAST_DEVICEINFO_ISET){
                        JSON_AddStringToObject(json_device, "app_device_mac", macAddr.mac);
                   }
              }
              if(access(MIRACAST_HDCPDISABLE_PATH, F_OK)!=0){                // If miracast hdcp disable flag file has been deleted, then delete miracast device info file, else not delete it
                   unlink(MIRACAST_DEVICEINFO_PATH);
              }
         }
#endif
         JSON_AddStringToObject(json_device,"p2p_device_name",widi_device_name);     //add device name into the item
         
         JSON_AddStringToObject(json_device,"p2p_wps_method",p->req_cm);          //add req_cm into the item
         /***add rtsp parameters***/
         if(access(RTSP_PARAMETER,F_OK)==0){
              fd = fopen(RTSP_PARAMETER,"r");
              if(fd != NULL){
                   memset(rtsp_para,0,sizeof(rtsp_para));
                   fread(rtsp_para,1,sizeof(rtsp_para),fd);
              //     printf("---%s---%d----rtsp_para is %s\n",__FUNCTION__,__LINE__,rtsp_para);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_PARAMETER);
         }
         JSON_AddStringToObject(json_device,"rtsp_parameter",rtsp_para);          //add rtsp server name into the item
         /***add rtsp exit code***/
         if(access(RTSP_OUT,F_OK)==0){
              fd = fopen(RTSP_OUT,"r");
              if(fd != NULL){
                   memset(rtsp_out,0,sizeof(rtsp_out));
                   fread(rtsp_out,1,sizeof(rtsp_out),fd);
              //     printf("---%s---%d----rtsp_out is %s\n",__FUNCTION__,__LINE__,rtsp_out);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_OUT);
         }
         JSON_AddStringToObject(json_device,"rtsp_exit_code",rtsp_out);          //add rtsp server name into the item
         /****add rtsp servername****/
         if(access(RTSP_SERVERNAME,F_OK)==0){
              fd = fopen(RTSP_SERVERNAME,"r");
              if(fd != NULL){
                   memset(rtsp_ser,0,sizeof(rtsp_ser));
                   fread(rtsp_ser,1,sizeof(rtsp_ser),fd);
              //     printf("---%s---%d----rtsp_ser is %s\n",__FUNCTION__,__LINE__,rtsp_ser);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_SERVERNAME);
         }
         JSON_AddStringToObject(json_device,"rtsp_server_name",rtsp_ser);          //add rtsp server name into the item
         /****add rtsp hdcp****/
         if(access(RTSP_HDCP,F_OK)==0){
              fd = fopen(RTSP_HDCP,"r");
              if(fd != NULL){
                   memset(hdcp_s,0,sizeof(hdcp_s));
                   fread(hdcp_s,1,sizeof(hdcp_s),fd);
                   hdcp = atoi(hdcp_s);
                   if(hdcp !=0 && hdcp !=1)
                        hdcp = 0;
              //     printf("---%s---%d----hdcp is %d\n",__FUNCTION__,__LINE__,hdcp);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_HDCP);
         }
         
         JSON_AddNumberToObject(json_device,"hdcp",hdcp);                         //add hdcp into the item
         /****add hdcp_decl_support*****/
         if(access(RTSP_HDCP_DECL_SUPPORT,F_OK)==0){
              fd = fopen(RTSP_HDCP_DECL_SUPPORT,"r");
              if(fd != NULL){
                   memset(hdcp_decl_support_s,0,sizeof(hdcp_decl_support_s));
                   fread(hdcp_decl_support_s,1,sizeof(hdcp_decl_support_s),fd);
                   hdcp_decl_support = atoi(hdcp_decl_support_s);
         //          printf("---%s---%d----hdcp_decl_support is %d\n",__FUNCTION__,__LINE__,hdcp_decl_support);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_HDCP_DECL_SUPPORT);
         }
         
         JSON_AddNumberToObject(json_device,"hdcp_decl_support",hdcp_decl_support);          //add hdcp_decl_support into the item
         /****add hdcp_nego_over*****/
         if(access(RTSP_HDCP_NEGO_OVER,F_OK)==0){
              fd = fopen(RTSP_HDCP_NEGO_OVER,"r");
              if(fd != NULL){
                   memset(hdcp_nego_over_s,0,sizeof(hdcp_nego_over_s));
                   fread(hdcp_nego_over_s,1,sizeof(hdcp_nego_over_s),fd);
                   hdcp_nego_over = atoi(hdcp_nego_over_s);
         //          printf("---%s---%d----hdcp_nego_over is %d\n",__FUNCTION__,__LINE__,hdcp_nego_over);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_HDCP_NEGO_OVER);
         }
         
         JSON_AddNumberToObject(json_device,"hdcp_nego_over",hdcp_nego_over);               //add hdcp_nego_over into the item
         /****add hdcp_key_source*****/
         if(access(RTSP_HDCP_KEY_SOURCE,F_OK)==0){
              fd = fopen(RTSP_HDCP_KEY_SOURCE,"r");
              if(fd != NULL){
                   memset(hdcp_key_source,0,sizeof(hdcp_key_source));
                   fread(hdcp_key_source,1,sizeof(hdcp_key_source),fd);
         //          printf("---%s---%d----hdcp_key_source is %s\n",__FUNCTION__,__LINE__,hdcp_key_source);
                   fclose(fd);
                   fd = NULL;
              }
              remove(RTSP_HDCP_KEY_SOURCE);
         }
         
         JSON_AddStringToObject(json_device,"hdcp_key_source",hdcp_key_source);               //add hdcp_key_source into the item     
#if EZCAST_ENABLE
         /****add mpegts*****/
         if(access(MPEGTS,F_OK)==0){
              fd = fopen(MPEGTS,"r");
              if(fd != NULL){
                   memset(mpegts_info,0,sizeof(mpegts_info));
                   fread(mpegts_info,1,sizeof(mpegts_info),fd);
         //          printf("---%s---%d----hdcp_key_source is %s\n",__FUNCTION__,__LINE__,hdcp_key_source);
                   fclose(fd);
                   fd = NULL;
              }
              remove(MPEGTS);
         }
         
         JSON_AddStringToObject(json_device,"mpegts_info",mpegts_info);               //add mpegts info into the item     
#endif

         JSON_AddStringToObject(json_device,"failed_code",error);                    //add error into the item

         /****add miracast h264 dec error infomation*****/
         if(access(MIRACAST_H264DEC_ERROR_INFO,F_OK)==0){
              fd = fopen(MIRACAST_H264DEC_ERROR_INFO,"r");
              if(fd != NULL){
                   memset(mpegts_info,0,sizeof(mpegts_info));
                   fread(mpegts_info,1,sizeof(mpegts_info),fd);
         //          printf("---%s---%d----hdcp_key_source is %s\n",__FUNCTION__,__LINE__,hdcp_key_source);
                   fclose(fd);
                   fd = NULL;
              }
              remove(MIRACAST_H264DEC_ERROR_INFO);
         }
         JSON_AddStringToObject(json_device,"video",mpegts_info);               //add miracast h264 dec error info into the item     
         
         if(json == NULL){
              json = JSON_CreateArray();                                             //create a json array file
         }
         
         array_size = JSON_GetArraySize(json);

         if(array_size >= ARRAY_MAX){
              JSON_DeleteItemFromArray(json,0);
         }
         printf("[%s,%d]:array_size is %d\n",__FUNCTION__,__LINE__,array_size);
         JSON_AddItemToArray(json,json_device);                                   //add the item into the array
    
         json_device = NULL;
         
         json_fd = fopen(WIFI_DIRECT_JSON_FILE,"w+");
         if(json_fd ==NULL){
              printf("---%s---%d--open error!--\n",__FUNCTION__,__LINE__);
              JSON_Delete(json);
              return -1;
         }
         json_string = JSON_Print(json);                                             //change into string 
    //     printf("---%s---%d----json_string is %s\n",__FUNCTION__,__LINE__,json_string);
         if(json_string != NULL){
              string_size = strlen(json_string);
              printf("[%s,%d]:string_size is %d \n",__FUNCTION__,__LINE__,string_size);
              if(fwrite(json_string,1,string_size,json_fd)<0){                              //write into json file
                   printf("---%s---%d--write error!--\n",__FUNCTION__,__LINE__);
                   ret = -1;
              }
              ezJSON_free(json_string);                                                       //must free because json_string is malloced in JSON_Print 
              json_string = NULL;
              ret = 0;
         }
         else{
              printf("---%s---%d----json_print error!\n",__FUNCTION__,__LINE__);
              ret = -1;
         }
         JSON_Delete(json);                                                            //must delete
         fflush(json_fd);
         fsync(fileno(json_fd));
         fclose(json_fd);
         return ret;
    }


int writeToJsonFile(struct p2p *p2p, char *error_tmp){
    JSON *json = NULL;
    char *json_string = NULL;
    FILE *json_fd = NULL;
    int json_size;
    int string_size;
    char *json_buf=NULL;
    int ret = 0;

    if(access(WIFI_DIRECT_JSON_FILE,F_OK)!=0)     {                    //if the json file is exist
         createJsonFile(p2p,NULL,error_tmp);
    }
    else{                                                            //the json file is unexist
         json_fd = fopen(WIFI_DIRECT_JSON_FILE,"r+");
         if(json_fd == NULL){
              printf("----%s---%d--open json file error!----------\n",__FUNCTION__,__LINE__);
              remove(WIFI_DIRECT_JSON_FILE);
              return -1;
         }

         fseek(json_fd,0,SEEK_END);
         json_size = ftell(json_fd);
         fseek(json_fd,0,SEEK_SET);

         json_buf = malloc(json_size);          //malloc the buffer for json file

         if(json_buf != NULL){
              if((string_size=fread(json_buf,1,json_size,json_fd))<0){
                   printf("---%s---%d----read json file error or malloc error!------\n",__FUNCTION__,__LINE__);
                   free(json_buf);
                   fclose(json_fd);
                   remove(WIFI_DIRECT_JSON_FILE);
                   return -1;
              }
         }
         fclose(json_fd);
         if((json = JSON_Parse(json_buf)) == NULL){
              free(json_buf);
              printf("---%s---%d----parse json file error!------\n",__FUNCTION__,__LINE__);
              remove(WIFI_DIRECT_JSON_FILE);
              return -1;
         }
         free(json_buf);
         createJsonFile(p2p,json,error_tmp);
         
    }
                   
    return 0;
}

#endif
/************************************/


/*
*function for setting private command to wireless device instead of iwpriv tool
*/
static void iwprivClose(int skfd){
    if(skfd != -1){
         debug_info("close socket!!!---skfd is %d",skfd);
         iw_sockets_close(skfd);
         debug_info("close socket!!!---skfd is %d",skfd);
    }
}

static int setPrivateCmd(int skfd,              /*socket*/
                             char *args,               /* Command line args */
                             char *ifname,               /* Dev name */
                             char *cmdname,          /* Command name*/
                             iwprivargs * priv,          /* Private ioctl description */
                             int          priv_num,     /* Number of descriptions */
                             u_char *data)               /*the data get from driver*/     
{
    struct iwreq     wrq;
    u_char     buffer[4096];     /* Only that big in v25 and later */
    int          i = 0;          /* Start with first command arg */
    int          k;          /* Index in private description table */
    int          temp;
    int          subcmd = 0;     /* sub-ioctl index */
    int          offset = 0;     /* Space for sub-ioctl index */
    int           count = 1;
    /* Check if we have a token index.
    * Do it now so that sub-ioctl takes precedence, and so that we
    * don't have to bother with it later on... */
    
    //printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
    if((count >= 1) && (sscanf(args, "[%i]", &temp) == 1))
    {
         subcmd = temp;
         args++;
         count--;
    }

    /* Search the correct ioctl */
    k = -1;
    while((++k < priv_num) && strcmp(priv[k].name, cmdname));

    /* If not found... */
    if(k == priv_num)
    {
         fprintf(stderr, "Invalid command : %s\n", cmdname);
         return(-1);
    }
//           printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
    /* Watch out for sub-ioctls ! */
    if(priv[k].cmd < SIOCDEVPRIVATE)
    {
         int     j = -1;

         /* Find the matching *real* ioctl */
         while((++j < priv_num) && ((priv[j].name[0] != '\0') || (priv[j].set_args != priv[k].set_args) || (priv[j].get_args != priv[k].get_args)));

         /* If not found... */
         if(j == priv_num)
         {
              fprintf(stderr, "Invalid private ioctl definition for : %s\n",
              cmdname);
              return(-1);
         }

         /* Save sub-ioctl number */
         subcmd = priv[k].cmd;
         /* Reserve one int (simplify alignment issues) */
         offset = sizeof(__u32);
         /* Use real ioctl definition from now on */
         k = j;
    }
//    printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);

    /* If we have to set some data */
    if((priv[k].set_args & IW_PRIV_TYPE_MASK) &&(priv[k].set_args & IW_PRIV_SIZE_MASK))
    {
         switch(priv[k].set_args & IW_PRIV_TYPE_MASK){
              case IW_PRIV_TYPE_BYTE:
                   /* Number of args to fetch */
                   wrq.u.data.length = count;
                   if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
                        wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

                   /* Fetch args */
                   for(; i < wrq.u.data.length; i++) {
                        sscanf(args[i], "%i", &temp);
                        buffer[i] = (char) temp;
                   }
              break;

              case IW_PRIV_TYPE_INT:
                   /* Number of args to fetch */
//                   printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
                   wrq.u.data.length = count;
                   if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
                        wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

                   /* Fetch args */
                   for(; i < wrq.u.data.length; i++) {
                        sscanf(args[i], "%i", &temp);
                        ((__s32 *) buffer)[i] = (__s32) temp;
                   }
              break;

              case IW_PRIV_TYPE_CHAR:
                   if(i < count)
                   {
                        /* Size of the string to fetch */
//                        printf("-----%s-----%d-----count is %d\n",__FUNCTION__,__LINE__,count);
                        wrq.u.data.length = strlen(args) + 1;
                        if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
                             wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

                        /* Fetch string */
                        memcpy(buffer, args, wrq.u.data.length);
                        buffer[sizeof(buffer) - 1] = '\0';
                        i++;
//                        printf("%s\n", buffer);
//                        printf("-----%s-----%d-----count is %d\n",__FUNCTION__,__LINE__,count);
                   }
                   else
                   {
                        printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
                        wrq.u.data.length = 1;
                        buffer[0] = '\0';
                   }
              break;

              case IW_PRIV_TYPE_FLOAT:
                   /* Number of args to fetch */
                   printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
                   wrq.u.data.length = count;
                   if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
                        wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

                   /* Fetch args */
                   for(; i < wrq.u.data.length; i++) {
                        double          freq;
                        if(sscanf(args[i], "%lg", &(freq)) != 1)
                        {
                             printf("Invalid float [%s]...\n", args[i]);
                             return(-1);
                        }    
                        if(strchr(args[i], 'G')) freq *= GIGA;
                        if(strchr(args[i], 'M')) freq *= MEGA;
                        if(strchr(args[i], 'k')) freq *= KILO;
                        sscanf(args[i], "%i", &temp);
                        iw_float2freq(freq, ((struct iw_freq *) buffer) + i);
                   }
              break;

              case IW_PRIV_TYPE_ADDR:
                   /* Number of args to fetch */
                   printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
                   wrq.u.data.length = count;
                   if(wrq.u.data.length > (priv[k].set_args & IW_PRIV_SIZE_MASK))
                        wrq.u.data.length = priv[k].set_args & IW_PRIV_SIZE_MASK;

                   /* Fetch args */
                   for(; i < wrq.u.data.length; i++) {
                        if(iw_in_addr(skfd, ifname, args[i],((struct sockaddr *) buffer) + i) < 0)
                        {
                             printf("Invalid address [%s]...\n", args[i]);
                             return(-1);
                        }
                   }
              break;

              default:
                   printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
                   fprintf(stderr, "Not implemented...\n");
                   return(-1);
         }
      
         if((priv[k].set_args & IW_PRIV_SIZE_FIXED) &&(wrq.u.data.length != (priv[k].set_args & IW_PRIV_SIZE_MASK)))
         {
              printf("The command %s needs exactly %d argument(s)...\n",
              cmdname, priv[k].set_args & IW_PRIV_SIZE_MASK);
              return(-1);
         }
    }     /* if args to set */
    else
    {
         wrq.u.data.length = 0L;
    }
    strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
//    printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);

  /* Those two tests are important. They define how the driver
   * will have to handle the data */
    if((priv[k].set_args & IW_PRIV_SIZE_FIXED) &&((iw_get_priv_size(priv[k].set_args) + offset) <= IFNAMSIZ))
    {
         printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
         /* First case : all SET args fit within wrq */
         if(offset)
         wrq.u.mode = subcmd;
         memcpy(wrq.u.name + offset, buffer, IFNAMSIZ - offset);
    }
    else
    {
         if((priv[k].set_args == 0) &&(priv[k].get_args & IW_PRIV_SIZE_FIXED) &&(iw_get_priv_size(priv[k].get_args) <= IFNAMSIZ))
         {
              printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
              /* Second case : no SET args, GET args fit within wrq */
              if(offset)
              wrq.u.mode = subcmd;
         }
         else
         {
              /* Third case : args won't fit in wrq, or variable number of args */
              wrq.u.data.pointer = (caddr_t) buffer;
              wrq.u.data.flags = subcmd;
         }
    }

    /* Perform the private ioctl */
    if(ioctl(skfd, priv[k].cmd, &wrq) < 0)
    {
         fprintf(stderr, "Interface doesn't accept private ioctl...\n");
         fprintf(stderr, "%s (%X): %s\n", cmdname, priv[k].cmd, strerror(errno));
         return(-1);
    }
//    printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);

    /* If we have to get some data */
    if((priv[k].get_args & IW_PRIV_TYPE_MASK) &&(priv[k].get_args & IW_PRIV_SIZE_MASK))
    {
         int     j;
         int     n = 0;          /* number of args */

//         printf("%-8.16s  %s:", ifname, cmdname);

         /* Check where is the returned data */
         if((priv[k].get_args & IW_PRIV_SIZE_FIXED) && (iw_get_priv_size(priv[k].get_args) <= IFNAMSIZ))
         {
              printf("-------%s---%d--------\n",__FUNCTION__,__LINE__);
              memcpy(buffer, wrq.u.name, IFNAMSIZ);
              n = priv[k].get_args & IW_PRIV_SIZE_MASK;
         }
         else{
//              printf("-------%s---%d--------\n",__FUNCTION__,__LINE__);
              n = wrq.u.data.length;
         }
         switch(priv[k].get_args & IW_PRIV_TYPE_MASK)
         {
              case IW_PRIV_TYPE_BYTE:
                   /* Display args */
                   for(j = 0; j < n; j++)
                   printf("%d  ", buffer[j]);
                   printf("\n");
              break;

              case IW_PRIV_TYPE_INT:
                   /* Display args */
                   for(j = 0; j < n; j++)
                   printf("%d  ", ((__s32 *) buffer)[j]);
                   printf("\n");
              break;

              case IW_PRIV_TYPE_CHAR:
                   /* Display args */
                   buffer[n] = '\0';
              break;

              case IW_PRIV_TYPE_FLOAT:
                   {
                        double          freq;
                        /* Display args */
                        for(j = 0; j < n; j++)
                        {
                             freq = iw_freq2float(((struct iw_freq *) buffer) + j);
                             if(freq >= GIGA)
                                  printf("%gG  ", freq / GIGA);
                             else
                                  if(freq >= MEGA)
                                       printf("%gM  ", freq / MEGA);
                                  else
                                       printf("%gk  ", freq / KILO);
                        }
                        printf("\n");
                   }
              break;

              case IW_PRIV_TYPE_ADDR:
                   {
                        char          scratch[128];
                        struct sockaddr *     hwa;
                        /* Display args */
                        for(j = 0; j < n; j++)
                        {
                             hwa = ((struct sockaddr *) buffer) + j;
                             if(j)
                                  printf("           %.*s", (int) strlen(cmdname), "                ");
                             //printf("%s\n", iw_saether_ntop(hwa, scratch));
                        }
                   }
              break;

              default:
                   printf("-------%s---%d--------\n",__FUNCTION__,__LINE__);
                   fprintf(stderr, "Not yet implemented...\n");
                   return(-1);
         }
         strcpy(data,buffer);
//         printf("data is %s",data);
    }     /* if args to set */
    return(0);
}

static int iwprivExecute(int skfd,char *ifname, char *action, char *command,u_char *data){
    iwprivargs *     priv;
    int          number;          /* Max of private ioctl */
    int          ret;
//    printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
    /* Read the private ioctls */
    number = iw_get_priv_info(skfd, ifname, &priv);

    /* Is there any ? */
    if(number <= 0){
         /* Should I skip this message ? */
         fprintf(stderr, "%-8.16s  no private ioctls.\n\n",ifname);
         if(priv)
              free(priv);
         return(-1);
    }

    /* Do it */
//         printf("-----%s-----%d-----\n",__FUNCTION__,__LINE__);
    ret = setPrivateCmd(skfd,command, ifname, action,priv, number,data);

    free(priv);
    return(ret);
}

/********************************/
static int _wifi_direct_copy_profile(struct p2p *p2p,char *from, char *to)
{
    FILE *fp_from,*fp_to;
    char buf[256] = {0};
    char keyWord[20] = {0};
    char *ptr = NULL;

    fp_from = fopen(from,"rb");
    if(fp_from == NULL){
         printf(">>>>[%s,%d]:open %s error\n",__FUNCTION__,__LINE__,from);
         return -1;
    }

    fp_to = fopen(to,"wb");
    if(fp_to == NULL){
         printf(">>>>[%s,%d]:open %s error\n",__FUNCTION__,__LINE__,to);
         fclose(fp_from);
         return -1;
    }
    sprintf(keyWord,"1%s",p2p->peer_ifaddr);
    _to_lower_case(keyWord);
//    printf("--%s---%d---keyWord is %s\n",__FUNCTION__,__LINE__,keyWord);
    while(!feof(fp_from)){
         ptr = fgets(buf,256,fp_from);
         //printf("---buf----%s",buf);
         if(ptr && (strstr(buf,keyWord)==NULL)){
              fputs(buf, fp_to);
         }
    }
    fflush(fp_to);
    fsync(fileno(fp_to));
    fclose(fp_to);
    fclose(fp_from);
    
    return 0;
}

static int wifi_direct_send_msg(int widi_cmd)
{
    int ret = -1;
    struct p2p *snd_msg=NULL;
    struct msg_buf  mdata ;
    
    snd_msg = &action_p2p;
    mdata.mtype = msg_id;
    
    switch(widi_cmd){
         case WIDI_CMD_SCAN:
              printf("WIDI_CMD_SCAN\n");
              break;
         case WIDI_CMD_WPA_SCAN:
              printf("WIDI_CMD_WPA_SCAN\n");
              break;
         case WIDI_CMD_CONNECT:
              printf("WIDI_CMD_CONNECT\n");
              break;               
         case WIDI_CMD_STOPCONNECT:
              printf("WIDI_CMD_STOPCONNECT\n");
              break;
         case WIDI_CMD_PROV:
              printf("WIDI_CMD_STOPSCAN\n");
              break;               
         default:
              printf("unkown cmd\n");
              break;     
    }
    mdata.msg_data= widi_cmd;
    msgsnd(msg_id,&mdata,sizeof(mdata.msg_data),0);     
    wifi_direct_sem_wait(&(snd_msg->msg.cmd_sem));
    
    return ret;
}

static int wifi_direct_init_syn_lock(struct p2p *p)
{
    int rtn = 0;
    
    if(sem_init(&p->syn_lock.sem_role,0,1)==-1){
         p2p_err("Sem init error");
         rtn = -1;
         goto wifi_syn_lock_out;
    }

    // init to one that can be used.
    if(sem_init(&p->syn_lock.sem_status,0,1)==-1){
         sem_destroy(&p->syn_lock.sem_role);
         rtn = -1;
         p2p_err("Sem init error");
         goto wifi_syn_lock_out;
    }
    
wifi_syn_lock_out:
    return rtn;
}

static int wifi_direct_exit_syn_lock(struct p2p *p)
{
    int rtn = 0;
    
    sem_destroy(&p->syn_lock.sem_role);
    sem_destroy(&p->syn_lock.sem_status);
    
    return rtn;
}

static int _wifi_direct_save_profile_to_disk(struct p2p *p)
{
    char buf[128];

    sprintf(buf,"cp -f %s %s",WIFI_DIRECT_PROFILE_LIST,WIFI_DIRECT_PROFILE_LIST_DISK);
    printf("profile save to disk -----> %s\n",buf);
    __wifi_direct_system(buf);

    return 0;
}

static int _wifi_direct_get_profile_from_disk(struct p2p *p)
{
    char buf[128];

    if(access(WIFI_DIRECT_PROFILE_LIST_DISK,F_OK)!=-1)
    {
         sprintf(buf,"cp -f %s %s",WIFI_DIRECT_PROFILE_LIST_DISK,WIFI_DIRECT_PROFILE_LIST);
         __wifi_direct_system(buf);
    }

    return 0;
}

static int wifi_direct_init(struct p2p *p)
{
    char hostname[33]={0x00};

#if EZCAST_LITE_ENABLE
	if(wifi_direct_mode == WIDI_MODE_MIX){
	 	if(p2p_current_port == P2P_WLAN0)
	    	strcpy(p->ifname,"wlan0");
		else
	    	strcpy(p->ifname,"wlan1");
	}else{
#endif
	strcpy(p->ifname,WIFI_DIRECT_INTF);
#if EZCAST_LITE_ENABLE
	}
#endif
    p->status = P2P_STATE_NONE;
    p->status_for_json = P2P_STATE_NONE;                    //status for json
    p->skfd = iw_sockets_open();
	p->intent = WIFI_DIRECT_INIT_INTENT;  
    p->pin = 12345670;
 //   p->role = P2P_ROLE_DISABLE;
   p->role = P2P_ROLE_DISABLE;
    // default wps info to PBC
    //p->wps_info=3;
    p->wpa_open=0;
    p->ap_open=0;
    strcpy( p->peer_devaddr, "00:00:00:00:00:00" );
    p->connect_flag = WIDI_CONNECT_DEFAULT;
    strcpy( p->peer_ifaddr, "00:00:00:00:00:00" );
    p->pbc_flag  = 0;
    p->ap_total = 0; //ap total
    p->scan_ok_flag = 0;
    p->phtread_msg_runing = 0;
    p->pthread_runing = 0;
    p->thread_poll_gc = 0;
    p->thread_go_gc = 0;
    p->thread_msg_gc = 0;
    p->polling_status_flage=0;
#if MODULE_CONFIG_EZCAST5G_ENABLE
	p->listen_ch=WIFI_DIRECT_INIT_LCH;
#else
	if(channel_p2p)
		p->listen_ch =channel_p2p;
	else
		p->listen_ch=WIFI_DIRECT_INIT_LCH;
#endif
    p->op_ch = 6;
    strcpy(p->req_cm ,"WIDI_PIN_PBC_NONE");               //add for miracast info collection ,the value will get from prov_req_cm
    p->prov_req_cm = WIDI_PIN_PBC_NONE;
	if(strlen(p->dev_name) == 0 || strlen(p->dev_name) > sizeof(p->dev_name)){
		EZCASTLOG("p->dev_name: %s\n", p->dev_name);
		ezcast_get_device_name(hostname,sizeof(hostname));
		EZCASTLOG("hostname: %s\n", hostname);
		memcpy(p->dev_name,hostname,strlen(hostname));
		strcpy( p->mac_connect_ok, "00:00:00:00:00:00" );
	}
    pthread_mutex_init(&p->locker_for_prov_req_cm, NULL);
    wifi_direct_init_syn_lock(p);
    
    return 0;
}

/* 
* @brief get the P2P status
*/
int wifi_direct_status(struct p2p *p)
{
    FILE *pf=NULL;
    char parse[CMD_SZ]={0x00};
    char cmd[CMD_SZ]={0x00};
    char *find_status=NULL;

	wifi_direct_sem_wait(&p->syn_lock.sem_status);
	
//    printf("----%s---%d---p2p_get status------\n",__FUNCTION__,__LINE__);
    iwprivExecute(p->skfd,p->ifname,"p2p_get","status",cmd);
//    printf("cmd is %s\n",cmd);
//    debug_info("p->skfd=====%d",p->skfd);
    find_status= strstr(cmd,"Status=");
    if(find_status != NULL){
         p->status = atoi(find_status+7);
    }
//    printf("----%s---%d---p->status  is  %d------\n",__FUNCTION__,__LINE__,p->status );
    wifi_direct_sem_post(&p->syn_lock.sem_status);

    //printf(">>> status:%d\n",p->status);
    
    return p->status;
}

/* 
* @brief get the P2P role
*/
int wifi_direct_role(struct p2p *p)
{
    FILE *pf=NULL;
    char tmp[CMD_SZ]={0x00};
    char cmd[CMD_SZ]={0x00};
    char *find_role=NULL;
    
    wifi_direct_sem_wait(&p->syn_lock.sem_role);
    
    memset( cmd, 0x00, CMD_SZ );
    /*
    sprintf(cmd, "iwpriv %s p2p_get role > /tmp/amwd_role.txt", p->ifname);
    __wifi_direct_system(cmd);
    
    pf = fopen("/tmp/amwd_role.txt", "r" );
    if (pf){
         while( !feof( pf ) ){
              memset( tmp, 0x00, CMD_SZ );
              fgets( tmp, CMD_SZ, pf );
              find_role=strstr( tmp, "Role=");
              if( find_role ){
                   p->role = atoi( find_role+5);
                   //p2pdbg_info(" role=%d\n",p->role);
                   break;
              }     
         }
         fclose( pf );
    }
    */
   // debug_info("p->skfd=====%d",p->skfd);
    iwprivExecute(p->skfd,p->ifname,"p2p_get","role",cmd);
  //  debug_info("p->skfd=====%d",p->skfd);
//    printf("cmd is %s\n",cmd);
    find_role=strstr( cmd, "Role=");
    if( find_role ){
         p->role = atoi( find_role+5);
         //p2pdbg_info(" role=%d\n",p->role);
    }     
//    printf("----%s---%d---p->role  is  %d------\n",__FUNCTION__,__LINE__,p->role);
    wifi_direct_sem_post(&p->syn_lock.sem_role);
    
    return p->role;
}

/* 
* @brief set wps information to driver.
*/
void wifi_direct_set_wpsinfo(struct p2p *p)
{
    char cmd[50]={0x0};

//    sprintf(cmd, "iwpriv %s p2p_set got_wpsinfo=%d", p->ifname, p->wps_info);
//    __wifi_direct_system(cmd);
    debug_info("p->skfd=====%d",p->skfd);
    sprintf(cmd, "got_wpsinfo=%d",p->wps_info);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
    debug_info("p->skfd=====%d",p->skfd);
    return;
}

/*
* @brief set the wifi drivers intent values
*/
int wifi_direct_set_intent(struct p2p *p)
{
    int ret = -1;
    char cmd[50]={0x00};
    
//    sprintf( cmd, "iwpriv %s p2p_set intent=%d", p->ifname, p->intent);
//    ret = __wifi_direct_system( cmd );
    debug_info("p->skfd=====%d",p->skfd);
    sprintf(cmd, "intent=%d",p->intent);
    ret=iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
    debug_info("p->skfd=====%d",p->skfd);
    if(ret != 0){
         goto flag_err;
    }
    
    return ret;
    
flag_err:
    p2pdbg_info("set intent error\n");
    return -1;
}

/*
* @brief get wifi direct softap p2p_hostapd.conf information
*/
static void wifi_direct_get_hostapd_conf(struct p2p *p)
{
    FILE *pf = NULL;
    char tmp[CMD_SZ]={0x00};
    int i,len;
    
    pf = fopen(WIFI_DIRECT_HOSTAPD_CONF, "r");     
    if (pf){
         while(!feof(pf)){
              memset(tmp, 0x00, CMD_SZ);
              fgets(tmp, CMD_SZ, pf);
              if(strncmp(tmp, "ssid=", 5) == 0)
              {
                   strcpy( p->apd_ssid, tmp+5 );
                   /** strip the newline character  */
                   len = strlen(p->apd_ssid);
                   for(i=0;i<len;i++){
                        if(*(p->apd_ssid+i)=='\n'){
                             *(p->apd_ssid+i) = 0;
                             break;
                        }
                   }
              }
              else if(strncmp(tmp, "channel=", 8) == 0)
              {
                   p->op_ch = atoi( tmp+8 );
              }
              else if(strncmp(tmp, "device_name=", 12) == 0)
              {
                   break;
              }
         }     
         fclose( pf );
    }
    
}

/*
* @brief set the P2P softap ssid
*/
static int wifi_direct_set_softap_ssid(struct p2p *p)
{
    char cmd[64]={0x00};
//    sprintf( cmd, "iwpriv %s p2p_set ssid=%s ", p->ifname, p->apd_ssid);
//    __wifi_direct_system( cmd );

    sprintf(cmd, "ssid=%s",p->apd_ssid);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);

    return 1;
}

/*
* @brief set the P2P listen channel.
*/
static int wifi_direct_set_listenchannel(struct p2p *p,int channel)
{
	char cmd[48]={0x00};

    sprintf(cmd, "listen_ch=%d",channel);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);

    return 1;
}


static void __do_change_opch(int channel)
{
    FILE *pfin = NULL;
    FILE *pfout = NULL;
    char cmd[CMD_SZ]={0};
    int chok=0,modeok=0,ht_capabok=0;
    
    pfin = fopen( WIFI_DIRECT_HOSTAPD_CONF, "r" );
    if(pfin == NULL){
         printf(">>> open %s error\n",WIFI_DIRECT_HOSTAPD_CONF);
         return;
    }
    
    pfout = fopen( "/tmp/p2p_hostapd_temp.conf", "w" );
    if(pfout == NULL){
         printf("open /tmp/p2p_hostapd_temp.conf error\n");
         fclose(pfin);
         return;
    }
    
    while(!feof(pfin)){
         memset(cmd, 0x00, CMD_SZ);
         fgets(cmd, CMD_SZ, pfin);

         if(strncmp(cmd, "channel=", 8) == 0)
         {
              memset(cmd, 0x00, CMD_SZ);
              sprintf(cmd, "channel=%d\n",channel);
              fputs(cmd, pfout);
              chok = 1;
         }
         else if(strncmp(cmd, "hw_mode=", 8) == 0)
         {
              memset(cmd, 0x00, CMD_SZ);
              if(channel>14){
                   sprintf(cmd, "hw_mode=a\n");
              }
              else{
                   sprintf(cmd, "hw_mode=g\n");
              }
              fputs(cmd, pfout);
              modeok = 1;
         }
         else if(strncmp(cmd, "ht_capab=",9) == 0)
         {
         #if WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT          //HT40 is useful for p2p connecting if the client keep alive in p2p + concurrent mode
              if(((channel >= 5) && (channel <=13)) || channel==40 || channel==48 || channel==56 || channel==64){
                   fputs("ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40-]\n", pfout);
              }
              else if(((channel >= 1) && (channel <=7)) || channel==36 || channel==44 || channel==52 || channel==60){
                   fputs("ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40+]\n", pfout);
              }
         #else
              fputs("ht_capab=[SHORT-GI-20]\n", pfout);
         #endif
              ht_capabok = 1;
         }
         else{
              fputs(cmd, pfout);
         }
    }

    if(chok == 0){
         memset(cmd, 0x00, CMD_SZ);
         sprintf(cmd, "channel=%d\n",channel);
         fputs(cmd, pfout);
    }

    if(modeok == 0){
         memset(cmd, 0x00, CMD_SZ);
         if(channel>14){
              sprintf(cmd, "hw_mode=a\n");
         }
         else{
              sprintf(cmd, "hw_mode=g\n");
         }
         fputs(cmd, pfout);
    }

    /**
    * Note from Realtek:
    * channel 5~13,40,48,56,64 use [HT40-]
    * channel 1~7,36,44,52,60 use [HT40+]
    */
    if(ht_capabok == 0){
         
    #if WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT          //HT40 is useful for p2p connecting if the client keep alive in p2p + concurrent mode
         if(((channel >= 5) && (channel <=13)) || channel==40 || channel==48 || channel==56 || channel==64){
              fputs("ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40-]\n", pfout);
         }
         else if(((channel >= 1) && (channel <=7)) || channel==36 || channel==44 || channel==52 || channel==60){
              fputs("ht_capab=[SHORT-GI-20][SHORT-GI-40][HT40+]\n", pfout);
         }
    #else
         fputs("ht_capab=[SHORT-GI-20]\n", pfout);
    #endif
    }
    
    fclose(pfout);
    fclose(pfin);

    sprintf(cmd, "rm -rf %s\n",WIFI_DIRECT_HOSTAPD_CONF);
    __wifi_direct_system(cmd);

    sprintf(cmd, "mv /tmp/p2p_hostapd_temp.conf %s\n",WIFI_DIRECT_HOSTAPD_CONF);
    __wifi_direct_system(cmd);
    
    __wifi_direct_system( "rm -rf /tmp/p2p_hostapd_temp.conf");
    
    return;
}



/**
* @brief call this after we have been negotiated to be GO and before hostapd start.
*/
static int wifi_direct_change_op_channel(struct p2p *p)
{
    FILE *fp=NULL;
    char tmp[CMD_SZ]={0x00};
    char cmd[CMD_SZ]={0x00};
    char *find_ch=NULL;
    int channel = -1;
    char str[ 100 ],ch[50];
    
    memset(cmd,0x00,CMD_SZ);
    /*
    sprintf(cmd, "iwpriv %s p2p_get op_ch > /tmp/amwd_op_ch.txt", p->ifname);
    __wifi_direct_system(cmd);
    
    fp = fopen("/tmp/amwd_op_ch.txt", "r" );
    if (fp){
         while( !feof( fp ) ){
              memset( tmp, 0x00, CMD_SZ );
              fgets( tmp, CMD_SZ, fp );
              find_ch=strstr( tmp, "Op_ch=");
              if( find_ch ){
                   channel = atoi(find_ch+6);
                   break;
              }     
         }
         fclose( fp );
    }
*/
    debug_info("p->skfd=====%d",p->skfd);
    iwprivExecute(p->skfd,p->ifname,"p2p_get","op_ch",cmd);
    debug_info("p->skfd=====%d",p->skfd);
    find_ch=strstr( cmd, "Op_ch=");
    if( find_ch ){
         channel = atoi(find_ch+6);
    }     

    if(channel != -1){
         printf(">>> operation channel is %d\n",channel);
         p->op_ch = channel;
		if(access("/sys/module/8821au",F_OK)==0){
			//channel=p->op_ch_5G;
			//printf("-----------------channel:%d\n",channel);
		}
         __do_change_opch(channel);
    }

    return 0;
}


/*
* @brief set the operation channel.
*/
static void wifi_direct_set_opchannel(struct p2p *p)
{
	char cmd[32]={0x00};
//	p->op_ch_5G=149;
	p->op_ch_5G= 40;		//just for test
	//    sprintf(cmd, "iwpriv %s p2p_set op_ch=%d", p->ifname, p->op_ch);
	//    __wifi_direct_system( cmd );
	if(access("/sys/module/8821au",F_OK)==0 || access("/sys/module/8821cu",F_OK)==0){
	#if 0
		int channel=1; 
		sprintf(cmd, "force_op_ch=%d",channel);
		iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
		usleep(5);
		printf("------%s-----------\n",__func__);
    #endif
		printf(">>>>>>>>>>>> do not set force_op_ch\n");
		printf(">>>>>>>>>>>> set op_ch = %d\n",p->op_ch_5G);
		memset(cmd,0x00,32);
		sprintf(cmd, "op_ch=%d",p->op_ch_5G);
		iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
	}else{
		p->op_ch = 11;
		sprintf(cmd, "op_ch=%d",p->op_ch);
		iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
	}
}

static int wifi_direct_set_wfd_type(struct p2p *p, int wfd_type)
{
    char cmd[48]={0x00};

//    sprintf( cmd, "iwpriv %s p2p_set wfd_type=%d", p->ifname, wfd_type);
//    __wifi_direct_system(cmd);

    sprintf(cmd, "wfd_type=%d",wfd_type);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);

    
    return 1;
}

/**
*let other miracast devices know the WIFI is ready for miracast connection or not
*/
static int wifi_direct_set_wfd_sa(struct p2p *p,int sa_enable)
{
    char cmd[48]={0x00};
    
//    sprintf( cmd, "iwpriv %s p2p_set sa=%d", p->ifname, sa_enable);
//    __wifi_direct_system(cmd);

    sprintf(cmd, "sa=%d",sa_enable);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);

    
    return 1;
}

static int wifi_direct_set_wfd_pc(struct p2p *p)
{
    int handle =1;
    char cmd[32]={0x00};

//    sprintf( cmd, "iwpriv %s p2p_set pc=%d", p->ifname, handle);
//    __wifi_direct_system(cmd);

    debug_info("p->skfd=====%d",p->skfd);
    sprintf(cmd, "pc=%d",handle);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
    debug_info("p->skfd=====%d",p->skfd);
    return 1;
}

/*
* When receive provision discovery request,
* it can show which device address that are connected.
*/
static int wifi_direct_peer_devaddr(struct p2p *p)
{
    FILE *pf = NULL;
    char addr_12[12] = { 0x00 };
    int i;
    char cmd[48]={0x00};
    char tmp[CMD_SZ]={0x00};
/*    
    sprintf( cmd, "iwpriv %s p2p_get peer_deva > /tmp/amwd_devaddr.txt", p->ifname);
    __wifi_direct_system(cmd);
    
    pf = fopen("/tmp/amwd_devaddr.txt","r");
    if(pf){
         while(!feof(pf)){
              memset(tmp,0x00,CMD_SZ);     
              fgets( tmp, CMD_SZ, pf );
              fgets( tmp, CMD_SZ, pf );
              strncpy(addr_12, tmp, 12 );
                   
              for(i=0; i<6; i++)
              {
                   p->peer_devaddr[3*i] = addr_12[2*i];
                   p->peer_devaddr[3*i+1] = addr_12[2*i+1];

                   if(i==5)
                        p->peer_devaddr[3*i+2] = '\0';
                   else
                        p->peer_devaddr[3*i+2] = ':';
              }
              break;
         }     
         fclose(pf);
    }
    */
  //  debug_info("p->skfd=====%d",p->skfd);

    iwprivExecute(p->skfd,p->ifname,"p2p_get","peer_deva",tmp);
//    printf("tmp is %s",tmp);
 //   debug_info("p->skfd=====%d",p->skfd);

    strncpy(addr_12, tmp+1, 12 );
//    printf("addr_12 is %s",addr_12);
    for(i=0; i<6; i++)
    {
         p->peer_devaddr[3*i] = addr_12[2*i];
         p->peer_devaddr[3*i+1] = addr_12[2*i+1];

         if(i==5)
              p->peer_devaddr[3*i+2] = '\0';
         else
              p->peer_devaddr[3*i+2] = ':';
    }
    
    //printf("%s %d>>>>>>>>>>>peer_devaddr:%s\n",__func__,__LINE__,p->peer_devaddr);
    return 0;     
}


/*
* get the peer device cmp
*/
int wifi_direct_peer_req_cm(struct p2p *p, char *peer_req_cm)
{
    FILE *pf = NULL;
    char cmd[32]={0x00};
//    char tmp[CMD_SZ]={0x00};
    char *tmp=NULL;
    /*
    sprintf( cmd, "iwpriv %s p2p_get req_cm > /tmp/amwd_cm.txt", p->ifname);
    __wifi_direct_system(cmd);
    
    pf = fopen("/tmp/amwd_cm.txt","r");
    if(pf){
         while( !feof( pf ) ){
              //memset( tmp, 0x00, CMD_SZ );
              fgets( tmp, CMD_SZ, pf );
              if( strncmp( tmp, "CM", 2) == 0 )
              {
                   strncpy(peer_req_cm, tmp+3, 3 );
                   printf(">>>>>>>>>>>>>>>req_cm:%s\n",peer_req_cm);
                   break;
              }     
         }     
         fclose( pf );
    }
    */
    debug_info("p->skfd=====%d",p->skfd);
    iwprivExecute(p->skfd,p->ifname,"p2p_get","req_cm",cmd);
    debug_info("p->skfd=====%d",p->skfd);
    if( (tmp=strstr( cmd, "CM"))!= NULL )
    {
         strncpy(peer_req_cm, tmp+3, 3 );
         printf(">>>>>>>>>>>>>>>req_cm:%s\n",peer_req_cm);
    }     
    return 0;
}

 /*
 * get the all ap information
 */
int wifi_read_all_ap(struct p2p *p)
{
    int sta_count = 0,rtn=0;
    FILE *pf;
    char tmp[CMD_SZ]={0x00};
    char cmd[64]={0x00};
    
    //get all connect  ap total
    sprintf( cmd, "hostapd_cli -p/tmp/hostapd all_sta > /tmp/amwd_all_sta.txt");
    __wifi_direct_system(cmd);
    
    pf = fopen("/tmp/amwd_all_sta.txt" , "r");
    if(pf){
         while(!feof(pf)){
              memset(tmp, 0x00,CMD_SZ);
              fgets(tmp,CMD_SZ,pf);
              if(strncmp( tmp, "dot11RSNAStatsSTAAddress=", 25) == 0){
                   sta_count++;
                   //if( p->no_sta_connected == sta_count)
                   //     rtn =1;
              }
         }
         fclose(pf);
    }
    
    if(sta_count!=0){
         rtn=1;
    }
    return rtn;
}

/**
* scan and find whether a device with  interface address "devaddr" exists or not.
*/
static int wifi_direct_scan_and_find_dev(struct p2p *p,char *ifa)
{
    FILE *p2p_fd =NULL;
    char parse[CMD_SZ] = {0x00};
    int find=0;
    
    wifi_direct_scan(p);

    __wifi_direct_system("cat /tmp/amwd_scan.txt");
    
    p2p_fd=fopen("/tmp/amwd_scan.txt","r");
    if(p2p_fd==NULL){
         return -1;
    }
    else{
         while(!feof( p2p_fd )){
              memset(parse,0x00,CMD_SZ);
              fgets(parse,CMD_SZ,p2p_fd);     
              if(strstr(parse,ifa)){
                   find = 1;
                   break;
              }
         }
         fclose(p2p_fd);
    }     

    if(find == 1){
         return 0;
    }
    
    return -1;
}

#if WIFI_DIRECT_NEW_FLOW
static int wifi_direct_start_do_nego_pthread(int method);
static int wifi_direct_join_persistant_group(struct p2p *p);
#endif
/*
* monitor the wifi driver status,if p->status=P2P_STATE_GONEGO_OK , then nego is ok;
*/
static  void *wifi_direct_polling_status(void *arg)
{
    struct p2p *p=(struct p2p*)arg;
    char peer_req_cm[8];
    int i;
    char cmd[32]={0x00};
    while( 1){
         wifi_direct_status(p);
         if((p->polling_status_flage==0)&&((p->status == P2P_STATE_RX_PROVISION_DIS_REQ)||(p->status == P2P_STATE_RECV_INVITE_REQ_MATCH)||
              (p->status == P2P_STATE_RECV_INVITE_REQ_DISMATCH)||(p->status == P2P_STATE_RECV_INVITE_REQ_JOIN ) \
              ||(p->status == P2P_STATE_TX_INFOR_NOREADY ) || (p->status == P2P_STATE_GONEGO_OK)))
         {

              if(p->status == P2P_STATE_RECV_INVITE_REQ_DISMATCH){
                   printf(">>> status 17,continue\n");
                   //iwprivExecute(p->skfd,p->ifname,"p2p_set","profilefound=0",NULL);
				   usleep(50000);
                   continue;
              }
              else if(p->status == P2P_STATE_RECV_INVITE_REQ_JOIN){
                   printf(">>> status 19, invite to join a group\n");
					p->polling_status_flage=1;
                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_INVITED;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
                   p->status_for_json = p->status;                    //remember the connection start status
                   break;
              }
              else if(p->status == P2P_STATE_RECV_INVITE_REQ_MATCH){
                   printf(">>> status 12, recv invitation request frame\n");
				#if WIFI_DIRECT_NEW_FLOW
					mixed_mode_change(MIXED_MODE_P2P);
				#endif
					p->polling_status_flage=1;
                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_PERSISTENT_GP_INVITED;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
                   p->status_for_json = p->status;                    //remember the connection start status
                   wifi_direct_get_ifaddr(p);
                   strcpy(p->peer_devaddr,p->peer_ifaddr);
                   wifi_p2p_get_device_name(p,-1);
                   wifi_direct_peer_devaddr(p);
				#if WIFI_DIRECT_NEW_FLOW
					wifi_direct_join_persistant_group(p);
				#endif
                   break;
              }
              
              /**
              * Special Note: state P2P_STATE_RX_PROVISION_DIS_REQ sometimes cannot be captured and 
              *   the driver will transmit to P2P_STATE_TX_INFOR_NOREADY. So I have to add this into
              *   the branch condition test.
              */
              p->polling_status_flage=1;
              p->status_for_json = p->status;                         //remember the connection start status
              memset( peer_req_cm, 0x00, 8);
              wifi_direct_peer_devaddr(p);
              wifi_p2p_get_device_name(p,-1);
              wifi_direct_peer_req_cm(p, peer_req_cm);

              printf(">>>>>> peer cm:%s\n",peer_req_cm);
              
              if( (strncmp( peer_req_cm, "dis", 3) == 0) || (strncmp( peer_req_cm, "lab", 3) == 0) )//display pin code method
              {
                   printf("Here is your PIN, continue: %d %d\n", p->pin,p->status);
                   p->connect_flag = 2;

                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_DISPLAY;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
                   
              }
              else if( (strncmp( peer_req_cm, "pad", 3) == 0) ){     
                   //keypad method     
                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_INPUT;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
                   printf("<%s %d>Please insert peer PIN code:\n", __func__,__LINE__);                                   
              }
              else if((strncmp( peer_req_cm, "pbc", 3) == 0)){ 
                   //push button method
                   memset(widi_device_name,0x00,32);
                   wifi_p2p_get_device_name(p,-1);
                   p->pbc_flag = 1;

                   /**
                   * Sony Xperia Z must be host.
                   */
                   if(strstr(widi_device_name,"Xperia")){
                        p->intent = 0;
                        wifi_direct_set_intent(p);
                   }

                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_DEFAULT;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
                   printf("<%s %d>Please push b to accept:\n", __func__,__LINE__);
              }
			  #if WIFI_DIRECT_NEW_FLOW
			  wifi_direct_start_do_nego_pthread(p->prov_req_cm);
			  #endif
              break;
           }
         else{
			usleep(50000);
			/*
		#if EZCAST_LITE_ENABLE
			if(mixed_current_status != MIXED_MODE_CLIENT_SOFTAP && mixed_current_status != MIXED_MODE_NULL){
				//EZCASTLOG("P2P wait!!!\n");
				__wifi_direct_sleep(1);
			}
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
			__wifi_direct_sleep(1);
		#endif
		#endif
			*/
         }
    }
    p->pthread_runing = 0;
    p->thread_poll_gc = 1;
    //pthread_detach(pthread_self());
    pthread_exit(NULL);
}

/*
* If p2p device becomes GO, we still polling driver status
* to check whether some other p2p devices connected 
*/
static void *wifi_polling_client(void * arg)
{
    struct p2p *p=(struct p2p*)arg;
    char peer_devaddr[18];
    char peer_req_cm[8];     
    //pthread_detach(pthread_self());
    while(1){
         if( p->no_sta_connected > 0 && ( p->wpsing == _FALSE ) )
         {
              if( wifi_read_all_ap(p) == _FALSE )
              {
                   p->no_sta_connected--;
              }
         }
         wifi_direct_status(p);
         if(p->status == P2P_STATE_RX_PROVISION_DIS_REQ || p->status == P2P_STATE_GONEGO_FAIL || p->status == P2P_STATE_GONEGO_ING ){          
                   
              memset( peer_devaddr, 0x00, 18);
              memset( peer_req_cm, 0x00, 4);     
              wifi_direct_peer_devaddr(p);
              wifi_direct_peer_req_cm(p, peer_req_cm);
/*#if WIFI_STA_P2PAUTOGO_ENABLE
		if(wifi_read_all_ap(p) == 1){
		printf("this is have sta connected to p2p group ower\n");
		__wifi_direct_system("killall hostapd");
		__wifi_direct_system("killall udhcpd");
		usleep(8000);
		p->ap_open=_FALSE;
		p->udhcpd_flag =1;
		}
#endif*/
              if( (strncmp( peer_req_cm, "dis", 3) == 0) || (strncmp( peer_req_cm, "lab", 3) == 0) ){
                   printf("Here is your PIN: %d\n", p->pin);
              #if 0
                   p->wps_info=2;
                   wifi_direct_set_wpsinfo(p);
                   wifi_direct_status(p);     
              #endif
                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_DISPLAY;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
                   
              }
              else if( (strncmp( peer_req_cm, "pbc", 3) == 0) ){
			  	
                   printf("Please push b to accept:\n");
                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_DEFAULT;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
              }
              else if( (strncmp( peer_req_cm, "pad", 3) == 0) ){
                   printf("Please insert peer PIN code:\n");
                   pthread_mutex_lock(&p->locker_for_prov_req_cm);
                   p->prov_req_cm = WIDI_PIN_PBC_INPUT;
                   pthread_mutex_unlock(&p->locker_for_prov_req_cm);
              }
              break;
           }
         usleep( POLLING_INTERVAL );
    }
    p->pthread_go_runing = 0;
    p->thread_go_gc = 1;
    pthread_exit(NULL);
}

static void *wifi_direct_msg_thread(void *arg)
{
    int msg_queue=-1;
    struct p2p *msg_thread=NULL;
    struct msg_buf rcv_msg;
    
    msg_thread = &action_p2p;
    
    while(1){
         msgrcv(msg_id,&rcv_msg,sizeof(rcv_msg.msg_data),msg_id,0);
         msg_queue = rcv_msg.msg_data;
         switch(msg_queue){
              case WIDI_CMD_SCAN:
                   msg_thread->ap_total=wifi_direct_scan_results();
                   msg_thread->msg.widi_cmd_queue[0] = -1;
                   break;          
              case WIDI_CMD_CONNECT:
                   msg_thread->msg.widi_cmd_queue[0] = -1;
                   break;
              case WIDI_CMD_PROV:
                   msg_thread->msg.widi_cmd_queue[0] = -1;
                   break;                    
              case WIDI_CMD_STOPCONNECT:
                   break;                    
              default:
                   break;
         }
         wifi_direct_sem_post(&msg_thread->msg.cmd_sem);
    }

    pthread_exit(NULL);
    return 0;
}

/**
* wifi direct persistent function
*/
int wifi_driect_set_persistent(struct p2p *p,int id)
{
    char cmd[48]={0x00};
    
//    sprintf(cmd,"iwpriv %s p2p_set persistent=%d",p->ifname,id);
//    __wifi_direct_system(cmd);

    sprintf(cmd, "persistent=%d",id);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
 
    return 0;
}

int wifi_direct_set_profilefound_information(struct p2p *p)
{
    char cmd[256]={0x00};
    char buf[256]={0x00};
    FILE *fp;
    int i,len;

//    sprintf(cmd,"iwpriv %s p2p_set profilefound=0",p->ifname);
//    __wifi_direct_system(cmd);


    iwprivExecute(p->skfd,p->ifname,"p2p_set","profilefound=0",NULL);


    fp = fopen(WIFI_DIRECT_PROFILE_LIST,"r");
    if(fp == NULL){
         return -1;
    }

    while(!feof(fp)){
         memset(buf, 0x00, 256);
         fgets(buf, 256, fp);
         len = strlen(buf);
         if(len>19){
              for(i=0;i<len;i++){
                   if(buf[i]=='\n'){
                        buf[i] = 0;
                        break;
                   }
              }
              printf(">>> set profile:%s\n",buf);
//              sprintf(cmd,"iwpriv %s p2p_set profilefound=%s",p->ifname,buf);
//              __wifi_direct_system(cmd);

              debug_info("p->skfd=====%d",p->skfd);
              sprintf(cmd, "profilefound=%s",buf);
              iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
              debug_info("p->skfd=====%d",p->skfd);
         }
    }

    fclose(fp);
    
    return 0;
}

/**
* change the hostapd config file interface.
*/
static void wifi_direct_rename_hostapd_intf(struct p2p *p)
{
    FILE *pfin = NULL;
    FILE *pfout = NULL;
    char cmd[CMD_SZ]={0};
    
    pfin = fopen( WIFI_DIRECT_HOSTAPD_CONF, "r" );
    if(pfin == NULL){
         printf(">>> open %s error\n",WIFI_DIRECT_HOSTAPD_CONF);
         return;
    }
    
    pfout = fopen( "/tmp/p2p_hostapd_temp.conf", "w" );
    if(pfout == NULL){
         printf("open /tmp/p2p_hostapd_temp.conf error\n");
         fclose(pfin);
         return;
    }
    
    while(!feof(pfin)){
         memset(cmd, 0x00, CMD_SZ);
         fgets(cmd, CMD_SZ, pfin);

         if(strncmp(cmd, "interface=", 10) == 0)
         {
              memset(cmd, 0x00, CMD_SZ);
              sprintf(cmd, "interface=%s\n", p->ifname );
              fputs(cmd, pfout );
         }
         else{
              fputs(cmd, pfout);
         }
    }
    
    fclose(pfout);
    fclose(pfin);

    sprintf(cmd, "rm -rf %s\n",WIFI_DIRECT_HOSTAPD_CONF);
    __wifi_direct_system(cmd);

    sprintf(cmd, "mv /tmp/p2p_hostapd_temp.conf %s\n",WIFI_DIRECT_HOSTAPD_CONF);
    __wifi_direct_system(cmd);
    
    __wifi_direct_system( "rm -rf /tmp/p2p_hostapd_temp.conf" );
    
    return;
}


/**
* change the udhcpd config file interface.
*/
static void wifi_direct_rename_udhcpd_intf(struct p2p *p)
{
    FILE *pfin = NULL;
    FILE *pfout = NULL;
    char cmd[CMD_SZ]={0};
    
    pfin = fopen( WIFI_DIRECT_UDHCPD_CONF, "r" );
    if(pfin == NULL){
         printf(">>> open %s error\n",WIFI_DIRECT_UDHCPD_CONF);
         return;
    }
    
    pfout = fopen( "/tmp/p2p_udhcpd_temp.conf", "w" );
    if(pfout == NULL){
         printf("open /tmp/p2p_udhcpd_temp.conf error\n");
         fclose(pfin);
         return;
    }
    
    while(!feof(pfin)){
         memset(cmd, 0x00, CMD_SZ);
         fgets(cmd, CMD_SZ, pfin);

         if(strncmp(cmd, "interface", 9) == 0)
         {
              memset(cmd, 0x00, CMD_SZ);
#if WIFI_DIRECT_DNSMASQ
	     sprintf(cmd, "interface=%s\n", p->ifname );
#else
              sprintf(cmd, "interface    %s\n", p->ifname );
#endif
              fputs(cmd, pfout );
         }
         else{
              fputs(cmd, pfout);
         }
    }
    
    fclose(pfout);
    fclose(pfin);

    sprintf(cmd, "rm -rf %s\n",WIFI_DIRECT_UDHCPD_CONF);
    __wifi_direct_system(cmd);

    sprintf(cmd, "mv /tmp/p2p_udhcpd_temp.conf %s\n",WIFI_DIRECT_UDHCPD_CONF);
    __wifi_direct_system(cmd);
    
    __wifi_direct_system( "rm -rf /tmp/p2p_udhcpd_temp.conf" );
    
    return;
}

/*
*choose the wlan0 channel form ap_info for wlan1 opch setting
*/
static void wifi_direct_choose_opch_from_apinfo(void){

    FILE *fd = NULL;
    char buffer[1024]={0};
    char locate[10] = {0};
    char *locate1=NULL;
    char *locate2=NULL;
    char ap_info[36] = "/proc/net/rtl819xC/wlan0/ap_info";

    switch(realtek_dongle_type_divided){
         case REALTEK_DU:
              memset(ap_info,0,sizeof(ap_info));
              strcpy(ap_info,"/proc/net/rtl819xD/wlan0/ap_info");
              break;
         case REALTEK_8188EU:
              memset(ap_info,0,sizeof(ap_info));
              strcpy(ap_info,"/proc/net/rtl8188eu/wlan0/ap_info");
              break;
         case REALTEK_8192EU:
              memset(ap_info,0,sizeof(ap_info));
              strcpy(ap_info,"/proc/net/rtl8192eu/wlan0/ap_info");
              break;
	  	case REALTEK_8811AU:
              memset(ap_info,0,sizeof(ap_info));
              strcpy(ap_info,"/proc/net/rtl8821au/wlan0/ap_info");
              break;
		 case REALTEK_8821CU:
              memset(ap_info,0,sizeof(ap_info));
              strcpy(ap_info,"/proc/net/rtl8821cu/wlan0/ap_info");
              break;
         default :
              break;
    }
    printf("--------%s---%d-----ap_info path is %s-------\n",__FUNCTION__,__LINE__,ap_info);
    if(access(ap_info,F_OK)==0){
         if((fd=fopen(ap_info,"r"))!=NULL){
              memset(buffer,0,sizeof(buffer));
              if(fread(buffer,1,sizeof(buffer),fd)>0){
                   locate1 = strstr(buffer,"cur_channel=");
                   printf("---%s---%d---locate1 is %s\n",__FUNCTION__,__LINE__,locate1);
                   if(locate1 != NULL){                         
                        locate2 = strstr(locate1,",");
                        memset(locate,0,sizeof(locate));
                        strncpy(locate,locate1+12,locate2-locate1-12);
                        printf("---%s---%d---locate is %s\n",__FUNCTION__,__LINE__,locate);
                        bestChannelChosen = atoi(locate);
                        printf("---%s---%d---bestChannelChosen is %d\n",__FUNCTION__,__LINE__,bestChannelChosen);
                   }
                   fclose(fd);
              }
              else{
                   fclose(fd);
              }
         }
    }
}

/**
* @brief before star the wifi direct engine, init it first.
* @param role: init the role of the P2P device. P2P_DEVICE,P2P_CLIENT,P2P_SOFTAP
*/
int wifi_direct_initialize(int role)
{
//#ifdef WIFI_STA_P2PAUTOGO_ENABLE
//if(role != 1){
//#endif
    struct p2p *wifi_p2p=NULL;
    char cmd[128];

#if EZCAST_ENABLE
	doNegoStart = 0;
	getPeerIpStart = 0;
#endif

#ifdef WIFI_DIRECT_USE_VFORK_FOR_SYSTEM
    /** mutex for __wifi_direct_system() */
    pthread_mutex_init(&_wifi_direct_sys_locker, NULL);
#endif
printf("---------- %s %d role:%d\n",__FILE__,__LINE__,role);
    if(access("/tmp/hostapd",F_OK)==0){
		set_ap_close_flag(1);
#if !WIFI_DIRECT_SOFTAP_ENABLE
         __wifi_direct_system("killall hostapd");
#if WIFI_DIRECT_DNSMASQ
		 __wifi_direct_system("killall dnsmasq");
#endif
         __wifi_direct_system("killall udhcpd");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(get_netlink_status()==0)
	{


	}
		
#elif !WIFI_STA_P2PAUTOGO_ENABLE

#endif
         __wifi_direct_system("ifconfig wlan1 down");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	#define WIFI_DIRECT_DNSMASQ 0    //use udhcpd
	#define WIFI_DIRECT_UDHCPD_CONF "/tmp/wfd/udhcpd.conf"
	__wifi_direct_system("brctl delif br0 wlan1");
	__wifi_direct_system("ifconfig br0 down");
	__wifi_direct_system("brctl delbr br0");
	// softap_start24G_probox();
	// if(get_netlink_status())
	//	 enable_route_function("eth0","wlan3","192.168.168.1/24",LAN_ROUTING);
#endif

#endif
    }

    if(access("/tmp/wpa_supplicant",F_OK)==0){
#if !WIFI_DIRECT_CLIENT_ENABLE
         __wifi_direct_system("killall wpa_supplicant");
         __wifi_direct_system("killall udhcpc");
         __wifi_direct_system("ifconfig wlan0 down");
#endif
    }

	if(access("/tmp/amgw.txt",F_OK)==0){
		remove("/tmp/amgw.txt");
	}
    //>>> copy the hostapd and udhpcd config file to destination dir.
   if(access("/tmp/wfd/p2p_hostapd_01.conf",F_OK)!=0){  //bluse add to test stations + p2p auto go
    wifi_create_dir("/tmp/wfd");
#if WIFI_DIRECT_CLIENT_ENABLE    
    if(0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK))
         sprintf(cmd,"cp -f /etc/p2p_hostapd_5G_01.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
    else
         sprintf(cmd,"cp -f /etc/p2p_hostapd_01.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
#else
    if(0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK)){
		EZCASTLOG("------ copy 5g config\n");
         sprintf(cmd,"cp -f /etc/p2p_hostapd_5G.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
    }else{
		EZCASTLOG("------ copy normal config\n");
         sprintf(cmd,"cp -f /etc/p2p_hostapd.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
    }
#endif
    __wifi_direct_system(cmd);

#if WIFI_DIRECT_CLIENT_ENABLE
    sprintf(cmd,"cp -f /etc/udhcpd_01.conf %s",WIFI_DIRECT_UDHCPD_CONF);
#elif WIFI_DIRECT_DNSMASQ
		sprintf(cmd,"cp -f /etc/dnsmasq_dhcp.conf %s",WIFI_DIRECT_UDHCPD_CONF);  
#else    
    printf("---- copy /udhcpd.conf config\n");
    sprintf(cmd,"cp -f /etc/udhcpd.conf %s",WIFI_DIRECT_UDHCPD_CONF);     
#endif
    __wifi_direct_system(cmd);
   }
    //>>> do basic init
    //>>> TODO: refresh the op_ch.
    wifi_p2p = &action_p2p;
    wifi_direct_init(wifi_p2p);
    wifi_direct_get_hostapd_conf(wifi_p2p);
    //strcpy(wifi_p2p->apd_ssid,"DIRECT-xy");
    wifi_direct_rename_hostapd_intf(wifi_p2p);
    wifi_direct_rename_udhcpd_intf(wifi_p2p);
    _wifi_direct_get_profile_from_disk(wifi_p2p);
#if WIFI_DIRECT_CLIENT_ENABLE
    wifi_direct_choose_opch_from_apinfo();
#endif

    if(bestChannelChosen > 11 || bestChannelChosen < 1)
         bestChannelChosen = 6;
    wifi_p2p->op_ch= bestChannelChosen;

    if(msg_id==-1){
         msg_id = msgget(ftok(".",1000),IPC_CREAT|IPC_EXCL|0600);
    }
    sem_init(&wifi_p2p->msg.cmd_sem,0,0);

    //>>> set interface up.
    sprintf(cmd,"ifconfig %s 0.0.0.0 up",wifi_p2p->ifname);
    __wifi_direct_system(cmd);
    
    int excRet=-1;
  //  debug_info("wifi_p2p->skfd=====%d",wifi_p2p->skfd);
    excRet = iwprivExecute(wifi_p2p->skfd,wifi_p2p->ifname,"p2p_set","enable=0",NULL);
  //  debug_info("wifi_p2p->skfd=====%d",wifi_p2p->skfd);
	
#if   WIFI_STA_P2PAUTOGO_ENABLE
	wifi_p2p->start_role = P2P_ROLE_GO;
#else
	wifi_p2p->start_role = role;
#endif
//#if   WIFI_STA_P2PAUTOGO_ENABLE
//}
//#endif
    return 0;
}

/**
* @brief before star the wifi direct engine, init it first, and this function need select WLAN0 or WLAN1.
* @param role: init the role of the P2P device. P2P_DEVICE,P2P_CLIENT,P2P_SOFTAP
* @param port: 0: WLAN0, 1:WLAN1
*/
#if EZCAST_LITE_ENABLE
static int wifi_direct_initialize_select(int role, int port)
{
    struct p2p *wifi_p2p=NULL;
    char cmd[128];

#if EZCAST_ENABLE
	doNegoStart = 0;
	getPeerIpStart = 0;
#endif

#ifdef WIFI_DIRECT_USE_VFORK_FOR_SYSTEM
    /** mutex for __wifi_direct_system() */
    pthread_mutex_init(&_wifi_direct_sys_locker, NULL);
#endif
	p2p_current_port = port;
    if(access("/tmp/hostapd",F_OK)==0){
		if(port == P2P_WLAN1){
			__wifi_direct_system("killall hostapd");
			#if WIFI_DIRECT_DNSMASQ
			__wifi_direct_system("killall dnsmasq");
			#endif
			__wifi_direct_system("killall udhcpd");

			__wifi_direct_system("ifconfig wlan1 down");
		}else if(port == P2P_WLAN0){
			__wifi_direct_system("kill $(ps | grep hostapd | grep rtl_hostapd.conf | awk \'{print $1}\')");
		}
    }

    if(access("/tmp/wpa_supplicant",F_OK)==0){
		if(port == P2P_WLAN0){
			__wifi_direct_system("killall wpa_supplicant");
			__wifi_direct_system("killall udhcpc");
			__wifi_direct_system("ifconfig wlan0 down");
		}else if(port == P2P_WLAN1){
			__wifi_direct_system("kill $(ps | grep wpa_supplicant | grep wlan1 | awk \'{print $1}\')");
		}
    }

    if(access("/tmp/amgw.txt",F_OK)==0){
        remove("/tmp/amgw.txt");
    }

    //>>> copy the hostapd and udhpcd config file to destination dir.
    wifi_create_dir("/tmp/wfd");
	if(port == P2P_WLAN1){ 
		if(0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK))
			sprintf(cmd,"cp -f /etc/p2p_hostapd_5G_01.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
		else
			sprintf(cmd,"cp -f /etc/p2p_hostapd_01.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
	}else{
		if(0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK))
			sprintf(cmd,"cp -f /etc/p2p_hostapd_5G.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
		else
			sprintf(cmd,"cp -f /etc/p2p_hostapd.conf %s",WIFI_DIRECT_HOSTAPD_CONF);
	}
    __wifi_direct_system(cmd);

	if(port == P2P_WLAN1)	
		sprintf(cmd,"cp -f /etc/udhcpd_01.conf %s",WIFI_DIRECT_UDHCPD_CONF);
#if WIFI_DIRECT_DNSMASQ
		sprintf(cmd,"cp -f /etc/dnsmasq_dhcp.conf %s",WIFI_DIRECT_UDHCPD_CONF);  
#else
		sprintf(cmd,"cp -f /etc/udhcpd.conf %s",WIFI_DIRECT_UDHCPD_CONF);	  
#endif
    __wifi_direct_system(cmd);
    //>>> do basic init
    //>>> TODO: refresh the op_ch.
    wifi_p2p = &action_p2p;
    wifi_direct_init(wifi_p2p);
    wifi_direct_get_hostapd_conf(wifi_p2p);
    //strcpy(wifi_p2p->apd_ssid,"DIRECT-xy");
    wifi_direct_rename_hostapd_intf(wifi_p2p);
    wifi_direct_rename_udhcpd_intf(wifi_p2p);
    _wifi_direct_get_profile_from_disk(wifi_p2p);
	if(port == P2P_WLAN1)	
	    wifi_direct_choose_opch_from_apinfo();

    if(bestChannelChosen > 11 || bestChannelChosen < 1)
         bestChannelChosen = 6;
    wifi_p2p->op_ch= bestChannelChosen;
	EZCASTLOG("wifi_p2p->op_ch: %d\n", wifi_p2p->op_ch);

    if(msg_id==-1){
         msg_id = msgget(ftok(".",1000),IPC_CREAT|IPC_EXCL|0600);
    }
    sem_init(&wifi_p2p->msg.cmd_sem,0,0);

    //>>> set interface up.
    sprintf(cmd,"ifconfig %s 0.0.0.0 up",wifi_p2p->ifname);
    __wifi_direct_system(cmd);
    
    int excRet=-1;
    debug_info("wifi_p2p->skfd=====%d",wifi_p2p->skfd);
    excRet = iwprivExecute(wifi_p2p->skfd,wifi_p2p->ifname,"p2p_set","enable=0",NULL);
    debug_info("wifi_p2p->skfd=====%d",wifi_p2p->skfd);
    
    /*
    sprintf(cmd,"iwpriv %s p2p_set enable=0",wifi_p2p->ifname);
    __wifi_direct_system(cmd);
    */
#if  WIFI_STA_P2PAUTOGO_ENABLE  //bluse add to test station + p2p auto go
	wifi_p2p->start_role = P2P_ROLE_GO;
#else
    wifi_p2p->start_role = role;
#endif
    return 0;
}
#else
static int wifi_direct_initialize_select(int role, int port)
{
	return 0;
}
#endif

/* 
* enable the wifi direct function
* realtek wifi driver have device,client and group owner mode 
*/
int wifi_direct_start(int handle)
{
//#if  WIFI_STA_P2PAUTOGO_ENABLE
//if(handle != 1){
//#endif
    int ret = -1;
    char cmd[256]={0x00};
    struct p2p *wifi_p2p=NULL;
    char call_buf[128]={0x00};
    char cmd2[CMD_SZ]={0x00};
    int count;
    
    wifi_p2p = &action_p2p;
    
    //>>> overide the parameter.
#if  WIFI_STA_P2PAUTOGO_ENABLE
	handle = P2P_ROLE_GO;
	//wifi_p2p->ap_open =_TRUE;
#else
    handle = wifi_p2p->start_role;
#endif
    //>>> creat a thread to process scan or connetc message
    if(wifi_p2p->phtread_msg_runing==0){
         ret = pthread_create(&wifi_p2p->pthread_msg, NULL, &wifi_direct_msg_thread, (void *)wifi_p2p);
         if(ret !=0){
              printf("%d,%s,error: wifi_direct_msg_thread create failed\n",__LINE__,__FILE__);
              return -1;
         }
         wifi_p2p->phtread_msg_runing = 1;
    }

    sprintf(cmd,"enable=%d",handle);
    int excRet;
    excRet = iwprivExecute(wifi_p2p->skfd,wifi_p2p->ifname,"p2p_set",cmd,NULL);
 
    //__wifi_direct_sleep(1);

    //set wifi direct device name
    wifi_direct_set_device_name(wifi_p2p);
    wifi_direct_set_wfd_type(wifi_p2p,1);
    wifi_direct_set_listenchannel(wifi_p2p,wifi_p2p->listen_ch);
    //wifi_direct_set_opchannel(wifi_p2p);
    wifi_direct_set_profilefound_information(wifi_p2p);     
    wifi_direct_set_softap_ssid(wifi_p2p);
    wifi_driect_set_persistent(wifi_p2p,1);
    wifi_direct_set_wfd_sa(wifi_p2p,1);
    //wifi_direct_set_wpsinfo(wifi_p2p);
    
    if(handle==P2P_ROLE_DEVICE){
         //wifi_p2p->res= 0;
#if WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT         //keep client alive
         wifi_p2p->intent = 15;
#endif
         wifi_direct_set_intent(wifi_p2p);
         //wifi_direct_role(wifi_p2p);

         //sprintf(cmd,"iwpriv %s dbg 0x7f250000 1",wifi_p2p->ifname);
         //system(cmd);

         //>>> creat a thread to monitor p2p device status
         if(wifi_p2p->pthread_runing==0){
              ret = pthread_create(&wifi_p2p->pthread, NULL, &wifi_direct_polling_status, (void *)wifi_p2p);
              wifi_p2p->pthread_runing = 1;
              if(ret != 0){
                   printf("%d,%s,error: wifi_direct_polling_status create failed\n",__LINE__,__FILE__);
                   pthread_cancel(wifi_p2p->pthread_msg);
                   pthread_join(wifi_p2p->pthread_msg, NULL);
                   wifi_p2p->phtread_msg_runing = 0;
                   return -1;
              }
         }
         //wifi_direct_send_msg(WIDI_CMD_SCAN);//send scan ap message
         
    }
    else if(handle==P2P_ROLE_CLIENT){
         wifi_p2p->intent = 0;
         wifi_direct_status(wifi_p2p);
         wifi_direct_role(wifi_p2p);
         wifi_direct_set_intent(wifi_p2p);
    }
    else if(handle==P2P_ROLE_GO){ 
         //usleep(5000);
 #if WIFI_STA_P2PAUTOGO_ENABLE
 	wifi_p2p->intent = 15;
#else
	wifi_p2p->intent = 1;
#endif
         wifi_direct_status(wifi_p2p);
         wifi_direct_role(wifi_p2p);
         wifi_direct_set_intent(wifi_p2p);
         /** group owner, start the hostapd */
         if(wifi_p2p->ap_open != _TRUE)
         {
              memset(cmd2, 0x00, CMD_SZ );
#if WIFI_DIRECT_HOSTAPD_DEBUG
              sprintf(cmd2, "hostapd %s -dd -t &",WIFI_DIRECT_HOSTAPD_CONF);
#else
              sprintf(cmd2, "hostapd %s &",WIFI_DIRECT_HOSTAPD_CONF);
#endif
	//debug_info("----------\n");
              __wifi_direct_system(cmd2);

              /**
              * sleep is not ok, we must do some active check to make sure the hostapd
              * really starts ok.
              */
              sprintf(cmd2,"/tmp/hostapd/%s",wifi_p2p->ifname);
              count=0;
              while(count < 30){
                   usleep( HOSTAPD_INIT_TIME );
                   if(access(cmd2,F_OK)==0){
                        printf(">>> hostapd start ok\n");
                        break;
                   }
              }
#if  WIFI_STA_P2PAUTOGO_ENABLE
	#if EZCAST_ENABLE
	
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0&& MODULE_CONFIG_EZCASTPRO_MODE!=8075//probox 
		sprintf(cmd, "ifconfig %s 192.168.168.1", wifi_p2p->ifname);
	#else
		sprintf(cmd, "ifconfig %s 192.168.203.1", wifi_p2p->ifname);
	#endif	
		
	#else
		sprintf(cmd, "ifconfig %s 192.168.111.1", wifi_p2p->ifname);
	#endif
		__wifi_direct_system(cmd);
		usleep(5000);

		__wifi_direct_system("rm -rf /tmp/udhcpd.leases");
		__wifi_direct_system("touch /tmp/udhcpd.leases");
#if WIFI_DIRECT_DNSMASQ
		printf(">>>>>start dnsmasq\n");
		sprintf(cmd,"/sbin/dnsmasq -d -C %s &",WIFI_DIRECT_UDHCPD_CONF);
		__wifi_direct_system(cmd);
#else
		printf(">>>>>start udhcpd\n");
		sprintf(cmd,"udhcpd %s",WIFI_DIRECT_UDHCPD_CONF);
		__wifi_direct_system(cmd);
#endif
#endif
              wifi_p2p->ap_open = _TRUE;
         }
         ret = pthread_create(&wifi_p2p->pthread_go, NULL, &wifi_polling_client, (void *)wifi_p2p);
         wifi_p2p->pthread_go_runing = 1;

         printf(">>>>>>>>>>>> start as GO,role=%d\n",wifi_p2p->role);
    }
    else{
         //>>> disable p2p functions
    }
//#if  WIFI_STA_P2PAUTOGO_ENABLE
//}  
//#endif
    return 0;
}
/*
* disable wifi direct function ,stop server and cancel some process 
* if enable = 0,disable wifi direct functions
*/
int wifi_direct_func_stop()
{
    char call_buf[50];
    int handle = 0;
    int ret = -1;
    void *thread_ret =NULL;
    
    msgctl(msg_id,IPC_RMID,NULL);
    msg_id = -1;
    
//    sprintf(call_buf,"iwpriv %s p2p_set enable=0",action_p2p.ifname);
//    ret = __wifi_direct_system(call_buf);
    
    debug_info("action_p2p.skfd=====%d",action_p2p.skfd);
    ret=iwprivExecute(action_p2p.skfd,action_p2p.ifname,"p2p_set","enable=0",NULL);
    debug_info("action_p2p.skfd=====%d",action_p2p.skfd);

    if(ret != 0){
         //goto flag_err;
         printf("[%s][%d] -- iwprivExecute run error!!!\n", __func__, __LINE__);
    }
    iwprivClose(action_p2p.skfd);               //close the socket
    
    p2pdbg_info("role:%d\n",action_p2p.role);
    wifi_direct_exit_syn_lock(&action_p2p);
    if(action_p2p.role == 2){
         __wifi_direct_system("killall wpa_supplicant");
         __wifi_direct_system("killall udhcpc");
    }
    else if(action_p2p.role ==3){
#if EZCAST_LITE_ENABLE
		if(wifi_direct_mode == WIDI_MODE_MIX){
			__wifi_direct_system("ifconfig wlan0 0.0.0.0 up");
			__wifi_direct_system("ifconfig wlan1 0.0.0.0 up");          //reset the wlan0
		}else{
#endif
	#if WIFI_DIRECT_CLIENT_ENABLE
         __wifi_direct_system("ifconfig wlan1 0.0.0.0 up");
	#else
         __wifi_direct_system("ifconfig wlan0 0.0.0.0 up");          //reset the wlan0
	#endif
#if EZCAST_LITE_ENABLE
		}
#endif
         __wifi_direct_system("killall hostapd");
#if WIFI_DIRECT_DNSMASQ
		__wifi_direct_system("killall dnsmasq");
#endif
         __wifi_direct_system("killall udhcpd");
    }
    
    if(action_p2p.phtread_msg_runing == 1){
         pthread_cancel(action_p2p.pthread_msg);
         pthread_join(action_p2p.pthread_msg, NULL);
         action_p2p.phtread_msg_runing =0;
    }
    
    if(action_p2p.pthread_go_runing == 1){
         pthread_cancel(action_p2p.pthread_go);
         pthread_join(action_p2p.pthread_go, NULL);
         action_p2p.pthread_go_runing  = 0;
    }
    else{
         if(action_p2p.thread_go_gc){
              pthread_join(action_p2p.pthread_go, NULL);
              action_p2p.thread_go_gc = 0;
         }
    }
    
    if(action_p2p.pthread_runing == 1){
         pthread_cancel(action_p2p.pthread);
         pthread_join(action_p2p.pthread, NULL);
         action_p2p.pthread_runing = 0;
    }
    else{
         if(action_p2p.thread_poll_gc){
              pthread_join(action_p2p.pthread, NULL);
              action_p2p.thread_poll_gc = 0;
         }
    }

    pthread_mutex_destroy(&action_p2p.locker_for_prov_req_cm);
    sem_destroy(&action_p2p.msg.cmd_sem);
    action_p2p.connect_flag = WIDI_CONNECT_DEFAULT;
    memset(&action_p2p,0x00,sizeof(struct p2p));

    __wifi_direct_system("killall ifconfig");

#ifdef WIFI_DIRECT_USE_VFORK_FOR_SYSTEM
    /** after all, destroy the __wifi_direct_system() locker */
    pthread_mutex_destroy(&_wifi_direct_sys_locker);
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		printf("wifi_dir_ect close[%s][%d]:\n",__func__,__LINE__);
		Reroute_for8189err();
#endif

    return 0;
    
flag_err:

#ifdef WIFI_DIRECT_USE_VFORK_FOR_SYSTEM    
    /** after all, destroy the __wifi_direct_system() locker */
    pthread_mutex_destroy(&_wifi_direct_sys_locker);
#endif

    printf("wifi direct function stop error\n");
    return ret;
}


extern int vid_pid_from_driver;
static int wifi_direct_RoutingForward()
{
	if(widi_connect_status()==WIFI_COMPLETED){
		char *pub_interface=NULL;
		char *private_interface=NULL;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		
					if(vid_pid_from_driver==0x148f8189){
						pub_interface="wlan2";
						private_interface="wlan3";
					}
					else{
		
						pub_interface="wlan0";
						private_interface="wlan1";
					}
#else
					pub_interface="wlan0";
					private_interface="wlan1";
#endif

		printf("!!!!! reinit p2p ,we should reseting Routing Forward\n");
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
		char *private_subnet="192.168.168.1/24";
#else
		char *private_subnet="192.168.203.1/24";
#endif
#else
		char *private_subnet="192.168.111.1/24";
#endif

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		//enable_route_function(pub_interface,"wlan1",private_subnet,WIFI_ROUTING);

		//enable_route_function(pub_interface,"wlan3","192.168.203.1/24",WIFI_ROUTING);
		enable_route_function(pub_interface,"wlan1","192.168.168.1/24",WIFI_ROUTING);
		printf("[%s][%d]wifiBridgeProcess\n",__func__,__LINE__);
		sleep(1);
		wifiBridgeProcess(10);
		
#else
		enable_route_function(pub_interface,private_interface,private_subnet,WIFI_ROUTING);
#endif
	}
	return 0;
}
static int reinit_wifi_direct_function(struct p2p *p)
{
	char call_buf[128]={00};
	char cmd[64]={0x00};
	char cmd2[256]={0x00};
	int count;
	int ret;
	#if  WIFI_STA_P2PAUTOGO_ENABLE
	__wifi_direct_system("killall hostapd");
#if WIFI_DIRECT_DNSMASQ
	__wifi_direct_system("killall dnsmasq");
#endif
	__wifi_direct_system("killall udhcpd");
	#endif
	__wifi_direct_system("killall tcpdump");
//   printf("%s.......\n",__FUNCTION__);
    if(p->wpa_open ==_TRUE ){
#if EZCAST_LITE_ENABLE
		if(wifi_direct_mode == WIDI_MODE_MIX){
			if(p2p_current_port != P2P_WLAN0){
				__wifi_direct_system("kill $(ps | grep wpa_supplicant | grep wlan1 | awk \'{print $1}\')");
			}else{
				printf("[%s,%d]:kill wpa......\n",__FUNCTION__,__LINE__);
				__wifi_direct_system("killall wpa_supplicant");
				__wifi_direct_system("killall udhcpc");
				__wifi_direct_system("rm -rf /tmp/wpa_ctrl_*");
			}
			p->wpa_open = _FALSE;
		}else{
#endif
	#if !WIFI_DIRECT_CLIENT_ENABLE
         printf("[%s,%d]:kill wpa......\n",__FUNCTION__,__LINE__);
         __wifi_direct_system("killall wpa_supplicant");
         __wifi_direct_system("killall udhcpc");
         __wifi_direct_system("rm -rf /tmp/wpa_ctrl_*");
         p->wpa_open = _FALSE;
	#endif
#if EZCAST_LITE_ENABLE
		}
#endif
    }
    
    if(p->ap_open==_TRUE){
#if EZCAST_LITE_ENABLE
		if(wifi_direct_mode == WIDI_MODE_MIX){
			if(p2p_current_port != P2P_WLAN1){
				__wifi_direct_system("kill $(ps | grep hostapd | grep rtl_hostapd.conf | awk \'{print $1}\')");
				__wifi_direct_system("kill $(ps | grep udhcpd | grep udhcpd.conf | awk \'{print $1}\')");
			}else{
				printf("[%s,%d]:kill hostapd......\n",__FUNCTION__,__LINE__);
				__wifi_direct_system("killall hostapd");
#if WIFI_DIRECT_DNSMASQ
				__wifi_direct_system("killall dnsmasq");
#endif
				__wifi_direct_system("killall udhcpd");
			}
		}else{
#endif
	#if !WIFI_DIRECT_CLIENT_ENABLE
	#if !WIFI_DIRECT_SOFTAP_ENABLE
         printf("[%s,%d]:kill hostapd......\n",__FUNCTION__,__LINE__);
	#if 0//defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		__wifi_direct_system("kill $(ps | grep hostapd | grep rtl_hostapd.conf | awk \'{print $1}\')");
      		
	#else
		__wifi_direct_system("killall hostapd");
	#endif
#if WIFI_DIRECT_DNSMASQ
		__wifi_direct_system("killall dnsmasq");
#endif
  	printf("[%s,%d]:kill udhcpd.....\n",__FUNCTION__,__LINE__);
	#if 0//defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		__wifi_direct_system("kill $(ps |grep udhcpd | grep udhcpd.conf | awk \'{print $1}\')");		
	#else
        	 __wifi_direct_system("killall udhcpd");
         
	#endif
	#endif
	#endif
#if EZCAST_LITE_ENABLE
		}
#endif
         p->ap_open = _FALSE;
    }
    __wifi_direct_system("killall ifconfig");

	if(access("/tmp/amgw.txt",F_OK)==0){
		remove("/tmp/amgw.txt");
	}
    iwprivClose(p->skfd);          //close the skfd before reinite it
    /**
    * first disable and then enable as device mode.
    */
    sprintf(call_buf,"ifconfig %s 0.0.0.0",p->ifname);
    __wifi_direct_system(call_buf);

    sprintf(call_buf,"ifconfig %s down",p->ifname);
    __wifi_direct_system(call_buf);

#if EZCAST_LITE_ENABLE
	if(wifi_direct_mode == WIDI_MODE_MIX){
		if(p2p_current_port == P2P_WLAN0)
	    	strcpy(p->ifname,"wlan0");
		else
	    	strcpy(p->ifname,"wlan1");
	}else{
#endif
	strcpy(p->ifname, WIFI_DIRECT_INTF );
#if EZCAST_LITE_ENABLE
	}
#endif
    p->status = P2P_STATE_NONE;
    p->status_for_json = P2P_STATE_NONE;          //used for store the miracast connect_in status
    p->skfd = iw_sockets_open();
#if WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT         //keep client alive
    p->intent = 15;
#else
	p->intent = WIFI_DIRECT_INIT_INTENT;
#endif
    p->pin = 12345670;
    p->role = P2P_ROLE_DEVICE;
    p->wpa_open=0;
    p->ap_open=0;
    strcpy( p->peer_devaddr, "00:00:00:00:00:00" );
    p->connect_flag = WIDI_CONNECT_DEFAULT;
    strcpy( p->peer_ifaddr, "00:00:00:00:00:00" );
    p->pbc_flag  = 0;
    p->ap_total = 0; //ap total
    p->scan_ok_flag = 0;
#if MODULE_CONFIG_EZCAST5G_ENABLE
	p->listen_ch=WIFI_DIRECT_INIT_LCH;
#else
	if(channel_p2p)
		p->listen_ch =channel_p2p;
	else
		p->listen_ch=WIFI_DIRECT_INIT_LCH;
#endif
    p->op_ch = bestChannelChosen;
#if WIFI_STA_P2PAUTOGO_ENABLE//bluse add to test station+p2p auto go
    p->wps_info=3;
#else
    p->wps_info=0;
#endif
    p->connect_go = 0;
    p->polling_status_flage = 0;

    strcpy(p->mac_connect_ok, "00:00:00:00:00:00");

    sprintf(call_buf,"ifconfig %s 0.0.0.0 up",p->ifname);
    __wifi_direct_system(call_buf);
    
//    sprintf(call_buf,"iwpriv %s p2p_set enable=0",p->ifname);
//    __wifi_direct_system(call_buf);


    iwprivExecute(p->skfd,p->ifname,"p2p_set","enable=0",NULL);
    __wifi_direct_sleep(1);

//    sprintf(call_buf,"iwpriv %s p2p_set enable=1",p->ifname);
//    __wifi_direct_system(call_buf);

#if WIFI_STA_P2PAUTOGO_ENABLE   //bluse add to test station+ p2p auto go
	iwprivExecute(p->skfd,p->ifname,"p2p_set","enable=3",NULL);
#else
	iwprivExecute(p->skfd,p->ifname,"p2p_set","enable=1",NULL);
#endif
    wifi_direct_set_device_name(p);
    wifi_direct_set_wfd_type(p,1);
    wifi_direct_set_listenchannel(p,p->listen_ch);
    //wifi_direct_set_opchannel(p);
    wifi_direct_set_profilefound_information(p);     
    wifi_direct_set_softap_ssid(p);
    wifi_driect_set_persistent(p,1);
    wifi_direct_set_wfd_sa(p,1);
    wifi_direct_set_intent(p);
    //wifi_direct_set_wpsinfo(p);


#if WIFI_STA_P2PAUTOGO_ENABLE    //bluse add to STA + p2p auto go

#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	sprintf(cmd, "ifconfig %s 192.168.168.1", p->ifname);
#else
	sprintf(cmd, "ifconfig %s 192.168.203.1", p->ifname);
#endif	
#else
	sprintf(cmd, "ifconfig %s 192.168.111.1", p->ifname);
#endif
	__wifi_direct_system(cmd);
	usleep(5000);

              memset(cmd2, 0x00, CMD_SZ );
              sprintf(cmd2, "hostapd -B %s -dd &",WIFI_DIRECT_HOSTAPD_CONF);
              __wifi_direct_system(cmd2);

              /**
              * sleep is not ok, we must do some active check to make sure the hostapd
              * really starts ok.
              */
              sprintf(cmd2,"/tmp/hostapd/%s",p->ifname);
              count=0;
              while(count < 30){
                   usleep( HOSTAPD_INIT_TIME );
                   if(access(cmd2,F_OK)==0){
		   	p->ap_open=_TRUE;
                        printf(">>> hostapd start ok\n");
                        break;
                   }
              }

		__wifi_direct_system("rm -rf /tmp/udhcpd.leases");
		__wifi_direct_system("touch /tmp/udhcpd.leases");
#if WIFI_DIRECT_DNSMASQ
		printf(">>>>>start dnsmasq\n");
		sprintf(cmd,"/sbin/dnsmasq -d -C %s &",WIFI_DIRECT_UDHCPD_CONF);
		__wifi_direct_system(cmd);
#else
		printf("%s>>>>>start udhcpd\n",__func__);
		sprintf(cmd,"udhcpd %s",WIFI_DIRECT_UDHCPD_CONF);
		__wifi_direct_system(cmd);
#endif
		usleep(5000);
		wifi_direct_RoutingForward();
		if(p->pthread_go_runing==0){
			ret = pthread_create(&p->pthread_go, NULL, &wifi_polling_client, (void *)p);
			p->pthread_go_runing = 1;
		}

#else
    /**
    * restart the status polling thread.
    */
    if(p->pthread_runing==0){
         /** first collect the memory garbage if the poll thread exited */
         if(p->thread_poll_gc){
              pthread_join(p->pthread, NULL);
              p->thread_poll_gc = 0;
         }
         ret = pthread_create(&p->pthread, NULL, &wifi_direct_polling_status, (void *)p);
         p->pthread_runing = 1;
    }
#endif
    /**
    * rescan.
    */
    //wifi_direct_send_msg(WIDI_CMD_SCAN);//send scan ap message     
    
    return 0;
}

/* 
* get other devices' supported config methods. 
*/
static int wifi_p2p_wps_cm(struct p2p *p, char *scan_addr, char *cms)
{
    FILE *pf = NULL;
    int cm=0, i=0;
    char parse[CMD_SZ] = {0x00};
    char cmd[48]={0x00};
    
    memset( cms, 0x00, 30 );
    sprintf( cmd, "iwpriv %s p2p_get2 wpsCM=%s > /tmp/amwd_wps_cm.txt", p->ifname, scan_addr);     
    
    pf = fopen( "/tmp/amwd_wps_cm.txt", "r" );
    if ( pf ){
         while( !feof( pf ) ){
              memset( parse, 0x00, CMD_SZ );
              fgets( parse, CMD_SZ, pf );
              if( strncmp( parse, "M=", 2 ) == 0 )
              {
                   cm = atoi( &parse[ 2 ] );
                   printf("<%s %d> cm=%d\n",__FUNCTION__,__LINE__,cm);     
                   if((cm & WPS_CONFIG_METHOD_LABEL) == WPS_CONFIG_METHOD_LABEL){
                        strncpy( cms+i, " LAB", 4 );
                        i=i+4;
                   }
                   if((cm & WPS_CONFIG_METHOD_DISPLAY) == WPS_CONFIG_METHOD_DISPLAY){
                        if((cm & WPS_CONFIG_METHOD_VDISPLAY) == WPS_CONFIG_METHOD_VDISPLAY){
                             strncpy( cms+i, " VDIS", 5 );
                             i=i+5;
                        }else if((cm & WPS_CONFIG_METHOD_PDISPLAY) == WPS_CONFIG_METHOD_PDISPLAY){
                             strncpy( cms+i, " PDIS", 5 );
                             i=i+5;
                        }else{
                             strncpy( cms+i, " DIS", 4 );
                             i=i+4;
                        }
                   }
                   if((cm & WPS_CONFIG_METHOD_E_NFC) == WPS_CONFIG_METHOD_E_NFC){
                        strncpy( cms+i, " ENFC", 5 );
                        i=i+5;
                   }
                   if((cm & WPS_CONFIG_METHOD_I_NFC) == WPS_CONFIG_METHOD_I_NFC){
                        strncpy( cms+i, " INFC", 5 );
                        i=i+5;
                   }
                   if((cm & WPS_CONFIG_METHOD_NFC) == WPS_CONFIG_METHOD_NFC){
                        strncpy( cms+i, " NFC", 4 );
                        i=i+4;
                   }
                   if((cm & WPS_CONFIG_METHOD_PBC) == WPS_CONFIG_METHOD_PBC){
                        if((cm & WPS_CONFIG_METHOD_VPBC) == WPS_CONFIG_METHOD_VPBC){
                             strncpy( cms+i, " VPBC", 5 );
                             i=i+5;
                        }else if((cm & WPS_CONFIG_METHOD_PPBC) == WPS_CONFIG_METHOD_PPBC){
                             strncpy( cms+i, " PPBC", 5 );
                             i=i+5;
                        }else{
                             strncpy( cms+i, " PBC", 4 );
                             i=i+4;
                        }
                   }
                   if((cm & WPS_CONFIG_METHOD_KEYPAD) == WPS_CONFIG_METHOD_KEYPAD){
                        strncpy( cms+i, " PAD", 4 );
                        i=i+4;
                   }

                   break;
              }     
         }

         fclose( pf );
    }
    
    return 0;
}

static int  wifi_direct_obtain_hostname(char *hostname)
{
    FILE *fp = NULL;
    int i,len;


    fp = fopen("/mnt/user1/system_setting/hostname.dat","r");
    if(fp ==NULL){
         memcpy(hostname,"iEZVu",strlen("iEZVu"));
         printf(">>> default hostname :%s\n",hostname);
    }
    else{
         fread(hostname,sizeof(char),32,fp);
         printf(">>>hostname :%s\n",hostname);
         fclose(fp);
    }

    len = strlen(hostname);
    for(i=0;i<len;i++){
         if(*(hostname+i)=='\n'){
              *(hostname+i) = 0;
              break;
         }
    }

    return 0;
}

/*
* set wifi direct device name
*/
static int wifi_direct_set_device_name(struct p2p *p)
{
    //char hostname[33]={0x00};
    char cmd[64]={0x00};
    
    //memset(p->dev_name,0x00,sizeof(p->dev_name));
    //wifi_direct_obtain_hostname(hostname);
    //memcpy(p->dev_name,hostname,strlen(hostname));
//    sprintf( cmd, "iwpriv %s p2p_set setDN=\"%s\"", p->ifname, p->dev_name);
//    __wifi_direct_system(cmd);     


    sprintf( cmd, "setDN=%s", p->dev_name);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
;
    return 1;
}

/* 
* get peer device name
*/
static int wifi_p2p_get_device_name(struct p2p *p, int ap_num )
{
    FILE *pf = NULL;
    int i;
    char parse[CMD_SZ] = {0x00};
    char cmd[64]={0x00};
    char *locate = NULL;
/*    
    if(ap_num == -1){
         sprintf( cmd, "iwpriv %s p2p_get2 devN=%s >/tmp/device.txt", p->ifname, p->peer_devaddr);
    }
    else{
         sprintf( cmd, "iwpriv %s p2p_get2 devN=%s >/tmp/device.txt", p->ifname, p->scan_pool[ap_num].mac_addr);
    }
    __wifi_direct_system(cmd);
    
    pf = fopen( "/tmp/device.txt", "r" );
    if ( pf ){
         while( !feof( pf ) ){
              memset( parse, 0x00, CMD_SZ );
              fgets( parse, CMD_SZ, pf );
              if( strncmp( parse, "N=", 2) == 0 )
              {
                   if(ap_num == -1){
                        memset(widi_device_name,0x00,48);
                        strncpy(widi_device_name,parse+2,strlen(parse+2));
                   }
                   else{
                        memset(p->scan_pool[ap_num].widi_device_name,0x00,32);
                        strncpy(p->scan_pool[ap_num].widi_device_name,parse+2,strlen(parse+2));
                        //strncpy(p->scan_pool[ap_num].widi_device_name,dns,strlen(dns)-1);
                        p2p_info("p2p device name: %s %d\n",p->scan_pool[ap_num].widi_device_name,ap_num);
                   }
                   break;
              }     
         }
         fclose( pf );
    }
*/
    if(ap_num == -1){
         sprintf( cmd, "devN=%s", p->peer_devaddr);
    }
    else{
         sprintf( cmd, "devN=%s", p->scan_pool[ap_num].mac_addr);
    }
    debug_info("p->skfd=====%d",p->skfd);
    iwprivExecute(p->skfd,p->ifname,"p2p_get2",cmd,parse);
    printf("---%s---%d-- p->peer_devaddr is %s--parse is %s\n",__FUNCTION__,__LINE__, p->peer_devaddr,parse);
    debug_info("p->skfd=====%d",p->skfd);
    if((locate = strstr( parse, "N=")) != NULL )
    {
         if(ap_num == -1){
              memset(widi_device_name,0x00,48);
              strncpy(widi_device_name,locate+2,strlen(locate+2));
         }
         else{
              memset(p->scan_pool[ap_num].widi_device_name,0x00,32);
              strncpy(p->scan_pool[ap_num].widi_device_name,locate+2,strlen(locate+2));
              //strncpy(p->scan_pool[ap_num].widi_device_name,dns,strlen(dns)-1);
              p2p_info("p2p device name: %s %d\n",p->scan_pool[ap_num].widi_device_name,ap_num);
         }
    }     
    
    return 0;
}

/* 
* scan wifi ap;send ssid information to swf layer,storage ssid in pscan_pool struct 
*/
int wifi_direct_scan(struct p2p *p)
{
    char cmd[48]={0x00};

    sprintf( cmd, "iwlist %s scan > /tmp/amwd_scan.txt", p->ifname);
    __wifi_direct_system( cmd );
    
    return 0;
}

static void wifi_direct_device_signalLevel(struct p2p *p,char *signal_level,int no_dev)
{
    char *tmp_level=NULL;
    char tmp[4]={0x00};
    int num = no_dev -1;

    /**
    * do not use signal level.
    */
#if 0
    tmp_level=strstr(signal_level,"/100");
    strncpy(tmp,signal_level+13,strlen(signal_level)-strlen(tmp_level)-strlen("Signal level="));
    p->scan_pool[num].singal_level = (int)(atoi(tmp)/20);
#endif
    p->scan_pool[num].singal_level = 5;

    //printf("<%s %d> num:%d debug sgn_level:%d\n",__func__,__LINE__,num,p->scan_pool[num].singal_level);
}

static int wifi_direct_scan_results()
{
    struct p2p *p=NULL;     
    FILE *p2p_fd =NULL;
    int no_dev=0,i=0;
    char parse[CMD_SZ] = { 0x00 };
    char cmd[32]={0x00};
    char dns[SSID_SZ] = { 0x00 };
    char *sgn_level = NULL;
    struct wd_scan *pscan_pool;

    p = &action_p2p;
    p->ap_total = 0;

    // into scan, set the scan ok flag to 0
    p->scan_ok_flag = 0;
    wifi_direct_scan(p);
    
    p2p_fd=fopen("/tmp/amwd_scan.txt","r");
    if(p2p_fd==NULL){
         //>>> although it fails, we still need to set ok flag to indicate the 
         //>>> scan has completed.
         p->scan_ok_flag = 1;
         goto flags_p2p;
    }
    else{
         while(!feof( p2p_fd )){
              memset(parse,0x00,CMD_SZ);
              fgets(parse,CMD_SZ,p2p_fd);               
              if(parse[0] == '\n' || parse[0] == '\0'){
                   break;
              }
              
              if(strncmp(parse+20,"Address:",8)==0){     

                   pscan_pool = &p->scan_pool[no_dev];
                   memset( pscan_pool->mac_addr, 0x00, sizeof(pscan_pool->mac_addr));
                   strncpy( pscan_pool->mac_addr, parse+29, 17);
              }
              else if( strncmp(parse+20, "ESSID:", 6) == 0 )
              {
                   pscan_pool = &p->scan_pool[no_dev];
                   
                   //wifi_p2p_wps_cm(p, pscan_pool->mac_addr,cmd);     
                   //p2p_device_name(p, pscan_pool->addr, dns);
                   //memset(  pscan_pool->ssid, 0x00, sizeof( pscan_pool->ssid));
                   //strncpy( pscan_pool->ssid, dns, SSID_SZ);
                   
                   if( strncmp(parse+26, "\"DIRECT-\"", 9) == 0 )
                   {
                        //not group owner
                        pscan_pool->go = 0;
                   }
                   else
                   {          
                        pscan_pool->go = 1;
                   }
                   no_dev++;
              }
              else if((sgn_level=strstr(parse,"Signal level=")) !=NULL){
                   wifi_direct_device_signalLevel(p,sgn_level,no_dev);
              }
         }

         fclose(p2p_fd);
    }     

    for(i=0;i<no_dev;i++){
         wifi_p2p_get_device_name(p,i);
    }
    p->ap_total = no_dev;
    p->scan_ok_flag = 1;

    /**
    * print the scan result.
    */
    printf("************************\n");
    printf("Mac\t\tGo\tName\n");
    if(no_dev>0){
         for(i=0;i<no_dev;i++){
              printf("%s\t%d\t%s\n",p->scan_pool[i].mac_addr,p->scan_pool[i].go,p->scan_pool[i].widi_device_name);
         }
    }
    printf("************************\n");
    
    return p->ap_total;
    
flags_p2p:
    return -1;
}

/**
* for client mode ,setup wps information;wpa_cli
* command:wpa_cli -p/var/run/wpa_supplicant status ,get the current status
*/  
void wifi_direct_wps(struct p2p *p)
{
    FILE *pf = NULL;
    int ret = _FALSE, parsing_ok = _FALSE;
    char parse[CMD_SZ]={0x00};
    char cmd[CMD_SZ]={0x00};
    
    do
    {
         //memset( cmd, 0x00, CMD_SZ );
         printf("---%s %d ap_open:%d wpa_open:%d wpa_info:%d\n",__func__,__LINE__,p->ap_open,p->wpa_open,p->wps_info);
         if( p->ap_open == _TRUE )  //ap connect hostapd_cli
         {
              if(p->wps_info==1 || p->wps_info==2)
                   sprintf( cmd, "hostapd_cli -p/tmp/hostapd wps_pin any %d > /tmp/amwd_wps.txt", p->pin);
              else if(p->wps_info==3)
                   sprintf( cmd, "hostapd_cli -p/tmp/hostapd wps_pbc any > /tmp/amwd_wps.txt");
         }
         else if(p->wpa_open == _TRUE)  //wps connect  wpa_supplicant
         {
              p2p_info("wps_info=%d connect_go=%d peer_ifaddr=%s\n",p->wps_info,p->connect_go,p->peer_ifaddr);

              if(p->connect_go==1)
              {
                   if(p->wps_info==1 || p->wps_info==2){
                        sprintf( cmd, "wpa_cli -p/tmp/wpa_supplicant wps_pin %s %d > /tmp/amwd_wps.txt", p->peer_ifaddr, p->pin);
                   }
                   else if(p->wps_info==3){
                        sprintf( cmd, "wpa_cli -p/tmp/wpa_supplicant wps_pbc %s > /tmp/amwd_wps.txt", p->peer_ifaddr);
                   }
              }
              else if( strncmp(p->peer_ifaddr, "00:00:00:00:00:00", 17)==0 )
              {
                   if(p->wps_info==1 || p->wps_info==2)  
                        sprintf( cmd, "wpa_cli -p/tmp/wpa_supplicant  wps_pin any %d > /tmp/amwd_wps.txt", p->pin);
                   else if(p->wps_info==3)
                        sprintf( cmd, "wpa_cli -p/tmp/wpa_supplicant  wps_pbc any > /tmp/amwd_wps.txt");
              }
              else
              {
                   if(p->wps_info==1 || p->wps_info==2){
                        sprintf( cmd, "wpa_cli -p/tmp/wpa_supplicant  wps_pin %s %d > /tmp/amwd_wps.txt",  p->peer_ifaddr, p->pin);
                   }
                   else if(p->wps_info==3){
                        sprintf( cmd, "wpa_cli  -p/tmp/wpa_supplicant  wps_pbc %s > /tmp/amwd_wps.txt", p->peer_ifaddr);
                        //sprintf( p->cmd, "wpa_cli  -p/tmp/wpa_supplicant  wps_pbc any");
                   }
              }
         }
         __wifi_direct_system(cmd);

         pf = fopen( "/tmp/amwd_wps.txt", "r" );     
         if ( pf )
         {
              while( !feof( pf ) ){
                   memset(parse, 0x00, CMD_SZ);
                   fgets(parse, CMD_SZ, pf);
                   if(p->ap_open == _TRUE)//ap connect
                   {
                        if( (p->wps_info==1 || p->wps_info==2) && (strncmp(parse, "", 2) == 0) )
                             parsing_ok = _TRUE;
                        else if( (p->wps_info==3) && (strncmp(parse, "OK", 2) == 0) )
                             parsing_ok = _TRUE;
                   }
                   else if(p->wpa_open == _TRUE) //wpa connect
                   {
                        if( (p->wps_info==1 || p->wps_info==2) &&     (strncmp(parse, "Selected", 8) == 0) )
                             parsing_ok = _TRUE;
                        else if( (p->wps_info==3) && (strncmp(parse, "OK", 2) == 0) )
                             parsing_ok = _TRUE;
                   }                    
                   if( parsing_ok == _TRUE )
                   {
                        ret = _TRUE;
                        p->wpsing = _TRUE;
                   }
              }          
              fclose( pf );
         }
         
         if( ret == 0 ){
              //usleep( HOSTAPD_INIT_TIME );
              __wifi_direct_sleep(1);
         }
    }while( ret == 0 );
}

static int  _wifi_direct_clientMode_connect_status(struct p2p *p)
{
    int ret = -1,wpa_status_ok = -1,ip_address_ok = -1;
    FILE *p2p_fd = NULL;
    char parse[CMD_SZ]={0x00};
    char cmd[CMD_SZ]={0x00};
    
    memset(cmd,0x00,CMD_SZ);
    if(access("/tmp/wpa_supplicant/",F_OK)==0){
         sprintf(cmd,"wpa_cli -p/tmp/wpa_supplicant status > /tmp/amwd_wpa_status.txt");
         __wifi_direct_system(cmd);
         p2p_fd = fopen("/tmp/amwd_wpa_status.txt","r");
         if(p2p_fd != NULL){
              while( !feof( p2p_fd ) ){
                   memset( parse, 0x00, CMD_SZ );
                   fgets( parse, CMD_SZ, p2p_fd );
                   if(strncmp( parse, "wpa_state=COMPLETED", 19) == 0 ){
                        wpa_status_ok = 1;
                   }
                   else if(strncmp( parse, "wpa_state=SCANNING", 18) == 0||strncmp( parse, "wpa_state=INACTIVE", 18) == 0||strncmp( parse, "wpa_state=DISCONNECTED", 22) == 0){
                        wpa_status_ok = 2;
                   }
                   else if(strncmp( parse, "ip_address=", 11)==0){
                        ip_address_ok = 1;
                   }
              }
              fclose(p2p_fd);
         }
         
         if((wpa_status_ok ==1)&&(ip_address_ok == 1)){
              wifi_direct_peer_devaddr(p);
              strncpy(p->mac_connect_ok,p->peer_devaddr,strlen(p->peer_devaddr));
              //printf("<%s %d>  mac_connect_ok=%s\n",__FUNCTION__,__LINE__,p->mac_connect_ok);
              ret = 0;
         }
         else if (wpa_status_ok == 2){
              ret = 2;
         }
    }
    else{
         ret =2;
    }
    
    return ret;
}

static int wifi_direct_wpa_conf_file(struct p2p *p,char *filename)
{
    int fd;
    FILE *fp = NULL;
    char buf[256]={0};
    int i;
    char mac[50]={0};

    sprintf(filename,"/tmp/%x.conf",APMac_Hash(p->peer_ifaddr));
    p2pdbg_info("filename:%s\n",filename);

    /** uuid generation */
    sprintf(buf,"ifconfig | grep %s > /tmp/amwd_ifinfo.txt",p->ifname);
    __wifi_direct_system(buf);
    
    fp = fopen( "/tmp/amwd_ifinfo.txt", "r");
    if(fp== NULL){
         printf( "Error to open the /tmp/amwd_ifinfo.txt file\n" );
    }
    else{
         memset(buf, 0x00, 256);
         if (fgets(buf,256,fp) == NULL){
              printf( "Read /tmp/amwd_ifinfo.txt content error!\n" );
         }
         else{
              memset(mac, 0x00, 50);
              for(i = 0; i < 256; i++ ){
                   if (strncmp( buf + i, "HWaddr ", 7 ) == 0 ){
                        strncpy(mac,buf + i + 7,17);
                        printf( "%s\n", mac );
                        break;
                   }
              }
         }
         fclose( fp );
    }
    
    //if(access(filename,F_OK)!=0){
         if(strlen(mac)>=17){
              sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\nupdate_config=1\nuuid=%c%c%c%c%c%c%c%c-%c%c%c%c-0000-0000-000000000000\n\n",
                   mac[ 0 ],
                   mac[ 1 ],
                   mac[ 3 ],
                   mac[ 4 ],
                   mac[ 6 ],
                   mac[ 7 ],
                   mac[ 9 ],
                   mac[ 10 ],
                   mac[ 12 ],
                   mac[ 13 ],
                   mac[ 15 ],
                   mac[ 16 ]);
         }
         else{
              sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\nupdate_config=1\n\n");
         }

         fp= fopen(filename,"wb");
         if(fp==NULL){
              p2p_err("can not open %s\n",filename);
              return -1;
         }
         if(fwrite(buf,sizeof(char),strlen(buf),fp) != strlen(buf)){
              p2p_err("write conf file error!");
              fclose(fp);
              return -1;
         }
         fsync(fileno(fp));
         fclose(fp);
         fp= NULL;
    //}
    return 0;
}

//static char wpa_curr[128]={0};
static int _wifi_direct_save_profile(struct p2p *p,char *filename)
{
    char buf[128]={0};
    char tmpbuf[128]={0};
    char tmpfile[128]={0};
    FILE *fp,*fp2;
    char ssid[64]={0};
    char bssid[32]={0};
    int ssid_len=0;
    char *ptr;
    int i,cnt=0;
    
    /** save the profile info*/
    fp = fopen(filename,"r");
    if(fp == NULL){
         return -1;
    }
    while(!feof(fp)){
         memset(buf, 0x00, 128);
         fgets(buf, 128, fp);
         if((ptr = strstr(buf,"bssid=")) != NULL){
              ptr+=6;
              i=0;
              while((*ptr != '\n') && (*ptr!=0x0d) && (*ptr!=0x0a)){
                   bssid[i] = *ptr;
                   i++;
                   ptr++;
              }
         }
         else if((ptr = strstr(buf,"ssid=")) != NULL){
              ptr+=6;
              i=0;
              while( (*ptr != '\"') && (*ptr != '\n') && (*ptr!=0x0d) && (*ptr!=0x0a) ){
                   ssid[i] = *ptr;
                   i++;
                   ptr++;
              }
         }
    }
    fclose(fp);

    ssid_len = strlen(ssid);
    if(ssid_len > 0 && strlen(bssid)==17 ){
         sprintf(buf,"1%s%02d%s",bssid,ssid_len,ssid);
         
         /** check if already exists */
         fp = fopen(WIFI_DIRECT_PROFILE_LIST,"r");
         if(fp == NULL){
              goto _WRITE_TO_END;
         }
         cnt=0;
         while(!feof(fp)){
              memset(tmpbuf, 0x00, 127);
              ptr = fgets(tmpbuf,127,fp);
              cnt++;
              if(ptr && strstr(tmpbuf,buf)){
                   /** already saved, skip */
                   //printf(">>>>>>>>>>> already exists\n");
                   fclose(fp);
                   sprintf(tmpfile,"/mnt/user1/%x.conf",APMac_Hash(p->peer_ifaddr));
                   _wifi_direct_copy_file(filename,tmpfile);
                   return 0;
              }
         }

         if(cnt > 10){
              //printf(">>>>>>>>>>> too many profile saved, delete one\n");
              fseek(fp, 0, SEEK_SET);
              
              fp2 = fopen("/tmp/wd_profile.tmp","w");
              if(fp2 == NULL){
                   printf(">>>>>>>>>>>>>>>> open /tmp/wd_profile.tmp error\n");
                   fclose(fp);
                   return -1;
              }

              cnt = 0;
              while(!feof(fp)){
                   memset(tmpbuf, 0x00, 127);
                   ptr = fgets(tmpbuf,127,fp);
                   cnt++;
                   if(ptr){
                        if(cnt==1){
                             /** delete the first and  its wpa configure file also*/
                             tmpbuf[18]=0;
                             sprintf(tmpfile,"/mnt/user1/%x.conf",APMac_Hash(&tmpbuf[1]));
                             remove(tmpfile);
                        }
                        else{
                             /** others just copy */
                             fputs(tmpbuf, fp2);
                        }
                   }
              }
              fclose(fp2);
              fclose(fp);

              _wifi_direct_copy_file("/tmp/wd_profile.tmp",WIFI_DIRECT_PROFILE_LIST);
              remove("/tmp/wd_profile.tmp");
              
         }
         
    _WRITE_TO_END:
         fp = fopen(WIFI_DIRECT_PROFILE_LIST,"a+");
         if(fp == NULL){
              return -1;
         }
         fputs(buf,fp);
         fputs("\n",fp);
         fclose(fp);

         _wifi_direct_copy_file(WIFI_DIRECT_PROFILE_LIST,WIFI_DIRECT_PROFILE_LIST_DISK);

         /**
         * write the wpa_config file to disk
         */
         sprintf(tmpfile,"/mnt/user1/%x.conf",APMac_Hash(p->peer_ifaddr));
         _wifi_direct_copy_file(filename,tmpfile);
         
    }
    else{
         printf(">>>>>> save error:%s,%d,%s",bssid,ssid_len,ssid);
    }
    
    return 0;     
}

/**/
widi_status_enum widi_wlan_status = WIFI_DISCONNECTED;
widi_current_status wlan_direct_status;
static int widi_connect_status()
{
    char *start, *end, *pos;
    int last = 0,ret;
    struct wpa_ctrl *widi_wpa = NULL;
    char *locate = "wpa_state=";
    char buf[2048] = {0};
    int tmp_status = WIFI_DISCONNECTED;
    int rtn = 0;
    size_t len = sizeof(buf) - 1;
    char path[64];
    int cnt=0;
#if WIFI_STA_P2PAUTOGO_ENABLE
  sprintf(path,"/tmp/wpa_supplicant/wlan0");
#else
    //>>> get the P2P socket path for wpa_supplicant.
    sprintf(path,"/tmp/wpa_supplicant/%s",action_p2p.ifname);
#endif
printf("%s %d path:%s\n",__func__,__LINE__,path);    
    if(access(path,F_OK)==0){
         widi_wpa=(struct wpa_ctrl *)wpa_ctrl_open(path);
         if(widi_wpa == NULL){
              printf("%s %d wpa_ctrl_open error\n",__FUNCTION__,__LINE__);
              return WIFI_DISCONNECTED;
         }
    }
    else{
         printf("%s %d wpa_supplicant opened error\n",__FUNCTION__,__LINE__);
         return WIFI_DISCONNECTED;
    }

    wpa_ctrl_request(widi_wpa, "STATUS", strlen("STATUS"), buf, &len, NULL);
    pos = strstr(buf,locate);
    if(pos==NULL){
         printf(">>> no wpa status found\n");
         wpa_ctrl_close(widi_wpa);
         return WIFI_DISCONNECTED;
    }

    start = buf;
    while(*start){
         last = 0;
         end = strchr(start, '\n');
         if(end == NULL){
              last = 1;
              end = start;
              while (end[0] && end[1])
                   end++;
         }
         *end = '\0';
         pos = strchr(start, '=');
         if(pos){
              *pos++ = '\0';

              if(strcmp(start, "wpa_state") == 0){
                   memset(wlan_direct_status.wpa_state,0,sizeof(wlan_direct_status.wpa_state));
                   memcpy(wlan_direct_status.wpa_state,pos,strlen(pos));
                   if(strncmp(pos,"COMPLETED",9)==0)
                        tmp_status = WIFI_COMPLETED;
                   else if(strncmp(pos,"INACTIVE",8)==0)
                        tmp_status = WIFI_INACTIVE;
                   else if(strncmp(pos,"SCANNING",8)==0)
                        tmp_status = WIFI_SCANNING;
                   else if(strncmp(pos,"ASSOCIATING",11)==0)
                        tmp_status = WIFI_ASSOCIATING;
                   else if(strncmp(pos,"ASSOCIATED",10)==0)
                        tmp_status = WIFI_ASSOCIATED;
                   else if(strncmp(pos,"4WAY_HANDSHAKE",14)==0)
                        tmp_status = WIFI_4WAY_HANDSHAKE;
                   else if(strncmp(pos,"GROUP_HANDSHAKE",14)==0)
                        tmp_status = WIFI_GROUP_HANDSHAKE;
                   else
                        tmp_status = WIFI_DISCONNECTED;
              }
         }
         
         if(last)
              break;
         
         start = end + 1;
    }

    wpa_ctrl_close(widi_wpa);
    
    return tmp_status;
}

static int widi_auto_ip()
{
    char callbuf[56];
    
    /** run the current udhcpc process */
    sprintf(callbuf,"udhcpc -b -t 5 -i %s",action_p2p.ifname);
    printf("the call is %s\n",callbuf);     
    __wifi_direct_system(callbuf);
    
    modify_wifi_mode(WIFI_DIRECT_CLIENT_OK);
    return 0;
}

#if WIFI_DIRECT_NEW_FLOW
static struct wpa_ctrl *monitor_conn = NULL;
static int exit_sockets[2] = {-1};

static int widi_connect_wpas(){
	char path[64];

	if(NULL != monitor_conn){
		wpa_ctrl_close(monitor_conn);
		monitor_conn = NULL;
	}
	#if WIFI_STA_P2PAUTOGO_ENABLE
  		sprintf(path,"/tmp/wpa_supplicant/wlan0");
	#else
	    //>>> get the P2P socket path for wpa_supplicant.
	    sprintf(path,"/tmp/wpa_supplicant/%s",action_p2p.ifname);
	#endif
	printf("%s %d path:%s\n",__func__,__LINE__,path);

	if(access(path,F_OK)==0){	
         monitor_conn=(struct wpa_ctrl *)wpa_ctrl_open(path);
         if(monitor_conn == NULL){
              printf("%s %d wpa_ctrl_open error\n",__FUNCTION__,__LINE__);
              return -1;
         }
    }
    else{
         printf("%s %d wpa_supplicant opened error\n",__FUNCTION__,__LINE__);
         return -1;
    }

	if (wpa_ctrl_attach(monitor_conn) != 0) {
		wpa_ctrl_close(monitor_conn);
		monitor_conn = NULL;
		return -1;
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets) == -1) {
		wpa_ctrl_close(monitor_conn);
		monitor_conn = NULL;
		return -1;
	}

	
	return 0;
}

static pthread_t p2p_event_thread;
#define IFNAME "IFNAME="
#define IFNAMELEN (sizeof(IFNAME) - 1)
#define WPA_EVENT_IGNORE "CTRL-EVENT-IGNORE "
//#ifdef TEMP_FAILURE_RETRY
//#undef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp) ({         \
    typeof (exp) _rc;                      \
    do {                                   \
        _rc = (exp);                       \
    } while (_rc == -1 && errno == EINTR); \
    _rc; })
//#endif

static int wifi_ctrl_recv(char *reply, size_t *reply_len)
{
	int res;
	int ctrlfd = wpa_ctrl_get_fd(monitor_conn);
	struct pollfd rfds[2];

	memset(rfds, 0, 2 * sizeof(struct pollfd));
	rfds[0].fd = ctrlfd;
	rfds[0].events |= POLLIN;
	rfds[1].fd = exit_sockets[1];
	rfds[1].events |= POLLIN;
	res = TEMP_FAILURE_RETRY(poll(rfds, 2, -1));
	if (res < 0) {
		printf("Error poll = %d", res);
		return res;
	}
	if (rfds[0].revents & POLLIN) {
		return wpa_ctrl_recv(monitor_conn, reply, reply_len);
	}

	/* it is not rfds[0], then it must be rfts[1] (i.e. the exit socket)
	 * or we timed out. In either case, this call has failed ..
	 */
	return -2;
}

static int widi_wait_for_event(char *buf, size_t buflen){
	size_t nread = buflen - 1;
	int result;
	char *match, *match2;

	if (monitor_conn == NULL) {
		return snprintf(buf, buflen, WPA_EVENT_TERMINATING " - connection closed");
	}

	result = wifi_ctrl_recv(buf, &nread);

	/* Terminate reception on exit socket */
	if (result == -2) {
		return snprintf(buf, buflen, WPA_EVENT_TERMINATING " - connection closed");
	}

	if (result < 0) {
		printf("wifi_ctrl_recv failed: %s\n", strerror(errno));
		return snprintf(buf, buflen, WPA_EVENT_TERMINATING " - recv error");
	}
	buf[nread] = '\0';
	/* Check for EOF on the socket */
	if (result == 0 && nread == 0) {
		/* Fabricate an event to pass up */
		printf("Received EOF on supplicant socket\n");
		return snprintf(buf, buflen, WPA_EVENT_TERMINATING " - signal 0 received");
	}
	/*
	 * Events strings are in the format
	 *
	 *	   IFNAME=iface <N>CTRL-EVENT-XXX 
	 *		  or
	 *	   <N>CTRL-EVENT-XXX 
	 *
	 * where N is the message level in numerical form (0=VERBOSE, 1=DEBUG,
	 * etc.) and XXX is the event name. The level information is not useful
	 * to us, so strip it off.
	 */

	if (strncmp(buf, IFNAME, IFNAMELEN) == 0) {
		match = strchr(buf, ' ');
		if (match != NULL) {
			if (match[1] == '<') {
				match2 = strchr(match + 2, '>');
				if (match2 != NULL) {
					nread -= (match2 - match);
					memmove(match + 1, match2 + 1, nread - (match - buf) + 1);
				}
			}
		} else {
			return snprintf(buf, buflen, "%s", WPA_EVENT_IGNORE);
		}
	} else if (buf[0] == '<') {
		match = strchr(buf, '>');
		if (match != NULL) {
			nread -= (match + 1 - buf);
			memmove(buf, match + 1, nread + 1);
			//printf("supplicant generated event without interface - %s\n", buf);
		}
	} else {
		/* let the event go as is! */
		printf("supplicant generated event without interface and without message level - %s\n", buf);
	}

	return nread;

}
static int wifi_direct_get_port();
static int wifi_direct_get_peer_ip_action();
static char wpa_file_name[128];

bool wifi_direct_set_static_ip(const char *ifa, const char *ip, const char *gateway, const char *mask)
{
	bool status = false;

	ifc_set_ip_addr(ifa, ip);
	ifc_set_netmask_addr(ifa, mask);
	ifc_set_route_addr(ifa, gateway);

	status = ping_network(gateway, 500);

	if(status < 0){
		printf("ping ip fail, use udhcpc\n");
	}else{
		printf("ping ip ok\n");
	}
	return status;
}

int wifi_direct_get_json_from_file(const char *fileName,const char *itemName,
											const char *subItemName_1,char *sbuItemVal_1, int sbuItemVal_1Len,
											const char *subItemName_2,char *sbuItemVal_2, int sbuItemVal_2Len,
											const char *subItemName_3,char *sbuItemVal_3, int sbuItemVal_3Len);


static void parse_ap_event(char *buf, struct p2p *p){
	if(strstr(buf, "CTRL-EVENT-DISCONNECTED") != NULL){

	}else if(strstr(buf, "CTRL-EVENT-TERMINATING") != NULL){
		pthread_exit(NULL);
	}else if(strstr(buf, "CTRL-EVENT-CONNECTED") != NULL){
		if(_wifi_direct_clientMode_connect_status(p) ==0){
         	p->connect_flag = WIDI_CONNECT_CLIENT_OK;
    	}
		wifi_direct_get_port();
		char ip[32]={0};
		char mask[32]={0};
		char gateway[32]={0};
		int result;
		#if 0
		printf("[%s][%d]p->peer_ifaddr=%s,p->ifname=%s\n",__func__,__LINE__,p->peer_ifaddr,p->ifname);
		result = wifi_direct_get_json_from_file(WIFI_P2P_IP_CONFIG_FILE,
								p->peer_ifaddr,
								CONFIG_P2P_IP,ip,sizeof(ip),
								CONFIG_P2P_GATEWAY,gateway,sizeof(gateway),
								CONFIG_P2P_MASK,mask,sizeof(mask));
		printf("ip=%s,gateway=%s,netmask=%s\n",ip,gateway,mask);
		#else
		result = -1;
		#endif
		if(result < 0){
			widi_auto_ip();
			wifi_direct_get_peer_ip_action();
		}else{
			modify_wifi_mode(WIFI_DIRECT_CLIENT_OK);
			bool ret = wifi_direct_set_static_ip(p->ifname, ip, gateway, mask);
			if(ret){
				printf(">>> %s:peer ip is %s\n",__FUNCTION__,gateway);
				FILE *fp = NULL;
				fp = fopen("/tmp/sourceip.log","w");
				if(fp){
					fputs(gateway,fp);
					fclose(fp);
				}
				getIpResult = 0;
			}else{
				widi_auto_ip();
				wifi_direct_get_peer_ip_action();
			}
		}
		char cmd[128] = {0};
		char filename[128] = {0};
		sprintf(filename,"/tmp/%x.conf",APMac_Hash(p->peer_ifaddr));
		_wifi_direct_save_profile(p,filename);
		//sprintf(cmd,"wpa_cli -i %s -p /tmp/wpa_supplicant save_config", p->ifname);
        //__wifi_direct_system(cmd);
	}
}

static int check_wpa_event(struct p2p *p){
	char buf[1024]={0};

	int backval = widi_wait_for_event(buf,sizeof(buf));
	if(backval > 0){
		printf("###########################%s\n",buf);
		//string wpaBuf(buf);

		//if(wpaBuf.find("P2P-") != string::npos){
		//	parseP2pEvent(wpaBuf);			
		//}
		//else{
			//parseApEvent(wpaBuf);
		//}
		parse_ap_event(buf, p);
	}
	return backval;
}

static void *widi_wpa_event(void * obj){
	struct p2p *p = (struct p2p *)obj;
	
	while(1){
		pthread_testcancel();
		check_wpa_event(p);
	}
	pthread_exit(NULL);
	return NULL;
}

static int widi_status_monitor(struct p2p *p){

	if(pthread_create(&p2p_event_thread, NULL, widi_wpa_event,p)){
		printf("create p2p pthread error %d",errno);
		return -1;
	}
}
#endif
static int widi_status_check()
{
    int error_num=0,ret = -1;
    struct timespec _time_escape;
    int sleep_rtn=0;
    char cmd[128]={0};
    _time_escape.tv_sec = 1;
    _time_escape.tv_nsec = 0;
    
    while(1){
         widi_wlan_status = widi_connect_status();
         //printf("wlan_status=>>%d\n",widi_wlan_status);
         if(widi_wlan_status == WIFI_COMPLETED){
              printf("%s %d connect completely!\n",__func__,__LINE__);
              
              //if(access("/proc/net/rtl8192eu",F_OK)==0) 
              {
               /**
          * added for 8192EU miracast re-connect problem,if connected completed,disable the autoconnect, 
          * otherwise the miracast's re-connection will be failed.
          */
                   sprintf(cmd,"wpa_cli -p/tmp/wpa_supplicant sta_autoconnect 0");
                   __wifi_direct_system(cmd);
                   printf("[%s %d] sta autoconnect disable\n",__func__,__LINE__);
              }
              
              widi_auto_ip();
              ret = 0;
              break;
         }
         else{
              //sleep_rtn = nanosleep(&_time_escape,NULL);
              __wifi_direct_sleep(1);
              error_num++;
              if(error_num>60){
                   widi_wlan_status = WIFI_DISCONNECTED;
                   p2p_info("cannot connect ap!\n");
                   error_num=0;
                   break;
              }
         }
    }
    return ret;
}

static int _start_wpa_conf( char * filename)
{
    char callbuf[128]={0};
    int ret1;
    int ret2;
    
    memset(callbuf,0,sizeof(callbuf));
#if WPA_DEBUG
    sprintf(callbuf,"wpa_supplicant -B -dd -i%s -c %s",action_p2p.ifname,filename);
#else
    sprintf(callbuf,"wpa_supplicant -B -i%s -c %s",action_p2p.ifname,filename);
#endif
    ret1=utime(filename,NULL);
    printf("[%s:%s:%d] [%s--%s] zhoudayuan callbuf=%s\r\n",__FILE__, __func__, __LINE__, __DATE__, __TIME__, callbuf);
    ret2=__wifi_direct_system(callbuf);
    
    return ret2;
}

#if WIFI_DIRECT_NEW_FLOW
#define PROPERTY_VALUE_MAX 92
#define SUPP_CONFIG_TEMPLATE "/etc/wpa_supplicant.conf"
#define P2P_CONFIG_FILE "/mnt/vram/user1/p2p_supplicant.conf"
#define CONTROL_IFACE_PATH "/tmp/wpa_supplicant"

int update_ctrl_interface(const char *config_file) {

	int srcfd, destfd;
	int nread;
	char ifc[PROPERTY_VALUE_MAX];
	char *pbuf;
	char *sptr;
	struct stat sb;
	int ret;

	if (stat(config_file, &sb) != 0)
		return -1;

	pbuf = (char *)malloc(sb.st_size + PROPERTY_VALUE_MAX);
	if (!pbuf)
		return 0;
	srcfd = TEMP_FAILURE_RETRY(open(config_file, O_RDONLY));
	if (srcfd < 0) {
		printf("Cannot open \"%s\": %s", config_file, strerror(errno));
		free(pbuf);
		return 0;
	}
	nread = TEMP_FAILURE_RETRY(read(srcfd, pbuf, sb.st_size));
	close(srcfd);
	if (nread < 0) {
		printf("Cannot read \"%s\": %s", config_file, strerror(errno));
		free(pbuf);
		return 0;
	}

/*	if (!strcmp(config_file, SUPP_CONFIG_FILE)) {
		if (wifi_ifname(PRIMARY) == NULL) {
			printf("%s: get wifi_ifname(PRIMARY) fail\n", __func__);
			return -1;
		}
		strcpy(ifc, wifi_ifname(PRIMARY));
	} else {*/
		strcpy(ifc, CONTROL_IFACE_PATH);
//	}
	/* Assume file is invalid to begin with */
	ret = -1;
	/*
	 * if there is a "ctrl_interface=<value>" entry, re-write it ONLY if it is
	 * NOT a directory.  The non-directory value option is an Android add-on
	 * that allows the control interface to be exchanged through an environment
	 * variable (initialized by the "init" program when it starts a service
	 * with a "socket" option).
	 *
	 * The <value> is deemed to be a directory if the "DIR=" form is used or
	 * the value begins with "/".
	 */
	if ((sptr = strstr(pbuf, "ctrl_interface="))) {
		ret = 0;
		if ((!strstr(pbuf, "ctrl_interface=DIR=")) &&
				(!strstr(pbuf, "ctrl_interface=/"))) {
			char *iptr = sptr + strlen("ctrl_interface=");
			int ilen = 0;
			int mlen = strlen(ifc);
			int nwrite;
			if (strncmp(ifc, iptr, mlen) != 0) {
				printf("ctrl_interface != %s", ifc);
				while (((ilen + (iptr - pbuf)) < nread) && (iptr[ilen] != '\n'))
					ilen++;
				mlen = ((ilen >= mlen) ? ilen : mlen) + 1;
				memmove(iptr + mlen, iptr + ilen + 1, nread - (iptr + ilen + 1 - pbuf));
				memset(iptr, '\n', mlen);
				memcpy(iptr, ifc, strlen(ifc));
				destfd = TEMP_FAILURE_RETRY(open(config_file, O_RDWR, 0660));
				if (destfd < 0) {
					printf("Cannot update \"%s\": %s", config_file, strerror(errno));
					free(pbuf);
					return -1;
				}
				TEMP_FAILURE_RETRY(write(destfd, pbuf, nread + mlen - ilen -1));
				close(destfd);
			}
		}
	}
	free(pbuf);
	return ret;
}

int ensure_config_file_exists(const char *config_file)
{
	char buf[2048];
	int srcfd, destfd;
	struct stat sb;
	int nread;
	int ret;

	ret = access(config_file, R_OK|W_OK);
	if ((ret == 0) || (errno == EACCES)) {
		if ((ret != 0) &&
			(chmod(config_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
			printf("Cannot set RW to \"%s\": %s", config_file, strerror(errno));
			return -1;
		}
		/* return if we were able to update control interface properly */
		if (update_ctrl_interface(config_file) >=0) {
			return 0;
		} else {
			/* This handles the scenario where the file had bad data
			 * for some reason. We continue and recreate the file.
			 */
		}
	} else if (errno != ENOENT) {
		printf("Cannot access \"%s\": %s", config_file, strerror(errno));
		return -1;
	}

	srcfd = TEMP_FAILURE_RETRY(open(SUPP_CONFIG_TEMPLATE, O_RDONLY));
	if (srcfd < 0) {
		printf("Cannot open \"%s\": %s", SUPP_CONFIG_TEMPLATE, strerror(errno));
		return -1;
	}

	destfd = TEMP_FAILURE_RETRY(open(config_file, O_CREAT|O_RDWR, 0660));
	if (destfd < 0) {
		close(srcfd);
		printf("Cannot create \"%s\": %s", config_file, strerror(errno));
		return -1;
	}

	while ((nread = TEMP_FAILURE_RETRY(read(srcfd, buf, sizeof(buf)))) != 0) {
		if (nread < 0) {
			printf("Error reading \"%s\": %s", SUPP_CONFIG_TEMPLATE, strerror(errno));
			close(srcfd);
			close(destfd);
			unlink(config_file);
			return -1;
		}
		TEMP_FAILURE_RETRY(write(destfd, buf, nread));
	}

	fsync(destfd);
	close(destfd);
	close(srcfd);

	/* chmod is needed because open() didn't set permisions properly */
	if (chmod(config_file, 0660) < 0) {
		printf("Error changing permissions of %s to 0660: %s",
			 config_file, strerror(errno));
		unlink(config_file);
		return -1;
	}

	return update_ctrl_interface(config_file);
}
#endif
static int _wifi_client(struct p2p *p,int save_profile)
{
    FILE *pf = NULL;
    char file_name[64];
	int count = 0;

    if(p->wpa_open==_TRUE){
         return -1;
    }
    else{
         p->wpa_open = _TRUE;
    }

    wifi_direct_get_ifaddr(p);
    
    p2p_info("start wpa_supplicant:%s\n",p->peer_ifaddr);
    wifi_direct_wpa_conf_file(p,file_name);
	#if WIFI_DIRECT_NEW_FLOW
	//memset(wpa_file_name, 0, sizeof(wpa_file_name));
	//strncpy(wpa_file_name, file_name, sizeof(wpa_file_name));
    /*start wpa_supplicant servers*/
	_start_wpa_conf(file_name);
    //__wifi_direct_sleep(1);
	while(count < 5){
		usleep(200*1000);
		int ret = widi_connect_wpas();
		if(ret<0){
			printf("CONNECT TO SUPPLICANT ERROR, TRY AGAIN %d\n", count);
		}else{
			printf("CONNECT TO SUPPLICANT SUCCESS, TRY TIME %d\n", count);
			widi_status_monitor(p);
			break;
		}
		count++;
	}
	
    /*handle wps information*/
    wifi_direct_wps(p);
	#else
	/*start wpa_supplicant servers*/
	_start_wpa_conf(file_name);
	
    __wifi_direct_sleep(1);

	/*handle wps information*/
    wifi_direct_wps(p);
	
    if((widi_status_check())!=0){
         /** 
         * if failed, do not save the profile.
         */
         save_profile = 0;
    }     

    if(save_profile){
         //sprintf(cmd,"wpa_cli -p/tmp/wpa_supplicant save_config");
         //__wifi_direct_system(cmd);
         _wifi_direct_save_profile(p,file_name);
    }
    if(_wifi_direct_clientMode_connect_status(p) ==0){
         p->connect_flag = WIDI_CONNECT_CLIENT_OK;                         //the flag is identy client connect ok
    }
    #endif
//	p->polling_status_flage=0;
    p->wpsing = _FALSE;

    return 0;
}

/* the client mode device start the servers*/
static void wifi_direct_client_mode(struct p2p *p)
{
    _wifi_client(p,1);
}

static void wifi_direct_go_mode(struct p2p *p)
{
    int count = 0, i = -1;
    char addr_lower[18];
    char cmd[CMD_SZ]={0x00};
 //   int udhcpd_flag=0;
    //>>> get interface address
    wifi_direct_get_ifaddr(p); 
    //memset( cmd, 0x00, CMD_SZ );
 #if !WIFI_STA_P2PAUTOGO_ENABLE
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
       #if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		sprintf(cmd, "ifconfig %s 192.168.203.1", p->ifname);
	#else
		sprintf(cmd, "ifconfig %s 192.168.168.1", p->ifname);
	#endif	
#else
	sprintf(cmd, "ifconfig %s 192.168.203.1", p->ifname);
#endif	
#else
	sprintf(cmd, "ifconfig %s 192.168.111.1", p->ifname);
#endif
    __wifi_direct_system(cmd);
    usleep(5000);
#endif

    if(p->ap_open != _TRUE)
    {
         printf(">>>>>start hostapd\n");
         /** chaneg operation channel first */
         wifi_direct_change_op_channel(p);
         memset( cmd, 0x00, CMD_SZ );
#if WIFI_DIRECT_HOSTAPD_DEBUG
         sprintf(cmd, "hostapd %s -dd -t &",WIFI_DIRECT_HOSTAPD_CONF);
#else
         sprintf(cmd, "hostapd %s &",WIFI_DIRECT_HOSTAPD_CONF);
#endif
         __wifi_direct_system( cmd );

         /**
         * sleep is not ok, we must do some active check to make sure the hostapd
         * really starts ok.
         */
         sprintf(cmd,"/tmp/hostapd/%s",p->ifname);
         count=0;
         while(count < 30){
              usleep( HOSTAPD_INIT_TIME );
              if(access(cmd,F_OK)==0){
                   printf(">>> hostapd start ok\n");
                   break;
              }
         }

         p->ap_open = _TRUE;
    }
    wifi_direct_wps(p); 

	//sprintf(cmd,"tcpdump -i %s -w /mnt/udisk/1.cap &",p->ifname);
	//system(cmd);
#if WIFI_STA_P2PAUTOGO_ENABLE
	__wifi_direct_system("rm -rf /tmp/udhcpd.leases");
	__wifi_direct_system("touch /tmp/udhcpd.leases");
#else
	memset(cmd,0x00,CMD_SZ);
	//sprintf(cmd, "ifconfig %s 192.168.111.1",p->ifname);
	//__wifi_direct_system(cmd);
	//>>> start udhcpd servers               
	__wifi_direct_system("rm -rf /tmp/udhcpd.leases");
	__wifi_direct_system("touch /tmp/udhcpd.leases");
#if WIFI_DIRECT_DNSMASQ
	printf(">>>>>start dnsmasq\n");
	sprintf(cmd,"/sbin/dnsmasq -d -C %s &",WIFI_DIRECT_UDHCPD_CONF);
	__wifi_direct_system(cmd);
#else
	printf(">>>>>start udhcpd\n");
	sprintf(cmd,"udhcpd %s",WIFI_DIRECT_UDHCPD_CONF);
	__wifi_direct_system(cmd);
#endif
#endif
    strncpy(p->mac_connect_ok,p->peer_devaddr,strlen(p->peer_devaddr));
    p->connect_flag = WIDI_CONNECT_SOFTAP_OK;
    modify_wifi_mode(WIFI_DIRECT_SOFTAP_OK);
    //After starting hostapd and doing WPS connection successful,
    //We create a thread to query driver if some other p2p devices connected.
#if 0
    wifi_direct_status(p);
    p->polling_status_flage=0;
    p->res= 0;//bq:add
    p->res_go = pthread_create(&p->pthread_go, NULL, &wifi_polling_client, (void *)p);

    if( p->res_go != 0 )
    {
         p2p_err( "<%s %d>Thread creation failed\n",__FUNCTION__,__LINE__ );
    }
#endif
    p->wpsing = _FALSE;
}

/*the function is prov discover.determine which is client mode or which is softap mode*/
static int wifi_direct_set_nego(struct p2p *p)
{
    FILE *pf=NULL;
    int retry_count = 0, success = 0,conut_nego = 0;
    int retry = NEGO_RETRY_INTERVAL, query = NEGO_QUERY_INTERVAL;
    char cmd[CMD_SZ]={0x00};
    int ret=0;
    
    printf(">>> start wifi direct negotiation:%s\n",p->peer_devaddr);
    //memset( p->cmd, 0x00, CMD_SZ );
///    sprintf( cmd, "iwpriv %s p2p_set nego=%s ", p->ifname, p->peer_devaddr);
///    __wifi_direct_system( cmd );

    debug_info("p->skfd=====%d",p->skfd);
    sprintf( cmd, "nego=%s ", p->peer_devaddr);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
    debug_info("p->skfd=====%d",p->skfd);
    usleep( PRE_NEGO_INTERVAL );
    
    wifi_direct_status(p);
    printf(">>>>>>>>>>>>>>start nego\n");
    while( !wifi_direct_nego_ok(p) && (retry_count < 120 / NEGO_QUERY_INTERVAL ))
    {
         retry_count++;          
         if( (retry_count % ( retry / query ) )==0 ){
              conut_nego++;
              memset( cmd, 0x00, CMD_SZ );
//              sprintf( cmd, "iwpriv %s p2p_set nego=%s ", p->ifname, p->peer_devaddr);
//              __wifi_direct_system( cmd );               

              debug_info("p->skfd=====%d",p->skfd);
              sprintf( cmd, "nego=%s ", p->peer_devaddr);
              iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
              debug_info("p->skfd=====%d",p->skfd);
              usleep( NEGO_QUERY_INTERVAL);
              wifi_direct_status(p);
         }
         else{               
              usleep( NEGO_QUERY_INTERVAL );
              wifi_direct_status(p);
         }
         //printf(">> stat=%d\n",p->status);
    
         if(conut_nego == 100){
              break;
         }     
    }
	printf(">>>>>>>>>>>>>>the count_nego is %d\n",conut_nego);
	#if WIFI_DIRECT_NEW_FLOW
	mixed_mode_change(MIXED_MODE_P2P);
	#endif
    if( wifi_direct_nego_ok(p) )
    {
         wifi_direct_role(p );
         printf("wifi direct handle ok role=%d\n",p->role);
         wifi_direct_get_ifaddr(p);
//		p->polling_status_flage = 0;
#if WIFI_DIRECT_TCPDUMP
	/**add tcpdump*/
	char cmd[64] = {0 };
	memset(cmd,0x00,CMD_SZ);
	printf(">>>>start tcpdump\n");
	sprintf(cmd,"tcpdump -i %s -w /mnt/udisk/miracast.cap &",p->ifname);
	__wifi_direct_system(cmd);
	/******/
#endif

	if( p->role == P2P_ROLE_CLIENT ){
              wifi_direct_client_mode(p);
         }
         else if( p->role == P2P_ROLE_GO )
         {
              wifi_direct_go_mode(p);
         }
         ret = 0;
    }
    else
    {
         //wifi_direct_status(p);
         p->connect_flag = WIDI_STATUS_PROV_FAILURE;
         ret = -1;
         printf(">>> negotiation failed\n");
         //p2pdbg_info("debug status:%d\n",p->status);
         
         /*          
         if(p->pthread_runing==0){
                   p->res = pthread_create(&p->pthread, NULL, &wifi_direct_polling_status, (void *)p);
                   p->pthread_runing = 1;
              if(p->res != 0)
                        p2p_err("creat pthread errno\n");
         }
         */
    }

    return ret;
}

/** 
* start provide discovery,decede which is client or which is softap
* default one method
*/
int wifi_direct_provide_discovery(int idex_ap)
{
    int wps_cm, retry_count=0;
    char prov[100] = { 0x00 };
    char cmd[CMD_SZ]={0x00};
    struct p2p *p=NULL;
    
    p = &action_p2p;
    wps_cm= 2;//default push button
    printf("<%s %d>start prov disc  peer_device: %s \n",__FUNCTION__,__LINE__,p->peer_devaddr);
    
    #if 0
    if( wps_cm == 0 )  // peer device input c continue
         sprintf( p->cmd, "iwpriv %s p2p_set prov_disc=%s_display", p->ifname, p->peer_devaddr);
    else if( wps_cm == 1 )
         sprintf( p->cmd, "iwpriv %s p2p_set prov_disc=%s_keypad", p->ifname, p->peer_devaddr);
    else if( wps_cm == 2 )
         sprintf( p->cmd, "iwpriv %s p2p_set prov_disc=%s_pbc", p->ifname, p->peer_devaddr);
    else if( wps_cm == 3 )
         sprintf( p->cmd, "iwpriv %s p2p_set prov_disc=%s_label", p->ifname, p->peer_devaddr);     
    #endif
    /**
    * Use PBC for provision discovery.
    */
//    sprintf( cmd, "iwpriv %s p2p_set prov_disc=%s_pbc", p->ifname, p->peer_devaddr);
//    __wifi_direct_system( cmd );     

    debug_info("p->skfd=====%d",p->skfd);
    sprintf( cmd, "prov_disc=%s_pbc",  p->peer_devaddr);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
    debug_info("p->skfd=====%d",p->skfd);
    strncpy( prov, cmd,strlen(cmd));
    usleep(PROV_WAIT_TIME);
    
    p->status=wifi_direct_status( p);
    
    while( p->status != P2P_STATE_RX_PROVISION_DIS_RSP && retry_count < MAX_PROV_RETRY){
         usleep( PROV_WAIT_TIME );
         retry_count++;
         wifi_direct_status( p);
         if( (retry_count % PROV_RETRY_INTERVAL) == 0){
              __wifi_direct_system( prov );
         }
    }

    if( p->status == P2P_STATE_RX_PROVISION_DIS_RSP){
         switch(wps_cm)
         {
              case 0: p->wps_info=1;     break;
              case 1: p->wps_info=2;     break;
              case 2: p->wps_info=3;     break;
              case 3: p->wps_info=1;     break;
         }
         
         if( wps_cm==1 || wps_cm==2 ){
              
              wifi_direct_set_wpsinfo(p);
              if(p->connect_go == 1)
                   wifi_direct_client_mode(p);
              else
                   wifi_direct_set_nego(p);
              
         }
         else if( wps_cm==0 || wps_cm==3 ){               
              /*bq check carefully*/
              /*     
              usleep(500);
              p->pin = 12345670; //default pin code
              wifi_direct_wpsinfo(p);     

              if(p->connect_go == 1)
                   wifi_direct_client_mode(p);
              else
                   wifi_direct_set_nego(p);     
              */
         }
    }
    else{                         
         p2pdbg_info("debug status:%d\n",p->status);
         p->connect_flag = WIDI_STATUS_PROV_FAILURE;
         /*     
         if(p->pthread_runing==0){
              p->res = pthread_create(&p->pthread, NULL, &wifi_direct_polling_status, (void *)p);
              p->pthread_runing = 1;
              if(p->res != 0)
                   p2p_err("creat pthread errno\n");
         }
         */     
    }
    return 0;
}

int wifi_direct_nego_ok(struct p2p *p)
{
    int ret = 0;     
    if( p->status == P2P_STATE_GONEGO_OK )
         ret = 1;
    
    return ret;
}

/* After negotiation success, get peer device's interface address.*/
static int wifi_direct_get_ifaddr(struct p2p *p)
{
    FILE *pf=NULL;
    int i;     
///    char parse[CMD_SZ]={0x00};
    char *parse=NULL;
    char cmd[CMD_SZ]={0x00};
    
    /* peer_ifaddr */
    /*
    sprintf( cmd, "iwpriv %s p2p_get peer_ifa > /tmp/amwd_ifa.txt", p->ifname);
    __wifi_direct_system(cmd);
         
    pf = fopen("/tmp/amwd_ifa.txt", "r" );
    if ( pf )
    {
         while( !feof( pf ) ){
              memset( parse, 0x00, CMD_SZ );
              fgets( parse, CMD_SZ, pf );
              if( strncmp( parse, "MAC", 3) == 0 )
              {
                   strncpy( p->peer_ifaddr, parse+4, 17 );
                   break;
              }     
         }     
         fclose( pf );
    }
*/
 //   debug_info("p->skfd=====%d",p->skfd);
    iwprivExecute(p->skfd,p->ifname,"p2p_get","peer_ifa",cmd);
    //printf("%s \n",parse);
    printf("%s %d ---bluse------ifname:%s cmd:%s\n",__func__,__LINE__,p->ifname,cmd);
    if( (parse=strstr(cmd, "MAC") )!= NULL )
    {
         strncpy( p->peer_ifaddr, parse+4, 17 );
    }     

    return 0;
}


/***
* get the peer mac address if we are invited to join a group. 
*/
static int wifi_direct_get_invaddr(struct p2p *p)
{
    FILE *pf=NULL;
    int i;     
///    char parse[CMD_SZ]={0x00};
    char *parse=NULL;
    char cmd[CMD_SZ]={0x00};
/*    
    sprintf(cmd, "iwpriv %s p2p_get inv_peer_deva > /tmp/amwd_invaddr.txt", p->ifname);
    __wifi_direct_system(cmd);
    
    pf = fopen("/tmp/amwd_invaddr.txt", "r" );
    if(pf){
         while( !feof( pf ) ){
              memset( parse, 0x00, CMD_SZ );
              fgets( parse, CMD_SZ, pf );
              if(strncmp( parse, "MAC", 3) == 0)
              {
                   strncpy(p->peer_devaddr, parse+4, 17);
                   break;
              }     
         }     
         fclose( pf );
    }
*/
    debug_info("p->skfd=====%d",p->skfd);
    iwprivExecute(p->skfd,p->ifname,"p2p_get","inv_peer_deva",cmd);
    debug_info("p->skfd=====%d",p->skfd);
    if((parse=strstr( cmd, "MAC")) != NULL)
    {
         strncpy(p->peer_devaddr, parse+4, 17);
    }     
    
    return 0;
}

int _widirect_set_nego(struct p2p *p)
{
    p->wps_info=3;
    
    wifi_direct_set_wpsinfo(p);
    wifi_direct_status(p);
    
    if(p->status != P2P_STATE_GONEGO_OK){          
         wifi_direct_set_nego(p);
    }
    else{
//		p->polling_status_flage=0;
         wifi_direct_role(p);                              
         if( p->role == P2P_ROLE_CLIENT ){
              wifi_direct_client_mode(p);
         }
         else if( p->role == P2P_ROLE_GO ){
              wifi_direct_go_mode(p);
         }
    }
    return 0;
}

int _widirect_cancle_pbc_request(struct p2p *p)
{
    int ret = -1,handle = 1;
    char cmd[CMD_SZ]={0x00};
    
    p->intent = 1;
    reinit_wifi_direct_function(p);
    p->role=wifi_direct_role(p);
    if(p->role == P2P_ROLE_DEVICE){
         ret = 0;
    }
    
    /*     
    if(p->pthread_runing==0){
              p->res = pthread_create(&p->pthread, NULL, &wifi_direct_polling_status, (void *)p);
              p->pthread_runing = 1;
              if(p->res != 0)
                        p2pdbg_info("creat pthread errno\n");
    }
    */
    
    p->polling_status_flage=0;
    wifi_direct_send_msg(WIDI_CMD_SCAN);//send scan ap message
    return ret;
}

int _identify_ap_connectok(struct p2p *p)
{
    int ap_idex = -1,i = 0;
    
    for(i = 0;i<p->ap_total;){
         if(strncmp(p->scan_pool[i].mac_addr,p->mac_connect_ok,strlen(p->mac_connect_ok))==0){
              ap_idex = i;
              break;
         }
         if(strncmp(p->mac_connect_ok,"00:00:00:00:00:00",strlen("00:00:00:00:00:00"))==0){
              ap_idex = -1;
              break;
         }
         i++;          
    }
    return  ap_idex;
}

void __wifi_direct_get_name(int sel_ap,char *ssid_buf,int buf_len)
{
    memset(ssid_buf,0,buf_len);
    strncpy(ssid_buf,action_p2p.scan_pool[sel_ap].widi_device_name,strlen(action_p2p.scan_pool[sel_ap].widi_device_name)-1);
}

//disconnect 
static int wifi_direct_disconnect()
{
    int ret = -1;
    struct p2p *p = NULL;
    char call_buf[128]={0x00};
    
    p = &action_p2p;
    if(p->ap_open == _TRUE){
#if WIFI_DIRECT_DNSMASQ
	sprintf(call_buf,"killall hostapd\nkillall dnsmasq\n");
#else
		 sprintf(call_buf,"killall hostapd\nkillall udhcpd\n");
#endif
    }
    else if(p->wpa_open==_TRUE){
         //sprintf(call_buf,"wpa_cli -p/tmp/wpa_supplicant disconnect\n");
         sprintf(call_buf,"killall wpa_supplicant\nkillall udhcpc\n");
    }
    memset(p->mac_connect_ok,0x00,32);// clear ui connect flag
    ret=__wifi_direct_system(call_buf);
    //if(wifi_direct_role(p)!=P2P_ROLE_DEVICE){
    usleep(500);
    reinit_wifi_direct_function(p);
    //}
    return ret;
}

static int _wifi_direct_check_reject_status(struct p2p *p)
{
    int rtn = -1;
    static int reject_num=0;
    
    p->connect_flag = WIDI_CONNECT_DEFAULT;
    p->status=wifi_direct_status(p);
    if(p->status==P2P_STATE_RX_PROVISION_DIS_RSP||p->status==P2P_STATE_RX_PROVISION_DIS_REQ||p->status==P2P_STATE_GONEGO_FAIL){
         reject_num++;
    }else{
         reject_num=0;
    }

    if(reject_num==3){
         reject_num=0;
         rtn = 0;
    }
    return rtn;
}

/**/
static int wifi_direct_check_goMode_connect_status(struct p2p *p)
{
#if 0
    int rtn = -1;
    int connect_nums = -1;
    static int flags=0;

    
    //p2pdbg_info("debug flags:%d\n",flags);
    if((flags++) >=3){
         flags=0;
         connect_nums=wifi_read_all_ap(p);
    }
    
    //p2pdbg_info("debug connect_nums:%d\n",connect_nums);
    if(connect_nums ==0){
         /*if connect to ap nums = 0,the peer device is disconnet*/
         reinit_wifi_direct_function(p);
         rtn = 0;
    }
    else{
         usleep(5000);
    }
    return rtn;
#endif

    if(wifi_read_all_ap(p) != 0){
         return 0;
    }
    else{
         return 1;
    }

}

/**
* @brief After we got provision request from peer device and got the config method we 
*   should call this function to do negotiation. Before we call this function, update 
*   the pin code if we need to input from UI.
*
* @param: cm the config method.
*/
static int __wifi_direct_do_negotiation(struct p2p *p,int cm)
{
    int cnt = 0;
    int ret=0;

    __dump_p2p(p);
    
    if(cm == WIDI_PIN_PBC_DEFAULT){
         //>>> PBC
         p->wps_info=3;
         wifi_direct_set_wpsinfo(p);
         wifi_direct_status(p);
    }
    else if(cm == WIDI_PIN_PBC_DISPLAY){
         //>>> self display pincode
         /**
         * if we display the pincode, the negotiation request will be initiated 
         * by the peer device,so we wait until the negotiation completed.
         */
         p->wps_info=2;
         wifi_direct_set_wpsinfo(p);

         do{
              __wifi_direct_sleep(1);
              wifi_direct_status(p);
              cnt++;
              if(cnt > 20){
                   printf(">>>>>>>>>>>> nego not complete for too long a time\n");
                   break;
              }
         }while(p->status != P2P_STATE_GONEGO_OK);
    }
    else if(cm == WIDI_PIN_PBC_INPUT){
         //>>> peer display pincode
         p->wps_info=1;
         wifi_direct_set_wpsinfo(p);
         wifi_direct_status(p);
    }

    /**
    * do the real negotiation.
    */
    wifi_direct_role(p);
	printf("%s %d ---------------role:%d status:%d\n",__func__,__LINE__,p->role,p->status);

    if(p->role == P2P_ROLE_GO && p->start_role == P2P_ROLE_GO){
         /** start as group owner */
//		p->polling_status_flage=0;
         wifi_direct_go_mode(p);
    }
    else{
         /** start as device, need to negotiate the role */
         if(p->status != P2P_STATE_GONEGO_OK){     
              ret=wifi_direct_set_nego(p);
         }
         else{
//			p->polling_status_flage=0;
              wifi_direct_get_ifaddr(p);
              if( p->role == P2P_ROLE_CLIENT ){
                   wifi_direct_client_mode(p);
              }
              else if( p->role == P2P_ROLE_GO ){
                   wifi_direct_go_mode(p);
              }
              else{
                   printf(">>> unknown P2P role\n");
                   ret = -1;
              }
         }
    }

    return ret;
}

/**
* join a group if being invited.
*/
static int __wifi_direct_join_group(struct p2p *p)
{
    int i;
    char cmd[128];
    
    wifi_direct_role(p);
    wifi_direct_get_ifaddr(p);
    wifi_direct_get_invaddr(p);
    printf(">>> role=%d,ifa=%s,devmac=%s\n",p->role,p->peer_ifaddr,p->peer_devaddr);
    wifi_p2p_get_device_name(p,-1);                    //get the device name for json file
    
    i=0;
    while(i<5){
         if(wifi_direct_scan_and_find_dev(p,p->peer_ifaddr)==0){
              printf(">>> find dev in scan list\n");
              break;
         }
         i++;
         printf(">>> not find dev in scan list:%d\n",i);
    }
    if(i>=5){
         return -1;
    }

    /** do provision discovery */
    printf(">>> begin provision discovery\n");
///    sprintf(cmd, "iwpriv %s p2p_set prov_disc=%s_keypad", p->ifname,p->peer_devaddr);
///    __wifi_direct_system(cmd);

    debug_info("p->skfd=====%d",p->skfd);
    sprintf(cmd, "prov_disc=%s_keypad",p->peer_devaddr);
    iwprivExecute(p->skfd,p->ifname,"p2p_set",cmd,NULL);
    debug_info("p->skfd=====%d",p->skfd);

    __wifi_direct_sleep(2);

    p->wps_info=1;
    p->connect_go = 1;
    wifi_direct_set_wpsinfo(p);

    printf(">>> start WPA\n");
    wifi_direct_client_mode(p);

    return 0;
    
}

#if WIFI_DIRECT_NEW_FLOW
static int wifi_direct_join_persistant_group(struct p2p *p){
	int i;
    char cmd[128];
    char vramfile[64]={0},tmpfile[64]={0};
    
    wifi_direct_get_ifaddr(p);

    /**
    * 1.Check if the connection configure file exist.
    * 2.If exist,copy to /tmp/ and start wpa_supplicant servers.
    * 3.If not return error.
    */
    sprintf(vramfile,"/mnt/user1/%x.conf",APMac_Hash(p->peer_ifaddr));
    sprintf(tmpfile,"/tmp/%x.conf",APMac_Hash(p->peer_ifaddr));
    if(access(vramfile,F_OK)==0){
         printf(">>>>>> persistant configure file %s exists\n",vramfile);
         _wifi_direct_copy_file(vramfile,tmpfile);
    }
    else{
         printf(">>>>>> persistant configure file NOT exists\n");
         if(access(WIFI_DIRECT_PROFILE_LIST_DISK,F_OK)==0){
		if(access(tmpfile,F_OK)==0)
			remove(tmpfile);
		remove(WIFI_DIRECT_PROFILE_LIST);			//remove the profile file
		remove(WIFI_DIRECT_PROFILE_LIST_DISK);
         }
         return -1;
    }

    if(access(tmpfile,F_OK)==0){
         _start_wpa_conf(tmpfile);
         p->wpa_open = _TRUE;
		int count = 0;
		while(count < 5){
			usleep(200*1000);
			int ret = widi_connect_wpas();
			if(ret<0){
				printf("CONNECT TO SUPPLICANT ERROR, TRY AGAIN %d\n", count);
			}else{
				printf("CONNECT TO SUPPLICANT SUCCESS, TRY TIME %d\n", count);
				widi_status_monitor(p);
				break;
			}
			count++;
		}
//		p->polling_status_flage=0;
         p->wpsing = _FALSE;
         p->role = P2P_ROLE_CLIENT;

         return 0;
    }
    else{
         return -1;
    }
}
#endif
static int __wifi_direct_join_persistant_group(struct p2p *p)
{
    int i;
    char cmd[128];
    char vramfile[64]={0},tmpfile[64]={0};
    
    wifi_direct_get_ifaddr(p);

    /**
    * 1.Check if the connection configure file exist.
    * 2.If exist,copy to /tmp/ and start wpa_supplicant servers.
    * 3.If not return error.
    */
    sprintf(vramfile,"/mnt/user1/%x.conf",APMac_Hash(p->peer_ifaddr));
    sprintf(tmpfile,"/tmp/%x.conf",APMac_Hash(p->peer_ifaddr));
    if(access(vramfile,F_OK)==0){
         printf(">>>>>> persistant configure file %s exists\n",vramfile);
         _wifi_direct_copy_file(vramfile,tmpfile);
    }
    else{
         printf(">>>>>> persistant configure file NOT exists\n");
         if(access(WIFI_DIRECT_PROFILE_LIST_DISK,F_OK)==0){
		if(access(tmpfile,F_OK)==0)
			remove(tmpfile);
		remove(WIFI_DIRECT_PROFILE_LIST);			//remove the profile file
		remove(WIFI_DIRECT_PROFILE_LIST_DISK);
         }
         return -1;
    }

    if(access(tmpfile,F_OK)==0){
		//ensure_config_file_exists(P2P_CONFIG_FILE);
    	//_start_wpa_conf(P2P_CONFIG_FILE);
         _start_wpa_conf(tmpfile);
         p->wpa_open = _TRUE;

         /** check connection status */
         if(widi_status_check()!=0){
              if(_wifi_direct_copy_profile(p,WIFI_DIRECT_PROFILE_LIST,WIFI_DIRECT_PROFILE_LIST_TMP)==0){
                   _wifi_direct_copy_file(WIFI_DIRECT_PROFILE_LIST_TMP,WIFI_DIRECT_PROFILE_LIST);
                   _wifi_direct_copy_file(WIFI_DIRECT_PROFILE_LIST_TMP,WIFI_DIRECT_PROFILE_LIST_DISK);
                   remove(WIFI_DIRECT_PROFILE_LIST_TMP);
              }
              else{
                   remove(WIFI_DIRECT_PROFILE_LIST_DISK);
                   remove(WIFI_DIRECT_PROFILE_LIST);
              }
              remove(tmpfile);
              remove(vramfile);
         }
         
         if(_wifi_direct_clientMode_connect_status(p) ==0){
              p->connect_flag = WIDI_CONNECT_CLIENT_OK;//the flag is identy client connect ok
         }
	         
//		p->polling_status_flage=0;
         p->wpsing = _FALSE;
         p->role = P2P_ROLE_CLIENT;
         
         return 0;
    }
    else{
         return -1;
    }
    
}


static int _wifi_direct_generate_pincode(struct p2p *p)
{
    char pincode[9]={0};
    int i;

    for(i=0;i<8;i++){
         pincode[i] = '0'+(rand()%9)+1;
    }

    p->pin = atoi(pincode);

    return 0;
}

static int _wifi_direct_check_connection_status(struct p2p *p)
{
    char cmd[64];
    
    if(p->ap_open == _TRUE){
         /// in GO mode
         sprintf( cmd, "hostapd_cli -p/tmp/hostapd all_sta > /tmp/amwd_connect_stat.txt", p->pin);
         
    }
    else if(p->wpa_open == _TRUE){
         /// in client mode
         
    }

    return -1;
}


/**
* For miracast only.
* Get the peer RTSP port.
*/
static int _wifi_direct_get_peer_port(struct p2p *p)
{
    char cmd[48];
    char *locate;
/*
    sprintf(cmd,"iwpriv %s p2p_get peer_port > /tmp/port.log",p->ifname);
    __wifi_direct_system(cmd);
*/
    FILE *fp=NULL;
    int filedes=-1;
    
    debug_info("p->skfd=====%d",p->skfd);

    iwprivExecute(p->skfd,p->ifname,"p2p_get","peer_port",cmd);
//    printf("cmd is %s",cmd);
    debug_info("p->skfd=====%d",p->skfd);

    locate = strstr(cmd,"Port=");

    if(locate)
         p->peer_port = atoi(locate+5);

    fp=fopen("/tmp/port.log","w");
    if(fp==NULL){
         perror("open port.log error!");
         return -1;
    }
    if(fwrite(cmd,sizeof(char),strlen(cmd),fp) <= 0){
         fclose(fp);
         perror("write port.log error!");
         return -1;
    }
    fflush(fp);
    filedes=fileno(fp);
    fsync(filedes);
    fclose(fp);
    return 0;
}


static int _to_upper_case(char *str)
{
    int len;
    int i;
    
    if(str == NULL){
         return -1;
    }

    len = strlen(str);
    for(i=0;i<len;i++){
         if(*(str+i) >= 'a' && *(str+i) <= 'f'){
              *(str+i) = *(str+i) - 'a' + 'A';
         }
    }

    return 0;
}

static int _to_lower_case(char *str)
{
    int len;
    int i;
    
    if(str == NULL){
         return -1;
    }

    len = strlen(str);
    for(i=0;i<len;i++){
         if(*(str+i) >= 'A' && *(str+i) <= 'F'){
              *(str+i) = *(str+i) - 'A' + 'a';
         }
    }

    return 0;
}


/**
* @brief In client mode, after we have got the ip address, we need to 
*   ping the gate way for once to invalidate the ARP cache.
*/
static int _wifi_direct_ping_route(struct p2p *p)
{
    int cnt=0;
    FILE *fp=NULL;
    char line[128];
    char *ptr1,*ptr2;
    char gatewy[32]={0};
    int i,ping_ok=0;

    while(cnt<5){
         __wifi_direct_system("route -e > /tmp/amwd_route.txt");
         fp = fopen("/tmp/amwd_route.txt","r");
         if(fp){
              while(!feof(fp)){
                   memset(line, 0x00, 128);
                   fgets(line, 128, fp);
                   ptr1 = strstr(line,"default");
                   ptr2 = strstr(line ,p->ifname);
                   if(ptr1 && ptr2){
                        /** gateway exist */
                        ptr1+=7;
                        while(*ptr1 == ' ') ptr1++;
                        i=0;
                        while(*ptr1 != ' '){
                             gatewy[i] = *ptr1;
                             i++;
                             ptr1++;
                        }
                        printf("gateway is %s\n",gatewy);

                        /** ping the gateway for once */
                        sprintf(line,"ping -c 1 %s",gatewy);
                        __wifi_direct_system(line);
                        ping_ok = 1;
                        break;
                   }
              }     
              fclose(fp);
         }

         if(ping_ok){
              break;
         }
         sleep(1);
         cnt++;
    }

    return 0;
}

/**
* Get the Miracast source IP address.
*/
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

static int _wifi_direct_get_gw_shortcut(struct p2p *p,char *ip)
{
    FILE *fp;
    char buf[64]={0};
    char *intf_ptr,*ip_ptr;
    int i,len;
    int ret = -1;

    if((p == NULL) || (ip == NULL)){
         return ret;
    }
	#if 0	
    __wifi_direct_system("cat /tmp/amgw.txt");
    
    fp = fopen("/tmp/amgw.txt","r");
    if(fp == NULL){
         return ret;
    }
	#else
	i = 0;
	while(i < 30){
		if(access("/tmp/amgw.txt",F_OK)==0){
			__wifi_direct_system("cat /tmp/amgw.txt");
			fp = fopen("/tmp/amgw.txt","r");
    		if(fp == NULL){
         		return ret;
   			}
			break;
		}else{
			printf("cannot open /tmp/amgw.txt ,try again.\n");
		}
		usleep(500*1000);
		i++;
	}
	#endif
    /**
    * In the form --> intf=$interface,gw=$i,
    */

    fgets(buf, 64, fp);
    intf_ptr = strstr(buf, "intf=");
    ip_ptr = strstr(buf, "gw=");

    if(intf_ptr==NULL || ip_ptr==NULL ){
         fclose(fp);
         return ret;
    }
    intf_ptr += 5;
    ip_ptr += 3;
    
    len = strlen(buf);
    for(i=0;i<len;i++){
         if(buf[i] == ',') buf[i]=0;
    }

    if((strcmp(intf_ptr,p->ifname)==0) && (strlen(ip_ptr)>0)){
         strcpy(ip,ip_ptr);
         ret = 0;
    }

    fclose(fp);

    return ret;
    
}

#if WIFI_DIRECT_NEW_FLOW
JSON *jsonDataGetFromFile(const char *filename, WIFI_DIRECT_JSON_STATE *error)
{
	unsigned long filesize = -1;
	FILE *fp = NULL;
	char *fileStream = NULL;
	JSON *js = NULL;

	fp = fopen(filename, "r");
	if(NULL == fp){
		printf("open file error\n");
		*error = JSON_ERROR_OPEN_FILE;
		goto ERROR;
	}
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, SEEK_SET, 0);
	
	fileStream = (char *)malloc(filesize);
	if(NULL == fileStream){
		printf("malloc error\n");
		*error = JSON_ERROR_MALLOC_FAIL;
		goto ERROR;
	}

	fread(fileStream, 1, filesize, fp);
	fclose(fp);
	fp = NULL;

	js = JSON_Parse(fileStream);
	if(NULL == js){
		perror("JSON_Parse");
		*error = JSON_ERROR_GET_JSON;
		goto ERROR;
	}
	free(fileStream);
	fileStream = NULL;
	*error = JSON_OK;

	return js;

	ERROR:
		if(fp){
			fclose(fp);
			fp = NULL;
		}
		if(fileStream){
			free(fileStream);
			fileStream = NULL;
		}

		return NULL;
}

JSON *isJsonHaveItem(JSON *json, const char *itemName){
	if(NULL == json){
		printf("json data error!\n");
		return NULL;
	}
	JSON *item = JSON_GetObjectItem(json, itemName);
	if(NULL == item){
		printf("item json data error!\n");
		return NULL;
	}
	return item;
}

int setStringValue2Json(JSON *js, const char *key, const char *strValue)
{
	int ret = -1;
	JSON *jsKey = NULL;

	if(NULL == js || NULL == key || NULL == strValue){
		printf("Parameters error!!!\n");
		return -1;
	}
	jsKey = JSON_GetObjectItem(js, key);
	if(NULL == jsKey){
		printf("Have not item \"%s\"\n", key);
		ret = -1;
		JSON_AddStringToObject(js, key, strValue);
		//goto __END__;
	}else{
		JSON *jsString = JSON_CreateString(strValue);
		if(jsString != NULL){
			JSON_ReplaceItemInObject(js, key, jsString);
			ret = 0;
		}else{
			goto __END__;
		}
	}	
__END__:
	jsKey = NULL;

	return ret;
}

int jsonDataWrite2File(const char *filename, JSON *js)
{
	char *str = NULL;
	int ret = -1;
	FILE *fp = NULL;

	if(filename == NULL || js == NULL){
		printf("Parameters error!!!\n");
		goto __END__;
	}

	str = JSON_Print(js);
	if(str == NULL){
		printf("Print JSON fail!!!\n");
		goto __END__;
	}

	fp = fopen(filename, "w");
	if(fp == NULL){
		printf("Open [%s] fail!!!\n", filename);
		perror("error");
		goto __END__;
	}

	ret = fwrite(str, 1, strlen(str), fp);
	if(ret <= 0){
		printf("Write [%s] fail!!!\n", filename);
		ret = -1;
		goto __END__;
	}
	ret = 0;

__END__:
	if(fp != NULL) {
		fclose(fp);
		sync();
	}
	if(str != NULL) 
		free(str);

	return ret;
}

WIFI_DIRECT_JSON_STATE wifi_direct_set_json_to_file(const char *filename,const char *itemName,
															const char *subItemName_1,const char *sbuItemVal_1,
															const char *subItemName_2,const char *sbuItemVal_2,
															const char *subItemName_3,const char *sbuItemVal_3)
{
	JSON *js = NULL;
	JSON *item = NULL;
	WIFI_DIRECT_JSON_STATE status = JSON_OK;

	js = jsonDataGetFromFile(filename, &status);
	if(NULL == js){
		printf("get json data from file fail!\n");
		return status;
	}

	item = isJsonHaveItem(js, itemName);
	if(item){
		printf("upate %s content\n", itemName);		
	}else{
		printf("add %s content\n", itemName);
		JSON_AddItemToObject(js, itemName, item = JSON_CreateObject());
	}
	setStringValue2Json(item, subItemName_1, sbuItemVal_1);
	setStringValue2Json(item, subItemName_2, sbuItemVal_2);
	setStringValue2Json(item, subItemName_3, sbuItemVal_3);

	jsonDataWrite2File(filename, js);

	JSON_Delete(js);

	return status;
}

int wifi_direct_create_ip_json_file(const char *fileName,const char *itemName,
										const char *subItemName_1,const char *sbuItemVal_1,
										const char *subItemName_2,const char *sbuItemVal_2,
										const char *subItemName_3,const char *sbuItemVal_3){
	JSON *js = NULL;
	JSON *item = NULL;

	js = JSON_CreateObject();
	if(js == NULL){
		printf("Create json fail!!!\n");
		return -1;
	}
	JSON_AddItemToObject(js, itemName, item = JSON_CreateObject());
	if(NULL == item){
		printf("json data error!");
		JSON_Delete(js);
		return -1;
	}
	JSON_AddStringToObject(item, subItemName_1, sbuItemVal_1);
	JSON_AddStringToObject(item, subItemName_2, sbuItemVal_2);
	JSON_AddStringToObject(item, subItemName_3, sbuItemVal_3);

	jsonDataWrite2File(fileName, js);

	JSON_Delete(js);

	return 0;
}

static int wifi_direct_set_ip_info_to_file(struct p2p *p, const char *filename)
{
	char ip[32] = {0};
	char gateway[32] = {0};
	char netmask[32] = {0};
		 
	ifc_get_ip_addr(p->ifname, ip);
	ifc_get_route_addr(p->ifname, gateway);
	ifc_get_netmask_addr(p->ifname, netmask);
	printf("ip=%s,gateway=%s,netmask=%s\n",ip,gateway,netmask);

	if(access(filename, F_OK) == 0){
		printf("p2p ip info config file exist!\n");
		WIFI_DIRECT_JSON_STATE status = wifi_direct_set_json_to_file(filename,
																	p->peer_ifaddr,
																	CONFIG_P2P_IP, ip,
																	CONFIG_P2P_GATEWAY,gateway,
																	CONFIG_P2P_MASK,netmask);
		if(status == JSON_ERROR_GET_JSON){
			printf("json file error, create again\n");
			wifi_direct_create_ip_json_file(filename,
											p->peer_ifaddr,
											CONFIG_P2P_IP, ip,
											CONFIG_P2P_GATEWAY,gateway,
											CONFIG_P2P_MASK,netmask);
		}
	}else{
		printf("p2p ip info config file not exist, create config file!\n");
		wifi_direct_create_ip_json_file(filename,
										p->peer_ifaddr,
										CONFIG_P2P_IP, ip,
										CONFIG_P2P_GATEWAY,gateway,
										CONFIG_P2P_MASK,netmask);
	}
}

int getStringValueFromJson(JSON *js, const char *key, char *strValue, int strValLen)
{
	JSON *jsKey = NULL;

	if(NULL == key)
		return -1;
	if(js != NULL && js->type == JSON_Object){
		jsKey = JSON_GetObjectItem(js, key);
		if(jsKey != NULL && jsKey->type == JSON_String){
			strncpy(strValue, jsKey->valuestring, strValLen);
			return 0;
		}
	}
	return -1;
}

int wifi_direct_get_json_from_file(const char *fileName,const char *itemName,
											const char *subItemName_1,char *sbuItemVal_1, int sbuItemVal_1Len,
											const char *subItemName_2,char *sbuItemVal_2, int sbuItemVal_2Len,
											const char *subItemName_3,char *sbuItemVal_3, int sbuItemVal_3Len){
	JSON *js = NULL;
	JSON *item = NULL;
	WIFI_DIRECT_JSON_STATE status = JSON_OK;
	
	js = jsonDataGetFromFile(fileName, &status);
	if(NULL == js){
		printf("get json data from file fail\n");
		return -1;
	}
	item = isJsonHaveItem(js, itemName);
	if(item){
		getStringValueFromJson(item, subItemName_1, sbuItemVal_1, sbuItemVal_1Len);
		getStringValueFromJson(item, subItemName_2, sbuItemVal_2, sbuItemVal_2Len);
		getStringValueFromJson(item, subItemName_3, sbuItemVal_3, sbuItemVal_3Len);
	}else{
		printf("no this item:%s\n", itemName);
		return -1;
	}
	JSON_Delete(js);

	return 0;
}
#endif
static int _wifi_direct_get_source_ip_address(struct p2p *p)
{
    char mac[32]={0};
    unsigned char mac_tmp[32]={0};
    FILE *fp;
    char line[256]={0};
    int ip_get_ok=0;
    char ip[32]={0};
    unsigned char ip_tmp[16]={0};
    int i,cnt=0;
    int file_len;
    char nouse[12];
    int ret=-1;

    wifi_direct_get_ifaddr(p);
    if(strlen(p->peer_ifaddr)>0){

	strcpy(mac,p->peer_ifaddr);
	_to_upper_case(mac);
    }
    else{
         printf("%s: peer mac address error\n",__FUNCTION__);
         return ret;
    }
#if WIFI_DIRECT_DNSMASQ
	if( p->role == P2P_ROLE_CLIENT|| p->role == P2P_ROLE_GO ){
#else
	if( p->role == P2P_ROLE_CLIENT ){
	  
		 /** the short-cut for getting peer ip address */
		 if(_wifi_direct_get_gw_shortcut(p,ip)==0){
			  ip_get_ok = 1;
			  goto _GET_IP_OUT;
		 }
#endif			  
         /** client mode, use the /proc/net/arp to get the address */
         _wifi_direct_ping_route(p);
         cnt=0;
         while(ip_get_ok==0){
              fp = fopen("/proc/net/arp","r");
              if(fp){
                   while(!feof(fp)){
                        memset(line, 0x00, 256);
                        fgets(line, 256, fp);
                        _to_upper_case(line);
                        if(strstr(line,mac) != NULL){
                             i=0;
                             while(line[i]!=' '){
                                  ip[i]=line[i];
                                  i++;
                             }
                             ip_get_ok = 1;
                             break;
                        }     
                   }     
                   fclose(fp);
              }

              if(ip_get_ok == 0){
                   __wifi_direct_sleep(1);
                   cnt++;
#if WIFI_DIRECT_DNSMASQ
				if(cnt >= 50){
#else
				if(cnt >= 10){
#endif
                        printf(">>>> could not get peer ip\n");
                        break;
                   }
              }
         }
    }
#if !WIFI_DIRECT_DNSMASQ
    else if( p->role == P2P_ROLE_GO ){
         /** group owner mode */
         cnt=0;
         while(ip_get_ok==0){
              fp = fopen("/tmp/udhcpd.leases","rb");
              if(fp == NULL){
                   __wifi_direct_sleep(1);//sleep(1);
                   cnt++;
                   if(cnt >= 60){
                        break;
                   }
                   else{
                        continue;
                   }
              }

              fseek(fp, 0, SEEK_END);
              file_len = ftell(fp);

              /**
              * begine of each least is int64 which means 8 bytes.
              */
              if(file_len != (sizeof(struct dyn_lease)+8)){
                   printf("---->lease file len error:%d,but expected:%d\n",file_len,sizeof(struct dyn_lease)+8);
                   fclose(fp);
                   __wifi_direct_sleep(1);//sleep(1);
                   cnt++;
                   if(cnt >= 60){
                        break;
                   }
                   else{
                        continue;
                   }
              }
              fseek(fp, 0, SEEK_SET);

              /**
              * no use for the first 12 bytes.
              */
              fread(nouse, 1,12, fp);

              /**
              * ip address
              */
              for(i=0;i<4;i++){
                   fread(&ip_tmp[i], 1,1, fp);
              }
              sprintf(ip,"%d.%d.%d.%d",ip_tmp[0],ip_tmp[1],ip_tmp[2],ip_tmp[3]);
              
              /**
              * mac address
              */
              for(i=0;i<6;i++){
                   fread(&mac_tmp[i], 1,1, fp);
              }
              sprintf(line,"%02x:%02x:%02x:%02x:%02x:%02x",mac_tmp[0],mac_tmp[1],mac_tmp[2],mac_tmp[3],mac_tmp[4],mac_tmp[5]);
              fclose(fp);

              printf(">>> host mode:%s,%s,mac=%s  line_len:%d mac:%d >>>>> cnt is %d\n",ip,line,mac,strlen(line),strlen(mac),cnt);

              _to_upper_case(line);
#if WIFI_STA_P2PAUTOGO_ENABLE
			if(strncmp(line,"00:00:00:00:00:00",17)!=0){
				ip_get_ok = 1;
				break;
			}
#else
			if(strcmp(line,mac)==0){
                   ip_get_ok = 1;
                   break;
              }
#endif
         }
    }
#endif //WIFI_DIRECT_DNSMASQ

_GET_IP_OUT:
    if(ip_get_ok){
	char cmd[64] = {0};
#if WIFI_DIRECT_PING
	sprintf(cmd,"ping %s -c 3",ip);
	__wifi_direct_system(cmd);
#endif
         printf(">>> %s:peer ip is %s\n",__FUNCTION__,ip);
         fp = fopen("/tmp/sourceip.log","w");
         if(fp){
              fputs(ip,fp);
              fclose(fp);
         }
         ret = 0;
		#if 0//WIFI_DIRECT_NEW_FLOW
		wifi_direct_set_ip_info_to_file(p, WIFI_P2P_IP_CONFIG_FILE);
		#endif
    }

    return ret;
}

#if EZCAST_LITE_ENABLE
static void start_thttpd(){

	system("thttpd -C /etc/thttpd.conf -u root");
}

void clean_change_to_client_softap(){
	change_to_client_softap = -1;
}

void set_change_to_client_softap(){
	change_to_client_softap = time(NULL);
}

int wifi_softap_start_from_miracast_fun(){
	int ret=0;
	//if(mixed_current_status == MIXED_STATUS_CLIENT_P2P)
		pac_system("iwpriv wlan1 p2p_set enable=0");
	//else
		pac_system("iwpriv wlan0 p2p_set enable=0");

	if(access("/tmp/hostapd/wlan1",F_OK)!=0)
		ret=softap_start();
	else
		modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
	
	return ret;
}

//static void *wifi_direct_start_thread(void *arg){
static void wifi_direct_start_delay(){
	sleep(1);
	EZCASTLOG("-- \n");
	wifi_create_dir("/mnt/vram/wifi");
	EZCASTLOG("-- \n");
	wifi_direct_start(1);
	EZCASTLOG("-- \n");
	modify_wifi_mode(WIFI_DIRECT_ANABLE);
	EZCASTLOG("-- \n");

	//pthread_exit(NULL);

	//return NULL;
}

static void start_client_delay(){
	usleep(500000);
	
	#if WIFI_STA_P2PAUTOGO_ENABLE //bluse add to test station + p2p auto go
	printf("[%s %d]-----------don,t close station mode\n",__FILE__,__LINE__);
	#else
	wifi_stop_process();		//stop the wifi related processes
	#endif	
	wifi_open_fun();
	wifi_start_fun();
	start_thttpd();
}

static int wifi_direct_to_client_softap(){
	//pthread_t tidp;
	
	switch(mixed_current_status){
		case MIXED_STATUS_CLIENT_P2P:
			EZCASTLOG("-- From MIXED_STATUS_CLIENT_P2P[%d]\n", mixed_current_status);
			wifi_direct_func_stop();
			wifi_softap_start_from_miracast_fun();
			start_routing();
			//start_thttpd();
			mixed_current_status = MIXED_STATUS_CLIENT_SOFTAP;
			break;
		case MIXED_STATUS_P2P_SOFTAP:
			EZCASTLOG(" -- From MIXED_STATUS_P2P_SOFTAP[%d]\n", mixed_current_status);
			wifi_direct_func_stop();
#if 0
			int ret = pthread_create(&tidp, NULL, wifi_direct_start_thread, NULL);
			if (ret != 0)
			{
				EZCASTLOG(" pthread_create error(%d), FIXME!\n", ret);
				ret = -1;
			}else{
				ret = pthread_join(tidp, NULL);
				if (ret != 0)
				{
					EZCASTLOG(" pthread_join error(%d)\n", ret);
				}
			}
#endif
			start_client_delay();
			
			mixed_current_status = MIXED_STATUS_CLIENT_SOFTAP;
			break;
		case MIXED_STATUS_P2P_WLAN0:
		case MIXED_STATUS_P2P_WLAN1:
			EZCASTLOG("-- From MIXED_STATUS_P2P_WLAN0 or MIXED_STATUS_P2P_WLAN1[%d]\n", mixed_current_status);
			wifi_direct_func_stop();
			wifi_softap_start_from_miracast_fun();
	#if WIFI_STA_P2PAUTOGO_ENABLE //bluse add to test station + p2p auto go
			printf("[%s %d]-----------don,t close station mode\n",__FILE__,__LINE__);
	#else
			wifi_stop_process();		//stop the wifi related processes
	#endif			
			wifi_open_fun();
			wifi_start_fun();
			start_thttpd();
			mixed_current_status = MIXED_STATUS_CLIENT_SOFTAP;
			break;
		default:
			EZCASTLOG("-- From MIXED_STATUS_NULL[%d]\n", mixed_current_status);
			break;
	}

	return 0;
}

static int wifi_direct_to_client_p2p(){
	pthread_t tidp;
	int now_wifimode;
	
	switch(mixed_current_status){
		case MIXED_STATUS_P2P_SOFTAP:
		case MIXED_STATUS_P2P_WLAN0:
		case MIXED_STATUS_P2P_WLAN1:
		case MIXED_STATUS_NULL:
			EZCASTLOG("-- From MIXED_STATUS_P2P_SOFTAP or MIXED_STATUS_P2P_WLAN0 or MIXED_STATUS_P2P_WLAN1[%d]\n", mixed_current_status);
			wifi_direct_to_client_softap();
		case MIXED_STATUS_CLIENT_SOFTAP:
			now_wifimode = read_wifi_mode();
			EZCASTLOG("--------========= now_wifimode: %d\n", now_wifimode);
			EZCASTLOG("-- From MIXED_STATUS_CLIENT_SOFTAP or MIXED_STATUS_P2P_SOFTAP or MIXED_STATUS_P2P_WLAN0 or MIXED_STATUS_P2P_WLAN1[%d]\n", mixed_current_status);
			disable_route_function(WIFI_ROUTING);
			wifi_direct_initialize_select(1, P2P_WLAN1);
#if 1
			wifi_create_dir("/mnt/vram/wifi");
			wifi_direct_start(1);
			modify_wifi_mode(WIFI_DIRECT_ANABLE);
#else
			int ret = pthread_create(&tidp, NULL, wifi_direct_start_thread, NULL);
			if (ret != 0)
			{
				printf("%s/%d: pthread_create error(%d), FIXME!\n", __FUNCTION__, __LINE__, ret);
				ret = -1;
			}else{
				pthread_detach(tidp);
			}
#endif
			//inti_concurrent_mode_best_channel();
			modify_wifi_mode(now_wifimode);
			//start_thttpd();
			mixed_current_status = MIXED_STATUS_CLIENT_P2P;
			mixed_mode_p2p_status = 1;
			break;
		default:
			break;
	}

	return 0;
}

static int wifi_direct_to_p2p_softap(){
	pthread_t tidp;
	int now_wifimode;
	switch(mixed_current_status){
		case MIXED_STATUS_CLIENT_P2P:
		case MIXED_STATUS_P2P_WLAN0:
		case MIXED_STATUS_P2P_WLAN1:
		case MIXED_STATUS_NULL:
			EZCASTLOG("-- From ALL[%d]\n", mixed_current_status);
			wifi_direct_to_client_softap();
		case MIXED_STATUS_CLIENT_SOFTAP:
			now_wifimode = read_wifi_mode();
			EZCASTLOG("--------========= now_wifimode: %d\n", now_wifimode);
			EZCASTLOG(" -- MIXED_STATUS_CLIENT_SOFTAP[%d]\n", mixed_current_status);
			disable_route_function(WIFI_ROUTING);
			wifi_direct_initialize_select(1, P2P_WLAN0);
			EZCASTLOG("-- \n");
			#if WIFI_STA_P2PAUTOGO_ENABLE //bluse add to test station + p2p auto go
				printf("[%s %d]-----------don,t close station mode\n",__FILE__,__LINE__);
			#else
				wifi_stop_process();		//stop the wifi related processes
			#endif
			start_thttpd();
			EZCASTLOG("-- \n");
			//wifi_close_fun();
			EZCASTLOG("-- \n");
#if 1
			wifi_create_dir("/mnt/vram/wifi");
			wifi_direct_start(1);
			modify_wifi_mode(WIFI_DIRECT_ANABLE);
#else
			int ret = pthread_create(&tidp, NULL, wifi_direct_start_thread, NULL);
			if (ret != 0)
			{
				printf("%s/%d: pthread_create error(%d), FIXME!\n", __FUNCTION__, __LINE__, ret);
				ret = -1;
			}else{
				pthread_detach(tidp);
			}
#endif
			modify_wifi_mode(now_wifimode);
			mixed_current_status = MIXED_STATUS_P2P_SOFTAP;
			mixed_mode_p2p_status = 1;
			break;
		default:
			break;
	}

	return 0;
}

static int wifi_direct_to_p2p(){
	switch(mixed_current_status){
		case MIXED_STATUS_NULL:
			wifi_direct_to_client_softap();
		case MIXED_STATUS_CLIENT_SOFTAP:
	#if WIFI_STA_P2PAUTOGO_ENABLE//bluse add to test station + p2p auto go
	printf("[%s %d]-----------don,t close station mode\n",__FILE__,__LINE__);
	#else
	wifi_stop_process();		//stop the wifi related processes
	#endif
			wifi_close_fun();
			wifi_direct_initialize_select(1, P2P_WLAN1);
			wifi_create_dir("/mnt/vram/wifi");
			wifi_direct_start(1);
			modify_wifi_mode(WIFI_DIRECT_ANABLE);
			mixed_current_status = MIXED_STATUS_P2P_WLAN1;
			break;
		case MIXED_STATUS_CLIENT_P2P:
	#if WIFI_STA_P2PAUTOGO_ENABLE //bluse add to test station + p2p auto go
	printf("[%s %d]-----------don,t close station mode\n",__FILE__,__LINE__);
	#else
	wifi_stop_process();		//stop the wifi related processes
	#endif
			wifi_close_fun();
			mixed_current_status = MIXED_STATUS_P2P_WLAN1;
			break;
		case MIXED_STATUS_P2P_SOFTAP:
			wifi_direct_concurrent_close_client_softap_fun();
			mixed_current_status = MIXED_STATUS_P2P_WLAN0;
			break;
		default:

			break;
	}

	return 0;
}

static int mixed_mode_close_p2p(){
	if((mixed_mode_p2p_status != 0) && (mixed_current_status == MIXED_STATUS_P2P_SOFTAP || mixed_current_status == MIXED_STATUS_CLIENT_P2P)){
		EZCASTLOG("p2p close when stream running!!!\n");
		wifi_direct_func_stop();
		mixed_mode_p2p_status = 0;
	}

	return 0;
}

static int mixed_mode_open_p2p(){
	if((mixed_mode_p2p_status == 0) && (mixed_current_status == MIXED_STATUS_P2P_SOFTAP || mixed_current_status == MIXED_STATUS_CLIENT_P2P)){
		EZCASTLOG("p2p open when stream stop!!!\n");
		if(mixed_current_status == MIXED_STATUS_P2P_SOFTAP)
			wifi_direct_initialize_select(1, P2P_WLAN0);
		else if(mixed_current_status == MIXED_STATUS_CLIENT_P2P)
			wifi_direct_initialize_select(1, P2P_WLAN1);
		wifi_create_dir("/mnt/vram/wifi");
		wifi_direct_start(1);
		modify_wifi_mode(WIFI_DIRECT_ANABLE);
		mixed_mode_p2p_status = 1;
	}

	return 0;
}

static int mixed_mode_change(const int new_mode){
	EZCASTLOG("-- new_mode: %d\n", new_mode);

	if(new_mode <= MIXED_MODE_NULL || new_mode >= MIXED_MODE_MAX){
		EZCASTLOG("-- new mode error[%d]!!!\n", new_mode);
		return -1;
	}

	if(new_mode == mixed_current_status || (new_mode == MIXED_MODE_P2P && mixed_current_status == MIXED_STATUS_P2P_WLAN1)){
		EZCASTLOG("-- not need change status[%d]!!!\n", mixed_current_status);
		return 0;
	}
	EZCASTLOG("-- \n");

	if(new_mode == MIXED_MODE_CLIENT_SOFTAP){
		wifi_direct_mode = WIDI_MODE_NORMAL;
	}else{
		wifi_direct_mode = WIDI_MODE_MIX;
	}
	switch(new_mode){
		case MIXED_MODE_CLIENT_SOFTAP:
			EZCASTLOG("-- Change to MIXED_MODE_CLIENT_SOFTAP\n");
			wifi_direct_to_client_softap();
			set_change_to_client_softap();
			doNegoStart = 0;
			getPeerIpStart = 0;
			
			// TODO: Bink.li: merge from branch<mirascreen_8258b_nand_P2P_AP>, but I don't know what the use of these code.
			#if 0//(defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1)
			char str_ip[16] = {0};
			int retip = -1;
			retip = getIP(0,str_ip);
			if( retip == 0 ){
				get_wifi_config_info();
			}
			#endif
			break;
		case MIXED_MODE_CLIENT_P2P:
			EZCASTLOG("-- Change to MIXED_MODE_CLIENT_P2P\n");
			wifi_direct_to_client_p2p();
			clean_change_to_client_softap();
			doNegoStart = 0;
			getPeerIpStart = 0;
			break;
		case MIXED_MODE_P2P_SOFTAP:
			EZCASTLOG("-- Change to MIXED_MODE_P2P_SOFTAP\n");
			wifi_direct_to_p2p_softap();
			clean_change_to_client_softap();
			doNegoStart = 0;
			getPeerIpStart = 0;
			break;
		case MIXED_MODE_P2P:
			EZCASTLOG("-- Change to MIXED_MODE_P2P\n");
			wifi_direct_to_p2p();
			clean_change_to_client_softap();
			break;
		case MIXED_MODE_P2P_CLOSE:
			mixed_mode_close_p2p();
			break;
		case MIXED_MODE_P2P_OPEN:
			mixed_mode_open_p2p();
			break;
		default:
			break;
	}

	return 0;
}

int is_wifi_connect_ready(){
	if(MIXED_STATUS_CLIENT_SOFTAP == mixed_current_status || MIXED_STATUS_NULL == mixed_current_status)
		return 1;
	
	return 0;
}

int is_wifi_scan_ready(){
	if(MIXED_STATUS_CLIENT_SOFTAP == mixed_current_status || MIXED_STATUS_NULL == mixed_current_status || MIXED_STATUS_CLIENT_P2P == mixed_current_status)
		return 1;
	
	return 0;
}

int mode_change_wait(){
	int now_time = time(NULL);
	if(change_to_client_softap < 0 || now_time < change_to_client_softap || (now_time - change_to_client_softap) > MODE_CHANGE_WAIT_TIME){
		return 0;
	}
	return 1;
}

static int save_connection_info(char *ssid, char *psk, int authenType){
	if(ssid == NULL || psk == NULL || authenType < 0)
		return -1;
	websetting_wifi_info_t *websetting_wifi_info = get_websetting_wifi_info();
	websetting_wifi_info->websetting_authen_type = authenType;
	snprintf(websetting_wifi_info->websetting_ssid, sizeof(websetting_wifi_info->websetting_ssid), "%s", ssid);
	snprintf(websetting_wifi_info->websetting_psk, sizeof(websetting_wifi_info->websetting_psk), "%s", psk);

	return 0;
}

int to_connect(char *ssid, char *psk, int index){
	int ret = -1;
	if(index < 0)
		return -1;
	int authenType = ezcast_wifi_getAuthenType(index);
	if(authenType < 0)
		return -1;
	ret = save_connection_info(ssid, psk, authenType);
	if(ret == 0)
		ezRemoteSendKey(0x1CA);
	return ret;
}

int to_connect_add(char *ssid, char *psk, int authenType){
	int ret = -1;
	
	ret = save_connection_info(ssid, psk, authenType);
	if(ret == 0)
		ezRemoteSendKey(0x1CA);
	return ret;
}

static int wifi_direct_mixed_mode_change(void* handle)
{
	int mode = -1;
    SWFEXT_FUNC_BEGIN(handle);

	mode = Swfext_GetNumber();
	if(mode <= MIXED_MODE_NULL || mode >= MIXED_MODE_MAX){
		EZCASTLOG("-- mode error[%d]!!!\n", mode);
		goto __END__;
	}
	EZCASTLOG("-- mode: %d\n", mode);
	mixed_mode_change(mode);
__END__:	
    SWFEXT_FUNC_END();
}

static int wifi_direct_get_mixed_mode(void* handle)
{
    SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(mixed_current_status);
    SWFEXT_FUNC_END();
}

static int wifi_direct_func_init_select(void* handle)
{
    int role, port;
    
    SWFEXT_FUNC_BEGIN(handle);

    role = Swfext_GetNumber();
	port = Swfext_GetNumber();
	wifi_direct_mode = WIDI_MODE_NORMAL;
    wifi_direct_initialize_select(role, port);
    
    SWFEXT_FUNC_END();
}
#endif

static int wifi_direct_func_start(void* handle)
{
    SWFEXT_FUNC_BEGIN(handle);
    wifi_create_dir("/mnt/vram/wifi");
    wifi_direct_start(1);
    modify_wifi_mode(WIFI_DIRECT_ANABLE);
    SWFEXT_FUNC_END();
}

static int wifi_direct_func_init(void* handle)
{
    int role;
    
    SWFEXT_FUNC_BEGIN(handle);

    role = Swfext_GetNumber();
	wifi_direct_mode = WIDI_MODE_NORMAL;
    wifi_direct_initialize(role);
    
    SWFEXT_FUNC_END();
}


static int wifi_direct_func_cancel(void* handle)
{
    SWFEXT_FUNC_BEGIN(handle);
    wifi_direct_func_stop();
    SWFEXT_FUNC_END();
}
static int wifi_direct_ap_fresh(void *handle)
{
    struct p2p *p = &action_p2p;
    
    SWFEXT_FUNC_BEGIN(handle);
    p2pdbg_info(" fresh direct ap\n");
    p->scan_ok_flag = 0;
    wifi_direct_send_msg(WIDI_CMD_SCAN);
    SWFEXT_FUNC_END();
}

//put wifi direct device name to swf
static int wifi_direct_getssid_app(void * handle)
{
    int index = 0;
    struct p2p *p =NULL;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p=&action_p2p;
    index = Swfext_GetNumber();
    memset(wifi_direct_apbuf,0,WIDI_BUF_LEN);
    if(p->ap_total>0){
         __wifi_direct_get_name(index,wifi_direct_apbuf,WIDI_BUF_LEN);
    }
    Swfext_PutString(wifi_direct_apbuf);
    
    SWFEXT_FUNC_END();     
}

//put wifi direct device signal level to swf
static int wifi_direct_get_signalLevel_app(void * handle)
{
    int index = 0;
    int sig_level;
    struct p2p *p =NULL;
    
    SWFEXT_FUNC_BEGIN(handle);

    p=&action_p2p;
    index = Swfext_GetNumber();
    sig_level = p->scan_pool[index].singal_level;
    Swfext_PutNumber(sig_level);
    
    SWFEXT_FUNC_END();     
}

/**
* generate our pincode.
*/
static int wifi_direct_generate_pincode_app(void *handle)
{
    struct p2p *p=NULL;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p = &action_p2p;
    _wifi_direct_generate_pincode(p);

    printf(">>>>>>>>>>>> generate pincode:%d\n",p->pin);

    SWFEXT_FUNC_END();
}

/**
* display our pincode.
*/
static int wifi_direct_display_pincode_app(void *handle)
{
    struct p2p *p=NULL;
    char pincode[16]={0};
    
    SWFEXT_FUNC_BEGIN(handle);
    p = &action_p2p;
    //if((p->status == P2P_STATE_RX_PROVISION_DIS_REQ)  ){
    sprintf(pincode,"%d",p->pin);
    Swfext_PutString(pincode);//display the wifi direct device  pin code
    //}

    printf(">>>>>>>>>>>>>> display pin code:%s\n",pincode);
    SWFEXT_FUNC_END();
}

/**
* set the pincode input from UI.
*/
static int wifi_direct_set_pincode_app(void *handle)
{
    struct p2p *p=NULL;
    char *pincode;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p = &action_p2p;
    pincode = Swfext_GetString();
    p->pin = atoi(pincode);

    SWFEXT_FUNC_END();
}

/*
* process wifi direct pincode and push button method,if peer device is pin code method,press ok button to display the pin code;
* if peer device is push button method,press ok button to start negations ;     
*/
static int wifi_direct_input_pincode_pbc_app(void *handle)
{
    struct p2p *p = NULL;
    int rtn=-1;
    
    SWFEXT_FUNC_BEGIN(handle);

    p=&action_p2p;
    
    if(p->connect_flag==2){//peer device is pin code method; display the pin code
         rtn = 2;
         p->connect_flag=0;
    }
    else{
         rtn = 1;
         _widirect_set_nego(p);     //push button ,star negotiation
    }
    p2pdbg_info("==========rtn:%d\n",rtn);
    Swfext_PutNumber(rtn);
    SWFEXT_FUNC_END();
}



//cancel the peer device connect request
static int wifi_direct_pbc_cancel_app(void *handle)
{
    int ret = -1;
    struct p2p *p = NULL;
    
    SWFEXT_FUNC_BEGIN(handle);

    p=&action_p2p;
    ret = _widirect_cancle_pbc_request(p);     //pin code input finished,star negotiation
    p2pdbg_info("ret=%d\n",ret);
    Swfext_PutNumber(ret);
    
    SWFEXT_FUNC_END();
}

static int widi_peer_device_name(void *handle)
{
    char device_buf[32] = {0};
    
    SWFEXT_FUNC_BEGIN(handle);
    strncpy(device_buf,widi_device_name,strlen(widi_device_name));
    p2pdbg_info("device_buf:%s\n",device_buf);
    Swfext_PutString(device_buf);
    SWFEXT_FUNC_END();
}

static int wifi_direct_pincode_flag_app(void *handle)
{
    int ret=WIDI_PIN_PBC_NONE;
    struct p2p *p = NULL;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p=&action_p2p;
    pthread_mutex_lock(&p->locker_for_prov_req_cm);
    ret = p->prov_req_cm;
    //printf("%d,%s,%d\n",__LINE__,__FILE__,ret);
    //>>> set to avoid next time getting wrong value.
#ifdef WIFI_DIRECT_JSON
    switch(p->prov_req_cm){
         case WIDI_PIN_PBC_NONE:
              strcpy(p->req_cm,"WIDI_PIN_PBC_NONE");
              break;
         case WIDI_PIN_PBC_DEFAULT:
              strcpy(p->req_cm,"WIDI_PIN_PBC_DEFAULT");
              break;     
         case WIDI_PIN_PBC_INPUT:
              strcpy(p->req_cm,"WIDI_PIN_PBC_INPUT");
              break;               
         case WIDI_PIN_PBC_DISPLAY:
              strcpy(p->req_cm,"WIDI_PIN_PBC_DISPLAY");
              break;               
         case WIDI_PIN_PBC_INVITED:
              strcpy(p->req_cm,"WIDI_PIN_PBC_INVITED");
              break;     
         case WIDI_PIN_PBC_PERSISTENT_GP_INVITED:
              strcpy(p->req_cm,"WIDI_PIN_PBC_PERSISTENT_GP_INVITED");
              break;     
         default:
              break;     
    }
#endif
    if(p->prov_req_cm != WIDI_PIN_PBC_NONE){
         p->prov_req_cm = WIDI_PIN_PBC_NONE;
    }
    pthread_mutex_unlock(&p->locker_for_prov_req_cm);
    
    Swfext_PutNumber(ret);
    
    SWFEXT_FUNC_END();
}

static int wifi_direct_getscanap_total(void *handle)
{
    SWFEXT_FUNC_BEGIN(handle);
    Swfext_PutNumber(action_p2p.ap_total);
    SWFEXT_FUNC_END();          
}

//idex_ap is define the select connect ap numbers
static int wifi_direct_prov_discover_app(void* handle)
{
    int idex_ap;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    idex_ap = Swfext_GetNumber();
    wpa_ap_num = idex_ap;
    //select provide discovery ap mac address
    strcpy(action_p2p.peer_devaddr,action_p2p.scan_pool[idex_ap].mac_addr);
    wifi_direct_provide_discovery(idex_ap);
    
    SWFEXT_FUNC_END();     
}

/*
* function:identy wifi direct connetc status,
* connect_flag: 0 -->default status 1-->client connecting  2-->client mode connect ok
*               3 -->softaping      4 -->softap ok 
*/                    
static int wifi_direct_connect_status_app(void *handle)
{
    SWFEXT_FUNC_BEGIN(handle);
    Swfext_PutNumber(action_p2p.connect_flag);
    SWFEXT_FUNC_END();     
}

//get the succeful connect ssid  ap_idex
static int widi_direct_connect_apidex_app(void *handle)
{
    int widi_ap_idex = -1;
    struct p2p *p = NULL;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p = &action_p2p;
    widi_ap_idex = _identify_ap_connectok(p);
    Swfext_PutNumber(widi_ap_idex);
    
    SWFEXT_FUNC_END();     
}


/**
* @brief check the connection status.
* @return 0-->connected;1-->disconnected;2-->ignore
*/
static int _widi_check_connect_status(void *handle)
{
    int ret = -1;
    static int num=0;
    struct p2p *p =NULL;
    int stat=2;
    
    SWFEXT_FUNC_BEGIN(handle);

    p = &action_p2p;
    
#if 0
    p->role = wifi_direct_role(p);
    if((p->connect_flag==WIDI_CONNECT_CLIENT_OK)&&(p->role==P2P_ROLE_CLIENT)){
         ret=_wifi_direct_clientMode_connect_status(p);
         if(ret == 2){
              wifi_driect_set_persistent(p,1);
              wifi_direct_set_profilefound_information(p);
              strcpy(p->mac_connect_ok,"00:00:00:00:00:00");
              p->connect_flag=WIDI_STATUS_DISCONNECT;
              reinit_wifi_direct_function(p);               
         }
         else if(ret == 1){
              
         }
    }
    else if(p->connect_flag==WIDI_CONNECT_SOFTAP_OK&&(p->role==P2P_ROLE_GO)){
         usleep(500);
         wifi_direct_check_goMode_connect_status(p);//chect soft ap connect status
    }
    else{
         /*check peer device reject connect request*/
         if(_wifi_direct_check_reject_status(p) == 0){
              p->connect_flag = WIDI_STATUS_REJECT;
              p2pdbg_info(" p->connect_flag:%d\n",p->connect_flag);
              reinit_wifi_direct_function(p);     
         }
    }

    if(p->scan_ok_flag==1&&p->ap_total <= 0){
         //p->scan_ok_flag = 0;
         wifi_direct_send_msg(WIDI_CMD_SCAN);
    }
#endif

    if(p->wpa_open == _TRUE){
         /// in client.
         ret=_wifi_direct_clientMode_connect_status(p);
         if(ret == 0){
              /// connected.
              stat = 0;
         }
         else{
              /// disconnected.
              stat = 1;
         }
    }
    else if(p->ap_open == _TRUE){
         //usleep(500);
         ret = wifi_direct_check_goMode_connect_status(p);
         if(ret == 0){
              /// connected.
              stat = 0;
         }
         else{
              /// disconnected.
              stat = 1;
         }
    }

    Swfext_PutNumber(stat);
    
    SWFEXT_FUNC_END();     
}

/*disconnect wifi direct connect*/
static int widi_disconnect(void *handle)
{
    SWFEXT_FUNC_BEGIN(handle);
    wifi_direct_disconnect();
    SWFEXT_FUNC_END();
}

 /*display wifi direct device self name*/
static int wifi_display_device_name(void *handle)
{
    struct p2p *p = &action_p2p;
    
    SWFEXT_FUNC_BEGIN(handle);
    Swfext_PutString(p->dev_name);
    SWFEXT_FUNC_END();
}

static int wifi_display_set_local_device_name(void *handle)
{
    struct p2p *p = &action_p2p;
    char *devname;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    devname = Swfext_GetString();
    if(devname && strlen(devname)>0 && strlen(devname)<sizeof(p->dev_name)){
         memset(p->dev_name,0,sizeof(p->dev_name));
         strcpy(p->dev_name,devname);
         printf(">>> device name is %s",p->dev_name);
    }
    else{
         printf(">>> Wifi Direct device name invalid\n");
    }
    
    SWFEXT_FUNC_END();
}

static int wifi_direct_get_scan_staus(void *handle)
{
    struct p2p *p = &action_p2p;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    Swfext_PutNumber(p->scan_ok_flag);
    
    SWFEXT_FUNC_END();
}

static int do_nego(int cm){
    struct p2p *p =NULL;
    int ret;
	
    p=&action_p2p;
	ret = __wifi_direct_do_negotiation(p,cm);
	if(ret !=0 ){
		 
#ifdef WIFI_DIRECT_JSON
		 writeToJsonFile(p,"do nego error");		  //if do nego error,then wirte into json file
#endif
		 return (1);
	}
	else{
		 return (0);
	}
	
}

static int wifi_direct_do_nego(void * handle)
{
    int cm;
    int ret;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    cm = Swfext_GetNumber();
	ret = do_nego(cm);
	Swfext_PutNumber(ret);
    
    SWFEXT_FUNC_END();     
}

static int wifi_direct_do_join_group(void * handle)
{
    struct p2p *p =NULL;
    int ret;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p=&action_p2p;
    
    ret = __wifi_direct_join_group(p);
    if(ret !=0 ){
#ifdef WIFI_DIRECT_JSON
         writeToJsonFile(p,"do join group error");
#endif
         Swfext_PutNumber(1);
    }
    else{
         Swfext_PutNumber(0);
    }
    
    SWFEXT_FUNC_END();     
}

static int wifi_direct_do_join_persistent_group(void * handle)
{
    struct p2p *p =NULL;
    int err;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p=&action_p2p;
    
    err = __wifi_direct_join_persistant_group(p);
    if(err){
#ifdef WIFI_DIRECT_JSON
         writeToJsonFile(p,"do join persistant group error");
         jsonWriteFlag = 1;
#endif
         printf(">>>>>> Join persistant group error\n");
    }
    
    SWFEXT_FUNC_END();     
}


static int wifi_direct_restart(void * handle)
{
    struct p2p *p =NULL;
    
    SWFEXT_FUNC_BEGIN(handle);

#if EZCAST_ENABLE
	doNegoStart = 0;
	getPeerIpStart = 0;
#endif
    p=&action_p2p;
    reinit_wifi_direct_function(p);
    modify_wifi_mode(WIFI_DIRECT_ANABLE);
    SWFEXT_FUNC_END();     
}

#if WIFI_DIRECT_NEW_FLOW
static int wifi_direct_get_port(){
	struct p2p *p =NULL;

	 p=&action_p2p;
    _wifi_direct_get_peer_port(p);

	return 0;
}
#endif
/**
* NOTE:For miracast usage only.
*/
static int wifi_direct_get_peer_port(void * handle)
{
    struct p2p *p =NULL;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    p=&action_p2p;
    _wifi_direct_get_peer_port(p);
    
    SWFEXT_FUNC_END();     
}

static int get_source_ip_address(){
		struct p2p *p =NULL;
		int ret, err = -1;
	
		p=&action_p2p;
		ret = _wifi_direct_get_source_ip_address(p);
		if(ret != 0){
#ifdef WIFI_DIRECT_JSON
			 if(jsonWriteFlag == 0){
				  writeToJsonFile(p,"get peer ip error");
			 }
			 else
				  jsonWriteFlag = 0;
#endif
			 return 1;
		}
		else{
			 return 0;
		}
}

/**
* NOTE: For Miracast usage only.
*/
static int wifi_direct_get_peer_ip(void * handle)
{
    int ret;
    
    SWFEXT_FUNC_BEGIN(handle);
    
	ret = get_source_ip_address();
	Swfext_PutNumber(ret);
	
    SWFEXT_FUNC_END();     
}

#if EZCAST_ENABLE
static void *pthread_get_source_ip_address(void *arg){
	//EZCASTLOG("start get IP address!!!\n");
	getPeerIpStart = 1;
	getIpResult = get_source_ip_address();
	getPeerIpStart = 0;

	pthread_exit(NULL);
}

#if WIFI_DIRECT_NEW_FLOW
static int wifi_direct_get_peer_ip_action(){
	pthread_t tidp;

	if(getPeerIpStart == 0){
		getIpResult = -1;
		int ret = pthread_create(&tidp, NULL, pthread_get_source_ip_address, NULL);
		if (ret != 0)
		{
			EZCASTLOG(" pthread_create error(%d), FIXME!\n", ret);
			return -1;
		}else{
			ret = pthread_detach(tidp);
			if (ret != 0)
			{
				EZCASTLOG(" pthread_detach error(%d)\n", ret);
			}
			return 0;
		}
	}else{
		return 0;
	}
    return -1;	
}
#endif
/**
* NOTE: For Miracast usage only.
*/
static int wifi_direct_get_peer_ip_pthread(void * handle)
{
	pthread_t tidp;
	
    SWFEXT_FUNC_BEGIN(handle);

	if(getPeerIpStart == 0){
		getIpResult = -1;
		int ret = pthread_create(&tidp, NULL, pthread_get_source_ip_address, NULL);
		if (ret != 0)
		{
			EZCASTLOG(" pthread_create error(%d), FIXME!\n", ret);
			Swfext_PutNumber(-1);
		}else{
			ret = pthread_detach(tidp);
			if (ret != 0)
			{
				EZCASTLOG(" pthread_detach error(%d)\n", ret);
			}
			Swfext_PutNumber(0);
		}
	}else{
		Swfext_PutNumber(0);
	}
    
    SWFEXT_FUNC_END();     
}

static int wifi_direct_get_peer_ip_result(void * handle)
{
	int val = -1;
    SWFEXT_FUNC_BEGIN(handle);

	val = getIpResult;
	getIpResult = -1;
	//EZCASTLOG("result: %d\n", val);
	Swfext_PutNumber(val);
    
    SWFEXT_FUNC_END();     
}

static void *pthread_do_nego(void *arg){
	int cm = *(int*)arg;
	EZCASTLOG("do nego, cm = %d\n", cm);
	
	doNegoStart = 1;
	doNegoResult = do_nego(cm);
	doNegoStart = 0;
	
	pthread_exit(NULL);
}

#if WIFI_DIRECT_NEW_FLOW
static int wifi_direct_start_do_nego_pthread(int method)
{
    int ret;
	pthread_t tidp;
        
	if(doNegoStart == 0){
		doNegoResult = -1;
		int ret = pthread_create(&tidp, NULL, pthread_do_nego, (void *)&method);
		if (ret != 0)
		{
			EZCASTLOG(" pthread_create error(%d), FIXME!\n", ret);
			return -1;
		}else{
			ret = pthread_detach(tidp);
			if (ret != 0)
			{
				EZCASTLOG(" pthread_detach error(%d)\n", ret);
			}
			return 0;
		}
	}else{
		return 0;
	}
    return -1;
}
#endif
static int wifi_direct_do_nego_pthread(void * handle)
{
    int cm;
    int ret;
	pthread_t tidp;
    
    SWFEXT_FUNC_BEGIN(handle);
    
    cm = Swfext_GetNumber();
	if(doNegoStart == 0){
		doNegoResult = -1;
		int ret = pthread_create(&tidp, NULL, pthread_do_nego, (void *)&cm);
		if (ret != 0)
		{
			EZCASTLOG(" pthread_create error(%d), FIXME!\n", ret);
			Swfext_PutNumber(-1);
		}else{
			ret = pthread_detach(tidp);
			if (ret != 0)
			{
				EZCASTLOG(" pthread_detach error(%d)\n", ret);
			}
			Swfext_PutNumber(0);
		}
	}else{
		Swfext_PutNumber(0);
	}
    
    SWFEXT_FUNC_END();     
}

static int wifi_direct_do_nego_result(void * handle)
{
	int val = -1;
    SWFEXT_FUNC_BEGIN(handle);

	val = doNegoResult;
	doNegoResult = -1;
	//EZCASTLOG("result: %d\n", val);
	Swfext_PutNumber(val);
    
    SWFEXT_FUNC_END();     
}

#endif

/*closet client or softap process*/
void wifi_direct_concurrent_close_client_softap_fun(){
    struct p2p *p =NULL;


    p=&action_p2p;
    
    if(access("/tmp/wpa_supplicant",F_OK)==0)     {          //stop client
         if(p->wpa_open ==_TRUE ){
              __wifi_direct_system("killall wpa_supplicant");
              __wifi_direct_system("killall udhcpc");
              __wifi_direct_system("rm -rf /tmp/wpa_ctrl_*");
              p->wpa_open = _FALSE;
         }
         else{
#if !WIFI_DIRECT_CLIENT_ENABLE_KEEP_CLIENT
              wifi_stop_process();               
#endif
         }
    }
    if(access("/tmp/hostapd",F_OK)==0){                    //stop softap 
         __wifi_direct_system("killall hostapd");
#if WIFI_DIRECT_DNSMASQ
	__wifi_direct_system("killall dnsmasq");
#endif
         __wifi_direct_system("killall udhcpd");
#if EZCAST_LITE_ENABLE
		if(wifi_direct_mode != WIDI_MODE_MIX){

		}
#else

#endif
         __wifi_direct_system("ifconfig wlan1 down");                    //without this,the RTSP connect would be failed in wlan0
    }
}
static int wifi_direct_concurrent_close_client_softap(void *handle)
{
    SWFEXT_FUNC_BEGIN(handle);
	wifi_direct_concurrent_close_client_softap_fun();
    SWFEXT_FUNC_END();
}
/*restart softap process*/
static int wifi_direct_softap_mode_startSoftap(void *handle){
    int ret;
    
    SWFEXT_FUNC_BEGIN(handle);

    ret = softap_start();

    SWFEXT_FUNC_END();
}


int swfext_widi_register(void)
{
    SWFEXT_REGISTER("wifi_direct_func_init_as", wifi_direct_func_init);//init wifi direct function
    SWFEXT_REGISTER("wifi_dirct_func_start_as", wifi_direct_func_start);//start wifi direct function
    SWFEXT_REGISTER("wifi_dirct_getap_total_as", wifi_direct_getscanap_total); //get scan ap total
    SWFEXT_REGISTER("wifi_dirct_scan_ap_inform_as", wifi_direct_ap_fresh);    //scan ap api
    SWFEXT_REGISTER("wifi_direct_stop_func_as",wifi_direct_func_cancel);     //stop wifi direct function
    SWFEXT_REGISTER("wifi_direct_start_prov_discover_as",wifi_direct_prov_discover_app); //prov discoversy
    SWFEXT_REGISTER("wifi_direct_getssid_as",wifi_direct_getssid_app);      //get scan ssid informations
    SWFEXT_REGISTER("wifi_direct_connect_status_as",wifi_direct_connect_status_app);      
    SWFEXT_REGISTER("wifi_direct_pincode_flag_as",wifi_direct_pincode_flag_app); 
    SWFEXT_REGISTER("wifi_direct_display_pincode_as",wifi_direct_display_pincode_app);
    SWFEXT_REGISTER("wifi_direct_generate_pincode_app_as",wifi_direct_generate_pincode_app);
    SWFEXT_REGISTER("wifi_direct_set_pincode_app_as",wifi_direct_set_pincode_app);
    SWFEXT_REGISTER("wifi_direct_input_pincode_as",wifi_direct_input_pincode_pbc_app);
    SWFEXT_REGISTER("widi_direct_connect_apidex_app_as",widi_direct_connect_apidex_app);//identify which ssid is connect ok      
    SWFEXT_REGISTER("wifi_direct_pbc_cancel_app_as",wifi_direct_pbc_cancel_app);
    SWFEXT_REGISTER("widi_check_connect_status_as",_widi_check_connect_status);
    SWFEXT_REGISTER("widi_peer_device_name_as",widi_peer_device_name);
    SWFEXT_REGISTER("widi_disconnect_as",widi_disconnect);
    SWFEXT_REGISTER("wifi_display_device_name_as",wifi_display_device_name);
    SWFEXT_REGISTER("wifi_direct_get_signalLevel_as",wifi_direct_get_signalLevel_app);
    SWFEXT_REGISTER("wifi_direct_get_scan_staus_as", wifi_direct_get_scan_staus);
    SWFEXT_REGISTER("wifi_direct_do_nego_as", wifi_direct_do_nego);
    SWFEXT_REGISTER("wifi_direct_restart_as", wifi_direct_restart);
    SWFEXT_REGISTER("wifi_direct_get_peer_port_as", wifi_direct_get_peer_port);
    SWFEXT_REGISTER("wifi_direct_get_peer_ip_as", wifi_direct_get_peer_ip);
#if EZCAST_ENABLE
    SWFEXT_REGISTER("wifi_direct_get_peer_ip_result", wifi_direct_get_peer_ip_result);
    SWFEXT_REGISTER("wifi_direct_get_peer_ip_pthread", wifi_direct_get_peer_ip_pthread);
    SWFEXT_REGISTER("wifi_direct_do_nego_result", wifi_direct_do_nego_result);
    SWFEXT_REGISTER("wifi_direct_do_nego_pthread", wifi_direct_do_nego_pthread);
#endif
    SWFEXT_REGISTER("wifi_display_set_local_device_name_as", wifi_display_set_local_device_name);
    SWFEXT_REGISTER("wifi_direct_do_join_group_as", wifi_direct_do_join_group);
    SWFEXT_REGISTER("wifi_direct_do_join_persistent_group_as", wifi_direct_do_join_persistent_group);
    SWFEXT_REGISTER("wifi_direct_concurrent_close_client_softap_as", wifi_direct_concurrent_close_client_softap);
    SWFEXT_REGISTER("wifi_direct_softap_mode_startSoftap_as", wifi_direct_softap_mode_startSoftap);
#if EZCAST_LITE_ENABLE
	SWFEXT_REGISTER("wifi_direct_func_init_select_as", wifi_direct_func_init_select);//init wifi direct function at WLAN0 or WLAN1,
	SWFEXT_REGISTER("wifi_direct_mixed_mode_change", wifi_direct_mixed_mode_change);
	SWFEXT_REGISTER("wifi_direct_get_mixed_mode", wifi_direct_get_mixed_mode);
#endif
    
    return 0;
}

