/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : logic_layer.c
* By      : xiongrui
* Version : V0.1
* Date    : 2007-7-23 9:06
*********************************************************************************************************
*/
#include "logic.h"

extern INT32U PHY_PageRead(struct PhysicOpParameter *NandOpPar);
extern INT32U PHY_PageWrite(struct PhysicOpParameter *NandOpPar);
extern INT32U PHY_EraseSuperBlk(struct PhysicOpParameter *PhyOpPar);
extern INT32U PHY_SyncNandOperation(INT32U BankNum);

extern INT32U FTL_CalPhyOpPar(struct PhysicOpParameter * PhyOpPar, INT32U ZoneNum, INT32U SupBlkNum, INT32U PageNum);
extern INT32U _GetFreeBlk(INT32U * FreeBlkPst, INT32U GetType);
extern INT32U FTL_MergeLogBlk(INT8U type);
extern INT32U FTL_BadBlkManage(struct DataBlkTblType * NewFreeBlk, struct DataBlkTblType * BadBlk, INT32U ErrPage);

extern struct  _Debug_Wear  WearDbg;


#if ZONE_TBL_MSG  ==0x01
static  INT32U bReadTbl=0x0;
static  INT32U bWriteTbl=0x0;
#endif
INT32U _DBG_PrintEraseInfo(struct DataBlkTblType  *pTmp,INT32U Cnt);

INT32U __PrintEraseInfo(struct DataBlkTblType  *pTmp,INT32U Cnt)
{
        INT32U iLoop,jLoop;        
        for(jLoop=iLoop =0x00 ;iLoop<Cnt;iLoop++)
        {
           if(pTmp[iLoop].PhyBlkNum !=0xFFF)
           {
                   INIT_BOOT("%4x,%x ",pTmp[iLoop].PhyBlkNum,  pTmp[iLoop].EraseCnt);
                  if(0x00==((jLoop++)+1)%8)
                  {
                            INIT_BOOT("\n");   
                  }
            }                    
        }
        INIT_BOOT("\n");  
        return 0x00;
}

/*!
*
* \par  Description:
*       This function get log number by logical block number.
*
* \param  [in]       LogicalBlkNum - logical number of data block number within zone
* \return      the poisition within log block space, 0xff means failed.
*
**/
INT8U _FindLogNumByLogicalBlkNum(INT32U LogicalBlkNum)
{
    INT32S loop;

    for (loop = 0; loop < MAX_LOG_BLK_NUM; loop++)
    {
        if(LTBL[loop].LogicBlkNum == LogicalBlkNum)
        {
            return loop;
        }
    }

    return 0xff;
}

/*!
*
* \par  Description:
*       This function rebuild page map table.
*
* \param  [in]       LoglBlkNum - log block serial number
* \return      sucess or failed.
* \note         only rebuiled table in  ram , not write into log block.
*
**/
INT32U _RebuildPageMapTbl(INT8U LogBlkNum)
{
    INT32U ret;
    INT16U status;
    INT16U lbn;
    INT16U bitmap;
    INT16U superpage;
    INT32U superblk;
    struct PhysicOpParameter NandOpPar;
    struct NandSpareData oob;
//@fish add  BCH8 0x07   BCH24 :0x1f
     bitmap =SINGLE_BITMAP_SPAREDATA;// 0x3;

    superblk = LTBL[LogBlkNum].PhyBlkNum.PhyBlkNum;
    NandOpPar.MainDataPtr = TMPBUF;
    NandOpPar.SpareDataPtr = &oob;
    NandOpPar.SectorBitmapInPage = bitmap;    /*just read fore 2 sectors*/

    MEMSET(L2P,0xff,PAGES_PER_SUPERBLK*2);
    for (superpage = 0 ; superpage <= LTBL[LogBlkNum].LastUsedPage; superpage++)
    {
        FTL_CalPhyOpPar(&NandOpPar, CURZONE, superblk, superpage);
        ret = PHY_PageRead(&NandOpPar);
        if (PHY_READ_ERR(ret))
        {
            return FALSE;
        }

        status = oob.UserData[0].SpareData1.LogStatus.PhyPageStatus;
        lbn = oob.UserData[0].SpareData1.LogicPageNum;

        if ((lbn != 0xffff) && (lbn < PAGE_CNT_PER_LOGIC_BLK)) /*legal page*/
        {
            L2P[lbn].PhyPageNum = superpage; /*l2p:logical to physical*/
        }

    }

    return TRUE;
}

/*!
*
* \par  Description:
*       This function read page map table to ram.
*
* \param  [in]       LoglBlkNum - log block serial number
* \return      sucess or failed.
*
**/
INT32U _ReadPageMapTbl(INT8U LogBlkNum)
{
    INT32U ret;
    INT16U status;
    INT16U logicpagenum;
    INT16U bitmap;
    INT16U superpage;
    INT16U checksum;
    INT32U superblk;
    struct PhysicOpParameter NandOpPar;
    struct NandSpareData oob;

    bitmap = 0x0f;  ////??????????
    //@fish 2010 -10 -18  add BCH8 0x07  BCH24 0x1f
    if(bitmap<SINGLE_BITMAP_SPAREDATA)
    {
    	 bitmap = SINGLE_BITMAP_SPAREDATA;	
	
    }
    superblk = LTBL[LogBlkNum].PhyBlkNum.PhyBlkNum;
    superpage = LTBL[LogBlkNum].LastUsedPage;


    if (superpage == 0xffff) /*log block is empty , new log block*/
    {
        MEMSET((INT8S *)L2P,0xff,PAGES_PER_SUPERBLK*sizeof(struct PageMapTblType));
        return TRUE;
    }

    NandOpPar.MainDataPtr = TMPBUF;
    NandOpPar.SpareDataPtr = &oob;
    NandOpPar.SectorBitmapInPage = bitmap;    /*read fore 4 sectors , 2k*/

    FTL_CalPhyOpPar(&NandOpPar, CURZONE, superblk, superpage);
    ret = PHY_PageRead(&NandOpPar);
    if (PHY_READ_ERR(ret))
    {
        return FALSE;
    }

    checksum = ((INT16U *)(NandOpPar.MainDataPtr))[1023]; /*2 bytes checksum locate at the end of 4th sector*/

    status = oob.UserData[0].SpareData1.LogStatus.PhyPageStatus;
    logicpagenum = oob.UserData[0].SpareData1.LogicPageNum;
 

  //  if ( (logicpagenum == 0xffff)&&(0xaa == status) && (checksum == _GetCheckSum((INT16U*)(NandOpPar.MainDataPtr),PAGES_PER_SUPERBLK)))
    if ( (logicpagenum == 0xffff)&&(PAGE_STATE_PMTBL == status) && (checksum == _GetCheckSum((INT16U*)(NandOpPar.MainDataPtr),PAGES_PER_SUPERBLK)))
    {
        MEMCPY((INT8S *)L2P,NandOpPar.MainDataPtr,PAGES_PER_SUPERBLK*sizeof(struct PageMapTblType)); /*valid page map table*/
    }
    else
    {
        if (TRUE != _RebuildPageMapTbl(LogBlkNum)) /*no  table page or checksum error , rebuild*/
        {
            FTL_ERR("rebuild logic page map table error,zone %d blk %d phyblk %d lastusedpage %d \n",\
                CURZONE,LTBL[LogBlkNum].LogicBlkNum,superblk,superpage);
            return FALSE;
        }
    }

    return TRUE;

}

/*!
*
* \par  Description:
*       This function write page map table to log block.
*
* \param  [in]       LoglBlkNumPst - log block serial number
* \return      sucess or failed.
*
**/
INT32U _WritePageMapTbl(INT8U LogBlkNumPst)
{
    INT16U loop;
    INT16U superpage;
    INT16U checksum;
    INT32U superblk;
    struct PhysicOpParameter NandOpPar;
    struct NandSpareData oob;

    superblk = LTBL[LogBlkNumPst].PhyBlkNum.PhyBlkNum;
    superpage = LTBL[LogBlkNumPst].LastUsedPage + 1; /*table page number*/
    MEMSET(&oob,0xff,sizeof(struct NandSpareData));
    MEMSET(TMPBUF,0xff,PAGE_BUFFER_SECT_NUM *512);

    if (superpage == PAGES_PER_SUPERBLK) /*log block is full*/
    {
        FTL_MergeLogBlk(SPECIAL_MERGE);

        if ( (PMT->ZoneNum != 0xff) && (PMT->LogicBlkNum != 0xffff) )/*move merge*/
        {
            superblk = LTBL[LogBlkNumPst].PhyBlkNum.PhyBlkNum;
            superpage = LTBL[LogBlkNumPst].LastUsedPage + 1; /*table page number*/
        }
        else
        {
            return TRUE;
        }
    }

    NandOpPar.MainDataPtr = TMPBUF;
    NandOpPar.SpareDataPtr = &oob;

    oob.UserData[0].SpareData1.LogStatus.PhyPageStatus = PAGE_STATE_PMTBL;    /*table page flag*/

    MEMCPY(TMPBUF,(INT8S *)L2P,PAGES_PER_SUPERBLK*sizeof(struct PageMapTblType));
    checksum = _GetCheckSum((INT16U*)L2P, PAGES_PER_SUPERBLK);
    ((INT16U*)(TMPBUF))[1023] = checksum;

redo:
    FTL_CalPhyOpPar(&NandOpPar, CURZONE, superblk, superpage);/*set physical addr*/
    for (loop = 0; loop < OPCYCLE; loop++)
    {
        NandOpPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
        PHY_PageWrite(&NandOpPar);
        if (TRUE != PHY_SyncNandOperation(NandOpPar.BankNum))
        {

            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType *BadBlk = &(LTBL[LogBlkNumPst].PhyBlkNum);

            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,superpage))
            {
                FTL_ERR("_WritePageMapTbl:bad block manage failed\n");
                return FALSE;
            }

            LTBL[LogBlkNumPst].PhyBlkNum = NewFreeBlk;
            superblk = NewFreeBlk.PhyBlkNum;
            goto redo;
        }
    }

    LTBL[LogBlkNumPst].LastUsedPage = superpage;

    /* clear page mapping table cache infor */
    PMT->ZoneNum = 0xff;
    PMT->LogicBlkNum = 0xffff;

    return TRUE;
}

/*!
*
* \par  Description:
*       This function try to find cache which cached current logicak block.
*
* \param  [in]       LogicBlkNum - logical block number within zone.
* \return      sucess or failed.
*
**/
INT32U _FindPageMapTblFromCache(INT32U LogicBlockNum)
{
    INT32S loop;
    for (loop = 0; loop < LOG_CACHE_NUM ; loop++)
    {
        if ( (NandDevInfo.PageTblCacheInfo->PageMapTblCache[loop].ZoneNum == CURZONE) \
            && (NandDevInfo.PageTblCacheInfo->PageMapTblCache[loop].LogicBlkNum == _FindLogNumByLogicalBlkNum(LogicBlockNum)))
        {
            PMT = &(NandDevInfo.PageTblCacheInfo->PageMapTblCache[loop]);
            return TRUE;
        }
    }

    return FALSE;
}

/*!
*
* \par  Description:
*       This function try to find cache which will be cached current logicak block.
*
* \param  [in]       void
* \return      the replace poisition in cache array
* \note    adopt LRU methods,choose the replace acche.
**/
INT32U _FindPageMapTblReplacePoint(void)
{
    INT32S i,j;
    INT32S accesscnt = NandDevInfo.PageTblCacheInfo->PageMapTblCache[0].AccessCount;
    INT32S poisition = 0;

    for ( i = 0 ; i < LOG_CACHE_NUM ;i++)
    {
        if(NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].ZoneNum == 0xff) /*find empty cache firstly*/
            break;
    }

    if ( i == LOG_CACHE_NUM) /*no empty cache*/
    {
        for (j = 1 ; j < LOG_CACHE_NUM; j++)/*find a acche which was accessed least recently*/
        {
            if (NandDevInfo.PageTblCacheInfo->PageMapTblCache[j].AccessCount > accesscnt)
            {
                poisition = j;
                accesscnt = NandDevInfo.PageTblCacheInfo->PageMapTblCache[j].AccessCount;
            }
        }
    }
    else
        poisition = i;

    for ( i = 0 ; i < LOG_CACHE_NUM ;i++)/*set access count 0*/
    {
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].AccessCount = 0;
    }

    return poisition;
}

/*!
*
* \par  Description:
*       This function fiil cache with page map table.
*
* \param  [in]       LogicalBlkNum - logical number of data block number within zone
* \return      sucess or failed
**/
INT32U _UpdatePageMapTblCache(INT32U LogicBlockNum)
{
    INT32S point,loop;

    struct ZoneTblCacheType *tmptbl = BMT;
///INIT_BOOT("###%s, logicblocknum:%d ###\n",__func__,LogicBlockNum);

    point = _FindPageMapTblReplacePoint();/*find replace point*/
    PMT = &(NandDevInfo.PageTblCacheInfo->PageMapTblCache[point]);
    if ( (PMT->TblNeedUpdate) && (PMT->ZoneNum != 0xff) )
    {
        if (PMT->ZoneNum != tmptbl->ZoneNum)/*page map cache locate other zone,switch zone table needed*/
        {
            for (loop = 0; loop < ZONE_CACHE_NUM ; loop++)
            {
                if ( (NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].ZoneNum == PMT->ZoneNum) )
                {
                    BMT = &(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop]);
                    break;
                }
            }
            if (loop == ZONE_CACHE_NUM)
            {
                FTL_ERR("_UpdatePageMapTblCache:no zone table found\n");
                return FALSE;
            }
        }

        if (TRUE != _WritePageMapTbl(PMT->LogicBlkNum))
        {
            FTL_ERR("_UpdatePageMapTblCache:write back table err\n");
            return FALSE;
        }

        BMT = tmptbl;

    }

    point = _FindLogNumByLogicalBlkNum(LogicBlockNum);
    if (TRUE != _ReadPageMapTbl(point))/*read current log block page map table to ram */
    {
        return FALSE;
    }

    PMT->ZoneNum = CURZONE;
    PMT->LogicBlkNum = point;
    PMT->TblNeedUpdate = 0;

    return TRUE;
}

/*!
*
* \par  Description:
*       This function prepare current log block page map table.
*
* \param  [in]    LogicalBlkNum - logical number of data block number within zone
* \return      sucess or failed
* \note    this function ensure current log block page map table exists in cache.
**/
INT32U  PMM_SwitchPageMapTbl(INT32U LogicBlockNum)
{

    if (TRUE != _FindPageMapTblFromCache(LogicBlockNum)) /*serach cache array*/
    {
        return (_UpdatePageMapTblCache(LogicBlockNum)); /*if cache miss ,fill cache*/
    }

    return TRUE;
}

/*!
*
* \par  Description:
*       This function set page cahche access count.
*
* \param  [in]    void
* \return      sucess or failed
**/
void PMM_CalAccessCount(void)
{
    INT32S loop;
    for ( loop = 0 ; loop < LOG_CACHE_NUM ;loop++)
    {
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[loop].AccessCount ++;
    }
    PMT->AccessCount = 0;
}

/*!
*
* \par  Description:
*       This function init page map table cache memory allocation.
*
* \param  [in]    void
* \return      sucess or failed
**/
INT32U PMM_InitPageMapTblCache(void)
{
    INT32S i;

    for (i = 0; i < LOG_CACHE_NUM; i++)
    {
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].TblNeedUpdate = 0;
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].ZoneNum = 0xff;
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].LogicBlkNum = 0xffff;
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].AccessCount = 0;
        NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].PageMapTbl = (void*)MALLOC(sizeof(struct PageMapTblType)*PAGES_PER_SUPERBLK);
        if (!NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].PageMapTbl)
        {
            return FALSE;
        }
    }
#ifdef MALLC_ADDR_PRINT	
    INIT_DRV("\n***Enter %s*****\n",__FUNCTION__);
    INIT_DRV("****LOG_CACHE_NUM:0x%x\n",LOG_CACHE_NUM);
    INIT_DRV("****PAGES_PER_SUPERBLK:0x%x\n",PAGES_PER_SUPERBLK);
    for (i = 0; i < LOG_CACHE_NUM; i++)
    {
		INIT_DRV("***PageMapTbl[%d]:0x%x\n",i,NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].PageMapTbl);		
	}
#endif
    return TRUE;
}

/*!
*
* \par  Description:
*       This function free page map table cache memory .
*
* \param  [in]    void
* \return      sucess or failed
**/
INT32U PMM_ExitPageMapTblCache(void)
{
    INT32S i;

    for (i = 0; i < LOG_CACHE_NUM; i++)
    {

        FREE(NandDevInfo.PageTblCacheInfo->PageMapTblCache[i].PageMapTbl);
    }

    return TRUE;
}
/***********************************************************************************/
/*!
*
* \par  Description:
*       This function read blk map table to ram.
*
* \param  [in]      ZoneNum - global zone number
* \return      success or failed.
*
**/
INT32U _ReadBlkMapTbl(INT8U ZoneNum)
{
    INT32U ret;
    INT16U bitmap;
    INT16U superpage;
    INT16U checksum;
    INT32U superblk;
    struct PhysicOpParameter NandOpPar;
#if _NEW_ZONE_TBL_==0x01
    INT32U checksum32;
#endif


    bitmap = 0x0f; /*size of block map table less than 2k,4sectors*/

#if _NEW_ZONE_TBL_==0x01   
     if(SECTOR_NUM_PER_SINGLE_PAGE ==Page_Size2K) //Page 2KB 
     {
            bitmap  =0xF; //Read 2KB  page 
     }
     else
     {
             bitmap =0xFF; //Read 4 KB Page 
     }
 #endif
 ///// @fish add  2010-10-18 ????????????
     if(bitmap<SINGLE_BITMAP_SPAREDATA)
     {
            bitmap = SINGLE_BITMAP_SPAREDATA;
     }
    superblk = ZoneInfo[ZoneNum].PhyBlkNum; /*zone table block physical addr*/
    superpage = ZoneInfo[ZoneNum].TablePosition;    /*zone table page number within superblock*/

#if ZONE_TBL_MSG  ==0x01
      INIT_BOOT("\n===NO:%5x,Read Zone :%2d, Blk:%x,Pge:%d===\n",bReadTbl++,ZoneNum,superblk,superpage);
#endif

    NandOpPar.MainDataPtr = TMPBUF;
    NandOpPar.SpareDataPtr = NULL;
    NandOpPar.SectorBitmapInPage = bitmap;

    /*read data block and free block map table*/
    FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage);
    ret = PHY_PageRead(&NandOpPar);
    if (PHY_READ_ERR(ret))
    {
        FTL_ERR("phyread blk map table failed\n");
        return FALSE;
    }
    
#if _NEW_ZONE_TBL_==0x01  
    if(SECTOR_NUM_PER_SINGLE_PAGE ==Page_Size2K) //Page 2KB  Second 2KB Page 
    {
            NandOpPar.MainDataPtr = TMPBUF+2048;
            NandOpPar.SpareDataPtr = NULL;
            NandOpPar.SectorBitmapInPage = bitmap;
            
            FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage+1);
            ret = PHY_PageRead(&NandOpPar);
            if (PHY_READ_ERR(ret))
            {
                FTL_ERR("phyread blk map table failed\n");
                return FALSE;
            } 
            //Default MainData Pointer
            NandOpPar.MainDataPtr = TMPBUF;
    }
#endif

    
#if _NEW_ZONE_TBL_==0x01
        checksum32 = ((INT32U *)(NandOpPar.MainDataPtr))[1023];
        if ( checksum32 == _GetCheckSum32((INT32U*)(NandOpPar.MainDataPtr),DATABLKS_PER_ZONE+FREEBLKS_PER_ZONE))
        {
                MEMCPY(DTBL,NandOpPar.MainDataPtr,DATABLKS_PER_ZONE*sizeof(struct DataBlkTblType));
                MEMCPY(FTBL,(INT32U *)(NandOpPar.MainDataPtr) + DATABLKS_PER_ZONE,FREEBLKS_PER_ZONE*sizeof(struct DataBlkTblType));
        }
#else
        checksum = ((INT16U *)(NandOpPar.MainDataPtr))[1023];
        if ( checksum == _GetCheckSum((INT16U*)(NandOpPar.MainDataPtr),DATABLKS_PER_ZONE+FREEBLKS_PER_ZONE))    
        {
                MEMCPY((INT8S*)DTBL,NandOpPar.MainDataPtr,DATABLKS_PER_ZONE*sizeof(struct DataBlkTblType));
                MEMCPY((INT8S*)FTBL,(INT8S*)((INT16U *)(NandOpPar.MainDataPtr) + DATABLKS_PER_ZONE),FREEBLKS_PER_ZONE*sizeof(struct DataBlkTblType));
        }
#endif     
        else
        {
                FTL_ERR("_ReadBlkMapTbl:zone table:check sum failed\n");
                return FALSE;
        }
 //       printf("checksum:%x\n",checksum);
   //     Dump_mem((INT16U*)(NandOpPar.MainDataPtr),2048,0,2);
       /*end*/
    /*read log block info table*/
    //superpage++;
        superpage+=  LOGTBL_OFFSET ; //Define 0x02    
    FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage);

    ret = PHY_PageRead(&NandOpPar);
    if (PHY_READ_ERR(ret))
    {
        FTL_ERR("phyread blk map table failed\n");
        return FALSE;
    }
	// Dump_mem((INT16U*)(NandOpPar.MainDataPtr),2048,0,2);
    checksum = ((INT16U *)(NandOpPar.MainDataPtr))[1023];
	
    if ( checksum == _GetCheckSum((INT16U*)(NandOpPar.MainDataPtr),LOGBLKS_PER_ZONE * (sizeof(struct LogBlkTblType)/sizeof(INT16U))))
    {
        MEMCPY((INT8U*)LTBL,NandOpPar.MainDataPtr,LOGBLKS_PER_ZONE*sizeof(struct LogBlkTblType));
    }
    else
    {
        FTL_ERR("_ReadBlkMapTbl:log table :check sum failed\n");
        return FALSE;
    }
 #if 0
	int i;
	for(i=0; i<MAX_LOG_BLK_NUM; i++)
	{
		INIT_BOOT("Log:%3x,%3x,%2x\n",CUR_LOG_BLK_TABLE[i].LogicBlkNum,CUR_LOG_BLK_TABLE[i].PhyBlkNum.PhyBlkNum,
			CUR_LOG_BLK_TABLE[i].LastUsedPage);	

	}
 #endif  
 //   Dump_mem((INT16U*)(NandOpPar.MainDataPtr),64,0,2);
    /*end*/

    return TRUE;

}

/*!
*
* \par  Description:
*       This function write all dirty page map table within current zone to flash.
*
* \param  [in]      ZoneNum - global zone number
* \return      success or failed.
* \note        this function must be called when zone changes.
**/
INT32U _WriteAllPageMapTblWithinZone(INT8U ZoneNum)
{
    INT32S nPMM;

    for (nPMM = 0; nPMM < LOG_CACHE_NUM; nPMM++)
    {
            if ((NandDevInfo.PageTblCacheInfo->PageMapTblCache[nPMM].ZoneNum == ZoneNum)\
                && (NandDevInfo.PageTblCacheInfo->PageMapTblCache[nPMM].TblNeedUpdate))
            {
                PMT = &(NandDevInfo.PageTblCacheInfo->PageMapTblCache[nPMM]);
                if (TRUE != _WritePageMapTbl(PMT->LogicBlkNum))
                {
                    return FALSE;
                }
                PMT->TblNeedUpdate = 0;
            }
    }

    return TRUE;
}

/*!
*
* \par  Description:
*       This function write blk map table to flash.
*
* \param  [in]      ZoneNum - global zone number
* \return      success or failed.
*
**/
INT32U _WriteBlkMapTbl(INT8U ZoneNum)
{
    INT8U loop;
    INT16S superpage;
    INT16U checksum;
    INT32U superblk;
    struct PhysicOpParameter NandOpPar;
    struct NandSpareData oob;
#if _NEW_ZONE_TBL_==0x01
    INT32U checksum32;
 #endif      
int i;

    if (TRUE != _WriteAllPageMapTblWithinZone(ZoneNum))/*write back all dirty page map table in this zone*/
    {
        FTL_ERR("write all page map table err\n");
        return FALSE;
    }

    MEMSET(&oob,0xff,sizeof(struct NandSpareData));
#if _NEW_ZONE_TBL_==0x01
    MEMSET(TMPBUF,0xff,ZONE_TBL_SIZE_4K);
#else
    MEMSET(TMPBUF,0xff,ZONE_TBL_SIZE_2K);
#endif 
    NandOpPar.MainDataPtr = TMPBUF;
    NandOpPar.SpareDataPtr = &oob;
    

    superblk = ZoneInfo[ZoneNum].PhyBlkNum;
    superpage = ZoneInfo[ZoneNum].TablePosition;

#if ZONE_TBL_MSG  ==0x01
    INIT_BOOT("\n****No:0x%5x,Write_Zone %2d blk:%d,Pge:%d*****\n",bWriteTbl++,ZoneNum,superblk,superpage);
#endif

	if (superpage >= PAGES_PER_SUPERBLK - 4) /*table block is full,erase it */
	{
		FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, 0);
		if (TRUE != PHY_EraseSuperBlk(&NandOpPar) )    
		{
			struct DataBlkTblType NewFreeBlk;
			struct DataBlkTblType BadBlk;
			BadBlk.PhyBlkNum = superblk;
			FTL_ERR("_WriteBlkMapTbl:bad block %d made\n",BadBlk.PhyBlkNum);    
			                  
			if (TRUE != FTL_BadBlkManage(&NewFreeBlk,&BadBlk,0))
			{
				FTL_ERR("_WriteBlkMapTbl:2:bad block manage failed\n");
				return FALSE;
			}

			superblk = NewFreeBlk.PhyBlkNum;
			ZoneInfo[ZoneNum].PhyBlkNum = superblk;
			

		}
		superpage = -4;
	}
redo:
    /*write data block and free block table*/
    superpage += 4;
    ZoneInfo[ZoneNum].TablePosition = superpage;

    FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage);
    MEMCPY(TMPBUF,(INT8S*)DTBL,DATABLKS_PER_ZONE*sizeof(struct DataBlkTblType));
    MEMCPY( (INT8S *)(TMPBUF) + DATABLKS_PER_ZONE*sizeof(struct DataBlkTblType),\
        (INT8S*)FTBL,FREEBLKS_PER_ZONE*sizeof(struct DataBlkTblType));
#if (_NEW_ZONE_TBL_==0x01)        
    checksum32 = _GetCheckSum32((INT32U*)(NandOpPar.MainDataPtr),DATABLKS_PER_ZONE+FREEBLKS_PER_ZONE);
    ((INT32U*)(TMPBUF))[1023] = checksum32;
#else
    checksum = _GetCheckSum((INT16U*)(NandOpPar.MainDataPtr),DATABLKS_PER_ZONE+FREEBLKS_PER_ZONE);
    ((INT16U*)(TMPBUF))[1023] = checksum;    
#endif

    if (superpage == 0) /*write table block flag in page 0*/
    {
        INT32S zone_in_die = ZoneNum % ZONE_NUM_PER_DIE;
        oob.UserData[0].SpareData0.LogicBlkInfo = (1<<15) | (zone_in_die << 10) | 0xaa ;
    }
    oob.UserData[0].SpareData1.LogStatus.PhyPageStatus = 0x55;
#if 0//  __KERNEL__
INIT_BOOT("Zone table\n");
Dump_mem((INT16U*)TMPBUF,2048,0,2);	
INIT_BOOT("Log table\n");
MEMSET(TMPBUF,0xff,2*1024);
MEMCPY(TMPBUF,(INT8S*)LTBL,LOGBLKS_PER_ZONE*sizeof(struct LogBlkTblType));

///int i;
for(i=0; i<MAX_LOG_BLK_NUM; i++)
{
	INIT_BOOT("Log:%3x,%3x,%2x\n",CUR_LOG_BLK_TABLE[i].LogicBlkNum,CUR_LOG_BLK_TABLE[i].PhyBlkNum.PhyBlkNum,
		CUR_LOG_BLK_TABLE[i].LastUsedPage);	

}
Dump_mem((INT16U*)(TMPBUF),64,0,2);	
return TRUE;
#endif


#if _NEW_ZONE_TBL_==0x01     
   if(SECTOR_NUM_PER_SINGLE_PAGE ==Page_Size2K) //Page 2KB 
   {
            for (loop = 0; loop < OPCYCLE; loop++)
            {
                NandOpPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);                 
                NandOpPar.MainDataPtr = TMPBUF;
                NandOpPar.SpareDataPtr = &oob;
                FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage+0x00);
                PHY_PageWrite(&NandOpPar);
                
         
                if (TRUE != PHY_SyncNandOperation(NandOpPar.BankNum))
                {
                    struct DataBlkTblType NewFreeBlk;
                    struct DataBlkTblType BadBlk;
                    BadBlk.PhyBlkNum = superblk;

                    if (TRUE != FTL_BadBlkManage(&NewFreeBlk,&BadBlk,0))
                    {
                        FTL_ERR("_WriteBlkMapTbl:2:bad block manage failed\n");
                        return FALSE;
                    }

                    superblk = NewFreeBlk.PhyBlkNum;
                    ZoneInfo[ZoneNum].PhyBlkNum = superblk;
                    superpage = -4;
                    goto redo;

                }
                
                NandOpPar.MainDataPtr = TMPBUF+2048;
                NandOpPar.SpareDataPtr = &oob;
                FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage+0x01);
                PHY_PageWrite(&NandOpPar);
				
                if (TRUE != PHY_SyncNandOperation(NandOpPar.BankNum))
                {
                    struct DataBlkTblType NewFreeBlk;
                    struct DataBlkTblType BadBlk;
                    BadBlk.PhyBlkNum = superblk;

                    if (TRUE != FTL_BadBlkManage(&NewFreeBlk,&BadBlk,0))
                    {
                        FTL_ERR("_WriteBlkMapTbl:2:bad block manage failed\n");
                        return FALSE;
                    }

                    superblk = NewFreeBlk.PhyBlkNum;
                    ZoneInfo[ZoneNum].PhyBlkNum = superblk;
                    superpage = -4;
                    goto redo;

                }
            }
            
   }
   else
#endif   
    {
    
    for (loop = 0; loop < OPCYCLE; loop++)
    {
        NandOpPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
        PHY_PageWrite(&NandOpPar);
        if (TRUE != PHY_SyncNandOperation(NandOpPar.BankNum))
        {
            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType BadBlk;
            BadBlk.PhyBlkNum = superblk;

            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,&BadBlk,0))
            {
                FTL_ERR("_WriteBlkMapTbl:2:bad block manage failed\n");
                return FALSE;
            }

            superblk = NewFreeBlk.PhyBlkNum;
            ZoneInfo[ZoneNum].PhyBlkNum = superblk;
            superpage = -4;
            goto redo;

        }
    }

	
    }
    /*end*/


#if NAND_SHARE_PAGES == 0x01  // added by jjf for shared page
    FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage+ZONETBL_OFFSET_BACK_UP);
    for (loop = 0; loop < OPCYCLE; loop++)
    {
        NandOpPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
        PHY_PageWrite(&NandOpPar);
        if (TRUE != PHY_SyncNandOperation(NandOpPar.BankNum))
        {
            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType BadBlk;
            BadBlk.PhyBlkNum = superblk;

            // if NewFreeBlk.PhyBlkNum > MAX_ZONETBL_AREA_BLOCK will rebuild zonetbl when next startup
            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,&BadBlk,0))
            {
                FTL_ERR("_WriteBlkMapTbl:2:bad block manage failed\n");
                return FALSE;
            }

            superblk = NewFreeBlk.PhyBlkNum;
            ZoneInfo[ZoneNum].PhyBlkNum = superblk;
            superpage = -4;
            goto redo;
        }
    }
#endif




    /*write log block table*/
    superpage += LOGTBL_OFFSET;
    FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage);
    MEMSET(&oob,0xff,sizeof(struct NandSpareData));
    MEMSET(TMPBUF,0xff,2*1024);
    MEMCPY(TMPBUF,(INT8S*)LTBL,LOGBLKS_PER_ZONE*sizeof(struct LogBlkTblType));
    checksum = _GetCheckSum((INT16U*)LTBL,LOGBLKS_PER_ZONE * (sizeof(struct LogBlkTblType)/sizeof(INT16U)));
    ((INT16U*)(TMPBUF))[1023] = checksum;

    oob.UserData[0].SpareData1.LogStatus.PhyPageStatus = 0x55;

    for (loop = 0; loop < OPCYCLE; loop++)
    {
        NandOpPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
        PHY_PageWrite(&NandOpPar);
        if (TRUE != PHY_SyncNandOperation(NandOpPar.BankNum))
        {
            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType BadBlk;
            BadBlk.PhyBlkNum = superblk;

            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,&BadBlk,0))
            {
                FTL_ERR("_WriteBlkMapTbl:2:bad block manage failed\n");
                return FALSE;
            }

            superblk = NewFreeBlk.PhyBlkNum;
            ZoneInfo[ZoneNum].PhyBlkNum = superblk;
            superpage = -4;
            goto redo;
        }
    }
#if 0	
	superblk = ZoneInfo[ZoneNum].PhyBlkNum;
	superpage = ZoneInfo[ZoneNum].TablePosition;
	FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage);
	PHY_PageRead(&NandOpPar);
#endif	
	
#if 0
//    Dump_mem(TMPBUF,2048,0,2);
//	int i;
	for(i=0; i<MAX_LOG_BLK_NUM; i++)
	{
		INIT_BOOT("Log:%3x,%3x,%2x\n",CUR_LOG_BLK_TABLE[i].LogicBlkNum,CUR_LOG_BLK_TABLE[i].PhyBlkNum.PhyBlkNum,
			CUR_LOG_BLK_TABLE[i].LastUsedPage);	

	}
	Dump_mem((INT16U*)(TMPBUF),64,0,2);	
   #endif 
    /*end*/

    return TRUE;

}

/*!
*
* \par  Description:
*       This function write dirty flag to flash.
*
* \param  [in]      ZoneNum - global zone number
* \return      success or failed.
* \note         if dirty flag found,the table in flash is unbelievable
**/
INT32U _WriteDirtyFlag(INT8U ZoneNum)
{
    INT32S loop;
    INT16S superpage;
    INT32U superblk;
    struct PhysicOpParameter NandOpPar;
    struct NandSpareData oob;

    superblk = ZoneInfo[ZoneNum].PhyBlkNum;
    superpage = ZoneInfo[ZoneNum].TablePosition + DIRTYFlag_OFFSET;
    MEMSET(&oob,0xff,sizeof(struct NandSpareData));

    oob.UserData[0].SpareData1.LogStatus.PhyPageStatus = 0x55;

    MEMSET((INT8S*)(TMPBUF), 0x55, 512);
    NandOpPar.MainDataPtr = TMPBUF;
    NandOpPar.SpareDataPtr = &oob;

    FTL_CalPhyOpPar(&NandOpPar, ZoneNum, superblk, superpage);
    for (loop = 0; loop < OPCYCLE; loop++)
    {
        NandOpPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
        PHY_PageWrite(&NandOpPar);
        PHY_SyncNandOperation(NandOpPar.BankNum);
    }

    return TRUE;

}

INT32U BMM_TrySetDirtyFlag(void)
{
    if (0 == BMT->TblNeedUpdate)
    {
        _WriteDirtyFlag(CURZONE);
        BMT->TblNeedUpdate = 1;
    }

    return TRUE;
}

/*!
*
* \par  Description:
*       This function try to find cache which cached current zone.
*
* \param  [in]       ZoneNum - global zone number
* \return      sucess or failed.
*
**/
INT32U _FindBlkMapTblFromCache(INT8U ZoneNum)
{
    INT32S loop;

    for (loop = 0; loop < ZONE_CACHE_NUM ; loop++)
    {
        if ( (NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].ZoneNum == ZoneNum) )
        {
            BMT = &(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop]);
            return TRUE;
        }
    }

    return FALSE;
}

/*!
*
* \par  Description:
*       This function try to find cache which will be cached current zone.
*
* \param  [in]       void
* \return      the replace poisition in cache array
* \note    adopt LRU methods,choose the replace acche.
**/
INT32U _FindBlkMapTblReplacePoint(void)
{
    INT32S i,j;

    INT32S poisition = 0;

    for ( i = 0 ; i < ZONE_CACHE_NUM ;i++)
    {
        if(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[i].ZoneNum == 0xff)/*get empty cache firstly*/
            break;
    }

    if ( i == ZONE_CACHE_NUM)/*get a cache which was accessed least recently*/
    {
        INT32S accesscnt = NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[0].AccessCount;
        for (j = 1 ; j < ZONE_CACHE_NUM; j++)
        {
            if (NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[j].AccessCount > accesscnt)
            {
                poisition = j;
                accesscnt = NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[j].AccessCount;
            }
        }
    }
    else
        poisition = i;

    for ( i = 0 ; i < ZONE_CACHE_NUM ;i++)/*clear all account*/
    {
        NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[i].AccessCount = 0;
    }

    return poisition;
}

/*!
*
* \par  Description:
*       This function fill cache with block map table.
*
* \param  [in]     ZoneNum - global zone number
* \return      sucess or failed
**/
INT32U _UpdateBlkMapTblCache(INT8U ZoneNum)
{
    INT32S point;
    point = _FindBlkMapTblReplacePoint(); /*get cache poisition*/
    BMT = &(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[point]);
    if (BMT->TblNeedUpdate)
    {
        if (TRUE != _WriteBlkMapTbl(CURZONE)) /*write original dirty table back*/
        {
            return FALSE;
        }
    }
    if (TRUE != _ReadBlkMapTbl(ZoneNum)) /*read curernt zone table*/
    {
        return FALSE;
    }
    BMT->ZoneNum = ZoneNum;
    BMT->TblNeedUpdate = 0;

    return TRUE;
}

/*!
*
* \par  Description:
*       This function prepare current zone table.
*
* \param  [in]    ZoneNum - global zone number
* \return      sucess or failed
* \note    this function ensure block map table  of current zone exists in cache.
**/
INT32U  BMM_SwitchBlkMapTbl(INT8U ZoneNum)
{

    if (TRUE != _FindBlkMapTblFromCache(ZoneNum))/*try to hit cache*/
    {
        return (_UpdateBlkMapTblCache(ZoneNum));    /*cache miss, fill cache*/
    }

    return TRUE;
}

/*!
*
* \par  Description:
*       This function set zone cache access count.
*
* \param  [in]    void
* \return      sucess or failed
**/
void BMM_CalAccessCount(void)
{
    INT32S loop;

    for (loop = 0; loop < ZONE_CACHE_NUM; loop++)
    {
        NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].AccessCount ++;
    }
    BMT->AccessCount = 0;

}
#if DEBUG_ERASE_CNT  ==0x01
INT32U _DBG_BubleSort (struct DataBlkTblType  *pTmp,INT32U Cnt)
{
     INT32U iLoop,jLoop,iTmp;
     struct DataBlkTblType  Tmp2;
     for(iLoop =0x00;iLoop<Cnt;iLoop++)
     {
             for(jLoop=Cnt-1;jLoop>iLoop;jLoop--)
             {
                       if( pTmp[jLoop].EraseCnt<pTmp[jLoop -1 ].EraseCnt)
                       {
                                Tmp2 = pTmp[jLoop -1 ];
                                 pTmp[jLoop -1 ] =  pTmp[jLoop  ];
                                 pTmp[jLoop  ] = Tmp2;
                       }
             }
     }     
}
INT32U _Debug_PrintEraseInfo(struct DataBlkTblType  *pTmp,INT32U Cnt)
{
        INT32U iLoop,jLoop;        
        for(jLoop=iLoop =0x00 ;iLoop<Cnt;iLoop++)
        {
           if(pTmp[iLoop].PhyBlkNum !=0xFFF)
           {
                   INIT_BOOT("%4x,%4x ",pTmp[iLoop].PhyBlkNum,  pTmp[iLoop].EraseCnt);
                  if(0x00==((jLoop++)+1)%8)
                  {
                            INIT_BOOT("\n");   
                  }
            }                    
        }
        INIT_BOOT("\n");  
        return 0x00;
}
INT32U  Debug_ReadZoneTbl(void)
{
       INT32U iLoop,bMax_Zone;
       bMax_Zone = ZONE_NUM_PER_DIE*DIE_CNT_PER_CHIP ;
       for(iLoop =0x00;iLoop<bMax_Zone;iLoop++)
       {
               _UpdateBlkMapTblCache(iLoop);
               
              INIT_BOOT("====%s,%d  Zone:%d,Max:%d,EraseCnt:%x ===\n",__func__,__LINE__,iLoop,bMax_Zone,WearDbg.wEraseBlkCnt);             
              MEMCPY(TMPBUF,DTBL,DATABLKS_PER_ZONE*sizeof(struct DataBlkTblType));
              MEMCPY((INT32U*)TMPBUF+DATABLKS_PER_ZONE,FTBL,
                                FREEBLKS_PER_ZONE*sizeof(struct DataBlkTblType));        
              _Debug_PrintEraseInfo((struct DataBlkTblType *)TMPBUF,1023);
       }
}
#endif

/*!
*
* \par  Description:
*       This function init block map table cache memory allocation.
*
* \param  [in]    void
* \return      sucess or failed
**/
INT32U BMM_InitBlkMapTblCache(void)
{
    INT32S loop;

    for (loop = 0; loop < ZONE_CACHE_NUM; loop++)
    {
                NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].TblNeedUpdate = 0;
                NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].ZoneNum = 0xff;
                NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].AccessCount = 0;
                NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].DataBlkTbl = (void*)MALLOC(DATABLKS_PER_ZONE * sizeof(struct DataBlkTblType));		
                NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].LogBlkTbl = (void*)MALLOC(LOGBLKS_PER_ZONE * sizeof(struct LogBlkTblType));       
                NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].FreeBlkTbl = (void*)MALLOC(FREEBLKS_PER_ZONE * sizeof(struct DataBlkTblType));
        
  }
  


    return TRUE;
}

/*!
*
* \par  Description:
*       This function free block map table cache memory .
*
* \param  [in]    void
* \return      sucess or failed
**/
INT32U BMM_ExitBlockMapTblCache(void)
{
    INT32S loop;

    for (loop = 0; loop < ZONE_CACHE_NUM; loop++)
    {

        FREE(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].DataBlkTbl);
        FREE(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].LogBlkTbl);
        FREE(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[loop].FreeBlkTbl);
    }

    return TRUE;
}





/*!
*
* \par  Description:
*       This function collect all dirty block map table.
*
* \param  [in]    void
* \return      sucess or failed
**/
/******************************************************************/
INT32U  BMM_WriteBackAllMapTbl(void)
{
    INT32S nBMM;

    /*protect current scene*/
    struct ZoneTblCacheType *tmptbl1 = BMT;
    struct PageMapTblCacheType *tmptbl2 = PMT;

    for (nBMM = 0; nBMM < ZONE_CACHE_NUM; nBMM++)
    {
        if (NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[nBMM].TblNeedUpdate)
        {
            BMT = &(NandDevInfo.ZoneTblCacheInfo->ZoneTblCache[nBMM]);
            if (TRUE != _WriteBlkMapTbl(CURZONE))
            {
                return FALSE;
            }
            BMT->TblNeedUpdate = 0;
        }

   }
   
    /*@fish add  Set Default the number of the logic block last acess */
  ///  FTL_SET_Default_Par();
 #if (SUPER_WEAR_LEVELLING ==1)   && DEBUG_ERASE_CNT ==0x01
    if((0x00==(WearDbg.wEraseBlkCnt%0x20))  && WearDbg.wEraseBlkCnt>0x50)
                 BUG_Wear("\n@#$<%s>_<%d>_ Total_Erase:0x%x @#$\n",__func__,__LINE__,WearDbg.wEraseBlkCnt);

     if( WearDbg.wEraseBlkCnt>0x50)
     {
         BUG_Wear("Wear_LevelCnt--SimpleMergeCnt --MoveMergeCnt --SwapMergeCnt\n");
         BUG_Wear("%12x,%12x,%12x,%12x\n",WearDbg.Wear_LevelCnt,  WearDbg.SimpleMergeCnt,
                    WearDbg.MoveMergeCnt,WearDbg.SwapMergeCnt);     
          Debug_ReadZoneTbl();
     }
 #endif 
 
    /*rescure current scene*/
    BMT = tmptbl1;
    PMT = tmptbl2;

    return TRUE;
}
