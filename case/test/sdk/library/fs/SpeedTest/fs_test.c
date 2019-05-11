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
#include <sys_monitor.h>

//#define CPU 0
//#define correct_test  1;
#define DATA_LENGTH	(256*1024)
static INT8U DIO_SPEED =0;
static INT8U USE_SYSBUF1 =0;
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
unsigned int  FS_Correct_Test(const  char *ReadFilename,const  char *WriteFilename,unsigned int  FileSize)
{
	
	INT32U    iLoop ;
       int fd  =0, fd1 =0 ;	
	 INT8U   *ReadBuffer,*WriteBuffer;   
        ssize_t   ReadCnt = 0 ,WriteCnt = 0;
	 struct timeval tpstart,tpend; 

	 ReadBuffer = (void *)valloc(512);
	if(!ReadBuffer){
		printf("valloc fail\n");
		exit(0);
	}

	 WriteBuffer = (void *)valloc(512);
	if(!WriteBuffer){
		printf("valloc fail\n");
		exit(0);
	}
if(DIO_SPEED)
        fd = open(ReadFilename, O_RDONLY|O_DIRECT, 0);
else
	fd = open(ReadFilename, O_RDONLY, 0);

	 if (fd<0)
	 {
                  printf("open %s fail\n",ReadFilename);
                  return fd;
         }
if(DIO_SPEED)
	 fd1 = open(WriteFilename, O_RDWR|O_DIRECT | O_CREAT, 0);
else
	 fd1 = open(WriteFilename, O_RDWR| O_CREAT, 0);

 	 if (fd1<0)
	 {
                  printf("open %s fail\n",WriteFilename);
                  return fd;
         }
       
	
        lseek(fd,0,SEEK_SET);
	 lseek(fd1,0,SEEK_SET);
	for(iLoop=0;iLoop<FileSize;iLoop+=512)
        {  
        	 printf( "read -------:%ld\n",iLoop);
         	 memset(ReadBuffer,0,512);
		 ReadCnt=read(fd, ReadBuffer, 512); 	
		 _Print_Buf(ReadBuffer,16);
		 printf( "write -------\n");
       		 WriteCnt=write(fd1, ReadBuffer, ReadCnt); 
		 fsync(fd1);
		 lseek(fd1,-512,SEEK_CUR);
		  memset(WriteBuffer,0,512);
		  ReadCnt=read(fd1, WriteBuffer, WriteCnt); 
		   _Print_Buf(WriteBuffer,16);
		if(memcmp(ReadBuffer,WriteBuffer,ReadCnt))
			break;
		  if(ReadCnt !=512 ||WriteCnt !=512 )
               {
               	printf("ReadCnt :%d ,WriteCnt :%d \n",ReadCnt,WriteCnt);
               	break;			  
               }	  	
		
        }	
        if(iLoop >= FileSize-512)
		printf("data is corrent,%ld \n",iLoop+WriteCnt);
	else
		printf("data is wrong,%ld \n",iLoop+WriteCnt);
	 close(fd);	
	 close(fd1);
	 free(ReadBuffer);
	 free(WriteBuffer);
	 
        return 0;
}

	
unsigned int  FS_API_read(const  char *Filename,unsigned int  len_per_read,unsigned int  FileSize,INT8U *Buf,unsigned int Advise)
{
	
	INT32U    iLoop ;
       int fd  =0;     
       unsigned  char *buffer = Buf;       
        ssize_t   ReadCnt = 0;
	 struct timeval tpstart,tpend; 

if(DIO_SPEED)
         	fd = open(Filename, O_RDONLY|O_DIRECT, 0);
else
	 	fd = open(Filename, O_RDONLY, 0);


         if (fd<0)
	 {
                  printf("open file fail\n");
                  return fd;
         }
		 
	//if(posix_fadvise(fd,0,FileSize, Advise)!=0)
	//	printf("posix_fadives set error \n");
	
        lseek(fd,0,SEEK_SET);
	//ReadCnt=read(fd, buffer, len_per_read); 	

	for(iLoop=0;iLoop<FileSize;iLoop+=len_per_read)
        {  
       	   ReadCnt=read(fd, buffer, len_per_read); 	

		//   _Print_Buf(buffer,512);
		//   return 0;
		   if(ReadCnt !=len_per_read)
               {
               	break;			  
               }	  	
		
        }
	
      //printf("iloop is %d,readcnt is %d,len_per_read is %dKB\n",iLoop/len_per_read,ReadCnt,len_per_read/1024);	
        
	 close(fd);	

        return 0;
}

unsigned int  FS_API_write(const  char *Filename,unsigned int  len_per_read,unsigned int  FileSize,INT8U *Buf)
{

	INT32U    iLoop;       
       unsigned char *buffer = Buf;       
        ssize_t   WriteCnt = 0;
	int fd =0;
	
if(DIO_SPEED)
         	fd = open(Filename, O_RDWR|O_DIRECT|O_CREAT, 0);
else
	 	fd = open(Filename, O_RDWR|O_CREAT, 0);

        if (fd<0)
	 {
                  printf("open file fail\n");
                  return fd;
         }
         	 
	  lseek(fd,0,SEEK_SET); 	 	  
        for(iLoop=0x00;iLoop< FileSize;iLoop+=len_per_read)
        {     
        	  WriteCnt=write(fd, buffer, len_per_read); 
  		//  _Print_Buf(buffer,512);
		//   return 0;
		   
                if(WriteCnt !=len_per_read)
                {	                		
				break;							
               }     
        	
	}	
	//printf("iloop is %d,WriteCnt is %d,len_per_Write is %dKB \n",iLoop/len_per_read,WriteCnt,len_per_read/1024);
	
          
	close(fd);	

        return 0;
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
	INT32U  FileSize =   10*1024*1024 ;
	INT8U   *TmpBuf;      
       char  ReadFile[256] ={0};	
	char  WriteFile[256] = {0}; 
	INT32U   iLoop;
       struct timeval tpstart,tpend; 
       INT32U timeuse; 
       INT32U wSpeed = 0;       
	 unsigned int Sector;
	unsigned int Advise = 0;	
	int sysfd;
	struct mem_dev sysbuf;
	char tmp[10]={0};
	int flag = 0;
	int fd = 0;
	struct file * tt;
	
	memset(ReadFile,0,sizeof(char)*256);
	strcpy(ReadFile,argv[1]);
	printf("ReadFIle:%s\n",ReadFile);	

	memset(WriteFile,0,sizeof(char)*256);
	strcpy(WriteFile,argv[2]);
	printf("WriteFile:%s\n",WriteFile);		

/*
	memset(tmp,0,sizeof(char)*10);
	strcpy(tmp,argv[3]);	
	if((strcmp(tmp,"u=1"))==0)
	{		
	      printf("unlink file\n");
		flag = 1;
	}
	if(flag){
	 	fd = unlink(WriteFile);
		if (fd<0)
		{
		          printf("unlink file %s fail\n",WriteFile);
		          return fd;
		 }
		printf("+++++++++++++++unlink file %s ok+++++++++++++++ \n",WriteFile);
	}else{
		 fd = open(ReadFile, O_RDWR|O_CREAT, 0);
		 if(fd<0)
		 {
		     printf("open file %s fail, error =%d\n",ReadFile,errno);
		       //     return fd;
		  }
		printf("+++++++++++++++open file %s ok+++++++++++++++ \n",ReadFile);	
		close(fd);	
	}
	return 0;
*/	
	memset(tmp,0,sizeof(char)*10);
	strcpy(tmp,argv[3]);	
	if((strcmp(tmp,"dio=1"))==0)
	{		
	      printf("dio ==1\n");
		DIO_SPEED = 1;
	}

	memset(tmp,0,sizeof(char)*10);
	strcpy(tmp,argv[4]);	
	if((strcmp(tmp,"sysbuf=1"))==0)
	{		
	 	printf("sysbuf ==1\n");
		USE_SYSBUF1 = 1;
	}
		
	if(USE_SYSBUF1){
		if ( (sysfd = open("/dev/sysbuf", O_RDWR)) < 0){
	  		printf("open file wrong!->return value:%d\n",sysfd);
	  		exit(0);
	 	}else{
	 		sysbuf.request_size = DATA_LENGTH;
			sysbuf.buf_attr = MEM_FS;
			if(ioctl(sysfd,MEM_GET,&sysbuf)){
				printf("sysbuf malloc fail\n");
				close(sysfd);
				exit(0);
			}else{
				TmpBuf = (char*)sysbuf.logic_address;
				printf("sys malloc success:%lx(%lx)\n",sysbuf.logic_address,sysbuf.physic_address);
				memset(TmpBuf,0,DATA_LENGTH);
			}	
		}
	}
	else{ 	
	    	TmpBuf = (void *)valloc(DATA_LENGTH);
		if(!TmpBuf){
			printf("valloc fail\n");
			exit(0);
		}
	}
    
 #ifdef correct_test
 	FS_Correct_Test(ReadFile,WriteFile,FileSize); 	
 #else
	
	printf("===========begin to test file VFS Read TmpBuf:%x==========\n",(unsigned int)TmpBuf); 
	printf("ReadfileSize:%d,%dKB  Name:%s \n",FileSize,FileSize/1024,ReadFile);	
	
	memset(TmpBuf,0x0,sizeof(TmpBuf));

	
	for(iLoop = 0x01;iLoop<8;iLoop++)
	{         
		Sector  =  (1<<iLoop)*512;  
		
#ifdef CPU
		easy_top_start();
		FS_API_read(ReadFile,Sector,FileSize,TmpBuf,Advise);
		easy_top_end();
#else
		gettimeofday(&tpstart,NULL); 
		FS_API_read(ReadFile,Sector,FileSize,TmpBuf,Advise);
		gettimeofday(&tpend,NULL); 

             	timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
             	timeuse/=1000;                     
             	wSpeed =((FileSize/1024)*1000)/(timeuse);
					
		//printf("\n tpstart:  %x,%x\n",(INT32U)tpstart.tv_sec,(INT32U)tpstart.tv_usec); 
		//printf(" tpend: %x,%x\n",(INT32U)tpend.tv_sec,(INT32U)tpend.tv_usec);
            	printf("  %5dKB,  %5dKB,%5dms,%d KB/S\n",Sector/1024,FileSize/1024,timeuse,wSpeed); 
#endif

			    
	} 

	
        printf("===========begin to test file VFS Write TmpBuf:%x==========\n",(unsigned int)TmpBuf); 
        printf("WritefileSize:%d,%dKB  Name:%s \n",FileSize,FileSize/1024,WriteFile);	

	memset(TmpBuf,0xaa,sizeof(TmpBuf));
	
	for(iLoop = 0x01;iLoop<8;iLoop++)
	{         
		Sector  =  (1<<iLoop)*512;    
#ifdef CPU			
		easy_top_start();
		FS_API_write(WriteFile,Sector,FileSize,TmpBuf);
		easy_top_end();
#else
		gettimeofday(&tpstart,NULL); 
		FS_API_write(WriteFile,Sector,FileSize,TmpBuf);
		gettimeofday(&tpend,NULL); 

              timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec;              
              timeuse/=1000;                     
              wSpeed =((FileSize/1024)*1000)/(timeuse);
                
               printf("  %5dKB,  %5dKB,%5dms,%d KB/S\n",Sector/1024,FileSize/1024,timeuse,wSpeed);    
             //  printf("\ntpend:  %x,%x\n",(INT32U)tpstart.tv_sec,(INT32U)tpstart.tv_usec);                
              // printf("tpstart: %x,%x\n",(INT32U)tpend.tv_sec,(INT32U)tpend.tv_usec);
#endif

	}  
	
	if(USE_SYSBUF1){
		ioctl(sysfd,MEM_PUT,&sysbuf);
		close(sysfd);
	}
	else
		free(TmpBuf);
#endif
	return 0;
	
}










