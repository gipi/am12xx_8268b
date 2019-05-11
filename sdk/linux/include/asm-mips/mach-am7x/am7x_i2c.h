#ifndef _ACTIONS_I2C_H_
#define _ACTIONS_I2C_H_
/**
*@file am7x_i2c.h
*
*@author yekai
*@date 2010-08-23
*@version 0.1
*/

/**
*@addtogroup i2c_driver_s
*@{
*/

/**I2C group number on am board*/
#if	CONFIG_AM_CHIP_ID==1211
#define USE_I2C_GROUP	2
#elif CONFIG_AM_CHIP_ID==1220 || CONFIG_AM_CHIP_ID==1213
#if (CONFIG_AM_CHIP_MINOR==8258 ||CONFIG_AM_CHIP_MINOR==8268)
#define USE_I2C_GROUP	1
#else 
#ifdef CONFIG_AM_8251
#define USE_I2C_GROUP	2
#else
#define USE_I2C_GROUP	1
#endif
#endif
#else
#define USE_I2C_GROUP	2
#endif

/**I2C base address*/
#if USE_I2C_GROUP==1
	#define AM_I2C_BASE		0xb0180000
#elif USE_I2C_GROUP==2
	#define AM_I2C_BASE		0xb0180020
#else
	#error "INVALIDE I2C GROUP"
#endif

/** 
*@name I2C Work speed 
*@{
*/
#define AM_I2C_NORMAL			(100*1000)
#define AM_I2C_FAST				(400*1000)
/**
*@}
*/

/**
*@name I2C Registers offset
*@{
*/
#define AM_I2C_CTL_OFFSET		0x0
#define AM_I2C_CLKDIV_OFFSET	0x4
#define AM_I2C_STAT_OFFSET		0x8
#define AM_I2C_ADDR_OFFSET		0xc
#define AM_I2C_DAT_OFFSET		0x10

#define AM_I2C_TRC_MASK		(1<<7)
#define AM_I2C_STPD_MASK		(1<<6)
#define AM_I2C_ACK_MASK		1
/**
*@}
*/

/**
*@name I2C bus status
*@{
*/
#define AM_I2C_BUS_READ		0x00001
#define AM_I2C_BUS_WRITE		0x00002
#define AM_I2C_BUS_START		0x00003
#define AM_I2C_BUS_BUSY		0x00004
#define AM_I2C_BUS_IDLE			0x00005
/**
*@}
*/

/**
*@name I2C Tramsfer flags
*@{
*/
#define AM_I2C_WRITE				0x00000
#define AM_I2C_READ				0x00001
#define AM_I2C_LAST				0x00002

#define AM_I2C_START_MASK		0x000f0
#define AM_I2C_START			0x00010
#define AM_I2C_START_REPEAT	0x00020
#define AM_I2C_START_NO_REP	0x00030

#define AM_I2C_STOP_MASK		0x00f00
#define AM_I2C_STOP				0x00100
#define AM_I2C_STOP_ACK			0x00200
#define AM_I2C_STOP_NO_ACK		0x00300
/**
*@}
*/

/** 
*@name I2C control command
*@{
*/
#define IIC_ENABLE 							(0x1 << 7) 	/**<bit7*/
#define IIC_RESET							(0x00000000)	
#define IIC_START			 					(0x1 << 2)	/**<bit2 & bit3*/
#define IIC_STOP								(0x2 << 2)	/**<bit2 & bit3*/
#define IIC_REPEAT_START					(0x3 << 2)	/**<bit2 & bit3*/
#define IIC_TRANS_READY						(0x1 << 1)	/**<bit1*/
#define IIC_REC_NO_ACK		  				0x1			/**<bit0*/
#define IIC_ACK_RECEIVED					0x1			/**<bit0*/

#define IIC_REC_ACK_CMD 					(IIC_ENABLE	| IIC_TRANS_READY )	  										//0x00000082
#define IIC_REC_NO_ACK_CMD					(IIC_ENABLE 	| IIC_TRANS_READY 	| IIC_REC_NO_ACK) 						//0x00000083
#define IIC_SEND_NO_REP_CMD 				(IIC_ENABLE 	| IIC_TRANS_READY)											//0x00000082
#define IIC_SEND_START_CMD 					(IIC_ENABLE 	| IIC_START 		| IIC_TRANS_READY)							//0x00000086
#define IIC_SEND_REP_CMD					(IIC_ENABLE 	| IIC_REPEAT_START 	| IIC_TRANS_READY)						//0x0000008e
#define IIC_STOP_CMD 						(IIC_ENABLE 	| IIC_TRANS_READY	|IIC_STOP	|IIC_ACK_RECEIVED)		//0x0000008b
/**
*@}
*/

/**I2C params*/
#define AM_I2C_TIMEOUT			0xfffff

/**
*@brief get i2c bus id
*
*@param[in] NULL
*@return ID of the current active i2c bus
*/
int am_get_i2c_bus_id(void);

/**
 *@}
 */

#endif /*_ACTIONS_I2C_H_*/