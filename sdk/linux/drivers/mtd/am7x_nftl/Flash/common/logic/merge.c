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
extern INT32U PHY_CopyNandPage(struct PhysicOpParameter *NandOpSrcPar, struct PhysicOpParameter *NandOpDstPar);

extern INT32U _GetFreeBlk(INT32U * FreeBlkPst, INT32U GetType);
extern INT32U FTL_CalPhyOpPar(struct PhysicOpParameter * PhyOpPar, INT32U ZoneNum, INT32U SupBlkNum, INT32U PageNum);
extern INT32U FTL_BadBlkManage(struct DataBlkTblType * NewFreeBlk, struct DataBlkTblType * BadBlk, INT32U ErrPage);
extern INT32U volatile PhyBlkEraseCnt;
extern INT16U volatile LastFreeBlkPst;


#if (SUPPORT_ALIGN_BANK)
#define RATE_MOVE 1/(BANK_TOTAl + 1)
#else
#define RATE_MOVE  1/2
#endif

//@fish add 2010-04-28 for Erase Count 
extern struct  _Debug_Wear  WearDbg;
#if(defined(__KERNEL__))  && (SUPER_WEAR_LEVELLING ==1)  
//@fish add 
extern struct ReClamBlkInfo    Reclaim_Blk;
 INT16U OverECCBlk[2];
INT32U wEraseCntTmp;
INT32U  _Search_ReclaimBlk(struct DataBlkECCInfo ReclaimBlk, INT32U *LogBlkPst)
{
    INT32S iLoop;

    for(iLoop=0; iLoop<ECC_BLK_NUM; iLoop++)
    {
        if( (Reclaim_Blk.BlkInfo[iLoop].PhyBlkNum== ReclaimBlk.PhyBlkNum )  &&
                      (Reclaim_Blk.BlkInfo[iLoop].ZoneNum== ReclaimBlk.ZoneNum) )        
        {
            if(LogBlkPst != NULL)
            {
                *LogBlkPst = iLoop;
            }
            return TRUE;
        }
    }

    return FALSE;
}
INT32U  _Get_ReclaimBlkPonit(void)
{

    INT32S loop;

    for (loop = 0; loop < ECC_BLK_NUM; loop++)
    {     
        if (Reclaim_Blk.BlkInfo[loop].PhyBlkNum == Phy_BLK_DEF)     /*this point is empty*/
        {
                Reclaim_Blk.FreeIndexPst=loop;
                return loop;
        }
    }
    return 0xFFF;
}
void Init_ReClaimBlk_Info(void)
{
        INT32S loop;
        for (loop = 0; loop < ECC_BLK_NUM; loop++)
        {
                Reclaim_Blk.BlkInfo[loop].PhyBlkNum = Phy_BLK_DEF;
                Reclaim_Blk.BlkInfo[loop].ZoneNum =0xFF;
                Reclaim_Blk.BlkInfo[loop].ECC_ErrCnt =0x00;
        }
}
INT32U Set_ReClaimBlk_Info(INT16U PhyBlk,INT16U PhyZone)
{
      struct DataBlkECCInfo   TmpBlkInfo;
       INT32U TmpPst;
       INT32U result,result2;
       INT16U FreePoint; 
       //init value 
      TmpBlkInfo.PhyBlkNum = PhyBlk;
      TmpBlkInfo.ZoneNum= PhyZone;
      result2 = FALSE;
      //search reclaim Block
       result=_Search_ReclaimBlk(TmpBlkInfo,&TmpPst);
       if(result != TRUE)
       {
                FreePoint = _Get_ReclaimBlkPonit();          
                if(FreePoint !=0xFFF)
                {
                         TmpBlkInfo.ECC_ErrCnt = 1;
                         TmpBlkInfo.ECC_ErrNum =1;
                         TmpBlkInfo.ECC_ErrNum+=PHY_BCH_CNT(BCH_GET_VAL,0);
                         Reclaim_Blk.BlkInfo[FreePoint]=TmpBlkInfo;
                     //    INIT_BOOT("\n<%s>_<%d> ",__func__,__LINE__);                  
                     //    INIT_BOOT("Index:%x,Blk:%x,Zone:%x,%x\n",FreePoint, Reclaim_Blk.BlkInfo[FreePoint].PhyBlkNum, Reclaim_Blk.BlkInfo[FreePoint].ZoneNum,
                    //        Reclaim_Blk.BlkInfo[FreePoint].ECC_ErrCnt);
                }                
       }
       else
       {
              
                Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrCnt +=1;
                Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrNum  += PHY_BCH_CNT(BCH_GET_VAL,0);     
             //   INIT_BOOT("\n YYY<%s>_<%d>_",__func__,__LINE__);
          //      INIT_BOOT("Idx:%x,blk:%x,Z0:%x,%x,%x\n",TmpPst, Reclaim_Blk.BlkInfo[TmpPst].PhyBlkNum, Reclaim_Blk.BlkInfo[TmpPst].ZoneNum,
                   //             Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrCnt,Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrNum);
                if((Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrCnt>=ECC_ERR_NUM)
                       || (Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrNum>=BCH_ERR_TOTALBIT))
                {
                         INIT_BOOT("\n<%s>_<%d>_",__func__,__LINE__);
                         INIT_BOOT("Idx:%x,blk:%x,Z0:%x,%x,%x\n",TmpPst, Reclaim_Blk.BlkInfo[TmpPst].PhyBlkNum, Reclaim_Blk.BlkInfo[TmpPst].ZoneNum,
                                Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrCnt,Reclaim_Blk.BlkInfo[TmpPst].ECC_ErrNum);
                        result2 = TRUE;      
                        Reclaim_Blk.BlkInfo[TmpPst].PhyBlkNum =Phy_BLK_DEF;
                        Reclaim_Blk.BlkInfo[TmpPst].ZoneNum = Phy_Zone_DEF;
                }
       }
       return result2;
       
}
INT8U _Get_FreeBlkInfo(void)
{
      INT32S loop,jLoop;
      INIT_BOOT("\nLastFreeBlkPst:0x%x\n",LastFreeBlkPst);
      jLoop =0x00;
     for (loop = 0; loop < FREEBLKS_PER_ZONE; loop++)
    {
        if (FTBL[loop].PhyBlkNum != 0xfff)     /*this point is empty*/
        {
               INIT_BOOT("No:%2x,%3x,%x, ",loop,FTBL[loop].PhyBlkNum,FTBL[loop].EraseCnt);
               if(0x00==(((jLoop++)+1)%5))
                     printf("\n");
        }
    }
    INIT_BOOT("\n");
    return 0x00;
}

INT8U _Clear_FreeBlk_Erase(struct DataBlkTblType BlkNum)
{
    INT32S loop;
    for (loop = 0; loop < FREEBLKS_PER_ZONE; loop++)
    {
        if (FTBL[loop].PhyBlkNum != 0xfff  &&FTBL[loop].EraseCnt ==MAX_ERASE_CNT )     /*this point is empty*/
        {
                if(FTBL[loop].PhyBlkNum!=BlkNum.PhyBlkNum)
                       FTBL[loop].EraseCnt-=2;
        }
    }  
    return 0x00;  
}
INT32U _Get_FreeBlk_EraseCnt(void)
{
    INT32U bEraseCnt;
    INT32S loop;
    bEraseCnt  = 0x00;
    for (loop = 0; loop < FREEBLKS_PER_ZONE; loop++)
    {
            if (FTBL[loop].PhyBlkNum != 0xfff  &&FTBL[loop].EraseCnt >bEraseCnt )     /*this point is empty*/
            {
                  bEraseCnt = FTBL[loop].EraseCnt;                         
            }
    }  
    return bEraseCnt;  
}
#endif

/*!
*
* \par  Description:
*       This function get one empty poisition from free block space.
*
* \param  [in]       void
* \return      the poisition within free block space, 0xff means failed.
*
**/
INT8U _GetEmptyFromFreeSpace(void)
{
    INT32S loop;

    for (loop = 0; loop < FREEBLKS_PER_ZONE; loop++)
    {
        if (FTBL[loop].PhyBlkNum == 0xfff)     /*this point is empty*/
        {
            return loop;
        }
    }

    return 0xff;
}

/*!
*
* \par  Description:
*       This function copy valuable data from data to log,then change data to free ,change log to data.
*
* \param  [in]       LogNum,serial number within log block space
* \return      sucess or failed.
* \note         this function was called when log block is in order,that is to say physical
*             page number is same with logical page number.
**/
INT32U _SwapDataAndLogMerge(INT8U LogNum)
{
    INT32S FreePoint;
    INT32S LastUsedPage,superpage;
    struct PhysicOpParameter src,dst;
    struct DataBlkTblType LogBlk,DataBlk;
    WearDbg.SwapMergeCnt +=1;

    LogBlk = LTBL[LogNum].PhyBlkNum;
    DataBlk = DTBL[LTBL[LogNum].LogicBlkNum];

    LastUsedPage =LTBL[LogNum].LastUsedPage;
    src.MainDataPtr = dst.MainDataPtr = NULL;
    src.SpareDataPtr = dst.SpareDataPtr = NULL;
redo:
    for (superpage = LastUsedPage + 1; superpage <PAGES_PER_SUPERBLK; superpage++)
    {
        FTL_CalPhyOpPar(&src, CURZONE, DataBlk.PhyBlkNum, superpage);  /*set source page addr*/
        FTL_CalPhyOpPar(&dst, CURZONE, LogBlk.PhyBlkNum, superpage);   /*set destination page addr*/
        if (TRUE != PHY_CopyNandPage(&src,&dst)) /*copy data from data block to log block*/
        {
            FTL_ERR("_SwapDataAndLogMerge:copy back error\n");
            return FALSE;
        }
        if (TRUE != PHY_SyncNandOperation(dst.BankNum))
        {
            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType *BadBlk = &LogBlk;

            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,LastUsedPage + 1))
            {
                FTL_ERR("_SwapDataAndLogMerge:1:bad block manage failed\n");
                return FALSE;
            }

            LogBlk = NewFreeBlk;
            goto redo;
        }
    }

    DTBL[LTBL[LogNum].LogicBlkNum] = LogBlk;
    MEMSET(&(LTBL[LogNum]),0xff,sizeof (struct LogBlkTblType));/*delete log block from log block space*/

    FTL_CalPhyOpPar(&src, CURZONE, DataBlk.PhyBlkNum, 0);  /*set source page addr*/
    if (TRUE != PHY_EraseSuperBlk(&src)) /*erase data block*/
    {
        struct DataBlkTblType NewFreeBlk;
        struct DataBlkTblType *BadBlk = &DataBlk;

        FTL_ERR("_SwapDataAndLogMerge:bad block %d made\n",BadBlk->PhyBlkNum);
        if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
        {
            FTL_ERR("_SwapDataAndLogMerge:2:bad block manage failed\n");
            return FALSE;
        }

        DataBlk = NewFreeBlk;
        DataBlk.EraseCnt -= 1;
    }

    if (DataBlk.EraseCnt < MAX_ERASE_CNT)  /*Don't care when erase count exceed of the limit(7)*/
    {
        DataBlk.EraseCnt++;
    }
    PhyBlkEraseCnt++;
    
#if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)       
    //@fish add for reclaim ECC 
    if(PHY_READ_ECC_TO_LIMIT(PHY_ReadRes(BCH_GET_VAL,0)))
    {            
    #if (_NEW_ZONE_TBL_ ==0x01)    
            wEraseCntTmp = _Get_FreeBlk_EraseCnt();
            if((wEraseCntTmp+ADD_ERASE_CNT) <MAX_ERASE_CNT)
                    DataBlk.EraseCnt =  wEraseCntTmp+ADD_ERASE_CNT;   
            else
                    DataBlk.EraseCnt = MAX_ERASE_CNT;
    #else
            DataBlk.EraseCnt = MAX_ERASE_CNT;
            _Clear_FreeBlk_Erase(DataBlk);
    #endif
     
           PhyBlkEraseCnt = WEAR_LEVELLING_FREQUENCE;      
           
           PHY_ReadRes(BCH_CLEAR_VAL,0);           
         ///  printf("\n\nrun_to_,<%s>_<%d>_blk:%x\n\n",__func__,__LINE__,DataBlk.PhyBlkNum);
    }
 #endif   
 
    FreePoint = _GetEmptyFromFreeSpace(); /*find empty to insert new free block  */
    if (FreePoint == 0xff)
    {
        FTL_ERR("_SwapDataAndLogMerge:no free space\n");
        return FALSE;
    }
    FTBL[FreePoint] = DataBlk;

    /*clear page map table cache*/
    PMT->ZoneNum = 0xff;
    PMT->LogicBlkNum = 0xffff;
    PMT->TblNeedUpdate = 0;

    return TRUE;
}

/*!
*
* \par  Description:
*       This function write manage info in page 0.
*
* \param  [in]    SrcBlk,the source block keeps old info
* \param  [in]    DstBlk,the destination block keeps new info
* \param  [in]    PhySrcPage,the source page is PhySrcBlk keeps data of logic page 0
* \param  [in]    seqplus,seq need added value

* \return   sucess or failed.
*
**/
INT32U _ProcessPage0(struct DataBlkTblType *SrcBlk,struct DataBlkTblType *DstBlk,INT32U PhySrcPage,INT8U seqplus)
{
    INT32S loop;
    INT32S seq,LogicalInfo;
    struct PhysicOpParameter src,dst;
    struct NandSpareData oob;

    src.MainDataPtr = dst.MainDataPtr = TMPBUF;
    src.SpareDataPtr = dst.SpareDataPtr = &oob;
    MEMSET(&oob,0xff,sizeof(struct NandSpareData));

    /*read fore 2 sectors of page 0 from log block , get seq value and LogicalInfo*/

/// src.SectorBitmapInPage = 0x3;
//  src.SectorBitmapInPage=SECTOR_BITMAP_SPAREDATA;
    src.SectorBitmapInPage=SECTOR_BITMAP_SPAREDATA;
    FTL_CalPhyOpPar(&src, CURZONE, SrcBlk->PhyBlkNum, 0);
    PHY_PageRead(&src);
//    INIT_BOOT("@@@run to :%s $$$ ,%d,%d\n",__func__,__LINE__);	
///  Str_printf(" ",(INT8U *)&oob,sizeof(struct NandSpareData));
     seq = oob.UserData[0].SpareData1.LogStatus.LogBlkAge;
    LogicalInfo = oob.UserData[0].SpareData0.LogicBlkInfo;

redo:
    FTL_CalPhyOpPar(&src, CURZONE, SrcBlk->PhyBlkNum, PhySrcPage);
    FTL_CalPhyOpPar(&dst, CURZONE, DstBlk->PhyBlkNum, 0);

    /*if buffer size is smaller than superpage size , we should operate more times for one superpage*/
    for (loop = 0; loop < OPCYCLE; loop++)
    {
        src.SectorBitmapInPage = ((1ULL << PAGE_BUFFER_SECT_NUM) - 1) << (loop * SECTOR_NUM_PER_SINGLE_PAGE);
        dst.SectorBitmapInPage = src.SectorBitmapInPage; /* set page bitmap*/
        PHY_PageRead(&src);

         /*only used oob area of fore 2 sectors ,and  page 0  keeps seq and logicalinfo*/
 
        oob.UserData[0].SpareData0.LogicBlkInfo = LogicalInfo;
        oob.UserData[0].SpareData1.LogStatus.LogBlkAge = seq + seqplus;
		
	//INIT_BOOT("@@@run to :%s $$$ ,%d,%d ,Blk:%x\n",__func__,seq,seqplus,DstBlk->PhyBlkNum);	
	//Str_printf(" ",(INT8U *)&oob,sizeof(struct NandSpareData));

        PHY_PageWrite(&dst);
        if (TRUE != PHY_SyncNandOperation(dst.BankNum))
        {
            struct DataBlkTblType NewFreeBlk;
            struct DataBlkTblType *BadBlk = DstBlk;

            if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
            {
                FTL_ERR("_ProcessPage0:bad block manage failed\n");
                return FALSE;
            }

            *DstBlk = NewFreeBlk;
            goto redo;
        }
    }/*end for (loop = 0..........)*/

    return TRUE;
}


/*!
*
* \par  Description:
*       This function copy data within same bank.
*
* \param  [in]    SrcBlk,the source block keeps old info
* \param  [in]    DstBlk,the destination block keeps new info
* \param  [in]    LogNUm,serial number within log block space

* \return   sucess or failed.
*
**/
INT32U _MoveMergeAlignBank(struct DataBlkTblType *SrcBlk,struct DataBlkTblType *DstBlk,INT8U LogNum)
{
    INT32S PhysicSrcPage,PhysicDstPage,LogicPage,LastUsedPage;
    INT8U bank;
    struct PhysicOpParameter src,dst;

    src.MainDataPtr = dst.MainDataPtr = NULL;
    src.SpareDataPtr = dst.SpareDataPtr = NULL;
    LastUsedPage = 0;
redo:
    /*copy valuable data from log block to free block*/
    for (bank = 0; bank < BANK_TOTAl; bank++)
    {
        PhysicDstPage = bank;
        for (LogicPage = bank; LogicPage < PAGES_PER_SUPERBLK; LogicPage += BANK_TOTAl)
        {
            PhysicSrcPage = L2P[LogicPage].PhyPageNum;
            if (PhysicSrcPage != 0xfff)
            {
                if (0 == PhysicDstPage)/*write manage info in spare area of page 0*/
                {
                    if (TRUE != _ProcessPage0(SrcBlk, DstBlk,PhysicSrcPage,0))
                    {
                        FTL_ERR("_MoveMergeAlignBank:process page 0 err\n");
                        return FALSE;
                    }
                }
                else/*copy back in same bank*/
                {
            #if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)   
                     PHY_ReadRes(BCH_CLEAR_VAL,0);
                     PHY_BCH_CNT(BCH_CLEAR_VAL,0);
                     PHY_Get_BCHRes();
            #endif
            
                    FTL_CalPhyOpPar(&src, CURZONE, SrcBlk->PhyBlkNum, PhysicSrcPage); /*set source page addr*/
                    FTL_CalPhyOpPar(&dst, CURZONE, DstBlk->PhyBlkNum, PhysicDstPage); /*set destination page addr*/
                    if (TRUE != PHY_CopyNandPage(&src, &dst))
                    {
                        FTL_ERR("_MoveMergeAlignBank:copy back err\n");
                        return FALSE;
                    }
             #if(defined(__KERNEL__)) && (SUPER_WEAR_LEVELLING ==1)          
                    if(PHY_READ_ECC_TO_LIMIT(PHY_ReadRes(BCH_GET_VAL,0))  
                               ||(NAND_DATA_ECC_ERROR==PHY_ReadRes(BCH_GET_VAL,0))   )
                    {   
                         INIT_BOOT("\nReadRes_<%s>_<%d>__blk:%x\n",__func__,__LINE__,SrcBlk->PhyBlkNum);
                         OverECCBlk[0x00] = SrcBlk->PhyBlkNum;
                         PHY_ReadRes(BCH_CLEAR_VAL,0);                 
                    }         
                    else if (NAND_DATA_ECC_LIMIT2 ==PHY_Get_BCHRes() &&
                                 (TRUE==Set_ReClaimBlk_Info(SrcBlk->PhyBlkNum,CURZONE)))
                    {
                    
                            INIT_BOOT("\nBCHRes_<%s>_<%d>__blk:%x\n",__func__,__LINE__,SrcBlk->PhyBlkNum);
                            OverECCBlk[0x00] = SrcBlk->PhyBlkNum;                    
                    }
              #endif   
                    if (TRUE != PHY_SyncNandOperation(dst.BankNum))
                    {
                        struct DataBlkTblType NewFreeBlk;
                        struct DataBlkTblType *BadBlk = DstBlk;

                        if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
                        {
                            FTL_ERR("_MoveMergeAlignBank:bad block manage failed\n");
                            return FALSE;
                        }
                        *DstBlk = NewFreeBlk;
                        goto redo;
                    }
                }

                L2P[LogicPage].PhyPageNum = PhysicDstPage; /*set page map relation*/
                PhysicDstPage += BANK_TOTAl;
            }
        }

        if ( (0 == bank) && ( 0 == PhysicDstPage))/*no valuable page in bank 0*/
        {
            if (TRUE != _ProcessPage0(SrcBlk, DstBlk,0,0))/*copy block info from page 0*/
            {
                FTL_ERR("_MoveMergeAlignBank:process page 0 err\n");
                return FALSE;
            }
        }

        if ( (PhysicDstPage - BANK_TOTAl) > LastUsedPage)
        {
            LastUsedPage = PhysicDstPage - BANK_TOTAl;
        }

    }

    LTBL[LogNum].LastUsedPage = LastUsedPage;

    return TRUE;
}


/*!
*
* \par  Description:
*       This function move valuable data from log block to free block,then replace them.
*
* \param  [in]       LogNum,serial number within log block space
* \return      sucess or failed.
* \note         this function was called when log block is full, and valid pages is less than half of one block.
**/
INT32U _MoveDataToLogMerge(INT8U LogNum)
{
    INT32S FreePoint;
    struct PhysicOpParameter src;
    struct DataBlkTblType LogBlk,FreeBlk;

    INT32S ret = TRUE;
    WearDbg.MoveMergeCnt+=1;

#if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)      
    
    OverECCBlk[0] = OverECCBlk[1] =0xFFFF;

#endif

    src.MainDataPtr = NULL;
    src.SpareDataPtr = NULL;

    LogBlk = LTBL[LogNum].PhyBlkNum;
    if (TRUE != _GetFreeBlk(&FreePoint,LOWEST_EC_TYPE))/*get one free block */
    {
        FTL_ERR("_MoveDataToLogMerge:can not get free block\n");
        return FALSE;
    }
    FreeBlk = FTBL[FreePoint];
    ret = _MoveMergeAlignBank(&LogBlk, &FreeBlk, LogNum);
    if (TRUE != ret)
    {
        FTL_ERR("_MoveDataToLogMerge:copy data err\n");
        return FALSE;
    }

    LTBL[LogNum].PhyBlkNum = FreeBlk;

    /*erase log block*/
    FTL_CalPhyOpPar(&src, CURZONE, LogBlk.PhyBlkNum, 0);
    if (TRUE != PHY_EraseSuperBlk(&src))
    {
        struct DataBlkTblType NewFreeBlk;
        struct DataBlkTblType *BadBlk = &LogBlk;

        FTL_ERR("_MoveDataToLogMerge:bad block %d made\n",BadBlk->PhyBlkNum);
        if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
        {
            FTL_ERR("_MoveDataToLogMerge:bad block manage failed\n");
            return FALSE;
        }

        LogBlk = NewFreeBlk;
        LogBlk.EraseCnt -= 1;
    }

    if (LogBlk.EraseCnt < MAX_ERASE_CNT)
    {
        LogBlk.EraseCnt++;
    }
    PhyBlkEraseCnt++;
 #if(defined(__KERNEL__))   &&(SUPER_WEAR_LEVELLING ==1)   
    //index 0-->data block index0 Logblock
    if(OverECCBlk[0] ==LogBlk.PhyBlkNum)
    {
        //   LogBlk.EraseCnt += ADD_ERASE_CNT;
      #if (_NEW_ZONE_TBL_ ==0x01)   
              wEraseCntTmp = _Get_FreeBlk_EraseCnt();
             if((wEraseCntTmp+ADD_ERASE_CNT) <MAX_ERASE_CNT)
                     LogBlk.EraseCnt =  wEraseCntTmp+ADD_ERASE_CNT;   
             else
                     LogBlk.EraseCnt = MAX_ERASE_CNT;
     #else
             LogBlk.EraseCnt = MAX_ERASE_CNT;
             _Clear_FreeBlk_Erase(LogBlk);
     #endif
           PhyBlkEraseCnt = WEAR_LEVELLING_FREQUENCE;      
           INIT_BOOT("\nLogBlk_<%s>_<%d>_blk:%x,%d\n",__func__,__LINE__,LogBlk.PhyBlkNum,FreePoint);
    }
 #endif  
    FTBL[FreePoint] = LogBlk;


    return TRUE;
}




/*!
*
* \par  Description:
*       This function copy valuable data from log block or dat block to free block, change free to data ,change
*       data and log to free.
*
* \param  [in]       LogNum,serial number within log block space
* \return      sucess or failed.
* \note         this function was called when log block is not suit for swap or move.
**/
INT32U _SimpleMergeDataAndLog(INT8U LogNum)
{
    INT32S PhysicSrcPage,page,OriginPage;
    INT32S FreePoint;
    INT32S indata;
    struct PhysicOpParameter src,dst;
    struct DataBlkTblType LogBlk,FreeBlk,DataBlk,SrcBlk;
#if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)      
   
    OverECCBlk[0] = OverECCBlk[1] =0xFFFF;
    
    WearDbg.SimpleMergeCnt+=1;


#endif

    LogBlk = LTBL[LogNum].PhyBlkNum;
    if (TRUE != _GetFreeBlk(&FreePoint,LOWEST_EC_TYPE))/*get one free block */
    {
        FTL_ERR("_SimpleMergeDataAndLog:can not get free block\n");
        return FALSE;
    }
    FreeBlk = FTBL[FreePoint];
    DataBlk = DTBL[LTBL[LogNum].LogicBlkNum];
redo:
    /*copy valuable data from data block or log block to free block*/
    for (page = 0; page < PAGES_PER_SUPERBLK; page++)
    {
        PhysicSrcPage = L2P[page].PhyPageNum;
        indata = ((PhysicSrcPage == 0xfff)?1:0);
        SrcBlk = (indata?DataBlk:LogBlk);
        OriginPage = (indata?page:PhysicSrcPage);

        if (0 == page) /*write some info in oob area of page 0,seq must increase */
        {
            INT8U seqplus = (indata?2:1);
            if (TRUE != _ProcessPage0(&SrcBlk, &FreeBlk, OriginPage, seqplus))
            {
                FTL_ERR("_SimpleMergeDataAndLog:copy page 0 err\n");
                return FALSE;
            }
        }
        else /*just use copyback operation*/
        {
#if (defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)   
             PHY_ReadRes(BCH_CLEAR_VAL,0);
             PHY_BCH_CNT(BCH_CLEAR_VAL,0);
             PHY_Get_BCHRes();
#endif


            FTL_CalPhyOpPar(&src, CURZONE, SrcBlk.PhyBlkNum,OriginPage);
            FTL_CalPhyOpPar(&dst, CURZONE, FreeBlk.PhyBlkNum,page);
            
            if (TRUE != PHY_CopyNandPage(&src, &dst))
            {
                FTL_ERR("_SimpleMergeDataAndLog:copy back err\n");
                return FALSE;
            }
            
      #if(defined(__KERNEL__)) && (SUPER_WEAR_LEVELLING ==1)          
            if(PHY_READ_ECC_TO_LIMIT(PHY_ReadRes(BCH_GET_VAL,0))  
                       ||(NAND_DATA_ECC_ERROR==PHY_ReadRes(BCH_GET_VAL,0))   )
            {   
                 INIT_BOOT("\nReadRes_<%s>_<%d>_%d_blk:%x,%x\n",__func__,__LINE__,indata,SrcBlk.PhyBlkNum,PHY_ReadRes(BCH_GET_VAL,0));
                 OverECCBlk[indata] = SrcBlk.PhyBlkNum;
                 PHY_ReadRes(BCH_CLEAR_VAL,0);                 
            }         
            else if (NAND_DATA_ECC_LIMIT2 ==PHY_Get_BCHRes() &&
                         (TRUE==Set_ReClaimBlk_Info(SrcBlk.PhyBlkNum,CURZONE)))
            {
            
                    INIT_BOOT("\nBCHRes_<%s>_<%d>_%d_blk:%x\n",__func__,__LINE__,indata,SrcBlk.PhyBlkNum);
                    OverECCBlk[indata] = SrcBlk.PhyBlkNum;                    
            }
      #endif      
            if (TRUE != PHY_SyncNandOperation(dst.BankNum))
            {
                struct DataBlkTblType NewFreeBlk;
                struct DataBlkTblType *BadBlk = &FreeBlk;

                if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
                {
                    FTL_ERR("_SimpleMergeDataAndLog:1:bad block manage failed\n");
                    return FALSE;
                }

                FreeBlk = NewFreeBlk;
                goto redo;
            }
        }
    }/*end for (page=0 .....)*/

    
    DTBL[LTBL[LogNum].LogicBlkNum] = FreeBlk; /*change free to data*/

    /*erase data block,change data to free*/
    FTL_CalPhyOpPar(&src, CURZONE, DataBlk.PhyBlkNum,0);
    if (TRUE != PHY_EraseSuperBlk(&src))
    {
        struct DataBlkTblType NewFreeBlk;
        struct DataBlkTblType *BadBlk = &DataBlk;

        FTL_ERR("_SimpleMergeDataAndLog:bad block %d made\n",BadBlk->PhyBlkNum);
        if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
        {
            FTL_ERR("_SimpleMergeDataAndLog:2:bad block manage failed\n");
            return FALSE;
        }

        DataBlk = NewFreeBlk;
        DataBlk.EraseCnt -= 1;
    }

    if (DataBlk.EraseCnt < MAX_ERASE_CNT)
    {
        DataBlk.EraseCnt++;
    }
    PhyBlkEraseCnt++;
    
#if(defined(__KERNEL__))   &&(SUPER_WEAR_LEVELLING ==1)   
    //index 1-->data block index0 Logblock
    if(OverECCBlk[1] ==DataBlk.PhyBlkNum)
    {
   #if (_NEW_ZONE_TBL_ ==0x01)
             wEraseCntTmp = _Get_FreeBlk_EraseCnt();
             if((wEraseCntTmp+ADD_ERASE_CNT) <MAX_ERASE_CNT)
                     DataBlk.EraseCnt =  wEraseCntTmp+ADD_ERASE_CNT;   
             else
                     DataBlk.EraseCnt = MAX_ERASE_CNT;
     #else
             DataBlk.EraseCnt = MAX_ERASE_CNT;
             _Clear_FreeBlk_Erase(DataBlk);
     #endif
           PhyBlkEraseCnt = WEAR_LEVELLING_FREQUENCE;      
           
           INIT_BOOT("\nDataBlk_<%s>_<%d>_blk:%x,%d\n",__func__,__LINE__,DataBlk.PhyBlkNum,FreePoint);
    }
 #endif   

 
    FTBL[FreePoint] = DataBlk;
    /*end*/

    /*erase log block,change log to free*/
    FTL_CalPhyOpPar(&src, CURZONE, LogBlk.PhyBlkNum,0);
    if (TRUE != PHY_EraseSuperBlk(&src))
    {
        struct DataBlkTblType NewFreeBlk;
        struct DataBlkTblType *BadBlk = &LogBlk;

        FTL_ERR("_SimpleMergeDataAndLog:bad block %d made\n",BadBlk->PhyBlkNum);
        if (TRUE != FTL_BadBlkManage(&NewFreeBlk,BadBlk,0))
        {
            FTL_ERR("_SimpleMergeDataAndLog:3:bad block manage failed\n");
            return FALSE;
        }

        LogBlk = NewFreeBlk;
        LogBlk.EraseCnt -= 1;
    }
    MEMSET(&(LTBL[LogNum]),0xff,sizeof (struct LogBlkTblType));/*delete log block from log block space*/
    FreePoint = _GetEmptyFromFreeSpace(); /*find empty to insert new free block  */
    if (FreePoint == 0xff)
    {
        FTL_ERR("_SimpleMergeDataAndLog:get free space failed\n");
        return FALSE;
    }

    if (LogBlk.EraseCnt < MAX_ERASE_CNT)
    {
        LogBlk.EraseCnt++;
    }
    PhyBlkEraseCnt++;

    
#if(defined(__KERNEL__)) && (SUPER_WEAR_LEVELLING ==1)        
   if(OverECCBlk[0] ==LogBlk.PhyBlkNum)
    {              
    #if (_NEW_ZONE_TBL_ ==0x01)   
             wEraseCntTmp = _Get_FreeBlk_EraseCnt();
             if((wEraseCntTmp+ADD_ERASE_CNT) <MAX_ERASE_CNT)
                     LogBlk.EraseCnt =  wEraseCntTmp+ADD_ERASE_CNT;   
             else
                     LogBlk.EraseCnt = MAX_ERASE_CNT;
     #else
               LogBlk.EraseCnt = MAX_ERASE_CNT;
               _Clear_FreeBlk_Erase(LogBlk);
     #endif
     
           PhyBlkEraseCnt = WEAR_LEVELLING_FREQUENCE;      
             INIT_BOOT("\nLogBlk_<%s>_<%d>_blk:%x,%x,%x\n",__func__,__LINE__,LogBlk.PhyBlkNum,LogBlk.EraseCnt,FreePoint);
       }
#endif  
    FTBL[FreePoint] = LogBlk;
    /*end*/
#if(defined(__KERNEL__))   && (SUPER_WEAR_LEVELLING ==1)  
    if(OverECCBlk[0] !=0xFFFF || OverECCBlk[1] !=0xFFFF)
    {
            INIT_BOOT("%x,%x,2:%x,%x\n",OverECCBlk[0],LogBlk.PhyBlkNum,OverECCBlk[1],DataBlk.PhyBlkNum);
            _Get_FreeBlkInfo();
    }
#endif
    /*clear page map table cache*/
    PMT->ZoneNum = 0xff;
    PMT->LogicBlkNum = 0xffff;
    PMT->TblNeedUpdate = 0;

    return TRUE;
}

/*!
*
* \par  Description:
*       This function make free space for write,free block or free page
*
* \para  [in]       type
* \retval        1 - current log block is full,SPECIAL_MERGE
* \retval        0 - no enough free block,NORMAL_MERGE
* \return      sucess or failed.
* \note         this function was called when log block is full or there is no enough free block. Move_Merge only make free
*             page in current log block , no free block created.
**/
INT32U FTL_MergeLogBlk(INT8U type)
{
    INT32S loop;

    INT8U inorder = 1;
    INT8U LogNum = PMT->LogicBlkNum;
    INT16U nValidPage = 0;

    for (loop = 0; loop < PAGES_PER_SUPERBLK; loop++)
    {
        /*calculate valid page number*/
        if ( L2P[loop].PhyPageNum != 0xfff)
        {
            nValidPage ++;
        }
        /*logical page number not same with physical page number , not in order*/
        if ( L2P[loop].PhyPageNum != loop)
        {
            inorder = 0;
        }
    }

    /*valid page number less than used page number ,not in order*/
    if (nValidPage < (LTBL[LogNum].LastUsedPage + 1))
    {
        inorder = 0;
    }

    /*continusly write in order ,suit for swap merge*/
    if (inorder)
    {
         return (_SwapDataAndLogMerge(LogNum));
    }
    else if ( (type == SPECIAL_MERGE) && (nValidPage < PAGES_PER_SUPERBLK * RATE_MOVE) )
    {
        /* merge current full log block , if valid page number less than half of block ,suit for move merge*/
        return (_MoveDataToLogMerge(LogNum));
    }
    else
    {
        /*completely merge , less efficiency*/
        return (_SimpleMergeDataAndLog(LogNum));
    }
}


