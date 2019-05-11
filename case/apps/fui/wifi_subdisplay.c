#ifdef MODULE_CONFIG_WIFI_SUBDISPLAY
#include <pthread.h>
#include <pwd.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "wifi_subdisplay.h"
#include "swf_types.h"
#include "lcm_op.h"
#include "sys_cfg.h"
#include "system_info.h"
#include "display.h"
#include "ezcast_public.h"
#include "fui_common.h"


#if 1
#define wifidisp_info(fmt,arg...) 	printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define wifidisp_err(fmt,arg...) 	printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#else
#define wifidisp_info(fmt,arg...)	do{}while(0);
#define wifidisp_err(fmt,arg...)	do{}while(0);
#endif

int wifidisplay_onoff=0;

extern unsigned long swf_heap_logic_start;
extern unsigned long swf_heap_physic_start;
/*
typedef struct _display_engine_info_tmp
{
	unsigned int updateFlag;
	unsigned int input_pix_fmt;			//input pixel format	
	unsigned int display_mode;			//pan_and_scan, letter_box, full_screen, actual_size
	unsigned int input_width;			//input width //%16byte=0;<=2048
	unsigned int input_height;			//input height;<=2048	
	unsigned int screen_type; //0:LCD;1:HDMI	
	unsigned int screen_width; //panel width;<=2048
	unsigned int screen_height;//panel height;<=2048		
	//display aspect ratio
	unsigned int dar_width_image;				//display width ratio,<=32
	unsigned int dar_height_image;				//display height ratio, <=32
	unsigned int par_width_image;				//pixel width ratio,<=32
	unsigned int par_height_image;				//pixel height ratio,<=32
	unsigned int dar_width_screen;		//display width ratio,high priority ,<=32
	unsigned int dar_height_screen;				//display height ratio,high priority ,<=32
	unsigned int par_width_screen;				//screen pixel width ratio,<=32
	unsigned int par_height_screen;				//screen pixel height ratio,<=32
			 		
	unsigned int colorspace;				//0:bt601,16-235; 1:bt601,0-255;2:bt709,16-235; 3:bt709,0-255;
	unsigned int DisplaySource;			//DE_IMAGE,DE_DEFAULTCOLOR
	unsigned int DefaultColor;				//RGB or YUV color mod
	unsigned int Mode;					//		RGBMODE,RGBMODE_I,BTMODE_I,BTMODE_P,cpu
	unsigned int DitherMode;				//DITHER_OFF,Dither_888To565,Dither_888To666
	unsigned int GammaMode;			//GAMMA_OFF,GAMMA_ON
	unsigned int* Gamma;
	



	unsigned char  *input_luma;				//input luminance buffer pointer
	unsigned int input_luma_bus_addr;		//input luminance buffer bus address//%16byte=0;
	unsigned char  *input_chroma;			//input chrominance buffer pointer
	unsigned int input_chroma_bus_addr;	//input chrominance buffer bus address//%16byte=0;
	unsigned char  *input_rgb;				//input RGB buffer pointer
	unsigned int input_rgb_bus_addr;		//input RGB buffer bus address//%16byte=0;
	
	signed int contrast;					//Adjust the contrast of the output picture [-64,64]
	signed int brightness;				//Adjust the brightness level of the output picture [-128,127]
	signed int saturation;					//Adjust the color saturation of the output picture [-64,128]
	signed int hue;						//Adjust the color hue of the output picture [-60,60]
	signed int sharpness;				//Adjust the sharpness of the output picture [-31,31]
////////////////////////////////////////////////////////////////////////////////////////////////
	int  a;
	int  b;
	int  c;
	int  d;
	int  e;
	int  ysub16;
	int  csc;							//enum CSCmode CSC_OFF,SDTV_SRGB,SDTV_PCRGB,HDTV_SRGB,HDTV_PCRGB
////////////////////////////////////////////////////////
	unsigned int* Pallet;				//osd 调色版数据指针
	unsigned int* Pallet256;				//osd 调色版数据指针	
	unsigned int TransColor;			//osd需要透明处理的颜色值rgb
	unsigned int * pIndex;			//osd显示数据,根据调色版的索引值
	unsigned int iIndexLen;			//osd显示数据长度,以字节为单位
	unsigned int osd_no;				//0-3
	unsigned int osd_sw;				//OSD_OFF,OSD_ON
	unsigned int osd_bitmode;		//OSDBIT4MODE,OSDBIT2MODE,OSDBIT1MODE
	unsigned int osd_alpha;			//osd透明值(0~8)
	unsigned int osd_addr;			//osd显示数据存放地址
	unsigned int osd_stride;			//BIT4MODE :%32pixel=0;BIT2MOD:%64pixel=0;BIT1MOD:%128pixel=0;
	unsigned int osd_xstart;			
	unsigned int osd_ystart;
	unsigned int osd_width;
	unsigned int osd_height;	
	
	
	unsigned int crop_enable;
	unsigned int crop_ori_x;				//
	unsigned int crop_ori_y;				//
	unsigned int crop_width;
	unsigned int crop_height;

	unsigned int pip_enable;
	unsigned int pip_ori_x;
	unsigned int pip_ori_y;
	unsigned int pip_frame_width;
	unsigned int pip_frame_height;

	unsigned int zoom;
	unsigned int zoom_fit_width;
	unsigned int zoom_fit_height;
	unsigned int zoom_min;
	unsigned int zoom_max;
	unsigned int crop_width_min;
	unsigned int crop_height_min;
	unsigned int move_x_enable;
	unsigned int move_y_enable;
	unsigned int move_center_x;
	unsigned int move_center_y;
	unsigned int move_step_x;
	unsigned int move_step_y;
	void *irq_func_entry[16];
}DE_INFO_tmp;

DE_INFO_tmp DeInfo;
*/
int getNetDisplayStatus()
{
	return wifidisplay_onoff;
}

int wifi_subdisplay_start()
{
	int ret = -1;
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
	{
		/** first set swf to sleep, and buffer will be released */
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}
	printf("Hello Wifi Subdisplay!\n");
	ret = ezFuiWifiStart(swf_heap_logic_start, swf_heap_physic_start, &fui_get_bus_address, NULL, NULL);
	if (ret != 0)
	{
		wifidisp_err("wifi subdisplay init error!");
		if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
		{
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}
	}
	wifidisplay_onoff = 1;
	return ret;
}
extern void *deinst;
static DE_config ds_conf;

static int set_de_defaultcolor()
{
	de_get_config(deinst,&ds_conf,DE_CFG_ALL);
	ds_conf.input.width = screen_output_data.screen_output_width;
	ds_conf.input.height = screen_output_data.screen_output_height;
	ds_conf.input.enable=0;
	printf("ds_conf.input.width: %d, ds_conf.input.height: %d\n", ds_conf.input.width, ds_conf.input.height);
	de_set_Config(deinst,&ds_conf,DE_CFG_ALL);
	
	return 0;
}

int wifi_subdisplay_end()
{
	int ret = -1;
	printf("Goodbye Wifi Subdisplay\n");
	ret = ezFuiWifiStop();
	set_de_defaultcolor();
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}
	wifidisplay_onoff = 0;
	return ret;
}
int startDirectMirroring()
{
	EZCASTLOG("wifidisplay_onoff: %d\n", wifidisplay_onoff);
	if(wifidisplay_onoff != 0)
	{
		ezFuiWifiStop();
		//set_de_defaultcolor();
	}
	
	if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE)
	{
		/** first set swf to sleep, and buffer will be released */
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	}

	return 0;
}
int stopDirectMirroring()
{
	EZCASTLOG("wifidisplay_onoff: %d\n", wifidisplay_onoff);
	set_de_defaultcolor();
	if((SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE) == 0)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	}

	return 0;
}
#else
int getNetDisplayStatus()
{
	return 0;
}
int wifi_subdisplay_start()
{
	return 0;
}

int wifi_subdisplay_end()
{
	return 0;
}
int startDirectMirroring()
{
	return 0;
}
int stopDirectMirroring()
{
	return 0;
}
#endif	/** MODULE_CONFIG_WIFI_SUBDISPLAY */
