#ifndef _SYS_VRAM_H_
#define _SYS_VRAM_H_
/**
*@file sys_vram.h 
*@brief This head file describes the VRAM operations for Actions-micro IC
*
*@author yekai
*@date 2010-06-24
*@version 0.1
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
*@addtogroup vram_lib_s
*@{
*/

/**
*@brief struct of vram params
*/
struct vram_param_t{
unsigned int task_id;		/**<application ID*/
unsigned int sub_type;		/**<data type of the application*/
unsigned int target_name; 	/**<the name of the target where data should be*/
unsigned int offset;		/**<offset in the target*/
unsigned char *pbuf;		/**<pointer of the data buffer*/
unsigned int length;		/**<length of the data buffer*/
};


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
int app_write_vram (struct vram_param_t* param);

/**
*@brief read application data from vram
*
*@param[in] param	: specified structure for vram read
*@retval >=0			: length of read data
*@retval <0			: error number
*@see the definition of vram_param_t structure
*/
int app_read_vram(struct vram_param_t* param);

/**
*@brief delete application data from vram
*
*@param[in] param:specified structure for vram delete
*@retval =0	: delete success
*@retval <0	: error number
*@see the definition of vram_param_t structure
*/
int app_delete_vram(struct vram_param_t* param);


/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif //_SYS_VRAM_H_

