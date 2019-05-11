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

unsigned int rw(unsigned char * mfilename,unsigned int mfilesize,unsigned char * buf)
{
	unsigned int filesize=mfilesize; 
	unsigned char *TmpBuf=buf;
	unsigned char filename2[50]="/mnt/udisk/test2.bin";//将在flash中生成的bin文件name
	unsigned char *filename1=mfilename;  //卡中的bin文件name
	int fp1,fp2;  //fp1指向卡文件，fp2指向flash文件
	unsigned int loop,Fileoffset;
	unsigned int checksum=0;
	unsigned char loop2;
	Trace_MSG("===========begin to test file VFS write from sdcard TmpBuf:%x==========\n",TmpBuf);
	Trace_MSG("ReadfileSize:%d,%dKB  Name:%s \n",filesize,filesize/1024,filename1);	
	fp1 = open(filename1, O_RDONLY|00040000);
	if(fp1 < 0)
	{
		Trace_MSG("filename:%s,open file fail\n",filename1);
		return 0x00;
	}
	
	for (loop2=0;loop2<100;loop2++)
	{
		fp2 = open(filename2, O_RDWR|O_CREAT, 0);
		if(fp2 < 0)
		{
			Trace_MSG("filename:%s,write file fail\n",filename2);
			return 0x00;
		}
		//checksum=0;
		for(loop=0;loop<filesize;loop+=10240)
		{
			Fileoffset=loop;
			lseek(fp1,Fileoffset,SEEK_SET);
			lseek(fp2,Fileoffset,SEEK_SET);
			if(filesize-loop>10240)
			{
				read(fp1,TmpBuf,10240);
				write(fp2, TmpBuf, 10240);
				//checksum+=m_check(TmpBuf,1024);
			}
			else
			{
				read(fp1,TmpBuf,filesize-loop);
				write(fp2, TmpBuf, filesize-loop);
				//checksum+=m_check(TmpBuf,filesize-loop);
			}
		}//从卡中写完一次文件到flash中
		close(fp2);
		fp2=NULL;
		
		printf("write from sdcard over %d times, now begin to check!\n",loop2+1);
		checksum=0;
		fp2 = open(filename2, O_RDONLY|00040000);
		if(fp2 < 0)
		{
			Trace_MSG("filename:%s,write file fail\n",filename2);
			return 0x00;
		}
		lseek(fp2,0,SEEK_SET);
		for(loop=0;loop<filesize;loop+=10240)
		{
			Fileoffset=loop;
			lseek(fp2,Fileoffset,SEEK_SET);
			if(filesize-loop>10240)
			{
				read(fp2,TmpBuf,10240);
				checksum+=m_check(TmpBuf,10240);
			}
			else
			{
				read(fp2,TmpBuf,filesize-loop);
				checksum+=m_check(TmpBuf,filesize-loop);
			}
		}
		printf("write from sdcard: %d times,    checksum==%d\n",(loop2+1),checksum);
		if (checksum!=0x4ddff0e)
		{
			printf("write from sdcard error  at: %d \n",loop2+1);
			break;
		}
		close(fp2);
		fp2=NULL;
	}
	
	if (100==loop2)
	{
		printf("Total writefilesize :%d,%dKB  Name:%s \n",filesize*(loop2),filesize*(loop2)/1024,filename2);
	} 
	else
	{
		printf("Total writefilesize :%d,%dKB  Name:%s \n",filesize*(loop2+1),filesize*(loop2+1)/1024,filename2);
	}
	
	close(fp1);
	return 0x00;
}

int main()
{
	unsigned int wfilesize=0x9b06a;
	unsigned char *TmpBuf=(void *)malloc(20*512);
	unsigned char filename[50]="/mnt/card/test1.bin";  //卡中的bin文件name
	rw(filename,wfilesize,TmpBuf);
	free(TmpBuf);
    return 0;
	
}