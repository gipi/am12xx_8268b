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
#include <linux/moduleparam.h>
#include <asm/uaccess.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/proc_fs.h>

#include <sys_msg.h>
#include <linux/delay.h>
#include <usb_debug.h>
#include "file_debug.h"
/*-------------------------------------------------------------------------*/

#define DRIVER_VERSION		"/2011/06/09"
#define DRIVER_DESC			"actions usb debug"

static const char longname[] 	= "actions-usb_debug";
static const char shortname[] 	= "usb_debug";


MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("kewen");
MODULE_LICENSE("Dual BSD/GPL");

/*-------------------------------------------------------------------------*/

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
#define STRING_SERIAL				3
#define STRING_CONFIG				4
#define STRING_INTERFACE			5

/*-------------------------------------------------------------------------*/

#define 	EP0_BUFSIZE				256	
#define	CONFIG_VALUE				1


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
	{STRING_INTERFACE,		"vendor debug"},
	{}
};


static struct usb_gadget_strings	stringtab = {
	.language	= 0x0409,		// en-us
	.strings	= strings,
};

/*-------------------------------------------------------------------------*/

struct usb_debug_dev * 	g_pusb_debug;
static dev_t 			g_usb_debug_devno;
static struct class *	usb_gadget_class;

typedef struct usb_ctrlrequest usb_debug_vendor_request;
#define VENDOR_CMDSIZE 	sizeof(usb_debug_vendor_request)

#ifdef COFIG_PROC_FOR_USB_DEBUG
#include <linux/seq_file.h>
static const char debug_filename[] = "driver/udb";
static int proc_debug_show(struct seq_file *s, void *unused)
{
	struct usb_debug_dev	*dev = s->private;
	seq_printf(s, "\n%s: version %s\n", DRIVER_DESC, DRIVER_VERSION);

	seq_printf(s, "\nbuf status:\n");
	seq_printf(s, "\n: buf[0]:%x",dev->buffhds[0].state);
	seq_printf(s, "\n: buf[1]:%x",dev->buffhds[1].state);
	seq_printf(s, "\n: current buf:%x",dev->next_buffhd_to_fill->state);
	seq_printf(s,"\n********************************\n");
	return 0;
}

static int proc_uoc_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_debug_show, PDE(inode)->data);
}

static const struct file_operations proc_ops = {
	.owner		= THIS_MODULE,
	.open		= proc_uoc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static void create_debug_file(struct usb_debug_dev *dev)
{
	dev->pde = proc_create_data(debug_filename, 0, NULL, &proc_ops, dev);
}

static void remove_debug_file(struct usb_debug_dev *dev)
{
	if(dev->pde)
		remove_proc_entry(debug_filename, NULL);
}
#else
static  void create_debug_file(struct aotg_uoc *uoc) {}
static  void remove_debug_file(struct aotg_uoc *uoc) {}
#endif

static void raise_thread_exception(struct usb_debug_dev *dev,enum dbg_state newstate);

static int exception_in_thread(struct usb_debug_dev *dev)
{
	return (dev->state > DBG_STATE_IDLE);
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


static int sleep_debug_thread(struct usb_debug_dev *udev)
{
	int	rc = 0;
	/* Wait until a signal arrives or we are woken up */
	for (;;) {
		try_to_freeze();
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current)) {
			rc = -EINTR;
			break;
		}
		if (udev->thread_wakeup_needed)
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);
	udev->thread_wakeup_needed = 0;
	return rc;
}

static void wakeup_debug_thread(struct usb_debug_dev *udev)
{
	/* Tell the main thread that something has happened */
	udev->thread_wakeup_needed = 1;
	if (udev->thread_task)
		wake_up_process(udev->thread_task);
		
}


static int start_debug_transfer(struct usb_debug_dev * dev,struct usb_ep *ep,
	struct usb_request *req,int *pbusy,enum dbg_buffer_state *state)
{
	int result;
	
	spin_lock_irq(&dev->lock);
	*pbusy = 1;
	*state = BUF_STATE_BUSY;
	spin_unlock_irq(&dev->lock);	
	
	result = usb_ep_queue(ep,req,GFP_KERNEL);

	if(result!=0){
		*pbusy = 0;
		*state = BUF_STATE_EMPTY;
		if (result != -ESHUTDOWN && !(result == -EOPNOTSUPP && req->length == 0))
			ERR( "error in submission: %s --> %d\n",ep->name, result);
	}

	return result;
}

static int enable_endpoint(struct usb_debug_dev *dev, struct usb_ep *ep,
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

static  int usb_debug_get_vendor_cmd(struct usb_request *req,enum vendor_cmd *cmd)
{	
	usb_debug_vendor_request * pkt;
	if(req->actual != VENDOR_CMDSIZE )
		return -EINVAL;
	
	pkt = (usb_debug_vendor_request *)req->buf;
	if((pkt->bRequestType & USB_TYPE_VENDOR)
		!=USB_TYPE_VENDOR)
		return -EINVAL;
	
	*cmd = (enum vendor_cmd)pkt->bRequest;
	return 0;
}

static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status || req->actual != req->length)
		DBG("%s --> %d, %u/%u\n", __FUNCTION__,
				req->status, req->actual, req->length);
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);
}

static void bulk_out_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_debug_dev	*dev = ep->driver_data;
	struct dbg_buffhd	*bh = req->context;
	struct am_sys_msg msg; 
	enum vendor_cmd cmd;
	unsigned long flags;
	
	
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);
	if(!usb_debug_get_vendor_cmd(req,&cmd)){
		switch(cmd){
		case VND_CMD_SWITCH:
			spin_lock_irqsave(&dev->lock,flags);
			req->status = -ESHUTDOWN;
			set_bit(USB_DEBUG_DEV_SUSPEND,&dev->atomic_bitflags);
			dev->receive_switch=1;
			spin_unlock_irqrestore(&dev->lock,flags);

			usb_gadget_disconnect(dev->gadget);
			mdelay(10);
			/*let msg to let ap know*/
#if 1			
			msg.type       = SYSMSG_USB;
			msg.subtype  = DEVICE_USB_VENDOR;
			msg.dataload = DEVICE_DEBUG_2_MASS;
			msg.reserved[0]=0xa0;
			am_put_sysmsg(msg);	
			INFO("issue usb switch msg\n");
#endif			
			break;
		case VND_CMD_QUIT:
			spin_lock_irqsave(&dev->lock,flags);
			req->status = -ESHUTDOWN;
			set_bit(USB_DEBUG_DEV_SUSPEND,&dev->atomic_bitflags);
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
			msg.dataload = DEVICE_DEBUG_QUIT;
			msg.reserved[0] = 0xa0;
			am_put_sysmsg(msg);	
			INFO("issue usb quit msg\n");
#endif			
			break;
		default:
			INFO("invalid vendor cmd\n");
			break;
		}
	}
	
	smp_wmb();
	spin_lock(&dev->lock);
	bh->outreq_busy = 0;
	bh->state = BUF_STATE_FULL;
	wakeup_debug_thread(dev);
	spin_unlock(&dev->lock);

}


static void bulk_in_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_debug_dev	*dev = ep->driver_data;
	struct dbg_buffhd	*bh = req->context;
	
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);
	
	spin_lock(&dev->lock);
	bh->inreq_busy = 0;
	bh->state = BUF_STATE_EMPTY;
	clear_bit(DEBUG_DATA_READY,&dev->trans_bitflags);
	wakeup_debug_thread(dev);
	spin_unlock(&dev->lock);	
}


static int usb_debug_intr_config(struct usb_debug_dev *dev, unsigned number)
{
	int		result = 0;
	//struct am_sys_msg msg;
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
		
		dev->running = 1;
		wakeup_debug_thread(dev);
		
		clear_bit(USB_DEBUG_DEV_NOTREADY,&dev->atomic_bitflags);
		//msg.type = SYSMSG_USB;
		//msg.subtype = DEVICE_USB_VENDOR;
		//msg.dataload = DEVICE_SUBDISPLAY;
		//msg.reserved=0xa0;
		//am_put_sysmsg(msg);
		
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





static int usb_debug_setup(struct usb_gadget *gadget, const struct usb_ctrlrequest *ctrl)
{
	struct usb_debug_dev	*psub = get_gadget_data(gadget);
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
		value = usb_debug_intr_config(psub, w_value);
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
			usb_debug_intr_config(psub, config);
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



static void usb_debug_disconnect(struct usb_gadget *gadget)
{
	struct usb_debug_dev	*dev = get_gadget_data(gadget);
	unsigned long		flags;
	spin_lock_irqsave(&dev->lock, flags);
	set_bit(USB_DEBUG_DEV_SUSPEND,&dev->atomic_bitflags);
	dev->disconnect = 1;
	//usb_debug_intr_config(dev,0);
	/* a more significant application might have some non-usb
	 * activities to quiesce here, saving resources like power
	 * or pushing the notification up a network stack.
	 */
	spin_unlock_irqrestore(&dev->lock, flags);

	raise_thread_exception(dev,DBG_STATE_DISCONNECT);
	
	/* next we may get setup() calls to enumerate new connections;
	 * or an unbind() during shutdown (including removing module).
	 */
}


static int receive_debug_cbw(struct usb_debug_dev *dev, struct dbg_buffhd *bh)
{
	struct usb_request	*req = bh->outreq;
	struct _info_head		*cbw = req->buf;
	/* Is the CBW valid? */
	if (req->actual != sizeof(struct _info_head))  {
		ERR("actual size=%d\n",req->actual);
		return -EINVAL;
	}

	/* Save the command for later */
	dev->cmnd_size = cbw->cdblen;
	memcpy(dev->cmnd, cbw->cdb, MAX_DEBUG_COMMAND);//dev->cmnd_size
	//check data dir from flag
	if (cbw->flag & USB_BULK_IN_FLAG)
		dev->data_dir = DATA_DIR_TO_HOST;
	else
		dev->data_dir = DATA_DIR_FROM_HOST;
	//check multi data info from flag & cdb
	if((cbw->flag & USB_DATA_TYPE_FLAG)>>4)
		dev->bulk_type = BULK_MULTI_DATA;
	else
		dev->bulk_type = BULK_SINGLE_DATA;

	
	dev->data_size = le32_to_cpu(cbw->dataprocess.datalen);
	if (dev->data_size == 0)
		dev->data_dir = DATA_DIR_NONE;

	dev->cmd_type = cbw->cmd_type;
	dev->tag = cbw->tag;

#if 0
	unsigned char *buf;
	int i;
	buf = (unsigned char*)cbw;
	for(i=0;i<sizeof(struct _info_head);i++){
		printk("%x ",buf[i]);	
	}
	printk("\n");
#endif

#ifndef TRANSFER_ONLY_IN_KERNEL
	set_bit(DEBUG_CMD_READY,&dev->trans_bitflags);
	wake_up_interruptible(&dev->cmd_wait);
#endif
	return 0;
}

static int get_debug_command(struct usb_debug_dev *dev)
{
	int result;
	struct dbg_buffhd	*bh;
	int			rc = 0;

	INFO("get host cmd\n");
	
	/* Wait for the next buffer to become available */
	
	bh = dev->next_buffhd_to_fill;
	while (bh->state != BUF_STATE_EMPTY) {
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
	
	/* Queue a request to read a Bulk-only CBW */
	bh->outreq->length = sizeof(struct _info_head);
	result = start_debug_transfer(dev,dev->bulk_out,bh->outreq,&bh->outreq_busy,&bh->state);
	/* We will drain the buffer in software, which means we
	 * can reuse it for the next filling.  No need to advance
	 * next_buffhd_to_fill. */

	/* Wait for the CBW to arrive */
	while (bh->state != BUF_STATE_FULL) {
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
	
	smp_rmb();
	rc = receive_debug_cbw(dev,bh);
	bh->state = BUF_STATE_EMPTY;
	return rc;
	
}
static int send_debug_status(struct usb_debug_dev *dev)
{
	struct dbg_buffhd	*bh;
	struct _info_head	*csw;
	int			rc;

	bh = dev->next_buffhd_to_fill;
	while (bh->state != BUF_STATE_EMPTY){
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
#ifndef TRANSFER_ONLY_IN_KERNEL
	wait_event_interruptible(dev->csw_wait, test_bit(DEBUG_CSW_READY,&dev->trans_bitflags));
	clear_bit(DEBUG_CSW_READY,&dev->trans_bitflags);
#endif
	INFO("send csw to host\n");
	/* Wait for the next buffer to become available */
	
	csw = (struct _info_head*)bh->inreq->buf;
	memset(csw,0,sizeof(struct _info_head));
	csw->cmd_type = dev->cmd_type;	//command process good
	csw->tag = dev->tag;
	csw->dataprocess.residue = dev->residue;
	csw->cdblen = dev->cmnd_size;
	memcpy(csw->cdb,dev->cmnd,sizeof(dev->cmnd));
	bh->state  = BUF_STATE_FULL;
	
	
#if 0
	unsigned char *buf;
	int i;
	buf = (unsigned char*)csw;
	for(i=0;i<sizeof(struct _info_head);i++){
		printk("%x ",buf[i]);	
	}
	printk("\n");
#endif

	bh->inreq->length = sizeof(struct _info_head);;	
	start_debug_transfer(dev, dev->bulk_in, bh->inreq,
			&bh->inreq_busy, &bh->state);
	dev->next_buffhd_to_fill = bh->next;
	return 0;	
}

static int check_debug_command(struct usb_debug_dev *dev, int cmnd_size,
		enum data_direction data_dir, const char *name)
{
	return 0;
}
static int finish_debug_reply(struct usb_debug_dev *dev)
{
	struct dbg_buffhd	*bh = dev->next_buffhd_to_fill;
	int 		rc = 0;

	switch (dev->data_dir) {
		case DATA_DIR_NONE:
			break;			// Nothing to send
		case DATA_DIR_UNKNOWN:
			/*should stall the ep*/
			break;
		/* All but the last buffer of data must have already been sent */
		case DATA_DIR_TO_HOST:
			if (dev->data_size == 0) // Nothing to send
				;		
			/* If there's no residue, simply send the last buffer */
			else if (dev->residue == 0) {
				bh->inreq->zero = 0;
				start_debug_transfer(dev, dev->bulk_in, bh->inreq,
						&bh->inreq_busy, &bh->state);
				dev->next_buffhd_to_fill = bh->next;
			}
			break;

		/* We have processed all we want from the data the host has sent.
		 * There may still be outstanding bulk-out requests. */
		case DATA_DIR_FROM_HOST:
			if (dev->residue == 0)
				;		// Nothing to receive
			break;
	}
	return rc;

}
static int do_single_reply(struct usb_debug_dev *dev)
{
	return 0;
}
static int do_datain(struct usb_debug_dev *dev)
{
	struct dbg_buffhd	*bh;
	int rc;
	
	
	bh = dev->next_buffhd_to_fill;
	
	/* Wait for the next buffer to become available */
	while (bh->state != BUF_STATE_EMPTY) {
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
#ifndef TRANSFER_ONLY_IN_KERNEL
	/*wait bulk data*/
	wait_event_interruptible(dev->data_wait, test_bit(DEBUG_DATA_READY,&dev->trans_bitflags));
	clear_bit(DEBUG_DATA_READY,&dev->trans_bitflags);
#endif
	bh->state = BUF_STATE_FULL;
	
#if 0
	int i;
	unsigned char *buf;
	buf = bh->inreq->buf;
	printk("transfer data :");
	for(i=0;i<dev->data_size;i++){
		if((i%16)==0)
			printk("\n");
		printk("%02x ",buf[i]);
	}
	printk("\n");
#endif
	
	bh->inreq->length = dev->data_size;
	start_debug_transfer(dev, dev->bulk_in, bh->inreq,
				&bh->inreq_busy, &bh->state);
	dev->next_buffhd_to_fill = bh->next;
	return 0;
}

static int do_dataout(struct usb_debug_dev *dev)
{
	struct dbg_buffhd	*bh;
	int rc;

	if(dev->data_size==0)
		return 0;

	bh = dev->next_buffhd_to_fill;
	
	while (bh->state != BUF_STATE_EMPTY) {
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
	
	/* Queue a request to read a Bulk-only CBW */
	bh->outreq->length = dev->data_size;
	rc = start_debug_transfer(dev,dev->bulk_out,
			bh->outreq,&bh->outreq_busy,&bh->state);
	
	while (bh->state != BUF_STATE_FULL) {
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
	
#if 0
	int i;
	unsigned char *buf;
	buf = bh->outreq->buf;
	printk("received data :");
	for(i=0;i<dev->data_size;i++){
		if((i%16)==0)
			printk("\n");
		printk("%02x ",buf[i]);
	}
	printk("\n");
#endif

#ifndef TRANSFER_ONLY_IN_KERNEL
	set_bit(DEBUG_DATA_READY,&dev->trans_bitflags);
	wake_up_interruptible(&dev->data_wait);
#endif
	bh->state = BUF_STATE_EMPTY;
	dev->next_buffhd_to_drain = dev->next_buffhd_to_fill;
	dev->next_buffhd_to_fill = bh->next;
	return 0;
}
static int do_data_process(struct usb_debug_dev* dev)
{
	int rc;
	
	switch (dev->bulk_type) {
		case BULK_SINGLE_DATA:
			rc = do_single_reply(dev);
			break;
		case BULK_MULTI_DATA:
			if(dev->data_dir==DATA_DIR_FROM_HOST)
				rc = do_dataout(dev);
			else if(dev->data_dir==DATA_DIR_TO_HOST)
				rc = do_datain(dev);
			else
				return 0;
			break;
		default:
			rc = 0;
			break;
	}
	return rc;
}
static int do_debug_command(struct usb_debug_dev *dev)
{
	struct dbg_buffhd	*bh;
	int			rc;
	//int			reply = -EINVAL;

	/* Wait for the next buffer to become available for data or status */
	bh = dev->next_buffhd_to_fill;
	while (bh->state != BUF_STATE_EMPTY) {
		rc = sleep_debug_thread(dev);
		if (rc)
			return rc;
	}
	rc = do_data_process(dev);
	/* Set up the single reply buffer for finish_reply() */

	/*
	if (reply == -EINTR || signal_pending(current))
		return -EINTR;

	if (reply == -EINVAL)
		reply = 0;		// Error reply length
		
	if (reply >= 0 && dev->data_dir == DATA_DIR_TO_HOST) {
		reply = min((u32) reply, dev->data_size_from_cmnd);
		bh->inreq->length = reply;
		bh->state = BUF_STATE_FULL;
		dev->residue -= reply;
	}	// Otherwise it's already set
	*/
	return rc;
}
static void raise_thread_exception(struct usb_debug_dev *dev,enum dbg_state newstate)
{
	unsigned long		flags;

	/* Do nothing if a higher-priority exception is already in progress.
	 * If a lower-or-equal priority exception is in progress, preempt it
	 * and notify the main thread by sending it a signal. */
	spin_lock_irqsave(&dev->lock, flags);
	if(dev->state<newstate){
		dev->state = newstate;
		if (dev->thread_task)
			send_sig_info(SIGUSR1, SEND_SIG_FORCED,dev->thread_task);
	}
	spin_unlock_irqrestore(&dev->lock, flags);
}

static void debug_handle_exception(struct usb_debug_dev *dev)
{
	siginfo_t		info;
	int			sig;
	int			i;
	int			num_active;
	struct dbg_buffhd	*bh;
	enum dbg_state		old_state;
	struct am_sys_msg msg;
	//int			rc;
	/* Clear the existing signals.  Anything but SIGUSR1 is converted
	* into a high-priority EXIT exception. */
	for (;;) {
		sig = dequeue_signal_lock(current, &current->blocked, &info);
		if (!sig)
			break;
		if (sig != SIGUSR1) {
			INFO( "Main thread exiting on signal\n");
			raise_thread_exception(dev,DBG_STATE_EXIT);
		}
	}
	
	for (i = 0; i < NUM_DBG_BUFFERS; ++i) {
		bh = &dev->buffhds[i];
		if (bh->inreq_busy)
			usb_ep_dequeue(dev->bulk_in, bh->inreq);
		if (bh->outreq_busy)
			usb_ep_dequeue(dev->bulk_out, bh->outreq);
	}

	/* Wait until everything is idle */
	for (;;) {
		for (i = 0; i < NUM_DBG_BUFFERS; ++i) {
			bh = &dev->buffhds[i];
			num_active += bh->inreq_busy + bh->outreq_busy;
		}
		if (num_active == 0)
			break;
		if (sleep_debug_thread(dev))
			return;
	}
	/* Clear out the controller's fifos */
	if (dev->bulk_in_enabled)
		usb_ep_fifo_flush(dev->bulk_in);
	if (dev->bulk_out_enabled)
		usb_ep_fifo_flush(dev->bulk_out);
	
	spin_lock_irq(&dev->lock);

	for (i = 0; i < NUM_DBG_BUFFERS; ++i) {
		bh = &dev->buffhds[i];
		bh->state = BUF_STATE_EMPTY;
	}
	dev->next_buffhd_to_fill = dev->next_buffhd_to_drain =
			&dev->buffhds[0];
	old_state = dev->state;

	dev->state = DBG_STATE_IDLE;
	spin_unlock_irq(&dev->lock);

	/* Carry out any extra actions required for the exception */
	switch (old_state) {
	default:
		break;

	case DBG_STATE_DISCONNECT:
		//usb_debug_intr_config(dev,0);
		msg.type = SYSMSG_USB;
		msg.subtype = DEVICE_USB_VENDOR;
		msg.dataload = DEVICE_DEBUG_QUIT;
		msg.reserved[0]=0xa0;
		am_put_sysmsg(msg);
		INFO("debug plug out\n");
		break;
	case DBG_STATE_EXIT:
	case DBG_STATE_TERMINATED:
		usb_debug_intr_config(dev, 0);			// Free resources
		spin_lock_irq(&dev->lock);
		dev->state = DBG_STATE_TERMINATED;	// Stop the thread
		spin_unlock_irq(&dev->lock);
		INFO("debug safe exit\n");
		break;
	}
}


static int debug_main_thread(void *debug)
{
	struct usb_debug_dev		*usbdev = debug;

	allow_signal(SIGINT);
	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	allow_signal(SIGUSR1);
	/* Allow the thread to be frozen */
	set_freezable();

	/* Arrange for userspace references to be interpreted as kernel
	 * pointers.  That way we can pass a kernel pointer to a routine
	 * that expects a __user pointer and it will work okay. */
	set_fs(get_ds());
	
	/* The main loop */
	while ((!kthread_should_stop())) {//&&(usbdev->state!=DBG_STATE_TERMINATED)

		if ((exception_in_thread(usbdev))||signal_pending(current)) {
			debug_handle_exception(usbdev);
			continue;
		}
		//printk("%s,%d\n",__FUNCTION__,__LINE__);
		
		if (!usbdev->running) {
			sleep_debug_thread(usbdev);
			continue;
		}
		
		
		if(get_debug_command(usbdev))
			continue;
		
		spin_lock_irq(&usbdev->lock);
		if (!exception_in_thread(usbdev))
			usbdev->state = DBG_STATE_DATA_PHASE;
		spin_unlock_irq(&usbdev->lock);
		
		if(do_debug_command(usbdev))
			continue;
		
		spin_lock_irq(&usbdev->lock);
		if (!exception_in_thread(usbdev))
			usbdev->state = DBG_STATE_STATUS_PHASE;
		spin_unlock_irq(&usbdev->lock);
		
		if(send_debug_status(usbdev))
			continue;
		
		spin_lock_irq(&usbdev->lock);
		if (!exception_in_thread(usbdev))
			usbdev->state = DBG_STATE_IDLE;
		spin_unlock_irq(&usbdev->lock);
		
	}

	spin_lock_irq(&usbdev->lock);
	usbdev->thread_task = NULL;
	spin_unlock_irq(&usbdev->lock);
	/* Let the unbind and cleanup routines know the thread has exited */
	//complete_and_exit(&usbdev->thread_notifier, 0);
	return 0;
}

extern struct file_operations usb_debug_io_operations;

static void /* __init_or_exit */ usb_debug_unbind(struct usb_gadget *gadget)
{
	struct usb_debug_dev	*dev = get_gadget_data(gadget);
	struct usb_request	*req = NULL;	
	/* If the thread is running, tell it to exit now */
	if (dev->need_exit != 1) {
		raise_thread_exception(dev, DBG_STATE_EXIT);
		usb_gadget_disconnect(gadget);
		usb_debug_intr_config(dev,0);
		kthread_stop(dev->thread_task);
	}
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
	/* Remove Character Device */
	cdev_del(&dev->dev);
	set_gadget_data(gadget, NULL);
}

static int __init usb_debug_bind(struct usb_gadget *gadget)
{
	struct 	usb_debug_dev	*dev = g_pusb_debug;
	int 	rc,i;
	struct usb_ep			*ep;
	struct usb_request		*req;

	dev->gadget = gadget;
	set_gadget_data(gadget, dev);
	dev->ep0 = gadget->ep0;
	dev->ep0->driver_data = dev;
	
	/*
	 * Register a character device as an interface to a user mode
	 * program that handles the usb_debug specific functionality.
	*/ 
	
	cdev_init(&dev->dev, &usb_debug_io_operations);
	dev->dev.owner = THIS_MODULE;
	rc= cdev_add(&dev->dev, g_usb_debug_devno, 1);
	if (rc) {
		ERR( "Failed to create char device\n");
		goto out;
	}
	set_bit(USB_DEBUG_DEV_NOTREADY, &dev->atomic_bitflags);
	
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

	/* Allocate the data buffers */
	for (i = 0; i < NUM_DBG_BUFFERS; ++i) {
		struct dbg_buffhd	*bh = &dev->buffhds[i];
		bh->buf = (void *)__get_free_pages(GFP_KERNEL,3);
		if (!bh->buf){
			ERR("%s:alloc failed\n",__FUNCTION__);
			goto out;		
		}
		
		bh->next = bh + 1;
		INFO("%s:alloc pages sucess ,addr :%x\n",__FUNCTION__,(u32)bh->buf);

		if(!(dev->outreq = usb_ep_alloc_request(dev->bulk_out,GFP_KERNEL)))
			goto out;
		if(!(dev->inreq  = usb_ep_alloc_request(dev->bulk_in,GFP_KERNEL)))
			goto out;
		
		bh->state = BUF_STATE_EMPTY;
		bh->inreq = dev->inreq;
		bh->outreq = dev->outreq;
		bh->inreq->buf = bh->outreq->buf = bh->buf;
		bh->inreq->context = bh->outreq->context = bh;
		bh->inreq->complete = bulk_in_complete;
		bh->outreq->complete = bulk_out_complete;
	}
	
	dev->buffhds[NUM_DBG_BUFFERS - 1].next = &dev->buffhds[0];
	dev->next_buffhd_to_fill = dev->next_buffhd_to_drain =&dev->buffhds[0];

	usb_gadget_set_selfpowered(gadget);
	sprintf(manufacturer,"actions-micro");	

	spin_lock_irq(&dev->lock);
	dev->running = 0;
	dev->need_exit= 0;
	spin_unlock_irq(&dev->lock);
	
	dev->thread_task = kthread_create(debug_main_thread, dev,
			"usb-debug-gadget");
	if (IS_ERR(dev->thread_task)) {
		rc = PTR_ERR(dev->thread_task);
		goto out;
	}
	/**/
	/* Tell the thread to start working */
	wake_up_process(dev->thread_task);
	
	INFO("usb_debug bind ok\n");
	return 0;
	
autoconf_fail:
	ERR("unable to autoconfigure all endpoints\n");
	rc = -ENOTSUPP;
	
out:
	ERR("bind failed,%d \n",rc);
	usb_debug_unbind(gadget);
	return rc;
}



static void usb_debug_suspend(struct usb_gadget *gadget)
{
	//struct usb_debug_dev		*dev = get_gadget_data(gadget);
	DBG("suspend\n");
}


static void usb_debug_resume(struct usb_gadget *gadget)
{
	//struct usb_debug_dev		*dev = get_gadget_data(gadget);
	DBG("resume\n");
}

/*-------------------------------------------------------------------------*/
static struct usb_gadget_driver		usb_debug_driver = {
#ifdef CONFIG_USB_GADGET_DUALSPEED
	.speed		= USB_SPEED_HIGH,
#else
	.speed		= USB_SPEED_FULL,
#endif

	.function		= (char*)longname,
	.bind		= usb_debug_bind,
	.unbind		= usb_debug_unbind,
	.disconnect	= usb_debug_disconnect,
	.setup		= usb_debug_setup,
	
	.suspend		= usb_debug_suspend,
	.resume		= usb_debug_resume,

	.driver		= {
		.name		= shortname,
		.owner		= THIS_MODULE,
		// .release = ...
		// .suspend = ...
		// .resume = ...
	},
};

static int __init usb_debug_init(void)
{
	int result;
	struct usb_debug_dev *pdev ;
	
	pdev = kzalloc(sizeof(struct usb_debug_dev),GFP_KERNEL);
	if(!pdev)
		return -ENOMEM;

	pdev->bulk_in_enabled =
		pdev->bulk_out_enabled = 0;
	
	spin_lock_init(&pdev->lock);
	init_waitqueue_head(&pdev->cmd_wait);
	init_waitqueue_head(&pdev->data_wait);
	init_waitqueue_head(&pdev->csw_wait);
	g_pusb_debug = pdev;

	usb_gadget_class = class_create(THIS_MODULE,"usb-debug-gadget");
	if (IS_ERR(usb_gadget_class)) {
		result = PTR_ERR(usb_gadget_class);
		ERR("unable to create usb_gadget class %d\n", result);
		goto failed;
	}

	if ( (result = alloc_chrdev_region(&g_usb_debug_devno, 
			0, 1,"usb_debug")) < 0) {
		ERR("alloc_chrdev_region %d\n", result);
		goto failed1;
	}
	
	device_create_drvdata(usb_gadget_class, NULL,
				      g_usb_debug_devno,
				      NULL, "usb_debug");
	
	if((result = usb_gadget_register_driver(&usb_debug_driver) )< 0){
		ERR("usb_debuglay driver init failed\n");
		goto failed2;
	}
	create_debug_file(pdev);
	INFO("usb_debug init ok\n");
	return 0;
failed2:
	unregister_chrdev_region(g_usb_debug_devno, 1);
failed1:
	class_destroy(usb_gadget_class);
failed:
	kfree(pdev);
	g_pusb_debug = pdev = NULL;
	return result;
}


static void __exit usb_debug_exit(void)
{	
	device_destroy(usb_gadget_class,g_usb_debug_devno);
	unregister_chrdev_region(g_usb_debug_devno, 2);
	class_destroy(usb_gadget_class);
	usb_gadget_unregister_driver(&usb_debug_driver);
	remove_debug_file(g_pusb_debug);
	if(g_pusb_debug){
		kfree(g_pusb_debug);
		g_pusb_debug = NULL;
	}
	INFO("usb_debug_exit success\n");
}

module_init(usb_debug_init);
module_exit(usb_debug_exit);
