/*
********************************************************************************
*                       AM7511T---upgrade.drv
*                (c) Copyright 2002-2007, Actions Co,Ld. 
*                        All Right Reserved 
*
* FileName: upgrade.h     Author: Tian Zhimin        Date:2008/06/26
* Description: defines macros for upgrading
* Others:      
* History:         
* <author>       <time>       <version >    <desc>
* Tian Zhimin  2008/06/26       1.0         build this file
********************************************************************************
*/ 



#ifndef  __OTA_H 
#define  __OTA_H 

#include <sysinfo.h>
#include <am_types.h>
#include<linux/types.h>
#include <ota_updata.h>

#define	__DEBUG_UPGRADE__	1
#define	__DEBUG_ERR__	1
#define __DEBUG_RES__	1

#if __DEBUG_UPGRADE__
#define INFO(stuff...)		printk("<INFO>: " stuff)
#else
#define INFO(stuff...)		do{}while(0)
#endif
#if __DEBUG_ERR__
#define ERR(stuff...)		printk("<ERR> : " stuff)
#else
#define ERR(stuff...)		do{}while(0)
#endif

#if __DEBUG_RES__
#define INFO_R(stuff...)		printk("<INFO>: " stuff)
#else
#define INFO_R(stuff...)		do{}while(0)
#endif


#define	TASK_STK_SIZE			4096
#define	SYSUP_TASK_PRIO			TASKPRIO_BG_TASK0
//#define	MY_TASK_ID			TASKID_BG_TASK0

typedef struct 
{
	INT32U	file_pMODE;		//文件模式
	//FS_u8		file_path[64];
	INT8U *		file_path;	//文件路径和名称
	struct file *	fp;	//file结构指针
	INT32U		file_length;
	loff_t	file_offset;	//文件偏移
	loff_t	offset2;


}File_OP_t;




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



#define SECTOR_RW_ONCE			32
#define BREC_RW_ONCE			(SECTOR_RW_ONCE - 1)


#endif /*__UPGRADE_H */

