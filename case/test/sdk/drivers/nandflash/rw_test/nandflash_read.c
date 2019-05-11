
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#if 1
#define  Trace_MSG(fmt,msg...)  printf(fmt,##msg)
#else
#define  Trace_MSG(fmt,msg...)  do {} while (0)
#endif

unsigned int m_check(unsigned char * buf, unsigned int longth)
{
    unsigned int m_sum=0;
	unsigned int i;
	for(i=0;i<longth;i++,buf++)
	{
	    m_sum+=*buf;
	}
	return m_sum;
}

/*int main(void)
{
    unsigned int filesize=0x9b06a;
	unsigned char *TmpBuf=(void *)malloc(2*512);
	unsigned char filename[50]="/mnt/udisk/test.bin";
	int fp;
	unsigned int loop,Fileoffset;
	unsigned int checksum=0;
	Trace_MSG("===========begin to test file VFS Read TmpBuf:%x==========\n",TmpBuf); 
	Trace_MSG("ReadfileSize:%d,%dKB  Name:%s \n",filesize,filesize/1024,filename);	
	 fp = open(filename, O_RDONLY|00040000);
	 if(fp < 0)
	 {
	         Trace_MSG("filename:%s,open file fail\n",filename);
	         return 0x00;
	 }
    for(loop=0;loop<filesize;loop+=1024)
	{
	    Fileoffset=loop;
		lseek(fp,Fileoffset,SEEK_SET);
		if(filesize-loop>1024)
		    {
			    read(fp,TmpBuf,1024);
			    checksum+=m_check(TmpBuf,1024);
			}
		else
		    {
			    read(fp,TmpBuf,filesize-loop);
				checksum+=m_check(TmpBuf,filesize-loop);
			}
	}
	
	printf("checksum==%d\n",checksum);
	
	close(fp);
	free(TmpBuf);
    return 0;
}*/

int main(void)
{
    unsigned int filesize=0x9b06a;
	unsigned char *TmpBuf=(void *)malloc(2*512);
	unsigned char filename[50]="/mnt/udisk/test.bin";
	int fp;
	unsigned int loop,Fileoffset;
	unsigned int checksum=0;
	unsigned char loop2;
	Trace_MSG("===========begin to test file VFS Read TmpBuf:%x==========\n",TmpBuf);
	Trace_MSG("ReadfileSize:%d,%dKB  Name:%s \n",filesize,filesize/1024,filename);	
	fp = open(filename, O_RDONLY|00040000);
	if(fp < 0)
		{
			Trace_MSG("filename:%s,open file fail\n",filename);
			return 0x00;
		}
	for (loop2=0;loop2<100;loop2++)
	{
	    checksum=0;
		for(loop=0;loop<filesize;loop+=1024)
		{
			Fileoffset=loop;
			lseek(fp,Fileoffset,SEEK_SET);
			if(filesize-loop>1024)
			{
				read(fp,TmpBuf,1024);
				checksum+=m_check(TmpBuf,1024);
			}
			else
			{
				read(fp,TmpBuf,filesize-loop);
				checksum+=m_check(TmpBuf,filesize-loop);
			}
		}
		
		printf("read : %d    checksum==%d\n",(loop2+1),checksum);
		if (checksum!=0x4ddff0e)
		{
			printf("read error at: %d\n",loop2+1);
			break;
		}
	}

	printf("Total readfilesize :%d,%dKB  Name:%s \n",filesize*(loop2),filesize*(loop2)/1024,filename);
	close(fp);
	
	
	int fp2;
	char *tmp = strrchr(filename,'/');
	checksum=0;
	strcpy(++tmp,"test1.bin");
	Trace_MSG("\n\n===========begin to test file VFS Write TmpBuf:%x==========\n",TmpBuf);
	printf("perWritefileSize:%d,%dKB  Name:%s \n",1024,1,filename);
	memset(TmpBuf,0xaa,4);//前4字节设置标志位
	fp2 = open(filename, O_RDWR|O_CREAT, 0);
	if(fp2 < 0)
		{
			Trace_MSG("filename:%s,write file fail\n",filename);
			return 0x00;
		}

	for (loop2=0;loop2<100;loop2++)
	{
		write(fp2, TmpBuf, 1024);  //连续写100次
	}
	close(fp2);
	fp2=NULL;
	
	fp2 = open(filename, O_RDONLY|00040000);
	lseek(fp2,0,SEEK_SET);
	for(loop2=0;loop2<100;loop2+=1024)
	{
	    lseek(fp2,loop2,SEEK_SET);
		read(fp2,TmpBuf,1024);
		checksum=m_check(TmpBuf,1024);
		printf("writre : %d    checksum==%d\n",(loop2+1),checksum);
		if (checksum!=0x1f2e4)
		{
			printf("write error at: %d \n",(loop2+1));
			break;
		}
	}
	if (loop2==100)
	{
		printf("Total writefilesize :%d,%dKB  Name:%s \n",1024*(loop2),loop2,filename);
	} 
	else
	{
		printf("Total writefilesize :%d,%dKB  Name:%s \n",1024*(loop2+1),loop2+1,filename);
	}
	
	close(fp2);
	free(TmpBuf);
    return 0;
}