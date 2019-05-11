/*
 * zero.c -- Gadget Zero, for USB development
 *
 * Copyright (C) 2003-2008 David Brownell
 * Copyright (C) 2008 by Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*
 * Gadget Zero only needs two bulk endpoints, and is an example of how you
 * can write a hardware-agnostic gadget driver running inside a USB device.
 * Some hardware details are visible, but don't affect most of the driver.
 *
 * Use it with the Linux host/master side "usbtest" driver to get a basic
 * functional test of your device-side usb stack, or with "usb-skeleton".
 *
 * It supports two similar configurations.  One sinks whatever the usb host
 * writes, and in return sources zeroes.  The other loops whatever the host
 * writes back, so the host can read it.
 *
 * Many drivers will only have one configuration, letting them be much
 * simpler if they also don't support high speed operation (like this
 * driver does).
 *
 * Why is *this* driver using two configurations, rather than setting up
 * two interfaces with different functions?  To help verify that multiple
 * configuration infrastucture is working correctly; also, so that it can
 * work with low capability USB controllers without four bulk endpoints.
 */

/*
 * driver assumes self-powered hardware, and
 * has no way for users to trigger remote wakeup.
 */

/* #define VERBOSE_DEBUG */

#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/device.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>
#include <linux/utsname.h>
#include <linux/device.h>
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <usb_subdisp.h>
#include <sys_msg.h>
#include <linux/delay.h>

#ifdef  DBG_SUBDISP
#define DBG(fmt,stuff...)   			printk(KERN_INFO fmt,##stuff)
#else
#define DBG(fmt,stuff...)   			do {} while (0)
#endif

#define ERR(fmt,stuff...)			printk(KERN_NOTICE fmt,##stuff)
#define WARNNING(fmt,stuff...)	printk(KERN_WARNING fmt,##stuff)
#define INFO(fmt,stuff...)			printk(KERN_INFO fmt,##stuff)


/*-------------------------------------------------------------------------*/

#define DRIVER_VERSION		"Cinco de Mayo 2008"
#define DRIVER_DESC			"actions usb subdisplay"

static const char longname[] 	= "actions-subdisplay";
static const char shortname[] 	= "subdisp";
static int msg_display = 0;

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("liucan");
MODULE_LICENSE("Dual BSD/GPL");

/*-------------------------------------------------------------------------*/
module_param_named(msg_display,msg_display, bool, S_IRUGO);//bq
MODULE_PARM_DESC(msg_display, "identify usb display message type");

/* Thanks to NetChip Technologies for donating this product ID.
 *
 * DO NOT REUSE THESE IDs with a protocol-incompatible driver!!  Ever!!
 * Instead:  allocate your own, using normal USB-IF procedures.
 */
#define 	DRIVER_VENDOR_NUM		0x1de1		/* OTG test device IDs */
#define 	DRIVER_PRODUCT_NUM		0x5501

/*string*/
#define STRING_MANUFACTURER		1
#define STRING_PRODUCT				2
#define STRING_SERIAL					3
#define STRING_CONFIG				4
#define STRING_INTERFACE			5

/*-------------------------------------------------------------------------*/

#define 	EP0_BUFSIZE					256	
#define	CONFIG_VALUE				1
#define 	CMD_BUFSIZE				sizeof(struct cmd_head)	


/*-------------------------------------------------------------------------*/
static struct usb_device_descriptor device_desc = {
	.bLength 				=	sizeof device_desc,
	.bDescriptorType 		=	USB_DT_DEVICE,

	.bcdUSB 			=	__constant_cpu_to_le16(0x0200),
	.bDeviceClass 		=	USB_CLASS_VENDOR_SPEC,

	.idVendor			=	__constant_cpu_to_le16(DRIVER_VENDOR_NUM),
	.idProduct 			=	__constant_cpu_to_le16(DRIVER_PRODUCT_NUM),

	.iManufacturer 		=	STRING_MANUFACTURER,
	.iProduct				=	STRING_PRODUCT,
	.iSerialNumber 		=	STRING_SERIAL,
	.bNumConfigurations	=	1,
};

static struct usb_config_descriptor config_desc = {
	.bLength 				=	sizeof config_desc,
	.bDescriptorType 		=	USB_DT_CONFIG,

	/* compute wTotalLength on the fly */
	.bNumInterfaces 		=	1,
	.bConfigurationValue 	=	CONFIG_VALUE,
	.iConfiguration 		=	STRING_CONFIG,
	.bmAttributes			=	USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower 			=	1,	/* self-powered */
};


static struct usb_otg_descriptor otg_desc = {
	.bLength 				=	sizeof otg_desc,
	.bDescriptorType 		=	USB_DT_OTG,
	.bmAttributes 			=	USB_OTG_SRP | USB_OTG_HNP,
};


static struct usb_interface_descriptor
intf_desc = {
	.bLength 				=	sizeof intf_desc,
	.bDescriptorType 		=	USB_DT_INTERFACE,

	.bNumEndpoints 		=	2,	
	.bInterfaceClass 		=	USB_CLASS_VENDOR_SPEC,
	.iInterface			=	STRING_INTERFACE,
};


static struct usb_endpoint_descriptor
fs_bulk_in_desc = {
	.bLength				=	USB_DT_ENDPOINT_SIZE,
	.bDescriptorType 		=	USB_DT_ENDPOINT,

	.bEndpointAddress 	=	USB_DIR_IN,
	.bmAttributes 			=	USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};


static struct usb_endpoint_descriptor
fs_bulk_out_desc = {
	.bLength 				=	USB_DT_ENDPOINT_SIZE,
	.bDescriptorType		=	USB_DT_ENDPOINT,

	.bEndpointAddress 	=	USB_DIR_OUT,
	.bmAttributes 			=	USB_ENDPOINT_XFER_BULK,
	/* wMaxPacketSize set by autoconfiguration */
};

#define FS_FUNCTION_PRE_EP_ENTRIES	2

static const struct usb_descriptor_header *fs_function[] = {
	(struct usb_descriptor_header *) &otg_desc,
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &fs_bulk_in_desc,
	(struct usb_descriptor_header *) &fs_bulk_out_desc,
	NULL,
};


#ifdef	CONFIG_USB_GADGET_DUALSPEED
/*
 * USB 2.0 devices need to expose both high speed and full speed
 * descriptors, unless they only run at full speed.
 *
 * That means alternate endpoint descriptors (bigger packets)
 * and a "device qualifier" ... plus more construction options
 * for the config descriptor.
 */
static struct usb_qualifier_descriptor
dev_qualifier = {
	.bLength =		sizeof dev_qualifier,
	.bDescriptorType =	USB_DT_DEVICE_QUALIFIER,

	.bcdUSB =		__constant_cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,

	.bNumConfigurations =	1,
};

static struct usb_endpoint_descriptor
hs_bulk_in_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	/* bEndpointAddress copied from fs_bulk_in_desc during fsg_bind() */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor
hs_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	/* bEndpointAddress copied from fs_bulk_out_desc during fsg_bind() */
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(512),
	.bInterval =		1,	// NAK every 1 uframe
};


static const struct usb_descriptor_header *hs_function[] = {
	(struct usb_descriptor_header *) &otg_desc,
	(struct usb_descriptor_header *) &intf_desc,
	(struct usb_descriptor_header *) &hs_bulk_in_desc,
	(struct usb_descriptor_header *) &hs_bulk_out_desc,
	NULL,
};

#define HS_FUNCTION_PRE_EP_ENTRIES	2
/* Maxpacket and other transfer characteristics vary by speed. */
#define ep_desc(g,fs,hs)	(((g)->speed==USB_SPEED_HIGH) ? (hs) : (fs))
#else
#define ep_desc(g,fs,hs)	fs
#endif	/* !CONFIG_USB_GADGET_DUALSPEED */


/* default serial number takes at least two packets */
static char				manufacturer[50];
static char serial[] = "00000000000000000000000000000000";

/* Static strings, in UTF-8 (for simplicity we use only ASCII characters) */
static struct usb_string		strings[] = {
	{STRING_MANUFACTURER,	manufacturer},
	{STRING_PRODUCT,		longname},
	{STRING_SERIAL,			serial},
	{STRING_CONFIG,			"Self-powered"},
	{STRING_INTERFACE,		"vendor subdisp"},
	{}
};


static struct usb_gadget_strings	stringtab = {
	.language	= 0x0409,		// en-us
	.strings	= strings,
};

/*-------------------------------------------------------------------------*/
struct subdisp_dev{
	spinlock_t			lock;
	struct usb_gadget		*gadget;
	
	struct usb_ep			*bulk_in;
	struct usb_ep			*bulk_out;

	/*copies from gadget ep0*/
	struct usb_ep			*ep0;	 
	struct usb_request		*ep0req;
	struct usb_request		*outreq;
	struct usb_request		*inreq;
	
	unsigned 				bulk_out_enabled:1,
						bulk_in_enabled:1,
						host_quit :1,
						disconnect:1,
						receive_switch:1;
						
	int 					outbusy;
	int 					inbusy;
							
	int 					config;
	
	char 				cmd_buf[40];
	wait_queue_head_t	rx_wait;
	wait_queue_head_t 	tx_wait;
	
	/*bit flags used to indicate dev state*/
	unsigned long			atomic_bitflags;
#define SUBDISP_DEV_NOTREADY		0
#define SUBDISP_DEV_OPEN			1
#define SUBDISP_DEV_BUSY				2
#define SUBDISP_DEV_ERR				3
#define SUBDISP_DEV_SUSPEND		4
	struct cdev			dev;
};


struct subdisp_dev * 	g_psubdisp;
static dev_t 			g_subdisp_devno;
static struct class *	usb_gadget_class;

static int subdisp_open(struct inode *inode, struct file *fd)
{
	struct subdisp_dev	*dev;
	int			ret = -EBUSY ;
	
	dev = container_of(inode->i_cdev, struct subdisp_dev, dev);
	
	spin_lock(&dev->lock);
	if (test_bit(SUBDISP_DEV_OPEN,&dev->atomic_bitflags)) {
		ERR("only opened once\n");
		goto endl;
	}
	
	set_bit(SUBDISP_DEV_OPEN, &dev->atomic_bitflags);
	fd->private_data = dev;
	ret = 0;
	DBG( "subdisp open ok\n");
	
endl:	
	spin_unlock(&dev->lock);
	DBG("printer_open returned %x\n", ret);
	return ret;
}


static int subdisp_close(struct inode *inode, struct file *fd)
{
	struct subdisp_dev	*dev = fd->private_data;
	if (!test_bit(SUBDISP_DEV_OPEN,&dev->atomic_bitflags)) {
		ERR("only close once\n");
		return -ESHUTDOWN;
	}
	
	spin_lock(&dev->lock);
	clear_bit(SUBDISP_DEV_OPEN,&dev->atomic_bitflags);
	fd->private_data = NULL;
	spin_unlock(&dev->lock);
	
	DBG( "subdisplay close \n");
	return 0;
}


static long start_transfer(struct subdisp_dev * dev,
struct usb_ep *ep,
struct usb_request *req,
int *pbusy){
	int result;
	
	set_bit(SUBDISP_DEV_BUSY,&dev->atomic_bitflags);
	*pbusy = 1;

	if((result = usb_ep_queue(ep,req,GFP_KERNEL)) < 0)
		return result;
	
	wait_event_interruptible(*(wait_queue_head_t*)req->context,
		!test_bit(SUBDISP_DEV_BUSY,&dev->atomic_bitflags));

	return req->status;
}


static int check_cmd(void *buf ,int tag,int dir)
{
	struct cmd_head * head;
 	head =(struct cmd_head *)buf ;
	int result = 0;
	if((head->tag != tag) || (head->flag != dir)){
		INFO("tag:%x,actual_tag:%x\n",tag,head->tag);
		INFO("flag:%x,actual_flag:%x\n",dir,head->flag);
		result = -EFAULT;
	}
	return result;
}


static void dump_head(struct cmd_head * head)
{
	DBG("cmd->tag:%d\n",head->tag);
	DBG("cmd->flag:%d\n",head->flag);
	DBG("cmd->len:%d\n",head->len);
}


static long subdisp_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{
	struct subdisp_dev	*dev = fd->private_data;
	struct buffer_head 	stream_buf;
	struct cmd_head		*phead;
	struct pic_fmt 		*pfmt;
	unsigned long 		size,buf;
	enum errtype 		err;
	int result = 0;

	switch(code){
	case IOCTL_SUBDISP_SET_DEVINFO:	
		if(test_bit(SUBDISP_DEV_SUSPEND,&dev->atomic_bitflags) 
			||test_bit(SUBDISP_DEV_NOTREADY,&dev->atomic_bitflags))
			return -ESHUTDOWN;
		
		INFO("%s: set device info\n",__FUNCTION__);
		
		/*get host cmd*/
		dev->outreq->buf = dev->cmd_buf;
		dev->outreq->length = CMD_BUFSIZE;
		result = start_transfer(dev,
			dev->bulk_out,
			dev->outreq,
			&dev->outbusy);
		if(result <0){
			INFO("%s: get cmd failed \n",__FUNCTION__);
			return result;	
		}
		
		result = check_cmd(dev->cmd_buf,
			TAG_DEVINFO_CMD,0);
		if(result < 0){
			INFO("%s: check cmd failed\n",__FUNCTION__);
			return result;
		}
	
		phead =(struct cmd_head *)dev->cmd_buf;
		phead->flag = 0x01;
		phead->len  = sizeof(struct pic_fmt);
		
		copy_from_user(phead->cdb, (const void *)arg, phead->len);
		//dump_head(phead);
		
		/*submit device info to host*/
		dev->inreq->buf = phead;	
		dev->inreq->length = CMD_BUFSIZE;
		
		result = start_transfer(dev,
			dev->bulk_in,
			dev->inreq,
			&dev->inbusy);
		if(result <0){
			INFO("%s: send status cmd failed\n",__FUNCTION__);
			return result;
		}	
		break;
		
	case IOCTL_SUBDISP_GET_IMAGE:	
		/*device may be err*/
		if(test_bit(SUBDISP_DEV_SUSPEND,&dev->atomic_bitflags)
			||test_bit(SUBDISP_DEV_NOTREADY,&dev->atomic_bitflags))
			return -ESHUTDOWN;
		
		copy_from_user(&stream_buf,(const void*)arg,sizeof(stream_buf));
		BUG_ON(stream_buf.size < CMD_BUFSIZE);
		
		buf = stream_buf.buffer;
		size = stream_buf.size;

		DBG("%s: get image,phy_buf:%x,size:%x\n",
			__FUNCTION__,buf,size);

		/*get image head*/
		phead = (struct cmd_head *)dev->cmd_buf;
		dev->outreq->buf = phead;
		dev->outreq->length = CMD_BUFSIZE;
		
		result = start_transfer(dev,
			dev->bulk_out,
			dev->outreq,
			&dev->outbusy);
		if(result <0){
			ERR("get pic fmt failed,result:%d\n",result);
			return result;
		}
		
		//dump_head(phead);
		BUG_ON(dev->outreq->actual != CMD_BUFSIZE);	
		result = check_cmd(dev->cmd_buf,
			TAG_PICFMT_CMD,0);
		if(result < 0){
			ERR("%s: check picfmt failed\n",__FUNCTION__);
			return result;
		}	
		
		pfmt   = (struct pic_fmt *)phead->cdb;	
		BUG_ON(pfmt->isize >size);	
		memcpy((void *)buf,pfmt,sizeof(struct pic_fmt));
		
		/*get image data*/
		dev->outreq->buf = (void *)(buf + sizeof(struct pic_fmt)) ;
		dev->outreq->length = pfmt->isize;	
		result = start_transfer(dev,
			dev->bulk_out,
			dev->outreq,
			&dev->outbusy);
		if(result <0){
			ERR("get data failed,result:%d\n",result);
			return result;
		}
		break;
		
	case IOCTL_SUBDISP_GET_LASTERR:	
		if(!test_bit(SUBDISP_DEV_SUSPEND,&dev->atomic_bitflags))
			return 0;
	
		if(dev->host_quit){
			err = ERR_QUIT;
		}else if(dev->receive_switch){
			err = ERR_SWITCH;
		}else if(dev->disconnect){
			err = ERR_DISCON;
		}else
			err = ERR_NONE;		
		
		copy_to_user((void *)arg,&err,sizeof(err));
		result = 0;
		break;
	default:
		result = -ENOTSUPP;
		break;
	}
	
	return result;
}


static struct file_operations subdisp_io_operations = {
	.owner =	THIS_MODULE,
	.open 			=	subdisp_open,
	.unlocked_ioctl 	= 	subdisp_ioctl,
	.release 			=	subdisp_close
};


static int enable_endpoint(struct subdisp_dev *dev, struct usb_ep *ep,
		const struct usb_endpoint_descriptor *d)
{
	int	rc;

	ep->driver_data = dev;
	INFO("%s:%d\n",__FUNCTION__,__LINE__);
	rc = usb_ep_enable(ep, d);
	if (rc)
		ERR("can't enable %s, result %d\n", ep->name, rc);
	return rc;
}


typedef struct usb_ctrlrequest subdisp_vendor_request;
#define VENDOR_CMDSIZE 	sizeof(subdisp_vendor_request)


enum vendor_cmd{
 	VND_CMD_SWITCH= 0x50,
	VND_CMD_QUIT	 = 0x51
};


static  int get_vendor_cmd(struct usb_request *req,enum vendor_cmd *cmd)
{	
	subdisp_vendor_request * pkt;
	if(req->actual != VENDOR_CMDSIZE )
		return -EINVAL;
	
	pkt = (subdisp_vendor_request *)req->buf;
	if((pkt->bRequestType & USB_TYPE_VENDOR)
		!=USB_TYPE_VENDOR)
		return -EINVAL;
	
	*cmd = (enum vendor_cmd)pkt->bRequest;
	return 0;
}


static int subdisp_set_config(struct subdisp_dev *dev, unsigned number);
static void bulk_out_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct subdisp_dev	*dev = ep->driver_data;
	enum vendor_cmd cmd;
	struct am_sys_msg msg; 
	unsigned long flags;
	switch (req->status) {
	default:
		ERR("tx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */	
		ERR("tx disc or unlink\n");
		break;
	case 0:
		DBG("tx ok\n");
		break;
	}	

	if(!get_vendor_cmd(req,&cmd)){
		switch(cmd){
		case VND_CMD_SWITCH:
			spin_lock_irqsave(&dev->lock,flags);
			req->status = -ESHUTDOWN;
			set_bit(SUBDISP_DEV_SUSPEND,&dev->atomic_bitflags);
			dev->receive_switch=1;
			spin_unlock_irqrestore(&dev->lock,flags);

			usb_gadget_disconnect(dev->gadget);
			mdelay(10);
			/*let msg to let ap know*/
#if 1			
			msg.type       = SYSMSG_USB;
			msg.subtype  = DEVICE_USB_VENDOR;
			msg.dataload = DEVICE_SUBDISP_2_MASS;
			if(msg_display == 0){
				msg.reserved[0]=0xa0;
			}else if(msg_display==1){
				msg.reserved[0]=0xa1;
			}
			am_put_sysmsg(msg);	
			INFO("issue usb switch msg\n");
#endif			
			break;
		case VND_CMD_QUIT:
			spin_lock_irqsave(&dev->lock,flags);
			req->status = -ESHUTDOWN;
			set_bit(SUBDISP_DEV_SUSPEND,&dev->atomic_bitflags);
			dev->host_quit =1;
			spin_unlock_irqrestore(&dev->lock,flags);

			/*set config to 0 to deinit resource*/
			//subdisp_set_config(dev,0);
			/*let msg to let ap know*/
			usb_gadget_disconnect(dev->gadget);
			mdelay(10);
#if 1			
			msg.type       = SYSMSG_USB;
			msg.subtype  = DEVICE_USB_VENDOR;
			msg.dataload = DEVICE_SUBDISP_QUIT;
			if(msg_display == 0){
				msg.reserved[0]=0xa0;
			}else if(msg_display==1){
				msg.reserved[0]=0xa1;
			}
			am_put_sysmsg(msg);	
			INFO("issue usb quit msg\n");
#endif			
			break;
		default:
			INFO("invalid vendor cmd\n");
			break;
		}
	}

	clear_bit(SUBDISP_DEV_BUSY,&dev->atomic_bitflags);
	wake_up_interruptible((wait_queue_head_t*)req->context);
}


static void bulk_in_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct subdisp_dev	*dev = ep->driver_data;
	switch (req->status) {
	default:
		INFO("rx err %d\n", req->status);
		/* FALLTHROUGH */
	case -ECONNRESET:		/* unlink */
	case -ESHUTDOWN:		/* disconnect etc */	
		INFO("rx disc or unlink\n");
		break;
	case 0:
		DBG("rx ok\n");
		break;
	}
	
	clear_bit(SUBDISP_DEV_BUSY,&dev->atomic_bitflags);
	wake_up_interruptible((wait_queue_head_t*)req->context);
}


static int subdisp_set_config(struct subdisp_dev *dev, unsigned number)
{
	int		result = 0;
	struct am_sys_msg msg;
	char 	* speed;
	struct usb_gadget	*gadget = dev->gadget;
	const struct usb_endpoint_descriptor	*d;

	if (number == dev->config)
		return 0;
	
	DBG("%s : config_num:%d\n ",__FUNCTION__,
		number);
disable_ep:	
	if (dev->bulk_in_enabled) {
		usb_ep_disable(dev->bulk_in);
		dev->bulk_in_enabled = 0;
	}

	if(dev->bulk_out_enabled){
		usb_ep_disable(dev->bulk_out);
		dev->bulk_out_enabled = 0;
	}
	
	dev->config = 0;
	if(result != 0)
		return result;
      
	switch (number) {
	case CONFIG_VALUE:
		/* one endpoint writes (sources) zeroes in (to the host) */
		/* Enable the endpoints */
		INFO("enable bulk  in\n");
		d = ep_desc(dev->gadget, &fs_bulk_in_desc, 
				&hs_bulk_in_desc);	
		if ((result = enable_endpoint(dev, dev->bulk_in, d)) != 0)
			goto disable_ep;
		dev->bulk_in_enabled = 1;

		INFO("enable bulk  out\n");
		d = ep_desc(dev->gadget, &fs_bulk_out_desc,
				&hs_bulk_out_desc);
		if ((result = enable_endpoint(dev, dev->bulk_out, d)) != 0)
			goto disable_ep;
		dev->bulk_out_enabled = 1;
		DBG("enable ep ok\n");

		clear_bit(SUBDISP_DEV_NOTREADY,&dev->atomic_bitflags);
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_VENDOR;
		msg.dataload = DEVICE_SUBDISPLAY;
		if(msg_display == 0){
			msg.reserved[0]=0xa0;
		}else if(msg_display==1){
			msg.reserved[0]=0xa1;
		}
		am_put_sysmsg(msg);
		
		break;
	default:
		result = -EINVAL;
		/* FALL THROUGH */
	case 0:
		return result;
	}
	
	switch (gadget->speed) {
		case USB_SPEED_LOW:	speed = "low"; break;
		case USB_SPEED_FULL:	speed = "full"; break;
		case USB_SPEED_HIGH:	speed = "high"; break;
		default:		speed = "?"; break;
	}
	
	dev->config = number;
	DBG("%s speed config #%d\n", speed, number);
	return result;
}


static int populate_config_buf(struct usb_gadget *gadget,
		u8 *buf, u8 type, unsigned index)
{
	enum usb_device_speed			speed = gadget->speed;
	int					len;
	const struct usb_descriptor_header	**function;

	if (index > 0)
		return -EINVAL;

	if (/*gadget_is_dualspeed(gadget) &&*/ type == USB_DT_OTHER_SPEED_CONFIG)
		speed = (USB_SPEED_FULL + USB_SPEED_HIGH) - speed;
	if (/*gadget_is_dualspeed(gadget) && */speed == USB_SPEED_HIGH)
		function = hs_function;
	else
		function = fs_function;

	/* for now, don't advertise srp-only devices */
	if (!gadget_is_otg(gadget))
		function++;

	len = usb_gadget_config_buf(&config_desc, buf, EP0_BUFSIZE, function);
	((struct usb_config_descriptor *) buf)->bDescriptorType = type;
	return len;
}



static int subdisp_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	struct subdisp_dev	*psub = get_gadget_data(gadget);
	struct usb_request		*req = psub->ep0req;
	
	int			value		 = -EOPNOTSUPP;
	u16			w_index 	 = __le16_to_cpu(ctrl->wIndex);
	u16			w_value 	 = __le16_to_cpu(ctrl->wValue);
	u16			w_length 	 = __le16_to_cpu(ctrl->wLength);
	
	/* usually this stores reply data in the pre-allocated ep0 buffer,
	 * but config change events will reconfigure hardware.
	 */
	req->zero = 0;
	switch (ctrl->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		if (ctrl->bRequestType != USB_DIR_IN)
			goto unknown;
		switch (w_value >> 8) {
			case USB_DT_DEVICE:
			value = min(w_length, (u16) sizeof device_desc);
			memcpy(req->buf, &device_desc, value);
				break;
			case USB_DT_DEVICE_QUALIFIER:
			if (!gadget_is_dualspeed(gadget))
				break;
			value = min(w_length, (u16) sizeof dev_qualifier);
			memcpy(req->buf, &dev_qualifier, value);
				break;

			case USB_DT_OTHER_SPEED_CONFIG:
			if (!gadget_is_dualspeed(gadget))
				break;
			// FALLTHROUGH
			case USB_DT_CONFIG:
			value = populate_config_buf(gadget,
					req->buf,
					w_value >> 8,
					w_value & 0xff);
			if (value >= 0)
				value = min(w_length, (u16) value);
				break;

			case USB_DT_STRING:
			/* wIndex == language code.
			 * this driver only handles one language, you can
			 * add string tables for other languages, using
			 * any UTF-8 characters
			 */
			value = usb_gadget_get_string(&stringtab,
					w_value & 0xff, req->buf);
			if (value >= 0)
				value = min(w_length, (u16) value);
				break;
			default:
				break;
		}
		break;

	case USB_REQ_SET_CONFIGURATION:
		if (ctrl->bRequestType != 0)
			goto unknown;
		
		spin_lock(&psub->lock);
		value = subdisp_set_config(psub, w_value);
		spin_unlock(&psub->lock);
		
		INFO("%s:%d set_config=%d\n",
			__FUNCTION__,__LINE__,w_value);
		break;
		
	case USB_REQ_GET_CONFIGURATION:
		if (ctrl->bRequestType != USB_DIR_IN)
			goto unknown;
		*(u8 *)req->buf = psub->config;
		value = min(w_length, (u16) 1);
		break;

	/* until we add altsetting support, or other interfaces,
	 * only 0/0 are possible.  pxa2xx only supports 0/0 (poorly)
	 * and already killed pending endpoint I/O.
	 */
	case USB_REQ_SET_INTERFACE:
		if (ctrl->bRequestType != USB_RECIP_INTERFACE)
			goto unknown;
		spin_lock(&psub->lock);
		if (psub->config && w_index == 0 && w_value == 0) {
			u8	config = psub->config;

			/* resets interface configuration, forgets about
			 * previous transaction state (queued bufs, etc)
			 * and re-inits endpoint state (toggle etc)
			 * no response queued, just zero status == success.
			 * if we had more than one interface we couldn't
			 * use this "reset the config" shortcut.
			 */
			subdisp_set_config(psub, config);
			value = 0;
		}
		spin_unlock(&psub->lock);
		break;
	case USB_REQ_GET_INTERFACE:
		if (ctrl->bRequestType != (USB_DIR_IN|USB_RECIP_INTERFACE))
			goto unknown ;
		
		if (!psub->config)
			break;
		
		if (w_index != 0) {
			value = -EDOM;
			break;
		}
		
		*(u8 *)req->buf = 0;
		value = min(w_length, (u16) 1);
		break;
	default:
unknown:		
		DBG("unknown control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		break;
	}

	/* respond with data transfer before status phase? */
	if (value >= 0) {
		req->length = value;
		req->zero = value < w_length;
		value = usb_ep_queue(gadget->ep0, req, GFP_ATOMIC);
		if (value < 0) {
			ERR("ep0 queue failed,value: %d\n", value);
			//req->status = 0;
			//zero_setup_complete(gadget->ep0, req);
		}
	}
	
	/* device either stalls (value < 0) or reports success */
	return value;
}



static void subdisp_disconnect(struct usb_gadget *gadget)
{
	struct subdisp_dev	*dev = get_gadget_data(gadget);
	unsigned long		flags;
	struct am_sys_msg msg;
	spin_lock_irqsave(&dev->lock, flags);
	set_bit(SUBDISP_DEV_SUSPEND,&dev->atomic_bitflags);
	dev->disconnect = 1;
	subdisp_set_config(dev,0);
	/* a more significant application might have some non-usb
	 * activities to quiesce here, saving resources like power
	 * or pushing the notification up a network stack.
	 */
	spin_unlock_irqrestore(&dev->lock, flags);
#if 1			
	msg.type       = SYSMSG_USB;
	msg.subtype  = DEVICE_USB_VENDOR;
	msg.dataload = DEVICE_SUBDISP_QUIT;
	if(msg_display == 0){
		msg.reserved[0]=0xa0;
	}else if(msg_display==1){
		msg.reserved[0]=0xa1;
	}
	am_put_sysmsg(msg);	
	INFO("usb-subdisplay plug out msg\n");
#endif			
	/* next we may get setup() calls to enumerate new connections;
	 * or an unbind() during shutdown (including removing module).
	 */
}

static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		DBG("%s --> %d, %u/%u\n", __FUNCTION__,
				req->status, req->actual, req->length);
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);
}


static void /* __init_or_exit */ subdisp_unbind(struct usb_gadget *gadget)
{
	struct subdisp_dev	*dev = get_gadget_data(gadget);
	struct usb_request	*req = NULL;	
	DBG("unbind\n");
	
	/* Free the request and buffer for endpoint 0 */
	req = dev->ep0req;
	if (req) {
		kfree(req->buf);
		usb_ep_free_request(dev->ep0, req);
	}
	
	req = dev->inreq;
	if(req){
		if (dev->inbusy)
			usb_ep_dequeue(dev->bulk_in,req);
		usb_ep_free_request(dev->bulk_in,req);	
	}
	
	req = dev->outreq;
	if(req){
		if (dev->outbusy)
			usb_ep_dequeue(dev->bulk_out,req);
		usb_ep_free_request(dev->bulk_out,req);
	}
	subdisp_set_config(dev,0);

	/* Remove Character Device */
	cdev_del(&dev->dev);
	set_gadget_data(gadget, NULL);
}


static int __init subdisp_bind(struct usb_gadget *gadget)
{
	struct 	subdisp_dev	*dev = g_psubdisp;
	int 	rc,i;
	struct usb_ep			*ep;
	struct usb_request		*req;

	dev->gadget = gadget;
	set_gadget_data(gadget, dev);
	dev->ep0 = gadget->ep0;
	dev->ep0->driver_data = dev;
	
	/*
	 * Register a character device as an interface to a user mode
	 * program that handles the subdisp specific functionality.
	*/ 
	
	cdev_init(&dev->dev, &subdisp_io_operations);
	dev->dev.owner = THIS_MODULE;
	rc= cdev_add(&dev->dev, g_subdisp_devno, 1);
	if (rc) {
		ERR( "Failed to create char device\n");
		goto out;
	}
	set_bit(SUBDISP_DEV_NOTREADY, &dev->atomic_bitflags);
	
	/* autoconfig all the endpoints we will use */
	usb_ep_autoconfig_reset(gadget);
	ep = usb_ep_autoconfig(gadget, &fs_bulk_in_desc);
	if (!ep)
		goto autoconf_fail;	
	ep->driver_data = dev;		// claim the endpoint
	dev->bulk_in = ep;
	
	ep = usb_ep_autoconfig(gadget, &fs_bulk_out_desc);
	if (!ep)
		goto autoconf_fail;
	ep->driver_data = dev;		// claim the endpoint
	dev->bulk_out = ep;

	/*have 2 eps for bulk only*/
	device_desc.bMaxPacketSize0	= dev->ep0->maxpacket;
	intf_desc.bNumEndpoints = i =  2;
	fs_function[i + FS_FUNCTION_PRE_EP_ENTRIES] = NULL;

#ifdef   CONFIG_USB_GADGET_DUALSPEED
	hs_function[i + HS_FUNCTION_PRE_EP_ENTRIES] = NULL;
	/* Assume ep0 uses the same maxpacket value for both speeds */
	dev_qualifier.bMaxPacketSize0 		= dev->ep0->maxpacket;
	/* Assume endpoint addresses are the same for both speeds */
	hs_bulk_in_desc.bEndpointAddress 	=fs_bulk_in_desc.bEndpointAddress;
	hs_bulk_out_desc.bEndpointAddress =fs_bulk_out_desc.bEndpointAddress;
#endif

	if (gadget_is_otg(gadget))
		otg_desc.bmAttributes |= USB_OTG_HNP;

	rc = -ENOMEM;
	/* Allocate the request and buffer for endpoint 0 */
	dev->ep0req = req = usb_ep_alloc_request(dev->ep0,
			GFP_KERNEL);
	if (!req)
		goto out;
	req->buf = kzalloc(EP0_BUFSIZE, GFP_KERNEL);
	if (!req->buf)	
		goto out;
	req->complete = ep0_complete;
	
	/*alloc request for bulk in and out*/
	dev->outreq = req = usb_ep_alloc_request(dev->bulk_out,
			GFP_KERNEL);
	if(!req)
		goto out;
	req->complete = bulk_out_complete;
	req->context = &dev->tx_wait;

	dev->inreq  = req = usb_ep_alloc_request(dev->bulk_in,
			GFP_KERNEL);
	if(!req)
		goto out;
	req->complete = bulk_in_complete;
	req->context = &dev->rx_wait;

	usb_gadget_set_selfpowered(gadget);
	sprintf(manufacturer,"actions-micro");
	INFO("subdisp bind ok\n");
	
	return 0;
	
autoconf_fail:
	ERR("unable to autoconfigure all endpoints\n");
	rc = -ENOTSUPP;
	
out:
	ERR("bind failed,%d \n",rc);
	subdisp_unbind(gadget);
	return rc;
}



static void subdisp_suspend(struct usb_gadget *gadget)
{
	//struct subdisp_dev		*dev = get_gadget_data(gadget);
	DBG("suspend\n");
}


static void subdisp_resume(struct usb_gadget *gadget)
{
	//struct subdisp_dev		*dev = get_gadget_data(gadget);
	DBG("resume\n");
}

/*-------------------------------------------------------------------------*/
static struct usb_gadget_driver		subdisp_driver = {
#ifdef CONFIG_USB_GADGET_DUALSPEED
	.speed		= USB_SPEED_HIGH,
#else
	.speed		= USB_SPEED_FULL,
#endif

	.function		= (char*)longname,
	.bind		= subdisp_bind,
	.unbind		= subdisp_unbind,
	.disconnect	= subdisp_disconnect,
	.setup		= subdisp_setup,
	
	.suspend		= subdisp_suspend,
	.resume		= subdisp_resume,

	.driver		= {
		.name		= shortname,
		.owner		= THIS_MODULE,
		// .release = ...
		// .suspend = ...
		// .resume = ...
	},
};

static int __init init(void)
{
	int result;
	struct subdisp_dev *pdev ;
	
	pdev = kzalloc(sizeof(struct subdisp_dev),GFP_KERNEL);
	if(!pdev)
		return -ENOMEM;

	pdev->bulk_in_enabled =
		pdev->bulk_out_enabled = 0;
	
	spin_lock_init(&pdev->lock);
	init_waitqueue_head(&pdev->rx_wait);
	init_waitqueue_head(&pdev->tx_wait);
	g_psubdisp = pdev;

	usb_gadget_class = class_create(THIS_MODULE,"usb-subdisp-gadget");
	if (IS_ERR(usb_gadget_class)) {
		result = PTR_ERR(usb_gadget_class);
		ERR("unable to create usb_gadget class %d\n", result);
		goto failed;
	}

	if ( (result = alloc_chrdev_region(&g_subdisp_devno, 
			0, 1,"usb_subdisp")) < 0) {
		ERR("alloc_chrdev_region %d\n", result);
		goto failed1;
	}
	
	device_create_drvdata(usb_gadget_class, NULL,
				      g_subdisp_devno,
				      NULL, "usb_subdisp");
	
	if((result = usb_gadget_register_driver(&subdisp_driver) )< 0){
		ERR("subdisplay driver init failed\n");
		goto failed2;
	}
	INFO("subdisp init ok\n");
	return 0;
failed2:
	unregister_chrdev_region(g_subdisp_devno, 1);
failed1:
	class_destroy(usb_gadget_class);
failed:
	kfree(pdev);
	g_psubdisp = pdev = NULL;
	return result;
}
module_init(init);

static void __exit cleanup(void)
{		 
	 device_destroy(usb_gadget_class,g_subdisp_devno);
	 unregister_chrdev_region(g_subdisp_devno, 2);
	 class_destroy(usb_gadget_class);
	 usb_gadget_unregister_driver(&subdisp_driver);	 
	 if(g_psubdisp){
	 	kfree(g_psubdisp);
		g_psubdisp = NULL;
	 }	
}
module_exit(cleanup);


