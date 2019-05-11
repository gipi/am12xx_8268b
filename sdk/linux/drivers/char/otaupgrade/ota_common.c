#include "am7x_flash_api.h"
#include "ota_common.h"
#include "am7x_ota.h"

LinuxUpdateInfo LinuxUp;
char *mbr;
DWORD LinuxOffset;
DWORD DownloadFinish=0;
DWORD DownloadWholeLen;
upgrade_func *fun;
BYTE *FirmBuffer;
unsigned int DecryptFlag=0;
unsigned int WholeFirmwareLen=0;
enum{FAT12,FAT16,FAT32};

static INT32S byte_compare( INT32S* srcdata_addr, INT32S* dstdata_addr,int len )
{
    INT32S i, *p, *q;
    p=srcdata_addr;
    q=dstdata_addr;
    for( i=0; i<len/4; i++)
    {
        if( (*p) != (*q) )
            return 0;
        p++;
        q++;
    }
    return 1;
}

INT32S sector_compare( INT32S* srcdata_addr, INT32S* dstdata_addr )
{
    INT32S i, *p, *q;
    p=srcdata_addr;
    q=dstdata_addr;
    for( i=0; i<128; i++)
    {
        if( (*p) != (*q) )
            return 0;
        p++;
        q++;
    }
    return 1;
}

int ProcessBrec(unsigned char BrecBuf[],unsigned int BrecSize)
{
	int i;
	unsigned short *brecc;
	unsigned short checksum;
	
	if(BrecSize<4) 
		return 1;

	BrecBuf[BrecSize-4]=(INT8U)0xaa;
	BrecBuf[BrecSize-3]=(INT8U)0x55;
	
	
	
	brecc=(unsigned short *)BrecBuf;
	
	checksum=0;
	
	for(i=0;i<(BrecSize/sizeof(unsigned short)-1);i++)
		checksum=(unsigned short)(brecc[i]+checksum);

	memcpy(BrecBuf+BrecSize-sizeof(unsigned short),&checksum,sizeof(unsigned short));//the last WORD is checksum
	return 1;
}



int my_calCRC2(unsigned char *buf, unsigned int length, unsigned char nBytes)
{
	unsigned int i=0,j=0;
	int checkSum=0;
	short checkSumShort=0;
	if(length == 0)
		return 0;
	for(i=0; i<length/nBytes; i++)
	{
		for(j=0; j<nBytes; j++)
		{
			checkSum += (buf[nBytes*i+j])<<(j*8);
			if(nBytes == 2)
				checkSumShort += (short)(buf[nBytes*i+j])<<(j*8);
		}
	}
	if(nBytes == 2)
		return checkSumShort;
	else
		return checkSum;
}

int CHK_Item_By_TYPE (unsigned char * Buff_addr, unsigned char * type, FWIMG_Head_Item_t ** Item)
{
	int ret = 1;
	int i;
	FWIMG_Head_Item_t * tmp_item = (FWIMG_Head_Item_t *) Buff_addr;
	
	for(i=0; i<FWIMG_Head_Max; i++)
	{
		if(memcmp(tmp_item->type, type, 4) == 0)
		{
			*Item = tmp_item;
			ret = 0;
			break;
		}
		tmp_item++;
	}

	return ret;

}

int CHK_Item_By_NAME (unsigned char * Buff_addr, unsigned char * filename, FWIMG_Head_Item_t ** Item)
{
	int ret = 1;
	int i;
	FWIMG_Head_Item_t * tmp_item = (FWIMG_Head_Item_t *) Buff_addr;

	for(i=0; i<FWIMG_Head_Max; i++)
	{
		if(strcmp(filename, tmp_item->filename) == 0)
		{
			*Item = tmp_item;
			ret = 0;
			break;

		}
		tmp_item++;
	}

	return ret;

}

int CHK_Item_By_TYPE_NAME (unsigned char * Buff_addr, unsigned char * type, char * filename, FWIMG_Head_Item_t ** Item)
{
	int ret = 1;
	int i;
	FWIMG_Head_Item_t * tmp_item = (FWIMG_Head_Item_t *) Buff_addr;
	
	for(i=0; i<FWIMG_Head_Max; i++)
	{
		if((memcmp(tmp_item->type, type, 4) == 0)&&(strcmp(filename, tmp_item->filename) == 0))
		{
			*Item = tmp_item;
			ret = 0;
			break;
		}
		tmp_item++;
	}

	return ret;

}




int FindSysFile(char *SysFlag, unsigned char *buf)
{
        int i;
        
        Sys_File * sysfileinfo;
        
        sysfileinfo=(Sys_File *)buf;

        for(i=0;i<FWIMG_Head_Max;i++)
        {
                if(0==strcasecmp(SysFlag,sysfileinfo->Sys_Flag))
                        break;
                sysfileinfo++;
        }

        if(i==FWIMG_Head_Max)
                return -1;
        else
                return i*sizeof(Sys_File );
	
}

int mbr_conv(void *old_mbr, part_desc *p, int n)
{
	int i;
	struct priv_mbr *mbr = (struct priv_mbr *)old_mbr;
	struct priv_ptr *ptr = mbr->parts;
	
	memset(mbr, 0, sizeof(*mbr));
	for(i=0; i<n; i++, ptr++, p++) {
		ptr->part_off  = p->p_start;
		ptr->part_size = p->p_size;
		ptr->system_id = p->p_sysid;
		ptr->status    = 0x0;
		snprintf(ptr->id, 6, "%s", p->f_attrstr);
	//	DMSG("OFF:0x%08x, SIZE:0x%08x\n", ptr->part_off, ptr->part_size);
	}
	strcpy((char*)mbr, 
		"Actions MicroEletronics Native MBR for Embedded Linux on NandFlash\n"
		"Version 1.0.0\n" 
		__TIME__ "\n"
		__DATE__ "\n"
		"Designed by Pan Ruochen <ijkxyz@msn.com>\n"
		);
	mbr->signature = MBR_SIGNATURE2;
	return 0;
}


int FindUpdateInfo(char *attrstr, PartUpdate_Info *partupdate_info_p)
{
	int i;
    for(i=0;i<LinuxMax;i++)
    {
        if(0==strcasecmp(attrstr,partupdate_info_p->partupdate_attrstr))
        {
            INFO("%s partupgrade info is %d\n",attrstr,partupdate_info_p->partupdate_flag);
			return partupdate_info_p->partupdate_flag;
        }
        partupdate_info_p++;		
    }
	INFO("no %s partupgrade info\n",attrstr);
	return 1;		//default update
}

int UdiskFormat(BYTE *buf, DWORD totalsec,DWORD *SelectFileLen,DWORD Udiskoffset)
{
	int temp1;
	int Fat;
	BYTE bitPerFat;
	char m_SysID[8];
		
	DWORD tempM=totalsec/(2*1024);	//to M byte
	DWORD TotalSector=totalsec;
	
	INFO("Udisk totalsec is 0x%x,%dMB,Udiskoffset is 0x%x\n",totalsec,tempM,Udiskoffset);

	Fat_BPB	BPB;
	memset(&BPB,0,sizeof(Fat_BPB));
	
	if(tempM<3)	//differ with pc
	{
		Fat=FAT12;
		bitPerFat=12;
		strcpy(m_SysID,"Fat12");
		
		BPB.SamllSector=(WORD)TotalSector;
		BPB.LargeSector=0;
		
		BPB.SecPerClust=1;
		BPB.RootEntCnt=512;
	}
	else if(tempM<32)
//	if(tempM<32)
	{
		Fat=FAT16;
		bitPerFat=16;
		strcpy(m_SysID,"Fat16");
		
		BPB.SamllSector=(WORD)TotalSector;
		BPB.LargeSector=0;
		
		BPB.SecPerClust=1;
		BPB.RootEntCnt=512;
		
	}
	else
	{
		Fat=FAT32;
		bitPerFat=32;
		strcpy(m_SysID,"Fat32");

		BPB.LargeSector=TotalSector;
		BPB.SamllSector=0;
		
		if( TotalSector < 0x20001 ){    // 小于64M
			BPB.SecPerClust = 1;
		}else if( TotalSector < 0x40001 ){    // 小于128M
			BPB.SecPerClust = 2;
		}else if( TotalSector < 0x80001){    // 小于256M
			BPB.SecPerClust = 4;
		}else if( TotalSector < 0x1000001 ){   // 小于8G
			BPB.SecPerClust = 8;
		}else if( TotalSector < 0x2000001 ){   // 小于16G
			BPB.SecPerClust = 16;
		}else
			BPB.SecPerClust = 32;
		
		BPB.RootEntCnt=0;
		
	}
	
	BPB.RsvdSecCnt=1;

	if( Fat==FAT32) 
		BPB.RsvdSecCnt=32;

	
	temp1=(TotalSector- BPB.RsvdSecCnt- BPB.RootEntCnt*32/512-2* BPB.SecPerClust)*bitPerFat;
	if(temp1<=0)
	{
		return 0;
	}
	DWORD temp2=512*8* BPB.SecPerClust+2*bitPerFat;
	
	DWORD SecPerFat=(DWORD)(temp1/temp2);
	
	if(temp1%temp2)
		SecPerFat+=1;
	
	Fat32_Section Fat32Section;
	memset(&Fat32Section,0,sizeof(Fat32_Section));
	if(Fat==FAT32)
	{
		Fat32Section.Fat32SecPerFat=SecPerFat;
		BPB.SecPerFat=0;

		Fat32Section.Fat32BackupBootSector=6;
		Fat32Section.Fat32Flag=0;
		Fat32Section.Fat32FSinfo=1;
		Fat32Section.Fat32Root1stCluster=2;
		Fat32Section.Fat32Version=0;

	}
	else
	{
		Fat32Section.Fat32SecPerFat=0;
		BPB.SecPerFat=(WORD)SecPerFat;
	}
	
	int m_cluster=(TotalSector-BPB.RsvdSecCnt-BPB.RootEntCnt*32/512-2*SecPerFat)/BPB.SecPerClust;
	int m_sector=m_cluster*BPB.SecPerClust;

	if((m_cluster<0) || ((m_sector*512)<*SelectFileLen) )
//	if(m_cluster<0) 
		return 0;

	BPB.BytesPerSec=512;
	BPB.FATSize=2;
	BPB.HiddenSector=0;
	BPB.MediaDescriptor=0xf8;
	BPB.NumHead=255;
	BPB.SecPerTrark=0x3f;

	Fat_ExBPB ExBPB;
	memset(&ExBPB,0,sizeof(Fat_ExBPB));
	
//	ExBPB.phyDrvNum		     =0x80;                                          
	ExBPB.Reserved		      =0;		                                           
	ExBPB.ExtendedBootSignature=0x29;	                          
	ExBPB.Volume               =0;   
	strcpy(ExBPB.VolumeLabel,"NO NAME");
	strcpy(ExBPB.SysID,m_SysID);
	

	
	char DBRBuf[512];
	memset(DBRBuf,0,512);
	char *DBRBufPointer;
	DBRBufPointer=DBRBuf;
	Fat_DBR dbr;
	dbr.JumpIns[0]=(char)0xeb;
	dbr.JumpIns[1]=(char)0x58;
	dbr.JumpIns[2]=(char)0x90;
	strcpy(dbr.OEMID,"MSWIN4.1");
	dbr.BPB=BPB;
	memcpy(DBRBuf,&dbr,sizeof(Fat_DBR));
	DBRBufPointer += sizeof(Fat_DBR);


	if(Fat==FAT32)
	{
		memcpy(DBRBufPointer,&Fat32Section,sizeof(Fat32_Section));
		
		DBRBufPointer +=sizeof(Fat32_Section);
	}
	
	memcpy(DBRBufPointer,&ExBPB,sizeof(Fat_ExBPB));
	
	*(WORD *)(DBRBuf + 0x1be) = 0xffff;
	*(WORD *)(DBRBuf + 0x1fe)=  0xaa55;

	memcpy(buf,DBRBuf,512);

	memset(buf+512,0,BPB.RsvdSecCnt*BPB.BytesPerSec-512);

	if(Fat==FAT32)
	{
		fat_boot_fsinfo *fsinfo;
		fsinfo=(fat_boot_fsinfo*)(buf+512);
		
		fsinfo->signature1=FAT_FSINFO_SIG1;
		fsinfo->signature2=FAT_FSINFO_SIG2;
		
		fsinfo->free_clusters=-1;
		fsinfo->next_cluster=2;
		buf[0x3fe]=0x55;	
		buf[0x3ff]=0xaa;

		if(Fat32Section.Fat32BackupBootSector)
		{
			memcpy(buf+Fat32Section.Fat32BackupBootSector*512,buf,512*2);
		}
	}

	WrBuffer(buf,BPB.RsvdSecCnt*BPB.BytesPerSec,Udiskoffset);


//fat
	BYTE FatBuffer[512];
	
	DWORD FatBufferSize;
	
	if(Fat==FAT32)
		FatBufferSize=Fat32Section.Fat32SecPerFat*BPB.BytesPerSec;
	else
		FatBufferSize=BPB.SecPerFat*BPB.BytesPerSec;
	
	memset(FatBuffer,0,512);
	
	if(Fat==FAT12)
	{
		*(DWORD *)(FatBuffer+ 0) =(DWORD)0x00fffff8;
	}
	else if(Fat==FAT16)
	{
		*(DWORD *)(FatBuffer+ 0) =(DWORD)0xfffffff8;
	}
	else
	{
		*(DWORD *)(FatBuffer + 0) =(DWORD)0x0ffffff8;
		*(DWORD *)(FatBuffer + 4) =(DWORD)0xffffffff;
		*(DWORD *)(FatBuffer + 8) =(DWORD)0x0fffffff;
	}

	int i;
	memset(buf,0,512);		

	DWORD Curflashoffset=Udiskoffset+BPB.RsvdSecCnt;
	WrBuffer(FatBuffer,512,Curflashoffset);	//first sec fat 
	Curflashoffset++;

	for(i=0;i<(FatBufferSize/512-1);i++)
	{
		WrBuffer(buf,512,Curflashoffset);
		Curflashoffset++;
	}
//fat2	
	WrBuffer(FatBuffer,512,Curflashoffset);	
	Curflashoffset++;

	for(i=0;i<(FatBufferSize/512-1);i++)
	{
		WrBuffer(buf,512,Curflashoffset);
		Curflashoffset++;
	}

//root	
	DWORD RootSize;
	
	if(Fat==FAT32)
		RootSize=BPB.SecPerClust*BPB.BytesPerSec;
	else
		RootSize=BPB.RootEntCnt*32; 
	
	for(i=0;i<(RootSize/512);i++)
	{
		WrBuffer(buf,512,Curflashoffset);
		Curflashoffset++;
	}
	
	return 1;
}


int	MakeLinuxMbr(unsigned int Logicalcap)
{
	int i;

	DWORD lba_start = PART_ALIGNMENT / SECTOR_SIZE;	
	DWORD Reserve_lba_start;
	for(i=0;i<LinuxUp.linuxNum;i++)
	{
		LinuxUp.linuxinfo[i].p_size=((LinuxUp.linuxinfo[i].f_malloclen + PART_ALIGNMENT - 1) / PART_ALIGNMENT * PART_ALIGNMENT) /SECTOR_SIZE;
		LinuxUp.linuxinfo[i].p_start=lba_start;
		lba_start+=LinuxUp.linuxinfo[i].p_size;
	}
	
	LONGLONG udisklen=(LONGLONG)Logicalcap*512-LinuxOffset-(LONGLONG)lba_start*512-LinuxUp.linuxReserveCap;
	if(LinuxUp.linuxFWBakSize>LinuxUp.linuxReserveCap) 
	{
		ERR("linuxFWBakSize larger than linuxReserveCap\n");
		return 1;
	}

	if(LinuxUp.linuxUdiskFlag)
	{
	//	LONGLONG udisklen=(LONGLONG)Logicalcap*512-LinuxOffset-lba_start*512;
		if(udisklen<0) return 1;

		strcpy(LinuxUp.linuxinfo[i].f_attrstr,"udisk");
		LinuxUp.linuxinfo[i].p_sysid=0xb;	//fat32
		LinuxUp.linuxinfo[i].p_size= (DWORD)((udisklen/ PART_ALIGNMENT * PART_ALIGNMENT) /SECTOR_SIZE);
		LinuxUp.linuxinfo[i].p_start=lba_start;
		LinuxUp.linuxinfo[i].f_len=0;
		LinuxUp.linuxNum++;
		lba_start+=LinuxUp.linuxinfo[i].p_size;
		i++;
	}
	else
	{
		if(udisklen<0)
		{
			if(0==strcasecmp(LinuxUp.linuxinfo[i-1].f_attrstr,"udisk"))
			{
				//自定义长度的udisk
				if(udisklen<0)
				{
					LinuxUp.linuxNum -=1;			//udisk 容量超过了flash剩余容量，按照剩余容量重新制作Udisk

					LinuxUp.linuxUdiskFlag=1;

					return MakeLinuxMbr(Logicalcap);
				}
			}
			else
			{
				return 1;							//error
			}

		}
	}

	if(LinuxUp.linuxReserveCap)
	{
		strcpy(LinuxUp.linuxinfo[i].f_attrstr,"reserve");
		LinuxUp.linuxinfo[i].p_sysid=0;
		LinuxUp.linuxinfo[i].p_size= (LinuxUp.linuxReserveCap  / PART_ALIGNMENT * PART_ALIGNMENT) /SECTOR_SIZE;
		Reserve_lba_start=Logicalcap-LinuxUp.linuxinfo[i].p_size-LinuxOffset/512;
		if(Reserve_lba_start>lba_start)
			LinuxUp.linuxinfo[i].p_start=Reserve_lba_start;
		else
			LinuxUp.linuxinfo[i].p_start=lba_start;
		LinuxUp.linuxinfo[i].f_len=LinuxUp.linuxFWBakSize;
		LinuxUp.linuxNum++;
	}
	mbr_conv(mbr, &LinuxUp.linuxinfo[0], LinuxUp.linuxNum);
	
	INFO("Linux Part\n\n");
	for(i=0;i<LinuxUp.linuxNum;i++)
	{
		INFO("%s:p_start=0x%x,p_size=0x%x\n",LinuxUp.linuxinfo[i].f_attrstr,LinuxUp.linuxinfo[i].p_start,LinuxUp.linuxinfo[i].p_size);
	}

	if(LinuxUp.linuxReserveCap)
		LinuxUp.linuxNum-=1;		//"reserve" fw bak

	return 0;
}

int CalculateDownloadLen(BYTE *firmwarebuf,PartUpdate_Info *partupdate_info_p)
{
	Sys_File *CurSysFile;
	int i;		
	int offset=FindSysFile("FIRM",firmwarebuf);//get offset after "FIRM"
	if(offset<0)
	{
		ERR("FIRM error\n\n\n");
		return 0;
	}
	CurSysFile=(Sys_File*)(firmwarebuf+offset);
	
	DWORD KernelLen=CurSysFile->Sys_para1;

	DownloadWholeLen=KernelLen*2;	//kernel and kernelbak

	for(i=0;i<LinuxUp.linuxNum;i++)
	{
		if(FindUpdateInfo(LinuxUp.linuxinfo[i].f_attrstr,partupdate_info_p))
			DownloadWholeLen+=LinuxUp.linuxinfo[i].f_len;
	}
#if 0
	if(LinuxUp.linuxFWBakSize &&(FindUpdateInfo("fwbak",partupdate_info_p)))
	{
		DownloadWholeLen+=LinuxUp.linuxFWBakSize;
	}
#endif

	INFO("DownloadWholeLen:0x%x\n\n\n",DownloadWholeLen);

	return 1;


}


void LinuxLoadFile(Sys_File *CurSysFile)
{
	int i;
	if((0==CurSysFile->Sys_para1) ||(0==CurSysFile->Sys_offset))
	{
		ERR("not valid linux firmware\n\n");
		return;
	}

	LinuxOffset = CurSysFile->Sys_len;
	LinuxUp.linuxNum=CurSysFile->Sys_para1;
	LinuxUp.linuxUdiskFlag=CurSysFile->Sys_para2;
	LinuxUp.linuxFileMode=CurSysFile->Sys_offset;	//f_mode=0,单独文件；f_mode=1，在固件中
	CurSysFile++;

	for(i=0;i<LinuxUp.linuxNum;i++)
	{
		strcpy(LinuxUp.linuxinfo[i].f_attrstr,CurSysFile->Sys_Flag);
		LinuxUp.linuxinfo[i].f_len=CurSysFile->Sys_len;
		LinuxUp.linuxinfo[i].p_sysid=(char)(CurSysFile->Sys_para2&0xff);

		if(CurSysFile->Sys_para2 & SectorMode)
		{
			LinuxUp.linuxinfo[i].f_malloclen=CurSysFile->Sys_para1*512;
		}
		else
		{
			LinuxUp.linuxinfo[i].f_malloclen=CurSysFile->Sys_para1;
		}
	
		if(CurSysFile->Sys_para2 & CompreddMode)
		{
			LinuxUp.linuxinfo[i].f_compressflag=1;
		}

		LinuxUp.linuxinfo[i].f_offset=CurSysFile->Sys_offset;
		
		CurSysFile++;
	}
	if(0==strcasecmp(CurSysFile->Sys_Flag,"reserve"))
	{
		LinuxUp.linuxReserveCap=CurSysFile->Sys_para1;
		if(CurSysFile->Sys_len)
			LinuxUp.linuxFWBakSize=WholeFirmwareLen;
	}
}

int WrBufferPartition(BYTE *WrBuf, DWORD len, DWORD flashoffset,DWORD *Finish,DWORD DownloadSize)
{
	DWORD Count=0;
	int	DownloadProgress;
	int FrameLen=16384;
	DWORD size=FrameLen;
	int remainlen=len;
	BYTE *WrBufp=WrBuf;

	INFO("WrBufferPartition:len=0x%x,flashoffset=0x%x\n",len,flashoffset);

	while(remainlen>0)
	{
		if(remainlen<FrameLen)
			size=remainlen;
		
		fun->nandflash_write(flashoffset+Count/512,WrBufp,size/512);

		remainlen       -= size;
		Count	       += size;
		WrBufp		+=size;
		*Finish 		+= size;

		DownloadProgress=((*Finish)>>9)*100/(DownloadSize>>9);

		fun->feed_status_common(DownloadProgress,S_RUNNING);

	}
	INFO("DownloadFinish:0x%x,DownloadSize:0x%x\n\n\n",*Finish,DownloadSize);

	return 1;
	
}
int WrBuffer(BYTE *WrBuf, DWORD len, DWORD flashoffset)
{
	fun->nandflash_write(flashoffset,WrBuf,len/512);
	return 1;
}

int WrPartition_Compress(DWORD fileoffset, DWORD filelen, DWORD flashoffset,DWORD *DownloadFinish,DWORD DownloadSize,BOOL Decrypt)
{

	BYTE Infobuf[512];
	Compress_Info *Info_data=(Compress_Info*)Infobuf;
	DWORD offset;
	unsigned char *UnzipData;

	INFO("WrPartition_Compress:fileoffset=0x%x,filelen=0x%x,flashoffset=0x%x\n",fileoffset,filelen,flashoffset);

	fun->firmware_seek(fileoffset);
	fun->firmware_read(Infobuf,512);

	offset=fileoffset+512;

	while(1)
	{
		if( (0==Info_data->ZipLen) ||(((BYTE*)Info_data)==(Infobuf+512)))
			break;

		UnzipData=fun->firmware_Uncompres(offset,Info_data->ZipLen,Info_data->UnzipLen,Info_data->ZipChecksum,Info_data->UnzipChecksum);

		if(NULL==UnzipData)
			return 0;

		WrBufferPartition(UnzipData, Info_data->UnzipLen,flashoffset,DownloadFinish,DownloadSize);
		flashoffset+=Info_data->UnzipLen/512;
		fun->Free_Unzipdata(UnzipData);

		offset+=(Info_data->ZipLen+511)/512*512;
		Info_data++;
	}

	return 1;
}




int WrPartition(DWORD fileoffset, DWORD filelen, DWORD flashoffset,DWORD *Finish,DWORD DownloadSize,BOOL Decrypt)
{
	DWORD Count=0;
	int	DownloadProgress;
	
	INFO("WrPartition:fileoffset=0x%x,filelen=0x%x,flashoffset=0x%x\n",fileoffset,filelen,flashoffset);

	int FrameLen=16384;
	
	DWORD size=FrameLen;
	int remainlen=filelen;

	fun->firmware_seek(fileoffset);
	
	while(remainlen>0)
	{
		
		if(remainlen<FrameLen)
			size=remainlen;
		
		if(0 == fun->firmware_read(FirmBuffer,size)){
			DownloadProgress=((*Finish)>>9)*100/(DownloadSize>>9);
			fun->feed_status_common(DownloadProgress,S_ERROR);
			return 0;
		}
		fun->delay_common(2);
		if(Decrypt)
		{
			;
		}
		fun->nandflash_write(flashoffset+Count/512,FirmBuffer,size/512);

		remainlen       -= size;
		Count	        += size;
		*Finish += size;

		//	DownloadProgress=(*Finish)*100/(DownloadSize);
	//@fish add 2011-01-11 ,int over flow ,
		DownloadProgress=((*Finish)>>9)*100/(DownloadSize>>9);

		fun->feed_status_common(DownloadProgress,S_RUNNING);


	}
	INFO("DownloadFinish:0x%x,DownloadSize:0x%x\n\n\n",*Finish,DownloadSize);

	return 1;
}
int Linuxkernel_process(BYTE *firmwarebuf,unsigned int Logicalcap,unsigned int old_mbroffset)
{
	int i;
	int offset;
	Sys_File *CurSysFile;
	BYTE LFIbuf[512];
	char *keybuf;
	unsigned char  *unzip_data=NULL;
		
	offset=FindSysFile("FIRM",firmwarebuf);
	if(offset<0){
		ERR("FIRM error\n\n\n");
		return 0;
	}
	CurSysFile=(Sys_File*)(firmwarebuf+offset);
	
	DWORD FirmwareLen=CurSysFile->Sys_para1;
	DWORD FirmwareOffset=CurSysFile->Sys_offset;
	DWORD scodecap=CurSysFile->Sys_len/512;
	DWORD ZipLen=CurSysFile->Sys_para2;
	DWORD ZipChecksum;
	DWORD UnZipChecksum;	
	INFO("len:0x%x,offset:0x%x,cap:0x%x,ziplen:0x%x\n\n",FirmwareLen,FirmwareOffset,scodecap,ZipLen);
	if(ZipLen)
	{
		ZipChecksum=*(DWORD*)((BYTE*)CurSysFile+8);
		UnZipChecksum=*(DWORD*)((BYTE*)CurSysFile+12);
	}

	if(ZipLen)
	{		
		INFO("###uncompress linux kernel###\n");
		unzip_data=fun->firmware_Uncompres(FirmwareOffset,ZipLen,FirmwareLen,ZipChecksum,UnZipChecksum);
		if(NULL==unzip_data)
			return 0;
		memcpy(LFIbuf,unzip_data,512);
	}
	else
	{
		fun->firmware_seek(FirmwareOffset);
		if(0 == fun->firmware_read(FirmBuffer,512))
		{
			fun->feed_status_common(0,S_ERROR);
			return 0;	
		}
		memcpy(LFIbuf,FirmBuffer,512);
	}
	

	LFIHead_t *LFIHeader=(LFIHead_t *)LFIbuf;
	LFIHeader->CapInfo.Logicalcap=Logicalcap;
	
	LFIHeader->CapInfo.Scodecap=scodecap;
	
	if( (scodecap*512*2) > LinuxOffset)
	{
		INFO( "kernel is %dK,reserved is %dK,no kernel backup\n",(scodecap*512)/1024,LinuxOffset/1024);
		LFIHeader->CapInfo.Scodebakcap=0;		//no kernel backup
	}
	else
		LFIHeader->CapInfo.Scodebakcap=scodecap;

	LFIHeader->CapInfo.Micap=LinuxOffset/512;	//LFI-0x90

	if((LFIHeader->CapInfo.Micap!=old_mbroffset) &&(old_mbroffset!=0))
	{
		INFO("remove HDCPKey,0x%x to 0x%x\n",old_mbroffset,LFIHeader->CapInfo.Micap);

		keybuf=kmalloc(1536, GFP_KERNEL);
		if(NULL==keybuf)
		{
			INFO("malloc keybuf error\n ");
			return 0;
		}
		fun->nandflash_read(old_mbroffset+1,keybuf,3);	//this is hdcp key

		fun->nandflash_write(LFIHeader->CapInfo.Micap+1,keybuf,3);

		kfree(keybuf);
	}
	

	//重新算checksum
	WORD *LFI_Header_point=(WORD*)LFIHeader;
	WORD headerchecksum=0;
	
	for(i=0;i<sizeof(LFIHead_t)/sizeof(WORD)-1;i++)
		headerchecksum=(WORD)(headerchecksum+LFI_Header_point[i]);
	
	LFIHeader->Headchecksum=headerchecksum;//前510byte校验和, WORD累加

	INFO("sending kernel\n");
	WrBuffer((BYTE *)LFIHeader,512,0);
	if(ZipLen)
		WrBufferPartition(unzip_data+512,FirmwareLen-512,1,&DownloadFinish,DownloadWholeLen);
	else 
	{
		if(0 ==WrPartition(FirmwareOffset+512,FirmwareLen-512,1,&DownloadFinish,DownloadWholeLen,DecryptFlag))
		return 0;
	}
	
	if(LFIHeader->CapInfo.Scodebakcap)
	{
		INFO("sending kernel backup\n");
		WrBuffer((BYTE *)LFIHeader,512,scodecap);
		if(ZipLen)
			WrBufferPartition(unzip_data+512,FirmwareLen-512,1+scodecap,&DownloadFinish,DownloadWholeLen);
		else 
		{
			if(0==WrPartition(FirmwareOffset+512,FirmwareLen-512,1+scodecap,&DownloadFinish,DownloadWholeLen,DecryptFlag))
			return 0;
		}
	}

	if(unzip_data)
		fun->Free_Unzipdata(unzip_data);
	return 1;
}

int	Linuxfirm_process(unsigned int Logicalcap,PartUpdate_Info *partupdate_info_p)
{
	int i;
	int result;
	DWORD len;
	
	for(i=0;i<(LinuxUp.linuxNum-LinuxUp.linuxUdiskFlag);i++)
	{	 
		 if(FindUpdateInfo(LinuxUp.linuxinfo[i].f_attrstr,partupdate_info_p))
		 {
		 
			 INFO("sending [%d]%s\n",i,LinuxUp.linuxinfo[i].f_attrstr);
		 
			 len=LinuxUp.linuxinfo[i].f_len;
		 
			if(LinuxUp.linuxinfo[i].f_compressflag)
			result=WrPartition_Compress(
						LinuxUp.linuxinfo[i].f_offset,
						len,
						LinuxOffset/512+LinuxUp.linuxinfo[i].p_start,
						&DownloadFinish,DownloadWholeLen,DecryptFlag);
			else
			 result=WrPartition(
						 LinuxUp.linuxinfo[i].f_offset,
						 len,
						 LinuxOffset/512+LinuxUp.linuxinfo[i].p_start,
						 &DownloadFinish,DownloadWholeLen,DecryptFlag);
			 if(0 == result)
			 	return 0;
		 }
		 else
		 {
			 INFO("skip [%d]%s\n",i,LinuxUp.linuxinfo[i].f_attrstr);
		}
	}
#if 1
	//Udisk
	if(LinuxUp.linuxUdiskFlag)
	{
		DWORD nouse=0;
		if(FindUpdateInfo("udisk",partupdate_info_p))
		{	
			for(i=0;i<LinuxMax;i++)
			{
				if(0==strcasecmp(LinuxUp.linuxinfo[i].f_attrstr,"udisk"))
					break;
			}
			if(i==LinuxMax)
			{
				ERR("not find udisk\n");
				return 0;
			}	
			INFO("Formating Udisk\n");	

			DWORD UdiskSec=(DWORD)LinuxUp.linuxinfo[i].p_size;
			result=UdiskFormat(FirmBuffer,UdiskSec,&nouse,LinuxOffset/512+LinuxUp.linuxinfo[i].p_start);
			if(0==result)
			{
				ERR("UdiskFormat error\n");
				return 0;
			}
		}
		else
		{
			INFO("skip Format Udisk\n");
		}
	}
#endif
#if 0
	//reserve,ota not process  
	if(LinuxUp.linuxFWBakSize)
	{
	
		if(FindUpdateInfo("fwbak",partupdate_info_p))
		{
			INFO("sending fwbak\n");
	
			DecryptFlag=0;

			for(i=0;i<LinuxMax;i++)
			{
				if(0==strcasecmp(LinuxUp.linuxinfo[i].f_attrstr,"reserve"))
					break;
			}	
			len=LinuxUp.linuxinfo[i].f_len;	
				result=WrPartition(
						LinuxUp.linuxinfo[i].f_offset,
						len,
						LinuxOffset/512+LinuxUp.linuxinfo[i].p_start,
						&DownloadFinish,DownloadWholeLen,DecryptFlag);
		}
		else
		{
			INFO("skip fwbak\n");
		}
	}
#endif

	return 1;
}

int Linux_Update(upgrade_func *func,unsigned int Logicalcap,unsigned int old_mbroffset,unsigned char *buffer,struct Flash_Partiton * linux_part,PartUpdate_Info *partupdate_info_p)
{
	int offset;
	char newmbr[512];
	char oldmbr[512];
	
	int fwback_space=0;

	fun=func;
	FirmBuffer=buffer;
	DownloadFinish=0;
	
	INFO("Logicalcap is 0x%x\n",Logicalcap);

//step 1
	fun->firmware_seek(0);
	fun->firmware_read((char *)FirmBuffer,FWIMG_Head_Max * sizeof(Sys_File));
	 if(0==MakeNewMbr(FirmBuffer, Logicalcap,newmbr,&fwback_space))
	 {
 		return 0;
	 }
	
//step 2,no need when recover
	fun->nandflash_read(old_mbroffset,oldmbr,1);	//read old mbr
	INFO("old mbr Offset=0x%x\n",old_mbroffset);

//step 3
	WrBuffer((unsigned char *)newmbr,512,LinuxOffset/512);	//write new mbr to nand
	INFO("new mbr Offset=0x%x\n",LinuxOffset/512);

//step 4,no need when recover
	Move_NeedSaved_LinuxPart(partupdate_info_p,newmbr,oldmbr,old_mbroffset);

//step 5
	CalculateDownloadLen(FirmBuffer,partupdate_info_p);

//step 6
	if(0==Linuxkernel_process(FirmBuffer,Logicalcap,old_mbroffset))
	 {
	 	return 0;
	 }
//step 7
	if(0==Linuxfirm_process(Logicalcap,partupdate_info_p))
	 {
	 	return 0;
	 }
	fun->feed_status_common(100,S_RUNNING);	

	return 1;
}

int MakeNewMbr(BYTE *FwHeader,unsigned int Logicalcap,char *newmbr,int * fw_back_space)
{
	int offset;
	Sys_File *CurSysFile;
	mbr=newmbr;

	memset(&LinuxUp,0,sizeof(LinuxUpdateInfo));

	offset=FindSysFile("CHECKSUM",FwHeader);//查看是否含有checksum信息        
	if(offset<=0)
		return 0;
	
	CurSysFile=(Sys_File*)(FwHeader+offset);
	DecryptFlag=CurSysFile->Sys_para1;
	WholeFirmwareLen= CurSysFile->Sys_offset+CurSysFile->Sys_len;

	offset=FindSysFile("LINUX",FwHeader);
	if(offset<=0)
		return 0;

	CurSysFile=(Sys_File*)(FwHeader+offset);
	LinuxLoadFile(CurSysFile);

	offset=FindSysFile("FIRM",FwHeader);
	if(offset<=0)
		return 0;

	CurSysFile=(Sys_File*)(FwHeader+offset);

	if(CurSysFile->Sys_len>LinuxOffset)
	{
		INFO("linux kernel is %dK,larger than reserved %dK\n",CurSysFile->Sys_len/1024,LinuxOffset/1024);

		LinuxOffset=(CurSysFile->Sys_len+0xFFFFF)/0x100000*0x100000;	//1M对齐

		//return 0;
	}
	if(MakeLinuxMbr(Logicalcap))
		return 0;

	if(LinuxUp.linuxReserveCap)
	{
		
		struct priv_mbr *mbrp = (struct priv_mbr *)newmbr;
		struct priv_ptr *ptr = mbrp->parts;
		ptr+=LinuxUp.linuxNum;	//this is reserved fw
		*fw_back_space=ptr->part_off+LinuxOffset/512;
		 INFO("fwbak pos is 0x%x sector\n",*fw_back_space);
	}
	else
	{
		*fw_back_space=-1;
		 ERR("no fwbak in fw\n");
	}
	return 1;
}

int GetInfo_InNewMbr(char *attrst,char *mbrdata,uint32_t *part_off,uint32_t *part_size)
{
	char attrshort[6];
	snprintf(attrshort, 6, "%s", attrst);

	struct priv_mbr *mbrp = (struct priv_mbr *)mbrdata;
	struct priv_ptr *ptr = mbrp->parts;
	int count=0;
	while(ptr->part_size)
	{
		if(0==strcasecmp(attrshort,ptr->id))
		{
			*part_off=ptr->part_off;
			*part_size=ptr->part_size;
			return count;
		}
		ptr++;
		count++;
	}
	return -1;
}
int GetInfo_InOldMbr(char *attrst,char *mbrdata,int partNum,uint32_t *part_off,uint32_t *part_size)
{
	char attrshort[6];
	snprintf(attrshort, 6, "%s", attrst);
	
	struct priv_mbr *mbrp = (struct priv_mbr *)mbrdata;
	struct priv_ptr *ptr = mbrp->parts;

	if(0==strcasecmp("PT000",ptr->id))
	{
		//this old version
		ptr+=partNum;
		*part_off=ptr->part_off;
		*part_size=ptr->part_size;
		if((0==*part_off) ||(0==*part_size))
			return -1;
		else
			return partNum; 
	}
	else
	{
		//new version
		int count=0;

		while(ptr->part_size)
		{
			if(0==strcasecmp(attrshort,ptr->id))
			{
				*part_off=ptr->part_off;
				*part_size=ptr->part_size;
				return count;
			}
			ptr++;
			count++;
		}
		return -1;
	}
}

void MoveFlashData(unsigned int dstbla,unsigned int srcbla,unsigned int lbasize)
{
	int OnceLbaSize=64;
	char *buf;
	char *readbuf;
	int count;

	buf=(char*)kmalloc(OnceLbaSize*512, GFP_KERNEL);
	if(NULL==buf)
	{
		ERR("malloc buf error\n");
		return;
	}
	readbuf=(char*)kmalloc(OnceLbaSize*512, GFP_KERNEL);
	if(NULL==readbuf)
	{
		kfree(buf);
		ERR("malloc buf error\n");
		return;
	}

	int remainlba=(int)lbasize;

	while(remainlba>0)
	{
		if(remainlba<OnceLbaSize)
			OnceLbaSize=remainlba;

		fun->nandflash_read(srcbla,buf,OnceLbaSize);

		count=0;
		while(count<2)
		{
			count++;

			fun->nandflash_write(dstbla,buf,OnceLbaSize);

			fun->nandflash_read(dstbla,readbuf,OnceLbaSize);

			if(byte_compare((INT32S*)buf,(INT32S*)readbuf,OnceLbaSize))
			{
				break;
			}
		}
		
		remainlba-=OnceLbaSize;
		dstbla+=OnceLbaSize;
		srcbla+=OnceLbaSize;
	}
	kfree(buf);
	kfree(readbuf);

}

void Move_NeedSaved_LinuxPart(PartUpdate_Info *partupdate_info_p,char *newmbr,char *oldmbr,unsigned int old_mbroffset)
{

	int PartNum;
	unsigned int new_part_off,new_part_size;
	unsigned int old_part_off,old_part_size;
	int 	count=0;

	INFO("Into Func() Move_NeedSaved_LinuxPart\n");
	Print_partupdate_inf(partupdate_info_p);


#if 1
	struct priv_mbr *mbrp = (struct priv_mbr *)newmbr;
	struct priv_ptr *ptr = mbrp->parts;
	
	INFO("New mbr info:\n");
	while(ptr->part_size)
	{
		INFO("%s:0x%x,0x%x\n",ptr->id,ptr->part_off,ptr->part_size);
		ptr++;
	}
	mbrp = (struct priv_mbr *)oldmbr;
	ptr = mbrp->parts;
	INFO("old mbr info\n");
	while(ptr->part_size)
	{
		INFO("%s:0x%x,0x%x\n",ptr->id,ptr->part_off,ptr->part_size);
		ptr++;
	}
#endif

	while(strlen(partupdate_info_p->partupdate_attrstr)&&(count<LinuxMax)) 
	{
		count++;
		if(partupdate_info_p->partupdate_flag)
		{
			partupdate_info_p++;
			continue;					//ini is TRUE
		}

		INFO("Check %s\n",partupdate_info_p->partupdate_attrstr);
		
		PartNum=GetInfo_InNewMbr(partupdate_info_p->partupdate_attrstr,newmbr,&new_part_off,&new_part_size);
		if(PartNum<0)
		{
			INFO("no %s in new mbr\n",partupdate_info_p->partupdate_attrstr);
			partupdate_info_p++;
			continue;					//this part no in NewFw
		}
		PartNum=GetInfo_InOldMbr(partupdate_info_p->partupdate_attrstr,oldmbr,PartNum,&old_part_off,&old_part_size);
		if(PartNum<0)
		{
			INFO("no %s in old mbr\n",partupdate_info_p->partupdate_attrstr);
			partupdate_info_p++;
			continue;					//this part no in NewFw
		}

		new_part_off+=LinuxOffset/512;
		old_part_off+=old_mbroffset;
		
		if((old_part_off!=new_part_off) && (old_part_size==new_part_size))
		{
			INFO("%s:Need move from 0x%x to 0x%x,size=0x%x\n",partupdate_info_p->partupdate_attrstr,old_part_off,new_part_off,new_part_size);

			MoveFlashData(new_part_off,old_part_off,new_part_size);
		}
		else
		{
			INFO("%s:No Need move,old_part_off=0x%x,new_part_off=0x%x,size=0x%x\n",partupdate_info_p->partupdate_attrstr,old_part_off,new_part_off,new_part_size);
		}
		
		partupdate_info_p++;
	}
	INFO("End Func() Move_NeedSaved_LinuxPart\n");

}

void Print_partupdate_inf(PartUpdate_Info *partupdate_info_p)
{
	int 	count=0;

	if(0==strlen(partupdate_info_p->partupdate_attrstr))
	{
		INFO("no partupdate Info\n");
		return;
	}
	
	INFO("Partupdate Info:\n");
	while(strlen(partupdate_info_p->partupdate_attrstr)&&(count<LinuxMax)) 
	{
		count++;
		if(partupdate_info_p->partupdate_flag)
			INFO("%s=TRUE\n",partupdate_info_p->partupdate_attrstr);
		else
			INFO("%s=FALSE\n",partupdate_info_p->partupdate_attrstr);
		
		partupdate_info_p++;
	}
}

