#ifndef _SYS_MAC_H_
#define _SYS_MAC_H_
/**
*@file sys_mac.h
*@brief this head file describes the MAC operations for Actions-micro IC
*
*@author yekai
*@date 2012-12-20
*@version 0.1
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <am7x_mac.h>
/**
*@addtogroup mac_lib_s
*@{
*/

/**register device path*/
#define  AM_MAC_PATH            "/dev/MAC"

/**
*@name command for gpio operation, not used yet
*@{
*/
#define MAC_MAGIC_MODE			(1<<0)
#define MAC_ARP_MODE  			(1<<1)
#define MAC_PING_MODE     	    (1<<2)
#define MAC_LINKUP_MODE		    (1<<3)
#define MAC_LINKDOWN_MODE		(1<<4)
#define MAC_NONE_MODE		    (0)


/**
*@}
*/

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
 int set_mac_addr_cdev(mac_address_p mac_value);

/**
*@brief get the mac address
*@param[in] mac_value	: pointer to the varient, to which the mac address should be returned
*@return
*@retval 0		: success
*@retval !0		: fail to get mac address
*@note
*/
 int get_mac_addr_cdev(mac_address_p mac_value);
 
/**
*@brief set the mac wakeup ip address
*@param[in] ip_value	: pointer to the wakeup ip address you wan to set to. 
*@return
*@retval 0		: success
*@retval !0		: fail to set mac wakeup ip 
*@note
*/
int set_mac_wakeupip(ip_address_p ip_value);

/**
*@brief set the mac wakeup ip mask
*@param[in] ip_value	: pointer to the wakeup ip mask you wan to set to. 
*@return
*@retval 0		: success
*@retval !0		: fail to set mac wakeup ip mask
*@note
*/
int set_mac_wakeupmask(ip_address_p ip_value);

/**
*@brief get the mac wakeup ip 
*@param[in] ip_value	: pointer to the varient,to which the mac address should be returned. 
*@return
*@retval 0		: success
*@retval !0		: fail to get mac wakeup ip 
*@note
*/
int get_mac_wakeupip(ip_address_p ip_value);

/**
*@brief get the mac wakeup ip mask
*@param[in] ip_value	: pointer to the varient,to which the mac address should be returned. 
*@return
*@retval 0		: success
*@retval !0		: fail to get mac wakeup ip mask
*@note
*/
int get_mac_wakeupmask(ip_address_p ip_value);

/**
*@brief set the mac wakeup ip mode
*@param[in] value	: the mode you want to set to. 
*@return
*@retval 0		: success
*@retval !0		: fail to set the mac wakeup mode
*@note
* the mode you wan to set to could be the result of one or more selection from MAC_MAGIC_MODE/MAC_ARP_MODE/MAC_PING_MODE/MAC_LINKUP_MODE/MAC_LINKDOWN_MODE by OR operation
*/
int set_mac_wakeupmode(unsigned char value);

/**
*@brief get the mac wakeup ip mode
*@param[in] value	: pointer to the varient,to which the mac address should be returned.  
*@return
*@retval 0		: success
*@retval !0		: fail to get the mac wakeup mode
*@note
*/
int get_mac_wakeupmode(unsigned char *value);

/**
*@brief judge whether the system supports these wakeup modes or not.
*@param[in] result	: pointer to the varient,to which the result should be returned.  
*@param[in] value	: the mode you want to test whether this system support or not
*@return
*@retval 0		: success
*@retval !0		: fail to get the mac wakeup mode
*@note
* the mode you wan to set to could be the result of one or more selection from MAC_MAGIC_MODE/MAC_ARP_MODE/MAC_PING_MODE/MAC_LINKUP_MODE/MAC_LINKDOWN_MODE by OR operation
*/
int is_mac_wakeupmode(unsigned char *result,unsigned char value);

#ifdef __cplusplus
}
#endif
 	
#endif //_SYS_MAC_H_
