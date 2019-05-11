#include <stdio.h>
#include <unistd.h>

#include <wire_heap.h>

#include "wire_osd.h"
#include "load_photo_api.h"
#include "wire_log.h"

static void *p_heap = NULL;
static void* inst = NULL;
static unsigned char *out_buf = NULL;
static unsigned long dec_buf_addr = 0;
static unsigned long out_buf_addr = 0;
static int src_width = 0;
static int src_height = 0;


#define HM_PIX_FMT_YUV420S		0x020001U
#define HM_PIX_FMT_YUV422I		0x010001U
#define P_HEAP_SIZE 8*1024*1024+128*1024
//#define DEC_BUF_SIZE 2*1024*1024//1920*1080*2+1024*128

void *getDeHandle()
{
	return inst;
}

void *getUIpHeap()
{
	return p_heap;
}
	
int wireUI_DeInit(int *width, int *height)
{
	DE_config ds_conf;
	
	osd_de_init(&inst);	
	osd_de_get_config(inst,&ds_conf,DE_CFG_ALL);
	*width = ds_conf.input.width;
	*height = ds_conf.input.height;
	ds_conf.input.enable = 0;
	osd_de_set_Config(inst,&ds_conf,DE_CFG_ALL);

	return 0;
}

int getScreenSize(int *width, int *height)
{
	if(width)
		*width = src_width;
	if(height)
		*height = src_height;

	return 0;
}

int wireUI_Init()
{
	p_heap = (void *)wire_MemoryInit(P_HEAP_SIZE);
	//WLOGI("wplayer_heap.p_heap = %p\n",wplayer_heap.p_heap);
	OSprintf("after sysbuf_heap_init\n");
	if(!p_heap){
		WLOGE("get heap failed\n");
		return -1;
	}

	wireUI_DeInit(&src_width, &src_height);
	
	int luma_size = src_width*src_height;
	int pix_fmt = HM_PIX_FMT_YUV422I;
	int buf_size = 0;
	
	if(pix_fmt == HM_PIX_FMT_YUV422I)
		buf_size = luma_size*2;
	else if(pix_fmt == HM_PIX_FMT_YUV420S)
		buf_size = luma_size*3/2;
	else
	{
		OSprintf("%s, out_pix_fmt error\n", __FUNCTION__);
		return -1;
	}
	out_buf = (unsigned char *)OSHmalloc(p_heap, buf_size, &out_buf_addr);
	if(!out_buf)
	{
		OSprintf("%s, OSHmalloc error\n", __FUNCTION__);
		if(p_heap){
			wire_MemoryRelease(p_heap);
			p_heap = NULL;
		}
		return -1;
	}
	return 0;
}

static void *heapDecMalloc(int size)
{
	if(!p_heap){
		WLOGE("pheap is NULL\n");
		return NULL;
	}
	return OSHmalloc(p_heap, size, &dec_buf_addr);
}

static void heapDecFree(void *p)
{
	if(p){
		OSHfree(p_heap, p);
		p = NULL;
	}
}

static unsigned long getBusAddress(unsigned long addr)
{
	return dec_buf_addr;
}

static int OSfread(void *ptr, int size, int nmemb, void *fp)
{
	return fread(ptr, size, nmemb, (FILE*)fp);
}

static long test_read(void *fp, unsigned char *buf, unsigned long size)
{
	FILE *file = (FILE *)fp;
	return OSfread(buf, 1, size, file);
}

static int OSfwrite(void *ptr, int size, int nmemb, void *fp)
{
	return fwrite(ptr, size, nmemb, (FILE*)fp);
}

static int OSfseek_set(void *fp, long offset)
{
	return fseek((FILE*)fp, offset, SEEK_SET);
}

static int OSfseek_cur(void *fp, long offset)
{
	return fseek((FILE*)fp, offset, SEEK_CUR);
}

static int OSfseek_end(void *fp, long offset)
{
	return fseek((FILE*)fp, offset, SEEK_END);
}

static long OSftell(void *fp)
{
	return ftell((FILE*)fp);
}

static int wireUI_SetInputPara(LP_INPUT_PARAM *lp_param, char *jpgPath, int x, int y, int width, int height)
{
	int ret;

	if(NULL == lp_param || NULL == out_buf){
		WLOGE("para error\n");
		return -1;
	}

	lp_param->handle = (void *)OSfopen((char *)jpgPath, "r");
	lp_param->lp_fread = test_read;
	lp_param->lp_fseek_cur = (long (*)(void *,long))OSfseek_cur;
	lp_param->lp_fseek_end = (long (*)(void *,long))OSfseek_end;
	lp_param->lp_fseek_set = (long (*)(void *,long))OSfseek_set;
	lp_param->lp_ftell = OSftell;	
	lp_param->lp_malloc = (void *(*)(unsigned long))heapDecMalloc;
	lp_param->lp_free = heapDecFree;
	lp_param->lp_get_bus_addr = getBusAddress;
	lp_param->lp_realloc = NULL;	
	lp_param->output_pix_fmt = LP_FMT_YCBCR_4_2_2_INTERLEAVED;
	
	lp_param->out_buf_y.buf = out_buf;
	lp_param->out_buf_uv.buf = NULL;
	lp_param->out_buf_v.buf = NULL;
	lp_param->out_buf_width = src_width;
	lp_param->out_buf_height = src_height;
	lp_param->out_pic_pos_x = x;
	lp_param->out_pic_pos_y = y;
	lp_param->out_pic_width = width;
	lp_param->out_pic_height = height;
	
	return 0;
}

int wireUI_Flip()
{
	DE_config de_config;

	osd_de_get_config(inst, &de_config, DE_CFG_ALL);

	de_config.input.default_color = 0;
	de_config.input.video_range = 1/*DE_SDTV_SRGB*/;
	de_config.input.img_uv = 0;
	de_config.input.crop_x =
	de_config.input.crop_y =
	de_config.input.crop_width =
	de_config.input.crop_height = 0;
	de_config.input.pix_fmt = DE_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	de_config.input.img = (unsigned long *)out_buf;
	de_config.input.bus_addr = out_buf_addr;
	de_config.input.width = src_width;
	de_config.input.height = src_height;
	de_config.output.display_mode = DE_DISPLAYMODE_LETTER_BOX;
	de_config.input.enable = 1;
	de_config.input.video_range = 1;
	
	osd_de_set_Config(inst, &de_config, DE_CFG_ALL/*DE_CFG_IN*/);

	return 0;
}

int wireUI_Hide()
{
	DE_config conf;
	
	if(NULL == inst)
		return -1;

	osd_de_get_config(inst, &conf, DE_CFG_ALL);
	conf.input.width = src_width;
	conf.input.height = src_height;
	conf.input.enable = 0; //DE_DEFAULTCOLOR
	osd_de_set_Config(inst, &conf, DE_CFG_ALL);
	WLOGI("outbuf_W = %d, outbuf_H = %d\n", src_width, src_height);

	return 0;
}

int wireUI_LoadPic(char *jpgPath, int pos_x, int pos_y, int pic_width, int pic_height)
{
	LP_INPUT_PARAM lp_param;
	int ret;

	if(access(jpgPath, F_OK) != 0)
	{
		WLOGE("%s not exist!!\n", jpgPath);
		return -1;
	}

	ret = wireUI_SetInputPara(&lp_param, jpgPath, (pos_x*src_width)/1920, (pos_y*src_height)/1080, (pic_width*src_width)/1920, (pic_height*src_height)/1080);
	if(ret < 0){
		WLOGE("set input para failed\n");
		return -1;
	}
	ret = load_jpeg(&lp_param);

	//wireUI_ShowPic(lp_param);

	return 0;
}

void wireUI_DeRelease()
{
	if(inst){
		osd_de_release(inst);
		inst = NULL;
	}
}

void wireUI_Release()
{
	if(out_buf){
		OSHfree(p_heap, out_buf);
		out_buf = NULL;
	}
	if(p_heap){
		wire_MemoryRelease(p_heap);
		p_heap = NULL;
	}
}

