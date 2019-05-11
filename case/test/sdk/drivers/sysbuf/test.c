#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>

#include "sys_buf.h"
//#include "lcm_op.h"

#define MAX_LOOP_CNT 2000
int main(void)
{
	int fd ,fd2,ret,i=0;
	unsigned long avail=0;
	int count = 0;
	struct mem_dev *arg =(struct mem_dev*)malloc(sizeof(struct mem_dev));
	void *fp1,*fp2;
	unsigned long physic_add1,physic_add2;
	char *buf;
	if ( (fd = open("/dev/sysbuf", O_RDWR)) < 0){
  		printf("open file wrong!->return value:%d\n",fd);
  		exit(1);
 	}

	

	while(1)
	{
	if(count++ == MAX_LOOP_CNT)
		break;
  	ioctl(fd,MEM_QUERY,&avail); 
	printf("max avail buf is :%lu KB\n",avail/1024);

	srand(time(0));

	arg->request_size  = (rand()%avail)&(~4095);
	
	arg->buf_attr  = UNCACHE;
	printf("sysbuf request size:%d KB\n",arg->request_size/1024);
	/** get memory**/
	ioctl(fd,MEM_GET,arg);
	physic_add1 = arg->physic_address;
	fp1 = (char*)arg->logic_address;
	printf("ioctl return physic address:%x  logic address:%x  \n", arg->physic_address,arg->logic_address);
	
	unsigned long addr = arg->logic_address+0x34;
	ioctl(fd,VirADDR2_PHYADDR,&addr);
	if(addr == INVALID_PHYSICADDRESS)
		printf("convert error ,maybe error virtual address \n");		
	else
		printf("ioctl vir2phy phyaddress :%x\n",addr);
	
#if 0	
	DE_INFO temp;
	
	if ( (fd2 = open("/dev/lcm", O_RDWR)) < 0){
  		printf("open file wrong!->return value:%d\n",fd);
  		exit(1);
 	}
	
	ioctl(fd2,GET_DE_CONFIG,&temp);
	temp.DisplaySource = 0;
	temp.input_rgb_bus_addr =physic_add1;
	temp.input_luma_bus_addr = physic_add1;
	ioctl(fd2,UPDATE_DE_CONFIG,&temp);
	
	
	/*fill the framebuffer*/
	for(i=0;i<arg->request_size;i++)
		*(char*)fp1++ =0xcc;

#endif 	
	/** query max available **/


	/**release the memory **/
	arg->physic_address =physic_add1;
	ioctl(fd,MEM_PUT,arg);

	}
	
	close(fd);
	
	return 0;
}

