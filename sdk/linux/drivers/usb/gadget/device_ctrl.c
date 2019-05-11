#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/string.h>

#include "file_debug.h"
#include <linux/delay.h>
#include "usb_debug.h"


static int usb_debug_open(struct inode *inode, struct file *fd)
{
	struct usb_debug_dev	*dev;
	int	ret = -EBUSY ;
	
	dev = container_of(inode->i_cdev, struct usb_debug_dev, dev);
	
	spin_lock(&dev->lock);
	if (test_bit(USB_DEBUG_DEV_OPEN,&dev->atomic_bitflags)) {
		ERR("only opened once\n");
		goto endl;
	}
	
	set_bit(USB_DEBUG_DEV_OPEN, &dev->atomic_bitflags);
	fd->private_data = dev;
	ret = 0;
	DBG( "usb_debug open ok\n");
	
endl:	
	spin_unlock(&dev->lock);
	DBG("printer_open returned %x\n", ret);
	return ret;
}

static int usb_debug_read(struct file *pfile, char __user *buf, size_t size, loff_t *offset)
{
	int result=0;
	struct usb_debug_dev*dev;

	dev = pfile->private_data;
	DBG( "usb_debug read\n");
	
	wait_event_interruptible(dev->data_wait, test_bit(DEBUG_DATA_READY,&dev->trans_bitflags));
	clear_bit(DEBUG_DATA_READY,&dev->trans_bitflags);
	
	/*copy bulk data*/
	if(copy_to_user(buf,dev->next_buffhd_to_drain->buf,size)){
		printk(KERN_ERR"copy to user data err\n");
		return -EFAULT;
	}	
	//dev->next_buffhd_to_drain->state = BUF_STATE_EMPTY;
	DBG( "usb_debug read ok\n");
	return result;
}

static int usb_debug_write(struct file *pfile, const char __user *buf, size_t size, loff_t *offset)
{	
	int result=0;
	struct usb_debug_dev*dev;
	struct _bulk_head	cur_data;
	int info_size = sizeof(struct _bulk_head);
	dev = pfile->private_data;
	DBG( "usb_debug write\n");
	dev->cmd_type = CMD_PHASE_ERROR;
	/*copy user head*/
	if(copy_from_user(&cur_data,buf,info_size)){
		printk(KERN_ERR"copy from user data err\n");
		return -EFAULT;
	}
	/*copy user data*/
	if(copy_from_user(dev->next_buffhd_to_fill->buf,buf+info_size,size-info_size)){
		printk(KERN_ERR"copy from user data err\n");
		return -EFAULT;
	}
	dev->cmd_type = CMD_PHASE_GOOD;
	dev->residue = dev->data_size - (size - info_size);
	dev->cmnd_size = cur_data.infolen;
	memcpy(dev->cmnd,cur_data.info,info_size);
	//dev->next_buffhd_to_fill->state = BUF_STATE_FULL;
	set_bit(DEBUG_DATA_READY,&dev->trans_bitflags);
	wake_up_interruptible(&dev->data_wait);
	
	DBG( "usb_debug write ok\n");
	return result;
}


static int usb_debug_release(struct inode *inode, struct file *fd)
{
	struct usb_debug_dev	*dev = fd->private_data;
	if (!test_bit(USB_DEBUG_DEV_OPEN,&dev->atomic_bitflags)) {
		ERR("only close once\n");
		return -ESHUTDOWN;
	}
	
	spin_lock(&dev->lock);
	clear_bit(USB_DEBUG_DEV_OPEN,&dev->atomic_bitflags);
	fd->private_data = NULL;
	spin_unlock(&dev->lock);
	
	DBG( "usb_debuglay close \n");
	return 0;
}


static long usb_debug_ioctl(struct file *pfile, unsigned int code, unsigned long arg)
{
	int result = 0;
	struct usb_debug_dev*dev;
	void __user *buf = (void __user *)arg;
	struct _cmd_info	cmd;
	struct _bulk_head cur_info;
	dev = pfile->private_data;
	DBG( "usb_debuglay ioctl cmd=%x \n",code);
	switch(code){
		case IOCTL_USB_DEBUG_GET_CMD:
			wait_event_interruptible(dev->cmd_wait, test_bit(DEBUG_CMD_READY,&dev->trans_bitflags));
			clear_bit(DEBUG_CMD_READY,&dev->trans_bitflags);
	
			cmd.bulk_type = dev->bulk_type;
			cmd.datadir = dev->data_dir;
			cmd.datalen = dev->data_size;
			cmd.head.cmd_type = dev->cmd_type;
			cmd.head.infolen = dev->cmnd_size;
			memcpy(cmd.head.info,dev->cmnd,sizeof(dev->cmnd));
			if(copy_to_user((void *)arg,&cmd,sizeof(struct _cmd_info)))
				return -EFAULT;
			break;
		case IOCTL_USB_DEBUG_SET_CSW:
			if(buf){
				if(copy_from_user(&cur_info,buf,sizeof(struct _bulk_head)))
					return -EFAULT;
				dev->cmd_type = cur_info.cmd_type;
				dev->cmnd_size = cur_info.infolen;
				memcpy(dev->cmnd,cur_info.info,sizeof(cur_info.info));
			}
			
			set_bit(DEBUG_CSW_READY,&dev->trans_bitflags);
			wake_up_interruptible(&dev->csw_wait);
			break;
		default:
			break;
	}

	return result;
}

struct file_operations usb_debug_io_operations = {
	.owner =	THIS_MODULE,
	.open 			=	usb_debug_open,
	.unlocked_ioctl 	= 	usb_debug_ioctl,
	.read			=	usb_debug_read,
	.write			=	usb_debug_write,
	.release 			=	usb_debug_release
};

