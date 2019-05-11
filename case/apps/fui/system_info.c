#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <errno.h>
#include "swf_ext.h"
#include "system_info.h"
#include "sys_rtc.h"
#include "fileselector.h"
#include "media_hotplug.h"
#include "sys_msg.h"
#include "am7x_dac.h"
#include "sys_vram.h"
#include "apps_vram.h"
#include "lcm_op.h"
#include "sys_cfg.h"
#include "am7x_dac.h"
#include "display.h"
#include "cardupgrade.h"
#include "image_decode.h"
#include "sys_pmu.h"
#include "file_list.h"
#include "wifi_engine.h"
#include "sys_gpio.h"
#include "ezcast_public.h"
#include <sys/time.h>
#include "websetting.h"
#include <signal.h>
#include"audio_engine.h"
#include "ez_json.h"
#include "ezcast_public.h"
#include <openssl/md5.h>
int cur_index=0;
int up_usbcard_loopbackplay_status=0; //0  noloopback 1 usb loopbackplay ,2 sdcard loopbackplay
extern int  usbplugin_afterup; 
extern int  sdcardplugin_afterup;
int 	Dongle_response	=-1;
int wifiChannel_mode=-1;   // 1:5g,  0:24g
#define wifiChannel_mode_24G 0
#define wifiChannel_mode_5G 1

int wifiChannel_set=-1;
int ProboxConnectSocketFd = -1;

char cgi_factorytest_result[256]="";
int sysinfo_refreshdata_factorytest();
int sys_info_factory_test(char *);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
int wireStopMirrorCallback();
#endif


#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
//for WiFi Enterprise
extern char temp_filename[256];
int EAP_method;
#endif
#if (EZWIRE_CAPTURE_ENABLE != 0)
extern int catptureStart;
#endif


#if 1
#define sysdbg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define sysdbg_info(fmt, arg...)
#endif
#define sysdbg_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
/************for communicate between CGI and FUI***************************/
int app_total= 0; 
int language_index=0;
/*********************************************************************/
int cur_media_stauts;
int	sd_in,cardin_type,cf_in;//udisk_in;//cardin_type notes the card type such as mmc or xd or sd card
int wlan_in;
int bluetooth_in;
char system_ver[64];
char global_value_str[GLOVAL_VALUE_STR_LEN]="";
#if FUN_WIRE_ENABLE
static int android_in_status = 0;
static int ios_in_status = 0;
static int android_if_port = 0;
#endif

#define ROOTPATH_LEN_MAX 32
#if EZCAST_ENABLE
char disableSetJsonValue = 0;
#endif
#if EZMUSIC_ENABLE
extern int change_default_ssid();
#endif
/* current active media */
int sys_active_media = 0;

/*multi sector number*/
int multisector_num[2] = {0,0};
int identy_hcd_flag = 0;
int udisk1_in,udisk2_in;
#if 1
struct mass_dev_t  double_dev[2];
#endif

char *disk_rootpath[] = 
{
	[NAND_DISK] = "/mnt/udisk/",
	[CARD_DISK] = "/mnt/card/",	
	[HARD_DISK] = "/mnt/cf/",
	[ROOT_DISK] = "/",
	[USB_DISK1] = "/mnt/usb1/",
	[USB_DISK2] = "/mnt/usb2/",
	[USB_DISK3] = "/mnt/usb3/",
	[USB_DISK4] = "/mnt/usb4/",
	[PRIV_DISK]="/am7x/case/"
};

#define REALTEK		0
#define RALINK		1
#define LAN				2

#define SOFTAP RALINK //LAN
#define APPINFO_FILE		"/tmp/appInfo.json"

#define MAX_BACKLIGHT_GRADE 5
#define BACKLIGHT_DEF_VALUE 31
#define CMD_BACKLIGHT_VIR_TO_REAL	0
#define CMD_BACKLIGHT_REAL_TO_VIR	1
int BackLight_Map[MAX_BACKLIGHT_GRADE][2]={//数组的第二个元素取值范围0~64
			{1,1},
			{2,8},
			{3,16},
			{4,23},
			{5,31},
};

#define MAX_AUTO_POWER_FREQ 4
char auto_power_frequency[MAX_AUTO_POWER_FREQ]={128,127,31,96};	//Once,Everyday,Mon-Fri,Weekend

int output_prev_mode = SYSDEFAULT_SCREEN_OUTPUT_MODE;
char output_prev_enable = HDMI_ENABLE;
int output_mode_now = SYSDEFAULT_SCREEN_OUTPUT_MODE;
int currentActiveSwfFlag = 0;

char output_enable_now = HDMI_ENABLE;
extern void *deinst;
extern int UsbPort;
wifi_entire_stutas_emun wifi_mode=WIFI_INIT_STATUS;
char *url[128];
typedef struct url_info_s
{
		
       char url[512];		
	int idx;
}url_info_t;
url_info_t all_url_info[512];

int getSwfBusyFlagForEz(){
	/*****see the API sysinfo_put_swf_activeflag()********/
	
	sysdbg_info("currentActiveSwfFlagForEz ====%d\n",currentActiveSwfFlag);
	return currentActiveSwfFlag;
	
}
int get_info_from_file(char *file_path,char *search_location_begin,char *search_location_end,char *result){
	
	FILE *fp=NULL;
	char *locate1=NULL; 
	char *locate2=NULL; 
	char *locate3=NULL; 
	char buf[256] = {0};
	int ret = -1;
	fp=fopen(file_path,"r");
	if(fp==NULL){
		ret = CANNOT_FIND_INFO_FILE ;
		return -1;
	}
	while(!feof(fp))
    {
		memset(buf,0,sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		locate1=strstr(buf,search_location_begin);
		
		if(locate1){
			locate2=locate1;
			locate3=strstr(locate1,search_location_end);
			if(locate3){
				memcpy(result,locate2+strlen(search_location_begin),strlen(locate2+strlen(search_location_begin))-strlen(locate3));
				ret = GET_FILEINFO_OK;
				break;
			}
			else{
				
				ret = CANNOT_FIND_END_KEYWORD;
			}
		}
		else{
			
			ret = CANNOT_FIND_BEGIN_KEYWORD;
		}	
		
		
		memset(buf,0,256);
	}
	fclose(fp);
	return ret;
}

void modify_wifi_mode(int wifi_mode_temp){
	wifi_mode=wifi_mode_temp;
	return NULL;
	//printf("function:%s,line:%d,wifi_mode========%d\n",__FUNCTION__,__LINE__,wifi_mode);
}


int read_wifi_mode(){
	//printf("function:%s,line:%d,wifi_mode========%d\n",__FUNCTION__,__LINE__,wifi_mode);
	return wifi_mode;
	
}

int _get_env_data(struct sysconf_param *sys_cfg_data){
	if(apps_vram_read(VRAM_ID_SYSPARAMS,sys_cfg_data,sizeof(struct sysconf_param))!=0){
		sysdbg_err("Get environment paras err");
		return -1;
	}
	return 0;
}

int _save_env_data(struct sysconf_param *sys_cfg_data){
	if(apps_vram_write(VRAM_ID_SYSPARAMS,sys_cfg_data,sizeof(struct sysconf_param))!=0){
		sysdbg_err("App vram write error");
		return -1;
	}
	return 0;
}

int _store_env_data()
{
	if(apps_vram_store(VRAM_ID_SYSPARAMS)!=0){
		sysdbg_err("App vram store error");
		return -1;
	}
	return 0;
}

static int _print_env_data(){
	struct sysconf_param syscfg;
	_get_env_data(&syscfg);
	sysdbg_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	sysdbg_info("lang_id=%d,active_media_type=%d,sys_vol=%d",syscfg.local_language_id,syscfg.active_media_type,syscfg.sys_volume);
	sysdbg_info("backlight=%d,brightness=%d,contrast=%d,saturation=%d,hue=%d",syscfg.backlight_strength,syscfg.brightness,syscfg.constrast,syscfg.saturation,syscfg.hue);
	sysdbg_info("photo::dsp_ratio=%d,back_music=%d,music_en=%d,interval=%d,slideshow_mode=%d,eff=%d,clockshow_en=%d",\
		syscfg.photo_display_ratio,syscfg.photo_background_music_en,syscfg.photo_slideshow_interval,\
		syscfg.photo_slideshow_mode,syscfg.photo_slideshow_effect,syscfg.photo_clockshow_en);
	sysdbg_info("Video::autoplay=%d,dsp_ratio=%d,mode=%d",syscfg.video_autoplay,syscfg.video_display_ratio,syscfg.video_playback_mode);
	sysdbg_info("music_effect=%d,calendar_clock_mode=%d",syscfg.music_effect,syscfg.calendar_clock_mode);
	sysdbg_info("poweron_en=%d,hour=%d,min=%d,sec=%d",syscfg.auto_power_on_enable,syscfg.power_on_time.hour,syscfg.power_on_time.min,syscfg.power_on_time.sec);
	sysdbg_info("poweroff_en=%d,hour=%d,min=%d,sec=%d",syscfg.auto_power_off_enable,syscfg.power_off_time.hour,syscfg.power_off_time.min,syscfg.power_off_time.sec);
	sysdbg_info("printer_en=%d\n",syscfg.printer_enable);
	sysdbg_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	return 0;
}

static int _map_backlight_strength(int cmd, int backlight){
	int i=0;
	sysdbg_info("cmd===%d,backlight=%d\n",cmd,backlight);
	if(cmd==CMD_BACKLIGHT_VIR_TO_REAL){
		for(i=0;i<MAX_BACKLIGHT_GRADE;i++){
			if(BackLight_Map[i][0]==backlight){
				return BackLight_Map[i][1];
			}
		}
		sysdbg_err("the virtual_backligth=%d is not in the map",backlight);
		return BackLight_Map[0][1];
	}
	else if(cmd==CMD_BACKLIGHT_REAL_TO_VIR){
		for(i=0;i<MAX_BACKLIGHT_GRADE;i++){
			if(BackLight_Map[i][1]==backlight){
				return BackLight_Map[i][0];
			}
		}
		sysdbg_err("the real_backligth=%d is not in the map",backlight);
		return BackLight_Map[0][0];
	}
	else{
		sysdbg_err("CMD is Error==%d",cmd);
	}
	return -1;
}

static int _set_bklight_length(int * backlight){
	FILE *backlight_f = NULL;
	int fd=0;
	char bk_name[] = "sys/class/backlight/am7xbl1/brightness";
	char buf[50];
	int existornot = access(bk_name,F_OK);
	if(existornot==-1)
	{
		printf("the file is not exist,backlight can't be set\n");
		return -1;
	}
	memset(buf,0,sizeof(buf));
	sprintf(buf,"echo %d > %s",*backlight,bk_name);
	printf("the call is %s\n",buf);
	system(buf);
	return 0;
}

static int _color_adjust(int cmd,void*para){
	DE_INFO	de_info;
	int fd = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if(ioctl(fd,GET_DE_CONFIG,&de_info)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	switch(cmd){
		case CMD_SET_HUE:
			de_info.hue = *(signed int*)para;
			break;
		case CMD_SET_SATURATION:
			de_info.saturation = *(signed int*)para;
			break;
		case CMD_SET_BRIGHTNESS:
			de_info.brightness = *(signed int*)para;
			break;
		case CMD_SET_CONTRAST:
			de_info.contrast = *(signed int*)para;
			break;
		case CMD_SET_SHARPNESS:
			de_info.sharpness= *(signed int*)para;
			break;
	}
	if(cmd == CMD_SET_SHARPNESS){
		if(ioctl(fd,SHARPNESS_ADJUST,&de_info)<0){
			sysdbg_err("de set error\n");
			close(fd);
			return -1;
		}
	}
	else{
		if(ioctl(fd,COLOR_ADJUST,&de_info)<0){
			sysdbg_err("de set error\n");
			close(fd);
			return -1;
		}
	}
	close(fd);
	return 0;
}

static int set_de_defaultcolor(int defaultcolor)
{
	DE_config ds_conf;
	
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.default_color = defaultcolor;
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}
static int _set_dac_hdmi(int hdmi_enable)
{
	int dsp;
	
#if AM_CHIP_ID == 1220
	return 0;
#else
	dsp = open("/dev/DAC",O_RDWR);
	if(dsp<0){
		sysdbg_err("Sorry Open DAC cfg Error\n");
		return -1;
	}
	ioctl(dsp,DACIO_SET_HDMI,hdmi_enable);
	close(dsp);
#endif
	return 0;
}

int _set_hdmi_mode(int * hmode){
	int fd;
	fd  = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if((*hmode)==DEFAULT_OUTPUT_RESOLUTION)
	{
		ioctl(fd,LCD_OPEN,hmode);
		_set_dac_hdmi(0);
	}else
	{
		ioctl(fd,HDMI_OPEN,hmode);
		_set_dac_hdmi(1|HDMI_VOLUME_SW_EN);
	}
	set_de_defaultcolor(RGB_BLACK);
	close(fd);
	return 0;
}

int _set_cvbs_mode(int * hmode){
	int fd;
	fd  = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if((*hmode)==DEFAULT_OUTPUT_RESOLUTION)
	{
		ioctl(fd,LCD_OPEN,hmode);
	}else
	{
		ioctl(fd,CVBS_OPEN,hmode);
	}
	_set_dac_hdmi(0);
	set_de_defaultcolor(YUV_BLACK);
	close(fd);
	return 0;
}

int _set_ypbpr_mode(int * hmode){
	int fd;
	fd  = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if((*hmode)==DEFAULT_OUTPUT_RESOLUTION)
	{
		ioctl(fd,LCD_OPEN,hmode);
	}else
	{
		ioctl(fd,YPbPr_OPEN,hmode);
	}
	_set_dac_hdmi(0);
	set_de_defaultcolor(YUV_BLACK);
	close(fd);
	return 0;
}

int _set_vga_mode(int * hmode){
	int fd;
	fd  = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if((*hmode)==DEFAULT_OUTPUT_RESOLUTION)
	{
		ioctl(fd,LCD_OPEN,hmode);
	}else
	{
		ioctl(fd,VGA_OPEN,hmode);
	}
	_set_dac_hdmi(0);
	set_de_defaultcolor(RGB_BLACK);
	close(fd);
	return 0;
}
/*
static void close_display(){
	int fd;
	EZCASTLOG("-----============== Close screen!!!!!!\n");
	fd  = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	ioctl(fd,CLOSE_DISPLAY);
	close(fd);
}

static void open_display(){
	int fd;
	EZCASTLOG("-----============== Open screen!!!!!!\n");
	fd  = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	ioctl(fd,0x21);
	close(fd);
}
*/
int _get_screen_size(){
	DE_INFO info;
	int fd = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	ioctl(fd,GET_DE_CONFIG,&info);
	screen_output_data.screen_output_width = info.screen_width;
	screen_output_data.screen_output_height = info.screen_height;
	sysdbg_info("screen width is %d,height is %d\n",screen_output_data.screen_output_width,screen_output_data.screen_output_height);

	if(screen_output_data.screen_output_true== 0)
	{
		switch(screen_output_data.screen_output_mode)
		{
			case	FMT_858x525_720x480_60I:
					screen_output_data.screen_output_width = 720;
					screen_output_data.screen_output_height = 480;
					break;
			case 	FMT_864x625_720x576_50I:
					screen_output_data.screen_output_width = 720;
					screen_output_data.screen_output_height = 576;
					break;
			case	FMT_2200x1125_1920x1080_60I:
			case 	FMT_2640x1125_1920x1080_50I:
					screen_output_data.screen_output_width = 1920;
					screen_output_data.screen_output_height = 1080;
					break;
			case	FMT_2304x1250_1920x1152_50I:
					screen_output_data.screen_output_width = 1920;
					screen_output_data.screen_output_height = 1152;
					break;
		}
	}
	sysdbg_info("screen real width is %d,height is %d\n",screen_output_data.screen_output_width,screen_output_data.screen_output_height);
	close(fd);
	return 0;
}
	
int _do_action(enum cmd_set_env_para_e cmd,void *para){
	sysdbg_info("Call do action to set env para  cmd=%d",cmd);
	

	switch(cmd){
		case CMD_SET_BACKLIGHT_LENGTH:
			_set_bklight_length((int*)para);
			break;
		case CMD_SET_HUE:
		case CMD_SET_SATURATION:
		case CMD_SET_BRIGHTNESS:
		case CMD_SET_CONTRAST:
		case CMD_SET_SHARPNESS:
			_color_adjust(cmd,para);
			break;
		case CMD_SET_LOCAL_LANGUAGE:
			break;
		case CMD_SET_DEFAULT_ENV:
			break;
		case CMD_SET_VIDEO_PALY_MODE:
			break;
		case CMD_SET_HDMI_MODE:
			_set_hdmi_mode((int*)para);
			break;
		case CMD_SET_CVBS_MODE:
			_set_cvbs_mode((int*)para);
			break;
		case CMD_SET_YPBPR_MODE:
			_set_ypbpr_mode((int*)para);
			break;
		case CMD_SET_VGA_MODE:
			_set_vga_mode((int*)para);
			break;

	}
	return 0;
}

int _exec_hdmi_mode(int output_mode)
{
	sysdbg_info("exec hdmi mode---output mode is %d\n", output_mode);
	output_mode_now = output_mode;
	//sys_cfg_data->hdmi_mode = output_mode;
	screen_output_data.screen_output_mode = output_mode;
	switch(output_mode)
	{
		case	FMT_858x525_720x480_60I:
		case 	FMT_864x625_720x576_50I:
		case	FMT_2200x1125_1920x1080_60I:
		case 	FMT_2640x1125_1920x1080_50I:
		//case	FMT_2304x1250_1920x1152_50I:
				//output_mode = FMT_1650x750_1280x720_60P;
				//sys_cfg_data->hdmi_output_true = 0;
				screen_output_data.screen_output_true = 1;
				break;
		case    FMT_858x525_720x480_60P:
		case  	FMT_864x625_720x576_50P:
		case 	FMT_2200x1125_1920x1080_30P:
		case	FMT_2640x1125_1920x1080_25P:
		case	FMT_2750x1125_1920x1080_24P:
		case	FMT_1650x750_1280x720_60P:
		case 	FMT_3300x750_1280x720_30P:
		case	FMT_1980x750_1280x720_50P:
		case	FMT_3960x750_1280x720_25P:
		case	FMT_4125x750_1280x720_24P:
		case	FMT_1280x800_60P:
		case	FMT_800x600_60P:
		case	FMT_1920x1080_60P:
		case	FMT_1250x810_1024x768_60P:
				//sys_cfg_data->hdmi_output_true = 1;
				screen_output_data.screen_output_true = 1;
				break;
		default:
				break;
	}
	_do_action(CMD_SET_HDMI_MODE,&output_mode);
	sysdbg_info("exec hdmi mode---set hmode is %d\n", output_mode);
	/*if(_save_env_data(sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Write System Data Error!");
	}*/
	return 0;
}

int set_back_light(int bk){
	struct sysconf_param sys_cfg_data;
	if(_get_env_data(&sys_cfg_data)!=0)
		sysdbg_err("Sorry Read System Data Error!");
	else
		sys_cfg_data.backlight_strength = bk;

	_save_env_data(&sys_cfg_data);
	
	sysdbg_info("backlight length is %d\n",sys_cfg_data.backlight_strength);
	_do_action(CMD_SET_BACKLIGHT_LENGTH,&sys_cfg_data.backlight_strength);

	return 0;
}

#if 1  //associated with photo
int get_photo_disp_ratio(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.photo_display_ratio;
}

int set_photo_disp_ratio(int disp_ratio){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(disp_ratio>DSP_CROP_TO_FIT || disp_ratio<DSP_FIT_TO_SCREEN){
		sysdbg_err("Crazy Disp Ratio is not support, disp_ratio=%d",disp_ratio);
		return -1;
	}
	sys_info.photo_display_ratio = disp_ratio;
	_save_env_data(&sys_info);
	return 0;
}

int set_photo_background_effect(char bkg_effect)
{
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(bkg_effect<PHOTO_BKG_BLACK || bkg_effect>PHOTO_BKG_RADIANTMIRROR){
		sysdbg_err("Crazy Blackground Effect  is not support, bkg_effect=%d",bkg_effect);
		return -1;
	}
	sys_info.photo_background_effect= bkg_effect;
	_save_env_data(&sys_info);
	return 0;
}

int get_photo_background_effect(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.photo_background_effect;
}


int get_photo_slideshow_interval(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.photo_slideshow_interval;
}

int set_photo_slideshow_interval(int slideshow_interval){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.photo_slideshow_interval = slideshow_interval;
	_save_env_data(&sys_info);
	return 0;
}
int get_photo_slideshow_mode(){ //this is called by photo_engine.c
	struct sysconf_param sys_info; 
	_get_env_data(&sys_info);
	return sys_info.photo_slideshow_mode;
}

int set_photo_slideshow_mode(int slideshow_mode){  //this is called by photo_engine.c
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.photo_slideshow_mode = slideshow_mode;
	_save_env_data(&sys_info);
	return 0;
}
	
int get_photo_slideshow_effect(){ //this is called by photo_engine.c

	struct sysconf_param sys_info; 
	_get_env_data(&sys_info);
	return sys_info.photo_slideshow_effect;
}

int set_photo_slideshow_effect(int slideshow_effect){  //this is called by photo_engine.c
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.photo_slideshow_effect = slideshow_effect;
	_save_env_data(&sys_info);
	return 0;
}

/*add by richard */
/*add by richard 05032012*/
static int get_char_width(void *handle)
{
	int font_size, ret;
	unsigned short unicode;

	SWFEXT_FUNC_BEGIN(handle);

	unicode = Swfext_GetNumber(); //char code
	font_size = Swfext_GetNumber(); //the font size prefered

	ret = SWF_GetCharWidth(unicode, font_size);
	//printf("------------------------- unicode [%d], size[%d], ret[%d]\n", unicode, font_size, ret);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

int get_photo_bk_music_en(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.photo_background_music_en;
}

int set_photo_bk_music_en(int isenable){
	struct sysconf_param sys_info;
	printf("%s\n",__FUNCTION__);
	_get_env_data(&sys_info);
	sys_info.photo_background_music_en = isenable;
	_save_env_data(&sys_info);
	return 0;
}

int get_photo_autorotation_exif_en()
{
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return (int)sys_info.photo_autorotation_exif_en;
}

int set_photo_autorotation_exif_en(int autorotation_en){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.photo_autorotation_exif_en = !!autorotation_en;
	_save_env_data(&sys_info);
	return 0;
}

int get_photo_autorotation_adhere_en()
{
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return (int)sys_info.photo_autorotation_adhere_en;
}

int set_photo_autorotation_adhere_en(int autorotation_en){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.photo_autorotation_adhere_en = !!autorotation_en;
	_save_env_data(&sys_info);
	return 0;
}


#endif


#if 1 //associated with video
int get_video_disp_ratio(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sysdbg_info("dispratio=%d",sys_info.video_display_ratio);
	return sys_info.video_display_ratio;
}

int set_video_disp_ratio(int disp_ratio){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(disp_ratio!=DSP_ORIGINAL && disp_ratio!=DSP_CUT_TO_SCREEN && disp_ratio!=DSP_FULL_SCREEN && disp_ratio!=DSP_LETTER_BOX){
		sysdbg_err("Crazy Disp Ratio is not support, disp_ratio=%d",disp_ratio);
		return -1;
	}
	sys_info.video_display_ratio= disp_ratio;
	sysdbg_info("dispratio=%d",disp_ratio);
	_save_env_data(&sys_info);
	return 0;
}

int get_video_repeat_mode(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.video_playback_mode;
}

int set_video_repeat_mode(int repeat_mode){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(repeat_mode>=MODE_INVAID || repeat_mode<MODE_REPEAT_SEQUENCE){
		sysdbg_err("Crazy MODE  is not support, repeat_mode=%d",repeat_mode);
		return -1;
	}
	sys_info.video_playback_mode= repeat_mode;
	_save_env_data(&sys_info);
	return 0;
}
	
#endif

#if 1 //associated with music

int get_music_repeat_mode(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.music_playback_mode;
}

int set_music_repeat_mode(int repeat_mode){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(repeat_mode>=MODE_INVAID || repeat_mode<MODE_REPEAT_SEQUENCE){
		sysdbg_err("Crazy MODE  is not support, repeat_mode=%d",repeat_mode);
		return -1;
	}
	sys_info.music_playback_mode= repeat_mode;
	_save_env_data(&sys_info);
	return 0;
}

#endif
#if 1

/**
@brief set the default language idx, it is get from the fui.res
**/
int system_init_default_lang_idx()
{
	int default_lang_id=0;
	struct sysconf_param sys_cfg_data;

	default_lang_id = locale_get_default_langidx();	
	if(default_lang_id!=SYSDEFAULT_LOCAL_LANGUAGE)
	{
		_get_env_data(&sys_cfg_data);
		sys_cfg_data.local_language_id = default_lang_id;
		sysdbg_info("Default Lang==%d",default_lang_id);
		_save_env_data(&sys_cfg_data);
		_store_env_data();
	}
	return default_lang_id;
}




#endif
#if 1 //associated with ebook

int get_ebook_text_color(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.ebook_text_color;
}

int set_ebook_text_color(int text_color){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(text_color>=0xFFFFFF || text_color<0){
		sysdbg_err("Crazy COLOR  is not support, text color=%d",text_color);
		return -1;
	}
	sys_info.ebook_text_color= text_color;
	_save_env_data(&sys_info);
	return 0;
}

int get_ebook_text_size(){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	return sys_info.ebook_text_size;
}

int set_ebook_text_size(int text_size){
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(text_size<0){
		sysdbg_err("Crazy SIZE  is not support, text size=%d",text_size);
		return -1;
	}
	sys_info.ebook_text_size= text_size;
	_save_env_data(&sys_info);
	return 0;
}

#endif

#if 1 
///< associated with albums
int get_is_udisk_dirty()
{
	struct sysconf_param sys_info;
	//printf("%s\n",__FUNCTION__);
	//_get_env_data(&sys_info);
	return sys_info.is_udisk_dirty;
}
int set_is_udisk_dirty(int is_udisk_dirty)
{
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	if(is_udisk_dirty==0 || is_udisk_dirty == 1)
		sys_info.is_udisk_dirty= is_udisk_dirty;
	else
		return -1;
	_save_env_data(&sys_info);
	_store_env_data(); // Add By HHL
	return 0;
}


#endif

unsigned char _get_root_path(char *pathorg,char *rootpath){
	int i=0;
	unsigned char find=0;
	char *tmp=pathorg;
	
	while(*tmp!=0){
		if(*tmp=='/'){
			find++;
			if(find==3)
				break;
		}
		tmp++;
	}
	if(find==3){
		strncpy(rootpath,pathorg,(tmp-pathorg)+1);
		return 1;
	}
	else{
		printf("%s,%d:Sorry, the org path=%s,is a invalid path");
		return 0;
	}
}

int sysinfo_get_disktype_by_path(char *path)
{
	int disktype = ROOT_DISK;
	char rootpath[ROOTPATH_LEN_MAX]="";
	memset(rootpath,0,ROOTPATH_LEN_MAX);
	if(_get_root_path(path,rootpath)==0){
		return disktype;
	}
	if(strstr(rootpath,disk_rootpath[NAND_DISK])){
		disktype = NAND_DISK;
	}
	else if(strstr(rootpath,disk_rootpath[CARD_DISK])){
		disktype = CARD_DISK;
	}
	else if(strstr(rootpath,disk_rootpath[HARD_DISK])){
		disktype = HARD_DISK;
	}
	else if(strstr(rootpath,disk_rootpath[USB_DISK1])){
		disktype = USB_DISK1;
	}
	else if(strstr(rootpath,disk_rootpath[USB_DISK2])){
		disktype = USB_DISK2;
	}
	else if(strstr(rootpath,disk_rootpath[USB_DISK3])){
		disktype = USB_DISK3;
	}
	else if(strstr(rootpath,disk_rootpath[USB_DISK4])){
		disktype = USB_DISK4;
	}
	else if(strstr(rootpath,disk_rootpath[PRIV_DISK])){\
		disktype = PRIV_DISK;
	}
	else{
		printf("[ERROR]%s,%d:Can't Find RootPath",__func__,__LINE__);
		disktype = ROOT_DISK;
	}

	return disktype;
}

#define DLNA_MAX_VOL 100
static int mute_flag_to_dmr = 0;
static int current_dmr_volume = DLNA_MAX_VOL/2;
#define DLNAVOL_SWITCH_TO_SYSVOL 0x01
#define SYSVOL_SWITCH_TO_DLNAVOL 0x02


#define sys_dmr_vol_dbg(fmt,arg...) printf("sys_dmr_vol_dbg[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

static int store_vol_into_vram(int vol_set){
	struct sysconf_param sys_cfg_data;

	_get_env_data(&sys_cfg_data);	
	if(sys_cfg_data.sys_volume!=vol_set)
	{
		sys_cfg_data.sys_volume = vol_set;
		_save_env_data(&sys_cfg_data);
		_store_env_data();

	}

	return 0;
}
static int read_vol_from_vram(){
	
	struct sysconf_param sys_cfg_data;
	int vol_read_from_vram;
	
	_get_env_data(&sys_cfg_data);
	vol_read_from_vram = sys_cfg_data.sys_volume;

	return vol_read_from_vram;
}


int set_mute_for_dmr(int mute_flag){
	
	int vol = 0;
	int vol_get=5;
	sys_dmr_vol_dbg("mute_flag ========%d\n",mute_flag);
	sys_dmr_vol_dbg("current_dmr_volume ========%d\n",current_dmr_volume);
	if(mute_flag){
		vol = 0;
		ezVolumeSet(vol);
	}
	else{
		vol = current_dmr_volume*_MAX_VOLUME_LEVEL/DLNA_MAX_VOL;
		sys_dmr_vol_dbg("vol ========%d\n",vol);
		ezVolumeSet(vol);
	}
	
	//store_vol_into_vram(vol);
	mute_flag_to_dmr = mute_flag;

	return 0;
}

int get_mute_for_dmr(){

	return mute_flag_to_dmr;
}


int set_volume_for_dmr(int vol){
	int rtn = -1;
	int vol_get = DLNA_MAX_VOL/2;
	struct sysconf_param sys_cfg_data;
	current_dmr_volume = vol;
	
	if(vol<0 || vol>DLNA_MAX_VOL)
		vol = vol_get;
	
	vol = vol*_MAX_VOLUME_LEVEL/DLNA_MAX_VOL;
	sys_dmr_vol_dbg("vol ========%d\n",vol);
	ezVolumeSet(vol);
	//store_vol_into_vram();
	
	return 0;
}

int get_volume_for_dmr()
{

	return current_dmr_volume;
}

static int sysinfo_set_volume_mute(void * handle)
{
	int vol = 0;
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	rtn = sys_volume_ctrl(_VOLUME_CTRL_SET,(void *)&vol);
	if(rtn!=-1){
		printf("set system volume mute successfully!");
	}
	else{
		printf("set system volume mute failed!");
	}
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int sysinfo_set_volume(void * handle)
{
	int vol=5,vol_get=5;
	int rtn=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	vol = Swfext_GetNumber();
	if(vol ==_VOLUME_LEVEL_MUTE){
		rtn = sys_volume_ctrl(_VOLUME_CTRL_MUTE,NULL);
	}
	else{
		rtn = sys_volume_ctrl(_VOLUME_CTRL_SET,(void *)&vol);
	}
	if(rtn!=-1){
		_get_env_data(&sys_cfg_data);
		sys_volume_ctrl(_VOLUME_CTRL_GET,(void *)&vol_get);
		//sysdbg_info("vol set is %d,vol get is %d\n",vol,vol_get);
		if(sys_cfg_data.sys_volume!=vol_get)
		{
			sys_cfg_data.sys_volume= vol_get;
			_save_env_data(&sys_cfg_data);
			_store_env_data();
		}
	}
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}

static int sysinfo_get_volume(void * handle)
{
	int vol = 5;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	vol = sys_cfg_data.sys_volume;
	//sysdbg_info("vol now is %d\n",vol);
	
	Swfext_PutNumber(vol);
	SWFEXT_FUNC_END();
}

static int sysinfo_get_fwversion(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	
	Swfext_PutString("1.0.0");
	
	SWFEXT_FUNC_END();
}


static int sysinfo_get_version(void * handle)
{

	FILE *fp=NULL;
	char *file_path = "/etc/version.conf"; 

	char *locate1=NULL;
	char *locate2=NULL; 
	char *locate3=NULL; 
	char buf[64] = {0};
	char resultbuf[40] = {0};
	
	SWFEXT_FUNC_BEGIN(handle);

	char *begin = Swfext_GetString();

	fp=fopen(file_path,"r");

	if(fp==NULL){
		goto version_end;
	}
	while(!feof(fp))
    {
		memset(buf,0,64);
		memset(resultbuf,0,40);
		fgets(buf,64,fp);
		printf("buf=========%s\n",buf);
		
		locate1=strstr(buf,begin);		
		if(locate1!=NULL){
			locate2=locate1;

			locate3=strstr(locate1,"\n");

			if(locate3!=NULL){
				memcpy(resultbuf,locate2+strlen(begin)+3,strlen(locate2+strlen(begin)+3)-strlen(locate3));
				printf("resultbuf============%s\n",resultbuf);
				break;
			}

		}
	}

	fclose(fp);

version_end:
	
	Swfext_PutString(resultbuf); //modified by richard 0503
	
	SWFEXT_FUNC_END();
}

/**
* @brief get the space for a mounted file system.
* the unit of the return value is SECTOR and each
* SECTOR is 512 bytes.
* 
* @param path: one of the files' path in the mounted
*    file system
* @param type: GET_SPACE_TOTAL the total space and 
*    GET_SPACE_FREE the free space.
*/
static long _get_fs_space(char * path, int type)
{
	struct statfs fsstatus;
	long space;
	long blocks;

	if(statfs(path, &fsstatus)==0){
		
		if(type == GET_SPACE_TOTAL){
			blocks = fsstatus.f_blocks;
		}
		else{
			blocks = fsstatus.f_bfree;
		}
		
		if(fsstatus.f_bsize>=512){
			return (fsstatus.f_bsize/512)*blocks;
		}
		else{
			return (fsstatus.f_bsize * blocks)/512;
		}
	}
	else{
		printf("get fs stat error==%d\n",errno);
		return -1;
	}
}


static int sysinfo_get_disk_space(void * handle)
{
	long space=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	space = _get_fs_space(disk_rootpath[NAND_DISK],GET_SPACE_TOTAL);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_disk_free_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);

	space = _get_fs_space(disk_rootpath[NAND_DISK],GET_SPACE_FREE);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_card_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);

	space = _get_fs_space(disk_rootpath[CARD_DISK],GET_SPACE_TOTAL);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_card_free_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);
	
	space = _get_fs_space(disk_rootpath[CARD_DISK],GET_SPACE_FREE);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);
	
	SWFEXT_FUNC_END();
}


static int sysinfo_get_cfcard_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);

	space = _get_fs_space(disk_rootpath[HARD_DISK],GET_SPACE_TOTAL);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);
	
	SWFEXT_FUNC_END();
}


static int sysinfo_get_cfcard_free_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);

	space = _get_fs_space(disk_rootpath[HARD_DISK],GET_SPACE_FREE);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);

	SWFEXT_FUNC_END();
}
static int sysinfo_get_udisk_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);

	space = _get_fs_space(disk_rootpath[USB_DISK1],GET_SPACE_TOTAL);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_udisk_free_space(void * handle)
{
	long space=0;

	SWFEXT_FUNC_BEGIN(handle);

	space = _get_fs_space(disk_rootpath[USB_DISK1],GET_SPACE_FREE);
	if(space == -1){
		space = 0;
	}
	space = space*512/1024;		//KB
	printf("%s,%d:total space is %d\n",__FILE__,__LINE__,space);
	Swfext_PutNumber(space);

	SWFEXT_FUNC_END();
}

static int sysinfo_set_backlight(void * handle)
{

	int bl = 21;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	bl = Swfext_GetNumber();
	if(bl>BACKLIGHT_DEF_VALUE)
		bl = BACKLIGHT_DEF_VALUE;
	else if(bl<1)
		bl = 1;
	set_back_light(bl);
	SWFEXT_FUNC_END();
}

static int sysinfo_get_back_light(void * handle)
{
	int bl = 21;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	bl = sys_cfg_data.backlight_strength;
	sysdbg_info("backlight length is %d\n",bl);
	Swfext_PutNumber(bl);
	
	SWFEXT_FUNC_END();
}


   
static int sysinfo_adjust_color(void * handle)
{
	
	unsigned int out=0,in;
	int func;
	int rw;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	func = Swfext_GetNumber();
	rw=Swfext_GetNumber();
	in=Swfext_GetNumber();

	//printf("%s,%d:func is %d, rw is %d, in is %d\n",__FILE__,__LINE__,func,rw,in);
	_get_env_data(&sys_cfg_data);
	
	if(rw==MODE_READ){
		switch(func){
			case COLOR_HUE:
				out=sys_cfg_data.hue;
				break;
			case COLOR_SATURATION:
				out=sys_cfg_data.saturation;
				break;	
			case COLOR_BRIGHTNESS:
				out=sys_cfg_data.brightness;
				break;
			case COLOR_CONTRAST:
				out=sys_cfg_data.constrast;
				break;
			case COLOR_SHARPNESS:
				out=sys_cfg_data.sharpness;
				break;
			default:
				out=0;
				break;
		}	
	}
	else{
		switch(func){
			case COLOR_HUE:
				sys_cfg_data.hue=in;
				_do_action(CMD_SET_HUE,&sys_cfg_data.hue);
				break;
			case COLOR_SATURATION:
				sys_cfg_data.saturation=in;
				_do_action(CMD_SET_SATURATION,&sys_cfg_data.saturation);
				break;
			case COLOR_BRIGHTNESS:
				sys_cfg_data.brightness=in;
				_do_action(CMD_SET_BRIGHTNESS,&sys_cfg_data.brightness);
				break;
			case COLOR_CONTRAST:
				sys_cfg_data.constrast=in;
				_do_action(CMD_SET_CONTRAST,&sys_cfg_data.constrast);
				break;
			case COLOR_SHARPNESS:
				sys_cfg_data.sharpness=in;
				_do_action(CMD_SET_SHARPNESS,&sys_cfg_data.sharpness);
				break;
			default:
				out=0;
				break;
		}
		_save_env_data(&sys_cfg_data);
	}
	//printf("%s,%d,out is %d\n",__FILE__,__LINE__,out);
	Swfext_PutNumber(out);
	
	SWFEXT_FUNC_END();
}

static int calc_wday(rtc_date_t *date)
{
	int year_n, month_n, day_n;
	year_n = date->year;
	month_n = date->month;
	day_n = date->day;
	if(month_n==1 || month_n==2)
	{
		month_n = month_n+12;
		year_n--;
	}
	//calculate weekday:0->Sunday,1->Monday,2->Tuesday,3->Wednesday,4->Thursday,5->Friday,6->Saturday
	date->wday = (day_n+1 + 2*month_n + 3*(month_n+1)/5 + year_n + year_n/4 - year_n/100 + year_n/400)%7;
	sysdbg_info("wday is %d\n",date->wday);
	return 0;
}


static int sysinfo_get_year(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(&date,NULL);
	Swfext_PutNumber(date.year);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_year(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(&date,NULL);
	date.year = Swfext_GetNumber();
	calc_wday(&date);
	tm_set_rtc(&date,NULL);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_month(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(&date,NULL);
	Swfext_PutNumber(date.month);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_month(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(&date,NULL);
	date.month= Swfext_GetNumber();
	calc_wday(&date);
	tm_set_rtc(&date,NULL);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_day(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(&date,NULL);
	Swfext_PutNumber(date.day);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_day(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(&date,NULL);
	date.day= Swfext_GetNumber();
	calc_wday(&date);
	tm_set_rtc(&date,NULL);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_wday(void * handle)
{
	rtc_date_t date;
	
	SWFEXT_FUNC_BEGIN(handle);
	tm_get_rtc(&date,NULL);
	Swfext_PutNumber(date.wday);
	
	SWFEXT_FUNC_END();
}


static int sysinfo_get_hour(void * handle)
{
	rtc_time_t time;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(NULL,&time);
	Swfext_PutNumber(time.hour);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_hour(void * handle)
{
	rtc_time_t time;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(NULL,&time);
	time.hour = Swfext_GetNumber();
	tm_set_rtc(NULL,&time);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_min(void * handle)
{
	rtc_time_t time;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(NULL,&time);
	Swfext_PutNumber(time.min);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_min(void * handle)
{
	rtc_time_t time;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(NULL,&time);
	time.min= Swfext_GetNumber();
	tm_set_rtc(NULL,&time);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_sec(void * handle)
{
	rtc_time_t time;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(NULL,&time);
	Swfext_PutNumber(time.sec);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_sec(void * handle)
{
	rtc_time_t time;
	
	SWFEXT_FUNC_BEGIN(handle);

	tm_get_rtc(NULL,&time);
	time.sec = Swfext_GetNumber();
	tm_set_rtc(NULL,&time);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_alarm_hour(void * handle)
{
	rtc_time_t time;

	SWFEXT_FUNC_BEGIN(handle);

	tm_get_alarm(NULL,&time);
	Swfext_PutNumber(time.hour);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_alarm_hour(void * handle)
{
	rtc_time_t time;

	SWFEXT_FUNC_BEGIN(handle);

	tm_get_alarm(NULL,&time);
	time.hour = Swfext_GetNumber();
	tm_set_alarm(NULL,&time);
	
	SWFEXT_FUNC_END();
}


static int sysinfo_get_alarm_min(void * handle)
{
	rtc_time_t time;

	SWFEXT_FUNC_BEGIN(handle);

	tm_get_alarm(NULL,&time);
	Swfext_PutNumber(time.min);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_alarm_min(void * handle)
{
	rtc_time_t time;

	SWFEXT_FUNC_BEGIN(handle);

	tm_get_alarm(NULL,&time);
	time.min = Swfext_GetNumber();
	tm_set_alarm(NULL,&time);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_alarm_enable(void * handle)
{
	int en=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	Swfext_PutNumber(en);

	SWFEXT_FUNC_END();
}

static int sysinfo_set_alarm_enable(void * handle)
{
	int en=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	en = Swfext_GetNumber();

	SWFEXT_FUNC_END();
}

static int sysinfo_get_alarm_ring(void * handle)
{
	int ring = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	Swfext_PutNumber(ring);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_alarm_ring(void * handle)
{
	int ring = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	ring = Swfext_GetNumber();
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_clock_mode(void * handle)
{
	int mode = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	_get_env_data(&sys_cfg_data);

	mode = sys_cfg_data.calendar_clock_mode;
	Swfext_PutNumber(mode);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_clock_mode(void * handle)
{
	int mode = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	mode = Swfext_GetNumber();
	sys_cfg_data.calendar_clock_mode = mode;
	_save_env_data(&sys_cfg_data);
	
	SWFEXT_FUNC_END();
}


static int sysinfo_get_auto_poweron_flag(void * handle)
{
	int onflag = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	onflag = sys_cfg_data.auto_power_on_enable;
	Swfext_PutNumber(onflag);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_poweron_flag(void * handle)
{
	int onflag = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	onflag = Swfext_GetNumber();
	sys_cfg_data.auto_power_on_enable = onflag;
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_auto_poweron_hour(void * handle)
{
	int onhour=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	onhour = sys_cfg_data.power_on_time.hour;
	Swfext_PutNumber(onhour);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_poweron_hour(void * handle)
{
	int onhour=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	onhour = Swfext_GetNumber();
	sys_cfg_data.power_on_time.hour = onhour;
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_auto_poweron_min(void * handle)
{
	int onmin=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	_get_env_data(&sys_cfg_data);
	onmin = sys_cfg_data.power_on_time.min;
	Swfext_PutNumber(onmin);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_poweron_min(void * handle)
{
	int onmin=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	onmin = Swfext_GetNumber();
	sys_cfg_data.power_on_time.min = onmin;
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();

	SWFEXT_FUNC_END();
}

static int sysinfo_get_auto_poweroff_flag(void * handle)
{
	int offflag = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	offflag = sys_cfg_data.auto_power_off_enable;
	Swfext_PutNumber(offflag);
	
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_poweroff_flag(void * handle)
{
	int offflag = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	offflag = Swfext_GetNumber();
	sys_cfg_data.auto_power_off_enable = offflag;
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_auto_poweroff_hour(void * handle)
{
	int offhour = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	offhour = sys_cfg_data.power_off_time.hour;
	Swfext_PutNumber(offhour);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_poweroff_hour(void * handle)
{
	int offhour = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	offhour = Swfext_GetNumber();
	sys_cfg_data.power_off_time.hour = offhour;
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();

	SWFEXT_FUNC_END();
}


static int sysinfo_get_auto_poweroff_min(void * handle)
{
	int offmin = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);


	_get_env_data(&sys_cfg_data);
	offmin = sys_cfg_data.power_off_time.min;
	Swfext_PutNumber(offmin);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_poweroff_min(void * handle)
{
	int offmin = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	offmin = Swfext_GetNumber();
	sys_cfg_data.power_off_time.min	 = offmin;
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();

	SWFEXT_FUNC_END();
}

static int sysinfo_set_auto_power_freq(void * handle)
{
	int freq=1;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	freq = Swfext_GetNumber();
	sys_cfg_data.auto_power_freq = auto_power_frequency[freq];
	printf("auto power freq is 0x%x\n",sys_cfg_data.auto_power_freq);
	_save_env_data(&sys_cfg_data);
	al_load_poweroff_info();

	SWFEXT_FUNC_END();
}

static int sysinfo_get_auto_power_freq(void * handle)
{
	int freq=1;
	int i;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	//freq = sys_cfg_data.auto_power_freq;
	//printf("get auto power freq is %d\n",sys_cfg_data.auto_power_freq);
	for(i=0;i<MAX_AUTO_POWER_FREQ;i++)
	{
		if(auto_power_frequency[i] == sys_cfg_data.auto_power_freq)
		{
			freq = i;
			//printf("get freq is %d\n",freq);
		}
	}
	Swfext_PutNumber(freq);
	
	SWFEXT_FUNC_END();
}


static int sysinfo_media_to_disk(void * handle)
{
	int  media,disk_type='z';

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [add translation]*/
	
	media = Swfext_GetNumber();
	sysdbg_info("Sorry This Function Do Nothing");
	Swfext_PutNumber(media);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_set_active_media(void * handle)
{
	int  media;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	media=Swfext_GetNumber();
	sys_active_media = media;
	sys_cfg_data.active_media_type = media;
	_save_env_data(&sys_cfg_data);
	
	SWFEXT_FUNC_END();
}

static int sysinfo_store_config(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	_store_env_data();
	
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
void ezcast_sysinfo_store_config(void )
{

	_store_env_data();

}
#endif
static int sysinfo_get_local_language(void * handle)
{
	int  lang = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	lang = sys_cfg_data.local_language_id;
	if(lang==SYSDEFAULT_LOCAL_LANGUAGE){
		lang = system_init_default_lang_idx();
		sysdbg_info("lang=%d",lang);
	}
		
	Swfext_PutNumber(lang);
	
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_sysinfo_get_local_language(void )
{
	int  lang = 0;
	struct sysconf_param sys_cfg_data;

	_get_env_data(&sys_cfg_data);
	lang = sys_cfg_data.local_language_id;
	if(lang==SYSDEFAULT_LOCAL_LANGUAGE){
		lang = system_init_default_lang_idx();
		sysdbg_info("lang=%d",lang);
	}
	return lang;
	
}
#endif
static int sysinfo_set_local_language(void * handle)
{
	int  lang = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	lang = Swfext_GetNumber();
	sys_cfg_data.local_language_id = lang;
	_save_env_data(&sys_cfg_data);

	_do_action(CMD_SET_LOCAL_LANGUAGE,&sys_cfg_data.local_language_id);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
void ezcast_sysinfo_set_local_language(int lang)
{
	struct sysconf_param sys_cfg_data;

	_get_env_data(&sys_cfg_data);
	sys_cfg_data.local_language_id = lang;
	_save_env_data(&sys_cfg_data);

	_do_action(CMD_SET_LOCAL_LANGUAGE,&sys_cfg_data.local_language_id);
}
#endif
static int sysinfo_get_language_total_num(void* handle)
{
	int langtotal=0;
	SWFEXT_FUNC_BEGIN(handle);
	langtotal = locale_get_lange_num(handle);
	Swfext_PutNumber(langtotal);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_language_idx2str(void* handle)
{
	int langidx=0;
	char *langstr=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	langidx = Swfext_GetNumber();
	langstr = (char*)locale_get_lange_str(langidx,handle);
	Swfext_PutString(langstr);
	SWFEXT_FUNC_END();
}
static int sysinfo_upgrade(void * handle)
{
	int temp = 0;
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	temp = Swfext_GetNumber(); //richardyu 0509
	sysdbg_info("begin cardupgrading...");
	ret = cardup_entry_main(temp); //richard 0509
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();
}
static int sysinfo_check_card_type(void * handle)
{
	int type = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	sysdbg_info("Sorry This Function Do Nothing");
	Swfext_PutNumber(cardin_type);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_check_card_status(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(sd_in);	
	
	SWFEXT_FUNC_END();
}
//1213 have two usb port,check udisk1_in ===>port 0 usb mass storage,
//udisk2_in =====>port1 usb mass storage
static int sysinfo_check_port_one_status(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(udisk1_in);	
	printf("debug %s %d udisk1_in:%d\n",__FUNCTION__,__LINE__,udisk1_in);
	SWFEXT_FUNC_END();
}
static int sysinfo_check_port_two_status(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(udisk2_in);	
	printf("debug %s %d udisk2_in:%d\n",__FUNCTION__,__LINE__,udisk2_in);
	SWFEXT_FUNC_END();
}
static int sysinfo_check_cfcard_status(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(cf_in);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_check_bluetooth_status(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(bluetooth_in);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_set_video_autoplay_status(void * handle)
{
	int status=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	status = Swfext_GetNumber();
	sys_cfg_data.video_autoplay = status;
	_save_env_data(&sys_cfg_data);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_get_video_autoplay_status(void * handle)
{
	int status=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	_get_env_data(&sys_cfg_data);
	status = sys_cfg_data.video_autoplay;
	Swfext_PutNumber(status);

	SWFEXT_FUNC_END();
}

static int sysinfo_set_default(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	resetConfigDefault();
	SWFEXT_FUNC_END();
}
static int sysinfo_get_active_work_path(void * handle)
{
	char path[8];
	char disk_type;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: [finish it]*/
	sysdbg_info("Sorry This Function Do Nothing");
	switch(sys_active_media)
	{
		default:
			sprintf(path,"/");
			break;
	}
	Swfext_PutString(path);
	
	SWFEXT_FUNC_END();	
}
static int  sysinfo_clear_key_msg(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
    	sysdbg_info("Sorry This Function Do Nothing");
	SWFEXT_FUNC_END();	
}
static int sysinfo_enable_printer(void * handle)
{
	int isenable=0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	isenable = Swfext_GetNumber();
	sys_cfg_data.printer_enable = isenable;
	_save_env_data(&sys_cfg_data);

	Swfext_PutNumber(0);
	
	SWFEXT_FUNC_END();	
}
static int sysinfo_get_private_disk_symbol(void * handle)
{
	char disk_type;
	
	SWFEXT_FUNC_BEGIN(handle);
	/** FIXME: [finish it]*/
	Swfext_PutString(disk_rootpath[PRIV_DISK]);
	
	SWFEXT_FUNC_END();	
}
/*
  *usb device identify :if gpio is high ,connect pc;
  */
static int check_usb_gpio_value(){

	char locate =-1;
	char locate1 = -1;
	#if AM_CHIP_ID ==1213
	#if AM_CHIP_MINOR==8251	
	get_gpio(26,&locate);  //1213 usb0 detect gpio
	locate1=0;
	#else	
	get_gpio(96,&locate);  //1213 usb0 detect gpio
	get_gpio(95,&locate1);//1213 usb1 detect gpio
	#endif
	printf("<%s %d>locate=%d locate1=%d\n",__FUNCTION__,__LINE__,locate,locate1);
	#else
	get_gpio(34,&locate);//1211 usb detect gpio
	locate1=0;
	printf("<%s %d>locate=%d locate1=%d\n",__FUNCTION__,__LINE__,locate,locate1);
	#endif	
	if(locate==1 && locate1 == 0)
		return 0; //detect1213  usb0 or 1211 usb
	else if(locate==0 && locate1 == 1)
		return 1; //detect 1213 usb1
	else
		return -1;  //nothing insert 
}
static int sysinfo_check_startup_ustatus(void *handle)
{
	int usbfd;
	int ret;
	unsigned char usb_status[4];

	int usb_data_type;
	unsigned char usb_data_type_array[4];
	SWFEXT_FUNC_BEGIN(handle);

	usbfd = open("/sys/devices/usb-uoc/b_plug",O_RDONLY);
	if(usbfd<0){
		printf("open usb device failed,ret =%d \n",usbfd);
	}
	memset(usb_status,0,sizeof(int));
	ret  = read(usbfd,usb_status,sizeof(int));
	close(usbfd);
	//Get usb data type
	usb_data_type = open("/sys/devices/usb-uoc/b_type",O_RDONLY);
	if(usb_data_type<0){
		printf("open usb_data_type failed,ret =%d \n",usb_data_type);
	}
	memset(usb_data_type_array,0,sizeof(int));
	ret  = read(usb_data_type,usb_data_type_array,sizeof(int));
	close(usb_data_type);


	printf("--usb type=%d \n",usb_data_type_array[0]);
	
	if(ret>0&&usb_status[0]=='1'  )
	{

		if(usb_data_type_array[0] =='1' )
		{
			printf("put mini usb mode  1\n");
			Swfext_PutNumber(1);
		}
		else
		{
			Swfext_PutNumber(0);
			//Swfext_PutNumber(2);
			printf("put mini usb mode  2\n");
		}

	}
	else
		Swfext_PutNumber(0);

	SWFEXT_FUNC_END();	

	
}
static int sysinfo_accept_pcconnect_info(void *handle)
{
	int plugtype;
	int ret;
	struct am_sys_msg msg;
	memset(&msg,0,sizeof(struct am_sys_msg));
	SWFEXT_FUNC_BEGIN(handle);
	plugtype = Swfext_GetNumber();
	//printf("%s %d=========\n",__FILE__,__LINE__);
	//sysdbg_info("#########Accept Pc Connect info######plugtype=%d",plugtype);
	ret = check_usb_gpio_value();
	printf("<%s %d>=======ret=%d\n",__FUNCTION__,__LINE__,ret);
	if(ret==0){
		msg.reserved[0] =0xa0; //bq add for 1213
	}
	else if(ret==1){
		msg.reserved[0] =0xa1; //bq add for 1213
	}else{
		if(UsbPort==USB_GROUP0)
			msg.reserved[0]=USB_GROUP0;
		else
			msg.reserved[0]=USB_GROUP1;

		UsbPort=0;
	}
	printf("get accept_pccinnect_info type = %d\n",plugtype);
	if(plugtype==OUT){//这个地方是断开PC连线需要做的处理
		/*FIXME:if you need to send call back msg,send it here*/
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_OUT;
		msg.dataload = DEVICE_MASS_STORAGE;	
		fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
		hotplug_put_msg(msg);
	}
	else if(plugtype==2){//这个地方是直接按ESC键回到主界面，没有断开PC连线所做的处理
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_OUT;
		msg.dataload = DEVICE_MASS_STORAGE;		
		fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
		hotplug_put_msg(msg);
	}
	else//这个地方是连接PC做的处理
	{
		msg.type = SYSMSG_HOTPLUG_CHECK;	
		msg.subtype = DEVICE_USB_IN;		
		fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
		hotplug_put_msg(msg);
	}
	//id_message = 0x00;//bq change
	SWFEXT_FUNC_END();		
}
#define DONGLE_NUM 10
static int sysinfo_accept_hotplug_info(void * handle)
{
	int mediatype = 0;
	int plugtype = -1;
	struct am_sys_msg msg;
	memset(&msg,0,sizeof(struct am_sys_msg));
	SWFEXT_FUNC_BEGIN(handle);
			
	mediatype = Swfext_GetNumber();
	plugtype = Swfext_GetNumber();

	msg.type = SYSMSG_HOTPLUG_CHECK;

	if(access("/sys/block/sda",F_OK)==0||access("/sys/block/sdb",F_OK)==0){
		printf("%s %d\n",__func__,__LINE__);
		if(identy_hcd_flag==1){
			msg.reserved[0] =0xa0;
		}else if(identy_hcd_flag==2)
			msg.reserved[0] =0xa1;
	}else{
	//	printf("%s %d\n",__func__,__LINE__);
		if(id_message==0xa0)
			msg.reserved[0] =0xa0; //bq add for 1213
		if(id_message == 0xa1)
			msg.reserved[0] =0xa1; //bq add for 1213
	}
	//printf("%s %d=========reserved:0x%0x\n",__FUNCTION__,__LINE__,msg.reserved[0] );
	//sysdbg_info("meidatype=%d,plugtype=%d,msg.type=0x%x,cardin_type=%d\n",mediatype,plugtype,msg.type,cardin_type);
printf("meidatype=%d,plugtype=%d,msg.type=0x%x,cardin_type=%d\n",mediatype,plugtype,msg.type,cardin_type);
	if(plugtype==OUT){
		if(mediatype == CARD_DISK){
			msg.subtype = (Card_OUT|cardin_type);
			goto ACCEPT_OUT1;
		}
		else if(mediatype == HARD_DISK){
			msg.subtype = CF_OUT;
			goto ACCEPT_OUT1;
		}
		else if(mediatype == USB_DISK1||mediatype == USB_DISK2
			||mediatype == USB_DISK3||mediatype == USB_DISK4 ||mediatype==DONGLE_NUM){
			msg.subtype = HOST_USB_OUT;;
			goto ACCEPT_OUT1;
		}
		else{
			printf("=============================%s %d\n",__func__,__LINE__);
			goto ACCEPT_OUT2;
		}
	}
	else if(plugtype==IN){
		if(mediatype == CARD_DISK){
			msg.subtype = (Card_IN|cardin_type);
			goto ACCEPT_OUT1;
		}
		else if(mediatype == HARD_DISK){
			msg.subtype = CF_IN;
			goto ACCEPT_OUT1;
		}
		else if(mediatype == DISK_NUM ||mediatype == USB_DISK1||mediatype == USB_DISK2
			||mediatype == USB_DISK3||mediatype == USB_DISK4 || mediatype==DONGLE_NUM){
			msg.subtype = HOST_USB_IN;;
			goto ACCEPT_OUT1;
		}
		else{
			goto ACCEPT_OUT2;
		}
	}
	else{
		goto ACCEPT_OUT2;
	}	
ACCEPT_OUT1:
	printf("%s %d=========type:%0x subtype:0x%0x reserved:%0x\n",__FUNCTION__,__LINE__,msg.type,msg.subtype,msg.reserved[0]);
	fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
	hotplug_put_msg(msg);	
ACCEPT_OUT2:
	//fprintf(stderr,"function:%s,line:%d\n",__FUNCTION__,__LINE__);
	SWFEXT_FUNC_END();	
}

int probox_save_outputmode(int outputmode,int outputformat)
{
	FILE *fp;
	char tmp[8]="";
	fp=fopen("/etc/8250probox.outputmode","w+");
	if(fp!=NULL)
	{
		printf("create 8250probox outputmode  file  \n");
		sprintf(tmp,"%d",outputmode);
		fputs(tmp,fp);
		fputs("\r\n",fp);
		memset(tmp,0,8);
		sprintf(tmp,"%d",outputformat);
		fputs(tmp,fp);
		fputs("\r\n",fp);
		printf("save output mode[%d],format[%d]",outputmode,outputformat);
		fclose(fp);
	}
	else{
		return -1;
	}

	return 1;

}
int probox_get_outputmode(int *outputmode,int *outputformat)
{

	
	FILE *fp;
	int exist;
	char tmp[8]="",tmp1[8]="";
	int output[2];
	int i;
	exist=access("/etc/8250probox.outputmode",0);
	if(exist!=0){
		*outputmode=-1;
		*outputformat=-1;
		return -1;
	}
	else{
		printf("read 8250probox outputmode config file\n");
		fp=fopen("/etc/8250probox.outputmode","r");
			if(fp!=NULL)
			{
				for(i=0;i<2;i++){
					fgets(tmp,8,fp);
					char *end = strchr(tmp, '\n');
					if(end != NULL){
					   char *end_r = strchr(tmp, '\r');
						if(end_r != NULL){
							   end = (end < end_r)?end:end_r;
							   int len = end - tmp;
								if(len>0&&len<8)
								strncpy(tmp1,tmp,len);
								tmp1[len]='\0';
					   }
					
					}

					//printf("index=%s\n",index);
					output[i]=atoi(tmp1);
					memset(tmp,0,8);
				}
				printf("output mode[%d],format[%d]",output[0],output[1]);
				*outputmode=output[0];
				*outputformat=output[1];
				fclose(fp);
			}
			
	}
	return 0;
}

static int sysinfo_get_outputmode(void * handle)
{
	int outputmode=-1,outputformat=-1;
	
	//char output_data[8];
	SWFEXT_FUNC_BEGIN(handle);
	int output_flag=Swfext_GetNumber();
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
	
	if(output_flag==0){
		Swfext_PutNumber(outputmode);
		printf("output outputmode=%d",outputmode);
	}
	else if(output_flag==1) {
		Swfext_PutNumber(outputformat);
		printf("output outputformat=%d",outputformat);

	}
	else {

		Swfext_PutNumber(-1);
		printf("invalid input parameter!!!\n");


	}
	
	SWFEXT_FUNC_END();	

}
static int sysinfo_save_outputmode(void * handle)
{
	int outputmode=-1,outputformat=-1;
	SWFEXT_FUNC_BEGIN(handle);
	outputmode=Swfext_GetNumber();
	outputformat=Swfext_GetNumber();
	probox_save_outputmode(outputmode,outputformat);
	printf("[%s,%d] ouputmode=%d,outputformat=%d",__func__,__LINE__,outputmode,outputformat);
	SWFEXT_FUNC_END();	

}

static int sysinfo_set_HDMI_mode(void * handle)
{
	int hmode = 0;
	//struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	hmode = Swfext_GetNumber();
	/*if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}*/

	sysdbg_info("output enable is 0x%x\n",output_enable_now);
	output_prev_enable = output_enable_now;
	if((output_enable_now & HDMI_ENABLE)!=0)
	{
		sysdbg_info("HDMI Output is enable\n");
		output_prev_mode = output_mode_now;
		if(_exec_hdmi_mode(hmode)!=0)
		{
			sysdbg_err("Sorry _exec_hdmi_mode Error!");
		}
	}
	else if((output_enable_now & CVBS_ENABLE)!=0)
	{
		sysdbg_info("CVBS Output is enable\n");
		_do_action(CMD_SET_CVBS_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
		/*sys_cfg_data.cvbs_mode = hmode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	else if((output_enable_now & YPBPR_ENABLE)!=0)
	{
		sysdbg_info("YPbPr Output is enable\n");
		_do_action(CMD_SET_YPBPR_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
		/*sys_cfg_data.ypbpr_mode = hmode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	else if((output_enable_now & VGA_ENABLE)!=0)
	{
		sysdbg_info("VGA Output is enable\n");
		_do_action(CMD_SET_VGA_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
		/*sys_cfg_data.vga_mode = hmode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	sysdbg_info("set previous output enable is 0x%x, mode is %d\n",output_prev_enable, output_prev_mode);
	_get_screen_size();
sysinfo_set_HDMI_mode_out:
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
 int ezcast_set_output_mode(int output_mode){
	 //struct sysconf_param sys_cfg_data;
	
	 sysdbg_info("sysinfo--output mode is %d\n",output_mode);
	 /*if(_get_env_data(&sys_cfg_data)!=0)
	 {
		 sysdbg_err("Sorry Read System Data Error!");
	 }
	 else*/
	 {
		 output_enable_now = output_mode;
		 output_mode_now = SYSDEFAULT_SCREEN_OUTPUT_MODE;
		 //sys_cfg_data.output_enable= output_mode;
	 }
	 /*if(_save_env_data(&sys_cfg_data)!=0)
	 {
		 sysdbg_err("Sorry Write System Data Error!");
	 }*/
	
 }

 void ezcast_sysinfo_set_HDMI_mode(int hmode)
{

	//struct sysconf_param sys_cfg_data;

	/*if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}*/

	sysdbg_info("output enable is 0x%x\n",output_enable_now);
	output_prev_enable = output_enable_now;
	if((output_enable_now & HDMI_ENABLE)!=0)
	{
		sysdbg_info("HDMI Output is enable\n");
		output_prev_mode = output_mode_now;
		if(_exec_hdmi_mode(hmode)!=0)
		{
			sysdbg_err("Sorry _exec_hdmi_mode Error!");
		}
	}
	else if((output_enable_now & CVBS_ENABLE)!=0)
	{
		sysdbg_info("CVBS Output is enable\n");
		_do_action(CMD_SET_CVBS_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
		/*sys_cfg_data.cvbs_mode = hmode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	else if((output_enable_now & YPBPR_ENABLE)!=0)
	{
		sysdbg_info("YPbPr Output is enable\n");
		_do_action(CMD_SET_YPBPR_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
		/*sys_cfg_data.ypbpr_mode = hmode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	else if((output_enable_now & VGA_ENABLE)!=0)
	{
		sysdbg_info("VGA Output is enable\n");
		_do_action(CMD_SET_VGA_MODE,&hmode);
		screen_output_data.screen_output_true= 1;
		screen_output_data.screen_output_mode = hmode;
		output_prev_mode = output_mode_now;
		output_mode_now = hmode;
		/*sys_cfg_data.vga_mode = hmode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	sysdbg_info("set previous output enable is 0x%x, mode is %d\n",output_prev_enable, output_prev_mode);
	_get_screen_size();
}
#endif
static int sysinfo_get_HDMI_mode(void * handle)
{
	int hmode = 0;
	//struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	/*if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}*/
	if((output_enable_now & HDMI_ENABLE)!=0)
	{
		sysdbg_info("HDMI Output is enable\n");
		hmode = output_mode_now;
		sysdbg_info("the HDMI mode is %d\n",hmode);
	}
	else if((output_enable_now & CVBS_ENABLE)!=0)
	{
		sysdbg_info("CVBS Output is enable\n");
		hmode = output_mode_now;
		sysdbg_info("the CVBS mode is %d\n",hmode);
	}
	else if((output_enable_now & YPBPR_ENABLE)!=0)
	{
		sysdbg_info("YPbPr Output is enable\n");
		hmode = output_mode_now;
		sysdbg_info("the YPbPr mode is %d\n",hmode);
	}
	else if((output_enable_now & VGA_ENABLE)!=0)
	{
		sysdbg_info("VGA Output is enable\n");
		hmode = output_mode_now;
		sysdbg_info("the VGA mode is %d\n",hmode);
	}
	Swfext_PutNumber(hmode);	
	SWFEXT_FUNC_END();	
}
static int sysinfo_set_output_prev_mode(void * handle)
{
	//struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	/*if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}*/
	sysdbg_info("previous output enable is 0x%x, mode is %d\n",output_prev_enable,output_prev_mode);
	//sys_cfg_data.output_enable = output_prev_enable;
	output_enable_now = output_prev_enable;
	if((output_prev_enable & HDMI_ENABLE)!=0)
	{
		sysdbg_info("HDMI Output is enable\n");
		if(_exec_hdmi_mode(output_prev_mode)!=0)
		{
			sysdbg_err("Sorry _exec_hdmi_mode Error!");
		}
	}
	else if((output_prev_enable & CVBS_ENABLE)!=0)
	{
		sysdbg_info("CVBS Output is enable\n");
		_do_action(CMD_SET_CVBS_MODE,&output_prev_mode);
		screen_output_data.screen_output_true= 1;
		output_mode_now = output_prev_mode;
		/*sys_cfg_data.ypbpr_mode = output_prev_mode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
		}*/
	}
	else if((output_prev_enable & YPBPR_ENABLE)!=0)
	{
		sysdbg_info("YPbPr Output is enable\n");
		_do_action(CMD_SET_YPBPR_MODE,&output_prev_mode);
		screen_output_data.screen_output_true= 1;
		output_mode_now = output_prev_mode;
		/*sys_cfg_data.ypbpr_mode = output_prev_mode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
			goto sysinfo_set_output_prev_mode_out;
		}*/
	}
	else if((output_prev_enable & VGA_ENABLE)!=0)
	{
		sysdbg_info("VGA Output is enable\n");
		_do_action(CMD_SET_VGA_MODE,&output_prev_mode);
		screen_output_data.screen_output_true= 1;
		output_mode_now = output_prev_mode;
		/*sys_cfg_data.vga_mode = output_prev_mode;
		if(_save_env_data(&sys_cfg_data)!=0)
		{
			sysdbg_err("Sorry Write System Data Error!");
			goto sysinfo_set_output_prev_mode_out;
		}*/
	}
	_get_screen_size();
sysinfo_set_output_prev_mode_out:
	SWFEXT_FUNC_END();
}
static int sysinfo_set_output_mode(void * handle){
	char output_mode=0;
	//struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	output_mode = Swfext_GetNumber();
	sysdbg_info("sysinfo--output mode is %d\n",output_mode);
	/*if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}
	else*/
	{
		output_enable_now = output_mode;
		output_mode_now = SYSDEFAULT_SCREEN_OUTPUT_MODE;
		//sys_cfg_data.output_enable= output_mode;
	}
	/*if(_save_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Write System Data Error!");
	}*/
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_output_mode(void * handle){
	char output_mode = 0;
	//struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	/*if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}
	else*/
	{
		sysdbg_info("sysinfo--output_enable_now is %d\n",output_enable_now);
		output_mode = output_enable_now;
	}
sysinfo_get_output_mode_out:
	Swfext_PutNumber(output_mode);
	SWFEXT_FUNC_END();
}
int system_get_screen_para(int cmd)
{
	int rtn = 0;
	switch(cmd){
		case CMD_GET_SCREEN_WIDTH://return cur screen width
			rtn = screen_output_data.screen_output_width;
			break;
		case CMD_GET_SCREEN_HEIGHT://return cur screen height
			rtn = screen_output_data.screen_output_height;
			break;
		default:
			sysdbg_err("Cmd Error!!!!!!!!!!");	
			break;
	}
	return rtn;
}
static int sysinfo_get_curscreen_param(void * handle){

	int paramindex;
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	paramindex = Swfext_GetNumber();

	rtn = system_get_screen_para(paramindex);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_multisector(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);

	//Swfext_PutNumber(multisector_num);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_cardup_status(void*handle)
{
	int status;
	SWFEXT_FUNC_BEGIN(handle);
	status = cardup_get_status();
	Swfext_PutNumber(status);
	SWFEXT_FUNC_END();
}
extern char * ota_get_local_version();
#define VersionLenth 20
static int sysinfo_get_ota_local_version(void *handle)
{
	char localVersion[VersionLenth];
	
	SWFEXT_FUNC_BEGIN(handle);
	
	bzero(localVersion,VersionLenth);
	strncpy(localVersion,ota_get_local_version(),VersionLenth);
	
	Swfext_PutString(localVersion);
	SWFEXT_FUNC_END();

}
#if WEBSETTING_ENABLE
char * ezcast_sysinfo_get_ota_local_version(void)
{
	static char localVersion[VersionLenth];
	
	
	bzero(localVersion,VersionLenth);
	memset(localVersion,0,sizeof(localVersion));
	strncpy(localVersion,ota_get_local_version(),VersionLenth);
	
	return localVersion;

}
#endif
static int sysinfo_get_ota_status(void*handle)
{
	int status;
	SWFEXT_FUNC_BEGIN(handle);
	status = ota_get_status();
	Swfext_PutNumber(status);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_sysinfo_get_ota_status(void)
{
	int status;
	status = ota_get_status();
	return status;
}
#endif
static int sysinfo_get_auto_screenoff_flag(void * handle)
{
	int screenoffflag = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	printf("%s\n",__FUNCTION__);

	_get_env_data(&sys_cfg_data);
	screenoffflag = sys_cfg_data.auto_screen_off_enable;
	Swfext_PutNumber(screenoffflag);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_set_auto_screenoff_flag(void * handle)
{
	int screenoffflag = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
	
	printf("%s\n",__FUNCTION__);
	_get_env_data(&sys_cfg_data);
	screenoffflag = Swfext_GetNumber();
	sys_cfg_data.auto_screen_off_enable = screenoffflag;
	_save_env_data(&sys_cfg_data);
	al_load_screenoff_info();
	
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_auto_screenoff_time(void * handle)
{
	int screenofftime = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	screenofftime = sys_cfg_data.screen_off_time;
	Swfext_PutNumber(screenofftime);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_set_auto_screenoff_time(void * handle)
{
	int screenofftime = 0;
	struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);

	_get_env_data(&sys_cfg_data);
	screenofftime = Swfext_GetNumber();
	sys_cfg_data.screen_off_time = screenofftime;
	_save_env_data(&sys_cfg_data);
	al_load_screenoff_info();

	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
#ifdef MODULE_CONFIG_PLL
static int sysinfo_change_pll(void *handle)
{
	int pll_clock=0;
	int rtn =-1;
	SWFEXT_FUNC_BEGIN(handle);
	sysdbg_info("Chang PLL---------------");
	pll_clock= Swfext_GetNumber();

	if(pll_clock==DDR_LOW_PLL ||pll_clock==DDR_HIGH_PLL)
		rtn = pll_change_pll(pll_clock);
	else 
		sysdbg_err("PLL Clock ERROR");
	
	if(rtn==0)
		sysdbg_info("Chang PLL OK");
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
#else
static int sysinfo_change_pll(void *handle)
{
	int pll_clock=0;
	int rtn =-1;
	SWFEXT_FUNC_BEGIN(handle);
	sysdbg_info("Chang PLL--------Do nothing-------");
	pll_clock= Swfext_GetNumber();
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
#endif
/**
* APIs for getting information about battery.
*/
#define SYSINFO_BETTERY_STAT_PATH "/sys/class/power_supply/am7x-battery/status"
#define SYSINFO_BETTERY_VOLTAGE_PATH "/sys/class/power_supply/am7x-battery/voltage_now"

/**
* @brief Get battery charging status.
*
* @return The charging status:
    - UNKNOWN
    - CHARGING
    - DISCHARGING
    - NOT_CHARGING
    - FULL
*/
static int sysinfo_get_battery_charge_status(void *handle)
{
	FILE *fp;
	char stat[32]={0};
	char *s=NULL;

	SWFEXT_FUNC_BEGIN(handle);

	fp = fopen(SYSINFO_BETTERY_STAT_PATH,"r");
	if(fp){
		s = fgets(stat, 32, fp);
		if(s){
			int i;
			int len;

			len = strlen(stat);
			/// remove the \n
			for(i=0;i<len;i++){
				if(stat[i] == '\n'){
					stat[i] = 0;
					break;
				}
			}
		}
		fclose(fp);
	}

	if(strlen(stat) <= 0){
		sprintf(stat,"%s","UNKNOWN");
	}

	Swfext_PutString(stat);

	SWFEXT_FUNC_END();
}
/**
* @brief Get battery current voltage.
*
* @return A string reprent the current voltage.
*/
#if AM_CHIP_ID == 1211 || AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213
#define SYSINFO_BETTERY_VOL_TABLE_SIZE 16
char voltage_map_table[SYSINFO_BETTERY_VOL_TABLE_SIZE][32] =
{
	"3",
	"3.076923077",
	"3.153846154",
	"3.230769231",
	"3.307692308",
	"3.384615385",
	"3.461538462",
	"3.538461538",
	"3.615384615",
	"3.692307692",
	"3.769230769",
	"3.846153846",
	"3.923076923",
	"4",
	"4.076923077",
	"4.153846154"
};
#else
#define SYSINFO_BETTERY_VOL_TABLE_SIZE 16
char voltage_map_table[SYSINFO_BETTERY_VOL_TABLE_SIZE][32] =
{
	"3",
	"3.076923077",
	"3.153846154",
	"3.230769231",
	"3.307692308",
	"3.384615385",
	"3.461538462",
	"3.538461538",
	"3.615384615",
	"3.692307692",
	"3.769230769",
	"3.846153846",
	"3.923076923",
	"4",
	"4.076923077",
	"4.153846154"
};
#endif
static int sysinfo_get_battery_voltage(void *handle)
{
	FILE *fp;
	int voltage=-1;

	SWFEXT_FUNC_BEGIN(handle);

	fp = fopen(SYSINFO_BETTERY_VOLTAGE_PATH,"r");
	if(fp){
		fscanf(fp,"%d",&voltage);
		fclose(fp);
	}

	if((voltage>=0) && (voltage<SYSINFO_BETTERY_VOL_TABLE_SIZE)){
		Swfext_PutString((char *)&voltage_map_table[voltage]);
	}
	else{
		Swfext_PutString("Error Voltage");
	}

	SWFEXT_FUNC_END();
}
/**
@brief get the global value 
@param[in] cmd: get num or get str, see CMD_GET_GLONUM
@param[in] index: the index of the num or str
@param[out] status: if the status value ==0, it means get value failed, else it is success
***/
void * sys_get_globalvalue(int cmd,int index,char * status)
{
	void *value=NULL;
	struct sysconf_param sys_info;
	*status = 0;
	if(_get_env_data(&sys_info)==0){
		if(sys_info.is_gloval_valid){
			if(cmd==CMD_GET_GLONUM){
				if(index<GLOVAL_VALUE_NUMS_NUM){
					value = (void*)(int)sys_info.glo_val.nums[index];
					*status = 1;
				}
				else
					sysdbg_err("Num index=%s,out of range,MAX<%d",index,GLOVAL_VALUE_NUMS_NUM);
			}
			else if(cmd==CMD_GET_GLOSTR){
				if(index<GLOVAL_VALUE_STRS_NUM){
					value = (void*)sys_info.glo_val.strs[index];
					*status = 1;
				}
				else
					sysdbg_err("Str index=%s,out of range,MAX<%d",index,GLOVAL_VALUE_STRS_NUM);
			}
		}
	}
	return value;
}
static int  sysinfo_get_globalvalue(void *handle)
{
	int cmd=0;
	char status=0;
	int index=0;
	
	void * value=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	cmd = Swfext_GetNumber();
	index = Swfext_GetNumber();
	memset(global_value_str,0,GLOVAL_VALUE_STR_LEN);
	value = sys_get_globalvalue(cmd,index,&status);
	if(cmd==CMD_GET_GLONUM){
		if(status){
			sprintf(global_value_str,"%d",(globalnum_type)((int)value));
		}
	}
	else if(cmd==CMD_GET_GLOSTR){
		if(status){
			memcpy(global_value_str,value,GLOVAL_VALUE_STR_LEN);
		}
	}
	Swfext_PutString(global_value_str);
	SWFEXT_FUNC_END();
}
/*************************************
*@check usb status
*@ return  0--no connect
               1--pc link with DP DM
               2--PC link without DP DM
               3--adapter 
*************************************/
static int sysinfo_check_usb_connect_status(void *handle)
{
	int usbfd;
	int ret;
	unsigned char usb_status[4];
	SWFEXT_FUNC_BEGIN(handle);

	usbfd = open("/sys/devices/usb-uoc/b_type",O_RDONLY);
	if(usbfd<0){
		printf("open usb device failed,ret =%d \n",usbfd);
	}
	memset(usb_status,0,sizeof(int));
	ret  = read(usbfd,usb_status,sizeof(int));
	if(ret>0&&usb_status[0]=='1')       // pc link with DP DM
	{
		Swfext_PutNumber(1);
		printf("##usb in with DP DM##\n");
	}
	else if(ret>0&&usb_status[0]=='2')//PC link but no DP DM
	{
		Swfext_PutNumber(2);
		printf("##usb in without DP DM##\n");
	}
	else if(ret>0&&usb_status[0]=='3')//adapter
	{
		Swfext_PutNumber(3);
		printf("##adapter in ##\n");
	}
	else 
	{
		Swfext_PutNumber(0);      //no connect
		printf("##no connect ##\n");
	}

	SWFEXT_FUNC_END();	
}
static int sysinfo_Standby(void *handle)
{
	int rtn = 0;
	SWFEXT_FUNC_BEGIN(handle);
	sysdbg_info("now is to enter standby");
	#if EZMUSIC_ENABLE
	SysCGI_priv.Bye_bye_flag=1;
	#endif
	rtn = auto_power_off(IRE_DC_MODE|EXT_DC_MODE);
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
static int sysinfo_access(void *handle)
{	
	char * path=NULL;
	int mode=0;
	int rtn=0;
	SWFEXT_FUNC_BEGIN(handle);
	path = Swfext_GetString();
	mode = Swfext_GetNumber();
	rtn = access(path,mode); 
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
static int sysinfo_writeUart_Str(void *handle)
{
	int fd;
	int nBytes = 0;
	char* writestr = NULL;
	char tmpbuf[50];
	int i;
	
	SWFEXT_FUNC_BEGIN(handle);
	writestr = Swfext_GetString();
	nBytes = Swfext_GetNumber();
	if((writestr != NULL) && (nBytes != 0)){
		for(i = 0; i < nBytes; i++){
			tmpbuf[i] = writestr[i];
		}
		fd = open("/dev/uartcom",O_RDWR);
		if(fd<0){
			printf("cant open uartcom device\n");
		}
		else{
			write(fd,tmpbuf,nBytes);
			close(fd);
		}
	}
	SWFEXT_FUNC_END();
}
#if 0
static int sysinfo_Softap(void *handle)
{
	int rtn = 0;
	char callbuf[50]={0};
	SWFEXT_FUNC_BEGIN(handle);
	#if(SOFTAP==LAN)
	memset(callbuf,0,50);
	sprintf(callbuf,"sh %s/dmlan.sh",AM_CASE_SC_DIR);
	system(callbuf);
	wifi_subdisplay_start();
	printf("start lan display\n");
	#else
	sprintf(callbuf,"mkdir /mnt/vram/softap/");
	system(callbuf);
	memset(callbuf,0,50);
	#if(SOFTAP==RALINK)
	if(access("/mnt/vram/softap/RT2870AP.dat",F_OK)==-1)
	{
		sprintf(callbuf,"cp /etc/ralink.dat /mnt/vram/softap/RT2870AP.dat");
		system(callbuf);
		memset(callbuf,0,50);
	}
	sprintf(callbuf,"sh %s/ralink_softap.sh",AM_CASE_SC_DIR);
	#endif
	#if(SOFTAP==REALTEK)
	if(access("/mnt/vram/softap/rtl_hostapd.conf",F_OK)==-1)
	{
		sprintf(callbuf,"cp /etc/realtek.conf /mnt/vram/softap/rtl_hostapd.conf");
		system(callbuf);
		memset(callbuf,0,50);
	}
	sprintf(callbuf,"sh %s/realtek_softap.sh",AM_CASE_SC_DIR);
	#endif
	sysdbg_info("the call is %s",callbuf);
	rtn = system(callbuf);
	sysdbg_info("the rtn is %d",rtn);
	#if 1
	#ifdef MODULE_CONFIG_WIFI_SUBDISPLAY
	wifi_subdisplay_start();
	printf("start wifi display\n");
	#endif	/** MODULE_CONFIG_WIFI_SUBDISPLAY */
	#endif
	#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
#else
int wifi_connect_flag = 0;
static int sysinfo_Softap(void *handle)
{
	int rtn = 0;
	char callbuf[50]={0};
	SWFEXT_FUNC_BEGIN(handle);
	if(wifi_connect_flag){
		goto softend;
	}
	#if(SOFTAP==RALINK)
	if(access("/mnt/vram/softap/RT2870AP.dat",F_OK)==-1)
	{
		sprintf(callbuf,"mkdir /mnt/vram/softap/");
		system(callbuf);
		memset(callbuf,0,50);
		
		sprintf(callbuf,"cp /etc/ralink.dat /mnt/vram/softap/RT2870AP.dat");
		system(callbuf);
		memset(callbuf,0,50);
	}
	sprintf(callbuf,"sh %s/ralink_softap.sh",AM_CASE_SC_DIR);
	#endif
	#if(SOFTAP==REALTEK)
	if(access("/mnt/vram/softap/rtl_hostapd.conf",F_OK)==-1)
	{
		sprintf(callbuf,"mkdir /mnt/vram/softap/");
		system(callbuf);
		memset(callbuf,0,50);
		
		sprintf(callbuf,"cp /etc/realtek.conf /mnt/vram/softap/rtl_hostapd.conf");
		system(callbuf);
		memset(callbuf,0,50);
	}
	sprintf(callbuf,"sh %s/realtek_softap.sh",AM_CASE_SC_DIR);
	#endif
	sysdbg_info("the call is %s",callbuf);
	rtn = system(callbuf);
	sysdbg_info("the rtn is %d",rtn);
	wifi_connect_flag = 1;
softend:
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
static int sysinfo_NetDisplay(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int rtn=-1;
	#ifdef MODULE_CONFIG_WIFI_SUBDISPLAY
#if EZCAST_MAC_LIMIT
	if(isNetDisplayEnable() == 0)
	{
		EZCASTLOG("EZCast is be limited!!!\n");
	}
	else
#endif
	{
#if EZCAST_ENABLE
		// Set neighbour stream;
		int nb = ezCastGetNumConfig("Neighbour");
		if(nb > 0){
			EZCASTLOG("------ Set neighbour enable!!!\n");
			ezNeighbourSetAutoCast(1);
		}else{
			EZCASTLOG("------ Set neighbour disable!!!\n");
			ezNeighbourSetAutoCast(0);
		}
#endif
		rtn=wifi_subdisplay_start();
	}
	#endif	/** MODULE_CONFIG_WIFI_SUBDISPLAY */
	
#if FUN_WIRE_ENABLE
	if(android_in_status != 0 || ios_in_status != 0)
		libusb_handle();
#endif
	Swfext_PutNumber(rtn);
	SWFEXT_FUNC_END();
}
static int sysinfo_StopNetDisplay(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	//printf("befort stop subdisplay!\n");
#if EZCAST_ENABLE
	ezcastCleanIcon();
#endif

	wifi_subdisplay_end();
	
#if (EZWIRE_CAPTURE_ENABLE != 0)
	ezConfigSetPlugPlay(0);
#endif
	
#if FUN_WIRE_ENABLE
	if(android_in_status != 0)
		system(AM_CASE_SC_DIR"/usb_process.sh host_adb_stop");
#endif

	//printf("systeminfo subdisplay end!\n");
	SWFEXT_FUNC_END();
}
#if EZMUSIC_ENABLE
void ezcast_sysinfo_StopNetDisplay(void)
{
	//printf("befort stop subdisplay!\n");
	wifi_subdisplay_end();
	//printf("systeminfo subdisplay end!\n");
}
#endif
#if FUN_WIRE_ENABLE
int setAndroidInStatus(int status)
{
	android_in_status = !!status;

	return android_in_status;
}
int setiOSInStatus(int status)
{
	ios_in_status = !!status;

	return ios_in_status;
}
#endif
static int sysinfo_LANStart(void *handle){
	char callbuf[50]={0};
	SWFEXT_FUNC_BEGIN(handle);
	memset(callbuf,0,50);
	sprintf(callbuf,"sh %s/dmlan.sh",AM_CASE_SC_DIR);
	system(callbuf);
	SWFEXT_FUNC_END();
}
extern int usbdisplay_sel;
static int sysinfo_SetForUSBDisplay(void *handle){
	int sel = 0;
	SWFEXT_FUNC_BEGIN(handle);
	sel = Swfext_GetNumber();
	usbdisplay_sel = sel;
	SWFEXT_FUNC_END();
}
#endif
enum media_type_e{
	MT_SD=1,
	MT_CF,
	MT_USB,
	MT_DONGLE,
};

static int sysinfo_check_media_status(void *handle){
	enum media_type_e media_type=0;
	int status=0;
	SWFEXT_FUNC_BEGIN(handle);
	media_type = Swfext_GetNumber();
	//sysdbg_info("Check media status media type===%d",media_type);
	switch(media_type){
		case MT_SD:
			status = sd_in;
			break;
		case MT_CF:
			status = cf_in;
			break;
		case MT_USB:
			status = udisk1_in;
			break;
		case MT_DONGLE:
			status = udisk2_in;
			//status = wlan_in;
			break;
		default:
			sysdbg_err("ERR: media type = %d is not support",media_type);
	}
	Swfext_PutNumber(status);
	SWFEXT_FUNC_END();
}
static int sysinfo_set_audio_autoplay_status(void * handle){
    int status=0;
    struct sysconf_param sys_cfg_data;
    SWFEXT_FUNC_BEGIN(handle);
    _get_env_data(&sys_cfg_data);
    status = Swfext_GetNumber();
    sys_cfg_data.audio_autoplay = status;
    _save_env_data(&sys_cfg_data);
    SWFEXT_FUNC_END();
}
static int sysinfo_get_audio_autoplay_status(void * handle){
    int status=0;
    struct sysconf_param sys_cfg_data;
    SWFEXT_FUNC_BEGIN(handle);
    _get_env_data(&sys_cfg_data);
    status = sys_cfg_data.audio_autoplay;
    Swfext_PutNumber(status);
    SWFEXT_FUNC_END();
}
static int sysinfo_check_ram(void *handle)//add by richard
{	
	int mem_used=0;
	SWFEXT_FUNC_BEGIN(handle);
	mem_used=SWF_MemCheck(1);
	Swfext_PutNumber(mem_used);
	SWFEXT_FUNC_END();
}
static int sysinfo_put_wifi_mode(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	
	wifi_mode=Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

/*********************CGI FUI START*********************************************************/
#if WEBSETTING_ENABLE
void set_ap_total(int tmp_app_total)
{
    app_total = tmp_app_total;
}

int get_ap_total()
{
    return app_total;
}

void arrange_wifi_list(int tmp_total_app)
{
	char temp[50]={0};
	int i,temp_i,j,temp_ssid_signal,arrange_index;
	char *connected_ssid=NULL;
	int wifi_mode_get_from_case=ezcast_sysinfo_get_wifi_mode();
	if((wifi_mode_get_from_case==9)||(wifi_mode_get_from_case==14))//9:WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK   14:WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR
	{
		connected_ssid= ezcast_wifi_get_connected_ssid();
		temp_i=1;
	}	
	else
		temp_i=0;
	if(connected_ssid!=NULL)
	{	
		for(i=0;i<tmp_total_app;i++)
		{
			if((strcmp(connected_ssid,SysCGI_priv.ssid_str[i])==0)&&(i!=0))
			{
				strcpy(temp,SysCGI_priv.ssid_str[i]);
				memset(SysCGI_priv.ssid_str[i],0,sizeof(SysCGI_priv.ssid_str[i]));
				strcpy(SysCGI_priv.ssid_str[i],SysCGI_priv.ssid_str[0]);
				memset(SysCGI_priv.ssid_str[0],0,sizeof(SysCGI_priv.ssid_str[0]));
				strcpy(SysCGI_priv.ssid_str[0],temp);
				temp_ssid_signal=SysCGI_priv.ssid_signal[i];
				SysCGI_priv.ssid_signal[i]=SysCGI_priv.ssid_signal[0];
				SysCGI_priv.ssid_signal[0]=temp_ssid_signal;	
				int ssid_index_temp=SysCGI_priv.ssid_index[i];
				SysCGI_priv.ssid_index[i]=SysCGI_priv.ssid_index[0];
				SysCGI_priv.ssid_index[0]=ssid_index_temp;			
			}					
		}
	}
	if(temp_i==0)
		arrange_index=1;
	else 
		arrange_index=0;	
	for(j=temp_i;j<tmp_total_app;j++)
	{
		for(i=temp_i;i<tmp_total_app-j-arrange_index;i++)
		{
			if(SysCGI_priv.ssid_signal[i]< SysCGI_priv.ssid_signal[i+1])
			{
				memset(temp,0,sizeof(temp));
				strcpy(temp,SysCGI_priv.ssid_str[i]);
				memset(SysCGI_priv.ssid_str[i],0,sizeof(SysCGI_priv.ssid_str[i]));
				strcpy(SysCGI_priv.ssid_str[i],SysCGI_priv.ssid_str[i+1]);
				memset(SysCGI_priv.ssid_str[i+1],0,sizeof(SysCGI_priv.ssid_str[i+1]));
				strcpy(SysCGI_priv.ssid_str[i+1],temp);
				temp_ssid_signal=SysCGI_priv.ssid_signal[i];
				SysCGI_priv.ssid_signal[i]=SysCGI_priv.ssid_signal[i+1];
				SysCGI_priv.ssid_signal[i+1]=temp_ssid_signal;
				int ssid_index_temp=SysCGI_priv.ssid_index[i+1];
				SysCGI_priv.ssid_index[i+1]=SysCGI_priv.ssid_index[i];
				SysCGI_priv.ssid_index[i]=ssid_index_temp;	
			}
		}
	}
}
void show_wifi_list(void)
{
	int i= 0; 
	int total_app=0; 
	char * ssid= ""; 
	char *ssid_string=NULL;
	char wifiSignal[2]={0};
	char ssid_index[2]={0};
	int SSID_signal;

		total_app = ezcast_wifi_getscanresults(); 
	if (total_app<=0)
		return;
	printf("@@@@[%s][%d]total_app=%d\n",__FILE__,__LINE__,total_app);//string

	SysCGI_priv.ssid_string_for_cgi=(char *)malloc(total_app*128*sizeof(char));
	if(SysCGI_priv.ssid_string_for_cgi == NULL)
	{
		EZCASTLOG("[SysCGI_priv.ssid_string_for_cgi]malloc fail<%s>.", strerror(errno));
		return; 
	}
	memset(SysCGI_priv.ssid_string_for_cgi,0,sizeof(SysCGI_priv.ssid_string_for_cgi));

	SysCGI_priv.ssid_signal_string=(char *)malloc(total_app*16*sizeof(char));//signal
	if(SysCGI_priv.ssid_signal_string == NULL)
	{
		EZCASTLOG("[SysCGI_priv.ssid_string_for_cgi]malloc fail<%s>.", strerror(errno));
		return; 
	}
	memset(SysCGI_priv.ssid_signal_string,0,sizeof(SysCGI_priv.ssid_signal_string));

	SysCGI_priv.ssid_index_string=(char *)malloc(total_app*16*sizeof(char));//index
	if(SysCGI_priv.ssid_index_string == NULL)
	{
		EZCASTLOG("[SysCGI_priv.ssid_string_for_cgi]malloc fail<%s>.", strerror(errno));
		return; 
	}
	memset(SysCGI_priv.ssid_index_string,0,sizeof(SysCGI_priv.ssid_index_string));

	for(i=0; i <total_app; i++)
	{
		SysCGI_priv.ssid_signal[i]=ezcast_wifi_getsingal(i);	
		//printf("SysCGI_priv.ssid_signal[%d]=============%d,%d\n",i,SysCGI_priv.ssid_signal[i],__LINE__);
		ssid = ezcast_wifi_getssid(i);
		if(strlen(ssid))
		{
			memset(SysCGI_priv.ssid_str[i],0,128*sizeof(char));
			strncpy(SysCGI_priv.ssid_str[i],ssid,strlen(ssid));
			//printf("SysCGI_priv.ssid_str[%d]=============%s,%d\n",i,SysCGI_priv.ssid_str[i],__LINE__);
			SysCGI_priv.ssid_index[i]=i;
		}
		else
			total_app-=1;
	}
	printf("@@@@[%s][%d]total_app=%d\n",__FILE__,__LINE__,total_app);
	arrange_wifi_list(total_app);	
	for(i=0; i < total_app; i++)
	{
		strcat(SysCGI_priv.ssid_string_for_cgi,SysCGI_priv.ssid_str[i]);
		strcat(SysCGI_priv.ssid_string_for_cgi,"\n");		
		//printf("@@@@[%d]SysCGI_priv.ssid_str[%d]======[%s]\n",__LINE__,i,SysCGI_priv.ssid_str[i]);
		SSID_signal=floor(SysCGI_priv.ssid_signal[i]/25)+1;
		memset(wifiSignal,0,sizeof(wifiSignal));
		sprintf(wifiSignal,"%d",SSID_signal);	
		strcat(SysCGI_priv.ssid_signal_string,wifiSignal);
		strcat(SysCGI_priv.ssid_signal_string,"\n");	
		memset(ssid_index,0,sizeof(ssid_index));	
		sprintf(ssid_index,"%d",SysCGI_priv.ssid_index[i]);
		strcat(SysCGI_priv.ssid_index_string,ssid_index);
		strcat(SysCGI_priv.ssid_index_string,"\n");			
	}
		//printf("@@@@[%d]ssid_string_for_cgi======[[%s]]\n",__LINE__,SysCGI_priv.ssid_string_for_cgi);
}
#endif

extern int id_hcd_ko;//bq:id_hcd_ko=1 am7x_hcd use the dongle other id_hcd_ko=2 am7x_hcd_next use the wifi dongle
#if WEBSETTING_ENABLE
int ezcast_dongle_out_times=0;
int ezcast_sysinfo_get_wifi_mode(void)
{	
	int ret3=-1;
	int dongle_type_divided=dongle_get_device_changing();
	if(dongle_type_divided==RALINK_dongle){
		if(wifi_mode!=WIFI_DISABLE_MANUAL&&wifi_mode!=WIFI_CLIENT_GETAUTOIP_OK&&wifi_mode!=WIFI_CLIENT_GETAUTOIP_ERR&&wifi_mode!=WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK &&wifi_mode!=WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR){
			int ret1=access("/sys/module/rt5370sta",F_OK);
			int ret2=access("/sys/module/rtnet3070ap",F_OK);	
		//	int ret3=(id_hcd_ko==HCD_USE_WIFI)?access("/sys/module/am7x_hcd",F_OK):access("/sys/module/am7x_hcd_next",F_OK);
			if(id_hcd_ko == HCD_USE_WIFI) 
				ret3 = access("/sys/module/am7x_hcd",F_OK);
			if(id_hcd_ko == HCD_NEXT_USE_WIFI)
				ret3 = access("/sys/module/am7x_hcd_next",F_OK);
			//printf("ret3====%d +++ ret1====%d +++ ret2====%d\n",ret3,ret1,ret2);
			if(ret3==0){
				ezcast_dongle_out_times=0;
				if(wifi_mode!=WIFI_SOFTAP_ENABLE && wifi_mode!=WIFI_CLIENT_MODE_ENABLE)
					wifi_mode=WIFI_DONGLE_PLUG_IN;
				if(ret1==0)
					wifi_mode=WIFI_CLIENT_MODE_ENABLE;
				if(ret2==0)
					wifi_mode=WIFI_SOFTAP_ENABLE;
			}
			if((ret3!=0) && (wifi_mode != WIFI_DONGLE_PLUG_OUT)){
				//wifi_mode=WIFI_DONGLE_PLUG_OUT;
				ezcast_dongle_out_times++;
				if(ezcast_dongle_out_times<3)
					wifi_mode=WIFI_MODE_CHANGEING_FOR_RALINK;
				else{
					
					wifi_mode=WIFI_DONGLE_PLUG_OUT;
					ezcast_dongle_out_times=0;

				}
			
			}
			
		
		}	
	
	}
	//printf("debug %s %d wifi mode :%d\n",__func__,__LINE__,wifi_mode);
	return wifi_mode;
}

int ssid_connect_ok_show()
{
	int i= 0;
	char *ssid_string=NULL;
	char *temp=NULL;
	char *tmp_ssid= ezcast_wifi_get_connected_ssid();
	char *tmp_ip= ezcast_wifi_getIP();
	memset(SysCGI_priv.connect_status,0,sizeof(SysCGI_priv.connect_status));
	if(strlen(tmp_ssid) != 0)
	{
	   for(i=0;i<app_total;i++)
	    {
		if((strstr(SysCGI_priv.ssid_str[i],"\n")!= NULL)&&(strstr(SysCGI_priv.ssid_str[i],"O")==NULL))
		{
			if(temp==NULL)
				temp=(char *)SWF_Malloc(strlen(SysCGI_priv.ssid_str[i])*sizeof(char));
			memset(temp,0,sizeof(temp));
			strncpy(temp,SysCGI_priv.ssid_str[i],(strlen(SysCGI_priv.ssid_str[i])-strlen("\n")));
		       if(strcmp(temp,tmp_ssid)==0)
		        {
				SWF_Free(temp);
				temp=NULL;		        
				if(strlen(tmp_ip) == 0)//can not get ip,show !
				{
					sprintf(SysCGI_priv.connect_status,"%d",i);
					printf("connect_status=========%s,%s,%d\n",SysCGI_priv.connect_status,__FUNCTION__,__LINE__);
					return 3;
				}
				else//show connect ok flag
				{
					sprintf(SysCGI_priv.connect_status,"%d",i);
					printf("connect_status=========%s,%s,%d\n",SysCGI_priv.connect_status,__FUNCTION__,__LINE__);
					return 1;
				}
		        }	
			SWF_Free(temp);
			temp=NULL;  
		}
	       else if(strcmp(SysCGI_priv.ssid_str[i],tmp_ssid)==0)
	        {
			if(strlen(tmp_ip) == 0)//can not get ip,show !
			{
				sprintf(SysCGI_priv.connect_status,"%d",i);
				printf("connect_status=========%s,%s,%d\n",SysCGI_priv.connect_status,__FUNCTION__,__LINE__);
				return 3;
			}
			else//show connect ok flag
			{
				sprintf(SysCGI_priv.connect_status,"%d",i);
				printf("connect_status=========%s,%s,%d\n",SysCGI_priv.connect_status,__FUNCTION__,__LINE__);
				return 1;
			}
	        }
	    }
	}
	return 0;
}
void get_scanresults(void)
{
	char * g_hide_ssid= "Add network";
	static char temp_count=0;
	temp_count++;
	app_total=ezcast_wifi_getscanresults();
	if(app_total <= 1)
	{
		if(temp_count <= 4)
		{
			printf("%s,%d\n",__FUNCTION__,__LINE__);
			ezcast_wifi_startscan();
			app_total=ezcast_wifi_getscanresults();
		}
	}
	show_wifi_list(); 
}
void init_SysCGI_Variable(void)
{
	int i=0;
	for(i;i<50;i++)
	{
		SysCGI_priv.ssid_index[i]=0;
		SysCGI_priv.ssid_signal[i]=0;
	}
}
void wifi_start_scan()
{
	ezcast_wifi_startscan();
	app_total=ezcast_wifi_getscanresults();	
}
void open_start_wifi()
{
	int ret=0;	
	int dongle_type=0;
	int temp= 0; 
	int i= 0; 
	int j= 0; 
	init_SysCGI_Variable();
#if !EZCAST_LITE_ENABLE
	ezcast_wifi_open();
	ret=ezcast_wifi_start();
#endif
	ezcast_wifi_startscan();
	if(ret==0)
	{
		dongle_type=ezcast_wifi_get_dongle_type();
		if(dongle_type==2)
		{
			for(i = 0; i < 100; i++)
			{   
			        for(j = 0; j < 100; j++)
			        {
					temp++; 
					temp--; 
				}
			}
		}
	}
}
#endif
/************************CGI FUI END******************************************************/


#if (EZMUSIC_ENABLE&&GPIO_WIFI_ON_LED)||( MODULE_CONFIG_EZCASTPRO_MODE==8075)

#define GPIO_BASE 0xB01C0000
#define GPIO_WIFI24G_ON_LED  71
#define GPIO_WIFI5G_ON_LED  70
#define GPIO_LAN_ON_LED  69


int Probox_thttpdconf_port_change()
{
	
	FILE *fp = NULL;
	char buf[4096] ={0};
	int ret=0;
	if(access("/mnt/user1/softap/thttpd.conf",F_OK)!=0){
			printf("cp thttpd.conf!\n");
			system("cp -f /mnt/bak/thttpd.conf /mnt/user1/softap/thttpd.conf ");

	}
	fp = fopen("/mnt/user1/softap/thttpd.conf","r");
	if(fp == NULL){
		return -1;
	}
	ret=fread(buf, 1, 4096, fp);
	ret=fclose(fp);
	modify_line_realtek(buf,"port=","80\r");

	fp = fopen("/mnt/user1/softap/thttpd.conf","wb+");
	if(fp == NULL){
		return -1;
	}
	ret=fwrite(buf, 1, 4096, fp);
	ret=fflush(fp);
	int fd_write = fileno(fp);
	ret=fsync(fd_write);
	ret=fclose(fp);
	system("rm /etc/thttpd.conf");
	system("ln -sf /mnt/user1/softap/thttpd.conf  /etc/thttpd.conf ");
	system("sync");
	return 1;
}
int checkAndRecover_thttpdconf()
{
	/****check and recover file  thttpd.conf ****/
	char cmd[128]="";
	int ret=0;
	char conf_file[100]="/mnt/user1/softap/thttpd.conf";
	char confbak_file[100]="/mnt/bak/thttpd.conf";
	char conf_thttpd_file[100]="/etc/thttpd.conf";
	if(access(conf_file,F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"cp %s %s",confbak_file,conf_file);
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]recover thttpd.conf ok!!\n",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]recover thttpd.conf error!!\n",__FUNCTION__,__LINE__);
		system("sync");
	}
	/****check and recover rtl_hostapd_01.conf link file****/
	if(access(conf_thttpd_file,F_OK)!=0){
		memset(cmd,0,128);
		sprintf(cmd,"ln -s  %s %s",conf_file,conf_thttpd_file);
		ret=system(cmd);
		if(ret == 0)
			printf("[%s,%d]link thttpd.conf ok!!\n",__FUNCTION__,__LINE__);
		else
			printf("[%s,%d]link thttpd.conf error!!\n",__FUNCTION__,__LINE__);
		system("sync");
	}
	return 1;
}
int Probox_init()
{
	
	//Probox_thttpdconf_port_change();
	//checkAndRecover_thttpdconf();
	printf("Probox open bitmask \n");
	ezRemoteConfigUpdate("airdisk",0, "all");
	//ezRemoteConfigUpdate("aircontrol",0, "all");
	//for wifi led
	
	adbAudioInit();
	system("insmod /lib/modules/2.6.27.29/am7x_hcd.ko");
	usleep(100*1000);
	system("modprobe /lib/modules/2.6.27.29/snd-usb-audio.ko");

}
int Probox_set_led(int ledflag)
{
	switch(ledflag){
	case LED24G_ON:
			set_gpio(71,0);
			set_gpio(70,1);
			set_gpio(69,1);
			printf("2.4G LED ON \n");
			break;
	case LED24G_OFF:
			set_gpio(71,1);
			printf("2.4G LED OFF \n");
			break;
	case LED5G_ON:
			set_gpio(71,1);
			set_gpio(70,0);
			set_gpio(69,1);
			printf("5G LED ON \n");
			break;
	case LED5G_OFF:
			set_gpio(70,1);
			printf("5G LED OFF \n");
			break;
	case LEDLAN_ON:
			set_gpio(71,1);
			set_gpio(70,1);
			set_gpio(69,0);
			printf("LAN LED ON \n");
			break;
	case LEDLAN_OFF:
			set_gpio(69,1);
			printf("LAN LED OFF\n");
			break;
	default:
		set_gpio(71,1);
		set_gpio(70,1);
		set_gpio(69,1);
	}
	return 1;
}



/*
***************************************************************************
*
*Breathing light function*
*By Raoliang@20140427
*
***************************************************************************
*/
int hostapd24g_connected=0;
int hostapd5g_connected=0;
int wpa24g_connected=0;
int wpa5g_connected=0;

int DY_SPEED =4; //设置每一个明亮级的停留时间
int DY_PWM=10; //设置明亮的总级数（级数过大会出现闪烁）
int DY_MAX=10 ; //设置最大明亮（值域：小于明亮总级数）           设小最大明亮值，可自行选取合适参数，越小呼吸周期越短
pthread_t LED_Breathinglight_thread_id;

//用来设置快 慢闪， 0快闪 ，1慢闪  
void LED_twinkle_set(int flag)
{
	
	if(flag==0)//快闪
    	{
		DY_SPEED =3; //设置每一个明亮级的停留时间
		DY_PWM=12; //设置明亮的总级数（级数过大会出现闪烁）
		DY_MAX=12 ; //设置最大明亮（值域：小于明亮总级数）           设小最大明亮值，可自行选取合适参数，越小呼吸周期越短
		printf("WIFI_LED twinkle fast \n");
	}
	else if(flag==1)//慢闪
    	{
		DY_SPEED =3; //设置每一个明亮级的停留时间
		DY_PWM=20; //设置明亮的总级数（级数过大会出现闪烁）
		DY_MAX=20 ; //设置最大明亮（值域：小于明亮总级数）           设小最大明亮值，可自行选取合适参数，越小呼吸周期越短
		printf("WIFI_LED twinkle slow \n");
	}
	else
	return 0;
}
int probox_check_hostapd(int flag)
{	
	if(flag==1){
		if(access("/tmp/hostapd_02",0)==0)
			return 0;
		else
			return -1;
	}else
	{
		if(access("/tmp/hostapd",0)==0)
			return 0;
		else
			return -1;
	}
}
int Probox_get_wifi_sta(long gpio)
{	
	int flag_breath;
	if(gpio==GPIO_WIFI5G_ON_LED){

		//flag_breath=probox_check_hostapd(1);
		if(hostapd5g_connected==0 && wpa5g_connected==0)
			flag_breath=0;//keep on
		else if(hostapd5g_connected ||wpa5g_connected)
			flag_breath=1;//slow	
		else
			;
		//printf("wifi_sta0:[%d][%d]\n",hostapd5g_connected,wpa5g_connected);
	}else{
		//flag_breath=probox_check_hostapd(0);
		 if(hostapd24g_connected==0 && wpa24g_connected==0)
			flag_breath=0;//keep on
		else if(hostapd24g_connected ||wpa24g_connected)
			flag_breath=1;//slow
		else
			;
		//printf("wifi_sta1:[%d][%d]\n",hostapd24g_connected,wpa24g_connected);
	}
	//printf("flag_breath=%d",flag_breath);
	return flag_breath;
}
//闪烁主函数 
static void *LED_breathinglight_thread(long  gpio)
{
	unsigned int t=1;
	unsigned int i=0;
	char *gpio_val=NULL;
	int  LOOP=0; //LOOP是循环切换标志
	int wifista_flag=0;
	LED_twinkle_set(1);   
	int count=0;
	printf("LED_breathinglight_thread\n");
	while (1){  
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		if(!access("/tmp/uart.testbuf", F_OK))
		{
			FILE *fp = NULL;
			char buf[32]="";
			char tx_buf[64]="";
			fp = fopen("/tmp/uart.testbuf", "r");
	  		if(fp != NULL){
				fgets(buf,32,fp);
				if(strcmp(buf,"")){
					sprintf(tx_buf,"Dongle Replay:%s",buf);
					printf("tx_buf=%s",tx_buf);
					sendCmdUart(tx_buf,strlen(tx_buf));	
				}
			}
			
			unlink("/tmp/uart.testbuf");
		}
		if(wifista_flag==1 && count >100){
			if(get_ap_close_flag()==0)
			{
     				 if(get_last_ui()!=2)
	  				hostapd5g_connected = wifi_count_accessDevices_softap();
				hostapd24g_connected=wifi_count_accessDevices_softap_24g();

			}
			else
			{
				hostapd24g_connected=0;
				hostapd24g_connected=0;
			}
		}
		else if(wifista_flag==0){
			if(get_ap_close_flag()==0)
			{
				if(get_last_ui()!=2)
	  				hostapd5g_connected = wifi_count_accessDevices_softap();
				hostapd24g_connected=wifi_count_accessDevices_softap_24g();
			}
			else
			{
				hostapd24g_connected=0;
				hostapd24g_connected=0;
			}
		}
		else ;
		if(hostapd24g_connected<0)hostapd24g_connected=0;
		if(hostapd5g_connected<0)hostapd5g_connected=0;
		wifista_flag=Probox_get_wifi_sta(gpio);
		//wifista_flag=1;
		//printf("wifista_flag=%d",wifista_flag);
#else
		wifista_flag=1;
#endif
		if(wifista_flag==0){
			//get_gpio(gpio,gpio_val); 
			//printf("keep led on gpio val=%s",gpio_val);
			set_gpio(gpio,0);
			sleep(10);
		}
		else if(wifista_flag==-1){
			//get_gpio(gpio,gpio_val);
			//printf(" keep led offgpio val=%s",gpio_val);
			set_gpio(gpio,0);
			sleep(10);
		}else{
			if (count<101)
				count++;
			else
				count=0;	
			if(LOOP == 0){ //变亮循环
			        for(i = 0; i < DY_SPEED; i++){
					display(t,gpio);
					//   printf("turn light \n");
				}
				t++;
				if(t>(DY_MAX-2)){
					LOOP = 1;
					led_light(gpio);
			           
				}
			}      
		  
			if(LOOP == 1){ //变暗循环
				for(i = 0; i < DY_SPEED; i++){
			        	display(t,gpio);
			       	// printf("turn dark\n");
			    	}
			    	t--;
			    	if(t<2){
					LOOP = 0;
					led_dark(gpio);
				}            
			}    
			}
	}       
	pthread_exit(NULL);
	return NULL;	
}
//delay 1ms
void mdelay(unsigned long t)
{
	t=t*1000;
	usleep(t);	
}
//模拟PWM宽度
void display (unsigned int a,long gpio)
{   	
	set_gpio(gpio,0); //LED小灯亮
	mdelay (a);  	
	set_gpio(gpio,1);//LED小灯灭
	mdelay (DY_PWM-a);//延时总PWM长度减a  	
}

//亮500ms来区分
void led_light(long gpio)
{
	set_gpio(gpio,0); //LED小灯亮
	mdelay (50);      //延时a的长度
}

//暗1s来区分：
void led_dark(long gpio)
{	
	set_gpio(gpio,1);//LED小灯灭
	mdelay (500);//延时总PWM长度减a
}
//0快闪，1慢闪 请在LED_twinkle_thread_loop（）之前调用


int sendCmdUart(char *data,int size){
	int fd=-1;
	fd = open("/dev/uartcom",O_RDWR);
	if(fd!=-1){
		int ret = write(fd,data,size);
		close(fd);
		return ret;
	}else{
		printf("uartcom open error (%s)\n",strerror(errno));
	}
	return -1;
}


int Probox_led_init()
{
	int rtn,i;
	pthread_t thread_id;

	reg_writel(GPIO_BASE+0x0050, (reg_readl(GPIO_BASE+0x0050)&~(0xf<<11)));
	//set gpio 69 70 71 as output Enable
	printf("mpf4 0x0050 register DATA=%x\n",reg_readl(GPIO_BASE+0x0050));
	reg_writel(GPIO_BASE+0x0018, (reg_readl(GPIO_BASE+0x0018)&~(0xE<<4))|(0xE<<4));
	reg_writel(GPIO_BASE+0x0020, (reg_readl(GPIO_BASE+0x0020)&~(0xE<<4))|(0xE<<4));
	//set_gpio(69,1);	//lan
	//set_gpio(70,0);//5g
	//set_gpio(71,1);//2.4G
	
	printf("	GPIO_95_64 DATA=%x\n",reg_readl(GPIO_BASE+0x0020));
	//set_gpio(GPIO_WIFI24G_ON_LED,0);
	//set_gpio(GPIO_WIFI5G_ON_LED,0);
	//set_gpio(GPIO_WIFI5G_ON_LED,0);

	for(i=0;i<3;i++){
		rtn=pthread_create(&thread_id, NULL, LED_breathinglight_thread, GPIO_WIFI24G_ON_LED);
		if(rtn==0)//success
			break;
		if(i>=3)
			printf("create Probox wifi led thread fail,wifi led not work!!!\n");
	}
	for(i=0;i<3;i++){
		rtn=pthread_create(&thread_id, NULL, LED_breathinglight_thread, GPIO_WIFI5G_ON_LED);
		if(rtn==0)//success
			break;
		if(i>=3)
			printf("create Probox wifi led thread fail,wifi led not work!!!\n");
	}
	return 0;
	//printf("	GPIO_95_64 DATA=%x\n",reg_readl(GPIO_BASE+0x0020));
}






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
static void WIFI_CLIENT_OK_LED_twinkle()
{
	SysCGI_priv.WIFI_CLIENT_OK_LED_twinkle_tick^=1;
	//printf("EZmusic_LED_priv.WIFI_CLIENT_OK_LED_twinkle_tick============%d,%s,%d\n",EZmusic_LED_priv.WIFI_CLIENT_OK_LED_twinkle_tick,__FUNCTION__,__LINE__);
	if(SysCGI_priv.WIFI_CLIENT_OK_LED_twinkle_tick)
		set_gpio(GPIO_WIFI_ON_LED,0);//WIFI-ON-LED
	else
		set_gpio(GPIO_WIFI_ON_LED,1);//WIFI-OFF-LED
}

#endif

static int sysinfo_get_ProboxConnectStatus(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	ret = ProboxGetConnectStatus();
	printf("ProboxGetConnectStatus=%d\n",ret);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}


int ProboxGetConnectStatus(void)
{
	#define LENGTH 1020
	FILE *fp=NULL;
	char buff[LENGTH]={0};
	int ret=-1;
	int mode=-1;
	char *p;
	
	fp=fopen("/proc/net/route","r");
	if(fp==NULL)
	{
		printf("open router file error\n");
		return -1;
	}
	
	fread(buff,LENGTH,1,fp);
	
	if(buff==NULL)
	{
		printf("read router file error\n");
		return -1;
	}
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	if(strstr(buff,PROBOX_5G_CONFIG)!=NULL)
	{
		mode=1;
		printf("probox connect 5G\n");
	}
	else if(strstr(buff,PROBOX_24G_CONFIG)!=NULL)
	{
		mode=2;
		printf("probox connect 24G\n");
	}
#else
	if(strstr(buff,PROBOX_5G_CONFIG)!=NULL)//For other platforms,only one wifi.
	{
		mode=2;
		printf("connect 24G\n");
	}
#endif
	else
	{
		mode =0;
		printf("no connect\n");
	}
		
	fclose(fp);
	return mode;
}

int dongle_out_times=0;
static int sysinfo_get_wifi_mode(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	int ret3=-1;
	int dongle_type_divided=dongle_get_device_changing();
	if(dongle_type_divided==RALINK_dongle){
		if(wifi_mode!=WIFI_DISABLE_MANUAL&&wifi_mode!=WIFI_CLIENT_GETAUTOIP_OK&&wifi_mode!=WIFI_CLIENT_GETAUTOIP_ERR&&wifi_mode!=WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK &&wifi_mode!=WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR){
			int ret1=access("/sys/module/rt5370sta",F_OK);
			int ret2=access("/sys/module/rtnet3070ap",F_OK);	
		//	int ret3=(id_hcd_ko==HCD_USE_WIFI)?access("/sys/module/am7x_hcd",F_OK):access("/sys/module/am7x_hcd_next",F_OK);
			if(id_hcd_ko == HCD_USE_WIFI) 
				ret3 = access("/sys/module/am7x_hcd",F_OK);
			if(id_hcd_ko == HCD_NEXT_USE_WIFI)
				ret3 = access("/sys/module/am7x_hcd_next",F_OK);
			//printf("ret3====%d +++ ret1====%d +++ ret2====%d\n",ret3,ret1,ret2);
			if(ret3==0){
				dongle_out_times=0;
				if(wifi_mode!=WIFI_SOFTAP_ENABLE && wifi_mode!=WIFI_CLIENT_MODE_ENABLE)
					wifi_mode=WIFI_DONGLE_PLUG_IN;
				if(ret1==0)
					wifi_mode=WIFI_CLIENT_MODE_ENABLE;
				if(ret2==0)
					wifi_mode=WIFI_SOFTAP_ENABLE;
			}
			if((ret3!=0) && (wifi_mode != WIFI_DONGLE_PLUG_OUT)){
				//wifi_mode=WIFI_DONGLE_PLUG_OUT;
				dongle_out_times++;
				if(dongle_out_times<3)
					wifi_mode=WIFI_MODE_CHANGEING_FOR_RALINK;
				else{
					
					wifi_mode=WIFI_DONGLE_PLUG_OUT;
					dongle_out_times=0;

				}
			
			}
			
		
		}	
	
	}
	//printf("debug %s %d wifi mode :%d\n",__func__,__LINE__,wifi_mode);
	Swfext_PutNumber(wifi_mode);
	#if (EZMUSIC_ENABLE&&GPIO_WIFI_ON_LED)
	int wifi_status=ezcast_wifi_getStatus();
	//printf("wifi_status============%d,%s,%d\n",wifi_status,__FUNCTION__,__LINE__);		
	if((WIFI_INACTIVE==wifi_status)||(WIFI_SCANNING==wifi_status)||(WIFI_DISCONNECTED_MANUAL==wifi_status)||(WIFI_DISCONNECTED==wifi_status))
	{
		//printf("SysCGI_priv.WIFI_CLIENT_OK_LED_flag============%d,%s,%d\n",SysCGI_priv.WIFI_CLIENT_OK_LED_flag,__FUNCTION__,__LINE__);	
		if(SysCGI_priv.WIFI_CLIENT_OK_LED_flag)
		{
			SysCGI_priv.WIFI_CLIENT_OK_LED_flag=0;
			//stop_timer(SysCGI_priv.WIFI_CLIENT_OK_LED_twinkle_timer);
			/** send request to cancel thread */
			pthread_cancel(LED_Breathinglight_thread_id);	
			/** wait thread until exit */
			pthread_join(LED_Breathinglight_thread_id, NULL);
			SysCGI_priv.WIFI_ON_LED_flag=0;	
			SysCGI_priv.CONNECTIONS_OK_LED_twinkle_flag=0;
		}
		//printf("wifi_mode============%d,%s,%d\n",wifi_mode,__FUNCTION__,__LINE__);
		if(WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK==wifi_mode)
		{
			//printf("EZmusic_LED_priv.WIFI_ON_LED_flag============%d,%s,%d\n",EZmusic_LED_priv.WIFI_ON_LED_flag,__FUNCTION__,__LINE__);		
			if(!SysCGI_priv.WIFI_ON_LED_flag)
			{
				SysCGI_priv.WIFI_ON_LED_flag=1;
				set_gpio(GPIO_WIFI_ON_LED,0);//WIFI-ON-LED
			}
		}		
	}
	//printf("wifi_mode============%d,%s,%d\n",wifi_mode,__FUNCTION__,__LINE__);
	if(WIFI_CONCURRENT_SOFTAP_ENABLE==wifi_mode)
	{
		//printf("EZmusic_LED_priv.WIFI_ON_LED_flag============%d,%s,%d\n",EZmusic_LED_priv.WIFI_ON_LED_flag,__FUNCTION__,__LINE__);
		if(0==SysCGI_priv.start_audio_once)
		{
			SysCGI_priv.start_audio_once=1;
			SysCGI_priv.start_audio_flag=1;
			SysCGI_priv.connect_router_waiting_flag=1;
		}		
		if(SysCGI_priv.WIFI_CLIENT_OK_LED_flag)
		{
			SysCGI_priv.WIFI_CLIENT_OK_LED_flag=0;
			//stop_timer(SysCGI_priv.WIFI_CLIENT_OK_LED_twinkle_timer);
			/** send request to cancel thread */
			pthread_cancel(LED_Breathinglight_thread_id);	
			/** wait thread until exit */
			pthread_join(LED_Breathinglight_thread_id, NULL);

		}
		if(!SysCGI_priv.WIFI_ON_LED_flag)
		{
			SysCGI_priv.WIFI_ON_LED_flag=1;
			set_gpio(GPIO_WIFI_ON_LED,0);//WIFI-ON-LED
		}
	}
	else if((WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK==wifi_mode)||(WIFI_CONCURRENT_CLIENT_GETAUTOIP_ERR==wifi_mode))
	{
		//printf("EZmusic_LED_priv.WIFI_CLIENT_OK_LED_flag============%d,%s,%d\n",EZmusic_LED_priv.WIFI_CLIENT_OK_LED_flag,__FUNCTION__,__LINE__);
		if((WIFI_INACTIVE!=wifi_status)&&(WIFI_SCANNING!=wifi_status)&&(WIFI_DISCONNECTED_MANUAL!=wifi_status)&&(WIFI_DISCONNECTED!=wifi_status))
		{
			if(!SysCGI_priv.WIFI_CLIENT_OK_LED_flag)
			{
				SysCGI_priv.WIFI_CLIENT_OK_LED_flag=1;
				SysCGI_priv.WIFI_ON_LED_flag=1;
				//printf("connect_router_success======%s,%d\n",__FUNCTION__,__LINE__);
				if(WIFI_CONCURRENT_CLIENT_GETAUTOIP_OK==wifi_mode){
					SysCGI_priv.connect_router_success_flag=1;
					SysCGI_priv.connected_router_useful_flag=1;
				}else{
					printf("connect_router_failed,%s,%d\n",__FUNCTION__,__LINE__);
					SysCGI_priv.connect_router_failed_flag=1;
					SysCGI_priv.not_connected_router_useful_flag=1;
				}
				//start_timer(SysCGI_priv.WIFI_CLIENT_OK_LED_twinkle_timer,300000,0);
				//signal(SIGALRM, WIFI_CLIENT_OK_LED_twinkle);	
				if (pthread_create(&LED_Breathinglight_thread_id, NULL, LED_breathinglight_thread, GPIO_WIFI_ON_LED) != 0)
				{
					printf("[%s]: LED_Breathinglight_thread_loop error\n",__FUNCTION__);
				}
			}			
		}
	}		
	#endif	
	SWFEXT_FUNC_END();
}
#if MODULE_CONFIG_LAN
extern char lanEnableFlag;
static int sysinfo_get_lan_status(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(lanEnableFlag);
	SWFEXT_FUNC_END();
}

int netlink_status_get=-1;;
int the_first_check=1;
static int sysinfo_get_netlink_status(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	netlink_status_get=get_netlink_status();
	//printf("netlink_status_get===============%d\n",netlink_status_get);
	Swfext_PutNumber(netlink_status_get);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int websetting_sysinfo_get_netlink_status()
{	

	int  netlink_status;
	netlink_status=get_netlink_status();
	//printf("netlink_status_get===============%d\n",netlink_status);
	return netlink_status;
}


#endif

static int sysinfo_check_netlink_for_into_SWF(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	int linkstatus_for_intoSWF=-1;
	linkstatus_for_intoSWF=get_netlink_status();
	printf("linkstatus_for_intoSWF=========%d\n",linkstatus_for_intoSWF);
	Swfext_PutNumber(linkstatus_for_intoSWF);
	SWFEXT_FUNC_END();
}
#else
static int sysinfo_get_lan_status(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_netlink_status(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}
static int sysinfo_check_netlink_for_into_SWF(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);

	SWFEXT_FUNC_END();
}
#endif

int getLanConnectStatus(){
#if MODULE_CONFIG_LAN
	return lanEnableFlag;
#else
	return 0;
#endif
}
static int sysinfo_get_password_from_DPF(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	char *password_tmp=Swfext_GetString();
	
	//printf("password_tmp=====%s\n",password_tmp);
	ezConfigSetPasscode(password_tmp);
	
	SWFEXT_FUNC_END();
}
//start cdrom function
static int sysinfo_start_cdrom_func(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	//char *password_tmp=Swfext_GetString();
	
	//printf("password_tmp=====%s\n",password_tmp);
	find_iso_dmg_file();
	
	SWFEXT_FUNC_END();
}
static int sysinfo_check_binfiles_exsit(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	int storage_media_choose=Swfext_GetNumber();
	int check_result=-1;
	int no_exsit_times=0;
	if(storage_media_choose==1){
		/*int	check_result_tmp=access("/mnt/usb1/ACTUPGRADE.BIN",F_OK)+access("/mnt/usb2/ACTUPGRADE.BIN",F_OK)+access("/mnt/usb3/ACTUPGRADE.BIN",F_OK)+access("/mnt/usb4/ACTUPGRADE.BIN",F_OK);
		if(check_result_tmp==-4){
			check_result=-1;
		}
		else
			check_result=0;
		*/
		check_result = access("/mnt/user1/thttpd/html/airusb/usb/ACTUPGRADE.BIN",F_OK);
		//printf("%s check_result=%d\n", __func__, check_result);
	}
	
	if(storage_media_choose==2){
		//check_result=access("/mnt/card/ACTUPGRADE.BIN",F_OK);
		check_result=access("/mnt/user1/thttpd/html/airusb/sdcard/ACTUPGRADE.BIN",F_OK);
	}
	
__check_binfiles_exsit_end___:
	Swfext_PutNumber(check_result);
	SWFEXT_FUNC_END();
}
/**add by chenshouhui for display hostname and setting hostname**/
static int sysinfo_get_hostname(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	FILE *fp=NULL;
	char callbuf[50];
	char hostname_from_mnt[128]={0};
	fp=fopen("/mnt/user1/system_setting/hostname.dat","r");
	if(fp==NULL){
		memcpy(hostname_from_mnt,"MY_DPF",strlen("MY_DPF"));
		goto ___get_hostname_end___;
	}
	
	fread(hostname_from_mnt, 1, 128, fp);
	fclose(fp);
___get_hostname_end___:	
	printf("hostname_from_mnt====================%s",hostname_from_mnt);
	sprintf(callbuf,"hostname '%s'",hostname_from_mnt);
	system(callbuf);
	Swfext_PutString(hostname_from_mnt);
	SWFEXT_FUNC_END();
}
static int sysinfo_set_hostname(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	FILE *fp=NULL;
	char *hostname_from_SWF=Swfext_GetString();
	printf("hostname_from_SWF==========================%s\n",hostname_from_SWF);
	char callbuf[50]={0};
	char buf[128]={0};
	int ret=0;
	if(access("/mnt/user1/system_setting/hostname.dat",F_OK)==-1){
		sprintf(callbuf,"mkdir /mnt/user1/system_setting/");
		system(callbuf);
		memset(callbuf,0,50);
		
		sprintf(callbuf,"cp /mnt/bak/hostname.dat /mnt/user1/system_setting/hostname.dat");
		system(callbuf);
		memset(callbuf,0,50);
	}
	
	memset(buf,0,128);
	
	memcpy(buf,hostname_from_SWF,strlen(hostname_from_SWF));
	fp=fopen("/mnt/user1/system_setting/hostname.dat","wb+");
	if(fp==NULL)
		goto ___set_hostname_end___;
	fwrite(buf,1,strlen(buf),fp);
	fprintf(stderr,"function:%s line:%d buf:%s\n",__FUNCTION__,__LINE__,buf);
	ret=fflush(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

	int fd_write = fileno(fp);
	ret=fsync(fd_write);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);

	ret=fclose(fp);
	
	sprintf(callbuf,"hostname '%s'",hostname_from_SWF);
	system(callbuf);
___set_hostname_end___:	
	SWFEXT_FUNC_END();
}
/**add by chenshouhui for usb switch function**/
enum{
	DEVICE_SWTO_HOST=0,//swith 8250 usb port0 to host mode(typeA)
	HOST_SWTO_DEVICE    //switch 8250 usb port to device mode(minB) 
}USB_SWITCH;
static int sysinfo_setgpio_for_usbswitch(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
#if MODULE_CONFIG_USB_SWITCH
	int direction=Swfext_GetNumber();
	printf("direction=====================%d\n",direction);
	if(direction==DEVICE_SWTO_HOST){
		printf("++++++++++++++++++++++++%s %d\n",__func__,__LINE__);
	/*	set_gpio(83,1);
		usleep(50);
		set_gpio(82,1);
	*/	
		set_gpio(82,1);
		usleep(50);
		set_gpio(83,1);
		
	}
	else{
		set_gpio(83,0);
		//OSSleep(5);
		usleep(50);
		set_gpio(82,0);
	}
#endif
	SWFEXT_FUNC_END();
}
#ifdef CASE_USB_COMPSITE_DEVICE
static int sysinfo_usb_compsite_device_total(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int device_total=-1;
	sysdbg_info("sysinfo_usb_compsite_device_total");
	_get_usb_compsite_device_info();
	device_total = _usb_compsite_device_total();
	sysdbg_info("device_total======%d\n",device_total);
	Swfext_PutNumber(device_total);
	SWFEXT_FUNC_END();
}
static int sysinfo_usb_compsite_device_intface_total(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int device_index = -1,interface_total = -1;
	device_index = Swfext_GetNumber();
	sysdbg_info("device_index======%d\n",device_index);
	interface_total = _usb_compsite_device_interface_total(device_index);
	
	sysdbg_info("interface_total======%d\n",interface_total);
	Swfext_PutNumber(interface_total);
	SWFEXT_FUNC_END();
}
static int sysinfo_usb_compsite_device_name_information(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int device_index =Swfext_GetNumber();
	char cpt_dev_buf[64];
	sysdbg_info("device_index======%d\n",device_index);
	_get_usb_compsite_device_name_info(device_index,cpt_dev_buf);
	sysdbg_info("cpt_dev_buf======%s\n",cpt_dev_buf);
	
	Swfext_PutString(cpt_dev_buf);
	SWFEXT_FUNC_END();
}
static int sysinfo_usb_compsite_device_class_information(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char cpt_intf_buf[64];
	int dev_index = Swfext_GetNumber();
	int interface_idex=Swfext_GetNumber();
	
	sysdbg_info("dev_index======%d\n",dev_index);
	sysdbg_info("interface_idex======%d\n",interface_idex);
	
	_get_usb_compsite_device_interfaceclass_info(dev_index,interface_idex ,cpt_intf_buf);
	
	sysdbg_info("cpt_intf_buf======%s\n",cpt_intf_buf);
	Swfext_PutString(cpt_intf_buf);
	
	SWFEXT_FUNC_END();
}
static int sysinfo_choose_usb_compsite_function(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int dev_index = Swfext_GetNumber();
	
	int intf_idex = Swfext_GetNumber();
	int class_type = Swfext_GetNumber();
	printf("dev_index======%d\n",dev_index);	
	printf("interface_idex======%d\n",intf_idex);
	//class_type = 8;
	printf("interface class type=%d\n",class_type);
	_choose_usb_compsite_device_function(dev_index,intf_idex,class_type);
	SWFEXT_FUNC_END();
}


#endif //CASE_USB_COMPSITE_DEVICE
static int sysinfo_get_connectpc_flag(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int connectpc_flag=CONNECT_GFILESTORAGE;
	if(access("/sys/module/g_file_storage",F_OK)==0)
		connectpc_flag=CONNECT_GFILESTORAGE;
	else if(access("/sys/module/g_subdisplay",F_OK)==0){
		connectpc_flag=CONNECT_EZUSB;
	}
	Swfext_PutNumber(connectpc_flag);
	SWFEXT_FUNC_END();
}
/***************************add for edid config******************************/
#if 1
#define ediddbg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define ediddbg_info(fmt, arg...)
#endif

int del_edid_binfile(){
	if(access(EDID_CONFIG_PATH_IN_CASE,F_OK)==0){
		char callbuf[128]={0};
		remove(EDID_CONFIG_PATH_IN_CASE);
		return 0;
	}
	return -1;
}

int search_in_edid_bin_file(char *keyword){
	FILE *fp;
	char tmep_buf[128]={0};
	char result_str[8]={0};
	int result=0;

	fp=fopen(EDID_CONFIG_PATH_IN_CASE,"r");
	if(fp==NULL){
		ediddbg_info("errno=====%d\n",errno);
		fclose(fp);	
		return -1;
	}
	fread(tmep_buf,1,128,fp);
	
	ediddbg_info("temp_buf:\n%s\n",tmep_buf);
	ediddbg_info("keyword=======%s\n",keyword);
	char *locate1=strstr(tmep_buf,keyword);
	if(locate1==NULL){
		ediddbg_info("errno=====%d\n",errno);
		return -1;
	}
	char *locate2=strstr(locate1,"\n");
	memcpy(result_str,locate1+strlen(keyword),strlen(locate1+strlen(keyword))-strlen(locate2));
	
	ediddbg_info("result===%s\n",result_str);
	result=atoi(result_str);
	ediddbg_info("result===%d\n",result);
	
	fclose(fp); 
	return result;
}

int find_result_form_edid_binfile(int index_choose){
	if(access(EDID_CONFIG_PATH_IN_CASE,F_OK)!=0){
	}
	else{
		switch(index_choose){
		case 0:
			if(search_in_edid_bin_file("hdmi valid :")==1)
				return 1;
			else if(search_in_edid_bin_file("vga valid :")==1)
				return 8;
			else 
				return -1;
			break;
		case 1:
			return search_in_edid_bin_file("hdmi format :");
			break;
		case 2:
			return search_in_edid_bin_file("vga format :");

			break;

		}
	}
	return -1;
}
static int sysinfo_creat_edid_bin(void *handle)
{	
	char edid_buf[64]={0};
	char head_info[48]={0};
	FILE *fp;
	int fd_write;
	int len=0;
	int i=0;
	unsigned char ch;
	int err;
	edid_config_info edid_config_info_temp;
	edid_config_info edid_config_info_temp2;

	SWFEXT_FUNC_BEGIN(handle);

	edid_config_info_temp.hdmi_valid=Swfext_GetNumber();
	edid_config_info_temp.hdmi_format=Swfext_GetNumber();
	edid_config_info_temp.vga_valid=Swfext_GetNumber();
	edid_config_info_temp.vga_format=Swfext_GetNumber();
	ediddbg_info("edid_config_info_temp.vga_valid====%d\n",edid_config_info_temp.vga_valid);
	ediddbg_info("edid_config_info_temp.hdmi_valid====%d\n",edid_config_info_temp.hdmi_valid);
	ediddbg_info("edid_config_info_temp.vga_format====%d\n",edid_config_info_temp.vga_format);
	ediddbg_info("edid_config_info_temp.hdmi_format====%d\n",edid_config_info_temp.hdmi_format);
	
	ediddbg_info();
	del_edid_binfile();
	ediddbg_info();
	//memset(head_info,0x0,48);
	//sprintf(edid_buf,"%s%d%d%d%d",head_info,edid_config_info_temp.hdmi_valid,edid_config_info_temp.hdmi_format,edid_config_info_temp.vga_valid,edid_config_info_temp.vga_format);

/**
	fp=fopen(EDID_CONFIG_PATH_IN_CASE,"w+");
	if(fp==NULL){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;

	}
*/
	fd_write = open(EDID_CONFIG_PATH_IN_CASE, O_RDWR|O_CREAT, 0664);
	if(fd_write < 0){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;
	}

	ediddbg_info();

	if( write(fd_write, head_info, 48) != 48 ) {
		printf(" write edid error 1\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}

	if( write(fd_write, &edid_config_info_temp, sizeof(edid_config_info)) != sizeof(edid_config_info) ) {
		printf(" write edid error 2\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}
	
	//write(head_info,1,48,fp);
	//fwrite(&edid_config_info_temp,1,sizeof(edid_config_info),fp);
	
	ediddbg_info();
	//fflush(fp);
	
	ediddbg_info();
	// = fileno(fp);
	
	ediddbg_info();
	fsync(fd_write);
	ediddbg_info();
	//fclose(fp);
	close(fd_write);
	ediddbg_info();

#if 0
	fd_write = open(EDID_CONFIG_PATH_IN_CASE, O_RDONLY);
	if(fd_write < 0){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;
	}

/**
	if( (len = lseek(fd_write, 0, SEEK_END)) < 0 ) {
		printf("seek error\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}

	lseek(fd_write, 0, SEEK_SET);
*/
	len = 60;
	i = 0;
	
	if( (len = lseek(fd_write, 48, SEEK_SET)) < 0 ) {
		printf("seek error\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}
	printf("file len = %d\n",len);

	edid_config_info_temp2.vga_valid = 32;
	edid_config_info_temp2.hdmi_valid = 32;
	edid_config_info_temp2.vga_format = 32;
	edid_config_info_temp2.hdmi_format = 32;
	err = read(fd_write, &edid_config_info_temp2, sizeof(edid_config_info));
	ediddbg_info("edid_config_info_temp2.vga_valid====%d\n",edid_config_info_temp2.vga_valid);
	ediddbg_info("edid_config_info_temp2.hdmi_valid====%d\n",edid_config_info_temp2.hdmi_valid);
	ediddbg_info("edid_config_info_temp2.vga_format====%d\n",edid_config_info_temp2.vga_format);
	ediddbg_info("edid_config_info_temp2.hdmi_format====%d\n",edid_config_info_temp2.hdmi_format);
	close(fd_write);
	
	printf("---------- out,err=%d\n",err);
#endif

___sysinfo_creat_edid_bin_end__:

	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_sysinfo_creat_edid_bin(int hdmi_valid,int hdmi_format,int vga_valid,int vga_format)
{	
	char edid_buf[64]={0};
	char head_info[48]={0};
	FILE *fp;
	int fd_write;
	int len=0;
	int i=0;
	unsigned char ch;
	int err;
	edid_config_info edid_config_info_temp;
	edid_config_info edid_config_info_temp2;


	edid_config_info_temp.hdmi_valid=hdmi_valid;
	edid_config_info_temp.hdmi_format=hdmi_format;
	edid_config_info_temp.vga_valid=vga_valid;
	edid_config_info_temp.vga_format=vga_format;
	ediddbg_info("edid_config_info_temp.vga_valid====%d\n",edid_config_info_temp.vga_valid);
	ediddbg_info("edid_config_info_temp.hdmi_valid====%d\n",edid_config_info_temp.hdmi_valid);
	ediddbg_info("edid_config_info_temp.vga_format====%d\n",edid_config_info_temp.vga_format);
	ediddbg_info("edid_config_info_temp.hdmi_format====%d\n",edid_config_info_temp.hdmi_format);
	
	ediddbg_info();
	del_edid_binfile();
	ediddbg_info();
	//memset(head_info,0x0,48);
	//sprintf(edid_buf,"%s%d%d%d%d",head_info,edid_config_info_temp.hdmi_valid,edid_config_info_temp.hdmi_format,edid_config_info_temp.vga_valid,edid_config_info_temp.vga_format);

/**
	fp=fopen(EDID_CONFIG_PATH_IN_CASE,"w+");
	if(fp==NULL){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;

	}
*/
	fd_write = open(EDID_CONFIG_PATH_IN_CASE, O_RDWR|O_CREAT, 0664);
	if(fd_write < 0){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;
	}

	ediddbg_info();

	if( write(fd_write, head_info, 48) != 48 ) {
		printf(" write edid error 1\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}

	if( write(fd_write, &edid_config_info_temp, sizeof(edid_config_info)) != sizeof(edid_config_info) ) {
		printf(" write edid error 2\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}
	
	//write(head_info,1,48,fp);
	//fwrite(&edid_config_info_temp,1,sizeof(edid_config_info),fp);
	
	ediddbg_info();
	//fflush(fp);
	
	ediddbg_info();
	// = fileno(fp);
	
	ediddbg_info();
	fsync(fd_write);
	ediddbg_info();
	//fclose(fp);
	close(fd_write);
	ediddbg_info();

#if 0
	fd_write = open(EDID_CONFIG_PATH_IN_CASE, O_RDONLY);
	if(fd_write < 0){
		printf("open file error\n");
		goto ___sysinfo_creat_edid_bin_end__;
	}

/**
	if( (len = lseek(fd_write, 0, SEEK_END)) < 0 ) {
		printf("seek error\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}

	lseek(fd_write, 0, SEEK_SET);
*/
	len = 60;
	i = 0;
	
	if( (len = lseek(fd_write, 48, SEEK_SET)) < 0 ) {
		printf("seek error\n");
		close(fd_write);
		goto ___sysinfo_creat_edid_bin_end__;
	}
	printf("file len = %d\n",len);

	edid_config_info_temp2.vga_valid = 32;
	edid_config_info_temp2.hdmi_valid = 32;
	edid_config_info_temp2.vga_format = 32;
	edid_config_info_temp2.hdmi_format = 32;
	err = read(fd_write, &edid_config_info_temp2, sizeof(edid_config_info));
	ediddbg_info("edid_config_info_temp2.vga_valid====%d\n",edid_config_info_temp2.vga_valid);
	ediddbg_info("edid_config_info_temp2.hdmi_valid====%d\n",edid_config_info_temp2.hdmi_valid);
	ediddbg_info("edid_config_info_temp2.vga_format====%d\n",edid_config_info_temp2.vga_format);
	ediddbg_info("edid_config_info_temp2.hdmi_format====%d\n",edid_config_info_temp2.hdmi_format);
	close(fd_write);
	
	printf("---------- out,err=%d\n",err);
#endif

___sysinfo_creat_edid_bin_end__:
	sync();
	return 0;
}
#endif
static int sysinfo_delete_edid_bin(void *handle)
{
	int ret = -1;
	SWFEXT_FUNC_BEGIN(handle);
	ret = del_edid_binfile();
	OSSleep(2000);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
static int sysinfo_read_edid_bin(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	int read_index=Swfext_GetNumber();
	ediddbg_info("read_index==============%d\n",read_index);
	int read_reslut=0;
/*	read_reslut=find_result_form_edid_binfile(read_index);
*/
	DE_INFO	de_info;
	int fd = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if(ioctl(fd,GET_DE_CONFIG,&de_info)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	switch(read_index){
		case 0:
			switch(de_info.screen_type){
				case 0:
					read_reslut=LCD_ENABLE;
					break;
				case 1:
					read_reslut=HDMI_ENABLE;
					break;
				case 2:
					read_reslut=VGA_ENABLE;
					break;
				case 3:
					read_reslut=CVBS_ENABLE;
					break;
				case 4:
					read_reslut=YPBPR_ENABLE;
					break;
				default:
					
					read_reslut=LCD_ENABLE;
					break;
					
			}
					
			break;
		case 1:
			read_reslut=de_info.screen_output_format;
			break;
	}
	printf("de_info.screen_type= %d %d\n ",de_info.screen_type,de_info.screen_output_format);
	Swfext_PutNumber(read_reslut);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
 int ezcast_sysinfo_read_edid_bin(int read_index)
{

	ediddbg_info("read_index==============%d\n",read_index);
	int read_reslut=0;
/*	read_reslut=find_result_form_edid_binfile(read_index);
*/
	DE_INFO	de_info;
	int fd = open("/dev/lcm",O_RDWR);
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if(ioctl(fd,GET_DE_CONFIG,&de_info)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	switch(read_index){
		case 0:
			switch(de_info.screen_type){
				case 0:
					read_reslut=LCD_ENABLE;
					break;
				case 1:
					read_reslut=HDMI_ENABLE;
					break;
				case 2:
					read_reslut=VGA_ENABLE;
					break;
				case 3:
					read_reslut=CVBS_ENABLE;
					break;
				case 4:
					read_reslut=YPBPR_ENABLE;
					break;
				default:
					
					read_reslut=LCD_ENABLE;
					break;
					
			}
					
			break;
		case 1:
			read_reslut=de_info.screen_output_format;
			break;
	}
	printf("de_info.screen_type= %d %d\n ",de_info.screen_type,de_info.screen_output_format);
	return read_reslut;
}
#endif
/***************************add for edid config******************************/

/***************************add for usb double mass storage device******************************/
#if 1
#define double_storagedbg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define double_storagedbg_info(fmt, arg...)
#endif

static int sysinfo_get_storage_total(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int tmp1=access("/sys/block/sda",F_OK);
	int tmp2=access("/sys/block/sdb",F_OK);
	double_storagedbg_info("tmp1=====%d,tmp2=======%d\n",tmp1,tmp2);
	int total_storage_device=0;
	if(tmp1==0&&tmp2==0)
		total_storage_device=2;
	else{ 
		if(tmp1!=0&&tmp2!=0)
			total_storage_device=0;
		else
			total_storage_device=1;
	}
	double_storagedbg_info("total_storage_device=======%d\n",total_storage_device);
	Swfext_PutNumber(total_storage_device);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_multisector_info(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int i=0;
	int udisk_idex;
	int sec_total=0;
	int device_index=Swfext_GetNumber();
	int info_index=Swfext_GetNumber();	
	double_storagedbg_info("device_index=====%d,function_index=======%d\n",device_index,info_index);

	if(device_index==0){
		for(i=0;i<2;i++){
			if(strncmp(double_dev[i].hcd_name,"am7x_hcd-",strlen("am7x_hcd-"))==0){
				udisk_idex=i;
				break;
			}
		}
		sec_total = double_dev[udisk_idex].sector_num;
	}
	if(device_index==1){
		for(i=0;i<2;i++){
			if(strncmp(double_dev[i].hcd_name,"am7x_hcd_next",strlen("am7x_hcd_next"))==0){
				udisk_idex=i;
				break;
			}
		}
		sec_total=double_dev[udisk_idex].sector_num;
	}

	double_storagedbg_info("result=======%d\n",sec_total);
	Swfext_PutNumber(sec_total);
	SWFEXT_FUNC_END();
}

#if 0
static int sysinfo_get_usb_mass_deviceName(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int i,dev_idex;
	char mass_devname[10];
	int device_index=Swfext_GetNumber();
	int info_index=Swfext_GetNumber();
	memset(mass_devname,0x00,10);
	int j;
	for(j=0;j<2;j++){
		printf("debug %s %d j:%d devname:%s\n",__FUNCTION__,__LINE__,j,double_dev[j].devname);
		printf("debug %s %d j:%d hcdname:%s\n",__FUNCTION__,__LINE__,j,double_dev[j].hcd_name);
		printf("debug %s %d j:%d sectornum:%d\n",__FUNCTION__,__LINE__,j,double_dev[j].sector_num);
	}
	printf("device_index=====%d,info_index=======%d\n",device_index,info_index);
	for(i=0;i<2;i++){
		
		if(strncmp(double_dev[i].hcd_name,"am7x_hcd-",strlen("am7x_hcd-"))==0&&device_index==0){
			strncpy(mass_devname,double_dev[i].devname,strlen(double_dev[i].devname));
			printf("debug %s %d massdev:%s\n",__FUNCTION__,__LINE__,mass_devname);
			break;
		}else if(strncmp(double_dev[i].hcd_name,"am7x_hcd_next",strlen("am7x_hcd_next"))==0&&device_index==0){
			strncpy(mass_devname,double_dev[i].devname,strlen(double_dev[i].devname));
			printf("debug %s %d massdev:%s\n",__FUNCTION__,__LINE__,mass_devname);
			break;
		}
		if(strncmp(double_dev[i].hcd_name,"am7x_hcd_next",strlen("am7x_hcd_next"))==0&&device_index==1){
			//printf("debug %s %d hcdname:%s\n",__FUNCTION__,__LINE__,double_dev[i].hcd_name);
			strncpy(mass_devname,double_dev[i].devname,strlen(double_dev[i].devname));
			printf("debug %s %d massdev:%s\n",__FUNCTION__,__LINE__,mass_devname);
			break;
		}else if(strncmp(double_dev[i].hcd_name,"am7x_hcd-",strlen("am7x_hcd-"))==0&&device_index==1){
			strncpy(mass_devname,double_dev[i].devname,strlen(double_dev[i].devname));
			printf("debug %s %d massdev:%s\n",__FUNCTION__,__LINE__,mass_devname);
			break;
		}
	}
	if(strncmp(mass_devname,"/dev/sda",strlen("/dev/sda"))==0){
		dev_idex =1;
	}
	else if(strncmp(mass_devname,"/dev/sdb",strlen("/dev/sdb"))==0){
		dev_idex = 2;
	}
	printf("debug:%s %d dev_idex:%d\n",__FUNCTION__,__LINE__,dev_idex);
	Swfext_PutNumber(dev_idex);
	SWFEXT_FUNC_END();
}
#else
static int sysinfo_get_usb_mass_deviceName(void *handle){
	int i,dev_idex=0;
	char mass_devname[10];
	SWFEXT_FUNC_BEGIN(handle);
	int device_index=Swfext_GetNumber();
	//int info_index=Swfext_GetNumber();
	memset(mass_devname,0x00,10);

	if(device_index==0){
		dev_idex=0;
	}
	if(device_index==1){
		dev_idex=1;
	}
#if 0	
	int j;
	for(j=0;j<2;j++){
		printf("debug %s %d j:%d devname:%s\n",__FUNCTION__,__LINE__,j,double_dev[j].devname);
		printf("debug %s %d j:%d hcdname:%s\n",__FUNCTION__,__LINE__,j,double_dev[j].hcd_name);
		printf("debug %s %d j:%d sectornum:%d\n",__FUNCTION__,__LINE__,j,double_dev[j].sector_num);
	}

	printf("device_index=====%d,info_index=======%d\n",device_index,info_index);
	if(access("/dev/sda",F_OK)==0 && access("/dev/sdb",F_OK)==0){
		if(strncmp(double_dev[device_index].hcd_name,"am7x_hcd-",strlen("am7x_hcd-"))==0){
			for(i=0;i<2;i++){
				if(strncmp(double_dev[i].hcd_name,"am7x_hcd-",strlen("am7x_hcd-"))==0){
					printf("%s %d i:%d devname:%s\n",__FUNCTION__,__LINE__,i,double_dev[i].devname);
					if(strncmp(double_dev[i].devname,"/dev/sda",strlen("/dev/sda"))==0)
						dev_idex = 1;
					else
						dev_idex = 2;
				}
			}
		}else if(strncmp(double_dev[device_index].hcd_name,"am7x_hcd_next",strlen("am7x_hcd_next"))==0){
			for(i=0;i<2;i++){
				if(strncmp(double_dev[i].hcd_name,"am7x_hcd_next",strlen("am7x_hcd_next"))==0){
					printf("%s %d i:%d devname:%s\n",__FUNCTION__,__LINE__,i,double_dev[i].devname);
					if(strncmp(double_dev[i].devname,"/dev/sda",strlen("/dev/sda"))==0)
						dev_idex = 1;
					else
						dev_idex = 2;
				}
			}
		}
	}else if(access("/dev/sda",F_OK) !=0 && access("/dev/sdb",F_OK)==0){
		dev_idex = 2;
	}else if(access("/dev/sda",F_OK)==0 && access("/dev/sdb",F_OK) !=0){
		dev_idex = 1;
	}else
		dev_idex=0;
#endif
	printf("debug:%s %d dev_idex:%d\n",__FUNCTION__,__LINE__,dev_idex);
	Swfext_PutNumber(dev_idex);
	SWFEXT_FUNC_END();
}
#endif
/***************************add for usb double mass storage device******************************/

static int sysinfo_ota_upgrade(void *handle){

         int mode=0;
		 int ret_sysinfo=-1;
         char *url=NULL;

         SWFEXT_FUNC_BEGIN(handle);

         mode=Swfext_GetNumber();

         url=Swfext_GetString();

		//printf("debug:%s %d in ota upgrade,url is %s\n",__FUNCTION__,__LINE__,url);
         ret_sysinfo=_ota_entry_main(mode,url);
		printf("system info.c ret==============%d\n",ret_sysinfo);
		 Swfext_PutNumber(ret_sysinfo);

         SWFEXT_FUNC_END();

}
#if EZCAST_ENABLE
static int sysinfo_ota_upgrade_from_app(void *handle)
{
	char *url=NULL;
	int ret = -1;

	SWFEXT_FUNC_BEGIN(handle);

	url = getOtaUrl();
	if(url != NULL)
		ret = manual_ota(url);
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();

}
#endif

#if WEBSETTING_ENABLE
int ezcast_sysinfo_ota_upgrade(int mode,char *url)
{

	 int ret_sysinfo=-1;
	printf("debug:%s %d in ota upgrade,url is %s\n",__FUNCTION__,__LINE__,url);
	 ret_sysinfo=_ota_entry_main(mode,url);
	printf("system info.c ret==============%d\n",ret_sysinfo);
	 return ret_sysinfo;
}
#endif
static int sysinfo_get_mtp_device_in_flag(void *handle){
	int mtp_device_in_flag=0;
	SWFEXT_FUNC_BEGIN(handle);
	mtp_device_in_flag=feedback_mtp_device_in_flag();
	printf("mtp_device_in_flag===================%d\n",mtp_device_in_flag);
	Swfext_PutNumber(mtp_device_in_flag);
	SWFEXT_FUNC_END();
}

void system_reboot(){
	printf("[%s][%d] -v2- reboot system!!!\n", __func__, __LINE__);
	system("reboot");
}

static int sysinfo_reboot_system(void *handle){
	int mtp_device_in_flag=0;
	SWFEXT_FUNC_BEGIN(handle);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	puts("turn off USB 1");
	int fd = open("/dev/uartcom",O_RDWR);
	if(fd<0){
		printf("cant open uartcom device\n");
	}else{
		ioctl(fd,21,NULL);
		close(fd);
	}
	sleep(3);
#endif	
	system_reboot();
	SWFEXT_FUNC_END();
}
static int sysinfo_SetWifiChannelRegion(void *handle){
	//int mtp_device_in_flag=0;
	char *region;
	SWFEXT_FUNC_BEGIN(handle);
	region=Swfext_GetString();
	printf("[%s %d] wifi channel region:%s\n",__func__,__LINE__,region);
	SWFEXT_FUNC_END();
}
static int sysinfo_put_swf_activeflag(void *handle){
	char *region;
	
	int SwfMemUsed = 0;
		
	SWFEXT_FUNC_BEGIN(handle);
	currentActiveSwfFlag = Swfext_GetNumber();
	
	SwfMemUsed = SWF_MemCheck(1);
	sysdbg_info("currentActiveSwfFlag ====%d\n",currentActiveSwfFlag);

	if(currentActiveSwfFlag == MAINMENU_SWF_ACTIVE){
		
		sysdbg_info("enter into mainmenu.swf");
		
	}else if(currentActiveSwfFlag == CONNECTPC_SWF_ACTIVE){
	
		sysdbg_info("enter into connectPC.swf");
		
	}else if(currentActiveSwfFlag == PHOTO_SWF_ACTIVE){
	
		sysdbg_info("enter into photo.swf");
		
	}else if(currentActiveSwfFlag == VIDEO_SWF_ACTIVE){
		sysdbg_info("enter into video.swf");
		
	}else if(currentActiveSwfFlag == AUDIO_SWF_ACTIVE){
		sysdbg_info("enter into audio.swf");
		
	}else if(currentActiveSwfFlag == OFFICE_SWF_ACTIVE){
		
		sysdbg_info("enter into office.swf");
		
	}else if(currentActiveSwfFlag == USBDISPLAY_SWF_ACTIVE){
		
		sysdbg_info("enter into usbDisplay.swf");
		
	}else if(currentActiveSwfFlag == WIFIDISPLAY_SWF_ACTIVE){
		
		sysdbg_info("enter into wifiDisplay.swf");
		
	}else if(currentActiveSwfFlag == LANDISPLAY_SWF_ACTIVE){
		
		sysdbg_info("enter into lanDisplay.swf");
		
	}else if(currentActiveSwfFlag == DLNA_SWF_ACTIVE){
		
		sysdbg_info("enter into dlna.swf");
		
	}else if(currentActiveSwfFlag == MIRACAST_SWF_ACTIVE){
		
		sysdbg_info("enter into miracast.swf");
		
	}else if(currentActiveSwfFlag == SET_SYS_SWF_ACTIVE){
		
		sysdbg_info("enter into set_sys.swf");
		
	}else if(currentActiveSwfFlag == SET_VIDEO_SWF_ACTIVE){
		
		sysdbg_info("enter into set_video.swf");
		
	}else if(currentActiveSwfFlag == SET_PHOTO_SWF_ACTIVE){
		
		sysdbg_info("enter into set_photo.swf");
		
	}else if(currentActiveSwfFlag == SET_AUDIO_SWF_ACTIVE){
		
		sysdbg_info("enter into set_audio.swf");
		
	}else if(currentActiveSwfFlag == SET_WIFI_SWF_ACTIVE){
		
		sysdbg_info("enter into set_wifi.swf");
		
	}else if(currentActiveSwfFlag == SET_LAN_SWF_ACTIVE){
		
		sysdbg_info("enter into set_lan.swf");
		
	}else{
		
		sysdbg_info("enter into undefined-flag swf");
	}
	
	sysdbg_info("SwfMemUsed ====0x%x\n",SwfMemUsed);
	SWFEXT_FUNC_END();
}

 extern char* ota_get_server_version(void);

static int sysinfo_get_ota_server_version(void *handle)
{

	char serverVersion[VersionLenth];
	
	SWFEXT_FUNC_BEGIN(handle);
	bzero(serverVersion,VersionLenth);
	strncpy(serverVersion,ota_get_server_version(),VersionLenth);

	Swfext_PutString(serverVersion);
	SWFEXT_FUNC_END();

}
#if WEBSETTING_ENABLE
char * ezcast_sysinfo_get_ota_server_version(void)
{

	static char serverVersion[VersionLenth];
	
	bzero(serverVersion,VersionLenth);
	memset(serverVersion,0,sizeof(serverVersion));
	strncpy(serverVersion,ota_get_server_version(),VersionLenth);

	return serverVersion;
}  
#endif
static int sysinfo_get_ota_check_status(void *handle)
{
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	ret = ota_get_check_status();
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

// For EZCast
#if EZCAST_ENABLE
static int ezcastpro_read_mode(void *handle)
{
	int ret=0;
	SWFEXT_FUNC_BEGIN(handle);	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	ret=MODULE_CONFIG_EZCASTPRO_MODE;
	printf("ezcastpro_read_mode ret==%d\n",ret);
#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();	

}
static int sysinfo_isPayUpgrade(void *handle)
{
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
#ifdef AM8252TO8251KEY_ENABLE
	ret = ezCastIsPayUpgrade();
#else
	ret = 0;
#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_sysinfo_get_ota_check_status(void )
{
	int ret = 0;
	ret = ota_get_check_status();
	printf("ret=========%d\n",ret);
	return ret;
}

#endif
void disable_set_json_value(){
	disableSetJsonValue = 1;
}

static int sysinfo_json_set_value(void *handle){

	int op=0;
	int ret=-1;
	char *val=NULL;
	char ssid_val[64];

	SWFEXT_FUNC_BEGIN(handle);

	op=Swfext_GetNumber();
	val = Swfext_GetString();
	switch(op){
		case JSON_SET_EZMIRROR_START:
			ezCustomerStartMiracast();
			break;
		case JSON_SET_EZMIRROR_STOP:
			ezCustomerStopMiracast();
			ezcastJsonChanged();
			break;
		case JSON_SET_INTE_SSID:
			break;
		case JSON_SET_LANGUAGE:
			if(val != NULL)
				ezConfigSetLanguage(val);
			break;
		default:
			break;
			
	}
	Swfext_PutNumber(ret);

	SWFEXT_FUNC_END();

}
#if WEBSETTING_ENABLE
int ezcast_sysinfo_json_set_value(int op,char *val)
{
	int ret=-1;
	char ssid_val[64];
	switch(op){
		case JSON_SET_EZMIRROR_START:
			ezCustomerStartMiracast();
			break;
		case JSON_SET_EZMIRROR_STOP:
			ezCustomerStopMiracast();
			ezcastJsonChanged();
			break;
		case JSON_SET_INTE_SSID:
			break;
		case JSON_SET_LANGUAGE:
			if(val != NULL)
				ezConfigSetLanguage(val);
			break;
		default:
			break;
			
	}
	return ret;
}
#endif
static int sysinfo_get_display_status(void *handle){
	int status;
	
	SWFEXT_FUNC_BEGIN(handle);

	status = ezCastGetCurStatus();
	//printf("\n\tcurrent status is: %d\n", status);
	Swfext_PutNumber(status);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_ezcast_vendor(void *handle){
	char vendor[128];
	
	SWFEXT_FUNC_BEGIN(handle);

	ota_get_vendor(vendor);
	printf("\n\tEZCast vendor is: %d\n", vendor);
	Swfext_PutString(vendor);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_disauto_status(void * handle){
	int val;
    struct sysconf_param sys_cfg_data;
	SWFEXT_FUNC_BEGIN(handle);
    _get_env_data(&sys_cfg_data);

	val = sys_cfg_data.lang_auto_disable;
	printf("sysinfo_get_disauto_status: %d\n", val);
	
	Swfext_PutNumber(val);
	
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int ezcast_sysinfo_get_disauto_status(void ){
	int val;
    struct sysconf_param sys_cfg_data;
    _get_env_data(&sys_cfg_data);

	val = sys_cfg_data.lang_auto_disable;
	printf("sysinfo_get_disauto_status: %d\n", val);
	
	return val;
	
}
#endif
static int sysinfo_set_disauto_status(void * handle){
    int status=0;
    struct sysconf_param sys_cfg_data;
    SWFEXT_FUNC_BEGIN(handle);
    _get_env_data(&sys_cfg_data);
    status = Swfext_GetNumber();
	printf("sysinfo_set_disauto_status: %d\n", status);
    sys_cfg_data.lang_auto_disable = status;
    _save_env_data(&sys_cfg_data);
	ezCustomerSetAutoLanguage(!status);
    SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
void ezcast_sysinfo_set_disauto_status(int status){
    struct sysconf_param sys_cfg_data;
	_get_env_data(&sys_cfg_data);
	printf("sysinfo_set_disauto_status: %d\n", status);
    sys_cfg_data.lang_auto_disable = status;
    _save_env_data(&sys_cfg_data);
	ezCustomerSetAutoLanguage(!status);
}
#endif

static void langstringexchange(char *lang, int len){
	int i;
	char tmp[64];
	
	char* complex_array[] ={"TW", "Hant", "MO", "HK", "SG"};
	memcpy(tmp, lang, (sizeof(tmp)>len)?len:sizeof(tmp));
	memset(lang, 0, len);
	if(strncmp(tmp, "zh", 2) == 0){	// chinese
		for(i=0; i<sizeof(complex_array)/sizeof(complex_array[0]); i++)
		{
			if(strstr(tmp, complex_array[i]) != NULL) {
				sprintf(lang, "zh-TW");
				return;
			} 
		}
		sprintf(lang, "zh-CN");
		return;
	}else{
		memcpy(lang, tmp, 2);
		return;
	}
}

static int sysinfo_langautoget(void *handle){
	char lang[64];
	char buff[1024];
	char cmd[64];
	int fd = -1, ret = -1;
	
	SWFEXT_FUNC_BEGIN(handle);
	fd = open(APPINFO_FILE, O_RDONLY);
	if(fd < 0){
		perror("Open APPINFO_FILE");
		sprintf(lang, "NULL");
	}else{
		memset(lang, 0, sizeof(lang));
		memset(buff, 0, sizeof(buff));
		ret = read(fd, buff, sizeof(buff));
		isHdcpEnable(buff);
		if(ret > 0){
			JSON_getStringItemInString(buff, "language", lang, sizeof(lang));
			ezConfigSetLanguage(lang); // mingcheng added: inform "wifi_subdisplay" of language
			langstringexchange(lang, sizeof(lang));
		}
		fsync(fd);
		close(fd);
		//unlink(APPINFO_FILE);
	}
	
	Swfext_PutString(lang);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
char* ezcast_sysinfo_langautoget(void ){
	static char lang[64];
	char buff[1024];
	char cmd[64];
	int fd = -1, ret = -1;
	
	fd = open(APPINFO_FILE, O_RDONLY);
	if(fd < 0){
		perror("Open APPINFO_FILE");
		sprintf(lang, "NULL");
	}else{
		memset(lang, 0, sizeof(lang));
		memset(buff, 0, sizeof(buff));
		ret = read(fd, buff, sizeof(buff));
		isHdcpEnable(buff);
		if(ret > 0){
			JSON_getStringItemInString(buff, "language", lang, sizeof(lang));
			ezConfigSetLanguage(lang); // mingcheng added: inform "wifi_subdisplay" of language
			langstringexchange(lang, sizeof(lang));
		}
		fsync(fd);
		close(fd);
		//unlink(APPINFO_FILE);
	}
	
	return lang;

}
#endif

static int  sysinfo_delete_wifi_conf(void *handle)
{
      int ret = 0;
      SWFEXT_FUNC_BEGIN(handle);
      system("rm -fr /mnt/vram/wifi");
      Swfext_PutNumber(ret);
      SWFEXT_FUNC_END();
}

static int  sysinfo_delete_EZCast_conf(void *handle)
{
      int ret = 0;
      SWFEXT_FUNC_BEGIN(handle);
      system("rm -fr /mnt/vram/ezcast");
      system("rm  /mnt/vram/CONNECTION_MODE_CONFIGED");
      system("rm  /mnt/vram/ROUTER_CTL_ENABLE");

      system("rm  /mnt/vram/USER_HOSTNAME");  //device name
      system("rm  /mnt/user1/softap/rtl_hostapd_01.conf");  //password
      system("rm  /mnt/user1/BOOTMODECHANGED");  //default mode
      system("rm  /mnt/vram/CUSTOM_SSID");  //remove CUSTOM_SSID to revert default SSID
      system("rm  /mnt/vram/STD_SSID");     //remove STD_SSID to recreate it
      system("rm  /mnt/vram/CUSTOM_PSK");   //remove CUSTOM_PSK to revert default PSK
	//apps_vram_set_default();
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.last_ui = 0;
	#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR)
	sys_info.softap_info.softap_psk_setted_flag = 0;
	#endif
	_save_env_data(&sys_info);
	_store_env_data();
  	del_edid_binfile(); //resolution
	  
      Swfext_PutNumber(ret);
      SWFEXT_FUNC_END();
}

#if WEBSETTING_ENABLE
int  ezcast_deleteEZCastConf()
{
	#if defined(MODULE_CONFIG_FLASH_TYPE) && MODULE_CONFIG_FLASH_TYPE != 0
	  system("rm -fr /mnt/vram/*");
	#else
      system("rm -fr /mnt/vram/ezcast");
      system("rm  /mnt/vram/CONNECTION_MODE_CONFIGED");
      system("rm  /mnt/vram/ROUTER_CTL_ENABLE");

      system("rm  /mnt/vram/USER_HOSTNAME");  //device name
      system("rm  /mnt/user1/softap/rtl_hostapd_01.conf");  //password
      system("rm  /mnt/user1/BOOTMODECHANGED");  //default mode  last_ui
      system("rm  /mnt/vram/CUSTOM_SSID");  //remove CUSTOM_SSID to revert default SSID
      system("rm  /mnt/vram/STD_SSID");     //remove STD_SSID to recreate it
      system("rm  /mnt/vram/CUSTOM_PSK");   //remove CUSTOM_PSK to revert default PSK
    #endif
	//apps_vram_set_default();
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	sys_info.last_ui = 0;
	#if (FLASH_TYPE==FLASH_TYPE_16M_SNOR)
	sys_info.softap_info.softap_psk_setted_flag = 0;
	#endif
	_save_env_data(&sys_info);
	_store_env_data();
  	del_edid_binfile(); //resolution
}

int  ezcast_sysinfo_delete_wifi_conf()
{
      system("rm -fr /mnt/vram/wifi");
}
#endif
#if EZMUSIC_ENABLE
static int  sysinfo_deleteEZmusicConf(void *handle)
{
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	system("rm -fr /mnt/vram/EZmusic");
	change_default_ssid();
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
#endif

static int sysinfo_sys_volume_ctrl_get(void * handle)
{
	int vol_get=0;
	SWFEXT_FUNC_BEGIN(handle);	
	sys_volume_ctrl(_VOLUME_CTRL_GET,(void *)&vol_get);
	printf("----vol get is %d\n",vol_get);
	Swfext_PutNumber(vol_get);
	SWFEXT_FUNC_END();
}

static int sysinfo_ez_socket_reset(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);	
	printf("EZ_Wifidisplay socket reset\n");
	ezFuiWifiReset();
	SWFEXT_FUNC_END();
}
static int sysinfo_ezcast_enable_judge(void *handle){
	int chip_id=-1;
	int ezcast_mask = 1;

	SWFEXT_FUNC_BEGIN(handle);

	ezcast_mask = ezCastNetDisplayEnable();
		
	Swfext_PutNumber(ezcast_mask);

	SWFEXT_FUNC_END();

}
/*
int set_airview(char airview_sw){
	struct sysconf_param sys_info;
	//printf("\n\t --77777777--set_airview %c\n",airview_sw);
	
	_get_env_data(&sys_info);
	if(sys_info.air_view != airview_sw){
		sys_info.air_view = airview_sw;
		_save_env_data(&sys_info);
		_store_env_data();
	}

	//printf("\n\t --88888--set_airview %c\n",sys_info.air_view);

	return 0;
}

char get_airview()
{
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	//printf("\n\t --2222--get_airview \n");
	if(sys_info.air_view==NULL){
		sys_info.air_view=1;
		set_airview(1);
		}
	return (char)sys_info.air_view;
}
*/
int get_last_ui()
{
     
	struct sysconf_param sys_info;
	_get_env_data(&sys_info);
	//ezCustomerSetBootmode((sys_info.last_ui == 2)?1:0);
	return  (int)sys_info.last_ui;
}

int set_last_ui(int last_ui, int changed_by_user){
	struct sysconf_param sys_info;
	
	_get_env_data(&sys_info);
	if(sys_info.last_ui != last_ui){
		sys_info.last_ui = last_ui;
		_save_env_data(&sys_info);
		_store_env_data();
		ezcastSchemaBootMode(last_ui);
		if(changed_by_user != 0){
			if(access("/mnt/user1/BOOTMODECHANGED", F_OK) != 0){
				FILE *fp = fopen("/mnt/user1/BOOTMODECHANGED", "w");
				if(fp != NULL)
					fclose(fp);
			}
		}
#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)	
	if(last_ui == 1)
	{

		wifi_direct_func_stop();
		wifi_softap_close();
		wifi_softap_start();
		if(get_netlink_status())
			setAutoConnEnable(0);
		wifi_open_fun();
		wifi_start_fun();
	}
#elif EZCAST_LITE_ENABLE
		if(last_ui == EZMIRRORLITE_ITEM){
			restart_udhcpd_without_dns();
			ezCastSetRouterCtlEnable(1);
		}else{
			#if !AUTO_DNS_ENABLE
			restart_udhcpd_with_dns();
			#else
			if(wpa_connect_status() == WIFI_COMPLETED)
				restart_udhcpd_with_dns();
			#endif
			if(getConnectMode() == CTLMODE_DIRECTONLY)
				ezCastSetRouterCtlEnable(0);
		}
#endif
	}
	return 0;
}

static int sysinfo_get_last_ui(void *handle){
	int last_ui;
	
	SWFEXT_FUNC_BEGIN(handle);

	last_ui = get_last_ui();
	printf("\n\tLast UI is %d\n", last_ui);
	Swfext_PutNumber(last_ui);

	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
 int ezcast_sysinfo_get_last_ui(void)
 {
	int last_ui;
	last_ui = get_last_ui();
	printf("\n\tLast UI is %d\n", last_ui);
	return last_ui;
}

/*
 char ezcast_sysinfo_get_air_view(void)
 {
	char airview_sw;
	airview_sw = get_airview();
	//printf("\n\t --1111111--airview_sw is %c\n", airview_sw);
	return airview_sw;
}

 int ezcast_sysinfo_set_air_view(char airview)
 {
	//printf("\n\t --ezcast_sysinfo_set_air_view--airview_sw is %c\n",airview);
	set_airview(airview);
}
*/
#endif
static int sysinfo_set_last_ui(void *handle){
	int last_ui, changed_by_user = 0;
	
	SWFEXT_FUNC_BEGIN(handle);

	last_ui = Swfext_GetNumber();
	changed_by_user = Swfext_GetNumber();
	printf("\n\tset last UI %d\n", last_ui);
	set_last_ui(last_ui, changed_by_user);

	SWFEXT_FUNC_END();
}

 void ezcast_set_device_name(char *devName)
 {
	if(devName != NULL){
		printf(" ezcast_set_device_name   devName=%s\n",devName);
		ezcastWriteDevName(devName);
	}
}

static int sysinfo_set_device_name(void *handle){
	char *devName = NULL;
	
	SWFEXT_FUNC_BEGIN(handle);
	devName = Swfext_GetString();
	ezcast_set_device_name(devName);
	SWFEXT_FUNC_END();
}

static int sysinfo_set_psk_hide_status(void *handle){
	int status = 0;
	
	SWFEXT_FUNC_BEGIN(handle);

	status = Swfext_GetNumber();
	ezcastSchemaSetPskHide(status);

	SWFEXT_FUNC_END();
}
int ezcast_get_device_name(char *devName, int len)
{
	int ret = -1;

	memset(devName, 0, len);
	ret = ezcastReadDevName(devName, len);
	if(ret != 0){
		ret = gethostname(devName, len);
	}
	printf("devName=%s\n",devName);
	return ret;
}

static int sysinfo_get_device_name(void *handle){
	char  devName[128];
	int ret = -1;
	
	SWFEXT_FUNC_BEGIN(handle);

	ezcast_get_device_name(devName, sizeof(devName));
	
	Swfext_PutString(devName);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_system_hostname(void *handle){
	char sysHostname[128] = {0};
	int ret = -1;
	
	SWFEXT_FUNC_BEGIN(handle);

	ret = gethostname(sysHostname, sizeof(sysHostname));
	if(ret == 0){
		Swfext_PutString(sysHostname);
	}else{
		Swfext_PutString("nil");
	}

	SWFEXT_FUNC_END();
}

#if WEBSETTING_ENABLE
void ezcast_sysinfo_set_last_ui(int last_ui,int changed_by_user){
	printf("\n\tset last UI %d\n", last_ui);
	set_last_ui(last_ui, changed_by_user);
}
static int sysinfo_Get_language_index(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(language_index);

	SWFEXT_FUNC_END();
}
static int sysinfo_Set_language_index(void *handle){
	
	SWFEXT_FUNC_BEGIN(handle);
	
	language_index = Swfext_GetNumber();

	SWFEXT_FUNC_END();
}


int ezcast_Get_language_index(void)
{
	return language_index;
}
void ezcast_sysinfo_Set_language_index(int lan_index){
	
	language_index=lan_index;
}

int ezcast_sysinfo_get_cpu_frequency(void)
{
	int freq_cpu = 0;
	unsigned long reg = 0;
	

	printf("[%s][%d] -- Get cpu frequency\n", __func__, __LINE__);
	reg = reg_readl(0xB0010000);
	printf("[%s][%d] -- reg: 0x%x\n", __func__, __LINE__, reg);
	freq_cpu = ((reg >> 2) & 0xff) * 6;
	printf("[%s][%d] -- cpu frequency: %d\n", __func__, __LINE__, freq_cpu);
	
	return freq_cpu;
}

char* ezcast_sysinfo_get_ip(int port)
{
	static char ipAddr[16];
	int ret = -1;
	//printf("[%s][%d] -- port: %d\n", __func__, __LINE__, port);
	ret = getIP(port, ipAddr);
	if(ret < 0){
		printf("[%s][%d] -- Get IP error!!!\n", __func__, __LINE__);
		sprintf(ipAddr, "error");
	}
	return(ipAddr);
}
char* ezcast_sysinfo_get_ipaddress(int port)
{
	static char ipAddr[16];
	int ret = -1;
	printf("[%s][%d] -- port: %d\n", __func__, __LINE__, port);
	ret = getIPAddress(port, ipAddr);
	if(ret < 0){
		printf("[%s][%d] -- Get IP error!!!\n", __func__, __LINE__);
		sprintf(ipAddr, "error");
	}
	return(ipAddr);
}


void ezcast_sysinfo_set_router_ctl(int val,int storage)
{
	setConnectMode(val);
}
int ezcast_sysinfo_get_router_ctl()
{
	return(getConnectMode());
}

#endif


#if AIRDISK_ENABLE
#define MAX_LEN		(64)
int get_usb_dev(char *dev){
	FILE *fp = NULL;
	int ret = -1;
	printf("[%s][%d] -- dev:\n", __func__, __LINE__);

	if(dev == NULL){
		printf("[%s][%d] -- Parameter error!!\n", __func__, __LINE__);
		return -1;
	}
	//df -h | grep "/dev/sd"
	//df -h | grep "/dev/sd" > 1 
	//unlink("/tmp/test_tmp");
	printf("[%s][%d] -- dev:\n", __func__, __LINE__);
	system("df -h");
	system("df -h | grep \"/dev/sd[a~b]\" | awk '{print $1}' > /tmp/test_tmp");
	printf("[%s][%d] -- dev:\n", __func__, __LINE__);
	fp = fopen("/tmp/test_tmp", "r");
	printf("[%s][%d] -- dev:\n", __func__, __LINE__);
	if(fp != NULL){
		char u_path[MAX_LEN];
		printf("[%s][%d] -- path:%s\n", __func__, __LINE__, u_path);
		ret = fread(u_path, 1, MAX_LEN, fp);
		fclose(fp);
		printf("[%s][%d] -- path:%s\n", __func__, __LINE__,u_path);
		unlink("/tmp/test_tmp");
		char *end = strchr(u_path, '\n');
		if(end != NULL){
			printf("[%s][%d] -- %d\n", __func__, __LINE__, __LINE__);
			char *end_r = strchr(u_path, '\r');
			if(end_r != NULL){
				end = (end < end_r)?end:end_r;
			}
			int len = end - u_path;
			memcpy(dev, u_path, len);
			dev[len] = '\0';
		}else{
			snprintf(dev, MAX_LEN, "%s", u_path);
		}
		printf("[%s][%d] -- usb dev: %s\n", __func__, __LINE__, dev);
		if(ret > 0)
			return 0;
		else
			return -1;
	}

	return -1;
}
int get_sdcard_dev(char *dev)
{

	FILE *fp = NULL;
	int ret = -1;
	
	if(dev == NULL){
		printf("[%s][%d] -- Parameter error!!\n", __func__, __LINE__);
		return -1;
	}
	system("df -h");
	system("df -h | grep \"/dev/sd_card_block[1-9]*\" | awk '{print $1}' > /tmp/test_tmp1");
	fp = fopen("/tmp/test_tmp1", "r");
	if(fp != NULL){
		char u_path[MAX_LEN];
		ret = fread(u_path, 1, MAX_LEN, fp);
		fclose(fp);
		unlink("/tmp/test_tmp1");
		char *end = strchr(u_path, '\n');
		if(end != NULL){
			//printf("[%s][%d] -- %d\n", __func__, __LINE__, end-1);
			char *end_r = strchr(u_path, '\r');
			if(end_r != NULL){
				end = (end < end_r)?end:end_r;
			}
			int len = end - u_path;
			memcpy(dev, u_path, len);
			dev[len] = '\0';
		}else{
			snprintf(dev, MAX_LEN, "%s", u_path);
		}
		printf("[%s][%d] -- get sdcard dev: %s\n", __func__, __LINE__, dev);
		if(ret > 0)
			return 0;
		else
			return -1;
	}

	return -1;
}


//add three interface for usb mount and umount ;
static int mount_usb_tohttp(void *handle)
{	
	
	char dev[64];
	char cmd[128];
	int audio_sta=0,video_sta=0,ez_play_sta=0;
	SWFEXT_FUNC_BEGIN(handle);
	//printf("[%s][%d] -- dev:\n", __func__, __LINE__);
	get_usb_dev(dev);
	//printf("[%s][%d] -- dev: %s\n", __func__, __LINE__, dev);
	if(strncmp(dev, "/dev/", strlen("/dev/")) == 0){
		snprintf(cmd, sizeof(cmd), "umount -lf %s", dev);
		if(system(cmd)<0)
			printf("---umount %s fail---\n", dev);
            /* Check folder before mount, if not exist, create it */
            system("if [ ! -e /mnt/user1/thttpd/html/airusb/usb ]; then mkdir -p /mnt/user1/thttpd/html/airusb/usb; fi");
			/* fix bug 64g sdcard cannot mount*/
			snprintf(cmd, sizeof(cmd), "mount %s  /mnt/user1/thttpd/html/airusb/usb	-o rw,fmask=111,dmask=111 ", dev);
/*
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074	
			snprintf(cmd, sizeof(cmd), "mount %s  /mnt/user1/thttpd/html/airusb/usb	-o rw,umask=111 ", dev);
		
#else
			snprintf(cmd, sizeof(cmd), "mount %s /mnt/user1/thttpd/html/airusb/usb -o rw,umask=111", dev);
#endif
*/
		printf("[%s][%d] -- cmd: %s\n", __func__, __LINE__, cmd);
		if(system(cmd)<0)
			printf("---mount airusb fail---\n");
	}


	//usb  plug in after startup ,enter audio_loopback mode
	#if 0
	if(usbplugin_afterup==1)
	{
		audio_sta=ezcast_ae_get_state();
		video_sta=ezcast_ve_get_state();
		ez_play_sta=ezPlayerOnIdle();//wifi subdispaly media state
		printf("---audio_loopback mode[usb loopbackplay]---\n");
		printf("---audio_sta=%d,video_sta=%d---\n",audio_sta,video_sta);
		if(access("/mnt/user1/thttpd/html/airusb/usb/ezcast_test.conf",0)==0)
			printf("---test config file exist,quit usb audio_loopback mode[usb]  ---\n");
		else
		{
			if(((audio_sta==AE_IDLE)||(audio_sta==-1)) && (video_sta==0)&&(ez_play_sta==1))//VE_IDLE
				audio_loopback_card_usb(1);
			else
				printf("---Audio or Video Playing,quit audio_loopback mode[usb]---\n");

		}
	}
	#endif
	SWFEXT_FUNC_END();

}
static int umount_usb(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	printf("unmount usb at http server \n");
	//device may be busy, -l ,wait device free . 
	if(system("umount -lf /mnt/user1/thttpd/html/airusb/usb")<0)
		printf("---umount airusb fail---\n");
	SWFEXT_FUNC_END();

}

static int mount_card_tohttp(void *handle)
{	


	char dev[128];
	char cmd[256];
	int audio_sta=0,video_sta=0,ez_play_sta=0;
	SWFEXT_FUNC_BEGIN(handle);
	get_sdcard_dev(dev);
	//printf("[%s][%d] -- dev: %s\n", __func__, __LINE__, dev);
	if(strncmp(dev, "/dev/", strlen("/dev/")) == 0){
		snprintf(cmd, sizeof(cmd), "umount -lf %s", dev);
		if(system(cmd)<0)
			printf("---umount %s fail---\n", dev);
            printf("---umount %s fail---\n", dev);
            /* Check folder before mount, if not exist, create it */
            system("if [ ! -e /mnt/user1/thttpd/html/airusb/sdcard ]; then mkdir -p /mnt/user1/thttpd/html/airusb/sdcard; fi");
		/* fix bug 64g sdcard cannot mount*/
		snprintf(cmd, sizeof(cmd), "mount %s /mnt/user1/thttpd/html/airusb/sdcard -o rw,fmask=111,dmask=111", dev);		 
/*
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074	
		snprintf(cmd, sizeof(cmd), "mount %s /mnt/user1/thttpd/html/airusb/sdcard -o rw", dev);
#else
		snprintf(cmd, sizeof(cmd), "mount %s /mnt/user1/thttpd/html/airusb/sdcard -o rw,umask=111", dev);
#endif
*/
		printf("[%s][%d] -- cmd: %s\n", __func__, __LINE__, cmd);
		if(system(cmd)<0)
			printf("---mount airusb fail---\n");
		}
	//sdcard plug in after startup,audio_loopback mode 
	#if 0
	if(sdcardplugin_afterup==1)
	{
		
		printf("---audio_loopback mode[sdcard loopbackplay]---\n");
		audio_sta=ezcast_ae_get_state();
		video_sta=ezcast_ve_get_state();
		ez_play_sta=ezPlayerOnIdle();//wifi subdispaly media state
		printf("---audio_sta=%d,video_sta=%d---\n",audio_sta,video_sta);
		if((audio_sta==AE_IDLE)||(audio_sta==-1) && (video_sta==0)&&(ez_play_sta==1))//VE_IDLE
			audio_loopback_card_usb(0);
		else
			printf("---Audio or Video Playing,quit audio_loopback mode[sdcard]---\n");
	}
	#endif 
	SWFEXT_FUNC_END();

}

static int umount_card(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	printf("unmount sdcard at http server \n");
	//device may be busy, -l ,wait device free . 
	if(system("umount -lf /mnt/user1/thttpd/html/airusb/sdcard")<0)
		printf("---umount airusb fail---\n");
	SWFEXT_FUNC_END();

}

/*
static int mount_memory_tohttp(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	printf("mount memory to http \n");
	if(system("chmod 666 -R /mnt/user1/thttpd/html/airusb/")<0)
		printf("---chmod airusb fail---\n");
	printf("Airdisk:umount /mnt/udisk\n");
	if(system("umount -lf /mnt/udisk")<0)
		printf("---umount memory fail---\n");
	if(system("mount /dev/partitions/udisk /mnt/user1/thttpd/html/airusb/memory -o rw,umask=111")<0)
		printf("---mount memory fail---\n");
	SWFEXT_FUNC_END();

}

*/
#endif   //#end if AIRDISK_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
#define TEST_ENABLE	0 // ezcastpro projector doesn't support test
#else
#define TEST_ENABLE	1
#endif
#if TEST_ENABLE
#define TEST_CONFIG_FILE	"ezcast_test.conf"
#define GET_TEST_ENABLE		0
#define GET_TEST_SSID		1
#define GET_TEST_PSK		2
#define GET_TEST_VIDEO_FILE	3
#define GET_TEST_LOOP_NUM	4
#define PATH_MAX_LEN		(64)
#define HDMI_EDID_READ          (0x21)


int get_usb_path(char *path){
	FILE *fp = NULL;
	int ret = -1;
	
	if(path == NULL){
		printf("[%s][%d] -- Parameter error!!\n", __func__, __LINE__);
		return -1;
	}
	system("df -h | grep \"/dev/sd[a~b]\" | awk '{print $6}' > /tmp/test_tmp");
	fp = fopen("/tmp/test_tmp", "r");
	if(fp != NULL){
		char u_path[PATH_MAX_LEN];
		ret = fread(u_path, 1, PATH_MAX_LEN, fp);
		fclose(fp);
		unlink("/tmp/test_tmp");
		char *end = strchr(u_path, '\n');
		if(end != NULL){
			//printf("[%s][%d] -- %d\n", __func__, __LINE__, end-1);
			char *end_r = strchr(u_path, '\r');
			if(end_r != NULL){
				end = (end < end_r)?end:end_r;
			}
			int len = end - u_path;
			memcpy(path, u_path, len);
			path[len] = '\0';
		}else{
			snprintf(path, PATH_MAX_LEN, "%s", u_path);
		}
		printf("[%s][%d] -- usb path: %s\n", __func__, __LINE__, path);
		if(ret > 0)
			return 0;
		else
			return -1;
	}

	return -1;
}

static int get_config_path(char *path){
	char u_path[PATH_MAX_LEN];
	memset(u_path, 0, sizeof(path));
	get_usb_path(u_path);
	printf("[%s][%d] -- path: %s\n", __func__, __LINE__, u_path);
	snprintf(path, PATH_MAX_LEN, "%s/%s", u_path, TEST_CONFIG_FILE);

	return 0;
}

static int find_string_test_config(const char *name, char *data, int len){
	FILE *fp = NULL;
	char buff[1024];
	char _name[32];
	char path[128];
	char *p1 = NULL, *p2 = NULL, *p3 = NULL;
	int ret = -1;
	int l;

	get_config_path(path);
	printf("[%s][%d] -- Get test config [%s]!!!\n", __func__, __LINE__, path);
	if(access(path, F_OK) == 0){
		fp = fopen(path, "r");
		if(fp != NULL){
			ret = fread(buff, 1, sizeof(buff)-1, fp);
			fclose(fp);
			if(ret > 0){
				buff[ret] = '\n';
				snprintf(_name, sizeof(_name), "%s:", name);
				p1 = strcasestr(buff, _name);
				if(p1 != NULL){
					p1 += strlen(_name);
					p2 = strchr(p1, '\n');
					if(p2 != NULL){
						l = p2 - p1;
						p3 = strchr(p1, '\r');
						if(p3 != NULL){
							if(p3 < p2){
								l = p3 - p1;
							}
						}
						while(l>0){
							printf("[%s][%d] -- p1[0] = %c, p1[0] = %d\n", __func__, __LINE__, p1[0], p1[0]);
							if(p1[0] != 32)		// Do not copy space
								break;
							else{
								p1++;
								l--;
							}
						}
						while(l>0){
							printf("[%s][%d] -- p1[l-1] = %c, p1[l-1] = %d\n", __func__, __LINE__, p1[l-1], p1[l-1]);
							if(p1[l-1] != 32)		// Do not copy space
								break;
							else
								l--;
						}
						printf("[%s][%d] -- l: %d\n", __func__, __LINE__, l);
						//snprintf(data, len, "%s", p1);
						int length = (l<(len-1))?l:(len-1);
						memcpy(data, p1, length);
						data[length] = '\0';
						printf("[%s][%d] -- data: %s\n", __func__, __LINE__, data);
						return 0;
					}
				}
				printf("[%s][%d] -- get name error!!!\n", __func__, __LINE__);
			}
		}
	}

	return -1;
}

static int get_test_config(char *flag, char *cfg, int len){
	char path[128];
	int ret = -1;
	
	if(strcmp(flag, "get_test_enable") == 0){
		get_config_path(path);
		printf("[%s][%d] -- TEST_CONFIG_FILE: %s\n", __func__, __LINE__, path);
		if(access(path, F_OK) == 0){
			sprintf(cfg, "enable");
			SysCGI_priv.enter_factory_test_video=1;
		}else{
			sprintf(cfg, "disable");
		}
		ret = 0;
	}else if(strcmp(flag, "get_usb_path") == 0){
		ret = get_usb_path(cfg);
	}else{
		ret = find_string_test_config(flag, cfg, len);
	}

	return ret;
}

static int sysinfo_get_test_config(void *handle){
	int ret=-1;
	char cfg[128];
	char  *flag;
	
	SWFEXT_FUNC_BEGIN(handle);

	flag = Swfext_GetString();
	printf("\nflag is: %s\n", flag);
	ret = get_test_config(flag, cfg, sizeof(cfg));
	if(ret != 0)
		sprintf(cfg, "null");
	printf("[%s][%d] -- cfg: %s\n", __func__, __LINE__, cfg);
	Swfext_PutString(cfg);

	SWFEXT_FUNC_END();
}
static int sysinfo_set_factory_test_video_end(void *handle)
{	
	SWFEXT_FUNC_BEGIN(handle);
	SysCGI_priv.enter_factory_test_video=Swfext_GetNumber();
	SWFEXT_FUNC_END();
}



/*
api and  Variable for factory test
*/
#define factory_test_start "testmode=true"
#define factory_test_video "testitem=0"
#define factory_test_wifi "testitem=1"
#define factory_test_mulitest "testitem=2"	
#define factory_test_wifichannel "wifi_channel_test=true"
#define factory_test_edid "edid_test=true"
#define factory_test_version "version_conf="
#define factory_test_language "language_conf="
#define factory_test_lan "lan_test=true"
#define factory_test_uart "uart_test=true"

#define AP1_SSID "AP1"
#define AP2_SSID "AP2"
#define AP3_SSID "AP3"

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
#define AP4_SSID "AP4"
#define AP5_SSID "AP5"
#define AP6_SSID "AP6"
int 	channel4_signal	=0;
int 	channel5_signal	=0;
int 	channel6_signal	=0;


int 	tmp_channel4_signal	=0;
int 	tmp_channel5_signal	=0;
int 	tmp_channel6_signal	=0;

#endif
int   	iperf_sta		=0;
int   	to_testswf_flag	=0;// 1 to test.swf ,2 to main.swf 
int 	count_item		=0;
int 	channel1_signal	=0;
int 	channel2_signal	=0;
int 	channel3_signal	=0;

int 	tmp_channel1_signal	=0;
int 	tmp_channel2_signal	=0;
int 	tmp_channel3_signal	=0;
int     fresh_time=0;
char 	old_channel[10] ="";
char 	version_buf[10] ="";
char 	language_buf[10]="";

unsigned  char 	itemlist=0x0;
unsigned  char 	old_itemlist=0x0;

float	wifithroughput	=0.00;

char hdmi_modes[19][15] = {"1920*1080_60I","1920*1080_50I","1920*1080_30P","1920*1080_25P","1920*1080_24P"
						 ,"1280*800_60P","1280*800_30P","1280*800_50P","1280*800_25P","1280*800_24P"
						 ,"1920*1152_50I","720*480_60I","720*576_50I","720*480_60P","720*576_50P"
						 ,"1024*768_60P","1280*800_60P","","1920x1080_60P","DEFAULT_OUTPUT_RESOLUTION"};

char vga_modes[][15]={"1280_1024","1280_768","800_600","640_480","1280_800","1920_1080","1366_768","1280_720","1920_1080_30"," "}; 
/*result for cgi show */
char fact_edid_result[15]="";
char fact_hdcpkey_result[15]="";
char fact_version_result[50]="";
char fact_language_result[50]="";
char fact_lan_result[50]="";
char fact_uart_result[50]="";
int fact_item=0;
int old_fact_item=0;
int wait_time=10;
int first_in=0;
static int sysinfo_itemlist_factorytest(void *handle)

{

  
	SWFEXT_FUNC_BEGIN(handle);
	//printf("-itemlist1=%d %d-\n",itemlist,count_item);
	if(count_item>100000)count_item=0;
	int testitemlist=(itemlist&0x01)+(itemlist>>1&0x01)*10+(itemlist>>2&0x01)*100+(itemlist>>3&0x01)*1000+(itemlist>>4&0x01)*10000;
	fact_item=testitemlist;
	if((fact_item!=0)&&first_in==0)  //new cmd get  
	{
		first_in=1;  //set new cmd receiving   flag 
		old_fact_item=0;  //enter waiting   
		count_item=0;  //count=0;start  timing  wait for show result in as 
	}
	wait_time=10;
	if(!(itemlist&0x01)){ 
		wait_time=wait_time-9 ;//not test wifi through 
		if(!(itemlist&0x02))      //not test wifi through  and not test wifi
		wait_time=0;
		//printf("wait time %d\n",wait_time);
	}
	count_item++;   
	//  time reach    if new cmd come  get all cmd   
	if((count_item>wait_time)&&(fact_item!=0))  
	{
		printf("start show result in as !!!-itemlist=%d-\n",itemlist);
		//get itemlist for as show result
		old_fact_item=fact_item;  
		
		//store itemlist for cgi_output item
		old_itemlist=itemlist;   

		//after get all cmd ,clear cmd ,set  readyr get new cmd flag
		itemlist=0x0;       //	
		first_in=0;   
	}
	//printf("old_fact_item%d -\n",old_fact_item);
	Swfext_PutNumber(old_fact_item);
	SWFEXT_FUNC_END();
	
}

int sysinfo_refreshdata_factorytest()
{
	memset(cgi_factorytest_result,0,256);
	fresh_time++;
	if(fresh_time<5) return -1;   //wait 5s to get test data
	//fresh time >0, cgidata show eventhrough it may be incorrect
	//app retry 15times
	printf("old_itemlist=%d,%d",old_itemlist,fresh_time);
	if(old_itemlist==0x0&&fresh_time<15) 
		return -1;
	if(old_itemlist&0x01)
	{
		if(wifithroughput==0.00&&fresh_time<15) return -1;
		sprintf(cgi_factorytest_result,"%s1=%5.2f%s,",cgi_factorytest_result,wifithroughput,"Mbits/sec");	
	}
	if(old_itemlist&0x02)
	{
		if((channel1_signal==0||channel2_signal==0||channel2_signal==0)&&fresh_time<15) return -1;
		sprintf(cgi_factorytest_result,"%s2=%d,3=%d,4=%d,",cgi_factorytest_result,channel1_signal,channel2_signal,channel3_signal);
	}
	if(old_itemlist&0x04)
	{
		if(!strcmp(fact_edid_result,"")&&fresh_time<15)return -1;
		sprintf(cgi_factorytest_result,"%s5=%s,",cgi_factorytest_result,fact_edid_result);
	}
	if(old_itemlist&0x08)
	{
		if(!strcmp(fact_version_result,"")&&fresh_time<15)return -1;
		sprintf(cgi_factorytest_result,"%s6=%s,",cgi_factorytest_result,fact_version_result);

	}
	if(old_itemlist&0x10)
	{
		
		if(!strcmp(fact_language_result,"")&&fresh_time<15)return -1;
		sprintf(cgi_factorytest_result,"%s7=%s,",cgi_factorytest_result,fact_language_result);

	}
	printf("cgi_factorytest_result=%s",cgi_factorytest_result);
	fresh_time=0;
	//cgi get all data ,clear it
	/*
	memset(fact_edid_result,0,15);
	memset(fact_version_result,0,50);
	memset(fact_language_result,0,50);
	wifithroughput=0.00;
	channel1_signal=0;
	channel2_signal=0;
	channel3_signal=0;
	fresh_time=0;
	*/
	return 0;
}

static int sysinfo_init_factorytest(void *handle)

{

  
	SWFEXT_FUNC_BEGIN(handle);
	int totestswf=to_testswf_flag;
	if(totestswf==1)
	printf("factory_test Enter test.swf\n ");
	Swfext_PutNumber(totestswf);
	if(to_testswf_flag!=0)
	to_testswf_flag=0;
	SWFEXT_FUNC_END();
	
	 
}
static int sysinfo_language_result_factorytest(void *handle)
{


	SWFEXT_FUNC_BEGIN(handle);
	char *result=Swfext_GetString();
	printf("language_result:%s",result);
	sprintf(fact_language_result,"%s",result);
	printf("fact_language_result%s\n",fact_language_result);
	//sprintf(cgi_factorytest_result,"%s7=%s,",cgi_factorytest_result,fact_language_result);
	//printf("Factroy Test Result=%s\n",cgi_factorytest_result);
	SWFEXT_FUNC_END();
	
	
}
static int sysinfo_version_result_factorytest(void *handle)
{


	SWFEXT_FUNC_BEGIN(handle);
	char *result=Swfext_GetString();
	sprintf(fact_version_result,"%s",result);
	printf("fact_version_result%s\n",fact_version_result);
	//sprintf(cgi_factorytest_result,"%s6=%s,",cgi_factorytest_result,fact_version_result);
	//printf("Factroy Test Result=%s\n",cgi_factorytest_result);
	SWFEXT_FUNC_END();
	
	
}

static int sysinfo_get_edid_factorytest(void *handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	char edid[15]="";
	sprintf(edid,"%s",fact_edid_result);
	//printf("edid=%s\n",fact_edid_result);
	Swfext_PutString(edid);
	SWFEXT_FUNC_END();
	

}

static int sysinfo_get_Throughput(void *handle)

{
	SWFEXT_FUNC_BEGIN(handle);
	char wifithroughput_data[7]="";
	if(wifithroughput!=0.00)
	sprintf(wifithroughput_data,"%7.2f",wifithroughput);
	//printf("wifithroughput_data:%s",wifithroughput_data);
	Swfext_PutString(wifithroughput_data);
	SWFEXT_FUNC_END();

	 
}

static int sysinfo_version_factorytest(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char tmp_version[10]="";
	if(strcmp(version_buf,"")){//not NULL
		sprintf(tmp_version,"%s",version_buf);
		memset(version_buf,0,10);
	}
	//printf("version:%s\n",tmp_version);
	Swfext_PutString(tmp_version);
	
	SWFEXT_FUNC_END();
	
}
static int sysinfo_language_factorytest(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	char tmp_version[10]="";
	if(strcmp(language_buf,"")){
		sprintf(tmp_version,"%s",language_buf);
		memset(language_buf,0,10);
	}
	//printf("language:%s\n",tmp_version);
	Swfext_PutString(tmp_version);
	SWFEXT_FUNC_END();
	
}
static int sysinfo_lan_factorytest(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString(fact_lan_result);
	SWFEXT_FUNC_END();
	
}
static int sysinfo_uart_factorytest(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutString(fact_uart_result);
	//printf("fact_uart_result=%s\n",fact_uart_result);
	SWFEXT_FUNC_END();
	
}
static int sysinfo_get_channel_signal(void *handle)

{

	SWFEXT_FUNC_BEGIN(handle);
	int channelsignal_data=0;
	
	if(tmp_channel1_signal>0)
	{
		//sprintf(channelsignal_data,"AP1%d",channel1_signal);
		channelsignal_data=tmp_channel1_signal;
		tmp_channel1_signal=0;
		
	}
	else if(tmp_channel2_signal>0)
	{

		//sprintf(channelsignal_data,"AP2%d",channel2_signal);
		channelsignal_data=tmp_channel2_signal+100;
		tmp_channel2_signal=0;
		

	}
	else if(tmp_channel3_signal>0)
	{

		//sprintf(channelsignal_data,"AP3%d",channel3_signal);
		channelsignal_data=tmp_channel3_signal+200;
		tmp_channel3_signal=0;
		
	}
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	else if(tmp_channel4_signal>0)
	{
		channelsignal_data=tmp_channel4_signal+300;
		tmp_channel4_signal=0;
		
	}
	else if(tmp_channel5_signal>0)
	{
		channelsignal_data=tmp_channel5_signal+400;
		tmp_channel5_signal=0;
		

	}
	else if(tmp_channel6_signal>0)
	{
		channelsignal_data=tmp_channel6_signal+500;
		tmp_channel6_signal=0;
		
	}
#endif

	Swfext_PutNumber(channelsignal_data);
	//printf("channel singnal data[%d]",channelsignal_data);
	SWFEXT_FUNC_END();
	

}

static int test_start()
{
	printf("[%s][%d] -- Verify Password:\n", __func__, __LINE__);
	if(iperf_sta==1)
	{
		system("killall iperf");
		iperf_sta=0;
	}
	return 0;

}
static int test_video()
{
	printf("[%s][%d] --Prepare for Video Play:\n", __func__, __LINE__);
	if(iperf_sta==1)
	{
		system("killall iperf");
		iperf_sta=0;
	}
	
	//to main.swf 
	return 0;
}

/*iperf   stop  if the next test cmd receive or  in 60s after server start*/
static int iperf_stop()

{
	
	if(iperf_sta==1)
	{
		system("killall iperf");
		printf("[%s][%d] -- stop iPerf server :\n", __func__, __LINE__);
		iperf_sta=0;
	}
	return 0;

}

static int  iperftimer_start()
{

	printf("[%s][%d] -- iPerf server will stop in 35s :\n", __func__, __LINE__);
	signal(SIGALRM,iperf_stop);
	struct itimerval tim;
	tim.it_value.tv_sec=35;
	tim.it_value.tv_usec=0;
	tim.it_interval.tv_usec=0;
	tim.it_interval.tv_sec=0;  //every 1 sec if  =0   just  one time
	
	int ret=setitimer(ITIMER_REAL,&tim,NULL);
	if(ret!=0)
	{
		printf("set timer error");
	}
	return 0;


}
static int get_iperf_wifithroughput()
{
	char **iperf_tmp;
	char *index;
	int i=0, n=0;
	int MAX_Len=60;
	char tmp[8];
	FILE *fp = NULL;
	fp = fopen("/tmp/iperl_data1", "r");
	iperf_tmp=(char **)malloc(0*sizeof(char **)); 
  	if(fp != NULL){
		  while(fgets(tmp,8,fp)!=NULL)
		  {
			char *end = strchr(tmp, '\n');
		   	if(end != NULL){
			//printf("[%s][%d] -- %d\n", __func__, __LINE__, __LINE__);
			   char *end_r = strchr(tmp, '\r');
			   	if(end_r != NULL){
					   end = (end < end_r)?end:end_r;
			   }
			   int len = end - tmp;
			   if(len<9)
			   {
			   	  // printf(" data len=%d",len);
				   iperf_tmp=(char **)realloc(iperf_tmp,(i+1)*sizeof(char **)); 
				   iperf_tmp[i]=(char *)malloc(sizeof(char)*len);
				   //iperf_tmp[i]=(char *)malloc(sizeof(char)*8);
				   memset(iperf_tmp[i],0,len);

				   memcpy(iperf_tmp[i], tmp, len);
				   iperf_tmp[i][len] = '\0';
				   printf("wifithroughput server data:%s Mbits/sec\n",iperf_tmp[i]);
				   i++;
				   if(i==MAX_Len)
				   {
				   		break;
				   }
			   	}
			    else
			    {
					printf("Invalid data!");
				}
		   }

		}
		int re=fclose(fp);
		if(re!=0) printf("[%s][%d] -- file close error:\n", __func__, __LINE__);
		if(i>3)
		{
			int TR_data=i;
			if(TR_data%3!=0) TR_data=(TR_data/3)*3;
			float TR_put[TR_data];
			float sum=0.00;
			for(n=0;n<TR_data;n++)
			{
				TR_put[n]=atof(iperf_tmp[n]);
				sum=sum+TR_put[n];

			}
			sum=sum/TR_data;
			wifithroughput=sum;
			printf("-----------wifithroughput ----------:\n");
			printf("%7.2f Mbits/sec\n",sum);
			printf("-----------wifithroughput ----------:\n");
		}
		for(n=0;n<i;n++)
   		{
   			//printf("[%s][%d] --  \n", __func__, __LINE__);
		   free(iperf_tmp[n]);
   		}
	}
	
	else{
		printf("[%s][%d] -- wifithroughput data is NULL:\n", __func__, __LINE__);
	}
	free(iperf_tmp);
	unlink("/tmp/iperl_data1");
	unlink("/tmp/iperl_data");
	//printf("[%s][%d] --  \n", __func__, __LINE__);
	return 0;
	//printf("[%s][%d] --  \n", __func__, __LINE__);
	
		
}

void *test_wifithroughput(void *arg)
{
	printf("[%s][%d] -- start iPerl :\n", __func__, __LINE__);
	
	if(iperf_sta==1)
	{
		system("killall iperf");
		iperf_sta=0;
	}
	iperftimer_start();
	iperf_sta=1;
	system("chmod 77 /sbin/iperf ");
	system("/sbin/iperf -s -i 1 |grep \"Mbits/sec\">/tmp/iperl_data");
	printf("[%s][%d] -- server stop get data :\n", __func__, __LINE__);
	system("cat /tmp/iperl_data |cut -d 't' -f 2 |awk '{print $2}' >/tmp/iperl_data1");
	
	get_iperf_wifithroughput();
	pthread_detach(pthread_self());
	return NULL;
	
}

//cat 1  |cut -d 't' -f 2 |awk '{print $2}'  >ts
//df -h | grep "/dev]" | awk '{print $1}'>ts
///	iperf -s -i 1 |grep "Mbits/sec"|cut -d 't' -f 2 |awk '{print $2}'


/*

to signal 1   printf ap1 signal ntensity  

*/


static int test_wifithroughput_thread() 
{
	pthread_t tid;
	int rtn = pthread_create(&tid, NULL,test_wifithroughput,NULL);
	if(rtn){
		printf("[%s][%d] -- Thread Create Error :\n", __func__, __LINE__);
	goto CREATE_THREAD_END;
	}
CREATE_THREAD_END:
	return rtn;


}
int iperf_modify_line_realtek(char * buf, char * key, char * value)
{

 	
 	char tmp[4096] = {0};
	char tmp2[256] = {0};
	char * locate1 = NULL;
	int key_len=0;
	key_len=strlen(key);
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	locate1 = strstr(buf,key);
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	memcpy(old_channel,(locate1+key_len),locate2-locate1-key_len);
	printf("<rtl_hostapd.conf> old_channel=%s\n",old_channel);
	memcpy(tmp,buf,strlen(buf)-strlen(locate1));
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	if(strcmp(key,"ssid=")==0){
		sprintf(tmp2,"ssid=%s",value);
	}
	else if(strcmp(key,"channel=")==0){
		sprintf(tmp2,"channel=%s",value);
	}
	else if(strcmp(key,"wpa=")==0){
		sprintf(tmp2,"wpa=%s",value);
	}
	else if(strcmp(key,"wpa_passphrase=")==0){
		sprintf(tmp2,"wpa_passphrase=%s",value);
	}
	else if(strcmp(key,"wpa_key_mgmt=")==0){
		sprintf(tmp2,"wpa_key_mgmt=%s",value);
	}
	else if(strcmp(key,"wpa_pairwise=")==0){
		sprintf(tmp2,"wpa_pairwise=%s",value);
	}
	else if(strcmp(key,"wps_state=")==0){
		sprintf(tmp2,"wps_state=%s",value);
	}
	else if(strcmp(key,"wme_enabled=")==0){
		sprintf(tmp2,"wme_enabled=%s",value);
	}
	else if(strcmp(key,"ht_capab=")==0){
		sprintf(tmp2,"ht_capab=%s",value);
	}
	if(tmp2==NULL)
		return -1;
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	int len1 = strlen(buf)-strlen(locate1);

	int len2 = strlen(tmp2);
	
	memcpy(tmp+len1,tmp2,len2);
	memcpy(tmp+len1+len2,locate2,strlen(locate2));
	

	memset(buf,0,strlen(buf));
	memcpy(buf,tmp,strlen(tmp));
	return 0;
}

static int set_wifichannel_conf(char *channel)
{




	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	printf("set_wifichannel_conf,channel=%s\n",channel);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	if(atoi(channel)>atoi("13"))
	{
		printf("use rtl_hostapd_01.conf\n");
		fp = fopen(HOSTAPD_CON_FILE,"r");
		if(fp == NULL){
				
		sprintf(callbuf,"cp %s %s",HOSTAPD_BAK_CON_FILE,HOSTAPD_CON_FILE);
		system(callbuf);
		fp = fopen(HOSTAPD_CON_FILE,"r");
		}
		
	}
	else
	{
	
		printf("use rtl_hostapd_02.conf\n");
		fp = fopen("/mnt/user1/softap/rtl_hostapd_02.conf","r");
		if(fp == NULL){
				
		sprintf(callbuf,"cp /mnt/bak/rtl_hostapd_02.conf  /mnt/user1/softap/rtl_hostapd_02.conf");
		system(callbuf);
		fp = fopen("/mnt/user1/softap/rtl_hostapd_02.conf","r");
		}

	}


#else
	fp = fopen(HOSTAPD_CON_FILE,"r");
	if(fp == NULL){
				
		sprintf(callbuf,"cp %s %s",HOSTAPD_BAK_CON_FILE,HOSTAPD_CON_FILE);
		system(callbuf);
		fp = fopen(HOSTAPD_CON_FILE,"r");
	}
#endif

	ret=fread(buf, 1, 4096, fp);
	ret=fclose(fp);
	modify_line_realtek(buf,"channel=",channel);
	
	if(buf!=NULL){
		//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	if(atoi(channel)>atoi("13"))
	{
		modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
		fp = fopen(HOSTAPD_CON_FILE,"wb+");
	}
	else
	{
		if(atoi(channel)<=5)
			modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
		else
			modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");
		fp = fopen("mnt/user1/softap/rtl_hostapd_02.conf","wb+");
	}
#else
		if(atoi(channel)<=5)
			modify_line_realtek(buf,"ht_capab=","[HT40+][SHORT-GI-20][SHORT-GI-40]");
		else
			modify_line_realtek(buf,"ht_capab=","[HT40-][SHORT-GI-20][SHORT-GI-40]");

		fp = fopen(HOSTAPD_CON_FILE,"wb+");
#endif

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

	return 0;
}
static int  switch_wifichannel_test(char *channel)
{

	int sys;
	printf("killall -9 hostapd:\n");
	system("killall -9 hostapd");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	printf("killall -9 hostapd_02:\n");
	system("killall -9 hostapd_02");

#endif
	sleep(1);/*2*/
	set_wifichannel_conf(channel);
	sleep(1);
	//	printf("hostapd -B /etc/rtl_hostapd_01.conf:\n");
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		printf("start hostapd\n");
		sys=system("hostapd -B /etc/rtl_hostapd_01.conf");
		sys=system("hostapd_02 -B /etc/rtl_hostapd_02.conf");
		
		pac_system("sh /am7x/case/scripts/wifi_bridge_process.sh wifi_bridge_add_br0_and_wlan1_wlan3");
#else
		sys=system("hostapd -B /etc/rtl_hostapd_01.conf");
#endif

	if(sys<0)
	{
		printf("hostapd start error:");
		
	}
	//sleep(1);
	return 0;

}
static int wifi_signal_by_ssid(char *ssid)
{
	char ssid_buf[256];
	int rtn,index=-1,i=0;
	index=-1;
	for(i=0;i<256;i++)
	{
		__get_ssid(i,ssid_buf,256);
		if(!strcmp(ssid_buf,ssid))
		{	index=i;
			printf("find ssid:%s in wifilist singnal:index:%d\n", ssid_buf,index);
			break;
		}
		else
		{
			memset(ssid_buf,0,256);
		}
	}
	rtn = __get_signal(index);
		
	if(rtn>100)
		rtn=(rtn-160)*5/3;
	
	
	if(index==-1)
	{
		printf("[%s][%d] --:SSID<%s>was not found check your router device :\n", __func__, __LINE__,ssid);
		return -1;
	}
		
	else
	{
		printf("[%s][%d] --:ssid:%s--signal:%d\n", __func__, __LINE__,ssid_buf,rtn);
		return rtn;
	}
	return 0;
}
static int test_wifichannel()
{
	printf("[%s][%d] -- start wifichannel test:\n", __func__, __LINE__);
	int rn;
	int sys;
	char ch;
	int i;
	FILE *fp=NULL;
	switch_wifichannel_test("1");
	sleep(1);
	rn=wifi_signal_by_ssid(AP1_SSID);
	channel1_signal=rn;
	if(rn>0)
	{
		printf("channel:1 --:ssid:AP1--signal:%d\n",rn);
	}
	
	switch_wifichannel_test("6");
	sleep(1);
	rn=wifi_signal_by_ssid(AP2_SSID);
	channel2_signal=rn;
	if(rn>0)
	{
		printf("channel:6 --:ssid:AP2--signal:%d\n",rn);
	}
	
	switch_wifichannel_test("11");
	sleep(1);
	rn=wifi_signal_by_ssid(AP3_SSID);
	channel3_signal=rn;
	if(rn>0)
	{
		printf("channel:11 --:ssid:AP3--signal:%d\n",rn);


	}
	
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	ezCastSetNumConfig(CONFNAME_SOFTAP_5G_COUNTRY,24);//ensure include the channel 36/157

	switch_wifichannel_test("36");
	sleep(1);
	rn=wifi_signal_by_ssid(AP4_SSID);
	channel4_signal=rn;
	if(rn>0)
	{
		printf("channel:36 --:ssid:AP4--signal:%d\n",rn);
	}


	switch_wifichannel_test("157");
	sleep(1);
	rn=wifi_signal_by_ssid(AP6_SSID);
	channel6_signal=rn;
	if(rn>0)
	{
		printf("channel:157 --:ssid:AP6--signal:%d\n",rn);
	}


	ezCastSetNumConfig(CONFNAME_SOFTAP_5G_COUNTRY,7);
	printf("set country japan-7\n");

	
	switch_wifichannel_test("100");
	sleep(1);
	rn=wifi_signal_by_ssid(AP5_SSID);
	channel5_signal=rn;
	if(rn>0)
	{
		printf("channel:100 --:ssid:AP5--signal:%d\n",rn);
	}
	ezCastSetNumConfig(CONFNAME_SOFTAP_5G_COUNTRY,24);


#endif
	tmp_channel1_signal=channel1_signal;
	tmp_channel2_signal=channel2_signal;
	tmp_channel3_signal=channel3_signal;
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	tmp_channel4_signal=channel4_signal;
	tmp_channel5_signal=channel5_signal;
	tmp_channel6_signal=channel6_signal;


#endif
	//switch back to ordinary channel
	
	//printf("killall -9 hostapd:\n");
	
	//system("killall -9 hostapd");
	//system("hostapd -B /etc/rtl_hostapd_01.conf");

	//write back to init channel 
	//switch_wifichannel_test(old_channel);
	pthread_detach(pthread_self());
	//printf("Channel Test Over switch back to old channel< channel=%s>\n",old_channel);
	return 0;

}
void *test_wifichannel_thread()
{

	pthread_t tid;
	
	int rtn = pthread_create(&tid, NULL,test_wifichannel,NULL);
	if(rtn){
		printf("[%s][%d] -- Thread Create Error :\n", __func__, __LINE__);
	goto CREATE_THREAD_END;
	}
CREATE_THREAD_END:
	return rtn;


}

static int separate_cmd(char *cmd)
{
	char *index;
	char tmp[1024]="";
	char tmp_cmd[40]=""; 
	if((index=strchr(cmd,'&'))!=NULL)
	{
		
		memcpy(tmp_cmd, cmd, index-cmd);
		sys_info_factory_test(tmp_cmd);
		//printf("tmp_CMD%s\n",tmp_cmd);
		index+=1;
		memcpy(tmp, index, strlen(cmd)+cmd-index);
		sys_info_factory_test(tmp);

	}
	else  ///last cmd 
	{
		sys_info_factory_test(cmd);
	}
	return 1;
 }
static int get_keybuf_value(char *buf,char *key,char value[10])
{
	char * locate1 = NULL;
	int key_len=0;
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	locate1 = strstr(buf,key);
	if(locate1!=NULL)
	{
		printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
		char * locate2 = NULL;
		//if((locate2=strstr(locate1,"&"))==NULL)
		locate2 = strchr(locate1,'\0');
		if(locate2!=NULL)
		{
			key_len=strlen(key);
			memcpy(value,(locate1+key_len),locate2-locate1-key_len);
		}
		//value=tmp;
		//printf("key value=%s\n",value);
	}
	return 0;
}
int read_ping_reply(){
	
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	int size=-1;
	char *locate1=NULL;
	if(access("/mnt/user1/lan/ping_result.txt",F_OK)==-1){
		printf("The file is not exsit!\n");
		return -1;
	}
	fp = fopen("/mnt/user1/lan/ping_result.txt","r");
		//fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
		if(fp == NULL){
			printf("Cann't open the file!\n");
			return -1;		
		}
	size=fread(buf, 1, 4096, fp);
	fprintf(stderr,"function:%s line:%d size:%d\n",__FUNCTION__,__LINE__,size);
	ret=fclose(fp);
	fprintf(stderr,"function:%s line:%d ret:%d\n",__FUNCTION__,__LINE__,ret);
	if(size>=strlen("0 packets received"))
		locate1=strstr(buf,"0 packets received");
	if(size==0||locate1!=NULL)
		return -1;
	else 
		return 0;
		
}
static int test_lan()  //for ezcast
{
	char callbuf[128];
	
	sprintf(callbuf,"ping www.baidu.com -w 5 > /mnt/user1/lan/ping_result.txt");
	system(callbuf);
	if(read_ping_reply()==0)
	{
		sprintf(fact_lan_result,"%s"," Successfuly");
	}
	else
		sprintf(fact_lan_result,"%s","Failed");
	printf("factory test lan: %s\n",fact_lan_result);
	return 0;
}
void wait_uart_testdata()
{
	int uart=-1;
	char buff[128]="";
	char buftest[]="factest";
	#define CMD_GET_FACTRESULT 26
	while(1){
	uart = open("/dev/uartcom",O_RDWR);
	if(uart!=-1)
	{
    		ioctl(uart,CMD_GET_FACTRESULT,buff);
		if(buff!=NULL )
		{
			printf("wait_uart_testdata=%s\n",buff);
			if(strncmp(buff,buftest,strlen(buftest))==0)
			{
				sprintf(fact_uart_result,"%s"," Successfuly");
			}
			else
				sprintf(fact_uart_result,"%s"," Failed");
			break;
		}
		
		close(uart);
		
	}
	usleep(40000);
	}

}
static int test_uart()  //for ezcast
{
	char buf[]="factest";
	#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
	sendCmdUart(buf,strlen(buf));
	sleep(5);
	wait_uart_testdata();
	#endif
	return 0;
}
static int test_edid()  //for ezcast
{
	int fd = open("/dev/lcm",O_RDWR);
	int hdmi_edid_val, vga_edid_val;
	int i;
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if(ioctl(fd,HDMI_EDID_READ,&hdmi_edid_val)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	printf("hdmi_edid_val=%d\n",hdmi_edid_val);
	
	#if (defined MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE != 8075)
	int edid=ezcast_sysinfo_read_edid_bin(1);//hdmi
	int hdmi_mode=ezcast_sysinfo_read_edid_bin(0);
	if(hdmi_mode==8)
		sprintf(fact_edid_result,"%s",vga_modes[edid]);
	else
		sprintf(fact_edid_result,"%s",hdmi_modes[edid]);
	/*for (i = 0; i < (sizeof(hdmi_modes)/sizeof(struct lcm_videomode)); i++){
		printk("--------hdmi_modes[%d].xres=%d yres=%d refresh=%d vmode=%d\n",i,hdmi_modes[i].xres,hdmi_modes[i].yres,hdmi_modes[i].refresh,hdmi_modes[i].vmode);
	}*/
	printf("factory test edid[%d] \n",edid);
	#else	
	printf("hdmi_edid_val=%d\n",vga_edid_val);
	if(ioctl(fd,HDMI_EDID_READ,&vga_edid_val)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	
	if(hdmi_edid_val==0 && vga_edid_val == 0)
		sprintf(fact_edid_result,"%s","HDMI_VGA_FAIL");
	else if(hdmi_edid_val != 0 && vga_edid_val != 0)
		sprintf(fact_edid_result,"%s", "HDMI_VGA_SUCC");
	else if(hdmi_edid_val == 0 && vga_edid_val != 0)
		sprintf(fact_edid_result, "%s", "VGA_SUCCESS");
	else
		sprintf(fact_edid_result, "%s", "HDMI_SUCCESS");
	#endif
	printf("factory test result :%s\n", fact_edid_result);
	return 0;
}
static int sysinfo_test_edid(void *handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	int fd = open("/dev/lcm",O_RDWR);
	int edid_val;
	if(fd<0){
		sysdbg_err("Sorry Open lcm cfg Error\n");
		return -1;
	}
	if(ioctl(fd,HDMI_EDID_READ,&edid_val)<0){
		sysdbg_err("de get config error\n");
		close(fd);
		return -1;
	}
	printf("edid_val=%d\n",edid_val);
	#if 0
	int edid=ezcast_sysinfo_read_edid_bin(1);//hdmi
	int hdmi_mode=ezcast_sysinfo_read_edid_bin(0);
	if(hdmi_mode==8)
		sprintf(fact_edid_result,"%s",vga_modes[edid]);
	else
		sprintf(fact_edid_result,"%s",hdmi_modes[edid]);
	#endif
	if(edid_val==0)
		sprintf(fact_edid_result,"%s","EDID_FAIL");
	else
		sprintf(fact_edid_result,"%s","EDID_OK");
	printf("test edid=%s\n",fact_edid_result);
	Swfext_PutString(fact_edid_result);
	SWFEXT_FUNC_END();



}
typedef enum
{
   H2_OK = 0,
   H2_ERROR
} H2status;

static int sysinfo_test_hdcpkey(void *handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	H2status rc = H2_ERROR;
	int fd;
	unsigned char HDCPKey[864];
	memset(fact_hdcpkey_result,0,sizeof(fact_hdcpkey_result));
	do
	{
		fd = open("/dev/am7x-cipher",2);

		if (fd < 0) {
			printf("open cipher error\n");
			snprintf(fact_hdcpkey_result,sizeof(fact_hdcpkey_result),"%s","HDCPKEY_FAIL");	
		}
		else
		{
			rc=ioctl(fd,0,(unsigned long)HDCPKey);
			close(fd);
			if(H2_OK==rc)
			{
				printf("Decrypt HDCP Key Ok\n");
				snprintf(fact_hdcpkey_result,sizeof(fact_hdcpkey_result),"%s","HDCPKEY_OK");	
			}
			else
			{
				printf(">>> No HDCP key!\n");
				snprintf(fact_hdcpkey_result,sizeof(fact_hdcpkey_result),"%s","HDCPKEY_FAIL");	
			}
		}

		

	} while(0);
	Swfext_PutString(fact_hdcpkey_result);
	SWFEXT_FUNC_END();

}
int sys_info_factory_test(char *cmd)

{
	 printf("[%s][%d] -- Factory_test CMD:\n", __func__, __LINE__);
	 printf("%s\n",cmd);
	 char *index=NULL;	
	 if(strlen(cgi_factorytest_result)>200)
				memset(cgi_factorytest_result,0,200);
	if(!strcmp(cmd, factory_test_start))
	{
		 Dongle_response=1;
		 to_testswf_flag=2;
		 itemlist=0x0;
		 test_start() ;
		 
	}
	else if(!strcmp(cmd, factory_test_video))
	{
		 Dongle_response=1;
		 to_testswf_flag=2;
		 test_video();
		 
	}
	else if(!strcmp(cmd, factory_test_wifi))
	{
		 Dongle_response=1;
		 to_testswf_flag=1;
		 itemlist=itemlist&(~0x01);
		 itemlist=itemlist|0x01;
		 test_wifithroughput_thread() ;
		 
	}
	else if((strchr(cmd,'&'))!=NULL)
		 separate_cmd(cmd);
	else if(!strcmp(cmd, factory_test_mulitest)) 
		 //est_mulitest() ;
		{
		 printf("mulitest");
		 Dongle_response=1;
		 // next cmd come stop iperf
		 if(iperf_sta==1)
		{
			system("killall -9 iperf");
			printf("---iperf server stop get iperf data---");
			iperf_sta=0;
		}
		
		 to_testswf_flag=1;
		}
	
	else if(!strcmp(cmd, factory_test_wifichannel))
		{

		
		 itemlist=itemlist&(~0x02);
		 itemlist=itemlist|0x02;	
		 test_wifichannel_thread();	
		}
	else if(!strcmp(cmd, factory_test_edid))
		
	{
		itemlist=itemlist&(~0x04);
		itemlist=itemlist|0x04;
		memset(fact_edid_result,15,0);
		test_edid();
			
	}
	
	else if(!strcmp(cmd, factory_test_lan)){
		itemlist=itemlist&(~0x20);
		itemlist=itemlist|0x20;
		memset(fact_lan_result,15,0);
		test_lan();
			
	}
	else if(!strcmp(cmd, factory_test_uart)){
		itemlist=itemlist&(~0x40);
		itemlist=itemlist|0x40;
		memset(fact_uart_result,15,0);
		test_uart();
			
	}
	else
	{

		 if((index=strstr(cmd, factory_test_version))!=NULL){
		 	  itemlist=itemlist&(~0x08);
			  itemlist=itemlist|0x08;
			  get_keybuf_value(cmd,factory_test_version,version_buf);
			  printf("--factory_test config version:[%s]--",version_buf);
		 }
		else if((index=strstr(cmd, factory_test_language))!=NULL){
			itemlist=itemlist&(~0x10);
			itemlist=itemlist|0x10;
		 	get_keybuf_value(cmd,factory_test_language,language_buf);
			printf("--factory_test config language:[%s]--",language_buf);
			
		 }
		else
		{

			 printf("Unknow Command");
		}

	}
	 
	 return 0;

}

#endif   //#endif  TEST_ENABLE
int getConnectMode(){
	return ezCastGetNumConfig(CONFNAME_CTLMODE);
}

void setConnectMode(int val){
	if(val < CTLMODE_START || val > CTLMODE_END){
		EZCASTWARN("arg[%d] error\n", val);
		return;
	}

	int oldVal = ezCastGetNumConfig(CONFNAME_CTLMODE);
	if(val != oldVal){
		ezCastSetNumConfig(CONFNAME_CTLMODE, val);
#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)
		if(get_last_ui()==2)
		{
		}
		else 
#endif
		if(val == CTLMODE_DIRECTONLY){
			ezCastSetRouterCtlEnable(0);
		}else{
			ezCastSetRouterCtlEnable(1);
		}
	}

#if ROUTER_ONLY_ENABLE
#if(MODULE_CONFIG_EZCASTPRO_MODE==8075)
	if(get_last_ui()==2)
	{
	}
	else 
#endif
	if(val == CTLMODE_ROUTERONLY){
		setRouterOnlyStatus(1);
		if(get_ap_close_flag() == 0 && (getLanConnectStatus() != 0 || wpa_connect_status() == WIFI_COMPLETED)){
			wifi_softap_close();
		}
	}else{
		setRouterOnlyStatus(0);
		if(get_ap_close_flag() != 0){
			wifi_softap_start();
		}
	}
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE!=0)
	if(val != oldVal){
		system_reboot();
	}
#endif
}

static int sysinfo_get_router_ctl(void *handle){
	int val = 0;
	
	SWFEXT_FUNC_BEGIN(handle);

	val = isRouterCtlEnable();
	Swfext_PutNumber(val);

	SWFEXT_FUNC_END();
}
void setChannel(int val,int channle)
{
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
	if(channle==wifiChannel_mode_5G){
		//channel_plan,24G:1~13 5G:36~165
		wifiChannel_mode=1;
		ezCastSetNumConfig(CONFNAME_SOFTAP_5G_CHANNEL, val);
		printf("Probox use setChannel 5G=%d\n",val);
	}
	else{
		wifiChannel_mode=0;
		ezCastSetNumConfig(CONFNAME_SOFTAP_CHANNEL, val);
		printf("Probox use setChannel 24G=%d\n",val);
	}
#else 
		ezCastSetNumConfig(CONFNAME_SOFTAP_CHANNEL, val);
		printf("use setChannel 24G=%d\n",val);
#endif 
	
	int oldVal = ezCastGetNumConfig(CONFNAME_CTLMODE);
#if ROUTER_ONLY_ENABLE
	if(oldVal == CTLMODE_ROUTERONLY){
		if(get_ap_close_flag() == 0 && (getLanConnectStatus() != 0 || wpa_connect_status() == WIFI_COMPLETED)){
			wifi_softap_close();
			wifi_softap_start();
		}
	}
	else{
		if(get_ap_close_flag() != 0){
		}
		else
		{
			wifi_softap_close();
			wifi_softap_start();
		}
	}
#else
	wifi_softap_close();
	wifi_softap_start();
#endif
}

static int sysinfo_get_connect_mode(void *handle){
	int val = 0;
	
	SWFEXT_FUNC_BEGIN(handle);

	val = getConnectMode();
	Swfext_PutNumber(val);

	SWFEXT_FUNC_END();
}

static int sysinfo_set_connect_mode(void *handle){
	int val = 0, storage = 0;
	
	SWFEXT_FUNC_BEGIN(handle);

	val = Swfext_GetNumber();
	setConnectMode(val);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_system_time(void *handle){
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_PutNumber(time(NULL));
	
	SWFEXT_FUNC_END();
}

static int sysinfo_get_cpu_frequency(void *handle){
	int freq_cpu = 0;
	unsigned long reg = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s][%d] -- Get cpu frequency\n", __func__, __LINE__);
	reg = reg_readl(0xB0010000);
	printf("[%s][%d] -- reg: 0x%x\n", __func__, __LINE__, reg);
	freq_cpu = ((reg >> 2) & 0xff) * 6;
	printf("[%s][%d] -- cpu frequency: %d\n", __func__, __LINE__, freq_cpu);
	
	Swfext_PutNumber(freq_cpu);

	SWFEXT_FUNC_END();
}
static int sysinfo_get_ddr_type(void *handle){
	int ddr_type= 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	printf("[%s][%d] -- Get DDR_TYPE\n", __func__, __LINE__);
	#ifdef MODULE_CONFIG_DDR_TYPE
		ddr_type=MODULE_CONFIG_DDR_TYPE;
	#else
		ddr_type=0;
	#endif
	printf("[%s][%d] -- ddr_type: %d\n", __func__, __LINE__, ddr_type);
	Swfext_PutNumber(ddr_type);

	SWFEXT_FUNC_END();
}
static int sysinfo_get_ota_download_number(void *handle){
	int ota_download_num_count = -1;
	SWFEXT_FUNC_BEGIN(handle);

	ota_download_num_count = get_ota_download_number();
	printf("[%s][%d] -- Get ota download number!! ota_download_num_count: %d\n", __func__, __LINE__, ota_download_num_count);
	Swfext_PutNumber(ota_download_num_count);

	SWFEXT_FUNC_END();
}

static int sysinfo_get_ota_enforce(void *handle){
	int ota_enforce_val = 0;
	SWFEXT_FUNC_BEGIN(handle);

	ota_enforce_val = get_ota_enforce();
	#if EZMUSIC_ENABLE
	//if(ota_enforce_val)
		SysCGI_priv.new_firmware_version_flag=1;
	#endif
	printf("[%s][%d] -- Get ota enforce flag!! ota_enforce_val: %d\n", __func__, __LINE__, ota_enforce_val);
	Swfext_PutNumber(ota_enforce_val);

	SWFEXT_FUNC_END();
}

int if_a_string_is_a_valid_ipv4_address(const char *str)
{
    struct in_addr addr;
    int ret;
    volatile int local_errno;

    errno = 0;
    ret = inet_pton(AF_INET, str, &addr);
    local_errno = errno;
    if (ret > 0){
        //EZCASTLOG("\"%s\" is a valid IPv4 address\n", str);
		return 0;
    }else if (ret < 0){
        EZCASTLOG("EAFNOSUPPORT: %s\n", strerror(local_errno));
		return -1;
    }else{
        EZCASTLOG("\"%s\" is not a valid IPv4 address\n", str);
		return -1;
    }

    return ret;
}

int getIP(int port, char *ip){
	FILE *fp = NULL;
	char val[17], cmd[128];
	char *chr = NULL;
	int ret = 0;
	
	if(ip == NULL){
		printf("[%s][%d] -- \"ip\" is NULL!!!\n", __func__, __LINE__);
		return -1;
	}
	//printf("[%s][%d] -- port: %d wpa24g_connected:%d\n", __func__, __LINE__, port,wpa24g_connected);
	if(port == 0){
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
			if(wpa24g_connected==1)
				sprintf(cmd, "ifconfig wlan2 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
			else
				sprintf(cmd, "ifconfig wlan0 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
		#else
			sprintf(cmd, "ifconfig wlan0 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
		#endif

	}
	else if(port == 2){
		sprintf(cmd, "ifconfig wlan2 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
	}
	else{//softap ip
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		       if(get_last_ui()==2)
			   	sprintf(cmd, "ifconfig wlan3 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
			else		
				sprintf(cmd, "ifconfig br0 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
		#else
			
			sprintf(cmd, "ifconfig wlan1 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
		#endif
	}
	//printf("[%s][%d] -- cmd: %s\n", __func__, __LINE__, cmd);
	system(cmd);
	fp = fopen("/tmp/ipAddr_EZCast", "r");
	if(fp == NULL){
		printf("[%s][%d] -- Open tmp file error!!!\n", __func__, __LINE__);
		return -1;
	}
	ret = fread(val, 1, sizeof(val), fp);
	fclose(fp);
	unlink("/tmp/ipAddr_EZCast");
	if(ret < 7){
		printf("[%s][%d] -- Get IP error!!!\n", __func__, __LINE__);
		return -1;
	}
	val[ret] = '\0';
	chr = strchr(val, '\n');
	if(chr != NULL){
		*chr = '\0';
		//printf("[%s][%d] -- chr point: %d[\\n]\n", __func__, __LINE__, chr - val);
	}
	chr = strchr(val, '\r');
	if(chr != NULL){
		*chr = '\0';
		//printf("[%s][%d] -- chr point: %d[\\r]\n", __func__, __LINE__, chr - val);
	}
	//printf("[%s][%d] -- val: %s\n", __func__, __LINE__, val);
	sprintf(ip, "%s", val);
	if(if_a_string_is_a_valid_ipv4_address(ip) != 0)
		return -1;
	//printf("[%s][%d] -- ip: %s\n", __func__, __LINE__, ip);
	return 0;
}
int getIPAddress(int port, char *ip){
	FILE *fp = NULL;
	char val[17], cmd[128];
	char *chr = NULL;
	int ret = 0;
	
	if(ip == NULL){
		printf("[%s][%d] -- \"ip\" is NULL!!!\n", __func__, __LINE__);
		return -1;
	}
        if(port == 1||port == 3){
		#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075
		if(get_last_ui()==2)
			snprintf(cmd,sizeof(cmd), "ifconfig wlan\"%d\" | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast",port);
		else		
			snprintf(cmd,sizeof(cmd), "ifconfig br0 | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast");
		#else
			
			snprintf(cmd,sizeof(cmd),"ifconfig wlan\"%d\" | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast",port);
		#endif
        	}
	else
		snprintf(cmd,sizeof(cmd), "ifconfig wlan\"%d\" | grep \"inet addr\" | cut -d: -f2 | cut -d\\  -f1 > /tmp/ipAddr_EZCast",port);
	system(cmd);
	fp = fopen("/tmp/ipAddr_EZCast", "r");
	if(fp == NULL){
		printf("[%s][%d] -- Open tmp file error!!!\n", __func__, __LINE__);
		return -1;
	}
	ret = fread(val, 1, sizeof(val), fp);
	fclose(fp);
	unlink("/tmp/ipAddr_EZCast");
	if(ret < 7){
		printf("[%s][%d] -- Get IP error!!!\n", __func__, __LINE__);
		return -1;
	}
	val[ret] = '\0';
	chr = strchr(val, '\n');
	if(chr != NULL){
		*chr = '\0';
		//printf("[%s][%d] -- chr point: %d[\\n]\n", __func__, __LINE__, chr - val);
	}
	chr = strchr(val, '\r');
	if(chr != NULL){
		*chr = '\0';
		//printf("[%s][%d] -- chr point: %d[\\r]\n", __func__, __LINE__, chr - val);
	}
	//printf("[%s][%d] -- val: %s\n", __func__, __LINE__, val);
	sprintf(ip, "%s", val);
	if(if_a_string_is_a_valid_ipv4_address(ip) != 0)
		return -1;
	//printf("[%s][%d] -- ip: %s\n", __func__, __LINE__, ip);
	return 0;
}
static int sysinfo_get_ip(void *handle){
	char ipAddr[16];
	int port;
	int ret = -1;
	
	SWFEXT_FUNC_BEGIN(handle);

	port = Swfext_GetNumber();
	//printf("[%s][%d] -- port: %d\n", __func__, __LINE__, port);
	ret = getIP(port, ipAddr);
	if(ret < 0){
		printf("[%s][%d] -- Get IP error!!!\n", __func__, __LINE__);
		sprintf(ipAddr, "error");
	}
	Swfext_PutString(ipAddr);

	SWFEXT_FUNC_END();
}
static int sysinfo_get_miracode(void *handle){
	int val = 0, storage = 0;
	char *wlan0_ip=NULL;
	char *p0=NULL,*p1=NULL,*p2=NULL,*p3=NULL;
	int num0,num1,num2,num3;
	int tmp=0,ret=0;
	char str_tmp[25],str_totle[25],str_ip[16];
	unsigned long int sum;
	
	SWFEXT_FUNC_BEGIN(handle);
	wlan0_ip = Swfext_GetString();
	memset(str_totle,0,sizeof(str_totle));
	memset(str_ip,0,sizeof(str_ip));
	printf("[%s][%d] -- get_miracode!!! wlan0_ip=%s\n", __func__, __LINE__,wlan0_ip);
	

	if(wlan0_ip==NULL)
	{
		printf("[%s][%d]",__func__, __LINE__);
		sprintf(str_totle, "error");
		goto end_getmiracode;
	}
	if(strcmp(wlan0_ip,"error")==0)
	{
		sleep(2);
		ret=getIP(0, str_ip);
		if(ret<0)
		{
			sprintf(str_totle, "error");
			goto end_getmiracode;
		}
		else
			wlan0_ip=str_ip;
	}
	printf("[%s][%d] -- get_miracode!!! wlan0_ip=%s\n", __func__, __LINE__,wlan0_ip);
	
	p3=strtok(wlan0_ip,".");
	if(p3==NULL)
	{
		sprintf(str_totle, "error");
		goto end_getmiracode;
	}
	p2=strtok(NULL,".");
	if(p2==NULL)
	{
		sprintf(str_totle, "error");
		goto end_getmiracode;
	}
	p1=strtok(NULL,".");
	if(p1==NULL)
	{
		sprintf(str_totle, "error");
		goto end_getmiracode;
	}
	p0=strtok(NULL,".");
	if(p0==NULL)
	{
		sprintf(str_totle, "error");
		goto end_getmiracode;
	}
	printf("%s%s%s%s\n",p3,p2,p1,p0); 

	num0= atoi (p3);
	num1= atoi (p2);
	num2= atoi (p1);
	num3= atoi (p0);
	sum=num3*256*256*256+num2*256*256+num1*256+num0;
	printf("---sum=%uld\n",sum); 
	int i = 0;
	for(i=0; i<7; i++){
		tmp=sum%26;
			//printf("----111----%d\n",tmp); 
		tmp=toascii(tmp+97);
		sprintf(str_tmp,"%c",tmp);
		strcat(str_totle,str_tmp);
		printf("----++++----%c\n",tmp); 
		sum /= 26;
	}
	printf("----str_totle----%s\n",str_totle); 
	end_getmiracode:
	Swfext_PutString(str_totle);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_miracode_enable(void *handle){
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	ret=ezCastGetNumConfig(CONFNAME_MIRACODE);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_24g_or_5g(void *handle){
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	#if (EZMUSIC_ENABLE&&GPIO_WIFI_ON_LED)||( MODULE_CONFIG_EZCASTPRO_MODE==8075)
	ret=wpa24g_connected;
	#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
/*api for loopback play audio 
/*api for loopback play audio once usb or sdcard in  if Dongle just boot  not do it */
int bootuptime_sysyinfo()     //0 port 0,1port 1
{
	FILE *fp = NULL;
	int ret=0;
	int sec=-1;
	int i=0;
	system("cat /proc/uptime | awk '{print $1}'>/tmp/uptimedata"); 
	fp = fopen("/tmp/uptimedata", "r");
	if(fp != NULL){
		char tmp[32]="";
		char up_sec[32]="";
		ret = fread(tmp, 1, 31, fp);
		
		fclose(fp);
		unlink("/tmp/uptimedata");
		int len=strlen(tmp);
		for( i=0;i<len;i++)
		{
			if(tmp[i]=='.')
			{
				len=i+1;
				
				break;
			}
		}
		memcpy(up_sec,tmp,len-1);
		up_sec[len]='\0';
		printf("sys_uptime_sec=%s",up_sec);
		sec=atoi(up_sec);
		return sec;
	}
	return sec;

}
int boot_complete_status()//0 booting,1 complete
{	
	int time_sec=bootuptime_sysyinfo();
	if(time_sec>15)
		return 1;
	else
		return 0;


}
int old_ez_play_sta=1;
#define AE_STOPLOOP 8
#define AE_RESUMELOOP 9
static int audio_check_status()
{	
	int Audio_total=0;
	//int ez_play_sta=ezPlayerOnIdle();
	Audio_total=ezcast_fui_get_total();
	
	int audio_playstat=ezcast_ae_get_state();
	/*
	//printf("Wifi SubDisplay Media State[%d]\n",ez_play_sta);
	if((ez_play_sta==1)&& (old_ez_play_sta==0))//wifi idle
	{
		
		audio_playstat=AE_RESUMELOOP;   //wifi play to idle, stop wifi paly,resume audio_play
		old_ez_play_sta=1;
	}
	else if((ez_play_sta==0) && (old_ez_play_sta==1) )
	{
		audio_playstat=AE_STOPLOOP;  //wifi start playing ，stop audioloopbackplay
		old_ez_play_sta=0;
		

	}
	*/
	//printf("--check audio play status[%d]<audio loopbackplay mode>--\n",audio_playstat);
	switch(audio_playstat)
	{
		case AE_STOP:
		case AE_IDLE:
			
			if(cur_index==(Audio_total-1))
			{
				cur_index=0;
				printf("Audio List reach end,turn to the first audio!!!\n");
			}
			else
			{
				cur_index++;
				printf("Turn to next audio!!!\n");

			}
			audio_play(cur_index);
			break;
		case AE_ERROR:
			ezcast_ae_stop();
			ezcast_ae_close();
			ezcast_ae_open();
			printf("AE_ERROR,restart audio play!!!\n");
			if(cur_index==(Audio_total-1))
			{
				cur_index=0;
				printf("Audio List reach end,turn to the first audio!!!\n");
			}
			else
			{
				cur_index++;
				audio_play(cur_index);
				printf("Turn to next audio!!!\n");

			}
			break;
		case AE_RESUMELOOP:
			ezcast_ae_open();
			printf("restart audio loopback play!!!\n");
			audio_play(cur_index);	
			break;
		case AE_STOPLOOP:
			printf("Wifi  Vido play,stop audio !!!\n");
			ezcast_ae_stop();
			ezcast_ae_close();
			
			break;
		default:
			break;

	}
	return 0;

}

static int  loopbackstatus_timer_start()
{
	struct itimerval audioloopback_tim;
	printf("[%s][%d] -- start a 1s timer for check audio status :\n", __func__, __LINE__);
	signal(SIGALRM,audio_check_status);
	
	audioloopback_tim.it_value.tv_sec=1;
	audioloopback_tim.it_value.tv_usec=0;
	audioloopback_tim.it_interval.tv_usec=0;
	audioloopback_tim.it_interval.tv_sec=1;  //every 1 sec if  =0   just  one time
	
	int ret=setitimer(ITIMER_REAL,&audioloopback_tim,NULL);
	if(ret!=0)
	{
		printf("set timer error");
	}
	return 0;


}
int audio_play(int index)
{

	int Audio_total=0,i=0;
	char filepath[128];
	
	char amplay_index[8]="0";
	FILE *fp;
	cur_index=index;
	Audio_total=ezcast_fui_get_total();
	ezcast_fui_get_filepath(cur_index,filepath);
	int audioOK=ezcast_ae_set_file(filepath);
	printf("[%s,%d]Set Audio Play File[%d]--%s\n",__func__,__LINE__,cur_index,filepath);
	if(audioOK==0)
	{
		ezcast_ae_play();
		
	}
	else
	{
	//change file
		
		printf("Unsupproted Audio Format  turn to Next!!!\n");
		cur_index=cur_index+1;
		if(cur_index>=Audio_total)
		{
			printf("Unsupproted Audio Format  no file for play [exit]!!!\n");
			audioloopbackplay_stop();
		}
		else
		audio_play(cur_index);
		
	}
	sprintf(amplay_index,"%d",cur_index);
	//printf("up_usbcard_loopbackplay_status=%d\n",up_usbcard_loopbackplay_status);
	if(up_usbcard_loopbackplay_status==1)
	{
		//printf("create index filein usb \n");
		fp=fopen("/mnt/user1/thttpd/html/airusb/usb/ezcast_amplay.index","w+");
		if(fp!=NULL)
		{
			//printf("write index filein usb \n");
			fputs(amplay_index,fp);
			fputs("\r\n",fp);
			fclose(fp);
		}
	}
	else if(up_usbcard_loopbackplay_status==2)
	{
		printf("create index filein sdcard \n");
		fp=fopen("/mnt/user1/thttpd/html/airusb/sdcard/ezcast_amplay.index","w+");
		if(fp!=NULL)
		{
		printf("create index filein sdcasrd \n");
			fputs(amplay_index,fp);
			fputs("\r\n",fp);
			fclose(fp);
		}


	}
	
	return 0;		
}
static void * audio_loopback(char *directroy)
{
	char filepath[128];
	char tmp[8],index[8]="";
	int Audio_total=0,i=1,playindex=0;
	FILE *fp;
	int exist=-1;
	pthread_detach(pthread_self());
	ezcast_ae_open();
	//printf("read index file  \n");
	if(up_usbcard_loopbackplay_status==1)
	{
		 exist=access("/mnt/user1/thttpd/html/airusb/usb/ezcast_amplay.index",0);
		if(exist==0)
		{
			//printf(" file EXIST  \n");
			fp=fopen("/mnt/user1/thttpd/html/airusb/usb/ezcast_amplay.index","r");
			if(fp!=NULL)
			{
					fgets(tmp,8,fp);
					char *end = strchr(tmp, '\n');
				   	if(end != NULL){
					   char *end_r = strchr(tmp, '\r');
					   	if(end_r != NULL){
							   end = (end < end_r)?end:end_r;
							   int len = end - tmp;
								if(len>0&&len<8)
								strncpy(index,tmp,len);
								index[len]='\0';
					   }
					
				   	}

					//printf("index=%s\n",index);
					playindex=atoi(index);
					memset(tmp,0,8);
					printf("/mnt/user1/thttpd/html/airusb/usb/ezcast_amplay.index[%d]",playindex);
				fclose(fp);
			}
		}
		
	}
	else if(up_usbcard_loopbackplay_status==2)
	{
		 exist=access("/mnt/user1/thttpd/html/airusb/sdcard/ezcast_amplay.index",0);
		if(exist==0)
		{
			fp=fopen("/mnt/user1/thttpd/html/airusb/sdcard/ezcast_amplay.index","r");
			if(fp!=NULL)
			{
					fgets(tmp,8,fp);
					char *end = strchr(tmp, '\n');
				   	if(end != NULL){
					   char *end_r = strchr(tmp, '\r');
					   	if(end_r != NULL){
							   end = (end < end_r)?end:end_r;
							   int len = end - tmp;
								if(len>0&&len<8)
								strncpy(index,tmp,len);
								index[len]='\0';
							 }
					
				   	}
					
					playindex=atoi(index);
					printf("/mnt/user1/thttpd/html/airusb/sdcard/ezcast_amplay.index[%d]",playindex);
					memset(tmp,0,8);
				fclose(fp);
			}
		}

	}
	else
		playindex=0;
		
	audio_play(playindex);
	//printf("play_index=%d",playindex);
	sleep(1);
	loopbackstatus_timer_start();
	return 0;

}

int audioloopbackplay_stop()
{
	struct itimerval audioloopback_tim;
	
	printf("stop audioloopbackplay!!!\n");
	up_usbcard_loopbackplay_status=0;
	signal(SIGALRM,NULL);
	audioloopback_tim.it_value.tv_sec=0;
	audioloopback_tim.it_value.tv_usec=0;
	audioloopback_tim.it_interval.tv_usec=0;
	audioloopback_tim.it_interval.tv_sec=0;
	int ret=setitimer(ITIMER_REAL,&audioloopback_tim,NULL);
	if(ret!=0)
	{
		printf("set timer error");
	}
	sleep(1);
	ezcast_ae_stop();
	ezcast_ae_close();	
	return 0;
}
int audio_loopback_card_usb(int flag)//0 card ,1 usb;
{
	pthread_t tid;
	int test_cfgfile=-1;
	char file_rootdir[64]="";
	char card_rootdir[64]="/mnt/user1/thttpd/html/airusb/sdcard/";
	char usb_rootdir[64] ="/mnt/user1/thttpd/html/airusb/usb/";
	char Audiotype[64]="MP3 WMA OGG WAV AAC FLAC COOK APE AC3 DTS RA AMR M4A";
	//printf("up_usbcard_loopback_status=%d\n",up_usbcard_loopbackplay_status);
	if((flag==0)&&(up_usbcard_loopbackplay_status==0))
	{
		
		strcpy(file_rootdir,card_rootdir);
		//ezcast_fui_release_path();
		ezcast_fui_set_path(card_rootdir,Audiotype);
		int audio_total=ezcast_fui_get_total();
		test_cfgfile=access("/mnt/user1/thttpd/html/airusb/usb/ezcast_test.conf",0);
		printf("audiofile_total=[%d] in usb test config file exist:[%d]\n",audio_total,test_cfgfile);
		if(audio_total<1 ||(test_cfgfile==0 ))
		{	
			up_usbcard_loopbackplay_status=0;
			printf("Sdcard plugin  Prepare for musci loopbackplay fail, no audio file\n");
			return -1;
		}
		else
		{
			up_usbcard_loopbackplay_status=2;
			//printf("up_usbcard_loopbackplay_status=%d\n",up_usbcard_loopbackplay_status);
		}
		//ezcast_fui_get_filepath(1,filepath);
		//printf("filepath=%s",filepath);
	}
	else if((flag==1)&&(up_usbcard_loopbackplay_status==0))
	{
		strcpy(file_rootdir,usb_rootdir);
		ezcast_fui_set_path(usb_rootdir,Audiotype);
		int audio_total=ezcast_fui_get_total();
		printf("audiofile_total=%d  in usb\n",audio_total);
		if(audio_total<1)
		{	
			up_usbcard_loopbackplay_status=0;
			printf("Usb plugin  Prepare for musci loopbackplay fail , no audio file\n");
			return -1;
		}
		else
		{
			up_usbcard_loopbackplay_status=1;
			//printf("up_usbcard_loopbackplay_status=%d\n",up_usbcard_loopbackplay_status);
		}
		
	}
	else
		return -1;
	printf("[%s,%d]audioloopback-- filerootpath:%s\n",__func__, __LINE__,file_rootdir);
	int rtn = pthread_create(&tid, NULL,audio_loopback,file_rootdir);
	if(rtn){
		printf("[%s][%d] -- Thread Create Error :\n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

enum{
	INC_EZCAST_ENABLE = 0,
	INC_EZCAST_LITE_ENABLE,
	INC_EZWILAN_ENABLE,
	INC_AUTOPLAY_SET_ENABLE,
	INC_TEST_ENABLE,
	INC_EZCAST5G_ENABLE,
	INC_ROUTER_ONLY_ENABLE,
	INC_EZWIRE_ENABLE,
	INC_EZWIRE_TYPE,
	INC_LAN_ONLY,
	INC_EZWIRE_CAPTURE,
	INC_EZWIRE_ANDROID_ENABLE,
	INC_IS_SNOR_FLASH,
	INC_PNP_USE_WIFIDISPLAY,
	INC_CHIP_TYPE
};
int sysinfo_getIncludeNumberValue(void *handle){
	int indexName = -1;
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	indexName = Swfext_GetNumber();
	switch(indexName){
		case INC_EZCAST_ENABLE:
			#ifdef EZCAST_ENABLE
				ret = EZCAST_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZCAST_LITE_ENABLE:
			#ifdef EZCAST_LITE_ENABLE
				ret = EZCAST_LITE_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZWILAN_ENABLE:
			#ifdef EZWILAN_ENABLE
				ret = EZWILAN_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_AUTOPLAY_SET_ENABLE:
			#ifdef AUTOPLAY_SET_ENABLE
				ret = AUTOPLAY_SET_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_TEST_ENABLE:
			#ifdef TEST_ENABLE
				ret = TEST_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZCAST5G_ENABLE:
			#ifdef EZCAST5G_ENABLE
				ret = EZCAST5G_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_ROUTER_ONLY_ENABLE:
			#ifdef ROUTER_ONLY_ENABLE
				ret = ROUTER_ONLY_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZWIRE_ENABLE:
			#ifdef EZWIRE_ENABLE
				ret = EZWIRE_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZWIRE_TYPE:
			#ifdef EZWIRE_TYPE
				ret = EZWIRE_TYPE;
			#else
				ret = 0;
			#endif
			break;
		case INC_LAN_ONLY:
			#ifdef LAN_ONLY
				ret = LAN_ONLY;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZWIRE_CAPTURE:
			#ifdef EZWIRE_CAPTURE_ENABLE
				ret = EZWIRE_CAPTURE_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_EZWIRE_ANDROID_ENABLE:
			#ifdef EZWIRE_ANDROID_ENABLE
				ret = EZWIRE_ANDROID_ENABLE;
			#else
				ret = 0;
			#endif
			break;
		case INC_IS_SNOR_FLASH:
			#ifdef IS_SNOR_FLASH
				ret = IS_SNOR_FLASH;
			#else
				ret = 0;
			#endif
			break;
		case INC_PNP_USE_WIFIDISPLAY:
			#ifdef PNP_USE_WIFIDISPLAY
				ret = 1;
			#else
				ret = 0;
			#endif
			break;
		case INC_CHIP_TYPE:
			#ifdef MODULE_CONFIG_CHIP_TYPE
				ret = MODULE_CONFIG_CHIP_TYPE;
			#else
				ret = 0;
			#endif
			break;
		default:
			ret = 0;
			break;
	}
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
#if 0
static int sysinfo_set_standby_time(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetNumber();
	ezCastSetNumConfig(CONFNAME_STANDBY, val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}
static int sysinfo_get_standby_time(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = ezCastGetNumConfig(CONFNAME_STANDBY);
	Swfext_PutNumber(val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}

static int sysinfo_set_neighbour(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetNumber();
	ezCastSetNumConfig(CONFNAME_NEIGHBOUR, !!val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}
static int sysinfo_get_neighbour(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = ezCastGetNumConfig(CONFNAME_NEIGHBOUR);
	Swfext_PutNumber(val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}
#endif
static int sysinfo_set_config(void *handle)
{
	int val;
	char *name = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	name = Swfext_GetString();
	val = Swfext_GetNumber();
	ezCastSetNumConfig(name, val);
	if(strcmp(name, CONFNAME_PSKHIDE) == 0){
		ezcastSchemaSetPskHide(val);
	}
	//Check_music_status();
	SWFEXT_FUNC_END();
}
static int sysinfo_get_config(void *handle)
{
	int val;
	char *name = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	name = Swfext_GetString();
	val = ezCastGetNumConfig(name);
	Swfext_PutNumber(val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}

static int sysinfo_get_wifidisplay_status(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = ezCustomerOnAirplay()||ezCustomerOnEzdisplay()||ezCustomerOnDlna();
	Swfext_PutNumber(val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}

static int sysinfo_set_ezair_mode(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetNumber();
	//ezConfigSetAirplayMirrorOnly(val);
	ezCastSetNumConfig(CONFNAME_EZAIRMODE, val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}

static int sysinfo_get_ezair_mode(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val = ezCastGetNumConfig(CONFNAME_EZAIRMODE);
	Swfext_PutNumber(val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}


#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
//for WiFi Enterprise
static int wifi_full_802XEPA_conf(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int ret=0;
	FILE *fp = NULL;
	char buf[1024];
	EAP_method=Swfext_GetNumber();
	//clear file 
	printf("full 802.1xEAP conf____eap_method[%d] \n",EAP_method);
	fp=fopen(temp_filename,"w");
	fclose(fp);
	if(EAP_method==3){//TLS mode
		if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0)
            fp=fopen("/mnt/user1/clientap/wifi_template_tls_8192du.conf", "r");
		else
            fp=fopen("/mnt/user1/clientap/wifi_template_tls.conf", "r");
	}else if(EAP_method==4){//PEAP-MS mode
		if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0)
            fp=fopen("/mnt/user1/clientap/wifi_template_peapms_8192du.conf", "r");
		else
            fp=fopen("/mnt/user1/clientap/wifi_template_peapms.conf", "r");
	}else{//PEAP
		if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0)
            fp=fopen("/mnt/user1/clientap/wifi_template_peap_8192du.conf", "r");
		else
            fp=fopen("/mnt/user1/clientap/wifi_template_peap.conf", "r");
	}
    if(fp!=NULL){
        fgets(buf, sizeof(buf), fp);
        fclose(fp);
    }
//	puts(buf);
	fp = fopen(temp_filename,"wb+");
	if(fp!=NULL){
		fputs(buf,fp);
	}else{ret=1;}
	fclose(fp);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
int eap_modify_line(char * buf, char * key, char * value)
{	
 	char tmp[4096] = {0};
	char tmp2[256] = {0};
	char * locate1 = NULL;
	int key_len=0;
	key_len=strlen(key);
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	locate1 = strstr(buf,key);
	if(locate1==NULL)return 1;
	///printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	if(locate2==NULL)return 1;
	//memcpy(old_channel,(locate1+key_len),locate2-locate1-key_len);
	//printf("<rtl_hostapd_01.conf> old_channel=%s\n",old_channel);
	memcpy(tmp,buf,strlen(buf)-strlen(locate1));
	//printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	if(strcmp(key,"ssid=")==0){
		sprintf(tmp2,"ssid=%s",value);
	}
	else if(strcmp(key,"identity=")==0){
		sprintf(tmp2,"identity=%s",value);  
	}
	else if(strcmp(key,"password=")==0){
		sprintf(tmp2,"password=%s",value);  
	}
	else if(strcmp(key,"eap=")==0){
		sprintf(tmp2,"eap=%s",value);  
	}
	else if(strcmp(key,"private_key_passwd=")==0){
		sprintf(tmp2,"private_key_passwd=%s",value);  
	}
	if(tmp2==NULL)
		return -1;
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	int len1 = strlen(buf)-strlen(locate1);

	int len2 = strlen(tmp2);
	
	memcpy(tmp+len1,tmp2,len2);
	memcpy(tmp+len1+len2,locate2,strlen(locate2));
	
	memset(buf,0,strlen(buf));
	memcpy(buf,tmp,strlen(tmp));
	return 0;
}

static int wifi_set_802XEPA_conf(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	int re=1;
	char val[256];
	char *conf_val=Swfext_GetString();
	int flag= Swfext_GetNumber();
	printf("--/mnt/vram/wifi/*.conf--802.1XEAP filepath:[%s] flag:[%d]val:[%s]\n",temp_filename,flag,conf_val);
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	fp = fopen(temp_filename,"r");
	if(fp == NULL){
				
		re=0;
		printf("open tmp 802.1 config file error!!!\n");
		ret=fclose(fp);
	}else{
		ret=fread(buf, 1, 4096, fp);
		ret=fclose(fp);
		switch(flag){
			case 1:
				sprintf(val,"\"%s\"",conf_val);
				eap_modify_line(buf,"ssid=",val);
				break;
			case 2:
				sprintf(val,"\"%s\"",conf_val);
				eap_modify_line(buf,"password=",val);
				break;
			case 3:
				eap_modify_line(buf,"eap=",conf_val);
				char cmd[48];
				sprintf(cmd,"cat %s",temp_filename);
				system(cmd);
				break;
			case 4:
				sprintf(val,"\"%s\"",conf_val);
				eap_modify_line(buf,"identity=",val);
				char cmd1[48];
				sprintf(cmd1,"cat %s",temp_filename);
				system(cmd1);
				break;
			case 5:
				sprintf(val,"\"%s\"",conf_val);			
				eap_modify_line(buf,"private_key_passwd=",val);
				break;
		}
		if(buf!=NULL){
			//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
			fp = fopen(temp_filename,"wb+");
			if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
				re=1;
				return -1;
			}else
				re=0;
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
	}
	if(flag==4){//support hidden SSID for all condtions
		memset(buf,0,sizeof(buf));
		sprintf(buf,"sed -i -e '/}/i\scan_ssid=1' %s",temp_filename);	
		system(buf);
		memset(buf,0,sizeof(buf));
		sprintf(buf,"cat %s",temp_filename);
		system(buf);
	}	
	Swfext_PutNumber(re);
	SWFEXT_FUNC_END();
}
#endif

#endif   //#endif EZCAST_ENABLE

#if WEBSETTING_ENABLE

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
//for WiFi Enterprise_WEBSETTING
extern int ezcast_wifi_full_802XEPA_conf(int mode)
{
	int ret=0;
	FILE *fp = NULL;
	char buf[1024]={0};
	EAP_method=mode;
	//clear file 
	printf("ezcast_wifi_full_802XEPA_conf____eap_method[%d] \n",EAP_method);
	//fp=fopen(temp_filename,"w");
	//fclose(fp);

    if(EAP_method==3){//TLS mode
        if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0)
            fp=fopen("/mnt/user1/clientap/wifi_template_tls_8192du.conf", "r");
        else
            fp=fopen("/mnt/user1/clientap/wifi_template_tls.conf", "r");
    }else if(EAP_method==4){//PEAP-MS mode
        if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0)
            fp=fopen("/mnt/user1/clientap/wifi_template_peapms_8192du.conf", "r");
        else
            fp=fopen("/mnt/user1/clientap/wifi_template_peapms.conf", "r");
    }else{//PEAP
        if(access("/sys/module/8192du",F_OK)==0||access("/sys/module/8192cu",F_OK)==0||access("/sys/modules/8188eus",F_OK)==0)
            fp=fopen("/mnt/user1/clientap/wifi_template_peap_8192du.conf", "r");
        else
            fp=fopen("/mnt/user1/clientap/wifi_template_peap.conf", "r");
    }
    if(fp!=NULL){
        fread(buf, sizeof(buf), 1, fp);
        fclose(fp);
    }
//	puts(buf);

	fp = fopen(temp_filename,"wb+");
	if(fp!=NULL){
		fputs(buf,fp);
	}else{ret=1;}
	fclose(fp);
	printf("full ezcast_wifi_full_802XEPA_conf.1xEAP conf____ %d\n",ret);

	return ret;
}

int ezcast_eap_modify_line(char * buf, char * key, char * value)
{	
 	char tmp[4096] = {0};
	char tmp2[256] = {0};
	char * locate1 = NULL;
	int key_len=0;
	key_len=strlen(key);
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	locate1 = strstr(buf,key);
	if(locate1==NULL)return 1;
	///printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	char * locate2 = NULL;
	locate2 = strstr(locate1,"\n");
	if(locate2==NULL)return 1;
	//memcpy(old_channel,(locate1+key_len),locate2-locate1-key_len);
	//printf("<rtl_hostapd_01.conf> old_channel=%s\n",old_channel);
	memcpy(tmp,buf,strlen(buf)-strlen(locate1));
	//printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	if(strcmp(key,"ssid=")==0){
		sprintf(tmp2,"ssid=%s",value);
	}
	else if(strcmp(key,"identity=")==0){
		sprintf(tmp2,"identity=%s",value);  
	}
	else if(strcmp(key,"password=")==0){
		sprintf(tmp2,"password=%s",value);  
	}
	else if(strcmp(key,"eap=")==0){
		sprintf(tmp2,"eap=%s",value);  
		printf("eap=:%s line:%d line---:\n",tmp2,__LINE__);
	}
	else if(strcmp(key,"private_key_passwd=")==0){
		sprintf(tmp2,"private_key_passwd=%s",value);  
	}
	if(tmp2==NULL)
		return -1;
	printf("function:%s line:%d errno:\n",__FUNCTION__,__LINE__);
	int len1 = strlen(buf)-strlen(locate1);

	int len2 = strlen(tmp2);
	
	memcpy(tmp+len1,tmp2,len2);
	memcpy(tmp+len1+len2,locate2,strlen(locate2));
	
	memset(buf,0,strlen(buf));
	memcpy(buf,tmp,strlen(tmp));
	printf("buf=:%s line:%d line---:\n",buf,__LINE__);

	return 0;
}

//				ezcast_wifi_set_802XEPA_conf("TLS",3);
extern int ezcast_wifi_set_802XEPA_conf(char *value, int val_flag)
{
	
	int re=1;
	char val[256];
	char *conf_val=value;
	int flag= val_flag;
	printf("--/mnt/vram/wifi/*.conf--802.1XEAP filepath:[%s] flag:[%d]val:[%s]\n",temp_filename,flag,conf_val);
	FILE *fp = NULL;
	char buf[4096] ={0};
	char callbuf[128];
	int ret=-1;
	fp = fopen(temp_filename,"r");
	if(fp == NULL){
				
		re=0;
		printf("open tmp 802.1 config file error!!!\n");
		ret=fclose(fp);
	}else{
		ret=fread(buf, 1, 4096, fp);
		ret=fclose(fp);
		switch(flag){
			case 1:
				sprintf(val,"\"%s\"",conf_val);
				ezcast_eap_modify_line(buf,"ssid=",val);
				break;
			case 2:
				sprintf(val,"\"%s\"",conf_val);
				ezcast_eap_modify_line(buf,"password=",val);
				break;
			case 3:
				ezcast_eap_modify_line(buf,"eap=",conf_val);
				char cmd[48];
				sprintf(cmd,"cat %s",temp_filename);
				system(cmd);
				break;
			case 4:
				sprintf(val,"\"%s\"",conf_val);
				ezcast_eap_modify_line(buf,"identity=",val);
				char cmd1[48];
				sprintf(cmd1,"cat %s",temp_filename);
				system(cmd1);
				break;
			case 5:
				sprintf(val,"\"%s\"",conf_val);			
				ezcast_eap_modify_line(buf,"private_key_passwd=",val);
				break;
		}
		if(buf!=NULL){
			//fprintf(stderr,"function:%s line:%d\n",__FUNCTION__,__LINE__);
			fp = fopen(temp_filename,"wb+");
			if(fp == NULL){
				fprintf(stderr,"function:%s line:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
				re=1;
				return -1;
			}else
				re=0;
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
	}
	if(flag==4){//support hidden SSID for all condtions
		memset(buf,0,sizeof(buf));
		sprintf(buf,"sed -i -e '/}/i\scan_ssid=1' %s",temp_filename);	
		system(buf);
		memset(buf,0,sizeof(buf));
		sprintf(buf,"cat %s",temp_filename);
		system(buf);
	}
	return re;
}
#endif

#endif

#if AUTOPLAY_SET_ENABLE
int sysinfo_getAutoplayEnable(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ezCastGetAutoplayVal(SET_AUTOPLAY_ENABLE));
	SWFEXT_FUNC_END();
}
int sysinfo_setAutoplayEnable(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	int autoplayEnable = !!Swfext_GetNumber();
	ezCastSetAutoplayVal(SET_AUTOPLAY_ENABLE, autoplayEnable);
	SWFEXT_FUNC_END();
}
int sysinfo_getAutoplayHostAp(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ezCastGetAutoplayVal(SET_AUTOPLAY_HOST_AP));
	SWFEXT_FUNC_END();
}
int sysinfo_setAutoplayHostAp(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	int autoplayHostAp = !!Swfext_GetNumber();
	ezCastSetAutoplayVal(SET_AUTOPLAY_HOST_AP, autoplayHostAp);
	SWFEXT_FUNC_END();
}
int sysinfo_getAutoplayProgressive(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ezCastGetAutoplayVal(SET_AUTOPLAY_PROGRESSIVE));
	SWFEXT_FUNC_END();
}
int sysinfo_setAutoplayProgressive(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	int autoplayProgressive = !!Swfext_GetNumber();
	ezCastSetAutoplayVal(SET_AUTOPLAY_PROGRESSIVE, autoplayProgressive);
	SWFEXT_FUNC_END();
}
int sysinfo_getAutoplayPlaylist(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ezCastGetAutoplayVal(SET_AUTOPLAY_PLAYLIST));
	SWFEXT_FUNC_END();
}
int sysinfo_setAutoplayPlaylist(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	int autoplayPlaylist = !!Swfext_GetNumber();
	ezCastSetAutoplayVal(SET_AUTOPLAY_PLAYLIST, autoplayPlaylist);
	SWFEXT_FUNC_END();
}
int sysinfo_getAutoplayWaitime(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ezCastGetAutoplayVal(SET_AUTOPLAY_WAITIME));
	SWFEXT_FUNC_END();
}
int sysinfo_setAutoplayWaitime(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	int autoplayWaitime = Swfext_GetNumber();
	ezCastSetAutoplayVal(SET_AUTOPLAY_WAITIME, autoplayWaitime);
	SWFEXT_FUNC_END();
}
int sysinfo_getAutoplayVolume(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ezCastGetAutoplayVal(SET_AUTOPLAY_VOLUME));
	SWFEXT_FUNC_END();
}
int sysinfo_setAutoplayVolume(void *handle){
	SWFEXT_FUNC_BEGIN(handle);
	int autoplayVolume = Swfext_GetNumber();
	ezCastSetAutoplayVal(SET_AUTOPLAY_VOLUME, autoplayVolume);
	SWFEXT_FUNC_END();
}
#endif		// #if AUTOPLAY_SET_ENABLE

int sysinfo_check_ezmusic_flag(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int ezmusic_flag=0;
	#if EZMUSIC_ENABLE
	ezmusic_flag=1;
	#endif
	printf("[%s][%d] -- ezmusic_flag: %d\n", __func__, __LINE__, ezmusic_flag);
	Swfext_PutNumber(ezmusic_flag);
	
	SWFEXT_FUNC_END();
}
#if EZMUSIC_ENABLE
static int mainswf_skip_get_keycode(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(SysCGI_priv.mainswf_skip_get_keycode_flag);
	SWFEXT_FUNC_END();
}

static void Check_music_status(void)
{
	int cur_status=ezcast_ae_get_state();
	printf("cur_status=====%d,%s,%d\n",cur_status,__FUNCTION__,__LINE__);
	if(((cur_status == AE_PLAY)||(cur_status ==AE_PAUSE)||(cur_status ==AE_ERROR)||(cur_status ==AE_FF)||(cur_status ==AE_FB))&&(1==SysCGI_priv.stream_play_status))
	{
		ezcast_ae_stop();
		ezcast_ae_close();
		printf("%s,%d\n",__FUNCTION__,__LINE__);
	}	
}
static int sysinfo_set_stream_play_status(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SysCGI_priv.stream_play_status=Swfext_GetNumber();
	//Check_music_status();
	SWFEXT_FUNC_END();
}
#endif
#if EZWILAN_ENABLE
enum{
	EZLAN_LED_BLUE = 0,
	EZLAN_LED_GREEN
};
static void setLED(int who, int val){
	switch(who){
		case EZLAN_LED_BLUE:		// WARN: The GPIO35 can not be set if function of MAC is used.
			EZCASTLOG("Set blue led %s!!!\n", (val==0)?"ON":"OFF");
			reg_writel(0xb01c0044, (reg_readl(0xb01c0044)&~(0x7<<13))|(0x4<<13));
			set_gpio(35, !!val);	
			break;
		case EZLAN_LED_GREEN:
			set_gpio(25, !!val);
			break;
		default:
			break;
	}
}
static int sysinfo_set_LED(void *handle)
{
	int who, val;
	SWFEXT_FUNC_BEGIN(handle);
	who = Swfext_GetNumber();
	val = Swfext_GetNumber();
	setLED(who, val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}
#endif
#if EZMUSIC_ENABLE  || (MODULE_CONFIG_EZCASTPRO_MODE==8075)
void EZ_URLJsonWrite(char *json_file, JSON *json_item)
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


void Write_MusicURL_TOJson(int idx)

{

		JSON *json_array,*json_url ,*json_item ;
		int i=0;
 		char show_time[30];
		char urlinfo_jsonfile[] ="/mnt/user1/thttpd/html/ezmusicinfo.json";//"/mnt/user1/ezdata/ezmusiclnfo.json";
		printf("%s urlinfo_jsonfile=%s\n",__func__,urlinfo_jsonfile);
		time_t now;
	       struct tm *gt;

		time(&now);
		gt = gmtime(&now);
	       sprintf(show_time,
		"%02d-%02d-%04d %02d:%02d:%02d", //expires=Thu, 01-Jan-1970 00:00:16 GMT;
		gt->tm_mday,
		gt->tm_mon,
		gt->tm_year + 1900, 	
		gt->tm_hour,
		gt->tm_min,
		gt->tm_sec
		);
			
		json_item = JSON_CreateObject();    
		if (json_item != NULL)
		{
			#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE == 8075)
			JSON_AddStringToObject(json_item, "channel_name", "AirDisk local multimedia files");
			#else
			JSON_AddStringToObject(json_item, "channel_name", "EZmusic Local music file");
			#endif
			JSON_AddNumberToObject(json_item, "current_page", 1);
			JSON_AddStringToObject(json_item, "function", "index");
			JSON_AddNumberToObject(json_item, "item_per_page", idx);
			JSON_AddNumberToObject(json_item, "next_page", 1);
			JSON_AddNumberToObject(json_item, "query_time", 60);
           		JSON_AddItemToObject(json_item ,"playlist",json_array=JSON_CreateArray());
			//if (json_url != NULL)
			{
				const char nil[] = "(x)";
				char *json_string;
				for(i=0;i<idx;i++)
	                     {
					 json_string =all_url_info[i].url;
					 //printf("Write_MusicURL_TOJson json_string=%s\n",json_string);
					 json_url=JSON_CreateObject();
					 JSON_AddStringToObject(json_url, "url", json_string? json_string : nil);
					 JSON_AddItemToArray(json_array, json_url);
		               }
			}
		}
		JSON_AddTrueToObject(json_item, "repeat_all");
		JSON_AddTrueToObject(json_item, "status");
		JSON_AddNumberToObject(json_item, "total_item", idx);
		JSON_AddStringToObject(json_item, "update", show_time);
		EZ_URLJsonWrite(urlinfo_jsonfile, json_item);
		JSON_Delete(json_item);
			
	
}
void Write_default_info_TOJson()
{
	 JSON *json_item ;
	 char ret=-1;
	 char urlinfo_jsonfile[] ="/mnt/user1/thttpd/html/ezmusicinfo.json";//"/mnt/user1/ezdata/ezmusiclnfo.json";
	 printf("%s urlinfo_jsonfile=%s\n",__func__,urlinfo_jsonfile);		
	 json_item = JSON_CreateObject();    
	if (json_item != NULL)
	{
			//JSON_AddFalseToObject	
		JSON_AddFalseToObject(json_item, "status");
		JSON_AddNumberToObject(json_item, "error", ret);
		EZ_URLJsonWrite(urlinfo_jsonfile, json_item);
		JSON_Delete(json_item);
     
	} 

	
}

char *url_encode(char const *s, int len, int *new_length)
{
    register unsigned char c;
    unsigned char *to, *start;
    unsigned char const *from, *end;

    from = (unsigned char *)s;
    end  = (unsigned char *)s + len;
    start = to = (unsigned char *) calloc(1, 3*len+1);

    while (from < end)
    {
        c = *from++;

        if (c == ' ')
        {
            *to++ = '%';
			*to++ = '2';
			*to++ = '0';
        }else{
            *to++ = c;
        }
    }
    *to = 0;
    if (new_length)
    {
        *new_length = to - start;
    }
    return (char *) start;
}


void Write_MusicURL_Array(int num,char *str)
{
	int new_length;
	printf("^^^^^^enc^^^^ %s\n", url_encode(str, strlen(str), &new_length));
	snprintf(all_url_info[num].url, sizeof(all_url_info[num].url), "%s", url_encode(str, strlen(str), &new_length));
}
void ezChannel_SetPlayWay(idx)
{

	struct sysconf_param sys_info;
	
	_get_env_data(&sys_info);
	if(sys_info.play_way != idx){
		sys_info.play_way = idx;
		_save_env_data(&sys_info);
		_store_env_data();
	}
	printf(" sys_info.play_way =%d ------------------------\n",sys_info.play_way );


}
int  ezChannel_GetPlayWay()
{

	struct sysconf_param sys_info;
	
	_get_env_data(&sys_info);
	printf("sys_info.play_way =%d ------------------------\n",sys_info.play_way );
	return sys_info.play_way;

}
#endif
int get_am_type()
{

	#if MODULE_CONFIG_EZCAST_ENABLE
		#if MODULE_CONFIG_EZMUSIC_ENABLE
			return 1;
		#elif MODULE_CONFIG_EZWILAN_ENABLE
			return 3;
		#elif MODULE_CONFIG_EZCASTPRO_MODE
			return 4;
		#else
			return 2;
		#endif
	#endif
	return 0;

}
int reset_userpassword()
{
	int ret=0;
	ret=get_am_type();
	printf("reset_userpassword   ret=%d\n",ret);
	if(ret==3||ret==4)
		system("rm -f "USERINFO_JSONFILE);
		//Write_Password_TOJson(save_name,"670b14728ad9902aecba32e22fa4f6bd");
	
	return 0;	

}
static int  sysinfo_reset_userpassword(void *handle)
{
      int ret = 0;
      SWFEXT_FUNC_BEGIN(handle);
      reset_userpassword();
	if(access(EZCAST_CONF, F_OK) == 0){
	unlink(EZCAST_CONF);
	}
      Swfext_PutNumber(0);
      SWFEXT_FUNC_END();
}
///

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
static int sysinfo_get_passcode(void *handle)
{
	char *val;
	SWFEXT_FUNC_BEGIN(handle);
	val=ezConfigGetPasscode();
	Swfext_PutString(val);
	SWFEXT_FUNC_END();
}
static int sysinfo_set_passcode(void *handle)
{
	char *val=NULL;
	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetString();
	printf("set passcode: %s[%d]\n", val, strlen(val));
	ezConfigSetPasscode(val);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_passcode_onoff(void *handle)
{
	char val;
	SWFEXT_FUNC_BEGIN(handle);
	val=ezCastGetNumConfig(CONFNAME_PASSCODE);
	printf("sys_get_passcode_onoff val=%d\n",val);
	Swfext_PutNumber(val);
	//Check_music_status();
	SWFEXT_FUNC_END();
}

static int sysinfo_get_user_define_passcode(void *handle)
{
	int val;
	SWFEXT_FUNC_BEGIN(handle);
	val=ezCastGetNumConfig(CONFNAME_PASSCODE_VAL);
	printf("sysinfo_get_user_define_passcode val=%d\n",val);
	Swfext_PutNumber(val);
	SWFEXT_FUNC_END();
}

unsigned int cal_checksum(unsigned char *ptr, unsigned int len, unsigned int oldCheckSum)
{
	unsigned int CheckSum,i;
	unsigned int *D_ptr;
	D_ptr=(unsigned int *)ptr;
	CheckSum=oldCheckSum;

	for(i=0;i<len/sizeof(unsigned int);i++)
	{
		CheckSum += *D_ptr;
		D_ptr++;
	}

	if(len%sizeof(unsigned int) > 0)
	{
		CheckSum += *D_ptr;
	}
	
	return CheckSum;
}

unsigned int getCheckSum(char *path)
{
#define BUFSIZE	1024*16
	char buff[BUFSIZE];
	FILE *fp = NULL;
	unsigned int checksum = 0;
	int ret = -1;

	if(path == NULL)
	{
		EZCASTWARN("Path NULL\n");
		return 0;
	}

	if(access(path, F_OK) != 0)
	{
		EZCASTWARN("File[%s] is not exist!!!\n", path);
		return 0;
	}

	fp = fopen(path, "r");
	if(fp == NULL)
	{
		EZCASTWARN("Open %s fail\n", path);
		perror("ERROR");
		return 0;
	}

	for(;;)
	{
		memset(buff, 0, sizeof(buff));
		ret = fread(buff, 1, sizeof(buff), fp);
		if(ret <= 0)
			break;
		checksum = cal_checksum(buff, ret, checksum);
	}

__END__:
	if(fp != NULL)	fclose(fp);
	return checksum;
}

static int crcCheckSwfFileExist(char *cksum_path, char *swf_path,char *bak_path){
	unsigned int right_cksum=0;
	unsigned int real_cksum=0;
	FILE *fp = NULL;
	int ret = -1;
       char callbuf[256];
	if(cksum_path == NULL || swf_path == NULL){
		EZCASTWARN("cksum_path or swf_path is NULL\n");
		return -1;
	}
	
	if(access(swf_path, F_OK) != 0){
		EZCASTLOG("SWF[%s] is not exist\n", swf_path);
		EZCASTLOG("bak_path=SWF[%s] \n", bak_path);
		if(bak_path!=NULL&&access(bak_path, F_OK) == 0)
		{
		      EZCASTLOG("SWF[%s] is  exist\n", bak_path);
			snprintf(callbuf, sizeof(callbuf),"cp -a %s %s", bak_path,swf_path);
	      	 	system(callbuf);
			system("sync");
		}
		else
			return -1;
	}
	if(access(swf_path, F_OK) != 0){
		EZCASTLOG("SWF[%s] is not exist\n", swf_path);
		return -1;
	}
	if(access(cksum_path, F_OK) != 0){
		EZCASTLOG("CheckSum[%s] is not exist\n", cksum_path);
		return -1;
	}

	fp = fopen(cksum_path, "r");
	if(fp == NULL){
		EZCASTWARN("Open %s fail\n", cksum_path);
		perror("ERROR");
		return -1;
	}

	ret = fread((void *)&right_cksum, 1, sizeof(right_cksum), fp);
	fclose(fp);
	if(ret <= 0){
		EZCASTWARN("Read %s fail\n", cksum_path);
		perror("ERROR");
		return -1;
	}
	EZCASTLOG("right_cksum: %ud\n", right_cksum);
	real_cksum = getCheckSum(swf_path);
	EZCASTLOG("real_cksum: %ud\n", real_cksum);
	if(real_cksum==right_cksum)
		return 0;
	return -1;
}
#define SWF_PATH	"/am7x/case/data"
#define CKSUM_PATH	"/mnt/vram/swf_cksum"
#define CKSUM_FTYPE	"cksum"
static int checkCustomerSwfExist(char *name){
	char swf_path[256];
	char cksum_path[256];
	char bak_swfpath[256];

	if(name == NULL){
		EZCASTWARN("File name is NULL\n");
		return -1;
	}

	snprintf(swf_path, sizeof(swf_path), "%s/%s", SWF_PATH, name);
	snprintf(cksum_path, sizeof(swf_path), "%s/%s.%s", CKSUM_PATH, name, CKSUM_FTYPE);
	snprintf(bak_swfpath, sizeof(bak_swfpath), "%s/%s", CKSUM_PATH, name);
	return crcCheckSwfFileExist(cksum_path, swf_path,bak_swfpath);
}
static int sysinfo_customerswf_file_exist(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	char *name = Swfext_GetString();
	ret = checkCustomerSwfExist(name);
	printf("sysinfo_customerswf_file_exist ret=%d\n",ret);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
#endif

#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
void sysproBox_wifi_subdisplay_end(void)
{
	printf("[%s---%d]proBox_wifi_subdisplay_end",__FILE__, __LINE__);
	Write_default_info_TOJson();
	wifi_subdisplay_end();
}

#if 0
void copyEZLauncherJSON2Udisk()
{
	FILE *fp;  
	char buffer[20];  
	char *cmd_str="cat /proc/bus/usb/devices  | grep -o ProdID=a101"; 
	
	printf("[%s][%d] \n", __func__, __LINE__);
	memset(buffer,'\0',20);  
	printf("buffer=%s strlen=%d\n",buffer,strlen(buffer));  
	fp=popen(cmd_str,"r");  
	fread(buffer,20,1,fp);  
	printf("buffer=%s strlen=%d\n",buffer,strlen(buffer));  
	pclose(fp);   
	
	if(strlen(buffer) > 0){
		system("cp -f  /tmp/ezlauncher_config.json  /mnt/user1/thttpd/html/airusb/usb/");		
		system("rm -f /mnt/user1/thttpd/html/airusb/usb/.SETOK | touch /mnt/user1/thttpd/html/airusb/usb/.SETOK");//this file  indicates that SYNC is complete.
	}
}
#endif
void copy_ezlauncher_json_to_html_folder()
{
	FILE *fp;  
	char buffer[20];  
	char *cmd_str="cat /proc/bus/usb/devices  | grep -o ProdID=1234"; 
	
	printf("[%s][%d] \n", __func__, __LINE__);
	memset(buffer,'\0',20);  
	fp=popen(cmd_str,"r");  
	fread(buffer,20,1,fp);  
	printf("buffer=%s strlen=%d\n",buffer,strlen(buffer));  
	pclose(fp);   
	
	if(strlen(buffer) > 0){
		printf("EZLauncher V2 is found.\n"); 
		_inform_ezlauncher_in();
		//system("cp -f  /tmp/ezlauncher_config.json  /mnt/user1/thttpd/html/");		
	}
}

#endif

static int refresh_apinfo_for_ezlauncher(void *handle)
{
    SWFEXT_FUNC_BEGIN(handle);
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8075	
        save_apinfo_for_ezlauncher();
        printf("[%s][%d] \n", __func__, __LINE__);
#endif	
    SWFEXT_FUNC_END();
}

#if FUN_WIRE_ENABLE
int ezwireSocketFd = -1;
int ezwireStatus = 0;
int setEzwireStatus(int port, int dev, int status){
	/*port: 0-->usb0, 1-->usb1*/
	if(port < 0 || port > 1 || status < 0 || status > 0xF  || dev > 0xF){
		EZCASTWARN("status[%d] or port[%d] error\n", status, port);
		return -1;
	}

	if(status == EZWIRE_STATUS_DISCONNECT)
		dev = EZWIRE_DEV_UNKNOWN;

	//EZCASTLOG(">>>>>>> port: %d, dev: %d, status: %d\n", port, dev, status);
	if(dev >= 0){
		int new_stat = ((dev&0xF)<<(4+(port*8)))|((status&0xF)<<(port*8));
		if((ezwireStatus&(0xFF<<(port*8))) != new_stat){
			ezwireStatus &= ~(0xFF<<(port*8));
			ezwireStatus |= new_stat;
		}
	}else{
		int new_stat = ((status&0xF)<<(port*8));
		if((ezwireStatus&(0xF<<(port*8))) != new_stat){
			ezwireStatus &= ~(0xF<<(port*8));
			ezwireStatus |= new_stat;
		}
	}

	//EZCASTLOG(">>>>>>> ezwireStatus: 0x%x\n", ezwireStatus);
	return ezwireStatus;
}

static int getEzwireStatus(){
	return ezwireStatus;
}

static char *getEZWireIp4Mobile(char *buf, int len){
#define MAX_INTERFACES (10)
	struct ifreq ifr;
	struct sockaddr_in *sin = NULL;
	
	if(ezwireSocketFd < 0){
		ezwireSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
		if(ezwireSocketFd < 0){
			EZCASTWARN("Create socket fail!!\n");
			perror("ERROR");
			goto __ERR__;
		}
	}

__GET_IOS_IP__:
	strcpy(ifr.ifr_name, EZWIRE_IOS_PORT);
	if(ioctl(ezwireSocketFd,SIOCGIFADDR,&ifr)<0){
		goto __GET_ANDROID_IP__;
	}
	goto __END__;

__GET_ANDROID_IP__:
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s%d", EZWIRE_ANDROID_IF_PRE, android_if_port);
	if(ioctl(ezwireSocketFd,SIOCGIFADDR,&ifr)<0){
		goto __ERR__;
	}

__END__:
	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	strncpy(buf, (char *)inet_ntoa(sin->sin_addr), len);
	buf[len-1] = '\0';
	return buf;

__ERR__:
	return NULL;
}

static int checkUsbAudio(){
	static int count = 0;
	if(access(USB_AUDIO_START, F_OK) != 0){
		count = 0;
		return 0;
	}
	
	if(count >= 5 && access(USB_AUDIO_SUPPORT, F_OK) != 0){
		count = 0;
		system(AM_CASE_SC_DIR"/eth_usb.sh checkuac &");
		return 0;
	}
	
	count++;
	return 0;
}

int ezwireAddUsbPort(int num)
{
	android_if_port = num;
	EZCASTLOG("android net interface: usb%d\n", android_if_port);
}

static int ezwireGetConnectStatus(){
#define MAX_INTERFACES (10)
	struct ifreq ifr[ MAX_INTERFACES ], *ifr_p;
	struct ifconf ifc;
	FILE *fp = NULL;
	char dnsmasq_info[64];
	int i, n, ret;

	checkUsbAudio();
	
	if(ezwireSocketFd < 0){
		ezwireSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
		if(ezwireSocketFd < 0){
			EZCASTWARN("Create socket fail!!\n");
			perror("ERROR");
			goto __ERR__;
		}
	}
	memset(ifr, 0, sizeof(ifr));
	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_req = ifr;
	
	if (ioctl(ezwireSocketFd, SIOCGIFCONF, &ifc) == 0)
	{
		for (i = 0, n = ifc.ifc_len/sizeof(struct ifreq); i < n; i++)
		{
			ifr_p = &ifr[ i ];
			
			if ( ifr_p->ifr_addr.sa_family == AF_INET && strlen(ifr_p->ifr_name) > 0 )
			{
				char androidIf[16];
				snprintf(androidIf, sizeof(androidIf), "%s%d", EZWIRE_ANDROID_IF_PRE, android_if_port);
				if (strcasecmp(ifr_p->ifr_name, EZWIRE_IOS_PORT) == 0 || strcasecmp(ifr_p->ifr_name, androidIf) == 0)
				{
					//EZCASTLOG("ifr_p->ifr_name: %s\n", ifr_p->ifr_name);
					if (ioctl(ezwireSocketFd, SIOCGIFADDR, ifr_p) == 0)
					{
						if(strcasecmp(ifr_p->ifr_name, EZWIRE_IOS_PORT) == 0)
							setEzwireStatus(1, EZWIRE_DEV_IOS, EZWIRE_STATUS_IP_OK);
						else if(strcasecmp(ifr_p->ifr_name, androidIf) == 0)
							setEzwireStatus(1, EZWIRE_DEV_ANDROID, EZWIRE_STATUS_IP_OK);
						else
							setEzwireStatus(1, EZWIRE_DEV_IGNORE, EZWIRE_STATUS_IP_OK);
						
						if(access(EZWIRE_PAIR_FLAG_FILE, F_OK) == 0){
							unlink(EZWIRE_PAIR_FLAG_FILE);
						}
						goto __USB0_STATUS__;
					}
				}
			}
		}
	}else
	{
		EZCASTWARN("ioctl fail\n");
		perror("ERROR");
	}
	if(access(EZWIRE_PAIR_FLAG_FILE, F_OK) == 0){
		setEzwireStatus(1, EZWIRE_DEV_IOS, EZWIRE_STATUS_CONNECTED);
	}

__USB0_STATUS__:
#if !EZWIRE_USB_DEVICE_ENABLE	// Only EZWire for 8251 and 8252w support PC connect.
	if(isAM8251W())
#endif
	{
		if(access("/tmp/udhcpd.leases", F_OK) == 0){
			fp = fopen("/tmp/udhcpd.leases", "r");
			if(fp == NULL){
				EZCASTWARN("open /tmp/udhcpd.leases fail\n");
				perror("ERROR");
				goto __ERR__;
			}
			ret = fread(dnsmasq_info, 1, sizeof(dnsmasq_info), fp);
			fclose(fp);
			if(ret > 5){
				setEzwireStatus(0, EZWIRE_DEV_PC_MAC, EZWIRE_STATUS_IP_OK);
				goto __END__;
			}
		}
	}
__END__:
	return getEzwireStatus();
	
__ERR__:
	return -1;
}
static int sysinfo_get_ezwire_status(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	ret = ezwireGetConnectStatus();
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
#if WEBSETTING_ENABLE
int websetting_sysinfo_get_ezwire_status()
{	

	int  netlink_status;
	netlink_status=ezwireGetConnectStatus();
	int status_usb1 = (netlink_status & 0xF00)>>8;
	printf("netlink_status_get===============%d,%d\n",netlink_status,status_usb1);
	return status_usb1;
}


#endif
static int sysinfo_get_ezwire_ip_mobile(void *handle)
{
	char buf[16];
	SWFEXT_FUNC_BEGIN(handle);
	if(getEZWireIp4Mobile(buf, sizeof(buf)) == NULL)
		sprintf(buf, "null");

	Swfext_PutString(buf);
		
	SWFEXT_FUNC_END();
}

static int getMd5Sum(char *md5, const char *buf, int len){
	MD5_CTX c;
	unsigned char md[17];
	int i;

	if(md5 == NULL || buf == NULL){
		EZCASTWARN("md5 or buf is NULL\n");
		return -1;
	}

	EZCASTLOG("len: %d\n", len);
	MD5_Init(&c);
	MD5_Update(&c, buf, len);
	MD5_Final(md, &c);
	md5[0] = '\0';
	for(i=0; i<16; i++){
		sprintf(&md5[2*i], "%02x", md[i]&0xff);
	}
	md5[32] = '\0';

	EZCASTLOG("md5: %s\n", md5);

	return 0;
}
static int md5CheckSwfFileExist(char *md5_path, char *swf_path){
	char right_md5[33];
	char real_md5[33];
	char *buff = NULL;
	FILE *fp = NULL;
	int ret = -1, fileSize = 0;
	struct stat st;

	if(md5_path == NULL || swf_path == NULL){
		EZCASTWARN("md5_path or swf_path is NULL\n");
		return -1;
	}
	
	if(access(swf_path, F_OK) != 0){
		EZCASTLOG("SWF[%s] is not exist\n", swf_path);
		return -1;
	}
	if(access(md5_path, F_OK) != 0){
		EZCASTLOG("MD5[%s] is not exist\n", md5_path);
		return -1;
	}

	memset(right_md5, 0, sizeof(right_md5));
	memset(real_md5, 0, sizeof(real_md5));

	fp = fopen(md5_path, "r");
	if(fp == NULL){
		EZCASTWARN("Open %s fail\n", md5_path);
		perror("ERROR");
		return -1;
	}

	ret = fread(right_md5, 1, sizeof(right_md5), fp);
	fclose(fp);
	if(ret <= 0){
		EZCASTWARN("Read %s fail\n", md5_path);
		perror("ERROR");
		return -1;
	}
	right_md5[32] = '\0';
	EZCASTLOG("right_md5: %s\n", right_md5);

	ret = stat(swf_path, &st);
	if(ret != 0){
		EZCASTWARN("Get stat of %s fail\n", swf_path);
		perror("ERROR");
		return -1;
	}
	fileSize = st.st_size;
	
	fp = fopen(swf_path, "r");
	if(fp == NULL){
		EZCASTWARN("Open %s fail\n", swf_path);
		perror("ERROR");
		return -1;
	}

	buff = (char *)malloc(fileSize+1);
	if(buff == NULL){
		EZCASTWARN("malloc fail\n");
		perror("ERROR");
		fclose(fp);
		return -1;
	}

	ret = fread(buff, 1, fileSize+1, fp);
	fclose(fp);
	if(ret <= 0){
		EZCASTWARN("Read %s fail\n", swf_path);
		perror("ERROR");
		free(buff);
		return -1;
	}

	getMd5Sum(real_md5, buff, ret);
	free(buff);
	if(strcasecmp(real_md5, right_md5) == 0)
		return 0;
	return -1;
}
static int checkSwfFileExist(char *name){
	char swf_path[256];
	char md5_path[256];

	if(name == NULL){
		EZCASTWARN("File name is NULL\n");
		return -1;
	}

	snprintf(swf_path, sizeof(swf_path), "/am7x/case/data/%s.swf", name);
	snprintf(md5_path, sizeof(swf_path), "/am7x/case/data/%s.md5", name);
	return md5CheckSwfFileExist(md5_path, swf_path);
}
static int sysinfo_swf_file_exist(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	char *name = Swfext_GetString();
	ret = checkSwfFileExist(name);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
int aoa_disable(int val)
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
static int sysinfo_aoa_disable(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	int val = Swfext_GetNumber();
	ret = aoa_disable(val);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
static int sysinfo_isQCMode(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	ret = isQCMode();
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

int setWirePlugModeDefault()
{
	FILE *fp = NULL;
	char val[32];

	fp = fopen("/mnt/vram/ezcast/PLUGPLAYMODE", "w");
	if(fp != NULL)
	{
		snprintf(val, sizeof(val), "PlugMode=%s\n", (EZWIRE_PLUGPLAY_DEFAULT==1)?"YES":"NO");
		fwrite(val, 1, strlen(val), fp);
		fclose(fp);
		sync();
		EZCASTLOG("Set plug mode to default: %s\n", val);
		//if(isDefaultOpen != 0)
			//system_reboot();
	}
	return EZWIRE_PLUGPLAY_DEFAULT;
}

int isWirePlugMode()
{
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
}
static int sysinfo_getWirePlugMode(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
#if EZWIRE_CAPTURE_ENABLE
	ret = isWirePlugMode();
#else
	ret = 0;
#endif
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int sysinfo_getWireDdefaultMode(void *handle)
{
	int ret=0;
	FILE *fp = NULL;
	char callbuf[24]={0};
	SWFEXT_FUNC_BEGIN(handle);
	fp=fopen("/mnt/vram/ezcast/DEFAULTMODE","r");
	if(fp==NULL)
	{
		system("echo 2 > /mnt/vram/ezcast/DEFAULTMODE");
		system("sync");
		 ret=1;;
	}
	else
	{
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		ret=atoi(callbuf);
		if(ret>0)
			ret=ret-1;
	}
	
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int sysinfo_setWireDdefaultMode(void *handle)
{
	int ret=0,val=0,val_old=0;
	char callbuf[24]={0};
	FILE *fp = NULL;
	SWFEXT_FUNC_BEGIN(handle);
	val = Swfext_GetNumber();
	val=val+1;
	fp=fopen("/mnt/vram/ezcast/DEFAULTMODE","r");
	if(fp!=NULL)
	{
		memset(callbuf,0,sizeof(callbuf));
		ret = fread(callbuf, 1, sizeof(callbuf), fp);	
		fclose(fp);
		val_old=atoi(callbuf);
		if(val==val_old)
			return 0;
	}		
	if(val==1)
	{
		system("echo 1 > /mnt/vram/ezcast/DEFAULTMODE");
		system("sync");

	}
	else
	{
		system("echo 2 > /mnt/vram/ezcast/DEFAULTMODE");
		system("sync");
	}
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}
static int sysinfo_projectorWire_insmod(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("sysinfo_ezwire_insmod");
	create_websetting_server();
	#if 1
		system("insmod /lib/modules/2.6.27.29/am7x_hcd.ko");
		printf("do nothing");
	#else
		system("insmod /lib/modules/2.6.27.29/am7x_hcd_next.ko");
	#endif
	SWFEXT_FUNC_END();
}
static int sysinfo_projectorWire_rmmod(void *handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	printf("sysinfo_ezwire_rmmod");
	iphone_out();
#if 0
	#if 1 //use usb0 for wire
		system("rmmod am7x_hcd.ko");
	#else
		//system("rmmod am7x_hcd_next.ko");	
	#endif
	#endif
	SWFEXT_FUNC_END();
}
#if 0
static int sysinfo_projectorWire_macaddress(char *half_address_info){
	char callbuf[50];
	char *mac_address_info;
	int ret=-1;
	FILE *fp;
	char buf[512];
	char mask_get_from_ifconfig[50]={0};
	memset(callbuf,0,50);
	sprintf(callbuf,"ifconfig eth0 > /tmp/ifconfig.wireinfo");

	printf("---the call is--- %s to get mac addresss\n",callbuf); 
	system(callbuf);
	fp = fopen("/tmp/ifconfig.wireinfo","r");
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
	printf("@@@@[%s][%d]\n",__FILE__,__LINE__);
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
	
	if(mac_address_info==NULL){
		strncpy(mac_address_info,"00:00:00:00:00:00",18);
	}

	memcpy(half_address_info, mac_address_info+6, 12);
	return 0;
}
#endif


int projectorWire_mac_address_info(char *mac_address_info)
{
	char callbuf[50];
	int ret=-1;
	FILE *fp;
	char buf[512];
	char mask_get_from_ifconfig[50]={0};
	sprintf(callbuf,"ifconfig eth0 > /tmp/ifconfig.wireinfo");
	printf("---the call is--- %s to get mac addresss\n",callbuf); 
	system(callbuf);
	fp = fopen("/tmp/ifconfig.wireinfo","r");
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


int projectorWire_half_mac_address(char *half_address_info){
	char mac[18];
	memset(mac,0,18);

	if(projectorWire_mac_address_info(mac)==-1){
		strncpy(mac,"00:00:00:00:00:00",18);
		}

	memcpy(half_address_info, mac+6, 12);
	printf("@@@@[%s][%d]half_address_info=%s\n",half_address_info);
	return 0;
}

int change_projectorwire_ssid(char *ssid_change){

	FILE *fp = NULL, *fp_p2p = NULL;
	char buf[4096] ={0};

	char callbuf[128];
	char *tmp_ssid=NULL;
	char conf_file[100]="";

	char confbak_file[100]="";
	int ret=-1;
	int dongle_type=0;
	short  ssid_change_len = 0;
	//get_half_mac_address(half_mac_info);
	//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);
	sprintf(conf_file,"%s",HOSTAPD_CON_FILE);
	sprintf(confbak_file,"%s",HOSTAPD_BAK_CON_FILE);

	dongle_type=dongle_get_device_changing();
	//printf("function:%d,line:%d\n",__FUNCTION__,__LINE__);

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
	if(fp == NULL){
		printf("[%s][%d] -- Open hostapd config error!!!\n", __func__, __LINE__);
		return -1;
	}
	ret=fread(buf, 1, 4096, fp);
	ret=fclose(fp);
	printf("ssid_change=================%s\n",ssid_change);

		if(ssid_change != NULL && strlen(ssid_change) > 0)
			modify_line_realtek(buf,"ssid=",ssid_change);

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
	return 0;
}

int sysinfo_projectorWire_ssid(){			//change the softap ssid and write it into the hostapd conf
	char rank_tail[20]={0};
	char ssid_change[50]={0};
	char psk_change[20]={0};
	int sum = 0;
	int ret=-1;
	int dongle_type=0;
	short  ssid_change_len = 0;

	dongle_type=dongle_get_device_changing();
	
	/*if(REALTEK_dongle==dongle_type)*/{

			projectorWire_half_mac_address(rank_tail);	
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

		ssid_change_len = strlen(ssid_change);
		getDefaultPsk(ssid_change, psk_change, sizeof(psk_change));

		printf("@@@@@ssid_change=%s,psk_change=%d\n",ssid_change,psk_change);
		change_projectorwire_ssid(ssid_change);			// For public mode

	}
	return 0;
}

static int sysinfo_projectorWireMode(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	adbAudioInit();
	usleep(100*1000);
	system("insmod /lib/modules/2.6.27.29/am7x_udc.ko");
#if 0
		usleep(100*1000);
		system("modprobe /lib/modules/2.6.27.29/snd-usb-audio.ko");
#endif

	system("insmod /lib/modules/2.6.27.29/g_ether.ko");
	system("ifconfig usb0 192.168.203.1 up");
	system("udhcpd /etc/udhcpd_usb.conf");
	ezwireAddUsbPort(1);


	system("thttpd -C /etc/thttpd.conf -u root");

	//ezConfigSetDevname("ezcast", strlen("ezcast"));
	//ezConfigSetDlnaId("ezcast");
	//ezConfigSetAirplayId("ezcast");
	ezFuiSetAudioLineIn(0);
#if 0//EZWIRE_TYPE == 3
		int mfd = open("/tmp/MiraWirePlus", O_CREAT|O_RDWR);
		if(mfd >= 0)
			close(mfd);
#endif

	if(access("/mnt/vram/ezcast/PLUGPLAYMODE", F_OK) != 0)
	{
		setWirePlugModeDefault();
	}
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
	ezFuiWireStopMirrorCallbackSet(wireStopMirrorCallback);
#endif
//	sysinfo_projectorWire_ssid();//for projector no dongle and lan
	
	SWFEXT_FUNC_END();

}
static int setWirePlugMode(int val)
{
	FILE *fp = NULL;
	char buf[32];

	snprintf(buf, sizeof(buf), "%s\n", (val == 0)?"PlugMode=NO":"PlugMode=YES");
	printf("To set PLUGPLAYMODE: %s\n", buf);
	fp = fopen("/mnt/vram/ezcast/PLUGPLAYMODE", "w");
	if(fp != NULL)
	{
		fwrite(buf, 1, strlen(buf), fp);
		fclose(fp);
		sync();
		iOSDisconnect();
		printf("Set OK.\n");
	}
}
static int sysinfo_setWirePlugMode(void *handle)
{
	int ret = 0;
	SWFEXT_FUNC_BEGIN(handle);
	int val = Swfext_GetNumber();
	setWirePlugMode(val);
	Swfext_PutNumber(ret);
	SWFEXT_FUNC_END();
}

static int sysinfo_PlugPlay(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int val = Swfext_GetNumber();
#if EZWIRE_CAPTURE_ENABLE
	EZCASTLOG("To %s plug&play.\n", (val == 0)?"stop":"start");
	if(val == 0)
	{
		if(catptureStart != 0)
		{
			setiOSInStatus(0);
			stopDirectMirroring();
			catptureStart = 0;
		}
	}
	else
	{
		if(catptureStart == 0)
		{
			startDirectMirroring();
			catptureStart = 1;
		}
	}
#endif
	SWFEXT_FUNC_END();
}

#if EZWIRE_CAPTURE_ENABLE
int getPlugPlayStatus()
{
	return catptureStart;
}
#endif

#else
static int sysinfo_get_ezwire_status(void *handle)
{
	char val;
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(0);
	SWFEXT_FUNC_END();
}
static int sysinfo_get_ezwire_ip_mobile(void *handle)
{
	char buf[16];
	SWFEXT_FUNC_BEGIN(handle);
	sprintf(buf, "null");

	Swfext_PutString(buf);
		
	SWFEXT_FUNC_END();
}
static int sysinfo_swf_file_exist(void *handle)
{
	int ret;
	SWFEXT_FUNC_BEGIN(handle);
	char *name = Swfext_GetString();
	Swfext_PutNumber(-1);
	SWFEXT_FUNC_END();
}

static int sysinfo_PlugPlay(void *handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	int val = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}
#endif
int swfext_system_info_register(void)
{
	SWFEXT_REGISTER("sys_setVolume", sysinfo_set_volume);
	SWFEXT_REGISTER("sys_getVolume", sysinfo_get_volume);
	SWFEXT_REGISTER("sys_setVolumeMute", sysinfo_set_volume_mute);
	
	SWFEXT_REGISTER("sys_getVersion", sysinfo_get_version);
	SWFEXT_REGISTER("sys_getFwVersion", sysinfo_get_fwversion);
	SWFEXT_REGISTER("sys_getDiskSpace", sysinfo_get_disk_space);
	SWFEXT_REGISTER("sys_getDiskSpaceLeft", sysinfo_get_disk_free_space);
	SWFEXT_REGISTER("sys_getCardSpace", sysinfo_get_card_space);
	SWFEXT_REGISTER("sys_getCardSpaceLeft", sysinfo_get_card_free_space);
	SWFEXT_REGISTER("sys_getCFSpace", sysinfo_get_cfcard_space);
	SWFEXT_REGISTER("sys_getCFSpaceLeft", sysinfo_get_cfcard_free_space);
	SWFEXT_REGISTER("sys_getUSBSpace", sysinfo_get_udisk_space);
	SWFEXT_REGISTER("sys_getUSBSpaceLeft", sysinfo_get_udisk_free_space);	
	
	SWFEXT_REGISTER("sys_setBackLightStrength", sysinfo_set_backlight);
	SWFEXT_REGISTER("sys_getBackLightStrength", sysinfo_get_back_light);
	
	SWFEXT_REGISTER("sys_ColorAdjustIO",sysinfo_adjust_color);

	SWFEXT_REGISTER("sys_getYear", sysinfo_get_year);
	SWFEXT_REGISTER("sys_setYear", sysinfo_set_year);
	SWFEXT_REGISTER("sys_getMonth", sysinfo_get_month);
	SWFEXT_REGISTER("sys_setMonth", sysinfo_set_month);
	SWFEXT_REGISTER("sys_getDay", sysinfo_get_day);
	SWFEXT_REGISTER("sys_setDay", sysinfo_set_day);
	SWFEXT_REGISTER("sys_getWday", sysinfo_get_wday);
	
	SWFEXT_REGISTER("sys_getHour", sysinfo_get_hour);
	SWFEXT_REGISTER("sys_setHour", sysinfo_set_hour);	
	SWFEXT_REGISTER("sys_getMin", sysinfo_get_min);
	SWFEXT_REGISTER("sys_setMin", sysinfo_set_min);
	SWFEXT_REGISTER("sys_getSecond", sysinfo_get_sec);
	SWFEXT_REGISTER("sys_setSecond", sysinfo_set_sec);

	SWFEXT_REGISTER("sys_getClockMode", sysinfo_get_clock_mode);
	SWFEXT_REGISTER("sys_setClockMode", sysinfo_set_clock_mode);	
	SWFEXT_REGISTER("sys_getAlarmHour", sysinfo_get_alarm_hour);
	SWFEXT_REGISTER("sys_setAlarmHour", sysinfo_set_alarm_hour);	
	SWFEXT_REGISTER("sys_getAlarmMin", sysinfo_get_alarm_min);
	SWFEXT_REGISTER("sys_setAlarmMin", sysinfo_set_alarm_min);	
	SWFEXT_REGISTER("sys_getAlarmEnable", sysinfo_get_alarm_enable);
	SWFEXT_REGISTER("sys_setAlarmEnable", sysinfo_set_alarm_enable);	
	SWFEXT_REGISTER("sys_getAlarmRing", sysinfo_get_alarm_ring);
	SWFEXT_REGISTER("sys_setAlarmRing", sysinfo_set_alarm_ring);
	SWFEXT_REGISTER("sys_storeConfig", sysinfo_store_config);

	
	SWFEXT_REGISTER("sys_getCurLanguage", sysinfo_get_local_language);
	SWFEXT_REGISTER("sys_setCurLanguage", sysinfo_set_local_language);
	SWFEXT_REGISTER("sys_getLanguageTotalNum", sysinfo_get_language_total_num);
	SWFEXT_REGISTER("sys_getLanguageIdx2Str", sysinfo_get_language_idx2str);
	
	SWFEXT_REGISTER("sys_checkCardStatus", sysinfo_check_card_status);
	SWFEXT_REGISTER("sys_checkCardType", sysinfo_check_card_type);
	SWFEXT_REGISTER("sys_systUpgrade", sysinfo_upgrade);
	SWFEXT_REGISTER("sys_checkUsb_one_Status", sysinfo_check_port_one_status);//usb port 0 ===>mass storage device status
	SWFEXT_REGISTER("sys_checkUsb_two_Status", sysinfo_check_port_two_status);//usb port1 ====>mass storage device status
	SWFEXT_REGISTER("sys_checkCFStatus", sysinfo_check_cfcard_status);
	SWFEXT_REGISTER("sys_checkBTStatus",sysinfo_check_bluetooth_status);
	SWFEXT_REGISTER("sys_setVideoAutoPlayStatus", sysinfo_set_video_autoplay_status);
	SWFEXT_REGISTER("sys_getVideoAutoPlayStatus", sysinfo_get_video_autoplay_status);
	SWFEXT_REGISTER("sys_getAutoPowerOnFlag", sysinfo_get_auto_poweron_flag);
	SWFEXT_REGISTER("sys_setAutoPowerOnFlag", sysinfo_set_auto_poweron_flag);
	SWFEXT_REGISTER("sys_getAutoPowerOnHour", sysinfo_get_auto_poweron_hour);
	SWFEXT_REGISTER("sys_setAutoPowerOnHour", sysinfo_set_auto_poweron_hour);
	SWFEXT_REGISTER("sys_getAutoPowerOnMin", sysinfo_get_auto_poweron_min);
	SWFEXT_REGISTER("sys_setAutoPowerOnMIn", sysinfo_set_auto_poweron_min);
	SWFEXT_REGISTER("sys_getAutoPowerOffFlag", sysinfo_get_auto_poweroff_flag);
	SWFEXT_REGISTER("sys_setAutoPowerOffFlag", sysinfo_set_auto_poweroff_flag);
	SWFEXT_REGISTER("sys_getAutoPowerOffHour", sysinfo_get_auto_poweroff_hour);
	SWFEXT_REGISTER("sys_setAutoPowerOffHour", sysinfo_set_auto_poweroff_hour);	
	SWFEXT_REGISTER("sys_getAutoPowerOffMin", sysinfo_get_auto_poweroff_min);
	SWFEXT_REGISTER("sys_setAutoPowerOffMin", sysinfo_set_auto_poweroff_min);
	SWFEXT_REGISTER("sys_getAutoPowerFreq", sysinfo_get_auto_power_freq);
	SWFEXT_REGISTER("sys_setAutoPowerFreq", sysinfo_set_auto_power_freq);
	SWFEXT_REGISTER("sys_media2Disk", sysinfo_media_to_disk);
	SWFEXT_REGISTER("sys_setActiveMedia", sysinfo_set_active_media);
	SWFEXT_REGISTER("sys_resotreSysDefaultConfig", sysinfo_set_default);
	SWFEXT_REGISTER("sys_getActiveWorkPath", sysinfo_get_active_work_path);
	
	SWFEXT_REGISTER("sys_getPrivateDiskSymbol", sysinfo_get_private_disk_symbol);
	
	SWFEXT_REGISTER("sys_clearKeyMessage", sysinfo_clear_key_msg);
	SWFEXT_REGISTER("sys_enablePrinter", sysinfo_enable_printer);
	SWFEXT_REGISTER("sys_acceptHotplugInfo", sysinfo_accept_hotplug_info);

	SWFEXT_REGISTER("sys_acceptPCconInfo", sysinfo_accept_pcconnect_info);

	SWFEXT_REGISTER("sys_checkStartupUstatus",sysinfo_check_startup_ustatus);
	SWFEXT_REGISTER("sys_setHDMIMode", sysinfo_set_HDMI_mode);
	SWFEXT_REGISTER("sys_getHDMIMode", sysinfo_get_HDMI_mode);
	SWFEXT_REGISTER("sys_setOutputPrevMode", sysinfo_set_output_prev_mode);

	SWFEXT_REGISTER("sys_setOutputMode", sysinfo_set_output_mode);
	SWFEXT_REGISTER("sys_getOutputMode", sysinfo_get_output_mode);
	SWFEXT_REGISTER("sys_getCurscreenparam", sysinfo_get_curscreen_param);
	
	SWFEXT_REGISTER("sys_getMultiSectorNum", sysinfo_get_multisector);

	SWFEXT_REGISTER("sys_getCardUpgradeStatus", sysinfo_get_cardup_status);
	SWFEXT_REGISTER("sys_getOtaUpgradeStatus", sysinfo_get_ota_status);
	SWFEXT_REGISTER("sys_getOtaLocalVersion", sysinfo_get_ota_local_version);

	SWFEXT_REGISTER("sys_getAutoScreenOffFlag", sysinfo_get_auto_screenoff_flag);
	SWFEXT_REGISTER("sys_setAutoScreenOffFlag", sysinfo_set_auto_screenoff_flag);
	SWFEXT_REGISTER("sys_getAutoScreenOffTime", sysinfo_get_auto_screenoff_time);
	SWFEXT_REGISTER("sys_setAutoScreenOffTime", sysinfo_set_auto_screenoff_time);
	SWFEXT_REGISTER("sys_change_pll", sysinfo_change_pll);

	SWFEXT_REGISTER("sys_getBatteryChargeStatus", sysinfo_get_battery_charge_status);
	SWFEXT_REGISTER("sys_getBatteryVoltage", sysinfo_get_battery_voltage);

	SWFEXT_REGISTER("sys_getGlobalValue", sysinfo_get_globalvalue);
	
	SWFEXT_REGISTER("sys_checkusbconnectstatus", sysinfo_check_usb_connect_status);

	SWFEXT_REGISTER("sys_Standby", sysinfo_Standby);

	SWFEXT_REGISTER("sys_Access", sysinfo_access);

	SWFEXT_REGISTER("sys_Softap", sysinfo_Softap);

	SWFEXT_REGISTER("sys_StopNetDisplay", sysinfo_StopNetDisplay);
	SWFEXT_REGISTER("sys_StartNetDisplay", sysinfo_NetDisplay);
	SWFEXT_REGISTER("sys_LanStart", sysinfo_LANStart);
	SWFEXT_REGISTER("sys_SetForUSBDisplay", sysinfo_SetForUSBDisplay);
	SWFEXT_REGISTER("sys_checkMediaStatus",sysinfo_check_media_status);

	SWFEXT_REGISTER("sys_Uart_Write_Str", sysinfo_writeUart_Str);
	SWFEXT_REGISTER("sys_setAudioAutoPlayStatus",sysinfo_set_audio_autoplay_status); 
	SWFEXT_REGISTER("sys_getAudioAutoPlayStatus",sysinfo_get_audio_autoplay_status); 
	SWFEXT_REGISTER("sys_putwifimode",sysinfo_put_wifi_mode); 
	SWFEXT_REGISTER("sys_getwifimode",sysinfo_get_wifi_mode); 
	
	SWFEXT_REGISTER("sys_checkRam",sysinfo_check_ram); 
	
	SWFEXT_REGISTER("sysinfo_getlanstatus",sysinfo_get_lan_status); 
	
	SWFEXT_REGISTER("sysinfo_getnetlinkstatus",sysinfo_get_netlink_status); 
	SWFEXT_REGISTER("sysinfo_getpasswordfromDPF",sysinfo_get_password_from_DPF); 
	SWFEXT_REGISTER("sysinfo_checkbinfilesexsit",sysinfo_check_binfiles_exsit); 
	SWFEXT_REGISTER("sysinfo_startcdromfunc",sysinfo_start_cdrom_func); 
	
	SWFEXT_REGISTER("sysinfo_gethostname",sysinfo_get_hostname); 
	SWFEXT_REGISTER("sysinfo_sethostname",sysinfo_set_hostname); 

	
	SWFEXT_REGISTER("sysinfo_setgpioforusbswitch",sysinfo_setgpio_for_usbswitch); 
	SWFEXT_REGISTER("sysinfo_netlink_for_intoSWF",sysinfo_check_netlink_for_into_SWF);
	//for usb compsite
	
	SWFEXT_REGISTER("sysinfo_getusbcompsitedevicetotal",sysinfo_usb_compsite_device_total); 
	SWFEXT_REGISTER("sysinfo_getusbcompsitedeviceintfacetotal",sysinfo_usb_compsite_device_intface_total);
	SWFEXT_REGISTER("sysinfo_getusbcompsitedevicenameinformation",sysinfo_usb_compsite_device_name_information);
	SWFEXT_REGISTER("sysinfo_getusbcompsitedeviceclassinformation",sysinfo_usb_compsite_device_class_information);
	SWFEXT_REGISTER("sysinfo_chooseusbcompsitefunction",sysinfo_choose_usb_compsite_function);

	SWFEXT_REGISTER("sysinfo_getconnectpcflag",sysinfo_get_connectpc_flag);

	
	SWFEXT_REGISTER("sysinfo_createdidbin",sysinfo_creat_edid_bin);
	SWFEXT_REGISTER("sysinfo_deleteedidbin",sysinfo_delete_edid_bin);
	SWFEXT_REGISTER("sysinfo_readedidbin",sysinfo_read_edid_bin);

	SWFEXT_REGISTER("sysinfo_getstoragetotal",sysinfo_get_storage_total);
	SWFEXT_REGISTER("sysinfo_getmutisectorinfo",sysinfo_get_multisector_info);	
	SWFEXT_REGISTER("sysinfo_getusbmass_devname",sysinfo_get_usb_mass_deviceName);

	SWFEXT_REGISTER("sysinfo_otaupgrade",sysinfo_ota_upgrade);
	SWFEXT_REGISTER("sysinfo_getmtpdeviceinflag",sysinfo_get_mtp_device_in_flag);
	
	SWFEXT_REGISTER("sysinfo_rebootsystem",sysinfo_reboot_system);
	
	SWFEXT_REGISTER("sysinfo_set_wifichannelregion",sysinfo_SetWifiChannelRegion);
	SWFEXT_REGISTER("sysinfo_putswfactiveflag",sysinfo_put_swf_activeflag);

	SWFEXT_REGISTER("sys_getOtaCheckStatus", sysinfo_get_ota_check_status);
	SWFEXT_REGISTER("sys_getOtaServerVersion", sysinfo_get_ota_server_version);
	SWFEXT_REGISTER("sys_check_ezmusic_flag", sysinfo_check_ezmusic_flag);	
#if AIRDISK_ENABLE
	SWFEXT_REGISTER("mount_usb_tohttp", mount_usb_tohttp);
	SWFEXT_REGISTER("umount_usb", umount_usb);
	SWFEXT_REGISTER("mount_card_tohttp", mount_card_tohttp);
	SWFEXT_REGISTER("umount_card", umount_card);
#endif

#if WEBSETTING_ENABLE
	SWFEXT_REGISTER("sys_Get_language_index", sysinfo_Get_language_index);	
	SWFEXT_REGISTER("sys_Set_language_index", sysinfo_Set_language_index);
#endif
#if EZCAST_ENABLE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	SWFEXT_REGISTER("sys_getcharwidth", get_char_width);
//for WiFi Enterprise
	SWFEXT_REGISTER("wifi_set_802XEPA_conf",  wifi_set_802XEPA_conf);
	SWFEXT_REGISTER("wifi_full_802XEPA_conf",  wifi_full_802XEPA_conf);
	SWFEXT_REGISTER("sys_customerswf_file_exist", sysinfo_customerswf_file_exist);
	SWFEXT_REGISTER("sys_set_passcode", sysinfo_set_passcode);
	SWFEXT_REGISTER("sys_get_passcode", sysinfo_get_passcode);
	SWFEXT_REGISTER("sys_get_passcode_onoff", sysinfo_get_passcode_onoff);
	SWFEXT_REGISTER("sys_get_user_define_passcode", sysinfo_get_user_define_passcode);
#endif
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && (MODULE_CONFIG_EZCASTPRO_MODE==8075)
		SWFEXT_REGISTER("sysinfo_save_outputmode", sysinfo_save_outputmode);
		SWFEXT_REGISTER("sysinfo_get_outputmode",  sysinfo_get_outputmode); 
		SWFEXT_REGISTER("sys_proBox_wifi_subdisplay_end", sysproBox_wifi_subdisplay_end);
#endif
	SWFEXT_REGISTER("sysinfo_otaupgrade_from_app",sysinfo_ota_upgrade_from_app);
	SWFEXT_REGISTER("sys_get_ProboxConnectStatus", sysinfo_get_ProboxConnectStatus);

	SWFEXT_REGISTER("sys_ezcastpro_read_mode", ezcastpro_read_mode);
	SWFEXT_REGISTER("sys_isPayUpgrade", sysinfo_isPayUpgrade);
	SWFEXT_REGISTER("sys_sysinfo_sys_volume_ctrl_get", sysinfo_sys_volume_ctrl_get);	
	SWFEXT_REGISTER("sys_deleteWifiConf", sysinfo_delete_wifi_conf);
	SWFEXT_REGISTER("sys_ezcast_mask", sysinfo_ezcast_enable_judge);
	SWFEXT_REGISTER("sys_get_last_ui", sysinfo_get_last_ui);
	SWFEXT_REGISTER("sys_set_last_ui", sysinfo_set_last_ui);
	SWFEXT_REGISTER("sys_ez_socket_reset", sysinfo_ez_socket_reset);
	SWFEXT_REGISTER("sys_json_set_value", sysinfo_json_set_value);
	SWFEXT_REGISTER("sys_get_display_status", sysinfo_get_display_status);
	SWFEXT_REGISTER("sys_get_disauto_status", sysinfo_get_disauto_status);
	SWFEXT_REGISTER("sys_set_disauto_status", sysinfo_set_disauto_status);
	SWFEXT_REGISTER("sys_langautoget", sysinfo_langautoget);
	SWFEXT_REGISTER("sys_get_ezcast_vendor", sysinfo_get_ezcast_vendor);
	SWFEXT_REGISTER("sys_get_ota_download_number", sysinfo_get_ota_download_number);
	SWFEXT_REGISTER("sys_get_ota_enforce", sysinfo_get_ota_enforce);
	SWFEXT_REGISTER("sys_get_cpu_frequency", sysinfo_get_cpu_frequency);
	SWFEXT_REGISTER("sys_get_ddr_type", sysinfo_get_ddr_type);
	SWFEXT_REGISTER("sysinfo_get_router_ctl", sysinfo_get_router_ctl);
	SWFEXT_REGISTER("sysinfo_get_connect_mode", sysinfo_get_connect_mode);
	SWFEXT_REGISTER("sysinfo_set_connect_mode", sysinfo_set_connect_mode);
	SWFEXT_REGISTER("sysinfo_get_ip", sysinfo_get_ip);
	SWFEXT_REGISTER("sys_get_miracode", sysinfo_get_miracode);
	SWFEXT_REGISTER("sys_get_miracode_enable", sysinfo_get_miracode_enable);
	SWFEXT_REGISTER("sys_get_24g_or_5g", sysinfo_get_24g_or_5g);
	SWFEXT_REGISTER("sysinfo_get_system_time", sysinfo_get_system_time);
	//SWFEXT_REGISTER("sys_set_standby_time", sysinfo_set_standby_time);
	//SWFEXT_REGISTER("sys_get_standby_time", sysinfo_get_standby_time);
	//SWFEXT_REGISTER("sys_set_neighbour", sysinfo_set_neighbour);
	//SWFEXT_REGISTER("sys_get_neighbour", sysinfo_get_neighbour);
	SWFEXT_REGISTER("sys_set_config", sysinfo_set_config);
	SWFEXT_REGISTER("sys_get_config", sysinfo_get_config);
	SWFEXT_REGISTER("sys_get_wifidisplay_status", sysinfo_get_wifidisplay_status);
	SWFEXT_REGISTER("sys_deleteEZCastConf", sysinfo_delete_EZCast_conf);

	SWFEXT_REGISTER("sys_set_device_name", sysinfo_set_device_name);
	SWFEXT_REGISTER("sys_get_device_name", sysinfo_get_device_name);
	SWFEXT_REGISTER("sys_set_psk_hide_status", sysinfo_set_psk_hide_status);
	
	SWFEXT_REGISTER("sys_getIncludeNumberValue", sysinfo_getIncludeNumberValue);
	SWFEXT_REGISTER("sys_reset_userpassword", sysinfo_reset_userpassword);
	//SWFEXT_REGISTER("sys_close_display", close_display);
	//SWFEXT_REGISTER("sys_open_display", open_display);
	SWFEXT_REGISTER("sys_set_ezair_mode", sysinfo_set_ezair_mode);
	SWFEXT_REGISTER("sys_get_ezair_mode", sysinfo_get_ezair_mode);
	
#if TEST_ENABLE
	SWFEXT_REGISTER("sys_get_test_config", sysinfo_get_test_config);
	SWFEXT_REGISTER("sys_set_factory_test_video_end", sysinfo_set_factory_test_video_end);
	SWFEXT_REGISTER("sysinfo_get_Throughput", sysinfo_get_Throughput);
	SWFEXT_REGISTER("sysinfo_get_channel_signal", sysinfo_get_channel_signal);
	SWFEXT_REGISTER("sysinfo_init_factorytest", sysinfo_init_factorytest);
	SWFEXT_REGISTER("sysinfo_version_factorytest", sysinfo_version_factorytest);
	SWFEXT_REGISTER("sysinfo_language_factorytest", sysinfo_language_factorytest);
	SWFEXT_REGISTER("sysinfo_get_lan_factorytest", sysinfo_lan_factorytest);
	SWFEXT_REGISTER("sysinfo_get_uart_factorytest", sysinfo_uart_factorytest);
	SWFEXT_REGISTER("sysinfo_itemlist_factorytest", sysinfo_itemlist_factorytest);
	SWFEXT_REGISTER("sysinfo_get_edid_factorytest", sysinfo_get_edid_factorytest);
	SWFEXT_REGISTER("sysinfo_version_result_factorytest", sysinfo_version_result_factorytest);
	SWFEXT_REGISTER("sysinfo_language_result_factorytest", sysinfo_language_result_factorytest);
	SWFEXT_REGISTER("sysinfo_test_edid", sysinfo_test_edid);
	SWFEXT_REGISTER("sysinfo_test_hdcpkey", sysinfo_test_hdcpkey);
	
#endif		// TEST_ENABLE
#if EZMUSIC_ENABLE
	SWFEXT_REGISTER("sys_deleteEZmusicConf", sysinfo_deleteEZmusicConf);
	SWFEXT_REGISTER("sysinfo_mainswf_skip_get_keycode",mainswf_skip_get_keycode);
	SWFEXT_REGISTER("sys_set_stream_play_status", sysinfo_set_stream_play_status);
#endif		// EZMUSIC_ENABLE
#endif		// EZCAST_ENABLE

#if EZWILAN_ENABLE
SWFEXT_REGISTER("sys_set_LED", sysinfo_set_LED);
#endif		// EZWILAN_ENABLE

#if AUTOPLAY_SET_ENABLE
	SWFEXT_REGISTER("sys_getAutoplayEnable", sysinfo_getAutoplayEnable);
	SWFEXT_REGISTER("sys_setAutoplayEnable", sysinfo_setAutoplayEnable);
	SWFEXT_REGISTER("sys_getAutoplayHostAp", sysinfo_getAutoplayHostAp);
	SWFEXT_REGISTER("sys_setAutoplayHostAp", sysinfo_setAutoplayHostAp);
	SWFEXT_REGISTER("sys_getAutoplayProgressive", sysinfo_getAutoplayProgressive);
	SWFEXT_REGISTER("sys_setAutoplayProgressive", sysinfo_setAutoplayProgressive);
	SWFEXT_REGISTER("sys_getAutoplayPlaylist", sysinfo_getAutoplayPlaylist);
	SWFEXT_REGISTER("sys_setAutoplayPlaylist", sysinfo_setAutoplayPlaylist);
	SWFEXT_REGISTER("sys_getAutoplayWaitime", sysinfo_getAutoplayWaitime);
	SWFEXT_REGISTER("sys_setAutoplayWaitime", sysinfo_setAutoplayWaitime);
	SWFEXT_REGISTER("sys_getAutoplayVolume", sysinfo_getAutoplayVolume);
	SWFEXT_REGISTER("sys_setAutoplayVolume", sysinfo_setAutoplayVolume);
#endif		// #if AUTOPLAY_SET_ENABLE

#if FUN_WIRE_ENABLE
	SWFEXT_REGISTER("sys_get_ezwire_status", sysinfo_get_ezwire_status);
	SWFEXT_REGISTER("sys_get_ezwire_ip_mobile", sysinfo_get_ezwire_ip_mobile);
	SWFEXT_REGISTER("sys_swf_file_exist", sysinfo_swf_file_exist);
	SWFEXT_REGISTER("sys_aoa_disable", sysinfo_aoa_disable);
	SWFEXT_REGISTER("sys_isQCMode", sysinfo_isQCMode);
	SWFEXT_REGISTER("sys_getWirePlugMode", sysinfo_getWirePlugMode);
	SWFEXT_REGISTER("sys_setWirePlugMode", sysinfo_setWirePlugMode);
	SWFEXT_REGISTER("sys_PlugPlay", sysinfo_PlugPlay);
	SWFEXT_REGISTER("sys_projectorwire_insmod", sysinfo_projectorWire_insmod);
	SWFEXT_REGISTER("sys_projectorwire_rmmod", sysinfo_projectorWire_rmmod);
	SWFEXT_REGISTER("sys_projectorwire_mode", sysinfo_projectorWireMode);
	SWFEXT_REGISTER("sys_getWireDdefaultMode", sysinfo_getWireDdefaultMode);
	SWFEXT_REGISTER("sys_setWireDdefaultMode", sysinfo_setWireDdefaultMode);

#endif	

	
    SWFEXT_REGISTER("sys_refresh_apinfo_for_ezlauncher", refresh_apinfo_for_ezlauncher);
	return 0;
}
/*
 int Mega_set_HDMI_output(int mode)
{

#if 1
	char output_mode=0;
	struct sysconf_param sys_cfg_data;
	output_mode = 1;
	printf("sysinfo--output mode is %d\n",output_mode);
	if(_get_env_data(&sys_cfg_data)!=0)
	{
		sysdbg_err("Sorry Read System Data Error!");
	}
	else
	{
		sys_cfg_data.output_enable= output_mode;
	}
	_save_env_data(&sys_cfg_data);

#endif



#if 1

	int hmode = 0;
	
	hmode = mode;

	printf("output enable is 0x%x\n",sys_cfg_data.output_enable);
	output_prev_enable = sys_cfg_data.output_enable;
	if((sys_cfg_data.output_enable & HDMI_ENABLE)!=0)
	{
		printf("HDMI Output is enable\n");
		output_prev_mode = sys_cfg_data.hdmi_mode;
		_exec_hdmi_mode(hmode);
	}
	if((sys_cfg_data.output_enable & CVBS_ENABLE)!=0)
	{
		printf("CVBS Output is enable\n");
		output_prev_mode = sys_cfg_data.cvbs_mode;
		sys_cfg_data.cvbs_mode = hmode;
		_save_env_data(&sys_cfg_data);
		_do_action(CMD_SET_CVBS_MODE,&hmode);
		//hdmi_output_data.hdmi_true= 1;
	}
	if((sys_cfg_data.output_enable & YPBPR_ENABLE)!=0)
	{
		printf("YPbPr Output is enable\n");
		output_prev_mode = sys_cfg_data.ypbpr_mode;
		sys_cfg_data.ypbpr_mode = hmode;
		_save_env_data(&sys_cfg_data);
		_do_action(CMD_SET_YPBPR_MODE,&hmode);
		//hdmi_output_data.hdmi_true= 1;
	}
	printf("set previous output enable is 0x%x, mode is %d\n",output_prev_enable, output_prev_mode);
	_get_screen_size();
#endif
}
*/
