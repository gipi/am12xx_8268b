/* drivers/char/lcm/am7x-lcm.c
 *
 * Copyright (C) 2010 Actions MicroEletronics Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Declare platform resources and register platform devices for
 * AM7XXX.
 *
 * Description: lcd control modules driver
 *			 include :   ***DE***(display engine)
 *					  ***LCD controler***		
 *
 *  2010/03/24:   by  zengtao <scopengl@gmail.com>
 *					Version 1.0
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/am7x_mconfig.h>

#include "linux/uaccess.h" 
#include "linux/fcntl.h"
#include <linux/interrupt.h>
#include <linux/delay.h>

#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif

/*
 * Actions register definations and operations
*/
#include <actions_io.h>
#include <actions_regs.h>
#include "am7x_lcm.h"
#include "deapi.h"
#include "hdmi.h"
#include "am7x_board.h"
#include "am7x_gpio.h"

#if CONFIG_AM_CHIP_MINOR == 8268
#define SSL_S_END_BIT 7
#define SSL_S_START_BIT 1
#else
#define SSL_S_END_BIT 8
#define SSL_S_START_BIT 2
#endif


//#define LCM_DPB_USE_TIMER
#ifndef LCM_DPB_USE_TIMER
#define LCM_DPB_USE_IRQ
#endif

//#define LCM_DPB_DBG

#ifdef LCM_DPB_DBG
#define DPB_DEBUGP(x...) printk( KERN_DEBUG x )
#define DPB_INFO(x...)		printk(KERN_INFO x)
#else
#define DPB_DEBUGP(x...)
#define DPB_INFO(x...)	
#endif

#define DPB_DEBUG_INFO(x...) printk( KERN_DEBUG x )
#define DPB_PERR(x...) printk( KERN_ERR x )

#define VGA_HDMI_SYNC_MODE		0x10//VGA &HDMI output at sametime
/*
 * For EDID&DDC read support
*/
#include <linux/fb.h>
#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
extern unsigned char * edid_i2c_hw_read();
#ifndef MODULE_CONFIG_UCLIBC
extern unsigned char * edid_i2c_gpio_read();
#endif
#endif

#if(MODULE_CONFIG_DDR_TYPE==2)&& (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)

#if MODULE_CONFIG_EZWIRE_TYPE==10
#define NOT_SUPPORT_1080P 0
#else
#define NOT_SUPPORT_1080P 1
#endif

#else
#define NOT_SUPPORT_1080P 0
#endif
#if MODULE_CONFIG_HDMI2MHL_ENABLE
/*
* For HDMI2MHL it6681 support
*/
extern int it6681_read_edid( void *it6681_dev_data, void *pedid, unsigned short max_length);
#endif

struct am7x_display_driver_data{
	/**edid config information **/
	struct edid_config edid_config;
	/**user config information **/
	struct lcm_config user_config;
	/** user color config **/
	struct color_config user_color_config;
	/**** Display engine info *****/
	DE_INFO DeInfo;

	unsigned int lvds_ana_ctl2;
	/** lcm operation can't  **/
	struct semaphore lcm_sem;

	/**  current device infomation ***/
	unsigned char current_device;
	unsigned char current_format;

	/**** Display device EDID info *****/
	struct fb_monspecs monA;
	unsigned char edid_valid;
		
};

/* 
	Source Product Description (SPD) InfoFrame
	description in CEA-861B 6.2
*/
struct _SPD_InfoFrame {
	unsigned char	version;	/* Version */
	unsigned char	device;		/* Source Device Infomation */
	unsigned char	name[9];	/* Product Name (8 bytes) */
	unsigned char	desc[17];	/* product Description (16 bytes) */
};


#if MODULE_CONFIG_EZCAST_ENABLE
#if (defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=8251) \
	|| (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)		// For EZWire
static unsigned char edid_mode = EDID_MODE_FOR_STDBOARD;
#else
static unsigned char edid_mode = EDID_MODE_FOR_EZCAST;
#endif
#else
static unsigned char edid_mode = EDID_MODE_FOR_STDBOARD;
#endif
static struct am7x_display_driver_data lcm_data;

//hdmi/lvds/lcd/tcon/ypbpr/vga common module data
static struct am7x_lcd_controller_data lcd_controller_data;

struct am7x_de_isr_data  de_isr_data;

static unsigned char edid_hdmi_status = 0;
static unsigned char edid_vga_status = 0;
#define Is_VGA60P(x) (!(VGA_1920_1080_30P==x))
#define Is_HDMI60P(x) ((FMT_1650x750_1280x720_60P==x)||(FMT_1920x1080_60P==x)||(FMT_858x525_720x480_60P==x)||(FMT_1250x810_1024x768_60P==x)||(FMT_1280x800_60P==x)||(FMT_800x600_60P==x))
static int is_60p_flag=0;

#if (defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=0)
struct lcm_videomode hdmi_modes[] = {
	/* 2 FMT_2200x1125_1920x1080_30P */
	{ NULL, 30, 1920, 1080, LCM_VMODE_NONINTERLACED, 2},
	/* 4 FMT_2750x1125_1920x1080_24P */
	{ NULL, 24, 1920, 1080, LCM_VMODE_NONINTERLACED, 4},
	/* 5 FMT_1650x750_1280x720_60P */
	{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 5},
#if (MODULE_CONFIG_EZCASTPRO_MODE==8251) ||(MODULE_CONFIG_EZCASTPRO_MODE == 8075)
	/* 17 FMT_1920x1080_60P */
	{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED,17},//this should be fixed with the freq of CPU!!	
	/* 1 FMT_2080x741_1920x720_59P */
	{ NULL, 59, 1920, 720, LCM_VMODE_NONINTERLACED, 21},
	/* 16 FMT_1280x800_60P */
	{ NULL, 60, 1280, 800, LCM_VMODE_NONINTERLACED, 16},
	/* 15 FMT_1250x810_1024x768_60P */
	{ NULL, 60, 1024, 768, LCM_VMODE_NONINTERLACED, 15},
#endif	
};
struct lcm_videomode vga_modes[] = {
	/* 7 VGA_1366_768_60 */
	{ NULL, 60, 1366, 768, LCM_VMODE_NONINTERLACED, 7},
	/* 0 VGA_1280_1024_60 */
	{ NULL, 60, 1280, 1024, LCM_VMODE_NONINTERLACED, 0},
	/* 5 VGA_1280_800_60 */
	{ NULL, 60, 1280, 800, LCM_VMODE_NONINTERLACED, 5},
	/* 2 VGA_1024_768_72 */
	{ NULL, 72, 1024, 768, LCM_VMODE_NONINTERLACED, 2},
#if MODULE_CONFIG_EZCASTPRO_MODE == 8075
	/* 8 VGA_1280_720_60 */
	{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 8},	
	/* 6 VGA_1920_1080_60 */
	{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED, 6},
#endif
};

#elif (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)

struct lcm_videomode hdmi_modes[] = {
#if (MODULE_CONFIG_EZWIRE_TYPE!=0&&NOT_SUPPORT_1080P==0)
	/* 17 FMT_1920x1080_60P */
	{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED,17},//this should be fixed with the freq of CPU!!
#endif
	/* 2 FMT_2200x1125_1920x1080_30P */
	{ NULL, 30, 1920, 1080, LCM_VMODE_NONINTERLACED, 2},
	/* 4 FMT_2750x1125_1920x1080_24P */
	//{ NULL, 24, 1920, 1080, LCM_VMODE_NONINTERLACED, 4},
	/* 5 FMT_1650x750_1280x720_60P */
	{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 5},
};
struct lcm_videomode vga_modes[] = {
#if (MODULE_CONFIG_EZWIRE_TYPE!=0&&NOT_SUPPORT_1080P==0)
	/* 6 VGA_1920_1080_60 */
	{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED, 6},
#endif
	/* 5 VGA_1280_800_60 */
	{ NULL, 60, 1280, 800, LCM_VMODE_NONINTERLACED, 5},
	/* 8 VGA_1280_720_60 */
	{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 8},
};

#else
struct lcm_videomode hdmi_modes[] = {
	/* 17 FMT_1920x1080_60P */
//	{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED,17},
	/* 19 FMT_1920x1080_50P */
//	{ NULL, 50, 1920, 1080, LCM_VMODE_NONINTERLACED,19},
#ifndef MODULE_CONFIG_MIRASCREEN5G_ENABLE
	/* 2 FMT_2200x1125_1920x1080_30P */
	{ NULL, 30, 1920, 1080, LCM_VMODE_NONINTERLACED, 2},
	/* 3 FMT_2640x1125_1920x1080_25P */
	{ NULL, 25, 1920, 1080, LCM_VMODE_NONINTERLACED, 3},
	/* 4 FMT_2750x1125_1920x1080_24P */
	{ NULL, 24, 1920, 1080, LCM_VMODE_NONINTERLACED, 4},
	/* 0 FMT_2200x1125_1920x1080_60I */
	{ NULL, 60, 1920, 1080, LCM_VMODE_INTERLACED, 0},
	/* 1 FMT_2640x1125_1920x1080_50I */
	{ NULL, 50, 1920, 1080, LCM_VMODE_INTERLACED, 1},
#endif
	/* 1 FMT_2080x741_1920x720_59P */
	{ NULL, 59, 1920, 720, LCM_VMODE_NONINTERLACED, 21},
	/* 16 FMT_1280x800_60P */
	{ NULL, 60, 1280, 800, LCM_VMODE_NONINTERLACED, 16},
	/* 5 FMT_1650x750_1280x720_60P */
	{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 5},
	/* 6 FMT_3300x750_1280x720_30P */
	{ NULL, 30, 1280, 720, LCM_VMODE_NONINTERLACED, 6},
	/* 7 FMT_1980x750_1280x720_50P */
	{ NULL, 50, 1280, 720, LCM_VMODE_NONINTERLACED, 7},
	/* 8 FMT_3960x750_1280x720_25P */
	{ NULL, 25, 1280, 720, LCM_VMODE_NONINTERLACED, 8},
	/* 9 FMT_4125x750_1280x720_24P */
	{ NULL, 24, 1280, 720, LCM_VMODE_NONINTERLACED, 9},
	/* 15 FMT_1250x810_1024x768_60P */
	{ NULL, 60, 1024, 768, LCM_VMODE_NONINTERLACED, 15},
	/* 10 FMT_2304x1250_1920x1152_50I */
	{ NULL, 50, 1920, 1152, LCM_VMODE_INTERLACED, 10},
	/* 11 FMT_858x525_720x480_60I */
	{ NULL, 60, 720, 480, LCM_VMODE_INTERLACED, 11},
	/* 12 FMT_864x625_720x576_50I */
	{ NULL, 50, 720, 576, LCM_VMODE_INTERLACED, 12},
	/* 13 FMT_858x525_720x480_60P */
	{ NULL, 60, 720, 480, LCM_VMODE_NONINTERLACED, 13},
	/* 14 FMT_864x625_720x576_50P */
	{ NULL, 50, 720, 576, LCM_VMODE_NONINTERLACED, 14},
	/* 18 FMT_800x600_60P */
	{ NULL, 60, 800, 600, LCM_VMODE_NONINTERLACED, 18},	
};

struct lcm_videomode vga_modes[] = {	
	/* 6 VGA_1920_1080_60 */
	{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED, 6},
	/* 9 VGA_1920_1080_30 */
	{ NULL, 30, 1920, 1080, LCM_VMODE_NONINTERLACED, 9},
	/* 8 VGA_1280_720_60 */
	{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 8},	
	/* 7 VGA_1366_768_60 */
	{ NULL, 60, 1366, 768, LCM_VMODE_NONINTERLACED, 7},
	/* 0 VGA_1280_1024_60 */
	{ NULL, 60, 1280, 1024, LCM_VMODE_NONINTERLACED, 0},//????????
	/* 5 VGA_1280_800_60 */
	{ NULL, 60, 1280, 800, LCM_VMODE_NONINTERLACED, 5},
	/* 1 VGA_1280_768_85 */
	{ NULL, 85, 1280, 768, LCM_VMODE_NONINTERLACED, 1},//??????
	/* 2 VGA_1024_768_72 */
	{ NULL, 72, 1024, 768, LCM_VMODE_NONINTERLACED, 2},
	/* 3 VGA_800_600_85 */
	{ NULL, 85, 800, 600, LCM_VMODE_NONINTERLACED, 3},
	/* 4 VGA_640_480_60 */
	{ NULL, 60, 640, 480, LCM_VMODE_NONINTERLACED, 4},	
};
#endif


enum LCM_DPB_CTL_STATE{
	LCM_DPB_STOP= 0,
	LCM_DPB_RUN,
	LCM_DPB_PAUSE
};

typedef struct __lcm_dpb_ctl{
	LCM_DPB_FIFO_T *in_dpb_fifo;
	LCM_DPB_FIFO_T *out_dpb_fifo;
	LCM_DPB_T dpb_cache;
	LCM_DPB_T prev_dpb_cache;
	LCM_DPB_T prev_dpb_cache2;
	LCM_DPB_T *prev_dpb;
	LCM_DPB_T *prev_dpb2;
	LCM_DPB_T *cur_dpb;
	int dpb_mode;
	#ifdef LCM_DPB_USE_TIMER
	struct timer_list dpb_timer;
	#endif
	volatile int dpb_fifo_flag;
}LCM_DPB_CTL;
LCM_DPB_CTL g_dpb_ctl, *pdpb_ctl=&g_dpb_ctl;

static int fifo_instert_tail(LCM_DPB_FIFO_T * const pRing, LCM_DPB_T *dpb)
{
	int ret = -1;
	unsigned long		flags;

#ifdef LCM_DPB_DBG
	if(!dpb){
		DPB_PERR("[fifo_instert_tail] dpb is NULL!\n");
	}
#endif

	if(pRing->len < pRing->maxLen){
		#ifdef LCM_DPB_DBG
		DPB_DEBUGP("in Ring[%d]=%lld\n", pRing->end, dpb->pts);
		#endif
		spin_lock_irqsave(&pRing->lock, flags);
		if(pRing->len < pRing->maxLen){
			pRing->fifo[pRing->end] = *dpb;
			++pRing->len;
			pRing->end = (pRing->end+1)&(pRing->maxLen-1);
			ret = 0;
		}
		spin_unlock_irqrestore(&pRing->lock, flags);
	}

#ifdef LCM_DPB_DBG
	if(ret<0){
		DPB_PERR("[fifo_instert_tail] fail!\n");
	}
#endif
	
	return ret;
}


static LCM_DPB_T * fifo_pop_head(LCM_DPB_FIFO_T * const pRing)
{
	LCM_DPB_T *dpb = NULL;
	unsigned long		flags;
	
	if(pRing->len > 0){
		spin_lock_irqsave(&pRing->lock, flags);
		if(pRing->len > 0){
			dpb = &(pRing->fifo[pRing->start]);
			--pRing->len;
			pRing->start = (pRing->start+1)&(pRing->maxLen-1);
		}
		spin_unlock_irqrestore(&pRing->lock, flags);
	}

	return dpb;
}


static inline LCM_DPB_T * fifo_check_pop_head(LCM_DPB_FIFO_T * const pRing)
{
	if(pRing->len > 0){
		return &(pRing->fifo[pRing->start]);
	}else{
		return NULL;
	}
}


static inline LCM_DPB_T * fifo_check_data(LCM_DPB_FIFO_T * const pRing, int index)
{
	if((pRing->len > 0) && (index < pRing->len)){
		return &(pRing->fifo[(pRing->start+index)&(pRing->maxLen-1)]);//(pRing->start+index)%pRing->maxLen
	}else{
		return NULL;
	}
}


static inline void fifo_reset(LCM_DPB_FIFO_T * pRing)
{
	pRing->len = pRing->end = pRing->start = 0;
}




/**
*@brief API for  read  lcm config, store it into global  data  
*@param[in]    : none
*@param[out]  : none
*/
static inline void read_user_config(void)
{
#if 1
	int ret = 0;
	ret = am_get_config(LCM_CONFIG_PATH, (char*)&lcm_data.user_config, DATA_OFFSET, sizeof(struct lcm_config));
	if(ret < 0)
	{
		printk(KERN_ERR "NOTICE:READ LCD CONFIG FILE ERROR\n");
		printk("SYSTEM DIE TO PROTECT LCD HARDWARE\n");	
		panic("read lcm config file error\n");
	}

	ret = am_get_config(LCM_GAMMA_PATH,(char*)&lcm_data.user_color_config, DATA_OFFSET, sizeof(struct color_config));
	if(ret < 0)
	{
		printk(KERN_ERR "NOTICE:READ LCD Color CONFIG FILE ERROR\n");
	}

	ret = am_get_config(EDID_CONFIG_PATH,(char*)&lcm_data.edid_config, DATA_OFFSET, sizeof(struct edid_config));
	printk("@@@@@@@@@@@ret %d v %d h %d %d %d\n",ret,lcm_data.edid_config.vga_valid,lcm_data.edid_config.hdmi_valid,lcm_data.edid_config.vga_format,lcm_data.edid_config.hdmi_format);
	if(ret < 0)
	{
		printk(KERN_ERR "NOTICE:READ EDID CONFIG FILE ERROR\n");
	}
#else
	struct file *fp;
	mm_segment_t fs; 
	char* buf;
	
	fs = get_fs(); 
    set_fs(KERNEL_DS); 
	fp = filp_open(LCM_CONFIG_PATH, O_RDONLY, 0644);
	if (IS_ERR(fp)) { 
       	printk("open %s error\n",LCM_CONFIG_PATH); 
        return -1; 
    }
	vfs_llseek(fp,0,SEEK_END);
	
	buf = (char*)&lcm_config;
	
	vfs_llseek(fp,DATA_OFFSET,SEEK_SET);
	vfs_read(fp,(char*)buf,sizeof(lcm_config),&fp->f_pos);
	set_fs(fs);
	filp_close(fp,NULL);
#endif

}



/**
*@brief API for  close same thing before config hardware  in case system die 
*@param[in]    : none
*@param[out]  : none
*/
void start_config(void)
{
	de_set(DE_RELEASE,0); //RESET DE 
	//RESET LCD 
	//RESET HDMI 
    //RESET BT  
}

/**
*@brief API for  open same thing let system work 
*@param[in]    : none
*@param[out]  : none
*/
void end_config(void)
{
	RegBitSet(1,DE_Con,9,9);			//DE enable 1:enable,0:disable
	RegBitSet(1,CMU_LCDPLL,0,0);		//LCD PLL enable 1:enable,0:disable 
}

/**
*@brief API for mfp configuration for lcd  
*@param[in]    : none
*@param[out]  : none
*/
void lcd_mfp_config(void)
{
	int rgbformat = lcm_data.user_config.lcm_rgbformat;
	int lcd_type = lcm_data.user_config.device_type;

	switch(lcd_type)
	{
		case E_LCDPADMODE_LCD:  // 666 888
		#if CONFIG_AM_CHIP_ID == 1213
			//LVDS pad: set as TTL RX
			RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
			RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
			//LCD_D23,22,21,20,19
			RegBitSet(1,GPIO_MFCTL0,8,8);
			//LCD_D18
			RegBitSet(1,GPIO_MFCTL0,9,9);
			//LCD_D15
			RegBitSet(1,GPIO_MFCTL0,10,10);
			//LCD_D14,13,12
			RegBitSet(1,GPIO_MFCTL0,11,11);
			//LCD_D11,10
			RegBitSet(1,GPIO_MFCTL4,22,21);
			//LCD_D7,6,5,4,3
			RegBitSet(1,GPIO_MFCTL0,14,13);
			//LCD_D2
			RegBitSet(1,GPIO_MFCTL0,16,15);
			
			//LCD_ENB
			RegBitSet(1,GPIO_MFCTL0,1,0);
			//LCD_CLK
			RegBitSet(1,GPIO_MFCTL0,3,2);
			//LCD_VS
			RegBitSet(1,GPIO_MFCTL0,5,4);
			//LCD_HS
			RegBitSet(1,GPIO_MFCTL0,7,6);
			if(rgbformat==E_LCDOUTPUTFMT_24P){ //888
			//LCD_D9
				RegBitSet(1,GPIO_MFCTL0,27,26);
				//LCD_D8
				RegBitSet(1,GPIO_MFCTL0,29,28);
				//LCD_D1
				RegBitSet(1,GPIO_MFCTL0,19,18);
				//LCD_D0
				RegBitSet(1,GPIO_MFCTL0,21,20);
				//LCD_D16
				RegBitSet(1,GPIO_MFCTL0,25,24);
				//LCD_D17
				RegBitSet(1,GPIO_MFCTL0,23,22);
			}
		#else
			//LVDS pad: set as TTL RX
			RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
			RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
			//LCD_D23,22,21,20,19
			RegBitSet(1,GPIO_MFCTL0,17,16);
			//LCD_D18
			RegBitSet(1,GPIO_MFCTL0,21,20);
			//LCD_D15,14
			RegBitSet(1,GPIO_MFCTL0,25,24);
			//LCD_D13,12
			RegBitSet(1,GPIO_MFCTL0,29,28);
			//LCD_D11,10
			RegBitSet(1,GPIO_MFCTL1,0,0);
			//LCD_D7,6,5,4,3
			RegBitSet(1,GPIO_MFCTL1,5,4);
			//LCD_D2
			RegBitSet(1,GPIO_MFCTL1,9,8);
			
			//LCD_ENB
			RegBitSet(1,GPIO_MFCTL7,25,24);
			//LCD_CLK
			RegBitSet(1,GPIO_MFCTL7,29,28);
			//LCD_VS
			RegBitSet(1,GPIO_MFCTL8,1,0);
			//LCD_HS
			RegBitSet(1,GPIO_MFCTL8,5,4);
			if(rgbformat==E_LCDOUTPUTFMT_24P){ //888
			//LCD_D9
				RegBitSet(1,GPIO_MFCTL2,11,8);
				//LCD_D8
				RegBitSet(1,GPIO_MFCTL2,15,12);
				//LCD_D1
				RegBitSet(2,GPIO_MFCTL7,10,8);
				//LCD_D0
				RegBitSet(2,GPIO_MFCTL7,14,12);
				//LCD_D16
				RegBitSet(2,GPIO_MFCTL7,18,16);
				//LCD_D17
				RegBitSet(1,GPIO_MFCTL7,22,20);
			}
		#endif
			break; 
		case E_LCDPADMODE_TCON: //666

		#if CONFIG_AM_CHIP_ID == 1213
			//printk("NO SUPPORT FOR PANEL:E_LCDPADMODE_TCON \n");
		#else
			RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
			RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
			//LCD_D23,22,21,20,19
			RegBitSet(1,GPIO_MFCTL0,17,16);
			//LCD_D18
			RegBitSet(1,GPIO_MFCTL0,21,20);
			//LCD_D15,14
			RegBitSet(1,GPIO_MFCTL0,25,24);
			//LCD_D13,12
			RegBitSet(1,GPIO_MFCTL0,29,28);
			//LCD_D11,10
			RegBitSet(1,GPIO_MFCTL1,0,0);
			//LCD_D7,6,5,4,3
			RegBitSet(1,GPIO_MFCTL1,5,4);
			//LCD_D2
			RegBitSet(1,GPIO_MFCTL1,9,8);
			
			//LCD_ENB-->POL
			RegBitSet(2,GPIO_MFCTL7,25,24);
			//LCD_CLK-->CPH
			RegBitSet(2,GPIO_MFCTL7,29,28);
			//LCD_VS -->OEH
			RegBitSet(2,GPIO_MFCTL8,1,0);
			//LCD_HS -->OEV
			RegBitSet(2,GPIO_MFCTL8,5,4);
			
			//LCD_D1 -->STV
			RegBitSet(4,GPIO_MFCTL7,10,8);
			//LCD_D0 -->CKV
			RegBitSet(4,GPIO_MFCTL7,14,12);
			//LCD_D16 -->STH
			RegBitSet(3,GPIO_MFCTL7,18,16);
		#endif
			break;
		default:
			break;
	}
}

/**
*@brief API for reset mfp configuration for lcd  
*@param[in]    : none
*@param[out]  : none
*/
void lcd_mfp_reset(void)
{
	int rgbformat = lcm_data.user_config.lcm_rgbformat;
	int lcd_type = lcm_data.user_config.device_type;

	switch(lcd_type)
	{
		case E_LCDPADMODE_LCD:  // 666 888		
		#if CONFIG_AM_CHIP_ID == 1213
			//LVDS pad: set as TTL RX
			RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
			RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
			//LCD_D23,22,21,20,19
			RegBitSet(0,GPIO_MFCTL0,8,8);
			//LCD_D18
			RegBitSet(0,GPIO_MFCTL0,9,9);
			//LCD_D15
			RegBitSet(0,GPIO_MFCTL0,10,10);
			//LCD_D14,13,12
			RegBitSet(0,GPIO_MFCTL0,11,11);
			//LCD_D11,10
			RegBitSet(0,GPIO_MFCTL4,22,21);
			//LCD_D7,6,5,4,3
			RegBitSet(0,GPIO_MFCTL0,14,13);
			//LCD_D2
			RegBitSet(0,GPIO_MFCTL0,16,15);
			
			//LCD_ENB
			RegBitSet(0,GPIO_MFCTL0,1,0);
			//LCD_CLK
			RegBitSet(0,GPIO_MFCTL0,3,2);
			//LCD_VS
			RegBitSet(0,GPIO_MFCTL0,5,4);
			//LCD_HS
			RegBitSet(0,GPIO_MFCTL0,7,6);
			if(rgbformat==E_LCDOUTPUTFMT_24P){ //888
			//LCD_D9
				RegBitSet(0,GPIO_MFCTL0,27,26);
				//LCD_D8
				RegBitSet(0,GPIO_MFCTL0,29,28);
				//LCD_D1
				RegBitSet(0,GPIO_MFCTL0,19,18);
				//LCD_D0
				RegBitSet(0,GPIO_MFCTL0,21,20);
				//LCD_D16
				RegBitSet(0,GPIO_MFCTL0,25,24);
				//LCD_D17
				RegBitSet(0,GPIO_MFCTL0,23,22);
			}
		#else
			//LVDS pad: set as TTL RX
			RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
			RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
			//LCD_D23,22,21,20,19
			RegBitSet(0,GPIO_MFCTL0,17,16);
			//LCD_D18
			RegBitSet(0,GPIO_MFCTL0,21,20);
			//LCD_D15,14
			RegBitSet(0,GPIO_MFCTL0,25,24);
			//LCD_D13,12
			RegBitSet(0,GPIO_MFCTL0,29,28);
			//LCD_D11,10
			RegBitSet(0,GPIO_MFCTL1,0,0);
			//LCD_D7,6,5,4,3
			RegBitSet(0,GPIO_MFCTL1,5,4);
			//LCD_D2
			RegBitSet(0,GPIO_MFCTL1,9,8);
			
			//LCD_ENB
			RegBitSet(0,GPIO_MFCTL7,25,24);
			//LCD_CLK
			RegBitSet(0,GPIO_MFCTL7,29,28);
			//LCD_VS
			RegBitSet(0,GPIO_MFCTL8,1,0);
			//LCD_HS
			RegBitSet(0,GPIO_MFCTL8,5,4);
			if(rgbformat==E_LCDOUTPUTFMT_24P){ //888
			//LCD_D9
				RegBitSet(0,GPIO_MFCTL2,11,8);
				//LCD_D8
				RegBitSet(0,GPIO_MFCTL2,15,12);
				//LCD_D1
				RegBitSet(0,GPIO_MFCTL7,10,8);
				//LCD_D0
				RegBitSet(0,GPIO_MFCTL7,14,12);
				//LCD_D16
				RegBitSet(0,GPIO_MFCTL7,18,16);
				//LCD_D17
				RegBitSet(0,GPIO_MFCTL7,22,20);
			}
		#endif
			break; 
		case E_LCDPADMODE_TCON: //666
		#if CONFIG_AM_CHIP_ID == 1213
			//printk("NO SUPPORT FOR PANEL:E_LCDPADMODE_TCON \n");
		#else
			RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
			RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
			//LCD_D23,22,21,20,19
			RegBitSet(0,GPIO_MFCTL0,17,16);
			//LCD_D18
			RegBitSet(0,GPIO_MFCTL0,21,20);
			//LCD_D15,14
			RegBitSet(0,GPIO_MFCTL0,25,24);
			//LCD_D13,12
			RegBitSet(0,GPIO_MFCTL0,29,28);
			//LCD_D11,10
			RegBitSet(0,GPIO_MFCTL1,0,0);
			//LCD_D7,6,5,4,3
			RegBitSet(0,GPIO_MFCTL1,5,4);
			//LCD_D2
			RegBitSet(0,GPIO_MFCTL1,9,8);
			
			//LCD_ENB-->POL
			RegBitSet(0,GPIO_MFCTL7,25,24);
			//LCD_CLK-->CPH
			RegBitSet(0,GPIO_MFCTL7,29,28);
			//LCD_VS -->OEH
			RegBitSet(0,GPIO_MFCTL8,1,0);
			//LCD_HS -->OEV
			RegBitSet(0,GPIO_MFCTL8,5,4);
			
			//LCD_D1 -->STV
			RegBitSet(0,GPIO_MFCTL7,10,8);
			//LCD_D0 -->CKV
			RegBitSet(0,GPIO_MFCTL7,14,12);
			//LCD_D16 -->STH
			RegBitSet(0,GPIO_MFCTL7,18,16);
		#endif
			break;
		default:
			break;
	}

}

/**
*@brief API for mfp configuration for vga  
*@param[in]    : none
*@param[out]  : none
*/
void vga_mfp_config(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	//LCD_ENB_1
	RegBitSet(1,GPIO_MFCTL0,1,0);
	//LCD_CLK_1
	RegBitSet(1,GPIO_MFCTL0,3,2);
	//LCD_VS_1
	RegBitSet(1,GPIO_MFCTL0,5,4);
	//LCD_HS_1
	RegBitSet(1,GPIO_MFCTL0,7,6);
#else
	//LCD_ENB_1
	RegBitSet(0x1,GPIO_MFCTL0,1,0);
	//LCD_CLK_1
	RegBitSet(0x1,GPIO_MFCTL0,6,4);
	//LCD_VS_1
	RegBitSet(0x1,GPIO_MFCTL0,10,8);
	//LCD_HS_1
	RegBitSet(0x1,GPIO_MFCTL0,14,12);

	//LCD_ENB_2
	RegBitSet(0x1,GPIO_MFCTL7,25,24);
	//LCD_CLK_2
	RegBitSet(0x1,GPIO_MFCTL7,29,28);
	//LCD_VS_2
	RegBitSet(0x1,GPIO_MFCTL8,1,0);
	//LCD_HS_2
	RegBitSet(0x1,GPIO_MFCTL8,5,4);
#endif
}

/**
*@brief API for mfp reset for vga  
*@param[in]    : none
*@param[out]  : none
*/
void vga_mfp_reset(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	//LCD_ENB_1
	RegBitSet(0,GPIO_MFCTL0,1,0);
	//LCD_CLK_1
	RegBitSet(0,GPIO_MFCTL0,3,2);
	//LCD_VS_1
	RegBitSet(0,GPIO_MFCTL0,5,4);
	//LCD_HS_1
	RegBitSet(0,GPIO_MFCTL0,7,6);
#else
	// hsync,vsync,clk
	RegBitSet(0x0,GPIO_MFCTL0,1,0);
	RegBitSet(0x0,GPIO_MFCTL0,6,4);
	RegBitSet(0x0,GPIO_MFCTL0,10,8);
	RegBitSet(0x0,GPIO_MFCTL0,14,12);

	RegBitSet(0x0,GPIO_MFCTL7,25,24);
	RegBitSet(0x0,GPIO_MFCTL7,29,28);
	RegBitSet(0x0,GPIO_MFCTL8,1,0);
	RegBitSet(0x0,GPIO_MFCTL8,5,4);
#endif
}

/**
*@brief API for mfp configuration for bt  
*@param[in]    : none
*@param[out]  : none
*/
void bt_mfp_config(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	//LVDS pad: set as TTL RX
	RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
	RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
	
	//BTVE_D7,6,5,4,3
	RegBitSet(2,GPIO_MFCTL0,14,13);
	//BTVE_D2
	RegBitSet(2,GPIO_MFCTL0,16,15);
	//BTVE_D1
	RegBitSet(3,GPIO_MFCTL0,19,18);
	//BTVE_D0
	RegBitSet(3,GPIO_MFCTL0,21,20);
	
	//BTVE_CLK
	RegBitSet(3,GPIO_MFCTL0,3,2);
	//BTVE_VS
	RegBitSet(3,GPIO_MFCTL0,5,4);
	//BTVE_HS
	RegBitSet(3,GPIO_MFCTL0,7,6);
#else
	//LVDS pad: set as TTL RX
	RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
	RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
	
	//LCD_D7,6,5,4,3
	RegBitSet(2,GPIO_MFCTL1,5,4);
	//LCD_D2
	RegBitSet(2,GPIO_MFCTL1,9,8);
	//LCD_D1
	RegBitSet(3,GPIO_MFCTL7,10,8);
	//LCD_D0
	RegBitSet(3,GPIO_MFCTL7,14,12);
	
	//LCD_CLK
	RegBitSet(3,GPIO_MFCTL0,6,4);//CLK
	RegBitSet(3,GPIO_MFCTL0,10,8); //VS
	RegBitSet(3,GPIO_MFCTL0,14,12); //HS
#endif

}

/**
*@brief API for mfp reset for bt  
*@param[in]    : none
*@param[out]  : none
*/
void bt_mfp_reset(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	//LVDS pad: set as TTL RX
	RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
	RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
	
	//BTVE_D7,6,5,4,3
	RegBitSet(0,GPIO_MFCTL0,14,13);
	//BTVE_D2
	RegBitSet(0,GPIO_MFCTL0,16,15);
	//BTVE_D1
	RegBitSet(0,GPIO_MFCTL0,19,18);
	//BTVE_D0
	RegBitSet(0,GPIO_MFCTL0,21,20);
	
	//BTVE_CLK
	RegBitSet(0,GPIO_MFCTL0,3,2);
	//BTVE_VS
	RegBitSet(0,GPIO_MFCTL0,5,4);
	//BTVE_HS
	RegBitSet(0,GPIO_MFCTL0,7,6);
#else
	//LVDS pad: set as TTL RX
	RegBitSet(0x55555555,LVDS_PAD_CFG0,31,0);
	RegBitSet(0x55555555,LVDS_PAD_CFG1,31,0);
	
	//LCD_D7,6,5,4,3
	RegBitSet(0,GPIO_MFCTL1,5,4);
	//LCD_D2
	RegBitSet(0,GPIO_MFCTL1,9,8);
	//LCD_D1
	RegBitSet(0,GPIO_MFCTL7,10,8);
	//LCD_D0
	RegBitSet(0,GPIO_MFCTL7,14,12);
	
	//LCD_CLK
	RegBitSet(0,GPIO_MFCTL7,29,28);
	//LCD_VS
	RegBitSet(0,GPIO_MFCTL8,1,0);
	//LCD_HS
	RegBitSet(0,GPIO_MFCTL8,5,4);
#endif
}

/**
*@brief API for  init  lcd  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void lcd_controller_init(void)
{
	act_writel(lcd_controller_data.lcd_ctl,LCD_CTL);
	act_writel(lcd_controller_data.lcd_rgbctl,LCD_RGBCTL);
	act_writel(lcd_controller_data.lcd_size,LCD_SIZE);
	act_writel(lcd_controller_data.lcd_lsp,LCD_LSP);
	act_writel(lcd_controller_data.lcd_htiming,LCD_HTIMING);
	act_writel(lcd_controller_data.lcd_vtiming_odd,LCD_VTIMING_ODD);
	act_writel(lcd_controller_data.lcd_color,LCD_CLR);
	act_writel(lcd_controller_data.lcd_dac_ctl,LCD_DAC_CTL);
	act_writel(lcd_controller_data.lcd_vtiming_even,LCD_VTIMING_EVEN);
	act_writel(lcd_controller_data.lcd_vsync_edge,LCD_VSYNC_EDGE);
}

/**
*@brief API for   lcd  disable  and reset
*@param[in]    : none
*@param[out]  : none
*/
static void lcd_controller_exit(void)
{
	RegBitSet(0,LCD_RGBCTL,0,0);
	RegBitSet(0,CMU_DEVRST,11,11);
	RegBitSet(1,CMU_DEVRST,11,11);
}

/**
*@brief API for  init  lvds  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void lvds_controller_init(void)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	if(user_config->lvds_en){
		//lvds special clock
		act_writel(0x80,CMU_DISPLAYCLK2);
		act_writel(0x70,LVDS_ANA_CTL2);  

		act_writel(0xfdf83,LVDS_CTL);	
		RegBitSet(user_config->lvds_format,LVDS_CTL,1,1);
		RegBitSet(user_config->lvds_channel,LVDS_CTL,2,2);
		RegBitSet(user_config->lvds_swap,LVDS_CTL,5,5);
		RegBitSet(user_config->lvds_map,LVDS_CTL,4,3);	
		RegBitSet(user_config->lvds_mirror,LVDS_CTL,6,6);
		RegBitSet(4,0xb0010090,28,26);
	//	RegBitSet(1,LVDS_ANA_CTL1,16,16);
	//	RegBitSet(3,LVDS_ANA_CTL1,18,17);
	//	RegBitSet(1,LVDS_ANA_CTL1,21,19);
	//lvds need not set mfp,set lvds analog according to channel and format
		if((user_config->lvds_channel==0)&&(user_config->lvds_format==1)){//single 24bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(1,LVDS_ANA_CTL2,6,6);
			RegBitSet(0xaaaaa,LVDS_PAD_CFG0,19,0);
		
		}else if((user_config->lvds_channel==0)&&(user_config->lvds_format==0)){//single 18bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(0,LVDS_ANA_CTL1,10,8);
			RegBitSet(1,LVDS_ANA_CTL2,6,6);
			RegBitSet(0xaaaa,LVDS_PAD_CFG0,19,4);
		}else if((user_config->lvds_channel==1)&&(user_config->lvds_format==1)){//dual 24bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(3,LVDS_ANA_CTL2,7,6);
			RegBitSet(1,LVDS_ANA_CTL2,4,4);
			RegBitSet(0xaaaaa,LVDS_PAD_CFG0,19,0);
			RegBitSet(0xaaaaa,LVDS_PAD_CFG1,23,4);
			RegBitSet(1,CMU_DISPLAYCLK2,24,24);
		}else if((user_config->lvds_channel==1)&&(user_config->lvds_format==0)){//dual 18bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(3,LVDS_ANA_CTL2,7,6);
			RegBitSet(1,LVDS_ANA_CTL2,4,4);
			RegBitSet(0xaaaa,LVDS_PAD_CFG0,19,4);
			RegBitSet(0xaaaa,LVDS_PAD_CFG1,23,8);
			RegBitSet(1,CMU_DISPLAYCLK2,24,24);
		}else{
			printk("lvds para error!\n");
		}
		
		printk("LVDS_CTL:%x\n",act_readl(LVDS_CTL));
		printk("LVDS_ANA_CTL1:%x\n",act_readl(LVDS_ANA_CTL1));
		printk("LVDS_ANA_CTL2:%x\n",act_readl(LVDS_ANA_CTL2));
		printk("LVDS_PAD_CFG0:%x\n",act_readl(LVDS_PAD_CFG0));
		printk("CMU_DISPLAYCLK2:%x\n",act_readl(CMU_DISPLAYCLK2));		
	}

}

/**
*@brief API for  lvds  disable  
*@param[in]    : none
*@param[out]  : none
*/
static void lvds_controller_exit(void)
{
	RegBitSet(0,LVDS_CTL,0,0);	
}

/**
*@brief API for  init  tcon  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void tcon_controller_init(void)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	if(user_config->tcon_en)
	{
		int i;
		unsigned int *pGpos = &user_config->gp0_con;
		unsigned int iVideoSize = user_config->tcon_ls<<16|user_config->tcon_fs;
		act_writel(user_config->tcon_ctl,TCON_CON);		

		for (i=0;i<8;i++)
		{
			act_writel(*(pGpos + i*3),(GPO0_CON+ i*0x10));
			act_writel(*(pGpos + (i*3+1)),(GPO0_HP + i*0x10));
			act_writel(*(pGpos + (i*3+2)),(GPO0_VP + i*0x10));
		}
		act_writel(iVideoSize,VIDEO_SIZE);	
	}

}

/**
*@brief API for  tcon  close  
*@param[in]    : none
*@param[out]  : none
*/
static void tcon_controller_exit(void)
{
	RegBitSet(0,TCON_CON,0,0);
}


/**
*@brief API for  init  dac  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void dac_controller_init(void)
{
	RegBitSet(1,LCD_DAC_CTL,0,0);
}
/**
*@brief API for  close  dac  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void dac_controller_exit(void)
{
	RegBitSet(0,LCD_DAC_CTL,0,0);
}

/**
*@brief API for  init  bt  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void bt_controller_init(void)
{
	// fixme just for bt656 35d pannel
	act_writel(0x00000028,BTVE_CTL);			//BTVE_CTL
	act_writel(0x020c06b3,BTVE_SIZE);	
	act_writel(0x00800000,BTVE_HS);
	act_writel(0x06b30113,BTVE_HDE);
	act_writel(0x00030001,BTVE_VSODP);
	act_writel(0x010a0107,BTVE_VSEVEN);
	act_writel(0x01030012,BTVE_VDEODD);	
	act_writel(0x020a0119,BTVE_VDEEVEP);	
	act_writel(0x01070001,BTVE_FT);
	act_writel(0x00800000,BTVE_LSP);
	act_writel(0x00000029,BTVE_CTL);
	act_writel(0x00000029,BTVE_CTL);
	act_writel(0x00000029,BTVE_CTL);
	act_writel(0x00000029,BTVE_CTL);
	act_writel(0x00000029,BTVE_CTL);
	act_writel(0x00000029,BTVE_CTL);
	act_writel(0x00000029,BTVE_CTL);
	printk("bt ctl:%d\n",act_readl(BTVE_CTL));
	
}

/**
*@brief API for  close  bt  controller  
*@param[in]    : none
*@param[out]  : none
*/
static void bt_controller_exit(void)
{
	RegBitSet(0,BTVE_CTL,0,0);
}

/**
*@brief API for config display engine information 
*@param[in]    : none
*@param[out]  : none
*/
static void config_de_common(void)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 

	DeInfo->crop_ori_x = 1;
	DeInfo->crop_ori_y = 1;
	DeInfo->crop_width = DeInfo->input_width;
	DeInfo->crop_height = DeInfo->input_height;
	DeInfo->pip_ori_x = 1;
	DeInfo->pip_ori_y = 1;
	DeInfo->pip_frame_width = DeInfo->screen_width;
	DeInfo->pip_frame_height = DeInfo->screen_height;
	
	DeInfo->input_pix_fmt = PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;
	DeInfo->DisplaySource = DE_DEFAULTCOLOR;
	if (DeInfo->input_pix_fmt == PIX_FMT_RGB32)
		DeInfo->colorspace=CSC_OFF;
	/* Notice : display engine must be disabled before set */

	/*init gamma data*/
	DeInfo->GammaMode = lcm_data.user_color_config.gamma_mode;//  color_data.gamma_mode;
	DeInfo->Gamma = (unsigned int*)lcm_data.user_color_config.gamma;

	DeInfo->hue= lcm_data.user_color_config.hue;
	DeInfo->saturation = lcm_data.user_color_config.saturation;
	DeInfo->brightness = lcm_data.user_color_config.brightness;
	DeInfo->contrast = lcm_data.user_color_config.contrast;

	de_set(DE_INIT,DeInfo);

	
//	de_set(GAMMA_ADJUST,DeInfo);  //gamma adjust

}

/**
*@brief function for lcd gamma adjust information 
*@param[in]    : none
*@param[out]  : none
*/
void lcd_adjust_gamma(void)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 
	de_set(GAMMA_ADJUST,DeInfo);
}


/**
*@brief API for lcd config display engine information 
*@param[in]    width:  output width
*@param[out]  height: output height
*/
static void lcd_config_de(int width,int height)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 
	struct lcm_config *user_config = &lcm_data.user_config;

	DeInfo->screen_type=DE_OUT_DEV_ID_LCD;
	DeInfo->screen_width =	user_config->device_width;
    DeInfo->screen_height = user_config->device_height;

	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}
	if (E_LCDOUTPUTFMT_18P == user_config->lcm_rgbformat)
	{
		DeInfo->DitherMode = Dither_888To666;
	}
	else if (E_LCDOUTPUTFMT_16P == user_config->lcm_rgbformat)
	{
		DeInfo->DitherMode = Dither_888To565;
	}
	else 
	{	
		DeInfo->DitherMode = DITHER_OFF;
	}
	
	DeInfo->Mode = RGBMODE;
	
	if ( E_LCDPADMODE_CPU == user_config->device_type)
		DeInfo->Mode = CPUMODE;
	
	DeInfo->colorspace = SDTV_SRGB;
	DeInfo->DefaultColor = 0x0;  //default black	
	config_de_common();
}

/**
*@brief API for hdmi display engine config 
*@param[in]   width    : output width 
*			  height  : output height 
			 interlace: interlace mode or not  	
*@param[out]  : none
*/
static void hdmi_config_de(int width,int height,int interlace)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 
	
	DeInfo->colorspace=SDTV_SRGB;
	DeInfo->Mode = RGBMODE;
	DeInfo->screen_type=DE_OUT_DEV_ID_HDMI;
    DeInfo->screen_height = height;
	DeInfo->screen_width = width;
	DeInfo->DitherMode = DITHER_OFF;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;
	if(interlace == 1){
		DeInfo->Mode=RGBMODE_I;
		DeInfo->screen_height = DeInfo->screen_height*2;
	}
		
	
	if(DeInfo->input_width ==0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}	
	DeInfo->DefaultColor = 0x0;  //default black
	config_de_common();

}

/**
*@brief API for ypbpr display engine config 
*@param[in]   width    : output width 
*			  height  : output height 
			 interlace: interlace mode or not  	
*@param[out]  : none
*/
static void ypbpr_config_de(int width,int height,int interlace)
{	
	DE_INFO *DeInfo = &lcm_data.DeInfo; 

	DeInfo->colorspace= CSC_OFF;
	DeInfo->Mode= RGBMODE;	
	DeInfo->screen_type = DE_OUT_DEV_ID_YPBPR;
	DeInfo->screen_height = height;
	DeInfo->screen_width = width;
	DeInfo->DitherMode = DITHER_OFF;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;	

	if(interlace == 1){
		DeInfo->Mode = RGBMODE_I;
		DeInfo->screen_height = DeInfo->screen_height*2;
	}
	
	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}
	DeInfo->DefaultColor = 0x108080;  //default black	
	config_de_common();

}

/**
*@brief API for cvbs config display engine information 
*@param[in]    width:  output width
*@param[out]  height: output height
*/
static void cvbs_config_de(int width,int height)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 

	DeInfo->colorspace= CSC_OFF;
	DeInfo->Mode= BTMODE_I;
	DeInfo->DitherMode = DITHER_OFF;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;

	DeInfo->screen_type = DE_OUT_DEV_ID_CVBS;
	DeInfo->screen_width = width;
	DeInfo->screen_height = height; 

	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}	
	DeInfo->DefaultColor = 0x108080;  //default black	
	config_de_common();		
}


/**
*@brief API for vga config display engine information 
*@param[in]    width:  output width
*@param[out]  height: output height
*/
static void vga_config_de(int width,int height)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 

	DeInfo->colorspace= SDTV_SRGB;
	DeInfo->Mode = RGBMODE;	
	DeInfo->screen_type = DE_OUT_DEV_ID_VGA;
	DeInfo->screen_height = height;
	DeInfo->screen_width = width;
	DeInfo->DitherMode = DITHER_OFF;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;	
	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}
	DeInfo->DefaultColor = 0x0;  //default black
	config_de_common();
}

/**
*@brief API for bt656 config display engine information 
*@param[in]    width:  output width
*@param[out]  height: output height
*/
static void bt_config_de(int width,int height)
{
	DE_INFO *DeInfo = &lcm_data.DeInfo; 
		
	DeInfo->colorspace= CSC_OFF;
	DeInfo->Mode= BTMODE_I;
	DeInfo->DitherMode = DITHER_OFF;
	DeInfo->screen_type = DE_OUT_DEV_ID_CVBS;
	DeInfo->screen_width = width;
	DeInfo->screen_height = height;

	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}
	DeInfo->DefaultColor = 0x108080;  //default black	
	config_de_common();
}

/**
*@brief API for lcd clock config  
*@param[in]   format:  nosense
*@param[out] :  none
*/
static void lcd_config_clock(int format)
{
	int clk = lcm_data.user_config.display_clk;
	unsigned short ssl,div;
	struct lcm_config *user_config = &lcm_data.user_config;
	

//	if(user_config->device_type == E_LCDPADMODE_LCD)
	//FIXME 
	{	
		act_writel(0x780,CMU_DISPLAYCLK);
		act_writel(0x53,CMU_LCDPLL);  //3
#if CONFIG_AM_CHIP_MINOR == 8268
		act_writel(0x2529,CMU_LCDPLL);  
#endif		
	}	
	

	if(user_config->lvds_en)
	{
#if CONFIG_AM_CHIP_ID == 1213
		RegBitSet(0x81,CMU_LCDPLL,8,0);
#else
  		act_writel(0x13d52081,CMU_LCDPLL);//50MHz
#endif 
  		act_writel(0x10d52081,CMU_LCDPLL);//50MHz
#if CONFIG_AM_CHIP_MINOR == 8268
		act_writel(0x2541,CMU_LCDPLL);	//3
#endif		
  		act_writel(0x00000081,CMU_DISPLAYCLK);	
	}

	if(user_config->tcon_en)
	{
		RegBitSet(1,CMU_DISPLAYCLK,12,12);  //enable tcon d clock
	}

	if(clk<=0||clk>=MAX_CLK_RATE)
	{
		clk = DEFAULT_CLK;
	}

	if(clk >95)
	{
		ssl = (clk/2)*3;  //clk/1.5   
		div = 1;
	}
	else
	{
		for(div = 2;;div +=2)
			if(div*clk > 95)
				break;
		ssl = (div*clk*2)/3;	
	}

	RegBitSet(ssl,CMU_LCDPLL,8,2);
#if CONFIG_AM_CHIP_MINOR == 8268
	RegBitSet(ssl,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT);
#endif
	RegBitSet(div-1,CMU_DISPLAYCLK,3,0);

	//OPEN ALL DISPLAY CLOCK HERE FIXME
//	RegBitSet(63,CMU_DISPLAYCLK,12,7);
	if(user_config->video_clk){
#if CONFIG_AM_CHIP_ID == 1213		
		RegBitSet(((user_config->video_clk-1)%4),CMU_LCDPLL,23,22);
#else
		RegBitSet(((user_config->video_clk-1)%16),CMU_LCDPLL,25,22);
#endif
#if CONFIG_AM_CHIP_MINOR != 8268
		RegBitSet(1,CMU_LCDPLL,1,1);
#endif
	}

	
	printk("CMU_LCDPLL:%x\n",act_readl(CMU_LCDPLL));
	printk("CMU_DISPLAYCLK:%x\n",act_readl(CMU_DISPLAYCLK));
	printk("clock rate:%dMhz  divider:%d\n",(ssl*3)/2,div);

}

/**
*@brief API for hdmi clock config  
*@param[in]   format:  hdmi timing format
*@param[out] :  none
*/
static void hdmi_config_clock(int format)
{
	int clk = lcm_data.user_config.display_clk;
	unsigned short ssl,div;
	struct lcm_config *user_config = &lcm_data.user_config;

			if(user_config->video_clk){
#if CONFIG_AM_CHIP_ID == 1213		
		RegBitSet(((user_config->video_clk-1)%4),CMU_LCDPLL,23,22);
#else
		RegBitSet(((user_config->video_clk-1)%16),CMU_LCDPLL,25,22);
#endif
#if CONFIG_AM_CHIP_MINOR != 8268
		RegBitSet(1,CMU_LCDPLL,1,1);
#endif
	}
			
	if(format != FMT_USER_DEFINED)
		return ;  // only config clock for user defined 
	
	if(clk<=0||clk>=MAX_CLK_RATE)
	{
		clk = DEFAULT_CLK;
	}
	
	if(clk >50)
	{
		ssl = (clk/2)*3;  //clk/1.5   
		div = 1;
	}
	else
	{
		for(div = 2;;div +=2)
			if(div*clk > 50)
				break;
		ssl = (div*clk*2)/3;	
	}
	
	RegBitSet(ssl,CMU_LCDPLL,8,2);
#if CONFIG_AM_CHIP_MINOR == 8268
	RegBitSet(ssl,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT);
#endif
	RegBitSet(div-1,CMU_DISPLAYCLK,3,0);
			//OPEN ALL DISPLAY CLOCK HERE FIXME
	//	RegBitSet(63,CMU_DISPLAYCLK,12,7);

	RegBitSet(0,0xb0050098,0,0); //set to dvi mode 
			
	printk("CMU_LCDPLL:%x\n",act_readl(CMU_LCDPLL));
	printk("CMU_DISPLAYCLK:%x\n",act_readl(CMU_DISPLAYCLK));
	printk("clock rate:%dMhz  divider:%d\n",(ssl*3)/2,div);

	// do nothing now , clock is enabled inside hdmi module
}

/**
*@brief API for ypbpr clock config  
*@param[in]   format:  ypbpr timing format
*@param[out] :  none
*/
static void ypbpr_config_clock(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	act_writel(act_readl(CMU_DEVCLKEN)|1<<17,CMU_DEVCLKEN); // enable dac

	if(format == YPBPR_1080P)
	{        
#if CONFIG_AM_CHIP_ID == 1213
		RegBitSet(0x18d,CMU_LCDPLL,8,0);
#if CONFIG_AM_CHIP_MINOR == 8268
	RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d5218d,CMU_LCDPLL);
#endif

		act_writel(0x00001080,CMU_DISPLAYCLK);
	}
	else if(format == YPBPR_720P)
	{
#if CONFIG_AM_CHIP_ID == 1213
		RegBitSet(0x18d,CMU_LCDPLL,8,0);
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d5218d,CMU_LCDPLL);
#endif
		act_writel(0x00001081,CMU_DISPLAYCLK);
	}
	else if(format == YPBPR_480I||format == YPBPR_576I)
	{
#if CONFIG_AM_CHIP_ID == 1213
		RegBitSet(0x121,CMU_LCDPLL,8,0);
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x121/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d52121,CMU_LCDPLL);
#endif
		act_writel(0x00001087,CMU_DISPLAYCLK);
	}
	else if(format == YPBPR_480P||format == YPBPR_576P)
	{
#if CONFIG_AM_CHIP_ID == 1213
		RegBitSet(0x121,CMU_LCDPLL,8,0);
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x121/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d52121,CMU_LCDPLL);
#endif
		act_writel(0x00001083,CMU_DISPLAYCLK);           
	}

	if(user_config->video_clk){
#if CONFIG_AM_CHIP_ID == 1213		
		RegBitSet(((user_config->video_clk-1)%4),CMU_LCDPLL,23,22);
#else
		RegBitSet(((user_config->video_clk-1)%16),CMU_LCDPLL,25,22);
#endif
#if CONFIG_AM_CHIP_ID != 1213		
		RegBitSet(1,CMU_LCDPLL,1,1);
#endif
	}

}

/**
*@brief API for cvbs clock config  
*@param[in]   format:  cvbs timing format 
*@param[out] :  none
*/
static void cvbs_config_clock(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	RegBitSet(1,CMU_DEVCLKEN,12,12);  //enable btve clock
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
	RegBitSet(0x121,CMU_LCDPLL,8,0);
#if CONFIG_AM_CHIP_MINOR == 8268
	RegBitSet(0x121/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
	act_writel(0x13d52121,CMU_LCDPLL);	//pll: 72*1.5M = 108M
#endif	
	act_writel(0x00008183,CMU_DISPLAYCLK);  //div:4   27M

	if(user_config->video_clk){
#if CONFIG_AM_CHIP_ID == 1213		
	RegBitSet(((user_config->video_clk-1)%4),CMU_LCDPLL,23,22);
#else
	RegBitSet(((user_config->video_clk-1)%16),CMU_LCDPLL,25,22);
#endif
#if CONFIG_AM_CHIP_MINOR != 8268
	RegBitSet(1,CMU_LCDPLL,1,1);
#endif
	}
	udelay(100);
}

/**
*@brief API for vga clock config  
*@param[in]   format:  vga timing format
*@param[out] :  none
*/
static void vga_config_clock(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	if(format == VGA_1280_1024 ){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x121,CMU_LCDPLL,8,0);//108M~~60HZ
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x121/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x13d52121,CMU_LCDPLL);	 //108M~~60HZ
#endif		
		act_writel(0x00001080,CMU_DISPLAYCLK);
	}
		else if(format == VGA_1280_800_71M){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x17d,CMU_LCDPLL,8,0);//71M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x17d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x13d5217d,CMU_LCDPLL);	//71M~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
		
	}
	else if(format == VGA_1280_800){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x1bd,CMU_LCDPLL,8,0);//83.5M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x1bd/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x13d521bd,CMU_LCDPLL);	//83.5M~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
		
	}
	else if(format == VGA_1920_1080){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x18d,CMU_LCDPLL,8,0);//148.5M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x10d5218d,CMU_LCDPLL);	//148.5M~~60hz
#endif		
		act_writel(0x00001080,CMU_DISPLAYCLK);
	}
	else if(format == VGA_1920_1080_30P){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x18d,CMU_LCDPLL,8,0);//74.25~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x10d5218d,CMU_LCDPLL);	//74.25~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
	}
	else if(format == VGA_1366_768){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0xc1,CMU_LCDPLL,8,0);//72M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0xc1/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x10d520c1,CMU_LCDPLL);	//72M~~60hz
#endif		
		act_writel(0x00001080,CMU_DISPLAYCLK);
	}
else if(format == VGA_1366_768_85M){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x1c9,CMU_LCDPLL,8,0);//85.5~~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x1c9/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x10d521c9,CMU_LCDPLL);	//85.5~~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
	}
	else if(format == VGA_1280_768){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x16d,CMU_LCDPLL,8,0);//68.25~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x16d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x13d5216d,CMU_LCDPLL);	//68.25~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
		
	}
	else if(format == VGA_1280_768_79M){
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0xd5,CMU_LCDPLL,8,0);//79.5M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0xd5/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else 
		act_writel(0x13d520d5,CMU_LCDPLL);//79.5M~~60hz
#endif		
		act_writel(0x00001080,CMU_DISPLAYCLK);
		
	}
	else if(format == VGA_1024_768)     
	{
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x15d,CMU_LCDPLL,8,0);//65M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x15d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d5215d,CMU_LCDPLL);//65M~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
	}
	else if(format == VGA_800_600)
	{
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x141,CMU_LCDPLL,8,0);//40M~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x141/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d52141,CMU_LCDPLL);//40M~~60hz
#endif
		act_writel(0x00001082,CMU_DISPLAYCLK);

	}
	else if(format == VGA_640_480)
	{
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0xa9,CMU_LCDPLL,8,0);//60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0xa9/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d520a9,CMU_LCDPLL);	 //60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
	}
	else if(format == VGA_1280_720)
	{
#if (CONFIG_AM_CHIP_ID == 1213) || (CONFIG_AM_CHIP_ID == 1220)
		RegBitSet(0x18d,CMU_LCDPLL,8,0);//74.25~~60hz
#if CONFIG_AM_CHIP_MINOR == 8268
		RegBitSet(0x18d/2|1,CMU_LCDPLL,SSL_S_END_BIT,SSL_S_START_BIT-1);
#endif
#else
		act_writel(0x13d52195,CMU_LCDPLL);//74.25~~60hz
#endif		
		act_writel(0x00001081,CMU_DISPLAYCLK);
	}

		if(user_config->video_clk){
#if CONFIG_AM_CHIP_ID == 1213		
		RegBitSet(((user_config->video_clk-1)%4),CMU_LCDPLL,23,22);
#else
		RegBitSet(((user_config->video_clk-1)%16),CMU_LCDPLL,25,22);
#endif
#if CONFIG_AM_CHIP_MINOR != 8268
		RegBitSet(1,CMU_LCDPLL,1,1);
#endif
	}
}

/**
*@brief API for bt clock config  
*@param[in]   format:  nosense
*@param[out] :  none
*/
static void bt_config_clock(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	RegBitSet(1,CMU_DISPLAYCLK,8,8);
	/*BTVD_CLKEN*/
	RegBitSet(1,CMU_DISPLAYCLK,11,11);	
	act_writel(0x0000016d,CMU_LCDPLL);	 //bd	   
#if CONFIG_AM_CHIP_MINOR == 8268
	act_writel(0x000025b7,CMU_LCDPLL);
#endif
	act_writel(0x00000184,CMU_DISPLAYCLK);	

		if(user_config->video_clk){
#if CONFIG_AM_CHIP_ID == 1213		
		RegBitSet(((user_config->video_clk-1)%4),CMU_LCDPLL,23,22);
#else
		RegBitSet(((user_config->video_clk-1)%16),CMU_LCDPLL,25,22);
#endif
#if CONFIG_AM_CHIP_MINOR != 8268
		RegBitSet(1,CMU_LCDPLL,1,1);
#endif
	}
}

/**
*@brief API for init lcd de data structure do not operate hardware registers 
*@param[in] format : nosense
*@param[out]  : none
*/
void init_lcd_data(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	DE_INFO *DeInfo = &lcm_data.DeInfo; 

	lcd_controller_data.lcd_htiming = user_config->hbp|user_config->hfp<<11|user_config->hswp<<22;
	lcd_controller_data.lcd_vtiming_odd = user_config->vbp|user_config->vfp<<11|user_config->vswp<<22;
	lcd_controller_data.lcd_vtiming_even = 0;
	lcd_controller_data.lcd_ctl =0x4;
	lcd_controller_data.lcd_color = 0xff;
	lcd_controller_data.lcd_lsp = 0x20001;
	lcd_controller_data.lcd_rgbctl = user_config->rgb_ctl;
	lcd_controller_data.lcd_dac_ctl = user_config->dac_ctl;
	lcd_controller_data.lcd_size = (user_config->device_width-1)|(user_config->device_height-1)<<16;
	lcd_controller_data.lcd_vsync_edge = 0;
	
	DeInfo->screen_width =	user_config->device_width;
    DeInfo->screen_height = user_config->device_height;

	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}
	if (E_LCDOUTPUTFMT_18P == user_config->lcm_rgbformat)
	{
		DeInfo->DitherMode = Dither_888To666;
	}
	else if (E_LCDOUTPUTFMT_16P == user_config->lcm_rgbformat)
	{
		DeInfo->DitherMode = Dither_888To565;
	}
	else 
	{	
		DeInfo->DitherMode = DITHER_OFF;
	}
	
	DeInfo->Mode = RGBMODE;
	
	if ( E_LCDPADMODE_CPU == user_config->device_type)
		DeInfo->Mode = CPUMODE;
	
	DeInfo->colorspace = SDTV_SRGB;
	DeInfo->DefaultColor = 0x0;  //default black	

	DeInfo->crop_ori_x = 1;
	DeInfo->crop_ori_y = 1;
	DeInfo->crop_width = DeInfo->input_width;
	DeInfo->crop_height = DeInfo->input_height;
	DeInfo->pip_ori_x = 1;
	DeInfo->pip_ori_y = 1;
	DeInfo->pip_frame_width = DeInfo->screen_width;
	DeInfo->pip_frame_height = DeInfo->screen_height;
	
	DeInfo->input_pix_fmt = PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;
	DeInfo->DisplaySource = DE_DEFAULTCOLOR;

	lcd_mfp_config();// only config mfp

//	lcd_adjust_gamma();
}

/**
*@brief API for set lcd panel controller interface 
*@param[in] format : nosense
*@param[out]  : none
*/
void init_lcd(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;

	lcd_controller_data.lcd_htiming = user_config->hbp|user_config->hfp<<11|user_config->hswp<<22;
	lcd_controller_data.lcd_vtiming_odd = user_config->vbp|user_config->vfp<<11|user_config->vswp<<22;
	lcd_controller_data.lcd_vtiming_even = 0;
	lcd_controller_data.lcd_ctl =0x4;
	lcd_controller_data.lcd_color = 0xff;
	lcd_controller_data.lcd_lsp = 0x20001;
	lcd_controller_data.lcd_rgbctl = user_config->rgb_ctl;
	lcd_controller_data.lcd_dac_ctl = user_config->dac_ctl;
	lcd_controller_data.lcd_size = (user_config->device_width-1)|(user_config->device_height-1)<<16;
	lcd_controller_data.lcd_vsync_edge = 0;

	start_config();		

	lcd_config_de(0,0);  //arg is not need now
	
	lcd_controller_init();


	if(user_config->lvds_en)
		lvds_controller_init();
	if(user_config->tcon_en)
		tcon_controller_init();

	if(user_config->lvds_en == 0) //need mfp configuration
		lcd_mfp_config();

	lcd_config_clock(0); //format is ignore 


	/**just for gpio 7555 demo board  FIXME**/
	
	if(user_config->power_gpio!=255)
		am_set_gpio(user_config->power_gpio,1);
	
	end_config();
	
	lcd_adjust_gamma(); //gamma should be set when lcdpll is enable

}
 int lcm_read_ddrtype()
{
	int ret=0,value=0;
	#if ((CONFIG_AM_CHIP_MINOR == 8258 || CONFIG_AM_CHIP_MINOR == 8268)&&(MODULE_CONFIG_EZWIRE_TYPE==10))
       value=RegBitRead(0xb00100b4,6,0);
	printk("val=%d \n",value);
	value=value*8;
	if(value<992)
	{
		ret=2;   //DDR2

	}
	else 
	{
		ret=3;	//DDR3
	}
       #endif
	
	return ret;
}
#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
int edid_is_change(unsigned char * edid,struct fb_monspecs *monA,unsigned char current_device)
{
	int format = -1;
	struct file *fp=NULL;
	mm_segment_t fs; 
	unsigned char edidinfo[EDID_LENGTH];
	int ret;
	int i;
	if ((edid) && (monA))
	{
		fb_edid_to_monspecs(edid, monA);
		if (monA->extensionflag >=1)
			fb_edid_add_monspecs(edid+EDID_LENGTH,monA);
		/*for (i=0;i<128*(monA->extensionflag+1);i++)
		{
			if (i%16==0)
				printk("\n");
			printk("0x%x ",*(edid+i));
		}
		printk("\n111111111111111111111111111111111111111111111\n");*/
		fs = get_fs(); 
		set_fs(KERNEL_DS);
		//for EDID device ID 
		fp = filp_open(EDID_INFO_PATH, O_RDONLY, 0644);
		if (IS_ERR(fp)) { 
	       		printk("open /mnt/vram/edidinfo.bin error\n"); 
			return -1;
		}
		ret = vfs_read(fp,edidinfo,EDID_LENGTH, &fp->f_pos);
		filp_close(fp,NULL);
		set_fs(fs);
		/*printk("@@@@@@@@@@@@@@@@@@ret=%d\n",ret);
		if (ret==EDID_LENGTH)
		{
			for (i=0;i<EDID_LENGTH;i++)
			{
				if (i%8==0)
					printk("\n");
				printk("0x%x ",edidinfo[i]);
			}
			printk("\n22222222222222222222222222222222222222222222\n");
		}
		else
		{
			printk("read /mnt/vram/edidinfo.bin error\n");
		}*/
		if ((ret==EDID_LENGTH) && (memcmp(edid,edidinfo,EDID_LENGTH)==0))
		{
			if ((current_device == E_LCDPADMODE_HDMI) && (lcm_data.edid_config.hdmi_valid) && (lcm_data.edid_config.hdmi_format < FMT_USER_DEFINED) && (lcm_data.edid_config.hdmi_format >= 0))
			{
				format = lcm_data.edid_config.hdmi_format;
				printk("EZ HDMI usedefined format = %d\n",format);
				
			}
			else if ((current_device == E_LCDPADMODE_VGA) && (lcm_data.edid_config.vga_valid) && (lcm_data.edid_config.vga_format < VGA_BUTT) && (lcm_data.edid_config.vga_format >= 0))
			{
				format = lcm_data.edid_config.vga_format;
				printk("EZ VGA usedefined format = %d\n",format);
			}
			//printk("%s %d format=%d\n",__FILE__,__LINE__,format);
			fs = get_fs(); 
			set_fs(KERNEL_DS);
			//for EDID device ID 
			fp = filp_open(EDID_INFO_PATH, O_WRONLY, 0644);
			if (IS_ERR(fp)) { 
		       		printk("open /mnt/vram/edidinfo.bin error\n"); 
				return -1;
			}
			ret = vfs_write(fp,edid,EDID_LENGTH*(monA->extensionflag+1),&fp->f_pos);
			do_fsync(fp,0);
			filp_close(fp,NULL);
			return format;
		}
		else
		{
			fs = get_fs(); 
			set_fs(KERNEL_DS);
			//for EDID device ID 
			fp = filp_open(EDID_INFO_PATH, O_WRONLY, 0644);
			if (IS_ERR(fp)) { 
		       		printk("open /mnt/vram/edidinfo.bin error\n"); 
				return -1;
			}
			ret = vfs_write(fp,edid,EDID_LENGTH*(monA->extensionflag+1),&fp->f_pos);
			//do_fsync(fp,0);
			filp_close(fp,NULL);

			/*printk("####################ret=%d\n",ret);
			fp = filp_open(EDID_INFO_PATH, O_RDONLY, 0644);
			if (IS_ERR(fp)) { 
		       		printk("open /mnt/vram/edidinfo.bin error\n"); 
				ret = -ENOENT;
				goto edid_match;
			}
			ret = vfs_read(fp,edidinfo,EDID_LENGTH, &fp->f_pos);
			filp_close(fp,NULL);
			printk("@@@@@@@@@@@@@@@@@@ret=%d\n",ret);
			for (i=0;i<128;i++)
			{
				if (i%8==0)
					printk("\n");
				printk("0x%x ",edidinfo[i]);
			}
			printk("\n333333333333333333333333333333333333333333333\n");*/

			fp = filp_open(EDID_CONFIG_PATH, O_WRONLY, 0644);
			if (IS_ERR(fp)) { 
		       		printk("open /mnt/vram/edid.bin error\n"); 
				return -1;
			}
			lcm_data.edid_config.hdmi_valid =0;
			lcm_data.edid_config.hdmi_format =0;
			lcm_data.edid_config.vga_valid =0;
			lcm_data.edid_config.vga_format =0;
			vfs_llseek(fp,EZ_OFFSET,SEEK_SET);
			ret = vfs_write(fp,&lcm_data.edid_config,sizeof(struct edid_config),&fp->f_pos);
			//do_fsync(fp,0);
			filp_close(fp,NULL);
			
			/*printk("####################ret=%d\n",ret);
			fp = filp_open(EDID_CONFIG_PATH, O_RDONLY, 0644);
			if (IS_ERR(fp)) { 
		       		printk("open /mnt/vram/edid.bin error\n"); 
				ret = -ENOENT;
				goto edid_match;
			}
			vfs_llseek(fp,EZ_OFFSET,SEEK_SET);
			ret = vfs_read(fp,&lcm_data.edid_config,sizeof(struct edid_config), &fp->f_pos);
			filp_close(fp,NULL);
			printk("@@@@@@@@@@@@@@@@@@ret=%d\n",ret);
			printk("@@@@@@@@@@@ret %d v %d h %d %d %d\n",ret,lcm_data.edid_config.vga_valid,lcm_data.edid_config.hdmi_valid,lcm_data.edid_config.vga_format,lcm_data.edid_config.hdmi_format);
			printk("\n444444444444444444444444444444444444444\n");*/
			set_fs(fs);
			//printk("%s %d ret=%d\n",__FILE__,__LINE__,ret);			
		}
	}
	return format;
}

int edid_find_formart_for_ezcast()
{
	int format = FMT_1650x750_1280x720_60P;	
	int i,j,ret;
	struct fb_monspecs *monA = NULL;
	unsigned char * edid=NULL;
	struct fb_videomode *md = NULL;
	struct lcm_videomode cmp_hdmimode[] = {	
#if (defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE >=30 && MODULE_CONFIG_CHIP_TYPE < 40))
		/* 17 FMT_1920x1080_60P */
		{ NULL, 60, 1920, 1080, LCM_VMODE_NONINTERLACED,17},
		/* 5 FMT_1650x750_1280x720_60P */
		{ NULL, 60, 1280, 720, LCM_VMODE_NONINTERLACED, 5},
#else
		/* 2 FMT_2200x1125_1920x1080_30P */
		{ NULL, 30, 1920, 1080, LCM_VMODE_NONINTERLACED, 2},
		/* 4 FMT_2750x1125_1920x1080_24P */
		//{ NULL, 24, 1920, 1080, LCM_VMODE_NONINTERLACED, 4},	
#endif
		{ NULL, 0, 0, 0, 0, 0}
	};
#if (defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE >=30 && MODULE_CONFIG_CHIP_TYPE < 40))
	format = FMT_1920x1080_60P;
#else
	format = FMT_1650x750_1280x720_60P;
#endif

	monA = &(lcm_data.monA); //kzalloc(sizeof(struct fb_monspecs), GFP_KERNEL);
#if MODULE_CONFIG_HDMI2MHL_ENABLE	
	if (lcm_data.user_config.hdmi_mhl_gpio == HDMI_HDMI2MHL_GPIO39)
	{
		act_writel((act_readl(GPIO_MFCTL1)|0x30000)&0xfffeffff,GPIO_MFCTL1);//open HDMI_HOTPLUG_IN GPIO81
		act_writel(act_readl(GPIO_63_32INEN)|0x00000080,GPIO_63_32INEN);
		if (act_readl(GPIO_63_32DAT)&(1<<7))
			goto hdmi;
		else
		{
			edid = kmalloc((128*4), GFP_KERNEL);
			if (edid)
			{
				it6681_read_edid(NULL,edid,(128*4));
			}
		}
	}	
	else
#endif		
	{
hdmi:
		#if 0
			edid = edid_i2c_gpio_read();
		#else
			edid = edid_i2c_hw_read();
		#endif
	}

	if ((edid) && (monA))
	{
		format = edid_is_change(edid,monA,E_LCDPADMODE_HDMI);
		if (format<0)
		{
		#if (defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE >=30 && MODULE_CONFIG_CHIP_TYPE < 40))
			format = FMT_1920x1080_60P;
		#else
			format = FMT_1650x750_1280x720_60P;
		#endif
			printk("EDID Change or No UserDefine Timing!!\n");
		}
		else
		{
			lcm_data.edid_valid = 1;//edid info 有效
			printk("NO EDID Change format=%d!!\n",format);
			goto end;
		}
	}
	else
	{
		lcm_data.edid_valid = 0;
		printk("Actions LCM DRV: ddc i2c read error\n");
		goto end;
	}
	if (monA->modedb == NULL)
	{
		lcm_data.edid_valid = 0;
		printk("Actions LCM DRV: Unable to get Mode Database\n");
		goto end;
	}
	lcm_data.edid_valid = 1;//edid info 有效
	for (i=0;i<monA->modedb_len;i++)
	{
		if (monA->modedb[i].flag & FB_MODE_IS_FIRST)
		{
			md = &(monA->modedb[i]);
			printk("FB_MODE_IS_FIRST mode[num].xres = %d mode[num].yres = %d refresh = %d vmode = %d \n",md->xres,md->yres,md->refresh,md->vmode);
			break;
		}
	}
	for (i=0;i<(sizeof(cmp_hdmimode)/sizeof(struct lcm_videomode)-1);i++)
	{
		//printk("cmp_hdmimode[%d].xres=%d yres=%d refresh=%d vmode = %d \n",i,cmp_hdmimode[i].xres,cmp_hdmimode[i].yres,cmp_hdmimode[i].refresh,cmp_hdmimode[i].vmode);
		for (j=0;j<monA->modedb_len;j++)
		{
			if (cmp_hdmimode[i].xres == monA->modedb[j].xres &&
			    cmp_hdmimode[i].yres == monA->modedb[j].yres &&
			    cmp_hdmimode[i].refresh == monA->modedb[j].refresh &&
			    cmp_hdmimode[i].vmode == monA->modedb[j].vmode) 
			    {
					format = cmp_hdmimode[i].format;
					printk("EZ Cast best format = %d\n",format);
					goto end;
				}
			//printk("modedb[%d].xres=%d yres=%d refresh=%d vmode=%d\n",j,monA->modedb[j].xres,monA->modedb[j].yres,monA->modedb[j].refresh,monA->modedb[j].vmode);
		}
	}
#if (defined(MODULE_CONFIG_CHIP_TYPE) && (MODULE_CONFIG_CHIP_TYPE >=30 && MODULE_CONFIG_CHIP_TYPE < 40))
	format = FMT_1920x1080_60P;
#else
	format = FMT_1650x750_1280x720_60P;
#endif
	printk("EZ Cast valid format = %d\n",format);
end:
	//if ((monA) && (monA->modedb))
		//fb_destroy_modedb(monA->modedb);
	//if (monA)
		//kfree(monA);
	if (edid)
		kfree(edid);
	return format;
}

int edid_find_best_display(unsigned char current_device)
{
	int format = -1;	
	int i,j,ret;
	struct fb_monspecs *monA = NULL;
	unsigned char * edid=NULL;
	struct fb_videomode *md = NULL;
	struct file *fp=NULL;
	mm_segment_t fs; 
	printk("%s,%d,current_device==%d,lcm_data.user_config.hdmi_mhl_gpio==%d,lcm_data.user_config.device_type==%d\n",
		__FILE__,__LINE__,current_device,lcm_data.user_config.hdmi_mhl_gpio,lcm_data.user_config.device_type);
		//richardyu 080814	
	monA = &(lcm_data.monA); //kzalloc(sizeof(struct fb_monspecs), GFP_KERNEL);
	//hdmi和vga分别采用hw和gpio模拟i2c，需要硬件上面支持
	if (current_device == E_LCDPADMODE_HDMI)
	{
#if MODULE_CONFIG_HDMI2MHL_ENABLE
		if (lcm_data.user_config.hdmi_mhl_gpio == HDMI_HDMI2MHL_GPIO39)
		{
			act_writel((act_readl(GPIO_MFCTL1)|0x30000)&0xfffeffff,GPIO_MFCTL1);//open HDMI_HOTPLUG_IN GPIO81
			act_writel(act_readl(GPIO_63_32INEN)|0x00000080,GPIO_63_32INEN);
			if (act_readl(GPIO_63_32DAT)&(1<<7))
				goto hdmi;
			else
			{
				edid = kmalloc((128*4), GFP_KERNEL);
				if (edid)
				{
					it6681_read_edid(NULL,edid,(128*4));
				}
			}
		}	
		else
#endif			
		{
hdmi:
		#if 0
			edid = edid_i2c_gpio_read();
		#else
			edid = edid_i2c_hw_read();
		#endif
		}
#if MODULE_CONFIG_EZCASTPRO_MODE==8075 || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
#if (defined(MODULE_CONFIG_EZWIRE_TYPE) && MODULE_CONFIG_EZWIRE_TYPE!=0&&NOT_SUPPORT_1080P==0)
  #if MODULE_CONFIG_EZWIRE_TYPE==10
  	format = FMT_1920x1080_60P;
  #else
		if(lcm_read_ddrtype()==2)//8258b DDR2
			format =  FMT_1650x750_1280x720_60P;
		else
			format = FMT_1920x1080_60P;
  #endif
#else
		format = FMT_1650x750_1280x720_60P;
#endif
#endif
	}
	else if (current_device == E_LCDPADMODE_VGA)
	{
#if (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)		//The default hotplug check pin of 8251 is 75, and EZWire is use gpio i2c to read edid
	    #ifndef MODULE_CONFIG_UCLIBC
		edid = edid_i2c_gpio_read();
		#endif
		if(edid)
			printk("[%s-%d] VGA edid[126]: %d\n", __func__, __LINE__, edid[126]);
#else
		//hdmi检测脚设置为gpio81时，VGA采用模拟I2C方式(硬件设计如此)
		if (lcm_data.user_config.hdmi_hpd_gpio== 81)
		{
		 	#ifndef MODULE_CONFIG_UCLIBC
			edid = edid_i2c_gpio_read();
			#endif
		}
		else
		{
			edid = edid_i2c_hw_read();
		}
#endif

#if MODULE_CONFIG_EZCASTPRO_MODE==8075 || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)

#if (defined(MODULE_CONFIG_EZWIRE_TYPE) && MODULE_CONFIG_EZWIRE_TYPE!=0&&NOT_SUPPORT_1080P==0)
	
	#if MODULE_CONFIG_EZWIRE_TYPE==10
		format = VGA_1920_1080;
	#else
		if(lcm_read_ddrtype()==2)//8258b DDR2
			format =  VGA_1280_720;
		else
			format = VGA_1920_1080;
	#endif
#else
		format = VGA_1280_720;
#endif

#endif
	}
	//printk("edid[0x%p], monA[0x%p]\n", edid, monA);
	if ((edid) && (monA))
	{
		format = edid_is_change(edid,monA,current_device);
		if (format<0)
		{
			printk("EDID Change or No UserDefine Timing!!\n");
		}
		else
		{
			lcm_data.edid_valid = 1;//edid info 有效
			printk("NO EDID Change!! format=%d\n",format);
			goto end;
		}	
	}
	else
	{
		lcm_data.edid_valid = 0;
		printk("Actions LCM DRV: ddc i2c read error\n");
		goto end;
	}
	
	if (monA->modedb == NULL)
	{
		lcm_data.edid_valid = 0;
		printk("Actions LCM DRV: Unable to get Mode Database\n");
		goto end;
	}
	lcm_data.edid_valid = 1;//edid info 有效
	for (i=0;i<monA->modedb_len;i++)
	{
		if (monA->modedb[i].flag & FB_MODE_IS_FIRST)
		{
			md = &(monA->modedb[i]);
			printk("FB_MODE_IS_FIRST mode[num].xres = %d mode[num].yres = %d refresh = %d vmode=%d\n",md->xres,md->yres,md->refresh,md->vmode);
			break;
		}
	}
	switch(current_device){
		case E_LCDPADMODE_HDMI: 
//NO  FB_MODE_IS_FIRST for 1:MiraWire with 8252B; 2:MiraWireDuo with 8252C; 3: MiraWirePlus with 8252B/8252W; 4: EZDocking with 8251W; 5:SNOR flash with 8252N; 6: MiraWire with CDC/EDM; 7: MiraWire with CDC/EDM at snor			
#if	(!(defined(MODULE_CONFIG_EZWIRE_TYPE) && MODULE_CONFIG_EZWIRE_TYPE!=0))
			for (i = 0; i < (sizeof(hdmi_modes)/sizeof(struct lcm_videomode)); i++) 
			{
				if (hdmi_modes[i].xres == md->xres &&
				    hdmi_modes[i].yres == md->yres &&
				    hdmi_modes[i].refresh == md->refresh &&
				    hdmi_modes[i].vmode == md->vmode) 
				    {
						format = hdmi_modes[i].format;
						printk("best format = %d w=%d h=%d refresh=%d vmode=%d i=%d\n",format,hdmi_modes[i].xres,hdmi_modes[i].yres,hdmi_modes[i].refresh,hdmi_modes[i].vmode,i);
						#if MODULE_CONFIG_EZWIRE_TYPE!=10
						if(lcm_read_ddrtype()==2&&format==FMT_1920x1080_60P)//8258b DDR2
						 	format = FMT_1650x750_1280x720_60P;
						#endif
						goto end;
					}	
			}
#endif			
			for (i = 0; i < (sizeof(hdmi_modes)/sizeof(struct lcm_videomode)); i++) 
			{
			          printk("hdmi_modes[%d].xres=%d yres=%d refresh=%d vmode=%d\n",i,hdmi_modes[i].xres,hdmi_modes[i].yres,hdmi_modes[i].refresh,hdmi_modes[i].vmode);
				for (j=0;j<monA->modedb_len;j++)
				{
					if (hdmi_modes[i].xres == monA->modedb[j].xres &&
					    hdmi_modes[i].yres == monA->modedb[j].yres &&
					    hdmi_modes[i].refresh == monA->modedb[j].refresh &&
					    hdmi_modes[i].vmode == monA->modedb[j].vmode) 
					    {
							format = hdmi_modes[i].format;
							printk("valid format = %d w=%d h=%d refresh=%d vmode=%d\n",format,hdmi_modes[i].xres,hdmi_modes[i].yres,hdmi_modes[i].refresh,hdmi_modes[i].vmode);
							#if MODULE_CONFIG_EZWIRE_TYPE!=10
							if(lcm_read_ddrtype()==2&&format==FMT_1920x1080_60P)//8258b DDR2
						 		format = FMT_1650x750_1280x720_60P;
							#endif
							goto end;
						}
					//printk("modedb[%d].xres=%d yres=%d refresh=%d vmode=%d\n",j,monA->modedb[j].xres,monA->modedb[j].yres,monA->modedb[j].refresh,monA->modedb[j].vmode);

				}
			}
			printk("no support hdmi format found!\n");
			break;
		case E_LCDPADMODE_VGA:
#if (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
			format = VGA_1280_800;
#endif
//NO  FB_MODE_IS_FIRST for 1:MiraWire with 8252B; 2:MiraWireDuo with 8252C; 3: MiraWirePlus with 8252B/8252W; 4: EZDocking with 8251W; 5:SNOR flash with 8252N; 6: MiraWire with CDC/EDM; 7: MiraWire with CDC/EDM at snor			
#if	(!(defined(MODULE_CONFIG_EZWIRE_TYPE) && MODULE_CONFIG_EZWIRE_TYPE!=0))
			for (i = 0; i < (sizeof(vga_modes)/sizeof(struct lcm_videomode)); i++)
			{
				if 	(vga_modes[i].xres == md->xres &&
				    vga_modes[i].yres == md->yres &&
				    vga_modes[i].refresh == md->refresh &&
				    vga_modes[i].vmode == md->vmode) 
				    {
						format = vga_modes[i].format;
						printk("best format = %d\n",format);
						goto end;
					}	
			}
#endif			
			for (i = 0; i < (sizeof(vga_modes)/sizeof(struct lcm_videomode)); i++)	
			{
				for (j=0;j<monA->modedb_len;j++)
				{
					if (vga_modes[i].xres == monA->modedb[j].xres &&
					    vga_modes[i].yres == monA->modedb[j].yres &&
					    vga_modes[i].refresh == monA->modedb[j].refresh &&
					    vga_modes[i].vmode == monA->modedb[j].vmode) 
					    {
							format = vga_modes[i].format;
							printk("valid format = %d\n",format);
							goto end;
						}	
				}
			}
			printk("no support vga format found!\n");
			break;
		default: 			
			break;
		}
end:
	//if ((monA) && (monA->modedb))
		//fb_destroy_modedb(monA->modedb);
	//if (monA)
		//kfree(monA);
	if (edid)
		kfree(edid);
	
#if 0//(defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
	char wireBuff[32];
	memset(wireBuff, 0, sizeof(wireBuff));
	ret = am_get_config("/mnt/vram/ezcast/PLUGPLAYMODE", wireBuff, -EZ_OFFSET, sizeof(wireBuff));
	if(ret < 0)
	{
		printk("[%s():%d] Read /mnt/vram/ezcast/PLUGPLAYMODE fail.\n", __func__, __LINE__);
	}
	else
	{
		printk("wireBuff: %s\n", wireBuff);
		if(strncmp(wireBuff, "PlugMode=YES", strlen("PlugMode=YES")) == 0)
		{
			format = 5; 	// If is Plugplay mode for iOS, than only support 1280x720 now. 	----Bink.li
			printk("Plugplay mode for iOS only support 1280x720 now(format=5).\n");
		}
	}
#endif

	return format;
}
#endif					      

/**
*@brief API for set hdmi interface 
*@param[in] format : hdmi timing format
*@param[out]  : none
*/
static void init_hdmi(int format)
{
	int SIZE_H = 0 ; 
	int SIZE_W = 0;
	int HSPW = 0;
	int HFP = 0;	
	int HBP =0 ;	
	int VSPWo = 0;	
	int VFPo = 0;
	int VBPo = 0;
	int VSPWe = 0;	
	int VFPe = 0;
	int VBPe = 0;
	

	int need_even = 0 ;

	struct lcm_config *user_config = &lcm_data.user_config;  //add for user defined timing format
	unsigned char hdmi_mode =0;
	struct _SPD_InfoFrame spd;
#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
	struct edid_config *edid_config = &lcm_data.edid_config;
	if (!edid_hdmi_status)
	{
		int format_edid = -1;
		format_edid = edid_find_best_display(lcm_data.current_device);
		if (format_edid >=0)
			format = format_edid;
		printk("%s %d format= %d\n",__FILE__,__LINE__,format);
		edid_vga_status = 1;
		edid_hdmi_status = 1;
	}
#endif	
	printk("LCM Driver[HDMI Final] format= %d\n",format);

	if(Is_HDMI60P(format))
		is_60p_flag =1;
	else
		is_60p_flag =0;

	switch(format)
	{
		case	FMT_858x525_720x480_60I:
				SIZE_H =(240-1);			
 				SIZE_W =(720-1);			
				HSPW =(124-1);	
 				HFP =(38-1);			
 				HBP =(124+114-1);
 				VSPWo =(3-1);
 				VFPo =(4-1);
 				VBPo = (18-1);
 				VSPWe = (3-1);
 				VFPe = (5-1);
 				VBPe = (18-1);
				need_even = 1;
				lcd_controller_data.lcd_vsync_edge =(310-1); 
				break;
		case 	FMT_864x625_720x576_50I: 
				SIZE_H =(288-1);			
 				SIZE_W =(720-1);			
				HSPW =(126-1);	
 				HFP =(24-1);			
 				HBP = (126+138-1);
 				VSPWo =(3-1);
 				VFPo =(2-1);
 				VBPo = (22-1);
 				VSPWe = (3-1);
 				VFPe = (3-1);
 				VBPe = (22-1);
				need_even = 1;
				lcd_controller_data.lcd_vsync_edge =(300-1);
				break;
		case	FMT_2200x1125_1920x1080_60I:
				SIZE_H =(540-1);			
				SIZE_W =(1920-1);			
				HSPW =(44-1); 
				HFP =(88-1);			
				HBP =(44+148-1);
				VSPWo =(5-1);
				VFPo =(2-1);
				VBPo = (5+15-1);
				VSPWe = (5-1);
				VFPe = (3-1);
				VBPe = (5+15-1);
				need_even = 1;
				lcd_controller_data.lcd_vsync_edge =(908-1);
				break;
		case 	FMT_2640x1125_1920x1080_50I:	
				SIZE_H =(540-1);			
 				SIZE_W =(1920-1);			
				HSPW =(44-1);	
 				HFP =(528-1);			
 				HBP =(44+148-1);
 				VSPWo =(5-1);
 				VFPo =(2-1);
 				VBPo = (5+15-1);
 				VSPWe = (5-1);
 				VFPe = (3-1);
 				VBPe = (5+15-1);
				need_even = 1;
				lcd_controller_data.lcd_vsync_edge =(1128-1);
				break;
		case  	FMT_864x625_720x576_50P:
				SIZE_H =(576-1);
				SIZE_W = (720-1);			
				HSPW = (64-1);			
				HFP = (12-1);			
				HBP = (132-1);						
				VSPWo = (5-1);			
				VFPo =(5-1);			
				VBPo =(44-1);			
				break;
		case    FMT_858x525_720x480_60P:
				SIZE_H =(480-1);
				SIZE_W = (720-1);			
				HSPW = (62-1);						
				HFP = (16-1);			
				HBP = (122-1);						
				VSPWo = (6-1);			
				VFPo =(9-1);			
				VBPo =(36-1);
				break;
		case	FMT_1650x750_1280x720_60P:
		case 	FMT_3300x750_1280x720_30P:
		case	FMT_3960x750_1280x720_25P:
		case	FMT_4125x750_1280x720_24P:	
				SIZE_H =(720-1);
				SIZE_W = (1280-1);			
				HSPW = (40-1);						
				HFP = (110-1);			
				HBP = (260-1);						
				VSPWo = (5-1);			
				VFPo =(5-1);			
				VBPo =(25-1);
				break;
		case	FMT_1980x750_1280x720_50P:
				SIZE_H =(720-1);
				SIZE_W = (1280-1);			
				HSPW = (40-1);						
				HFP = (440-1);			
				HBP = (260-1);						
				VSPWo = (5-1);			
				VFPo =(5-1);			
				VBPo =(25-1);
				break;
		
		case	FMT_2640x1125_1920x1080_25P:
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (44-1);						
				HFP = (528-1);			
				HBP = (192-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);
				break;
		case	FMT_2750x1125_1920x1080_24P:			
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (44-1);						
				HFP = (638-1);			
				HBP = (192-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);			
				break;
		case 	FMT_2200x1125_1920x1080_30P:
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (44-1);						
				HFP = (88-1);			
				HBP = (192-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);				
				break;
		case 	FMT_1920x1080_60P:
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (44-1);						
				HFP = (88-1);			
				HBP = (44+148-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);			
				break;
		case 	FMT_854x480_50P:
				SIZE_H =(480-1);
				SIZE_W = (854-1);			
				HSPW = (16-1);						
				HFP = (40-1);			
				HBP = (56-1);						
				VSPWo = (2-1);			
				VFPo = (39-1);			
				VBPo =(12-1);			
				break;
		case 	FMT_1920x1080_50P:
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (44-1);						
				HFP = (528-1);			
				HBP = (192-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);				
				break;
		case	FMT_1280x800_60P:
				SIZE_H =(800-1);
				SIZE_W = (1280-1);			
				HSPW = (136);						
				HFP = (64-1);			
				HBP = (336-1);						
				VSPWo = (3);			
				VFPo =(1-1);			
				VBPo =(27-1);
				break;
		case	FMT_800x600_60P:
				SIZE_H =(600-1);
				SIZE_W = (800-1);			
				HSPW = (128-1);						
				HFP = (40-1);			
				HBP = (216-1);						
				VSPWo = (4-1);			
				VFPo =(1-1);			
				VBPo =(27-1);
				break;
		case	FMT_1250x810_1024x768_60P:
				SIZE_H =(768-1);
				SIZE_W =(1024-1);
				HSPW =(136);
				HFP =(24-1);	
				HBP =(296-1);
				VSPWo = (6);
				VFPo = (3-1);
				VBPo = (35-1); 
				break;
		case	FMT_2080x741_1920x720_59P:
				SIZE_H =(720-1);
				SIZE_W =(1920-1);
				HSPW =(32-1);
				HFP =(48-1);	
				HBP =(112-1);
				VSPWo = (10-1);
				VFPo = (3-1);
				VBPo = (18-1); 
				break;
		case    FMT_USER_DEFINED:  //value 16 ,for DVI user defined format		
				SIZE_H =user_config->device_height-1;
				SIZE_W = user_config->device_width-1;			
				HSPW = user_config->hswp;						
				HFP = user_config->hfp;			
				HBP = user_config->hbp;						
				VSPWo = user_config->vswp;			
				VFPo = user_config->vfp;			
				VBPo = user_config->vbp;	
				break;
		
			
		default:
				printk("not supported format:%x\n",format);
				break;
	}



	lcd_controller_data.lcd_ctl =4;
	lcd_controller_data.lcd_color = 0xff;
	lcd_controller_data.lcd_size = (SIZE_H<<16)|(SIZE_W);
	lcd_controller_data.lcd_htiming = (HSPW<<22)|(HFP<<11)|(HBP);
	lcd_controller_data.lcd_vtiming_odd = (VSPWo<<22)|(VFPo<<11)|(VBPo);
	lcd_controller_data.lcd_lsp = 1;
	lcd_controller_data.lcd_dac_ctl = 0x80; //default value
	if(need_even == 1){
		lcd_controller_data.lcd_vtiming_even = (VSPWe<<22)|(VFPe<<11)|(VBPe);
	}
	else{
		lcd_controller_data.lcd_vtiming_even = 0;
		lcd_controller_data.lcd_vsync_edge = 0;
	}
	
	if(format == FMT_858x525_720x480_60I|| format == FMT_864x625_720x576_50I)
		lcd_controller_data.lcd_rgbctl = 0x1c0001;
	else if(format == FMT_2200x1125_1920x1080_60I||format == FMT_2640x1125_1920x1080_50I) //1080 i
		lcd_controller_data.lcd_rgbctl =0x18c001;
	else if(format ==FMT_864x625_720x576_50P||format ==FMT_858x525_720x480_60P)
		lcd_controller_data.lcd_rgbctl = 0x080001;  //480 p
	else lcd_controller_data.lcd_rgbctl = 0x08c001;  //720 P   user defined 

	start_config();
	
#if CONFIG_AM_CHIP_ID == 1213
	/** may use EDID or user define format instead of default */
	lcm_data.current_format = format;
#endif

	hdmi_config_de(SIZE_W+1,SIZE_H+1,need_even);

	lcd_controller_init();
	if (!(lcm_data.current_device&VGA_HDMI_SYNC_MODE)&&(edid_mode == EDID_MODE_FOR_STDBOARD)&&(lcm_data.edid_valid)&&((lcm_data.monA.extensionflag ==0) || (lcm_data.monA.vsdbflag ==0)))
	{
		printk("LCM DRIVE[NOTICE:HDMI MODE:HDMI_MODE_DVI,Check EDID}\n");
		hdmi_mode = HDMI_MODE_DVI;
	}
	else
		hdmi_mode = HDMI_MODE_HDMI;
	spd.version = 1;
	spd.device = lcm_data.user_config.hdmi_device_info;
	memcpy(spd.desc,lcm_data.user_config.hdmi_product_decs,17);
	memcpy(spd.name,lcm_data.user_config.hdmi_vendor_name,9);
	hdmi_init(format,hdmi_mode,&spd);

	hdmi_config_clock(format);
	
	end_config();

	lcd_adjust_gamma(); //gamma should be set when lcdpll is enable
	
#if MODULE_CONFIG_EZCASTPRO_MODE==8075 || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
	if (!(lcm_data.current_device&VGA_HDMI_SYNC_MODE)&&((format == FMT_1650x750_1280x720_60P)\
		||(format == FMT_1920x1080_60P)||(format == FMT_1280x800_60P)||(format == FMT_1250x810_1024x768_60P)))
	{
		int sync_vga_format = VGA_1280_720;
		lcm_data.current_device |=VGA_HDMI_SYNC_MODE;
		if (format == FMT_1920x1080_60P)
			sync_vga_format = VGA_1920_1080;
		else if (format == FMT_1280x800_60P)
			sync_vga_format = VGA_1280_800;
		else if(format == FMT_1250x810_1024x768_60P)
			sync_vga_format = VGA_1024_768;
		init_vga(sync_vga_format);		
	}	
#endif	
}



/**
*@brief API for set ypbpr interface 
*@param[in] format : ypbpr timing format
*/
void init_ypbpr(int format)
{
	int SIZE_H =0 ; 
	int SIZE_W = 0;
	int HSPW = 0;
	int HFP = 0;	
	int HBP =0 ;	
	int VSPWo = 0;	
	int VFPo = 0;
	int VBPo = 0;
	int VSPWe = 0;	
	int VFPe = 0;
	int VBPe = 0;
	
	int need_even = 0 ;
	switch(format){
		case YPBPR_576I:
	//576i	
				SIZE_H	= (288-1);
				SIZE_W	= (720-1);
				HSPW   =  (63-1);
	    		HFP	=  (18-1);
				HBP 	= (63+63-1);
				VSPWo	=  (3-1);
				VFPo	=  (2-1);
				VBPo	=  (22-1);
				VSPWe	=  (3-1);
				VFPe	  = (3-1);
				VBPe	 = (22-1);
				need_even = 1;
				break;
		case YPBPR_480I:		
	//480i
				SIZE_H	= (240-1);
				SIZE_W	= (720-1);
				HSPW	= (63-1);
		   	    HFP	 = (16-1);
				HBP 	= (59+63-1);
				VSPWo	 = (3-1);
				VFPo	=  (4-1);
				VBPo	=  (18-1); 
				VSPWe	=  (3-1);
				VFPe	=  (5-1);
				VBPe	=  (18-1);
				need_even =1;
				break;	
	///////////////////////////////////////////////////////////
		case YPBPR_576P:	
		//720x576p
				SIZE_H	= (576-1);
				SIZE_W	= (720-1);
				HSPW	= (64-1);
				HFP 	= (12-1);
				HBP    =  (64+68-1) ;
				VSPWo	=  (5-1);
				VFPo	=  (5-1);
				VBPo   =   (39+5-1);
				break;
		case YPBPR_480P:
		//720x480p
				SIZE_H	= (480-1);	
				SIZE_W	= (720-1);
				HSPW   =  (62-1);
				HFP 	= (16-1);
				HBP 	= (62+60-1);
				VSPWo	 = (6-1);
				VFPo	  =(9-1);
				VBPo =	 (30+6-1);
				break;
		case YPBPR_720P:
		//1280x720p 		
				SIZE_H	= (720-1);	
				SIZE_W	= (1280-1);
						
				HSPW   =  (80-1);
				HFP 	= (70-1);
				HBP 	= (300-1);
				VSPWo	 = (7-1);
				VFPo	  =(3-1);
				VBPo =	 (27-1);		
				break;
			case YPBPR_1080P:
		//1920x1080p
				SIZE_H	= (1080-1); 
				SIZE_W	= (1920-1);
						
				HSPW   =  (88-1);
				HFP 	= (44-1);
				HBP 	= (236-1);
				VSPWo	 = (5-1);
				VFPo	  =(4-1);
				VBPo =	 (41-1);
							
				break;
			default:
				printk("unsuppported ypbpr format:%x\n",format);
				break;
		}


	lcd_controller_data.lcd_ctl =4;
	lcd_controller_data.lcd_color = 0xff;
	lcd_controller_data.lcd_size = (SIZE_H<<16)|(SIZE_W);
	lcd_controller_data.lcd_htiming = (HSPW<<22)|(HFP<<11)|(HBP);
	lcd_controller_data.lcd_vtiming_odd = (VSPWo<<22)|(VFPo<<11)|(VBPo);
	lcd_controller_data.lcd_lsp = 1;
	lcd_controller_data.lcd_vsync_edge =0; 

	if(format == YPBPR_1080P||format == YPBPR_720P)
		lcd_controller_data.lcd_rgbctl = 0x00032021;
	else if(format == YPBPR_480P||format == YPBPR_576P)
		lcd_controller_data.lcd_rgbctl = 0x00022021;
	else 
		lcd_controller_data.lcd_rgbctl = 0x000122021;

	if(format == YPBPR_1080P)
		lcd_controller_data.lcd_dac_ctl= 0x3ff;
	else
		lcd_controller_data.lcd_dac_ctl = 0x38f;
	
	if(need_even == 1){
		lcd_controller_data.lcd_vtiming_even = (VSPWe<<22)|(VFPe<<11)|(VBPe);
	}
	else{
		lcd_controller_data.lcd_vtiming_even = 0;
	}

	start_config();
	
	ypbpr_config_de(SIZE_W+1,SIZE_H+1,need_even);
	
	lcd_controller_init();	
	if(format == YPBPR_1080P) //TCON 
	{
#if CONFIG_AM_CHIP_ID == 1213
		act_writel(0,GPO3_CON);
       act_writel(0x002c07db,GPO3_HP);         
       act_writel(0,GPO3_VP);

       act_writel(1,GPO4_CON);
       act_writel(0x005807af,GPO4_HP);          
       act_writel(0,GPO4_VP);

       act_writel(9,GPO5_CON);
       act_writel(0x07e80833,GPO5_HP);         
       act_writel(0x0005043c,GPO5_VP);

       act_writel(0x08980465,VIDEO_SIZE);
       act_writel(1,TCON_CON);
#else
		act_writel(0,GPO5_CON);
		act_writel(0x002c07db,GPO5_HP);	
		act_writel(0,GPO5_VP);

		act_writel(1,GPO6_CON);
		act_writel(0x005807af,GPO6_HP);	
		act_writel(0,GPO6_VP);
	
		act_writel(9,GPO7_CON);
		act_writel(0x07e80833,GPO7_HP);	
		act_writel(0x0005043c,GPO7_VP);

		act_writel(0x08980465,VIDEO_SIZE);
		act_writel(1,TCON_CON);
#endif		
	}
	if(format == YPBPR_720P) //TCON
	{
#if CONFIG_AM_CHIP_ID == 1213
		act_writel(0,GPO3_CON);
		act_writel(0x00280571,GPO3_HP);         
		act_writel(0,GPO3_VP);

		act_writel(1,GPO4_CON);
		act_writel(0x00500549,GPO4_HP);         
		act_writel(0,GPO4_VP);

		act_writel(9,GPO5_CON);
		act_writel(0x05d205c1,GPO5_HP);         
		act_writel(0x000702d4,GPO5_VP);

		act_writel(0x067202ee,VIDEO_SIZE);
		act_writel(1,TCON_CON);
 #else  
		act_writel(0,GPO5_CON);
		act_writel(0x00280571,GPO5_HP);	
		act_writel(0,GPO5_VP);

		act_writel(1,GPO6_CON);
		act_writel(0x00500549,GPO6_HP);	
		act_writel(0,GPO6_VP);
	
		act_writel(9,GPO7_CON);
		act_writel(0x05d205c1,GPO7_HP);	
		act_writel(0x000702d4,GPO7_VP);

		act_writel(0x067202ee,VIDEO_SIZE);
		act_writel(1,TCON_CON);
#endif
	}
	ypbpr_config_clock(format);
	
	end_config();

	lcd_adjust_gamma(); //gamma should be set when lcdpll is enable
	
}

/**
*@brief API for init cvbs de data structure do not operate hardware registers 
*@param[in] format : nosense
*@param[out]  : none
*/
void init_cvbs_data(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	DE_INFO *DeInfo = &lcm_data.DeInfo; 
	int width=0,height=0;

	if(format == PAL)
	{
		width = 720;
		height = 576; 
	}
	else if(format == NTSC)
	{
		width = 720;
		height = 480;
	}

	DeInfo->screen_type = DE_OUT_DEV_ID_CVBS;
	DeInfo->screen_width = width;
	DeInfo->screen_height = height; 

	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}

	DeInfo->colorspace= CSC_OFF;
	DeInfo->Mode= BTMODE_I;
	DeInfo->DitherMode = DITHER_OFF;	
	DeInfo->DefaultColor = 0x108080;  //default black	

	DeInfo->crop_ori_x = 1;
	DeInfo->crop_ori_y = 1;
	DeInfo->crop_width = DeInfo->input_width;
	DeInfo->crop_height = DeInfo->input_height;
	DeInfo->pip_ori_x = 1;
	DeInfo->pip_ori_y = 1;
	DeInfo->pip_frame_width = DeInfo->screen_width;
	DeInfo->pip_frame_height = DeInfo->screen_height;
	
	DeInfo->input_pix_fmt = PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;
	DeInfo->DisplaySource = DE_DEFAULTCOLOR;
	RegBitSet(DeInfo->DisplaySource,DE_Con,12,12);//1:enable,0:disable
	#if ((CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
		act_writel(0x1,DB_WR);
	#endif
	/* Notice : display engine must be disabled before set */

	/*init gamma data*/
	DeInfo->GammaMode = lcm_data.user_color_config.gamma_mode;//  color_data.gamma_mode;
	DeInfo->Gamma = (unsigned int*)lcm_data.user_color_config.gamma;

	DeInfo->hue= lcm_data.user_color_config.hue;
	DeInfo->saturation = lcm_data.user_color_config.saturation;
	DeInfo->brightness = lcm_data.user_color_config.brightness;
	DeInfo->contrast = lcm_data.user_color_config.contrast;

//	lcd_adjust_gamma();
}



/**
*@brief API for set cvbs interface 
*@param[in] format : cvbs timing format
*@param[out]  : none
*/
void init_cvbs(int format)
{
	int width=0,height=0;

	if(format == PAL)
	{
		width = 720;
		height = 576; 
	}
	else if(format == NTSC)
	{
		width = 720;
		height = 480;
	}

	start_config();
	
	cvbs_config_clock(format);
	
	cvbs_config_de(width,height);
	if(format == PAL) //ntsc  itu encoder 
	{
		act_writel(0x20,BTVE_CTL);
		act_writel(0x027006BF,BTVE_SIZE);  //
		act_writel(0x800000,BTVE_HS);
		act_writel(0x06BF011F,BTVE_HDE);
		act_writel(0x00030001,BTVE_VSODP);
		act_writel(0x013B0139,BTVE_VSEVEN);
		act_writel(0x01360017,BTVE_VDEODD);
		act_writel(0x026f0150,BTVE_VDEEVEP);
		act_writel(0x01390001,BTVE_FT);
		act_writel(0x00800000,BTVE_LSP);
// tv encoder 
		act_writel(0x14,BT_IVECTL);
		act_writel(0x1,BT_IVEOUTCTL);
		act_writel(0x21,BTVE_CTL);
	}
	else if(format == NTSC)
	{	
		act_writel(0x28,BTVE_CTL);
		act_writel(0x020c06b3,BTVE_SIZE); 
		act_writel(0x800000,BTVE_HS);
		act_writel(0x06b30113,BTVE_HDE);
		act_writel(0x00030001,BTVE_VSODP);	
		act_writel(0x010a0107,BTVE_VSEVEN);
		act_writel(0x01030012,BTVE_VDEODD);
		act_writel(0x020a0119,BTVE_VDEEVEP);
		act_writel(0x01070001,BTVE_FT);
		act_writel(0x00800000,BTVE_LSP);
		act_writel(0x10,BT_IVECTL);
		act_writel(0x1,BT_IVEOUTCTL);
    	act_writel(0x00000029,BTVE_CTL);
	}
	end_config();

	lcd_adjust_gamma(); //gamma should be set when lcdpll is enable
}


/**
*@brief API for set vga interface 
*@param[in] format : vga timing format
*@param[out]  : none
*/
void init_vga(int format)
{
	int SIZE_H =0 ; 
	int SIZE_W = 0;
	int HSPW = 0;
	int HFP = 0;	
	int HBP =0 ;	
	int VSPWo = 0;	
	int VFPo = 0;
	int VBPo = 0;

#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
	struct edid_config *edid_config = &lcm_data.edid_config;
	if (!edid_vga_status)
	{
		int format_edid = -1;
		format_edid = edid_find_best_display(lcm_data.current_device);
		if (format_edid >=0)
			format = format_edid;
		printk("%s %d format= %d\n",__FILE__,__LINE__,format);
		edid_vga_status = 1;
		edid_hdmi_status = 1;
	}
#endif	
	printk("%s %d format= %d\n",__FILE__,__LINE__,format);

	if(Is_VGA60P(format))
		is_60p_flag =1;
	else
		is_60p_flag =0;

	switch(format){
		case VGA_1280_1024:
			 SIZE_H = (1024-1);
			 SIZE_W = (1280-1);
			 HSPW = (112-1);
			 HFP = (48-1);
			 HBP = (360-1); 	
			 VSPWo = (3-1);
			 VFPo =(1-1);
			 VBPo =(41-1);
			break;
		case VGA_1280_800_71M:
			 SIZE_H = (800-1);
			 SIZE_W = (1280-1);
			 HSPW = (32-1);
			 HFP = (48-1);
			 HBP = (112-1); 	
			 VSPWo = (6-1);
			 VFPo =(3-1);
			 VBPo =(20-1);
			break;
		case VGA_1280_800:
			 SIZE_H = (800-1);
			 SIZE_W = (1280-1);
			 HSPW = (128-1);
			 HFP = (72-1);
			 HBP = (328-1);
			 VSPWo = (6-1);
			 VFPo =(3-1);
			 VBPo =(28-1);
			break;
		case VGA_1920_1080:
		case VGA_1920_1080_30P:
			SIZE_H =(1080-1);
			SIZE_W = (1920-1);			
			HSPW = (44-1);						
			HFP = (88-1);			
			HBP = (192-1);						
			VSPWo = (5-1);			
			VFPo =(4-1);			
			VBPo =(41-1);
			break;
		case VGA_1366_768:
			SIZE_H =(768-1);
			SIZE_W = (1366-1);			
			HSPW = (56-1);						
			HFP = (14-1);			
			HBP = (120-1);						
			VSPWo = (3-1);			
			VFPo =(1-1);			
			VBPo =(31-1);
			break;
		case VGA_1366_768_85M:
			SIZE_H =(768-1);
			SIZE_W = (1366-1);			
			HSPW = (143-1);						
			HFP = (70-1);			
			HBP = (356-1);						
			VSPWo = (3-1);			
			VFPo =(3-1);			
			VBPo =(27-1);
			break;
		case VGA_1280_768:
			 SIZE_H =(768-1);
			 SIZE_W =(1280-1);
			 HSPW = (32-1);
			 HFP =(48-1);
			 HBP =(112-1);	
			 VSPWo = (7-1);
			 VFPo =(3-1);	
			 VBPo =(19-1);							
			break;
		case VGA_1280_768_79M:
			 SIZE_H =(768-1);
			 SIZE_W =(1280-1);
			 HSPW = (128-1);
			 HFP =(64-1);
			 HBP =(320-1);	
			 VSPWo = (7-1);
			 VFPo =(3-1);	
			 VBPo =(27-1);							
			break;
		case VGA_1024_768:
			 SIZE_H =(768-1);
			 SIZE_W =(1024-1);
			 HSPW =(136-1);
			 HFP =(24-1);	
			 HBP =(296-1);
			 VSPWo = (6-1);
			 VFPo = (3-1);
			 VBPo = (35-1); 		
			break;
		case VGA_800_600:
			 SIZE_H =(600-1);
			 SIZE_W =(800-1);	
			 HSPW =(128-1);	
			 HFP =(40-1);
			 HBP =(216-1);
			 VSPWo =(4-1);
			 VFPo =(1-1);	
			 VBPo =(27-1);			 
			break;
		case VGA_640_480:
			 SIZE_H =(480-1);
			 SIZE_W =(640-1);
			 HSPW =(64-1);
			 HFP = (16-1);
			 HBP = (184-1);
			 VSPWo =(3-1);
			 VFPo =(1-1);
			 VBPo =(19-1);			
			break;
		case VGA_1280_720:
			 SIZE_H =(720-1);
			 SIZE_W = (1280-1);			
			 HSPW = (40-1);						
			 HFP = (110-1);			
			 HBP = (260-1);						
			 VSPWo = (5-1);			
			 VFPo =(5-1);			
			 VBPo =(25-1);
			break;
		default:
			printk("error vga format:%d\n",format);
			break;
	}
#if MODULE_CONFIG_EZCASTPRO_MODE==8075 || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
	if (!(lcm_data.current_device&VGA_HDMI_SYNC_MODE)&&((format == VGA_1280_720)\
		||(format == VGA_1920_1080)||(format == VGA_1280_800)||(format == VGA_1024_768)))
	{
		int sync_hdmi_format = FMT_1650x750_1280x720_60P;
		lcm_data.current_device |=VGA_HDMI_SYNC_MODE;
		if (format == VGA_1920_1080)
			sync_hdmi_format = FMT_1920x1080_60P;
		else if (format == VGA_1280_800)
			sync_hdmi_format = FMT_1280x800_60P;
		else if(format == VGA_1024_768)
			sync_hdmi_format = FMT_1250x810_1024x768_60P;
		init_hdmi(sync_hdmi_format);		
	}	
#endif	
	lcd_controller_data.lcd_ctl = 0x4;
	lcd_controller_data.lcd_rgbctl = 0x8c001;
	lcd_controller_data.lcd_dac_ctl = 0xff;
	lcd_controller_data.lcd_htiming = (HSPW<<22)|(HFP<<11)|(HBP);
	lcd_controller_data.lcd_vtiming_odd = (VSPWo<<22)|(VFPo<<11)|(VBPo);
	lcd_controller_data.lcd_vtiming_even = 0;
	lcd_controller_data.lcd_lsp =0;
	lcd_controller_data.lcd_vsync_edge = 292-1;
	lcd_controller_data.lcd_size = (SIZE_H<<16)|(SIZE_W);
	lcd_controller_data.lcd_color = 0xff;

	start_config();

	vga_mfp_config();

#if CONFIG_AM_CHIP_ID == 1213
	/** may use EDID or user define format instead of default */
	lcm_data.current_format = format;
#endif

	vga_config_de(SIZE_W+1,SIZE_H+1);
	
	lcd_controller_init();
	
	vga_config_clock(format);
	
	end_config();

	lcd_adjust_gamma(); //gamma should be set when lcdpll is enable

}

/**
*@brief API for init bt de data structure do not operate hardware registers 
*@param[in] format : nosense
*@param[out]  : none
*/
void init_bt_data(int format)
{
	struct lcm_config *user_config = &lcm_data.user_config;
	DE_INFO *DeInfo = &lcm_data.DeInfo; 
	int width=0,height=0;

	bt_mfp_config();

	if(format == PAL)
	{
		width = 720;
		height = 576; 
	}
	else if(format == NTSC)
	{
		width = 720;
		height = 484;
	}

	DeInfo->screen_type = DE_OUT_DEV_ID_CVBS;
	DeInfo->screen_width = width;
	DeInfo->screen_height = height; 

	if(DeInfo->input_width == 0&&DeInfo->input_height ==0){
		DeInfo->input_width = DeInfo->screen_width;
		DeInfo->input_height = DeInfo->screen_height;
	}

	DeInfo->colorspace= CSC_OFF;
	DeInfo->Mode= BTMODE_I;
	DeInfo->DitherMode = DITHER_OFF;	
	DeInfo->DefaultColor = 0x108080;  //default black

	DeInfo->crop_ori_x = 1;
	DeInfo->crop_ori_y = 1;
	DeInfo->crop_width = DeInfo->input_width;
	DeInfo->crop_height = DeInfo->input_height;
	DeInfo->pip_ori_x = 1;
	DeInfo->pip_ori_y = 1;
	DeInfo->pip_frame_width = DeInfo->screen_width;
	DeInfo->pip_frame_height = DeInfo->screen_height;
	
	DeInfo->input_pix_fmt = PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	DeInfo->display_mode = E_DISPLAYMODE_FULL_SCREEN;
	DeInfo->DisplaySource = DE_DEFAULTCOLOR;
	RegBitSet(DeInfo->DisplaySource,DE_Con,12,12);//1:enable,0:disable
	#if ((CONFIG_AM_CHIP_ID == 1207)||(CONFIG_AM_CHIP_ID == 1211)||(CONFIG_AM_CHIP_ID == 1220)||(CONFIG_AM_CHIP_ID == 1213))
		act_writel(0x1,DB_WR);
	#endif
	/* Notice : display engine must be disabled before set */

	/*init gamma data*/
	DeInfo->GammaMode = lcm_data.user_color_config.gamma_mode;//  color_data.gamma_mode;
	DeInfo->Gamma = (unsigned int*)lcm_data.user_color_config.gamma;

	DeInfo->hue= lcm_data.user_color_config.hue;
	DeInfo->saturation = lcm_data.user_color_config.saturation;
	DeInfo->brightness = lcm_data.user_color_config.brightness;
	DeInfo->contrast = lcm_data.user_color_config.contrast;		
	
//	lcd_adjust_gamma();
}


/**
*@brief API for set bi656/601 interface 
*@param[in] format : nosense
*@param[out]  : none
*/
void init_bt(int format)
{
	// FIXME ,JUST FOR BT656 35DPANNEL NOW
	int width = 720,height = 484;

	start_config();

	bt_config_clock(format);

	bt_mfp_config();

	bt_config_de(width,height);

	bt_controller_init();
	
	end_config();

	lcd_adjust_gamma(); //gamma should be set when lcdpll is enable
}


/**
*@brief API for disable display clock module 
*@param[in]    : none
*@param[out]  : none
*/
void close_dclock(void)
{
	act_writel(0x4,CMU_DISPLAYCLK);
}
/**
*@brief API for disable lcd module 
*@param[in]    : none
*@param[out]  : none
*/
void close_lcd(void)
{
	struct lcm_config *user_config = &lcm_data.user_config;

	if(user_config->lvds_en)
		lvds_controller_exit();
	if(user_config->tcon_en)
		tcon_controller_exit();
	
	if(user_config->lvds_en == 0) //need mfp configuration
		lcd_mfp_reset();
	if(user_config->power_gpio !=255)
		am_set_gpio(user_config->power_gpio,0);
	lcd_controller_exit();
	close_dclock();
}

/**
*@brief API for disable hdmi module
*@param[in]    : none
*@param[out]  : none
*/
void close_hdmi(void)
{
	hdmi_close();
	lcd_controller_exit();
	close_dclock();
}

/**
*@brief API for disable ypbpr module
*@param[in]    : none
*@param[out]  : none
*/
void close_ypbpr(void)
{
	lcd_controller_exit();
	dac_controller_exit();
	close_dclock();
}

/**
*@brief API for disable cvbs module
*@param[in]    : none
*@param[out]  : none
*/
void close_cvbs(void)
{
	bt_controller_exit();	
	close_dclock();
}

/**
*@brief API for disable vga module
*@param[in]    : none
*@param[out]  : none
*/
void close_vga(void)
{
	lcd_controller_exit();
	vga_mfp_reset();
	dac_controller_exit();	
	close_dclock();
}

/**
*@brief API for disable bt module
*@param[in]    : none
*@param[out]  : none
*/
void close_bt(void)
{
	bt_controller_exit();
	bt_mfp_reset();
	close_dclock();
}
/**
*@brief API for disable current display device module
*@param[in]   type : current device type
*@param[out]  : none
*/
void close_device(int type)
{
//close de first 
	de_set(DE_RELEASE,0);
	switch(type)
	{
		case E_LCDPADMODE_CPU: 			
		case E_LCDPADMODE_LCD:			
		case E_LCDPADMODE_TCON:
		case E_LCDPADMODE_DTCON:
		case E_LCDPADMODE_LVDS:
			close_lcd();
			break;	
		case E_LCDPADMODE_BT:
			close_bt();
			break;
		case E_LCDPADMODE_HDMI:
			close_hdmi();
			break;
		case E_LCDPADMODE_YPBPR:
			close_ypbpr();
			break;
		case E_LCDPADMODE_CVBS:
			close_cvbs();
			break;
		case E_LCDPADMODE_VGA:
			close_vga();
		case E_LCDPADMODE_BUTT:
			
			break;
		default:
			printk(KERN_ERR "error device type:%d\n",type);
			break;
	}

}

int am7x_display_module_init(void)
{	
	lcm_read_ddrtype();
	act_writel(act_readl(CMU_DEVCLKEN)|1<<1|1<<14,CMU_DEVCLKEN);
	RegBitSet(1,CMU_DEVCLKEN,12,12);
	/*BTVD hclk enable.*/
	RegBitSet(1,CMU_DEVCLKEN,30,30);
	/*BTVE_CLK EN*/

#if MODULE_CONFIG_EZCAST_ENABLE
/**Disable clock to reduce power consumption :BTVD/EMAC/USBP/SPDIF/(KEYSCAN)/TP/(ADC)/DAC/USBC/BTVE/SDIO/SPI/CRC/NOR*/
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=8251
	printk("ezcastpro 8250, CMU_DEVCLKEN not change\n");
#elif defined(MODULE_CONFIG_EZMUSIC_ENABLE) && (MODULE_CONFIG_EZMUSIC_ENABLE != 0)
	printk("ezmusic 8253, disable some modules clock\n");
	act_writel(0xB3F543F6,CMU_DEVCLKEN);
#else	
	act_writel(0xB3F5C7F6,CMU_DEVCLKEN);
#endif	
	printk("%s:CMU_DEVCLKEN=0x%x\n",__FILE__,act_readl(CMU_DEVCLKEN));
#endif	
	memset(&lcm_data.DeInfo,0,sizeof(lcm_data.DeInfo)),
	read_user_config();
	
	lcm_data.DeInfo.hue = lcm_data.user_color_config.hue;
	lcm_data.DeInfo.saturation = lcm_data.user_color_config.saturation;
	lcm_data.DeInfo.brightness = lcm_data.user_color_config.brightness;
	lcm_data.DeInfo.contrast = lcm_data.user_color_config.contrast;
	lcm_data.DeInfo.Gamma = lcm_data.user_color_config.gamma;
	lcm_data.DeInfo.GammaMode = lcm_data.user_color_config.gamma_mode;
			
	lcm_data.current_device = lcm_data.user_config.device_type;
	lcm_data.current_format = lcm_data.user_config.output_format;
	lcm_data.edid_valid = 0;//edid info 默认无效
	
/*HDMI/VGA user defined first*/	
#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))	
	if ((lcm_data.current_device == E_LCDPADMODE_HDMI)||(lcm_data.current_device == E_LCDPADMODE_VGA))
	{
		if (edid_mode == EDID_MODE_FOR_EZCAST)
		{			
			lcm_data.current_device = E_LCDPADMODE_HDMI;
			lcm_data.current_format = edid_find_formart_for_ezcast();
			edid_vga_status = 1;
			edid_hdmi_status = 1; 
		}
		else if (edid_mode == EDID_MODE_FOR_STDBOARD)
		{
			 unsigned int detect_reg_addr;
			 unsigned char detect_reg_bit;
			 if (lcm_data.user_config.hdmi_hpd_gpio == HDMI_HOTPLUG_GPIO81)
			 {
				act_writel(act_readl(GPIO_MFCTL3)&0xfffffffc,GPIO_MFCTL3);//open HDMI_HOTPLUG_IN GPIO81
				act_writel(act_readl(GPIO_95_64INEN)|0x00020000,GPIO_95_64INEN);
				act_writel(act_readl(GPIO_95_64OUTEN)&0xfffdffff,GPIO_95_64OUTEN);
				printk("GPIO_MFCTL3=0x%x reg[GPIO_95_64DAT]=0x%x\n",act_readl(GPIO_MFCTL3), act_readl(GPIO_95_64DAT));
				detect_reg_addr = GPIO_95_64DAT;
				detect_reg_bit = 17;
			 }
			 else if (lcm_data.user_config.hdmi_hpd_gpio == HDMI_HOTPLUG_GPIO75)
			 {
				 act_writel(act_readl(GPIO_MFCTL4)|0x00100000,GPIO_MFCTL4);// open HDMI_HOTPLUG_IN function by LYP 2013-3-5 
				 printk("GPIO_MFCTL4=0x%x reg[0xB0050004]=0x%x\n",act_readl(GPIO_MFCTL4), act_readl(0xB0050004));
				 detect_reg_addr = 0xB0050004;
				 detect_reg_bit = 29;
			 }
			 else
			 {
				detect_reg_addr = 0;
				detect_reg_bit = 0;
			 }
			#if  (CONFIG_AM_CHIP_MINOR==8258 ||CONFIG_AM_CHIP_MINOR==8268)
			if(1)
			#else
			if ((detect_reg_addr) && (detect_reg_bit) && (act_readl(detect_reg_addr)&(1<<detect_reg_bit)))//open HDMI_HOTPLUG_IN GPIO81	
			#endif
			{
				lcm_data.current_device = E_LCDPADMODE_HDMI;
				if (lcm_data.user_config.device_type == E_LCDPADMODE_HDMI)
					lcm_data.current_format = lcm_data.user_config.output_format;
				else
				{
					lcm_data.current_format = FMT_1650x750_1280x720_60P;
					close_device(lcm_data.user_config.device_type);
				}	
				printk("LCM Driver[HDMI Default] device = %d format= %d\n",lcm_data.current_device,lcm_data.current_format);
			}
			else
			{
				lcm_data.current_device = E_LCDPADMODE_VGA;
				if (lcm_data.user_config.device_type == E_LCDPADMODE_VGA)
					lcm_data.current_format = lcm_data.user_config.output_format;
				else
				{
					lcm_data.current_format = VGA_1280_720;
					close_device(lcm_data.user_config.device_type);
				}	
				printk("LCM Driver[VGA Default] device = %d format= %d\n",lcm_data.current_device,lcm_data.current_format);
			}
		}
	}
	else
	{
		edid_vga_status = 1;
		edid_hdmi_status = 1;
	}	
#endif	
	switch(lcm_data.current_device){
		case E_LCDPADMODE_CPU: 			
		case E_LCDPADMODE_LCD:			
		case E_LCDPADMODE_TCON:
		case E_LCDPADMODE_DTCON:
		case E_LCDPADMODE_LVDS:
//add for boot display if boot display is used only init sdram data
			if(RegBitRead(DE_Con,9,9) == 1)
				init_lcd_data(lcm_data.current_format);
			else				
				init_lcd(lcm_data.current_format);
			break;
		case E_LCDPADMODE_BT:
			if(RegBitRead(DE_Con,9,9) == 1)
				init_bt_data(lcm_data.current_format);
			else
				init_bt(lcm_data.current_format);
			break;
		case E_LCDPADMODE_HDMI:
			init_hdmi(lcm_data.current_format);
			break;
		case E_LCDPADMODE_YPBPR:
			init_ypbpr(lcm_data.current_format);
			break;
		case E_LCDPADMODE_CVBS:
			if(RegBitRead(DE_Con,9,9) == 1)
				init_cvbs_data(lcm_data.current_format);
			else
				init_cvbs(lcm_data.current_format);
			break;
	
		case E_LCDPADMODE_VGA:
			init_vga(lcm_data.current_format);
			break;
		default:  //default set to lcd display
			
			break;
	}
	lcm_data.DeInfo.screen_output_format = lcm_data.current_format;

	return 0;
}



static inline long long us_gettimeofday()
{
	struct timeval tv;
	do_gettimeofday(&tv);

	return ((long long)tv.tv_sec)*1000*1000 + tv.tv_usec;

}



#ifdef LCM_DPB_USE_TIMER
static void lcm_dpb_timer_handle(void)
{
	int off_jiff = 0;
	long long cur_us;
	register LCM_DPB_T * tmp_dpb;
	register LCM_DPB_CTL * pList = pdpb_ctl;

	tmp_dpb = fifo_check_pop_head(pList->in_dpb_fifo);
	cur_us = us_gettimeofday();

	if(tmp_dpb&&(tmp_dpb->pts<=cur_us)){
		disable_irq(IRQ_DE);
		if(pList->cur_dpb){
			if(NULL==pList->prev_dpb){
				pList->prev_dpb_cache = *pList->cur_dpb;
				pList->prev_dpb = &pList->prev_dpb_cache;
			}else if(NULL==pList->prev_dpb2){
				pList->prev_dpb_cache2 = *pList->cur_dpb;
				pList->prev_dpb2 = &pList->prev_dpb_cache2;
			}else{
				fifo_instert_tail(pList->out_dpb_fifo, pList->cur_dpb);
				DPB_DEBUG_INFO("When display lose a dpb, new pts=%lld, old pts=%lld\n", tmp_dpb->pts, pList->cur_dpb->pts);
			}
		}

		pList->cur_dpb=fifo_pop_head(pList->in_dpb_fifo);
		if(pList->cur_dpb){
			act_writel(pList->cur_dpb->img_bus_addr, Frm_BA);
			act_writel(pList->cur_dpb->uv_img_bus_addr, UV_BA);
			act_writel(0x01, DB_WR);
			pList->dpb_cache = *pList->cur_dpb;
			pList->cur_dpb = &pList->dpb_cache;
		}

		enable_irq(IRQ_DE);
	}

	//calc off_jiff
	tmp_dpb = fifo_check_pop_head(pList->in_dpb_fifo);
	if(tmp_dpb){
		off_jiff = (int)(tmp_dpb->pts - cur_us);
		off_jiff = (off_jiff*HZ)/(1000*1000) + 1; //us->jiffies

		if((off_jiff > HZ) || (off_jiff<0)){
			DPB_DEBUG_INFO("off_jiff=%d, cur_us=%lld, pts=%lld\n", off_jiff, cur_us, tmp_dpb->pts);
			tmp_dpb->pts = 0;
			off_jiff = 1;
		}
		
		mod_timer(&pList->dpb_timer, jiffies + off_jiff);
		DPB_DEBUGP("calc off_jiff=%d\n", off_jiff);
	}
}
#endif

//for framerate counter
static int count;
static int am7x_lcm_ioctl(struct inode * inode, struct file * file, unsigned int cmd, unsigned long arg)
{
	struct semaphore *lcm_sem = &lcm_data.lcm_sem;
	DE_INFO	*DeInfo = &lcm_data.DeInfo;
	int format;
	LCM_DPB_T lcm_dpb, *tmp_dpb;
	int tmp;
	unsigned char *edid_hdmi=NULL;
	unsigned char *edid_vga=NULL;
	int Edid_Pass=0;
	if(SET_DPB_ADDR==cmd){
		copy_from_user((void *)&lcm_dpb, (void *)arg, sizeof(LCM_DPB_T));
#if 1	//only display frame rate
		if(pdpb_ctl->dpb_mode)
		{
			struct timeval tv;
			static int tv_sec=0;
			static int count_d=0;
			static int decoder_rate=0;

			count_d++;
			do_gettimeofday(&tv);
			if(tv.tv_sec!=tv_sec)
			{
				tv_sec=tv.tv_sec;
				decoder_rate=count_d;
				count_d=0;
				printk("f:%d,%d\n",decoder_rate,pdpb_ctl->in_dpb_fifo->len);
			}
		}
#endif
		if(LCM_DPB_NORMAL==pdpb_ctl->dpb_mode)
		{
			if((0>=lcm_dpb.pts)&&(pdpb_ctl->in_dpb_fifo->len>0)){
				DPB_DEBUG_INFO("dpb lose!pts=%lld, len=%d\n", lcm_dpb.pts, pdpb_ctl->in_dpb_fifo->len);
				do{
					tmp_dpb = fifo_pop_head(pdpb_ctl->in_dpb_fifo);
				}while(tmp_dpb&&(0==fifo_instert_tail(pdpb_ctl->out_dpb_fifo, tmp_dpb)));//instert always success, if dpb size < FIFO LEN, so not handle fail.
			}
			lcm_dpb.pts += us_gettimeofday();
		}
		
		if(0==fifo_instert_tail(pdpb_ctl->in_dpb_fifo, &lcm_dpb)){
			#ifdef LCM_DPB_USE_TIMER
			if((1==pdpb_ctl->in_dpb_fifo->len)){
				lcm_dpb_timer_handle();
			}
			#endif
			DPB_DEBUGP("(in)Len=%d, pts=%lld\n", pdpb_ctl->in_dpb_fifo->len, lcm_dpb.pts);
			return 0;
		}else{
			printk(KERN_ERR "[am7x_lcm_ioctl]fifo_instert_tail FAIL!!\n");
			return -EFAULT;
		}
	}else if(GET_DPB_FREE_ADDR==cmd){
		if((NULL!=pdpb_ctl->out_dpb_fifo)&&(tmp_dpb=fifo_pop_head(pdpb_ctl->out_dpb_fifo))){
			copy_to_user((void*)arg, (void *)tmp_dpb, sizeof(*tmp_dpb));
			DPB_DEBUGP("(out)Len=%d\n", pdpb_ctl->out_dpb_fifo->len);
			return 0;
		}
		return -EFAULT;
	}else if(GET_DPB_NOTDISPLAY==cmd){
		tmp = pdpb_ctl->in_dpb_fifo->len;
		copy_to_user((void*)arg, (void *)&tmp, sizeof(int));
		return 0;
	}else if(SET_DPB_REAlTIMEMODE==cmd)
	{
		if(is_60p_flag)
		{
			pdpb_ctl->dpb_mode=arg;
			printk("set dpb real time mode:%d,is 60p\n",arg);	
		}
		else
			printk("set dpb real time mode,but not 60p\n");	
		return 0;
	}else if(SET_DPB_DDRSWAP==cmd)
	{
#if 0 //CONFIG_AM_CHIP_MINOR == 8268
		tmp=act_readl(CMU_DDRPLL);
		tmp=((tmp>>1) & 0x3f )*24;
		printk("8268 ddr Freq=%dM\n",tmp);	
		if( (tmp<800) && (FMT_1920x1080_60P == lcm_data.current_format))
			return 0;	// not open swap,or HDMI Splash
#endif	
		tmp=act_readl(SDR_ADDRSWAP);
		if(tmp!=arg)
		{
			act_writel(arg,SDR_ADDRSWAP);
			printk("SET_DPB_DDRSWAP=0x%x\n",arg);	
		}
		return 0;
	}else if(GET_DPB_DDRSWAP_INFO==cmd)
	{
		unsigned int swap_bankaddr=0;
#if CONFIG_AM_CHIP_MINOR == 8268
#if (MODULE_CONFIG_DDR_CAPACITY == 128)
		if(FMT_1920x1080_60P == lcm_data.current_format)
		{
			tmp=act_readl(SDR_ADDRSWAP);
			tmp= (tmp>>4)&0xff;
			if(0x80==tmp)
				swap_bankaddr=0x7000000;
			else if(0x40==tmp)
				swap_bankaddr=0x6000000;
		}
#endif
#endif
		copy_to_user((void*)arg, &swap_bankaddr, sizeof(unsigned int));
		return 0;
	}
	else if(SET_DPB_FIFO_MODE_PAUSE==cmd){
		copy_from_user((void *)&tmp, (void *)arg, sizeof(int));
		if(0==tmp){
			pdpb_ctl->dpb_fifo_flag = LCM_DPB_PAUSE;
			#ifdef LCM_DPB_USE_TIMER
			mod_timer(&pdpb_ctl->dpb_timer, jiffies + 0x0FFFFFFF);
			#endif
			DPB_DEBUGP("dpb lcm PAUSE!\n");
		}else{
			pdpb_ctl->dpb_fifo_flag = LCM_DPB_RUN;
			#ifdef LCM_DPB_USE_TIMER
			if(pdpb_ctl->in_dpb_fifo->len>0){
				lcm_dpb_timer_handle();
			}
			#endif
			DPB_DEBUGP("dpb lcm RESUME!\n");
		}

		return 0;
	}else if(GET_DPB_USING_ADDR==cmd){
		if((NULL!=pdpb_ctl->in_dpb_fifo)&&(tmp_dpb=fifo_pop_head(pdpb_ctl->in_dpb_fifo))){
			copy_to_user((void*)arg, (void *)tmp_dpb, sizeof(*tmp_dpb));
			DPB_DEBUGP("GET(in)Len=%d\n", pdpb_ctl->in_dpb_fifo->len);
			return 0;
		}
		return -EFAULT;
	}
	
	down(lcm_sem);
	switch(cmd){
		case GET_DE_CONFIG:
			DeInfo->input_luma_bus_addr=de_isr_data.current_image_yaddr;
			DeInfo->input_chroma_bus_addr=de_isr_data.current_image_caddr;
			if(DeInfo->osd_no==0)
			{
				DeInfo->osd_addr=de_isr_data.current_osd0_addr;
				DeInfo->osd_sw=RegBitRead(OSD0con,19,19);
			}
			else if(DeInfo->osd_no==1)
			{
				DeInfo->osd_addr=de_isr_data.current_osd1_addr;
				DeInfo->osd_sw=RegBitRead(OSD1con,19,19);
			}	
			copy_to_user((void*)arg,(void*)DeInfo,sizeof(*DeInfo));

			break;
		case UPDATE_DE_CONFIG:
		case COLOR_ADJUST:			   
		case SET_DISPLAY_MODE:  
		case CHANGE_FRAME_ADDR:	
		case SET_OSD:	          
		case WRITE_OSDINDEX:	  
		case SET_PALLET:	      
		case SET_PALLET256:
		case SHARPNESS_ADJUST:
		case SET_CURSOR:
			copy_from_user((void*)DeInfo,(void*)arg,sizeof(*DeInfo));	
			// reinit gamma data; avoid user space config
			lcm_data.DeInfo.Gamma = lcm_data.user_color_config.gamma;
			de_set(cmd,DeInfo);	
			break;	
		case GET_MONSPECS:
			copy_to_user((void*)arg,(void*)&(lcm_data.monA),sizeof(lcm_data.monA));
			break;
		case GET_MODEDB:
			copy_to_user((void*)arg,(void*)(lcm_data.monA.modedb),lcm_data.monA.modedb_len *sizeof(struct fb_videomode));
			break;
		case GAMMA_ADJUST:	
			copy_from_user((void*)DeInfo,(void*)arg,sizeof(*DeInfo));	
			copy_from_user(lcm_data.user_color_config.gamma,DeInfo->Gamma,sizeof(lcm_data.user_color_config.gamma));
			lcm_data.DeInfo.Gamma = lcm_data.user_color_config.gamma;
			de_set(cmd,DeInfo);	
			break;	
		case HDMI_OPEN:
#if MODULE_CONFIG_EZCASTPRO_MODE==8075 || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
			if (lcm_data.current_device&VGA_HDMI_SYNC_MODE)
			{
				close_device(E_LCDPADMODE_HDMI);
				close_device(E_LCDPADMODE_VGA);
			}
			else
#endif		
			{
				close_device(lcm_data.current_device);
			}
			copy_from_user(&format,(void*)arg,sizeof(int));
			lcm_data.current_device = E_LCDPADMODE_HDMI;
			lcm_data.current_format = format;
			lcm_data.DeInfo.screen_output_format = lcm_data.current_format;
			init_hdmi(format);			
			break;
		case LCD_OPEN:
			close_device(lcm_data.current_device);
			copy_from_user(&format,(void*)arg,sizeof(int));
			init_lcd(format); // lcd need not format
			//Notice LCD device open should only be used when 
			//   lcd is the default device
			lcm_data.current_device = lcm_data.user_config.device_type;
			lcm_data.current_format = format;
			lcm_data.DeInfo.screen_output_format = lcm_data.current_format;
			
			break;
		case CVBS_OPEN:
			close_device(lcm_data.current_device);
			copy_from_user(&format,(void*)arg,sizeof(int));
			init_cvbs(format);
			lcm_data.current_device = E_LCDPADMODE_CVBS;
			lcm_data.current_format = format;
			lcm_data.DeInfo.screen_output_format = lcm_data.current_format;
			break;
		case YPbPr_OPEN:
			close_device(lcm_data.current_device);
			copy_from_user(&format,(void*)arg,sizeof(int));
			init_ypbpr(format);
			lcm_data.current_device = E_LCDPADMODE_YPBPR;
			lcm_data.current_format = format;
			lcm_data.DeInfo.screen_output_format = lcm_data.current_format;
			break;			
		case VGA_OPEN:
#if MODULE_CONFIG_EZCASTPRO_MODE==8075 || (defined(MODULE_CONFIG_EZWIRE_ENABLE) && MODULE_CONFIG_EZWIRE_ENABLE!=0)
			if (lcm_data.current_device&VGA_HDMI_SYNC_MODE)
			{
				close_device(E_LCDPADMODE_HDMI);
				close_device(E_LCDPADMODE_VGA);
			}
			else
#endif		
			{
				close_device(lcm_data.current_device);
			}
			copy_from_user(&format,(void*)arg,sizeof(int));
			lcm_data.current_device = E_LCDPADMODE_VGA;
			lcm_data.current_format = format;
			lcm_data.DeInfo.screen_output_format = lcm_data.current_format;
			init_vga(format);				
			break;
		case SET_DPB_FIFO_MODE_START:
			#define DPB_MAX_NUM	(32)//need 2 power
			copy_from_user((void *)&tmp, (void *)arg, sizeof(int));
			tmp = DPB_MAX_NUM;
			printk("Use DE FIFO to display!maxLen=%d\n", tmp);
			count=0;
			disable_irq(IRQ_DE);
			memset(pdpb_ctl, 0, sizeof(LCM_DPB_CTL));	//reset
			pdpb_ctl->in_dpb_fifo = kzalloc(sizeof(LCM_DPB_FIFO_T)+sizeof(LCM_DPB_T)*tmp, GFP_KERNEL);
			if(pdpb_ctl->in_dpb_fifo){
				pdpb_ctl->in_dpb_fifo->maxLen = tmp;
				fifo_reset(pdpb_ctl->in_dpb_fifo);
				
				pdpb_ctl->out_dpb_fifo = kzalloc(sizeof(LCM_DPB_FIFO_T)+sizeof(LCM_DPB_T)*tmp, GFP_KERNEL);
				if(pdpb_ctl->out_dpb_fifo){
					pdpb_ctl->out_dpb_fifo->maxLen = tmp;
					fifo_reset(pdpb_ctl->out_dpb_fifo);

					spin_lock_init(&pdpb_ctl->in_dpb_fifo->lock);
					spin_lock_init(&pdpb_ctl->out_dpb_fifo->lock);
					pdpb_ctl->dpb_fifo_flag = LCM_DPB_RUN;
					
					#ifdef LCM_DPB_USE_TIMER
					init_timer(&pdpb_ctl->dpb_timer);
					pdpb_ctl->dpb_timer.expires = jiffies + 0x0fffffff;
					pdpb_ctl->dpb_timer.function = (void*)lcm_dpb_timer_handle;
					add_timer(&pdpb_ctl->dpb_timer);
					#endif
				}else{
					kfree(pdpb_ctl->in_dpb_fifo);
					pdpb_ctl->in_dpb_fifo = NULL;
					printk(KERN_ERR "malloc fail when out_dpb_fifo[%s]\n", __func__);
				}
			}else{
				printk(KERN_ERR "malloc fail when in_dpb_fifo[%s]\n", __func__);
			}

			enable_irq(IRQ_DE);
			break;


		case SET_DPB_FIFO_MODE_END:
			#ifdef LCM_DPB_USE_TIMER
			del_timer_sync(&pdpb_ctl->dpb_timer);
			#endif
			disable_irq(IRQ_DE);
			if(pdpb_ctl->in_dpb_fifo){
				kfree(pdpb_ctl->in_dpb_fifo);
				pdpb_ctl->in_dpb_fifo = NULL;
			}
			if(pdpb_ctl->out_dpb_fifo){
				kfree(pdpb_ctl->out_dpb_fifo);
				pdpb_ctl->out_dpb_fifo = NULL;
			}
			memset(pdpb_ctl, 0, sizeof(*pdpb_ctl));
			pdpb_ctl->dpb_fifo_flag = LCM_DPB_STOP;
			enable_irq(IRQ_DE);
			printk("SET_DPB_FIFO_MODE_END count=%d\n",count);
			break;
		case HDMI_EDID_READ:
		#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
			edid_hdmi=edid_i2c_hw_read();
			printk("edid_hdmi addr=0x%x\n",edid_hdmi);
			if(edid_hdmi){
				Edid_Pass=1;	
			}else{
				Edid_Pass=0;
			}
			if(edid_hdmi)
				kfree(edid_hdmi);
			copy_to_user((void*)arg,(void*)(&Edid_Pass),sizeof(Edid_Pass));
		#endif
			break;
		case VGA_EDID_READ:
		#if ((CONFIG_AM_CHIP_ID == 1213) && (EDID_ENABLE ==1))
			#ifndef MODULE_CONFIG_UCLIBC
			edid_vga=edid_i2c_gpio_read();
			#endif
			printk("edid_vga addr=%x\n",edid_vga);
			if(edid_vga){
				Edid_Pass=1;	
			}else{
				Edid_Pass=0;
			}
			if(edid_vga)
				kfree(edid_vga);
			copy_to_user((void*)arg,(void*)(&Edid_Pass),sizeof(Edid_Pass));
		#endif
			break;
		default:
			up(lcm_sem);
			return -EFAULT;
	}
	lcm_data.DeInfo.screen_output_format = lcm_data.current_format;
	up(lcm_sem);
	return 0;
}

static int am7x_lcm_open(struct inode * inode, struct file * filp)
{
/**/
	return 0;
}

static struct file_operations lcm_fops ={
	.open = am7x_lcm_open,
	.ioctl = am7x_lcm_ioctl,
};

/*****************************************************/
/*****************************************************
	LCM Device Driver :main Datastructure.

	struct platform_driver {
		int (*probe)(struct platform_device *);
		int (*remove)(struct platform_device *);
		void (*shutdown)(struct platform_device *);
		int (*suspend)(struct platform_device *, pm_message_t state);
		int (*suspend_late)(struct platform_device *, pm_message_t state);
		int (*resume_early)(struct platform_device *);
		int (*resume)(struct platform_device *);
		struct pm_ext_ops *pm;
		struct device_driver driver;
};

	The probe functions will be call when the platform device is registered
	to the system(Am7x_platformdevice_register)
	
*****************************************************/
/*****************************************************/


static irqreturn_t de_isr(int irq, void *dev_id)
{
	int i;
	long long cur_us;
	register LCM_DPB_T * tmp_dpb;
	register LCM_DPB_CTL * pList = pdpb_ctl;
	
	
//	printk("de_isr:%x\n",act_readl(DE_INT));
	if((RegBitRead(DE_INT,10,0)&0x404)!=0x0)
	{
		if(de_isr_data.de_lamp==1)
		{	
			//printk("\npost\n");
			up(&(de_isr_data.de_sem));
			de_isr_data.de_lamp=0;
		}

		if(LCM_DPB_REALTIME_FAST==pList->dpb_mode)
		{
			if(pList->in_dpb_fifo->len>0)
			{//set next dpb
				pList->dpb_cache = *fifo_pop_head(pList->in_dpb_fifo);
				pList->cur_dpb = &pList->dpb_cache;
				act_writel(pList->cur_dpb->img_bus_addr, Frm_BA);
				act_writel(pList->cur_dpb->uv_img_bus_addr, UV_BA);
				act_writel(0x01, DB_WR);
			}
		}
		else{
		de_isr_data.current_image_yaddr=act_readl(Frm_BA);
		de_isr_data.current_image_caddr=act_readl(UV_BA);
		de_isr_data.current_osd0_addr=act_readl(OSD0_BA);
		de_isr_data.current_osd1_addr=act_readl(OSD1_BA);

		if(pList->prev_dpb&&(de_isr_data.current_image_yaddr!=pList->prev_dpb->img_bus_addr)){
			fifo_instert_tail(pList->out_dpb_fifo, pList->prev_dpb);
			pList->prev_dpb = NULL;
		}

		#ifdef LCM_DPB_USE_TIMER
		if(pList->prev_dpb2&&(de_isr_data.current_image_yaddr!=pList->prev_dpb2->img_bus_addr)){
			fifo_instert_tail(pList->out_dpb_fifo, pList->prev_dpb2);
			pList->prev_dpb2 = NULL;
		}
		#endif
		
		#ifdef LCM_DPB_USE_IRQ
		if(pList->in_dpb_fifo&&pList->in_dpb_fifo->len>0){//set next dpb
		
			if(LCM_DPB_REALTIME==pList->dpb_mode)
			{
				i=1;
			}
			else
			{
				cur_us = us_gettimeofday()+4500;
				
				i = 0;
				while(tmp_dpb = fifo_check_data(pList->in_dpb_fifo, i)){
					if(tmp_dpb->pts  > cur_us){
						break;
					}
					++i;
				}
			}
			
			if(i>0){
				while(--i>0){
					tmp_dpb = fifo_pop_head(pList->in_dpb_fifo);
					fifo_instert_tail(pList->out_dpb_fifo, tmp_dpb);
					DPB_DEBUG_INFO("lcm INT lose dpb.\n");
					DPB_DEBUGP("Lose pts=%lld\n", tmp_dpb->pts);
				}

				if(pList->cur_dpb){
					if(NULL==pList->prev_dpb){
						pList->prev_dpb_cache = *pList->cur_dpb;
						pList->prev_dpb = &pList->prev_dpb_cache;
					}else{
						fifo_instert_tail(pList->out_dpb_fifo, pList->cur_dpb);
						DPB_DEBUG_INFO("When display lose a dpb\n");
					}
				}

				pList->dpb_cache = *fifo_pop_head(pList->in_dpb_fifo);
				pList->cur_dpb = &pList->dpb_cache;
				act_writel(pList->cur_dpb->img_bus_addr, Frm_BA);
				act_writel(pList->cur_dpb->uv_img_bus_addr, UV_BA);
				act_writel(0x01, DB_WR);
				count++;
				DPB_DEBUGP("dis pts=%lld, cur us=%lld\n", pList->cur_dpb->pts, cur_us);
			}

		}
		#endif
		}
		
	}

#ifdef FIFO_DBG
	{
		int i;
		
		for(i=4;i<7;i++)
		{
			if(RegBitRead(DE_INT,i,i)==1)				
				printk("DE_INT bit %d is empty\n",i);				
		}
		for(i=0;i<10;i++)
			act_writel(0x4f4, DE_INT);	
	}
#endif

CLEAR_IRQ:
	for(i=0;i<10;i++)
		act_writel(0x404, DE_INT);

	return IRQ_RETVAL(1);
}

static int __devinit am7x_lcm_probe(struct platform_device *pdev)
{
	int result;
	printk("lcm probe\n");
/*
 *	Basic hardware initialize
*/	
	am7x_display_module_init();

/*
 *	irq initialize
*/	
	sema_init(&(de_isr_data.de_sem), 0);
	sema_init(&(lcm_data.lcm_sem),1);
	result = request_irq(de_irq_no, de_isr,IRQF_DISABLED, "de",NULL);
	if(result == -EINVAL)
	{
		printk(KERN_ERR "LCM: Bad irq number or handler\n");
		return result;
	}
	else if(result == -EBUSY)
	{
		printk(KERN_ERR "LCM: IRQ %d busy, change your config\n",de_irq_no);
		return result;
	}

	de_isr_data.de_isr_init_flag = 1;

/*
 *	Register the  lcm char device to system	
*/
	if (register_chrdev(AM7X_LCM_MAJOR,"lcm",&lcm_fops))
		printk("unable to get major %d for memory devs\n", FB_MAJOR);
	return 0;
}
/*
  * 	These functions maybe used later
 */

static int __devexit am7x_lcm_remove(struct platform_device *pdev)
{
//	am7x_lcm_release_config();
	struct fb_monspecs *monA = NULL;
	monA = &(lcm_data.monA); //kzalloc(sizeof(struct fb_monspecs), GFP_KERNEL);
	if ((monA) && (monA->modedb))
		fb_destroy_modedb(monA->modedb);
	return 0;
}


int am7x_lcm_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct lcm_config *user_config = &lcm_data.user_config;

	printk("lcm suspend\n");
	start_config();
	if(user_config->lvds_en){
		RegBitSet(0,LVDS_CTL,0,0);	
		lcm_data.lvds_ana_ctl2 =act_readl(LVDS_ANA_CTL2);
		act_writel(0x0,LVDS_ANA_CTL2);  
	}
	RegBitSet(0,CMU_DISPLAYCLK,8,7);
	RegBitSet(0,CMU_DEVCLKEN,1,1);//LCD
	RegBitSet(0,CMU_DEVCLKEN,14,14);	
	RegBitSet(0,CMU_DEVCLKEN,12,12);
	/*BTVD hclk enable.*/
	RegBitSet(0,CMU_DEVCLKEN,30,30);	
	return 0;
}

int am7x_lcm_resume(struct platform_device *pdev)
{
	struct lcm_config *user_config = &lcm_data.user_config;

	printk("lcm resume\n");


	RegBitSet(1,CMU_DEVCLKEN,1,1);//LCD
	RegBitSet(1,CMU_DEVCLKEN,14,14);	
	RegBitSet(1,CMU_DEVCLKEN,12,12);  //tcon
	
	/*BTVD hclk enable.*/
	RegBitSet(1,CMU_DEVCLKEN,30,30);		
	RegBitSet(3,CMU_DISPLAYCLK,8,7);

	if(user_config->lvds_en){
		RegBitSet(1,LVDS_CTL,0,0);	
		act_writel(lcm_data.lvds_ana_ctl2,LVDS_ANA_CTL2);  
	}	
	end_config();
	return 0;	
}

static struct platform_driver am7x_lcm_driver = {
	.probe = am7x_lcm_probe,
	.remove = am7x_lcm_remove,
	.suspend = am7x_lcm_suspend, /* optional but recommended */
	.resume = am7x_lcm_resume,   /* optional but recommended */
	.driver = {
		.name = "lcm-am7x",  //same name as in platform
	},
};

static int __init am7x_lcm_init(void)
{
	pdpb_ctl->dpb_fifo_flag = LCM_DPB_STOP;
	return  platform_driver_register(&am7x_lcm_driver);
}

static void __exit am7x_lcm_exit(void)
{
	platform_driver_unregister(&am7x_lcm_driver);
}

/*
  * Modularization 
 */

module_init(am7x_lcm_init);
module_exit(am7x_lcm_exit);
 
MODULE_LICENSE("GPL");
