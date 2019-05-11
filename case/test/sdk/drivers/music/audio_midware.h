#ifndef AUDIO_MIDWARE_H
# define	AUDIO_MIDWARE_H


//
// 文件信息包括总时间，比特率等


#define			PLAY				0x01
#define			STOP				0x02
#define			pause				0x03
#define			SET_SEEK_TIME		0x04
#define			FAST_FORWARD			0x06	
#define			FAST_BACKWARD			0x07
#define			CANCEL_FF			0x08
#define			CANCEL_FB			0x09
                                                             	
#define			GET_MEDIA_INFO			0x13    	
#define			GET_PLAYER_STATUS		0x14    	//获取player当前的状态                                                               	

#define			SET_FILE			0x16		//在指定单曲播放时,为文件名(包含路径);

/**
 *@brief player tag struct
 */

/**
 *@brief player status enumeration.
 */
typedef enum{
		PLAYER_PLAYING,
		PLAYER_FF,
		PLAYER_BF,
		PLAYER_PAUSED,
		PLAYER_STOPPED,
		PLAYER_ERROR,
		FM_SEARCHING						//在radio播放时用到此状态
}player_status_t;							//当前decoder状态信息的常量
/**
 *@brief play mode enumeration.
 */
typedef  enum{
		NORMAL_PLAY,						//正常播放					
		TAG_PLAY,						//TAG播放
		SEEK_PLAY						//断点续传/触摸屏seek播放
} play_mode_t;
typedef struct{
			play_mode_t mode;					//当前的播放模式
			unsigned int param;					//不同播放模式下的输入参数
		}play_param_t;	
								//启动时的播放模式	
/**
 *@brief playe parameter struct
 */
	
//不同播放模式时的输入参数
typedef enum{
		ERR_OPEN_FILE,
		ERR_FILE_NOT_SUPPORT,
		ERR_DECODER_ERROR,
		ERR_NO_LICENSE,                    			//无license,播放次数已满或播放时间到期
      		ERR_SECURE_CLOCK,			                        //DRM时钟错误
      		ERR_LICENSE_INFO,                        			//DRM信息错误
      		ERR_OTHER						//其他错误
}err_constant_t;



/**@}*/
/*********数据结构定义**********************/
/**
 *@brief music file infomation struct
 */
 #define FULL_PATH_SIZE	128

typedef struct{
		/*文件名*/	
		char filepath[FULL_PATH_SIZE];			/**< the file path*/
		char filename[FULL_PATH_SIZE];		/**< the file name */
		unsigned int file_index;				/**< the file index */
		unsigned int file_len;    				/**< length of the file */
		
		/*媒体信息*/
		unsigned int total_time;				/**< record the total time of music*/
		unsigned int bitrate;					/**< record the bitrate of music */
		unsigned int sample_rate;				/**< record the sample rate of music */
		unsigned int channels;					/**< record the channels of music */
	
}music_file_info_t;
/**


 *@brief music status struct
 */
typedef struct{
		int file_index;						/**< index for current music */ //当前播放歌曲的索引
		int cur_time;						      /**< real time for current music */ //歌曲当前的播放时间		
		player_status_t status;				/**< player status */ //codec当前的状态		
		unsigned int err_no;					/**< error number */
	
}music_status_t;							//decoder当前需要返回给ap的状态信息,这些状态flag均是只读数据


/*函数接口*/
/**
 * @brief  Open the audio decoder.
 *
 * 
 * @param[in] param the Param is not used.
 * @return return the result of audio decoder open.
 * - open successfully:return a music_dec_engine_t struct with  (void *)type.
 * - open fail: return NULL.
 */ 
void *audioDecOpen(void *param);

int audioDecCmd(void *handle,unsigned int cmd,unsigned int param);

int audioDecClose(void *handle);

#endif

