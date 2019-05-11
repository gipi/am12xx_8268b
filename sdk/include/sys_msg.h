#ifndef __SYS_MSG__
#define __SYS_MSG__
/**
*@file sys_msg.h
*@This head file represents the system message information and corresponding APIs
*
*
*@author yekai
*@date 2010-03-012
*@version 0.1
*/
#ifdef __cplusplus
extern "C" {
#endif

#include <am_types.h>

/**
*@addtogroup message_lib_s
*@{
*/

/**system message device number*/
#define    SYSMSG_MAX_DEVS        1
/**Max message number*/
#define    MAX_MSG_NUM            20

/**
*@brief system message definition. All the message should be defined in this format
*/
struct am_sys_msg{
	long type;				/**<message type: SYSMSG_XXX */
	unsigned int subtype;		/**<message subtype, Ex: SWF_MSG_MOVE@SYSMSG_MOUSE*/
	unsigned int dataload;		/**<message private data used to transfer more information*/
	unsigned int reserved[4];
};

/**
*@name message mask used by system device
*@{
*/
#define	CARD_MSG_EN		(1<<1)
#define	USB_MSG_EN		(1<<2)
#define	KEY_MSG_EN		(1<<3)
#define	MOUSE_MSG_EN		(1<<4)
#define	HOTPLUG_MSG_EN	(1<<5)
#define	RECOVER_MSG_EN	(1<<6)
#define	TOUCH_MSG_EN		(1<<7)
#define	ALARM_MSG_EN		(1<<8)

/**
*@}
*/

/**
*@name the message type definition. You can add new types here.
*@{
*/
/**the magic number means "AM7X"*/
#define    SYSMSG_MAGIC_NUMBER        	0x414D3758
#define    SYSMSG_CARD                		(SYSMSG_MAGIC_NUMBER + CARD_MSG_EN)
#define    SYSMSG_USB                 		(SYSMSG_MAGIC_NUMBER + USB_MSG_EN)
#define    SYSMSG_KEY                 			(SYSMSG_MAGIC_NUMBER + KEY_MSG_EN)
#define  	SYSMSG_ALARM				(SYSMSG_MAGIC_NUMBER+ALARM_MSG_EN)
#define 	SYSMSG_RECOVER			(SYSMSG_MAGIC_NUMBER+RECOVER_MSG_EN)
/** use in hotplug work thread for checking if driver can be unistalled.*/
#define    SYSMSG_HOTPLUG_CHECK       (SYSMSG_MAGIC_NUMBER + HOTPLUG_MSG_EN)
#define    SYSMSG_TOUCH				(SYSMSG_MAGIC_NUMBER + TOUCH_MSG_EN)
#define    SYSMSG_MOUSE				(SYSMSG_MAGIC_NUMBER + MOUSE_MSG_EN)
/**
*@}
*/
//bq:usb compsite interface class macro define
#define CASE_USB_COMPSITE_DEVICE 1
#ifdef  CASE_USB_COMPSITE_DEVICE
#define USB_COMPSITE_PER_INTERFACE		0	//device
#define USB_COMPSITE_AUDIO			1
#define USB_COMPSITE_COMM			2
#define USB_COMPSITE_HID			3
#define USB_COMPSITE_PHYSICAL		5
#define USB_COMPSITE_STILL_IMAGE		6
#define USB_COMPSITE_PRINTER		7
#define USB_COMPSITE_MASS_STORAGE		8
#define USB_COMPSITE_HUB			9
#define USB_COMPSITE_CDC_DATA		10
#define USB_COMPSITE_CSCID			11	
#define USB_COMPSITE_CONTENT_SEC		12
#define USB_COMPSITE_VIDEO			13
#define USB_COMPSITE_WIRELESS_CONTROLLER	14
#define USB_COMPSITE_MISC			15
#define USB_COMPSITE_APP_SPEC		16
#define USB_COMPSITE_VENDOR_SPEC		17
#endif
#if 1
struct mass_dev_t{
	int sector_num;
	char devname[10];
	char hcd_name[16];
};
#endif
/**
*@brief struct of key map info
*/
typedef struct _key_map_info{

	union _keyvalue{	
		struct _adczone{
			unsigned char	iadcfrom;
			unsigned char	iadcto;
		}adczone;
		
		unsigned short hwcode;	/**<use as ir code,or gpio num,or matrix num*/
	}keyvalue;

	unsigned char specialflag;		/**<when kb type is matrix: 1,horizontal;2,vertica*/
	unsigned char	keytype; 		/**<Disable | ADC | GPIO | IR | MATRIX*/
}key_map_info;

/**
*@brief struct of key configuration info
*/
typedef struct _key_cfg_info{
	
	unsigned char typeused;		/**<use bitmap,eg:0x03,means,use boardkey & ir key*/
	unsigned char bkeytype;		/**<board key type:	0,traditional kb;		1,martix kb*/
	union _bkeynum{
		struct _matrixbk{
			unsigned char ihor;
			unsigned char iver;
		}matrixbk;
		unsigned short tbknum;
	}bkeynum;					/**<board key total num,include,adc key and gpio key*/
	unsigned short irkeynum;		/**<ir key total num*/
	unsigned short irusrcode;		/**<ir key usr code*/
	
}key_cfg_info;

#define EVIOSETKEYCFG			_IOW( 'E', 0x01, int[2])
#define EVIOINITKEYCODE			_IOW( 'E', 0x02, int)

#define EVIOSETKEYCODE(len)		_IOC( _IOC_WRITE,'E', 0x01, len)
/**
*@name key message type definition
*@brief We use am_sys_msg->dataload to represent key messages.
*    The 8 msb represent type as "UP/DOWN/HOLD/LONG" and 
*    the 24 lsb represent key value.
*@{
*/
#define    KEY_ACTION_MASK_SHIFT    24
#define    KEY_ACTION_MASK          (~((1<<KEY_ACTION_MASK_SHIFT)-1))
#define    KEY_ACTION_UP            0x1
#define    KEY_ACTION_DOWN          0x2
#define    KEY_ACTION_HOLD          0x3
#define    KEY_ACTION_LONG          0x4
#define    KEY_ACTION_SYNC          0x5

#define    KEY_VALUE_MASK           ((1<<KEY_ACTION_MASK_SHIFT)-1)
/**
*@}
*/

/**
*@name key sub-type info
*@brief The lower 16bits of am_sys_msg->subtype used for subtype
*    of the the key message.
*@{
*/
#define    KEY_SUBTYPE_SHIFT       16
#define    KEY_SUBTYPE_MASK        ((1<<KEY_SUBTYPE_SHIFT)-1)

/** adc key and GPIO key */
#define    KEY_SUBTYPE_BOARD       0x1

/** ir key */
#define    KEY_SUBTYPE_IR          0x2
/**
*@}
*/


/**
*@name touch screen  related messages
*@brief these types of msg are set as the msg subtype, they are indicated
*    as different action of touch. You could add more types.
*@{
*/
#define	TOUCH_ACTION_UP		0x0
#define	TOUCH_ACTION_DOWN	0x1
#define	TOUCH_ACTION_SLIDE	0x2
/**
*@}
*/

/**
*@name card type definition
*@{
*/
#define	SD 			1
#define    MMC           	2
#define    MS      		3
#define    MS_PRO        	4
#define    XD            		5
#define    CF            		6	
#define 	Card_IN 		(1<<16)
#define 	Card_OUT  	(0<<16)

#define SD_IN            	(Card_IN|SD)
#define SD_OUT        	(Card_OUT|SD)
#define MMC_IN            	(Card_IN|MMC)
#define MMC_OUT        	(Card_OUT|MMC)
#define MS_IN            	(Card_IN|MS)
#define MS_Pro_OUT	(Card_OUT|MS_PRO) 
#define MS_Pro_IN	(Card_IN|MS_PRO)
#define MS_OUT        	(Card_OUT|MS) 
#define XD_IN		(Card_IN|XD)
#define XD_OUT 		(Card_OUT|XD)
#define CF_IN 		(Card_IN|CF)
#define CF_OUT 		(Card_OUT|CF)

#define    OUT         		0
#define    IN          		1
/**
*@}
*/


/**
*@brief USB related messages 
*/
#define USB_GROUP0    0xa0
#define USB_GROUP1    0xa1
/**
*@name USB subtype
*@{
*/
#define	USB_MSG_BASE			0x10
#define	DEVICE_USB_IN			(0x0+USB_MSG_BASE)
#define	DEVICE_USB_OUT		(0x1+USB_MSG_BASE)
#define	HOST_USB_IN			(0x2+USB_MSG_BASE)
#define	HOST_USB_OUT			(0x3+USB_MSG_BASE)
#define 	DEVICE_USB_VENDOR	(0x4+USB_MSG_BASE)
/**
*@}
*/

/**
*@name USB dataload
*@{
*/
#define	DEVICE_RAW					0x0
#define	DEVICE_MASS_STORAGE			0x1
#define    DEVICE_MASS_STORAGE_NEXT          0x11
#define	DEVICE_SUBDISPLAY				0x2
#define	DEVICE_PICTBRIDGE				0x3
#define	DEVICE_USB_DEBUG				0x4
#define	DEVICE_POWER_ADAPTER			0x5
#define	DEVICE_CON_NODPDM			0x6

#define	DEVICE_MASS_2_SUBDISP		0x10
#define	DEVICE_SUBDISP_2_MASS		0x11
#define	DEVICE_SUBDISP_QUIT			0x12
#define	DEVICE_MASS_2_DEBUG			0x13
#define	DEVICE_DEBUG_2_MASS			0x14
#define	DEVICE_DEBUG_QUIT				0x15
/**
*@}
*/

/**
*@name host msg
*@{
*/
#define	HOST_RAW					0x4
#define	HOST_RAW_NEXT                         0x41
#define	HOST_MASS_STORAGE		0x5
#define	HOST_USB_HID				0X6
#define	HOST_USB_BLUETOOTH		0x7
//#define	HOST_HID_MOUSE				0x6
//#define	HOST_HID_KEY				0x7
#define	HOST_VIDEO_CAPTURE			0x8
#define	HOST_MASS_STOARGE_SCANOK	0x9
#define	HOST_WLAN						0xa
#define	HOST_WLAN_STA				0x10
#define	HOST_WLAN_SOFTAP				0x11
#define	HOST_IPHONE	                0x12  //zdy add solution iphone
#define HOST_ANDROID_USB_SHARE		0x13

#define	HOST_USB_UVC				0xc0

/*
*@ remove disk symbol
*@{
*/
#define	DEVICE_R_UDISK				0xe
/**
*@}
*/

/*
  *
  */
 extern int id_message; //ba add message
 extern int identify_message(struct am_sys_msg msg);  //bq add

/**
*@name system recover subtype
*@{
*/
#define SYS_CHECK_START		0x1		/**<start to check firmware*/
#define SYS_CHECK_STOP			0x2		/**<check over and ready to quit*/
#define SYS_RECOVER_START		0x3		/**<check fail and start to recover*/
#define SYS_RECOVER_END		0x4		/**<system recover finished*/
/**
*@}
*/

/**
*@brief API for modules in kernel mode:put a system message in the pool
*
*@param[in] msg	: system message
*@retval 0		: success
*@retval !0		: message pool full
*/
INT32S am_put_sysmsg(struct am_sys_msg msg);

/**
*@brief API for modules in kernel mode: register a device to system message
*
*@param[in] p	: pointer of system message handler
*@param[in] mask	: message mask, 1 for per bit is enable
*@param[in] pdata	: private data used by device function p
*@retval 0		: success
*@retval !0		: standard error code
*/
INT32S am_register_sysmsg(void (*p)(struct am_sys_msg*,void *), INT32U mask,void *pdata);

/**
*@brief API for modules in kernel mode: unregister a device from system message
*
*@param[in] p	: pointer of system message handler
*@retval 0		: success
*@retval !0		: standard error code
*/
INT32S am_unregister_sysmsg(void (*handler)(struct am_sys_msg*,void *));

/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif /*__SYS_MSG__*/
