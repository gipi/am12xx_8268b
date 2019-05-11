/* DMA sound playback with OSS. */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "am7x_dac.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#include "sys_buf.h"


unsigned char buf[1024];

int main(void)
{	int len;
    /* file handle for /dev/dsp */
    	int dsp = 0;
	int file_in;
	int fd;
	unsigned char vol;
    /* Attempt to open /dev/dsp for playback. We need to open for
       read/write in order to mmap() the device file. */
	struct mem_dev example ;
	example.request_size = 30*1024;
        example.buf_attr = UNCACHE;
        fd = open("/dev/sysbuf",O_RDWR);
        ioctl(fd,MEM_GET,&example);
        
        close(fd); 






   	dsp = open("/dev/DAC",2);
	
    /* This could very easily fail, so we must handle errors. */
	if (dsp < 0) {
		perror("DMA player: error opening /dev/dsp for playback");
		return 0;	
    	}
    	dac_para_t key_dac_para;
	dac_fifo_para_t dac_fifo_para_set;
	key_dac_para.channel=0;//stereo
	key_dac_para.precision=1; //16bit
	key_dac_para.fifo_input_sel=4;//by fred sdram
	
	key_dac_para.sample_rate=0x9;			
	dac_fifo_para_set.fifo_irq_end_enable=1;
	dac_fifo_para_set.fifo_irq_mid_enable=0;
	dac_fifo_para_set.fifo_drq_enable=0;
	dac_fifo_para_set.fifo_drq_kesound_enable=0;

	ioctl(dsp,DACIO_SET_ADDR,(unsigned int)example.physic_address);
	ioctl(dsp,DACIO_SET_FIFO,(unsigned int)&dac_fifo_para_set);//by fred
	ioctl(dsp,DACIO_SET_PARA,(unsigned int)&key_dac_para);//by fred
	vol=20;
	ioctl(dsp,DACIO_SET_VOLUME,(unsigned char *)&vol);
	ioctl(dsp,DACIO_GET_VOLUME,(unsigned char *)&vol);
	printf("vol=0x%x\n",vol);

	file_in=open("a.pcm",0);
	printf("file_in=%d\n",file_in);
	ioctl(dsp,DACIO_SET_START,0);
	while(1){
		//operate file 
		
		if(1024==read(file_in,buf,1024)){		
			//	printf("len=1024\n");	
			while(1024!=write(dsp,buf,1024))//write to dac
			;
		}
		else
			break;
		
	}
    close(dsp);
	close(file_in);
    return 0;
	
 
}

