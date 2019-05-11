#include "load_image_engine.h"
#include "sys_cfg.h"
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>

#define loadimg_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define loadimg_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

static void* loadimg_dllhandle=NULL;
LP_FUNCTIONS *loadimg_func=NULL;

//#define LOAD_PHOTO_USE_DYNAMIC

#if 1

int loadimg_dec_close()
{
	#ifdef LOAD_PHOTO_USE_DYNAMIC
	if(loadimg_dllhandle!=NULL){
		dlclose(loadimg_dllhandle);
		loadimg_dllhandle=NULL;
		loadimg_func = NULL;
	}
	#else 

	#endif
	return 0;
}

int loadimg_dec_open()
{
	#ifdef LOAD_PHOTO_USE_DYNAMIC
	char mmmlib[64];
	memset((void*)mmmlib,0,64);
	sprintf(mmmlib,"%s%s", AM_DYNAMIC_LIB_DIR,"lib_load_photo.so");

	loadimg_dec_close();
	loadimg_dllhandle = dlopen(mmmlib, RTLD_LAZY);
	if(loadimg_dllhandle==NULL){
		loadimg_err("Open Lib load photo.so failed!\n");
		return -1;
	}
	loadimg_func=(LP_FUNCTIONS *) dlsym(loadimg_dllhandle,"lp_functions");
	if(loadimg_func==NULL){
		loadimg_err("Get lp_functions failed!\n");
		loadimg_dec_close();
		return -1;
	}
	#else 

	#endif
	return 0;
}


/**
@brief using load_photo to  parser the pictures
@param[in] io_layer	: the operations using for load_photo so when parsering the photo
@param[in] img_info	: the output information to tell the parser
@return	see LP_RET in load_photo_api.h
**/
int loadimg_show_pic(io_layer_t* io_layer,img_info_t* img_info)
{
	LP_INPUT_PARAM lp_param;
	LP_RET lp_ret=LP_DECODE_ERROR;
	LP_LINEAR_BUFFER *lp_buffer;
	memset(&lp_param,0,sizeof(LP_INPUT_PARAM));

	if(io_layer==NULL || img_info==NULL){
		loadimg_err("Param Error!");
		return lp_ret;
	}
	
	lp_param.handle = io_layer->handle;
	lp_param.lp_fread = io_layer->img_fread;
	lp_param.lp_fseek_cur = io_layer->img_fseek_cur;
	lp_param.lp_fseek_end = io_layer->img_fseek_end;
	lp_param.lp_fseek_set = io_layer->img_fseek_set;
	lp_param.lp_ftell = io_layer->img_ftell;
	lp_param.lp_free = io_layer->img_free;
	lp_param.lp_malloc = io_layer->img_malloc;
	lp_param.lp_realloc = io_layer->img_realloc;

	lp_param.lp_get_bus_addr = io_layer->lp_get_bus_addr;

	lp_param.out_pic_width = img_info->out_pic_width;
	lp_param.out_pic_height = img_info->out_pic_height;
	lp_param.out_pic_pos_x = img_info->out_pic_pos_x;
	lp_param.out_pic_pos_y = img_info->out_pic_pos_y;


	lp_param.out_buf_width = img_info->buf_width;
	lp_param.out_buf_height = img_info->buf_height;

	lp_param.output_pix_fmt = img_info->format;

	//the folling buffer may changed when the format is changed
	//lp_param.out_buf_y = img_info->get_buffer(lp_param.out_buf_width,lp_param.out_buf_height);
	//lp_param.out_buf_uv.buf = NULL;
	//lp_param.out_buf_v.buf = NULL;
	memcpy(&lp_param.out_buf_y,&img_info->buf_info.buf_y,sizeof(LP_LINEAR_BUFFER));
	memcpy(&lp_param.out_buf_uv,&img_info->buf_info.buf_uv,sizeof(LP_LINEAR_BUFFER));
	memcpy(&lp_param.out_buf_v,&img_info->buf_info.buf_v,sizeof(LP_LINEAR_BUFFER));
	
	loadimg_info("Y buf=0x%x,size=%d,bus_addr=0x%x",lp_param.out_buf_y.buf,lp_param.out_buf_y.size,\
		lp_param.out_buf_y.bus_addr);
	
	#ifdef LOAD_PHOTO_USE_DYNAMIC
	if(loadimg_func!=NULL){
		switch(img_info->file_format){
			case FILE_FORMAT_JPEG:
				lp_ret = loadimg_func->load_jpeg(&lp_param);
				break;
			case FILE_FORMAT_BMP:
				lp_ret = loadimg_func->load_bmp(&lp_param);
				break;
			case FILE_FORMAT_PNG:
				lp_ret = loadimg_func->load_png(&lp_param);
				break;
			case FILE_FORMAT_STATIC_GIF:
				lp_ret = loadimg_func->load_static_gif(&lp_param);
				break;
			default:
				loadimg_err("Error File Format\n");
				break;
		}
	}
	#else 
		loadimg_info("File Format==%d",img_info->file_format);
		switch(img_info->file_format){
			case FILE_FORMAT_JPEG:
				lp_ret = load_jpeg(&lp_param);
				break;
			case FILE_FORMAT_BMP:
				lp_ret = load_bmp(&lp_param);
				break;
			case FILE_FORMAT_PNG:
				lp_ret = load_png(&lp_param);
				break;
			case FILE_FORMAT_STATIC_GIF:
				lp_ret = load_static_gif(&lp_param);
				break;
			default:
				loadimg_err("Error File Format\n");
				break;
		}

	#endif
	return lp_ret;
}



#if 1
loadimg_gifres_t * loadimg_get_gifres(io_layer_t* io_layer)
{
	loadimg_gifres_t * gifres=NULL;
	gifres = io_layer->img_malloc(sizeof(loadimg_gifres_t));
	if(gifres==NULL)
		loadimg_info("Sorry Malloc Failed!");
	return gifres;
}

void loadimg_free_gifres(io_layer_t* io_layer,loadimg_gifres_t *gifres)
{
	if(gifres!=NULL){
		 io_layer->img_free((void*)gifres);
	}
}


/**
@brief start to load the gif
@param[in] io_layer	: the operations using for load_photo so when parsering the photo
@param[in] img_info	: the output information to tell the parser
@param[out] gifres		: the returned value of the middle ware will be stored in the curframe_decret and curframe_delay
@return the pointer to where the information of gif be stored, if failed, it will be NULL
**/

int  loadimg_gif_load_start(io_layer_t* io_layer,img_info_t* img_info,loadimg_gifres_t * gifres)
{
	LP_INPUT_PARAM lp_param;
	LP_RET lp_ret=LP_DECODE_ERROR;

	if(io_layer==NULL || img_info==NULL){
		loadimg_err("Param Error!");
		return -1;
	}
	
	if(img_info->file_format!=FILE_FORMAT_DYNAMIC_GIF){
		loadimg_err("Format is error!,format=%d",img_info->format);
		return -1;
	}
		
	memset(&lp_param,0,sizeof(LP_INPUT_PARAM));
	
	lp_param.handle = io_layer->handle;
	lp_param.lp_fread = io_layer->img_fread;
	lp_param.lp_fseek_cur = io_layer->img_fseek_cur;
	lp_param.lp_fseek_end = io_layer->img_fseek_end;
	lp_param.lp_fseek_set = io_layer->img_fseek_set;
	lp_param.lp_ftell = io_layer->img_ftell;
	lp_param.lp_free = io_layer->img_free;
	lp_param.lp_malloc = io_layer->img_malloc;
	lp_param.lp_realloc = io_layer->img_realloc;

	lp_param.lp_get_bus_addr = io_layer->lp_get_bus_addr;

	lp_param.out_pic_width = img_info->out_pic_width;
	lp_param.out_pic_height = img_info->out_pic_height;
	lp_param.out_pic_pos_x = img_info->out_pic_pos_x;
	lp_param.out_pic_pos_y = img_info->out_pic_pos_y;


	lp_param.out_buf_width = img_info->buf_width;
	lp_param.out_buf_height = img_info->buf_height;

	lp_param.output_pix_fmt = img_info->format;	

	memcpy(&lp_param.out_buf_y,&img_info->buf_info.buf_y,sizeof(LP_LINEAR_BUFFER));
	memcpy(&lp_param.out_buf_uv,&img_info->buf_info.buf_uv,sizeof(LP_LINEAR_BUFFER));
	memcpy(&lp_param.out_buf_v,&img_info->buf_info.buf_v,sizeof(LP_LINEAR_BUFFER));
	
	loadimg_info("Y buf=0x%x,size=%d,bus_addr=0x%x",lp_param.out_buf_y.buf,lp_param.out_buf_y.size,\
		lp_param.out_buf_y.bus_addr);
	
	#ifdef LOAD_PHOTO_USE_DYNAMIC
	if(loadimg_func!=NULL){
		lp_ret = loadimg_func->load_gif_start(&gifres->gif_handle,&lp_param,&gifres->curframe_delay);
	}
	#else 
		lp_ret = load_gif_start(&gifres->gif_handle,&lp_param,&gifres->curframe_delay);
	#endif
	
	gifres->curframe_decret = lp_ret;
	
	loadimg_info("ret=%d,gifhandle=0x%x,delaytime=%d",lp_ret,gifres->gif_handle,gifres->curframe_delay);

	return 0;
	
}

/**
@brief get next frame of gif
@param[in] io_layer	: the operations using for load_photo so when parsering the photo
@param[in] img_info	: the output information to tell the parser
@param[out] gifres		: the returned value of the middle ware will be stored in the curframe_decret and curframe_delay
@return 
	- -1	: failed 
	- 0	: succeed
**/
int loadimg_gif_get_next_frame(io_layer_t* io_layer,img_info_t* img_info,loadimg_gifres_t *gifres)
{
	LP_INPUT_PARAM lp_param;
	LP_RET lp_ret=LP_DECODE_ERROR;

	if(io_layer==NULL || gifres==NULL){
		loadimg_err("Param Error!");
		return -1;
	}

	lp_param.handle = gifres->file_handle;
	
	if(io_layer!=NULL){
		lp_param.lp_fread = io_layer->img_fread;
		lp_param.lp_fseek_cur = io_layer->img_fseek_cur;
		lp_param.lp_fseek_end = io_layer->img_fseek_end;
		lp_param.lp_fseek_set = io_layer->img_fseek_set;
		lp_param.lp_ftell = io_layer->img_ftell;
		lp_param.lp_free = io_layer->img_free;
		lp_param.lp_malloc = io_layer->img_malloc;
		lp_param.lp_realloc = io_layer->img_realloc;
		
		lp_param.lp_get_bus_addr = io_layer->lp_get_bus_addr;
	}
	
	if(img_info!=NULL){
		lp_param.out_pic_width = img_info->out_pic_width;
		lp_param.out_pic_height = img_info->out_pic_height;
		lp_param.out_pic_pos_x = img_info->out_pic_pos_x;
		lp_param.out_pic_pos_y = img_info->out_pic_pos_y;

		lp_param.out_buf_width = img_info->buf_width;
		lp_param.out_buf_height = img_info->buf_height;

		lp_param.output_pix_fmt = img_info->format;	

		memcpy(&lp_param.out_buf_y,&img_info->buf_info.buf_y,sizeof(LP_LINEAR_BUFFER));
		memcpy(&lp_param.out_buf_uv,&img_info->buf_info.buf_uv,sizeof(LP_LINEAR_BUFFER));
		memcpy(&lp_param.out_buf_v,&img_info->buf_info.buf_v,sizeof(LP_LINEAR_BUFFER));	
	}

	loadimg_info("");

	#ifdef LOAD_PHOTO_USE_DYNAMIC
	if(loadimg_func!=NULL){
		lp_ret = loadimg_func->load_gif_play_next_frame(gifres->gif_handle,&lp_param,&gifres->curframe_delay);
	}
	#else 
		lp_ret = load_gif_play_next_frame(gifres->gif_handle,&lp_param,&gifres->curframe_delay);
	#endif

	gifres->curframe_decret = lp_ret;
	
	loadimg_info("lp_ret=%d,gifhandle=0x%x,delaytime=%d",lp_ret,gifres->gif_handle,gifres->curframe_delay);
	
	return 0;
}

/**
@brief free the resource which is occupied by the middle ware
@param[in] io_layer	: the operations using for load_photo so when parsering the photo
@param[in] img_info	: the output information to tell the parser
@param[out] gifres		: the returned value of the middle ware will be stored in the curframe_decret and curframe_delay
@return 
	- -1	: failed 
	- 0	: succeed
**/

int loadimg_gif_load_end(io_layer_t* io_layer,img_info_t* img_info,loadimg_gifres_t *gifres)
{
	LP_INPUT_PARAM lp_param;
	LP_RET lp_ret=LP_DECODE_ERROR;

	if(io_layer==NULL || gifres==NULL){
		loadimg_err("Param Error!");
		return -1;
	}

	lp_param.handle = gifres->file_handle;
	
	if(io_layer!=NULL){
		lp_param.lp_fread = io_layer->img_fread;
		lp_param.lp_fseek_cur = io_layer->img_fseek_cur;
		lp_param.lp_fseek_end = io_layer->img_fseek_end;
		lp_param.lp_fseek_set = io_layer->img_fseek_set;
		lp_param.lp_ftell = io_layer->img_ftell;
		lp_param.lp_free = io_layer->img_free;
		lp_param.lp_malloc = io_layer->img_malloc;
		lp_param.lp_realloc = io_layer->img_realloc;
		
		lp_param.lp_get_bus_addr = io_layer->lp_get_bus_addr;
	}
	
	if(img_info!=NULL){
		lp_param.out_pic_width = img_info->out_pic_width;
		lp_param.out_pic_height = img_info->out_pic_height;
		lp_param.out_pic_pos_x = img_info->out_pic_pos_x;
		lp_param.out_pic_pos_y = img_info->out_pic_pos_y;

		lp_param.out_buf_width = img_info->buf_width;
		lp_param.out_buf_height = img_info->buf_height;

		lp_param.output_pix_fmt = img_info->format;	

		memcpy(&lp_param.out_buf_y,&img_info->buf_info.buf_y,sizeof(LP_LINEAR_BUFFER));
		memcpy(&lp_param.out_buf_uv,&img_info->buf_info.buf_uv,sizeof(LP_LINEAR_BUFFER));
		memcpy(&lp_param.out_buf_v,&img_info->buf_info.buf_v,sizeof(LP_LINEAR_BUFFER));	
	}

	loadimg_info("");
	
	#ifdef LOAD_PHOTO_USE_DYNAMIC
	if(loadimg_func!=NULL){
		lp_ret = loadimg_func->load_gif_end(gifres->gif_handle,&lp_param);
	}
	#else 
		lp_ret = load_gif_end(gifres->gif_handle,&lp_param);
	#endif
	
	loadimg_info("lp_ret=%d,gifhandle=0x%x,delaytime=%d",lp_ret,gifres->gif_handle,gifres->curframe_delay);

	return 0;
}
#endif


#endif

