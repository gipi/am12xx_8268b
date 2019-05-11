#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include "am7x_dac.h"
#include "wire_factory_test.h"
#include "wire_log.h"

#define PATH_MAX_LEN			(64)
#define CONFIG_TEST_VIDEO_FILE 	"v_file"
#define CONFIG_TEST_AUDIO_FILE 	"a_file"
#define CONFIG_TEST_VIDEO_FLAG 	"video_test"
#define CONFIG_TEST_AUDIO_FLAG 	"audio_test"
#define CONFIG_TEST_TIME_FLAG 	"test_time"


static int setSystemVolume(int vol)
{
	int fd;
	int err;
	
	fd = open("/dev/DAC",2);
	if (fd < 0) {
		printf("open dac error when get\n");
		return -1;	
	}

	err = ioctl(fd,DACIO_SET_VOLUME,(unsigned char *)&vol);
	if(err < 0){
		printf("dac get volume error\n");
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static int get_config_path(char *path){
	char u_path[PATH_MAX_LEN];
	memset(u_path, 0, sizeof(path));
	get_usb_path(u_path);
	printf("[%s][%d] -- path: %s\n", __func__, __LINE__, u_path);
	snprintf(path, PATH_MAX_LEN, "%s/%s", u_path, TEST_CONFIG_FILE);

	return 0;
}

static int get_string_from_config(const char *name, char *data, int len)
{
	FILE *fp = NULL;
	char buff[1024];
	char _name[32];
	char path[128];
	char *p1 = NULL, *p2 = NULL, *p3 = NULL;
	int ret = -1;
	int l;

	get_config_path(path);
	printf("[%s][%d] -- Get test config [%s]!!!\n", __func__, __LINE__, path);
	if(access(path, F_OK) == 0){
		fp = fopen(path, "r");
		if(fp != NULL){
			ret = fread(buff, 1, sizeof(buff)-1, fp);
			fclose(fp);
			if(ret > 0){
				buff[ret] = '\n';
				snprintf(_name, sizeof(_name), "%s:", name);
				p1 = strcasestr(buff, _name);
				if(p1 != NULL){
					p1 += strlen(_name);
					p2 = strchr(p1, '\n');
					if(p2 != NULL){
						l = p2 - p1;
						p3 = strchr(p1, '\r');
						if(p3 != NULL){
							if(p3 < p2){
								l = p3 - p1;
							}
						}
						while(l>0){
							printf("[%s][%d] -- p1[0] = %c, p1[0] = %d\n", __func__, __LINE__, p1[0], p1[0]);
							if(p1[0] != 32)		// Do not copy space
								break;
							else{
								p1++;
								l--;
							}
						}
						while(l>0){
							printf("[%s][%d] -- p1[l-1] = %c, p1[l-1] = %d\n", __func__, __LINE__, p1[l-1], p1[l-1]);
							if(p1[l-1] != 32)		// Do not copy space
								break;
							else
								l--;
						}
						printf("[%s][%d] -- l: %d\n", __func__, __LINE__, l);
						//snprintf(data, len, "%s", p1);
						int length = (l<(len-1))?l:(len-1);
						memcpy(data, p1, length);
						data[length] = '\0';
						printf("[%s][%d] -- data: %s\n", __func__, __LINE__, data);
						return 0;
					}
				}
				printf("[%s][%d] -- get name error!!!\n", __func__, __LINE__);
			}
		}
	}
	return -1;
}

#define AUDIO_TIME 100	//ms
int do_factory_test()
{
	int total_len = 0;
	int cur_len = 0;
	int pak_len = 0;
	char path[32] = {0};
	char value[32] = {0};
	char v_flag[32] = {0};
	char a_flag[32] = {0};
	char t_flag[32] = {0};
	int v_value = 0;
	int a_value = 0;
	int t_value = 0;
	char ifname_video[128] = {0};
	char ifname_audio[128] = {0};
	int i = 9;
	int ret;
	FILE *fp_video = NULL;
	FILE *fp_audio = NULL;
	unsigned char *data_video = NULL;
	unsigned char *data_audio = NULL;
	int data_size = 1024*1024;
	int data_head_size = 32;
	int data_audio_buf = AUDIO_TIME*192;	//48k samplerate,16bit
	FRAME_HEAD_INFO head_info;	
	INITAUDIO InitAudio = NULL;
	SENDAUDIOBUF SendAudioBuf = NULL;
	UNUINTAUDIO	UninitAudio = NULL;
	unsigned long sec0,usec0,sec1,usec1;
	int diff_ms=0;
	int count=0;

	get_string_from_config(CONFIG_TEST_VIDEO_FLAG, v_flag, sizeof(v_flag));
	get_string_from_config(CONFIG_TEST_AUDIO_FLAG, a_flag, sizeof(a_flag));
	get_string_from_config(CONFIG_TEST_TIME_FLAG, t_flag, sizeof(t_flag));
	WLOGI("video_test:%s,audio_test:%s,test_time:%s\n", v_flag, a_flag,t_flag);
	if(!strcmp(v_flag, "true")){
		v_value = 1;
	}else{
		v_value = 0;
	}
	if(!strcmp(a_flag, "true")){
		a_value = 1;
	}else{
		a_value = 0;
	}
	t_value = atoi(t_flag);
	WLOGI("test Num is %d\n",t_value);
	if(t_value <= 0){
		WLOGI("test Num is %d,stop test\n", t_value);
		//return -1;
	}
	//t_value--;
	
	get_usb_path(path);
	WLOGI("mount path is %s\n", path);

	if(v_value == 1){
		get_string_from_config(CONFIG_TEST_VIDEO_FILE, value, sizeof(value));
		OSsprintf(ifname_video, "%s/%s", path, value);
		OSprintf(" ifname = %s\n", ifname_video);
			
		fp_video = (FILE *)OSfopen(ifname_video, (char *)"rb");
		if(!fp_video)
		{
			OSprintf("%s, OSfopen error\n", __FUNCTION__);
			goto FAC_TEST_FAIL;
		}
			
		OSfseek_end(fp_video, 0);
		total_len = OSftell(fp_video);
		OSprintf("total_len = %d\n",total_len);
		OSfseek_set(fp_video, 0);
	
		data_video = (unsigned char *)OSmalloc(data_size);
		if(NULL == data_video){
			WLOGE("malloc failed!\n");
			goto FAC_TEST_FAIL;
		}
		OSmemset(data_video, 0, data_size);

		ret = wire_HantroOpen();
		if(ret < 0){
			WLOGE("open player failed!\n");
			goto FAC_TEST_FAIL;
		}
		
	}	
	
	if(a_value == 1){
		get_string_from_config(CONFIG_TEST_AUDIO_FILE, value, sizeof(value));
		OSsprintf(ifname_audio, "%s/%s", path, value);
		OSprintf(" ifname = %s\n", ifname_audio);
			
		fp_audio = (FILE *)OSfopen(ifname_audio, (char *)"rb");
		if(!fp_audio)
		{
			OSprintf("%s, OSfopen error\n", __FUNCTION__);
			goto FAC_TEST_FAIL;
		}
			
		OSfseek_end(fp_audio, 0);
		total_len = OSftell(fp_audio);
		OSprintf("total_len = %d\n",total_len);
		OSfseek_set(fp_audio, 0);
	
		data_audio = (unsigned char *)OSmalloc(data_audio_buf);
		if(NULL == data_audio){
			WLOGE("malloc failed!\n");
			goto FAC_TEST_FAIL;
		}
		OSmemset(data_audio, 0, data_audio_buf);

		void *audioHandle = dlopen("libaudio_player.so", RTLD_LAZY | RTLD_LOCAL);
		if(audioHandle == NULL)
		{
			WLOGE("open libaudio_player.so fail.\n");
			perror("ERROR");
			goto FAC_TEST_FAIL;
		}

		InitAudio =		OSDLGetProcAddr(audioHandle,"InitAudio");
		SendAudioBuf = OSDLGetProcAddr(audioHandle,"SendAudioBuf");
		UninitAudio	=	OSDLGetProcAddr(audioHandle,"UninitAudio");

		InitAudio(48000, 2, 16);
		int vol = 38;
		setSystemVolume(vol);
	}
	
	while(!feof(fp_video)){
		if(v_value == 1){
			if(OSfread(&head_info, 1, sizeof(FRAME_HEAD_INFO), fp_video) != sizeof(FRAME_HEAD_INFO)){
				WLOGI();
				//break;
			}
			if(OSfread(data_video, 1, head_info.len, fp_video) != head_info.len){
				WLOGI();
				//break;
			}			
			if(head_info.w == 0 || head_info.h == 0){		
				wire_SetFrameSPSPPS(data_video, head_info.len);//spspps
			}else{
				OSGetTime(&sec0,&usec0);
				wire_HantroPlay(data_video, head_info.len, head_info.w, head_info.h, 0);
				OSGetTime(&sec1,&usec1);
				diff_ms=(sec1-sec0)*1000000+(usec1-usec0);
				diff_ms/=1000;
				//OSprintf("%02d:%dms\n",count++,diff_ms);
				if((diff_ms+2)<AUDIO_TIME)
				data_audio_buf=(diff_ms+2)*192;
			}
		}
		
		if(a_value == 1){
			if(!feof(fp_audio)){
				if(OSfread(data_audio, 1, data_audio_buf, fp_audio) != data_audio_buf){
					WLOGI();
					//break;
				}
				SendAudioBuf(data_audio, data_audio_buf);
			}
		}
		//WLOGI("feof is %d\n",feof(fp_video));
		if(t_value != 0 && feof(fp_video)){
			//t_value--;
		//if(feof(fp_video)){
			if(fp_audio)
			OSfseek_set(fp_audio, 0);
			OSfseek_set(fp_video, 0);
			count=0;
			WLOGI("test Num is %d\n",t_value);
		}
		if(get_usb_out_flag())
		{
			WLOGI("exit loop\n");
			break;
		}
		
	}
	WLOGI("exit factory test\n");
	if(v_value == 1)
		wire_HantroClose();
	if(a_value == 1)
		UninitAudio();
	ezwireDrawDefaultBg();
	ezwireDrawFlip();
	#if (EZWIRE_TYPE==MIRAPLUG || EZWIRE_TYPE==MIRALINE)
	show_local_version();
	#endif
	if(fp_video){
		OSfclose(fp_video);
		fp_video = NULL;
	}
	if(data_video){
		OSfree(data_video);
		data_video = NULL;
	}

	if(fp_audio){
		OSfclose(fp_audio);
		fp_audio = NULL;
	}
	if(data_audio){
		OSfree(data_audio);
		data_audio = NULL;
	}
	return 0;
#if 0
	file_data = (unsigned char *)OSmalloc(total_len);
	if(NULL == file_data){
		WLOGE("malloc failed!\n");
		goto FAC_TEST_FAIL;
	}

	if(OSfread(file_data, 1, total_len, fp_in) != total_len)
			goto FAC_TEST_FAIL;
	OSfclose(fp_in);	

	//data_head = (char *)OSmalloc(sizeof(FRAME_HEAD_INFO));
	//if(NULL == data_head){
	//	WLOGE("malloc failed!\n");
	//	goto FAC_TEST_FAIL;
	//}
	//OSmemset(data_head, 0, data_head_size);
	data = (unsigned char *)OSmalloc(data_size);
	if(NULL == data){
		WLOGE("malloc failed!\n");
		goto FAC_TEST_FAIL;
	}
	OSmemset(data, 0, data_size);

	ret = wire_HantroOpen();
	if(ret < 0){
		WLOGE("open player failed!\n");
		goto FAC_TEST_FAIL;
	}

	while(cur_len < total_len){
		OSmemcpy(&head_info, file_data+cur_len, sizeof(FRAME_HEAD_INFO));
		OSmemcpy(data, file_data+cur_len+sizeof(FRAME_HEAD_INFO), head_info.len);
		if(head_info.w == 0 || head_info.h == 0){		
			wire_SetFrameSPSPPS(data, head_info.len);//spspps
		}else{
			wire_HantroPlay(data, head_info.len, head_info.w, head_info.h);
		}
		cur_len = cur_len+sizeof(FRAME_HEAD_INFO)+head_info.len;
	}
	wire_HantroClose();
	ezwireDrawDefaultBg();
	ezwireDrawFlip();
	return 0;
FAC_TEST_FAIL:
	if(fp_in){
		OSfclose(fp_in);
		fp_in = NULL;
	}
	if(file_data){
		OSfree(file_data);
		file_data = NULL;
	}
	//if(data_head){
	//	OSfree(data_head);
	//	data_head = NULL;
	//}
	if(data){
		OSfree(data);
		data = NULL;
	}
	return -1;
#endif
FAC_TEST_FAIL:
	if(v_value == 1)
		wire_HantroClose();
	if(a_value == 1)
		UninitAudio();
	ezwireDrawDefaultBg();
	ezwireDrawFlip();
	if(fp_video){
		OSfclose(fp_video);
		fp_video = NULL;
	}
	if(data_video){
		OSfree(data_video);
		data_video = NULL;
	}

	if(fp_audio){
		OSfclose(fp_audio);
		fp_audio = NULL;
	}
	if(data_audio){
		OSfree(data_audio);
		data_audio = NULL;
	}
	return -1;

}
