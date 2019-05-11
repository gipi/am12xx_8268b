#ifdef  MODULE_CONFIG_DLNA  //module config

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <semaphore.h>
#include <fcntl.h>
#include "swf_ext.h"
#include "system_info.h"
#include "sys_vram.h"
#include "apps_vram.h"
#include "dlna_dmr.h"
 

typedef enum
{
	UT_Unknown = 0,
	UT_Normal,
	UT_PlaySingle,
	UT_PlayList_S,
	UT_PlayList_V,
	UT_PlayContainer
} URIType;

static pthread_t dmrstart_id;

/* Application defaults */
#define INTRO_MILLISECONDS		5000
#define DEFAULT_DURATION		10000
#define NUMBER_OF_THREADS		3
#define IMAGE_PLAY_MILLISECONDS	0

#define			DEFCMD				0x00
#define			PLAY				0x01
#define			STOP				0x02
#define			PAUSE				0x03

#define			REPLAY_ONE			0x04
#define			REPLAY_ALL			0x05

#define			SET_SEEK_TIME			0x04
#define			FAST_FORWARD			0x06	
#define			FAST_BACKWARD			0x07
#define			CANCEL_FF			0x08
#define			CANCEL_FB			0x09
#define			DLNA_DMR_CLOSE			0x0A
#define			GET_MEDIA_INFO			0x13    	
#define			GET_PLAYER_STATUS		0x14    	//                                                                  	

#define DMR_ENGINE_DBG(fmt, arg...) //printf("DMR_ENGINE_DBG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

extern int get_volume_for_dmr();
extern int set_volume_for_dmr(int vol);
extern int get_mute_for_dmr();
extern int set_mute_for_dmr(int mute_flag);

char get_rank_uuid_part(){
	char rank_char = 0;
	
	char rank_type =rand()%(1-0+1)+0;
	//printf("rank_type ======== %d\n",rank_type);
	
	if(rank_type == 0){
		
		rank_char = rand()%('f' - 'a' + 1) + 'a';
	}

	else{
		
		rank_char = rand()%('9' - '0' + 1) + '0';
	}
	
	//printf("rank_char ======== %c\n",rank_char);	
	return rank_char;
}


static char *dmr_uuid_generate( ){
	char *dmr_uuid = malloc(36+1);
	int index = 0;
	if(dmr_uuid == NULL){
		perror("malloc failed \n");
		return NULL;
	}
	srand(time(NULL));
	memset(dmr_uuid,0,37);	
	for(index = 0 ; index <36 ; index++){
		if(index == 8 || index == 13 || index == 18 || index == 23 )
			dmr_uuid[index] = '-';
		else{
			dmr_uuid[index] = get_rank_uuid_part();		
		}
	}
	printf("dmr_uuid ========== %s\n",dmr_uuid);
	return dmr_uuid;
}

dmr_start_arg_t * dmr_start_arg_p;

extern int is_video_stream;
extern int is_audio_stream;

long get_total_time_for_dmr(){
	DMR_ENGINE_DBG("is_video_stream ======== %d\n",is_video_stream);
	DMR_ENGINE_DBG("is_audio_stream ======== %d\n",is_audio_stream);
	
	if(is_video_stream)
		return get_video_total_time_for_dmr();
	else if(is_audio_stream)
		return ae_get_total_time_for_dmr();
	else
		return -1;
}

long get_play_time_for_dmr(){	
	DMR_ENGINE_DBG("is_video_stream ======== %d\n",is_video_stream);
	DMR_ENGINE_DBG("is_audio_stream ======== %d\n",is_audio_stream);
	
	if(is_video_stream)
		return get_video_play_time_for_dmr();

	else if(is_audio_stream)
		return ae_get_current_time_for_dmr();
	else
		return -1;
}

int seek_play_for_dmr(long seek_time){
	if(is_video_stream)
		return seek_video_play_for_dmr(seek_time);

	else if(is_audio_stream)
		return ae_seek_play_for_dmr();
	else
		return -1;
}



int dmr_is_running = 0;
static int dlna_dmr_start_work(void*handle)
{
	int ret= 0;
	char *dmr_uuid = NULL; 
	struct sysconf_param sys_info;
	dmr_start_arg_t dmr_start_arg ;


	
	SWFEXT_FUNC_BEGIN(handle);
	
	
	memset(dmr_start_arg.dmr_title,0,256);
	memset(dmr_start_arg.dmr_uuid,0,37);
	_get_env_data(&sys_info);

	

	if(strlen(sys_info.dmr_uuid) == 0){
		strncpy(sys_info.dmr_uuid,dmr_uuid_generate(),37);
		if(strlen(sys_info.dmr_uuid) != strlen("a757463e-bc26-6c76-17da-1c8f2bd3ad13") ){
			strncpy(sys_info.dmr_uuid ,"a757463e-bc26-6c76-17da-1c8f2bd3ad13",strlen("a757463e-bc26-6c76-17da-1c8f2bd3ad13"));
		}
		_save_env_data(&sys_info);
		_store_env_data();
	}
	
	memcpy(dmr_start_arg.dmr_title,sys_info.dmr_title,256);
	memcpy(dmr_start_arg.dmr_uuid,sys_info.dmr_uuid,37);
	dmr_start_arg_p = &dmr_start_arg ;
	dmr_start_arg_p->get_player_total_time = &get_total_time_for_dmr;
	dmr_start_arg_p->get_player_play_time = &get_play_time_for_dmr;
	dmr_start_arg_p->seek_play_for_dmr = &seek_play_for_dmr;
	dmr_start_arg_p->get_volume_for_dmr = &get_volume_for_dmr;
	dmr_start_arg_p->set_volume_for_dmr = &set_volume_for_dmr;
	dmr_start_arg_p->get_mute_for_dmr = &get_mute_for_dmr;
	dmr_start_arg_p->set_mute_for_dmr = &set_mute_for_dmr;
	

	dlna_dmr_start_run((void *)dmr_start_arg_p);

	
#if 0	
	if(strlen(sys_info.dmr_title) != 0){
		
		dlna_dmr_start_run(sys_info.dmr_title);
	}
	else{
		dlna_dmr_start_run(NULL);
	}
	
#endif


	
	dmr_is_running = 1;
	Swfext_PutNumber(ret);
	
	SWFEXT_FUNC_END();	
}

static int dlna_dmr_stop(void*handle)
{
	SWFEXT_FUNC_BEGIN(handle);
	
	dlna_dmr_exit(NULL);
	
	SWFEXT_FUNC_END();	
}

static int dlna_dmr_get_cmd(void*handle)
{	
	int cmd=0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	cmd=dlna_dmr_cmd(NULL);
	Swfext_PutNumber(cmd);
	
	SWFEXT_FUNC_END();		
}
#define test_dmr_stream_video ;
#ifdef test_dmr_stream_video
#define TEST_VIDEO_PATH   "/mnt/udisk/testvideo"
#define TEST_VIDEO_BUFFER_SIZE 1024*16
static int video_dmr_stream_test(char *url){
	
	struct wrap_stream_t *s_cfg=NULL;
	char  *pch=NULL;
	long long totalsize = 1000;
	long long tempsize = 10;
	long long sumtempsize = 100;
	int  cfgfd,rtn=0;
	
	s_cfg=am_ota_open(url,NULL,&rtn);
	printf("%s,%d,url ===== %s\n",__FUNCTION__,__LINE__,url);
	if(s_cfg){
		totalsize = am_ota_get_filesize(s_cfg);
		printf("totalsize ========== %lld\n",totalsize);
		while(1){
			if(sumtempsize == totalsize)
				break;
			
			pch=(char *)malloc(TEST_VIDEO_BUFFER_SIZE * sizeof(char));
			if(pch){
				tempsize = am_ota_read(s_cfg,pch,TEST_VIDEO_BUFFER_SIZE);
				
				printf("tempsize ========== %lld\n",tempsize);
				if(tempsize > 0){
					cfgfd=open(TEST_VIDEO_PATH,O_CREAT|O_RDWR|O_APPEND,0x777);
					if(-1 != cfgfd){
						write(cfgfd,pch,tempsize);
						close(cfgfd);
					}
					else{
						printf("download config file failed!\n");
					}
				}
				else
					break;
				sumtempsize = sumtempsize + tempsize;
				
				printf("sumtempsize ========== %lld\n",sumtempsize);
				free(pch);
			}
			
		}
	}
	
	am_ota_close(s_cfg);
	return 0;
}

static int 	video_dmr_curl_test(char *url){
	printf("********************* tset curl begin *************************\n");
	char callbuf[2048] = {0};
	memset(callbuf,0,2048);
	sprintf(callbuf,"curl -o /mnt/udisk/test.mp4 %s",url);
	system(callbuf);
	printf("********************* tset curl end *************************\n");
}
#endif
static int dlna_dmr_get_uri(void*handle)
{	
	char* uri;
	int file_format;
	SWFEXT_FUNC_BEGIN(handle);
	
	uri = (char *)dlna_dmr_acquire_uri(NULL);
#ifdef test_dmr_stream_video

	//video_dmr_stream_test(uri);
	//video_dmr_curl_test(uri);
#endif
	Swfext_PutString(uri);
	
	SWFEXT_FUNC_END();		
}
static int dlna_dmr_set_status(void*handle)
{
	SWFEXT_FUNC_BEGIN(handle);

	dlna_dmr_configure_status(NULL);
	
	SWFEXT_FUNC_END();		
}
static int dlna_dmr_get_media_type(void* handle)
{
	int MediaType = 0;
	
	SWFEXT_FUNC_BEGIN(handle);
	
	MediaType = dlna_dmr_check_media_type(NULL);
	Swfext_PutNumber(MediaType);
	
	SWFEXT_FUNC_END();		

}
static int dlna_dmr_setting_title(void *handle){
	
	char *dmr_title = NULL; 
	struct sysconf_param sys_info;
	
	SWFEXT_FUNC_BEGIN(handle);
	dmr_title = Swfext_GetString();
	
	_get_env_data(&sys_info);
	//printf("dmr_title ======= %s \n",dmr_title);
	strncpy(sys_info.dmr_title,dmr_title,strlen(dmr_title));
	//printf("sys_info.dmr_title ======= %s \n",sys_info.dmr_title);
	_save_env_data(&sys_info);
	_store_env_data();
	SWFEXT_FUNC_END();		
}
static int dlna_dmr_get_media_title(void *handle){
#if 0	
	char dmr_media_title_arr[1024] = {0};
	
	SWFEXT_FUNC_BEGIN(handle);

	memset(dmr_media_title_arr,0,1024);
	strncpy(dmr_media_title_arr,dlna_dmr_check_media_title(),strlen(dlna_dmr_check_media_title()));
	DMR_ENGINE_DBG("dmr_media_title_arr ================== %s\n",dmr_media_title_arr);
	
	Swfext_PutString(dmr_media_title_arr);
	
	SWFEXT_FUNC_END();	
#endif
	char *dmr_media_title_arr = NULL;
	SWFEXT_FUNC_BEGIN(handle);

	dmr_media_title_arr = dlna_dmr_check_media_title();
	if(dmr_media_title_arr == NULL){
		Swfext_PutString("");
		goto __end_get_media_title__;
	}
	Swfext_PutString(dmr_media_title_arr);
	free(dmr_media_title_arr);
__end_get_media_title__:
	SWFEXT_FUNC_END();	
}

int swfext_dlna_dmr_register(void)
{
	SWFEXT_REGISTER("dlna_DMRStartWork", dlna_dmr_start_work);
	SWFEXT_REGISTER("dlna_DMRStop", dlna_dmr_stop);
	SWFEXT_REGISTER("dlna_DMRGetCmd",dlna_dmr_get_cmd);
	SWFEXT_REGISTER("dlna_DMRGetUri",dlna_dmr_get_uri);
	SWFEXT_REGISTER("dlna_DMRGetMediaType",dlna_dmr_get_media_type);
	SWFEXT_REGISTER("dlna_DMRSettingTitle",dlna_dmr_setting_title);
	SWFEXT_REGISTER("dlna_DMRgetmediatitle",dlna_dmr_get_media_title);
	return 0;
}
#endif
