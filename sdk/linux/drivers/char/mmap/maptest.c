#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
//the value must be smaller than drv define len
//in map_driver.c
//#define MAPLEN (4096*10)

#define LEN (10*4096)
int main(void)
{
	int fd;
	unsigned int *vadr;

	if ((fd=open("/dev/mapfile", O_RDWR))<0)
	{
		perror("open");
		return -1;
		//exit(-1);
	}
	vadr = (int *)mmap(0, LEN, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (vadr == MAP_FAILED)
	{
		perror("mmap");
		return -1;
		//exit(-1);
	}
	//printf this message in map_driver
	//strcpy((char*)vmalloc_area,"hello world from kernel space !"); 

	printf("%s\n",(char*)vadr);
	//test memory!!
	memset((char*)vadr,0,LEN);
	
	printf("%s:%p mapsize:%d\n",__func__,vadr,LEN);
	close(fd);
	
	return(0);
}


