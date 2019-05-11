#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#define __USE_GNU 1
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys_buf.h>


#define DATA_LENGTH	(16*1024)
#define FILE_PATH		"/mnt/udisk/test.txt"
#define TEST_DATA		't'		

#define TEST_RAW_API	0
#define TEST_F_API		1

int main(void)
{
	int val;
	char* buf=NULL;
#if TEST_RAW_API
	int fd=0;
#elif TEST_F_API
	FILE *fd = NULL;
#endif
	
	buf = (char *)valloc(DATA_LENGTH);
	if(!buf){
		printf("valloc fail\n");
		goto TEST_FAIL;
	}

/*-----------------------------------------------------
write test
-------------------------------------------------------*/
#if  TEST_RAW_API
	printf("start test fs syscall:\n");
	fd = open(FILE_PATH,O_RDWR);
#elif TEST_F_API
	printf("start test libc api:\n");
	fd = fopen(FILE_PATH,"w+");
#endif
	if(fd){
		int i=0;

		memset(buf,TEST_DATA,DATA_LENGTH);
		for(i=0;i<10;i++){
		#if TEST_RAW_API
			val = write(fd,buf,DATA_LENGTH);
		#elif TEST_F_API
			val = fwrite(buf,1,DATA_LENGTH,fd);
		#endif
			if(val<0)
				printf("write failed %d\n",-errno);
			else
				printf("write success: %x\n",*(unsigned int *)buf);
		}
	#if TEST_RAW_API
		fsync(fd);
		close(fd);
	#elif TEST_F_API
		fflush(fd);
		fclose(fd);
	#endif
	}else{
		printf("open %s failed %d\n",FILE_PATH,-errno);
		goto TEST_FAIL;
	}
/*-----------------------------------------------------
read test
-------------------------------------------------------*/
#if  TEST_RAW_API
	fd = open(FILE_PATH,O_RDONLY);
#elif TEST_F_API
	fd = fopen(FILE_PATH,"r");
#endif
	if(fd){
		int i=0;

		memset(buf,0,DATA_LENGTH);
		for(i=0;i<10;i++){
		#if TEST_RAW_API
			val = read(fd,buf,DATA_LENGTH);
		#elif TEST_F_API
			val = fread(buf,1,DATA_LENGTH,fd);
		#endif
			if(val<0)
				printf("read failed %d\n",-errno);
			else
				printf("read success: %x\n",*(unsigned int *)buf);
		}
	#if TEST_RAW_API
		close(fd);
	#elif TEST_F_API
		fclose(fd);
	#endif
	}else{
		printf("open %s failed %d\n",FILE_PATH,-errno);
		goto TEST_FAIL;
	}

	free(buf);

	exit(0);
TEST_FAIL:
	if(buf)
		free(buf);
	exit(1);
}

