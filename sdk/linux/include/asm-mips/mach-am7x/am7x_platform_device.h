/**
 *@file am7x_platform_device.h
 *@brief platform device info on am7x board
 *@author yekai
 *@date 2010-11-16
 */

#ifndef _AM7x_PLATFORM_DEVICE_WAPPER_H_
#define _AM7x_PLATFORM_DEVICE_WAPPER_H_

/**
*@addtogroup platform_driver_s
*@{
*/

#undef PLATFORM_DMA_SUPPORT

/**
*@name board device name
*@{
*/
#define AM7X_DEV_NAME_RTC  "rtc-am7xxx"
#ifdef PLATFORM_DMA_SUPPORT
#define AM7X_DEV_NAME_DMA  "dma-am7xxx"
#endif
#define AM7X_DEV_NAME_SD  "sd-am7xxx"
#define AM7X_DEV_NAME_FB  "fb-am7xxx"
#define AM7X_DEV_NAME_MH  "mheap-am7xxx"
#define AM7X_DEV_NAME_USB   "aotg-usb-am7x-"
#if CONFIG_AM_CHIP_ID == 1213
#define AM7X_DEV_NAME_USB1   "aotg-usb-am7x_next"
#endif
#define AM7X_DEV_NAME_LCM  "lcm-am7x"
#define AM7X_DEV_NAME_CEC  "cec-am7x"
#define AM7X_DEV_NAME_NET_KSZ884X    "ksz8841/2"
#define AM7X_DEV_NAME_KBD  "key-am7x"	
#define AM7X_DEV_NAME_NET "net-am7x"	
/**
*@}
*/

#ifdef PLATFORM_DMA_SUPPORT
/** 
 * @brief struct am7x_dma_platform_data - Controller configuration parameters
 */
struct am7x_dma_platform_data {
    unsigned int  nr_channels;	/**<number of channels supported by hardware (max 10)*/
};
#endif

/**
 *@brief struct of usb data configuration
 */
struct am7x_usb_platform_data{
	unsigned int vbus_check_gpio;		/**<gpio number for vbus check*/
	unsigned int is_otg;				/**<determine whether it is otg function*/	
	unsigned int host_only;			/**<determine whether it supports host only*/
	unsigned int reserved1;			/**<reserved place*/
};


/**
 *@}
 */

#endif /* _AM7x_PLATFORM_DEVICE_WAPPER_H_ */

