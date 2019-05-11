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

	
unsigned int  FS_API_read(const  char *Filename,unsigned int  len_per_read,unsigned int  FileSize,INT8U *Buf,unsigned int is_directio)
{
	
	INT32U    iLoop ;
	int fd  =0;     
	unsigned  char *buffer = Buf;     
	int mask;
	ssize_t   ReadCnt = 0;
	struct timeval tpstart,tpend; 

	mask = O_RDONLY;
	if(is_directio)
		mask |= O_DIRECT;
			
	fd = open(Filename, mask, 0);
	if (fd<0){
		  printf("open file fail\n");
		  return fd;
	}
		 
	//if(posix_fadvise(fd,0,FileSize, Advise)!=0)
	//	printf("posix_fadives set error \n");
	
    	lseek(fd,0,SEEK_SET);
	for(iLoop=0;iLoop<FileSize;iLoop+=len_per_read)
	{  
		ReadCnt=read(fd, buffer, len_per_read); 	
		if(ReadCnt !=len_per_read)
	    {
			break;			  
	    }	  	
	}
	
    printf("iloop is %d,readcnt is %d,len_per_read is %dKB\n",iLoop/len_per_read,ReadCnt,len_per_read/1024);	
      
	close(fd);	
    return 0;
}


unsigned int  FS_API_write(const  char *Filename,unsigned int  len_per_read,unsigned int  FileSize,INT8U *Buf,int is_directio)
{

	INT32U    iLoop; 
	int mask;
	unsigned char *buffer = Buf;       
	ssize_t   WriteCnt = 0;
	int fd =0;

	mask = O_RDWR|O_CREAT;
	if(is_directio)
		mask += O_DIRECT;

	fd = open(Filename, mask, 0);
	if (fd<0){
		  printf("open file fail\n");
		  return fd;
	}
	 
	lseek(fd,0,SEEK_SET); 	 	  
	for(iLoop=0x00;iLoop< FileSize;iLoop+=len_per_read){     
		WriteCnt=write(fd, buffer, len_per_read); 
		if(WriteCnt !=len_per_read){	                		
			break;							
		}     

	}	

	printf("\niloop is %d,WriteCnt is %d,len_per_Write is %dKB \n",iLoop/len_per_read,WriteCnt,len_per_read/1024);
	close(fd);	
    return 0;
}

int  main(int argc,char* argv[])
{
	INT32U  FileSize =   10*1024*1024;
	INT8U   *TmpBuf; 
	
	char  filename[256];
	char *tmp;
	INT32U   iLoop;	
	struct timeval tpstart,tpend; 
	INT32U timeuse; 
	INT32U wSpeed = 0;       
	unsigned int Sector;
	unsigned int Advise = 0;	
	
	memset(filename,0,sizeof(char)*256);
	strcpy(filename,argv[1]);
	printf("ReadFIle:%s\n",filename);

	TmpBuf = (void *)valloc(64*1024);
	memset(TmpBuf,0x0,sizeof(TmpBuf));

	printf("===========begin to test file VFS Read TmpBuf:%x==========\n",(unsigned int)TmpBuf); 
	printf("ReadfileSize:%d,%dKB  Name:%s \n",FileSize,FileSize/1024,filename);	
	printf("\n============READ DIRECT_IO================\n");
	for(iLoop = 0x01;iLoop<8;iLoop++)
	{         
		Sector  =  (1<<iLoop)*512;  

		gettimeofday(&tpstart,NULL); 

		FS_API_read(filename,Sector,FileSize,TmpBuf,1);
	
		gettimeofday(&tpend,NULL); 

		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		timeuse/=1000;                     
		wSpeed =((FileSize/1024)*1000)/(timeuse);
				
		printf("\n tpstart:  %x,%x\n",(INT32U)tpstart.tv_sec,(INT32U)tpstart.tv_usec); 
		printf(" tpend: %x,%x\n",(INT32U)tpend.tv_sec,(INT32U)tpend.tv_usec);
		printf("\n%5dKB,  %5dKB,%5dms,%d KB/S\n",Sector/1024,FileSize/1024,timeuse,wSpeed);     
	} 

	printf("\n============READ BUFFER IO ================\n");
	memset(TmpBuf,0xaa,sizeof(TmpBuf));
	for(iLoop = 0x01;iLoop<8;iLoop++)
	{         
		Sector  =  (1<<iLoop)*512;  

		gettimeofday(&tpstart,NULL); 

		FS_API_read(filename,Sector,FileSize,TmpBuf,0);
	
		gettimeofday(&tpend,NULL); 

		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		timeuse/=1000;                     
		wSpeed =((FileSize/1024)*1000)/(timeuse);
				
		printf("\n tpstart:  %x,%x\n",(INT32U)tpstart.tv_sec,(INT32U)tpstart.tv_usec); 
		printf(" tpend: %x,%x\n",(INT32U)tpend.tv_sec,(INT32U)tpend.tv_usec);
		printf("\n%5dKB,  %5dKB,%5dms,%d KB/S\n",Sector/1024,FileSize/1024,timeuse,wSpeed);     
	} 
/*		
	tmp = strrchr(filename,'/');
	strcpy(++tmp,"test1.bin");
	
	printf("===========begin to test file VFS Write TmpBuf:%x==========\n",(unsigned int)TmpBuf); 
	printf("WritefileSize:%d,%dKB  Name:%s \n",FileSize,FileSize/1024,filename);	
	printf("\n============WRITE DIRECT IO ================\n");
	memset(TmpBuf,0xaa,sizeof(TmpBuf));
	for(iLoop = 0x01;iLoop<8;iLoop++){         
		Sector  =  (1<<iLoop)*512;    

		gettimeofday(&tpstart,NULL); 

		FS_API_write(filename,Sector,FileSize,TmpBuf,1);

		gettimeofday(&tpend,NULL); 

		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		//get ms
		timeuse/=1000;     

		wSpeed =((FileSize/1024)*1000)/(timeuse);

		printf("  %5dKB,  %5dKB,%5dms,%d KB/S\n",Sector/1024,FileSize/1024,timeuse,wSpeed);    
		printf("\ntpend:  %x,%x\n",(INT32U)tpstart.tv_sec,(INT32U)tpstart.tv_usec);                
		printf("tpstart: %x,%x\n",(INT32U)tpend.tv_sec,(INT32U)tpend.tv_usec);

	}  

	
	memset(TmpBuf,0xaa,sizeof(TmpBuf));
	printf("\n============WRITE BUFFER IO ================\n");
	for(iLoop = 0x01;iLoop<8;iLoop++){         
		Sector  =  (1<<iLoop)*512;    
		
		gettimeofday(&tpstart,NULL); 
		
		FS_API_write(filename,Sector,FileSize,TmpBuf,0);
		
		gettimeofday(&tpend,NULL); 
		
		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
		//get ms
		timeuse/=1000;     

		wSpeed =((FileSize/1024)*1000)/(timeuse);

		printf("  %5dKB,  %5dKB,%5dms,%d KB/S\n",Sector/1024,FileSize/1024,timeuse,wSpeed);    
		printf("\ntpend:  %x,%x\n",(INT32U)tpstart.tv_sec,(INT32U)tpstart.tv_usec);                
		printf("tpstart: %x,%x\n",(INT32U)tpend.tv_sec,(INT32U)tpend.tv_usec);

	}  
*/
	free(TmpBuf);
	exit(0) ;
}










