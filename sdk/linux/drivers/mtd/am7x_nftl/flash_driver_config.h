/*
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : flash_driver_config.h
* By      : nand flash group
* Version : V0.1
* Date    : 2007-7-3 17:06
*********************************************************************************************************
*/
#ifndef _FLASH_DRIVER_CONFIG_
#define _FLASH_DRIVER_CONFIG_

#include "flash_driver_oal.h"
#include "./../flash_drv_params.h"


//#define KERNEL_DRIVER
#define NO_KERNEL_DRIVER
#define FWSC_DRIVER     0x00


#define NAND_DMA_CHANNEL_NUM                             (0)
#define NAND_DMA_CHANNEL1_NUM                            (0)
#define NAND_DMA_CHANNEL2_NUM                            (0)


/* the max chip number that driver support */
#define NAND_MAX_CHIP_NUM                               (2)



/* the max die number that driver support */
#define NAND_MAX_DIE_NUM                                (4)

/* the max zone number in a die */
#define MAX_ZONE_NUM_IN_DIE                             (4)


/* zone table cache number that driver used */
#define ZONE_CACHE_NUM                                  (2)

/* page mapping table cache number that driver used */
#define LOG_CACHE_NUM                                   (8)

/* the max log block number that one zone contain */
#define MAX_LOG_BLK_NUM                                 (8)

/* the block number of zone */
#define BLK_NUM_PER_ZONE                                (1024)

/* the default data block number in one zone */
#define DEFAULT_DATA_BLK_PER_ZONE                       (984)

/* if the driver need support multiplane program */
#define SUPPORT_MULTI_PLANE                             (1)

/* if the driver need support internal interleave */
#define SUPPORT_INTERNAL_INTERLEAVE                     (0)

/* if the driver need support external inter leave */
#define SUPPORT_EXTERNAL_INTERLEAVE                     (0)

/* if the driver need support read-reclaim */
#define SUPPORT_READ_RECLAIM                            (1)

/* if the driver need support wear-levelling */
#define SUPPORT_WEAR_LEVELLING                          (1)

/*@fish add  for Wear for Copy Page */
#define SUPER_WEAR_LEVELLING                                    (1)

#define TEST_WEAR_LEVELING                                      (0)
#define DEBUG_ERASE_CNT                                          (0)

/*@fish add for Code Proect */
#define CODE_PROTECT_WEAR_LEVELING    (1)
/* if use dma to transfer data on ram */
#define SUPPORT_DMA_TRANSFER_DATA_ON_RAM                (0)	//(1)

/* if support copy nand page with copy command */
#define SUPPORT_PAGE_COPYBACK                           (1)

/* if support small block flash */
#define SUPPORT_SMALL_BLOCK                             (0)

/* define after how many erase times should do one wear-levelling */
#define WEAR_LEVELLING_FREQUENCE                        (15*2)

/* define the nand flash sector size */
#define NAND_SECTOR_SIZE                                (512)
#define NAND_SECTOR_1KB                                (512*2)

/* define the nand flash spare area size, contain user data and ecc parity */
#define SPARE_SIZE_PER_SECTOR                           (16)
#define SPARE_SIZE_PER_SECTOR_12BitECC          (23)
#define SPARE_SIZE_BCH8            (16)
#define SPARE_SIZE_BCH12          (23)
#define SPARE_SIZE_BCH24          (44) 
#define SPARE_SIZE_BCH40          (72)
#define SPARE_SIZE_BCH60          (107)

/* define the spare size for store user data for ervery sector */
#define NAND_SPARE_SIZE                                 (3)

/* page buffer size can be how many times of sigle page size, the value can only be 1 or 2 */
#define PAGE_BUFFER_SIZE                                (2)

#define OPCYCLE (SECTOR_NUM_PER_SUPER_PAGE / PAGE_BUFFER_SECT_NUM)
/*times of phyical read or physical write function was called when read or write one super page*/

#if ((PAGE_BUFFER_SIZE != 1) && (PAGE_BUFFER_SIZE != 2))
#error Configuration Error! Page buffer size config is valid!!!
#endif

/* boot chip number */
#define BOOT_CHIP_NUM                                   (0)





/* the sector nuber of a single page */
#define SECTOR_NUM_PER_SINGLE_PAGE                      (NandStorageInfo.SectorNumPerPage)

/* the sector number of a super page */
#define SECTOR_NUM_PER_SUPER_PAGE                       (NandStorageInfo.SectorNumPerPage * NandStorageInfo.PlaneCntPerDie)

/* the spare area start address in a page */
#define SPARE_AREA_START_ADDR                           (NAND_SECTOR_SIZE * SECTOR_NUM_PER_SINGLE_PAGE)

#if((PAGE_BUFFER_SIZE == 1))
#define PAGE_BUFFER_SECT_NUM                            (SECTOR_NUM_PER_SINGLE_PAGE)
#endif

#if((PAGE_BUFFER_SIZE == 2))
#define PAGE_BUFFER_SECT_NUM                            (SECTOR_NUM_PER_SINGLE_PAGE * NandStorageInfo.PlaneCntPerDie)
#endif

/*logic page must located in corresponding bank if 1. for example: 2 bank , page (0,2,4...) - bank0, page(1,3...)-bank1 */
#define SUPPORT_ALIGN_BANK 					(0)// 1

/*restore  data from  bad block before delete it if 1*/
#define SUPPORT_BAD_BLOCK_DATA_SAVE			(1)

#define PAGE_COPYBACK_BUFFER                            (NandDevInfo.PageCache)
#define LOGIC_READ_PAGE_CACHE                           (NandDevInfo.PageCache)
#define LOGIC_WRITE_PAGE_CACHE                          (NandDevInfo.PageCache)
#define NAND_PAGE_CACHE                                 (NandDevInfo.PageCache)
#define PHY_TMPBUF     (NandDevInfo.PhyCache)



//#define PHY_DBG_MESSAGE_ON
#ifdef PHY_DBG_MESSAGE_ON
#define	    PHY_DBG(x...)           PRINT(x)
#else
#define     PHY_DBG(x...)
#endif

#define PHY_ERR_MESSAGE_ON
#ifdef PHY_ERR_MESSAGE_ON
#define     PHY_ERR(x...)           PRINT(x)
#else
#define     PHY_ERR(x...)
#endif

#define FTL_ERR_MESSAGE_ON
#ifdef FTL_ERR_MESSAGE_ON
#define     FTL_ERR(x...)           PRINT(x)
#else
#define     FTL_ERR(x...)
#endif

//#define FTL_DBG_MESSAGE_ON
#ifdef FTL_DBG_MESSAGE_ON
#define     FTL_DBG(x...)           PRINT(x)
#else
#define     FTL_DBG(x...)
#endif

//#define INIT_DBG_MESSAGE_ON
#ifdef INIT_DBG_MESSAGE_ON
#define     INIT_DBG(x...)          PRINT(x)
#else
#define     INIT_DBG(x...)
#endif

#define INIT_ERR_MESSAGE_ON
#ifdef INIT_ERR_MESSAGE_ON
#define     INIT_ERR(x...)          PRINT(x)
#else
#define     INIT_ERR(x...)
#endif

//#define INIT_BUG_DRV
#ifdef INIT_BUG_DRV
#define     INIT_DRV(x...)          PRINT(x)
#else
#define     INIT_DRV(x...)
#endif


/*  */
#define ZONE_TBL_MSG   		        (0x01)
/*  */
#define ECC_READ_RECLAIM   	        (0x01)
/*  */
#define ECC_OVER_MODE       	    (0x01)
/*  */
#define ERASE_DEBUG_MSG  	        (0x00)



#define INIT_DBG_BOOT_ON
#ifdef INIT_DBG_BOOT_ON
#define     INIT_BOOT(x...)          printf(x)
#else
#define     INIT_BOOT(x...)
#endif

#define _Trace_NF_
#ifdef _Trace_NF_
#define     Trace_NF(x...)          printk(x)   //Delete 
#else
#define     Trace_NF(x...)
#endif



#endif//the end of #ifndef _FLASH_DRIVER_CONFIG_
