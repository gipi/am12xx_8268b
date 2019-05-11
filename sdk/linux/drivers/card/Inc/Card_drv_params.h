#ifndef _CARD_DRV_PARAMS_H
#define _CARD_DRV_PARAMS_H
//#include "amTypedef.h"


#define		SD_PARTS 	8
#define 	SD_BLKDEV_DISKNAME		"sd_card_block"

#define		eMMC_PARTS 	8
#define 	eMMC_BLKDEV_DISKNAME	"nftl"

#define		MS_PARTS 	8
#define 	MS_BLKDEV_DISKNAME		"ms_card_block"

#define		XD_PARTS 	8
#define 	XD_BLKDEV_DISKNAME		"xd_card_block"

#define		CF_PARTS 	8
#define 	CF_BLKDEV_DISKNAME		"cf_card_block"



#define SELF_SET_DMA 		 (0x00)

#define CARD_RW_TRUE         (0x00)
#define CARD_RW_FALSE        (0x01)
#define CARD_RW_NOExist      (0x02)
#define CARD_WRITE_PROTECT   (0x03)
#define CARD_INIT_TRUE       (0x01)
#define CARD_INIT_FALSE      (0x00)

#define PAGE_TRANS_N
#define PAGE_TRANS_WRITE
#define CPU_RW    			(0x00)
#define DMA_RW  			(0x01)
#define SD_MODE             (DMA_RW) //CPU_RW//DMA_RW        //SD/MMC Card
#define MS_MODE         	(DMA_RW)    //ms standard  Card
#define XD_MODE         	(DMA_RW)   //DMA_RW   //XD Card
#define CF_MODE         	(DMA_RW)
#define eMMC_MODE			(DMA_RW)

#define DMA_DELAY   		 (0x100)


#define Loop_Mode     		 0x00
#define CRC_IRQ_Mode         0x01
#define DMA_IRQ_Mode         0x02
#define CRC_DMA_Finish       DMA_IRQ_Mode  //CRC_IRQ_Mode //CRC_IRQ_Mode //DMA_IRQ_Mode

#define CF_ReadWrite_Cnt  0x10 
#define SD_ReadWrite_Cnt  0x20
#define MS_ReadWrite_Cnt  0x20
#define XD_ReadWrite_Cnt  0x08 


#define SET_CF_SPECAL_DMA   	0x01
#define SET_SD_SPECAL_DMA	    0x01


#define SDCARD_TEST_IRQ         0x00
//@fish DMA 使用MMU
//#define Card_DMA_MMU

/**/
#define  Card_Reader_CTL_ON     (0x00)
#define  Card_Reader_CTL_OFF    (0x01)

#define Card_Type_None   	0x0
#define Card_Type_SD        0x01
#define Card_Type_MS        0x02
#define Card_Type_XD        0x04
#define Card_Type_CF        0x05
#define Card_Type_eMMC	    0x06



#define  SDDRV_VER         "SD Card Date:"__DATE__" Tim:"__TIME__
#define  MSDRV_VER         "MS Card Date:"__DATE__" Tim:"__TIME__
#define  XDDRV_VER         "XD Card Date:"__DATE__" Tim:"__TIME__
#define  CFDRV_VER         "CF Card Date:"__DATE__" Tim:"__TIME__
#define  CARDDET_VER       "Card    Data:"__DATE__" Tim:"__TIME__



/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+
+   end of AL1207 
+ 
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/***
* @breif fish add 2009-02-13*
*/
#define IS_PHY_ADDR_Card(addr)   ((addr&0xE0000000)>=0x80000000)
#define BUFF_ADDR_LOG   0x00
#define BUFF_ADDR_PHY   0x01



/***
* @breif fish add 2009-06-02
* 由于增加EMI相关测试，原来CLK 48MHZ过来，目前把
* CLK Soucre 默认改成24MHZ.
* @breif fish add 2009-06-19 把CLK 改回48MHZ,
*/


#define MAX_ACMD41_CNT   80
#define FOR_READBOY   (0)



#endif
