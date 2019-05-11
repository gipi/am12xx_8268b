#ifndef _AM7X_MAC_H
#define _AM7X_MAC_H

#include <asm/ioctl.h>
#include "am7x_wolconfig.h"

#define INT32U unsigned int
// 20160415 TsungRung added for 8201F(Phy) WOL function.
#if (WAKE_ON_LAN_WAKE_ON_LAN_FUNCTION)
#define ENABLE_8201F_WOL_MODE 1
#else
#define ENABLE_8201F_WOL_MODE 0
#endif

#ifndef EFAULT
#define EFAULT 1
#endif

#ifndef EINVAL
#define EINVAL 1
#endif

typedef struct mac_address_s{
	unsigned char mac_addr_0;
	unsigned char mac_addr_1;
	unsigned char mac_addr_2;
	unsigned char mac_addr_3;
	unsigned char mac_addr_4;
	unsigned char mac_addr_5;
}mac_address_t;
typedef struct mac_address_s * mac_address_p;

typedef struct ip_address_s{
	unsigned char ip_addr_0;
	unsigned char ip_addr_1;
	unsigned char ip_addr_2;
	unsigned char ip_addr_3;
}ip_address_t;

typedef struct ip_address_s * ip_address_p; 

#if(ENABLE_8201F_WOL_MODE)
#define MACIO_MAXNR					14
#else
#define MACIO_MAXNR					9
#endif
#define MACIO_SET_MAC_ADDR			_IOW('d',1,mac_address_p)
#define MACIO_GET_MAC_ADDR	   	 	_IOR('d',2,mac_address_p)	/* the volum's value is 0x0 ~ 0x1f */
#define MACIO_SET_WAKEUP_IP			_IOW('d',3,ip_address_p)
#define MACIO_SET_WAKEUP_MASK		_IOW('d',4,ip_address_p)

#define MACIO_GET_WAKEUP_IP			_IOR('d',5,ip_address_p)
#define MACIO_GET_WAKEUP_MASK 		_IOR('d',6,ip_address_p)
#define MACIO_SET_WAKEUP_MODE       _IOW('d',7,unsigned char)
#define MACIO_GET_WAKEUP_MODE		_IOR('d',8,unsigned char)
#define MACIO_CONTROL_EEE_FUNCTION	_IOR('d',9,unsigned char)
#if(ENABLE_8201F_WOL_MODE)
#define MACIO_SET_WOL_MODE                        _IOW('d',10,unsigned char)
#define MACIO_RESTORE_FROM_WOL_MODE     _IOW('d',11,unsigned char)
#define MACIO_SET_PHY_REG                          _IOW('d',12,unsigned char)
#define MACIO_SET_PHY_REG_VALUE          _IOW('d',13,unsigned char)
#define MACIO_GET_PHY_REG		              _IOW('d',14,unsigned char)
#endif

#endif
