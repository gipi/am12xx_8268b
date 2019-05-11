#ifndef _OTA_FW_H
#define _OTA_FW_H

//#include <linux/types.h>

#define  FWIMG_Head_Max 32
#define  LinuxMax       16
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


#define SECTOR_SIZE     512
#define MBR_SIGNATURE   0xaa55
#define MBR_SIGNATURE2  0x594C

#define MAX_PRIV_PARTS    16

#define PART_ALIGNMENT  0x10000
#define SECTOR_SIZE     512


typedef struct {
	int (*firmware_seek)(unsigned int offset);
	int (*firmware_read)(char *buf,unsigned int len);
	int (*nandflash_read)(int lbaoffset,char *buf,unsigned int sec);
	int (*nandflash_write)(int lbaoffset,char *buf,unsigned int sec); 
	void (*feed_status_common)(int prg, Fwu_state_t state);
	void (*delay_common)(int times);
}upgrade_func;

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

#endif

