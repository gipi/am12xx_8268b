/*
********************************************************************************
*						AM7511T---upgrade.drv
*				 (c) Copyright 2002-2007, Actions Co,Ld. 
*						 All Right Reserved 
*
* FileName: upgrade.h	  Author: Tian Zhimin		 Date:2008/06/26
* Description: defines functions for upgrading
* Others:	   
* History:		   
* <author>		 <time> 	  <version >	<desc>
* Tian Zhimin  2008/06/26		1.0 		build this file
********************************************************************************
*/ 
#include "am7x_ota.h"
#include "ota_common.h"
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/major.h>
#include <linux/syscalls.h>
#include "linux/delay.h"
#include <linux/device.h>
#include "ota_updata.h"
#include "am7x_flash_api.h"
#include "linux/kthread.h"
#include <linux/uaccess.h>
#include <am7x_dev.h>
#include <asm-mips/unaligned.h>
//#include <linux/semaphore.h>
//#include <linux/mutex.h>
#include <linux/it6681.h>
#include "../../../../../case/include/ezcastpro.h"
#ifndef __ACT_MODULE_CONFIG_H__ //For LinuxTool bug!! (i.e.; #if !define xxx)
#define __ACT_MODULE_CONFIG_H__
#include "../../../../../scripts/mconfig.h"
#endif
#ifdef CONFIG_MTD_M25P80
#include <linux/vmalloc.h>
#include "sys_buf_def.h"
#define SNOR_FLASH	//8252n,snor
unsigned char *fw_cache=NULL;
void nand_adfu_open();
void nand_adfu_close();
void nand_adfu_end();
int firmware_read(char *buf,unsigned int len);
int firmware_seek(unsigned int offset);
int snor_release()
{
	printk("snor_release\n");
	if(fw_cache)
	{
		vfree(fw_cache);
		fw_cache = NULL;
	}
	//nand_adfu_close();	//snor
}
#endif


#define	SECTOR_SIZE	  512
#define CHECK_FIRM    0x01
#define NR_LFI_SECTS  (0x800000*4 / 512)
#define FW_ID_NAME    "ActionsFirmware"
#define FW_ID_SIZE    16
#define MULTIBOOTFLAG "MultiSectorBoot"
#define OTADIRTYSIGN  0x55aaffff
#define OTACLEARSIGN  0x55aa5aa5

extern 	INT32S mem_ungzip(unsigned char *dest, INT32S *destLen, const unsigned char *source, INT32S sourceLen);
#if 1
#define UP_MSG(format,args...)   printk(format,##args)
#else
#define UP_MSG(format,args...)   do {} while (0)
#endif

Fwu_status_t fw_status =
{
	.prg = INI_PRG,
	.state = S_INIT
};
File_OP_t file_opp = 
{
	.file_pMODE = O_RDONLY
};

CapInfo_t old_capinfo;	//存放原来image的信息
int compress_flag;//固件压缩标志，1 ；压缩固件，0；未压缩固件,-1:无效固件
u8 * temp_buf_cached;
unsigned int checksum,fw_offset,filelen;
unsigned char *compress_cache=NULL,*src_cache=NULL;

unsigned int BootDiskType;		//表示系统当前的flash类型

static int brec_sector_num=32;

struct StorageInfo * tmpStorageInfo1;

struct task_struct *Upthread_tsk=NULL;

PartUpdate_Info  PartUpdate_info[LinuxMax];

INT32U bIrqFlag;
static INT32U kernelcap;

#define MAX__PARTS   16 
#define NAME_SIZE_MAX  10
#define PART_Str_SIZE   128 
int src_lenth=0,dest_lenth=0,uncomp_len=0;
struct Flash_Partiton  Fparts[MAX__PARTS];
struct Flash_Partiton  * linux_mbr=NULL;//*linux_mbr=Fparts;
#define FW_BACK_PAR_NAME  "reserve"

static uint32_t   fw_back_addr = 0,ota_read_lba=0,ota_write_lba=0,fw_back_space=0;
unsigned int   cal_checksum(unsigned char *ptr,   unsigned int len); 

static INT32S Check_All0xFF(INT8U *buf,INT32U lenth){
	INT32U i;
	for(i=0;i<lenth;i++){
		if(0xff != *buf)
			return 1;
		buf++;
	}
	return 0;
}

static inline unsigned long translate_block(unsigned long block)
{
	return block + kernelcap;
}

static int find_newfw_addr(unsigned int *lba){
	int	ret= -1,partCnt;
	u8  *TmpBuf=NULL,iLoop;
	if(memcmp(&BootDiskType,"F648",4)==0)
		printk("boot media is NAND Flash\n");
	partCnt=Flash_GetPartNum(Fparts); 
	TmpBuf=(char *)kzalloc(512,GFP_KERNEL);;
	if(NULL==TmpBuf)
		return ret;
	memset(TmpBuf,0,512);
	nand_adfu_read(translate_block(0x00),TmpBuf ,0x01);	
    for(iLoop =0x00;iLoop<partCnt;iLoop++)
    {              
        Fparts[iLoop].part_off =get_unaligned((INT32U*)(TmpBuf+256+16*(0+iLoop)+6));        
        Fparts[iLoop].part_size =get_unaligned((INT32U*)(TmpBuf+256+16*(0+iLoop)+10));
        printk("NO:%d,%s offset:%x,Cap:%x %dMB\n",iLoop,Fparts[iLoop].partName,Fparts[iLoop].part_off,
                Fparts[iLoop].part_size,  Fparts[iLoop].part_size/2048);
			if(strcmp(FW_BACK_PAR_NAME,Fparts[iLoop].partName) == 0){	
				*lba= translate_block(Fparts[iLoop].part_off);
				fw_back_space = Fparts[iLoop].part_size;
				printk("firmware backup address is 0x%x\n",*lba);
				linux_mbr= &Fparts[iLoop];
				ret = 0;			
				break;
			}
    } 
	kfree(TmpBuf);
	return ret;
}

void save_Irq(void)
{
	local_irq_save((unsigned long)bIrqFlag);
}

void restore_Irq(void)
{
	local_irq_restore((unsigned long)bIrqFlag);
}

 void _Print_Buf( const INT8U  * pad, const INT8U * pData, INT16U inLen)
{
	INT16U iLoop;
	printk("%s\n", pad);
	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printk("%02x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printk("  %d\n",iLoop);
		}
	}
       printk("\n");
	
}

void SetMultiBootPara(MuiltSectorBoot *MultiSecBoot,unsigned int BootDiskType,unsigned int len)
{
	int i;
	//重新计算checksum
	unsigned int *Boot=(unsigned int *)MultiSecBoot;
	unsigned int checksum=0;
//	
	strcpy(MultiSecBoot->MultiSecFlag,"MultiSectorBoot");

	if((BootDiskType==SNOR080)||(BootDiskType==SNOR090))
	{//snor
		MultiSecBoot->MbrecNum=1;

		MultiSecBoot->Mbrec0_Block=0;
		MultiSecBoot->Mbrec0_StartSector=1;
		MultiSecBoot->Mbrec0_SectorNum=len/512;
	}
	else
	{//nand
#ifndef SNOR_FLASH
		MultiSecBoot->MbrecNum=4;
		
		MultiSecBoot->Mbrec0_Block=0;
		MultiSecBoot->Mbrec0_StartSector=1;
		MultiSecBoot->Mbrec0_SectorNum=len/512;

		MultiSecBoot->Mbrec1_Block=1;
		MultiSecBoot->Mbrec1_StartSector=1;
		MultiSecBoot->Mbrec1_SectorNum=len/512;
		
		MultiSecBoot->Mbrec2_Block=2;
		MultiSecBoot->Mbrec2_StartSector=1;
		MultiSecBoot->Mbrec2_SectorNum=len/512;
		
		MultiSecBoot->Mbrec3_Block=3;
		MultiSecBoot->Mbrec3_StartSector=1;
		MultiSecBoot->Mbrec3_SectorNum=len/512;

		tmpStorageInfo1 = GetNandStorageInfo();
		UP_MSG("@PageNumPerPhyBlk=%d\n",tmpStorageInfo1->PageNumPerPhyBlk);
		MultiSecBoot->PAGE_CNT_PER_PHY_BLK = 
				(unsigned int)(tmpStorageInfo1->PageNumPerPhyBlk);//GetPageCntPerPhyBlk();	
#endif				
	}

	strcpy(MultiSecBoot->BootCheckFlag,"ActBrm");
	MultiSecBoot->BootCheckFlag[6]=(char)0xaa;
	MultiSecBoot->BootCheckFlag[7]=(char)0x55;
	
	checksum=0;	
	
	for(i=0;i<(512/sizeof(unsigned int)-1);i++)
		checksum=(unsigned int)(Boot[i]+checksum);
	checksum=(unsigned int)(checksum+0x1234);
	
	MultiSecBoot->BootCheckSum=checksum;

}

INT32U  _Get_TimeOut(INT8U  Flag)
{
	static struct timeval tpstart,tpend; 
	INT32U wTime;

	wTime=0x00;
	if(0x0==Flag)
	{	     	
		do_gettimeofday(&tpstart); 
	}
	else
	{
	   
		 do_gettimeofday(&tpend); 
		 wTime=1000000*(tpend.tv_sec-tpstart.tv_sec)+  tpend.tv_usec-tpstart.tv_usec; 
                //get ms
                  if(Flag ==2)//Display ms
                	{
               		 wTime/=1000;    
                	}
		
	}
	return wTime;	
}
#ifndef SNOR_FLASH
int mbrc_read_compare(int blocknum, INT8U *pMbrc, INT8U *buffer, unsigned int sectorCnt)
{
        INT32S i, ret;

        for(i=0; i<sectorCnt; i++)
        {        
                brec_sector_read(blocknum, i, buffer);
               //	UP_MSG("nand data of block:%d,  sector:%d is:\n",blocknum,i);
				//dump_read_data(buffer,512);
                ret = sector_compare((int *)buffer, (int *)(pMbrc+SECTOR_SIZE*i));
                if(ret == 0)
                {    
                	//printk("not equal sector is %d\n",i);
                    UP_MSG("mbrc_read_compare not equal====>break\n");						
                    return 1;	//not equal
                }
        }

	UP_MSG("mbrec equal\n");
	return 0;	//equal
}

int mbrc_write(int blocknum, INT8U *buffer, int sectorCnt)
{
    int i;			
    for(i=0; i<sectorCnt; i++)
    {
		brec_sector_write(blocknum, i, buffer+SECTOR_SIZE*i);
    }		
    return 0;
}
#endif

void feed_status(int prg, Fwu_state_t state)
{
	fw_status.prg = prg;
	fw_status.state = state;
//	UP_MSG("###feed status prg: %d ,%d\n",prg,state);
}
#ifdef SNOR_FLASH
static int Write_MBREC(void)
{
	FWIMG_Head_Item_t * Head_Item;
	unsigned int file_offset=0,file_length=0;
	if(CHK_Item_By_TYPE_NAME((unsigned char *)temp_buf_cached, (unsigned char*)&BootDiskType, "mbrec", &Head_Item))
	{
		ERR("MBREC Item is not found by type_name\n");		
		return 1;
	}	
	INFO("MBREC->type    : %s\n", Head_Item->type);
	INFO("MBREC->filename: %s\n", Head_Item->filename);
	INFO("MBREC->offset  : 0x%x\n", Head_Item->offset);
	INFO("MBREC->size    : 0x%x\n", Head_Item->size);
	INFO("MBREC->address : 0x%x\n", Head_Item->address);
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;

	int i;
	int sectornum;
	unsigned int len;
	unsigned char *MbrecPointer;//=(unsigned char *)new_mbrec_addr
	MuiltSectorBoot *SectorBoot;
	unsigned int old_mbrec_addr,new_mbrec_addr;
	
	len=Head_Item->size;	
	old_mbrec_addr = ((unsigned int)temp_buf_cached + FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));	//存放从文件中读出的数据, 
	new_mbrec_addr = ((unsigned int)temp_buf_cached + 512+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	
	SectorBoot=(MuiltSectorBoot *)new_mbrec_addr;
	memset((char *)SectorBoot,0,sizeof(MuiltSectorBoot));
	SectorBoot->MbrecRunAddr=Head_Item->para2;	
	MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
	firmware_seek(file_offset);
	firmware_read(MbrecPointer,file_length);

	SetMultiBootPara(SectorBoot,BootDiskType,len);
	ProcessBrec(MbrecPointer,len);
	
	len+=512;
	INFO("MBrec Size =0x%x\n",len);

	sectornum=len/512;
	if(sectornum>=16)
	{
		INFO_R("MBrec Size >=8 KB \n"); 		
		return 0x01 ;
	}
	
	MbrecPointer=(unsigned char *)new_mbrec_addr;

	if((BootDiskType==SNOR080)||(BootDiskType==SNOR090))
	{//snor 	
		for(i=0;i<sectornum;i++)
		{
			nand_adfu_read(i, (char *)old_mbrec_addr,1); 
			if(memcmp(MbrecPointer, (char *)old_mbrec_addr, 0x200))
			{
				INFO_R("Write_MBREC, num = %d\n", i);
				
				nand_adfu_write(i, MbrecPointer,1);
				nand_adfu_read(i, (char *)old_mbrec_addr,1); 

				if(memcmp(MbrecPointer, (char *)old_mbrec_addr, 0x200))
				{
					ERR("SNOR MBREC write err\n");
					
					return 1;
				}
					
			}

			MbrecPointer+=0x200;
		}
	}
	return 0;
}
static int Write_BREC (void)
{
	int  i, j;
	FWIMG_Head_Item_t * Head_Item;
	unsigned short CheckSumOld=0, CheckSumNew=0;
	char temp_buf[512];
	unsigned int file_offset=0,file_length=0;
	unsigned int old_brec_addr = ((unsigned int)temp_buf_cached + 512+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	int brec_start_lba;
	unsigned short *data_pointer;
	if(CHK_Item_By_TYPE_NAME((unsigned char *)temp_buf_cached, (unsigned char*)&BootDiskType, "brec", &Head_Item))
	{
		ERR("BREC Item is not found by type_name\n");
		
		return 1;
	}
	INFO("BREC->type	: %s\n", Head_Item->type);
	INFO("BREC->filename: %s\n", Head_Item->filename);
	INFO("BREC->offset	: 0x%x\n", Head_Item->offset);
	INFO("BREC->size	: 0x%x\n", Head_Item->size);
	INFO("BREC->address : 0x%x\n", Head_Item->address);
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;
	brec_sector_num = (Head_Item->size)>>9;
	firmware_seek(file_offset+file_length-512);
	firmware_read(temp_buf,512);
	CheckSumNew = *(unsigned short *)(temp_buf + 510);
	UP_MSG("BREC_SECTOR_NUM is %d\n", brec_sector_num );
	if(file_length != brec_sector_num*512)
		ERR("BREC Size May Be Wrong\n");

	brec_start_lba=SNOR_BREC_OFFSET/512;	//SNOR_BREC_OFFSET is mbrec size,can not be change

	nand_adfu_read(brec_sector_num - 1+brec_start_lba, (char *)old_brec_addr,1); 
	CheckSumOld = *(unsigned short *)(old_brec_addr + 510);

	if(CheckSumOld != CheckSumNew)	
	{
		INFO("BREC CheckSumOld: 0x%x, CheckSumNew: 0x%x\n", CheckSumOld, CheckSumNew);		
		firmware_seek(file_offset);
		for(i = 0; i < brec_sector_num; i++)
		{	
			firmware_read(temp_buf,512);
			nand_adfu_write(i+brec_start_lba,temp_buf,1);
		}	
		//check update result
#if 1	
		CheckSumOld=0;
		for(i = 0; i < brec_sector_num; i++)
		{	
			nand_adfu_read(i+brec_start_lba,temp_buf,1);
			data_pointer=(unsigned short *)temp_buf;
			if(i!=brec_sector_num-1)
			{
				for(j=0;j<512/sizeof(short);j++)
					CheckSumOld+=*data_pointer++;
			}
			else
			{
				for(j=0;j<512/sizeof(short)-1;j++)
					CheckSumOld+=*data_pointer++;
			}
		}
		if(CheckSumOld != CheckSumNew)	
			INFO(" recover brec error,CheckSumOld: 0x%x, CheckSumNew: 0x%x\n",CheckSumOld, CheckSumNew);		
		else
			INFO(" recover brec ok\n");		
#endif		
	}
	else
	{
		INFO("brec equal\n");	
	}

	INFO_R("Write_BREC over\n");
	
	return 0;
	
}

#else
static int Write_MBREC(void)
{
	unsigned short mbrec_sign;
	unsigned int CheckSumNew, CheckSumOld, i, j,*OTADirtyFlag;
	FWIMG_Head_Item_t * Head_Item;
	unsigned int file_offset=0,file_length=0,file_temp=0;
	unsigned int old_mbrec_addr,new_mbrec_addr;
	old_mbrec_addr = ((unsigned int)temp_buf_cached + FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));	//存放从文件中读出的数据, 
	new_mbrec_addr = ((unsigned int)temp_buf_cached + 512+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	if(CHK_Item_By_TYPE_NAME((unsigned char *)temp_buf_cached, (unsigned char*)&BootDiskType, "mbrec", &Head_Item))
	{
		ERR("MBREC Item is not found by type_name\n");		
		return 1;
	}	
	INFO("MBREC->type    : %s\n", Head_Item->type);
	INFO("MBREC->filename: %s\n", Head_Item->filename);
	INFO("MBREC->offset  : 0x%x\n", Head_Item->offset);
	INFO("MBREC->size    : 0x%x\n", Head_Item->size);
	INFO("MBREC->address : 0x%x\n", Head_Item->address);
	ota_read_lba = fw_back_addr+fw_offset;
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;
	file_temp = file_offset + file_length;
	mbrec_sign = *(unsigned short*)(old_mbrec_addr + 506);
	if(512==Head_Item->size)
	{
		ota_read_lba += (file_offset/512);
		if(file_temp%512 == 0){
			file_temp = file_temp/512 - file_offset/512;
		}else{
			file_temp = file_temp/512 - file_offset/512 +1;
		}
		nand_adfu_read(ota_read_lba,(unsigned char *)new_mbrec_addr,file_temp);
		new_mbrec_addr = (unsigned short *)((unsigned int)new_mbrec_addr + (unsigned int)(file_offset & 0x1ff));
		mbrec_sign = *(unsigned short*)(new_mbrec_addr + 506);
		OTADirtyFlag = (unsigned char *)(new_mbrec_addr + Head_Item->size);
		memset(OTADirtyFlag,0xff,512);
		*((unsigned int *)OTADirtyFlag) = OTADIRTYSIGN;

		if(mbrec_sign == 0x55aa)	//
		{
			CheckSumNew = my_calCRC2((unsigned char *)(new_mbrec_addr), 508, 4);
			CheckSumNew += 0x1234;
			if(CheckSumNew != *(unsigned int *)(new_mbrec_addr + 508 ))
			{
				ERR("The Geted MBREC Check SUM error, CheckSum: 0x%x, SHOULD BE: 0x%x\n", CheckSumNew, *(unsigned int *)(new_mbrec_addr + 508));
				
				return 1;
			}
		}
		else
		{
			ERR("Read_MBREC error\n");
			
			return 1;
		}
		
		//get old mbrec and recover
		for(i=0; i<4; i++)	//new_mbrec_addr未更新
		{
			brec_sector_read(i, 0, (char *)old_mbrec_addr); 	//????????读取数据的空间大小
			if(memcmp((char *)new_mbrec_addr, (char *)old_mbrec_addr, 0x200))
			{
				INFO_R("Write_MBREC, num = %d\n", i);
				
				brec_sector_write(i, 0, (char *)new_mbrec_addr);
				brec_sector_read(i, 0, (char *)old_mbrec_addr);
				
				CheckSumOld = my_calCRC2((unsigned char *)old_mbrec_addr, 508, 4);
				CheckSumOld += 0x1234;
				
				if(CheckSumOld != CheckSumNew)
				{
					ERR("Read MBREC again, checksum error\n");
					
					return 1;
				}
			}			
			brec_sector_read(i, 19, (INT8U *)(OTADirtyFlag+512));
			if(0 == Check_All0xFF((INT8U *)(OTADirtyFlag+512),512))
				brec_sector_write(i, 19, (INT8U *)OTADirtyFlag);
		}
	}
	else		//multi-sector
	{
		int sectornum;
		unsigned int len;
		unsigned char *MbrecPointer;//=(unsigned char *)new_mbrec_addr
		MuiltSectorBoot *SectorBoot;
		len=Head_Item->size;	
		INFO("multi mbrec len= 0x%x\n", len);
		SectorBoot=(MuiltSectorBoot *)new_mbrec_addr;
		memset((char *)SectorBoot,0,sizeof(MuiltSectorBoot));
		SectorBoot->MbrecRunAddr=Head_Item->para2;	
		MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
		ota_read_lba += (file_offset/512);
		MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
		nand_adfu_read(ota_read_lba,(unsigned char *)MbrecPointer,file_length/512);//(unsigned char *)MbrecPointer
		SetMultiBootPara(SectorBoot,BootDiskType,len);
		ProcessBrec(MbrecPointer,len);

		len+=512;
		INFO("MBrec Size =0x%x\n",len);

		sectornum=len/512;
		if(sectornum>=16)
		{
			INFO_R("MBrec Size >=8 KB \n");			
			return 0x01 ;
		}

		MbrecPointer=(unsigned char *)new_mbrec_addr;
		OTADirtyFlag=(unsigned char *)(MbrecPointer + len);
		memset(OTADirtyFlag,0xff,512);
		*((unsigned int *)OTADirtyFlag) = OTADIRTYSIGN;

		if((BootDiskType==SNOR080)||(BootDiskType==SNOR090))
		{//snor		
			for(i=0;i<sectornum;i++)
			{
				brec_sector_read(0, i, (char *)old_mbrec_addr); 
				if(memcmp(MbrecPointer, (char *)old_mbrec_addr, 0x200))
				{
					INFO_R("Write_MBREC, num = %d\n", i);
					
					brec_sector_write(0, i, MbrecPointer);
					brec_sector_read(0, i, (char *)old_mbrec_addr);

					if(memcmp(MbrecPointer, (char *)old_mbrec_addr, 0x200))
					{
						ERR("SNOR MBREC write err\n");
						
						return 1;
					}
						
			  	}

				MbrecPointer+=0x200;
			
			}
		}
		else
		{
			for(i=0;i<4;i++)
			{
				if( mbrc_read_compare( i, MbrecPointer,(INT8U*) old_mbrec_addr, sectornum) )//读并比较Block i Mbrec
                {
                    INFO_R("nand Write_MBREC, num = %d\n", i);
					ReadWrite_Entry(0x45,i,MbrecPointer+sectornum*512,0xffff);	
                    mbrc_write(i, MbrecPointer, sectornum);

                    if( mbrc_read_compare( i, MbrecPointer, (INT8U*) old_mbrec_addr, sectornum) )
                    {
                        INFO_R("nand second Write_MBREC, num = %d\n", i);						
						ReadWrite_Entry(0x45,i,MbrecPointer+sectornum*512,0xffff); 	
                        mbrc_write(i, MbrecPointer, sectornum);
                        if(!mbrc_read_compare( i, MbrecPointer, (INT8U*) old_mbrec_addr, sectornum) )
                        {
                            UP_MSG("find correct mbrec====>file:%s, line:%d\n", __FILE__, __LINE__);
                            j++;
                        }
					    else
					    {
							ERR("NAND MBREC write err\n");
							continue;

					    }
                    }
                    else 
                    {
                        UP_MSG("find correct mbrec====>file:%s, line:%d\n", __FILE__, __LINE__);
                        j++;
                    }
                 }
                 else 
                 {
                    UP_MSG("find correct mbrec====>file:%s, line:%d\n", __FILE__, __LINE__);
                    j++;
                 }
				brec_sector_read(i, (16 + sectornum)/16 * 16 + 3, (INT8U *)(OTADirtyFlag+512));
				if(0 == Check_All0xFF((INT8U *)(OTADirtyFlag+512),512))				 
				 	brec_sector_write(i,(16 + sectornum)/16 * 16 + 3,(INT8U *)OTADirtyFlag);
			}
		}
	}	
	INFO_R("Write_MBREC over\n");	
	return 0;		
}

static int Write_BREC (void)
{
	int  i, Brec_Num;
	FWIMG_Head_Item_t * Head_Item;
	unsigned short CheckSumOld=0, CheckSumNew=0;
	char temp_buf[1024];
	unsigned int file_offset=0,file_length=0,file_temp=0;
	unsigned int old_brec_addr = ((unsigned int)temp_buf_cached + 512+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	
	if(CHK_Item_By_TYPE_NAME((unsigned char *)temp_buf_cached, (unsigned char*)&BootDiskType, "brec", &Head_Item))
	{
		ERR("BREC Item is not found by type_name\n");
		
		return 1;
	}
	INFO("BREC->type    : %s\n", Head_Item->type);
	INFO("BREC->filename: %s\n", Head_Item->filename);
	INFO("BREC->offset  : 0x%x\n", Head_Item->offset);
	INFO("BREC->size    : 0x%x\n", Head_Item->size);
	INFO("BREC->address : 0x%x\n", Head_Item->address);
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;
	file_temp = file_offset + file_length;
	brec_sector_num = (Head_Item->size)>>9;
	ota_read_lba = fw_back_addr+fw_offset;
	ota_read_lba += ((file_temp/512)-1);
	nand_adfu_read(ota_read_lba,(unsigned char *)temp_buf,1);
	CheckSumNew = *(unsigned short *)(temp_buf + 510);
	UP_MSG("BREC_SECTOR_NUM is %d\n", brec_sector_num );
	UP_MSG("BootDiskType:0x%x\n", BootDiskType );
	if(file_length != brec_sector_num*512)
		ERR("BREC Size May Be Wrong\n");
		
		
	for(Brec_Num=4; Brec_Num<8; Brec_Num++)
	{
		brec_sector_read(Brec_Num, brec_sector_num - 1, (char *)old_brec_addr);
		CheckSumOld = *(unsigned short *)(old_brec_addr + 510);

		if(CheckSumOld != CheckSumNew)	
		{
			INFO("CheckSumOld: 0x%x, CheckSumNew: 0x%x\n", CheckSumOld, CheckSumNew);		
			INFO_R("Write_BREC, Num = %d\n", Brec_Num);			
			ReadWrite_Entry(0x45,Brec_Num,(unsigned char *)temp_buf,0xffff);	
			ota_read_lba = fw_back_addr+fw_offset;
			ota_read_lba += (file_offset/512);
			if(file_offset%512 == 0){
				file_temp = 1;
			}else{
				file_temp = 2;
			}
			for(i = 0; i < brec_sector_num; i++)
			{	
				nand_adfu_read(ota_read_lba,temp_buf,file_temp);
				ota_read_lba += 1;
				brec_sector_write(Brec_Num, i, (char *)(temp_buf + (file_offset & 0x1ff)));
			}	
			//check update result
			brec_sector_read(Brec_Num, brec_sector_num - 1, (char *)old_brec_addr);
			CheckSumOld = *(unsigned short *)(old_brec_addr + 510);
			if(CheckSumOld != CheckSumNew)	
			{
				INFO(" recover brec %d error,CheckSumOld: 0x%x, CheckSumNew: 0x%x\n", Brec_Num,CheckSumOld, CheckSumNew);		
			}
		}
		else
		{
			INFO("brec equal,Num = %d\n", Brec_Num);	
		}
	}


	INFO_R("Write_BREC over\n");
	
	return 0;
	
}
#endif

int firmware_read(char *buf,unsigned int len)
{
#ifdef SNOR_FLASH
	memcpy(buf,fw_cache+ota_read_lba*512,len);
#else
	nand_adfu_read(ota_read_lba,(unsigned char *)buf,len/512);
#endif
	ota_read_lba += (len/512);
	return 1;
}

int firmware_seek(unsigned int offset)
{
	INFO("firmware_seek to: 0x%x\n", offset);
#ifdef SNOR_FLASH
	ota_read_lba = (offset/512);
#else
	ota_read_lba = (offset/512) + fw_back_addr+fw_offset;
#endif
	return 1;
}

unsigned char *firmware_Uncompres(unsigned int offset,unsigned int ZipLen,unsigned int UnzipLen,unsigned int ZipChecksum,unsigned int UnzipChecksum)
{
	int ret;
	unsigned char *ZipData;
	unsigned char *UnzipData;
	unsigned int ZipLenAlign=(ZipLen+511)/512*512;
	unsigned int Curchecksum;
	ZipData = (unsigned char *)src_cache;
	if(NULL==ZipData)
		return NULL;
	UnzipData = (unsigned char *)compress_cache;
	if(NULL==UnzipData)
	{
		return NULL;
	}	
	firmware_seek(offset);
	firmware_read(ZipData,ZipLenAlign);
	Curchecksum = cal_checksum(ZipData,ZipLenAlign);;
	if(ZipChecksum != Curchecksum){
		ERR("zip file checksum error,current :0x%x,zip :0x%x\n",Curchecksum,ZipChecksum);
		return NULL;
	}	
	ret= mem_ungzip(UnzipData,&UnzipLen, ZipData, ZipLen);	//need add
	if(ret)
	{
		ERR("uncompress file error,offset :0x%x,len :0x%x,ret: %d,unziplen :0x%x\n",offset,ZipLen,ret,UnzipLen);
		return NULL;
	}	
	Curchecksum = cal_checksum(UnzipData,UnzipLen);
	if(UnzipChecksum != Curchecksum){
		ERR("unzip file checksum error,current :0x%x,unzip :0x%x,UnzipLen :0x%x\n",Curchecksum,UnzipChecksum,UnzipLen);
		return NULL;
	}		
	return UnzipData;
}

void Free_Unzipdata(unsigned char *Unzipdata)
{
	//free(Unzipdata);
}

int nandflash_read(int lbaoffset,char *buf,unsigned int sec)
{
//	INFO("NAND R:0x%x,%d\n",lbaoffset,sec);	
#ifdef SNOR_FLASH
	lbaoffset+=brec_sector_num+SNOR_BREC_OFFSET/512;
#endif
	nand_adfu_read(lbaoffset,(unsigned char *)(buf), (sec)); 
	return 1;
}

int nandflash_write(int lbaoffset,char *buf,unsigned int sec)
{
//	INFO("NAND W:0x%x,%d\n",lbaoffset,sec);	
#ifdef SNOR_FLASH
		lbaoffset+=brec_sector_num+SNOR_BREC_OFFSET/512;
#endif
    nand_adfu_write(lbaoffset, (unsigned char *)(buf), (sec)); 
//	ota_write_lba += (sec);
	return 1;
}

void feed_status_common(int prg, Fwu_state_t state)
{
	save_Irq();
	feed_status(prg, state);
	restore_Irq();
}

void delay_common(int times)
{
	mdelay(times);
}

INT8U *Test_Malloc(INT16U Size)
{
	return (u8*)kmalloc(Size, GFP_KERNEL);
}

static int Write_Image(unsigned int Logicalcap,unsigned int mbroffsetlba)
{
	upgrade_func func;
	func.firmware_read=firmware_read;
	func.firmware_seek=firmware_seek;
	func.nandflash_read=nandflash_read;
	func.nandflash_write=nandflash_write;
	func.feed_status_common=feed_status_common;
	func.delay_common=delay_common;
	func.firmware_Uncompres = firmware_Uncompres;
	func.Free_Unzipdata = Free_Unzipdata;
    INFO("Write IMG start\n");
#ifdef SNOR_FLASH
	nandflash_read(0,temp_buf_cached,1);	//read FLI 
	mbroffsetlba=*(INT32U *)(temp_buf_cached+0x90);
	INFO("old mbr Offset=0x%x\n",mbroffsetlba);

	firmware_seek(0);
	firmware_read(temp_buf_cached,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
#else	
	nand_adfu_read(fw_back_addr+fw_offset,temp_buf_cached,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512);
#endif
	if(0==Linux_Update(&func, Logicalcap,mbroffsetlba,temp_buf_cached,linux_mbr,PartUpdate_info))
		return 1;	
//exit flash driver
#ifndef SNOR_FLASH
    nand_logical_update();	
#endif
    INFO_R("Write IMG over\n");
    return 0;      
}

static int ClearOTADirtyFlag(void){
#ifndef SNOR_FLASH
	int i,j,sector_num;
	FWIMG_Head_Item_t * Head_Item;
	unsigned int file_offset=0,file_length=0,file_temp=0;
	unsigned int new_mbrec_addr;
	unsigned int len=0;
	unsigned char *MbrecPointer;
	MuiltSectorBoot *multiboot=(MuiltSectorBoot *)temp_buf_cached;
/*
	for(i=0;i<4;i++){
		brec_sector_read(i,0,temp_buf_cached);
		if(0 == memcmp(MULTIBOOTFLAG,temp_buf_cached,strlen(MULTIBOOTFLAG))){
			sector_num =(1+ multiboot->Mbrec0_SectorNum);
			for(j=1;j<sector_num;j++)		
				brec_sector_read(i,j,(unsigned char *)(temp_buf_cached+j*512));
			MbrecPointer = temp_buf_cached;
			break;
		}
	}
	if(i>=4){
	*/
		ota_read_lba = fw_back_addr+fw_offset;
		nand_adfu_read(ota_read_lba,temp_buf_cached,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512);
		new_mbrec_addr = ((unsigned int)temp_buf_cached+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
		if(CHK_Item_By_TYPE_NAME((unsigned char *)temp_buf_cached, (unsigned char*)&BootDiskType, "mbrec", &Head_Item))
		{
			ERR("MBREC Item is not found by type_name\n");		
			return 1;
		}
		file_offset = Head_Item->offset;
		file_length = Head_Item->size;
		file_temp = file_offset + file_length;
		len=Head_Item->size;
		multiboot =(MuiltSectorBoot *)new_mbrec_addr;
		memset((char *)multiboot,0,sizeof(MuiltSectorBoot));
		multiboot->MbrecRunAddr=Head_Item->para2;	
		MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
		ota_read_lba += (file_offset/512);
		MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
		nand_adfu_read(ota_read_lba,(unsigned char *)MbrecPointer,file_length/512);//(unsigned char *)MbrecPointer
		SetMultiBootPara(multiboot,BootDiskType,len);
		ProcessBrec(MbrecPointer,len);
		len+=512;
		sector_num=len/512;
		if(sector_num>=16)
		{
			INFO_R("MBrec Size >=8 KB \n");			
			return 0x01 ;
		}
		MbrecPointer=(unsigned char *)new_mbrec_addr;
//	}
	for(i=0;i<4;i++){
		ReadWrite_Entry(0x45,i,MbrecPointer+sector_num*512,0xffff);		
		for(j=0;j<sector_num;j++)
			brec_sector_write(i,j,(unsigned char *)(MbrecPointer+j*512));			
	}	
    INFO("Finish clearing OTA dirty flag\n");
#endif	
	return 0;
}
static void upgrade_main_task (void )
{
	INFO("upgrade_main_task\n");
#ifdef SNOR_FLASH
	firmware_seek(0);
	firmware_read(temp_buf_cached,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	INFO("lock system read and write \n");
	nand_adfu_open();		
#else
	ota_read_lba = fw_back_addr+fw_offset;
	nand_adfu_read(ota_read_lba,temp_buf_cached,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512);
#endif	
	save_Irq();//临界区保护，关中断	
	feed_status(INI_PRG, S_RUNNING);
	restore_Irq();//临界区保护，开中断

	if(Write_MBREC())
	{
		ERR("Write_MBREC error\n");
		save_Irq();//临界区保护，关中断	
		feed_status(WRITE_MBREC, S_ERROR);
		restore_Irq();//临界区保护，开中断
		goto UPGRADE_END;
	}	
	save_Irq(); 	//临界区保护，关中断	
	feed_status(INI_PRG, S_RUNNING);
	restore_Irq(); 	//临界区保护，开中断
	msleep(2);
	//S_OSTimeDly(2);
	if(Write_BREC())
	{
		ERR("Write_BREC error\n");
		save_Irq(); 	//临界区保护，关中断	
		feed_status(WRITE_BREC, S_ERROR);
		restore_Irq(); //临界区保护，开中断
		goto UPGRADE_END;
	}
	save_Irq();	//临界区保护，关中断	
	feed_status(INI_PRG, S_RUNNING);
	restore_Irq();//临界区保护，开中断	 
	mdelay(2);	
	if(Write_Image(old_capinfo.Logicalcap,old_capinfo.Micap))
	{
		ERR("Write_Image error\n");
		restore_Irq();//临界区保护，关中断	
		feed_status(WRITE_FW, S_ERROR);
		restore_Irq();//临界区保护，开中断
		goto UPGRADE_END;
	}
	save_Irq();
	ClearOTADirtyFlag();	
	restore_Irq();	//临界区保护，开中断
	mdelay(2);	
	save_Irq(); 	//临界区保护，关中断	
	feed_status(FINISH_PRG, S_FINISHED);	
	restore_Irq();	//临界区保护，开中断	
	UP_MSG("####upgrade success ##### \n");	
UPGRADE_END:
	src_cache = NULL;
	compress_cache = NULL;
#ifdef SNOR_FLASH
#if 1
	printk("snor_end\n");
	if(fw_cache)
	{
		vfree(fw_cache);
		fw_cache = NULL;
	}
	nand_adfu_end();
#else
	snor_release();
#endif
	
#endif
	mdelay(2);	
	UP_MSG("####main task over ##### \n");	
}

#if CHECK_FIRM==0x01
unsigned int   cal_checksum(unsigned char *ptr,   unsigned int len)   
{   
	
        unsigned int CheckSum,i;
        unsigned int *D_ptr;
        D_ptr=(unsigned int *)ptr;
        CheckSum=0;
        for(i=0;i<len/sizeof(unsigned int);i++)   
        {
                CheckSum += *D_ptr;
                D_ptr++;
        }
        return CheckSum;
}

unsigned int Firm_ChechsumFlow(void)
{
        int offset,ret=0;
        _Get_TimeOut(0);
#ifdef SNOR_FLASH
		offset=FindSysFile("CHECKSUM",fw_cache);//查看是否含有checksum信息
		if(offset<0) return -1;
		
		Sys_File *CurSysFile=(Sys_File*)(fw_cache+offset);
		unsigned int ReadCheckSum=CurSysFile->Sys_para2;	//固件中的checksum
		unsigned int CalcuCheckSum=0;
		CalcuCheckSum=cal_checksum(fw_cache+CurSysFile->Sys_offset,CurSysFile->Sys_len);
        if(ReadCheckSum !=CalcuCheckSum)
        {
            INFO("firmware checksum error\n");
            INFO("%x,%x\n",ReadCheckSum,CalcuCheckSum);
            ret=-1;	//不能再继续升级了
        }
		else
            INFO("firmware checksum ok\n");
#else
        INFO("Start Time: 0x%p\n",temp_buf_cached);
		/*if OTA file is nomal file*/
		if(compress_flag >=0){
			nand_adfu_read(ota_read_lba,temp_buf_cached,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512);
	        offset=FindSysFile("CHECKSUM",temp_buf_cached);//查看是否含有checksum信息
	        if(offset>0)
	        {
	                Sys_File *CurSysFile=(Sys_File*)(temp_buf_cached+offset);
	                unsigned int ReadCheckSum=CurSysFile->Sys_para2;	//固件中的checksum
					unsigned char *Buf;
	                unsigned int CalcuCheckSum=0;
	                int remainlen=CurSysFile->Sys_len;		//需计算的固件长度	
	                int size=SECTOR_RW_ONCE*512;		//可增大或减小   
					if((1 == compress_flag)&&(compress_cache)){
						Buf = (unsigned char *)compress_cache;
						size = dest_lenth/512*512;						
					}
					else{
						Buf = temp_buf_cached;
					}						
	                INFO("Sys_Flag:%s,\n",CurSysFile->Sys_Flag);
	                INFO("Sys_offset:0x%x,\n",CurSysFile->Sys_offset);
	                INFO("Sys_len:0x%x, %d KB\n",CurSysFile->Sys_len,(CurSysFile->Sys_len)/1024);
	                INFO("Sys_para1:0x%x,\n",CurSysFile->Sys_para1);
	                INFO("Sys_para2:0x%x,\n",CurSysFile->Sys_para2);
					
					nand_adfu_read(ota_read_lba,Buf,size/512);
					ota_read_lba += size/512;
	            	CalcuCheckSum +=cal_checksum(Buf+ CurSysFile->Sys_offset,size-CurSysFile->Sys_offset);
					remainlen -= size-CurSysFile->Sys_offset;
	                while(remainlen>0)
	                {
	                        if(remainlen<size)
	                                size=remainlen;
	                     	nand_adfu_read(ota_read_lba,Buf,size/512);
							ota_read_lba += size/512;                   
	                        CalcuCheckSum +=cal_checksum(Buf,size);
	                        remainlen -= size;
	                }
	              
	                if(ReadCheckSum !=CalcuCheckSum)
	                {
	                        INFO("firmware checksum error\n");
	                        INFO("%x,%x\n",ReadCheckSum,CalcuCheckSum);
	                        ret=-1;
	                //不能再继续升级了
	                }

	        }
	        else
	        {
	                ret=-1;
	                //固件中不含checksum信息。
	        }
		}
#endif	
        INFO("End of Time:%d ms\n", _Get_TimeOut(2));    
        return ret;
}
#endif

static int Updata_main_thread(void *data)
{
	int ret=0;
	INFO("ota upgrade start 2013-06-27 svn:10105\n");
	feed_status(INI_PRG, S_INIT);
	ota_read_lba = fw_back_addr+fw_offset;
	ota_write_lba = fw_back_addr+fw_offset;
#if CHECK_FIRM==0x01   
	feed_status(INI_PRG, S_WAIT);
	if(0x00 != Firm_ChechsumFlow())
	{
		ERR("Can not find firmware or checksum error\n");
		ret = -EIO;
		feed_status(INI_PRG, S_ERROR);
#ifdef SNOR_FLASH
		snor_release();
#endif		
		goto OTAUPEnd;
	}
	INFO("Firm_ChechsumFlow over  ok\n");	
#endif 
	schedule();
	upgrade_main_task();
	while(!kthread_should_stop())
	{		
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}
	return 0;
OTAUPEnd:
	while(!kthread_should_stop())
	{
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();
	}	
	return ret;	
}

int Fwu_upgrade_ota(void)
{
	int ret=0;
	if(0xffffffff == BootDiskType)
	{
		ERR("Can Not Identify The BootDisk Type\n");
		ret = -EIO;
		feed_status(INI_PRG, S_ERROR);
#ifdef SNOR_FLASH
		snor_release();
#endif		
		goto OutEnd;
	}
	Upthread_tsk=kthread_run(Updata_main_thread,NULL,"upgrade_main_task");	
	if (IS_ERR(Upthread_tsk))
	{
		ERR("task up error\n");
		return (int)PTR_ERR(Upthread_tsk);
	}	
	INFO("upgrade main ok\n");		
	return 0;
OutEnd:
	return ret;
}
#ifdef SNOR_FLASH
int ota_write_data(ota_data_t *ota)
{
	int len;
	int offset;
	if(ota->sector_num > MAX_SECTOR_ONCE)
		len=MAX_SECTOR_ONCE;
	else
		len=ota->sector_num;

	len*=512;
	offset=ota->offset*512;
	memcpy(fw_cache+offset,ota->buf,len);
	return 0;
}
int ota_read_data(ota_data_t *ota)
{
	int len;
	int offset;
	if(ota->sector_num > MAX_SECTOR_ONCE)
		len=MAX_SECTOR_ONCE;
	else
		len=ota->sector_num;

	len*=512;
	offset=ota->offset*512;
	memcpy(ota->buf,fw_cache+offset,len);
	return 0;
}
#else
int ota_write_data(ota_data_t *ota){
	ota_write_lba = fw_back_addr + ota->offset;
//	printk("in ota write addr:0x%x\n",ota_write_lba);
	if(ota->sector_num > MAX_SECTOR_ONCE){
		return nand_adfu_write(ota_write_lba,(unsigned char *)(ota->buf),MAX_SECTOR_ONCE);
	}else{
		return nand_adfu_write(ota_write_lba,(unsigned char *)(ota->buf),ota->sector_num);	
	}
}

int ota_read_data(ota_data_t *ota){
	ota_read_lba = fw_back_addr + ota->offset;
	if(ota->sector_num > MAX_SECTOR_ONCE){
		return nand_adfu_read(ota_read_lba,(unsigned char *)temp_buf_cached,MAX_SECTOR_ONCE);
	}else{
		return nand_adfu_read(ota_read_lba,(unsigned char *)temp_buf_cached,ota->sector_num);	
	}
}
#endif

int Fwu_ota_upgradeExit(void)
{
	if(Upthread_tsk)
	{	
		save_Irq();
		printk("stop MyThread\n");
		kthread_stop(Upthread_tsk);
		if(fw_status.state==S_FINISHED){
			printk("Reboot now\n");
#if MODULE_CONFIG_HDMI2MHL_ENABLE==1
			it6681_i2c_poweroff();//reset MHL
#endif			
			am_setup_watchdog(1);
		}
		restore_Irq();
	}
	return 0x00;	
}

int Fwu_getStatus(Fwu_status_t *status)
{
	status->prg = fw_status.prg;
	status->state = fw_status.state;
//	UP_MSG("###get status prg: %d ,%d\n",status->prg ,status->state);
	return 0;
}

static int am_ota_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int ret=0,i=0,tmp;
	ota_data_t ota_src;
	Fwu_status_t  Tmp;
	int ret2;
	switch(cmd){
		case OTA_UPGRADE:
			ret=Fwu_upgrade_ota();			
			break;
		case OTA_GETSTATUS:	
			Fwu_getStatus(&Tmp);			
			ret=copy_to_user((Fwu_status_t *)arg,(Fwu_status_t *)&Tmp,sizeof(Tmp));		
		//	UP_MSG("FWU_GETSTATUS ret:%d\n",ret);		
			break;
		case OTA_EXITUP:
			ret=Fwu_ota_upgradeExit();
			break;
		case OTA_GETPRG:
			ret2=access_ok(VERIFY_WRITE, (unsigned char __user*)arg, 4) ;
			UP_MSG("1 FWU_GETPRG ret:%d\n",ret2);
			ret2 = put_user(fw_status.prg,(unsigned char __user*)arg);
			UP_MSG("2 FWU_GETPRG ret:%d\n",ret2);
			break;
		case OTA_ENTER:
			ret=copy_to_user((unsigned int *)arg,(unsigned int *)&fw_back_space,sizeof(fw_back_space));	
			if(ret != 0){
				printk("copy to usr error_%d",__LINE__);
				ret = -1;				
			}
			break;
		case OTA_DOWNLOAD_FW:		
			if(copy_from_user((ota_data_t *)&ota_src,(ota_data_t *)arg,sizeof(struct ota_fw))){
				printk("copy from usr error_%d\n",__LINE__);
				ret = -1;
				return ret;
			}				
			if(copy_from_user((char *)temp_buf_cached,(char *)ota_src.buf,OTA_RWSIZE)){				
				printk("copy from usr error_%d\n",__LINE__);
				ret = -1;
				return ret;				
			}
			ota_src.buf=(char *)temp_buf_cached;
			do{
				if(0==ota_write_data(&ota_src))
					break;
				else
					i++;
			}while(i<5);
			if(i>=5){
				printk("write OTA firemware error\n");
				ret= -1;				
			}
			break;
		case OTA_READ_FW:
			if(copy_from_user((ota_data_t *)&ota_src,(ota_data_t *)arg,sizeof(ota_data_t))){
				printk("copy from usr error_%d",__LINE__);
				ret = -1;
				return ret;
				}
			ret=ota_read_data(&ota_src);
			if(0!=ret){
				printk("ota read OTA firmware error,ret=%d\n",ret);
				return ret;
			}
			if(copy_to_user((char *)ota_src.buf,(char *)temp_buf_cached,ota_src.sector_num * 512)){
				printk("copy to usr error_%d\n",__LINE__);
				ret = -2;
			}
			break;
		case OTA_WRITE_FW:
			copy_from_user((ota_data_t *)&ota_src,(ota_data_t *)arg,sizeof(ota_data_t));
			ota_write_data(&ota_src);				
			copy_to_user((ota_data_t *)arg,(ota_data_t *)&ota_src,sizeof(ota_data_t));
			break;
		case OTA_SETPARTUPGRADE:
			if(0 != copy_from_user(PartUpdate_info,(PartUpdate_Info *)arg,sizeof(PartUpdate_Info) * LinuxMax)){
				printk("copy part upgrade info error!\n");
				ret= -EFAULT;
			}
			break;
		case OTA_GETFWSTATUS:
#ifdef SNOR_FLASH
			firmware_seek(0);
			firmware_read(temp_buf_cached,512);
#else			
			nand_adfu_read(fw_back_addr,temp_buf_cached,1);
#endif
			if(memcmp(temp_buf_cached,FW_ID_NAME,FW_ID_SIZE) ==0){
				fw_offset = 0;
				tmp =FindSysFile("COMPRESS",temp_buf_cached);
				if(tmp>0){
					compress_flag = 1;
					Sys_File *CurSysFile=(Sys_File *)((unsigned int)temp_buf_cached+tmp);
					src_lenth= (int) CurSysFile->Sys_para1;			//for malloc zip memory
					dest_lenth= (int) CurSysFile->Sys_para2;	   //for malloc unzip memory					
				}
				else{
					compress_flag = 0;
					src_cache = NULL;
					compress_cache = NULL;
				}
			}
			else{
				compress_flag = -1;
			}			
			if(copy_to_user((int *)arg,(int *)&compress_flag,sizeof(compress_flag))){
				printk("copy from usr error_%d",__LINE__);
				ret = -1;
				return ret;
			}
			break;
		case OTA_GETCOMPRFILELENTH:
			i =  src_lenth;
			if(copy_to_user((int *)arg,(int *)&i,sizeof(i))){
				printk("copy from usr error_%d",__LINE__);
				ret = -1;
				return ret;
			}
			break;
		case OTA_GETORIFILELENTH:
			i =  dest_lenth;
			if(copy_to_user((int *)arg,(int *)&i,sizeof(i))){
				printk("copy from usr error_%d",__LINE__);
				ret = -1;
				return ret;
			}
			break;
		case OTA_GETFILEOFFSET:
			i =  fw_offset;
			if(copy_to_user((int *)arg,(int *)&i,sizeof(i))){
				printk("copy from usr error_%d",__LINE__);
				ret = -1;
				return ret;
			}
			break;
		case OTA_SETSRCADDR:
			src_cache = arg;
			break;
		case OTA_SETDESTADDR:			
			compress_cache = arg;
			break;
#ifdef SNOR_FLASH			
		case OTA_SETFWLEN:
			fw_cache=vmalloc(arg);
			if(!fw_cache)
			{
				printk("fw_cache vmalloc %d error\n\n",arg);
				ret = -1;
				return ret;
			}
			break;
#endif			
		case OTA_UPDATEMBR: 
			if(copy_from_user((ota_data_t *)&ota_src,(ota_data_t *)arg,sizeof(struct ota_fw))){
				printk("copy from usr error_%d\n",__LINE__);
				ret = -1;
				return ret;
			}				
			if(copy_from_user((char *)temp_buf_cached,(char *)ota_src.buf,OTA_RWSIZE)){				
				printk("copy from usr error_%d\n",__LINE__);
				ret = -1;
				return ret;				
			}
			if(1 != MakeNewMbr(temp_buf_cached,old_capinfo.Logicalcap,(char *)(temp_buf_cached+1024),&fw_back_addr)){				
				printk("Make new MBR error\n");
				ret = -2;
			}
			break;
		default:
			break;
	}
	return ret;
}

static int am_ota_open(struct inode *pinode, struct file *pfile)
{
	UP_MSG("am_upgrade_open\n");
	if(!pfile)
		return -ENOENT;
	pfile->f_pos = 0;
	return 0;
}

static int am_ota_release(struct inode *pinode, struct file *pfile)
{
	UP_MSG("am_upgrade_release\n");
	return 0;
}
static struct file_operations otadrv_fops={
	.owner  = THIS_MODULE,
	.llseek = NULL,	
	.ioctl = am_ota_ioctl,
	
	.open = am_ota_open,
	.release = am_ota_release,
};
dev_t  ota_dev;
struct cdev  *ota_cdev=NULL;
static struct  class *ota_class=NULL; 

static int __init  am7x_ota_init(void) 
{
	int result=0;
	LFIHead_t * lfi_hd;	
	printk("ota start\n");	
	memset((char *)PartUpdate_info,0,sizeof(PartUpdate_Info) * LinuxMax);
	ota_dev =MKDEV(AM_OTA_MAJOR,0);
	result = register_chrdev_region(ota_dev,0x01,"otadrv");
	printk("ota register ret=%d\n",result);
	if(result){
		printk(KERN_ERR "alloc_chrdev_region() failed for upgrade %d\n",result);
		return -EIO;
	}
	printk("ota major=%d, minor=%d\n",MAJOR(ota_dev),MINOR(ota_dev));
	ota_cdev = kzalloc(sizeof(struct cdev),GFP_KERNEL);
	if(!ota_cdev){
		printk(KERN_ERR "malloc memory  fails for ota device\n");
		unregister_chrdev_region(ota_dev,0x01);
		return -ENOMEM;
	}
  	cdev_init(ota_cdev, &otadrv_fops);
	if(cdev_add(ota_cdev, ota_dev, 1)){
		printk(KERN_ERR "can not create a cdev for otadrv\n");
		goto Init_err;
	}
	ota_class = class_create(THIS_MODULE,"otadrv");
	if(IS_ERR(ota_class)){
		printk(KERN_ERR "can not create a cdev_class for otadrv\n");
		goto Init_err;
		}
	device_create(ota_class,NULL,ota_dev,0,"otadrv");	
	temp_buf_cached = (unsigned char *)kmalloc((SECTOR_RW_ONCE+1)*512, GFP_KERNEL);
	if(0 == temp_buf_cached)
	{
		ERR("fatal alloc error.\n");
		goto Init_err;
	}
	fw_offset = 0;
#ifdef SNOR_FLASH
	//nand_adfu_open();
	//nand_adfu_read((SNOR_BREC_OFFSET+SNOR_BREC_SIZE)/512,temp_buf_cached,0x01);
	BootDiskType = get_nand_flash_type();	
	old_capinfo.Logicalcap = 64*1024*1024;	//no use in snor
#else
	nand_adfu_read(0x0,temp_buf_cached,0x01);
	kernelcap=*(INT32U *)(temp_buf_cached+0x90);
	if(0x00 == kernelcap)
		kernelcap = 0x10000;
	printk("kernel capacity is:0x%x",kernelcap);	
	BootDiskType = get_nand_flash_type();	
	INFO("BootDiskType: 0x%x\n", BootDiskType);
	if(0xffffffff == BootDiskType)
	{
		ERR("Can Not Identify The BootDisk Type\n");
		kfree(temp_buf_cached);
		goto Init_err;		
	}
#ifndef SNOR_FLASH 
	if(find_newfw_addr(&fw_back_addr)){
		ERR("No Space For OTA function\n");
		goto Init_err;
		}
	nand_adfu_read(0,temp_buf_cached,1);
#endif	
	lfi_hd = (LFIHead_t *)temp_buf_cached;	
	memcpy(&old_capinfo, &(lfi_hd->CapInfo), sizeof(CapInfo_t));
	INFO_R("CapInfo.Scodecap	: 0x%x\n", old_capinfo.Scodecap);
	INFO_R("CapInfo.Scodebakcap	: 0x%x\n", old_capinfo.Scodebakcap);
	INFO_R("CapInfo.Appinfocap	: 0x%x\n", old_capinfo.Appinfocap);
	INFO_R("CapInfo.Liccap		: 0x%x\n", old_capinfo.Liccap);
	INFO_R("CapInfo.Micap		: 0x%x\n", old_capinfo.Micap);
	INFO_R("CapInfo.Dspcap		: 0x%x\n", old_capinfo.Dspcap);
	INFO_R("CapInfo.Udiskcap	: 0x%x\n", old_capinfo.Udiskcap);
	INFO_R("CapInfo.Logicalcap	: 0x%x\n", old_capinfo.Logicalcap);	
#endif

	return 0;
Init_err:
	printk(KERN_ERR "register failed  for Upgrade device\n");
	kfree(ota_cdev);
	device_destroy(ota_class,ota_dev);
	class_destroy(ota_class);
	unregister_chrdev_region(ota_dev,0x01);	
	return -ENODEV;
}

static void __exit  am7x_ota_exit(void) 
{
	INFO("upgrade_uninstall\n");
	if(ota_cdev){
		cdev_del(ota_cdev);
		kfree(ota_cdev);
	}
	device_destroy(ota_class,ota_dev);
	class_destroy(ota_class);
	unregister_chrdev_region(ota_dev,0x01);
	if(temp_buf_cached != NULL)
		kfree(temp_buf_cached);
}

module_init(am7x_ota_init);
module_exit(am7x_ota_exit);

MODULE_DESCRIPTION("Actions-micro ota");
MODULE_AUTHOR("leiwg<leiwg@actions-micro.com>");
MODULE_LICENSE("GPL");





