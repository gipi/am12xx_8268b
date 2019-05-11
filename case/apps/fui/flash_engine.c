#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include "fui_common.h"
#include "swf_ext.h"
#include "display.h"
#include "swf_types.h"
#include "iconv.h"

#define DEFAULT_INTERVAL 3
#define OSD_BUF_NUM 2
#define OSD_FUI

/**
* use the whole OSD area, not partial
*/
#define OSD_FULL_EN 

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif


static int fui_fps=(100/DEFAULT_INTERVAL);

extern unsigned long swf_heap_logic_start;
extern unsigned long swf_heap_physic_start;

extern void *deinst;

typedef struct _FlashInfo 
{
	SWF_DEC *	s;
	int			x;
	int			y;
	int			width;
	int			height;
	int			frame_addr;
	int			type;
	int			interval;
	char		path[240];
	
	BLIST		roi;
	sem_t *  roi_sem;

}FLASH_INFO;

typedef struct _ROIInfo
{
	BLIST	BList;
	int		x;
	int		y;
	int		width;
	int		height;
	int		id;
}ROI_INFO;

static FLASH_INFO flashInfo;

int videoOsd_Release=0;

static int _flashengine_update_osd(ROI_INFO * roi,void *dev,unsigned long *viraddr,unsigned long bus_addr)
{
	
#ifdef USE_NEW_VIDEO_OSD
	/// if use new video osd mode,just return.
	return 0;
#endif

	enable_2d_task(1);
	
#ifdef OSD_FULL_EN
	DE_config ds_conf;

	de_get_config(dev,&ds_conf,DE_CFG_OSD1);
	
	ds_conf.osd_input1.enable = 1;
	ds_conf.osd_input1.stride = flashInfo.width;
	ds_conf.osd_input1.pix_fmt= DE_PIX_FMT_OSDBIT16MODE;
	ds_conf.osd_input1.img = viraddr;
	ds_conf.osd_input1.bus_addr = bus_addr;
	ds_conf.osd_input1.tparent_color = 0;
	ds_conf.osd_input1.idx_fmt = 0;

	ds_conf.osd_output1.alpha = 8;
	ds_conf.osd_output1.pip_x = flashInfo.x+1;
	ds_conf.osd_output1.pip_y = flashInfo.y+1;
	ds_conf.osd_output1.width = flashInfo.width;
	ds_conf.osd_output1.height = flashInfo.height;

	de_set_Config(dev,&ds_conf,DE_CFG_OSD1);

	return 0;
#else

	int dst_x = MAX(0, flashInfo.x + roi->x);
	int dst_y = MAX(0, flashInfo.y + roi->y);
	int src_x = dst_x - flashInfo.x;
	int src_y = dst_y - flashInfo.y;
	int w = MIN(roi->width - (src_x - roi->x), IMAGE_WIDTH_E  - dst_x);
	int h = MIN(roi->height- (src_y - roi->y), IMAGE_HEIGHT_E - dst_y);
	DE_config ds_conf;
	if(roi->id == 0){
		de_get_config(dev,&ds_conf,DE_CFG_OSD1);
		ds_conf.osd_input1.enable = 1;
		ds_conf.osd_input1.stride = flashInfo.width;
		ds_conf.osd_input1.pix_fmt= DE_PIX_FMT_OSDBIT16MODE;
		ds_conf.osd_input1.img = viraddr+ (src_y * flashInfo.width + src_x)*2;
		ds_conf.osd_input1.bus_addr = bus_addr+  (src_y * flashInfo.width + src_x)*2;
		ds_conf.osd_input1.tparent_color = 0;
		ds_conf.osd_input1.idx_fmt = 0;

		ds_conf.osd_output1.alpha = 8;
		ds_conf.osd_output1.pip_x = dst_x+1;
		ds_conf.osd_output1.pip_y = dst_y+1;
		ds_conf.osd_output1.width = w;
		ds_conf.osd_output1.height = h;
		de_set_Config(dev,&ds_conf,DE_CFG_OSD1);

		return 0;
	}
	else if(roi->id == 1){
		de_get_config(dev,&ds_conf,DE_CFG_OSD2);
		
		ds_conf.osd_input2.enable = 1;
		ds_conf.osd_input2.stride = flashInfo.width;
		ds_conf.osd_input2.pix_fmt= DE_PIX_FMT_OSDBIT16MODE;
		ds_conf.osd_input2.img = viraddr+  (src_y * flashInfo.width + src_x)*2;
		ds_conf.osd_input2.bus_addr = bus_addr+  (src_y * flashInfo.width + src_x)*2;
		ds_conf.osd_input2.tparent_color = 0;
		ds_conf.osd_input2.idx_fmt = 0;

		ds_conf.osd_output2.alpha = 8;
		ds_conf.osd_output2.pip_x = dst_x+1;
		ds_conf.osd_output2.pip_y = dst_y+1;
		ds_conf.osd_output2.width = w;
		ds_conf.osd_output2.height = h;

		de_set_Config(dev,&ds_conf,DE_CFG_OSD2);

		return 0;
	}

	return -1;
#endif
	
}

static int _flashengine_disable_osd(int osd_no,void *dev)
{
	DE_config ds_conf;

#ifdef USE_NEW_VIDEO_OSD
		/// if use new video osd mode,just return.
		return 0;
#endif

	
	enable_2d_task(0);
	if(osd_no==0){
		de_get_config(dev,&ds_conf,DE_CFG_OSD1);
		ds_conf.osd_input1.enable = 0;
		de_set_Config(dev,&ds_conf,DE_CFG_OSD1);

		return 0;
	}
	else if(osd_no == 1){
		de_get_config(dev,&ds_conf,DE_CFG_OSD2);
		ds_conf.osd_input2.enable = 0;
		de_set_Config(dev,&ds_conf,DE_CFG_OSD2);

		return 0;
	}

	return -1;
	
}

/**
* semaphore operations.
*/
static int _flashengine_sem_init(sem_t *psem,int count)
{
	return sem_init(psem,0,count);
}

static int _flashengine_sem_pend(sem_t *psem)
{
	int err;

_FE_PEND_REWAIT:
	err = sem_wait(psem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto _FE_PEND_REWAIT;	
		}
		else{
			printf("_flashengine_sem_pend: errno:%d\n",errsv);
			return -1;
		}
	}
	return err;
}

static int _flashengine_sem_post(sem_t *psem)
{
	int err;
	
	err = sem_post(psem);

	return err;
}

static int _flashengine_sem_destroy(sem_t *psem)
{
	int err;
	
	err = sem_destroy(psem);

	return err;
}

static void _init_flash_info()
{
	flashInfo.s = NULL;
	flashInfo.x = 0;
	flashInfo.y = 0;
	flashInfo.width = 0;
	flashInfo.height = 0;
	flashInfo.path[0] = 0;
	flashInfo.frame_addr = 0;
	flashInfo.type = 0;
	flashInfo.interval = DEFAULT_INTERVAL;
	BLIST_INIT(&flashInfo.roi);
	flashInfo.roi_sem = (sem_t *)malloc(sizeof(sem_t));
	if(flashInfo.roi_sem){
		//sem_init(flashInfo.roi_sem,0,1);
		_flashengine_sem_init(flashInfo.roi_sem,1);
	}
}

static void _get_full_path(void * handle, char * path, char * fullpath)
{
	if(path[0] != '/'){
		/** relative path */
		char * p, * q;
		
		strcpy(fullpath, SWF_GetFilePath(handle));
		
		for(p = fullpath, q = NULL; *p != 0; p++){
			if(*p == '/'){
				q = p + 1;
			}
		}
		
		if(q){
			strcpy(q, path);
		}
		else{
			strcpy(fullpath, path);
		}
	}
	else{
		strcpy(fullpath, path);
	}
}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


void fui_frame_ready(unsigned char * frame)
{
	//DO NOTHING...
}


int fui_get_fps()
{
	return fui_fps;
}

int fui_set_fps(int fps)
{
	// set fps between 2~33
	fui_fps=MAX(MIN((100/DEFAULT_INTERVAL), fps),2);

	return 0;
}

int fui_interval()
{
	if(flashInfo.s)
	{
		return flashInfo.interval;
	}
	else
	{
		return 100/fui_fps;
	}
}



void fui_show(int frame_addr, void * dev)
{
	DE_config ds_conf;
	BLIST* blist;
	
	if(frame_addr)
	{
		int err;
		
		if(flashInfo.s && flashInfo.type){
			// TO BE ADDED
	#ifdef OSD_FUI
			ROI_INFO * roi = NULL;
	
			_flashengine_sem_pend(flashInfo.roi_sem);
			
			if(flashInfo.frame_addr)
			{
				flashInfo.frame_addr = frame_addr;
			#ifdef OSD_FULL_EN
				_flashengine_update_osd(NULL,dev,(unsigned long *)frame_addr,fui_get_bus_address((unsigned long)frame_addr));
			#else
				ITERATE_OVER_LIST(&flashInfo.roi, blist)
				{
					roi = (ROI_INFO *)blist;
					_flashengine_update_osd(roi,dev,(unsigned long *)frame_addr,fui_get_bus_address((unsigned long)frame_addr));
				}
			#endif
			}
			else
			{
				flashInfo.frame_addr = frame_addr;
			}
			
			_flashengine_sem_post(flashInfo.roi_sem);
	#endif
		}
		else{
			if(dev){
				de_get_config(dev,&ds_conf,DE_CFG_ALL);
			#if 0
				printf("pip_x=%d\n",ds_conf.output.pip_x);
				printf("pip_y=%d\n",ds_conf.output.pip_y);
				printf("display_mode=%d\n",ds_conf.output.display_mode);
				printf("output_mode=%d\n",ds_conf.output.output_mode);
				printf("pip_width=%d\n",ds_conf.output.pip_width);
				printf("pip_height=%d\n",ds_conf.output.pip_height);
				printf("dar_width=%d\n",ds_conf.output.dar_width);
				printf("dar_height=%d\n",ds_conf.output.dar_height);
			#endif
				ds_conf.input.enable = 1;
				ds_conf.input.img = (unsigned long*)frame_addr;
				ds_conf.input.bus_addr = fui_get_bus_address((unsigned long)frame_addr);
				de_set_Config(dev,&ds_conf,DE_CFG_IN);
			}
		}
	}
}


static int flashengine_add_roi(void * handle)
{
	ROI_INFO * roi;
	int roi_id = -1;
	int err;
	int i, n = 0;
	BLIST* blist;


	_flashengine_sem_pend(flashInfo.roi_sem);

#ifdef OSD_FULL_EN
	/**
	* ignore the parameters.
	*/
	SWFEXT_FUNC_BEGIN(handle);

	Swfext_GetNumber();
	Swfext_GetNumber();
	Swfext_GetNumber();
	Swfext_GetNumber();

	Swfext_PutNumber(0);
	
#else
	ITERATE_OVER_LIST(&flashInfo.roi, blist)
	{
		roi = (ROI_INFO *)blist;
		n |= (1 << roi->id);
	}
	
	for(i = 0; i < OSD_BUF_NUM; i++)
	{
		if((n & 1) == 0)
		{
			roi_id = i;
			goto __found;
		}
		n >>= 1;
	}
	goto __end;


__found:
	roi = (ROI_INFO*)SWF_StaticMalloc(sizeof(ROI_INFO));
	SWFEXT_FUNC_BEGIN(handle);
	roi->x      = Swfext_GetNumber();
	roi->y      = Swfext_GetNumber();
	roi->width  = Swfext_GetNumber();
	roi->height = Swfext_GetNumber();
	roi->id     = roi_id;
	BListAdd(&flashInfo.roi, &roi->BList);
	printf("enable osd %d\n", roi->id);
	printf("roi->x = %d\n", roi->x);
	printf("roi->y = %d\n", roi->y);
	printf("roi->width = %d\n", roi->width);
	printf("roi->height = %d\n", roi->height);
	
__end:
	Swfext_PutNumber(roi_id);
#endif
	videoOsd_Release=0;
	_flashengine_sem_post(flashInfo.roi_sem);
	
	SWFEXT_FUNC_END();
}

static int flashengine_set_roi(void * handle)
{
	ROI_INFO * roi;
	int id;
	int err;
	BLIST* blist;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	id = Swfext_GetNumber();
	
	_flashengine_sem_pend(flashInfo.roi_sem);

#ifdef OSD_FULL_EN
	Swfext_GetNumber();
	Swfext_GetNumber();
	Swfext_GetNumber();
	Swfext_GetNumber();
	
	Swfext_PutNumber(1);
#else
	ITERATE_OVER_LIST(&flashInfo.roi, blist)
	{
		roi = (ROI_INFO *)blist;
		if(roi->id == id)
		{
			roi->x      = Swfext_GetNumber();
			roi->y      = Swfext_GetNumber();
			roi->width  = Swfext_GetNumber();
			roi->height = Swfext_GetNumber();
			Swfext_PutNumber(1);
			goto __end;
		}
	}
	Swfext_PutNumber(0);
#endif

	
__end:
	_flashengine_sem_post(flashInfo.roi_sem);
	SWFEXT_FUNC_END();
}


static int flashengine_remove_roi(void * handle)
{
	ROI_INFO * roi;
	int id;
	int err;
	BLIST* blist;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	id = Swfext_GetNumber();
	_flashengine_sem_pend(flashInfo.roi_sem);

#ifdef OSD_FULL_EN
	Swfext_PutNumber(1);
#else
	ITERATE_OVER_LIST(&flashInfo.roi, blist)
	{
		roi = (ROI_INFO *)blist;
		
		if(roi->id == id)
		{
			BListRemove(&roi->BList);
			_flashengine_disable_osd(roi->id,deinst);
			SWF_Free(roi);
			Swfext_PutNumber(1);
			goto __end;
		}
	}
	Swfext_PutNumber(0);
#endif
	
__end:
	_flashengine_sem_post(flashInfo.roi_sem);
	
	videoOsd_Release=1;
	SWFEXT_FUNC_END();
}

static int get_OSD_exit_flag(void * handle)
{

	
	SWFEXT_FUNC_BEGIN(handle);
	
	
	Swfext_PutNumber(videoOsd_Release);
	SWFEXT_FUNC_END();
}

#ifdef USE_NEW_VIDEO_OSD
static int new_video_osd_mode = 0;
int flashengine_is_new_video_osd_mode()
{
	return new_video_osd_mode;
}

int flashengine_set_new_video_osd_mode(int mode)
{
	new_video_osd_mode = mode;

	return 0;
}
#endif



/** flashengine_play(path, trans, x, y, width, height, fps) */
static int flashengine_play(void * handle)
{
#ifndef  FUI_TIME_DEBUG
	unsigned int flag = SWF_FULLSCREEN_FLAG | SWF_STANDALONE |SWF_NON_RELOAD;
#else
	unsigned int flag = SWF_LBOX_FLAG | SWF_STANDALONE |SWF_NON_RELOAD;
#endif
	int narg;
	SWF_RECT win;
	char * path;
	
	
	SWFEXT_FUNC_BEGIN(handle);
	narg = Swfext_GetParamNum();
	path = Swfext_GetString();
	if(!path)
	{
		goto __end;
	}

	if(path)
	{
		if(flashInfo.s)
		{
			SWF_RemoveInst(flashInfo.s);
		}
		
		//get parameters
		_get_full_path(handle, path, flashInfo.path);
		narg--;
		
		flashInfo.type = Swfext_GetNumber();
		if(flashInfo.type)
		{
#ifdef OSD_FUI
			flag |= SWF_FORMAT_RGB565;
#else
			flag |= SWF_FORMAT_RGB888;
#endif
		}
		narg--;
		
		if(narg > 0)
		{
			flashInfo.x      = Swfext_GetNumber();
			flashInfo.y      = Swfext_GetNumber();
			flashInfo.width  = Swfext_GetNumber();
			flashInfo.height = Swfext_GetNumber();
			
			narg -= 4;
			if(narg > 0)
			{
				int fps = Swfext_GetNumber();
				if(fps)
				{
					flashInfo.interval = MAX(3, 100 / fps);
				}
			}
			else
			{
				flashInfo.interval = DEFAULT_INTERVAL;
			}
		}
		else
		{
			flashInfo.x = 0;
			flashInfo.y = 0;
			flashInfo.width  = 800;
#ifndef FUI_TIME_DEBUG
			flashInfo.height = 600;
#else
			flashInfo.height = 480;
#endif
			flashInfo.interval = DEFAULT_INTERVAL;
		}
		
#ifdef USE_NEW_VIDEO_OSD
		///new video osd mode
		flashInfo.width = 32;
		flashInfo.height = 32;

		// replace the osd with the new osd swf file.
		/**
		{
			int k;
			int len;
			char *ptr=NULL;

			len = strlen(flashInfo.path);
			for(k=len-1;k>0;k--){
				if(flashInfo.path[k] == '/'){
					ptr = &flashInfo.path[k+1];
					break;
				}
			}

			if(ptr){
				strcpy(ptr,"videoOSDNew.swf");
			}
			else{
				strcpy(flashInfo.path,"videoOSDNew.swf");
			}
		}
		*/
		
		flashengine_set_new_video_osd_mode(1);
#endif

		printf("flashInfo.x = %d\n",flashInfo.x);
		printf("flashInfo.y = %d\n",flashInfo.y);
		printf("flashInfo.width = %d\n",flashInfo.width);
		printf("flashInfo.height = %d\n",flashInfo.height);
		printf("flashInfo.path = %d\n",flashInfo.path);
		//set window param
		win.Xmin = 0;
		win.Ymin = 0;
		win.Xmax = flashInfo.width  - 1;
		win.Ymax = flashInfo.height - 1;

		//insert instance
		flashInfo.s = SWF_AddInst(flashInfo.path, &win, flag, NULL);
		flashInfo.frame_addr = 0;

		if(flashInfo.type)
		{
			printf("load osd movieclip %s\n", flashInfo.path);
		}

		if(flashInfo.s)
		{
			Swfext_PutNumber(1);
			goto __end;
		}
	}

	Swfext_PutNumber(0);

__end:
	
	SWFEXT_FUNC_END();	
}

static int flashengine_stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	if(flashInfo.s)
	{
		ROI_INFO * roi = NULL;
		int err;
		
		_flashengine_sem_pend(flashInfo.roi_sem);
		
		SWF_RemoveInst(flashInfo.s);
		if(Swfext_GetParamType() != SWFDEC_AS_TYPE_BOOLEAN || Swfext_GetNumber() != 0)
		{
			SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		}
		flashInfo.s = NULL;
		flashInfo.type = 0;
		flashInfo.interval = DEFAULT_INTERVAL;
		flashInfo.frame_addr = 0;
		
#ifdef OSD_FULL_EN
	#ifdef OSD_FUI
		_flashengine_disable_osd(0,deinst);
	#endif
#else
		while( (roi = (ROI_INFO*)BListRemoveItemFromHead(&flashInfo.roi))!=NULL)
		{
	#ifdef OSD_FUI
			_flashengine_disable_osd(roi->id,deinst);
	#endif
			SWF_Free(roi);
		}
#endif	
		_flashengine_sem_post(flashInfo.roi_sem);
	}

#ifdef USE_NEW_VIDEO_OSD
	flashengine_set_new_video_osd_mode(0);
#endif

	SWFEXT_FUNC_END();	
}

static int flashengine_pause(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	SWFEXT_FUNC_END();	
}

static int flashengine_resume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	SWFEXT_FUNC_END();	
}

void flashengine_exception_release()
{
	int err;
	
	if(flashInfo.s)
	{
		ROI_INFO * roi = NULL;

		if(flashInfo.roi_sem){
			printf("get fe sem\n");
			_flashengine_sem_pend(flashInfo.roi_sem);
			printf("get fe sem ok\n");
			_flashengine_sem_destroy(flashInfo.roi_sem);
			free(flashInfo.roi_sem);
			flashInfo.roi_sem = NULL;
		}
		
		SWF_RemoveInst(flashInfo.s);
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		flashInfo.s = NULL;
		flashInfo.type = 0;
		flashInfo.interval = DEFAULT_INTERVAL;
		flashInfo.frame_addr = 0;
		
#ifdef OSD_FULL_EN
		_flashengine_disable_osd(0,deinst);
#else

		while((roi = (ROI_INFO*)BListRemoveItemFromHead(&flashInfo.roi))!=NULL)
		{
		#ifdef OSD_FUI
			_flashengine_disable_osd(roi->id,deinst);
		#endif
			SWF_Free(roi);
		}
#endif
	}
}

unsigned int autoFPSEn=0;

static int flashengine_auto_fps_enable(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	autoFPSEn=1;
	
	SWFEXT_FUNC_END();	
}

static int flashengine_auto_fps_disable(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	autoFPSEn=0;
	fui_set_fps(30);
	
	SWFEXT_FUNC_END();	
}

/*
* rate range is [2,30]
*/
static int flashengine_set_frame_rate(void * handle)
{
	int rate;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	rate = Swfext_GetNumber();
	fui_set_fps(rate);
	
	SWFEXT_FUNC_END();	
}


static int swf_put_ime_message(void * handle)
{
	char *info;
	int msg;
	char *newInfo;
	int len;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	msg = Swfext_GetNumber();
	msg |= SWF_MSG_IME;

	info = Swfext_GetString();
	len = strlen(info);
	if(len >0 && len<16){
		newInfo = SWF_Malloc(len);
		strcpy(newInfo,info);
		SWF_Message(NULL, msg, newInfo);
	}
	
	SWFEXT_FUNC_END();
}

static int swf_GetImeMessage(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(GetImeMessage());
	SWFEXT_FUNC_END();
}

static int swf_SetImeSwfName(void * handle)
{
	char *filename;
	SWFEXT_FUNC_BEGIN(handle);
	filename = Swfext_GetString();
	Swfext_PutNumber(SetImeSwfName(filename));
	SWFEXT_FUNC_END();
}

static int swf_SetImeState(void * handle)
{
	int state;
	int xmin, xmax, ymin, ymax;
	SWF_RECT ime_area;
	SWFEXT_FUNC_BEGIN(handle);
	state = Swfext_GetNumber();
	ime_area.Xmin = Swfext_GetNumber();
	ime_area.Ymin= Swfext_GetNumber();
	ime_area.Xmax = Swfext_GetNumber();
	ime_area.Ymax = Swfext_GetNumber();
	Swfext_PutNumber(SetImeState(state, &ime_area));
	SWFEXT_FUNC_END();
}

static int swf_SetImeKeyRange(void * handle)
{
	int keymin, keymax;
	SWFEXT_FUNC_BEGIN(handle);
	keymin = Swfext_GetNumber();
	keymax= Swfext_GetNumber();
	Swfext_PutNumber(SetImeKeyRange(keymin, keymax));
	SWFEXT_FUNC_END();
}

static int swf_SendMessageFromIME(void * handle)
{
	int key;
	SWFEXT_FUNC_BEGIN(handle);
	key = Swfext_GetNumber();
	SWF_Message(NULL, SWF_MSG_KEYSPECIAL|key, NULL);
	Swfext_PutNumber(1);
	SWFEXT_FUNC_END();
}

static int swf_gui_set_stage_bkcolor(void * handle)
{
	void *target;
	unsigned int color;
	
	SWFEXT_FUNC_BEGIN(handle);

	color = (unsigned int)Swfext_GetNumber();
	SWF_GUISetStageBkColor(color);

	SWFEXT_FUNC_END();
}

static int swf_render_current_frame(void * handle)
{
	SWFEXT_FUNC_BEGIN( handle );
	
	FUI_Render_CurrFrame();
	
	SWFEXT_FUNC_END();
}


#if 0


#include "rb_gui.h"

HVDC vdec=NULL;

static int swf_gui_create_dev(void * handle)
{
	int w,h;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();

	printf("w=%d,h=%d\n",w,h);

	vdec = rbCreateVDC(w,h);
	if(vdec == NULL){
		printf("create vdc error\n");
	}
	
	SWFEXT_FUNC_END();
}

static int swf_gui_delete_dev(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(vdec != NULL){
		rbDeleteVDC(vdec);
	}
	
	SWFEXT_FUNC_END();
}

static int swf_gui_attach_dev(void * handle)
{
	void *target;
	
	SWFEXT_FUNC_BEGIN(handle);

	target   = Swfext_GetObject();
	
	if(vdec != NULL){
		rbAttachVdc(target,vdec->width ,vdec->height ,vdec);
	}
	
	SWFEXT_FUNC_END();
}

static int swf_gui_dettach_dev(void * handle)
{
	void *target;
	
	SWFEXT_FUNC_BEGIN(handle);

	target   = Swfext_GetObject();
	
	if(vdec != NULL){
		rbDettachVdc(target);
	}
	
	SWFEXT_FUNC_END();
}

static int swf_gui_update_next_frame(void * handle)
{
	RECT rect;
	rbCOLOR color;
	char *ptr;
	rbCOLOR bordercolor;
	static int count=0;
	

	ptr = SWF_Malloc(128);
	strcpy(ptr,"test for gui");
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(vdec != NULL){
		int i,j;
		int cur,to;

		///printf("total mem==0x%x\n",test_get_total_mem());
		///printf("used mem==0x%x\n",test_get_used_mem());
		
		///cur = test_get_opened_file_nr();
		///to = test_get_max_file_nr();
		///printf("max file nr==%d\n",to);
		///printf("open file nr==%d\n",cur);

		
		/// draw rect
		printf(">>>> begin draw\n");

		rect.left =0;
		rect.right = 300;
		rect.top = 0;
		rect.bottom =300;

		if(count==0){
			color.alpha = 0xff;
			color.red = 0xff;
			color.blue = 0xff;
			color.green = 0xff;
			printf("draw white rect\n");
		}
		else{
			color.alpha = 0xff;
			color.red = 0;
			color.blue = 0;
			color.green = 0;
			printf("draw black rect\n");
		}
		
		rbFillRect(vdec,&rect,color);
		
		/// draw line
		
		color.alpha = 0xff;
		color.red = 0xff;
		color.blue = 0;
		color.green = 0;
		printf("draw line\n");
		if(count==0){
			rbDrawLine(vdec,10,10,200,200,color);
		}
		else{
			rbDrawLine(vdec,50,50,200,200,color);
		}

		

		/// draw rect with border
		bordercolor.alpha = 0xff;
		bordercolor.red = 0xff;
		bordercolor.blue = 0x5;
		bordercolor.green = 100;

		color.alpha = 0xff;
		color.red = 100;
		color.blue = 100;
		color.green = 100;

		if(count==0){
			rect.left =10;
			rect.right = 100;
			rect.top = 160;
			rect.bottom =200;
		}
		else{
			rect.left =150;
			rect.right = 250;
			rect.top = 160;
			rect.bottom =200;
		}
		printf("draw border rect\n");
		rbDrawRectangle(vdec, &rect,bordercolor,color);

		/// draw ellipse
		bordercolor.alpha = 0xff;
		bordercolor.red = 0xff;
		bordercolor.blue = 0x5;
		bordercolor.green = 100;

		color.alpha = 0xff;
		color.red = 100;
		color.blue = 100;
		color.green = 100;
		if(count==0){
			rect.left =100;
			rect.right = 260;
			rect.top = 100;
			rect.bottom =160;
		}
		else{
			rect.left =10;
			rect.right = 150;
			rect.top = 100;
			rect.bottom =160;
		}
		rbDrawEllipse(vdec,&rect,bordercolor,color);


		/// draw text
		color.alpha = 0xff;
		color.red = 0;
		color.blue = 0;
		color.green = 0xff;
		printf("draw text\n");
		rbSetTextColor(vdec,color);
		rbSetFontSize(vdec,32);

		if(count==0){
			rbTextOut(vdec,0,250,ptr,strlen(ptr));
		}
		else{
			rbTextOut(vdec,0,200,ptr,strlen(ptr));
		}

#if 0
		/// draw pixel
		printf("draw pixel\n");
		printf(":::::dr=0x%x,dg=0x%x,db=0x%x,dalpha=0x%x\n",color.red,color.green,color.blue,color.alpha);
		for(i=0;i<10;i++){
			if(count==0){
				rbSetPixel(vdec,100+i,2,color);
			}
			else{
				rbSetPixel(vdec,100+i,6,color);
			}
		}

		for(i=0;i<10;i++){
			rbCOLOR color2;
			if(count==0){
				rbGetPixel(vdec,100+i,2,&color2);
			}
			else{
				rbGetPixel(vdec,100+i,6,&color2);
			}
			printf("r=0x%x,g=0x%x,b=0x%x,alpha=0x%x\n",color2.red,color2.green,color2.blue,color2.alpha);
		}
#endif

		/// invert rect
		rect.left =110;
		rect.right = 210;
		rect.top = 16;
		rect.bottom =80;
		printf("invert rect\n");
		rbInvertRect(vdec,&rect);

		if(count==0){
			count =1;
		}
		else{
			count = 0;
		}

#if 0
		{
			rbIMAGE testimg;
			FILE *fp;
			long filelen;
			
			testimg.width = 1024;
			testimg.height = 600;
			testimg.type = 5;

			fp = fopen("/mnt/udisk/1.raw","r");
			if(fp){
				///printf("====open file ok\n");
				fseek(fp, 0, SEEK_END);
				filelen = ftell(fp);
				printf("====file len:%d\n",filelen);
				testimg.data = (char *)SWF_Malloc(filelen);
				fseek(fp, 0, SEEK_SET);
				if(testimg.data){
					long err;
					
					err = fread(testimg.data, 1, filelen, fp);
					///printf("====file read:%d\n",err);
					testimg.datasize = filelen;
					
					rbDrawImage(vdec,&testimg,0,0);
					SWF_Free(testimg.data);
				}
				fclose(fp);
			}
		}
#endif

		printf(">>>> end draw\n\n\n");
	}

	SWF_Free(ptr);
	
	SWFEXT_FUNC_END();
}


static int swf_gui_show_dev(void * handle)
{
	void *target;
	
	SWFEXT_FUNC_BEGIN(handle);

	target = Swfext_GetObject();

	if(target){
		SWF_GUIInvalidateObject(target);
	}
		
	SWFEXT_FUNC_END();
}

#endif





int swfext_flashengine_register(void)
{
	_init_flash_info();
	
	SWFEXT_REGISTER("flash_AddROI", flashengine_add_roi);
	SWFEXT_REGISTER("flash_SetROI", flashengine_set_roi);
	SWFEXT_REGISTER("flash_RemoveROI", flashengine_remove_roi);
	SWFEXT_REGISTER("flash_Play", flashengine_play);
	SWFEXT_REGISTER("flash_Stop", flashengine_stop);
	SWFEXT_REGISTER("flash_Pause", flashengine_pause);
	SWFEXT_REGISTER("flash_Resume", flashengine_resume);
	
	SWFEXT_REGISTER("flash_getexitflag", get_OSD_exit_flag);
	SWFEXT_REGISTER("flash_AutoFPSEn", flashengine_auto_fps_enable);
	SWFEXT_REGISTER("flash_AutoFPSDisable", flashengine_auto_fps_disable);
	SWFEXT_REGISTER("flash_SetFrameRate", flashengine_set_frame_rate);
	SWFEXT_REGISTER("flash_SwfRender", swf_render_current_frame);
	SWFEXT_REGISTER("flash_putIMEMsg", swf_put_ime_message);
	SWFEXT_REGISTER("flash_guiSetStageBkColor", swf_gui_set_stage_bkcolor);

	SWFEXT_REGISTER("flash_GetImeMessage", swf_GetImeMessage);
	SWFEXT_REGISTER("flash_SetImeSwfName", swf_SetImeSwfName);
	SWFEXT_REGISTER("flash_SetImeState", swf_SetImeState);
	SWFEXT_REGISTER("flash_SetImeKeyRange", swf_SetImeKeyRange);
	SWFEXT_REGISTER("flash_SendMessageFromIME", swf_SendMessageFromIME);
	

	#if 0
	//the following function is not used now
	SWFEXT_REGISTER("flash_guiCreateDev", swf_gui_create_dev);
	SWFEXT_REGISTER("flash_guiDeleteDev", swf_gui_delete_dev);
	SWFEXT_REGISTER("flash_guiAttachDev", swf_gui_attach_dev);
	SWFEXT_REGISTER("flash_guiDettachDev", swf_gui_dettach_dev);
	SWFEXT_REGISTER("flash_guiUpdateNextFrame", swf_gui_update_next_frame);
	SWFEXT_REGISTER("flash_guiShowDev", swf_gui_show_dev);
	
	#endif
	
	return 0;
}


/**
* The following section mainly used for display thread control.
*/

#ifdef INDEPENDENT_DISPLAY_EN

#define DISP_WORK_FIFO_LENGTH 16

struct disp_workflow 
{
	sem_t  *inq, *outq;
	int rp, wp;
	int fifo_number;
	int buffer[DISP_WORK_FIFO_LENGTH];
};

struct disp_workflow disp_wf;

static pthread_t disp_threadid;
pthread_mutex_t fifo_mutex;

static void dispwork_sem_pend(sem_t * sem)
{
	int err;

__PEND_REWAIT:
	err = sem_wait(sem);
	if(err == -1){
		int errsv = errno;
		if(errsv == EINTR){
			//Restart if interrupted by handler
			goto __PEND_REWAIT;	
		}
		else{
			printf("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}

	return;
	
}
static void dispwork_sem_post(sem_t * sem)
{
	int err;
	err = sem_post(sem);
}

static int dispwork_sem_getvalue(sem_t * sem)
{
	int err;
	int value=0;
	
	err = sem_getvalue(sem,&value);
	if(err == 0 ){
		return value;
	}
	else{
		return -1;
	}
}


/**
* Tell the display thead that address should be switched.
*/
void put_display_task(int address)
{
	dispwork_sem_pend(disp_wf.outq);
	
	disp_wf.buffer[disp_wf.wp] = address;
	disp_wf.wp++;
	if(disp_wf.wp >= DISP_WORK_FIFO_LENGTH){ 
		disp_wf.wp = 0;
	}

	///pthread_mutex_lock(&fifo_mutex);
	///disp_wf.fifo_number++;
	///pthread_mutex_unlock(&fifo_mutex);
	
	dispwork_sem_post(disp_wf.inq);
	
	return;
}

/**
* Get the current display address.
*/
int get_current_display_addr(int flag)
{
	int addr;
	
	if(flashInfo.s && flashInfo.type){
		/**
		* in video mode.
		*/
		_flashengine_sem_pend(flashInfo.roi_sem);
		addr = flashInfo.frame_addr;
		_flashengine_sem_post(flashInfo.roi_sem);
		
		return addr;
	}
	else{
		DE_config ds_conf;
		
		de_get_config(deinst,&ds_conf,DE_CFG_IN);

		addr = (int)ds_conf.input.bus_addr;
		
		return fui_get_virtual_address(addr);
	}
	
	return 0;
}

#define DISP_SKIP_FRAME_NR 2

void *disp_thread_workflow(void * arg)
{
	int address;
	int nr;
	int i;
	
	for(;;) 
	{		
		nr = dispwork_sem_getvalue(disp_wf.inq);
		if(nr > DISP_SKIP_FRAME_NR){
			for(i=0;i<DISP_SKIP_FRAME_NR;i++){
				dispwork_sem_pend(disp_wf.inq);
				address = disp_wf.buffer[disp_wf.rp];
				disp_wf.rp++;
				if(disp_wf.rp >= DISP_WORK_FIFO_LENGTH){ 
					disp_wf.rp = 0;
				}

				if(i<(DISP_SKIP_FRAME_NR-1)){
					dispwork_sem_post(disp_wf.outq);
				}

				///pthread_mutex_lock(&fifo_mutex);
				///disp_wf.fifo_number--;
				///pthread_mutex_unlock(&fifo_mutex);
			}
		}
		else{
			dispwork_sem_pend(disp_wf.inq);
			address = disp_wf.buffer[disp_wf.rp];
			disp_wf.rp++;
			if(disp_wf.rp >= DISP_WORK_FIFO_LENGTH){ 
				disp_wf.rp = 0;
			}
			///pthread_mutex_lock(&fifo_mutex);
			///disp_wf.fifo_number--;
			///pthread_mutex_unlock(&fifo_mutex);
		}

		/**
		* change the de address.
		*/
		if(SWF_GetState(SWF_GetActiveInst()) & SWF_STATE_ACTIVE){
			fui_show(address, deinst);
		}else{
			printf("###when swf sleep set de address!\n");
		}
		
		dispwork_sem_post(disp_wf.outq);
	}
	
	return NULL;
}


void open_display_workflow(void)
{
	int i;
	
	disp_wf.rp = disp_wf.wp = 0;

	disp_wf.inq  = (sem_t *)malloc(sizeof(sem_t));
	sem_init(disp_wf.inq, 0, 0);

	disp_wf.outq = (sem_t *)malloc(sizeof(sem_t));
	sem_init(disp_wf.outq, 0, DISP_WORK_FIFO_LENGTH - 1);

	for(i=0;i<DISP_WORK_FIFO_LENGTH;i++){
		disp_wf.buffer[i] = 0;
	}

	///disp_wf.fifo_number = 0;
	///pthread_mutex_init(&fifo_mutex,NULL);

	pthread_create(&disp_threadid,NULL,disp_thread_workflow,NULL);
	
}

void close_disp_workflow()
{

	int err;
	
	/** send request to cancel 2d thread */
	err=pthread_cancel(disp_threadid);
	
	/** wait thread until exit */
	err=pthread_join(disp_threadid, NULL);

	sem_destroy(disp_wf.inq);
	free(disp_wf.inq);
	disp_wf.inq = NULL;
	
	sem_destroy(disp_wf.outq);
	free(disp_wf.outq);
	disp_wf.outq = NULL;

	///pthread_mutex_destroy(&fifo_mutex);
	
}

#endif /** INDEPENDENT_DISPLAY_EN */

