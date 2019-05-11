/*
*********************************************************************************************************
*                                                USDK130
*                                          ucOS + MIPS,Nand Module
*
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : brec_read_write.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-10-17 9:22
*********************************************************************************************************
*/
#if (!defined(__KERNEL__)) 
#include "os_cpu.h"
#include "sysinfo.h"
#include "adfu.h"
#endif

#include "nand_flash_driver_type.h"
#include "./../flash_drv_params.h"
#include "sys_buf_def.h"

#define MBREC_BLK_CNT       4
unsigned int MBRC_Total_Size, MBRC_SECTOR_NUM;
INT16U bPageSize;
extern INT32U  PHY_WaitNandFlashReady(INT32U BankNum);
extern INT32U  PHY_SyncNandOperation(INT32U BankNum);
extern INT32U  PHY_EraseNandBlk(struct PhysicOpParameter *NandOpPar);
extern INT32U  PHY_PageRead(struct PhysicOpParameter *NandOpPar);
extern INT32U  PHY_PageWrite(struct PhysicOpParameter *NandOpPar);
extern INT32U FTL_CalPhyOpPar(struct PhysicOpParameter * PhyOpPar, INT32U ZoneNum, 
	INT32U SupBlkNum, INT32U PageNum);
extern INT32U  _PHY_WholePageRead(struct PhysicOpParameter *NandOpPar);
extern INT32U  PHY_WholePageWrite(struct PhysicOpParameter *NandOpPar);

//extern INT8U MBRC_Total_Size; //CODE+TABLE

static void ConvertNandParameter(INT32U BrecBlkNum, INT32U BrecSectNum, struct PhysicOpParameter *Nand_Parameter);
extern INT32U  PHY_EraseSuperBlk(struct PhysicOpParameter *PhyOpPar);

INT8U SET_BootData_Parm(INT8U BrecBlkNum)
{
	INT32U   PNFlag = 1;

#if  CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213 

    if(BrecBlkNum>=8 )
#else
	if(BrecBlkNum>=4)
#endif
    {
        switch(BCH_MODE)
        {
            case ECC_Type_8BitECC:
	                NandStorageInfo.OperationOpt &=~(0xE000);
                bPageSize =0x04;
                break;
            case ECC_Type_12BitECC:
            case ECC_Type_24BitECC:
					bPageSize =0x08;	
	                break;
			case ECC_Type_40BitECC:
			case ECC_Type_60BitECC:				
					PNFlag = 0;
                bPageSize =0x08;	
                break;
            default:
                break;
        }
    }
	else
	{
#ifdef Mbrec_Support_BCH24
        NandStorageInfo.OperationOpt = 0x4000;//&=~(0xE000);
		bPageSize =0x08;
#else
        NandStorageInfo.OperationOpt &=~(0xE000);
        bPageSize =0x04;
#endif
    }
    Set_NAND_ECCCTL( NandStorageInfo.OperationOpt &(0xE000),0x00);
    _Pseudo_NoiseFunc(PNFlag); //Disable pseudo noise 
    NandStorageInfo.SectorNumPerPage = bPageSize;
    
    return 0x00;
}

INT8U Restore_BootData_parm(void)
{
    Set_NAND_ECCCTL(NandStorageInfo.OperationOpt &(0xE000),0);
	_Pseudo_NoiseFunc(0); //Enable pseudo noise 
    return 0x00;
}

#if (defined(__KERNEL__)   || BREC_DRIVER == 0x01)
INT32U Test_ReadMbrecData(INT32U BrecBlkNum,INT32U BrecSectNum, INT8U *Buf)
{
	/* for save nand flash driver prameter */
	INT8U   tmpPlaneCntPerDie;      //the count of plane in one die
	INT8U   tmpSectorNumPerPage;    //page size,based on 0.5k
	INT16U  tmpOperationOpt;        //the operation support bitmap
	struct  PhysicOpParameter nand_parameter;

	/* save nand flash driver parameter */
	tmpPlaneCntPerDie = NandStorageInfo.PlaneCntPerDie;
	tmpSectorNumPerPage = NandStorageInfo.SectorNumPerPage;
	tmpOperationOpt = NandStorageInfo.OperationOpt;
	
	
	/* reset nand flash driver parameter */
	bPageSize =0x04;
	NandStorageInfo.PlaneCntPerDie = 1;
	NandStorageInfo.SectorNumPerPage = 0x04;
	NandStorageInfo.OperationOpt &= ~(MULTI_PAGE_WRITE);
	

#if CONFIG_AM_CHIP_ID ==1211|| CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213  


    SET_BootData_Parm(BrecBlkNum);
#endif

	/* calculate the nand operation parameter */
	ConvertNandParameter(BrecBlkNum, BrecSectNum, &nand_parameter);

	/* set buffer address */
	nand_parameter.MainDataPtr = (void*)Buf;
	nand_parameter.SpareDataPtr = NULL;

	/* get the sector data to buffer */
	_PHY_PageRead(&nand_parameter);

	/* restore nand flash driver parameter */
	NandStorageInfo.PlaneCntPerDie = tmpPlaneCntPerDie;
	NandStorageInfo.SectorNumPerPage = tmpSectorNumPerPage;
	NandStorageInfo.OperationOpt = tmpOperationOpt;


#if CONFIG_AM_CHIP_ID ==1211|| CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213    	

	Restore_BootData_parm();
#endif
  
    return TRUE;
}
#endif
/*
*********************************************************************************************************
*                       READ A SECTOR OF BREC CODE
*
* Description: read a sector of brec code.
*
* Aguments   : Brec_Block_Num   Block number of the brec code. one mbrec code or one brec code
*                               use a brecblock,it may be n times of nand block;
*              Brec_Sector_Num  the sector offset in the brec block;
*              Buffer_Addr      the buffer address;
*
* Returns    : the result of read.
*               0 - read successfully;
*              !0 - error;
*
* Note       : This routine is used for adfu under udisk status to update firmware.
*********************************************************************************************************
*/
INT32S brec_sector_read(INT32U BrecBlkNum,INT32U BrecSectNum, INT8U *Buf)
{
    /* for save nand flash driver prameter */
    INT8U   tmpPlaneCntPerDie;      //the count of plane in one die
    INT8U   tmpSectorNumPerPage;    //page size,based on 0.5k
    INT16U  tmpOperationOpt;        //the operation support bitmap
    INT32U  result;
    struct  PhysicOpParameter nand_parameter;

    /* write data to nand flash if it is in write buffer */
    FTL_FlushPageCache();

    /* save nand flash driver parameter */
    tmpPlaneCntPerDie = NandStorageInfo.PlaneCntPerDie;
    tmpSectorNumPerPage = NandStorageInfo.SectorNumPerPage;
    tmpOperationOpt = NandStorageInfo.OperationOpt;

    /* reset nand flash driver parameter */
    bPageSize =0x04;
    NandStorageInfo.PlaneCntPerDie = 1;
    NandStorageInfo.SectorNumPerPage = 4;
    NandStorageInfo.OperationOpt &= ~(MULTI_PAGE_WRITE);
#if defined(__KERNEL__)     
//   printf("## %s %d %d,%x\n",__func__,BrecBlkNum,BrecSectNum,Buf);
#endif


#if CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213  

    SET_BootData_Parm(BrecBlkNum);
#endif

	/* calculate the nand operation parameter */
	ConvertNandParameter(BrecBlkNum, BrecSectNum, &nand_parameter);

	/* set buffer address */
	nand_parameter.MainDataPtr = (void*)Buf;
	nand_parameter.SpareDataPtr = NULL;

	/* get the sector data to buffer */
	result = PHY_PageRead(&nand_parameter);

	/* restore nand flash driver parameter */
	NandStorageInfo.PlaneCntPerDie = tmpPlaneCntPerDie;
	NandStorageInfo.SectorNumPerPage = tmpSectorNumPerPage;
	NandStorageInfo.OperationOpt = tmpOperationOpt;


#if CONFIG_AM_CHIP_ID ==1211|| CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213    	

	Restore_BootData_parm();
#endif
  
    return result;
}


/*
*********************************************************************************************************
*                       WRITE A SECTOR OF BREC CODE
*
* Description: write a sector of brec code.
*
* Aguments   : Brec_Block_Num   Block number of the brec code. one mbrec code or one brec code
*                               use a brecblock,it may be n times of nand block;
*              Brec_Sector_Num  the sector offset in the brec block;
*              Buffer_Addr      the buffer address;
*
* Returns    : the result of write.
*               0 - write successfully;
*              !0 - write error;
*
* Note       : This routine is used for adfu under udisk status to update firmware.
*********************************************************************************************************
*/
INT32S brec_sector_write(INT32U BrecBlkNum,INT32U BrecSectNum, INT8U *Buf)
{
	/* for save nand flash driver prameter */
	INT8U   tmpPlaneCntPerDie;      //the count of plane in one die
	INT8U   tmpSectorNumPerPage;    //page size,based on 0.5k
	INT16U  tmpOperationOpt;        //the operation support bitmap
	struct  PhysicOpParameter nand_parameter;
	struct  NandSpareData tmpSpareData;
	INT32S     result = 0;


	/* write data to nand flash if it is in write buffer */
	FTL_FlushPageCache();

	/* save nand flash driver parameter */
	tmpPlaneCntPerDie = NandStorageInfo.PlaneCntPerDie;
	tmpSectorNumPerPage = NandStorageInfo.SectorNumPerPage;
	tmpOperationOpt = NandStorageInfo.OperationOpt;
	
	/* reset nand flash driver parameter */
	bPageSize =0x04; 
	NandStorageInfo.PlaneCntPerDie = 1;
	NandStorageInfo.SectorNumPerPage = 4;
	NandStorageInfo.OperationOpt &= ~(MULTI_PAGE_WRITE);
    

#if CONFIG_AM_CHIP_ID ==1211  || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213 

    SET_BootData_Parm(BrecBlkNum);
#endif

	/* set spare area data */
	MEMSET((INT8U*)&tmpSpareData, 0xff, sizeof(struct NandSpareData));

	tmpSpareData.UserData[0].SpareData0.LogicBlkInfo = 0x80bb;
	
	/* calculate the nand operation parameter */
	ConvertNandParameter(BrecBlkNum, BrecSectNum, &nand_parameter);

	
	/* set buffer address */
	nand_parameter.MainDataPtr = (void*)NAND_PAGE_CACHE;
	nand_parameter.SpareDataPtr = (void*)&tmpSpareData;

	/*2008-3-7	now Mbrc size is 16KB, so Mbrc and Brec parameters are calculated by the same method.*/
	{
		/* access brec data */
		if((nand_parameter.PageNum == 0) && ((BrecSectNum % bPageSize) == 0))
		{
			/* write a new block, need erase it first */
			result = PHY_EraseNandBlk(&nand_parameter);
			/* sync the erase operation */
			result |= PHY_SyncNandOperation(nand_parameter.BankNum);
			/* wait nand flash true ready */
			PHY_WaitNandFlashReady(nand_parameter.BankNum);
			if(result != TRUE) 
				goto _TEMP_BREAK;
		}
		if(BrecBlkNum < 4)
		{
			if(BrecSectNum ==0x00 )
			{
				if(Buf[506] ==0xaa && Buf[507] ==0x55)
				{
					MBRC_Total_Size = *(volatile unsigned int*)(Buf + 0x28);//must before createTbl()  
#ifdef Mbrec_Support_BCH24
                    MBRC_Total_Size = MBRC_Total_Size *2;
                    MBRC_Total_Size +=2;//add mbrec table size
#else
                    MBRC_Total_Size +=1;
#endif
					printf("MBRC_Total_Size:%x\n",MBRC_Total_Size);
				}
				else if (Buf[508] ==0xaa && Buf[509] ==0x55) //multi mbrec sector 0x01
				{
					MBRC_Total_Size =0x01;
				}
			}
			//
		}
        
		/* copy data from user buffer to nand write buffer */
		MEMCPY(NAND_PAGE_CACHE + (BrecSectNum % bPageSize) * 0x200, Buf, 0x200);

		/* if the whole 2k data all in write buffer, write to nand */
		//if((BrecSectNum % 4) == 3)
		//if((BrecSectNum % 4) == 3 || ((BrecBlkNum < 4)&&(BrecSectNum == (INT32U)(MBRC_Total_Size-1))))
		if((BrecSectNum % bPageSize) == (bPageSize-1) || ((BrecBlkNum < 4)&&(BrecSectNum == (INT32U)(MBRC_Total_Size-1))))
		{
		
			/* write the data to nand flash */
			PHY_PageWrite(&nand_parameter);
			result = PHY_SyncNandOperation(nand_parameter.BankNum);
			PHY_WaitNandFlashReady(nand_parameter.BankNum);
		}
	}

_TEMP_BREAK:

	/* restore nand flash driver parameter */
	NandStorageInfo.PlaneCntPerDie = tmpPlaneCntPerDie;
	NandStorageInfo.SectorNumPerPage = tmpSectorNumPerPage;
	NandStorageInfo.OperationOpt = tmpOperationOpt;
	

#if CONFIG_AM_CHIP_ID ==1211|| CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213    	

	Restore_BootData_parm();
#endif
  
    return TRUE;
}
#if (defined(__KERNEL__)) 
EXPORT_SYMBOL(brec_sector_read);
EXPORT_SYMBOL(brec_sector_write);
#endif

/*
*********************************************************************************************************
*                       CONVERT THE NAND OPERATION PARAMETER BY BRECBLOCK AND BRECSECTOR
*
* Description: convert the brec-block and brec-sector to nand operation parameter.
*
* Aguments   : Brec_Block_Num   Block number of the brec code. one mbrec code or one brec code
*                               use a brecblock,it may be n times of nand block;
*              Brec_Sector_Num  the sector offset in the brec block;
*              Nand_Parameter   the pointer to the nand flash operate parameter;
*
* Returns    : NULL.
*********************************************************************************************************
*/
static void ConvertNandParameter(INT32U BrecBlkNum, INT32U BrecSectNum, struct PhysicOpParameter *Nand_Parameter)
{
	INT32U tmpBlkCntPerBrec, tmpNandBlkSize, tmpNandBlkNum, tmpNandPageNum, tmpSectBitMap;

	/*2008-3-7	now Mbrc size is 16KB, so Mbrc and Brec parameters are calculated by the same method.*/
	{
		/* calculate the sector number of the block for brec code */
		tmpNandBlkSize = bPageSize * PAGE_CNT_PER_PHY_BLK;
		
		/* 1.0 calculate how many blocks does the brec-block contain */
		tmpBlkCntPerBrec = (NAND_BREC_SIZE/512) / tmpNandBlkSize;
			
		if(((NAND_BREC_SIZE/512) % tmpNandBlkSize) != 0) 
			tmpBlkCntPerBrec ++;

		/* calculate the nand block number of current brec-block */
		tmpNandBlkNum = MBREC_BLK_CNT + (BrecBlkNum - 4) * tmpBlkCntPerBrec + (BrecSectNum / tmpNandBlkSize);
		/* calculate the nand page number of current brec-sector */
		tmpNandPageNum = (BrecSectNum % tmpNandBlkSize) / bPageSize;
		/* calculate the sector bitmap in page */
		tmpSectBitMap = 1 << ((BrecSectNum % tmpNandBlkSize) % bPageSize);

		/* set the return value */
		Nand_Parameter->BankNum = 0x00;
		Nand_Parameter->PageNum = tmpNandPageNum;
		Nand_Parameter->PhyBlkNumInBank = tmpNandBlkNum;
		Nand_Parameter->SectorBitmapInPage = tmpSectBitMap;
	}

#if 0
	printf("Parameter_blknum:%d,%d,blk:%d,Page:%d,bitmap:%x\n",BrecBlkNum,BrecSectNum,Nand_Parameter->PhyBlkNumInBank,
		Nand_Parameter->PageNum,Nand_Parameter->SectorBitmapInPage);
#endif
}



INT32U Read_Brec_code(INT16U size,INT8U *Buf)
{
	INT32U iLoop,BitMap,sublk,jLoop,Breccnt,j;
	INT16U *p;
	INT16U checksum,checksum2;
	INT16U BrecFlag;
	struct BootBlk_Info  BlkInfo;
	INT16U MaxECCBit,TotalECCBit,result;
	
	Breccnt=size;
	
	for(iLoop =0x04;iLoop <8;iLoop++)
	{
		BitMap =0xFF;
		sublk =   iLoop;
		checksum =0x00;
		brec_sector_read( sublk,Breccnt-1 , NAND_PAGE_CACHE);
		BrecFlag =*(INT16U*)(NAND_PAGE_CACHE+508);
		if(BrecFlag !=0x55aa)
		{		
			INIT_BOOT("brec flag error \n");
		//	continue;
		//	result =0x1;
		//	return result;
		}
		
		MaxECCBit = 0x00;
		TotalECCBit = 0x00;
		PHY_BCH_CNT(BCH_CLEAR_VAL,0);
		 
		for(jLoop=0x00;jLoop<Breccnt;jLoop++)
		{	
		   
			brec_sector_read( sublk, jLoop, NAND_PAGE_CACHE);

    		//@fish add 2011-02-11 增加几个error bit 
    		TotalECCBit += PHY_BCH_CNT(BCH_GET_VAL,0);
    		if(MaxECCBit<PHY_BCH_CNT(BCH_GET_VAL,0))
    		{
    		    MaxECCBit = PHY_BCH_CNT(BCH_GET_VAL,0);		    
    		}
    		PHY_BCH_CNT(BCH_CLEAR_VAL,0);	
    		///INIT_BOOT("## %d,%d\n",TotalECCBit,MaxECCBit);
    		
			p =(INT16U*) (NAND_PAGE_CACHE);
			for( j=0; j<255; j++ )
			{
				checksum = checksum + (*p);
				p++;
			}
			if(jLoop != (Breccnt - 1) )     //Brec最后一个sector
			{
				checksum = checksum + (*p);
			}

		}
			  
		checksum2 =*(INT16U*) (NAND_PAGE_CACHE+510);
		if(checksum  !=checksum2)
		{
			INIT_BOOT("### Boot_data:%d, checksum fail Fail### :0x%x 0x%x %dKB\n\n",
			    iLoop-4,checksum,checksum2,Breccnt/2);
		}
		//else
		{
			//INIT_BOOT("###  Boot_data :%d, checksum OK OK### :0x%x 0x%x %dKB\n\n",
			   // iLoop-4,checksum,checksum2,Breccnt/2);
		} 
		//INIT_BOOT("###Brec data blk:%d, checksum :0x%x 0x%x %dKB\n\n",
		//	    iLoop-4,checksum,checksum2,Breccnt/2);
		
		MEMSET(&BlkInfo,0x00,sizeof(struct BootBlk_Info));
		MEMCPY(BlkInfo.name,"brec",4);
		BlkInfo.blknum = iLoop-4;
		if(checksum2!=checksum)
			BlkInfo.check = 0x01;//CurCheckSum
		else
			BlkInfo.check =0x00;
		BlkInfo.Flag = *(INT16U*) (NAND_PAGE_CACHE+508);
		if(BlkInfo.Flag !=0x55aa)
			BlkInfo.check = 0x01;
		BlkInfo.Flashchecksum = checksum2;
		BlkInfo.Calchecksum = checksum;

		BlkInfo.ECC_Bit_Max = 0;
		BlkInfo.ECC_Bit_Total = 0;//TotalECCBit;
		if(BCH_MODE !=ECC_Type_8BitECC)
		{
		    BlkInfo.RecalaimVal = SET_READ_Reclaim/2;
		}
		else
		{
		    BlkInfo.RecalaimVal = SET_READ_Reclaim;
		}
		BlkInfo.BCHType = 8;//BCH8
		INIT_BOOT("brec data blk:%d,checksum:%x %x %3dkb ",iLoop-4,checksum,checksum2,
				Breccnt/2);
		INIT_BOOT("ECC:%2x,%2x,Reclaim:%d,%d\n",BlkInfo.ECC_Bit_Max,BlkInfo.ECC_Bit_Total,BlkInfo.RecalaimVal,
			BlkInfo.BCHType);
		
		MEMCPY(Buf+sizeof(struct BootBlk_Info)*iLoop,(void*)&BlkInfo,sizeof(struct BootBlk_Info));
		//if(iLoop ==4)
		//	Str_printf("",&BlkInfo,sizeof(struct BootBlk_Info));
             
    }

    result =0x00;
    return result;
}
#define Multi_FLAG "MultiSectorBoot" //size of 16 

INT32U Read_BOOTData(INT8U *Buf)
{
	INT16U iLoop,sublk;
	INT16U jLoop;
	INT16U checksum=0;
	INT16U  j=0x00;
	INT16U Breccnt;
	INT32U CurCheckSum;
	INT32U* FlashRdBuf ;
	INT16U *ptr;
	//INIT_BOOT("buffer addr:0x%x\n",Buf);
	struct BootBlk_Info  BlkInfo;
	INT16U checksum2;
	INT16U MaxECCBit,TotalECCBit;
    int mbrec_table_size = 512;
	
	MEMSET(Buf,0x00,8*sizeof(struct BootBlk_Info));
	
	for(iLoop =0x00;iLoop <4;iLoop++)
	{
		sublk =iLoop;
		MaxECCBit =0x00;
		TotalECCBit =0x00;
		PHY_BCH_CNT(BCH_CLEAR_VAL,0);
		
		brec_sector_read( sublk, 0x00, NAND_PAGE_CACHE);
#ifdef Mbrec_Support_BCH24
        brec_sector_read( sublk, 0x01, NAND_PAGE_CACHE+512);//added by jjf for bch24 mbrec
#endif       
		//@fish add 
		TotalECCBit += PHY_BCH_CNT(BCH_GET_VAL,0);
		if(MaxECCBit<PHY_BCH_CNT(BCH_GET_VAL,0))
		{
		    MaxECCBit = PHY_BCH_CNT(BCH_GET_VAL,0);		    
		}
		PHY_BCH_CNT(BCH_CLEAR_VAL,0);
		
		if(!strncmp(Multi_FLAG, NAND_PAGE_CACHE,16))
        {
            Breccnt = *(volatile unsigned int*)(NAND_PAGE_CACHE + 0x28);//must before createTbl()
#ifdef Mbrec_Support_BCH24
            Breccnt = Breccnt*2 + 2;
            mbrec_table_size = 1024;
#else
            Breccnt = Breccnt + 1;                     
            mbrec_table_size = 512;
#endif
        }
        else
        {
            Breccnt = 1;
            Breccnt = 1;                       
        }
		CurCheckSum = 0;
		FlashRdBuf= (INT32U*)NAND_PAGE_CACHE;
		for (jLoop = 0;jLoop < (mbrec_table_size)/4-1; jLoop++)
			CurCheckSum += * FlashRdBuf ++;

		CurCheckSum += 0x1234;
		//if(iLoop ==0x00)
		//	Str_printf("", NAND_PAGE_CACHE+496, 16);

		/////Breccnt =3;
		checksum =0x00;
        for(jLoop=mbrec_table_size/512;jLoop<Breccnt;jLoop++)
		{
		    
			brec_sector_read( sublk, jLoop, NAND_PAGE_CACHE);
			
			TotalECCBit += PHY_BCH_CNT(BCH_GET_VAL,0);
    		if(MaxECCBit<PHY_BCH_CNT(BCH_GET_VAL,0))
    		{
    		    MaxECCBit = PHY_BCH_CNT(BCH_GET_VAL,0);    		    
    		}
    		PHY_BCH_CNT(BCH_CLEAR_VAL,0);
		
		
			ptr =(INT16U*) (NAND_PAGE_CACHE);
			for( j=0; j<255; j++ )
			{
				checksum = checksum + (*ptr);
				ptr++;
			}
			if(jLoop != (Breccnt - 1) )     //Brec最后一个sector
			{
				checksum = checksum + (*ptr);
			}
		}
		checksum2 = *(INT16U*)(NAND_PAGE_CACHE+510);
		
		
	//	if(iLoop ==0x00)
	//		Str_printf("", NAND_PAGE_CACHE+496, 16);
		MEMSET(&BlkInfo,0x00,sizeof(struct BootBlk_Info));
		MEMCPY(BlkInfo.name,"mbrc",4);
		BlkInfo.blknum = iLoop;
		if(checksum2!=checksum)
			BlkInfo.check = 0x01;//CurCheckSum
		else
			BlkInfo.check =0x00;
		BlkInfo.Flag = *(INT16U*)(NAND_PAGE_CACHE+508);;
		BlkInfo.Flashchecksum = checksum2;
		BlkInfo.Calchecksum = checksum;
		
		BlkInfo.ECC_Bit_Max = 0;
		BlkInfo.ECC_Bit_Total = 0;// TotalECCBit;
		if(BCH_MODE !=ECC_Type_8BitECC)
		{
		    BlkInfo.RecalaimVal = SET_READ_Reclaim/2;
		}
		else
		{
		    BlkInfo.RecalaimVal = SET_READ_Reclaim;
		}
		BlkInfo.BCHType = 8;//BCH8
		
		///INIT_BOOT("mbrec table blk:%d, checksum:%x\n",iLoop,CurCheckSum);
		INIT_BOOT("mbrc data blk:%d,checksum:%x %x %3dkb ",iLoop,checksum,checksum2,Breccnt/2);
		INIT_BOOT("ECC:%2x,%2x,Reclaim:%d,%d\n",BlkInfo.ECC_Bit_Max,BlkInfo.ECC_Bit_Total,BlkInfo.RecalaimVal,
			BlkInfo.BCHType);
		MEMCPY(Buf+sizeof(struct BootBlk_Info)*iLoop,(void*)&BlkInfo,sizeof(struct BootBlk_Info));
		//if(iLoop ==0)
		//	Str_printf("",&BlkInfo,sizeof(struct BootBlk_Info));

	}

    if(0x00 == Read_Brec_code((NAND_BREC_SIZE/512),Buf))
    {
        return 0x0;
    }
    else
    {    
    	Read_Brec_code(128,Buf);
    	Read_Brec_code(160,Buf);
    	Read_Brec_code(256,Buf);
    	return 0x00;
	}	

 	
}
#if   1//FWSC_DRIVER ==1
INT32U Entry_Phy_ReadPage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
	INT32U bitmap;
	//INT8U Zone;
	struct PhysicOpParameter param;	
	struct NandSpareData  SData;
	INT32U ret;
	INT32U ZONENUM_PER_DIE,DieNo,SuBlkInDie;
	
	ZONENUM_PER_DIE =  ZONE_NUM_PER_DIE;
	DieNo = Block/BLK_CNT_PER_DIE;
	SuBlkInDie = Block % BLK_CNT_PER_DIE;
	bitmap= (1<<Sector)-1;
	FTL_CalPhyOpPar(&param, DieNo * ZONENUM_PER_DIE, SuBlkInDie, PageInSuBlk);

	param.SectorBitmapInPage = bitmap;
	param.MainDataPtr = Buf;
	param.SpareDataPtr = (INT8U *)&SData;
	
	INIT_BOOT("## %s,blk:%d,%d,bitmap:%x##\n",__func__,param.PhyBlkNumInBank,
		param.PageNum,param.SectorBitmapInPage);
	ret = PHY_PageRead(&param);
	if (PHY_READ_ERR(ret))
	{
		INIT_BOOT("[SCAN] %s: Read Err for Page %d of SuBlk %d in Die %d, ret %d\n",
			__FUNCTION__, PageInSuBlk, SuBlkInDie, DieNo, ret);
	}
	return 0x00;
	
}

INT32U Entry_Phy_WritePage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
	INT32U bitmap;
	///INT8U Zone;
	struct PhysicOpParameter param;	
	struct NandSpareData  SData;
	INT32U ret;
	INT32U ZONENUM_PER_DIE,DieNo,SuBlkInDie;
	
	ZONENUM_PER_DIE =  ZONE_NUM_PER_DIE;
	DieNo = Block/BLK_CNT_PER_DIE;
	SuBlkInDie = Block % BLK_CNT_PER_DIE;
	bitmap= (1<<Sector)-1;
	FTL_CalPhyOpPar(&param, DieNo * ZONENUM_PER_DIE, SuBlkInDie, PageInSuBlk);

	param.SectorBitmapInPage = bitmap;
	param.MainDataPtr = Buf;
	param.SpareDataPtr = (INT8U *)&SData;

	INIT_BOOT("## %s,blk:%d,%d,bitmap:%x##\n",__func__,param.PhyBlkNumInBank,
		param.PageNum,param.SectorBitmapInPage);
	
	ret = PHY_PageWrite(&param);
	ret = PHY_SyncNandOperation(param.BankNum);
	if (PHY_READ_ERR(ret))
	{
		INIT_BOOT("[SCAN] %s: Read Err for Page %d of SuBlk %d in Die %d, ret %d\n",
			__FUNCTION__, PageInSuBlk, SuBlkInDie, DieNo, ret);
	}
	return 0x00;
	
}

INT32U Entry_Log_ReadBlk(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
	
	FTL_Read(Block,Sector,Buf);//	(INT32U Lba, INT32U Length, void * SramBuffer)
	return 0x00;
	
}
INT32U Entry_Log_WriteBlk(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
	
	FTL_Write(Block,Sector,Buf);
	return 0x00;
	
}

INT32U Entry_Phy_EraseBlk(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
	INT32U bitmap;
	INT8U   tmpPlaneCntPerDie;      //the count of plane in one die
	INT8U   tmpSectorNumPerPage;    //page size,based on 0.5k
	INT16U  tmpOperationOpt;        //the operation support bitmap
	struct PhysicOpParameter param;	
	INT32U ret;
	INT32U ZONENUM_PER_DIE,DieNo,SuBlkInDie;
	
	tmpPlaneCntPerDie = NandStorageInfo.PlaneCntPerDie;
    tmpSectorNumPerPage = NandStorageInfo.SectorNumPerPage;
    tmpOperationOpt = NandStorageInfo.OperationOpt;

    /* reset nand flash driver parameter */

    NandStorageInfo.PlaneCntPerDie = 1;
    NandStorageInfo.SectorNumPerPage = 4;
    NandStorageInfo.OperationOpt &= ~(MULTI_PAGE_WRITE);
	
	ZONENUM_PER_DIE =  ZONE_NUM_PER_DIE;
	DieNo = Block/BLK_CNT_PER_DIE;
	SuBlkInDie = Block % BLK_CNT_PER_DIE;
	bitmap= (1<<Sector)-1;
	
	FTL_CalPhyOpPar(&param, DieNo * ZONENUM_PER_DIE, SuBlkInDie, 0);

	ret = PHY_EraseSuperBlk(&param);
	ret = PHY_SyncNandOperation(param.BankNum);
	
	/* restore nand flash driver parameter */
	NandStorageInfo.PlaneCntPerDie = tmpPlaneCntPerDie;
	NandStorageInfo.SectorNumPerPage = tmpSectorNumPerPage;
	NandStorageInfo.OperationOpt = tmpOperationOpt;
	
	return 0x00;
	
}

INT32U Entry_Phy_Readboot(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{	
	INT32U iLoop;
    INIT_BOOT("## %s,blk:%d,%d,sector:%x##\n",__func__,Block,PageInSuBlk,Sector);
	for(iLoop =0x00;iLoop<Sector;iLoop++)
	{
     	brec_sector_read(Block,iLoop+PageInSuBlk,(INT8U*) (Buf+iLoop*512));
       
	}
	return 0x00;
	
}

INT32U Entry_Phy_Writeboot(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{

	INT32U iLoop;
    INIT_BOOT("## %s,blk:%d,%d,sector:%x##\n",__func__,Block,PageInSuBlk,Sector);
	for(iLoop =0x00;iLoop<Sector;iLoop++)
	{
		brec_sector_write(Block,iLoop+PageInSuBlk,(INT8U*) (Buf+iLoop*512));
	}
	return 0x00;
	
}

INT32U  Entry_Phy_ReadWholePage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
    INT32U bitmap;
    struct PhysicOpParameter param;	
    struct NandSpareData  SData;
    INT32U ret;
    INT32U ZONENUM_PER_DIE,DieNo,SuBlkInDie;
    INT16U  tmpOperationOpt;        //the operation support bitmap
    
	
    ZONENUM_PER_DIE =  ZONE_NUM_PER_DIE;
    DieNo = Block/BLK_CNT_PER_DIE;
    SuBlkInDie = Block % BLK_CNT_PER_DIE;
    bitmap= (1<<Sector)-1;

     tmpOperationOpt = NandStorageInfo.OperationOpt;
    NandStorageInfo.OperationOpt &= ~(MULTI_PAGE_WRITE); 
    FTL_CalPhyOpPar(&param, DieNo * ZONENUM_PER_DIE, SuBlkInDie, PageInSuBlk);

    param.SectorBitmapInPage = bitmap;
    param.MainDataPtr = Buf;
    param.SpareDataPtr = (INT8U *)&SData;

    INIT_BOOT("## %s,blk:%d,%d,bitmap:%x##\n",__func__,param.PhyBlkNumInBank,
            param.PageNum,param.SectorBitmapInPage);
    ret = _PHY_WholePageRead(&param);
    if (PHY_READ_ERR(ret))
    {
        INIT_BOOT("[SCAN] %s: Read Err for Page %d of SuBlk %d in Die %d, ret %d\n",
                __FUNCTION__, PageInSuBlk, SuBlkInDie, DieNo, ret);
    }
    
    NandStorageInfo.OperationOpt = tmpOperationOpt;
    
    return 0x00;

    
}

INT32U  Entry_Phy_WriteWholePage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
    INT32U bitmap;
    struct PhysicOpParameter param;	
    struct NandSpareData  SData;
    INT32U ret;
    INT32U ZONENUM_PER_DIE,DieNo,SuBlkInDie;
    INT16U  tmpOperationOpt;        //the operation support bitmap
    
	
    ZONENUM_PER_DIE =  ZONE_NUM_PER_DIE;
    DieNo = Block/BLK_CNT_PER_DIE;
    SuBlkInDie = Block % BLK_CNT_PER_DIE;
    bitmap= (1<<Sector)-1;

     tmpOperationOpt = NandStorageInfo.OperationOpt;
    NandStorageInfo.OperationOpt &= ~(MULTI_PAGE_WRITE); 
    FTL_CalPhyOpPar(&param, DieNo * ZONENUM_PER_DIE, SuBlkInDie, PageInSuBlk);

    param.SectorBitmapInPage = bitmap;
    param.MainDataPtr = Buf;
    param.SpareDataPtr = (INT8U *)&SData;

    INIT_BOOT("## %s,blk:%d,%d,bitmap:%x##\n",__func__,param.PhyBlkNumInBank,
            param.PageNum,param.SectorBitmapInPage);
    ret = PHY_WholePageWrite(&param);
    if (PHY_READ_ERR(ret))
    {
        INIT_BOOT("[SCAN] %s: Read Err for Page %d of SuBlk %d in Die %d, ret %d\n",
                __FUNCTION__, PageInSuBlk, SuBlkInDie, DieNo, ret);
    }
    
    NandStorageInfo.OperationOpt = tmpOperationOpt;
    
    return 0x00;

    
}


INT32U   ReadWrite_Entry(INT8U entrynum, INT32S lba, INT8U* buffer,INT32U Pagetmp)
{
	INT32U pageCnt,SectorCnt;
	///printf("####log Lba:%x,%x,%x###\n",entrynum,lba,rw_sector_cnt_once);
	pageCnt = Pagetmp&0xFFFF;
	SectorCnt = (Pagetmp >>16) &0xFFFF;
	printf("\n####%s_NO:%x,lba:%x,Page:0x%x,Sect:0x%x,%x###\n",__func__,
		entrynum,lba,pageCnt,SectorCnt,Pagetmp);

	switch(entrynum)
	{	
		case GET_Block_Info://Get Flash information
			MEMSET(buffer,0x00,512);
			MEMCPY(buffer,(void*)&NandStorageInfo,sizeof(NandStorageInfo));
			Str_printf("", buffer, sizeof(NandStorageInfo));
			break;
		case PHY_ReadBlock:
			Entry_Phy_ReadPage(lba,pageCnt,SectorCnt,buffer);
			break;
		case PHY_WriteBlock:
			Entry_Phy_WritePage(lba,pageCnt,SectorCnt,buffer);
			break;
		case Log_ReadBlock:
			Entry_Log_ReadBlk(lba,pageCnt,SectorCnt,buffer);
			break;
		case Log_WriteBlock:
			Entry_Log_WriteBlk(lba,pageCnt,SectorCnt,buffer);
			break;
		case PHY_ReadBootBlk:
			Entry_Phy_Readboot(lba,pageCnt,SectorCnt,buffer);
			break;
		case PHY_WriteBootBlk:
			Entry_Phy_Writeboot(lba,pageCnt,SectorCnt,buffer);
            break;
		case PHY_EraseBlk:
			Entry_Phy_EraseBlk(lba,pageCnt,SectorCnt,buffer);
			break;
		case PHY_ReadWholePage:
			Entry_Phy_ReadWholePage(lba,pageCnt,SectorCnt,buffer);
			break;
		case PHY_WriteWholePage:
			Entry_Phy_WriteWholePage(lba,pageCnt,SectorCnt,buffer);
			break;
        case PHY_CHECK_BOOTDATA:
            Read_BOOTData(buffer);
            break;
		default:
			break;
	}	
	return 0x00;
	
}

#else
/*
*
*/
INT32U Entry_Phy_ReadPage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
    return 0x00;
}
/*
*
*/
INT32U Entry_Phy_WritePage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
    return 0x00;
}
/*
*
*/
INT32U Entry_Phy_EraseBlk(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf)
{
    return 0x00;
}
#endif

#if (defined(__KERNEL__)) 
EXPORT_SYMBOL(ReadWrite_Entry);
#endif












