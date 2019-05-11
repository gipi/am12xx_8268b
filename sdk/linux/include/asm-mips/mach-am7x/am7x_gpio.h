#ifndef _ACTIONS_GPIO_H_
#define _ACTIONS_GPIO_H_
/**
*@file gpio.h
*
*@author yekai
*@date 2010-01-28
*@version: 0.2
*/
#include <linux/types.h>

/**
*@addtogroup gpio_driver_s
*@{
*/

/**
*@name driver info
*@{
*/
#define DRIVER_NAME "am7x_gpio"
#define GPIO_MAX_DEVS 	2
#define GPIO_BASE_N		0
/**
*@}
*/

/**gpio config file path*/
#define GPIO_CONFIG_FILE		"/am7x/sdk/easy/gpio.bin"

/**
*@name gpio register info
*@{
*/

#if CONFIG_AM_CHIP_ID == 1213
#define GPIO_MFL_MAX	5
#define GPIO_IEN_MAX	4
#define GPIO_OEN_MAX	4
#else
/**max number of multifunction */
#define GPIO_MFL_MAX	9
/**max number of input enable*/
#define GPIO_IEN_MAX	3
/**max number of output enable*/
#define GPIO_OEN_MAX	3
#endif
/**interval between two mfl registers*/
#define GPIO_MFL_INTER	4
/**interval between two input enable registers*/
#define GPIO_IEN_INTER	12
/**interval between two output enable registers*/
#define GPIO_OEN_INTER	12

/**
*@}
*/

/**
*@brief struct of gpio configuration
*/
struct am_gpio_config{
	INT32U gpio_mfl[GPIO_MFL_MAX];		/**<gpio multi-function registers*/
	INT32U gpio_in_en[GPIO_IEN_MAX];		/**<gpio input enable registers*/
	INT32U gpio_out_en[GPIO_OEN_MAX];	/**<gpio output enable registers*/
};

/**
*@brief set the specified gpio
*
*@param[in] num	: gpio number
*@param[in] value	: gpio state, high(1) or low(0)
*@return NULL
*/
void am_set_gpio(loff_t num,INT8U value);

/**
*@brief read the specified gpio
*
*@param[in] num	: gpio number
*@return state of the gpio
*/
INT8U am_get_gpio(loff_t num);

/**
 *@}
 */

#endif /*_ACTIONS_GPIO_H_*/