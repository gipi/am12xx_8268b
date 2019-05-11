#ifndef _ACTIONS_FLASH_API_H_
#define _ACTIONS_FLASH_API_H_
/*
*@ mach-am7x/include/flash_api.h
*
*@author: onlyfish
*@date: 2010-07-12
*@version: 0.1
*/
/**
*******************************************************************************
* @file	flash_drv_api.h
* @brief  写该文件的简要说明
*
* 该文件名的详细说明,\n
* 第二行详细说明\n
* 第三行详细说明\n
* @version  Version 1.1
* @author: onlyfish
*@date: 2010-07-12
* @warning 写上关于警告的说明
* @bug 写上关于bug的说明
* @deprecated 写上过时的说明，比如说，该文件到什么时候就过时，没用了
* @todo 写上将要做的事
* @see 写上要参看的文件或者变量
* @ingroup nand_drv
********************************************************************************/
#include <linux/types.h>
#include <am_types.h>

#define MAX__PARTS   16 
#define NAME_SIZE_MAX  10
#define PART_Str_SIZE   128 
struct Flash_Partiton {
    char      partName[NAME_SIZE_MAX];
    uint32_t part_off;   /* LBA of first sector in the partition */
    uint32_t part_size;  /* number of blocks in partition */
}__attribute__ ((packed));//,aligned(4)


/**
* @addtogroup nand_drv
* @{
*/

/*driver info*/
/**
* @brief 功能：写磁盘对应sectors数据
*
* @param:offset：写起始offset地址，
*
* @param:buffer   写数据缓冲
*
* @param:sectors 写sectors数目，sector为逻辑单位，典型值为512
* 
*
* @param:"rootfs","udisk","vram"                    
* 
*	                        
* @return 
*         若写失败，返回失败err code，若成功返回写的实际sectors数目。  
*             
*/
INT32S flash_write_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part );

/**
* @brief 功能：读磁盘对应sectors数据
*
* @param:offset：读起始offset地址，
*
* @param:buffer   读数据缓冲
*
* @param:sectors 读sectors数目，sector为逻辑单位，典型值为512
* 
*
* @param:"rootfs","udisk","vram"                    
* 
*	                        
* @return 
*         若写失败，返回失败err code，若成功返回写的实际sectors数目。  
*             
*/
INT32S flash_read_sectors(INT32U offset, INT8U *buffer, INT32U sectors,INT8U *Part );
/**
* @brief 功能：返回各个区容量信息
* 
*
* @param:"rootfs","udisk","vram"                    
* 
*	                        
* @return :
*        partition Capacity(以Sector为单位) 
*             
*/
INT32S flash_getcapacity(INT8U *Part );


/*
*@brief 功能：
*        从Flash的指定的lba位置读出一个扇区的数据到sramaddress
*        
*@param：lba           逻辑地址
*                      
*@param：sramaddress   数据在RAM中的首地址指针
* 
*@param：SecCnt  ,表示传入读取Sector数目
*

*@return:0     - read ok
*        非0   - read error
*                1 -   LBA ERROR
*                2 -   FLASH ECC ERROR  
*
*/
INT32S nand_adfu_read(INT32U lba, INT8U* sramaddress,INT32U SecCnt);


/*
*@brief 功能：
*        将sramaddress的数据写入到Flash的lba处
*        
*@param：lba           逻辑地址
*                      
*@param：sramaddress   数据在RAM中的首地址指针
* 
*@param：SecCnt  ,表示传入读取Sector数目
*

*@return:0     - write ok
*        非0   - write error
*                1 -   LBA ERROR
*                2 -   FLASH ECC ERROR  
*
*/
INT32S nand_adfu_write(INT32U lba, INT8U* sramaddress,INT32U SecCnt);

/*
*@brief 功能：
*        用于U盘下ADFU升级时读取物理区间上的Mbrec和Brec代码，
*        为了使ADFU不关心具体Flash的内部块组织结构，对Driver
*        内部的物理读写作一个封装，使得ADFU能够以简得的参数
*        方便的读写物理区间上的代码.
*
*@param：Brec_Block_Num - ADFU角度看到的块号，Mbrec占用四个块，
*                           每一份占用一个块；Bbrec每份占用一个块，
*
*@param：Brec_Sector_Num - ADFU角度看到的扇区号，每份Mbrec或
*                          Brec代码内部的相对偏移扇区数。
*
*@return .  读取的结果，返回值为0表示读取成功。      
*
*/
INT32S brec_sector_read(INT32S Brec_Block_Num,INT32S Brec_Sector_Num,INT8S *Buffer_Addr);


/*
*@brief 功能：
*        用于U盘下ADFU升级时写物理区间上的Mbrec和Brec代码，
*        为了使ADFU不关心具体Flash的内部块组织结构，对Driver
*        内部的物理读写作一个封装，使得ADFU能够以简得的参数
*        方便的读写物理区间上的代码.
*@param：Brec_Block_Num - ADFU角度看到的块号，Mbrec占用四个块，
*                           每一份占用一个块；Bbrec每份占用一个块，
*@param：Brec_Sector_Num - ADFU角度看到的扇区号，每份Mbrec或
*                          Brec代码内部的相对偏移扇区数。
*
*@param：Buffer_Addr     - 数据Buffer指针。
*
*@return:写入的结果，返回值为0表示写入成功。    
*
*/
INT32S brec_sector_write(INT32S Brec_Block_Num,INT32S Brec_Sector_Num,INT8S *Buffer_Addr);

/*
*@brief function：get flash type 
*
* @param：null
*
* @return .  nand flash type   
*            0x38343646 - string "F648"  
*
*/
int Flash_GetPartNum(struct Flash_Partiton *Fparts);


INT32U get_nand_flash_type(void);

#ifndef DEFINE_StorageInfo
#define PLATFORM_IS_32BIT
struct StorageInfo{
    INT8U                       ChipEnableInfo;                     //chip connect information
    INT8U                       ChipCnt;                            //the count of chip current connecting nand flash
    INT8U                       BankCntPerChip;                     //the banks count in chip
    INT8U                       DieCntPerChip;                      //the die count in a chip
    INT8U                       PlaneCntPerDie;                     //the count of plane in one die
    INT8U                       SectorNumPerPage;                   //page size,based on 0.5k
    INT16U                      PageNumPerPhyBlk;                   //the page number of physic block
    INT16U                      TotalBlkNumPerDie;                  //total number of the physic block in a die
    INT16U                      OperationOpt;                       //the operation support bitmap
    INT8U                       FrequencePar;                       //frequence parameter,may be based on xMHz
    INT8U                       NandChipId[5];                      //nand flash id
    #ifdef PLATFORM_IS_32BIT
    INT8U                       Reserved[2];                        //Reserved
    #endif
    INT16U            CodeDataBlk;
};
#endif
/*
*@brief function：get NAND FLASH DRIVERS Strage information for 
     chip id,Block size,page size 
*
* @param：null
*
* @return .   
*           
*
*/
struct StorageInfo * GetNandStorageInfo(void);

/*
*@brief function：update flash cache buffer to nand flash device 
*
* @param：null
*
* @return .   
*           
*
*/
struct StorageInfo * nand_logical_update(void);

/*
*@brief function：update flash cache buffer to nand flash device 
*@fish add 2011-05-06  for Stop am7x_recover.ko kernel thread
* @param：null
*
* @return .   
*           
*
*/
#define CHECK_STOP_VAL     (0x01)
#define CHECK_START_VAL    (0x00)
INT32U Stop_Check_Recover(INT8U Flag);

INT32U   ReadWrite_Entry(INT8U entrynum, INT32S lba, INT8U* buffer,INT32U Pagetmp);

/**
 * @}
 */
#endif /*_ACTIONS_GPIO_H_*/
