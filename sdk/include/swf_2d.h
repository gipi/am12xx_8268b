#ifndef SWF_2D_H
#define SWF_2D_H
/**
* @addtogroup 2d_lib_s
* @{
*/
#include "swf_types.h"

#ifndef EXPORT_SYMBOL
#ifdef WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL   __attribute__ ((visibility("default")))
#endif
#endif
enum
{
	_2D_BUSY,
	_2D_IDLE
};
typedef struct _FillInfo
{
	unsigned int DefColor;	//DEFAULT_COLOR_CONF (only when input_type = 0)
	SWF_MATRIX  Matrix;			//CONTAIN COEFF_A_B_C_D
													//ScaleX : CONTAIN COEFF_A
													//ScaleY : CONTAIN COEFF_D	
													//RotateSkew0 : CONTAIN COEFF_C
													//RotateSkew1 : CONTAIN COEFF_B
													//TranslateX : input_start_x <- input_start_x + TranslateX
													//TranslateY : input_start_y <- input_start_y + TranslateY
	SWF_CXFORM  Cxform;			//CONTAIN FORMAT_TRANS_COEFF (only when input_type = 1)
													//R' = max(0, min(((R * RedMultTerm) / 256) + RedAddTerm, 255))
													//G' = max(0, min(((G * GreenMultTerm) / 256) + GreenAddTerm, 255))
													//B' = max(0, min(((B * BlueMultTerm) / 256) + BlueAddTerm, 255))
													//A' = max(0, min(((A * AlphaMultTerm) / 256) + AlphaAddTerm, 255))							
	int		input_addr;					//SRC_CENTER_ADDR
	int		input_vir_addr;				//SRC_CENTER_ADDR vir
	int		input_type;				// 0:default color 1:bitmap
	int		input_stride;			//STRIDE_CONF
	int		input_start_x;		//HEAD_POINT_ROTATE_X_COORDI,16.16
	int		input_start_y;		//HEAD_POINT_ROTATE_Y_COORDI,16.16
	int		input_width;			//SRC_BOUNDARY_X_COORDI
	int		input_height;			//SRC_BOUNDARY_Y_COORDI
	int		input_repeat;			//0: no action 1: repeat 2: extend
	int		input_format;			//0: ARGB 3: YUV422 (only when input_type = 1)
	int		input_need_blend;	//0: do not need 1:need
	int		input_need_interpolate;	//0: do not need 1:need
	int		output_addr;			//DST_HEAD_ADDR_CONF
	int		output_vir_addr;			//DST_HEAD_ADDR_CONF vir
	int		output_type;			// 0:rect 1:pair 
	int		output_offset;		//DST_HEAD_ADDR_CONF
	int		output_stride;		//STRIDE_CONF
	int		output_width;			//SCAN_AREA_CONF
	int		output_height;		//SCAN_AREA_CONF
	int		output_format;		//0: ARGB 3: YUV422
	int		pair_addr;				//POINT_PAIR_ADDR
	int		pair_vir_addr;				//POINT_PAIR_ADDR
	int		pair_num;					//POINT_PAIR_NUM
	int		z_buffer;					//2D_ZBUF_HEAD_ADDR_CONF
	int		z_buffer_vir;					//2D_ZBUF_HEAD_ADDR_CONF vir
	int		z_stride;					//2D_ZBUF_STRIDE_CONF
	int		z_depth;					//2D_COLOR_KEY_CONF

	int		aa_mode;					//0:reserved, 1:low-quality, 2:mid-quality, 3:high-quality

	int		blend_mode; //0:normal,1:invert
	int   z_buffer_new;
	int   z_buffer_new_vir;

	//for hardware clip
	SWF_RECT clip;	//for hardware clip
	int start_time; //1:start
} FILLINFO;
typedef struct _SpeedInfo
{	
	int total_time;
	int total_task_num;
	int hw_time;
}SPEEDINFO;
/*
*
* extern int Get2DStatus(void)
*
* @brief
*      get the status of the  2d hardware
*
* @param  
*
* @return
*      returns 2d status
*
* @example
*      
*/
EXPORT_SYMBOL 
int Get2DStatus(void);
/*
*
* extern int Render(FILLINFO * Fillinfo)
*
* @brief
*      a 2d render operation
*
* @param  
*      Fillinfo:
*				the pointer to a render config
*
* @return
*      returns 2d status
*
* @example
*      
*/
EXPORT_SYMBOL 
int Render(FILLINFO * Fillinfo);
/*
*
* int add_render_task(FILLINFO * Fillinfo)
*
* @brief
*     add_render_task
*
* @param  
*      
*				
*
* @return
*      returns status;0:ok;-1:failed
*
* @example
*      
*/
EXPORT_SYMBOL 
int add_render_task(FILLINFO * Fillinfo);
/*
*
* int wait_render_end(int *total_time,int * hw_time);
*
* @brief
*     wait_render_end
*
* @param  
*      
*				
*
* @return
*      returns status;0:ok;-1:failed
*
* @example
*      
*/
EXPORT_SYMBOL 
int wait_render_end(SPEEDINFO* SpeedInfo);
#endif

