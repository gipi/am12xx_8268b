 /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7x_display.c

@abstract: actions-mircro display device driver source file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Display Engine driver;
 *  include framebuffer,osd	
 *
 *
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "linux/kernel.h"
#include "linux/slab.h"
#include "linux/delay.h"

#include "am7xfb.h"
#include "sys_cfg.h"
#include <actions_io.h>
#include <actions_regs.h>
#include "am7x_gpio.h"
#include "am7x_board.h"


#define LCM_CONFIG_PATH  "/am7x/case/data/fb.bin"


#define SUPPORT_LCD
#define SUPPORT_HDMI
#define SUPPORT_CVBS
#define SUPPORT_YPBPR
#define SUPPORT_BTTV


#ifdef SUPPORT_LCD
static struct am7x_lcd_data   lcd_data;  
#endif


#ifdef SUPPORT_HDMI
static struct am7x_hdmi_data  hdmi_data;
#endif

#ifdef SUPPORT_CVBS
static struct am7x_cvbs_data  cvbs_data;		
#endif

#ifdef SUPPORT_YPBPR
static struct am7x_ypbpr_data ypbpr_data;
#endif

#ifdef SUPPORT_BTTV
static struct am7x_bttv_data bttv_data;
#endif

void RegBitSet(int val,int reg,int msb,int lsb){                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);		        
} 


static int lcd_mode(void* priv_data)
{
	
	return RGB;
}

static int hdmi_mode(void* priv_data)
{
	struct am7x_hdmi_data* hdmi = (struct am7x_hdmi_data*)priv_data;
	if(hdmi->need_even)
		return RGB_I;
	return RGB;
}

static int ypbpr_mode(void* priv_data)
{
	
	return RGB;
}

int cacl_display_mode(struct am7x_display_device* device)
{
	int mode = RGB ;
	switch(device->device_type)
	{
		case LCD:
			mode = lcd_mode(device->priv_data);	
			break;
		case HDMI:
			mode = hdmi_mode(device->priv_data);
			break;
		case CVBS:
			mode = BT_I;
			break;
		case YPbPr:
			mode = ypbpr_mode(device->priv_data);
			break;
		case BTTV:
			mode = BT_I;
			break;
		default:
			printk("error device type:%x\n",device->device_type);
			break;

	}
	
	return mode;
}

int cacl_lcd_colorspace(struct am7x_lcd_data *lcd)
{
	// 888 
	if(LCD_24P == lcd->lcm_rgbformat)
		return RGB_888;
	// 666
	return RGB_666;
}



int set_display_device_data(struct am7x_display_device* device,void* data)
{
	device->priv_data = data;
	return 0;
}

void* get_display_device_data(struct am7x_display_device* device)
{
	return (void*)device->priv_data;
}

int config_lcd(void)  //only process normal lcd panel now
{
	int ret = 0;
	ret = am_get_config(LCM_CONFIG_PATH, &lcd_data, 0, sizeof(lcd_data));
#if 0
	lcd->clk = 30; //30Mhz
	lcd->lcd_width = PanelWidth;
	lcd->lcd_height = PanelHeight;
	lcd->lcd_ctl = 0x4;
	lcd->lcd_rgbctl = 0x00002001;
	lcd->lcd_default_color = 0xff0000;
	lcd->lcd_htiming = 0x198f0;
	lcd->lcd_vtiming_odd = 0x140500a;
	lcd->lsp = 0x1;
#endif	
	if(ret)
	{
		printk("error framebuffer config\n");
	}
	return ret;	
}

int config_hdmi(int format)
{
	struct am7x_hdmi_data* hdmi = &hdmi_data;

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
	switch(format)
	{
		case	FMT_858x525_720x480_60I:
				SIZE_H =(240-1);			
 				SIZE_W =(720-1);			
				HSPW =(124);	
 				HFP =(38-1);			
 				HBP =(124+114-1);
 				VSPWo =(3-1);
 				VFPo =(4-1);
 				VBPo = (18-1);
 				VSPWe = (3-1);
 				VFPe = (5-1);
 				VBPe = (18-1);
				need_even = 1;
				break;
		case 	FMT_864x625_720x576_50I: 
				SIZE_H =(288-1);			
 				SIZE_W =(720-1);			
				HSPW =(126);	
 				HFP =(24-1);			
 				HBP = (126+138-1);
 				VSPWo =(3-1);
 				VFPo =(2-1);
 				VBPo = (22-1);
 				VSPWe = (3-1);
 				VFPe = (3-1);
 				VBPe = (22-1);
				need_even = 1;
				break;
		case	FMT_2200x1125_1920x1080_60I:
				SIZE_H =(540-1);			
				SIZE_W =(1920-1);			
				HSPW =(44); 
				HFP =(88-1);			
				HBP =(44+148-1);
				VSPWo =(5-1);
				VFPo =(2-1);
				VBPo = (5+15-1);
				VSPWe = (5-1);
				VFPe = (3-1);
				VBPe = (5+15-1);
				need_even = 1;
				break;
		case 	FMT_2640x1125_1920x1080_50I:	
				SIZE_H =(540-1);			
 				SIZE_W =(1920-1);			
				HSPW =(44);	
 				HFP =(88-1);			
 				HBP =(44+148-1);
 				VSPWo =(5-1);
 				VFPo =(2-1);
 				VBPo = (5+15-1);
 				VSPWe = (5-1);
 				VFPe = (3-1);
 				VBPe = (5+15-1);
				need_even = 1;
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
		case	FMT_1980x750_1280x720_50P:
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
		
		case	FMT_2640x1125_1920x1080_25P:
		case	FMT_2750x1125_1920x1080_24P:			
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (43-1);						
				HFP = (638-1);			
				HBP = (192-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);			
				break;
		case 	FMT_2200x1125_1920x1080_30P:
				SIZE_H =(1080-1);
				SIZE_W = (1920-1);			
				HSPW = (43-1);						
				HFP = (538-1);			
				HBP = (192-1);						
				VSPWo = (5-1);			
				VFPo =(4-1);			
				VBPo =(41-1);				
				break;
		default:
				printk("not supported format:%x\n",format);
				break;
	}

	if(format >= FMT_858x525_720x480_60P)
		hdmi->clk = 27;
	else	
		hdmi->clk = 75;
	hdmi->timing_format = format;
	hdmi->lcd_width = SIZE_W;
	hdmi->lcd_height = SIZE_H;
	hdmi->lcd_ctl = 0x6;
	hdmi->lcd_rgbctl = 0x00002001;
	hdmi->lcd_default_color = 0xff0000;
	hdmi->lcd_htiming = (HSPW<<22)|(HFP<<11)|(HBP);
	hdmi->lcd_vtiming_odd = (VSPWo<<22)|(VFPo<<11)|(VBPo);
	if(need_even == 1){
		hdmi->lcd_vtiming_even = (VSPWe<<22)|(VFPe<<11)|(VBPe);
		hdmi->lcd_vsync_edge = (291-1);
	}
	hdmi->lsp = 0x1;
	hdmi->need_even = need_even;
	
	return 0;	
}

int config_cvbs(int format)
{
	return 0;
	
}

int config_ypbpr(int format)
{
	return 0;

}

int config_bttv(void)
{
	return 0;

}

int set_display_clk(int clk)  //set lcdpll and divider
{
	int pll ;
	int div  ;
	
	switch(clk){
		case 27:
			pll = 72;  //72*1.5= 108 M
			div = 3;	//div = 4
		case 30:  //lcd 
			pll = 80; //80*1.5 = 120 M
			div = 3; // div = 4	
			break;
		case 50:
			pll = 100; //100*1.5 = 150M
			div = 2;   //div = 3 	
		case 75: //hdmi
			pll = 100;  //100*1.5 = 150 M
			div = 1;	// div = 2
			break;	
		default:
			pll = 72;
			div = 3;
			break;
	}
#if CONFIG_AM_CHIP_MINOR == 8268
	act_writel(0x00002541,CMU_LCDPLL);
#else
  	act_writel(0x13d52081,CMU_LCDPLL);
#endif
  	act_writel(0x00000081,CMU_DISPLAYCLK);		

#if CONFIG_AM_CHIP_MINOR == 8268
	RegBitSet(pll,CMU_LCDPLL,7,1);
#else
	RegBitSet(pll,CMU_LCDPLL,8,2);
#endif
	RegBitSet(div,CMU_DISPLAYCLK,3,0);
	RegBitSet(1,CMU_DISPLAYCLK,7,7);  //LCD /DE
	RegBitSet(1,CMU_LCDPLL,0,0);
	udelay(100);  // wait the hardward to be stable

	printk("CMU_LCDPLL:%x\n",act_readl(CMU_LCDPLL));
	printk("CMU_DISPLAYCLK:%x\n",act_readl(CMU_DISPLAYCLK));
	
	return 0;
}

int disable_display_clk(void)
{
	RegBitSet(0,CMU_LCDPLL,0,0);
	return 0;
}

void mfp_config(void)
{
	if(lcd_data.lcm_type == LCD_TTL){
		
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
		if(LCD_24P == lcd_data.lcm_rgbformat){ //888
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
		if(LCD_24P == lcd_data.lcm_rgbformat){
			RegBitSet(0x2,GPIO_MFCTL7,14,12); //LCD D0
			RegBitSet(0x2,GPIO_MFCTL7,18,16); //LCD D1
			RegBitSet(0x1,GPIO_MFCTL2,15,12); //LCD D8
			RegBitSet(0x1,GPIO_MFCTL2,11,8);  //LCD D9
			RegBitSet(0x2,GPIO_MFCTL7,18,16); //LCD D16
			RegBitSet(0x1,GPIO_MFCTL1,22,20); // LCD D17
		}
		
		RegBitSet(3,GPIO_MFCTL1,9,8); //LCD d2
		RegBitSet(1,GPIO_MFCTL1,5,4); //LCD d3~d7
		RegBitSet(1,GPIO_MFCTL1,0,0); //LCD d10~d11
		RegBitSet(1,GPIO_MFCTL0,29,28); //LCD d12~d13
		RegBitSet(1,GPIO_MFCTL0,25,24); //LCD d14~d15
		RegBitSet(1,GPIO_MFCTL0,21,20); //LCD d18
		RegBitSet(1,GPIO_MFCTL0,17,16); //LCD d19~d23
		RegBitSet(1,GPIO_MFCTL7,25,24); //LCD enb
		RegBitSet(1,GPIO_MFCTL7,29,28); //LCD clk
		RegBitSet(1,GPIO_MFCTL8,1,0); //LCD vsync
		RegBitSet(1,GPIO_MFCTL8,5,4); //LCD d8	
#endif

	}
	
	
}

void lvds_config(void)
{
	if(lcd_data.lvds_en){
		//lvds special clock
		act_writel(0x80,CMU_DISPLAYCLK2);
		act_writel(0x70,LVDS_ANA_CTL2);  

		act_writel(0xfdf83,LVDS_CTL);
		act_writel(0x70,LVDS_ANA_CTL2);  
		RegBitSet(lcd_data.lvds_format,LVDS_CTL,1,1);
		RegBitSet(lcd_data.lvds_channel,LVDS_CTL,2,2);
		RegBitSet(lcd_data.lvds_swap,LVDS_CTL,5,5);
		RegBitSet(lcd_data.lvds_map,LVDS_CTL,4,3);	
		RegBitSet(lcd_data.lvds_mirror,LVDS_CTL,6,6);
		RegBitSet(4,0xb0010090,28,26);
		RegBitSet(1,LVDS_ANA_CTL1,16,16);
		RegBitSet(3,LVDS_ANA_CTL1,18,17);
		RegBitSet(1,LVDS_ANA_CTL1,21,19);
	//lvds need not set mfp,set lvds analog according to channel and format
		if((lcd_data.lvds_channel==0)&&(lcd_data.lvds_format==1)){//single 24bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(1,LVDS_ANA_CTL2,6,6);
			RegBitSet(0xaaaaa,LVDS_PAD_CFG0,19,0);
		
		}else if((lcd_data.lvds_channel==0)&&(lcd_data.lvds_format==0)){//single 18bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(0,LVDS_ANA_CTL1,10,8);
			RegBitSet(1,LVDS_ANA_CTL2,6,6);
			RegBitSet(0xaaaa,LVDS_PAD_CFG0,19,4);
		}else if((lcd_data.lvds_channel==1)&&(lcd_data.lvds_format==1)){//dual 24bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(3,LVDS_ANA_CTL2,7,6);
			RegBitSet(1,LVDS_ANA_CTL2,4,4);
			RegBitSet(0xaaaaa,LVDS_PAD_CFG0,19,0);
			RegBitSet(0xaaaaa,LVDS_PAD_CFG1,23,4);
			RegBitSet(1,CMU_DISPLAYCLK2,24,24);
		}else if((lcd_data.lvds_channel==1)&&(lcd_data.lvds_format==0)){//dual 18bit
			RegBitSet(3,LVDS_ANA_CTL1,24,23);
			RegBitSet(3,LVDS_ANA_CTL2,7,6);
			RegBitSet(1,LVDS_ANA_CTL2,4,4);
			RegBitSet(0xaaaa,LVDS_PAD_CFG0,19,4);
			RegBitSet(0xaaaa,LVDS_PAD_CFG1,23,8);
			RegBitSet(1,CMU_DISPLAYCLK2,24,24);
		}else{
			printk("lvds para error!\n");
		}
	
	}

//	act_writel(0xfdf81,LVDS_CTL); 
//	act_writel(0x19C8B40,LVDS_ANA_CTL1);
//	act_writel(0x70,LVDS_ANA_CTL2);    

	printk("LVDS_CTL:%x\n",act_readl(LVDS_CTL));
	printk("LVDS_ANA_CTL1:%x\n",act_readl(LVDS_ANA_CTL1));
	printk("LVDS_ANA_CTL2:%x\n",act_readl(LVDS_ANA_CTL2));
	printk("LVDS_PAD_CFG0:%x\n",act_readl(LVDS_PAD_CFG0));
	printk("CMU_DISPLAYCLK2:%x\n",act_readl(CMU_DISPLAYCLK2));	


}

void tcon_config(void)
{
	;
}

int hardware_lcd_init(struct am7x_display_device *device)
{
	struct am7x_lcd_data* lcd = (struct am7x_lcd_data*)get_display_device_data(device);

#if 1
	act_writel(1<<11|act_readl(CMU_DEVRST),CMU_DEVRST);  //lcd reset
	act_writel(1<<14|1<<1|act_readl(CMU_DEVCLKEN),CMU_DEVCLKEN); //lcd de clk enable



	mfp_config();
	lvds_config();
	tcon_config();

	//lcd controller
	act_writel(lcd->hbp|lcd->hfp<<11|lcd->hswp<<22,LCD_HTIMING);
	act_writel(lcd->vbp|lcd->vfp<<11|lcd->vswp<<22,LCD_VTIMING_ODD);
	act_writel(0x20001,LCD_LSP);
	act_writel(lcd->rgb_ctl,LCD_RGBCTL);
	act_writel(lcd->dac_ctl,LCD_DAC_CTL);
	act_writel((lcd->device_width-1)|(lcd->device_height-1)<<16,LCD_SIZE);
	
	act_writel(0x4,LCD_CTL);
	act_writel(0xff,LCD_CLR);

	set_display_clk(lcd->d_clk);
#endif

	

	return 0;
}

int hardware_lcd_close(struct am7x_display_device *device)
{
	act_writel(1<<11|act_readl(CMU_DEVRST),CMU_DEVRST);  //lcd reset
	act_writel((~1<<1)&act_readl(CMU_DEVCLKEN),CMU_DEVCLKEN);
	disable_display_clk();
	return 0;
}

int hardware_hdmi_init(struct am7x_display_device *device)
{
	struct am7x_hdmi_data* hdmi = (struct am7x_hdmi_data*)get_display_device_data(device);

	set_display_clk(hdmi->clk);
	

	act_writel(hdmi->lcd_ctl,LCD_CTL);	
	act_writel(hdmi->lcd_default_color,LCD_CLR);
	act_writel(hdmi->lcd_rgbctl,LCD_RGBCTL);
		
	act_writel(hdmi->lsp,LCD_LSP);
	act_writel(hdmi->lcd_htiming,LCD_HTIMING);
	act_writel(hdmi->lcd_vtiming_odd,LCD_VTIMING_ODD);
	act_writel((hdmi->lcd_height<<16)|(hdmi->lcd_width),LCD_SIZE);	
	if(hdmi->need_even == 1)
	{
		act_writel(hdmi->lcd_vtiming_even,LCD_VTIMING_EVEN);
		act_writel(hdmi->lcd_vsync_edge,LCD_VSYNC_EDGE);
	}
	
	return 0;
}

int hardware_hdmi_close(struct am7x_display_device *device)
{
	
	return 0;
}

int hardware_ypbpr_init(struct am7x_display_device *device)
{
	return 0;

}

int hardware_ypbpr_close(struct am7x_display_device *device)
{
	return 0;
}

int hardware_cvbs_init(struct am7x_display_device *device)
{
	return 0;
}

int hardware_cvbs_close(struct am7x_display_device *device)
{

	return 0;
}

int hardware_bttv_init(struct am7x_display_device *device)
{
	
	return 0;	
}

int hardware_bttv_close(struct am7x_display_device *device)
{

	return 0;
}


struct am7x_display_device* alloc_display_device(int type)
{
	struct am7x_display_device *device = NULL; 

	device = kzalloc(sizeof(struct am7x_display_device),GFP_KERNEL);
	if(device == NULL)
		panic("am7xfb malloc failed\n");

	device->device_type = type;

	return device;
}

void destroy_display_device(struct am7x_display_device *device)
{
	if(device)
		kfree(device);
}

int init_display_device(struct am7x_display_device *device)
{
	int ret = 0;
	void *data = NULL;
	char *des = NULL;
	int x_res =0,y_res =0;
	int colorspace = 0;
//	int display_mode = 0;

	
	if(device == NULL)
		return -1;

	switch(device->device_type)
	{
		case LCD:
			data = (void*)&lcd_data;	
			des = lcd_des;
			x_res = lcd_data.device_width;
			y_res = lcd_data.device_height;
			colorspace = cacl_lcd_colorspace(&lcd_data);
			break;
		case HDMI:
			data = (void*)&hdmi_data;
			des = hdmi_des;
			x_res = hdmi_data.lcd_width;
			y_res = hdmi_data.lcd_height;
			colorspace = RGB_888;			
			break;
		case YPbPr:  //to be finished 
			data = (void*)&ypbpr_data;
			des = ypbpr_des;
			break;
		case CVBS: //to be finished 
			data = (void*)&cvbs_data;
			des = cvbs_des;
			break;
		case BTTV: //to be finished 
			data = (void*)&bttv_data;
			des = bttv_des;
			break;
		default:
			printk(KERN_ERR"unknow display device type");
			return -1;
			break;		
	}

	set_display_device_data(device,data);
	device->description = des;
	device->x_res = x_res;
	device->y_res = y_res;
	device->colorspace = colorspace;
	

	return ret;	
}



/** just for demo/qc board  **/
struct am7x_display_device* init_device(struct am7x_display_device *device,int device_type)
{
	device = alloc_display_device(device_type);
	init_display_device(device);

	return device;
}

int config_device(int type,int  format)
{
	switch(type)	
	{
		case LCD:
			config_lcd();
			break;
		case HDMI:
			config_hdmi(format);
			break;
		case YPbPr:
			config_ypbpr(format);
			break;
		case CVBS:
			config_cvbs(format);
			break;
		case BTTV:
			config_bttv();
			break;
		default:
			printk("config device\n");
			break;
	}


	return 0;
}



int hardware_init_display_device(struct am7x_display_device *device)
{
	int ret = 0;

	if(device == 0)
		return ret;
	printk("device->device_type  %d\n",device->device_type);

	switch(device->device_type){
		case LCD:
			ret = hardware_lcd_init(device);
			break;
		case HDMI:
			ret = hardware_hdmi_init(device);
			break;
		case YPbPr:
			ret = hardware_ypbpr_init(device);
			break;
		case CVBS:
			ret = hardware_cvbs_init(device);
			break;
		case BTTV:
			ret = hardware_bttv_init(device);	
			break;
		default:
			printk(KERN_ERR"unsupported device type\n");
			ret = -1 ;
			break;
	}
	
	return ret;
}

int hardware_close_display_device(struct am7x_display_device *device)
{
	int ret = 0;

	if(device == 0)
		return ret;

	switch(device->device_type){
		case LCD:
			ret = hardware_lcd_close(device);
			break;
		case HDMI:
			ret = hardware_hdmi_close(device);
			break;
		case YPbPr:
			ret = hardware_ypbpr_close(device);
			break;
		case CVBS:
			ret = hardware_cvbs_close(device);
			break;
		case BTTV:
			ret = hardware_bttv_close(device);	
			break;
		default:
			printk(KERN_ERR"unsupported device type\n");
			ret = -1; 
			break;
	}
	
	return ret;	
}


