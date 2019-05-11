/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : nand_base.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-7-18 14:28
*********************************************************************************************************
*/

#include "nand_flash_driver_type.h"
#include "nand_reg_def-1201.h"
#include "do_dma.h"


/* backward compatibility */
#define  SRAMOC_CTL        (0xB0030000 + 0x00)

#if __KERNEL__

#if CONFIG_AM_CHIP_ID==1203
	static INT32U F_MF0Val,F_MF1Val,F_MF2Val;
#elif CONFIG_AM_CHIP_ID==1213
	static INT32U F_MF2Val,F_MF3Val,F_MF4Val;
#else
	static INT32U F_MF3Val,F_MF4Val,F_MF5Val;
#endif

void Nand_GetMultiPin(void);
void NAND_RealseMultiPin(void);
#else
#include "syscfg.h"
#include "r4ktlb.h"

#endif

extern INT32U NandDMAChannel;
extern INT32U bPageCnt;
extern INT32U bECCCnt;
extern struct  _Debug_Wear  WearDbg;

INT32U wTotalByte =0x00;
INT32U wTotalBit=0x00;

extern INT32S decode_data(INT8U * data, INT8U * udata, INT8U * parity);

INT8U GetBitNum(INT8U val)
{
	INT8U iLoop;
	INT8U cnt=0;
	for(iLoop=0x00;iLoop<8;iLoop++)
	{
		if(val&(1<<iLoop))
		{
			cnt++;
		}
	}
	return cnt;	
}

/*
*********************************************************************************************************
*               SET NAND ACCESS CHANNEL TO SPECIAL CHANNEL
*
*Description: set nand access channel to special channel.
*
*Arguments  : MemoryAddr    the memory address DMA will access
*
*Return     : none
*********************************************************************************************************
*/
void    _BusToSpecialMode(void *MemoryAddr)
{
    /* set nand controller access channel to special mode */
    NAND_REG_WRITE(NAND_CTL, NAND_REG_READ(NAND_CTL) | NAND_Specail_BUS);

    /* DSPMemory need set to special channel when accessed by special dma */
    if (IS_DSPMEM_ADDR(MemoryAddr))
    {
        REG_WRITE(SRAMOC_CTL, REG_READ(SRAMOC_CTL) | (1 << 19));
    }
}

/*
*********************************************************************************************************
*               SET NAND ACCESS CHANNEL TO NORMAL CHANNEL
*
*Description: set nand access channel to special channel.
*
*Arguments  : MemoryAddr    the memory address DMA will access
*
*Return     : none
*********************************************************************************************************
*/
void    _BusToNormalMode(void *MemoryAddr)
{
    /* set nand controller access channel to special mode */
    NAND_REG_WRITE(NAND_CTL, NAND_REG_READ(NAND_CTL) & ~NAND_Specail_BUS);

    /* DSPMemory need set to special channel when accessed by special dma */
    if (IS_DSPMEM_ADDR(MemoryAddr))
    {
        REG_WRITE(SRAMOC_CTL, REG_READ(SRAMOC_CTL) & ~(1 << 19));
    }
}

/*
*********************************************************************************************************
*               CONFIG DMA TO TRANSFER DATA FROM NAND FIFO TO RAM
*
*Description: cofig dma to transfer data from nand fifo to ram.
*
*Arguments  : nDmaNum    the dma channel number;
*             Mem_Addr   the ram address where store the data read from nand fifo;
*             ByteCount  the length of the data need transfer;
*
*Return     : none
*********************************************************************************************************
*/
void    _NandDmaReadConfig(INT32U nDmaNum, void* Mem_Addr, INT32U ByteCount)
{

        /*2008-1-2 add set DMA_CMD before transfers */
        reset_dma(nDmaNum);
        REG_WRITE(DMA_IOMAP_BASE + 0x0114 + nDmaNum * 0x0020, 0x0);          /* when low to high, DMA will start            */

        if (!IS_DSPMEM_ADDR(Mem_Addr))
        {		
		/* write back and invalid the cache */
		dma_cache_wback_inv((INT32U)((INT32U)Mem_Addr & ~MASK_COST_Read), ByteCount);

		if ((INT32U)Mem_Addr % 4 == 0)
		{
			/* address is 32bit mode,transfer by 32bit */
		#if BUS_DMA_BlockMode_Test==1
			set_dma_mode(nDmaNum, FlashDMA_R_SDR_32);	/*burst 8 error*/
		#else
			set_dma_mode(nDmaNum, 0x008401a4);
			//  set_dma_mode(nDmaNum, FlashDMA_R_SDR_32);	/*burst 8 error*/
		#endif
		}
		else
		{
		/* address is 8bit mode,transfer by 8bit   */
		/*2008-1-4 Nandflash must use 32bit mode*/	
	#if BUS_DMA_BlockMode_Test==1
			set_dma_mode(nDmaNum, FlashDMA_R_SDR_8);
	#else
			//set_dma_mode(nDmaNum, 0x008001a0);	   
			set_dma_mode(nDmaNum, 0x008001a4);
			//set_dma_mode(nDmaNum, 0xa08001a4);		/*burst 8 error*/
	#endif
		}
        }/*end of if (!IS_DSPMEM_ADDR(Mem_Addr))*/
        else{
                
                if ((INT32U)Mem_Addr % 4 == 0)
                {
                        /* address is 32bit mode,transfer by 32bit */
                        set_dma_mode(nDmaNum, 0x008c01a4);
                }
                else
                {
                        /* address is 8bit mode,transfer by 8bit   */
                        //set_dma_mode(nDmaNum, 0x008801a0);
                        set_dma_mode(nDmaNum, 0x008801a4);	
                }
        }/* end of else */

        /* set dma source address */
        set_dma_src_addr(nDmaNum, NAND_IOMAP_PHY_BASE + NAND_DATA);
        /* set dma dstination address */
        set_dma_dst_addr(nDmaNum, (INT32U)Mem_Addr & ~0xe0000000);
        /* set dma byte count */
        set_dma_count(nDmaNum, ByteCount);
}

/*
*********************************************************************************************************
*               CONFIG DMA TO TRANSFER DATA FROM RAM TO NAND FIFO
*
*Description: cofig dma to transfer data from ram to nand fifo.
*
*Arguments  : nDmaNum    the dma channel number;
*             Mem_Addr   the ram address where store the data wrtie to nand fifo;
*             ByteCount  the length of the data need transfer;
*
*Return     : none
*********************************************************************************************************
*/
void    _NandDmaWriteConfig(INT32U nDmaNum, void* Mem_Addr, INT32U ByteCount)
{
        /*2008-1-2 add set DMA_CMD before transfers */
        reset_dma(nDmaNum);	
        REG_WRITE(DMA_IOMAP_BASE + 0x0114 + nDmaNum * 0x0020, 0x0);          /* when low to high, DMA will start            */

        if (!IS_DSPMEM_ADDR(Mem_Addr))
        {
		//#ifdef CACHE_WBACK_DRIVER			
		/* write back and invalid the cache */   	
		{
			dma_cache_wback_inv((INT32U)((INT32U)Mem_Addr & ~MASK_COST_Write), ByteCount);
		}

		if ((INT32U)Mem_Addr % 4 == 0)
		{
			/* address is 32bit mode,transfer by 32bit */
			//   set_dma_mode(nDmaNum, 0x01a40084);	
	#if BUS_DMA_BlockMode_Test==1
			set_dma_mode(nDmaNum, FlashDMA_W_SDR_32);	
	#else
			set_dma_mode(nDmaNum, 0x01a40084);	
	#endif
		}
		else
		{
			/* address is 8bit mode,transfer by 8bit   */
			/* address is 8bit mode,transfer by 8bit   */

			/*2008-1-4 Nandflash must use 32bit mode*/
			//set_dma_mode(nDmaNum, 0x01a00080);
	#if BUS_DMA_BlockMode_Test==1
			set_dma_mode(nDmaNum, FlashDMA_W_SDR_8);	
	#else
			set_dma_mode(nDmaNum, 0x01a40080);
	#endif
	
		}
        }/*end of if (!IS_DSPMEM_ADDR(Mem_Addr))*/
        else
        {
                if ((INT32U)Mem_Addr % 4 == 0)
                {
                        /* address is 32bit mode,transfer by 32bit */
                        set_dma_mode(nDmaNum, 0x01a4008c);
                }
                else
                {
                        /* address is 8bit mode,transfer by 8bit   */
                        //set_dma_mode(nDmaNum, 0x01a00088);
                        /*2008-1-4 Nandflash must use 32bit mode*/
                        set_dma_mode(nDmaNum, 0x01a40088);
                }
        }/* end of else */
        
        /* set dma source address */
        set_dma_src_addr(nDmaNum, (INT32U)Mem_Addr & ~0xe0000000);

        /* set dma dstination address */
        set_dma_dst_addr(nDmaNum, NAND_IOMAP_PHY_BASE + NAND_DATA);
        /* set dma byte count */
        set_dma_count(nDmaNum, ByteCount);
}
/*
*********************************************************************************************************
*                       ERROR CHECK AND CORRECT BY RS CODE
*
* Description: check and correct data error by BCH code.if the data read out of nand flash is not the
*              as what it is write in,the hardware ecc module will report the error.
*
* Aguments   : srambuffer   memory address which save the data which need to be checked and corrected.
*
* Returns    : the result of error checking and correcting. 0 - success; 1 - fail.
*
* Note       : this module does't correct the userdata error now.
*********************************************************************************************************
*/
void   Debug_Flash_Register(void)
{
    INIT_BOOT("NAND_ECCCTL:%x,",NAND_REG_READ(NAND_ECCCTL));
    INIT_BOOT("0x%x,", NAND_REG_READ(NAND_ROWADDRLO) );
    INIT_BOOT("Block:0x%x,", NAND_REG_READ(NAND_ROWADDRLO) /NandStorageInfo.PageNumPerPhyBlk );
    INIT_BOOT("Page:0x%x,", NAND_REG_READ(NAND_ROWADDRLO) %NandStorageInfo.PageNumPerPhyBlk);
	INIT_BOOT("#nandBC:0x%x,", NAND_REG_READ(NAND_BC) );
	INIT_BOOT("#nandstatus:0x%x\n\n", NAND_REG_READ(NAND_STATUS) );
}
void   Debug_Flash_Register2(INT32U error_cn)
{
    INT16U i,error_pos;

    Debug_Flash_Register();
    
    for (i = 0; i < error_cn; i++)
    {
        error_pos =  (NAND_REG_READ(NAND_ECCEX0 + 4 * i) >> 8) & 0x07ff;
        INIT_BOOT("No:%x,%x,%x\n",i,NAND_REG_READ(NAND_ECCEX0 + 4 * i),error_pos );
    }
}
INT32S bch24_ecc_correct(INT8U* srambuffer, INT8U* oob_buf)
{
    INT32S i;
    INT32S error_cn ;	/*error count reported by ECCCTLs*/
    INT32S error_pos; /*error position*/
    INT8U Reclaim_Bit,SaveReclaim;     
    INT8U EccPrint;
    INT16U BCHData;	
    INT16U Error_Bit,ErrBit;
	SaveReclaim =0x00;
    if (NAND_REG_READ(NAND_STATUS) & NAND_STATUS_DAFF)	/*data is all 0xff*/
    {       

        if(NAND_SUPPORT_PN && NAND_REG_READ(NAND_CTL) &(1<<31))
        {
            if(NAND_REG_READ(NAND_ECCCTL) &(3<<9))  // 1KB  BHC24
            {
                BCHData =1024;	
            }
            else
            {
                BCHData =512;	
            }
            MEMSET(srambuffer,0xff,BCHData);
        }

        return 0;
    }

#if CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213
	error_cn = (NAND_REG_READ(NAND_ECCCTL) >> 26) & 0x3f; 
#else
    error_cn = (NAND_REG_READ(NAND_ECCCTL) >> 27) & 0x1f;    
#endif
    if (!error_cn)    /* hardware doesn't report error        */
    {
        return 0;    
    }    

#if CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213   
    if (error_cn == 0x3f) 	/*too much error , or data is all oxff .*/
#else
	if (error_cn == 0x1f) 	/*too much error , or data is all oxff .*/
#endif
    {     
#if ECC_OVER_MODE ==0x01	
      INIT_BOOT("<%s>_OVER_ECC_<0x%x>_<0x%x>_<0x%x>\n",\
      __func__,NAND_REG_READ(NAND_ECCCTL),NAND_REG_READ(NAND_ROWADDRLO),NAND_REG_READ(NAND_MAIN_COLADDR));
      //Debug_Flash_Register();	
#endif	
    
    return NAND_DATA_ECC_ERROR;

    }
#if 0
	INIT_BOOT("\n==_<%s>_<%d>_OVER_ECC_%dBit ==\n",__func__,__LINE__,error_cn);
	Debug_Flash_Register();	
	return 0x00;
#endif

#if  0//(defined(__KERNEL__))	
	if(error_cn>=0x01  )
	{
		printf("\n==_<%s>_<%d>_RECLAIM_ECC_%dBit ==\n",__func__,__LINE__,error_cn);
		Debug_Flash_Register();  

	}
#endif

    Error_Bit =0x00;
    // if(NAND_REG_READ(NAND_ECCCTL) &(1<<9))  // 1KB  BHC24 
    if(BCH_MODE ==ECC_Type_12BitECC || BCH_MODE ==ECC_Type_24BitECC||BCH_MODE == ECC_Type_40BitECC || BCH_MODE ==ECC_Type_60BitECC )
    {
        BCHData =1024;
        /* can corrected errors */
        for (i = 0; i < error_cn; i++)                               
        {
            error_pos = (NAND_REG_READ(NAND_ECCEX0 + 4 * i) >> 8) & 0x07ff;
#if CONFIG_AM_CHIP_ID == 1211  
            ErrBit =  GetBitNum(NAND_REG_READ(NAND_ECCEX0 + 4 * i)&0xff);
            Error_Bit	+= ErrBit;
#endif
            if (error_pos <= 0x3ff)
            {
                /* correct the error in main area */            
                srambuffer[error_pos] ^= (NAND_REG_READ(NAND_ECCEX0 + 4 * i) & 0x0ff);
            }
            else if (error_pos <= 0x401 && oob_buf != NULL) // 0x1FF+2
            {
                /* correct the error in oob data area */
                oob_buf[error_pos - 0x400] ^= (NAND_REG_READ(NAND_ECCEX0 + 4 * i) & 0x0ff);
            }
            else 
            {
                /*skip error in ECC area*/
                continue;
            }
        }
    }
    else 
    {  //BCH8
        BCHData=512;
        /* can corrected errors */
        for (i = 0; i < error_cn; i++)                               
        {
#if CONFIG_AM_CHIP_ID == 1211        
            ErrBit =  GetBitNum(NAND_REG_READ(NAND_ECCEX0 + 4 * i)&0xff);
            Error_Bit	+= ErrBit;
#endif
            error_pos = (NAND_REG_READ(NAND_ECCEX0 + 4 * i) >> 8) & 0x03ff;

            if (error_pos <= 0x1ff)
            {
                /* correct the error in main area */            
                srambuffer[error_pos] ^= (NAND_REG_READ(NAND_ECCEX0 + 4 * i) & 0x0ff);
            }
            else if (error_pos <= 0x201 && oob_buf != NULL) // 0x1FF+2
            {
                /* correct the error in oob data area */
                oob_buf[error_pos - 0x200] ^= (NAND_REG_READ(NAND_ECCEX0 + 4 * i) & 0x0ff);
            }
            else 
            {
                /*skip error in ECC area*/
                continue;
            }
        }
    }
    
    /* if the srambuffer is cacheaddr,...       * need to flush the data back to sdram 
    */
    if (IS_CACHE_ADDR(srambuffer)) 
    {
        //  dma_cache_wback_inv( KVA_TO_PA(srambuffer), BCHData);
        dma_cache_wback_inv((INT32U) srambuffer&~MASK_COST_Read, BCHData);
    }

#if CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

	Error_Bit = (NAND_REG_READ(NAND_ECCCTL) >> 16) & 0x3f;
#endif
    WearDbg.Error_Bit += Error_Bit;
	WearDbg.Error_Byte+= error_cn;
	
    /* read reclaim enabled by now *///@fish add 
    Reclaim_Bit =0x00;
    EccPrint = 0x00;
    switch(BCH_MODE)
    {
        case ECC_Type_8BitECC: //BCH8 512Byte
            Reclaim_Bit = RECLAIM_VAL;
            EccPrint =8;
            break;
        case ECC_Type_12BitECC:
            //BCH12 512B ==>BCH24 1KB  reclaim val *2
            Reclaim_Bit = RECLAIM_VAL*2;
            EccPrint =16;
            break;
        case ECC_Type_24BitECC:
		case ECC_Type_40BitECC:
            Reclaim_Bit = RECLAIM_VAL*2;
            EccPrint =16;
            break;
		case ECC_Type_60BitECC:
			Reclaim_Bit = RECLAIM_VAL*2;
			EccPrint =24;
			break;

        default:
            break;
    }
//	INIT_BOOT("\n==_<%s>_<%d>_ECC_%dbit_%dByte ==\n",__func__,__LINE__,Error_Bit,error_cn);
//	Debug_Flash_Register(); 
	
    if (Error_Bit >= EccPrint)	  
    {	
#if ECC_READ_RECLAIM ==0x01      
        printf("\n==_<%s>_<%d>_RECLAIM_ECC_%dBit ==\n",__func__,__LINE__,error_cn);
        Debug_Flash_Register();   
#endif   
    } 
        
#if 0	
    if(Error_Bit != error_cn)
    {
        INIT_BOOT("\n==_<%s>_<%d>_ECC_%dbit_%dByte ==\n",__func__,__LINE__,Error_Bit,error_cn);
        Debug_Flash_Register();
        for (i = 0; i < error_cn; i++)
        {
            error_pos =  (NAND_REG_READ(NAND_ECCEX0 + 4 * i) >> 8) & 0x07ff;
            ErrBit =  GetBitNum(NAND_REG_READ(NAND_ECCEX0 + 4 * i)&0xff);
            INIT_BOOT("No:%x,reg:%5x,",i,NAND_REG_READ(NAND_ECCEX0 + 4 * i));            
             INIT_BOOT("bit:%x,data:%x\n",   ErrBit, srambuffer[error_pos]);
        }

    }
#endif
/*@fish add 2011-02-09 修改error bit ，判断条件修改，
if(Error_Bit>=4) 不判断
**/

    PHY_BCH_CNT(BCH_SET_VAL,Error_Bit);

    if (Error_Bit >= Reclaim_Bit)	  
    {		
        return NAND_DATA_ECC_LIMIT;	
    }
    else if(Error_Bit>=4)
    {	
        PHY_Set_BCHRes(NAND_DATA_ECC_LIMIT2);  
    }  
    
    return 0;

        
}


INT32S bch_ecc_correct(INT8U* srambuffer, INT8U* oob_buf)
{

#if CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213    

    return    bch24_ecc_correct(srambuffer,oob_buf);    
#else
    INT32S i;
    INT32S error_cn ;	/*error count reported by ECCCTLs*/
    INT32S error_pos; /*error position*/
    INT8U MAX_ECC_Err;   

        
    if (NAND_REG_READ(NAND_STATUS) & NAND_STATUS_DAFF)	/*data is all 0xff*/
    {           
        return 0;
    }       
    error_cn = (NAND_REG_READ(NAND_ECCCTL) >> 28) & 0xF;
  
    if (!error_cn) 		 /* hardware doesn't report error        */
    {
        return 0;    
    }
        
    if (error_cn == 0xF) 	/*too much error , or data is all oxff .*/
    {                  
        if (CHIP_TYPE == AM_CHIP_7331)
            return NAND_DATA_ECC_ERROR;
        else
            return 0;	/*too much error ,can't correct*/          
    }

    /* can corrected errors */
    for (i = 0; i < error_cn; i++)                               
    {
        error_pos = (NAND_REG_READ(NAND_ECCEX0 + 4 * i) >> 8) & 0x03ff;

        if (error_pos <= 0x1ff)
        {
            /* correct the error in main area */            
            srambuffer[error_pos] ^= (NAND_REG_READ(NAND_ECCEX0 + 4 * i) & 0x0ff);
        }
        else if (error_pos <= 0x202 && oob_buf != NULL)
        {
            /* correct the error in oob data area */
            oob_buf[error_pos - 0x200] ^= (NAND_REG_READ(NAND_ECCEX0 + 4 * i) & 0x0ff);
        }
        else 
        {
            /*skip error in ECC area*/
            continue;
        }
    }
        

    /* if the srambuffer is cacheaddr,...   
    * need to flush the data back to sdram 
    */
    if (IS_CACHE_ADDR(srambuffer)) 
    {
      //  dma_cache_wback_inv( KVA_TO_PA(srambuffer), 0x200);
        dma_cache_wback_inv( srambuffer, 0x200);
    }

    /* read reclaim enabled by now */
    if(NAND_REG_READ(NAND_ECCCTL) &(1<<8))  // 512Byte/BCH12
    {
         MAX_ECC_Err =9;               
    }
    else
    {
        MAX_ECC_Err =6;
    }
    
    if (error_cn >= MAX_ECC_Err)	  
    {	
        printf("\n==_<%s>_<%d>_RECLAIM_ECC_%dBit ==\n",__func__,__LINE__,error_cn);
        Debug_Flash_Register();   
        return NAND_DATA_ECC_LIMIT;	
    }  
#endif		
        return 0;
}



/*
*********************************************************************************************************
*                       CALCULATE PHYSIC BLOCK NUMBER IN CHIP
*
* Description: calculate the physic block number in chip by block number in bank
*
* Aguments   : ChipNum          the pointer to the chip number;
*              BlkNumInChip     the pointer to the block number in chip;
*              BankNum          the bank number current operation;
*              BlkNumInBank     the physic block number in bank
*
* Returns    : result
*               TRUE    calculate parameters ok;
*               FALSE   calculate parameters failed;
*
* Note       : this function translate the block number from bank based to chip based,
*********************************************************************************************************
*/
INT32U  _CalPhyBlkNumByBlkNumInBank(INT32U *ChipNum, INT32U *BlkNumInChip, INT32U BankNum, INT32U BlkNumInBank)
{
    INT32U  tempChipNum;
    INT32U  tempBlkNumInChip;
    INT32U  tempBankNumInChip;
    INT32U  tempBankBlkNumBase;

    /* calculate the chip number and the bank number in chip */
    tempChipNum = BankNum / NandStorageInfo.BankCntPerChip;
    tempBankNumInChip = BankNum % NandStorageInfo.BankCntPerChip;

    PHY_DBG("PHY_DBG: Block Address Translate, BankNum is:0x%x, BlkNumInBank is: 0x%x\n", BankNum, BlkNumInBank);

    /* calculate the blocks base address in the bank */
    if (NAND_SUPPORT_INTERNAL_INTERLEAVE)
    {
        tempBankBlkNumBase = tempBankNumInChip * NandStorageInfo.TotalBlkNumPerDie;
    }
    else
    {
        tempBankBlkNumBase = 0;
    }

    /* calculate block number in chip */
    if (NAND_SUPPORT_MULTI_PLANE)
    {
        if(MULTI_BLOCK_ADDR_OFFSET == 1)
        {
            /* if the multi-block offset is 1, the real block address is just
               plane cont multiple block number in bank */
            tempBlkNumInChip = tempBankBlkNumBase + NandStorageInfo.PlaneCntPerDie * BlkNumInBank;
        }
        else
        {
            /* if the multi-block offset is not 1, the read block address can't be
               calculate by the plane count multiple block number in bank */
            INT32U tempBlkNumPerDie, tempDieNum, tempBlkNumInDie;

            /* calculate the multi-block count of a die */
            tempBlkNumPerDie = NandStorageInfo.TotalBlkNumPerDie / NandStorageInfo.PlaneCntPerDie;

            /* calculate which die this block belong to in the chip */
            tempDieNum = BlkNumInBank / tempBlkNumPerDie;

            /* calculate the block number in die */
            tempBlkNumInDie = BlkNumInBank - tempBlkNumPerDie * tempDieNum;

            /* calculate the block number in chip */
            tempBlkNumInChip = tempBankBlkNumBase + NandStorageInfo.TotalBlkNumPerDie * tempDieNum + tempBlkNumInDie;
        }
    }
    else
    {
        tempBlkNumInChip = tempBankBlkNumBase + BlkNumInBank;
    }

    PHY_DBG("PHY_DBG:                          ChipNum is:0x%x, BlkNumInChip is: 0x%x\n", tempChipNum, tempBlkNumInChip);

    /* transfer the value of result to uplayer */
    *ChipNum = tempChipNum;
    *BlkNumInChip = tempBlkNumInChip;

    return TRUE;
}


INT32U Conventional_dev_list []={0x1d00f19b,0xffffffff};

void Set_NAND_CTL_Access_Mode(INT8U *nand_id)
{
	INT32U i;

	for(i=0;;i++){ 
		if(Conventional_dev_list[i]==0xffffffff)
			break;
		if(*(INT32U*)nand_id==Conventional_dev_list[i]){
			NAND_REG_WRITE(NAND_CTL,NAND_REG_READ(NAND_CTL)&(~NAND_CTL_MODE_EDO));
			//INIT_BOOT("NAND_CTL:%x %x\n",NAND_REG_READ(NAND_CTL),NAND_REG_READ(NAND_CLKCTL));
			break;
		}
	}
}


INT32U  Set_NAND_ECCCTL(INT16U EccType,INT8U Flag)
{
        INT32U Reg;
        Reg = NAND_REG_READ(NAND_ECCCTL);
	if(Flag)
                INIT_BOOT("\n===Set:0x%x, NAND_ECCCTL:0x%x,\n",EccType,Reg);
#if CONFIG_AM_CHIP_ID==1203  
 	switch(EccType)
        {
		case ECC_Type_8BitECC:		
			NAND_REG_WRITE(NAND_ECCCTL, (1<<0));//			
			if(Flag)
			      INIT_BOOT("Set ECC type BCH8 \n");
			break;
		case ECC_Type_12BitECC:
		case ECC_Type_24BitECC:				
			NAND_REG_WRITE(NAND_ECCCTL, (1<<8)|(3<<3)  );			
			//bch12,512+23byte
			INIT_BOOT("Set ECC type BCH12 \n");
			break;
		default:
			break;
 	}

#elif CONFIG_AM_CHIP_ID==1211 || CONFIG_AM_CHIP_ID == 1220 || CONFIG_AM_CHIP_ID == 1213

	switch(EccType)
        {
		case ECC_Type_8BitECC:
			NAND_REG_WRITE(NAND_ECCCTL, (1<<0));//
			if(Flag)
				INIT_BOOT("AM%d Set ECC type BCH8 \n",CONFIG_AM_CHIP_ID);
			break;
		case ECC_Type_12BitECC:
		case ECC_Type_24BitECC:	
			if(SP_USE_BYTE ==2) //user 2Byte 
			{
				// 1KB+44Byte(3 user Byte+42 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (1<<9)|(3<<3)|(1<<0) );
			}
			else if (SP_USE_BYTE ==3) //user Byte 2Byte
			{
				// 1KB+45Byte(3 user Byte+42 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (1<<9)|(4<<3)|(1<<0) );
			}
			else
			{
				NAND_REG_WRITE(NAND_ECCCTL, (1<<9)|(2<<3)|(1<<0) );			
			}
			//bch12,1KB+2+42byte
			if(Flag)
			     INIT_BOOT("AM%d Set ECC type BCH24 \n",CONFIG_AM_CHIP_ID);
			break;
		case ECC_Type_40BitECC:
			if(SP_USE_BYTE ==2) //user 2Byte 
			{
				// 1KB+72Byte(3 user Byte+70 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (2<<9)|(6<<3)|(1<<0) );
			}
			else if (SP_USE_BYTE ==3) //user Byte 2Byte
			{
				// 1KB+73Byte(3 user Byte+70 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (2<<9)|(7<<3)|(1<<0) );
			}
			else  //user Byte 0Byte
			{
				// 1KB+73Byte(3 user Byte+70 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (2<<9)|(5<<3)|(1<<0) );
			}
			break;
		case ECC_Type_60BitECC:
			if(SP_USE_BYTE ==2) //user 2Byte 
			{
				// 1KB+72Byte(3 user Byte+70 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (3<<9)|(9<<3)|(1<<0) );
			}
			else if (SP_USE_BYTE ==3) //user Byte 2Byte
			{
					// 1KB+73Byte(3 user Byte+70 parity Data )
				NAND_REG_WRITE(NAND_ECCCTL, (3<<9)|(10<<3)|(1<<0) );
			}
            else 
            {
            	NAND_REG_WRITE(NAND_ECCCTL, (3<<9)|(8<<3)|(1<<0) );
            }
		if(Flag)
			     INIT_BOOT("AM%d Set ECC type BCH60 \n",CONFIG_AM_CHIP_ID);
			break;

		default:
			break;
 	}
#endif     
       if(Flag)	
      	   INIT_BOOT("NAND_ECCCTL:0x%x == \n\n", NAND_REG_READ(NAND_ECCCTL));
        return 0x00;
}

INT32U Set_BITMAP_INFO(void)
{
    struct BitMapInfo   *tmpBITMAP;	

    tmpBITMAP = NandDevInfo.BITMAP_DATA;

	tmpBITMAP->USER_DataByte =0x02;
	tmpBITMAP->USER_SectorCnt=0x03;
	tmpBITMAP->TOTAL_SPAREDATA = 0x02*0x03;
	
	switch(BCH_MODE)
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
    
  //  tmpBITMAP->BCH8_Reclaim = 0x6;
  //  tmpBITMAP->BCH12_Reclaim= 0x3;
  //  tmpBITMAP->BCH24_Reclaim= 0x14;
    
    switch(BCH_MODE)	
	{
		case ECC_Type_24BitECC: //for AL1203,AL1207.			
			SET_READ_Reclaim = RECLAIM_VAL;				
			break;
		case ECC_Type_40BitECC:
			SET_READ_Reclaim = RECLAIM_VAL;
			break;
		case ECC_Type_60BitECC: //for AL1203,AL1207.			
			SET_READ_Reclaim = RECLAIM_VAL; 			
			break;

		case ECC_Type_12BitECC:
			SET_READ_Reclaim = RECLAIM_VAL*2;	
			break;			
		case ECC_Type_8BitECC:
			break;		
		default :
			break;
	}
    INIT_BOOT("RECLAIM_VAL:%d bitECC,set:%d bit\n",RECLAIM_VAL,SET_READ_Reclaim);
    return 0x00;
	
}

/*
*********************************************************************************************************
*                       NAND GET DEVICE
*
* Description: enable the nand controller, and set the chip select, and set nand
*              flash pin mode.
*
* Aguments   : ChipNum          the chip select, which need enable;
*
* Returns    : result
*               TRUE    get device ok;
*               FALSE   get device failed;
*
* Note       : this function enable controller,set the chip seclect,and set the pin
*              owner to nand if needed.
*********************************************************************************************************
*/
INT32U  _NandGetDevice(INT32U ChipNum)
{
    INT32U tmpNandCtl;
    INT32U tmpChip;

#if __KERNEL__
    Flash_Disable_Irq();
    Nand_GetMultiPin();
#endif

    if(ChipNum >= NAND_MAX_CHIP_NUM)
    {
        /* the chip number is invalid */
        PHY_ERR("PHY_ERR: nand chip number 0x%x is invalid when selected chip!\n", ChipNum);
        return PHYSICAL_LAYER_ERROR;
    }
    /* ChipNum is start from 0, but nand /ce start from 1 */
    tmpChip = ChipNum ;		//ChipNum + 1;

    /* set the chip enable mask */
    tmpChip = 1 << (3 + tmpChip);

    /* set the chip enable, and enable nand controller */
    tmpNandCtl = NAND_REG_READ(NAND_CTL);
    tmpNandCtl |= (tmpChip | NAND_CTL_EN);
    NAND_REG_WRITE(NAND_CTL, tmpNandCtl);
    
#if CONFIG_AM_CHIP_ID==1203
    REG_WRITE(0xb01c0094,REG_READ(0xb01c0094)|0x01);
#elif CONFIG_AM_CHIP_ID==1211
    REG_WRITE(0xb01c0094,REG_READ(0xb01c0094)|1<<6);
#elif CONFIG_AM_CHIP_ID==1213
    REG_WRITE(0xb01c0094,REG_READ(0xb01c0094)|7);

#endif
    return TRUE;
}

/*
*********************************************************************************************************
*                       NAND GET DEVICE
*
* Description: disable the nand controller, disable the chip select, and release pin.
*
* Aguments   : none
*
* Returns    : result
*               TRUE    release device ok;
*               FALSE   release device failed;
*********************************************************************************************************
*/
INT32U  _NandReleaseDevice(void)
{
    INT32U tmpNandCtl;

    tmpNandCtl = NAND_REG_READ(NAND_CTL);

    /* clear all chip select mask */
    tmpNandCtl &= ~(0xf<<3);

    /* disable nand controller */
    tmpNandCtl &= ~NAND_CTL_EN;

    NAND_REG_WRITE(NAND_CTL, tmpNandCtl);

#if __KERNEL__

    NAND_RealseMultiPin();
    Flash_Enable_Irq();

#endif

#if CONFIG_AM_CHIP_ID==1203
    REG_WRITE(0xb01c0094,REG_READ(0xb01c0094)&(0xFFFFFFFE));
#elif CONFIG_AM_CHIP_ID==1211 
    REG_WRITE(0xb01c0094,REG_READ(0xb01c0094)&(~(1<<6)));
#elif CONFIG_AM_CHIP_ID==1213 
    REG_WRITE(0xb01c0094,REG_READ(0xb01c0094)&(~(7)));

#endif
    return TRUE;
}


#if SUPPORT_DMA_TRANSFER_DATA_ON_RAM

/*
*********************************************************************************************************
*                       CONFIG DMA FOR TRANSFERRING DATA ON RAM
*
* Description: config dma for transfer data between ram
*
* Aguments   : nDmaNum      dma number which current using
*              SrcAddr      source address on ram which data be transferred from;
*              DstAddr      destination address on ram which data be transferred to;
*              ByteCnt      the byte count of data need be transfferred;
*
* Returns    : none
*********************************************************************************************************
*/
static void _DmaConfigForRam(INT32U nDmaNum, void* SrcAddr, void* DstAddr, INT32U ByteCnt)
{

        INT32U tmpValue;

        /* if source address is sdram or cache,flush cache data */
        if(!IS_DSPMEM_ADDR(SrcAddr))
        {   
               dma_cache_wback_inv(((INT32U)SrcAddr) & ~MASK_COST_Read, ByteCnt);              
        }
        /* if destination address is sdram or cahch, flush it */
        if(!IS_DSPMEM_ADDR(DstAddr))
        {                
            dma_cache_wback_inv(((INT32U)SrcAddr) & ~MASK_COST_Read, ByteCnt);     
        }

        /* calculate dma transfer mode */
        if((((INT32U)SrcAddr) % 4 != 0) || (((INT32U)DstAddr) % 4 != 0))
        {
                tmpValue = 0x00800080;
        }
        else
        {
                tmpValue = 0x00840084;
        }

        if((INT32U)SrcAddr > 0xb0000000)
        {
                tmpValue |= 0x8;
        }
        if((INT32U)DstAddr > 0xb0000000)
        {
                tmpValue |= 0x80000;
        }

        /* address is 16bit mode,transfer by 16bit */
        set_dma_mode(nDmaNum, tmpValue);
        set_dma_src_addr(nDmaNum, ((INT32U)SrcAddr) & 0x1fffffff);
        set_dma_dst_addr(nDmaNum, ((INT32U)DstAddr) & 0x1fffffff);
        set_dma_count(nDmaNum, ByteCnt);
}

/*
*********************************************************************************************************
*                       TRANSFER DATA ON RAM BY DMA
*
* Description: transfer data on ram by dma
*
* Aguments   : SrcAddr      source address on ram which data be transferred from;
*              DstAddr      destination address on ram which data be transferred to;
*              ByteCnt      the byte count of data need be transfferred;
*
* Returns    : none
*********************************************************************************************************
*/
void DmaTransferDataForRam(void* SrcAddr, void* DstAddr, INT32S ByteCnt)
{
        INT32U tmpFlag = 0;
        

        if(SrcAddr == NULL || DstAddr == NULL)
        //    if(SrcAddr == NULL || (((INT32U)SrcAddr&0xe0000000) == 0x0) || DstAddr == NULL || (((INT32U)DstAddr&0xe0000000) == 0x0))
        {
                /* the buffer address is invalid */
                printf("xxxPHY_DBG: memory address is invalid!, SrcAddr: 0x%x, DstAddr: 0x%x\n", SrcAddr, DstAddr);
                return ;
        }
        /*@fish addd 2008-12-27
        * 对于DMA拷贝数据,Log to Phy地址转换统一到
        * set_dma_src_addr(),set_dma_Dst_addr统一设置.        */

        /* config dma for transferring */
        _DmaConfigForRam(NandDMAChannel, SrcAddr, DstAddr, ByteCnt);

        if (ISSPECIALDMA(NandDMAChannel))
        {
                /* check if need set dspram bus priority to special bus */
                if(IS_DSPMEM_ADDR(SrcAddr) || IS_DSPMEM_ADDR(DstAddr))
                {
                        tmpFlag ++;
                        REG_WRITE(SRAMOC_CTL, REG_READ(SRAMOC_CTL) | (1 << 19));
                }
        }

        /* start dma to start transfer data */
        start_dma(NandDMAChannel);

        /* wait dma transfer data completely */
        dma_end(NandDMAChannel);

        if(tmpFlag != 0)
        {
                /* set dspram bus to AHB Bus if necessary */
                REG_WRITE(SRAMOC_CTL, REG_READ(SRAMOC_CTL) & ~(1 << 19));
        }
}
#endif

#if __KERNEL__
void Nand_GetMultiPin(void)
{
  #if CONFIG_AM_CHIP_ID==1203
    
    F_MF0Val =0x00;
    if (CHIP_TYPE==AM_CHIP_7531)  //176 Pin
    {                       
        F_MF1Val=REG_READ(GPIO_MFCTL1);	
        F_MF2Val=REG_READ(GPIO_MFCTL2);
        REG_WRITE(GPIO_MFCTL1, (F_MF1Val&NF_MF1_MSK_7531)|NF_MF1_VAL_7531);
        if(MAX_CHIP_NUM==1)                     
            REG_WRITE(GPIO_MFCTL2, (F_MF2Val&NF_MF2_MSK_7531_1CE)|NF_MF2_VAL_7531_1CE); 
        else
            REG_WRITE(GPIO_MFCTL2, (F_MF2Val&NF_MF2_MSK_7531)|NF_MF2_VAL_7531); 
    }
    else if (CHIP_TYPE==AM_CHIP_7331)
    {                  
        F_MF2Val=REG_READ(GPIO_MFCTL2);	
        F_MF3Val=REG_READ(GPIO_MFCTL3);
        REG_WRITE(GPIO_MFCTL3, (F_MF3Val&NF_MF3_MSK_7331)|NF_MF3_VAL_7331);
        if(MAX_CHIP_NUM==1)                       
            REG_WRITE(GPIO_MFCTL2, (F_MF2Val&NF_MF2_MSK_7331_1CE)|NF_MF2_VAL_7331_1CE); 
        else                    
            REG_WRITE(GPIO_MFCTL2, (F_MF2Val&NF_MF2_MSK_7331)|NF_MF2_VAL_7331 ); 
    }
       
   
#elif CONFIG_AM_CHIP_ID==1211 
  //INT32U MF3Val,MF4Val,MF5Val;
    F_MF3Val=REG_READ(GPIO_MFCTL3);
    F_MF4Val=REG_READ(GPIO_MFCTL4);
    F_MF5Val=REG_READ(GPIO_MFCTL5);  
    if(MAX_CHIP_NUM == 1) {
        F_MF3Val=(F_MF3Val&NF_MF3_MSK_7555_1CE)|NF_MF3_VAL_7555_1CE;
        REG_WRITE(GPIO_MFCTL3, F_MF3Val); 
    }
    else {
		F_MF3Val=(F_MF3Val&NF_MF3_MSK_7555)|NF_MF3_VAL_7555;
        REG_WRITE(GPIO_MFCTL3, F_MF3Val);
    }
    F_MF4Val=(F_MF4Val&NF_MF4_MSK_7555)|NF_MF4_VAL_7555;
    REG_WRITE(GPIO_MFCTL4, F_MF4Val);
    F_MF5Val=(F_MF5Val&NF_MF5_MSK_7555)|NF_MF5_VAL_7555;
    REG_WRITE(GPIO_MFCTL5, F_MF5Val);
#elif CONFIG_AM_CHIP_ID == 1220 
	F_MF5Val=0x55555489;	

	REG_WRITE(GPIO_MFCTL5, F_MF5Val);
#elif    CONFIG_AM_CHIP_ID==1213//208&176pin

			F_MF2Val=REG_READ(GPIO_MFCTL2);
			F_MF3Val=REG_READ(GPIO_MFCTL3);  
			if(MAX_CHIP_NUM == 2) {
				F_MF2Val=(F_MF2Val&NF_MF2_MSK_8250_2CE)|NF_MF2_VAL_8250_2CE;
				REG_WRITE(GPIO_MFCTL2, F_MF2Val);
				F_MF3Val=(F_MF3Val&NF_MF3_MSK_8250_2CE)|NF_MF3_VAL_8250_2CE;
				REG_WRITE(GPIO_MFCTL3, F_MF3Val);
			}
			else if(MAX_CHIP_NUM == 1){
				F_MF2Val=(F_MF2Val&NF_MF2_MSK_8250_1CE)|NF_MF2_VAL_8250_1CE;
				REG_WRITE(GPIO_MFCTL2, F_MF2Val);
				F_MF3Val=(F_MF3Val&NF_MF3_MSK_8250_2CE)|NF_MF3_VAL_8250_2CE;
				REG_WRITE(GPIO_MFCTL3, F_MF3Val);	
			}
			else if(MAX_CHIP_NUM == 3){
				F_MF2Val=(F_MF2Val&NF_MF2_MSK_8250)|NF_MF2_VAL_8250;
				REG_WRITE(GPIO_MFCTL2, F_MF2Val);
				F_MF3Val=(F_MF3Val&NF_MF3_MSK_8250_2CE)|NF_MF3_VAL_8250_2CE;
				REG_WRITE(GPIO_MFCTL3, F_MF3Val);				
				}
			else {
				F_MF2Val=(F_MF2Val&NF_MF2_MSK_8250)|NF_MF2_VAL_8250;
				REG_WRITE(GPIO_MFCTL2, F_MF2Val);
				F_MF3Val=(F_MF3Val&NF_MF3_MSK_8250_2CE)|NF_MF3_VAL_8250_2CE;
				REG_WRITE(GPIO_MFCTL3, F_MF3Val);
			}
#else
    {
         #error " no chip id defined"
    }

#endif

	

}


void NAND_RealseMultiPin(void)
{
#if CONFIG_AM_CHIP_ID==1203

    if (CHIP_TYPE==AM_CHIP_7531) 
    {
        REG_WRITE(GPIO_MFCTL1, F_MF1Val);
        REG_WRITE(GPIO_MFCTL2, F_MF2Val);
    }
    else if (CHIP_TYPE==AM_CHIP_7331)
    {
        //       REG_WRITE(GPIO_MFCTL2, F_MF2Val);
        REG_WRITE(GPIO_MFCTL2, (F_MF2Val&~(7<<24))|(1<<24)); 
        REG_WRITE(GPIO_MFCTL3, F_MF3Val);
    }
 #elif    CONFIG_AM_CHIP_ID==1211
	 REG_WRITE(GPIO_MFCTL3, F_MF3Val);
	 REG_WRITE(GPIO_MFCTL4, F_MF4Val);
	 REG_WRITE(GPIO_MFCTL5, F_MF5Val);
#elif CONFIG_AM_CHIP_ID == 1220 
	
 	REG_WRITE(GPIO_MFCTL5, F_MF5Val);
#elif CONFIG_AM_CHIP_ID == 1213
	REG_WRITE(GPIO_MFCTL2, F_MF2Val);
	REG_WRITE(GPIO_MFCTL3, F_MF3Val);

#else
        #error " no chip id defined"                  
#endif

 }


#endif


