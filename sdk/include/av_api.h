#ifndef __AVAPI_H__
#define __AVAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
	#define int64	__int64
	#define uint64	unsigned __int64
#else
	#define int64 long long
	#define uint64 unsigned long long
#endif

/**
*@file   av_api.h
*@brief  video player API
*@author xiaoyue
*/

/**
* @addtogroup video_lib_s
* @{
*/

/**
*@name video display format
*@{
*/
/** default unkown format */
#define VIDEO_PIX_FMT_UNKNOWN							0
/** yuv 420 semiplanar,y...yuv...uv */
#define VIDEO_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR            0x020001U
/** yuv 422 interleaved,yuyv...yuyv */
#define VIDEO_PIX_FMT_YCBCR_4_2_2_INTERLEAVED           0x010001U
/** rgb 32bit */
#define VIDEO_PIX_FMT_RGB32                             0x041001U
/** rgb 16bit,5-6-5 */
#define VIDEO_PIX_FMT_RGB16_5_6_5                       0x040002U
/** @}*/ //video display format

/**
*@name video display mode
*@{
*/
/** letter box mode */
#define VIDEO_DISPLAY_MODE_LETTER_BOX			0
/** pan and scan mode */
#define VIDEO_DISPLAY_MODE_PAN_AND_SCAN			1
/** full screen mode */
#define VIDEO_DISPLAY_MODE_FULL_SCREEN			2
/** actual size mode */
#define VIDEO_DISPLAY_MODE_ACTUAL_SIZE			3
/** userdefinedl size mode */
#define VIDEO_DISPLAY_MODE_DAR_DATA_FOR_DE				4
/** @}*/ //video display mode

/**
*@name max stream/program/heap num
*@{
*/
/** max heap block,use for mem cfg */
#define MAX_HEAP_BLOCK	8
/** max program num in a stream */
#define MAX_PRG_NUM		16
/** max stream num in a file */
#define MAX_STREAM_NUM  8
/** max preview pic num */
#define MAX_PRE_NUM 8
/** @}*/ //max stream/program/heap num

/**
 *@brief struct of stream infomation
 */
typedef struct _stream_info_s {
	int valid;     			/**< if we can support(1: support, 0: do not support)*/
	int id;        			/**< stream id */     
	int codecid;   			/**< codec id */
	unsigned int width;		/**< stream pic width */
	unsigned int height;	/**< stream pic height */
	unsigned int frmrate;
	unsigned int duration;  /**< total time */
	char stream_type[12];   /**< stream type(h264, mpeg4, mpeg1/2, mp3, ac3...)*/
}stream_info_s;

/**
 *@brief struct of programe infomation
 */
typedef struct _program_info_s {
	char prg_name[100];  						/**< program name */
	int prg_id;									/**< program id */
	int prg_v_nb; 								/**< video program num */
	int prg_a_nb;    							/**< audio program num */
	stream_info_s video_info[MAX_STREAM_NUM]; 	/**< video stream information */
	stream_info_s audio_info[MAX_STREAM_NUM]; 	/**< audio stream information */          
}program_info_s;

/**
 *@brief struct of file info,filled by video player
 */
typedef struct _file_info_s
{
	unsigned int total_time;		 /**< total time */	
	int64 total_size;
	unsigned int prg_nb;			 /**< programe num*/
	int seekable;					/**< seekable info*/
	int frame_width;
	int frame_height;
	int frame_rate;
	int sample_rate; ///< samples per second
	int channels;	 ///< number of audio channels
	int bit_per_sample;
	int audio_codecid;	
	int video_codecid;	
	float bit_rate;//Kbps
	program_info_s prg[MAX_PRG_NUM]; /**< programe information*/
	char demux_dll[32];
	char video_dll[32];
	char audio_dll[32];
}file_info_s;


/**
 *@brief enum indicate decode the stream or the file
 */
typedef enum _file_flag_e{
	AVFMT_FILE,  /**< file format */          
	AVFMT_NOFILE,/**< realtime stream for cmmb*/     
	AVFMT_STREAM,/**< realtime stream */ 
}file_flag_e;

/**
 *@brief struct of file operation, used for setting file operation func.  
 */
typedef struct _file_iocontext_s
{
	void *(*open)(char *path, char *mode);						/**< open file opertion func */	
	int (*read)(void *fp, unsigned char *buf, int buf_size);	/**< read file opertion func */
	int (*write)(void *fp, unsigned char *buf, int buf_size);	/**< write file opertion func */
	long (*seek_set)(void *fp, int64 offset);					/**< seek set opertion func */
	long (*seek_end)(void *fp, int64 offset);					/**< seek end opertion func */
	int64 (*tell)(void *fp);									/**< tell file opertion func */
	int (*close)(void *fp);										/**< close file opertion func */
	int64 (*get_length)(void *fp);							/**< get file length opertion func */
	int (*get_seekable)(void *fp);							/**< get file seekable info opertion func */
	int (*set_stop)(void *fp);							/**< set file_io stop opertion func */
}file_iocontext_s;

/**
 *@brief struct of set file, used for setting file information.  
 */
typedef struct _v_set_file_s
{
	char filename[256*3]; 	/**< file name */
	file_flag_e fileflag; 	/**< file type,realtime type,reserve for cmmb*/
	char* demuxname;		/**< userdefined demuxname >*/
	unsigned int av_fifo_enable:1,	/**<av fifo enable>*/
			isM3u8File:1,
			xx:30;
	file_iocontext_s f_io;  /**< file io operation func*/
	file_info_s *pInfo;     /**< file information*/
}v_set_file_s;

/**
 *@brief struct of video heap, used for setting memory for player.  
 */
typedef struct _video_heap_s
{
	unsigned int h_b_size;    				/**< mem block num,max to MAX_HEAP_BLOCK */ 
	unsigned char *vir_addr[MAX_HEAP_BLOCK];/**< vir address */ 
	unsigned int bus_addr[MAX_HEAP_BLOCK];	/**< bus address */ 
	unsigned int size[MAX_HEAP_BLOCK];		/**< mem block size */ 
}video_heap_s;

/**
 *@brief struct of video player param, used for setting player output.  
 */
typedef struct _v_set_par_s
{
	unsigned int frame_w; 		/**< wanted display width */
	unsigned int frame_h;		/**< wanted display height */
	unsigned int pix_fmt;		/**< wanted display format, see video display format*/
	unsigned int display_mode;	/**< wanted display mode, see video display mode*/
	video_heap_s p_heap;		/**< set mem heap for player,see video heap*/
	unsigned int is_preview;	/**< indicate if preview mode,distinguish preview status and play status*/
}v_set_par_s;

/**
 *@brief struct of preview entry param, used for preview.  
 */
typedef struct _prev_entry
{
	unsigned int pre_time; 		/**< preview pic of stream time */
	unsigned int prev_w;		/**< wanted preview pic width */
	unsigned int prev_h;		/**< wanted preview pic height */
	unsigned char *vir_addr;	/**< preview pic vir address */
	unsigned int bus_addr;		/**< preview pic bus address */
	unsigned int actual_pre_w; 	/**< actual preview pic width,return by video player */
	unsigned int actual_pre_h; 	/**< actual preview pic height,return by video player */
} prev_entry;

/**
 *@brief struct of preview infomation, used for preview.  
 */
typedef struct _v_prev_par_s
{
	unsigned int prev_num; 			/**< preview num,max to MAX_PRE_NUM */
	prev_entry *pprev[MAX_PRE_NUM];	/**< preview entry information,see prev_entry */
} v_prev_par_s;

/**
 *@brief struct of set program, used for selecting the A/V stream in one program.  
 */ 
typedef struct _v_set_program_s {
	int v_id;      		/**< video stream id */
	int v_codecid;		/**< video stream codec id */
    int a_id;  			/**< audio stream id */
    int a_codecid;		/**< audio stream codec id */
	int a_onlyswitch;	/**< indicate if only change audio stream */
} v_set_program_s;

/**
 *@brief enum for notify msg, used for notify_func.  
 */ 
typedef enum _notify_msg_e
{
	FILE_INFO_READY_MSG,	/**< file information is filled by video player */
	PREVIEW_OK_MSG,			/**< preview pic is filled by video player*/
	NO_SUPPORT_MSG,			/**< not support */
	NEED_STOP_MSG,			/**< need to send stop cmd to player */
	STOP_OK_MSG,			/**< video player is stopped */
	CLEAR_FFB_FLAG,			/**< video player find need to clear ff/fb flag */
	VIDEO_DEC_MEMFAIL_MSG,			/**< no enough buffer for decoder */
	VIDEO_DEC_FREEZED_PIC_RDY_MSG,	/**< hantro error */
	VIDEO_READ_INDICATE_MSG,	/**< video read indicate mesg. */
	VIDEO_SEEK_START_MSG,	/**< video seek-start mesg. */
	VIDEO_SEEK_STOP_MSG, /**< video seek-stop mesg. */
	VIDEO_SEEK_DISABLE_MSG, /**< video seek-disable mesg. */
	VIDEO_INFO_READY_MSG,/**< video fileinfo can be read it mesg. */
	VIDEO_SEEK_FLUSH_OK_MSG,/**< video seek flush ok  mesg. */
}notify_msg_e;

/**
 *@brief a member of cmd struct, this enum is cmd entry.  
 */ 
typedef enum _con_cmd_e 
{
	V_SET_NOTIFY_FUNC, 	/**< set notify func cmd */
	V_SET_FILE,			/**< set file cmd */
	V_SET_PARAM,		/**< set player param cmd */
	V_SET_PROGRAM,		/**< set player programe cmd */
	V_GET_PERVIEW,		/**< get preview pic cmd */
	V_PLAY,				/**< play cmd */
	V_SEEK_SINGLE,		/**< single seek cmd */
	V_SEEK_CONTINUE,	/**< continue seek cmd */
	V_PAUSE,			/**< pause cmd */
	V_RESUME, 			/**< resume cmd */
	V_STOP,				/**< stop cmd */
	V_TERMINAL,			/**< terminal cmd */
	V_GET_CUR_TIME,		/**< get player cur time cmd */
	V_CHANGE_OUTPUT,	/**< change player output cmd */
	V_SET_EZSTREAM,		/**< set ezstream cmd */
	V_GET_CUR_STATUS,				/**< get player status cmd */
	V_SET_CUR_AUDIO_TIME,				/**< set audio time for av sync cmd */
}con_cmd_e;

/**
 *@brief a member of cmd struct, this uion is cmd param. 
 */ 
typedef union _par_cmd_u 
{
	void (* notify_func)(notify_msg_e);	/**< set notify fuc for palyer, fill with V_SET_NOTIFY_FUNC*/
	v_set_file_s set_file;				/**< set file information, fill with V_SET_FILE*/
	v_set_par_s set_par;				/**< set player param, fill with V_SET_PARAM*/
	v_set_program_s set_prog;			/**< set program information, fill with V_SET_PROGRAM*/
	v_prev_par_s prev_par;				/**< set preview information, fill with V_GET_PERVIEW*/
	unsigned int seek_single_time;		/**< set single seek time, fill with V_SEEK_SINGLE*/
	int seek_continue_step;				/**< set continue seek step time, fill with V_SEEK_CONTINUE*/
	unsigned int cur_time; 				/**< player cur time,returned by player. fill with V_GET_CUR_TIME*/
	int64 cur_audio_time_avsync;		/**< cur audio time,set by user for av sync*/
}par_cmd_u;

/**
 *@brief struct of cmd.
 */
typedef struct _video_cmd_s
{
	con_cmd_e con; /**< cmd content */
	par_cmd_u par; /**< cmd param */
}video_cmd_s;

 /**
 * @brief  open video player.
 * @param[in] param: now can set NULL,reserved for using later.
 * @return return the result of video player open.
 * - open successfully: return a handle of player with (void *)type.
 * - open fail: return NULL.
 */ 
void *video_dec_open(void *param);
/**
 * @brief  send cmd to video player.
 * @param[in] handle : the handle is return from video_dec_open().
 * @param[in] cmd 	 : the cmd is used for send control cmd to player.
 * @return return the result.
 * - 0: cmd send successfully.
 * - !0: cmd send failed.
 */ 
int video_dec_cmd(void *handle, video_cmd_s *cmd);
/**
 * @brief  close the video player.
 *
 * 
 * @param[in] handle : the handle is return from video_dec_open().
 * @param[in] param : now can set NULL,reserved for using later.
 * @return return the result.
 * - 0: close successfully.
 * - !0: close failed.
 */ 
int video_dec_close(void *handle,void *param);

/**
 * @}
 */
#ifdef __cplusplus
}
#endif

#endif
