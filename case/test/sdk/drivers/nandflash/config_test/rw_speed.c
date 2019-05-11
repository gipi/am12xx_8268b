#include "rw_speed.h"

//return 1:ok  0:fail
int static w_speed(unsigned int flag,char * buf)
{
	unsigned int w_filesize;
	char * TmpBuf=buf;
	int fp;
	unsigned int i_loop,j_loop,read_sector;
	unsigned int timeuse,wspeed;
	struct timeval tpstart,tpend;
	unsigned int fileoffset=0;
	char w_filename[50]="/mnt/udisk/wspeed.bin";
	memset(TmpBuf,0xaa,128*512);
	if (flag==0)
	{
		w_filesize=50*1024*1024;
	} 
	else
	{
		w_filesize=100*1024*1024;
	}
	Trace_MSG("<Test Info> WritefileSize:%d,%dKB  Name:%s \n",w_filesize,w_filesize/1024,w_filename);	
	fp = open(w_filename, O_RDWR|O_CREAT, 0);
	if(fp < 0)
	{
		Trace_MSG("<Test Info> filename:%s,write file fail\n",w_filename);
		return 0x00;
	}
	for (i_loop=1;i_loop<8;i_loop++)
	{
		read_sector=(1<<i_loop)*512;
		gettimeofday(&tpstart);
		for (j_loop=0;j_loop<w_filesize;j_loop+=read_sector)
		{
			fileoffset=j_loop;
			lseek(fp,fileoffset,SEEK_SET);
			if (w_filesize-fileoffset>read_sector)
			{
				write(fp,TmpBuf,read_sector);
			} 
			else
			{
				write(fp,TmpBuf,w_filesize-fileoffset);
			}
		}
		gettimeofday(&tpend);
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		//get ms
		timeuse/=1000;     
		
		wspeed =((w_filesize/1024)*1000)/(timeuse);
		
		Trace_MSG("<Test Info>   %5dKB,  %5dKB,%5dms,  %d KB/S\n",
			read_sector/1024,w_filesize/1024,timeuse, wspeed);

	}

	close(fp);		 
	//Trace_MSG("run to :%s,%s\n",__FILE__,__func__);

	return 0x01;

}

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
//return filesize：ok   00: fail  
unsigned int static rd(unsigned char * sdfilename,unsigned char * nffilename,unsigned char * buf,unsigned int w_sum)
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
    //static unsigned int filenum=1;
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
    printf("<Test Info> file.size: %dKB\n",filesize/1024);
    //filenum++;
	
	////////////////////////////////////
	//for (loop2=0;loop2<w_sum;loop2++)
	//{
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
			//if (loop2==0) 
			SDchecksum+=m_check(TmpBuf,0x4000);
		}
		else
		{
			read(fp1,TmpBuf,filesize-loop);
			write(fp2, TmpBuf, filesize-loop);
			//if (loop2==0)
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
	
	
	for (loop2=0;loop2<w_sum;loop2++)
	{
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
			printf("<Test Info> read file : %s from media error  at: %d times!\n",filename2,loop2+1);
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
	return filesize;
}

//return 1:ok  0:fail
int static r_speed(unsigned int flag,char * buf)
{
	char * TmpBuf=buf;

	unsigned int i_loop,j_loop,read_sector;
	unsigned int timeuse,wspeed;
	struct timeval tpstart,tpend;
	unsigned int fileoffset=0;

	int fp;
	unsigned int filesize=0;
	char n_filename[50]="/mnt/udisk/rspeed.bin";
	char m_filename[50];
	memset(m_filename,0,sizeof(char)*50);
	if (flag==0)
	{
		strcat(m_filename,"/mnt/usb1/rspeed.bin");
	} 
	else
	{
		strcat(m_filename,"/mnt/card/rspeed.bin");
	}

	filesize=rd(m_filename,n_filename,TmpBuf,1);
	if (0==filesize)
	{
		return 0x00;
	}

	Trace_MSG("<Test Info> ReadfileSize:%d,%dKB  Name:%s \n",filesize,filesize/1024,n_filename);
	fp = open(n_filename, O_RDONLY|00040000);
	if(fp < 0)
	{
		Trace_MSG("<Test Info> filename:%s,read file fail\n",n_filename);
		return 0x00;
	}

	for (i_loop=1;i_loop<8;i_loop++)
	{
		read_sector=(1<<i_loop)*512;
		gettimeofday(&tpstart);
		for (j_loop=0;j_loop<filesize;j_loop+=read_sector)
		{
			fileoffset=j_loop;
			lseek(fp,fileoffset,SEEK_SET);
			if (filesize-fileoffset>read_sector)
			{
				read(fp,TmpBuf,read_sector);
			} 
			else
			{
				read(fp,TmpBuf,filesize-fileoffset);
			}
		}
		gettimeofday(&tpend);
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		//get ms
		timeuse/=1000;     
		
		wspeed =((filesize/1024)*1000)/(timeuse);
		
		Trace_MSG("<Test Info>   %5dKB,  %5dKB,%5dms,  %d KB/S\n",
			read_sector/1024,filesize/1024,timeuse, wspeed);
		
	}


	close(fp);		 
	
	
	return 0x01;


}

//return 0:ok  1:fail
int RW_Speed(unsigned int sum_param,char * param_buf)
{
	int param[10];
	char t_buf[10];
	char *tp=param_buf;
	int t_i=0;
	char * TmpBuf=(void*)malloc(128*512);
	for (t_i=0;t_i<sum_param;t_i++)
	{
		strcpy(t_buf,tp);
		param[t_i]=atoi(t_buf);
		tp+=strlen(tp)+1;
	}//完成参数的转化，char-->int
	if (param[0]==0)
	{
		printf("<Test Info> test write speed begin...\n");
		if (param[1]==0)
		{
			printf("<Test Info> write filesize is 50MB \n");
		} 
		else
		{
			printf("<Test Info> write filesize is 100MB \n");
		}
		///////////////////////////////
		//插入写速度函数
		if (0==w_speed(param[1],TmpBuf))
		{
			return 1;
		}
	} 
	else
	{
		printf("<Test Info> test read speed begin...\n");
		if (param[1]==0)
		{
			printf("<Test Info> read file media is upan \n");
		} 
		else
		{
			printf("<Test Info> read file media is sdcard \n");
		}
		///////////////////////////////
		//插入写速度函数
		if (0==r_speed(param[1],TmpBuf))
		{
			return 1;
		}
	}

	free(TmpBuf);
    Trace_MSG("<Test Info> run to :%s,%s\n",__FILE__,__func__);
	return  0x00 ;
}