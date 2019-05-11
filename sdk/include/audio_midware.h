#ifndef AUDIO_MIDWARE_H
#define	AUDIO_MIDWARE_H


#ifdef  _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif


/**
* @decode cmd definition
*/
#define			PLAY				0x01
#define			STOP				0x02
#define			pause				0x03
#define			SET_SEEK_TIME			0x04
#define			FAST_FORWARD			0x06	
#define			FAST_BACKWARD			0x07
#define			CANCEL_FF			0x08
#define			CANCEL_FB			0x09
#define			GET_MEDIA_INFO			0x13    	
#define			GET_PLAYER_STATUS		0x14    	//获取player当前的状态                                                               	
#define			SET_FILE			0x16		//在指定单曲播放时,为文件名(包含路径);
#define			SET_FLASH			0x17
#define			SET_PCM_INFO		0x18
#define			SET_NOTIFY_FUNC		0x19
#define			TERMINAL			0x20
#define			SET_EQ				0x21
#define			GET_EQ				0x22

/**@}*/

#define			FLASHLOOP_FLAG			-55

/**
 *@brief player status enumeration.
 */
typedef enum{
		PLAYER_PLAYING=1,
		PLAYER_PAUSED,
		PLAYER_FF,
		PLAYER_BF,
		PLAYER_STOPPED
}player_status_t;							//当前decoder状态信息的常量
/**
 *@brief play mode enumeration.
 */
typedef  enum{
		NORMAL_PLAY,						//正常播放					
		TAG_PLAY,						//TAG播放
		SEEK_PLAY						//断点续传/触摸屏seek播放
} play_mode_t;

/**
 *@brief playe parameter struct
 */
	typedef struct{
			play_mode_t mode;					//当前的播放模式
			unsigned int param;					//不同播放模式下的输入参数
		}play_param_t;	
								//启动时的播放模式	
/**
 *@brief error constant enumeration.
 */

typedef enum{
		ERR_OPEN_FILE=1,
		ERR_FILE_NOT_SUPPORT,
		ERR_DECODER_ERROR,
		ERR_NO_LICENSE,                    			//无license,播放次数已满或播放时间到期
      	ERR_SECURE_CLOCK,			                        //DRM时钟错误
      	ERR_LICENSE_INFO,                        			//DRM信息错误
      	ERR_OTHER						//其他错误
}err_constant_t;

								/**
 *@brief enum for notify msg, used for notify_func.  
 */ 
typedef enum _audio_notify_msg_e
{
	AUDIO_FILE_INFO_READY_MSG,	/**< file information is filled by audio player */
	AUDIO_NO_SUPPORT_MSG,			/**< not support */
	AUDIO_STOP_OK_MSG,			/**< audio player is stopped */
}audio_notify_msg_e;




/*********数据结构定义**********************/
/**
 *@brief music file infomation struct
 */
 #define AUDIO_FULL_PATH_SIZE	768

typedef struct{
		/*文件名*/	
		char filepath[AUDIO_FULL_PATH_SIZE];			/**< the file path*/
		char filename[AUDIO_FULL_PATH_SIZE];		/**< the file name */
		unsigned int file_index;				/**< the file index */
		unsigned int file_len;    				/**< length of the file */
		
		/*媒体信息*/
		unsigned int total_time;				/**< record the total time of music*/
		unsigned int bitrate;					/**< record the bitrate of music */
		unsigned int sample_rate;				/**< record the sample rate of music */
		unsigned int channels;					/**< record the channels of music */
	
}music_file_info_t;

typedef struct{
	unsigned int sample_rate;				/**< record the sample rate of music */
	unsigned int channels;					/**< record the channels of music */
	unsigned int bits_per_sample;
}pcm_info_t;
/**


 *@brief music status struct
 */
typedef struct{
		int file_index;						/**< index for current music */ //当前播放歌曲的索引
		int cur_time;						      /**< real time for current music */ //歌曲当前的播放时间		
		player_status_t status;				/**< player status */ //codec当前的状态		
		unsigned int err_no;					/**< error number */
		int cur_time_ms;			      /**< real time for current music */ //歌曲当前的播放时间		
	
}music_status_t;							//decoder当前需要返回给ap的状态信息,这些状态flag均是只读数据
/**
 *@brief file opration function struct
 */
typedef struct _file_iocontext_s
{	
	char *file_name;
	void *handle;
	long (*read)(void *, unsigned char *, unsigned long);
	long (*seek_set)(void *, long);
	long (*seek_cur)(void *, long);
	long (*seek_end)(void *, long);
	long (*tell)(void *);
	long (*get_file_length)(void *);
}file_iocontext_s;

/**
 *@brief struct of set file, used for setting file information.  
 */
typedef struct _a_set_file_s
{
	file_iocontext_s *f_io;  /**< file io operation func*/
	music_file_info_t *pInfo;     /**< file information*/
}a_set_file_s;


/**
 *@brief equalizer struct
 */
typedef struct
{
	char name[32];
	unsigned char sliderpos[10];	//range from 0 to 100
	unsigned char preamppos;		//no use now
	unsigned char reserved;			//no use now
}eq_info_t;




/*函数接口*/
/**
 * @brief  Open the audio decoder.
 *
 * 
 * @param[in] param the Param is used for play device select.
 * @return return the result of audio decoder open.
 * - open successfully:return a music_dec_engine_t struct with  (void *)type.
 * - open fail: return NULL.
 */ 
EXPORT void *audioDecOpen(void *param);
/**
 * @brief  send cmd to audio decoder.
 *
 * 
 * @param[in] handle the handle is a music_dec_engine_t type struct from the return of audioDecOpen().
 * @param[in] cmd the cmd is used for send control cmd to decoder
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
EXPORT int audioDecCmd(void *handle,unsigned int cmd,unsigned int param);
/**
 * @brief  close the audio decoder.
 *
 * 
 * @param[in] handle the handle is a music_dec_engine_t type struct from the return of audioDecOpen().
 * @return return the result.
 * - 0: close successfully.
 * - !0: close failed.
 */ 
EXPORT int audioDecClose(void *handle);
/**

 * @}

 */


#endif

