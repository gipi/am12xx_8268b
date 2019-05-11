#ifdef MODULE_CONFIG_NETWORK
#include <unistd.h>
#include "file_list.h"
#include "wifi_engine.h"
#include "filelist_engine.h"
#include <sys/wait.h>
#include <sys/types.h>
#include "wifi_remote_control.h"
#include "system_info.h"
#include "sys_vram.h"
#include "apps_vram.h"
#include "sys_gpio.h"
#include "ezcast_public.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "websetting.h"
extern struct wifi_info_t *__get_wifiinfo();
extern int wifiChannel_mode;
extern int wifiChannel_set;
#if WEBSETTING_ENABLE
extern void create_websetting_server();
#endif
static char* __get_conf_filename(int sel_ap);
static int CheckAuthenType(int sel_ap);

#define WPA_CLI_SO			4
#define WPA_CLI_TEST		0
#define DONGLE_IN 	1
#define DONGLE_OUT 	2

#define WPA_CLI_RETURN		"wpa_cli    /tmp/wpa_supplicant"

#if 0
	#define wifidbg_info(fmt, arg...) printf("WIFIINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define wifidbg_info(fmt, arg...)	//printf("WIFIINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#endif
#define debug_info(fmt,arg...)	printf("[debug info :%s,%d]:"fmt"\n",__FUNCTION__,__LINE__,##arg)

wifi_info wifi_s;

/****connecting ap info*****/
int selected_ap;
char selected_ssid[MAX_SSID_LEN];
char selected_filename[WIFI_BUF_LEN]="/etc/wireless-tkip.conf";
char temp_filename[WIFI_BUF_LEN];
char selectedIPfile[64];			//store the manual ip mask gateway
char *latestAPconf = "/mnt/vram/wifi/latestAP.conf";
char *latestAPconf_bak = "/mnt/user1/softap/latestAP.conf";

/****connecting ap info*****/
int wps_connect_flag=0;
int autoConnectFlag=0;			//auto connect flag
int autoConnectEnable = 1;
int connect_sem_flag=0;		//for block WIFI_CMD_CONNECT & selected_filename
status_enum wlan_status = WIFI_DISCONNECTED;
current_status wlan_current_status;
static int is_dns_enable = 0;
//extern int cur_port;
extern int vid_pid_from_driver_firstin;
extern int wpa24g_connected;
extern int wpa5g_connected;
int wifi_client_auto=0;//0 off,1 on
int rtk8189es_enable=0;

char ctrl_iface[128];

struct wpa_ctrl *ctrl_conn;
char ctrl_connect_opened=0;

static wifi_ioctrl wifi_cmd_array[WIFI_IOARRAY_LEN];
static unsigned long wifi_timestamp=1;
int wifi_remote_control_started=0;
int auto_route_has_get=0;
int websetting_set_internet_access=0;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
#include "customer_ssid.h"
extern int bestChannelChosen;
int concurrent_ch_record=0;
#endif

#if ROUTER_ONLY_ENABLE
int routerOnly = 0;
int getRouterOnlyStatus(){
	return routerOnly;
}
void setRouterOnlyStatus(int val){
	routerOnly = val;
}
#endif

static int vf_system(const char *cmdstring){
	pid_t pid;
	int status;
	//printf("____now in the vf system______\n");
	if(cmdstring == NULL)
		return 1;
	if((pid = vfork()) < 0){
		status = -1;
	}
	else if(pid == 0){			//child process
		execl("/bin/sh","sh","-c",cmdstring,(char *)0);
		_exit(127);
	}
	else {
		while(waitpid(pid,&status,0) < 0){
			if(errno != EINTR){
				status = -1;
				break;
			}
		}
	}
	return(status);
}

typedef void (*sighandler_t)(int);

int pac_system(const char *cmd_in){
	int ret;
	sighandler_t old_handler;
	printf("[%s]___pac system_________: %s\n",__FILE__,cmd_in);
	old_handler = signal(SIGCHLD,SIG_DFL);
	ret = vf_system(cmd_in);
	signal(SIGCHLD,old_handler);
	if(ret != 0)
		printf("_____the error no. is %d_______\n",errno);
	return ret;
}


int APHash (char *str)
{
   // printf("go to point 3\n");	
    unsigned int hash = 0;
    int i;
    for (i = 0; *str; i++)
      {
	  if ((i & 1) == 0)
	    {
		hash ^= ((hash << 7) ^(*str++) ^ (hash >> 3));
	    }
	  else
	    {
		hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
	    }
      }
   // printf("go to point 4\n");
    return (hash & 0x7FFFFFFF);
}
extern int wifi_client_thread_is_running ;

int init_wifi_wlan_current_status(){
	memset(&wlan_current_status,0,sizeof(wlan_current_status));
	wlan_current_status.current_status_value = -1;
	return 0;
}
void  wifi_sem_wait(sem_t *sem)
{
	int err;
	int times=0;
		
	if(read_wifi_mode() == WIFI_DONGLE_PLUG_OUT)
		return ;
WIFI_PEND_REWAIT:
	err = sem_wait(sem);
	//printf("err==================%d\n",err);
	if(err == -1){
		int errsv = errno;
		
		printf("errsv==================%d\n",errsv);
		if(errsv == EINTR){
			if(times<20){
				sleep(2);
				times++;
				fprintf(stderr,"function:%s line:%d times:%d\n",__FUNCTION__,__LINE__,times);
				goto WIFI_PEND_REWAIT;
			}
			else 
				return;
		}
		else{
			wifidbg_err("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}
	return ;
}

void  wifi_sem_post(sem_t *sem)
{
	int err;
	err = sem_post(sem);
	return;
}


static int conf_name_transition( int sel_ap)
{
	int filename_int;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	char *filename_temp=wifi_info->ap_entry[sel_ap].ssid;
	filename_int=APHash(filename_temp);
	return filename_int;
}


int wifi_create_dir(char * dir_path)
{
	int rtn=0;
	rtn = access(dir_path,W_OK);
	if(rtn!=0){
		rtn = mkdir(dir_path,0777);
		if(rtn!=0){
			wifidbg_err("Make Dir Failed=%s\n",dir_path);
		}
		//system("sync");
	}
	return rtn;	
}


extern int vid_pid_from_driver;
int wlan_up(char * net_interface)
{
	char callbuf[WIFI_BUF_LEN]={0};
/*	if(access("/sys/module/rt5370sta",F_OK)==0){
		int i=10000;
		while(i--)
			;
	}*/
	sprintf(callbuf,"ifconfig %s 0.0.0.0 up",net_interface);
	printf("the call is %s\n",callbuf);
	pac_system(callbuf);

	sprintf(callbuf,"ifconfig lo up");
	printf("the call is %s\n",callbuf);
	pac_system(callbuf);
	
	return 0;	
}

int wlan_down(char * net_interface)
{
	char callbuf[WIFI_BUF_LEN]={0};
/*	if(access("/sys/module/rt5370sta",F_OK)==0){
		int i=10000;
		while(i--)
			;
	}*/
	sprintf(callbuf,"ifconfig %s down",net_interface);
	printf("the call is %s\n",callbuf);
	pac_system(callbuf);
	return 0;
	
}
char * get_modify_line_realtek(char *filepath, char * key)
{
	static char tmp1[256];
	char * locate1 = NULL;
	int key_len=0;

	FILE *fp = NULL;
	char buf[4096] ={0};
	int ret=-1;
	fp = fopen(filepath,"r");
	if(fp == NULL){
		return NULL;
	}
	ret=fread(buf, 1, 4096, fp);
	ret=fclose(fp);
	if(buf==NULL){
		return NULL;
	}
	
	key_len=strlen(key);
	locate1 = strstr(buf,key);
	if(locate1==NULL) return NULL;
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	memset(tmp1,0,256);
	memcpy(tmp1,(locate1+key_len),locate2-locate1-key_len);
	//tmp2=tmp1;
	//printf("[%s][%d]<%s> %s=%s",__func__,__LINE__,filepath,key,tmp1);
	return tmp1;
}

int udhcpd_dns_check(){
	char buf[32];
	char udh_conf[64]="/tmp/udhcpd_01.conf";
	char udh_result[64]="/tmp/udhcpd_check_result";
	char udh_sh[64]="/am7x/case/scripts/dns_check.sh";
	char tmp_udh_sh[64]="";
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	if(vid_pid_from_driver==0x148f8189){
		sprintf(udh_conf,"/tmp/udhcpd_02.conf");
		sprintf(udh_result,"/tmp/udhcpd_check_result_02");
		sprintf(udh_conf,"/am7x/case/scripts/dns_check02.sh");
	}
#endif
	if((access(udh_conf, F_OK) == 0)){
		if(access(udh_result, F_OK) == 0){
			unlink(udh_result);
		}
		sprintf(tmp_udh_sh,"sh %s",udh_sh);
		system(tmp_udh_sh);
		if(access(udh_result, F_OK) == 0){
			unlink(udh_result);
			return 0;
		}
		return 1;
	}

	return -1;
}

void restart_udhcpd_without_dns(){
	printf("[%s][%d]\n",__func__,__LINE__);
	#if MODULE_CONFIG_EZWILAN_ENABLE		// Do not disable dns for EZCastLAN
		return;
	#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	char udh_conf[64]="/etc/udhcpdbr1_nodns.conf"; 
#else
	char udh_conf[64]="/etc/udhcpd_nodns_01.conf";
#endif
	char udhtmp_conf[64]="/tmp/udhcpd_01.conf";	
	char cmd[128]="";
	int ret = udhcpd_dns_check();
	sprintf(cmd,"ln -sf %s %s", udh_conf,udhtmp_conf);
	if(ret == 0 && access(udh_conf, F_OK) == 0){
		printf("access %s ok\n", udh_conf);
		system(cmd);
		ret = udhcpd_dns_check();
		if(ret == 0){								// Try 3 times if fail;
			EZCASTLOG("Try again!!!\n");
			usleep(100000);
			system(cmd);
			ret = udhcpd_dns_check();
			if(ret == 0){
				EZCASTLOG("Try again!!!\n");
				usleep(100000);
				system(cmd);
				ret = udhcpd_dns_check();
				if(ret == 0){
					EZCASTLOG("Link udhcpd_nodns_01.conf fail!!!\n");
					goto __END__;
				}
			}
		}
		is_dns_enable = 0;
__END__:
	EZCASTLOG("Restart udhcpd!!\n");
	pac_system("killall -9 udhcpd");
	pac_system("udhcpd /tmp/udhcpd_01.conf");
	}
}

void restart_udhcpd_with_dns(){
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	char udh_conf[64]="/etc/udhcpdbr1.conf"; 
#else
	char udh_conf[64]="/etc/udhcpd_01.conf";
#endif
	char udhtmp_conf[64]="/tmp/udhcpd_01.conf"; 
	char cmd[128]="";
	int ret = udhcpd_dns_check();
	sprintf(cmd,"ln -sf %s %s",udh_conf,udhtmp_conf);
	if(ret != 0 && access(udh_conf, F_OK) == 0){
		printf("access %s ok\n", udh_conf);
		system(cmd);
		ret = udhcpd_dns_check();
		if(ret != 0){								// Try 3 times if fail;
			EZCASTLOG("Try again!!!\n");
			usleep(100000);
			system(cmd);
			ret = udhcpd_dns_check();
			if(ret != 0){
				EZCASTLOG("Try again!!!\n");
				usleep(100000);
				system(cmd);
				ret = udhcpd_dns_check();
				if(ret != 0){
					EZCASTLOG("Link %s fail!!!\n",udh_conf);
					goto __END__;
				}
			}
		}
		is_dns_enable = 1;
__END__:
	EZCASTLOG("Restart udhcpd!!\n");
	pac_system("killall -9 udhcpd");
	pac_system("udhcpd /tmp/udhcpd_01.conf");
	}
}

void udhcpd_dns_enable(){
#if AUTO_DNS_ENABLE
	if(is_dns_enable == 0){
		struct sysconf_param sys_info;
		
		_get_env_data(&sys_info);
		if(sys_info.last_ui != EZMIRRORLITE_ITEM){
			restart_udhcpd_with_dns();
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

			printf("[%s][%d]\n",__func__,__LINE__);
		if(vid_pid_from_driver==0x148f8189)
		
			printf("killall hostapd_02");
			//system("killall hostapd_02\n");
			//connect 24g  not kill 5g,  
		else{
			printf("udhcpd_dns_enable  killall hostapd\n");
			//connect 5g will kill 24g  need to restarts 24g
			//system("killall hostapd");

			//system("hostapd_02 -B /etc/rtl_hostapd_02.conf");
			}
#else  
		system("killall hostapd");
		realtek_softap_func(DONGLE_IN, NULL);
#endif	
		printf("udhcpd_dns_enable  call realtek_softap_func \n");
	//	realtek_softap_func(DONGLE_IN, NULL);
			//printf("killall hostapd\n");
		}
	}
#endif
}

void udhcpd_dns_disable(){
#if AUTO_DNS_ENABLE
	if(is_dns_enable != 0){
		struct sysconf_param sys_info;
		
		_get_env_data(&sys_info);
		if(sys_info.last_ui != EZMIRRORLITE_ITEM){
			is_dns_enable = 0;
			restart_udhcpd_without_dns();
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

			printf("[%s][%d]\n",__func__,__LINE__);
			if(vid_pid_from_driver==0x148f8189)
				system("killall hostapd_02");
				//printf("killall hostapd_02\n");
			else{
				system("killall hostapd");
				printf("killall hostapd\n");
				system("hostapd_02 -B /etc/rtl_hostapd_02.conf");
			}
#else
			system("killall hostapd");
			realtek_softap_func(DONGLE_IN, NULL);
#endif
		//	realtek_softap_func(DONGLE_IN, NULL);
		}
	}
#endif
}

int kill_wpa_supplicant()
{
	char callbuf[WIFI_BUF_LEN]={0};
	sprintf(callbuf,"killall udhcpc");
	wifidbg_info("the call is %s",callbuf);
	pac_system(callbuf);

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(vid_pid_from_driver==0x148f8189){
		sprintf(callbuf,"killall wpa2_supplicant");
	}else{
		sprintf(callbuf,"killall wpa_supplicant");
	}

#else
	sprintf(callbuf,"killall wpa_supplicant");
#endif
	wifidbg_info("the call is %s",callbuf);
	pac_system(callbuf);
	
#if 0
	if(dongle_get_ctrl_iface(ctrl_iface)==0){
		while(access(ctrl_iface,F_OK)!=-1)
			system(callbuf);
	}
#endif
	return 0;
}

int wlan_conf(char * net_interface, char * conf_path)
{
	FILE *fp;
	char callbuf[WIFI_BUF_LEN]={0};

	printf("net_interface=========%s\n",net_interface);
	printf("conf_path=========%s\n",conf_path);
	/*if(access(conf_path,F_OK)!=0){
		printf("the conf_path is not exsit\n");
		return -1;
	}*/
	if(strncmp(conf_path,"/etc/wireless-tkip.conf",strlen("/etc/wireless-tkip.conf"))){
		if((fp=fopen(conf_path,"r")) != NULL){
			if(fgetc(fp) == EOF){ 
				fclose(fp);
				printf("the conf is empty!!\n");
				remove(conf_path);
				system("sync");
				return -1;
				}
			else 
				fclose(fp);
		}
		else
			{
			printf("the conf_path is not exsit\n");
			return -1;
		}
	}
	memset(callbuf,0,sizeof(callbuf));
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE
	wifiBridgeProcess(WIFI_BRIDGE_ADD_BR0);
	sprintf(callbuf,"wpa_supplicant -B -i wlan0 -c %s -b br0",conf_path);
#else
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(!strcmp("wlan2",net_interface))
		sprintf(callbuf,"wpa2_supplicant -B -i %s -c %s",net_interface,conf_path);
	else 
		sprintf(callbuf,"wpa_supplicant -B -i %s -c %s",net_interface,conf_path);
#else
	sprintf(callbuf,"wpa_supplicant -B -i %s -c %s",net_interface,conf_path);

#endif
	printf("--wlan_conf callbuf=%s",callbuf);
#endif
	memcpy(selected_filename,conf_path,WIFI_BUF_LEN);
	utime(conf_path,NULL);
	pac_system(callbuf);
	return 0;
}
struct wpa_ctrl *ctrl_conn_scan;

int openCtrlConnection()
{
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	static char semi_ctrl_operation_has_init=0;
#if 0	
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	printf("semi_ctrl_operation_has_init========%d\n",semi_ctrl_operation_has_init);
	if(semi_ctrl_operation_has_init==0){
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
		if(sem_init(&wifi_info->syn_lock.semi_ctrl_operation,0,1)==-1){
			sem_destroy(&wifi_info->syn_lock.semi_ctrl_operation);
			wifidbg_err("Sem init error");
			return -1;
		}
		semi_ctrl_operation_has_init=1;
	}
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
#endif
	if(ctrl_connect_opened==1){
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
		closeCtrlConnection();
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
	}
	if(dongle_get_ctrl_iface(ctrl_iface)==-1)
		return -1;
	
	printf("%s,%d ctrl_iface=[%s]\n",__FUNCTION__,__LINE__,ctrl_iface);
	//wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	ctrl_conn = (struct wpa_ctrl *)wpa_ctrl_open(ctrl_iface);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	//wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(ctrl_conn==NULL){
		wifidbg_err("open ctrl interface failed!");
		return -1;
	}
	ctrl_connect_opened=1;

	return 0;
}

int closeCtrlConnection()
{
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(ctrl_connect_opened==1){
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
		wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
		wpa_ctrl_close(ctrl_conn);
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
		wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
		ctrl_connect_opened=0;
	}
	return 0;
}

int ctrlRequest(const char *cmd, char *buf, size_t *buflen)
{
	int ret;
	char *device=NULL;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	if (ctrl_connect_opened == 0)
		return -3;
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(!strcmp("SCAN",cmd)||!strcmp("SCAN_RESULTS",cmd)){ //use 5G to scan
		if(strcmp(ctrl_iface,"/tmp/wpa_supplicant/wlan0")){
			
			if(access("/tmp/wpa_supplicant/wlan0",0)!=0){
				wifidbg_err("--ProBox open 5G_wlan0 to scan wifi list--\n");
				pac_system("wpa_supplicant -B -i wlan0 -c /etc/wireless-tkip.conf");
				wifidbg_err("Probox dongle_get_ctrl_iface wlan0 fail!!!\n");
				//return -1;
			}
			ctrl_conn_scan = (struct wpa_ctrl *)wpa_ctrl_open("/tmp/wpa_supplicant/wlan0");
			ret = wpa_ctrl_request(ctrl_conn_scan, cmd, strlen(cmd), buf, buflen, NULL);
		}else
			{
			wifidbg_err("--ProBox use 5G_wlan0 to scan wifi list--\n");
			ret = wpa_ctrl_request(ctrl_conn, cmd, strlen(cmd), buf, buflen, NULL);

		}
	}
	else
		{
		ret = wpa_ctrl_request(ctrl_conn, cmd, strlen(cmd), buf, buflen, NULL);

	}

#else
		ret = wpa_ctrl_request(ctrl_conn, cmd, strlen(cmd), buf, buflen, NULL);

	
#endif
	
	
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	if (ret == -2)
		wifidbg_err("'%s' command timed out.\n", cmd);
	else if (ret < 0)
		wifidbg_err("'%s' command failed.\n", cmd);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);

	//wifidbg_info("'%s' command results is %s,ret is %d.\n", cmd,buf,ret);

	return ret;
}



static int analyze_scan(char * buf, AP_info * ap_entry_tmp, int index)
{
	char *pos=NULL,*tmpbuf=NULL;
	char tmp[50]={0};
	char *locate="\t";
	//wifidbg_info("%s:num is %d",buf,AP_num);
	if(strstr(buf,"<hidden>")){
		wifidbg_info("this is a hidden ap");
		return -1;
	}
	if(access("/mnt/user1/softap/rtl_hostapd_02.conf",0)==0)
	{
		//printf("Probox shield selt ssid ");
		char *tmp_24gssid=get_modify_line_realtek("/mnt/user1/softap/rtl_hostapd_02.conf","ssid=");
		//printf("tmp_24gssid=%s\n ,buf=%s\n ",tmp_24gssid,buf);
		if(strstr(buf,tmp_24gssid)){
		//	printf("this is a Probox self hostap!!!\n");
			return -1;
		}
	}
	pos = buf;
	tmpbuf = strstr(pos,locate);	
	if(tmpbuf==NULL)
		return 0;
	memset(tmp,0,sizeof(tmp));
	strncpy(ap_entry_tmp[index].mac_address, pos, strlen(pos)-strlen(tmpbuf));
	//wifidbg_info("%s",AP_Info[AP_num].mac_address);
	pos=tmpbuf;

	pos++;
	tmpbuf = strstr(pos,locate);
	//wifidbg_info("%d",strlen(pos)-strlen(tmpbuf));
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp, pos, strlen(pos)-strlen(tmpbuf));
	//wifidbg_info("%s",tmp);
	ap_entry_tmp[index].frequency = atoi(tmp);
	pos=tmpbuf;

	pos++;
	tmpbuf = strstr(pos,locate);	
	//wifidbg_info("%d",strlen(pos)-strlen(tmpbuf));
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp, pos, strlen(pos)-strlen(tmpbuf));
	//wifidbg_info("%s",tmp);
	ap_entry_tmp[index].singal_level = atoi(tmp);
	if(ap_entry_tmp[index].singal_level<0)
		ap_entry_tmp[index].singal_level=0-ap_entry_tmp[index].singal_level;
	pos=tmpbuf;

	pos++;
	tmpbuf = strstr(pos,locate);	
	//wifidbg_info("%d",strlen(pos)-strlen(tmpbuf));
	memset(tmp,0,sizeof(tmp));
	strncpy(tmp, pos, strlen(pos)-strlen(tmpbuf));
	//wifidbg_info("%s",tmp);
	if(strstr(tmp,"WPA"))
	{
		ap_entry_tmp[index].authen = WPA;
	}
	else if(strstr(tmp,"WEP"))
	{
		ap_entry_tmp[index].authen = WEP;
	}
	else
		ap_entry_tmp[index].authen = OPEN;
	//wifidbg_info("%d",wifi_s.ap_entry[index].authen);
	pos=tmpbuf;

	//wifidbg_info("%s",pos);
	pos++;
	memset(ap_entry_tmp[index].ssid,0,sizeof(ap_entry_tmp[index].ssid));
	if(strlen(pos)==0){
		wifidbg_info("pos is null");
		return -1;
	}
	strncpy(ap_entry_tmp[index].ssid, pos, strlen(pos));

	ap_entry_tmp[index].isvalid = 1;

	return 0;
}

static int check_conf_exist(char * net_ssid)
{
	char conf_file[256] = {0};
	memset(conf_file,0,strlen(conf_file));
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		if(vid_pid_from_driver==0x148f8189)
			sprintf(conf_file,"/mnt/vram/wifi/24G/%x.conf",APHash(net_ssid));
		else
			sprintf(conf_file,"/mnt/vram/wifi/5G/%x.conf",APHash(net_ssid));
	#else
	sprintf(conf_file,"/mnt/vram/wifi/%x.conf",APHash(net_ssid));
	#endif
	if(access(conf_file,F_OK)!=-1)
	{
		wifidbg_info("%d is exist",conf_file);
		return 1;
	}
	else
	{
		wifidbg_info("%s is not exist",conf_file);
		return 0;
	}
}


static void dump_ap_entry(struct wifi_info_t * wifi_info)
{
	int i=0;
	for(i=0;i<wifi_info->AP_num;i++){
		//printf("[%d]%s\t%d\t%d\t%s\n",i,wifi_info->ap_entry[i].mac_address,wifi_info->ap_entry[i].frequency,wifi_info->ap_entry[i].singal_level,wifi_info->ap_entry[i].ssid);
	}
}

/**
*if more than two APs have the same ssid,then filter these APs and leave the AP which has the strongest signal.
*/
static void filter_sameSsid(int *apNum , AP_info *ap_entry_tmp)
{
	int ap_num = *apNum;
	int i,j;
	//printf("[debug filter:%s,%d]\n",__FUNCTION__,__LINE__);
	if(ap_num == 0) return;

	for(i = 0;i < ap_num-1;i++)
		for(j = i+1;j < ap_num;j++)
		{
			__Cycle:
			
			if(strcmp(ap_entry_tmp[i].ssid,ap_entry_tmp[j].ssid) == 0)	
			{
				//printf("[debug filter:%s,%d]:ap_entry_tmp[%d].ssid==%s\n",__FUNCTION__,__LINE__,j,ap_entry_tmp[j].ssid);
				if(ap_entry_tmp[i].singal_level < ap_entry_tmp[j].singal_level)
				{
					memset(ap_entry_tmp+i,0,sizeof(AP_info));
					memcpy(ap_entry_tmp+i,ap_entry_tmp+j,sizeof(AP_info));
					memset(ap_entry_tmp+j,0,sizeof(AP_info));
					
					if(j != ap_num-1)
					{
						memcpy(ap_entry_tmp+j,ap_entry_tmp+ap_num-1,sizeof(AP_info));
						memset(ap_entry_tmp+ap_num-1,0,sizeof(AP_info));

					}
					ap_num--;
				//	printf("[debug filter:%s,%d]:ap_entry_tmp[%d].ssid==%s\n",__FUNCTION__,__LINE__,j,ap_entry_tmp[j].ssid);
					goto __Cycle;
				}
				else
				{
				//	printf("[debug filter:%s,%d]:ap_entry_tmp[%d].ssid==%s\n",__FUNCTION__,__LINE__,j,ap_entry_tmp[j].ssid);

					memset(ap_entry_tmp+j,0,sizeof(AP_info));
					
					if(j != ap_num-1)
					{
						memcpy(ap_entry_tmp+j,ap_entry_tmp+ap_num-1,sizeof(AP_info));
						memset(ap_entry_tmp+ap_num-1,0,sizeof(AP_info));

					}
					ap_num--;
				//	printf("[debug filter:%s,%d]:ap_entry_tmp[%d].ssid==%s\n",__FUNCTION__,__LINE__,j,ap_entry_tmp[j].ssid);
					goto __Cycle;
				}
			}

		}
	//printf("[debug filter:%s,%d]:ap_num===%d\n",__FUNCTION__,__LINE__,ap_num);
	*apNum = ap_num;
}

static int __save_ap_entry(int AP_num,AP_info * ap_entry_tmp)
{
	int ret=0;
	//int i=0;

	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}
	
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	
	filter_sameSsid(&AP_num,ap_entry_tmp);
	wifi_info->AP_num = AP_num;
/*
	for(i=0;i<AP_num;i++)
		printf("[debug info :%s,%d]:ap_entry_tmp[%d].ssid ======== %s\n",__FUNCTION__,__LINE__,i,ap_entry_tmp[i].ssid);
	*/
	if(wifi_info->AP_num==0){
		ret=0;goto __end;
	}

	if(wifi_info->ap_entry){
		free(wifi_info->ap_entry);
		wifi_info->ap_entry=NULL;
	}
	wifi_info->ap_entry = (AP_info *)malloc(wifi_info->AP_num * sizeof(AP_info));
	if(wifi_info->ap_entry != NULL){
		memset(wifi_info->ap_entry,0,wifi_info->AP_num * sizeof(AP_info));
		memcpy(wifi_info->ap_entry,ap_entry_tmp,wifi_info->AP_num * sizeof(AP_info));
	}
	else{
		wifidbg_info("wifi_s AP_entry malloc failed");
		ret = -1; goto __end;
	}
/*
	for(i=0;i<AP_num;i++)
		printf("[debug info :%s,%d]:ap_entry[%d].ssid ======== %s\n",__FUNCTION__,__LINE__,i,wifi_info->ap_entry[i].ssid);
*/
	//dump_ap_entry(wifi_info);
	
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

static void dump(char *buf)
{
	int count=1;
	char black=' ';
	while(*buf)
		{
			count++;
			//printf("%c",*(buf));
			if(count%4==0) 		//printf("\n");
			buf++;
		}
}

/**
*compare the AP.conf get from the latestAP.conf with the ap in the list
*/
static int apMatch(char *tempFileName){

	char apInList[WIFI_BUF_LEN];
	int i;
	struct wifi_info_t *wifiAp = __get_wifiinfo();

	for(i=0;i<wifiAp->AP_num;i++){
	
		memset(apInList,0,strlen(apInList));
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		if(wifiAp->ap_entry[i].frequency<3000)
			sprintf(apInList,"/mnt/vram/wifi/24G/%x.conf",APHash(wifiAp->ap_entry[i].ssid));
		else
			sprintf(apInList,"/mnt/vram/wifi/5G/%x.conf",APHash(wifiAp->ap_entry[i].ssid));
	#else
		sprintf(apInList,"/mnt/vram/wifi/%x.conf",APHash(wifiAp->ap_entry[i].ssid));
	#endif
		if(strcmp(tempFileName,apInList) == 0){
				return 0;
		}
	}
	printf("the ap was not found in the ap list!\n");
	return -1;
}

/*
*check the conf whether it is the hidden ap
*/
static int isHiddenAP(char *tempFileName){
	int ret=0;
	char *keyword = "scan_ssid=1";
	FILE *fp=NULL;
	char buf[512]={0};
	char *locate=NULL;
	
	if((fp=fopen(tempFileName,"r"))==NULL){
		debug_info("open %s error !",tempFileName);
		ret = -1;
		return ret;
	}
	else if(fread(buf,1,sizeof(buf),fp)==0){
		fclose(fp);
		debug_info("read error!");
		ret=-1;
		return ret;
	}
	else {
		fclose(fp);
		if((locate=strstr(buf,keyword))!=NULL)
			ret = 1;
	}
	return ret;
}

static int wpa_scan_results(char * buf,AP_info * ap_entry_tmp,int AP_tmpnum)
{
	char *locate=NULL, *tmpbuf=NULL;
	char tmp[200]={0};
	int i=0;
	int rtn = 0;

	char *find_c="\n";
	locate=strstr(buf,find_c);
	if(!locate)
	{
		wifidbg_info("Not Found!");
		return -1;
	}
	locate++;
	while(locate[0]!='\0')
	{
		
		tmpbuf = strstr(locate,find_c);
		//wifidbg_info("%d",strlen(pos)-strlen(tmpbuf));
		memset(tmp,0,sizeof(tmp));
		if(tmpbuf!=NULL&&strlen(locate)-strlen(tmpbuf)>0)
			strncpy(tmp, locate, strlen(locate)-strlen(tmpbuf));
		//wifidbg_info("%s",tmp);
		//printf("analyze_scan begin!!!\n");
		rtn = analyze_scan(tmp,ap_entry_tmp,i);
		
		//printf("analyze_scan OK!!!\n");
		//if(rtn==0)
			//wifidbg_info("%s\t%d\t%d\t%s\n",ap_entry_tmp[i].mac_address,ap_entry_tmp[i].frequency,ap_entry_tmp[i].singal_level,ap_entry_tmp[i].ssid);
		locate=tmpbuf;
		if(locate)
		{
			if(rtn==0){
				i++;
			}
			locate++;
		}
	}
	wifidbg_info("i is %d,ap num is %d",i,AP_tmpnum);
//	printf("[debug %s %d]  ap_num:%d\n",__func__,__LINE__,AP_tmpnum);
	//printf("go to point @@@@@@!\n");
	//dump(tmpbuf);
	//printf("go to point &&&&&&&&&!\n");

	rtn = __save_ap_entry(i,ap_entry_tmp);
	return rtn;
}

struct autoConnectInfo_s{
	int flag;
	char selected_filename[WIFI_BUF_LEN];
}autoConnectInfo;

// Save selected_filename for auto connect and if connect fail, this connect config file will not delete.
void saveAutoConnectInfo(char *selected_filename){
	if(selected_filename != NULL){
		autoConnectInfo.flag = 1;
		snprintf(autoConnectInfo.selected_filename, sizeof(autoConnectInfo.selected_filename), "%s", selected_filename);
	}
}
int wpa_updatetime()
{
	//	printf("%s,selected_filename==%s\n",__func__,selected_filename);
	wifi_create_dir("/mnt/vram/cert/");
	wifi_create_dir("/etc/cert/");
	if(access("/etc/cert/CA.pem",F_OK)) system("cp -f /mnt/vram/cert/CA.pem  /etc/cert/CA.pem ");
	if(access("/etc/cert/Certificate.pem",F_OK))system("cp -f /mnt/vram/cert/Certificate.pem  /etc/cert/Certificate.pem ");
	if(access("/etc/cert/Privatekey.pem",F_OK)) system("cp -f /mnt/vram/cert/Privatekey.pem  /etc/cert/Privatekey.pem ");;

	if(access("/mnt/vram/time.txt",F_OK)==0&&strstr(selected_filename,"wireless-tkip")==NULL){
		int fd=open(selected_filename,O_RDONLY);
		if(fd<0){
			printf("open fd failed,ret =%d \n",fd);
		}else{
			char buff[1024]={0};
			printf("%s read %d bytes\n",__func__,read(fd,buff,sizeof(buff)));
//			printf("/mnt/vram/time.txt exist!![ %s ]\n",buff);
			if(strstr(buff,"key_mgmt=WPA-EAP")!=NULL){
				puts("connect to WPA-EAP, compare and update time!!");
				system("[ $(cat /mnt/vram/time.txt | xargs date +\%s -d) -gt $(date +\%s) ] && cat /mnt/vram/time.txt | xargs date || echo \"time.txt is old!!\"");
				system("touch /etc/cert/*.pem");		
			}
			close(fd);
		}
	}
	else{
		printf("/mnt/vram/time.txt not exist!!!\n");
		return 0;
	}
 	return 1;
}
int auto_connect()
{
	int i=0,hiddenAP;
	int rtn = -1;
	static int count_num=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	FILE *fp_conf;
	char temp_filename[WIFI_BUF_LEN]={0};
	char temptest[WIFI_BUF_LEN]={0};
	
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return rtn;
	}
	
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	
	if(wifi_info->ap_entry==NULL){
		wifidbg_err("Ap Entry is empty");
//		rtn = -1; 
//		goto __end;
	}	
	
	rebak_latestAPconf();
	if(access(latestAPconf,F_OK) == 0)
	{
		printf("[Debug Info:%s,%d]:in the latest ap conf\n",__FUNCTION__,__LINE__);
		fp_conf = fopen(latestAPconf,"r");
		if(fp_conf == NULL)
		{
			wifidbg_err("open latest ap conf error!\n");
			rtn = -3;						//latest ap operation conf error
			goto __connectAuto;
		}
		//if(fgets(temp_filename,WIFI_BUF_LEN,fp_conf)==0)
		if(fread(temp_filename,1,sizeof(temp_filename),fp_conf)==0)
		{
			wifidbg_err("latest ap conf is empty!\n");
			rtn = -3;						//latest ap operation conf error
			fclose(fp_conf);
			remove(latestAPconf);
			remove(latestAPconf_bak);
			system("sync");
			goto __connectAuto;
		}
		else
		{
			fclose(fp_conf);
			if(strncmp(temp_filename,"/mnt/vram/wifi/",15)!=0)
			{
				wifidbg_err("latest ap conf content error!\n");
				rtn = -3;					//latest ap conf content error
				remove(latestAPconf);
				remove(latestAPconf_bak);
				system("sync");
				goto __connectAuto;
			}
			else
			{
				if(access(temp_filename,F_OK) == 0){
					hiddenAP= isHiddenAP(temp_filename);
					if(hiddenAP==1){
						debug_info("connect hiden ap!!!!!!!!!!!");
						//rtn = connectAP(temp_filename);
						printf("[Debug Info:%s,%d]:temp_filename == %s\n",__FUNCTION__,__LINE__,temp_filename);
						memset(selected_filename,0,sizeof(selected_filename));
						memcpy(selected_filename,temp_filename,sizeof(temp_filename));						
						if(access("/tmp/wpa_supplicant/wlan0",F_OK)==0  ||access("/tmp/wpa_supplicant/wlan2",F_OK)==0){
							//printf("[debug %s %d]\n",__func__,__LINE__);
							saveAutoConnectInfo(selected_filename);
							rtn = wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
							#if EZMUSIC_ENABLE
							//printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
							SysCGI_priv.connect_router_waiting_lan_flag=1;
							SysCGI_priv.connect_router_waiting_flag=1;
							#endif	
						}
						else if(wlan_current_status.current_status_value != WIFI_COMPLETED)
						{
							count_num++;
							if(count_num==3)
							{
								count_num = 0;
								saveAutoConnectInfo(selected_filename);
								rtn = wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
								#if EZMUSIC_ENABLE
								//printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
								SysCGI_priv.connect_router_waiting_lan_flag=1;
								SysCGI_priv.connect_router_waiting_flag=1;
								#endif	
							}
						}
					}
					else if(hiddenAP==0&&apMatch(temp_filename) == 0){			
						//rtn = connectAP(temp_filename);
						printf("[Debug Info:%s,%d]:temp_filename == %s\n",__FUNCTION__,__LINE__,temp_filename);
						memset(selected_filename,0,sizeof(selected_filename));
						memcpy(selected_filename,temp_filename,sizeof(temp_filename));						
						if(access("/tmp/wpa_supplicant/wlan0",F_OK)==0  ||access("/tmp/wpa_supplicant/wlan2",F_OK)==0){
							//printf("[debug %s %d]\n",__func__,__LINE__);
							saveAutoConnectInfo(selected_filename);
							rtn = wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
														#if EZMUSIC_ENABLE
							//printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
							SysCGI_priv.connect_router_waiting_lan_flag=1;
							SysCGI_priv.connect_router_waiting_flag=1;
							#endif	
						}
						else if(wlan_current_status.current_status_value != WIFI_COMPLETED)
						{
							count_num++;
							if(count_num==3)
							{
								count_num = 0;
								saveAutoConnectInfo(selected_filename);
								rtn = wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
								#if EZMUSIC_ENABLE
								//printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
								SysCGI_priv.connect_router_waiting_lan_flag=1;
								SysCGI_priv.connect_router_waiting_flag=1;
								#endif	
							}
						}
					}
					else
						goto __connectAuto;
				}
				else {	//if the configeration file don't exist ,clear the record!
					remove(latestAPconf);
					remove(latestAPconf_bak);
					system("sync");
					goto __connectAuto;
				}
			}
		}
	}
		
	else{

__connectAuto:
		for(i=0;i<wifi_info->AP_num;i++){
			if(check_conf_exist(wifi_info->ap_entry[i].ssid)==1&&wps_connect_flag==0){
			#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
				if(wifi_info->ap_entry[i].frequency<3000)//2.4G
					sprintf(selected_filename,"/mnt/vram/wifi/24G/%x.conf",APHash(wifi_info->ap_entry[i].ssid));
				else
					sprintf(selected_filename,"/mnt/vram/wifi/5G/%x.conf",APHash(wifi_info->ap_entry[i].ssid));
			#else
				sprintf(selected_filename,"/mnt/vram/wifi/%x.conf",APHash(wifi_info->ap_entry[i].ssid));
			#endif
			//	selected_ap = i;
				fprintf(stderr,"function:%s line:%d selected_filename:%s\n",__FUNCTION__,__LINE__,selected_filename);
				if(access("/tmp/wpa_supplicant/wlan0",F_OK)==0 ||access("/tmp/wpa_supplicant/wlan2",F_OK)==0){
					//printf("[debug %s %d]\n",__func__,__LINE__);
					saveAutoConnectInfo(selected_filename);
					rtn = wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
					#if EZMUSIC_ENABLE
					//printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
					SysCGI_priv.connect_router_waiting_lan_flag=1;
					SysCGI_priv.connect_router_waiting_flag=1;
					#endif	
				}else{
					if(wlan_current_status.current_status_value!=WIFI_COMPLETED){
						count_num++;
						if(count_num==3){
							count_num = 0;
							saveAutoConnectInfo(selected_filename);
							rtn = wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
							#if EZMUSIC_ENABLE
							//printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
							SysCGI_priv.connect_router_waiting_lan_flag=1;
							SysCGI_priv.connect_router_waiting_flag=1;
							#endif	
						}
					}
				}
				break;
				
				//rtn = wifi_send_msg(wifi_info,WIFI_CMD_SCAN);				
			}
		}
		//fprintf(stderr,"function:%s line:%d",__FUNCTION__,__LINE__);
		if(i==wifi_info->AP_num)
		{
			rtn=-2;
			#if EZMUSIC_ENABLE
			//printf("not_connected_router_useful_flag======%s,%d\n",__FUNCTION__,__LINE__);
			//SysCGI_priv.connect_router_waiting_flag=1;
			SysCGI_priv.not_connected_router_useful_flag=1;
			#endif
		}
			
			
	}
__end:
	memset(temp_filename,0,sizeof(temp_filename));
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	
	return rtn;
}

static int wpa_scan_ap()
{
	char buf[2048] = {0};
	int rtn = 0;
	size_t len = sizeof(buf) - 1;
	if(ctrl_connect_opened == 0 || (rtn=ctrlRequest("SCAN", buf, &len)) < 0) {
		wifidbg_err("ctrl Request error!rtn:%d",rtn);
		rtn = -1;
	}
	//wifidbg_err("scan result is %d",rtn);
	//wifidbg_err("scan result is %s",buf);
	return rtn;
}

static int calculate_APnum(char * buf)
{
	char * find_c = "\n";
	char * find_hiddenap = "<hidden>";
	char * locate=NULL, *tmpbuf=NULL;
	int count = 0;
	int count_hiddenap = 0;
	locate = strstr(buf,find_c);
	if(!locate)
	{
		wifidbg_info("Not Found!");
		return -1;
	}
	locate++;
	while(locate[0]!='\0')
	{
		
		tmpbuf = strstr(locate,find_c);
		locate=tmpbuf;
		if(locate)
		{
			count++;
			locate++;
		}
	}
	locate = strstr(buf,find_hiddenap);
	while(locate!=NULL){
		count_hiddenap++;
		locate = locate + 8;
		tmpbuf = strstr(locate,find_hiddenap);
		locate = tmpbuf;
	}
	count = count - count_hiddenap;
	return count;
}

static int wpa_get_scan_results()
{
	AP_info * ap_entry_tmp = NULL;
	int AP_tmpnum = 0;
	int rtn = 0;
	int buf_size=0X5000;
	char buf[0X5000] = {0};
	size_t len = buf_size - 1;
	if(ctrl_connect_opened == 0 || (rtn=ctrlRequest("SCAN_RESULTS", buf, &len)) < 0) {
		wifidbg_err("ctrl Request error!rtn:%d",rtn);
		rtn = -1;
	}
	if(rtn == 0){
		AP_tmpnum = calculate_APnum(buf);
		if(AP_tmpnum>0){
			ap_entry_tmp = (AP_info *)malloc(AP_tmpnum * sizeof(AP_info));
			if(ap_entry_tmp != NULL){
				memset(ap_entry_tmp,0,AP_tmpnum * sizeof(AP_info));
				wpa_scan_results(buf,ap_entry_tmp,AP_tmpnum);
				
			}
			else{
				wifidbg_err("wifi_s AP_entry malloc failed");
				
				rtn= -1;
				goto ___wpa_get_scan_results__end___;
			}
		}
	}
	
	if(ap_entry_tmp){
		wifidbg_info("free ap_entry_tmp info");
		free(ap_entry_tmp);
		ap_entry_tmp=NULL;
	}

___wpa_get_scan_results__end___:
	
	return rtn;
}

int __release_ap_entry()
{

	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	wifi_info->AP_num = 0;
	if(wifi_info->ap_entry){
		//wifidbg_info("free ap info");
		free(wifi_info->ap_entry);
		wifi_info->ap_entry=NULL;
	}
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

static int wifi_scan_check()
{
	int zero_num=0;
	int sleep_times=0;
	struct timespec time_escape;
	time_escape.tv_sec = 1;
	time_escape.tv_nsec = 0;
	while(1){
		//printf("__release_ap_entry\n");
		__release_ap_entry();
		
		//printf("wpa_get_scan_results\n");
		wpa_get_scan_results();
		if(wifi_s.AP_num > 0){
			wifidbg_info("scan completely!");
			break;
		}
		else{
			nanosleep(&time_escape,NULL);
			zero_num++;
			//wifidbg_info("zero num is %d\n",zero_num);
			if(zero_num>=20){
				zero_num = 0;
				wifidbg_info("there is no ap!");
				break;
			}
		}
		if(access("/tmp/wpa_supplicant",F_OK)!=0)
			break;
	}
	return 0;
}

static int check_conffile_num();
static int check_conffile_num_doublewifi();




static int __get_conf_content(char *buf,int sel_ap,char*f_name,int name_len)
{
	int password_len=0;
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	//wps_connect_flag=0;
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}
	memset(f_name,0,name_len);
	
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret = -1; goto __end;
	}

	switch(wifi_info->ap_entry[sel_ap].authen)
	{
		case WPA://remove updata_config=1 options
			if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0){
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n}\n",wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].ap_password);
			}else{
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\nnetwork={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n}\n",
						wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].ap_password);
			}
			break;
		case WEP:
			password_len=strlen(wifi_info->ap_entry[sel_ap].ap_password);
			printf("password_len=============================%d\n",password_len);
			if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0){//realtek wpa config
				if(password_len==10 || password_len==26 || password_len==32)				
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key%d=%s\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else if(password_len==5 || password_len==13 || password_len==16)
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key%d=\"%s\"\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else
					;
			}else{ //ralink wpa config file
				if(password_len==10 || password_len==26 || password_len==32)	
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\n\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key%d=%s\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else if(password_len==5 || password_len==13 || password_len==16)
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key%d=\"%s\"\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else
					;
			}
			break;
		case OPEN:
			if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0){
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n}\n",
					wifi_info->ap_entry[sel_ap].ssid);
			}else{
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n}\n",
					wifi_info->ap_entry[sel_ap].ssid);
			}
			break;
	}
	//printf("%s, len is %d\n",buf,strlen(buf));
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	if(wifi_info->ap_entry[sel_ap].frequency<3000)//2.4G
		sprintf(f_name,"/mnt/vram/wifi/24G/%x.conf",conf_name_transition( sel_ap));
	else
		sprintf(f_name,"/mnt/vram/wifi/5G/%x.conf",conf_name_transition( sel_ap));
#else 
	sprintf(f_name,"/mnt/vram/wifi/%x.conf",conf_name_transition( sel_ap));
#endif
	
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	
	return ret;
}

/**
*write down the added ap conf content
*/
static int __get_addedAP_content(char *buf,int sel_ap,char*f_name,int name_len)
{
	int password_len=0;
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	//wps_connect_flag=0;
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}
	memset(f_name,0,name_len);
	
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret = -1; goto __end;
	}

	switch(wifi_info->ap_entry[sel_ap].authen)
	{
		case WPA://remove updata_config=1 options
			if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0){
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tpsk=\"%s\"\n}\n",wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].ap_password);
			}else{
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tpsk=\"%s\"\n}\n",
						wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].ap_password);
			}
			break;
		case WEP:
			password_len=strlen(wifi_info->ap_entry[sel_ap].ap_password);
			printf("password_len=============================%d\n",password_len);
			if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0){//realtek wpa config
				if(password_len==10 || password_len==26 || password_len==32)				
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n\twep_key%d=%s\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else if(password_len==5 || password_len==13 || password_len==16)
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n\twep_key%d=\"%s\"\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else
					;
			}else{ //ralink wpa config file
				if(password_len==10 || password_len==26 || password_len==32)	
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n\twep_key%d=%s\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else if(password_len==5 || password_len==13 || password_len==16)
					sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n\twep_key%d=\"%s\"\n\twep_tx_keyidx=%d\n\tpriority=5\n}\n",
					wifi_info->ap_entry[sel_ap].ssid,wifi_info->ap_entry[sel_ap].key_index,wifi_info->ap_entry[sel_ap].ap_password,wifi_info->ap_entry[sel_ap].key_index);
				else
					;
			}
			break;
		case OPEN:
			if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0){
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\ndevice_name=RTL8192DU\nmanufacturer=Realtek\nmodel_name=RTW_STA\nmodel_number=WLAN_DU\nserial_number=12345\ndevice_type=6-0050F204-1\nos_version=01020300\nconfig_methods=virtual_display virtual_push_button physical_push_button\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n}\n",
					wifi_info->ap_entry[sel_ap].ssid);
			}else{
				sprintf(buf,"\nctrl_interface=/tmp/wpa_supplicant\n\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tkey_mgmt=NONE\n}\n",
					wifi_info->ap_entry[sel_ap].ssid);
			}
			break;
	}
	//printf("%s, len is %d\n",buf,strlen(buf));
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(wifi_info->ap_entry[sel_ap].frequency<3000)
		sprintf(f_name,"/mnt/vram/wifi/24G/%x.conf",conf_name_transition( sel_ap));
	else
		sprintf(f_name,"/mnt/vram/wifi/5g/%x.conf",conf_name_transition( sel_ap));
#else
	sprintf(f_name,"/mnt/vram/wifi/%x.conf",conf_name_transition( sel_ap));
#endif
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	
	return ret;
}


int save_conffile()
{
	FILE *fp_conf = NULL;
	int fd_conf;
	char buf[512]={0};
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	printf("selected_ap==============%d\n",selected_ap);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
	if(access("/mnt/vram/wifi/5G",0)!=0)
		system("mkdir /mnt/vram/wifi/5G ");
	if(access("/mnt/vram/wifi/24G",0)!=0)
		system("mkdir /mnt/vram/wifi/24G ");


	printf("--save_conffile frequency=%d\n",wifi_info->ap_entry[selected_ap].frequency);
	if(wifi_info->AP_num == 0)				//if the ap_num is 0,don't save conf file
		return -1;
	else{
		if(wifi_info->ap_entry[selected_ap].frequency<3000)//2.4G
			sprintf(temp_filename,"/mnt/vram/wifi/24G/%x.conf",APHash(wifi_info->ap_entry[selected_ap].ssid));
		else
			sprintf(temp_filename,"/mnt/vram/wifi/5G/%x.conf",APHash(wifi_info->ap_entry[selected_ap].ssid));
	}
#else
	if(wifi_info->AP_num == 0)				//if the ap_num is 0,don't save conf file
		return -1;
	else
		sprintf(temp_filename,"/mnt/vram/wifi/%x.conf",APHash(wifi_info->ap_entry[selected_ap].ssid));

#endif
	if(__get_conf_content(buf,selected_ap,temp_filename,WIFI_BUF_LEN)==-1)
		return -1;
	
	fp_conf = fopen(temp_filename,"wb+");
	if(fp_conf==NULL)
	{
		wifidbg_err("can not open %s\n",temp_filename);
		return -1;
	}
	if(fwrite(buf,sizeof(char),strlen(buf),fp_conf) != strlen(buf))
	{
		wifidbg_err("write conf file error!");
		fclose(fp_conf);
		return -1;
	}
	debug_info("temp_filename is %s",temp_filename);
	fflush(fp_conf);
	fd_conf= fileno(fp_conf);
	fsync(fd_conf);
	fclose(fp_conf);
	fp_conf = NULL;
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	check_conffile_num_doublewifi();
#else
	check_conffile_num();
#endif
	return 0;
} 

/**
*save the added ap conf file
*/
int save_addedAP_conffile()
{
	FILE *fp_conf = NULL;
	int fd_conf;
	char buf[512]={0};
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	printf("selected_ap==============%d\n",selected_ap);
	
	if(wifi_info->AP_num == 0)				//if the ap_num is 0,don't save conf file
		return -1;
	else
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(access("/mnt/vram/wifi/5G",0)!=0)
		system("mkdir /mnt/vram/wifi/5G ");
	if(access("/mnt/vram/wifi/24G",0)!=0)
		system("mkdir /mnt/vram/wifi/24G ");
	if(wifi_info->ap_entry[selected_ap].frequency<3000)
		sprintf(temp_filename,"/mnt/vram/wifi/24G/%x.conf",APHash(wifi_info->ap_entry[selected_ap].ssid));
	else
		sprintf(temp_filename,"/mnt/vram/wifi/5G/%x.conf",APHash(wifi_info->ap_entry[selected_ap].ssid));
		
#else
	sprintf(temp_filename,"/mnt/vram/wifi/%x.conf",APHash(wifi_info->ap_entry[selected_ap].ssid));
#endif
	if(__get_addedAP_content(buf,selected_ap,temp_filename,WIFI_BUF_LEN)==-1)
		return -1;
	
	fp_conf = fopen(temp_filename,"wb+");
	if(fp_conf==NULL)
	{
		wifidbg_err("can not open %s\n",temp_filename);
		return -1;
	}
	if(fwrite(buf,sizeof(char),strlen(buf),fp_conf) != strlen(buf))
	{
		wifidbg_err("write conf file error!");
		fclose(fp_conf);
		return -1;
	}
	debug_info("temp_filename is %s",temp_filename);
	fflush(fp_conf);
	fd_conf= fileno(fp_conf);
	fsync(fd_conf);
	fclose(fp_conf);
	fp_conf = NULL;
	check_conffile_num();
	return 0;
} 


int delete_conffile(int sel_ap)
{
	int rtn=0;
	char *conf_file=NULL;
	conf_file = __get_conf_filename(sel_ap);
	//fprintf(stderr,"function:%s line:%d conf_file:%s\n",__FUNCTION__,__LINE__,conf_file);
	
	if(conf_file==NULL)
		return -1;
	if(access(conf_file,F_OK)==0){
		remove(conf_file);
		system("sync");
	}
	//rtn = (rtn==ASFS_ERR_OK?0:-1);
	return rtn;
}

/**defined for calculate signal level*/
#define MAX_RSSI 256
#define MIN_RSSI -200
#define MAX_CAL_RSSI -55
#define MIN_CAL_RSSI -100
#define RSSI_LEVEL 5  // 0-4

static int calculate_Signal_Level(int rssi,int levels){
	if(rssi < MIN_CAL_RSSI)
		return 0;
	else if(rssi >= MAX_CAL_RSSI)
		return levels -1;
	else{
		float inputRange = (MAX_CAL_RSSI - MIN_CAL_RSSI);
		float outputRange = levels;
		return (int)((float)(rssi - MIN_CAL_RSSI) * outputRange / inputRange);
	}
}

int wpa_connect_status()
{
	char *start, *end, *pos;
	int last = 0;
	char *locate = "wpa_state=";
	int tmp_status = WIFI_DISCONNECTED;
	char buf[2048] = {0};
	int rtn = 0;
	int temp_wifiMode = -1;
//	int wpa_status=0;
//	struct wifi_info_t *wifi_info = __get_wifiinfo();
//	sem_getvalue(&wifi_info->semi_wpastatus, &wpa_status);
//	wifi_sem_wait(&wifi_info->semi_wpastatus);
//	sem_getvalue(&wifi_info->semi_wpastatus, &wpa_status);
	size_t len = sizeof(buf) - 1;
	wifidbg_info("Run here!!!!!!!!!!!!!!");
	
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(ctrl_connect_opened == 0 || (rtn=ctrlRequest("STATUS", buf, &len)) < 0) {
		wifidbg_err("ctrl Request error!rtn:%d\tctrl_connect_opened ==== %d\n",rtn,ctrl_connect_opened);
		if(rtn == -2){
			wifidbg_err("nothing will be done!");
			return 	WIFI_COMPLETED;
		}
		if(ctrl_connect_opened == 0){
			temp_wifiMode = read_wifi_mode();
			switch(temp_wifiMode){
				case WIFI_CLIENT_MODE_ENABLE:
				case WIFI_CLIENT_GETAUTOIP_OK:
				case WIFI_CLIENT_GETAUTOIP_ERR:  
				case WIFI_CONCURRENT_CLIENT_ENABLE:
				case WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK:
				case WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR:
					openCtrlConnection();
					break;
				default :
					break;
			}
		}
		return WIFI_DISCONNECTED;
	}
	
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	//wifidbg_info("status result is %d",rtn);
	//wifidbg_info("status result is:\n %s",buf);
	
	pos = strstr(buf,locate);
	if(pos==NULL){
		wifidbg_err("no find wpa state");
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
			if (strcmp(start, "bssid") == 0){
				memset(wlan_current_status.bssid,0,sizeof(wlan_current_status.bssid));
				memcpy(wlan_current_status.bssid,pos,strlen(pos));
				//printf("bssid is %s\n",wlan_current_status.bssid);
			}
			else if(strcmp(start, "ssid") == 0){
				memset(wlan_current_status.ssid,0,sizeof(wlan_current_status.ssid));
				memcpy(wlan_current_status.ssid,pos,strlen(pos));
				//printf("ssid is %s\n",wlan_current_status.ssid);
			}
			else if(strcmp(start, "ip_address") == 0){
				memset(wlan_current_status.ip_address,0,sizeof(wlan_current_status.ip_address));
				memcpy(wlan_current_status.ip_address,pos,strlen(pos));
				//wifidbg_info("ip_address is %s",wlan_current_status.ip_address);
			}
			else if(strcmp(start, "wpa_state") == 0){
				memset(wlan_current_status.wpa_state,0,sizeof(wlan_current_status.wpa_state));
				memcpy(wlan_current_status.wpa_state,pos,strlen(pos));
				//wifidbg_info("wpa_state is %s",wlan_current_status.wpa_state);
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
			else if(strcmp(start, "key_mgmt") == 0){
				memset(wlan_current_status.key_mgmt,0,sizeof(wlan_current_status.key_mgmt));
				memcpy(wlan_current_status.key_mgmt,pos,strlen(pos));
				//wifidbg_info("key_mgmt is %s",wlan_current_status.key_mgmt);
			}
			else if(strcmp(start, "pairwise_cipher") == 0){
				memset(wlan_current_status.pairwise_cipher,0,sizeof(wlan_current_status.pairwise_cipher));
				memcpy(wlan_current_status.pairwise_cipher,pos,strlen(pos));
				//wifidbg_info("pairwise_cipher is %s",wlan_current_status.pairwise_cipher);
			}
			else if(strcmp(start, "group_cipher") == 0){
				memset(wlan_current_status.group_cipher,0,sizeof(wlan_current_status.group_cipher));
				memcpy(wlan_current_status.group_cipher,pos,strlen(pos));
				//wifidbg_info("group_cipher is %s",wlan_current_status.group_cipher);
			}
			else if(strcmp(start, "mode") == 0){
				memset(wlan_current_status.mode,0,sizeof(wlan_current_status.mode));
				memcpy(wlan_current_status.mode,pos,strlen(pos));
				//wifidbg_info("mode is %s",wlan_current_status.mode);
			}
			else if(strcmp(start, "passphrase") == 0){
				memset(wlan_current_status.passphrase,0,sizeof(wlan_current_status.passphrase));
				memcpy(wlan_current_status.passphrase,pos,strlen(pos));
				//wifidbg_info("mode is %s",wlan_current_status.mode);
				
			}
			else if(strcmp(start, "signal_level") == 0){
				int signal_level = 0;
				memset(wlan_current_status.signal_level,0,sizeof(wlan_current_status.signal_level));
				if(access("/sys/module/8821cu",F_OK) == 0 || access("/sys/module/8188fu",F_OK) == 0){
					int rssi = atoi(pos) - 100;
					if(rssi != -1 && rssi > MIN_RSSI && rssi < MAX_RSSI){
						if(rssi > 0)
							rssi -= MAX_RSSI;
						int level = calculate_Signal_Level(rssi,RSSI_LEVEL);
						if(level == 0){
							signal_level = 0;
						}else if(level == 1){
							signal_level = 25;
						}else if(level == 2){
							signal_level = 50;
						}else if(level == 3){
							signal_level = 75;
						}else if(level == 4){
							signal_level = 100;
						}else{
							signal_level = 0;
						}
					}
					sprintf(wlan_current_status.signal_level, "%d", signal_level);
				}else{
					memcpy(wlan_current_status.signal_level,pos,strlen(pos));
				}
				//printf("[debug info]%s,%d,wlan_current_status.signal_level is %s\n",__FUNCTION__,__LINE__,wlan_current_status.signal_level);
			}
		}
		if(last)
			break;
		start = end + 1;
	}
	//wifidbg_info("the router is %s",wlan_current_status.router);
	//wifidbg_info("the DNS is %s",wlan_current_status.DNS_server);
	//printf("[debug info : %s,%d]:tmp_staus====%d\n",__FUNCTION__,__LINE__,tmp_status);
	wlan_current_status.current_status_value = tmp_status;
//	sem_getvalue(&wifi_info->semi_wpastatus, &wpa_status);
//	wifi_sem_wait(&wifi_info->semi_wpastatus);
//	sem_getvalue(&wifi_info->semi_wpastatus, &wpa_status);
	return tmp_status;
}

#if EZCAST_ENABLE
int get_connection_status(){
	return wlan_current_status.current_status_value;
}
#endif

static char* __get_conf_filename(int sel_ap)
{
	char *name=NULL;
	int filename_int=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	char *filename_temp=wifi_info->ap_entry[sel_ap].ssid;
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return name;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		debug_info("wifi_info->AP_num is %d",wifi_info->AP_num);
		goto __end;
	}
	
	filename_int=APHash(filename_temp);
	memset(temp_filename,0,WIFI_BUF_LEN);
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(wifi_info->ap_entry[sel_ap].frequency<3000)//2.4G
		sprintf(temp_filename,"/mnt/vram/wifi/24G/%x.conf",filename_int);
	else
		sprintf(temp_filename,"/mnt/vram/wifi/5g/%x.conf",filename_int);
#else
	sprintf(temp_filename,"/mnt/vram/wifi/%x.conf",filename_int);

#endif


/*
	if(connect_sem_flag == 0){
		memset(selected_filename,0,WIFI_BUF_LEN);
		strcpy(selected_filename,temp_filename);	
		debug_info("selected_filename is %s",selected_filename);
	}
*/


	//name = selected_filename;
	//printf("name is %s\n",name);
	
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return temp_filename;
}

static int CheckAuthenType(int sel_ap)
{
	FILE *fp=NULL;
	int authenType = -1;
	int ret=0;
	char buf[512]={0};
	char *keyWord = "key_mgmt=NONE";
	char *wepWord = "wep_key";
	char *keyPointer =NULL;
	char *wepPointer =NULL;
	int confAuthen =-1;
/*
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		ret = -2;
		goto _end;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret = -2;
		goto _end;
	}
*/
	authenType = __get_authentype(sel_ap);
	debug_info("authenType is ==== %d",authenType);
	if( authenType == -1){
		ret = -1;
		goto _end;
	}
	debug_info("temp_filename is ==== %s",temp_filename);	
	if(access(temp_filename,F_OK)!=0){
		ret = -2;
		goto _end;
	}
	fp=fopen(temp_filename,"r");

	if(fp==NULL){
		ret = -2;
		goto _end;
	}
	if(fread(buf,1,512,fp)==0){
		fclose(fp);
		ret = -2;
		goto _end;
	}		
	fclose(fp);

	keyPointer = strstr(buf,keyWord);
	wepPointer = strstr(buf,wepWord);

	if(keyPointer == NULL && wepPointer == NULL)
		confAuthen = 0;
	else if(wepPointer == NULL)
		confAuthen = 2;
	else 
		confAuthen = 1;

	debug_info("confAuthen ==== is %d",confAuthen);

	if(confAuthen != authenType){
		
		remove(temp_filename);
		system("sync");
		memset(temp_filename,0,WIFI_BUF_LEN);
		ret = 1;
		goto _end;
	}

_end:
	debug_info("ret is %d",ret);
	//wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}


extern char *ssid_connecting;

int __check_conf_exist(int sel_ap)
{
	char * conf_name=NULL;
	wps_connect_flag=0;
	conf_name = __get_conf_filename(sel_ap);
	debug_info("conf_name is %s\tsel_ap is %d",conf_name,sel_ap);
	if(conf_name==NULL){
	
		return 0;
	}
	//debug_info("%s",conf_name);
	if(access(conf_name,F_OK)!=-1)
	{
		if(CheckAuthenType(sel_ap) == 0){
			selected_ap=sel_ap;
			debug_info("%s is exist",conf_name);
			return 1;
		}
		else
			return 0;
	}
	else
	{
		debug_info("%s is not exist",conf_name);
		return 0;
	}
}

int wifi_reset()
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	int rtn = 0;
	char *device=NULL;
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	
	if(strstr(selected_filename,"24G")){
		printf("ifconfig wlan2 down\n");
		system("ifconfig wlan2 down ");
	}
	#endif
	memset(selected_filename,0,strlen(selected_filename));
	strncpy(selected_filename, "/etc/wireless-tkip.conf", strlen("/etc/wireless-tkip.conf"));
	memset(selected_ssid,0,strlen(selected_ssid));
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	closeCtrlConnection();
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	kill_wpa_supplicant();
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	device = dongle_get_device();
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	wlan_down(device);
	if(device==NULL){
		return -1;
	}
	//printf("%s,%d\n",__FUNCTION__,__LINE__);

	rtn = wlan_up(device);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(rtn!=0)
	{
		wifidbg_err("ifconfig %s up is error!",device);
		return rtn;
	}
	//printf("%s,%d\n",__FUNCTION__,__LINE__);

	rtn = wlan_conf(device,selected_filename);
	//printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(rtn!=0)
	{
		wifidbg_err("wpa_supplicant conf is error!");
		return rtn;
	}	
	wifidbg_err();
	if(openCtrlConnection()!=0)
	{
		wifidbg_err("open ctrl connectiong is error!");
		rtn = -1;
	}
	wlan_status = WIFI_INACTIVE;
	wifidbg_err();
	return rtn;
}

int __set_signal(int sel_ap ,int singal_level)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return 0;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret=0;goto __end;
	}
	

	wifi_info->ap_entry[sel_ap].singal_level = singal_level;

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

int __get_signal(int sel_ap)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return 0;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret=0;goto __end;
	}
	

	ret = wifi_info->ap_entry[sel_ap].singal_level;

	if(access("/sys/module/8821cu",F_OK) == 0){
		int rssi = ret - 100;
		if(rssi != -1 && rssi > MIN_RSSI && rssi < MAX_RSSI){
			if(rssi > 0)
				rssi -= MAX_RSSI;
			int level = calculate_Signal_Level(rssi,RSSI_LEVEL);
			wifidbg_info("index[%d]:wifi signal level = %d\n",sel_ap,level);
			if(level == 0){
				ret = 0;
			}else if(level == 1){
				ret = 25;
			}else if(level == 2){
				ret = 50;
			}else if(level == 3){
				ret = 75;
			}else if(level == 4){
				ret = 100;
			}else{
				wifidbg_info("index[%d]:get wifi signal level error\n",sel_ap);
				ret = 0;
			}
		}
	}
__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}


int __get_ssid(int sel_ap,char *ssid_buf,int buf_len)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	//debug_info();
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL){
		debug_info();
		ret=-1;
		goto __end;
	}
	
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

	memset(ssid_buf,0,buf_len);
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	strncpy(ssid_buf,wifi_info->ap_entry[sel_ap].ssid,buf_len);	
	//debug_info("ssid_buf is %s",ssid_buf);

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}
int __set_ssid(int sel_ap,char *ssid_buf)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	
	if(wifi_info==NULL){
		wifidbg_err("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret=-1;goto __end;
	}
	
	memset(wifi_info->ap_entry[sel_ap].ssid,0,MAX_SSID_LEN);
	memcpy(wifi_info->ap_entry[sel_ap].ssid,ssid_buf,strlen(ssid_buf));
	

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

int __set_keyidx(int sel_ap,int key_idx)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret=-1;goto __end;
	}

	 if(key_idx<0 || key_idx>3){
	 	ret =-1;goto __end;
	 }
	
	wifi_info->ap_entry[sel_ap].key_index = key_idx;

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}


int __get_authentype(int sel_ap)
{
	int ret=-1;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		debug_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		debug_info();
		ret=-1;
		goto __end;
	}

	ret = wifi_info->ap_entry[sel_ap].authen;

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

int __set_authentype(int sel_ap,int authen_type)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret=-1;goto __end;
	}

	if(authen_type<WPA || authen_type>OPEN){
		wifidbg_err("Auth Type is error!");
		ret=-1; goto __end;
	}

	wifi_info->ap_entry[sel_ap].authen = authen_type;

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

#if 0
void __set_ap_info(int sel_ap,void info_handel,int cmd)
{
	int ret=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	
	if(wifi_info==NULL){
		wifidbg_err("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		ret=-1;goto __end;
	}
	switch(cmd){
		case ISVALID_INFO:
			break;
		case SSID_INFO:
			char *ssid_info = (char *)info_handel;
			memset(wifi_info->ap_entry[sel_ap].ssid,0,MAX_SSID_LEN);
			memcpy(wifi_info->ap_entry[sel_ap].ssid,ssid_info,strlen(ssid_info));
			break;
		case MAC_ADDRESS_INFO:
			break;
		case FREQUENCY_INFO:
			break;
		case SIGNAL_LEVEL_INFO:
			int singal_level = (int)info_handel ;
			wifi_info->ap_entry[sel_ap].singal_level = singal_level;
			break;
		case AUTHEN_INFO:
			authen_type authen_type = (authen_type)info_handel; 
			wifi_info->ap_entry[sel_ap].authen = authen_type;
			
			break;
		case ISVALID_INFO:
			
			break;
		case KEY_INDEX_INFO:

		case AP_PASSWORD_INFO:
		case IP_INFO:
	}

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

int __get_ap_info(int sel_ap,int cmd)
{
	void get_apinfo_result;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	
	if(wifi_info==NULL){
		wifidbg_err("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		(int)get_apinfo_result
		get_apinfo_result=-1;
		goto __end;
	}
	switch(cmd){
		case ISVALID_INFO:
			break;
		case SSID_INFO:
			
			(char[MAX_SSID_LEN])get_apinfo_result;
			memset(get_apinfo_result,0,MAX_SSID_LEN);
			memcpy(get_apinfo_result,wifi_info->ap_entry[sel_ap].ssid,strlen(wifi_info->ap_entry[sel_ap].ssid));
			break;
		case MAC_ADDRESS_INFO:
			break;
		case FREQUENCY_INFO:
			break;
		case SIGNAL_LEVEL_INFO:
			
			(int)get_apinfo_result;
			get_apinfo_resul t= wifi_info->ap_entry[sel_ap].singal_level
			break;
		case AUTHEN_INFO:
			
			(int)get_apinfo_result;
			get_apinfo_result = wifi_info->ap_entry[sel_ap].authen;
			
			
			break;
		case ISVALID_INFO:
			
			break;
		case KEY_INDEX_INFO:

		case AP_PASSWORD_INFO:
		case IP_INFO:
	}

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return get_apinfo_result;
}

#endif


int start_thttpd_process(){
	char cmd[128]={0};
	
	memset(cmd,0,128);	
	sprintf(cmd,"killall thttpd");
	system(cmd);
			
	memset(cmd,0,128);	
	sprintf(cmd,"chmod -R 644 /mnt/user1/thttpd/html/");
	system(cmd);
	
	memset(cmd,0,128);	
	sprintf(cmd,"chmod -R 755 /mnt/user1/thttpd/html/cgi-bin/");
	system(cmd);
				
	memset(cmd,0,128);	
	sprintf(cmd,"thttpd -C /etc/thttpd.conf -u root");
	system(cmd);
	
	return 0;
}



int __get_connected_ap_index()
{
	int ret=0;
	int i = 0;
	
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	int connect_ap_index = wifi_info->AP_num - 1;
	
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL){
		ret=-1;
		goto __end;
	}
	for(i = 0 ; i<wifi_info->AP_num ; i++){
		if(strcmp(wifi_info->ap_entry[i].ssid,wlan_current_status.ssid) == 0){
			connect_ap_index = i;
			goto __end;
		}
	}

	


__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return connect_ap_index;
}

int exchange_wifi_ap(){
	
	char first_ap[MAX_SSID_LEN]={0};
	int connected_ssid_index = 0;
	
	int first_authentype = 0;
	int connecting_ap_authentype = 0;
	
	int first_signlevel = 0;
	int connecting_ap_signlevel = 0;
	
	memset(first_ap,0,MAX_SSID_LEN);
	__get_ssid(0,first_ap,MAX_SSID_LEN);
	//printf(">>>>>>>>>>>>>>>>>.first_ap ========= %s\n",first_ap);
	//printf("wlan_current_status.ssid ========= %s\n",wlan_current_status.ssid);
	//printf("wlan_current_status.current_status_value ========= %d \n",wlan_current_status.current_status_value);
	if(strlen(wlan_current_status.ssid)&&strcmp(first_ap,wlan_current_status.ssid )&& wlan_current_status.current_status_value == WIFI_COMPLETED){
		connected_ssid_index = __get_connected_ap_index();
		__set_ssid(0,wlan_current_status.ssid);
		__set_ssid(connected_ssid_index,first_ap);

		first_authentype = __get_authentype(0);
		connecting_ap_authentype = __get_authentype(connected_ssid_index);
		__set_authentype(0,connecting_ap_authentype);
		__set_authentype(connected_ssid_index,first_authentype);


		first_signlevel = __get_signal(0);
		connecting_ap_signlevel = __get_signal(connected_ssid_index);
		__set_signal(0,connecting_ap_signlevel);
		__set_signal(connecting_ap_signlevel,first_signlevel);
		
		return 0 ;
	}
	return -1;
}
#if 0
int confirm_ap_ssid(char *conf_path){
	int ret = -1;
	char ssid_from_conf[MAX_SSID_LEN] = {0};
	
	ret = get_info_from_file(conf_path,"ssid=\"","\"",ssid_from_conf);
	if(GET_FILEINFO_OK < ret)
		return -1;
	
	if(strcmp(wlan_current_status.ssid,ssid_from_conf) == 0)
		return 0;
	else 
		return -1;
}


int compare_wpa_psk(){
	
}
int confirm_ap_psk(char *conf_path,int ap_security){
	
	switch(ap_security){
		case WPA:
			return compare_wpa_psk(conf_path);
			
			break;
			
		case WEP:
			
			return compare_wep_psk();
			
			break;
			
	}
	return -1;
}

int confirm_ap_security(char *conf_path){
	
	char NONE_security_from_conf[63] = {0};
	char security_from_conf[63] = {0};
	int ret = -1;
	int ap_security = 0;
	
	ret = get_info_from_file(conf_path,"key_mgmt","\n",security_from_conf);
	
	if(CANNOT_FIND_INFO_FILE == ret)
			return -1;
	if(CANNOT_FIND_BEGIN_KEYWORD == ret &&(strstr(wlan_current_status.key_mgmt,"wpa"))){
		ap_security = WPA ;
		return confire_ap_psk(conf_path,WPA);
	}
	if(GET_FILEINFO_OK == ret){
		ret = get_info_from_file(conf_path,"wep_tx_keyidx","\n",NONE_security_from_conf);
		
		if(CANNOT_FIND_BEGIN_KEYWORD == ret && wlan_current_status.key_mgmt == "NONE"){
			return 0;
		}
		if(GET_FILEINFO_OK == ret && wlan_current_status.key_mgmt == "NONE"){
			
			return confire_ap_psk(conf_path,WEP);
		}
	}
	return -1;
}



int confirm_ap_info(){
	char conf_path[WIFI_BUF_LEN] = {0};
	
	sprintf(conf_path,"/mnt/vram/wifi/%x.conf",APHash(wlan_current_status.ssid));
	
	
}
#endif

/**
*save the latest ap configeration path in the latestAP.conf
*/
static int save_latestAPconf()
{
	FILE *fp_conf;
	int fd_conf;
	//char latestAPconf_bak[] = "/mnt/user1/softap/latestAP.conf";
	char cmd[128]={0};
	if(strcmp(selected_filename,"/etc/wireless-tkip.conf")){
		fp_conf = fopen(latestAPconf,"wt");		//open the latestAP.conf or create this file if don't exist

		if(fp_conf == NULL)
		{
			wifidbg_err("open latestAP error\n");
			return -1;
		}
		printf("[Debug Info:%s,%d]:selected_filename == %s\n",__FUNCTION__,__LINE__,selected_filename);
		if(fwrite(selected_filename,sizeof(char),strlen(selected_filename),fp_conf) != strlen(selected_filename))
		{
			wifidbg_err("write latestAP error\n");
			fclose(fp_conf);
			return -1;
		}

		fflush(fp_conf);
		//fd_conf= fileno(fp_conf);
	//	fsync(fd_conf);
		fclose(fp_conf);


		memset(cmd,0,128);
		sprintf(cmd,"cp -f %s %s",latestAPconf,latestAPconf_bak);
		system(cmd);
		
		sync();
		fp_conf = NULL;		
		return 0;
	}
	else{
		wifidbg_err("selected filename is /etc/wireless-tkip.conf\n");
		return -1;
	}
}


void rebak_latestAPconf(){ //for auto connect ap error.
	FILE *fp_conf;
	char cmd[128]={0};
	int ret;
	//char latestAPconf_bak[] = "/mnt/user1/softap/latestAP.conf";
	
	if(access(latestAPconf_bak,F_OK)==0){
		memset(cmd,0,128);
		sprintf(cmd,"cp -f %s %s",latestAPconf_bak,latestAPconf);
		ret=system(cmd);
		printf("[%s][%d]:%s\n",__FILE__,__LINE__,cmd);
		if(ret == 0)
			printf("[%s,%d]recover %s ok!!\n",__FUNCTION__,__LINE__,latestAPconf);
		else
			printf("[%s,%d]recover %s error!!\n",__FUNCTION__,__LINE__,latestAPconf);
		system("sync");
	}
}
void set_wifi_client_mode(){
	char *ip = NULL;
	char *mask = NULL;
	char *gateway = NULL;
	char *dns1 = NULL;
	char *dns2 = NULL;
	int ret =-1;
	char buf[256];
	enum{
		WLANSETTING_IP = 0,
		WLANSETTING_GATEWAY,
		WLANSETTING_MASK,
		WLANSETTING_DNS1,
		WLANSETTING_DNS2
	};

		
	printf("[%s][%d]\n",__FILE__,__LINE__);
	if(0==webset_wifi_getWiFiAutomaticEnable())
	{
		ip=getWiFiSettingVal(WLANSETTING_IP);
		mask=getWiFiSettingVal(WLANSETTING_MASK);
		gateway=getWiFiSettingVal(WLANSETTING_GATEWAY);
		dns1=getWiFiSettingVal(WLANSETTING_DNS1);
		dns2=getWiFiSettingVal(WLANSETTING_DNS2);

		if(access("/tmp/dnsmasq.conf", F_OK)==0){
			unlink("/tmp/dnsmasq.conf");
		}
		system("cp /etc/dnsmasq.conf /tmp/dnsmasq.conf");
		memset(buf, 0, sizeof(buf));


		if(dns1!=NULL&&dns2!=NULL)
		{
			printf("dns1!=NULL&&dns2!=NULL\n");
			setDNS(0,dns1);
			setDNS(1,dns2);
			snprintf(buf, sizeof(buf), "echo server=%s >> /tmp/dnsmasq.conf", dns1);
		}
		else if(dns1==NULL&&dns2!=NULL)
		{
			printf("dns1==NULL");
			setDNS(0,dns2);
					snprintf(buf, sizeof(buf), "echo server=%s >> /tmp/dnsmasq.conf", dns2);
		}
		else if(dns1!=NULL&&dns2==NULL)
		{
			printf("dns2==NULL\n");
			setDNS(0,dns1);
			snprintf(buf, sizeof(buf), "echo server=%s >> /tmp/dnsmasq.conf", dns1);
		}

		system(buf);
		
		ret = ifconfigConnection(ip,mask,gateway);

		if( ret == 0)
		{
			saveManualip(ip,mask,gateway);
			printf("[%s][%d]test\n",__FILE__,__LINE__);
			//wlan_status = WIFI_AUTO_IP_SUCCESS;
		}
	}
	else 
		printf("use aoto ip cpnfig\n");



}
int get_wifi_config_info(){
	int rtn = -1;
#if (!defined(MODULE_CONFIG_3TO1) || MODULE_CONFIG_3TO1!=1)//mirascreen_free do not need
	udhcpd_dns_enable();
#endif
	if(checkIPfileExist() == 1){
		if(connectManualip() == 0){		//connect manualip and enable route
			#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
				modify_wifi_mode(WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK);
			#else
				modify_wifi_mode(WIFI_CLIENT_GETAUTOIP_OK);
			#endif
			save_latestAPconf();
			wlan_status = WIFI_AUTO_IP_SUCCESS;
					
#if ROUTER_ONLY_ENABLE
			if(getRouterOnlyStatus() != 0){
				if(get_ap_close_flag() == 0){
					wifi_softap_close();
				}
			}
#endif

		}else{
			#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
				modify_wifi_mode(WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR);
			#else
				modify_wifi_mode(WIFI_CLIENT_GETAUTOIP_ERR);
			#endif
			wlan_status = WIFI_AUTO_IP_ERROR;
		}
	}
	else if(0==setAutoIPandRoute()){
		wifidbg_info("set ip and route successfully");
		//printf("[debug %s %d] >>>>\n",__func__,__LINE__);	
		wpa_connect_status();
		wlan_status = WIFI_AUTO_IP_SUCCESS;
		//__get_ssid(selected_ap,selected_ssid,MAX_SSID_LEN);
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
#ifndef MODULE_CONFIG_WIFI_BRIDGE_ENABLE

#if ROUTER_ONLY_ENABLE
		if(getRouterOnlyStatus() != 0){
			if(get_ap_close_flag() == 0){
				wifi_softap_close();
			}
		}else
#endif
		{
			if(dongle_get_device_changing()==REALTEK_dongle){
				char *pub_interface=NULL;
				char *private_interface=NULL;
				if(vid_pid_from_driver==0x148f8189){
					pub_interface="wlan2";
					private_interface="wlan3";
				}
				else{
					pub_interface="wlan0";
					private_interface="wlan1";
				}
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
				puts("use ezcastpro subnet 192.168.168.1/24");
				char *private_subnet="192.168.168.1/24";
#else
				char *private_subnet="192.168.203.1/24";
#endif
#else
				char *private_subnet="192.168.111.1/24";
#endif

				//route wlan3 to pub_interface//route wlan1 to pub_interface
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
				//wifiBridgeProcess(WIFI_BRIDGE_ADD_BR1_AND_WLAN1_WLAN3);
				
				enable_route_function(pub_interface,"br0",private_subnet,WIFI_ROUTING);
				printf("[%s][%d]wifiBridgeProcess\n",__func__,__LINE__);

#else		
	            enable_route_function(pub_interface,private_interface,private_subnet,WIFI_ROUTING);
				
#endif		
			}
		}
#endif
			modify_wifi_mode(WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK);
		#else		
			modify_wifi_mode(WIFI_CLIENT_GETAUTOIP_OK);
		#endif

		save_latestAPconf();

		rtn = 0;
		//start_thttpd_process();

	}
	else{
		wifidbg_err("set ip and route failed");
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE				
			modify_wifi_mode(WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR);
	
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE
			if(vid_pid_from_driver==0x148f8189)
				wifiBridgeProcess(WIFI_BRIDGE_DEL_WLAN3);
			else
				wifiBridgeProcess(WIFI_BRIDGE_DEL_WLAN1);
			//wifi_reset();
#endif
#else		
			modify_wifi_mode(WIFI_CLIENT_GETAUTOIP_ERR);
#endif
		//confirm_ap_info();
//#ifndef MODULE_CONFIG_WIFI_BRIDGE_ENABLE

		wlan_status = WIFI_AUTO_IP_ERROR;
//#endif
	}	
	return 0;
	
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
int wifi_freqz_mapping(int frqz)
{
	int i=0;
	int ch_arr_24G[]={12,17,22,27,32,37,42,47,52,57,62,67,72};
	frqz-=2400;
	for(i=0;i<sizeof(ch_arr_24G)/sizeof(int);i++){
		if(frqz==ch_arr_24G[i])break;
	}	
	i++;
	debug_info("input frqz==%d, output==%d",frqz+2400,i);	
	return i;
}
void ezcastpro_host_reset()
{
	debug_info("call ezFuiWifiRemoteReset!!");
	ezFuiWifiRemoteReset();
}
void ezcastpro_HostCleanWithChannel()
{
	int softap_ori_channel=0;
	char tmp_buf[2]={0};
	char call_path[100]="";
	struct wifi_info_t *wifi_info = __get_wifiinfo();	
	int client_channel=wifi_freqz_mapping(wifi_info->ap_entry[selected_ap].frequency);
	
	debug_info("sel_ap==%d,ssid==%s,ap_ch==%d,rec_ch==%d",selected_ap,wifi_info->ap_entry[selected_ap].ssid,
				bestChannelChosen,concurrent_ch_record);//richardyu 070914
	while(softap_ori_channel==0){
		FILE* tmp_fp=NULL;
		if(vid_pid_from_driver==0x148f8189)
		{
			tmp_fp=popen("cat /mnt/user1/softap/rtl_hostapd_02.conf | grep channel | awk -F= '$2{print$2}'","r");
		}
		else
		{
			sprintf(call_path,"cat %s | grep channel | awk -F= '$2{print$2}'",HOSTAPD_CON_FILE);
			tmp_fp=popen(call_path,"r");
		}
		if(tmp_fp==NULL){
			puts("get softap_ori_channel fail 1");
			sleep(1);
			continue;
		}else{
			int tmp_rtn=fread(tmp_buf,1,2,tmp_fp);
			if(tmp_rtn>0){
				softap_ori_channel=(tmp_rtn==1)?tmp_buf[0]-48:tmp_buf[1]-48+10;
				printf("read %d bytes, they are %d,%d, softap_ori_channel==%d\n",tmp_rtn,tmp_buf[0],
				tmp_buf[1],softap_ori_channel);					
			}
			pclose(tmp_fp);
		}
	}
	if(concurrent_ch_record==0){
		concurrent_ch_record=softap_ori_channel;
	}
	if(concurrent_ch_record!=client_channel){
		ezcastpro_host_reset();
		concurrent_ch_record=client_channel;							
	}
}
#endif
int wifi_status_check()
{
	int rtn = -1;
	int error_num=0;
	struct timespec time_escape;
	int sleep_rtn=0;
	time_escape.tv_sec = 1;
	time_escape.tv_nsec = 0;
	//int has_find_confile=0;
	char cmd[128]={0};
	int num_count ;
	if(wps_flag == 0xa0){
		num_count = 60;
	}else{
		num_count = 19;
	}
	if(read_wifi_mode()!=WIFI_DONGLE_PLUG_OUT){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		modify_wifi_mode(WIFI_CONCURRENT_CLIENT_ENABLE);
#else
		modify_wifi_mode(WIFI_CLIENT_MODE_ENABLE);
#endif
	}
	while(1){
		//printf("%s,%d\n",__FUNCTION__,__LINE__);
		//printf("[debug %s %d] >>>>\n",__func__,__LINE__);	
		wlan_status = wpa_connect_status();
		//printf("wlan_status====================================%d\n",wlan_status);
		if(wlan_status == WIFI_COMPLETED){
			debug_info("connect completely!");
			
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE!=0)
			set_wifi_client_mode();
#endif
			get_wifi_config_info();
			
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	printf("Probox double wifi switch ,clear 24G/5G conf file!");
	if(vid_pid_from_driver==0x148f8189){
		system("rm -rf /mnt/vram/wifi/5G/*");
		wpa24g_connected=1;		
		}
	else{
		system("rm -rf /mnt/vram/wifi/24G/*");	
		wpa5g_connected=1;	
		system("ifconfig wlan2 down");
	}
#endif

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0&& MODULE_CONFIG_EZCASTPRO_MODE!=8075
			ezcastpro_HostCleanWithChannel();
#endif			
			break;
		
		}
		else{
		GO_SLEEP:
/*
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
			
			modify_wifi_mode(WIFI_CONCURRENT_CLIENT_ENABLE);
#else
			modify_wifi_mode(WIFI_CLIENT_MODE_ENABLE);
#endif
*/
			sleep_rtn = nanosleep(&time_escape,NULL);
			error_num++;
			printf("error num is %d\n",error_num);
			#if EZMUSIC_ENABLE
			if(1==SysCGI_priv.play_connect_router_waiting_end)
			{
				SysCGI_priv.connect_router_waiting_flag=1;
				SysCGI_priv.play_connect_router_waiting_end=0;
			}
			#endif
			if(error_num>num_count){
				wlan_status = WIFI_CONNECT_FAILED;		//connect failed
				#if EZMUSIC_ENABLE
				printf("connect_router_failed======%s,%d\n",__FUNCTION__,__LINE__);
				SysCGI_priv.connect_router_failed_flag=1;
				SysCGI_priv.not_connected_router_useful_flag=1;
				#endif
				wifidbg_err("cannot connect ap!   selected_filename====%s",selected_filename);
				if(strcmp(selected_filename,"/etc/wireless-tkip.conf")!=0&&strlen(selected_filename)&&access(selected_filename,F_OK)==0){
#if EZCAST_ENABLE		// EZCast do not remove password while auto connect fail.
					int rmConfFlag = 1;
					if(autoConnectInfo.flag != 0){
						if(strcmp(selected_filename, autoConnectInfo.selected_filename) == 0){
							EZCASTLOG("WiFi config[%s] not remove\n", selected_filename);
							rmConfFlag = 0;
						}
						autoConnectInfo.flag = 0;
						memset(autoConnectInfo.selected_filename, 0, sizeof(autoConnectInfo.selected_filename));
					}
					if(rmConfFlag != 0){
						remove(selected_filename);
						system("sync");
					}
#else
					remove(selected_filename);
					system("sync");
#endif
#if 0					
					struct wifi_info_t *wifi_info = __get_wifiinfo();
					int i=0;
					for(i=0;i<wifi_info->AP_num;i++){
						if(check_conf_exist(wifi_info->ap_entry[i].ssid)==1){
							sprintf(selected_filename,"/mnt/vram/wifi/%x.conf",APHash(wifi_info->ap_entry[i].ssid));
							selected_ap = i;
							has_find_confile=1;
							
							wifi_send_msg(wifi_info,WIFI_CMD_CONNECT);
							break;
						}
					}
#endif
				}
				OSSleep(1000);
				wifi_reset();
				wps_flag = 0xa1;
				error_num=0;
				#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1)
				printf("\n\n-----------------wifi connect failed-----------------\n\n");
                SWF_Message(NULL, SWF_MSG_KEY_WIFI_FAILED, NULL);//added by Denny 
                #endif
				break;
			}
		}
	}
	return rtn;
}
static int wpa_connect_ap()
{
	int ret= 0;
	char *device=NULL;

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
	if(strstr(selected_filename,"/mnt/vram/wifi/24G/"))
		vid_pid_from_driver=0x148f8189;
	else
		vid_pid_from_driver=vid_pid_from_driver_firstin;//5g

	printf("[%s][%d]selected_filename=%s vid_pid_from_driver=%d \n",__func__,__LINE__,selected_filename,vid_pid_from_driver);
#endif
	kill_wpa_supplicant();
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
	//if(vid_pid_from_driver==0x148f8189)//use 24g to connect 
		//system("wpa2_supplicant -B -i wlan2 -c /etc/wireless-tkip.conf");	
	//else
	//	system("wpa_supplicant -B -i wlan0 -c /etc/wireless-tkip.conf");
		

	//openCtrlConnection  must keep  wpa_supplicant wlanx  up, otherwise,  open ctrl error
#endif

	device = dongle_get_device();
	if(device==NULL){
		return -1;
	}

	ret = wlan_up(device);
	if(ret!=0)
	{
		wifidbg_err("ifconfig %s up is error!",device);
		return ret;
	}
	ret = wlan_conf(device,selected_filename);
	
	if(ret!=0)
	{
		wifidbg_err("wpa_supplicant conf is error!");
		return ret;
	}

	if(openCtrlConnection()!=0)
	{
		wifidbg_err("open ctrl connectiong is error!");
		ret = -1;
	}
	return ret;
}


static int __delete_ap_info()
{
	int ret=0;
	int i=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		return -1;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL){
		ret=-1;goto __end;
	}

	for(i=0;i<wifi_info->AP_num;i++){
		wifi_info->ap_entry[i].isvalid = 0;
	}
	wifi_info->AP_num = 0;

__end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}

int delete_APInfo()
{
	int i;
	return __delete_ap_info();
}


static int __get_selap_idx(char *sel_ssid)
{

	int ret=0;
	int i=0;
	struct wifi_info_t *wifi_info = __get_wifiinfo();

	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

		return -1;
	}
	
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ssid==NULL){
		fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

		ret=-1;goto __end;
	}
	
	//fprintf(stderr,"function:%s line:%d sel_ssid:%s wlan_status=%d\n",__FUNCTION__,__LINE__,sel_ssid,wlan_status);
	for(i=0;i<wifi_info->AP_num;i++){
		//printf("sel_ssid:%s,scan_ssid=%s\n",sel_ssid,wifi_info->ap_entry[i].ssid);
		if(strcmp(sel_ssid,wifi_info->ap_entry[i].ssid)==0&&(wlan_status==WIFI_COMPLETED||wlan_status==WIFI_AUTO_IP_ERROR||wlan_status==WIFI_AUTO_IP_SUCCESS))
			break;
	}
	
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	if(i!=wifi_info->AP_num)
		ret = i;
	else 
		ret=-1;
__end:
	
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return ret;
}
char ssid_connecting_tmp[MAX_SSID_LEN];

int indexofselectAP()
{
	
	char *ssid_temp=NULL;
if(access("/tmp/wpa_supplicant/wlan0",F_OK)==0){
//	printf("[debug %s %d] >>>>\n",__func__,__LINE__);	
	//if(wpa_connect_status()!=WIFI_COMPLETED)
	if(wlan_current_status.current_status_value!=WIFI_COMPLETED)
	{
		return -1;
	}
}
	
if(access("/tmp/wpa_supplicant/ra0",F_OK)==0){
	if(wlan_current_status.current_status_value!=WIFI_COMPLETED){
		return -1;
	}
}
#if 0	
if(wps_flag == 0xa0){	
	ssid_temp=wifi_conf_content_for_display("ssid=\"");
	if(ssid_temp&&strcmp(wlan_current_status.ssid,ssid_temp)!=0){
		
		return -1;
	}
}	
#endif
	return __get_selap_idx(wlan_current_status.ssid);
}

static int __digital_transfer(char ch)
{
	if(ch >= '0' && ch <= '9'){
		return ch - '0';
	}

	if(ch >= 'a' && ch <= 'f'){
		return (ch - 'a')+10;
	}

	if(ch >= 'A' && ch <= 'F'){
		return (ch - 'A')+10;
	}

	return 0;
}
#if 0
static transfer_gateway_info(char *ptr ,char *transfer_result){
	
	int p1,p2,p3,p4;
	
	p1 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
	ptr+=2;
	p2 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
	ptr+=2;
	p3 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
	ptr+=2;
	p4 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
	
	sprintf(transfer_result,"%d.%d.%d.%d",p4,p3,p2,p1);
	
	printf("[%s,%d] transfer_result ======== %s\n",__FUNCTION__,__LINE__,transfer_result);
}
static int __get_kernel_gateway_info(char *gateway,char *iface){
	char gateway_code[8] = {0};
	char search_location_begin[10] = {0};
	
	snprintf(search_location_begin,"%s   ",iface,strlen(iface));
	
	get_info_from_file("/proc/net/route",search_location_begin," ",gateway_code);
	
	printf("[%s,%d] gateway_code ======== %s\n",__FUNCTION__,__LINE__,gateway_code);
	transfer_gateway_info(gateway_code,gateway);
}
#endif

#if 1
int __get_kernel_gateway_info(char *gateway,char *iface)
{
	FILE *fp;
	char buf[256];
	char *ptr,*ptr2;
	int p1,p2,p3,p4;
	int cnt=0;
	int length_begin_location = 16;
	if((gateway == NULL) || (iface==NULL)){
		
		printf("[%s,%d]  gateway == NULL || (iface==NULL\n",__FUNCTION__,__LINE__);
		return -1;
	}

	fp = fopen("/proc/net/route","r");
	if(fp == NULL){
		printf("[%s,%d]  cann't open /proc/net/route\n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	int get_kernel_gateway_info_times=0;
	
	while(!feof(fp)){
		memset(buf,0,sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		get_kernel_gateway_info_times++;
		
		//printf("[%s,%d] get_kernel_gateway_info_times ======== %d\n",__FUNCTION__,__LINE__,get_kernel_gateway_info_times);
		if(get_kernel_gateway_info_times>10)
			break;
		//printf("[%s,%d] buf ======== %s\n",__FUNCTION__,__LINE__,buf);
		//printf("[%s,%d] iface ======== %s\n",__FUNCTION__,__LINE__,iface);
#if 0
		if((ptr = strstr(buf,"Iface"))!= NULL){
			int length1 = strlen(ptr) ;
			
			printf("[%s,%d] length1 ======== %d\n",__FUNCTION__,__LINE__,length1);
			char *temp_ptr = strstr(ptr,"Gateway");
			
			if(temp_ptr){
				int length2 = strlen(temp_ptr);
				length_begin_location = length1 - length2 ;
				printf("length_begin_location ============== %d\n" ,length_begin_location );
			}
			else{
				printf(" error in /proc/net/route\n ");
				break ;
			}
		}
#endif
		if((ptr = strstr(buf,iface))!=NULL){
#if 1			
			//printf("[%s,%d] ptr ======== %s\n",__FUNCTION__,__LINE__,ptr);
			while((*ptr!=' ') && (*ptr!='\t')) 
				ptr++;
			//printf("[%s,%d] ptr ======== %s\n",__FUNCTION__,__LINE__,ptr);
			while((*ptr==' ') || (*ptr=='\t')) 
				ptr++;
			
			//printf("[%s,%d] ptr ======== %s\n",__FUNCTION__,__LINE__,ptr);
			/** Destination*/
			while(*ptr == '0') 
				ptr++;
			
			//printf("[%s,%d] ptr ======== %s\n",__FUNCTION__,__LINE__,ptr);
			if((*ptr != ' ') && (*ptr != '\t')) continue;

			/** gateway */
			while((*ptr==' ') || (*ptr=='\t')) ptr++;
			ptr2 = ptr;
			while((*ptr2!=' ') && (*ptr2!='\t')){ 
				ptr2++;
				cnt++;
			}
			
			//printf("[%s,%d] cnt ======== %d\n",__FUNCTION__,__LINE__,cnt);
			if(cnt != 8){
				break;
			}
#endif		
#if 0			
			int i =0 ;
			ptr += length_begin_location - 1;
			//printf("ptr ================ %c \n",*ptr);
			
			//printf("ptr ================ %s \n",ptr);
			for(i = 0;i < 8;i++){
				//printf("i ==== %d\n",i);
				if(*(ptr+i) != '0')
					break;
				
			}
			//printf("i end ==== %d\n",i);
			if(i == 8)
				continue;
#endif			
			//printf("ptr ================ %c \n",*ptr);
			p1 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
			ptr+=2;
			p2 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
			ptr+=2;
			p3 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
			ptr+=2;
			p4 = __digital_transfer(*ptr)*16 + __digital_transfer(*(ptr+1));
			ptr+=2;
			
			sprintf(gateway,"%d.%d.%d.%d",p4,p3,p2,p1);
			
			printf("[%s,%d] gateway ======== %s\n",__FUNCTION__,__LINE__,gateway);
			break;
		}
	}

	fclose(fp);
	return 0;
}
#endif
static int __get_dns_information(char *dns_serv)
{
	FILE *fp;
	char buf[256];
	char *ptr,*ptr2;

	if(dns_serv == NULL){
		return -1;
	}

	ptr2 = dns_serv;

	fp = fopen("/etc/resolv.conf","r");
	if(fp == NULL){
		return -1;
	}
	int get_dns_information_times=0;

	while(fgets(buf, sizeof(buf), fp) != NULL){
		get_dns_information_times++;
		if(get_dns_information_times>10)
			break;
		if((ptr = strstr(buf,"nameserver"))!=NULL){
			ptr += 10;
			while((*ptr==' ') || (*ptr=='\t')) ptr++;
			while((*ptr == '.') || ((*ptr >= '0') && (*ptr <= '9'))){
				*ptr2 = *ptr;
				ptr++;
				ptr2++;
			}
			*ptr2 = ' ';
			ptr2++;
		}
	}
	*ptr2 = 0;

	fclose(fp);
	return 0;
}
char route_for_wifidisplay_valid[18]={0};


#define IFNAME_MAX_LEGTH 16
#define IP_MAX_LEGTH 16

char * getIpAddressByIoctl(char *interfaceName){

	int inet_sock;

    struct ifreq ifr;
	char *ipaddr = calloc(IP_MAX_LEGTH,1);


	if(interfaceName == NULL){
		printf("[%s-%d] interfaceName error !\n",__FILE__,__LINE__);
		return NULL;

	}

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0); 

	if(inet_sock < 0){
		
		printf("[%s-%d] socket error !\n",__FILE__,__LINE__);
		return NULL;
	}


	strncpy(ifr.ifr_name, interfaceName, IFNAME_MAX_LEGTH - 1);


    if (ioctl(inet_sock, SIOCGIFADDR, &ifr) ==  0){  
    	snprintf( ipaddr, IP_MAX_LEGTH,"%s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
		printf("ipaddr ======== %s\n",ipaddr);
		return ipaddr;
	}

	return NULL;
}
int getNetRunningFlagByIoctl(char *interfaceName){
#if 0

	int inet_sock;
	int netRunningFlag = -1;
    struct ifreq ifr;


	if(interfaceName == NULL){
		printf("[%s-%d] interfaceName error !\n",__FILE__,__LINE__);
		return netRunningFlag;

	}

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0); 

	if(inet_sock < 0){
		
		printf("[%s-%d] socket error !\n",__FILE__,__LINE__);
		return netRunningFlag;
	}


	strncpy(ifr.ifr_name, interfaceName, IFNAME_MAX_LEGTH - 1);
	printf("ifr.ifr_name ====== %s\n",ifr.ifr_name);


    if (ioctl(inet_sock, SIOCGIFFLAGS, (caddr_t)&ifr) ==  0){  
		netRunningFlag = ifr.ifr_flags;
		printf("netRunningFlag ====== 0x%x\n",netRunningFlag);
		return netRunningFlag;
	}
	
	return netRunningFlag;
#endif
    return 0Xff;
}


static int judge_get_ip_successfully(){
	
#ifndef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
	int count_get_ip = 0;
	int ret = -1; 
	while(1){
		if(count_get_ip > 10)
			break;
		wpa_connect_status();
		
		if(strlen(wlan_current_status.ip_address)==0){
			OSSleep(1000);
			count_get_ip ++;
			continue;
		}
		else{
			count_get_ip = 0;
			ret = 0 ;
			printf("wlan_current_status.ip_address  ========== %s\n",wlan_current_status.ip_address);
			break;
		}
	}
	return ret;
#else
	int ret = -1;
	char *brIpAddress = NULL;
	brIpAddress = getIpAddressByIoctl("br0");
	if(brIpAddress && strlen(brIpAddress) > strlen("0.0.0.0"))
		ret = 0;
	
	if(brIpAddress){
		free(brIpAddress);
		brIpAddress = NULL;
	}
	return ret;
#endif
}
static int getAutoIP()
{
	char callbuf[WIFI_BUF_LEN]={0};
	char buf[512] = {0};
	char * tmpbuf=NULL;
	char *device=NULL;
	FILE *fp=NULL;
	int discover_time = 0;
	int DNS_length = 0;
	int rtn = 0;
	int close_times=0;
	auto_route_has_get=0;
	device = dongle_get_device();
	if(device==NULL){
		return -1;
	}

	/** kill the previous udhcpc process */
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);

	/** run the current udhcpc process */
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
	wifiBridgeProcess(WIFI_BRIDGE_ENABLE_BR0);
#else
	sprintf(callbuf,"udhcpc -i %s -t 10 -n",device);
	printf("the call is %s\n",callbuf);
	
#endif
	system(callbuf);

	/** after running, kill it*/
	

	/** 
	* get and save the router and DNS server.
	* Use other method to get the related information.
	*/
	memset(wlan_current_status.router,0,sizeof(wlan_current_status.router));
	memset(wlan_current_status.DNS_server,0,sizeof(wlan_current_status.DNS_server));
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
	
	__get_kernel_gateway_info(wlan_current_status.router,"br0");
#else
	__get_kernel_gateway_info(wlan_current_status.router,device);
#endif
	__get_dns_information(wlan_current_status.DNS_server);
	memset(route_for_wifidisplay_valid,0,18);
	memcpy(route_for_wifidisplay_valid,wlan_current_status.router,strlen(wlan_current_status.router));
	printf("wlan_current_status.router=======%s\n",wlan_current_status.router);
	printf("wlan_current_status.DNS_server=======%s\n",wlan_current_status.DNS_server);
	//sprintf(callbuf,"killall udhcpc");
//	system(callbuf);
	init_wifi_wlan_current_status();
	
	rtn = judge_get_ip_successfully();

	//if(rtn==-1){
printf("debug %s %d++++++++++++++++\n",__func__,__LINE__);
	sprintf(callbuf,"killall udhcpc\n");
	system(callbuf);
//	}
	//remove("/mnt/vram/wifi/resolv.conf");
	return rtn;
}

void wifi_find_infomation(char *find_buf,char *key,char *store_buf){
	if(find_buf==NULL)
		return;
	char *locate1=strstr(find_buf,key);
	if(locate1!=NULL){
		locate1+=strlen(key);
		char *locate1_tmp=locate1;
		char *locate2=strstr(locate1,"\n");
		if(locate2!=locate1_tmp){
			memcpy(store_buf,locate1_tmp,strlen(locate1_tmp)-strlen(locate2));
			printf("store_buf======%s\n",store_buf);
		}
	}
	
}

static int getAutoIP_bak()
{
	char callbuf[WIFI_BUF_LEN]={0};
	char buf[2048] = {0};
	char * tmpbuf=NULL;
	char *device=NULL;
	FILE *fp=NULL;
	int discover_time = 0;
	int DNS_length = 0;
	int rtn = 0;
	int close_times=0;
	device = dongle_get_device();
	if(device==NULL)
		return -1;
	
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
	sprintf(callbuf,"udhcpc -b -i %s",device);
	system(callbuf);
	printf("the call is %s\n",callbuf);	
	fp = popen(callbuf, "r");
	

	
    if(NULL == fp)
    {
    	fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);

        wifidbg_err("popen error:%s\n",strerror(errno));
        return -1;
    }
	memset(wlan_current_status.router,0,sizeof(wlan_current_status.router));
	memset(wlan_current_status.DNS_server,0,sizeof(wlan_current_status.DNS_server));
	fread(buf,1,2048,fp);
	wifi_find_infomation(buf,"router is ",wlan_current_status.router);
	
	wifi_find_infomation(buf,"dns is ",wlan_current_status.DNS_server);
	
/*	while(fgets(buf, sizeof(buf), fp) != NULL)
    {
    	
        tmpbuf = buf;
		
		
		
        while(*tmpbuf && isspace(*tmpbuf))
            ++ tmpbuf;
		
    	
		if(strncmp(tmpbuf, "Sending discover...", strlen("Sending discover...")) == 0)
			discover_time++;
		
		if(discover_time>1000){
			wifidbg_err("discover_time > 1000");
			 fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);

			rtn = -1;
			break;
		}
		
    	
        if(strncmp(tmpbuf, "router is", strlen("router is")) == 0){
			tmpbuf+=strlen("router is ");
			printf("tmpbuf is %s\n",tmpbuf);
			memcpy(wlan_current_status.router,tmpbuf,strlen(tmpbuf)-1);
        }
		
		if(strncmp(tmpbuf, "dns is", strlen("dns is")) == 0){
			tmpbuf+=strlen("dns is ");
			printf("buf is %s,len is %d,DNS len is %d\n",tmpbuf,strlen(tmpbuf),DNS_length);
			memcpy(wlan_current_status.DNS_server+DNS_length,tmpbuf,strlen(tmpbuf)-1);
			DNS_length += strlen(tmpbuf);
			wlan_current_status.DNS_server[DNS_length-1] = ' ';
		}
		
    }

    */
	sprintf(callbuf,"killall udhcpc");
	system(callbuf);
  	rtn = pclose(fp);
	//printf("wlan_current_status.router=======%s\n",wlan_current_status.router);
	memset(route_for_wifidisplay_valid,0,18);
	memcpy(route_for_wifidisplay_valid,wlan_current_status.router,strlen(wlan_current_status.router));
	//if(strlen(wlan_current_status.router)==0||strlen(wlan_current_status.DNS_server)==0)
	//	rtn=-1;

	//fprintf(stderr,"function:%s line:%d rtn:%d\n",__FUNCTION__,__LINE__,rtn);

	return rtn;
}

int setAutoIPandRoute()
{
	if(getAutoIP()==0){
		printf("get auto ip address successfully\n");
	}
	else{
		printf("get auto ip address failed\n");
		return -1;
	}
	return 0;
}

int setIP()
{
	char callbuf[WIFI_BUF_LEN]={0};
	char *device=NULL;
	device = dongle_get_device();
	if(device == NULL)
		return -1;
	else
		sprintf(callbuf,"ifconfig %s %s",device,wlan_current_status.ip_address);
	
	wifidbg_info("the call is %s",callbuf);
	return system(callbuf);
}

int setRoute()
{
	char callbuf[WIFI_BUF_LEN]={0};

	sprintf(callbuf,"route add default gw %s",wlan_current_status.router);
	wifidbg_info("the call is %s",callbuf);
	return system(callbuf);
}

int setDNS(int index,char *DNS_server)
{
	FILE * fp = NULL;
	int fd = 0;
	char writebuf[50] = {0};
	int rtn = 0;
	if(index==0){
		fp = fopen("/mnt/vram/wifi/resolv.conf","wb+");
	}
	else{
		fp = fopen("/mnt/vram/wifi/resolv.conf","a+");	
	}
	if(fp==NULL){
		wifidbg_err("open resolv.conf error!");
		rtn =  -1;
	}
	sprintf(writebuf,"nameserver %s\n",DNS_server);
	if(fwrite(writebuf, strlen(writebuf), 1,fp)!=1){
		wifidbg_err("Write DNS server Error!");
		rtn =  -1;
		goto setDNS_out;
	}
	fflush(fp);
	fd = fileno(fp);
	if(fsync(fd)==-1){
		wifidbg_err("flush Error!\n");
		rtn =  -1;
		goto setDNS_out;
	}
setDNS_out:
	fclose(fp);
	return 0;
}

static int wifi_req_enqueue(wifi_info *wifi_data,wifi_ioctrl * req){
	int i=0,rtn,tmp_idx;
	if(wifi_data==NULL)
		return -1;
	wifi_sem_wait(&wifi_data->syn_lock.semi_req);
	for(i=0;i<WIFI_IOARRAY_LEN;i++){
		/**search the positon to insert the request,**/
		tmp_idx = (wifi_data->req_start_idx+i)%WIFI_IOARRAY_LEN;
		if(wifi_cmd_array[tmp_idx].active==WIFI_REQ_INVALID){
				break;
		}
	}
	if(i>=WIFI_IOARRAY_LEN){
		wifidbg_err("Sorry,the Queue is full");
		rtn = -1;
	}
	else{
		memcpy(wifi_cmd_array+tmp_idx,req,sizeof(wifi_ioctrl));
		wifi_cmd_array[tmp_idx].active = WIFI_REQ_ENQUEUE;
		wifidbg_info("REQ: ENQUEUE idx===%d\n",tmp_idx);
		wifi_data->req_start_idx = tmp_idx;
		rtn  = tmp_idx;
	}
	wifi_sem_post(&wifi_data->syn_lock.semi_req);
	return rtn;
}

static int wifi_req_dequeue(wifi_info *wifi_data,wifi_ioctrl * req){
	int i=0,rtn;
	if(wifi_data==NULL)
		return -1;
	memset(req,0,sizeof(wifi_ioctrl));
	wifi_sem_wait(&wifi_data->syn_lock.semi_req);
	
	for(i=0;i<WIFI_IOARRAY_LEN;i++){
		if(wifi_cmd_array[i].active==WIFI_REQ_ENQUEUE){
			break;
		}
	}

	
	if(i>=WIFI_IOARRAY_LEN){
		rtn = -1;
	}
	else{
		memcpy(req,wifi_cmd_array+i,sizeof(wifi_ioctrl));
		wifi_cmd_array[i].active = WIFI_REQ_DEQUEUE;
		rtn  = i;
	}
	wifi_sem_post(&wifi_data->syn_lock.semi_req);
	return rtn;
}

static int  wifi_get_msg(wifi_info *wifi_data,wifi_ioctrl * req)
{
	int rtn=0;
	rtn = wifi_req_dequeue(wifi_data, req);
	return rtn;
}

int wifi_req_init_queue(wifi_info *wifi_data)
{
	if(wifi_data==NULL)
		return -1;
	wifi_data->req_start_idx = 0;
	memset(wifi_cmd_array,0,sizeof(wifi_ioctrl)*WIFI_IOARRAY_LEN);
	return 0;
}

static int wifi_req_done(wifi_info *wifi_data,int req_idx)
{
	int rtn=0;
	if(wifi_data==NULL)
		return -1;
	wifi_sem_wait(&wifi_data->syn_lock.semi_req);
	if(wifi_cmd_array[req_idx].active==WIFI_REQ_DEQUEUE){
		wifidbg_info("REQ DONE idx=%d\n",req_idx);
		wifi_cmd_array[req_idx].active = WIFI_REQ_DONE;
		wifi_cmd_array[req_idx].active = WIFI_REQ_INVALID;
	}
	else
		rtn = -1;
	wifi_sem_post(&wifi_data->syn_lock.semi_req);
	return rtn;
}

static void  *wifi_thread(void *arg)
{	
	int rtn = 0;
	wifi_info *wifi_data = (wifi_info *)arg;
	wifi_ioctrl req;
	int wpa_cli_value = 0;
	if(wifi_data==NULL){
		wifidbg_err("Thread wifi_data=NULL\n");
		return NULL;
	}
	
	while(1){
		//wifidbg_err("Wait Semi Start----------\n");
		wifi_sem_wait(&wifi_data->syn_lock.semi_start);
		pthread_testcancel();
		rtn = wifi_get_msg(wifi_data,&req);
		//wifidbg_err("Get Msg idx=%d\n",rtn);
		//printf("req.para_size=============%d\n",req.para_size);
		
		//printf("sizeof(wifi_info)=============%d\n",sizeof(wifi_info));
		//wifidbg_info("req.iocmd === %d\n",req.iocmd);
		if(rtn>=0){
			switch(req.iocmd){
				case WIFI_CMD_SCAN:
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

					printf("%s,%d vid_pid_from_driver=vid_pid_from_driver_firstin  usre 5g to scan wifi list\n",__FUNCTION__,__LINE__);
					vid_pid_from_driver=vid_pid_from_driver_firstin;//usre 5g to scan
#endif
					//wifidbg_err("WIFI_CMD_SCAN");
					if(req.para_size == sizeof(wifi_info)){
						sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
					//	printf("%s,%d	scanning~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n",__FUNCTION__,__LINE__);
						wifi_sem_wait(&wifi_data->semi_wpa_cli);
						//printf("%s,%d\n",__FUNCTION__,__LINE__);
						sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
						wpa_scan_ap();
						//printf("%s,%d\n",__FUNCTION__,__LINE__);
						wifi_scan_check();
						//printf("%s,%d\n",__FUNCTION__,__LINE__);
						wifi_sem_post(&wifi_data->semi_wpa_cli);
						//printf("%s,%d\n",__FUNCTION__,__LINE__);
						sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
						//printf("%s,%d\n",__FUNCTION__,__LINE__);
						//wifidbg_err("3########semi wpa cli value is %d\n",wpa_cli_value);
						
					}
					break;
				case WIFI_CMD_STOPSCAN:
					//wifidbg_err("WIFI_CMD_STOPSCAN");
					break;
				case WIFI_CMD_CONNECT:
					wifidbg_err("WIFI_CMD_CONNECT");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
					wpa_updatetime();
#endif
					if(req.para_size == sizeof(wifi_info)){
						sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
						//wifidbg_err("4########semi wpa cli value is %d\n",wpa_cli_value);
						wifi_sem_wait(&wifi_data->semi_wpa_cli);
						connect_sem_flag = 1;
						sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
						//wifidbg_err("5########semi wpa cli value is %d\n",wpa_cli_value);
						memset(selected_ssid,0,strlen(selected_ssid));
						init_wifi_wlan_current_status();
						//printf("@@@@@@@@@@@@@@@@@@@@@@@selected_ssid===========%s\n",selected_ssid);
						if(wpa_connect_ap()){
							wifi_reset();
							connect_sem_flag = 0;
							wifi_sem_post(&wifi_data->semi_wpa_cli);	
							break;
						}
						wifi_status_check();
										
						connect_sem_flag = 0;
						wifi_sem_post(&wifi_data->semi_wpa_cli);						
						sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
						//wifidbg_err("6########semi wpa cli value is %d\n",wpa_cli_value);
					}
					break;
				case WIFI_CMD_STOPCONNECT:
					wifidbg_err("WIFI_CMD_STOPCONNECT");
					break;
				default:
					wifidbg_err("Not Supprot yet CMD=%d\n",req.iocmd);
					break;
			}
			wifi_req_done(wifi_data,rtn);
		}
	}
	pthread_exit((void*)rtn);
	return NULL;
}

int wifi_create_thread(wifi_info * wifi_data)
{
	int rtn = 0;
	pthread_t  tid;
	int arg=0;

	if(sem_init(&wifi_data->syn_lock.semi_start,0,0)==-1){
		wifidbg_err("Sem init error");
		goto wifi_create_thread_out;
	}

	if(sem_init(&wifi_data->syn_lock.semi_end,0,0)==-1){
		sem_destroy(&wifi_data->syn_lock.semi_start);
		wifidbg_err("Sem init error");
		goto wifi_create_thread_out;
	}

	if(sem_init(&wifi_data->syn_lock.semi_req,0,0)==-1){
		sem_destroy(&wifi_data->syn_lock.semi_start);
		sem_destroy(&wifi_data->syn_lock.semi_end);
		wifidbg_err("Sem init error");
		goto wifi_create_thread_out;
	}


	if(sem_init(&wifi_data->syn_lock.semi_apentry,0,1)==-1){
		sem_destroy(&wifi_data->syn_lock.semi_start);
		sem_destroy(&wifi_data->syn_lock.semi_end);
		sem_destroy(&wifi_data->syn_lock.semi_req);
		wifidbg_err("Sem init error");
		goto wifi_create_thread_out;
	}

	rtn = pthread_create(&tid, NULL,wifi_thread,wifi_data);

	if(rtn){
		wifidbg_err("Create wifi thread error!");
		sem_destroy(&wifi_data->syn_lock.semi_start);
		sem_destroy(&wifi_data->syn_lock.semi_end);
		sem_destroy(&wifi_data->syn_lock.semi_req);
		sem_destroy(&wifi_data->syn_lock.semi_apentry);
		goto wifi_create_thread_out;
	}
	wifi_sem_post(&wifi_data->syn_lock.semi_req);
	
	wifi_data->thread_id = tid;
	wifi_data->is_thread_run = 1;
	
wifi_create_thread_out:
	return rtn;
}

int wifi_thread_exit(wifi_info * wifi_data)
{
	void * thread_ret=NULL;
	if(wifi_data && wifi_data->is_thread_run){
		pthread_cancel(wifi_data->thread_id);
		pthread_join(wifi_data->thread_id,&thread_ret);

		sem_destroy(&wifi_data->syn_lock.semi_start);
		sem_destroy(&wifi_data->syn_lock.semi_end);
		sem_destroy(&wifi_data->syn_lock.semi_req);
		sem_destroy(&wifi_data->syn_lock.semi_apentry);
		//sem_destroy(&wifi_data->syn_lock.semi_ctrl_operation);
		wifi_data->is_thread_run = 0;
	}
	return 0;
}

static unsigned long wifi_get_timestamp()
{
	struct timeval cur_time;
	if(gettimeofday(&cur_time,NULL)==0)	
		return cur_time.tv_sec*1000000L+cur_time.tv_usec;
	else{
		if(wifi_timestamp>=0xff)
			wifi_timestamp = 1;
		wifi_timestamp++;
		return wifi_timestamp;
	}
}

int wifi_send_msg(wifi_info *wifi_data,wifi_ioctl_cmd cmd)
{
	int rtn = 0;
	int req_idx=0;
	wifi_ioctrl req;
	req.iocmd = cmd;
	switch(cmd){
		case WIFI_CMD_SCAN:
			{
				wifidbg_info("WIFI_CMD_SCAN");
				req.para_size = sizeof(wifi_info);
				break;
			}
		case WIFI_CMD_STOPSCAN:
			{
				wifidbg_info("WIFI_CMD_STOPSCAN");
				break;
			}
		case WIFI_CMD_CONNECT:
			{
				wifidbg_info("WIFI_CMD_CONNECT");
				req.para_size = sizeof(wifi_info);
				break;
			}
		case WIFI_CMD_STOPCONNECT:
			{
				wifidbg_info("WIFI_CMD_STOPCONNECT");
				break;
			}
	}
	req.timestamp = wifi_get_timestamp();
	req_idx = wifi_req_enqueue(wifi_data,&req);
	wifidbg_info("Send Msg reqidx=%d\n",req_idx);
	if(req_idx==-1)
		rtn = -1;
	else{
		if(wifi_data){
			wifidbg_info("Post Semi Start~~~~~~~~~~~~~~~~~~~~~~\n");
			wifi_sem_post(&wifi_data->syn_lock.semi_start);
		}
		else
			rtn = -1;
	}
	return rtn;
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
static int check_conffile_num_doublewifi()
{
	UI_FILELIST tmpl_files_24g;
	UI_FILELIST tmpl_files_5g;
	int listtotal24g=0,listtotal5g=0,listtotal=0;
	char * path=NULL;
	struct stat filestat;
	char *timestr;
	time_t old_time=2147483647;
	int old_confindex=-1;
	ui_filelist_init(&tmpl_files_24g, "/mnt/vram/wifi/24G/", "conf", 0, FLIST_MODE_BROWSE_FILE);
	listtotal24g = ui_filelist_get_filetotal_num(&tmpl_files_24g);
	printf("list2.4 newG=======%d\n",listtotal24g);
	if(listtotal24g > 0)
	{
		int i, templ_index = -1;
		wifidbg_info("conf file total is %d",listtotal24g);
		if(listtotal24g<=MAX_CONFFILE+2){
			//wifidbg_info("conf file total is %d",listtotal);
			if(listtotal24g>0)
				ui_filelist_exit(&tmpl_files_24g);
			return 0;
		}
		for(i=0; i<listtotal24g; i++)
		{
			path = ui_filelist_get_cur_filepath(&tmpl_files_24g, i);
			if(strcmp(path,"/mnt/vram/wifi/24G/resolv.conf")==0 || strcmp(path,"/mnt/vram/wifi/24G/latestAP.conf")==0)
				continue;
			if(stat(path,&filestat)==0){
				//timestr = ctime(&filestat.st_atime);
				//wifidbg_info("i is %d,path is %s,last modify is %ld,date is %s",i,path,filestat.st_atime,timestr);
				int file_time = filestat.st_atime;
				if(filestat.st_atime >= 315532800){						// The file access/modify/changed time will be inited to 1980-01-01 00:00:00.000000000 after reboot system.
					file_time = filestat.st_atime - 315532800;
				}
				//printf("file time: %d\n", file_time);
				if(file_time < old_time){
					old_time = filestat.st_atime;
					old_confindex = i;
				}
			}
		}

		if(listtotal24g>0){
			path = ui_filelist_get_cur_filepath(&tmpl_files_24g, old_confindex);
			wifidbg_info("delete path is %s",path);//delete  last hash.conf
			file_opt(MNAVI_CMD_FILE_DEL,NULL,path);
		}
		if(listtotal24g>0)
			ui_filelist_exit(&tmpl_files_24g);	
	}
	ui_filelist_init(&tmpl_files_5g, "/mnt/vram/wifi/5G/", "conf", 0, FLIST_MODE_BROWSE_FILE);
	listtotal5g = ui_filelist_get_filetotal_num(&tmpl_files_5g);
	printf("list5G=======%d\n",listtotal5g);
	listtotal=listtotal24g+listtotal5g;
	printf("listtotal=======%d\n",listtotal);
	if(listtotal5g>0)
	{	
		int i, templ_index = -1;
		printf("[%d][%s]",__LINE__,__func__);
		wifidbg_info("conf file total is %d",listtotal24g);
		if(listtotal5g<=MAX_CONFFILE+2){
			//wifidbg_info("conf file total is %d",listtotal);
			if(listtotal5g>0)
				ui_filelist_exit(&tmpl_files_5g);
			return 0;
		}
		printf("[%d][%s]",__LINE__,__func__);
		for(i=0; i<listtotal5g; i++)
		{printf("[%d][%s]",__LINE__,__func__);
			path = ui_filelist_get_cur_filepath(&tmpl_files_5g, i);
			if(strcmp(path,"/mnt/vram/wifi/5G/resolv.conf")==0 || strcmp(path,"/mnt/vram/wifi/5G/latestAP.conf")==0)
				continue;
			if(stat(path,&filestat)==0){
				//timestr = ctime(&filestat.st_atime);
				//wifidbg_info("i is %d,path is %s,last modify is %ld,date is %s",i,path,filestat.st_atime,timestr);
				int file_time = filestat.st_atime;
				if(filestat.st_atime >= 315532800){						// The file access/modify/changed time will be inited to 1980-01-01 00:00:00.000000000 after reboot system.
					file_time = filestat.st_atime - 315532800;
				}
				//printf("file time: %d\n", file_time);
				if(file_time < old_time){
					old_time = filestat.st_atime;
					old_confindex = i;
				}
			}
		
		}
		printf("[%d][%s]",__LINE__,__func__);
		//wifidbg_info("delete conf index is %d",old_confindex);
	if(listtotal5g>0){
		path = ui_filelist_get_cur_filepath(&tmpl_files_5g, old_confindex);
		wifidbg_info("delete path is %s",path);//delete  last hash.conf
		file_opt(MNAVI_CMD_FILE_DEL,NULL,path);
	}
	printf("[%d][%s]",__LINE__,__func__);
 	if(listtotal5g>0)
		ui_filelist_exit(&tmpl_files_5g);
	}
	return 0;
	
}
#endif
static int check_conffile_num()
{
	UI_FILELIST tmpl_files;
	int listtotal=0;
	char * path=NULL;
	struct stat filestat;
	char *timestr;
	time_t old_time=2147483647;
	int old_confindex=-1;
	ui_filelist_init(&tmpl_files, "/mnt/vram/wifi/", "conf", 0, FLIST_MODE_BROWSE_FILE);
	listtotal = ui_filelist_get_filetotal_num(&tmpl_files);
	printf("listtotal=======%d\n",listtotal);
	if(listtotal > 0)
	{
		int i, templ_index = -1;
		wifidbg_info("conf file total is %d",listtotal);
		if(listtotal<=MAX_CONFFILE+2){
			//wifidbg_info("conf file total is %d",listtotal);
			ui_filelist_exit(&tmpl_files);
			return 0;
		}
		for(i=0; i<listtotal; i++)
		{
			path = ui_filelist_get_cur_filepath(&tmpl_files, i);
			if(strcmp(path,"/mnt/vram/wifi/resolv.conf")==0 || strcmp(path,"/mnt/vram/wifi/latestAP.conf")==0)
				continue;
			if(stat(path,&filestat)==0){
				//timestr = ctime(&filestat.st_atime);
				//wifidbg_info("i is %d,path is %s,last modify is %ld,date is %s",i,path,filestat.st_atime,timestr);
				int file_time = filestat.st_atime;
				if(filestat.st_atime >= 315532800){						// The file access/modify/changed time will be inited to 1980-01-01 00:00:00.000000000 after reboot system.
					file_time = filestat.st_atime - 315532800;
				}
				//printf("file time: %d\n", file_time);
				if(file_time < old_time){
					old_time = filestat.st_atime;
					old_confindex = i;
				}
			}
		}
		//wifidbg_info("delete conf index is %d",old_confindex);
		path = ui_filelist_get_cur_filepath(&tmpl_files, old_confindex);
		wifidbg_info("delete path is %s",path);
		file_opt(MNAVI_CMD_FILE_DEL,NULL,path);
	}
	ui_filelist_exit(&tmpl_files);
	return 0;
}
extern int dongle_type_divided ;
int isAutoConnEnable(){
	return autoConnectEnable;
}
void setAutoConnEnable(int val){
	autoConnectEnable = !!val;
}

static void  *wifi_auto_connect(void *arg)
{	
	int rtn = 0;
	int rtn1=0;
	wifi_info *wifi_data = (wifi_info *)arg;
	wifi_ioctrl req;
	int wpa_cli_value = 0;
	int re_connect_times=0;
	int temp_status= -1;
	static int connect_status_no_completely = 0;
	int connect_status_change = 0 ;
	int autoConnectDone = 0;
	char pre_connect_conf[WIFI_BUF_LEN] = "";
	if(wifi_data==NULL){
		wifidbg_err("Thread wifi_data=NULL\n");
		return NULL;
	}
#if 0	
	while(1){
		pthread_testcancel();
		rtn = sleep(1);
		sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
		wifi_sem_wait(&wifi_data->semi_wpa_cli);
		sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
		int temp_status=0;
___re_auto_connect___:	
		
		temp_status=wpa_connect_status();
		
		printf("temp_status====================%d\n",temp_status);
		if(temp_status!=WIFI_COMPLETED){
			rtn1=auto_connect();
			printf("re_connect_times=========%d\n",re_connect_times);
			if(rtn1==-1&&re_connect_times<10){
				sleep(1);
				re_connect_times++;
				goto ___re_auto_connect___;
			}
			else{
				wlan_status=WIFI_DISCONNECTED;
				break;

			}
		}
		else{
			break;
		}
		
		
		re_connect_times=0;
		wifi_sem_post(&wifi_data->semi_wpa_cli);
		sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
	}
#endif
	while(1){
		//printf("pre_connect_conf ====== %s\n",pre_connect_conf);
		pthread_testcancel();
		rtn = OSSleep(1800);			//sleep for ralink dongle
#if 0		
		if(dongle_type_divided == REALTEK_dongle)
			rtn = OSSleep(1000);
		
		if(dongle_type_divided == RALINK_dongle)
			rtn = OSSleep(2500);
#endif
		sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
		wifi_sem_wait(&wifi_data->semi_wpa_cli);
		sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
//		printf("[Debug Info:%s,%d]:connect_sem_flag == %d\n",__FUNCTION__,__LINE__,connect_sem_flag);
		connect_sem_flag = 0;
		autoConnectDone = 0;

		temp_status = wpa_connect_status();					//check the status 
		
//		printf("[Debug Info:%s,%d]:temp_status == %d\n",__FUNCTION__,__LINE__,temp_status);
		
//		printf("[Debug Info:%s,%d]:wlan_status == %d\n",__FUNCTION__,__LINE__,wlan_status);
		
		if(temp_status != WIFI_COMPLETED &&(wlan_status == WIFI_AUTO_IP_ERROR || wlan_status == WIFI_AUTO_IP_SUCCESS )&& wlan_status != WIFI_DISCONNECTED_MANUAL){
				
			wlan_status = WIFI_INACTIVE ;
			wifi_reset();
			//udhcpd_dns_disable();
			OSSleep(1000);		//sleep1s
/*			rtn = wifi_send_msg(wifi_data,WIFI_CMD_SCAN);
			printf(">>>>>>>>>>>>>>>>>>>>>>>>scan after disconnect!  rtn ==== %d<<<<<<<<<<<\n",rtn);
*/
		}
		//printf("[Debug Info:%s,%d]:temp_status == %d\twlan_status == %d\tautoConnectFlag == %d\n",__FUNCTION__,__LINE__,temp_status,wlan_status,autoConnectFlag);
		if(isAutoConnEnable() != 0 && temp_status!=WIFI_COMPLETED && wlan_status != WIFI_DISCONNECTED_MANUAL && autoConnectFlag == 0){
			connect_status_no_completely ++;
			//printf("[debug %s %d] connect_status_no_completely:%d\n",__func__,__LINE__,connect_status_no_completely);
			if(connect_status_no_completely >3&&temp_status!=WIFI_COMPLETED){
				connect_status_no_completely=1;
				rtn1=auto_connect();
				autoConnectFlag =1;
				autoConnectDone = 1;
			}
		}
		/*
		else{
			OSSleep(1500);
			memset(pre_connect_conf,0,sizeof(pre_connect_conf));
			sprintf(pre_connect_conf,"/mnt/vram/wifi/%x.conf",APHash(wlan_current_status.ssid));
			connect_status_no_completely = 0;
		}
		*/
		if((temp_status == WIFI_COMPLETED)&&(wlan_status != WIFI_AUTO_IP_ERROR && wlan_status != WIFI_AUTO_IP_SUCCESS && wlan_status != WIFI_DISCONNECTED_MANUAL)){
			
			get_wifi_config_info();
		}
#if ROUTER_ONLY_ENABLE
		if(getRouterOnlyStatus() != 0 && get_ap_close_flag() != 0 && getLanConnectStatus() == 0){
			if(autoConnectDone != 0)
				temp_status = wpa_connect_status(); 
			#if MODULE_CONFIG_LAN
			if(temp_status != WIFI_COMPLETED){
				if(get_netlink_status()==0)
					wifi_softap_start();
			}
			#else
			if(temp_status != WIFI_COMPLETED){
					wifi_softap_start();
			}
			#endif
		}
#endif
		wifi_sem_post(&wifi_data->semi_wpa_cli);
		sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
	}
wifi_auto_connect_out:
	wifi_sem_post(&wifi_data->semi_wpa_cli);
	sem_getvalue(&wifi_data->semi_wpa_cli, &wpa_cli_value);
	pthread_exit((void*)rtn);
	return NULL;
}

int wifi_create_auto_connect(wifi_info * wifi_data)
{
	int rtn = 0;
	pthread_t  tid;
	int arg=0;

	wifi_data->autoconnect_thread_id = -1;

	rtn = pthread_create(&tid, NULL,wifi_auto_connect,wifi_data);

	if(rtn){
		wifidbg_err("Create wifi auto connect thread error!");
		goto wifi_create_auto_connect_out;
	}

	wifi_data->autoconnect_thread_id = tid;
	
wifi_create_auto_connect_out:
	return rtn;
}

int wifi_autoconnect_exit(wifi_info * wifi_data)
{
	void * thread_ret=NULL;
	if(wifi_data && wifi_data->autoconnect_thread_id!=-1){
		pthread_cancel(wifi_data->autoconnect_thread_id);
		pthread_join(wifi_data->autoconnect_thread_id,&thread_ret);
		sem_destroy(&wifi_data->semi_wpa_cli);
		//sem_destroy(&wifi_data->semi_wpastatus);
	}
	return 0;
}


int wifi_check_remote_control_thread(int direction)
{
	int ret = -1;
	if(direction==0){
		if(wifi_remote_control_started == 0)
		{
			#ifdef MODULE_CONFIG_WIFI_REMOTE_CONTROL
	    //Wifi Remote Control init
		    printf("\n\n\n##########Init Wifi Remote Control thread##########\n\n\n");
		    ret=wifi_remote_control_init();
			if(ret==0)
		    	wifi_remote_control_started = 1;
			else
				wifi_remote_control_started = 0;
		    #else
			    printf("\n\n\n##########Init Wifi Remote Control thread not start##########\n\n\n");
	   	 #endif
		}
		else
		{
			ret = -1;
	    	printf("\n\n\n##########Wifi Remote Control thread already started ##########\n\n\n");
		}
	}

	if(direction==1){
		if(wifi_remote_control_started == 1)
		{
		   
		    ret=wifi_remote_control_exit();
			if(ret ==0)
		   		 wifi_remote_control_started = 0;
		  
		   
		}
		else
		{
			ret = -1;
	    	printf("\n\n\n##########Wifi Remote Control thread already closed ##########\n\n\n");
		}
	}
	
	return ret;
}
#if 1
	#define wifi_concurrent_dbg_info(fmt, arg...) printf("WIFIINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
	
#else
	#define wifidbg_info(fmt, arg...)
#endif


int ez_remote_enable_flag(){
	return wifi_remote_control_started;
}

int wifi_concurrent_mode_open(){
	char callbuf[50]={0};
	int ret=0;
	ret=insmod_ko();
	if(ret==-1){
		wifi_concurrent_dbg_info("insmod ko error\n");
		goto	__wifi_concurrent_mode_open__out__;
	}
	memset(callbuf,0,50);
	sprintf(callbuf,"ifconfig wlan0 up");
	ret=system(callbuf);
	
	memset(callbuf,0,50);
	sprintf(callbuf,"ifconfig wlan1 up");
	system(callbuf);
	
	if(wifi_create_dir("/mnt/vram/wifi")!=0)
	{
		wifi_concurrent_dbg_info("create wifi dir error!");
		ret = -1;
		goto __wifi_concurrent_mode_open__out__;
	}
	
__wifi_concurrent_mode_open__out__:
	return ret;
}

int get_mac_address_info(char *mac_address_info, int port){
	char callbuf[50];
	int ret=-1;
	FILE *fp;
	char buf[512];
	char mask_get_from_ifconfig[50]={0};
	if(port == MAC_AUTO){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		memset(callbuf,0,50);
		sprintf(callbuf,"ifconfig wlan1	> /tmp/ifconfig.info");
#else
		memset(callbuf,0,50);
		sprintf(callbuf,"ifconfig wlan0 > /tmp/ifconfig.info");
#endif
	}else if(port == MAC_WLAN0){
		memset(callbuf,0,50);
		sprintf(callbuf,"ifconfig wlan0 > /tmp/ifconfig.info");
	}else if(port == MAC_WLAN1) {
		memset(callbuf,0,50);
		sprintf(callbuf,"ifconfig wlan1 > /tmp/ifconfig.info");
	}
	else if(port == MAC_WLAN2) {
		memset(callbuf,0,50);
		sprintf(callbuf,"ifconfig wlan2 > /tmp/ifconfig.info");
	}
	else{

		memset(callbuf,0,50);
		sprintf(callbuf,"ifconfig wlan3 > /tmp/ifconfig.info");

	}
	printf("---the call is--- %s to get mac addresss\n",callbuf); 
	system(callbuf);
	fp = fopen("/tmp/ifconfig.info","r");
	if(NULL == fp)
	{
			fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);	
			printf("fopen error:%s\n",strerror(errno));
			return -1;
	}		
	ret=fread(buf, 1, 512, fp);
	fclose(fp);

	if(ret <= 0 ){
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
	
	char *locate3=strstr(buf,"HWaddr ");
	if(locate3==NULL)
		return -1;
	
	locate3+=strlen("HWaddr ");
	char *locate3_tmp=locate3;
	char *locate4=strstr(locate3,"\n");
	if(locate4==NULL)
		return -1;
	
	memset(mask_get_from_ifconfig,0,50);
	memcpy(mask_get_from_ifconfig,locate3_tmp,strlen(locate3_tmp)-strlen(locate4));

	memset(mac_address_info,0,18);
	strncpy(mac_address_info,mask_get_from_ifconfig,17);
	
	printf("mac_address_info============%s\n",mac_address_info);
	return 0;
}
int get_half_mac_address(char *half_address_info){
	char mac[18];
	memset(mac,0,18);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	if(vid_pid_from_driver==0x148f8189){
		if(get_mac_address_info(mac, MAC_WLAN3)==-1){
		strncpy(mac,"00:00:00:00:00:00",18);		
		}
	}
	else{
	if(get_mac_address_info(mac, MAC_AUTO)==-1){
		strncpy(mac,"00:00:00:00:00:00",18);
		}
	}
#else
	if(get_mac_address_info(mac, MAC_AUTO)==-1){
		strncpy(mac,"00:00:00:00:00:00",18);
		}
#endif
	memcpy(half_address_info, mac+6, 12);
	return 0;
}
int get_full_mac_address(char *full_address_info){
	get_mac_address_info(full_address_info, MAC_AUTO);

	return 0;
}
int get_rank_tail(char *rank_tail_p){			//get a random value
	unsigned long rank_interger = 0;
	rank_interger = (rand() % (999999-100000+1))+ 100000;
	printf("rank_interger ========= %d\n",rank_interger);
	sprintf(rank_tail_p,"-%d",rank_interger);
	
	printf("rank_tail_p ========= %s\n",rank_tail_p);
	return 0;
}
static void restore_softap_config(){
	char cmd[128];
	char softapconf_bak[100] = "";
	char softapconf[100] = "";
	char softapconf_ln[100] = "/etc/rtl_hostapd_01.conf";
	sprintf(softapconf,"%s",HOSTAPD_CON_FILE);
	sprintf(softapconf_bak,"%s",HOSTAPD_BAK_CON_FILE);

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	printf("--restore_softap_config--\n");
	if(vid_pid_from_driver!=0x148f8189){

	}else
	{

		sprintf(softapconf_bak,"%s","/mnt/bak/rtl_hostapd_02.conf");
		sprintf(softapconf,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(softapconf_ln,"%s","/etc/rtl_hostapd_02.conf");

	}
#endif
	sprintf(cmd, "rm %s", softapconf);
	system(cmd);

	sprintf(cmd, "cp %s %s", softapconf_bak, softapconf);
	system(cmd);

	sprintf(cmd, "rm %s", softapconf_ln);
	system(cmd);

	sprintf(cmd, "ln -s %s %s", softapconf, softapconf_ln);
	system(cmd);
}

extern char * get_soft_ap_entry(int dongle_type,char *key);

#define SSID_NAME			"EZ Mobile-"
#if (FLASH_TYPE != FLASH_TYPE_NAND)
	#if (!defined(MODULE_CONFIG_3TO1) || (MODULE_CONFIG_3TO1 == 0))
		#define EZCAST_NAME			"EZCast-"
		#define EZCAST_NAME_8252	"EZCast-"
		#define EZCAST_NAME_5G		"EZCast5G-"
	#elif defined(MODULE_CONFIG_CHIP_TYPE) && MODULE_CONFIG_CHIP_TYPE == 33
		#define EZCAST_NAME			"Miraplus "
		#define EZCAST_NAME_8252	"Miraplus "
		#define EZCAST_NAME_5G		"Miraplus5G "
	#else
		#define EZCAST_NAME			"Mirascreen "
		#define EZCAST_NAME_8252	"Mirascreen "
		#define EZCAST_NAME_5G		"Mirascreen5G "
	#endif
#else
	#define EZCAST_NAME			"EZCast-"
	#define EZCAST_NAME_8252	"EZCast-"
	#define EZCAST_NAME_5G		"EZCast5G-"
#endif

#define EZCAST_NAME_WILAN	"EZCastLAN-"
#define EZCAST_NAME_MUSIC	"EZMusic#"
#ifdef	CUSTOMER_SSID
	#define EZCAST_NAME_PRO		CUSTOMER_SSID
#else
	#define EZCAST_NAME_PRO 	"Pro D01_"
#endif
#ifndef EZCAST_NAME_LANPRO
    #define EZCAST_NAME_LANPRO		"Pro B01_"
#endif
#ifndef EZCAST_NAME_LANPRO5G
    #define EZCAST_NAME_LANPRO5G		"Pro5G B02_"
#endif
#ifndef EZCAST_NAME_LANPRO24G
    #define EZCAST_NAME_LANPRO24G		"Pro B02_"
#endif

#define MIN_NUM(x, y)		(x)<(y)?(x):(y)
#define SSID_PRE_CONFIG 0
#define SSID_PREFIX_FILE	"/mnt/vram/SSID_PREFIX"
#define SSID_PREFIX_FILE1	"/mnt/vram/SSID_PREFIX_1"

int compare_mac_addr_and_ssid(){
	char address_info_tmp[20]={0};
	char address_info[20]={0};
	char  *ssid=NULL;
	int ret = 1;

	
	get_half_mac_address(address_info_tmp);	
	if(strlen(address_info_tmp) < 11){
		return 0;
	}else{
		strncat(address_info,address_info_tmp,2);
		strncat(address_info,address_info_tmp+3,2);
		strncat(address_info,address_info_tmp+6,2);
		strncat(address_info,address_info_tmp+9,2);	
	}

	ssid=get_soft_ap_entry(1,"ssid=");
	if(ssid == NULL){
		printf("get ssid error!!!\n");
		restore_softap_config();		//if the conf file is unexist or empty,then restore it.
		ret = 1;
		goto _END_;
	}
	printf("---address_info=%s  ssid=%s\n",address_info,ssid);
	if(strlen(ssid) < 8){
		ret = 1;
		goto _END_;
	}else{
		printf("ssid end: %s\n", ssid+ (strlen(ssid)-8));
		if(0 == strcmp(address_info,ssid+ (strlen(ssid)-8))){
			ret = 0;
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
			if(vid_pid_from_driver!=0x148f8189){
				if(strstr(ssid,EZCAST_NAME_LANPRO5G)) ret=0;
				else ret=1;
			}else{
				if(strstr(ssid,EZCAST_NAME_LANPRO24G)) ret=0;
				else ret=1;
			}
		#endif
			goto _END_;
		}else{
			ret = 1;
			goto _END_;
		}
	}	

_END_:
	if(ssid)
		free(ssid);
	return ret;
	
}

#if EZCAST_ENABLE
int json_get_connected_ssid(char *ssid){
	wpa_connect_status();
	if(wlan_current_status.current_status_value == WIFI_COMPLETED)
		memcpy(ssid, wlan_current_status.ssid, MAX_SSID_LEN);

	return 0;
}

void get_softap_ssid(char *ssid_val, int max_len){
	char  *ssid=NULL;
	char  *ssid_search_ralink="SSID=";
	char  *ssid_search_realtek="ssid=";
	int dongle_type=0;

	dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		ssid=get_soft_ap_entry(1,ssid_search_realtek);
	else if(dongle_type==2)
		ssid=get_soft_ap_entry(2,ssid_search_ralink);

	if(ssid){
		memcpy(ssid_val, ssid, max_len);
		free(ssid);
		ssid = NULL;
	}
}

void get_softap_psk(char *psk_val, int max_len){
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

	if(psk){
		printf("psk========%s\n",psk);
		if(strcmp(psk,"@@OPEN@@")==0){
			memcpy(psk_val, "		 ", max_len);
		}else{
			memcpy(psk_val, psk, max_len);
		}
		free(psk);
		psk = NULL;
	}
}

int ezremote_get_softap_ssid(char *remote_ssid, int max_len){
	char  *ssid=NULL;
	char  *ssid_search_ralink="SSID=";
	char  *ssid_search_realtek="ssid=";
	int dongle_type=0;

	dongle_type = dongle_get_device_changing();
	if(dongle_type==1)
		ssid=get_soft_ap_entry(1,ssid_search_realtek);
	else if(dongle_type==2)
		ssid=get_soft_ap_entry(2,ssid_search_ralink);

	memcpy(remote_ssid, ssid, ((strlen(ssid)<max_len)?strlen(ssid):max_len));
	if(ssid){
		free(ssid);
		ssid = NULL;
	}
	return 0;
}

int wifi_concurrent_mode_restart(){
	int ret=0;
	
	realtek_softap_func(DONGLE_OUT,NULL);
	OSSleep(3000);
	realtek_softap_func(DONGLE_IN,NULL);
	modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
	wifi_check_remote_control_thread(0);

	return ret;
}

int config_get_ssid_pre(char *ssid, int maxlen){
	char ssid_pre[32];
	char *p_tmp1 = NULL, *p_tmp2 = NULL;
	int len = -1, ret = -1;
	
	FILE *fp = fopen("/etc/ssid_pre", "r");
	if(fp == NULL){
		printf("[%s][%d] -- ", __func__, __LINE__);
		perror("open ssid_pre");
		return -1;
	}
	ret = fread(ssid_pre, 1, sizeof(ssid_pre), fp);
	fclose(fp);
	if(ret <= 0){
		printf("[%s][%d] -- ", __func__, __LINE__);
		perror("read ssid_pre");
		return -1;
	}
	p_tmp1 = strchr(ssid_pre, '\n');
	if(p_tmp1 != NULL){
		len = p_tmp1 - ssid_pre;
		printf("[%s][%d] -- len: %d\n", __func__, __LINE__, len);
	}
	p_tmp2 = strchr(ssid_pre, '\r');
	if(p_tmp2 != NULL){
		//len = p_tmp2 - ssid_pre;
		if(p_tmp1 == NULL || p_tmp2 < p_tmp1){
			len = p_tmp2 - ssid_pre;
			printf("[%s][%d] -- len: %d\n", __func__, __LINE__, len);
		}
	}
	if(len < 0){
		len = strlen(ssid_pre);
		printf("[%s][%d] -- len: %d\n", __func__, __LINE__, len);
	}
	if(len >= maxlen){
		printf("[%s][%d] -- The ssid_pre in config is too long!!!", __func__, __LINE__);
		return -1;
	}
	memcpy(ssid, ssid_pre, len);
	ssid[len] = '-';
	printf("[%s][%d] -- ssid_pre: %d\n", __func__, __LINE__, ssid);
	return 0;
}

#endif

char * right_nstr(char *dst,char *src, int n)
{
    char *p = src;
    char *q = dst;
    int len = strlen(src);
    if(n>len) n = len;
    p += (len-n);   /*¡ä¨®¨®¨°¡À?¦Ì¨²n??¡Á?¡¤??a¨º?*/
    while(*p!='\0'){*(q++) = *(p++);}
    return dst;
}

int change_ssid_psk(char *ssid_change, char *psk_change){
	int sum = 0;
	
	FILE *fp = NULL, *fp_p2p = NULL;
	char buf[4096] ={0};
	char buf_p2p[4096] ={0};
	char callbuf[128];
	char *tmp_psk=NULL;
	char *tmp_ssid=NULL;
	char conf_file[100]="";
	char conf_p2p_file[100]="";
	char confbak_file[100]="";
    char modify_ssid[100]="";
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	if(vid_pid_from_driver!=0x148f8189){
		sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
		sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
		sprintf(conf_p2p_file,"%s",P2P_CON_FILE);
		//char confbak_file[100]="";
		
	}
	else{
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
		sprintf(conf_p2p_file,"%s","/etc/p2p_hostapd_02.conf");
		
	}
#else
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
	sprintf(conf_p2p_file,"%s",P2P_CON_FILE);
#endif
	int ret=-1;
	int dongle_type=0;
	short  ssid_change_len = 0;
	//get_half_mac_address(half_mac_info);
	//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
	dongle_type=dongle_get_device_changing();
	//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
	EZCASTLOG("ssid: %s(%d), psk: %s(%d)\n", ssid_change, strlen(ssid_change), psk_change, strlen(psk_change));

	if(REALTEK_dongle==dongle_type){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
		fp = fopen(conf_file,"r");
		//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
		if(fp == NULL){
			memset(callbuf,0,128);		
			sprintf(callbuf,"cp %s  %s",confbak_file,conf_file);
			system(callbuf);
			fp = fopen(conf_file,"r");
		}	
		//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
#else
		fp = fopen(HOSTAPD_CON_FILE,"r");
		if(fp == NULL){
			memset(callbuf,0,128);				
			sprintf(callbuf,"cp /mnt/bak/rtl_hostapd.conf  /mnt/user1/softap/rtl_hostapd.conf");
			system(callbuf);
			fp = fopen("/mnt/user1/softap/rtl_hostapd.conf","r");
		}	

#endif
	}
	else if(RALINK_dongle==dongle_type){
		fp = fopen("/mnt/user1/softap/RT2870AP.dat","r");
		if(fp == NULL){
			memset(callbuf,0,128);				
			sprintf(callbuf,"cp /mnt/bak/RT2870AP.dat  /mnt/user1/softap/RT2870AP.dat");
			system(callbuf);
			fp = fopen("/mnt/user1/softap/RT2870AP.dat","r");
		}	
	}
	if(fp == NULL){
		printf("[%s][%d] -- Open hostapd config error!!!\n", __func__, __LINE__);
		return -1;
	}
	ret=fread(buf, 1, 4096, fp);
	ret=fclose(fp);
	printf("ssid_change=================%s\n",ssid_change);
	if(REALTEK_dongle==dongle_type){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		char ssid_num[10]="";	
		char ssid_numsame[64]="";
		char psk_val[64]="";
		char ignore_ssid[64]="";
		char ignore_broadcast_ssid=0;
		char *tmp_ignore_ssid=NULL;
		if(vid_pid_from_driver==0x148f8189){//8189es use the same psk as 8821  
            tmp_ignore_ssid=get_modify_line_realtek(HOSTAPD_CON_FILE,"ignore_broadcast_ssid=");
            sprintf(ignore_ssid,"%s",tmp_ignore_ssid);
            tmp_psk=get_modify_line_realtek(HOSTAPD_CON_FILE,"wpa_passphrase=");
            sprintf(psk_val,"%s",tmp_psk);

            if (access("/mnt/vram/CUSTOM_SSID", F_OK) == 0){
                /* Mos: if CUSTOM_SSID exist, just simply set SSID as ssid_change */
                if(ssid_change != NULL && strlen(ssid_change) > 0){
                    modify_line_realtek(buf,"ssid=",ssid_change);
                    printf("ssid=%s",ssid_change);
                }
            }
            else{
    			tmp_ssid=get_modify_line_realtek(HOSTAPD_CON_FILE,"ssid=");
	    		printf("[%s][%d][tmp_psk=%s][tmp_ssid=%s]",__func__,__LINE__,psk_val,tmp_ssid);
		    	sprintf(ssid_numsame,"%s",tmp_ssid);
			    right_nstr(ssid_num,ssid_numsame,8);
    			printf("ssid_num=%s",ssid_num);
	    		memset(ssid_numsame,0,sizeof(ssid_numsame));
		    	snprintf(ssid_numsame,strlen(ssid_change)-8+1,"%s",ssid_change);
			    printf("ssid_numsame=%s",ssid_numsame);
    			strcat(ssid_numsame,ssid_num);
		
    			printf("[%s][%d] change 8189es psk as conf1 =wpa_passphrase=%s,ssid=%s",__func__,__LINE__,tmp_psk,ssid_numsame);
	    		if(ssid_numsame != NULL && strlen(ssid_numsame) > 0){
		    		modify_line_realtek(buf,"ssid=",ssid_numsame);
			    	printf("ssid=%s",ssid_numsame);
    			}
            }
			if(psk_val != NULL && strlen(psk_val) >= 8 && access("/mnt/vram/CUSTOM_PSK", F_OK) != 0){
				printf("wpa_passphrase=%s",psk_val);
				modify_line_realtek(buf,"wpa_passphrase=",psk_val);//Add by Denny
			}
			if(ignore_ssid != NULL && strlen(ignore_ssid) >0){
				printf("ignore_broadcast_ssid=%s",ignore_ssid);
				modify_line_realtek(buf,"ignore_broadcast_ssid=",ignore_ssid);//Add by luoyuping
			}
		}
		else{
			if(ssid_change != NULL && strlen(ssid_change) > 0){
                if (access("/mnt/vram/CUSTOM_SSID", F_OK) == 0){
                    /* Mos: If CUSTOM_SSID exist, just add postfix into ssid and write into conf */
                    sprintf(modify_ssid, "%s_5G", ssid_change);
                    modify_line_realtek(buf,"ssid=",modify_ssid);
                    printf("ssid=%s",modify_ssid);
                }
                else{
				    modify_line_realtek(buf,"ssid=",ssid_change);
				    printf("ssid=%s",ssid_change);
                }
			}
			if(psk_change != NULL && strlen(psk_change) >= 8 && access("/mnt/vram/CUSTOM_PSK", F_OK) != 0){
			printf("wpa_passphrase=%s",psk_change);
			modify_line_realtek(buf,"wpa_passphrase=",psk_change);//Add by Denny
			}
			if(ignore_broadcast_ssid=ezCastGetNumConfig(CONFNAME_IGNORESSID))
			{
				printf("ignore_broadcast_ssid=%d\n",ignore_broadcast_ssid);
				modify_line_realtek(buf,"ignore_broadcast_ssid=","1");//Add by luoyuping
			}
		}
		//printf("[%s][%d] change 8189es psk as =%s\n",__func__,__LINE__,tmp_psk);		
#else
		if(ssid_change != NULL && strlen(ssid_change) > 0)
		{
			modify_line_realtek(buf,"ssid=",ssid_change);
			EZCASTLOG("modify SSID success!!\n");
		}
		// TODO: I don't known why can't change password if there is the file "CUSTOM_PSK", remove this condition now.   -- Bink.li
		//if(psk_change != NULL && strlen(psk_change) >= 8 && access("/mnt/vram/CUSTOM_PSK", F_OK) != 0){
		if(psk_change != NULL && strlen(psk_change) >= 8){
			printf("wpa_passphrase=%s",psk_change);
			modify_line_realtek(buf,"wpa_passphrase=",psk_change);//Add by Denny
			EZCASTLOG("modify PSK success!!\n");
		}
#endif
		fp = fopen(conf_file,"wb+");
		if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);

				return -1;
		}
#else
		if(ssid_change != NULL && strlen(ssid_change) > 0)
			modify_line_realtek(buf,"ssid=",ssid_change);
		fp = fopen("mnt/user1/softap/rtl_hostapd.conf","wb+");
		if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);

				return -1;
		}
#endif
#if 1// EZCAST_ENABLE
		fp_p2p = fopen(conf_p2p_file,"r");
		if(fp_p2p != NULL){
			ret=fread(buf_p2p, 1, 4096, fp_p2p);
			ret=fclose(fp_p2p);
			if(ssid_change != NULL && strlen(ssid_change) > 0)
				modify_line_realtek(buf_p2p,"ssid=",ssid_change);
			if(psk_change != NULL && strlen(psk_change) >= 8)
				modify_line_realtek(buf_p2p,"wpa_passphrase=",psk_change);
			fp_p2p = fopen(conf_p2p_file,"wb+");
			if(fp_p2p != NULL){
				ret=fwrite(buf_p2p, 1, 4096, fp_p2p);
				ret=fflush(fp_p2p);
				int fd_p2p_write = fileno(fp_p2p);
				ret=fsync(fd_p2p_write);
				ret=fclose(fp_p2p);
			}
		}
#endif
	}
	else if(RALINK_dongle==dongle_type){
		if(ssid_change != NULL && strlen(ssid_change) > 0)
			modify_line(buf,"SSID=",ssid_change);
		fp = fopen("mnt/user1/softap/RT2870AP.dat","wb+");
		if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);

				return -1;
		}
	}

	ret=fwrite(buf, 1, 4096, fp);
	ret=fflush(fp);
	int fd_write = fileno(fp);
	ret=fsync(fd_write);
	ret=fclose(fp);
	return 0;
}

int getSsidPrefix(char *buf, int maxlen){
	int len = 0, ret = -1;
	FILE *fp = NULL;
	char ezcast_prefix[48]="/mnt/vram/SSID_PREFIX";
#if EZCAST_ENABLE
   #if (AM_CHIP_MINOR != 8258 && AM_CHIP_MINOR != 8268)
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		if(vid_pid_from_driver==0x148f8189)
			sprintf(ezcast_prefix,SSID_PREFIX_FILE1);
		else
			sprintf(ezcast_prefix,SSID_PREFIX_FILE);
	
	#else	
		sprintf(ezcast_prefix,SSID_PREFIX_FILE);
	#endif
	if(access(ezcast_prefix, F_OK) == 0){
		fp = fopen(ezcast_prefix, "r");
		if(fp != NULL){
			ret = fread(buf, 1, maxlen-1, fp);
			fclose(fp);
			if(ret > 0){
				buf[maxlen-1] = '\0';
				EZCASTLOG("Old SSID prefix: %s\n", buf);
				return 0;
			}
		}
	}
	EZCASTLOG("SSID prefix not save\n");
	#endif
#if SSID_PRE_CONFIG
	if(config_get_ssid_pre(buf, maxlen) != 0){
		len = strlen(EZCAST_NAME);
		strncpy(buf, EZCAST_NAME, MIN_NUM(maxlen, len));
	}
#else

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	#if EZWILAN_ENABLE
		#if MODULE_CONFIG_EZCASTPRO_MODE==8075
			if(vid_pid_from_driver==0x148f8189){
				len = strlen(EZCAST_NAME_LANPRO24G);
				strncpy(buf, EZCAST_NAME_LANPRO24G, MIN_NUM(maxlen, len));
			}	
			else{
				len = strlen(EZCAST_NAME_LANPRO5G);
				strncpy(buf, EZCAST_NAME_LANPRO5G, MIN_NUM(maxlen, len));
			}
		#else
			len = strlen(EZCAST_NAME_LANPRO);
			strncpy(buf, EZCAST_NAME_LANPRO, MIN_NUM(maxlen, len));
		#endif
	#else
		len = strlen(EZCAST_NAME_PRO);
		strncpy(buf, EZCAST_NAME_PRO, MIN_NUM(maxlen, len));
	#endif
#elif EZMUSIC_ENABLE
		len = strlen(EZCAST_NAME_MUSIC);
		strncpy(buf, EZCAST_NAME_MUSIC, MIN_NUM(maxlen, len));
#elif EZWILAN_ENABLE
		len = strlen(EZCAST_NAME_WILAN);
		strncpy(buf, EZCAST_NAME_WILAN, MIN_NUM(maxlen, len));
#elif (FLASH_TYPE != FLASH_TYPE_NAND)
		int wifiModule = get_wifi_model_type();
		if(wifiModule == REALTEK_8821CU || wifiModule == REALTEK_8811AU)
		{
			EZCASTLOG("EZCAST_NAME_5G: %s\n", EZCAST_NAME_5G);
			len = strlen(EZCAST_NAME_5G);
			strncpy(buf, EZCAST_NAME_5G, MIN_NUM(maxlen, len));
		}
		else
		{
			EZCASTLOG("EZCAST_NAME: %s\n", EZCAST_NAME);
			len = strlen(EZCAST_NAME);
			strncpy(buf, EZCAST_NAME, MIN_NUM(maxlen, len));
		}
#elif EZCAST5G_ENABLE
		len = strlen(EZCAST_NAME_5G);
		strncpy(buf, EZCAST_NAME_5G, MIN_NUM(maxlen, len));
#else
		len = strlen(EZCAST_NAME);
		strncpy(buf, EZCAST_NAME, MIN_NUM(maxlen, len));
#endif

#endif
   EZCASTLOG("Use SSID prefix: %s\n", buf);
	#if (AM_CHIP_MINOR != 8258 && AM_CHIP_MINOR != 8268)
	fp = fopen(ezcast_prefix, "w");
	if(fp != NULL){
		ret = fwrite(buf, 1, strlen(buf), fp);
		if(ret <= 0){
			EZCASTWARN("Write ssid prefix file fail\n");
		}
		fclose(fp);
		sync();
	}
	#endif
#else
	len = strlen(SSID_NAME);
	strncpy(buf, SSID_NAME, MIN_NUM(maxlen, len));
#endif
	return 0;
}

int get_rand_psk(char *rank_tail_p,int sum_tmp){
	unsigned long rank_interger = 0;
	rank_interger = (sum_tmp % (99999999-10000000+1))+ 10000000;
	printf("rank_interger ========= %d\n",rank_interger);
	sprintf(rank_tail_p,"%d",rank_interger);	
	printf("rank_tail_p ========= %s\n",rank_tail_p);
	return 0;
}

char *getDefaultPsk(char *ssid, char *buf, int bufLen){
	char rank_tail[20]={0};

	if(ssid == NULL || buf == NULL){
		EZCASTWARN("ssid[%p] or buf[%p] is null\n", ssid, buf);
		return NULL;
	}
	int ssid_len = strlen(ssid);
	if(ssid_len < 8){
		EZCASTWARN("SSID[%s:%d] is too short!!\n", ssid, ssid_len);
		return NULL;
	}
	int sum = (ssid[ssid_len-1]<<24) | (ssid[ssid_len-2]<<20) 
		| (ssid[ssid_len-3]<<16) | (ssid[ssid_len-4]<<12) 
		| (ssid[ssid_len-5]<<8) | (ssid[ssid_len-6]<<4) 
		| ssid[ssid_len-7] | ssid[ssid_len-8];
	
	printf("\n=========sum:%d\n",sum);
	
	get_rand_psk(rank_tail,sum);	
	snprintf(buf, bufLen, "%s", rank_tail);

	return buf;
}

int change_default_ssid(){			//change the softap ssid and write it into the hostapd conf
	char rank_tail[20]={0};
	char ssid_change[50]={0};
	char psk_change[20]={0};
	int sum = 0;
	int ret=-1;
	int dongle_type=0;
	short  ssid_change_len = 0;
    FILE *fp = NULL;

	dongle_type=dongle_get_device_changing();
	
	if(REALTEK_dongle==dongle_type){

        if (access("/mnt/vram/CUSTOM_SSID", F_OK) == 0){
            /* Mos: /mnt/vram/CUSTOM_SSID exist, read the custom SSID */
            fp = fopen("/mnt/vram/CUSTOM_SSID", "r");
            if(fp != NULL){
                fgets(ssid_change, sizeof(ssid_change), fp);
                printf("%s, %d: CUSTOM_SSID set! CUSTOM_SSID: %s\n", __func__, __LINE__, ssid_change);
                fclose(fp);
            }
        }
        else{

    		/*
	    	***************************************************************
		    *				   Chang EZCast SSID and PSK   Start
    		***************************************************************
	    	*/
		    get_half_mac_address(rank_tail);	
    		printf("------function:%s,line:%d  mac=%s strlen(rank_tail)=%d\n",__FUNCTION__,__LINE__,rank_tail,strlen(rank_tail));

	    	memset(ssid_change,0,50);
		    getSsidPrefix(ssid_change, sizeof(ssid_change));
    		if(strlen(rank_tail) < 11){
	    		strncat(ssid_change,"12345678",8);
		    }else{	
    			strncat(ssid_change,rank_tail,2);
	    		strncat(ssid_change,rank_tail+3,2);
		    	strncat(ssid_change,rank_tail+6,2);
    			strncat(ssid_change,rank_tail+9,2);
	    	}
            if (access("/mnt/vram/STD_SSID", F_OK) != 0) {
                fp = fopen("/mnt/vram/STD_SSID","w");
                if (fp != NULL){
                    fputs(ssid_change, fp);
                    fclose(fp);
                }
            }
        }

		ssid_change_len = strlen(ssid_change);
#if EZCAST_ENABLE

#if EZMUSIC_ENABLE
		sprintf(psk_change, "00000000");
#else
		getDefaultPsk(ssid_change, psk_change, sizeof(psk_change));

#endif
		ret = change_ssid_psk(ssid_change, psk_change);		// For EZCast/EZCast Pro/EZMusic etc.
#else
		ret = change_ssid_psk(ssid_change, NULL);			// For public mode
#endif
	/*
	***************************************************************
	*				   Chang EZCast SSID and PSK   End
	***************************************************************
	*/	
	}
	else if(RALINK_dongle==dongle_type){
		get_rank_tail(rank_tail);	
		memset(ssid_change,0,50);
		strncpy(ssid_change,"RT2860AP",strlen("RT2860AP"));
		strncat(ssid_change,rank_tail,strlen(rank_tail));
		
		ret = change_ssid_psk(ssid_change, NULL);
	}
	
	return 0;
}


void createQRCodeForWire()
{
#ifdef MODULE_CONFIG_QR_URL
#ifndef MODULE_CONFIG_QR_APP_VENDOR
#define MODULE_CONFIG_QR_APP_VENDOR	"mirascreen_none"
#endif
	char url[256];
	char vendor[64] = "unknown";
	char chipid[32], tmp[32], ssid[64];  //, psk[64], psk_encode[128];

	memset(url, 0, sizeof(url));
	memset(chipid, 0, sizeof(chipid));
	memset(ssid, 0, sizeof(ssid));
	//memset(psk, 0, sizeof(psk));
	//memset(psk_encode, 0, sizeof(psk_encode));

	ota_get_vendor(vendor, sizeof(vendor));
	gethostname(ssid, sizeof(ssid));
	am_soc_get_chip_id_high_32bit_to_string(chipid, sizeof(chipid));
	#if 0
	ezremote_get_softap_ssid(ssid, sizeof(ssid));
	get_softap_psk(psk, sizeof(psk));
	ezcastQREncrypt(psk_encode, psk, sizeof(psk_encode));
	EZCASTLOG("psk: %s, encode: %s\n", psk, psk_encode);
	get_mac_address_info(tmp, MAC_WLAN0);
	strncat(mac,tmp,2);
	strncat(mac,tmp+3,2);
	strncat(mac,tmp+6,2);
	strncat(mac,tmp+9,2);
	strncat(mac,tmp+12,2);
	strncat(mac,tmp+15,2);
	#endif
	
	char *curl_mac = curl_escape(chipid, 0);
	char *curl_ssid = curl_escape(ssid, 0);
	//char *curl_psk = curl_escape(psk_encode, 0);
	char *curl_app_vendor = curl_escape(MODULE_CONFIG_QR_APP_VENDOR, 0);
	char *curl_fw_version = curl_escape(ota_get_local_version(), 0);
	char *curl_vendor = curl_escape(vendor, 0);

	char *qm = strchr(MODULE_CONFIG_QR_URL, '?');
	snprintf(url, sizeof(url), "%s%s&vendor=%s&app_vendor=%s&device_id=%s&firmware_version=%s&softap_ssid=%s", \
		MODULE_CONFIG_QR_URL, qm?"&":"?", \
		curl_vendor?curl_vendor:"unknown", \
		curl_app_vendor?curl_app_vendor:"unknown", \
		curl_mac?curl_mac:"unknown", \
		curl_fw_version?curl_fw_version:"unknown", \
		curl_ssid?curl_ssid:"unknown");

	EZCASTLOG("QRCode url: %s\n", url);

	if(curl_mac)
	{
		curl_free(curl_mac);
		curl_mac = NULL;
	}
	if(curl_ssid)
	{
		curl_free(curl_ssid);
		curl_ssid = NULL;
	}
	//if(curl_psk)
	//{
	//	curl_free(curl_psk);
	//	curl_psk = NULL;
	//}
	if(curl_app_vendor)
	{
		curl_free(curl_app_vendor);
		curl_app_vendor = NULL;
	}
	if(curl_fw_version)
	{
		curl_free(curl_fw_version);
		curl_fw_version = NULL;
	}
	if(curl_vendor)
	{
		curl_free(curl_vendor);
		curl_vendor = NULL;
	}

	int err = __qrcode_gen_open();
	if(err < 0)
	{
		EZCASTWARN("Failed to open qrc\n");
		return;
	}
	
	err = __qrcode_gen_encode((unsigned char *)url);
	if(err == -1)
	{
		EZCASTWARN("QR encode error\n");
	}
	else
	{
		err = __qrcode_gen_bitmap(TMP_QRCODE_FILE);
		if(err < 0)
			EZCASTWARN("QRCode encode fail.\n");
	}

	__qrcode_gen_close();
	
#endif
}


void createQRCodeForWiFiProduct()
{
#ifdef MODULE_CONFIG_QR_URL
#ifndef MODULE_CONFIG_QR_APP_VENDOR
#define MODULE_CONFIG_QR_APP_VENDOR	"mirascreen_none"
#endif
	char url[256];
	char vendor[64] = "unknown";
	char mac[32], tmp[32], ssid[64], psk[64], psk_encode[128];

	memset(url, 0, sizeof(url));
	memset(mac, 0, sizeof(mac));
	memset(ssid, 0, sizeof(ssid));
	memset(psk, 0, sizeof(psk));
	memset(psk_encode, 0, sizeof(psk_encode));

	ota_get_vendor(vendor, sizeof(vendor));
	ezremote_get_softap_ssid(ssid, sizeof(ssid));
	get_softap_psk(psk, sizeof(psk));
	ezcastQREncrypt(psk_encode, psk, sizeof(psk_encode));
	EZCASTLOG("psk: %s, encode: %s\n", psk, psk_encode);
	get_mac_address_info(tmp, MAC_WLAN0);
	strncat(mac,tmp,2);
	strncat(mac,tmp+3,2);
	strncat(mac,tmp+6,2);
	strncat(mac,tmp+9,2);
	strncat(mac,tmp+12,2);
	strncat(mac,tmp+15,2);

	char *curl_mac = curl_escape(mac, 0);
	char *curl_ssid = curl_escape(ssid, 0);
	char *curl_psk = curl_escape(psk_encode, 0);
	char *curl_app_vendor = curl_escape(MODULE_CONFIG_QR_APP_VENDOR, 0);
	char *curl_fw_version = curl_escape(ota_get_local_version(), 0);
	char *curl_vendor = curl_escape(vendor, 0);

	char *qm = strchr(MODULE_CONFIG_QR_URL, '?');
	snprintf(url, sizeof(url), "%s%s&vendor=%s&app_vendor=%s&device_id=%s&firmware_version=%s&softap_ssid=%s&P=%s", \
		MODULE_CONFIG_QR_URL, qm?"&":"?", \
		curl_vendor?curl_vendor:"unknown", \
		curl_app_vendor?curl_app_vendor:"unknown", \
		curl_mac?curl_mac:"unknown", \
		curl_fw_version?curl_fw_version:"unknown", \
		curl_ssid?curl_ssid:"unknown", \
		curl_psk?curl_psk:"unknown");

	EZCASTLOG("QRCode url: %s\n", url);

	if(curl_mac)
	{
		curl_free(curl_mac);
		curl_mac = NULL;
	}
	if(curl_ssid)
	{
		curl_free(curl_ssid);
		curl_ssid = NULL;
	}
	if(curl_psk)
	{
		curl_free(curl_psk);
		curl_psk = NULL;
	}
	if(curl_app_vendor)
	{
		curl_free(curl_app_vendor);
		curl_app_vendor = NULL;
	}
	if(curl_fw_version)
	{
		curl_free(curl_fw_version);
		curl_fw_version = NULL;
	}
	if(curl_vendor)
	{
		curl_free(curl_vendor);
		curl_vendor = NULL;
	}

	int err = __qrcode_gen_open();
	if(err < 0)
	{
		EZCASTWARN("Failed to open qrc\n");
		return;
	}
	
	err = __qrcode_gen_encode((unsigned char *)url);
	if(err == -1)
	{
		EZCASTWARN("QR encode error\n");
	}
	else
	{
		err = __qrcode_gen_bitmap(TMP_QRCODE_FILE);
		if(err < 0)
			EZCASTWARN("QRCode encode fail.\n");
	}

	__qrcode_gen_close();
	
#endif
}


int change_wifi_softap_defalut_ssid(int mode){ //add mode for 24G ,5G
	
	int first_change_flag=0;
	int dongle_type=0;
	int isSsidChanged = 0;
	struct sysconf_param sys_info;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	char conf_udp_file[100]="/etc/udhcpdbr1.conf";
	char conf_tmpudp_file[100]="/tmp/udhcpd_01.conf";
#else
	char conf_udp_file[100]="/etc/udhcpd_01.conf";
	char conf_tmpudp_file[100]="/tmp/udhcpd_01.conf";
#endif
    FILE * fp = NULL;

	_get_env_data(&sys_info);
	dongle_type= dongle_get_device_changing();
	if(REALTEK_dongle==dongle_type)	{
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE		
		first_change_flag=sys_info.realtek_wifi_concurrent_softap_ssid_init_flag;
		if(first_change_flag==0){
			sys_info.realtek_wifi_concurrent_softap_ssid_init_flag=1;
		}
#else  	
		first_change_flag=sys_info.realtek_softap_ssid_init_flag;
		if(first_change_flag==0){
			sys_info.realtek_softap_ssid_init_flag=1;
		}		
#endif
	}
	else if(RALINK_dongle==dongle_type) {
		first_change_flag=sys_info.ralink_softap_ssid_init_flag;
		if(first_change_flag==0){
			sys_info.ralink_softap_ssid_init_flag=1;
		}
	}	
	printf("first_change_flag==============%d\n",first_change_flag);
#if EZCAST_ENABLE

	EZCASTLOG("----------=================---------------+++++++++++++++==========--------------\n");
#if EZCAST_MAC_LIMIT
	EZCASTLOG("================== EZCast5G limit firmware!!!\n");
	if(isWifiEnable() == 0){
		change_ssid_psk("(error)", "00000000");
		return -1;
	}
#endif
	if(sys_info.softap_info.softap_psk_setted_flag == 1){
		printf("***************************************************************\n");
		printf("*	 Softap has setted by user!\n");
		printf("***************************************************************\n");

		//char ssidPrefix[50] = {0};
		//getSsidPrefix(ssidPrefix, sizeof(ssidPrefix));
		//if(strncmp(sys_info.softap_info.softap_ssid, ssidPrefix, strlen(ssidPrefix)) == 0){
			//printf("sys_info.softap_info.softap_ssid=%s sys_info.softap_info.softap_psk=%s\n",sys_info.softap_info.softap_ssid,sys_info.softap_info.softap_psk);
			change_ssid_psk(sys_info.softap_info.softap_ssid, sys_info.softap_info.softap_psk);
			EZCASTLOG("ssid: %s, psk: %s\n", sys_info.softap_info.softap_ssid, sys_info.softap_info.softap_psk);
			#if (FLASH_TYPE!=FLASH_TYPE_16M_SNOR)
			sys_info.softap_info.softap_psk_setted_flag = 0;
			memset(sys_info.softap_info.softap_ssid, 0, sizeof(sys_info.softap_info.softap_ssid));
			memset(sys_info.softap_info.softap_psk, 0, sizeof(sys_info.softap_info.softap_psk));
			_save_env_data(&sys_info);
			_store_env_data();
			#endif
#if !(defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0)
			ezCastCleanAutoplaySetting();
#endif

#if EZCAST_ENABLE && !EZMUSIC_ENABLE && !EZWIRE_ENABLE
			//ezCastDelConfig(CONFNAME_EZAIRMODE);
#endif
		//}else{
		//	first_change_flag = 0;
		//}
	}
	if(first_change_flag==0||compare_mac_addr_and_ssid()||(access("/mnt/vram/CUSTOM_SSID", F_OK) == 0)){
		EZCASTLOG("init default ssid[first_change_flag: %d]!!!\n", first_change_flag);
		//printf("sys_info.softap_info.softap_ssid=%s,sys_info.softap_info.softap_psk",sys_info.softap_info.softap_ssid,sys_info.softap_info.softap_psk);
		change_default_ssid();
		isSsidChanged = 1;
		sys_info.psk_changed = 0;
		#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR)
		sys_info.softap_info.softap_psk_setted_flag = 1;
		memset(sys_info.softap_info.softap_ssid, 0, sizeof(sys_info.softap_info.softap_ssid));
		memset(sys_info.softap_info.softap_psk, 0, sizeof(sys_info.softap_info.softap_psk));
		get_softap_ssid(sys_info.softap_info.softap_ssid, sizeof(sys_info.softap_info.softap_ssid));
		get_softap_psk(sys_info.softap_info.softap_psk, sizeof(sys_info.softap_info.softap_psk));
		#else
		sys_info.softap_info.softap_psk_setted_flag = 0;
		#endif	
		_save_env_data(&sys_info);
		_store_env_data();
	}
	char curssid[50];
	memset(curssid, 0, sizeof(curssid));
    if ((access("/mnt/vram/STD_SSID", F_OK) == 0) && (access("/mnt/vram/CUSTOM_SSID", F_OK) == 0)){
        fp = fopen("/mnt/vram/STD_SSID", "r");
        if(fp != NULL){
            fgets(curssid, sizeof(curssid), fp);
            fclose(fp);
            printf("%s, %d: Set hostname as %s\n", __func__, __LINE__, curssid);
        }        
    }
    else{
	    ezremote_get_softap_ssid(curssid, sizeof(curssid));
    }
	createQRCodeForWiFiProduct();
	ezremote_save_ssid(curssid);
	ezCustomerSetPskChanged((int)!!sys_info.psk_changed);
	ezCustomerSetAutoLanguage((int)!sys_info.lang_auto_disable);
	ezcastSchemaBootMode(sys_info.last_ui);
	ezRemoteMsgInit();

	int connectMode = getConnectMode();
#if ROUTER_ONLY_ENABLE
	if(connectMode == CTLMODE_ROUTERONLY){
		setRouterOnlyStatus(1);
	}else{
		setRouterOnlyStatus(0);
	}
#endif

	if(connectMode == CTLMODE_DIRECTONLY){
		ezCastSetRouterCtlEnable(0);
	}else{
		ezCastSetRouterCtlEnable(1);
	}
#if AUTOPLAY_SET_ENABLE
	ezCastSetAutoplay();
#endif
	setHotspotConnectStatus(HOTSPOT_UNCONNECT);
	
	char cmd[256]="";
	
#if EZCAST_LITE_ENABLE
	if(sys_info.last_ui == EZMIRRORLITE_ITEM){
		ezCastSetRouterCtlEnable(1);
	}
#endif

#if AUTO_DNS_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(access("/etc/udhcpdbr1_nodns.conf", F_OK) == 0){
		system("ln -sf /etc/udhcpdbr1_nodns.conf /tmp/udhcpd_01.conf");
	}
#else
	if(access("/etc/udhcpd_nodns_01.conf", F_OK) == 0){
		system("ln -sf /etc/udhcpd_nodns_01.conf /tmp/udhcpd_01.conf");
	}
#endif
	else
	{
		sprintf(cmd,"ln -sf %s %s",conf_udp_file,conf_tmpudp_file);
		system(cmd);
	}
#elif EZCAST_LITE_ENABLE && (MODULE_CONFIG_EZWILAN_ENABLE ==0)
	if(sys_info.last_ui == EZMIRRORLITE_ITEM && access("/etc/udhcpd_nodns_01.conf", F_OK) == 0){
		system("ln -sf /etc/udhcpd_nodns_01.conf /tmp/udhcpd_01.conf");
	}else{
		sprintf(cmd,"ln -sf %s %s",conf_udp_file,conf_tmpudp_file);
		system(cmd);
	}
#else
	sprintf(cmd,"ln -sf %s %s",conf_udp_file,conf_tmpudp_file);

	system(cmd);
#endif

#else
	if(first_change_flag==0||compare_mac_addr_and_ssid()){
		change_default_ssid();
		//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
		_save_env_data(&sys_info);
		_store_env_data();
	}	
#endif

#if defined(MODULE_CONFIG_FLASH_TYPE) && MODULE_CONFIG_FLASH_TYPE != 0
	int fdw = open("/proc/sys/vm/drop_caches", O_RDWR);
	if(fdw >= 0)
	{
		char val[2] = "1";
		write(fdw, val, sizeof(val));
		close(fdw);
	}
#endif

	return 0;
}
int wifi_concurrent_mode_init(){
	int ret = 0;
	int val = -1;
	int val_5g = -1;
	insmod_ko();
	system("ifconfig wlan1 up");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	change_wifi_softap_defalut_ssid(RTL_BAND_5G);
	change_wifi_softap_defalut_ssid(RTL_BAND_24G);

#else
	change_wifi_softap_defalut_ssid(RTL_BAND_24G);
#endif

#if EZCAST_ENABLE
    val=ezCastGetNumConfig(CONFNAME_SOFTAP_CHANNEL);
#endif
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	vid_pid_from_driver=vid_pid_from_driver_firstin;
	printf("[%s][%d]\n",__FILE__,__LINE__);
	val_5g=ezCastGetNumConfig(CONFNAME_SOFTAP_5G_CHANNEL);
	 printf("softap_start 24G=%d,5G=%d\n",val,val_5g);  

	if(val_5g>0)
	{
		printf("(val_5g>0)\n");
		wifiChannel_set=1;
		set_concurrent_mode_channel(val_5g);
	}
	else
		int_probox_mode_best_channel(RTL_BAND_5G);

	if(val>0)
	{
		set_concurrent_mode_channel(val);
	}
	else 
		int_probox_mode_best_channel(RTL_BAND_24G);
	checkAndRecoverForConcurrent(RTL_BAND_24G);
	checkAndRecoverForConcurrent(RTL_BAND_5G);
#else	
	if(val<=0)
		inti_concurrent_mode_best_channel();
	else
		set_concurrent_mode_channel(val);
	realtek_softap_func(DONGLE_IN,NULL);

	if(access("/tmp/hostapd",F_OK)!=0){
		realtek_softap_func(DONGLE_OUT,NULL);
		realtek_softap_func(DONGLE_IN,NULL);
	}
	checkAndRecoverForConcurrent(RTL_BAND_24G);
#endif
		
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
	if(rtk8189es_enable==0){
		printf("[%s][%d]RTK8189ES.Dongle Enable :vpid=%x\n",__func__,__LINE__,0x148f8189);
		rtk8189es_in(0x148f8189);//   8821 hot plug ,so 8189 must start only once
		rtk8189es_enable=1;
	}else
		wifiBridgeProcess(WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3);						
#endif
	modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
	wifi_check_remote_control_thread(0);
#if MODULE_CONFIG_SNMP_ENABLE
	int snmp_flag=0;
	snmp_flag=ezCastGetNumConfig(CONFNAME_SNMP);
	printf("snmp_flag=%d  \n",snmp_flag);
	if(snmp_flag)
	{
		am_snmpv3_enable();
	}
#endif
#if WEBSETTING_ENABLE
	create_websetting_server();
	if(ezCastGetNumConfig(CONFNAME_INTERNET_CONTROL)==1)
		Start_TcpSocket(0);
#endif
#if EZCAST_ENABLE
	htmlSettingStart();
#endif
	return ret;
}

enum {
	WPS_DEFAULT=0,
	WPS_PBUTTON,
	WPS_PCODE
};
int wifi_wps_disconnect()
{
	char call_buf[100];
	sprintf(call_buf,"wpa_cli -p/tmp/wpa_supplicant remove_network 0");
	//sprintf(call_buf,"killall wpa_supplicant");
	printf("[%s %d] remove_network id\n",__func__,__LINE__);
	system(call_buf);
	return 0;
}
/*function is process wps connect method(client mode)
  *flags value:
 * 	1 : wps push button method
 * 	2 : wps pin code method
 *  return value:
 *	0: is connect successful
 *     others is connect failure
*/
int wifi_wps_method_connect(int flags,int pincode)
{
	int rtn = -1;
	char call_buf[128];
	wps_connect_flag=1;
//printf("%s %d<<<<<<<<<<<<\n",__func__,__LINE__);
	if(access("/tmp/wpa_supplicant",F_OK)!=0){
		sprintf(call_buf,"wpa_supplicant -B -iwlan0 -c /etc/wireless-tkip.conf");
		system(call_buf);
	}

	if(flags==WPS_PBUTTON){
//printf("%s %d<<<<<<<<<<<<\n",__func__,__LINE__);
		memset(call_buf,0x00,128);
		sprintf(call_buf,"wpa_cli -p/tmp/wpa_supplicant  wps_pbc any");
		system(call_buf);
	}

	if(flags==WPS_PCODE){
		memset(call_buf,0x00,128);
		sprintf(call_buf,"wpa_cli -p/tmp/wpa_supplicant wps_pin any %d",pincode);
		system(call_buf);
	}

	rtn = wifi_status_check();
	if(strncmp(selected_filename,"/etc/wireless-tkip.conf",strlen("/etc/wireless-tkip.conf"))!=0){
		printf("[%s %d] config:%s\n",__func__,__LINE__,selected_filename);
		remove(selected_filename);
		system("sync");
	}
	return rtn;
}

char* _get_ip_filename()
{
	char ipFileName[64]={0};
	//debug_info();
	wpa_connect_status();
	if(wlan_current_status.current_status_value == WIFI_COMPLETED){		
		sprintf(ipFileName,"/mnt/vram/wifi/%x.ip",APHash(wlan_current_status.ssid));
		memset(selectedIPfile,0,64);
		strcpy(selectedIPfile ,ipFileName);
	}
	else{
		printf("[debug info:%s,%d]:wlan_current_status.current_status_value != WIFI_COMPLETED\n",__FUNCTION__,__LINE__);
		return  NULL;
	}
	//printf("[debug info:%s,%d]:the ip file name is %s\n",__FUNCTION__,__LINE__,ipFileName);
	return selectedIPfile;
}

int checkIPfileExist(){
	char *ipFile=NULL;
	debug_info();
	ipFile = _get_ip_filename();
	
	if(ipFile==NULL){	
		return 0;
	}	

	if(access(ipFile,F_OK) == 0)
	{
		printf("[debug info:%s,%d]:the ip file %s is exist!\n",__FUNCTION__,__LINE__,ipFile);
		return 1;
	}
	else
	{
		memset(selectedIPfile,0,64);
		printf("[debug info:%s,%d]:the ip file %s is  not exist!\n",__FUNCTION__,__LINE__,ipFile);
		return 0;
	}


}

int saveManualip(char *ip_temp,char *mask_temp, char *gateway_temp ){	
	FILE *fp_conf = NULL;
	int fd_conf;
	char buf[256]={0};
	char *ipFile=NULL;

	memset(buf,0,256);
//	char *ipFileName=NULL;
	ipFile = _get_ip_filename();
/*
	wpa_connect_status();
	if(wlan_current_status.current_status_value == WIFI_COMPLETED)
		sprintf(ipFile,"/mnt/vram/wifi/%x.ip",APHash(wlan_current_status.ssid));
	else
		return -1;
*/
	printf("ip=%s\tmask=%s\tgateway=%s\n",ip_temp,mask_temp,gateway_temp);
	sprintf(buf,"ip=%s\tmask=%s\tgateway=%s\n",ip_temp,mask_temp,gateway_temp);
	
	printf("buf  ======== %s\n",buf);
	fp_conf = fopen(ipFile,"wb+");
	if(fp_conf==NULL)
	{
		wifidbg_err("can not open %s\n",ipFile);
		return -1;
	}
	if(fwrite(buf,sizeof(char),strlen(buf),fp_conf) != strlen(buf))
	{
		wifidbg_err("write conf file error!");
		fclose(fp_conf);
		return -1;
	}
	fflush(fp_conf);
	fd_conf= fileno(fp_conf);
	fsync(fd_conf);
	fclose(fp_conf);
	fp_conf = NULL;
	return 0;
}

int connectManualip(){
	FILE *fp_ip=NULL;
	char ip[24]={0};
	char mask[24]={0};
	char gateway[24]={0};
	char *locate1=NULL;
	char *locate2=NULL;
	char buf[64]={0};
	char *ipFile=NULL;
	int ret;

	printf("[debug info:%s,%d]:selectedIPfile========%s\n",__FUNCTION__,__LINE__,selectedIPfile);

	if(access(selectedIPfile,F_OK) == 0){
		fp_ip = fopen(selectedIPfile , "r");
	}else{		
		printf("[debug info:%s,%d]: selectedIPfile is not exsit!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	if(fp_ip == NULL)
		return -1;
	if(fread(buf,1,64,fp_ip)==0){
		fclose(fp_ip);
		return -1;
	}
	fclose(fp_ip);
	
	locate1=buf;
	locate2=strstr(locate1,"\t");
	locate1+=3;
//	debug_info("locate1 is %s\tlocate2 is %s strlen(locate1)-strlen(locate2) is %d",locate1,locate2,strlen(locate1)-strlen(locate2));
	strncpy(ip,locate1,strlen(locate1)-strlen(locate2));
	debug_info("ip is =====%s",ip);

	locate1=locate2+1;
	locate2=strstr(locate1,"\t");
	locate1+=5;
//	debug_info("locate1 is %s\tlocate2 is %s strlen(locate1)-strlen(locate2) is %d",locate1,locate2,strlen(locate1)-strlen(locate2));
	strncpy(mask,locate1,strlen(locate1)-strlen(locate2));
	debug_info("mask is =====%s\n",mask);

	locate1=locate2+1;
	locate2=strstr(locate1,"\n");
	locate1+=8;
//	debug_info("locate1 is %s\tlocate2 is %s strlen(locate1)-strlen(locate2) is %d",locate1,locate2,strlen(locate1)-strlen(locate2));
	strncpy(gateway,locate1,strlen(locate1)-strlen(locate2));
	debug_info("gateway is =======%s\n",gateway);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE!=0)
	wifi_client_auto=1;
#endif

	ret=ifconfigConnection(ip,mask,gateway);
	wifi_client_auto=0;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE!=0)
	if( ret == 0)
	{
		saveManualip(ip,mask,gateway);
		printf("[%s][%d]test\n",__FILE__,__LINE__);
		//wlan_status = WIFI_AUTO_IP_SUCCESS;
	}
#endif
	if(ret == -2){
		printf("[debug info : %s,%d]: ip conflict !!!\n",__FUNCTION__,__LINE__);
		remove(selectedIPfile);
		system("sync");
		memset(selectedIPfile,0,64);
	}
	memset(ip,0,24);
	memset(mask,0,24);
	memset(gateway,0,24);
	return ret;
}

int ifconfigConnection(char *ip, char *mask, char *gateway){

	char callbuf[128]={0};
	int ret = 0;
	int dongle_type;
	
	dongle_type = dongle_get_device_changing();
	//printf("dongle_type=====================%d\n",dongle_type);
	if(dongle_type == REALTEK_dongle){

		ret= check_wifi_ip_arping(ip,dongle_type);
		printf("ret======%d\n",ret);
		if(ret!=0){
			ret=-2;
		//	clear_wifi_error(dongle_type);
			return ret;
		}		
		
		memset(callbuf,0,128);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		if(vid_pid_from_driver==0x148f8189){
			sprintf(callbuf,"ifconfig wlan2 %s netmask %s up",ip,mask) ;
		}
		else
#endif
			sprintf(callbuf,"ifconfig wlan0 %s netmask %s up",ip,mask) ;
		printf("the callbuf is %s\n",callbuf);
		ret = system(callbuf);	
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);		
		if(ret != 0){
			//	sprintf(callbuf,"ifconfig wlan0 down") ;				//if set error , then disconnect the ap 
				ret=-1;
			//	system(callbuf);
				return ret;
			}
		memset(callbuf,0,128);
		sprintf(callbuf,"route add default gw %s",gateway) ;
		if(wifi_client_auto==0)
			ret=system(callbuf);
		if(ret!=0){
				sprintf(callbuf,"route del default gw %s",gateway);
				system(callbuf);
				return ret;
		//		find_wifi_gateway(ip);
		//		memcpy(gateway,wlan_current_status.router,50);
		//		sprintf(callbuf,"route add default gw %s",gateway) ;
		//		ret=pac_system(callbuf);
				
		}	
		sleep(2);
		
		char *pub_interface=NULL;
		char *private_interface=NULL;
		//char *private_subnet="192.168.111.1/24";
		
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
					puts("use ezcastpro subnet 192.168.168.1/24");
					char *private_subnet="192.168.168.1/24";
#else
					char *private_subnet="192.168.203.1/24";
#endif
#else
					char *private_subnet="192.168.111.1/24";
#endif
		

		printf("[%s][%d]vid_pid_from_driver=%x\n",__func__,__LINE__,vid_pid_from_driver);
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

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
		enable_route_function(pub_interface,"br0",private_subnet,WIFI_ROUTING);
		printf("[%s][%d]wifiBridgeProcess\n",__func__,__LINE__);
#else
		enable_route_function(pub_interface,private_interface,private_subnet,WIFI_ROUTING);
#endif
	}
	
	else if(dongle_type == RALINK_dongle){
		
		ret= check_wifi_ip_arping(ip,dongle_type);
		//ret=read_wifi_arping_reply();
		printf("ret======%d\n",ret);
		if(ret!=0){
			ret=-2;
		//	clear_wifi_error(dongle_type);
			return ret;
		}
		
		memset(callbuf,0,128);
		sprintf(callbuf,"ifconfig ra0 %s netmask %s up",ip,mask) ;
		printf("the callbuf is %s\n",callbuf);		
		ret = system(callbuf);	
		
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);	
		
		if(ret != 0){
				sprintf(callbuf,"ifconfig ra0 down") ;
				ret=-1;
				system(callbuf);
				return ret;
			}

		sprintf(callbuf,"route add default gw %s",gateway) ;
		ret=system(callbuf);
		if(ret!=0){
				sprintf(callbuf,"route del default gw %s",gateway);
				system(callbuf);
				return ret;
		//		find_wifi_gateway(ip);
		//		memcpy(gateway,wlan_current_status.router,50);
		//		sprintf(callbuf,"route add default gw %s",gateway) ;
		//		ret=pac_system(callbuf);
			}	
		sleep(2);
				
		char *pub_interface="eth0";
		char *private_interface="ra0";
		//char *private_subnet="192.168.111.1/24";
		
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
		puts("use ezcastpro subnet 192.168.168.1/24");
		char *private_subnet="192.168.168.1/24";
#else
		char *private_subnet="192.168.203.1/24";
#endif
#else
		char *private_subnet="192.168.111.1/24";
#endif
		enable_route_function(pub_interface,private_interface,private_subnet,WIFI_ROUTING);
	}
	debug_info("ret is %d",ret);
	return ret;

}

int delete_ipfile(int sel_ap)
{
	int rtn=0;
	char ip_file[64]={0};
	struct wifi_info_t *wifi_info = __get_wifiinfo();
	
	if(wifi_info==NULL){
		wifidbg_info("sav ap entry error, wifi_info=0x%x",wifi_info);
		rtn = -1;
		return rtn;
	}

	wifi_sem_wait(&wifi_info->syn_lock.semi_apentry);
	if(wifi_info->ap_entry==NULL || sel_ap<0 || sel_ap>=wifi_info->AP_num){
		rtn = -1;
		goto _end;
	}

	sprintf(ip_file,"/mnt/vram/wifi/%x.ip",APHash(wifi_info->ap_entry[sel_ap].ssid));	
		
	if(ip_file==NULL){
		rtn = -1;
		goto _end;
	}
	if(access(ip_file,F_OK)==0){
		remove(ip_file);
		system("sync");
		debug_info("delete file is %s",ip_file);
		rtn = 1;
	}
	memset(ip_file,0,64);
_end:
	wifi_sem_post(&wifi_info->syn_lock.semi_apentry);
	return rtn;
}


#endif	/** MODULE_CONFIG_NETWORK */
