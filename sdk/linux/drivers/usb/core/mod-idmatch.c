 /* Functions local to drivers/usb/core/ */
#include <linux/usb.h>
#include <linux/usb/ch9.h>
#include <linux/usb/hotplug.h>
#include <linux/mod_devicetable.h>
#include <sys_msg.h>
#include "hcd.h"
#include "hub.h"
#include "../am7x/am7x_hcd.h"  //bq add
#include "../am7x/aotg_io.h"

#define DEBUG_HOTPLUG	
#ifdef  DEBUG_HOTPLUG
#define HP_DBG(fmt,stuff...)   printk(KERN_INFO fmt,##stuff)
#else
#define HP_DBG(fmt,stuff...)   do {} while (0)
#endif

struct hp_device_info {
	struct usb_device_id   id;
	unsigned int hp_msg;
};

#define USB_HCD   1
#define USB_HCD_NEXT  2
unsigned int cur_pv=0;	/*use global var to save current device PID & VID*/
//static struct usb_device * mass_dev= NULL;
//static int mass_flag1=0;
//static int mass_flag2=0;
//static int massdev_num = 0;
#define USB_INTERFACE_CLASS_HID	3

static struct hp_device_info  dev_idmatch_tbl[]={
// 1 msg for mass storage
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS,
			0,0,0,0,0,0,0,
			USB_INTERFACE_CLASS_HID,0,0,0
		},
		MSG_USB_HID
	},
	
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS,
			0,0,0,0,0,0,0,
			USB_CLASS_MASS_STORAGE,0,0,0
		},
		MSG_MASS_STORAGE
	},

	//add  device imag class match flag  
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS,
			0,0,0,0,0,0,0,
			USB_CLASS_STILL_IMAGE,0,0,0
		},
		MSG_IMAGE_DEV
	},
	
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x07b8,0x3071,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*ralink wlan*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0ace,0x1215,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*atheros wlan*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8176,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8178,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8194,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
			
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x018a,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x14b2,0x3c2b,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*nec wlan*/
	},
	
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x148f,0x5370,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*ralink wlan 5370*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x148f,0x3072,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*ralink wlan 3072*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x148f,0x3070,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*ralink wlan (148f)3070*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x148f,0x3370,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*ralink wlan (148f)3370*/
	},


	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x07b8,0x3070,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*ralink wlan (07b8)3070*/
	},
			
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x4855,0x0090,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8193,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8111,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x0193,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8171,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8179,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x0179,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan:8188eus dongle*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x2001,0x3308,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x7392,0x7811,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek wlan*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x818b,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8192eu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x818c,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8192eu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x0811,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8192eu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8822,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8192eu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xa811,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8192eu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x0820,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8192eu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x0821,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8811au*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x0823,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8811au/8821au*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x20f4,0x804b,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8811au/8821au*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xf179,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8188fu/8188ftv*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xb82b,0,0,0,0,0,
			USB_CLASS_VENDOR_SPEC,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xb820,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xc821,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xc820,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xc82a,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xc82b,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0xc811,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},
	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_VENDOR | \
				USB_DEVICE_ID_MATCH_PRODUCT,
			0x0bda,0x8811,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0,0,0
		},
		MSG_WLAN	/*realtek 8821cu*/
	},

	{
		{
			USB_DEVICE_ID_MATCH_INT_CLASS | USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
				USB_DEVICE_ID_MATCH_INT_PROTOCOL,
			0,0,0,0,0,0,0,
			USB_CLASS_WIRELESS_CONTROLLER,0x01,0x01,0 // Based On Bluetooth Specification
		},
		MSG_USB_BLUETOOTH	/*bluetooth*/
	},
// add the new device match id & msg here

// last be zero to judge end
	{},
};

void dump_interface_description(struct usb_device * udev)
{
	struct usb_interface_descriptor 	*intf_desc;
	struct usb_device_descriptor 	*dev_desc;
	int intf_num;
	
	if (!udev  || !udev->actconfig) {
		HP_DBG(KERN_INFO "%s: bad param\n",__FUNCTION__);
		return;
	}
	intf_num = 0;
	dev_desc = &(udev->descriptor);
	intf_num = udev->actconfig->desc.bNumInterfaces;

	if(intf_num > 1){
		HP_DBG ("!!%s:multi interface usb device \n",__FUNCTION__);
	}	
	
	intf_desc = &(udev->actconfig->interface[0]->cur_altsetting->desc);

	HP_DBG("dev_desc->idVendor:0x%04x\n",dev_desc->idVendor);
	HP_DBG("dev_desc->idProduct:0x%04x\n",dev_desc->idProduct);
	HP_DBG("dev_desc->bcdDevice:0x%04x\n",dev_desc->bcdDevice);
	HP_DBG("dev_desc->bDeviceClass:0x%02x\n",dev_desc->bDeviceClass);
	HP_DBG("dev_desc->bDeviceSubClass:0x%02x\n",dev_desc->bDeviceSubClass);
	HP_DBG("dev_desc->bDeviceProtocol:0x%02x\n",dev_desc->bDeviceProtocol);
	HP_DBG("intf_desc->bInterfaceClass:0x%02x\n",intf_desc->bInterfaceClass);
	HP_DBG("intf_desc->bInterfaceSubClass:0x%02x\n",intf_desc->bInterfaceSubClass);
	HP_DBG("intf_desc->bInterfaceProtocol:0x%02x\n",intf_desc->bInterfaceProtocol);
	return;
}

int  usb_idmatch(struct usb_device * udev)
{
	struct usb_interface_descriptor 	intf_desc[USB_MAXINTERFACES];
	struct usb_device_descriptor 	*dev_desc;
	struct hp_device_info *dev_info;
	struct usb_device_id  *id; 
	int intf_num;
	int msg;
	int idx = 0;
	int idx_ifs=0;
#if 1 //bq:display the device speed information
	printk("<%s >device speed:%d\n",__func__,udev->speed);
	switch(udev->speed){
		case 1:
			printk(KERN_INFO "LOW SPEED DEVICE\n");
			break;
		case 2:
			printk(KERN_INFO "FULL SPEED DEVICE\n");
			break;
		case 3:
			printk(KERN_INFO "HIGH SPEED DEVICE\n");
			break;
		case 4:
			printk(KERN_INFO "VARIABLE SPEED DEVICE\n");
			break;
		default:
			printk(KERN_INFO "UNKOWN SPEED DEVICE\n");
			break;
	}
#endif
	if (!udev  || !udev->actconfig) {
		HP_DBG(KERN_INFO "%s: bad param\n",__FUNCTION__);
		return 0;
	}	
	
	/*FIX ME :here we only process device with single 
		interface,if multi interface ,seems we need to use the default
		usb core match methods*/
	intf_num = 0;
	dev_desc = &(udev->descriptor);
	intf_num = udev->actconfig->desc.bNumInterfaces;
	printk(KERN_INFO "device config num=%d\n",dev_desc->bNumConfigurations);
	printk(KERN_INFO "<%s %d>multi interface usb device interfaceNum:%d\n",__FUNCTION__,__LINE__,intf_num);
#if 0
	printk(KERN_WARNING "Class    	= 0x%x\n",dev_desc->bDeviceClass);	
	printk(KERN_WARNING "subClass	= 0x%x\n",dev_desc->bDeviceSubClass);
	printk(KERN_WARNING "Protocol	= 0x%x\n",dev_desc->bDeviceProtocol);
	printk(KERN_INFO "<%s %d>multi interface usb device interfaceNum1:%d\n",__FUNCTION__,__LINE__,udev->config->desc.bNumInterfaces);
#endif
	if(intf_num > 1){
		HP_DBG ("!!%s:multi interface usb device \n",__FUNCTION__);
	}

	for(idx_ifs=0;idx_ifs<intf_num;idx_ifs++){
		intf_desc[idx_ifs] = (udev->actconfig->interface[idx_ifs]->cur_altsetting->desc);
		printk(KERN_WARNING "interface Class[%d] = 0x%x\n",idx_ifs, intf_desc[idx_ifs].bInterfaceClass);
		printk(KERN_WARNING "interface subClass[%d] = 0x%x\n", idx_ifs,intf_desc[idx_ifs].bInterfaceSubClass);
		printk(KERN_WARNING "interface Protocol[%d] = 0x%x\n", idx_ifs,intf_desc[idx_ifs].bInterfaceProtocol);
	}
		
	for (idx = 0;idx < ARRAY_SIZE(dev_idmatch_tbl);idx++) {
	
		 dev_info = &dev_idmatch_tbl[idx];
		 id = &dev_info->id;
		#if 0
		printk(KERN_WARNING "id->match_flags = 0x%x\n",id->match_flags);
		printk(KERN_WARNING "id->idVendor = 0x%x, dev_desc->idVendor = 0x%x\n",id->idVendor,dev_desc->idVendor);
		printk(KERN_WARNING "id->idProduct = 0x%x, dev_desc->idProduct = 0x%x\n",id->idProduct,dev_desc->idProduct);
		printk(KERN_WARNING "id->bcdDevice_lo = 0x%x, dev_desc->bcdDevice = 0x%x\n",id->bcdDevice_lo,dev_desc->bcdDevice);
		printk(KERN_WARNING "id->bcdDevice_hi = 0x%x, dev_desc->bcdDevice = 0x%x\n",id->bcdDevice_hi,dev_desc->bcdDevice);
		printk(KERN_WARNING "id->bDeviceClass = 0x%x, dev_desc->bDeviceClass = 0x%x\n",id->bDeviceClass,dev_desc->bDeviceClass);
		printk(KERN_WARNING "id->bDeviceSubClass = 0x%x, dev_desc->bDeviceSubClass = 0x%x\n",id->bDeviceSubClass,dev_desc->bDeviceSubClass);
		printk(KERN_WARNING "id->bDeviceProtocol = 0x%x, dev_desc->bDeviceProtocol = 0x%x\n",id->bDeviceProtocol,dev_desc->bDeviceProtocol);
		printk(KERN_WARNING "id->bInterfaceClass = 0x%x, intf_desc->bInterfaceClass = 0x%x\n",id->bInterfaceClass,intf_desc->bInterfaceClass);
		printk(KERN_WARNING "id->bInterfaceSubClass = 0x%x, intf_desc->bInterfaceSubClass = 0x%x\n",id->bInterfaceSubClass,intf_desc->bInterfaceSubClass);
		printk(KERN_WARNING "id->bInterfaceProtocol = 0x%x, intf_desc->bInterfaceProtocol = 0x%x\n",id->bInterfaceProtocol,intf_desc->bInterfaceProtocol);
		#endif
		if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
			(id->idVendor != dev_desc->idVendor))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
			(id->idProduct != dev_desc->idProduct))
			continue;
		
		/* No need to test id->bcdDevice_lo != 0, since 0 is never
		greater than any unsigned number. */
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
			(id->bcdDevice_lo > dev_desc->bcdDevice))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
			(id->bcdDevice_hi < dev_desc->bcdDevice))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
			(id->bDeviceClass != dev_desc->bDeviceClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
			(id->bDeviceSubClass!= dev_desc->bDeviceSubClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
			(id->bDeviceProtocol != dev_desc->bDeviceProtocol))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
			(id->bInterfaceClass != intf_desc->bInterfaceClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
			(id->bInterfaceSubClass != intf_desc->bInterfaceSubClass))
			continue;

		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
			(id->bInterfaceProtocol != intf_desc->bInterfaceProtocol))
			continue;

		msg = dev_info->hp_msg;

		cur_pv = (dev_desc->idVendor <<16) | dev_desc->idProduct;
		HP_DBG("%s: idx:%d,msg:%x\n", __FUNCTION__,idx,msg);
		
		if(msg == MSG_MASS_STORAGE){
			struct usb_hcd *hcd = bus_to_hcd(udev->bus);
			usb_hcd_ioctl(hcd,HCD_CMD_SET_DMA,1); //1213 usb1 dma have some problem
			usb_hcd_ioctl(hcd,HCD_CMD_REINIT_BUF,2); 
		}

		return msg;
	}
	return 0;
}
/*
  *usb hostplug information process function
  */
void usb_hotplug_msg(int plug_msg,int plugstatus,int port_num)
{
	struct am_sys_msg msg;
	if(plug_msg <= 0) 	
		return ;
	
	msg.type = SYSMSG_USB;
	msg.subtype =  plugstatus ? HOST_USB_IN : HOST_USB_OUT;
	msg.dataload = plug_msg;

	if(port_num==1)
		msg.reserved[0]=0xa0;
	else
		msg.reserved[0]=0xa1;

	msg.reserved[1] = cur_pv; //before svn8515 msg.reserved[0] storage vidpid
	printk("%s %d  port_num:%d reserved[0]=%x\n",__FUNCTION__,__LINE__,port_num,msg.reserved[0]);
	HP_DBG("%s subtype:%x msg:%x\n",__FUNCTION__,msg.subtype,plug_msg);
	am_put_sysmsg(msg); //sent  message to filesystem
}
EXPORT_SYMBOL(usb_hotplug_msg);


