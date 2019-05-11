#include "destory_fw.h"

void static hex_printf2(  const INT8U* pData, INT16U inLen)
{
	INT16U iLoop;   
//	printf("%s",pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%02x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  /*%4d*/\n",iLoop);
		}
	}
	printf("\n");

}

int static destory_kernel(int flag1,int flag2)
{
	LFIHead_t * lfi_hd;	//逻辑目录项
	char *TmpBuf[512];
	CapInfo_t m_capinfo;
    Sys_File *m_sysfile;
    unsigned int i=0;
	unsigned int des_offset=0;

	NAND_Log_Read(0,TmpBuf,1);
	lfi_hd = (LFIHead_t *)TmpBuf;
	memcpy(&m_capinfo, &(lfi_hd->CapInfo), sizeof(CapInfo_t));
	
	if (flag1==0)
	{
		i=0;
	} 
	else
	{
		i=m_capinfo.Scodecap;
	}
	NAND_Log_Read(0x01+i,TmpBuf,1);
	//hex_printf2(TmpBuf,512);
	m_sysfile=(Sys_File *)TmpBuf;
	m_sysfile+=flag2;
	des_offset=i+m_sysfile->Sys_offset;
	memset(TmpBuf,0xaa,512);
	NAND_Log_Write(des_offset,TmpBuf,1);

	return 0x00;

}


//Return 0: ok   1:fail
int check_by_name(unsigned char * item_buf,unsigned char * item_name,Sys_File ** t_sysfile)
{
    unsigned int ret=1;
	unsigned int i;
	Sys_File * tmp_item=(Sys_File *)item_buf;

	for (i=0;i<32;i++)
	{
		if (strcmp(item_name,tmp_item->Sys_Flag)==0)
		{
			*t_sysfile=tmp_item;
			ret=0;
			break;
		}
		tmp_item++;
	}
	return ret;
}

//Return 0: ok   1:fail
int static destory_root(int flag1)
{
	Sys_File *m_sysfile;
	char *TmpBuf[512];
    unsigned int i=0;
	unsigned int des_offset=0;
	NAND_Log_Read(0x01,TmpBuf,1);
	m_sysfile=(Sys_File *)TmpBuf;
	
	if (flag1==0)
	{
	    if(check_by_name(TmpBuf,"ROOTFS",&m_sysfile))
        {
            printf("<Test Info> not find ROOTFS !!\n");
            return 0x01;
        }   
	} 
	else
	{
	    if(check_by_name(TmpBuf,"ROOTBAK",&m_sysfile))
        {
            printf("<Test Info> not find ROOTBAK !!\n");
            return 0x01;
        }   
	}

	des_offset=m_sysfile->Sys_offset;
    //printf("root.offset == %5x\n",des_offset);
	memset(TmpBuf,0xaa,512);
	NAND_Log_Write(des_offset,TmpBuf,1);

	return 0x00;
}

int Destory_FW(unsigned int sum_param,char * param_buf)
{
	int param[10];
	char t_buf[10];
	char *tp=param_buf;
	int t_i=0;
	
	int * p_blk=&param[2];//得到块号信息
	int destoryblk_num=0;
	for (t_i=0;t_i<sum_param;t_i++)
	{
		strcpy(t_buf,tp);
		param[t_i]=atoi(t_buf);
		tp+=strlen(tp)+1;
	}//完成参数的转化，char-->int

    switch (param[0])
    {
    case 0:
		printf("<Test Info> Start to destory kernel ...\n");
		destory_kernel(0,param[1]);
		break;
	case 1:
		printf("<Test Info> Start to destory kernelbak ...\n");
		destory_kernel(1,param[1]);
    	break;
	case 2:
		printf("<Test Info> Start to destory rootfs ...\n");
		if(destory_root(0))
            return 0x01;
    	break;
	case 3:
		printf("<Test Info> Start to destory rootbak ...\n");
		if(destory_root(1))
            return 0x01;
    	break;
	default:
		break;
    }
    
	/*char *TmpBuf[512];

	NAND_Log_Read(0x10,TmpBuf,1);
	hex_printf2(TmpBuf,512);

	NAND_Log_Read(0x2810,TmpBuf,1);
	hex_printf2(TmpBuf,512);

	NAND_Log_Read(0x10080,TmpBuf,1);
	hex_printf2(TmpBuf,512);

	NAND_Log_Read(0x2ae80,TmpBuf,1);
	hex_printf2(TmpBuf,512);*/
	
	return 0x00;
}