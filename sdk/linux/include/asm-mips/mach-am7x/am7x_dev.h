#ifndef _ACTIONS_DEV_H_
#define _ACTIONS_DEV_H_
/**
*@file am7x_dev.h
*
*@author yekai
*@date 2010-08-13
*@version 0.1
* ------------------
*@modification on 2010-09-09 by yekai
*	-add some mutex for device op and use function instead of inline.
*/
#include "actions_regs.h"
#include "actions_io.h"

/**
*@addtogroup device_driver_s
*@{
*/

/**
*@brief disable the specified device clock
*
*@param[in] num	: offset in device clock register
*@param[in] reg	: the device clock register address
*@return NULL
*/
void am_disable_dev_clk(unsigned int num,AM7X_HWREG reg);

/**
*@brief enable the specified device clock
*
*@param[in] num	: offset in device clock register
*@param[in] reg	: the device clock register address
*@return NULL
*/
void am_enable_dev_clk(unsigned int  num,AM7X_HWREG reg);

/**
*@brief reset the specified device
*
*@param[in] num	: offset in device clock register
*@param[in] reg	: the device clock register address
*@return NULL
*/
void am_reset_dev(unsigned int  num, AM7X_HWREG reg);

/**
*@brief setup watchdog
*
*@param[in] wdtime : period after which system will restart automatically\n
*				     0~7 is the legal value and others will disable watchdog
*@return NULL
*/
void am_setup_watchdog(__u8 wdtime);

/**
*@brief clear watchdog timer and start to recount
*
*@param[in] null
*@return NULL
*/
void am_clear_watchdog(void);
/**
 *@}
 */

#endif /*_ACTIONS_DEV_H_*/
