#include "fui_common.h"
#include "swf_2d.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include "swfdec.h"
#include <string.h>
#include "image_decode.h"
#include "photo_engine.h"
#include "load_photo_api.h"
#include "osapi.h"
#include "fui_memfs.h"
#include "audio_midware.h"
#include "audio_common.h"
#include "../../../include/ezcast_public.h"

extern void* adllhandle;
extern audio_dec_open	a_open;
extern audio_dec_cmd	a_cmd;
extern audio_dec_close	a_close;
extern struct _audio_decode_info audio_decode_info[AUDIO_ENGINE_MAX];

//void * audio_mutex=NULL;
//static int swf_audio_state=0;
#define FUI_EVENT_SOUND 0x55
#define FUI_STREAM_SOUND 0xaa

#define DEFINE_AUDIO_IO(x) 	audio##x##_read_stream




static void Initial_Identify_MATRIX(SWF_MATRIX * pMatrix)
{
	pMatrix->ScaleX=0x10000;
	pMatrix->ScaleY=0x10000;
	pMatrix->RotateSkew0=0;
	pMatrix->RotateSkew1=0;
	pMatrix->TranslateX=0;
	pMatrix->TranslateY=0;
}

static void Initial_CXFORM(SWF_CXFORM * colortransform)
{
	colortransform->RedMultTerm = 0x100;
	colortransform->GreenMultTerm = 0x100;
	colortransform->BlueMultTerm = 0x100;
	colortransform->AlphaMultTerm = 0x100;
	colortransform->RedAddTerm = 0;
	colortransform->GreenAddTerm =0;
	colortransform->BlueAddTerm = 0;
	colortransform->AlphaAddTerm = 0;
}

/**
* for some version of the jpeg data, it is not standard,so here
* we do some calibration.
*/
static int jpeg_baseline_check(unsigned char *buf, int size)
{
	unsigned char *p = buf;
	unsigned char *p_end = buf+size;
	unsigned char *p_soi = NULL;
	int dqt_flg = 0;
	int dht_flg = 0;
	int sof_flg = 0;
	int len;
	
	while(p < p_end-1)
	{
		if(p[0] == 0xff)
		{
			switch(p[1])
			{
			case 0xc0:	//SOF0
				sof_flg = 1;
				len = (p[2]<<8)|p[3];
				p += 4+(len < 2 ? len : len-2);
				break;
			case 0xc1:	//SOF1
			case 0xc2:	//SOF2
			case 0xc3:	//SOF3
				len = (p[2]<<8)|p[3];
				p += 4+(len < 2 ? len : len-2);
				break;
			case 0xc4:	//DHT
				dht_flg = 1;
				len = (p[2]<<8)|p[3];
				p += 4+(len < 2 ? len : len-2);
				break;
			case 0xc5:	//SOF5
			case 0xc6:
			case 0xc7:
			case 0xc8:
			case 0xc9:
			case 0xca:
			case 0xcb:	//SOF11
			case 0xcc:	//DAC
			case 0xcd:	//SOF13
			case 0xce:	//SOF14
			case 0xcf:	//SOF15
				len = (p[2]<<8)|p[3];
				p += 4+(len < 2 ? len : len-2);
				break;
			case 0xd0:	//RST0
			case 0xd1:	//RST1
			case 0xd2:	//RST2
			case 0xd3:	//RST3
			case 0xd4:	//RST4
			case 0xd5:	//RST5
			case 0xd6:	//RST6
			case 0xd7:	//RST7
				p += 2;
				break;
			case 0xd8:	//SOI
				if(!p_soi)
				{
					p_soi = p;
					p += 2;
				}
				else
				{
					memmove(p, p+2, p_end-(p+2));
					p_end -= 2;
				}
				break;
			case 0xd9:	//EOI
				memmove(p, p+2, p_end-(p+2));
				p_end -= 2;
				break;
			case 0xda:	//SOS
				if(p_soi && sof_flg && dqt_flg && dht_flg)
					return p_end-buf;
				p += 2;
				break;
			case 0xdb:	//DQT
				dqt_flg = 1;
				len = (p[2]<<8)|p[3];
				p += 4+(len < 2 ? len : len-2);
				break;
			case 0xdc:	//DNL
			case 0xdd:	//DRI
			case 0xde:	//DHP
			case 0xdf:	//EXP
			case 0xe0:	//APP0
			case 0xe1:
			case 0xe2:
			case 0xe3:
			case 0xe4:
			case 0xe5:
			case 0xe6:
			case 0xe7:
			case 0xe8:
			case 0xe9:
			case 0xea:
			case 0xeb:
			case 0xec:
			case 0xed:
			case 0xef:	//APP15
			case 0xf0:	//JPG0
			case 0xf1:
			case 0xf2:
			case 0xf3:
			case 0xf4:
			case 0xf5:
			case 0xf6:
			case 0xf7:
			case 0xf8:
			case 0xf9:
			case 0xfa:
			case 0xfb:
			case 0xfc:
			case 0xfd:
			case 0xfe:	//COM
				len = (p[2]<<8)|p[3];
				p += 4+(len < 2 ? len : len-2);
				break;
			default:
				p += 2;
				break;
			}
		}
		else
		{
			p++;
		}
	}

	return 0;
}
static long test_read(void *fp, unsigned char *buf, unsigned long size)
{
	FILE *file = (FILE *)fp;
	return OSfread(buf, 1, size, file);
}

void dump_memory(unsigned char *buf, int len)
{
	int i;
	for(i=0; i<len; i++)
	{
		if(!(i%16))
			printf("\n");
		printf("0x%x, ", buf[i]);
	}
}
void memory_check(unsigned char *buf, int len)
{
	int i;
	printf("buf=%x\n",(int)buf);
	printf("len=%x\n",(int)len);
	for(i=0; i<len; i++)
	{
		if(buf[i] != 0xcc)
		{
			dump_memory(buf, len);
			break;
		}
	}
}
int  load_pic(LP_INPUT_PARAM *lp_param,char* type)
{
	LP_FUNCTIONS *lp_function;
	void *dll_handle = NULL;
	LP_RET lp_ret = LP_OK;
	
#ifdef WIN32
	lp_function = get_lp_functions();
#endif


#ifdef WIN32
	if(OSstrcmp(type,".jpg")==0){
		lp_ret = lp_function->load_jpeg(lp_param);
	}
	else if(OSstrcmp(type,".bmp")==0){
		lp_ret = lp_function->load_bmp(lp_param);
	}
	else if(OSstrcmp(type,".png")==0){
		lp_ret = lp_function->load_png(lp_param);
	}
	else if(OSstrcmp(type,".gif")==0){
		lp_ret = lp_function->load_static_gif(lp_param);
	}
	
	if (lp_ret != LP_OK){
		OSprintf("load_pic error\n");
	}
#else
	if(OSstrcmp(type,".jpg")==0){
		lp_ret = load_jpeg(lp_param);
	}
	else if(OSstrcmp(type,".bmp")==0){
		lp_ret = load_bmp(lp_param);
	}
	else if(OSstrcmp(type,".png")==0){
		lp_ret = load_png(lp_param);
	}
	else if(OSstrcmp(type,".gif")==0){
		lp_ret = load_static_gif(lp_param);
	}
	
	if (lp_ret != LP_OK){
		OSprintf("load_pic error\n");
	}
#endif
	
end:

	return lp_ret;
}

static int fui_dec_internal_jpeg(char *jpegdata, long jpegsize, unsigned char **decdata, int *decw, int *dech, int *decstride)
{
	int w,h;
	void *fp;
	LP_INPUT_PARAM lp_param;
	long len;
	int ret=0;
	
	/**
	* make width and height 2 aligned.
	*/
	w = ((*decw+1)/2)*2;
	///h = ((*dech+1)/2)*2;
	h = *dech;
	*decstride = w;

	len = w*(h+1)*2;

	/**
	* malloc decoder data.
	*/
	*decdata = (unsigned char *)SWF_Malloc(len);
	if(*decdata == NULL){
		printf("fui_dec_internal_jpeg malloc error\n");
		return LP_DECODE_ERROR;
	}

	/**
	* open memory fs.
	*/
	fp = fui_memfs_open(jpegdata, jpegsize);
	if(fp == NULL){
		SWF_Free(*decdata);
		*decdata = NULL;
		printf("fui_dec_internal_jpeg fp error\n");
		return LP_DECODE_ERROR;
	}

	/**
	* begin decode.
	*/
	lp_param.handle = fp;
	lp_param.lp_fread = fui_memfs_read;
	lp_param.lp_fseek_cur = fui_memfs_seek_cur;
	lp_param.lp_fseek_end = fui_memfs_seek_end;
	lp_param.lp_fseek_set = fui_memfs_seek_set;
	lp_param.lp_ftell = fui_memfs_tell;	
	
	lp_param.lp_malloc = (void *(*)(unsigned long))SWF_Malloc;
	lp_param.lp_free = SWF_Free;
	lp_param.lp_get_bus_addr = fui_get_bus_address;
	lp_param.lp_realloc = NULL;
	
	lp_param.output_pix_fmt = LP_FMT_YCBCR_4_2_2_INTERLEAVED;
	lp_param.out_buf_y.buf = *decdata;
	lp_param.out_buf_y.bus_addr = fui_get_bus_address((unsigned long)decdata);
	lp_param.out_buf_y.size = len;
	lp_param.out_buf_uv.buf = NULL;
	lp_param.out_buf_v.buf = NULL;
	lp_param.out_buf_width = w;
	lp_param.out_buf_height = h;
	lp_param.out_pic_pos_x = 0;
	lp_param.out_pic_pos_y = 0;
	lp_param.out_pic_width = w;
	lp_param.out_pic_height = h;

	/**
	* do decode.
	*/
	ret = load_pic(&lp_param,".jpg");
	if(ret != LP_OK){
		SWF_Free(*decdata);
		*decdata = NULL;
	}

	if(fp){
		fui_memfs_close(fp);
		fp = NULL;
	}
	
	return ret;
	
}

static int fui_dec_external_image(SWF_IMAGE_INFO * info)
{
	LP_INPUT_PARAM lp_param;
	int ret;
	
	lp_param.handle = (void *)OSfopen((char *)info->jpeg_data, "r");
	lp_param.lp_fread = test_read;
	lp_param.lp_fseek_cur = (long (*)(void *,long))OSfseek_cur;
	lp_param.lp_fseek_end = (long (*)(void *,long))OSfseek_end;
	lp_param.lp_fseek_set = (long (*)(void *,long))OSfseek_set;
	lp_param.lp_ftell = OSftell;	
	lp_param.lp_malloc = (void *(*)(unsigned long))SWF_Malloc;
	lp_param.lp_free = SWF_Free;
	lp_param.lp_get_bus_addr = fui_get_bus_address;
	lp_param.lp_realloc = NULL;
	lp_param.output_pix_fmt = info->flag&0x3;
	if(lp_param.output_pix_fmt == SWF_BMP_FMT_ARGB){
		lp_param.output_pix_fmt = LP_FMT_RGB_32;
	}
	else{
		lp_param.output_pix_fmt = LP_FMT_YCBCR_4_2_2_INTERLEAVED;
	}
	lp_param.out_buf_y.buf = info->pixel;
	lp_param.out_buf_uv.buf = NULL;
	lp_param.out_buf_v.buf = NULL;
	lp_param.out_buf_width = info->width;
	lp_param.out_buf_height = info->height;
	lp_param.out_pic_pos_x = 0;
	lp_param.out_pic_pos_y = 0;
	lp_param.out_pic_width = info->width;
	lp_param.out_pic_height = info->height;

	if(info->dataType == SWF_BMP_TYPE_PNG){
		ret = load_pic(&lp_param,".png");
	}
	else if(info->dataType == SWF_BMP_TYPE_BMP){
		ret = load_pic(&lp_param,".bmp");
	}
	else if(info->dataType == SWF_BMP_TYPE_GIF){
		ret = load_pic(&lp_param,".gif");
	}
	else if(info->dataType == SWF_BMP_TYPE_JPEG){
		ret = load_pic(&lp_param,".jpg");
	}
	else{
		ret = LP_UNSUPPORTED_FMT;
	}

	OSfclose(lp_param.handle);

	return 0;
}

/**
* Must be sure that the src_w will not exceed the dst buffer's width.
*/
#define FUI_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define FUI_MIN(a,b) (((a) < (b)) ? (a) : (b))

#define FUI_YUV2RGB(r, g, b, y, u, v)\
	{\
		int c,d,e;\
		c = y - 16;\
		d = u - 128;\
		e = v - 128;\
		r = FUI_MAX(0,FUI_MIN(255,(298 * c + 409 * e + 128) >> 8));\
		g = FUI_MAX(0,FUI_MIN(255,(298 * c - 100 * d - 208 * e + 128) >> 8));\
		b = FUI_MAX(0,FUI_MIN(255,(298 * c + 516 * d + 128) >> 8));\
	}

static int fui_yuv422_to_argb(
	unsigned char *src,
	unsigned int src_w,
	unsigned int src_h,
	unsigned int src_stride,
	unsigned char *dst,
	unsigned int dst_stride)
{
	unsigned int *psrc = NULL;
	unsigned int *pdst = NULL;
	int i,j;

	if((src == NULL) || (dst == NULL)){
		return -1;
	}

	for(i=0;i<src_h;i++){
		psrc = (unsigned int *)(src + i*src_stride*2);
		pdst = (unsigned int *)(dst + i*dst_stride*4);

		for(j=0;j<src_w/2;j++){
			unsigned int yuv;
			unsigned int y1,y2,u,v;
			unsigned int r,g,b;
			unsigned int argb;

			/// get y,u,v
			yuv = *(psrc + j);
			y1 = yuv&0xff;
			y2 = (yuv>>16)&0xff;
			u = (yuv>>8)&0xff;
			v = (yuv>>24)&0xff;

			/// get rgb1
			FUI_YUV2RGB(r,g,b,y1,u,v);
			argb = (b&0xff) | ((g<<8)&0xff00) | ((r<<16)&0xff0000) | 0xff000000; 
			*(pdst + j*2) = argb;

			FUI_YUV2RGB(r,g,b,y2,u,v);
			argb = (b&0xff) | ((g<<8)&0xff00) | ((r<<16)&0xff0000) | 0xff000000; 
			*(pdst + j*2 +1) = argb;
		}
	}

	return 0;
}


int fui_decode_bitmap(SWF_IMAGE_INFO * info)
{
	unsigned int len,mode=0;
	unsigned char *buf=NULL;
	IMG_DECODE_INFO_S rlt;
	
	
	if(info->flag & 0x10)
	{
		info->width /= 2;
		info->height /= 2;	
	}
	
	if(info->jpeg_size == 0)
	{
		int softdec = 0;

		if(info->width== 0 || info->height == 0){
			info->width = 80;
			info->height = 60;
		}
		info->width  = (info->width + 7) / 8 * 8;
		info->height = (info->height + 1) / 2 * 2;
		info->stride = info->width;
		
		/**
		* For png and gif, can only be decoded to ARGB.
		* For image that with width<48 or height<48, must use software
		*    to decode.
		*/

		if((info->dataType == SWF_BMP_TYPE_PNG) || (info->dataType == SWF_BMP_TYPE_GIF)){
			info->flag = (info->flag&0xf0)|SWF_BMP_FMT_ARGB;
			softdec = 1;
		}
		else{
			if((info->width<=48) || (info->height<=48)){
				softdec = 1;
				if(info->dataType == SWF_BMP_TYPE_BMP){
					info->flag = (info->flag&0xf0)|SWF_BMP_FMT_ARGB;
				}
			}
		}

		if(softdec){
			info->GetStatus = NULL;
		}
		else{
			info->GetStatus = get_bitmap_status;
			mode=get_image_dec_mode();
		}
		
		if(((info->flag)&0x3)== SWF_BMP_FMT_ARGB){
			len=info->width*(info->height+1)*4;	
		}
		else{
			len=info->width*(info->height+1)*2;
		}
		
		info->pixel = (unsigned char *)SWF_Malloc(len);

		if((info->handle)!=NULL){
			softdec = 0;
		}
		
		if(info->pixel != NULL)
		{

			buf =(unsigned char*)info->pixel;
			
			if(softdec){
				/** use soft decode */
				fui_dec_external_image(info);
			}
			else{
				if((info->handle)==NULL)
				{
					if(info->width<=160 && info->height<=160){
						img_dec_send_cmd(BG_IMG_DEC_PREV,mode,info->jpeg_data,buf,info->width,info->height,info->id);
					}
					else{
						img_dec_send_cmd(BG_IMG_DEC_FULL2BUF,mode,info->jpeg_data,buf,info->width,info->height,info->id);
					}
				}
				else{
					/**
					* used currently for Ebook. 
					*/
					LP_INPUT_PARAM lp_param;
					int ret;

					lp_param.handle = (void *)info->handle;
					lp_param.lp_fread = test_read;
					lp_param.lp_fseek_cur = (long (*)(void *,long))OSfseek_cur;
					lp_param.lp_fseek_end = (long (*)(void *,long))OSfseek_end;
					lp_param.lp_fseek_set = (long (*)(void *,long))OSfseek_set;
					lp_param.lp_ftell = OSftell;	
					lp_param.lp_malloc = (void *(*)(unsigned long))SWF_Malloc;
					lp_param.lp_free = SWF_Free;
					lp_param.lp_get_bus_addr = fui_get_bus_address;
					lp_param.lp_realloc = NULL;
					lp_param.output_pix_fmt = info->flag&0x3;
					if(lp_param.output_pix_fmt == SWF_BMP_FMT_ARGB){
						lp_param.output_pix_fmt = LP_FMT_RGB_32;
					}
					else{
						lp_param.output_pix_fmt = LP_FMT_YCBCR_4_2_2_INTERLEAVED;
					}
					//lp_param.output_pix_fmt = LP_FMT_YCBCR_4_2_2_INTERLEAVED;//;LP_FMT_YCBCR_4_2_0_SEMIPLANAR
					lp_param.out_buf_y.buf = buf;
					lp_param.out_buf_uv.buf = NULL;//output_buf + output_width * output_height;
					lp_param.out_buf_v.buf = NULL;
					lp_param.out_buf_width = info->width;
					lp_param.out_buf_height = info->height;
					lp_param.out_pic_pos_x = 0;
					lp_param.out_pic_pos_y = 0;
					lp_param.out_pic_width = info->width;
					lp_param.out_pic_height = info->height;
					ret=load_pic(&lp_param,(char *)info->jpeg_data);
					if(ret!=LP_OK){
						memset(info->pixel, 0x0, len);
					}
//					{
//						char fname[64];
//						
//						if(info->dataType == SWF_BMP_TYPE_JPEG)
//						{
//							sprintf(fname,"decode_out_%d_%d.yuv",info->width,info->height);
//							fwtie
//						}
//					}
				}
			}
			return 1;
		}

	}
	else
	{
		int err;
		long newSize;
		
		info->GetStatus = NULL;

		if( (info->width>0) && (info->height>0)){
			newSize = info->jpeg_size;
			newSize = jpeg_baseline_check(info->jpeg_data, info->jpeg_size);
			
			if(newSize>0){
				/**
				* use load photo instead of hantro.
				*/
		#if 1
		#if  (!EZMUSIC_ENABLE)
				fui_dec_internal_jpeg((char *)info->jpeg_data, newSize,&info->pixel, &info->width, &info->height, &info->stride);
		#endif
		#else
			#if AM_CHIP_ID == 1203 || AM_CHIP_ID == 1211 || AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213
				hantro_jpeg(info->jpeg_data, newSize, &info->pixel, &info->width, &info->height, &info->stride, info->flag & 0xF);
			#elif AM_CHIP_ID == 1207
				hantro_jpeg(info->jpeg_data, newSize, &info->pixel, &info->width, &info->height, &info->stride, SWF_BMP_FMT_YUV422);
			#endif
		#endif
			}
			else{
				info->pixel=NULL;
			}
		}
		else{
			info->pixel=NULL;
		}
			
		if(info->pixel != NULL)
		{
			
#if AM_CHIP_ID == 1211 || AM_CHIP_ID == 1220 || AM_CHIP_ID == 1213
			if((info->flag & 0xF) != SWF_BMP_FMT_YUV422)
			{
				unsigned char * pixel = SWF_Malloc(info->width * (info->height+1) * 4);
				int err;

				err = fui_yuv422_to_argb(info->pixel,info->width,info->height,info->stride,pixel,info->width);
				SWF_Free(info->pixel);
				if(err < 0){
					goto decout;
				}
				info->pixel  = pixel;
				info->stride = info->width;
			}
#endif


#if AM_CHIP_ID == 1207
			if((info->flag & 0xF) != SWF_BMP_FMT_YUV422)
			{
				unsigned char * pixel = SWF_Malloc(info->width * (info->height+1) * 4);
				FILLINFO fillinfo;
				fillinfo.input_type    = 1;
				fillinfo.input_addr    = (int)info->pixel;
				fillinfo.input_start_x = 0;
				fillinfo.input_start_y = 0;
				fillinfo.input_stride  = info->stride;
				fillinfo.input_repeat  = 0;
				fillinfo.input_need_blend = 0;
				fillinfo.input_need_interpolate = 0;
				fillinfo.input_format  = 3;
				fillinfo.input_width   = info->width;
				fillinfo.input_height  = info->height;
				fillinfo.output_type   = 0;
				fillinfo.output_format = 0;
				fillinfo.output_addr   = (int)pixel;
				fillinfo.output_offset = 0;
				fillinfo.output_stride = info->width;
				fillinfo.output_width  = info->width;
				fillinfo.output_height = info->height;
				fillinfo.z_buffer      = 0;
				fillinfo.blend_mode = 0;
				fillinfo.clip.Xmin = 0;
				fillinfo.clip.Xmax = info->width - 1;
				fillinfo.clip.Ymin = 0;
				fillinfo.clip.Ymax = info->height - 1;
				Initial_CXFORM(&fillinfo.Cxform);
				Initial_Identify_MATRIX(&fillinfo.Matrix);
				//r4k_dma_cache_inv(pixel, info->width * info->height * 4);
				SWF_2DFill(&fillinfo);
				SWF_Free(info->pixel);
				info->pixel  = pixel;
				info->stride = info->width;
			}
#endif
			return 1;
		}
	}

decout:
	info->width  = 4;
	info->height = 4;
	info->stride = info->width;
	info->pixel  = (unsigned char *)SWF_Malloc(info->width * (info->height+1) * 4);
	memset(info->pixel, 0x80, info->width * info->height * 4);
	printf("use default\n");
	return 0;
}

typedef struct
{
	char *buf;
	int len;                
}sdram_input_t;

void fui_audio_engine_close(int handle)
{
	int i;
	if(handle)
	{
		for(i=0;i<AUDIO_ENGINE_MAX;i++)
		{
			if(audio_decode_info[i].audio_engine&&audio_decode_info[i].s==handle)
			{
				a_cmd(audio_decode_info[i].audio_engine, STOP,0);
				a_close(audio_decode_info[i].audio_engine);
				memset(&(audio_decode_info[i]),0,sizeof(struct _audio_decode_info));
			}
		}	
		return;
	}
	for(i=0;i<AUDIO_ENGINE_MAX;i++)
	{
		if(audio_decode_info[i].audio_engine)
		{
			a_cmd(audio_decode_info[i].audio_engine, STOP,0);
			a_close(audio_decode_info[i].audio_engine);
			memset(&(audio_decode_info[i]),0,sizeof(struct _audio_decode_info));
		}
	}
}
int audio_read_stream(void *opaque,void *buf,int buf_size,int eg_sn)
{
	struct _audio_decode_info *info=&(audio_decode_info[eg_sn]);
	int size=info->size;
	void *tmp;
	//printf("read %d:0x%x,%x,%d\n",eg_sn,buf_size,info->size,info->loop_count);
	/*if(info->file.header.sample_rate)
	{
		size=sizeof(audio_header_info_t);
		memcpy(buf,&(info->file.header),size);
		memset(&(info->file.header),0,size);
		return size;
	}*/
	if(info->type==FUI_EVENT_SOUND){ //返回指定大小
//again:
		if(info->size>buf_size)
		{
			size=buf_size;
			memcpy(buf,info->pos,size);
			info->size-=size;
			info->pos+=size;		
		}
		else if(info->size>0)
		{
			size=info->size;
			memcpy(buf,info->pos,size);
			info->size-=size;
			info->pos+=size;
			/*if(info->loop_count<=1){
				//a_close(info->audio_engine);
				//memset(info,0,sizeof(struct _audio_decode_info));
			}
			else
			{
				info->loop_count--;
				info->size=info->pos-info->buf;
				info->pos=info->buf;
			}*/
		}else
		{
			if(info->loop_count<=1)
			{
				/*if(info->DeviceId==102) {
				info->loop_count--;
				info->size=info->pos-info->buf;
				info->pos=info->buf;
				goto again;
				}*/
				if(info->owner)
					SWF_Message(NULL, SWF_MSG_SOUND_END,&(info->owner));
				else
					info->stop_flag=1;

			}
			else
			{
				info->loop_count--;
				info->size=info->pos-info->buf;
				info->pos=info->buf;
				//if(info->file.header.format_tag==('S'<<8)+'F')
					return -55;
				//else
				//	goto again;
			}
		/*	printf("close %d\n",eg_sn);
			tmp=info->audio_engine;
			memset(info,0,sizeof(struct _audio_decode_info));			
			//printf("0x%x=0x%x\n",&(info->audio_engine),info->audio_engine);
			a_cmd(tmp, STOP,NULL);
			printf("close:0x%x\n",tmp);
			a_close(tmp);
		*/
		}
		//printf("return 0x%x\n",size);
		return size;	
	}
	else
	{
wait_data:
		//printf("test2:0x%x\n",info->size);
		if(info->size<=0)
		{
			//printf("no data\n");
			info->nodate_count++;
			if(info->nodate_count>50)
			{
				/*tmp=info->audio_engine;
				memset(info,0,sizeof(struct _audio_decode_info));
				a_cmd(tmp, STOP,NULL);
				a_close(tmp);*/
				info->stop_flag=1;
				return 0;
			}
			OSSleep(10);
			goto wait_data;
		}
		info->nodate_count=0;
		if(info->file.header.format_tag==('S'<<8)+'F') //一次返回一整祯
		{
			size=info->size;
			memcpy(buf,info->pos,size);
			info->size-=size;
			info->pos+=size;
		}else{	//返回指定大小
			if(info->size>buf_size)
			{
				size=buf_size;
				memcpy(buf,info->pos,size);
				info->size-=size;
				info->pos+=size;		
			}
			else if(info->size>0)
			{
				size=info->size;
				memcpy(buf,info->pos,size);
				info->size-=size;
				info->pos+=size;
				//a_close(info->audio_engine);
				//memset(info,0,sizeof(struct _audio_decode_info));
			}
		}
		return size;			
	}
}
int audio0_read_stream(void *opaque,void *buf,int buf_size)
{
	return audio_read_stream(opaque,buf,buf_size,0);
}
int audio1_read_stream(void *opaque,void *buf,int buf_size)
{
	return audio_read_stream(opaque,buf,buf_size,1);
}
int audio2_read_stream(void *opaque,void *buf,int buf_size)
{
	return audio_read_stream(opaque,buf,buf_size,2);
}
int audio3_read_stream(void *opaque,void *buf,int buf_size)
{
	return audio_read_stream(opaque,buf,buf_size,3);
}
static int num=0;
static int num_bak;
int fui_play_audio(SWF_AUDIO_INFO * info)
{
	//return 0;
	num_bak=num;
	num++;
	//printf("play %d,%d\n",info->id,num_bak);
#if 0
	sdram_input_t sdram_input;
	sdram_input.buf = info->audio_data;
	sdram_input.len = info->audio_size;

	void * handle = audioDecOpen(&sdram_input);
	if(handle != NULL)
	{
		if(SWF_GET_SOUND_FMT(info->flag) == SOUND_FMT_MP3)
		{
			play_param_t play_param;
			audioDecCmd(handle, SET_FILE, "mem.mp3");
			play_param.mode = NORMAL_PLAY;
			audioDecCmd(handle, PLAY,(unsigned int)&play_param);
		}
	}
#endif
	struct _audio_decode_info *decode_info=NULL;
	int i;
	int ret;
	play_param_t play_param;
	struct _pack{
		void *a;
		void *b;
	}pack;
	/*if(info->format != SOUND_FMT_ADPCM&&info->format != SOUND_FMT_MP3)
	{
		printf("audio format %d unsupported\n",info->format);
		return 0;
	}*/
	/*for(i=0;i<AUDIO_ENGINE_MAX;i++)
	{
		printf("%d,engine=0x%x\n",i,audio_decode_info[i].audio_engine);
	}*/
	//OSMutexLock(audio_mutex);
	for(i=0;i<AUDIO_ENGINE_MAX;i++)
	{
		decode_info=&(audio_decode_info[i]);
		if(decode_info->audio_engine)
		{
			if(decode_info->stop_flag==1)
			{
				printf("test1\n");
				a_cmd(decode_info->audio_engine, STOP,0);
				printf("test2\n");
				a_close(decode_info->audio_engine);				
				printf("test3\n");
				memset(decode_info,0,sizeof(struct _audio_decode_info));				
				printf("close :%d\n",i);
			}
		}
	}
	/*if(swf_audio_state)	
	{
		OSMutexUnlock(audio_mutex);
		return 0;
	}*/
	/*if(info->format==0x0055)
	{
		for(i=0;i<AUDIO_ENGINE_MAX;i++)
		{
			decode_info=&(audio_decode_info[i]);
			if(decode_info->audio_engine)
			{
				if(decode_info->file.header.format_tag==0x0055&&decode_info->DeviceId!=info->DeviceId)
				{
					printf("unsupport more than one mp3 channel,%d is playing\n",decode_info->DeviceId);
					//OSMutexUnlock(audio_mutex);
					return 0;
				}
			}
		}
	}*/
	if(info->id) //event sound,asynchronous
	{
		/*for(i=0;i<AUDIO_ENGINE_MAX;i++)
		{
			decode_info=&(audio_decode_info[i]);
			if(decode_info->DeviceId==info->id)
			{
				if(SWF_GET_SOUND_SYNC(info->flag) == SOUND_SYNC_NOMULTIPLE)
					goto exit;									
				if(SWF_GET_SOUND_SYNC(info->flag) == SOUND_SYNC_STOP)
				{
					printf("sync_stop\n");
					a_cmd(decode_info->audio_engine, STOP,0);
					a_close(decode_info->audio_engine);
					memset(decode_info,0,sizeof(struct _audio_decode_info));
				}				
			}
		}*/
		//if(info->owner==0) //sound tag
		{
			if(SWF_GET_SOUND_SYNC(info->flag) == SOUND_SYNC_NOMULTIPLE)
			{
				for(i=0;i<AUDIO_ENGINE_MAX;i++)
				{
					decode_info=&(audio_decode_info[i]);
					if(decode_info->DeviceId==info->id)
					{
						goto exit;
					}
				}
			}
			else if(SWF_GET_SOUND_SYNC(info->flag) == SOUND_SYNC_STOP)
			{
				printf("sync_stop\n");
				for(i=0;i<AUDIO_ENGINE_MAX;i++)
				{
					decode_info=&(audio_decode_info[i]);
					if(decode_info->DeviceId==info->id)
					{
						a_cmd(decode_info->audio_engine, STOP,0);
						a_close(decode_info->audio_engine);
						memset(decode_info,0,sizeof(struct _audio_decode_info));
					}
				}
				goto exit;
			}
		}
		for(i=0;i<AUDIO_ENGINE_MAX;i++)
		{
			//printf("0x%x=0x%x\n",&(decode_info->audio_engine),decode_info->audio_engine);
			decode_info=&(audio_decode_info[i]);
			if(info->owner) //sound class
			{
				if(decode_info->owner==info->owner)
				{
					goto _play;				
				}
			}
			else //sound tag
			{	
				if(decode_info->audio_engine==NULL)
				{
					decode_info->audio_engine=a_open(NULL);
					if(decode_info->audio_engine==NULL) 
					{
						printf("open audio engine err\n");
						goto exit;
					}
					else
						goto _play;
				}

			}
		}
		printf("audio_engine is full\n");	
		goto exit;
_play:
				switch(i)
				{
					case 0:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(0);
						break;
					case 1:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(1);
						break;
					case 2:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(2);
						break;
					case 3:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(3);
						break;
					default:
						printf("don't support so many audio engine\n");
						goto exit;
				}
				decode_info->file.file_iocontext.handle=decode_info;
				decode_info->file.header.sample_rate=info->sample_rate;
				decode_info->file.header.channels=info->channels;
				decode_info->file.header.format_tag=info->format;
				decode_info->owner=info->owner;
				decode_info->s=(int)info->swf_inst;
				pack.a=&(decode_info->file.file_iocontext);
				pack.b=&(decode_info->file.header);
				//printf("1\n");
				ret=a_cmd(decode_info->audio_engine,SET_FLASH,(unsigned int)&pack);
				//printf("2\n");
				if(ret)
				{
					printf("audio cmd 0x%x err\n",SET_FLASH);
					a_close(decode_info->audio_engine);
					memset(decode_info,0,sizeof(struct _audio_decode_info));
					goto exit;
				}
				play_param.mode=SEEK_PLAY;
				play_param.param=info->offset;
				decode_info->type=FUI_EVENT_SOUND;
				decode_info->pos=decode_info->buf=info->audio_data;
				decode_info->size=info->audio_size;
				decode_info->DeviceId=info->id;
				decode_info->loop_count=info->loop;
				//printf("3:%x\n",info->audio_size);
				ret=a_cmd(decode_info->audio_engine,PLAY,(unsigned int)&play_param);
				//printf("4\n");
				if(ret)
				{
					printf("audio cmd 0x%x err\n",PLAY);
					a_close(decode_info->audio_engine);
					memset(decode_info,0,sizeof(struct _audio_decode_info));
		}			
	}
	else if(/*decode_info->DeviceId*/info->DeviceId)	//stream sound,synchronous
	{
		for(i=0;i<AUDIO_ENGINE_MAX;i++)
		{
			decode_info=&(audio_decode_info[i]);
			if(decode_info->DeviceId==info->DeviceId)
			{
				goto go_on2;
			}
		}
		for(i=0;i<AUDIO_ENGINE_MAX;i++)
		{
			decode_info=&(audio_decode_info[i]);
			if(decode_info->audio_engine==NULL){ 				
				decode_info->audio_engine=a_open(NULL);	
				printf("open:0x%x\n",decode_info->audio_engine);
				if(decode_info->audio_engine==NULL) 
				{
					printf("open audio engine err\n");
					goto exit;
				}
				switch(i)
				{
					case 0:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(0);
						break;
					case 1:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(1);
						break;
					case 2:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(2);
						break;
					case 3:
						decode_info->file.file_iocontext.read=(long(*)(void *,unsigned char *, unsigned long))DEFINE_AUDIO_IO(3);
						break;
					default:
						printf("don't support so many audio engine instances\n");
						goto exit;
				}				
				//printf("test3\n");
				decode_info->file.file_iocontext.handle=decode_info;
				decode_info->file.header.sample_rate=info->sample_rate;
				decode_info->file.header.channels=info->channels;
				decode_info->file.header.format_tag=info->format;
				decode_info->s=(int)info->swf_inst;
				pack.a=&(decode_info->file.file_iocontext);
				pack.b=&(decode_info->file.header);
				ret=a_cmd(decode_info->audio_engine,SET_FLASH,(unsigned int)&pack);
				//printf("test4\n");
				if(ret)
				{
					printf("audio cmd 0x%x err\n",SET_FLASH);
					a_close(decode_info->audio_engine);
					memset(decode_info,0,sizeof(struct _audio_decode_info));
					goto exit;
				}
				play_param.mode=NORMAL_PLAY;
				play_param.param=0;
				ret=a_cmd(decode_info->audio_engine,PLAY,(unsigned int)&play_param);
				//printf("test5\n");
				if(ret)
				{
					printf("audio cmd 0x%x err\n",PLAY);
					a_close(decode_info->audio_engine);
					memset(decode_info,0,sizeof(struct _audio_decode_info));
					goto exit;
				}
				decode_info->DeviceId=info->DeviceId;
				goto go_on2;			
			}
		}
		printf("audio_engine is full\n");
		goto exit;
	go_on2:
		decode_info->type=FUI_STREAM_SOUND;
		decode_info->pos=decode_info->buf=info->audio_data;
		decode_info->size=info->audio_size;
		while(decode_info->size)
		{
			//printf("test1:0x%x\n",decode_info->size);
			OSSleep(10);
		}
	}
exit:
	//printf("play_exit:%d\n",num_bak);
	//OSMutexUnlock(audio_mutex);
	return 0;
}

int fui_stop_audio(int s)

{	
	fui_audio_engine_close(s);

	return 0;
}

int disable_swf_audio(void)
{
	//OSMutexLock(audio_mutex);
	//swf_audio_state=1;
	fui_audio_engine_close(0);
	//OSMutexUnlock(audio_mutex);
	return 0;
}
int enable_swf_audio(void)
{
	//swf_audio_state=0;
	return 0;
}

int _wifi_direct_copy_file(char *from, char *to)
{
	FILE *fp_from,*fp_to;
	char buf[256];
	int size;

	fp_from = fopen(from,"rb");
	if(fp_from == NULL){
		printf(">>>>[%s,%d]:open %s error\n",__FUNCTION__,__LINE__,from);
		return -1;
	}

	fp_to = fopen(to,"wb");
	if(fp_to == NULL){
		printf(">>>>[%s,%d]:open %s error\n",__FUNCTION__,__LINE__,to);
		fclose(fp_from);
		return -1;
	}

	while(!feof(fp_from)){
		size = fread(buf, sizeof(char), 256, fp_from);
		if(size > 0){
			fwrite(buf, sizeof(char), size, fp_to);
		}
	}
	fflush(fp_to);
	fsync(fileno(fp_to));
	fclose(fp_to);
	fclose(fp_from);
	
	return 0;
}

/********used for common file
int _wifi_direct_copy_file(char *from, char *to)
{
	FILE *fp_from,*fp_to;
	char *buf;
	int size;
	int file_size;

	printf("\n------------------copy------------- file--------------\n\n");
	fp_from = fopen(from,"rb");
	if(fp_from == NULL){
		printf(">>>>[%s,%d]:open %s error\n",__FUNCTION__,__LINE__,from);
		return -1;
	}
	
	fseek(fp_from,0L,SEEK_END);
	file_size = ftell(fp_from);
	fseek(fp_from,0L,SEEK_SET);
	
	fp_to = fopen(to,"wb");
	if(fp_to == NULL){
		printf(">>>>[%s,%d]:open %s error\n",__FUNCTION__,__LINE__,to);
		fclose(fp_from);
		return -1;
	}

	buf = malloc(file_size);
	if(buf == NULL )
		return -1;
	
//	while(!feof(fp_from)){
		size = fread(buf, sizeof(char), file_size, fp_from);
		printf("buf is-----------------------------------------------\n");
		printf("%s",buf);
		printf("file_size is %d-----------------------------------------\n",file_size = ftell(fp_from));
		printf("\tsize is %d-----------------------------------------\n",size);
		printf("last char is %d\n",buf+file_size-1);
		if(size > 0){
			fwrite(buf, sizeof(char), size, fp_to);
		}
//	}
	
	free(buf);
	
	fflush(fp_to);
	fsync(fileno(fp_to));
	fclose(fp_to);
	fclose(fp_from);
	
	return 0;
}
*******/

