#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
typedef  enum{
		NORMAL_PLAY,						//正常播放					
		TAG_PLAY,						//TAG播放
		SEEK_PLAY						//断点续传/触摸屏seek播放
} play_mode_t;

typedef struct{
	play_mode_t mode;					//当前的播放模式
	unsigned int param;					//不同播放模式下的输入参数
}play_param_t;	
main(){

	void *handle;
	play_param_t play_param;
printf("1\n");
	handle=audioDecOpen(0);
printf("2\n");
	audioDecCmd(handle, 0x16,"a.mp3");//setfile
printf("3\n");
	play_param.mode=NORMAL_PLAY;
	play_param.param=0;
	audioDecCmd(handle, 0x1,(unsigned int)&play_param);
	
printf("4\n");
while(1);



}
