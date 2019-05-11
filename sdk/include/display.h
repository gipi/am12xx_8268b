/**
*@file display.h
*@brief display API 
this head file describes the display library api for Actions-micro IC.
the API is useful for midware soft.If you is familar with frame buffer API.
The fb is the better for you. The display API can control dispaly of the system.
It include the display input ,display mode,display output.
*
*@author 
*@date 2010-11-17
*@version 0.1
*/

/**
*@addtogroup display_lib_s
*@{
*/
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

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
/**
*@}
*/

/**
*@name CVBS timing definition
*@{
*/
#define DE_OUT_MODE_NTSC			1 
#define DE_OUT_MODE_PAL				2 
/**
*@}
*/

/**
*@name HDMI timing definition
*@{
*/
#define DE_OUT_MODE_HDMI_480P						1
#define DE_OUT_MODE_HDMI_480I						2
#define DE_OUT_MODE_HDMI_576P						3
#define DE_OUT_MODE_HDMI_576I						4
#define DE_OUT_MODE_HDMI_720P_30HZ					5
#define DE_OUT_MODE_HDMI_720P_60HZ					6
#define DE_OUT_MODE_HDMI_1080I_30HZ					7
/**
*@}
*/

/**
*@name VGA timing definition
*@{
*/
#define DE_OUT_MODE_VGA_640x480							1
#define DE_OUT_MODE_SVGA_800x600						2
#define DE_OUT_MODE_XGA_1024x768						3
/**
*@}
*/

/**
*@name display input pixel format
*@{
*/
#define DE_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR         3	
#define PIX_FMT_YCBCR_4_2_0_SEMIPLANAR            3
#define DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED        5	
#define	PIX_FMT_YCBCR_4_2_2_INTERLEAVED			  5
#define DE_PIX_FMT_RGB32                          9	
#define PIX_FMT_RGB32                             9	
#define DE_PIX_FMT_RGB16_5_6_5                    13
#define PIX_FMT_RGB16_5_6_5                    	  13

/**
*@name osd pixel format
*@{
*/
#define DE_PIX_FMT_OSDBIT1MODE					  0	
#define DE_PIX_FMT_OSDBIT2MODE					  1	
#define DE_PIX_FMT_OSDBIT4MODE					  2	
#define DE_PIX_FMT_OSDBIT8MODE					  3	
#define DE_PIX_FMT_OSDBIT16MODE					  4
#define DE_PIX_FMT_OSDBIT32MODE					  5

		

/**
*@name index format
*@{
*/
#define DE_IDX_FMT_RGB16 0
#define DE_IDX_FMT_RGB32 1



	
/**
*@}
*/

/**
*@name display input colorspace 
*@{
*/
#define		DE_CSC_OFF				0
#define		DE_SDTV_SRGB			1
#define		DE_SDTV_PCRGB			2
#define		DE_HDTV_SRGB			3
#define		DE_HDTV_PCRGB			4
#define		DE_CSC_NOCHANGE			5
/**
*@}
*/

/**
*@name display mode 
*@{
*/
#define	DE_DISPLAYMODE_LETTER_BOX		0
#define	DE_DISPLAYMODE_PAN_AND_SCAN		1		
#define	DE_DISPLAYMODE_FULL_SCREEN		2		
#define	DE_DISPLAYMODE_ACTUAL_SIZE		3
#define	DE_DISPLAYMODE_DAR_SIZE			4 	
/**
*@}
*/

/**
*@name  config flag,you can config part of display with the config flag
*@{
*/
#define DE_CFG_IN											0x00000001
#define DE_CFG_OUT											0x00000002
#define DE_CFG_DEVIFO										0x00000004
#define DE_CFG_OSD_IN1										0x00000008
#define DE_CFG_OSD_OUT1										0x00000010
#define DE_CFG_OSD1											(DE_CFG_OSD_IN1|DE_CFG_OSD_OUT1)
#define DE_CFG_OSD_IDX										0x00000020
#define DE_CFG_GAMMA										0x00000040
#define DE_CFG_ALL											(DE_CFG_IN|DE_CFG_OUT|DE_CFG_DEVIFO|DE_CFG_OSD_IDX|DE_CFG_GAMMA)
#define DE_CFG_INPUT_ADD_UNBLOCK							0x00000080
#define DE_CFG_OSDINPUT1_ADD_UNBLOCK						0x00000100
#define DE_CFG_OSDINPUT2_ADD_UNBLOCK						0x00000200
#define DE_CFG_OSD_IN2										0x00000400
#define DE_CFG_OSD_OUT2										0x00000800
#define DE_CFG_OSD2											(DE_CFG_OSD_IN2|DE_CFG_OSD_OUT2)
#define DE_CFG_CURSOR										0x00001000
/**
*@}
*/

/**
*@name  function return value
*@{
*/
typedef enum _de_result_
{
	DE_OK			=0,
	DE_MEMFAIL		=-1,
	DE_ERROR		=-16,
}DE_result;
/**
*@}
*/

/**
*@name display device info 
*@{
*/
typedef struct _de_dev_info_
{
	int enable;				//dev enable
	int width; 				//dev width
	int height;				//dev height
	int par_width;		//a pixel width ratio
	int par_height;		//a pixel height ratio
	int mode;					//DE_OUT_DEV_ID_LCD,DE_OUT_DEV_ID_HDMI,DE_OUT_DEV_ID_VGA,DE_OUT_DEV_ID_CVBS
	int *mode_list;
	int back_light;		//for LCD device
}DE_dev_info;
/**
*@}
*/

/**
*@name image layer input info
*@{
*/
typedef struct _de_input_
{
	int enable;									// input enable
	int default_color;					//default color,ARGB
	int width;									//input width
	int height;									//input height
	int pix_fmt;								// pixel format
	int video_range;						//color space
	unsigned long *img;					//input y logic address
	unsigned long *img_uv;			//input uv logic address
	unsigned long bus_addr;			//input y phsics address
	unsigned long bus_addr_uv;	//input uv phsics address
	int crop_x;									//just for DE_DISPLAYMODE_DAR_SIZE;>0
	int crop_y;									//just for DE_DISPLAYMODE_DAR_SIZE;>0
	int crop_width;							//just for DE_DISPLAYMODE_DAR_SIZE
	int crop_height;						//just for DE_DISPLAYMODE_DAR_SIZE
}DE_input;
/**
*@}
*/

/**  gamma max defination */
#define MAX_GAMMA_NUM 			256
/**
*@name image layer display mode info
*@{
*/
typedef struct _de_output_
{
	int pip_x;												//just for DE_DISPLAYMODE_DAR_SIZE;>0
	int pip_y;												//just for DE_DISPLAYMODE_DAR_SIZE;>0
	int pip_width;										//just for DE_DISPLAYMODE_DAR_SIZE
	int pip_height;										//just for DE_DISPLAYMODE_DAR_SIZE
	int dar_width;										//dispalay width ratio						
	int dar_height;										//dispalay height ratio							
	int display_mode;									//letter box, pan and scan, full screen, actrul size, customize
	int output_mode;									//NTSC,PAL, VGA,SVGA, XGA, HDMI_480p, HDMI_720p_30Hz, HDMI_720p_60Hz...
	int	brightness;										//Adjust the brightness level of the output picture [-128,127]
	int contrast;											//Adjust the contrast of the output picture [-64,64]								
	int saturation;										//Adjust the color saturation of the output picture [-64,128]
	int hue;													//Adjust the color hue of the output picture [-60,60]	
	int sharpness;										//Adjust the sharpness of the output picture [-31,31]	
	int gamma_enable;									//gamma enable
	unsigned int gamma[MAX_GAMMA_NUM];//gamma table
}DE_output;
/**
*@}
*/

/**  index max defination */
#define MAX_INDEX_NUM 			256
/**
*@name OSD layer input info 
*@{
*/
typedef struct _de_osd_input_
{
	int enable;												//osd input enable
	int stride;												//osd input width
	int pix_fmt;											//osd pixel format
	unsigned long *img;										//osd logic address
	unsigned long bus_addr;									//osd phsics address
	unsigned int tparent_color;								//RGB565
	int idx_fmt;											//index format
	unsigned int index[MAX_INDEX_NUM];						//index table
}DE_OSD_input;
/**
*@}
*/

/**
*@name OSD layer display mode info
*@{
*/
typedef struct _de_osd_output_
{
	int alpha;		//osd alpha
	int pip_x;		//osd x coordinate in screen;>0
	int pip_y;		//osd y coordinate in screen;>0
	int width;		//osd width in screen
	int height;		//osd height in screen
}DE_OSD_output;
/**
*@}
*/

/**
*@name cursor info
*@{
*/
typedef struct _cursor_
{
	int enable; //enable
	int x;		// x coordinate 
	int y;		// y coordinate 
}DE_CURSOR;
/**
*@}
*/

/**
*@name display info,include display device ,display input ,display mode.
*@{
*/
typedef struct _de_config_
{
	DE_dev_info		dev_info[MAX_DEV_NUM];		//device info
	DE_input 		input;						//image input info
	DE_output 		output;						//image display mode info
	DE_OSD_input	osd_input1;					//osd1 input info
	DE_OSD_output	osd_output1;				//osd1 display mode info
	DE_OSD_input	osd_input2;					//osd2 input info
	DE_OSD_output	osd_output2;				//osd2 display mode info
	DE_CURSOR   	cursor;						//cursor
	int				de_status; 					//0:idel,1:busy
	int				debug_enable;				//debug enable
}DE_config;
/**
*@}
*/

/*
*
* DE_result  de_init(void* *inst)
*
* @brief
*      initial a de instance
*
* @param  
*      the pointer to the instance
*
* @return
*      returns DE_result
*
* @example
*      please see test code
*/
DE_result  de_init(void* *inst);

/*
*
* DE_result  de_get_config(void* inst,DE_config *conf, int flg)
*
* @brief
*      get the config of the  instance
*
* @param  
*      inst:
*				the instance
*			 conf:
*				the pointer to the config
*  		 flg:
* 			the valid flag
*
* @return
*      returns DE_result
*
* @example
*      please see test code
*/
DE_result  de_get_config(void* inst,DE_config *conf, int flg);

/*
*
* DE_result  de_set_Config(void* inst,DE_config *conf, int flg)
*
* @brief
*      set the config of the  instance
*
* @param  
*      inst:
*				the instance
*			 conf:
*				the pointer to the config
*  		 flg:
* 			the valid flag
*
* @return
*      returns DE_result
*
* @example
*      please see test code
*/
DE_result  de_set_Config(void* inst,DE_config *conf, int flg);

/*
*
* DE_result  de_release(void* inst)
*
* @brief
*      release a de instance
*
* @param  
*      the instance
*
* @return
*      returns DE_result
*
* @example
*      please see test code
*/
DE_result  de_release (void* inst);

/*
*
* void enable_2d_task(int enable)
*
* @brief
*      fix bug for 1211 fifo bug
*
* @param  
*      enable=1;enable 2d_task
*      enable=1;disable 2d_task
* @return
*      
*
* @example
*      please see test code
*/ 
void enable_2d_task(int enable);
 
 /**
 *@}
 */
#endif
