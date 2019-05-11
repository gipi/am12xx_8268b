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
#include "regdef.h"
#else
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#endif

extern INT32U PHY_PageRead(struct PhysicOpParameter *NandOpPar);
extern INT32U PHY_PageWrite(struct PhysicOpParameter *NandOpPar);
extern INT32U PHY_SyncNandOperation(INT32U BankNum);
extern INT32U PHY_WaitNandFlashReady(INT32U BankNum);
extern INT32U PHY_NandReset(INT32U ChipNum);
extern INT32U PHY_EraseSuperBlk(struct PhysicOpParameter *PhyOpPar);
extern INT32U PHY_CopyNandPage(struct PhysicOpParameter *NandOpSrcPar, struct PhysicOpParameter *NandOpDstPar);
extern INT32U BMM_TrySetDirtyFlag(void);
extern void   BMM_CalAccessCount(void);
extern INT32U BMM_ExitBlockMapTblCache(void);
extern INT32U BMM_InitBlkMapTblCache(void);
extern INT32U BMM_SwitchBlkMapTbl(INT8U ZoneNum);
extern INT32U BMM_WriteBackAllMapTbl(void);
extern void   PMM_CalAccessCount(void);
extern INT32U PMM_ExitPageMapTblCache(void);
extern INT32U PMM_InitPageMapTblCache(void);
extern INT32U PMM_SwitchPageMapTbl(INT32U LogicBlockNum);
extern INT32U FTL_MergeLogBlk(INT8U type);
extern void   _WriteBadBlkFlag(INT32U ZoneNum, INT32U SuperBlkNum);
extern INT32U FTL_BadBlkManage(struct DataBlkTblType * NewFreeBlk, struct DataBlkTblType * BadBlk, INT32U ErrPage);

#if SUPPORT_READ_RECLAIM
INT32U FTL_ReadReclaim(INT32U BufPageNum, INT32U Bitmap, void * Buf);
#endif

struct NandLogicCtlPar  LogicRWCtlPar;
struct SpareData WriteSpareData[2];
struct NandSpareData TmpSData;
static INT32U WLogBlkPosition;
INT32U volatile PhyBlkEraseCnt;
INT16U volatile LastFreeBlkPst = 0;
extern struct  _Debug_Wear  WearDbg;

/*
*********************************************************************************************************
*               CALCULATE PHYSIC OPERATION PARAMETERS
*
*Description: calculate the physic operation parameters by zone number, super block number,
*             and page number in super block;
*
*Arguments  : PhyOpPar      the pointer to the physic operation parameter;
*             ZoneNum       the zone number the super block belong to;
*             SupBlkNum     the super block number in a die;
*             PageNum       the page number in the super block;
*
*Return     : TRUE      create block mapping table success;
*             FALSE     create block mapping table failed;
*
*Note       : this function calculate the physic opeation parameters by zone number, super block
*             number and page number; it will check the operations the nand flash support, and
*             analyze how to translate the parameters under different cases;
*********************************************************************************************************
*/
INT32U  FTL_CalPhyOpPar(struct PhysicOpParameter *PhyOpPar, INT32U ZoneNum, INT32U SupBlkNum, INT32U PageNum)
{
    INT32U  tmpDieNum;
    INT32U  tmpBankNum = 0xffffffff;
    INT32U  tmpBlkNum = 0xffffffff;
    INT32U  tmpPageNum = 0xffffffff;

    FTL_DBG("FTL_DBG: calculate physic operation parameters.\n");
    FTL_DBG("         ZoneNum:0x%x, SupBlkNum:0x%x, PageNum: 0x%x\n", ZoneNum, SupBlkNum, PageNum);

    /* calculate the die number which the zone belonged to */
    tmpDieNum = ZoneNum / ZONE_NUM_PER_DIE;

    /* if nand flash support internal and external interleave */
    if(NAND_SUPPORT_EXTERNAL_INTERLEAVE && NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        /* calculate bank number, block number in bank and page number in block */
        tmpBankNum = PageNum % TOTAL_BNK_CNT;
        tmpBlkNum = SupBlkNum;
        tmpPageNum = PageNum / TOTAL_BNK_CNT;
    }

    if(NAND_SUPPORT_EXTERNAL_INTERLEAVE && !NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        /* calculate bank number, block number in bank and page number in block */
        tmpBankNum = PageNum % TOTAL_BNK_CNT;
        tmpBlkNum = SupBlkNum + tmpDieNum * (BLK_CNT_PER_DIE / PLANE_CNT_PER_DIE);
        tmpPageNum = PageNum / TOTAL_BNK_CNT;
    }

    if(!NAND_SUPPORT_EXTERNAL_INTERLEAVE && NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        /* calculate bank number, block number in bank and page number in block */
        tmpBankNum = (PageNum % BNK_CNT_PER_CHIP) + tmpDieNum * BNK_CNT_PER_CHIP;
        tmpBlkNum = SupBlkNum;
        tmpPageNum = PageNum / BNK_CNT_PER_CHIP;
    }

    if(!NAND_SUPPORT_EXTERNAL_INTERLEAVE && !NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        /* calculate bank number, block number in bank and page number in block */
        tmpBankNum = tmpDieNum / DIE_CNT_PER_CHIP;
        tmpBlkNum = SupBlkNum + (tmpDieNum % DIE_CNT_PER_CHIP) * (BLK_CNT_PER_DIE / PLANE_CNT_PER_DIE);
        tmpPageNum = PageNum;
    }

    /* set the return value */
    PhyOpPar->BankNum = tmpBankNum;
    PhyOpPar->PhyBlkNumInBank = tmpBlkNum;
    PhyOpPar->PageNum = tmpPageNum;

    FTL_DBG("FTL_CalPhyOpPar:BankNum:0x%x, PhyBlkNumInBank:0x%x, PageNum: 0x%x\n", tmpBankNum, tmpBlkNum, tmpPageNum);

    return TRUE;
}


/*
*********************************************************************************************************
*               CALCULATE LOGIC PAGE PARAMETERS
*
*Description: calculate the logic parameters by buffer page number, sector bitmap
*
*Arguments  : LogicPage     the pointer to the logic page parameter
*             BufPageNum    buffer page number calculate based on buffer size
*             Bitmap        the bitmap
*
*Return     : TRUE      create block mapping table success;
*             FALSE     create block mapping table failed;
*
*Note       : this function calculate the physic opeation parameters by zone number, super block
*             number and page number; it will check the operations the nand flash support, and
*             analyze how to translate the parameters under different cases;
*********************************************************************************************************
*/
INT32U _CalLogicPagePar(struct LogicPagePar *LogicPage, INT32U BufPageNum, INT32U Bitmap)
{
    INT32U tmpBlkNum;
    INT32U tmpPageNum;
    INT32U tmpBitmap;
    INT32U tmpZoneNum;

    FTL_DBG("FTL_DBG: calculate logical page parameters.\n");
    FTL_DBG("         buffer page number:0x%x, sector bitmap:0x%x\n", BufPageNum, Bitmap);

    tmpPageNum = BufPageNum;
    tmpBitmap = Bitmap;

    /* if buffer page size is not same as super page size, need process */
    if(PAGE_BUFFER_SECT_NUM != SECTOR_NUM_PER_SUPER_PAGE)
    {
        if(tmpPageNum % PLANE_CNT_PER_DIE)
        {
            tmpBitmap <<= SECTOR_NUM_PER_SINGLE_PAGE;
        }
        tmpPageNum /= PLANE_CNT_PER_DIE;
    }

    /* calculate global logic block number */
    tmpBlkNum = tmpPageNum / PAGE_CNT_PER_LOGIC_BLK;
    /* calculate page number in logic block */
    tmpPageNum %= PAGE_CNT_PER_LOGIC_BLK;

    /* calculate zone number */
    tmpZoneNum = tmpBlkNum / DATA_BLK_NUM_PER_ZONE;
    /* calculate logic block number in zone */
    tmpBlkNum %= DATA_BLK_NUM_PER_ZONE;

    /* set return parameters */
    LogicPage->ZoneNum = tmpZoneNum;
    LogicPage->LogicBlkNumInZone = tmpBlkNum;
    LogicPage->LogicPageNum = tmpPageNum;
    LogicPage->SectorBitmap = tmpBitmap;

    FTL_DBG("_CalLogicPagePar:ZoneNum:0x%x, BlkNum:0x%x, PageNum:0x%x, Bitmap:0x%x\n",
                        tmpZoneNum, tmpBlkNum, tmpPageNum, tmpBitmap);

    return TRUE;
}


/*
*********************************************************************************************************
*               CLOSE LAST WRITE PAGE
*
*Description: check if last operation is write, if so, close last write page, need wait nand flash
*             ready, and if the page data is not full, need copy the empty single page.
*
*Arguments  : none
*
*Return     : TRUE      close page ok;
*             FALSE     close page failed failed;
*
*Note       : this function need check if the nand flash is true ready. and if the super page data
*             is not full, need copy the empty single page.
*********************************************************************************************************
*/

INT32U  _CloseLastWritePage(void)
{
    struct PhysicOpParameter tmpLastWPagePar;
    INT32U ret;    
    /* check if last operation is write */
    if(LogicRWCtlPar.LastOp == 'w')
    {
        /* calculate last write page parameters */
        FTL_CalPhyOpPar(&tmpLastWPagePar, LogicRWCtlPar.ZoneNum, LogicRWCtlPar.PhyLogBlk.PhyBlkNum,
                                          LogicRWCtlPar.LogPageNum);

        /* synchronize last operation */
        PHY_SyncNandOperation(tmpLastWPagePar.BankNum);
        PHY_WaitNandFlashReady(tmpLastWPagePar.BankNum);

        /* check if need copy page */
        if((LogicRWCtlPar.LogPageBitmap != FULL_BITMAP_OF_SUPER_PAGE) && (LogicRWCtlPar.LogPageBitmap != 0x00))
        {
            struct PhysicOpParameter SrcPagePar;
            struct PhysicOpParameter DstPagePar;

            /* calculate source page parameter and destination page parameter */
            FTL_CalPhyOpPar(&SrcPagePar, LogicRWCtlPar.ZoneNum, LogicRWCtlPar.LogPageLastBlk.PhyBlkNum,
                                         LogicRWCtlPar.LogPageLastPageNum);
            FTL_CalPhyOpPar(&DstPagePar, LogicRWCtlPar.ZoneNum, LogicRWCtlPar.PhyLogBlk.PhyBlkNum,
                                         LogicRWCtlPar.LogPageNum);

            /* set sector bitmap to copy data */
            SrcPagePar.SectorBitmapInPage = LogicRWCtlPar.LogPageBitmap ^ FULL_BITMAP_OF_SUPER_PAGE;
            DstPagePar.SectorBitmapInPage = SrcPagePar.SectorBitmapInPage;

            /* set buffer address for copy a page */
            SrcPagePar.MainDataPtr = PAGE_COPYBACK_BUFFER;
            SrcPagePar.SpareDataPtr = NULL;

            DstPagePar.MainDataPtr = PAGE_COPYBACK_BUFFER;
            DstPagePar.SpareDataPtr = WriteSpareData;

            /* call physic page read and page write to copy a page */
            PHY_PageRead(&SrcPagePar);
            PHY_PageWrite(&DstPagePar);

            /* synchronize last operation */
            ret = PHY_SyncNandOperation(tmpLastWPagePar.BankNum);
            ret |= PHY_WaitNandFlashReady(tmpLastWPagePar.BankNum);
            FTL_ERR("warning Enter %s %d\n",__func__,__LINE__);
            if(ret != TRUE)
            {
                FTL_ERR("error %s %d\n",__func__,__LINE__);    
            }
            LogicRWCtlPar.LogPageBitmap = 0x00;
        }

        LogicRWCtlPar.LastOp = 'n';
    }

    return TRUE;
}


/*
*********************************************************************************************************
*               BLOCK MAPPING MANAGEMENT GET DATA BLOCK NUMBER
*
*Description: get the physic block number from data block mapping table.
*
*Arguments  : LogicBlockNum     the logic block number whose physic block number is needed get;
*             DataBlkNum        the pointer to the data block number parameter
*
*Return     : result
*
*Notes      : this function just get block number from ram.
*********************************************************************************************************
*/
INT32U  _GetDataBlkNum(INT32U LogicBlockNum, struct DataBlkTblType *DataBlkNum)
{
    if(LogicBlockNum > DATA_BLK_NUM_PER_ZONE)
    {
        FTL_ERR("FTL_ERR: get data block number error, LogicBlkNumInZone is:0x%x", LogicBlockNum);
        return LOGICAL_LAYER_ERROR;
    }

    *DataBlkNum = CUR_DATA_BLK_TABLE[LogicBlockNum];

    return TRUE;
}


/*
*********************************************************************************************************
*               SEARCH LOG BLOCK BY LOGIC BLOCK NUMBER
*
*Description: scan the log block table to find if the logic block contain a log block.
*
*Arguments  : LogicBlockNum     the logic block number whose log block is needed search;
*
*Return     : result
*                   TRUE        search success, the logic block contain a log block;
*                   FALSE       searce failed, the logic block doesn't contain a log block;
*********************************************************************************************************
*/
INT32U  _SearchLogBlk(INT32U LogicBlockNum, INT32U *LogBlkPst)
{
    INT32S i;

    for(i=0; i<MAX_LOG_BLK_NUM; i++)
    {
        if(CUR_LOG_BLK_TABLE[i].LogicBlkNum == LogicBlockNum)
        {
            if(LogBlkPst != NULL)
            {
                *LogBlkPst = i;
            }
            return TRUE;
        }
    }

    return FALSE;
}

/*
*********************************************************************************************************
*               GET LOG PAGE NUMBER FROM PAGE MAPPING TABLE
*
*Description: get log page number from page mapping table.
*
*Arguments  : LogicPageNum      the logic page number whose log page is needed;
*             LogPageNum        the pointer to log page number return value;
*             GetType           get type, for read or write;
*
*Return     : result
*                   TRUE        get log page success;
*                   FALSE       get log page failed;
*********************************************************************************************************
*/
INT32U  _GetLogPageNum(INT32U LogicPageNum, struct PageMapTblType *LogPageNum, INT32U GetType)
{
    INT32U  result;
    struct PageMapTblType tmpLogPage;

    if(GetType == 'r')
    {
        /* look for the page for read */
        *LogPageNum = CUR_PAGE_MAP_TABLE[LogicPageNum];

        /* check if the page number is valid */
        if(LogPageNum->PhyPageNum != 0xfff)
        {
            /* the logic page contain a log page, get log page ok */
            result = TRUE;
        }
        else
        {
            /* the logic page doesn't contain a log page */
            result = FALSE;
        }
    }
    else
    {
        /* look for a log page to write data */
        INT32U tmpLogBlkPst;
        INT32U tmpBankTotal;
        INT32U tmpBankNum;

        tmpLogBlkPst =0xFFF;
        /* calculate the bank total, and bank number of current need log page */
        tmpBankTotal = PAGE_CNT_PER_LOGIC_BLK / PAGE_CNT_PER_PHY_BLK;
        tmpBankNum = LogicPageNum % tmpBankTotal;

        /* search log block position */
         _SearchLogBlk(CUR_LOG_BLK_TABLE[CUR_PAGE_MAP_CACHE->LogicBlkNum].LogicBlkNum, &tmpLogBlkPst);

        /* get log page from last used position */
        tmpLogPage.PhyPageNum = CUR_LOG_BLK_TABLE[tmpLogBlkPst].LastUsedPage;

        #if (SUPPORT_ALIGN_BANK)
        /* calculate the log page number, the log page and the data page must in the same bank */
        if(tmpBankNum > (tmpLogPage.PhyPageNum % tmpBankTotal))
        {
            tmpLogPage.PhyPageNum = tmpLogPage.PhyPageNum + tmpBankNum - (tmpLogPage.PhyPageNum % tmpBankTotal);
        }
        else
        {
            tmpLogPage.PhyPageNum = tmpLogPage.PhyPageNum + tmpBankNum + (tmpBankTotal - (tmpLogPage.PhyPageNum % tmpBankTotal));
        }
        #else
        /* log page number is just the last used page increase */
        tmpLogPage.PhyPageNum = tmpLogPage.PhyPageNum + 1;
        #endif

        /* check if the log page number is valid */
        if(tmpLogPage.PhyPageNum < PAGE_CNT_PER_LOGIC_BLK)
        {
            LogPageNum->PhyPageNum = tmpLogPage.PhyPageNum;
            result = TRUE;
        }
        else
        {
            FTL_DBG("FTL_DBG: log page number is invalid when get log page!\n");
            LogPageNum->PhyPageNum = 0xfff;
            result = FALSE;
        }
    }

    return result;
}


/*
*********************************************************************************************************
*                       GET FREE BLOCK
*
*Description: scan the free block table to find a free block with lowest erase count
*             or highest erase count.
*
*Arguments  : FreeBlkPst    the pointer to the free block position;
*             GetType       the type of the free block needed, lowest Ec or highest Ec;
*
*Return     : result
*                   TRUE        get success;
*                   FALSE       get failed;
*********************************************************************************************************
*/
INT32U  _GetFreeBlk(INT32U *FreeBlkPst, INT32U GetType)
{
    INT32S i;
    INT32U tmpEraseCnt;

    INT32U tmpFreeBlkPst = 0xffffffff;
    INT32U tmpFreeBlkCnt = 0xffffffff;
    INT16U start = LastFreeBlkPst + 1;

    tmpFreeBlkCnt = 0;

    if(GetType == LOWEST_EC_TYPE)
    {
    #if _NEW_ZONE_TBL_==0x01 
        tmpEraseCnt = MAX_ERASE_CNT; 
    #else
        tmpEraseCnt = 0xff;
    #endif             
    }
    else
    {
        tmpEraseCnt = 0x00;
    }

    for(i=0; i < BLK_NUM_PER_ZONE - 1 - DATA_BLK_NUM_PER_ZONE - 1; i++,start++)
    {
        if(start == BLK_NUM_PER_ZONE - 1 - DATA_BLK_NUM_PER_ZONE)
        {
            start = 0;
        }

        /* check if the item is valid */
        if(CUR_FREE_BLK_TABLE[start].PhyBlkNum != 0xfff)
        {
            if(((GetType == LOWEST_EC_TYPE) && (CUR_FREE_BLK_TABLE[start].EraseCnt < tmpEraseCnt))
                || ((GetType != LOWEST_EC_TYPE) && (CUR_FREE_BLK_TABLE[start].EraseCnt > tmpEraseCnt)))
            {
                tmpEraseCnt = CUR_FREE_BLK_TABLE[start].EraseCnt;
                tmpFreeBlkPst = start;
            }

            tmpFreeBlkCnt++;
        }
    }

    if(tmpFreeBlkCnt < 1)
    {
        /* the free block count is too few */
        FTL_ERR("FTL_ERR: can't allocate free block from free block table!\n");
        return FALSE;
    }
    else
    {
        *FreeBlkPst = tmpFreeBlkPst;
        LastFreeBlkPst = tmpFreeBlkPst;
        return TRUE;
    }
}


#if SUPPORT_WEAR_LEVELLING

INT32U  FTL_Clear_CodeDataBlock(void)
{
       INT32U ii,jLoop;
       INT32U result;
       INT32U tmpMergeTypeFlag;
        INT32U tmpOldLogBlkPst;
        BUG_Wear("\n__<%s>_ Merge_Block_<%d>__\n",__func__,__LINE__);
        jLoop =0x00;
       for(ii=0x00;ii<CODE_DATA_BLK;ii++)
       {     
            result = _SearchLogBlk(ii, &tmpOldLogBlkPst);
            if(result == TRUE)
            {
                     tmpMergeTypeFlag = NORMAL_MERGE;
                     BUG_Wear("No:%2x,%2x,%4x, ",ii,tmpOldLogBlkPst,
                                    CUR_LOG_BLK_TABLE[tmpOldLogBlkPst].LogicBlkNum );
                    if(0x00==(((jLoop++)+1)%5))
                             BUG_Wear("\n");
                       /* swap the page mapping table to ram */
                    result = PMM_SwitchPageMapTbl(CUR_LOG_BLK_TABLE[tmpOldLogBlkPst].LogicBlkNum);
                    if(result != TRUE)
                    {
                        FTL_ERR("FTL_ERR: swap page mapping table failed when create new log block!\n");
                        return result;
                    }
                    /* merge the eldest log block */
                    result = FTL_MergeLogBlk(tmpMergeTypeFlag);
                    if(result != TRUE)
                    {
                        FTL_ERR("FTL_ERR: merge log block failed when create new log block!\n");
                        return result;
                    }
                    
            }
       }        
       return 0x00;
}

/*
*********************************************************************************************************
*               FLASH TRANSLATE LAYER WEAR-LEVELLING
*
*Description: wear levelling.
*
*Arguments  : none
*
*Return     : Result
*               TRUE    do wear-levelling ok;
*               FALSE   do wear-levelling failed;
*
*Notes      : this function read the erase count of the blocks in a zone, and find a data block which
*             has the lowest erase count to replace a free block which has the highest erase count,
*             the highest free block erase count must much higher than the lowest data block erase count
*********************************************************************************************************
*/
INT32U  FTL_WearLevelling(void)
{
	INT32S i;

	INT32U result;
	INT32U tmpEraseCnt;
	INT32U tmpHighestECFreeBlk;
	struct PhysicOpParameter tmpPhyROpPar;
	struct PhysicOpParameter tmpPhyWOpPar;
	INT32U tmpLowestECDataBlk = 0xffffffff;
	INT32S  bMinDatablk;
    
	/* if last operation is write, need close last write page */
	_CloseLastWritePage();

	WearDbg.Wear_LevelCnt +=1;

	/* search the data block which has the lowest erase count */
#if _NEW_ZONE_TBL_==0x01 
	tmpEraseCnt = MAX_ERASE_CNT; 
#else
	tmpEraseCnt = 0xff;
#endif  
#if CODE_PROTECT_WEAR_LEVELING ==0x01
	if(CUR_ZONE_NUM ==0x00)
	{
		bMinDatablk =CODE_DATA_BLK;
	}
	else
#endif   
	{
		bMinDatablk =0x00;
	}
   
    for(i = DATA_BLK_NUM_PER_ZONE - 1; i >= bMinDatablk; i--)
    {
        if(CUR_DATA_BLK_TABLE[i].EraseCnt < tmpEraseCnt)
        {
            tmpEraseCnt = CUR_DATA_BLK_TABLE[i].EraseCnt;
            tmpLowestECDataBlk = i;
        }
    }

    /* search the free block which has the highest erase count */
    result = _GetFreeBlk(&tmpHighestECFreeBlk, HIGHEST_EC_TYPE);
    if(result != TRUE)
    {
        FTL_ERR("FTL_ERR: get free block failed when do wear-levelling!\n");
        return result;
    }

    /*if data blocks erased more than free blocks , do nothing*/
    if (tmpEraseCnt >= CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].EraseCnt)
    {
        if (tmpEraseCnt == MAX_ERASE_CNT ) /*erase count over the limit , so clear all*/
        {
            INIT_BOOT("\n\n_<%s>_<%d>erase_count_over_the_limit ,_so_clear_all\n\n",__func__,__LINE__);
            for(i=0; i<DATA_BLK_NUM_PER_ZONE; i++)
            {
                CUR_DATA_BLK_TABLE[i].EraseCnt = 0;
            }
        }
        PhyBlkEraseCnt = 0;
        return TRUE;
    }

    /* check if the data block with lowest erase count contain a log block */
    result = _SearchLogBlk(tmpLowestECDataBlk, NULL);
    if(result != TRUE)
    {
        struct NandSpareData oob;
        /*judge the data block free or dirty?*/
        FTL_CalPhyOpPar(&tmpPhyROpPar, ZoneTblCache.CurZoneTbl->ZoneNum,
                        CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].PhyBlkNum, 0);
		
        tmpPhyROpPar.SectorBitmapInPage = SINGLE_BITMAP_SPAREDATA ;//0x3;
        tmpPhyROpPar.SpareDataPtr = &oob;
        tmpPhyROpPar.MainDataPtr= PAGE_COPYBACK_BUFFER;
        PHY_PageRead(&tmpPhyROpPar);

        /*if data block is not free,copying data needed*/

	if(oob.UserData[0].SpareData0.LogicBlkInfo != 0xffff)  

        {
            /* copy the data */
            for(i=0; i<PAGE_CNT_PER_LOGIC_BLK; i++)
            {
                FTL_CalPhyOpPar(&tmpPhyROpPar, ZoneTblCache.CurZoneTbl->ZoneNum,
                        CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].PhyBlkNum, i);

                FTL_CalPhyOpPar(&tmpPhyWOpPar, ZoneTblCache.CurZoneTbl->ZoneNum,
                        CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].PhyBlkNum, i);

                tmpPhyROpPar.SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;
                tmpPhyWOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;

                /* sync nand flash operation, and waite nand flash ready */
                PHY_SyncNandOperation(tmpPhyROpPar.BankNum);
                PHY_WaitNandFlashReady(tmpPhyROpPar.BankNum);

                /* copy a full super page */
                result = PHY_CopyNandPage(&tmpPhyROpPar, &tmpPhyWOpPar);
                if(result != TRUE)
                {
                    FTL_ERR("%s %d ERROR!\n",__func__,__LINE__);
                }
            }
        }
   #if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)   
   	if(0x00 == (WearDbg.Wear_LevelCnt%0x10))
   	{
		BUG_Wear("\nN:%4x,%4x,S",WearDbg.wEraseBlkCnt,WearDbg.Wear_LevelCnt);
		BUG_Wear("p:%4x,Mv:%4x,Sw:%4x,",WearDbg.SimpleMergeCnt,WearDbg.MoveMergeCnt,WearDbg.SwapMergeCnt);   
        BUG_Wear("%x,%x ",WearDbg.Error_Bit,WearDbg.Error_Byte);
		BUG_Wear("Dta:%3x,%d,%3x,%x,",tmpLowestECDataBlk,CUR_ZONE_NUM,CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].PhyBlkNum,
			CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].EraseCnt);
		BUG_Wear("Fre:%2x,%3x,%x&&\n",tmpHighestECFreeBlk,CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].PhyBlkNum,
			CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].EraseCnt);
   	} 
   #endif
     
        /* exchange the item of the data block table and free block table */
        {
            struct DataBlkTblType tmpBlk;
            tmpBlk = CUR_DATA_BLK_TABLE[tmpLowestECDataBlk];
            CUR_DATA_BLK_TABLE[tmpLowestECDataBlk] = CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk];
            CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk] = tmpBlk;
        }

        /*if data block is not free,erase it*/

	if (oob.UserData[0].SpareData0.LogicBlkInfo != 0xffff)

        {
            /* erase data block */
            FTL_CalPhyOpPar(&tmpPhyWOpPar, ZoneTblCache.CurZoneTbl->ZoneNum,
                            CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].PhyBlkNum, 0);

            /* sync nand flash operation, and waite nand flash ready */
            PHY_SyncNandOperation(tmpPhyROpPar.BankNum);
            PHY_WaitNandFlashReady(tmpPhyROpPar.BankNum);

            result = PHY_EraseSuperBlk(&tmpPhyWOpPar);
            if(result == TRUE)
            {
                /* process the erase count */
                if(CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].EraseCnt < MAX_ERASE_CNT)
                {
                    CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].EraseCnt++;
                }
            }
            else
            {
                /* write bad block flag */
                _WriteBadBlkFlag(ZoneTblCache.CurZoneTbl->ZoneNum, CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk].PhyBlkNum);

                /* the data block is a bad block */
                *(INT16U*)&(CUR_FREE_BLK_TABLE[tmpHighestECFreeBlk]) = 0xffff;
            }
        }
    }
#if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)   
    else
    {
        CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].EraseCnt ++; 
        
        BUG_Wear("\n&&& %5x,Data_:%3x,%3x",WearDbg.wEraseBlkCnt,tmpLowestECDataBlk, tmpHighestECFreeBlk);      
        BUG_Wear("%x,%x ",WearDbg.Error_Bit,WearDbg.Error_Byte);
        BUG_Wear("blk:%3x,%x\n",CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].PhyBlkNum,CUR_DATA_BLK_TABLE[tmpLowestECDataBlk].EraseCnt);
	
    }
#endif
    /* clear the physic block erase counter */
    PhyBlkEraseCnt = 0;

    return TRUE;
}
#endif


/*
*********************************************************************************************************
*                       CREATE NEW LOG BLOCK
*
*Description: create a new log block.
*
*Arguments  : LogicBlkNum   the logic block number which need create a log block;
*
*Return     : result
*                   TRUE        create log block success;
*                   FALSE       create log block failed;
*********************************************************************************************************
*/
INT32U  _CreateNewLogBlk(INT32U LogicBlkNum)
{
	INT32S i;

	INT32U tmpEmptyLogBlkPst;
	INT32U tmpLogBlkAge;
	INT32U tmpOldLogBlkPst;
	INT32U tmpFreeBlkPst;
	INT32U tmpMergeTypeFlag;
	INT32U result;
	struct PhysicOpParameter tmpPhysicPar;

	tmpEmptyLogBlkPst = 0xff;
	tmpLogBlkAge = 0xff;
	tmpMergeTypeFlag = SPECIAL_MERGE;
	tmpOldLogBlkPst =0xFFF;
    /* check if the logic block has contained a log block already */
    result = _SearchLogBlk(LogicBlkNum, &tmpOldLogBlkPst);
    if(result != TRUE)
    {
        for(i=0; i<MAX_LOG_BLK_NUM; i++)
        {
            /* search if there is a empty item in log block table */
            if(CUR_LOG_BLK_TABLE[i].LogicBlkNum == 0xffff)
            {
                tmpEmptyLogBlkPst = i;
                break;
            }

            /* look for the eldest access log block */
            if(LogicRWCtlPar.LogBlkAccessAge[i] < tmpLogBlkAge)
            {
                tmpLogBlkAge = LogicRWCtlPar.LogBlkAccessAge[i];
                tmpOldLogBlkPst = i;
            }
#if 1
            /* look for the eldest access log block. TODO: firstly choose log block that can use swap merge */
            if(CUR_LOG_BLK_TABLE[i].LastUsedPage == (PAGE_CNT_PER_LOGIC_BLK - 1))
            {
                tmpLogBlkAge = LogicRWCtlPar.LogBlkAccessAge[i];
                tmpOldLogBlkPst = i;
                break;
            }
#endif
        }

        tmpMergeTypeFlag = NORMAL_MERGE;
    }

    /* if there is no empty item in log block table, need merge the eldest log block */
    if(tmpEmptyLogBlkPst == 0xff)
    {
        /* swap the page mapping table to ram */
        result = PMM_SwitchPageMapTbl(CUR_LOG_BLK_TABLE[tmpOldLogBlkPst].LogicBlkNum);
        if(result != TRUE)
        {
            FTL_ERR("FTL_ERR: swap page mapping table failed when create new log block!\n");
            return result;
        }

        /* merge the eldest log block */
        result = FTL_MergeLogBlk(tmpMergeTypeFlag);
        if(result != TRUE)
        {
            FTL_ERR("FTL_ERR: merge log block failed when create new log block!\n");
            return result;
        }

        tmpEmptyLogBlkPst = tmpOldLogBlkPst;
    }

#if SUPPORT_WEAR_LEVELLING
	/*global erase count over the limit,do wearleaving */
	if(PhyBlkEraseCnt >= WEAR_LEVELLING_FREQUENCE)
	{
		FTL_WearLevelling();
	}
#endif

    tmpFreeBlkPst =0xFFFF;
    /* get a free block from free block table */
    result = _GetFreeBlk(&tmpFreeBlkPst, LOWEST_EC_TYPE);
    if(result != TRUE)
    {
        FTL_ERR("FTL_ERR: get free block failed when create new log block!\n");
        return result;
    }

   if(TRUE != _SearchLogBlk(LogicBlkNum,&tmpOldLogBlkPst))
   {
        /* set log block table item */
        CUR_LOG_BLK_TABLE[tmpEmptyLogBlkPst].LogicBlkNum = LogicBlkNum;
        CUR_LOG_BLK_TABLE[tmpEmptyLogBlkPst].PhyBlkNum = CUR_FREE_BLK_TABLE[tmpFreeBlkPst];
        CUR_LOG_BLK_TABLE[tmpEmptyLogBlkPst].LastUsedPage = 0xffff;
        /* delete free block table item */
        *(INT16U*)&CUR_FREE_BLK_TABLE[tmpFreeBlkPst] = 0xffff;
   }

    /* update the data block physic number, because item has changed */
    _GetDataBlkNum(LogicBlkNum, &LogicRWCtlPar.PhyDataBlk);

    /* init spare area data */
 //   MEMSET(&WriteSpareData, 0xff, NAND_SPARE_SIZE * 4);
MEMSET(&WriteSpareData, 0xff, sizeof(struct NandSpareData ));
	
    /* get the spare data from data block */
    FTL_CalPhyOpPar(&tmpPhysicPar, ZoneTblCache.CurZoneTbl->ZoneNum, CUR_DATA_BLK_TABLE[LogicBlkNum].PhyBlkNum, 0);
    tmpPhysicPar.SectorBitmapInPage = SINGLE_BITMAP_SPAREDATA ;//0x3;
	
    tmpPhysicPar.MainDataPtr = PAGE_COPYBACK_BUFFER;
    tmpPhysicPar.SpareDataPtr = WriteSpareData;
    PHY_PageRead(&tmpPhysicPar);
	/* check if data block is a clean block, and set a flag */

	if(WriteSpareData[0].SpareData0.LogicBlkInfo == 0xffff)
	{
		result = 1;
	}
	else
	{
		result = 0;
	}
	/* set the logic block information */
	{
		INT32U tmpZoneInDie = (ZoneTblCache.CurZoneTbl->ZoneNum) % ZONE_NUM_PER_DIE;

		WriteSpareData[0].SpareData0.LogicBlkInfo = LogicBlkNum | (tmpZoneInDie << 10);
		WriteSpareData[1].SpareData0.LogicBlkInfo = LogicBlkNum | (tmpZoneInDie << 10);

	}

    /* if data block is a clean block, need write the logic info first */
    if(result)
    {
        INT32S loop;
        for (loop = 0; loop < OPCYCLE; loop++)
        {
            tmpPhysicPar.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
            PHY_PageWrite(&tmpPhysicPar);
	
            result = PHY_SyncNandOperation(tmpPhysicPar.BankNum);
            if(result != TRUE)
            {
                FTL_ERR("%s %d ERR!\n",__func__,__LINE__);
            }
        }

    }

	/* increase the sequence value */

	WriteSpareData[0].SpareData1.LogStatus.LogBlkAge++;

    return TRUE;
}


/*
*********************************************************************************************************
*                       UPDATE PAGE MAPPING TABLE ITEM
*
*Description: update page mapping table item.
*
*Arguments  : LogicPageNum      logic page number,which item need update;
*             LogPageNum        log page number, which need be filled to the table;
*
*Return     : none;
*********************************************************************************************************
*/
void  _UpdatePageMapTblItem(INT32U LogicPageNum, INT32U LogPageNum)
{
    CUR_PAGE_MAP_TABLE[LogicPageNum].PhyPageNum = LogPageNum;
    CUR_PAGE_MAP_CACHE->TblNeedUpdate = NEED_UPDATE_PAGE_MAP_TBL;
}


/*
*********************************************************************************************************
*                       SET LOG BLOCK ACCESS TIME
*
*Description: set log bock access time.
*
*Arguments  : LogBlkPosition    the position that current access log block in the log block table;
*
*Return     : none;
*********************************************************************************************************
*/
void _SetLogBlkAccessTime(INT32U LogBlkPosition)
{
    LogicRWCtlPar.AccessTimer++;
    if(LogicRWCtlPar.AccessTimer == 0)
    {
        INT32S i;
        for(i=0; i<MAX_LOG_BLK_NUM; i++)
        {
            LogicRWCtlPar.LogBlkAccessAge[i] = 0;
        }
        LogicRWCtlPar.AccessTimer++;
    }
    LogicRWCtlPar.LogBlkAccessAge[LogBlkPosition] = LogicRWCtlPar.AccessTimer;
}


/*
*********************************************************************************************************
*               FLASH TRANSLATE LAYER PAGE BASED READ
*
*Description: read data from logic page to sram for flash translate layer inner.
*
*Arguments  : LogicPageNum  global logic page number;
*             Bitmap        the bitmap of sectors in current page read;
*             SramBuffer    the buffer address which the data be read to;
*
*Return     : TRUE      read ok;
*             FALSE     read failed;
*             FALSE|ECCERROR    the ecc error;
*             FALSE|ADDRERROR   the address is invalid;
*
*Notes      :
*********************************************************************************************************
*/
INT32U  FTL_LogicPageRead(INT32U BufPageNum, INT32U Bitmap, void* Buf)
{
    INT32U result;
    struct LogicPagePar CurReadPage;
    struct PageMapTblType tmpLogPage;
    struct PhysicOpParameter PhysicReadPar;

    /* calculate current page parameters */
    result = _CalLogicPagePar(&CurReadPage, BufPageNum, Bitmap);

    /* check if last operation is write, if so, need close last write */
    if(LogicRWCtlPar.LastOp != 'r')
    {
        if(LogicRWCtlPar.LastOp == 'w')
        {
            result = _CloseLastWritePage();
        }
        LogicRWCtlPar.LastOp = 'r';
    }

    /* check if the zone number has changed */
    if(CurReadPage.ZoneNum != LogicRWCtlPar.ZoneNum)
    {
        /* switch the zone table current access to zone table cache */
        result = BMM_SwitchBlkMapTbl(CurReadPage.ZoneNum);
        if(result)
        {
            FTL_ERR("FTL_ERR: switch block mapping table failed \n");
            return result;
        }

        LogicRWCtlPar.ZoneNum = CurReadPage.ZoneNum;

        /* zone table has changed, need clear the super block number */
        LogicRWCtlPar.SuperBlkNum = 0xffff;
    }

    /* check if the super block has changed */
    if(CurReadPage.LogicBlkNumInZone != LogicRWCtlPar.SuperBlkNum)
    {
        INT32U LogBlkPosition;

        /* get data block number from current access zone table */
        result = _GetDataBlkNum(CurReadPage.LogicBlkNumInZone, &LogicRWCtlPar.PhyDataBlk);
        if(result)
        {
            FTL_ERR("FTL_ERR: get data block number failed!\n");
            return result;
        }

        LogicRWCtlPar.SuperBlkNum = CurReadPage.LogicBlkNumInZone;

        /* search if current logic block contain a log block */
        result = _SearchLogBlk(CurReadPage.LogicBlkNumInZone, &LogBlkPosition);

        if(result == TRUE)
        {
            /* get log block number */
            LogicRWCtlPar.PhyLogBlk = CUR_LOG_BLK_TABLE[LogBlkPosition].PhyBlkNum;

            /* switch page mapping table for current access */
            result = PMM_SwitchPageMapTbl(CurReadPage.LogicBlkNumInZone);
            if(result)
            {
                FTL_ERR("FTL_ERR: switch page mapping table failed. \n");
                return result;
            }

            /* get log page number */
            result = _GetLogPageNum(CurReadPage.LogicPageNum, &tmpLogPage, 'r');

            if(result == TRUE)
            {
                /* current logic page contain a log page, and get log page success */
                LogicRWCtlPar.LogPageNum = tmpLogPage.PhyPageNum;
            }
            else
            {
                /* current logic page doesn't contaon a log page */
                LogicRWCtlPar.LogPageNum = 0xffff;
            }

            /* set the log block access time record */
            _SetLogBlkAccessTime(LogBlkPosition);
        }
        else
        {
            /* the logic block doesn't contain a log block */
            *(INT16U*)(&LogicRWCtlPar.PhyLogBlk) = 0xffff;
            LogicRWCtlPar.LogPageNum = 0xffff;
        }
    }/* end of <if(CurReadPage.LogicBlkNumInZone != LogicRWCtlPar.SuperBlkNum)>*/
    else
    {
        /* check if current logic block contain a log block */
        if(LogicRWCtlPar.PhyLogBlk.PhyBlkNum  != 0xfff)
        {
            result = _GetLogPageNum(CurReadPage.LogicPageNum, &tmpLogPage, 'r');
            if(result == TRUE)
            {
                /* current logic page contain a log page, and get log page success */
                LogicRWCtlPar.LogPageNum = tmpLogPage.PhyPageNum;
            }
            else
            {
                /* current logic page doesn't contaon a log page */
                LogicRWCtlPar.LogPageNum = 0xffff;
            }
        }
        else
        {
            LogicRWCtlPar.LogPageNum = 0xffff;
        }
    }

    /* set physic operation address */
    if((LogicRWCtlPar.PhyLogBlk.PhyBlkNum  != 0xfff) && (LogicRWCtlPar.LogPageNum != 0xffff))
    {
        /* log block and log page are valid, need get data from log page */
        result = FTL_CalPhyOpPar(&PhysicReadPar, CurReadPage.ZoneNum,
                                        LogicRWCtlPar.PhyLogBlk.PhyBlkNum, LogicRWCtlPar.LogPageNum);
    }
    else
    {
        /* data is in data block, need get data from data block */
        result = FTL_CalPhyOpPar(&PhysicReadPar, CurReadPage.ZoneNum,
                                        LogicRWCtlPar.PhyDataBlk.PhyBlkNum, CurReadPage.LogicPageNum);
    }

    PhysicReadPar.SectorBitmapInPage = CurReadPage.SectorBitmap;
    PhysicReadPar.MainDataPtr = Buf;
    PhysicReadPar.SpareDataPtr = NULL;
/*    if(bPrintFlag )
   {
        INIT_DRV("R:%x,%x,%x\n",PhysicReadPar.PageNum,PhysicReadPar.PhyBlkNumInBank,PhysicReadPar.SectorBitmapInPage);
      
   }*/

    /* call physic layer function to get data to ram */
    result = PHY_PageRead(&PhysicReadPar);

    /* set nand flash driver paramters */
    LogicRWCtlPar.SuperPageNum = CurReadPage.LogicPageNum;

    /*set cache access count*/
    if(LogicRWCtlPar.PhyLogBlk.PhyBlkNum != 0xfff)
    {
        PMM_CalAccessCount();
    }
    BMM_CalAccessCount();

    return result;
}


/*
*********************************************************************************************************
*               FLASH TRANSLATE LAYER PAGE BASED WRITE
*
*Description: write data to global logic page from sram for flash translate layer inner.
*
*Arguments  : LogicPageNum  global logic page number;
*             Bitmap        the bitmap of sectors in current page;
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
INT32U  FTL_LogicPageWrite(INT32U BufPageNum, INT32U Bitmap, void* Buf)
{
    INT32U result;
    INT32U tmpNeedGetNewLogPage;
    struct LogicPagePar CurWritePage;
    struct PhysicOpParameter PhysicWritePar;
	//INT32U ra_reg;
//	ra_reg = GET_CPU_REG(ra);
    /* calculate current page parameters */
    result = _CalLogicPagePar(&CurWritePage, BufPageNum, Bitmap);

    /* init physic parameter */
    PhysicWritePar.SectorBitmapInPage = 0;

    /* check if the zone number has changed */
    if(CurWritePage.ZoneNum != LogicRWCtlPar.ZoneNum)
    {
        /* close last write page, and swap current access zone table to ram */
        _CloseLastWritePage();
        result = BMM_SwitchBlkMapTbl(CurWritePage.ZoneNum);
        if(result != TRUE)
        {
            FTL_ERR("FTL_ERR: swap block mapping table failed!\n");
            return result;
        }

        LogicRWCtlPar.ZoneNum = CurWritePage.ZoneNum;

        /* zone table has changed, need clear the super block number */
        LogicRWCtlPar.SuperBlkNum = 0xffff;
    }

    /* check if need write dirty flag for zone table */
    if(NandDevInfo.ZoneTblCacheInfo->CurZoneTbl->TblNeedUpdate == 0)
    {
        /* try to set dirty flag for current zone table */
        BMM_TrySetDirtyFlag();
    }

    /* check if the super block has changed */
    if(CurWritePage.LogicBlkNumInZone != LogicRWCtlPar.SuperBlkNum)
    {
        /* close last write page, and swap current access zone table to ram */
        _CloseLastWritePage();

        /* get data block number from current access zone table */
        result = _GetDataBlkNum(CurWritePage.LogicBlkNumInZone, &LogicRWCtlPar.PhyDataBlk);
        if(result)
        {
            FTL_ERR("FTL_ERR: get data block number failed!\n");
            return result;
        }

        LogicRWCtlPar.SuperBlkNum = CurWritePage.LogicBlkNumInZone;
    }

    /* set need get new log page valid */
    tmpNeedGetNewLogPage = 0x01;

    /* set the logic block information */
    {
        INT32U tmpZoneInDie = (ZoneTblCache.CurZoneTbl->ZoneNum) % ZONE_NUM_PER_DIE;

	WriteSpareData[0].SpareData0.LogicBlkInfo = LogicRWCtlPar.SuperBlkNum | (tmpZoneInDie << 10);
	WriteSpareData[1].SpareData0.LogicBlkInfo = LogicRWCtlPar.SuperBlkNum | (tmpZoneInDie << 10);

    }

    /* check if last operation is write */
    if(LogicRWCtlPar.LastOp != 'w')
    {
        /* search if the logic block contain log block */
        result = _SearchLogBlk(CurWritePage.LogicBlkNumInZone, &WLogBlkPosition);
        if(result != TRUE)
        {
            /* create a new log block */
            result = _CreateNewLogBlk(CurWritePage.LogicBlkNumInZone);
            if(result != TRUE)
            {
                return result;
            }

            /* set the global parameter, the PhyLogBlk */
            _SearchLogBlk(CurWritePage.LogicBlkNumInZone, &WLogBlkPosition);
        }

        LogicRWCtlPar.PhyLogBlk = CUR_LOG_BLK_TABLE[WLogBlkPosition].PhyBlkNum;

        /* switch page mapping table for current access */
        result = PMM_SwitchPageMapTbl(CurWritePage.LogicBlkNumInZone);
        if(result)
        {
            FTL_ERR("FTL_ERR: switch page mapping table failed when logic page write!\n");
            return result;
        }

        /* set the log block access time record */
        _SetLogBlkAccessTime(WLogBlkPosition);
    }/* end of <if(LogicRWCtlPar.LastOp != 'w')>*/
    else
    {
        /* last operation is writing the same block, check if write is sequently write */
        if(BufPageNum == LogicRWCtlPar.LastWBufPageNum + 1)
        {
            /* check if need open new page to write */
            if(CurWritePage.LogicPageNum == LogicRWCtlPar.SuperPageNum)
            {
                tmpNeedGetNewLogPage = 0x00;
            }
        }
        else
        {
            /* not sequently write, need close last write page */
            _CloseLastWritePage();
        }
    }

    /* check if need open new log page to write */
    if(tmpNeedGetNewLogPage)
    {
        struct PageMapTblType tmpLogPageNum;

        /* get new log page to write */
        result = _GetLogPageNum(CurWritePage.LogicPageNum, &tmpLogPageNum, 'w');

        if(result != TRUE)
        {
            /* get log page to write failed, the log block is full, need create new log block */
            result = _CreateNewLogBlk(CurWritePage.LogicBlkNumInZone);
            if(result != TRUE)
            {
                FTL_ERR("FTL_ERR: create new log block failed when logic page write!\n");
                return result;
            }

            /* switch page mapping table to ram */
            result = PMM_SwitchPageMapTbl(CurWritePage.LogicBlkNumInZone);
            if(result != TRUE)
            {
                FTL_ERR("FTL_ERR: switch page mapping table to ram failed when logic page write!\n");
                return result;
            }

            /* get new log page again */
            result = _GetLogPageNum(CurWritePage.LogicPageNum, &tmpLogPageNum, 'w');
            if(result != TRUE)
            {
                FTL_ERR("FTL_ERR: get log page to write failed!\n");
                return result;
            }

            /* set the global parameter, the PhyLogBlk */
            _SearchLogBlk(CurWritePage.LogicBlkNumInZone, &WLogBlkPosition);
            LogicRWCtlPar.PhyLogBlk = CUR_LOG_BLK_TABLE[WLogBlkPosition].PhyBlkNum;

            /* set the log block access time record */
            _SetLogBlkAccessTime(WLogBlkPosition);
        }

        LogicRWCtlPar.LogPageNum = tmpLogPageNum.PhyPageNum;

        /* set the logic page data last position */
        result = _GetLogPageNum(CurWritePage.LogicPageNum, &tmpLogPageNum, 'r');
        if(result == TRUE)
        {
            /* the logic page data last position is in log block */
            LogicRWCtlPar.LogPageLastPageNum = tmpLogPageNum.PhyPageNum;
            LogicRWCtlPar.LogPageLastBlk =  LogicRWCtlPar.PhyLogBlk;
        }
        else
        {
            /* the logic page data last position is in data block */
            LogicRWCtlPar.LogPageLastPageNum =  CurWritePage.LogicPageNum;
            LogicRWCtlPar.LogPageLastBlk = LogicRWCtlPar.PhyDataBlk;
        }

        /* check if need process log page 0, because log page and data page must in the same bank, so the log
           page 0 may be not used, if so, need process page 0, because of the spare data */
        if((CUR_LOG_BLK_TABLE[WLogBlkPosition].LastUsedPage == 0xffff) && (LogicRWCtlPar.LogPageNum != 0))
        {
            /* write the spare data to log page 0 */

            FTL_ERR("waring enter %s %d\n",__func__,__LINE__);
		WriteSpareData[0].SpareData1.LogicPageNum = 0xffff;

		MEMSET(LOGIC_WRITE_PAGE_CACHE, 0xff, PAGE_BUFFER_SECT_NUM * NAND_SECTOR_SIZE);
		FTL_CalPhyOpPar(&PhysicWritePar, LogicRWCtlPar.ZoneNum, LogicRWCtlPar.PhyLogBlk.PhyBlkNum, 0x00);
		PhysicWritePar.SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;
		PhysicWritePar.MainDataPtr = LOGIC_WRITE_PAGE_CACHE;
		PhysicWritePar.SpareDataPtr = WriteSpareData;
            result |= PHY_PageWrite(&PhysicWritePar);
            result |= PHY_SyncNandOperation(PhysicWritePar.BankNum);
            FTL_ERR("warning enter %s %d \n",__func__,__LINE__);
            if(result != TRUE)
            {
                FTL_ERR("error %s %d \n",__func__,__LINE__);
            }
        }

        /* set last used page number of the log block at log block table */
        CUR_LOG_BLK_TABLE[WLogBlkPosition].LastUsedPage = LogicRWCtlPar.LogPageNum;

        /* clear log page sector bitmap */
        LogicRWCtlPar.LogPageBitmap = 0x00;
    }
    LogicRWCtlPar.LastOp = 'w';
redo:
    /* check if the sector bitmap is not 0, 0 mean's need not write data */
    if(Bitmap != 0)
    {
        /* check if the buffer data is full */
        if(Bitmap != FULL_BITMAP_OF_BUFFER_PAGE)
        {
            /* buffer page data is not full need get data to fill it */
            FTL_CalPhyOpPar(&PhysicWritePar, LogicRWCtlPar.ZoneNum, LogicRWCtlPar.LogPageLastBlk.PhyBlkNum,
                                    LogicRWCtlPar.LogPageLastPageNum);

            /* set physic operation parameters, and get data */
            if(CurWritePage.SectorBitmap & FULL_BITMAP_OF_BUFFER_PAGE)
            {
                PhysicWritePar.SectorBitmapInPage = CurWritePage.SectorBitmap ^ FULL_BITMAP_OF_BUFFER_PAGE;
                CurWritePage.SectorBitmap = FULL_BITMAP_OF_BUFFER_PAGE;
            }
            else
            {
                PhysicWritePar.SectorBitmapInPage = CurWritePage.SectorBitmap ^ FULL_BITMAP_OF_SUPER_PAGE ^ FULL_BITMAP_OF_SINGLE_PAGE;
                CurWritePage.SectorBitmap = (FULL_BITMAP_OF_SUPER_PAGE ^ FULL_BITMAP_OF_SINGLE_PAGE);
            }
            PhysicWritePar.SpareDataPtr = NULL;

            /* set buffer address, for the physic page read buffer rule */
            {
                INT32S i = 0;
                while(Bitmap & (1<<i))
                {
                    i++;
                }
                PhysicWritePar.MainDataPtr = (INT8U*)Buf + i * NAND_SECTOR_SIZE;
            }

            /* need synch the nand flash operation first, maybe last operation is write, just write half page */
            PHY_SyncNandOperation(PhysicWritePar.BankNum);
            PHY_PageRead(&PhysicWritePar);
	
        }

        WriteSpareData[0].SpareData1.LogicPageNum = CurWritePage.LogicPageNum;
        WriteSpareData[1].SpareData1.LogicPageNum = CurWritePage.LogicPageNum;

        if(LogicRWCtlPar.LogPageNum != 0)
        {
            WriteSpareData[0].SpareData1.LogStatus.PhyPageStatus = DATA_PAGE_FLAG;
            WriteSpareData[1].SpareData1.LogStatus.PhyPageStatus = DATA_PAGE_FLAG;
        }

        /* wrtie data to nand flash */
        FTL_CalPhyOpPar(&PhysicWritePar, LogicRWCtlPar.ZoneNum, LogicRWCtlPar.PhyLogBlk.PhyBlkNum,
                                            LogicRWCtlPar.LogPageNum);

        /* init physic parameter bitmap */
        PhysicWritePar.SectorBitmapInPage = 0;

        /* set physic operation parameters, and get data */
        if(CurWritePage.SectorBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
        {
            /* the bitmap of low plane page is valid */
            PhysicWritePar.SectorBitmapInPage |= FULL_BITMAP_OF_SINGLE_PAGE;
        }

        if(CurWritePage.SectorBitmap & (FULL_BITMAP_OF_SUPER_PAGE ^ FULL_BITMAP_OF_SINGLE_PAGE))
        {
            /* the bitmap of high plane page is valid if high plane exist */
            PhysicWritePar.SectorBitmapInPage |= FULL_BITMAP_OF_SUPER_PAGE ^ FULL_BITMAP_OF_SINGLE_PAGE;
        }

        PhysicWritePar.MainDataPtr = Buf;
        PhysicWritePar.SpareDataPtr = WriteSpareData;
        if(LogicRWCtlPar.LogPageBitmap == 0x00)
        {
            /* check if need synch current bank operation */
            PHY_SyncNandOperation(PhysicWritePar.BankNum);
        }
	//if(bPrintFlag )
	{
	///INIT_BOOT("Write:%x,%x,%x,\n",PhysicWritePar.PageNum,PhysicWritePar.PhyBlkNumInBank,PhysicWritePar.SectorBitmapInPage);             
	}
#if 0	
 	INT8U *tmp2,*sp;
  	INT32U bitmap2;
	INT16U  Page2;
	tmp2= PhysicWritePar.MainDataPtr;
	sp= PhysicWritePar.SpareDataPtr;
	bitmap2 =PhysicWritePar.SectorBitmapInPage;
	Page2 = PhysicWritePar.PageNum;
	PhysicWritePar.SectorBitmapInPage =0xFFFF;
	//PhysicWritePar.SpareDataPtr = (INT8U *)&TmpSData;
	PhysicWritePar.MainDataPtr=PHY_TESTBUF;
	//INIT_BOOT("2 PageNum:%x\n",PhysicWritePar.PageNum);
	///if(PhysicWritePar.PageNum ==0x40)
	{
		//ReadBlock(&PhysicWritePar,PhysicWritePar.PageNum);
		///INIT_BOOT("delay\n");
		//while(1);
	}
	//INIT_BOOT("3 PageNum:%x\n",PhysicWritePar.PageNum);
//	PhysicWritePar.PhyBlkNumInBank =1925;
//	PHY_EraseSuperBlk(&PhysicWritePar);
	
	
	if(1)//if(PhysicWritePar.PageNum ==0x00)
	{
		INIT_BOOT("delay###\n");
		bPrintFlag =0x01;
		_Short_DelayUS(1000*200);
		PhysicWritePar.PhyBlkNumInBank =1925;
		//PHY_EraseSuperBlk(&PhysicWritePar);
		
		
		//WrtieBlock(&PhysicWritePar,PhysicWritePar.PageNum);
		//ReadBlock(&PhysicWritePar,PhysicWritePar.PageNum);
		//while(1);
		//
	}
	else if (PhysicWritePar.PageNum >=0x02 && PhysicWritePar.PhyBlkNumInBank ==1925)
	{
			INIT_BOOT("finish delay\n");
			//_Short_DelayUS(1000*200);
		//	ReadBlock(&PhysicWritePar,PhysicWritePar.PageNum);
			//while(1);
	}
	PhysicWritePar.MainDataPtr =  tmp2;
	PhysicWritePar.SpareDataPtr = sp;
	 PhysicWritePar.SectorBitmapInPage =bitmap2 ;
	 PhysicWritePar.PageNum =0x04;
	//Str_printf("\n\n", (INT8U *)PhysicWritePar.SpareDataPtr, sizeof(struct NandSpareData));
	//INIT_BOOT("Write:Page %x,Blk:%x,bitmap:%x,\n",PhysicWritePar.PageNum,PhysicWritePar.PhyBlkNumInBank,
	 //    PhysicWritePar.SectorBitmapInPage); 
	 *(INT32U*)(PhysicWritePar.MainDataPtr)=0x11223344;
#endif	
        result =  PHY_PageWrite(&PhysicWritePar);
        if(result != TRUE)
        {   //added by jjf for 如果写入nand失败,则进行坏块管理
            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType *BadBlk = &CUR_LOG_BLK_TABLE[WLogBlkPosition].PhyBlkNum;

            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,LogicRWCtlPar.LogPageNum))
            {
                FTL_ERR("_WritePageMapTbl:bad block manage failed\n");
                return FALSE;
            }

            CUR_LOG_BLK_TABLE[WLogBlkPosition].PhyBlkNum = NewFreeBlk;
            LogicRWCtlPar.PhyLogBlk = CUR_LOG_BLK_TABLE[WLogBlkPosition].PhyBlkNum;
            goto redo;
            
        }
#if  0// (defined(__KERNEL__))

	if(1) //if(bPrintFlag==1)
	{
		INT8U *tmp;
		tmp= PhysicWritePar.MainDataPtr;
		PhysicWritePar.SpareDataPtr = (INT8U *)&TmpSData;
		PhysicWritePar.MainDataPtr=PHY_TESTBUF;
		MEMSET((INT8U *)&TmpSData, 0xff, sizeof(struct NandSpareData));
		PHY_PageRead(&PhysicWritePar);
		if(MEMCMP((INT8U *)&WriteSpareData,(INT8U *)&TmpSData,6))
		{
			INIT_BOOT("Write Spare\n");
			Str_printf("", (INT8U *)&WriteSpareData, sizeof(struct NandSpareData));
			INIT_BOOT("ReadSpare\n");
			Str_printf("", (INT8U *)&TmpSData, sizeof(struct NandSpareData));
			if(PhysicWritePar.SectorBitmapInPage ==0xFFFF)
			{
				Str_printf2("Write",	tmp,32);
				Str_printf2("Read",	PHY_TESTBUF,32);
			}
		}
	}

	
#endif


        /* set log page bitmap */
        LogicRWCtlPar.LogPageBitmap |= PhysicWritePar.SectorBitmapInPage;

        /* check if log page only write right plane */
        if(!(LogicRWCtlPar.LogPageBitmap & FULL_BITMAP_OF_SINGLE_PAGE))
        {
            FTL_ERR("%s %d\n",__func__,__LINE__);
            _CloseLastWritePage();
        }
        /* update page table item */
        _UpdatePageMapTblItem(CurWritePage.LogicPageNum, LogicRWCtlPar.LogPageNum);
    }

    /* set the driver parameters */
    LogicRWCtlPar.SuperPageNum =  CurWritePage.LogicPageNum;

    /* set last buffer page number, for check the sequently write */
    if(Bitmap == 0)
    {
        LogicRWCtlPar.LastWBufPageNum = (INT32U)(BufPageNum - 1);
    }
    else
    {
        LogicRWCtlPar.LastWBufPageNum = BufPageNum;
    }

    /*set cache access count*/
    PMM_CalAccessCount();
    BMM_CalAccessCount();

    return TRUE;
}


#if SUPPORT_READ_RECLAIM
/*
*********************************************************************************************************
*               FLASH TRANSLATE LAYER READ-RECLAIM
*
*Description: prepair the data which has 4bit ecc error occur when read.
*
*Arguments  : none
*
*Return     : Result
*               TRUE    do read-reclaim ok;
*               FALSE   do read-reclaim failed;
*
*Notes      : if 4bit ecc error occur when read data on the logic area,wrtie the data to the block
*             again throuh log block.
*********************************************************************************************************
*/
INT32U  FTL_ReadReclaim(INT32U BufPageNum, INT32U Bitmap, void* Buf)
{
    INT32U result = TRUE;
    INT8S *RecleaimBuf;
    INT32U TempBitmap;

    if (Bitmap != FULL_BITMAP_OF_BUFFER_PAGE)
    {
        INT32S loop = 0;

        FTL_LogicPageWrite(BufPageNum, 0, Buf);

        while( !(Bitmap & (1 << loop)) )
        {
            loop++;
        }

        MEMCPY(LOGIC_WRITE_PAGE_CACHE + 512*loop,Buf,512);
        TempBitmap = (1<<loop);
        RecleaimBuf = LOGIC_WRITE_PAGE_CACHE;
    }
    else
    {
        TempBitmap = Bitmap;
        RecleaimBuf = Buf;
    }

    FTL_LogicPageWrite(BufPageNum, TempBitmap, RecleaimBuf);

    return result;
}
#endif


