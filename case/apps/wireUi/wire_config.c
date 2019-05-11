#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "sys_cfg.h"
#include <sys/msg.h>
#include <sys_pmu.h>
#include "sys_gpio.h"
#include "ezcast_public.h"
#include "wire_osd.h"
#include "wire_ui.h"
#include "wire_config.h"
#include "wire_cJSON.h"
#include "wire_ota.h"
#include "wire_log.h"
#include <fcntl.h>
#if EZMUSIC_ENABLE
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../scripts/mconfig.h"
#endif
#endif
#define SSID_MAX_LEN		(128)
#define VERSION_MAX_LEN		(64)

typedef struct ezcast_msg_s{
	char	isSet;
	char 	broadcast_mac[13];
	char	broadcast_ssid[SSID_MAX_LEN];
	char 	broadcast_version[VERSION_MAX_LEN];
}ezcast_msg_t;

char * ota_get_local_version(char *local_version);
void showADQRcode();

#define WIRE_ICON_OSD_W (500)
#define WIRE_ICON_OSD_H	(190)
#define WIRE_OSD_W		(640)
#define WIRE_OSD_H		(360)

static void *p_heap = NULL;
static void* inst = NULL;
static char *buffer = NULL;
static int bg_flag = 0;
static int windowSet_flag = 0;
static bool plugFlag = true;
static int iosSetStatus = WIRE_IPHONE_SETUP_STOP;
static int iosMirrorStatus = WIRE_IPHONE_PLUG_STOP;
static int androidSetStatus = WIRE_ANDROID_SETUP_STOP;
static int androidAdbStatus = WIRE_ANDROID_PLUG_STOP;
static ezcast_msg_t ezcast_msg;
static int usbdev_out=0;
char gpio26 = 1;

void system_reboot(){
	printf("[%s][%d] -v2- reboot system!!!\n", __func__, __LINE__);
	system("reboot");
}

static int is_dir_exist(const char *dirPath)
{
	if(NULL == dirPath){
		WLOGE("dirPath is NULL.\n");
		return -1;
	}
	DIR *dir = opendir(dirPath);
	if(dir == NULL){
		WLOGE("dirPath[%s] is not exist.\n", dirPath);
		return 0;
	}
	closedir(dir);
	
	return 1;
}
int set_usb_out_flag(int stat)
{
	usbdev_out=stat;
}
int get_usb_out_flag(int stat)
{
	return usbdev_out;
}
int getWireSetupStatus()
{
	return iosSetStatus;
}

void setWireSetupStatus(int stat)
{
	iosSetStatus = stat;
}

int getWireMirrorStatus()
{
	return iosMirrorStatus;
}

void setWireMirrorStatus(int stat)
{
	iosMirrorStatus = stat;
}

bool getWirePlugFlag()
{
	return plugFlag;
}

void setWirePlugFlag(bool Flag)
{
	plugFlag = Flag;
}
int getAndroidSetupStatus()
{
	return androidSetStatus;
}

void setAndroidSetupStatus(int stat)
{
	androidSetStatus = stat;
}

int getAndroidAdbStatus()
{
	return androidAdbStatus;
}

void setAndroidAdbStatus(int stat)
{
	androidAdbStatus = stat;
}

static int setWirePlugModeDefault()
{
	FILE *fp = NULL;
	char val[32];

	int ret = is_dir_exist("/mnt/vram/ezcast/");
	if(ret < 0){
		WLOGE("error error.\n");
		return -1;
	}else if(ret == 0){
		system("mkdir /mnt/vram/ezcast/");
	}	

	fp = fopen("/mnt/vram/ezcast/PLUGPLAYMODE", "w");
	if(fp != NULL)
	{
		snprintf(val, sizeof(val), "PlugMode=%s\n", (EZWIRE_PLUGPLAY_DEFAULT==1)?"YES":"NO");
		fwrite(val, 1, strlen(val), fp);
		fclose(fp);
		sync();
		WLOGI("Set plug mode to default: %s\n", val);
	}
	#if (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)
	setAoaMode(0);
	#endif
	return EZWIRE_PLUGPLAY_DEFAULT;
}

int setWirePlugMode(int on_off)
{
	FILE *fp = NULL;
	char val[32];

	int ret = is_dir_exist("mkdir /mnt/vram/ezcast/");
	if(ret < 0){
		WLOGE("error error.\n");
		return -1;
	}else if(ret == 0){
		system("mkdir /mnt/vram/ezcast/");
	}	

	fp = fopen("/mnt/vram/ezcast/PLUGPLAYMODE", "w");
	if(fp != NULL)
	{
		snprintf(val, sizeof(val), "PlugMode=%s\n", (on_off==1)?"YES":"NO");
		fwrite(val, 1, strlen(val), fp);
		fclose(fp);
		sync();
		WLOGI("Set plug mode to default: %s\n", val);
	}
	return on_off;
}


int isWirePlugMode()
{
      
	#if  (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)
	FILE *fp = NULL;
	
	if(access("/mnt/vram/ezcast/PLUGPLAYMODE", F_OK) != 0)
	{
		return setWirePlugModeDefault();
	}
	else
	{
		fp = fopen("/mnt/vram/ezcast/PLUGPLAYMODE", "r");
		if(fp != NULL)
		{
			char buff[32];
			int ret = fread(buff, 1, sizeof(buff), fp);
			fclose(fp);
			if(ret > 0)
			{
				if(strncmp(buff, "PlugMode=YES", strlen("PlugMode=YES")) == 0)
					return 1;
			}
		}
	}

	return 0;
	#else

	if(plugFlag)
		return 1;
	else
		return 0;
	
	#endif
}

int setAoaMode(int val)
{
	int ret;
	
	if(val > 0)
	{
		if(access(AOA_DISABLE, F_OK) != 0)
		{
			int fd = open(AOA_DISABLE, O_CREAT);
			if(fd >= 0)
			{
				close(fd);
				sync();
			}
			else
			{
				EZCASTWARN("Create %s fail...\n");
				perror("ERROR");
			}
		}
	}
	else if(val == 0)
	{
		if(access(AOA_DISABLE, F_OK) == 0)
		{
			unlink(AOA_DISABLE);
			sync();
		}
	}
	
	if(access(AOA_DISABLE, F_OK) == 0)
		ret = 1;
	else
		ret = 0;
	
	if(val >= 0)
		setAdbNoAudio(ret);

	return ret;
}


static void _img_memset4(unsigned int * buffer,unsigned int color, int count){
 	int i; 
	if(((int)buffer&0x3)) 
		printf("memset4 error %x %x\n",buffer); 
	for(i=0;i<count;i++) 
		*(buffer+i)=color; 
}
#define REDDOT_PATH "/mnt/vram/ezcast/REDOTNUM"
int setRedDotNum(int num,int pic)
{
       char val[64];
	FILE *fp = NULL;
	fp = fopen(REDDOT_PATH, "w");
	if(fp != NULL)
	{
		snprintf(val, sizeof(val), "count=%d,pic=%d\n", num,pic);
		fwrite(val, 1, strlen(val), fp);
		fclose(fp);
		sync();
		WLOGI("setRedDotNum: %s\n", val);
	}
	return 0;

}
int readRedDotNum(int *count,int *pic)
{
	FILE *fp = NULL;
	int next_count=0, next_pic=1;
	int ret = 0, len=0;
	ret=is_dir_exist("/mnt/vram/ezcast/");
	if(ret < 0){
		WLOGE("error error.\n");
		return -1;
	}else if(ret == 0){
		system("mkdir /mnt/vram/ezcast/");
	}	

	if(access(REDDOT_PATH,F_OK)!=0)
	{
		
		next_count++;
		*count=0;
		*pic=1;
		setRedDotNum(next_count,next_pic);	
		
	}
	else
	{
	      	char buff[64];
		char *str1=NULL;
		char *str2=NULL;
		fp = fopen(REDDOT_PATH, "r");
	       ret = fread(buff, 1, sizeof(buff), fp);
		if(ret>0)
		{
			
			str1=strstr(buff,"count=");
			if(str1!=NULL)
			{
				len=strlen("count=");
				*count=*(str1+len)-'0';
				next_count=*count;
				if(*count==2)
				{
              			next_count=0;
				}
				else
					next_count++;
			}
			else
				printf("read %s  error \n",REDDOT_PATH);
			str2=strstr(buff,"pic=");
			if(str2!=NULL)
			{
				len=strlen("pic=");
				*pic=*(str2+len)-'0';
				next_pic=*pic;
				if(*count==2)
				{
					if(*pic==3)
					{
              				next_pic=1;
					}
					else
						next_pic++;
				}
			}
			else
				printf("read %s  error \n",REDDOT_PATH);
			fclose(fp);
			setRedDotNum(next_count,next_pic);	
			
		}
		else
		{
			printf("read %s  error \n",REDDOT_PATH);
		}
		
		
	}
	return 0;
}

int  showRedDot()
{
	int num=0,pic=0;
	readRedDotNum(&num,&pic);
	printf("To show number:%d  pic=%d \n",num,pic);
	if(num==2)
	{
		char path[64]={0};
		sprintf(path,"/am7x/case/data/red_dot_0%d.jpg",pic);
		wireUI_LoadPic(path, WIRE_UI_REDDOT_X, WIRE_UI_REDDOT_Y, 24, 24);
	}
	return 0;
}
int ezwireMainUiStart()
{
	return wireUI_Init();
}
int delete_warn()
{
	wireUI_LoadPic(WIRE_UI_WARN_EMPTY, WIRE_UI_WARN_X, WIRE_UI_WARN_Y, WIRE_UI_WARN_WIDTH, WIRE_UI_WARN_HEIGHT);

}
int draw_warn_aoa()
{
	wireUI_LoadPic(WIRE_UI_WARN_AOA, WIRE_UI_WARN_X, WIRE_UI_WARN_Y, WIRE_UI_WARN_WIDTH, WIRE_UI_WARN_HEIGHT);

}
int draw_warn_adb()
{
	wireUI_LoadPic(WIRE_UI_WARN_ADB, WIRE_UI_WARN_X, WIRE_UI_WARN_Y, WIRE_UI_WARN_WIDTH, WIRE_UI_WARN_HEIGHT);

}
int ezwireDrawDefaultBg()
{
	int ret;
	#if EZWIRE_TYPE==MIRAPLUG//8M snor
	
	#if defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE!=0
	ret = wireUI_LoadPic(WIRE_UI_BG, WIRE_UI_BG_X, WIRE_UI_BG_Y, WIRE_UI_BG_WIDTH, WIRE_UI_BG_HEIGHT);
	#else
	ret = wireUI_LoadPic(WIRE_UI_MIRAPLUG_BG, WIRE_UI_BG_X, WIRE_UI_BG_Y, WIRE_UI_BG_WIDTH, WIRE_UI_BG_HEIGHT);
	#endif
	if(ret < 0){
		WLOGE("loadPic failed!\n");
		return -1;
	}
	
	if(isWirePlugMode())
	{
		setWirePlugFlag(true);
		#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		wireUI_LoadPic(WIRE_UI_PLUG_ON, WIRE_UI_PLUGMODE_X, WIRE_UI_PLUGMODE_Y, WIRE_UI_PLUGMODE_WIDTH, WIRE_UI_PLUGMODE_HEIGHT);
		wireUI_LoadPic(WIRE_UI_SETUP_OFF, WIRE_UI_SETUPMODE_X, WIRE_UI_SETUPMODE_Y, WIRE_UI_SETUPMODE_WIDTH, WIRE_UI_SETUPMODE_HEIGHT);
		#else
		wireUI_LoadPic(WIRE_UI_PNP, WIRE_UI_TETHERING_X, WIRE_UI_TETHERING_Y, WIRE_UI_TETHERING_WIDTH, WIRE_UI_TETHERING_HEIGHT);
		#endif
	}
	else
	{
		setWirePlugFlag(false);
		#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		wireUI_LoadPic(WIRE_UI_PLUG_OFF, WIRE_UI_PLUGMODE_X, WIRE_UI_PLUGMODE_Y, WIRE_UI_PLUGMODE_WIDTH, WIRE_UI_PLUGMODE_HEIGHT);
		wireUI_LoadPic(WIRE_UI_SETUP_ON, WIRE_UI_SETUPMODE_X, WIRE_UI_SETUPMODE_Y, WIRE_UI_SETUPMODE_WIDTH, WIRE_UI_SETUPMODE_HEIGHT);
		#else
		wireUI_LoadPic(WIRE_UI_TETHERING, WIRE_UI_TETHERING_X, WIRE_UI_TETHERING_Y, WIRE_UI_TETHERING_WIDTH, WIRE_UI_TETHERING_HEIGHT);
		#endif

	}
	#else
	ret = wireUI_LoadPic(WIRE_UI_BG, WIRE_UI_BG_X, WIRE_UI_BG_Y, WIRE_UI_BG_WIDTH, WIRE_UI_BG_HEIGHT);
	if(ret < 0){
		WLOGE("loadPic failed!\n");
		return -1;
	}
	if(isWirePlugMode())
	{
		setWirePlugFlag(true);
		#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		wireUI_LoadPic(WIRE_UI_PLUG, WIRE_UI_PLUG_X, WIRE_UI_PLUG_Y, WIRE_UI_PLUG_WIDTH, WIRE_UI_PLUG_HEIGHT);
		#else
		wireUI_LoadPic(WIRE_UI_PNP, WIRE_UI_TETHERING_X, WIRE_UI_TETHERING_Y, WIRE_UI_TETHERING_WIDTH, WIRE_UI_TETHERING_HEIGHT);
		#endif
	}
	else
	{
		setWirePlugFlag(false);
		#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
		wireUI_LoadPic(WIRE_UI_SETUP, WIRE_UI_PLUG_X, WIRE_UI_PLUG_Y, WIRE_UI_PLUG_WIDTH, WIRE_UI_PLUG_HEIGHT);
		#else
		wireUI_LoadPic(WIRE_UI_TETHERING, WIRE_UI_TETHERING_X, WIRE_UI_TETHERING_Y, WIRE_UI_TETHERING_WIDTH, WIRE_UI_TETHERING_HEIGHT);
		#endif
	}
	#endif
	#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
	wireUI_LoadPic(WIRE_UI_DONGLE, WIRE_UI_DONGLE_X, WIRE_UI_DONGLE_Y, WIRE_UI_DONGLE_WIDTH, WIRE_UI_DONGLE_HEIGHT);
	wireUI_LoadPic(WIRE_UI_PHONE, WIRE_UI_PHONE_X, WIRE_UI_PHONE_Y, WIRE_UI_PHONE_WIDTH, WIRE_UI_PHONE_HEIGHT);	
	wireUI_LoadPic(WIRE_UI_ARROW_PLUG, WIRE_UI_ARROW_X, WIRE_UI_ARROW_Y, WIRE_UI_ARROW_WIDTH, WIRE_UI_ARROW_HEIGHT);
	#else
	showADQRcode();
	#endif
	bg_flag = 1;
	windowSet_flag = 0;
	
	return 0;
}

int ezwireDrawWindow(char *jpgPath, int pos_x, int pos_y, int pic_width, int pic_height)
{
	if(bg_flag == 1){
		wireUI_LoadPic(jpgPath, pos_x, pos_y, pic_width, pic_height);
		windowSet_flag = 1;
	}else{
		ezwireDrawDefaultBg();
		wireUI_LoadPic(jpgPath, pos_x, pos_y, pic_width, pic_height);
		windowSet_flag = 1;
	}
	return 0;
}

int ezwireDrawFlip()
{
	return wireUI_Flip();
}

int ezwireMainUiHide()
{
	return  wireUI_Hide();
}

void ezwireUiDeRelease()
{
	wireUI_DeRelease();
}

void ezwireMainUiStop()
{
	wireUI_Release();
}
static int am_nosupport_chip_id(int id)
{
	printf("\n***********************************************************\n");
	printf("Do not support this IC[%d]!!!\n", id);
	printf("***********************************************************\n");
	while(1){
		sleep(1);
	}
}
static int am_soc_judge_chip_id()
{
	#if AM_CHIP_MINOR == 8258
	unsigned int val=0;
	int i=0;
	int chipid = CHIP_ID_AM8258ERROR;
	// Must enable nor flash IF hclk
	unsigned int old_devclk = reg_readl(0xb0010080);
	reg_writel(0xb0010080, old_devclk|0x1);
	reg_writel(0xb0038008,0x301065);  //write password to efuse reg
	for(i=0;i<0x200;i++);  
	reg_writel(0xb0038008,0x0);
	
	val = reg_readl(0xb0038008);
	val = reg_readl(0xb0038008);
	val = reg_readl(0xb0038008);
	val = reg_readl(0xb0038008);
	val = (val>>28)&0xf;
	switch(val){
		case 0:
		chipid = CHIP_ID_AM8258B;
		break;

		case 1:
		chipid = CHIP_ID_AM8258L;
		break;

		case 2:
		chipid = CHIP_ID_AM8258D;
		break;
		
		case 3:
		chipid = CHIP_ID_AM8258N;
		break;
		
		default:
		chipid = CHIP_ID_AM8258ERROR;
		break;       
	}
	printf("CHIP_ID_AM8258=%d\n",chipid);
	reg_writel(0xb0010080, old_devclk);
#if !defined(MODULE_CONFIG_AD_UI_ENABLE) || MODULE_CONFIG_AD_UI_ENABLE==0
	#if (EZWIRE_TYPE==MIRAPLUG)
		#if defined(MODULE_CONFIG_8258N_ENABLE) && (MODULE_CONFIG_8258N_ENABLE == 1)
		if(chipid!=CHIP_ID_AM8258N)
		{
			am_nosupport_chip_id(chipid);
		}
		#else
		if(chipid!=CHIP_ID_AM8258B)
		{
			am_nosupport_chip_id(chipid);
		}
		#endif
	#elif (EZWIRE_TYPE==MIRALINE)
		#if defined(MODULE_CONFIG_8258D_ENABLE) && (MODULE_CONFIG_8258D_ENABLE == 1)
		if(chipid!=CHIP_ID_AM8258D)
		{
			//am_nosupport_chip_id(chipid);
		}
		#else
		if(chipid!=CHIP_ID_AM8258L)
		{
			//am_nosupport_chip_id(chipid);
		}
		#endif
	#endif
#endif
	#endif
	return 0;
}

int ezwireUiInit()
{
#if 0
	DE_config ds_conf;	
	int scr_width = 0, src_height = 0;
	unsigned long buffer_addr;	
	int osd_width = WIRE_OSD_W;
	int osd_height = WIRE_OSD_H;
	int osd_icon_width = WIRE_ICON_OSD_W;
	int osd_icon_hight = WIRE_ICON_OSD_H;
	int ret;

	osd_de_init(&inst);	
	osd_de_get_config(inst,&ds_conf,DE_CFG_ALL);
	scr_width = ds_conf.input.width;
	src_height = ds_conf.input.height;
	ds_conf.input.enable=0;
	osd_de_set_Config(inst,&ds_conf,DE_CFG_IN);

	p_heap = (void *)wire_SysHeapInit(4*1024*1024);
	if(!p_heap){
		printf("[%s]%d, swf_malloc fail\n", __func__, __LINE__);
		goto __UI_INIT_ERR;
	}

	buffer = (char *)OSHmalloc(p_heap, WIRE_ICON_OSD_W*WIRE_ICON_OSD_H*2, &buffer_addr);
	if(!buffer)
	{
		OSprintf("%s, OSHmalloc error\n", __FUNCTION__);
		goto __UI_INIT_ERR;
	}
	
	ds_conf.input.enable = 1;
	ds_conf.input.width = osd_icon_width;
	ds_conf.input.height = osd_icon_hight;
	
	_img_memset4((unsigned int*)buffer,0x80108010,ds_conf.input.width*ds_conf.input.height/2);
	ds_conf.input.img = (unsigned long*)buffer;
	ds_conf.input.bus_addr = (unsigned long)buffer_addr;
	osd_de_set_Config(inst,&ds_conf,DE_CFG_OSD1);					
	ret = osdengine_init_osd((scr_width-osd_width)/2,(src_height-osd_height)/2,osd_width,osd_height,DE_PIX_FMT_OSDBIT8MODE,"/am7x/case/data/wire_256.plt");
	if(ret < 0){
		WLOGE("init osd failed\n");
		goto __UI_INIT_ERR;
	}
	osdengine_show_icon((osd_width-osd_icon_width)/2,(osd_height-osd_icon_hight)/2,"/am7x/case/data/wire_256_RGB_Default.bin", 1);	

	return 0;
__UI_INIT_ERR:
	osdengine_release_osd();
	
	if(inst){
		osd_de_release(inst);
		inst = NULL;
	}
	if(buffer){
		OSHfree(p_heap, buffer);
		buffer = NULL;
	}

	if(p_heap){
		wire_SysHeapRelease(p_heap);
		p_heap = NULL;
	}
	return -1;
#else
	int ret;
	am_soc_judge_chip_id();
	ret = ezwireMainUiStart();
	if(ret < 0){
		WLOGE("UI_Init failed!\n");
		return -1;
	}
	ret = ezwireDrawDefaultBg();
	if(ret < 0){
		WLOGE("DrawBg failed!\n");
		return -1;
	}	
	ezwireDrawFlip();

	return 0;
#endif

}

void ezwireUiRelease()
{
#if 0
	osdengine_release_osd();
	
	if(inst){
		osd_de_release(inst);
		inst = NULL;
	}
	if(buffer){
		OSHfree(p_heap, buffer);
		buffer = NULL;
	}

	if(p_heap){
		wire_SysHeapRelease(p_heap);
		p_heap = NULL;
	}
#else
	ezwireUiDeRelease();
	ezwireMainUiStop();
#endif
}

void ezwireInit()
{

	get_gpio(26, &gpio26);
	EZCASTLOG("gpio26: %d\n", gpio26);
      
	system("insmod /lib/modules/2.6.27.29/am7x_hcd_next.ko");
	system("insmod /lib/modules/2.6.27.29/am7x_hcd.ko");
	usleep(100*1000);
	#if (EZWIRE_TYPE==MIRAPLUG ||MODULE_CONFIG_ADB_MIRROR_ONLY==1)//iOS & Android with 8M snor
	adbAudioInit();
	system("modprobe /lib/modules/2.6.27.29/snd-usb-audio.ko");
	#endif
#if EZWIRE_CAPTURE_ENABLE
	if(access("/mnt/vram/ezcast/PLUGPLAYMODE", F_OK) != 0)
	{
		setWirePlugMode(1);
	}
#endif

}

static char *trim(char * src)
{
	int len = strlen(src); 
	char *begin = src;
	char *end = src + len - 1; //the last char may be '\n'
	if(len == 0 ) 
		return NULL;
	while((*begin == ' ') || (*begin == '\t') || (*begin == '\n'))
		begin++;
	while((*end == ' ') || (*end == '\t') || (*end == '\n'))
		end--;
	if(begin > end)
		return NULL;
	
	*++end = '\0';

	return begin;
}

static char *take_out(char *src, char spilt, bool dirct)
{
	char *ret = NULL;
	char *pos = NULL;
	int len = 0;
	if((pos = strchr(src, spilt)) == NULL)	
		goto end;
    
	if(dirct) { //right
		 ret = trim(pos + 1);
	} else {//left
		 *pos = '\0'; 
		 ret = trim(src);

	}
end:
	return ret;
}

static int seek_pos(FILE *fp, char *attr)
{
	int ret = -1;
	char buf[256];	
	char *pos  = NULL;
	bool resert = false;
	int len = -1;
	while(1)
	{		
		memset(buf, '\0', sizeof(buf));
		if(fgets(buf, sizeof(buf), fp) != NULL) {	
			len = strlen(buf);
			if((pos = trim(buf)) == NULL)
				continue;
			if(strncmp(pos, attr, strlen(attr)) == 0) {
				fseek(fp, -len, SEEK_CUR);
				ret = 0;
				break;
			} 
		} else {					
			if(resert) // already resert, not find
				break;
			else      //seek to begin of the file
			{
				resert = true;
				fseek(fp, 0, SEEK_SET);
			}
		}
	}
	
	return ret;
}

int ota_get_vendor(char *name)
{
	FILE *fp;
	char *pos, *vendor;
	char buf[128];

	if(name != NULL){
		EZCASTLOG("name: %s\n", name);
		fp = fopen(VERSION_CFG_PATH, "r");
		if(fp == NULL){
			perror("open VERSION_CFG_PATH");
			return -1;
		}
		
		seek_pos(fp, "VERSION_MODEL");
		fgets(buf, sizeof(buf), fp);
		if((pos = take_out(buf, '=', true)) == NULL) {
			printf("ota url is error\n");
			return -1;
		}
		vendor = trim(pos);
		//printf("\t[%s] [%d] vendor: %s\n", __func__, __LINE__, vendor);
		memcpy(name, vendor, strlen(vendor));
		name[strlen(vendor)] = '\0';
		
		return 0;
	}

	return -1;
}

static int ezCastGetVendor(char *name){
	int err = -1;
	err = ota_get_vendor(name);
	printf("[%s][%d] -- vendor is: %s\n", __func__, __LINE__, name);
	return err;
}

static int am_soc_get_chip_id_high_32bit_to_string(char *buf, int len)
{
	unsigned int val;
	int i;

	if(buf == NULL){
		printf("[%s-%d] buf NULL\n", __func__, __LINE__);
		return -1;
	}

	// Must enable nor flash IF hclk
	unsigned int old_devclk = reg_readl(0xb0010080);
	reg_writel(0xb0010080, old_devclk|0x1);
	
	reg_writel(0xb0038008,0x301065);  //write password to efuse reg
	for(i=0;i<0x200;i++);  
	reg_writel(0xb0038008,0x0);
	
	val = reg_readl(0xb0038008);
	#if (AM_CHIP_MINOR == 8258 || AM_CHIP_MINOR == 8268)
	val = reg_readl(0xb0038008);
	val = reg_readl(0xb0038008);
	printf("chipid[32,63]: 0x%08X\n", val);
	#else
	printf("chipid[0,31]: 0x%08X\n", val);
	#endif
	snprintf(buf, len, "%08X", val);
	reg_writel(0xb0010080, old_devclk);

	return 0;
}

#if defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE!=0
static int __qrcode_gen_bitmap(struct qrc_code *qrcoder, char *file)
{
	int w,i,j,k,m;
	unsigned char *buf,*ptr;
	int black,margin=4,pixsize=4,err;
	int filesize=0;
	unsigned int *pInt;

	if(file == NULL){
		return 0;
	}

	w = qrc_get_width(qrcoder);
	if(w <= 0){
		printf("QR CODE w=%d error\n",w);
		return 0;
	}

	buf = (unsigned char *)malloc((w+2*margin)*(w+2*margin)*2*pixsize*pixsize);
	if(buf == NULL){
		printf("QR CODE generate error:not enough memory\n");
		return 0;
	}

	pInt = (unsigned int *)buf;
	for(i=0;i<(w+2*margin)*(w+2*margin)*2*pixsize*pixsize/4;i++){
		*(pInt + i) = 0x80EB80EB;
	}

	for(i=0;i<w;i++){
		for(j=0;j<w;j++){
			black = qrc_get_code_at_pos(qrcoder,j,i);
			if(black==1){
				ptr = buf + (i+margin)*pixsize*(w+2*margin)*pixsize*2 + (j+margin)*pixsize*2; 
				for(k=0;k<pixsize;k++){
					pInt = (unsigned int *)ptr;
					for(m=0;m<pixsize/2;m++){
						*(pInt + m) = 0x80008000;
					}
					ptr += (w + 2*margin)*pixsize*2;
				}

			}

		}
	}

	err = jpeg_encode((void *)file,(void*)buf,NULL,NULL,(w+2*margin)*pixsize*2,4,0,0,(w+2*margin)*pixsize,(w+2*margin)*pixsize,3,&filesize,1);
	if(err != 0){
		printf("QR CODE generate error:encode error\n");
		err = 0;
	}
	else{
		err = 1;
	}

	free(buf);

	return err;
}


int createQRcode(char *url)
{
	int ret = -1;
	struct qrc_code *qrcoder=qrc_open();
	if(qrcoder == NULL)
	{
		EZCASTWARN("qrc_open fail.\n");
		goto __END__;
	}

	ret = qrc_encode(qrcoder,(unsigned char *)url);
	if(ret < 0)
	{
		EZCASTWARN("QR encode error\n");
		goto __END__;
	}

	if(access(WIRE_UI_ADQR, F_OK) == 0)
	{
		unlink(WIRE_UI_ADQR);
	}
	__qrcode_gen_bitmap(qrcoder, WIRE_UI_ADQR);

__END__:
	if(qrcoder != NULL)
	{
		qrc_close(qrcoder);
		qrcoder = NULL;
	}
}


int createADQRcode()
{
#define QR_WEBSITE	"https://qrcode.any-cast.com/l"
	char vendor[128];
	char id[32];
	char version[128];
	char strVal[1024];
	int ret = -1;

	ret = ota_get_vendor(vendor);
	if(ret < 0)
	{
		EZCASTWARN("Get vendor name fail.");
		snprintf(vendor, sizeof(vendor), "unknown");
	}

#if EZWIRE_ENABLE
	ret = am_soc_get_chip_id_high_32bit_to_string(id, sizeof(id));
#else
	ret = get_mac_address_info(id, MAC_WLAN0);
#endif
	if(ret < 0)
	{
		EZCASTWARN("Get ChipID/MAC fail.");
		snprintf(id, sizeof(id), "unknown");
	}

	memset(version, 0, sizeof(version));
	snprintf(strVal, sizeof(strVal), "%s?vendor=%s&device_id=%s&firmware_version=%s&softap_ssid=%s", \
			(char *)QR_WEBSITE, vendor, id, ota_get_local_version(version), ezcast_msg.broadcast_ssid);

	EZCASTLOG("url: %s\n", strVal);

	createQRcode(strVal);

	return 0;


}
#else
int createADQRcode()
{
	return 0;
}
#endif

static void (*cJSON_free)(void *ptr) = free;

static int ota_conf_init()
{
	char str[128];
	cJSON *js, *js_sub;
	FILE *fp;
	char *ch = NULL;
	int ret;

	js = cJSON_CreateObject();
	if(js == NULL){
		perror("Create Object");
		return -1;
	}

	js_sub = cJSON_CreateNumber(1);
	if(js_sub == NULL){
		perror("Create Object vendor");
		return -1;
	}
	cJSON_AddItemToObject(js, "version", js_sub);
	js_sub = NULL;

	memset(str, 0, sizeof(str));
	ezCastGetVendor(str);
	printf("\t[%s] [%d] vendor: %s\n", __func__, __LINE__, str);
	js_sub = cJSON_CreateString(str);
	if(js_sub == NULL){
		perror("Create Object vendor");
		cJSON_Delete(js);
		return -1;
	}
	cJSON_AddItemToObject(js, "vendor", js_sub);
	js_sub = NULL;
	
	memset(str, 0, sizeof(str));

	if(am_soc_get_chip_id_high_32bit_to_string(str, sizeof(str)) == 0)
		js_sub = cJSON_CreateString(str);

	if(js_sub == NULL){
		perror("Create Object mac_address");
		cJSON_Delete(js);
		return -1;
	}
	//free(ch);
	//ch = NULL;
	cJSON_AddItemToObject(js, "mac_address", js_sub);
	js_sub = NULL;

	js_sub = cJSON_CreateString(ezcast_msg.broadcast_ssid);
	if(js_sub == NULL){
		perror("Create Object softap_ssid");
		cJSON_Delete(js);
		return -1;
	}
	cJSON_AddItemToObject(js, "softap_ssid", js_sub);
	js_sub = NULL;

	js_sub = cJSON_CreateString(ezcast_msg.broadcast_version);
	if(js_sub == NULL){
		perror("Create Object firmware_version");
		cJSON_Delete(js);
		return -1;
	}
	cJSON_AddItemToObject(js, "firmware_version", js_sub);
	js_sub = NULL;

	fp = fopen(GETOTAURL_OTACONF, "w");
	if(!fp){
		perror("Open ota_config.json");
		cJSON_Delete(js);
		return -1;
	}

	if((ch = cJSON_Print(js)) != NULL){
		ret = fwrite(ch, 1, strlen(ch), fp);
		if(ret < 0){
			perror("Write ota_config.conf");
		}
		cJSON_free(ch);
	}

	fclose(fp);
	cJSON_Delete(js);

	return 0;
}

static int ezremote_save_ssid(const char *ssid)
{
	if(ssid != NULL){
		EZCASTLOG("ssid: %s\n", ssid);
		snprintf(ezcast_msg.broadcast_ssid, SSID_MAX_LEN, "%s", ssid);
		sethostname(ezcast_msg.broadcast_ssid, strlen(ezcast_msg.broadcast_ssid));
		return 0;
	}else{
		EZCASTWARN("SSID is NULL!!!");
	}
	return -1;
}

static int ezwire_set_devname(char *prefix)
{
	FILE *fp = NULL;
	int ret = -1;
	char _devName[128];
	char chipid[32];

	if(prefix == NULL){
		EZCASTWARN("prefix is NULL\n");
		return -1;
	}

	memset(chipid, 0, sizeof(chipid));

	am_soc_get_chip_id_high_32bit_to_string(chipid, sizeof(chipid));

	snprintf(_devName, sizeof(_devName), "%s%s", prefix, chipid);
	ezremote_save_ssid(_devName);
	return 0;
}

char * ota_get_local_version(char *local_version)
{
	FILE *fp = NULL;
	char buf[256];
	char *temp_value = NULL;
    
	fp = fopen(VERSION_PATH, "r");	
	if(fp == NULL) {		
		printf("%s not exist\n",VERSION_PATH);				
		goto end;	
	}
	if((seek_pos(fp, "FIRMWARE") != 0)) {
		goto end;
	}
	if(fgets(buf, sizeof(buf), fp) != NULL) {
		if((temp_value = take_out(buf, '=', true)) == NULL)
			goto end;	
        memset(local_version, '\0', sizeof(local_version));
        memcpy(local_version, temp_value, strlen(temp_value));
	} else {
		goto end;
	}
	
end:	

	if(fp != NULL)
		fclose(fp);
	return local_version;
}

static int ezremote_get_version(char *remote_version, int max_len)
{
	char local_version[20] = "error";

	memcpy(remote_version, ota_get_local_version(local_version), max_len);

	return 0;
}

int ezremote_save_version()
{
	memset(ezcast_msg.broadcast_version, 0, sizeof(ezcast_msg.broadcast_version));
	ezremote_get_version(ezcast_msg.broadcast_version, VERSION_MAX_LEN);
	printf("@@@@@@@@@@@@@@@@@@@ broadcast_version: %s\n", ezcast_msg.broadcast_version);
	return 0;
}

int show_local_version()
{
	long version=0;
	int len=0,i=0,pos_x=0,pos_w=0,num=0;
	char path[64]={0};
	pos_x=WIRE_UI_VERSION_X;
  	version=atol(ezcast_msg.broadcast_version);
	printf(" test version=%ld,len: %d\n", version);
	while(version>0)
	{
              num=version%10;
		version=version/10;
		sprintf(path,"/am7x/case/data/%d.jpg",num);
		pos_w=WIRE_UI_VERSION_WIDTH;
		wireUI_LoadPic(path, pos_x, WIRE_UI_VERSION_Y, WIRE_UI_VERSION_WIDTH, WIRE_UI_VERSION_HEIGHT);
		pos_x=pos_x-pos_w;
	}
	
	return 0;

}

#if defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE!=0
void showADQRcode()
{
	if(access(WIRE_UI_ADQR, F_OK) != 0)
	{
		createADQRcode();
	}
	wireUI_LoadPic(WIRE_UI_ADQR, WIRE_UI_ADQR_X, WIRE_UI_ADQR_Y, WIRE_UI_ADQR_WIDTH, WIRE_UI_ADQR_HEIGHT);
}
#else
void showADQRcode()
{
}
#endif

void productInit()
{
	ezwire_set_devname(DEVNAME);

	ezremote_save_version();
#if defined(MODULE_CONFIG_AD_UI_ENABLE) && MODULE_CONFIG_AD_UI_ENABLE!=0
	createADQRcode();
	showADQRcode();
#endif
	#if (EZWIRE_TYPE==MIRAPLUG || EZWIRE_TYPE==MIRALINE)
	show_local_version();
	showRedDot();
	#endif
	ota_conf_init();
}
