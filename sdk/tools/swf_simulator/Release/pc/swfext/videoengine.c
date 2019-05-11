#include "swf.h"
#include "swfext.h"
#include "stdio.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "windows.h"
#include "fui_common.h"

enum VeState
{
	VE_IDLE=0,
	VE_PLAY=1,
	VE_PAUSE=2,
	VE_FF=3,
	VE_FB=4,
	VE_CLOSE = 5,
};

static CvCapture * capture = NULL;
static int totalFrame = 0;
static int totalTime = 0;
static int fps = 0;
static HANDLE vt = 0;
static int ve_state = VE_IDLE;
static int ve_mode = 0;
static int ve_ratio = 0;
static int lcd_w = 0, lcd_h = 0;

static INT32S  decode_w = 0,decode_h = 0;
static INT8U * decode_buffer = NULL;
static void *  target = NULL;
static INT32S  ve_pic_status = 1;
static char    target_path[AS_MAX_TEXT];

static INT32S GetBitmapStatus(int id)
{
	return ve_pic_status;
}

static void Ipl2Argb(IplImage * img, int stride, INT32U * frame)
{
	int i, j;
	for(i = 0; i < img->height; i++)
	{
		for(j = 0; j < img->width; j++)
		{
			INT8U r, g, b;
			b = img->imageData[i * img->widthStep + j * 3 + 0];
			g = img->imageData[i * img->widthStep + j * 3 + 1];
			r = img->imageData[i * img->widthStep + j * 3 + 2];
			frame[i * stride + j] = ARGB_MUX(255, r, g, b);
		}
	}
}

static void Ipl2YUV(IplImage * img, INT32U * frame, int stride)
{
	int i, j;
	for(i = 0; i < img->height; i++)
	{
		for(j = 0; j < img->width; j++)
		{
			INT8U r, g, b, y1, u1, v1, y2, u2, v2;
			b = img->imageData[i * img->widthStep + j * 3 + 0];
			g = img->imageData[i * img->widthStep + j * 3 + 1];
			r = img->imageData[i * img->widthStep + j * 3 + 2];
			RGB2YUV(y1, u1, v1, r, g, b);
			j++;
			b = img->imageData[i * img->widthStep + j * 3 + 0];
			g = img->imageData[i * img->widthStep + j * 3 + 1];
			r = img->imageData[i * img->widthStep + j * 3 + 2];
			RGB2YUV(y2, u2, v2, r, g, b);
			u1 = (u1 + u2) / 2;
			v1 = (v1 + v2) / 2;
			frame[(i * stride + j) / 2] = ARGB_MUX(v1, y2, u1, y1);
		}
	}
}

extern void swf_combine_show(int viewPort, int channel, int type);

DWORD WINAPI VideoThread(LPVOID lpParameter)
{
	while(ve_state != VE_CLOSE)
	{
		switch(ve_state)
		{
		case VE_IDLE:
			break;
		case VE_PLAY:
		case VE_FF:
		case VE_FB:
			if(capture != NULL)
			{
				if(ve_mode)
				{
					IplImage * img = cvQueryFrame(capture);
					if(img != NULL)
					{
						FUI_param_t param;
						IplImage * dst = cvCreateImage(cvSize(800,600), IPL_DEPTH_8U, 3);
						cvFlip(img, img, 0);
						cvResize(img, dst, CV_INTER_LINEAR);
						Ipl2YUV(dst, (INT32U*)decode_buffer, dst->width);
						cvReleaseImage(&dst);

// 						param.or_x = 50;
// 						param.or_y = 50;
// 						param.pic_w = 700;
// 						param.pic_h = 500;

						param.or_x  = 0;
						param.or_y  = 0;
						param.pic_w = 800;
						param.pic_h = 600;
						param.flag = 0;
						fui_frame_ready(decode_buffer, &param);

						swf_show(decode_buffer, SWF_BMP_FMT_YUV422);
					}
					else
					{
						cvSetCaptureProperty(capture, CV_CAP_PROP_POS_MSEC, 0);
						ve_state = VE_IDLE;
					}
				}
			}
			break;
		case VE_PAUSE:
			break;
		}
		if(fps == 0)
		{
			cvWaitKey(100);
		}
		else
		{
			cvWaitKey(1000 / fps);
		}
	}
	vt = 0;
	return 0;
}

static INT32S Open(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	SWFEXT_FUNC_END();
}

static char * file_map(char * filename)
{
	static char buf[256];
	if(filename[1] == ':')
	{
		switch(filename[0])
		{
		case 'd':
		case 'D':
			STRCPY(buf, "SD");
			break;
		case 'e':
		case 'E':
			STRCPY(buf, "CF");
			break;
		case 'f':
		case 'F':
			STRCPY(buf, "UDISK");
			break;
		default:
			STRCPY(buf, "LOCAL");
			break;
		}
		STRCAT(buf, filename + 2);
		return buf;
	}
	else
	{
		return filename;
	}
}

static INT32S SetFile(void * handle)
{
	int type;
	char * path;
	SWFEXT_FUNC_BEGIN(handle);
	type = Swfext_GetParamType();
	if(type == SWFDEC_AS_TYPE_STRING)
	{
		path = Swfext_GetString();
		if(STRCMPI(path, target_path) == 0)
		{
			SWFEXT_FUNC_END();
		}
		capture = cvCreateFileCapture(file_map(path));
		if(capture != NULL)
		{
			totalFrame = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
			fps = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
			totalTime = totalFrame * 1000 / fps;
			lcd_w = (SWF_GetActiveInst()->window_rect.Xmax+1)/16;
			lcd_h = (SWF_GetActiveInst()->window_rect.Ymax+1)/16;
			if(decode_buffer == NULL)
			{
				decode_buffer = (INT8U*)malloc(lcd_w * lcd_h * 4);
				MEMSET(decode_buffer, 0x80, lcd_w * lcd_h * 4);
			}
			if(vt == 0)
			{
				ve_state = VE_IDLE;
				vt = CreateThread(NULL, 0, VideoThread, NULL, 0, NULL);
			}
			Swfext_PutNumber(1);
		}
		else
		{
			printf("cannot open %s\n", path);
			Swfext_PutNumber(0);
		}
	}
	SWFEXT_FUNC_END();	
}

static INT32S Close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(capture != NULL)
	{
		ve_state = VE_CLOSE;
		cvReleaseCapture(&capture);
		target_path[0] = 0;
	}
	printf("video close\n");
	SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	SWFEXT_FUNC_END();	
}

extern SWF_RECT frame;

static INT32S AttachPicture(void * handle)
{
	IplImage * img;
	int w = frame.Xmax + 1;
	int h = frame.Ymax + 1;
	SWFEXT_FUNC_BEGIN(handle);
	target   = Swfext_GetObject();
	decode_w = Swfext_GetNumber();
	decode_h = Swfext_GetNumber();
	img = cvQueryFrame(capture);
	if(img != NULL)
	{
		IplImage * dst = cvCreateImage(cvSize(w,h), IPL_DEPTH_8U, 3);
		cvResize(img, dst, CV_INTER_LINEAR);
		cvFlip(dst, NULL, 0);
		Ipl2Argb(dst, lcd_w, (INT32U*)decode_buffer);
		cvReleaseImage(&dst);
		ve_pic_status++;
	}
	SWF_AttachBitmap(target,(INT32U*)decode_buffer,decode_w,decode_h,w,h,w,SWF_BMP_FMT_ARGB,GetBitmapStatus);
	Swfext_PutNumber(1);
	SWFEXT_FUNC_END();
}

static INT32S Detach(void * handle)
{
	int clone;
	SWFEXT_FUNC_BEGIN(handle);
	clone = Swfext_GetNumber();
	if(target != NULL)
	{
		SWF_DetachBitmap(target,clone);
		Swfext_PutNumber(1);
	}
	else
	{
		Swfext_PutNumber(0);
	}
	SWFEXT_FUNC_END();
}

static INT32S Show(void * handle)
{
	int time, ratio, type;

	SWFEXT_FUNC_BEGIN(handle);
	time = Swfext_GetNumber();
	type = Swfext_GetParamType();
	if(type != SWFDEC_AS_TYPE_NULL)
	{
		ratio = Swfext_GetNumber();
	}
	if(capture != NULL)
	{
		IplImage * img;
		cvSetCaptureProperty(capture, CV_CAP_PROP_POS_MSEC, time);
		img = cvQueryFrame(capture);
		if(img != NULL)
		{
			IplImage * dst = cvCreateImage(cvSize(decode_w,decode_h), IPL_DEPTH_8U, 3);
			cvResize(img, dst, CV_INTER_LINEAR);
			cvFlip(dst, NULL, 0);
			Ipl2Argb(dst, lcd_w, (INT32U*)decode_buffer);
			cvReleaseImage(&dst);
			ve_pic_status++;
		}
	}
	SWFEXT_FUNC_END();	
}

static INT32S Play(void * handle)
{
	int type, ratio, repeat;
	static IplImage * img = NULL;

	if(capture != NULL)
	{
		SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
		
		SWFEXT_FUNC_BEGIN(handle);
		type = Swfext_GetParamType();
		if(type != SWFDEC_AS_TYPE_NULL)
		{
			ratio = Swfext_GetNumber();
		}
		type = Swfext_GetParamType();
		if(type != SWFDEC_AS_TYPE_NULL)
		{
			repeat = Swfext_GetNumber();
		}
		
		ve_state = VE_PLAY;
		ve_mode = 1;
	}

	SWFEXT_FUNC_END();	
}

static INT32S Stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	ve_state = VE_IDLE;
	ve_mode = 0;
	if(capture != NULL)
	{
		cvSetCaptureProperty(capture, CV_CAP_PROP_POS_MSEC, 0);
	}
	SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	SWFEXT_FUNC_END();	
}

static INT32S Pause(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(ve_state == VE_PLAY)
	{
		ve_state = VE_PAUSE;
	}
	SWFEXT_FUNC_END();	
}


static INT32S Resume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(ve_state == VE_PAUSE)
	{
		ve_state = VE_PLAY;
	}
	SWFEXT_FUNC_END();	
}

static INT32S FF(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("video ff\n");
	SWFEXT_FUNC_END();	
}

static INT32S FB(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("video fb\n");
	SWFEXT_FUNC_END();	
}


static INT32S State(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ve_state);
	SWFEXT_FUNC_END();	
}

static INT32S TotalTime(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(totalTime);
	SWFEXT_FUNC_END();	
}


static INT32S CurTime(void * handle)
{
	static int counter = 0;
	SWFEXT_FUNC_BEGIN(handle);
	if(capture != NULL)
	{
		int msec = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_POS_MSEC);
		Swfext_PutNumber(msec);
	}
	SWFEXT_FUNC_END();	
}

static INT32S GetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ve_mode);
	SWFEXT_FUNC_END();
}

static INT32S SetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	ve_mode = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

static INT32S GetPlayRatio(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(ve_ratio);
	SWFEXT_FUNC_END();
}

static INT32S SetPlayRatio(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	ve_ratio = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}


INT32S swfext_video_register(void)
{
	SWFEXT_REGISTER("ve_Open", Open);
	SWFEXT_REGISTER("ve_Close", Close);
	SWFEXT_REGISTER("ve_Picture", AttachPicture);
	SWFEXT_REGISTER("ve_SetFile", SetFile);
	SWFEXT_REGISTER("ve_Show", Show);
	SWFEXT_REGISTER("ve_Play", Play);
	SWFEXT_REGISTER("ve_Stop", Stop);
	SWFEXT_REGISTER("ve_Pause", Pause);
	SWFEXT_REGISTER("ve_Resume", Resume);
	SWFEXT_REGISTER("ve_FF", FF);
	SWFEXT_REGISTER("ve_FB", FB);
	SWFEXT_REGISTER("ve_TotalTime", TotalTime);
	SWFEXT_REGISTER("ve_CurTime", CurTime);
	SWFEXT_REGISTER("ve_State", State);
	SWFEXT_REGISTER("ve_GetPlayRatio", GetPlayRatio);
	SWFEXT_REGISTER("ve_SetPlayRatio", SetPlayRatio);
	SWFEXT_REGISTER("ve_GetPlayMode", GetPlayMode);
	SWFEXT_REGISTER("ve_SetPlayMode", SetPlayMode);
	return 0;
}
