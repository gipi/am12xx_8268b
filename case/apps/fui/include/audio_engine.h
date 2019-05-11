#ifndef SWFEXT_MUSIC_ENGINE_H
#define SWFEXT_MUSIC_ENGINE_H

//#define AUDIO_DEBUG
#ifdef AUDIO_DEBUG
#define ae_info(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define ae_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#else

#define ae_info(fmt,arg...) do{}while(0);
#define ae_err(fmt,arg...) do{}while(0);
#endif

typedef enum{
	AE_IDLE=0,  //< music engine state: idle
	AE_PLAY=1, //< music engine state: play
	AE_PAUSE=2,//< music engine state: pause
	AE_FF=3,//< music engine state: fast forward
	AE_FB=4,//< music engine state: fast backward
	AE_STOP=5,	//< music engine state: stop
	AE_ERROR = 6,//< music engine state: error
	AE_READY = 7, //< music engine state: Ready
	AE_WAIT_STOP_OK = 8, //< music engine state: 
}MUSIC_STATE;

typedef enum{
	ID3_AUTHOR=1,
	ID3_COMPOSER=2,
	ID3_ALBUM=4,
	ID3_GENRE=8,
	ID3_YEAR=16,
	ID3_PIC_WH=32,
}MUSIC_ID3_INFO_E;


/**
*brief open an audio engine
*
*@param[in] NULL
*@return the pointer to the engine
*/
void *audio_play_open();


/**
*brief close an audio engine
*
*@param[in] NULL
*@return the pointer to the engine
*/
int audio_play_close(void *handle);



/**
*brief send the command to the middware
*
*@param[in] handle: the audio engine
*@param[in] cmd the cmd is used for send control cmd to decoder
* - PLAY:start music play
* - STOP:stop music play
* - PAUSE:pause music play
* - CONTINUE:continue music play
* - FAST_FORWARD:fast forward music play
* - FAST_BACKWARD:fast backward music play
* - CANCEL_FF:cancel fastforward
* - CANCEL_FB:cancel fastbackward
* - SET_FILE:set file play  
* - GET_MEDIA_INFO:get music info
* - GET_PLAYER_STATUS:get player status
* @param[in] param the param in used for some cmd to get or send ext info.
* @return return the result.
* - 0:send cmd successfully.
* - -1: failed.
*/
int audio_play_cmd(void *handle,unsigned int cmd,unsigned int param);

#endif