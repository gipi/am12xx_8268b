#ifndef _SYS_NAND_H_
#define _SYS_NAND_H_
#include "am_types.h"
/**
*@file sys_NAND.h 
*@brief This head file describes the VRAM operations for Actions-micro IC
*
*@author ONLYFISH
*@date 2011-03-21
*@version 0.1
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup vram_lib_s
*@{
*/

//PhyBoot_Read  IOCTL_PHY_Erase
#define IOCTL_GET_FLASHOPS  (0xf6)//_IOR(0xF5,0,long)	
#define IOCTL_LOG_Read      (0xf7)
#define IOCTL_LOG_Write     (0xf8)
#define IOCTL_PHYBoot_Read  (0xf9)
#define IOCTL_PHYBoot_Write (0xfa)
#define IOCTL_PHY_Erase     (0xfb)
#define IOCTL_PHY_ReadPge   (0xfc)
#define IOCTL_PHY_WritePge  (0xfd)
#define IOCTL_WRITE_ENABLE  (0x01)
#define IOCTL_WRITE_DISABLE (0x02)

typedef struct _nand_parm{
    INT32U Lba;  //Block param 
    INT8U *Buf;
    INT32U Length;
    INT32U bPage;
    
}nand_parm;



/**
*@brief struct of vram params
*/



/**
*@name vram id distribution.
*@{
*/
#define    VRAM_ID_SYSPARAMS    1
/**
*@}
*/


/**
*@brief write application data to vram
*
*@param[in] param	: specified structure for vram write
*@retval >=0			: length of written data
*@retval <0			: error number
*@see the definition of vram_param_t structure
*/
int NAND_Log_Read(INT32U Lba, INT8U *Buf, INT32U Length);

/**
*@brief read application data from vram
*
*@param[in] param	: specified structure for vram read
*@retval >=0			: length of read data
*@retval <0			: error number
*@see the definition of vram_param_t structure
*/

int NAND_Log_Write(INT32U Lba, INT8U *Buf, INT32U Length);
/**
*@brief delete application data from vram
*
*@param[in] param:specified structure for vram delete
*@retval =0	: delete success
*@retval <0	: error number
*@see the definition of vram_param_t structure
*/

int NAND_PhyBoot_Read(INT32U block, INT32U SecInBlk,INT32U Sector, INT8U *Buf);
int NAND_PhyBoot_write(INT32U block, INT32U SecInBlk,INT32U Sector, INT8U *Buf);
int NAND_PhyBlk_Erase(INT32U block, INT32U Sector);
int NAND_Phy_ReadPge(INT32U block, INT32U PageInSuBlk,INT32U Sector, INT8U *Buf);
int NAND_Phy_WritePge(INT32U block, INT32U PageInSuBlk,INT32U Sector, INT8U *Buf);
/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif //_SYS_VRAM_H_

