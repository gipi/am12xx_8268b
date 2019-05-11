#include "nand_flash_driver_type.h"

#define BMT (NandDevInfo.ZoneTblCacheInfo->CurZoneTbl)
#define DTBL (BMT->DataBlkTbl)			/*data block map table of current zone*/
#define LTBL (BMT->LogBlkTbl)			/*log block map table of current zone*/
#define FTBL (BMT->FreeBlkTbl)			/*free block map table of current zone*/

#define PMT	(NandDevInfo.PageTblCacheInfo->CurPageTbl)
#define L2P 	(PMT->PageMapTbl)		 /*page map table of current log block*/
#define CURZONE (BMT->ZoneNum) /*current accessed zone*/

#define PAGES_PER_SUPERBLK (NandDevInfo.LogicOrganizePar->PageNumPerLogicBlk)
#define DATABLKS_PER_ZONE DATA_BLK_NUM_PER_ZONE
#define FREEBLKS_PER_ZONE (BLK_NUM_PER_ZONE - 1 - DATA_BLK_NUM_PER_ZONE)
#define LOGBLKS_PER_ZONE MAX_LOG_BLK_NUM

#define BANK_TOTAl (PAGE_CNT_PER_LOGIC_BLK / PAGE_CNT_PER_PHY_BLK) 
#define TMPBUF NandDevInfo.PageCache

