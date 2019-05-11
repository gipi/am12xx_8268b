/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : scan_nand.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-7-27 16:21
*********************************************************************************************************
*/

#include  "nand_flash_driver_type.h"

extern INT32U  _ReadNandStatus(INT32U ReadStatusCmd, INT32U BankInChip);
extern INT32U _NandReleaseDevice(void);
extern INT32U _NandGetDevice(INT32U ChipNum);
extern INT32U  PHY_NandReset(INT32U ChipNum);
extern INT32U  PHY_ReadNandId(INT32U ChipNum, void *NandChipID);
extern void  SettingNandFreq(INT32U NandFreq, INT32U CoreClock);
extern void Set_NAND_CTL_Access_Mode(INT8U *nand_id);


#define NAND_MFR_TOSHIBA    0x98
#define NAND_MFR_SAMSUNG    0xec
#define NAND_MFR_HYNIX      0xad
#define NAND_MFR_MICRON     0x2c
#define NAND_MFR_ST         0x20
#define NAND_MFR_INFINEON   0xc1
#define NAND_MFR_INTEL      0x89
#define NAND_MFR_RENESSAS   0x07
#define NAND_MFR_PowerFlash   0x92
#define NAND_MFR_MIRA		0xc8
#define NAND_MFR_SPANSION    0x01
#define NAND_MFR_MXIC		0xc2
#define NAND_MFR_WinBond	0xef
#define NAND_MFR_MakerFounder	0x9B


extern struct NandDevPar   SamsungNandTbl[];
extern struct NandDevPar   HynixNandTbl[];
extern struct NandDevPar   ToshibaNandTbl[];
extern struct NandDevPar   MicronNandTbl[];
extern struct NandDevPar   IntelNandTbl[];
extern struct NandDevPar   StNandTbl[];
extern struct NandDevPar   RenessasNandTbl[];
extern struct NandDevPar   UserCreateNandTbl[];
extern struct NandDevPar   PowerFlashNandTbl[];
extern struct NandDevPar   MiraFlashNandTbl[];
extern struct NandDevPar   SpansionFlashNandTbl[];
extern struct NandDevPar   MxicFlashNandTbl[];
extern struct NandDevPar   WinBondFlashNandTbl[];
extern struct NandDevPar   MakerFounderFlashNandTbl[];

extern struct SpecialCmdType SpecialType_Samsung_SLC_512;
extern struct SpecialCmdType SpecialType_Samsung_SLC_2K;
extern struct SpecialCmdType SpecialType_Samsung_SLC_4K;
extern struct SpecialCmdType SpecialType_Samsung_MLC_2K;
extern struct SpecialCmdType SpecialType_Samsung_MLC_4K;
extern struct SpecialCmdType SpecialType_Micron;
extern struct SpecialCmdType SpecialType_Toshiba_SLC_2K;
extern struct SpecialCmdType SpecialType_Toshiba_MLC_2K_1024;

/*
*********************************************************************************************************
*               SEARCH NAND FLASH ID FROM TABLE
*
*Description: search nand flash id from nand id table.
*
*Arguments  : NandPar   the pointer to the nand parameters;
*             NandId    the pointer to the and id;
*             NandIdTbl the pointer to the id table;
*
*Return     : TRUE      scan nand storage success;
*             FALSE     scan nand storage failed;
*********************************************************************************************************
*/
INT32U _SearchIdTable(struct NandDevPar *NandPar, INT8U *NandId, struct NandDevPar *NandIdTbl)
{
    INT32S     i, j;
    INT32U  result;
    INT8U   *tmpNandId;

    i = 0;
    result = FALSE;
    tmpNandId = NandId;

    while(NandIdTbl[i].NandID[0] != 0xff)
    {
        for(j = 1; j < 4; j++)
        {
            /* skip compare the 0xff value */
            if((NandIdTbl[i].NandID[j] != 0xff) && (NandIdTbl[i].NandID[j] != tmpNandId[j]))
            {
        		break;
        	}
        }

        if(j == 4)
        {
            *NandPar = NandIdTbl[i];
            result = TRUE;
            break;
        }

        i++;
    }

    return result;
}


/*
*********************************************************************************************************
*               SEARCH NAND FLASH ID FROM NAND ID TABLE
*
*Description: search nand flash id from nand id table.
*
*Arguments  : NandPar   the pointer to the nand parameters;
*             NandId    the pointer to the and id;
*
*Return     : TRUE      scan nand storage success;
*             FALSE     scan nand storage failed;
*********************************************************************************************************
*/
INT32U _SearchNandId(struct NandDevPar *NandPar, void *NandId)
{
    INT8U   *tmpNandId;
    INT32U  result = TRUE;

    tmpNandId = NandId;

    switch(tmpNandId[0])
    {
        case NAND_MFR_TOSHIBA:
            result = _SearchIdTable(NandPar, NandId, ToshibaNandTbl);
            break;

        case NAND_MFR_SAMSUNG:
            result = _SearchIdTable(NandPar, NandId, SamsungNandTbl);
            break;

        case NAND_MFR_HYNIX:
            result = _SearchIdTable(NandPar, NandId, HynixNandTbl);
            break;

        case NAND_MFR_MICRON:
            result = _SearchIdTable(NandPar, NandId, MicronNandTbl);
            break;

        case NAND_MFR_INTEL:
            result = _SearchIdTable(NandPar, NandId, IntelNandTbl);
            break;

        case NAND_MFR_ST:
            result = _SearchIdTable(NandPar, NandId, StNandTbl);
            break;

        case NAND_MFR_RENESSAS:
            result = FALSE;
            break;
       case NAND_MFR_PowerFlash:
	     result = _SearchIdTable(NandPar, NandId, PowerFlashNandTbl);
            break;
		case NAND_MFR_MIRA:
	     result = _SearchIdTable(NandPar, NandId, MiraFlashNandTbl);
            break;	
		case NAND_MFR_SPANSION:
	     result = _SearchIdTable(NandPar, NandId, SpansionFlashNandTbl);
            break;
		case NAND_MFR_MXIC:
	     result = _SearchIdTable(NandPar, NandId, MxicFlashNandTbl);
            break;
        case NAND_MFR_WinBond:
         result = _SearchIdTable(NandPar, NandId, WinBondFlashNandTbl);
            break;   
        case NAND_MFR_MakerFounder:
            result = _SearchIdTable(NandPar, NandId, MakerFounderFlashNandTbl);  
            break;
        default:
            result = FALSE;
            break;
    }

    return result;
}


/*
*********************************************************************************************************
*               PARSE NAND FLASH ID
*
*Description: parse nand flash id.
*
*Arguments  : NandPar   the pointer to the nand parameters;
*             NandId    the pointer to the and id;
*
*Return     : TRUE      scan nand storage success;
*             FALSE     scan nand storage failed;
*
*Note       : parse tne nand flash id, if the nand flash driver may support the nand flash,
*             create an item in the nand flash id table.
*********************************************************************************************************
*/
INT32U _ParseNandFlashID(struct NandDevPar *NandPar, void *NandId)
{
    INT8U   *tmpNandId;
    INT32U  tmpNandCellType;
    INT32U  tmpVariable;
    INT32U  tmpCapacity;
    INT32U  tmpPlanes;
    INT32U  result = FALSE;
    struct NandDevPar *tmpNandPar;

    tmpNandId = NandId;
    tmpNandPar = &UserCreateNandTbl[0];

    INIT_DBG("INIT_DBG: _ParseNandFlashID() %02x %02x %02x %02x\n", 
        tmpNandId[0], tmpNandId[1], tmpNandId[2], tmpNandId[3]);

    /* create nand flash id for id table item */
    tmpNandPar->NandID[0] = tmpNandId[0];
    tmpNandPar->NandID[1] = tmpNandId[1];
    tmpNandPar->NandID[2] = tmpNandId[2];
    tmpNandPar->NandID[3] = tmpNandId[3];

    /* set chip capacity */
    switch(tmpNandId[1])
    {
        case 0xf1:
            tmpCapacity = 1;    //1Gbit
            break;
        case 0xda:
            tmpCapacity = 2;    //2Gbit
            break;
        case 0xdc:
            tmpCapacity = 4;    //4Gbit
            break;
        case 0xd3:
            tmpCapacity = 8;    //8Gbit
            break;
        case 0xd5:
            tmpCapacity = 16;   //16Gbit
            break;
        case 0xd7:
            tmpCapacity = 32;   //32Gbit
            break;
        case 0xd9:
            tmpCapacity = 64;   //64Gbit
            break;
        default:
            tmpCapacity = 0;    //can't recgnize
            break;
    }

    /* set nand flash die count */
    tmpNandPar->DieCntPerChip = (INT8U)(1 << ((tmpNandId[2]>>0) & 0x03));

    /* set nand flash cell type */
    tmpNandCellType = (INT32U)(2 << ((tmpNandId[2]>>2) & 0x03));

    /* set sector number per page */
    tmpNandPar->SectNumPerPPage = (INT8U)(2 << ((tmpNandId[3]>>0) & 0x03));

    /* set page number per physic block */
    tmpVariable = (INT32U)(1 << ((tmpNandId[3]>>4) & 0x03));
    tmpVariable = tmpVariable * 64 * 2;
    tmpNandPar->PageNumPerPBlk = (INT8U)(tmpVariable / tmpNandPar->SectNumPerPPage);

    /* set nand flash access frequence */
    tmpNandPar->Frequence = 20;

    /* calculate physic block number per die */
    tmpVariable = (tmpCapacity * 1024 * 1024 * 2) /8;
    tmpVariable = tmpVariable / tmpNandPar->SectNumPerPPage;
    tmpVariable = tmpVariable / tmpNandPar->PageNumPerPBlk;
    tmpNandPar->BlkNumPerDie = tmpVariable / tmpNandPar->DieCntPerChip;

    /* set data block number per zone */
    tmpNandPar->DataBlkNumPerZone = DEFAULT_DATA_BLK_PER_ZONE;

    /* set operation option */
    tmpNandPar->OperationOpt = 0;

    tmpPlanes = 1 << ((tmpNandId[2] >> 4) & 0x3);
    if (tmpPlanes == 2)
    {
        tmpNandPar->OperationOpt |= MULTI_PAGE_WRITE;
    }

    /* check if current driver can support the nand flash */
    /* currently, only support 2K page and 4KB page */
    if((tmpCapacity == 0) || 
        ((tmpNandPar->SectNumPerPPage != 4) && (tmpNandPar->SectNumPerPPage != 8)))
    {
        NandPar = NULL;
        return FALSE;
    }

    switch (tmpNandId[0])
    {
        case NAND_MFR_SAMSUNG:
        case NAND_MFR_HYNIX:
        case NAND_MFR_ST:
		case NAND_MFR_SPANSION:
            if (tmpNandPar->SectNumPerPPage == 4)
            {
                if (tmpNandCellType == 2)
                {
                    tmpNandPar->SpecOp = &SpecialType_Samsung_SLC_2K;
                }
                else
                {
                    tmpNandPar->SpecOp = &SpecialType_Samsung_MLC_2K;
                }
                
            }
            else
            {
                if (tmpNandCellType == 2)
                {
                    tmpNandPar->SpecOp = &SpecialType_Samsung_SLC_4K;
                }
                else
                {
                    tmpNandPar->SpecOp = &SpecialType_Samsung_MLC_4K;
                }
            }
            break;

        case NAND_MFR_MICRON:
        case NAND_MFR_INTEL:
            tmpNandPar->SpecOp = &SpecialType_Micron;
            break;

        case NAND_MFR_TOSHIBA:
            if (tmpNandCellType == 2)
            {
                tmpNandPar->SpecOp = &SpecialType_Toshiba_SLC_2K;
            }
            else
            {
                /* There is no two-plane support flag in Toshiba flash chipID,
                 * in fact, all Toshiba MLC flash support two-plane operation. 
                 */
                tmpPlanes = 2;
                tmpNandPar->OperationOpt |= MULTI_PAGE_WRITE;
                tmpNandPar->SpecOp = &SpecialType_Toshiba_MLC_2K_1024;
                tmpNandPar->SpecOp->MultiBlkAddrOffset = tmpNandPar->BlkNumPerDie / tmpPlanes;
            }

            break;

        default:
            break;
    }

    *NandPar = *tmpNandPar;
    result = TRUE;

    return result;
}


/*
*********************************************************************************************************
*               CHECK NAND FLASH RESET STATUS
*
*Description: check nand flash status after reset.
*
*Arguments  : ChipNum   chip select number;
*
*Return     : result
*                   TRUE    nand flash is ok;
*                   FALSE   nand flash is dead or no nand flash;
*********************************************************************************************************
*/
INT32U _CheckResetResult(INT32U ChipNum)
{
    INT32U result;
    INT32U tmpChipNum;

    tmpChipNum = ChipNum;
    result = 0;

    /* enable nand controller, set chip select,and get pin if need */
    _NandGetDevice(tmpChipNum);

    while(!(result & NAND_STATUS_READY))
    {
        result = _ReadNandStatus(0x70, 0x00);
    }

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    if(result & NAND_OPERATE_FAIL)
    {
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    return result;
}


/*
*********************************************************************************************************
*               SCAN NAND STORAGE SYSTEM TO INIT CIRCUMSTANCE
*
*Description: scan the nand storage system to init some parameters for nand flash driver.
*
*Arguments  : none
*
*Return     : TRUE      scan nand storage success;
*             FALSE     scan nand storage failed;
*********************************************************************************************************
*/
INT32U  INIT_ScanNandStorage(void)
{
    INT32S i=0x00;
    INT32U result;
    INT8U  tmpNandFlashId[5];
    struct NandDevPar NandPar;
    struct StorageInfo *tmpNandInfo;
    struct LogicOrganizeType *tmpLogOrgaType;
    struct SpecialCmdType *tmpSpeOpPar;
    struct BitMapInfo   *tmpBITMAP;	

    /* check if there is nand flash on boot chip enable */
    PHY_NandReset(BOOT_CHIP_NUM);

    result = _CheckResetResult(BOOT_CHIP_NUM);
    if(result != TRUE)
    {
        INIT_ERR("INIT_ERR: There is no nand flash on boot chip!\n");

		PHY_ReadNandId(BOOT_CHIP_NUM, &tmpNandFlashId);
		INIT_ERR("Flash ChipNo:%d CHIPID:%x\n",i, *(INT32U *)tmpNandFlashId);

        return FALSE;
    }

    /* read nand flash id from boot chip */
    PHY_ReadNandId(BOOT_CHIP_NUM, &tmpNandFlashId);

        INIT_BOOT("\n====22 boot nand Flash CHIPID:0x%x ====\n", *(INT32U *)tmpNandFlashId);
   
    
    /* search nand flash id from nand id table, to get nand parameters */
    result = _SearchNandId(&NandPar, &tmpNandFlashId);
    if(result != TRUE)
    {
        /* can't find the nand flash type in nand flash id table, parse it */
	INIT_ERR("can't find the nand flash type in nand flash id table, parse it !!!\n");
      //  result = _ParseNandFlashID(&NandPar, &tmpNandFlashId);
      //  if(result != TRUE)
        {
            INIT_ERR("INIT_ERR: Nand flash driver can't recognize the boot nand type! (id 0x%x)\n",
                        *(INT32U *)tmpNandFlashId);
            return FALSE;
        }
    }

    tmpNandInfo = NandDevInfo.NandFlashInfo;
    tmpLogOrgaType = NandDevInfo.LogicOrganizePar;
    tmpSpeOpPar = NandDevInfo.SpecialCommand;
    tmpBITMAP = NandDevInfo.BITMAP_DATA;

    /* process nand storage infor struct */
    tmpNandInfo->BankCntPerChip = NandPar.DieCntPerChip;
    tmpNandInfo->DieCntPerChip = NandPar.DieCntPerChip;
    tmpNandInfo->PlaneCntPerDie = 2;
    tmpNandInfo->SectorNumPerPage = NandPar.SectNumPerPPage;
    tmpNandInfo->PageNumPerPhyBlk = NandPar.PageNumPerPBlk;
    tmpNandInfo->TotalBlkNumPerDie = NandPar.BlkNumPerDie;
    tmpNandInfo->OperationOpt = NandPar.OperationOpt;
    tmpNandInfo->FrequencePar = NandPar.Frequence;
    tmpNandInfo->FrequencePar= NandPar.Frequence_N;
    tmpNandInfo->Reclaim = NandPar.Reclaim;
    tmpNandInfo->SpareSize= NandPar.SpareSize;
    tmpNandInfo->NandChipId[0] = tmpNandFlashId[0];

    /* fill 0xff for these don't care chipid byte */
    for (i = 0; i < 4; i++)
    {
        tmpNandInfo->NandChipId[i] = NandPar.NandID[i];
    }

    /* process the optional operation */
    if(!SUPPORT_INTERNAL_INTERLEAVE)
    {
        tmpNandInfo->OperationOpt &= ~INTERNAL_INTERLEAVE;
    }

    /* assure the block number in a die is greater than a minimum block number in a zone */
    if(!SUPPORT_MULTI_PLANE || tmpNandInfo->TotalBlkNumPerDie <= BLK_NUM_PER_ZONE)
    {
        tmpNandInfo->OperationOpt &= ~MULTI_PAGE_WRITE;
    }

    /* process bank count in chip */
    if(!((tmpNandInfo->OperationOpt) & INTERNAL_INTERLEAVE))
    {
        tmpNandInfo->BankCntPerChip = 1;
    }

    /* process plane count per die */
    if(!((tmpNandInfo->OperationOpt) & MULTI_PAGE_WRITE))
    {
        tmpNandInfo->PlaneCntPerDie = 1;
    }



    

    /* process logic organize struct */
    tmpLogOrgaType->DataBlkNumPerZone = NandPar.DataBlkNumPerZone;
    tmpLogOrgaType->PageNumPerLogicBlk = tmpNandInfo->PageNumPerPhyBlk * tmpNandInfo->BankCntPerChip;
    tmpLogOrgaType->ZoneNumPerDie = (tmpNandInfo->TotalBlkNumPerDie / tmpNandInfo->PlaneCntPerDie) / BLK_NUM_PER_ZONE;
    tmpLogOrgaType->SectorNumPerLogicPage = tmpNandInfo->SectorNumPerPage * tmpNandInfo->PlaneCntPerDie;

    /* process special command and address struct */
    tmpSpeOpPar->MultiProgCmd[0] = NandPar.SpecOp->MultiProgCmd[0];
    tmpSpeOpPar->MultiProgCmd[1] = NandPar.SpecOp->MultiProgCmd[1];

    tmpSpeOpPar->MultiCopyReadCmd[0] = NandPar.SpecOp->MultiCopyReadCmd[0];
    tmpSpeOpPar->MultiCopyReadCmd[1] = NandPar.SpecOp->MultiCopyReadCmd[1];
    tmpSpeOpPar->MultiCopyReadCmd[2] = NandPar.SpecOp->MultiCopyReadCmd[2];

    tmpSpeOpPar->MultiCopyProgCmd[0] = NandPar.SpecOp->MultiCopyProgCmd[0];
    tmpSpeOpPar->MultiCopyProgCmd[1] = NandPar.SpecOp->MultiCopyProgCmd[1];
    tmpSpeOpPar->MultiCopyProgCmd[2] = NandPar.SpecOp->MultiCopyProgCmd[2];

    tmpSpeOpPar->MultiBlkAddrOffset = NandPar.SpecOp->MultiBlkAddrOffset;
    tmpSpeOpPar->BadBlkFlagPst = NandPar.SpecOp->BadBlkFlagPst;
    tmpSpeOpPar->ReadMultiOpStatusCmd = NandPar.SpecOp->ReadMultiOpStatusCmd;
    tmpSpeOpPar->InterChip0StatusCmd = NandPar.SpecOp->InterChip0StatusCmd;
    tmpSpeOpPar->InterChip1StatusCmd = NandPar.SpecOp->InterChip1StatusCmd;

    /* process nand chip count */
    tmpNandInfo->ChipCnt = 0;
    tmpNandInfo->ChipEnableInfo = 0;

    for(i = 0; i < NAND_MAX_CHIP_NUM; i++)
    {
        INT32S j;

        /* check if there is nand flash on chip ce */
        PHY_NandReset(i);
        /* wait nand flash status ready */
        _CheckResetResult(i);

        /* read current nand flash id */
        PHY_ReadNandId(i, &tmpNandFlashId);
	INIT_ERR("Flash ChipNo:%d CHIPID:%x\n",i, *(INT32U *)tmpNandFlashId);
        

        /* compare chipid */
        for (j = 0; j < 4; j++)
        {
            if (tmpNandInfo->NandChipId[j] != 0xff && 
                tmpNandInfo->NandChipId[j] != tmpNandFlashId[j])
            {
                break;
            }
        }

        if (j != 4)
        {
            break;
        }

        tmpNandInfo->ChipCnt++;
        tmpNandInfo->ChipEnableInfo |= 0x01 << i;
    }

    /* process if external interleave */
    if(SUPPORT_EXTERNAL_INTERLEAVE)
    {
        if(tmpNandInfo->ChipCnt > 1)
        {
            tmpNandInfo->OperationOpt |= EXTERNAL_INTERLEAVE;
            tmpLogOrgaType->PageNumPerLogicBlk *= tmpNandInfo->ChipCnt;
        }
    }

#if CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

	tmpBITMAP->USER_DataByte =0x02;
	tmpBITMAP->USER_SectorCnt=0x03;
	tmpBITMAP->TOTAL_SPAREDATA = 0x02*0x03;
	switch(tmpNandInfo->OperationOpt &(0xE000))
	{
		case ECC_Type_8BitECC: //BCH8 512Byte
			if(NAND_SUPPORT_MULTI_PLANE)
			{
				tmpBITMAP->Sector_BitmapBBF = GenSectorBitMap(0x3,0x3);
				tmpBITMAP->Sector_BitmapSpare = GenSectorBitMap(0x07,0x07);
			}
			else
			{
				tmpBITMAP->Sector_BitmapBBF = GenSectorBitMap(0x3,0);
				tmpBITMAP->Sector_BitmapSpare= GenSectorBitMap(0x7,0);
			}
			tmpBITMAP->Single_BitmapSpare = GenSectorBitMap(0x7,0);
			break;
			//BCH24  1KB
		case ECC_Type_12BitECC:
		case ECC_Type_24BitECC:
		case ECC_Type_40BitECC:
		case ECC_Type_60BitECC:
			if(NAND_SUPPORT_MULTI_PLANE)
			{
				tmpBITMAP->Sector_BitmapBBF = GenSectorBitMap(0xF,0xF);
				tmpBITMAP->Sector_BitmapSpare = GenSectorBitMap(0x3F,0x3F);
			}
			else
			{
				tmpBITMAP->Sector_BitmapBBF = GenSectorBitMap(0xF,0);
				tmpBITMAP->Sector_BitmapSpare= GenSectorBitMap(0x3F,0);
			}
			tmpBITMAP->Single_BitmapSpare = GenSectorBitMap(0x3F,0);
			break;
		default:
			break;
	}

	
	switch(tmpNandInfo->OperationOpt &(0xE000))
	{
		case ECC_Type_8BitECC: //BCH8 512Byte
			SET_READ_Reclaim = RECLAIM_VAL;
			break;
			//BCH24  1KB
		case ECC_Type_12BitECC:
			
			SET_READ_Reclaim = RECLAIM_VAL*2;
			break;
		case ECC_Type_24BitECC:		
			SET_READ_Reclaim = RECLAIM_VAL;
			break;
		case ECC_Type_40BitECC:
			SET_READ_Reclaim = RECLAIM_VAL;
			break;
		case ECC_Type_60BitECC:
			SET_READ_Reclaim = RECLAIM_VAL;

		default:
			break;
	}

	
#elif  CONFIG_AM_CHIP_ID==1203
	tmpBITMAP->USER_DataByte =0x03;
	tmpBITMAP->USER_SectorCnt=0x02;
	tmpBITMAP->TOTAL_SPAREDATA = 0x02*0x03;
	
	switch(tmpNandInfo->OperationOpt &(0xE000))
	{
		case ECC_Type_8BitECC:			
		case ECC_Type_12BitECC:
		case ECC_Type_24BitECC:
			if(NAND_SUPPORT_MULTI_PLANE)
			{
				tmpBITMAP->Sector_BitmapBBF = GenSectorBitMap(0x1,0x1);
				tmpBITMAP->Sector_BitmapSpare = GenSectorBitMap(0x03,0x03);
			}
			else
			{
				tmpBITMAP->Sector_BitmapBBF = GenSectorBitMap(0x1,0);
				tmpBITMAP->Sector_BitmapSpare= GenSectorBitMap(0x3,0);
			}
			tmpBITMAP->Single_BitmapSpare = GenSectorBitMap(0x3,0);
			break;
		default:
			break;
	}
#endif

	/*is support EDO mode*/
	Set_NAND_CTL_Access_Mode(NandPar.NandID);

	    /*process 8Bit ECC ,12Bit ECC NF CTL*/
    Set_NAND_ECCCTL(tmpNandInfo->OperationOpt &(0xE000),0x01);

    /* setting nand frequence parameter */
    SettingNandFreq(tmpNandInfo->FrequencePar, 120);

    /* reset nand flash, and wait nand flash ready */
    PHY_NandReset(BOOT_CHIP_NUM);
    result = _CheckResetResult(BOOT_CHIP_NUM);
    if(result != TRUE)
    {
        INIT_ERR("INIT_ERR: Nand Controller statemachine is dead!\n");
        return FALSE;
    }

    /* dump nand storage informaiton parameters */
    INIT_DBG("INIT_DBG: ================NAND STORAGE INFOR================\n");
    INIT_DBG("INIT_DBG:     ChipEnableInfo is:0x%x\n", NandDevInfo.NandFlashInfo->ChipEnableInfo);
    INIT_DBG("INIT_DBG:     ChipCnt        is:0x%x\n", NandDevInfo.NandFlashInfo->ChipCnt);
    INIT_DBG("INIT_DBG:     BankCntPerChip is:0x%x\n", NandDevInfo.NandFlashInfo->BankCntPerChip);
    INIT_DBG("INIT_DBG:     DieCntPerChip  is:0x%x\n", NandDevInfo.NandFlashInfo->DieCntPerChip);
    INIT_DBG("INIT_DBG:     PlaneCntPerDie is:0x%x\n", NandDevInfo.NandFlashInfo->PlaneCntPerDie);
    INIT_DBG("INIT_DBG:     SectorNumPerPage  is:0x%x\n", NandDevInfo.NandFlashInfo->SectorNumPerPage);
    INIT_DBG("INIT_DBG:     PageNumPerPhyBlk  is:0x%x\n", NandDevInfo.NandFlashInfo->PageNumPerPhyBlk);
    INIT_DBG("INIT_DBG:     TotalBlkNumPerDie is:0x%x\n", NandDevInfo.NandFlashInfo->TotalBlkNumPerDie);
    INIT_DBG("INIT_DBG:     OperationOpt is:0x%x\n", NandDevInfo.NandFlashInfo->OperationOpt);
    INIT_DBG("INIT_DBG:     FrequencePar is:0x%x\n", NandDevInfo.NandFlashInfo->FrequencePar);
    INIT_DBG("INIT_DBG:     Reclaim is:%d\n", NandDevInfo.NandFlashInfo->Reclaim);
    INIT_DBG("INIT_DBG:     SpareSize is:%d\n", NandDevInfo.NandFlashInfo->SpareSize);
    INIT_DBG("INIT_DBG:     NandChipId   is:0x%x\n", (NandDevInfo.NandFlashInfo->NandChipId[0] << 0) \
                                                   | (NandDevInfo.NandFlashInfo->NandChipId[1] << 8) \
                                                   | (NandDevInfo.NandFlashInfo->NandChipId[2] << 16) \
                                                   | (NandDevInfo.NandFlashInfo->NandChipId[3] << 24));
    INIT_DBG("INIT_DBG: ====================================================\n\n");

    /* dump nand flash logic organizition parameters */
    INIT_DBG("INIT_DBG: ================LOGIC ORGANIZE INFO=================\n");
    INIT_DBG("INIT_DBG:     DataBlkNumPerZone  is:0x%x\n", NandDevInfo.LogicOrganizePar->DataBlkNumPerZone);
    INIT_DBG("INIT_DBG:     PageNumPerLogicBlk is:0x%x\n", NandDevInfo.LogicOrganizePar->PageNumPerLogicBlk);
    INIT_DBG("INIT_DBG:     SectorNumPerLogicPage is:0x%x\n", NandDevInfo.LogicOrganizePar->SectorNumPerLogicPage);
    INIT_DBG("INIT_DBG:     ZoneNumPerDie      is:0x%x\n", NandDevInfo.LogicOrganizePar->ZoneNumPerDie);
    INIT_DBG("INIT_DBG: ====================================================\n\n");

    /* dump nand flash special command and struct parameters */
    INIT_DBG("INIT_DBG: ===============SPECICAL CMMOND INFO=================\n");
    INIT_DBG("INIT_DBG:     MultiProgCmd         is:0x%x, 0x%x\n", NandDevInfo.SpecialCommand->MultiProgCmd[0], \
                                                                  NandDevInfo.SpecialCommand->MultiProgCmd[1]);
    INIT_DBG("INIT_DBG:     MultiCopyReadCmd     is:0x%x, 0x%x, 0x%x\n",  \
                                                                  NandDevInfo.SpecialCommand->MultiCopyReadCmd[0], \
                                                                  NandDevInfo.SpecialCommand->MultiCopyReadCmd[1], \
                                                                  NandDevInfo.SpecialCommand->MultiCopyReadCmd[2]);
    INIT_DBG("INIT_DBG:     MultiCopyProgCmd     is:0x%x, 0x%x, 0x%x\n", \
                                                                  NandDevInfo.SpecialCommand->MultiCopyProgCmd[0], \
                                                                  NandDevInfo.SpecialCommand->MultiCopyProgCmd[1], \
                                                                  NandDevInfo.SpecialCommand->MultiCopyProgCmd[2]);
    INIT_DBG("INIT_DBG:     MultiBlkAddrOffset   is:0x%x\n", NandDevInfo.SpecialCommand->MultiBlkAddrOffset);
    INIT_DBG("INIT_DBG:     BadBlkFlagPst        is:0x%x\n", NandDevInfo.SpecialCommand->BadBlkFlagPst);
    INIT_DBG("INIT_DBG:     ReadMultiOpStatusCmd is:0x%x\n", NandDevInfo.SpecialCommand->ReadMultiOpStatusCmd);
    INIT_DBG("INIT_DBG:     InterChip0StatusCmd  is:0x%x\n", NandDevInfo.SpecialCommand->InterChip0StatusCmd);
    INIT_DBG("INIT_DBG:     InterChip1StatusCmd  is:0x%x\n", NandDevInfo.SpecialCommand->InterChip1StatusCmd);
    INIT_DBG("INIT_DBG: =====================================================\n\n");
    INIT_DBG("INIT_DBG: ===============Sector Bimmap=================\n");
    INIT_DBG("INIT_DBG 1.===>Sector_BitmapBBF:0x%x\n",NandDevInfo.BITMAP_DATA->Sector_BitmapBBF);
    INIT_DBG("INIT_DBG 2.===>Sector_BitmapSpare:0x%x\n",NandDevInfo.BITMAP_DATA->Sector_BitmapSpare);
    INIT_DBG("INIT_DBG 3.===>USER_DataByte:0x%x\n",NandDevInfo.BITMAP_DATA->USER_DataByte);
    INIT_DBG("INIT_DBG 4.===>USER_SectorCnt:0x%x\n",NandDevInfo.BITMAP_DATA->USER_SectorCnt);
    INIT_DBG("INIT_DBG 5.===>TOTAL_SPAREDATA:%x\n",NandDevInfo.BITMAP_DATA->TOTAL_SPAREDATA);
    INIT_DBG("INIT_DBG 6.===>Single_BitmapSpare:%x\n",NandDevInfo.BITMAP_DATA->Single_BitmapSpare);
  INIT_DBG("INIT_DBG: =====================================================\n\n");	
    return TRUE;
}

