

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/string.h>



#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))
#define act_readl(port)  (*(volatile unsigned int *)(port))
static void RegBitSet(int val,int reg,int msb,int lsb){                                             
unsigned int mask = 0xFFFFFFFF;
unsigned int old  = act_readl(reg);

	mask = mask << (31 - msb) >> (31 - msb + lsb) << lsb;
	act_writel((old & ~mask) | ((val << lsb) & mask), reg);	         
}                                                                                             
                                                                                             
                                                                                          
static unsigned int RegBitRead(int reg,int msb,int lsb){                                                     
	unsigned int valr;
	
	valr=act_readl(reg);                                                                       
	return (valr<<(31-msb)>>(31-msb+lsb));                                                     
}


int main(int argc,char **argv)
{
	unsigned int add,val;
	int count=256;
	int fd;
	unsigned int vir_add,offset,vir_ori,size;

	add=strtoul(argv[2],NULL,16);
	if(strcmp(argv[1],"dump")==0)
		count=strtoul(argv[3],NULL,16);		
	printf("add=0x%x\n",add);
	printf("count=0x%x\n",count);
	fd = open("/dev/mem", O_RDWR | O_SYNC);	
	if(fd<0)
		printf("open/dev/mem error\n");
	offset=add-(add&0xfffff000);
	size=offset+count*4;
	vir_ori=(int)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, add&0x1ffff000);
	vir_add=vir_ori+offset;
	if(strcmp(argv[1],"rm")==0)
	{
		int i;
		for(i=0;i<count;i++)
		{
			if(i%4==0)
				printf("%08x:",add+4*i);
			printf(" %08x",act_readl(vir_add+4*i));
			if((i+1)%4==0)
				printf("\n");
		}				
	}
	else if(strcmp(argv[1],"mm")==0)
	{
		val=strtoul(argv[3],NULL,16);
		act_writel(val,vir_add);
	}
	else if(strcmp(argv[1],"dump")==0)
	{
		char fname[128];
		FILE* fp;
		
		sprintf(fname, "%s",argv[4]);
		fp = fopen(fname, "w+");
		if(fp == NULL)
		{
			printf("fopen %s error\n",fname);
			goto _exit;
		}

	  	fwrite((int*)(vir_add),1,count,fp);
	  	fclose(fp);
	}
_exit:	
	munmap((void*)vir_ori,size);	
	close(fd);
	
	return 0;
}

