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

#ifndef  __UPGRADE_FW_H 
#define  __UPGRADE_FW_H 
#include <linux/types.h>
#include <am_types.h>
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

#define 	FWU_SETPATH          _IOW('c',1,unsigned long)	 //   (0x00)
#define 	FWU_GETSTATUS     _IOR('c',2,Fwu_status_t)	// (0x01)
#define 	FWU_EXITUP		    _IOR('c',3,unsigned char)	//  (0x02)
#define 	FWU_GETPRG	    _IOR('c',4,unsigned long)	 // (0x03)
#define 	FWU_SETPARTUPGRADE	_IOR('c',5,PartUpdate_Info *)
#define LinuxMax 16



extern int Fwu_upgradeByPath(char *filepath);
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
