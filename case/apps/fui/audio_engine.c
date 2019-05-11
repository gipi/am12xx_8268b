#include <dlfcn.h>
#include "swf_ext.h"
#include "audio_engine.h"
#include "fileselector.h"
#include "lyric_analysis.h"
#include "sys_cfg.h"
#include "audio_midware.h"
#include "sys_conf.h"
#include "sys_resource.h"
#include "id3.h"
#include "load_image_engine.h"
#include "fui_memfs.h"
#include "audio_common.h"
#include "stream_api.h"
#include <semaphore.h>
#include "ezcast_public.h"

extern void* adllhandle;
extern audio_dec_open	a_open;
extern audio_dec_cmd	a_cmd;
extern audio_dec_close	a_close;

void * audio_engine=NULL;
static volatile MUSIC_STATE audio_status=AE_IDLE;
char cur_file[AUDIO_FULL_PATH_SIZE];
music_status_t player_status;
music_file_info_t media_info;

static int id3_pic_parser_status=0;

static unsigned int  audio_error=0;
static unsigned char  audio_setfile_ok=0;
static int seek_resume=0;
static unsigned int seekplay_time=0;
int callBackStatus = 5;

/** 与歌词相关的变量定义 */
#define LYRIC_BUFFER  256
static  lyric_info_t Lyric;
lyric_tag_time_t Tagtime;
int lyricbytes=0;
char Buf[LYRIC_BUFFER];
char lyric_tmpbuf[LYRIC_BUFFER];

/**id3 information definition**/
#define ID3_BUFFER_LEN 256

//#define CSH_DEBUG_REPEAT

char music_id3_buffer[ID3_BUFFER_LEN];

void* a_info_sem = NULL;

sem_t *a_stop_ok_sem = NULL;

#if 1
extern unsigned long fui_get_bus_address(unsigned long logicaddr);

file_iocontext_s file_iocontext=
{
	.handle = NULL
};

static int _ae_change_buf_format(char *srcbuf,int *srclen,int srcformat,char *destbuf,int *destlen)
{
	int is_change_ok=0;
	charsetchg_info_t change_info;
	change_info.src_charset = srcformat;
	change_info.dest_charset = LAN_UTF8;
	change_info.inputbuf = srcbuf;
	change_info.inputbuflen = (size_t*)srclen;
	change_info.outputbuf = destbuf;
	change_info.outputbuflen = (size_t*)destlen;
	//print_test(StrTempBuf,TmpLineInfo.len,16);
	is_change_ok = change_chardec(&change_info);
	return is_change_ok;
}

static void _ae_print_test(char* strprint, int len,int eachline)
{
	int i=0;
	ae_info("\n~~~~~~~~~~len=%d~~~~~~~~~~~~\n",len);
	for(i=0;i<len;i++){
		if(i%eachline==0 && i!=0)
			printf("\n");
		printf("0x%2x,",*(strprint+i));
	}
	ae_info("\n~~~~~~~~~~~~~~~~~~~~~~\n");
}



#endif


/*static int _audio_dll_open(void)
{
	char audiolibpath[128];
	//open audio dll
	sprintf(audiolibpath,"%s%s",AM_DYNAMIC_LIB_DIR,"libmusic.so");
	ae_info("audiolibpath == %s\n",audiolibpath);
	if(adllhandle){
		dlclose(adllhandle);
	}
	adllhandle = dlopen(audiolibpath,RTLD_LAZY|RTLD_GLOBAL);
	ae_info("handle == 0x%x\n",adllhandle);
	if(adllhandle==NULL){
		printf("dl open error\n");
		goto dll_open_error;
	}

	//restore audio dll api
	a_open = dlsym(adllhandle,"audioDecOpen");
	if(a_open == NULL){
		ae_info("a_open error:%s\n",dlerror());
		goto init_api_error;
	}

	a_cmd = dlsym(adllhandle,"audioDecCmd");
	if(a_cmd == NULL){
		ae_info("a_cmd error\n");
		a_open = NULL;
		goto init_api_error;
	}

	a_close = dlsym(adllhandle,"audioDecClose");
	if(a_close == NULL){
		a_open = NULL;
		a_cmd = NULL;
		ae_info("a_close error\n");
		goto init_api_error;
	}

	return 0;


init_api_error:
	
	if(adllhandle){
		dlclose(adllhandle);
		adllhandle = NULL;
	}
	
dll_open_error:
	
	return -1;
}

int _audio_dll_close()
{
	if(audio_engine && a_close){
		a_close(audio_engine);
	}

	a_open = NULL;
	a_cmd = NULL;
	a_close = NULL;

	if(adllhandle){
		dlclose(adllhandle);
		adllhandle = NULL;
	}
	
	return 0;
}*/

file_iocontext_s fops;

#if 1
#if EZMUSIC_ENABLE
extern int ezAudioOutGet(void);
#endif
/**
*brief open an audio engine
*
*@param[in] NULL
*@return the pointer to the engine
*/
void *audio_play_open()
{
	
	/*if(-1==_audio_dll_open())
	{
		ae_info("Player dll open fail\n");
		return NULL;
	}*/
	audio_error=0;
	#if EZMUSIC_ENABLE
	audio_play_dev_e play_dev;
#if 0
	play_dev= PlayDev_Earphone_SPDIF51 ;
#else
	switch(ezAudioOutGet())
	{
		case 0:
			play_dev=PlayDev_Earphone_SPDIF51;
			break;
		case 1:
			play_dev=PlayDev_Earphone_SPDIF2;
			break;
		case 2:
			play_dev=PlayDev_I2S;
			break;
		default:
			play_dev=PlayDev_Earphone_SPDIF51;
			break;
	}
#endif
	return a_open(&play_dev);
	#endif
	return a_open(NULL);
}


/**
*brief close an audio engine
*
*@param[in] NULL
*@return the pointer to the engine
*/
int audio_play_close(void *handle)
{
	return a_close( handle);
}


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
int audio_play_cmd(void *handle,unsigned int cmd,unsigned int param)
{
	int rtn;
	if (handle ==NULL) return -1;
	if(audio_error&&(cmd!=STOP)){
		//printf("audio_error %d   %x  %x %d\n",audio_error,&audio_error,cmd,__LINE__);
		//audio_error=0;
		return -1;
	}
	rtn = a_cmd(handle,cmd,param);
	if(rtn == -1){
		ae_info("set audio error %x\n",cmd);
		//audio_error=1;
		
	}

	return rtn;
}

#endif
static void audio_send_safecmd()
{
	if(audio_engine){
		if(audio_status!=AE_STOP && audio_status!=AE_IDLE&&audio_status!=AE_WAIT_STOP_OK){
			if(audio_status==AE_FF){
				audio_play_cmd( audio_engine, CANCEL_FF, 0);
			}
			else if(audio_status==AE_FB){
				audio_play_cmd( audio_engine, CANCEL_FB, 0);
			}
			else if(audio_status==AE_PLAY){
				audio_play_cmd( audio_engine, pause, 0 );
			}
			audio_status=AE_WAIT_STOP_OK;
			audio_play_cmd( audio_engine, STOP, 0);
		}
	}
	else{
		ae_info("audio handle error : %s\n",__FUNCTION__);
	}
}




void *_ae_engine_create(void)
{
	return NULL;
}

void _ae_engine_delete()
{
	if(audio_engine){
		audio_play_close(audio_engine);
		
		audio_engine=NULL;
		ae_info("unload music engine ok\n");
	}
	else{
		ae_info("del ae null\n");
	}
}

#ifdef MODULE_CONFIG_AUDIO

long ae_get_total_time_for_dmr()
{
	long total_time = 0;
	
	
	if(audio_engine==NULL || audio_setfile_ok == 0){
		total_time = -1;
	}
	else{
		total_time = media_info.total_time;
		printf("get totaltime %ld\n",total_time);
	}
	return total_time*1000;
}

long ae_get_current_time_for_dmr()
{
	long crrent_time = 0;
	
		
	if(audio_engine==NULL || audio_setfile_ok == 0){
		crrent_time = -1;
	}
	else{
		if(seek_resume==0){
			audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
			crrent_time = player_status.cur_time;
		}
		else if(seek_resume==1){
			crrent_time =seekplay_time;
		}
	}
	ae_info("crrent_time =================== %ld\n",crrent_time);
	return crrent_time*1000;
}

int ae_seek_play_for_dmr(int seek_time)
{
	static play_param_t play_param;
	int total_time;

	seek_time = seek_time/1000;

	if(audio_engine==NULL || audio_setfile_ok == 0){
		ae_info(" fast backward error : [music engine null]\n");
		return -1;
	}
	else{
		switch(audio_status){
			case AE_FB:
				ae_info("cancel FB first\n");
				audio_status = AE_PLAY;
				audio_play_cmd(audio_engine,CANCEL_FB,0);
				break;
			
			case AE_FF:
				ae_info("cancel FF first\n");
				audio_status = AE_PLAY;
				audio_play_cmd(audio_engine,CANCEL_FF,0);
				break;

			case AE_PAUSE:
				ae_info("now status is pause\n");
				seek_resume = 1;
				break;
			default:
				goto seek_play;
				break;
			}
		seek_play:		
			ae_info("seek play\n");
			total_time = media_info.total_time;
			
			ae_info("total_time =================== %d\n",total_time);
			ae_info("seekplay_time =================== %d\n",seek_time);
			if(seek_time>=total_time)
				seek_time = total_time;
			else if(seek_time<0)
				seekplay_time = 0;
			ae_info("seek_time time is %d\n",seek_time);
			audio_play_cmd(audio_engine,SET_SEEK_TIME,seek_time);
	}
	
	return 0;
}

void _audio_get_player_msg(audio_notify_msg_e msg)
{
	//printf("-----get player msg:%d-----\n",msg);
	static audio_notify_msg_e preMsg = -1;
	switch(msg){
		
		case AUDIO_NO_SUPPORT_MSG:
			printf("FUI/audio_engie.c file is not support\n");
			audio_setfile_ok=0;
			if(a_info_sem){
				OSSemPost(a_info_sem);
			}
			audio_status = AE_ERROR;
			break;
		case AUDIO_STOP_OK_MSG:
			printf("FUI/audio_engie.c file AUDIO_STOP_OK_MSG\n");
			dlna_dmr_cb(&callBackStatus);
			if(a_stop_ok_sem){
				sem_post(a_stop_ok_sem);
			}
			audio_status=AE_STOP;
			break;	

		case AUDIO_FILE_INFO_READY_MSG:
			printf("audio file information ready\n");
			audio_setfile_ok=1;
			if(a_info_sem){
				OSSemPost(a_info_sem);
			}
			audio_status = AE_READY;
			break;
		default:
			printf("-----get unknown player msg:%d-----\n",msg);
			break;
	}
/*	
#ifndef CSH_DEBUG_REPEAT
#ifdef  MODULE_CONFIG_DLNA	
 			if(AUDIO_STOP_OK_MSG == msg)
				dlna_dmr_cb(&msg);
#endif
#endif
*/
	
}

int 	ezcast_ae_open(void)
{
	audio_setfile_ok = 0;
	printf("audio_engine=========%s,%d\n",audio_engine,__LINE__);
	if(audio_engine==NULL){
		ae_info("open audio engine   audio_error%d\n",audio_error);

		/**
		* disable swf internal audio when open audio-player
		*/
		disable_swf_audio();
		audio_engine = audio_play_open();
		printf("audio_engine=========%s,%d\n",audio_engine,__LINE__);
		if(NULL==audio_engine)
		{
			//audio_play_close(audio_engine);
			//audio_engine = audio_play_open();
			return 0;
		}

		/**
		* register audio callback function.
		*/

		audio_play_cmd(audio_engine,SET_NOTIFY_FUNC,(unsigned int)(&_audio_get_player_msg));
		audio_status = AE_IDLE;//Add By Denny@20141209

	}
	return 1;
}

static int ae_open(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);
	audio_setfile_ok = 0;
	if(audio_engine==NULL){
		ae_info("open audio engine   audio_error%d\n",audio_error);

		/**
		* disable swf internal audio when open audio-player
		*/
		disable_swf_audio();
		audio_engine = audio_play_open();
		if(NULL==audio_engine)
		{
			audio_play_close(audio_engine);
			audio_engine = audio_play_open();
		}
		/**
		* register audio callback function.
		*/

		audio_play_cmd(audio_engine,SET_NOTIFY_FUNC,(unsigned int)(&_audio_get_player_msg));

	}
	audio_status = AE_IDLE;

	SWFEXT_FUNC_END();	
}
void  audio_sem_wait(sem_t *sem)
{
	int err;
	int times=0;

AUDIO_PEND_REWAIT:
	err = sem_wait(sem);
	//printf("err==================%d\n",err);
	if(err == -1){
		int errsv = errno;
		
		printf("errsv==================%d\n",errsv);
		if(errsv == EINTR){
			if(times<20){
				sleep(2);
				times++;
				fprintf(stderr,"function:%s line:%d times:%d\n",__FUNCTION__,__LINE__,times);
				goto AUDIO_PEND_REWAIT;
			}
			else 
				return;
		}
		else{
			printf("work_sem_pend: errno:%d\n",errsv);
			return;
		}
	}
	return ;
}

void ezcast_ae_close(void)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(audio_engine){
		switch(audio_status){
			case AE_FF:
				audio_play_cmd(audio_engine,CANCEL_FF,0); 
				break;
			case AE_FB:
				audio_play_cmd(audio_engine,CANCEL_FB,0); 
				break;
				
			default:
				break;
		}
		audio_status = AE_IDLE;
		ae_info("close audio engine\n");
		if(a_stop_ok_sem){
			audio_sem_wait(a_stop_ok_sem);
			sem_destroy(a_stop_ok_sem);
			free(a_stop_ok_sem);
			a_stop_ok_sem = NULL;

		}
		audio_play_cmd(audio_engine,TERMINAL,0);
		
		_ae_engine_delete();
		//_audio_dll_close();
		_audio_clear_setfile_para(&file_iocontext);
		audio_engine = NULL;

		/**
		* enable swf internal audio when close audio-player
		*/
		enable_swf_audio();
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
}

static int ae_close(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);



	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(audio_engine){
		switch(audio_status){
			case AE_FF:
				audio_play_cmd(audio_engine,CANCEL_FF,0); 
				break;
			case AE_FB:
				audio_play_cmd(audio_engine,CANCEL_FB,0); 
				break;
				
			default:
				break;
		}
		audio_status = AE_IDLE;
		ae_info("close audio engine\n");
		if(a_stop_ok_sem){
			audio_sem_wait(a_stop_ok_sem);
			sem_destroy(a_stop_ok_sem);
			free(a_stop_ok_sem);
			a_stop_ok_sem = NULL;

		}
		audio_play_cmd(audio_engine,TERMINAL,0);
		
		_ae_engine_delete();
		//_audio_dll_close();
		_audio_clear_setfile_para(&file_iocontext);
		audio_engine = NULL;

		/**
		* enable swf internal audio when close audio-player
		*/
		enable_swf_audio();
	}
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	SWFEXT_FUNC_END();	
}


void ezcast_ae_play(void)
{
	char * file;
	int n;
	static play_param_t play_param;	

	printf("[%s]%d, audio_status=%d\n", __func__, __LINE__, audio_status);
	if(audio_engine==NULL){
		ae_info("play error : [music engine null]\n");
	}
	else{
		ae_info("audio_status%d\n",audio_status);
		switch(audio_status){
			case AE_FF:
				ae_info("play FF\n");
				audio_play_cmd(audio_engine,FAST_FORWARD,0);
				break;
			case AE_FB:
				ae_info("play FB\n");
				audio_play_cmd(audio_engine,FAST_BACKWARD,0);
				break;
			case AE_PAUSE:
				ae_info("pause to play\n");
				play_param.mode=TAG_PLAY;
				play_param.param=0;
				audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
				break;
			default:
				ae_info("normal play%d\n",audio_error);
				play_param.mode=NORMAL_PLAY;
				play_param.param=0;
				audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
				break;
		}
		audio_status = AE_PLAY;
	}
}

static int ae_play(void * handle)
{
	char * file;
	int n;
	static play_param_t play_param;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("[%s]%d, audio_status=%d\n", __func__, __LINE__, audio_status);
	if(audio_engine==NULL){
		ae_info("play error : [music engine null]\n");
	}
	else{
		ae_info("audio_status%d\n",audio_status);
		switch(audio_status){
			case AE_FF:
				ae_info("play FF\n");
				audio_play_cmd(audio_engine,FAST_FORWARD,0);
				break;
			case AE_FB:
				ae_info("play FB\n");
				audio_play_cmd(audio_engine,FAST_BACKWARD,0);
				break;
			case AE_PAUSE:
				ae_info("pause to play\n");
				play_param.mode=TAG_PLAY;
				play_param.param=0;
				audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
				break;
			default:
				ae_info("normal play%d\n",audio_error);
				play_param.mode=NORMAL_PLAY;
				play_param.param=0;
				audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
				break;
		}
		audio_status = AE_PLAY;
	}

	SWFEXT_FUNC_END();	
	
}


int is_audio_stream = 0;
static int ae_set_file(void * handle)
{
	char * file;
	int n;
	int err_set=0;
	int audio_set_setfile_para_ret = -1;
	int i = 0;
	int strLen = 0;
	a_set_file_s aSetfile;
	
	
	SWFEXT_FUNC_BEGIN(handle);

	n = Swfext_GetParamNum();

	is_audio_stream = 0;
	if(n == 0){
		file = AM_CASE_SWF_DIR;
	}
	else{
		file = Swfext_GetString();
	}
	
	strLen = strnlen(file, sizeof(cur_file)+4);
	ae_info("n %d music open file %s,filelen=%d\n",n,file, strLen);
	if(strLen>=sizeof(cur_file))
	{
		printf("Audio set file file name is error!\n");
		err_set = -1;
		goto exit;
	}
	
	strcpy(cur_file,file);
	printf("%s%d:curr_file===%s\n",__func__,__LINE__,file);
	if(audio_engine==NULL){
		ae_info("music engine null\n");
	}
	else{
		printf("audio_status=%d\n", audio_status);
		if((audio_status!=AE_WAIT_STOP_OK)&&(audio_status!=AE_IDLE)&&(audio_status!=AE_STOP)){
			printf("Audio Set File send Stop cmd!\n");
			audio_status = AE_WAIT_STOP_OK;
			audio_play_cmd(audio_engine,STOP,0);
		}

		if(audio_status == AE_WAIT_STOP_OK){
			i = 20;
			do{
				OSSleep(80);
				printf("Audio set file need wait stop!i=%d\n", i);
			}while((audio_status==AE_WAIT_STOP_OK) && --i);

			if(audio_status == AE_WAIT_STOP_OK){
				printf("Audio set file fail![%s]%d\n", __func__, __LINE__);
				err_set = -1;
				goto ___set_file__failed;
			}
			//If audio decodec not STOP, can not close handle.
		}

		audio_set_setfile_para_ret = _audio_set_setfile_para(&file_iocontext,file);
		
		if (audio_set_setfile_para_ret == 2){
			audio_para_t audio_info; 
			is_audio_stream = 1;
			http_get_media_info((void *)&audio_info);
			if (audio_info.file_type == 1){ // file_type = 1 ---> PCM
				///*
				printf("sample_rate = %u\nchannels = %u\nbits_per_sample = %u\n", \
					audio_info.pcm_info.sample_rate, \
					audio_info.pcm_info.channels, \
					audio_info.pcm_info.bits_per_sample);
				//*/
				audio_play_cmd(audio_engine,(unsigned int)SET_PCM_INFO,(unsigned int)&audio_info.pcm_info);
			}
		}
		else if(audio_set_setfile_para_ret == -1){
			err_set = -1;
			goto ___set_file__failed;
		}
		
		a_info_sem = OSSemCreate(0);
		audio_setfile_ok = 0;
		aSetfile.f_io = &file_iocontext;
		aSetfile.pInfo = &media_info;
		err_set= audio_play_cmd(audio_engine,(unsigned int)SET_FILE,(unsigned int)&aSetfile);
		OSSemPend(a_info_sem, 60000);	
		OSSemClose(a_info_sem);
		a_info_sem = NULL;
		
		if (audio_setfile_ok ==0)
		{
			err_set = -1;
			goto ___set_file__failed;
		}
			
	}
	
exit:	
	if(err_set!=0){
		audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
		err_set = player_status.err_no;
	}
	else{
		audio_play_cmd(audio_engine,GET_MEDIA_INFO,(unsigned int)&media_info);	
	}
	
___set_file__failed:
	
	Swfext_PutNumber(err_set);
	if(err_set == 0){
		if(a_stop_ok_sem){
			sem_destroy(a_stop_ok_sem);
			free(a_stop_ok_sem);
			a_stop_ok_sem = NULL;
			printf("\n*******a_stop_ok_sem has not destroyed********** !\n\n");
		}
		a_stop_ok_sem =(sem_t *) malloc(sizeof(sem_t));
		if(a_stop_ok_sem){
			sem_init(a_stop_ok_sem,0,0);
		}
	}
	printf("[%s]%d, audio_setfile_ok=%d, err_set=%d\n", __func__, __LINE__, audio_setfile_ok, err_set);
	SWFEXT_FUNC_END();	
}

int ezcast_ae_set_file(char * file)
{
	int err_set=0;
	int audio_set_setfile_para_ret = -1;
	int i = 0;
	int strLen = 0;
	a_set_file_s aSetfile;
	is_audio_stream = 0;
	
	strLen = strnlen(file, sizeof(cur_file)+4);
	ae_info("music open file %s,filelen=%d\n",file, strLen);
	if(strLen>=sizeof(cur_file))
	{
		printf("Audio set file file name is error!\n");
		err_set = -1;
		goto exit;
	}
	
	strcpy(cur_file,file);
	printf("%s%d:curr_file===%s\n",__func__,__LINE__,file);
	audio_status=ezcast_ae_get_state();//Add By Denny@20141209
	printf("audio_status=%d\n", audio_status);
	if(audio_engine==NULL){
		ae_info("music engine null\n");
	}
	else{
		if((audio_status!=AE_WAIT_STOP_OK)&&(audio_status!=AE_IDLE)&&(audio_status!=AE_STOP)){
			printf("Audio Set File send Stop cmd!\n");
			audio_status = AE_WAIT_STOP_OK;
			audio_play_cmd(audio_engine,STOP,0);
		}

		if(audio_status == AE_WAIT_STOP_OK){
			i = 20;
			do{
				OSSleep(80);
				printf("Audio set file need wait stop!i=%d\n", i);
			}while((audio_status==AE_WAIT_STOP_OK) && --i);

			if(audio_status == AE_WAIT_STOP_OK){
				printf("Audio set file fail![%s]%d\n", __func__, __LINE__);
				err_set = -1;
				goto ___set_file__failed;
			}
			//If audio decodec not STOP, can not close handle.
		}

		audio_set_setfile_para_ret = _audio_set_setfile_para(&file_iocontext,file);
		
		if (audio_set_setfile_para_ret == 2){
			audio_para_t audio_info; 
			is_audio_stream = 1;
			http_get_media_info((void *)&audio_info);
			if (audio_info.file_type == 1){ // file_type = 1 ---> PCM
				///*
				printf("sample_rate = %u\nchannels = %u\nbits_per_sample = %u\n", \
					audio_info.pcm_info.sample_rate, \
					audio_info.pcm_info.channels, \
					audio_info.pcm_info.bits_per_sample);
				//*/
				audio_play_cmd(audio_engine,(unsigned int)SET_PCM_INFO,(unsigned int)&audio_info.pcm_info);
			}
		}
		else if(audio_set_setfile_para_ret == -1){
			err_set = -1;
			goto ___set_file__failed;
		}
		
		a_info_sem = OSSemCreate(0);
		audio_setfile_ok = 0;
		aSetfile.f_io = &file_iocontext;
		aSetfile.pInfo = &media_info;
		err_set= audio_play_cmd(audio_engine,(unsigned int)SET_FILE,(unsigned int)&aSetfile);
		OSSemPend(a_info_sem, 60000);	
		OSSemClose(a_info_sem);
		a_info_sem = NULL;
		
		if (audio_setfile_ok ==0)
		{
			err_set = -1;
			goto ___set_file__failed;
		}
			
	}
	
exit:	
	if(err_set!=0){
		audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
		err_set = player_status.err_no;
	}
	else{
		audio_play_cmd(audio_engine,GET_MEDIA_INFO,(unsigned int)&media_info);	
	}
	
___set_file__failed:
	
	
	if(err_set == 0){
		if(a_stop_ok_sem){
			sem_destroy(a_stop_ok_sem);
			free(a_stop_ok_sem);
			a_stop_ok_sem = NULL;
			printf("\n*******a_stop_ok_sem has not destroyed********** !\n\n");
		}
		a_stop_ok_sem =(sem_t *) malloc(sizeof(sem_t));
		if(a_stop_ok_sem){
			sem_init(a_stop_ok_sem,0,0);
		}
	}
	printf("[%s]%d, audio_setfile_ok=%d, err_set=%d\n", __func__, __LINE__, audio_setfile_ok, err_set);
	return(err_set);
}


int ezcast_ae_stop(void)
{
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(is_audio_stream==1){
		printf("%s,%d\n",__FUNCTION__,__LINE__);
		am_stream_stop(NULL);
		printf("%s,%d\n",__FUNCTION__,__LINE__);
		is_audio_stream = 0;
	}
	if(audio_engine==NULL){
		ae_info(" stop error : [music engine null]\n");
	}
	else{
#if 0
		if((void *)file_iocontext.read == (void *)am_stream_audio_read){
			/**
			* streaming audio, stop the stream first.
			*/
			am_stream_stop(file_iocontext.handle);
		}
#endif
		if((audio_status!=AE_WAIT_STOP_OK)&&(audio_status!=AE_IDLE)&&(audio_status!=AE_STOP)){
			printf("ae_stop send Stop cmd!\n");
			audio_status = AE_WAIT_STOP_OK;
			audio_play_cmd(audio_engine,STOP,0);	
		}

		printf("Audio stop!audio_status=%d\n", audio_status);
	}
	is_audio_stream = 0;
	printf("%s,%d\n",__FUNCTION__,__LINE__);
}

static int ae_stop(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	if(is_audio_stream==1){
		printf("%s,%d\n",__FUNCTION__,__LINE__);
		am_stream_stop(NULL);
		printf("%s,%d\n",__FUNCTION__,__LINE__);
		is_audio_stream = 0;
	}
	if(audio_engine==NULL){
		ae_info(" stop error : [music engine null]\n");
	}
	else{
#if 0
		if((void *)file_iocontext.read == (void *)am_stream_audio_read){
			/**
			* streaming audio, stop the stream first.
			*/
			am_stream_stop(file_iocontext.handle);
		}
#endif
		if((audio_status!=AE_WAIT_STOP_OK)&&(audio_status!=AE_IDLE)&&(audio_status!=AE_STOP)){
			printf("ae_stop send Stop cmd!\n");
			audio_status = AE_WAIT_STOP_OK;
			audio_play_cmd(audio_engine,STOP,0);	
		}

		printf("Audio stop!audio_status=%d\n", audio_status);
	}
	is_audio_stream = 0;
	printf("%s,%d\n",__FUNCTION__,__LINE__);
	SWFEXT_FUNC_END();	
}

static int ae_pause(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	
	if(audio_engine==NULL){
		ae_info(" pause error : [music engine null]\n");
	}
	else{
		ae_info("~~~~~~~~~~~~~Audio pause\n");
		audio_status = AE_PAUSE;
		audio_play_cmd(audio_engine,pause,0);
	}
	
	SWFEXT_FUNC_END();	
}


static int ae_resume(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	static play_param_t play_param;
	if(audio_engine==NULL){
		ae_info(" resume error : [music engine null]\n");	
	}
	else{
		ae_info("~~~~~~~~~~~~pause to play\n");
		if(seek_resume==0){
			ae_info("~~~~~~~~~~~~normal pause to play\n");
			play_param.mode=TAG_PLAY;
			play_param.param=0;
		}
		else if(seek_resume==1){
			ae_info("~~~~~~~~~~~~seek pause to play\n");
			play_param.mode=SEEK_PLAY;
			play_param.param=seekplay_time;
			seek_resume=0;
		}
		audio_status = AE_PLAY;
		audio_play_cmd(audio_engine,PLAY,(unsigned int)&play_param);
		
	}
	SWFEXT_FUNC_END();	
}


static int ae_fast_forward(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);
	
	if(audio_engine==NULL){
		ae_info(" fast forward error : [music engine null]\n");
	}
	else{
		switch(audio_status){
			case AE_FF:
				ae_info("cancel FF\n");
				audio_status = AE_PLAY;
				audio_play_cmd(audio_engine,CANCEL_FF,0);
			break;
			
			default:
				ae_info("play FF\n");
				audio_status = AE_FF;
				audio_play_cmd(audio_engine,FAST_FORWARD,0);
			break;
		}
	}
	
	SWFEXT_FUNC_END();	
}

static int ae_fast_backward(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);

	if(audio_engine==NULL){
		ae_info(" fast backward error : [music engine null]\n");
	}
	else{
		switch(audio_status){
			case AE_FB:
				ae_info("cancel FB\n");
				audio_status = AE_PLAY;
				audio_play_cmd(audio_engine,CANCEL_FB,0);
			break;
			
			default:
				ae_info("play FB\n");
				audio_status = AE_FB;
				audio_play_cmd(audio_engine,FAST_BACKWARD,0);
			break;
		}
	}
	
	SWFEXT_FUNC_END();	
}

static int ae_seek_play(void* handle)
{
	static play_param_t play_param;
	int total_time;
	SWFEXT_FUNC_BEGIN(handle);
	seekplay_time = Swfext_GetNumber();

	if(audio_engine==NULL){
		ae_info(" fast backward error : [music engine null]\n");
	}
	else{
		switch(audio_status){
			case AE_FB:
				ae_info("cancel FB first\n");
				audio_status = AE_PLAY;
				audio_play_cmd(audio_engine,CANCEL_FB,0);
				break;
			
			case AE_FF:
				ae_info("cancel FF first\n");
				audio_status = AE_PLAY;
				audio_play_cmd(audio_engine,CANCEL_FF,0);
				break;

			case AE_PAUSE:
				ae_info("now status is pause\n");
				seek_resume = 1;
				break;
			default:
				goto seek_play;
				break;
			}
		seek_play:		
			ae_info("seek play\n");
			total_time = media_info.total_time;
			if(seekplay_time>=total_time)
				seekplay_time = total_time;
			else if(seekplay_time<0)
				seekplay_time = 0;
			ae_info("seek time is %d\n",seekplay_time);
			if(!seek_resume)
				audio_status = AE_PLAY;
			audio_play_cmd(audio_engine,SET_SEEK_TIME,seekplay_time);
			printf("[%s]%d,audio_status=%d\n", __func__, __LINE__, audio_status);
	}
	
	SWFEXT_FUNC_END();	

}

int ezcast_ae_get_state(void)
{
	int rtn = -1;
	static int prevState = -1;	
	
	if(0==audio_setfile_ok){
		return(audio_status);
	}
	
	if(audio_engine!=NULL){
		audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
		rtn = player_status.status;
#if 0		
#ifdef  MODULE_CONFIG_DLNA	
		if(rtn!=prevState)
			dlna_dmr_cb(&rtn);

		prevState = player_status.status;
#endif
#endif
	}
	
	return(rtn);
}

static int ae_get_state(void * handle)
{
	int rtn = -1;
	static int prevState = -1;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(0==audio_setfile_ok){
		Swfext_PutNumber(audio_status);
		goto end;
	}
	
	if(audio_engine!=NULL){
		audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
		rtn = player_status.status;
		
#ifdef  MODULE_CONFIG_DLNA	
		if(rtn!=prevState)
			dlna_dmr_cb(&rtn);

		prevState = player_status.status;
#endif
	}
	
	Swfext_PutNumber(rtn);
end:
	SWFEXT_FUNC_END();	
}


static int ae_get_total_time(void * handle)
{
	int total_time = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	if(audio_engine==NULL){
		total_time = 0;
	}
	else{
		total_time = media_info.total_time;
		printf("get totaltime %d\n",total_time);
	}
	Swfext_PutNumber(total_time);
	
	SWFEXT_FUNC_END();	
}


static int ae_get_current_time(void * handle)
{
	int crrent_time = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	
	if(audio_engine==NULL){
		crrent_time = 0;
	}
	else{
		if(seek_resume==0){
			audio_play_cmd(audio_engine,GET_PLAYER_STATUS,(unsigned int)&player_status);
			crrent_time = player_status.cur_time;
		}
		else if(seek_resume==1){
			crrent_time =seekplay_time;
		}
	}
	Swfext_PutNumber(crrent_time);
	
	SWFEXT_FUNC_END();	
}


static int ae_get_singer(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get singer!!! */
	Swfext_PutString("unknown singer");

	SWFEXT_FUNC_END();	
}


static int ae_get_album(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get album!!! */
	Swfext_PutString("unknown album");

	SWFEXT_FUNC_END();		
}



static int ae_get_file_extention(void * handle)
{

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get file extension!!! */
	Swfext_PutString("mp3");

	SWFEXT_FUNC_END();	
}




static int ae_get_sample_rate(void * handle)
{
	int sample_rate = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	if(audio_engine==NULL){
		sample_rate = 0;
	}
	else{
		sample_rate = media_info.sample_rate;
		printf("get sample_rate %d\n",sample_rate);
	}
	Swfext_PutNumber(sample_rate);
	
	SWFEXT_FUNC_END();	
}

static int ae_get_bitrate(void * handle)
{
	int bitrate = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	if(audio_engine==NULL){
		bitrate = 0;
	}
	else{
		bitrate = media_info.bitrate;
		printf("get bitrate %d\n",bitrate);
	}
	Swfext_PutNumber(bitrate);
	
	SWFEXT_FUNC_END();	
}

static int ae_get_channels(void * handle)
{
	int channels = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	if(audio_engine==NULL){
		channels = 0;
	}
	else{
		channels = media_info.channels;
		printf("get channels %d\n",channels);
	}
	Swfext_PutNumber(channels);
	
	SWFEXT_FUNC_END();	
}

static int ae_get_lyrics(void * handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get lyrics!!! */
	Swfext_PutString("unknown lyrics");

	SWFEXT_FUNC_END();	
}


static int ae_attach_picture(void * handle)
{
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: not used currently!!! */
	
	SWFEXT_FUNC_END();
}

static int ae_get_effect(void * handle)
{
	int effect=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	/** FIXME: get effect!!! */
	Swfext_PutNumber(effect);
	
	SWFEXT_FUNC_END();
}

static int ae_set_effect(void * handle)
{
	int effect;

	SWFEXT_FUNC_BEGIN(handle);

	effect = Swfext_GetNumber();
	if(audio_engine != NULL){
		/** FIXME: set effect!!! */
	}
	
	SWFEXT_FUNC_END();
}


static int ae_get_play_mode(void * handle)
{
	int playmode=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: get play mode!!! */
	playmode = get_music_repeat_mode();
	printf("[%s %d] playmode =================== %d\n",__FUNCTION__,__LINE__,playmode);
	Swfext_PutNumber(playmode);
	
	SWFEXT_FUNC_END();
}

static int ae_set_play_mode(void * handle)
{
	int playmode;

	SWFEXT_FUNC_BEGIN(handle);

	/** FIXME: set play mode!!! */
	playmode = Swfext_GetNumber();
	printf("[%s %d] playmode =================== %d\n",__FUNCTION__,__LINE__,playmode);
	set_music_repeat_mode(playmode);
	
	SWFEXT_FUNC_END();
}
#endif

#if 1

/**
@brief save the pic extracted from audio file, this function is just for test
**/
static int _ae_save_id3_pic(MUSIC_INFO*music_info)
{
	FILE* fp=NULL;
	int fd=0;
	int len_write=0;
	fp = fopen("id3_pic.out","wb+");
	ae_info("type===%s",music_info->image_type);
	if(fp!=NULL){
		len_write = fwrite(music_info->pic.content,1,music_info->pic.length,fp);
		if(len_write!=music_info->pic.length){
			ae_err("Write Error!");
		}
		ae_info("write=%d,length=%d",len_write,music_info->pic.length);
		fflush((FILE *)fp);
		fd = fileno((FILE *)fp);
		if(fsync(fd)==-1){
			printf("%s,%d: Fflush Error=%d!\n",__FILE__,__LINE__,errno);
			return -1;
		}
		fclose(fp);
		ae_info("Save OK");
	}
	else
		ae_err("Create Temp File Failed!");
	return 0;

}


/**
@brief save the id3 info to outbuf which is specified
@param[in] music_info	: the struct of MUSIC_INFO where the music info had been stored
@param[in] mode		: see ID3_AUTHOR etc
@param[in] outbuf		: where to save the info
@param[in] buflen		: the length of the outbuf
@return
	- 0	: succeed
	- -2	: the space of the outbuf is too small
	- -1	: mode is not support
**/
static char _ae_save_id3info(MUSIC_INFO*music_info,MUSIC_ID3_INFO_E mode,char *outbuf,int buflen)
{
	char rtn=0;
	switch(mode){
		case ID3_AUTHOR:
			if(music_info->author.content!=NULL){
				if(music_info->author.length<=buflen){
					memcpy(outbuf,music_info->author.content,music_info->author.length);
					rtn = music_info->author.length;
				}
				else
					rtn = -2;
			}
			break;
		case ID3_COMPOSER:
			if(music_info->composer.content!=NULL){
				if(music_info->composer.length<=buflen){
					memcpy(outbuf,music_info->composer.content,music_info->composer.length);
					rtn = music_info->composer.length;
				}
				else
					rtn = -2;
			}
			break;
		case ID3_ALBUM:
			if(music_info->album.content!=NULL){
				if(music_info->album.length<=buflen){
					memcpy(outbuf,music_info->album.content,music_info->album.length);
					rtn = music_info->album.length;
				}
				else
					rtn = -2;
			}
			break;
		case ID3_GENRE:
			if(music_info->genre.content!=NULL){
				if(music_info->genre.length<=buflen){
					memcpy(outbuf,music_info->genre.content,music_info->genre.length);
					rtn = music_info->genre.length;
				}
				else
					rtn = -2;
			}
			break;
		case ID3_YEAR:
			if(music_info->year.content!=NULL){
				if(music_info->year.length<=buflen){
					memcpy(outbuf,music_info->year.content,music_info->year.length);
					rtn = music_info->year.length;
				}
				else
					rtn = -2;
			}
			break;
		case ID3_PIC_WH:
			if(music_info->pic.content!=NULL){
				if(music_info->pic.length>0){
					//_ae_save_id3_pic(music_info);
				}
			}
			break;
		default:
			printf("%s,%d:Carzy Mode Error!",__FILE__,__LINE__);
			rtn=-1;
			break;
	}
	if(rtn==-2)
		ae_info("%s,%d:Sorry The tmp buflen is short!\n",__FILE__,__LINE__);
	return rtn;
}

/**
@brief map the MUSIC_ID3_INFO_E to the mode which is supported by the id3 parser
@param[in] cmd	: see MUSIC_ID3_INFO_E
@return see MASK_AUTHOR etc
**/
static int _ae_map_id3cmd(MUSIC_ID3_INFO_E cmd)
{
	int mode=0;
	switch(cmd){
		case ID3_AUTHOR:
			mode = (int)MASK_AUTHOR;
			break;
		case ID3_COMPOSER:	
			mode = (int)MASK_COMPOSER;
			break;
		case ID3_PIC_WH:
		case ID3_ALBUM:
			mode = (int)MASK_ALBUM;
			break;
		case ID3_GENRE:
			mode = (int)MASK_GENRE;
			break;
		case ID3_YEAR:
			mode = (int)MASK_YEAR;
			break;
		default:
			ae_err("Crazy: cmd=%d is out of range",cmd);
			break;
	}
	return mode;
}

static char* _ae_get_id3_pic_parser_buf(int format,int w,int h)
{
	char * buffer=NULL;
	if(format==SWF_BMP_FMT_YUV422){
		buffer = (char*)SWF_Malloc(w*h*2);
		if(buffer==NULL){
			ae_info("Malloc Pic Parser Buf Failed!");
		}
	}
	else
		ae_info("Sorry, The Format is not support!");
	return buffer;
}

static void _ae_free_id3_pic_parser_buf(char*buf)
{
	if(buf!=NULL){
		SWF_Free(buf);
		buf = NULL;
	}
}

static int _ae_set_default_fops(io_layer_t  *io_layer)
{
	io_layer->img_fread = fui_memfs_read;
	io_layer->img_fseek_set = fui_memfs_seek_set;
	io_layer->img_fseek_cur = fui_memfs_seek_cur;
	io_layer->img_fseek_end = fui_memfs_seek_end;
	io_layer->img_ftell = fui_memfs_tell;
	io_layer->img_malloc = (void*)SWF_Malloc;
	io_layer->img_realloc =NULL;
	io_layer->img_free = SWF_Free;
	io_layer->lp_get_bus_addr = fui_get_bus_address;
	return 0;
}

static int _ae_get_id3_pic_parser_status(int id)
{
	return id3_pic_parser_status;
}

static unsigned char* _ae_parser_id3_pic(MUSIC_INFO *musicinfo,void* target, int w, int h)
{
	io_layer_t io_layer;
	char is_pic_support=0;
	img_info_t img_info;
	int ret=0;
	io_layer.handle = fui_memfs_open(musicinfo->pic.content,musicinfo->pic.length);
	_ae_set_default_fops(&io_layer);

	/** check whether the pic is supported**/
	if(strcmp(musicinfo->image_type,"jpeg")==0){
		is_pic_support = 1;
	}
	else if(strcmp(musicinfo->image_type,"bmp")==0){
		is_pic_support = 1;
	}
	else
		is_pic_support = 0;

	/**parser the pic**/
	if(is_pic_support){
		img_info.file_format = FILE_FORMAT_JPEG;
		img_info.format = LP_FMT_YCBCR_4_2_2_INTERLEAVED;
		img_info.out_pic_width = w;
		img_info.out_pic_height = h;
		img_info.out_pic_pos_x = 0;
		img_info.out_pic_pos_y =0 ;
		img_info.buf_width = w;
		img_info.buf_height = h;
		img_info.buf_info.buf_y.buf=(unsigned char*) _ae_get_id3_pic_parser_buf((int)SWF_BMP_FMT_YUV422,w,h);
		if(img_info.buf_info.buf_y.buf==NULL)
			goto PARSER_ID3_PIC_END;
		img_info.buf_info.buf_y.size = w*h*2;
		img_info.buf_info.buf_y.bus_addr = fui_get_bus_address((unsigned long)img_info.buf_info.buf_y.buf);

		img_info.buf_info.buf_uv.buf= NULL;
		img_info.buf_info.buf_v.buf= NULL;
		id3_pic_parser_status = 0;
		SWF_AttachBitmap(target,(unsigned int*)img_info.buf_info.buf_y.buf,w,h,w,h,w,SWF_BMP_FMT_YUV422,_ae_get_id3_pic_parser_status);

		loadimg_dec_open();
		ret = loadimg_show_pic(&io_layer,&img_info);
		loadimg_dec_close();

		ae_info("ShowPic ret=%d\n",ret);
		id3_pic_parser_status = 1;
	}
	else
		img_info.buf_info.buf_y.buf = NULL;

PARSER_ID3_PIC_END:
	if(io_layer.handle)
		fui_memfs_close(io_layer.handle);
	return img_info.buf_info.buf_y.buf;
}

static unsigned char* _ae_id3_attah_pic(void*target,int w,int h,char* audiofilepath)
{
	unsigned char *parser_buf=NULL;
	MUSIC_INFO musicinfo;

	FILE *fhandle=NULL;
	fhandle=fopen(audiofilepath,"rb");
	if(fhandle==NULL){
		goto _ATTACH_PIC_END;
	}
	get_audio_info(fhandle,&musicinfo,(int)MASK_ALBUM);	//get the id3 pic and album info
	if(musicinfo.pic.content!=NULL){ //check whether the pic is exist
		if(musicinfo.pic.length>0){
			parser_buf = _ae_parser_id3_pic(&musicinfo,target,w,h);
		}
	}
	id3_free(&musicinfo);
		
_ATTACH_PIC_END:
	if(fhandle!=NULL)
		fclose(fhandle);
	ae_info("ParserBuf=0x%x",parser_buf);
	return parser_buf;
}
#endif

#ifdef MODULE_CONFIG_AUDIO
static int ae_get_id3_info(void *handle)
{
	char *filename=NULL;
	int id3_len=0;
	MUSIC_ID3_INFO_E cmd=0;
	FILE *fhandle=0;
	MUSIC_INFO musicinfo;
	char *tmpbuf = lyric_tmpbuf;
	int temp_len=LYRIC_BUFFER;
	SWFEXT_FUNC_BEGIN(handle);
	filename = Swfext_GetString();
	cmd = (MUSIC_ID3_INFO_E)Swfext_GetNumber();
	memset(music_id3_buffer,0,ID3_BUFFER_LEN);
	memset(tmpbuf,0,LYRIC_BUFFER);
	fhandle=fopen(filename,"rb");
	ae_info("FileName=%s,cmd=%d",filename,cmd);
	if(fhandle){
		int mode = _ae_map_id3cmd(cmd);
		get_audio_info(fhandle,&musicinfo,mode);
		id3_len = _ae_save_id3info(&musicinfo,cmd,music_id3_buffer,ID3_BUFFER_LEN-1);
		if(id3_len>0){
			_ae_change_buf_format(music_id3_buffer,&id3_len,LAN_UTF16LE,tmpbuf,&temp_len);
		}
		id3_free(&musicinfo);
		fclose(fhandle);
	}
	else
		printf("%s,%d:Open file Error!\n",__FILE__,__LINE__);
	_ae_print_test(tmpbuf,LYRIC_BUFFER-temp_len,16);
	Swfext_PutString(tmpbuf);
	SWFEXT_FUNC_END();
}

static int ae_attach_id3_pic(void *handle)
{
	unsigned char*tmpbufaddr=NULL;
	char *audiofilepath=NULL;
	void *target=NULL;
	int w=0,h=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	audiofilepath = Swfext_GetString();
	target   = Swfext_GetObject();
	w = Swfext_GetNumber();
	h = Swfext_GetNumber();

	tmpbufaddr= _ae_id3_attah_pic(target,w,h,audiofilepath);
	
	Swfext_PutNumber((int)tmpbufaddr);
	//ae_info("PutNumber=0x%x",tmpbufaddr);
	SWFEXT_FUNC_END();
}

static int ae_detach_id3_pic(void*handle)
{
	int parser_buf=0;
	SWFEXT_FUNC_BEGIN(handle);
	parser_buf =  Swfext_GetNumber();
	if(parser_buf){
		ae_info("ParserBuf===0x%x\n",parser_buf);
		_ae_free_id3_pic_parser_buf((char*)parser_buf);
	}

	SWFEXT_FUNC_END();
}

#endif

#if 1
//////////////与歌词相关函数定义////////////
#ifdef MODULE_CONFIG_LYRICS
static int ae_open_lyric_file(void *handle)
{
	char *file;
	int rtn;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	file=Swfext_GetString();
	Lyric=openLyric(file);
	printf("%s,%d:Open Lyric File=%s\n",__FILE__,__LINE__,file);
	rtn = analyseLyric(&Lyric);
	if(rtn==0){
		closeLyric(&Lyric);
	}
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_close_lyric_file(void *handle)
{
	int lyrichandle;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	lyrichandle = Swfext_GetNumber();
	if(lyrichandle!=0){
		closeLyric(&Lyric);
	}
	
	SWFEXT_FUNC_END();
}

static int ae_get_tag_time(void *handle)
{
	int timetag;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	timetag = Tagtime.min*60+Tagtime.sec;
	Swfext_PutNumber(timetag);
	
	SWFEXT_FUNC_END();
}



static int ae_get_lyric(void *handle)
{
	int tmpbuflen=LYRIC_BUFFER;
	
	SWFEXT_FUNC_BEGIN(handle);
	memset(lyric_tmpbuf,0,LYRIC_BUFFER);
	if(Lyric.filetype == LYRIC_MBCS){
		/** FIXME: please add code convert here*/
		int change_ok=0;
		change_ok = _ae_change_buf_format(Buf,&lyricbytes,LAN_GB2312,lyric_tmpbuf,&tmpbuflen);
		//_ae_print_test(lyric_tmpbuf,tmpbuflen,16);

	}
	else{
		printf("Erro Lyric txt type");
	}
	
	Swfext_PutString(lyric_tmpbuf);
	
	SWFEXT_FUNC_END();
}

static int ae_get_lyric_next_line(void *handle)
{
	char rtn;
	
	SWFEXT_FUNC_BEGIN(handle);
	Tagtime.min=0;
	Tagtime.sec=0;
	Tagtime.millisec=0;
	memset(Buf,0,(unsigned int)LYRIC_BUFFER);
	rtn=getNextLine(&Lyric, Buf, LYRIC_BUFFER, & Tagtime);
	lyricbytes = rtn;
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_get_lyric_info(void *handle)
{
	char tagindex=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	tagindex =  Swfext_GetNumber();
	switch(tagindex){
		case LYRIC_TAG_AL:
			Swfext_PutString(Lyric.album);
			break;
		case LYRIC_TAG_AR:
			Swfext_PutString(Lyric.ar);
			break;
		case LYRIC_TAG_BY:
			Swfext_PutString(Lyric.by);
			break;
		case LYRIC_TAG_OFFSET:
			Swfext_PutString(Lyric.offset);
			break;
		case LYRIC_TAG_RE:
			Swfext_PutString(Lyric.re);
			break;
		case LYRIC_TAG_TI:
			Swfext_PutString(Lyric.ti);
			break;
		case LYRIC_TAG_VE:
			Swfext_PutString(Lyric.ve);
			break;
		default:
			Swfext_PutString("unknown");
			break;
	}
	
	SWFEXT_FUNC_END();
}

static int ae_change_lyric_line(void *handle)
{
	lyric_tag_time_t tagtime;
	char rtn;
	int time_change=0;
	SWFEXT_FUNC_BEGIN(handle);
	time_change = Swfext_GetNumber();
	if(audio_engine==NULL){
		/** do nothing */
	}
	else{
		/** FIXME: get current playback time */
		tagtime.min = time_change/60;
		tagtime.sec = time_change%60;
		tagtime.millisec = 0;
		rtn = changLine(&Lyric,&tagtime);
		Swfext_PutNumber(rtn);
	}
	
	SWFEXT_FUNC_END();
}
#else
static int ae_open_lyric_file(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_close_lyric_file(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_get_tag_time(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}



static int ae_get_lyric(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_get_lyric_next_line(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_get_lyric_info(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

static int ae_change_lyric_line(void *handle)
{
	int rtn=0;
	
	SWFEXT_FUNC_BEGIN(handle);

	printf("open the module config first, do nothing\n");
	
	Swfext_PutNumber(rtn);
	
	SWFEXT_FUNC_END();
}

#endif
#endif

int swfext_music_register(void)
{
	#ifdef MODULE_CONFIG_AUDIO
	SWFEXT_REGISTER("ae_Open", ae_open);
	SWFEXT_REGISTER("ae_Close", ae_close);
	SWFEXT_REGISTER("ae_SetFile", ae_set_file);
	SWFEXT_REGISTER("ae_Play", ae_play);
	SWFEXT_REGISTER("ae_Stop", ae_stop);
	SWFEXT_REGISTER("ae_Pause", ae_pause);
	SWFEXT_REGISTER("ae_Resume", ae_resume);
	SWFEXT_REGISTER("ae_FF", ae_fast_forward);
	SWFEXT_REGISTER("ae_FB", ae_fast_backward);
	SWFEXT_REGISTER("ae_seekplay", ae_seek_play);
	SWFEXT_REGISTER("ae_State", ae_get_state);
	SWFEXT_REGISTER("ae_TotalTime", ae_get_total_time);
	SWFEXT_REGISTER("ae_CurTime", ae_get_current_time);
	SWFEXT_REGISTER("ae_Singer", ae_get_singer);
	SWFEXT_REGISTER("ae_Album", ae_get_album);
	SWFEXT_REGISTER("ae_Extention", ae_get_file_extention);
	SWFEXT_REGISTER("ae_SampleRate", ae_get_sample_rate);
	SWFEXT_REGISTER("ae_BitRate", ae_get_bitrate);
	SWFEXT_REGISTER("ae_Channels", ae_get_channels);
	SWFEXT_REGISTER("ae_Picture", ae_attach_picture);	
	SWFEXT_REGISTER("ae_GetEffect", ae_get_effect);
	SWFEXT_REGISTER("ae_SetEffect", ae_set_effect);
	SWFEXT_REGISTER("ae_GetPlayMode", ae_get_play_mode);
	SWFEXT_REGISTER("ae_SetPlayMode", ae_set_play_mode);

	SWFEXT_REGISTER("ae_GetID3Info",ae_get_id3_info);
	SWFEXT_REGISTER("ae_AttachID3Pic",ae_attach_id3_pic);
	SWFEXT_REGISTER("ae_DetachID3Pic",ae_detach_id3_pic);
	#endif

	#ifdef MODULE_CONFIG_LYRICS
	/** Lyric funciton */
	SWFEXT_REGISTER("ae_openLyricFile", ae_open_lyric_file);
	SWFEXT_REGISTER("ae_closeLyricFile", ae_close_lyric_file);
	SWFEXT_REGISTER("ae_getTagTime", ae_get_tag_time);
	SWFEXT_REGISTER("ae_getLyric", ae_get_lyric);
	SWFEXT_REGISTER("ae_getLyricNextLine", ae_get_lyric_next_line);
	SWFEXT_REGISTER("ae_getLyricInfo", ae_get_lyric_info);
	SWFEXT_REGISTER("ae_changeLyricLine", ae_change_lyric_line);
	#endif

	return 0;
	
}
