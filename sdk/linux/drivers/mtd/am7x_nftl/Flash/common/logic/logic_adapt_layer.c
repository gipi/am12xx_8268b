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
#include "sys_buf_def.h"
#include "flash_driver_config.h"

extern INT32U FTL_LogicPageRead(INT32U BufPageNum, INT32U Bitmap, void * Buf);
extern INT32U FTL_LogicPageWrite(INT32U BufPageNum, INT32U Bitmap, void * Buf);
extern INT32U _CloseLastWritePage(void);

#if SUPPORT_DMA_TRANSFER_DATA_ON_RAM
extern void   DmaTransferDataForRam(void* SrcAddr, void* DstAddr, INT32S ByteCnt);
#endif
INT32U  FTL_Read(INT32U Lba, INT32U Length, void* SramBuffer);
#if SUPPORT_READ_RECLAIM
extern INT32U FTL_ReadReclaim(INT32U BufPageNum, INT32U Bitmap, void * Buf);
#endif
INT8U bPrintFlag;
INT8U bPrintFlag2;
extern struct NandLogicCtlPar  LogicRWCtlPar;
extern struct SpareData WriteSpareData[2];

extern struct  _Debug_Wear  WearDbg;
extern struct ECC_proetect_Read  Prot_Read;
static struct AdaptProcPagePar  AdaptPagePar = {0xffffffff, 0,{0,0}};
void Str_printf( const INT8U*  pad, const INT8U* pData, INT16U inLen)
{
	INT16U iLoop;   
	printf("%s",pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%02x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  %4d\n",iLoop);
		}
	}
	printf("\n");

}
void Str_printf2( const INT8U*  pad, const INT8U* pData, INT16U inLen)
{
	INT16U iLoop;   
	printf("%s",pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("0x%02x, ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  /*%4d*/\n",iLoop);
		}
	}
	printf("\n");

}
void Dump_mem(void *startaddr, INT32S size, INT32U showaddr, INT32S show_bytes)
{

    INT16U iLoop;   
    INT8U *Buf_8Bit;
    INT16U *Buf_16Bit;
    INT32U *Buf_32Bit;
	
    switch(show_bytes)
    {
        case 0x01://8Bit 
            Buf_8Bit=(INT8U*)startaddr;
            for( iLoop=0; iLoop< size; iLoop++)
            {
                printf("%2x ", Buf_8Bit[iLoop]);
                if( 0== ((iLoop+1) &0x0f) )
                {
                    printf("  :%4x\n",iLoop);
                }
            }
            break;
        case 0x02://16Bit 
            Buf_16Bit=(INT16U*)startaddr;
            for( iLoop=0; iLoop< size/(sizeof(INT16U)); iLoop++)
            {
                printf("%4x ", Buf_16Bit[iLoop]);
                if( 0== ((iLoop+1) &0x0f) )
                {
                    printf("  :%4x\n",iLoop);
                }
            }
            break;
        case 0x04:
            Buf_32Bit=(INT32U*)startaddr;
            for( iLoop=0; iLoop< size/(sizeof(INT32U)); iLoop++)
            {
                printf("%8x ", Buf_32Bit[iLoop]);
                if( 0== ((iLoop+1) &0x07) )
                {
                    printf("  :%4x\n",iLoop);
                }
            }
            break;
        default:
            break;

    }
    printf("\n");	
	
}
void _Short_DelayUS(INT32U DelayTim)
{
	volatile INT32U del=DelayTim*15;
	volatile INT32U  resi=0;
	while(del >0 )
	{
        del--;
        resi= resi+2;
	}
	return;
}
#if (defined(__KERNEL__)   || BREC_DRIVER == 0x01)
INT8U Test_Buf[512];
INT8U *BootBuf=NULL;
INT8U Check_Mbrec_Data(INT8U *BufAdr)
{
    INT32U CurCheckSum,nn;
    INT32U* FlashRdBuf = (INT32U*)BufAdr;

    CurCheckSum = 0;
    for (nn= 0;nn < (512)/4-1; nn++)
         CurCheckSum += * FlashRdBuf ++;
    CurCheckSum += 0x1234;     
    INIT_DBG("#:%x,%x,%x,%x\n",BufAdr[506],BufAdr[507],BufAdr[508],BufAdr[509] );
    if (CurCheckSum != * FlashRdBuf)
    {
            return 0x01;
    }
    else
    {
            return 0x00;
    }
}

INT32U Test_ReadCard_Data(void)
{
    INT8U Result,ii;
    INT8U *Buf;
    INT32U Addr;

#if (defined(__KERNEL__)  ) 
    ////   Buf =(INT8U*)&Test_Buf;
    if(BootBuf ==NULL)
    {
        Addr=(INT32U)(&Test_Buf);
        INIT_BOOT("##Addr:%x###\n",Addr);
        Buf = (INT8U*)(Addr);
    }
    else
    {
        Buf = BootBuf;
    }
#elif BREC_DRIVER == 0x01 

    Addr=(INT32U)(&Test_Buf)+0x20000000;
    Buf = (INT8U*)(Addr);
#endif       
    Result =0x01;       
    for(ii=3;ii>0;ii--)
    {
        Test_ReadMbrecData(ii,0,Buf);
        ///	Str_printf("", Buf, 512);
        if(! Check_Mbrec_Data(Buf))
        {
           
            if((Buf[506] ==0xaa && Buf[507] ==0x55)
                ||( Buf[508] ==0xaa && Buf[509] ==0x55))                      
            {
                Result=0x00;
                break;  
            }  
        }               
    }   
    return Result;
          
}
/*
*/

void Set_TestBuf(void)
{
	
#if (defined(__KERNEL__)  ) 
	BootBuf =(void*)MALLOC(512);
	INIT_BOOT("####run to :%s,%p###\n",__func__,BootBuf);
#elif BREC_DRIVER == 0x01              
#endif  
 	return ;
 
}

void FTL_Correct_ModeInit(void)
{
    INT8U ii;
    INT8U *Buf;
    INT32U Addr;
    Addr =0x00;
#if (defined(__KERNEL__)  ) 
    //Buf =(INT8U*)&Test_Buf;
    BootBuf =(void*)MALLOC(512);
    Buf = BootBuf;
#elif BREC_DRIVER == 0x01       
    Addr=(INT32U)(&Test_Buf)+0x20000000;
    Buf = (INT8U*)(Addr);
#endif  

    for(ii=0x00;ii<4;ii++)
    {
    Prot_Read.MBrcBlk[ii]=0x00;
    }    
    Prot_Read.MultiMbrcFlag =0x00;

    for(ii=0x00;ii<4;ii++)
    {
        Test_ReadMbrecData(ii,0,Buf);
        if(!Check_Mbrec_Data(Buf))
        {
            Prot_Read.MBrcBlk[ii] =0x00;
            if(Buf[506] ==0xaa && Buf[507] ==0x55)  //mulit brec 
            {
                Prot_Read.MultiMbrcFlag =0x01;
            } 
        }  
    }
   ///// INIT_DBG("###CodeSize:%x,%x ###\n",Prot_Read.CodeSize,Prot_Read.MultiMbrcFlag);
                
}
#endif
/*
*********************************************************************************************************
*               CALCLUATE THE GLOBAL PAGE PARAMTER
*
*Description: calculate the page parameter of the adaptor layer current processing.
*
*Arguments  : Lba           global logic sector number;
*             Length        the sector count that current need process;
*             PagePar       the page parameter of the current processing data;
*
*Return     : none
*********************************************************************************************************
*/
static void  _CalculatePagePar(INT32U Lba, INT32U Length, struct AdaptProcPagePar *HeadPagePar,
                                    INT32U *MiddPageNum, struct AdaptProcPagePar *TailPagePar)
{
    INT32U  tmpLength;
    INT32U  tmpBitmap;
    INT32U  temp;

    FTL_DBG("FTL_DBG: calculate buffer page parameter\n");
    FTL_DBG("         logical sector address:0x%x, sector count:0x%x\n", Lba, Length);

    tmpLength = Length;

    /* init middle page number, and tail page parameter */
    *MiddPageNum = 0x00;
    TailPagePar->PageNum = 0xffffffff;
    TailPagePar->SectorBitmap = 0x00;

    /* calculate the head page number */
    HeadPagePar->PageNum = Lba / PAGE_BUFFER_SECT_NUM;

    /* calculate the head page sector bitmap */
    temp = Lba % PAGE_BUFFER_SECT_NUM;
    tmpBitmap = FULL_BITMAP_OF_BUFFER_PAGE;
    tmpBitmap <<= temp;
    tmpLength = Length + temp;

    if(tmpLength > PAGE_BUFFER_SECT_NUM)
    {
        /* set head page sector bitmap */
        HeadPagePar->SectorBitmap = tmpBitmap & FULL_BITMAP_OF_BUFFER_PAGE;

        tmpLength -= PAGE_BUFFER_SECT_NUM;

        /* calculate the middle page number */
        while(tmpLength >= PAGE_BUFFER_SECT_NUM)
        {
            *MiddPageNum = *MiddPageNum + 1;
            tmpLength -= PAGE_BUFFER_SECT_NUM;
        }

        /* calculate the tail page parameters */
        if(tmpLength)
        {
            /* calculate tail page sector bitmap */
            tmpBitmap = FULL_BITMAP_OF_BUFFER_PAGE;
            tmpBitmap >>= (PAGE_BUFFER_SECT_NUM - tmpLength);
            TailPagePar->SectorBitmap = tmpBitmap;

            /* calculate tail page number */
            TailPagePar->PageNum = HeadPagePar->PageNum + *MiddPageNum + 1;
        }
    }
    else
    {
        /* set head page sector bitmap */
        HeadPagePar->SectorBitmap = tmpBitmap & (FULL_BITMAP_OF_BUFFER_PAGE >> (PAGE_BUFFER_SECT_NUM - tmpLength));
    }

    FTL_DBG("         HPageNum:0x%x, HPageBitmap:0x%x\n", HeadPagePar->PageNum, HeadPagePar->SectorBitmap);
    FTL_DBG("         middle page count:0x%x\n", *MiddPageNum);
    FTL_DBG("         TPageNum:0x%x, TPageBitmap:0x%x\n\n", TailPagePar->PageNum, TailPagePar->SectorBitmap);
}


/*
*********************************************************************************************************
*               MERGE THE DATA OF CURRENT WRITE WITH THE DATA IN BUFFER
*
*Description: if there are some data in buffer, and current write data is in the same page with
*             the buffer caching data.
*
*Arguments  : BufPageBitmap     the pointer to the page buffer bitmap
*             UserPageBitmap    the bitmap of the
*
*Return     : TRUE      calculate ok;
*             FALSE     calculate failed;
*
*Notes      :
*********************************************************************************************************
*/
static void _MergeBufData(INT32U *BufPageBitmap, INT32U UserPageBitmap, void *NandCachePtr, void *UserBufPtr)
{
    INT32S i;
    INT8U  *tmpSrc, *tmpDst;
    INT32U temp1,temp2;
	

    temp1 = 0;
    temp2 = 0;

    for(i=0; i<PAGE_BUFFER_SECT_NUM; i++)
    {
        tmpSrc = (INT8U*)UserBufPtr + temp2;
        tmpDst = (INT8U*)NandCachePtr + temp1;

        if(UserPageBitmap & (1 << i))
        {
          #if SUPPORT_DMA_TRANSFER_DATA_ON_RAM
//		#ifdef DMA_MMU_MODE	//tl
//			(INT32U)tmpSrc=LogToPhy((INT32U)tmpSrc)+0xa0000000;//tl
//			(INT32U)tmpDst=LogToPhy((INT32U)tmpDst)+0xa0000000;//tl
//		#endif					//tl
	     /* use dma to transfer data on ram */		
		//DmaTransferDataForRam(tmpSrc, tmpDst, NAND_SECTOR_SIZE);
		 MEMCPY(tmpDst, tmpSrc, NAND_SECTOR_SIZE);
		#else
            /* use mips to transfer data on ram */
			MEMCPY(tmpDst, tmpSrc, NAND_SECTOR_SIZE);
		#endif

			temp2 += NAND_SECTOR_SIZE;
        }

        temp1 += NAND_SECTOR_SIZE;
    }

    *BufPageBitmap =  *BufPageBitmap | UserPageBitmap;
}


/*
*********************************************************************************************************

*               FLASH TRANSLATE LAYER FLUSH PAGE BUFFER CACHE
*Description: write the data in page buffer cache to nand flash.
*Arguments  : none

*Return     : TRUE      flush ok;
*             FALSE     flush failed;*

*Notes      : this function check if there is some data in page buffer cache, if so, write the data

*             to nand flash, and close the write page.

*********************************************************************************************************
*/
INT32U  FTL_FlushPageCache(void)
{
    INT32U result = TRUE;

    /* check if there is some data in page cache */
    if(AdaptPagePar.PageNum != 0xffffffff)
    {
        /* write buffer data to nand flash */
        result = FTL_LogicPageWrite(AdaptPagePar.PageNum, AdaptPagePar.SectorBitmap, LOGIC_WRITE_PAGE_CACHE);
        /* clear page buffer parameters */
        AdaptPagePar.PageNum = 0xffffffff;
        AdaptPagePar.SectorBitmap = 0x0;
    }

    /* close the write page */
    result |= _CloseLastWritePage();
    if(result != TRUE)
    {
        result = FALSE;
    }

    return result;
}


/*
*********************************************************************************************************
*               FLASH TRANSLATE LAYER SECTOR BASED READ
*
*Description: read data from logic sectors to sram for up-layers.
*
*Arguments  : Lba           global logic sector number;
*             Length        the count of sectors need be read;
*             SramBuffer    the buffer address which the data be read to;
*
*Return     : TRUE     read ok;
*             FALSE    read failed;
*             FALSE|ECCERROR      the ecc error;
*             FALSE|ADDRERROR     the address is invalid;
*
*Notes      :
*********************************************************************************************************
*/
INT32U  FTL_Read(INT32U Lba, INT32U Length, void* SramBuffer)
{
    INT32S i;
    INT32U result;
    INT32U tmpMidPageNum;
    INT32U tmpPageNum;
    INT8U  *tmpBuf;
    struct AdaptProcPagePar HeadPagePar, TailPagePar;

    FTL_DBG("FTL_DBG: FTL_Read\n");
    FTL_DBG("         logical address:0x%x, sector count:0x%x, buffer address:0x%x\n\n", Lba, Length, SramBuffer);

    /* calculate head page parameter, tail page parameter and middle page number */
    _CalculatePagePar(Lba, Length, &HeadPagePar, &tmpMidPageNum, &TailPagePar);

    /* check if need flush page buffer */
    if(AdaptPagePar.PageNum != 0xffffffff)
    {
        /* write buffer data to nand flash */
        result = FTL_LogicPageWrite(AdaptPagePar.PageNum, AdaptPagePar.SectorBitmap, LOGIC_WRITE_PAGE_CACHE);

        /* clear page buffer parameters */
        AdaptPagePar.PageNum = 0xffffffff;
        AdaptPagePar.SectorBitmap = 0x0;
    }

    tmpBuf = (INT8U*)SramBuffer;

    /* read head page */
    result = FTL_LogicPageRead(HeadPagePar.PageNum, HeadPagePar.SectorBitmap, tmpBuf);
    if(result != TRUE)
    {
        if(PHY_READ_ERR(result))
        {
            return FALSE;
        }
        Trace_NF("run to :%s,%d  lba:%x,%d ,%x\n",__func__,__LINE__,Lba,Length,result);
        #if SUPPORT_READ_RECLAIM
        /*be close to ecc limit, reclaim operation*/
        if (PHY_READ_ECC_TO_LIMIT(result))
        {
            FTL_ReadReclaim(HeadPagePar.PageNum, HeadPagePar.SectorBitmap, tmpBuf);
        }
        #endif
    }

    /* calculate user buffer address for next page */
    for(i=0; i<PAGE_BUFFER_SECT_NUM; i++)
    {
        if(HeadPagePar.SectorBitmap & (1ULL << i))
        {
            tmpBuf += NAND_SECTOR_SIZE;
        }
    }

    /* read middle page data */
    tmpPageNum = HeadPagePar.PageNum + 1;
    while(tmpMidPageNum)
    {
        /* read whole page data */
        result = FTL_LogicPageRead(tmpPageNum, FULL_BITMAP_OF_BUFFER_PAGE, tmpBuf);
        if(result != TRUE)
        {
            if(PHY_READ_ERR(result))
            {
                return FALSE;
            }
            Trace_NF("run to :%s,%d  lba:%x,%d ,%x\n",__func__,__LINE__,Lba,Length,result);
            #if SUPPORT_READ_RECLAIM
            /*be close to ecc limit, reclaim operation*/
            if (PHY_READ_ECC_TO_LIMIT(result))
            {
                FTL_ReadReclaim(tmpPageNum, FULL_BITMAP_OF_BUFFER_PAGE, tmpBuf);
            }
            #endif
        }

        /* increase page number and buffer address */
        tmpPageNum ++;
        tmpMidPageNum --;
        tmpBuf += PAGE_BUFFER_SECT_NUM * NAND_SECTOR_SIZE;
    }

    /* read the tail page data */
    if(TailPagePar.SectorBitmap)
    {
        result = FTL_LogicPageRead(TailPagePar.PageNum, TailPagePar.SectorBitmap, tmpBuf);
        if(result != TRUE)
        {
            if (PHY_READ_ERR(result))
            {
                return FALSE;
            }
            Trace_NF("run to :%s,%d  lba:%x,%d ,%x\n",__func__,__LINE__,Lba,Length,result);
            #if SUPPORT_READ_RECLAIM
            /*be close to ecc limit, reclaim operation*/
            if (PHY_READ_ECC_TO_LIMIT(result))
            {
                FTL_ReadReclaim(TailPagePar.PageNum, TailPagePar.SectorBitmap, tmpBuf);
            }
            #endif
       }
    }

    FTL_DBG("FTL_DBG: FTL_Read completely, the return value is:0x%x\n", result);

    return TRUE;
}

/*
*********************************************************************************************************
*               FLASH TRANSLATE LAYER SECTOR BASED WRITE
*
*Description: write data to logic sectors from sram for up-layers.
*
*Arguments  : Lba           global logic sector number;
*             Length        the count of sectors need be read;
*             SramBuffer    the buffer address which the data stored;
*
*Return     : TRUE      write ok;
*             FALSE     write failed;
*             FALSE|NANDERROR   nand flash program error;
*             FALSE|ADDRERROR   the address is invalid;
*
*Notes      :
*********************************************************************************************************
*/
INT32U  FTL_Write(INT32U Lba, INT32U Length, void* SramBuffer)
{
    INT32S i;
    INT32U result;
    INT32U tmpMidPageNum;
    INT32U tmpPageNum;
    INT32U tmpBitmap;
    INT8U  *tmpBuf;

    struct AdaptProcPagePar HeadPagePar, TailPagePar;

    FTL_DBG("FTL_DBG: FTL_Write \n");
    FTL_DBG("         logical address:0x%x, sector count:0x%x, buffer address:0x%x\n\n", Lba, Length, SramBuffer);

    tmpBuf = (INT8U*)SramBuffer;

   // if(Lba<=12*2048)

    /* calculate head page parameter, tail page parameter and middle page number */
    _CalculatePagePar(Lba, Length, &HeadPagePar, &tmpMidPageNum, &TailPagePar);

    /* process buffer data and head page */
    if(HeadPagePar.PageNum == AdaptPagePar.PageNum)
    {
        /* buffer page and user page are the same page, need merge user data and buffer data */
        tmpBitmap = AdaptPagePar.SectorBitmap;
        _MergeBufData(&tmpBitmap, HeadPagePar.SectorBitmap, LOGIC_WRITE_PAGE_CACHE, tmpBuf);
        AdaptPagePar.SectorBitmap = tmpBitmap;

        /* check if the buffer page need write */
        if(tmpBitmap & (1ULL << (PAGE_BUFFER_SECT_NUM - 1)))
        {
            /* write the buffer page to nand flash */
            result = FTL_LogicPageWrite(AdaptPagePar.PageNum, tmpBitmap, LOGIC_WRITE_PAGE_CACHE);
            AdaptPagePar.PageNum = 0xffffffff;
            AdaptPagePar.SectorBitmap = 0x0;
        }
        else
        {
            /* the highest sector data is invalid, process completely */
            return TRUE;
        }
    }
    else
    {
        if(AdaptPagePar.PageNum != 0xffffffff)
        {
            /* the buffer page number is valid and different of the user page, need write first */
            result = FTL_LogicPageWrite(AdaptPagePar.PageNum, AdaptPagePar.SectorBitmap, LOGIC_WRITE_PAGE_CACHE);
            AdaptPagePar.PageNum = 0xffffffff;
            AdaptPagePar.SectorBitmap = 0x0;
        }

        if(HeadPagePar.SectorBitmap != FULL_BITMAP_OF_BUFFER_PAGE)
        {
            /* write current user page with no data, to prepare the write environment */
            result = FTL_LogicPageWrite(HeadPagePar.PageNum, 0, LOGIC_WRITE_PAGE_CACHE);

            tmpBitmap = 0;
            _MergeBufData(&tmpBitmap, HeadPagePar.SectorBitmap, LOGIC_WRITE_PAGE_CACHE, tmpBuf);

            if(tmpMidPageNum | TailPagePar.SectorBitmap)
            {
                result = FTL_LogicPageWrite(HeadPagePar.PageNum, tmpBitmap, LOGIC_WRITE_PAGE_CACHE);
            }
            else
            {
                /* user data write completely */
                AdaptPagePar.PageNum = HeadPagePar.PageNum;
                AdaptPagePar.SectorBitmap = tmpBitmap;

                return TRUE;
            }
        }
        else
        {
            result = FTL_LogicPageWrite(HeadPagePar.PageNum, FULL_BITMAP_OF_BUFFER_PAGE, tmpBuf);
        }
    }

    /* calculate user buffer address for next page */
    for(i=0; i<PAGE_BUFFER_SECT_NUM; i++)
    {
        if(HeadPagePar.SectorBitmap & (1ULL << i))
        {
            tmpBuf += NAND_SECTOR_SIZE;
        }
    }

    /* write middle page data */
    tmpPageNum = HeadPagePar.PageNum + 1;
    while(tmpMidPageNum)
    {
        /* read whole page data */
        result = FTL_LogicPageWrite(tmpPageNum, FULL_BITMAP_OF_BUFFER_PAGE, tmpBuf);

        /* increase page number and buffer address */
        tmpPageNum ++;
        tmpMidPageNum --;
        tmpBuf += PAGE_BUFFER_SECT_NUM * NAND_SECTOR_SIZE;
    }

    /* process the tail page */
    if(TailPagePar.SectorBitmap)
    {
        /* write current user page with no data, to prepare the write environment */
        result = FTL_LogicPageWrite(TailPagePar.PageNum, 0, LOGIC_WRITE_PAGE_CACHE);

        /* transfer user page data to page buffer */
        tmpBitmap = 0;
        _MergeBufData(&tmpBitmap, TailPagePar.SectorBitmap, LOGIC_WRITE_PAGE_CACHE, tmpBuf);

        /* set buffer page parameters */
        AdaptPagePar.PageNum = TailPagePar.PageNum;
        AdaptPagePar.SectorBitmap = tmpBitmap;
    }

    FTL_DBG("FTL_DBG: FTL_Write completely, the return value is:0x%x\n", result);

    return TRUE;
}



/*
*********************************************************************************************************
*                       FLASH TRANSLATE LAYER INIT
*
*Description: init nand flash translate layer global parameter, and alloc buffer for FTL.
*
*Arguments  : none;
*
*Return     : TRUE;
*********************************************************************************************************
*/
INT32U  FTL_Param_Init(void)
{
	INT32S  i;

	FTL_DBG("FTL_DBG: FTL_Param_Init.\n");

	/* init global parameters */
	LogicRWCtlPar.LastOp = 'n';
	LogicRWCtlPar.ZoneNum = 0xff;
	LogicRWCtlPar.SuperBlkNum = 0xffff;
	LogicRWCtlPar.SuperPageNum = 0xffff;
	LogicRWCtlPar.LogPageLastPageNum = 0xffff;
	LogicRWCtlPar.LogPageLastBlk.PhyBlkNum = 0xfff;
	LogicRWCtlPar.LastWBufPageNum = 0xffffffff;
	LogicRWCtlPar.PhyDataBlk.PhyBlkNum = 0xfff;
	LogicRWCtlPar.PhyLogBlk.PhyBlkNum = 0xfff;
	LogicRWCtlPar.LogPageNum = 0xffff;
	LogicRWCtlPar.LogPageBitmap = 0x0000;
	LogicRWCtlPar.AccessTimer = 0;

	for(i=0; i<MAX_LOG_BLK_NUM; i++)
	{
		LogicRWCtlPar.LogBlkAccessAge[i] = 0;
	}

	/* init spare area data, global variable */
	//MEMSET(&WriteSpareData, 0xff, NAND_SPARE_SIZE * 4);
         MEMSET(&WriteSpareData, 0xff, sizeof(struct NandSpareData ));

	FTL_DBG("FTL_DBG: Init FTL Param complete.\n");
	return TRUE;
}

INT32U  FTL_Init(void)
{
	INT32S i;

	FTL_DBG("FTL_DBG: FTL_Init.\n");
	bPrintFlag =0x00;
	i =0x00;

	memset(&WearDbg,0x00,sizeof(struct  _Debug_Wear));


	/* init spare area data, global variable */
	//    MEMSET(&WriteSpareData, 0xff, NAND_SPARE_SIZE * 4);
	FTL_Param_Init();

	if(PAGE_BUFFER_SECT_NUM ==Page_Size2K)
	{
		NandDevInfo.PageCache = (void*)MALLOC(PAGE_BUFFER_SECT_NUM * NAND_SECTOR_SIZE*2);
	}
	else
	{
		NandDevInfo.PageCache = (void*)MALLOC(PAGE_BUFFER_SECT_NUM * NAND_SECTOR_SIZE);
	}
	NandDevInfo.PhyCache = (void*)MALLOC(SECTOR_NUM_PER_SINGLE_PAGE* NAND_SECTOR_SIZE+512);
	NandDevInfo.TestBuf= (void*)MALLOC(SECTOR_NUM_PER_SINGLE_PAGE* NAND_SECTOR_SIZE+512);


#ifdef __KERNEL__ 
	NandDevInfo.PageCache += 0x20000000;
	NandDevInfo.PhyCache += 0x20000000;
	NandDevInfo.TestBuf	 += 0x20000000;
#endif
	
	PMM_InitPageMapTblCache();
	BMM_InitBlkMapTblCache();
/*
.Init 
*/
#if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)  
	Init_ReClaimBlk_Info();
#endif

/*
.Init 
*/
#if (defined(__KERNEL__)   || BREC_DRIVER == 0x01)
	FTL_Correct_ModeInit();
#endif


	INIT_BOOT("##1.Reclaim: %d,%d,brecsize:%dKB\n",RECLAIM_VAL, SET_READ_Reclaim, NAND_BREC_SIZE/1024);
	INIT_BOOT("##2.Read Super:%d, Readclaim:%d\n",0x01,SUPPORT_READ_RECLAIM);
	INIT_BOOT("##3.Write Super:%d, Wearleveling:%d\n",SUPER_WEAR_LEVELLING,SUPPORT_WEAR_LEVELLING);	 
	INIT_BOOT("##4.ECC_reclaim,%d,overecc:%d\n",ECC_READ_RECLAIM,ECC_OVER_MODE);
	INIT_BOOT("##5.DataBlk:%d,MultiPlane:%d\n",DATA_BLK_NUM_PER_ZONE,NAND_SUPPORT_MULTI_PLANE);
	INIT_BOOT("##6.PageCache:0x%x,%x\n",(INT32U)NandDevInfo.PageCache,PAGE_BUFFER_SECT_NUM);	
	INIT_BOOT("##7.NandDevInfo.PhyCache:0x%x:\n",(INT32U)NandDevInfo.PhyCache); 


	FTL_DBG("FTL_DBG: Init FTL complete.\n");

	return TRUE;
}


/*
*********************************************************************************************************
*                       FLASH TRANSLATE LAYER EXIT
*
*Description: exit nand flash driver translate layer.
*
*Arguments  : none;
*
*Return     : TRUE;
*
*Note       : this function need flush the data in the ram to nand flash, and write all mapping
*             table to nand flash and free nand page buffer.
*********************************************************************************************************
*/
INT32U  FTL_Exit(void)
{
	FTL_DBG("FTL_DBG: Exit FTL.\n");

	/* flush cache data to nand flash */
	FTL_FlushPageCache();

	/* write back all mapping table to nand flash */
	BMM_WriteBackAllMapTbl();

	/* exit mapping table cache manage */
	PMM_ExitPageMapTblCache();
	BMM_ExitBlockMapTblCache();

	/* free buffer */
	FREE(NandDevInfo.PageCache);

	FTL_DBG("FTL_DBG: Exit FTL complete.\n");

    return TRUE;
}



