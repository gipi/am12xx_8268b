#include "swf.h"
#include "swf_types.h"
#include "swfext.h"
#include "swfdec.h"
#include "stdio.h"
#include "stdlib.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

enum PeState
{
	PE_IDLE=0,
	PE_PLAY=1,
	PE_PAUSE=2,
};

enum PeMode
{
	PE_RATIO_LETTER_BOX=0,
	PE_RATIO_FULL_SCREEN=1,
	PE_RATIO_ACTUAL_SIZE=2,
	PE_MODE_ZOOMOUT=3,
	PE_MODE_ZOOMIN=4,
	PE_MODE_ROTATE_LEFT=5,
	PE_MODE_ROTATE_RIGHT=6,
	PE_MODE_MOVE_LEFT=7,
	PE_MODE_MOVE_RIGHT=8,			
	PE_MODE_MOVE_UP=9,
	PE_MODE_MOVE_DOWN=10,
};

static INT32S  decode_w = 0,decode_h = 0;
static INT8U * decode_buffer = NULL, * decode_sim_buffer = NULL;
static void *  target = NULL;
static char	   root_path[128];
static INT32S  state = PE_IDLE;
static INT32S  pe_status = 0;

static INT32S  play_mode = 0;
static INT32S  play_ratio = 0;
static INT32S  show_time = 0;
static INT32S  show_effect = 0;
static INT32S  clock_enable = 0;

static INT32S  angle = 0;
static INT32S  zoom  = 1;
static INT32S  posX  = 0;
static INT32S  posY  = 0;

extern SWF_RECT frame;
static INT32S  frame_w = 0;
static INT32S  frame_h = 0;

static INT32S GetBitmapStatus(INT32S id)
{
	return pe_status;
}

static INT32S Open(void * handle)
{
	int type;
	SWFEXT_FUNC_BEGIN(handle);
	type = Swfext_GetParamType();
	if(type == SWFDEC_AS_TYPE_NULL)
	{
		STRCPY(root_path, "c:\\");
	}
	else
	{
		char * path = Swfext_GetString();
		STRCPY(root_path, path);
	}
	frame_w = frame.Xmax + 1;
	frame_h = frame.Ymax + 1;
	if(decode_sim_buffer == NULL)
	{
		decode_sim_buffer = (INT8U*)SWF_Malloc(frame_w * frame_h * 2);
	}
	if(decode_buffer == NULL)
	{
		decode_buffer = (INT8U*)malloc(frame_w * frame_h * 4);
		Swfext_PutNumber(1);
	}
	else
	{
		Swfext_PutNumber(0);
	}
	SWFEXT_FUNC_END();
}

static INT32S Attach(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	target   = Swfext_GetObject();
	decode_w = Swfext_GetNumber();
	decode_h = Swfext_GetNumber();
	SWF_AttachBitmap(target,(INT32U*)decode_buffer,decode_w,decode_h,frame_w,frame_h,frame_w,SWF_BMP_FMT_ARGB,GetBitmapStatus);
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
		target = NULL;
		Swfext_PutNumber(1);
	}
	else
	{
		Swfext_PutNumber(0);
	}
	SWFEXT_FUNC_END();
}

static INT32S Play(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("play slide show!\n");
	/*
	SWF_SetState(SWF_GetActiveInst(), SWF_STATE_SLEEP);
	state = PE_PLAY;
	{
		IplImage * img = cvCreateImage(cvSize(800,600), IPL_DEPTH_8U, 3);
		cvZero(img);
		cvShowImage("swf", img);
		cvReleaseImage(&img);
	}
	*/
	SWFEXT_FUNC_END();
}

static INT32S Pause(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	state = PE_PAUSE;
	printf("pause\n");
	SWFEXT_FUNC_END();
}

static INT32S Resume(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	state = PE_PLAY;
	printf("resume\n");
	SWFEXT_FUNC_END();
}

static INT32S Stop(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	printf("stop slide show!\n");
	/*
	SWF_SetState(SWF_GetActiveInst(), SWF_STATE_ACTIVE);
	*/
	state = PE_IDLE;
	SWFEXT_FUNC_END();
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

static INT32S Show(void * handle)
{
	char * path;
	int    mode;
	IplImage * img;
	
	SWFEXT_FUNC_BEGIN(handle);
	path = Swfext_GetString();
	mode = Swfext_GetNumber();

	printf("show mode : %d\n", mode);
	img = cvLoadImage(file_map(path), CV_LOAD_IMAGE_COLOR);

	if(img != NULL)
	{
		IplImage * dst = cvCreateImage(cvSize(frame_w,frame_h), IPL_DEPTH_8U, 3);
		int w, h;

		switch(mode)
		{
		case PE_RATIO_LETTER_BOX:
		case PE_RATIO_FULL_SCREEN:
		case PE_RATIO_ACTUAL_SIZE:
			cvResize(img, dst, CV_INTER_LINEAR);
			posX  = 0;
			posY  = 0;
			zoom  = 1;
			angle = 0;
			goto finish;
		case PE_MODE_ROTATE_LEFT:
			angle = (angle == 270) ? 0 : angle + 90;
			break;
		case PE_MODE_ROTATE_RIGHT:
			angle = (angle == 0) ? 270 : angle - 90;
			break;
		case PE_MODE_ZOOMIN:
			if(zoom < 8)
			{
				zoom *= 2;
				posX *= 2;
				posY *= 2;
			}
			break;
		case PE_MODE_ZOOMOUT:
			if(zoom > 1) 
			{
				zoom /= 2;
				posX /= 2;
				posY /= 2;
			}
			break;
		case PE_MODE_MOVE_LEFT:
			if(posX > 0) posX--;
			break;
		case PE_MODE_MOVE_RIGHT:
			if(posX < zoom - 1) posX++;
			break;
		case PE_MODE_MOVE_UP:
			if(posY > 0) posY--;
			break;
		case PE_MODE_MOVE_DOWN:
			if(posY < zoom - 1) posY++;
			break;
		}

		w = img->width / zoom;
		h = img->height / zoom;
		cvSetImageROI(img, cvRect(w * posX, h * posY, w * (posX+1), h * (posY+1)));
		cvResize(img, dst, CV_INTER_LINEAR);
		
		if(angle)
		{
			CvMat * mat = cvCreateMat(2, 3, CV_32FC1);
			IplImage * rot0 = cvCreateImage(cvSize(800,800), IPL_DEPTH_8U, 3);
			IplImage * rot1 = cvCreateImage(cvSize(800,800), IPL_DEPTH_8U, 3);
			cvResize(dst, rot0, CV_INTER_LINEAR);
			cv2DRotationMatrix(cvPoint2D32f(400,400), angle, 1, mat);
			cvWarpAffine(rot0, rot1, mat, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
			cvResize(rot1, dst, CV_INTER_LINEAR);
			cvReleaseImage(&rot0);
			cvReleaseImage(&rot1);
		}
finish:
		Ipl2Argb(dst, frame_w, (INT32U*)decode_buffer);
		cvReleaseImage(&dst);
		cvReleaseImage(&img);
		pe_status++;
		Swfext_PutNumber(1);
	}
	else
	{
		pe_status = 0;
		Swfext_PutNumber(0);
	}
	SWFEXT_FUNC_END();
}

static INT32S GetState(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(state);
	SWFEXT_FUNC_END();
}


static INT32S GetStatus(void * handle)
{
	INT32S status;
	SWFEXT_FUNC_BEGIN(handle);
	status = GetBitmapStatus(0);
	Swfext_PutNumber(status);
	SWFEXT_FUNC_END();
}

static INT32S Close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	if(decode_sim_buffer != NULL)
	{
		SWF_Free(decode_sim_buffer);
		decode_sim_buffer = NULL;
	}
	Swfext_PutNumber(1);
	SWFEXT_FUNC_END();
}

static INT32S GetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(play_mode);
	SWFEXT_FUNC_END();
}

static INT32S SetPlayMode(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	play_mode = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

static INT32S GetPlayRatio(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(play_ratio);
	SWFEXT_FUNC_END();
}

static INT32S SetPlayRatio(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	play_ratio = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

static INT32S GetSlideShowTime(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(show_time);
	SWFEXT_FUNC_END();
}

static INT32S SetSlideShowTime(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	show_time = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}
static INT32S GetSlideShowEffect(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	show_effect = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

static INT32S SetSlideShowEffect(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(show_effect);
	SWFEXT_FUNC_END();
}


static INT32S GetClockEnable(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	Swfext_PutNumber(clock_enable);
	SWFEXT_FUNC_END();
}

static INT32S SetClockEnable(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	clock_enable = Swfext_GetNumber();
	SWFEXT_FUNC_END();
}

INT32S swfext_pe_register(void)
{
	SWFEXT_REGISTER("pe_Open", Open);
	SWFEXT_REGISTER("pe_Close", Close);
	SWFEXT_REGISTER("pe_Show", Show);
	SWFEXT_REGISTER("pe_Play", Play);
	SWFEXT_REGISTER("pe_Pause", Pause);
	SWFEXT_REGISTER("pe_Resume", Resume);
	SWFEXT_REGISTER("pe_GetStatus", GetStatus);
	SWFEXT_REGISTER("pe_Stop", Stop);
	SWFEXT_REGISTER("pe_Attach", Attach);
	SWFEXT_REGISTER("pe_Detach", Detach);
	SWFEXT_REGISTER("pe_GetPlayMode", GetPlayMode);
	SWFEXT_REGISTER("pe_SetPlayMode", SetPlayMode);
	SWFEXT_REGISTER("pe_GetPlayRatio", GetPlayRatio);
	SWFEXT_REGISTER("pe_SetPlayRatio", SetPlayRatio);
	SWFEXT_REGISTER("pe_GetSlideShowTime", GetSlideShowTime);
	SWFEXT_REGISTER("pe_SetSlideShowTime", SetSlideShowTime);
	SWFEXT_REGISTER("pe_GetSlideShowEffect", GetSlideShowEffect);
	SWFEXT_REGISTER("pe_SetSlideShowEffect", SetSlideShowEffect);
	SWFEXT_REGISTER("pe_GetClockEnable", GetClockEnable);
	SWFEXT_REGISTER("pe_SetClockEnable", SetClockEnable);
	return 0;
}


