
#include "osapi.h"
#include "audio_midware.h"
//#include "includes.h"
//#include "atj213x.h"

int stop_flag=0;
int key_detect(void* handle,void* music_status){
		int time;
		int quite=0;
		play_param_t play_param;
		if(OSKBHit())
		{
			int ch = OSKBGetChar();
			switch(ch)
			{
			case 'q':
			case 'Q':

				OSprintf("quit\n");
				quite=1;
				break;
			case 'f':
			case 'F':
				OSprintf(" fast forward\n");
				audioDecCmd(handle, FAST_FORWARD,1);
				break;
			case 't':
			case 'T':
				OSprintf(" test\n");
				audioDecCmd(handle, SET_SEEK_TIME,100);
				break;
			case 'b':
			case 'B':
				OSprintf(" back forward\n");
				audioDecCmd(handle, FAST_BACKWARD,1);
				break;
			case 'c':
			case 'C':
				OSprintf(" cancel forward\n");
				audioDecCmd(handle, CANCEL_FF,0);
				break;
			case 'r':
			case 'R':
				OSprintf(" play\n");
				stop_flag=0;
				play_param.mode=NORMAL_PLAY;
				play_param.param=0;
				audioDecCmd(handle,  PLAY,(unsigned int)&play_param);
				audioDecCmd(handle, GET_PLAYER_STATUS,music_status);
			//	((music_status_t*)music_status)->status=PLAYER_PLAYING;
				break;
			case 's':
			case 'S':
				OSprintf(" stop\n");
				stop_flag=1;
				audioDecCmd(handle, STOP,&time);
				((music_status_t*)music_status)->cur_time=time;
			//	((music_status_t*)music_status)->status=PLAYER_STOPPED;
				break;
			case 'p':
			case 'P':
				OSprintf(" continue\n");
				stop_flag=0;
				play_param.mode=TAG_PLAY;
				play_param.param=0;
				audioDecCmd(handle, PLAY,(unsigned int)&play_param);
				audioDecCmd(handle, GET_PLAYER_STATUS,music_status);
			//	((music_status_t*)music_status)->status=PLAYER_PLAYING;
				break;
			default:
				OSprintf("invalid command:%c\n", ch);
			}
		}
		
return quite;
}
typedef struct {
	int offset;
	int len;
	char *ext;
}flash_input_t;
//#define test_flash_input
void main()
{
	void *handle;

	music_file_info_t media_info;
	music_status_t music_status;
	play_param_t play_param;
#ifdef test_flash_input
	flash_input_t input;
	input.len=0xa8800;
	input.offset=0x128800;
	input.ext=".mp3";
#endif
	OSKBOpen();
	handle=audioDecOpen(0);
#ifdef test_flash_input
	audioDecCmd(handle, SET_INDEX,&input);
#endif
	audioDecCmd(handle, SET_FILE,"F:\\vc_code\\audio_midware\\OSAPI\\a.mp3");//setfile
	
	audioDecCmd(handle, GET_MEDIA_INFO,&media_info); //get media info
	OSprintf("total time %d:%d \n",media_info.total_time/60,media_info.total_time%60);
	play_param.mode=NORMAL_PLAY;
	play_param.param=0;
	audioDecCmd(handle, PLAY,(unsigned int)&play_param);

	while(1){
		if(key_detect(handle,&music_status))
			break;
		if(stop_flag==0){
			audioDecCmd(handle, GET_PLAYER_STATUS,&music_status);			
				OSprintf("\r%d:%d ", music_status.cur_time/60, music_status.cur_time%60);		
		}
		else
			OSprintf("\r%d:%d stopped", music_status.cur_time/60, music_status.cur_time%60);
		if(music_status.status==PLAYER_STOPPED)
			OSSleep(20);
	}
	audioDecClose(handle);
	OSKBClose();
	OSprintf("end\n");


 }


