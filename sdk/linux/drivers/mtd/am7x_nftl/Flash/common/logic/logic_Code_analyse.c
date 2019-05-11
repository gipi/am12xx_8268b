/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : logic_layer.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-7-19 9:06
*********************************************************************************************************
*/
#include "nand_flash_driver_type.h"
#if (!defined(__KERNEL__)) 
#include "mipsregs.h"
#endif

#define LFI_MAX_SECTOR  ((240*32)/512)  //16 Secotr
#define LFI_OF_SECTOR   (512/32)
#define  MAX_R_SECTOR   16
#if 0

void strprintf( const INT8U    * pad, const INT8U * pData, INT16U inLen)
{
	INT16U  iLoop;
	printf("%s", pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%2X ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  %d\n",iLoop);
		}
	}
	
}
INT32S System_code(INT32U  OffSet)
{

	INT16U i,j,k,n,index,DIRItemcheckOver,FileCheckOver;
	INT16U LFIHeadCheckSum,CurLFIHeadCheckSum;
	INT32U DIRItemsCheckSum,CurItemCheckSum,FileCheckSum,CurFileCheckSum,FileOffset,FileLength;
	INT8U *FlashRdBuf,*FlashReadFileBuf;	
	struct SD_DIR * sd_dir ;// (struct SD_DIR *) FlashRdBuf;
	INT32U Tmp,TotalTime;
	INT32U CapOffset = 0;	
	INT8U *Tmp_Buf;
        INT32U bStepCnt;
        INT32U  FileSector;
        INIT_BOOT("\n****strat of %s****\n",__func__);
  	Tmp_Buf=(INT8U *)malloc(512*MAX_R_SECTOR);
	if(Tmp_Buf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
        FlashRdBuf =(INT8U*)malloc(512*MAX_R_SECTOR);
        if(FlashRdBuf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
        FlashReadFileBuf =(INT8U*)malloc(512*MAX_R_SECTOR);
        if(FlashReadFileBuf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
	/*checksum  LFIHead */	
	//nand_flash_entry(N_LOG_READ, 0,0,0, FlashRdBuf);	/*read LFI head*/	
        FTL_Read(0x00+OffSet, 0x01, FlashRdBuf);
	CapOffset=*(INT32U*)(FlashRdBuf+0x80);
	INIT_BOOT("***offset: %x,SystemSize:0x%x, BackSize:0x%x***\n",OffSet,CapOffset,*(INT32U*)(FlashRdBuf+0x84));
	
	LFIHeadCheckSum=FlashRdBuf[510]+FlashRdBuf[511]*0x100;
	CurLFIHeadCheckSum = 0;
	for (n = 0;n < 510/2; n++) 
	{
		CurLFIHeadCheckSum += (FlashRdBuf[2*n] + FlashRdBuf[2*n+1] * 0x100);
	}
	if (LFIHeadCheckSum!=CurLFIHeadCheckSum) 
	{
		INIT_BOOT("***%s,%d,check LFI head check Error 0x %X,0x%x***\n",__func__,__LINE__,LFIHeadCheckSum,CurLFIHeadCheckSum);
		return -1;
	}

	/*Check LFIHead DIR Items*/
//	DIRItemsCheckSum=(FlashRdBuf[16] + FlashRdBuf[17] * 0x100
//		                   + FlashRdBuf[18] * 0x10000+FlashRdBuf[19] * 0x1000000);
       DIRItemsCheckSum=(FlashRdBuf[28] + FlashRdBuf[29] * 0x100
		                   + FlashRdBuf[30] * 0x10000+FlashRdBuf[31] * 0x1000000);

	CurItemCheckSum=0;
	DIRItemcheckOver=0;
	for(i=1;(i<=LFI_MAX_SECTOR) && (DIRItemcheckOver==0);i++)
	{
		//nand_flash_entry(N_LOG_READ, 0,0,i, FlashRdBuf);/*read LFI Dir Item*/	
                FTL_Read(i+OffSet, 0x01, FlashRdBuf);
		for(j=0;(j<512/32) && (DIRItemcheckOver==0);j++)
		{
			if ((FlashRdBuf[32*j]==0)&& (FlashRdBuf[32*j+1]==0)&& (FlashRdBuf[32*j+2]==0)&& (FlashRdBuf[32*j+3]==0))
			{	
				DIRItemcheckOver=1;	
			}
			else
			{
				for (n=0;n<32/4;n++)
				{
					index=32*j+4*n;
					CurItemCheckSum+=(FlashRdBuf[index] + FlashRdBuf[index+1] * 0x100
					          + FlashRdBuf[index+2] * 0x10000+FlashRdBuf[index+3] * 0x1000000);
				}
			 }
		}
	}

	if(DIRItemsCheckSum !=CurItemCheckSum)
	{
		INIT_BOOT("\n**%s,%d, Check LFIHead DIR Items ***\n",__func__,__LINE__);
		return -1;
	}

	FileCheckOver=0;
	TotalTime=0x00;
	INIT_BOOT("****File Name  Offset   Size  Checksum ***\n");
	for(i=1;(i<=LFI_MAX_SECTOR) && (FileCheckOver==0);i++)
	{		
		//nand_flash_entry(N_LOG_READ, 0,0,i, FlashRdBuf);//read LFI Dir Item	
		FTL_Read(i+OffSet, 0x01, FlashRdBuf);
		for(j=0;(j<LFI_OF_SECTOR) && (FileCheckOver==0);j++)
		{
			if ((FlashRdBuf[32*j]==0)&& (FlashRdBuf[32*j+1]==0)&& (FlashRdBuf[32*j+2]==0)
					&& (FlashRdBuf[32*j+3]==0))
			{	
				FileCheckOver=1;
			}
			else
			{
			        INT32U SectorCnt;
				sd_dir = (struct SD_DIR *) (&FlashRdBuf[32*j]);
                                INIT_BOOT("   %s,%6x,%6x,%10x, ",sd_dir->fname,sd_dir->offset,sd_dir->size,sd_dir->checksum);	
				CurFileCheckSum=0;				
				FileOffset=  sd_dir->offset;	
				FileLength=   sd_dir->size;	
				FileCheckSum=sd_dir->checksum;	
                
                                k=0x00;
				CurFileCheckSum=0x00;
                                SectorCnt = FileLength/512;
                               
                                while(SectorCnt)
                                {
                                         if(SectorCnt>MAX_R_SECTOR)
                                         {
                                                    FileSector =  MAX_R_SECTOR;
                                         }
                                         else
                                         {
                                                 FileSector = SectorCnt;
                                         }
                                         
                                        FTL_Read(FileOffset+k+OffSet, FileSector, FlashReadFileBuf);
                                        for (n = 0;n < (FileSector*512)/4; n++)
					{
				               CurFileCheckSum += (FlashReadFileBuf[4*n] + FlashReadFileBuf[4*n+1] * 0x100 
                                                                + FlashReadFileBuf[4*n+2] * 0x10000+FlashReadFileBuf[4*n+3] * 0x1000000);
				        }
                                        k += FileSector;
                                        SectorCnt -= FileSector;
                                         
                                }
                                
                
				if (FileCheckSum!=CurFileCheckSum) 
				{
					INIT_BOOT("\n****Checksum %x,Cur_Checksum:%x******\n",sd_dir->checksum,CurFileCheckSum);
                                                                        
                                        if(OffSet ==0x00)
                                        {
                                        
                                                 for(k=0;k<FileLength/512;k++)
                                                {	             
                                                        FTL_Read(FileOffset+k, 0x01, FlashReadFileBuf);
                                                        FTL_Read(FileOffset+k+CapOffset, 0x01, Tmp_Buf);
                                                        if(memcmp(Tmp_Buf,FlashReadFileBuf,512*1)!=0x00)
                                                        {
                                                            bStepCnt++;
                                                            INIT_BOOT("%6x,",k);   
                                                            if(0x00 == (bStepCnt+1)%10)
                                                                  printf("\n");
                                                        }
                                                 }
                                                
                                        #if 1
                                                for(k=0;k<FileLength/512;k++)
            				        {	                  
                                                        FTL_Read(FileOffset+k, 0x01, FlashReadFileBuf);
                                                        FTL_Read(FileOffset+k+CapOffset, 0x01, Tmp_Buf);
            						if(memcmp(Tmp_Buf,FlashReadFileBuf,512*1)!=0x00)
            						{
            						        bStepCnt++;
            							INIT_BOOT("%5x,%5x,",k,FileOffset+k);
                                                                strprintf("Code \n",FlashReadFileBuf,512);   
                                                                strprintf("back code \n",Tmp_Buf,512); 
            						}
            				       }
                                      #endif
                                                
                                         }                                  
                                       
				}
                                else
                                {
                                     INIT_BOOT(" Check OK !\n");
                                 }                       
				//INIT_BOOT("T:%5d ms ,%s %6x %4d %d\n",FileLength,sd_dir->fname,sd_dir->size,sd_dir->size/0x200,TotalTime);
			}
		}
	}
    
        free(Tmp_Buf);
        free(FlashRdBuf);
        free(FlashReadFileBuf);

	return 0;
}
static INT32U System_get_file(INT8U *fileName,INT32U Code_offset)
{
	INT16U i,j,k,n,index,DIRItemcheckOver,FileCheckOver;
	INT16U LFIHeadCheckSum,CurLFIHeadCheckSum;
	INT32U DIRItemsCheckSum,CurItemCheckSum,FileCheckSum,CurFileCheckSum,FileOffset,FileLength;
	INT8U *FlashRdBuf,*FlashReadFileBuf;	
	struct SD_DIR * sd_dir ;// (struct SD_DIR *) FlashRdBuf;
	INT32U Tmp,TotalTime;
	INT32U CapOffset = 0;	
	INT8U *Tmp_Buf;
        INT32U bStepCnt;      
        INT32U bDestoryOFFset;
        
        
  	Tmp_Buf=(INT8U *)malloc(512);
	if(Tmp_Buf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
        FlashRdBuf =(INT8U*)malloc(512);
        if(FlashRdBuf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
        FlashReadFileBuf =(INT8U*)malloc(512);
        if(FlashReadFileBuf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
        
        FTL_Read(0x00+Code_offset, 0x01, FlashRdBuf);
	CapOffset=*(INT32U*)(FlashRdBuf+0x80);
	INIT_BOOT("***offset: %x,SystemSize:0x%x, BackSize:0x%x***\n",Code_offset,CapOffset,*(INT32U*)(FlashRdBuf+0x84));
	
	LFIHeadCheckSum=FlashRdBuf[510]+FlashRdBuf[511]*0x100;
	CurLFIHeadCheckSum = 0;
	for (n = 0;n < 510/2; n++) 
	{
		CurLFIHeadCheckSum += (FlashRdBuf[2*n] + FlashRdBuf[2*n+1] * 0x100);
	}
	if (LFIHeadCheckSum!=CurLFIHeadCheckSum) 
	{
		INIT_BOOT("***%s,%d,check LFI head check Error 0x %X,0x%x***\n",__func__,__LINE__,LFIHeadCheckSum,CurLFIHeadCheckSum);
		return -1;
	}

	/*Check LFIHead DIR Items*/
       DIRItemsCheckSum=(FlashRdBuf[28] + FlashRdBuf[29] * 0x100
		                   + FlashRdBuf[30] * 0x10000+FlashRdBuf[31] * 0x1000000);

	CurItemCheckSum=0;
	DIRItemcheckOver=0;
	for(i=1;(i<=LFI_MAX_SECTOR) && (DIRItemcheckOver==0);i++)
	{
		//nand_flash_entry(N_LOG_READ, 0,0,i, FlashRdBuf);/*read LFI Dir Item*/	
                FTL_Read(i+Code_offset, 0x01, FlashRdBuf);
		for(j=0;(j<512/32) && (DIRItemcheckOver==0);j++)
		{
			if ((FlashRdBuf[32*j]==0)&& (FlashRdBuf[32*j+1]==0)&& (FlashRdBuf[32*j+2]==0)&& (FlashRdBuf[32*j+3]==0))
			{	
				DIRItemcheckOver=1;	
			}
			else
			{
				for (n=0;n<32/4;n++)
				{
					index=32*j+4*n;
					CurItemCheckSum+=(FlashRdBuf[index] + FlashRdBuf[index+1] * 0x100
					          + FlashRdBuf[index+2] * 0x10000+FlashRdBuf[index+3] * 0x1000000);
				}
			 }
		}
	}

	if(DIRItemsCheckSum !=CurItemCheckSum)
	{
		INIT_BOOT("\n**%s,%d, Check LFIHead DIR Items ***\n",__func__,__LINE__);
		return -1;
	}

	FileCheckOver=0;
	TotalTime=0x00;
	INIT_BOOT("****File Name  Offset   Size  Checksum ***\n");
	for(i=1;(i<=LFI_MAX_SECTOR) && (FileCheckOver==0);i++)
	{		
		//nand_flash_entry(N_LOG_READ, 0,0,i, FlashRdBuf);//read LFI Dir Item	
		FTL_Read(i+Code_offset, 0x01, FlashRdBuf);
		for(j=0;(j<LFI_OF_SECTOR) && (FileCheckOver==0);j++)
		{
			if ((FlashRdBuf[32*j]==0)&& (FlashRdBuf[32*j+1]==0)&& (FlashRdBuf[32*j+2]==0)
					&& (FlashRdBuf[32*j+3]==0))
			{	
				FileCheckOver=1;
			}
			else
			{				
				sd_dir = (struct SD_DIR *) (&FlashRdBuf[32*j]);
                                INIT_BOOT("   %s,%6x,%6x,%8x, \n",sd_dir->fname,sd_dir->offset,sd_dir->size,sd_dir->checksum);	
				CurFileCheckSum=0;				
				FileOffset=  sd_dir->offset;	
				FileLength=   sd_dir->size;	
				FileCheckSum=sd_dir->checksum;				
				CurFileCheckSum=0x00;
                                if(0x00==strcmp(sd_dir->fname,fileName))
                                 {
                                        bDestoryOFFset =0x00;
                                  //      bDestoryOFFset =(FileLength/512)/2;
                                     //   bDestoryOFFset =(FileLength/512)/4;
                                  //      bDestoryOFFset =(FileLength/512)/6;
                                  //      bDestoryOFFset =(FileLength/512)/7;
                                   bDestoryOFFset =(FileLength/512)/2  +100;
                                         printf("offset:%d\n",bDestoryOFFset+FileOffset+Code_offset);
                                         printf("FileOffset:%d,bDestoryOFFset:%d,Code_offset:%d\n",FileOffset,bDestoryOFFset,Code_offset);
                                         memset(FlashRdBuf,0x00,512);
                                         bDestoryOFFset =FileOffset+bDestoryOFFset+Code_offset;
                                        for(k=bDestoryOFFset;k<bDestoryOFFset+4;k++)
                                         {
                                               printf("%d,\n",k);
                                               FTL_Write(k, 0x01, FlashRdBuf);
                                         }
                                        
                                        FTL_Exit();
                                        CurFileCheckSum =0x00;
                              /*          for(k=0;k<FileLength/512;k++)
        				{        					
        					FTL_Read(FileOffset+k+Code_offset, 0x01, FlashReadFileBuf);
        					for (n = 0;n < 512/4; n++)
        					{
        				               CurFileCheckSum += (FlashReadFileBuf[4*n] + FlashReadFileBuf[4*n+1] * 0x100  + FlashReadFileBuf[4*n+2] * 0x10000+FlashReadFileBuf[4*n+3] * 0x1000000);
        				         }
        				}*/
                                        INIT_BOOT("sysChecksum %x,%x********\n",FileCheckSum,CurFileCheckSum);
                                        //FileCheckSum!=CurFileCheckSum
                                        
                                        return ;
                                 }					       
				
			}
		}
	}
    
        free(Tmp_Buf);
        free(FlashRdBuf);
        free(FlashReadFileBuf);

}
INT32S System_code_Destory()
{      
	System_get_file("EASY    RES",0x00);
        
	return 0;
}
INT32S System_code_check()
{
        INT16U i,n;
	INT16U LFIHeadCheckSum,CurLFIHeadCheckSum;
	INT32U CurFileCheckSum;
	INT8U *FlashRdBuf;	
	struct SD_DIR * sd_dir ;// (struct SD_DIR *) FlashRdBuf;
	INT32U CapOffset ,CapoffsetBak;	      
        INT32U sysCode_Size;  
        INIT_BOOT("\n****Start of %s****\n",__func__);
        FlashRdBuf =(INT8U*)malloc(512);
        if(FlashRdBuf ==0x00)
	{
		INIT_BOOT("%s,%s,%d mallc Error\n",__FILE__,__func__,__LINE__);
		return -1;
	}
     
        CapOffset =CapoffsetBak =0x00;
        
	/*checksum  LFIHead */		
        FTL_Read(0x00, 0x01, FlashRdBuf);
	CapOffset=*(INT32U*)(FlashRdBuf+0x80);
        CapoffsetBak =*(INT32U*)(FlashRdBuf+0x84);
	INIT_BOOT("***system_check,SystemSize:0x%x, BackSize:0x%x***\n",CapOffset,CapoffsetBak);
	
	LFIHeadCheckSum=FlashRdBuf[510]+FlashRdBuf[511]*0x100;
	CurLFIHeadCheckSum = 0;
	for (n = 0;n < 510/2; n++) 
	{
		CurLFIHeadCheckSum += (FlashRdBuf[2*n] + FlashRdBuf[2*n+1] * 0x100);
	}
	if (LFIHeadCheckSum!=CurLFIHeadCheckSum) 
	{
		INIT_BOOT("***%s,%d,check LFI head check Error 0x %X,0x%x***\n",__func__,__LINE__,LFIHeadCheckSum,CurLFIHeadCheckSum);
		return -1;
	}
      
    
        System_code(0x00);
        
        printf("\n\n****CapOffset: %x,CapoffsetBak:%x ****\n",CapOffset,CapoffsetBak);
        if(CapOffset == CapoffsetBak)
                 sysCode_Size =CapOffset;
        else 
        {
                sysCode_Size =0x00;   
                printf("\n\n==================================================\n");
                printf("===========NO back code  ===========\n");
                printf("==================================================\n");
                return 0x00;
                 
        }
         printf("\n\n=========================================\n");
         printf("===========start check Back code===========\n");
         printf("=========================================\n");
        FTL_Read(0x00+sysCode_Size, 0x01, FlashRdBuf);
	CapOffset=*(INT32U*)(FlashRdBuf+0x80);
	INIT_BOOT("***system_check,SystemSize:0x%x, BackSize:0x%x***\n",CapOffset,*(INT32U*)(FlashRdBuf+0x84));
	
	LFIHeadCheckSum=FlashRdBuf[510]+FlashRdBuf[511]*0x100;
	CurLFIHeadCheckSum = 0;
	for (n = 0;n < 510/2; n++) 
	{
		CurLFIHeadCheckSum += (FlashRdBuf[2*n] + FlashRdBuf[2*n+1] * 0x100);
	}
	if (LFIHeadCheckSum!=CurLFIHeadCheckSum) 
	{
		INIT_BOOT("***%s,%d,check LFI head check Error 0x %X,0x%x***\n",__func__,__LINE__,LFIHeadCheckSum,CurLFIHeadCheckSum);
		return -1;
	}   
    
	System_code(sysCode_Size);
        free(FlashRdBuf);
        
	 INIT_BOOT("\n****end of %s****\n",__func__);
	return 0;
}
INT32S  Flash_CodeCheck(void)
{
       System_code_check();
    /////System_code_Des(0x00);
    //System_code_Destory();
}

#endif
