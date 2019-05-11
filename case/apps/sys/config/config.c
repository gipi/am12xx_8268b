#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "sys_cfg.h"
#include "display.h"
#include "lcm_op.h"
#include "am7x_cec.h"
#include "apps_vram.h"
#include "sys_rtc.h"
#include "am7x_dac.h"
#include <sys/time.h>
#include <am7x_mconfig.h>
#include "ezcast_public.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../scripts/mconfig.h"
#endif

#define hdmi_cec_DISABLE		"/mnt/vram/ezcast/hdmicec_disable"

static int fui_insmod_2d()
{
	int err;
	char cmd[256];

	sprintf(cmd,"%s%s/%s","sh ",AM_CASE_SC_DIR,"driver_load_2d.sh");
	err = system(cmd);

	return err;
}

static void print_de_cfg(DE_config ds_conf)
{
#if 0
	printf("\n\n==================de config==================\n");
	printf("=======de input =========\n");
	printf("bus_addr=0x%x\n",ds_conf.input.bus_addr);
	printf("bus_addr_uv=0x%x\n",ds_conf.input.bus_addr_uv);
	printf("crop_enable=0x%x\n",ds_conf.input.crop_enable);
	printf("crop_height=0x%x\n",ds_conf.input.crop_height);
	printf("crop_width=0x%x\n",ds_conf.input.crop_width);
	printf("crop_x=0x%x\n",ds_conf.input.crop_x);
	printf("crop_y=0x%x\n",ds_conf.input.crop_y);
	printf("enable=0x%x\n",ds_conf.input.enable);
	printf("height=0x%x\n",ds_conf.input.height);
	printf("img=0x%x\n",ds_conf.input.img);
	printf("img_uv=0x%x\n",ds_conf.input.img_uv);
	printf("pix_fmt=0x%x\n",ds_conf.input.pix_fmt);
	printf("video_range=0x%x\n",ds_conf.input.video_range);
	printf("width=0x%x\n",ds_conf.input.width);
	
	printf("========de output=========\n");
	printf("brightness=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].brightness);
	printf("contrast=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].contrast);
	printf("dar_height=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].dar_height);
	printf("dar_width=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].dar_width);
	printf("display_mode=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].display_mode);
	printf("enable=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].enable);
	printf("gamma=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].gamma);
	printf("hue=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].hue);
	printf("output_mode=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].output_mode);
	printf("pip_enable=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].pip_enable);
	printf("pip_height=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].pip_height);
	printf("pip_width=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].pip_width);
	printf("pip_x=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].pip_x);
	printf("pip_y=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].pip_y);
	printf("saturation=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].saturation);
	printf("sharpness=0x%x\n",ds_conf.output[DE_OUT_DEV_ID_LCD].sharpness);	
	printf("==================de config end==================\n\n");
#endif
}

static int _set_de_form_global_value(DE_config *ds_conf)
{
	struct sysconf_param sys_cfg_data;
	int de_input_width=0;
	int de_input_heigt=0;
	int de_default_color=0;
	int de_pix_format=0;
	int de_output_dislplay_mode=0;
	if(apps_vram_read(VRAM_ID_SYSPARAMS,&sys_cfg_data,sizeof(struct sysconf_param))==0){
		if(sys_cfg_data.is_gloval_valid){
			de_input_width = sys_cfg_data.glo_val.nums[0];
			de_input_heigt = sys_cfg_data.glo_val.nums[1];
			de_default_color=sys_cfg_data.glo_val.nums[2];
			de_pix_format =sys_cfg_data.glo_val.nums[3];
			de_output_dislplay_mode = sys_cfg_data.glo_val.nums[4];
			printf("GlobalValue width=%d,height=%d,default_color=0x%x,format=%d,display_mode=%d\n",
				de_input_width,
				de_input_heigt,
				de_default_color,
				de_pix_format,
				de_output_dislplay_mode);
			if(de_input_width && de_input_heigt){
				ds_conf->input.width = de_input_width;
				ds_conf->input.height= de_input_heigt;
			}
			if(de_pix_format)
				ds_conf->input.pix_fmt = de_pix_format;
			ds_conf->input.default_color = de_default_color;
			if(de_output_dislplay_mode)
				ds_conf->output.display_mode = de_output_dislplay_mode;
		}
		return 0;
	}
	return -1;
}

#define LCM_CONFIG_PATH  "/am7x/case/data/lcm.bin"
#define HDMI_HDMI2MHL_GPIO39			39
struct lcm_config{
	unsigned char align[0x30];
	//default device select
	unsigned char  device_type;    //default device type
	unsigned char  output_format;  //default out format 
	// lcd pannel about control information 	
	unsigned char  lvds_en;	 	   //lvds controller enable or not
	unsigned char  tcon_en;        //tcon controller enable or not   
	unsigned char  cpu_en;	       //cpu interface enable or not
	unsigned char  lcm_rgbformat;  //output color width
	unsigned char  power_gpio; 
	// clock set 
	unsigned char  display_clk;	   //display clock
	unsigned char  video_clk;      //may change clock when play video
	// device resolution 
	unsigned short device_width; 	  
	unsigned short device_height;  
	// lcd  timimg
	unsigned short hbp;
	unsigned short hfp;
	unsigned short hswp;
	unsigned short vbp;
	unsigned short vfp;
	unsigned short vswp;
	unsigned int   rgb_ctl;
	unsigned int   dac_ctl;
	// lvds  config
	unsigned char  lvds_format;	
	unsigned char  lvds_channel;
	unsigned char  lvds_swap;
	unsigned char  lvds_map;
	unsigned char  lvds_mirror;
	// tcon  config
	unsigned int tcon_ctl;
	unsigned int gp0_con;
	unsigned int gp0_hp;
	unsigned int gp0_vp;
	unsigned int gp1_con;
	unsigned int gp1_hp;
	unsigned int gp1_vp;
	unsigned int gp2_con;
	unsigned int gp2_hp;
	unsigned int gp2_vp;
	unsigned int gp3_con;
	unsigned int gp3_hp;
	unsigned int gp3_vp;
	unsigned int gp4_con;
	unsigned int gp4_hp;
	unsigned int gp4_vp;
	unsigned int gp5_con;
	unsigned int gp5_hp;
	unsigned int gp5_vp;
	unsigned int gp6_con;
	unsigned int gp6_hp;
	unsigned int gp6_vp;
	unsigned int gp7_con;
	unsigned int gp7_hp;
	unsigned int gp7_vp;
	unsigned int tcon_ls;
	unsigned int tcon_fs;
	//reserved val config
	unsigned char reserved_val1;
	unsigned char reserved_val2;
	unsigned char reserved_val3;
	unsigned char reserved_val4;

	//hdmi/mhl detect gpio
	unsigned char hdmi_hpd_gpio;
	unsigned char hdmi_mhl_gpio;

	//mhl device capability
	unsigned short mhl_adopter_id;
	unsigned short mhl_device_id;

	//hdmi Source Product Description InfoFrame	
	unsigned char	hdmi_device_info;		/* Source Device Infomation */
	unsigned char	hdmi_vendor_name[9];	/* Product Name (8 bytes) */
	unsigned char	hdmi_product_decs[17];	/* product Description (16 bytes) */
	
	
}__attribute__((packed));

static int _lcm_init()
{
	void* ds_inst;
	DE_config ds_conf;
	int fd,ret;
	struct lcm_config lcm_data;
	fd = open(LCM_CONFIG_PATH, O_RDONLY);
	if(fd < 0)
	{
		printf("NOTICE:READ LCD CONFIG FILE ERROR\n");
		perror("ERROR");
		return -1;
	}
	ret = read(fd,(char*)&lcm_data,sizeof(struct lcm_config));
	close(fd);

	de_init(&ds_inst);
	de_get_config(ds_inst,&ds_conf,DE_CFG_ALL);

	//print_de_cfg(ds_conf);

	/**
	* input params
	*/
	ds_conf.input.enable=0;
	ds_conf.input.pix_fmt=PIX_FMT_YCBCR_4_2_2_INTERLEAVED;

	/**
	* output params
	*/
	ds_conf.dev_info[DE_OUT_DEV_ID_LCD].enable=1;
	ds_conf.output.display_mode=DE_DISPLAYMODE_LETTER_BOX;

	_set_de_form_global_value(&ds_conf);

#if ((AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))	
	if ((lcm_data.device_type == E_LCDPADMODE_HDMI) || (lcm_data.device_type == E_LCDPADMODE_VGA))//E_LCDPADMODE_HDMI==6
	{
		if(ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].width && ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].height && ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].enable)
		{
			ds_conf.input.width = ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].width;
			ds_conf.input.height= ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].height;
		}
		else if (ds_conf.dev_info[DE_OUT_DEV_ID_VGA].width && ds_conf.dev_info[DE_OUT_DEV_ID_VGA].height && ds_conf.dev_info[DE_OUT_DEV_ID_VGA].enable)
		{
			ds_conf.input.width = ds_conf.dev_info[DE_OUT_DEV_ID_VGA].width;
			ds_conf.input.height= ds_conf.dev_info[DE_OUT_DEV_ID_VGA].height;
		}
	}
	printf("ds_conf.input.width = %d ds_conf.input.height = %d\n",ds_conf.input.width,ds_conf.input.height);
#endif


#if ((AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
#if (MODULE_CONFIG_EZCASTPRO_MODE==8075) || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
	DE_INFO	de_info;
	int lcm = open("/dev/lcm",O_RDWR);
	if(lcm<0){
		printf("Config _lcm_init open lcm_drv Error\n");
		return -1;
	}
	if(ioctl(lcm,GET_DE_CONFIG,&de_info)<0){
		printf("Config de get config error\n");
		close(lcm);
		return -1;
	}
	INT32U format = de_info.screen_output_format;
	close(lcm);
	if ((ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].enable)\
		||((ds_conf.dev_info[DE_OUT_DEV_ID_VGA].enable)&&((format == VGA_1280_720)\
		||(format == VGA_1920_1080)||(format == VGA_1280_800)||(format == VGA_1024_768))))
#else
	if (ds_conf.dev_info[DE_OUT_DEV_ID_HDMI].enable)
#endif
#else
	printf("\n");
	printf("lcm_data.device_type=%d\n",lcm_data.device_type);
	printf("\n");
	if (lcm_data.device_type == 6)//E_LCDPADMODE_HDMI==6
#endif	
	{
		int dsp = open("/dev/DAC",O_RDWR);
		if(dsp<0){
			printf("Sorry Open DAC cfg Error\n");
			return -1;
		}
		ioctl(dsp,DACIO_SET_HDMI,1|HDMI_VOLUME_SW_EN);
		close(dsp);
	}
	de_set_Config(ds_inst,&ds_conf,DE_CFG_ALL);
	de_release(ds_inst);
	return 0;
}

static int _cec_op()
{
	int err,cec;
	char cmd[256];
	unsigned int physical_address;
	unsigned char logical_address;
	unsigned char buf[32];
	
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_cec.ko");
	err = system(cmd);
	
	cec = open("/dev/am7x-cec",O_RDWR);
	if(cec<0){
		printf("Sorry Open CEC cfg Error\n");
		return -1;
	}
	ioctl(cec,CEC_GET_PHYSICAL_ADDR,&physical_address);
	ioctl(cec,CEC_GET_SRC_ADDR,&logical_address);
	printf("physical_address=0x%x logical_address=%d\n",physical_address,logical_address);
	buf[0] = BROADCAST;
	buf[1] = CEC_OP_ACTIVE_SOURCE;
	buf[2] = physical_address >> 8;
	buf[3] = physical_address & 0xFF;
	write(cec,buf,4);		
	buf[0] = logical_address << 4 & 0xf0;;
	buf[1] = CEC_OP_IMAGE_VIEW_ON;
	write(cec,buf,2);
	buf[0] = logical_address << 4 & 0xf0;;
	buf[1] = CEC_OP_TEXT_VIEW_ON;
	write(cec,buf,2);
	close(cec);

	return 0;
	
}

static int calculate_wday(rtc_date_t *date)
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
	return 0;
}

static int _rtc_set_default()
{
	rtc_date_t rdate;
	rtc_time_t rtime;
	int err;

	rdate.year = 2010;
	rdate.month = 1;
	rdate.day = 1;
	calculate_wday(&rdate);

	rtime.hour = 0;
	rtime.min = 0;
	rtime.sec = 0;

	err = tm_set_rtc(&rdate,&rtime);
	if( err != 0){
		printf("rtc init error\n");
	}

	return err;
	
}

static int _apps_vram_init()
{	
	if(apps_vram_init()<0){
		/**
		* vram not initialized,set default.
		*/
		printf("set vram to default value\n");
		apps_vram_set_default();

		/**
		* may be the first time bootup after upgraded.
		*/
		_rtc_set_default();
	}
	apps_vram_release();

	return 0;
}

int main()
{
	int err;
	char cmd[256];
	
	printf("This is config.app'\n");
#if !defined(MODULE_CONFIG_EZCASTPRO_MODE) || (MODULE_CONFIG_EZCASTPRO_MODE==0)
	//Set printk level
	system("echo 4 > /proc/sys/kernel/printk");
#endif

	//Set watchdog
	system("sysctl -w kernel.panic=10");
	system("sysctl -w kernel.panic_on_oops=1");

	/*
	** Fix airplay mirror issue from softap mode.
	*/
#if (defined(MODULE_CONFIG_EZCAST_ENABLE) && MODULE_CONFIG_EZCAST_ENABLE!=0) && !(defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
	system("sysctl -w net.ipv4.route.max_size=8192");
	system("sysctl -w net.ipv4.route.gc_elasticity=4");
#endif
	
	/*******************************************
	*step 1. init embeded drivers 
	********************************************/
	init_gpio();
#if (AM_CHIP_MINOR == 8258 ||AM_CHIP_MINOR == 8268)
	set_gpio(4, 0);
#endif
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"amreg.ko");
	err = system(cmd);
	
#if MODULE_CONFIG_EZWIRE_ENABLE
	set_gpio(25, 0);	// Lighten LED
#endif

	#if MODULE_CONFIG_EZMUSIC_ENABLE
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_spdif.ko");
	err = system(cmd);	
	#endif
	/*******************************************
	*step 2. init module drivers
	*********************************************/
	/** 
	* insmod 2d 
	*/
	fui_insmod_2d();

#if 1
	/*Optimization of kernel network parameters*/
	sprintf(cmd,"%s%s/%s","sh ",AM_CASE_SC_DIR,"network_para.sh");
	err = system(cmd);
#endif

    /** install hantro decoder driver */
	sprintf(cmd,"%s%s/%s","sh ",AM_CASE_SC_DIR,"driver_load_hantro.sh");
	err = system(cmd);
    
#if ((AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
	FILE *fd;
	int ret;
	fd = fopen("/mnt/vram/edidinfo.bin", "r");
	if(!fd)
	{
		printf("NOTICE:open read_only mode /mnt/vram/edidinfo.bin ERROR\n");
		fd = fopen("/mnt/vram/edidinfo.bin", "w");
		if (!fd)
		{
			printf("NOTICE:open write_only mode /mnt/vram/edidinfo.bin ERROR\n");
		}
	}
	fclose(fd);
#endif
#if 1
	 /*
	*bluse add:switch usb otg status to host mode,
	*beacause am1213(8250) switch function :usb otg default in device mode
	  */
	if(AM_CHIP_ID == 1213){
		printf("swtich usb otg state device --->host\n");
		set_gpio(82,1);
		usleep(50);
		set_gpio(83,1);
	}
#endif	

#if MODULE_CONFIG_HDMI2MHL_ENABLE
	int fp;
	int val = 0;
	struct lcm_config lcm_data;
	fp = open(LCM_CONFIG_PATH, O_RDONLY);
	if(fp>=0)
	{
		read(fp,(char*)&lcm_data,sizeof(struct lcm_config));
		close(fp);
		printf("lcm_data.hdmi_mhl_gpio=%d\n",lcm_data.hdmi_mhl_gpio);
		if (lcm_data.hdmi_mhl_gpio == HDMI_HDMI2MHL_GPIO39)
		{
			get_gpio(HDMI_HDMI2MHL_GPIO39, &val);
			printf("HDMI_HDMI2MHL_GPIO39 = %d\n",val);			
		}
	}
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c-am7x.ko");
	err = system(cmd);
	sprintf(cmd,"%s%s/%s dependence=%d mhl_adopter_id=%d mhl_device_id=%d","insmod  ",AM_SYS_LIB_DIR,"am7x_it6681.ko",val,lcm_data.mhl_adopter_id,lcm_data.mhl_device_id);
	err = system(cmd);
	if (!val)
		OSSleep(3000);
#endif	
	/** we set all kinds of configurations via shell scripts */
	sprintf(cmd,"%s%s/%s","sh ",AM_CASE_SC_DIR,"am_syscfg.sh");
	err = system(cmd);
	
	
#if SYS_MOUSE_USED
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"mousedev.ko");
	err = system(cmd);
#endif

#if MODULE_CONFIG_EZMUSIC_ENABLE
	/*wm8988 depends on i2c_am7x.ko  must start after it */
	/*
	int I2S_EN=0;
	get_gpio(32, &I2S_EN);//I2S pin Enable to load it 
	if(I2S_EN==0)
	{
		printf("[%s,%d]insmod i2c_wm988.ko",__func__,__LINE__);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2c_wm8988.ko");
		err=system(cmd);
	}
	*/
	memset(cmd,0,sizeof(cmd));
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"i2s_i2c.ko");
	err = system(cmd);
#endif
	/**
	* make all device node.
	*/
	sprintf(cmd,"%s","mdev -s");
	err = system(cmd);
	if(err < 0){
		printf("config error : create device error\n");
	}
		
#if	0
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_psmouse.ko");
	err = system(cmd);
#endif
#if 0
	 /*
	*bluse add:switch usb otg status to host mode,
	*beacause am1213(8250) switch function :usb otg default in device mode
	  */
	if(1){
		printf("swtich usb otg state device --->host\n");
		set_gpio(82,1);
		usleep(50);
		set_gpio(83,1);
	}
#endif
	/** 
	* init the vram,note that you should release vram when you
	* exit a process.
	*/
	_apps_vram_init();

	/**
	* Init the default time for gettimeofday().
	* About around 2010.
	*/
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0
	memset(cmd,0,256);
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_uartcom.ko");	
	err=system(cmd);
	printf("return of insmod am7x_uartcom.ko==%d\n",err);
	
	memset(cmd,0,256);
	sprintf(cmd,"%s%s/%s","insmod  ",AM_SYS_LIB_DIR,"am7x_pipe.ko");	
	err=system(cmd);
	
	memset(cmd,0,256);
	system("lsmod");
	sprintf(cmd,"%s c %d %d","mknod /dev/uartcom",231,0);
	err = system(cmd);
	sprintf(cmd,"%s c %d %d","mknod /dev/am7x_pipe",234,0);
	err = system(cmd);
#endif

    /* Mos: Load latest date to avoid system time has big gap with real time */
    if( access( "/mnt/vram/date.txt", F_OK ) != -1 ) {
        system("date `cat /mnt/vram/date.txt`");
    }
    else if( access( "/usr/share/date_default.txt", F_OK ) != -1 ) {
        system("date `cat /usr/share/date_default.txt`");
    }
	/** change de configurations */
#if (EZWIRE_TYPE != 8 && EZWIRE_TYPE != 9&&EZWIRE_TYPE != 10)
		_lcm_init();
#else
		int dsp = open("/dev/DAC",O_RDWR);
		if(dsp<0){
			printf("Sorry Open DAC cfg Error\n");
			return -1;
		}
		ioctl(dsp,DACIO_SET_HDMI,1|HDMI_VOLUME_SW_EN);
		close(dsp);
#endif
	
/* Mos: Enable HDMI CEC one touch play only for ezcast,
 * Cus part of EZCastpro product has hardware issue */
#if	!defined(MODULE_CONFIG_EZCASTPRO_MODE)
	/** CEC one touch play */
	if(access(hdmi_cec_DISABLE, F_OK) != 0){
		_cec_op();  //on
	}
#endif

	system("sysctl -w net.core.rmem_max=1024000");
#if (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE != 0)
	system("ifconfig lo up");
#endif
	return 0;

}
