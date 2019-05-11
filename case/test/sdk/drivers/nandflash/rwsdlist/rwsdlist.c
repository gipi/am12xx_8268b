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
#include <sys/stat.h>



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

unsigned int mk_filename(unsigned char * mfilename,unsigned char num)
{
	unsigned char *filename=mfilename;
	unsigned char *tmp=strrchr(filename,'/');
	strcpy(++tmp,"test");//加上test的序号
	tmp=strrchr(filename,'t');
	unsigned char i='1'+num;
	tmp++;
	*tmp=i;
	strcpy(++tmp,".bin");
	return 0x00;
}

unsigned int rw(unsigned char * sdfilename,unsigned char * nffilename,unsigned char * buf)
{
	unsigned int filesize=0; 
	unsigned char *TmpBuf=buf;
	unsigned char *filename2=nffilename;//将在flash中生成的bin文件name
	unsigned char *filename1=sdfilename;  //卡中的bin文件name
	int fp1,fp2;  //fp1指向卡文件，fp2指向flash文件
	unsigned int loop,Fileoffset;
	unsigned int checksum=0;
	unsigned int SDchecksum=0;
	static unsigned int loopsize=0;
	static unsigned int looptimes=1;
	unsigned char loop2;
	struct stat mstatbuf;
	//Trace_MSG("===========begin to test file VFS write from sdcard TmpBuf:%x==========\n",TmpBuf);
	fp1 = open(filename1, O_RDONLY|00040000);
	if(fp1 < 0)
	{
		Trace_MSG("filename:%s,open file fail\n",filename1);
		return 0x00;
	}
	if(fstat(fp1,&mstatbuf)<0)
	{
	    printf("find filesize error!\n");
		return 0x00;
	}
	//fstat(fp1,&mstatbuf);
	//printf("mstatbuf.size== %d\n",mstatbuf.st_size);
	filesize=mstatbuf.st_size;
	Trace_MSG("ReadfileSize:%d,%dKB  Name:%s \n",filesize,filesize/1024,filename1);	
	
	for (loop2=0;loop2<100;loop2++)
	{
		fp2 = open(filename2, O_RDWR|O_CREAT, 0);
		if(fp2 < 0)
		{
			Trace_MSG("filename:%s,write file fail\n",filename2);
			return 0x00;
		}
		//SDchecksum=0;
		for(loop=0;loop<filesize;loop+=0x4000)
		{
			Fileoffset=loop;
			lseek(fp1,Fileoffset,SEEK_SET);
			lseek(fp2,Fileoffset,SEEK_SET);
			if(filesize-loop>0x4000)
			{
				read(fp1,TmpBuf,0x4000);
				write(fp2, TmpBuf, 0x4000);
				if (loop2==0) SDchecksum+=m_check(TmpBuf,0x4000);
			}
			else
			{
				read(fp1,TmpBuf,filesize-loop);
				write(fp2, TmpBuf, filesize-loop);
				if (loop2==0)
				{
					/****找出sd卡中存储的SDchecksum****/
					/*read(fp1,TmpBuf,4);
					SDchecksum=*(unsigned int *)TmpBuf;
					printf("SDchecksum = %d\n",SDchecksum);*/
					
					/****计算出sd卡中存储的SDchecksum****/
					SDchecksum+=m_check(TmpBuf,filesize-loop);
					//printf("SDchecksum = %d\n",SDchecksum);
				}
			}
		}//从卡中写完一次文件到flash中
		close(fp2);
		fp2=NULL;
		
		//printf("write from sdcard over %d times, now begin to check!\n",loop2+1);
		checksum=0;
		fp2 = open(filename2, O_RDONLY|00040000);
		if(fp2 < 0)
		{
			Trace_MSG("filename:%s,write file fail\n",filename2);
			return 0x00;
		}
		lseek(fp2,0,SEEK_SET);
		for(loop=0;loop<filesize;loop+=0x4000)
		{
			Fileoffset=loop;
			lseek(fp2,Fileoffset,SEEK_SET);
			if(filesize-loop>0x4000)
			{
				read(fp2,TmpBuf,0x4000);
				checksum+=m_check(TmpBuf,0x4000);
			}
			else
			{
				read(fp2,TmpBuf,filesize-loop);
				checksum+=m_check(TmpBuf,filesize-loop);
			}
		}
		//printf("write from sdcard: %d times,    checksum==%d\n",(loop2+1),checksum);
		if (checksum!=SDchecksum)
		{
			printf("write file : %s from sdcard error  at: %d times!\n",filename2,loop2+1);
			return 0x00;
		}
		close(fp2);
		fp2=NULL;
		
		loopsize+=filesize;
		
		if((loopsize/1024)>102400)
		{
		    printf("write to nandflash 100MB %d times OK \n", looptimes);
			loopsize=0;
			looptimes++;
		}
	}
	
	/*if (100==loop2)
	{
		printf("Total writefilesize :%d,%dKB  Name:%s \n",filesize*(loop2),filesize*(loop2)/1024,filename2);
	} 
	else
	{
		printf("Total writefilesize :%d,%dKB  Name:%s \n",filesize*(loop2+1),filesize*(loop2+1)/1024,filename2);
	}*/
	
	close(fp1);
	return 0x00;
}

int main()
{
	unsigned char *TmpBuf=(void *)malloc(16*1024);
	unsigned char sdfilename[50]="/mnt/usb1/bin/";  //卡中的bin文件name
	unsigned char nffilename[50]="/mnt/udisk/";  //flash中的bin文件name
	unsigned char out_loop;
	
	while(1)
	{
	    for (out_loop=0;out_loop<4;out_loop++)
	    {
		    mk_filename(sdfilename,out_loop);
		    mk_filename(nffilename,out_loop);
		    printf("read sdcard file: %s , write to nandflash: %s\n",sdfilename,nffilename);
		    rw(sdfilename,nffilename,TmpBuf);
		    printf("write %s over!\n\n",nffilename);
	    }
	}

	free(TmpBuf);
    return 0;
	
}

