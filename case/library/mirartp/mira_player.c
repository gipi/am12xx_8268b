#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <dlfcn.h>
#include <memory.h>

#include "sys_cfg.h"
#include "av_api.h" //For 'int64'
#include "mira_player.h"


/**
 * @brief prototype definitions for the video APIs
 */
typedef void *(*vOpenFn_t)(void *param);
typedef int (*vCmdFn_t)(void *handle, video_cmd_s *cmd);
typedef int (*vCloseFn_t)(void *handle, void *param);

typedef enum _vState_e _vState_t;
enum _vState_e
{
	V_ST_IDLE = 0,		/* idle: when no video played or closed */
	V_ST_PLAYING = 1,	/* play: video played */
	V_ST_PAUSED = 2,	/* pause: video paused */
	V_ST_FF = 3,		/* fast forward play */
	V_ST_FB = 4,		/* fast back */
	V_ST_STOPPED = 5,	/* stop: video stopped */
	V_ST_ERROR = 6,		/* error: error happened*/
	V_ST_READY_STOP = 7	/* stop ready:video ready to stop*/
};

typedef struct _vPlayer_s _vPlayer_t;
struct _vPlayer_s
{
	/** Handler for video_player.so dynamic library */
	void *dLib;

	/** Function pointers to video MMM APIs */
	void *handle;
	vOpenFn_t openFn;
	vCmdFn_t cmdFn;
	vCloseFn_t closeFn;

	/* Current file info struct */
	file_info_s fileInfo;
	video_cmd_s fileCmd;
	video_cmd_s paramCmd;

	/** Video play state */
	_vState_t state;
};

typedef struct ezPlayerPriv_s ezPlayerPriv_t;
struct ezPlayerPriv_s
{
	_vPlayer_t vPlayer;
};
static ezPlayerPriv_t ezPlayerPriv;



static int _vPlayerSendCmd(void *handle, video_cmd_s *cmd)
{
	if (handle != NULL && ezPlayerPriv.vPlayer.cmdFn != NULL)
	{
		printf("%s,%d, cmd=%d\n",__FUNCTION__,__LINE__, cmd->con);
		return ezPlayerPriv.vPlayer.cmdFn(handle, cmd);
	}
	return -1;
}

static void _vPlayerGetMsgCb(notify_msg_e msg)
{
	video_cmd_s cmd;
	
	switch (msg) 
	{
	case NO_SUPPORT_MSG:
		printf("MiraPlay MSG: NO_SUPPORT_MSG\n");
		break;
		
	case FILE_INFO_READY_MSG:
		printf("MiraPlay MSG: FILE_INFO_READY_MSG\n");
		ezPlayerPriv.vPlayer.state = V_ST_PLAYING;
		break;
		
	case NEED_STOP_MSG:
		printf("MiraPlay MSG: NEED_STOP_MSG\n");
		ezPlayerPriv.vPlayer.state = V_ST_READY_STOP;
		cmd.con = V_STOP;
		_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &cmd);	
		break;
		
	case STOP_OK_MSG:
		printf("MiraPlay MSG: STOP_OK_MSG\n");
		ezPlayerPriv.vPlayer.state = V_ST_STOPPED;
		break;
		
	case PREVIEW_OK_MSG:
		printf("MiraPlay MSG: PREVIEW_OK_MSG\n");
		break;
		
	case CLEAR_FFB_FLAG:
		printf("MiraPlay MSG: CLEAR_FFB_FLAG\n");
		break;
		
	case VIDEO_DEC_MEMFAIL_MSG:
		printf("MiraPlay MSG: VIDEO_DEC_MEMFAIL_MSG\n");
		break;
		
	case VIDEO_DEC_FREEZED_PIC_RDY_MSG:
		printf("MiraPlay MSG: VIDEO_DEC_FREEZED_PIC_RDY_MSG\n");
		break;
		
	case VIDEO_READ_INDICATE_MSG:
		//printf("MiraPlay MSG: VIDEO_READ_INDICATE_MSG\n");
		break;
		
	case VIDEO_SEEK_START_MSG:
		printf("MiraPlay MSG: VIDEO_SEEK_START_MSG\n");
		break;
		
	case VIDEO_SEEK_STOP_MSG:
		printf("MiraPlay MSG: VIDEO_SEEK_STOP_MSG\n");
		break;
		
	default:
		//printf("-----%s(%d)-----\n", __FUNCTION__, msg);
		break;
	}
}

static int _vPlayerOpen(void)
{
	char mmmLib[64];
	
	/** 
	 * Open the MMM dynamic lib 
	 */
	sprintf(mmmLib, "%s%s", AM_DYNAMIC_LIB_DIR, "libvideo_player.so");
	if (ezPlayerPriv.vPlayer.dLib != NULL) 
	{
		dlclose(ezPlayerPriv.vPlayer.dLib);
	}
	ezPlayerPriv.vPlayer.dLib = dlopen(mmmLib, RTLD_LAZY|RTLD_LOCAL);//RTLD_GLOBAL);
	if (ezPlayerPriv.vPlayer.dLib == NULL) 
	{
		printf("dlopen() error\n");
		return -1;
	}

	/**
	 * Resolve the MMM APIs.
	 */
	ezPlayerPriv.vPlayer.openFn = dlsym(ezPlayerPriv.vPlayer.dLib, "video_dec_open");
	if (ezPlayerPriv.vPlayer.openFn == NULL) 
	{
		printf("ezPlayerPriv.vPlayer.openFn error:%s\n", dlerror());
		goto EZSTREAM_DEC_OPEN_ERROR;
	}

	ezPlayerPriv.vPlayer.cmdFn = dlsym(ezPlayerPriv.vPlayer.dLib, "video_dec_cmd");
	if (ezPlayerPriv.vPlayer.cmdFn == NULL) 
	{
		printf("ezPlayerPriv.vPlayer.cmdFn error:%s\n", dlerror());
		goto EZSTREAM_DEC_OPEN_ERROR;
	}

	ezPlayerPriv.vPlayer.closeFn = dlsym(ezPlayerPriv.vPlayer.dLib, "video_dec_close");
	if (ezPlayerPriv.vPlayer.closeFn == NULL) 
	{
		printf("ezPlayerPriv.vPlayer.closeFn error:%s\n", dlerror());
		goto EZSTREAM_DEC_OPEN_ERROR;
	}

	/**
	 * Open the video dec handle
	 */
	ezPlayerPriv.vPlayer.handle = ezPlayerPriv.vPlayer.openFn(NULL);
	if (ezPlayerPriv.vPlayer.handle == NULL) 
	{
		printf("ezPlayerPriv.vPlayer.handle error\n");
		goto EZSTREAM_DEC_OPEN_ERROR;
	}

	printf("%s,%d\n",__FUNCTION__,__LINE__);

	/**
	 * It seems everything init ok, just return
	 */
	return 0;

EZSTREAM_DEC_OPEN_ERROR:
	ezPlayerPriv.vPlayer.openFn = NULL;
	ezPlayerPriv.vPlayer.cmdFn = NULL;
	ezPlayerPriv.vPlayer.closeFn = NULL;

	if (ezPlayerPriv.vPlayer.dLib != NULL) 
	{
		dlclose(ezPlayerPriv.vPlayer.dLib);
		ezPlayerPriv.vPlayer.dLib = NULL;
	}

	return -1;
}

static int _vPlayerClose(void)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if (ezPlayerPriv.vPlayer.handle != NULL && ezPlayerPriv.vPlayer.closeFn != NULL) 
	{
		ezPlayerPriv.vPlayer.closeFn(ezPlayerPriv.vPlayer.handle, NULL);
		ezPlayerPriv.vPlayer.handle = NULL;
	}

	ezPlayerPriv.vPlayer.openFn = NULL;
	ezPlayerPriv.vPlayer.cmdFn = NULL;
	ezPlayerPriv.vPlayer.closeFn = NULL;

	if (ezPlayerPriv.vPlayer.dLib != NULL) 
	{
		dlclose(ezPlayerPriv.vPlayer.dLib);
		ezPlayerPriv.vPlayer.dLib = NULL;
	}

	return 0;
}

int ezMiraSetFile()
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	ezPlayerPriv.vPlayer.fileCmd.con = V_SET_EZSTREAM;//V_SET_FILE;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.fileflag = 0;
	strcpy(ezPlayerPriv.vPlayer.fileCmd.par.set_file.filename, "ezStream.tmp");
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.open = _mirartpStreamOpen;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.close = _mirartpStreamClose;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.read = _mirartpStreamRead;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.write = NULL;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.seek_set = _mirartpStreamSeek;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.seek_end = _mirartpStreamSeek;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.tell = _mirartpStreamTell;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.get_length = _mirartpStreamGetLen;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.f_io.get_seekable = _mirartpStreamGetSeekable;
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.fileflag = AVFMT_STREAM;
	memset(&ezPlayerPriv.vPlayer.fileInfo, 0, sizeof(file_info_s));
	ezPlayerPriv.vPlayer.fileCmd.par.set_file.pInfo = &ezPlayerPriv.vPlayer.fileInfo;

	return 0;
}

int ezMiraSetParam(unsigned char*vheap,unsigned int pheap,unsigned int heapsize,int w,int h)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	ezPlayerPriv.vPlayer.paramCmd.con = V_SET_PARAM;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.is_preview = 0;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.frame_w = w;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.frame_h = h;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.pix_fmt = VIDEO_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;//VIDEO_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.p_heap.h_b_size = 1;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.p_heap.vir_addr[0] = vheap;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.p_heap.bus_addr[0] = pheap;
	ezPlayerPriv.vPlayer.paramCmd.par.set_par.p_heap.size[0] = heapsize;

	return 0;

}

void *ezMiraOpen()
{
	int err;
	video_cmd_s cmd;

	printf("%s,%d\n",__FUNCTION__,__LINE__);

	ezPlayerPriv.vPlayer.state = V_ST_IDLE;
	if ((err = _vPlayerOpen()) == -1) 
	{
		printf("_vPlayerOpen error(%d)\n", err);
		return NULL;
	}

	/**
	 * Register video callback function.
	 */
	cmd.con = V_SET_NOTIFY_FUNC;
	cmd.par.notify_func = _vPlayerGetMsgCb;	
	err = _vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &cmd);

	return (ezPlayerPriv.vPlayer.handle);
	
}

void ezMiraClose(void)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	ezMiraStop();
	
	if (ezPlayerPriv.vPlayer.handle != NULL)
	{
		video_cmd_s cmd;
		cmd.con = V_TERMINAL;
		_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &cmd);	

		_vPlayerClose();
	}
}

void ezMiraPlay(void)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if (ezPlayerPriv.vPlayer.handle != NULL)
	{
		ezPlayerPriv.vPlayer.fileCmd.par.set_file.demuxname = "lib_ts.so";
		ezPlayerPriv.vPlayer.fileCmd.par.set_file.av_fifo_enable = 0;
		
		_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &ezPlayerPriv.vPlayer.fileCmd);
		_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &ezPlayerPriv.vPlayer.paramCmd);
		
		do
		{
			video_cmd_s cmd;
			cmd.con = V_PLAY;
			_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &cmd);
			printf("%s,%d\n",__FUNCTION__,__LINE__);
		} while (0);
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);

}

void ezMiraStop(void)
{
	int cnt=0;
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if (ezPlayerPriv.vPlayer.handle != NULL)
	{
		video_cmd_s cmd;

		if (ezPlayerPriv.vPlayer.state != V_ST_STOPPED &&
			ezPlayerPriv.vPlayer.state != V_ST_ERROR &&
			ezPlayerPriv.vPlayer.state != V_ST_IDLE)
		{
			if (ezPlayerPriv.vPlayer.state == V_ST_PAUSED)
			{ 
				//Resume DAC, if under paused condition.
				printf("MiraPlayer resume (DAC) before stop!\n");
				cmd.con = V_RESUME;
				_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &cmd);
			}
			ezPlayerPriv.vPlayer.state = V_ST_READY_STOP;

			cmd.con = V_STOP;
			_vPlayerSendCmd(ezPlayerPriv.vPlayer.handle, &cmd);
			
			while (ezPlayerPriv.vPlayer.state != V_ST_STOPPED &&
				   ezPlayerPriv.vPlayer.state != V_ST_ERROR)
			{
				sleep(1);
				cnt++;
				if(cnt > 10){
					printf("MiraPlayer wait too long for video stop\n");
				}
			}
		}

		ezPlayerPriv.vPlayer.state = V_ST_IDLE;
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
}

void ezMiraInit()
{
	memset(&ezPlayerPriv, 0, sizeof(ezPlayerPriv_t));
	ezPlayerPriv.vPlayer.state = V_ST_IDLE;
}

