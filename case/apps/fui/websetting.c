#include <sys/un.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "websetting.h"
#include <signal.h>
#include <errno.h>
#include <sys_gpio.h>
#include "display.h"
#include "swf_ext.h"
#include "math.h"
#include "ezcast_public.h"
#include "ez_json.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "ez_json.h"
#include "config.h"
#include "am_net_snmp.h"
#include <fcntl.h>
#include "am7x_dac.h"
#include <openssl/md5.h>
#include "wifi_engine.h"
#include "swfdec.h"
#include "apps_vram.h"
#include "swf_types.h"

#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif
#define APPINFO_FILE		"/tmp/appInfo.json"
#define F_OK 0 
#define SWF_MSG_KEY_LOAD_LANGUAGE 	0x1C4
#define SWF_MSG_KEY_SETPSK                	0x1C8
#define SWF_MSG_KEY_OTA_DOWNLOAD  	0x1C9
#if EZMUSIC_ENABLE
#define AE_ERR_NO_ERROR   		0 		///< audio player no error
#define AE_PLAY 					1		///music engine state: play
#define AE_STOP                     		5 		///< play status:stop
#define AE_ERROR  				6		///< music engine state: error
#endif
#if WEBSETTING_ENABLE
#define CARD_STATE_RUNUING  	2 		///< running
#define CARD_STATE_FINISHED   	3 		///< finished
#define UPGRADE_MODE                    2
#endif
#define wifiChannel_mode_24G 0
#define wifiChannel_mode_5G 1

#if WEBSETTING_ENABLE
int Selection_EQ_Value[19][10]={{50,50,41,34,34,34,41,50,50,50},
			{25,31,44,50,50,66,69,69,50,50},
			{50,50,50,50,50,50,50,50,50,50},
			{50,50,50,50,50,50,69,69,69,76},
			{38,22,36,60,57,46,38,25,17,12},
			{23,23,34,34,50,63,63,63,50,50}, 
			{31,31,50,50,50,50,50,50,31,31}, 
			{55,38,31,30,36,53,57,57,55,55}, 
			{50,50,52,66,50,33,33,50,50,50}, 
			{30,38,65,71,60,39,26,22,22,22}, 
			{38,46,53,57,53,39,28,25,22,19}, 
			{57,63,61,52,39,34,26,25,22,25}, 
			{25,25,25,34,46,61,73,77,79,79}, 
			{39,39,44,52,61,65,60,52,42,26}, 
			{76,76,76,61,42,22,9,9,9,6}, 
			{31,34,50,69,63,46,28,22,19,19}, 
			{63,50,39,36,34,34,39,42,42,44}, 
			{30,34,50,65,63,50,30,25,25,26}, 
			{50,50,50,50,50,50,50,50,50,50}};
struct CGIsetting_priv_s{
	int socket_ser_fd;
	int set_wifi_flag;
	int check_OTA_flag_set_wifi;
	char *sendline;
	char *language_string;
	char *g5country_string;
	char *ota_upgrade_string;
	char *up_progress_text;
	int set_resolution_timeout_tick;
	int current_resolution_index;
	struct itimerval set_resolution_timeout_timer;
	struct itimerval OTA_START_LED_twinkle_timer;
	char result_str[150];
	char encrypted_psk[128];
	int hdmimode;
	int exit;
	int OTA_START_LED_twinkle_tick;
	int last_resolution;
	pthread_t looper;
	char *warning_tone_p[14];
	unsigned int  	AUDIO_EN:1,
				SPDIF_EN:1,
				I2S_EN:1,
				play_warning_tone_thread_exit:1,
				xx:28;
};


typedef struct channel_5g_region
{
	unsigned int  total_5gchannel;
	unsigned int  channel_plan[40];	
}channel5g_tbl;

channel5g_tbl channel5g_regiontbl[]={
	{4,{36,40,44,48}},//South Africa
	{9,{36,40,44,48,149,153,157,161,165}},//Australia
	{9,{36,40,44,48,149,153,157,161,165}},//Canada
	{5,{149,153,157,161,165}},//ÖÐ¹ú
	{14,{36,40,44,48,52,56,60,100,104,108,112,116,132,136,140}},//Europe
	{9,{36,40,44,48,149,153,157,161,165}},//Ó¡¶È
	{4,{36,40,44,48}},//ÒÔÉ«ÁÐ
	//{18,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140}},//ÈÕ±¾
	{4,{36,40,44,48}},//ÈÕ±¾
	{8,{36,40,44,48,149,153,157,161}},//º«¹ú
	{9,{36,40,44,48,149,153,157,161,165}},//ÂíÀ´Î÷ÑÇ
	{9,{36,40,44,48,149,153,157,161,165}},//Ä«Î÷¸ç
	{5,{149,153,157,161,165}},//ÖÐ¶«(Iran/Labanon/Qatar)
	{5,{149,153,157,161,165}},
	{5,{149,153,157,161,165}},
	{4,{36,40,44,48}},//ÖÐ¶«(Turkey/Egypt/Tunisia/Kuwait)
	{4,{36,40,44,48}},
	{4,{36,40,44,48}},
	{4,{36,40,44,48}},
	{9,{36,40,44,48,149,153,157,161,165}},//ÖÐ¶«(Saudi Arabia)
	{4,{36,40,44,48}},//United Arab Emirates
	{4,{36,40,44,48}},//¶íÂÞË¹
	{9,{36,40,44,48,149,153,157,161,165}},//ÐÂ¼ÓÆÂ
	{9,{36,40,44,48,149,153,157,161,165}},//ÄÏÃÀÖÞ
	{8,{56,60,64,149,153,157,161,165}},//Ì¨Íå
	{9,{36,40,44,48,149,153,157,161,165}}//ÃÀ¹ú

};


struct CGIsetting_priv_s CGIsetting_priv;
static int websettingInitOK=0;
struct EQsetting_priv_s{
	int EQ_socket_cli_fd;
	struct sockaddr_un EQ_unc;  
};
struct EQsetting_priv_s EQsetting_priv;
                                                   
const char lan_value_item[][6]= {"en","fr","de","es","pl","zh-CN","zh-TW","ja","ko","it","cs","da","ru","nl","fi","no","pt","hu","ro","sk","tr","sv","el","ar", "id", "he", "tha", "fa"};
const char language_item[][20]= {"English", "français", "Deutsch", "español", "polski", "简体中文", "繁體中文", "日本語", "한국어", "italiano", "čeština", "Dansk", "русский", "Nederlands", "Suomi", "Norsk", "português", "magyar", "română", "Slovenský", "Türk", "Svenska", "ελληνικά", "العربية", "Bahasa Indonesia", "עברית", "ไทย", "فارسی", "IDS_SETTING_AUTO"};
const char Channel_5G_country[][25]= {"South Africa", "Australia", "Canada", "中国", "Europe", "भारत", "ישראל", "日本", "한국", "Malaysia", "México", "ایران", "لبنان", "قطر", "Türkiye", "مصر", "تونس", "الكويت", "مملكة عربية سعودية", "الأمارات العربية المتحدة", "Россия", "Singapore", "South America", "台湾", "American"};

extern int app_total;
extern void *deinst;
extern int wifidisplay_onoff;
recv_ip_info_t recv_ip_info[64];
recv_ip_info_t last_recv_ip_info[64];
int total_num=0;
int last_total_num=0;
int hSocket; 
int first_start_flag=1;
#if EZMUSIC_ENABLE
#define WarningTone_PATH      "/mnt/user1/warningtone/"
#define WarningTone_PATH_Default_language      "/mnt/user1/warningtone/en/"
static char WarningTone_1_Default[]=WarningTone_PATH_Default_language"en_Hello please wait for initialization.mp3";
static char WarningTone_2_Default[]=WarningTone_PATH_Default_language"en_Connecting to home router please wait.mp3";
static char WarningTone_3_Default[]=WarningTone_PATH"connect_router_waiting.mp3";
static char WarningTone_4_Default[]=WarningTone_PATH_Default_language"en_Home router connected successfully.mp3";
static char WarningTone_5_Default[]=WarningTone_PATH_Default_language"en_Home router connected unsuccessfully.mp3";
static char WarningTone_6_Default[]=WarningTone_PATH_Default_language"en_Now ready for use with internet connected.mp3";
static char WarningTone_7_Default[]=WarningTone_PATH_Default_language"en_Now ready for use without internet connected.mp3";
static char WarningTone_8_Default[]=WarningTone_PATH_Default_language"en_Upgrading firmware please wait.mp3";
static char WarningTone_9_Default[]=WarningTone_PATH"otaupgrade_waiting.mp3";
static char WarningTone_10_Default[]=WarningTone_PATH_Default_language"en_Firmware upgraded successfully.mp3";
static char WarningTone_11_Default[]=WarningTone_PATH_Default_language"en_Firmware upgraded unsuccessfully.mp3";
static char WarningTone_12_Default[]=WarningTone_PATH_Default_language"en_Home router disconnected please reconnect.mp3";
static char WarningTone_13_Default[]=WarningTone_PATH_Default_language"en_New firmware version please check setting.mp3";
static char WarningTone_14_Default[]=WarningTone_PATH_Default_language"en_Bye Bye.mp3";
int  EZMUSIC_FIRST_BOOT=1;	//for the first boot without internet connected warnning

#endif
static void start_timer(struct itimerval timer,int timer_delay)
{
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = timer_delay;
	timer.it_value.tv_usec = 0;
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
static int check_confexist(int index)
{
	int ret;
	ret=ezcast_wifi_getconfexist(index);
	return  ret;
}
static int GetAuthenType(int index)
{
	int ssid_AuthenType=-1;
	ssid_AuthenType=ezcast_wifi_getAuthenType(index);
	printf("ssid_AuthenType========%d\n",ssid_AuthenType);

	return ssid_AuthenType;
}
static void ok_btn(char *tmp_ssid, char *tmp_passwd,int index)
{
	int ssid_AuthenType=ezcast_wifi_getAuthenType(index);
	if(ssid_AuthenType==2//AUTHEN_OPEN
        || ssid_AuthenType==1//AUTHEN_WEP
        || ssid_AuthenType==0//AUTHEN_WPA
         )
	{
		ezcast_wifi_setAuthenType(index, ssid_AuthenType);
	}
	ezcast_wifi_setssid(index,tmp_ssid);
	ezcast_wifi_setpassword(index,tmp_passwd);
	int rtn=ezcast_wifi_saveconf();
	if(rtn==0)
	{
		ezcast_wifi_connectAP();
		printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
		SysCGI_priv.connect_router_waiting_lan_flag=1;
		SysCGI_priv.connect_router_waiting_flag=1;
    	}
}

static void check_connect_status(void)
{
	int ret;
	int wifi_mode_get_from_case=ezcast_sysinfo_get_wifi_mode();
	char *tmp_ssid= ezcast_wifi_get_connected_ssid();
	if(tmp_ssid!=NULL)
	{
		if((wifi_mode_get_from_case==9)||((wifi_mode_get_from_case==14)&&(strlen(tmp_ssid) != 0)))//9:WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK   14:WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR
		{
			sprintf(CGIsetting_priv.result_str,"%d",9);
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
		}		
	}
}
static int check_auto_connect(void)
{
	int ret;
	int wifi_mode_get_from_case=ezcast_sysinfo_get_wifi_mode();
	char *tmp_ssid= ezcast_wifi_get_connected_ssid();
	if(tmp_ssid!=NULL)
	{	
		if((wifi_mode_get_from_case==9)||((wifi_mode_get_from_case==14)&&(strlen(tmp_ssid) != 0)))//9:WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK   14:WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR
		{
			ret=ssid_connect_ok_show();
			return ret;
		}
		else	
		{
			int wlan_status=ezcast_wifi_getStatus();
			if((wlan_status==9)//WIFI_CONNECT_FAILED
			||(wlan_status==7)//WIFI_AUTO_IP_ERROR
	         	||((wlan_status!=0)&&(wlan_status!=8))//WIFI_COMPLETED    WIFI_AUTO_IP_SUCCESS
	         	)
			{		
				if(CGIsetting_priv.set_wifi_flag==1)
				{
					CGIsetting_priv.set_wifi_flag=0;	
					return 2;		
				}
			}
			return 0;
		}
	}
	return -1;
}
static void get_ssid_string(int index)
{
	//printf("SysCGI_priv.ssid_str[%d]=========%s\n",index,SysCGI_priv.ssid_str[index]);
	if(!(strstr(SysCGI_priv.ssid_str[index],"\n")))
	{		
		if(2==GetAuthenType(SysCGI_priv.ssid_index[index]))//OPEN
		{		
			char * connected_ssid=ezcast_wifi_get_connected_ssid();
			if(0!=strcmp(connected_ssid,SysCGI_priv.ssid_str[index]))
			{
				ezcast_wifi_disconnect_ap();
				ezcast_wifi_setAuthenType(index,2);//AUTHEN_OPEN
				ezcast_wifi_setssid(index,SysCGI_priv.ssid_str[index]);
				ezcast_wifi_setpassword(index,"********");
				int rtn=ezcast_wifi_saveconf();
				if(rtn==0)
				{
					ezcast_wifi_connectAP();
			    	}	
			}	
			strcat(SysCGI_priv.ssid_str[index],"\n");
			strcat(SysCGI_priv.ssid_str[index],"O");				
		}
		else if(check_confexist(SysCGI_priv.ssid_index[index]))
		{
			strcat(SysCGI_priv.ssid_str[index],"\n");
		}		
	}
	CGIsetting_priv.sendline=SysCGI_priv.ssid_str[index];
}

static int Get_OTA_version(void)
{
	if(!CGIsetting_priv.ota_upgrade_string)
	{
		CGIsetting_priv.ota_upgrade_string=(char *)SWF_Malloc(100*sizeof(char));
	}
	memset(CGIsetting_priv.ota_upgrade_string,0,100);		
	char *server_version_string=ezcast_sysinfo_get_ota_server_version();
	char *local_version_string=ezcast_sysinfo_get_ota_local_version();
	strncpy(CGIsetting_priv.ota_upgrade_string,local_version_string,strlen(local_version_string));
	strcat(CGIsetting_priv.ota_upgrade_string,"\n");
	if(!strcmp(server_version_string,"error"))
		strcat(CGIsetting_priv.ota_upgrade_string,"newest");
	else
		strcat(CGIsetting_priv.ota_upgrade_string,server_version_string);
	strcat(CGIsetting_priv.ota_upgrade_string,"\n");
	CGIsetting_priv.sendline=CGIsetting_priv.ota_upgrade_string;
	printf("CGIsetting_priv.sendline===========%s,%d\n",CGIsetting_priv.sendline,__LINE__);	
	return 1;
}
static int set_de_defaultcolor()
{
	DE_config ds_conf;
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.enable=0;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}
static void swf_sleep()
{
	/** sleep SWF */
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}	
	set_de_defaultcolor();
}
static int Send_FLA_Key(int keyCode)
{
	int ret;
	ret = SWF_Message(NULL, keyCode, NULL);
	//printf("ret=================%d,%s\n",ret,__FUNCTION__);
	return ret;
}
#if (EZMUSIC_ENABLE&&GPIO_POWER_ON_LED)
static void OTA_START_LED_twinkle()
{
	CGIsetting_priv.OTA_START_LED_twinkle_tick^=1;
	//printf("EZmusic_LED_priv.WIFI_CLIENT_OK_LED_twinkle_tick============%d,%s,%d\n",CGIsetting_priv.OTA_START_LED_twinkle_tick,__FUNCTION__,__LINE__);
	if(CGIsetting_priv.OTA_START_LED_twinkle_tick)
		set_gpio(GPIO_POWER_ON_LED,0);//PWR-ON-LED
	else
		set_gpio(GPIO_POWER_ON_LED,1);//PWR-OFF-LED
}
#endif
static int Download_OTA_binfile(void)
{	
	#if EZMUSIC_ENABLE
	char *ota_url="http://www.iezvu.com/upgrade/ezcast_music/ezcast_music.gz";
	#else
	char *ota_url="http://www.iezvu.com/upgrade/ezcast/ezcast.bin";
	#endif
	char ota_upgrade_ret=ezcast_sysinfo_ota_upgrade(1,ota_url);	
	if(ota_upgrade_ret==0)
	{
		ota_upgrade_ret=1;
		#if EZMUSIC_ENABLE	
		ezcast_sysinfo_StopNetDisplay();
		//SysCGI_priv.mainswf_skip_get_keycode_flag=1;
		#endif
		Send_FLA_Key(SWF_MSG_KEY_OTA_DOWNLOAD);		
		#if EZMUSIC_ENABLE		
		if(0==SysCGI_priv.otaupgrade_waiting_lan_once)
		{
			SysCGI_priv.otaupgrade_waiting_lan_once=1;
			SysCGI_priv.otaupgrade_waiting_lan_flag=1;
		}
		SysCGI_priv.otaupgrade_waiting_flag=1;
		#endif		
		#if (EZMUSIC_ENABLE&&GPIO_POWER_ON_LED)
		start_timer(CGIsetting_priv.OTA_START_LED_twinkle_timer,1);
		signal(SIGALRM, OTA_START_LED_twinkle);	
		#endif

		return 1;
	}
	return -1;
}



int json_write_to_string(JSON *retx){
	char *str = NULL;
	str = JSON_Print(retx);
	strcat(CGIsetting_priv.language_string,str);
	//printf("---json_write_to_file-666=======%d%s\n",__LINE__,CGIsetting_priv.language_string);
	if(str != NULL) ezJSON_free(str);
	return 1;
}

static int Get_main_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	char name[256]={0};
	int i=0;
	char number[1000]={0},tmp[1000]={0};
	char  i_string="";
	JSON *js= NULL;
	
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_MAIN_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			js = JSON_CreateObject();
			JSON_AddStringToObject(js, name, tmp);
			}
		else
			
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MAIN_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MAIN_LAN2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");


		string_temp=ezcast_get_string("IDS_MENU_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN2");//username
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_STTING_PAS");//password
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN3");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN4");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN5");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN6");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN7");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN8");//Conference control

		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "main_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		
		json_write_to_string(js);
		if(js != NULL) JSON_Delete(js);

		//printf("---11111111111=======%d\n",i);
		//printf("---11111111111=======%s\n",number);
		//printf("---11111111111=======%s\n",CGIsetting_priv.language_string);
		
		//strcat(number,CGIsetting_priv.language_string);
		//sprintf(CGIsetting_priv.language_string, "%d:%s", i,number);
		
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);

	/*
		char *str_tmp = strchr(CGIsetting_priv.sendline, ':');
		int tmp_num[50];
		if(str_tmp != NULL){
			printf("[%s-%d] %s\n", __func__, __LINE__, str_tmp);
			char tmp_buf[256];
			memset(tmp_buf, 0, sizeof(tmp_buf));
			memcpy(tmp_buf, CGIsetting_priv.sendline, str_tmp-CGIsetting_priv.sendline);
			int i = 0, all_num = atoi(tmp_buf);
			char *tmp2 = NULL;
			for(i=0; i<all_num; i++){
				tmp2 = strchr(++str_tmp, ':');
				memset(tmp_buf, 0, sizeof(tmp_buf));
				memcpy(tmp_buf, str_tmp, tmp2-str_tmp);
				printf("[%s-%d] %s\n", __func__, __LINE__, tmp_buf);
				tmp_num[i] = atoi(tmp_buf);
				str_tmp = tmp2;
			}
			str_tmp++;
			for(i=0; i<all_num-1; i++){
				memset(tmp_buf, 0, sizeof(tmp_buf));
				memcpy(tmp_buf, str_tmp, tmp_num[i]);
				str_tmp += tmp_num[i];
				printf("[%s-%d] %s\n", __func__, __LINE__, tmp_buf);
			}
		}
*/
		
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_menu_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	char name[256]={0};
	int i=0;
	char number[1000]={0},tmp[1000]={0};
	char  i_string="";
	JSON *js= NULL;

//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_MENU_LAN9");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			js = JSON_CreateObject();
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN10");//12
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN11");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN12"); //16
		string_length+=strlen(string_temp);
	//	printf("---------IDS_STTING_EZMUSIC_OUTPUT------------%s%d\n",string_temp,__LINE__);  //ok
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN13");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN14");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN16");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_DIALOG_OK");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_DIALOG_CAN");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN17"); //Device Management
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_MENU_LAN18");//Netword Setup
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		string_temp=ezcast_get_string("IDS_STTING_ADMINSETTING");//Admin Setting
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");	
		string_temp=ezcast_get_string("IDS_STTING_ABOUT");//about
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");	
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN7");//Password is not correct!
		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			sprintf(name, "%s%d", "menu_string",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			json_write_to_string(js);
			if(js != NULL) JSON_Delete(js);

			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		//strcat(number,":");
		
		//printf("---2222222222=======%d\n",i);
		//printf("---2222222222=======%s\n",number);
		//printf("---2222222222=======%s\n",CGIsetting_priv.language_string);
		
		//strcat(number,CGIsetting_priv.language_string);
		//sprintf(CGIsetting_priv.language_string, "%d:%s", i,number);
		
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}


static int Get_Config_Val()
{
	JSON *js= NULL;
         printf("Start Get_Config_Val  \n");
	js = JSON_CreateObject();
	if (js != NULL)
	{
			
		char json_string[10];

		sprintf(json_string,"%d",AirView_ONOFF);
		JSON_AddStringToObject(js, "AirView_ONOFF", json_string);

		sprintf(json_string,"%d",ConferenceCtl_ONOFF);;
		JSON_AddStringToObject(js, "ConferenceCtl_ONOFF", json_string);

		sprintf(json_string,"%d",LinkStaus_ONOFF);;
		JSON_AddStringToObject(js, "LinkStaus_ONOFF", json_string);

		sprintf(json_string,"%d",AirSetup_ONOFF);;
		JSON_AddStringToObject(js, "AirSetup_ONOFF", json_string);
		
		sprintf(json_string,"%d",AddCA_ONOFF);;
		JSON_AddStringToObject(js, "AddCA_ONOFF", json_string);
		
		sprintf(json_string,"%d",HostAuthority_ONOFF);;
		JSON_AddStringToObject(js, "HostAuthority_ONOFF", json_string);

		sprintf(json_string,"%d",AccessCtl_ONOFF);;
		JSON_AddStringToObject(js, "AccessCtl_ONOFF", json_string);
		
		sprintf(json_string,"%d",PasswordModify_ONOFF);;
		JSON_AddStringToObject(js, "PasswordModify_ONOFF", json_string);
		
		sprintf(json_string,"%d",RebootCtl_ONOFF);;
		JSON_AddStringToObject(js, "RebootCtl_ONOFF", json_string);
		
		sprintf(json_string,"%d",ResetToDef_ONOFF);;
		JSON_AddStringToObject(js, "ResetToDef_ONOFF", json_string);
	
		char *str = NULL;
	       str = JSON_Print(js);
		memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
              strcat(CGIsetting_priv.result_str,str);
		strcat(CGIsetting_priv.result_str,"\n");		
	       if(str!=NULL)
				ezJSON_free(str);
	       if(js != NULL) JSON_Delete(js);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		printf("Get_Config_Val  CGIsetting_priv.sendline=%s\n",CGIsetting_priv.sendline);
				
	}
}
static int Get_airview_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_CBV_LAN");

		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		//printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}
static int Get_ezwire_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_ADB");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_ADB_WARN");

		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		//printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_paswod_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN3");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN5");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN6");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN8");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_EZMUSIC_PSW_ALLOW");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN9");//Password must be between 6 and 64 characters long.	
		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_conference_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN3");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN5");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN6");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_CONFERENCE_LAN7");	
		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_fileupload_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_WIFIENTERPRISE");//WiFi Enterprise
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_UPLOAD_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_UPLOAD_LAN2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_UPLOAD_LAN3");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_UPLOAD_LAN4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_UPLOAD_LAN5");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_UPLOAD_LAN6");	
		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_authority_host_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_NETWORK_CONTROL");//Network setup control
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_DEVICE_CONTROL");	//Device manager control
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN5");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN6");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN7");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN8");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN9");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN10");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN11");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN12");		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_control_access_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN3");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN5");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN6");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN7");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN8");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN9");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETTING24");	//OFF
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETTING28");	//Random
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_ACCESS_CON_LAN10");	//Fixed
	
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_serverota_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_OTA_LAN");
	
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_rebot_rest_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN3");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN5");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN6");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETYes");
				string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETNo");
	
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}

static int Get_lan_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_LANSET0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LANSET1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LANSET2");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LANSET3");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LANSET4");	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LANSET5");//Network cable is plugged in and turn off wireless scanning! 	
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LANSET7");

		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_CHANNELSET1");//Channel 
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_CHANNELSET2");//Country
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_BANDWIDTH");//Bandwidth:
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");

		string_temp=ezcast_get_string("IDS_STTING_CHANNELSET3");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		
		string_temp=ezcast_get_string("IDS_STTING_WIFIP");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_HOST_AUTHORITY_LAN5");//WIFIĜëӾ²֍
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");		
		string_temp=ezcast_get_string("IDS_STTING_LANIP");//LAN IP Setting
	
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		//printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	return 1;
}
//for 8268
#if defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE > 29 && MODULE_CONFIG_CHIP_TYPE < 40)
static int Get_webmultilanguage_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		
		string_temp=ezcast_get_string("IDS_SETTING0");  //Setting
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_INT");  //Internet
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_PAS"); //Password
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_RES");  //Resolution
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LAN"); //Language
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_HOMEPAGE");//Default Mode
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_EZCAST_SRT4");//EZAir mode
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_EZCAST_SRT6");//Mirror Only
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_EZCAST_SRT7"); //Mirror+Streaming
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_CHANGES_EFFECT"); //Changes take effect after reboot.
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_UPG"); //Upgrade
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_OK");//OK
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_CAN");//Cancel
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_PASSWORD_WARN");//Please enter a new password to save again
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_MENU_LAN16");//Reset to default
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN0");//Reboot
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");

		string_temp=ezcast_get_string("IDS_STTING_WAITING"); //Please wait
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		
		string_temp=ezcast_get_string("IDS_WIFI_LIST_WARN3"); //not wifi connect!
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		
		string_temp=ezcast_get_string("IDS_new_version_msg1"); //No new version!



		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
	//	printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}
#endif

static int Get_index_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_RES");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_PAS");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_INT");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_LAN");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_HOMEPAGE");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_UPG");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_PASSWORD_WARN");//Please enter a new password to save again
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_OK");//9
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_CAN");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_CTL");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_TR");//12:Via Router Allowed
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_DL");//Direct Link Only
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_RO");//Via Router Only
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_EZMUSIC_OUTPUT"); //16
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_PASSWORD_LAN10"); //Admin Password
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_WIFIENTERPRISE"); //WiFi Enterprise
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_HIDESSID"); //Hide SSID
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_CHANGES_EFFECT"); //Changes take effect after reboot.
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_MAIN_REBOOT"); //New setting will take effect after reboot!
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_EZCAST_SRT4");//EZAir mode
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");

		string_temp=ezcast_get_string("IDS_EZCAST_SRT6");//Mirror Only
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_EZCAST_SRT7");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_AIRDISKAUTOPLAY");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_AUTOPLAY");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_HDMIAUDIO");//HDMI Audio:
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_HDMICEC_WARN");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");		
		string_temp=ezcast_get_string("IDS_STTING_REFRESH_RETE");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_second_mc");//seconds
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETTING34");//Minute
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_MANUALUPDATA");//Manual update
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		
		string_temp=ezcast_get_string("IDS_STTING_MIRACODE_TIT");//Miracode control
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_MAXCONNECTIONS");//Max Connections
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_USERS");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_MAXUSER_WARN");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		
		string_temp=ezcast_get_string("IDS_STTING_bottom0");//info
		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
	//	printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}


static int Get_router_warn_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	char name[256]={0};
	int i=0;
	char number[1000]={0},tmp[1000]={0};
	JSON *js= NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_CTL_WARN0");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "router_warn",i);
			sprintf(tmp, "%s", string_temp);
			js = JSON_CreateObject();
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_CTL_WARN1");
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "router_warn",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		//strcat(CGIsetting_priv.language_string,"\n");

		
		string_temp=ezcast_get_string("IDS_STTING_RO_WARN1");

		
		EZCASTLOG("string_temp: %s\n", string_temp);
		char *tp = strchr(string_temp, '\n');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("n is not found\n");
		tp = strchr(string_temp, '\r');
		if(tp != NULL)
			EZCASTLOG("tp: %s\n", tp);
		else
			EZCASTLOG("r is not found\n");

		//printf("---IDS_STTING_CTL_WARN1---%d%d\n",string_temp[strlen(string_temp)+1],string_temp[strlen(string_temp)+2]);
		string_length+=strlen(string_temp);
		if(string_temp&&(string_length+1<1000)){
			i++;
			//sprintf(tmp,"%d",strlen(string_temp));
			//strcat(number,tmp);
			//strcat(CGIsetting_priv.language_string,string_temp);
			sprintf(name, "%s%d", "router_warn",i);
			sprintf(tmp, "%s", string_temp);
			JSON_AddStringToObject(js, name, tmp);
			}
		
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}

		json_write_to_string(js);
		if(js != NULL) JSON_Delete(js);
		
		//strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
		EZCASTLOG("len: %d\n", strlen(CGIsetting_priv.language_string));
		printf("---sendline=======%s\n",CGIsetting_priv.sendline);
		EZCASTLOG("n: %d, n-1: %d, n-2: %d\n", CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-1], CGIsetting_priv.sendline[strlen(CGIsetting_priv.sendline)-2]);
	}
	/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}

static int Get_wifi_list_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_WAITING");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_AP");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;	
	}
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	return 1;
}
static int Set_wifi_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_OK");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_CAN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_FOR");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_WARN2");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_WAITING");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_EZMUSIC_FORGET_PSW");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}

		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_SECURITY");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETTING_ADD");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		//printf("sendline==========%s\n",CGIsetting_priv.sendline);
	}
//	else 
//	{
//		CGIsetting_priv.sendline=NULL;
//		return -1;
//	}
	return 1;
}
static int Get_wifilist_text(void)
{
	int string_length=0;
	char *string_temp=NULL;

	memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
	string_temp=ezcast_get_string("IDS_DIALOG_OK");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_DIALOG_CAN");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_DIALOG_FOR");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_WIFI_LIST_WARN2");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_STTING_WAITING");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_STTING_EZMUSIC_FORGET_PSW");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}

	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_WIFI_LIST_SECURITY");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_SETTING_ADD");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_REBOOT_RESET_LAN1");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_wifi_setup_txt");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_SETTING0");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_scan_wifi_txt");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	strcat(CGIsetting_priv.language_string,"\n");
	string_temp=ezcast_get_string("IDS_STTING_PAS");
	if(string_temp&&(string_length+1<1000))
		strcat(CGIsetting_priv.language_string,string_temp);
	else
	{
		printf("not enought buffer for string !%d\n",__LINE__);
		return -1;			
	}
	
	strcat(CGIsetting_priv.language_string,"\n");
	CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	//printf("sendline==========%s\n",CGIsetting_priv.sendline);

	return 1;
}
static int Add_network_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_WIFI_LIST_SECURITY");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SETTING_ADD"); //Add network
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
	//	printf("========Add_network_text=========%s\n",CGIsetting_priv.language_string);
/*
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_OK");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_CAN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_WARN2");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_Main0");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_Main1");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		*/
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		
		//printf("sendline==========%s\n",sendline);
	}
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	return 1;
}
static int Get_OTA_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_MAIN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_LOCAL_VERSION_ATTR");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_SERVER_VERSION_ATTR");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_WARN2");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_WAITING_DOWNING");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_UPGRADE_WARN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
	/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}
static int Get_OTA_faild_connect_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_WARN1");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_WARN3");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		//printf("CGIsetting_priv.sendline======%s\n",CGIsetting_priv.sendline);
	}
	/*
	else 
	{
	
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}
static int Get_OTA_warn1_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_WARN0");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_WARN3");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		//printf("CGIsetting_priv.sendline======%s\n",CGIsetting_priv.sendline);
	}
/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}
static int Get_OTA_warn2_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_WARN2");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_UPG_WARN3");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		//printf("CGIsetting_priv.sendline======%s\n",CGIsetting_priv.sendline);
	}
/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
}
static void send_scanresults(void)
{
	if(SysCGI_priv.ssid_string_for_cgi!=NULL)
		CGIsetting_priv.sendline=SysCGI_priv.ssid_string_for_cgi;	
}
static void get_ap_name(char *cmd)
{
	char *ssid_index=NULL;
	int ssid_index_temp;
	char setcmd_tmp[30]={0};
	memset(setcmd_tmp,0,sizeof(setcmd_tmp));
	strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_ap_name")));
	//printf("setcmd_tmp=========%s\n",setcmd_tmp);
	sscanf(setcmd_tmp, "%d", &ssid_index_temp);
	get_ssid_string(ssid_index_temp);
}
static int set_lan_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	//if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_LAN_MAIN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_WAITING");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");		
		string_temp=ezcast_get_string("IDS_DIALOG_SR");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_SO");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_DIALOG_CAN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_HOSTNAME");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_HN_MAIN1");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_DEVICENAME_SET1");//The device name can not be more 
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_DEVICENAME_SET2");//Only numbers and letters are allowed!
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		string_temp=ezcast_get_string("IDS_STTING_DEVICENAME_SET3");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");	
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		//printf("sendline===========%s,%d\n",sendline,__LINE__);
	}
	/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
	//printf("sendline===========%s\n",sendline);
}
static int resolution_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_RES_MAIN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_RES_SEC");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_RES_ADD");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_RES_WARN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}

		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
	//printf("CGIsetting_priv.sendline===========%s\n",CGIsetting_priv.sendline);
}

static int eq_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_STTING_EZMUSIC_MODE");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_DIALOG_RES_SEC");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	return 1;
	//printf("CGIsetting_priv.sendline===========%s\n",CGIsetting_priv.sendline);
}
	
static int Get_last_UI(void)
{
	int ret=ezcast_sysinfo_get_last_ui();
	return ret;
}
static int Get_Default_mode_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_DIALOG_HP_MAIN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	return 1;
	//printf("CGIsetting_priv.sendline===========%s\n",CGIsetting_priv.sendline);	
}
static int Wifi_connect_fail_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_WIFI_LIST_CONNECT_WARN");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_CONNECT_WARN1");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_CONNECT_WARN2");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_CONNECT_WARN3");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	return 1;
	//printf("CGIsetting_priv.sendline===========%s\n",CGIsetting_priv.sendline);	
}
static int new_version_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
//	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_new_version_msg");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
/*
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	*/
	return 1;
	//printf("CGIsetting_priv.sendline===========%s\n",CGIsetting_priv.sendline);	
}
static int websetting_get_ignoressid(char *cmd,char *val)
{
	char *CMD=cmd;
	static char var_array[128]={0};
	
	val=wifi_get_ignoressid_flag();
    if (val !=NULL)    {
	    printf("websetting_get_ignoressid  val=%s \n",val);
	    strncpy(var_array,val,sizeof(var_array));
    }
    else{
        /* Mos: if wifi_get_ignoressid_flag can not find the flag, and return Null, it will cause system crash */
        printf("websetting_get_ignoressid  can not find the flag, assume 0");
        strncpy(var_array,"0",sizeof("0"));
    }
	
	CGIsetting_priv.sendline=var_array;	
	
	if(val){
		free(val);
		val = NULL;
	}
	return 1;

}
static int websetting_set_ignoressid(char *cmd,char *val)
{
	char setcmd_tmp[30]={0};
	char *psk=NULL;
	char *VAL=val;
	int ret=0;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_ignoressid")));
	else
		return -1;
	sscanf(setcmd_tmp, "%d", &ret);
	ezCastSetNumConfig(CONFNAME_IGNORESSID,ret);
	printf("websetting_set_ignoressid  ignore_ssid=%s  \n",setcmd_tmp);
	wifi_set_ignoressid_flag(setcmd_tmp);  
	return 1;
}
static int websetting_set_softap_psk_ignoressid(char *cmd,char *val)
{
	char setcmd_tmp[30]={0};
	char *psk=NULL;
	char *ignore_ssid=NULL;
	char *VAL=val;
	int ret=0;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_psk_hidessid")));
	else
		return -1;
	ignore_ssid=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");
//	printf("websetting_set_softap_psk_hidessid  ignore_ssid=%s  psk=%s\n",ignore_ssid,psk);
	wifi_set_ignoressid_flag(ignore_ssid);  
	sscanf(ignore_ssid, "%d", &ret);
	ezCastSetNumConfig(CONFNAME_IGNORESSID,ret);
	ezcast_change_psk(psk);
	Send_FLA_Key(SWF_MSG_KEY_SETPSK);
	return 1;
}
static int websetting_get_softap_psk(char *cmd,char *val)
{
	char *CMD=cmd;
	static char var_array[128]={0};
	val=ezcast_wifi_get_softap_psk();
	strcpy(var_array,val);
	CGIsetting_priv.sendline=var_array;	
	if(val){
		free(val);
		val = NULL;
	}
	return 1;
}
static int websetting_set_softap_psk(char *cmd,char *val)
{
	char setcmd_tmp[30]={0};
	char *VAL=val;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_softap_psk")));
	else
		return -1;
	ezcast_change_psk(setcmd_tmp);
	Send_FLA_Key(SWF_MSG_KEY_SETPSK);
	return 1;
}
static int websetting_set_softap_ssid(char *cmd,char *val)
{
	char setcmd_tmp[32]={0};
	char *VAL=val;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_softap_ssid")));
	else
		return -1;
	ezcast_set_custom_ssid(setcmd_tmp);
	Send_FLA_Key(SWF_MSG_KEY_SETPSK);
	return 1;
}
static int websetting_delete_password(char *cmd,char *val)
{
	int delete_password_status=-1;
	char *ssid_index=NULL;
	char *ssid=NULL;
	char setcmd_tmp[50]={0};
	int ssid_index_temp;
	int ret;
	char *VAL=val;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("delete_password")));
	else
		return -1;
	ssid_index=strtok(setcmd_tmp,"\n");
	ssid=strtok(NULL,"\n");
	sscanf(ssid_index, "%d", &ssid_index_temp);
	ret=ezcast_wifi_getconfexist(ssid_index_temp);
	if(ret==1)
	{
		char ssid_str_tmp[50];
		memset(ssid_str_tmp,0,sizeof(ssid_str_tmp));
		//printf("SysCGI_priv.ssid_str[ssid_index_temp]=========%s,%d\n",SysCGI_priv.ssid_str[ssid_index_temp],__LINE__);
		strncpy(ssid_str_tmp,SysCGI_priv.ssid_str[ssid_index_temp],(strlen(SysCGI_priv.ssid_str[ssid_index_temp])-strlen("\n")));
		memset(SysCGI_priv.ssid_str[ssid_index_temp],0,sizeof(SysCGI_priv.ssid_str[ssid_index_temp]));
		strcpy(SysCGI_priv.ssid_str[ssid_index_temp],ssid_str_tmp);
		//printf("SysCGI_priv.ssid_str[ssid_index_temp]=========%s,%d\n",SysCGI_priv.ssid_str[ssid_index_temp],__LINE__);
		
		ret = ezcast_wifi_disconnect_ap();
		if(ret == 0 || strcmp(ssid, ezcast_wifi_get_connected_ssid()) != 0)
		{
			if(!ezcast_wifi_deleteconf(ssid_index_temp))
			{
				printf("-delete password success!\n");
				delete_password_status=1;		
			#if defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1
				SWF_Message(NULL, SWF_MSG_KEY_Z, NULL);
			#endif
			}
			else
			{
				printf("-delete password error!\n");
				delete_password_status=0;
			}		
		}
		else
		{
			printf("Can not disconnect current AP.\n");
		}
	}
	return delete_password_status;
}
#if EZMUSIC_ENABLE
static void change_warning_tone_source(char dest1,char dest2)
{
	int i;
	for(i=0;i<14;i++)
	{
		if((i!=2)&&(i!=8))
		{
			*(CGIsetting_priv.warning_tone_p[i]+30)=dest1;
			*(CGIsetting_priv.warning_tone_p[i]+31)=dest2;	
			*(CGIsetting_priv.warning_tone_p[i]+33)=dest1;
			*(CGIsetting_priv.warning_tone_p[i]+34)=dest2;	
		}	
	}	
}
static void check_multilanguage_for_warning_tone(int language_id)
{
	switch(language_id)
	{
		case 0:
			change_warning_tone_source('e','n');
			break;
		case 1:
			change_warning_tone_source('f','r');
			break;
		case 2:
			change_warning_tone_source('d','e');
			break;
		case 3:
			change_warning_tone_source('e','s');
			break;
		case 4:
			change_warning_tone_source('p','l');
			break;
		case 5:
			change_warning_tone_source('z','T');
			break;
		case 6:
			change_warning_tone_source('z','T');
			break;
		case 7:
			change_warning_tone_source('j','a');
			break;
		case 8:
			change_warning_tone_source('k','o');
			break;
		case 9:
			change_warning_tone_source('i','t');
			break;
		case 10:
			change_warning_tone_source('c','s');
			break;
		case 11:
			change_warning_tone_source('d','a');
			break;
		case 12:
			change_warning_tone_source('r','u');
			break;
		case 13:
			change_warning_tone_source('n','l');
			break;
		case 14:
			change_warning_tone_source('f','i');
			break;
		case 15:
			change_warning_tone_source('n','o');
			break;
		case 16:
			change_warning_tone_source('p','t');
			break;
		//case 17:
			//change_warning_tone_source('h','u');
			//break;
		//case 18:
			//change_warning_tone_source('r','o');
			//break;
		//case 19:
			//change_warning_tone_source('s','k');
			//break;
		case 20:
			change_warning_tone_source('t','r');
			break;
		case 21:
			change_warning_tone_source('s','v');
			break;
		//case 22:
			//change_warning_tone_source('e','l');
			//break;
		case 23:
			change_warning_tone_source('a','r');
			break;
		case 24:
			change_warning_tone_source('i','d');
			break;
		//case 25:
			//change_warning_tone_source('h','e');
			//break;
		case 26:
			change_warning_tone_source('t','h');
			break;
		//case 27:
			//change_warning_tone_source('f','a');
			//break;
	}
}
#endif
static int websetting_set_language(char *cmd,char *val)
{
	int lan_index_tmp;
	char *VAL=val;
	char *lan_index=NULL;
	lan_index=strtok(cmd,"\n");
	lan_index=strtok(NULL,"\n");
	sscanf(lan_index, "%d", &lan_index_tmp);
	if(lan_index_tmp<sizeof(lan_value_item)/sizeof(lan_value_item[0]))
	{
		ezcast_sysinfo_set_local_language(lan_index_tmp);
		SysCGI_priv.language_index=lan_index_tmp;
		ezcast_sysinfo_set_disauto_status(1);
		ezcast_sysinfo_store_config();
		ezcast_sysinfo_json_set_value(5,(char *)lan_value_item[lan_index_tmp]);
		//ezcast_set_language((char *)lan_value_item[lan_index_tmp]);
		ezcast_set_language_swf_name((char *)lan_value_item[lan_index_tmp],"ezcast");
		ezcast_sysinfo_Set_language_index(lan_index_tmp);
		Send_FLA_Key(SWF_MSG_KEY_LOAD_LANGUAGE);
		#if EZMUSIC_ENABLE
		check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
		#endif
	}
	else
	{
		ezcast_sysinfo_set_disauto_status(0);
		ezcast_sysinfo_store_config();
	}
	return 1;
}

void websetting_connect(char *ssid, char *psk, int index){
	int ret = -1;
	
	ret=ezcast_wifi_getconfexist(index);
	if(ret==1)
	{
		if(ezcast_wifi_connectAP()==0)
		{
			printf("connect_router_waiting======%s,%d\n",__FUNCTION__,__LINE__);
			SysCGI_priv.connect_router_waiting_lan_flag=1;
			SysCGI_priv.connect_router_waiting_flag=1;
		}
	}
	else
	{
	ezcast_wifi_disconnect_ap();
	ok_btn(ssid,psk,index);
	}	
}

static int websetting_set_wifi(char *cmd,char *val)
{
	char setcmd_tmp[150]={0};
	char *VAL=val;
	char *ssid=NULL;
	char *psk=NULL;
	char *ssid_index=NULL;
	int ssid_index_temp;
	int index;
	int ret;
	CGIsetting_priv.set_wifi_flag=1;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_wifi")));
	else
		return -1;
	ssid=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");
	ssid_index=strtok(NULL,"\n");
	if(strlen(ssid_index)>3)
	{
		return 0;
	}
	sscanf(ssid_index, "%d", &ssid_index_temp);
	if(ssid_index_temp>app_total)
		return 0;
#if EZCAST_LITE_ENABLE
	#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1) 
	if(is_wifi_connect_ready() == 0)
	{
		SWF_Message(NULL, SWF_MSG_KEY_WIFIMODE_CHANGE, NULL);//added by Denny 

		int waitCount = 0;
		while(is_wifi_connect_ready() == 0)
		{
			if(waitCount > 8)
				return 1;
			
			sleep(1);
			waitCount++;
		}
	}
	#else
	if(is_wifi_connect_ready() == 0){
		ret = to_connect(ssid, psk, ssid_index_temp);
		if(ret == 0)
			return 1;
		return 0;
	}
	#endif
#endif
#if EZMUSIC_ENABLE
	SysCGI_priv.connections_num=ezcast_wifi_count_accessDevices();
	if((1==SysCGI_priv.stream_play_status)&&(1==SysCGI_priv.connections_num))
		SysCGI_priv.stream_play_status=0;
#endif
	websetting_connect(ssid, psk, ssid_index_temp);

	return 1;
}

// ------------------------------------------------------
//                            3G/4G mode connect.
// ------------------------------------------------------

struct hotspotInfo{
	char ssid[128];
	char psk[128];
};

struct hotspotPthreadStatus_s{
	int pthreadRunning;
	int pthreadExit;
};

struct hotspotStatus_s{
	int isHotApStore;
	int connectStatus;
	unsigned int prevCheckTime;
	struct hotspotInfo apInfo;
};

struct hotspotPthreadStatus_s hotspotPthreadStatus;
struct hotspotStatus_s hotspotStatus;

static int getHotspotStoreStatus(){
	return hotspotStatus.isHotApStore;
}

static int getPrevHotspotCheckTime(){
	return hotspotStatus.prevCheckTime;
}

static int setHotspotCheckTime(){
	return (hotspotStatus.prevCheckTime = time(NULL));
}

static void hotspotInfoStore(const struct hotspotInfo *apInfo){
	memcpy(&hotspotStatus.apInfo, apInfo, sizeof(struct hotspotInfo));
	hotspotStatus.isHotApStore = 1;
}

void setHotspotConnectStatus(int status){
	hotspotStatus.connectStatus = status;
	setHotspotCheckTime();
}

static int hotspotStatusCheck(){
	if(getHotspotStoreStatus() != 1){
		if(hotspotStatus.connectStatus == HOTSPOT_READY && get_hotspot_connect_status() != 0){
			int curTime = time(NULL);
			if((curTime - get_hotspot_connect_status()) > HOTSPOT_READY_TIMEOUT){
				setHotspotConnectStatus(HOTSPOT_UNCONNECT);
				hotspotStatus.prevCheckTime = 0;
				return 1;
			}
		}else{
			setHotspotConnectStatus(HOTSPOT_UNCONNECT);
			hotspotStatus.prevCheckTime = 0;
			return 1;
		}
	}

	int ret = 2;
	if(hotspotStatus.connectStatus == HOTSPOT_SUCCESS){
		int curTime = time(NULL);
		int prevTime = getPrevHotspotCheckTime();
		if(curTime - prevTime > 5){
			char curSsid[128];
			setHotspotCheckTime();
			if(getConnectedSsid(curSsid, sizeof(curSsid)) == 0){
				if(strcmp(curSsid, hotspotStatus.apInfo.ssid) != 0){
					setHotspotConnectStatus(HOTSPOT_UNCONNECT);
					ezCastRouterCtlInit();
				}
			}else{
				setHotspotConnectStatus(HOTSPOT_UNCONNECT);
				ezCastRouterCtlInit();
			}
			ret = 0;
		}
	}

	return ret;
}

int getHotspotStatus(){
	hotspotStatusCheck();
	
	return hotspotStatus.connectStatus;
}

static int do_hotspot_connect(char *ssid, char *psk, int index){
	int curTime = time(NULL);
	int connectCount = 0;
	int ret;

	EZCASTLOG("To connect ssid: %s, psk: %s, index: %d\n", ssid, psk, index);
	websetting_connect(ssid, psk, index);
	setHotspotConnectStatus(HOTSPOT_CONNECTING);
	int startConnectTime = set_hotspot_connect_status();
	ret = -1;

	do{
		if(hotspotPthreadStatus.pthreadExit != 0){
			ret = 3;
			break;
		}
		sleep(2);

		int conStatus = ezcast_wifi_getStatus();
		EZCASTLOG("conStatus: %d\n", conStatus);
		if(conStatus > 8){
			EZCASTLOG("connect fail!!!\n");
			setHotspotConnectStatus(HOTSPOT_FAIL);
			ret = 1;
			break;
		}else if(conStatus == 7 || conStatus == 8){
			EZCASTLOG("connect success!!!\n");
			ezCastSetRouterCtlEnable(1);
			setHotspotConnectStatus(HOTSPOT_SUCCESS);
			ret = 0;
			break;
		}

		curTime = time(NULL);
		EZCASTLOG("curTime: %d, startConnectTime: %d\n", curTime, startConnectTime);
		if((curTime - startConnectTime) > HOTSPOT_CON_TIMEOUT){
			EZCASTLOG("connect timeout!!!\n");
			ret = 2;
			break;
		}
		
	}while(1);

	return ret;
}

static void *pthread_hotspot_connect(void *arg){
	if(arg == NULL){
		goto __END__;
	}

	hotspotPthreadStatus.pthreadRunning = 1;
	int index = -1;
	int ret = -1;
	struct hotspotInfo conInfo;
	memcpy(&conInfo, arg, sizeof(conInfo));
	EZCASTLOG("conInfo.ssid: %s, conInfo.psk: %s\n", conInfo.ssid, conInfo.psk);
	int startConnectTime = get_hotspot_connect_status();

	//do scan
	int startScanTime = time(NULL);
	int curTime = startScanTime;
	int scanCount = 0;
	int conCount = 0;
	setHotspotConnectStatus(HOTSPOT_READY);
	hotspotInfoStore(&conInfo);
__SCAN__:
	do{
		if(hotspotPthreadStatus.pthreadExit != 0){
			goto __END__;
		}
		curTime = time(NULL);
		if(curTime - startConnectTime < HOTSPOT_SCAN_READY_TIME){
			sleep(HOTSPOT_SCAN_READY_TIME - (curTime - startConnectTime));
			continue;
		}
		EZCASTLOG("start scan!!!\n");
		ezcast_wifi_startscan();
		setHotspotConnectStatus(HOTSPOT_SCAN);
		startScanTime = time(NULL);
		scanCount++;
		break;
	}while(1);
	
__SCAN_RESULT__:
	do{
		if(hotspotPthreadStatus.pthreadExit != 0){
			goto __END__;
		}
		curTime = time(NULL);
		if(curTime - startConnectTime > (HOTSPOT_SCAN_READY_TIME + HOTSPOT_SCAN_TIMEOUT*9)){
			EZCASTLOG("Can not find the hotspot[%s]!!!\n", conInfo.ssid);
			goto __END__;
		}
		if(curTime - startScanTime > HOTSPOT_SCAN_TIMEOUT){
			EZCASTLOG("Scan timeout!!!\n");
			goto __SCAN__;
		}
		sleep(2);
		int apNum = ezcast_wifi_getscanresults();
		if(apNum > 0){
			index = get_index_ssid(conInfo.ssid);
			if(index < 0){
				EZCASTLOG("Not found!!!\n");
				goto __SCAN__;
			}
			
			ret = do_hotspot_connect(conInfo.ssid, conInfo.psk, index);
			if(ret != 0){
				if(conCount < 3){
					conCount++;
					EZCASTLOG("connect again!!!\n");
					goto __SCAN__;
				}
				EZCASTLOG("Can not connect[count: %d]!!!\n", conCount);
			}
			hotspotApinfoClean();
			// Delete client connection infomation after hotspot connect.
			/*
 			int hssid = APHash(conInfo.ssid);
			char filename[256];
			memset(filename,0,256);
			sprintf(filename,"/mnt/vram/wifi/%x.conf",hssid);
			unlink(filename);
			sync();
			*/
			break;
		}
	}while(1);

__END__:
	if(hotspotStatus.connectStatus != HOTSPOT_FAIL && hotspotStatus.connectStatus != HOTSPOT_SUCCESS && hotspotStatus.connectStatus != HOTSPOT_TIMEOUT)
		setHotspotConnectStatus(HOTSPOT_UNCONNECT);
	hotspotPthreadStatus.pthreadRunning = 0;
	pthread_exit(NULL);
	return NULL;
}

int hotspot_connect(char *ssid, char *psk){
	int ret = -1;
	pthread_t tidp;
	struct hotspotInfo conInfo;

	if(ssid == NULL || psk == NULL){
		EZCASTWARN("ssid or psk error!!!\n");
		return -1;
	}
	snprintf(conInfo.ssid, sizeof(conInfo.ssid), "%s", ssid);
	snprintf(conInfo.psk, sizeof(conInfo.psk), "%s", psk);

	hotspotInfoStore(&conInfo);
	EZCASTLOG("[%d:%d]SSID: %s, PSK: %s\n", hotspotPthreadStatus.pthreadRunning, hotspotPthreadStatus.pthreadExit, conInfo.ssid, conInfo.psk);
	do{
		if(hotspotPthreadStatus.pthreadRunning != 0){
			hotspotPthreadStatus.pthreadExit = 1;
		}else{
			hotspotPthreadStatus.pthreadExit = 0;
			break;
		}
		sleep(1);
	}while(1);
	ret = pthread_create(&tidp, NULL, pthread_hotspot_connect, &conInfo);
	if (ret != 0)
	{
		EZCASTWARN("pthread_create error(%d), FIXME!\n", ret);
		perror("pthread_create");
		ret = -1;
	}
	
	ret = pthread_detach(tidp);
	if(ret != 0){
		EZCASTWARN("pthread_detach error!!!\n");
		perror("pthread_detach");
	}

	return ret;
}

static int websetting_set_hotspot_ap(char *cmd,char *val)
{
	char setcmd_tmp[40]={0};
	char *VAL=val;
	char *ssid=NULL;
	char *psk=NULL;
	char psk_decrypt[128]={0};
	int ret;
	CGIsetting_priv.set_wifi_flag=1;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_hotspot_ap")));
	else
		return -1;
	ssid=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");
	EZCASTLOG("ssid: %s, psk: %s\n", ssid, psk);
#if MODULE_CONFIG_LAN
	if(get_netlink_status() != 0){
		EZCASTLOG("LAN is ON, disable wifi connect!!!\n");
		return 0;
	}
#endif
	ret = ezcastPskDecrypt(psk_decrypt, psk, sizeof(psk_decrypt));
	if(ret < 0){
		EZCASTWARN("psk decrypt error!!!\n");
		return -1;
	}
	EZCASTLOG("ssid: %s, psk: %s\n", ssid, psk_decrypt);
	set_hotspot_connect_status();
	setHotspotConnectStatus(HOTSPOT_READY);
#if EZCAST_LITE_ENABLE
	#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1) 
	if(is_wifi_connect_ready() == 0)
	{
		SWF_Message(NULL, SWF_MSG_KEY_WIFIMODE_CHANGE, NULL);//added by Denny 

		int waitCount = 0;
		while(is_wifi_connect_ready() == 0)
		{
			if(waitCount > 8)
				return 0;
			
			sleep(1);
			waitCount++;
		}
	}
	#else
	if(is_wifi_connect_ready() == 0){
		ezRemoteSendKey(0x1CA);
		hotspotApinfoStore(ssid, psk_decrypt);
		return 1;
	}
	#endif
#endif

	hotspot_connect(ssid, psk_decrypt);

	return 1;
}

void websetting_add_connect(char *ssid, char *psk, int AuthenType_temp){
	char setcmd_tmp[40]={0};
	
	printf("ssid===========%s\n",ssid);
	if(ssid != ezcast_wifi_get_connected_ssid())
	{
		printf("AuthenType_temp==========%d,%d\n",AuthenType_temp,__LINE__);
		ezcast_wifi_disconnect_ap();
		ezcast_wifi_addnetwork();
		ezcast_wifi_setssid(app_total,ssid);
		ezcast_wifi_setAuthenType(app_total,AuthenType_temp);
		printf("psk==========%s\n",psk);
		ezcast_wifi_setpassword(app_total,psk);
		if(ezcast_wifi_saveaddedAPconf()==0)
		{
			ezcast_wifi_connectAP();
		}
		else
		{
			printf("save config file failture!\n");
		}
	}

}
static int websetting_add_network(char *cmd,char *val)
{
	char *VAL=val;
	char setcmd_tmp[40]={0};
	char *ssid=NULL;
	char *psk=NULL;
	char *AuthenType=NULL;
	int AuthenType_temp;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("add_network")));
	else
		return -1;

    /* Mos: dirty workaround to avoid AuthenType get null string when psk is empty */
    if (setcmd_tmp[strlen(setcmd_tmp)-2] == '2'){
        ssid=strtok(setcmd_tmp,"\n");
        psk="";
        AuthenType_temp=2;
    }
    else{
    	ssid=strtok(setcmd_tmp,"\n");
	    psk=strtok(NULL,"\n");
    	AuthenType=strtok(NULL,"\n");
	    sscanf(AuthenType, "%d", &AuthenType_temp);
    }
	
#if EZCAST_LITE_ENABLE
	#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1) 
	if(is_wifi_connect_ready() == 0)
	{
		SWF_Message(NULL, SWF_MSG_KEY_WIFIMODE_CHANGE, NULL);//added by Denny 

		int waitCount = 0;
		while(is_wifi_connect_ready() == 0)
		{
			if(waitCount > 8)
				return 0;
			
			sleep(1);
			waitCount++;
		}
	}
	#else
	if(is_wifi_connect_ready() == 0){
		int ret = to_connect_add(ssid, psk, AuthenType_temp);
		if(ret == 0)
			return 1;
		return 0;
	}
	#endif
#endif
	websetting_add_connect(ssid, psk, AuthenType_temp);

	return 1;
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
static int  websetting_add_network_set_802XEPA_conf(char *cmd,char *val)
{
	char *VAL=val;
	char setcmd_tmp[40]={0};
	char *ssid=NULL;
	char *psk=NULL;
	char *AuthenType=NULL;
	char *eap_key_psk=NULL;
	char *eap_identity=NULL;
	int ret=0;
	int AuthenType_temp;
	printf("set_802XEPA_conf====00000000=====%s,%d\n",cmd,__LINE__);

	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("add_802XEPA_network")));
	else
		return -1;
	ssid=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");
	printf("set_802XEPA_conf====111111111=====%s,%s\n",ssid,psk);
	eap_identity=strtok(NULL,"\n");
	eap_key_psk=strtok(NULL,"\n");
	AuthenType=strtok(NULL,"\n");
	printf("set_802XEPA_conf====222222=====%s,%s,%s\n",eap_identity,eap_key_psk,AuthenType);
	sscanf(AuthenType, "%d", &AuthenType_temp);
	printf("set_802XEPA_conf====33333=====%d\n",AuthenType_temp);
	ezcast_wifi_disconnect_ap();
	ezcast_wifi_addnetwork();
	if(save_addedAP_conffile()==0){
		ret=ezcast_wifi_full_802XEPA_conf(AuthenType_temp);
		if(ret==1) printf("802.1xEAp config file open error!!!");
		printf("set_802XEPA_conf====ret=====%d\n",ret);
		
		if(ssid != ezcast_wifi_get_connected_ssid())
		{
			printf("AuthenType_temp==========%d,%d\n",AuthenType_temp,__LINE__);
			//set data
			ezcast_wifi_set_802XEPA_conf(ssid,1);
			ezcast_wifi_set_802XEPA_conf(psk,2);
			if(AuthenType_temp==3){
				ezcast_wifi_set_802XEPA_conf("TLS",3);
				ezcast_wifi_set_802XEPA_conf(eap_key_psk,5);
			}else{
				ezcast_wifi_set_802XEPA_conf("PEAP",3); 
			}
			ezcast_wifi_set_802XEPA_conf(eap_identity,4);	
				
			//if(ezcast_wifi_saveaddedAPconf()==0)
			{
				ezcast_wifi_connectAP();
			}
			//else
			{
				printf("save config file failture!\n");
			}
		}

	}
}

#endif
static void set_resolution_timeout_function()
{

	int outputmode=-1,outputformat=-1;
	CGIsetting_priv.set_resolution_timeout_tick++;
	if(CGIsetting_priv.set_resolution_timeout_tick>=15)
	{	

		//if(CGIsetting_priv.current_resolution_index<20) {outputmode=1; outputformat=CGIsetting_priv.current_resolution_index;}
		//else if(CGIsetting_priv.current_resolution_index<30) {outputmode=8; outputformat=CGIsetting_priv.current_resolution_index-20;}
		//else { outputmode=2; outputformat=CGIsetting_priv.current_resolution_index-30; }
		//printf("set_resolution_timeout_function format =%d",outputformat);
#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)
		if(CGIsetting_priv.current_resolution_index>=200)//vga flag;
			CGIsetting_priv.current_resolution_index-=200;
		else if(CGIsetting_priv.current_resolution_index>=100)
			CGIsetting_priv.current_resolution_index-=100;
		else
			;	
#endif
		printf("set_resolution_timeout_function CGIsetting_priv.current_resolution_index =%d",CGIsetting_priv.current_resolution_index);
		ezcast_sysinfo_set_HDMI_mode(CGIsetting_priv.current_resolution_index);
		CGIsetting_priv.set_resolution_timeout_tick=0;
		stop_timer(CGIsetting_priv.set_resolution_timeout_timer);
	}
}
static int websetting_set_resolution(char *cmd,char *val)
{
	char *VAL=val;
	int resolution_index_tmp;
	char *resolution_index=NULL;
	char *current_resolution_index_p=NULL;
	int reboot_flag=-1;
	int outputmode=-1,outputformat=-1;
	printf("websetting_set_resolution cmd=%s",cmd);
	/*
	cmd=set_resolution   ok  
	6@
	OK


	cmd=set_resolution  cancel
	17  resolution for set back
	0cur_resolution

	cmd=set_resolution  resolution select
	17     //
	5  cur_resolution

	*/
	if(strstr(cmd,"OK") != 0) //ok button 
	{
		char temp[5];
		reboot_flag=1;
		strncpy(temp,cmd,(strlen(cmd)-2));		
		resolution_index=strtok(temp,"\n");
		resolution_index=strtok(NULL,"\n");	
		
		CGIsetting_priv.set_resolution_timeout_tick=0;
		stop_timer(CGIsetting_priv.set_resolution_timeout_timer);
	}
	else
	{
		resolution_index=strtok(cmd,"\n");
		resolution_index=strtok(NULL,"\n");
		current_resolution_index_p=strtok(NULL,"\n");
		//cancel or time out use  current_resolution_index_ to set back
		printf("current_resolution_index_p=========%s\n",current_resolution_index_p);
		sscanf(current_resolution_index_p, "%d", &CGIsetting_priv.current_resolution_index);
		if(0!=CGIsetting_priv.current_resolution_index)  //selset  ,start timer ,after timer reach,use
		//index_val to set, use  current_resolution_index to set back after 15s
		{
			start_timer(CGIsetting_priv.set_resolution_timeout_timer,1);
			signal(SIGALRM, set_resolution_timeout_function);				
		}
		else  //cancel  use resolution_val to set back and store
		{
			CGIsetting_priv.set_resolution_timeout_tick=0;
			stop_timer(CGIsetting_priv.set_resolution_timeout_timer);
		}
	}	
	sscanf(resolution_index, "%d", &resolution_index_tmp);
	//resolution_index_tmp=atoi(resolution_index);
	//if(resolution_index_tmp<20) {outputmode=1; outputformat=resolution_index_tmp;}
	//else if(resolution_index_tmp<30) {outputmode=8; outputformat=resolution_index_tmp-20;}
	//else { outputmode=2; outputformat=resolution_index_tmp-30; }
	printf("websetting_set_resolution format =%d",resolution_index_tmp);
	#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)
		int last_res=0,real_res=0;
		int hdmi_tovga_format=0;
		if(resolution_index_tmp>=200){outputmode=8;outputformat=resolution_index_tmp-200;}
		else if	(resolution_index_tmp>=100){outputmode=1;outputformat=resolution_index_tmp-100;}
		else {outputmode=1;outputformat=resolution_index_tmp;}
		ezcast_set_output_mode(outputmode);	
		//ezcast_sysinfo_set_HDMI_mode(outputformat);
		ezcast_sysinfo_set_HDMI_mode(outputformat);
		if(reboot_flag==1)
		{	
			if(resolution_index_tmp>=200){
				printf("save VGA EDID!\n");
				printf("outputformat=%d\n",outputformat);
				ezcast_sysinfo_creat_edid_bin(0,0,1,outputformat);//vga
			}
			else if(resolution_index_tmp>=100){//hdmi &&vga  turn hdmi format value to vga format value
				if(outputformat==2)hdmi_tovga_format=9;
				else if(outputformat==17)hdmi_tovga_format=6;
				else if(outputformat==16)hdmi_tovga_format=5;
				else if(outputformat==5)hdmi_tovga_format=8;
				else if(outputformat==15)hdmi_tovga_format=2;
				else hdmi_tovga_format=8;
				printf("save VGA && HDMI EDID!\n");
				ezcast_sysinfo_creat_edid_bin(1,outputformat,1,hdmi_tovga_format);
			}

			else{
				printf("save HDMI EDID!\n");
				ezcast_sysinfo_creat_edid_bin(1,outputformat,0,0);
			}
			if(2==outputformat ||4==outputformat||17==outputformat)
				real_res=1;
			if(CGIsetting_priv.last_resolution==2 || CGIsetting_priv.last_resolution==4||CGIsetting_priv.last_resolution==17)
				last_res=1;
			if(real_res != 1 || last_res != 1)
				system("reboot");
			else 
				return -1; 
		}	
		
	#else
	ezcast_sysinfo_set_HDMI_mode(resolution_index_tmp);
	if(reboot_flag==1)
	{	
		ezcast_sysinfo_creat_edid_bin(1,resolution_index_tmp,0,0);
		if(((4==CGIsetting_priv.last_resolution)&&(2==resolution_index_tmp))||((2==CGIsetting_priv.last_resolution)&&(4==resolution_index_tmp)))// 4: FMT_2750x1125_1920x1080_24P  2:FMT_2200x1125_1920x1080_30P
			return 1;
		else if(((17==CGIsetting_priv.current_resolution_index)&&(2==resolution_index_tmp))||((2==CGIsetting_priv.current_resolution_index)&&(17==resolution_index_tmp)))// 17: FMT_2750x1125_1920x1080_60P  2:FMT_2200x1125_1920x1080_30P
			return 1;
		else
			system("reboot");
	}	
	#endif
	return 1;
}
static int websetting_set_default_mode(char *cmd,char *val)
{
	char *VAL=val;
	int default_mode_index_tmp;
	char *default_mode_index=NULL;
	int reboot_flag=-1;
	int last_index=0;
	default_mode_index=strtok(cmd,"\n");
	default_mode_index=strtok(NULL,"\n");	
	sscanf(default_mode_index, "%d", &default_mode_index_tmp);
	last_index=ezcast_sysinfo_get_last_ui();
	ezcast_sysinfo_set_last_ui(default_mode_index_tmp,1);
	#if (MODULE_CONFIG_EZCASTPRO_MODE==8075) 
	if(last_index!=default_mode_index_tmp)
		ezRemoteSendKey(0x1CB);
	#endif
	return 1;
}
static int websetting_get_5G_country(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	char *string_temp=NULL;
	int i;     
	int string_length=0;
	memset(CGIsetting_priv.g5country_string,0,1000*sizeof(char));
	if(CGIsetting_priv.g5country_string!=NULL)
	{	
		for(i=0;i<(sizeof(Channel_5G_country)/sizeof(Channel_5G_country[0]));i++)
		{
			string_length+=strlen(Channel_5G_country[i]);
			string_length++;
		}	
		printf("string_length=========%d\n",string_length);
		if(string_length>1000)
		{		
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;
		}		
		for(i=0;i<(sizeof(Channel_5G_country)/sizeof(Channel_5G_country[0]));i++)
		{
			strcat(CGIsetting_priv.g5country_string,Channel_5G_country[i]);
			strcat(CGIsetting_priv.g5country_string,"\n");			
		}
	}
	if(string_temp!=NULL)
	{
		strcat(CGIsetting_priv.g5country_string,string_temp);
		strcat(CGIsetting_priv.g5country_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.g5country_string;
	}
	CGIsetting_priv.sendline=CGIsetting_priv.g5country_string;
	return 1;
}

static int websetting_get_language(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	char *string_temp=NULL;
	int i;     
	int string_length=0;
	if(CGIsetting_priv.language_string!=NULL)
	{	
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		for(i=0;i<(sizeof(language_item)/sizeof(language_item[0]))-1;i++)
		{
			string_length+=strlen(language_item[i]);
			string_length++;
		}	
		printf("string_length=========%d\n",string_length);
		if(string_length>1000)
		{		
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;
		}		
		for(i=0;i<(sizeof(language_item)/sizeof(language_item[0]))-1;i++)
		{
			strcat(CGIsetting_priv.language_string,language_item[i]);
			strcat(CGIsetting_priv.language_string,"\n");			
		}
		EZCASTLOG("language_string: %s\n", CGIsetting_priv.language_string);
		string_temp=ezcast_get_string("IDS_SETTING_AUTO");
		string_length+=strlen(string_temp);
		if(string_length+1>1000)
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;
		}		
		EZCASTLOG("string_temp: %s\n", string_temp);
		if(string_temp!=NULL)
		{
			strcat(CGIsetting_priv.language_string,string_temp);
			strcat(CGIsetting_priv.language_string,"\n");
			CGIsetting_priv.sendline=CGIsetting_priv.language_string;
		}
		EZCASTLOG("language_string: %s\n", CGIsetting_priv.language_string);
		return 1;
	}
	return -1;
}
static int websetting_ap_index_string(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	get_scanresults();
	CGIsetting_priv.sendline=SysCGI_priv.ssid_index_string;
	return 1;
}
static int websetting_wifi_start_scan(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	static int ScanWifiNum=0;
#if (defined(EZCAST_LITE_ENABLE) && EZCAST_LITE_ENABLE==1) 
	if(is_wifi_scan_ready() == 0)
	{
		EZCASTLOG("WiFi client is not ready, need change mode.\n");
		app_total = 0;
		#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1)
		SWF_Message(NULL, SWF_MSG_KEY_WIFIMODE_CHANGE, NULL);//added by Denny 
		#endif
	}
	else
#endif
	{
		app_total=ezcast_wifi_getscanresults(); 
		printf("[%s][%d]:app_total===========%d\n",__FILE__,__LINE__,app_total);
		if(app_total<=1)
		{
			//usleep(800*1000);
			ScanWifiNum++;
			if(3>ScanWifiNum)
			{
				wifi_start_scan();
				app_total=ezcast_wifi_getscanresults(); 
				printf("[%s][%d]:app_total===========%d\n",__FILE__,__LINE__,app_total);
			}	
#if EZMUSIC_ENABLE
			/* Mos: http://220.128.123.30/mantis/view.php?id=17204
			 * somecase wifi scan step into fail case, then return "OK" to web
			 * But Web does not handle "OK" case, then only show 1 SSID
			 *	
			 * r13459 | liangwenhua | 2015-02-27 17:01:23 +0800 (Fri, 27 Feb 2015) | 2 lines
			 *
			 * Cause this code commit by liangwenhua, look like only for ezmusic
			 * So use macro to enable only for EZMUSIC as workaround
			 */
			else
			{
				strncpy(CGIsetting_priv.result_str,"OK",strlen("OK"));
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
				printf("app_total=====error======%s\n",CGIsetting_priv.result_str);
				return 1;
			}
#endif
		}	
	}
	sprintf(CGIsetting_priv.result_str,"%d",app_total);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

#if EZMUSIC_ENABLE
static void *get_warning_tone_status_thread(void *arg)
{
	while(1)
	{
		if(1==SysCGI_priv.play_warning_tone_flag)
		{
			int cur_status=ezcast_ae_get_state();
			if((cur_status == AE_STOP)||(cur_status ==AE_ERROR)||(1==SysCGI_priv.stream_play_status))
			{
				ezcast_ae_stop();
				ezcast_ae_close();
				SysCGI_priv.play_warning_tone_flag=0;
				if(1==SysCGI_priv.play_connect_router_waiting_start)
				{
					SysCGI_priv.play_connect_router_waiting_start=0;
					SysCGI_priv.play_connect_router_waiting_end=1;
				}	
				if(1==SysCGI_priv.play_otaupgrade_waiting_start)
				{
					SysCGI_priv.play_otaupgrade_waiting_start=0;
					SysCGI_priv.play_otaupgrade_waiting_end=1;
				}
			}	
		}
		sleep(1);	
	}
	printf("play_warn_music_thread  exit start!!!!!!!!!!!!!!!!!!!!!%s,%d\n",__FUNCTION__,__LINE__);
	pthread_exit(NULL);
	return NULL;
}
static void play_warning_tone(char *path)
{
	if(0==SysCGI_priv.stream_play_status)
	{
		int ret=ezcast_ae_open();
		if(!ret)
		{
			CGIsetting_priv.play_warning_tone_thread_exit=1;
			goto end;
		}	
		int audioOK=ezcast_ae_set_file(path);
		if(audioOK==AE_ERR_NO_ERROR)
		{
			ezcast_ae_play();
		}
		else
		{
			ezcast_ae_stop();
			ezcast_ae_close();
			SysCGI_priv.play_warning_tone_flag=0;
			if(1==SysCGI_priv.play_connect_router_waiting_start)
			{
				SysCGI_priv.play_connect_router_waiting_start=0;
				SysCGI_priv.play_connect_router_waiting_end=1;
			}
			if(1==SysCGI_priv.play_otaupgrade_waiting_start)
			{
				SysCGI_priv.play_otaupgrade_waiting_start=0;
				SysCGI_priv.play_otaupgrade_waiting_end=1;
			}
		}				
	}
	else
	{
		end:
		SysCGI_priv.play_warning_tone_flag=0;	
	}		
}
static int get_wifi_conf_num(void)
{
	FILE *fp;
	char l[100];
	int wifi_conf_num=0;
	fp =popen("ls -l /mnt/vram/wifi/ | wc -l", "r");
	if(NULL == fp)
	{
		fprintf(stderr,"function:%s line:%d \n",__FUNCTION__,__LINE__);
		printf("popen error:%s\n",strerror(errno));
		return -1;
	}
	if (fgets(l, sizeof(l), fp) != NULL)
	{
		sscanf(l, "%d", &wifi_conf_num);
	}
	pclose(fp);
	return wifi_conf_num;
}
static void check_router_connect_status(void)
{
	static int wifi_conf_num_last=0;
	char * connected_ssid=ezcast_wifi_get_connected_ssid();	
	if(strlen(connected_ssid))
	{
		if(0==SysCGI_priv.connected_ssid_once)
		{
			SysCGI_priv.connected_ssid_once=1;	
			wifi_conf_num_last=get_wifi_conf_num();
		}
	}		
	else
	{
		if(1==SysCGI_priv.connected_ssid_once)
		{				
			SysCGI_priv.connected_ssid_once=0;
			int wifi_conf_num_current=get_wifi_conf_num();
			if(((wifi_conf_num_last>wifi_conf_num_current)&&(wifi_conf_num_last>=0)&&(wifi_conf_num_current>=0))||((1==wifi_conf_num_last)&&(2==wifi_conf_num_current)))
				SysCGI_priv.router_disconnect_flag=1;
		}
	}
}
static void Play_ota_upgrade_warning_tone(void)
{
	if(1==SysCGI_priv.start_ota_upgrade_flag)
	{
		if(1==SysCGI_priv.play_otaupgrade_waiting_end)
		{
			SysCGI_priv.otaupgrade_waiting_flag=1;
			SysCGI_priv.play_otaupgrade_waiting_end=0;
		}			
	}
}
static void *play_warning_tone_thread(void *arg)
{
	while(0==CGIsetting_priv.play_warning_tone_thread_exit)
	{
		if(0==SysCGI_priv.enter_factory_test_video)
		{
			int cur_status=ezcast_ae_get_state();
			if(AE_PLAY==cur_status)
				goto end;
			if(1==SysCGI_priv.start_audio_flag)
			{
				if(0==SysCGI_priv.play_warning_tone_flag)
				{

				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.start_audio_flag=0;				
				play_warning_tone(WarningTone_1_Default);			
			}
		}	
		else if(1==SysCGI_priv.connect_router_waiting_lan_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.connect_router_waiting_lan_flag=0;
				play_warning_tone(WarningTone_2_Default);			
			}
		}		
		else if(1==SysCGI_priv.connect_router_waiting_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.connect_router_waiting_flag=0;
				SysCGI_priv.play_connect_router_waiting_start=1;
				play_warning_tone(WarningTone_3_Default);
			}
		}
		else if(1==SysCGI_priv.connect_router_success_flag)
		{			
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.connect_router_success_flag=0;
				play_warning_tone(WarningTone_4_Default);												     
			}
		}
		else if(1==SysCGI_priv.connect_router_failed_flag)
		{			
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.connect_router_failed_flag=0;
				play_warning_tone(WarningTone_5_Default);			
			}
		}
		else if(1==SysCGI_priv.connected_router_useful_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.connected_router_useful_flag=0;
				play_warning_tone(WarningTone_6_Default);
			}
		}
		else if(1==SysCGI_priv.not_connected_router_useful_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.not_connected_router_useful_flag=0;
				#if EZMUSIC_ENABLE
				if(EZMUSIC_FIRST_BOOT)
				{
					EZMUSIC_FIRST_BOOT=0;
					play_warning_tone(WarningTone_7_Default);
				}
				#else
					play_warning_tone(WarningTone_7_Default);
				#endif
				
			}
		}
		else if(1==SysCGI_priv.otaupgrade_waiting_lan_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.otaupgrade_waiting_lan_flag=0;
				play_warning_tone(WarningTone_8_Default);			
			}
		}
		else if(1==SysCGI_priv.otaupgrade_waiting_flag)
		{
			if((0==SysCGI_priv.play_warning_tone_flag)&&(0==SysCGI_priv.otaupgrade_failed_and_stop))
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.otaupgrade_waiting_flag=0;
				SysCGI_priv.play_otaupgrade_waiting_start=1;
				play_warning_tone(WarningTone_9_Default);			
			}
		}
		else if(1==SysCGI_priv.otaupgrade_successful_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.otaupgrade_successful_flag=0;
				play_warning_tone(WarningTone_10_Default);			
			}
		}
		else if(1==SysCGI_priv.otaupgrade_failed_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.otaupgrade_failed_flag=0;
				SysCGI_priv.otaupgrade_failed_and_stop=1;
				play_warning_tone(WarningTone_11_Default);			
			}
		}
		else if(1==SysCGI_priv.router_disconnect_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.router_disconnect_flag=0;
				play_warning_tone(WarningTone_12_Default);			
			}
		}
		else if(1==SysCGI_priv.new_firmware_version_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.new_firmware_version_flag=0;
				play_warning_tone(WarningTone_13_Default);			
			}
		}
		else if(1==SysCGI_priv.Bye_bye_flag)
		{
			if(0==SysCGI_priv.play_warning_tone_flag)
			{
				SysCGI_priv.play_warning_tone_flag=1;
				SysCGI_priv.Bye_bye_flag=0;
				play_warning_tone(WarningTone_14_Default);			
			}
		}
		check_router_connect_status();
		Play_ota_upgrade_warning_tone();
		}
		end:
		sleep(1);
	}
	printf("play_warn_music_thread  exit start!!!!!!!!!!!!!!!!!!!!!%s,%d\n",__FUNCTION__,__LINE__);
	pthread_exit(NULL);
	return NULL;
}

void Create_play_warning_tone_thread(void)
{
	pthread_t Create_play_warning_tone_thread_looper;
	pthread_t play_warning_tone_status_thread_looper;
	if (pthread_create(&Create_play_warning_tone_thread_looper, NULL, play_warning_tone_thread, NULL) != 0)
	{
		printf("[%s]: play_warning_tone_thread create error\n",__FUNCTION__);
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&play_warning_tone_status_thread_looper, NULL, get_warning_tone_status_thread, NULL) != 0)
	{
		printf("[%s]: get_warning_tone_status_thread create error\n",__FUNCTION__);
		exit(EXIT_FAILURE);
	}
}
#endif
static int websetting_wifi_start(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	open_start_wifi();
	strncpy(CGIsetting_priv.result_str,"OK",strlen("OK"));
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_wifi_mode(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	sprintf(CGIsetting_priv.result_str,"%d",ezcast_sysinfo_get_wifi_mode());
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_last_ui(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int ret=Get_last_UI();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}
#if MODULE_CONFIG_LAN_ONLY
static int websetting_get_lan_ip(char *cmd,char *val)
{
	char * Lan_ip=NULL;
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_lan_ip")));
	else
		return -1;
	
	sscanf(setcmd_tmp,"%d",&ret);
	Lan_ip=getLanIp();
	if(Lan_ip!=NULL)
	{
		strcat(CGIsetting_priv.result_str,Lan_ip);
	}
	else 
		return -1;
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;

}
#endif
static int websetting_get_mac_ip(char *cmd,char *val)
{
	char * mac_addr=NULL;
	char * wlan1_ip_addr=NULL;
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_mac_ip")));
	else
		return -1;
	
	sscanf(setcmd_tmp,"%d",&ret);


	mac_addr=ezcast_wifi_get_connect_mac_address(ret);
	wlan1_ip_addr=ezcast_sysinfo_get_ip(1);


	
	if(mac_addr!=NULL)
	{
		strncpy(CGIsetting_priv.result_str,mac_addr,strlen(mac_addr));
		if(wlan1_ip_addr!=NULL)
		{
			strcat(CGIsetting_priv.result_str,"\n");
			strcat(CGIsetting_priv.result_str,wlan1_ip_addr);
			strcat(CGIsetting_priv.result_str,"\n");
		}		
	}
	else 
		return -1;
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

static int websetting_get_connected_ssid(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	static int get_connected_ssid_tick=0;
	//int wifi_mode_get_from_case=ezcast_sysinfo_get_wifi_mode();
	char * connected_ssid=ezcast_wifi_get_connected_ssid();
	int Lan_Con_Sta=0;
#if EZWILAN_ENABLE
	Lan_Con_Sta=get_netlink_status();
#endif
	if(connected_ssid!=NULL)
	{
		if(strlen(connected_ssid) != 0)//9:WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK   14:WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR
		{
			get_connected_ssid_tick=0;
			strncpy(CGIsetting_priv.result_str,connected_ssid,strlen(connected_ssid));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
		}		
		else
		{	
			if(!get_connected_ssid_tick)
			{
				get_connected_ssid_tick=1;
				if(Lan_Con_Sta==1)
					strncpy(CGIsetting_priv.result_str,"to_wifi_list&lan",strlen("to_wifi_list&lan"));
				else
					strncpy(CGIsetting_priv.result_str,"to_wifi_list",strlen("to_wifi_list"));
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;			
			}
			else
				CGIsetting_priv.sendline=NULL;
		}			
	}
	return 1;
}
static int websetting_get_OTA_version(char *cmd,char *val)
{
	char *VAL=val;
	printf("\n\n-----------------websetting_get_OTA_version-----------------\n\n");
	int ret=Get_OTA_version();
	return ret;
}
static int websetting_get_OTA_conf(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	#if EZMUSIC_ENABLE
	char *ota_url="http://www.iezvu.com/upgrade/ezcast_music/ezcast_music_test.conf";
	#else
	char *ota_url="http://www.iezvu.com/upgrade/ezcast/ezcast.conf";
	#endif
	ezcast_sysinfo_ota_upgrade(4,ota_url);
	return 1;
}
static int websetting_get_main_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_main_text();
	return ret;
}
static int websetting_get_menu_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_menu_text();
	return ret;
}
static int websetting_get_airview_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_airview_text();
	return ret;
}
static int websetting_get_ezwire_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_ezwire_text();
	return ret;
}

static int websetting_get_paswod_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_paswod_text();
	return ret;
}

static int websetting_get_conference_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_conference_text();
	return ret;
}

static int websetting_get_fileupload_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_fileupload_text();
	return ret;
}

static int websetting_get_authority_host_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_authority_host_text();
	return ret;
}

static int websetting_get_control_access_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_control_access_text();
	return ret;
}

static int websetting_get_serverota_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_serverota_text();
	return ret;
}

static int websetting_get_rebot_rest_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_rebot_rest_text();
	return ret;
}

static int websetting_get_lan_multstring(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_lan_text();
	return ret;
}
//for 8268
#if defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE > 29 && MODULE_CONFIG_CHIP_TYPE < 40)
static int websetting_get_webmultilanguage_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_webmultilanguage_text();
	return ret;
}
#endif
static int websetting_get_index_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_index_text();
	return ret;
}
static int websetting_get_wifi_list_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_wifi_list_text();	
	return ret;
}
static int websetting_get_default_modetext(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_Default_mode_text();
	return ret;
}	
static int websetting_get_lan_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=set_lan_text();
	return ret;
}


static int websetting_get_route_warn_string(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_router_warn_text();
	return ret;
}

static int websetting_get_wifi_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	#if EZMUSIC_ENABLE
	check_multilanguage_for_warning_tone(index);
	#endif
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Set_wifi_text();
	return ret;
}	

static int websetting_get_wifilist_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret = Get_wifilist_text();
	return ret;
}	
static int websetting_get_addnet_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Add_network_text();
	return ret;
}	
static int websetting_get_connectfail_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Wifi_connect_fail_text();
	return ret;
}	
static int websetting_get_OTA_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_OTA_text();
	return ret;
}	
static int Get_password_text(void)
{
	int string_length=0;
	char *string_temp=NULL;
	if(!access(APPINFO_FILE,F_OK))
	{
		memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
		string_temp=ezcast_get_string("IDS_PASSWORD_WIFI"); //IDS_STTING_EZMUSIC_PSW_LEN
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_STTING_EZMUSIC_PSW_ALLOW");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
		string_temp=ezcast_get_string("IDS_WIFI_LIST_WARN2");
		if(string_temp&&(string_length+1<1000))
			strcat(CGIsetting_priv.language_string,string_temp);
		else
		{
			printf("not enought buffer for string !%d\n",__LINE__);
			return -1;			
		}
		strcat(CGIsetting_priv.language_string,"\n");
        string_temp=ezcast_get_string("IDS_STTING_NETWORK_SSID_PSK_ALLOW");
        if(string_temp&&(string_length+1<1000))
            strcat(CGIsetting_priv.language_string,string_temp);
        else
        {
            printf("not enought buffer for string !%d\n",__LINE__);
            return -1;          
        }
        strcat(CGIsetting_priv.language_string,"\n");
        string_temp=ezcast_get_string("IDS_STTING_NETWORK_SSID_PSK_LEN_ALLOW");
        if(string_temp&&(string_length+1<1000))
            strcat(CGIsetting_priv.language_string,string_temp);
        else
        {
            printf("not enought buffer for string !%d\n",__LINE__);
            return -1;          
        }
        strcat(CGIsetting_priv.language_string,"\n");
		CGIsetting_priv.sendline=CGIsetting_priv.language_string;
	}
	else 
	{
		CGIsetting_priv.sendline=NULL;
		return -1;
	}
	return 1;
}
static int websetting_get_password_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_password_text();
	return ret;
}	
static int websetting_get_OTA_warn1_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	Get_OTA_warn1_text();
	return 1;
}	
static int websetting_get_OTA_warn2_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_OTA_warn2_text();
	return ret;
}	
static int websetting_get_OTA_faild_connect(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=Get_OTA_faild_connect_text();
	return ret;
}	
static int websetting_get_res_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=resolution_text();
	return ret;
}	

static int websetting_get_eq_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=eq_text();
	return ret;
}	
static int websetting_get_new_version_text(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int index=ezcast_sysinfo_get_local_language();
	ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
	int ret=new_version_text();
	return ret;
}	
static int websetting_get_down_bin(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int ret=Download_OTA_binfile();
	if(1==ret)
	{
		strncpy(CGIsetting_priv.result_str,"OK",strlen("OK"));
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		printf("CGIsetting_priv.sendline================%s\n",CGIsetting_priv.sendline);
	}
	return ret;
}	
static int websetting_get_connections_num(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int connections_num=ezcast_wifi_count_accessDevices();
	//if(connections_num>0)
	{
		sprintf(CGIsetting_priv.result_str,"%d",connections_num);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	}
	return 1;
}	
static int websetting_get_ssid_signal(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	CGIsetting_priv.sendline=SysCGI_priv.ssid_signal_string;

	return 1;
}	
static int websetting_get_ap_ssid(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	char * ap_ssid=ezcast_wifi_get_softap_ssid();
	if(ap_ssid!=NULL)
	{
		strcpy(CGIsetting_priv.result_str,ap_ssid);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		free(ap_ssid);
		ap_ssid = NULL;
		return 1;		
	}
	else
		return -1;
}	
static int websetting_check_wifi(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	check_connect_status();
	return 1;
}	
static int websetting_check_auto_connect(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int ret=check_auto_connect();
	if(ret==1)//connect success
	{	
		CGIsetting_priv.sendline=SysCGI_priv.connect_status;	
	}
	else if(ret==2)//connect error
	{
		strncpy(CGIsetting_priv.result_str,"ER",strlen("ER"));
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	}
	else if(ret==3)//connect success can not get ip
	{
		strcat(SysCGI_priv.connect_status,"\n");
		CGIsetting_priv.sendline=SysCGI_priv.connect_status;	
	}
	return 1;
}
static int ezcast_read_outputformat()
{
	int format=-1;
	int outputmode=-1,outputformat=-1;
	#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)
	probox_get_outputmode(&outputmode,&outputformat);
	printf("outputmode[%d][%d]",outputmode,outputformat);
	if(outputmode<0){
		outputmode=ezcast_sysinfo_read_edid_bin(0);
		outputformat=ezcast_sysinfo_read_edid_bin(1);
		printf("outputmode from edid[%d][%d]",outputmode,outputformat);
		if(outputmode==1) outputmode=1;
		else if(outputmode==2) outputmode==8;
		else outputmode=2;
	}

	#endif
	printf("outputmode format[%d][%d]",outputmode,outputformat);
	if(outputmode==1)	format=outputformat;
	else if(outputmode==8) format=20+format;
	else if(outputmode==2) format=30+format;
	else format=-1;

	return format;
}

static int websetting_get_resolution(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;	
	int res_tmp=0;
#if(MODULE_CONFIG_EZCASTPRO_MODE==8075 ||MODULE_CONFIG_EZWIRE_ENABLE==1)

	if(ezcast_sysinfo_read_edid_bin(0)==8){//vga only  mode add 200 for vga flag	
		res_tmp=ezcast_sysinfo_read_edid_bin(1);
		printf("websetting read edid res_tmp=%d\n",res_tmp);
		if(res_tmp==0)   //VGA_1280x1024  VGA_1024x768  VGA only
			CGIsetting_priv.last_resolution=200+ezcast_sysinfo_read_edid_bin(1);
		else if(res_tmp==9)   CGIsetting_priv.last_resolution=2;    //turn to hdmi value  1920*1080 30P
		else if(res_tmp==6)   CGIsetting_priv.last_resolution=17;
		else if(res_tmp==5)   CGIsetting_priv.last_resolution=16; 
		else if(res_tmp==8)   CGIsetting_priv.last_resolution=5;
		else if(res_tmp==2)   CGIsetting_priv.last_resolution=15; 
		else CGIsetting_priv.last_resolution=5; 
	}
	else
		CGIsetting_priv.last_resolution=ezcast_sysinfo_read_edid_bin(1);
#else
	CGIsetting_priv.last_resolution=ezcast_sysinfo_read_edid_bin(1);

	#endif
	sprintf(CGIsetting_priv.result_str,"%d",CGIsetting_priv.last_resolution);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	printf("[%s][%d] CGIsetting_priv.last_resolution=%d\n",__func__,__LINE__,CGIsetting_priv.last_resolution);
	return 1;
}

static int websetting_get_ap_name(char *cmd,char *val)
{
	char *VAL=val;
	get_ap_name(cmd);
	return 1;
}
static int websetting_get_scan_result(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;	
	send_scanresults();
	return 1;
}
/*
static int websetting_get_lan_index(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int i=0;
	int lan_index=0;
	if(!access(APPINFO_FILE,F_OK))
	{
		const char * lan_string=ezcast_sysinfo_langautoget();
		if((lan_string!=NULL)&&(!ezcast_sysinfo_get_disauto_status()))
		{
			for(i=0;i<sizeof(lan_value_item)/sizeof(lan_value_item[0]);i++)
			{
				if(strcmp(ezcast_sysinfo_langautoget(),(char *)lan_value_item[i])==0)
				{	
					//ezcast_set_language((char *)lan_value_item[i]);	
					ezcast_set_language_swf_name((char *)lan_value_item[i],"ezcast");
					sprintf(CGIsetting_priv.result_str,"%d",i);
					strcat(CGIsetting_priv.result_str,"auto");
					CGIsetting_priv.sendline=CGIsetting_priv.result_str;
					#if (EZMUSIC_ENABLE)
					SysCGI_priv.language_index=i;
					check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
					#endif
					return 1;
				}
			}
		}
		else
		{
			lan_index=ezcast_sysinfo_get_local_language();
			sprintf(CGIsetting_priv.result_str,"%d",lan_index);
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
			#if (EZMUSIC_ENABLE)
			SysCGI_priv.language_index=lan_index;
			check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
			#endif
			return 1;
		}		
	}	
	else
	{
			if(!ezcast_sysinfo_get_disauto_status()){
				strcat(CGIsetting_priv.result_str,"auto");
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
				return 1;
				}
			else{
				lan_index=ezcast_sysinfo_get_local_language();
				sprintf(CGIsetting_priv.result_str,"%d",lan_index);
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
				#if (EZMUSIC_ENABLE)
				SysCGI_priv.language_index=lan_index;
				check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
				#endif
				return 1;
			}
		}
	return -1;
}
*/

static int websetting_get_lan_index(char *cmd,char *val)
{	char *VAL=val;
	char *CMD=cmd;
	int lan_index=0;
//read currentLanguageId:	
	lan_index=ezcast_sysinfo_get_local_language();
	SysCGI_priv.language_index=lan_index;
	#if (EZMUSIC_ENABLE)
	check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
	#endif
//read disauto status to html display	
	if(!ezcast_sysinfo_get_disauto_status()){
		strcat(CGIsetting_priv.result_str,"auto");
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
		}
	else{
		sprintf(CGIsetting_priv.result_str,"%d",lan_index);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
		}
return -1;
}

static int websetting_get_ota_check_status(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int ret=ezcast_sysinfo_get_ota_check_status();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;	
}
static int websetting_Check_new_version_connect_wifi_first(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	static int check_ota_connect_wifi_first_flag=0;
	int ret=ezcast_sysinfo_get_ota_check_status();
	if(2==ret)
		check_ota_connect_wifi_first_flag++;
	if(check_ota_connect_wifi_first_flag>1)
		ret=3;
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;	
}
static int websetting_get_cpu_frequency(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	int cpu_frequency=ezcast_sysinfo_get_cpu_frequency();
	if(cpu_frequency>=552)
		sprintf(CGIsetting_priv.result_str,"%d",1);
	else
		sprintf(CGIsetting_priv.result_str,"%d",0);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;	
}
static int websetting_set_music_output(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	FILE *fp=NULL;
	int ret;
	char callbuf[100]={0};
	int music_output_mode_index;
	char *music_output_mode_index_temp=NULL;
	strncpy(CGIsetting_priv.result_str,cmd,(strlen(cmd)-strlen("set_music_output")));
	sscanf(CGIsetting_priv.result_str, "%d", &music_output_mode_index);
	if(access("/mnt/vram/EZmusic/music_output.conf",F_OK)==-1)
	{
		if(access("/mnt/vram/EZmusic",F_OK)==-1)
		{
			sprintf(callbuf,"mkdir /mnt/vram/EZmusic/");
			system(callbuf);				
		}	
		memset(callbuf,0,sizeof(callbuf));
		sprintf(callbuf,"touch /mnt/vram/EZmusic/music_output.conf");
		system(callbuf);
	}	
	fp=fopen("/mnt/vram/EZmusic/music_output.conf","rw+");
	if(fp==NULL)
	{
		perror("/mnt/vram/EZmusic/music_output.conf not exit !\n");
		return -1;
	}
	memset(callbuf,0,sizeof(callbuf));
	ret = fread(callbuf, 1, sizeof(callbuf), fp);
	if(ret>0)
	{
		music_output_mode_index_temp=strstr(callbuf,"music_output_mode_index\n");
		if(music_output_mode_index_temp)
		{
			memset(callbuf,0,sizeof(callbuf));
			strncpy(callbuf,"music_output_mode_index\n",strlen("music_output_mode_index\n"));
			sprintf(CGIsetting_priv.result_str,"%d",music_output_mode_index);
			strcat(callbuf,CGIsetting_priv.result_str);
			strcat(callbuf,"\n");
			fclose(fp);
			fp=fopen("/mnt/vram/EZmusic/music_output.conf","w+");
			if(fp==NULL)
			{
				perror("/mnt/vram/EZmusic/music_output.conf not exit !\n");
				ret=-1;
			}
			fwrite(callbuf,1,strlen(callbuf),fp);
			fflush(fp);
			int fd_write = fileno(fp);
			fsync(fd_write);
			fclose(fp);
			ret=1;
		}
		else
		{
			printf("music_output.conf format error !\n");
			fclose(fp);
			ret=-1;
		}
	}
	else
	{
		memset(callbuf,0,sizeof(callbuf));
		strncpy(callbuf,"music_output_mode_index\n",strlen("music_output_mode_index\n"));
		sprintf(CGIsetting_priv.result_str,"%d",music_output_mode_index);
		strcat(callbuf,CGIsetting_priv.result_str);
		strcat(callbuf,"\n");
		fseek(fp,0,SEEK_SET);
		fwrite(callbuf,1,strlen(callbuf),fp);
		fflush(fp);
		int fd_write = fileno(fp);
		fsync(fd_write);
		fclose(fp);	
		ret=1;
	}
	#if EZMUSIC_ENABLE
	switch(music_output_mode_index)
	{
		/*case 0:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,0);
			#endif
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
			#endif
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
			#endif
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,1);
			#endif		
			break;*/
		case 0:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,1);
			#endif
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,0);
			#endif
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
			#endif
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,1);
			#endif
			break;
		case 1:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,1);
			#endif
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
			#endif
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,0);
			#endif
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,1);
			#endif
			break;
		case 2:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,1);
			#endif
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
			#endif
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
			#endif
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,0);
			#endif
			break;
	}
	#endif
	return ret;
}

static int websetting_get_music_output(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	FILE *fp=NULL;
	int ret;
	char callbuf[100]={0};
	char *music_output_mode_index_temp=NULL;
	if(access("/mnt/vram/EZmusic/music_output.conf",F_OK)==-1)//not exit
	{
		strncpy(CGIsetting_priv.result_str,"default",strlen("default"));
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
	}
	else
	{
		fp=fopen("/mnt/vram/EZmusic/music_output.conf","r");
		if(fp==NULL)
		{
			perror("/mnt/vram/EZmusic/music_output.conf not exit !\n");
			strncpy(CGIsetting_priv.result_str,"default",strlen("default"));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}		
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		if(ret > 0)
		{
			music_output_mode_index_temp=strtok(callbuf,"\n");
			if(!music_output_mode_index_temp)
			{
				printf("music_output.conf format error !\n");
				strncpy(CGIsetting_priv.result_str,"default",strlen("default"));
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
			}
			music_output_mode_index_temp=strtok(NULL,"\n");
			if(!music_output_mode_index_temp)
			{
				printf("music_output.conf format error !\n");
				strncpy(CGIsetting_priv.result_str,"default",strlen("default"));
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
			}
			strncpy(CGIsetting_priv.result_str,music_output_mode_index_temp,strlen(music_output_mode_index_temp));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}
	}
	return 1;
}
static int websetting_get_EQ_default(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	FILE *fp=NULL;
	int ret;
	char callbuf[100]={0};
	char *EQ_default_mode_index_temp=NULL;
	if(access("/mnt/vram/EZmusic/EQ_mode.conf",F_OK)==-1)//not exit
	{
		
		strncpy(CGIsetting_priv.result_str,"0",strlen("0"));
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
	}
	else
	{
		fp=fopen("/mnt/vram/EZmusic/EQ_mode.conf","r");
		if(fp==NULL)
		{
			perror("/mnt/vram/EZmusic/EQ_mode.conf not exit !\n");
			strncpy(CGIsetting_priv.result_str,"0",strlen("0"));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}		
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		if(ret > 0)
		{
			EQ_default_mode_index_temp=strtok(callbuf,"\n");
			if(!EQ_default_mode_index_temp)
			{
				printf("EQ_mode.conf format error !\n");
				strncpy(CGIsetting_priv.result_str,"0",strlen("0"));
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
			}
			EQ_default_mode_index_temp=strtok(NULL,"\n");
			if(!EQ_default_mode_index_temp)
			{
				printf("EQ_mode.conf format error !\n");
				strncpy(CGIsetting_priv.result_str,"0",strlen("0"));
				CGIsetting_priv.sendline=CGIsetting_priv.result_str;
			}
			strncpy(CGIsetting_priv.result_str,EQ_default_mode_index_temp,strlen(EQ_default_mode_index_temp));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}
	}
	return 1;
}
static int websetting_set_EQ_default(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	FILE *fp=NULL;
	int ret;
	char callbuf[100]={0};
	int EQ_default_mode_index;
	char *EQ_default_mode_index_temp=NULL;
	strncpy(CGIsetting_priv.result_str,cmd,(strlen(cmd)-strlen("set_EQ_default")));
	sscanf(CGIsetting_priv.result_str, "%d", &EQ_default_mode_index);
	if(access("/mnt/vram/EZmusic/EQ_mode.conf",F_OK)==-1)
	{
		if(access("/mnt/vram/EZmusic",F_OK)==-1)
		{
			sprintf(callbuf,"mkdir /mnt/vram/EZmusic/");
			system(callbuf);				
		}
		memset(callbuf,0,sizeof(callbuf));
		sprintf(callbuf,"touch /mnt/vram/EZmusic/EQ_mode.conf");
		system(callbuf);
	}	
	fp=fopen("/mnt/vram/EZmusic/EQ_mode.conf","rw+");
	if(fp==NULL)
	{
		perror("/mnt/vram/EZmusic/EQ_mode.conf not exit !\n");
		return -1;
	}
	memset(callbuf,0,sizeof(callbuf));
	ret = fread(callbuf, 1, sizeof(callbuf), fp);
	if(ret>0)
	{
		EQ_default_mode_index_temp=strstr(callbuf,"EQ_default_mode_index\n");
		if(EQ_default_mode_index_temp)
		{
			memset(callbuf,0,sizeof(callbuf));
			strncpy(callbuf,"EQ_default_mode_index\n",strlen("EQ_default_mode_index\n"));
			sprintf(CGIsetting_priv.result_str,"%d",EQ_default_mode_index);
			strcat(callbuf,CGIsetting_priv.result_str);
			strcat(callbuf,"\n");
			fclose(fp);
			fp=fopen("/mnt/vram/EZmusic/EQ_mode.conf","w+");
			if(fp==NULL)
			{
				perror("/mnt/vram/EZmusic/EQ_mode.conf not exit !\n");
				return -1;
			}			
			fwrite(callbuf,1,strlen(callbuf),fp);
			ret=fflush(fp);
			int fd_write = fileno(fp);
			ret=fsync(fd_write);
			ret=fclose(fp);		
			return 1;				
		}
		else
		{
			printf("EQ_mode.conf format error !\n");
			fclose(fp);	
			return -1;
		}
	}
	else
	{
		memset(callbuf,0,sizeof(callbuf));
		strncpy(callbuf,"EQ_default_mode_index\n",strlen("EQ_default_mode_index\n"));
		sprintf(CGIsetting_priv.result_str,"%d",EQ_default_mode_index);
		strcat(callbuf,CGIsetting_priv.result_str);
		strcat(callbuf,"\n");
		fseek(fp,0,SEEK_SET);
		fwrite(callbuf,1,strlen(callbuf),fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);		
		return 1;				
	}
}
static int websetting_set_custom_value(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	FILE *fp=NULL;
	int ret,i;
	char *p2=NULL;
	char callbuf[150]={0};
	char temp_string[2]={0};
	char *custom_value_index_temp=NULL;
	char *custom_value_temp=NULL;
	int custom_value_index,custom_value;
	strncpy(CGIsetting_priv.result_str,cmd,(strlen(cmd)-strlen("set_EQ_Custom")));
	custom_value_temp=strtok(CGIsetting_priv.result_str,"\n");
	sscanf(custom_value_temp, "%d", &custom_value);
	custom_value_index_temp=strtok(NULL,"\n");
	sscanf(custom_value_index_temp, "%d", &custom_value_index);
	if(access("/mnt/vram/EZmusic/custom_value.conf",F_OK)==-1)
	{
		if(access("/mnt/vram/EZmusic",F_OK)==-1)
		{
			sprintf(callbuf,"mkdir /mnt/vram/EZmusic/");
			system(callbuf);				
		}
		memset(callbuf,0,sizeof(callbuf));
		sprintf(callbuf,"touch /mnt/vram/EZmusic/custom_value.conf");
		system(callbuf);
	}	
	fp=fopen("/mnt/vram/EZmusic/custom_value.conf","rw+");
	if(fp==NULL)
	{
		perror("/mnt/vram/EZmusic/custom_value.conf not exit !\n");
		return -1;
	}
	memset(callbuf,0,sizeof(callbuf));
	ret = fread(callbuf, 1, sizeof(callbuf), fp);
	if(ret>0)
	{
		if(-1==custom_value)//set  custom value to selection default
		{
			memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
			for(i=0;i<10;i++)
			{				
				sprintf(temp_string,"%d",i);
				strcat(CGIsetting_priv.result_str,temp_string);
				strcat(CGIsetting_priv.result_str,":");
				sprintf(temp_string,"%d",Selection_EQ_Value[custom_value_index][i]);
				strcat(CGIsetting_priv.result_str,temp_string);
				strcat(CGIsetting_priv.result_str,"\n");					
			}
		}
		else
		{
			memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
			sprintf(temp_string,"%d",custom_value_index);
			strcat(CGIsetting_priv.result_str,temp_string);
			strcat(CGIsetting_priv.result_str,":");
			char *p1=strstr(callbuf,CGIsetting_priv.result_str);
			if(custom_value_index<9)
			{
				memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
				sprintf(CGIsetting_priv.result_str,"%d",custom_value_index+1);
				strcat(CGIsetting_priv.result_str,":");
				p2=strstr(callbuf,CGIsetting_priv.result_str);
			}
			memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
			sprintf(temp_string,"%d",custom_value_index);
			strncpy(CGIsetting_priv.result_str,callbuf,strlen(callbuf)-strlen(p1)+strlen(temp_string)+strlen(":"));
			sprintf(temp_string,"%d",custom_value);
			strcat(CGIsetting_priv.result_str,temp_string);
			strcat(CGIsetting_priv.result_str,"\n");
			if(custom_value_index<9)
				strcat(CGIsetting_priv.result_str,p2);
		}
		memset(callbuf,0,sizeof(callbuf));
		strncpy(callbuf,CGIsetting_priv.result_str,strlen(CGIsetting_priv.result_str));
		fclose(fp);
		fp=fopen("/mnt/vram/EZmusic/custom_value.conf","w+");
		if(fp==NULL)
		{
			perror("/mnt/vram/EZmusic/custom_value.conf not exit !\n");
			return -1;
		}		
		fwrite(callbuf,1,strlen(callbuf),fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);		
		return 1;					
	}
	else
	{
		if(-1==custom_value)//set  custom value to selection default
		{
			memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
			for(i=0;i<10;i++)
			{				
				sprintf(temp_string,"%d",i);
				strcat(CGIsetting_priv.result_str,temp_string);
				strcat(CGIsetting_priv.result_str,":");
				sprintf(temp_string,"%d",Selection_EQ_Value[custom_value_index][i]);
				strcat(CGIsetting_priv.result_str,temp_string);
				strcat(CGIsetting_priv.result_str,"\n");					
			}
		}
		else
		{
			memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
			for(i=0;i<10;i++)
			{
				if(i==custom_value_index)
				{
					sprintf(temp_string,"%d",custom_value_index);
					strcat(CGIsetting_priv.result_str,temp_string);
					strcat(CGIsetting_priv.result_str,":");
					int ret_1=sprintf(temp_string, "%d",custom_value);
					strcat(CGIsetting_priv.result_str,temp_string);
					strcat(CGIsetting_priv.result_str,"\n");			
				}
				else
				{	
					sprintf(temp_string,"%d",i);
					strcat(CGIsetting_priv.result_str,temp_string);
					strcat(CGIsetting_priv.result_str,":");
					sprintf(temp_string,"%d",Selection_EQ_Value[0][i]);
					strcat(CGIsetting_priv.result_str,temp_string);
					strcat(CGIsetting_priv.result_str,"\n");						
				}
			}			
		}
		memset(callbuf,0,sizeof(callbuf));
		strncpy(callbuf,CGIsetting_priv.result_str,strlen(CGIsetting_priv.result_str));
		fseek(fp,0,SEEK_SET);
		fwrite(callbuf,1,strlen(callbuf),fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);		
		return 1;				
	}
}
static int websetting_get_custom_value(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	FILE *fp=NULL;
	int ret,i;
	char callbuf[100]={0};
	char temp_string[2]={0};
	char *EQ_default_mode_index_temp=NULL;
	if(access("/mnt/vram/EZmusic/custom_value.conf",F_OK)==-1)//not exit
	{
		if(access("/mnt/vram/EZmusic",F_OK)==-1)
		{
			sprintf(callbuf,"mkdir /mnt/vram/EZmusic/");
			system(callbuf);				
		}
		memset(callbuf,0,sizeof(callbuf));
		sprintf(callbuf,"touch /mnt/vram/EZmusic/custom_value.conf");
		system(callbuf);	
		fp=fopen("/mnt/vram/EZmusic/custom_value.conf","rw+");
		if(fp==NULL)
		{
			perror("/mnt/vram/EZmusic/custom_value.conf not exit !\n");
			return -1;
		}
		for(i=0;i<10;i++)
		{				
			sprintf(temp_string,"%d",i);
			strcat(CGIsetting_priv.result_str,temp_string);
			strcat(CGIsetting_priv.result_str,":");
			sprintf(temp_string,"%d",Selection_EQ_Value[0][i]);
			strcat(CGIsetting_priv.result_str,temp_string);
			strcat(CGIsetting_priv.result_str,"\n");					
		}	
		memset(callbuf,0,sizeof(callbuf));
		strncpy(callbuf,CGIsetting_priv.result_str,strlen(CGIsetting_priv.result_str));
		fseek(fp,0,SEEK_SET);
		fwrite(callbuf,1,strlen(callbuf),fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);
		return 1;
	}
	else
	{
		fp=fopen("/mnt/vram/EZmusic/custom_value.conf","r");
		if(fp==NULL)
		{
			perror("/mnt/vram/EZmusic/custom_value.conf not exit !\n");
			strncpy(CGIsetting_priv.result_str,"default",strlen("default"));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}		
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		if(ret > 0)
		{
			strncpy(CGIsetting_priv.result_str,callbuf,strlen(callbuf));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}
	}
	return 1;
}

static int websetting_get_music_output_en(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;	
	char temp_string[1]={0};
	//sprintf(CGIsetting_priv.result_str,"%d",CGIsetting_priv.AUDIO_EN);
	//strcat(CGIsetting_priv.result_str,"\n");
	sprintf(temp_string,"%d",CGIsetting_priv.SPDIF_EN);
	strcat(CGIsetting_priv.result_str,temp_string);
	strcat(CGIsetting_priv.result_str,"\n");
	sprintf(temp_string,"%d",CGIsetting_priv.I2S_EN);
	strcat(CGIsetting_priv.result_str,temp_string);
	strcat(CGIsetting_priv.result_str,"\n");
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_router_ctl(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;	
	sprintf(CGIsetting_priv.result_str,"%d",ezcast_sysinfo_get_router_ctl());
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static websetting_set_router_ctl (char *cmd,char *val)
{
	char setcmd_tmp[30]={0};
	char *VAL=val;
	int connection_index;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_router_ctl")));
	else
		return -1;
	sscanf(setcmd_tmp, "%d", &connection_index);
	ezcast_sysinfo_set_router_ctl(connection_index,1);
	//Send_FLA_Key(SWF_MSG_KEY_SETPSK);
	return 1;
}

static int websetting_get_ipaddress(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	char setcmd_tmp[30]={0};
	int port=0;
	if(cmd!=NULL)	
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_wlan_ipaddress")));
	else
		return -1;
	sscanf(setcmd_tmp, "%d", &port);
	char * wlan0_ip_addr=ezcast_sysinfo_get_ipaddress(port);
	if(wlan0_ip_addr!=NULL)
		strncpy(CGIsetting_priv.result_str,wlan0_ip_addr,strlen(wlan0_ip_addr));
	else 
		return -1;
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_wlan0_ip(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	char * wlan0_ip_addr=ezcast_sysinfo_get_ip(0);
	if(wlan0_ip_addr!=NULL)
		strncpy(CGIsetting_priv.result_str,wlan0_ip_addr,strlen(wlan0_ip_addr));
	else 
		return -1;
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_ota_status(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	static int get_download_FINISHED_state=0;
	int ret=ezcast_sysinfo_get_ota_status();
	int ota_prg=(ret)%256;
	printf("ota_prg============%d\n",ota_prg);
	int ota_state=(ret)>>8;
	//printf("ota_state============%d\n",ota_state);
	if(CARD_STATE_RUNUING==ota_state)
	{
		sprintf(CGIsetting_priv.result_str,"%d",ota_prg);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	}
	else if(CARD_STATE_FINISHED==ota_state)
	{
		sprintf(CGIsetting_priv.result_str,"%d",100);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
		if(0==get_download_FINISHED_state)
		{
			get_download_FINISHED_state=1;
			return 1;
		}
		#if EZMUSIC_ENABLE
		if(1==get_download_FINISHED_state)
		{
			if(0==SysCGI_priv.start_ota_upgrade_flag)
				SysCGI_priv.start_ota_upgrade_flag=1;		
		}
		#endif
	}
	return 1;
}
static int websetting_enter_check_ota_boot(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	static int enter_check_ota_boot_flag=0;
	sprintf(CGIsetting_priv.result_str,"%d",enter_check_ota_boot_flag);
	if(0==enter_check_ota_boot_flag)
		enter_check_ota_boot_flag=1;	
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}	
static int websetting_set_devicename_reboot(char *cmd,char *val)
{   
      char name[40]={0};
	char *VAL=val;
	if(cmd!=NULL)
		strncpy(name,cmd,(strlen(cmd)-strlen("set_devicename_reboot")));
	else
		return -1;
	if(strstr(name,":")==NULL)//The device name can't contain ":"
	{
     	printf("websetting_set_devicename_reboot  %s  \n",name);
	 	ezcast_set_device_name(name);
		system("reboot");
	}
	else 
		return -1;
	return 1;
		
}
static int websetting_set_device_name(char *cmd,char *val)
{   
	char name[40]={0};
	char *VAL=val;
	if(cmd!=NULL)
		strncpy(name,cmd,(strlen(cmd)-strlen("set_device_name")));
	else
		return -1;
	if(strstr(name,":")==NULL)//The device name can't contain ":"
	{
     	printf("websetting_set_device_name  %s  \n",name);
	 	ezcast_set_device_name(name);
	}
	else 
		return -1;	
	return 1;
		
}
static int websetting_get_device_name(char *cmd,char *val)
{
	char *CMD=cmd;
	int ret = ezcast_get_device_name(CGIsetting_priv.result_str, sizeof(CGIsetting_priv.result_str));
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_setAutoplayEnable(char *cmd,char *val)
{   
	char setcmd_tmp[40]={0};
	char *VAL=val;
       char  *set_cmd;
	int opt=0;
	int data;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("setAutoplayEnable")));
	else
		return -1;
	set_cmd=strtok(setcmd_tmp,"\n");
	VAL=strtok(NULL,"\n");
	sscanf(VAL, "%d", &data);
     //  printf("websetting_setAutoplayEnable  %s %s %d  \n",set_cmd,VAL,data);
	if(strstr(set_cmd,"SET_AUTOPLAY_ENABLE")!=NULL)
		opt=SET_AUTOPLAY_ENABLE;
	else if(strstr(set_cmd,"SET_AUTOPLAY_VOLUME")!=NULL)
		opt=SET_AUTOPLAY_VOLUME;
	else if(strstr(set_cmd,"SET_AUTOPLAY_WAITIME")!=NULL)
		opt=SET_AUTOPLAY_WAITIME;
	else if(strstr(set_cmd,"SET_AUTOPLAY_LIST")!=NULL)
		opt=SET_AUTOPLAY_PLAYLIST;
	else if(strstr(set_cmd,"SET_AUTOPLAY_HOST_AP")!=NULL)
		opt=SET_AUTOPLAY_HOST_AP;
	else if(strstr(set_cmd,"SET_AUTOPLAY_PROGRESSIVE")!=NULL)
		opt=SET_AUTOPLAY_PROGRESSIVE;
	 ezCastSetAutoplayVal(opt, data);
	return 1;
		
}
static int websetting_getAutoplayEnable(char *cmd,char *val)
{
	
	char *CMD=cmd;
	char ret=0;
	
	ret=ezCastGetAutoplayVal(SET_AUTOPLAY_ENABLE);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	// printf("websetting_getAutoplayEnable  %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_getAutoplayVolume(char *cmd,char *val)
{
	
	char *CMD=cmd;
	char ret=0;
	
	ret=ezCastGetAutoplayVal(SET_AUTOPLAY_VOLUME);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	// printf("websetting_getAutoplayVolume  %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_getAutoplayWaitime(char *cmd,char *val)
{
	
	char *CMD=cmd;
	int ret=0;
	ret=ezCastGetAutoplayVal(SET_AUTOPLAY_WAITIME);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	// printf("websetting_getAutoplayWaitime  %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_getAutoplaylist(char *cmd,char *val)
{
	
	char *CMD=cmd;
	int ret=0;
	ret=ezCastGetAutoplayVal(SET_AUTOPLAY_PLAYLIST);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	// printf("websetting_getAutoplayWaitime  %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_getPROGRESSIVE(char *cmd,char *val)
{
	
	char *CMD=cmd;
	int ret=0;
	ret=ezCastGetAutoplayVal(SET_AUTOPLAY_PROGRESSIVE);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	// printf("websetting_getAutoplayWaitime  %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_otaFrom(char *cmd,char *val)
{
	char *CMD=cmd;
	char *VAL=val;
	char url[512] = {0};
	int ret=0;
	if(cmd==NULL){
		EZCASTWARN("cmd is NULL!!!\n");
		return -1;
	}
	strncpy(url,cmd,(strlen(cmd)-strlen("ota_from")));
	if(strlen(url) <= 0){
		EZCASTWARN("url error!!!\n");
		goto __END__;
	}
	EZCASTLOG("manual ota url: %s\n", url);
	otaFromUrl(url);

__END__:
	return 1;
}

static int websetting_do_cmd(char *cmd,char *val)
{
	char *CMD=cmd;
	char *VAL=val;
	char opt_cmd[512] = {0};
	int ret=0;
	static int adbDisplayStarted = 0;
	if(cmd==NULL){
		EZCASTWARN("cmd is NULL!!!\n");
		return -1;
	}
	strncpy(opt_cmd,cmd,(strlen(cmd)-strlen("do_command")));
	if(strlen(opt_cmd) <= 0){
		EZCASTWARN("url error!!!\n");
		goto __END__;
	}
	EZCASTLOG("do command: %s\n", opt_cmd);
	if(strcmp(opt_cmd, "start_wifi_display") == 0 && wifidisplay_onoff == 0){
		printf("start subdisplay!!!\n");
		wifi_subdisplay_start();
		wifidisplay_onoff=1;
	}else if(strcmp(opt_cmd, "stop_wifi_display") == 0 && wifidisplay_onoff != 0){
		printf("stop subdisplay!!!\n");
		wifi_subdisplay_end();
		wifidisplay_onoff=0;
	}else if(strcmp(opt_cmd, "adbNoAudio") == 0){
		setAdbNoAudio(1);
	}else if(strcmp(opt_cmd, "startAdbSocket") == 0){
		ezFuiAdbDisplayStart(55555);
		_inform_adb_stop();
		adbDisplayStarted = 1;
	}else if(strcmp(opt_cmd, "stopAdbSocket") == 0){
		if(adbDisplayStarted != 0)
			ezFuiAdbDisplayStop();
		adbDisplayStarted = 0;
	}



__END__:
	return 1;
}



void EZ_PasswordJsonWrite(char *json_file, JSON *json_item)
{
	if (json_file != NULL && json_item != NULL)
	{
		char *json_string;
		int json_length;

		FILE *fp;

		json_string = JSON_Print(json_item);

		if (json_string != NULL)
		{
			json_length = strlen(json_string);

			if (json_length > 0 && (fp = fopen(json_file, "w")) != NULL)
			{
				fwrite(json_string, sizeof(char), json_length, fp);

				fclose(fp);
			}

			ezJSON_free(json_string);
		}
	}
}

int Write_Password_TOJson(char *name,char *psk)

{

		JSON *json_array ;


	  
		json_array = JSON_CreateArray();

		if (json_array != NULL)
		{
			
			JSON *json_item = JSON_CreateObject();
			if (json_item != NULL)
			{
				char *json_string;
				const char nil[] = "(x)";

				json_string = name;
				JSON_AddStringToObject(json_item, "act_username", json_string ? json_string : nil);

				json_string = psk;
				JSON_AddStringToObject(json_item, "act_password", json_string ? json_string : nil);

				JSON_AddItemToArray(json_array, json_item);

			}

			EZ_PasswordJsonWrite(USERINFO_JSONFILE, json_array);
			JSON_Delete(json_array);
		}

}
int Read_Password_FromJson(char *path,char *name,char *psk,int len)

{
	int ret = -1;
	FILE* fp = NULL;
	size_t read_len = -1;
	char buff[2048];
	char *buff_p = NULL;
	JSON *js, *js_sub;
	char * ap_ssid = NULL;
	fp = fopen(path, "r");
	if(fp == NULL){
		printf(" ");
		printf("open password file fail");
		ap_ssid=ezcast_wifi_get_softap_ssid();
		if(ap_ssid!=NULL)
		{
			strcat(ap_ssid,"_admin");
	             	Write_Password_TOJson(ap_ssid,"670b14728ad9902aecba32e22fa4f6bd");
			free(ap_ssid);
			ap_ssid = NULL;
		} else {
/* Mos: "Pro lan plus" could not login cause "Pro lan plus" does not have SSID,
 *  * frontend will send user string "_admin", and that is diffent with "admin", cause login fail */
             		Write_Password_TOJson("_admin","670b14728ad9902aecba32e22fa4f6bd");
		}
		fp = fopen(path, "r");
		if(fp == NULL)
			return -1;
	}

	read_len = fread(buff, 1, sizeof(buff), fp);
	if(read_len < 0){
		printf(" ");
		printf("read password file fail");
		fclose(fp);
		return -1;
	}
	fclose(fp);

	buff_p = strchr(buff, '{');
	if(buff_p == NULL){
		printf("File is not a json string!!!\n");
		return -1;
	}

	js = JSON_Parse(buff_p);
	if(js == NULL){
		printf(" ");
		printf("JSON_Parse");
		return -1;
	}

	//if(name != NULL){
		js_sub = JSON_GetObjectItem(js, "act_username");
		if(js_sub == NULL || js_sub->type != JSON_String){
			printf("Get \"%s\" error!!!\n", "User name");
			ret = -1;
		}else{
			int str_len = strlen(js_sub->valuestring);
			//printf("[%s][%d] -- str_len = %d, len = %d, final_len = %d\n", __func__, __LINE__, str_len, len, (str_len<len)?str_len:len);
			memcpy(name, js_sub->valuestring, (str_len<len)?str_len:len);
			ret = 0;
		}
		js_sub = NULL;
	//}

	//if(psk != NULL){
		js_sub = JSON_GetObjectItem(js, "act_password");
		if(js_sub == NULL || js_sub->type != JSON_String){
			printf("Get \"%s\" error!!!\n", "Password");
			ret = -1;
		}else{
			int str_len = strlen(js_sub->valuestring);
			//printf("[%s][%d] -- str_len = %d, len = %d, final_len = %d\n", __func__, __LINE__, str_len, len, (str_len<len)?str_len:len);
			memcpy(psk, js_sub->valuestring, (str_len<len)?str_len:len);
			ret = 0;
		}
		js_sub = NULL;
	//}

    /* Mos: Special case, Some device might change SSID due to some reason, but save_name is keep old,
     * That will cause login fail forever, so we need check it again, if true, modify it with new one */
    ap_ssid=ezcast_wifi_get_softap_ssid();
    if(ap_ssid!=NULL && ret == 0){
        if (strncmp(name, ap_ssid, strlen(ap_ssid)) != 0)
        {
            printf("\x1B[31m Error: SSID might change, rewrite login user name!!\x1B[0m \n");
            strcat(ap_ssid,"_admin");
            Write_Password_TOJson(ap_ssid,psk);
        }
        free(ap_ssid);
        ap_ssid = NULL;
    }

	JSON_Delete(js);
	return ret;

}
static int getMd5Sum(char *md5, const char *buf, int len){
	MD5_CTX c;
	unsigned char md[17];
	int i;

	if(md5 == NULL || buf == NULL){
		printf("md5 or buf is NULL\n");
		return -1;
	}

	//printf("len: %d\n", len);
	MD5_Init(&c);
	MD5_Update(&c, buf, len);
	MD5_Final(md, &c);
	md5[0] = '\0';
	for(i=0; i<16; i++){
		sprintf(&md5[2*i], "%02x", md[i]&0xff);
	}
	md5[32] = '\0';

	//printf("md5: %s\n", md5);

	return 0;
}
static int websetting_set_usr_password(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char real_md5[33];
	char name[50]={0};
	char *tmp=NULL;
	char *psk=NULL;
	char *ap_ssid=NULL;
	int ret=1;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("modify_usr_password")));
	else
		return -1;
	
	tmp=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");

	ap_ssid=ezcast_wifi_get_softap_ssid();
	if(ap_ssid!=NULL)
	{
		strcat(ap_ssid,"_admin");
		strncpy(name, ap_ssid,strlen(ap_ssid));
		free(ap_ssid);
		ap_ssid = NULL;
	}

	if(psk!=NULL && name!=NULL)
	{
		getMd5Sum(real_md5,psk,strlen(psk));
		Write_Password_TOJson(name,real_md5);
	}	
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf("websetting_set_usr_password  %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}


static int websetting_get_user_name(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char  save_name[64]="";
	char  save_psk[64]="";
	char name[64]={0};
	//char *psk=NULL;
	char *ap_ssid=NULL;
	int ret=0;
	Read_Password_FromJson(USERINFO_JSONFILE,save_name, save_psk,64);
	printf("save_name=%s save_psk=%s  \n",save_name,save_psk);
	if(strncmp(save_name,"admin",strlen(save_name))==0){  //add by cxf for admin...
		ap_ssid=ezcast_wifi_get_softap_ssid();
		if(ap_ssid!=NULL)
		{
			strcat(ap_ssid,"_admin");
			strncpy(name, ap_ssid,strlen(ap_ssid));
			Write_Password_TOJson(name,save_psk);
			free(ap_ssid);
			ap_ssid = NULL;
			printf("---fix----name=%s save_psk=%s  \n",name,save_psk);
		}

	}
	sprintf(CGIsetting_priv.result_str,"%s",save_name);
      // printf("websetting_get_user_name  save_name=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}
static int websetting_get_user_psk(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char  save_name[64]="";
	char  save_psk[64]="";
	char *name=NULL;
	char *psk=NULL;
	int ret=0;
	Read_Password_FromJson(USERINFO_JSONFILE,save_name, save_psk,64);
	//printf("save_name=%s save_psk=%s  \n",save_name,save_psk);
 
	sprintf(CGIsetting_priv.result_str,"%s",save_psk);
      // printf("websetting_get_user_name  save_name=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}
static int websetting_get_super_psk(char *cmd,char *val)
{
	char *CMD=cmd;
	char *ssid;
	int ret;
	char psk_change[20]={0};
	char rank_tail[20]={0};
	char  super_psk[20]={0};
	char supperpsk_md5[33]={0};

	ssid=ezcast_wifi_get_softap_ssid();
	if(ssid == NULL){
		printf("ssid[%p] is null\n", ssid);
		return NULL;
	}
	int ssid_len = strlen(ssid);
	if(ssid_len < 8){
		printf("SSID[%s:%d] is too short!!\n", ssid, ssid_len);
		return -1;
	}
	int sum = (ssid[ssid_len-1]<<24) | (ssid[ssid_len-2]<<20) 
		| (ssid[ssid_len-3]<<16) | (ssid[ssid_len-4]<<12) 
		| (ssid[ssid_len-5]<<8) | (ssid[ssid_len-6]<<4) 
		| ssid[ssid_len-7] | ssid[ssid_len-8];
	
	get_rand_psk(rank_tail,sum);	
	snprintf(psk_change, sizeof(psk_change), "%s", rank_tail);
	ret=atoi(psk_change);
	ret=(int)(ret/888);
	ret*=666;
	sprintf(val,"%d",ret);
	//printf("---------val= %s \n",val);
	char *local_version_string=ezcast_sysinfo_get_ota_local_version();
	if(local_version_string==NULL)
		return -1;
	//printf("-------local_version_string= %s \n",local_version_string);
	int i=0,k=0,n=0;
	int len1=strlen(val);
	int len2=strlen(local_version_string);
	for ( i=0;i<8;i++)
	{
		if(k<len1)
		{
			strncat(super_psk,val++,1);
			k++;
		}
			
		else
		{
			val=val-k;
			k=0;
			strncat(super_psk,val++,1);
			
		}

		if(n<len2)
		{
			strncat(super_psk,local_version_string++,1);
			n++;
		}
		else
		{
			local_version_string=local_version_string-n;
			n=0;
			strncat(super_psk,local_version_string++,1);
		}
	}
	printf("websetting_get_super_psk  super_psk=%s  \n",CGIsetting_priv.result_str);
	memset(CGIsetting_priv.encrypted_psk,0,sizeof(CGIsetting_priv.encrypted_psk));
	getMd5Sum(supperpsk_md5,super_psk,strlen(super_psk));
	strncpy(CGIsetting_priv.encrypted_psk,supperpsk_md5,strlen(supperpsk_md5));
      // printf("websetting_get_super_psk  super_psk=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.encrypted_psk;
	free(ssid);
	ssid = NULL;
	return 1;
}
static int websetting_compare_psk_modfiy(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char  save_name[64]="";
	char  save_psk[64]="";
	int ret=0;
	Read_Password_FromJson(USERINFO_JSONFILE,save_name, save_psk,64);
	//printf("save_name=%s save_psk=%s  \n",save_name,save_psk);
	if(strlen(save_psk)==0){  //if save_psk="" then sent init value  by cxf
		strncpy(save_psk,"670b14728ad9902aecba32e22fa4f6bd",strlen("670b14728ad9902aecba32e22fa4f6bd"));
		Write_Password_TOJson(save_name,save_psk);
	}
	 if(strncmp("670b14728ad9902aecba32e22fa4f6bd",save_psk,strlen(save_psk))==0)
		ret=1;
	else
		ret=0;
		
	sprintf(CGIsetting_priv.result_str,"%d",ret);
    //   printf("websetting_compare_psk_modfiy=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}
static int websetting_compare_user_psk(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char  save_name[64]="";
	char  save_psk[64]="";
	char psk_md5[33]={0};
	char *name=NULL;
	char *psk=NULL;
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("compare_user_psk")));
	else
		return -1;
	//printf("setcmd_tmp=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");
	if(psk!=NULL)
	{
		getMd5Sum(psk_md5,psk,strlen(psk));
	}
	Read_Password_FromJson(USERINFO_JSONFILE,save_name, save_psk,64);
	//printf("save_name=%s save_psk=%s encrypted_psk= \n",save_name,save_psk,CGIsetting_priv.encrypted_psk);
	if(name!=NULL&&psk!=NULL)
	{
		if(strncmp(name,save_name,strlen(save_name))==0&&strncmp(psk_md5,save_psk,strlen(save_psk))==0&&(strlen(save_psk)==strlen(psk_md5)))
		   	ret=1;     //is the same
		else if(strncmp(name,save_name,strlen(save_name))==0&&strncmp(psk_md5,CGIsetting_priv.encrypted_psk,strlen(CGIsetting_priv.encrypted_psk))==0)
		   	ret=1;     //is the same
		 else
		  	ret=0;
		 
	}
	sprintf(CGIsetting_priv.result_str,"%d",ret);
     //  printf("websetting_get_user_name  save_psk=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}
static int websetting_login_user_password(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char  save_name[64]="";
	char  save_psk[64]="";
	char *name=NULL;
	char *psk=NULL;
	char psk_md5[33];
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("login_user_password")));
	else
		return -1;
	name=strtok(setcmd_tmp,"\n");
	psk=strtok(NULL,"\n");
	if(psk!=NULL)
	{
		getMd5Sum(psk_md5,psk,strlen(psk));
	}
	Read_Password_FromJson(USERINFO_JSONFILE,save_name, save_psk,64);
	//printf("save_name=%s save_psk=%s encrypted_psk=%s \n",save_name,save_psk,CGIsetting_priv.encrypted_psk);
       if(strncmp(name,save_name,strlen(save_name))==0&&strncmp(psk_md5,save_psk,strlen(save_psk))==0&&(strlen(psk_md5)==strlen(save_psk)))
	   	ret=1;     //is the same
	else if(strncmp(name,save_name,strlen(save_name))==0&&strncmp(psk_md5,CGIsetting_priv.encrypted_psk,strlen(CGIsetting_priv.encrypted_psk))==0)
		ret=1;     //is the same
	 else
	  	ret=0;
	sprintf(CGIsetting_priv.result_str,"%d",ret);
     //  printf("websetting_login_user_password  result=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}
static int websetting_save_encrypted_psk(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char supperpsk_md5[33];
	char *value=NULL;
	int ret;
	if(cmd!=NULL)
	{
		memset(CGIsetting_priv.encrypted_psk,0,sizeof(CGIsetting_priv.encrypted_psk));
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_encrypted_psk")));
		getMd5Sum(supperpsk_md5,setcmd_tmp,strlen(setcmd_tmp));
		strncpy(CGIsetting_priv.encrypted_psk,supperpsk_md5,strlen(supperpsk_md5));
	}
	else
		return -1;
	return 1;
	
}

#define air_view_DISABLE		"/mnt/vram/ezcast/air_view"

static int websetting_get_air_view()
{
//	char ret=ezcast_sysinfo_get_air_view();
//	sprintf(CGIsetting_priv.result_str,"%c",ret);

	int air_view_Enable = 0;
	if(access(air_view_DISABLE, F_OK) != 0){ //no file
		air_view_Enable = 1;  //on
		}
	sprintf(CGIsetting_priv.result_str,"%d",air_view_Enable);
	//printf(" -------websetting_get_air_view----%c\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	//printf(" -------websetting_get_air_view--0000--%d\n",air_view_Enable);
	return air_view_Enable;
	
}
static int websetting_set_air_view(char *cmd,char *val)
{
	char setcmd_tmp[10]={0};
	char *VAL=val;
	int ret=1;
	char *values=NULL;
	char data;
	int air_view_Enable = 0;

	air_view_Enable=websetting_get_air_view();
	air_view_Enable = !air_view_Enable;
	
	//printf(" -------websetting_set_air_view--333--%d\n",air_view_Enable);

	if(air_view_Enable == 0){
		if(access(air_view_DISABLE, F_OK) != 0){
			FILE *fp = NULL;
			fp = fopen(air_view_DISABLE, "w");
			if(fp != NULL)
				fclose(fp);
		}
	//printf(" -------websetting_set_air_view--1111--\n");
	}else{
		if(access(air_view_DISABLE, F_OK) == 0){
			unlink(air_view_DISABLE);
		}
	//printf(" -------websetting_set_air_view--2222--\n");
	}
	sync();

	sprintf(CGIsetting_priv.result_str,"%d",ret);
      // printf("websetting_set_air_view ---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}

#define hdmi_cec_DISABLE		"/mnt/vram/ezcast/hdmicec_disable"
#define hdmi_audio_DISABLE 		"/mnt/vram/ezcast/hdmiaudio_disable"

static int websetting_get_hdmi_cec()
{
	int hdmi_cec_Enable = 0;
	int hdmi_audio_Enbale = 0;
	char setcmd_tmp[10]={0};
	if(access(hdmi_cec_DISABLE, F_OK) != 0){ //no file
		hdmi_cec_Enable = 1;  //on
		}
	
	sprintf(CGIsetting_priv.result_str,"%d",hdmi_cec_Enable);
	strcat(CGIsetting_priv.result_str,"\n");
	//printf(" -------websetting_get_hdmi_cec----%d\n",hdmi_cec_Enable);
	
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE == 8075)
	if(access(hdmi_audio_DISABLE, F_OK) != 0){ //no file
		hdmi_audio_Enbale = 1; //no
	}
	sprintf(setcmd_tmp, "%d", hdmi_audio_Enbale);
	strcat(setcmd_tmp,"\n");
	strcat(CGIsetting_priv.result_str, setcmd_tmp);
	//printf(" -------websetting_get_hdmi_audio----%d\n", hdmi_audio_Enbale);
	#else
	sprintf(setcmd_tmp, "%d", 0);
	strcat(setcmd_tmp,"\n");
	strcat(CGIsetting_priv.result_str, setcmd_tmp);
	//printf(" -------websetting_get_default----%d\n", 0);
	#endif
	
	CGIsetting_priv.sendline = CGIsetting_priv.result_str;
	
	return 1;
}
static int websetting_set_hdmi_cec(char *cmd,char *val)
{
	char setcmd_tmp[10]={0};
	char *VAL=val;
	int ret=1;
	char *values=NULL;
	char data;
	int cec_f; // cec enable flag
	int audio_f; // hdmi audio enable flag

	if(cmd != NULL)
		strncpy(setcmd_tmp, cmd, (strlen(cmd) - strlen("set_hdmi_cec")));
	else
		return -1;
	
	values = strtok(setcmd_tmp, "\n");
	sscanf(values, "%d", &cec_f);
	values = strtok(NULL, "\n");
	sscanf(values, "%d", &audio_f);
//	printf("--------- cec_flag = %d\t audio_flag = %d-----------\n", cec_f, audio_f);

	if(cec_f == 0){
		if(access(hdmi_cec_DISABLE, F_OK) != 0){
		FILE *fp = NULL;
		fp = fopen(hdmi_cec_DISABLE, "w");
		if(fp != NULL)
			fclose(fp);
		}
	}else{
		if(access(hdmi_cec_DISABLE, F_OK) == 0){
		unlink(hdmi_cec_DISABLE);
		}
	}


	//#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE == 8075)
	if(audio_f == 0){
		if(access(hdmi_audio_DISABLE, F_OK) != 0){
		FILE *fp = NULL;
		fp = fopen(hdmi_audio_DISABLE, "w");
		if(fp != NULL)
			fclose(fp);
		}
		int dsp = open("/dev/DAC",O_RDWR);
		if(dsp<0){
			printf("Sorry Open DAC cfg Error\n");
			return -1;
		}
		ioctl(dsp,DACIO_SET_HDMI, 0);
	//	printf("************ set hdmi audio disable *****\n");
		close(dsp);	
	}else{
		int dsp = open("/dev/DAC",O_RDWR);
		if(dsp<0){
			printf("Sorry Open DAC cfg Error\n");
			return -1;
		}
		ioctl(dsp,DACIO_SET_HDMI, 1|HDMI_VOLUME_SW_EN);
	//	printf("*********** set hdmi audio enable *****\n");
		unlink(hdmi_audio_DISABLE);
		close(dsp);
		}
	//#endif
	
	sync();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;

	return 1;
}

/***
static int websetting_set_conferent_host(char *cmd,char *val)
{
	char setcmd_tmp[256];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_conferent_host")));
	else
		return -1;
	printf("websetting_set_conferent_host setcmd_tmp=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_CONFERENT,ret);
	return 1;
}
static int websetting_set_airview_host(char *cmd,char *val)
{
	char setcmd_tmp[256];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_host_airview")));
	else
		return -1;
	printf("websetting_set_airview_host setcmd_tmp=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_AIRVIEW,ret);
	return 1;
}
static int websetting_set_airsetting_host(char *cmd,char *val)
{
	char setcmd_tmp[256];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_airopt_host")));
	else
		return -1;
	printf("websetting_set_airsetting_host setcmd_tmp=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_AIRSETTING,ret);
	return 1;
}

static int websetting_get_conferent_host(char *cmd,char *val)
{
	char setcmd_tmp[256];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_conferent_host")));
	else
		return -1;
	ret=ezCastGetNumConfig(CONFNAME_CONFERENT);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
      printf("websetting_get_conferent_host---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_airview_host(char *cmd,char *val)
{
	char setcmd_tmp[256];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_host_airview")));
	else
		return -1;
	ret=ezCastGetNumConfig(CONFNAME_AIRVIEW);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
      printf("websetting_get_airview_host---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_airsetting_host(char *cmd,char *val)
{
	char setcmd_tmp[256];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_airopt_host")));
	else
		return -1;
	ret=ezCastGetNumConfig(CONFNAME_AIRSETTING);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
      printf("websetting_get_airsetting_host---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
*/
static int websetting_confname_wr(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *value=NULL;
	char *cmd_string=NULL;
	char *cmd_name=NULL;
    	char tmp[32];
	char string[32]="";
//	char callbuf[1000]={0};
	if(cmd==NULL)
		return -1;
	
       value=strtok(cmd,"\n");
	printf("value=%s \n",value);
	cmd_string=strtok(NULL,"\n");
	printf("cmd_string=%s \n",cmd_string);
	cmd_name=strtok(NULL,"\n");
	printf("cmd_name=%s \n",cmd_name);
	//sscanf(name,"%d",&ret);
	if(strcmp(cmd_string,"set")==0){
		ret = atoi(value);
		ezCastSetNumConfig(cmd_name,ret);
	return 1;
	}
	else if(strcmp(cmd_string,"get")==0){
		ret=ezCastGetNumConfig(cmd_name);
		sprintf(CGIsetting_priv.result_str,"%d",ret);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
	}

	else if(strcmp(cmd_string,"host_val")==0){
#if 1
		ret=ezCastGetNumConfig(CONFNAME_CONFERENT);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_AIRVIEW);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_AIRSETTING);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_INTERNET_EN);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_PASSWORD_HIDE);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_REBOOT);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_DEVICEMANAGER);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_NETWORKSETUP);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		ret=ezCastGetNumConfig(CONFNAME_AIRVIEW_RATE);
			sprintf(tmp,"%d",ret);
			strcat(string,tmp);
			strcat(string,",");
		sprintf(CGIsetting_priv.result_str,"%s",string);
 #else
		FILE *fp = NULL;
		fp=fopen("/mnt/vram/ezcast/ezcast.conf","r");
		if(fp==NULL)
		{
			perror("/mnt/vram/ezcast/ezcast.conf not exit !\n");
			strncpy(CGIsetting_priv.result_str,"default",strlen("default"));
			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}		
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		if(ret > 0)
		{
			//strncpy(CGIsetting_priv.result_str,callbuf,strlen(callbuf));
			sprintf(CGIsetting_priv.result_str,"%s",callbuf);

			CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		}
	
#endif
	     //printf("------------------33333333333333websetting_get_set_CONFNAME---ret: %s  \n",CGIsetting_priv.result_str);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
	}
	return -1;
}

#if 1
 
int SendAndGetResponse(int hSocket, ezWifiPkt_t *pkt, unsigned char *buf, size_t buf_len, unsigned int data_size)
{
	int ret;
	
	ret = write(hSocket, pkt, sizeof(ezWifiPkt_t));
//	printf("write pkt ret=%d  --------------------r\n",ret);
	if (ret != sizeof(ezWifiPkt_t))
	{
//		printf("write pkt error\n");

		return -1;
	}

	if (data_size != 0)
	{

	     // printf( "write buf =%s data_size=%d      ---------------------\n",buf,data_size);
		ret = write(hSocket, buf, data_size);
		if (ret != data_size)
		{
			printf( "write buf error\n");

			return -1;
		}
	}
	
	// read result
	fd_set readFds;
	struct timeval timeout;
	int nRead = 0;
	
	FD_ZERO(&readFds);
	FD_SET(hSocket, &readFds);
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	ret = select(hSocket+1, &readFds, NULL, NULL, &timeout);
	if (ret == 0) //Timeout
	{
		printf("select timeout\n");

		return -1;
	}
	else if (ret > 0)
	{
		if (FD_ISSET(hSocket, &readFds))
		{
			nRead = read(hSocket, buf, TCPPACKET_HDR_SIZE);
			if (nRead != TCPPACKET_HDR_SIZE)
			{
				printf("read error\n");

				return -1;
			}
		}
	}
	else
	{
		printf("select error\n");

		return -1;
	}

	return 0;
}

 int set_internet_access_control(char *ip,char *cmd)

{
	char call_buf[128];
	memset(call_buf,0x00,128);
	sprintf(call_buf,"sh /am7x/case/scripts/network_control.sh %s %s",ip,cmd);
	system(call_buf);

}
void deal_recv_data(char *buf)
{
	char *locate_start=NULL;
	char *locate_end=NULL;
	char *locate=NULL;
	char ip_buf[64];
	char position_buf[6];
	int i=0;
	int j=0;
	if(buf!=NULL)
	{
		
		locate=buf;
		//printf("*******buf=%s*********\n",buf);
		while(1)
		{
			locate_start=strstr(locate,"<Position>");
			if(locate_start==NULL)
				break;
			else
			{
				locate_start+=strlen("<Position>");
				//printf("locate_start=%s\n",locate_start);
				locate_end=strstr(locate_start,"</Position>");
				//printf("locate_end=%s\n",locate_end);
				if(locate_end!=NULL)
				{
					memset(position_buf,0,sizeof(position_buf));
					strncpy(position_buf,locate_start,strlen(locate_start)-strlen(locate_end));
					recv_ip_info[i].position=atoi(position_buf);
					//printf("recv_ip_info[%d].position=%d\n",i,recv_ip_info[i].position);
					if(recv_ip_info[i].position>0)
						recv_ip_info[i].position=1;//playing
					else
						recv_ip_info[i].position=0;//not playing
					if(recv_ip_info[i].position>0)
					{
					 	locate_start=strstr(locate_end,"<IPAddress>");
						if(locate_start==NULL)
							break;
						else
						{
							locate_start+=strlen("<IPAddress>");
							locate_end=strstr(locate_start,"</IPAddress>");
							if(locate_end!=NULL)
							{
								memset(recv_ip_info[i].projective_ip,0,sizeof(recv_ip_info[0].projective_ip));
								strncpy(recv_ip_info[i].projective_ip,locate_start,strlen(locate_start)-strlen(locate_end));
								//printf("recv_ip_info[%d].projective_ip=%s\n",i,recv_ip_info[i].projective_ip);
								locate_end+=strlen("</IPAddress>");
								locate=locate_end;
								i++;
		
							}
						}
						
					}
					else
					{
						locate_end+=strlen("</Position>");
						locate=locate_end;
					}
					total_num=i;
				}
				else
					break;

			}
			
				
		}
		for (i=0;i<total_num;i++)
			{
				for (j=0;j<last_total_num;j++)
				{
					if(strcmp(recv_ip_info[i].projective_ip,last_recv_ip_info[j].projective_ip)==0)
					{
					 	break;

					}
		 	     }
			    if(j==last_total_num)
			    {			 	
							//ok
					//printf("playing-----------------------3\n");
					set_internet_access_control(recv_ip_info[i].projective_ip,"ACCEPT");

			    }

			
	   		}
			for (i=0;i<total_num;i++)
			{
				memset(last_recv_ip_info[i].projective_ip,0,sizeof(last_recv_ip_info[0].projective_ip));
				memcpy(last_recv_ip_info[i].projective_ip,recv_ip_info[i].projective_ip,sizeof(recv_ip_info[0].projective_ip));
				last_recv_ip_info[i].position=recv_ip_info[i].position;
			}
			if(last_total_num>total_num)
			{
				for (;i<last_total_num;i++)
				{
					set_internet_access_control(last_recv_ip_info[i].projective_ip,"DROP");
					memset(last_recv_ip_info[i].projective_ip,0,sizeof(last_recv_ip_info[0].projective_ip));
					last_recv_ip_info[i].position=0;
				}	
			}
			last_total_num=total_num;

	}


}
static void *receive_tcpmsg_loop(void *arg)

{

	unsigned char buf[BUF_LEN];
	ezWifiPkt_t pkt;
	unsigned int data_size = 0;	
	struct sockaddr_in Address;  /* Internet socket address stuct */
	memset(&pkt, 0, sizeof(ezWifiPkt_t));
	pkt.seqNo = 0;
	pkt.totalSize = TCPCOMMON_HDR_SIZE;
	pkt.tag = PICO_QUERY_INFO;
	pkt.flag = FLAG_DPF_TO_DPF;
	pkt.len = sizeof(ezWifiQueryCdb_t);

	ezWifiQueryCdb_t cdb;
	memset(&cdb, 0, sizeof(ezWifiQueryCdb_t));
	cdb.info = INFO_TCP_CONNECTIONS;
	memcpy(&pkt.cdb, &cdb, sizeof(ezWifiQueryCdb_t));
	 
    /* connect to host */
     system("ifconfig lo up");
     if(first_start_flag==1)
     {
     		sleep(20);
     }
	    /* make a socket */
    hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(hSocket == SOCKET_ERROR)
    {
	printf("socket error\n");;
	return -1;
    }
   
    printf(" ==========================Start_TcpSocket==============================\n");
    /* fill address struct */
    Address.sin_addr.s_addr=inet_addr(DEFAULT_HOST_NAME);
    Address.sin_port=htons(DEFAULT_HOST_PORT);
    Address.sin_family=AF_INET;	
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
     {
		printf("--------------------connect error---------------------------\n");
		perror("error");
		return -1;
      }	
      while(ezCastGetNumConfig(CONFNAME_INTERNET_CONTROL)==1)
	{
               sleep(2);
		if (wifidisplay_onoff==1&&SendAndGetResponse(hSocket, &pkt, buf, sizeof(buf), 0) == 0)
		{
			unsigned long result = ((unsigned long *)(buf+TCP_HDR_QUERYRESULT_OFFSET))[0];
			if (result != QUERY_INFO_OK)
			{
				printf("result != QUERY_INFO_OK, result=%ld\n", result);

				continue;
			}
			
			data_size = ((unsigned long*)(buf+TCP_HDR_QUERYISIZE_OFFSET))[0];
			if (data_size == 0)
			{
				printf("data_size == 0\n");

				continue;
			}
			
			int nRead = read(hSocket, buf, data_size);
			deal_recv_data(buf);
			
			if (nRead != data_size)
			{
				//printf("nRead != data_size, nRead=%d, data_size=%d\n", nRead, data_size);

				continue;
			}
		
			if (data_size + 1 <= sizeof(buf))
			{
				buf[data_size] = 0; // add null terminator
			}

			//printf("%s", buf);
		}
		else

		{
		    
				//printf("err==========wifidisplay_onoff=%d===hSocket=%d=====\n",wifidisplay_onoff,hSocket);
				if(wifidisplay_onoff==1)
				{
				           hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  					   if(hSocket == SOCKET_ERROR)
					    {
						printf("socket error\n");;
						continue;
					    }
					  if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
					   {
							printf("1111-------------------connect error---------------------------\n");
							perror("error");
							continue;
					   }
				}
				else
					 if(hSocket != SOCKET_ERROR)
				       {
						close(hSocket);
						hSocket=-1;
						printf("socket error\n");
						int i=0;
						for (i=0;i<total_num;i++){
							set_internet_access_control(recv_ip_info[i].projective_ip,"DROP");
							memset(recv_ip_info[i].projective_ip,0,sizeof(recv_ip_info[0].projective_ip));
							recv_ip_info[i].position=0;
						}	
						continue;
				       }					
				continue;

		}

	}
	 if(hSocket != SOCKET_ERROR)
	{
		close(hSocket);
		hSocket=-1;
	}
	 	last_total_num=0;
       printf("pthread_exit start!!!!!!!!!!!!!!!!!!!!!%s,%d\n",__FUNCTION__,__LINE__); 
	pthread_exit(NULL);
	return NULL;


}
int Start_TcpSocket(int flag) {

  	pthread_t looper;
	printf(" start!!!!!!!!!!!!!!!!!!!!!%s,%d\n",__FUNCTION__,__LINE__); 
	if(flag==1)
		first_start_flag=0;
	set_internet_access_control("ALL","DROP");
	if (pthread_create(&looper, NULL, receive_tcpmsg_loop, NULL) != 0)
	{
		printf("[%s]: recv send tcpmsg  loop create error\n",__FUNCTION__);
		exit(EXIT_FAILURE);
		 
	}	
	return 0;
}
#endif

static int websetting_get_internet_access_control(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_internet_access_control")));
	else
		return -1;
	ret=ezCastGetNumConfig(CONFNAME_INTERNET_CONTROL);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_get_airsetting_host---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_set_internet_access_control(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_internet_access_control")));
	else
		return -1;
	printf("websetting_set_internet_access_controlsetcmd_tmp=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_INTERNET_CONTROL,ret);
	if(ret==2)
		 set_internet_access_control("ALL","DROP");
	else if(ret==1)
		Start_TcpSocket(1);
	else
		set_internet_access_control("ALL","ACCEPT");
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_set_internet_access_control---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
void  websetting_set_internet_access_control_state(void)
{
		int ret_tmp=0;
		char cmd_tmp[256];
		int control_state_rtn=0;
	
		ret_tmp=ezCastGetNumConfig(CONFNAME_INTERNET_CONTROL);
		printf("[%s][%d]ret=%d",__FILE__,__LINE__,ret_tmp);
		sprintf(cmd_tmp,"%d",ret_tmp);
		strcat(cmd_tmp,"set_internet_access_control");
		control_state_rtn=websetting_set_internet_access_control(cmd_tmp,NULL);
		printf("[%s][%d]control_state_rtn=%d\n",__FILE__,__LINE__,control_state_rtn);
}

static int websetting_set_passcode_onoff(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_passcode")));
	else
		return -1;
	printf("websetting_set_passcode_onoff=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_PASSCODE,ret);
	if(ret==2)
	{
		name=strtok(NULL,"\n");
		printf("name=%s \n",name);
		sscanf(name,"%d",&ret);
		ezCastSetNumConfig(CONFNAME_PASSCODE_VAL,ret);
	}
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_set_passcode_onoff---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_passcode_onoff(char *cmd,char *val)
{
	int ret=0;
	char *name=NULL;
	ret=ezCastGetNumConfig(CONFNAME_PASSCODE);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_get_passcode_onoff---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_passcode_val(char *cmd,char *val)
{
	int ret=0;
	char *name=NULL;
	ret=ezCastGetNumConfig(CONFNAME_PASSCODE_VAL);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_get_passcode_val---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_set_miracode_onoff(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_miracode")));
	else
		return -1;
	printf("websetting_set_miracode_onoff=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_MIRACODE,ret);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_set_miracode_onoff---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	
	return 1;
}
	static int websetting_set_channel(char *cmd,char *val)
{
	char setcmd_tmp[20]={0};
	int ret=0;
	int value=0;
	char *channel=NULL;
	char *bandwidth=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_channel")));
	else
		return -1;
	printf("websetting_set_channel=%s \n",setcmd_tmp);
	channel=strtok(setcmd_tmp,"\n");
	bandwidth=strtok(NULL,"\n");
 	sscanf(bandwidth,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_BANDWIDTH_24G,ret);
	sscanf(channel,"%d",&ret);
	setChannel(ret,wifiChannel_mode_24G);

	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_set_channel---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}
	static int websetting_get_softap_channel(char *cmd,char *val)
{
	char setcmd_tmp[20]={0};
	int tmp_val=0;
	tmp_val=ezCastGetNumConfig(CONFNAME_SOFTAP_CHANNEL);//get_softap_channel();
	sprintf(CGIsetting_priv.result_str,"%d",tmp_val);
	strcat(CGIsetting_priv.result_str,"\n");
	tmp_val=ezCastGetNumConfig(CONFNAME_BANDWIDTH_24G);
	sprintf(setcmd_tmp,"%d",tmp_val);
	strcat(setcmd_tmp,"\n");
	strcat(CGIsetting_priv.result_str,setcmd_tmp);
	printf("websetting_get_softap_channel---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}
	/*
	static int websetting_get_softap_5G_channel(char *cmd,char *val)
{
	char setcmd_tmp[20];
	int channel=0;
	channel=ezCastGetNumConfig(CONFNAME_SOFTAP_5G_CHANNEL);//get_softap_5g_channel();
	sprintf(CGIsetting_priv.result_str,"%d",channel);
	printf("websetting_get_softap_5G_channel---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}
*/
	static int websetting_set_softap_5G_country(char *cmd,char *val)
{
		char setcmd_tmp[256]={0};
		int ret=0;
		char *name=NULL;
		char *channel=NULL;
		char *bandwidth=NULL;// CONFNAME_BANDWIDTH_24G
		int status=0,name_val=0,channel_val=0,bandwidth_val,i=0;//add for anmp

		status=ProboxGetConnectStatus();
		if(status!=1)//probox not connecting 5G routing  
		{
			if(cmd!=NULL)
				strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_5G_country")));
			else
				return -1;
			printf("websetting_set_softap_5G_country=%s \n",setcmd_tmp);
			name=strtok(setcmd_tmp,"\n");
			channel=strtok(NULL,"\n");
			bandwidth=strtok(NULL,"\n");
			printf("name=%s channel=%s bandwidth=%s\n ",name,channel,bandwidth);
			name_val=atoi(name);
			channel_val=atoi(channel);
			for(i=0;i<channel5g_regiontbl[name_val].total_5gchannel;i++)
			{
				if((channel_val==0)||(channel_val==channel5g_regiontbl[name_val].channel_plan[i]))
				{
					sscanf(name,"%d",&ret);
					ezCastSetNumConfig(CONFNAME_SOFTAP_5G_COUNTRY,ret);
					sscanf(bandwidth,"%d",&ret);
					ezCastSetNumConfig(CONFNAME_BANDWIDTH_5G,ret);					
					sscanf(channel,"%d",&ret);
					setChannel(ret,wifiChannel_mode_5G);
					return 1;
				}

			}

		}
		else
			return -1;

}
	static int websetting_get_softap_5G_country(char *cmd,char *val)
{
	char setcmd_tmp[20]={0};
	int tmp_val=0;
	printf("[%s][%d]\n",__FILE__,__LINE__);
	tmp_val=ezCastGetNumConfig(CONFNAME_SOFTAP_5G_COUNTRY);//get_softap_5g_channel();
	sprintf(CGIsetting_priv.result_str,"%d",tmp_val);
	strcat(CGIsetting_priv.result_str,"\n");
	tmp_val=ezCastGetNumConfig(CONFNAME_SOFTAP_5G_CHANNEL);
	sprintf(setcmd_tmp,"%d",tmp_val);
	strcat(setcmd_tmp,"\n");
	strcat(CGIsetting_priv.result_str,setcmd_tmp);
	tmp_val=ezCastGetNumConfig(CONFNAME_BANDWIDTH_5G);
	sprintf(setcmd_tmp,"%d",tmp_val);
	strcat(setcmd_tmp,"\n");
	strcat(CGIsetting_priv.result_str,setcmd_tmp);
	printf("websetting_get_softap_5G_channel---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}

	static int get_channel_value(char *conf_file,char *confbak_file)
	{

		int channel=-1;
		FILE *fp = NULL;
		char *locate=NULL,*locate1=NULL;
		char buf[4096] ={0};
		char callbuf[128];
		char tmp[128]={0};
		int ret=-1;
		fp = fopen(conf_file,"r");
		if(fp == NULL)
		{	
			printf("open conf_file error\n");
			sprintf(callbuf,"cp %s  %s",confbak_file,conf_file);
			system(callbuf);
			fp = fopen(conf_file,"r");
			if(fp == NULL){
				printf("%s %s \n",__func__,strerror(errno));
				return -1;
			}
		}
		ret=fread(buf, 1, 4096, fp);
		ret=fclose(fp);
		locate1 = strstr(buf,"channel=");
		if(locate1!=NULL)
		{
			locate1=locate1+strlen("channel=");
			char *locate2=strstr(locate1,"\n");
			memset(tmp,0,128);
			if(locate1<locate2)
				memcpy(tmp,locate1,strlen(locate1)-strlen(locate2));
		}
		channel=atoi(tmp);
		printf("get_channel_value=%d\n",channel);
		return channel;
	}
		static int websetting_get_auto_channel_val(char *cmd,char *val)
	{
		char setcmd_tmp[20]={0};
		int channel24g=0;
		int channel5g=0;

		char conf_file[100]="";
		char confbak_file[100]="";
		sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
		sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		char conf_file24g[100]="/mnt/user1/softap/rtl_hostapd_02.conf";
		char confbak_file24g[100]="/mnt/bak/rtl_hostapd_02.conf";
		channel5g=get_channel_value(conf_file,confbak_file);
		channel24g=get_channel_value(conf_file24g,confbak_file24g);
#else
		channel24g=get_channel_value(conf_file,confbak_file);
#endif

		sprintf(CGIsetting_priv.result_str,"%d",channel24g);
		strcat(CGIsetting_priv.result_str,"\n");
		sprintf(setcmd_tmp,"%d",channel5g);
		strcat(setcmd_tmp,"\n");
		strcat(CGIsetting_priv.result_str,setcmd_tmp);

		printf("websetting_get_softap_channel---ret: %s  \n",CGIsetting_priv.result_str);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
		return 1;
	}

	static int websetting_get_ProboxConnectStatus(char *cmd,char *val)
{
	int value =0;
	value=ProboxGetConnectStatus();
	sprintf(CGIsetting_priv.result_str,"%d",value);
	printf("websetting_set_channel---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}	


	static int websetting_get_config_val(char *cmd,char *val)
{
	Get_Config_Val();
	return 1;
}
static int websetting_get_miracode_onoff(char *cmd,char *val)
{
	int ret=0;
	char *name=NULL;
	ret=ezCastGetNumConfig(CONFNAME_MIRACODE);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_get_miracode_onoff---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	
	return 1;
}

static int websetting_get_otaserver_path(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char tmp[512];
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_otaserver_path")));
	else
		return -1;
	memset(tmp, 0, sizeof(tmp));
	ret=ezCastGetStrConfig(CONFNAME_OTASERVER, tmp, sizeof(tmp));
//	printf("---websetting_get_otaserver_path---tmp: %s  \n",tmp);

	if(ret == 0){
	sprintf(CGIsetting_priv.result_str,"%s",tmp);
      printf("---websetting_get_otaserver_path---result_str: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
	}
	else{
		CGIsetting_priv.sendline=0;
		return 0;
		}
	
}

static int websetting_set_otaserver_path(char *cmd,char *val)
{
	char setcmd_tmp[512]={0};
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_otaserver_path")));
	else
		return -1;
	if(!strcmp(setcmd_tmp,""))
		sprintf(setcmd_tmp,"%s","nil");
	ezCastSetStrConfig(CONFNAME_OTASERVER,setcmd_tmp);
	return 1;
}

int resetConfigDefault()
{
	char * ap_ssid = NULL;
	printf(" -------reset_default----\n");
	//ezcast_sysinfo_set_local_language(0);
	//ezcast_sysinfo_set_disauto_status(0);

	ap_ssid=ezcast_wifi_get_softap_ssid();
	if(ap_ssid!=NULL)
	{
		strcat(ap_ssid,"_admin");
		Write_Password_TOJson(ap_ssid,"670b14728ad9902aecba32e22fa4f6bd");
		free(ap_ssid);
		ap_ssid = NULL;
	} else {
	/* Mos: "Pro lan plus" could not login cause "Pro lan plus" does not have SSID, 
	* frontend will send user string "_admin", and that is diffent with "admin", that cause login failed */
		Write_Password_TOJson("_admin","670b14728ad9902aecba32e22fa4f6bd");
	}

	ezcast_wifi_dongle_mode_change_close_process();
	ezcast_wifi_stop();
	ezcast_sysinfo_delete_wifi_conf();
	ezcast_deleteEZCastConf();
	if(access(hdmi_cec_DISABLE, F_OK) == 0){
		unlink(hdmi_cec_DISABLE);
	}
	if(access(air_view_DISABLE, F_OK) == 0){
		unlink(air_view_DISABLE);
	}
	if(access(EZCAST_CONF, F_OK) == 0){
		unlink(EZCAST_CONF);
	}
	apps_vram_set_default();

	//_do_action(CMD_SET_DEFAULT_ENV,NULL);
	system("reboot");
}

static int websetting_reset_default()
{
	resetConfigDefault();
}

static int websetting_reboot_system()
{
	printf(" -------reboot----\n");
	system("reboot");
}
static int websetting_set_routeronly(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_routeronly")));
	else
		return -1;
	printf("websetting_set_routeronly setcmd_tmp=%s \n",setcmd_tmp);
       name=strtok(setcmd_tmp,"\n");
	printf("name=%s \n",name);
	sscanf(name,"%d",&ret);
	//ezCastSetNumConfig(CONFNAME_ROUTERONLY,ret);
	if(!ret)
	      wifi_softap_start();
	return 1;
}
static int websetting_get_routeronly(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	char *name=NULL;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_routeronly")));
	else
		return -1;
	ret=0;//ezCastGetNumConfig(CONFNAME_ROUTERONLY);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
      printf("websetting_get_routeronly ---ret: %s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
#if MODULE_CONFIG_EZWIRE_ENABLE
static int websetting_get_netlink_status()
{
	int ret = websetting_sysinfo_get_ezwire_status();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf(" -------ezwire_get_netlink_status----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;

}
static int websetting_set_ezwire_audio_status(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_ezwireaudio")));
	else
		return -1;
	
	sscanf(setcmd_tmp,"%d",&ret);
	aoa_disable(ret);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf(" -------websetting_set_ezwire_audio_status----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;

}
static int websetting_get_ezwire_audio_status()
{
	int ret = aoa_disable(-1);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf(" -------websetting_get_ezwire_audio_status----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;

}

#endif
enum{
	LANSETTING_IP = 0,
	LANSETTING_GATEWAY,
	LANSETTING_MASK,
	LANSETTING_DNS1,
	LANSETTING_DNS2
};

#if EZWILAN_ENABLE
static int websetting_get_netlink_status()
{
	int ret=websetting_sysinfo_get_netlink_status();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf(" -------websetting_get_netlink_status----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;

}
static int websetting_lan_getLanAutomatic()
{
	int ret=webset_lan_getLanAutomaticEnable();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf(" -------websetting_lan_getLanAutomatic----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}
static int websetting_lan_setLanAutomatic()
{
	int ret=webset_lan_setLanAutomatic_on();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf(" -------websetting_lan_getLanAutomatic----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}
static int websetting_lan_setLanAutomatic_off()
{
	int ret=webset_lan_setLanAutomatic_off();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf(" -------websetting_lan_getLanAutomatic----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}

static int websetting_lan_storeLanManualInfo(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char *value=NULL;
	int ret;
	
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("writeLanManualInfo")));
	else
		return -1;

	value=&setcmd_tmp;
	//printf(" ---lan_storeLanManualInfo----ip----%s\n",value);

	ret=webset_lan_storeLanManualInfo(value);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf(" -------lan_storeLanManualInfo----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;

}


static int websetting_lan_getLanSetting_Val(char *cmd,char *val)
{
	
	char *CMD=cmd;
	char *ret=NULL;
	
	ret=getLanSettingVal(LANSETTING_IP);
	if(ret != NULL)
		sprintf(CGIsetting_priv.result_str,"%s",ret);
	ret=getLanSettingVal(LANSETTING_GATEWAY);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=getLanSettingVal(LANSETTING_MASK);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=getLanSettingVal(LANSETTING_DNS1);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=getLanSettingVal(LANSETTING_DNS2);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	//printf("--------------websetting_lan_getLanSetting_Val  %s  \n",CGIsetting_priv.result_str);
	strcat(CGIsetting_priv.result_str,"\n");					
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

static int websetting_lan_getLan_auto_ip(char *cmd,char *val)
{
	
	char *CMD=cmd;
	char *ret=NULL;
	
	ret=webset_getLanSettingautoVal(LANSETTING_IP);
	if(ret != NULL)
		sprintf(CGIsetting_priv.result_str,"%s",ret);
	ret=webset_getLanSettingautoVal(LANSETTING_GATEWAY);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=webset_getLanSettingautoVal(LANSETTING_MASK);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=webset_getLanSettingautoVal(LANSETTING_DNS1);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=webset_getLanSettingautoVal(LANSETTING_DNS2);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	//printf("--------------websetting_lan_getLanSetting_Val  %s  \n",CGIsetting_priv.result_str);
	strcat(CGIsetting_priv.result_str,"\n");					
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

static int websetting_lan_get_mac_ip(char *cmd,char *val)
{
	char *VAL=val;
	char *CMD=cmd;
	char * mac_addr=webset_lan_getMacAddress();
	if(mac_addr!=NULL)
	{
		strncpy(CGIsetting_priv.result_str,mac_addr,strlen(mac_addr));
	}
	else 
		return -1;
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	//printf("--------------websetting_lan_get_mac_ip  %s  \n",CGIsetting_priv.result_str);
	return 1;
	
}

#endif

static int websetting_wifi_setWiFiAutomatic_off()//close wlan set
{
	int ret=webset_wifi_setWiFiAutomatic_off();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf(" =====webset_wifi_getWiFiAutomaticEnable===off====%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}

static int websetting_wifi_setWiFiAutomatic()//open wlan set
{
	int ret=webset_wifi_setWiFiAutomatic_on();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf(" =====webset_wifi_getWiFiAutomaticEnable===on====%d\n",ret);
	webset_wifi_DeleteWiFiManualInfo();
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}

static int websetting_wifi_getWiFiAutomatic()//get wlan set
{
	int ret=webset_wifi_getWiFiAutomaticEnable();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf(" -------websetting_wifi_getLanAutomatic----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;
}


static int websetting_wifi_storeWiFiManualInfo(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	char *value=NULL;
	int ret;
	
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("writeWiFiManualInfo")));
	else
		return -1;

	value=&setcmd_tmp;
	printf(" ---websetting_wifi_storeWiFiManualInfo----ip----%s\n",value);

	ret=webset_wifi_storeWiFiManualInfo(value);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf(" -------lan_storeLanManualInfo----%d\n",ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;	
	return 1;

}




static int websetting_wifi_getWiFiSetting_Val(char *cmd,char *val)
{
	
	char *CMD=cmd;
	char *ret=NULL;
	
	ret=getWiFiSettingVal(LANSETTING_IP);
	if(ret != NULL)
		sprintf(CGIsetting_priv.result_str,"%s",ret);
	ret=getWiFiSettingVal(LANSETTING_GATEWAY);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=getWiFiSettingVal(LANSETTING_MASK);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=getWiFiSettingVal(LANSETTING_DNS1);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=getWiFiSettingVal(LANSETTING_DNS2);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	printf("--------------websetting_wifi_getWiFiSetting_Val  %s  \n",CGIsetting_priv.result_str);
	strcat(CGIsetting_priv.result_str,"\n");					
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

static int websetting_wifi_getWiFi_auto_ip(char *cmd,char *val)
{
	
	char *CMD=cmd;
	char *ret=NULL;
	
	ret=webset_getWiFiSettingautoVal(LANSETTING_IP);
	if(ret != NULL)
		sprintf(CGIsetting_priv.result_str,"%s",ret);
	ret=webset_getWiFiSettingautoVal(LANSETTING_GATEWAY);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=webset_getWiFiSettingautoVal(LANSETTING_MASK);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=webset_getWiFiSettingautoVal(LANSETTING_DNS1);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	ret=webset_getWiFiSettingautoVal(LANSETTING_DNS2);
	if(ret != NULL){
		strcat(CGIsetting_priv.result_str,"\n");					
		strcat(CGIsetting_priv.result_str,ret);
		}
	//printf("--------------websetting_wifi_getWiFiSetting_Val  %s  \n",CGIsetting_priv.result_str);
	strcat(CGIsetting_priv.result_str,"\n");					
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
#if EZWILAN_ENABLE

static int websetting_wifi_getip(char *cmd,char *val)//for snmp
{
	int ret=webset_wifi_getWiFiAutomaticEnable();
	printf("@@websetting_wifi_getip =%d\n",ret);	
	if(ret==0)
		websetting_wifi_getWiFiSetting_Val(cmd,val);
	else
		websetting_wifi_getWiFi_auto_ip(cmd,val);
	return 1; 

}

static int websetting_wifi_setip(char *cmd,char *val)
{

	
	websetting_wifi_storeWiFiManualInfo(cmd,val);
	int ret=webset_wifi_setWiFiAutomatic_off();		
	printf("@@websetting_wifi_setip =%d[]\n",ret);
	return 1;

}
static int websetting_lan_getip(char *cmd,char *val)//for snmp
{
	int ret=webset_lan_getLanAutomaticEnable();
	printf("@@websetting_lan_getip =%d\n",ret);	
	if(ret==0)
		websetting_lan_getLanSetting_Val(cmd,val);
	else
		websetting_lan_getLan_auto_ip(cmd,val);
	return 1; 

}
static int websetting_lan_setip(char *cmd,char *val)
{		
	websetting_lan_storeLanManualInfo(cmd,val);
	int ret=webset_lan_setLanAutomatic_off();
	printf("@@websetting_lan_setip =%d[]\n",ret);	
	return 1;

}

#endif

#if EZMUSIC_ENABLE || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
static int websetting_set_ezchannel_playway(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_ezchannel_playway")));
	else
		return -1;
	sscanf(setcmd_tmp,"%d",&ret);
	printf("websetting_set_ezchannel_playwayret=%d  \n",ret);
	ezChannel_SetPlayWay(ret);
	
	return 1;
}
static int websetting_get_ezchannel_playway(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("get_ezchannel_playway")));
	else
		return -1;
	ret=ezChannel_GetPlayWay();
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_set_ezchannel_playway  result=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

#endif
static int websetting_get_dev_management_res(char *cmd, char *var){
	char setcmd_tmp[256]={0};
	char dev_name[100]; // Notice: set the max length of device name is 100!
	char default_mode[4];
	char last_resolution[4];
	char sysHostname[128] = {0};
	char *psk_val;
	char connection_mode[4];
	char languageIndex[8];
	char ip_addr[16];
	int i;	   
	int string_length = 0;
	int res;
	int lan_index = 0;
	char *ap_ssid;
	char *ret = NULL;
	char *string_temp = NULL;

	// get wifi mode
	sprintf(CGIsetting_priv.result_str, "%d", ezcast_sysinfo_get_wifi_mode());
	strcat(CGIsetting_priv.result_str, "\t");
	
	// get LAN set
	#if EZWILAN_ENABLE
		res = webset_lan_getLanAutomaticEnable();
	#else
		res = 0;
	#endif
	sprintf(ip_addr,"%d", ret);
	strcat(CGIsetting_priv.result_str, ip_addr);

	// get setting var
	/*
	if (res != 1){
		ret = getLanSettingVal(LANSETTING_IP);
		if(ret != NULL){
			sprintf(ip_addr,"%s", ret);
			strcat(CGIsetting_priv.result_str, ip_addr);
		}
		
		ret = getLanSettingVal(LANSETTING_GATEWAY);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str, "\n");					
			strcat(CGIsetting_priv.result_str, ret);
		}
		
		ret = getLanSettingVal(LANSETTING_MASK);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str, "\n");					
			strcat(CGIsetting_priv.result_str, ret);
		}
		
		ret = getLanSettingVal(LANSETTING_DNS1);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str, "\n");					
			strcat(CGIsetting_priv.result_str, ret);
		}
		
		ret = getLanSettingVal(LANSETTING_DNS2);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str, "\n");					
			strcat(CGIsetting_priv.result_str, ret);
		}
		//printf("--------------websetting_lan_getLanSetting_Val  %s  \n",CGIsetting_priv.result_str);
	}else{
		// auto ip
		ret = webset_getWiFiSettingautoVal(LANSETTING_IP);
		if(ret != NULL){
			sprintf(ip_addr,"%s", ret);
			strcat(CGIsetting_priv.result_str, ip_addr);
		}
		
		ret = webset_getWiFiSettingautoVal(LANSETTING_GATEWAY);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str,"\n");					
			strcat(CGIsetting_priv.result_str,ret);
		}
		
		ret = webset_getWiFiSettingautoVal(LANSETTING_MASK);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str,"\n");					
			strcat(CGIsetting_priv.result_str,ret);
		}
		
		ret = webset_getWiFiSettingautoVal(LANSETTING_DNS1);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str,"\n");					
			strcat(CGIsetting_priv.result_str,ret);
		}
		
		ret=webset_getWiFiSettingautoVal(LANSETTING_DNS2);
		if(ret != NULL){
			strcat(CGIsetting_priv.result_str,"\n");					
			strcat(CGIsetting_priv.result_str,ret);
		}
		//printf("--------------websetting_wifi_getWiFiSetting_Val  %s  \n",CGIsetting_priv.result_str);
	}
	*/
	
	strcat(CGIsetting_priv.result_str, "\t");					
	
	// get Language
	// read currentLanguageId:	
	lan_index = ezcast_sysinfo_get_local_language();
	SysCGI_priv.language_index = lan_index;
	#if (EZMUSIC_ENABLE)
		check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
	#endif
	// read disauto status to html display:
	if(!ezcast_sysinfo_get_disauto_status()){
		sprintf(languageIndex, "auto");	
	}else{	
		sprintf(languageIndex, "%s", language_item[lan_index]);
	}
	strcat(CGIsetting_priv.result_str, languageIndex);	
	strcat(CGIsetting_priv.result_str,"\t");

	// get device name
	res = ezcast_get_device_name(dev_name, sizeof(dev_name));
	strcat(CGIsetting_priv.result_str, dev_name);
	strcat(CGIsetting_priv.result_str,"\t");
	
	// get default mode
	res = Get_last_UI();
	sprintf(default_mode, "%d", res);
	strcat(CGIsetting_priv.result_str, default_mode);
	strcat(CGIsetting_priv.result_str, "\t");
	
	// get resolution
	#if(MODULE_CONFIG_EZCASTPRO_MODE == 8075 ||MODULE_CONFIG_EZWIRE_ENABLE == 1)
	if(ezcast_sysinfo_read_edid_bin(0) == 8){//vga only  mode add 200 for vga flag	
		res = ezcast_sysinfo_read_edid_bin(1);
		printf("websetting read edid res_tmp=%d\n",res);
		if(res == 0)   //VGA_1280x1024  VGA_1024x768  VGA only
			CGIsetting_priv.last_resolution = 200 + ezcast_sysinfo_read_edid_bin(1);
		else if(res == 9)   
			CGIsetting_priv.last_resolution = 2;    //turn to hdmi value  1920*1080 30P
		else if(res == 6)   
			CGIsetting_priv.last_resolution = 17;
		else if(res == 5)   
			CGIsetting_priv.last_resolution = 16; 
		else if(res == 8)   
			CGIsetting_priv.last_resolution = 5;
		else if(res == 2)   
			CGIsetting_priv.last_resolution = 15; 
		else 
			CGIsetting_priv.last_resolution = 5; 
	}else{
		CGIsetting_priv.last_resolution = ezcast_sysinfo_read_edid_bin(1);
	}
	#else
		CGIsetting_priv.last_resolution = ezcast_sysinfo_read_edid_bin(1);

	#endif
	sprintf(last_resolution,"%d",CGIsetting_priv.last_resolution);
	strcat(CGIsetting_priv.result_str, last_resolution);
	strcat(CGIsetting_priv.result_str, "\t");
	
	// get connection
	sprintf(connection_mode,"%d",ezcast_sysinfo_get_router_ctl());
	strcat(CGIsetting_priv.result_str, connection_mode);
	strcat(CGIsetting_priv.result_str, "\t");
	
	// get ssid
	#if MODULE_CONFIG_EZWIRE_ENABLE
		res = gethostname(sysHostname, sizeof(sysHostname)); //ezwire id
		if(res == 0){
			strcat(CGIsetting_priv.result_str, sysHostname);
		}else{
			strcat(CGIsetting_priv.result_str, "nil");
		}
	#else
		ap_ssid = ezcast_wifi_get_softap_ssid();
		if(ap_ssid != NULL){
			strcat(CGIsetting_priv.result_str, ap_ssid);	
			free(ap_ssid);
			ap_ssid = NULL;
		}else{
			strcat(CGIsetting_priv.result_str, "nil");		
		}
	#endif

	strcat(CGIsetting_priv.result_str, "\t");
	// get password
	#if (MODULE_CONFIG_EZWIRE_ENABLE||LAN_ONLY)
		strcat(CGIsetting_priv.result_str, "nil");		
	#else
		strcat(CGIsetting_priv.result_str, ezcast_wifi_get_softap_psk());
	#endif

	printf("\n-----------------------------------------------\n");
	printf("-----------------------------------------------\n");
	printf("-----------------------------------------------\n");
	printf("%s", CGIsetting_priv.result_str);
	printf("-----------------------------------------------\n");
	strcat(CGIsetting_priv.result_str, "\t");
	CGIsetting_priv.sendline = CGIsetting_priv.result_str;
	return 1;	
}

static int websetting_set_ezair_mode(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_ezairmode")));
	else
		return -1;
	sscanf(setcmd_tmp,"%d",&ret);
	//printf("websetting_set_ezair_mode=%d  \n",ret);
	ezConfigSetAirplayMirrorOnly(ret);
	ezCastSetNumConfig(CONFNAME_EZAIRMODE, ret);
	return 1;
}

static int websetting_get_ezair_mode(char *cmd,char *val)
{
	int ret;
	ret = ezCastGetNumConfig(CONFNAME_EZAIRMODE);
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	//printf("websetting_get_ezair_mode=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_max_num(char *cmd,char *val)
{
	int ret;
	char *tmp=NULL;
	tmp = get_max_connect_mun();
	sprintf(CGIsetting_priv.result_str,"%s",tmp);
	printf("websetting_get_max_num=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_set_max_num(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	int 	RTL_BAND_5G=0x05,RTL_BAND_24G=0x02;
	int len = strlen(cmd)-strlen("set_max_num");
	if(len >= sizeof(setcmd_tmp))
		len = sizeof(setcmd_tmp) - 1;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd, len);
	else
		return -1;
	setcmd_tmp[len] = '\0';
	printf("websetting_set_max_num=[%s]  \n",setcmd_tmp);
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		set_max_connect_mun(setcmd_tmp,RTL_BAND_5G);
		set_max_connect_mun(setcmd_tmp,RTL_BAND_24G);
	#else
		set_max_connect_mun(setcmd_tmp,RTL_BAND_5G);
	#endif
	return 1;
}

#if MODULE_CONFIG_SNMP_ENABLE
static int websetting_set_snmp(char *cmd,char *val)
{
	char setcmd_tmp[256]={0};
	int ret=0;
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("set_snmp")));
	else
		return -1;
	sscanf(setcmd_tmp,"%d",&ret);
	ezCastSetNumConfig(CONFNAME_SNMP,ret);
	printf("websetting_set_snmp=%d  \n",ret);
	if(ret)
	{
		am_snmpv3_enable();
	}
		
	else
	{
		am_snmpv3_disable();
	}
	return 1;
}

static int websetting_get_snmp(char *cmd,char *val)
{
	int ret;
	ret = ezCastGetNumConfig(CONFNAME_SNMP);
	if(ret)
	{
		am_snmpv3_enable();
	}
		
	else
	{
		am_snmpv3_disable();
	}
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_get_snmp=%s  \n",CGIsetting_priv.result_str);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
static int websetting_get_snmp_userinfo(char *cmd,char *val)
{
	int ret;
	FILE *fp;
	char callbuf[100]={0};
	fp=fopen("/mnt/user1/net-snmp/net-snmp.conf","r");
	if(fp==NULL)
	{
		perror("/mnt/user1/net-snmp/net-snmp.confnot exit !\n");
		strncpy(CGIsetting_priv.result_str,"err",strlen("err"));
		//printf(" 1 websetting_get_snmp_username=%s  \n",CGIsetting_priv.result_str);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
		return 1;
	}
	memset(callbuf,0,sizeof(callbuf));
	ret = fread(callbuf, 1, sizeof(callbuf), fp);	
	fclose(fp);
	if(ret > 0)
	{
		strncpy(CGIsetting_priv.result_str,callbuf,strlen(callbuf));
		//printf("websetting_get_snmp_username=%s  \n",CGIsetting_priv.result_str);
		CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	}
	return 1;
}
static int websetting_creat_snmp_user_without_psk(char *cmd,char *val)
{
	int ret;
	char setcmd_tmp[256]={0};
       char *usm_user=NULL;
	char *auth_algorithm=NULL;
	char *auth_password=NULL;
	char *privacy_algorithm=NULL;
	char *privacy_password=NULL;

	am_snmpv3_hash_conf_t   *hash_conf;
	am_snmpv3_encry_conf_t  *encry_conf;
	memset(setcmd_tmp,0,sizeof(setcmd_tmp));
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("creat_snmp_user_without_psk")));
	else
		return -1;
	hash_conf = (am_snmpv3_hash_conf_t *)malloc( sizeof(am_snmpv3_hash_conf_t));
	if(hash_conf != NULL){
		memset(hash_conf,0, sizeof(am_snmpv3_hash_conf_t));
	}
	encry_conf = (am_snmpv3_encry_conf_t *)malloc( sizeof(am_snmpv3_encry_conf_t));
	if(encry_conf != NULL){
		memset(encry_conf,0, sizeof(am_snmpv3_encry_conf_t));
	}
	usm_user=strtok(setcmd_tmp,"\n");
	auth_algorithm=strtok(NULL,"\n");
	if(strcmp(auth_algorithm,"MD5")==0||strstr(auth_algorithm,"MD5")!=NULL)
	{
		  hash_conf->type=AM_SNMPV3_HASH_MD5;
		  strcpy(hash_conf->code," ");	
		//  printf(" 1-----snmpv3_hash_conf.code=%s snmpv3_hash_conf.type=%d  \n",hash_conf->code,hash_conf->type);
	}
	privacy_algorithm=strtok(NULL,"\n");
	if(strcmp(privacy_algorithm,"DES")==0)
	{
                encry_conf->type=AM_SNMPV3_ENCRY_DES;
		  strcpy(encry_conf->code," ");
		  
		//  printf(" 2----snmpv3_encry_conf.code=%s  snmpv3_encry_conf.type=%d\n",encry_conf->code,encry_conf->type);
	}
	
        am_snmpv3_usr_create(usm_user,hash_conf,encry_conf);
	return 1;
}
static int websetting_creat_snmp_user_with_psk(char *cmd,char *val)
{
	int ret;
	char setcmd_tmp[256]={0};
       char *usm_user=NULL;
	char *auth_algorithm=NULL;
	char *auth_password=NULL;
	char *privacy_algorithm=NULL;
	char *privacy_password=NULL;

	am_snmpv3_hash_conf_t   *hash_conf;
	am_snmpv3_encry_conf_t  *encry_conf;
	memset(setcmd_tmp,0,sizeof(setcmd_tmp));
	if(cmd!=NULL)
		strncpy(setcmd_tmp,cmd,(strlen(cmd)-strlen("creat_snmp_user_with_psk")));
	else
		return -1;
	hash_conf = (am_snmpv3_hash_conf_t *)malloc( sizeof(am_snmpv3_hash_conf_t));
	if(hash_conf != NULL){
		memset(hash_conf,0, sizeof(am_snmpv3_hash_conf_t));
	}
	encry_conf = (am_snmpv3_encry_conf_t *)malloc( sizeof(am_snmpv3_encry_conf_t));
	if(encry_conf != NULL){
		memset(encry_conf,0, sizeof(am_snmpv3_encry_conf_t));
	}
	usm_user=strtok(setcmd_tmp,"\n");
	auth_algorithm=strtok(NULL,"\n");
	auth_password=strtok(NULL,"\n");
	if(strcmp(auth_algorithm,"MD5")==0||strstr(auth_algorithm,"MD5")!=NULL)
	{
		  hash_conf->type=AM_SNMPV3_HASH_MD5;
		  strcpy(hash_conf->code,auth_password);	
		  //printf(" 1-----snmpv3_hash_conf.code=%s snmpv3_hash_conf.type=%d  \n",hash_conf->code,hash_conf->type);
	}
	privacy_algorithm=strtok(NULL,"\n");
	privacy_password=strtok(NULL,"\n");
	if(strcmp(privacy_algorithm,"DES")==0)
	{
                encry_conf->type=AM_SNMPV3_ENCRY_DES;
		  if(privacy_password==NULL)
		  {
			 strcpy(encry_conf->code," ");
		  }
		  else
		  {
			 strcpy(encry_conf->code,privacy_password);

		  }
		  //printf(" 2----snmpv3_encry_conf.code=%s  snmpv3_encry_conf.type=%d\n",encry_conf->code,encry_conf->type);
	}
       am_snmpv3_usr_create(usm_user,hash_conf,encry_conf);
	return 1;
}
#endif

static int websetting_read_ddrtype(char *cmd,char *val)
{
	int ret=0,value=0;
	char setcmd_tmp[256]={0};
	#if (AM_CHIP_MINOR == 8258 || AM_CHIP_MINOR == 8268)
       value=RegBitRead(0xb00100b4,6,0);
	printf("val=%d \n",value);
	value=value*8;
	if(value<992)
		ret=2;   //DDR2
	else 
		ret=3;	//DDR3
       #endif
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_read_ddrtype=%s ret=%d \n",CGIsetting_priv.result_str,ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}

static int websetting_ezFuiWifiReset(char *cmd,char *val)
{
       int ret=0,value=0;
       char setcmd_tmp[256]={0};
     printf("------------------[%s][%d] ezFuiWifiReset:\n",__func__,__LINE__);
        //ezFuiWifiReset();
       return 1;
}

static int websetting_get_mirascreen5g_flag(char *cmd,char *val)
{
	int ret=0;
	#if (defined(MODULE_CONFIG_MIRASCREEN5G_ENABLE) && MODULE_CONFIG_MIRASCREEN5G_ENABLE!=0)
	ret=1;
	#endif
	sprintf(CGIsetting_priv.result_str,"%d",ret);
	printf("websetting_get_mirascreen5g_flag=%s ret=%d \n",CGIsetting_priv.result_str,ret);
	CGIsetting_priv.sendline=CGIsetting_priv.result_str;
	return 1;
}
const struct websetting_funLink_s websetting_funLink[] = 
{
		{"get_softap_psk", websetting_get_softap_psk},
		{"set_softap_psk", websetting_set_softap_psk},
		{"set_softap_ssid", websetting_set_softap_ssid},
		{"set_psk_ignoressid", websetting_set_softap_psk_ignoressid},
		{"get_ignoressid", websetting_get_ignoressid},
		{"set_ignoressid", websetting_set_ignoressid},
		{"delete_password", websetting_delete_password},
		{"set_language", websetting_set_language},
		{"set_wifi", websetting_set_wifi},
		{"set_hotspot_ap", websetting_set_hotspot_ap},
		{"add_network", websetting_add_network},
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
		{"add_802XEPA_network", websetting_add_network_set_802XEPA_conf},
		#endif
		{"set_resolution", websetting_set_resolution},
		{"set_default_mode", websetting_set_default_mode},
		{"get_multilanguage", websetting_get_language},
		{"get_5G_country", websetting_get_5G_country},
		{"get_apindex_string", websetting_ap_index_string},
		{"get_lan_index", websetting_get_lan_index},
		{"wifi_start", websetting_wifi_start},
		{"get_wifi_mode", websetting_get_wifi_mode},
		{"get_last_ui", websetting_get_last_ui},
		{"get_mac_ip", websetting_get_mac_ip},
		#if MODULE_CONFIG_LAN_ONLY
		{"get_lan_ip", websetting_get_lan_ip},
		#endif
		{"get_connected_ssid", websetting_get_connected_ssid},
		{"get_OTA_version", websetting_get_OTA_version},
		{"get_OTA_conf", websetting_get_OTA_conf},
		{"get_index_text", websetting_get_index_text},
		//for 8268
		#if defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE > 29 && MODULE_CONFIG_CHIP_TYPE < 40)
		{"get_webmultilanguage_text", websetting_get_webmultilanguage_text},
		#endif
		{"get_wifi_list_text", websetting_get_wifi_list_text},
		{"get_defaultmodetext", websetting_get_default_modetext},
		{"get_lan_text", websetting_get_lan_text},
		{"get_wifi_text", websetting_get_wifi_text},
		{"get_wifilist_text", websetting_get_wifilist_text},
		{"get_addnet_text", websetting_get_addnet_text},
		{"get_connectfail_text", websetting_get_connectfail_text},
		{"get_OTA_text", websetting_get_OTA_text},
		{"get_OTA_warn1_text", websetting_get_OTA_warn1_text},
		{"get_OTA_warn2_text", websetting_get_OTA_warn2_text},
		{"get_OTA_faild_connect", websetting_get_OTA_faild_connect},
		{"get_res_text", websetting_get_res_text},
		{"get_new_ver_text", websetting_get_new_version_text},
		{"get_down_bin", websetting_get_down_bin},
		{"get_connections", websetting_get_connections_num},
		{"get_scan_result", websetting_get_scan_result},
		{"get_ssid_signal", websetting_get_ssid_signal},
		{"get_ap_ssid", websetting_get_ap_ssid},
		{"check_wifi", websetting_check_wifi},
		//{"check_connect_status", websetting_check_auto_connect},
		{"check_connect_status", websetting_check_wifi},
		{"get_resolution", websetting_get_resolution},
		{"get_ap_name", websetting_get_ap_name},
		{"wifi_scan", websetting_wifi_start_scan},
		{"enter_check_ota_boot", websetting_enter_check_ota_boot},
		{"get_ota_check_status", websetting_get_ota_check_status},
		{"Check_new_version_o", websetting_Check_new_version_connect_wifi_first},
		{"get_frequency", websetting_get_cpu_frequency},
		{"set_music_output", websetting_set_music_output},
		{"get_music_output", websetting_get_music_output},
		{"set_EQ_default", websetting_set_EQ_default},
		{"get_EQ_default", websetting_get_EQ_default},
		{"set_EQ_Custom", websetting_set_custom_value},
		{"get_EQ_Custom", websetting_get_custom_value},
		{"get_en_music_output", websetting_get_music_output_en},
		{"get_router_ctl", websetting_get_router_ctl},
		{"set_router_ctl", websetting_set_router_ctl},
		{"get_wlan_ipaddress", websetting_get_ipaddress},
		{"get_wlan0_ip", websetting_get_wlan0_ip},
		{"get_ota_status", websetting_get_ota_status},
		{"get_eq_text", websetting_get_eq_text},
		{"get_password_text", websetting_get_password_text},
		{"set_devicename_reboot", websetting_set_devicename_reboot},
		{"set_device_name", websetting_set_device_name},
		{"get_device_name", websetting_get_device_name},
		{"setAutoplayEnable", websetting_setAutoplayEnable},
		{"getAutoplayEnable", websetting_getAutoplayEnable},
		{"getAutoplayVolume", websetting_getAutoplayVolume},
		{"getAutoplayWaitime", websetting_getAutoplayWaitime},
		{"getAutoplaylist", websetting_getAutoplaylist},
		{"getssid", websetting_getPROGRESSIVE},
		{"ota_from", websetting_otaFrom},
		{"modify_usr_password", websetting_set_usr_password},
		{"login_user_password", websetting_login_user_password},
		{"get_user_name", websetting_get_user_name},
		{"get_user_psk", websetting_get_user_psk},
		{"get_supper_psk", websetting_get_super_psk},
		{"compare_user_psk", websetting_compare_user_psk},
		{"compare_psk_modfiy", websetting_compare_psk_modfiy},
		{"set_encrypted_psk", websetting_save_encrypted_psk},
		{"set_airview", websetting_set_air_view},
		{"get_airview", websetting_get_air_view},
		{"set_hdmicec", websetting_set_hdmi_cec},
		{"get_hdmicec", websetting_get_hdmi_cec},
		{"reset_default", websetting_reset_default},
		{"reboot", websetting_reboot_system},
		{"set_routeronly", websetting_set_routeronly},
		{"get_routeronly", websetting_get_routeronly},
	//	{"set_conferent_host", websetting_set_conferent_host},
	//	{"set_host_airview", websetting_set_airview_host},
	//	{"set_airopt_host", websetting_set_airsetting_host},
	//	{"get_conferent_host", websetting_get_conferent_host},
	//	{"get_host_airview", websetting_get_airview_host},
	//	{"get_airopt_host", websetting_get_airsetting_host},
		{"CONFNAME_WR", websetting_confname_wr},
	//	{"get_psk_visibility", websetting_get_psk_visibility_host},
		{"get_otaserver_path",websetting_get_otaserver_path},
		{"set_otaserver_path",websetting_set_otaserver_path},
		{"get_internet_access_control", websetting_get_internet_access_control},
		{"set_internet_access_control", websetting_set_internet_access_control},
		{"get_pass_code_val", websetting_get_passcode_val},
		{"get_passcode", websetting_get_passcode_onoff},
		{"set_passcode", websetting_set_passcode_onoff},
		{"get_miracode", websetting_get_miracode_onoff},//by cxf
		{"set_miracode", websetting_set_miracode_onoff},
		{"set_channel", websetting_set_channel},
		{"get_softap_channel", websetting_get_softap_channel},
	//	{"get_softap_5gchannel",websetting_get_softap_5G_channel},
		{"set_5G_country",websetting_set_softap_5G_country},
		{"get_5G_initvalue",websetting_get_softap_5G_country},
		{"get_config_val", websetting_get_config_val},
		{"get_ProboxConnectStatus", websetting_get_ProboxConnectStatus},
		{"get_auto_channel_val",websetting_get_auto_channel_val},
		{"get_main_text", websetting_get_main_text},//too long
		{"get_menu_text", websetting_get_menu_text},

		{"get_view_text", websetting_get_airview_text},
		{"get_paswod_text", websetting_get_paswod_text},
		{"get_confer_text", websetting_get_conference_text},
		{"get_filuplod_text", websetting_get_fileupload_text},
		{"get_authority_host_text", websetting_get_authority_host_text},
		{"get_control_access_text", websetting_get_control_access_text},
		{"get_serverota_text", websetting_get_serverota_text},
		{"get_rebret_text", websetting_get_rebot_rest_text},
		{"gtmulilan", websetting_get_lan_multstring},
		{"gtroutwarntxt", websetting_get_route_warn_string},
		{"do_command", websetting_do_cmd},
		{"get_ezairmode", websetting_get_ezair_mode},
		{"set_ezairmode", websetting_set_ezair_mode},  
		{"get_max_num", websetting_get_max_num},
		{"set_max_num", websetting_set_max_num},  
		#if MODULE_CONFIG_SNMP_ENABLE
		{"set_snmp", websetting_set_snmp},
		{"get_snmp", websetting_get_snmp},
		{"get_userinfo_snmp", websetting_get_snmp_userinfo},
		{"creat_snmp_user_with_psk", websetting_creat_snmp_user_with_psk},
		{"creat_snmp_user_without_psk", websetting_creat_snmp_user_without_psk},
		{"get_connect_num", websetting_get_connections_num},
		 #endif
		{"setWiFiAuto_off",websetting_wifi_setWiFiAutomatic_off},
		{"setWiFiAutomatic",websetting_wifi_setWiFiAutomatic},
		{"getWiFiAutomatic",websetting_wifi_getWiFiAutomatic},
		{"writeWiFiManualInfo",websetting_wifi_storeWiFiManualInfo},
		{"getWiFiSetting_Val",websetting_wifi_getWiFiSetting_Val},
		{"getWiFi_auto_ip",websetting_wifi_getWiFi_auto_ip},
		#if EZWILAN_ENABLE
		//add for snmp
		{"getWebSetting_wifi_ip",websetting_wifi_getip},
		{"setwebsetting_wifi_ip",websetting_wifi_setip},
		{"getwebSetting_lan_ip",websetting_lan_getip},
		{"setwebSetting_lan_ip",websetting_lan_setip},
        //end
       	#endif
		{"read_ddrtype", websetting_read_ddrtype},
        {"ezFuiWifiReset", websetting_ezFuiWifiReset},
		#if MODULE_CONFIG_EZWIRE_ENABLE
		{"get_netlink", websetting_get_netlink_status},
		{"set_ezwireaudio", websetting_set_ezwire_audio_status},
		{"get_ezwireaudio", websetting_get_ezwire_audio_status},
		{"get_ezwir_text", websetting_get_ezwire_text},
		#endif
		#if EZWILAN_ENABLE
		{"get_netlink", websetting_get_netlink_status},
		{"getLanAutomatic", websetting_lan_getLanAutomatic},
		{"setLanAutomatic", websetting_lan_setLanAutomatic},
		{"setLanAuto_off", websetting_lan_setLanAutomatic_off},
		{"getLanSettingVal", websetting_lan_getLanSetting_Val},
		{"getLanSettingautoip", websetting_lan_getLan_auto_ip},
		{"writeLanManualInfo", websetting_lan_storeLanManualInfo},
		{"getLan_mac_ip", websetting_lan_get_mac_ip},
		
		#endif
		#if EZMUSIC_ENABLE || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		{"get_ezchannel_playway", websetting_get_ezchannel_playway},
		{"set_ezchannel_playway", websetting_set_ezchannel_playway},
		#endif
		{"read_total_resource", websetting_get_dev_management_res},
		{"read_ddrtype", websetting_read_ddrtype},
		{"get_mirascreen5g_flag", websetting_get_mirascreen5g_flag}
		
		

};

static int discriminate_and_execute_cmd(char *cmd)
{
	int i,ret;
	memset(CGIsetting_priv.result_str,0,sizeof(CGIsetting_priv.result_str));
	CGIsetting_priv.sendline=NULL;
	//printf("cmd============%s,%d\n",cmd,__LINE__);
	if(cmd == NULL)
	{
		printf("Argument error!!!\n");
		return -1;
	}
	for(i=0; i<(sizeof(websetting_funLink)/sizeof(struct websetting_funLink_s)); i++)
	{
		if((!strcmp(cmd, websetting_funLink[i].name))||(strstr(cmd, websetting_funLink[i].name)))
		{
			ret=websetting_funLink[i].func(cmd,CGIsetting_priv.result_str);
			return ret;
		}
	}
	return -1;	
}

static void Free_resources(void)
{
	if(SysCGI_priv.ssid_string_for_cgi)
	{
		free(SysCGI_priv.ssid_string_for_cgi);
		SysCGI_priv.ssid_string_for_cgi=NULL;
	}
	if(SysCGI_priv.ssid_signal_string)
	{
		free(SysCGI_priv.ssid_signal_string);
		SysCGI_priv.ssid_signal_string=NULL;
	}
	if(SysCGI_priv.ssid_index_string)
	{
		free(SysCGI_priv.ssid_index_string);
		SysCGI_priv.ssid_index_string=NULL;
	}
	if(CGIsetting_priv.ota_upgrade_string)
	{
		SWF_Free(CGIsetting_priv.ota_upgrade_string);
		CGIsetting_priv.ota_upgrade_string=NULL;
	}
	if(CGIsetting_priv.language_string)
	{
		free(CGIsetting_priv.language_string);
		CGIsetting_priv.language_string=NULL;
	}
	if(CGIsetting_priv.sendline)
	{
		SWF_Free(CGIsetting_priv.sendline);
		CGIsetting_priv.sendline=NULL;		
	}
	#if EZMUSIC_ENABLE
	ezmusic_am7x_dlna_stop_work();
	#endif
	/*system("lsmod -l");
	system("killall -9 *.cgi");	

	system("killall -9 udhcpd");
	system("killall -9 hostapd");
	system("ps");*/
}
static void *receive_cgimsg_loop(void *arg)
{
	int ret_value;
	int socket_back_cli_fd;
	socklen_t csize;
	fd_set rfd_set,allset;
	struct sockaddr_un un1;
	int receivesize;
	char receiveline[512];
	int i,maxi,maxfd,sockfd;
	int nready,client[5];
	struct timeval timeout;
	maxfd=CGIsetting_priv.socket_ser_fd;
	maxi=-1;
	for(i=0;i<5;i++)
		client[i]=-1;
	FD_ZERO(&allset);
	FD_SET(CGIsetting_priv.socket_ser_fd, &allset);	
	while(!CGIsetting_priv.exit)
	{
		rfd_set=allset;
		/** timeout each second for get the chance to check loop_exit */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		nready = select(maxfd+1,&rfd_set,NULL,NULL,&timeout);
		if(nready==0)//time out
			continue;
		if(nready>0&&FD_ISSET(CGIsetting_priv.socket_ser_fd,&rfd_set))
		{
			csize = sizeof(un1);
			socket_back_cli_fd = accept(CGIsetting_priv.socket_ser_fd, (struct sockaddr *)&un1, &csize);
			for(i=0;i<5;i++)
			{
				if(client[i]<0)
				{
					client[i]=socket_back_cli_fd;
					break;
				}				
			}
			if(i==5)
			{
				printf("too many clients\n");
				break;
			}
			FD_SET(socket_back_cli_fd,&allset);
			if(socket_back_cli_fd>maxfd)
				maxfd=socket_back_cli_fd;
			if(i>maxi)
				maxi=i;
			if(--nready<=0)
				continue;
		}		
		for(i=0;i<=maxi;i++)	
		{
			if((sockfd=client[i])<0)
				continue;
			if(FD_ISSET(sockfd,&rfd_set))
			{	
				memset(receiveline,0,sizeof(receiveline));
				if((receivesize = read(sockfd,receiveline,sizeof(receiveline)))<=0)
				{
					close(sockfd);
					FD_CLR(sockfd,&allset);
					client[i]=-1;
				}
				else
				{

					if(strlen(receiveline))
					{
						if(strstr(receiveline, "set") == 0 )//get string
						{
							ret_value=discriminate_and_execute_cmd(receiveline);
							if((!ret_value)||(CGIsetting_priv.sendline==NULL))
							{
								close(sockfd);
								FD_CLR(sockfd,&allset);
								client[i]=-1;							
								continue;
							}
							write(sockfd,CGIsetting_priv.sendline,strlen(CGIsetting_priv.sendline));
							if(sockfd>0)
							{
								close(sockfd);
								FD_CLR(sockfd,&allset);
								client[i]=-1;	
								sockfd=-1;
							}
						}
						else//set
						{
							ret_value=discriminate_and_execute_cmd(receiveline);
							//if((ret_value)||(CGIsetting_priv.sendline==NULL))
							//{
								close(sockfd);
								FD_CLR(sockfd,&allset);
								client[i]=-1;	
								//continue;
							//}							
						}						
					}
				}
					
				if(--nready<=0)
					break;
			}
		}
	}
	printf("pthread_exit start!!!!!!!!!!!!!!!!!!!!!%s,%d\n",__FUNCTION__,__LINE__);
	Free_resources();
	pthread_exit(NULL);
	return NULL;
}
void init_Variable(void)
{
	int i=0;
	CGIsetting_priv.sendline=NULL;
	CGIsetting_priv.language_string=NULL;
	CGIsetting_priv.ota_upgrade_string=NULL;
	CGIsetting_priv.up_progress_text=NULL;
	CGIsetting_priv.exit=0;
	SysCGI_priv.ssid_string_for_cgi=NULL;
	SysCGI_priv.ssid_signal_string=NULL;
	SysCGI_priv.ssid_index_string=NULL;
	SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag=0;
	SysCGI_priv.CONNECTIONS_OK_LED_twinkle_tick=0;
	SysCGI_priv.WIFI_ON_LED_flag=0;
	SysCGI_priv.WIFI_CLIENT_OK_LED_flag=0;
	SysCGI_priv.WIFI_CLIENT_OK_LED_twinkle_tick=0;
	CGIsetting_priv.check_OTA_flag_set_wifi=1;
	CGIsetting_priv.set_resolution_timeout_tick=0;
	CGIsetting_priv.AUDIO_EN=1;
	SysCGI_priv.play_warning_tone_flag=0;
	SysCGI_priv.connect_router_waiting_lan_flag=0;
	SysCGI_priv.connect_router_waiting_flag=0;
	SysCGI_priv.connect_router_success_flag=0;
	SysCGI_priv.connect_router_failed_flag=0;
	SysCGI_priv.connected_router_useful_flag=0;
	SysCGI_priv.not_connected_router_useful_flag=0;
	SysCGI_priv.play_connect_router_waiting_start=0;
	SysCGI_priv.play_connect_router_waiting_end=0;
	SysCGI_priv.play_otaupgrade_waiting_start=0;
	SysCGI_priv.play_otaupgrade_waiting_end=0;
	SysCGI_priv.otaupgrade_successful_flag=0;
	SysCGI_priv.otaupgrade_failed_flag=0;
	SysCGI_priv.otaupgrade_waiting_flag=0;
	SysCGI_priv.otaupgrade_download_finished=0;
	SysCGI_priv.otaupgrade_waiting_lan_flag=0;
	SysCGI_priv.otaupgrade_waiting_lan_once=0;
	SysCGI_priv.connected_ssid_once=0;
	SysCGI_priv.router_disconnect_flag=0;
	SysCGI_priv.new_firmware_version_flag=0;
	SysCGI_priv.Bye_bye_flag=0;
	SysCGI_priv.mainswf_skip_get_keycode_flag=0;
	SysCGI_priv.start_ota_upgrade_flag=0;
	SysCGI_priv.start_audio_flag=0;
	SysCGI_priv.start_audio_once=0; 
	SysCGI_priv.language_index=0;
	SysCGI_priv.otaupgrade_failed_and_stop=0;
	SysCGI_priv.enter_factory_test_video=0;
	SysCGI_priv.stream_play_status=0;
	CGIsetting_priv.play_warning_tone_thread_exit=0;
	if(!CGIsetting_priv.language_string)
	{
		CGIsetting_priv.language_string=(char *)malloc(1000*sizeof(char));
		if(!CGIsetting_priv.language_string)
		{
			printf("malloc for CGIsetting_priv.language_string error!\n");
			exit(EXIT_FAILURE);
		}
	}
	memset(CGIsetting_priv.language_string,0,1000*sizeof(char));
	if(!CGIsetting_priv.g5country_string)
	{
		CGIsetting_priv.g5country_string=(char *)malloc(1000*sizeof(char));
		if(!CGIsetting_priv.g5country_string)
		{
			printf("malloc for CGIsetting_priv.language_string error!\n");
			exit(EXIT_FAILURE);
		}
	}
	memset(CGIsetting_priv.g5country_string,0,1000*sizeof(char));

	for(i=0;i<14;i++)
		CGIsetting_priv.warning_tone_p[i]=NULL;
	#if EZMUSIC_ENABLE
	CGIsetting_priv.warning_tone_p[0]=WarningTone_1_Default;
	CGIsetting_priv.warning_tone_p[1]=WarningTone_2_Default;
	CGIsetting_priv.warning_tone_p[2]=WarningTone_3_Default;
	CGIsetting_priv.warning_tone_p[3]=WarningTone_4_Default;
	CGIsetting_priv.warning_tone_p[4]=WarningTone_5_Default;
	CGIsetting_priv.warning_tone_p[5]=WarningTone_6_Default;
	CGIsetting_priv.warning_tone_p[6]=WarningTone_7_Default;
	CGIsetting_priv.warning_tone_p[7]=WarningTone_8_Default;
	CGIsetting_priv.warning_tone_p[8]=WarningTone_9_Default;
	CGIsetting_priv.warning_tone_p[9]=WarningTone_10_Default;
	CGIsetting_priv.warning_tone_p[10]=WarningTone_11_Default;
	CGIsetting_priv.warning_tone_p[11]=WarningTone_12_Default;
	CGIsetting_priv.warning_tone_p[12]=WarningTone_13_Default;
	CGIsetting_priv.warning_tone_p[13]=WarningTone_14_Default;
	#endif
}
#if EZMUSIC_ENABLE
int ezAudioOutGet(void)
{
	int index = -1;
	char path[] = "/mnt/vram/EZmusic/music_output.conf";
	FILE *file = fopen(path, "r");
	if (file != NULL)
	{
		char l[1000];	
		if ( fgets(l, sizeof(l), file) != NULL && fgets(l, sizeof(l), file) != NULL )
		{
			sscanf(l, "%d", &index);
		}
		fclose(file);
	}
	else
		index=0;
	printf("------------------>AudioOutGet  = %d<-------------------\r\n",index);
	return (index);
}
void set_music_output_led(void)
{
	switch(ezAudioOutGet())
	{
		/*case 0:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,0);
			#endif
			
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
			#endif
			
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
			#endif
			
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,1);
			#endif				
			break;*/
		case 0:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,1);
			#endif
			
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,0);
			#endif
			
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
			#endif
			
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,1);
			#endif	
			break;
		case 1:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,1);
			#endif
			
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
			#endif
			
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,0);
			#endif
			
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,1);
			#endif	
			break;
		case 2:
			#if MODULE_CONFIG_AUDIO_LED
			if(CGIsetting_priv.AUDIO_EN)
				set_gpio(MODULE_CONFIG_AUDIO_LED,1);
			#endif
			
			#if MODULE_CONFIG_SPDIF2_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF2_1_LED,1);
			#endif
			
			#if MODULE_CONFIG_SPDIF5_1_LED
			if(CGIsetting_priv.SPDIF_EN)				
				set_gpio(MODULE_CONFIG_SPDIF5_1_LED,1);
			#endif
			
			#if MODULE_CONFIG_I2S_LED
			if(CGIsetting_priv.I2S_EN)				
				set_gpio(MODULE_CONFIG_I2S_LED,0);
			#endif	
			break;
	}
}
static int Create_default_music_output_conf(void)
{
	FILE *fp=NULL;
	char callbuf[100]={0};
	char tmp[10]={0};
	int music_output_mode_index=0;
	if(access("/mnt/vram/EZmusic/music_output.conf",F_OK)==-1)
	{
		if(access("/mnt/vram/EZmusic",F_OK)==-1)
		{
			sprintf(callbuf,"mkdir /mnt/vram/EZmusic/");
			system(callbuf);		
			memset(callbuf,0,sizeof(callbuf));
			sprintf(callbuf,"touch /mnt/vram/EZmusic/music_output.conf");
			system(callbuf);
			memset(callbuf,0,sizeof(callbuf));
			fp=fopen("/mnt/vram/EZmusic/music_output.conf","rw+");
			if(fp==NULL)
			{
				perror("/mnt/vram/EZmusic/music_output.conf not exit !\n");
				return -1;
			}			
			strncpy(callbuf,"music_output_mode_index\n",strlen("music_output_mode_index\n"));
			if((1==CGIsetting_priv.I2S_EN)&&(0==CGIsetting_priv.SPDIF_EN))
				music_output_mode_index=2;
			else if((0==CGIsetting_priv.I2S_EN)&&(1==CGIsetting_priv.SPDIF_EN))
				music_output_mode_index=0;
			else
				music_output_mode_index=0;
			sprintf(tmp,"%d",music_output_mode_index);
			strcat(callbuf,tmp);
			strcat(callbuf,"\n");
			fclose(fp);
			fp=fopen("/mnt/vram/EZmusic/music_output.conf","w+");
			if(fp==NULL)
			{
				perror("/mnt/vram/EZmusic/music_output.conf not exit !\n");
				return -1;
			}
			fwrite(callbuf,1,strlen(callbuf),fp);
			fflush(fp);
			int fd_write = fileno(fp);
			fsync(fd_write);
			fclose(fp);					
		}	
	}	
	return 1;
}
static void check_music_ouput_en(void)
{
	char val = -1;
	#if MODULE_CONFIG_AUDIO_EN
	get_gpio(MODULE_CONFIG_AUDIO_EN, &val);
	printf("val============%d,%s,%d\n",val,__FUNCTION__,__LINE__);
	if(0==val)
		CGIsetting_priv.AUDIO_EN=1;
	else
		CGIsetting_priv.AUDIO_EN=0;
	#endif
	#if MODULE_CONFIG_SPDIF_EN
	get_gpio(MODULE_CONFIG_SPDIF_EN, &val);
	printf("val============%d,%s,%d\n",val,__FUNCTION__,__LINE__);
	if(0==val)
		CGIsetting_priv.SPDIF_EN=1;
	else
		CGIsetting_priv.SPDIF_EN=0;
	#endif
	
	#if MODULE_CONFIG_I2S_EN
	get_gpio(MODULE_CONFIG_I2S_EN, &val);
	printf("val============%d,%s,%d\n",val,__FUNCTION__,__LINE__);
	if(0==val)
		CGIsetting_priv.I2S_EN=1;
	else
		CGIsetting_priv.I2S_EN=0;
	#endif	
	Create_default_music_output_conf();
}
#endif
void websettingSocketUnix(void)
{
	struct sockaddr_un uns;
	int size;
	uns.sun_family = AF_UNIX;
    	strcpy(uns.sun_path, "/tmp/cgi_setting_server");
	if ((CGIsetting_priv.socket_ser_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		printf("[%s]: socket create error\n",__FUNCTION__);
		exit(EXIT_FAILURE);
	}
	/** before bind, must make sure the path file not exist */
	unlink(uns.sun_path);
	size = offsetof(struct sockaddr_un, sun_path) + strlen(uns.sun_path);
	if (bind(CGIsetting_priv.socket_ser_fd, (struct sockaddr *)&uns, size) < 0)
	{
		printf("[%s]: bind error\n",__FUNCTION__);
		close(CGIsetting_priv.socket_ser_fd);
		CGIsetting_priv.socket_ser_fd = -1;
		exit(EXIT_FAILURE);
	}
	if (listen(CGIsetting_priv.socket_ser_fd, 5) < 0)
	{
		printf("[%s]: listen error\n",__FUNCTION__);
		close(CGIsetting_priv.socket_ser_fd);
		CGIsetting_priv.socket_ser_fd = -1;
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&CGIsetting_priv.looper, NULL, receive_cgimsg_loop, NULL) != 0)
	{
		printf("[%s]: recv send cgimsg  loop create error\n",__FUNCTION__);
		close(CGIsetting_priv.socket_ser_fd);
		CGIsetting_priv.socket_ser_fd = -1;
		exit(EXIT_FAILURE);
	}	
}
#if EZMUSIC_ENABLE
void create_dms_server(void)
{	
	ezmusic_am7x_dlna_start_work();
	sleep(5);
	ezmusic_am7x_dlna_start_DMS();	
}
#endif
void create_websetting_server(void)
{
	if(!websettingInitOK)
	{
		websettingInitOK=1;
		init_Variable();
		websettingSocketUnix();
		#if EZMUSIC_ENABLE
		check_music_ouput_en();
		set_music_output_led();
		websetting_get_lan_index(NULL,NULL);
		check_multilanguage_for_warning_tone(SysCGI_priv.language_index);
		Create_play_warning_tone_thread();
		//create_dms_server();
		#endif		
#if defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE > 29 && MODULE_CONFIG_CHIP_TYPE < 40)
		int index=ezcast_sysinfo_get_local_language();
		ezcast_set_language_swf_name((char *)lan_value_item[index],"ezcast");
#endif
	}
}
#endif
