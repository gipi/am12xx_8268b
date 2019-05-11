/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : logic_layer.c
* By      : xiongrui
* Version : V0.1
* Date    : 2007-7-19 9:06
*********************************************************************************************************
*/

/*!
 *  \file  bbm.c
 *  \brief bad block manage.
 *  \author  R.x
 *  \version 1.0
 *  \date    2007-7-23
 *
 */
#include "logic.h"
extern INT32U _GetFreeBlk(INT32U * FreeBlkPst, INT32U GetType);
extern INT32U FTL_CalPhyOpPar(struct PhysicOpParameter * PhyOpPar, INT32U ZoneNum, INT32U SupBlkNum, INT32U PageNum);
extern INT32U PHY_NandReset(INT32U ChipNum);
extern INT32U PHY_WaitNandFlashReady(INT32U BankNum);
extern INT32U PHY_PageWrite(struct PhysicOpParameter *NandOpPar);
extern INT32U PHY_SyncNandOperation(INT32U BankNum);
extern INT32U PHY_CopyNandPage(struct PhysicOpParameter *NandOpSrcPar, struct PhysicOpParameter *NandOpDstPar);

/*!
*
* \par  Description:
*       This function write bad flag.
*
* \param  [in]       SuperBlkNum - the physic block number of the bad block in a die
* \param  [in]       ZoneNum - current zone
* \return      none.
*
**/
void  _WriteBadBlkFlag(INT32U ZoneNum, INT32U SuperBlkNum)
{
    struct PhysicOpParameter tmpPhyOpPar;
    INT8U  tmpSpareData[NAND_SPARE_SIZE * 4];
    INT32U tmpPageNum;
    INT32U tmpChipNum;
    INT32U result;

    if((BAD_BLOCK_FLAG_PST = 0x0) || (BAD_BLOCK_FLAG_PST == 0x01))
    {
        /* bad block flag is at the spare area of first page */
        tmpPageNum = 0;
    }
    else
    {
        /* bad block flag is at the spare area of last page */
        tmpPageNum = (PAGE_CNT_PER_PHY_BLK - 1) * (PAGE_CNT_PER_LOGIC_BLK / PAGE_CNT_PER_PHY_BLK);
    }

    /* calculate the physic operation parameter for the bad block */
    result = FTL_CalPhyOpPar(&tmpPhyOpPar, ZoneNum, SuperBlkNum, tmpPageNum);
    if(result != TRUE)
    {
        FTL_ERR("FTL_ERR: calculate physic operation parameter failed when write bad block flag!\n");
        return;
    }
    /* set bad table flag to write to the spare area */
    MEMSET(tmpSpareData, 0xff, NAND_SPARE_SIZE * 4);
    tmpSpareData[0] = 0x00;
    tmpSpareData[NAND_SPARE_SIZE*2] = 0;

    /* set the physic operation parameters */
    tmpPhyOpPar.SectorBitmapInPage = FULL_BITMAP_OF_BUFFER_PAGE;
    tmpPhyOpPar.MainDataPtr = PAGE_COPYBACK_BUFFER;
    tmpPhyOpPar.SpareDataPtr = tmpSpareData;

    /* reset current nand chip */
    tmpChipNum = tmpPhyOpPar.BankNum / BNK_CNT_PER_CHIP;
    PHY_NandReset(tmpChipNum);
    PHY_WaitNandFlashReady(tmpPhyOpPar.BankNum);

    /* write bad block flag by physic function */
    PHY_PageWrite(&tmpPhyOpPar);
    PHY_SyncNandOperation(tmpPhyOpPar.BankNum);

    return;
}

/*!
*
* \par  Description:
*       This function find free block for swap bad block.
*
* \param  [in]       NewFreeBlk - new data blk type item
* \return      sucess or failed.
*
**/
INT32U _GetSwapBlock(struct DataBlkTblType *NewFreeBlk)
{
    INT32U FreePoint;

    if (TRUE != _GetFreeBlk(&FreePoint,LOWEST_EC_TYPE))/*get one free block */
    {
        FTL_ERR("_SwapBadBlock:can not get free block\n");
        return FALSE;
    }

    *NewFreeBlk = FTBL[FreePoint];
    MEMSET(&(FTBL[FreePoint]), 0xff, sizeof(struct DataBlkTblType));/*delete it from free block list*/

    return TRUE;

}

INT32U _RestoreData(struct DataBlkTblType *NewFreeBlk,struct DataBlkTblType *BadBlk, INT32U ErrPage)
{
    INT32U page;
    INT32U ret;
    struct PhysicOpParameter src,dst;

    FTL_DBG("_RestoreData: bad block %d,replacement block %d, ErrPage %u\n",
        BadBlk->PhyBlkNum,NewFreeBlk->PhyBlkNum, ErrPage);

    src.MainDataPtr = dst.MainDataPtr = NULL;
    src.SpareDataPtr = dst.SpareDataPtr = NULL;

    for (page = 0; page < ErrPage; page++)
    {
        FTL_CalPhyOpPar(&src, CURZONE, BadBlk->PhyBlkNum, page); /*set source page addr*/
        FTL_CalPhyOpPar(&dst, CURZONE, NewFreeBlk->PhyBlkNum, page); /*set destination page addr*/
        ret =  PHY_CopyNandPage(&src, &dst);
        ret |= PHY_SyncNandOperation(dst.BankNum);
        if(ret != TRUE)
        {
            FTL_ERR("%s %d error!\n",__func__,__LINE__);
        }
    }

    return TRUE;
}

INT32U  FTL_BadBlkManage(struct DataBlkTblType *NewFreeBlk,struct DataBlkTblType *BadBlk, INT32U ErrPage )
{
    /*get new block*/
    if (TRUE != _GetSwapBlock(NewFreeBlk))
    {
        return FALSE;
   }

    /*restore data of bad block*/
    if (ErrPage && SUPPORT_BAD_BLOCK_DATA_SAVE)
    {
        _RestoreData(NewFreeBlk, BadBlk, ErrPage);
    }

    /*write bad block flag*/
    _WriteBadBlkFlag(CURZONE, BadBlk->PhyBlkNum);

    FTL_DBG("bad block %d,replacement block %d\n",BadBlk->PhyBlkNum,NewFreeBlk->PhyBlkNum);
    return TRUE;
}
