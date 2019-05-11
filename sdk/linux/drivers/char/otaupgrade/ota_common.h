#ifndef _OTA_COMMON_H
#define _OTA_COMMON_H

#include "ota_updata.h"
#include "am7x_flash_api.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>

#define  FWIMG_Head_Max 32
#define SectorMode   		0x00000100
#define CompreddMode 	0x00010000

//fw head
typedef struct
{
    char type[4]; 
    char filename[12];
    int  offset;
	int  size;
	int  address;
	int para2;	
}__attribute__ ((packed)) FWIMG_Head_Item_t;                 //32×Ö½Ú

typedef struct 
{
	char  Sys_Flag[16];
	unsigned int Sys_offset;
	unsigned int Sys_len;
	unsigned int Sys_para1;
	unsigned int Sys_para2;
}__attribute__ ((packed))Sys_File;


typedef unsigned char   BYTE;
typedef unsigned short   WORD;
typedef unsigned int   DWORD;
typedef long long   LONGLONG;



typedef struct {
	unsigned int   p_size;     /* partition size */
	unsigned char  p_sysid;    /* system id */
	unsigned int   p_start;		/* partition offset */
	unsigned int   p_oldstart;    /* last time partition offset */
	//@fish 2011-01-06 del no use 
	//char		   f_path[256];  
	char		   f_attrstr[8];
	unsigned int   f_len;
	unsigned int   f_malloclen;
	unsigned int   f_offset;
	int			   f_updateflag;
	unsigned int   f_compressflag;
}part_desc;


typedef struct 
{
	part_desc linuxinfo[LinuxMax];
	int		  linuxNum;
	int		  linuxUdiskFlag;
	int		  linuxFileMode;
	DWORD	  linuxReserveCap;
	DWORD	  linuxFWBakSize;
}LinuxUpdateInfo;


/*
static struct {
	const char   *fs_type;
	unsigned char magic_number;
} fs_type_list[] = {
	{ "FAT12", 0x1 },
	{ "FAT16", 0x6 },
	{ "FAT32", 0xb },
	{ "EXT2",  0x83 },
	{ "EXT3",  0x83 },
	{ "ROMFS", 0x83 },
};
*/


#define SECTOR_SIZE     512
#define MBR_SIGNATURE   0xaa55
#define MBR_SIGNATURE2  0x594C

#define MAX_PRIV_PARTS    16

#define PART_ALIGNMENT  0x10000
#define SECTOR_SIZE     512


struct mbr_ptr {
    uint8_t  status;
    uint8_t  s_head;
    uint8_t  s_sector    : 6;
    uint8_t  s_cylinder2 : 2;
    uint8_t  s_cylinder1;
    uint8_t  system_id;
    uint8_t  e_head;
    uint8_t  e_sector     : 6;
    uint8_t  e_cylinder2  : 2;
    uint8_t  e_cylinder1;
    uint32_t part_off;  /* LBA of first sector in the partition */
    uint32_t part_size; /* number of blocks in partition */
} __attribute__((packed));


struct priv_ptr {
    uint8_t  system_id;
    char     id[6];
    uint8_t  status;
    uint32_t part_off;   /* LBA of first sector in the partition */
    uint32_t part_size;  /* number of blocks in partition */
} __attribute__((packed));



struct priv_mbr {
	uint8_t          data[254];
	struct priv_ptr  parts[MAX_PRIV_PARTS];
	uint16_t         signature;
};

typedef struct {
	int (*firmware_seek)(unsigned int offset);
	int (*firmware_read)(char *buf,unsigned int len);
	int (*nandflash_read)(int lbaoffset,char *buf,unsigned int sec);
	int (*nandflash_write)(int lbaoffset,char *buf,unsigned int sec); 
	void (*feed_status_common)(int prg, Fwu_state_t state);
	void (*delay_common)(int times);
	unsigned char *(*firmware_Uncompres)(unsigned int offset,unsigned int ZipLen,unsigned int UnzipLen,unsigned int ZipChecksum,unsigned int UnzipChecksum);
	void (*Free_Unzipdata)(unsigned char *Unzipdata);
}upgrade_func;

typedef struct 
{
	DWORD ZipLen;
	DWORD UnzipLen;
	DWORD ZipChecksum;
	DWORD UnzipChecksum;
}Compress_Info;

typedef struct
{
	WORD BytesPerSec;
	BYTE SecPerClust;
	WORD RsvdSecCnt;
	BYTE FATSize;
	WORD RootEntCnt;       // number of root dir entries,only for FAT16.     
	WORD SamllSector;
	BYTE MediaDescriptor; //F8
	WORD SecPerFat;
	WORD SecPerTrark;	//3
	WORD NumHead;		//0x40
	DWORD HiddenSector;
	DWORD LargeSector;
} __attribute__((packed))Fat_BPB;

typedef struct
{
	DWORD Fat32SecPerFat;
	WORD  Fat32Flag;
	WORD  Fat32Version;
	DWORD Fat32Root1stCluster;
	WORD  Fat32FSinfo;
	WORD  Fat32BackupBootSector;
	BYTE  Fat32reseved[12];
} __attribute__((packed))Fat32_Section;

typedef struct
{
	char JumpIns[3];
	char OEMID[8];
	Fat_BPB BPB;
} __attribute__((packed))Fat_DBR;

typedef struct
{
	BYTE phyDrvNum;		//0x80
	BYTE Reserved;		//0
	BYTE ExtendedBootSignature;	//0x29
	DWORD Volume;
	char VolumeLabel[11];
	char SysID[8];
} __attribute__((packed))Fat_ExBPB;


#define FAT_FSINFO_SIG1	0x41615252
#define FAT_FSINFO_SIG2	0x61417272
typedef struct
{
	int   signature1;	/* 0x41615252L */
	int   reserved1[120];	/* Nothing as far as I can tell */
	int   signature2;	/* 0x61417272L */
	int   free_clusters;	/* Free cluster count.  -1 if unknown */
	int   next_cluster;	/* Most recently allocated cluster */
	int   reserved2[4];
} __attribute__((packed))fat_boot_fsinfo;

INT32S sector_compare( INT32S* srcdata_addr, INT32S* dstdata_addr );


int ProcessBrec(unsigned char BrecBuf[],unsigned int BrecSize);

int my_calCRC2(unsigned char *buf, unsigned int length, unsigned char nBytes);

int CHK_Item_By_TYPE (unsigned char * Buff_addr, unsigned char * type, FWIMG_Head_Item_t ** Item);
int CHK_Item_By_NAME (unsigned char * Buff_addr, unsigned char * filename, FWIMG_Head_Item_t ** Item);
int CHK_Item_By_TYPE_NAME (unsigned char * Buff_addr, unsigned char * type, char * filename, FWIMG_Head_Item_t ** Item);

int FindSysFile(char *SysFlag, unsigned char *buf);


int Linux_Update(upgrade_func *func,unsigned int Logicalcap,unsigned int old_mbroffset,unsigned char *buffer,struct Flash_Partiton * linux_part,PartUpdate_Info *partupdate_info_p);

int WrBuffer(BYTE *WrBuf, DWORD len, DWORD flashoffset);
int AnalysisUpdataFlag(char *flagbuf,PartUpdate_Info *cur_info);
void AnalysisUpdataFile(void);
int CalculateDownloadLen(BYTE *firmwarebuf,PartUpdate_Info *partupdate_info_p);
int MakeNewMbr(BYTE *FwHeader,unsigned int Logicalcap,char *newmbr,int * fw_back_space);
void Move_NeedSaved_LinuxPart(PartUpdate_Info *partupdate_info_p,char *newmbr,char *oldmbr,unsigned int old_mbroffset);
int GetInfo_InNewMbr(char *attrst,char *mbrdata,unsigned int *part_off,unsigned int *part_size);
int GetInfo_InOldMbr(char *attrst,char *mbrdata,int partNum,unsigned int *part_off,unsigned int *part_size);
void MoveFlashData(unsigned int dstbla,unsigned int srcbla,unsigned int lbasize);
void Print_partupdate_inf(PartUpdate_Info *partupdate_info_p);



#endif

