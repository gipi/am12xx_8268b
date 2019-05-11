
#ifndef _DEAPI_H_
#define _DEAPI_H_

#include "DEdrive.h" 

#define SHARPNESS_ERROR				0
/*注意dedrive.h中的
enum swcsc
{
	OFF,
	ON
};//CSC;
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
#define PIX_FMT_RGB32_CUSTOM                        16		// O

/************Display Mode**************/
typedef enum eDisplayMode
{
	E_DISPLAYMODE_LETTER_BOX,
	E_DISPLAYMODE_PAN_AND_SCAN,					
	E_DISPLAYMODE_FULL_SCREEN, 		
	E_DISPLAYMODE_ACTUAL_SIZE,
	E_DISPLAYMODE_DAR_SIZE,
	//E_DISPLAYMODE_BUTT    	
}E_DISPLAYMODE;

/**
*@name display devide type
*@{
*/
#define DE_OUT_DEV_ID_LCD		0 
#define DE_OUT_DEV_ID_HDMI		1 
#define DE_OUT_DEV_ID_VGA		2 
#define DE_OUT_DEV_ID_CVBS		3 
#define DE_OUT_DEV_ID_YPBPR		4
#define MAX_DEV_NUM 			5

/******display engine control******/
/**********fixme: *****************/
#define DE_INIT 			0x00
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
	INT32U Mode;					//		RGBMODE,RGBMODE_I,BTMODE_I,BTMODE_P,cpu
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

#if 0
	INT32S  de_init(DE_INFO *de_ifo);
	INT32S  de_release(DE_INFO *de_ifo);
	INT32S  de_cfg_update(DE_INFO *de_ifo);
	INT32S  de_color_adjust(DE_INFO *de_ifo);
	INT32S  de_gamma_adjust(DE_INFO *de_ifo);
	INT32S  de_set_display_mode(DE_INFO *de_ifo);
	INT32S  de_pic_display_mode(DE_INFO *de_ifo);
	INT32S  de_change_frame_addr(DE_INFO *de_ifo);
	INT32S  de_set_osd(DE_INFO *de_ifo );
void de_write_osdIndex(DE_INFO *de_ifo);
void de_set_pallet(DE_INFO *de_ifo);
void de_set_pallet256(DE_INFO *de_ifo);


	INT32S  de_zoom_in(DE_INFO *de_ifo);
	INT32S  de_move_up(DE_INFO *de_ifo,	INT32U  iUpStep);
	INT32S  de_move_down(DE_INFO *de_ifo,	INT32U  iDownStep);
	INT32S  de_move_left(DE_INFO *de_ifo,	INT32U  iLeftStep);
	INT32S  de_move_right(DE_INFO *de_ifo,	INT32U  iRightStep);
	INT32S  de_zoom_in_step(DE_INFO *de_ifo);
#if 0
	INT32S  de_zoom_out(DE_INFO *de_ifo);
	INT32S  de_set_display_aspect_ratio(DE_INFO *de_ifo, INT32U dar_width, INT32U dar_height);
	INT32S  de_set_irq(DE_INFO *de_ifo, INT32U irq_num, void *irq_func_entry);
#endif
#endif
  INT32S de_set(INT32S cmd,DE_INFO *de_ifo);

#endif//_DEAPI_H_

