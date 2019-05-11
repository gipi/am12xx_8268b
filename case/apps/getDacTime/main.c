#include <stdio.h>
#include "am7x_dac.h"

int main(int argc, char *argv[])
{
	int fd;
	int err;
	int time_ms=0;
	
	fd = open("/dev/DAC",2);
	if (fd < 0) {
		printf("open dac error \n");
		return 0;	
	}

	err = ioctl(fd,DACIO_GET_PLAY_TIME,&time_ms);
	if(err < 0){
		printf("dac get play time error\n");
		close(fd);
		return 0;	
	}
	
	close(fd);
	printf("dac play time :%d ms\n",time_ms);

	return time_ms;
}



