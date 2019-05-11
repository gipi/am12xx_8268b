#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "sys_buf.h"

#include "audio_linein.h"

int mode=1;
int VolumeDown=0;

//example:
//linein.app cpu 8252c
//linein.app dma 8252w

int main(int argc, char** argv)
{
	int i=0;
	int fd=-1;
	void* handle;
	int delay_ms=0;
	int size;
	struct mem_dev adc_buf ;
	struct mem_dev dac_buf ;
	linein_input_t input;
	char lineinout[64];
	char cmd[128];
	
	memset(&adc_buf,0,sizeof(struct mem_dev));
	memset(&dac_buf,0,sizeof(struct mem_dev));
	memset(&input,0,sizeof(linein_input_t));
	memset(lineinout,0,sizeof(lineinout));
    memset(cmd,0,sizeof(cmd));
	strcpy(lineinout,"/tmp/lineinout");

	for(i=1;i<argc;i++)
	{
		if((0==strcmp(argv[i],"cpu"))||(0==strcmp(argv[i],"adb")))
		{
			mode=0;	//cpu mode
		}
		if((0==strcmp(argv[i],"dma"))||((0==strcmp(argv[i],"tethering"))))
		{
			mode=1;	//dma mode
		}
		if(0==strcmp(argv[i],"8252c"))
		{
			VolumeDown=1;	
		}		
		if(0==strcmp(argv[i],"8252w"))
		{
			VolumeDown=0;	
		}
	}
	
    sprintf(cmd,"rm -f %s",lineinout);
	system(cmd);
	
#if 1	
    fd = open("/dev/sysbuf",O_RDWR);

	size=48000*2*4*100/1000;	//100ms,32bit stereo
	adc_buf.request_size =size; 
	adc_buf.buf_attr = UNCACHE;
    ioctl(fd,MEM_GET,&adc_buf);     
	if(0==adc_buf.physic_address)
	{
		OSprintf("adc_buf malloc %d error\n\n",adc_buf.request_size);
		goto End;
	}
	//OSprintf("adc_buf.physic_address=%x,size=%d\n",adc_buf.physic_address,size);
	input.adc_addr=adc_buf.physic_address;
	input.adc_len=size;

	size=48000*2*2;	//16bit,stereo,can play 1s
	dac_buf.request_size = size;
	dac_buf.buf_attr = UNCACHE;
	ioctl(fd,MEM_GET,&dac_buf); 	
	if(0==dac_buf.physic_address)
	{
		OSprintf("adc_buf malloc %d error\n\n",dac_buf.request_size);
		goto End;
	}
	//OSprintf("dac_buf.physic_address=%x,size=%d\n",dac_buf.physic_address,size);
	input.dac_addr=dac_buf.physic_address;
	input.dac_len=size;
#endif	

	input.delay=delay_ms;

	handle=AudioLineInStart(&input);
	if(NULL==handle)
	{
		printf("AudioLineInStart error\n");
		return 0;
	}	


	while(0!=access(lineinout,F_OK))
	{
		OSSleep(100);
	}
		
	AudioLineInStop(handle);
	
End:	

	if(adc_buf.physic_address)
		ioctl(fd,MEM_PUT,&adc_buf);
	if(dac_buf.physic_address)
		ioctl(fd,MEM_PUT,&dac_buf);
	if(fd>=0)
		close(fd);

	system(cmd);
	OSprintf("cmd:%s\n",cmd);
	OSprintf("exit linein.app!\n");

	return 0;
}
