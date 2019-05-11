 /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
@file: am7xfb.h

@abstract: actions-mircro framebuffer head file.

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
#ifndef AM7XFB_H
#define AM7XFB_H

#define AM7XFB_DEVICENAME	"am7x-fb"
#define AM7X_FBID			"am7x1211"


#define MAX_FB_SUPPORTED	3
//#define ENABLE_OSDFB


/** device description  ***/
#define  lcd_des  "am7x_lcd output";
#define  hdmi_des  "am7x_hdmi output";
#define  ypbpr_des  "am7x_ypbpr output";
#define  cvbs_des  "am7x_cvbs output";
#define  bttv_des  "am7x_bttv output";
#define  default_des  "am7x_default output";


/*****/
enum display_device_type{
	LCD = 0,
	HDMI,
	YPbPr,
	CVBS,
	BTTV,
	NULL_DEVICE
};

enum am7xfb_state{
	INACTIVE_STATE = 0,
	ACTIVE_STATE
};

/** am7xfb support input pixel color space **/
#define RGB_565		1
#define RGB_888		2
#define RGB_666		3	
#define YUV_420		4
#define YUV_422		5
#define YUV_444		7
#define ARGB_888	8


/** displaymode  for de **/
#define RGB			1
#define RGB_I		2
#define BT_P		3
#define BT_I		4
#define CPU			5



/**am7xfb output interface **/
#define LCD_INTERFACE	1
#define BT_INTERFACE	2

/** **/
#undef	TRUE
#define TRUE		1
#undef	FALSE		
#define	FALSE		0



/** am7xfb  ioctl cmd  **/
#define	AM7XFBIO_SETADDR	0xff01
#define AM7XFBIO_GETADDR	0xff02
#define AM7XFBIO_SETINPUT	0xff03
#define AM7XFBIO_GETINPUT	0xff04
#define AM7XFBIO_SETOUPUT	0xff05
#define AM7XFBIO_GETOUPUT	0xff06

#define AM7XFBIO_ENABLE		0xff07
//#define AM7XFBIO_GETDEVICE	0xff08
 
#define AM7XFBIO_SETCONTRAST	0xff09
#define AM7XFBIO_GETCONTRAST	0xff10
#define AM7XFBIO_SETBRIGHTNESS	0xff0a
#define AM7XFBIO_GETBRIGHTNESS	0xff0b
#define AM7XFBIO_SETTRANSPRENT	0xff0c
#define AM7XFBIO_GETTRANSPRENT	0xff0d

#define AM7XFBIO_SETGAMMA		0xff0e

#define AM7XFBIO_SETINPUTCOLORSPACE	0xff0f
#define AM7XFBIO_GETINPUTCOLORSPACE	0xff10

#define AM7XFBIO_GETDEVICEINFO	0xff11
//#define AM7XFBIO_SETDEVICEINFO	0xff12

#define AM7XFBIO_SET2_HDMI		0xff13
#define AM7XFBIO_CTL_HDMI		0xff14

#define AM7XFBIO_SET2_LCD		0xff15
#define AM7XFBIO_CTL_LCD		0xff16

#define AM7XFBIO_SET2_YPBPR		0xff17
#define AM7XFBIO_CTL_YPBPR		0xff18

#define AM7XFBIO_SET2_CVBS		0xff19
#define AM7XFBIO_CTL_CVBS		0xff1a

#define AM7XFBIO_SET2_BTTV		0xff1b
#define AM7XFBIO_CTL_BTTV		0xff1c

struct am7xfb_outimage{
	int image_xres;  //framebuffer size
	int image_yres;	
	int image_xres_virtual;  // visual size
	int image_yres_virtual;
	int image_x_offset;		//visual offset
	int image_y_offset;
	int image_colorspace;  //determinate by display device 

};

struct am7xfb_inimage{
	int default_color_mode_enable;
	int background_default_color;
	int image_xres;
	int image_yres;
	int image_colorspace;
	unsigned long framebuffer_addr;
};

/** main framebuffer parameter **/
struct	am7xfb_par{  //am7x de capibility
	int state;

	struct am7xfb_outimage out;
	struct am7xfb_inimage	in; 

	int display_mode;  // rgb_i  rgb  bt_p bt_i cpu
	
	struct am7x_display_device *device;	
	
	void* priv_data;
};

struct am7x_display_device{
	int device_type;
	char* description;
	int x_res;
	int y_res;
	int colorspace;
	void* priv_data;  // heap data donot need  free	
};

enum lcd_type{
	LCD_TTL = 0,
	LCD_LVDS,
	LCD_TCON,
	LCD_CPU,
	
};

enum interface_type{
	LCD_16P = 0,
	LCD_18P,
	LCD_24P,
	LCD_16S,
	LCD_18S,
	LCD_24S
};


struct am7x_lcd_data
{
	unsigned char  lcm_type;   //tcon lvds cpu just for decription
	unsigned char  lcd_bt;
	unsigned char  lvds_en;	 
	unsigned char  tcon_en;
	unsigned char  cpu_en;
	unsigned char  d_clk;	
	unsigned short device_width;
	unsigned short device_height;
	unsigned short image_width;
	unsigned short image_height;

	unsigned short hbp;
	unsigned short hfp;
	unsigned short hswp;
	unsigned short vbp;
	unsigned short vfp;
	unsigned short vswp;
	unsigned int rgb_ctl;
	unsigned int dac_ctl;
	
	unsigned char lvds_format;	
	unsigned char lvds_channel;
	unsigned char lvds_swap;
	unsigned char lvds_map;
	unsigned char lvds_mirror;

	unsigned char lcm_rgbformat;

	unsigned int lcm_reserve1;
	unsigned int lcm_reserve2;
	unsigned int lcm_reserve3;
	unsigned int lcm_reserve4;
	unsigned int lcm_reserve5;

}__attribute__((packed));


enum hdmi_timing{
	/// SMPTE 274M
	FMT_2200x1125_1920x1080_60I,
	FMT_2640x1125_1920x1080_50I,

	FMT_2200x1125_1920x1080_30P,
	FMT_2640x1125_1920x1080_25P,
	FMT_2750x1125_1920x1080_24P,
	/// SMPTE 296M
	FMT_1650x750_1280x720_60P,
	FMT_3300x750_1280x720_30P,
	FMT_1980x750_1280x720_50P,
	FMT_3960x750_1280x720_25P,
	FMT_4125x750_1280x720_24P,

	FMT_2304x1250_1920x1152_50I,
	/// ITU-R-656
	FMT_858x525_720x480_60I,
	FMT_864x625_720x576_50I,
	/// SMPTE 293M
	FMT_858x525_720x480_60P,
	FMT_864x625_720x576_50P,

	FMT_1250x810_1024x768_60P,			
};

struct am7x_hdmi_data
{
	int timing_format;  
	/*
		1080i@60hz  1080p@30hz 720p@60hz......  
	*/
	int clk; //Mhz
	int lcd_ctl;
	int lcd_rgbctl;
	int lsp;
	int lcd_width;
	int lcd_height;
	int lcd_htiming;
	int lcd_vtiming_odd;
	int lcd_vtiming_even;
	int lcd_default_color;	
	int lcd_vsync_edge;	

	int need_even;
	
	void* priv_data;  //for hdmi controller
};

enum cvbs_format{
	PAL = 0,
	NTSC	
};

struct am7x_cvbs_data
{
	int format; /** pal or ntsc */

	int clk;
	int btve_ctl;
	int btve_size;
	int btve_hs;
	int btve_hde;
	int btve_vsodp;
	int btve_vseven;
	int btve_ft;
	int btve_lsp;

	int btive_ctl;
	int btive_outctl;

	void* priv_data;
};

enum ypbpr_timing{
	YPBPR_1080P = 0,
	YPBPR_720P,
	YPBPR_576P,
	YPBPR_576I,
	YPBPR_480P,
	YPBPR_480I
};

struct am7x_ypbpr_data
{
	int timing;

	int need_even;
};

struct am7x_bttv_data{
	

};

/** osd framebuffer parmeter **/
struct am7xosd_par
{
	
		
};

#if 0
struct am7xfb_config_file{
	char default_device;  //lcd ypbpr cvbs hdmi
	union{
		int timing_format; //hdmi ypbpr cvbs
		struct lcm_config lcm;
	};
}__attribute__((packed));
#endif


int de_set_input(struct am7xfb_inimage* in);
int de_set_output(struct am7xfb_outimage* out);
int de_enable(void);
int de_disable(void);
int de_set_display_mode(int mode);
int de_set_framebuffer_addr(unsigned long addr);


int hardware_init_display_device(struct am7x_display_device *device);
int hardware_close_display_device(struct am7x_display_device *device);

int init_display_device(struct am7x_display_device *device);
void destroy_display_device(struct am7x_display_device *device);

struct am7x_display_device* init_device(struct am7x_display_device *device,int device_type);
int config_device(int type,int  format);

int cacl_display_mode(struct am7x_display_device* device);


void RegBitSet(int val,int reg,int msb,int lsb);


#endif
