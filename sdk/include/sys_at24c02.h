#ifndef _SYS_AT24C02_H_
#define _SYS_AT24C02_H_
/**
*@file sys_at24c02.h
*@brief this head file describes the at24c02 operations for Actions-micro IC
*
*@author leiwg
*@date 2013-1-8
*@version 0.1
*/
#include <am7x_mac.h>
#include "../../case/include/ezcastpro.h"
#include "ezcast_config.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef  EEPROM_TYPE
#if defined(MODULE_CONFIG_EZCASTPRO_MODE) && MODULE_CONFIG_EZCASTPRO_MODE!=8251
	#define  EEPROM_TYPE   1
#elif EZWILAN_ENABLE
	#define  EEPROM_TYPE   1
#else
	#define  EEPROM_TYPE   0          //0:do not support EEPROM;1:support I2C interface EEPROM;2:support SPI interface EEPROM
#endif
#endif
/**register device path*/
#define  AM_EEPROM_PATH            "/dev/i2c_24c02"

/**
*@name command for i2c eeprom operation
*@{
*/

/**
*@}
*/
		 
#define MAX_DATA_LEN  0x08
		
struct eeprom_data_t{
		unsigned char len;
		unsigned int  addr;
		unsigned char buf[MAX_DATA_LEN];
	};
typedef struct eeprom_data_t * eeprom_data_p;

#define EEPROM_WRITE_MULTIBYTE	_IOWR('E',1,eeprom_data_p)
#define EEPROM_READ_MULTIBYTE	_IOWR('E',2,eeprom_data_p)

/**
*@brief set mac address to mac_value
*
*@param[in] mac_value	: the pointer of mac address struct which points to the mac address you want to set to.
*@return
*@retval 0		: success
*@retval !0		: fail
*@note
*	none
*/

int i2c_eeprom_read(eeprom_data_p eeprom_data);

/**
*@brief set mac address to mac_value
*
*@param[in] mac_value	: the pointer of mac address struct which points to the mac address you want to set to.
*@return
*@retval 0		: success
*@retval !0		: fail
*@note
*	none
*/
int i2c_eeprom_write(eeprom_data_p eeprom_data);

/**
*@brief write mac address to i2c eeprom 
*
*@param[in] mac_value	: the pointer of mac address struct which points to the mac address you want to set to.
*@return
*@retval 0		: success
*@retval !0		: fail
*@note
*	none
*/

int i2c_read_mac_addr(mac_address_p mac_value);

/**
*@brief read mac address from i2c eeprom
*@param[in] mac_value	: pointer to the varient, to which the mac address should be returned
*@return
*@retval 0		: success
*@retval !0		: fail to get mac address
*@note
*/
int i2c_write_mac_addr(mac_address_p mac_value);

/**
*@brief read mac address from i2c eeprom
*@param[in] mac_value	: pointer to the varient, to which the mac address should be returned
*@return
*@retval 0		: success
*@retval !0		: fail to get mac address
*@note
*/
int spi_read_mac_addr(mac_address_p mac_value);
/**
*@brief write mac address to i2c eeprom 
*
*@param[in] mac_value	: the pointer of mac address struct which points to the mac address you want to set to.
*@return
*@retval 0		: success
*@retval !0		: fail
*@note
*	none
*/
int spi_write_mac_addr(mac_address_p mac_value);

 
#ifdef __cplusplus
}
#endif
 	
#endif //_SYS_AT24C02_H_
