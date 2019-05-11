/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : physic_layer.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-7-18 14:17
*********************************************************************************************************
*/

#include "nand_flash_driver_type.h"
#include "nand_flash_command-1201.h"
#include "nand_reg_def-1201.h"
#include "do_dma.h"
#if (!defined(__KERNEL__)) 
#include "mipsregs.h"
#include "regdef.h"
#else
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#endif

extern void _BusToNormalMode(void *MemoryAddr);
extern void _BusToSpecialMode(void *MemoryAddr);
extern void _NandDmaReadConfig(INT32U nDmaNum, void* Mem_Addr, INT32U ByteCount);
extern void _NandDmaWriteConfig(INT32U nDmaNum, void* Mem_Addr, INT32U ByteCount);
extern INT32U _NandReleaseDevice(void);
extern INT32U _NandGetDevice(INT32U ChipNum);
extern INT32U _RsEccCorrect(INT8U *MBuf, INT8U *SBuf);
extern INT32U _CalPhyBlkNumByBlkNumInBank(INT32U *ChipNum, INT32U *BlkNumInChip, INT32U BankNum, INT32U BlkNumInBank);
extern INT32S nand_flash_inithw(void);

extern void dma_end(INT32S dmanr);

#define MASK_COST_Write 0x30000000
#define MASK_COST_Read  0x30000000

#define MAX_LoopCnt   1000
extern INT32U NandDMAChannel;

static INT32U LastPhysicOpCmd = 0xffff;
extern INT8U bPrintFlag;
extern struct  _Debug_Wear  WearDbg;

INT32U PHYReadResult;
INT32U PHY_BchRes=0x00;
INT32U PHY_BchECCCnt=0x00;
INT32U ReadCap=0x00;
extern struct ECC_proetect_Read  Prot_Read;
INT32U bPageCnt=0x00;
INT32U bECCCnt =0x00;
INT32U wPseudo_Noise=0x00;


INT32U  PHY_EraseSuperBlk(struct PhysicOpParameter *PhyOpPar);
INT32U  _PHY_PageRead1KB(struct PhysicOpParameter *NandOpPar);
int MicronSetFeature(INT32U WriteData);

/**
 * \brief   Gets the check sum of data.
 *
 * This routine calculates the check sum of data.
 *
 * \param[in]  buf The Data buffer, two-bytes for every member
 * \param[in]  len The two-bytes length of buf.
 *
 * \return  The two-bytes checksum of data.
 */
INT32U bTestECCCnt;
INT32U bTotalECC=0x00;
INT32U bECCCount =0x00;

INT16U _GetCheckSum(INT16U *buf,INT32U len)
{
    INT32U loop;
    INT16U sum = 0;

    for(loop = 0; loop <len; loop++)
    {
        sum += buf[loop];
    }

    return sum;
}


INT32U _GetCheckSum32(INT32U *buf,INT32U len)
{
    INT32U loop;
    INT32U sum = 0;

    for(loop = 0; loop <len; loop++)
    {
        sum += buf[loop];
    }

    return sum;
}

#if 1//(defined(__KERNEL__))||(BREC_DRIVER == 0x01)
INT32U PHY_ReadRes(INT8U Flag,INT32U res)
{
    INT32U result;
    result = 0x0;
    switch(Flag)
    {
        case BCH_CLEAR_VAL:
            PHYReadResult = 0x00;
            break;
        case BCH_SET_VAL:
            PHYReadResult  |= res;
            break;
        case BCH_GET_VAL:
            result= PHYReadResult;
            break;
        default:
                break;                   
    }
    return result;
           
}
INT32U PHY_Set_BCHRes(INT32U bResult)
{
      PHY_BchRes  |= bResult;
      return 0x00;
}
INT32U PHY_Get_BCHRes(void)
{   
      INT32U result;
      result = PHY_BchRes;
      PHY_BchRes = 0x00;
      return result;
}
INT32U PHY_BCH_CNT(INT8U Flag,INT32U bTmp)
{     
    INT32U result;
    result =0x00;
    switch(Flag)
    {
        case BCH_CLEAR_VAL:
            PHY_BchECCCnt =0x00;
            break;
        case BCH_SET_VAL: //add value
            PHY_BchECCCnt += bTmp;                     
            break;
        case BCH_GET_VAL://Get value
            result=PHY_BchECCCnt;
            break;
        default:
            break;
    }
    return result;
       

}

#endif



INT32S Check_All_FF(const void *buf, INT32S size)
{
	INT32S i;

	for (i = 0; i <(INT32S)( size / sizeof(INT32U)); i++)
		if (((const INT32U *)buf)[i] != ~((INT32U)0))
			return 0;

	for (i = i; i < size; i++)
		if (((const INT8U *) buf)[i] != 0xFF)
			return 0;

	return 1;
}


INT8U _Get_Bitmap_index(INT32U Bitmap)
{
	INT32U iLoop;
	INT32U res;
	res =0xFF;
	for(iLoop =0x00;iLoop<32;iLoop++)
	{	
		if(Bitmap&(1<<iLoop))
		{
			res= iLoop;
			break;
		}
	}
	return res;
	
}
/**
check Bitmap: 
    for example: 
    0xFFCF, 0xFFDF,0xFFEF  
    return 0x02 
    
*/
INT8U _Get_Bitmap_type1KB(INT32U Bitmap)
{

    INT32U iLoop,jLoop;
    INT32U tmpMap,tempSectBitmap;
    INT32U Bitmap1KB,BufferRead;
    
    tempSectBitmap  = Bitmap;
    BufferRead =0x00;
    Bitmap1KB =0x00;
    for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2,jLoop++)
    {
        tmpMap =(tempSectBitmap>>iLoop)&0x03;	
    //    INIT_BOOT("tmpMap:%x,%x,%x,%x\n",iLoop,tmpMap,Bitmap1KB,BufferRead);
        if(tmpMap==3)
        {
            Bitmap1KB |=(1<<jLoop);
            if(BufferRead ==1)
            {
                BufferRead =0x02;
            }
        }
        //else if (tmpMap ==0x00 && Bitmap1KB !=0x00)
        else if( tmpMap>=0 &&  Bitmap1KB !=0x00)
        {
            BufferRead =0x01;
        }
    }
  //  INIT_BOOT("tmpMap:%x,%x,%x,%x\n",iLoop,tmpMap,Bitmap1KB,BufferRead);
    return BufferRead;	
}

INT8U _Get_Bitmap_type512B(INT32U Bitmap)
{

    INT32U iLoop,jLoop;
    INT32U tmpMap,tempSectBitmap;
    INT32U Bitmap1KB,BufferRead;
    
    tempSectBitmap  = Bitmap;
    BufferRead =0x00;
    Bitmap1KB = 0x00;
    for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=1,jLoop++)
    {
        tmpMap =(tempSectBitmap>>iLoop)&0x01;	
        ///INIT_BOOT("tmpMap:%x,%x,%x\n",iLoop,tmpMap,Bitmap1KB);
        if(tmpMap==1)
        {
            Bitmap1KB |=(1<<jLoop);
            if(BufferRead ==1)
            {
                BufferRead =0x02;
            }
        }
        //else if (tmpMap ==0x00 && Bitmap1KB !=0x00)
        else if( tmpMap==0 &&  Bitmap1KB !=0x00)
        {
            BufferRead =0x01;
        }
    }
    return BufferRead;	
}

void Print_msg(void)
{
	INT32U iLoop;
	INIT_BOOT("nand_ctl:0x%x\n",NAND_REG_READ(NAND_CTL));
	INIT_BOOT("eccctl:0x%x\n",NAND_REG_READ(NAND_ECCCTL));
	INIT_BOOT("status:0x%x\n",NAND_REG_READ(NAND_STATUS));
	INIT_BOOT("rbctl:0x%x\n",NAND_REG_READ(NAND_RB_CTL));
	INIT_BOOT("pagectl:0x%x\n",NAND_REG_READ(NAND_PAGE_CTL));
	INIT_BOOT("nand_clkctl:0x%x\n",NAND_REG_READ(NAND_CLKCTL));
	INIT_BOOT("nand_bitmap:0x%x\n",NAND_REG_READ(NAND_BIT_MAP));
	INIT_BOOT("PhyPagenum(ROWADDRLO):0x%x\n",NAND_REG_READ(NAND_ROWADDRLO));
	for(iLoop =0x00;iLoop<8;iLoop++)
	{
		INIT_BOOT("offset:0x%x,val:0x%x\n",(0xb0060100 +iLoop*4 + 0x20 * NandDMAChannel),REG_READ(0xb0060100 +iLoop*4 + 0x20 * NandDMAChannel));	
	}
}

void Get_bPageCntVal(void)
{
	INIT_BOOT("bPageCnt:%d bECCCnt:%d\n",bPageCnt,bECCCnt);
}

INT32U _Pseudo_NoiseFunc(INT32U Flag)
{
	wPseudo_Noise = Flag;
	return wPseudo_Noise;
}

INT32U _SetPseudo_Noise(INT8U Flag)
{
	
	if(!NAND_SUPPORT_PN)
	{
		return 0x00;	
	}
	else if(wPseudo_Noise==0x01) //diable Pseudo_Noise function 
	{
		return 0x00;	
	}
	
	if(Flag) // Set Pseudo noise enable
	{
		NAND_REG_WRITE(NAND_CTL, NAND_REG_READ(NAND_CTL)|(1<<31));}
	else
	{
		NAND_REG_WRITE(NAND_CTL, NAND_REG_READ(NAND_CTL)&(~(1<<31)));	
	}

	return 0x00;
}

/*
*********************************************************************************************************
*               PHYSIC LAYER SEND PAGE DATA
*
*Description: send one page data from ram to nand page regester.
*
*Arguments  : PageBitmap    the bitmap of the page;
*             MainDataPtr   the address of the buffer which stored the main area data;
*             SpareDataPtr  the address of the buffer which stored the userdata;
*
*Return     : TRUE              send page data ok;
*             FALSE             send page data failed;
*             FALSE|ERRORTYPE   send page data error;
*********************************************************************************************************
*/

#if CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

INT32U  _SendPageData1KB(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
	INT32S i,iLoop,jLoop;
	INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
	INT32U tempMainColAddr, tempSpareColAddr;
	INT32U TmpData;
	INT32U tmpUserData;   
	

	if (MainDataPtr == NULL)
	{
		/* the buffer address is invalid */
		printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
		return PHY_LAYER_BUF_ERR;
	}

	pMain = (INT8U*)MainDataPtr;
	pSpare = (INT8U*)SpareDataPtr;

	tempMainColAddr = 0;
	tempSpareColAddr = 0;
#if 0
	if(bPrintFlag ==0x01 )
	{
	///	INIT_BOOT("run to:### %s,%d#### \n",__func__,__LINE__);
		INIT_BOOT("Page:%x,Bitmap:%x,MainPtr:%x,SPtr:%x\n",PhyPageNum,PageBitmap,
			(INT32U)MainDataPtr,(INT32U)SpareDataPtr);
		INIT_BOOT("Page:%d, blk:%d \n",PhyPageNum%128,PhyPageNum/128);
	}
#endif
	/* send page data based on bitmap */
	for(i=0,iLoop=0; i<SECTOR_NUM_PER_SINGLE_PAGE; i+=2,iLoop++)
	{
		if(PageBitmap & (3<<i))
		{
			/* set spare data register */
			if(pSpare && (iLoop<SP_SECTOR_CNT))
			{
				              
				if(SP_USE_BYTE==2)
				{
					tmpUserData = (*pSpare) | (*(pSpare+1) << 8) | (0xff << 16) | (0xff << 24);
				}
				else
				{
					tmpUserData = (*pSpare) | (*(pSpare+1) << 8) | (*(pSpare+2) << 16) | (0xff << 24);
				}				
				pSpare += SP_USE_BYTE;
			}
			else
			{
				tmpUserData = 0xffffffff;
				
			}
			NAND_REG_WRITE(NAND_UDATA, tmpUserData);
			//INIT_BOOT("No:%x,%x\n",i,tmpUserData);
		#if   1//(defined(__KERNEL__))	
//			jLoop=0x00;
			while(1)
			{
				TmpData =NAND_REG_READ(NAND_UDATA);
				if((tmpUserData &0xFFFF) ==(TmpData&0xFFFF))
				{
					break;	
				}
				NAND_REG_WRITE(NAND_UDATA, tmpUserData);	
//				jLoop++;
//				if(0x00==(jLoop+1)%10)
//				{
//					INIT_BOOT("NO:%d %x,%x,%x\n",jLoop,iLoop,TmpData,tmpUserData);
//				}
			}
			
		#endif
			/*========================================================*/
			/***************** process main area data *****************/
			/*========================================================*/
			/* config dma for transfering data from ram to nand fifo */
			_NandDmaWriteConfig(NandDMAChannel, pMain, NAND_SECTOR_1KB);

			/* wait nand controller statemachine ready*/
			wait_sm_finish();
			//wait_CMD_ready();

			/* config nand controller register */
			NAND_REG_WRITE(NAND_BC, NAND_SECTOR_1KB);


			NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);

			/* send random write command */
			NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_WRITE_85 & (~NAND_CMD_SEQU));

			/* switch bus to special mode if use special dma */
			if(ISSPECIALDMA(NandDMAChannel))
			{
				_BusToSpecialMode(pMain);
			}

			/* start dma to transfer data, and wait transfer complete */
			start_dma(NandDMAChannel);
			dma_end(NandDMAChannel);

			/* wait nand controller statemachine to ready */
			wait_sm_finish();

			/* switch bus back to nomal mode */
			if(ISSPECIALDMA(NandDMAChannel))
			{
				_BusToNormalMode(pMain);
			}

			/*========================================================*/
			/***************** process spare area data ****************/
			/*========================================================*/


			//wait_CMD_ready();

			/* config spare area address */
			NAND_REG_WRITE(NAND_SPARE_COLADDR, SPARE_AREA_START_ADDR + tempSpareColAddr);

			/* send access spare area data command */
			NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SPARE_WRITE_85);
			wait_sm_finish();

			pMain += NAND_SECTOR_1KB;
		}

		/* increase the collumn address for nand */
		tempMainColAddr += NAND_SECTOR_1KB;    
//		tempSpareColAddr+=SPARE_SIZE_BCH24;
#if CONFIG_AM_CHIP_ID == 1220 

		if(BCH_MODE == ECC_Type_40BitECC )
			tempSpareColAddr +=  SPARE_SIZE_BCH40;
		else
			tempSpareColAddr +=  SPARE_SIZE_BCH24;	
#elif CONFIG_AM_CHIP_ID == 1213 
		if(BCH_MODE == ECC_Type_60BitECC )
			tempSpareColAddr +=  SPARE_SIZE_BCH60;
		else if(BCH_MODE == ECC_Type_40BitECC)
			tempSpareColAddr +=  SPARE_SIZE_BCH40;
		else
			tempSpareColAddr +=  SPARE_SIZE_BCH24;
#else
        tempSpareColAddr +=  SPARE_SIZE_BCH24;
#endif

	}

	return TRUE;
}

INT32U _Set_USerData(void *SpareDataPtr)
{
	INT8U iLoop,jLoop,ii,RightFlag;
	INT32U tmpUserData,TmpData,bCnt;
	INT32U UserDataArray[8];
	INT8U *pSpare;  
	INT32U bSector1KB;
	
	pSpare = SpareDataPtr;
	bCnt =0x00;
	
	if(BCH_MODE ==ECC_Type_12BitECC || BCH_MODE ==ECC_Type_24BitECC|| BCH_MODE ==ECC_Type_40BitECC ||BCH_MODE ==ECC_Type_60BitECC)
	{
		bSector1KB = SECTOR_NUM_PER_SINGLE_PAGE/2;
	}
	else
	{
		bSector1KB = SECTOR_NUM_PER_SINGLE_PAGE;	
	}
	
	for(iLoop=0x00; iLoop<bSector1KB; iLoop++)
	{
		if(iLoop<SP_SECTOR_CNT)
		{
			if(SP_USE_BYTE==2)
			{
				tmpUserData = (*pSpare) | (*(pSpare+1) << 8) | (0xff << 16) | (0xff << 24);
			}
			else
			{
				tmpUserData = (*pSpare) | (*(pSpare+1) << 8) | (*(pSpare+2) << 16) | (0xff << 24);
			}
			pSpare += SP_USE_BYTE;				
		}
		else
		{
			tmpUserData = 0xffffffff;
			
		}
		NAND_REG_WRITE(NAND_UDATA+iLoop*4, tmpUserData);		
		UserDataArray[iLoop] = tmpUserData;
	#if 1	
		for(jLoop=0x00;jLoop<=iLoop;jLoop++)
		{
			TmpData=NAND_REG_READ(NAND_UDATA+jLoop*4);
			if((UserDataArray[jLoop] &0xFFFF) !=(TmpData&0xFFFF))
			{				
				NAND_REG_WRITE(NAND_UDATA+jLoop*4, UserDataArray[jLoop]);
			//	INIT_BOOT("\n## Error %s  %d##\n",__func__,bCnt++);
				ii=0x00;
				while(1)
				{
					TmpData =NAND_REG_READ(NAND_UDATA+jLoop*4);
					ii++;
					if(0x00 ==(ii+1)%10)
					{
						INIT_BOOT("%xiLoop:%d,%x,%x\n",ii++,jLoop,TmpData,UserDataArray[iLoop]);
					}
					if((UserDataArray[jLoop] &0xFFFF) ==(TmpData&0xFFFF))					
					{
						break;	
					}
					NAND_REG_WRITE(NAND_UDATA+jLoop*4, UserDataArray[jLoop]);							
				}
				
			}
		}
	#endif	
	}
	
	for(iLoop=0x00; iLoop<bSector1KB; iLoop++)
	{
		TmpData=NAND_REG_READ(NAND_UDATA+iLoop*4);
		if((UserDataArray[iLoop] &0xFFFF) !=(TmpData&0xFFFF))
		{
			RightFlag=0x01;
			INIT_BOOT("iLoop:%d,%x,%x\n",iLoop,TmpData,UserDataArray[iLoop]);
		}
		///INIT_BOOT("iLoop:%d,%x,%x\n",iLoop,TmpData,UserDataArray[iLoop]);
	}
	
#if 0	
	bCnt =0x00;
	while(1)
	{
		RightFlag =0x00;
		for(iLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE/2; iLoop++)
		{
			TmpData=NAND_REG_READ(NAND_UDATA+iLoop*4);
			if((UserDataArray[iLoop] &0xFFFF) !=TmpData)
			{
				RightFlag=0x01;
				//INIT_BOOT("iLoop:%d,%x,%x\n",iLoop,TmpData,UserDataArray[iLoop]);
			}			
		}
		
		if(RightFlag ==0x00)
		{
			break;	
		}
		INIT_BOOT("## %s  NO:%d##\n",__func__,bCnt++);
		
		for(iLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE/2; iLoop++)
		{
			NAND_REG_WRITE(NAND_UDATA+iLoop*4, UserDataArray[iLoop]);							
		}
		
	}
#endif
	return 0x00;
	
}

INT32U  _SendPageData_hw(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
//	INT32S i,iLoop;
	INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
	INT32U tempMainColAddr, tempSpareColAddr;
///	INT32U tmpUserData;    
	INT32U TotalByte;
	

	if (MainDataPtr == NULL)
	{
		/* the buffer address is invalid */
		printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
		return PHY_LAYER_BUF_ERR;
	}

	pMain = (INT8U*)MainDataPtr;
	pSpare = (INT8U*)SpareDataPtr;

	tempMainColAddr = 0;
	tempSpareColAddr = 0;
	
	

#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;
#endif

	TotalByte =0x00;
	_Set_USerData(pSpare);	
	TotalByte=SECTOR_NUM_PER_SINGLE_PAGE*NAND_SECTOR_SIZE;
	

#if 0
	if(bPrintFlag ==0x01)
	{		
		INIT_BOOT("TotalByte:%d,PageBitmap:%x,PageNum:%x \n",TotalByte,
			PageBitmap,PhyPageNum);
		INIT_BOOT("ptr:0x%x,0x%x",(INT32U)MainDataPtr,(INT32U)SpareDataPtr);
	}
#endif

	_NandDmaWriteConfig(NandDMAChannel, pMain, TotalByte);

	/* wait nand flash status ready */
	wait_nand_ready();
	wait_sm_finish();
	NAND_REG_WRITE(NAND_BC, NAND_SECTOR_SIZE);
	NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
	NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);
	NAND_REG_WRITE(NAND_BIT_MAP, PageBitmap);
	
	switch(SECTOR_NUM_PER_SINGLE_PAGE)
	{
		case 0x04:
			NAND_REG_WRITE(NAND_PAGE_CTL, (0<<2)|(0<<1)|(1<<0));
			break;
		case 0x08://4//4K page	
			NAND_REG_WRITE(NAND_PAGE_CTL, (1<<2)|(0<<1)|(1<<0));
			break;
		case 0x10://8KB Page
			NAND_REG_WRITE(NAND_PAGE_CTL, (2<<2)|(0<<1)|(1<<0));
			break;
		default:
			break;
	}
	/* switch bus to special mode if use special dma */
	if(ISSPECIALDMA(NandDMAChannel))
	{
		_BusToSpecialMode(pMain);
	}

	start_dma(NandDMAChannel);	
	//dma_end(NandDMAChannel);	

	/* wait nand controller statemachine to ready */
	wait_sm_finish();

	/* switch bus back to nomal mode */
	if(ISSPECIALDMA(NandDMAChannel))
	{
		_BusToNormalMode(pMain);
	}
	
	
	return TRUE;
}

INT32U  _SendPageData1KB_hw(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
	INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
	INT32U tempMainColAddr;
 
	INT32U TotalByte;	

	if (MainDataPtr == NULL)
	{
		/* the buffer address is invalid */
		printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
		return PHY_LAYER_BUF_ERR;
	}

	pMain = (INT8U*)MainDataPtr;
	pSpare = (INT8U*)SpareDataPtr;

	tempMainColAddr = 0;
	
	/* wait nand flash status ready */
	wait_nand_ready();
	wait_sm_finish();
	NAND_REG_WRITE(NAND_BC, NAND_SECTOR_1KB);
	
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;
#endif

	TotalByte =0x00;
  
	_Set_USerData(pSpare);	
	TotalByte=SECTOR_NUM_PER_SINGLE_PAGE*NAND_SECTOR_SIZE;
	

#if 0
	if(bPrintFlag ==0x01)
	{	
		INIT_BOOT("TotalByte:%d,PageBitmap:%x,PhyPageNum:%x\n",TotalByte,PageBitmap,PhyPageNum);
	}
#endif	

	_NandDmaWriteConfig(NandDMAChannel, pMain, TotalByte);
	

	wait_sm_finish();
	
	NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
	NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);
	NAND_REG_WRITE(NAND_BIT_MAP, PageBitmap);
	
#if  0//(defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode =FLAH_EIP_IRQ;
	flash_irq_enable();
#endif	
	/* switch bus to special mode if use special dma */
	if(ISSPECIALDMA(NandDMAChannel))
	{
		_BusToSpecialMode(pMain);
	}
	
	switch(SECTOR_NUM_PER_SINGLE_PAGE)
	{
		case 0x08://4//4K page
		case 0x04: // 2 K page 
			NAND_REG_WRITE(NAND_PAGE_CTL, (1<<2)|(0<<1)|(1<<0));
			break;
		case 0x10://8KB Page
			NAND_REG_WRITE(NAND_PAGE_CTL, (2<<2)|(0<<1)|(1<<0));
			break;
		default:
			break;
	}

	
	start_dma(NandDMAChannel);
#if 0// (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)	
	if(0x01==Flash_timeout())
	{		
		flash_clr_status();	
		NAND_REG_WRITE(NAND_CTL,NAND_REG_READ(NAND_CTL)|(1<<1));	
		///result = PHY_LAYER_TIMEOUT_ERR;
		Print_msg();
	
	}	
#elif (defined(__KERNEL__)) && (DMA_IRQ_MODE ==0x01)	
#else
	dma_end(NandDMAChannel);
#endif
	//dma_end(NandDMAChannel);

	/* switch bus back to nomal mode */
	if(ISSPECIALDMA(NandDMAChannel))
	{
		_BusToNormalMode(pMain);
	}

	/* wait nand controller statemachine to ready */
	wait_sm_finish();
	
	return TRUE;
}
#endif


INT32U  _SendPageData(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
    INT32S i;
    INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
    INT32U tempMainColAddr, tempSpareColAddr;
    INT32U tmpUserData,jLoop,TmpData;

    if (MainDataPtr == NULL)
    {
        /* the buffer address is invalid */
        printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
        return PHY_LAYER_BUF_ERR;
    }

    pMain = (INT8U*)MainDataPtr;
    pSpare = (INT8U*)SpareDataPtr;

    tempMainColAddr = 0;
    tempSpareColAddr = 0;
#if (SUPPORT_SMALL_BLOCK)
    if (NAND_IS_SMALL_BLOCK)
    {
        /* get the real physical page number of small block */
        PhyPageNum = PhyPageNum * SECTOR_NUM_PER_SINGLE_PAGE;
    }
#endif

	/* send page data based on bitmap */
	for(i=0; i<SECTOR_NUM_PER_SINGLE_PAGE; i++)
	{
		if(PageBitmap & (1<<i))
		{
			/* set spare data register */
			if(pSpare && (i<SP_SECTOR_CNT))
			{
				if(SP_USE_BYTE==2)
					tmpUserData = (*pSpare) | (*(pSpare+1) << 8) | (0xff << 16) | (0xff << 24);      
				else
					tmpUserData = (*pSpare) | (*(pSpare+1) << 8) | (*(pSpare+2) << 16) | (0xff << 24);
			//	NAND_REG_WRITE(NAND_UDATA, tmpUserData);
				pSpare += SP_USE_BYTE;
			}
			else
			{
				tmpUserData = 0xffffffff;
				
			}
			NAND_REG_WRITE(NAND_UDATA, tmpUserData);	
#if   1//(defined(__KERNEL__))	
			jLoop=0x00;
			while(1)
			{
				TmpData =NAND_REG_READ(NAND_UDATA);
				if((tmpUserData &0xFFFF) ==(TmpData&0xFFFF))
				{
					break;	
				}
				NAND_REG_WRITE(NAND_UDATA, tmpUserData);		
				//INIT_BOOT("NO:%d %x,%x,%x\n",jLoop++,iLoop,TmpData,tmpUserData);
			}
#endif

			//NAND_REG_WRITE(NAND_BUF1, 0xffffffff);

			/*========================================================*/
			/***************** process main area data *****************/
			/*========================================================*/
			/* config dma for transfering data from ram to nand fifo */
			_NandDmaWriteConfig(NandDMAChannel, pMain, NAND_SECTOR_SIZE);

			/* wait nand controller statemachine ready*/
			wait_sm_finish();
			//wait_CMD_ready();

			/* config nand controller register */
			NAND_REG_WRITE(NAND_BC, NAND_SECTOR_SIZE);

#if (SUPPORT_SMALL_BLOCK)
			if (NAND_IS_SMALL_BLOCK)
			{
				NAND_REG_WRITE(NAND_ROWADDRLO, (PhyPageNum + i) << 0x8);
				NAND_REG_WRITE(NAND_ROWADDRHI, 0x0);

				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SB_WMAIN_POINT_00 | NAND_CMD_SEQU); // 0x80010000
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SB_WRITE_80 | NAND_CMD_SEQU); // 0x804b0080
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SB_WRITE_SPARE | NAND_CMD_SEQU); //0x810a0000
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SB_WRITE_CONFIRM_10); //0x00010010
			}
			else
			{
#endif
				NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);

				/* send random write command */
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_WRITE_85 & (~NAND_CMD_SEQU));

#if (SUPPORT_SMALL_BLOCK)
			}   /* if (!NAND_IS_SMALL_BLOCK) */
#endif

			/* switch bus to special mode if use special dma */
			if(ISSPECIALDMA(NandDMAChannel))
			{
				_BusToSpecialMode(pMain);
			}

			/* start dma to transfer data, and wait transfer complete */
			start_dma(NandDMAChannel);
			dma_end(NandDMAChannel);

			/* wait nand controller statemachine to ready */
			wait_sm_finish();

			/* switch bus back to nomal mode */
			if(ISSPECIALDMA(NandDMAChannel))
			{
				_BusToNormalMode(pMain);
			}

			/*========================================================*/
			/***************** process spare area data ****************/
			/*========================================================*/

#if (SUPPORT_SMALL_BLOCK)
			if (NAND_IS_SMALL_BLOCK)
			{
				wait_nand_ready();
			}
			else
			{
#endif
				//wait_CMD_ready();

				/* config spare area address */
				NAND_REG_WRITE(NAND_SPARE_COLADDR, SPARE_AREA_START_ADDR + tempSpareColAddr);

				/* send access spare area data command */
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SPARE_WRITE_85);
				wait_sm_finish();

#if (SUPPORT_SMALL_BLOCK)
			}   /* if (!NAND_IS_SMALL_BLOCK) */
#endif

			pMain += NAND_SECTOR_SIZE;
		}

		/* increase the collumn address for nand */
		tempMainColAddr += NAND_SECTOR_SIZE;    
		if(BCH_MODE == ECC_Type_12BitECC) 
		{
			tempSpareColAddr  += SPARE_SIZE_BCH12;	
		}
		else
		{
			tempSpareColAddr  += SPARE_SIZE_BCH8;	
		}
			
		//if(NAND_REG_READ(NAND_ECCCTL) &(1<<8) ) //12 Bit ECC
		//	tempSpareColAddr +=  SPARE_SIZE_PER_SECTOR_12BitECC ;
		//else
		//	tempSpareColAddr += SPARE_SIZE_PER_SECTOR;

	}

	return TRUE;
}

/*
*********************************************************************************************************
*               PHYSIC LAYER GET PAGE DATA
*
*Description: get one page data from nand page regester to ram.
*
*Arguments  : PageBitmap    the bitmap of the page;
*             MainDataPtr   the address of the buffer which stored the main area data;
*             SpareDataPtr  the address of the buffer which stored the userdata;
*
*Return     : TRUE              get page data ok;
*             FALSE             get page data failed;
*             FALSE|ERRORTYPE   get page data error;
*********************************************************************************************************
*/

#if CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

INT32U  _GetPageData1KB(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
	INT32S i,iLoop;
	INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
	INT32U tempMainColAddr, tempSpareColAddr;
	INT32U result;
	INT8U tempSpareBuf[8];
	INT32U SPData;
	INT32U BitMap;
	INT8U *Buf;

	if(MainDataPtr == NULL)
	{
		/* the buffer address is invalid */
		printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
		return PHY_LAYER_BUF_ERR;
	}
#if 0		
	if(bPrintFlag ==0x01)
	{		
		INIT_BOOT("\n## %s Page:%x,Bitmap:%x,MainPtr:%x,SPtr:%x\n",__func__,PhyPageNum,PageBitmap,
				(INT32U)MainDataPtr,(INT32U)SpareDataPtr);
	}
#endif

	pMain = (INT8U*)MainDataPtr;
	pSpare = (INT8U*)SpareDataPtr;

	result = 0;
	tempMainColAddr = 0;
	tempSpareColAddr = 0;

	for(i=0,iLoop=0x00; i<SECTOR_NUM_PER_SINGLE_PAGE; i+=2,iLoop++)
	{         
		if(( PageBitmap & (3<<i) ) ||(pSpare && (iLoop< SP_SECTOR_CNT)))
		{
			/*========================================================*/
			/***************** process main area data *****************/
			/*========================================================*/
			/* config dma to transfer data from nand fifo to ram */
			BitMap = (PageBitmap>>i &0x03);
			if(BitMap ==0x03)
				Buf = pMain;
			else
				Buf = PHY_TMPBUF;
            
		#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
			Flash_DMA_Mode =FLAH_EIP_IRQ;
		#endif		
			_NandDmaReadConfig(NandDMAChannel, Buf, NAND_SECTOR_1KB);
			

			/* wait nand flash status ready */
			wait_nand_ready();
			/* wait nand controller statemachine ready*/
			wait_sm_finish();                

			/* config nand controller register */
			NAND_REG_WRITE(NAND_BC, NAND_SECTOR_1KB);

			/*========================================================*/
			/***************** process spare area data ****************/
			/*========================================================*/

			/* set address to access spare area */                  
			NAND_REG_WRITE(NAND_SPARE_COLADDR, SPARE_AREA_START_ADDR + tempSpareColAddr);	 

			NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);	 

			/* send command to access spare area data first.*/
			NAND_REG_WRITE(NAND_CMD_FSM,NAND_CMD_SPARE_READ_05);
			NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SPARE_READ_E0);
			wait_sm_finish();

			/* send nand command to get data */
			NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_05);
			NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_E0);


			/* switch bus to special mode if use special dma */
			if(ISSPECIALDMA(NandDMAChannel))
			{
				_BusToSpecialMode(pMain);
			}
			
		#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)		
			Flash_DMA_Mode =FLAH_EIP_IRQ;
			flash_irq_enable();
			start_dma(NandDMAChannel);
			if(0x01==Flash_timeout())
			{		
				INIT_BOOT("####run to :%s,%d Total read :%d KB#### \n",__func__,__LINE__,ReadCap);				
			}
			wait_sm_finish();

		#else
			/* start dma to transfer data, and wait transfer complete */
			start_dma(NandDMAChannel);
			#if 0
			dma_end(NandDMAChannel);	
			#endif 
			/* wait nand controller state machine ready */
			wait_sm_finish();
		#endif
			/* switch bus back to nomal mode */
			if (ISSPECIALDMA(NandDMAChannel))
			{
				_BusToNormalMode(pMain);
			}
#if  CONFIG_AM_CHIP_ID == 1213
			dma_cache_wback_inv((INT32U)((INT32U)Buf & ~MASK_COST_Read),NAND_SECTOR_1KB);
#endif
			if(pSpare && (iLoop< SP_SECTOR_CNT))
			{
				/* copy spare data to spare data buffer */
				*(INT32U*)tempSpareBuf =SPData= NAND_REG_READ(NAND_UDATA);
				*pSpare = tempSpareBuf[0];
				*(pSpare+1) = tempSpareBuf[1];
				if(SP_USE_BYTE==3)
					*(pSpare+2) = tempSpareBuf[2];

				result = result | bch_ecc_correct(Buf,pSpare);   /* check if any data error,if so,correct it    */
				pSpare += SP_USE_BYTE;     
			}
			else
			{
				SPData= NAND_REG_READ(NAND_UDATA);
				result = result | bch_ecc_correct(Buf,NULL);   /* check if any data error,if so,correct it    */  

			}
			BitMap = (PageBitmap>>i &0x03);
			switch(BitMap)
			{
				case 0x01:
					MEMCPY(pMain,PHY_TMPBUF,512);
					break;
				case 0x2:
					MEMCPY(pMain+512,PHY_TMPBUF+512,512);
					break;
				default:
					break;
			}


		}
		/* increase buffer address and the collumn address for nand flash */                
		pMain += NAND_SECTOR_1KB;
		tempMainColAddr += NAND_SECTOR_1KB;
#if CONFIG_AM_CHIP_ID == 1220 || (CONFIG_AM_CHIP_ID == 1213)
		if(BCH_MODE == ECC_Type_40BitECC )
			tempSpareColAddr +=  SPARE_SIZE_BCH40;
		else
			tempSpareColAddr +=  SPARE_SIZE_BCH24;	
#elif CONFIG_AM_CHIP_ID == 1213 
				if(BCH_MODE == ECC_Type_60BitECC )
					tempSpareColAddr +=  SPARE_SIZE_BCH60;
				else if(BCH_MODE == ECC_Type_40BitECC)
					tempSpareColAddr +=  SPARE_SIZE_BCH40;
				else
					tempSpareColAddr +=  SPARE_SIZE_BCH24;

#else
		tempSpareColAddr +=  SPARE_SIZE_BCH24;
#endif
	}
#if 0
	if(bPrintFlag==1)
	{
		INIT_BOOT("Delay time\n");
		///_Short_DelayUS(1000*200);
		
		NAND_REG_WRITE(NAND_ECCCTL, 0);//
		_NandDmaReadConfig(NandDMAChannel, PHY_TMPBUF, 8192+376);

		/* wait nand flash status ready */
		wait_nand_ready();
		/* wait nand controller statemachine ready*/
		wait_sm_finish();       
		NAND_REG_WRITE(NAND_BC, 8568);
		NAND_REG_WRITE(NAND_SPARE_COLADDR, 0x00 );	 
		//   printf("SPARE_AREA_START_ADDR + tempSpareColAddr:%X\n",SPARE_AREA_START_ADDR + tempSpareColAddr);

		NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x00);	 

		/* send command to access spare area data first.*/
		//NAND_REG_WRITE(NAND_CMD_FSM,NAND_CMD_SPARE_READ_05);
		//NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SPARE_READ_E0);
		///wait_sm_finish();

		/* send nand command to get data */
		NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_05);
		NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_E0);
		start_dma(NandDMAChannel);
		
		 /* wait nand controller state machine ready */
                   wait_sm_finish();
	#if  1//__KERNEL__	 
		 Str_printf2("",PHY_TMPBUF , 8192);
		 for(i=0x00;i<SECTOR_NUM_PER_SINGLE_PAGE/2;i++)
		 {
		 	INIT_BOOT("offset:%d\n",8192+44*i);
		 	Str_printf("",PHY_TMPBUF+8192+44*i , 44);
		 }
		#if 0	 
		 INIT_BOOT("main data\n");
		 for(i=0x00;i<8192;i++)
		 {
		 	INIT_BOOT("0x%x\n",*(INT8U*)(PHY_TMPBUF+i));	
		 }
		 INIT_BOOT("Spare Data \n");
		  for(i=0x00;i<376;i++)
		 {
		 	INIT_BOOT("%x\n",*(INT8U*)(PHY_TMPBUF+i+8192));	
		 }
		#endif		  
	#endif
		 Set_NAND_ECCCTL( NandStorageInfo.OperationOpt &(0xF000),0x01);
	}
 #endif 

 #if 0
	if(pSpare)
	{
		pMain = (INT8U*)MainDataPtr;
		pSpare = (INT8U*)SpareDataPtr;
		
		tempMainColAddr = 0;
		tempSpareColAddr = 0;
		PageBitmap =FULL_BITMAP_OF_SINGLE_PAGE;
		for(i=0; i<SP_SECTOR_CNT; i++)
		{ 

			if(PageBitmap )
			{
				/*========================================================*/
				/***************** process main area data *****************/
				/*========================================================*/
				/* config dma to transfer data from nand fifo to ram */  

				Buf = PHY_TMPBUF;
				_NandDmaReadConfig(NandDMAChannel, Buf, NAND_SECTOR_1KB);

				/* wait nand flash status ready */
				wait_nand_ready();
				/* wait nand controller statemachine ready*/
				wait_sm_finish();                

				/* config nand controller register */
				NAND_REG_WRITE(NAND_BC, NAND_SECTOR_1KB);

				/*========================================================*/
				/***************** process spare area data ****************/
				/*========================================================*/

				/* set address to access spare area */                          
				NAND_REG_WRITE(NAND_SPARE_COLADDR, SPARE_AREA_START_ADDR + tempSpareColAddr);	 

				NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);	 

				/* send command to access spare area data first.*/
				NAND_REG_WRITE(NAND_CMD_FSM,NAND_CMD_SPARE_READ_05);
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SPARE_READ_E0);
				wait_sm_finish();

				/* send nand command to get data */
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_05);
				NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_E0);                            

				/* start dma to transfer data, and wait transfer complete */
				start_dma(NandDMAChannel);

				/* wait nand controller state machine ready */
				wait_sm_finish();    

				if(pSpare && (i < SP_SECTOR_CNT))
				{
					/* copy spare data to spare data buffer */
					*(INT32U*)tempSpareBuf =SPData= NAND_REG_READ(NAND_UDATA);
					*pSpare = tempSpareBuf[0];
					*(pSpare+1) = tempSpareBuf[1];
					if(SP_USE_BYTE ==3)
						*(pSpare+2) = tempSpareBuf[2];

					result = result | bch_ecc_correct(Buf,pSpare);   /* check if any data error,if so,correct it    */
					pSpare += SP_USE_BYTE;                                
				}


			}

			/* increase buffer address and the collumn address for nand flash */  
			tempMainColAddr += NAND_SECTOR_1KB;
			tempSpareColAddr +=  SPARE_SIZE_BCH24;
		}

	}
#endif

#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;
//	flash_irq_disable();
#endif	


    return result;
}




INT32U  _GetPageData_hw(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
	INT32S i,iLoop,ReadCnt;
	INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
	INT32U tempMainColAddr;
	INT32U result;
	INT8U tempSpareBuf[8];
	INT32U SPData;
	INT32U TotalByte;
	INT32S error_cn ;
	volatile INT32U  ii;

	if(MainDataPtr == NULL)
	{
		/* the buffer address is invalid */
		printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
		return PHY_LAYER_BUF_ERR;
	}
	ReadCnt =0x00;
	
	pMain = (INT8U*)MainDataPtr;
	pSpare = (INT8U*)SpareDataPtr;
	ii =0x00;
	result = 0;
	tempMainColAddr = 0;
	TotalByte =0x00;	

	for(i=0; i<SECTOR_NUM_PER_SINGLE_PAGE; i++)
	{ 
		if(PageBitmap & (1<<i))
		{
			TotalByte+=NAND_SECTOR_SIZE;
		}	
	}
#if 0    
	if(bPrintFlag ==0x01)
	{		
		ECC_Err("\n\nTotalByte:%d,PageBitmap:%x,PhyPageNum:%x %x,%x\n",TotalByte,PageBitmap,
			 PhyPageNum,(INT32U)MainDataPtr,(INT32U)SpareDataPtr);
	}
#endif	

	_NandDmaReadConfig(NandDMAChannel, pMain, TotalByte);
#if 0  
	if(bPrintFlag ==0x01)
	{
		ECC_Err("before pagectl:%x,nandctl:0x%x,eectl:0x%x\n",NAND_REG_READ(NAND_PAGE_CTL),
		NAND_REG_READ(NAND_CTL),NAND_REG_READ(NAND_ECCCTL));
	}
 #endif   
	/* wait nand flash status ready */
	wait_nand_ready();
	/* wait nand controller statemachine ready*/
	wait_sm_finish();  
	
	NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);	
	/* set address to access spare area */                  
	//NAND_REG_WRITE(NAND_SPARE_COLADDR, SPARE_AREA_START_ADDR + tempSpareColAddr);	 

	NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);	

	/* config nand controller register */
	NAND_REG_WRITE(NAND_BC, NAND_SECTOR_SIZE);
	NAND_REG_WRITE(NAND_BIT_MAP, PageBitmap);
	
#if (defined(__KERNEL__))
	flash_clr_status();
	ReadCap+=(TotalByte/1024);
#endif



#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode =FLAH_EIP_IRQ;
	flash_clr_status();
	flash_irq_enable();
#endif
	/* switch bus to special mode if use special dma */
	if(ISSPECIALDMA(NandDMAChannel))
	{
		_BusToSpecialMode(pMain);
	}
	
	switch(SECTOR_NUM_PER_SINGLE_PAGE)
	{
		case 0x04://4 /////2K page
			NAND_REG_WRITE(NAND_PAGE_CTL, (0<<2)|(1<<1)|(0<<0));
			break;
		case 0x08: ///// 4K page 
			NAND_REG_WRITE(NAND_PAGE_CTL, (1<<2)|(1<<1)|(0<<0));
			break;
		case 0x10:///// 4K page 
			NAND_REG_WRITE(NAND_PAGE_CTL, (2<<2)|(1<<1)|(0<<0));
			break;
		default:
			break;
	}
	
	/* start dma to transfer data, and wait transfer complete */
	start_dma(NandDMAChannel);		
	
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)	
	if(0x01==Flash_timeout())
	{		
		INIT_BOOT("@@@Start nand flash reset register####\n");
		flash_clr_status();	
		NAND_REG_WRITE(NAND_CTL,NAND_REG_READ(NAND_CTL)|(1<<1));	
		result = PHY_LAYER_TIMEOUT_ERR;
		Print_msg();
		//if(ReadCnt<2)
		///	goto ResetRead;
	}	
#endif
	

	/* wait nand controller state machine ready */
	wait_sm_finish();
	/* switch bus back to nomal mode */
	if (ISSPECIALDMA(NandDMAChannel))
	{
		_BusToNormalMode(pMain);
	}
	iLoop =0x00;		
	for(i=0; i<SECTOR_NUM_PER_SINGLE_PAGE; i++)
	{ 
		if(PageBitmap & (1<<i))
		{
			*(INT32U*)tempSpareBuf =SPData= NAND_REG_READ(NAND_UDATA+iLoop*4);
			if(pSpare && i<SP_SECTOR_CNT)
			{
				*pSpare = tempSpareBuf[0];
				*(pSpare+1) = tempSpareBuf[1];
				pSpare += SP_USE_BYTE;   
			}
			error_cn =(SPData>>24)&0x1f;
			if(error_cn==0x1f)
			{
				INIT_BOOT("\n\n####### %s, OVERECC %d Bit#######\n",__func__,error_cn);	
				INIT_BOOT("i:0x%x,Spa:0x%x,%x\n",i,SPData,NAND_UDATA+iLoop*4 );
				Debug_Flash_Register();
			}
			else if (error_cn>6)
			{
				INIT_BOOT("\n\n########%s  ECC %d Bit#########\n",__func__,error_cn);	
				INIT_BOOT("i:0x%x,Spa:0x%x,%x\n",i,SPData,NAND_UDATA+iLoop*4 );
				result |= NAND_DATA_ECC_LIMIT;
				Debug_Flash_Register();	
			}
			//if(bPrintFlag ==0x01)	  
			 //	ECC_Err("i:0x%x,Spa:0x%x,%x error_cn:%x\n",i,SPData,NAND_UDATA+iLoop*4 ,error_cn);
			  iLoop++;
		}
	}

		

	
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;
#endif
	return result;
}



INT32U  _GetPageData1KB_hw(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
	INT32S i,iLoop,ReadCnt;
	INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
	INT32U tempMainColAddr;
	INT32U result;
	INT8U tempSpareBuf[8];
	INT32U SPData;
	INT32U TotalByte;
	INT32S error_cn ;
	volatile INT32U  ii;

	if(MainDataPtr == NULL)
	{
		/* the buffer address is invalid */
		printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
		return PHY_LAYER_BUF_ERR;
	}
	ReadCnt =0x00;
//ResetRead:	
	pMain = (INT8U*)MainDataPtr;
	pSpare = (INT8U*)SpareDataPtr;

	result = 0;
	tempMainColAddr = 0;
	TotalByte =0x00;	
	ii =0x00;
	for(i=0; i<SECTOR_NUM_PER_SINGLE_PAGE/2; i++)
	{ 
		if(PageBitmap & (1<<i))
		{
			TotalByte+=NAND_SECTOR_1KB;
		}
		
	}


	
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode =FLAH_EIP_IRQ;
#elif (defined(__KERNEL__)) && (DMA_IRQ_MODE ==0x01)	
	
#endif
	_NandDmaReadConfig(NandDMAChannel, pMain, TotalByte);

	/* wait nand flash status ready */
	wait_nand_ready();
	/* wait nand controller statemachine ready*/
	wait_sm_finish();  
	
	NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);	
	/* set address to access spare area */                  

	NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);	

	/* config nand controller register */
	NAND_REG_WRITE(NAND_BC, NAND_SECTOR_1KB);
	NAND_REG_WRITE(NAND_BIT_MAP, PageBitmap);
	
#if (defined(__KERNEL__))
	flash_clr_status();
	ReadCap+=(TotalByte/1024);
#endif

#if TEST_RB_CTL	==0x01
	NAND_REG_WRITE(NAND_RB_CTL, (1<<31)|(0x3000));
#endif

#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode =FLAH_EIP_IRQ;
	flash_irq_enable();
#endif

	/* switch bus to special mode if use special dma */
	if(ISSPECIALDMA(NandDMAChannel))
	{
		_BusToSpecialMode(pMain);
	}
	switch(SECTOR_NUM_PER_SINGLE_PAGE)
	{
		case 0x08://4 //4K page
		case 0x04:// 2K page 
			NAND_REG_WRITE(NAND_PAGE_CTL, (1<<2)|(1<<1)|(0<<0));
			break;
		case 0x10://8KB Page
			NAND_REG_WRITE(NAND_PAGE_CTL, (2<<2)|(1<<1)|(0<<0));
			break;
		default:
			break;
	}
	
	/* start dma to transfer data, and wait transfer complete */
	start_dma(NandDMAChannel);
	
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)	
	if(0x01==Flash_timeout())
	{		
	//	INIT_BOOT("####run to :%s,%d Total read :%d KB#### \n",__func__,__LINE__,ReadCap);
		flash_clr_status();	
		NAND_REG_WRITE(NAND_CTL,NAND_REG_READ(NAND_CTL)|(1<<1));	
		result = PHY_LAYER_TIMEOUT_ERR;
		Print_msg();
		//if(ReadCnt<2)
		///	goto ResetRead;
	}	
#elif (defined(__KERNEL__)) && (DMA_IRQ_MODE ==0x01)	
#endif
	


	/* wait nand controller state machine ready */
	wait_sm_finish();
	/* switch bus back to nomal mode */
	if (ISSPECIALDMA(NandDMAChannel))
	{
		_BusToNormalMode(pMain);
	}
	iLoop =0x00;		
	for(i=0; i<SECTOR_NUM_PER_SINGLE_PAGE/2; i++)
	{ 
		if(PageBitmap & (1<<i))
		{
			*(INT32U*)tempSpareBuf =SPData= NAND_REG_READ(NAND_UDATA+iLoop*4);
			if(pSpare && i<SP_SECTOR_CNT)
			{
				*pSpare = tempSpareBuf[0];
				*(pSpare+1) = tempSpareBuf[1];
				pSpare += SP_USE_BYTE;   
			}
			error_cn =(SPData>>24)&0x1f;
			if(error_cn==0x1f)
			{
				INIT_BOOT("\n\n####### %s, OVERECC %d Bit#######\n",__func__,error_cn);	
				INIT_BOOT("i:0x%x,Spa:0x%x,%x\n",i,SPData,NAND_UDATA+iLoop*4 );
				Debug_Flash_Register();
			}
			else if (error_cn>12)
			{
				INIT_BOOT("\n\n########%s  ECC %d Bit#########\n",__func__,error_cn);	
				INIT_BOOT("i:0x%x,Spa:0x%x,%x\n",i,SPData,NAND_UDATA+iLoop*4 );
				result |= NAND_DATA_ECC_LIMIT;
				Debug_Flash_Register();	
			}
        #if 0
            if(error_cn>=0x01)
            {
                INIT_BOOT("i:0x%x,error_cn,%d,Spa:0x%x,%x\n",i,error_cn,SPData,NAND_UDATA+iLoop*4 );
                Debug_Flash_Register();
            }
            if(bTestECCCnt<error_cn)
            {
                bTestECCCnt = error_cn;
            }
        #endif
      
			  iLoop++;
		}
	}

    
#if Total_Page_READERR   ==0x1
    bTotalECC += bTestECCCnt;
    if(bTestECCCnt>=1)
    {   
        bECCCount++;
        if(0x00==((bECCCount+1) %50))
            INIT_BOOT("Page:%x,%d,ECC:%d,%d,%d\n",PageBitmap,PhyPageNum,bTestECCCnt,bTotalECC,bECCCount);
    }
  #endif
  
    //  dma_cache_wback_inv(pMain, TotalByte);
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;
#endif
	return result;
}

#endif
INT32U  _Send_WholePageData(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
   // INT32S i;
    INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
    INT32U tempMainColAddr, tempSpareColAddr;
    INT16U      wholeSize;
 
    if (MainDataPtr == NULL)
    {
        /* the buffer address is invalid */
        printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
        return PHY_LAYER_BUF_ERR;
    }

    pMain = (INT8U*)MainDataPtr;
    pSpare = (INT8U*)SpareDataPtr;

    tempMainColAddr = 0;
    tempSpareColAddr = 0;
    
     wholeSize = PAGE_SPARESIZE  +NandStorageInfo.SectorNumPerPage*512;
    ///INIT_BOOT("Delay time,%d \n",wholeSize);

    _NandDmaWriteConfig(NandDMAChannel, pMain, wholeSize);

    /* wait nand controller statemachine ready*/
    wait_sm_finish(); 
    
    /* config nand controller register */
    NAND_REG_WRITE(NAND_BC, wholeSize);

    NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);

    /* send random write command */
    NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_WRITE_85 & (~NAND_CMD_SEQU));
    /* switch bus to special mode if use special dma */
    if(ISSPECIALDMA(NandDMAChannel))
    {
        _BusToSpecialMode(pMain);
    }

    /* start dma to transfer data, and wait transfer complete */
    start_dma(NandDMAChannel);
    dma_end(NandDMAChannel);

    /* wait nand controller statemachine to ready */
    wait_sm_finish();

    /* switch bus back to nomal mode */
    if(ISSPECIALDMA(NandDMAChannel))
    {
        _BusToNormalMode(pMain);
    }
    
    return TRUE;
}

INT32U  _GetWholePageData(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
   //// INT32S i,iLoop;
    INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
    INT32U tempMainColAddr, tempSpareColAddr;
    INT32U result;
    INT16U      wholeSize;

    if(MainDataPtr == NULL)
    {
        /* the buffer address is invalid */
        printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
        return PHY_LAYER_BUF_ERR;
    }

    pMain = (INT8U*)MainDataPtr;
    pSpare = (INT8U*)SpareDataPtr;

    result = 0;
    tempMainColAddr = 0;
    tempSpareColAddr = 0;      
    
 //   NAND_REG_WRITE(NAND_ECCCTL, 0);//

    wholeSize = PAGE_SPARESIZE  +NandStorageInfo.SectorNumPerPage*512;
    INIT_BOOT("Delay time,%d \n",wholeSize);
    _NandDmaReadConfig(NandDMAChannel, MainDataPtr, wholeSize);

    /* wait nand flash status ready */
    wait_nand_ready();
    /* wait nand controller statemachine ready*/
    wait_sm_finish();       
    NAND_REG_WRITE(NAND_BC, wholeSize);
    NAND_REG_WRITE(NAND_SPARE_COLADDR, 0x00 );	 
    //   printf("SPARE_AREA_START_ADDR + tempSpareColAddr:%X\n",SPARE_AREA_START_ADDR + tempSpareColAddr);

    NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x00);	 

    /* send nand command to get data */
    NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_05);
    NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_E0);
    start_dma(NandDMAChannel);
	dma_end(NandDMAChannel);
    /* wait nand controller state machine ready */
    wait_sm_finish();
 
   Str_printf("",MainDataPtr , wholeSize);
#if 0	    
    for(i=0x00;i<SECTOR_NUM_PER_SINGLE_PAGE/2;i++)
    {
        INIT_BOOT("offset:%d\n",8192+44*i);
        Str_printf("",PHY_TMPBUF+8192+44*i , 44);
    }

    INIT_BOOT("main data\n");
    for(i=0x00;i<8192;i++)
    {
    INIT_BOOT("0x%x\n",*(INT8U*)(PHY_TMPBUF+i));	
    }
    INIT_BOOT("Spare Data \n");
    for(i=0x00;i<376;i++)
    {
        INIT_BOOT("%x\n",*(INT8U*)(PHY_TMPBUF+i+8192));	
    }
#endif	

 //   Set_NAND_ECCCTL( NandStorageInfo.OperationOpt &(0xF000),0x01);   

#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;    
#endif	


    return result;
}

INT32U  _GetPageData(INT32U PhyPageNum, INT32U PageBitmap, void *MainDataPtr, void *SpareDataPtr)
{
    INT32S i;
    INT8U *pMain, *pSpare;      /* the pointer to the main data buffer and spare data buffer */
    INT32U tempMainColAddr, tempSpareColAddr;
    INT32U result;
    INT8U tempSpareBuf[8];
    INT32U SPData;

    if(MainDataPtr == NULL)
    {
        /* the buffer address is invalid */
        printf("PHY_DBG: memory address is invalid!, MainDataPtr: 0x%x\n", (INT32U)MainDataPtr);
        return PHY_LAYER_BUF_ERR;
    }

    pMain = (INT8U*)MainDataPtr;
    pSpare = (INT8U*)SpareDataPtr;

    result = 0;
    tempMainColAddr = 0;
    tempSpareColAddr = 0;

#if (SUPPORT_SMALL_BLOCK)
    if (NAND_IS_SMALL_BLOCK)
    {
        /* get the real physical page number of small block */
        PhyPageNum = PhyPageNum * SECTOR_NUM_PER_SINGLE_PAGE;
    }
#endif

    for(i=0; i<SECTOR_NUM_PER_SINGLE_PAGE; i++)
    {

        if(PageBitmap & (1<<i))
        {
            /*========================================================*/
            /***************** process main area data *****************/
            /*========================================================*/
            /* config dma to transfer data from nand fifo to ram */

            _NandDmaReadConfig(NandDMAChannel, pMain, NAND_SECTOR_SIZE);

            /* wait nand flash status ready */
            wait_nand_ready();
            /* wait nand controller statemachine ready*/
            wait_sm_finish();
            //wait_CMD_ready();

            /* config nand controller register */
            NAND_REG_WRITE(NAND_BC, NAND_SECTOR_SIZE);

            /*========================================================*/
            /***************** process spare area data ****************/
            /*========================================================*/
            //wait_CMD_ready();
#if (SUPPORT_SMALL_BLOCK)
            if (NAND_IS_SMALL_BLOCK)
            {
                NAND_REG_WRITE(NAND_ROWADDRLO, (PhyPageNum + i) << 0x8);
                NAND_REG_WRITE(NAND_ROWADDRHI, 0x0);

                /* send nand command to get data */
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SB_READ_SPARE_50); //0x014d0050

                wait_sm_finish();
                wait_nand_ready();

                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SB_READ_MAIN_00); //0x004d0000
            }
            else
            {
#endif

                /* set address to access spare area */
                //NAND_REG_WRITE(NAND_ADDRLO1234,(NAND_REG_READ(NAND_ADDRLO1234) & ~0xffff) | (SPARE_AREA_START_ADDR + tempSpareColAddr));
                NAND_REG_WRITE(NAND_SPARE_COLADDR, SPARE_AREA_START_ADDR + tempSpareColAddr);	 

                //NAND_REG_WRITE(NAND_ADDRLO1234,(NAND_REG_READ(NAND_ADDRLO1234) & ~0xffff) | tempMainColAddr);
                NAND_REG_WRITE(NAND_MAIN_COLADDR, tempMainColAddr);	 

                /* send command to access spare area data first.*/
                NAND_REG_WRITE(NAND_CMD_FSM,NAND_CMD_SPARE_READ_05);
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_SPARE_READ_E0);
                wait_sm_finish();

                /* send nand command to get data */
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_05);
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RANDOM_READ_E0);

#if (SUPPORT_SMALL_BLOCK)
            }   /* if (NAND_IS_SMALL_BLOCK) */
#endif

            /* switch bus to special mode if use special dma */
            if(ISSPECIALDMA(NandDMAChannel))
            {
                _BusToSpecialMode(pMain);
            }

#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)		
            Flash_DMA_Mode =FLAH_EIP_IRQ;
            flash_irq_enable();
            start_dma(NandDMAChannel);
            if(0x01==Flash_timeout())
            {		
                INIT_BOOT("####run to :%s,%d Total read :%d KB#### \n",__func__,__LINE__,ReadCap);				
            }
            wait_sm_finish();			
#else			
            /* start dma to transfer data, and wait transfer complete */
            start_dma(NandDMAChannel);
#if 0
            dma_end(NandDMAChannel);	
#endif 
            /* wait nand controller state machine ready */
            wait_sm_finish();

#endif
            /* switch bus back to nomal mode */
            if (ISSPECIALDMA(NandDMAChannel))
            {
                _BusToNormalMode(pMain);
            }

#if (SUPPORT_SMALL_BLOCK)
            if (NAND_IS_SMALL_BLOCK)
            {
                /* some small block flash has a R/B switch after read */
                wait_nand_ready();
            }
            else
            {
#endif

#if (SUPPORT_SMALL_BLOCK)
            }   /* if (NAND_IS_SMALL_BLOCK) */
#endif
#if  CONFIG_AM_CHIP_ID == 1213			
			dma_cache_wback_inv((INT32U)((INT32U)pMain & ~MASK_COST_Read), NAND_SECTOR_SIZE);
#endif

            if(pSpare && (i < SP_SECTOR_CNT))
            {
                /* copy spare data to spare data buffer */
                *(INT32U*)tempSpareBuf =SPData= NAND_REG_READ(NAND_UDATA);
                *pSpare = tempSpareBuf[0];
                *(pSpare+1) = tempSpareBuf[1];
                if(SP_USE_BYTE==3)
                    *(pSpare+2) = tempSpareBuf[2];

                result = result | bch_ecc_correct(pMain,pSpare);   /* check if any data error,if so,correct it    */
                pSpare += SP_USE_BYTE;                          

            }
            else
            {
                SPData= NAND_REG_READ(NAND_UDATA);
                result = result | bch_ecc_correct(pMain,NULL);   /* check if any data error,if so,correct it    */             

            }
            /*	if(bPrintFlag ==1)
            {
            Str_printf("ph \n", pMain, 256);	
            }
            */	


        }

        /* increase buffer address and the collumn address for nand flash */
        pMain += NAND_SECTOR_SIZE;
        tempMainColAddr += NAND_SECTOR_SIZE;
        if(BCH_MODE == ECC_Type_12BitECC)
        {
            tempSpareColAddr +=  SPARE_SIZE_BCH12;
        }
        else
        {
            tempSpareColAddr +=  SPARE_SIZE_BCH8;
        }
    }
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)
	Flash_DMA_Mode = Flash_DMA_Loop;
	//flash_irq_disable();
#endif	
    return result;
}


/*
*********************************************************************************************************
*               PHYSIC LAYER NAND FLASH ID NUMBER
*
*Description: read the id number of the selected chip.
*
*Arguments  : ChipNum       the chip number which nand flash's id need read;
*             NandChipID    the pointer to where the nand flash id number will be stored;
*
*Return     : TRUE              read nand flash id ok;
*             FALSE             read nand flash id failed;
*             FALSE|ERRORTYPE   read nand flash id error;
*********************************************************************************************************
*/
INT32U  PHY_ReadNandId(INT32U ChipNum, void *NandChipID)
{
    INT8U *tmpPtr;
    INT32U result,chipid_tmp;

    PHY_DBG("PHY_DBG: Read nand flash chip id, chip number is:0x%x\n",ChipNum);

    tmpPtr = NandChipID;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
//    printf("NAND_CTL:%x,\n",NAND_REG_READ(NAND_CTL));

    /* wait nand command register ready, set nand address and byte counter */
    wait_nand_ready();
    wait_sm_finish();
    NAND_REG_WRITE(NAND_BC, 8);
    NAND_REG_WRITE(NAND_ROWADDRLO, 0);

    /* send the command to read chip id, and waite nand fifo ready */
    NAND_REG_WRITE(NAND_CMD_FSM,NAND_CMD_READID_90);
    //wait_fifo_ready();
    wait_fifo_no_empt();                                 /* for mips access,must waite nand fifo ready  */
    chipid_tmp= NAND_REG_READ(NAND_DATA); /* transfer data with 32bit type by mips        */
    NAND_REG_READ(NAND_DATA); 
    /* get the chip id from nand fifo */
    {
        INT32S i;
        for(i=0; i<4; i++)
        {
            //*tmpPtr ++ = NAND_REG_READ_B(NAND_FIFODATA);
     	    *tmpPtr ++ = chipid_tmp % 0x100;
	    chipid_tmp >>= 8;			
        }
    }

    /* wait nand controller statemachine ready */
    //wait_sm_finish();

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    PHY_DBG("PHY_DBG: Read nand chip id completely, chip id is:0x%x\n",*(tmpPtr-1)<<24 | *(tmpPtr-2)<<16 | *(tmpPtr-3)<<8 |*(tmpPtr-4));

    return TRUE;
}

/*
*********************************************************************************************************
*               PHYSIC LAYER RESET NAND FLASH
*
*Description: reset the nand flash.
*
*Arguments  : ChipNum       the chip number which nand flash need reset;
*
*Return     : TRUE              nand flash reset is ok;
*             FALSE             nand flash reset is failed;
*             FALSE|ERRORTYPE   nand flash reset error;
*********************************************************************************************************
*/
INT32U  PHY_NandReset(INT32U ChipNum)
{
    INT32U result;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* wait command register ready, send the nand reset command to nand flash */
    //wait_CMD_ready();
    wait_sm_finish();
    NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_RESET_FF);
    wait_sm_finish();

    wait_nand_ready();

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    return TRUE;
}

/*
*********************************************************************************************************
*               PHYSIC LAYER READ NAND FLASH STATUS
*
*Description: read nand flash status.
*
*Arguments  : ReadStatusCmd the command which is used to read nand flash status;
*
*Return     : NandStatus    the nand flash status get from nand flash;
*
*Note       : to read nand status,some command need not send address cycle,ie.0x70/0x71 ;
*             some command need send 1 address cycle, but nand flash will ignore it, ie.0xf1/0xf2;
*             some command need send 3 address cycle, to select some bank, ie. 0x78;
*
*********************************************************************************************************
*/
INT32U  _ReadNandStatus(INT32U ReadStatusCmd, INT32U BankInChip)
{
    INT32U tmpNandStatus;
    INT32U tmpReadStatusCmd;
    INT32U tmpNandAddress;

    //wait_CMD_ready();
    
    NAND_REG_WRITE(NAND_BC, 1);

    tmpReadStatusCmd = NAND_CMD_STATUS_70 & ~0xff;

    if(!(ReadStatusCmd == 0x70 || ReadStatusCmd == 0x71))
    {
        /* not 0x70 or 0x71, need send some address cycle */
        if(ReadStatusCmd == 0x78)
        {
            tmpReadStatusCmd |= NAND_CMD_ROW_ADDRCYCLE_3;
            tmpNandAddress = (BankInChip * BLK_CNT_PER_DIE) * PAGE_CNT_PER_PHY_BLK;
        }
        else
        {
            tmpReadStatusCmd |= NAND_CMD_ROW_ADDRCYCLE_0;
            tmpNandAddress = 0x00;
        }

        /* set nand flash address */
         //NAND_REG_WRITE(NAND_ROWADDRLO,tmpNandAddress);
         NAND_REG_WRITE(NAND_MAIN_COLADDR,tmpNandAddress);
    }

    /* set read nand status command */
    tmpReadStatusCmd |= ReadStatusCmd;
    /* send read status command */
    NAND_REG_WRITE(NAND_CMD_FSM, tmpReadStatusCmd);

    //wait_fifo_ready();
    wait_fifo_no_empt();

    tmpNandStatus = NAND_REG_READ(NAND_DATA);
    wait_sm_finish();

	tmpNandStatus &= 0xff;
	
    if(tmpNandStatus ==  ReadStatusCmd)
    {
        tmpNandStatus = 0xff;
    }

    return tmpNandStatus;
}



/*
*********************************************************************************************************
*               WAIT NAND FLASH TRUE READY
*
*Description: wait nand flash true ready.
*
*Arguments  : BankNum       the bank number which need to check status;
*
*Return     : result
*               TRUE    nand flash true ready;
*               FALSE   nand flash can't be ready;
*
*Note       : this function check the nand flash status by read nand flash status,check if
*             the nand status is ready, if the nand flash support cache program,it need to
*             check if the nand flash is true ready.
*********************************************************************************************************
*/
INT32U  PHY_WaitNandFlashReady(INT32U BankNum)
{
    INT32U tmpChipNum;
    INT32U result;
    tmpChipNum = BankNum / NandStorageInfo.BankCntPerChip;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(tmpChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* wait nand flash status ready */
    wait_nand_ready();

    /* check if the nand flash status is ready */
    while(!(result & NAND_STATUS_READY))
    {
        result = _ReadNandStatus(0x70, 0x00);
    }

    /* if the nand flash support cache program, need check if nand is true ready */
    if(NAND_SUPPORT_CACHE_PROGRAM)
    {
        while(!(result & NAND_CACHE_READY))
        {
            result = _ReadNandStatus(0x70, 0x00);
        }
    }

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();
    result = TRUE;

    return result;
}

/*
*********************************************************************************************************
*               WAIT NAND FLASH BANK READY
*
*Description: wait nand flash true ready.
*
*Arguments  : BankNum       the bank number which need to check status;
*
*Return     : result
*               TRUE    nand flash true ready;
*               FALSE   nand flash can't be ready;
*
*Note       : this function check the nand flash status by read nand flash status,check if
*             the nand status is ready, if the nand flash support cache program,it need to
*             check if the nand flash is true ready.
*********************************************************************************************************
*/
void  _WaitNandBnkReady(INT32U BankNumInChip)
{
    INT32U tmpReadStatusCmd;
    INT32U result = 0;


    /* check if nand flash is doing interleave operation */
    if(NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        if(!BankNumInChip)
        {
            /* the status need read is bank0 */
            tmpReadStatusCmd = NandDevInfo.SpecialCommand->InterChip0StatusCmd;
        }
        else
        {
            /* the staus need read is bank1 */
            tmpReadStatusCmd = NandDevInfo.SpecialCommand->InterChip1StatusCmd;
        }
    }
    else
    {
        tmpReadStatusCmd = 0x70;
    }

    /* read nand status to check if nand is ready or busy */
    while(!(result & NAND_STATUS_READY))
    {

        /* warning: interleave maybe not valid, but interleave feature is not enabled by now. 
         *   lw 2008-05-27 
         */

        /* some flash use read status to check if r/b is ready, the program timing
           will be error !! K9G4G08U0M for example */

//        result = _ReadNandStatus(tmpReadStatusCmd, BankNumInChip);
        /* wait 2 micro-seconds, to wait nand flash ready */
//        udlay(2);
        wait_nand_ready();
        result = 0xc0;
    }

    return;
}


/*
*********************************************************************************************************
*               PHYSIC LAYER CHECK NAND WRITE PROTECT STATUS
*
*Description: check if the nand flash is in the write protected status.
*
*Arguments  : ChipNum       the chip number which nand flash is checked;
*
*Return     : TRUE      the nand flash is not write protected;
*             FALSE     the nand flash is write protected;
*********************************************************************************************************
*/
INT32U  PHY_CheckWriteProtect(INT32U ChipNum)
{
    INT32U result;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    result = _ReadNandStatus(0x70, 0x00);

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    if(result & NAND_WRITE_PROTECT)
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}



/*
*********************************************************************************************************
*               PHYSIC LAYER SYNC NAND FLASH OPERATION
*
*Description: this function check the status of the selected bank, send the program cmd,if needed.
*
*Arguments  : BankNum       the interchip number whose status need be read;
*
*Return     : the result
*               TRUE    - sync nand ok
*               FALSE   - sync nand failed
*********************************************************************************************************
*/
INT32U PHY_SyncNandOperation(INT32U BankNum)
{
    INT32U tmpChipNum;
    INT32U tmpBankNum;
    INT32U tmpReadStatusCmd;
    INT32U result;
	//INT32U ra_reg;
	//ra_reg = GET_CPU_REG(ra);
    tmpChipNum = BankNum / NandStorageInfo.BankCntPerChip;
    tmpBankNum = BankNum % NandStorageInfo.BankCntPerChip;

    PHY_DBG("PHY_DBG: synch nand operation, chipnum:0x%x, banknum:0x%x!\n", tmpChipNum, tmpBankNum);

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(tmpChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* check if need send page program verify command */
    if(LastPhysicOpCmd == 0x80)
    {
        /* send nand program command */
        //wait_CMD_ready();
        PHY_ERR("Warning : %s %d\n",__func__,__LINE__);
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_10);
        wait_sm_finish();

        /* clear last operation command */
        LastPhysicOpCmd = 0xffff;
    }

    /* check if nand flash is doing interleave operation */
    if(NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        if(!tmpBankNum)
        {
            /* the status need read is bank0 */
            tmpReadStatusCmd = NandDevInfo.SpecialCommand->InterChip0StatusCmd;
        }
        else
        {
            /* the staus need read is bank1 */
            tmpReadStatusCmd = NandDevInfo.SpecialCommand->InterChip1StatusCmd;
        }
    }
    else
    {
        tmpReadStatusCmd = 0x70;
    }

    /* read nand status to check if nand is ready */
    while(!(result & NAND_STATUS_READY))
    {
        result = _ReadNandStatus(tmpReadStatusCmd, tmpBankNum);
    }

    /* if nand flash support cache program, need check if nand is true ready */
    if(NAND_SUPPORT_CACHE_PROGRAM)
    {
        while(!(result & NAND_CACHE_READY))
        {
            result = _ReadNandStatus(tmpReadStatusCmd, tmpBankNum);
        }
    }

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    if(result & NAND_OPERATE_FAIL)
    {
        /* nand operation failed */
        PHY_ERR("\nPHY_DBG: nand physic operate failed, when sync nand operation, status 0x%x !\n", result);
        result = NAND_OP_ERROR;
    }
    else
    {
        result = TRUE;
    }

    return result;
}


#if CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

INT32U  _PHY_PageRead_HW1KB(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */

	INT32U result,res;
	INT32U PhyPageNum;
	INT8S   *tempMainPtr, *tempSparePtr;
	INT32U tempSectBitmap;
	INT32U iLoop,tmpMap,Bitmap1KB,jLoop;
	INT8S * TmpPtr,*DstPtr;
	INT32U Bitmap_tmp;
	INT32U H_Bitmap1KB,H_Bitmap;
	INT32U T_Bitmap1KB,T_Bitmap,M_Bitmap;
	INT8U Index;
	INT8U BufferRead;
    char *TmpBuf,*SPptr,*TmpBuf2;
        
	TmpBuf=SPptr=TmpBuf2 =NULL;
    bTestECCCnt =0x00;
    PHY_DBG("\nPHY_DBG: Physic page read.\n");
  /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

	_SetPseudo_Noise(1); //Enable Pseudo noise 
	
    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;

    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempSectBitmap = NandOpPar->SectorBitmapInPage;
    
#if 0
    if(bPrintFlag==0x01)
	{
		  INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x, Page:%x,Blk:%x\n",__func__,PhyPageNum,tempSectBitmap,
		  	    NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		  INIT_BOOT("tempMainPtr:0x%x tempSparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,
                             (INT32U)tempSparePtr, SECTOR_NUM_PER_SINGLE_PAGE);
	}
  #endif      
    /* calculate the  main area data pointer */
    {
        INT32S i = 0;
        while(!(tempSectBitmap & (1<<i)))
        {
            i++;
        }
        tempMainPtr -= NAND_SECTOR_SIZE * i;
    }
	tempSectBitmap = NandOpPar->SectorBitmapInPage;
_PROCESS_PAGE_READ_DATA:

	/* wait nand flash status ready */
	wait_nand_ready();
	Bitmap1KB=0x00;
	BufferRead =0x00;

    BufferRead= _Get_Bitmap_type1KB(tempSectBitmap);
  #if 0      
	for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2,jLoop++)
	{
		tmpMap =(tempSectBitmap>>iLoop)&0x03;	
		///INIT_BOOT("tmpMap:%x,%x,%x\n",iLoop,tmpMap,Bitmap1KB);
		if(tmpMap==3)
		{
			Bitmap1KB |=(1<<jLoop);
			if(BufferRead ==1)
			{
				BufferRead =0x02;	
				///INIT_BOOT("tempMainPtr:0x%x tempSparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr,tempSectBitmap);
				//break;
			}
		}
		//else if (tmpMap ==0x00 && Bitmap1KB !=0x00)
                else 
		{
			BufferRead =0x01;
			//bPrintFlag =0x02;			
		}
		
	}	
#endif
	if(((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE ) && tempSparePtr  )	
		||(BufferRead==2) )
	{
		Bitmap1KB=0x00;
		for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2,jLoop++)
		{
			tmpMap =(tempSectBitmap>>iLoop)&0x03;		
			if(tmpMap)
			{
				Bitmap1KB |=(1<<jLoop);
			}			
			
		}
		if(tempSparePtr && Bitmap1KB<7)
		{
			Bitmap1KB |=0x07;
		}	
	
		/* get page data of the first plane */
		 if((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE) ==FULL_BITMAP_OF_SINGLE_PAGE) 
		{
			TmpPtr = tempMainPtr;	
		}
		else
		{
			TmpPtr = PHY_TMPBUF;	
			
		}
		tmpMap =Bitmap1KB ;
       
		/* get page data of the first plane */
		result |= _GetPageData1KB_hw(PhyPageNum, tmpMap & FULL_BITMAP_OF_SINGLE_PAGE, TmpPtr, tempSparePtr);
		//memcpy buffer 
		
		if((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)!=FULL_BITMAP_OF_SINGLE_PAGE)
		{
			DstPtr = tempMainPtr;
            for(iLoop=0; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2)
            {		        
                if(tempSectBitmap & (3<<iLoop))
                {
                    Bitmap_tmp = (tempSectBitmap>>iLoop &0x03);
                    switch(Bitmap_tmp)
                    {
                        case 0x01:
                            MEMCPY(DstPtr,TmpPtr,512);
                            break;
                        case 0x02:
                            MEMCPY(DstPtr+512,TmpPtr+512,512);
                            break;
                        case 0x03:
                            MEMCPY(DstPtr,TmpPtr,1024);
                            break;
                    }
                    TmpPtr+=NAND_SECTOR_1KB;					
                }

                DstPtr+=NAND_SECTOR_1KB;	

            }
			
		}
	}   
	else if(tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
	{		
		H_Bitmap1KB = H_Bitmap =0x00;
		T_Bitmap1KB = T_Bitmap =0x00;
		Bitmap1KB = M_Bitmap =0x00;
		for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2,jLoop++)
		{
			tmpMap =(tempSectBitmap>>iLoop)&0x03;		
			if(tmpMap==3)
			{
				if(Bitmap1KB==0x00)
				{
					M_Bitmap = 1<<iLoop;	
				}
				Bitmap1KB |=(1<<jLoop);				
			}
			else if(tmpMap !=0)
			{
				if(Bitmap1KB ==0 &&H_Bitmap1KB ==0x00 )
				{
					H_Bitmap1KB =(1<<jLoop);
					if(tmpMap==1)
						H_Bitmap= 1<<iLoop;
					else
						H_Bitmap= 1<<(iLoop+1);
				}				
				else if((H_Bitmap !=0x00) || (Bitmap1KB!=0x00))
				{
					T_Bitmap1KB =(1<<jLoop);
					if(tmpMap==1)
						T_Bitmap= 1<<iLoop;
					else
						T_Bitmap= 1<<(iLoop+1);
				}
			}
		}
	#if 0	
		if(bPrintFlag==0x01)	
		{
			INIT_BOOT("H_Bitmap1KB:%x,%x ,%x,%x\n",H_Bitmap1KB,H_Bitmap,M_Bitmap,Bitmap1KB);
			INIT_BOOT("T_Bitmap1KB:%x,%x\n",T_Bitmap1KB,T_Bitmap);
		}
	#endif	
		if(H_Bitmap1KB)
		{			
			tmpMap = H_Bitmap1KB;			
			TmpPtr = PHY_TMPBUF;
			Index= _Get_Bitmap_index(H_Bitmap);
			if(bPrintFlag==0x01)
			{
				INIT_BOOT("Head index: %x,buffer:%x ,%d ,%x\n",Index,(INT32U)(tempMainPtr+Index*512),Index%2,(INT32U)tempMainPtr );
			}
			result |= _GetPageData1KB_hw(PhyPageNum, tmpMap & FULL_BITMAP_OF_SINGLE_PAGE, TmpPtr, tempSparePtr);
			
			MEMCPY(tempMainPtr+Index*512,TmpPtr+(Index%2)*512,512);
			
		}
		if(Bitmap1KB)
		{
			tmpMap = Bitmap1KB;			
			Index= _Get_Bitmap_index(M_Bitmap);
			TmpPtr = tempMainPtr+Index*512;
			if(bPrintFlag==0x01)
			{
				ECC_Err("index: %x,buffer:%x ,%d ,%x\n",Index,(INT32U)(tempMainPtr+Index*512),Index%2,(INT32U)tempMainPtr );
			}
			
			res= _GetPageData1KB_hw(PhyPageNum, tmpMap & FULL_BITMAP_OF_SINGLE_PAGE, TmpPtr, tempSparePtr);
			if(res==PHY_LAYER_TIMEOUT_ERR)
			{
				return res;				
			}
			result|=res;
		}
		if(T_Bitmap1KB)
		{
			tmpMap = T_Bitmap1KB;
			TmpPtr = PHY_TMPBUF;
			Index= _Get_Bitmap_index(T_Bitmap);
			if(bPrintFlag==0x01)
			{
				INIT_BOOT("index: %x,buffer:%x ,%d\n",Index,(INT32U)(tempMainPtr+Index*512),Index%2 );	
			}
			result |= _GetPageData1KB_hw(PhyPageNum, tmpMap & FULL_BITMAP_OF_SINGLE_PAGE, TmpPtr, tempSparePtr);
			MEMCPY(tempMainPtr+Index*512,TmpPtr+(Index%2)*512,512);
			
		}
		
	}
	
    /* increase the main data pointer and the spare data pointer to next page */
    tempMainPtr += NAND_SECTOR_1KB * (SECTOR_NUM_PER_SINGLE_PAGE/2);

    /* check if spare data pointer is valid, if so, need increase the address */
    if(tempSparePtr)
    {
        tempSparePtr += SPARE_DATA_SIZE ;
    }

    /* increase the page number two the second plane */
    PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

    /* process the sector bitmap in page */
    tempSectBitmap = tempSectBitmap >> NandStorageInfo.SectorNumPerPage;
	

    /* check if the page read completely */
    if(tempSectBitmap)
    {
        goto _PROCESS_PAGE_READ_DATA;
    }

    _SetPseudo_Noise(0); //Disable Pseudo noise 
    /* release nand flash chip select, and pin */
    _NandReleaseDevice();
    
    PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);

    return result;
}

INT32U  _PHY_PageRead_HW(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */
    INT32U result;
    INT32U PhyPageNum;
    INT8S   *tempMainPtr, *tempSparePtr;
    INT32U tempSectBitmap;
    INT32U iLoop,tmpMap,Bitmap,jLoop;
    INT8S * TmpPtr,*DstPtr;
    INT32U Bitmap_tmp;

    INT8U Index;
    INT8U BufferRead;	

    PHY_DBG("\nPHY_DBG: Physic page read.\n");
  /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
		
	_SetPseudo_Noise(1); //Enable Pseudo noise 
	
    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;

    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempSectBitmap = NandOpPar->SectorBitmapInPage;

#if 0	
	if(bPrintFlag==0x01)
	{
		  INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x, Page:%x,Blk:%x\n",__func__,PhyPageNum,tempSectBitmap,
		  	    NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		  INIT_BOOT("tempMainPtr:0x%x tempSparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr,SECTOR_NUM_PER_SINGLE_PAGE);
	}		
#endif
    /* calculate the  main area data pointer */
    {
        INT32S i = 0;
        while(!(tempSectBitmap & (1<<i)))
        {
            i++;
        }
        tempMainPtr -= NAND_SECTOR_SIZE * i;
    }
	tempSectBitmap = NandOpPar->SectorBitmapInPage;
	
_PROCESS_PAGE_READ_DATA:

	/* wait nand flash status ready */
	wait_nand_ready();
	Bitmap=0x00;
	BufferRead =0x00;
	Bitmap_tmp = tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE;
	for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop++,jLoop++)
	{
		tmpMap =(Bitmap_tmp>>iLoop)&0x01;			
		if(tmpMap)
		{
			Bitmap |=(1<<jLoop);
			if(BufferRead ==1)
			{
				BufferRead =0x02;	
				//INIT_BOOT("xxtempMainPtr:0x%x tempSparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr,tempSectBitmap);
				
			}
		}
		///else if (tmpMap ==0x00 && Bitmap !=0x00)
                else
		{
			BufferRead =0x01;					
		}
		
	}	
	if(((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE ) && tempSparePtr  )	
		||(BufferRead==2) )
	{
		
		/* get page data of the first plane */	
		 if((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE) ==FULL_BITMAP_OF_SINGLE_PAGE) 
		{
			TmpPtr = tempMainPtr;	
		}
		else
		{
			TmpPtr = PHY_TMPBUF;
		}
		tmpMap = FULL_BITMAP_OF_SINGLE_PAGE ; 
		
		/* get page data of the first plane */
		result |= _GetPageData_hw(PhyPageNum, tmpMap & FULL_BITMAP_OF_SINGLE_PAGE, TmpPtr, tempSparePtr);

                //memcpy buffer 
		if((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)!=FULL_BITMAP_OF_SINGLE_PAGE)
		{	
			//INIT_BOOT("MainPtr:0x%x SparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr,tempSectBitmap);
			Bitmap_tmp = tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE;
			DstPtr = tempMainPtr;
			for(iLoop=0; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop++)
		        {		        
		                if(Bitmap_tmp & (1<<iLoop))
		                {		      
					MEMCPY(DstPtr,TmpPtr,512);	
				//	INIT_BOOT("ptr:0x%x,0x%x\n",tempMainPtr,TmpPtr);
		                }
				TmpPtr+=NAND_SECTOR_SIZE;	
				DstPtr+=NAND_SECTOR_SIZE;					
			}
			//INIT_BOOT("\n");
		}		
		
	}   
	else if(tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
	{		
		tmpMap = tempSectBitmap;			
		Index= _Get_Bitmap_index(tempSectBitmap);
		TmpPtr = tempMainPtr+Index*512;
	#if 0	
		 if((tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE) !=FULL_BITMAP_OF_SINGLE_PAGE) 		
		{
			ECC_Err("index: %d,buffer:0x%x ,0x%x,Bitmap:0x%x### \n",Index,(INT32U)(tempMainPtr+Index*512),
				(INT32U)tempMainPtr,tempSectBitmap );
		}	
	#endif	 
		result |= _GetPageData_hw(PhyPageNum, tmpMap & FULL_BITMAP_OF_SINGLE_PAGE, TmpPtr, tempSparePtr);
		/*if(res==PHY_LAYER_TIMEOUT_ERR)
		{
			return res;				
		}
		result|=res;	*/
	}
	
    /* increase the main data pointer and the spare data pointer to next page */
    tempMainPtr += NAND_SECTOR_SIZE* (SECTOR_NUM_PER_SINGLE_PAGE);

    /* check if spare data pointer is valid, if so, need increase the address */
    if(tempSparePtr)
    {
        tempSparePtr += SPARE_DATA_SIZE ;
    }

    /* increase the page number two the second plane */
    PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

    /* process the sector bitmap in page */
    tempSectBitmap = tempSectBitmap >> NandStorageInfo.SectorNumPerPage;
	
     /* check if the page read completely */
    if(tempSectBitmap)
    {
        goto _PROCESS_PAGE_READ_DATA;
    }

    _SetPseudo_Noise(0); //Disable Pseudo noise 

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);

    return result;
}

INT32U  _PHY_PageRead1KB(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */

    INT32U result;
    INT32U PhyPageNum;
    INT8S   *tempMainPtr, *tempSparePtr;
    INT32U tempSectBitmap;
	

		


    PHY_DBG("\nPHY_DBG: Physic page read.\n");
  /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    _SetPseudo_Noise(1); //Enable Pseudo noise 

    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;

    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempSectBitmap = NandOpPar->SectorBitmapInPage;
#if 0	
	if(bPrintFlag==0x01  )
	{
		  INIT_BOOT("\n##%s :Page:0x%x,Bit:%x, Page:%x,Blk:%x\n",__func__,PhyPageNum,tempSectBitmap,
		  	    NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		///  INIT_BOOT("tempMainPtr:0x%x tempSparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr,SECTOR_NUM_PER_SINGLE_PAGE);
	}		
#endif
    /* calculate the  main area data pointer */
    {
        INT32S i = 0;
        while(!(tempSectBitmap & (1<<i)))
        {
            i++;
        }
        tempMainPtr -= NAND_SECTOR_SIZE * i;
    }
    tempSectBitmap = NandOpPar->SectorBitmapInPage;

_PROCESS_PAGE_READ_DATA:

    /* wait nand flash status ready */
    wait_nand_ready();

    /* process the first plane */
    if(tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* first plane bitmap is valid */
        //wait_CMD_ready();
        NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
        NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);

        /* send page read command */
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_READ_00);
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_READ_30 & (~NAND_CMD_SEQU));
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)		
        Flash_DMA_Mode =FLAH_EIP_IRQ;
        flash_irq_enable();
        start_dma(NandDMAChannel);
        if(0x01==Flash_timeout())
        {		
            INIT_BOOT("####run to :%s,%d Total read :%d KB#### \n",__func__,__LINE__,ReadCap);				
        }		
#endif
        wait_sm_finish();

        /* get page data of the first plane */
        result |= _GetPageData1KB(PhyPageNum, tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);
    }

    //??????
    /* increase the main data pointer and the spare data pointer to next page */
    tempMainPtr += NAND_SECTOR_1KB * (SECTOR_NUM_PER_SINGLE_PAGE/2);

    //??????
    /* check if spare data pointer is valid, if so, need increase the address */
    if(tempSparePtr)
    {
        tempSparePtr += SPARE_DATA_SIZE;
    }

    /* increase the page number two the second plane */
    PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

    /* process the sector bitmap in page */
    tempSectBitmap = tempSectBitmap >> NandStorageInfo.SectorNumPerPage;

    /* check if the page read completely */
    if(tempSectBitmap)
    {
        goto _PROCESS_PAGE_READ_DATA;
    }

    _SetPseudo_Noise(0); //Disable Pseudo noise 
    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);

    return result;
}


INT32U  _PHY_WholePageRead(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */

    INT32U result;
    INT32U PhyPageNum;
    INT8S   *tempMainPtr, *tempSparePtr;
    INT32U tempSectBitmap;
    
    PHY_DBG("\nPHY_DBG: Physic page read.\n");
  /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    _SetPseudo_Noise(1); //Enable Pseudo noise 

    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;

    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempSectBitmap = NandOpPar->SectorBitmapInPage;

 
    tempSectBitmap = NandOpPar->SectorBitmapInPage;

_PROCESS_PAGE_READ_DATA:

    /* wait nand flash status ready */
    wait_nand_ready();

    /* process the first plane */
    if(tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* first plane bitmap is valid */
        //wait_CMD_ready();
        NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
        NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);

        /* send page read command */
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_READ_00);
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_READ_30 & (~NAND_CMD_SEQU));

        wait_sm_finish();

        /* get page data of the first plane */
        result |= _GetWholePageData(PhyPageNum, tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);
    }
  
    /* increase the main data pointer and the spare data pointer to next page */
    tempMainPtr += NAND_SECTOR_1KB * (SECTOR_NUM_PER_SINGLE_PAGE/2);


    /* check if spare data pointer is valid, if so, need increase the address */
    if(tempSparePtr)
    {
        tempSparePtr += SPARE_DATA_SIZE;
    }

    /* increase the page number two the second plane */
    PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

    /* process the sector bitmap in page */
    tempSectBitmap = tempSectBitmap >> NandStorageInfo.SectorNumPerPage;

    /* check if the page read completely */
    if(tempSectBitmap)
    {
        goto _PROCESS_PAGE_READ_DATA;
    }

    _SetPseudo_Noise(0); //Disable Pseudo noise 

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);

    return result;
}


#endif

/*
*********************************************************************************************************
*               PHYSIC LAYER READ PAGE
*
*Description: read a single page from nand flash to ram.
*
*Arguments  : NandOpPar     nand flash operate parameters.
*
*Return     : TRUE              read success;
*             FALSE             read failed;
*             FALSE|ERRORTYPE   error;
*********************************************************************************************************
*/

int likelySupportMicronReadRetry(){

     unsigned char MicronReadRetryID[]={0x2c,0x64,0x44,0x4b};
     int i;

     
     for(i=0;i<4;i++){
        if(NandStorageInfo.NandChipId[i] != MicronReadRetryID[i])   
        break;
     }
     if(i==4)
        return 1;
     else {
            return 0;
         //   INIT_BOOT("id:%x %x %x %x",NandStorageInfo.NandChipId[0],NandStorageInfo.NandChipId[1],NandStorageInfo.NandChipId[2],NandStorageInfo.NandChipId[3]);
     }
        
}
int MicronReadRetry(struct PhysicOpParameter *NandOpPar,int retryCnt){
    INT32U result=0;
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */

    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    //readData = MicronGetFeature();
    MicronSetFeature(retryCnt);
    
    
    _NandReleaseDevice();
    INIT_BOOT("Retry :%d\n",retryCnt);
    return result;
}

int MicronSetFeature(INT32U WriteData){
    
    INT32U WriteCmd;
    INT32U ReadAddress;
   
    ReadAddress = 0x89;
    WriteCmd = 0xEF |NAND_CMD_WCMD | NAND_CMD_RW_WRITE | NAND_CMD_DMAS |NAND_CMD_ROW_ADDRCYCLE_1;
    

    NAND_REG_WRITE(NAND_BC, 4);
    NAND_REG_WRITE(NAND_ROWADDRLO, ReadAddress);
    NAND_REG_WRITE(NAND_CMD_FSM, WriteCmd);

    wait_fifo_no_full();
    NAND_REG_WRITE(NAND_DATA,WriteData);
    wait_sm_finish();

    return TRUE;
}

int MicronGetFeature(){
    
    INT32U ReadData;
    INT32U ReadCmd;
    INT32U ReadAddress;

    
    ReadAddress = 0x89;
    ReadCmd = 0xEE |NAND_CMD_WCMD | NAND_CMD_RBI |NAND_CMD_DMAS | NAND_CMD_ROW_ADDRCYCLE_1;

    NAND_REG_WRITE(NAND_BC, 4);
    NAND_REG_WRITE(NAND_ROWADDRLO,ReadAddress);
    NAND_REG_WRITE(NAND_CMD_FSM, ReadCmd);

    
    wait_fifo_no_empt();
    ReadData = NAND_REG_READ(NAND_DATA);
    wait_sm_finish();

    return ReadData;

}


INT32U  _PHY_PageRead(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */

    INT32U result;
    INT32U PhyPageNum;
    INT8S   *tempMainPtr, *tempSparePtr;
    INT32U tempSectBitmap;

    PHY_DBG("\nPHY_DBG: Physic page read.\n");

        

#if CONFIG_AM_CHIP_ID==1211||CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

	if(BCH_MODE ==ECC_Type_12BitECC || BCH_MODE ==ECC_Type_24BitECC || BCH_MODE ==ECC_Type_40BitECC || BCH_MODE ==ECC_Type_60BitECC)
	{        
#if Flash_PAGE_R_BCH24==0x01 && (defined(__KERNEL__))
		return _PHY_PageRead_HW1KB(NandOpPar);
#else
           //     INIT_BOOT("_PHY_PageRead\n");
                // added by jjf for micron read retry
                if(likelySupportMicronReadRetry()){
                    int retryCnt=1;
                    int retryFlg= 0;
                    do{
                        result = _PHY_PageRead1KB(NandOpPar);
                     //   INIT_BOOT("read ret:%x\n",result);
                        if(result == NAND_DATA_ECC_ERROR){
                            MicronReadRetry(NandOpPar,retryCnt);
                            retryFlg = 1;
                            
                        }else{
                            break;
                        }
                        retryCnt++;
                    }while(retryCnt<8);
                    if(retryFlg){
                        MicronReadRetry(NandOpPar,0);
                    }
                    return result;
                }else{
                    return _PHY_PageRead1KB(NandOpPar);
                }
                
                

#endif

	}
	else
	{
#if Flash_PAGE_R_BCH8==0x01
		return _PHY_PageRead_HW(NandOpPar);
#endif		
	}
	
#endif

    /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
	
    _SetPseudo_Noise(1); //Enable Pseudo noise 

    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;

    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempSectBitmap = NandOpPar->SectorBitmapInPage;
#if 1		
	if(bPrintFlag==0x01)
	{
		INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x,",__func__,PhyPageNum,tempSectBitmap );
		INIT_BOOT("Page,%d, blk:%d,\n",NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		INIT_BOOT("MainPtr:x%x,0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr);
	}			
#endif
    /* calculate the  main area data pointer */
    {
        INT32S i = 0;
        while(!(tempSectBitmap & (1<<i)))
        {
            i++;
        }
        tempMainPtr -= NAND_SECTOR_SIZE * i;
    }
    tempSectBitmap = NandOpPar->SectorBitmapInPage;

_PROCESS_PAGE_READ_DATA:

    /* wait nand flash status ready */
    wait_nand_ready();

    /* process the first plane */
    if(tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {

#if (SUPPORT_SMALL_BLOCK)
        if (!NAND_IS_SMALL_BLOCK)
        {
#endif

            /* first plane bitmap is valid */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
            NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);

            /* send page read command */
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_READ_00);
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_READ_30 & (~NAND_CMD_SEQU));
#if (defined(__KERNEL__)) && (EIP_IRQ_MODE ==0x01)		
            Flash_DMA_Mode =FLAH_EIP_IRQ;
            flash_irq_enable();
            start_dma(NandDMAChannel);
            if(0x01==Flash_timeout())
            {		
                INIT_BOOT("####run to :%s,%d Total read :%d KB#### \n",__func__,__LINE__,ReadCap);				
            }		
#endif

            wait_sm_finish();
#if (SUPPORT_SMALL_BLOCK)
        }   /* if (!NAND_IS_SMALL_BLOCK) */
#endif

        /* get page data of the first plane */
        result |= _GetPageData(PhyPageNum, tempSectBitmap & FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);
    }

    /* increase the main data pointer and the spare data pointer to next page */
    tempMainPtr += NAND_SECTOR_SIZE * SECTOR_NUM_PER_SINGLE_PAGE;

    /* check if spare data pointer is valid, if so, need increase the address */
    if(tempSparePtr)
    {
        tempSparePtr += SPARE_DATA_SIZE;
    }

    /* increase the page number two the second plane */
    PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

    /* process the sector bitmap in page */
    tempSectBitmap = tempSectBitmap >> NandStorageInfo.SectorNumPerPage;

    /* check if the page read completely */
    if(tempSectBitmap)
    {
        goto _PROCESS_PAGE_READ_DATA;
    }

    _SetPseudo_Noise(0); //Disable Pseudo noise 

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);

    return result;
}

INT32U  PHY_PageRead_Spare(struct PhysicOpParameter *NandOpPar)
{
    return 0x00;	
}



INT32U  PHY_PageRead(struct PhysicOpParameter *NandOpPar)
{
#if 1
    INT32U iLoop,result;
    INT32U InsertSDCnt;
    
   
#if ECC_CORRECT_MODE 

    INT32U bReadTrueCnt;
    INT32U bReadErrorFlag,PhyReadCnt,bReadTrueCnt2; 
    iLoop =0x00;
    InsertSDCnt =0x00;
    PhyReadCnt= bReadTrueCnt =0x00;
    bReadErrorFlag =0x00;
    bReadTrueCnt2 =0x00;
    while(iLoop<MAX_LoopCnt)
    {
        
CONTINUE_READ:     

        result= _PHY_PageRead(NandOpPar); 
        if(bReadErrorFlag ==0x00  &&(0x00 == result  ||PHY_READ_ECC_TO_LIMIT(result) ))
        {                                              
            break;
        }            
        else if (bReadErrorFlag ==0x01 &&(0x00 == result  ||PHY_READ_ECC_TO_LIMIT(result) ) )
        {
            PhyReadCnt++;                       
            Trace_NF("X:%d ",PhyReadCnt);  
            _Short_DelayUS(1000*10);
            if(0x00 ==Test_ReadCard_Data() )
            {
                bReadTrueCnt++;                       
                Trace_NF("Y:%d ",bReadTrueCnt);  
                if(bReadTrueCnt<REPEAT_READ_CNT)
                    goto CONTINUE_READ;
            }   
            //@fish add 2010-06-04 fix on   PhyRead 和Mbrec Read OK
            if(PhyReadCnt >=REPEAT_READ_CNT  && bReadTrueCnt>=REPEAT_READ_CNT) 
                break;          
        }
        else 
        {
            //增加正常OVER ECC 出错情况                      
            if( 0x00 ==Test_ReadCard_Data())
            {
                bReadTrueCnt2++;
                if(bReadTrueCnt2>3)
                    break;
            }                                                        

        }
        INIT_BOOT("cnt:%x\n",bReadTrueCnt2);
        bReadErrorFlag =0x01;
        bReadTrueCnt= PhyReadCnt = 0x00;
        _Short_DelayUS(1000*10);
        iLoop++;           
        if(0x00==iLoop%10)
        {
            Trace_NF("&%d,",iLoop);                        
        } 
        
#if 0//(defined(__KERNEL__))            
        if (0)//if(0x00==iLoop%2)
        {
            //printf("b crd(%x)\n",read_c0_status());
            if(0x00  ==card_detecting())//SD insert
            {
                Trace_NF("&SD Power on %x ^&&\n",iLoop);
                bReadTrueCnt2 =0x00;
                SD_Power_CTRL(0x01);//Power off SD card VCC 
                _Short_DelayUS(16*1000);
                SD_Power_CTRL(0x00);//Power on SD Card VCC 
                _Short_DelayUS(15*1000);
                //current video status 
                //   if(get_video()==0x1)
                //      Card_SetFlag();
            }
        }  
#endif     
                    

    }

    if(iLoop>=1)
    {             
        Trace_NF("\n==<%s>_<%d>_",__func__,__LINE__);
        Trace_NF("##Blk:%x,Page:%x,Bit:%x,Blk:%x##Loop:%d,res:0x%x\n",NandOpPar->PhyBlkNumInBank,
            NandOpPar->PageNum,        NandOpPar->SectorBitmapInPage, NandOpPar->BankNum,iLoop,result);   
    }   
    
    PHY_ReadRes(BCH_SET_VAL,result);
    
    if(result == NAND_DATA_ECC_0xFF || result ==NAND_DATA_ECC_ERROR)
    {
        result =0x00;
    }


#else
     PHY_DBG("\nPHY_DBG: Physic page read.\n");
    /* calculate chip select number and physic block number in chip */  
    result= _PHY_PageRead(NandOpPar);  
    
#endif
   

    PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);

    return result;
    
#else
	INT32U result;
	INT32U ra_reg;
	PHY_DBG("\nPHY_DBG: Physic page read.\n");

	ra_reg = GET_CPU_REG(ra);

	result = _PHY_PageRead(NandOpPar);   
	
	if(result==PHY_LAYER_TIMEOUT_ERR  &&
		(BCH_MODE ==ECC_Type_12BitECC || BCH_MODE ==ECC_Type_24BitECC) )
	{
		result=_PHY_PageRead1KB(NandOpPar);
	}

	PHY_DBG("PHY_DBG: Physic page read complete, the result is:0x%x\n",result);
	return result;
#endif
}



#if CONFIG_AM_CHIP_ID ==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213


INT32U  _PHY_PageWrite1KB_HW(struct PhysicOpParameter *NandOpPar)
{
	INT32U ChipNum;         /* the chip number cur operation */
	INT32U BlkNumInChip;    /* the real block number in chip */
	INT32U tempBitmap;
	INT32U result;
	INT32U PhyPageNum;
	INT32U tmpBnkNumInChip;
	void   *tempMainPtr, *tempSparePtr;
	INT32U Bitmap1KB,iLoop,tmpMap;
	INT32U jLoop;

	PHY_DBG("\nPHY_DBG: Physic page write.\n");


	/* calculate chip select number and physic block number in chip */
	result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
	if(result)
	{
		PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
		result = PHYSICAL_LAYER_ERROR;
		return result;
	}

	tmpBnkNumInChip = NandOpPar->BankNum % NandStorageInfo.BankCntPerChip;

	/* enable nand controller, set chip select,and get pin if need */
	result = _NandGetDevice(ChipNum);
	if(result != TRUE)
	{
		result = PHYSICAL_LAYER_ERROR;
		return result;
	}
	_SetPseudo_Noise(1); //Enable Pseudo noise 
	
    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;
    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempBitmap = NandOpPar->SectorBitmapInPage;
#if 0		
	if(bPrintFlag==0x01)
	{		
            INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x, Page:%x,Blk:%x\n",__func__,PhyPageNum,tempBitmap,
                        NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
            INIT_BOOT("tempMainPtr:0x%x tempSparePtr:0x%x  :0x%x\n",(INT32U)tempMainPtr,
                        (INT32U)tempSparePtr, SECTOR_NUM_PER_SINGLE_PAGE);
	}			
#endif
    /* process the first plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
              Bitmap1KB =0x00;
    	for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2,jLoop++)
    	{
    		tmpMap =(FULL_BITMAP_OF_SINGLE_PAGE>>iLoop)&0x03;
    		if(tmpMap==3)
    		{
    			Bitmap1KB |=(1<<jLoop);
    		}		   	
    	}
            /* send page data of the first plane */
        result = _SendPageData1KB_hw(PhyPageNum, Bitmap1KB, tempMainPtr, tempSparePtr); 
			
    	/* increase the main data pointer and the spare data pointer to next page */
    	tempMainPtr += NandStorageInfo.SectorNumPerPage * NAND_SECTOR_SIZE;
    	if(tempSparePtr)
    	{
    		tempSparePtr += SPARE_DATA_SIZE;
    	}
	
    }
    tempBitmap = tempBitmap >> NandStorageInfo.SectorNumPerPage;

    /* process the second plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
            /* increase the page number two the second plane */
    	PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;
    	Bitmap1KB =0x00;
    	for(iLoop=0,jLoop=0x00; iLoop<SECTOR_NUM_PER_SINGLE_PAGE; iLoop+=2,jLoop++)
    	{
    		tmpMap =(FULL_BITMAP_OF_SINGLE_PAGE>>iLoop)&0x03;
    		if(tmpMap==3)
    		{
    			Bitmap1KB |=(1<<jLoop);
    		}		   	
    	}
        result = _SendPageData1KB_hw(PhyPageNum, Bitmap1KB, tempMainPtr, tempSparePtr);

        LastPhysicOpCmd = 0xffff;
    }
	
   	_SetPseudo_Noise(0); //Disable Pseudo noise 
    
    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    PHY_DBG("PHY_DBG: Physic page write complete, the result is:0x%x\n",result);

    return result;
}


INT32U  _PHY_PageWrite_HW(struct PhysicOpParameter *NandOpPar)
{
	INT32U ChipNum;         /* the chip number cur operation */
	INT32U BlkNumInChip;    /* the real block number in chip */
	INT32U tempBitmap;
	INT32U result;
	INT32U PhyPageNum;
	INT32U tmpBnkNumInChip;
	void   *tempMainPtr, *tempSparePtr;
    

	PHY_DBG("\nPHY_DBG: Physic page write.\n");

	/* calculate chip select number and physic block number in chip */
	result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
	if(result)
	{
		PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
		result = PHYSICAL_LAYER_ERROR;
		return result;
	}

	tmpBnkNumInChip = NandOpPar->BankNum % NandStorageInfo.BankCntPerChip;

	/* enable nand controller, set chip select,and get pin if need */
	result = _NandGetDevice(ChipNum);
	if(result != TRUE)
	{
		result = PHYSICAL_LAYER_ERROR;
		return result;
	}
	
	_SetPseudo_Noise(1); //Enable Pseudo noise 

	
	LastPhysicOpCmd = 0xffff;	
	
    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;
    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempBitmap = NandOpPar->SectorBitmapInPage;

#if 0		
	if(bPrintFlag==0x01)
	{
		INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x,",__func__,PhyPageNum,tempBitmap );
		INIT_BOOT("Page,%d, blk:%d,\n",NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		INIT_BOOT("MainPtr:x%x,0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr);
	}			
#endif
        /* process the first plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {                
        /* send page data of the first plane */
        result = _SendPageData_hw(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr); 
    	/* increase the main data pointer and the spare data pointer to next page */
    	tempMainPtr += NandStorageInfo.SectorNumPerPage * NAND_SECTOR_SIZE;
    	if(tempSparePtr)
    	{
    		tempSparePtr += SPARE_DATA_SIZE;
    	}
	
    }
    tempBitmap = tempBitmap >> NandStorageInfo.SectorNumPerPage;

    /* process the second plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* increase the page number two the second plane */
        PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

        result = _SendPageData_hw(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);

        LastPhysicOpCmd = 0xffff;
    }
	
    _SetPseudo_Noise(0); //Enable Pseudo noise 

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();
    

    PHY_DBG("PHY_DBG: Physic page write complete, the result is:0x%x\n",result);

    return result;
}

INT32U  _PHY_PageWrite1KB(struct PhysicOpParameter *NandOpPar)
{
	INT32U ChipNum;         /* the chip number cur operation */
	INT32U BlkNumInChip;    /* the real block number in chip */
	INT32U tempBitmap;
	INT32U result;
	INT32U PhyPageNum;
	INT32U tmpBnkNumInChip;
    INT32U nandstatus;
	void   *tempMainPtr, *tempSparePtr;

	PHY_DBG("\nPHY_DBG: Physic page write.\n");

	/* calculate chip select number and physic block number in chip */
	result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
	if(result)
	{
		PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
		result = PHYSICAL_LAYER_ERROR;
		return result;
	}
	
    tmpBnkNumInChip = NandOpPar->BankNum % NandStorageInfo.BankCntPerChip;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
    _SetPseudo_Noise(1); //Enable Pseudo noise 

    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;
    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempBitmap = NandOpPar->SectorBitmapInPage;
#if 0		
	if(bPrintFlag==0x01  ||NandOpPar->PhyBlkNumInBank <7 )
	{
		INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x,",__func__,PhyPageNum,tempBitmap );
		INIT_BOOT("Page,%x, blk:%x,\n",NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		INIT_BOOT("MainPtr:x%x,0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr);
	}			
#endif
    /* process the first plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* first plane bitmap is valid */          
        wait_sm_finish();
        NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
        NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_80 & (~NAND_CMD_DMAS));
        wait_sm_finish();

        /* record current operation command */
        if (NAND_SUPPORT_MULTI_PLANE)
        {
            LastPhysicOpCmd = 0x80;
        }

        /* send page data of the first plane */
        result = _SendPageData1KB(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);

        /* increase the main data pointer and the spare data pointer to next page */
        tempMainPtr += SECTOR_NUM_PER_SINGLE_PAGE * NAND_SECTOR_SIZE;	
        if(tempSparePtr)
        {
            tempSparePtr += SPARE_DATA_SIZE;
        }

    }

    tempBitmap = tempBitmap >> NandStorageInfo.SectorNumPerPage;

    /* process the second plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* increase the page number two the second plane */
        PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

        if(LastPhysicOpCmd == 0x80)
        {
            /* last write is the first plane, send the dumy program command */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_CMD_FSM, (NAND_CMD_WRITE_10 & (~0xff)) | NandDevInfo.SpecialCommand->MultiProgCmd[0]);

            /* wait nand controller statemachine ready, and flash RB-Status ready */
            wait_sm_finish();

            /* read status to check if nand flash is ready */
            _WaitNandBnkReady(tmpBnkNumInChip);

            /* send the second plane write command */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
            NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);			
            NAND_REG_WRITE(NAND_CMD_FSM, ((NAND_CMD_WRITE_80 & (~0xff)) & (~NAND_CMD_DMAS)) | NandDevInfo.SpecialCommand->MultiProgCmd[1]);
        }
        else
        {
            /* send page write command */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
            NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);			
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_80 & (~NAND_CMD_DMAS));
        }

        result = _SendPageData1KB(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);

        LastPhysicOpCmd = 0xffff;
    }


    /* send page program command or page program with cache command */
    if(LastPhysicOpCmd == 0xffff)
    {
        //wait_CMD_ready();
        if(NAND_SUPPORT_CACHE_PROGRAM)
        {
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_15);
        }
        else
        {
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_10);
        }

        /* wait nand controller statemachine ready */
        wait_sm_finish();
        wait_nand_ready();
        nandstatus = _ReadNandStatus(0x70,NandOpPar->BankNum);
        if(nandstatus&NAND_OPERATE_FAIL)
        {
            PHY_ERR("status:0x%x Blknum:0x%x PageNum:0x%x program failed !\n",nandstatus,BlkNumInChip,NandOpPar->PageNum);
            result |= NAND_OP_ERROR;
        }
    }
	
    _SetPseudo_Noise(0); //Disable Pseudo noise 
    
    /* release nand flash chip select, and pin */
    _NandReleaseDevice();
    PHY_DBG("PHY_DBG: Physic page write complete, the result is:0x%x\n",result);

    return result;
}


INT32U  PHY_WholePageWrite(struct PhysicOpParameter *NandOpPar)
{
	INT32U ChipNum;         /* the chip number cur operation */
	INT32U BlkNumInChip;    /* the real block number in chip */
	INT32U tempBitmap;
	INT32U result;
	INT32U PhyPageNum;
	INT32U tmpBnkNumInChip;
	void   *tempMainPtr, *tempSparePtr;

	PHY_DBG("\nPHY_DBG: Physic page write.\n");

	/* calculate chip select number and physic block number in chip */
	result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
	if(result)
	{
		PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
		result = PHYSICAL_LAYER_ERROR;
		return result;
	}
	
    tmpBnkNumInChip = NandOpPar->BankNum % NandStorageInfo.BankCntPerChip;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }
_SetPseudo_Noise(1); //Enable Pseudo noise 

    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;
    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempBitmap = NandOpPar->SectorBitmapInPage;

    /* process the first plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* first plane bitmap is valid */          
        wait_sm_finish();
        NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
        NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_80 & (~NAND_CMD_DMAS));
        wait_sm_finish();

        /* record current operation command */
        if (NAND_SUPPORT_MULTI_PLANE)
        {
            LastPhysicOpCmd = 0x80;
        }

        /* send page data of the first plane */
        result = _Send_WholePageData(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);

        /* increase the main data pointer and the spare data pointer to next page */
        tempMainPtr += SECTOR_NUM_PER_SINGLE_PAGE * NAND_SECTOR_SIZE;	
        if(tempSparePtr)
        {
            tempSparePtr += SPARE_DATA_SIZE;
        }

    }

    /* send page program command or page program with cache command */
    if(LastPhysicOpCmd == 0xffff)
    {
        //wait_CMD_ready();
        if(NAND_SUPPORT_CACHE_PROGRAM)
        {
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_15);
        }
        else
        {
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_10);
        }

        /* wait nand controller statemachine ready */
        wait_sm_finish();
    }
	
    _SetPseudo_Noise(0); //Disable Pseudo noise 
    
    /* release nand flash chip select, and pin */
    _NandReleaseDevice();
    PHY_DBG("PHY_DBG: Physic page write complete, the result is:0x%x\n",result);

    return result;
}
#endif

/*
*********************************************************************************************************
*               PHYSIC LAYER WRITE PAGE
*
*Description: write a single page to nand flash from ram.
*
*Arguments  : NandOpPar     nand flash physic operate parameters.
*
*Return     : TRUE              write success;
*             FALSE             write failed;
*             FALSE|ERRORTYPE   error;
*
*Note       : this function can support single page write, and multi-page write, for multi-page
*             write, it can only support two plane.
*********************************************************************************************************
*/
INT32U  PHY_PageWrite(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */
    INT32U tempBitmap;
    INT32U result;
    INT32U PhyPageNum;
    INT32U tmpBnkNumInChip;
    INT32U nandstatus;
    void   *tempMainPtr, *tempSparePtr;

    PHY_DBG("\nPHY_DBG: Physic page write.\n");


#if CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	if(BCH_MODE ==ECC_Type_12BitECC || BCH_MODE ==ECC_Type_24BitECC || BCH_MODE ==ECC_Type_40BitECC ||BCH_MODE ==ECC_Type_60BitECC)
	{

#if Flash_PAGE_W_BCH24==0x01
		return _PHY_PageWrite1KB_HW(NandOpPar);
#else
		return _PHY_PageWrite1KB(NandOpPar);
#endif
	}
	else
	{
#if Flash_PAGE_W_BCH8==0x01
		return _PHY_PageWrite_HW(NandOpPar);
#endif
	}
#endif
    /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    tmpBnkNumInChip = NandOpPar->BankNum % NandStorageInfo.BankCntPerChip;

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

	_SetPseudo_Noise(1); //Enable Pseudo noise 

    /* calculate the page address */
    PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;
    tempMainPtr = NandOpPar->MainDataPtr;
    tempSparePtr = NandOpPar->SpareDataPtr;
    tempBitmap = NandOpPar->SectorBitmapInPage;
	
#if 0		
	if(bPrintFlag==0x01   )
	{
		INIT_BOOT("\n##%s :Pagenum:0x%x,Bitmap:%x,",__func__,PhyPageNum,tempBitmap );
		INIT_BOOT("Page,%d, blk:%d,",NandOpPar->PageNum,NandOpPar->PhyBlkNumInBank);
		INIT_BOOT("MainPtr:x%x,0x%x\n",(INT32U)tempMainPtr,(INT32U)tempSparePtr);
	}		
#endif
    /* process the first plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {

#if (SUPPORT_SMALL_BLOCK)
    if (!NAND_IS_SMALL_BLOCK)
    {
#endif

        /* first plane bitmap is valid */
        //wait_CMD_ready();
        wait_sm_finish();
        NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
	    NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);
        NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_80 & (~NAND_CMD_DMAS));
	    wait_sm_finish();
	
        /* record current operation command */
        if (NAND_SUPPORT_MULTI_PLANE)
        {
            LastPhysicOpCmd = 0x80;
        }

#if (SUPPORT_SMALL_BLOCK)
    }   /* if (!NAND_IS_SMALL_BLOCK) */
#endif

        /* send page data of the first plane */
        result = _SendPageData(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);

        /* increase the main data pointer and the spare data pointer to next page */
        tempMainPtr += NandStorageInfo.SectorNumPerPage * NAND_SECTOR_SIZE;
        if(tempSparePtr)
        {
            tempSparePtr += SPARE_DATA_SIZE;
        }

    }

    tempBitmap = tempBitmap >> NandStorageInfo.SectorNumPerPage;

    /* process the second plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* increase the page number two the second plane */
        PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

        if(LastPhysicOpCmd == 0x80)
        {
            /* last write is the first plane, send the dumy program command */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_CMD_FSM, (NAND_CMD_WRITE_10 & (~0xff)) | NandDevInfo.SpecialCommand->MultiProgCmd[0]);

            /* wait nand controller statemachine ready, and flash RB-Status ready */
            wait_sm_finish();

            /* read status to check if nand flash is ready */
            _WaitNandBnkReady(tmpBnkNumInChip);

            /* send the second plane write command */
            //wait_CMD_ready();
    	    NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
    	    NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);			
            NAND_REG_WRITE(NAND_CMD_FSM, ((NAND_CMD_WRITE_80 & (~0xff)) & (~NAND_CMD_DMAS)) | NandDevInfo.SpecialCommand->MultiProgCmd[1]);
        }
        else
        {
            /* send page write command */
            //wait_CMD_ready();
    	    NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);
    	    NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);			
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_80 & (~NAND_CMD_DMAS));
        }

        result = _SendPageData(PhyPageNum, FULL_BITMAP_OF_SINGLE_PAGE, tempMainPtr, tempSparePtr);

        LastPhysicOpCmd = 0xffff;
    }

#if (SUPPORT_SMALL_BLOCK)
    if (!NAND_IS_SMALL_BLOCK)
    {
#endif
    /* send page program command or page program with cache command */
    if(LastPhysicOpCmd == 0xffff)
    {
        //wait_CMD_ready();
        if(NAND_SUPPORT_CACHE_PROGRAM)
        {
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_15);
        }
        else
        {
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_10);
        }

        /* wait nand controller statemachine ready */
        wait_sm_finish();
        wait_nand_ready();
        nandstatus = _ReadNandStatus(0x70,NandOpPar->BankNum);
        if(nandstatus&NAND_OPERATE_FAIL)
        {
            PHY_ERR("status:0x%x Blknum:0x%x PageNum:0x%x program failed !\n",nandstatus,BlkNumInChip,NandOpPar->PageNum);    
            result |= NAND_OP_ERROR;
        }
    }

#if (SUPPORT_SMALL_BLOCK)
    }   /* if (!NAND_IS_SMALL_BLOCK) */
#endif

	_SetPseudo_Noise(0); //Disable Pseudo noise 
	
    /* release nand flash chip select, and pin */
    _NandReleaseDevice();


    PHY_DBG("PHY_DBG: Physic page write complete, the result is:0x%x\n",result);

    return result;
}


/*
*********************************************************************************************************
*               PHYSIC LAYER COPY PAGE SIMULATE
*
*Description: copy a page from one block to another block, by call page read and page write function.
*
*Arguments  : NandOpSrcPar      the parameter of the source page which need be copied
*             NandOpDstPar      the parameter of the destination page which copied to
*
*Return     : TRUE              page copy success;
*             FALSE             page copy failed;
*             FALSE|ERRORTYPE   page copy error;
*
*Note       : this function copy one page, the page may be a single page or an extened page, it is
*             marked by the plane bitmap. The nand don't support copyback operation,
*             page copy operation will use external buffer to copy a page.
*********************************************************************************************************
*/
INT32U  _PageCopySimulate(struct PhysicOpParameter *NandOpSrcPar, struct PhysicOpParameter *NandOpDstPar)
{
    INT8U  SpareData[3*4];
    INT32U result;
    struct PhysicOpParameter tmpSrcPar, tmpDstPar;
    INT32U bResult;
    
    tmpSrcPar = *NandOpSrcPar;
    tmpDstPar = *NandOpDstPar;

    /* set the external buffer address for page copy */
    tmpSrcPar.MainDataPtr = PAGE_COPYBACK_BUFFER;
    tmpSrcPar.SpareDataPtr = (void*)SpareData;
    tmpDstPar.MainDataPtr = PAGE_COPYBACK_BUFFER;
    tmpDstPar.SpareDataPtr = (void*)SpareData;
    if(PAGE_BUFFER_SECT_NUM == SECTOR_NUM_PER_SUPER_PAGE)
    {
        /* read page data to external buffer */
        
        bResult=PHY_PageRead(&tmpSrcPar);
	
#if (SUPER_WEAR_LEVELLING ==1)          
        PHY_ReadRes(BCH_SET_VAL,bResult);
         if(bResult!=0)
        {
            INIT_BOOT("run to<%s>_<%d> res:%x,\n",__func__,__LINE__,bResult);
    		//bPrintFlag =0x01;
    		PHY_PageRead(&tmpSrcPar);
    		///bPrintFlag =0x00;
         }
	
 #endif      
        result = PHY_PageWrite(&tmpDstPar);
    }
    else
    {
        tmpSrcPar.SectorBitmapInPage = FULL_BITMAP_OF_BUFFER_PAGE;
        tmpDstPar.SectorBitmapInPage = tmpSrcPar.SectorBitmapInPage;

        /* read page data to external buffer */
        bResult=PHY_PageRead(&tmpSrcPar);
 #if (SUPER_WEAR_LEVELLING ==1)        
        PHY_ReadRes(BCH_SET_VAL,bResult);
        if(bResult!=0)
        {
            INIT_BOOT("run to<%s>_<%d> res:%x,\n",__func__,__LINE__,bResult);
        }
  #endif      
        result = PHY_PageWrite(&tmpDstPar);
        if(result)
        {
            PHY_DBG("PHY_DBG: physic page write for copy error!\n");
        }

        tmpSrcPar.SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE ^ FULL_BITMAP_OF_BUFFER_PAGE;
        tmpDstPar.SectorBitmapInPage = tmpSrcPar.SectorBitmapInPage;

        /* wait nand flash true ready, then process high page */
        result |= PHY_SyncNandOperation(tmpSrcPar.BankNum);

        /* read page data to external buffer */
        bResult=PHY_PageRead(&tmpSrcPar);
  #if (SUPER_WEAR_LEVELLING ==1)       
        PHY_ReadRes(BCH_SET_VAL,bResult);     
        if(bResult!=0)
        {
            INIT_BOOT("run to<%s>_<%d> res:%x,\n",__func__,__LINE__,bResult);
        }
   #endif     
        result |= PHY_PageWrite(&tmpDstPar);
        if(result)
        {
            PHY_DBG("PHY_DBG: physic page write for copy error!\n");
        }
    }

    return result;
}

/*
*********************************************************************************************************
*               PHYSIC LAYER COPY PAGE
*
*Description: copy a page from one block to another block.
*
*Arguments  : NandOpSrcPar      the parameter of the source page which need be copied
*             NandOpDstPar      the parameter of the destination page which copied to
*
*Return     : TRUE              page copy success;
*             FALSE             page copy failed;
*             FALSE|ERRORTYPE   page copy error;
*
*Note       : this function copy one page, the page may be a single page or an extened page, it is
*             marked by the plane bitmap. If the nand support copyback operation, page copy operation
*             will use page copy command to copy a page; If the nand don't support copyback operation,
*             page copy operation will use external buffer to copy a page.
*********************************************************************************************************
*/
INT32U  PHY_CopyNandPage(struct PhysicOpParameter *NandOpSrcPar, struct PhysicOpParameter *NandOpDstPar)
{
#if SUPPORT_PAGE_COPYBACK

    INT32U result;
    INT32U tmpChipNum;
    INT32U tmpSrcBlkNum;
    INT32U tmpDstBlkNum;
    INT32U tmpSrcPageNum;
    INT32U tmpDstPageNum;

    /* set sector bitmap */
    NandOpSrcPar->SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;
    NandOpDstPar->SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;

    /* when use page copy command to copy a page, must be sure the */
    /* source page and the dest page are same as odd or even */
    if((!NAND_SUPPORT_PAGE_COPYBACK) || ((NandOpSrcPar->PageNum & 0x1) ^ (NandOpDstPar->PageNum & 0x1)))
    {
        /* the nand flash don't support page copy back command, need simulate page copy */
        result = _PageCopySimulate(NandOpSrcPar, NandOpDstPar);
        return result;
    }
    else
    {
        /* check if the source block and destination block are in the same bank */
        if(NandOpSrcPar->BankNum != NandOpDstPar->BankNum)
        {
            PHY_ERR("PHY_ERR: the source block and destination block are not in the same bank when copy!\n");
            return PHYSICAL_LAYER_ERROR;
        }

        /* calculate the source block address */
        result = _CalPhyBlkNumByBlkNumInBank(&tmpChipNum, &tmpSrcBlkNum, \
                    NandOpSrcPar->BankNum, NandOpSrcPar->PhyBlkNumInBank);

        /* calculate the destination block address */
        result = _CalPhyBlkNumByBlkNumInBank(&tmpChipNum, &tmpDstBlkNum, \
                    NandOpDstPar->BankNum, NandOpDstPar->PhyBlkNumInBank);

        /* calculate the page address for the source page */
        tmpSrcPageNum = tmpSrcBlkNum * NandStorageInfo.PageNumPerPhyBlk + NandOpSrcPar->PageNum;

        /* calculate the page address for the destination page */
        tmpDstPageNum = tmpDstBlkNum * NandStorageInfo.PageNumPerPhyBlk + NandOpDstPar->PageNum;

        /* enable nand controller, set chip select,and get pin if need */
        result = _NandGetDevice(tmpChipNum);
        if(result != TRUE)
        {
            result = PHYSICAL_LAYER_ERROR;
           return result;
        }

        wait_nand_ready();
        if(!NAND_SUPPORT_MULTI_PLANE)
        {
            /* send copyback read command for page copy */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);			
            NAND_REG_WRITE(NAND_ROWADDRLO, tmpSrcPageNum );
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_00);
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_35);

            /* wait nand flash statemachine ready, and Rb status ready */
            wait_sm_finish();
            wait_nand_ready();

            /* wait nand controller command register ready */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_MAIN_COLADDR,0x0);
            NAND_REG_WRITE(NAND_ROWADDRLO, tmpDstPageNum);
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_WRITE_85 & ~NAND_CMD_SEQU);
            wait_sm_finish();

            /* send nand program command */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_10);
            wait_sm_finish();
        }
        else
        {
            /* use multi-plane copy back command to copy page */
            if(NandDevInfo.SpecialCommand->MultiCopyReadCmd[0] == 0x00)
            {
                /* read out the low page for copy */
    	         NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);			
    	         NAND_REG_WRITE(NAND_ROWADDRLO, tmpSrcPageNum);
                //wait_CMD_ready();
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_00);
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_35);
                wait_sm_finish();
                wait_nand_ready();

                /* set source page number to the second plane */
                tmpSrcPageNum = tmpSrcPageNum +  MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

                /* read out the high page for copy */
    	         NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);			
    	         NAND_REG_WRITE(NAND_ROWADDRLO, tmpSrcPageNum);				
                //wait_CMD_ready();
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_00);
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_35);
                wait_sm_finish();
                wait_nand_ready();
            }
            else
            {
                /* send the low page address for copy read */
                NAND_REG_WRITE(NAND_ROWADDRLO, tmpSrcPageNum);
                //wait_CMD_ready();
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPY_READ_60);
                wait_sm_finish();

                /* set source page number to the second plane */
                tmpSrcPageNum = tmpSrcPageNum +  MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

                /* send the high page address for copy read */
                NAND_REG_WRITE(NAND_ROWADDRLO, tmpSrcPageNum);
                //wait_CMD_ready();
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPY_READ_60);
                wait_sm_finish();

                /* send the multi-plane read for copy back verify command */
                //wait_CMD_ready();
                NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_COPYBACK_READ_35);
                wait_sm_finish();
                wait_nand_ready();
            }

            /* page page write for copy page */
            /* send low page address for copy write */
            //wait_CMD_ready();
            //NAND_REG_WRITE(NAND_ADDRLO1234, (tmpDstPageNum & 0xffff) << 16);
            //NAND_REG_WRITE(NAND_ADDRLO56, tmpDstPageNum >> 16);
            NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);			
            NAND_REG_WRITE(NAND_ROWADDRLO, tmpDstPageNum);  
			
            NAND_REG_WRITE(NAND_CMD_FSM, (NAND_CMD_COPYBACK_WRITE_85 & (~0xff))
                             | NandDevInfo.SpecialCommand->MultiCopyProgCmd[0]);
            wait_sm_finish();

            /* set the high page address for copy page write */
            tmpDstPageNum = tmpDstPageNum +  MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

            /* send dumy program command for copy page write */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_CMD_FSM, (NAND_CMD_WRITE_10 & (~0xff)) | NandDevInfo.SpecialCommand->MultiCopyProgCmd[1]);
            wait_sm_finish();
            wait_nand_ready();

            /* send high page address for copy write */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_MAIN_COLADDR, 0x0);			
	        NAND_REG_WRITE(NAND_ROWADDRLO, tmpDstPageNum);				
			
            NAND_REG_WRITE(NAND_CMD_FSM, (NAND_CMD_COPYBACK_WRITE_85 & (~0xff))
                             | NandDevInfo.SpecialCommand->MultiCopyProgCmd[2]);
            wait_sm_finish();

            /* send nand program command for copy page write */
            //wait_CMD_ready();
            NAND_REG_WRITE(NAND_CMD_FSM, NAND_CMD_WRITE_10);
            wait_sm_finish();
            wait_nand_ready();
        }

        /* release nand flash chip select, and pin */
        _NandReleaseDevice();
    }

    return TRUE;

#else

    INT32U result;

    /* set sector bitmap */
    NandOpSrcPar->SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;
    NandOpDstPar->SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;

    /* the nand flash don't support page copy back command, need simulate page copy */
    result = _PageCopySimulate(NandOpSrcPar, NandOpDstPar);
    return result;

#endif

}


/*
*********************************************************************************************************
*               PHYSIC LAYER ERASE NAND FLASH BLOCK
*
*Description: erase nand flash block.
*
*Arguments  : ChipNum       the chip select number which is operated;
*             BlockNum      the block number which need be erased;
*             PlaneBitmap   the bitmap of the plane which need be operated;
*
*Return     : TRUE              erase block success;
*             FALSE             erase block failed;
*             FALSE|ERRORTYPE   erase block error;
*
*Note       : this function erase one block of the nand flash. the block may be a single blcok
*             or a extended block. the type of the block is marked by the plane bitmap.
*********************************************************************************************************
*/
INT32U  PHY_EraseNandBlk(struct PhysicOpParameter *NandOpPar)
{
    INT32U ChipNum;         /* the chip number cur operation */
    INT32U BlkNumInChip;    /* the real block number in chip */
    INT32U tempBitmap;
    INT32U result;
    INT32U PhyPageNum;
#if ERASE_DEBUG_MSG==1    
   static INT32U wEraseCnt=0x00; 
#endif
    /* calculate chip select number and physic block number in chip */
    result = _CalPhyBlkNumByBlkNumInBank(&ChipNum, &BlkNumInChip, NandOpPar->BankNum, NandOpPar->PhyBlkNumInBank);
    if(result)
    {
        PHY_ERR("PHY_ERR: Block Address Transfer Error!!\n");
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* enable nand controller, set chip select,and get pin if need */
    result = _NandGetDevice(ChipNum);
    if(result != TRUE)
    {
        result = PHYSICAL_LAYER_ERROR;
        return result;
    }

    /* calculate the page address */

#if (SUPPORT_SMALL_BLOCK)
    if (NAND_IS_SMALL_BLOCK)
    {
        PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk * SECTOR_NUM_PER_SINGLE_PAGE 
            + NandOpPar->PageNum;
    }
    else
    {
#endif

        PhyPageNum = BlkNumInChip * NandStorageInfo.PageNumPerPhyBlk + NandOpPar->PageNum;

#if (SUPPORT_SMALL_BLOCK)
    }   /* if (!NAND_IS_SMALL_BLOCK) */
#endif

    tempBitmap = NandOpPar->SectorBitmapInPage;

_PROCESS_BLOCK_ERASE:

    /* process the block in first plane */
    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        /* send block erase command */
        //wait_CMD_ready();
        //NAND_REG_WRITE(NAND_ADDRLO1234, PhyPageNum);
        NAND_REG_WRITE(NAND_ROWADDRLO, PhyPageNum);
        NAND_REG_WRITE(NAND_CMD_FSM, (NAND_CMD_ERASE_60) & ~NAND_CMD_SEQU);
        wait_sm_finish();
    }
    WearDbg.wEraseBlkCnt++;
    
   
#if ERASE_DEBUG_MSG==1   
if(PhyPageNum /NandStorageInfo.PageNumPerPhyBlk <8)
     INIT_BOOT("&&&Era_blk:%4x,%4x\n", PhyPageNum /NandStorageInfo.PageNumPerPhyBlk,wEraseCnt++);
#endif  

    /* process the bitmap and the physic page number */
    tempBitmap = tempBitmap >> NandStorageInfo.SectorNumPerPage;
    PhyPageNum += MULTI_BLOCK_ADDR_OFFSET * NandStorageInfo.PageNumPerPhyBlk;

    if(tempBitmap & FULL_BITMAP_OF_SINGLE_PAGE)
    {
        goto _PROCESS_BLOCK_ERASE;
    }

    /* send block erase verify command */
    //wait_CMD_ready();
    NAND_REG_WRITE(NAND_CMD_FSM,NAND_CMD_ERASE_D0);
    wait_sm_finish();

    /* release nand flash chip select, and pin */
    _NandReleaseDevice();

    return TRUE;
}


/*
*********************************************************************************************************
*                       ERASE SUPER BLOCK
*
*Description: erase super block.
*
*Arguments  : ZoneNum   the zone number that the super block belong to;
*             SupBlkNum the super block nuber;
*
*Return     : result
*                   TRUE    -   erase ok;
*                   FALSE   -   erase failed;
*********************************************************************************************************
*/
INT32U  PHY_EraseSuperBlk(struct PhysicOpParameter *PhyOpPar)
{
    INT32S i;
    INT32U tmpBnkCnt;
    INT32U tmpBnkNum;
    struct PhysicOpParameter tmpNandOpPar;
    INT32U result = TRUE;

    PHY_DBG("PHY_DBG: Erase Super Block, BlkNum:0x%x\n", PhyOpPar->PhyBlkNumInBank);

    tmpNandOpPar = *PhyOpPar;

    /* calculate the bank total */
    tmpBnkCnt = PAGE_CNT_PER_LOGIC_BLK / PAGE_CNT_PER_PHY_BLK;
    tmpBnkNum = tmpNandOpPar.BankNum;

    for(i=0; i<(INT32S)tmpBnkCnt; i++)
    {
        tmpNandOpPar.BankNum = tmpBnkNum + i;
        tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SUPER_PAGE;
        /* erase the block */
        PHY_EraseNandBlk(&tmpNandOpPar);
    }

    /* check the erase result */
    for(i=0; i<(INT32S)tmpBnkCnt; i++)
    {
        result = PHY_SyncNandOperation(tmpBnkNum + i);
        if(result != TRUE)
        {
            PHY_ERR("PHY_ERR: erase super block failed!\n");
            return result;
        }
    }

    return result;
}


#if 1
INT32S nand_erase_all(void)
{
    struct PhysicOpParameter tmpNandOpPar;
    INT32S i;
    PHY_ERR("erase all blocks\n");

    tmpNandOpPar.BankNum = 0;
    tmpNandOpPar.SectorBitmapInPage = 0xf;
    tmpNandOpPar.PageNum = 0;

    for (i = 0; i < (NandDevInfo.NandFlashInfo->TotalBlkNumPerDie) *
            (NandDevInfo.NandFlashInfo->DieCntPerChip); i++)
    {
        tmpNandOpPar.PhyBlkNumInBank = i;
        PHY_EraseNandBlk(&tmpNandOpPar);
        PHY_SyncNandOperation(0);
    }

    return 0;
}
#endif

EXPORT_SYMBOL(PHY_NandReset);
