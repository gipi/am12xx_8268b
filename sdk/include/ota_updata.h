/*
********************************************************************************
*                       noya131---upgrade.drv
*                (c) Copyright 2002-2007, Actions Co,Ld. 
*                        All Right Reserved 
*
* FileName: upgrade_fw.h     Author: huang he        Date:2007/12/13
* Description: defines api and common variables for upgrade.drv
* Others:      
* History:         
* <author>    <time>       <version >    <desc>
*   huang he    2007/12/13       1.0         build this file
********************************************************************************
*/ 

#ifndef  __OTA_UPDATA_H 
#define  __OTA_UPDATA_H 
#include <linux/types.h>
#include <sysinfo.h>

typedef struct 
{
	char      partupdate_attrstr[8];
	int	     partupdate_flag;
}PartUpdate_Info;


#define WRITE_MBREC	2
#define WRITE_BREC		3
#define WRITE_FW		4

#define	INI_PRG						0
#define	WRITE_MBREC_BREC_PRG		2
#define	BACKUP_LICENSE_PRG		3
#define	WRITE_LFI_PRG				93
#define	WRITE_USER_CODE_PRG		98
#define	WRITE_APP_FLAG_PRG		99
#define	FINISH_PRG					100

#define RETRYTIME          20
#define MAX_SECTOR_ONCE    16 //32
#define SECTORLEN          512
#define READLENGTH         (2048)
#define OTA_RWSIZE  (MAX_SECTOR_ONCE*SECTORLEN)
#define OTA_CFG_PATH   "/tmp/ota_partupgrade.conf"
typedef    struct ota_fw{
	unsigned int  offset;
	unsigned int  sector_num;
	char 	*buf;//buf[OTA_RWSIZE]
}ota_data_t;

#define 	OTA_UPGRADE         _IOWR('o',1,unsigned long)	 //   (0x00)
#define 	OTA_GETSTATUS     	_IOWR('o',2,Fwu_status_t *)	// (0x01)
#define 	OTA_EXITUP		    	_IOWR('o',3,int)	//  (0x02)
#define 	OTA_GETPRG	    		_IOWR('o',4,unsigned long)	 // (0x03)
#define 	OTA_ENTER	    			_IOWR('o',5,unsigned int)	 // (0x03)
#define 	OTA_DOWNLOAD_FW	    _IOWR('o',6,ota_data_t *)	 // (0x03)
#define 	OTA_READ_FW	    	  _IOWR('o',7,ota_data_t*)	 // (0x03)
#define 	OTA_WRITE_FW	    	_IOWR('o',8,ota_data_t *)	 // (0x03)	
#define 	OTA_SETPARTUPGRADE	    	_IOWR('o',9,PartUpdate_Info *)// (0x03)			
#define		OTA_GETFWSTATUS     _IOWR('o',10,int *)
#define		OTA_GETCOMPRFILELENTH     _IOWR('o',11,int *)
#define		OTA_GETORIFILELENTH     _IOWR('o',12,int * )
#define		OTA_SETSRCADDR     _IOWR('o',13,unsigned long)
#define		OTA_SETDESTADDR     _IOWR('o',14,unsigned long)
#define		OTA_GETFILEOFFSET     _IOWR('o',15,unsigned int)
//#define		OTA_GETREQUEST     _IOWR('o',16,ota_request_t *)
//#define		OTA_ANSWERREQUEST     _IOWR('o',17,ota_request_t *)
#define 	OTA_UPDATEMBR	    _IOWR('o',18,ota_data_t *)	 // (0x03)
#define		OTA_SETFWLEN     _IOWR('o',19,unsigned long)	//fw in ddr,8252n,snor


typedef   struct fw_info{
	char  magicnum[8];
	char  type[8];
	unsigned int offset;
	unsigned int lenth;
	unsigned int crc;
	unsigned int in_offset;
}fw_info_t;

#define  FWMAGIC  "ACTMROFW"
#define  MAGICLEN 8

#define LinuxMax 16

extern int Fwu_upgrade_ota(void);
/*
描述：由upgrade.drv提供
功能：ap调用后，upgrade.drv开始升级
参数：flepath为ap找到的固件完整路径名
返回值：0表示正确，否则错误
*/

///extern int Fwu_getStatus(Fwu_status_t *status);
/*
描述：由upgrade.drv提供
功能：供ap查询状态
参数：status为upgrade.drv返回的状态
返回值：0表示正确，否则错误
*/

//#define	FILE_TEST	//for file test, but not upgrade
#endif /*__UPGRADE_FW_H */
