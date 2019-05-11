/*mtd
*********************************************************************************************************
*                                       NAND FLASH DRIVER MODULE
*                               (c) Copyright 2007, Actions Co,Ld.
*                                          All Right Reserved
* File    : Nand_Flash_Driver_Struct.h
* By      : nand flash group
* Version : V0.1
* Date    : 2007-6-11 10:30
*********************************************************************************************************
*/

#ifndef _NAND_FLASH_DRIVER_STRUCT_H_
#define _NAND_FLASH_DRIVER_STRUCT_H_

#include "flash_driver_config.h"


#define PLATFORM_IS_32BIT
#define _NEW_ZONE_TBL_     (0)
#define NAND_SHARE_PAGES (1)
//#define Mbrec_Support_BCH24


#include <am_types.h>
#if __KERNEL__
#include "linux/init.h"
#include "linux/module.h"
#include <linux/gfp.h>
#include <linux/string.h>
#endif
#define ECC_CORRECT_MODE    (defined(__KERNEL__)  ||BREC_DRIVER == 0x01  ) 

///    //                             && ((CHIP_TYPE==AM_CHIP_7331)||AM_CHIP_ID==1207) )
#define REPEAT_READ_CNT             0x03      
#if CONFIG_AM_CHIP_ID  ==1211
#define User_Data_2Byte   (0x01)
#else
#define User_Data_2Byte   (0x00)
#endif
//#define Flash_PAGE_RW_BCH24      (0x00)
//#define Flash_PAGE_RW_BCH8      (0x00)

#define Total_Page_READERR          0x00

#define Flash_PAGE_R_BCH24          (0x00)
#define Flash_PAGE_W_BCH24          (0x00)

#define Flash_PAGE_R_BCH8           (0x00)
#define Flash_PAGE_W_BCH8           (0x00)

//测试功能函数入口
#define _TEST_HW_PageCTL_           (0x00)

#define SPECIAL_DMA_Test            (0)
#define BUS_DMA_BlockMode_Test      (0)
#define PSEUDO_NOISE_TEST           (0)


#define DMA_IRQ_MODE                (0x00)
#define EIP_IRQ_MODE                (0x01)
#define TEST_RB_CTL                 (0x00)

//@fish add 2011-02-21 测试Mbrec,Brec code 由MPUpdate tool 操作。
#define _BOOT_DATA_WR_				(0x01)
/* define result value */
/* ================================================================ */
#define NAND_OP_TRUE    (0)
#define NAND_OP_FALSE   (1)

#undef  TRUE
#undef  FALSE
#define TRUE            (0)
#define FALSE           (1)

//#define NULL            (0)
/* ================================================================ */


#define FLASH_DMA_IRQ               (0x02)
#define FLAH_EIP_IRQ		        (0x01)
#define Flash_DMA_Loop              (0x00)


/* define management data structure for nand driver */
/* ================================================================================================================== */
struct NandDev{
    struct StorageInfo          *NandFlashInfo;       //nand system information
    struct ZoneInfoType         *ZoneInfo;            //zone information
    struct ZoneTblCacheInfo     *ZoneTblCacheInfo;    //the zone table cache information
    struct PageTblCacheInfo     *PageTblCacheInfo;    //the page mapping table cache information
    struct SpecialCmdType       *SpecialCommand;      //nand flash multi-plane program command
    struct LogicOrganizeType    *LogicOrganizePar;    //logic organise parameters
    struct BitMapInfo		*BITMAP_DATA;
    void        *PageCache;                           //the pointer to page buffer
    void        *PhyCache;                            //
    void        *TestBuf;                             //
};

struct StorageInfo{
    INT8U       ChipEnableInfo;        //chip connect information
    INT8U       ChipCnt;               //the count of chip current connecting nand flash
    INT8U       BankCntPerChip;        //the banks count in chip
    INT8U       DieCntPerChip;         //the die count in a chip
    INT8U       PlaneCntPerDie;        //the count of plane in one die
    INT8U       SectorNumPerPage;      //page size,based on 0.5k
    INT16U      PageNumPerPhyBlk;      //the page number of physic block
    INT16U      TotalBlkNumPerDie;     //total number of the physic block in a die
    INT16U      OperationOpt;          //the operation support bitmap
    INT8U       FrequencePar;          //frequence parameter,may be based on xMHz
    INT8U       NandChipId[5];         //nand flash id
    INT8U       Reclaim;
    INT16U     	SpareSize;		       //增加冗余数目
    INT16U      CodeDataBlk;            //system code block number 
    INT32U      NAND_CTLREG;            //for Save nand ctl register 
    INT8U       Set_Reclaim;            //Set recalim value
    INT8U       BootReclaim;            //boot data reclaim value 
#ifdef PLATFORM_IS_32BIT
    INT8U       Reserved[1];            //Reserved
#endif
};


struct ZoneTblCacheType{
    INT8U       ZoneNum;                 //the number of the zone which is cached in ram
    INT8U       LogBlkNum;              //the number of Log blocks in current cached table
    INT8U       FreeBlkNum;             //the number of free blocks in current cached table
    INT8U       TblNeedUpdate;          //the flag which marked if the table need be updated to nand
    INT16U      AccessCount;
    struct DataBlkTblType       *DataBlkTbl;     //the pointer to data block table
    struct LogBlkTblType        *LogBlkTbl;      //the pointer to Log block table
    struct DataBlkTblType       *FreeBlkTbl;     //the pointer to Free block table
};

struct ZoneTblCacheInfo{
    struct ZoneTblCacheType     *CurZoneTbl;            //the pointer to the current operation zone table
    struct ZoneTblCacheType     ZoneTblCache[ZONE_CACHE_NUM];       //the array of the zone table
};


struct PageMapTblCacheType{
    INT8U       TblNeedUpdate;          //the flag which marked if the page mapping table need be updated to nand
    INT8U       ZoneNum;                //the zone number of the page mapping table belonged to
    INT16U      LogicBlkNum;           //the logic block number in zone of the page mapping table belonged to
    INT16U      AccessCount;
    struct PageMapTblType       *PageMapTbl;  //the pointer to page mapping table
};

struct PageTblCacheInfo{
    struct PageMapTblCacheType  *CurPageTbl;                        //the pointer to current operation pagemaptbl
    struct PageMapTblCacheType  PageMapTblCache[LOG_CACHE_NUM];     //the array of the pagemaptbl
};

struct LogicOrganizeType{
    INT16U      DataBlkNumPerZone;        //the number of data blocks in one zone
    INT16U      PageNumPerLogicBlk;       //the page number of logic block
    INT8U       SectorNumPerLogicPage;    //the sector number per super page
    INT8U       ZoneNumPerDie;            //the zone number per die
    #ifdef  PLATFORM_IS_32BIT
    INT8U       Reserved[2];              //reserved
    #endif
};
struct BitMapInfo{
    INT32U      Sector_BitmapBBF;       // 3Byte Bad_Flag &&  LogicBlkInfo Info  SECTOR_BITMAP_BBF
    INT32U      Sector_BitmapSpare;	    // 6Byte  Bad_Flag && Logicblkinfo && SECTOR_BITMAP_SPAREDATA
    INT32U      Single_BitmapSpare;	    //
    INT8U       USER_DataByte;	        //  IC 1203 3Byte, IC 1211 2userBata 
    INT8U       USER_SectorCnt;	        // total Sector
    INT8U       TOTAL_SPAREDATA;	
};
//表示ZoneTable Block Count 
#define ZONETBL_BLK_CNT             3 
//#define MAX_ZONETBL_AREA_BLOCK  48   /* zone table area is at the first 48 blocks in die */
///#define MAX_ERASE_BLK   7



#if (_NEW_ZONE_TBL_ ==0x01)
#define MAX_ERASE_CNT           (0xFFFF)
#define ADD_ERASE_CNT           (0x40)
#else
#define MAX_ERASE_CNT           (0xF) //Erase Cnt bit 4
#endif

#define ZONEBLK_BAKCNT    1 

#define DEFAULT_DATA1           0x55AA5501
#define DEFAULT_DATA2           0x55AA5502
#define DEFAULT_DATA3           0x55AA5503
#define DEFAULT_PhyBlk          0xFFFF
struct ZoneInfoType{
    INT16U                      PhyBlkNum;                          //the block No. whice saved zone table in a chip
    INT16U                      TablePosition:12;                   //the page No. which saved zone table
    INT16U                      Reserved:4;                         //reserved

};

#define ZONETBL_SIZE            (0x04) 
#define ZONE_Type               (INT32U)

#define  ZONETBL_OFFSET         (0x00)
#define  ZONETBL_OFFSET_BACK_UP         (0x01)

#define  LOGTBL_OFFSET          (0x02)
#define  DIRTYFlag_OFFSET       (0x03)


#define Page_Size2K             (4)
#define TBL_CHKSUM_OFF_4K       (1023*4)
#define TBL_CHKSUM_OFF_2K       (1023*2)
#define TABLE_CHKSUM_OFFSET		(1023*2)
#define  ZONE_TBL_SIZE_4K       (1024*4)
#define  ZONE_TBL_SIZE_2K       (1024*2)

#define ZONE_TLB_2K             (1)
#define ZONE_TLB_4K             (0)
//@fish  20110209 delete reserved 把EraseCnt 从原来的3Bit 改成用4Bit 表示减少对于
//不稳定块Erase Count
#if _NEW_ZONE_TBL_ ==0x01
struct DataBlkTblType{
    INT32U      PhyBlkNum:12;   //the physical block No. which mapping to the logical block
    INT32U      EraseCnt:20;    //erase count of the physic block                 
};
#else
struct DataBlkTblType{
    INT16U      PhyBlkNum:12;   //the physical block No. which mapping to the logical block
    INT16U      EraseCnt:4;     //erase count of the physic block
};
#endif

struct  _Debug_Wear{
    INT32U      wEraseBlkCnt;   //total block erase count
    INT32U      Wear_LevelCnt;  //wear leveling count
    INT32U      SimpleMergeCnt; //
    INT32U      SwapMergeCnt;   //
    INT32U      MoveMergeCnt;   //
    INT32U      Error_Byte;     //total Error byte count
    INT32U      Error_Bit;      //total error bit count 
 };

 
struct LogBlkTblType{
    INT16U      LogicBlkNum;            //the logic block number of the log block
    struct DataBlkTblType       PhyBlkNum;  //the physic block number of the log block information
    INT16U      LastUsedPage;           //the number of the page which used last time
};
#define ECC_BLK_NUM             (256)
#define Phy_BLK_DEF             (0xFFFF)
#define Phy_Zone_DEF            (0xFF)
#define ECC_ERR_NUM             (32)
#define BCH_ERR_TOTALBIT        (0x78) //120    

#define BCH_CLEAR_VAL           (0x00)
#define BCH_SET_VAL             (0x01)
#define BCH_GET_VAL             (0x02)

struct DataBlkECCInfo{
    INT16U      PhyBlkNum;      //ECC Physcial Block Num
    INT16U      ECC_ErrNum;     //
    INT8U       ZoneNum;        //
    INT8U       ECC_ErrCnt;     //
};
struct ReClamBlkInfo{
       struct DataBlkECCInfo   BlkInfo[ECC_BLK_NUM];
       INT16U    FreeIndexPst;   
};
struct PageMapTblType{
    INT16U      PhyPageNum:12;  //the physic page number for the logic page
    INT16U      Reserved:4;     //reserved
};

struct LogicBlkInfoType{
    INT16U      LogicBlkNum:10; //logic block number
    INT16U      ZoneInDie:4;    //zone number in die
    INT16U      Reserved:1;     //reserved
    INT16U      BlkType:1;      //block type, brec code block, table block or data block
};


struct SpecialCmdType{
    INT8U       MultiProgCmd[2];       //the command for multi-plane program
    INT8U       MultiCopyReadCmd[3];   //the command that read for multi-plane copyback
    INT8U       MultiCopyProgCmd[3];   //the command that program for multi-plane copyback
    INT16U      MultiBlkAddrOffset;    //the offset of two blocks for multi-plane
    INT8U       BadBlkFlagPst;         //the bad block flag position,//0x00(1st page),0x01(1&& 2 page),0x02(last page),0x03(last 2 page)
    INT8U       ReadMultiOpStatusCmd;  //the command for multi-plane operation
    INT8U       InterChip0StatusCmd;   //the command to read inter-chip0 status for interleave
    INT8U       InterChip1StatusCmd;   //the command to read inter-chip1 status for interleave
    #ifdef  PLATFORM_IS_32BIT
    INT8U       Reserved[2];            //reserved
    #endif
};



#define BAD_BLK_FLAG            (0xFF)
#define PAGE_STATE_FREE         (0xff)// user 2byte
#define PAGE_STATE_USED         (0x55)// user 2byte
#define PAGE_STATE_PMTBL        (0xaa)// user 2byte


#define SPARE_DATA_SIZE         (NandDevInfo.BITMAP_DATA->TOTAL_SPAREDATA)
#define SP_USE_BYTE             (NandDevInfo.BITMAP_DATA->USER_DataByte)
#define SP_SECTOR_CNT           (NandDevInfo.BITMAP_DATA->USER_SectorCnt)

#define SECTOR_BITMAP_BBF       (NandDevInfo.BITMAP_DATA->Sector_BitmapBBF)
#define SECTOR_BITMAP_SPAREDATA (NandDevInfo.BITMAP_DATA->Sector_BitmapSpare)
#define SINGLE_BITMAP_SPAREDATA (NandDevInfo.BITMAP_DATA->Single_BitmapSpare)
struct BootBlk_Info{
    INT8U       name[4]; // mbrc or brec
    INT8U       blknum; // blocknum 0 1,2,3 ....
    INT8U       check; // check sum fail or ok
    INT16U      Flag; //0x55 aa 
    INT16U      Flashchecksum; //flash IC checksum
    INT16U      Calchecksum;  // 计算checksum ;
    INT8U       ECC_Bit_Max;    //boot code  each sector max error ECC bit ; 
    INT16U      ECC_Bit_Total;  //boot code  total Error ECC bit
    INT16U		ECC_Err_Cnt;    //boot code toal sector count 
    INT8U       RecalaimVal;    //read recalim 根据不同flash ID此值不相同。
    INT8U       BCHType;        //BCH8 value 8, BCH12 value 12 ....
    INT8U       rev[12];        //reserved 
};


union LogPageStatus{
    INT8U       LogBlkAge;      //log block age for update sequence
    INT8U       PhyPageStatus;  //page status, 0xff-EmptyPage/0x55-UserDataPage/0xaa-PageMappingTblPage
};
struct SpareData0{
    INT8U       BadFlag;        //bad block flag
    INT16U      LogicBlkInfo;   //logic block information for mapping manage
}  __attribute__((packed));
struct SpareData1{
    INT16U      LogicPageNum;   //the logic page number which this physic page mapping to
    union LogPageStatus     LogStatus;  //log status, may be LogBlkAge or PhysicPageStatus
}  __attribute__((packed));

struct SpareData{
    struct SpareData0       SpareData0;
    struct SpareData1       SpareData1;
};


struct NandSpareData{
    struct SpareData        UserData[2];    //the spare area for sector0

};
/* ================================================================================================================== */


/* ===================================================================================================================*/
struct PhysicOpParameter{
    INT8U   BankNum;        //Bank number
    INT16U  PageNum;        //page number in physic block,the page is based on multi-plane if support
    INT16U  PhyBlkNumInBank;    //physic block number in bank,the physic block is based on multi-plane if support
    INT32U  SectorBitmapInPage; //page sector bitmap
    void    *MainDataPtr;       //pointer to main data buffer
    void    *SpareDataPtr;      //pointer to spare data buffer
};

struct LogicPagePar{
    INT16U      LogicBlkNumInZone;     //the logic block number in zone which this global page belonged to
    INT16U      LogicPageNum;          //the page number in block
    INT32U      SectorBitmap;          //the bitmap of the page data
    INT8U       ZoneNum;               //the zone number which this global page number belonged to
#ifdef  PLATFORM_IS_32BIT
    INT8U       Reserved[3];           //reserved
#endif
};


struct AdaptProcPagePar{
    INT32U      PageNum;                //the page number of the page that the adaptor processing page
    INT32U      SectorBitmap;           //the sector bitmap of the page that the adaptor processing page
#ifdef  PLATFORM_IS_32BIT
    INT8U       Reserved[2];            //reserved
#endif
};


struct NandRWPar{
    INT32U          lba;                //global logical secter No.
    INT16U          length;             //read or write sector number
    INT8U           reserved0;          //reserved0,different use for different platform
    INT8U           reserved1;          //reserved1
    void            *sramaddr;          //sram address
};

struct NandLogicCtlPar{
    INT8U          LastOp;                      //last operation type,read or write
    INT8U          ZoneNum;                     //the number of the last access zone
    INT16U         SuperBlkNum;                 //the number of the logic block last acess
    INT16U         SuperPageNum;                //the number of the page last operation
    INT16U         LogPageLastPageNum;          //the page number of current log page last position
    struct DataBlkTblType       LogPageLastBlk; //the block number of current log page last position
    INT32U         LastWBufPageNum;             //last write buffer page number
    struct DataBlkTblType       PhyDataBlk;     //the super physic block number of data block
    struct DataBlkTblType       PhyLogBlk;      //the super physic block number of log block
    INT16U         LogPageNum;                  //last operate page number in log block
    INT32U         LogPageBitmap;               //the bitmap of the log page last write
    INT8U          LogBlkAccessAge[MAX_LOG_BLK_NUM];   //record every log block access time,for merge
    INT8U          AccessTimer;                 //the timer for log page write access,for merge
};

struct NandDevPar{
    INT8U       NandID[4];                          //nand flash id
    INT8U       DieCntPerChip;                      //the die count in a chip
    INT8U       SectNumPerPPage;                    //page size,based on 0.5k    
    INT16U      PageNumPerPBlk;                     //the page number of physic block//@fish modify INT8U-->INT16U  2010-04-30 
    INT8U       Frequence;                          //frequence parameter,may be based on xMHz
    INT16U      BlkNumPerDie;                       //total number of the physic block in a die
    INT16U      DataBlkNumPerZone;                  //the number of data blocks in one zone
    INT32U      OperationOpt;                       //the operation support bitmap
    INT8U       Frequence_N;                        //new Frequence @fish add 
    INT8U	    Reclaim;
    INT16U      SpareSize;
    struct SpecialCmdType *SpecOp;                  //special operation struct
};
struct ECC_proetect_Read
{
    INT32U      CodeSize;
    INT8U       bCodeArea;
    INT8U       MultiMbrcFlag;
    INT8U       MBrcBlk[4];       
 };
/* ================================================================================================================== */

/* sector bitmap defines */
#define GenSectorBitMap(lbitmap, rbitmap) \
	((lbitmap) | ((rbitmap) << SECTOR_NUM_PER_SINGLE_PAGE))

/* ================================================================================================================== */
/* define the page status */

#define DATA_PAGE_FLAG              0x55
#define TABLE_PAGE_FLAG             0xaa
#define EMPTY_PAGE_FLAG             0xff

/* define the page mapping table need update flag */
#define NEED_UPDATE_PAGE_MAP_TBL    0x01
/* define get the free block type, lowest erase count or highest erase count */
#define LOWEST_EC_TYPE              0x00
#define HIGHEST_EC_TYPE             0x01

/* define merge type */
#define NORMAL_MERGE                0x00        /* normal merge type, can't do move merge */
#define SPECIAL_MERGE               0x01        /* special merge type, can do move merge */
/* ================================================================================================================== */



/* ================================================================ */
//Operation Options Bitmap
#define     PAGE_CACHE_PROGRAM          0x0001
#define     PAGE_CACHE_READ             0x0002
#define     MULTI_PAGE_WRITE            0x0004
#define     MULTI_PAGE_READ             0x0008
#define     PAGE_COPYBACK               0x0010
#define     INTERNAL_INTERLEAVE         0x0020
#define     EXTERNAL_INTERLEAVE         0x0040
#define     SMALL_BLOCK                 0x0080

/*add onlyfish 2009-08-07 StorageInfo->OperationOpt  Bit [13:15],  ECC bit
000===> 1Bit/4Bit/8Bit  
001 ===>12Bit  
010 ===>24Bit 
011-->reserved   */
#define ECC_Type_8BitECC      0x0000
#define ECC_Type_12BitECC     0x2000
#define ECC_Type_24BitECC     0x4000
#define ECC_Type_40BitECC     0x6000
#define ECC_Type_60BitECC     0x8000
//@fish 支持
#define PN_Randomizer	      (0x0800)

//nand status define
#define     NAND_OPERATE_FAIL           0x0001
#define     NAND_CACHE_READY            0x0020
#define     NAND_STATUS_READY           0x0040
#define     NAND_WRITE_PROTECT          0x0080
/* ================================================================ */
#define     MAX_FREE_BLK_CNT            0x0c

/* ================================================================ */
//Error type define
#define     PHYSICAL_LAYER_ERROR        0x1000
#define     LOGICAL_LAYER_ERROR         0x2000
#define     MAPPING_MODULE_ERROR        0x4000

#define     BUFFER_ADDR_ERROR           0x01
#define     DATA_ECC_ERROR              0x02
#define     OPERATE_ERROR               0x03
#define     TIMEOUT_ERROR	            0x04
#define     DATA_ECC_0xFF               0x100
#define     DATA_ECC_LIMIT              0x80
#define     DATA_ECC_LIMIT2             0x90

#define     PHY_READ_ERR_MASK           0x7f

#define     PHY_LAYER_TIMEOUT_ERR       (PHYSICAL_LAYER_ERROR | TIMEOUT_ERROR)
#define     PHY_LAYER_BUF_ERR           (PHYSICAL_LAYER_ERROR | BUFFER_ADDR_ERROR)
#define     NAND_DATA_ECC_ERROR         (PHYSICAL_LAYER_ERROR | DATA_ECC_ERROR)
#define     NAND_DATA_ECC_LIMIT         (PHYSICAL_LAYER_ERROR | DATA_ECC_LIMIT)
#define     NAND_DATA_ECC_0xFF          (PHYSICAL_LAYER_ERROR | DATA_ECC_0xFF)
#define     NAND_OP_ERROR               (PHYSICAL_LAYER_ERROR | OPERATE_ERROR)
#define     NAND_DATA_ECC_LIMIT2        (PHYSICAL_LAYER_ERROR | DATA_ECC_LIMIT2)

#define     PHY_READ_ERR(err)           ((err) & PHY_READ_ERR_MASK)
#define     PHY_READ_ECC_TO_LIMIT(err)  ((err) & DATA_ECC_LIMIT)
#define     PHY_READ_ECC_ALLFF(err)     ((err) & DATA_ECC_0xFF)


static int T_a = 555;

/* ================================================================================================================== */

#define PHY_TESTBUF     (NandDevInfo.TestBuf)

/* ================================================================================================================== */

/* define the full bitmap of the single page */
#define FULL_BITMAP_OF_SINGLE_PAGE          ((INT32U)((1ULL<<NandStorageInfo.SectorNumPerPage)-1))
/* define the full bitmap of the super page */
#define FULL_BITMAP_OF_SUPER_PAGE           ((INT32U)((1ULL<<SECTOR_NUM_PER_SUPER_PAGE) - 1))
/* define the full bitmap of the buffer page */
#define FULL_BITMAP_OF_BUFFER_PAGE          ((INT32U)((1ULL<<PAGE_BUFFER_SECT_NUM) - 1))
/* the multi block offset */
#define MULTI_BLOCK_ADDR_OFFSET             (NandDevInfo.SpecialCommand->MultiBlkAddrOffset)
/* the bad block flag position */
#define BAD_BLOCK_FLAG_PST                  (NandDevInfo.SpecialCommand->BadBlkFlagPst)
/* check if nand flash can suport internal interleave */
#define NAND_SUPPORT_INTERNAL_INTERLEAVE    (INTERNAL_INTERLEAVE & NandStorageInfo.OperationOpt)
/* check if nand flash can suport external interleave */
#define NAND_SUPPORT_EXTERNAL_INTERLEAVE    (EXTERNAL_INTERLEAVE & NandStorageInfo.OperationOpt)
/* check if nand flash can support multi-plane operation */
#define NAND_SUPPORT_MULTI_PLANE            (MULTI_PAGE_WRITE & NandStorageInfo.OperationOpt)

/* check if nand flash can support multi-plane operation */
#define NAND_SUPPORT_MULTI_PLANE_R            (MULTI_PAGE_READ & NandStorageInfo.OperationOpt)
/* check if nand flash can support cache program operation */
#define NAND_SUPPORT_CACHE_PROGRAM          (PAGE_CACHE_PROGRAM & NandStorageInfo.OperationOpt)
/* check if nand flash can support copy back operation */
#define NAND_SUPPORT_PAGE_COPYBACK          (PAGE_COPYBACK & NandStorageInfo.OperationOpt)

/* check if flash is small block */
#define NAND_IS_SMALL_BLOCK                 (SMALL_BLOCK & NandStorageInfo.OperationOpt)
/*support randomizer function **/
#define NAND_SUPPORT_PN		((PN_Randomizer & NandStorageInfo.OperationOpt))

extern struct NandDev NandDevInfo;
extern struct StorageInfo          NandStorageInfo;
extern struct SpecialCmdType       SpecialCommand;
extern struct LogicOrganizeType    LogicOrganisePara;
extern struct ZoneInfoType         ZoneInfo[NAND_MAX_DIE_NUM * MAX_ZONE_NUM_IN_DIE];
extern struct ZoneTblCacheInfo     ZoneTblCache;
extern struct PageTblCacheInfo     PageMapTblCache;

#define ZONE_NUM_PER_DIE        (NandDevInfo.LogicOrganizePar->ZoneNumPerDie)
#define DIE_CNT_PER_CHIP        (NandStorageInfo.DieCntPerChip)
#define BNK_CNT_PER_CHIP        (NandStorageInfo.BankCntPerChip)
#define TOTAL_BNK_CNT           (NandStorageInfo.ChipCnt * NandStorageInfo.BankCntPerChip)
#define BLK_CNT_PER_DIE         (NandStorageInfo.TotalBlkNumPerDie)
#define PLANE_CNT_PER_DIE       (NandStorageInfo.PlaneCntPerDie)
#define PAGE_CNT_PER_LOGIC_BLK  (NandDevInfo.LogicOrganizePar->PageNumPerLogicBlk)
#define PAGE_CNT_PER_PHY_BLK    (NandStorageInfo.PageNumPerPhyBlk)

#define DATA_BLK_NUM_PER_ZONE   (NandDevInfo.LogicOrganizePar->DataBlkNumPerZone)

#define CUR_DATA_BLK_TABLE      (ZoneTblCache.CurZoneTbl->DataBlkTbl)
#define CUR_LOG_BLK_TABLE       (ZoneTblCache.CurZoneTbl->LogBlkTbl)
#define CUR_FREE_BLK_TABLE      (ZoneTblCache.CurZoneTbl->FreeBlkTbl)

#define CUR_ZONE_NUM            (ZoneTblCache.CurZoneTbl->ZoneNum)

#define CODE_DATA_BLK           (NandDevInfo.NandFlashInfo->CodeDataBlk)
#define CUR_PAGE_MAP_TABLE      (PageMapTblCache.CurPageTbl->PageMapTbl)
#define CUR_PAGE_MAP_CACHE      (PageMapTblCache.CurPageTbl)


//@fish add 
//@fish add 2010-07-22 BCH12,BCH24
#define BCH_MODE                  (NandStorageInfo.OperationOpt&0xE000)
#define RECLAIM_VAL				  (NandStorageInfo.Reclaim)
#define PAGE_SPARESIZE            (NandStorageInfo.SpareSize)
#define SET_READ_Reclaim          (NandStorageInfo.Set_Reclaim)


//=============================================================================
#define ENTRY_START_CMD     (0x40)
#define GET_Block_Info		(ENTRY_START_CMD+0x00)//0x40
#define PHY_ReadBlock       (ENTRY_START_CMD+0x01)//0x41
#define PHY_WriteBlock		(ENTRY_START_CMD+0x02)//0x42
#define Log_ReadBlock		(ENTRY_START_CMD+0x03)//0x43
#define Log_WriteBlock		(ENTRY_START_CMD+0x04)//0x44
#define PHY_EraseBlk		(ENTRY_START_CMD+0x05)//0x45
#define PHY_ReadBootBlk		(ENTRY_START_CMD+0x06)//0x46
#define PHY_WriteBootBlk	(ENTRY_START_CMD+0x07)//0x47
#define PHY_ReadWholePage   (ENTRY_START_CMD+0x08)//0x48
#define PHY_WriteWholePage  (ENTRY_START_CMD+0x09)//0x49

#define PHY_CHECK_BOOTDATA  (ENTRY_START_CMD+0x0d)//0x4d



//mtm.c
extern INT32U  PMM_ExitPageMapTblCache(void);
extern INT32U  FTL_FlushPageCache(void);
extern INT32U  BMM_WriteBackAllMapTbl(void);
extern INT32U  BMM_ExitBlockMapTblCache(void);
extern INT32U  PMM_InitPageMapTblCache(void);
extern INT32U  BMM_InitBlkMapTblCache(void);

extern INT32U  FTL_Init(void);
extern INT32U  FTL_Param_Init(void);
extern INT32U  FTL_Read(INT32U Lba, INT32U Length, void* SramBuffer);
extern INT32U  FTL_Write(INT32U Lba, INT32U Length, void* SramBuffer);
extern INT32S  nand_flash_inithw(void);
extern INT32U  INIT_CreateBlkTbls(void);

//Oal/alloc.c
extern void    kmemcpy(INT8S* desti_buffer, INT8S* source_buffer, INT32S buffer_size);
extern void    kmemset(void* Buf, INT8U Value, INT32U Length);
extern INT32S  kmemcmp(const void * cs,const void * ct,INT32U count);
extern INT32U  Set_NF_ECCCTL(INT16U EccType);
extern void    start_dma(INT32S DMANR);
extern void    stop_dma(INT32S dmanr);
extern INT32S  bch_ecc_correct(INT8U* srambuffer, INT8U* oob_buf);

extern INT32U  Set_NAND_ECCCTL(INT16U EccType,INT8U Flag);
extern INT16U  _GetCheckSum(INT16U *buf,INT32U len);

extern INT32U  PHY_BCH_CNT(INT8U Flag,INT32U bTmp);
extern INT32U  Test_ReadMbrecData(INT32U BrecBlkNum,INT32U BrecSectNum, INT8U *Buf);
extern INT32U  PHY_Set_BCHRes(INT32U bResult);
extern INT32U  PHY_ReadRes(INT8U Flag,INT32U res);
extern INT32U  PHY_Get_BCHRes(void);

extern void    Init_ReClaimBlk_Info(void);
extern INT32U  _PHY_PageRead(struct PhysicOpParameter *NandOpPar);

extern void    Str_printf( const INT8U*  pad, const INT8U* pData, INT16U inLen);
extern void    Str_printf2( const INT8U*  pad, const INT8U* pData, INT16U inLen);
extern void    Dump_mem(void *startaddr, INT32S size, INT32U showaddr, INT32S show_bytes);
extern INT32S brec_sector_read(INT32U BrecBlkNum,INT32U BrecSectNum, INT8U *Buf);
extern INT32S brec_sector_write(INT32U BrecBlkNum,INT32U BrecSectNum, INT8U *Buf);
extern INT32U Entry_Phy_EraseBlk(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf);
extern INT32U Entry_Phy_ReadPage(INT32U Block,INT32U PageInSuBlk,INT32U Sector,INT8U *Buf);
extern INT32U Entry_Phy_WritePage(INT32U Block, INT32U PageInSuBlk, INT32U Sector, INT8U * Buf);
extern INT32U   ReadWrite_Entry(INT8U entrynum, INT32S lba, INT8U* buffer,INT32U Pagetmp);


#if __KERNEL__
static inline void *malloc(unsigned long size)
{  return kmalloc(size, GFP_KERNEL | GFP_DMA); }

static inline void free(void *ptr)
{  kfree(ptr); }

#define init_heap_for_nand()    do {} while (0)

void Flash_Disable_Irq(void);
void Flash_Enable_Irq(void);

INT32U Test_ReadCard_Data(void);
#else

extern void    dma_cache_wback_inv(INT32U addr, INT32U size);

#endif
void           Get_bPageCntVal(void);
extern unsigned int MBRC_SECTOR_NUM, MBRC_Total_Size;



extern INT8U bPrintFlag;
extern INT8U bPrintFlag2;
//@fish add 2010-08-24 
extern INT8U Flash_DMA_Mode ;
extern void _Short_DelayUS(INT32U DelayTim);
extern INT32U   _Get_TimeOut3(INT8U  Flag );

//
extern INT32S _dma_start(INT32U dmanr, INT32U cmd);
extern INT8U  flash_irq_init(void);
extern void    Nand_ReadConfig(INT32U nDmaNum, void* Mem_Addr, INT32U ByteCount);
extern INT32U nand_StartDma(INT32U nDmaNum);
extern INT32U Flash_Dmatimeout(void);
extern int  Isr_DMA_Flashdriver(int isr,void *id);
extern int  Isr_Flashdriver(int isr,void *id);
extern INT32U Flash_timeout(void);
extern void   Debug_Flash_Register(void);
extern void Print_msg(void);

extern void flash_irq_enable(void);
extern void flash_irq_disable(void);
extern void flash_clr_status(void);
extern INT32U  PHY_PageRead_Spare(struct PhysicOpParameter *NandOpPar);
extern INT32U _Pseudo_NoiseFunc(INT32U Flag);

extern void Set_TestBuf(void);

//#define ECC_ERR_MESSAGE_ON
#ifdef ECC_ERR_MESSAGE_ON
#define     ECC_Err(x...)          PRINT(x)
#else
#define     ECC_Err(x...)
#endif


#define DUG_WEAR_LEVELING
#ifdef DUG_WEAR_LEVELING
#define     BUG_Wear(x...)          PRINT(x)
#else
#define     BUG_Wear(x...)
#endif

#endif  //#ifdef _NAND_FLASH_DRIVER_STRUCT_H_

