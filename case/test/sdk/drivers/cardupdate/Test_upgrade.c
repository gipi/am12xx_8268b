
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <upgrade_fw.h>
#include <sys/ioctl.h>
#include <unistd.h>


#if 1
#define  Trace_MSG(fmt,msg...)  printf(fmt,##msg)
#else
#define  Trace_MSG(fmt,msg...)  do {} while (0)
#endif

#if 0
unsigned int _Test_ReadFun_(INT8U *filename,INT8U*WriteName,INT8U *Buf,INT32U FileSize)
{
        INT32U read_Sector;
        INT32U   iLoop,jLoop;
	INT8U   *TmpBuf;
        struct timeval tpstart,tpend; 
        INT32U timeuse; 
        INT32U wSpeed;
        INT8U  *ReadFile;
        int fp;
        INT32U FileOffset;
        INT32U Tmp1,Tmp2;
        ssize_t  bSize;
        
        ReadFile = filename;       
        TmpBuf = Buf;   

       
        Trace_MSG("===========begin to test file VFS Read TmpBuf:%x==========\n",TmpBuf); 
	Trace_MSG("ReadfileSize:%d,%dKB  Name:%s \n",FileSize,FileSize/1024,ReadFile);	
	 fp = open(filename, O_RDONLY|00040000);
	 if(fp < 0)
	 {
	         Trace_MSG("filename:%s,open file fail\n",filename);
	         return 0x00;
	 }
	 for(iLoop = 0x01;iLoop<8;iLoop++)
	{         
		read_Sector  =  (1<<iLoop)*512;    
		
		gettimeofday(&tpstart); 
		FileOffset =0x00;
		 for(jLoop=0x00;jLoop<FileSize;jLoop+=read_Sector)
		 {		
		         FileOffset = jLoop;
		         lseek(fp, FileOffset, SEEK_SET);
		        bSize= read(fp, Buf, read_Sector);
		       // if(bSize !=  (ssize_t)read_Sector)
		        {
		              //  Trace_MSG("Error  : %x,%x,%d,%x \n",jLoop,bSize,read_Sector);
		        }       
		         
		 }		
	
		gettimeofday(&tpend); 

                timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
                //get ms
                timeuse/=1000;     
                
                wSpeed =((FileSize/1024)*1000)/(timeuse);
                
               Trace_MSG("  %5dKB,  %5dKB,%5dms,  %d KB/S\n",
                              read_Sector/1024,FileSize/1024,timeuse, wSpeed);     
                 
	}  
	
	 close(fp);		 
	 Trace_MSG("run to :%s,%s\n",__FILE__,__func__);
	
  
        
}
#endif
Fwu_status_t ap_fw_status =
{
	.prg = INI_PRG,
	.state = S_INIT
};
 void _Print_Buf2( const INT8U * pData, INT16U inLen)
{
	INT16U iLoop;
	//printf("%s", pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%2x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  %d\n",iLoop);
		}
	}
       printf("\n");
	
}
INT32U Test_Update()
{
	int fd ,fd2,ret,i=0;
	INT8U *arg;	
	void *fp1,*fp2;
	unsigned long physic_add1,physic_add2;
	char *buf;
	
	
	arg =(INT8U*)malloc(sizeof(512));
	
	if ( (fd = open("/dev/upgrade", O_RDWR)) < 0){
		printf("open file wrong!->return value:%d\n",fd);
		exit(1);
	}
	//arg = "/mnt/card/up.bin";
	strcpy(arg,"/mnt/card/up.bin");
	ret=ioctl(fd,FWU_SETPATH,arg);
	printf("FWU_SETPATH return %d\n",ret);
	if(ret !=0x0)
		goto OutTest;

	while(ap_fw_status.state-S_FINISHED)
	{
		//delay(50);//mdelay(50); //sleep(1);
		usleep(900);
		ret =ioctl(fd,FWU_GETSTATUS,&ap_fw_status);		

		if(ap_fw_status.prg)
			printf("prg = %d percent!state:%x \n", ap_fw_status.prg,ap_fw_status.state);
	//	ioctl(fd,FWU_GETPRG,arg);	
	//	_Print_Buf2(arg,16);
		
	}
OutTest:	
	ioctl(fd,FWU_EXITUP,arg);	
	close(fd);
	free(arg);
	return 0x00;
}


int main(void)
{	
	INT32U  FileSize;
	INT8U   *TmpBuf;
	struct timeval tpstart,tpend; 
	INT8U  ReadFile[256];
	Trace_MSG("This test code is mainly for system lib'\n");
#if 0
	//TmpBuf =(void*)malloc(128 * 512);
	//FileSize =9*1024*1024;  
	//  strcpy(ReadFile,"/mnt/udisk/test.bin");
	// _Test_ReadFun_(ReadFile,"/mnt/udisk/test1.bin",TmpBuf,FileSize);
	
	

	//free(TmpBuf);
#else
	Test_Update();
#endif
	return  0x00 ;
	
	
}
