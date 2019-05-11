#ifndef SWFEXT_VIDEO_ENGINE_H
#define SWFEXT_VIDEO_ENGINE_H

#include "av_api.h"
#include "ezcast_public.h"

typedef enum{
	/* idle: when no video played or closed */
	VE_IDLE=0,
	/* play: video played */
	VE_PLAY=1,
	/* pause: video paused */
	VE_PAUSE=2,
	/* fast forward play */
	VE_FF=3,
	/* fast back */
	VE_FB=4,
	/* stop: video stopped */
	VE_STOP=5,
	/* error: error happened*/
	VE_ERROR=6,
	/* stop ready:video ready to stop*/
	VE_READY_STOP = 7
}VIDEO_STATE;

#define FF_FB_STEP 20


/////以下是为TS PS 频道选择添加的/////
typedef enum {
	VIDEOINFO_PRO_NUM=0,/*获取prg_nb*/
	VIDEOINFO_PRO_INFO=1,/*获取prg内容*/
	/*以下 配合VIDEOINFO_PRO_INFO使用*/
	PRO_NAME=3,/*获取prg_name*/
	PRO_V_NUM=4,
	PRO_A_NUM=5,
	VIDEO_INFO=6,
	AUDIO_INFO=7,
	/*以下配合VIDEO_INFO 和AUDIO_INFO使用*/
	STREAM_ID=8,
	STREAM_VALID=9,
	STREAM_CODECID=10,
	STREAM_TYPE=11,
}video_info;

#define VIDEOINFO_ERR_MSG "Error:: Get video file info error\n"
#define VIDEOINFO_ERR_SET "Error: Set video channel error\n"

#define V_DBG printf


/**
* @brief prototype definitions for the video APIs
*/
typedef void *(*videofunc_open)(void *param);
typedef int (*videofunc_cmd)(void *handle, video_cmd_s *cmd);
typedef int (*videofunc_close)(void *handle,void *param);

/**
* @brief video heap size.
*/
#if AM_CHIP_ID == 1213
// 128 DDR
#if !EZCAST_ENABLE
#define _VIDEO_HEAP1_SIZE    (48*1024*1024)
#else
#if MODULE_CONFIG_DDR_CAPACITY==256
#define _VIDEO_HEAP1_SIZE    (90*1024*1024)
#elif MODULE_CONFIG_EZMUSIC_ENABLE
#define _VIDEO_HEAP1_SIZE    (6*1024*1024)
#else
//#define _VIDEO_HEAP1_SIZE    (40*1024*1024)
#define _VIDEO_HEAP1_SIZE    44031936	//temporary change.from 0x84602040 to 0x87000000,the ending is bank align
#endif
#endif

#else
#define _VIDEO_HEAP1_SIZE    (36*1024*1024)
#endif
#define _VIDEO_HEAP2_SIZE    (32*1024*1024)
#define _VIDEO_HEAP3_SIZE    (28*1024*1024)



#define _VIDEO_PREV_SIZE	 (16*1024*1024)


/**
*brief open a video engine
*
*@param[in] NULL
*@retval 1:success
*@retval 0:error
*/
int _video_dec_open();


/**
*brief close an video engine
*
*@param[in] NULL
*@retval 0		: close successfully
*@retval !0		: close failed
*/
int _video_dec_close();


/**
 *@brief  send cmd to video player.
 *@param[in] handle 	: the handle is return from video_dec_open() which is call in the  function named _video_dec_open.
 *@param[in] cmd 	 	: the cmd is used for send control cmd to player.
 *@retval 0			: cmd send successfully.
 *@retval !0			: cmd send failed.
 */ 
int _video_dec_send_cmd(void *handle,video_cmd_s *cmd);

#endif
