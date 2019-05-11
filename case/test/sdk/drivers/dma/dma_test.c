/**
 *This file is mainly for dma test
 *As DMA has no interface in usr space, we test it though GPIO API.
 *I known it is shameful to do it like this but it is only a private code for kernel developer.
 *In order to make it work well, SUPPORT_DMA_TEST macro in GPIO driver should be enabled!
 *
 *author: yekai
 *date:2010-07-13
 *version:0.1
 *----------------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys_gpio.h>


int main(void)
{
	int fd,i;
	char value=-1;
	
	printf("This test code is mainly for dma'\n");
	fd = open(GPIO_DEV_PATH,O_RDONLY);
	if(fd<0){
		printf("cant open gpio device\n");
		return -errno;
	}else{
		/*0x100: DMA_TEST_CMD
		   1: DMA_TYPE_BUS(0:SPECIAL,2:CH2)
		*/
		if(ioctl(fd,0x100,1))
			printf("test fail:%d\n",-errno);
		else
			printf("congratulations!\n");
		close(fd);
	}

	exit(0);
}
