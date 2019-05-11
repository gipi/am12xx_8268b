
/* load photo api */

#ifndef __LOAD_PHOTO_API_H__
#define __LOAD_PHOTO_API_H__

#ifdef __cplusplus
extern "C"
{
#endif
	
typedef enum tagLPPixFormat
{
	LP_FMT_YCBCR_4_2_0_SEMIPLANAR       = 0,
	LP_FMT_YCBCR_4_2_2_INTERLEAVED      = 1,
	LP_FMT_RGB_32                       = 2
}LP_PIX_FMT;
	
typedef enum tagLPRet
{
	LP_OK                   = 0,
	LP_DECODE_ERROR   		= -1,
	LP_UNSUPPORTED_FMT 		= -2,
	LP_GIF_END              = 1
}LP_RET;
	
typedef struct tagLPLinearBuf
{
	unsigned char *buf;
	unsigned long bus_addr;
	unsigned long size;
}LP_LINEAR_BUFFER;
	
typedef struct tagLPInputParam
{
	void *handle;
	int (*lp_fread)(void*, void*, int);
	int (*lp_fseek_set)(void*, int);
	int (*lp_fseek_cur)(void*, int);
	int (*lp_fseek_end)(void*, int);
	int (*lp_ftell)(void*);
	void *(*lp_malloc)(int);
	unsigned long (*lp_get_bus_addr)(unsigned long);
	void *(*lp_realloc)(void*, int);
	void (*lp_free)(void*);
	LP_LINEAR_BUFFER out_buf_y;    //y or RGB
	LP_LINEAR_BUFFER out_buf_uv;   //uv or only u
	LP_LINEAR_BUFFER out_buf_v;    //v
	unsigned long out_buf_width;
	unsigned long out_buf_height;
	LP_PIX_FMT output_pix_fmt;
	unsigned long out_pic_width;
	unsigned long out_pic_height;
	long out_pic_pos_x;
	long out_pic_pos_y;
}LP_INPUT_PARAM;
	
typedef struct tagLPFunctions
{
	LP_RET (*load_jpeg)(LP_INPUT_PARAM *lp_param);
	LP_RET (*load_bmp)(LP_INPUT_PARAM *lp_param);
	LP_RET (*load_png)(LP_INPUT_PARAM *lp_param);
	LP_RET (*load_static_gif)(LP_INPUT_PARAM *lp_param);
	LP_RET (*load_gif_start)(void **p_gif_handle, LP_INPUT_PARAM *lp_param, unsigned short *delay_time);
	LP_RET (*load_gif_play_next_frame)(void *gif_handle, LP_INPUT_PARAM *lp_param, unsigned short *delay_time);
	LP_RET (*load_gif_end)(void *gif_handle, LP_INPUT_PARAM *lp_param);
}LP_FUNCTIONS;
	
#ifdef WIN32
	LP_FUNCTIONS *get_lp_functions();
#endif
	
#ifdef __cplusplus
}
#endif

#endif  //__LOAD_PHOTO_API_H__