
#include "write_flash.h"
/*#include <stdlib.h>
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
*/
unsigned int static m_check(unsigned char * buf, unsigned int longth)
{
    unsigned int m_sum=0;
	unsigned int i;
	for(i=0;i<longth;i++,buf++)
	{
		m_sum+=*buf;
	}
	return m_sum;
}

unsigned int static mk_filename(unsigned char * mfilename,unsigned char num)
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

//return 01：ok   00: fail  
unsigned int static rw(unsigned char * sdfilename,unsigned char * nffilename,unsigned char * buf,unsigned int w_sum)
{
	unsigned int filesize=0; 
	unsigned char *TmpBuf=buf;
	unsigned char *filename2=nffilename;//将在flash中生成的bin文件name
	unsigned char *filename1=sdfilename;  //卡中的bin文件name
	int fp1,fp2;  //fp1指向卡文件，fp2指向flash文件
	unsigned int loop,Fileoffset;
	unsigned int checksum=0;
	unsigned int SDchecksum=0;
	unsigned char loop2;
	struct stat mstatbuf;
    static unsigned int filenum=1;
	//Trace_MSG("===========begin to test file VFS write from sdcard TmpBuf:%x==========\n",TmpBuf);
	fp1 = open(filename1, O_RDONLY|00040000);
	if(fp1 < 0)
	{
		Trace_MSG("<Test Info> filename:%s,open file fail\n",filename1);
		return 0x00;
	}
	if(fstat(fp1,&mstatbuf)<0)
	{
	    printf("<Test Info> find filesize error!\n");
		return 0x00;
	}
	//fstat(fp1,&mstatbuf);
	//printf("mstatbuf.size== %d\n",mstatbuf.st_size);
	filesize=mstatbuf.st_size;
	//Trace_MSG("ReadfileSize:%d,%dKB  Name:%s \n",filesize,filesize/1024,filename1);	
    printf("<Test Info> file%d.size: %dKB\n",filenum,filesize/1024);
    filenum++;
	
	for (loop2=0;loop2<w_sum;loop2++)
	{
		fp2 = open(filename2, O_RDWR|O_CREAT, 0);
		if(fp2 < 0)
		{
			Trace_MSG("<Test Info> filename:%s,write file fail\n",filename2);
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
			Trace_MSG("<Test Info> filename:%s,write file fail\n",filename2);
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
			printf("<Test Info> write file : %s from sdcard error  at: %d times!\n",filename2,loop2+1);
			return 0x00;
		}
		close(fp2);
		fp2=NULL;
		
		
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
	return 0x01;
}

/*int main()
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
	
}*/

//return 0:0k    1:fail
int Write_flash(unsigned int sum_param,char *param_buf)
{
	unsigned char *TmpBuf=(void *)malloc(16*1024);
	int param[10];
	char t_buf[10];
	char *tp=param_buf;
	int t_i=0;
	char mediafilename[50];
	memset(mediafilename,0,sizeof(char)*50);
	for (t_i=0;t_i<sum_param;t_i++)
	{
		strcpy(t_buf,tp);
		param[t_i]=atoi(t_buf);
		tp+=strlen(tp)+1;
	}//完成参数的转化，char-->int
	if (param[0]==0)//解析介质参数
	{
		strcat(mediafilename,"/mnt/usb1/bin/");
	} 
	else
	{
		strcat(mediafilename,"/mnt/card/bin/");
	}
	char nandfilename[50]="/mnt/udisk/";
	printf("<Test Info> media path == %s\n",mediafilename);
	printf("<Test Info> file sum == %d\n",param[1]);
	printf("<Test Info> write sum == %d\n",param[2]);
	for (t_i=0;t_i<param[1];t_i++)
	{
		mk_filename(mediafilename,t_i);
		mk_filename(nandfilename,t_i);
		if (0==rw(mediafilename,nandfilename,TmpBuf,param[2]))
		{
			printf("<Test Info> write flash error!!\n");
			return 1;
		}
	}

	free(TmpBuf);
    return 0;



}

