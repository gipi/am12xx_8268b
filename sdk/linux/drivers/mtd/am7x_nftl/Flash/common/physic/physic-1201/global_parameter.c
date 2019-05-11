/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : global_parameter.c
* By      : mengzh
* Version : V0.1
* Date    : 2007-7-18 16:52
*********************************************************************************************************
*/

#include "nand_flash_driver_type.h"

struct StorageInfo          NandStorageInfo;
struct SpecialCmdType       SpecialCommand;
struct LogicOrganizeType    LogicOrganisePara;
struct ZoneInfoType         ZoneInfo[NAND_MAX_DIE_NUM * MAX_ZONE_NUM_IN_DIE];
struct ZoneTblCacheInfo     ZoneTblCache;
struct PageTblCacheInfo     PageMapTblCache;
struct BitMapInfo     BITMAPInfo;

struct  _Debug_Wear  WearDbg;
#if(defined(__KERNEL__)) && (SUPER_WEAR_LEVELLING ==1)  
struct ReClamBlkInfo    Reclaim_Blk;
#endif
struct ECC_proetect_Read  Prot_Read;

///INT8U MBRC_Total_Size; //CODE+TABLE

struct NandDev NandDevInfo =
{
	.NandFlashInfo = &NandStorageInfo,
	.ZoneInfo  = ZoneInfo,
	.ZoneTblCacheInfo = &ZoneTblCache,
	.PageTblCacheInfo = &PageMapTblCache,
	.SpecialCommand = &SpecialCommand,
	.LogicOrganizePar = &LogicOrganisePara,
	.BITMAP_DATA = &BITMAPInfo,
};

/* last free block position */
INT32U NandDMAChannel;
INT8U Flash_DMA_Mode ;


