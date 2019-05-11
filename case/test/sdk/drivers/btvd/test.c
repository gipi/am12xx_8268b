#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include "btvd_api.h"
#include "swfdec.h"


#define act_writel(val,reg)  (*(volatile int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))                                                           
static void RegBitSet(int val,int reg,int msb,int lsb)                                            
{                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
}                                                                                                                                                                                                                                                                               
static unsigned int RegBitRead(int reg,int msb,int lsb)
{                                                     
	unsigned int valr;
	
	valr=act_readl(reg);                                                                       
	return (valr<<(31-msb)>>(31-msb+lsb));                                                     
}				


BTVD_INFO info;
static void isr(void)
{
	printf("test\n");
	info.buffer_vir_addr=info.buffer_vir_addr;
	info.buffer_phy_addr=info.buffer_phy_addr;
}
int main (void)
{
	int fd;
	int btvd_base_addr;
	FILE* fp;
	char fname[128];
	
	fd = open("/dev/mem", O_RDWR | O_SYNC);		
	if(fd<0)
		printf("open/dev/mem error\n");
	btvd_base_addr=(int)mmap(0, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x100d0000);
	close(fd);
	
	info.video_format=BT656_480I;
	info.isr=isr;
	info.buffer_vir_addr=(int)SWF_Malloc(720*480*2);
	info.buffer_phy_addr=(int)SWF_MemVir2Phy((void*)(info.buffer_vir_addr));
	fd = open("/dev/btvd", O_RDWR);
	if(fd < 0)
		printf("open/dev/btvd error\n");
	ioctl(fd,BTVD_IOCSOPEN,&info);	
	close(fd);
	
	
	RegBitSet(1,btvd_base_addr+0x18,1,1);
	sleep(1000);
	if(RegBitRead(btvd_base_addr+0x18,1,1)==0)
	{
		printf("no find bt signal\n");
		SWF_Free((void*)(info.buffer_vir_addr));
		return -1;
	}else
	{
		printf("find bt signal\n");
		sprintf(fname, "btvd_out_720x480.yuv");
		fp = fopen(fname, "w+");
		if(fp == NULL)
		{
			printf("fopen %s error\n",fname);
			return -1;
		}
	  	fwrite((int*)(info.buffer_vir_addr),1,720*480*2,fp);
	  	fclose(fp);
	  	SWF_Free((void*)(info.buffer_vir_addr));
		return 0;
	}
	
}