#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys_pmu.h>
#include <sys_rtc.h>


#define PMU_DEV_PATH		"/dev/pmu"

void mem_dump_long(void *buf, int len)
{	
	int i;
	unsigned int *p;
	for(i=0, p=(unsigned int *)buf; i<len; ) {		
		if( i % 16 == 0 )
			printf("%08x:  ", p );
		printf("%08x ", *p);
		p++;
		i += 4;
		if( i % 16 == 0 )	
		printf("\n");
	}
	if( i % 16 != 0 )
		printf("\n");
	printf("\n");
}

static
unsigned int crc32(unsigned char *ptr, unsigned int len)
{   
	unsigned int   i;   
	unsigned int   crc=0;   
	while(len--!=0)   
	{   
		for(i=0x80;   i!=0;   i/=2)   
		{   
			if((crc&0x8000)!=0)   
			{   
				crc*=2;   
				crc^=0x1021;   
			}   /*   余式CRC   乘以2   再求CRC   */   
			else   
			{   
				crc*=2;   
			}   
			if((*ptr&i)!=0)   
			{   
				crc^=0x1021;   /*   再加上本位的CRC   */   
			}   
		}   
		ptr++;   
	}   
    
	return(crc);   
}

/**  
	this function  can be call when need to change pll 
**/
void change_pll(unsigned int clock)
{
	struct am_chpll_arg chpll_arg;
	struct stat stbuf;
	int fd, fd_pmu;
	void *buf;
	const char *filename = "/am7x/sdk/chpll.bin";

	
	if( stat(filename, &stbuf) != 0 ) {
		fprintf(stderr, "No such file: %s\n", filename);
		exit(-19);
	}	

	fd_pmu = open(PMU_DEV_PATH,O_RDWR);
	if(fd_pmu == -1){
		printf("can not open pmu device\n");
		exit(-19);
	}
	
	buf = malloc(stbuf.st_size);
	if( buf == NULL ) {
		fprintf(stderr, "Out of memory!");
		return -1;
	}

	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "open %s error!",filename);
		return -1;		
	}
	if(read(fd, buf, stbuf.st_size) < 0)
	{
		fprintf(stderr, "read %s error!",filename);
		close(fd);
		return -1;		
	}
	close(fd);

	chpll_arg.sram_entry  = (void *) 0xb4040000; //sram code entry
	chpll_arg.code_start  = buf;
	chpll_arg.code_size   = stbuf.st_size;	
	chpll_arg.clock = clock;

	ioctl(fd_pmu, AM_PMU_PLOW, &chpll_arg);

	close(fd_pmu);
	free(buf);

}


/**  
	this function  need to be called only once the system power up 
**/
void calibrate_clkdly()
{
	struct am_chpll_arg chpll_arg;
	struct stat stbuf;
	int fd, fd_pmu;
	void *buf;
	const char *filename = "/am7x/sdk/calib.bin";

	
	if( stat(filename, &stbuf) != 0 ) {
		fprintf(stderr, "No such file: %s\n", filename);
		exit(-19);
	}	

	fd_pmu = open(PMU_DEV_PATH,O_RDWR);
	if(fd_pmu == -1){
		printf("can not open pmu device\n");
		exit(-19);
	}
	
	buf = malloc(stbuf.st_size);
	if( buf == NULL ) {
		fprintf(stderr, "Out of memory!");
		return -1;
	}

	fd = open(filename, O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "open %s error!",filename);
		return -1;		
	}
	if(read(fd, buf, stbuf.st_size) < 0)
	{
		fprintf(stderr, "read %s error!",filename);
		close(fd);
		return -1;		
	}
	close(fd);

	chpll_arg.sram_entry  = (void *) 0xb4040000; //sram code entry
	chpll_arg.code_start  = buf;
	chpll_arg.code_size   = stbuf.st_size;	

	ioctl(fd_pmu, AM_PMU_GETCD, &chpll_arg);

	close(fd_pmu);
	free(buf);

}

#if 0
int main(int argc, char *argv[])
{
	int fd, fd_pmu, i;
	struct stat stbuf1, stbuf2;
	void *buf;
	struct am_chpll_arg chpll_arg;
	const char *filename1 = "/am7x/sdk/calib.bin";
	const char *filename2 = "/am7x/sdk/chpll.bin";
//	unsigned int clocks[] = { 120, 196, 392, 424 };
//	unsigned int clocks[] = { 120, 196, 392, 360 };
	unsigned int clocks[] = { DDR_LOW_PLL, DDR_HIGH_PLL };
	const int NC = sizeof(clocks) / sizeof(clocks[0]);
	unsigned int span = 1;

	if(argc > 1)
		span = atoi(argv[1]);
	
	printf("span = %u\n", span);
	
	if( stat(filename1, &stbuf1) != 0 ) {
		fprintf(stderr, "No such file: %s\n", filename1);
		exit(-19);
	}
	if( stat(filename2, &stbuf2) != 0 ) {
		fprintf(stderr, "No such file: %s\n", filename2);
		exit(-19);
	}

	buf = malloc(0x8000);
	if( buf == NULL ) {
		fprintf(stderr, "Out of memory!");
		return -1;
	}
	fd_pmu = open(PMU_DEV_PATH,O_RDWR);
	if(fd_pmu == -1){
		printf("can not open pmu device\n");
		exit(-19);
	}

	fd = open(filename1, O_RDONLY);
	read(fd, buf, stbuf1.st_size);
	close(fd);

	chpll_arg.sram_entry  = (void *) 0xb4040000;
	chpll_arg.code_start  = buf;
	chpll_arg.code_size   = stbuf1.st_size;

	mem_dump_long(buf, 64);
	printf("================================================\n");
	printf("%s:%d: [%s] CRC32 = 0x%08x\n", __FILE__, __LINE__, filename1, crc32(buf, stbuf1.st_size));

	for(i=0; i<1; i++){
		chpll_arg.clock = clocks[i % NC] ;
		ioctl(fd_pmu, AM_PMU_GETCD, &chpll_arg);
		sleep(1);
	}

	fd = open(filename2, O_RDONLY);
	read(fd, buf, stbuf2.st_size);
	close(fd);

	mem_dump_long(buf, 64);
	printf("================================================\n");
	printf("%s:%d: [%s] CRC32 = 0x%08x\n", __FILE__, __LINE__, filename2, crc32(buf, stbuf2.st_size));

	chpll_arg.sram_entry  = (void *) 0xb4040000;
	chpll_arg.code_start  = buf;
	chpll_arg.code_size   = stbuf2.st_size;

	printf("span = %u\n", span);
	for(i=0; i<1; i++){
		chpll_arg.clock = clocks[span % NC] ;
		printf("\n\nRound %u\n", i);
		printf("%06u: Change ddr pll to %uM\n", i, chpll_arg.clock);
		ioctl(fd_pmu, AM_PMU_PLOW, &chpll_arg);
		usleep(1000 * 1000 * span);
	}
	close(fd_pmu);
	exit(0);
}
#else

int main(int argc,char* argv[])
{
	unsigned int clk = DDR_LOW_PLL; 
	calibrate_clkdly();

	if(argc > 1)
		clk = atoi(argv[1]);
	change_pll(clk);
	
	return 0;
}
#endif 
