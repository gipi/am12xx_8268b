/**
@file: main.c

@abstract: main entry source file.

@notice: Copyright (c), 2010-2015 Actions-Mirco Communications, Inc.
 *
 *  This program is develop for Actions-Mirco Boot Display;
 *  include code recover, upgrade, welcome logo	 
 *
 *  The initial developer of the original code is scopengl
 *
 *  scopengl@gmail.com
 *
 */
 
#include "syscfg.h"
#include "am_types.h"
#include "sysinfo.h"
#include "ota_fw.h"
#include "boot_display_api.h"
#include "alloc.h"
#include "welcome.h"

#define CL_SIZE       256
#define	SECTOR_RW_ONCE          64
#define PageNumPerBlock         64
#define SECT_SIZE               0x200
#define MAX__PARTS   16 
#define NAME_SIZE_MAX  10
#define PART_Str_SIZE   128 
struct Flash_Partiton {
    char      partName[NAME_SIZE_MAX];
    INT32U    part_off;   /* LBA of first sector in the partition */
    INT32U    part_size;  /* number of blocks in partition */
}__attribute__ ((packed));//,aligned(4)
#define FW_BACK_PAR_NAME  "reserve"

#define N_PHY_ERASE      0x00
#define N_PHY_READ       0x01
#define N_PHY_WRITE      0x02
#define N_LOG_INIT       0x03
#define N_LOG_EXIT       0x04
#define N_LOG_READ       0x05
#define N_LOG_WRITE      0x06
#define N_LOG_UPDATE     0x07 

#define MULTIBOOTFLAG "MultiSectorBoot"
#define OTADIRTYSIGN  0x55aaffff
#define OTACLEARSIGN  0x55aa5aa5

#define NAND_SECTOR_SIZE   (512)
#ifdef  UPGRADE
#undef  UPGRADE
#endif
#define UPGRADE      1
#ifdef  NOTUPGRADE
#undef  NOTUPGRADE
#endif
#define NOTUPGRADE   0   
#define UDISK_UPDATAFLAG   NOTUPGRADE   //CONTORL UDISK PARTITION UPGRADE OR NOT
#define VRAM_UPDATAFLAG    UPGRADE		//CONTORL VRAM PARTITION UPGRADE OR NOT

typedef struct 
{
	char 			MultiSecFlag[16];		// "MultiSectorBoot"
	unsigned int		MbrecNum;				// Mbrec的份数,最多支持4份
	unsigned int 		MbrecEncryptFlag;		// Mbrec 是否加密：1加密，0未加密
	unsigned int 		MbrecRunAddr;			// Mbrec 在Sram中的运行地址
	unsigned int 		MbrecDecryptAddr;		//解密Mbrec的存放地址，可与MbrecRunAddr同
	unsigned int		Mbrec0_Block;			//第0份 Mbrec 所在Block
	unsigned int 		Mbrec0_StartSector;		//第0份 Mbrec 起始扇区
	unsigned int 		Mbrec0_SectorNum;		//第0份 Mbrec 所用扇区数
	unsigned int 		Mbrec0_Reserve;   
	unsigned int 		Mbrec1_Block;			//第1份 Mbrec 所在Block
	unsigned int 		Mbrec1_StartSector; 	//第1份 Mbrec 起始扇区
	unsigned int 		Mbrec1_SectorNum; 		//第1份 Mbrec 所用扇区数
	unsigned int 		Mbrec1_Reserve;   
	unsigned int 		Mbrec2_Block;  			//第2份 Mbrec 所在Block
	unsigned int 		Mbrec2_StartSector; 	//第2份 Mbrec 起始扇区
	unsigned int 		Mbrec2_SectorNum;  		//第2份 Mbrec 所用扇区数
	unsigned int 		Mbrec2_Reserve;  
	unsigned int 		Mbrec3_Block;  			//第3份 Mbrec 所在Block
	unsigned int 		Mbrec3_StartSector;  	//第3份 Mbrec 起始扇区
	unsigned int 		Mbrec3_SectorNum;  		//第3份 Mbrec 所用扇区数
	unsigned int 		Mbrec3_Reserve; 
	unsigned int 		PAGE_CNT_PER_PHY_BLK; //tl
	char 			Reserve0[380];	//tl Reserve0[384];  
	unsigned char 	MbrecDecryptKey[16];	//
	unsigned char    	ChipIDFlag;				//是否烧入ChipID:1 已烧入,0未烧入ChipID或Mbrec未加密
	unsigned char    	ChipIDTransLen;			//调用ChipID_To_MbrecDecryptKey所使用的ChipID长度  
	unsigned char    	ChipIDStartByte;		//调用ChipID_To_MbrecDecryptKey用到的ChipID的起始byte    
	unsigned char    	Reserve1;                              
	char 			BootCheckFlag[8];		//"ActBrm",0xaa,0x55
	unsigned int 		BootCheckSum;			//前508个byte的32位累加和+0x1234
}__attribute__ ((packed)) MuiltSectorBoot;

struct priv_ptr {
    unsigned char  system_id;
    char     id[6];
    unsigned char  status;
    unsigned int part_off;   /* LBA of first sector in the partition */
    unsigned int part_size;  /* number of blocks in partition */
} __attribute__((packed));

#define MAX_PRIV_PARTS    16

struct priv_mbr {
	unsigned char          data[254];
	struct priv_ptr  parts[MAX_PRIV_PARTS];
	unsigned short         signature;
};

typedef struct 
{
	char      partupdate_attrstr[8];
	int	     partupdate_flag;
}PartUpdate_Info;

typedef struct 
{
	DWORD ZipLen;
	DWORD UnzipLen;
	DWORD ZipChecksum;
	DWORD UnzipChecksum;
}Compress_Info;

struct Flash_Partiton  Fparts[MAX__PARTS],* linux_mbr=NULL;
static INT32U kernel_offset = 0x10000;
static INT32U fw_back_addr = 0,recover_read_lba=0,write_lba=0,fw_back_space=0;
static unsigned int BootDiskType;		//表示系统当前的flash类型
static int brec_sector_num=32;
CapInfo_t old_capinfo;	//存放原来image的信?
PartUpdate_Info  PartUpdate_info[LinuxMax];
char   partInfo[128];
int (*flash_entry)(unsigned char, unsigned short, unsigned short,int,unsigned char*);
unsigned char *FlashRdBuffer=NULL,*zipmem = NULL,*unzipmem = NULL;
int compress_flag = -1;
extern 	INT32S mem_ungzip(unsigned char *dest, INT32S *destLen, const unsigned char *source, INT32S sourceLen);
struct	am7x_chipinfo am7x_chipinfo;


static int strcasecmp(const char * cs,const char * ct)
{
    register signed char __res;
    /*
    if(ct==NULL)
        return -1;
    if(cs==NULL)
        return 1;
*/
//OSprintf("string1 :%s\nstring2:%s\n",cs,ct);
    while (1) {
        if (!*cs && !*ct)
            return 0;
        else if (!*cs)
            return -1;  
        else if (!*ct)
            return 1;  
        __res = *cs - *ct;
        if (__res != 0 )
        {
            if((*cs>='A')&&(*cs<='Z'))
                __res += 32;   //'a'-'A'=32
            else
                __res-=32;
            if (__res != 0 )
                break;
        }
        cs++;
        ct++;
    }
 return __res;
}
/*
void Str_printf( const INT8U*  pad, const INT8U* pData, INT16U inLen)
{
	INT16U iLoop;   
	printf("%s",pad);//

	for( iLoop=0; iLoop< inLen; iLoop++)
	{
		printf("%02x ", pData[iLoop]);
		if( 0== ((iLoop+1) &0x0f) )
		{
			printf("  %4d\n",iLoop);
		}
	}
	printf("\n");
}
*/
void delay(unsigned int delay_time){
	unsigned int i;
	unsigned int tmp = 0;
	while(delay_time--){
		for(i=0;i<500;i++);
			tmp = tmp +2;
	}
}

struct status{
	unsigned char prg;
	unsigned char state;
}recoversta;

#define SD_BOOTARG_TXT  "BOOTARG TXT"
#define BOOTARG_TXT     "bootarg.txt"
#define LK_ARGS_ADDR  0x80900000
#define TAG_RDSTART   "rd_start="
static char linux_kern_cmdline[4096];
static INT32U rd_start = 0x80288000;
static INT32S Check_All0xFF(INT8U *buf,INT32U lenth){
	INT32U i;
	for(i=0;i<lenth;i++){
		if(0xff != *buf)
			return 1;
		buf++;
	}
	return 0;
}

static void parse_boot_args(INT32U lba, INT32U size)
{
	INT32U i, sects = (size + NAND_SECTOR_SIZE - 1) / NAND_SECTOR_SIZE;
	char *p, *end;
	if(size >= sizeof(linux_kern_cmdline)) {
		printf("size of bootarg.txt is too big!\n");
		return;
	}
	for(i=0; i<sects; i++)
	{
		(*flash_entry)(N_LOG_READ, 0,1, lba+i, FlashRdBuffer);
		memcpy(linux_kern_cmdline + i * NAND_SECTOR_SIZE,FlashRdBuffer,512);			
	}
	if(linux_kern_cmdline[0] ==0x00 && linux_kern_cmdline[1]==0x00 && linux_kern_cmdline[2]==0x00)
	{
		//printf("run Buf:%x%x\n",&linux_kern_cmdline[0]);
		//Str_printf("First \n", linux_kern_cmdline, 256);
		for(i=0; i<sects; i++)
			(*flash_entry)(N_LOG_READ, 0,1, lba+i, linux_kern_cmdline + i * NAND_SECTOR_SIZE);
		
	}
	/*
	if(linux_kern_cmdline[0] ==0x00 && linux_kern_cmdline[1]==0x00 && linux_kern_cmdline[2]==0x00)
	{
		Str_printf("seccond\n", linux_kern_cmdline, 256);	
	}
	*/
	printf("Kernel command line: %s\n", linux_kern_cmdline);
	p = strstr(linux_kern_cmdline, TAG_RDSTART);
	if(p != NULL) {
		printf("A1: %s\n", p);
		rd_start = simple_strtoul(p+sizeof(TAG_RDSTART)-1, &end, 16);
		printf("A2: 0x%08x\n", rd_start);
	}
	p = linux_kern_cmdline + strlen(linux_kern_cmdline) - 1;
	while(p >= linux_kern_cmdline) {
		if(*p != '\r' && *p != '\n')
			break;
		p--;
	}
	* ++ p = ' ';
	* ++ p = '\0';
}

int load_bootarg(void)
{
	INT32S 	i,j,FindFileNum,FieldID=-1;
	INT32U	FileOffset, FileLength;
	INT32U file_off=0;
	INT32U file_size=0;

	FindFileNum=0;
	
	for(i=1; (i<=(240*sizeof(SD_DIR))/SECT_SIZE); i++) // sizeof(SD_DIR)=32, max SD_DIR entries are 240.
	{	
		(*flash_entry)(N_LOG_READ, 0, 1, i,FlashRdBuffer);
		SD_DIR * sd_dir = (SD_DIR *) FlashRdBuffer;

		for(j=0;(j<SECT_SIZE/sizeof(SD_DIR)); j++, sd_dir++)
		{	
			if( strcmp(sd_dir->fname, SD_BOOTARG_TXT) == 0 ) 
			{
				FieldID = 1;
				file_off  =  sd_dir->offset;
				file_size  = sd_dir->size;
				printf("%s\n", sd_dir->fname);
				goto start_loading;
			}
			else if( *(volatile unsigned long *)(sd_dir->fname) == 0 ) {
				printf("No any more files\n");
				goto start_loading;
			}
		}
	}
start_loading:
	if(FieldID == 1) {
		parse_boot_args( file_off,file_size);
	}
	return FieldID;
}

INT32U get_bootmedia_type(void)
{
    INT32U nand_type;
	nand_type = '8' * 0x1000000 + '4' * 0x10000 + '6' * 0x100 + 'F';
	return nand_type;
}

static inline unsigned long translate_block(unsigned long block)
{
	return block + kernel_offset;
}

char * get_partion(void){
	char cmdline[CL_SIZE], *ptr;
	int iLoop;
	strcpy(cmdline, linux_kern_cmdline);
	ptr = strstr(cmdline, "AM7X_PARTS");	
	strcpy(partInfo,ptr);
	for(iLoop=0x00;iLoop<128;iLoop++){
		if(0x20==partInfo[iLoop] ){
			partInfo[iLoop] = '\0';
			break;
		}		   
	}
	return (char *)&(partInfo[0]);
}

INT32U fill_partion_tbl(struct Flash_Partiton *Fparts){
	int iLoop,jLoop,PartNo;
    char ch,* part_Info;
    part_Info=get_partion();
	if(memcmp(partInfo,"AM7X_PARTS=",11))
    {
        return 0x00;    
    }
    jLoop =0x00;
    iLoop = 11;
    ch = part_Info[NAME_SIZE_MAX];
    PartNo =0x00;
    while(iLoop<PART_Str_SIZE)
    {
        ch = part_Info[iLoop];
        if(ch ==',')
        {      
            if(jLoop>=NAME_SIZE_MAX)
            {
                jLoop= NAME_SIZE_MAX-1;
            }
            Fparts[PartNo].partName[jLoop] ='\0';
            PartNo++;
            jLoop =0x00;                       
        }
        else if (ch==0x00)
        {     
            if(jLoop>=NAME_SIZE_MAX)
            {
                jLoop= NAME_SIZE_MAX-1;
            }
            Fparts[PartNo].partName[jLoop] ='\0';			
			PartNo++;
            break;
        }
        else{
            Fparts[PartNo].partName[jLoop++] =ch;
        }
        iLoop++;
    }
	return PartNo;
}

static int find_newfw_addr(unsigned int *lba){
	int	ret= -1,partCnt;
	unsigned char  *TmpBuf=NULL,iLoop;
	partCnt=fill_partion_tbl(Fparts); 
	TmpBuf=FlashRdBuffer;
	memset(TmpBuf,0,512);	
	(*flash_entry)(N_LOG_READ,0, 1,translate_block(0x00),TmpBuf);
    for(iLoop =0x00;iLoop<partCnt;iLoop++)
    {              
        Fparts[iLoop].part_off =*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+6))+*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+7)) * 0x100\
								+*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+8))*0x10000 + *((unsigned char *)(TmpBuf+256+16*(0+iLoop)+9)) * 0x1000000;
        Fparts[iLoop].part_size =*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+10))+*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+11))*0x100\
								+*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+12))* 0x10000 +*((unsigned char *)(TmpBuf+256+16*(0+iLoop)+13))* 0x1000000;
        printf("NO:%d,%s offset:%x,Cap:%x %dMB\n",iLoop,Fparts[iLoop].partName,Fparts[iLoop].part_off,
                Fparts[iLoop].part_size,  Fparts[iLoop].part_size/2048);
			strcpy(PartUpdate_info[iLoop].partupdate_attrstr,Fparts[iLoop].partName);
			if(strcmp("udisk",Fparts[iLoop].partName) == 0){
				PartUpdate_info[iLoop].partupdate_flag = UDISK_UPDATAFLAG;
			}
			else if(strcmp("vram",Fparts[iLoop].partName) == 0){
				PartUpdate_info[iLoop].partupdate_flag = VRAM_UPDATAFLAG;
			}
			else{
				PartUpdate_info[iLoop].partupdate_flag = UPGRADE;
			}
			if(strcmp(FW_BACK_PAR_NAME,Fparts[iLoop].partName) == 0){	
				if((iLoop > 0)&&(Fparts[iLoop].part_off >= (Fparts[iLoop-1].part_off + Fparts[iLoop-1].part_size))){
					printf("find back fm success! offset =0x%x\n",Fparts[iLoop].part_off);
					*lba= translate_block(Fparts[iLoop].part_off);
					fw_back_space = Fparts[iLoop].part_size;
					printf("fw_back address is 0x%x\n",*lba);
					linux_mbr= &Fparts[iLoop];
					ret = 0;
				}
				else{
					printf("invalid firmware backup address!\n");				
				}				
				break;
			}
    } 
	return ret;
}

INT32U get_kernelcap(void){
	INT32U kernelcap;	
	(*flash_entry)(N_LOG_READ,0,1,0,FlashRdBuffer);
	kernelcap= *(INT32U*)(FlashRdBuffer+0x90);
	if(0x00 == kernelcap)
		kernelcap = 0x10000;
	printf("kernelcap is 0x%x\n",kernelcap);
	return kernelcap;
}

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

void fillstatus(unsigned char prg,unsigned char state){
	recoversta.prg = prg;
	recoversta.state = state;
}

int firmware_read(char *buf,unsigned int len)
{
	unsigned int iloop,sec=len/512;
	
	(*flash_entry)(N_LOG_READ, 0,sec,recover_read_lba+iloop,(unsigned char *)((unsigned int)buf + iloop*512));
	recover_read_lba += (len/512);
	return 1;
}

int firmware_seek(unsigned int offset)
{
	printf("firmware_seek to: 0x%x\n", offset);
	recover_read_lba = (offset/512) + fw_back_addr;
	return 1;
}

unsigned char *firmware_Uncompres(unsigned int offset,unsigned int ZipLen,unsigned int UnzipLen,unsigned int ZipChecksum,unsigned int UnzipChecksum)
{
	int ret;
	unsigned char *ZipData;
	unsigned char *UnzipData;
	unsigned int ZipLenAlign=(ZipLen+511)/512*512;
	unsigned int Curchecksum;
	ZipData = (unsigned char *)unzipmem;
	if(NULL==ZipData)
		return NULL;
	UnzipData = (unsigned char *)zipmem;
	if(NULL==UnzipData)
	{
		return NULL;
	}	
	firmware_seek(offset);
	firmware_read(ZipData,ZipLenAlign);
	Curchecksum = cal_checksum(ZipData,ZipLenAlign);;
	if(ZipChecksum != Curchecksum){
		printf("zip file checksum error,current :0x%x,zip :0x%x\n",Curchecksum,ZipChecksum);
		return NULL;
	}	
	ret= mem_ungzip(UnzipData,&UnzipLen, ZipData, ZipLen);	
	if(ret)
	{
		printf("uncompress file error,offset :0x%x,len :0x%x,ret: %d,unziplen :0x%x\n",offset,ZipLen,ret,UnzipLen);
		return NULL;
	}	
	Curchecksum = cal_checksum(UnzipData,UnzipLen);
	if(UnzipChecksum != Curchecksum){
		printf("unzip file checksum error,current :0x%x,unzip :0x%x,UnzipLen :0x%x\n",Curchecksum,UnzipChecksum,UnzipLen);
		return NULL;
	}
	return UnzipData;
}

void Free_Unzipdata(unsigned char *Unzipdata)
{
	//free(Unzipdata);
	return ;
}

int nandflash_read(int lbaoffset,unsigned char *buf,unsigned int sec)
{	
	unsigned int iloop;
	int ret;
	ret = (*flash_entry)(N_LOG_READ,0,sec,lbaoffset,(unsigned char *)(buf));
	if(ret!= 0){
		printf("read firmware error! lba=0x%x,ret=%d\n",lbaoffset,ret);
		return 0;	
	}
	return 1;
}

int nandflash_write(int lbaoffset,unsigned char *buf,unsigned int sec)
{
	unsigned int iloop;	
	int ret;
	ret = (*flash_entry)(N_LOG_WRITE,0,sec,lbaoffset,(unsigned char *)((unsigned int)buf));
	if(ret!= 0){
		printf("write firmware error! lba=0x%x,ret=%d\n",lbaoffset,ret);
		return 0;
	}
	return 1;
}

void fillstatus_common(int prg, Fwu_state_t state)
{
	fillstatus(prg, state);
}

unsigned int Firm_ChechsumFlow(void)
{
    int offset,result,ret=0;
    printf("Checking firmware ...\n");	
	(*flash_entry)(N_LOG_READ, 0,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,recover_read_lba,FlashRdBuffer);
	offset=FindSysFile("CHECKSUM",FlashRdBuffer);//查看是否含有checksum信息
	if(offset>0)
	{
	    Sys_File *CurSysFile=(Sys_File*)(FlashRdBuffer+offset);
	    unsigned int ReadCheckSum=CurSysFile->Sys_para2;	//固件中的checksum

	    unsigned int CalcuCheckSum=0;
	    int remainlen=CurSysFile->Sys_len;		//需计算的固件长度		
	    int size = SECTOR_RW_ONCE*512;		//可增大或减小         
	    unsigned char *Buf;

	    Buf = FlashRdBuffer;
	    printf("Sys_Flag:%s,\n",CurSysFile->Sys_Flag);
	    printf("Sys_offset:0x%x,\n",CurSysFile->Sys_offset);
	    printf("Sys_len:0x%x, %d KB\n",CurSysFile->Sys_len,(CurSysFile->Sys_len)/1024);
	    printf("Sys_para1:0x%x,\n",CurSysFile->Sys_para1);
	    printf("Sys_para2:0x%x,\n",CurSysFile->Sys_para2);					
		recover_read_lba = fw_back_addr; 
		CalcuCheckSum = 0;
		(*flash_entry)(N_LOG_READ,0, size/512,recover_read_lba,Buf);
		recover_read_lba += size/512;
	    CalcuCheckSum += cal_checksum(Buf+ (CurSysFile->Sys_offset),size-(CurSysFile->Sys_offset));
		remainlen -= (size-(CurSysFile->Sys_offset));
	    while(remainlen>0)
	    {
	    	if(remainlen<size)
				size = remainlen;
			if((*flash_entry)(N_LOG_READ, 0,size/512,recover_read_lba,Buf)){				
				printf("read error : addr:0x%x, size :0x%x\n",recover_read_lba,size);
			}		
			recover_read_lba += size/512;                   
	        CalcuCheckSum +=cal_checksum(Buf,size);
	        remainlen -= size;
	    }
	    if(ReadCheckSum !=CalcuCheckSum)
	    {
	        printf("firmware checksum error\n");
	        printf("%x,%x\n",ReadCheckSum,CalcuCheckSum);
	        ret=-1;
	                //不能再继续升级了
	   }
	}
	else
	{
		printf("Can't find \"CHECKSUM\"!\n");
	  	ret=-1;
	                //固件中不含checksum信息。
	}
    return ret;
}


void SetMultiBootPara(MuiltSectorBoot * MultiSecBoot,unsigned int BootDiskType,unsigned int len)
{
	int i;
	//重新计算checksum
	unsigned int *Boot=(unsigned int *)MultiSecBoot;
	unsigned int checksum=0;
	MuiltSectorBoot * tmpStorageInfo=NULL;
	(*flash_entry)(N_PHY_READ, 0, 0, 0, FlashRdBuffer);	
	tmpStorageInfo = (MuiltSectorBoot *)FlashRdBuffer;
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

		printf("@PageNumPerPhyBlk=0x%x\n",tmpStorageInfo->PAGE_CNT_PER_PHY_BLK);
		MultiSecBoot->PAGE_CNT_PER_PHY_BLK = 
				(unsigned int)(tmpStorageInfo->PAGE_CNT_PER_PHY_BLK);//GetPageCntPerPhyBlk();	
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
            if(0==strcmp(SysFlag,sysfileinfo->Sys_Flag))
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
		snprintf(ptr->id, 6, "%s", p->f_attrstr);

		ptr->part_off  = p->p_start;
		ptr->part_size = p->p_size;
		ptr->system_id = p->p_sysid;
		ptr->status    = 0x0;		
		
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

LinuxUpdateInfo LinuxUp;
char *mbr;
INT32U LinuxOffset;
INT32U DownloadFinish=0;
INT32U DownloadWholeLen;
//upgrade_func *fun;
INT8U *FirmBuffer;
unsigned int DecryptFlag=0;
unsigned int WholeFirmwareLen=0;
enum{FAT12,FAT16,FAT32};

int mbrc_read_compare(int blocknum, INT8U *pMbrc, INT8U *buffer, unsigned int sectorCnt)
{
        INT32S i, ret;

        for(i=0; i<sectorCnt; i++)
        {               		
				(*flash_entry)(N_PHY_READ,blocknum,i,0,buffer ); 
                ret = sector_compare((int *)buffer, (int *)(pMbrc+SECTOR_SIZE*i));
                if(ret == 0)
                {    
                    printf("mbrc_read_compare not equal====>break\n");						
                    return 1;	//not equal
                }
        }
	printf("mbrec equal\n");
	return 0;	//equal
}

int mbrc_write(int blocknum, INT8U *buffer, int sectorCnt)
{
    int i;			
    for(i=0; i<sectorCnt; i++)
    {   	
		(*flash_entry)(N_PHY_WRITE,blocknum,i,0,buffer+SECTOR_SIZE*i ); 
    }		
    return 0;
}

int FindUpdateInfo(char *attrstr, PartUpdate_Info *partupdate_info_p)
{
	int i;
    for(i=0;i<LinuxMax;i++)
    {
        if(0==strcasecmp(attrstr,partupdate_info_p->partupdate_attrstr))
        {
            printf("%s partupgrade info is %d\n",attrstr,partupdate_info_p->partupdate_flag);
			return partupdate_info_p->partupdate_flag;
        }
        partupdate_info_p++;		
    }
	printf("no %s partupgrade info\n",attrstr);
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
	
	printf("Udisk totalsec is 0x%x,%dMB,Udiskoffset is 0x%x\n",totalsec,tempM,Udiskoffset);

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

//	WrBuffer(buf,BPB.RsvdSecCnt*BPB.BytesPerSec,Udiskoffset);
	nandflash_write(Udiskoffset,buf,BPB.RsvdSecCnt*BPB.BytesPerSec/512);

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
	nandflash_write(Curflashoffset,FatBuffer,1);
	Curflashoffset++;

	for(i=0;i<(FatBufferSize/512-1);i++)
	{
		nandflash_write(Curflashoffset,buf,1);
		Curflashoffset++;
	}
//fat2	
	nandflash_write(Curflashoffset,FatBuffer,1);

	Curflashoffset++;

	for(i=0;i<(FatBufferSize/512-1);i++)
	{
		nandflash_write(Curflashoffset,buf,1);
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
		nandflash_write(Curflashoffset,buf,1);
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
		printf("linuxFWBakSize larger than linuxReserveCap\n");
		return 1;
	}

	if(LinuxUp.linuxUdiskFlag)
	{
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
	
	printf("Linux Part\n\n");
	for(i=0;i<LinuxUp.linuxNum;i++)
	{
		printf("%s:p_start=0x%x,p_size=0x%x\n",LinuxUp.linuxinfo[i].f_attrstr,LinuxUp.linuxinfo[i].p_start,LinuxUp.linuxinfo[i].p_size);
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
		printf("FIRM error\n\n");
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

	printf("DownloadWholeLen:0x%x\n\n\n",DownloadWholeLen);

	return 1;
}

void LinuxLoadFile(Sys_File *CurSysFile)
{
	int i;
	if((0==CurSysFile->Sys_para1) ||(0==CurSysFile->Sys_offset))
	{
		printf("not valid linux firmware\n\n");
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

	printf("WrBufferPartition:len=0x%x,flashoffset=0x%x\n",len,flashoffset);

	while(remainlen>0)
	{
		if(remainlen<FrameLen)
			size=remainlen;
		
		nandflash_write(flashoffset+Count/512,WrBufp,size/512);

		remainlen       -= size;
		Count	       += size;
		WrBufp		+=size;
		*Finish 		+= size;

		DownloadProgress=((*Finish)>>9)*100/(DownloadSize>>9);

		fillstatus(DownloadProgress,S_RUNNING);

	}
	printf("DownloadFinish:0x%x,DownloadSize:0x%x\n\n\n",*Finish,DownloadSize);

	return 1;
	
}


int WrPartition_Compress(DWORD fileoffset, DWORD filelen, DWORD flashoffset,DWORD *DownloadFinish,DWORD DownloadSize,int Decrypt)
{

	BYTE Infobuf[512];
	Compress_Info *Info_data=(Compress_Info*)Infobuf;
	DWORD offset;
	unsigned char *UnzipData;

	printf("WrPartition_Compress:fileoffset=0x%x,filelen=0x%x,flashoffset=0x%x\n",fileoffset,filelen,flashoffset);

	firmware_seek(fileoffset);
	firmware_read(Infobuf,512);

	offset=fileoffset+512;

	while(1)
	{
		if( (0==Info_data->ZipLen) ||(((BYTE*)Info_data)==(Infobuf+512)))
			break;

		UnzipData=firmware_Uncompres(offset,Info_data->ZipLen,Info_data->UnzipLen,Info_data->ZipChecksum,Info_data->UnzipChecksum);

		if(NULL==UnzipData)
			return 0;

		WrBufferPartition(UnzipData, Info_data->UnzipLen,flashoffset,DownloadFinish,DownloadSize);
		flashoffset+=Info_data->UnzipLen/512;
		Free_Unzipdata(UnzipData);

		offset+=(Info_data->ZipLen+511)/512*512;
		Info_data++;
	}
	return 1;
}

int WrPartition(DWORD fileoffset, DWORD filelen, DWORD flashoffset,DWORD *Finish,DWORD DownloadSize,INT32S Decrypt)
{
	DWORD Count=0;
	int	DownloadProgress;
	
	printf("WrPartition:fileoffset=0x%x,filelen=0x%x,flashoffset=0x%x\n",fileoffset,filelen,flashoffset);

	int FrameLen=16384;
	
	DWORD size=FrameLen;
	int remainlen=filelen;

	firmware_seek(fileoffset);
	
	while(remainlen>0)
	{		
		if(remainlen<FrameLen)
			size=remainlen;
		
		if(0 == firmware_read(FirmBuffer,size)){
			DownloadProgress=((*Finish)>>9)*100/(DownloadSize>>9);
			fillstatus(DownloadProgress,S_ERROR);
			return 0;
		}
		delay(2);
		nandflash_write(flashoffset+Count/512,FirmBuffer,size/512);

		remainlen       -= size;
		Count	        += size;
		*Finish += size;

		DownloadProgress=((*Finish)>>9)*100/(DownloadSize>>9);

		fillstatus(DownloadProgress,S_RUNNING);
	}
	printf("DownloadFinish:0x%x,DownloadSize:0x%x\n\n\n",*Finish,DownloadSize);
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
		printf("FIRM error\n");
		goto PROCESS_ERROR;
	}

	CurSysFile=(Sys_File*)(firmwarebuf+offset);
	
	DWORD FirmwareLen=CurSysFile->Sys_para1;
	DWORD FirmwareOffset=CurSysFile->Sys_offset;
	DWORD scodecap=CurSysFile->Sys_len/512;
	DWORD ZipLen=CurSysFile->Sys_para2;
	DWORD ZipChecksum;
	DWORD UnZipChecksum;	
	printf("len:0x%x,offset:0x%x,cap:0x%x,ziplen:0x%x\n\n",FirmwareLen,FirmwareOffset,scodecap,ZipLen);
	if(ZipLen)
	{
		ZipChecksum=*(DWORD*)((BYTE*)CurSysFile+8);
		UnZipChecksum=*(DWORD*)((BYTE*)CurSysFile+12);
	}

	if(ZipLen)
	{		
		printf("###uncompress linux kernel###\n");
		unzip_data=firmware_Uncompres(FirmwareOffset,ZipLen,FirmwareLen,ZipChecksum,UnZipChecksum);
		if(NULL==unzip_data)
			goto PROCESS_ERROR;
		memcpy(LFIbuf,unzip_data,512);
	}
	else
	{
		firmware_seek(FirmwareOffset);
		if(0 == firmware_read(FirmBuffer,512))
		{
			fillstatus(0,S_ERROR);
			goto PROCESS_ERROR;	
		}
		memcpy(LFIbuf,FirmBuffer,512);
	}
	

	LFIHead_t *LFIHeader=(LFIHead_t *)LFIbuf;
	LFIHeader->CapInfo.Logicalcap=Logicalcap;
	
	LFIHeader->CapInfo.Scodecap=scodecap;
	
	if( (scodecap*512*2) > LinuxOffset)
	{
		printf( "kernel is %dK,reserved is %dK,no kernel backup\n",(scodecap*512)/1024,LinuxOffset/1024);
		LFIHeader->CapInfo.Scodebakcap=0;		//no kernel backup
	}
	else
		LFIHeader->CapInfo.Scodebakcap=scodecap;

	LFIHeader->CapInfo.Micap=LinuxOffset/512;	//LFI-0x90

	if((LFIHeader->CapInfo.Micap!=old_mbroffset) &&(old_mbroffset!=0))
	{
		printf("remove HDCPKey,0x%x to 0x%x\n",old_mbroffset,LFIHeader->CapInfo.Micap);

		keybuf=malloc(1536);
		if(NULL==keybuf)
		{
			printf("malloc keybuf error\n ");
			goto PROCESS_ERROR;
		}
		nandflash_read(old_mbroffset+1,keybuf,3);	//this is hdcp key

		nandflash_write(LFIHeader->CapInfo.Micap+1,keybuf,3);

		free(keybuf);
	}
	

	//重新算checksum
	WORD *LFI_Header_point=(WORD*)LFIHeader;
	WORD headerchecksum=0;
	
	for(i=0;i<sizeof(LFIHead_t)/sizeof(WORD)-1;i++)
		headerchecksum=(WORD)(headerchecksum+LFI_Header_point[i]);
	
	LFIHeader->Headchecksum=headerchecksum;//前510byte校验和, WORD累加

	printf("sending kernel\n");
	nandflash_write(0,(BYTE *)LFIHeader,1);
	if(ZipLen)
		WrBufferPartition(unzip_data+512,FirmwareLen-512,1,&DownloadFinish,DownloadWholeLen);
	else 
	{
		if(0 ==WrPartition(FirmwareOffset+512,FirmwareLen-512,1,&DownloadFinish,DownloadWholeLen,DecryptFlag))
		goto PROCESS_ERROR;
	}
	
	if(LFIHeader->CapInfo.Scodebakcap)
	{
		printf("sending kernel backup\n");
		nandflash_write(scodecap,(BYTE *)LFIHeader,1);
		if(ZipLen)
			WrBufferPartition(unzip_data+512,FirmwareLen-512,1+scodecap,&DownloadFinish,DownloadWholeLen);
		else 
		{
			if(0==WrPartition(FirmwareOffset+512,FirmwareLen-512,1+scodecap,&DownloadFinish,DownloadWholeLen,DecryptFlag))
			goto PROCESS_ERROR;
		}
	}
	if(unzip_data)
		Free_Unzipdata(unzip_data);
	return 1;
PROCESS_ERROR:
	return 0;
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
		 
			 printf("sending [%d]%s\n",i,LinuxUp.linuxinfo[i].f_attrstr);
		 
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
			 printf("skip [%d]%s\n",i,LinuxUp.linuxinfo[i].f_attrstr);
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
				printf("not find udisk\n");
				return 0;
			}	
			printf("Formating Udisk\n");	
			DWORD UdiskSec=(DWORD)LinuxUp.linuxinfo[i].p_size;
			result=UdiskFormat(FirmBuffer,UdiskSec,&nouse,LinuxOffset/512+LinuxUp.linuxinfo[i].p_start);
			if(0==result)
			{
				printf("UdiskFormat error\n");
				return 0;
			}
		}
		else
		{
			printf("skip Format Udisk\n");
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

int Linux_Update(unsigned int Logicalcap,unsigned int old_mbroffset,unsigned char *buffer,struct Flash_Partiton * linux_part,PartUpdate_Info *partupdate_info_p)
{
	int offset;
	char newmbr[512];
//	char oldmbr[512];
	
	int fwback_space=0;

	FirmBuffer=buffer;
	DownloadFinish=0;
	
	printf("Logicalcap is 0x%x\n",Logicalcap);

//step 1
	firmware_seek(0);
	firmware_read((char *)FirmBuffer,FWIMG_Head_Max * sizeof(Sys_File));
	 if(0==MakeNewMbr(FirmBuffer, Logicalcap,newmbr,&fwback_space))
	 {
 		return 0;
	 }
	
//step 2,no need when recover
#if 0
	nandflash_read(old_mbroffset,oldmbr,1);	//read old mbr
	INFO("old mbr Offset=0x%x\n",old_mbroffset);
#endif

//step 3
	//WrBuffer((unsigned char *)newmbr,512,LinuxOffset/512);	//write new mbr to nand
	nandflash_write(LinuxOffset/512, (unsigned char *)newmbr, 1);
	printf("new mbr Offset=0x%x\n",LinuxOffset/512);

//step 4,no need when recover
	//Move_NeedSaved_LinuxPart(partupdate_info_p,newmbr,oldmbr,old_mbroffset);

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

	fillstatus(100,3);	
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
		printf("linux kernel is %dK,larger than reserved %dK\n",CurSysFile->Sys_len/1024,LinuxOffset/1024);

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
		 printf("fwbak pos is 0x%x sector\n",*fw_back_space);
	}
	else
	{
		*fw_back_space=-1;
		 printf("no fwbak in fw\n");
	}
	return 1;
}

INT32U recover_brec(void){
	int  i, Brec_Num;
	FWIMG_Head_Item_t * Head_Item;
	unsigned short CheckSumOld=0, CheckSumNew=0;
	char temp_buf[8192];
	unsigned int file_offset=0,file_length=0,file_temp=0;
	(*flash_entry)(N_LOG_READ,0, FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,fw_back_addr,FlashRdBuffer);
	unsigned int new_brec_addr = ((unsigned int)FlashRdBuffer + FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	unsigned int old_brec_addr = ((unsigned int)FlashRdBuffer + 512+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	
	if(CHK_Item_By_TYPE_NAME((unsigned char *)FlashRdBuffer, (unsigned char*)&BootDiskType, "brec", &Head_Item))
	{
		printf("BREC Item is not found by type_name\n");
		
		return 1;
	}
	printf("BREC->type    : %s\n", Head_Item->type);
	printf("BREC->filename: %s\n", Head_Item->filename);
	printf("BREC->offset  : 0x%x\n", Head_Item->offset);
	printf("BREC->size    : 0x%x\n", Head_Item->size);
	printf("BREC->address : 0x%x\n", Head_Item->address);
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;
	file_temp = file_offset + file_length;
	brec_sector_num = (Head_Item->size)>>9;
	recover_read_lba =fw_back_addr + ((file_temp/512)-1);	
	(*flash_entry)(N_LOG_READ, 0,1,recover_read_lba,temp_buf);
	CheckSumNew = *(unsigned short *)(temp_buf + 510);

	printf("BREC_SECTOR_NUM is %d\n", brec_sector_num );
	printf("BootDiskType:0x%x\n", BootDiskType );
	if(file_length != brec_sector_num*512)
		printf("BREC Size May Be Wrong\n");
	(*flash_entry)(N_PHY_READ,4, brec_sector_num - 1,0, (char *)old_brec_addr);
	CheckSumOld = *(unsigned short *)(old_brec_addr + 510);

	if(CheckSumOld != CheckSumNew)	
	{
		printf("CheckSumOld: 0x%x, CheckSumNew: 0x%x\n", CheckSumOld, CheckSumNew);					
		for(Brec_Num=4; Brec_Num<8; Brec_Num++)
		{
			printf("Write_BREC, Num = %d\n", Brec_Num);			
			(*flash_entry)(N_PHY_READ,Brec_Num, 0, 0,(unsigned char *)temp_buf); 	//????????读取数据的空间大小
			recover_read_lba = fw_back_addr;
			recover_read_lba += (file_offset/512);
			if(file_offset%512 == 0){
				file_temp = 1;
			}else{
				file_temp = 2;
			}
			for(i = 0; i < brec_sector_num; i++)
			{					
				(*flash_entry)(N_LOG_READ, 0,file_temp,recover_read_lba,temp_buf);
				recover_read_lba += 1;				
				(*flash_entry)(N_PHY_WRITE,Brec_Num, i,0,(char *)(temp_buf + (file_offset & 0x1ff)));
			}
			if((BootDiskType==SNOR080)||(BootDiskType==SNOR090))
				break;				
		}
	}
	(*flash_entry)(N_PHY_READ,4, brec_sector_num - 1,0, (char *)old_brec_addr);
	CheckSumOld = *(unsigned short *)(old_brec_addr + 510);
	
	if(CheckSumOld != CheckSumNew)
	{
		printf("Recover BREC error!! CheckSumOld: 0x%x, CheckSumNew: 0x%x\n", CheckSumOld, CheckSumNew);
		
		return 1;
	}
	printf("Write_BREC over\n");
	
	return 0;		
}

INT32U recover_mbrec(void){
	unsigned short mbrec_sign;
	unsigned int CheckSumNew, CheckSumOld, i, j,*OTADirtyFlag;
	FWIMG_Head_Item_t * Head_Item;
	unsigned int file_offset=0,file_length=0,file_temp=0;
	unsigned int old_mbrec_addr,new_mbrec_addr;
//	char temp_buf[2048];
	old_mbrec_addr = ((unsigned int)FlashRdBuffer + FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));	//存放从文件中读出的数据, 
	new_mbrec_addr = ((unsigned int)FlashRdBuffer + 512+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	if(CHK_Item_By_TYPE_NAME((unsigned char *)FlashRdBuffer, (unsigned char*)&BootDiskType, "mbrec", &Head_Item))
	{
		printf("MBREC Item is not found by type_name\n");		
		return 1;
	}	
	printf("MBREC->type    : %s\n", Head_Item->type);
	printf("MBREC->filename: %s\n", Head_Item->filename);
	printf("MBREC->offset  : 0x%x\n", Head_Item->offset);
	printf("MBREC->size    : 0x%x\n", Head_Item->size);
	printf("MBREC->address : 0x%x\n", Head_Item->address);
	recover_read_lba = fw_back_addr;
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;
	file_temp = file_offset + file_length;
	mbrec_sign = *(unsigned short*)(old_mbrec_addr + 506);
	if(512==Head_Item->size)
	{
		recover_read_lba += (file_offset/512);
		if(file_temp%512 == 0){
			file_temp = file_temp/512 - file_offset/512;
		}else{
			file_temp = file_temp/512 - file_offset/512 +1;
		}
		(*flash_entry)(N_LOG_READ, 0,file_temp,recover_read_lba,(unsigned char *)new_mbrec_addr);
		new_mbrec_addr = (unsigned short *)(new_mbrec_addr + (unsigned int)(file_offset & 0x1ff));
		mbrec_sign = *(unsigned short*)(new_mbrec_addr + 506);
		OTADirtyFlag = (unsigned char *)(new_mbrec_addr + Head_Item->size);
		memset(OTADirtyFlag,0xff,512);
		*((unsigned int *)OTADirtyFlag) = OTADIRTYSIGN;

		if(mbrec_sign == 0x55aa)	
		{
			CheckSumNew = my_calCRC2((unsigned char *)(new_mbrec_addr), 508, 4);
			CheckSumNew += 0x1234;
			if(CheckSumNew != *(unsigned int *)(new_mbrec_addr + 508 ))
			{
				printf("The Geted MBREC Check SUM error, CheckSum: 0x%x, SHOULD BE: 0x%x\n", CheckSumNew, *(unsigned int *)(new_mbrec_addr + 508));
				
				return 1;
			}
		}
		else
		{
			printf("Read_MBREC error\n");
			
			return 1;
		}
		
		//get old mbrec and recover
		for(i=0; i<4; i++)	//new_mbrec_addr未更新
		{
			(*flash_entry)(N_PHY_READ,i, 0, 0,(char *)old_mbrec_addr); 	//????????读取数据的空间大小
			if(memcmp((char *)new_mbrec_addr, (char *)old_mbrec_addr, 0x200))
			{
				printf("Write_MBREC, num = %d\n", i);
				
				(*flash_entry)(N_PHY_WRITE, i , 0, 0, (char *)new_mbrec_addr);
				(*flash_entry)(N_PHY_READ, i, 0, 0,(char *)old_mbrec_addr);
				
				CheckSumOld = my_calCRC2((unsigned char *)old_mbrec_addr, 508, 4);
				CheckSumOld += 0x1234;
				
				if(CheckSumOld != CheckSumNew)
				{
					printf("Read MBREC again, checksum error\n");
					
					return 1;
				}
			}			
			(*flash_entry)(N_PHY_READ,i, 19, 0, (INT8U *)(OTADirtyFlag+512));
			if(0==Check_All0xFF((INT8U *)(OTADirtyFlag+512),512))				
				(*flash_entry)(N_PHY_READ,i, 19, 0, (INT8U *)(OTADirtyFlag));

		}
	}
	else		//multi-sector
	{
		int sectornum;
		unsigned int len;
		unsigned char *MbrecPointer;//=(unsigned char *)new_mbrec_addr
		MuiltSectorBoot *SectorBoot;
		len=Head_Item->size;	
		printf("multi mbrec len= 0x%x\n", len);
		SectorBoot=(MuiltSectorBoot *)new_mbrec_addr;
		memset((char *)SectorBoot,0,sizeof(MuiltSectorBoot));
		SectorBoot->MbrecRunAddr=Head_Item->para2;	
		MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
		recover_read_lba += (file_offset/512);
		MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));		
		(*flash_entry)(N_LOG_READ, 0,file_length/512,recover_read_lba,(unsigned char *)(MbrecPointer));
		SetMultiBootPara(SectorBoot,BootDiskType,len);
		ProcessBrec(MbrecPointer,len);

		len+=512;
		printf("MBrec Size =0x%x\n",len);

		sectornum=len/512;
		if(sectornum>=16)
		{
			printf("MBrec Size >=8 KB \n");			
			return 0x01 ;
		}
		MbrecPointer=(unsigned char *)new_mbrec_addr;
		OTADirtyFlag=(unsigned char *)(MbrecPointer + len);
		memset(OTADirtyFlag,0xff,512);
		*((unsigned int *)OTADirtyFlag) = OTADIRTYSIGN;
		for(i=0;i<4;i++)
		{
			if( mbrc_read_compare( i, MbrecPointer,(INT8U*) old_mbrec_addr, sectornum) )//读并比较Block i Mbrec
            {
                printf("nand Write_MBREC, num = %d\n", i);				
				(*flash_entry)(N_PHY_ERASE,i,0,0,FlashRdBuffer+sectornum*512); 	
                mbrc_write(i, MbrecPointer, sectornum);
                if( mbrc_read_compare( i, MbrecPointer, (INT8U*) old_mbrec_addr, sectornum) )
                {
                    printf("nand second Write_MBREC, num = %d\n", i);					
                    mbrc_write(i, MbrecPointer, sectornum);
                    if(!mbrc_read_compare( i, MbrecPointer, (INT8U*) old_mbrec_addr, sectornum) )
                    {
                        printf("find correct mbrec====>file:%s, line:%d\n", __FILE__, __LINE__);
                        j++;
                    }
					else
					{
						printf("NAND MBREC write err\n");
						return 1;
					}
                }
                else 
                {
                     printf("find correct mbrec====>file:%s, line:%d\n", __FILE__, __LINE__);
                     j++;
                }
            }
            else 
            {
                printf("find correct mbrec====>file:%s, line:%d\n", __FILE__, __LINE__);
                j++;
            }			
			(*flash_entry)(N_PHY_READ, i,(16 + sectornum)/16 * 16 + 3,0,(INT8U *)(OTADirtyFlag+512));
			if(0==Check_All0xFF((INT8U *)(OTADirtyFlag+512),512))
				(*flash_entry)(N_PHY_WRITE, i,(16 + sectornum)/16 * 16 + 3,0,(INT8U *)(OTADirtyFlag));
		}
	}	
	printf("Write_MBREC over\n");	
	return 0;	
}

INT32U recover_image(unsigned int Logicalcap,unsigned int mbroffsetlba){
	(*flash_entry)(N_LOG_READ,0,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,fw_back_addr,FlashRdBuffer);
	if(0==Linux_Update(Logicalcap,mbroffsetlba,FlashRdBuffer,linux_mbr,PartUpdate_info))
		return 1;	
	(*flash_entry)(N_LOG_UPDATE,0,0,0,FlashRdBuffer);
    printf("Write IMG over\n");
    return 0; 		
}

INT32U ClearOTADirtyFlag(void){
	int i,j,sector_num;
	FWIMG_Head_Item_t * Head_Item;
	unsigned int file_offset=0,file_length=0,file_temp=0,len;
	unsigned int old_mbrec_addr,new_mbrec_addr;
	unsigned char *MbrecPointer;	
	MuiltSectorBoot * multiboot =(MuiltSectorBoot *)FlashRdBuffer;		
	(*flash_entry)(N_LOG_READ, 0,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,fw_back_addr,(unsigned char *)(FlashRdBuffer));
	new_mbrec_addr = ((unsigned int)FlashRdBuffer+FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t));
	if(CHK_Item_By_TYPE_NAME((unsigned char *)FlashRdBuffer, (unsigned char*)&BootDiskType, "mbrec", &Head_Item))
	{
		printf("MBREC Item is not found by type_name\n");		
		return 1;
	}	
	recover_read_lba = fw_back_addr;
	file_offset = Head_Item->offset;
	file_length = Head_Item->size;
	file_temp = file_offset + file_length;	
	len=Head_Item->size;	
	multiboot=(MuiltSectorBoot *)new_mbrec_addr;
	memset((char *)multiboot,0,sizeof(MuiltSectorBoot));
	multiboot->MbrecRunAddr=Head_Item->para2;	
	MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));
	recover_read_lba += (file_offset/512);
	MbrecPointer=(unsigned char *)(new_mbrec_addr+sizeof(MuiltSectorBoot));		
	(*flash_entry)(N_LOG_READ, 0,file_length/512,recover_read_lba,(unsigned char *)(MbrecPointer));
	SetMultiBootPara(multiboot,BootDiskType,len);
	ProcessBrec(MbrecPointer,len);
	len+=512;
	sector_num=len/512;
	if(sector_num>=16)
	{
		printf("MBrec Size >=8 KB \n");			
		return 0x01 ;
	}
	MbrecPointer=(unsigned char *)new_mbrec_addr;
/*
	for(i=0;i<4;i++){
		(*flash_entry)(N_PHY_READ, i , 0, 0,FlashRdBuffer);		
		if(0 == memcmp(MULTIBOOTFLAG,FlashRdBuffer,strlen(MULTIBOOTFLAG))){
			sector_num =(1+ multiboot->Mbrec0_SectorNum);
			for(j=1;j<sector_num;j++)		
				(*flash_entry)(N_PHY_READ,i,j,0,(unsigned char *)(FlashRdBuffer+j*512));
			break;
		}
	}
	if(i>=4)
		return 1;	
*/
	for(i=0;i<4;i++){
		(*flash_entry)(N_PHY_ERASE,i,0,0,MbrecPointer+sector_num*512);		
		for(j=0;j<sector_num;j++)
			(*flash_entry)(N_PHY_WRITE,i,j,0,(unsigned char *)(MbrecPointer+j*512));			
//		printf("clear OTA flag\n");
	}	
    printf("Finish clearing OTA dirty flag\n");
	return 0;
}

int recover_firmware(unsigned int flash_read_fun_addr,unsigned int recover_mode){
	unsigned int  firmware_addr,unziplen,ziplen;
	int      i,j,sector_num,tmp,ret=0;	
	MuiltSectorBoot * multiboot =(MuiltSectorBoot *)FlashRdBuffer;
	FlashRdBuffer = (unsigned char *)malloc(SECTOR_RW_ONCE*512*sizeof(unsigned char));
	flash_entry = (int(*)(unsigned char,unsigned short,unsigned short,int,unsigned char*))flash_read_fun_addr;
	if(!FlashRdBuffer)
		return 1;
	LFIHead_t * lfi_hd; //逻辑目录项
	/*
	if(1==recover_mode){
		for(i=0;i<4;i++){
			(*flash_entry)(N_PHY_READ, i , 0, 0,FlashRdBuffer);
			if(0 == memcmp(MULTIBOOTFLAG,FlashRdBuffer,strlen(MULTIBOOTFLAG))){
				sector_num = (17+ (multiboot->Mbrec0_SectorNum))/16 * 16 + 3;
				break;
			}
		}
		if(i<4){
			tmp = 0;
			for(i=0;i<4;i++){
				(*flash_entry)(N_PHY_READ, i , sector_num, 0,FlashRdBuffer);
				if(OTADIRTYSIGN == *(unsigned int *)FlashRdBuffer)
					tmp++;
			}
			if(tmp<4){
				if(0==ClearOTADirtyFlag())
					goto RECOVER_END;
			}
		}
	}
	*/
//	printf("Start recovering\n");
	kernel_offset = get_kernelcap();
	BootDiskType = get_bootmedia_type();
	load_bootarg();
	if(find_newfw_addr(&fw_back_addr)){
		printf("can't find backup firemware\n");
		ret = 1;
		goto RECOVER_END;
	}
	recover_read_lba = fw_back_addr;
	write_lba = fw_back_addr;	
	if(Firm_ChechsumFlow()){
		ret = 1;
		goto RECOVER_END;
	}
	(*flash_entry)(N_LOG_READ,0,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,0,FlashRdBuffer);
	lfi_hd = (LFIHead_t *)FlashRdBuffer;	
	memcpy(&old_capinfo, &(lfi_hd->CapInfo), sizeof(CapInfo_t));
	printf("CapInfo.Scodecap	: 0x%x\n", old_capinfo.Scodecap);
	printf("CapInfo.Scodebakcap : 0x%x\n", old_capinfo.Scodebakcap);
	printf("CapInfo.Appinfocap	: 0x%x\n", old_capinfo.Appinfocap);
	printf("CapInfo.Liccap		: 0x%x\n", old_capinfo.Liccap);
	printf("CapInfo.Micap		: 0x%x\n", old_capinfo.Micap);
	printf("CapInfo.Dspcap		: 0x%x\n", old_capinfo.Dspcap);
	printf("CapInfo.Udiskcap	: 0x%x\n", old_capinfo.Udiskcap);
	printf("CapInfo.Logicalcap	: 0x%x\n", old_capinfo.Logicalcap);
	(*flash_entry)(N_LOG_READ,0,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,fw_back_addr,FlashRdBuffer);
	tmp =FindSysFile("COMPRESS",FlashRdBuffer);
	if(tmp>0){
//		printf("file is compressed format!\n");
		compress_flag = 1;
		Sys_File *CurSysFile=(Sys_File *)((unsigned int)FlashRdBuffer+tmp);
		unziplen=  CurSysFile->Sys_para1;			//for malloc zip memory
		ziplen=  CurSysFile->Sys_para2;	
		unzipmem = malloc(unziplen);
		zipmem = malloc(ziplen);
		if((!zipmem)||(!unzipmem)){			
			printf("malloc memery space for unzip file error\n");
			if(zipmem)
				free(zipmem);
			if(unzipmem)
				free(unzipmem);
			ret = 1;
			goto RECOVER_END;

		}
	}
	else{
		compress_flag = 0;
	}
	recover_read_lba = fw_back_addr;	
	(*flash_entry)(N_LOG_READ,0,FWIMG_Head_Max*sizeof(FWIMG_Head_Item_t)/512,recover_read_lba,FlashRdBuffer);
	fillstatus(0, 1);	
	if(recover_mbrec())
	{
		printf("Write_MBREC error\n");
		fillstatus(1, 4);
		ret = 1;
		goto RECOVER_FREEMEM;
	}	
	fillstatus(1, 1);
	if(recover_brec())
	{
		printf("Write_BREC error\n");
		fillstatus(2, 2);
		ret = 1;
		goto RECOVER_FREEMEM;
	}
	fillstatus(2, 1);
	if(recover_image(old_capinfo.Logicalcap,old_capinfo.Micap))
	{
		printf("Write_Image error\n");	
		fillstatus(3, 1);
		ret = 1;
		goto RECOVER_FREEMEM;
	}
	ClearOTADirtyFlag();
RECOVER_FREEMEM:
	if(1 == compress_flag){
		if(zipmem)
			free(zipmem);
		if(unzipmem)
			free(unzipmem);
	}
RECOVER_END:
	return ret;		
}

int __attribute__((section(".entry")))recover_entry(unsigned int flash_read_fun_addr,unsigned int mode,unsigned char **part_info,unsigned int part_info_len)
{
	int ret = 0 ;
	serial_init();
	init_heap();
	ret = recover_firmware(flash_read_fun_addr,mode);
	return ret;
}


