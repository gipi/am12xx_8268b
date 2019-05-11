#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define __USE_GNU 1
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys_buf.h>


#define DATA_LENGTH	(16*1024)
#define FILE_PATH		"/mnt/card/test.jpg"

#define USE_SYSBUF		1

int main(void)
{
	int fd=0,val;
	char* buf=NULL;
	
#if USE_SYSBUF 
	int sysfd;
	struct mem_dev sysbuf;

	if ( (sysfd = open("/dev/sysbuf", O_RDWR)) < 0){
  		printf("open file wrong!->return value:%d\n",sysfd);
  		goto TEST_FAIL;
 	}else{
 		sysbuf.request_size = DATA_LENGTH;
		//sysbuf.buf_attr = UNCACHE;
		sysbuf.buf_attr = MEM_FS;
		if(ioctl(sysfd,MEM_GET,&sysbuf)){
			printf("sysbuf malloc fail\n");
			close(sysfd);
			goto TEST_FAIL;
		}else{
			buf = (char*)sysbuf.logic_address;
			printf("sys malloc success:%lx(%lx)\n",sysbuf.logic_address,sysbuf.physic_address);
			memset(buf,0,DATA_LENGTH);
		}	
	}
#else
	buf = (char *)valloc(DATA_LENGTH);
	if(!buf){
		printf("valloc fail\n");
		goto TEST_FAIL;
	}
#endif

	fd = open(FILE_PATH,O_RDONLY|O_DIRECT);
	if(fd){
		int i=0;
		for(i=0;i<10;i++){
		val = read(fd,buf,DATA_LENGTH);
		if(val<0)
			printf("read failed %d\n",-errno);
		else
				printf("read success!%x\n",*(unsigned int *)buf);
		}
		close(fd);
	}else{
		printf("open %s failed %d\n",FILE_PATH,-errno);
	}

#if USE_SYSBUF
	ioctl(sysfd,MEM_PUT,&sysbuf);
	close(sysfd);
#else
	free(buf);
#endif

	exit(0);
TEST_FAIL:
	exit(1);
}

