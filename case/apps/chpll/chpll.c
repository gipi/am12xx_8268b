#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys_pmu.h>
#include <sys_rtc.h>
#include "sys_cfg.h"

#define PMU_DEV_PATH		"/dev/pmu"

/**  
	this function  need to be called only once the system power up 
**/
#if MODULE_CONFIG_PLL
int pll_calibrate_clkdly()
{
	struct am_chpll_arg chpll_arg;
	struct stat stbuf;
	int rtn=-1;
	int fd=-1, fd_pmu=-1;
	void *buf=NULL;
	const char *filename = "/am7x/sdk/calib.bin";



	if( stat(filename, &stbuf) != 0 ) {
		fprintf(stderr, "No such file: %s\n", filename);
		rtn = -1;
		goto CALIBRATE_CLKDLY_END;
	}	

	fd_pmu = open(PMU_DEV_PATH,O_RDWR);
	if(fd_pmu == -1){
		printf("can not open pmu device\n");
		rtn = -1;
		goto CALIBRATE_CLKDLY_END;
	}
	
	buf = malloc(stbuf.st_size);
	if( buf == NULL ) {
		fprintf(stderr, "Out of memory!");
		rtn = -1;
		goto CALIBRATE_CLKDLY_END;
	}

	fd = open(filename, O_RDONLY);
	if(fd < 0){
		fprintf(stderr, "open %s error!",filename);
		rtn = -1;
		goto CALIBRATE_CLKDLY_END;
	}
	if(read(fd, buf, stbuf.st_size) < 0){
		fprintf(stderr, "read %s error!",filename);
		rtn = -1;
		goto CALIBRATE_CLKDLY_END;	
	}
	chpll_arg.sram_entry  = (void *) 0xb4040000; //sram code entry
	chpll_arg.code_start  = buf;
	chpll_arg.code_size   = stbuf.st_size;	

	ioctl(fd_pmu, AM_PMU_GETCD, &chpll_arg);
	rtn = 0;

CALIBRATE_CLKDLY_END:
	if(fd>=0)
		close(fd);
	if(fd_pmu!=-1)
		close(fd_pmu);
	if(buf)
		free(buf);
	printf("%s,%d:PLL ClkDly Calibrate OK rtn=%d!\n",__FILE__,__LINE__,rtn);
	return rtn;
}

#else
int pll_calibrate_clkdly()
{
	return 0;
}
#endif
int main(int argc,char* argv[])
{
	printf("%%%%%%%%%%%%%%%%%%%%%%%%Call TEST CLD DELAY\n");
	pll_calibrate_clkdly();	
	return 0;
}

