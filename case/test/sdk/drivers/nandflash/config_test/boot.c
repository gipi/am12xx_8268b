
//#define Multi_FLAG "MultiSectorBoot" //size of 16 

#include "boot.h"

 
INT32U bSet_Value_ECC(INT8U *Buffer)
{
    INT8U bVaildMbrcNum,bVaildBrecNum;
    INT16U iLoop;
    struct BootBlk_Info  BlkInfo;
    INT8S   MbrcECCBit,BrecECCBit;
    INT8U   Recalimval,BCHTypeVal;
    INT8S   bModfiyValue,bVaildBootCnt;
    INT8S   bMaxSetECCBit;
    bVaildMbrcNum =0;
    bVaildBrecNum = 0;
    //先确认当前四份Mbrec ,Brec 有效份数
    BrecECCBit=MbrcECCBit =0x00;
    printf("name    chksum   chksum  MaxBit TtalBit BCHType Reclaim \n");
    for(iLoop =0x00 ;iLoop <4 ;iLoop++)
    {
         
         memcpy(&BlkInfo,Buffer+iLoop*sizeof(struct BootBlk_Info),sizeof(struct BootBlk_Info));
         if(BlkInfo.check ==0x00)
             bVaildMbrcNum++;
         if(MbrcECCBit<BlkInfo.ECC_Bit_Max)
         {
             MbrcECCBit = BlkInfo.ECC_Bit_Max;
         }
         Recalimval =BlkInfo.RecalaimVal;
         BCHTypeVal = BlkInfo.BCHType;
         printf("%s%d   0x%2x   0x%2x ",BlkInfo.name,BlkInfo.blknum,BlkInfo.Flashchecksum,BlkInfo.Calchecksum);
         printf(" %2d     %3d     %3d     %4d  \n",BlkInfo.ECC_Bit_Max,BlkInfo.ECC_Bit_Total,BCHTypeVal,Recalimval);
         
    }
    for(iLoop =0x04 ;iLoop <8 ;iLoop++)
    {
         memcpy(&BlkInfo,Buffer+iLoop*sizeof(struct BootBlk_Info),sizeof(struct BootBlk_Info));
         
         if(BlkInfo.check ==0x00)
             bVaildBrecNum++;
         if(BrecECCBit<BlkInfo.ECC_Bit_Max)
         {
             BrecECCBit = BlkInfo.ECC_Bit_Max;
         }  
         Recalimval =BlkInfo.RecalaimVal;
         BCHTypeVal = BlkInfo.BCHType;
         printf("%s%d   0x%2x   0x%2x ",BlkInfo.name,BlkInfo.blknum,BlkInfo.Flashchecksum,BlkInfo.Calchecksum);
         printf(" %2d     %3d     %3d     %4d  \n",BlkInfo.ECC_Bit_Max,BlkInfo.ECC_Bit_Total,BCHTypeVal,Recalimval);
    }
    printf("VaildBrecNum:%d,VaildMbrcNum:%d\n",bVaildBrecNum,bVaildMbrcNum);

    //bVaildBootCnt 由于MPupdate 默认配置文件来取得，最小值1， 最大值4, 默认值暂时定3
    //否则提示相关提示，并且停止升级。
    bVaildBootCnt = 3 ; 
    if(bVaildBootCnt<1 && bVaildBootCnt>4)
    {
         printf("");
         return 0x01;
    }
    /*判断每份boot 最大出错ECC error bit 数目
    bModfiyValue 此变量，根据MPUpdate 配置文件来取得。
    */
    bModfiyValue = 0;  //无符号变量
    bMaxSetECCBit= Recalimval + bModfiyValue;
    //判断配置文件参数非法，则对此值无效，增加相关提示，并且停止升级。
    if(bMaxSetECCBit >=BCHTypeVal)
    {        
         printf("");
         return 0x02;
    }

    ////提示出错，Mbrec,Brec 有效份低于设置有效份数。
    if(bVaildBrecNum <=bVaildBootCnt  || bVaildMbrcNum <=bVaildBootCnt)
    {
         printf("");
         return 0x03;
    }

    //表示Mbrec Brec 最大出错Bit 超过 ，增加相关提示，并且停止升级。
    if( BrecECCBit >=bMaxSetECCBit || MbrcECCBit >bMaxSetECCBit )
    {
         printf("");
         return 0x04;
    }

    //表示成功。
    return 0x00;    
}


/*******************************************************************************
* Function:       // INT32S cmp_brec_check_sum(unsigned int)
* Description:
* Input：         // 输入参数
* Output：        // 其他 校验和,-1 FAIL
* Other：         //
*******************************************************************************/
INT32S static brec_read_check_sum(unsigned short BlockStartNum, unsigned short SectorStartNum, INT8U *BufferAddr,unsigned int MAXSec)
{
	unsigned short n,CheckSum = 0;
	unsigned int m, CurBrecCheckSum;

	/*计算brec的校验和*/
	for (m = SectorStartNum; m < MAXSec; m++)
	{
		//NAND_PhyBoot_Read(N_PHY_READ,BlockStartNum ,m,0, BufferAddr);
		
		
		
		NAND_PhyBoot_Read(BlockStartNum,m,1,BufferAddr);
		if (m==MAXSec-1)  	/*last Brec sector*/
		{
			for (n = 0;n < 255; n++)       
				CheckSum += BufferAddr[2*n] + BufferAddr[2*n+1] * 0x100;
		}
		else
		{
			for (n = 0;n < 256; n++)       
				CheckSum += BufferAddr[2*n] + BufferAddr[2*n+1] * 0x100;
		}
		//Str_printf("", BufferAddr, SECT_SIZE);
	}
	//printf("CheckSum:$%x,%x,%x\n",CheckSum,MAXSec,m);
	/*判断累加起来的brec校验和与原来brec中存放的校验和是否一致*/
	if ((BufferAddr[508] + BufferAddr[509] * 0x100) != 0x55aa)
	{
		return -1;
	}
	if (CheckSum != BufferAddr[510] + BufferAddr[511] * 0x100) 
	{
		return -1;
	}
	CurBrecCheckSum = (unsigned int)CheckSum ;//+ 0x55aa0000
	return  CurBrecCheckSum;
}

//return 0: fail   非0: 正确的份数
int static m_mbrccheck(void)
{
	unsigned int iloop,jloop;
	unsigned char *TmpBuf=(void *)malloc(8*512);
	unsigned int MBRC_SECTOR_NUM,MBRC_Total_Size;
	unsigned int mbrc_tab_checksum=0;
	unsigned int mbrc_checksum=0;
	unsigned int tmpmbrc_checksum=0;
    unsigned int correct_mbrcnum=0;
	unsigned int *t_p=(unsigned int*)TmpBuf;
	int flag=0;
	for (iloop=0;iloop<4;iloop++)
	{
		NAND_PhyBoot_Read(iloop,0,1,TmpBuf);
		if(!strncmp(Multi_FLAG, TmpBuf,16))
        {
            MBRC_SECTOR_NUM = *(volatile unsigned int*)(TmpBuf + 0x28);//must before createTbl()
            MBRC_Total_Size = MBRC_SECTOR_NUM + 1;                     
        }
        else
        {
            MBRC_Total_Size = 1;
            MBRC_SECTOR_NUM = 1;
        }
		for (jloop=0;jloop<512/4-1;jloop++)
		{
			mbrc_tab_checksum+=*t_p++;
		}
        mbrc_tab_checksum+=0x1234;
		if (mbrc_tab_checksum!=*t_p)//说明mbrc是错的
		{
			continue;
		}
		else
		{
			flag=1;
			break;
		}
		/*else
		{
			if (MBRC_Total_Size==0x01)
			{
				return mbrc_tab_checksum;
			}
		}
		return brec_read_check_sum(iloop,1,TmpBuf,MBRC_Total_Size);*/
	}
	 if (flag==1)
	 {
		 //说明找到第一个正确的mbrc,块号是iloop
		 mbrc_checksum=brec_read_check_sum(iloop,1,TmpBuf,MBRC_Total_Size);

	 } 
	 else
	 {
		 //说明没有一份mbrc是对的
		 free(TmpBuf);
		 printf("<Test Info> no mbrc is ok !!\n");
		 return 0;
	 }

	 for (jloop=0;jloop<4;jloop++)
	 {
		 tmpmbrc_checksum=brec_read_check_sum(jloop,1,TmpBuf,MBRC_Total_Size);
		 printf("<Test Info> mbrc data blk: %d,curchecksum: %5d,checksum: %5d,%3dkb\n",jloop,tmpmbrc_checksum,mbrc_checksum,MBRC_Total_Size/2);
         if(tmpmbrc_checksum==mbrc_checksum)
            correct_mbrcnum++;
     }
	 free(TmpBuf);
	 return correct_mbrcnum;
}


//return 0: fail   非0: 正确的份数
int static m_breccheck(void)
{
	unsigned int iloop,jloop;
	unsigned char *TmpBuf=(void *)malloc(8*512);
	unsigned short BlockStartNum,SectorStartNum;
	unsigned int BrecSrcCheckSum, tmpBrecCheckSum;
    unsigned int correct_brecnum=0;
	int flag=0;
	for (iloop=0;iloop<4;iloop++)
	{
		BlockStartNum=4+iloop;
		SectorStartNum=0;
		BrecSrcCheckSum=brec_read_check_sum(BlockStartNum,SectorStartNum,TmpBuf,256);
		if (BrecSrcCheckSum!=-1)
		{
			//说明找到了正确的一份brec
			flag=1;
			break;
		}
	}
	if (flag==0)
	{
		//说明4份brec都是错误的
		printf("<Test Info> no brec is ok!!\n");
		free(TmpBuf);
		return 0;
	}
	for (jloop=4;jloop<8;jloop++)
	{
	    ///可在循环中判断两个checksum是否相同
        SectorStartNum=0;
		tmpBrecCheckSum=brec_read_check_sum(jloop,SectorStartNum,TmpBuf,256);
		printf("<Test Info> brec data blk: %d,curchecksum: %5d,checksum: %5d,%3dkb\n",jloop-4,tmpBrecCheckSum,BrecSrcCheckSum,128);
        if(tmpBrecCheckSum==BrecSrcCheckSum)
            correct_brecnum++;
    }
	
	free(TmpBuf);

    /////利用循环中的比较结果计数，把返回值定为正确brec的分数
	return correct_brecnum;
}


//return 0:ok   1:fail
int m_bootcheck(void)
{
    int i,j;
    i=m_mbrccheck();
    j=m_breccheck();
    
    if(0==i)
    return 1;
    if(0==j)
	return 1;
    printf("<Test Info> correct_mbrcnum: %3d, correct_brecnum: %3d\n",i,j);

    if(i==4&&j==4)
    return 0;//说明bootdata完全正确
    else 
    return 1;
    
}

