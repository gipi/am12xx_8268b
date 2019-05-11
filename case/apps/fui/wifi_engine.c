#ifdef MODULE_CONFIG_NETWORK
#include "swf_ext.h"
#include "wifi_engine.h"
#include <fcntl.h>
#include "apps_vram.h"
#include "ezcast_public.h"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include "websetting.h"
#include "net_config_info.h"
#include "iwlib.h"

#define WIFI_DIRECT_CLIENT_ENABLE 0		//enable p2p+client mode or not
 
extern wifi_info wifi_s;

extern int selected_ap;
extern char selected_filename[256];
extern char temp_filename[WIFI_BUF_LEN];		//temp save the conf file path
extern char selected_ssid[MAX_SSID_LEN];
extern int autoConnectFlag;
extern char Rtl_ChannelRegion[16];
extern int connect_sem_flag;			//define as sem lock
extern status_enum wlan_status;
extern current_status wlan_current_status;
extern int autoConnectFlag;
extern int vid_pid_from_driver_firstin;
extern int vid_pid_from_driver;


lease_info leaseInfo;

char *device=NULL;

extern int  softap_start();

//char *best_channel=NULL;
static char *password_for_display;
int wifi_client_thread_is_running=0;
char tmp_infobuf[WIFI_BUF_LEN];
int wps_flag ;
int apmode_close_flag=0;
static char *pre_gateway;

char wifi_ip_address_for_display[50]={0};
char wifi_mask_for_display[50]={0};
char wifi_gateway_for_display[50]={0};
char wifi_first_dns_for_display[50]={0};
char wifi_second_dns_for_display[50]={0};

int hotspot_connect_status = -1;

#if EZCAST_LITE_ENABLE
	websetting_wifi_info_t websetting_wifi_info;
#endif

#if 1
	#define wifiautodbg_info(fmt, arg...) printf("WIFIINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
		
#else
	#define wifiautodbg_info(fmt, arg...)
#endif

#define wireless_tkip_conf "/etc/wireless-tkip.conf"
#define wireless_tkip_bak_conf "/etc/wireless-tkip.conf"
#define wireless_tkip_conf_content "ctrl_interface=/tmp/wpa_supplicant"

static int wireless_tkip_conf_test(){
	int fd_recover = -1;
	char content_buf[64] = "";
	int wireless_tkip_conf_content_length = 0;

	wireless_tkip_conf_content_length = strlen(wireless_tkip_conf_content) + 1;
	memset(content_buf,0,64);
	strncpy(content_buf,"ctrl_interface=/tmp/wpa_supplicant",wireless_tkip_conf_content_length);
	if(access(wireless_tkip_conf,F_OK) != 0){
		printf("/etc/wireless-tkip.conf is not exsit!\n");
		if(access(wireless_tkip_bak_conf,F_OK) !=0){
			fd_recover = open(wireless_tkip_conf, O_RDWR|O_CREAT, 0664);
			 write(fd_recover, wireless_tkip_conf_content, wireless_tkip_conf_content_length);
			 fsync(fd_recover);
			 close(fd_recover);
		}
	}
	return 0;
}
struct wifi_info_t *__get_wifiinfo()
{
	return &wifi_s;
}

struct lease_info_t *__get_leaseInfo(){
	return &leaseInfo;
}

int wifi_open_fun(){
	int ret=-1;

	wifidbg_info("now is in wifi open");
	if(wifi_create_dir("/mnt/vram/wifi")!=0)
	{
		wifidbg_err("create wifi dir error!");
		return -1;
	}
	
	wireless_tkip_conf_test();

	return 0;
}

static int wifi_open(void * handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = wifi_open_fun();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_open(void)
{
	int rtn=0;
	int ret=-1;
	char callbuf[50];
	wifidbg_info("now is in wifi open");
	if(wifi_create_dir("/mnt/vram/wifi")!=0)
	{
		wifidbg_err("create wifi dir error!");
		rtn = -1;
		goto wifi_open_out;
	}
	
	wireless_tkip_conf_test();
wifi_open_out:
	return rtn;
}
#endif
extern  int wifi_switch[2];
#define DONGLE_OUT 	2

void wifi_close_fun(){
	wifidbg_info("now is in wifi close");
//	printf("@@@wifi_close!!!@@@\n");
	char callbuf[128]={0};
	if(dongle_get_device_changing()==1){
		
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		wifi_check_remote_control_thread(1);
		disable_route_function(WIFI_ROUTING);
						

		return;
#endif
		if(access("/sys/module/8192cu",F_OK)==0){
			memset(callbuf,0,50);
			sprintf(callbuf,"ifconfig wlan0 down");
//			#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
//			sprintf(callbuf,"ifconfig wlan1 down");
//			#endif
			system(callbuf);
		}
		else
			return;
		
		memset(callbuf,0,50);
		sprintf(callbuf,"rmmod 8192cu");
		system(callbuf);
	}
	else if(dongle_get_device_changing()==2){
		//char callbuf[50]={0};
		if(access("/sys/module/rt5370sta",F_OK)==0){
			sprintf(callbuf,"killall wpa_supplicant\nkillall udhcpc\nifconfig ra0 down\nmodprobe -r rt5370sta");
			system(callbuf);
		}

		if(access("/sys/module/rtnet3070ap",F_OK)==0){
            //sprintf(callbuf,"killall thttpd\nkillall udhcpd\nifconfig ra0 down\nmodprobe -r rtnet3070ap");
            sprintf(callbuf,"killall udhcpd\nifconfig ra0 down\nmodprobe -r rtnet3070ap");
			system(callbuf);
		}
		
/*	
		int ret1=access("/sys/module/rt5370sta",F_OK);		
		int ret2=access("/sys/module/rtnet3070ap",F_OK);
		if(ret1==0||ret2==0){
			if(ret2==0){	
				memset(callbuf,0,50);
				sprintf(callbuf,"killall udhcpd");
				system(callbuf);
			}
		
			memset(callbuf,0,50);
			sprintf(callbuf,"ifconfig ra0 down");
			system(callbuf);
		}
		else
			goto __wifi_close_end__;
		
		memset(callbuf,0,50);
		sprintf(callbuf,"modprobe -r rt5370sta");
		system(callbuf);

		memset(callbuf,0,50);
		sprintf(callbuf,"modprobe -r rtnet3070ap");
		system(callbuf);
*/	
		wifi_switch[0]=-1;
		wifi_switch[1]=-1;
	}
	if(read_wifi_mode()==WIFI_CLIENT_MODE_ENABLE)
		__release_ap_entry();

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		printf("5G dongle out \n");
		Probox_set_led(LED5G_OFF);	
#endif
}

static int wifi_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	wifi_close_fun();
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
void ezcast_wifi_close(void)
{
	wifi_close_fun();
}
#endif


void __reset_connect_ap(char *connect_ap)
{
	if(connect_ap){
		memset(connect_ap,0,strlen(connect_ap));
		sprintf(connect_ap,"%s","/etc/wireless-tkip.conf");
	}
}
extern int vid_pid_from_driver;


char content_for_display[MAX_SSID_LEN];

char* wifi_conf_content_for_display(char *key){
	FILE *fp;
	char callbuf[50];
	//char buf[128];
	char buf[512];
	memset(content_for_display,0,50);
	
	if(strcmp(selected_filename,"/etc/wireless-tkip.conf")==0){
		return NULL;
	}
		
	if(access(selected_filename,F_OK)!=0)
		return NULL;
	fp = fopen(selected_filename,"r");
	if(fp == NULL){
		return NULL;
	}

	fread(buf, 1, 512, fp);
	fclose(fp);
	if(buf==NULL){
		return NULL;
	}	
	char *buf_tmp1=buf;
	
	char *locate1=strstr(buf,key);
	locate1+=strlen(key);
	buf_tmp1=locate1;

	if(locate1==NULL){
		printf("the key doesn't exsit\n");
		
		return NULL;
		
	}
	char *buf_tmp2=buf_tmp1;
	char *locate2=strstr(buf_tmp2,"\"");
	//printf("strlen(locate1)=====%d\n",strlen(locate1));
	//printf("strlen(locate2)=====%d\n",strlen(locate2));
	if(strlen(locate1)-strlen(locate2)<=0)
		return NULL;
		
	memcpy(content_for_display,buf_tmp1,strlen(locate1)-strlen(locate2));
	//printf("content_for_display=====%s\n",content_for_display);
	if(strlen(content_for_display)==0){
		
		
		return NULL;
		
	}
	//memset(selected_ssid,0,32);
	//memcpy(selected_ssid,content_for_display,50);
	return content_for_display;
}

int wifi_start_fun(){
	char *device = NULL;
	int ret_wlan_conf=-1;
	char callbuf[50];

	printf("%s  %d>>>>>>> wifi_client_thread_is_running:%d\n",__func__,__LINE__,wifi_client_thread_is_running);

	memset(callbuf,0,50);
	connect_sem_flag = 0;		//reset the sem flag
#if EZCAST_ENABLE
	int _hotspotStatus = getHotspotStatus();
	if(_hotspotStatus != HOTSPOT_READY && _hotspotStatus != HOTSPOT_SCAN && _hotspotStatus != HOTSPOT_CONNECTING){
#endif
	autoConnectFlag = 0;		//start auto connect when client mode start
#if EZCAST_ENABLE
	}
#endif

	if(wifi_client_thread_is_running==1)
		return 0;
		
	wlan_status = WIFI_DISCONNECTED;
	__reset_connect_ap(selected_filename);
	
	device = dongle_get_device();
	wifiautodbg_info("device is %s",device);
	if(device==NULL){
		return PORT_ERROR;
	}

	if(strncmp(device,"ra0",strlen("ra0"))==0){
	/*	printf("%s %d>>>>>>%s\n",__func__,__LINE__,device);
		while(1){
			printf("=============%s %d\n",__func__,__LINE__);
				int i=50000;
				while(i--)
					;
			system("lsmod");
			if(access("/sys/module/rt5370sta",F_OK)==0||access("/sys/module/rtnet3070ap",F_OK)==0){
				break;
			}
		}*/
		//printf("debug %s %d==============\n",__func__,__LINE__);
		OSSleep(2500);
		//printf("debug %s %d==============\n",__func__,__LINE__);
	}


	
	init_wifi_wlan_current_status();
#if EZCAST_ENABLE
	char *interface_tmp=NULL;

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	
	if(vid_pid_from_driver==0x148f8189)
		interface_tmp="wlan2";
	else
		interface_tmp="wlan0";

	
#else
	interface_tmp="wlan0";
	
	
#endif	
	sprintf(callbuf,"ifconfig %s 0.0.0.0",interface_tmp);
	system(callbuf);	
	memset(callbuf,0,50);
	sprintf(callbuf,"ifconfig %s down",interface_tmp);
	system(callbuf);

#endif	
	if(wlan_up(device)!=0)
	{
		fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		return 0;

	}
	
	wlan_conf(device,selected_filename);
	if(openCtrlConnection()!=0)
	{
		printf("open ctrl connectiong is error!\n");
		return -1;
	}
	
thread_start:
	if(sem_init(&wifi_s.semi_wpa_cli,0,1)==-1){
		sem_destroy(&wifi_s.semi_wpa_cli);
		wifidbg_err("Sem Wpa_cli init error");
		return -1;
	}
/*		if(sem_init(&wifi_s.semi_wpastatus,0,1)==-1){
		sem_destroy(&wifi_s.semi_wpa_cli);
		sem_destroy(&wifi_s.semi_wpastatus);
		wifidbg_err("Sem Wpa_clie init error");
		rtn = -1;
		goto wifi_start_out;
	}*/
	if(wifi_create_thread(&wifi_s)!=0){
		wifidbg_err("wifi create thread error!");
		sem_destroy(&wifi_s.semi_wpa_cli);
		//sem_destroy(&wifi_s.semi_wpastatus);
		return -1;
	}
	else{
		if(wifi_req_init_queue(&wifi_s)==-1){
			wifidbg_info("wifi init queue error!");
			sem_destroy(&wifi_s.semi_wpa_cli);
		//	sem_destroy(&wifi_s.semi_wpastatus);
			wifi_thread_exit(&wifi_s);
			return -1;
		}
	}
	if(wifi_create_auto_connect(&wifi_s)!=0){
		printf("wifi create thread error!\n");
		sem_destroy(&wifi_s.semi_wpa_cli);
		//sem_destroy(&wifi_s.semi_wpastatus);
		wifi_thread_exit(&wifi_s);
		return -1;
	}
	printf("%s >>>>>>> wifi_client_thread_is_running:%d\n",__func__,wifi_client_thread_is_running);	
	wifi_client_thread_is_running=1;
	printf("wifi start OK\n");
	#if 0//EZMUSIC_ENABLE
	printf("\n--------------Start Named Denny@20140910--------------\n");
	system("killall named");
	system("/sbin/named -c /etc/named.conf");
	#endif

	wlan_status = wpa_connect_status();			//update the status
	if(read_wifi_mode()!=WIFI_CLIENT_GETAUTOIP_ERR&&read_wifi_mode()!=WIFI_CLIENT_GETAUTOIP_OK&&read_wifi_mode()!=WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR&&read_wifi_mode()!=WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		modify_wifi_mode(WIFI_CONCURRENT_CLIENT_ENABLE);
#else
		modify_wifi_mode(WIFI_CLIENT_MODE_ENABLE);
#endif
	}

	return 0;
}

static int wifi_start(void * handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	rtn = wifi_start_fun();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_start(void)
{
	int rtn = wifi_start_fun();
	return rtn;
}
#endif
static int rtn_pre;
static int wifi_startscan(void * handle)
{
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	wifiautodbg_info("now is in wifi startscan");
//	delete_APInfo();
#if (defined(EZCAST_LITE_ENABLE) && EZCAST_LITE_ENABLE==1) 
	if(is_wifi_connect_ready() == 1)
#endif
	{
		rtn = wifi_send_msg(&wifi_s,WIFI_CMD_SCAN);
	}
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_wifi_startscan(void)
{
	int rtn=0;
	wifidbg_info("now is in wifi startscan");
//	delete_APInfo();
	rtn = wifi_send_msg(&wifi_s,WIFI_CMD_SCAN);
	return rtn;
}
#endif
static int wifi_getscanresults(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	//wifidbg_info("now is to get scan results");
	//wifidbg_info("AP num is %d",wifi_s.AP_num);
//printf("[debug %s %d]>>>>>>>>>>>>>>>>>>>>>>apnum:%d\n",__func__,__LINE__,wifi_s.AP_num);	
	Swfext_PutNumber(wifi_s.AP_num);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_getscanresults(void)
{
	//wifidbg_info("now is to get scan results");
	//wifidbg_info("AP num is %d",wifi_s.AP_num);
//printf("[debug %s %d]>>>>>>>>>>>>>>>>>>>>>>apnum:%d\n",__func__,__LINE__,wifi_s.AP_num);	
	return wifi_s.AP_num;
}
#endif
int wifi_stop_process(){
	//printf("debug %s %d>>>>>>>>>>>>>>>>\n",__func__,__LINE__);	
	if(wifi_client_thread_is_running==0 ){
		printf("wifi has stop!\n");
		return -1;
	}
	
	system("wpa_cli -p/tmp/wpa_supplicant sta_autoconnect 0");
	printf("[%s %d] sta autoconnect disable\n",__func__,__LINE__);

	wifi_thread_exit(&wifi_s);
	
	wifi_autoconnect_exit(&wifi_s);

	closeCtrlConnection();
	
	kill_wpa_supplicant();
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
	wifiBridgeProcess(WIFI_BRIDGE_DEL_BR0);
#endif
	wlan_status = WIFI_DISCONNECTED;
	printf("wifi stop  OK\n");
		
	wifi_client_thread_is_running=0;
//printf("%s >>>>>>> wifi_client_thread_is_running:%d\n",__func__,wifi_client_thread_is_running);	


	return 0;
}

static int wifi_stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	wifi_stop_process();		//stop the wifi related processes

	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
void ezcast_wifi_stop(void)
{
	wifi_stop_process();		//stop the wifi related processes
}
#endif
static int __add_network()
{
	int ret=0;
	AP_info* ap_new = NULL;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL){
		ap_new = (AP_info *)malloc(sizeof(AP_info));
		wifi_info->AP_num++;
	}
	else{
		wifi_info->AP_num++;
		ap_new = (AP_info *)realloc((void *)wifi_info->ap_entry, wifi_info->AP_num * sizeof(AP_info));		
	}
	if(ap_new != NULL){
		wifi_info->ap_entry = ap_new;
		ret = 0;
	}
	else{
		wifidbg_info("wifi_s AP_entry realloc failed");
		ret = -1;
	}	
	selected_ap=wifi_info->AP_num-1;
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	
	return ret;
}

static int wifi_addnetwork(void * handle)
{
	int rtn = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi addnetwork");
	rtn = __add_network();
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_addnetwork(void)
{
	int rtn = 0;
	
	wifidbg_info("now is in wifi addnetwork");
	rtn = __add_network();
	
	return rtn;

}
#endif

static int wifi_getssid(void * handle)
{
	int index = 0;
	char * AP_ssid = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	
	//wifidbg_info("now is in wifi_getssid");
	index = Swfext_GetNumber();
	
	memset(tmp_infobuf,0,WIFI_BUF_LEN);
	__get_ssid(index,tmp_infobuf,WIFI_BUF_LEN);
	Swfext_PutString(tmp_infobuf);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
char* ezcast_wifi_getssid(int index)
{
	char * AP_ssid = NULL;
	
	//wifidbg_info("now is in wifi_getssid");
	
	memset(tmp_infobuf,0,WIFI_BUF_LEN);
	__get_ssid(index,tmp_infobuf,WIFI_BUF_LEN);
	return tmp_infobuf;
}
#endif

int set_hotspot_connect_status(){
	return (hotspot_connect_status = time(NULL));
}

int get_hotspot_connect_status(){
	return hotspot_connect_status;
}

int get_index_ssid(char *ssid){
	int i = 0;
	char ssid_info[WIFI_BUF_LEN];
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	
	for(i=0; i<wifi_info->AP_num; i++){
		__get_ssid(i,ssid_info,WIFI_BUF_LEN);
		if(strcmp(ssid, ssid_info) == 0)
			return i;
	}

	return -1;
}

struct hotspotApInfo_s{
	int isReady;
	char ssid[128];
	char psk[128];
};
struct hotspotApInfo_s hotspotApInfo;

void hotspotApinfoStore(char *ssid, char *psk){
	snprintf(hotspotApInfo.ssid, sizeof(hotspotApInfo.ssid), "%s", ssid);
	snprintf(hotspotApInfo.psk, sizeof(hotspotApInfo.psk), "%s", psk);
	hotspotApInfo.isReady = 1;
}

void hotspotApinfoClean(){
	memset(hotspotApInfo.ssid, 0, sizeof(hotspotApInfo.ssid));
	memset(hotspotApInfo.psk, 0, sizeof(hotspotApInfo.psk));
	hotspotApInfo.isReady = 0;
}

#if EZCAST_LITE_ENABLE
websetting_wifi_info_t * get_websetting_wifi_info(){
	return &websetting_wifi_info;
}

static int wifi_web_connect(void * handle)
{
	int index = 0;
	SWFEXT_FUNC_BEGIN(handle);

	int hotspot_status = get_hotspot_connect_status();
	if(hotspot_status > 0 && (time(NULL)-hotspot_status) < HOTSPOT_READY_TIMEOUT){
		if(hotspotApInfo.isReady == 1){
			EZCASTLOG("To connect: [%s][%s]\n", hotspotApInfo.ssid, hotspotApInfo.psk);
			hotspot_connect(hotspotApInfo.ssid, hotspotApInfo.psk);
		}
	}else{
		websetting_wifi_info_t *connect_info = get_websetting_wifi_info();
		index = get_index_ssid(connect_info->websetting_ssid);
		EZCASTLOG("::: index = %d :::\n", index);
		if(index >= 0){
			websetting_connect(connect_info->websetting_ssid, connect_info->websetting_psk, index);
		}else{
			websetting_add_connect(connect_info->websetting_ssid, connect_info->websetting_psk, connect_info->websetting_authen_type);
		}
	}
	
	SWFEXT_FUNC_END();	
}

#endif
static int wifi_setssid(void * handle)
{
	int rtn = 0;
	int index = 0;
	char * AP_ssid = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setssid");
	index = Swfext_GetNumber();
	AP_ssid = Swfext_GetString();
	wifidbg_info("ap ssid is %s",AP_ssid);
	rtn = __set_ssid(index,AP_ssid);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_setssid(int index,char * AP_ssid)
{
	int rtn = 0;
	wifidbg_info("now is in wifi_setssid");
	wifidbg_info("ap ssid is %s",AP_ssid);
	rtn = __set_ssid(index,AP_ssid);
	return rtn;
}
#endif
static int wifi_getsingal(void * handle)
{
	int index = 0;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	//wifidbg_info("now is in wifi_getsingal");
	
	index = Swfext_GetNumber();
	rtn = __get_signal(index);
	
	if(rtn>99)
		rtn=99;
	if(rtn<1)
		rtn = 1 ;
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_getsingal(int index)
{
	int rtn=0;
	//wifidbg_info("now is in wifi_getsingal");
	
	rtn = __get_signal(index);
	
	if(rtn>99)
		rtn=99;
	if(rtn<1)
		rtn = 1 ;
	return rtn;
}
#endif
static int wifi_getconfexist(void * handle)
{
	int rtn = -1;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_getconfexist");
	selected_ap = Swfext_GetNumber();
	rtn = __check_conf_exist(selected_ap);
	printf("[debug info :%s,%d]:rtn ========== %d\n",__FUNCTION__,__LINE__,rtn);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_getconfexist(int index)
{
	int rtn = -1;
	wifidbg_info("now is in wifi_getconfexist");
	selected_ap = index;
	rtn = __check_conf_exist(selected_ap);
	printf("[debug info :%s,%d]:rtn ========== %d\n",__FUNCTION__,__LINE__,rtn);
	return rtn;
}
#endif
static int wifi_setwepkeyindex(void * handle)
{
	int rtn = 0;
	int index = 0;
	int key_index = 0;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setwepkeyindex");
	index = Swfext_GetNumber();
	key_index = Swfext_GetNumber();
	rtn = __set_keyidx(index,key_index);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int __set_passwd(int sel_ap,char *passwd,int passwd_len)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	if(wifi_info==NULL || wifi_info->AP_num==0){
		wifidbg_err("set passwd error wifi_info=0x%x",wifi_info);
		return WPA_PWD_ERROR;
	}
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap >= wifi_info->AP_num){
		ret= WPA_PWD_ERROR;goto __end;
	}
		
	if(wifi_info->ap_entry[sel_ap].authen==WPA && (passwd_len<8 || passwd_len>63)){
		wifidbg_err("WPA mode, password length must be in [8,63]");
		ret = WPA_PWD_ERROR;goto __end;
	}
	else if(wifi_info->ap_entry[sel_ap].authen==WEP && (passwd_len!=10 && passwd_len!=26 && passwd_len!=32) && (passwd_len!=5 && passwd_len!=13 && passwd_len!=16)){
		wifidbg_err("WEP mode, if Hexadecimal, password length must 10 or 26 or 32, if ASCII, password length must 5 or 13 or 16");
		ret = WEP_PWD_ERROR;goto __end;
	}
	memset(wifi_info->ap_entry[sel_ap].ap_password,0,sizeof(wifi_info->ap_entry[sel_ap].ap_password));
	strncpy(wifi_info->ap_entry[sel_ap].ap_password, passwd, passwd_len);
	wifidbg_info("%s, len is %d",wifi_info->ap_entry[sel_ap].ap_password, strlen(wifi_info->ap_entry[sel_ap].ap_password));
	
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

static int wifi_setpassword(void * handle)
{
	int rtn = 0;
	char * ap_password = NULL;
	int password_length = 0;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setpassword");
	selected_ap = Swfext_GetNumber();
	ap_password = Swfext_GetString();
	password_length = strlen(ap_password);
	if(selected_ap<0 || selected_ap>=wifi_s.AP_num){
		wifidbg_err("AP index is out of range, should be [0,%d]", wifi_s.AP_num-1);
		rtn = INDEX_OUTRANGE;
		goto wifi_setpassword_out;
	}
	printf("ap password is %s,len is %d",ap_password,password_length);
	rtn = __set_passwd(selected_ap,ap_password,password_length);
	password_for_display=ap_password;
wifi_setpassword_out:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_wifi_setpassword(int index,char * ap_password)
{
	int rtn = 0;
	int password_length = 0;
	wifidbg_info("now is in wifi_setpassword");
	selected_ap = index;
	password_length = strlen(ap_password);
	if(selected_ap<0 || selected_ap>=wifi_s.AP_num){
		wifidbg_err("AP index is out of range, should be [0,%d]", wifi_s.AP_num-1);
		rtn = INDEX_OUTRANGE;
		goto wifi_setpassword_out;
	}
	printf("ap password is %s,len is %d",ap_password,password_length);
	rtn = __set_passwd(selected_ap,ap_password,password_length);
	password_for_display=ap_password;
wifi_setpassword_out:
	return rtn;
}
#endif
static int wifi_getpassword_for_display(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	printf("[debug %s %d] >>>>\n",__func__,__LINE__);	
	wpa_connect_status();
	if(wlan_current_status.current_status_value == WIFI_COMPLETED)
		Swfext_PutString(wlan_current_status.passphrase);
	else
		Swfext_PutString("");
	SWFEXT_FUNC_END();
}


static int wifi_get_connecting_ssid(void * handle)
{
	char *connecting_ssid=NULL;
	char *key_content="ssid=\"";
	SWFEXT_FUNC_BEGIN(handle);

	connecting_ssid=wifi_conf_content_for_display(key_content);

	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutString(connecting_ssid);

//	Swfext_PutString(wlan_current_status.ssid);
	SWFEXT_FUNC_END();
}

static int wifi_get_connected_ssid(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
//printf("[debug %s %d] >>>>\n",__func__,__LINE__);	
	wpa_connect_status();
	if(wlan_current_status.current_status_value == WIFI_COMPLETED)
		Swfext_PutString(wlan_current_status.ssid);
	else
		Swfext_PutString("");
	SWFEXT_FUNC_END();
}
#if EZCAST_ENABLE
int getConnectedSsid(char *val, unsigned int len){
	if(wlan_current_status.current_status_value == WIFI_COMPLETED){
		snprintf(val, len, "%s", wlan_current_status.ssid);
		return 0;
	}else{
		return -1;
	}
}
#endif
#if WEBSETTING_ENABLE
char* ezcast_wifi_get_connected_ssid(void)
{
	
//printf("[debug %s %d] >>>>\n",__func__,__LINE__);
	static char *connected_ssid=NULL;
	wpa_connect_status();
	if(wlan_current_status.current_status_value == WIFI_COMPLETED)
	{
		connected_ssid=wlan_current_status.ssid;
		//printf("connected_ssid=============%s\n",connected_ssid);
		return connected_ssid;	
	}		
	else
		return "";
}
#endif
static int wifi_saveconf(void * handle)
{
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_saveconf");
	rtn = save_conffile();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_wifi_saveconf(void)
{
	int rtn = 0;
	wifidbg_info("now is in wifi_saveconf");
	rtn = save_conffile();
	return rtn;
}
#endif
static int wifi_saveaddedAPconf(void * handle)
{
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_saveaddedapconf");
	rtn = save_addedAP_conffile();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_wifi_saveaddedAPconf(void)
{
	int rtn = 0;
	wifidbg_info("now is in wifi_saveaddedapconf");
	rtn = save_addedAP_conffile();
	return rtn;
}
#endif

static int wifi_connectAP(void * handle)
{
	int rtn = 0;

	SWFEXT_FUNC_BEGIN(handle);
	
	if(connect_sem_flag == 0){														//if the connect sem flag is free, send the connect cmd
		if(strcmp(temp_filename,selected_filename)!=0&&strlen(temp_filename)!=0){		//copy the conf file path to selected_filename
			memset(selected_filename,0,WIFI_BUF_LEN);
			strcpy(selected_filename,temp_filename);	
			wifiautodbg_info("selected_filename is %s",selected_filename);
		}
		wifiautodbg_info("now is in wifi_connectAP");
		rtn = wifi_send_msg(&wifi_s,WIFI_CMD_CONNECT);
	}
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_connectAP(void)
{
	int rtn = 0;

	
	if(connect_sem_flag == 0){														//if the connect sem flag is free, send the connect cmd
		if(strcmp(temp_filename,selected_filename)!=0&&strlen(temp_filename)!=0){		//copy the conf file path to selected_filename
			memset(selected_filename,0,WIFI_BUF_LEN);
			strcpy(selected_filename,temp_filename);	
			wifiautodbg_info("selected_filename is %s",selected_filename);
		}
		wifiautodbg_info("now is in wifi_connectAP");
		rtn = wifi_send_msg(&wifi_s,WIFI_CMD_CONNECT);
	}
	return rtn;
}
#endif
static int wifi_getAuthenType(void * handle)
{
	int index = 0;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	//wifidbg_info("now is in wifi_getAuthenType");
	index = Swfext_GetNumber();
	
	rtn = __get_authentype(index);
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_getAuthenType(int index)
{
	int rtn=0;
	//wifidbg_info("now is in wifi_getAuthenType");
	
	rtn = __get_authentype(index);
	
	return rtn;
}
#endif
static int wifi_setAuthenType(void * handle)
{
	int rtn = 0;
	int index = 0;
	int auth_type = -1;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setAuthenType");
	index = Swfext_GetNumber();
	auth_type = Swfext_GetNumber();
	
	rtn = __set_authentype(index,auth_type);
	
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_setAuthenType(int index,int auth_type)
{
	int rtn = 0;
	wifidbg_info("now is in wifi_setAuthenType");
	
	rtn = __set_authentype(index,auth_type);
	
	return rtn;
}
#endif

int get_ap_close_flag(){
	return apmode_close_flag;
}
int set_ap_close_flag(int val){
	 apmode_close_flag=val;
	 return 0;
}
void wifi_softap_close(void){
	printf("[%s-%d] Softap close\n", __func__, __LINE__);
	if(access("/tmp/hostapd",F_OK)==0){                    //stop softap 
		system("killall hostapd");
		system("killall udhcpd");
		system("ifconfig wlan1 down");                    //without this,the RTSP connect would be failed in wlan0 ifconfig wlan3 down
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		system("ifconfig wlan3 down");
		system("ifconfig br0 down");
	#endif
	}
	apmode_close_flag=1;
}
void wifi_softap_start(void){
	printf("[%s-%d] Softap close\n", __func__, __LINE__);
    if(access("/tmp/hostapd/wlan1",F_OK)!=0||access("/tmp/hostapd/wlan3",F_OK)!=0)
    {
		softap_start();
    }
	apmode_close_flag=0;
}
static int wifi_getStatus(void * handle)
{
	int index = 0;
	SWFEXT_FUNC_BEGIN(handle);
	//printf("[Debug Info:%s,%d]:status is %d index=%d",__FUNCTION__,__LINE__,wlan_status,index);
	Swfext_PutNumber(wlan_status);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_getStatus(void)
{
	//printf("[Debug Info:%s,%d]:status is %d",__FUNCTION__,__LINE__,wlan_status);
	
	return wlan_status;
}
#endif
static int wifi_setdefault(void * handle)
{
	char conf_file[256] = "/etc/wireless-tkip.conf";
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setdefault");
	file_opt(MNAVI_CMD_FILE_DEL,NULL,selected_filename);
	memset(selected_filename,0,strlen(selected_filename));
	strncpy(selected_filename, conf_file, strlen(conf_file));
	memset(selected_ssid,0,strlen(selected_ssid));
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();	
}

static int wifi_getindexofselectAP(void * handle)
{
	int index = 0;
	SWFEXT_FUNC_BEGIN(handle);
	index = indexofselectAP();
	Swfext_PutNumber(index);
	SWFEXT_FUNC_END();	
}

static int wifi_setIP(void * handle)
{
	int rtn = 0;
	char * ip_addr = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setIP");
	ip_addr = Swfext_GetString();
	wifidbg_info("ip addr is %s",ip_addr);
	memcpy(wlan_current_status.ip_address,ip_addr,strlen(ip_addr));
	rtn = setIP();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}

static int wifi_getIP(void * handle)
{
	int rtn = 0;
	char ip_addr[18] = {0};
	SWFEXT_FUNC_BEGIN(handle);
	
	wifidbg_info("now is in wifi_getIP");
	wifidbg_info("ip address is %s",wlan_current_status.ip_address);
	memcpy(ip_addr,wlan_current_status.ip_address,strlen(wlan_current_status.ip_address));
	Swfext_PutString(ip_addr);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
char* ezcast_wifi_getIP(void)
{
	int rtn = 0;
	static char ip_addr[18] = {0};
	
	wifidbg_info("now is in wifi_getIP");
	wifidbg_info("ip address is %s",wlan_current_status.ip_address);
	memset(ip_addr,0,sizeof(ip_addr));
	memcpy(ip_addr,wlan_current_status.ip_address,strlen(wlan_current_status.ip_address));
	return ip_addr;
}
#endif
static int wifi_setrouter(void * handle)
{
	int rtn = 0;
	char * router = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setrouter");
	router = Swfext_GetString();
	wifidbg_info("router is %s",router);
	memcpy(wlan_current_status.router,router,strlen(router));
	rtn = setRoute();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}

static int wifi_getrouter(void * handle)
{
	char router[18] = {0};
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_getrouter");
	wifidbg_info("router is %s",wlan_current_status.router);
	memcpy(router,wlan_current_status.router,strlen(wlan_current_status.router));
	Swfext_PutString(router);
	SWFEXT_FUNC_END();	
}

static int wifi_setDNS(void * handle)
{
	int rtn = 0;
	int DNS_index = 0;
	char * DNS_server = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_setDNS");
	DNS_index = Swfext_GetNumber();
	DNS_server = Swfext_GetString();
	wifidbg_info("DNS server is %s",DNS_server);
	if(DNS_index==0){
		memset(wlan_current_status.DNS_server,0,sizeof(wlan_current_status.DNS_server));
		memcpy(wlan_current_status.DNS_server,DNS_server,strlen(DNS_server));
	}
	else{
		memcpy(wlan_current_status.DNS_server+strlen(wlan_current_status.DNS_server),DNS_server,strlen(DNS_server));
	}
	wlan_current_status.DNS_server[strlen(wlan_current_status.DNS_server)] = ' ';
	wifidbg_info("DNS server is %s",wlan_current_status.DNS_server);
	rtn = setDNS(DNS_index,DNS_server);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}

static int wifi_getDNS(void * handle)
{
	char DNS_server[50] = {0};
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_getDNS");
	wifidbg_info("DNS server is %s",wlan_current_status.DNS_server);
	memcpy(DNS_server,wlan_current_status.DNS_server,strlen(wlan_current_status.DNS_server));
	Swfext_PutString(DNS_server);
	SWFEXT_FUNC_END();	
}

static int wifi_get_signal_level(void * handle)
{
	int signal_level = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	//printf("[debug %s %d] >>>>\n",__func__,__LINE__);	
	//wpa_connect_status();
	if(wifi_client_thread_is_running == 1){
		signal_level = atoi(wlan_current_status.signal_level);
	//	printf("[debug %s %d] :signal_level is %d\n",__func__,__LINE__,signal_level);	
		Swfext_PutNumber(signal_level);
	}		
	else
		Swfext_PutNumber(signal_level);
	
	SWFEXT_FUNC_END();
}


static int wifi_deleteconf(void * handle)
{
	int index = -1;
	int rtn=0;
	int ret=0;
	char conf_file[256] = {0};
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_deleteconf");
	index = Swfext_GetNumber();
	rtn = delete_conffile(index);
	ret = delete_ipfile(index);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_deleteconf(int index)
{
	int rtn=0;
	int ret=0;
	char conf_file[256] = {0};
	wifidbg_info("now is in wifi_deleteconf");
	rtn = delete_conffile(index);
	ret = delete_ipfile(index);
	return rtn;
}
#endif
static int wifi_autoconnect(void * handle)
{
	int rtn = -1;
	SWFEXT_FUNC_BEGIN(handle);
	wifidbg_info("now is in wifi_autoconnect");
	
	rtn = auto_connect();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
static int wifi_dongle_mode_change(void * handle)
{
	int rtn = -1;
	int mode=-1;
	SWFEXT_FUNC_BEGIN(handle);
	mode=Swfext_GetNumber();
	rtn = process_wifi_function_switch(mode);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_dongle_mode_change_close_process()
{
	process_wifi_function_switch(CLOSE_WIFI_PROCESS);
}
#endif

static int wifi_get_softap_ssid(void * handle)
{
	char  *ssid=NULL;
	char  *ssid_search_ralink="SSID=";
	char  *ssid_search_realtek="ssid=";
	int dongle_type=0;
	SWFEXT_FUNC_BEGIN(handle);
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(access(HOSTAPD_CON_FILE,0)==0)
		ssid=get_modify_line_realtek(HOSTAPD_CON_FILE,"ssid=");
	Swfext_PutString(ssid);
	#else
	dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		ssid=get_soft_ap_entry(1,ssid_search_realtek);
	else if(dongle_type==2)
		ssid=get_soft_ap_entry(2,ssid_search_ralink);
     
	Swfext_PutString(ssid);
	if(ssid){
		free(ssid);
		ssid = NULL;
	}
	#endif
	SWFEXT_FUNC_END();	
}

static int wifi_get_softap_ssid_Probox24g(void * handle)
{
	char *tmp_24gssid=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	if(access("/mnt/user1/softap/rtl_hostapd_02.conf",0)==0)
		tmp_24gssid=get_modify_line_realtek("/mnt/user1/softap/rtl_hostapd_02.conf","ssid=");
	printf("ssid_Probox24g    =%s\n",tmp_24gssid);
	Swfext_PutString(tmp_24gssid);
	SWFEXT_FUNC_END();
}




#if WEBSETTING_ENABLE
EXPORT_SYMBOL
char * ezcast_wifi_get_softap_ssid(void)
{
	static char  *ssid=NULL;
	char  *ssid_search_ralink="SSID=";
	char  *ssid_search_realtek="ssid=";
	int dongle_type=0;
	dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		ssid=get_soft_ap_entry(1,ssid_search_realtek);
	else if(dongle_type==2)
		ssid=get_soft_ap_entry(2,ssid_search_ralink);

	return ssid;
}
#endif
static int wifi_get_softap_psk(void * handle)
{
	char  *psk=NULL;
	char  *psk_search_ralink="WPAPSK=";
	char  *psk_search_realtek="wpa_passphrase=";
	char  *device=NULL;
	int dongle_type=0;
	SWFEXT_FUNC_BEGIN(handle);
	dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		psk=get_soft_ap_entry(1,psk_search_realtek);
	else if(dongle_type==2)
		psk=get_soft_ap_entry(2,psk_search_ralink);
	if(psk != NULL && strcmp(psk,"@@OPEN@@")==0){
		bzero(psk,4096);
		strncpy(psk,"        ",4096);
	}
	
	Swfext_PutString(psk);

	if(psk){
		free(psk);
		psk = NULL;
	}
	
	SWFEXT_FUNC_END();	
}

// used for EZCast
static int wifi_get_default_psk(void *handle){
	SWFEXT_FUNC_BEGIN(handle);

	char *ssid = NULL;
	char def_psk[20];
	int dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		ssid=get_soft_ap_entry(1,"ssid=");
	else if(dongle_type==2)
		ssid=get_soft_ap_entry(2,"SSID=");

	if(ssid != NULL){
		if(getDefaultPsk(ssid, def_psk, sizeof(def_psk)) == NULL){
			sprintf(def_psk, "null");
		}
		free(ssid);
	}else{
		sprintf(def_psk, "null");
	}

	Swfext_PutString(def_psk);

	SWFEXT_FUNC_END();
}

#if WEBSETTING_ENABLE
char  * ezcast_wifi_get_softap_psk(void)
{
	char  *psk=NULL;
	char  *psk_search_ralink="WPAPSK=";
	char  *psk_search_realtek="wpa_passphrase=";
	char  *device=NULL;
	int dongle_type=0;
	dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		psk=get_soft_ap_entry(1,psk_search_realtek);
	else if(dongle_type==2)
		psk=get_soft_ap_entry(2,psk_search_ralink);
	
	if(psk != NULL && strcmp(psk,"@@OPEN@@")==0){
		bzero(psk,4096);
		strncpy(psk,"        ",4096);
	}
	return psk;
}
#endif

#if WEBSETTING_ENABLE
char* wifi_get_ignoressid_flag(void)
{
	char  *val_str=NULL;
	char  *ssid_flag_realtek="ignore_broadcast_ssid=";
	
	val_str=get_soft_ap_entry(1,ssid_flag_realtek);
	
	return val_str;
}
int wifi_set_ignoressid_flag(char *val)   //please note: it is  open for probox only 
{
	#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)
	set_ignoressid(RTL_BAND_5G,val);
	#endif
	set_ignoressid(RTL_BAND_24G,val);
	int oldVal = ezCastGetNumConfig(CONFNAME_CTLMODE);
#if ROUTER_ONLY_ENABLE
	if(oldVal == CTLMODE_ROUTERONLY){
	if(get_ap_close_flag() == 0 ){
			process_wifi_function_switch(RESTART_WIFI_PROCESS);
		}	
	}
	else
		process_wifi_function_switch(RESTART_WIFI_PROCESS);
			
#else		
	process_wifi_function_switch(RESTART_WIFI_PROCESS);
#endif
	//wifi_softap_close();
	//wifi_softap_start();
}
#endif
static int wifi_get_softap_mode(void * handle)
{
	char  *mode=NULL;
	char  *mode_search_ralink="EncrypType=";
	char  *mode_search_realtek="wpa=";
	int dongle_type=0;
	
	char  *mode_search_result=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	dongle_type = dongle_get_device_changing();
	if(dongle_type==2){	
		mode_search_result=get_soft_ap_entry(2,mode_search_ralink);
		printf("mode_search=======%s\n",mode_search_result);
		if(strcmp(mode_search_result,"NONE")==0){
			mode="open";
		}
		else if(strcmp(mode_search_result,"TKIP")==0){
			mode="wpa";
		}
		else if(strcmp(mode_search_result,"AES")==0){		
			mode="wpa2";
		}
	}

	else if(dongle_type==1){	
		mode_search_result=get_soft_ap_entry(1,mode_search_realtek);
		printf("mode_search=======%s\n",mode_search_result);
		if(strcmp(mode_search_result,"0")==0){
			mode="open";
		}
		else if(strcmp(mode_search_result,"1")==0){
			mode="wpa";
		}
		else if(strcmp(mode_search_result,"2")==0){
			mode="wpa2";
		}
	}
	//printf("mode============================%s\n",mode);
	if(mode_search_result){
		free(mode_search_result);
		mode_search_result = NULL;

	}
	Swfext_PutString(mode);
	SWFEXT_FUNC_END();	
}
 int get_softap_channel()
{
	char *channel=NULL;
	char *channel_search_ralink="Channel=";
	char *channel_search_realtek="channel=";
	int dongle_type=0;
	int ret=0;
	dongle_type=dongle_get_device_changing();
	//
	if(dongle_type==1)
		channel=get_soft_ap_entry(1,channel_search_realtek);
	else if(dongle_type==2)
		channel=get_soft_ap_entry(2,channel_search_ralink);

	if(channel){
		ret=atoi(channel);
		free(channel);
		channel = NULL;
	}
	printf("get channel=======%d\n",ret);
	return ret;
}
static int wifi_get_softap_channel(void * handle)
{
	char *channel=NULL;
	char *channel_search_ralink="Channel=";
	char *channel_search_realtek="channel=";
	int dongle_type=0;

	SWFEXT_FUNC_BEGIN(handle);
	dongle_type=dongle_get_device_changing();
	//printf("dongle_type=======%d\n",dongle_type);
	if(dongle_type==1)
		channel=get_soft_ap_entry(1,channel_search_realtek);
	else if(dongle_type==2)
		channel=get_soft_ap_entry(2,channel_search_ralink);

		
	//printf("channel========%s\n",channel);
	Swfext_PutString(channel);

	if(channel){
		free(channel);
		channel = NULL;
	}
	SWFEXT_FUNC_END();	
}

static int wifi_get_softap_config_info(void *handle)
{
	int secrurity=-1;
	SWFEXT_FUNC_BEGIN(handle);
	secrurity=Swfext_GetNumber();

	ModifySettings(secrurity);
	
	SWFEXT_FUNC_END();	
}

static int wifi_get_softap_input_config_info(void *handle)
{
	char *softap_ssid=NULL;
	char *softap_psk=NULL;
	char * softap_channel=NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	softap_ssid=Swfext_GetString();
	softap_psk=Swfext_GetString();
	softap_channel=Swfext_GetString();

	putdown_softap_info(softap_ssid,softap_psk,softap_channel);

	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
void ezcast_change_psk(char *psk)
{
	char  *ssid=NULL;
	char  *ssid_search_ralink="SSID=";
	char  *ssid_search_realtek="ssid=";
	int dongle_type=0;
	char *channel=NULL;
	char *channel_search_ralink="Channel=";
	char *channel_search_realtek="channel=";	
	int secrurity=-1;
	struct sysconf_param sys_info;
    FILE * fp = NULL;
	if((strlen(psk)>=8)/*&&(strlen(psk)<=12)*/)
	{
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8251
		if(ezCastGetNumConfig(CONFNAME_IGNORESSID))
		{
			if(access("/tmp/hostapd",F_OK)==0){                    //stop softap 
				system("killall hostapd");
				system("killall udhcpd");
				system("ifconfig wlan1 down");                    
			}
			sleep(4);
		}
		#endif
		/************get ssid start***************/
		dongle_type = dongle_get_device_changing();
		if(dongle_type==1)
			ssid=get_soft_ap_entry(1,ssid_search_realtek);
		else if(dongle_type==2)
			ssid=get_soft_ap_entry(2,ssid_search_ralink);
		/************get ssid end***************/

		/************get channel start***************/
		dongle_type=dongle_get_device_changing();
		//printf("dongle_type=======%d\n",dongle_type);
		if(dongle_type==1)
			channel=get_soft_ap_entry(1,channel_search_realtek);
		else if(dongle_type==2)
			channel=get_soft_ap_entry(2,channel_search_ralink);
		/************get channel end***************/

		putdown_softap_info(ssid,psk,channel);	
		secrurity=2;
		ModifySettings(2);
		/************psk_changed start***************/
		if(_get_env_data(&sys_info)==0)
		{
			sys_info.psk_changed = 1;
			#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR)
			sys_info.softap_info.softap_psk_setted_flag = 1;
			memset(sys_info.softap_info.softap_ssid, 0, sizeof(sys_info.softap_info.softap_ssid));
			memset(sys_info.softap_info.softap_psk, 0, sizeof(sys_info.softap_info.softap_psk));
			get_softap_ssid(sys_info.softap_info.softap_ssid, sizeof(sys_info.softap_info.softap_ssid));
			get_softap_psk(sys_info.softap_info.softap_psk, sizeof(sys_info.softap_info.softap_psk));
			EZCASTLOG("ssid: %s, psk: %s\n", sys_info.softap_info.softap_ssid, sys_info.softap_info.softap_psk);
			#endif
			_save_env_data(&sys_info);
			_store_env_data();
		}
		ezCustomerSetPskChanged(1);			// JSON_SET_PSKCHANGED
        //Mos: leave Mark for psk changed
        fp = fopen("/mnt/vram/CUSTOM_PSK","w");
        fclose(fp);
		/************psk_changed end***************/
		if(ssid){
			free(ssid);
			ssid = NULL;
		}	
		if(channel){
			free(channel);
			channel = NULL;
		}
	}
}

void ezcast_set_custom_ssid(char *ssid)
{
	char  *psk=NULL;
	char  *ssid_search_realtek="ssid=";
	int dongle_type=0;
	char *channel=NULL;
	char *channel_search_realtek="channel=";	
	char *psk_search_realtek="wpa_passphrase=";
	int secrurity=-1;
	struct sysconf_param sys_info;
    FILE * fp = NULL;
	if((strlen(ssid)>=1)&&(strlen(ssid)<32))
	{
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8251
		if(ezCastGetNumConfig(CONFNAME_IGNORESSID))
		{
			if(access("/tmp/hostapd",F_OK)==0){                    //stop softap 
				system("killall hostapd");
				system("killall udhcpd");
				system("ifconfig wlan1 down");                    
			}
			sleep(4);
		}
		#endif

        fp = fopen("/mnt/vram/CUSTOM_SSID","w");
        if (fp != NULL){
            fputs(ssid, fp);
            fclose(fp);
        }

        /* Mos: When OTA upgrade from old version, device might not create this file
         * If user set custom ssid without reset to default, device will missing STD_SSID */
        if (access("/mnt/vram/STD_SSID", F_OK) != 0){
            system("hostname|tr -d '\n' > /mnt/vram/STD_SSID");
        }


		//ssid=get_soft_ap_entry(1,ssid_search_realtek);
		/*channel=get_soft_ap_entry(1,channel_search_realtek);
		psk=get_soft_ap_entry(1,psk_search_realtek);

		putdown_softap_info(ssid,psk,channel);	
		secrurity=2;
		ModifySettings(2);*/
		/************psk_changed start***************/
		/*if(_get_env_data(&sys_info)==0)
		{
			sys_info.ssid_changed = 1;
			#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR)
			sys_info.softap_info.softap_psk_setted_flag = 1;
			memset(sys_info.softap_info.softap_ssid, 0, sizeof(sys_info.softap_info.softap_ssid));
			memset(sys_info.softap_info.softap_psk, 0, sizeof(sys_info.softap_info.softap_psk));
			get_softap_ssid(sys_info.softap_info.softap_ssid, sizeof(sys_info.softap_info.softap_ssid));
			get_softap_psk(sys_info.softap_info.softap_psk, sizeof(sys_info.softap_info.softap_psk));
			#endif
			_save_env_data(&sys_info);
			_store_env_data();
		}
		ezCustomerSetPskChanged(1);			// JSON_SET_PSKCHANGED*/
		/************psk_changed end***************/
		if(psk){
			free(psk);
			psk = NULL;
		}	
		if(channel){
			free(channel);
			channel = NULL;
		}
	}
}

#endif
static int wifi_get_best_channel(void *handle)
{
	char *device=NULL;
	//
	SWFEXT_FUNC_BEGIN(handle);
	
	device = dongle_get_device();
	if(strcmp(device,"wlan0")==0){
		char channel_temp[4]={0};
		memset(channel_temp,0,4);
		if(0==access("/sys/module/8192du",F_OK)){
			if(rtl_band_use==RTL_BAND_24G){  //switch realtek wifi band
				best_channel_setting(channel_temp,RTL_BAND_24G);	
			}else{
				best_channel_setting(channel_temp,RTL_BAND_5G);
			}
		}else{
			best_channel_setting(channel_temp,RTL_BAND_24G);
		}
		//best_channel_setting(channel_temp,0x05);
		if(strlen(channel_temp)==0){
			
			Swfext_PutString("1");
			goto ___get_best_channel_out____;
		}
			
		
		Swfext_PutString(channel_temp);
	}
___get_best_channel_out____:
	SWFEXT_FUNC_END();	
}

static int wifi_remote_control(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int ret=-1;
	int direction=Swfext_GetNumber();
	ret=wifi_check_remote_control_thread(direction);
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	
}

static int wifi_get_dongle_type(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int ret=-1;
	char *device=NULL;
	device = dongle_get_device();
	if(strcmp(device,"wlan0")==0||strcmp(device,"wlan1")==0){
		ret=1;
	}
	if(strcmp(device,"ra0")==0){
		ret=2;
	}
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	
}
#if WEBSETTING_ENABLE
int ezcast_wifi_get_dongle_type(void)
{
	int ret=-1;
	char *device=NULL;
	device = dongle_get_device();
	if(strcmp(device,"wlan0")==0||strcmp(device,"wlan1")==0){
		ret=1;
	}
	if(strcmp(device,"ra0")==0){
		ret=2;
	}
	return ret;
}
#endif
static int auto_channel_choose_flag;
static int wifi_auto_channel_choose_put(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	auto_channel_choose_flag=Swfext_GetNumber();
	FILE *fp;
	int ret=-1;
	fp = fopen("/mnt/user1/softap/bestchannel_has_choose.txt","r");
	char callbuf[50]={0};
	char buf[1]={0};
	if(fp == NULL){
				
			sprintf(callbuf,"cp /mnt/bak/bestchannel_has_choose.txt /mnt/user1/softap/bestchannel_has_choose.txt");
			system(callbuf);
			fp = fopen("/mnt/user1/softap/bestchannel_has_choose.txt","r");
	}
	ret=fread(buf, 1, 1, fp);
	ret=fclose(fp);
	memset(buf,0,1);
	if(auto_channel_choose_flag==1)
		memcpy(buf,"1",1);
	else if(auto_channel_choose_flag==0)
		memcpy(buf,"0",1);
	printf("auto_channel_choose_flag_put======%d\n",auto_channel_choose_flag);
	if(buf!=NULL){
		fp = fopen("/mnt/user1/softap/bestchannel_has_choose.txt","wb+");
		if(fp == NULL){
			return -1;
		}
	fwrite(buf,1,1,fp);
	fflush(fp);
	int fd_write = fileno(fp);
	fsync(fd_write);
	fclose(fp);
	}
	SWFEXT_FUNC_END();	
}
extern wifi_dongle_type_emun dongle_type_divided;

static int wifi_auto_channel_choose_get(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	system("chmod -R 777 mnt/user1");
	if(dongle_type_divided==RALINK_dongle){
		auto_channel_choose_flag=0;
		goto ___wifi_auto_channel_choose_get__end__;
	}
	FILE *fp;
	int ret=-1;
	fp = fopen("/mnt/user1/softap/bestchannel_has_choose.txt","r");
	char callbuf[50]={0};
	char buf[1]={0};
	
	if(fp == NULL){
		if(_wifi_direct_copy_file("/mnt/bak/bestchannel_has_choose.txt","/mnt/user1/softap/bestchannel_has_choose.txt")==-1){
			auto_channel_choose_flag=1;
			goto ___wifi_auto_channel_choose_get__end__;
		}
		
		fp = fopen("/mnt/user1/softap/bestchannel_has_choose.txt","r");
		if(fp == NULL){
			auto_channel_choose_flag=1;
			goto ___wifi_auto_channel_choose_get__end__;
		}				
	}
	ret=fread(buf,1,1,fp);
	printf("strlen(buf)=======%d\n",strlen(buf));
	ret=fclose(fp);
	printf("buf=========%s\n",buf);
	if(strstr(buf,"1")!=NULL)
		auto_channel_choose_flag=1;
	else if(strstr(buf,"0")!=NULL)
		auto_channel_choose_flag=0;
	printf("auto_channel_choose_flag_get======%d\n",auto_channel_choose_flag);
	
___wifi_auto_channel_choose_get__end__:
	Swfext_PutNumber(auto_channel_choose_flag);
	
	SWFEXT_FUNC_END();	
}

int read_arping_reply_wifidisplay(){
	
	FILE *fp = NULL;
	char buf[256] ={0};
	char callbuf[128];
	int ret=-1;
	char *locate1=NULL;
___reread_arping_reply_wifidisplay___:
	if(access("/mnt/user1/wifidisplay/arping_result.txt",F_OK)==-1){
		printf("The file is not exsit!\n");
		return -1;
	}
	fp = fopen("/mnt/user1/wifidisplay/arping_result.txt","r");
		//fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
	
		if(fp == NULL){
			printf("Cann't open the file!\n");
			return -1;		
		}
	ret=fread(buf, 1, 256, fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	ret=fclose(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	if(buf==NULL){
		
		fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);
		goto ___reread_arping_reply_wifidisplay___;
	}
	locate1=strstr(buf,"Received 1 reply");
	
	if(locate1!=NULL)
		return 1;
	else 
		return 0;
		
}
extern char route_for_wifidisplay_valid[18];

static int wifi_check_wifidisplay_valid(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	char callbuf[50]={0};
	FILE *fp;
	int ret;
	char buf[256]={0};
	char mask_address[18]={0};
	int wifidisplay_valid=0;
	char *locate1=NULL;
	char *dongle_device=dongle_get_device();
	if(dongle_device==NULL)
		goto check_wifidisplay_valid_end;
/*	sprintf(callbuf,"arping -I %s -c 1 %s > /mnt/user1/wifidisplay/arping_result.txt",dongle_device,ip_address);
	printf("the callbuf is %s\n",callbuf);
	
	ret=system(callbuf);
	sleep(5);
	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);
	printf("ret == %d\n",ret);
	if(ret!=0)
    	printf("system call error:%s\n",strerror(errno));
*/ 

	
	memcpy(mask_address,route_for_wifidisplay_valid,18);
	printf("mask_address======%s\n",mask_address);
	sprintf(callbuf,"arping -I %s -c 1 %s",dongle_device,mask_address);
	fp =popen(callbuf, "r");
	
    if(NULL == fp)
    {
    	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);

        printf("popen error:%s\n",strerror(errno));
        return -1;
    }
	while(feof(fp) == 0){
		printf("@@@@@@@@@@@@@@@@@@@\n");
		ret=fread(buf, 1, 256, fp);
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	}
	ret = pclose(fp);
	printf("*******************\n%s\n",buf);
	locate1=strstr(buf,"Received 1 replies");
	if(locate1!=NULL)
		wifidisplay_valid=1;
	else 
		wifidisplay_valid=0;

 	//wifidisplay_valid=read_arping_reply_wifidisplay();
   
check_wifidisplay_valid_end:
	Swfext_PutNumber(wifidisplay_valid);
	
	SWFEXT_FUNC_END();		
}

static int wifi_restart_ap_mode(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	int ret=0;
	printf("function:%s,line:%d\n",__FUNCTION__,__LINE__);
#if EZCAST_ENABLE
	//wifi_close_fun();
	wifi_stop_process();
	if(access("/tmp/hostapd",F_OK)==0){
		 system("killall hostapd");
		 system("killall udhcpd");

		 system("ifconfig wlan1 down");
	}

	if(access("/tmp/wpa_supplicant",F_OK)==0){
		 system("killall wpa_supplicant");
		 system("killall udhcpc");
		 system("ifconfig wlan0 down");
	}

	if(access("/tmp/hostapd/wlan1",F_OK)!=0)
		ret=softap_start();
	else
		modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
	ret = wifi_start_fun();
	//ret = wifi_open_fun();
#else
	ret=wifi_concurrent_mode_init();
#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();		
} 

static int wifi_ez_remote_flag(void *handle){

	int ez_remote_start_flag = 0;
	SWFEXT_FUNC_BEGIN(handle);
	ez_remote_start_flag = ez_remote_enable_flag();
	Swfext_PutNumber(ez_remote_start_flag);
	SWFEXT_FUNC_END();		
} 
//int wps_connect_flag=0;
/*if client ap push the wps button,the wifi ap(router) should press push button in the 2 minuter*/
static int wifi_wps_pbutton_connect(void *handle)
{
	int rtn = -1;
	int pincode=0;
	SWFEXT_FUNC_BEGIN(handle);
	//wps_connect_flag=1;
	wifi_wps_disconnect();
	OSSleep(500);
	if(access("/tmp/wpa_supplicant",F_OK)!=0){
		system("wpa_supplicant -B -iwlan0 -c /etc/wireless-tkip.conf");
	}
	//system("wpa_cli -p/tmp/wpa_supplicant status");
	printf("%s %d<<<<<<<<<<<<\n",__func__,__LINE__);
	wps_flag= 0xa0;
	rtn=wifi_wps_method_connect(1,pincode);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
/*the user should input wifi  ap default pin code or current pin code*/
static int wifi_wps_pincode_connect(void *handle)
{
	int rtn = -1;
	int pincode;
	char *pin_code=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	wps_flag= 0xa0;
	pin_code = Swfext_GetString();
	//wps_connect_flag
	wifi_wps_disconnect();
	OSSleep(500);
	if(access("/tmp/wpa_supplicant",F_OK)!=0){
		system("wpa_supplicant -B -iwlan0 -c /etc/wireless-tkip.conf");
	}
	//system("wpa_cli -p/tmp/wpa_supplicant status");
//printf("debug %s %d >>>>>>>>>>>>>>>>>> pincode:%s\n",__func__,__LINE__,pin_code);
	pincode=atoi(pin_code);
printf("debug %s %d <<<<<<<<<<<<<<<<<<<< pincode:%d\n",__func__,__LINE__,pincode);
	rtn=wifi_wps_method_connect(2,pincode);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int wifi_exchange_ap(void *handle)
{
	int ret = -1;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	ret	= exchange_wifi_ap();
	
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();
}
int wifi_info_for_wifidisplay()
{
	char callbuf[50];
	int ret=-1;
	FILE *fp;
	char buf[512];
	sprintf(callbuf,"ifconfig wlan0");
	system(callbuf);
	printf("the call is %s\n",callbuf);	
	fp = popen(callbuf, "r");
	
    if(NULL == fp)
    {
    	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);

        printf("popen error:%s\n",strerror(errno));
        return -1;
    }
	
	ret=fread(buf, 1, 512, fp);
	
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
  	ret = pclose(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	
	char *locate1=strstr(buf,"inet addr:");
	if(locate1==NULL)
		return -1;
	locate1+=strlen("inet addr:");
	char *locate1_tmp=locate1;
	char *locate2=strstr(locate1," ");
	if(locate2==NULL)
		return -1;
	memset(wlan_current_status.ip_address,0,50);
	memcpy(wlan_current_status.ip_address,locate1_tmp,strlen(locate1_tmp)-strlen(locate2));
	printf("ip_address=======%s\n",wlan_current_status.ip_address);

	return 0;
}

void clear_wifi_error(int dongleType){
	int dongle_type = dongleType;
	char callbuf[50];
	printf("dongle_type=========clear_wifi_error=========%d\n",dongle_type);
	
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
	
	if(dongle_type == REALTEK_dongle){
		sprintf(callbuf,"ifconfig wlan0 down");
		system(callbuf);
	}
	else if(dongle_type == RALINK_dongle){
		sprintf(callbuf,"ifconfig ra0 down");
		system(callbuf);
	}
	
}

static int wifi_ip_mask_match_judge(void *handle)
{
		SWFEXT_FUNC_BEGIN(handle);
		int ret=-1;
		
		unsigned short int ip_1=Swfext_GetNumber();
		
		unsigned short int ip_2=Swfext_GetNumber();
		
		unsigned short int ip_3=Swfext_GetNumber();
		unsigned short int ip_4=Swfext_GetNumber();
		
		unsigned short int mask_1=Swfext_GetNumber();
		
		unsigned short int mask_2=Swfext_GetNumber();
		
		unsigned short int mask_3=Swfext_GetNumber();
		
		unsigned short int mask_4=Swfext_GetNumber();
		printf("ip1==%d\n,ip2==%d\n,ip3==%d\n,ip4==%d\n",ip_1,ip_2,ip_3,ip_4);
		
		printf("mask_1==%d\n,mask_2==%d\n,mask_3==%d\n,mask_4==%d\n",mask_1,mask_2,mask_3,mask_4);
		ret=mask_match_mask_judging(ip_1,ip_2,ip_3,ip_4,mask_1,mask_2,mask_2,mask_4);

//		printf("wifi_ip_mask_match_judge----------%d",ret);

		Swfext_PutNumber(ret);
		SWFEXT_FUNC_END();	
	}

static int wifi_ip_gateway_match(void *handle){
		SWFEXT_FUNC_BEGIN(handle);
		int ret=-1;
		
		unsigned short int ip_1=Swfext_GetNumber();
		
		unsigned short int ip_2=Swfext_GetNumber();
		
		unsigned short int ip_3=Swfext_GetNumber();
		unsigned short int ip_4=Swfext_GetNumber();
		
		unsigned short int gateway_1=Swfext_GetNumber();
		
		unsigned short int gateway_2=Swfext_GetNumber();
		
		unsigned short int gateway_3=Swfext_GetNumber();
		
		unsigned short int gateway_4=Swfext_GetNumber();
		//printf("ip1==%d\n,ip2==%d\n,ip3==%d\n,ip4==%d\n",ip_1,ip_2,ip_3,ip_4);
		
	//	printf("gateway_1==%d\n,gateway_2==%d\n,gateway_3==%d\n,gateway_4==%d\n",gateway_1,gateway_2,gateway_3,gateway_4);
		ret=GatewayMatchJudging(ip_1,ip_2,ip_3,ip_4,gateway_1,gateway_2,gateway_3,gateway_4);

		printf("wifi_ip_gateway_match_judge----------%d",ret);

		Swfext_PutNumber(ret);
		SWFEXT_FUNC_END();	

}

void find_wifi_gateway(char * ip_data){
	
	char gateway_data_part[50];
	char *locate1=NULL;
	char *locate2=NULL;
	char *locate3=NULL;
	char *ip_data_tmp=ip_data;
	memset(wlan_current_status.router,0,50);
	printf("ip_data=======%s\n",ip_data);
	locate1=strstr(ip_data,".");
	printf("locate1=====%s\n",locate1);
	if(locate1==NULL)
		return ;
	locate1++;
	locate2=strstr(locate1,".");
	if(locate2==NULL)
		return ;
	printf("locate2=====%s\n",locate2);
	locate2++;
	locate3=strstr(locate2,".");
	if(locate3==NULL)
		return ;

	
	printf("locate3=====%s\n",locate3);
	memset(gateway_data_part,0,50);
	memcpy(gateway_data_part,ip_data_tmp,strlen(ip_data_tmp)-strlen(locate3));
	strcat(gateway_data_part,".1");
	printf("gateway_data_part======%s\n",gateway_data_part);
	memcpy(wlan_current_status.router,gateway_data_part,strlen(gateway_data_part));
	//printf("*gateway_data=======%s\n",*gateway_data);
/*	char *gateway_data_tmp=NULL;
	gateway_data_tmp=*gateway_data;
	
	memset(gateway_data_tmp,0,strlen(gateway_data_tmp));
	memcpy(gateway_data_tmp,gateway_data_part,strlen(gateway_data_part));
*/	
}

static int wifi_open_manual(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char callbuf[128];
	int ret=-1;
	FILE *fp;
	char *ip=Swfext_GetString();
	char *mask=Swfext_GetString();
	char *gateway=Swfext_GetString();
  	char buf[256];
	int rtn;
	char *pub_interface=NULL;
	char *private_interface=NULL;
	char *private_subnet=NULL;
	int dongle_type = 0;

	ret = ifconfigConnection(ip,mask,gateway);
	if( ret == 0){
		saveManualip(ip,mask,gateway);
		//save_latestAPconf();
		wlan_status = WIFI_AUTO_IP_SUCCESS;
	}
	
wifi_open_end:		

	strncpy(wlan_current_status.ip_address,ip,strlen(ip));
	
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	
}


static int wifi_disconnect_ap(void *handle)
{
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	ret = wifi_reset();
	wlan_status = WIFI_DISCONNECTED_MANUAL;
	//udhcpd_dns_disable();
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_wifi_disconnect_ap(void)
{
	int ret = -1;
	ret = wifi_reset();
	wlan_status = WIFI_DISCONNECTED_MANUAL;
	//udhcpd_dns_disable();
	return ret;
}
#endif
/*
 * get all access device information
 */
int wifi_count_accessDevices_softap_24g()
{
	int sta_count = -1;
	FILE *pf;
	char tmp[100]={0x00};
	char cmd[64]={0x00};
	sprintf( cmd, "hostapd_cli -p/tmp/hostapd_02 all_sta > /tmp/softap_access_02.txt");

	system(cmd);
	pf = fopen("/tmp/softap_access_02.txt","r");

	if(pf){
		sta_count = 0;
		while(!feof(pf)){
			memset(tmp, 0x00,100);
			fgets(tmp,100,pf);
			if(strncmp( tmp, "dot11RSNAStatsSTAAddress=", 25) == 0){
				sta_count++;
			}
		}
		fclose(pf);
		unlink("/tmp/softap_access_02.txt");
	}

	return sta_count;	
}


int wifi_count_accessDevices_softap()
{
	int sta_count = -1;
	FILE *pf;
	char tmp[100]={0x00};
	char cmd[64]={0x00};
	sprintf( cmd, "hostapd_cli -p/tmp/hostapd all_sta > /tmp/softap_access.txt");
	system(cmd);
	pf = fopen("/tmp/softap_access.txt","r");

	if(pf){
		sta_count = 0;
		while(!feof(pf)){
			memset(tmp, 0x00,100);
			fgets(tmp,100,pf);
			if(strncmp( tmp, "dot11RSNAStatsSTAAddress=", 25) == 0){
				sta_count++;
			}
		}
		fclose(pf);
		unlink("/tmp/softap_access.txt");
	}

	return sta_count;	
}
#if (EZMUSIC_ENABLE&&GPIO_WIFI_ON_LED)
#define AE_ERR_NO_ERROR   0 		///< audio player no error
#define AE_STOP                     5 		///< play status:stop
static void start_timer(struct itimerval timer,int timer_udelay,int timer_delay)
{
	timer.it_interval.tv_usec = timer_udelay;
	timer.it_interval.tv_sec = timer_delay;
	timer.it_value.tv_usec = timer_udelay;
	timer.it_value.tv_sec = timer_delay;	
	if(setitimer( ITIMER_REAL, &timer, NULL) < 0 )
	{
		printf("set timer error!\n");
	}	
}
static void stop_timer(struct itimerval timer)
{
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	if(setitimer( ITIMER_REAL, &timer, NULL) < 0 )
	{
		printf("set timer error!\n");
	}
}
static void get_warn_music_status(void)
{
	int cur_status=ezcast_ae_get_state();
	if(cur_status == AE_STOP)
	{
		ezcast_ae_stop();
		ezcast_ae_close();
		stop_timer(SysCGI_priv.get_warn_music_status_twinkle_timer);
		//SystemInfo.setVolume(14);
	}	
}
static void play_warn_music(void)
{
	ezcast_ae_open();
	int audioOK=ezcast_ae_set_file("/usr/share/ezdata/start.mp3");
	if(audioOK==AE_ERR_NO_ERROR)
	{
		ezcast_ae_play();
		start_timer(SysCGI_priv.get_warn_music_status_twinkle_timer,0,1);
		signal(SIGALRM, get_warn_music_status);	
	}
	else
	{
		ezcast_ae_close();
	}	
}
static void CONNECTIONS_OK_LED_twinkle()
{
	SysCGI_priv.CONNECTIONS_OK_LED_twinkle_tick^=1;
	//printf("SysCGI_priv.CONNECTIONS_OK_LED_twinkle_tick============%d\n",SysCGI_priv.CONNECTIONS_OK_LED_twinkle_tick);
	if(SysCGI_priv.CONNECTIONS_OK_LED_twinkle_tick)
		set_gpio(GPIO_WIFI_ON_LED,0);//WIFI-ON-LED
	else
		set_gpio(GPIO_WIFI_ON_LED,1);//WIFI-OFF-LED	
}
#endif
static int wifi_count_accessDevices(void * handle)
{
     int deviceNum = 0;
     int ret=0; 
     SWFEXT_FUNC_BEGIN(handle);
     if(apmode_close_flag==0){
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
      if(get_last_ui()!=2)
	  	deviceNum = wifi_count_accessDevices_softap();
	deviceNum+=wifi_count_accessDevices_softap_24g();
#else
	 deviceNum = wifi_count_accessDevices_softap();
#endif
     	}
      Swfext_PutNumber(deviceNum);
	#if (EZMUSIC_ENABLE&&GPIO_WIFI_ON_LED)
	if(!SysCGI_priv.WIFI_CLIENT_OK_LED_flag)
	{
		if(deviceNum)
		{
			if(!SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag)
			{
			       //play_warn_music();			       			      
				SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag=1;
				start_timer(SysCGI_priv.CONNECTIONS_OK_LED_twinkle_timer,0,1);
				signal(SIGALRM, CONNECTIONS_OK_LED_twinkle);				
			}	
		}
		else
		{
			//printf("SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag============%d\n",SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag);
			if(SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag)
			{
				SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag=0;
				stop_timer(SysCGI_priv.CONNECTIONS_OK_LED_twinkle_timer);
				set_gpio(GPIO_WIFI_ON_LED,0);//WIFI-ON-LED	
			}				
		}		
	}
	#endif	 
     SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_wifi_count_accessDevices(void)
{
     int deviceNum = 0;
 #if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
      if(get_last_ui()!=2)
	  	deviceNum = wifi_count_accessDevices_softap();
	deviceNum+=wifi_count_accessDevices_softap_24g();
#else
	 deviceNum = wifi_count_accessDevices_softap();
#endif
     return deviceNum;
}
#endif


 static int wifi_softap_start_from_miracast(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int ret=0;
#if WIFI_DIRECT_CLIENT_ENABLE
	pac_system("iwpriv wlan1 p2p_set enable=0");
#else
	pac_system("iwpriv wlan0 p2p_set enable=0");
#endif

#if ROUTER_ONLY_ENABLE
      if(getConnectMode()!=CTLMODE_ROUTERONLY)
      	{
	
		if(access("/tmp/hostapd/wlan1",F_OK)!=0)
		{
			ret=softap_start();
			set_ap_close_flag(0);
		}
		else
			modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
      	}
#else
	if(access("/tmp/hostapd/wlan1",F_OK)!=0)
	{
		ret=softap_start();
		set_ap_close_flag(0);
	}
	else
		modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
#endif
	
	Swfext_PutNumber(ret);
 	SWFEXT_FUNC_END();	
 }
 
static int wifi_set_channel_region(void *handle)
{

	int rtn=-1;
	char *channelRegion=NULL ;
	struct sysconf_param sysCfg;
	SWFEXT_FUNC_BEGIN(handle);
	channelRegion=Swfext_GetString() ;
	
	printf("\t%d---now is in %s,channelRegion is %s\n",__LINE__,__FUNCTION__,channelRegion);
	if(_get_env_data(&sysCfg)!=0){
		printf("\t%d---now is in %s  get env data error!!\n",__LINE__,__FUNCTION__);
		goto _end;
	}
	
	printf("\t%d---now is in %s,sysCfg.wifiChannelRegion  is %s\n",__LINE__,__FUNCTION__,sysCfg.wifiChannelRegion);

	if(strcmp(channelRegion,sysCfg.wifiChannelRegion)==0){		//if the chosen is same as the previous one ,do nothing
		rtn = 0;
	}
	else		//if the chosen differ,save it
	{
		strcpy(sysCfg.wifiChannelRegion,channelRegion);
		if(_save_env_data(&sysCfg)!=0){
			printf("\t%d---now is in %s  save env data error!!\n",__LINE__,__FUNCTION__);
			goto _end;
		}
		else{
			if(_store_env_data()==0){
				rtn = 1;				
			}
			else
				rtn =  -1;
		}
	}
_end:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
		
}

static  int wifiGetChannelRegion(void *handle){
	struct sysconf_param sysCfg;
	SWFEXT_FUNC_BEGIN(handle);

	if(_get_env_data(&sysCfg)==0){
		if(strlen(sysCfg.wifiChannelRegion)>0){
			printf("\t%d---now is in %s,sysCfg.wifiChannelRegion is %s\n",__LINE__,__FUNCTION__,sysCfg.wifiChannelRegion);
			Swfext_PutString(sysCfg.wifiChannelRegion);
			goto _end;
		}
	}
		
	printf("\t%d---now is in %s,Rtl_ChannelRegion is %s\n",__LINE__,__FUNCTION__,Rtl_ChannelRegion);
	Swfext_PutString(Rtl_ChannelRegion);

_end:	
	SWFEXT_FUNC_END();
}


#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
static int hostcontrol_user_clean(void *handle){

	SWFEXT_FUNC_BEGIN(handle);
	ezcastpro_host_reset();
	SWFEXT_FUNC_END();	
}

#endif
static int send_msg_to_app(void *handle){
	char *cmd_as;
	int cmd_len;
	char cmd_send[100];
	int ret = -1;

	SWFEXT_FUNC_BEGIN(handle);

	cmd_as = Swfext_GetString();
	printf("command from as is: %s\n", cmd_as);
	ret = ezRemoteSendEzcastMsg(cmd_as);
	printf("send length is : %d\n", ret);
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();	
	return 0;
}

static int wifi_psk_changed(void * handle){
	struct sysconf_param sysCfg;

	SWFEXT_FUNC_BEGIN(handle);
	//printf("*************** password have changed!!!\n");
	if(_get_env_data(&sysCfg)==0){
		sysCfg.psk_changed = 1;
		_save_env_data(&sysCfg);
		_store_env_data();
	}
	ezCustomerSetPskChanged(1);			// JSON_SET_PSKCHANGED
	SWFEXT_FUNC_END();
}

static int wifi_isLastRouterApExist(void * handle){
	int ret = 0;
	DIR *dir;
	struct dirent *ptr;
	char *tmp;
	
	SWFEXT_FUNC_BEGIN(handle);

	dir = opendir("/mnt/vram/wifi");
	if(dir != NULL){
		while((ptr = readdir(dir)) != NULL){
			if(ptr->d_type != DT_DIR){
				//printf("d_name: %s, d_type: %d\n", ptr->d_name, ptr->d_type);
				tmp = strrchr(ptr->d_name, '.');
				if(tmp != NULL){
					//printf("tmp: %s\n", tmp);
					if(!strcmp(tmp, ".conf") && strcmp(ptr->d_name, "latestAP.conf") && strcmp(ptr->d_name, "resolv.conf") && strcmp(ptr->d_name, "p2p_ip_info.conf")){
						printf("[%s][%d] -- config file name: %s\n", __func__, __LINE__, ptr->d_name);
						ret = 1;
						break;
					}
				}
			}
		}
		closedir(dir);
	}

	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int wifi_softapIgnore(void * handle){
	FILE *fp = NULL;
	char buf[4096];
	int ret = -1;
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s][%d] -- Let softap ignore!!!\n", __func__, __LINE__);
	fp = fopen(HOSTAPD_CON_FILE,"r");
	if(fp != NULL){
		ret=fread(buf, 1, 4096, fp);
		ret=fclose(fp);
		fp = NULL;
		
		modify_line_realtek(buf,"ignore_broadcast_ssid=","1");
		modify_line_realtek(buf,"max_num_sta=","0");
		fp = fopen(HOSTAPD_CON_FILE,"wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			goto __END__;
		}
		ret=fwrite(buf, 1, 4096, fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);
	}

	system("hostapd_cli -p/tmp/hostapd reload /etc/rtl_hostapd_01.conf");
	//system("cat /etc/rtl_hostapd_01.conf");

__END__:	
	//Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int wifi_softapShow(void * handle){
	FILE *fp = NULL;
	char buf[4096];
	int ret = -1;
	char *tmp;
	int ignore_status=-1, max_num=-1, dongle_type;
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s][%d] -- Let softap show!!!\n", __func__, __LINE__);
	dongle_type = dongle_get_device_changing();
	tmp = get_soft_ap_entry(dongle_type,"ignore_broadcast_ssid=");
	if(tmp != NULL){
		ignore_status = atoi(tmp);
		free(tmp);
	}
	tmp = get_soft_ap_entry(dongle_type,"max_num_sta=");
	if(tmp != NULL){
		max_num = atoi(tmp);
		free(tmp);
	}
	if(ignore_status == 0 && max_num == 8){
		printf("[%s] [%d] -- rtl_hostapd_01.conf not change!!!\n", __func__, __LINE__);
		goto __END__;
	}
	
	fp = fopen(HOSTAPD_CON_FILE,"r");
	if(fp != NULL){
		ret=fread(buf, 1, 4096, fp);
		ret=fclose(fp);
		fp = NULL;
		
		modify_line_realtek(buf,"ignore_broadcast_ssid=","0");
		modify_line_realtek(buf,"max_num_sta=","8");
		fp = fopen(HOSTAPD_CON_FILE,"wb+");
		if(fp != NULL){
			ret=fwrite(buf, 1, 4096, fp);
			ret=fflush(fp);
			int fd_write = fileno(fp);
			ret=fsync(fd_write);
			ret=fclose(fp);
		}
	}

	system("hostapd_cli -p/tmp/hostapd reload /etc/rtl_hostapd_01.conf");

__END__:
	
	//Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
static int wifi_get_connect_mac_address(void * handle)
{
	char macaddress[24];
	int port=MAC_WLAN0;
	SWFEXT_FUNC_BEGIN(handle);
	port = Swfext_GetNumber();
	get_mac_address_info(macaddress, port);
	Swfext_PutString(macaddress);
	SWFEXT_FUNC_END();
}
static int wifi_get_connection_status(void * handle){
	SWFEXT_FUNC_BEGIN(handle);
	int wifi_status = get_connection_status();
	Swfext_PutNumber(wifi_status);
	SWFEXT_FUNC_END();
}
static int wifi_get_hotspot_status(void * handle){
	SWFEXT_FUNC_BEGIN(handle);
	int wifi_status = getHotspotStatus();
	Swfext_PutNumber(wifi_status);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
char * ezcast_wifi_get_connect_mac_address(int port)
{
	static char macaddress[24];
	if(port==1)
		//port=MAC_WLAN0;
		get_mac_address_info(macaddress, MAC_WLAN0);
	else
		get_mac_address_info(macaddress, MAC_WLAN3);
	return macaddress;
}
#endif
static int set_Airplay_Id(void * handle)
{
	 SWFEXT_FUNC_BEGIN(handle);
	 char *namestr = Swfext_GetString();
	 printf("[%s] [%d] -- set Airplay name: %s\n", __func__, __LINE__, namestr);
	 ezConfigSetAirplayId(namestr);
	 SWFEXT_FUNC_END();
}
static int set_DLNA_Id(void * handle)
{
	 SWFEXT_FUNC_BEGIN(handle);
	 char *namestr = Swfext_GetString();
	 printf("[%s] [%d] -- set DLNA name: %s\n", __func__, __LINE__, namestr);
	 ezConfigSetDlnaId(namestr);
	 SWFEXT_FUNC_END();
}

static int wifi_get_cur_channel(void *handle){
	SWFEXT_FUNC_BEGIN(handle);

	int chn = get_cur_channel();
	Swfext_PutNumber(chn);

	SWFEXT_FUNC_END();
}
#endif
static int wifi_Probox_rebridge(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	printf("rebridge wlan1 wlan3!!!\n");
	wifiBridgeProcess(WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3);
	SWFEXT_FUNC_END();
}





void start_routing(){
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
	enable_route_function(pub_interface,"br0","192.168.168.1/24",WIFI_ROUTING);
	printf("[%s][%d]wifiBridgeProcess\n",__func__,__LINE__);
#else

	enable_route_function(pub_interface,private_interface,private_subnet,WIFI_ROUTING);
#endif
}

/*used in p2p+concurrent mode for starting routing*/
static int wifi_start_routing(void *handle){
	SWFEXT_FUNC_BEGIN(handle);

	start_routing();
	
	SWFEXT_FUNC_END();
}
/*used in p2p+concurrent mode for diabling routing*/
static int wifi_disable_routing(void *handle){
	SWFEXT_FUNC_BEGIN(handle);

	disable_route_function(WIFI_ROUTING);
	//wifi_direct_choose_opch_from_apinfo();

	SWFEXT_FUNC_END();
}

static int parse_lease_file(char *lease_file , lease_info *lease){
		
	FILE *fp;
	int file_len=0, item_nu=0,ret;
	int lease_time = 864000;			//must equal the lease_time set in udhcpd_01.conf
	fp = fopen(lease_file,"rb");
	if(fp == NULL){
		printf("open lease file error ! \n");	
		ret = -1;
		goto _end;
	}
	fseek(fp,0,SEEK_END);
	file_len=ftell(fp);

	if(file_len < (sizeof(struct dyn_lease)+8)||(file_len-8)%36 != 0){	//if the len is less then 1 item + 8 Bytes (44 Bytes) ,it means the file is broken
		fclose(fp);			
		printf("the file content error!\n");
		ret = -1;
		goto _end;
	}
	item_nu = (file_len-8)/36;		//get the item number
	fseek(fp,8,SEEK_SET);
	
	if(lease->glease != NULL){
		free(lease->glease);
		lease->glease = NULL;
	}
	lease->glease = (struct dyn_lease *)malloc(item_nu*sizeof(struct dyn_lease));
	if(lease->glease == NULL){
		printf("glease malloc error!\n");
		fclose(fp);
		ret = -1;
		goto _end;
	}	
	lease->item_nu = 0;
	int i;
	for(i=0; i < item_nu; i++){
		memset(&(lease->glease[i]),0,sizeof(struct dyn_lease));
		fread(&(lease->glease[i]),1,sizeof(struct dyn_lease),fp);		//read one item from leases file
		lease->glease[i].expires = ntohl(lease->glease[i].expires);
		if(lease->glease[i].expires == lease_time){			//find the latest item
			memset(lease->latest_hostName,0,sizeof(lease->latest_hostName));
			strcpy(lease->latest_hostName,lease->glease[i].hostname);
			printf("get the latest host name %s\n",lease->latest_hostName);
		}
		lease->item_nu++;
		if(0){
			printf("-----------------------------\n");
			printf("[the %d item]\n",i);
			printf("expires is\t:%u\n",lease->glease[i].expires);
			unsigned char *ip = (unsigned char *)&(lease->glease[i].lease_nip); 
			printf("lease_nip\t:");
			int j;	
			for(j=0; j<3 ; j++)
				printf("%d.",*(ip++));
			printf("%d\nlease_mac\t:",*ip);					
			for(j=0;j<5;j++)
				printf("%02x:",lease->glease[i].lease_mac[j]);
			printf("%02x",lease->glease[i].lease_mac[j]);
			printf("\nhostname\t:%s\n",lease->glease[i].hostname);
			printf("pad is\t:%02x%02x\n",lease->glease[i].pad[0],lease->glease[i].pad[1]);
			printf("-----------------------------\n\n");
		}
	}		
	
	fclose(fp);
	ret = 0;
_end:
	return ret;
}

static int wifi_get_hostname(){
	lease_info *lease = __get_leaseInfo();
	struct stat statinfo;
	char *lease_file = "/tmp/udhcpd.leases";
	int ret;
	
	if(access(lease_file,F_OK)!=0){
		printf("the lease file is unexist!\n");	
		ret = -1;
	}
	else{
		stat(lease_file,&statinfo);		//get the lease_file time
	//	printf("last_chage is %ld,  mtime is %ld \n",lease->last_change,statinfo.st_mtime);
		if(lease->last_change == 0||difftime(statinfo.st_mtime,lease->last_change)>0){
			ret = parse_lease_file(lease_file,lease);	
			printf("last_chage is %ld,  mtime is %ld \n",lease->last_change,statinfo.st_mtime);
			lease->last_change = statinfo.st_mtime;
		}
		else
			ret = 0;
	}

	return ret;
} 

static int wifi_get_latest_hostname(void *handle){
	lease_info *tmp_lease = __get_leaseInfo();

	SWFEXT_FUNC_BEGIN(handle);
	wifi_get_hostname();
	if(tmp_lease->item_nu!= 0 && tmp_lease->latest_hostName != NULL)	
		Swfext_PutString(tmp_lease->latest_hostName);
	else
		Swfext_PutString("");
	SWFEXT_FUNC_END();
}

static int wifi_setAutoConnect(void *handle){
	int val;

	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetNumber();
	setAutoConnEnable(val);
	SWFEXT_FUNC_END();
}

static int wifi_getWifiModelType(void *handle){
	int val = -1;
	SWFEXT_FUNC_BEGIN(handle);
	val = get_wifi_model_type();
	printf("[%s:%d] -- WifiModelType: %d\n", __func__, __LINE__, val);
	Swfext_PutNumber(val);
	SWFEXT_FUNC_END();
}

static int wifi_setChannelMode(void *handle){
	int val;

	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetNumber();
	EZCASTLOG("---- val: 0x%x\n", val);
	set_rtl_band_use(val);
	SWFEXT_FUNC_END();
}
	
static int wifi_getChannelMode(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(get_rtl_band_use());
	SWFEXT_FUNC_END();
}

//for wifi client mode 
#define WIFIINFO_STORE_PATH			"/mnt/vram/ezcast/wifiinfo.bin"
#define WIFI_AUTOMATIC_DISABLE		"/mnt/vram/ezcast/WIFIUTOMATICDISABLE"
typedef struct lanStore_s{
	char ip[16];
	char mask[16];
	char gateway[16];
	char dns1[16];
	char dns2[16];
}wifiStore_t;

enum{
	WLANSETTING_IP = 0,
	WLANSETTING_GATEWAY,
	WLANSETTING_MASK,
	WLANSETTING_DNS1,
	WLANSETTING_DNS2
};
int webset_wifi_setWiFiAutomatic_off(){	//off set wifi wutomatic
	
	int isAutomaticEnable;
	
	if(access(WIFIINFO_STORE_PATH, F_OK) == 0 && access(WIFI_AUTOMATIC_DISABLE, F_OK) != 0){
		FILE *fp = NULL;
		fp = fopen(WIFI_AUTOMATIC_DISABLE, "w");
		//printf("=====creat file===111111111111111111====\n");
		if(fp != NULL)
			fclose(fp);
		}
	sync();
	
	isAutomaticEnable=webset_wifi_getWiFiAutomaticEnable();
	//printf("=====webset_wifi_getWiFiAutomaticEnable===off====%d\n",isAutomaticEnable);
	
	return isAutomaticEnable;
}


int webset_wifi_setWiFiAutomatic_on(){
	
	int isAutomaticEnable;
		if(access(WIFI_AUTOMATIC_DISABLE, F_OK) == 0){
			unlink(WIFI_AUTOMATIC_DISABLE);
            /* Mos: remove all ip file to avoid wifi connection keep in static IP configuration */
            system("rm /mnt/vram/wifi/*.ip");
	//	printf("=====off  file===22222222222222222222222====\n");

		}
	sync();
	
	isAutomaticEnable=webset_wifi_getWiFiAutomaticEnable();
//	printf("=====webset_wifi_getWiFiAutomaticEnable==on===%d\n",isAutomaticEnable);
	
	return isAutomaticEnable;
}




int webset_wifi_getWiFiAutomaticEnable(){
	int isAutomaticEnable = 0;
	if(access(WIFI_AUTOMATIC_DISABLE, F_OK) != 0){  //文件不存在
		isAutomaticEnable = 1;  //auto on
	}
	return isAutomaticEnable;
}

void webset_wifi_DeleteWiFiManualInfo()
{
	if(access(WIFIINFO_STORE_PATH, F_OK) == 0){
		unlink(WIFIINFO_STORE_PATH);
		printf("=====off  file===webset_wifi_DeleteWiFiManualInfo====\n");
	}
	sync();

}

int webset_wifi_storeWiFiManualInfo(char *val){// store webset wifi info

	char setcmd_tmp[256];
	char *ip = NULL;
	char *mask = NULL;
	char *gateway = NULL;
	char *dns1 = NULL;
	char *dns2 = NULL;
	int ret =-1;
	if(val!=NULL)
		sprintf(setcmd_tmp,"%s",val);
	else
		return -1;
	
	printf("=====0000===webset_wifi_storeWiFiManualInfo====%s\n",setcmd_tmp);
	
	ip=strtok(setcmd_tmp,"\n");
	mask=strtok(NULL,"\n");
	gateway=strtok(NULL,"\n");
	dns1=strtok(NULL,"\n");
	dns2=strtok(NULL,"\n");
	
	//printf("=====0000===webset_wifi_storeWiFiManualInfo====%d\n",webset_ip_gateway_match(ip,gateway));
	//printf("=====1111===webset_wifi_storeWiFiManualInfo====%d\n",webset_ip_mask_match_judge(ip,mask));

    if(webset_ip_gw_subnet_check(ip,gateway,mask)!=0)
        return -1;
	if(webDnsCheck(dns1)!=0)
		dns1 = " ";
	if(webDnsCheck(dns2)!=0)
		dns2 = " ";
	wifiStore_t wifiStoreData;
	FILE *fp = NULL;

	memset(wifiStoreData.ip, 0, sizeof(wifiStoreData.ip));
	memset(wifiStoreData.mask, 0, sizeof(wifiStoreData.mask));
	memset(wifiStoreData.gateway, 0, sizeof(wifiStoreData.gateway));
	memset(wifiStoreData.dns1, 0, sizeof(wifiStoreData.dns1));
	memset(wifiStoreData.dns2, 0, sizeof(wifiStoreData.dns2));
	
	snprintf(wifiStoreData.ip, 16, "%s", ip);
	snprintf(wifiStoreData.mask, 16, "%s", mask);
	snprintf(wifiStoreData.gateway, 16, "%s", gateway);
	snprintf(wifiStoreData.dns1, 16, "%s", dns1);
	snprintf(wifiStoreData.dns2, 16, "%s", dns2);

	//printf("=====manual save ok!===webset_wifi_storeWiFiManualInfo====%s\n",wifiStoreData.ip);

	fp = fopen(WIFIINFO_STORE_PATH, "w");
	if(fp != NULL){
		fwrite(&wifiStoreData, sizeof(wifiStore_t), 1, fp);
		fclose(fp);
		sync();
		return 1;
		
/*

	ret = ifconfigConnection(wifiStoreData.ip,wifiStoreData.mask,wifiStoreData.gateway);
	if( ret == 0){
		saveManualip(ip,mask,gateway);
		printf("[][]test\n",__FILE__,__LINE__);
		wlan_status = WIFI_AUTO_IP_SUCCESS;
	}
*/
	}else{
		EZCASTWARN("open [%s] error!!!\n", WIFIINFO_STORE_PATH);
		perror("error");
		return 0;

	}
}


void web_dns_get_from_resolv_file(){
	FILE *fp;
	char callbuf[50]={0};
	char buf[256]={0};
	int ret=-1;
	
	if(access("/mnt/vram/wifi/resolv.conf",F_OK)==-1){
		printf("cann't access to the resolv.conf");
		return ;
	}
	fp = fopen("/mnt/vram/wifi/resolv.conf","r");

	if(fp == NULL){
			printf("read resolv.conf open error!");
			return ;
		}
	ret=fread(buf, 1, 256, fp);

	ret=fclose(fp);
	fprintf(stderr,"function:%s line:%d ret=%d\n",__FUNCTION__,__LINE__,ret);	
	
	
	printf("%s\n",buf);
	char *locate1=strstr(buf,"nameserver ");
	if(locate1==NULL)
		return ;
	locate1+=strlen("nameserver ");
	char *locate1_begin_point=locate1;
	char *locate2=strstr(locate1,"\n");
	if(locate2==NULL)
		return ;
	memset(wifi_first_dns_for_display,0,50);
	memcpy(wifi_first_dns_for_display,locate1_begin_point,strlen(locate1_begin_point)-strlen(locate2));
	fprintf(stderr,"function:%s line:%d first_dns_for_display=%s\n",__FUNCTION__,__LINE__,wifi_first_dns_for_display);	
	char *locate3=strstr(locate2,"nameserver ");
	if(locate3==NULL)
		return ;
	locate3+=strlen("nameserver ");
	char *locate3_begin_point=locate3;
	char *locate4=strstr(locate3,"\n");
	if(locate4==NULL)
		return ;
	memset(wifi_second_dns_for_display,0,50);
	memcpy(wifi_second_dns_for_display,locate3_begin_point,strlen(locate3_begin_point)-strlen(locate4));
	
	fprintf(stderr,"function:%s line:%d second_dns_for_display=%s\n",__FUNCTION__,__LINE__,wifi_second_dns_for_display);	
}


static char *getWiFiIp(){
	char ipTemp[16 + 1] = {0};
	
	memset(ipTemp, 0, 16 + 1);

	getIfconfigInfo("wlan0",IFCONFIG_IP,ipTemp,16 + 1);
	
	if( strlen(ipTemp) > strlen("0.0.0.0") )
		getIfconfigInfo("wlan0",IFCONFIG_IP,wifi_ip_address_for_display,50);
	else
		getIfconfigInfo("wlan2",IFCONFIG_IP,wifi_ip_address_for_display,50);
	
	return wifi_ip_address_for_display;
}


static char *getWiFiMask(){
	char ipTemp[16 + 1] = {0};

	memset(ipTemp, 0, 16 + 1);

	getIfconfigInfo("wlan0",IFCONFIG_IP,ipTemp,16 + 1);

	if( strlen(ipTemp) > strlen("0.0.0.0") )
		getIfconfigInfo("wlan0",IFCONFIG_MASK,wifi_mask_for_display,50);
	else
		getIfconfigInfo("wlan2",IFCONFIG_MASK,wifi_mask_for_display,50);
	return wifi_mask_for_display;
}

static char *getWiFiGateway(){
	char ipTemp[16 + 1] = {0};

	memset(ipTemp, 0, 16 + 1);

	getIfconfigInfo("wlan0",IFCONFIG_IP,ipTemp,16 + 1);
	memset(wifi_gateway_for_display,0,50);

	if( strlen(ipTemp) > strlen("0.0.0.0") )
		__get_kernel_gateway_info(wifi_gateway_for_display,"wlan0");
	else
		__get_kernel_gateway_info(wifi_gateway_for_display,"wlan2");

	return wifi_gateway_for_display;
}

int getWiFiSettingVal(int flag){
	int ret = -1;
	char *val = NULL;
	static wifiStore_t wifiStoreData;
	int isInfoStore = 0;
	if(access(WIFIINFO_STORE_PATH, F_OK) == 0){
		FILE *fp = fopen(WIFIINFO_STORE_PATH, "r");
		if(fp != NULL){
			ret = fread(&wifiStoreData, sizeof(wifiStore_t), 1, fp);
			if(ret > 0){
				isInfoStore = 1;
				printf("isInfoStore=%d\n",isInfoStore);
			}
			fclose(fp);
		}
	}
	switch(flag){
		case WLANSETTING_IP:
			if(isInfoStore == 1)
				val = wifiStoreData.ip;
			else
				val = getWiFiIp();
			break;
		case WLANSETTING_GATEWAY:
			if(isInfoStore == 1)
				val = wifiStoreData.gateway;
			else
				val = getWiFiGateway();
			break;
		case WLANSETTING_MASK:
			if(isInfoStore == 1)
				val = wifiStoreData.mask;
			else
				val = getWiFiMask();
			break;
		case WLANSETTING_DNS1:
			if(isInfoStore == 1)
				val = wifiStoreData.dns1;
			else{
				web_dns_get_from_resolv_file();
				val = wifi_first_dns_for_display;
			}
			break;
		case WLANSETTING_DNS2:
			if(isInfoStore == 1)
				val = wifiStoreData.dns2;
			else{
				web_dns_get_from_resolv_file();
				val = wifi_second_dns_for_display;
			}
			break;
		default:
			break;
	}
	printf("getWiFiSettingVal%d=%s\n",flag,val);
	return val;
}


int webset_getWiFiSettingautoVal(int flag){
	char *val = "Auto";
	int wlan_status=ProboxGetConnectStatus();
	if (wlan_status!=0)
	{
		switch(flag){
			case WLANSETTING_IP:
					val = getWiFiIp();
				break;
			case WLANSETTING_GATEWAY:
					val = getWiFiGateway();
				break;
			case WLANSETTING_MASK:
					val = getWiFiMask();
				break;
			case WLANSETTING_DNS1:
					web_dns_get_from_resolv_file();
					val = wifi_first_dns_for_display;
				break;
			case WLANSETTING_DNS2:
					web_dns_get_from_resolv_file();
					val = wifi_second_dns_for_display;
				break;
			default:
				break;
		}
	}
	return val;
}


/* 
 * Set the hidden ap's ssid, then it can be scanned out by WPAS. 
 */
int extension_scanning_set(char *ifname,char *essid){

	int skfd;			/* generic raw socket desc.	*/	

	if(ifname == NULL){
		printf("need a interface!\n");
		return -1;
	}
	if(essid == NULL){
		printf("need a essid!\n");
		return -1;
	}
	
	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0)
	{
		perror("socket");
		return -1;
	}
	
	struct iwreq		wrq;
	struct iw_scan_req	scanopt;		/* Options for 'set' */
	int scanflags = 0;				/* Flags for scan */
	struct iw_range range;
	int has_range;

	/* Get range stuff */
	has_range = (iw_get_range_info(skfd, ifname, &range) >= 0);

	/* Check if the interface could support scanning. */
	if((!has_range) || (range.we_version_compiled < 14)){
		fprintf(stderr, "%-8.16s	Interface doesn't support scanning.\n\n",ifname);
		return(-1);
	}

	/* Clean up set args */
	memset(&scanopt, 0, sizeof(scanopt));

	/* Store the ESSID in the scan options */
	scanopt.essid_len = strlen(essid);
	memcpy(scanopt.essid, essid, scanopt.essid_len);
	/* Initialise BSSID as needed */
	if(scanopt.bssid.sa_family == 0){
		scanopt.bssid.sa_family = ARPHRD_ETHER;
		memset(scanopt.bssid.sa_data, 0xff, ETH_ALEN);
	}
	/* Scan only this ESSID */
	scanflags |= IW_SCAN_THIS_ESSID;

	/* Check if we have scan options */
	if(scanflags){
		wrq.u.data.pointer = (caddr_t) &scanopt;
		wrq.u.data.length = sizeof(scanopt);
		wrq.u.data.flags = scanflags;
	}
	else{
		wrq.u.data.pointer = NULL;
		wrq.u.data.flags = 0;
		wrq.u.data.length = 0;
	}

	/* Initiate Scanning */
	if(iw_set_ext(skfd, ifname, SIOCSIWSCAN, &wrq) < 0){
		if((errno != EPERM) || (scanflags != 0)){
			fprintf(stderr, "%-8.16s	Interface doesn't support scanning : %s\n\n",ifname, strerror(errno));
			return(-1);
		}

		/* If we don't have the permission to initiate the scan, we may
		   * still have permission to read left-over results.
		   * But, don't wait !!! */

		/* Not cool, it display for non wireless interfaces... */
		fprintf(stderr, "%-8.16s	(Could not trigger scanning, just reading left-over results)\n", ifname);
	}

 	/* Close the socket. */
	iw_sockets_close(skfd);

	return 0;
}



int swfext_wifi_register(void)
{
       
	SWFEXT_REGISTER("wifi_Open", wifi_open);
	SWFEXT_REGISTER("wifi_Close", wifi_close);

	SWFEXT_REGISTER("wifi_Start", wifi_start);
	SWFEXT_REGISTER("wifi_StartScan", wifi_startscan);
	SWFEXT_REGISTER("wifi_GetScanResults", wifi_getscanresults);
	SWFEXT_REGISTER("wifi_Stop", wifi_stop);
	SWFEXT_REGISTER("wifi_AddNetwork", wifi_addnetwork);
	SWFEXT_REGISTER("wifi_GetAPSsid", wifi_getssid);
	SWFEXT_REGISTER("wifi_SetAPSsid", wifi_setssid);
	SWFEXT_REGISTER("wifi_GetSingal", wifi_getsingal);
	SWFEXT_REGISTER("wifi_GetConfExist", wifi_getconfexist);
	SWFEXT_REGISTER("wifi_SetWEPKeyIndex", wifi_setwepkeyindex);
	SWFEXT_REGISTER("wifi_SetPassword", wifi_setpassword);
	SWFEXT_REGISTER("wifi_SaveConf", wifi_saveconf);
	SWFEXT_REGISTER("wifi_SaveaddedAPConf", wifi_saveaddedAPconf);		//save conf for added ap
	
	SWFEXT_REGISTER("wifi_ConnectAP", wifi_connectAP);
	SWFEXT_REGISTER("wifi_GetAuthenType", wifi_getAuthenType);
	SWFEXT_REGISTER("wifi_SetAuthenType", wifi_setAuthenType);
	SWFEXT_REGISTER("wifi_GetStatus", wifi_getStatus);
	SWFEXT_REGISTER("wifi_SetDefault", wifi_setdefault);
	SWFEXT_REGISTER("wifi_GetIndexofSelectAP", wifi_getindexofselectAP);
	SWFEXT_REGISTER("wifi_SetIPAddress", wifi_setIP);
	SWFEXT_REGISTER("wifi_GetIPAddress", wifi_getIP);
	SWFEXT_REGISTER("wifi_SetRouter", wifi_setrouter);
	SWFEXT_REGISTER("wifi_GetRouter", wifi_getrouter);
	SWFEXT_REGISTER("wifi_SetDNS", wifi_setDNS);
	SWFEXT_REGISTER("wifi_GetDNS", wifi_getDNS);
	SWFEXT_REGISTER("wifi_get_signal_level_as", wifi_get_signal_level);
	SWFEXT_REGISTER("wifi_DeleteConf", wifi_deleteconf);
	SWFEXT_REGISTER("wifi_Autoconnect", wifi_autoconnect);
	SWFEXT_REGISTER("wifi_setAutoConnect", wifi_setAutoConnect);
	SWFEXT_REGISTER("wifi_donglemodechange", wifi_dongle_mode_change);
	SWFEXT_REGISTER("wifi_getsoftapssid", wifi_get_softap_ssid);
	SWFEXT_REGISTER("wifi_getsoftapssid_Probox24g", wifi_get_softap_ssid_Probox24g);
	SWFEXT_REGISTER("wifi_getsoftappsk", wifi_get_softap_psk);
	SWFEXT_REGISTER("wifi_getsoftapmode", wifi_get_softap_mode);
	SWFEXT_REGISTER("wifi_getsoftapchannel", wifi_get_softap_channel);
	SWFEXT_REGISTER("wifi_getsoftapconfiginfo", wifi_get_softap_config_info);
	SWFEXT_REGISTER("wifi_getsoftapinputconfiginfo", wifi_get_softap_input_config_info);
	SWFEXT_REGISTER("wifi_getbestchannel", wifi_get_best_channel);
	SWFEXT_REGISTER("wifi_remotecontrol", wifi_remote_control);
	SWFEXT_REGISTER("wifi_getdongletype", wifi_get_dongle_type);
	SWFEXT_REGISTER("wifi_getpasswordfordisplay", wifi_getpassword_for_display);
	SWFEXT_REGISTER("wifi_getconnectingssid", wifi_get_connecting_ssid);
	SWFEXT_REGISTER("wifi_getconnectedssid", wifi_get_connected_ssid);
	SWFEXT_REGISTER("wifi_putautochannelchoose", wifi_auto_channel_choose_put);
	SWFEXT_REGISTER("wifi_getautochannelchoose", wifi_auto_channel_choose_get);
	SWFEXT_REGISTER("wifi_checkwifidisplayvalid", wifi_check_wifidisplay_valid);
	SWFEXT_REGISTER("wifi_restartapmode", wifi_restart_ap_mode);
	SWFEXT_REGISTER("wifi_ezremoteflag", wifi_ez_remote_flag);
	
	SWFEXT_REGISTER("wifi_wpsbtn_connect", wifi_wps_pbutton_connect);
	SWFEXT_REGISTER("wifi_wpspcode_connect", wifi_wps_pincode_connect);
	
	SWFEXT_REGISTER("wifi_exchangeap", wifi_exchange_ap);
	SWFEXT_REGISTER("wifi_disconnect", wifi_disconnect_ap);
	SWFEXT_REGISTER("wifi_countAccessDevices", wifi_count_accessDevices);
	SWFEXT_REGISTER("wifi_start_softap", wifi_softap_start_from_miracast);
	SWFEXT_REGISTER("wifi_set_ChannelRegion", wifi_set_channel_region);
	SWFEXT_REGISTER("wifi_get_ChannelRegion", wifiGetChannelRegion);

	SWFEXT_REGISTER("wifi_openmanual", wifi_open_manual);
	SWFEXT_REGISTER("wifi_ipmaskmatchjudge", wifi_ip_mask_match_judge);
	SWFEXT_REGISTER("wifi_ipgatewaymatch",  wifi_ip_gateway_match);
	SWFEXT_REGISTER("wifi_Probox_rebridge",  wifi_Probox_rebridge);
	
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	SWFEXT_REGISTER("hostcontrol_user_clean", hostcontrol_user_clean);
#endif
	SWFEXT_REGISTER("wifi_feedbackcast", send_msg_to_app);
	SWFEXT_REGISTER("wifi_psk_changed", wifi_psk_changed);
	SWFEXT_REGISTER("wifi_isLastRouterApExist", wifi_isLastRouterApExist);
	SWFEXT_REGISTER("wifi_softapIgnore", wifi_softapIgnore);
	SWFEXT_REGISTER("wifi_softapShow", wifi_softapShow);
	SWFEXT_REGISTER("wifi_get_connect_mac_address", wifi_get_connect_mac_address);
	SWFEXT_REGISTER("wifi_get_connection_status", wifi_get_connection_status);
	SWFEXT_REGISTER("set_Airplay_Id", set_Airplay_Id);
	SWFEXT_REGISTER("set_DLNA_Id", set_DLNA_Id);
	SWFEXT_REGISTER("wifi_get_hotspot_status", wifi_get_hotspot_status);
	SWFEXT_REGISTER("wifi_getWifiModelType", wifi_getWifiModelType);
	SWFEXT_REGISTER("wifi_setChannelMode", wifi_setChannelMode);
	SWFEXT_REGISTER("wifi_getChannelMode", wifi_getChannelMode);
	SWFEXT_REGISTER("wifi_get_cur_channel",  wifi_get_cur_channel);
	SWFEXT_REGISTER("wifi_get_default_psk", wifi_get_default_psk);
#endif
#if EZCAST_LITE_ENABLE
	SWFEXT_REGISTER("wifi_web_connect", wifi_web_connect);
#endif

	SWFEXT_REGISTER("wifi_DisableRouting",wifi_disable_routing);
	SWFEXT_REGISTER("wifi_StartRouting",  wifi_start_routing);
	SWFEXT_REGISTER("wifi_getLatestHostName",wifi_get_latest_hostname);
	return 0;
}
#else

int swfext_wifi_register(void)
{
	return 0;
}

#endif	/** MODULE_CONFIG_NETWORK */
