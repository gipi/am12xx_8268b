

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
static int gpio_base_add;

void bdi_debug(void)
{
	int fd;
	
	fd = open("/dev/mem", O_RDWR | O_SYNC);		
	if(fd<0)
		printf("open/dev/mem error\n");
	gpio_base_add=(int)mmap(0, 0x200, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x101c0000);
	close(fd);
	
	printf("bdi_debug\n");
	//1211
	/*RegBitSet(0x0,gpio_base_add+0x48,11,8);
	RegBitSet(0x0,gpio_base_add+0x48,15,12);
	RegBitSet(0x0,gpio_base_add+0x48,18,16);
	RegBitSet(0x0,gpio_base_add+0x4c,2,0);
	RegBitSet(0x0,gpio_base_add+0x4c,13,12);
	RegBitSet(0x0,gpio_base_add+0x54,30,28);
	*/
	//1220
	RegBitSet(0x0,gpio_base_add+0x48,2,0);
	RegBitSet(0x0,gpio_base_add+0x48,6,4);
	RegBitSet(0x0,gpio_base_add+0x48,14,12);
	RegBitSet(0x0,gpio_base_add+0x48,18,16);
	RegBitSet(0x0,gpio_base_add+0x48,23,20);
	RegBitSet(0x0,gpio_base_add+0x5c,7,6);
	asm("sdbbp");		
	while(1);
}

int main(int argc,char **argv)
{

	bdi_debug();
	return 0;
}

