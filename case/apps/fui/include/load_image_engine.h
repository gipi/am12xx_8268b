#ifndef _LOAD_IMAGE_ENGINE_H_

#include "load_photo_api.h"

#define _LOAD_IMAGE_ENGINE_H_

typedef enum{
	FILE_FORMAT_JPEG,
	FILE_FORMAT_BMP,
	FILE_FORMAT_PNG,
	FILE_FORMAT_STATIC_GIF,
	FILE_FORMAT_DYNAMIC_GIF
}loadimg_file_formt_e;


typedef struct io_layer_s{
	void *handle;										///<handle to the file
	long (*img_fread)(void *, unsigned char *, unsigned long);	///<read operation
	long (*img_fseek_set)(void *, long);						///<seek set operation
	long (*img_fseek_cur)(void *, long);					///<seek cur operation
	long (*img_fseek_end)(void *, long);					///<seek end operation
	long (*img_ftell)(void *);								///<ftell operation
	void *(*img_malloc)(unsigned long);						///<malloc operation
	void *(*img_realloc)(void *, unsigned long);				///<remalloc operation
	void (*img_free)(void *);								///<free operation
	unsigned long (*lp_get_bus_addr)(unsigned long);			///<get bus addr
}io_layer_t;

typedef struct buf_info_s{
	LP_LINEAR_BUFFER buf_y;    //y or RGB
	LP_LINEAR_BUFFER buf_uv;   //uv or only u
	LP_LINEAR_BUFFER buf_v;    //v
}buf_info_t;

typedef struct img_info_s{
	loadimg_file_formt_e file_format;
	LP_PIX_FMT format;
	unsigned long out_pic_width;
	unsigned long out_pic_height;
	long out_pic_pos_x;
	long out_pic_pos_y;
	int buf_width;
	int buf_height;
	buf_info_t buf_info;
}img_info_t;


typedef struct loadimg_gif_res_s
{
	void *gif_handle;///<the pointer which returned by middle ware
	void *file_handle; ///<the handle point to the file which had been opened
	LP_RET curframe_decret;///<the decoding result which return from middle ware when decoding current frame
	unsigned short curframe_delay;///<the delay time of current frame
	void * output_buffer;///<the pointer to the output buffer
	void * target;///< the movieclip
	int target_h;///<the height of the movieclip
	int target_w;///<the width of the movieclip
}loadimg_gifres_t;


int loadimg_dec_open();
int loadimg_dec_close();
int loadimg_show_pic(io_layer_t* io_layer,img_info_t* img_info);


loadimg_gifres_t * loadimg_get_gifres(io_layer_t* io_layer);
void loadimg_free_gifres(io_layer_t* io_layer,loadimg_gifres_t *gifres);

int  loadimg_gif_load_start(io_layer_t* io_layer,img_info_t* img_info,loadimg_gifres_t * gifres);
int loadimg_gif_get_next_frame(io_layer_t* io_layer,img_info_t* img_info,loadimg_gifres_t *gifres);
int loadimg_gif_load_end(io_layer_t* io_layer,img_info_t* img_info,loadimg_gifres_t *gifres);



#endif
