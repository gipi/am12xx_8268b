/**
*@file lcm_op.h
*@brief this head file describes the lcm device operations for Actions-micro IC
*
*@author 
*@date 2010-11-17
*@version 0.1
*/



/**
*@addtogroup lcmlib
*@{
*/
#ifndef LCM_OP_H
#define LCM_OP_H

#include "am_types.h"
/**
*     usage:  DE_INFO  tmp ;
*             fd = open("/dev/lcm",MODE);
*             ioctl(fd,GET_DE_CONFIG,&tmp);
*                
*             for example :set de addess
*             tmp.DisplaySource   = DE_IMAGE
*             tmp.input_rgb_bus_addr   = physic_addr;
*                
*             ioctl(fd,UPDATE_DE_CONFIG,&tmp);
*/


/**
*@name command for lcm operation
*@{
*/

/*lcm ioctl number*/
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
#define SHARPNESS_ADJUST		0x0b
#define SET_CURSOR				0x0c
#define GET_MONSPECS			0x0d
#define GET_MODEDB				0x0e

/******display device control******/
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

/**
*@}
*/


typedef struct __lcm_dpb{
	unsigned long dbp_vir_addr;
	unsigned long img_bus_addr;
	unsigned long uv_img_bus_addr;//img_bus_addr+width*height
	long long pts;//system time
}LCM_DPB_T;



/**
*@brief	Display device format definations
*/
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
*@brief	HDMI timing format definations
*/
typedef enum DS_VIDEO_TIMING_FMT{
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
	FMT_1280x800_60P,
	FMT_1920x1080_60P,
	FMT_800x600_60P,
	FMT_1920x1080_50P,
	FMT_854x480_50P,

	FMT_2080x741_1920x720_59P,
	
	FMT_USER_DEFINED,  //value 18 ,for DVI user defined format
} DS_VIDEO_TIMING_FMT;

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
*@brief	de data source definations
*/
enum source
{
	DE_IMAGE,
	DE_DEFAULTCOLOR
};

/**
*@name input pixel format  definition
*@{
*/
#define PIX_FMT_UNKNOWN								0
#define PIX_FMT_YCBCR_4_0_0							1		//I
#define PIX_FMT_YCBCR_4_2_0_PLANAR					2		//I
#define PIX_FMT_YCBCR_4_2_0_SEMIPLANAR              3		//IO
#define PIX_FMT_YCBCR_4_2_0_TILED					4		//I
#define PIX_FMT_YCBCR_4_2_2_INTERLEAVED             5		//IO
#define PIX_FMT_YCBCR_4_2_2_SEMIPLANAR				6		//I
#define PIX_FMT_YCBCR_4_4_0_SEMIPLANAR				7		//I
#define PIX_FMT_YCBCR_4_4_4_SEMIPLANAR				8		//I
#define PIX_FMT_RGB32                               9		// O
#define PIX_FMT_BGR32                               10		// O
#define PIX_FMT_RGB16_5_5_5                         11		// O
#define PIX_FMT_BGR16_5_5_5                         12		// O
#define PIX_FMT_RGB16_5_6_5                         13		// O
#define PIX_FMT_BGR16_5_6_5                         14		// O
#define PIX_FMT_RGB16_CUSTOM                        15		// O
#define PIX_FMT_RGB32_CUSTOM                        16	
/**
*@}
*/


/**
 *@brief device color space 
*/
enum cscscolorspace
{
	CSC_OFF,
	SDTV_SRGB,
	SDTV_PCRGB,
	HDTV_SRGB,
	HDTV_PCRGB,
	CSC_NOCHANGE
};

/**
*@brief de input mode 
*/
enum deimagemode
{
	DE_YUV420,
	DE_YUV422,
	DE_RGB32,
	DE_RGB565,
	OSDBIT4MODE,
	OSDBIT2MODE,
	OSDBIT1MODE,
	OSDBIT16MODE,
	OSDBIT8MODE,
	OSDBIT32MODE	
};//InImageMode;osdMode

/**
*@brief de dither mode 
*/
enum dither
{
	DITHER_OFF,
	Dither_888To565,
	Dither_888To666
};//DitherMode

/**
*@brief de output mode 
*/
enum outmode
{
	RGBMODE,
	BTMODE_I,
	BTMODE_P,
	CPUMODE
};//DisplayMode

/**
*@brief	de ioctl main data structure
*/
typedef struct _display_engine_info_
{
	INT32U updateFlag;
	INT32U input_pix_fmt;			//input pixel format	
	INT32U display_mode;			//pan_and_scan, letter_box, full_screen, actual_size
	INT32U input_width;			//input width //%16byte=0;<=2048
	INT32U input_height;			//input height;<=2048	
	INT32U screen_type; //0:LCD;1:HDMI;2:vga;3:cvbs/bt;4:ypbpr
	INT32U screen_output_format; //HDMI/VGA/CVBS format
	INT32U screen_width; //panel width;<=2048
	INT32U screen_height;//panel height;<=2048		
	//display aspect ratio
	INT32U dar_width_image;				//display width ratio,<=32
	INT32U dar_height_image;				//display height ratio, <=32
	INT32U par_width_image;				//pixel width ratio,<=32
	INT32U par_height_image;				//pixel height ratio,<=32
	INT32U dar_width_screen;		//display width ratio,high priority ,<=32
	INT32U dar_height_screen;				//display height ratio,high priority ,<=32
	INT32U par_width_screen;				//screen pixel width ratio,<=32
	INT32U par_height_screen;				//screen pixel height ratio,<=32
			 		
	INT32U colorspace;				//0:bt601,16-235; 1:bt601,0-255;2:bt709,16-235; 3:bt709,0-255;
	INT32U DisplaySource;			//DE_IMAGE,DE_DEFAULTCOLOR
	INT32U DefaultColor;				//RGB or YUV color mod
	INT32U Mode;					//RGBMODE,BTMODE_I,BTMODE_P,cpu
	INT32U DitherMode;				//DITHER_OFF,Dither_888To565,Dither_888To666
	INT32U GammaMode;			//GAMMA_OFF,GAMMA_ON
	INT32U* Gamma;
	



	INT8U  *input_luma;				//input luminance buffer pointer
	INT32U input_luma_bus_addr;		//input luminance buffer bus address//%16byte=0;
	INT8U  *input_chroma;			//input chrominance buffer pointer
	INT32U input_chroma_bus_addr;	//input chrominance buffer bus address//%16byte=0;
	INT8U  *input_rgb;				//input RGB buffer pointer
	INT32U input_rgb_bus_addr;		//input RGB buffer bus address//%16byte=0;
	
	signed int contrast;					//Adjust the contrast of the output picture [-64,64]
	signed int brightness;				//Adjust the brightness level of the output picture [-128,127]
	signed int saturation;					//Adjust the color saturation of the output picture [-64,128]
	signed int hue;						//Adjust the color hue of the output picture [-60,60]
	signed int sharpness;				//Adjust the sharpness of the output picture [-31,31]
////////////////////////////////////////////////////////////////////////////////////////////////
	INT32S  a;
	INT32S  b;
	INT32S  c;
	INT32S  d;
	INT32S  e;
	INT32S  ysub16;
	INT32S  csc;							//enum CSCmode CSC_OFF,SDTV_SRGB,SDTV_PCRGB,HDTV_SRGB,HDTV_PCRGB
////////////////////////////////////////////////////////
	INT32U* Pallet;				//osd 调色版数据指针
	INT32U* Pallet256;				//osd 调色版数据指针	
	INT32U TransColor;			//osd需要透明处理的颜色值rgb
	INT32U * pIndex;			//osd显示数据,根据调色版的索引值
	INT32U iIndexLen;			//osd显示数据长度,以字节为单位
	INT32U osd_no;				//0-3
	INT32U osd_sw;				//OSD_OFF,OSD_ON
	INT32U osd_bitmode;		//OSDBIT4MODE,OSDBIT2MODE,OSDBIT1MODE
	INT32U osd_alpha;			//osd透明值(0~8)
	INT32U osd_addr;			//osd显示数据存放地址
	INT32U osd_stride;			//BIT4MODE :%32pixel=0;BIT2MOD:%64pixel=0;BIT1MOD:%128pixel=0;
	INT32U osd_xstart;			
	INT32U osd_ystart;
	INT32U osd_width;
	INT32U osd_height;	
////////////////////////////////////////////////////////	
	INT32U cursor_sw;				
	INT32U cursor_x;	
	INT32U cursor_y;		
	
	INT32U crop_enable;
	INT32U crop_ori_x;				//
	INT32U crop_ori_y;				//
	INT32U crop_width;
	INT32U crop_height;

	INT32U pip_enable;
	INT32U pip_ori_x;
	INT32U pip_ori_y;
	INT32U pip_frame_width;
	INT32U pip_frame_height;

	INT32U zoom;
	INT32U zoom_fit_width;
	INT32U zoom_fit_height;
	INT32U zoom_min;
	INT32U zoom_max;
	INT32U crop_width_min;
	INT32U crop_height_min;
	INT32U move_x_enable;
	INT32U move_y_enable;
	INT32U move_center_x;
	INT32U move_center_y;
	INT32U move_step_x;
	INT32U move_step_y;

	void *irq_func_entry[16];

}DE_INFO;

struct fb_chroma {
	INT32U redx;	/* in fraction of 1024 */
	INT32U greenx;
	INT32U bluex;
	INT32U whitex;
	INT32U redy;
	INT32U greeny;
	INT32U bluey;
	INT32U whitey;
};

struct fb_videomode {
	const char *name;	/* optional */
	INT32U refresh;		/* optional */
	INT32U xres;
	INT32U yres;
	INT32U pixclock;
	INT32U left_margin;
	INT32U right_margin;
	INT32U upper_margin;
	INT32U lower_margin;
	INT32U hsync_len;
	INT32U vsync_len;
	INT32U sync;
	INT32U vmode;
	INT32U flag;
};

struct fb_monspecs {
	struct fb_chroma chroma;
	struct fb_videomode *modedb;	/* mode database */
	INT8U  manufacturer[4];		/* Manufacturer */
	INT8U  monitor[14];		/* Monitor String */
	INT8U  serial_no[14];		/* Serial Number */
	INT8U  ascii[14];		/* ? */
	INT32U modedb_len;		/* mode database length */
	INT32U model;			/* Monitor Model */
	INT32U serial;			/* Serial Number - Integer */
	INT32U year;			/* Year manufactured */
	INT32U week;			/* Week Manufactured */
	INT32U hfmin;			/* hfreq lower limit (Hz) */
	INT32U hfmax;			/* hfreq upper limit (Hz) */
	INT32U dclkmin;			/* pixelclock lower limit (Hz) */
	INT32U dclkmax;			/* pixelclock upper limit (Hz) */
	INT16U input;			/* display type - see FB_DISP_* */
	INT16U dpms;			/* DPMS support - see FB_DPMS_ */
	INT16U signal;			/* Signal Type - see FB_SIGNAL_* */
	INT16U vfmin;			/* vfreq lower limit (Hz) */
	INT16U vfmax;			/* vfreq upper limit (Hz) */
	INT16U gamma;			/* Gamma - in fractions of 100 */
	INT16U gtf	: 1;		/* supports GTF */
	INT16U misc;			/* Misc flags - see FB_MISC_* */
	INT8U  version;			/* EDID version... */
	INT8U  revision;			/* ...and revision */
	INT8U  max_x;			/* Maximum horizontal size (cm) */
	INT8U  max_y;			/* Maximum vertical size (cm) */
	INT8U  extensionflag;	/* Extension flag*/
	INT8U  vsdbflag;			/* VSDB flag*/
};

#endif
/**
 *@}
 */
