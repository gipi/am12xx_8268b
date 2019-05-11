/*
 * board initialization should put one of these into dev->platform_data
 * and place the sl811hs onto platform_bus named "sl811-hcd".
 */

#ifndef __LINUX_USB_HOTPLUG_H
#define __LINUX_USB_HOTPLUG_H

//host msg
#define	MSG_HOST_RAW					0x4
#define	MSG_HOST_RAW_NEXT			0x41 //bq: add to solution 1213 usb message process 
#define	MSG_MASS_STORAGE				0x5
#define	MSG_USB_HID					0x6
#define	MSG_USB_BLUETOOTH			0x7
#define	MSG_VIDEO_CAPTURE			0x8
#define	MSG_WLAN						0xa
#define	MSG_WLAN_STA					0x10
#define	MSG_WLAN_SOFTAP				0x11

//used to indicate we scann complete
#define	MSG_MASS_STOARGE_SCANOK	0x9

//bq:add a usb compsite device message
#define	MSG_USB_COMPSITE_DEV				0xb

//add usb image device message
#define	MSG_IMAGE_DEV				0xc

 void usb_hotplug_msg(int plugmsg,int plugstatus,int port_num);
#endif /* __LINUX_USB_SL811_H */
