/**
 * drivers/char/amreg.c
 *
 *this driver is for register op in user space
 *
 *author: yekai
 *date:2012-11-19
 *version:0.1
 *----------------------
 *----------------------
 */


#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/major.h>
#include <linux/device.h>

#include <actions_io.h>
#include <am7x_gpio.h>
#include <sys_gpio.h>
#include <am7x_board.h>

#include "actions_regs.h"
#include "actions_io.h"


#define AM_REG_START_ADDR		(0)
#define am_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))
#define am_readl(port)  (*(volatile unsigned int *)(port))



struct amreg_t{
	char *driver_name;
	int dev;
	struct cdev *cdev;
	struct class *reg_class;
};



static struct amreg_t am_reg = {
	.driver_name = "amreg",
};



static int am_gpio_open(struct inode *pinode, struct file *pfile)
{
	if(!pfile)
		return -ENOENT;

	pfile->f_pos = 0;

	return 0;
}



static int am_reg_ioctl(struct inode *pinode, struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int ret=0;
	INT32U *val = (INT32U *)arg;


	switch(cmd){
		case 0://REG_READ
			val[1] = am_readl(val[0]);
			break;

		case 1://REG_WRITE
			am_writel(val[1], val[0]);
		break;
		default:
			printk("[%s]error cmd = 0x%x\n", __func__, cmd);
			break;
	}

	return ret;
}



static struct file_operations regdrv_fops=
{
	.owner  = THIS_MODULE,
	.open = am_gpio_open,
	.ioctl = am_reg_ioctl,
};



static int  __init am_reg_init(void)
{
	int result;
	int major = 0;
	int minor = 0;
	int count = 1;


	printk("[%s] start!\n", __func__);
	if(major)
	{
		am_reg.dev = MKDEV(major, minor);
		result = register_chrdev_region(am_reg.dev, count, am_reg.driver_name);
	}
	else
	{
		result = alloc_chrdev_region(&am_reg.dev, minor, count, am_reg.driver_name);
		major = MAJOR(am_reg.dev);
	}
	printk(KERN_INFO"[%s]major =%d\n", __func__, major);

	am_reg.cdev= cdev_alloc();
	cdev_init(am_reg.cdev,&regdrv_fops);
	am_reg.cdev->owner = THIS_MODULE;
	
	result = cdev_add(am_reg.cdev, am_reg.dev, count);
	if(result < 0)
	{
		printk("can't add [%s]dev", __func__);
		unregister_chrdev_region(am_reg.dev, count);
		cdev_del(am_reg.cdev);
		return result;
	}

	am_reg.reg_class = class_create(THIS_MODULE, "am_reg_class");
	if(IS_ERR(am_reg.reg_class)) {
		unregister_chrdev_region(am_reg.dev, count);	
		cdev_del(am_reg.cdev);
		printk("Err: failed in creating class.\n");
		return -1; 
	}

	device_create(am_reg.reg_class, NULL, am_reg.dev, NULL, am_reg.driver_name);//创建设备文件

	return 0;
}

static void __exit am_reg_exit(void)
{
	int count = 1;
	
	if(am_reg.cdev)
		cdev_del(am_reg.cdev);
	
	if(am_reg.reg_class){
		device_destroy(am_reg.reg_class, am_reg.dev);    //delete device node under /dev//必须先删除设备，再删除class类
		class_destroy(am_reg.reg_class);                 //delete class created by us
	}
	
	if(am_reg.dev)
		unregister_chrdev_region(am_reg.dev, count);
}


module_init(am_reg_init);
module_exit(am_reg_exit);

MODULE_AUTHOR("Charles");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Using to set reg from use space");

