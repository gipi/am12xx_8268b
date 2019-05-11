#ifndef _ACTIONS_BOARD_H_
#define _ACTIONS_BOARD_H_
/**
*@file am7x_board.h
*
*@author yekai
*@date 2010-08-13
*@version 0.1
*/

/**
*@addtogroup board_driver_s
*@{
*/
#include <sys_cfg.h>

/**
*@brief multi-pin module types
*/
typedef enum{
	DEVICE_RES,		/**< board device resource for individual module*/
#ifdef  CONFIG_AM_8251
	MAC_I2C,
#endif
	FLASH_CARD,		/**< multi-pin for flash and card, which is used in common*/
	MAX_MULTI_ID = FLASH_CARD,		/**<max number for multi-pin modules*/
}AM_MULTI_TYPE; 

/**offset of am easy table*/
#define EZ_OFFSET		48
/**max multi-pin resource number on board*/
#define MAX_MULTI_RES	(1+MAX_MULTI_ID)

/**
*@brief Initialize the specified board resource
*
*This function is used to set the initial value of a system source,
*which will be accessed by more than one modules, such as bus/pin/buffer .etc
*@param[in] id	:  resource id.
*@param[in] val	:  available number of the source
*@retval 0		: success
*@retval !0		: standard error no
*/
int am_init_multi_res(AM_MULTI_TYPE id,unsigned int val);

/**
*@brief get the specified board resource
*
*@param[in] id	:  resource id.

*@retval 0		: success
*@retval !0		: standard error no
*@note
*	-if the resource is available, function will return and decrease the corresponding resource number,
*	-otherwise it will suspend in uninterruptible state
*/
int am_get_multi_res(AM_MULTI_TYPE id);

/**
*@brief release the specified board resource and increase the available resource number
*
*@param[in] id	:  resource id.
*@retval 0		: success
*@retval !0		: standard error no
*/
int am_release_multi_res(AM_MULTI_TYPE id);

/**
*@brief read config file made by EZ
*
*@param[in] path	: path of the config file
*@param[in] buf	: buffer where data will be read
*@param[in] offset	: start read point offset in file
*@param[in] count	: size of read data
*
*@retval 0		: success
*@retval !0		: standard error no
*/
int am_get_config(char *path, char *buf, loff_t offset, size_t count);


/**
*@brief get system infomation, gpio,mem....etc
*
*@param[in] NULL
*
*@return	pointer of system info structure
*/
struct sys_cfg* get_sys_info(void);
/**
 *@}
 */


#endif /*_ACTIONS_BOARD_H_*/
