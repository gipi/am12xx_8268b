#ifndef SWF_2D_H
#define SWF_2D_H

#include "swf_types.h"

typedef struct _FillInfo
{
	INT32U		DefColor;		//DEFAULT_COLOR_CONF (only when input_type = 0)
	SWF_MATRIX  Matrix;			//CONTAIN COEFF_A_B_C_D
								//ScaleX : CONTAIN COEFF_A
								//ScaleY : CONTAIN COEFF_D	
								//RotateSkew0 : CONTAIN COEFF_C
								//RotateSkew1 : CONTAIN COEFF_B
								//TranslateX : input_start_x <- input_start_x + TranslateX
								//TranslateY : input_start_y <- input_start_y + TranslateY
	SWF_CXFORM  Cxform;				//CONTAIN FORMAT_TRANS_COEFF (only when input_type = 1)
								//R' = max(0, min(((R * RedMultTerm) / 256) + RedAddTerm, 255))
								//G' = max(0, min(((G * GreenMultTerm) / 256) + GreenAddTerm, 255))
								//B' = max(0, min(((B * BlueMultTerm) / 256) + BlueAddTerm, 255))
								//A' = max(0, min(((A * AlphaMultTerm) / 256) + AlphaAddTerm, 255))
								
	int		input_addr;			//SRC_CENTER_ADDR
	int		input_type;			// 0:default color 1:bitmap
	int		input_stride;		//STRIDE_CONF
	int		input_start_x;		//HEAD_POINT_ROTATE_X_COORDI,16.16
	int		input_start_y;		//HEAD_POINT_ROTATE_Y_COORDI,16.16
	int		input_width;		//SRC_BOUNDARY_X_COORDI
	int		input_height;		//SRC_BOUNDARY_Y_COORDI
	int		input_repeat;		//0: extended 1:repeat
	int		input_format;		//0: ARGB 3: YUV422 (only when input_type = 1)
	int		input_need_blend;		//0: do not need 1:need
	int		input_need_interpolate;	//0: do not need 1:need

	int		output_addr;		//DST_HEAD_ADDR_CONF
	int		output_type;		// 0:rect 1:pair 
	int		output_offset;		//DST_HEAD_ADDR_CONF
	int		output_stride;		//STRIDE_CONF
	int		output_width;		//SCAN_AREA_CONF
	int		output_height;		//SCAN_AREA_CONF
	int		output_format;		//0: ARGB 3: YUV422

	int		pair_addr;			//POINT_PAIR_ADDR
	int		pair_num;			//POINT_PAIR_NUM

	int		z_buffer;			//2D_ZBUF_HEAD_ADDR_CONF
	int		z_stride;			//2D_ZBUF_STRIDE_CONF
	int		z_depth;			//2D_COLOR_KEY_CONF

//#if ANTI_ALIASING == 1
#if AM_CHIP_ID == 1207 || AM_CHIP_ID == 1209
	int		aa_mode;		//0:reserved, 1:low-quality, 2:mid-quality, 3:high-quality
#endif
} FILLINFO;

#endif

