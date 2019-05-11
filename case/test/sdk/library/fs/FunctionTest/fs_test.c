#include <linux/fs.h>
#include <sys/time.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <limits.h>

#define __USE_GNU 1 //必须加上，否则提示O_DIRECT没有定义
#define __USE_XOPEN2K 1

#include <fcntl.h>
#include <sys/mman.h> /* for mmap and munmap */
#include <time.h>
#include "linux/hdreg.h"
#include <am_types.h>
#include "sys_buf.h"
#include "lcm_op.h"
#include "malloc.h"


 void _Print_Buf( const INT8U * pData, INT16U inLen)
{
	INT16U iLoop;

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%2x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x1f) )
		{
			printf("  %d\n",iLoop);
		}
	}
       printf("\n");
	
}


/*
	main参数含义: 
	第一个: 读文件的路径；
	第二个: 写文件的路径；
	第三个: 是否使用dio方式,dio=0代表使用buffer cache方式；
	第四个: 是否使用sysbuf分配内存；
	例如:
	t_speed.app  /mnt/card/test.bin  /mnt/card/test.bin  dio=1  sysbuf=1
*/

int  main(int argc, char** argv)
{   
	int fd = 0;
	int CreatFileNO = 0;
	int MkDirNO = 0;
	int UnlinkFileNO = 0;
	int RmDirNO = 0;
	int RenameNO = 0;
	int i = 0;
	char buf[128] = {0};
	char temp[100]={0};
	char rename_tmp[100]={0};
	int len = 0, t =1;
	
	strcpy(buf,argv[1]);	
	len = strlen(buf);
	for(t =1 ,i=len-1;i>=0;i--){		
		CreatFileNO += (buf[i]-48)* t;
		t = t*10;
	}
	printf("buf:%s,  len:%d, CreatFileNO:%d\n",buf,len,CreatFileNO);	

	strcpy(buf,argv[2]);	
	len = strlen(buf);
	for(t =1 ,i=len-1;i>=0;i--){		
		RenameNO += (buf[i]-48)* t;
		t = t*10;
	}
	printf("buf:%s,  len:%d, RenameNO:%d\n",buf,len,RenameNO);	
	
	strcpy(buf,argv[3]);	
	len = strlen(buf);
	for(t =1 ,i=len-1;i>=0;i--){		
		UnlinkFileNO += (buf[i]-48)* t;
		t = t*10;
	}
	printf("buf:%s,  len:%d, UnlinkFileNO:%d\n",buf,len,UnlinkFileNO);		

	for(i=0;i<CreatFileNO;i++){
		printf("\n ++++++++++++++++++++++++++++++begin open file%d.txt ++++++++++++++++++++++++++++++ \n",i);	
		memset(temp,0,100);
		sprintf(temp,"/mnt/usb/%.4d.txt",i);
		 fd = open(temp, O_RDWR|O_CREAT, 0);
	        if (fd<0)
		 {
	                  printf("\n+++++++++++++++++++++++++++++++open file%d.txt fail++++++++++++++++++++++++++++++++\n",i);
	                 continue;
	         }
		printf("\n ++++++++++++++++++++++++++++++open file%d.txt ok++++++++++++++++++++++++++++++ \n",i);	
		close(fd);
	}
	if(i<RenameNO){
		RenameNO = i;
		printf("creat file num %d < RenameNo %d \n",i,RenameNO);
	}
	
	for(i=0;i<RenameNO;i++){
		printf("\n ++++++++++++++++++++++++++++++begin rename file%d.txt ++++++++++++++++++++++++++++++ \n",i);			

		memset(temp,0,100);
		sprintf(temp,"/mnt/usb/%.4d.txt",i);
		sprintf(rename_tmp,"/mnt/usb/%.4d_rename.txt",i);
		 fd = rename(temp,rename_tmp);
	        if (fd<0)
		 {
	                  printf("\n ++++++++++++++++++++++++++++++rename file%d.txt fail ++++++++++++++++++++++++++++++\n",i);
	                  continue;
	         }
		printf("\n ++++++++++++++++++++++++++++++rename file%d.txt ok++++++++++++++++++++++++++++++ \n",i);			
	}
	
	for(i=0;i<UnlinkFileNO;i++){
		printf("\n++++++++++++++++++++++++++++++begin unlink file%d.txt ++++++++++++++++++++++++++++++ \n",i);
		memset(temp,0,100);
		sprintf(temp,"/mnt/usb/%.4d_rename.txt",i);
		 fd = unlink(temp);
	        if (fd<0)
		 {
	                  printf("\n++++++++++++++++++++++++++++++unlink file%d.txt fail ++++++++++++++++++++++++++++++\n ",i);
	                  continue;
	         }
		printf("\n++++++++++++++++++++++++++++++unlink file%d.txt ok++++++++++++++++++++++++++++++ \n",i);
	}
	
	return 0;


	
}










