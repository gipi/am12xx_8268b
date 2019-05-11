/**
 * \file  nand_scan.c
 * \brief  Scan the zone tables in flash
 *
 * This module search the zone tables in flash firstly. If cannot be found or\n
 * checksum error, rebuild the zone tables.
 *
 *  Copyright (c) 2007 Actions Semiconductor Co., Ltd.
 *
 * \author David
 *
 * \date 2007.07
 *
 * \version 0.0.1
 */

#include "nand_flash_driver_type.h"
//#include "adfu.h"
//#include "Flash_driver_config.h"
#include "syscfg.h"

#define LittleToBig16(num)   (INT16U)((num>>8)&0xFF) |((num&0xFF)<<8)
#define LittleToBig32(num)   (INT32U)((num&0xFF000000)>>24) |\
                                                ((num&0xFF0000)>>8)|((num&0xFF00)<<8)|((num&0xFF)<<24)

 extern INT8U *GPageBuf ;
static inline INT32S Check_all_ff(const void *buf, INT32S size)
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

#if  ODFACT_EraseALL_Block==0x01 && 0x01 ==ODFACT_ENABLE	
typedef struct __Erase_Blk_Inf2
{
          INT8U   *Write_Buff;
          INT8U   *Read_Buff;
          INT8U   * BadTable;
          INT32U   bDataBlkCnt;
          
}Erase_Blk_Inf2;
Erase_Blk_Inf2 *Blk_Info2;
INT32U  Verify_Check_0xFF(struct PhysicOpParameter *tmpNandOpParTmp)    //Write_Read_Data_TEST()
{	
	INT16U  iLoop;
	INT32S   read_pages[4];
	INT32S   BBFP_Bitmap,ret;
	INT32S   Page_Size;
	struct NandSpareData SData;
	struct PhysicOpParameter tmpNandOpPar;	
	INT32U Ret1,Ret2,Ret3;	
        INT32U Result;
        INT8U *Write_Data_Buff,*GPageBuf;
	
        /* init read pages */
        read_pages[0] = 0;	/* page 0 always be read for getting the logical block info */
        read_pages[1] = 1;
        read_pages[2] = 1;
        read_pages[3] = 1;

        /* init read pages */
        read_pages[1] = 1;
        read_pages[2] = PAGE_CNT_PER_PHY_BLK- 1;
        read_pages[3] = PAGE_CNT_PER_PHY_BLK- 2;
        Page_Size =NandDevInfo.NandFlashInfo->SectorNumPerPage * NAND_SECTOR_SIZE;

        Write_Data_Buff =Blk_Info2->Write_Buff;
        GPageBuf = Blk_Info2->Read_Buff;

        tmpNandOpPar.BankNum = tmpNandOpParTmp->BankNum;
        tmpNandOpPar.PageNum = tmpNandOpParTmp->PageNum;
        tmpNandOpPar.PhyBlkNumInBank = tmpNandOpParTmp->PhyBlkNumInBank;		
        tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
        Result =0x00;

        MEMSET(Write_Data_Buff,0xFF,NandStorageInfo.SectorNumPerPage*512);
     
 	for(iLoop=0x00;iLoop<4;iLoop++)
	{		
		if (read_pages[iLoop] == -1)
			continue;
			
       
                //tmpNandOpPar.BankNum = ChipNo;
                tmpNandOpPar.PageNum = read_pages[iLoop];
                //tmpNandOpPar.PhyBlkNumInBank = Block_Num+TotalBlkNumPerDie*Die_No;
                tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
                tmpNandOpPar.MainDataPtr =GPageBuf;
                tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;
                Ret3 = PHY_PageRead(&tmpNandOpPar);	
                if(Ret3 !=TRUE)
                {
                        INIT_BOOT("Ret1:%x,Ret2:%x,Ret3:%x*****\n",Ret3);
                        Result =0x02;
                        goto Check_Result;
                }

                if(0x00 != MEMCMP(Write_Data_Buff,GPageBuf,NandStorageInfo.SectorNumPerPage*512))
                {
                        Result =0x01;
                        Str_printf("GPageBuf\n",GPageBuf,16);
                        //Str_printf("Write_Data_Buff\n",Write_Data_Buff,16);
                        goto Check_Result;
                }             
		
	}
Check_Result:

	return Result;

	
}
INT32U  Verify_Check_55AA(INT16U Test_Data, struct PhysicOpParameter *tmpNandOpParTmp,INT8U CheckMode)    //Write_Read_Data_TEST()
{	
	INT16U  iLoop;
	INT32S   read_pages[8];
	INT32S   BBFP_Bitmap,ret;
	INT32S   Page_Size;
	struct NandSpareData SData;
	struct PhysicOpParameter tmpNandOpPar;	
	INT32U Ret1,Ret2,Ret3;	
        INT32U Result;
        INT8U *Write_Data_Buff,*GPageBuf;
	
	/* init read pages */
	read_pages[0] = 0;	/* page 0 always be read for getting the logical block info */	
	/* init read pages */
	if(CheckMode==0x00)
         {
            	for(iLoop =0x00;iLoop<8;iLoop++)
            	{
            	        read_pages[iLoop] = iLoop; 
            	}
	}
	else if(CheckMode==0x01)
	{
	         for(iLoop =0x00;iLoop<8;iLoop++)
            	{
            	        read_pages[7-iLoop] = PAGE_CNT_PER_PHY_BLK-iLoop-1; 
            	}
	}
	else
	{
	         read_pages[0] = 0x00;
	         read_pages[1] = 0x01;
	         read_pages[2] = 0x02;
	         read_pages[3] = 0x03;
	         read_pages[4] = PAGE_CNT_PER_PHY_BLK-0x04;
	         read_pages[5] = PAGE_CNT_PER_PHY_BLK-0x03;
	         read_pages[6] = PAGE_CNT_PER_PHY_BLK-0x02;
	         read_pages[7] = PAGE_CNT_PER_PHY_BLK-0x01;
	}
	/*for(iLoop =0x00;iLoop<8;iLoop++)
	{
	     printf("%x,",read_pages[iLoop]);
	     if(iLoop==7)
	           printf("\n");
         }*/
	     
	
	Page_Size =NandDevInfo.NandFlashInfo->SectorNumPerPage * NAND_SECTOR_SIZE;
	
         Write_Data_Buff =Blk_Info2->Write_Buff;
         GPageBuf = Blk_Info2->Read_Buff;
         
        tmpNandOpPar.BankNum = tmpNandOpParTmp->BankNum;
        tmpNandOpPar.PageNum = tmpNandOpParTmp->PageNum;
        tmpNandOpPar.PhyBlkNumInBank = tmpNandOpParTmp->PhyBlkNumInBank;		
        tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
       Result =0x00;
       if(CheckMode ==0x3)
       {
               for(iLoop=0x00;iLoop<NandStorageInfo.SectorNumPerPage*256;iLoop++)
               {
                         *(INT16U*)(Write_Data_Buff+iLoop*2)=(Test_Data+iLoop);
               }
       }
       else
       {
                for(iLoop=0x00;iLoop<NandStorageInfo.SectorNumPerPage*256;iLoop++)
               {
                         *(INT16U*)(Write_Data_Buff+iLoop*2)=(Test_Data);
               }
        }
        
    	for(iLoop=0x00;iLoop<8;iLoop++)
	{		
		if (read_pages[iLoop] == -1)
			continue;

		//Write Page 
		//tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = read_pages[iLoop];
		//tmpNandOpPar.PhyBlkNumInBank = Block_Num+TotalBlkNumPerDie*Die_No;		
		tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
		tmpNandOpPar.MainDataPtr =Write_Data_Buff;
		tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;	
		Ret1 = PHY_PageWrite(&tmpNandOpPar);
		Ret2 = PHY_SyncNandOperation(tmpNandOpPar.BankNum);
                  if((Ret1 !=TRUE)||(Ret2 !=TRUE))
		{
			INIT_BOOT("Ret1:%x,Ret2:%x,Ret3:%x*****\n",Ret1,Ret2,Ret3);
			Result =0x02;
			goto Check_Result;
		}
		
                 //*****Read Page Data*******/
		//tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = read_pages[iLoop];
		//tmpNandOpPar.PhyBlkNumInBank = Block_Num+TotalBlkNumPerDie*Die_No;
		tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
		tmpNandOpPar.MainDataPtr =GPageBuf;
		tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;
		Ret3 = PHY_PageRead(&tmpNandOpPar);	
		if(Ret3 !=TRUE)
		{
			INIT_BOOT("Ret1:%x,Ret2:%x,Ret3:%x*****\n",Ret1,Ret2,Ret3);
			Result =0x02;
			goto Check_Result;
		}
	
		if(0x00 != MEMCMP(Write_Data_Buff,GPageBuf,NandStorageInfo.SectorNumPerPage*512))
		{
			Result =0x01;
			printf("%x,%x ,",read_pages[iLoop],iLoop);
			Str_printf("GPageBuf\n",GPageBuf,16);
			Str_printf("Write_Data_Buff\n",Write_Data_Buff,16);
			goto Check_Result;
		}
		
	}
Check_Result:

	return Result;	
}
void Dump_Message(INT32U Block,INT32U line)
{
     INIT_BOOT("\n%d %d\n",Block,line);
}
INT32U NFVerify_DieBlock(INT32U ChipNo,INT32U DieToal,INT32U DieNo,INT32U TotalBlk)
{
	struct PhysicOpParameter tmpNandOpPar;	
	struct NandSpareData SData;
	INT32U iLoop,TotalBlkNumPerDie,Die_No;
	INT32U bRet,bRet2,DieCntPerChip;
	INT8U  bBadBlkFlag;	
	
	TotalBlkNumPerDie= TotalBlk;
	Die_No=DieNo;
	DieCntPerChip=DieToal;
         
        INIT_BOOT("\n 1. Erase All Block .....................\n");
        for(iLoop= 0x00;iLoop<TotalBlkNumPerDie;iLoop++)
	{
		tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = 0x00;
		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
		tmpNandOpPar.SectorBitmapInPage = 0x01;	
		bBadBlkFlag=0x00;
                  if(0x00==((tmpNandOpPar.PhyBlkNumInBank+1) %16))
		{
			INIT_BOOT("%4x,",tmpNandOpPar.PhyBlkNumInBank);
			if(0x00==((tmpNandOpPar.PhyBlkNumInBank+1) %128))
			{
				INIT_BOOT("\n");
			}
		}	
		/* Erase Block  and check status */
		bRet=PHY_EraseNandBlk(&tmpNandOpPar);		
		bRet2=PHY_SyncNandOperation(tmpNandOpPar.BankNum);
		if((bRet!=TRUE) ||(bRet2!=TRUE))
		{
			Blk_Info2->bDataBlkCnt++;
			bBadBlkFlag=1;
			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);
			goto Check_Error1;
		}		
                 /* eheck Block data 0xFF  */
		bRet=Verify_Check_0xFF(&tmpNandOpPar);
		if(bRet)
		{
			Blk_Info2->bDataBlkCnt++;
			bBadBlkFlag=0x01;
			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);	
			goto Check_Error1;					
		}		
#ifdef CHECK_Data_55AA
                 bRet=Verify_Check_55AA(0xAA55,&tmpNandOpPar,0x00);
        		if(bRet)
        		{
        			Blk_Info2->bDataBlkCnt++;
        			bBadBlkFlag =0x01;
        			 Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);
                            goto Check_Error1;
        		}      
        	
        		tmpNandOpPar.BankNum = ChipNo;
        		tmpNandOpPar.PageNum = 0x00;
        		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
        		tmpNandOpPar.SectorBitmapInPage = 0x01;	
        		
                   bRet=Verify_Check_55AA(0x55AA,&tmpNandOpPar,0x01);             
        		if(bRet)		
        		{
        			Blk_Info2->bDataBlkCnt++;
        			bBadBlkFlag =0x01;
        			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);
        			goto Check_Error1;
        		}
        		
        		/* Erase Block  and check status */
    		bRet=PHY_EraseNandBlk(&tmpNandOpPar);		
    		bRet2=PHY_SyncNandOperation(tmpNandOpPar.BankNum);
    		if((bRet!=TRUE) ||(bRet2!=TRUE))
    		{			
    			bBadBlkFlag=1;
    			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);
    			Blk_Info2->bDataBlkCnt++;
    			goto Check_Error1;
    		}
    		
    		bRet=Verify_Check_0xFF(&tmpNandOpPar);
    		if(bRet)
		{
			Blk_Info2->bDataBlkCnt++;
			bBadBlkFlag=0x01;
			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);
			goto Check_Error1;					
		}
#endif
              
 #ifdef CHECK_Data_1234
                  bRet=Verify_Check_55AA(0x1234,&tmpNandOpPar,0x03);             
        		if(bRet)		
        		{
        			Blk_Info2->bDataBlkCnt++;
        			bBadBlkFlag =0x01;
        			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);
        			goto Check_Error1;
        		}
        		
        		/* Erase Block  and check status */
    		bRet=PHY_EraseNandBlk(&tmpNandOpPar);		
    		bRet2=PHY_SyncNandOperation(tmpNandOpPar.BankNum);
    		if((bRet!=TRUE) ||(bRet2!=TRUE))
    		{			
    			bBadBlkFlag=1;
    			Blk_Info2->bDataBlkCnt++;
    			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);    			
    			goto Check_Error1;
    		}
    	
    		bRet=Verify_Check_0xFF(&tmpNandOpPar);
    		if(bRet)
		{
			Blk_Info2->bDataBlkCnt++;
			bBadBlkFlag=0x01;
			Dump_Message(Blk_Info2->bDataBlkCnt,__LINE__);	
			goto Check_Error1;					
		}
    		
 #endif
 Check_Error1:
               if(bBadBlkFlag ==0x01)
               {
               

                SData.UserData[0].SpareData0.BadFlag = 0x0;

                tmpNandOpPar.BankNum = ChipNo;
                tmpNandOpPar.PageNum = 0x00;
                tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
                tmpNandOpPar.SectorBitmapInPage = 0x01;	
                tmpNandOpPar.MainDataPtr =(INT8U*)GPageBuf;
                tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;

                SData.UserData[0].SpareData0.LogicBlkInfo =0x00;
                SData.UserData[0].SpareData0.BadFlag =0x00;     


                bRet = PHY_PageWrite(&tmpNandOpPar);
                bRet = PHY_SyncNandOperation(tmpNandOpPar.BankNum);

                SData.UserData[0].SpareData0.LogicBlkInfo =0x11;
                SData.UserData[0].SpareData0.BadFlag =0x11;
	
                bRet = PHY_PageRead(&tmpNandOpPar);

                //	INIT_BOOT("Before BadFlag:%x\n",SData.UserData[0].SpareData0.BadFlag);
                //	INIT_BOOT("LogicBlkInfo:%x\n",SData.UserData[0].SpareData0.LogicBlkInfo);

                *(INT16U*)(Blk_Info2->BadTable+(Blk_Info2->bDataBlkCnt-1)*2)=(INT16U)iLoop;		  
                //INIT_BOOT("***1ChipNo:%d,Die_No:%d,Blk:%d *****\n",ChipNo,Die_No,iLoop);
        			
               }
        
         }
        
        INIT_BOOT("\n[Index] [Blk] [Index] [Blk] [Index] [Blk] [Index] [Blk]\n\n");
        for(iLoop=0x00;iLoop<Blk_Info2->bDataBlkCnt;iLoop++)
        {
                INIT_BOOT("%6d,%6d ",iLoop+1,*(INT16U*)(Blk_Info2->BadTable+iLoop*2));
                if((0x00==(iLoop+1)%4) || iLoop==(Blk_Info2->bDataBlkCnt-1))
                {
                        INIT_BOOT("\n");
                }
        }
    return  0x00;	
		
}
INT32S NF_Eraseblk_Verify(void)
{
        struct PhysicOpParameter tmpNandOpPar;	 		
        INT32U  Total_PhyBlk;
        INT32U  Total_ChipCnt;
        INT32U  iLoop,ChipNo,Die_No,result,kLoop,jLoop,Res1,Res2;
        INT16U bPage_Size,TotalBlkNumPerDie,DieCntPerChip,TotalBlkNumPerBlk;	
        INT32U  bRet;       

        INIT_BOOT("**********start Read CE0~CE1 Flash ID**********\n");

        /* default bank count is 1 */
   //     TOTAL_BANKS = 1;                
   //     DIENUM_IN_FLASH = (NandDevInfo.NandFlashInfo->ChipCnt)  * (NandDevInfo.NandFlashInfo->DieCntPerChip);

   //     ZONENUM_PER_DIE = (NandDevInfo.LogicOrganizePar->ZoneNumPerDie);
    //    SUBLK_PER_DIE = (ZONENUM_PER_DIE * BLK_NUM_PER_ZONE);

    //    PAGES_PER_SUBLK = NandDevInfo.LogicOrganizePar->PageNumPerLogicBlk;
   //     MULTI_PLANE_SUPPORT =0x00;	
        NandStorageInfo.OperationOpt = 0x00;
        NandStorageInfo.PlaneCntPerDie=0x01;

        bPage_Size = NandDevInfo.NandFlashInfo->SectorNumPerPage* NAND_SECTOR_SIZE;
        Total_ChipCnt = NandDevInfo.NandFlashInfo->ChipCnt;
        TotalBlkNumPerDie =NandDevInfo.NandFlashInfo->TotalBlkNumPerDie;
        DieCntPerChip= NandDevInfo.NandFlashInfo->DieCntPerChip;
        TotalBlkNumPerBlk  = TotalBlkNumPerDie* DieCntPerChip*Total_ChipCnt;
        
         Blk_Info2=(Erase_Blk_Inf2*)MALLOC(sizeof(Erase_Blk_Inf2));
         MEMSET(Blk_Info2,0x00,sizeof(Erase_Blk_Inf2));           
	Blk_Info2->Write_Buff =(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage* NAND_SECTOR_SIZE);
         Blk_Info2->Read_Buff =(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage* NAND_SECTOR_SIZE);	
	Blk_Info2->BadTable = (INT8U*)MALLOC(512*2);	
	
        INIT_BOOT("\n**********Start of Erase Block**********\n");
    //    INIT_BOOT("TOTAL_BANKS:%x\n", TOTAL_BANKS);
      //  INIT_BOOT("DIENUM_IN_FLASH:%x\n", DIENUM_IN_FLASH);
       // INIT_BOOT("ZONENUM_PER_DIE:%x\n", ZONENUM_PER_DIE);
      //  INIT_BOOT("SUBLK_PER_DIE:%x\n", SUBLK_PER_DIE);
       // INIT_BOOT("PAGES_PER_SUBLK: %x\n", PAGES_PER_SUBLK);
      //  INIT_BOOT("MULTI_PLANE_SUPPORT: %x\n", MULTI_PLANE_SUPPORT);
      //  INIT_BOOT("DIE_FIRST_SUBLK_NUM:%x\n", DIE_FIRST_SUBLK_NUM);  
        INIT_BOOT("FULL_BITMAP_OF_SINGLE_PAGE:%x\n",FULL_BITMAP_OF_SINGLE_PAGE);
        INIT_BOOT("FULL_BITMAP_OF_SUPER_PAGE:%x\n",FULL_BITMAP_OF_SUPER_PAGE);
        INIT_BOOT("FULL_BITMAP_OF_BUFFER_PAGE:%x\n",FULL_BITMAP_OF_BUFFER_PAGE);
        INIT_BOOT("Total_ChipCnt:0x%x,%d\n",Total_ChipCnt,Total_ChipCnt);	
        INIT_BOOT("DieCntPerChip:0x%x,%d\n",DieCntPerChip,DieCntPerChip);	
        INIT_BOOT("Page_Size is:0x%x,%d\n",bPage_Size,bPage_Size);
         INIT_BOOT("TotalBlkNumPerDie:0x%x,%d\n",TotalBlkNumPerDie,TotalBlkNumPerDie);            
        INIT_BOOT("TotalBlkNumPerBlk:0x%x,%d\n",TotalBlkNumPerBlk,TotalBlkNumPerBlk);  
        INIT_BOOT("Blk_Info2->Write_Buff is:0x%x,\n",Blk_Info2->Write_Buff);
        INIT_BOOT("Blk_Info2->Read_Buff is:0x%x\n",Blk_Info2->Read_Buff);
        INIT_BOOT("Blk_Info2->BadTable is:0x%x\n",Blk_Info2->BadTable);
        INIT_BOOT("Blk_Info2 is:0x%x,%x\n",Blk_Info2,sizeof(Erase_Blk_Inf2));
 
	
        Blk_Info2->bDataBlkCnt=0x00;
	for(ChipNo =0x00;ChipNo<Total_ChipCnt;ChipNo++)
	{			
		for( Die_No=0x00;Die_No<DieCntPerChip;Die_No++)
		{	
			INIT_BOOT("***ChipNo is:%d***\n",ChipNo);
			INIT_BOOT("***Die_No is:%d***\n",Die_No);			
			NFVerify_DieBlock(ChipNo,DieCntPerChip,Die_No,TotalBlkNumPerDie);		   
		}		
	}
	
	INIT_BOOT("****Bad Block Table:%d****:\n",Blk_Info2->bDataBlkCnt);
	INIT_BOOT("[Index] [Blk] [Index] [Blk] [Index] [Blk] [Index] [Blk]\n");
	for(iLoop=0x00;iLoop<Blk_Info2->bDataBlkCnt;iLoop++)
	{
		INIT_BOOT("%6d,%6d ",iLoop+1,*(INT16U*)(Blk_Info2->BadTable+iLoop*2));
		if((0x00==(iLoop+1)%4) || iLoop==(Blk_Info2->bDataBlkCnt-1))
		{
			INIT_BOOT("\n");
		}
	}
	INIT_BOOT("\n\n*********Finish end of %d Block *********\n",ChipNo);
	return 0;

	
}
#endif

#if  (SUPPORT_Tool_Erase ==0x01)  && (0x01 ==TOOL_ERASE_ENABLE )

typedef struct _Erase_Blk_Info
{
        INT32U Flash_ID;
        INT16U Total_Blk;
        INT8U   Cmd_Max;
        INT8U   Cur_CMD_Cnt;
        INT8U   Cur_FunNo;
        INT16U  CMD_Per_Blk;
        INT32U  BadBlk_Cnt;
        INT8U   * Write_Buff;
        INT8U   *Write_Buff55;
        INT8U   *Write_BuffAA;
        INT8U  *Write_Buff12;
        INT8U   *Read_Buff;
}Erase_Blk_Info;
//static INT8U   * Write_Data_Buff=NULL;
Erase_Blk_Info   *Blk_Info;

//Little Endian

INT32S MEMCMP_32Bit( INT32S* srcdata_addr, INT32S* dstdata_addr,INT32U Size )
{
    INT32S i, *p, *q;
    
    p=srcdata_addr;
    q=dstdata_addr;
    for( i=0; i<(INT32S)(Size/4); i++)
    {
        if( (*p) != (*q) )
            return 0;
        p++;
        q++;
    }
    return 1;

}

INT32U Erase_ALLBlk_Init(void )
{
       struct ScanDieInfo *sdi;
	INT32U DieNo;
	INT32U ret = TRUE;

//	DMSG(DBL_LOG, "[INIT] INIT_CreateBlkTbls() enter.\n");

//	DASSERT(NandDevInfo.ZoneInfo);
//	DASSERT(NandDevInfo.NandFlashInfo);
//	DASSERT(NandDevInfo.LogicOrganizePar);	
 /* init heap for nand flash driver */
	//init_heap_for_nand();

	//ret = INIT_ScanNandStorage();
	//if (ret != TRUE)
	//{
		//DMSG(DBL_ERR, "[INIT] NandProbe err, ret %u\n", ret);		
	//}
	//CaclConsts();
	//sdi = &DieInfo;
	/* allocate global page buffer */
    GPageBuf = (INT8U*)MALLOC(NandDevInfo.LogicOrganizePar->SectorNumPerLogicPage * NAND_SECTOR_SIZE);
	if (!GPageBuf)
	{
        printf("%s:malloc GPageBuf fail!\n",__func__);
       // ret = SCAN_ERR_NO_MEM;		
	}
    return 0x00;	
}
INT32U  Erase_Check_0xFF(struct PhysicOpParameter *tmpNandOpParTmp)    //Write_Read_Data_TEST()
{	
	INT16U  iLoop;
	INT32S   read_pages[4];
	INT32S   BBFP_Bitmap,ret;
	INT32S   Page_Size;
	struct NandSpareData SData;
	struct PhysicOpParameter tmpNandOpPar;	
	INT32U Ret1,Ret2,Ret3;	
        INT32U Result;
        INT8U *Write_Data_Buff,*GPageBuf;
	
        /* init read pages */
        read_pages[0] = 0;	/* page 0 always be read for getting the logical block info */
        read_pages[1] =-1;
        read_pages[2] = -1;
        read_pages[3] = -1;

        /* init read pages */
     //   read_pages[1] = 1;
     //   read_pages[2] = PAGE_CNT_PER_PHY_BLK- 2;
        read_pages[3] = PAGE_CNT_PER_PHY_BLK- 1;
        Page_Size =NandDevInfo.NandFlashInfo->SectorNumPerPage * NAND_SECTOR_SIZE;

        Write_Data_Buff =Blk_Info->Write_Buff;
        GPageBuf = Blk_Info->Read_Buff;

        tmpNandOpPar.BankNum = tmpNandOpParTmp->BankNum;
        tmpNandOpPar.PageNum = tmpNandOpParTmp->PageNum;
        tmpNandOpPar.PhyBlkNumInBank = tmpNandOpParTmp->PhyBlkNumInBank;		
        tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
        Result =0x00;
        
   //     MEMSET(Write_Data_Buff,0xFF,NandStorageInfo.SectorNumPerPage*512);
 	for(iLoop=0x00;iLoop<4;iLoop++)
	{		
		if (read_pages[iLoop] == -1)
			continue;
			
                //printf("run to :%s,%d\n",__FUNCTION__,__LINE__);
                //tmpNandOpPar.BankNum = ChipNo;
                tmpNandOpPar.PageNum = read_pages[iLoop];
                //tmpNandOpPar.PhyBlkNumInBank = Block_Num+TotalBlkNumPerDie*Die_No;
                tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
                tmpNandOpPar.MainDataPtr =GPageBuf;
                tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;
                Ret3 = PHY_PageRead(&tmpNandOpPar);	
                if(Ret3 !=TRUE)
                {
                        INIT_BOOT("Ret1:%x,Ret2:%x,Ret3:%x*****\n",Ret3);
                        Result =0x02;
                        goto Check_Result;
                }

             //   if(0x00 != MEMCMP(Write_Data_Buff,GPageBuf,NandStorageInfo.SectorNumPerPage*512))
                if(0x00==Check_all_ff(GPageBuf, NandStorageInfo.SectorNumPerPage*512 ))
                {
                        Result =0x01;
                        Str_printf("GPageBuf\n",GPageBuf,16);
                        //Str_printf("Write_Data_Buff\n",Write_Data_Buff,16);
                        goto Check_Result;
                }
		
	}
Check_Result:

	return Result;

	
}
INT32U  Erase_Check_55AA(INT16U Test_Data, struct PhysicOpParameter *tmpNandOpParTmp,INT8U CheckMode)    //Write_Read_Data_TEST()
{	
	INT16U  iLoop;
	INT32S   read_pages[8];
	INT32S   BBFP_Bitmap,ret;
	INT32S   Page_Size;
	struct NandSpareData SData;
	struct PhysicOpParameter tmpNandOpPar;	
	INT32U Ret1,Ret2,Ret3;	
        INT32U Result;
        INT8U *Write_Data_Buff,*GPageBuf;
        INT8U CheckCnt;
	Ret3 =0x00;
	/* init read pages */
	read_pages[0] = 0;	/* page 0 always be read for getting the logical block info */	
	CheckCnt =8;
	for(iLoop =0x00;iLoop<8;iLoop++)
         {
            	        read_pages[iLoop] = -1; 
          }
	/* init read pages */
	if(CheckMode==0x00)
         {
                  CheckCnt=1;
            	for(iLoop =0x00;iLoop<CheckCnt;iLoop++)
            	{
            	        read_pages[iLoop] = iLoop; 
            	}
	}
	else if(CheckMode==0x01)
	{
	         CheckCnt=1;
	         for(iLoop =0x00;iLoop<CheckCnt;iLoop++)
            	{
            	        read_pages[CheckCnt-1-iLoop] = PAGE_CNT_PER_PHY_BLK-iLoop-1; 
            	}
	}
	else
	{
	         read_pages[0] = 0x00;
	         read_pages[1] = 0x01;
	         read_pages[2] = 0x02;
	         read_pages[3] = 0x03;
	         read_pages[4] = PAGE_CNT_PER_PHY_BLK-0x04;
	         read_pages[5] = PAGE_CNT_PER_PHY_BLK-0x03;
	         read_pages[6] = PAGE_CNT_PER_PHY_BLK-0x02;
	         read_pages[7] = PAGE_CNT_PER_PHY_BLK-0x01;
	         CheckCnt=2;
	         read_pages[0] = 0x00;
	         read_pages[1] = PAGE_CNT_PER_PHY_BLK-0x01;
	}
	
	Page_Size =NandDevInfo.NandFlashInfo->SectorNumPerPage * NAND_SECTOR_SIZE;
	
         Write_Data_Buff =Blk_Info->Write_Buff;
         GPageBuf = Blk_Info->Read_Buff;
         
        tmpNandOpPar.BankNum = tmpNandOpParTmp->BankNum;
        tmpNandOpPar.PageNum = tmpNandOpParTmp->PageNum;
        tmpNandOpPar.PhyBlkNumInBank = tmpNandOpParTmp->PhyBlkNumInBank;		
        tmpNandOpPar.SectorBitmapInPage = 0xF;	
       Result =0x00;
       switch(CheckMode)
       {
               case 0x00: //0xAA
                            Write_Data_Buff =Blk_Info->Write_BuffAA;
                            break;
                case 0x01: //0x55;
                            Write_Data_Buff =Blk_Info->Write_Buff55;
                            break;
                case 0x03: //random
                            Write_Data_Buff =Blk_Info->Write_Buff12;
                            break;
                 default:
                            break;               
               
       }
     /*  if(CheckMode ==0x3)
       {
               for(iLoop=0x00;iLoop<NandStorageInfo.SectorNumPerPage*256;iLoop++)
               {
                         *(INT16U*)(Write_Data_Buff+iLoop*2)=Test_Data+iLoop;
               }
       }
       else
       {
            MEMSET(Write_Data_Buff,(INT8U)Test_Data,NandStorageInfo.SectorNumPerPage*512);
        }*/
        
    	for(iLoop=0x00;iLoop<CheckCnt;iLoop++)
	{		
		if (read_pages[iLoop] == -1)
			continue;

		//Write Page 
		//tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = read_pages[iLoop];
		//tmpNandOpPar.PhyBlkNumInBank = Block_Num+TotalBlkNumPerDie*Die_No;		
		tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
		tmpNandOpPar.MainDataPtr =Write_Data_Buff;
		tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;	
		Ret1 = PHY_PageWrite(&tmpNandOpPar);
		Ret2 = PHY_SyncNandOperation(tmpNandOpPar.BankNum);
                  if((Ret1 !=TRUE)||(Ret2 !=TRUE))
		{
			INIT_BOOT("Ret1:%x,Ret2:%x,Ret3:%x*****\n",Ret1,Ret2,Ret3);
			Result =0x02;
			goto Check_Result;
		}
		
                 //*****Read Page Data*******/
		//tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = read_pages[iLoop];
		//tmpNandOpPar.PhyBlkNumInBank = Block_Num+TotalBlkNumPerDie*Die_No;
		tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
		tmpNandOpPar.MainDataPtr =GPageBuf;
		tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;
		Ret3 = PHY_PageRead(&tmpNandOpPar);	
		if(Ret3 !=TRUE)
		{
			INIT_BOOT("Ret1:%x,Ret2:%x,Ret3:%x*****\n",Ret1,Ret2,Ret3);
			Result =0x02;
			goto Check_Result;
		}
	
		//if(0x00 != MEMCMP(Write_Data_Buff,GPageBuf,NandStorageInfo.SectorNumPerPage*512))
		if(0x00 == MEMCMP_32Bit((INT32S*)Write_Data_Buff,(INT32S*)GPageBuf,NandStorageInfo.SectorNumPerPage*512))		
		{
			Result =0x01;
			Str_printf("GPageBuf\n",GPageBuf,16);
			Str_printf("Write_Data_Buff\n",Write_Data_Buff,16);
			goto Check_Result;
		}
		
	}
Check_Result:

	return Result;

	
}

INT32U Erase_CMD_0xF0_F6(INT8U Fun_No,INT8U *buf,INT32U CMD_No)
{
        INT8U *Tmp_buf;
        struct PhysicOpParameter tmpNandOpPar;	
        struct NandSpareData SData;
        INT32U iLoop,TotalBlkNumPerDie,Die_No;
        INT32U bRet,bRet2,DieCntPerChip;
        INT8U  bBadBlkFlag;	
        INT8U ChipNo,ChipCnt;
        INT16U StartBlock,bPage_Size;
        INT32U Block_Tmp,Block_End,BadBlk_Cnt,BadBlk_Test;
        INT32U TotalBlkNumChip;

        Tmp_buf =buf;   
        TotalBlkNumPerDie =NandDevInfo.NandFlashInfo->TotalBlkNumPerDie;
        DieCntPerChip= NandDevInfo.NandFlashInfo->DieCntPerChip;  
        ChipCnt = NandDevInfo.NandFlashInfo->ChipCnt;
        TotalBlkNumPerDie =NandDevInfo.NandFlashInfo->TotalBlkNumPerDie;
        DieCntPerChip= NandDevInfo.NandFlashInfo->DieCntPerChip;	
        bPage_Size = NandDevInfo.NandFlashInfo->SectorNumPerPage* NAND_SECTOR_SIZE;	
        TotalBlkNumChip = TotalBlkNumPerDie* DieCntPerChip;
        
        StartBlock = CMD_No*Blk_Info->CMD_Per_Blk;     
        //get chip no
        ChipNo =StartBlock/TotalBlkNumChip;
        Block_Tmp = StartBlock%TotalBlkNumChip;
        
        //get Die No   and Start of Die block     
        Die_No = Block_Tmp/TotalBlkNumPerDie;        
        Block_Tmp = Block_Tmp%TotalBlkNumPerDie;
        
        Block_End = Block_Tmp+Blk_Info->CMD_Per_Blk;   
        
        BadBlk_Cnt = 0x00;
        BadBlk_Test=0x00;
        MEMSET(Tmp_buf,0xFF,512*4); 
        printf("***%s_0x%x****,%02x,%02x,%04x,%04x,%4x***\n",__FUNCTION__,Fun_No,CMD_No,ChipNo,Die_No,
                            Block_Tmp, Block_End+Die_No*TotalBlkNumPerDie,StartBlock+Die_No*TotalBlkNumPerDie);

          INIT_BOOT("==============Chip :%d Die:%d =============\n\n",ChipNo,Die_No);
	
          for(iLoop=0x00;iLoop<(INT32U)(NandStorageInfo.SectorNumPerPage*128);iLoop++)
          {
                         *(INT32U*)(Blk_Info->Write_Buff55+iLoop*4)=0x55555555;
          } 
          
           for(iLoop=0x00;iLoop<(INT32U)(NandStorageInfo.SectorNumPerPage*128);iLoop++)
          {
                         *(INT32U*)(Blk_Info->Write_BuffAA+iLoop*4)=0xAAAAAAAA;
          } 
          for(iLoop=0x00;iLoop<(INT32U)(NandStorageInfo.SectorNumPerPage*128);iLoop++)
          {
                         *(INT32U*)(Blk_Info->Write_Buff12+iLoop*4)=0x55555555+iLoop*123456;
          } 
          
         //init tmpnand struct      
        for(iLoop= Block_Tmp;iLoop<Block_End;iLoop++)
        {     
        
                  bBadBlkFlag=0x00;
                  
                  tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = 0x00;
		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
		tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
		tmpNandOpPar.MainDataPtr =GPageBuf;
		tmpNandOpPar.SpareDataPtr=&SData;
		
                    if(0x00==((tmpNandOpPar.PhyBlkNumInBank+1) %64))
                    {
                            INIT_BOOT("%04d,",tmpNandOpPar.PhyBlkNumInBank);
                            if(0x00==((tmpNandOpPar.PhyBlkNumInBank+1) %512))
                            {
                                    INIT_BOOT("\n");
                            }
                    }	
		
                 //Check Bad Block  if bad check continue
                if(Fun_No ==0xf1 ||Fun_No ==0xF5||Fun_No ==0xF6)
                {
                        bRet=Erase_Check_0xFF(&tmpNandOpPar);	
                        if(bRet)
                        {
                                BadBlk_Cnt++;
                                bBadBlkFlag =0x01;
                                INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
                                goto Check_Err;
                        }
                        if(Fun_No ==0xf1 )
                              goto Check_Result;
                }
		
		/* Erase Block  and check status */
		bRet=PHY_EraseNandBlk(&tmpNandOpPar);		
		bRet2=PHY_SyncNandOperation(tmpNandOpPar.BankNum);
		if((bRet!=TRUE) ||(bRet2!=TRUE))
		{			
			bBadBlkFlag=1;
			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
			BadBlk_Cnt ++;
			goto Check_Err;
		}		
		tmpNandOpPar.BankNum = ChipNo;
		tmpNandOpPar.PageNum = 0x00;
		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
		tmpNandOpPar.SectorBitmapInPage = FULL_BITMAP_OF_SINGLE_PAGE;	
		tmpNandOpPar.MainDataPtr =GPageBuf;
		tmpNandOpPar.SpareDataPtr=&SData;

		bRet=Erase_Check_0xFF(&tmpNandOpPar);	
		if(bRet)
		{
			BadBlk_Cnt++;
			bBadBlkFlag =0x01;
			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
                          goto Check_Err;
		}
		
		 if(Fun_No ==0xf5 )
                              goto Check_Result;
                  
		//Function 
                  //Support function Write block First 8 Page 0xAA Last 8page data 0xaa 
		if((Fun_No==0xF3) ||(Fun_No==0xF4) ||(Fun_No ==0xF6))
		{		
		         bRet=Erase_Check_55AA(0xAA,&tmpNandOpPar,0x00);
                		if(bRet)
                		{
                			BadBlk_Cnt++;
                			bBadBlkFlag =0x01;
                			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
                                    goto Check_Err;
                		}                		
                		tmpNandOpPar.BankNum = ChipNo;
                		tmpNandOpPar.PageNum = 0x00;
                		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
                		tmpNandOpPar.SectorBitmapInPage = 0x01;	
                		
                           bRet=Erase_Check_55AA(0x55,&tmpNandOpPar,0x01);             
                		if(bRet)		
                		{
                			BadBlk_Cnt++;
                			bBadBlkFlag =0x01;
                			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);	
                			goto Check_Err;
                		}
                		/* Erase Block  and check status */
            		bRet=PHY_EraseNandBlk(&tmpNandOpPar);		
            		bRet2=PHY_SyncNandOperation(tmpNandOpPar.BankNum);
            		if((bRet!=TRUE) ||(bRet2!=TRUE))
            		{			
            			bBadBlkFlag=1;
            			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
            			BadBlk_Cnt ++;
            			goto Check_Err;
            		}		
            		tmpNandOpPar.BankNum = ChipNo;
            		tmpNandOpPar.PageNum = 0x00;
            		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
            		tmpNandOpPar.SectorBitmapInPage = 0x0F;	
            		tmpNandOpPar.MainDataPtr =GPageBuf;
            		tmpNandOpPar.SpareDataPtr=&SData;

            		bRet=Erase_Check_0xFF(&tmpNandOpPar);	
            		if(bRet)
            		{
            			BadBlk_Cnt++;
            			bBadBlkFlag =0x01;
            			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
                                    goto Check_Err;
            		}
		}
		if(0xF4==Fun_No)
		{
		         bRet=Erase_Check_55AA(0x55,&tmpNandOpPar,0x03);             
                		if(bRet)		
                		{
                			BadBlk_Cnt++;
                			bBadBlkFlag =0x01;
                			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
                			goto Check_Err;
                		}
                		/* Erase Block  and check status */
            		bRet=PHY_EraseNandBlk(&tmpNandOpPar);		
            		bRet2=PHY_SyncNandOperation(tmpNandOpPar.BankNum);
            		if((bRet!=TRUE) ||(bRet2!=TRUE))
            		{			
            			bBadBlkFlag=1;
            			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
            			BadBlk_Cnt ++;
            			goto Check_Err;
            		}		
            		tmpNandOpPar.BankNum = ChipNo;
            		tmpNandOpPar.PageNum = 0x00;
            		tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
            		tmpNandOpPar.SectorBitmapInPage = 0x0F;	
            		tmpNandOpPar.MainDataPtr =GPageBuf;
            		tmpNandOpPar.SpareDataPtr=&SData;

            		bRet=Erase_Check_0xFF(&tmpNandOpPar);	
            		if(bRet)
            		{
            			BadBlk_Cnt++;
            			bBadBlkFlag =0x01;
            			INIT_BOOT("\nBad:%d :%d,%d\n",BadBlk_Cnt,__LINE__,tmpNandOpPar.PhyBlkNumInBank);
                                    goto Check_Err;
            		}
		}
		
		
Check_Err:
                if(bBadBlkFlag ==0x01)
                {

 
                        SData.UserData[0].SpareData0.BadFlag = 0x0;

                        tmpNandOpPar.BankNum = ChipNo;
                        tmpNandOpPar.PageNum = 0x00;
                        tmpNandOpPar.PhyBlkNumInBank = iLoop+TotalBlkNumPerDie*Die_No;
                        tmpNandOpPar.SectorBitmapInPage = 0x01;	
                        tmpNandOpPar.MainDataPtr =(INT8U*)GPageBuf;
                        tmpNandOpPar.SpareDataPtr=(INT8U *)&SData;
    
                        SData.UserData[0].SpareData0.LogicBlkInfo =0x00;
                        SData.UserData[0].SpareData0.BadFlag =0x00;   

	
                        bRet = PHY_PageWrite(&tmpNandOpPar);
				
                        bRet = PHY_SyncNandOperation(tmpNandOpPar.BankNum);

                        SData.UserData[0].SpareData0.LogicBlkInfo =0x11;
                        SData.UserData[0].SpareData0.BadFlag =0x11;
		

		 MEMSET((INT8U *)&SData, 0xff, sizeof(struct NandSpareData));
                        bRet = PHY_PageRead(&tmpNandOpPar);        		

                        *(INT16U*)(Tmp_buf+(BadBlk_Cnt-1)*2)=LittleToBig16(tmpNandOpPar.PhyBlkNumInBank);		  
                        INIT_BOOT("***Bad Blk:%d,%d,%d *****\n",ChipNo,Die_No,tmpNandOpPar.PhyBlkNumInBank);

                }
Check_Result:               
	        bBadBlkFlag =0x00;
	}
	
         return 0x00;
}


static INT32U Entry_Flag_Tmp =0x00;
INT32U Erase_Tool_init()
{
   // serial_init();
    INIT_BOOT("\n######%s ,%d 2010-03-08 #####\n",__func__,Entry_Flag_Tmp);
    INIT_BOOT("\n***AL%d %x,%s,%d***** \n\n",AM_CHIP_ID,CHIP_TYPE,FWSC_VER,MAX_CHIP_NUM);
    //nand_flash_inithw();
    Erase_ALLBlk_Init();            
   //// Entry_Flag_Tmp =0x01;
    Blk_Info  =(Erase_Blk_Info *)malloc(sizeof(Erase_Blk_Info));
    if(!Blk_Info)
    {           
            INIT_BOOT("malloc fail ");
    }
    MEMSET(Blk_Info,0x00,sizeof(Blk_Info));
}
INT32U Erase_ALL_BlkFun(INT8U Fun,INT32U CMDCnt,INT8U *buf)
{
        INT32U result;        
        result =0x00;     
        if(0xF0==Fun)
        {
               INIT_BOOT("\n$$$$$$ %s %d ########\n",__func__,Entry_Flag_Tmp);
        }
        if (Entry_Flag_Tmp != 0x01)
        {  
              Entry_Flag_Tmp =0x01;     
        }
        
        switch(Fun)
        {
            case 0xf0:  // Flash ID,block no,die no,
                 {
                        INT32U iLoop;
                        INT32U TotalBlkNumPerDie,TotalBlkNumFlash;
                        INT8U *Tmp_buf;                        
                        Tmp_buf =buf;      
                        printf("run to :%x,%s\n",__LINE__,__func__);
						
                        NandStorageInfo.OperationOpt  &= ~(MULTI_PAGE_WRITE);
	                    NandStorageInfo.PlaneCntPerDie=0x01;                  
                         NandDevInfo.PhyCache =  (INT8U*)MALLOC(SECTOR_NUM_PER_SINGLE_PAGE* NAND_SECTOR_SIZE);
                        TotalBlkNumPerDie = NandDevInfo.NandFlashInfo->TotalBlkNumPerDie;    
                        TotalBlkNumFlash = TotalBlkNumPerDie*NandDevInfo.NandFlashInfo->DieCntPerChip
                                                *NandDevInfo.NandFlashInfo->ChipCnt;
                        Blk_Info->Write_Buff=(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage
                                                                  * NAND_SECTOR_SIZE);                      
                        Blk_Info->Read_Buff=(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage
                                                                  * NAND_SECTOR_SIZE);
                         Blk_Info->Write_Buff55=(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage
                                                                  * NAND_SECTOR_SIZE);
                        Blk_Info->Write_Buff12=(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage
                                                                  * NAND_SECTOR_SIZE);                                      
                         Blk_Info->Write_BuffAA=(INT8U*)MALLOC(NandDevInfo.NandFlashInfo->SectorNumPerPage
                                                                  * NAND_SECTOR_SIZE);  
                                                                  
                     //   INIT_BOOT("TOTAL_BANKS:%u,%x\n", TOTAL_BANKS,Tmp_buf);
                    //    INIT_BOOT("DIENUM_IN_FLASH:%x\n", DIENUM_IN_FLASH);
                     //   INIT_BOOT("ZONENUM_PER_DIE:%u\n", ZONENUM_PER_DIE);
                    //    INIT_BOOT("SUBLK_PER_DIE:%x\n", SUBLK_PER_DIE);
                     //   INIT_BOOT("PAGES_PER_SUBLK: %x\n", PAGES_PER_SUBLK);
                    //    INIT_BOOT("MULTI_PLANE_SUPPORT: %x\n", MULTI_PLANE_SUPPORT);
                       // INIT_BOOT("DIE_FIRST_SUBLK_NUM:%x\n", DIE_FIRST_SUBLK_NUM);  
                        INIT_BOOT("Buffer size:%x\n",NandDevInfo.NandFlashInfo->SectorNumPerPage
                                                                  * NAND_SECTOR_SIZE);
                        INIT_BOOT("TotalBlkNumPerDie:0x%x,%d\n",TotalBlkNumPerDie,TotalBlkNumPerDie);     
                        INIT_BOOT("TotalBlkNumFlash:0x%x,%d\n",TotalBlkNumFlash,TotalBlkNumFlash);   
                        INIT_BOOT("FULL_BITMAP_OF_SINGLE_PAGE:%x\n",FULL_BITMAP_OF_SINGLE_PAGE);
                        INIT_BOOT("FULL_BITMAP_OF_SUPER_PAGE:%x\n",FULL_BITMAP_OF_SUPER_PAGE);
                        INIT_BOOT("FULL_BITMAP_OF_BUFFER_PAGE:%x\n",FULL_BITMAP_OF_BUFFER_PAGE);
			            INIT_BOOT("SpecialCommand->BadBlkFlagPst:%x\n", NandDevInfo.SpecialCommand->BadBlkFlagPst);
                        INIT_BOOT("BuffAddr:%x,%x,%x,%x\n",Blk_Info->Write_Buff,Blk_Info->Write_Buff55,
                                                 Blk_Info->Write_BuffAA,Blk_Info->Write_Buff12);
                        
                        MEMSET(Tmp_buf,0xFF,512);                   
                        //Set Byte0~3
                        for(iLoop = 0x00;iLoop<4;iLoop++)
                        {
                             *(INT8U*)(Tmp_buf+iLoop) = NandDevInfo.NandFlashInfo->NandChipId[iLoop];
                        }                      
                        //Set Byte4:5
                        Blk_Info ->Total_Blk =TotalBlkNumFlash;                        
                        Blk_Info->CMD_Per_Blk = 1024;
                        Blk_Info ->Cmd_Max= TotalBlkNumFlash/Blk_Info->CMD_Per_Blk ;
                  #if 0
                        *(INT16U*)(Tmp_buf+4) =LittleToBig16(TotalBlkNumFlash);                            
                        *(INT8U*)(Tmp_buf+6) = (INT8U)(Blk_Info ->Cmd_Max);                     
                        *(INT16U*)(Tmp_buf+10) =LittleToBig16( Blk_Info->CMD_Per_Blk);         
                  #else
                        *(INT32U*)(Tmp_buf+4) =LittleToBig32(TotalBlkNumFlash);                            
                        *(INT16U*)(Tmp_buf+8) =LittleToBig16( TotalBlkNumFlash/Blk_Info->CMD_Per_Blk);                     
                        *(INT16U*)(Tmp_buf+10) =LittleToBig16( Blk_Info->CMD_Per_Blk);  
			            *(INT16U*)(Tmp_buf+12) =LittleToBig16( NandDevInfo.NandFlashInfo->TotalBlkNumPerDie);  		
                  #endif
                        Str_printf("",Tmp_buf,16);
                        Blk_Info->Cur_CMD_Cnt =0x00;                      
                        break;
                 }
            case 0xf1:  // NO Erase ALL Block                      
            case 0xF2: // Erase bad block,                   
            case 0xF3:                    
            case 0xF4:
            case 0xF5:
            case 0xF6:
			Blk_Info->Cur_FunNo=Fun;
                    result = Erase_CMD_0xF0_F6(Fun,buf,CMDCnt);
                    break;                        
            default:
                    break;
        }
        return result;
}
#endif
