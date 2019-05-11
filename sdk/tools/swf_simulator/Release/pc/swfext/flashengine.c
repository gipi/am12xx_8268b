#include "fui_common.h"
#include "swfext.h"
#include "string.h"

#ifdef MIPS_VERSION
#include "lcm_for_gui.h"
#endif

#define DEFAULT_INTERVAL 3

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
	
	int			pause_flag;
	INT8U *		pause_src;
	INT8U *		pause_dst;
	FUI_param_t pause_param;
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

static int get_next_roi_id()
{
	static roi_seed = 0;
	if(++roi_seed > 0xFFFFF)
	{
		roi_seed = 1;
	}
	return roi_seed;
}

static void init_flash_info()
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
}

static void get_full_path(void * handle, char * path, char * fullpath)
{
	if(path[1] != ':')
	{
		//relative path
		char * p, * q;
		strcpy(fullpath, SWF_GetFilePath(handle));
		for(p = fullpath, q = NULL; *p != 0; p++)
		{
			if(*p == '\\')
			{
				q = p + 1;
			}
		}
		if(q)
		{
			strcpy(q, path);
		}
		else
		{
			strcpy(fullpath, path);
		}
	}
	else
	{
		strcpy(fullpath, path);
	}
}

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

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

static void flash_copy(int dst, int src, int dstride, int sstride, int w, int h)
{
	FILLINFO info;

	info.input_type    = 1;
	info.input_addr    = src;
	info.input_start_x = 0;
	info.input_start_y = 0;
	info.input_stride  = sstride;
	info.input_repeat  = 0;
	info.input_need_blend = 1;
	info.input_need_interpolate = 0;
	info.input_format  = 0;
	info.input_width   = w;
	info.input_height  = h;
	
	info.output_type   = 0;
	info.output_format = 3;
	info.output_addr   = dst;
	info.output_offset = 0;
	info.output_stride = dstride;
	info.output_width  = w;
	info.output_height = h;
	
	info.z_buffer      = 0;
	Initial_CXFORM(&info.Cxform);
	Initial_Identify_MATRIX(&info.Matrix);

	SWF_2DFill(&info);
}

static void flash_set(int dst, unsigned int color, int dstride, int w, int h)
{
	FILLINFO info;
	info.input_type    = 0;
	info.DefColor      = color;
	info.input_need_blend = 0;
	info.output_type   = 0;
	info.output_format = 3;
	info.output_addr   = dst;
	info.output_offset = 0;
	info.output_stride = dstride;
	info.output_width  = w;
	info.output_height = h;
	info.z_buffer      = 0;
	Initial_CXFORM(&info.Cxform);
	SWF_2DFill(&info);
}

static void flash_set_window(INT16U * dst, INT32U color, int dstride, SWF_RECT * win1, SWF_RECT * win2)
{
	int x0 = MAX(win1->Xmin, win2->Xmin);
	int y0 = MAX(win1->Ymin, win2->Ymin);
	int x1 = MIN(win1->Xmax, win2->Xmax);
	int y1 = MIN(win1->Ymax, win2->Ymax);
	if(x1 > x0 && y1 > y0)
	{
		flash_set((int)(dst + y0 * dstride + x0), color, dstride, x1 - x0, y1 - y0);
	}
}

static void flash_set_video(INT16U * dst, int dstride, int x, int y, int w, int h, FUI_param_t * param)
{
#ifdef MIPS_VERSION
	int width  = IMAGE_WIDTH_E;
	int height = IMAGE_HEIGHT_E;
#else
	int width  = 800;
	int height = 600;
#endif
	SWF_RECT w1, w2;

	w1.Xmin = x;
	w1.Ymin = y;
	w1.Xmax = x + w;
	w1.Ymax = y + h;

	w2.Xmin = 0;
	w2.Ymin = 0;
	w2.Xmax = width;
	w2.Ymax = param->or_y;
	flash_set_window(dst, 0, width, &w1, &w2);

	w2.Xmin = 0;
	w2.Ymin = param->or_y;
	w2.Xmax = param->or_x;
	w2.Ymax = param->or_y + param->pic_h;
	flash_set_window(dst, 0, width, &w1, &w2);

	w2.Xmin = param->or_x + param->pic_w;
	w2.Ymin = param->or_y;
	w2.Xmax = width;
	w2.Ymax = param->or_y + param->pic_h;
	flash_set_window(dst, 0, width, &w1, &w2);
	
	w2.Xmin = 0;
	w2.Ymin = param->or_y + param->pic_h;
	w2.Xmax = width;
	w2.Ymax = height;
	flash_set_window(dst, 0, width, &w1, &w2);
}

int fui_frame_ready(INT8U * frame, void * arg)
{
	INT16U * dst = (INT16U*)frame;
	INT32U * src = (INT32U*)flashInfo.frame_addr;
#ifdef MIPS_VERSION
	int width  = IMAGE_WIDTH_E;
	int height = IMAGE_HEIGHT_E;
#else
	int width  = 800;
	int height = 600;
#endif
	FUI_param_t * param = (FUI_param_t*)arg;

	if(param->flag)
	{
		flash_set_video(dst, width, 0, 0, width, height, param);
	}

	if(src)
	{
		if(BLIST_IS_EMPTY(&flashInfo.roi))
		{
			int dst_x = MAX(0, flashInfo.x);
			int dst_y = MAX(0, flashInfo.y);
			int src_x = dst_x - flashInfo.x;
			int src_y = dst_y - flashInfo.y;
			int w = MIN(flashInfo.width - src_x, width - dst_x);
			int h = MIN(flashInfo.height- src_y, height- dst_y);
			if(w > 0 && h > 0)
			{
				//check if dst buffer should be cleaned first
				flash_set_video(dst, width, dst_x, dst_y, w, h, param);
				//flash copy
				flash_copy((int)(dst + dst_y * width + dst_x), (int)(src + src_y * flashInfo.width + src_x), width, flashInfo.width, w, h);
			}
		}
		else
		{
			ROI_INFO * roi;
			ITERATE_OVER_LIST(&flashInfo.roi, (BLIST*)roi)
			{
				int dst_x = MAX(0, flashInfo.x + roi->x);
				int dst_y = MAX(0, flashInfo.y + roi->y);
				int src_x = dst_x - flashInfo.x;
				int src_y = dst_y - flashInfo.y;
				int w = MIN(roi->width - (src_x - roi->x), width  - dst_x);
				int h = MIN(roi->height- (src_y - roi->y), height - dst_y);
				//check if dst buffer should be cleaned first
				flash_set_video(dst, width, dst_x, dst_y, w, h, param);
				//flash copy
				flash_copy((int)(dst + dst_y * width + dst_x), (int)(src + src_y * flashInfo.width + src_x), width, flashInfo.width, w, h);
			}
		}
	}

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
		return DEFAULT_INTERVAL;
	}
}

void fui_show(int frame_addr, void * dev)
{
	if(frame_addr)
	{
		if(flashInfo.s && flashInfo.type)
		{
			//flashInfo.frame_addr = frame_addr;
			//SET OSD DISPLAY
			ROI_INFO * roi;
			swf_show_osd(NULL,0,0,0,0,0);
			ITERATE_OVER_LIST(&flashInfo.roi, (BLIST*)roi)
			{
				int dst_x = MAX(0, flashInfo.x + roi->x);
				int dst_y = MAX(0, flashInfo.y + roi->y);
				int src_x = dst_x - flashInfo.x;
				int src_y = dst_y - flashInfo.y;
				int w = MIN(roi->width - (src_x - roi->x), 800 - dst_x);
				int h = MIN(roi->height- (src_y - roi->y), 600 - dst_y);
				swf_show_osd((INT16U*)frame_addr + src_y * flashInfo.width + src_x, dst_x, dst_y, w, h, flashInfo.width);
			}
		}
		else
		{
#ifdef MIPS_VERSION
			DE_DisplayMode_DataType * pDispModeData = (DE_DisplayMode_DataType*)dev;
			pDispModeData->iInputLumaAaddr = frame_addr;
			pDispModeData->DisplaySource   = 0;
			S_LCD_DESet_Display_Mode(pDispModeData,DE_NO_UPDATA);
#else
			swf_show(frame_addr, (int)dev);
#endif
		}
	}
}

//AddROI(x,y,width,height)
static int AddROI(void * handle)
{
	ROI_INFO * roi = (ROI_INFO*)SWF_StaticMalloc(sizeof(ROI_INFO));
	SWFEXT_FUNC_BEGIN(handle);
	roi->x      = Swfext_GetNumber() / 2 * 2;
	roi->y      = Swfext_GetNumber() / 2 * 2;
	roi->width  = Swfext_GetNumber() / 2 * 2;
	roi->height = Swfext_GetNumber() / 2 * 2;
	roi->id     = get_next_roi_id();
	BListAdd(&flashInfo.roi, &roi->BList);
	Swfext_PutNumber(roi->id);
	SWFEXT_FUNC_END();
}

//RemoveROI(id)
static int RemoveROI(void * handle)
{
	ROI_INFO * roi;
	int id;
	SWFEXT_FUNC_BEGIN(handle);
	id = Swfext_GetNumber();
	ITERATE_OVER_LIST(&flashInfo.roi, (BLIST*)roi)
	{
		if(roi->id == id)
		{
			BListRemove(&roi->BList);
			SWF_Free(roi);
			Swfext_PutNumber(1);
			goto __end;
		}
	}
	Swfext_PutNumber(0);
__end:
	SWFEXT_FUNC_END();
}

//Play(path, trans, x, y, width, height, fps)
static int Play(void * handle)
{
	INT32U flag = SWF_FULLSCREEN_FLAG | SWF_STANDALONE | SWF_NON_RELOAD;
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
		get_full_path(handle, path, flashInfo.path);
		narg--;
		
		flashInfo.type = Swfext_GetNumber();
		if(flashInfo.type)
		{
			flag |= SWF_FORMAT_RGB565;
		}
		narg--;
		
		if(narg > 0)
		{
			flashInfo.x      = Swfext_GetNumber() / 2 * 2;
			flashInfo.y      = Swfext_GetNumber() / 2 * 2;
			flashInfo.width  = Swfext_GetNumber() / 2 * 2;
			flashInfo.height = Swfext_GetNumber() / 2 * 2;
			narg -= 4;
			if(narg > 0)
			{
				int fps = Swfext_GetNumber();
				if(fps)
				{
					flashInfo.interval = MAX(2, 100 / fps - 5);
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
#ifdef MIPS_VERSION
			flashInfo.width  = IMAGE_WIDTH_E;
			flashInfo.height = IMAGE_HEIGHT_E;
#else
			flashInfo.width  = 800;
			flashInfo.height = 600;
#endif
			flashInfo.interval = DEFAULT_INTERVAL;
		}
		
		//set window param
		win.Xmin = 0;
		win.Ymin = 0;
		win.Xmax = flashInfo.width  - 1;
		win.Ymax = flashInfo.height - 1;

		//insert instance
		flashInfo.s = SWF_AddInst(flashInfo.path, &win, flag, NULL);

		if(flashInfo.s)
		{
			Swfext_GetNumber(1);
			goto __end;
		}
	}

	Swfext_GetNumber(0);

__end:
	SWFEXT_FUNC_END();	
}

static int Stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(flashInfo.s)
	{
		ROI_INFO * roi;
		SWF_RemoveInst(flashInfo.s);
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
		flashInfo.s = NULL;
		flashInfo.type = 0;
		flashInfo.interval = DEFAULT_INTERVAL;
		while(roi = (ROI_INFO*)BListRemoveItemFromHead(&flashInfo.roi))
		{
			SWF_Free(roi);
		}
		cvDestroyWindow("osd");
	}
	SWFEXT_FUNC_END();	
}

static int GetLastFlashName(void * handle)
{
	char str[255];
	SWFEXT_FUNC_BEGIN(handle);
	SWF_GetLatestInstName(str);
	Swfext_PutString(str);
	SWFEXT_FUNC_END();	
}

static int GetFilePath(void * handle)
{
	char *str;
	SWFEXT_FUNC_BEGIN(handle);
	str = SWF_GetFilePath(handle);
	Swfext_PutString(str);
	SWFEXT_FUNC_END();	
}

#if 0
INT32S gc_test_count_text( void * handle )
{
    void * obj = 0;
    unsigned int chinese = 0;
    unsigned int english = 0;
    char * str = 0;
    SWFEXT_FUNC_BEGIN(handle);
    obj = SWF_AllocObject( 0 );
    str = Swfext_GetString();
    printf( "%s\n", str );
    SWF_getTextCount( str, 8, &english, &chinese );
    printf( "e %d c %d\n", english, chinese );
    SWF_AddNumberToObject(handle,  (void*)obj, "eng", english );
    SWF_AddNumberToObject(handle,  (void*)obj, "chn", chinese );
    Swfext_PutObject( obj );
    SWFEXT_FUNC_END();
}

INT32S gc_test_paint_text( void * handle )
{
    void * obj = 0;
    unsigned int begin = 0;
    unsigned int end = 0;
    unsigned int color = 0;
    SWFEXT_FUNC_BEGIN(handle);
    obj = Swfext_GetObject();
    begin = Swfext_GetNumber();
    end = Swfext_GetNumber();
    color = Swfext_GetNumber();
    printf( "%p %u %u %x\n", obj, begin, end, color );
    SWF_SetTextColor( obj, begin, end, color );
    printf( "end" );
    SWFEXT_FUNC_END();
}
#endif

INT32S swfext_flashengine_register(void)
{
	init_flash_info();
	SWFEXT_REGISTER("flash_AddROI", AddROI);
	SWFEXT_REGISTER("flash_RemoveROI", RemoveROI);
	SWFEXT_REGISTER("flash_Play", Play);
	SWFEXT_REGISTER("flash_Stop", Stop);

	SWFEXT_REGISTER("flash_GetLastFlashName", GetLastFlashName);
	SWFEXT_REGISTER("flash_GetFilePath", GetFilePath);

#if 0
	SWFEXT_REGISTER("gc_test_count_text", gc_test_count_text);
	SWFEXT_REGISTER("gc_test_paint_text", gc_test_paint_text);
#endif
	
	return 0;
}
