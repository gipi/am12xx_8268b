#ifndef  AM_LCM_H
#define AM_LCM_H

#include <linux/semaphore.h>
#include <linux/spinlock.h>


#define EDID_CONFIG_PATH  "/mnt/vram/edid.bin"
#define EDID_INFO_PATH  "/mnt/vram/edidinfo.bin"
#define LCM_CONFIG_PATH  "/am7x/case/data/lcm.bin"
#define LCM_GAMMA_PATH   "/am7x/case/data/gamma.bin"
#define DATA_OFFSET		0
#define de_irq_no 		26
#define MAX_CLK_RATE    190 
#define DEFAULT_CLK		30
#define EDID_LENGTH				0x80

/******HDMI config GPIO******/
#define HDMI_HOTPLUG_GPIO81		81
#define HDMI_HOTPLUG_GPIO75		75
#define HDMI_HDMI2MHL_GPIO39			39

/******display engine control******/
/**********fixme: *****************/
#define GET_DE_CONFIG  			0x00
#define DE_RELEASE     			0X01 
#define UPDATE_DE_CONFIG		0x02
#define COLOR_ADJUST			0x03
#define GAMMA_ADJUST	    	0x04
#define SET_DISPLAY_MODE  		0x05
#define CHANGE_FRAME_ADDR		0x06
#define SET_OSD	          		0x07
#define WRITE_OSDINDEX	  		0x08
#define SET_PALLET	      		0x09
#define SET_PALLET256     		0x0a
#define SHARPNESS_ADJUST		0x0b  //add for sharpness
#define SET_CURSOR				0x0c
#define GET_MONSPECS			0x0d
#define GET_MODEDB				0x0e

/******display device control******/
/**********fixme: *****************/
#define HDMI_OPEN				0x15
#define LCD_OPEN				0x16
#define CVBS_OPEN				0x17
#define YPbPr_OPEN				0x18
#define VGA_OPEN				0x19
#define CLOSE_DISPLAY 			0x1a  /**close all display device**/

//For dpb fifo mode
#define SET_DPB_FIFO_MODE_START		(0x1b)
#define SET_DPB_ADDR			(0x1c)
#define GET_DPB_FREE_ADDR		(0x1d)
#define SET_DPB_FIFO_MODE_END		(0x1e)
#define SET_DPB_FIFO_MODE_PAUSE	(0x1f)
#define GET_DPB_USING_ADDR		(0x20)
#define HDMI_EDID_READ          (0x21)
#define VGA_EDID_READ			(0x22)
#define SET_DPB_REAlTIMEMODE	(0x23)
#define SET_DPB_DDRSWAP			(0x24)
#define GET_DPB_NOTDISPLAY		(0x25)
#define GET_DPB_DDRSWAP_INFO	(0x26)

enum LCM_DPB_MODE{
	LCM_DPB_NORMAL= 0,
	LCM_DPB_REALTIME_FAST,
	LCM_DPB_REALTIME
};


/******EDID On/Off******/
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE==8074
#define EDID_ENABLE				0
#else
#define EDID_ENABLE				1
#endif

/******EDID mode******/
#define EDID_MODE_FOR_STDBOARD		0
#define EDID_MODE_FOR_EZCAST		1


#define LCM_VMODE_NONINTERLACED  0	/* non interlaced */
#define LCM_VMODE_INTERLACED	1	/* interlaced	*/
#define LCM_VMODE_DOUBLE		2	/* double scan */
#define LCM_VMODE_ODD_FLD_FIRST	4	/* interlaced: top line first */
#define LCM_VMODE_MASK		255



typedef struct __lcm_dpb{
	unsigned long dbp_vir_addr;
	unsigned long img_bus_addr;
	unsigned long uv_img_bus_addr;//img_bus_addr+width*height
	long long pts;//system time
}LCM_DPB_T;


typedef struct {
	spinlock_t lock;
	int start, end;	
	int len;
	int maxLen;
	LCM_DPB_T fifo[0];
} LCM_DPB_FIFO_T;


enum DISPLAY_DEVICE_TYPE
{
	E_LCDPADMODE_CPU = 0,		
	E_LCDPADMODE_LCD,			
	E_LCDPADMODE_TCON,
	E_LCDPADMODE_BT,
	E_LCDPADMODE_DTCON,
	E_LCDPADMODE_LVDS,
	E_LCDPADMODE_HDMI,
	E_LCDPADMODE_YPBPR,
	E_LCDPADMODE_CVBS,
	E_LCDPADMODE_VGA,
	E_LCDPADMODE_BUTT    	
};

/**
*@brief  CVBS timing format definations
*/
enum CVBS_FORMAT{
	PAL,
	NTSC
};

/**
*@brief	ypbpr timing format definations
*/
enum YPbPr_FORMAT{
	YPBPR_1080P,
	YPBPR_720P,
	YPBPR_576P,
	YPBPR_576I,
	YPBPR_480P,
	YPBPR_480I
};

/**
*@brief	vga timing format definations
*/
enum VGA_FORMAT{
	VGA_1280_1024,
	VGA_1280_768,
	VGA_1024_768,
	VGA_800_600,
	VGA_640_480,
	VGA_1280_800,
	VGA_1920_1080,
	VGA_1366_768,
	VGA_1280_720,
	VGA_1920_1080_30P,
	VGA_1280_800_71M,
	VGA_1366_768_85M,
	VGA_1280_768_79M,
	VGA_BUTT
};


/**
*@brief	lcd  color format definations
*/
typedef enum eLCDOutputFmt
{
	E_LCDOUTPUTFMT_24P = 0,		
	E_LCDOUTPUTFMT_18P,			
	E_LCDOUTPUTFMT_16P, 		
	E_LCDOUTPUTFMT_24S = 4,
	E_LCDOUTPUTFMT_18S,
	E_LCDOUTPUTFMT_BUTT    	
}E_LCDOUTPUTFMT;

/**
*@brief	for EDID videomode  definations
*/
typedef struct lcm_videomode{
	const char *name;	/* optional */
	unsigned int refresh;		/* optional */
	unsigned int xres;
	unsigned int yres;
	unsigned int vmode;
	unsigned int format;
};

/****************************************************
	Description: common functions used 
*****************************************************/
void RegBitSet(int val,int reg,int msb,int lsb);
unsigned int RegBitRead(int reg,int msb,int lsb);
void init_vga(int format);

struct am7x_de_isr_data{
	/**Notice : am7x display register is set after a frame is finished
	*     so the actual work set may diff with the register value
	*/
	unsigned int current_image_yaddr;
	unsigned int current_image_caddr;
	unsigned int current_osd0_addr;
	unsigned int current_osd1_addr;


	/** for de isr	***/
	struct semaphore de_sem;
	int de_lamp;
	int de_isr_init_flag;


};

/**lcd controller register set**/
struct am7x_lcd_controller_data{
	unsigned int lcd_ctl;
	unsigned int lcd_rgbctl;
	unsigned int lcd_size;
	unsigned int lcd_lsp; 	
	unsigned int lcd_htiming;
	unsigned int lcd_vtiming_odd;
	unsigned int lcd_color;
	unsigned int lcd_dac_ctl;
	unsigned int lcd_vtiming_even;
	unsigned int lcd_vsync_edge;
			
};

struct edid_config{
	unsigned int vga_valid;
	unsigned int hdmi_valid;
	int vga_format;
	int hdmi_format;	
};

/**
*@brief	user lcd device configuration file
*/
struct lcm_config{
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


struct color_config{
	unsigned int gamma_mode;
	unsigned long gamma[256];
	signed int hue;
	signed int saturation;
	signed int brightness;
	signed int contrast;
}__attribute__((packed));


#endif
