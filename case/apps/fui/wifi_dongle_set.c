#include "wifi_engine.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <media_hotplug.h>
#include "apps_vram.h"
#include "ezcast_public.h"

typedef int (*dongle_handle_func)(int dongle_dir,void*para);
typedef struct dongle_s
{
	unsigned int vidpid;		///usb dongle vid pid, the highest 16 bit is vid, and the lower two bytes are pid
	char *device_name;		///the device name ,such as the realtek dongle is wlan0 and the ralink is ra0
	char *dev_name;
	char *driver_name;	///the name of driver ,the realtek dongle is 8192cu.ko and the ralink is rt5370sta.ko 
	char *dev_rmmod;	//rmmod需要移除的设备
	dongle_handle_func dongle_func;	//dongle插入或是拔出时，装载或是卸载了ko之后会被调用到的函数
}dongle_t;

#define DONGLE_OK 	1
#define DONGLE_ERR 	0

#define DONGLE_IN 	1
#define DONGLE_OUT 	2

#define CONNECT_TYPE_STA 		1
#define CONNECT_TYPE_SOFTAP	2

int bestChannelChosen;		//added for p2p host by amos

int dongle_connect_type =-1;
int dongle_softap_mode_first=0;
int mode_change=0;
int dongle_out_ok=0;
int dongle_in_ok=0;
unsigned int vid_pid_from_driver=0;
unsigned int vid_pid_from_driver_firstin=0;

#define SSID_LENGTH	81
#define KEY_LENGTH	64
#define BUF_LENGTH	128
#define AM_CASE_SC_DIR         	"/am7x/case/scripts"
#define AM_SYS_LIB_DIR 		"/lib/modules/2.6.27.29"

static int softap_dongle_type=-1;

static int securityChoice = 2;
static char second_key[KEY_LENGTH];
extern int wifi_client_thread_is_running;
extern current_status wlan_current_status;
extern int id_hcd_ko;  //bq
static char *ssid;
char *channel;
extern int wifiChannel_mode;
extern int wifiChannel_set;
static char *first_key ;
static int realtek_last_mode=2;
wifi_dongle_type_emun dongle_type_divided=INIT_DONGLE_TYPE;
realtek_wifi_dongle_type_emun realtek_dongle_type_divided=REALTEK_CU;
extern char selected_filename[WIFI_BUF_LEN];
//int cur_port=0;  //flag for loading 8189es
int channel_p2p=0;



int wifi_off=0;

char key1[50];
char key2[50];
char key3[50];
char key4=0x0a;

int hcd_mannual=0;
char device_name[50];


struct mode_data_s
{
	unsigned int dongle_pidvid;
	int connect_type;
};
int switch_mode_active=0;
struct mode_data_s mode_data;

static int ralink_softap_func(int dongle_dir,void *data);
static int realtek_sta_func(int dongle_dir,void *data);
static int ralink_sta_func(int dongle_dir,void *data);

extern int wifi_remote_control_started;
extern int wifi_check_remote_control_thread();
struct dongle_info_s
{
	int status;
	sem_t lock;
	dongle_t *dongle;
};

struct dongle_info_s cur_dongle;
struct  dongle_info_s  cur_dongle_temp;
//int dongle_tbl_length=13;
int wlan0_length=6;
dongle_t dongle_tbl[]=
{
	{0x0bda8176,"wlan0","wlan1","8192cu.ko","8192cu",NULL}, 	///realtek dongle
	{0x0bda8178,"wlan0","wlan1","8192cu.ko","8192cu",NULL}, 	///realtek dongle8192
	{0x0bda018a,"wlan0","wlan1","8192cu.ko","8192cu",NULL},	///kingnet dongle
	{0x0ace1215,"wlan0","wlan1","8192cu.ko","8192cu",NULL},	///atheros dongle
	{0x48550090,"wlan0","wlan1","8192cu.ko","8192cu",NULL}, 
	{0x0bda8194,"wlan0","wlan1","8192du.ko","8192du",NULL}, 	///realtek dongle8194
	{0x0bda8193,"wlan0","wlan1","8192du_vc.ko","8192du",NULL}, 
	{0x0bda8179,"wlan0","wlan1","8188eu.ko","8188eu",NULL}, 	///realtek 8188eu dongle
	{0x0bda0179,"wlan0","wlan1","8188eu.ko","8188eu",NULL}, 	//add realtek 8188etv dongle
	{0x20013308,"wlan0","wlan1","8192cu.ko","8192cu",NULL}, 	
	{0x73927811,"wlan0","wlan1","8192cu.ko","8192cu",NULL}, 	
	{0x0bda818b,"wlan0","wlan1","8192eu.ko","8192eu",NULL}, 	
	{0x0bda818c,"wlan0","wlan1","8192eu.ko","8192eu",NULL}, 
	{0x0bda0811,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x0bda8822,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x0bdaa811,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x0bda0820,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x0bda0821,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x0bda0823,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x20f4804b,"wlan0","wlan1","8821au.ko","8821au",NULL},
	{0x0bdab82b,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdab820,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdac821,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdac820,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdac82a,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdac82b,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdac811,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bda8811,"wlan0","wlan1","8821cu.ko","8821cu",NULL},
	{0x0bdaf179,"wlan0","wlan1","8188fu.ko","8188fu",NULL},
	{0x07b83071,"ra0","","rt5370sta.ko","rt5370sta",NULL},
	{0x14b23c2b,"ra0","","rt5370sta.ko","rt5370sta",NULL},//NEC dongle
	{0x148f5370,"ra0","","rt5370sta.ko","rt5370sta",NULL},
	{0x148f3072,"ra0","","rt5370sta.ko","rt5370sta",NULL},
	{0x07B83070,"ra0","","rt5370sta.ko","rt5370sta",NULL},
	{0x148f3070,"ra0","","rt5370sta.ko","rt5370sta",NULL},
    	{0x148f3370,"ra0","","rt5370sta.ko","rt5370sta",NULL},//148f3370
	{0x148f8189,"wlan2","wlan3","8189es.ko","8189es",NULL},

};

dongle_t dongle_softap_tbl[]=		//如果dongle当softap使用，则填到下表，ko的顺序为装载的顺序
{
	{0x07b83071,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},
	{0x14b23c2b,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},
	{0x148f5370,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},
	{0x148f3072,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},
	{0x148f3070,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},
	{0x07B83070,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},
	{0x148f3370,"ra0","","rtutil3070ap.ko rt3070ap.ko rtnet3070ap.ko","rtnet3070ap rt3070ap rtutil3070ap",ralink_softap_func},//148f3370
	
	{0x48550090,"wlan0","wlan1","8192cu.ko","8192cu",realtek_softap_func}, 
	{0x0bda8176,"wlan0","wlan1","8192cu.ko","8192cu",realtek_softap_func}, 
	{0x0bda8178,"wlan0","wlan1","8192cu.ko","8192cu",realtek_softap_func}, 
	{0x0bda018a,"wlan0","wlan1","8192cu.ko","8192cu",realtek_softap_func},	///kingnet dongle
	{0x0bda8194,"wlan0","wlan1","8192du.ko","8192du",realtek_softap_func}, 	///realtek dongle8194
	{0x0bda8179,"wlan0","wlan1","8188eu.ko","8188eu",realtek_softap_func}, 	///realtek dongle8179
	{0x0bda0179,"wlan0","wlan1","8188eu.ko","8188eu",realtek_softap_func}, 	///realtek dongle8179
	{0x20013308,"wlan0","wlan1","8192cu.ko","8192cu",realtek_softap_func}, 	
	{0x73927811,"wlan0","wlan1","8192cu.ko","8192cu",realtek_softap_func},
	{0x0bda8193,"wlan0","wlan1","8192du_vc.ko","8192du",realtek_softap_func}, 
	{0x0bda818b,"wlan0","wlan1","8192eu.ko","8192eu",realtek_softap_func}, 	///realtek dongle8179
	{0x0bda818c,"wlan0","wlan1","8192eu.ko","8192eu",realtek_softap_func},
	{0x0bda0811,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x0bda8822,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x0bdaa811,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x0bda0820,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x0bda0821,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x0bda0823,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x20f4804b,"wlan0","wlan1","8821au.ko","8821au",realtek_softap_func},
	{0x0bdab82b,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bdab820,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bdac821,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bdac820,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bdac82a,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bdac82b,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bdac811,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x0bda8811,"wlan0","wlan1","8821cu.ko","8821cu",realtek_softap_func},
	{0x148f8189,"wlan2","wlan3","8189es.ko","8189es",realtek_softap_func},
	{0x0bdaf179,"wlan0","wlan1","8188fu.ko","8188fu",realtek_softap_func},
};

channel_region_t channel_region_tbl[]={
		{"CHINA",0x0C,13,5,{1,2,3,4,5,6,7,8,9,10,11,12,13,149,153,157,161,165},149},
		{"TAIWAN",0x0B,11,15,{1,2,3,4,5,6,7,8,9,10,11,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},149},
		{"EUROPE",0x02,11,19,{1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,52,56,58,60, 100,104,108,112,116,120,124,128,132,136,140},36},
		{"AMERICA",0x00,11,21,{1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,52,56,60,64,100,104,108,112,116,132,136,140,149,153,157,161,165},36},
		//{"JAPAN",0x10,13,19,{1,2,3,4,5,6,7,8,9,10,11,12,13,36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},36},
		{"JAPAN",0x10,13,4,{1,2,3,4,5,6,7,8,9,10,11,12,13,36,40,44,48},36},
		{"SPAIN",0x03,13,0,{1,2,3,4,5,6,7,8,9,10,11,12,13},36},
		{"FRANCE",0x04,13,0,{1,2,3,4,5,6,7,8,9,10,11,12,13},36},
		{"KOREA",0x0e,11,20,{1,2,3,4,5,6,7,8,9,10,11,36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,149,153,157,161,165},36},
};
typedef struct channel_5gregion
{
	unsigned int  total_5gchannel;
	unsigned int  channel_plan[40];	
}channel5g_t;

channel5g_t channel5g_region_tbl[]={
	{4,{36,40,44,48}},//South Africa
	{8,{36,40,44,48,149,153,157,161}},//Australia
	{8,{36,40,44,48,149,153,157,161}},//Canada
	{4,{149,153,157,161}},//中国
	{14,{36,40,44,48,52,56,60,100,104,108,112,116,132,136}},//Europe
	{8,{36,40,44,48,149,153,157,161,}},//印度
	{4,{36,40,44,48}},//以色列
	//{18,{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136}},//日本
	{4,{36,40,44,48}},//日本
	{8,{36,40,44,48,149,153,157,161}},//韩国
	{8,{36,40,44,48,149,153,157,161}},//马来西亚
	{8,{36,40,44,48,149,153,157,161}},//墨西哥
	{4,{149,153,157,161}},//中东(Iran/Labanon/Qatar)
	{4,{149,153,157,161}},
	{4,{149,153,157,161}},
	{4,{36,40,44,48}},//中东(Turkey/Egypt/Tunisia/Kuwait)
	{4,{36,40,44,48}},
	{4,{36,40,44,48}},
	{4,{36,40,44,48}},
	{4,{36,40,44,48,149,153,157,161}},//中东(Saudi Arabia)
	{4,{36,40,44,48}},//United Arab Emirates
	{4,{36,40,44,48}},//俄罗斯
	{8,{36,40,44,48,149,153,157,161}},//新加坡
	{8,{36,40,44,48,149,153,157,161}},//南美洲
	{7,{56,60,64,149,153,157,161}},//台湾
	{8,{36,40,44,48,149,153,157,161}}//美国

};

int channel_plan_value;
int channel_total_2g;
int channel_total_5g;
int channel_region_index = 0;		//get the channel region index for channel_region_tbl[]
int rtl_band_use=RTL_BAND_24G; 		//rtl_band default is 2.4GHZ for realtek 8192du wifi driver
char Rtl_ChannelRegion[16]={"CHINA"};
int channel_HT40_1[30]={1,2,3,4,5,36,44,52,60,100,108,116,124,132,149,157};
int channel_HT40_0[30]={6,7,8,9,10,11,12,13,40,48,56,64,104,112,120,128,136,144,153,161};
int channel_NA[30]={140,165};
char *myitoa(int num,char *str,int radix)
{
	char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned unum; 
	int i=0,j,k;
	
	if(radix==10&&num<0) 
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}
	else unum=(unsigned)num; 
	do
	{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	 	str[i]='\0';
	if(str[0]=='-') k=1; 
	else k=0;
	char temp;
	for(j=k;j<=(i-k-1)/2;j++)
	{
		temp=str[j];
		str[j]=str[i-j-1];
		str[i-j-1]=temp;
	}
	return str;
}

#define CHANNEL_INDEX_MAX 11
#define CHANNEL_INDEX_MIN 0

typedef struct channelInfoFromProc{
	unsigned int channelIndex;
	unsigned int frameNum;
	unsigned int aroundFrameAverageNum;
	unsigned char isMinFrame; 
}channelInfoFromProc_a;

unsigned short aroundChannle[11+1][9+1] ={
		{2,3,4,5,0},  // 1
		{1,3,4,5,6,0}, // 2
		{1,2,4,5,6,7,0}, // 3
		{1,2,3,5,6,7,8,0}, // 4
		{1,2,3,4,6,7,8,9,0},// 5
		{2,3,4,5,7,8,9,10,0}, // 6 
		{3,4,5,6,8,9,10,11,0},// 7
		{4,5,6,7,9,10,11,12,0}, // 8
		{5,6,7,8,10,11,12,13,0}, // 9
		{6,7,8,9,11,12,13,0}, // 10
		{7,8,9,10,12,13,0}, // 11
		{}
		
};
void get_bestchannel_in_1611(char *best_channel_get_from_proc)
{
	FILE *fp = NULL;
	int ret=0;
	char buf[2048] = {0};
	int channel_index = CHANNEL_INDEX_MIN;
	static int count_channel=0;
	char tmp[4]={0};

	
	char temp_buf[128]={0};
	char *locate1=NULL;
	char key[64] = {0};

	channelInfoFromProc_a channelInfoEntry[CHANNEL_INDEX_MAX + 3];
	unsigned int frameNumMin = 0xFFFF;
	unsigned int aroundFrameAverageNumMin = 0xFFFF;
	unsigned char isMinFrameTotal = 0; 
	unsigned char isMinAroundFrameAverageTotal = 0; 

#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
	if(realtek_dongle_type_divided==REALTEK_CU)
		fp = fopen("/proc/net/rtl819xC/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_DU)
		fp = fopen("/proc/net/rtl819xD/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188EU)
		fp = fopen("/proc/net/rtl8188eu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8192EU)
		fp = fopen("/proc/net/rtl8192eu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8811AU)
		fp = fopen("/proc/net/rtl8821au/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8821CU)
		fp = fopen("/proc/net/rtl8821cu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8189ES)
			fp = fopen("/proc/net/rtl8189es/wlan3/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188FU)
		fp = fopen("/proc/net/rtl8188fu/wlan1/best_channel","r");

#else

	if(realtek_dongle_type_divided==REALTEK_CU)
		fp = fopen("/proc/net/rtl819xC/wlan0/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_DU)
		fp = fopen("/proc/net/rtl819xD/wlan0/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188EU)
		fp = fopen("/proc/net/rtl8188eu/wlan0/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8192EU)
		fp = fopen("/proc/net/rtl8192eu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8189ES)
			fp = fopen("/proc/net/rtl8189es/wlan3/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188FU)
		fp = fopen("/proc/net/rtl8188fu/wlan0/best_channel","r");
#endif	
		if(fp == NULL){
			printf("read best_channel open error!");
			strncpy(best_channel_get_from_proc,"11",strlen("11"));
			return ;
		}
		while(!feof(fp)){
			memset(temp_buf,0,128);
			memset(key,0,64);
			locate1 = NULL ;
			if(channel_index + 1 < 10)
				snprintf(key,64,"The rx cnt of channel   %d = ",channel_index+1);
			else
				snprintf(key,64,"The rx cnt of channel  %2d = ",channel_index+1);
			//printf("key====================%s\n",key);
			//printf("channel_index====================%d\n",channel_index);
			
			fgets(temp_buf,128,fp);
			locate1 = strstr(temp_buf,key);
			//printf("temp_buf=%s",temp_buf);
			if(locate1!=NULL){
				locate1=locate1+strlen(key);
				//printf("locate1=%s",locate1);
				char *locate2=strstr(locate1,"\n");
				memset(tmp,0,4);
				//printf("locate1=%d",strlen(locate1)-strlen(locate2));
				if(strlen(locate1)-strlen(locate2)>0)
					memcpy(tmp,locate1,strlen(locate1)-strlen(locate2));
				
				int tmp_int=atoi(tmp);
				
				//printf("channel_index + 1====================%d\n",channel_index + 1);
				//printf("tmp_int====================%d\n",tmp_int);
				
				channelInfoEntry[channel_index].frameNum = tmp_int;
				
				channelInfoEntry[channel_index].channelIndex = channel_index+1 ;
				channel_index+=5;
			}
			if(channel_index  > CHANNEL_INDEX_MAX)
				break;
		}
		
		channel_index = CHANNEL_INDEX_MIN;
		while(channel_index < CHANNEL_INDEX_MAX){
			//printf("frameNumMin====================%d\n",frameNumMin);
			//printf("channelInfoEntry[channel_index].frameNum====================%d\n",channelInfoEntry[channel_index].frameNum);
			if(frameNumMin > channelInfoEntry[channel_index].frameNum){
				frameNumMin = channelInfoEntry[channel_index].frameNum;
			}
			channel_index +=5;
		}
		//printf("frameNumMin last====================%d\n",frameNumMin);

		
		channel_index = CHANNEL_INDEX_MIN;
		while(channel_index < CHANNEL_INDEX_MAX){
			if(frameNumMin == channelInfoEntry[channel_index].frameNum){
				channelInfoEntry[channel_index].isMinFrame = 1;
				isMinFrameTotal ++ ;
			}else{
				channelInfoEntry[channel_index].isMinFrame = 0;
			}
			channel_index +=5;
		}

		//printf("isMinFrameTotal====================%d\n",isMinFrameTotal);

		if(isMinFrameTotal == 1){
			
			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX+1){
				if(frameNumMin == channelInfoEntry[channel_index].frameNum){
					
					printf("channelInfoEntry[channel_index].channelIndex====================%d\n",channelInfoEntry[channel_index].channelIndex);
					myitoa(channelInfoEntry[channel_index].channelIndex,best_channel_get_from_proc,10);
					printf("best_channel_get_from_proc ====================%s\n",best_channel_get_from_proc);
					break;
				}
				
				channel_index +=5;
			}			
		}
		else if(isMinFrameTotal > 1){			
			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX+1){
				if(channelInfoEntry[channel_index].isMinFrame == 1){
					int i = 0;
					int aroundCount = 0;
					
					channelInfoEntry[channel_index].aroundFrameAverageNum = 0;
					for(i = 0 ; i < 9 ;i++){
						if(aroundChannle[channel_index][i] == 0){
							aroundCount = i;
							break;
						}
					}
					//printf("aroundCount ====== %d\n",aroundCount);
					for(i=0 ; i< aroundCount ; i++){
						
						//printf("aroundChannle[channel_index][i]-1====== %d\n",aroundChannle[channel_index][i]-1);
						//printf("channelInfoEntry[aroundChannle[channel_index][i]-1].frameNum====== %d\n",channelInfoEntry[aroundChannle[channel_index][i]-1].frameNum);
						
						channelInfoEntry[channel_index].aroundFrameAverageNum += channelInfoEntry[aroundChannle[channel_index][i]-1].frameNum;
						
						//printf("channelInfoEntry[channel_index].aroundFrameAverageNum temp====== %d\n",channelInfoEntry[channel_index].aroundFrameAverageNum);
					}
					
					//printf("channelInfoEntry[channel_index].aroundFrameAverageTotal ====== %d\n",channelInfoEntry[channel_index].aroundFrameAverageNum);
					channelInfoEntry[channel_index].aroundFrameAverageNum *= 10; //aviod some Average equel 

					channelInfoEntry[channel_index].aroundFrameAverageNum /= aroundCount;
					
					//printf("channelInfoEntry[channel_index].aroundFrameAverageNum ====== %d\n",channelInfoEntry[channel_index].aroundFrameAverageNum);
				}
				channel_index +=5;
			}

			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX+1){
				
				if(1 == channelInfoEntry[channel_index].isMinFrame){
					if(aroundFrameAverageNumMin > channelInfoEntry[channel_index].aroundFrameAverageNum);
						aroundFrameAverageNumMin = channelInfoEntry[channel_index].aroundFrameAverageNum;
				}
				channel_index +=5;
			}

			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX+1){
				
				if(aroundFrameAverageNumMin == channelInfoEntry[channel_index].aroundFrameAverageNum){
					isMinAroundFrameAverageTotal ++;
					memset(best_channel_get_from_proc,0,sizeof(best_channel_get_from_proc));
					myitoa(channelInfoEntry[channel_index].channelIndex,best_channel_get_from_proc,10);
				}				
				channel_index +=5;
			}			
		}
		else{
			printf("[%s - %d]That must be error,isMinFrameTotal should not < 1\n",__FILE__,__LINE__);
			strncpy(best_channel_get_from_proc,"11",sizeof("11"));
		}
		
		if(strcmp(best_channel_get_from_proc,"0")==0)		//if the channel is 0,then set the channel to 11
			strncpy(best_channel_get_from_proc,"11",sizeof("11"));		
		ret=fclose(fp);
		//printf("best_channel_get_from_proc=%s",best_channel_get_from_proc);


}
void best_channel_setting(char *best_channel_get_from_proc,int band_type){
#if !(defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1) 
	char callbuf[128];
	FILE *fp = NULL;
	int ret=0;
	char buf[2048] = {0};
	int channel_index = CHANNEL_INDEX_MIN;
	static int count_channel=0;
	char tmp[128]={0};
	int val_country=-1;
	char temp_buf[128]={0};
	char *locate1=NULL;
	char key[64] = {0};

	channelInfoFromProc_a channelInfoEntry[CHANNEL_INDEX_MAX + 3];
	unsigned int frameNumMin = 0xFFFF;
	unsigned int aroundFrameAverageNumMin = 0xFFFF;
	unsigned char isMinFrameTotal = 0; 
	unsigned char isMinAroundFrameAverageTotal = 0; 
	
	memset(&channelInfoEntry,0,(CHANNEL_INDEX_MAX + 1)*sizeof(channelInfoFromProc_a));
//	printf("[%s,%d]:best_channel_get_from_proc is %s\n",__FUNCTION__,__LINE__,best_channel_get_from_proc);
	if(best_channel_get_from_proc==NULL)
		return;	

	memset(callbuf,0,128);
	sprintf(callbuf,"chmod 777 /sbin/iwlist");
	system(callbuf);
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
	memset(callbuf,0,128);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	realtek_dongle_type_divided=get_realtek_dongle_type();
	if(vid_pid_from_driver==0x148f8189)
		sprintf(callbuf,"iwlist wlan3 scan > /dev/null");
	else
		sprintf(callbuf,"iwlist wlan1 scan > /dev/null");
#else 
	sprintf(callbuf,"iwlist wlan1 scan > /dev/null");
#endif
	system(callbuf);
#else
	memset(callbuf,0,128);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	if(vid_pid_from_driver==0x148f8189)
		sprintf(callbuf,"iwlist wlan2 scan > /dev/null");
	else
		sprintf(callbuf,"iwlist wlan0 scan > /dev/null");
#else
	sprintf(callbuf,"iwlist wlan0 scan > /dev/null");
#endif
	system(callbuf);
#endif	
	printf("realtek_dongle_type_divided=%d 5g_num:%d 2g_num:%d\n",realtek_dongle_type_divided,channel_total_5g,channel_total_2g);
	
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
	if(realtek_dongle_type_divided==REALTEK_CU)
		fp = fopen("/proc/net/rtl819xC/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_DU)
		fp = fopen("/proc/net/rtl819xD/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188EU)
		fp = fopen("/proc/net/rtl8188eu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8192EU)
		fp = fopen("/proc/net/rtl8192eu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8811AU)
		fp = fopen("/proc/net/rtl8821au/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8821CU)
		fp = fopen("/proc/net/rtl8821cu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8189ES)
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		fp = fopen("/proc/net/rtl8821au/wlan1/best_channel","r");
#else
		fp = fopen("/proc/net/rtl8189es/wlan3/best_channel","r");
#endif
	else if(realtek_dongle_type_divided==REALTEK_8188FU)
		fp = fopen("/proc/net/rtl8188fu/wlan1/best_channel","r");

#else

	if(realtek_dongle_type_divided==REALTEK_CU)
		fp = fopen("/proc/net/rtl819xC/wlan0/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_DU)
		fp = fopen("/proc/net/rtl819xD/wlan0/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188EU)
		fp = fopen("/proc/net/rtl8188eu/wlan0/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8192EU)
		fp = fopen("/proc/net/rtl8192eu/wlan1/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8189ES)
		fp = fopen("/proc/net/rtl8189es/wlan3/best_channel","r");
	else if(realtek_dongle_type_divided==REALTEK_8188FU)
		fp = fopen("/proc/net/rtl8188fu/wlan0/best_channel","r");
#endif	
	if(fp == NULL){
		printf("read best_channel open error!");
		if(band_type==0x02)
			strncpy(best_channel_get_from_proc,"11",strlen("11"));
/*
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	//rtl8189es/wlan3/best_channel	cannot find ,use 11 default
	//printf("vid_pid_from_driver=%d",vid_pid_from_driver);
	if(vid_pid_from_driver==0x148f8189){
		printf("8189 best channel 11!!!\n");
		memset(best_channel_get_from_proc,0,sizeof(best_channel_get_from_proc));
		strncpy(best_channel_get_from_proc,"11",strlen("11"));
	}
#endif
*/
		return ;
	}
	
	if(band_type==0x02){// 2.4GHZ channel band
			
		while(!feof(fp)){

			memset(temp_buf,0,128);
			memset(key,0,64);
			locate1 = NULL ;
			if(channel_index + 1 < 10)
				snprintf(key,64,"The rx cnt of channel   %d = ",channel_index+1);
			else
				snprintf(key,64,"The rx cnt of channel  %2d = ",channel_index+1);
			//printf("key====================%s\n",key);
			//printf("channel_index====================%d\n",channel_index);
			
			fgets(temp_buf,128,fp);
			//printf("temp_buf====================%s\n",temp_buf);
			locate1 = strstr(temp_buf,key);
			if(locate1!=NULL){
				locate1=locate1+strlen(key);
				char *locate2=strstr(locate1,"\n");
				memset(tmp,0,4);
				if(strlen(locate1)-strlen(locate2)>0)
					memcpy(tmp,locate1,strlen(locate1)-strlen(locate2));
				
				int tmp_int=atoi(tmp);
				
				//printf("channel_index + 1====================%d\n",channel_index + 1);
				//printf("tmp_int====================%d\n",tmp_int);
				
				channelInfoEntry[channel_index].frameNum = tmp_int;
				
				channelInfoEntry[channel_index].channelIndex = channel_index + 1;
				
				channel_index++;
			}
			if(channel_index  > CHANNEL_INDEX_MAX + 1)
				break;
		}
		
		channel_index = CHANNEL_INDEX_MIN;
		while(channel_index < CHANNEL_INDEX_MAX){
			//printf("frameNumMin====================%d\n",frameNumMin);
			//printf("channelInfoEntry[channel_index].frameNum====================%d\n",channelInfoEntry[channel_index].frameNum);
			if(frameNumMin > channelInfoEntry[channel_index].frameNum){
				frameNumMin = channelInfoEntry[channel_index].frameNum;
			}
			channel_index ++;
		}
		//printf("frameNumMin last====================%d\n",frameNumMin);

		
		channel_index = CHANNEL_INDEX_MIN;
		while(channel_index < CHANNEL_INDEX_MAX){
			if(frameNumMin == channelInfoEntry[channel_index].frameNum){
				channelInfoEntry[channel_index].isMinFrame = 1;
				isMinFrameTotal ++ ;
			}else{
				channelInfoEntry[channel_index].isMinFrame = 0;
			}
			channel_index ++;
		}

		printf("isMinFrameTotal====================%d\n",isMinFrameTotal);

		if(isMinFrameTotal == 1){
			
			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX){
				if(frameNumMin == channelInfoEntry[channel_index].frameNum){
					
					printf("channelInfoEntry[channel_index].channelIndex====================%d\n",channelInfoEntry[channel_index].channelIndex);
					myitoa(channelInfoEntry[channel_index].channelIndex,best_channel_get_from_proc,10);
					printf("best_channel_get_from_proc ====================%s\n",best_channel_get_from_proc);
					break;
				}
				
				channel_index ++;
			}			
		}
		else if(isMinFrameTotal > 1){			
			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX){
				if(channelInfoEntry[channel_index].isMinFrame == 1){
					int i = 0;
					int aroundCount = 0;
					
					channelInfoEntry[channel_index].aroundFrameAverageNum = 0;
					for(i = 0 ; i < 9 ;i++){
						if(aroundChannle[channel_index][i] == 0){
							aroundCount = i;
							break;
						}
					}
					//printf("aroundCount ====== %d\n",aroundCount);
					for(i=0 ; i< aroundCount ; i++){
						
						//printf("aroundChannle[channel_index][i]-1====== %d\n",aroundChannle[channel_index][i]-1);
						//printf("channelInfoEntry[aroundChannle[channel_index][i]-1].frameNum====== %d\n",channelInfoEntry[aroundChannle[channel_index][i]-1].frameNum);
						
						channelInfoEntry[channel_index].aroundFrameAverageNum += channelInfoEntry[aroundChannle[channel_index][i]-1].frameNum;
						
						//printf("channelInfoEntry[channel_index].aroundFrameAverageNum temp====== %d\n",channelInfoEntry[channel_index].aroundFrameAverageNum);
					}
					
					//printf("channelInfoEntry[channel_index].aroundFrameAverageTotal ====== %d\n",channelInfoEntry[channel_index].aroundFrameAverageNum);
					channelInfoEntry[channel_index].aroundFrameAverageNum *= 10; //aviod some Average equel 

					channelInfoEntry[channel_index].aroundFrameAverageNum /= aroundCount;
					
					//printf("channelInfoEntry[channel_index].aroundFrameAverageNum ====== %d\n",channelInfoEntry[channel_index].aroundFrameAverageNum);
				}
				channel_index ++;
			}

			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX){
				
				if(1 == channelInfoEntry[channel_index].isMinFrame){
					if(aroundFrameAverageNumMin > channelInfoEntry[channel_index].aroundFrameAverageNum);
						aroundFrameAverageNumMin = channelInfoEntry[channel_index].aroundFrameAverageNum;
				}
				channel_index ++;
			}

			channel_index = CHANNEL_INDEX_MIN;
			while(channel_index < CHANNEL_INDEX_MAX){
				
				if(aroundFrameAverageNumMin == channelInfoEntry[channel_index].aroundFrameAverageNum){
					isMinAroundFrameAverageTotal ++;
					memset(best_channel_get_from_proc,0,sizeof(best_channel_get_from_proc));
					myitoa(channelInfoEntry[channel_index].channelIndex,best_channel_get_from_proc,10);
				}				
				channel_index ++;
			}			
		}
		else{
			printf("[%s - %d]That must be error,isMinFrameTotal should not < 1\n",__FILE__,__LINE__);
			strncpy(best_channel_get_from_proc,"11",sizeof("11"));
		}
		
		if(strcmp(best_channel_get_from_proc,"0")==0)		//if the channel is 0,then set the channel to 11
			strncpy(best_channel_get_from_proc,"11",sizeof("11"));		
		bestChannelChosen = atoi(best_channel_get_from_proc);
	}
	else if(band_type==0x05){// 5GHZ channel band
	
		int val_length;
		int i;
		int channel_rx_cnt=-1;
		char temp_buf[128]={0};
		int tmp_rxcnt[40]={0};
		char *locate=NULL,*locate1=NULL;
		memset(key,0,64);

		#if EZCAST_ENABLE
		val_country=ezCastGetNumConfig(CONFNAME_SOFTAP_5G_COUNTRY);
		#else
		val_country = 24;	// USA
		#endif
		printf("get_bestchannel5g val_country=%d\n",val_country);
		val_length=channel5g_region_tbl[val_country].total_5gchannel;

		while(!feof(fp))
		{
			fgets(temp_buf,128,fp);
			for(i=0;i<val_length;i++)
			{
				if(channel5g_region_tbl[val_country].channel_plan[i]<100)
					snprintf(key,64,"The rx cnt of channel  %2d = ",channel5g_region_tbl[val_country].channel_plan[i]);
				else
					snprintf(key,64,"The rx cnt of channel %3d = ",channel5g_region_tbl[val_country].channel_plan[i]);

				locate1 = strstr(temp_buf,key);
				//printf("key=%s\n",key);
				if(locate1!=NULL)
				{
					locate1=locate1+strlen(key);
					char *locate2=strstr(locate1,"\n");
					memset(tmp,0,128);
					if(locate1<locate2)
						memcpy(tmp,locate1,strlen(locate1)-strlen(locate2));
					
					tmp_rxcnt[i]=atoi(tmp);
				}
				
			}
		}
		channel_rx_cnt=tmp_rxcnt[0];
		myitoa(channel5g_region_tbl[val_country].channel_plan[0],best_channel_get_from_proc,10);	
		for(i=0;i<val_length;i++)
		{
			if(tmp_rxcnt[i]<channel_rx_cnt)
			{
				channel_rx_cnt=tmp_rxcnt[i];
				myitoa(channel5g_region_tbl[val_country].channel_plan[i],best_channel_get_from_proc,10);	
			}
		}

		
		//bestChannelChosen = channel_rx_cnt;
	}
	ret=fclose(fp);
	bestChannelChosen = atoi(best_channel_get_from_proc);
	fprintf(stderr,"function:%s line:%d best_channel_get_from_proc:%s\n",__FUNCTION__,__LINE__,best_channel_get_from_proc);
#endif
}

static int realtek_sta_func(int dongle_dir,void *data)
{
	int ret=0;
	if(dongle_dir==DONGLE_OUT){
		char cmd[128];
		memset(cmd,0,128);
		sprintf(cmd,"killall udhcpc");
		ret = system(cmd);
		fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		if(ret!=0)
			goto __end;	
	}
__end:
	if(ret!=0)
		ret = -1;
	return ret;
}


static int ralink_sta_func(int dongle_dir,void *data)
{
	int ret=0;
	if(dongle_dir==DONGLE_OUT){
		char cmd[128];
		memset(cmd,0,128);
		sprintf(cmd,"killall udhcpc");
		ret = system(cmd);
		fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		if(ret!=0)
			goto __end;	
		
	}
__end:
	if(ret!=0)
		ret = -1;
	return ret;
}

static int ralink_softap_func(int dongle_dir,void *data)
{
	int ret=0;
	char cmd[128];

	wifidbg_info("ralink_softap_func dongledir=%d",dongle_dir);
	if(dongle_dir==DONGLE_IN){
		memset(cmd,0,128);	
		sprintf(cmd,"chmod -R 644 /root/html/");
		ret= system(cmd);
	
		memset(cmd,0,128);	
		sprintf(cmd,"chmod -R 755 /root/html/cgi-bin/");
		ret= system(cmd);
		
		memset(cmd,0,128);
		sprintf(cmd,"ifconfig ra0 up");
		ret = system(cmd);
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
		memset(cmd,0,128);
		sprintf(cmd,"ifconfig ra0 192.168.111.1");
		ret = system(cmd);
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

		memset(cmd,0,128);
		sprintf(cmd,"udhcpd /etc/ra0-udhcpd.conf");
		ret = system(cmd);
		
		memset(cmd,0,128);	
		sprintf(cmd,"thttpd -C /etc/thttpd.conf");
		ret= system(cmd);
	}
	else {
		memset(cmd,0,128);
		sprintf(cmd,"killall udhcpd");
		ret = system(cmd);
		fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	
		if(ret!=0)
			goto __end;	
	}
__end:
	if(ret!=0)
		ret = -1;
	return ret;
}


int re_insmod_realtek_ko(){

		int num=100;
		char callbuf[50];
		int ret=0;
		for(num=100;ret==0||num<0;num--)
			{
					sprintf(callbuf,"insmod /lib/modules/2.6.27.29/8192cu.ko");
					system(callbuf);
					sprintf(callbuf,"ifconfig wlan0 192.168.0.1");
					ret=system(callbuf);
					if(ret==0)
						return 0;
					
			}
		return -1;
}

int realtek_interface_identy(){
	int ret = -1,fd=-1;
	char *dev_name=NULL;
	char buf[512] ={0x00};
	dev_name=malloc(32);
	if(dev_name == NULL){
		free(dev_name);
		goto flage; 
	}
	printf("%s %d\n",__FUNCTION__,__LINE__);
	strcpy(dev_name,"/proc/net/dev");
	fd = open("/proc/net/dev",O_RDONLY);
	read(fd,buf,512);
	printf("%s %d buf:%s\n",__FUNCTION__,__LINE__,buf);
	if(strstr(buf,"wlan0")!=NULL && strstr(buf,"wlan1")!=NULL ){
		ret = 2;
	}else if(strstr(buf,"wlan0")!=NULL ){
		ret = 1;
	}

	free(dev_name);
	printf("%s %d ret:%d\n",__FUNCTION__,__LINE__,ret);
	return ret;
flage:
	return ret;
}

/**
*check the concurrent mode softap relational conf
*/
void checkAndRecoverForConcurrent(int mode){//add mode for 24G ,5G
	int ret;
	char cmd[128]={0};
	struct stat buf;
	char conf_file[100]="";
	char confbak_file[100]="";
	char conf_rtl_file[100]="/etc/rtl_hostapd_01.conf";
	char confbak_udp_file[100]="/mnt/bak/udhcpd_01.conf";
	char conf_udp_file[100]="/etc/udhcpd_01.conf";

	printf("--checkAndRecoverForConcurrent--mode==%d\n",mode);
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
/*
	if(vid_pid_from_driver==0x148f8189){
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
		sprintf(conf_rtl_file,"%s","/etc/rtl_hostapd_02.conf");
		sprintf(confbak_udp_file,"%s","/mnt/bak/udhcpd_02.conf");
		sprintf(conf_udp_file,"%s","/etc/udhcpd_02.conf");
	}
*/

	if(mode==RTL_BAND_24G)//5G use 01.conf,24G use 02.conf
	{
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
		sprintf(conf_rtl_file,"%s","/etc/rtl_hostapd_02.conf");
		sprintf(confbak_udp_file,"%s","/mnt/bak/udhcpd_02.conf");
		sprintf(conf_udp_file,"%s","/etc/udhcpd_02.conf");
	}
#endif
	/****check and recover rtl_hostapd_01.conf****/
	if(access(conf_file,F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"cp %s %s",confbak_file,conf_file);
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]recover %s ok!!\n",__FUNCTION__,__LINE__,conf_file);
		else
			printf("[%s,%d]recover %s error!!\n",__FUNCTION__,__LINE__,conf_file);
		system("sync");
	}
	/****check and recover rtl_hostapd_01.conf link file****/
	if(access(conf_rtl_file,F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"ln -s  %s %s",conf_file,conf_rtl_file);
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]link %s ok!!\n",__FUNCTION__,__LINE__,conf_rtl_file);
		else
			printf("[%s,%d]link %s error!!\n",__FUNCTION__,__LINE__,conf_rtl_file);
		system("sync");
	}
	else{
		printf("--ln softap to /etc--\n");
		lstat(conf_rtl_file,&buf);					//check the symbolic link .if unexist ,rebuild it
		if(!S_ISLNK(buf.st_mode)){
			memset(cmd,0,128);
			sprintf(cmd,"ln -sf	%s %s",conf_file,conf_rtl_file);
		//	printf("--show /etc/rtl_hostapd_02.conf--\n");
			//system("cat /etc/rtl_hostapd_02.conf");
			ret=system(cmd);
			if(ret == 0)
				printf("[%s,%d]link %s ok!!\n",__FUNCTION__,__LINE__,conf_rtl_file);
			else
				printf("[%s,%d]link %s error!!\n",__FUNCTION__,__LINE__,conf_rtl_file);
			system("sync");

		}
	}
	/****check and recover udhcpd_01.conf****/
	if(access(conf_udp_file,F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"cp %s  %s",confbak_udp_file,conf_udp_file);
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]recover %s ok!!\n",__FUNCTION__,__LINE__,conf_udp_file);
		else
			printf("[%s,%d]recover %s error!!\n",__FUNCTION__,__LINE__,conf_udp_file);
		system("sync");
	}
	//system("sync");
}
/**
*check and recover independent softap relational conf
*/
void checkAndRecoverForIndependent(){
	int ret;
	char cmd[128]={0};
	struct stat buf;
	/****check and recover rtl_hostapd.conf****/
	if(access("/mnt/user1/softap/rtl_hostapd.conf",F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"cp /mnt/bak/rtl_hostapd.conf  /mnt/user1/softap/rtl_hostapd.conf");
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]recover rtl_hostapd.conf ok!!",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]recover rtl_hostapd.conf error!!",__FUNCTION__,__LINE__);
		system("sync");
	}
	
	/***check and recover rtl_hostapd.conf link file****/
	if(access("/etc/rtl_hostapd.conf",F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"ln -s  /mnt/user1/softap/rtl_hostapd.conf /etc/rtl_hostapd.conf");
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]link rtl_hostapd.conf ok!!",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]link rtl_hostapd.conf error!!",__FUNCTION__,__LINE__);
		system("sync");
	}
	else{
		lstat("/etc/rtl_hostapd.conf",&buf);						//check the symbolic link.if unexist ,rebuild it
		if(!S_ISLNK(buf.st_mode)){
			memset(cmd,0,128);
			sprintf(cmd,"ln -sf  /mnt/user1/softap/rtl_hostapd.conf /etc/rtl_hostapd.conf");
			ret=system(cmd);
			if(ret == 0)
				printf("[%s,%d]link rtl_hostapd.conf ok!!",__FUNCTION__,__LINE__);
			else
				printf("[%s,%d]link rtl_hostapd.conf error!!",__FUNCTION__,__LINE__);
			system("sync");
		}
	}
	/***check and recover udhcpd.conf***/
	if(access("/etc/udhcpd.conf",F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"cp /mnt/bak/udhcpd.conf  /etc/udhcpd.conf");
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]recover udhcpd.conf ok!!",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]recover udhcpd.conf error!!",__FUNCTION__,__LINE__);
		system("sync");
	}
	//system("sync");
}


int realtek_softap_func(int dongle_dir,void *data)
{
	int ret=-1;
	int rtl_interface = -1;
	char cmd[128];

	if(dongle_dir==DONGLE_IN){

#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE		//bluse:add

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
			checkAndRecoverForConcurrent(RTL_BAND_5G); 
			checkAndRecoverForConcurrent(RTL_BAND_24G); 	//check and recover some conf files	
#else
			checkAndRecoverForConcurrent(RTL_BAND_24G); 	//check and recover some conf files
#endif
			memset(cmd,0,128);
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
		
	#if MODULE_CONFIG_EZCASTPRO_MODE==8075
			if(vid_pid_from_driver!=0x148f8189){
				//system("cat /etc/rtl_hostapd_01.conf");
				sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_concurrent_softap_mode_EZCastPro");
				printf("wlan1 start softap at EZCastPro!!!\n");
			}else{
				//system("cat /etc/rtl_hostapd_02.conf");
				sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_concurrent_softap_mode_EZCastPro%d",2);
				printf("wlan3 start softap at EZCastPro!!!\n");

			}
	#else
			//system("cat /etc/rtl_hostapd_01.conf");
			sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_concurrent_softap_mode_EZCastPro");
			printf("wlan1 start softap at EZCastPro!!!\n");
	
	#endif
#else	
			sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_concurrent_softap_mode_EZCast");
			printf("start softap at EZCast!!!\n");
#endif			
#else
			sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_concurrent_softap_mode");
#endif
			printf("cmd===============%s\n",cmd);
			ret = pac_system(cmd);			
#else
			checkAndRecoverForIndependent();		//check and recover some conf files
			memset(cmd,0,128);
			sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_softap_mode");
			ret = pac_system(cmd);
#endif	
	}
	else {
			memset(cmd,0,128);
			sprintf(cmd,"sh /am7x/case/scripts/wifi_softap_process.sh realtek_client_mode");
			ret = pac_system(cmd);
	}

	return ret;
}

static unsigned int get_pidvid(dongle_t * dongle)
{
	if(dongle)
		return dongle->vidpid;
	else
		return 0;
}

unsigned int dongle_get_pidvid()
{
	unsigned int pidvid=0;
	if(cur_dongle.status==DONGLE_OK){
		wifi_sem_wait(&cur_dongle.lock);
		pidvid = get_pidvid(cur_dongle.dongle);
		wifi_sem_post(&cur_dongle.lock);
	}
	return pidvid;
}


static dongle_t * match_dongle(unsigned int vidpid)
{
	dongle_t * dongle=NULL;
	if(vidpid==0){
		wifidbg_info("vid pid may be error!check it,vidpid=0x%8x",vidpid);
		return NULL;
	}

	if(dongle_connect_type==CONNECT_TYPE_STA){
		dongle = dongle_tbl;
	}
	else{
		dongle = dongle_softap_tbl;
	}
	while(1){
		if(dongle->vidpid==vidpid){
			break;
		}
		else if(dongle->vidpid==0){///the end of the table
			dongle = NULL;
			break;
		}
		dongle++;
	}
	return dongle;
}

static char* get_device(dongle_t * dongle)
{
		
	if(dongle)
		return dongle->device_name;
	else
		return NULL;
}

static char * get_driver(dongle_t * dongle)
{
	if(dongle)
		return dongle->driver_name;
	else
		return NULL;
}

static char * get_dev(dongle_t * dongle)
{
	if(dongle)
		return dongle->dev_rmmod;
	else
		return NULL;
}

static dongle_handle_func get_func(dongle_t * dongle)
{
	if(dongle)
		return dongle->dongle_func;
	else
		return NULL;
}

static int handle_dongle(int dongle_dir)
{
	int ret=0;
	printf("now is handle_dongle beginning!\n");
	printf("dongle_dir=======%d\n",dongle_dir);
	dongle_handle_func func=NULL;
	if(cur_dongle.status==DONGLE_OK){
		wifi_sem_wait(&cur_dongle.lock);
		func = get_func(cur_dongle.dongle);
		wifi_sem_post(&cur_dongle.lock);
	}
	else
		ret = -1;

	if(func){
		ret = func(dongle_dir,NULL);
	}

	return ret;
}

int pre_vid_pid_form_driver=0;
int int_probox_mode_best_channel(int mode_type)
{
	FILE *fp = NULL;
	char buf[4096] ={0};
	char conf_file[128] = {0};
	char confbak_file[100]={0};
	char callbuf[128];
	int ret=-1;
	int channel_int=0;
	int chna=0;
	int Bandwidth_24g=0;
	int Bandwidth_5g=0;
	char channel_temp[4]={0};
	
	if(mode_type==RTL_BAND_5G){
		sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
		sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
		if(0==access("/sys/module/8821au",F_OK)){
			#if EZCAST_ENABLE
			Bandwidth_5g=ezCastGetNumConfig(CONFNAME_BANDWIDTH_5G);//for pro box
			#endif
			myitoa(channel_region_tbl[channel_region_index].default_channel_5g,channel_temp,10);
			best_channel_setting(channel_temp,RTL_BAND_5G);
			printf("%s--5G\n",__func__);
		}
	}
	else{
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
		if(0==access("/sys/module/8189es",F_OK)){
			#if EZCAST_ENABLE
			Bandwidth_24g=ezCastGetNumConfig(CONFNAME_BANDWIDTH_24G);//for pro box
			#endif
			best_channel_setting(channel_temp,RTL_BAND_24G);
			printf("%s--2.4G\n",__func__);
		}
	}

	if(channel_temp[0]=='\0'){
		printf("function:%s,line:%d channel==================NULL\n",__FUNCTION__,__LINE__);
		return -1;
	}
	printf("%s channel_temp============%s\n",__func__,channel_temp);

	channel=channel_temp;
	
	fp = fopen(conf_file,"r");
	if(fp == NULL){
				
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

	channel_int=atoi(channel);
	printf("<%s %d>channel_int=%d\n",__func__,__LINE__,channel_int);

	if((channel_int>=36)&&(channel_int<=165))//channel 5g
	{
		if(Bandwidth_5g==1)//1--> 20MHz, 0--> 40MHz)
		{						
			modify_line_realtek(buf,"ht_capab=","[SHORT-GI-20][SHORT-GI-40]");
		}
		else
		{
			for(chna=0;chna<30;chna++)
			{
				if(channel_int==channel_HT40_1[chna])
				{
					printf("[HT40+]channel_int=%d\n",channel_int);	
					modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
				}
				else if(channel_int==channel_HT40_0[chna])
				{
					printf("[HT40-]channel_int=%d\n",channel_int);
					modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
				}
				else if (channel_int==channel_NA[chna])
				{
					printf("[HT40]channel_int=%d\n",channel_int);
					modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
				}
			}

		}
	}
	else if((channel_int>=1)&&(channel_int<=14))//channel 24g
	{
		if(Bandwidth_24g==1)//1--> 20MHz, 0--> 40MHz)
		{						
			modify_line_realtek(buf,"ht_capab=","[SHORT-GI-20][SHORT-GI-40]");
		}
		else
		{
			for(chna=0;chna<30;chna++)
			{
				if(channel_int==channel_HT40_1[chna])
				{
					printf("[HT40+]channel_int=%d\n",channel_int);	
					modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
				}
				else if(channel_int==channel_HT40_0[chna])
				{
					printf("[HT40-]channel_int=%d\n",channel_int);
					modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
				}
				else if (channel_int==channel_NA[chna])
				{
					printf("[HT40]channel_int=%d\n",channel_int);
					modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
				}
			}
		}
	}

	//if(channel_int>=1&&channel_int<=11)
	modify_line_realtek(buf,"channel=",channel);
	
	if(buf!=NULL){
		fp = fopen(conf_file,"wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			return -1;
		}
		
		ret=fwrite(buf, 1, 4096, fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);
	}
	return 0;
	
}

int set_max_connect_mun(char* mode_val,int channel)
{
	FILE *fp = NULL;
	char buf[4096] ={0};
	char conf_file[100]="";
	char confbak_file[100]="";
	char callbuf[128];
	int ret=-1;
	char *max_mun= NULL;
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
	if(mode_val==NULL)
		return -1;
	if(RTL_BAND_5G==channel){
		sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
		sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
	} else {
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		char confbak_file[100]="/mnt/bak/rtl_hostapd_02.conf";
	}
	fp = fopen(conf_file,"r");
	if(fp == NULL){
				
		sprintf(callbuf,"cp %s  %s",confbak_file,conf_file);
		system(callbuf);
		fp = fopen(conf_file,"r");
		if(fp == NULL){
			printf("%s %d open error %s\n",__func__,__LINE__,strerror(errno));
			return -1;
		}
	}
	ret=fread(buf, 1, 4096, fp);
	ret=fclose(fp);
	if(atoi(mode_val)>2007){
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
		modify_line_realtek(buf,"max_num_sta=","16");
	#else
		modify_line_realtek(buf,"max_num_sta=","8");
	#endif
	}
	else
		modify_line_realtek(buf,"max_num_sta=",mode_val);
	
	if(buf!=NULL){
		fp = fopen(conf_file,"wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			return -1;
		}
		
		ret=fwrite(buf, 1, 4096, fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);
	}
	return 0;
	
}

int get_max_connect_mun()
{
	FILE *fp = NULL;
	char buf[4096] ={0};
	char conf_file[128] = "";
	char callbuf[128];
	int ret=-1;
	char *tmp= NULL;
	
	char confbak_file[100]="";
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
	fp = fopen(conf_file,"r");
	if(fp == NULL){
		sprintf(callbuf,"cp %s  %s",confbak_file,conf_file);
		system(callbuf);
		fp = fopen(conf_file,"r");
		if(fp == NULL){
			printf("%s %d open error %s\n",__func__,__LINE__,strerror(errno));
			return -1;
		}
	}
	tmp=get_modify_line_realtek(conf_file,"max_num_sta=");
	return tmp;
	}
int inti_concurrent_mode_best_channel(){
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	int channel_int=0;
	int chna=0;
	int Bandwidth_24g=0;
	int Bandwidth_5g=0;
	int band_type;
	char channel_temp[4]={0};
	char conf_file[100]="";
	char confbak_file[100]="";
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	Bandwidth_24g=ezCastGetNumConfig(CONFNAME_BANDWIDTH_24G);//for pro lan/dongle
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	//rtl_band_use=RTL_BAND_5G;
	//printf("Probox use 5G band !!!\n");
	if(vid_pid_from_driver!=0x148f8189){
		rtl_band_use=RTL_BAND_5G;
		Bandwidth_5g=ezCastGetNumConfig(CONFNAME_BANDWIDTH_5G);
		sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
		sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
	}
	else{
		rtl_band_use=RTL_BAND_24G;
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		char confbak_file[100]="/mnt/bak/rtl_hostapd_02.conf";
	}
#else
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
#endif
	
	#if (defined(MODULE_CONFIG_3TO1) && MODULE_CONFIG_3TO1==1)
	channel_p2p = 6;
	memset(channel_temp, 0, sizeof(channel_temp));
	sprintf(channel_temp, "%d", channel_p2p);
	channel=channel_temp;
	printf("<%s %d>softap channel as same as p2p listen channel for softap+p2p\n",__func__,__LINE__);
	printf("<%s %d>channel=%s,channel_p2p=%d\n",__func__,__LINE__,channel,channel_p2p);
	#else
	memset(channel_temp,0,4);
	if(0==access("/sys/module/8192du",F_OK) || 0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK)){
		if(rtl_band_use==RTL_BAND_24G){  //switch realtek wifi band
			best_channel_setting(channel_temp,RTL_BAND_24G);
			printf("--2.4G\n");
		}else{
			myitoa(channel_region_tbl[channel_region_index].default_channel_5g,channel_temp,10);
			best_channel_setting(channel_temp,RTL_BAND_5G);
			printf("--5G\n");
		}
	}else{
		best_channel_setting(channel_temp,RTL_BAND_24G);
	}

	if(channel_temp[0]== '\0'){
		printf("function:%s,line:%d channel==================NULL\n",__FUNCTION__,__LINE__);
		return -1;
	}
	printf("channel_temp===============%s\n",channel_temp);
	if(realtek_dongle_type_divided==REALTEK_8188EU){
		printf("8188 get bestchannel for p2p!!!\n");
		char tmp_chn[4]="";
		get_bestchannel_in_1611(tmp_chn);
		channel_p2p=atoi(tmp_chn);
		if(channel_p2p==1 ||channel_p2p==6||channel_p2p==11)
			printf("p2p_channel=%d\n",channel_p2p);
		else
			channel_p2p=0;
	}
	channel=channel_temp;
	#endif

	//printf("channel1111111111111111=======%s\n",channel);
	
	fp = fopen(conf_file,"r");
	if(fp == NULL){
				
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
	
	channel_int=atoi(channel);
	printf("<%s %d>channel_int=%d\n",__func__,__LINE__,channel_int);

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0

	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)//for pro box
		if((channel_int>=36)&&(channel_int<=165))//channel 5g
		{
			if(Bandwidth_5g==1)//0 --> 20MHz, 1--> 40MHz)
			{						
				modify_line_realtek(buf,"ht_capab=","[SHORT-GI-20][SHORT-GI-40]");
			}
			else
			{
				for(chna=0;chna<30;chna++)
				{
					if(channel_int==channel_HT40_1[chna])
					{
						printf("[HT40+]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
					}
					else if(channel_int==channel_HT40_0[chna])
					{
						printf("[HT40-]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
					}
					else if (channel_int==channel_NA[chna])
					{
						printf("[HT40]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
					}
				}

			}
		}
		else if((channel_int>=1)&&(channel_int<=14))//channel 24g
#endif

		{
			if(Bandwidth_24g==1)//0 --> 20MHz, 1--> 40MHz)
			{						
				modify_line_realtek(buf,"ht_capab=","[SHORT-GI-20][SHORT-GI-40]");
			}
			else
			{
				for(chna=0;chna<30;chna++)
				{
					if(channel_int==channel_HT40_1[chna])
					{
						printf("[HT40+]channel_int=%d\n",channel_int);	
						modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
					}
					else if(channel_int==channel_HT40_0[chna])
					{
						printf("[HT40-]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
					}
					else if (channel_int==channel_NA[chna])
					{
						printf("[HT40]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
					}
				}
			}
		}
	
#else

	
	for(chna=0;chna<30;chna++){
	if(channel_int==channel_HT40_1[chna]){
		printf("[HT40+]channel_int=%d\n",channel_int);
		modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
	}
	else if(channel_int==channel_HT40_0[chna]){
		printf("[HT40-]channel_int=%d\n",channel_int);
		modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
	}
	else if (channel_int==channel_NA[chna]){
		printf("[HT40]channel_int=%d\n",channel_int);
		modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
		}
	}
#endif

	//if(channel_int>=1&&channel_int<=11)
		modify_line_realtek(buf,"channel=",channel);
	if(access("/sys/module/8192du",F_OK)==0 || 0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK)){
		if(rtl_band_use==RTL_BAND_5G){
			char *band_value="a";
			modify_line_realtek(buf,"channel=",channel);
			if(strstr(buf,"hw_mode=a")==NULL)
				modify_line_realtek(buf,"hw_mode=",band_value);
			printf("<%s %d>realtek 8192du use 5GHZ band\n",__func__,__LINE__);
		}else{
			char *band_value="g";
			if(strstr(buf,"hw_mode=g")==NULL)
				modify_line_realtek(buf,"hw_mode=",band_value);
			printf("<%s %d>realtek 8192du use 2.4GHZ band\n",__func__,__LINE__);
		}
	}else{
		char *band_value="g";
		if(strstr(buf,"hw_mode=g")==NULL)
			modify_line_realtek(buf,"hw_mode=",band_value);
		printf("<%s %d> realtek wifi use 2.4GHZ band\n",__func__,__LINE__);
	}
	if(buf!=NULL){
		fp = fopen(conf_file,"wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			return -1;
		}
		
		ret=fwrite(buf, 1, 4096, fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);

	}
	return 0;
	
}

int set_concurrent_mode_channel(int channel_int)   //it is just for setting 2.4G 5G wifi channel.
{
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	int band_type;
	char channel_temp[4]={0};
	char channel_str[5]={0};
	int chna=0;
	int Bandwidth_24g=0;
	int Bandwidth_5g=0;
	char conf_file[100]="";
	char confbak_file[100]="";
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	Bandwidth_24g=ezCastGetNumConfig(CONFNAME_BANDWIDTH_24G);//for pro lan/dongle
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	//rtl_band_use=RTL_BAND_5G;
	//printf("Probox use 5G band !!!\n");
	if((channel_int>=36)&&(channel_int<=165)){
		Bandwidth_5g=ezCastGetNumConfig(CONFNAME_BANDWIDTH_5G);
		rtl_band_use=RTL_BAND_5G;
	}
	else if((channel_int>=1)&&(channel_int<=14)){
		rtl_band_use=RTL_BAND_24G;	
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
	}

#endif

	//printf("channel1111111111111111=======%s\n",channel);
	
	fp = fopen(conf_file,"r");
	if(fp == NULL){
				
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
	
       sprintf(channel_str,"%d",channel_int);
	printf("<%s %d>channel_int=%d channel_str=%s\n",__func__,__LINE__,channel_int,channel_str);
	/*
	if(channel_int<=5||channel_int>=36)
		modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
	else
		modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
	*/
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0

	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)//for pro box
		if((channel_int>=36)&&(channel_int<=165))//channel 5g
		{
			if(Bandwidth_5g==1)//1--> 20MHz, 0--> 40MHz)
			{						
				modify_line_realtek(buf,"ht_capab=","[SHORT-GI-20][SHORT-GI-40]");
			}
			else
			{
				for(chna=0;chna<30;chna++)
				{
					if(channel_int==channel_HT40_1[chna])
					{
						printf("[HT40+]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
					}
					else if(channel_int==channel_HT40_0[chna])
					{
						printf("[HT40-]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
					}
					else if (channel_int==channel_NA[chna])
					{
						printf("[HT40]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
					}
				}

			}
		}
		else if((channel_int>=1)&&(channel_int<=14))//channel 24g
#endif

		{
			if(Bandwidth_24g==1)//1--> 20MHz, 0--> 40MHz)
			{						
				modify_line_realtek(buf,"ht_capab=","[SHORT-GI-20][SHORT-GI-40]");
			}
			else
			{
				for(chna=0;chna<30;chna++)
				{
					if(channel_int==channel_HT40_1[chna])
					{
						printf("[HT40+]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
					}
					else if(channel_int==channel_HT40_0[chna])
					{
						printf("[HT40-]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
					}
					else if (channel_int==channel_NA[chna])
					{
						printf("[HT40]channel_int=%d\n",channel_int);
						modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
					}
				}
			}
		}
	
#else

	
	for(chna=0;chna<30;chna++){
	if(channel_int==channel_HT40_1[chna]){
		printf("[HT40+]channel_int=%d\n",channel_int);
		modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
	}
	else if(channel_int==channel_HT40_0[chna]){
		printf("[HT40-]channel_int=%d\n",channel_int);
		modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
	}
	else if (channel_int==channel_NA[chna]){
		printf("[HT40]channel_int=%d\n",channel_int);
		modify_line_realtek(buf,"ht_capab=","[HT40][SHORT-GI-20][SHORT-GI-40]");
		}
	}
#endif

	//if(channel_int>=1&&channel_int<=11)
		modify_line_realtek(buf,"channel=",channel_str);

	if(buf!=NULL){
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		fp = fopen(conf_file,"wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			return -1;
		}
		
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		ret=fwrite(buf, 1, 4096, fp);
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		ret=fflush(fp);
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		int fd_write = fileno(fp);
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		ret=fsync(fd_write);
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		ret=fclose(fp);
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

	}
	//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	return 0;
	
}

int  set_ignoressid(int type,char *hide_str)  //type:  set2.4G  5G ;  val :0 open  ,1 hide
{
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	int band_type;
	char channel_str[5]={0};
	int chna=0;
	char conf_file[100]="";
	char confbak_file[100]="";
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
       if(type==RTL_BAND_24G)
       {
		sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
		sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
       }
#endif
	//printf("set_hide_ssid=======type=%d val=%d\n",type,val);
	
	fp = fopen(conf_file,"r");
	if(fp == NULL){
				
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
	
      // sprintf(hide_str,"%d",val);
	//printf("<%s %d>hide_str=%s\n",__func__,__LINE__,hide_str);
	
	modify_line_realtek(buf,"ignore_broadcast_ssid=",hide_str);

	if(buf!=NULL){
		fp = fopen(conf_file,"wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
			return -1;
		}
		
		ret=fwrite(buf, 1, 4096, fp);
		ret=fflush(fp);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		ret=fclose(fp);

	}
	return 0;
	
}
 int Store_WifiWpaConfigFile(int dongle_type)
{
	int ret = -1;
	char call_buf[512]={0x00};
	int fd_tmp,fd_etc;
	if(access("/tmp/wireless-tkip.conf",F_OK)!=0){

		if(dongle_type==REALTEK_dongle){
			fd_etc=open("/etc/wireless-tkip.conf",O_RDONLY);
			if(fd_etc<0){
				printf("[%s %d] open </etc/wireless-tkip.conf> error\n",__func__,__LINE__);
				goto flag;
			}
			printf("[debug %s %d] fd_etc:%d\n",__func__,__LINE__,fd_etc);
			fd_tmp=open("/tmp/wireless-tkip.conf",O_RDWR|O_CREAT);
			if(fd_tmp<0){
				close(fd_etc);
				printf("[%s %d] open </tmp/wireless-tkip.conf> error\n",__func__,__LINE__);
				goto flag;
			}
			printf("[debug %s %d] fd_tmp:%d\n",__func__,__LINE__,fd_tmp);
			size_t num;
			while((num=read(fd_etc,call_buf,512))>0){
				//printf("[debug %s %d] num:%d\n",__func__,__LINE__,num);
				write(fd_tmp,call_buf,num);
			}
			close(fd_etc);
			close(fd_tmp);
		}else{
			sprintf(call_buf,"ctrl_interface=/tmp/wpa_supplicant");
			fd_tmp=open("/tmp/wireless-tkip.conf",O_RDWR|O_CREAT);
			if(fd_tmp<0){
				printf("[%s %d] open </tmp/wireless-tkip.conf> error\n",__func__,__LINE__);
				goto flag;
			}
			write(fd_tmp,call_buf,64);
			close(fd_tmp);
		}
	}
	return 0;
flag:
	memset(call_buf,0x00,512);
	sprintf(call_buf,"cp -dprf /etc/wireless-tkip.conf /tmp/wireless-tkip.conf");
	system(call_buf);
	return -1;
}

static int Remove_TmpWpaConfigfile()
{
	int ret=-1;
	ret=remove("/tmp/wireless-tkip");
	printf("[%s %d] ret:%d\n",__func__,__LINE__);
	return ret;
}

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075


int Reroute_for8189err()
{
		//system("rmmod 8189es.ko");
		//	system("modprobe /lib/modules/2.6.27.29/8189es.ko");
		//re bridge  wifi 
		//wifiBridgeProcess(10);
		return 0;
}
//probox use port 80 for thttpd 192.168.168.1

int wifi_8189es_init()
{
	char cmd[256]="";
	if(access("/sbin/hostapd_02",F_OK)!=0){
		sprintf(cmd,"%s","ln -s /sbin/hostapd /sbin/hostapd_02");
		system(cmd);
	}
	//if(access("/sbin/udhcpd_02",F_OK)!=0){
	//	sprintf(cmd,"%s","ln -s /sbin/udhcpd /sbin/udhcpd_02");
		//system(cmd);
	//}
	if(access("/sbin/wpa2_supplicant",F_OK)!=0){
		memset(cmd,0,256);
		sprintf(cmd,"%s","ln -s /sbin/wpa_supplicant /sbin/wpa2_supplicant");
		system(cmd);
	}
	
	
	insmod_8189es();
	system("ifconfig wlan3 up");//need to be done in wifi_softap_process.sh 
	change_wifi_softap_defalut_ssid(RTL_BAND_24G);//change SSID
	 int val=ezCastGetNumConfig(CONFNAME_SOFTAP_CHANNEL);
	if(val<=0)
		int_probox_mode_best_channel(RTL_BAND_24G);
	else
		set_concurrent_mode_channel(val);
	//after 8189 startup ,set back to 5g band
	rtl_band_use=RTL_BAND_5G;
	checkAndRecoverForConcurrent(RTL_BAND_24G);
	#if 0
	realtek_softap_func(DONGLE_IN,NULL);
	if(access("/tmp/hostapd_02",F_OK)!=0){
		realtek_softap_func(DONGLE_OUT,NULL);
		realtek_softap_func(DONGLE_IN,NULL);
	}

	#endif
	
	//wifidbg_err("8189es check wlan3  /etc/rtl_hostapd_o2.conf\n ");
	//system("cat /etc/rtl_hostapd_02.conf ");
	//modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
	//modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
	//wifi_check_remote_control_thread(0);
	return 0;
}



void rtk8189es_in(unsigned int vidpid)//0x148f8189
{	
	int ret=0;
	int val;
	char callbuf[50]={0};
	//vid_pid_from_driver=0x148f8189;
	vid_pid_from_driver=vidpid;
	//vid_pid_from_driver_8189es=vid_pid_from_driver;//save 8189 pid 
	//cur_port=2;
	fprintf(stderr,"function:%s line:%d vidpid=%d\n",__FUNCTION__,__LINE__,vidpid);
	val=ezCastGetNumConfig(CONFNAME_SOFTAP_CHANNEL);
	
	if(sem_init(&cur_dongle.lock,0,1)==-1){
		wifidbg_err("init dongle lock error!");
		cur_dongle.dongle = NULL;
		cur_dongle.status = DONGLE_ERR;
	}
	else{
		cur_dongle.dongle= match_dongle(vidpid);
	}
	
	cur_dongle.status = DONGLE_OK;
	
	dongle_type_divided=dongle_get_device_changing();
	//Store_WifiWpaConfigFile(dongle_type_divided);
	//change_wifi_softap_defalut_ssid();
	printf("dongle_type_divided=================%d\n",dongle_type_divided);
	if(dongle_type_divided==REALTEK_dongle){
		realtek_dongle_type_divided=get_realtek_dongle_type();
	if(val>0)
	{
		wifiChannel_set=0;
		set_concurrent_mode_channel(val);
	}
		printf("function:%s,line:%d,realtek_dongle_type_divided=============%d\n",__FUNCTION__,__LINE__,realtek_dongle_type_divided);
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		wifi_8189es_init();

		vid_pid_from_driver=vid_pid_from_driver_firstin;
		realtek_dongle_type_divided=get_realtek_dongle_type();
		//8189es start after 8821 ,after 8189es startup,set current device as 8821 for hot plug detect and scan wisi list

		wifiBridgeProcess(WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3);
		printf("--8189_init complete!!!--\n");
		Probox_led_init();
		return;
#else
		insmod_8189es();
		sprintf(callbuf,"ifconfig wlan2 up");
		system(callbuf);
		change_wifi_softap_defalut_ssid(RTL_BAND_24G);
#endif
	#if 0//test wifi direct and concurrent mode
		sprintf(callbuf,"iwpriv wlan0 dbg 0x7f230000 1");
		system(callbuf);

		sprintf(callbuf,"iwpriv wlan0 dbg 0x7f110000 1");
		system(callbuf);
	#endif
	}
	
	modify_wifi_mode(WIFI_DONGLE_PLUG_IN);
	
	//set current driver as  8821au,  use this pid to scan(user 5G scan)  and check wifi status
	
}
#endif

void dongle_in(unsigned int vidpid)
{
	
	int ret=0;
	char callbuf[50]={0};
	vid_pid_from_driver=vidpid;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

	vid_pid_from_driver_firstin=vid_pid_from_driver;//save 8821 first in pid
#endif
	//cur_port=0;
	fprintf(stderr,"function:%s line:%d vidpid=%d\n",__FUNCTION__,__LINE__,vidpid);
	
	//fprintf(stderr,"function:%s line:%d mode_change=%d\n",__FUNCTION__,__LINE__,mode_change);
	if(sem_init(&cur_dongle.lock,0,1)==-1){
		wifidbg_err("init dongle lock error!");
		cur_dongle.dongle = NULL;
		cur_dongle.status = DONGLE_ERR;
	}
	else{
		cur_dongle.dongle= match_dongle(vidpid);
	}
	
	cur_dongle.status = DONGLE_OK;
	
	dongle_type_divided=dongle_get_device_changing();
	//Store_WifiWpaConfigFile(dongle_type_divided);
	//change_wifi_softap_defalut_ssid();
	printf("dongle_type_divided=================%d\n",dongle_type_divided);
	if(dongle_type_divided==REALTEK_dongle){
		realtek_dongle_type_divided=get_realtek_dongle_type();
		printf("function:%s,line:%d,realtek_dongle_type_divided=============%d\n",__FUNCTION__,__LINE__,realtek_dongle_type_divided);
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		//system("insmod /lib/modules/2.6.27.29/8192du.ko rtw_channel_plan=0x12");
		//system("insmod /lib/modules/2.6.27.29/8192du.ko rtw_channel_plan=0x3");
		wifi_concurrent_mode_init();	
		return;
#else
		insmod_ko();
		sprintf(callbuf,"ifconfig wlan0 up");
		system(callbuf);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		change_wifi_softap_defalut_ssid(RTL_BAND_5G);
#else
		change_wifi_softap_defalut_ssid(RTL_BAND_24G);
#endif
#endif
		#if 0//test wifi direct and concurrent mode
		sprintf(callbuf,"iwpriv wlan0 dbg 0x7f230000 1");
		system(callbuf);

		sprintf(callbuf,"iwpriv wlan0 dbg 0x7f110000 1");
		system(callbuf);
		#endif
	}
	modify_wifi_mode(WIFI_DONGLE_PLUG_IN);
			
}

void set_rtl_band_use(int val){
	if(val == RTL_BAND_24G || val == RTL_BAND_5G){
		rtl_band_use = val;
		printf("[%s:%d] -- rtl_band_use: 0x%x\n", __func__, __LINE__, rtl_band_use);
	}else
		printf("<ERROR> [%s:%d] -- val error, val=%d\n", __func__, __LINE__, val);
}

int get_rtl_band_use(){
	return rtl_band_use;
}

int get_wifi_model_type(){
	return realtek_dongle_type_divided;
}

int  softap_start(){
	pac_system("ifconfig wlan1 0.0.0.0 up");
	int val= 0;
	#if EZCAST_ENABLE
	val = ezCastGetNumConfig(CONFNAME_SOFTAP_CHANNEL);
	#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	vid_pid_from_driver=vid_pid_from_driver_firstin;
	printf("vid_pid_from_driver=%d\n",vid_pid_from_driver);
	 int val_5g=ezCastGetNumConfig(CONFNAME_SOFTAP_5G_CHANNEL);
	 printf("softap_start 5G=%d\n",val_5g);  

	if(val>0)
		set_concurrent_mode_channel(val);
	else
		int_probox_mode_best_channel(RTL_BAND_24G);
		
	if(val_5g>0)
		set_concurrent_mode_channel(val_5g);
	else
		int_probox_mode_best_channel(RTL_BAND_5G);	
#else	
	if(val<=0)
		inti_concurrent_mode_best_channel();
	else
		set_concurrent_mode_channel(val);
	realtek_softap_func(DONGLE_IN,NULL);
#endif
	printf("---softap_start---------\n");	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	pac_system("sh /am7x/case/scripts/wifi_bridge_process.sh wifi_bridge_add_br0_and_wlan1_wlan3");
#endif
	modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
      wifi_check_remote_control_thread(0);
	return 0;
}

int  softap_start24G_probox(){
	printf("-----------------------softap_start24G_probox----------------------\n");
	pac_system("ifconfig wlan3 0.0.0.0 up");
	int val= 0;
	#if EZCAST_ENABLE
	val=ezCastGetNumConfig(CONFNAME_SOFTAP_CHANNEL);
	#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
       vid_pid_from_driver=0x148f8189;
	if(val>0)
		set_concurrent_mode_channel(val);
	else
		int_probox_mode_best_channel(RTL_BAND_24G);
		
#else	
	if(val<=0)
		inti_concurrent_mode_best_channel();
	else
		set_concurrent_mode_channel(val);
#endif
	printf("---softap_start---------\n");
	realtek_softap_func(DONGLE_IN,NULL);
	set_ap_close_flag(0);
	modify_wifi_mode(WIFI_CONCURRENT_SOFTAP_ENABLE);
      wifi_check_remote_control_thread(0);
#if WEBSETTING_ENABLE
	create_websetting_server();
	//if(ezCastGetNumConfig(CONFNAME_INTERNET_CONTROL)==1)
	//	Start_TcpSocket(0);
#endif
	return 0;
}


int dongle_get_device_changing()
{
	char device[50];
	int vidpid_tmp=vid_pid_from_driver;
	int dongle_tbl_length = sizeof(dongle_tbl)/sizeof(dongle_t);
	int i=0;
	for(i=0;i<dongle_tbl_length;i++){
		if(vidpid_tmp==dongle_tbl[i].vidpid){
			if(strcmp(dongle_tbl[i].device_name,"wlan0")==0 ||strcmp(dongle_tbl[i].device_name,"wlan2")==0)
				return REALTEK_dongle;
			if(strcmp(dongle_tbl[i].device_name,"ra0")==0)
				return RALINK_dongle;		
		}
	}
	return INIT_DONGLE_TYPE;
}
int get_realtek_dongle_type(){
	int vidpid_tmp=vid_pid_from_driver;
	int dongle_tbl_length = sizeof(dongle_tbl)/sizeof(dongle_t);
	int i=0;
	printf("function:%s,line:%d,vidpid_tmp:0x%x,dongle_tbl_length:%d\n",__FUNCTION__,__LINE__,vidpid_tmp,dongle_tbl_length);
	for(i=0;i<dongle_tbl_length;i++){
		if(vidpid_tmp==dongle_tbl[i].vidpid){
			printf("dongle_tbl[i].driver_name=======%s\n",dongle_tbl[i].driver_name);
			if(strncmp(dongle_tbl[i].dev_rmmod,"8192cu",strlen("8192cu"))==0)
				return REALTEK_CU;
			if(strncmp(dongle_tbl[i].dev_rmmod,"8192du",strlen("8192du"))==0)
				return REALTEK_DU;		
			if(strncmp(dongle_tbl[i].dev_rmmod,"8188eu",strlen("8188eu"))==0)
				return REALTEK_8188EU;	
			if(strncmp(dongle_tbl[i].dev_rmmod,"8192eu",strlen("8192eu"))==0)
				return REALTEK_8192EU;
			if(strncmp(dongle_tbl[i].dev_rmmod,"8811au",strlen("8811au"))==0)
				return REALTEK_8811AU;
			if(strncmp(dongle_tbl[i].dev_rmmod,"8821au",strlen("8821au"))==0)
				return REALTEK_8811AU;
			if(strncmp(dongle_tbl[i].dev_rmmod,"8821cu",strlen("8821cu"))==0)
				return REALTEK_8821CU;
			if(strncmp(dongle_tbl[i].dev_rmmod,"8189es",strlen("8189es"))==0)
				return REALTEK_8189ES;
			if(strncmp(dongle_tbl[i].dev_rmmod,"8188fu",strlen("8188fu"))==0)
				return REALTEK_8188FU;
		}
	}
	return -1;

}

void dongle_out()
{	
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
	char *device = NULL;
	char call_buf[64] = {0};
	device = dongle_get_device();
	if(strcmp(device,"wlan0")==0){
		printf("functin:%s,line:%d\n",__FUNCTION__,__LINE__);
		sprintf(call_buf,"killall hostapd\nkillall udhcpd\nkillall wpa_supplicant");
		system(call_buf);
	}
	wifi_check_remote_control_thread(1);
#endif
	//printf("functin:%s,line:%d\n",__FUNCTION__,__LINE__);
	//Remove_TmpWpaConfigfile();
	if(read_wifi_mode()!=WIFI_MODE_CHANGEING_FOR_RALINK)
		modify_wifi_mode(WIFI_DONGLE_PLUG_OUT);	
}

void dongle_out_bak()
{
	int ret=0;
	char callbuf[50];
	if(vid_pid_from_driver==0x0bda8176||vid_pid_from_driver==0x0bda8178||vid_pid_from_driver==0x0bda8194||vid_pid_from_driver==0x0bda018a||vid_pid_from_driver==0x48550090){
		if(access("/sys/module/8192cu",F_OK)==0){
			printf("%s %d==================\n",__FUNCTION__,__LINE__);
			sprintf(callbuf,"rmmod 8192cu");
			system(callbuf);
		}
		else{
			printf("the 8192cu.ko is not exsit!");
		}
		
	}
	
	else if(vid_pid_from_driver==0x07b83071||vid_pid_from_driver==0x14b23c2b||vid_pid_from_driver==0x148f5370||vid_pid_from_driver==0x148f3072||vid_pid_from_driver==0x148f3070){
		int access_ok1=access("/sys/module/rtnet3070ap",F_OK);
		int access_ok2=access("/sys/module/rt5370sta",F_OK);//ralink new driver
		if(access_ok1==0){
				sprintf(callbuf,"modprobe -r rtnet3070ap");
				system(callbuf);
			}
	
		if(access_ok2==0){
				sprintf(callbuf,"modprobe -r rt5370sta");
				system(callbuf);
			}
		else{
			printf("the dongle is not exisit");
		}	

		
	}
	dongle_out_ok=1;
	dongle_in_ok=0;

}

char *dongle_get_device()
{
	int vidpid_tmp=vid_pid_from_driver;
	int i=0;
	int dongle_tbl_length = sizeof(dongle_tbl)/sizeof(dongle_t);
	if(vidpid_tmp==0)
		return NULL;
	printf("%s %d length:%d\n",__FUNCTION__,__LINE__,sizeof(dongle_tbl)/sizeof(dongle_t));
	for(i=0;i<dongle_tbl_length;i++){	
		if(vidpid_tmp==dongle_tbl[i].vidpid){

			return dongle_tbl[i].device_name;
		}
	}
	return NULL;
}

static char *dongle_get_driver()
{
	char *driver=NULL;
	if(cur_dongle.status==DONGLE_OK){
		wifi_sem_wait(&cur_dongle.lock);
		driver = get_driver(cur_dongle.dongle);
		wifi_sem_post(&cur_dongle.lock);
	}
	return driver;
}

static char *dongle_get_dev()
{
	char *dev_rmmod=NULL;
	if(cur_dongle.status==DONGLE_OK){
		wifi_sem_wait(&cur_dongle.lock);
		dev_rmmod = get_dev(cur_dongle.dongle);
		wifi_sem_post(&cur_dongle.lock);
	}
	return dev_rmmod;
}


int dongle_get_ctrl_iface(char*ctl_iface)
{
	char *device=NULL;
	device = dongle_get_device();
	//device = dongle_get_device();//wlan 0 wlan 1,wlan2 wlan3

	printf("--dongle_get_ctrl_iface device=%s",device);
	if(device==NULL){
		wifidbg_info("get dongle device error!");
		return -1;
	}
	else{
		sprintf(ctl_iface, "/tmp/wpa_supplicant/%s",device);
	}
	return 0;
}

#define DIR_FORWORD 		0
#define DIR_BACKWORD 	1
static int get_one_driver(char *one_driver,char *driver,int *pos,int direction)
{
	int i=0;
	int j=0;
	int length=0;
	if(!one_driver || !driver ||!pos)
		return -1;

	i=*pos;
	while(1){
		if(direction==DIR_FORWORD){
			if(driver[i]==0 || driver[i]==0x20){
				if(driver[i]==0x20)
					*pos = i+1;
				else
					*pos = i;
				break;
			}
			else{
				one_driver[j]=driver[i];
				j++;
			}
			i++;
		}
		else{
			if(i<0 || driver[i]==0x20){
				if(i<0)
					*pos = i;
				else
					*pos = i-1;
				break;
			}
			else{
				length++;
			}
			i--;
		}
	}
	if(direction==DIR_BACKWORD && length!=0){
		if(*pos<0)
			memcpy(one_driver,driver,length);
		else
			memcpy(one_driver,driver+*pos+2,length);
	}
	return 0;
}
static int  wifi_get_channel_region(char *rtl_chn)
{
	int ret = -1;
	//int fd = -1;
	struct sysconf_param sys_cfg_data;
	//char region_buf[32]={0x00},tmp_buf[64];
	dongle_type_divided=dongle_get_device_changing();
	if(dongle_type_divided==REALTEK_dongle){
		#if 0
		fd = open("/am7x/case/data/channel_region.bin",O_RDONLY);
		if(fd<0){
			printf("[%s %d] channel region open error\n",__func__,__LINE__);
			return ret;
		}
		lseek(fd,48,SEEK_SET);
		read(fd,region_buf,16);
		//printf("debug %s %d ============= region_buf:%s\n",__func__,__LINE__,region_buf);
		sprintf(rtl_chn,"%s",region_buf);
		//printf("debug %s %d ============= region_buf:%s\n",__func__,__LINE__,rtl_chn);
		close(fd);
		#else
		_get_env_data(&sys_cfg_data);
		if(strlen(sys_cfg_data.wifiChannelRegion)>0){
			printf("[%s] wifiChannelRegion:%s\n",__func__,sys_cfg_data.wifiChannelRegion);
			memcpy((void *)rtl_chn,(void *)sys_cfg_data.wifiChannelRegion,strlen(sys_cfg_data.wifiChannelRegion));
			memcpy((void *)Rtl_ChannelRegion,(void *)rtl_chn,strlen(rtl_chn));
		}else{
			printf("[%s] vram not store channel region\n",__func__);
			memcpy((void *)rtl_chn,(void *)Rtl_ChannelRegion,strlen(Rtl_ChannelRegion));
		}
		#endif
		ret=REALTEK_dongle;
	}

	if(dongle_type_divided==RALINK_dongle){	
		ret= RALINK_dongle;
	}

	return ret;
}
static int realtek_get_channel_plan(char *buf_tmp)
{
	int i, channel_plan=-1;
	int channel_region_len=sizeof(channel_region_tbl)/sizeof(channel_region_t);
	for(i=0;i<channel_region_len;i++){
		if(strcmp(buf_tmp,channel_region_tbl[i].region_name)==0){
			channel_region_index = i;
			channel_plan=channel_region_tbl[i].region_code;
			channel_plan_value=channel_plan;
			channel_total_2g=channel_region_tbl[i].total_channel_2g;
			channel_total_5g=channel_region_tbl[i].total_channel_5g;
			printf("[%s %d]=channel_plan:%02x channel_2g_total:%d channel_5g_total:%d\n",__func__,__LINE__,channel_plan,channel_total_2g,channel_total_5g);
			break;
		}
	}
	//printf("[%s %d]===========channel_plan:%02x\n",__func__,__LINE__,channel_plan);
	return channel_plan;
}
int insmod_ko()
{
	char one_driver[64];
	char callbuf[WIFI_BUF_LEN]={0};
	char *driver=NULL;
	int pos=0,len=0;
	int ret=0;
	fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

	driver = dongle_get_driver();
	if(driver){
		len = strlen(driver);
		while(1){
			memset(one_driver,0,64);
			get_one_driver(one_driver,driver,&pos,DIR_FORWORD);
			printf("koname=%s,pos=%d\n",one_driver,pos);
			if(strcmp(one_driver,"8192du.ko")==0||strcmp(one_driver,"8192cu.ko")==0||strcmp(one_driver,"8188eu.ko")==0){
				char buf_region[32]={0x00};
				wifi_get_channel_region(buf_region);
				//printf("[%s %d]==:%s Rtl_ChannelRegion:%s\n",__func__,__LINE__,buf_region,Rtl_ChannelRegion);
				int region_code=realtek_get_channel_plan(buf_region);
				/*
				*if custom want to Certificate,we should add some paraments into wifi driver rtw_wifi_spec=1;
				*/
				sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x%x  ",one_driver,region_code);
//				printf("[%s %d]============call_buf:%s\n",__func__,__LINE__,callbuf);
			}
            else if(strcmp(one_driver,"8821au.ko")==0){
//                sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x8  ",one_driver);
//                sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x0c  ",one_driver);
                sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x13 rtw_vht_enable=2 ",one_driver);
                printf("[%s:%d] 2015/6/29 11:00\r\n",  __func__, __LINE__);
            }
			else if(strcmp(one_driver,"8821cu.ko")==0){
//                sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x8  ",one_driver);
//                sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x0c  ",one_driver);
                sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x13 rtw_vht_enable=2 ",one_driver);
                printf("[%s:%d] 2015/6/29 11:00\r\n",  __func__, __LINE__);
            }
            else{
				sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s",one_driver);
			}
			ret = system(callbuf);
			
			if(ret!=0){///理论上insmod_ko不能被调用两次，如果插入dongle的时候就自动调用insmod，但wifi ap再次调用就会失败，所以这个地方暂时默认它全部返回成功
				ret = 0;
				//return -1;
			}
			if(pos==len){
				break;
			}
			
		}
	}
	else
		return -1;
	return ret;
}
int insmod_8189es()
{	
	char one_driver[64];
	char callbuf[WIFI_BUF_LEN]={0};
	char *driver=NULL;
	int pos=0,len=0,err=0;
	int ret=0;
	char cmd[256]="";
	fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
	
	driver = dongle_get_driver();
	if(driver){
		len = strlen(driver);
		while(1){
			memset(one_driver,0,64);
			get_one_driver(one_driver,driver,&pos,0);
			printf("koname=%s,pos=%d\n",one_driver,pos);
			printf("Load Drivers for probox SDIO!!!\n");
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"%s%s/%s","modprobe  ",AM_SYS_LIB_DIR,"mmc_core.ko");
			err=system(cmd);
			if(err<0)  printf("add SDIO driver [mmc_core.ko] error!!!\n");
			
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"%s%s/%s","modprobe  ",AM_SYS_LIB_DIR,"actions_mci.ko");
			err=system(cmd);
			if(err<0)  printf("add SDIO driver  [actions_mci.ko] error!!!\n");
			
			memset(cmd,0,sizeof(cmd));
			sprintf(cmd,"%s%s/%s","modprobe  ",AM_SYS_LIB_DIR,"8189es.ko");
			err=system(cmd);
			if(err<0)  printf("add SDIO driver  [8189es.ko] error!!!\n");
			/*
			if(strcmp(one_driver,"8192du.ko")==0||strcmp(one_driver,"8192cu.ko")==0||strcmp(one_driver,"8188eu.ko")==0 ){
				char buf_region[32]={0x00};
				wifi_get_channel_region(buf_region);
				//printf("[%s %d]==:%s Rtl_ChannelRegion:%s\n",__func__,__LINE__,buf_region,Rtl_ChannelRegion);
				int region_code=realtek_get_channel_plan(buf_region);
				
				sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x%x  ",one_driver,region_code);
//				printf("[%s %d]============call_buf:%s\n",__func__,__LINE__,callbuf);
			}
			else if(strcmp(one_driver,"8821au.ko")==0){
//				  sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s	rtw_channel_plan=0x8  ",one_driver);
//				  sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s	rtw_channel_plan=0x0c  ",one_driver);
				sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s  rtw_channel_plan=0x13  ",one_driver);
				printf("[%s:%d] 2015/6/29 11:00\r\n",  __func__, __LINE__);
			}
			else{
				sprintf(callbuf,"modprobe  /lib/modules/2.6.27.29/%s",one_driver);
			}
			ret = system(callbuf);
			
			if(ret!=0){///理论上insmod_ko不能被调用两次，如果插入dongle的时候就自动调用insmod，但wifi ap再次调用就会失败，所以这个地方暂时默认它全部返回成功
				ret = 0;
				//return -1;
			}
			*/
			
			if(pos==len){
				break;
			}
			
		}
	}
	else
		return -1;
	return ret;
}
int rmmod_ko()
{
	char callbuf[WIFI_BUF_LEN]={0};
	char one_driver[64];
	char *driver=NULL;
	int pos=0,ret=0,len=0;
	fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);

	if(mode_change==1){
		if(cur_dongle.dongle==dongle_tbl)
			cur_dongle.dongle==dongle_softap_tbl;
		else if(cur_dongle.dongle==dongle_softap_tbl)
			cur_dongle.dongle==dongle_tbl;
		}
	driver = dongle_get_dev();
	if(driver){
		len = strlen(driver);
		while(1){
			memset(one_driver,0,64);
			get_one_driver(one_driver,driver,&pos,DIR_FORWORD);
			printf("devname=%s,pos=%d\n",one_driver,pos);
			sprintf(callbuf,"rmmod %s",one_driver);
			ret = system(callbuf);
			if(ret!=0)
				return -1;
			if(pos==len){
				break;
			}
		}

	}
	else
		return -1;
	return ret;
}


int handle_hcd_ko(int dir)
{
	int ret=0;
	char callbuf[WIFI_BUF_LEN]={0};
	if(dir==0){
		hcd_mannual=1;
		(id_hcd_ko==HCD_USE_WIFI)?sprintf(callbuf,"rmmod %s","am7x_hcd"):sprintf(callbuf,"rmmod %s","am7x_hcd_next");
	//	printf("<%s %d>=====callbuf=%s\n",__FUNCTION__,__LINE__,callbuf);
	//	sprintf(callbuf,"rmmod %s","am7x_hcd_next");
		ret = system(callbuf);
	}
	else{
		
		hcd_mannual=1;
		(id_hcd_ko==HCD_USE_WIFI)?sprintf(callbuf,"insmod /lib/modules/2.6.27.29/%s","am7x_hcd.ko"):sprintf(callbuf,"insmod /lib/modules/2.6.27.29/%s","am7x_hcd_next.ko");
	//	printf("<%s %d>=====callbuf=%s\n",__FUNCTION__,__LINE__,callbuf);
	//	sprintf(callbuf,"insmod /lib/modules/2.6.27.29/%s","am7x_hcd_next.ko");
		ret = system(callbuf);
	}
	return ret;
}

int switch_mode_sta2softap()
{
	if(switch_mode_active==1){
		switch_mode_active = 0;
		handle_hcd_ko(0); ///hcd 的卸载和再次装载是必须的
		sleep(3);///适当的延迟是必须的
		handle_hcd_ko(1);
		dongle_connect_type = mode_data.connect_type;
		memset(&mode_data,0,sizeof(struct mode_data_s));
	}
	return 0;
}

int switch_mode_softap2sta()
{
	if(switch_mode_active==1){
		switch_mode_active = 0;
		handle_hcd_ko(0);///hcd 的卸载和再次装载是必须的
		sleep(2);///适当的延迟是必须的
		dongle_connect_type = mode_data.connect_type;
		cur_dongle.dongle= match_dongle(mode_data.dongle_pidvid);
		handle_hcd_ko(1);
		sleep(2);///适当的延迟是必须的
		insmod_ko();
		handle_dongle(DONGLE_IN);
		memset(&mode_data,0,sizeof(struct mode_data_s));
	}
	return 0;
}



int process_wifi_function_switch(int switch_operation)
{

	unsigned int pidvid=0;
	int ret=0;
	int realtek_softap_ok=-1;
	int realtek_clientap_ok=-1;
	char scripts_url[128]={0};
	char callbuf[256];
	unsigned int vidpid;

	
	printf("<%s %d>===========new_connect_type=%d\n",__FUNCTION__,__LINE__,switch_operation);
	mode_change=1;
	vidpid=vid_pid_from_driver;
	printf("vidpid======0x%x\n",vidpid);
	printf("<%s %d>===========dongle_type_divided=%d\n",__FUNCTION__,__LINE__,dongle_type_divided);
	
	if(dongle_type_divided==REALTEK_dongle){//for realtek
		if(switch_operation==SWITCH_TO_CLIENT){
			printf("transfer SOFTAP to CONNECT_TYPE_STA\n");
			realtek_softap_func(DONGLE_OUT,NULL);
			
		}
		else if(switch_operation==SWITCH_TO_SOFTAP){
			printf("transfer CONNECT_TYPE_STA to SOFTAP\n");
			realtek_softap_func(DONGLE_IN,NULL);
			dongle_connect_type=switch_operation;
			
			modify_wifi_mode(WIFI_SOFTAP_ENABLE);
		}
		else if(switch_operation==CLOSE_WIFI_PROCESS){//used to close softap
				ret=realtek_softap_func(DONGLE_OUT,NULL);
				
				dongle_connect_type=switch_operation;

			}
		else if(switch_operation==RESTART_WIFI_PROCESS){
			ret=realtek_softap_func(DONGLE_OUT,NULL);
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
			int pid=vid_pid_from_driver;
			//ret=realtek_softap_func(DONGLE_IN,NULL);
			//probox restart another wifi  
			if(vid_pid_from_driver==0x148f8189)
				vid_pid_from_driver=vid_pid_from_driver_firstin;
			else
				vid_pid_from_driver=0x148f8189;
			//ret=realtek_softap_func(DONGLE_IN,NULL);
			// rebridge wlan1 and wlan3
			wifiBridgeProcess(WIFI_BRIDGE_ADD_BR0_AND_WLAN1_WLAN3);
			vid_pid_from_driver=pid;
	#else
			ret=realtek_softap_func(DONGLE_IN,NULL);
	#endif

		}
	}	
		


	if(dongle_type_divided==RALINK_dongle){  //for ralink dongle
		if(switch_operation==SWITCH_TO_CLIENT){
			printf("<%s %d>transfer SOFTAP to CONNECT_TYPE_STA\n",__FUNCTION__,__LINE__);
			wifi_client_thread_is_running=0;
			dongle_connect_type=switch_operation;
			if(id_hcd_ko==HCD_USE_WIFI){
				sprintf(callbuf,"sh %s/%s  %s %d",AM_CASE_SC_DIR,"wifi_softap_process.sh","ralink_client_mode",HCD_USE_WIFI);
			}else{
				sprintf(callbuf,"sh %s/%s  %s %d",AM_CASE_SC_DIR,"wifi_softap_process.sh","ralink_client_mode",HCD_NEXT_USE_WIFI);
			}
			system(callbuf);
			wlan_current_status.current_status_value=WIFI_DISCONNECTED;
			identy_ra0_wifi_switch();//bq add for wifi mode switch 
			
		}
		else if(switch_operation==SWITCH_TO_SOFTAP){
			printf("transfer CONNECT_TYPE_STA to SOFTAP\n");
			wifi_client_thread_is_running=0;
			dongle_connect_type=switch_operation;
			if(id_hcd_ko==HCD_USE_WIFI){
				sprintf(callbuf,"sh %s/%s  %s %d",AM_CASE_SC_DIR,"wifi_softap_process.sh","ralink_softap_mode",HCD_USE_WIFI);
			}else{
				sprintf(callbuf,"sh %s/%s  %s %d",AM_CASE_SC_DIR,"wifi_softap_process.sh","ralink_softap_mode",HCD_NEXT_USE_WIFI);
			}
			system(callbuf);
			
			modify_wifi_mode(WIFI_SOFTAP_ENABLE);
			identy_ra0_wifi_switch();//bq add for wifi mode switch 
			}
		else if(switch_operation==CLOSE_WIFI_PROCESS){//used to close softap
			printf("close ralink process\n");
			if(id_hcd_ko==HCD_USE_WIFI){
				sprintf(callbuf,"sh %s/%s  %s %d",AM_CASE_SC_DIR,"wifi_softap_process.sh","ralink_close",HCD_USE_WIFI);
			}else{
				sprintf(callbuf,"sh %s/%s  %s %d",AM_CASE_SC_DIR,"wifi_softap_process.sh","ralink_close",HCD_NEXT_USE_WIFI);
			}
			system(callbuf);
			wifi_off=1;			
			dongle_connect_type=switch_operation;		
		}

	}

	return 0;
}

#define tmp2Lenth  256

int modify_line_realtek(char * buf, char * key, char * value)
{
 
 	char tmp[4096] = {0};
	char tmp2[tmp2Lenth] = {0};
	char * locate1 = NULL;
	char * locate2 = NULL;

	int len1 = 0;
	int len2 = 0;
	
	locate1 = strstr(buf,key);
	if(locate1 == NULL){
		printf("[%s/%d]:error,can't find the key\n",__FILE__,__LINE__);
		return -1;
		
	}
	locate2 = strstr(locate1,"\n");
	
	strncpy(tmp,buf,strlen(buf)-strlen(locate1));

	bzero(tmp2,tmp2Lenth);
	snprintf(tmp2,tmp2Lenth,"%s%s",key,value);
	//printf("tmp2 ===== %s\n",tmp2);

	len1 = strlen(buf)-strlen(locate1);
	len2 = strlen(tmp2);
	
	//printf("len1 ===== %d\n",len1);
	//printf("len2 ===== %d\n",len2);

	if(len1 < 0 || len2 < 0){
		printf("[%s/%d]:error,len1 < 0 || len2 < 0\n",__FILE__,__LINE__);
		return -1;
	}
	
	strncpy(tmp+len1,tmp2,len2);
	strncpy(tmp+len1+len2,locate2,strlen(locate2));
	
	bzero(buf , strlen(buf));
	strncpy(buf,tmp,strlen(tmp));
	
	return 0;
}

int modify_line(char * buf, char * key, char * value)
{
	char tmp[4096] = {0};
	char tmp2[256] = {0};
	char * locate1 = NULL;
	locate1 = strstr(buf,key);
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	memcpy(tmp,buf,strlen(buf)-strlen(locate1));

	if(strcmp(key,"SSID=")==0){
		sprintf(tmp2,"SSID=%s",value);
	}
	else if(strcmp(key,"AuthMode=")==0){
		sprintf(tmp2,"AuthMode=%s",value);
	}
	else if(strcmp(key,"EncrypType=")==0){
		sprintf(tmp2,"EncrypType=%s",value);
	}
	else if(strcmp(key,"IEEE8021X=")==0){
		sprintf(tmp2,"IEEE8021X=%s",value);
	}
	else if(strcmp(key,"Key1Type=")==0){
		sprintf(tmp2,"Key1Type=%s",value);
	}
	else if(strcmp(key,"Key1Str=")==0){
		sprintf(tmp2,"Key1Str=%s",value);
	}
	else if(strcmp(key,"DefaultKeyID=")==0){
		sprintf(tmp2,"DefaultKeyID=%s",value);
	}
	else if(strcmp(key,"IEEE8021X=")==0){
		sprintf(tmp2,"IEEE8021X=%s",value);
	}
	else if(strcmp(key,"WPAPSK=")==0){
		sprintf(tmp2,"WPAPSK=%s",value);
	}
	else if(strcmp(key,"Channel=")==0){
		sprintf(tmp2,"Channel=%s",value);
	}

	if(tmp2==NULL)
		return -1;
	int len1 = strlen(buf)-strlen(locate1);
	int len2 = strlen(tmp2);
	memcpy(tmp+len1,tmp2,len2);
	memcpy(tmp+len1+len2,locate2,strlen(locate2));
	memset(buf,0,strlen(buf));
	memcpy(buf,tmp,strlen(tmp));
	return 0;
}

	
static int save_open(char * buf)
{
	modify_line(buf,"AuthMode=","OPEN");
	modify_line(buf,"EncrypType=","NONE");
	modify_line(buf,"IEEE8021X=","0");
	modify_line(buf,"SSID=",ssid);
	modify_line(buf,"Channel=",channel);
	modify_line(buf,"WPAPSK=","               ");

	return 0;
}
static int save_open_realtek(char * buf)
{
	
	int channel_int=0;
	char channel_temp[4]={0x00};
	printf("channel=============%s\n",channel);
	channel_int=atoi(channel);
	printf("channel_int=======%d\n",channel_int);
	
	if(access("/sys/module/8192du",F_OK)!=0 && 0!=access("/sys/module/8821au",F_OK) || 0!=access("/sys/module/8821cu",F_OK)){
		if(channel_int<=5)
			modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
		else
		modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
	}else{
		if(rtl_band_use==RTL_BAND_24G){ 
			/*rtl 8192du use 2.4G band*/
			if(channel_int<=5)
				modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
			else
				modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
		}else{
			/*rtl 8192du use 5G band*/
			best_channel_setting(channel_temp,RTL_BAND_5G); //again obtain rtl8192du best channel;
			channel=channel_temp;
			modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
			printf("<%s %d> realtek 8192du channel:%s\n",__func__,__LINE__,channel);
		}
	}
	modify_line_realtek(buf,"channel=",channel);
	modify_line_realtek(buf,"ssid=",ssid);
	modify_line_realtek(buf,"wpa=","0");

	modify_line_realtek(buf,"wpa_passphrase=","           ");
	return 0;
}
static int save_wep(char * buf)
{
	modify_line(buf,"AuthMode=","OPEN");
	modify_line(buf,"EncrypType=","WEP");
	modify_line(buf,"IEEE8021X=","0");
	if(strlen(first_key)==5 || strlen(first_key)==13){
		modify_line(buf,"Key1Type=","1");
	}
	else if(strlen(first_key)==10 || strlen(first_key)==26){
		modify_line(buf,"Key1Type=","0");
	}
	modify_line(buf,"Key1Str=",first_key);
	modify_line(buf,"DefaultKeyID=","1");
	modify_line(buf,"SSID=",ssid);
	modify_line(buf,"Channel=",channel);

	return 0;
}

static int save_wpa(char * buf)
{
	modify_line(buf,"AuthMode=","WPAPSK");

	modify_line(buf,"EncrypType=","TKIP");

	modify_line(buf,"IEEE8021X=","0");

	modify_line(buf,"WPAPSK=",first_key);

	modify_line(buf,"SSID=",ssid);
	
	modify_line(buf,"Channel=",channel);


	return 0;
}

static int save_wpa2(char * buf)
{
	modify_line(buf,"AuthMode=","WPA2PSK");
	modify_line(buf,"EncrypType=","AES");
	modify_line(buf,"IEEE8021X=","0");
	modify_line(buf,"WPAPSK=",first_key);
	modify_line(buf,"SSID=",ssid);
	modify_line(buf,"Channel=",channel);

	return 0;
}

static int save_wpa_realtek(char * buf)
{
	char wpa_passphrase[128];
	int channel_int=0;
	char channel_temp[4]={0x00};
	channel_int=atoi(channel);
	//printf("<%s %d>channel_int=%d\n",channel_int);
	if(access("/sys/module/8192du",F_OK)!=0 && 0!=access("/sys/module/8821au",F_OK) && 0!=access("/sys/module/8821cu",F_OK)){
	if(channel_int<=5)
		modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
	else
		modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
	}else{
		if(rtl_band_use==RTL_BAND_24G){
			/*rtl 8192du use 2.4G band*/
			if(channel_int<=5)
				modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
			else
				modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
		}else{
			/*rtl 8192du use 5G band*/
			best_channel_setting(channel_temp,RTL_BAND_5G); //again obtain rtl8192du best channel;
			channel=channel_temp;
			modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
			printf("<%s %d> realtek 8192du channel:%s\n",__func__,__LINE__,channel);
		}
	}
	modify_line_realtek(buf,"channel=",channel);
	
	modify_line_realtek(buf,"ssid=",ssid);
	modify_line_realtek(buf,"wpa=","1");

	modify_line_realtek(buf,"wpa_passphrase=",first_key);



	return 0;
}


static int save_wpa2_realtek(char * buf)
{
	char wpa_passphrase[128];
	int channel_int=0;
	char channel_temp[4]={0x00};
	channel_int=atoi(channel);
	printf("channel_int=======%d\n",channel_int);
	if(access("/sys/module/8192du",F_OK)!=0 && 0!=access("/sys/module/8821au",F_OK) && 0!=access("/sys/module/8821cu",F_OK)){
	if(channel_int<=5)
		modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
	else
		modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
	}else{
		if(rtl_band_use==RTL_BAND_24G){
			/*rtl 8192du use 2.4G band*/
			if(channel_int<=5)
				modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
			else
				modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
		}else{
			/*rtl 8192du use 5G band*/
			best_channel_setting(channel_temp,RTL_BAND_5G); //again obtain rtl8192du best channel;
			channel=channel_temp;
			modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
			printf("<%s %d> realtek 8192du channel:%s\n",__func__,__LINE__,channel);
		}
	}
	modify_line_realtek(buf,"channel=",channel);
	modify_line_realtek(buf,"ssid=",ssid);
	modify_line_realtek(buf,"wpa=","2");

	modify_line_realtek(buf,"wpa_passphrase=",first_key);



	return 0;
}




static int save_ap_info(int secrurity_mode)
{
	FILE *fp = NULL;
	char buf[4096] ={0};
	char buf_8189[4096] ={0};
	char callbuf[128];
    char ssid_buf[32]={0};
	int vidpid=0;
	int ret=-1;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	vid_pid_from_driver=vid_pid_from_driver_firstin;
	rtl_band_use=RTL_BAND_5G;
#endif

	vidpid=dongle_get_pidvid();
	printf("%s %d channel===%s,ssid====%s,psk======%s\n",__FUNCTION__,__LINE__,channel,ssid,first_key);
	if(dongle_type_divided==REALTEK_dongle){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		printf("%s %d\n",__FUNCTION__,__LINE__);
		fp = fopen(HOSTAPD_CON_FILE,"r");
		//fp = fopen("/etc/rtl_hostapd_01.conf","r");
		//fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
	
		if(fp == NULL){
				
				sprintf(callbuf,"cp %s %s",HOSTAPD_BAK_CON_FILE,HOSTAPD_CON_FILE);
				system(callbuf);
				fp = fopen(HOSTAPD_CON_FILE,"r");
		}
#else
		fp = fopen("/mnt/user1/softap/rtl_hostapd.conf","r");
		//fp = fopen("/etc/rtl_hostapd.conf","r");
		//fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
	
		if(fp == NULL){			
				sprintf(callbuf,"cp /mnt/bak/rtl_hostapd.conf %s",HOSTAPD_CON_FILE);
				system(callbuf);
				fp = fopen("/mnt/user1/softap/rtl_hostapd.conf","r");
		}
#endif
		ret=fread(buf, 1, 4096, fp);
		ret=fclose(fp);
		//probox change psk of 8189 /mnt/user1/softap/rtl_hostapd_02.conf
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075

		fp = fopen("/mnt/user1/softap/rtl_hostapd_02.conf","r");
		if(fp == NULL){
				sprintf(callbuf,"cp /mnt/bak/rtl_hostapd_02.conf  /mnt/user1/softap/rtl_hostapd_02.conf");
				system(callbuf);
				fp = fopen("/mnt/user1/softap/rtl_hostapd_02.conf","r");
		}
		ret=fread(buf_8189, 1, 4096, fp);
		ret=fclose(fp);
		fp = fopen("/mnt/user1/softap/rtl_hostapd_02.conf","wb+");
		if(fp == NULL){
			fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
		}
		printf("%s %d new passphrase=%s  buf_8189[%s] \n",__FUNCTION__,__LINE__,first_key,buf_8189);
		modify_line_realtek(buf_8189,"wpa_passphrase=",first_key);
        modify_line_realtek(buf_8189,"ssid=", ssid);
		ret=fwrite(buf_8189, 1, 4096, fp);
		//fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

		ret=fflush(fp);
		//fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
		int fd_write = fileno(fp);
		ret=fsync(fd_write);
		//fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

		ret=fclose(fp);

		modify_line_realtek(buf,"wpa_passphrase=",first_key);
        sprintf(ssid_buf, "%s_5G", ssid);
        modify_line_realtek(buf,"ssid=", ssid_buf);
		

#else

		printf("secrurity_mode===========%d\n",secrurity_mode);
		switch(secrurity_mode){
		case 0:
			save_open_realtek(buf);
			break;
		case 1:
			save_wpa_realtek(buf);
			break;
		case 2:
			save_wpa2_realtek(buf);
			break;
		}	
#endif
		
		if(buf!=NULL){
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE			
			fp = fopen(HOSTAPD_CON_FILE,"wb+");
			//fp = fopen("/etc/rtl_hostapd_01.conf","wb+");
			if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);

					return -1;
			}
#else
			fp = fopen(HOSTAPD_CON_FILE,"wb+");
			//fp = fopen("/etc/rtl_hostapd.conf","wb+");
			if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);

					return -1;
			}
#endif
			ret=fwrite(buf, 1, 4096, fp);
			//fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

			ret=fflush(fp);
			//fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

			int fd_write = fileno(fp);
			ret=fsync(fd_write);
			//fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

			ret=fclose(fp);
			fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

		}
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
		//if((getNetRunningFlagByIoctl("br0") & (IFF_UP|IFF_RUNNING)) == (IFF_UP|IFF_RUNNING))

		
		
#if ROUTER_ONLY_ENABLE
		int oldVal = ezCastGetNumConfig(CONFNAME_CTLMODE);
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

#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE			
#ifdef MODULE_CONFIG_WIFI_BRIDGE_ENABLE	
		char *br0Ip = NULL;

		if( (br0Ip = getIpAddressByIoctl("br0")) && (strlen(br0Ip) > strlen("0.0.0.0")) ){		
			wifiBridgeProcess(WIFI_BRIDGE_ADD_WLAN1);
			free(br0Ip);
			br0Ip = NULL;
		}
#endif
#endif
	}
	else if(dongle_type_divided==RALINK_dongle){
		fp = fopen("/mnt/user1/softap/RT2870AP.dat","r");
		if(fp == NULL){
			sprintf(callbuf,"cp /mnt/bak/RT2870AP.dat  /mnt/user1/softap/RT2870AP.dat");
			system(callbuf);
			printf("read RT2870AP.dat open error!");
			return -1;
			}
		fread(buf, 1, 4096, fp);
		fclose(fp);
	
		switch(secrurity_mode){
		case 0:
			save_open(buf);
			break;
		case 1:
			save_wpa(buf);
			break;
		case 2:
			save_wpa2(buf);
			break;
		}

		if(buf!=NULL){
		fp = fopen("/mnt/user1/softap/RT2870AP.dat","wb+");
		if(fp == NULL){
			return -1;
		}
		fwrite(buf, 1, 4096, fp);
		fflush(fp);
		int fd_write = fileno(fp);
		fsync(fd_write);
		fclose(fp);
		
		}
	}
	return 0;

}

int first_time_read_rtlconf=0;
int first_time_read_rt2870ap=0;

#define bufLenth 4096

char * get_soft_ap_entry(int dongle_type,char *key){			//if the return is not NULL , must free it
	FILE *fp = NULL;
	char buf[bufLenth] = {0};
	char callbuf[128];
	//char tmp[4096] = {0};
	char *tmp = NULL;
	char tmp2[256] = {0};
	char * locate1 = NULL;
	int ret=-1;
	char ap_info_path[128]={0};
	char conf_file[100]="";
	char confbak_file[100]="";
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	if(proBoxUpgradeEnv()==0)//for ensure probox ota 5Gconf
	{
			if(vid_pid_from_driver!=0x148f8189){
				sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
				sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);
				}
			else {
				sprintf(conf_file,"%s","/mnt/user1/softap/rtl_hostapd_02.conf");
				sprintf(confbak_file,"%s","/mnt/bak/rtl_hostapd_02.conf");
			}
	}
#endif
	tmp = (char*)calloc(bufLenth,1);
	if(tmp == NULL){
		printf("[%s/%d]error:calloc failed!\n",__FILE__,__LINE__);
		return NULL;
	}
	if(dongle_type==1){
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
		#if 0
		fp = fopen(HOSTAPD_CON_FILE,"r");
		if(fp == NULL){
			sprintf(callbuf,"cp %s %s",HOSTAPD_BAK_CON_FILE,HOSTAPD_CON_FILE);
			system(callbuf);
			printf("read tl_realtek.conf open error!");
			return -1;
		}
		#endif
		if(strncmp(key,"channel=",sizeof("channel="))==0){
			
			memset(ap_info_path,0,128);
			if(realtek_dongle_type_divided==REALTEK_CU)
				strncpy(ap_info_path,"/proc/net/rtl819xC/wlan1/ap_info",sizeof("/proc/net/rtl819xC/wlan1/ap_info"));
			else if(realtek_dongle_type_divided==REALTEK_DU)
				strncpy(ap_info_path,"/proc/net/rtl819xD/wlan1/ap_info",sizeof("/proc/net/rtl819xD/wlan1/ap_info"));
			else if(realtek_dongle_type_divided==REALTEK_8188EU)
				strncpy(ap_info_path,"/proc/net/rtl8188eu/wlan1/ap_info",sizeof("/proc/net/rtl8188eu/wlan1/ap_info"));
			else if(realtek_dongle_type_divided==REALTEK_8192EU)
				strncpy(ap_info_path,"/proc/net/rtl8192eu/wlan1/ap_info",sizeof("/proc/net/rtl8192eu/wlan1/ap_info"));

			get_info_from_file(ap_info_path,"cur_channel=",",",tmp);
			return tmp;
		}
		else{//SSID
			fp = fopen(conf_file,"r");
			//printf("conf_file=%s\n",conf_file);
			if(fp == NULL){
				printf("[debug info:%s,%d]:open rtl_hostapd_0x.conf  error!---error num is %d\n",__FUNCTION__,__LINE__,errno);
				sprintf(callbuf,"cp %s  %s",confbak_file,conf_file);
				system(callbuf);	
				if(tmp){
					free(tmp);
					tmp = NULL;
				}
				return NULL;
			}
		}
		
#else		
		fp = fopen(conf_file,"r");
		if(fp == NULL){
			printf("[debug info:%s,%d]:open rtl_hostapd.conf  error!---error num is %d\n",__FUNCTION__,__LINE__,errno);
			sprintf(callbuf,"cp /mnt/bak/rtl_hostapd.conf  %s",conf_file);
			system(callbuf);
			if(tmp){
				free(tmp);
				tmp = NULL;
			}
			return NULL;
		}
#endif

	}
	else if(dongle_type==2){
		fp = fopen("/mnt/user1/softap/RT2870AP.dat","r");
		if(fp == NULL){
			sprintf(callbuf,"cp /mnt/bak/RT2870AP.dat  /mnt/user1/softap/RT2870AP.dat");
			system(callbuf);
			printf("read RT2870AP.dat open error!");
			if(tmp){
				free(tmp);
				tmp = NULL;
			}
			return NULL;
		}
	}
	fread(buf, 1, 4096, fp);
	fclose(fp);

	locate1 = strstr(buf,key);
	if(locate1==NULL){
		printf("cannot find the KEY!!\n");
		if(tmp){
			free(tmp);
			tmp = NULL;
		}
		return NULL;	
	}
	locate1=locate1+strlen(key);
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	memcpy(tmp,locate1,strlen(locate1)-strlen(locate2));
	return tmp;
}



static int exec_open()
{
	char callbuf[BUF_LENGTH]={0};
	int res = -1;
	
	
	sprintf(callbuf,"iwpriv ra0 set AuthMode=OPEN");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set EncrypType=NONE");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set IEEE8021X=0");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set SSID=%s",ssid);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}
	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set Channel=%s",channel);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}
	return 0;
}
static int exec_wep()
{
	char callbuf[BUF_LENGTH]={0};
	int res = -1;

	sprintf(callbuf,"iwpriv ra0 set AuthMode=OPEN");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set EncrypType=WEP");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set IEEE8021X=0");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set Key1=%s",first_key);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set DefaultKeyID=1");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set SSID=%s",ssid);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set Channel=%s",channel);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}
	return 0;
}

static int exec_wpa()
{
	char callbuf[BUF_LENGTH]={0};
	int res = -1;

	sprintf(callbuf,"iwpriv ra0 set AuthMode=WPAPSK");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set EncrypType=TKIP");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set IEEE8021X=0");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}
	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set SSID=%s",ssid);
	printf("the callbuf in line929 is %s\n",callbuf);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set WPAPSK=%s",first_key);
	printf("the callbuf in line966 is %s\n",callbuf);

	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set Channel=%s",channel);
	printf("the callbuf in line975 is %s\n",callbuf);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	return 0;
}

static int exec_wpa2()
{
	char callbuf[BUF_LENGTH]={0};
	int res = -1;
	sprintf(callbuf,"iwpriv ra0 set AuthMode=WPA2PSK");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set EncrypType=AES");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set IEEE8021X=0");
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set SSID=%s",ssid);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set WPAPSK=%s",first_key);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}

	memset(callbuf,0,BUF_LENGTH);
	sprintf(callbuf,"iwpriv ra0 set Channel=%s",channel);
	res = system(callbuf);
	if(res != 0) {
		return -1;
	}
	
	return 0;
}


static int exec_ap(security_mode)
{
	int vidpid=0;
	vidpid=dongle_get_pidvid();
	char callbuf[50];
	sprintf(callbuf,"chmod 777 /sbin/iwpriv");
	system(callbuf);
	if(dongle_type_divided==RALINK_dongle){// only ralink dongle can use iwpriv to setting configuration
	switch(security_mode){
		case 0:
			exec_open();
			break;
		case 1:
			exec_wpa();
			break;
		case 2:
			exec_wpa2();
			break;
	}
	
	}
	return 0;
}


void putdown_softap_info(char *softap_ssid,char *softap_psk,char *softap_channel){
	ssid=softap_ssid;
	first_key=softap_psk;
	channel=softap_channel;

}
void ModifySettings(int secrurity_mode)
{
	printf("ModifySettings***********************************\n");
	if(secrurity_mode!=0&&strlen(first_key)<8)
		first_key="11111111";
	int channel_temp_int=strtol(channel,NULL,10);
	if(channel_temp_int<1||channel_temp_int>11){
		char channel_temp[4]={0};
		memset(channel_temp,0,4);
		if(access("/sys/module/8192du",F_OK)==0 || 0==access("/sys/module/8821au",F_OK) || 0==access("/sys/module/8821cu",F_OK)){
			if(rtl_band_use==RTL_BAND_5G){
				best_channel_setting(channel_temp,RTL_BAND_5G);
				channel=channel_temp;
			}else{
				best_channel_setting(channel_temp,RTL_BAND_24G);
				channel=channel_temp;
			}
		}else{
			best_channel_setting(channel_temp,RTL_BAND_24G);
			channel=channel_temp;
		}
	}
		
#ifdef MODULE_CONFIG_WIFI_CONCURRENT_MODE
	secrurity_mode=2;
#endif
	exec_ap(secrurity_mode);
	printf("save_ap_info***********************************\n");
	save_ap_info(secrurity_mode);
}


